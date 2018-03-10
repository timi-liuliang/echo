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

#pragma once

#ifndef CU_SOLVER_KERNEL_CU
#error include CuCollision.h only from CuSolverKernel.cu
#endif

#include "IndexPair.h"
#include <cfloat>

// no __ballot, don't use acceleration structure
#define USE_ACCELERATION_STRUCTURE __CUDA_ARCH__ >= 200

#define UINT16_MAX 0xffff

namespace
{
	struct CuCollision
	{
		struct ShapeMask 
		{
			uint32_t mSpheres;
			uint32_t mCones;

			__device__ friend ShapeMask& operator&=(ShapeMask& left, const ShapeMask& right)
			{
				left.mSpheres = left.mSpheres & right.mSpheres;
				left.mCones = left.mCones & right.mCones;
				return left;
			}
		};

		struct CollisionData
		{
			SharedPointer<float>::Type mSphereX;
			SharedPointer<float>::Type mSphereY;
			SharedPointer<float>::Type mSphereZ;
			SharedPointer<float>::Type mSphereW;

			SharedPointer<float>::Type mConeCenterX;
			SharedPointer<float>::Type mConeCenterY;
			SharedPointer<float>::Type mConeCenterZ;
			SharedPointer<float>::Type mConeRadius;
			SharedPointer<float>::Type mConeAxisX;
			SharedPointer<float>::Type mConeAxisY;
			SharedPointer<float>::Type mConeAxisZ;
			SharedPointer<float>::Type mConeSlope;
			SharedPointer<float>::Type mConeSqrCosine;
			SharedPointer<float>::Type mConeHalfLength;
		};

	public:

		__device__ CuCollision(SharedPointer<uint32_t>::Type);

		template <typename CurrentT, typename PreviousT>
		__device__ void operator()(CurrentT& current, PreviousT& previous, float alpha);

	private:

		__device__ void buildSphereAcceleration(const CollisionData&);
		__device__ void buildConeAcceleration();
		__device__ void mergeAcceleration();

		template <typename CurrentT>
		__device__ bool buildAcceleration(const CurrentT&, float);

		__device__ static ShapeMask getShapeMask(const float&, SharedPointer<const uint32_t>::Type);
		__device__ ShapeMask getShapeMask(const float*) const;
		__device__ ShapeMask getShapeMask(const float*, const float*) const;

		// todo: bank conflicts due to passing as fixed sized c-array!
		__device__ uint32_t collideCapsules(const float*, float*, float*) const;
		__device__ uint32_t collideCapsules(const float*, float*, float*, float*) const;

		template <typename CurrentT, typename PreviousT>
		__device__ void collideCapsules(CurrentT& current, PreviousT& previous) const;
		template <typename CurrentT, typename PreviousT>
		__device__ void collideVirtualCapsules(CurrentT& current, PreviousT& previous) const;
		template <typename CurrentT, typename PreviousT>
		__device__ void collideContinuousCapsules(CurrentT& current, PreviousT& previous) const;

		template <typename CurrentT, typename PreviousT>
		__device__ void collideConvexes(CurrentT& current, PreviousT& previous, float alpha);
		__device__ uint32_t collideConvexes(const float*, float*) const;

		template <typename CurrentT>
		__device__ void collideTriangles(CurrentT& current, float alpha);
		template <typename CurrentT>
		__device__ void collideTriangles(CurrentT& current, uint32_t i);

	public:

		SharedPointer<uint32_t>::Type mCapsuleIndices;
		SharedPointer<uint32_t>::Type mCapsuleMasks;
		SharedPointer<uint32_t>::Type mConvexMasks;

		CollisionData mPrevData;
		CollisionData mCurData;

		// acceleration structure 
		SharedPointer<uint32_t>::Type mShapeGrid;
		float mGridScale[3];
		float mGridBias[3];
		static const uint32_t sGridSize = 8;
	};

	template <typename T>
	__device__ void swap(T& a, T& b)
	{
		T c = a;
		a = b; 
		b = c;
	}
}

__shared__ uninitialized<CuCollision> gCollideParticles;

namespace 
{
	// initializes one pointer past data!
	__device__ void allocate(CuCollision::CollisionData& data)
	{
		if(threadIdx.x < 15)
		{
			SharedPointer<float>::Type* ptr = &data.mSphereX;
			ptr[threadIdx.x] = *ptr + threadIdx.x*gClothData.mNumCapsules
				+ min(threadIdx.x, 4) * (gClothData.mNumSpheres - gClothData.mNumCapsules);
		}
	}

	__device__ void generateSpheres(CuCollision::CollisionData& data, float alpha)
	{
		// interpolate spheres and transpose
		if(threadIdx.x < gClothData.mNumSpheres * 4)
		{
			float start = gFrameData.mStartCollisionSpheres[threadIdx.x];
			float target = gFrameData.mTargetCollisionSpheres[threadIdx.x];
			float value = start + (target - start) * alpha;
			if(threadIdx.x % 4 == 3)
				value = max(value, 0.0f);
			int32_t j = threadIdx.x % 4 * gClothData.mNumSpheres + threadIdx.x / 4;
			data.mSphereX[j] = value;
		}

		__syncthreads();
	}

	__device__ void generateCones(CuCollision::CollisionData& data,
		SharedPointer<const uint32_t>::Type iIt)
	{
		// generate cones
		if(threadIdx.x < gClothData.mNumCapsules)
		{
			uint32_t firstIndex = iIt[0];
			uint32_t secondIndex = iIt[1];

			float firstX = data.mSphereX[firstIndex];
			float firstY = data.mSphereY[firstIndex];
			float firstZ = data.mSphereZ[firstIndex];
			float firstW = data.mSphereW[firstIndex];

			float secondX = data.mSphereX[secondIndex];
			float secondY = data.mSphereY[secondIndex];
			float secondZ = data.mSphereZ[secondIndex];
			float secondW = data.mSphereW[secondIndex];

			float axisX = (secondX - firstX) * 0.5f;
			float axisY = (secondY - firstY) * 0.5f;
			float axisZ = (secondZ - firstZ) * 0.5f;
			float axisW = (secondW - firstW) * 0.5f;

			float sqrAxisLength = axisX*axisX + axisY*axisY + axisZ*axisZ;
			float sqrConeLength = sqrAxisLength - axisW*axisW;

			float invAxisLength = rsqrtf(sqrAxisLength);
			float invConeLength = rsqrtf(sqrConeLength);

			if(sqrConeLength <= 0.0f)
				invAxisLength = invConeLength = 0.0f;

			float axisLength = sqrAxisLength * invAxisLength;

			data.mConeCenterX[threadIdx.x] = (secondX + firstX) * 0.5f;
			data.mConeCenterY[threadIdx.x] = (secondY + firstY) * 0.5f;
			data.mConeCenterZ[threadIdx.x] = (secondZ + firstZ) * 0.5f;
			data.mConeRadius[threadIdx.x] = (axisW + firstW) * invConeLength * axisLength;

			data.mConeAxisX[threadIdx.x] = axisX * invAxisLength;
			data.mConeAxisY[threadIdx.x] = axisY * invAxisLength;
			data.mConeAxisZ[threadIdx.x] = axisZ * invAxisLength;
			data.mConeSlope[threadIdx.x] = axisW * invConeLength;

			float sine = axisW * invAxisLength;
			data.mConeSqrCosine[threadIdx.x] = 1 - sine * sine;
			data.mConeHalfLength[threadIdx.x] = axisLength;
		}

		__syncthreads();
	}
}

