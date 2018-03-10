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


#include "PxsAABBManagerAux.h"
#include "PxsRigidBody.h"
#include "PxvGeometry.h"
#include "PsAtomic.h"
#include "GuConvexMeshData.h"
#include "GuTriangleMeshData.h"
#include "GuHeightFieldData.h"

#ifdef __SPU__
#include "CmPS3MemFetch.h"
#include "CellComputeAABBTask.h"
#endif

using namespace physx;

///////////////////////////////////////////////////////////////////////////////

PxBounds3 computeAABBNoCCD(const PxcAABBDataDynamic& aabbData, const PxsComputeAABBParams& /*params*/)
{
	const PxsShapeCore* PX_RESTRICT shapeCore = aabbData.mShapeCore;
	const PxBounds3* PX_RESTRICT localSpaceBounds = aabbData.mLocalSpaceAABB;

	// PT: sad but true: writing this with one line gets rid of an LHS
	PxBounds3 bounds;
	if(aabbData.mBodyAtom)
	{
		const PxsBodyCore* PX_RESTRICT bodyCore = static_cast<const PxsBodyCore*>(aabbData.mRigidCore);
		shapeCore->geometry.computeBounds(bounds, bodyCore->body2World * bodyCore->body2Actor.getInverse() *shapeCore->transform, shapeCore->contactOffset, localSpaceBounds);
	}
	else 
	{
		const PxsRigidCore* PX_RESTRICT rigidCore = aabbData.mRigidCore;
		shapeCore->geometry.computeBounds(bounds, rigidCore->body2World * shapeCore->transform, shapeCore->contactOffset, localSpaceBounds);
	}

	PX_ASSERT(bounds.minimum.x <= bounds.maximum.x
		&&	  bounds.minimum.y <= bounds.maximum.y
		&&	  bounds.minimum.z <= bounds.maximum.z);

	return bounds;
}

//Returns true if this is fast-moving
PxF32 computeSweptBounds(const PxcAABBDataDynamic& aabbData, PxBounds3& destBounds)
{
	const PxsShapeCore* PX_RESTRICT shapeCore = aabbData.mShapeCore;
	const PxsRigidBody* PX_RESTRICT bodyAtom = (PxsRigidBody*)aabbData.mBodyAtom;
	const PxsBodyCore* PX_RESTRICT bodyCore = static_cast<const PxsBodyCore*>(aabbData.mRigidCore);
	const PxBounds3* PX_RESTRICT localSpaceBounds = aabbData.mLocalSpaceAABB;

	//NEW: faster, more approximate mode: take world bounds at start and end of motion, and make a bound around both.
	//also, if we're not moving fast enough, non-swept bounds are returned.

	PX_ASSERT(bodyAtom);	//only call for dynamics!

	//Hmm, core and localSpaceBounds passed from spu because we 
	//can't patch up mBodyAtom's reference to PxsBodyCore on spu.
	//Need to handle the case on spu where core is null because we have  
	//a static or kinematic object.
	PX_ASSERT(bodyCore);

	PxTransform endShape2world = bodyCore->body2World * bodyCore->body2Actor.getInverse() * shapeCore->transform; 
	PxVec3 endOrigin, endExtent;
	PxReal ccdThreshold = shapeCore->geometry.computeBoundsWithCCDThreshold(endOrigin, endExtent, endShape2world, localSpaceBounds);//hopefully this does not do too much compute...this should really be cached!!

	PxBounds3 bounds = PxBounds3::centerExtents(endOrigin, endExtent);

	//KS - Always inflate bounds here
	PxTransform startShape2World = bodyAtom->getLastCCDTransform() * bodyCore->body2Actor.getInverse() * shapeCore->transform;	//build shape 2 world

	PxBounds3 startBounds;
	shapeCore->geometry.computeBounds(startBounds, startShape2World, 0.0f, localSpaceBounds);//hopefully this does not do too much compute...this should really be cached!!

	bounds.include(startBounds);

	//AM: we fatten by the contact offset here to be able to do dist based CG.
	const PxVec3 offset(shapeCore->contactOffset);

	destBounds.minimum = bounds.minimum - offset;
	destBounds.maximum = bounds.maximum + offset;

	PX_ASSERT(destBounds.minimum.x <= destBounds.maximum.x
		&&	  destBounds.minimum.y <= destBounds.maximum.y
		&&	  destBounds.minimum.z <= destBounds.maximum.z);

	const PxVec3 startOrigin = startBounds.getCenter();
	const PxVec3 trans = startOrigin - endOrigin;

	return physx::intrinsics::fsel(trans.magnitudeSquared() - (ccdThreshold * ccdThreshold),  1.0f, 0.0f);
	//return (trans.magnitudeSquared() >= (ccdThreshold * ccdThreshold) ? 1 : 0);
}

