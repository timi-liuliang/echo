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

#include "PxMemory.h"
#include "SqBucketPruner.h"
#include "GuIntersectionBoxBox.h"
#include "GuGeomUtilsInternal.h"
#include "PsVecMath.h"

using namespace physx::shdfnd::aos;

using namespace physx;
using namespace Sq;
using namespace Gu;
using namespace Ps;

#define INVALID_HANDLE	0xffffffff

/*
TODO:
- if Core is always available, mSortedObjects could be replaced with just indices to mCoreObjects => less memory.
- UTS:
	- test that queries against empty boxes all return false
- invalidate after 16 removes
- check shiftOrigin stuff (esp what happens to emptied boxes)
	- isn't there a very hard-to-find bug waiting to happen in there,
	when the shift touches the empty box and overrides mdata0/mdata1 with "wrong" values that break the sort?
- revisit updateObject/removeObject
- optimize/cache computation of free global bounds before clipRay

- remove temp memory buffers (sorted arrays)
- take care of code duplication
- better code to generate SIMD 0x7fffffff
- refactor SIMD tests
- optimize:
	- better split values
	- optimize update (bitmap, less data copy, etc)
	- use ray limits in traversal code too?
	- the SIMD XBOX code operates on Min/Max rather than C/E. Change format?
	- or just try the alternative ray-box code (as on PC) ==> pretty much exactly the same speed
*/

//#define VERIFY_SORT
//#define BRUTE_FORCE_LIMIT	32
#define LOCAL_SIZE	256			// Size of various local arrays. Dynamic allocations occur if exceeded.
#define USE_SIMD				// Use SIMD code or not (sanity performance check)
#define NODE_SORT				// Enable/disable node sorting
#define NODE_SORT_MIN_COUNT	16	// Limit above which node sorting is performed
#ifdef PX_X360
	#define DELAYED_WRITES			// Less VCMPs, but delays ray shrinking. This flag enables those inside buckets.
	//#define DELAYED_WRITES_BLOCK	// Delayed writes in traversal code. Removes VCMPs from top issues but slower overall because of delayed ray shrinking.
#endif

#define ALIGN16(size) (((unsigned)(size)+15)&((unsigned)~15))

#ifdef _DEBUG
	#define AlignedLoad		V4LoadU
	#define AlignedStore	V4StoreU
#else
	#define AlignedLoad		V4LoadA
	#define AlignedStore	V4StoreA
#endif

#ifdef __SPU__
#include "CmPS3MemFetch.h"

//static BucketBox CELL_ALIGN(128, gCurrentBox);
//static Prunable* CELL_ALIGN(128, gCurrentObject);

enum DMAIndex
{
	DMA_INPUTDATA_GET = 1,
};

#define BOX_CACHE_SIZE	128
static PxU32 gBaseIndex_CachedBoxes = 0xffffffff;
static BucketBox CELL_ALIGN(128, gCachedBoxes[BOX_CACHE_SIZE]);
static PrunerPayload CELL_ALIGN(128, gCachedObjects[BOX_CACHE_SIZE]);

static PX_FORCE_INLINE const BucketBox& fetchCachedData(PrunerPayload*& object,
														const BucketBox* PX_RESTRICT ppuStartAddress,
														const BucketBox* PX_RESTRICT ppuBoxAddress,
														PxU32 totalAllocated,
														PrunerPayload* PX_RESTRICT ppuStartAddressObjects)
{
	const PxU32 index = ppuBoxAddress - ppuStartAddress;
//pxPrintf("index: (%d)\n", index);

	const PxU32 requiredBase = index/BOX_CACHE_SIZE;
	if(PX_EXPECT_FALSE(requiredBase!=gBaseIndex_CachedBoxes))
	{
		gBaseIndex_CachedBoxes = requiredBase;

		if(0)
		{
			// This version just loads BOX_CACHE_SIZE entries all the time, and thus reads past the end of PPU buffers

			Cm::memFetchAlignedAsync(Cm::MemFetchPtr(gCachedBoxes), PxU32(ppuStartAddress + requiredBase*BOX_CACHE_SIZE), sizeof(BucketBox)*BOX_CACHE_SIZE, DMA_INPUTDATA_GET);
			Cm::memFetchAlignedAsync(Cm::MemFetchPtr(gCachedObjects), PxU32(ppuStartAddressObjects + requiredBase*BOX_CACHE_SIZE), sizeof(PrunerPayload)*BOX_CACHE_SIZE, DMA_INPUTDATA_GET+1);
		}
		else
		{
			const PxU32 N = totalAllocated - requiredBase;
			const PxU32 B = BOX_CACHE_SIZE;
			const PxU32 M = PxMin(N, B);
			const PxU32 bytesToLoadForBoxes = CELL_ALIGN_SIZE_16(M*sizeof(BucketBox));
			const PxU32 bytesToLoadForObjects = CELL_ALIGN_SIZE_16(M*sizeof(PrunerPayload));
	//pxPrintf("DMA START: (%d) (%d) (%d)\n", requiredBase, PxU32(ppuStartAddress + requiredBase*BOX_CACHE_SIZE)&15, bytesToLoadForBoxes&15);
	//pxPrintf("DMA START: (%d) (%d) (%d)\n", requiredBase, PxU32(ppuStartAddressObjects + requiredBase*BOX_CACHE_SIZE)&15, bytesToLoadForObjects&15);
	//pxPrintf("ppuStartAddressObjects: (%x)\n", PxU32(ppuStartAddressObjects));
			Cm::memFetchAlignedAsync(Cm::MemFetchPtr(gCachedBoxes), PxU32(ppuStartAddress + requiredBase*BOX_CACHE_SIZE), bytesToLoadForBoxes, DMA_INPUTDATA_GET);
			Cm::memFetchAlignedAsync(Cm::MemFetchPtr(gCachedObjects), PxU32(ppuStartAddressObjects + requiredBase*BOX_CACHE_SIZE), bytesToLoadForObjects, DMA_INPUTDATA_GET+1);
		}

		Cm::memFetchWaitMask(CELL_BIT(DMA_INPUTDATA_GET));
 		Cm::memFetchWaitMask(CELL_BIT(DMA_INPUTDATA_GET+1));
//pxPrintf("DMA END\n");
 	}
	const PxU32 cacheIndex = index & (BOX_CACHE_SIZE-1);
	object = &gCachedObjects[cacheIndex];
	return gCachedBoxes[cacheIndex];
}
#endif

// SAT-based ray-box overlap test has accuracy issues for long rays, so we clip them against the global AABB to limit these issues.
static void clipRay(const PxVec3& rayOrig, const PxVec3& rayDir, float& maxDist, const PxVec3& boxMin, const PxVec3& boxMax)
{
	const PxVec3 boxCenter = (boxMax + boxMin)*0.5f;
	const PxVec3 boxExtents = (boxMax - boxMin)*0.5f;
	const float dpc = boxCenter.dot(rayDir);
	const float extentsMagnitude = boxExtents.magnitude();
	const float dpMin = dpc - extentsMagnitude;
	const float dpMax = dpc + extentsMagnitude;
	const float dpO = rayOrig.dot(rayDir);
	const float boxLength = extentsMagnitude * 2.0f;
	const float distToBox = PxMin(PxAbs(dpMin - dpO), PxAbs(dpMax - dpO));
	maxDist = distToBox + boxLength * 2.0f;
}

BucketPrunerNode::BucketPrunerNode()
{
	for(PxU32 i=0;i<5;i++)
		mBucketBox[i].setEmpty();
}

static const PxU8 gCodes[] = {	4, 4, 4, 4, 4, 3, 2, 2,
								4, 1, 0, 0, 4, 1, 0, 0,
								4, 1, 0, 0, 2, 1, 0, 0,
								3, 1, 0, 0, 2, 1, 0, 0};

#ifdef PX_WINDOWS
static PX_FORCE_INLINE PxU32 classifyBox_x86(const BucketBox& box, const PxVec4& limits, const bool useY, const bool isCrossBucket)
{
	const Vec4V extents = AlignedLoad(&box.mExtents.x);
	const Vec4V center = AlignedLoad(&box.mCenter.x);
	const Vec4V plus = V4Add(extents, center);
	const Vec4V minus = V4Sub(extents, center);

	Vec4V tmp;
	if(useY)	// PT: this is a constant so branch prediction works here
		tmp = _mm_shuffle_ps(plus, minus, _MM_SHUFFLE(0,1,0,1));
	else
		tmp = _mm_shuffle_ps(plus, minus, _MM_SHUFFLE(0,2,0,2));

	const Vec4V comp = _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(0,2,1,3)); // oh well, nm

	const PxU32 Code = (PxU32)_mm_movemask_ps(V4IsGrtr(V4LoadA(&limits.x), comp));
	return gCodes[Code | PxU32(isCrossBucket)<<4];
}

static PX_FORCE_INLINE PxU32 classifyBox_x86(const Vec4V boxMin, const Vec4V boxMax, const PxVec4& limits, const bool useY, const bool isCrossBucket)
{
	const Vec4V plus = boxMax;
	const Vec4V minus = V4Neg(boxMin);

	Vec4V tmp;
	if(useY)	// PT: this is a constant so branch prediction works here
		tmp = _mm_shuffle_ps(plus, minus, _MM_SHUFFLE(0,1,0,1));
	else
		tmp = _mm_shuffle_ps(plus, minus, _MM_SHUFFLE(0,2,0,2));

	const Vec4V comp = _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(0,2,1,3)); // oh well, nm

	const PxU32 Code = (PxU32)_mm_movemask_ps(V4IsGrtr(V4LoadA(&limits.x), comp));
	return gCodes[Code | PxU32(isCrossBucket)<<4];
}
#endif

static PX_FORCE_INLINE PxU32 classifyBox(const BucketBox& box, const float limitX, const float limitYZ, const PxU32 yz, const bool isCrossBucket)
{
	const bool upperPart = (box.mCenter[yz] + box.mExtents[yz])<limitYZ;
	const bool lowerPart = (box.mCenter[yz] - box.mExtents[yz])>limitYZ;
	const bool leftPart = (box.mCenter.x + box.mExtents.x)<limitX;
	const bool rightPart = (box.mCenter.x - box.mExtents.x)>limitX;

	// Table-based box classification avoids many branches
	const PxU32 Code = PxU32(rightPart)|(PxU32(leftPart)<<1)|(PxU32(lowerPart)<<2)|(PxU32(upperPart)<<3);
	return gCodes[Code + (isCrossBucket ? 16 : 0)];
}

