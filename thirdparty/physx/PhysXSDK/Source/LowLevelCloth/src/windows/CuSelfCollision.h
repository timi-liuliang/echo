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
#error include CuSelfCollision.h only from CuSolverKernel.cu
#endif

#include <cfloat>

namespace
{	
#if __CUDA_ARCH__ >= 300
	template <int>
	__device__ void scanWarp(uint32_t* counts)
	{
		asm volatile (
			"{"
			"	.reg .u32 tmp;"
			"	.reg .pred p;"
			"	shfl.up.b32 tmp|p, %0, 0x01, 0x0;"
			"@p add.u32 %0, tmp, %0;"
			"	shfl.up.b32 tmp|p, %0, 0x02, 0x0;"
			"@p add.u32 %0, tmp, %0;"
			"	shfl.up.b32 tmp|p, %0, 0x04, 0x0;"
			"@p add.u32 %0, tmp, %0;"
			"	shfl.up.b32 tmp|p, %0, 0x08, 0x0;"
			"@p add.u32 %0, tmp, %0;"
			"	shfl.up.b32 tmp|p, %0, 0x10, 0x0;"
			"@p add.u32 %0, tmp, %0;"
			"}"
			: "+r"(*counts) : 
		);
	}
#else
	template <int stride>
	__device__ void scanWarp(volatile uint32_t* counts)
	{
		const uint32_t laneIdx = threadIdx.x & warpSize-1;
		if(laneIdx >=  1) *counts += counts[- 1*stride]; 
		if(laneIdx >=  2) *counts += counts[- 2*stride]; 
		if(laneIdx >=  4) *counts += counts[- 4*stride]; 
		if(laneIdx >=  8) *counts += counts[- 8*stride]; 
		if(laneIdx >= 16) *counts += counts[-16*stride];
	}
#endif

	// sorts array by upper 16bits
	// [keys] must be at least 2*n in length, in/out in first n elements
	// [histogram] must be at least 34*16 = 544 in length
	__device__ void radixSort(uint32_t* keys, uint32_t n, 
		SharedPointer<uint32_t>::Type histogram)
	{
#if __CUDA_ARCH__ < 200
		// n^2 sort because compiler messes up atomics in large kernel, see CL 13355721
		for(uint32_t i=threadIdx.x; i<n; i+=blockDim.x)
		{
			uint32_t key = keys[i], index = n;
			for(uint32_t k = 0; k < n; ++k)
				index += keys[k] < key || k < i && keys[k] == key;
			keys[index] = key;
		}
		__syncthreads();

		for(uint32_t i=threadIdx.x; i<n; i+=blockDim.x)
			keys[i] = keys[i + n];

		__syncthreads();
#else
		const uint32_t numWarps = blockDim.x >> 5;
		const uint32_t warpIdx = threadIdx.x >> 5;
		const uint32_t laneIdx = threadIdx.x & warpSize-1;

		const uint32_t laneMask = (1u << laneIdx) - 1;
		const uint32_t mask1 =   (threadIdx.x & 1) - 1;
		const uint32_t mask2 = !!(threadIdx.x & 2) - 1;
		const uint32_t mask4 = !!(threadIdx.x & 4) - 1;
		const uint32_t mask8 = !!(threadIdx.x & 8) - 1;

		const uint32_t tn = (n + blockDim.x - 1) / blockDim.x;
		const uint32_t startIndex = tn * (threadIdx.x - laneIdx) + laneIdx;
		const uint32_t endIndex = min(startIndex + tn * warpSize, n + 31 & ~31); // full warps for ballot

		uint32_t* srcKeys = keys;
		uint32_t* dstKeys = keys + n;

		uint32_t* hIt = histogram + 16 * warpIdx;
		uint32_t* pIt = histogram + 16 * laneIdx + 16;
		uint32_t* tIt = histogram + 16 * numWarps + laneIdx; 

		for(uint32_t p=16; p<32; p+=4) // radix passes (4 bits each)
		{
			// gather bucket histograms per warp
			uint32_t warpCount = 0;
			for(uint32_t i=startIndex; i<endIndex; i+=32)
			{
				uint32_t key = i < n ? srcKeys[i] >> p : 15;
				uint32_t ballot1 = __ballot(key & 1);
				uint32_t ballot2 = __ballot(key & 2);
				uint32_t ballot4 = __ballot(key & 4);
				uint32_t ballot8 = __ballot(key & 8);
				warpCount += __popc((mask1 ^ ballot1) & (mask2 ^ ballot2) & (mask4 ^ ballot4) & (mask8 ^ ballot8));
			}

			if(laneIdx >= 16)
				hIt[laneIdx] = warpCount;

			__syncthreads();

			// prefix sum of histogram buckets
			for(uint32_t i=warpIdx; i<16; i+=numWarps)
				scanWarp<16>(pIt+i);

			__syncthreads();

			// prefix sum of bucket totals (exclusive)
			if(threadIdx.x < 16)
			{
				*tIt = tIt[-1] & !threadIdx.x - 1;
				scanWarp<1>(tIt);
				hIt[threadIdx.x] = 0;
			}

			__syncthreads();

			if(laneIdx < 16)
				hIt[laneIdx] += *tIt;

			// split indices
			for(uint32_t i=startIndex; i<endIndex; i+=32)
			{
				uint32_t key = i < n ? srcKeys[i] >> p : 15;
				uint32_t ballot1 = __ballot(key & 1);
				uint32_t ballot2 = __ballot(key & 2);
				uint32_t ballot4 = __ballot(key & 4);
				uint32_t ballot8 = __ballot(key & 8);
				uint32_t bits = ((key & 1)-1 ^ ballot1) & (!!(key & 2)-1 ^ ballot2) & (!!(key & 4)-1 ^ ballot4) & (!!(key & 8)-1 ^ ballot8);
				uint32_t index = hIt[key & 15] + __popc(bits & laneMask);

				if(i < n)
					dstKeys[index] = srcKeys[i];

				if(laneIdx < 16)
					hIt[laneIdx] += __popc((mask1 ^ ballot1) & (mask2 ^ ballot2) & (mask4 ^ ballot4) & (mask8 ^ ballot8));
			}

			__syncthreads();

			::swap(srcKeys, dstKeys);
		}
#endif

#ifndef NDEBUG
		for(uint32_t i=threadIdx.x; i<n; i+=blockDim.x)
			assert(!i || keys[i-1]>>16 <= keys[i]>>16);
#endif
	}
}


