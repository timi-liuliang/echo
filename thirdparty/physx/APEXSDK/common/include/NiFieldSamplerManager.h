/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __NI_FIELD_SAMPLER_MANAGER_H__
#define __NI_FIELD_SAMPLER_MANAGER_H__

#include "NxApex.h"
#include "ApexSDKHelpers.h"

namespace physx
{
struct PxFilterData;

namespace apex
{

struct NiFieldSamplerQueryDesc;
class NiFieldSamplerQuery;

class NiFieldSamplerScene;

struct NiFieldSamplerDesc;
class NiFieldSampler;

struct NiFieldBoundaryDesc;
class NiFieldBoundary;

class NiFieldSamplerManager
{
public:
	virtual NiFieldSamplerQuery*	createFieldSamplerQuery(const NiFieldSamplerQueryDesc&) = 0;

	virtual void					registerFieldSampler(NiFieldSampler* , const NiFieldSamplerDesc& , NiFieldSamplerScene*) = 0;
	virtual void					unregisterFieldSampler(NiFieldSampler*) = 0;

	virtual void					registerFieldBoundary(NiFieldBoundary* , const NiFieldBoundaryDesc&) = 0;
	virtual void					unregisterFieldBoundary(NiFieldBoundary*) = 0;

#if NX_SDK_VERSION_MAJOR == 3
	virtual void					registerUnhandledParticleSystem(PxActor*) = 0;
	virtual void					unregisterUnhandledParticleSystem(PxActor*) = 0;
	virtual bool					isUnhandledParticleSystem(PxActor*) = 0;

	virtual bool					doFieldSamplerFiltering(const PxFilterData &o1, const PxFilterData &o2, PxF32 &weight) const = 0;
#elif NX_SDK_VERSION_MAJOR == 2
	virtual bool					doFieldSamplerFiltering(const NxGroupsMask64 &o1, const NxGroupsMask64 &o2, PxF32 &weight) const = 0;
#endif
};

}
} // end namespace physx::apex

#endif // #ifndef __NI_FIELD_SAMPLER_MANAGER_H__
