/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __NI_FIELD_SAMPLER_QUERY_H__
#define __NI_FIELD_SAMPLER_QUERY_H__

#include "NxApexDefs.h"
#include "ApexMirroredArray.h"

#include "PxTask.h"
#include "ApexActor.h"
#include "PxMat34Legacy.h"

namespace physx
{
namespace apex
{


class NiFieldSamplerScene;

struct NiFieldSamplerQueryDesc
{
	PxU32					maxCount;
#if NX_SDK_VERSION_MAJOR == 2
	NxGroupsMask64			samplerFilterData;
#else
	PxFilterData			samplerFilterData;
#endif
	NiFieldSamplerScene*	ownerFieldSamplerScene;


	NiFieldSamplerQueryDesc()
	{
		maxCount                = 0;
#if NX_SDK_VERSION_MAJOR == 2
		samplerFilterData.bits0 = 0;
		samplerFilterData.bits1 = 0;
#else
		samplerFilterData.word0 = 0xFFFFFFFF;
		samplerFilterData.word1 = 0xFFFFFFFF;
		samplerFilterData.word2 = 0xFFFFFFFF;
		samplerFilterData.word3 = 0xFFFFFFFF;
#endif
		ownerFieldSamplerScene  = 0;
	}
};

struct NiFieldSamplerQueryData
{
	PxF32						timeStep;
	PxU32						count;
	bool						isDataOnDevice;

	PxU32						positionStrideBytes; //Stride for position
	PxU32						velocityStrideBytes; //Stride for velocity
	PxF32*						pmaInPosition;
	PxF32*						pmaInVelocity;
	PxVec4*						pmaOutField;

	PxU32						massStrideBytes; //if massStride set to 0 supposed single mass for all objects
	PxF32*						pmaInMass;

	PxU32*						pmaInIndices;
};


#if defined(APEX_CUDA_SUPPORT)

class ApexCudaArray;

struct NiFieldSamplerQueryGridData
{
	PxU32 numX, numY, numZ;

	PxMat34Legacy	gridToWorld;

	PxF32			mass;

	PxF32			timeStep;

	PxVec3			cellSize;

	ApexCudaArray*	resultVelocity; //x, y, z = velocity vector, w = weight

	CUstream		stream;
};
#endif

class NiFieldSamplerCallback
{
public:
	virtual void operator()(void* stream = NULL) = 0;
};

class NiFieldSamplerQuery : public ApexActor
{
public:
	virtual PxTaskID submitFieldSamplerQuery(const NiFieldSamplerQueryData& data, PxTaskID taskID) = 0;

	//! NiFieldSamplerCallback will be called before FieldSampler computations
	virtual void setOnStartCallback(NiFieldSamplerCallback*) = 0;
	//! NiFieldSamplerCallback will be called after FieldSampler computations
	virtual void setOnFinishCallback(NiFieldSamplerCallback*) = 0;

#if defined(APEX_CUDA_SUPPORT)
	virtual physx::PxVec3 executeFieldSamplerQueryOnGrid(const NiFieldSamplerQueryGridData&)
	{
		APEX_INVALID_OPERATION("not implemented");
		return physx::PxVec3(0.0f);
	}
#endif

protected:
	virtual ~NiFieldSamplerQuery() {}
};

}
} // end namespace physx::apex

#endif // #ifndef __NI_FIELD_SAMPLER_QUERY_H__