namespace 
{
	struct CuSelfCollision
	{
		template <typename CurrentT>
		__device__ void operator()(CurrentT& current);

	private:
		template <typename CurrentT>
		__device__ void buildAcceleration(const CurrentT& current);
		template <bool useRestPositions, typename CurrentT>
		__device__ void collideParticles(CurrentT& current) const;

	public:

		float mPosBias[3];
		float mPosScale[3];
		const float* mPosPtr[3];
	};
}

__shared__ uninitialized<CuSelfCollision> gSelfCollideParticles;

template <typename CurrentT>
__device__ void CuSelfCollision::operator()(CurrentT& current)
{
	if(min(gClothData.mSelfCollisionDistance, gFrameData.mSelfCollisionStiffness) <= 0.0f)
		return;

	ProfileDetailZone zone(cloth::CuProfileZoneIds::SELFCOLLIDE);

	if(threadIdx.x < 3)
	{
		float upper = gFrameData.mParticleBounds[threadIdx.x*2];
		float negativeLower = gFrameData.mParticleBounds[threadIdx.x*2+1];

		// expand bounds
		float eps = (upper + negativeLower) * 1e-4f;
		float expandedUpper = upper + eps;
		float expandedNegativeLower = negativeLower + eps;
		float expandedEdgeLength = expandedUpper + expandedNegativeLower;

		float* edgeLength = mPosBias; // use as temp
		edgeLength[threadIdx.x] = expandedEdgeLength;
	
		__threadfence_block();

		// calculate shortest axis
		uint32_t shortestAxis = edgeLength[0] > edgeLength[1];
		if(edgeLength[shortestAxis] > edgeLength[2])
			shortestAxis = 2;

		uint32_t writeAxis = threadIdx.x - shortestAxis;
		writeAxis += writeAxis >> 30;

		float maxInvCellSize = __fdividef(127.0f, expandedEdgeLength); 
		float invCollisionDistance = __fdividef(1.0f, gClothData.mSelfCollisionDistance);
		float invCellSize = min(maxInvCellSize, invCollisionDistance);

		mPosScale[writeAxis] = invCellSize;
		mPosBias[writeAxis] = invCellSize * expandedNegativeLower;
		mPosPtr[writeAxis] = &*current[threadIdx.x];
	}

	__syncthreads();

	buildAcceleration(current);
	
	if(gFrameData.mRestPositions)
		collideParticles<true>(current);
	else
		collideParticles<false>(current);
}

