/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __NI_FIELD_SAMPLER_SCENE_H__
#define __NI_FIELD_SAMPLER_SCENE_H__

#include "NxApexDefs.h"
#include "PxTask.h"

#include "NiModule.h"

#if defined(APEX_CUDA_SUPPORT)
#include "ApexCudaWrapper.h"
#endif


namespace physx
{
namespace apex
{


struct NiFieldSamplerSceneDesc
{
	bool	isPrimary;

	NiFieldSamplerSceneDesc()
	{
		isPrimary = false;
	}
};

namespace fieldsampler
{
struct NiFieldSamplerKernelLaunchData;
}

struct NiFieldSamplerQueryData;

class NiFieldSamplerScene : public NiModuleScene
{
public:
	virtual void getFieldSamplerSceneDesc(NiFieldSamplerSceneDesc& desc) const = 0;

	virtual const physx::PxTask* onSubmitFieldSamplerQuery(const NiFieldSamplerQueryData& data, const physx::PxTask* )
	{
		PX_UNUSED(data);
		return 0;
	}

#if defined(APEX_CUDA_SUPPORT)
	virtual ApexCudaConstStorage*	getFieldSamplerCudaConstStorage()
	{
		APEX_INVALID_OPERATION("not implemented");
		return 0;
	}

	virtual bool					launchFieldSamplerCudaKernel(const fieldsampler::NiFieldSamplerKernelLaunchData&)
	{
		APEX_INVALID_OPERATION("not implemented");
		return false;
	}
#endif

	virtual NxApexSceneStats* getStats()
	{
		return 0;
	}

};

#define FSST_PHYSX_MONITOR_LOAD		"FieldSamplerScene::PhysXMonitorLoad"
#define FSST_PHYSX_MONITOR_FETCH	"FieldSamplerScene::PhysXMonitorFetch"
#define FSST_PHYSX_MONITOR_UPDATE	"FieldSamplerPhysXMonitor::Update"
}

} // end namespace physx::apex

#endif // __NI_FIELD_SAMPLER_SCENE_H__
