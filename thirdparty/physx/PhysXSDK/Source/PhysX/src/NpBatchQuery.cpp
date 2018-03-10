/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#include "NpBatchQuery.h"
#include "NpReadCheck.h"
#include "NpActor.h"
#include "NpShapeManager.h"
#include "SqUtilities.h"
#include "PsAtomic.h"
#include "PsFoundation.h"
#include "PsUtilities.h"
#include "NpScene.h"

using namespace physx;
using namespace Sq;
using namespace Cm;

#if !defined(PX_P64)
PX_COMPILE_TIME_ASSERT(0==(sizeof(PxRaycastHit)& 0x0f));
PX_COMPILE_TIME_ASSERT(0==(sizeof(PxSweepHit)& 0x0f));
PX_COMPILE_TIME_ASSERT(0==(sizeof(PxOverlapHit)& 0x0f));
#endif

#define DMA_STORE_CHANNEL 2

#define CHECK_RUNNING(QueryMessage) \
	if(Ps::atomicCompareExchange(&mBatchQueryIsRunning, -1, 0) == 1)\
	{\
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, QueryMessage); return;\
	}

NpBatchQuery::NpBatchQuery(NpScene& owner, const PxBatchQueryDesc& d)
	: mNpScene(&owner), mNbRaycasts(0), mNbOverlaps(0), mNbSweeps(0), mBatchQueryIsRunning(0), mDesc(d), mPrevOffset(PxU32(eTERMINAL))
{
	mRaycastTask = NULL;
	mOverlapTask = NULL;
	mSweepTask = NULL;
	mHasMtdSweep = false;
}

NpBatchQuery::~NpBatchQuery()
{
	mNpScene->getSceneQueryManagerFast().freeSPUTasks(this);
}

void NpBatchQuery::setUserMemory(const PxBatchQueryMemory& userMem)
{
	if(Ps::atomicCompareExchange(&mBatchQueryIsRunning, 0, 0) != 0)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxBatchQuery::setUserMemory: This batch is still executing, skipping setUserMemory"); 
		return;
	}

	PxBatchQueryDesc& desc = getDesc();

	desc.queryMemory = userMem;
}

const PxBatchQueryMemory& NpBatchQuery::getUserMemory()
{
	return getDesc().queryMemory;
}

// ROS abbreviates Raycast/Overlap/Sweep
struct QTypeROS { enum Enum { eRAYCAST = 0, eOVERLAP = 1, eSWEEP = 2 }; }; // AP: perhaps can be shared with some other code

namespace physx
{
struct BatchStreamHeader
{
	BatchStreamHeader(
		PxHitFlags aHitFlags, const PxQueryCache* aCache, const PxQueryFilterData& aFd,
		void* aUserData, PxU16 aMaxTouchHits, QTypeROS::Enum aHitTypeId) :
			hitFlags(aHitFlags), fd(aFd), userData(aUserData), cache(aCache),
			maxTouchHits(aMaxTouchHits), hitTypeId(char(aHitTypeId))
	{
		nextQueryOffset = PxU32(NpBatchQuery::eTERMINAL);
	}

	BatchStreamHeader() {}

	// TODO: possibly maintain 3 separate offset lists in the same array for raycasts/overlaps/sweeps
	// offset of a 4-byte ptr to offset in the previously stored batch query in the stream, this is not a ptr because of reallocs
	PxU32						nextQueryOffset;
	PxHitFlags					hitFlags;
	PxQueryFilterData			fd;
	void*						userData;
	const PxQueryCache*			cache;
	PxU16						maxTouchHits;
	char						hitTypeId; // Sq::QTypeROS
};
} // namespace physx