template <typename CurrentT>
__device__ void CuSelfCollision::buildAcceleration(const CurrentT& current) 
{
	ProfileDetailZone zone(cloth::CuProfileZoneIds::SELFCOLLIDE_ACCELERATION);

	uint32_t numIndices = gClothData.mNumSelfCollisionIndices;
	const uint32_t* indices = gClothData.mSelfCollisionIndices;
	uint32_t* sortedKeys = gClothData.mSelfCollisionKeys;
	uint16_t* cellStart = gClothData.mSelfCollisionCellStart;

	typename CurrentT::ConstPointerType rowPtr = mPosPtr[1];
	typename CurrentT::ConstPointerType colPtr = mPosPtr[2];

	float rowScale = mPosScale[1], rowBias = mPosBias[1];
	float colScale = mPosScale[2], colBias = mPosBias[2];

	// calculate keys
	for(uint32_t i = threadIdx.x; i < numIndices; i += blockDim.x)
	{
		uint32_t index = indices ? indices[i] : i;
		assert(index < gClothData.mNumParticles);

		uint32_t rowIndex = uint32_t(max(0.0f, min(rowPtr[index] * rowScale + rowBias, 127.5f)));
		uint32_t colIndex = uint32_t(max(0.0f, min(colPtr[index] * colScale + colBias, 127.5f)));
		assert(rowIndex < 128 && colIndex < 128);

		uint32_t key = (colIndex << 7 | rowIndex) + 129; // + row and column sentinel
		assert(key <= UINT16_MAX);

		sortedKeys[i] = key << 16 | index; // (key, index) pair in a single uint32_t
	}
	__syncthreads();

	// get scratch shared mem buffer used for radix sort(histogram) 
	SharedPointer<uint32_t>::Type buffer = reinterpret_cast<
		SharedPointer<uint32_t>::Type const&>(gCollideParticles.get().mCurData.mSphereX);

	// sort keys (__synchthreads inside radix sort)
	radixSort(sortedKeys, numIndices, buffer);

	// mark cell start if keys are different between neighboring threads
	for (uint32_t i=threadIdx.x; i<numIndices; i+=blockDim.x)
	{
		uint32_t key = sortedKeys[i] >> 16;
		uint32_t prevKey = i ? sortedKeys[i-1] >> 16 : key-1;
		if (key != prevKey)
		{
			cellStart[key] = i;
			cellStart[prevKey+1] = i;
		}
	}
	__syncthreads();
}