void BucketPrunerNode::classifyBoxes(	float limitX, float limitYZ,
										PxU32 nb, BucketBox* PX_RESTRICT boxes, const PrunerPayload* PX_RESTRICT objects,
										BucketBox* PX_RESTRICT sortedBoxes, PrunerPayload* PX_RESTRICT sortedObjects,
										bool isCrossBucket, PxU32 sortAxis)
{
	const PxU32 yz = PxU32(sortAxis == 1 ? 2 : 1);

#ifndef PX_PS3
	#ifdef _DEBUG
	{
		float prev = boxes[0].mDebugMin;
		for(PxU32 i=1;i<nb;i++)
		{
			const float current = boxes[i].mDebugMin;
			PX_ASSERT(current>=prev);
			prev = current;
		}
	}
	#endif
#endif

	// Local (stack-based) min/max bucket bounds
	PX_ALIGN(16, PxVec4) bucketBoxMin[5];
	PX_ALIGN(16, PxVec4) bucketBoxMax[5];
	{
		const PxBounds3 empty = PxBounds3::empty();
		for(PxU32 i=0;i<5;i++)
		{
			mCounters[i] = 0;
			bucketBoxMin[i] = PxVec4(empty.minimum, 0.0f);
			bucketBoxMax[i] = PxVec4(empty.maximum, 0.0f);
		}
	}

	{
#ifdef PX_WINDOWS
		// DS: order doesn't play nice with x86 shuffles :-|
		PX_ALIGN(16, PxVec4) limits(-limitX, limitX, -limitYZ, limitYZ);
		const bool useY = yz==1;
#endif
		// Determine in which bucket each object falls, update bucket bounds
		for(PxU32 i=0;i<nb;i++)
		{
			const Vec4V boxCenterV = AlignedLoad(&boxes[i].mCenter.x);
			const Vec4V boxExtentsV = AlignedLoad(&boxes[i].mExtents.x);
			const Vec4V boxMinV = V4Sub(boxCenterV, boxExtentsV);
			const Vec4V boxMaxV = V4Add(boxCenterV, boxExtentsV);

#ifdef PX_WINDOWS
//			const PxU32 index = classifyBox_x86(boxes[i], limits, useY, isCrossBucket);
			const PxU32 index = classifyBox_x86(boxMinV, boxMaxV, limits, useY, isCrossBucket);
	#ifdef PX_DEBUG
			const PxU32 index_ = classifyBox(boxes[i], limitX, limitYZ, yz, isCrossBucket);
			PX_ASSERT(index == index_);
	#endif
#else
			const PxU32 index = classifyBox(boxes[i], limitX, limitYZ, yz, isCrossBucket);
#endif
			// Merge boxes
			{
				const Vec4V mergedMinV = V4Min(V4LoadA(&bucketBoxMin[index].x), boxMinV);
				const Vec4V mergedMaxV = V4Max(V4LoadA(&bucketBoxMax[index].x), boxMaxV);
				V4StoreA(mergedMinV, &bucketBoxMin[index].x);
				V4StoreA(mergedMaxV, &bucketBoxMax[index].x);
			}
			boxes[i].mData0 = index;	// Store bucket index for current box in this temporary location
			mCounters[index]++;
		}
	}

	{
		// Regenerate offsets
		mOffsets[0]=0;
		for(PxU32 i=0;i<4;i++)
			mOffsets[i+1] = mOffsets[i] + mCounters[i];
	}

	{
		// Group boxes with same bucket index together
		for(PxU32 i=0;i<nb;i++)
		{
			const PxU32 bucketOffset = mOffsets[boxes[i].mData0]++;	// Bucket index for current box was stored in mData0 by previous loop
			// The 2 following lines are the same as:
			// sortedBoxes[bucketOffset] = boxes[i];
			AlignedStore(AlignedLoad(&boxes[i].mCenter.x), &sortedBoxes[bucketOffset].mCenter.x);
			AlignedStore(AlignedLoad(&boxes[i].mExtents.x), &sortedBoxes[bucketOffset].mExtents.x);
#ifndef PX_PS3
	#ifdef _DEBUG
			sortedBoxes[bucketOffset].mDebugMin = boxes[i].mDebugMin;
	#endif
#endif
			sortedObjects[bucketOffset] = objects[i];
		}
	}

	{
		// Regenerate offsets
		mOffsets[0]=0;
		for(PxU32 i=0;i<4;i++)
			mOffsets[i+1] = mOffsets[i] + mCounters[i];
	}

	{
		// Convert local (stack-based) min/max bucket bounds to persistent center/extents format
		const float Half = 0.5f;
		const FloatV HalfV = FLoad(Half);
		PX_ALIGN(16, PxVec4) bucketCenter;
		PX_ALIGN(16, PxVec4) bucketExtents;
		for(PxU32 i=0;i<5;i++)
		{
			// The following lines are the same as:
			// mBucketBox[i].mCenter = bucketBox[i].getCenter();
			// mBucketBox[i].mExtents = bucketBox[i].getExtents();
			const Vec4V bucketBoxMinV = V4LoadA(&bucketBoxMin[i].x);
			const Vec4V bucketBoxMaxV = V4LoadA(&bucketBoxMax[i].x);
			const Vec4V bucketBoxCenterV = V4Scale(V4Add(bucketBoxMaxV, bucketBoxMinV), HalfV);
			const Vec4V bucketBoxExtentsV = V4Scale(V4Sub(bucketBoxMaxV, bucketBoxMinV), HalfV);
			V4StoreA(bucketBoxCenterV, &bucketCenter.x);
			V4StoreA(bucketBoxExtentsV, &bucketExtents.x);
			mBucketBox[i].mCenter = PxVec3(bucketCenter.x, bucketCenter.y, bucketCenter.z);
			mBucketBox[i].mExtents = PxVec3(bucketExtents.x, bucketExtents.y, bucketExtents.z);
		}
	}

#ifndef PX_PS3
	#ifdef _DEBUG
	for(PxU32 j=0;j<5;j++)
	{
		const PxU32 count = mCounters[j];
		if(count)
		{
			const BucketBox* base = sortedBoxes + mOffsets[j];
			float prev = base[0].mDebugMin;
			for(PxU32 i=1;i<count;i++)
			{
				const float current = base[i].mDebugMin;
				PX_ASSERT(current>=prev);
				prev = current;
			}
		}
	}
	#endif
#endif
}

///////////////////////////////////////////////////////////////////////////////

static void processChildBuckets(PxU32 nbAllocated,
								BucketBox* sortedBoxesInBucket, PrunerPayload* sortedObjectsInBucket,
								const BucketPrunerNode& bucket, BucketPrunerNode* PX_RESTRICT childBucket,
								BucketBox* PX_RESTRICT baseBucketsBoxes, PrunerPayload* PX_RESTRICT baseBucketsObjects,
								PxU32 sortAxis)
{
	PX_UNUSED(nbAllocated);

	const PxU32 yz = PxU32(sortAxis == 1 ? 2 : 1);
	for(PxU32 i=0;i<5;i++)
	{
		const PxU32 nbInBucket = bucket.mCounters[i];
		if(!nbInBucket)
		{
			childBucket[i].initCounters();
			continue;
		}
		BucketBox* bucketsBoxes = baseBucketsBoxes + bucket.mOffsets[i];
		PrunerPayload* bucketsObjects = baseBucketsObjects + bucket.mOffsets[i];
		PX_ASSERT(nbInBucket<=nbAllocated);

		const float limitX = bucket.mBucketBox[i].mCenter.x;
		const float limitYZ = bucket.mBucketBox[i].mCenter[yz];
		const bool isCrossBucket = i==4;
		childBucket[i].classifyBoxes(limitX, limitYZ, nbInBucket, bucketsBoxes, bucketsObjects,
			sortedBoxesInBucket, sortedObjectsInBucket,
			isCrossBucket, sortAxis);

		PxMemCopy(bucketsBoxes, sortedBoxesInBucket, sizeof(BucketBox)*nbInBucket);
		PxMemCopy(bucketsObjects, sortedObjectsInBucket, sizeof(PrunerPayload)*nbInBucket);
	}
}

///////////////////////////////////////////////////////////////////////////////

static PX_FORCE_INLINE PxU32 encodeFloat(PxU32 newPos)
{
	//we may need to check on -0 and 0
	//But it should make no practical difference.
	if(newPos & 0x80000000) //negative?
		return ~newPos;//reverse sequence of negative numbers
	else
		return newPos | 0x80000000; // flip sign
}

static PX_FORCE_INLINE void computeRayLimits(float& rayMin, float& rayMax, const PxVec3& rayOrig, const PxVec3& rayDir, float maxDist, PxU32 sortAxis)
{
	const float rayOrigValue = rayOrig[sortAxis];
	const float rayDirValue = rayDir[sortAxis] * maxDist;
	rayMin = PxMin(rayOrigValue, rayOrigValue + rayDirValue);
	rayMax = PxMax(rayOrigValue, rayOrigValue + rayDirValue);
}

static PX_FORCE_INLINE void computeRayLimits(float& rayMin, float& rayMax, const PxVec3& rayOrig, const PxVec3& rayDir, float maxDist, const PxVec3& inflate, PxU32 sortAxis)
{
	const float inflateValue = inflate[sortAxis];
	const float rayOrigValue = rayOrig[sortAxis];
	const float rayDirValue = rayDir[sortAxis] * maxDist;
	rayMin = PxMin(rayOrigValue, rayOrigValue + rayDirValue) - inflateValue;
	rayMax = PxMax(rayOrigValue, rayOrigValue + rayDirValue) + inflateValue;
}

static PX_FORCE_INLINE void encodeBoxMinMax(BucketBox& box, const PxU32 axis)
{
	const float min = box.mCenter[axis] - box.mExtents[axis];
	const float max = box.mCenter[axis] + box.mExtents[axis];

	box.mData0 = encodeFloat(PX_IR(min));
	box.mData1 = encodeFloat(PX_IR(max));
}

///////////////////////////////////////////////////////////////////////////////

BucketPrunerCore::BucketPrunerCore(bool externalMemory) :
	mCoreNbObjects		(0),
	mCoreCapacity		(0),
	mCoreBoxes			(NULL),
	mCoreObjects		(NULL),
	mCoreRemap			(NULL),
	mSortedWorldBoxes	(NULL),
	mSortedObjects		(NULL),
	mNbFree				(0),
	mSortedNb			(0),
	mSortedCapacity		(0),
	mSortAxis			(0),
	mDirty				(true),
	mOwnMemory			(!externalMemory)
{
	mGlobalBox.setEmpty();

	mLevel1.initCounters();

	for(PxU32 i=0;i<5;i++)
		mLevel2[i].initCounters();
	for(PxU32 j=0;j<5;j++)
		for(PxU32 i=0;i<5;i++)
			mLevel3[j][i].initCounters();
}

BucketPrunerCore::~BucketPrunerCore()
{
	release();
}

void BucketPrunerCore::release()
{
	mDirty			= true;
	mCoreNbObjects	= 0;

	mCoreCapacity	= 0;
	if(mOwnMemory)
	{
		PX_FREE_AND_RESET(mCoreBoxes);
		PX_FREE_AND_RESET(mCoreObjects);
		PX_FREE_AND_RESET(mCoreRemap);
	}

	PX_FREE_AND_RESET(mSortedWorldBoxes);
	PX_FREE_AND_RESET(mSortedObjects);
	mSortedNb = 0;
	mSortedCapacity = 0;
}

void BucketPrunerCore::setExternalMemory(PxU32 nbObjects, PxBounds3* boxes, PrunerPayload* objects)
{
	PX_ASSERT(!mOwnMemory);
	mCoreNbObjects	= nbObjects;
	mCoreBoxes		= boxes;
	mCoreObjects	= objects;
	mCoreRemap		= NULL;
}

void BucketPrunerCore::allocateSortedMemory(PxU32 nb)
{
	mSortedNb = nb;
	if(nb<=mSortedCapacity && (nb>=mSortedCapacity/2))
		return;

	const PxU32 capacity = Ps::nextPowerOfTwo(nb);
	mSortedCapacity = capacity;

	PxU32 bytesNeededForBoxes = capacity*sizeof(BucketBox);
	bytesNeededForBoxes = ALIGN16(bytesNeededForBoxes);

	PxU32 bytesNeededForObjects = capacity*sizeof(PrunerPayload);
	bytesNeededForObjects = ALIGN16(bytesNeededForObjects);

	PX_FREE(mSortedObjects);
	PX_FREE(mSortedWorldBoxes);
	mSortedWorldBoxes = (BucketBox*)PX_ALLOC(bytesNeededForBoxes, PX_DEBUG_EXP("BucketPruner"));
	mSortedObjects = (PrunerPayload*)PX_ALLOC(bytesNeededForObjects, PX_DEBUG_EXP("BucketPruner"));
	PX_ASSERT(!(size_t(mSortedWorldBoxes)&15));
	PX_ASSERT(!(size_t(mSortedObjects)&15));
}

///////////////////////////////////////////////////////////////////////////////

void BucketPrunerCore::resizeCore()
{
	const PxU32 capacity = mCoreCapacity ? mCoreCapacity*2 : 32;
	mCoreCapacity = capacity;

	const PxU32 bytesNeededForBoxes = capacity*sizeof(PxBounds3);
	const PxU32 bytesNeededForObjects = capacity*sizeof(PrunerPayload);
	const PxU32 bytesNeededForRemap = capacity*sizeof(PxU32);

	PxBounds3* newCoreBoxes = (PxBounds3*)PX_ALLOC(bytesNeededForBoxes, PX_DEBUG_EXP("BucketPruner"));
	PrunerPayload* newCoreObjects = (PrunerPayload*)PX_ALLOC(bytesNeededForObjects, PX_DEBUG_EXP("BucketPruner"));
	PxU32* newCoreRemap = (PxU32*)PX_ALLOC(bytesNeededForRemap, PX_DEBUG_EXP("BucketPruner"));
	if(mCoreBoxes)
	{
		PxMemCopy(newCoreBoxes, mCoreBoxes, mCoreNbObjects*sizeof(PxBounds3));
		PX_FREE(mCoreBoxes);
	}
	if(mCoreObjects)
	{
		PxMemCopy(newCoreObjects, mCoreObjects, mCoreNbObjects*sizeof(PrunerPayload));
		PX_FREE(mCoreObjects);
	}
	if(mCoreRemap)
	{
		PxMemCopy(newCoreRemap, mCoreRemap, mCoreNbObjects*sizeof(PxU32));
		PX_FREE(mCoreRemap);
	}
	mCoreBoxes = newCoreBoxes;
	mCoreObjects = newCoreObjects;
	mCoreRemap = newCoreRemap;
}

PX_FORCE_INLINE void BucketPrunerCore::addObjectInternal(const PrunerPayload& object, const PxBounds3& worldAABB)
{
	if(mCoreNbObjects==mCoreCapacity)
		resizeCore();

	const PxU32 index = mCoreNbObjects++;
	mCoreObjects[index] = object;
	mCoreBoxes[index] = worldAABB;	// PT: TODO: check assembly here
	mCoreRemap[index] = 0xffffffff;

	// Objects are only inserted into the map once they're part of the main/core arrays.
	bool ok = mMap.insert(object, index);
	PX_UNUSED(ok);
	PX_ASSERT(ok);
}

