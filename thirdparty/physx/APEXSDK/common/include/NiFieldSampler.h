/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __NI_FIELD_SAMPLER_H__
#define __NI_FIELD_SAMPLER_H__

#include "InplaceTypes.h"
#include "NiFieldBoundary.h"

#ifndef __CUDACC__
#include "NiApexSDK.h"
#endif

namespace physx
{
namespace apex
{


struct NiFieldSamplerType
{
	enum Enum
	{
		FORCE,
		ACCELERATION,
		VELOCITY_DRAG,
		VELOCITY_DIRECT,
	};
};

struct NiFieldSamplerGridSupportType
{
	enum Enum
	{
		NONE = 0,
		SINGLE_VELOCITY,
		VELOCITY_PER_CELL,
	};
};

#ifndef __CUDACC__

struct NiFieldSamplerDesc
{
	NiFieldSamplerType::Enum			type;
	NiFieldSamplerGridSupportType::Enum	gridSupportType;
	bool								cpuSimulationSupport;
#if NX_SDK_VERSION_MAJOR == 2
	NxGroupsMask64						samplerFilterData;
	NxGroupsMask64						boundaryFilterData;
#else
	PxFilterData						samplerFilterData;
	PxFilterData						boundaryFilterData;
#endif
	physx::PxF32						boundaryFadePercentage;

	physx::PxF32						dragCoeff; //only used then type is VELOCITY_DRAG

	void*								userData;

	NiFieldSamplerDesc()
	{
		type                     = NiFieldSamplerType::FORCE;
		gridSupportType          = NiFieldSamplerGridSupportType::NONE;
		cpuSimulationSupport = true;
#if NX_SDK_VERSION_MAJOR == 2
		samplerFilterData.bits0  = 0;
		samplerFilterData.bits1  = 0;
		boundaryFilterData.bits0 = 0;
		boundaryFilterData.bits1 = 0;
#else
		samplerFilterData.word0  = 0xFFFFFFFF;
		samplerFilterData.word1  = 0xFFFFFFFF;
		samplerFilterData.word2  = 0xFFFFFFFF;
		samplerFilterData.word3  = 0xFFFFFFFF;
		boundaryFilterData.word0 = 0xFFFFFFFF;
		boundaryFilterData.word1 = 0xFFFFFFFF;
		boundaryFilterData.word2 = 0xFFFFFFFF;
		boundaryFilterData.word3 = 0xFFFFFFFF;
#endif
		boundaryFadePercentage   = 0.1;
		dragCoeff                = 0;
		userData                 = NULL;
	}
};


class NiFieldSampler
{
public:
	//returns true if shape/params was changed
	//required to return true on first call!
	virtual bool updateFieldSampler(NiFieldShapeDesc& shapeDesc, bool& isEnabled) = 0;

	struct ExecuteData
	{
		physx::PxU32            count;
		physx::PxU32            positionStride;
		physx::PxU32            velocityStride;
		physx::PxU32            massStride;
		physx::PxU32            indicesMask;
		const physx::PxF32*	    position;
		const physx::PxF32*		velocity;
		const physx::PxF32*		mass;
		const physx::PxU32*		indices;
		physx::PxVec3*	        resultField;
	};

	virtual void executeFieldSampler(const ExecuteData& data)
	{
		PX_UNUSED(data);
		APEX_INVALID_OPERATION("not implemented");
	}

#if defined(APEX_CUDA_SUPPORT)
	struct CudaExecuteInfo
	{
		physx::PxU32		executeType;
		InplaceHandleBase	executeParamsHandle;
	};

	virtual void getFieldSamplerCudaExecuteInfo(CudaExecuteInfo& info) const
	{
		PX_UNUSED(info);
		APEX_INVALID_OPERATION("not implemented");
	}
#endif

	virtual physx::PxVec3 queryFieldSamplerVelocity() const
	{
		APEX_INVALID_OPERATION("not implemented");
		return physx::PxVec3(0.0f);
	}

protected:
	virtual ~NiFieldSampler() {}
};

#endif // __CUDACC__

}
} // end namespace physx::apex

#endif // #ifndef __NI_FIELD_SAMPLER_H__