__device__ CuCollision::CuCollision(SharedPointer<uint32_t>::Type scratchPtr)
{
	uint32_t numCapsules2 = 2*gClothData.mNumCapsules;
	uint32_t numCapsules4 = 4*gClothData.mNumCapsules;
	uint32_t numConvexes = gClothData.mNumConvexes;

	if(threadIdx.x < 3)
	{
		(&mCapsuleIndices)[threadIdx.x] = scratchPtr + threadIdx.x * numCapsules2;
		(&mShapeGrid)[-int32_t(threadIdx.x*14)] = scratchPtr + numCapsules4 + numConvexes;
	}

	SharedPointer<uint32_t>::Type indexPtr = scratchPtr + threadIdx.x;
	if(threadIdx.x < numCapsules2)
	{
		uint32_t index = (&gClothData.mCapsuleIndices->first)[threadIdx.x];
		*indexPtr = index;

		SharedPointer<volatile uint32_t>::Type maskPtr = indexPtr + numCapsules2;
		*maskPtr = 1u << index;
		*maskPtr |= maskPtr[-int32_t(threadIdx.x&1)];
	}
	indexPtr += numCapsules4;

	if(threadIdx.x < numConvexes)
		*indexPtr = gClothData.mConvexMasks[threadIdx.x];

	if(gClothData.mEnableContinuousCollision || gClothData.mFrictionScale > 0.0f)
	{
		allocate(mPrevData);

		__syncthreads(); // mPrevData raw hazard

		generateSpheres(mPrevData, 0.0f);
		generateCones(mPrevData, mCapsuleIndices + 2*threadIdx.x);
	}

	allocate(mCurData); // also initializes mShapeGrid (!)
}

template <typename CurrentT, typename PreviousT>
__device__ void CuCollision::operator()(CurrentT& current, PreviousT& previous, float alpha)
{
	ProfileDetailZone zone(cloth::CuProfileZoneIds::COLLIDE);

	// if(current.w > 0) current.w = previous.w (see SwSolverKernel::computeBounds())
	for(uint32_t i=threadIdx.x; i<gClothData.mNumParticles; i+=blockDim.x)
	{
		if(current(i, 3) > 0.0f)
			current(i, 3) = previous(i, 3);
	}

	collideConvexes(current, previous, alpha);
	collideTriangles(current, alpha);

	if(buildAcceleration(current, alpha))
	{
		if(gClothData.mEnableContinuousCollision)
			collideContinuousCapsules(current, previous);
		else
			collideCapsules(current, previous);

		collideVirtualCapsules(current, previous);
	}

	// sync otherwise first threads overwrite sphere data before
	// remaining ones have had a chance to use it leading to incorrect
	// velocity calculation for friction / ccd

	__syncthreads();

	if(gClothData.mEnableContinuousCollision || gClothData.mFrictionScale > 0.0f)
	{
		// store current collision data for next iteration
		SharedPointer<float>::Type dstIt = mPrevData.mSphereX + threadIdx.x;
		SharedPointer<const float>::Type srcIt = mCurData.mSphereX + threadIdx.x;
		for(; dstIt < mCurData.mSphereX; dstIt += blockDim.x, srcIt += blockDim.x)
			*dstIt = *srcIt;
	}

	// __syncthreads() called in updateSleepState()
}

#if USE_ACCELERATION_STRUCTURE

// build per-axis mask arrays of spheres on the right/left of grid cell
__device__ void CuCollision::buildSphereAcceleration(const CollisionData& data)
{
	if (threadIdx.x >= 192)
		return;

	uint32_t sphereIdx = threadIdx.x & 31; 
	uint32_t axisIdx = threadIdx.x >> 6; // coordinate index (x, y, or z)
	uint32_t signi = threadIdx.x << 26 & 0x80000000; // sign bit (min or max)

	float signf = copysignf(1.0f, reinterpret_cast<const float&>(signi));
	float pos = signf * data.mSphereW[sphereIdx] + 
		data.mSphereX[sphereIdx + gClothData.mNumSpheres*axisIdx];

	// use overflow so we can test for non-positive
	// 'volatile' to work around compiler bug (merging 'dst+i' and 'index' incorrectly)
	volatile int32_t index = signi - int32_t(floorf(
		pos * mGridScale[axisIdx] + mGridBias[axisIdx]));

	uint32_t* dst = mShapeGrid + sGridSize * ((signi>>31)*3 + axisIdx);
	// #pragma unroll
	for(uint32_t i=0; i<sGridSize; ++i, ++index)
		dst[i] |= __ballot(index <= 0);
}

// generate cone masks from sphere masks
__device__ void CuCollision::buildConeAcceleration()
{
	if (threadIdx.x >= 192)
		return;

	uint32_t coneIdx = threadIdx.x & 31; 

	uint32_t sphereMask = mCurData.mConeRadius[coneIdx] && coneIdx 
		< gClothData.mNumCapsules ? mCapsuleMasks[2*coneIdx+1] : 0;
		
	uint32_t offset = threadIdx.x / 32 * sGridSize;
	uint32_t* src = mShapeGrid + offset;
	uint32_t* dst = src + 6*sGridSize;

	// #pragma unroll
	for(uint32_t i=0; i<sGridSize; ++i)
		dst[i] |= __ballot(src[i] & sphereMask);
}

// convert right/left mask arrays into single overlap array
__device__ void CuCollision::mergeAcceleration()
{
	if(threadIdx.x < sGridSize*12)
	{
		uint32_t* dst = mShapeGrid + threadIdx.x;
		if (!(gClothData.mEnableContinuousCollision || threadIdx.x*43 & 1024))
			*dst &= dst[sGridSize*3]; // above is same as 'threadIdx.x/24 & 1'

		// mask garbage bits from build*Acceleration
		uint32_t shapeIdx = threadIdx.x>=sGridSize*6; // spheres=0, cones=1
		*dst &= (1<<(&gClothData.mNumSpheres)[shapeIdx]) - 1;
	}
}
#endif

