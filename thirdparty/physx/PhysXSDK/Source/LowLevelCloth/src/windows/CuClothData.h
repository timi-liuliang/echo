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

#include "Types.h"
#include "Simd4f.h"

namespace physx
{
namespace cloth
{

class CuCloth;
struct CuPhaseConfig;
template <typename> struct IterationState;
struct IndexPair;
struct CuIterationData;
struct CuTether;

// reference to cloth instance bulk data (POD)
// should not need frequent updates (stored on device)
struct CuClothData
{
	CuClothData() {}
	CuClothData(CuCloth&);

	// particle data
	uint32_t mNumParticles;
	float* mParticles; 
	float* mParticlesHostCopy;

	// fabric constraints
	uint32_t mNumPhases;
	const CuPhaseConfig* mPhaseConfigs;

	const CuTether* mTethers;
	uint32_t mNumTethers;
	float mTetherConstraintScale;

	// motion constraint data
	float mMotionConstraintScale;
	float mMotionConstraintBias;

	// collision data
	uint32_t mNumSpheres; // don't change this order, it's
	uint32_t mNumCapsules; // needed by mergeAcceleration()
	const IndexPair* mCapsuleIndices;
	uint32_t mNumPlanes;
	uint32_t mNumConvexes;
	const uint32_t* mConvexMasks;
	uint32_t mNumTriangles;

	// virtual particle data
	const uint32_t* mVirtualParticleSetSizesBegin;
	const uint32_t* mVirtualParticleSetSizesEnd;
	const uint16_t* mVirtualParticleIndices;
	const float* mVirtualParticleWeights;

	bool mEnableContinuousCollision;
	float mCollisionMassScale;
	float mFrictionScale;

	float mSelfCollisionDistance;
	uint32_t mNumSelfCollisionIndices;
	const uint32_t* mSelfCollisionIndices;
	float* mSelfCollisionParticles;
	uint32_t* mSelfCollisionKeys;
	uint16_t* mSelfCollisionCellStart;

	// sleep data
	uint32_t mSleepTestInterval;
	uint32_t mSleepAfterCount;
	float mSleepThreshold;
};

// per-frame data (stored in pinned memory)
struct CuFrameData
{
	CuFrameData() {} // not initializing pointers to 0!

	explicit CuFrameData(CuCloth&, uint32_t, 
		const IterationState<Simd4f>&, const CuIterationData*);

	bool mDeviceParticlesDirty;

	// number of particle copies that fit in shared memory (0, 1, or 2)
	uint32_t mNumSharedPositions;

	// iteration data
	float mIterDt;
	uint32_t mNumIterations;
	const CuIterationData* mIterationData;

	float mTetherConstraintStiffness;

	// motion constraint data
	const float* mStartMotionConstraints;
	float* mTargetMotionConstraints;
	const float* mHostMotionConstraints;
	float mMotionConstraintStiffness;

	// separation constraint data
	const float* mStartSeparationConstraints;
	float* mTargetSeparationConstraints;
	const float* mHostSeparationConstraints;

	// particle acceleration data
	float* mParticleAccelerations;
	const float* mHostParticleAccelerations;

	// rest positions
	const float* mRestPositions;

	// collision data
	const float* mStartCollisionSpheres;
	const float* mTargetCollisionSpheres;
	const float* mStartCollisionPlanes;
	const float* mTargetCollisionPlanes;
	const float* mStartCollisionTriangles;
	const float* mTargetCollisionTriangles;

	float mSelfCollisionStiffness;

	float mParticleBounds[6]; // maxX, -minX, maxY, ...

	uint32_t mSleepPassCounter; 
	uint32_t mSleepTestCounter;

	float mStiffnessExponent;
};

// per-iteration data (stored in pinned memory)
struct CuIterationData
{
	CuIterationData() {} // not initializing!

	explicit CuIterationData(const IterationState<Simd4f>&);

	float mIntegrationTrafo[24];
	uint32_t mIsTurning;
};

}

}