bool BucketPrunerCore::addObject(const PrunerPayload& object, const PxBounds3& worldAABB)
{
/*
	We should probably use a bigger Payload struct here, which would also contains the external handle.
	(EDIT: we can't even do that, because of the setExternalMemory function)
	When asked to update/remove an object it would be O(n) to find the proper object in the mSortedObjects array.

	-

	For removing it we can simply empty the corresponding box, and the object will never be returned from queries.
	Maybe this isn't even true, since boxes are sorted along one axis. So marking a box as empty could break the code relying on a sorted order.
	An alternative is to mark the external handle as invalid, and ignore the object when a hit is found.

	(EDIT: the sorting is now tested via data0/data1 anyway so we could mark the box as empty without breaking this)

	-

	For updating an object we would need to keep the (sub) array sorted (not the whole thing, only the array within a bucket).
	We don't know the range (what part of the array maps to our bucket) but we may have the bucket ID somewhere? If we'd have this
	we could parse the array left/right and resort just the right boxes. If we don't have this we may be able to "quickly" find the
	range by traversing the tree, looking for the proper bucket. In any case I don't think there's a mapping to update within a bucket,
	unlike in SAP or MBP. So we should be able to shuffle a bucket without having to update anything. For example there's no mapping
	between the Core array and the Sorted array. It's a shame in a way because we'd need one, but it's not there - and in fact I think
	we can free the Core array once Sorted is created, we don't need it at all.

	If we don't want to re-sort the full bucket we can just mark it as dirty and ignore the sort-based early exits in the queries. Then we
	can incrementally resort it over N frames or something.

	This only works if the updated object remains in the same bucket though. If it moves to another bucket it becomes tempting to just remove
	the object and re-insert it.

	-

	Now for adding an object, we can first have a "free pruner" and do the 16 next entries brute-force. Rebuilding every 16 objects might
	give a good speedup already. Otherwise we need to do something more complicated.
*/

	PX_ASSERT(mOwnMemory);
	PX_ASSERT(!mDirty || !mNbFree);
	if(!mDirty)
	{
		// In this path the structure is marked as valid. We do not want to invalidate it for each new object...
		if(mNbFree<FREE_PRUNER_SIZE)
		{
			// ...so as long as there is space in the "free array", we store the newly added object there and
			// return immediately. Subsequent queries will parse the free array as if it was a free pruner.
			const PxU32 index = mNbFree++;
			mFreeObjects[index] = object;
			mFreeBounds[index] = worldAABB;
			return true;
		}

		// If we reach this place, the free array is full. We must transfer the objects from the free array to
		// the main (core) arrays, mark the structure as invalid, and still deal with the incoming object.

		// First we transfer free objects, reset the number of free objects, and mark the structure as
		// invalid/dirty (the core arrays will need rebuilding).
		for(PxU32 i=0;i<mNbFree;i++)
			addObjectInternal(mFreeObjects[i], mFreeBounds[i]);

		mNbFree = 0;
		mDirty = true;
//		mSortedNb = 0;	// PT: TODO: investigate if this should be done here

		// After that we still need to deal with the new incoming object (so far we only
		// transferred the already existing objects from the full free array). This will
		// happen automatically by letting the code continue to the regular codepath below.
	}

	// If we reach this place, the structure must be invalid and the incoming object
	// must be added to the main arrays.
	PX_ASSERT(mDirty);

	addObjectInternal(object, worldAABB);
	return true;
}

bool BucketPrunerCore::removeObject(const PrunerPayload& object)
{
	// Even if the structure is already marked as dirty, we still need to update the
	// core arrays and the map.

	// The map only contains core objects, so we can use it to determine if the object
	// exists in the core arrays or in the free array.
	const BucketPrunerMap::Entry* removedEntry = mMap.find(object);
	if(removedEntry)
	{
		// In this codepath, the object we want to remove exists in the core arrays.

		// We will need to remove it from both the core arrays & the sorted arrays.
		const PxU32 i = removedEntry->second;		// This is the object's index in the core arrays.
		const PxU32 sortedIndex = mCoreRemap[i];	// This is the object's index in the sorted arrays.

		// But first, remove it from the map
		bool status = mMap.erase(object);
		PX_ASSERT(status);
		PX_UNUSED(status);

		// Then let's deal with the core arrays
		mCoreNbObjects--;
		if(i!=mCoreNbObjects)
		{
			// If it wasn't the last object in the array, close the gaps as usual
			const PrunerPayload& movedObject = mCoreObjects[mCoreNbObjects];
			mCoreBoxes[i] = mCoreBoxes[mCoreNbObjects];
			mCoreObjects[i] = movedObject;
			mCoreRemap[i] = mCoreRemap[mCoreNbObjects];

			// Since we just moved the last object, its index in the core arrays has changed.
			// We must reflect this change in the map.
			BucketPrunerMap::Entry* movedEntry = const_cast<BucketPrunerMap::Entry*>(mMap.find(movedObject));
			PX_ASSERT(movedEntry->second==mCoreNbObjects);
			movedEntry->second = i;
		}

		// Now, let's deal with the sorted arrays.
		// If the structure is dirty, the sorted arrays will be rebuilt from scratch so there's no need to
		// update them right now.
		if(!mDirty)
		{
			// If the structure is valid, we want to keep it this way to avoid rebuilding sorted arrays after
			// each removal. We can't "close the gaps" easily here because order of objects in the arrays matters.
			
			// Instead we just invalidate the object by setting its bounding box as empty.
			// Queries against empty boxes will never return a hit, so this effectively "removes" the object
			// from any subsequent query results. Sorted arrays now contain a "disabled" object, until next build.
			
			// Invalidating the box does not invalidate the sorting, since it's now captured in mData0/mData1.
			// That is, mData0/mData1 keep their previous integer-encoded values, as if the box/object was still here.
			PxBounds3 empty;
			empty.setEmpty();
			mSortedWorldBoxes[sortedIndex].mCenter = empty.getCenter();
			mSortedWorldBoxes[sortedIndex].mExtents = empty.getExtents();
			// Note that we don't touch mSortedObjects here. We could, but this is not necessary.
		}
		return true;
	}

	// Here, the object we want to remove exists in the free array. So we just parse it.
	for(PxU32 i=0;i<mNbFree;i++)
	{
		if(mFreeObjects[i]==object)
		{
			// We found the object we want to remove. Close the gap as usual.
			mNbFree--;
			mFreeBounds[i] = mFreeBounds[mNbFree];
			mFreeObjects[i] = mFreeObjects[mNbFree];
			return true;
		}
	}
	// We didn't find the object. Can happen with a double remove. PX_ASSERT might be an option here.
	return false;
}

bool BucketPrunerCore::updateObject(const PxBounds3& worldAABB, const PrunerPayload& object)
{
	if(!removeObject(object))
		return false;

	return addObject(object, worldAABB);
}

///////////////////////////////////////////////////////////////////////////////

static PxU32 sortBoxes(	PxU32 nb, const PxBounds3* PX_RESTRICT boxes, const PrunerPayload* PX_RESTRICT objects,
						BucketBox& _globalBox, BucketBox* PX_RESTRICT sortedBoxes, PrunerPayload* PX_RESTRICT sortedObjects)
{
	// Compute global box & sort axis
	PxU32 sortAxis;
	{
		PX_ASSERT(nb>0);
		Vec4V mergedMinV = V4LoadU(&boxes[nb-1].minimum.x);
		Vec4V mergedMaxV = Vec4V_From_Vec3V(V3LoadU(&boxes[nb-1].maximum.x));
		for(PxU32 i=0;i<nb-1;i++)
		{
			mergedMinV = V4Min(mergedMinV, V4LoadU(&boxes[i].minimum.x));
			mergedMaxV = V4Max(mergedMaxV, V4LoadU(&boxes[i].maximum.x));
		}

/*		PX_ALIGN(16, PxVec4) mergedMin;
		PX_ALIGN(16, PxVec4) mergedMax;
		V4StoreA(mergedMinV, &mergedMin.x);
		V4StoreA(mergedMaxV, &mergedMax.x);

		_globalBox.mCenter.x = (mergedMax.x + mergedMin.x)*0.5f;
		_globalBox.mCenter.y = (mergedMax.y + mergedMin.y)*0.5f;
		_globalBox.mCenter.z = (mergedMax.z + mergedMin.z)*0.5f;
		_globalBox.mExtents.x = (mergedMax.x - mergedMin.x)*0.5f;
		_globalBox.mExtents.y = (mergedMax.y - mergedMin.y)*0.5f;
		_globalBox.mExtents.z = (mergedMax.z - mergedMin.z)*0.5f;*/

			const float Half = 0.5f;
			const FloatV HalfV = FLoad(Half);
			PX_ALIGN(16, PxVec4) mergedCenter;
			PX_ALIGN(16, PxVec4) mergedExtents;

			const Vec4V mergedCenterV = V4Scale(V4Add(mergedMaxV, mergedMinV), HalfV);
			const Vec4V mergedExtentsV = V4Scale(V4Sub(mergedMaxV, mergedMinV), HalfV);
			V4StoreA(mergedCenterV, &mergedCenter.x);
			V4StoreA(mergedExtentsV, &mergedExtents.x);
			_globalBox.mCenter = PxVec3(mergedCenter.x, mergedCenter.y, mergedCenter.z);
			_globalBox.mExtents = PxVec3(mergedExtents.x, mergedExtents.y, mergedExtents.z);

		const PxF32 absY = PxAbs(_globalBox.mExtents.y);
		const PxF32 absZ = PxAbs(_globalBox.mExtents.z);
		sortAxis = PxU32(absY < absZ ? 1 : 2);
//		printf("Sort axis: %d\n", sortAxis);
	}

	float* keys = reinterpret_cast<float*>(sortedObjects);
	for(PxU32 i=0;i<nb;i++)
		keys[i] = boxes[i].minimum[sortAxis];

	Gu::RadixSortBuffered rs;	// ###TODO: some allocs here, remove
	const PxU32* ranks = rs.Sort(keys, nb).GetRanks();

#ifdef PX_PS3

	// ### TEMP

	for(PxU32 i=0;i<nb;i++)
	{
		const PxU32 index = *ranks++;
		sortedBoxes[i].mCenter = boxes[index].getCenter();
		sortedBoxes[i].mExtents = boxes[index].getExtents();
		sortedObjects[i] = objects[index];
	}

#else
	const float Half = 0.5f;
	const FloatV HalfV = FLoad(Half);
	for(PxU32 i=0;i<nb;i++)
	{
		const PxU32 index = *ranks++;
//const PxU32 index = local[i].index;
//		sortedBoxes[i].mCenter = boxes[index].getCenter();
//		sortedBoxes[i].mExtents = boxes[index].getExtents();

		const Vec4V bucketBoxMinV = V4LoadU(&boxes[index].minimum.x);
		const Vec4V bucketBoxMaxV = Vec4V_From_Vec3V(V3LoadU(&boxes[index].maximum.x));
		const Vec4V bucketBoxCenterV = V4Scale(V4Add(bucketBoxMaxV, bucketBoxMinV), HalfV);
		const Vec4V bucketBoxExtentsV = V4Scale(V4Sub(bucketBoxMaxV, bucketBoxMinV), HalfV);
		// We don't need to preserve data0/data1 here
		AlignedStore(bucketBoxCenterV, &sortedBoxes[i].mCenter.x);
		AlignedStore(bucketBoxExtentsV, &sortedBoxes[i].mExtents.x);

#ifndef PX_PS3
	#ifdef _DEBUG
		sortedBoxes[i].mDebugMin = boxes[index].minimum[sortAxis];
	#endif
#endif
		sortedObjects[i] = objects[index];
	}
#endif
	return sortAxis;
}

#ifdef NODE_SORT
	template<class T>
	PX_CUDA_CALLABLE PX_FORCE_INLINE void tswap(T& x, T& y)
	{
		T tmp = x;
		x = y;
		y = tmp;
	}

/*	PX_FORCE_INLINE __m128 DotV(const __m128 a, const __m128 b)	
	{
		const __m128 dot1 = _mm_mul_ps(a, b);
		const __m128 shuf1 = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(dot1), _MM_SHUFFLE(0,0,0,0)));
		const __m128 shuf2 = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(dot1), _MM_SHUFFLE(1,1,1,1)));
		const __m128 shuf3 = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(dot1), _MM_SHUFFLE(2,2,2,2)));
		return _mm_add_ps(_mm_add_ps(shuf1, shuf2), shuf3);
	}*/