void computeSweptBounds(const PxcRigidBody* bodyAtom, const PxsRigidCore* rigidCore, const PxBounds3& localSpaceActorBounds, PxBounds3& destBounds)	//this already includes the fat.  Note: if we do this when global second bp is off, then the bounds will be trailing at the end of the frame because we move out of them at integration, but this is OK.
{
	PX_ASSERT(rigidCore);
	PX_ASSERT(bodyAtom);

	//Take world bounds at start and end of motion, and make a bound around both.
	//Be conservative

	//Prev bounds.
	destBounds = PxBounds3::transformFast(bodyAtom->mLastTransform, localSpaceActorBounds);

	//Curr bounds.
	const PxBounds3 currBounds = PxBounds3::transformFast(rigidCore->body2World, localSpaceActorBounds);

	//Union of prev and curr bounds.
	destBounds.include(currBounds);

	PX_ASSERT(destBounds.minimum.x <= destBounds.maximum.x
		&&	  destBounds.minimum.y <= destBounds.maximum.y
		&&	  destBounds.minimum.z <= destBounds.maximum.z);
}

void physx::PxsComputeAABB
(const PxcAABBDataStatic& aabbData, PxBounds3& updatedBodyShapeBounds)
{
	PX_ASSERT(aabbData.mShapeCore);
	PX_ASSERT(aabbData.mRigidCore);
	const PxsShapeCore* PX_RESTRICT shapeCore = aabbData.mShapeCore;
	const PxsRigidCore* PX_RESTRICT rigidCore = aabbData.mRigidCore;

#ifndef __SPU__

	PX_ALIGN(16, PxTransform globalPose);

	const Vec3V body2WorldPos = V3LoadU(rigidCore->body2World.p);
	const QuatV body2WorldRot = QuatVLoadU(&rigidCore->body2World.q.x);

	const Vec3V body2ActorPos = V3LoadU(shapeCore->transform.p);
	const QuatV body2ActorRot = QuatVLoadU(&shapeCore->transform.q.x);

	const Vec3V trnsl0 = QuatRotate(body2WorldRot,body2ActorPos);
	const Vec3V trnsl1 = V3Add(trnsl0,body2WorldPos);
	const QuatV rot1 = QuatMul(body2WorldRot,body2ActorRot);

	V3StoreA(trnsl1,globalPose.p);
	V4StoreA(rot1,&globalPose.q.x);

	shapeCore->geometry.computeBounds(updatedBodyShapeBounds, globalPose, shapeCore->contactOffset, NULL);

#else

	shapeCore->geometry.computeBounds(updatedBodyShapeBounds, rigidCore->body2World * shapeCore->transform, shapeCore->contactOffset, NULL);

#endif

	PX_ASSERT(updatedBodyShapeBounds.minimum.x <= updatedBodyShapeBounds.maximum.x
	 	 &&	  updatedBodyShapeBounds.minimum.y <= updatedBodyShapeBounds.maximum.y
		 &&	  updatedBodyShapeBounds.minimum.z <= updatedBodyShapeBounds.maximum.z);
}