namespace
{
#if __CUDA_ARCH__ >= 300
	__device__ float mergeBounds(SharedPointer<float>::Type buffer)
	{
		float value = *buffer;
		value = max(value, __shfl_down(value, 1));
		value = max(value, __shfl_down(value, 2));
		value = max(value, __shfl_down(value, 4));
		value = max(value, __shfl_down(value, 8));
		return max(value, __shfl_down(value, 16));
	}
#else
	__device__ float mergeBounds(SharedPointer<volatile float>::Type buffer)
	{
		// ensure that writes to buffer are visible to all threads
		__threadfence_block();

		*buffer = max(*buffer, buffer[16]);
		*buffer = max(*buffer, buffer[ 8]);
		*buffer = max(*buffer, buffer[ 4]);
		*buffer = max(*buffer, buffer[ 2]);
		return    max(*buffer, buffer[ 1]);
	}
#endif
	// computes maxX, -minX, maxY, ... with a stride of 32, threadIdx.x must be < 192
	__device__ float computeSphereBounds(const CuCollision::CollisionData& data, SharedPointer<float>::Type buffer)
	{
		assert(threadIdx.x < 192);

		uint32_t sphereIdx = min(threadIdx.x & 31, gClothData.mNumSpheres-1); // sphere index
		uint32_t axisIdx = threadIdx.x >> 6; // coordinate index (x, y, or z)
		uint32_t signi = threadIdx.x << 26; // sign bit (min or max)
		float signf = copysignf(1.0f, reinterpret_cast<const float&>(signi));

		*buffer = data.mSphereW[sphereIdx] + signf * data.mSphereX[
			sphereIdx + gClothData.mNumSpheres*axisIdx];
		
		return mergeBounds(buffer);

	}

#if __CUDA_ARCH__ >= 300
	template <typename CurrentT>
	__device__ float computeParticleBounds(const CurrentT& current, SharedPointer<float>::Type buffer)
	{
		uint32_t numThreadsPerAxis = blockDim.x * 342 >> 10 & ~31; // same as / 3
		uint32_t axis = (threadIdx.x >= numThreadsPerAxis) + (threadIdx.x >= 2*numThreadsPerAxis);
		uint32_t threadIdxInAxis = threadIdx.x - axis*numThreadsPerAxis;
		int laneIdx = threadIdx.x & 31;

		if(threadIdxInAxis < numThreadsPerAxis)
		{
			typename CurrentT::ConstPointerType posIt = current[axis];
			uint32_t i = min(threadIdxInAxis, gClothData.mNumParticles-1);
			float minX = posIt[i], maxX = minX;
			while(i += numThreadsPerAxis, i < gClothData.mNumParticles)
			{
				float posX = posIt[i];
				minX = min(minX, posX);
				maxX = max(maxX, posX);
			}

			minX = min(minX, __shfl_down(minX,  1));
			maxX = max(maxX, __shfl_down(maxX,  1));
			minX = min(minX, __shfl_down(minX,  2));
			maxX = max(maxX, __shfl_down(maxX,  2));
			minX = min(minX, __shfl_down(minX,  4));
			maxX = max(maxX, __shfl_down(maxX,  4));
			minX = min(minX, __shfl_down(minX,  8));
			maxX = max(maxX, __shfl_down(maxX,  8));
			minX = min(minX, __shfl_down(minX, 16));
			maxX = max(maxX, __shfl_down(maxX, 16));

			if(!laneIdx)
			{
				SharedPointer<float>::Type dst = buffer - threadIdx.x 
					+ (threadIdxInAxis >> 5) + (axis << 6);
				dst[  0] =  maxX;
				dst[ 32] = -minX;
			}
		}

		__syncthreads();

		if(threadIdx.x >= 192)
			return 0.0f;

		float value = *buffer;
		if(laneIdx >= (numThreadsPerAxis >> 5))
			value = -FLT_MAX;

		// blockDim.x <= 3*512, increase to 3*1024 by adding a shfl by 16
		assert(numThreadsPerAxis <= 16*32);

		value = max(value, __shfl_down(value, 1));
		value = max(value, __shfl_down(value, 2));
		value = max(value, __shfl_down(value, 4));
		return  max(value, __shfl_down(value, 8));
	}
#else
	template <typename CurrentT>
	__device__ float computeParticleBounds(const CurrentT& current, SharedPointer<float>::Type buffer)
	{
		if(threadIdx.x >= 192)
			return 0.0f;

		uint32_t axisIdx = threadIdx.x >> 6; // x, y, or z
		uint32_t signi = threadIdx.x << 26; // sign bit (min or max)
		float signf = copysignf(1.0f, reinterpret_cast<const float&>(signi));

		typename CurrentT::ConstPointerType pIt = current[axisIdx];
		typename CurrentT::ConstPointerType pEnd = pIt + gClothData.mNumParticles;
		pIt += min(threadIdx.x & 31, gClothData.mNumParticles-1);

		*buffer = *pIt * signf;
		while(pIt += 32, pIt < pEnd)
			*buffer = max(*buffer, *pIt * signf);

		return mergeBounds(buffer);
	}
#endif
}

// build mask of spheres/cones touching a regular grid along each axis
template <typename CurrentT>
__device__ bool CuCollision::buildAcceleration(const CurrentT& current, float alpha)
{
	ProfileDetailZone zone(cloth::CuProfileZoneIds::COLLIDE_ACCELERATION);

	// use still unused cone data as buffer for bounds computation
	SharedPointer<float>::Type buffer = mCurData.mConeCenterX + threadIdx.x;
	float curParticleBounds = computeParticleBounds(current, buffer);
	uint32_t warpIdx = threadIdx.x >> 5;

	if(!gClothData.mNumSpheres)
	{
		if(threadIdx.x < 192 && !(threadIdx.x & 31))
			gFrameData.mParticleBounds[warpIdx] = curParticleBounds;
		return false;
	}

	generateSpheres(mCurData, alpha);

	if(threadIdx.x < 192)
	{
		float sphereBounds = computeSphereBounds(mCurData, buffer);
		float particleBounds = curParticleBounds;
		if(gClothData.mEnableContinuousCollision)
		{
			sphereBounds = max(sphereBounds, computeSphereBounds(mPrevData, buffer));
			float prevParticleBounds = gFrameData.mParticleBounds[warpIdx];
			particleBounds = max(particleBounds, prevParticleBounds);
		}

		float bounds = min(sphereBounds, particleBounds);
		float expandedBounds = bounds + abs(bounds) * 1e-4f;

		// store bounds data in shared memory
		if(!(threadIdx.x & 31))
		{
			mGridScale[warpIdx] = expandedBounds;
			gFrameData.mParticleBounds[warpIdx] = curParticleBounds;
		}
	}

	__syncthreads(); // mGridScale raw hazard
	
	if(threadIdx.x < 3)
	{
		float negativeLower = mGridScale[threadIdx.x*2+1];
		float edgeLength = mGridScale[threadIdx.x*2] + negativeLower;
		float divisor = max(edgeLength, FLT_EPSILON);
		mGridScale[threadIdx.x] = __fdividef(sGridSize - 1e-3, divisor);
		mGridBias[threadIdx.x] = negativeLower * mGridScale[threadIdx.x];
		if(edgeLength < 0.0f) mGridScale[0] = 0.0f; // mark empty intersection
	}

	// initialize sphere *and* cone grid to 0
	if(threadIdx.x < 2*6*sGridSize)
		mShapeGrid[threadIdx.x] = 0;

	__syncthreads(); // mGridScale raw hazard

	// generate cones even if test below fails because 
	// continuous collision might need it in next iteration
	generateCones(mCurData, mCapsuleIndices + 2*threadIdx.x);

	if(mGridScale[0] == 0.0f)
		return false; // early out for empty intersection

#if USE_ACCELERATION_STRUCTURE
	if(gClothData.mEnableContinuousCollision)
		buildSphereAcceleration(mPrevData);
	buildSphereAcceleration(mCurData);
	__syncthreads(); // mCurData raw hazard

	buildConeAcceleration();
	__syncthreads(); // mShapeGrid raw hazard

	mergeAcceleration();
#endif
	__syncthreads(); // mShapeGrid raw hazard

	return true;
}

__device__ CuCollision::ShapeMask CuCollision::getShapeMask(
	const float& position, SharedPointer<const uint32_t>::Type sphereGrid)
{
	ShapeMask result;
#if USE_ACCELERATION_STRUCTURE
	uint32_t index = int32_t(floorf(position));
	uint32_t outMask = (index < sGridSize) - 1;

	const uint32_t* gridPtr = sphereGrid + (index & sGridSize-1);
	result.mSpheres = gridPtr[0] & ~outMask;
	result.mCones = gridPtr[sGridSize*6] & ~outMask;
#else
	result.mSpheres = (0x1 << gClothData.mNumSpheres)-1;
	result.mCones = (0x1 << gClothData.mNumCapsules)-1;
#endif

	return result;
}

// lookup acceleration structure and return mask of potential intersectors
__device__ CuCollision::ShapeMask CuCollision::getShapeMask(const float* positions) const
{
	ShapeMask result;

	result  = getShapeMask(positions[0] * mGridScale[0] + mGridBias[0], mShapeGrid);
	result &= getShapeMask(positions[1] * mGridScale[1] + mGridBias[1], mShapeGrid + 8);
	result &= getShapeMask(positions[2] * mGridScale[2] + mGridBias[2], mShapeGrid + 16);

	return result;
}