static void writeGeom(BatchQueryStream& stream, const PxGeometry& geom)
{
	PxGeometryType::Enum geomType = geom.getType();
	stream.write<PxU32>(PxU32(geomType));
	switch (geomType)
	{
	case PxGeometryType::eCAPSULE:
		stream.write<PxCapsuleGeometry>(static_cast<const PxCapsuleGeometry&>(geom));
		break;
	case PxGeometryType::eSPHERE:
		stream.write<PxSphereGeometry>(static_cast<const PxSphereGeometry&>(geom));
		break;
	case PxGeometryType::eCONVEXMESH:
		stream.write<PxConvexMeshGeometry>(static_cast<const PxConvexMeshGeometry&>(geom));
		break;
	case PxGeometryType::eBOX:
		stream.write<PxBoxGeometry>(static_cast<const PxBoxGeometry&>(geom));
		break;
	case PxGeometryType::ePLANE:
	case PxGeometryType::eTRIANGLEMESH:
	case PxGeometryType::eHEIGHTFIELD:
	case PxGeometryType::eGEOMETRY_COUNT:
	case PxGeometryType::eINVALID:
	default:
		PX_ALWAYS_ASSERT_MESSAGE("Unsupported geometry type in writeGeom");
	}
}

static PxGeometry* readGeom(BatchQueryStreamReader& reader)
{
	PxU32 geomType = *reader.read<PxU32>();
	PxGeometry* anyGeom = NULL;
	switch (geomType)
	{
	case PxGeometryType::eCAPSULE:
		anyGeom = reader.read<PxCapsuleGeometry>();
		break;
	case PxGeometryType::eSPHERE:
		anyGeom = reader.read<PxSphereGeometry>();
		break;
	case PxGeometryType::eCONVEXMESH:
		anyGeom = reader.read<PxConvexMeshGeometry>();
		break;
	case PxGeometryType::eBOX:
		anyGeom = reader.read<PxBoxGeometry>();
		break;
	default:
		PX_ALWAYS_ASSERT_MESSAGE("Unsupported geometry type in readGeom");
	}

	return anyGeom;
}

static void writeQueryInput(BatchQueryStream& stream, const MultiQueryInput& input)
{
	stream.write<MultiQueryInput>(input);
	if (input.rayOrigin)
		stream.write<PxVec3>(input.rayOrigin);
	if (input.unitDir)
		stream.write<PxVec3>(input.unitDir);
	if (input.pose)
		stream.write<PxTransform>(input.pose);
	if (input.geometry)
		writeGeom(stream, *input.geometry);
}

static MultiQueryInput* readQueryInput(BatchQueryStreamReader& stream)
{
	MultiQueryInput* input = stream.read<MultiQueryInput>();
	if (input->rayOrigin)
		input->rayOrigin = stream.read<PxVec3>();
	if (input->unitDir)
		input->unitDir = stream.read<PxVec3>();
	if (input->pose)
		input->pose = stream.read<PxTransform>();
	if (input->geometry)
		input->geometry = readGeom(stream);

	return input;
}

extern NpQuerySpuContext* gSpuContext;

template<typename ResultType, typename HitType>
void writeStatus(ResultType* aRes, const PxHitBuffer<HitType>& hits, void* userData, bool overflow)
{
	PX_COMPILE_TIME_ASSERT((!PX_IS_PS3) || (sizeof(ResultType) % 16 == 0));
	ResultType tmp;
	ResultType* res = PX_IS_SPU ? &tmp : aRes;
	res->userData = userData;
	res->block = hits.block;
	res->hasBlock = hits.hasBlock;
	res->nbTouches = hits.nbTouches;
	res->queryStatus = PxU8(overflow ? PxBatchQueryStatus::eOVERFLOW : PxBatchQueryStatus::eSUCCESS);
	res->touches = (overflow && res->nbTouches == 0) ? NULL : hits.touches;

	if (PX_IS_SPU)
	{
		memStoreAsync<ResultType>(res, MemFetchPtr(aRes), DMA_STORE_CHANNEL);
		memStoreWait(DMA_STORE_CHANNEL);
	}
}