PxF32 physx::PxsComputeAABB
(const bool secondBroadphase, const PxcAABBDataDynamic& aabbData, 
 PxBounds3& updatedBodyShapeBounds)
{
	PX_ASSERT(aabbData.mShapeCore);
	PX_ASSERT(aabbData.mRigidCore);
	PX_ASSERT(aabbData.mBodyAtom);

	const PxsShapeCore* PX_RESTRICT shapeCore = aabbData.mShapeCore;
	const PxsRigidCore* PX_RESTRICT rigidCore = aabbData.mRigidCore;
	const PxBounds3* PX_RESTRICT localSpaceBounds = aabbData.mLocalSpaceAABB;

	if(!secondBroadphase || 0==rigidCore->hasCCD())				
	{
		const PxsBodyCore* PX_RESTRICT bodyCore = static_cast<const PxsBodyCore*>(rigidCore);

		//bodyCore->body2World * bodyCore->body2Actor.getInverse() *shapeCore->transform

		PX_ALIGN(16, PxTransform globalPose);
		const PxVec3 negBody2Actor = -bodyCore->body2Actor.p;

		const Vec3V body2WorldPos = V3LoadU(bodyCore->body2World.p);
		const QuatV body2WorldRot = QuatVLoadU(&bodyCore->body2World.q.x);

		const Vec3V body2ActorPos = V3LoadU(negBody2Actor);
		const QuatV body2ActorRot = QuatVLoadU(&bodyCore->body2Actor.q.x);		

		const Vec3V body2ActorPosInv = QuatRotateInv(body2ActorRot,body2ActorPos);
		const QuatV body2ActorRotInv = QuatConjugate(body2ActorRot);

		const Vec3V shape2ActorPos = V3LoadU(shapeCore->transform.p);
		const QuatV shape2ActorRot = QuatVLoadU(&shapeCore->transform.q.x);

		const Vec3V trnsl0 = QuatRotate(body2WorldRot,body2ActorPosInv);
		const Vec3V trnsl1 = V3Add(trnsl0,body2WorldPos);
		const QuatV rot1 = QuatMul(body2WorldRot,body2ActorRotInv);

		const Vec3V trnsl2 = QuatRotate(rot1,shape2ActorPos);
		const Vec3V trnsl3 = V3Add(trnsl2,trnsl1);
		const QuatV rot3 = QuatMul(rot1,shape2ActorRot);

		V3StoreA(trnsl3,globalPose.p);
		V4StoreA(rot3,&globalPose.q.x);		

		shapeCore->geometry.computeBounds(updatedBodyShapeBounds, globalPose, shapeCore->contactOffset, localSpaceBounds);

		PX_ASSERT(updatedBodyShapeBounds.minimum.x <= updatedBodyShapeBounds.maximum.x
			&&	  updatedBodyShapeBounds.minimum.y <= updatedBodyShapeBounds.maximum.y
			&&	  updatedBodyShapeBounds.minimum.z <= updatedBodyShapeBounds.maximum.z);

		return 0.0f;
	}
	else
	{
		return computeSweptBounds(aabbData, updatedBodyShapeBounds);	//this already includes the fat.  Note: if we do this when global second bp is off, then the bounds will be trailing at the end of the frame because we move out of them at integration, but this is OK.
	}
}

void physx::PxsComputeAABBLocalSpace(const PxcAABBDataStatic& aabbData, PxBounds3& updatedBodyShapeBounds)
{
	PX_ASSERT(aabbData.mShapeCore);
	const PxsShapeCore* PX_RESTRICT shapeCore = aabbData.mShapeCore;
	const PxTransform& localPose = shapeCore->transform;
	const PxReal contactOffset = shapeCore->contactOffset;
	shapeCore->geometry.computeBounds(updatedBodyShapeBounds, localPose, contactOffset, NULL);

	PX_ASSERT(updatedBodyShapeBounds.minimum.x <= updatedBodyShapeBounds.maximum.x
		&&	  updatedBodyShapeBounds.minimum.y <= updatedBodyShapeBounds.maximum.y
		&&	  updatedBodyShapeBounds.minimum.z <= updatedBodyShapeBounds.maximum.z);
}