__device__ CuCollision::ShapeMask CuCollision::getShapeMask(const float* prevPos, const float* curPos) const
{
	ShapeMask result;

	float prevX = prevPos[0] * mGridScale[0] + mGridBias[0];
	float prevY = prevPos[1] * mGridScale[1] + mGridBias[1];
	float prevZ = prevPos[2] * mGridScale[2] + mGridBias[2];

	float curX = curPos[0] * mGridScale[0] + mGridBias[0];
	float curY = curPos[1] * mGridScale[1] + mGridBias[1];
	float curZ = curPos[2] * mGridScale[2] + mGridBias[2];

	float maxX = min(max(prevX, curX), 7.0f);
	float maxY = min(max(prevY, curY), 7.0f);
	float maxZ = min(max(prevZ, curZ), 7.0f);

	result  = getShapeMask(maxX, mShapeGrid);
	result &= getShapeMask(maxY, mShapeGrid + 8);
	result &= getShapeMask(maxZ, mShapeGrid + 16);

	float minX = max(min(prevX, curX), 0.0f);
	float minY = max(min(prevY, curY), 0.0f);
	float minZ = max(min(prevZ, curZ), 0.0f);

	result &= getShapeMask(minX, mShapeGrid + 24);
	result &= getShapeMask(minY, mShapeGrid + 32);
	result &= getShapeMask(minZ, mShapeGrid + 40);

	return result;
}

__device__ uint32_t CuCollision::collideCapsules(const float* positions, float* delta, float* velocity) const
{
	ShapeMask shapeMask = getShapeMask(positions);

	delta[0] = delta[1] = delta[2] = 0.0f;
	velocity[0] = velocity[1] = velocity[2] = 0.0f;

	uint32_t numCollisions = 0;

	bool frictionEnabled = gClothData.mFrictionScale > 0.0f;

	// cone collision
	for(; shapeMask.mCones; shapeMask.mCones &= shapeMask.mCones - 1)
	{
		uint32_t j = __ffs(shapeMask.mCones) - 1;

#if !(USE_ACCELERATION_STRUCTURE)
		if(mCurData.mConeRadius[j] == 0.0f)
			continue;
#endif

		float deltaX = positions[0] - mCurData.mConeCenterX[j];
		float deltaY = positions[1] - mCurData.mConeCenterY[j];
		float deltaZ = positions[2] - mCurData.mConeCenterZ[j];

		float axisX = mCurData.mConeAxisX[j];
		float axisY = mCurData.mConeAxisY[j];
		float axisZ = mCurData.mConeAxisZ[j];
		float slope = mCurData.mConeSlope[j];

		float dot = deltaX*axisX + deltaY*axisY + deltaZ*axisZ;
		float radius = max(dot * slope + mCurData.mConeRadius[j], 0.0f);
		float sqrDistance = deltaX*deltaX + deltaY*deltaY + deltaZ*deltaZ - dot*dot;

		SharedPointer<const uint32_t>::Type mIt = mCapsuleMasks + 2*j;
		uint32_t bothMask = mIt[1];

		if(sqrDistance > radius * radius)
		{
			shapeMask.mSpheres &= ~bothMask;
			continue;
		}

		sqrDistance = max(sqrDistance, FLT_EPSILON);
		float invDistance = rsqrtf(sqrDistance);

		float base = dot + slope * sqrDistance * invDistance;

		float halfLength = mCurData.mConeHalfLength[j]; 
		uint32_t leftMask = base < -halfLength;
		uint32_t rightMask = base > halfLength;

		uint32_t firstMask = mIt[0];
		uint32_t secondMask = firstMask ^ bothMask;

		shapeMask.mSpheres &= ~(firstMask & leftMask-1);
		shapeMask.mSpheres &= ~(secondMask & rightMask-1);

		if(!leftMask && !rightMask)
		{
			deltaX = deltaX - base * axisX;
			deltaY = deltaY - base * axisY;
			deltaZ = deltaZ - base * axisZ;

			float sqrCosine = mCurData.mConeSqrCosine[j];
			float scale = radius * invDistance * sqrCosine - sqrCosine;

			delta[0] = delta[0] + deltaX * scale;
			delta[1] = delta[1] + deltaY * scale;
			delta[2] = delta[2] + deltaZ * scale;

			if (frictionEnabled)
			{
				uint32_t s0 = mCapsuleIndices[2*j];
				uint32_t s1 = mCapsuleIndices[2*j+1];

				// load previous sphere pos
				float s0vx = mCurData.mSphereX[s0] - mPrevData.mSphereX[s0];
				float s0vy = mCurData.mSphereY[s0] - mPrevData.mSphereY[s0];
				float s0vz = mCurData.mSphereZ[s0] - mPrevData.mSphereZ[s0];
	
				float s1vx = mCurData.mSphereX[s1] - mPrevData.mSphereX[s1];
				float s1vy = mCurData.mSphereY[s1] - mPrevData.mSphereY[s1];
				float s1vz = mCurData.mSphereZ[s1] - mPrevData.mSphereZ[s1];
				
				// interpolate velocity between the two spheres
				float t = dot*0.5f + 0.5f;

				velocity[0] += s0vx + t*(s1vx-s0vx);
				velocity[1] += s0vy + t*(s1vy-s0vy);
				velocity[2] += s0vz + t*(s1vz-s0vz);
			}

			++numCollisions;
		}
	}

	// sphere collision
	for(; shapeMask.mSpheres; shapeMask.mSpheres &= shapeMask.mSpheres - 1)
	{
		uint32_t j = __ffs(shapeMask.mSpheres) - 1;

		float deltaX = positions[0] - mCurData.mSphereX[j];
		float deltaY = positions[1] - mCurData.mSphereY[j];
		float deltaZ = positions[2] - mCurData.mSphereZ[j];

		float sqrDistance = FLT_EPSILON + deltaX*deltaX + deltaY*deltaY + deltaZ*deltaZ;
		float relDistance = rsqrtf(sqrDistance) * mCurData.mSphereW[j];

		if(relDistance > 1.0f)
		{
			float scale = relDistance - 1.0f;

			delta[0] = delta[0] + deltaX * scale;
			delta[1] = delta[1] + deltaY * scale;
			delta[2] = delta[2] + deltaZ * scale;

			if (frictionEnabled)
			{
				velocity[0] += mCurData.mSphereX[j] - mPrevData.mSphereX[j];
				velocity[1] += mCurData.mSphereY[j] - mPrevData.mSphereY[j];
				velocity[2] += mCurData.mSphereZ[j] - mPrevData.mSphereZ[j];
			}

			++numCollisions;
		}
	}

	return numCollisions;
}

static const __device__ float gSkeletonWidth = 0.95f;