// PT: hmmm, by construction, isn't the order always the same for all bucket pruners?
// => maybe not because the bucket boxes are still around the merged aabbs, not around the bucket
// Still we could do something here
static /*PX_FORCE_INLINE*/ PxU32 sort(const BucketPrunerNode& parent, const PxVec3& rayDir)
{
	const PxU32 totalCount = parent.mCounters[0]+parent.mCounters[1]+parent.mCounters[2]+parent.mCounters[3]+parent.mCounters[4];
	if(totalCount<NODE_SORT_MIN_COUNT)
		return 0|(1<<3)|(2<<6)|(3<<9)|(4<<12);

	float dp[5];
/*	const __m128 rayDirV = _mm_loadu_ps(&rayDir.x);
	__m128 dp0V = DotV(rayDirV, _mm_loadu_ps(&parent.mBucketBox[0].mCenter.x));	_mm_store_ss(&dp[0], dp0V);
	__m128 dp1V = DotV(rayDirV, _mm_loadu_ps(&parent.mBucketBox[1].mCenter.x));	_mm_store_ss(&dp[1], dp1V);
	__m128 dp2V = DotV(rayDirV, _mm_loadu_ps(&parent.mBucketBox[2].mCenter.x));	_mm_store_ss(&dp[2], dp2V);
	__m128 dp3V = DotV(rayDirV, _mm_loadu_ps(&parent.mBucketBox[3].mCenter.x));	_mm_store_ss(&dp[3], dp3V);
	__m128 dp4V = DotV(rayDirV, _mm_loadu_ps(&parent.mBucketBox[4].mCenter.x));	_mm_store_ss(&dp[4], dp4V);
*/

#ifdef VERIFY_SORT
	PxU32 code;
	{
		dp[0] = parent.mCounters[0] ? PxAbs(parent.mBucketBox[0].mCenter.dot(rayDir)) : PX_MAX_F32;
		dp[1] = parent.mCounters[1] ? PxAbs(parent.mBucketBox[1].mCenter.dot(rayDir)) : PX_MAX_F32;
		dp[2] = parent.mCounters[2] ? PxAbs(parent.mBucketBox[2].mCenter.dot(rayDir)) : PX_MAX_F32;
		dp[3] = parent.mCounters[3] ? PxAbs(parent.mBucketBox[3].mCenter.dot(rayDir)) : PX_MAX_F32;
		dp[4] = parent.mCounters[4] ? PxAbs(parent.mBucketBox[4].mCenter.dot(rayDir)) : PX_MAX_F32;

		PxU32 ii0 = 0;
		PxU32 ii1 = 1;
		PxU32 ii2 = 2;
		PxU32 ii3 = 3;
		PxU32 ii4 = 4;

		// PT: using integer cmps since we used fabsf above
	//	const PxU32* values = reinterpret_cast<const PxU32*>(dp);
		const PxU32* values = PxUnionCast<PxU32*, PxF32*>(dp);

		PxU32 value0 = values[0];
		PxU32 value1 = values[1];
		PxU32 value2 = values[2];
		PxU32 value3 = values[3];
		PxU32 value4 = values[4];

		for(PxU32 j=0;j<5-1;j++)
		{
			if(value1<value0)
			{
				tswap(value0, value1);
				tswap(ii0, ii1);
			}
			if(value2<value1)
			{
				tswap(value1, value2);
				tswap(ii1, ii2);
			}
			if(value3<value2)
			{
				tswap(value2, value3);
				tswap(ii2, ii3);
			}
			if(value4<value3)
			{
				tswap(value3, value4);
				tswap(ii3, ii4);
			}
		}
		//return ii0|(ii1<<3)|(ii2<<6)|(ii3<<9)|(ii4<<12);
		code = ii0|(ii1<<3)|(ii2<<6)|(ii3<<9)|(ii4<<12);
	}
#endif

	dp[0] = parent.mCounters[0] ? parent.mBucketBox[0].mCenter.dot(rayDir) : PX_MAX_F32;
	dp[1] = parent.mCounters[1] ? parent.mBucketBox[1].mCenter.dot(rayDir) : PX_MAX_F32;
	dp[2] = parent.mCounters[2] ? parent.mBucketBox[2].mCenter.dot(rayDir) : PX_MAX_F32;
	dp[3] = parent.mCounters[3] ? parent.mBucketBox[3].mCenter.dot(rayDir) : PX_MAX_F32;
	dp[4] = parent.mCounters[4] ? parent.mBucketBox[4].mCenter.dot(rayDir) : PX_MAX_F32;

	const PxU32* values = PxUnionCast<PxU32*, PxF32*>(dp);

//	const PxU32 mask = ~7U;
	const PxU32 mask = 0x7ffffff8;
	PxU32 value0 = (values[0]&mask);
	PxU32 value1 = (values[1]&mask)|1;
	PxU32 value2 = (values[2]&mask)|2;
	PxU32 value3 = (values[3]&mask)|3;
	PxU32 value4 = (values[4]&mask)|4;

#define SORT_BLOCK								\
	if(value1<value0)	tswap(value0, value1);	\
	if(value2<value1)	tswap(value1, value2);	\
	if(value3<value2)	tswap(value2, value3);	\
	if(value4<value3)	tswap(value3, value4);
	SORT_BLOCK
	SORT_BLOCK
	SORT_BLOCK
	SORT_BLOCK

	const PxU32 ii0 = value0&7;
	const PxU32 ii1 = value1&7;
	const PxU32 ii2 = value2&7;
	const PxU32 ii3 = value3&7;
	const PxU32 ii4 = value4&7;
	const PxU32 code2 = ii0|(ii1<<3)|(ii2<<6)|(ii3<<9)|(ii4<<12);
#ifdef VERIFY_SORT
	PX_ASSERT(code2==code);
#endif
	return code2;
}

static void gPrecomputeSort(BucketPrunerNode& node, const PxVec3* PX_RESTRICT dirs)
{
	for(int i=0;i<8;i++)
		node.mOrder[i] = Ps::to16(sort(node, dirs[i]));
}
#endif

void BucketPrunerCore::classifyBoxes()
{
	if(!mDirty)
		return;

	mDirty = false;

	const PxU32 nb = mCoreNbObjects;
	if(!nb)
	{
		mSortedNb=0;
		return;
	}

	PX_ASSERT(!mNbFree);

#ifdef BRUTE_FORCE_LIMIT
	if(nb<=BRUTE_FORCE_LIMIT)
	{
		allocateSortedMemory(nb);
		BucketBox* sortedBoxes = mSortedWorldBoxes;
		PrunerPayload* sortedObjects = mSortedObjects;

		const float Half = 0.5f;
		const __m128 HalfV = _mm_load1_ps(&Half);
		PX_ALIGN(16, PxVec4) bucketCenter;
		PX_ALIGN(16, PxVec4) bucketExtents;
		for(PxU32 i=0;i<nb;i++)
		{
			const __m128 bucketBoxMinV = _mm_loadu_ps(&mCoreBoxes[i].minimum.x);
			const __m128 bucketBoxMaxV = _mm_loadu_ps(&mCoreBoxes[i].maximum.x);
			const __m128 bucketBoxCenterV = _mm_mul_ps(_mm_add_ps(bucketBoxMaxV, bucketBoxMinV), HalfV);
			const __m128 bucketBoxExtentsV = _mm_mul_ps(_mm_sub_ps(bucketBoxMaxV, bucketBoxMinV), HalfV);
			_mm_store_ps(&bucketCenter.x, bucketBoxCenterV);
			_mm_store_ps(&bucketExtents.x, bucketBoxExtentsV);
			sortedBoxes[i].mCenter = PxVec3(bucketCenter.x, bucketCenter.y, bucketCenter.z);
			sortedBoxes[i].mExtents = PxVec3(bucketExtents.x, bucketExtents.y, bucketExtents.z);

			sortedObjects[i] = mCoreObjects[i];
		}
		return;
	}
#endif


size_t* remap = (size_t*)PX_ALLOC(nb*sizeof(size_t), PX_DEBUG_EXP(""));
for(PxU32 i=0;i<nb;i++)
{
	remap[i] = mCoreObjects[i].data[0];
	mCoreObjects[i].data[0] = i;
}

//	printf("Nb objects: %d\n", nb);

	PrunerPayload localTempObjects[LOCAL_SIZE];
	BucketBox localTempBoxes[LOCAL_SIZE];
	PrunerPayload* tempObjects;
	BucketBox* tempBoxes;
	if(nb>LOCAL_SIZE)
	{
		tempObjects = (PrunerPayload*)PX_ALLOC(sizeof(PrunerPayload)*nb, PX_DEBUG_EXP("BucketPruner"));
		tempBoxes = (BucketBox*)PX_ALLOC(nb*sizeof(BucketBox), PX_DEBUG_EXP("BucketPruner"));
	}
	else
	{
		tempObjects = localTempObjects;
		tempBoxes = localTempBoxes;
	}

	mSortAxis = sortBoxes(nb, mCoreBoxes, mCoreObjects,
		mGlobalBox, tempBoxes, tempObjects
		);

	PX_ASSERT(mSortAxis);

	allocateSortedMemory(nb);
	BucketBox* sortedBoxes = mSortedWorldBoxes;
	PrunerPayload* sortedObjects = mSortedObjects;

	const PxU32 yz = PxU32(mSortAxis == 1 ? 2 : 1);
	const float limitX = mGlobalBox.mCenter.x;
	const float limitYZ = mGlobalBox.mCenter[yz];
	mLevel1.classifyBoxes(limitX, limitYZ, nb, tempBoxes, tempObjects,
		sortedBoxes, sortedObjects,
		false, mSortAxis);

	processChildBuckets(nb, tempBoxes, tempObjects,
		mLevel1, mLevel2, mSortedWorldBoxes, mSortedObjects,
		mSortAxis);

	for(PxU32 j=0;j<5;j++)
		processChildBuckets(nb, tempBoxes, tempObjects,
		mLevel2[j], mLevel3[j], mSortedWorldBoxes + mLevel1.mOffsets[j], mSortedObjects + mLevel1.mOffsets[j],
		mSortAxis);

	{
		for(PxU32 i=0;i<nb;i++)
		{
			encodeBoxMinMax(mSortedWorldBoxes[i], mSortAxis);
		}
	}

	if(nb>LOCAL_SIZE)
	{
		PX_FREE(tempBoxes);
		PX_FREE(tempObjects);
	}

for(PxU32 i=0;i<nb;i++)
{
	const PxU32 coreIndex = PxU32(mSortedObjects[i].data[0]);
	const size_t saved = remap[coreIndex];
	mSortedObjects[i].data[0] = saved;
	mCoreObjects[coreIndex].data[0] = saved;
	if(mCoreRemap)
		mCoreRemap[coreIndex] = i;
//	remap[i] = mCoreObjects[i].data[0];
//	mCoreObjects[i].data[0] = i;
}
PX_FREE(remap);

/*	if(mOwnMemory)
	{
		PX_FREE_AND_RESET(mCoreBoxes);
		PX_FREE_AND_RESET(mCoreObjects);
	}*/


#ifdef NODE_SORT
	{
		PxVec3 dirs[8];
		dirs[0] = PxVec3(1.0f, 1.0f, 1.0f);
		dirs[1] = PxVec3(1.0f, 1.0f, -1.0f);
		dirs[2] = PxVec3(1.0f, -1.0f, 1.0f);
		dirs[3] = PxVec3(1.0f, -1.0f, -1.0f);
		dirs[4] = PxVec3(-1.0f, 1.0f, 1.0f);
		dirs[5] = PxVec3(-1.0f, 1.0f, -1.0f);
		dirs[6] = PxVec3(-1.0f, -1.0f, 1.0f);
		dirs[7] = PxVec3(-1.0f, -1.0f, -1.0f);
		for(int i=0;i<8;i++)
			dirs[i].normalize();

		gPrecomputeSort(mLevel1, dirs);

		for(PxU32 i=0;i<5;i++)
			gPrecomputeSort(mLevel2[i], dirs);

		for(PxU32 j=0;j<5;j++)
		{
			for(PxU32 i=0;i<5;i++)
				gPrecomputeSort(mLevel3[j][i], dirs);
		}
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef PX_WINDOWS
	struct RayParams
	{
		PX_ALIGN(16,	PxVec3	mData2);	float	padding0;
		PX_ALIGN(16,	PxVec3	mFDir);		float	padding1;
		PX_ALIGN(16,	PxVec3	mData);		float	padding2;
		PX_ALIGN(16,	PxVec3	mInflate);	float	padding3;
	};

	static PX_FORCE_INLINE void precomputeRayData(RayParams* PX_RESTRICT rayParams, const PxVec3& rayOrig, const PxVec3& rayDir, float maxDist)
	{
	#ifdef USE_SIMD
		const float Half = 0.5f * maxDist;
		const __m128 HalfV = _mm_load1_ps(&Half);
		const __m128 DataV = _mm_mul_ps(_mm_loadu_ps(&rayDir.x), HalfV);
		const __m128 Data2V = _mm_add_ps(_mm_loadu_ps(&rayOrig.x), DataV);
		const PxU32 MaskI = 0x7fffffff;
		const __m128 FDirV = _mm_and_ps(_mm_load1_ps((float*)&MaskI), DataV);
		_mm_store_ps(&rayParams->mData.x, DataV);
		_mm_store_ps(&rayParams->mData2.x, Data2V);
		_mm_store_ps(&rayParams->mFDir.x, FDirV);
	#else
		const PxVec3 data = 0.5f * rayDir * maxDist;
		rayParams->mData = data;
		rayParams->mData2 = rayOrig + data;
		rayParams->mFDir.x = PxAbs(data.x);
		rayParams->mFDir.y = PxAbs(data.y);
		rayParams->mFDir.z = PxAbs(data.z);
	#endif
	}

	template <int inflateT>
	static PX_FORCE_INLINE IntBool _segmentAABB(const BucketBox& box, const RayParams* PX_RESTRICT params)
	{
	#ifdef USE_SIMD
		const PxU32 maskI = 0x7fffffff;
		const __m128 fdirV = _mm_load_ps(&params->mFDir.x);
//		#ifdef _DEBUG
		const __m128 extentsV = inflateT ? _mm_add_ps(_mm_loadu_ps(&box.mExtents.x), _mm_load_ps(&params->mInflate.x)) : _mm_loadu_ps(&box.mExtents.x);
		const __m128 DV = _mm_sub_ps(_mm_load_ps(&params->mData2.x), _mm_loadu_ps(&box.mCenter.x));
/*		#else
		const __m128 extentsV = inflateT ? _mm_add_ps(_mm_load_ps(&box.mExtents.x), _mm_load_ps(&params->mInflate.x)) : _mm_load_ps(&box.mExtents.x);
		const __m128 DV = _mm_sub_ps(_mm_load_ps(&params->mData2.x), _mm_load_ps(&box.mCenter.x));
		#endif*/
		__m128 absDV = _mm_and_ps(DV, _mm_load1_ps((float*)&maskI));
		absDV = _mm_cmpgt_ps(absDV, _mm_add_ps(extentsV, fdirV));
		const PxU32 test = (PxU32)_mm_movemask_ps(absDV);
		if((test&7)!=0)
			return 0;

		const __m128 dataZYX_V = _mm_load_ps(&params->mData.x);
		const __m128 dataXZY_V = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(dataZYX_V), _MM_SHUFFLE(3,0,2,1)));
		const __m128 DXZY_V = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(DV), _MM_SHUFFLE(3,0,2,1)));
		const __m128 fV = _mm_sub_ps(_mm_mul_ps(dataZYX_V, DXZY_V), _mm_mul_ps(dataXZY_V, DV));

		const __m128 fdirZYX_V = _mm_load_ps(&params->mFDir.x);
		const __m128 fdirXZY_V = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(fdirZYX_V), _MM_SHUFFLE(3,0,2,1)));
		const __m128 extentsXZY_V = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(extentsV), _MM_SHUFFLE(3,0,2,1)));
		const __m128 fg = _mm_add_ps(_mm_mul_ps(extentsV, fdirXZY_V), _mm_mul_ps(extentsXZY_V, fdirZYX_V));

		__m128 absfV = _mm_and_ps(fV, _mm_load1_ps((float*)&maskI));
		absfV = _mm_cmpgt_ps(absfV, fg);
		const PxU32 test2 = (PxU32)_mm_movemask_ps(absfV);
		return (test2&7)==0;
	#else
		const float boxExtentsx = inflateT ? box.mExtents.x + params->mInflate.x : box.mExtents.x;
		const float Dx = params->mData2.x - box.mCenter.x;	if(fabsf(Dx) > boxExtentsx + params->mFDir.x)	return IntFalse;

		const float boxExtentsz = inflateT ? box.mExtents.z + params->mInflate.z : box.mExtents.z;
		const float Dz = params->mData2.z - box.mCenter.z;	if(fabsf(Dz) > boxExtentsz + params->mFDir.z)	return IntFalse;

		const float boxExtentsy = inflateT ? box.mExtents.y + params->mInflate.y : box.mExtents.y;
		const float Dy = params->mData2.y - box.mCenter.y;	if(fabsf(Dy) > boxExtentsy + params->mFDir.y)	return IntFalse;

		float f;
		f = params->mData.y * Dz - params->mData.z * Dy;	if(fabsf(f) > boxExtentsy*params->mFDir.z + boxExtentsz*params->mFDir.y)	return IntFalse;
		f = params->mData.z * Dx - params->mData.x * Dz;	if(fabsf(f) > boxExtentsx*params->mFDir.z + boxExtentsz*params->mFDir.x)	return IntFalse;
		f = params->mData.x * Dy - params->mData.y * Dx;	if(fabsf(f) > boxExtentsx*params->mFDir.y + boxExtentsy*params->mFDir.x)	return IntFalse;
		return IntTrue;
	#endif
	}