void physx::PxsComputeAABBLocalSpace(const PxcAABBDataDynamic& aabbData, PxBounds3& updatedBodyShapeBounds)
{
	PX_ASSERT(aabbData.mShapeCore);

	const PxsShapeCore* PX_RESTRICT shapeCore = aabbData.mShapeCore;
	const PxsRigidCore* PX_RESTRICT rigidCore = aabbData.mRigidCore;
	const PxBounds3* PX_RESTRICT localSpaceBounds = aabbData.mLocalSpaceAABB;
	PX_ASSERT(aabbData.mBodyAtom);

	const PxsBodyCore* PX_RESTRICT bodyCore = static_cast<const PxsBodyCore*>(rigidCore);

	PX_ALIGN(16, PxTransform globalPose);
	const PxVec3 negBody2Actor = -bodyCore->body2Actor.p;

	const Vec3V body2ActorPos = V3LoadU(negBody2Actor);
	const QuatV body2ActorRot = QuatVLoadU(&bodyCore->body2Actor.q.x);		

	const Vec3V body2ActorPosInv = QuatRotateInv(body2ActorRot,body2ActorPos);
	const QuatV body2ActorRotInv = QuatConjugate(body2ActorRot);

	const Vec3V shape2ActorPos = V3LoadU(shapeCore->transform.p);
	const QuatV shape2ActorRot = QuatVLoadU(&shapeCore->transform.q.x);

	const Vec3V trnsl2 = QuatRotate(body2ActorRotInv,shape2ActorPos);
	const Vec3V trnsl3 = V3Add(trnsl2,body2ActorPosInv);
	const QuatV rot3 = QuatMul(body2ActorRotInv,shape2ActorRot);

	V3StoreA(trnsl3,globalPose.p);
	V4StoreA(rot3,&globalPose.q.x);		

	shapeCore->geometry.computeBounds(updatedBodyShapeBounds, globalPose, shapeCore->contactOffset, localSpaceBounds);

	PX_ASSERT(updatedBodyShapeBounds.minimum.x <= updatedBodyShapeBounds.maximum.x
		&&	  updatedBodyShapeBounds.minimum.y <= updatedBodyShapeBounds.maximum.y
		&&	  updatedBodyShapeBounds.minimum.z <= updatedBodyShapeBounds.maximum.z);
}


void physx::PxsComputeAABB
(const PxsRigidCore* rigidCore, const PxBounds3& localSpaceActorBounds, 
 PxBounds3& updatedWorldSpaceActorBounds)
{
	PX_ASSERT(rigidCore);

	updatedWorldSpaceActorBounds = PxBounds3::transformFast(rigidCore->body2World, localSpaceActorBounds);

	PX_ASSERT(updatedWorldSpaceActorBounds.minimum.x <= updatedWorldSpaceActorBounds.maximum.x
		&&	  updatedWorldSpaceActorBounds.minimum.y <= updatedWorldSpaceActorBounds.maximum.y
		&&	  updatedWorldSpaceActorBounds.minimum.z <= updatedWorldSpaceActorBounds.maximum.z);
}

PxF32 physx::PxsComputeAABB
(const bool secondBroadphase, 
 const PxcRigidBody* bodyAtom, const PxsRigidCore* rigidCore, const PxBounds3& localSpaceActorBounds, 
 PxBounds3& updatedWorldSpaceActorBounds)
{
	PX_ASSERT(bodyAtom);
	PX_ASSERT(rigidCore);

	if(!secondBroadphase || 0==rigidCore->hasCCD())				
	{
		updatedWorldSpaceActorBounds = PxBounds3::transformFast(rigidCore->body2World, localSpaceActorBounds);

		PX_ASSERT(updatedWorldSpaceActorBounds.minimum.x <= updatedWorldSpaceActorBounds.maximum.x
			&&	  updatedWorldSpaceActorBounds.minimum.y <= updatedWorldSpaceActorBounds.maximum.y
			&&	  updatedWorldSpaceActorBounds.minimum.z <= updatedWorldSpaceActorBounds.maximum.z);

		return 0.0f;
	}
	else
	{
		computeSweptBounds(bodyAtom, rigidCore, localSpaceActorBounds, updatedWorldSpaceActorBounds);	//this already includes the fat.  Note: if we do this when global second bp is off, then the bounds will be trailing at the end of the frame because we move out of them at integration, but this is OK.

		PX_ASSERT(updatedWorldSpaceActorBounds.minimum.x <= updatedWorldSpaceActorBounds.maximum.x
			&&	  updatedWorldSpaceActorBounds.minimum.y <= updatedWorldSpaceActorBounds.maximum.y
			&&	  updatedWorldSpaceActorBounds.minimum.z <= updatedWorldSpaceActorBounds.maximum.z);

		return 1.0f;
	}
}