__device__ uint32_t CuCollision::collideCapsules(const float* prevPos, float* curPos, float* delta, float* velocity) const
{
	ShapeMask shapeMask = getShapeMask(prevPos, curPos);

	delta[0] = delta[1] = delta[2] = 0.0f;
	velocity[0] = velocity[1] = velocity[2] = 0.0f;

	uint32_t numCollisions = 0;
	bool frictionEnabled = gClothData.mFrictionScale > 0.0f;

	// cone collision
	for(; shapeMask.mCones; shapeMask.mCones &= shapeMask.mCones - 1)
	{
		uint32_t j = __ffs(shapeMask.mCones) - 1;

#if !(USE_ACCELERATION_STRUCTURE)
		if(mCurData.mConeRadius[j] == 0.0f)
			continue;
#endif

		float prevAxisX = mPrevData.mConeAxisX[j];
		float prevAxisY = mPrevData.mConeAxisY[j];
		float prevAxisZ = mPrevData.mConeAxisZ[j];
		float prevSlope = mPrevData.mConeSlope[j];

		float prevX = prevPos[0] - mPrevData.mConeCenterX[j];
		float prevY = prevPos[1] - mPrevData.mConeCenterY[j];
		float prevZ = prevPos[2] - mPrevData.mConeCenterZ[j];
		float prevT = prevY*prevAxisZ - prevZ*prevAxisY;
		float prevU = prevZ*prevAxisX - prevX*prevAxisZ;
		float prevV = prevX*prevAxisY - prevY*prevAxisX;
		float prevDot = prevX*prevAxisX + prevY*prevAxisY + prevZ*prevAxisZ;
		float prevRadius = max(prevDot * prevSlope + mCurData.mConeRadius[j], 0.0f);

		float curAxisX = mCurData.mConeAxisX[j];
		float curAxisY = mCurData.mConeAxisY[j];
		float curAxisZ = mCurData.mConeAxisZ[j];
		float curSlope = mCurData.mConeSlope[j];

		float curX = curPos[0] - mCurData.mConeCenterX[j];
		float curY = curPos[1] - mCurData.mConeCenterY[j];
		float curZ = curPos[2] - mCurData.mConeCenterZ[j];
		float curT = curY*curAxisZ - curZ*curAxisY;
		float curU = curZ*curAxisX - curX*curAxisZ;
		float curV = curX*curAxisY - curY*curAxisX;
		float curDot = curX*curAxisX + curY*curAxisY + curZ*curAxisZ;
		float curRadius = max(curDot * curSlope + mCurData.mConeRadius[j], 0.0f);

		float curSqrDistance = FLT_EPSILON + curT*curT + curU*curU + curV*curV;

		// reduce radius to prevent CCD near surface
		float prevInnerRadius = prevRadius * gSkeletonWidth;
		float curInnerRadius = curRadius * gSkeletonWidth;

		float dotPrevPrev = prevT*prevT + prevU*prevU + prevV*prevV - prevInnerRadius*prevInnerRadius;
		float dotPrevCur = prevT*curT + prevU*curU + prevV*curV - prevInnerRadius*curInnerRadius;
		float dotCurCur = curSqrDistance - curInnerRadius*curInnerRadius;

		float d = dotPrevCur * dotPrevCur - dotCurCur * dotPrevPrev;
		float sqrtD = sqrtf(d);
		float halfB = dotPrevCur - dotPrevPrev;
		float minusA = dotPrevCur - dotCurCur + halfB;

		// time of impact or 0 if prevPos inside cone
		float toi = __fdividef(min(0.0f, halfB + sqrtD), minusA);
		bool hasCollision = toi < 1.0f && halfB < sqrtD;

		// a is negative when one cone is contained in the other,
		// which is already handled by discrete collision.
		hasCollision = hasCollision && minusA < -FLT_EPSILON;

		if(hasCollision)
		{
			float deltaX = prevX - curX;
			float deltaY = prevY - curY;
			float deltaZ = prevZ - curZ;

			// interpolate delta at toi
			float posX = prevX - deltaX * toi;
			float posY = prevY - deltaY * toi;
			float posZ = prevZ - deltaZ * toi;

			float curHalfLength = mCurData.mConeHalfLength[j];
			float curScaledAxisX = curAxisX * curHalfLength;
			float curScaledAxisY = curAxisY * curHalfLength;
			float curScaledAxisZ = curAxisZ * curHalfLength;

			float prevHalfLength = mPrevData.mConeHalfLength[j];
			float deltaScaledAxisX = curScaledAxisX - prevAxisX * prevHalfLength;
			float deltaScaledAxisY = curScaledAxisY - prevAxisY * prevHalfLength;
			float deltaScaledAxisZ = curScaledAxisZ - prevAxisZ * prevHalfLength;

			float oneMinusToi = 1.0f - toi;

			// interpolate axis at toi
			float axisX = curScaledAxisX - deltaScaledAxisX * oneMinusToi;
			float axisY = curScaledAxisY - deltaScaledAxisY * oneMinusToi;
			float axisZ = curScaledAxisZ - deltaScaledAxisZ * oneMinusToi;
			float slope = (prevSlope * oneMinusToi + curSlope * toi) * gSkeletonWidth;

			float sqrHalfLength = axisX*axisX + axisY*axisY + axisZ*axisZ;
			float invHalfLength = rsqrtf(sqrHalfLength);
			float dot = (posX*axisX + posY*axisY + posZ*axisZ) * invHalfLength;

			float sqrDistance = posX*posX + posY*posY + posZ*posZ - dot*dot;
			float invDistance = sqrDistance > 0.0f ? rsqrtf(sqrDistance) : 0.0f;

			float base = dot + slope * sqrDistance * invDistance;
			float scale = base * invHalfLength;

			if(abs(scale) < 1.0f)
			{
				deltaX = deltaX + deltaScaledAxisX * scale;
				deltaY = deltaY + deltaScaledAxisY * scale;
				deltaZ = deltaZ + deltaScaledAxisZ * scale;

				curX = curX + deltaX * oneMinusToi;
				curY = curY + deltaY * oneMinusToi;
				curZ = curZ + deltaZ * oneMinusToi;

				curDot = curX*curAxisX + curY*curAxisY + curZ*curAxisZ;
				curRadius = max(curDot * curSlope + mCurData.mConeRadius[j], 0.0f);
				curSqrDistance = curX*curX + curY*curY + curZ*curZ - curDot*curDot;

				curPos[0] = mCurData.mConeCenterX[j] + curX;
				curPos[1] = mCurData.mConeCenterY[j] + curY;
				curPos[2] = mCurData.mConeCenterZ[j] + curZ;
			}
		}

		// curPos inside cone (discrete collision)
		bool hasContact = curRadius*curRadius > curSqrDistance;

		SharedPointer<const uint32_t>::Type mIt = mCapsuleMasks + 2*j;
		uint32_t bothMask = mIt[1];

		uint32_t cullMask = bothMask & (hasCollision | hasContact)-1;
		shapeMask.mSpheres &= ~cullMask;

		if(!hasContact)
			continue;

		float invDistance = curSqrDistance > 0.0f ? rsqrtf(curSqrDistance) : 0.0f;
		float base = curDot + curSlope * curSqrDistance * invDistance;

		float halfLength = mCurData.mConeHalfLength[j]; 
		uint32_t leftMask = base < -halfLength;
		uint32_t rightMask = base > halfLength;

		// can't disable sphere collision if post-ccd position is on cone side
		// uint32_t firstMask = mIt[0];
		// uint32_t secondMask = firstMask ^ bothMask;
		// shapeMask.mSpheres &= ~(firstMask & leftMask-1);
		// shapeMask.mSpheres &= ~(secondMask & rightMask-1);

		if(!leftMask && !rightMask)
		{
			float deltaX = curX - base * curAxisX;
			float deltaY = curY - base * curAxisY;
			float deltaZ = curZ - base * curAxisZ;

			float sqrCosine = mCurData.mConeSqrCosine[j];
			float scale = curRadius * invDistance * sqrCosine - sqrCosine;

			delta[0] = delta[0] + deltaX * scale;
			delta[1] = delta[1] + deltaY * scale;
			delta[2] = delta[2] + deltaZ * scale;

			if (frictionEnabled)
			{
				uint32_t s0 = mCapsuleIndices[2*j];
				uint32_t s1 = mCapsuleIndices[2*j+1];

				// load previous sphere pos
				float s0vx = mCurData.mSphereX[s0] - mPrevData.mSphereX[s0];
				float s0vy = mCurData.mSphereY[s0] - mPrevData.mSphereY[s0];
				float s0vz = mCurData.mSphereZ[s0] - mPrevData.mSphereZ[s0];

				float s1vx = mCurData.mSphereX[s1] - mPrevData.mSphereX[s1];
				float s1vy = mCurData.mSphereY[s1] - mPrevData.mSphereY[s1];
				float s1vz = mCurData.mSphereZ[s1] - mPrevData.mSphereZ[s1];

				// interpolate velocity between the two spheres
				float t = curDot*0.5f + 0.5f;

				velocity[0] += s0vx + t*(s1vx-s0vx);
				velocity[1] += s0vy + t*(s1vy-s0vy);
				velocity[2] += s0vz + t*(s1vz-s0vz);
			}

			++numCollisions;
		}
	}

	// sphere collision
	for(; shapeMask.mSpheres; shapeMask.mSpheres &= shapeMask.mSpheres - 1)
	{
		uint32_t j = __ffs(shapeMask.mSpheres) - 1;

		float prevX = prevPos[0] - mPrevData.mSphereX[j];
		float prevY = prevPos[1] - mPrevData.mSphereY[j];
		float prevZ = prevPos[2] - mPrevData.mSphereZ[j];
		float prevRadius = mPrevData.mSphereW[j] * gSkeletonWidth;

		float curX = curPos[0] - mCurData.mSphereX[j];
		float curY = curPos[1] - mCurData.mSphereY[j];
		float curZ = curPos[2] - mCurData.mSphereZ[j];
		float curRadius = mCurData.mSphereW[j] * gSkeletonWidth;

		float sqrDistance = FLT_EPSILON + curX*curX + curY*curY + curZ*curZ;

		float dotPrevPrev = prevX*prevX + prevY*prevY + prevZ*prevZ - prevRadius*prevRadius;
		float dotPrevCur = prevX*curX + prevY*curY + prevZ*curZ - prevRadius*curRadius;
		float dotCurCur = sqrDistance - curRadius*curRadius;

		float sqrtD = sqrtf(dotPrevCur * dotPrevCur - dotCurCur * dotPrevPrev);
		float halfB = dotPrevCur - dotPrevPrev;
		float minusA = dotPrevCur - dotCurCur + halfB;

		// time of impact or 0 if prevPos inside sphere
		float toi = __fdividef(min(0.0f, halfB + sqrtD), minusA);
		bool hasCollision = toi < 1.0f && halfB < sqrtD;

		// a is negative when one cone is contained in the other,
		// which is already handled by discrete collision.
		hasCollision = hasCollision && minusA < -FLT_EPSILON;

		if(hasCollision)
		{
			float deltaX = prevX - curX;
			float deltaY = prevY - curY;
			float deltaZ = prevZ - curZ;

			float oneMinusToi = 1.0f - toi;

			curX = curX + deltaX * oneMinusToi;
			curY = curY + deltaY * oneMinusToi;
			curZ = curZ + deltaZ * oneMinusToi;

			curPos[0] = mCurData.mSphereX[j] + curX;
			curPos[1] = mCurData.mSphereY[j] + curY;
			curPos[2] = mCurData.mSphereZ[j] + curZ;

			sqrDistance = FLT_EPSILON + curX*curX + curY*curY + curZ*curZ;
		}

		float relDistance = rsqrtf(sqrDistance) * mCurData.mSphereW[j];

		if(relDistance > 1.0f)
		{
			float scale = relDistance - 1.0f;

			delta[0] = delta[0] + curX * scale;
			delta[1] = delta[1] + curY * scale;
			delta[2] = delta[2] + curZ * scale;

			if (frictionEnabled)
			{
				velocity[0] += mCurData.mSphereX[j] - mPrevData.mSphereX[j];
				velocity[1] += mCurData.mSphereY[j] - mPrevData.mSphereY[j];
				velocity[2] += mCurData.mSphereZ[j] - mPrevData.mSphereZ[j];
			}

			++numCollisions;
		}
	}

	return numCollisions;
}