#else
	#include "SqPrunerTestsSIMD.h"

	#ifdef DELAYED_WRITES
	struct BPRayAABBTest : RayAABBTest
	{
		PX_FORCE_INLINE BPRayAABBTest(const PxVec3& origin_, const PxVec3& unitDir_, const PxReal maxDist, const PxVec3& inflation_) :
			RayAABBTest(origin_, unitDir_, maxDist, inflation_)
		{
		}

		template<bool TInflate>
		PX_FORCE_INLINE void delayedTest(PxU32 res[4], const Vec3V center, const Vec3V extents) const
		{
			const Vec3V iExt = TInflate ? V3Add(extents, mInflation) : extents;

			// coordinate axes
			const Vec3V nodeMax = V3Add(center, iExt);
			const Vec3V nodeMin = V3Sub(center, iExt);

			// cross axes
			const Vec3V offset = V3Sub(mOrigin, center);
			const Vec3V offsetYZX = V3PermYZX(offset);
			const Vec3V iExtYZX = V3PermYZX(iExt);
		
			const Vec3V f = V3NegMulSub(mDirYZX, offset, V3Mul(mDir, offsetYZX));		
			const Vec3V g = V3MulAdd(iExt, mAbsDirYZX, V3Mul(iExtYZX, mAbsDir));

			const BoolV
				maskA = V3IsGrtrOrEq(nodeMax, mRayMin),
				maskB = V3IsGrtrOrEq(mRayMax, nodeMin),
				maskC = V3IsGrtrOrEq(g, V3Abs(f));
			const BoolV andABCMasks = BAnd(BAnd(maskA, maskB), maskC);

			BStoreA(andABCMasks, res);
		}
	};

	//static PX_FORCE_INLINE PxU32 testDelayedResults(const PxU32 res[4])
	static PX_FORCE_INLINE bool testDelayedResults(const PxU32 res[4])
	{
	/*	if(		res[0]==0xffffffff
			&&	res[1]==0xffffffff
			&&	res[2]==0xffffffff
			&&	res[3]==0xffffffff)
			return 1;
		else return 0;*/
		const PxU32 all = res[0]&res[1]&res[2]&res[3];
		return all==0xffffffff;
	}

	template <int inflateT>
	static PX_FORCE_INLINE void blockSegmentAABBDelayed(PxU32 res[5][4], const BucketPrunerNode& bucket, const BPRayAABBTest& test)
	{
	/*	if(bucket.mCounters[0])
			test.delayedTest(&res[0][0], Vec3V_From_PxVec3(bucket.mBucketBox[0].mCenter), Vec3V_From_PxVec3(bucket.mBucketBox[0].mExtents));
		if(bucket.mCounters[1])
			test.delayedTest(&res[1][0], Vec3V_From_PxVec3(bucket.mBucketBox[1].mCenter), Vec3V_From_PxVec3(bucket.mBucketBox[1].mExtents));
		if(bucket.mCounters[2])
			test.delayedTest(&res[2][0], Vec3V_From_PxVec3(bucket.mBucketBox[2].mCenter), Vec3V_From_PxVec3(bucket.mBucketBox[2].mExtents));
		if(bucket.mCounters[3])
			test.delayedTest(&res[3][0], Vec3V_From_PxVec3(bucket.mBucketBox[3].mCenter), Vec3V_From_PxVec3(bucket.mBucketBox[3].mExtents));
		if(bucket.mCounters[4])
			test.delayedTest(&res[4][0], Vec3V_From_PxVec3(bucket.mBucketBox[4].mCenter), Vec3V_From_PxVec3(bucket.mBucketBox[4].mExtents));*/

		for(PxU32 i=0;i<5;i++)
		{
			if(bucket.mCounters[i])
				test.delayedTest<inflateT>(&res[i][0], V3LoadU(bucket.mBucketBox[i].mCenter), V3LoadU(bucket.mBucketBox[i].mExtents));
		}
	}
	#else
	typedef RayAABBTest BPRayAABBTest;
	#endif

template <int inflateT>
static PX_FORCE_INLINE IntBool _segmentAABB(const BucketBox& box, const BPRayAABBTest& test)
{
	return (IntBool)test.check<inflateT>(V3LoadU(box.mCenter), V3LoadU(box.mExtents));
}

/*static PX_FORCE_INLINE IntBool _segmentAABB(const BucketBox& box, const BPRayAABBTest& test, PxU32 rayMinLimitX, PxU32 rayMaxLimitX)
{
	if(rayMinLimitX>box.mData1)
		return 0;
	if(rayMaxLimitX<box.mData0)
		return 0;

	return test(Vec3V_From_PxVec3(box.mCenter), Vec3V_From_PxVec3(box.mExtents));
}*/
#endif

template <int inflateT>
static PxAgain processBucket(
	PxU32 nb, const BucketBox* PX_RESTRICT baseBoxes, PrunerPayload* PX_RESTRICT baseObjects,
	PxU32 offset, PxU32 totalAllocated,
	const PxVec3& rayOrig, const PxVec3& rayDir, float& maxDist,
#ifdef PX_WINDOWS
	RayParams* PX_RESTRICT rayParams,
#else
	BPRayAABBTest& test, const PxVec3& inflate,
#endif
	PrunerCallback& pcb, PxU32& _rayMinLimitInt, PxU32& _rayMaxLimitInt, PxU32 sortAxis)
{
	PX_UNUSED(totalAllocated);

	const BucketBox* PX_RESTRICT _boxes = baseBoxes + offset;
	PrunerPayload* PX_RESTRICT _objects = baseObjects + offset;

	PxU32 rayMinLimitInt = _rayMinLimitInt;
	PxU32 rayMaxLimitInt = _rayMaxLimitInt;

	const BucketBox* last = _boxes + nb;

#ifdef DELAYED_WRITES
	const PxU32 MaxRes = 4;
	PX_ALIGN_PREFIX(16)	PrunerPayload* resObjects[MaxRes];
	PX_ALIGN_PREFIX(16)	PxU32 results[4*MaxRes];

	PxU32 nbRes = 0;
	while(_boxes!=last)
	{
		const BucketBox& currentBox = *_boxes++;
		PrunerPayload* currentObject = _objects++;

		if(currentBox.mData1<rayMinLimitInt)
			continue;

		if(currentBox.mData0>rayMaxLimitInt)
			goto Exit;

		resObjects[nbRes] = currentObject;
		test.delayedTest<inflateT>(&results[nbRes*4], V3LoadU(currentBox.mCenter), V3LoadU(currentBox.mExtents));
		nbRes++;
		if(nbRes==MaxRes)
		{
			nbRes=0;
			for(PxU32 i=0;i<MaxRes;i++)
			{
				if(testDelayedResults(&results[i*4]))
				{
					PxReal saveMaxDist = maxDist;
					const PxAgain again = pcb.invoke(maxDist, resObjects[i]);
					if (!again)
						return false;
					if(maxDist < saveMaxDist)
					{
						float rayMinLimit, rayMaxLimit;
						if(inflateT)
							computeRayLimits(rayMinLimit, rayMaxLimit, rayOrig, rayDir, maxDist, inflate, sortAxis);
						else
							computeRayLimits(rayMinLimit, rayMaxLimit, rayOrig, rayDir, maxDist, sortAxis);

						test.setDistance(maxDist);

						rayMinLimitInt = encodeFloat(PX_IR(rayMinLimit));
						rayMaxLimitInt = encodeFloat(PX_IR(rayMaxLimit));
					}
				}
			}
		}
	}
Exit:
	for(PxU32 i=0;i<nbRes;i++)
	{
		if(testDelayedResults(&results[i*4]))
		{
			//const PxU32 _status = (callback)(resObjects[i], maxDist, userData);
			PxReal saveMaxDist = maxDist;
			PxAgain again = pcb.invoke(maxDist, resObjects[i]);
			if(!again)
				return false;
			if(maxDist < saveMaxDist)
			{
				float rayMinLimit, rayMaxLimit;
				if(inflateT)
					computeRayLimits(rayMinLimit, rayMaxLimit, rayOrig, rayDir, maxDist, inflate, sortAxis);
				else
					computeRayLimits(rayMinLimit, rayMaxLimit, rayOrig, rayDir, maxDist, sortAxis);

				test.setDistance(maxDist);

				rayMinLimitInt = encodeFloat(PX_IR(rayMinLimit));
				rayMaxLimitInt = encodeFloat(PX_IR(rayMaxLimit));
			}
		}
	}
#else

	while(_boxes!=last)
	{
#ifdef __SPU__
		PrunerPayload* currentObject;
		const BucketBox& currentBox = fetchCachedData(currentObject, baseBoxes, _boxes, totalAllocated, baseObjects);

		_boxes++;
		_objects++;
#else
		const BucketBox& currentBox = *_boxes++;
		PrunerPayload* currentObject = _objects++;
#endif

		if(currentBox.mData1<rayMinLimitInt)
			continue;

		if(currentBox.mData0>rayMaxLimitInt)
			goto Exit;

#ifdef PX_WINDOWS
		if(!_segmentAABB<inflateT>(currentBox, rayParams))
			continue;
#else
		if(!_segmentAABB<inflateT>(currentBox, test))
			continue;
#endif

		const float MaxDist = maxDist;
		const PxAgain again = pcb.invoke(maxDist,currentObject);
		if(!again)
			return false;
		if(maxDist < MaxDist)
		{
			float rayMinLimit, rayMaxLimit;
#ifdef PX_WINDOWS
			if(inflateT)
				computeRayLimits(rayMinLimit, rayMaxLimit, rayOrig, rayDir, maxDist, rayParams->mInflate, sortAxis);
			else
				computeRayLimits(rayMinLimit, rayMaxLimit, rayOrig, rayDir, maxDist, sortAxis);

			precomputeRayData(rayParams, rayOrig, rayDir, maxDist);
#else
			if(inflateT)
				computeRayLimits(rayMinLimit, rayMaxLimit, rayOrig, rayDir, maxDist, inflate, sortAxis);
			else
				computeRayLimits(rayMinLimit, rayMaxLimit, rayOrig, rayDir, maxDist, sortAxis);

			test.setDistance(maxDist);
#endif
			rayMinLimitInt = encodeFloat(PX_IR(rayMinLimit));
			rayMaxLimitInt = encodeFloat(PX_IR(rayMaxLimit));
		}
	}
Exit:
#endif

	_rayMinLimitInt = rayMinLimitInt;
	_rayMaxLimitInt = rayMaxLimitInt;
	return true;
}

