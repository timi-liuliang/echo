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

#include "PxPhysXConfig.h"
#if PX_USE_CLOTH_API

#include "extensions/PxClothTetherCooker.h"
#include "PxStrideIterator.h"
#include "PxVec4.h"
#include "foundation/PxMemory.h"

// from shared foundation
#include <PsFoundation.h>

using namespace physx;

struct physx::PxClothSimpleTetherCookerImpl
{
	PxClothSimpleTetherCookerImpl(const PxClothMeshDesc& desc);

	PxU32	getCookerStatus() const;
	void	getTetherData(PxU32* userTetherAnchors, PxReal* userTetherLengths) const;

public:
	// output
	shdfnd::Array<PxU32>	mTetherAnchors;
	shdfnd::Array<PxReal>	mTetherLengths;

protected:
	void	createTetherData(const PxClothMeshDesc &desc);

	PxU32	mCookerStatus;
};

PxClothSimpleTetherCooker::PxClothSimpleTetherCooker(const PxClothMeshDesc& desc)
: mImpl(new PxClothSimpleTetherCookerImpl(desc))
{
}

PxClothSimpleTetherCooker::~PxClothSimpleTetherCooker()
{
	delete mImpl;
}

PxU32 PxClothSimpleTetherCooker::getCookerStatus() const
{
	return mImpl->getCookerStatus();
}

void PxClothSimpleTetherCooker::getTetherData(PxU32* userTetherAnchors, PxReal* userTetherLengths) const
{
	mImpl->getTetherData(userTetherAnchors, userTetherLengths);
}

///////////////////////////////////////////////////////////////////////////////
PxClothSimpleTetherCookerImpl::PxClothSimpleTetherCookerImpl(const PxClothMeshDesc &desc) : mCookerStatus(1)
{
	createTetherData(desc);
}

///////////////////////////////////////////////////////////////////////////////
void PxClothSimpleTetherCookerImpl::createTetherData(const PxClothMeshDesc &desc)
{
	PxU32 numParticles = desc.points.count;

	if (!desc.invMasses.data)
		return;

	// assemble points
	shdfnd::Array<PxVec4> particles;
	particles.reserve(numParticles);
	PxStrideIterator<const PxVec3> pIt((const PxVec3*)desc.points.data, desc.points.stride);
	PxStrideIterator<const PxReal> wIt((const PxReal*)desc.invMasses.data, desc.invMasses.stride);
	for(PxU32 i=0; i<numParticles; ++i)
		particles.pushBack(PxVec4(*pIt++, wIt.ptr() ? *wIt++ : 1.0f));

	// compute tether data
	shdfnd::Array<PxU32> attachedIndices;
	for(PxU32 i=0; i < numParticles; ++i)
		if(particles[i].w == 0.0f)
			attachedIndices.pushBack(i);

	PxU32 n = attachedIndices.empty() ? 0 : numParticles;
	for(PxU32 i=0; i < n; ++i)
	{
		mTetherAnchors.reserve(numParticles);
		mTetherLengths.reserve(numParticles);

		PxVec3 position = reinterpret_cast<const PxVec3&>(particles[i]);
		float minSqrDist = FLT_MAX;
		PxU32 minIndex = numParticles;
		const PxU32 *aIt, *aEnd = attachedIndices.end();
		for(aIt = attachedIndices.begin(); aIt != aEnd; ++aIt)
		{
			float sqrDist = (reinterpret_cast<const PxVec3&>(
				particles[*aIt]) - position).magnitudeSquared();
			if(minSqrDist > sqrDist)
				minSqrDist = sqrDist, minIndex = *aIt;
		}

		mTetherAnchors.pushBack(minIndex);
		mTetherLengths.pushBack(PxSqrt(minSqrDist));
	}

	PX_ASSERT(mTetherAnchors.size() == mTetherLengths.size());
	if (numParticles == mTetherAnchors.size() && numParticles == mTetherLengths.size())
	{
		mCookerStatus = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////  
PxU32 PxClothSimpleTetherCookerImpl::getCookerStatus() const
{
	return mCookerStatus;
}

///////////////////////////////////////////////////////////////////////////////
void  
PxClothSimpleTetherCookerImpl::getTetherData(PxU32* userTetherAnchors, PxReal* userTetherLengths) const
{
	PxMemCopy(userTetherAnchors, mTetherAnchors.begin(), mTetherAnchors.size() * sizeof(PxU32));
	PxMemCopy(userTetherLengths, mTetherLengths.begin(), mTetherLengths.size() * sizeof(PxReal));
}


#endif //PX_USE_CLOTH_API