namespace
{
	__device__ inline void calcFrictionImpulse(const float* curPos, const float* prevPos, 
		const float* shapeVelocity, float scale, const float* collisionImpulse, 
		float* frictionImpulse)
	{
		const float frictionScale = gClothData.mFrictionScale;

		// calculate collision normal
		float deltaSq = collisionImpulse[0]*collisionImpulse[0] +
						collisionImpulse[1]*collisionImpulse[1] + 
						collisionImpulse[2]*collisionImpulse[2];

		float rcpDelta = rsqrtf(deltaSq + FLT_EPSILON);

		float nx = collisionImpulse[0] * rcpDelta;
		float ny = collisionImpulse[1] * rcpDelta;
		float nz = collisionImpulse[2] * rcpDelta;

		// calculate relative velocity scaled by number of collision
		float rvx = curPos[0] - prevPos[0] - shapeVelocity[0] * scale;
		float rvy = curPos[1] - prevPos[1] - shapeVelocity[1] * scale;
		float rvz = curPos[2] - prevPos[2] - shapeVelocity[2] * scale;

		// calculate magnitude of relative normal velocity
		float rvn = rvx*nx + rvy*ny + rvz*nz;

		// calculate relative tangential velocity
		float rvtx = rvx - rvn*nx;
		float rvty = rvy - rvn*ny;
		float rvtz = rvz - rvn*nz;

		// calculate magnitude of vt
		float rcpVt = rsqrtf(rvtx*rvtx + rvty*rvty + rvtz*rvtz + FLT_EPSILON);

		// magnitude of friction impulse (cannot be larger than -|vt|)
		float j = max(-frictionScale*deltaSq*rcpDelta*scale*rcpVt, -1.0f);

		frictionImpulse[0] = rvtx*j;
		frictionImpulse[1] = rvty*j;
		frictionImpulse[2] = rvtz*j;
	}
}

template <typename CurrentT, typename PreviousT>
__device__ void CuCollision::collideCapsules(CurrentT& current, PreviousT& previous) const
{
	ProfileDetailZone zone(cloth::CuProfileZoneIds::COLLIDE_CAPSULES);

	bool frictionEnabled = gClothData.mFrictionScale > 0.0f;
	bool massScaleEnabled = gClothData.mCollisionMassScale > 0.0f;

	float curPos[3], prevPos[3], delta[3], velocity[3];
	for(uint32_t i = threadIdx.x; i < gClothData.mNumParticles; i += blockDim.x)
	{
		curPos[0] = current(i, 0);
		curPos[1] = current(i, 1);
		curPos[2] = current(i, 2);

		if(uint32_t numCollisions = collideCapsules(curPos, delta, velocity))
		{
			float scale = __fdividef(1.0f, numCollisions);

			current(i, 0) = curPos[0] + delta[0] * scale;
			current(i, 1) = curPos[1] + delta[1] * scale;
			current(i, 2) = curPos[2] + delta[2] * scale;

			if (frictionEnabled)
			{
				prevPos[0] = previous(i, 0);
				prevPos[1] = previous(i, 1);
				prevPos[2] = previous(i, 2);

				float frictionImpulse[3];
				calcFrictionImpulse(curPos, prevPos, velocity, scale, delta, frictionImpulse);

				previous(i, 0) = prevPos[0] - frictionImpulse[0];
				previous(i, 1) = prevPos[1] - frictionImpulse[1];
				previous(i, 2) = prevPos[2] - frictionImpulse[2];
			}			

			if(massScaleEnabled)
			{
				float deltaLengthSq = delta[0]*delta[0] + delta[1]*delta[1] + delta[2]*delta[2];
				float massScale = 1.0f + gClothData.mCollisionMassScale * deltaLengthSq * scale * scale;
				current(i, 3) = __fdividef(current(i, 3), massScale);
			}
		}
	}
}

namespace
{
	template <typename PointerT>
	__device__ float lerp(PointerT pos, const uint32_t* indices, const float* weights)
	{
		return pos[indices[0]] * weights[0] 
			 + pos[indices[1]] * weights[1] 
			 + pos[indices[2]] * weights[2];
	}

	template <typename PointerT>
	__device__ void apply(PointerT pos, const uint32_t* indices, const float* weights, float delta)
	{
		pos[indices[0]] += delta * weights[0];
		pos[indices[1]] += delta * weights[1];
		pos[indices[2]] += delta * weights[2];
	}
}