template <bool useRestPositions, typename CurrentT>
__device__ void CuSelfCollision::collideParticles(CurrentT& current) const
{
	ProfileDetailZone zone(cloth::CuProfileZoneIds::SELFCOLLIDE_PARTICLES);

	const uint32_t* sortedKeys = gClothData.mSelfCollisionKeys;
	float* sortedParticles = gClothData.mSelfCollisionParticles;
	uint16_t* cellStart = gClothData.mSelfCollisionCellStart;

	const float cdist = gClothData.mSelfCollisionDistance;
	const float cdistSq = cdist * cdist;

	const uint32_t numIndices = gClothData.mNumSelfCollisionIndices;
	const uint32_t numParticles = gClothData.mNumParticles;

	// point to particle copied in device memory that is being updated
	float* xPtr = sortedParticles;
	float* yPtr = sortedParticles + numParticles;
	float* zPtr = sortedParticles + 2*numParticles;
	float* wPtr = sortedParticles + 3*numParticles;

	// copy current particles to temporary array 
	for (uint32_t i=threadIdx.x; i < numParticles; i+=blockDim.x)
	{
		xPtr[i] = current(i, 0);
		yPtr[i] = current(i, 1);
		zPtr[i] = current(i, 2);
		wPtr[i] = current(i, 3);
	}
	__syncthreads();

	// copy only sorted (indexed) particles to shared mem
	for (uint32_t i=threadIdx.x; i < numIndices; i+=blockDim.x)
	{
		uint32_t index = sortedKeys[i] & UINT16_MAX;
		current(i,0) = xPtr[index];
		current(i,1) = yPtr[index];
		current(i,2) = zPtr[index];
		current(i,3) = wPtr[index];	
	}
	__syncthreads();

	typename CurrentT::ConstPointerType rowPtr = mPosPtr[1];
	typename CurrentT::ConstPointerType colPtr = mPosPtr[2];

	float rowScale = mPosScale[1], rowBias = mPosBias[1];
	float colScale = mPosScale[2], colBias = mPosBias[2];

	for(uint32_t i = threadIdx.x; i < numIndices; i += blockDim.x)
	{		
		const uint32_t index = sortedKeys[i] & UINT16_MAX;
		assert(index < gClothData.mNumParticles);

		float restX, restY, restZ;
		if (useRestPositions)
		{
			const float* restIt = gFrameData.mRestPositions + index*4;
			restX = restIt[0];
			restY = restIt[1];
			restZ = restIt[2];
		}

		float posX = current(i, 0);
		float posY = current(i, 1);
		float posZ = current(i, 2);
		float posW = current(i, 3);

		float deltaX = 0.0f;
		float deltaY = 0.0f;
		float deltaZ = 0.0f;
		float deltaW = FLT_EPSILON;

		// get cell index for this particle
		uint32_t rowIndex = uint32_t(max(0.0f, min(rowPtr[i] * rowScale + rowBias, 127.5f)));
		uint32_t colIndex = uint32_t(max(0.0f, min(colPtr[i] * colScale + colBias, 127.5f)));
		assert(rowIndex < 128 && colIndex < 128);

		uint32_t key = colIndex << 7 | rowIndex;
		assert(key <= UINT16_MAX);

		// check cells in 3 columns
		for(uint32_t keyEnd = key + 256; key<=keyEnd; key += 128) 
		{
			uint32_t startIndex; // min<unsigned>(cellStart[key+0..2])
			uint32_t endIndex; // max<signed>(0, cellStart[key+1..3])

			asm volatile (
				"{\n\t"
				"	.reg .u32 start1, start2;\n\t"
				"	ld.global.s16 %1, [%2+6];\n\t"
				"	ld.global.s16 %0, [%2+0];\n\t"
				"	ld.global.s16 start1, [%2+2];\n\t"
				"	ld.global.s16 start2, [%2+4];\n\t"
				"	max.s32 %1, %1, 0;\n\t"
				"	min.u32 %0, %0, start1;\n\t"
				"	max.s32 %1, %1, start1;\n\t"
				"	min.u32 %0, %0, start2;\n\t"
				"	max.s32 %1, %1, start2;\n\t"
				"}\n\t"
				: "=r"(startIndex), "=r"(endIndex)
#if defined(_WIN64) || defined(__x86_64__)
				: "l"(cellStart + key)
#else
				: "r"(cellStart + key)
#endif
			);

			for (uint32_t j=startIndex; j<endIndex; ++j)
			{
				if (j != i) // avoid same particle
				{
					float dx = posX - current(j, 0);
					float dy = posY - current(j, 1);
					float dz = posZ - current(j, 2);

					float distSqr = dx*dx + dy*dy + dz*dz;
					if(distSqr > cdistSq)
						continue;

					if (useRestPositions)
					{				
						const uint32_t jndex = sortedKeys[j] & UINT16_MAX;
						assert(jndex < gClothData.mNumParticles);

						// calculate distance in rest configuration
						const float* restJt = gFrameData.mRestPositions + jndex*4;			
						float rx = restX - restJt[0];
						float ry = restY - restJt[1];
						float rz = restZ - restJt[2];

						if (rx*rx + ry*ry + rz*rz <= cdistSq)
							continue;
					}

					// premultiply ratio for weighted average
					float ratio = fmaxf(0.0f, cdist * rsqrtf(FLT_EPSILON + distSqr) - 1.0f);
					float scale = __fdividef(ratio * ratio, FLT_EPSILON + posW + current(j, 3));

					deltaX += scale * dx;
					deltaY += scale * dy;
					deltaZ += scale * dz;
					deltaW += ratio;
				}
			}
		}

		const float stiffness = gFrameData.mSelfCollisionStiffness * posW;
		float scale = __fdividef(stiffness, deltaW);

		// apply collision impulse		
		xPtr[index] += deltaX * scale;
		yPtr[index] += deltaY * scale;
		zPtr[index] += deltaZ * scale;

		assert(!isnan(xPtr[index] + yPtr[index] + zPtr[index]));
	}
	__syncthreads();

	// copy temporary particle array back to shared mem
	// (need to copy whole array)
	for(uint32_t i = threadIdx.x; i < numParticles; i += blockDim.x)
	{	
		current(i,0) = xPtr[i];
		current(i,1) = yPtr[i];
		current(i,2) = zPtr[i];
		current(i,3) = wPtr[i];
	}

	// unmark occupied cells to empty again (faster than clearing all the cells)
	for(uint32_t i = threadIdx.x; i < numIndices; i += blockDim.x)
	{
		uint32_t key = sortedKeys[i] >> 16;
		cellStart[key  ] = 0xffff;
		cellStart[key+1] = 0xffff;
	}
	__syncthreads();
}