void NpBatchQuery::resetResultBuffers()
{
	for (PxU32 i = 0; i<mNbRaycasts; i++)
	{
		mDesc.queryMemory.userRaycastResultBuffer[i].queryStatus = PxBatchQueryStatus::ePENDING;
		mDesc.queryMemory.userRaycastResultBuffer[i].hasBlock = false;
		mDesc.queryMemory.userRaycastResultBuffer[i].nbTouches = 0;
		mDesc.queryMemory.userRaycastResultBuffer[i].touches = NULL;
		mDesc.queryMemory.userRaycastResultBuffer[i].userData = NULL;
	}
	for (PxU32 i = 0; i<mNbOverlaps; i++)
	{
		mDesc.queryMemory.userOverlapResultBuffer[i].queryStatus = PxBatchQueryStatus::ePENDING;
		mDesc.queryMemory.userOverlapResultBuffer[i].hasBlock = false;
		mDesc.queryMemory.userOverlapResultBuffer[i].nbTouches = 0;
		mDesc.queryMemory.userOverlapResultBuffer[i].touches = NULL;
		mDesc.queryMemory.userOverlapResultBuffer[i].userData = NULL;
	}
	for (PxU32 i = 0; i<mNbSweeps; i++)
	{
		mDesc.queryMemory.userSweepResultBuffer[i].queryStatus = PxBatchQueryStatus::ePENDING;
		mDesc.queryMemory.userSweepResultBuffer[i].hasBlock = false;
		mDesc.queryMemory.userSweepResultBuffer[i].nbTouches = 0;
		mDesc.queryMemory.userSweepResultBuffer[i].touches = NULL;
		mDesc.queryMemory.userSweepResultBuffer[i].userData = NULL;
	}
}

// executed on PPU to raise an error on aborted query
void NpBatchQuery::checkForSPUErrors()
{
	// implementation archived in revision 30
}

void NpBatchQuery::finalizeExecute()
{
	if (PX_IS_SPU)
	{
		memStoreWait(DMA_STORE_CHANNEL); // wait on any outstanding result & hit stores
	}
	else
	{
		mPrevOffset = PxU32(eTERMINAL); // reset the first BatchStreamHeader offset
		mStream.rewind(); // clear out the executed queries - rewind the data stream so that the query object can be reused
		mNbRaycasts = mNbOverlaps = mNbSweeps = 0; // also reset the counts so the query object can be reused
		mHasMtdSweep = false; // reset the mtd flag

		Ps::atomicExchange(&mBatchQueryIsRunning, 0);
	}
}

// fixed memory buffer with extra single hit for overflow detection
template<typename HitType>
struct PxOverflowBuffer : PxHitBuffer<HitType>
{
	bool overflow;
	PxU32 saveNbTouches;
	HitType extraHit;
	HitType* saveTouches;
	bool processCalled;
	PxOverflowBuffer(HitType* hits, PxU32 count) : PxHitBuffer<HitType>(hits, count), overflow(false), processCalled(false)
	{
	}

	virtual PxAgain processTouches(const HitType* /*hits*/, PxU32 /*count*/)
	{
		if (processCalled)
			return false;
		saveTouches = this->touches;
		saveNbTouches = this->nbTouches;
		processCalled = true;
		this->touches = PX_IS_SPU ? this->touches : &extraHit; // on SPU we overwrite the first hit since we will overflow anyway
		this->maxNbTouches = 1;
		return true;
	}

	virtual void finalizeQuery()
	{
		if (processCalled)
		{
			overflow = (this->nbTouches > 0);
			this->nbTouches = saveNbTouches;
			this->touches = saveTouches;
		}
	}
};