template <typename CurrentT, typename PreviousT>
__device__ void CuCollision::collideVirtualCapsules(CurrentT& current, PreviousT& previous) const
{
	const uint32_t* setSizeIt = gClothData.mVirtualParticleSetSizesBegin;

	if(!setSizeIt)
		return;

	if(gClothData.mEnableContinuousCollision)
	{
		// copied from mergeAcceleration
		SharedPointer<uint32_t>::Type dst = mShapeGrid + threadIdx.x;
		if (!(threadIdx.x*43 & 1024) && threadIdx.x < sGridSize*12)
			*dst &= dst[sGridSize*3]; 
		__syncthreads(); // mShapeGrid raw hazard
	}

	ProfileDetailZone zone(cloth::CuProfileZoneIds::COLLIDE_VIRTUAL_CAPSULES);

	const uint32_t* setSizeEnd = gClothData.mVirtualParticleSetSizesEnd;
	const uint16_t* indicesEnd = gClothData.mVirtualParticleIndices;
	const float* weightsIt = gClothData.mVirtualParticleWeights;

	bool frictionEnabled = gClothData.mFrictionScale > 0.0f;
	bool massScaleEnabled = gClothData.mCollisionMassScale > 0.0f;

	float curPos[3], prevPos[3], delta[3], velocity[3];
	for(; setSizeIt != setSizeEnd; ++setSizeIt)
	{
		__syncthreads();

		const uint16_t* indicesIt = indicesEnd + threadIdx.x * 4;
		for(indicesEnd += *setSizeIt * 4; indicesIt < indicesEnd; indicesIt += blockDim.x * 4)
		{

			// Expand uint16_t to uint32_t to remove bank conflicts
			uint32_t indices[4] = { indicesIt[0], 
				indicesIt[1], indicesIt[2], indicesIt[3] };

			const float* weights = weightsIt + indices[3] * 4;

			curPos[0] = lerp(current[0], indices, weights);
			curPos[1] = lerp(current[1], indices, weights);
			curPos[2] = lerp(current[2], indices, weights);

			if(uint32_t numCollisions = collideCapsules(curPos, delta, velocity))
			{
				float scale = __fdividef(1.0f, numCollisions);
				float wscale = weights[3]*scale;

				float dx = delta[0] * wscale;
				float dy = delta[1] * wscale;
				float dz = delta[2] * wscale;

				apply(current[0], indices, weights, dx);
				apply(current[1], indices, weights, dy);
				apply(current[2], indices, weights, dz);

				if (frictionEnabled)
				{
					prevPos[0] = lerp(previous[0], indices, weights);
					prevPos[1] = lerp(previous[1], indices, weights);
					prevPos[2] = lerp(previous[2], indices, weights);

					float frictionImpulse[3];
					calcFrictionImpulse(curPos, prevPos, velocity, scale, delta, frictionImpulse);

					apply(previous[0], indices, weights, -frictionImpulse[0]*weights[3]);
					apply(previous[1], indices, weights, -frictionImpulse[1]*weights[3]);
					apply(previous[2], indices, weights, -frictionImpulse[2]*weights[3]);
				}

				if(massScaleEnabled)
				{
					float deltaLengthSq = (delta[0]*delta[0] + delta[1]*delta[1] + delta[2]*delta[2])*scale*scale;
					float invMassScale = __fdividef(1.0f, 1.0f + 
						gClothData.mCollisionMassScale * deltaLengthSq);

					// not multiplying by weights[3] here because unlike applying velocity
					// deltas where we want the interpolated position to obtain a particular
					// value, we instead just require that the total change is equal to invMassScale
					invMassScale = invMassScale - 1.0f;
					current(indices[0], 3) *= 1.0f + weights[0]*invMassScale;
					current(indices[1], 3) *= 1.0f + weights[1]*invMassScale;
					current(indices[2], 3) *= 1.0f + weights[2]*invMassScale;
				}
			}
		}
	}
}

template <typename CurrentT, typename PreviousT>
__device__ void CuCollision::collideContinuousCapsules(CurrentT& current, PreviousT& previous) const
{
	ProfileDetailZone zone(cloth::CuProfileZoneIds::COLLIDE_CONTINUOUS_CAPSULES);

	bool frictionEnabled = gClothData.mFrictionScale > 0.0f;
	bool massScaleEnabled = gClothData.mCollisionMassScale > 0.0f;

	float prevPos[3], curPos[3], delta[3], velocity[3];
	for(uint32_t i = threadIdx.x; i < gClothData.mNumParticles; i += blockDim.x)
	{
		prevPos[0] = previous(i, 0);
		prevPos[1] = previous(i, 1);
		prevPos[2] = previous(i, 2);

		curPos[0] = current(i, 0);
		curPos[1] = current(i, 1);
		curPos[2] = current(i, 2);

		if(uint32_t numCollisions = collideCapsules(prevPos, curPos, delta, velocity))
		{
			float scale = __fdividef(1.0f, numCollisions);

			current(i, 0) = curPos[0] + delta[0] * scale;
			current(i, 1) = curPos[1] + delta[1] * scale;
			current(i, 2) = curPos[2] + delta[2] * scale; 

			if (frictionEnabled)
			{
				float frictionImpulse[3];
				calcFrictionImpulse(curPos, prevPos, velocity, scale, delta, frictionImpulse);

				previous(i, 0) = prevPos[0] - frictionImpulse[0];
				previous(i, 1) = prevPos[1] - frictionImpulse[1];
				previous(i, 2) = prevPos[2] - frictionImpulse[2];
			}			

			if(massScaleEnabled)
			{
				float deltaLengthSq = delta[0]*delta[0] + delta[1]*delta[1] + delta[2]*delta[2];
				float massScale = 1.0f + gClothData.mCollisionMassScale * deltaLengthSq * scale * scale;
				current(i, 3) = __fdividef(current(i, 3), massScale);
			}
		}
	}
}

__device__ uint32_t CuCollision::collideConvexes(const float* positions, float* delta) const
{
	ProfileDetailZone zone(cloth::CuProfileZoneIds::COLLIDE_CONVEXES);

	delta[0] = delta[1] = delta[2] = 0.0f;

	SharedPointer<const float>::Type planeX = mCurData.mSphereX;
	SharedPointer<const float>::Type planeY = planeX + gClothData.mNumPlanes;
	SharedPointer<const float>::Type planeZ = planeY + gClothData.mNumPlanes;
	SharedPointer<const float>::Type planeW = planeZ + gClothData.mNumPlanes;

	uint32_t numCollisions = 0;
	SharedPointer<const uint32_t>::Type cIt = mConvexMasks;
	SharedPointer<const uint32_t>::Type cEnd = cIt + gClothData.mNumConvexes;
	for(; cIt != cEnd; ++cIt)
	{
		uint32_t mask = *cIt;

		uint32_t maxIndex = __ffs(mask) - 1;
		float maxDist = planeW[maxIndex] + positions[2] * planeZ[maxIndex] +
			positions[1] * planeY[maxIndex] + positions[0] * planeX[maxIndex];

		while((maxDist < 0.0f) && (mask &= mask - 1))
		{
			uint32_t i = __ffs(mask) - 1;
			float dist = planeW[i] + positions[2] * planeZ[i] +
				positions[1] * planeY[i] + positions[0] * planeX[i];
			if(dist > maxDist)
				maxDist = dist, maxIndex = i;
		}

		if(maxDist < 0.0f)
		{
			delta[0] -= planeX[maxIndex] * maxDist;
			delta[1] -= planeY[maxIndex] * maxDist;
			delta[2] -= planeZ[maxIndex] * maxDist;

			++numCollisions;
		}
	}

	return numCollisions;
}