#ifdef NODE_SORT
static PxU32 computeDirMask(const PxVec3& dir)
{
	const PxU32 X = PX_IR(dir.x)>>31;
	const PxU32 Y = PX_IR(dir.y)>>31;
	const PxU32 Z = PX_IR(dir.z)>>31;
	return Z|(Y<<1)|(X<<2);
}
#endif

template <int inflateT>
static PxAgain stab(const BucketPrunerCore& core, PrunerCallback& pcb, const PxVec3& rayOrig, const PxVec3& rayDir, float& maxDist, const PxVec3 inflate)
{
	if(maxDist==PX_MAX_F32)
	{
		/*const*/ PxVec3 boxMin = core.mGlobalBox.getMin() - inflate;
		/*const*/ PxVec3 boxMax = core.mGlobalBox.getMax() + inflate;

		if(core.mNbFree)
		{
			// TODO: optimize this
			PxBounds3 freeGlobalBounds;
			freeGlobalBounds.setEmpty();
			for(PxU32 i=0;i<core.mNbFree;i++)
				freeGlobalBounds.include(core.mFreeBounds[i]);
			freeGlobalBounds.minimum -= inflate;
			freeGlobalBounds.maximum += inflate;
			boxMin = boxMin.minimum(freeGlobalBounds.minimum);
			boxMax = boxMax.maximum(freeGlobalBounds.maximum);
		}

		clipRay(rayOrig, rayDir, maxDist, boxMin, boxMax);
	}

#ifdef PX_WINDOWS
	RayParams rayParams;
	#ifdef USE_SIMD
	rayParams.padding0 = rayParams.padding1 = rayParams.padding2 = rayParams.padding3 = 0.0f;
	#endif
	if(inflateT)
		rayParams.mInflate = inflate;

	precomputeRayData(&rayParams, rayOrig, rayDir, maxDist);
#else
	BPRayAABBTest test(rayOrig, rayDir, maxDist, inflateT ? inflate : PxVec3(0.0f));
#endif

	for(PxU32 i=0;i<core.mNbFree;i++)
	{
		BucketBox tmp;
		tmp.mCenter = core.mFreeBounds[i].getCenter();
		tmp.mExtents = core.mFreeBounds[i].getExtents();

	#ifdef PX_WINDOWS
		if(_segmentAABB<inflateT>(tmp, &rayParams))
	#else
		if(_segmentAABB<inflateT>(tmp, test))
	#endif
		{
			if(!pcb.invoke(maxDist, &core.mFreeObjects[i]))
				return false;
		}
	}

	const PxU32 nb = core.mSortedNb;
	if(!nb)
		return true;

#ifdef PX_WINDOWS
	if(!_segmentAABB<inflateT>(core.mGlobalBox, &rayParams))
		return true;
#else
	if(!_segmentAABB<inflateT>(core.mGlobalBox, test))
		return true;
#endif

	const PxU32 sortAxis = core.mSortAxis;
	float rayMinLimit, rayMaxLimit;
	if(inflateT)
		computeRayLimits(rayMinLimit, rayMaxLimit, rayOrig, rayDir, maxDist, inflate, sortAxis);
	else
		computeRayLimits(rayMinLimit, rayMaxLimit, rayOrig, rayDir, maxDist, sortAxis);

	PxU32 rayMinLimitInt = encodeFloat(PX_IR(rayMinLimit));
	PxU32 rayMaxLimitInt = encodeFloat(PX_IR(rayMaxLimit));

/*
float rayMinLimitX, rayMaxLimitX;
if(inflateT)
	computeRayLimits(rayMinLimitX, rayMaxLimitX, rayOrig, rayDir, maxDist, inflate, 0);
else
	computeRayLimits(rayMinLimitX, rayMaxLimitX, rayOrig, rayDir, maxDist, 0);

PxU32 rayMinLimitIntX = encodeFloat(PX_IR(rayMinLimitX));
PxU32 rayMaxLimitIntX = encodeFloat(PX_IR(rayMaxLimitX));
*/

#ifdef __SPU__
	gBaseIndex_CachedBoxes = 0xffffffff;
#endif

	float currentDist = maxDist;

#ifdef DELAYED_WRITES_BLOCK
	PxU32 resi[5][4];
	blockSegmentAABBDelayed<inflateT>(resi, core.mLevel1, test);
#endif

#ifdef NODE_SORT
	const PxU32 dirIndex = computeDirMask(rayDir);
	PxU32 orderi = core.mLevel1.mOrder[dirIndex];
//	PxU32 orderi = sort(core.mLevel1, rayDir);

	for(PxU32 i_=0;i_<5;i_++)
	{
		const PxU32 i = orderi&7;	orderi>>=3;
#else
	for(PxU32 i=0;i<5;i++)
	{
#endif

#ifdef DELAYED_WRITES_BLOCK
		if(core.mLevel1.mCounters[i] && testDelayedResults(&resi[i][0]))
#else
	#ifdef PX_WINDOWS
		if(core.mLevel1.mCounters[i] && _segmentAABB<inflateT>(core.mLevel1.mBucketBox[i], &rayParams))
	#else
		if(core.mLevel1.mCounters[i] && _segmentAABB<inflateT>(core.mLevel1.mBucketBox[i], test))
//		if(core.mLevel1.mCounters[i] && _segmentAABB<inflateT>(core.mLevel1.mBucketBox[i], test, rayMinLimitIntX, rayMaxLimitIntX))
	#endif
#endif
		{

#ifdef DELAYED_WRITES_BLOCK
			PxU32 resj[5][4];
			blockSegmentAABBDelayed<inflateT>(resj, core.mLevel2[i], test);
#endif

#ifdef NODE_SORT
			PxU32 orderj = core.mLevel2[i].mOrder[dirIndex];
//			PxU32 orderj = sort(core.mLevel2[i], rayDir);

			for(PxU32 j_=0;j_<5;j_++)
			{
				const PxU32 j = orderj&7;	orderj>>=3;
#else
			for(PxU32 j=0;j<5;j++)
			{
#endif

#ifdef DELAYED_WRITES_BLOCK
				if(core.mLevel2[i].mCounters[j] && testDelayedResults(&resj[j][0]))
#else
	#ifdef PX_WINDOWS
				if(core.mLevel2[i].mCounters[j] && _segmentAABB<inflateT>(core.mLevel2[i].mBucketBox[j], &rayParams))
	#else
				if(core.mLevel2[i].mCounters[j] && _segmentAABB<inflateT>(core.mLevel2[i].mBucketBox[j], test))
//				if(core.mLevel2[i].mCounters[j] && _segmentAABB<inflateT>(core.mLevel2[i].mBucketBox[j], test, rayMinLimitIntX, rayMaxLimitIntX))
	#endif
#endif
				{
					const BucketPrunerNode& parent = core.mLevel3[i][j];
					const PxU32 parentOffset = core.mLevel1.mOffsets[i] + core.mLevel2[i].mOffsets[j];

#ifdef DELAYED_WRITES_BLOCK
					PxU32 resk[5][4];
					blockSegmentAABBDelayed<inflateT>(resk, parent, test);
#endif

#ifdef NODE_SORT
					PxU32 orderk = parent.mOrder[dirIndex];
//					PxU32 orderk = sort(parent, rayDir);

					for(PxU32 k_=0;k_<5;k_++)
					{
						const PxU32 k = orderk&7;	orderk>>=3;
#else
					for(PxU32 k=0;k<5;k++)
					{
#endif
						const PxU32 nbInBucket = parent.mCounters[k];
#ifdef DELAYED_WRITES_BLOCK
						if(nbInBucket && testDelayedResults(&resk[k][0]))
#else
	#ifdef PX_WINDOWS
						if(nbInBucket && _segmentAABB<inflateT>(parent.mBucketBox[k], &rayParams))
	#else
						if(nbInBucket && _segmentAABB<inflateT>(parent.mBucketBox[k], test))
//						if(nbInBucket && _segmentAABB<inflateT>(parent.mBucketBox[k], test, rayMinLimitIntX, rayMaxLimitIntX))
	#endif
#endif
						{
							const PxU32 offset = parentOffset + parent.mOffsets[k];
							const PxAgain again = processBucket<inflateT>(	nbInBucket, core.mSortedWorldBoxes, core.mSortedObjects,
																			offset, core.mSortedNb,
																			rayOrig, rayDir, currentDist,
#ifdef PX_WINDOWS
																			&rayParams,
#else
																			test, inflate,
#endif
																			pcb,
																			rayMinLimitInt, rayMaxLimitInt,
																			sortAxis);
							if(!again)
								return false;
						}
					}
				}
			}
		}
	}

	maxDist = currentDist;
	return true;
}
		
PxAgain BucketPrunerCore::raycast(const PxVec3& origin, const PxVec3& unitDir, PxReal& inOutDistance, PrunerCallback& pcb) const
{
	return ::stab<0>(*this, pcb, origin, unitDir, inOutDistance, PxVec3(0.0f));
}

PxAgain BucketPrunerCore::sweep(const ShapeData& queryVolume, const PxVec3& unitDir, PxReal& inOutDistance, PrunerCallback& pcb) const
{
	PxVec3 extents = queryVolume.getPrunerInflatedWorldAABB().getExtents();
	return ::stab<1>(*this, pcb, queryVolume.getPrunerInflatedWorldAABB().getCenter(), unitDir, inOutDistance, extents);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<bool doAssert, typename Test>
static PX_FORCE_INLINE bool processBucket(	PxU32 nb, const BucketBox* PX_RESTRICT baseBoxes, PrunerPayload* PX_RESTRICT baseObjects,
											PxU32 offset, PxU32 totalAllocated,
											const Test& test, PrunerCallback& pcb,
											PxU32 minLimitInt, PxU32 maxLimitInt)
{
	PX_UNUSED(totalAllocated);

	const BucketBox* PX_RESTRICT boxes = baseBoxes + offset;
	PrunerPayload* PX_RESTRICT objects = baseObjects + offset;

	while(nb--)
	{
#ifdef __SPU__
		PrunerPayload* currentObject;
		const BucketBox& currentBox = fetchCachedData(currentObject, baseBoxes, boxes, totalAllocated, baseObjects);

		boxes++;
		objects++;
#else
		const BucketBox& currentBox = *boxes++;
		PrunerPayload* currentObject = objects++;
#endif

		if(currentBox.mData1<minLimitInt)
		{
			if(doAssert)
				PX_ASSERT(!test(currentBox));
			continue;
		}

		if(currentBox.mData0>maxLimitInt)
		{
			if(doAssert)
				PX_ASSERT(!test(currentBox));
			return true;
		}

		if(test(currentBox))
		{
			PxReal dist = -1.0f; // no distance for overlaps
			if(!pcb.invoke(dist,currentObject))
				return false;
		}
	}
	return true;
}

template<typename Test, bool isPrecise>
class BucketPrunerOverlapTraversal
{
public:
	/*PX_FORCE_INLINE*/ bool operator()(const BucketPrunerCore& core, const Test& test, PrunerCallback& pcb, const PxBounds3& cullBox)
	{
		for(PxU32 i=0;i<core.mNbFree;i++)
		{
			BucketBox tmp;
			tmp.mCenter = core.mFreeBounds[i].getCenter();
			tmp.mExtents = core.mFreeBounds[i].getExtents();
			if(test(tmp))
			{
				PxReal dist = -1.0f; // no distance for overlaps
				if(!pcb.invoke(dist, &core.mFreeObjects[i]))
					return false;
			}
		}

		const PxU32 nb = core.mSortedNb;
		if(!nb)
			return true;

#ifdef BRUTE_FORCE_LIMIT
		if(nb<=BRUTE_FORCE_LIMIT)
		{
			for(PxU32 i=0;i<nb;i++)
			{
				if(test(core.mSortedWorldBoxes[i]))
				{
					PxReal dist = -1.0f; // no distance for overlaps
					if(!pcb.invoke(dist, core.mSortedObjects+i))
						return false;
				}
			}
			return true;
		}
#endif

		if(!test(core.mGlobalBox))
			return true;

#ifdef __SPU__
		gBaseIndex_CachedBoxes = 0xffffffff;
#endif

		const PxU32 sortAxis = core.mSortAxis;
		const float boxMinLimit = cullBox.minimum[sortAxis];
		const float boxMaxLimit = cullBox.maximum[sortAxis];

		const PxU32 rayMinLimitInt = encodeFloat(PX_IR(boxMinLimit));
		const PxU32 rayMaxLimitInt = encodeFloat(PX_IR(boxMaxLimit));

		for(PxU32 i=0;i<5;i++)
		{
			if(core.mLevel1.mCounters[i] && test(core.mLevel1.mBucketBox[i]))
			{
				for(PxU32 j=0;j<5;j++)
				{
					if(core.mLevel2[i].mCounters[j] && test(core.mLevel2[i].mBucketBox[j]))
					{
						for(PxU32 k=0;k<5;k++)
						{
							const PxU32 nbInBucket = core.mLevel3[i][j].mCounters[k];
							if(nbInBucket && test(core.mLevel3[i][j].mBucketBox[k]))
							{
								const PxU32 offset = core.mLevel1.mOffsets[i] + core.mLevel2[i].mOffsets[j] + core.mLevel3[i][j].mOffsets[k];
								if(!processBucket<isPrecise>(nbInBucket, core.mSortedWorldBoxes, core.mSortedObjects,
									offset, core.mSortedNb, test, pcb, rayMinLimitInt, rayMaxLimitInt))
									return false;
							}
						}
					}
				}
			}
		}
		return true;
	}
};

///////////////////////////////////////////////////////////////////////////////

#ifdef PX_WINDOWS
PX_FORCE_INLINE PxU32 BAllTrue3_R(const BoolV a)
{
#if COMPILE_VECTOR_INTRINSICS
	const PxI32 moveMask = _mm_movemask_ps(a);
	return PxU32((moveMask & 0x7) == (0x7));
#else
	PX_ASSERT(false);
	return 0;
#endif
}
#endif

struct SphereAABBTest_SIMD
{
	PX_FORCE_INLINE SphereAABBTest_SIMD(const Gu::Sphere& sphere) :
#ifdef PX_WINDOWS
		mCenter	(V4LoadU(&sphere.center.x)),
#else
		mCenter	(V3LoadU(sphere.center)),
#endif
		mRadius2(FLoad(sphere.radius * sphere.radius))
	{}

	PX_FORCE_INLINE Ps::IntBool operator()(const BucketBox& box) const		
	{
	#ifdef PX_WINDOWS
		const Vec4V boxCenter = AlignedLoad(&box.mCenter.x);
		const Vec4V boxExtents = AlignedLoad(&box.mExtents.x);
		//
		const Vec4V offset = V4Sub(mCenter, boxCenter);
		const Vec4V closest = V4Clamp(offset, V4Neg(boxExtents), boxExtents);
		const Vec4V d = V4Sub(offset, closest);
		//return BAllEq(FIsGrtrOrEq(mRadius2, V3Dot(d, d)), BTTTT());

		// this will not compile with scalar SIMD due to type mismatch
		// see US12750 about implementing a safe Vec4-3Dot function
		const FloatV dot = V3Dot(d,d); 
		return (Ps::IntBool)BAllTrue3_R(FIsGrtrOrEq(mRadius2, dot));
	#else
		const Vec3V boxCenter = V3LoadU(box.mCenter);
		const Vec3V boxExtents = V3LoadU(box.mExtents);
		//
		const Vec3V offset = V3Sub(mCenter, boxCenter);
		const Vec3V closest = V3Clamp(offset, V3Neg(boxExtents), boxExtents);
		const Vec3V d = V3Sub(offset, closest);
		return (IntBool)BAllEq(FIsGrtrOrEq(mRadius2, V3Dot(d, d)), BTTTT());
	#endif
	}

private:
	SphereAABBTest_SIMD& operator=(const SphereAABBTest_SIMD&);
#ifdef PX_WINDOWS
	const Vec4V		mCenter;
#else
	const Vec3V		mCenter;
#endif
	const FloatV	mRadius2;
};

struct SphereAABBTest_Scalar
{
	PX_FORCE_INLINE SphereAABBTest_Scalar(const Gu::Sphere& sphere) :
		mCenter	(sphere.center),
		mRadius2(sphere.radius * sphere.radius)
	{}

	PX_FORCE_INLINE Ps::IntBool operator()(const BucketBox& box) const		
	{
		const PxVec3 minimum = box.getMin();
		const PxVec3 maximum = box.getMax();

		float d = 0.0f;

		//find the square of the distance
		//from the sphere to the box
		for(PxU32 i=0;i<3;i++)
		{
			if(mCenter[i]<minimum[i])
			{
				const float s = mCenter[i] - minimum[i];
				d += s*s;
			}
			else if(mCenter[i]>maximum[i])
			{
				const float s = mCenter[i] - maximum[i];
				d += s*s;
			}
		}
		return d <= mRadius2;
	}

private:
	SphereAABBTest_Scalar& operator=(const SphereAABBTest_Scalar&);
	const PxVec3	mCenter;
	float			mRadius2;
};

#ifdef USE_SIMD
typedef SphereAABBTest_SIMD		BucketPrunerSphereAABBTest;
#else
typedef SphereAABBTest_Scalar	BucketPrunerSphereAABBTest;
#endif

///////////////////////////////////////////////////////////////////////////////

static PX_FORCE_INLINE bool intersects(const BucketBox& box, const PxBounds3& b)
{
	const PxVec3 bucketMin = box.getMin();
	const PxVec3 bucketMax = box.getMax();

	return !(b.minimum.x > bucketMax.x || bucketMin.x > b.maximum.x ||
			 b.minimum.y > bucketMax.y || bucketMin.y > b.maximum.y ||
			 b.minimum.z > bucketMax.z || bucketMin.z > b.maximum.z);
}

struct BucketPrunerAABBAABBTest
{
	PX_FORCE_INLINE BucketPrunerAABBAABBTest(const PxBounds3& queryBox) : mBox(queryBox)	{}

	PX_FORCE_INLINE Ps::IntBool operator()(const BucketBox& box) const
	{	
		// PT: isValid asserts on our empty boxes!
		return intersects(box, mBox);
//		return mBox.intersects(PxBounds3(box.getMin(), box.getMax()));
	}
private:
	BucketPrunerAABBAABBTest& operator=(const BucketPrunerAABBAABBTest&);
	const PxBounds3	mBox;
};

/*struct BucketPrunerAABBAABBTest_SIMD
{
	PX_FORCE_INLINE BucketPrunerAABBAABBTest_SIMD(const PxBounds3& b)
	: mCenter(V3LoadU(b.getCenter()))
	, mExtents(V3LoadU(b.getExtents()))
	{}

	PX_FORCE_INLINE Ps::IntBool operator()(const BucketBox& box) const
	{	
		return V3AllGrtrOrEq(V3Add(mExtents, AlignedLoad(&box.mExtents.x)), V3Abs(V3Sub(AlignedLoad(&box.mCenter.x), mCenter)));
	}
private:
	BucketPrunerAABBAABBTest_SIMD& operator=(const BucketPrunerAABBAABBTest_SIMD&);
	const Vec3V mCenter, mExtents;
};*/

///////////////////////////////////////////////////////////////////////////////

struct OBBAABBTest_SIMD
{
	OBBAABBTest_SIMD(const PxMat33& rotation, const PxVec3& translation, const PxVec3& extents)
	{
		const Vec3V eps = V3Load(1e-6f);

		mT = V3LoadU(translation);
		mExtents = V3LoadU(extents);	

		// storing the transpose matrices yields a simpler SIMD test
		mRT = Mat33V_From_PxMat33(rotation.getTranspose());	
		mART = Mat33V(V3Add(V3Abs(mRT.col0), eps), V3Add(V3Abs(mRT.col1), eps), V3Add(V3Abs(mRT.col2), eps));
		mBB_xyz = M33TrnspsMulV3(mART, mExtents);

/*		if(fullTest)
		{
			const Vec3V eYZX = V3PermYZX(mExtents), eZXY = V3PermZXY(mExtents);

			mBB_123 = V3MulAdd(eYZX, V3PermZXY(mART.col0), V3Mul(eZXY, V3PermYZX(mART.col0)));
			mBB_456 = V3MulAdd(eYZX, V3PermZXY(mART.col1), V3Mul(eZXY, V3PermYZX(mART.col1)));
			mBB_789 = V3MulAdd(eYZX, V3PermZXY(mART.col2), V3Mul(eZXY, V3PermYZX(mART.col2)));
		}*/
	}

	// TODO: force inline it?
	PX_FORCE_INLINE Ps::IntBool operator()(const BucketBox& box) const	
	{	
		const Vec3V extentsV = V3LoadU(box.mExtents);

		const Vec3V t = V3Sub(mT, V3LoadU(box.mCenter));

		// class I - axes of AABB
		if(V3OutOfBounds(t, V3Add(extentsV, mBB_xyz)))
			return Ps::IntFalse;

		const Vec3V rX = mRT.col0, rY = mRT.col1, rZ = mRT.col2;
		const Vec3V arX = mART.col0, arY = mART.col1, arZ = mART.col2;

		const FloatV eX = V3GetX(extentsV), eY = V3GetY(extentsV), eZ = V3GetZ(extentsV);
		const FloatV tX = V3GetX(t), tY = V3GetY(t), tZ = V3GetZ(t);

		// class II - axes of OBB
		{
			const Vec3V v = V3ScaleAdd(rZ, tZ, V3ScaleAdd(rY, tY, V3Scale(rX, tX)));
			const Vec3V v2 = V3ScaleAdd(arZ, eZ, V3ScaleAdd(arY, eY, V3ScaleAdd(arX, eX, mExtents)));
			if(V3OutOfBounds(v, v2))
				return Ps::IntFalse;
		}

//		if(!fullTest)
			return Ps::IntTrue;

/*		// class III - edge cross products. Almost all OBB tests early-out with type I or type II,
		// so early-outs here probably aren't useful (TODO: profile)

		const Vec3V va = V3NegScaleSub(rZ, tY, V3Scale(rY, tZ));
		const Vec3V va2 = V3ScaleAdd(arY, eZ, V3ScaleAdd(arZ, eY, mBB_123));
		const BoolV ba = BOr(V3IsGrtr(va, va2), V3IsGrtr(V3Neg(va2), va));
	
		const Vec3V vb = V3NegScaleSub(rX, tZ, V3Scale(rZ, tX));
		const Vec3V vb2 = V3ScaleAdd(arX, eZ, V3ScaleAdd(arZ, eX, mBB_456));
		const BoolV bb = BOr(V3IsGrtr(vb, vb2), V3IsGrtr(V3Neg(vb2), vb));
		
		const Vec3V vc = V3NegScaleSub(rY, tX, V3Scale(rX, tY));
		const Vec3V vc2 = V3ScaleAdd(arX, eY, V3ScaleAdd(arY, eX, mBB_789));
		const BoolV bc = BOr(V3IsGrtr(vc, vc2), V3IsGrtr(V3Neg(vc2), vc));

		return BAllEq(BOr(ba, BOr(bb,bc)), BFFFF());*/
	}
	
	Vec3V		mExtents;	// extents of OBB
	Vec3V		mT;			// translation of OBB
	Mat33V		mRT;		// transpose of rotation matrix of OBB
	Mat33V		mART;		// transpose of mRT, padded by epsilon
	Vec3V		mBB_xyz;	// extents of OBB along coordinate axes

/*	Vec3V		mBB_123;	// projections of extents onto edge-cross axes
	Vec3V		mBB_456;
	Vec3V		mBB_789;*/
};

struct OBBAABBTest_Scalar
{
	OBBAABBTest_Scalar(const PxMat33& rotation, const PxVec3& translation, const PxVec3& extents)
	{
		mR = rotation;
		mT = translation;
		mExtents = extents;

		const PxVec3 eps(1e-6f);		
		mAR = PxMat33(mR[0].abs() + eps, mR[1].abs() + eps, mR[2].abs() + eps);			// Epsilon prevents floating-point inaccuracies (strategy borrowed from RAPID)
		mBB_xyz = mAR.transform(mExtents);												// Precompute box-box data - Courtesy of Erwin de Vries

/*		PxReal ex = mExtents.x, ey = mExtents.y, ez = mExtents.z;
		mBB_1 = ey*mAR[2].x + ez*mAR[1].x; mBB_2 = ez*mAR[0].x + ex*mAR[2].x; mBB_3 = ex*mAR[1].x + ey*mAR[0].x;
		mBB_4 = ey*mAR[2].y + ez*mAR[1].y; mBB_5 = ez*mAR[0].y + ex*mAR[2].y; mBB_6 = ex*mAR[1].y + ey*mAR[0].y;
		mBB_7 = ey*mAR[2].z + ez*mAR[1].z; mBB_8 = ez*mAR[0].z + ex*mAR[2].z; mBB_9 = ex*mAR[1].z + ey*mAR[0].z;*/
	}

	PX_FORCE_INLINE Ps::IntBool operator()(const BucketBox& box) const	
	{
		const PxVec3& c = box.mCenter;
		const PxVec3& e = box.mExtents;

		const PxVec3 T = mT - c;
		// Class I : A's basis vectors
		if(PxAbs(T.x) > e.x + mBB_xyz.x)	return Ps::IntFalse;
		if(PxAbs(T.y) > e.y + mBB_xyz.y)	return Ps::IntFalse;
		if(PxAbs(T.z) > e.z + mBB_xyz.z)	return Ps::IntFalse;

		// Class II : B's basis vectors
		if(PxAbs(T.dot(mR[0])) > e.dot(mAR[0]) + mExtents.x)	return Ps::IntFalse;
		if(PxAbs(T.dot(mR[1])) > e.dot(mAR[1]) + mExtents.y)	return Ps::IntFalse;
		if(PxAbs(T.dot(mR[2])) > e.dot(mAR[2]) + mExtents.z)	return Ps::IntFalse;

		// Class III : 9 cross products
		if(0)
		{
			if(PxAbs(T.z*mR[0].y - T.y*mR[0].z) > e.y*mAR[0].z + e.z*mAR[0].y + mBB_1) return Ps::IntFalse;	// L = A0 x B0
			if(PxAbs(T.z*mR[1].y - T.y*mR[1].z) > e.y*mAR[1].z + e.z*mAR[1].y + mBB_2) return Ps::IntFalse;	// L = A0 x B1
			if(PxAbs(T.z*mR[2].y - T.y*mR[2].z) > e.y*mAR[2].z + e.z*mAR[2].y + mBB_3) return Ps::IntFalse;	// L = A0 x B2

			if(PxAbs(T.x*mR[0].z - T.z*mR[0].x) > e.x*mAR[0].z + e.z*mAR[0].x + mBB_4) return Ps::IntFalse;	// L = A1 x B0
			if(PxAbs(T.x*mR[1].z - T.z*mR[1].x) > e.x*mAR[1].z + e.z*mAR[1].x + mBB_5) return Ps::IntFalse;	// L = A1 x B1
			if(PxAbs(T.x*mR[2].z - T.z*mR[2].x) > e.x*mAR[2].z + e.z*mAR[2].x + mBB_6) return Ps::IntFalse;	// L = A1 x B2

			if(PxAbs(T.y*mR[0].x - T.x*mR[0].y) > e.x*mAR[0].y + e.y*mAR[0].x + mBB_7) return Ps::IntFalse;	// L = A2 x B0
			if(PxAbs(T.y*mR[1].x - T.x*mR[1].y) > e.x*mAR[1].y + e.y*mAR[1].x + mBB_8) return Ps::IntFalse;	// L = A2 x B1
			if(PxAbs(T.y*mR[2].x - T.x*mR[2].y) > e.x*mAR[2].y + e.y*mAR[2].x + mBB_9) return Ps::IntFalse;	// L = A2 x B2
		}
		return Ps::IntTrue;
	}

private:
	PxMat33		mR;					// rotation matrix
	PxMat33		mAR;				// absolute rotation matrix
	PxVec3		mT;					// translation from obb space to model space
	PxVec3		mExtents;

	PxVec3		mBB_xyz;

	float		mBB_1, mBB_2, mBB_3;
	float		mBB_4, mBB_5, mBB_6;
	float		mBB_7, mBB_8, mBB_9;
};

#ifdef USE_SIMD
typedef OBBAABBTest_SIMD	BucketPrunerOBBAABBTest;
#else
typedef OBBAABBTest_Scalar	BucketPrunerOBBAABBTest;
#endif

///////////////////////////////////////////////////////////////////////////////

PxAgain	BucketPrunerCore::overlap(const ShapeData& queryVolume, PrunerCallback& pcb) const
{
	PX_ASSERT(!mDirty);
	PxAgain again = true;

	switch(queryVolume.getOriginalPxGeom().getType())
	{
	case PxGeometryType::eBOX:
		{
			if(PxAbs(queryVolume.getPrunerWorldTransform().q.w) < 0.999999f)
			{	
				const PxBounds3& cullBox = queryVolume.getPrunerInflatedWorldAABB();
				BucketPrunerOverlapTraversal<BucketPrunerOBBAABBTest, false> overlap;
				again = overlap(*this,
							BucketPrunerOBBAABBTest(
								queryVolume.getPrunerWorldRot33(), queryVolume.getPrunerWorldTransform().p,
								queryVolume.getPrunerBoxGeom().halfExtents*SQ_PRUNER_INFLATION),
							pcb, cullBox);
			}
			else
			{
				const PxBounds3& box = queryVolume.getPrunerInflatedWorldAABB();
				BucketPrunerOverlapTraversal<BucketPrunerAABBAABBTest, true> overlap;
				again = overlap(*this, BucketPrunerAABBAABBTest(box), pcb, box);
			}
		}
		break;
	case PxGeometryType::eCAPSULE:
		{
			const PxBounds3& cullBox = queryVolume.getPrunerInflatedWorldAABB();
			BucketPrunerOverlapTraversal<BucketPrunerOBBAABBTest, false> overlap;
			again = overlap(*this,
						BucketPrunerOBBAABBTest(
							queryVolume.getPrunerWorldRot33(), queryVolume.getPrunerWorldTransform().p,
							queryVolume.getPrunerBoxGeom().halfExtents*SQ_PRUNER_INFLATION),
						pcb, cullBox);
		}
		break;
	case PxGeometryType::eSPHERE:
		{
			const Sphere& sphere = queryVolume.getNPGuSphere();
			const PxVec3 sphereExtents(sphere.radius);
			BucketPrunerOverlapTraversal<BucketPrunerSphereAABBTest, true> overlap;
			const PxBounds3& cullBox = queryVolume.getPrunerInflatedWorldAABB();
			again = overlap(*this, BucketPrunerSphereAABBTest(sphere), pcb, cullBox);
		}
		break;
	case PxGeometryType::eCONVEXMESH:
		{
			const PxBounds3& cullBox = queryVolume.getPrunerInflatedWorldAABB();
			BucketPrunerOverlapTraversal<BucketPrunerOBBAABBTest, false> overlap;
			again = overlap(*this,
						BucketPrunerOBBAABBTest(
							queryVolume.getPrunerWorldRot33(), queryVolume.getPrunerWorldTransform().p,
							queryVolume.getPrunerBoxGeom().halfExtents*SQ_PRUNER_INFLATION),
						pcb, cullBox);
		}
		break;
	case PxGeometryType::ePLANE:
	case PxGeometryType::eTRIANGLEMESH:
	case PxGeometryType::eHEIGHTFIELD:
	case PxGeometryType::eGEOMETRY_COUNT:
	case PxGeometryType::eINVALID:
	default:
		PX_ALWAYS_ASSERT_MESSAGE("unsupported overlap query volume geometry type");
	}
	return again;
}

///////////////////////////////////////////////////////////////////////////////

void BucketPrunerCore::shiftOrigin(const PxVec3& shift)
{
	for(PxU32 i=0;i<mNbFree;i++)
	{
		mFreeBounds[i].minimum -= shift;
		mFreeBounds[i].maximum -= shift;
	}

	const PxU32 nb = mCoreNbObjects;
	//if (nb)
	{
		mGlobalBox.mCenter -= shift;
#ifndef PX_PS3
	#ifdef _DEBUG
		mGlobalBox.mDebugMin -= shift[mSortAxis];
	#endif
#endif
		encodeBoxMinMax(mGlobalBox, mSortAxis);

		for(PxU32 i=0; i < nb; i++)
		{
			mCoreBoxes[i].minimum -= shift;
			mCoreBoxes[i].maximum -= shift;
		}

		for(PxU32 i=0; i < mSortedNb; i++)
		{
			mSortedWorldBoxes[i].mCenter -= shift;
#ifndef PX_PS3
	#ifdef _DEBUG
			mSortedWorldBoxes[i].mDebugMin -= shift[mSortAxis];
	#endif
#endif
			encodeBoxMinMax(mSortedWorldBoxes[i], mSortAxis);
		}

		for(PxU32 i=0; i < 5; i++)
			mLevel1.mBucketBox[i].mCenter -= shift;

		for(PxU32 i=0; i < 5; i++)
			for(PxU32 j=0; j < 5; j++)
				mLevel2[i].mBucketBox[j].mCenter -= shift;

		for(PxU32 i=0; i < 5; i++)
			for(PxU32 j=0; j < 5; j++)
				for(PxU32 k=0; k < 5; k++)
					mLevel3[i][j].mBucketBox[k].mCenter -= shift;
	}
}

///////////////////////////////////////////////////////////////////////////////

static void visualize(Cm::RenderOutput& out, const BucketBox& bounds)
{
	out << Cm::DebugBox(PxBounds3(bounds.getMin(), bounds.getMax()), true);
}

void BucketPrunerCore::visualize(Cm::RenderOutput& out, PxU32 color) const
{
	const PxTransform idt = PxTransform(PxIdentity);
	out << idt;
	out << color;

	::visualize(out, mGlobalBox);

	for(PxU32 i=0;i<5;i++)
	{
		if(!mLevel1.mCounters[i])
			continue;

		::visualize(out, mLevel1.mBucketBox[i]);

		for(PxU32 j=0;j<5;j++)
		{
			if(!mLevel2[i].mCounters[j])
				continue;
				
			::visualize(out, mLevel2[i].mBucketBox[j]);

			for(PxU32 k=0;k<5;k++)
			{
				if(!mLevel3[i][j].mCounters[k])
					continue;

				::visualize(out, mLevel3[i][j].mBucketBox[k]);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BucketPruner::BucketPruner()
{
}

BucketPruner::~BucketPruner()
{
}

bool BucketPruner::addObjects(PrunerHandle* results, const PxBounds3* bounds, const PrunerPayload* payload, PxU32 count)
{
	PxU32 valid = 0;
	for(PxU32 i=0;i<count;i++)
	{
		PrunerHandle h = mPool.addObject(bounds[i], payload[i]);
		results[i] = h;
		if(h == INVALID_PRUNERHANDLE)
			break;

		mCore.mDirty = true;
		valid++;
	}

	mCore.setExternalMemory(mPool.getNbActiveObjects(), mPool.getCurrentWorldBoxes(), mPool.getObjects());

	return valid == count;
}

void BucketPruner::removeObjects(const PrunerHandle* handles, PxU32 count)
{
	for(PxU32 i=0;i<count;i++)
	{
		mPool.removeObject(handles[i]);
	}
	mCore.setExternalMemory(mPool.getNbActiveObjects(), mPool.getCurrentWorldBoxes(), mPool.getObjects());
	mCore.mDirty = true;
}

void BucketPruner::updateObjects(const PrunerHandle* handles, const PxBounds3* newBounds, PxU32 count)
{
	for(PxU32 i=0;i<count;i++)
	{
		mPool.updateObject(handles[i],newBounds[i]);
	}
	mCore.setExternalMemory(mPool.getNbActiveObjects(), mPool.getCurrentWorldBoxes(), mPool.getObjects());
	mCore.mDirty = true;
}

void BucketPruner::commit()
{
	mCore.build();
}

void BucketPruner::shiftOrigin(const PxVec3& shift)
{
	mCore.shiftOrigin(shift);
}

PxAgain BucketPruner::sweep(const ShapeData& queryVolume, const PxVec3& unitDir, PxReal& inOutDistance, PrunerCallback& pcb) const
{
	PX_ASSERT(!mCore.mDirty);
	if(mCore.mDirty)
		return true; // it may crash otherwise
	return mCore.sweep(queryVolume, unitDir, inOutDistance, pcb);
}

PxAgain BucketPruner::overlap(const ShapeData& queryVolume, PrunerCallback& pcb) const
{
	PX_ASSERT(!mCore.mDirty);
	if(mCore.mDirty)
		return true; // it may crash otherwise
	return mCore.overlap(queryVolume, pcb);
}

PxAgain BucketPruner::raycast(const PxVec3& origin, const PxVec3& unitDir, PxReal& inOutDistance, PrunerCallback& pcb) const
{
	PX_ASSERT(!mCore.mDirty);
	if(mCore.mDirty)
		return true; // it may crash otherwise
	return mCore.raycast(origin, unitDir, inOutDistance, pcb);
}

void BucketPruner::visualize(Cm::RenderOutput& out, PxU32 color) const
{
	mCore.visualize(out, color);
}