void NpBatchQuery::execute()
{
	NP_READ_CHECK(mNpScene);

	if(mNbRaycasts)
	{
		PX_CHECK_AND_RETURN(mDesc.queryMemory.userRaycastResultBuffer!=NULL, "PxBatchQuery execute: userRaycastResultBuffer is NULL");
		PX_CHECK_AND_RETURN(mDesc.queryMemory.raycastTouchBufferSize > 0 ? 
			(mDesc.queryMemory.userRaycastTouchBuffer != NULL)	: true, "PxBatchQuery execute: userRaycastTouchBuffer is NULL");
	}
	if(mNbOverlaps)
	{
		PX_CHECK_AND_RETURN(mDesc.queryMemory.userOverlapResultBuffer!=NULL, "PxBatchQuery execute: userOverlapResultBuffer is NULL");
		PX_CHECK_AND_RETURN(mDesc.queryMemory.overlapTouchBufferSize > 0 ? 
			(mDesc.queryMemory.userOverlapTouchBuffer != NULL)	: true, "PxBatchQuery execute: userOverlapTouchBuffer is NULL");
	}
	if(mNbSweeps)
	{
		PX_CHECK_AND_RETURN(mDesc.queryMemory.userSweepResultBuffer!=NULL, "PxBatchQuery execute: userSweepResultBuffer is NULL");
		PX_CHECK_AND_RETURN(mDesc.queryMemory.sweepTouchBufferSize > 0 ? 
			(mDesc.queryMemory.userSweepTouchBuffer != NULL)	: true, "PxBatchQuery execute: userSweepTouchBuffer is NULL");
	}

	PX_SIMD_GUARD;

#if !PX_IS_SPU
		CM_PROFILE_ZONE_WITH_SUBSYSTEM(mNpScene->getScene(),BatchedSceneQuery,execute);
		PxI32 ret = Ps::atomicCompareExchange(&mBatchQueryIsRunning, 1, 0);
		if(ret == 1)
		{
			Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxBatchQuery::execute: This batch is already executing"); 
			return;
		}
		else if(ret == -1)
		{
			Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxBatchQuery::execute: Another thread is still adding queries to this batch"); 
			return;
		}

		resetResultBuffers(); // reset the buffers on PPU for now, possible optimization with DMA reset on SPU
#endif

	// If PVD is connected and IS_PVD_SQ_ENABLED, record the offsets for queries in pvd buffers and run the queries on PPU
	bool isSqCollectorLocked = false;
	PX_UNUSED(isSqCollectorLocked);

#if PX_SUPPORT_VISUAL_DEBUGGER	
	PxU32 pvdRayQstartIdx = 0;
	PxU32 pvdOverlapQstartIdx = 0;
	PxU32 pvdSweepQstartIdx = 0;	
	if(mNpScene->getScene().getSceneVisualDebugger().isConnected(true) && IS_PVD_SQ_ENABLED)
	{
		mNpScene->getBatchedSqCollector().getLock().lock();
		isSqCollectorLocked = true;
	
		pvdRayQstartIdx = mNpScene->getBatchedSqCollector().mAccumulatedRaycastQueries.size();
		pvdOverlapQstartIdx = mNpScene->getBatchedSqCollector().mAccumulatedOverlapQueries.size();
		pvdSweepQstartIdx = mNpScene->getBatchedSqCollector().mAccumulatedSweepQueries.size();
	}
#endif

	bool runOnPPU[3] = { true, true, true }; // also present on SPU technically, but never gets set to false
	SceneQueryManager& sqm = mNpScene->getSceneQueryManagerFast();
	if (PX_IS_PPU && !isSqCollectorLocked && sqm.canRunOnSPU(*this))
	{
		// flush updates here since we can't do it on SPU
		sqm.flushUpdates();

		// =================== PS3 PPU batch query ==================
		// kick off SPU tasks for raycast/overlaps/sweeps and block
		sqm.blockingSPURaycastOverlapSweep(this, runOnPPU);
		if (!((runOnPPU[0] && mNbRaycasts > 0) || (runOnPPU[1] && mNbOverlaps > 0) || (runOnPPU[2] && mNbSweeps > 0)))
		{
			checkForSPUErrors();
			finalizeExecute();
			return;
		} // fall through to PPU query if any of the 3 runOnPPU bools are set to true
	}
	// else fall through to PPU. could be because of eNONE pruning structure
	// also so that a shared breakpoint can be set in PS3 debugger

	PxClientID clientId = mDesc.ownerClient;

	// setup local pointers to user provided output buffers
	PxRaycastHit* raycastHits = mDesc.queryMemory.userRaycastTouchBuffer;					PX_UNUSED(raycastHits);
	PxRaycastQueryResult* raycastResults = mDesc.queryMemory.userRaycastResultBuffer;		PX_UNUSED(raycastResults);
	PxU32 raycastHitsSize = mDesc.queryMemory.raycastTouchBufferSize;						PX_UNUSED(raycastHitsSize);

	PxOverlapHit* overlapHits = mDesc.queryMemory.userOverlapTouchBuffer;					PX_UNUSED(overlapHits);
	PxOverlapQueryResult* overlapResults = mDesc.queryMemory.userOverlapResultBuffer;		PX_UNUSED(overlapResults);
	PxU32 overlapHitsSize = mDesc.queryMemory.overlapTouchBufferSize;						PX_UNUSED(overlapHitsSize);

	PxSweepHit* sweepHits = mDesc.queryMemory.userSweepTouchBuffer;							PX_UNUSED(sweepHits);
	PxSweepQueryResult* sweepResults = mDesc.queryMemory.userSweepResultBuffer;				PX_UNUSED(sweepResults);
	PxU32 sweepHitsSize = mDesc.queryMemory.sweepTouchBufferSize;							PX_UNUSED(sweepHitsSize);

	BatchQueryFilterData bfd(mDesc.filterShaderData, mDesc.filterShaderDataSize, mDesc.preFilterShader, mDesc.postFilterShader);

	// data declarations for double buffering the input stream
	PX_COMPILE_TIME_ASSERT(SPU_BQ_STREAM_BUFFER_SIZE >= 256); // AP this is unsafe, any single query has to fit in this buffer
	const PxU32 bufSize = PX_IS_SPU ? SPU_BQ_STREAM_BUFFER_SIZE : 1;
	PX_ALIGN_PREFIX(16) char buffer[(bufSize+32*PX_IS_SPU)*2] PX_ALIGN_SUFFIX(16); // extra 32 bytes for unaligned transfers
	PxU32 channels[2] = { 1, 2 }, chanIdx = 0;
	char* buffers[2] = { buffer, buffer+bufSize+32*PX_IS_SPU };
	PxU32 curQueryOffset = 0; // first query starts at 0
	if (mPrevOffset == eTERMINAL) // except if zero queries were queued
	{
		finalizeExecute();
		return;
	}

	// fetch the first buffer
	char* dmaPtr = memFetchAsync<char>(buffers[chanIdx], MemFetchPtr(mStream.begin()+curQueryOffset), bufSize, channels[chanIdx]);

	PxU32 hitsSpaceLeft; PX_UNUSED(hitsSpaceLeft);

	// ====================== parse and execute the batch query memory stream ====================== 
	PxU32 queryCount = 0;
	do {
		// block on previous buffer (SPU only, compiles out on PPU)
		memFetchWait(channels[chanIdx]);

		// parse a query from the input stream, create a stream reader at current double buffer
		BatchQueryStreamReader reader(PX_IS_SPU ? dmaPtr : mStream.begin()+curQueryOffset);
		BatchStreamHeader& h = *reader.read<BatchStreamHeader>();
		if (h.fd.clientId == 0)
			h.fd.clientId = clientId; // override a zero clientId with PxBatchQueryDesc.ownerClient

		// Start fetching the next buffer. We overfetch here, could consider storing the size for the next buffer in the prev
		dmaPtr = memFetchAsync<char>(buffers[1-chanIdx], MemFetchPtr(mStream.begin()+h.nextQueryOffset), bufSize, channels[1-chanIdx]);
		curQueryOffset = h.nextQueryOffset;
		Ps::prefetchLine(mStream.begin() + curQueryOffset);
		chanIdx = 1-chanIdx;

		MultiQueryInput& input = *readQueryInput(reader);

		// ====================== switch over query type - QTypeROS::eRAYCAST, eOVERLAP, eSWEEP =====================
		switch (h.hitTypeId)
		{
			// =============== Current query is a raycast =====================
			case QTypeROS::eRAYCAST: if ((!PX_IS_SPU | IS_SPU_RAYCAST) && runOnPPU[0])
			{
				PxU32 nbRaycastHits = PxU32(raycastHits - mDesc.queryMemory.userRaycastTouchBuffer);
				PX_ASSERT(nbRaycastHits <= raycastHitsSize);
				hitsSpaceLeft = raycastHitsSize - nbRaycastHits;
				PxOverflowBuffer<PxRaycastHit> hits(raycastHits, PxMin<PxU32>(h.maxTouchHits, hitsSpaceLeft));
				mNpScene->NpScene::multiQuery<PxRaycastHit>(input, hits, h.hitFlags, h.cache, h.fd, NULL, &bfd);
				hits.overflow |= (hitsSpaceLeft == 0 && h.maxTouchHits > 0); // report overflow if 0 space left and maxTouchHits>0
				writeStatus<PxRaycastQueryResult, PxRaycastHit>(raycastResults++, hits, h.userData, hits.overflow);
				raycastHits += hits.nbTouches;
			} break;

			// ================ Current query is an overlap ====================
			case QTypeROS::eOVERLAP: if ((!PX_IS_SPU | IS_SPU_OVERLAP) && runOnPPU[1])
			{
				PxU32 nbOverlapHits = PxU32(overlapHits - mDesc.queryMemory.userOverlapTouchBuffer);
				PX_ASSERT(nbOverlapHits <= overlapHitsSize);
				hitsSpaceLeft = overlapHitsSize - nbOverlapHits;
				PxOverflowBuffer<PxOverlapHit> hits(overlapHits, PxMin<PxU32>(h.maxTouchHits, hitsSpaceLeft));
				mNpScene->NpScene::multiQuery<PxOverlapHit>(input, hits, h.hitFlags, h.cache, h.fd, NULL, &bfd);
				hits.overflow |= (hitsSpaceLeft == 0 && h.maxTouchHits > 0); // report overflow if 0 space left and maxTouchHits>0
				writeStatus<PxOverlapQueryResult, PxOverlapHit>(overlapResults++, hits, h.userData, hits.overflow);
				overlapHits += hits.nbTouches;
			} break;

			// ================== Current query is a sweep =========================
			case QTypeROS::eSWEEP: if ((!PX_IS_SPU | IS_SPU_SWEEP) && runOnPPU[2])
			{

				PxU32 nbSweepHits = PxU32(sweepHits - mDesc.queryMemory.userSweepTouchBuffer);
				PX_ASSERT(nbSweepHits <= sweepHitsSize);
				hitsSpaceLeft = sweepHitsSize - nbSweepHits;
				PxOverflowBuffer<PxSweepHit> hits(sweepHits, PxMin<PxU32>(h.maxTouchHits, hitsSpaceLeft));
				mNpScene->NpScene::multiQuery<PxSweepHit>(input, hits, h.hitFlags, h.cache, h.fd, NULL, &bfd);
				hits.overflow |= (hitsSpaceLeft == 0 && h.maxTouchHits > 0); // report overflow if 0 space left and maxTouchHits>0
				writeStatus<PxSweepQueryResult, PxSweepHit>(sweepResults++, hits, h.userData, hits.overflow);
				sweepHits += hits.nbTouches;
			} break;
			default:
				PX_ALWAYS_ASSERT_MESSAGE("Unexpected batch query type (raycast/overlap/sweep).");
		}

		if (h.nextQueryOffset == eTERMINAL) // end of stream
			// AP: previously also had a break on hitCount==-1 which is aborted due to out of space
			// abort stream parsing if we ran into an aborted query (hitCount==-1).. but it was easier to just continue
			// the perf implications for aborted queries are not a significant consideration and this allows to avoid
			// writing special case code for filling the query buffers after aborted query
			break;
		#undef MULTIQ
		queryCount++;
	} while (queryCount < 1000000);
	//PX_ASSERT(queryCount == mNbRaycasts*IS_SPU_RAYCAST + mNbOverlaps*IS_SPU_OVERLAP + mNbSweeps*IS_SPU_SWEEP);

#if PX_SUPPORT_VISUAL_DEBUGGER
	if(mNpScene->getScene().getSceneVisualDebugger().isConnected(true) && isSqCollectorLocked && IS_PVD_SQ_ENABLED)
	{
		mNpScene->getBatchedSqCollector().collectBatchedRaycastHits( mDesc.queryMemory.userRaycastResultBuffer, mNbRaycasts, pvdRayQstartIdx);
		mNpScene->getBatchedSqCollector().collectBatchedOverlapHits( mDesc.queryMemory.userOverlapResultBuffer, mNbOverlaps, pvdOverlapQstartIdx);
		mNpScene->getBatchedSqCollector().collectBatchedSweepHits( mDesc.queryMemory.userSweepResultBuffer, mNbSweeps, pvdSweepQstartIdx);
	}

	// Maybe connection is disconnected after the lock
	if( isSqCollectorLocked )
	{
		mNpScene->getBatchedSqCollector().getLock().unlock();
		isSqCollectorLocked = false;
	}
#endif
	
	finalizeExecute();
}