template <typename CurrentT, typename PreviousT>
__device__ void CuCollision::collideConvexes(CurrentT& current, PreviousT& previous, float alpha)
{
	if(!gClothData.mNumConvexes)
		return;

	// interpolate planes and transpose
	if(threadIdx.x < gClothData.mNumPlanes * 4)
	{
		float start = gFrameData.mStartCollisionPlanes[threadIdx.x];
		float target = gFrameData.mTargetCollisionPlanes[threadIdx.x];
		uint32_t j = threadIdx.x % 4 * gClothData.mNumPlanes + threadIdx.x / 4;
		mCurData.mSphereX[j] = start + (target - start) * alpha;
	}

	__syncthreads();

	bool frictionEnabled = gClothData.mFrictionScale > 0.0f;

	float curPos[3], prevPos[3], delta[3];
	for(uint32_t i = threadIdx.x; i < gClothData.mNumParticles; i += blockDim.x)
	{
		curPos[0] = current(i, 0);
		curPos[1] = current(i, 1);
		curPos[2] = current(i, 2);

		if(uint32_t numCollisions = collideConvexes(curPos, delta))
		{
			float scale = __fdividef(1.0f, numCollisions);

			current(i, 0) = curPos[0] + delta[0] * scale;
			current(i, 1) = curPos[1] + delta[1] * scale;
			current(i, 2) = curPos[2] + delta[2] * scale;

			if (frictionEnabled)
			{
				prevPos[0] = previous(i, 0);
				prevPos[1] = previous(i, 1);
				prevPos[2] = previous(i, 2);

				float frictionImpulse[3] = {};
				calcFrictionImpulse(curPos, prevPos, frictionImpulse, scale, delta, frictionImpulse);

				previous(i, 0) = prevPos[0] - frictionImpulse[0];
				previous(i, 1) = prevPos[1] - frictionImpulse[1];
				previous(i, 2) = prevPos[2] - frictionImpulse[2];
			}			

		}
	}

	__syncthreads();
}

namespace 
{
	struct TriangleData
	{
		float baseX, baseY, baseZ;
		float edge0X, edge0Y, edge0Z;
		float edge1X, edge1Y, edge1Z;
		float normalX, normalY, normalZ;

		float edge0DotEdge1;
		float edge0SqrLength;
		float edge1SqrLength;

		float det;
		float denom;

		float edge0InvSqrLength;
		float edge1InvSqrLength;

		// initialize struct after vertices have been stored in first 9 members
		__device__ void initialize()
		{
			edge0X -= baseX, edge0Y -= baseY, edge0Z -= baseZ;
			edge1X -= baseX, edge1Y -= baseY, edge1Z -= baseZ;

			normalX = edge0Y*edge1Z - edge0Z*edge1Y; 
			normalY = edge0Z*edge1X - edge0X*edge1Z; 
			normalZ = edge0X*edge1Y - edge0Y*edge1X;

			float normalInvLength = rsqrtf(normalX*normalX + normalY*normalY + normalZ*normalZ);
			normalX *= normalInvLength;
			normalY *= normalInvLength;
			normalZ *= normalInvLength;

			edge0DotEdge1  = edge0X*edge1X + edge0Y*edge1Y + edge0Z*edge1Z;
			edge0SqrLength = edge0X*edge0X + edge0Y*edge0Y + edge0Z*edge0Z;
			edge1SqrLength = edge1X*edge1X + edge1Y*edge1Y + edge1Z*edge1Z;

			det   = __fdividef(1.0f, edge0SqrLength*edge1SqrLength - edge0DotEdge1*edge0DotEdge1);
			denom = __fdividef(1.0f, edge0SqrLength+edge1SqrLength - edge0DotEdge1-edge0DotEdge1);

			edge0InvSqrLength = __fdividef(1.0f, edge0SqrLength);
			edge1InvSqrLength = __fdividef(1.0f, edge1SqrLength);
		}
	};
}

template <typename CurrentT>
__device__ void CuCollision::collideTriangles(CurrentT& current, uint32_t i)
{
	ProfileDetailZone zone(cloth::CuProfileZoneIds::COLLIDE_TRIANGLES);

	float posX = current(i, 0);
	float posY = current(i, 1);
	float posZ = current(i, 2);

	const TriangleData* tIt = reinterpret_cast<const TriangleData*>(&mCurData.mSphereX[0]);
	const TriangleData* tEnd = tIt + gClothData.mNumTriangles;

	float normalX, normalY, normalZ, normalD = 0.0f;
	float minSqrLength = FLT_MAX;

	for(; tIt != tEnd; ++tIt)
	{
		float dx = posX - tIt->baseX;
		float dy = posY - tIt->baseY;
		float dz = posZ - tIt->baseZ;

		float deltaDotEdge0  = dx*tIt->edge0X  + dy*tIt->edge0Y  + dz*tIt->edge0Z;
		float deltaDotEdge1  = dx*tIt->edge1X  + dy*tIt->edge1Y  + dz*tIt->edge1Z;
		float deltaDotNormal = dx*tIt->normalX + dy*tIt->normalY + dz*tIt->normalZ;

		float s = tIt->edge1SqrLength*deltaDotEdge0 - tIt->edge0DotEdge1*deltaDotEdge1;
		float t = tIt->edge0SqrLength*deltaDotEdge1 - tIt->edge0DotEdge1*deltaDotEdge0;

		s = t > 0.0f ? s * tIt->det : deltaDotEdge0 * tIt->edge0InvSqrLength;
		t = s > 0.0f ? t * tIt->det : deltaDotEdge1 * tIt->edge1InvSqrLength;

		if(s + t > 1.0f)
		{
			s = (tIt->edge1SqrLength - tIt->edge0DotEdge1 + 
				deltaDotEdge0 - deltaDotEdge1) * tIt->denom;
		}

		s = fmaxf(0.0f, fminf(1.0f, s));
		t = fmaxf(0.0f, fminf(1.0f - s, t));

		dx = dx - tIt->edge0X * s - tIt->edge1X * t;
		dy = dy - tIt->edge0Y * s - tIt->edge1Y * t;
		dz = dz - tIt->edge0Z * s - tIt->edge1Z * t;

		float sqrLength = dx*dx + dy*dy + dz*dz;

		if(0.0f > deltaDotNormal)
			sqrLength *= 1.0001f;

		if(sqrLength < minSqrLength)
		{
			normalX = tIt->normalX;
			normalY = tIt->normalY;
			normalZ = tIt->normalZ;
			normalD = deltaDotNormal;
			minSqrLength = sqrLength;
		}
	}

	if(normalD < 0.0f)
	{
		current(i, 0) = posX - normalX * normalD;
		current(i, 1) = posY - normalY * normalD;
		current(i, 2) = posZ - normalZ * normalD;
	}
}

namespace
{
	static const uint32_t sTrianglePadding = 
		sizeof(TriangleData)/sizeof(float) - 9;
}

template <typename CurrentT>
__device__ void CuCollision::collideTriangles(CurrentT& current, float alpha)
{
	if(!gClothData.mNumTriangles)
		return;

	// interpolate triangle vertices and store in shared memory
	for(uint32_t i=threadIdx.x, n=gClothData.mNumTriangles*9; i<n; i+=blockDim.x)
	{
		float start = gFrameData.mStartCollisionTriangles[i];
		float target = gFrameData.mTargetCollisionTriangles[i];
		uint32_t idx = i * 7282 >> 16; // same as i/9
		uint32_t offset = i + idx * sTrianglePadding;
		mCurData.mSphereX[offset] = start + (target - start) * alpha;
	}

	__syncthreads();

	for(uint32_t i=threadIdx.x; i<gClothData.mNumTriangles; i+=blockDim.x)
		reinterpret_cast<TriangleData*>(&mCurData.mSphereX[0])[i].initialize();

	__syncthreads();

	for(uint32_t i = threadIdx.x; i < gClothData.mNumParticles; i += blockDim.x)
		collideTriangles(current, i);

	__syncthreads();

}