#ifdef __SPU__

struct EAAABBDataStatic
{
	PxU8 mShapeCore[sizeof(PxsShapeCore)];
	PxU8 mBodyCore[sizeof(PxsBodyCore) + 48];
	Cm::MemFetchGatherList<3> mList;
	PxcAABBDataStatic mAABBData;
};

struct EAAABBDataDynamic
{
	PxU8 mShapeCore[sizeof(PxsShapeCore)];
	PxU8 mRigidBodyPlusBodyCorePlusLocalSpaceAABB[sizeof(PxcRigidBody) + sizeof(PxsBodyCore) + 48];
	Cm::MemFetchGatherList<5> mList;
	PxcAABBDataDynamic mAABBData;
};


void initEAABBBData(EAAABBDataStatic& eaAABBData)
{
	eaAABBData.mList.init();

	eaAABBData.mList.setSize(0, sizeof(PxsShapeCore));

	eaAABBData.mAABBData.mShapeCore=(PxsShapeCore*)eaAABBData.mShapeCore;
}

void initEAABBBData(EAAABBDataDynamic& eaAABBData)
{
	eaAABBData.mList.init();

	eaAABBData.mList.setSize(0, sizeof(PxsShapeCore));

	eaAABBData.mAABBData.mShapeCore=(PxsShapeCore*)eaAABBData.mShapeCore;
}

PX_FORCE_INLINE void prefetchAsync(const PxcAABBDataStatic*& aabbData, EAAABBDataStatic* eaAABBData)
{
	PX_ASSERT(aabbData->mShapeCore);
	eaAABBData->mList.setEA(0, (uintptr_t)aabbData->mShapeCore);
	PX_ASSERT(eaAABBData->mList.getSize(0) == sizeof(PxsShapeCore));
	PX_ASSERT(eaAABBData->mAABBData.mShapeCore==(PxsShapeCore*)eaAABBData->mShapeCore);

	eaAABBData->mAABBData.mRigidCore=NULL;
	PxU32 listTide=1;
	PxU32 dataTide=0;

	eaAABBData->mList.setEA(1, (uintptr_t)aabbData->mRigidCore);
	eaAABBData->mList.setSize(1, sizeof(PxsRigidCore));
	eaAABBData->mAABBData.mRigidCore=(PxsRigidCore*)(eaAABBData->mBodyCore + 0);
	listTide=2;
	dataTide=sizeof(PxsRigidCore);

	Cm::memFetchGatherListAsync(Cm::MemFetchPtr(eaAABBData->mShapeCore), eaAABBData->mList, listTide, 1);

	aabbData=&eaAABBData->mAABBData;
}