///////////////////////////////////////////////////////////////////////////////
void NpBatchQuery::writeBatchHeader(const BatchStreamHeader& h)
{
	PxU32 streamPos = (PxU32)mStream.getPos(); // save the stream pos before we write the header
	mStream.write<BatchStreamHeader>(h);
	// link into a list as offset
	PxU32* prevPtr = (mPrevOffset == eTERMINAL) ? &mPrevOffset : reinterpret_cast<PxU32*>(mStream.begin()+mPrevOffset);
	PxU32 headerPtr = streamPos+offsetof(BatchStreamHeader, nextQueryOffset);
	*prevPtr = headerPtr;
	mPrevOffset = headerPtr;
}

///////////////////////////////////////////////////////////////////////////////
void NpBatchQuery::raycast(
	const PxVec3& origin, const PxVec3& unitDir, PxReal distance, PxU16 maxTouchHits,
	PxHitFlags hitFlags, const PxQueryFilterData& fd, void* userData, const PxQueryCache* cache)
{
	PX_CHECK_AND_RETURN(distance>0, "PxBatchQuery::raycast: The maximum distance must be greater than zero!");
	PX_CHECK_AND_RETURN(unitDir.isNormalized(), "PxBatchQuery::raycast: Direction must be normalized");
	PX_CHECK_AND_RETURN(origin.isFinite(), "PxBatchQuery::raycast: origin is not valid");
	if (mNbRaycasts >= mDesc.queryMemory.getMaxRaycastsPerExecute())
	{
		PX_CHECK_AND_RETURN(mNbRaycasts < mDesc.queryMemory.getMaxRaycastsPerExecute(),
			"PxBatchQuery: number of raycast() calls exceeds PxBatchQueryMemory::raycastResultBufferSize, query discarded");
		return;
	}
	CHECK_RUNNING("PxBatchQuery::raycast: This batch is still executing, skipping query.");
	mNbRaycasts++;

	writeBatchHeader(BatchStreamHeader(hitFlags, cache, fd, userData, maxTouchHits, QTypeROS::eRAYCAST));
	writeQueryInput(mStream, MultiQueryInput(origin, unitDir, distance));

	Ps::atomicExchange(&mBatchQueryIsRunning, 0);
}

///////////////////////////////////////////////////////////////////////////////
void NpBatchQuery::overlap(
	const PxGeometry& geometry, const PxTransform& pose, PxU16 maxTouchHits,
	const PxQueryFilterData& fd, void* userData, const PxQueryCache* cache)
{
	PX_CHECK_AND_RETURN(pose.isValid(), "NpBatchQuery::overlapMultiple pose is not valid.");
	if (mNbOverlaps >= mDesc.queryMemory.getMaxOverlapsPerExecute())
	{
		PX_CHECK_AND_RETURN(mNbOverlaps < mDesc.queryMemory.getMaxOverlapsPerExecute(),
			"PxBatchQuery: number of overlap() calls exceeds PxBatchQueryMemory::overlapResultBufferSize, query discarded");
		return;
	}
	CHECK_RUNNING("PxBatchQuery::overlap: This batch is still executing, skipping query.")
	mNbOverlaps++;

	writeBatchHeader(BatchStreamHeader(PxHitFlags(), cache, fd, userData, maxTouchHits, QTypeROS::eOVERLAP));
	writeQueryInput(mStream, MultiQueryInput(&geometry, &pose));
	
	Ps::atomicExchange(&mBatchQueryIsRunning, 0);
}

void NpBatchQuery::sweep(
	const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance, PxU16 maxTouchHits,
	PxHitFlags hitFlags, const PxQueryFilterData& fd, void* userData, const PxQueryCache* cache, const PxReal inflation)
{
	PX_CHECK_AND_RETURN(pose.isValid(), "Batch sweep input check: pose is not valid.");
	PX_CHECK_AND_RETURN(unitDir.isFinite(), "Batch sweep input check: unitDir is not valid.");
	PX_CHECK_AND_RETURN(unitDir.isNormalized(), "Batch sweep input check: direction must be normalized");
	PX_CHECK_AND_RETURN(distance >= 0.0f, "Batch sweep input check: distance cannot be negative");
	PX_CHECK_AND_RETURN(distance != 0.0f || !(hitFlags & PxHitFlag::eASSUME_NO_INITIAL_OVERLAP),
		"Batch sweep input check: zero-length sweep only valid without the PxHitFlag::eASSUME_NO_INITIAL_OVERLAP flag");

	if (mNbSweeps >= mDesc.queryMemory.getMaxSweepsPerExecute())
	{
		PX_CHECK_AND_RETURN(mNbSweeps < mDesc.queryMemory.getMaxSweepsPerExecute(),
			"PxBatchQuery: number of sweep() calls exceeds PxBatchQueryMemory::sweepResultBufferSize, query discarded");
		return;
	}

	
	CHECK_RUNNING("PxBatchQuery::sweep: This batch is still executing, skipping query.")
	mNbSweeps++;

	writeBatchHeader(BatchStreamHeader(hitFlags, cache, fd, userData, maxTouchHits, QTypeROS::eSWEEP));

	//set the MTD flag
	mHasMtdSweep |= !!(hitFlags & PxHitFlag::eMTD);

	if((hitFlags & PxHitFlag::ePRECISE_SWEEP) && (hitFlags & PxHitFlag::eMTD))
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, " Precise sweep doesn't support MTD. Perform MTD with default sweep");
		hitFlags &= ~PxHitFlag::ePRECISE_SWEEP;
	}

	if((hitFlags & PxHitFlag::eASSUME_NO_INITIAL_OVERLAP) && (hitFlags & PxHitFlag::eMTD))
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, " eMTD cannot be used in conjunction with eASSUME_NO_INITIAL_OVERLAP. eASSUME_NO_INITIAL_OVERLAP will be ignored");
		hitFlags &= ~PxHitFlag::eASSUME_NO_INITIAL_OVERLAP;
	}

	PxReal realInflation = inflation;
	if((hitFlags & PxHitFlag::ePRECISE_SWEEP)&& inflation > 0.f)
	{
		realInflation = 0.f;
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, " Precise sweep doesn't support inflation, inflation will be overwritten to be zero");
	}
	
	writeQueryInput(mStream, MultiQueryInput(&geometry, &pose, unitDir, distance, realInflation));
	
	Ps::atomicExchange(&mBatchQueryIsRunning, 0);
}

void NpBatchQuery::release()
{
	if(Ps::atomicCompareExchange(&mBatchQueryIsRunning, 0, 0) != 0)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxBatchQuery::release: This batch is still executing, skipping release"); 
		return;
	}

	mNpScene->releaseBatchQuery(this);
}

PxBatchQueryPreFilterShader NpBatchQuery::getPreFilterShader() const
{
	return mDesc.preFilterShader;
}

PxBatchQueryPostFilterShader NpBatchQuery::getPostFilterShader() const
{
	return mDesc.postFilterShader;
}

const void* NpBatchQuery::getFilterShaderData() const
{
	return mDesc.filterShaderData;
}

PxU32 NpBatchQuery::getFilterShaderDataSize() const
{
	return mDesc.filterShaderDataSize;
}

PxClientID NpBatchQuery::getOwnerClient() const
{
	return mDesc.ownerClient;
}