PX_FORCE_INLINE void prefetchAsync(const PxcAABBDataDynamic*& aabbData, EAAABBDataDynamic* eaAABBData)
{
	PX_ASSERT(aabbData->mShapeCore);
	eaAABBData->mList.setEA(0, (uintptr_t)aabbData->mShapeCore);
	PX_ASSERT(eaAABBData->mList.getSize(0) == sizeof(PxsShapeCore));
	PX_ASSERT(eaAABBData->mAABBData.mShapeCore==(PxsShapeCore*)eaAABBData->mShapeCore);

	eaAABBData->mAABBData.mBodyAtom=NULL;
	eaAABBData->mAABBData.mRigidCore=NULL;
	eaAABBData->mAABBData.mLocalSpaceAABB=NULL;
	PxU32 listTide=1;
	PxU32 dataTide=0;

	if(aabbData->mBodyAtom)
	{
		PX_ASSERT(aabbData->mRigidCore);

		eaAABBData->mList.setEA(1, (uintptr_t)aabbData->mBodyAtom);
		eaAABBData->mList.setSize(1, sizeof(PxcRigidBody));
		eaAABBData->mAABBData.mBodyAtom=(PxcRigidBody*)(eaAABBData->mRigidBodyPlusBodyCorePlusLocalSpaceAABB + 0);

		eaAABBData->mList.setEA(2, (uintptr_t)aabbData->mRigidCore);
		eaAABBData->mList.setSize(2, sizeof(PxsBodyCore));
		eaAABBData->mAABBData.mRigidCore=(PxsBodyCore*)(eaAABBData->mRigidBodyPlusBodyCorePlusLocalSpaceAABB + sizeof(PxcRigidBody));

		listTide=3;
		dataTide=sizeof(PxcRigidBody) + sizeof(PxsBodyCore);
	}
	else
	{
		eaAABBData->mList.setEA(1, (uintptr_t)aabbData->mRigidCore);
		eaAABBData->mList.setSize(1, sizeof(PxsRigidCore));
		eaAABBData->mAABBData.mRigidCore=(PxsRigidCore*)(eaAABBData->mRigidBodyPlusBodyCorePlusLocalSpaceAABB + 0);
		listTide=2;
		dataTide=sizeof(PxsRigidCore);
	}

	if(aabbData->mLocalSpaceAABB)
	{
		//ea might not be 16-byte aligned. work out the 16-byte aligned address.
		const uintptr_t ea = (uintptr_t)aabbData->mLocalSpaceAABB;
		const uintptr_t ea16 = (ea & ~0x0f);
		eaAABBData->mList.setEA(listTide, ea16);
		eaAABBData->mList.setSize(listTide, 48);
		eaAABBData->mAABBData.mLocalSpaceAABB=(PxBounds3*)(eaAABBData->mRigidBodyPlusBodyCorePlusLocalSpaceAABB + dataTide + ea - ea16);
		listTide++;
		dataTide+=48;
	}

	Cm::memFetchGatherListAsync(Cm::MemFetchPtr(eaAABBData->mShapeCore), eaAABBData->mList, listTide, 1);

	aabbData=&eaAABBData->mAABBData;
}


PX_FORCE_INLINE void wait()
{
	Cm::memFetchWait(1);
}

#else

PX_FORCE_INLINE void prefetchAsync(const PxcAABBDataDynamic* PX_RESTRICT aabbData)
{
	PX_ASSERT(aabbData->mShapeCore);

	Ps::prefetchLine(aabbData->mShapeCore);
	Ps::prefetchLine(aabbData->mBodyAtom);
	Ps::prefetchLine(aabbData->mRigidCore);
	Ps::prefetchLine(aabbData->mLocalSpaceAABB);
}

PX_FORCE_INLINE void prefetchAsync(const PxcAABBDataStatic* PX_RESTRICT aabbData)
{
	PX_ASSERT(aabbData->mShapeCore);
	Ps::prefetchLine(aabbData->mShapeCore);
	Ps::prefetchLine(aabbData->mRigidCore);
}


PX_FORCE_INLINE void wait()
{
}

#endif

PxU32 physx::updateBodyShapeAABBs
(const PxcBpHandle* PX_RESTRICT updatedAABBHandles, const PxU32 numUPdatedAABBHandles, 
 const PxcBpHandle* PX_RESTRICT aabbDataHandles, const PxcAABBDataDynamic* PX_RESTRICT aabbData,
 const bool secondBroadPhase,
 IntegerAABB* bounds, const PxU32 /*maxNumBounds*/)
{
	PX_ASSERT(updatedAABBHandles);
	PX_ASSERT(numUPdatedAABBHandles>0);

#ifdef __SPU__
	EAAABBDataDynamic PX_ALIGN(16, eaaabbData0);
	EAAABBDataDynamic PX_ALIGN(16, eaaabbData1);
	EAAABBDataDynamic* currEaAABBData=&eaaabbData0;
	EAAABBDataDynamic* nextEaAABBData=&eaaabbData1;
	initEAABBBData(*currEaAABBData);
	initEAABBBData(*nextEaAABBData);
#endif //__SPU__

	//Prefetch the very first aabb data.
	const PxU32 nextHandle0=updatedAABBHandles[0];
	const PxcAABBDataDynamic* nextAABBData = &aabbData[aabbDataHandles[nextHandle0]];
#ifdef __SPU__
	prefetchAsync(nextAABBData, nextEaAABBData);
#else
	prefetchAsync(nextAABBData);
#endif

	//Update aabbs in blocks of four where we can update the aabb and prefetch the data for the next one.
	const PxU32 num = (numUPdatedAABBHandles & 3) ? numUPdatedAABBHandles & ~3 : numUPdatedAABBHandles-4;

	PxF32 numFastMovingObjects = 0;
	for(PxU32 i=0;i<num;i+=4)
	{
		//Hoping the compiler will unroll the loop here to save me copying the same code out 4 times.
		for(PxU32 j=0;j<4;j++)
		{
			const PxcAABBDataDynamic* PX_RESTRICT currAABBData=nextAABBData;

			PX_ASSERT((i+j+1) < numUPdatedAABBHandles);
			const PxU32 nextHandle=updatedAABBHandles[i+j+1];
			nextAABBData = &aabbData[aabbDataHandles[nextHandle]];
			wait();
#ifdef __SPU__
			EAAABBDataDynamic* tmp=currEaAABBData;
			currEaAABBData=nextEaAABBData;
			nextEaAABBData=tmp;
			prefetchAsync(nextAABBData, nextEaAABBData);
#else
			prefetchAsync(nextAABBData);
#endif
			PxBounds3 updatedBodyShapeBounds;
			numFastMovingObjects+=PxsComputeAABB(secondBroadPhase, *currAABBData, updatedBodyShapeBounds);
			bounds[updatedAABBHandles[i+j]].encode(updatedBodyShapeBounds);
		}
	}

	//Remaining updateAABB + prefetch of next.
	for(PxU32 i=num;i<(numUPdatedAABBHandles-1);i++)
	{ 
		const PxcAABBDataDynamic* PX_RESTRICT currAABBData=nextAABBData;

		PX_ASSERT((i+1) < numUPdatedAABBHandles);
		const PxU32 nextHandle=updatedAABBHandles[i+1];
		nextAABBData = &aabbData[aabbDataHandles[nextHandle]];
		wait();
#ifdef __SPU__
		EAAABBDataDynamic* tmp=currEaAABBData;
		currEaAABBData=nextEaAABBData;
		nextEaAABBData=tmp;
		prefetchAsync(nextAABBData, nextEaAABBData);
#else
		prefetchAsync(nextAABBData);
#endif
		PxBounds3 updatedBodyShapeBounds;
		numFastMovingObjects+=PxsComputeAABB(secondBroadPhase, *currAABBData, updatedBodyShapeBounds);
		bounds[updatedAABBHandles[i]].encode(updatedBodyShapeBounds);
	}

	//Very last update aabb (no prefetch obviously).
	const PxcAABBDataDynamic* PX_RESTRICT currAABBData=nextAABBData;
	PxBounds3 updatedBodyShapeBounds;
	wait();
	numFastMovingObjects+=PxsComputeAABB(secondBroadPhase, *currAABBData, updatedBodyShapeBounds);
	bounds[updatedAABBHandles[numUPdatedAABBHandles-1]].encode(updatedBodyShapeBounds);

	//OK, write this value back to the params
	const PxU32 numFastMovingObjectsU32=(PxU32)numFastMovingObjects;
	return numFastMovingObjectsU32;
}


void physx::updateBodyShapeAABBs
(const PxcBpHandle* PX_RESTRICT updatedAABBHandles, const PxU32 numUPdatedAABBHandles, 
 const PxcBpHandle* PX_RESTRICT aabbDataHandles, const PxcAABBDataStatic* PX_RESTRICT aabbData,
 IntegerAABB* bounds, const PxU32 /*maxNumBounds*/)
{
	PX_ASSERT(updatedAABBHandles);
	PX_ASSERT(numUPdatedAABBHandles>0);

#ifdef __SPU__
	EAAABBDataStatic PX_ALIGN(16, eaaabbData0);
	EAAABBDataStatic PX_ALIGN(16, eaaabbData1);
	EAAABBDataStatic* currEaAABBData=&eaaabbData0;
	EAAABBDataStatic* nextEaAABBData=&eaaabbData1;
	initEAABBBData(*currEaAABBData);
	initEAABBBData(*nextEaAABBData);
#endif //__SPU__

	//Prefetch the very first aabb data.
	const PxU32 nextHandle0=updatedAABBHandles[0];
	const PxcAABBDataStatic* nextAABBData = &aabbData[aabbDataHandles[nextHandle0]];
#ifdef __SPU__
	prefetchAsync(nextAABBData, nextEaAABBData);
#else
	prefetchAsync(nextAABBData);
#endif

	//Update aabbs in blocks of four where we can update the aabb and prefetch the data for the next one.
	const PxU32 num = (numUPdatedAABBHandles & 3) ? numUPdatedAABBHandles & ~3 : numUPdatedAABBHandles-4;

	for(PxU32 i=0;i<num;i+=4)
	{
		//Hoping the compiler will unroll the loop here to save me copying the same code out 4 times.
		for(PxU32 j=0;j<4;j++)
		{
			const PxcAABBDataStatic* PX_RESTRICT currAABBData=nextAABBData;

			PX_ASSERT((i+j+1) < numUPdatedAABBHandles);
			const PxU32 nextHandle=updatedAABBHandles[i+j+1];
			nextAABBData = &aabbData[aabbDataHandles[nextHandle]];
			wait();
#ifdef __SPU__
			EAAABBDataStatic* tmp=currEaAABBData;
			currEaAABBData=nextEaAABBData;
			nextEaAABBData=tmp;
			prefetchAsync(nextAABBData, nextEaAABBData);
#else
			prefetchAsync(nextAABBData);
#endif
			PxBounds3 updatedBodyShapeBounds;
			PxsComputeAABB(*currAABBData, updatedBodyShapeBounds);
			bounds[updatedAABBHandles[i+j]].encode(updatedBodyShapeBounds);
		}
	}

	//Remaining updateAABB + prefetch of next.
	for(PxU32 i=num;i<(numUPdatedAABBHandles-1);i++)
	{ 
		const PxcAABBDataStatic* PX_RESTRICT currAABBData=nextAABBData;

		PX_ASSERT((i+1) < numUPdatedAABBHandles);
		const PxU32 nextHandle=updatedAABBHandles[i+1];
		nextAABBData = &aabbData[aabbDataHandles[nextHandle]];
		wait();
#ifdef __SPU__
		EAAABBDataStatic* tmp=currEaAABBData;
		currEaAABBData=nextEaAABBData;
		nextEaAABBData=tmp;
		prefetchAsync(nextAABBData, nextEaAABBData);
#else
		prefetchAsync(nextAABBData);
#endif
		PxBounds3 updatedBodyShapeBounds;
		PxsComputeAABB(*currAABBData, updatedBodyShapeBounds);
		bounds[updatedAABBHandles[i]].encode(updatedBodyShapeBounds);
	}

	//Very last update aabb (no prefetch obviously).
	const PxcAABBDataStatic* PX_RESTRICT currAABBData=nextAABBData;
	PxBounds3 updatedBodyShapeBounds;
	wait();
	PxsComputeAABB(*currAABBData, updatedBodyShapeBounds);
	bounds[updatedAABBHandles[numUPdatedAABBHandles-1]].encode(updatedBodyShapeBounds);
}


