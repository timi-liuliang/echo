/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __CUDA_MODULE_SCENE__
#define __CUDA_MODULE_SCENE__

// Make this header is safe for inclusion in headers that are shared with device code.
#if !defined(__CUDACC__)

#include "NxApexDefs.h"
#if defined(APEX_CUDA_SUPPORT)
#include "NiApexScene.h"
#include "PsShare.h"
#include "PsArray.h"
#include <cuda.h>
#include "ApexCudaWrapper.h"

namespace physx
{
class PhysXGpuIndicator;
class PxGpuDispatcher;

namespace apex
{
class NxApexCudaTestManager;

/* Every CUDA capable NiModuleScene will derive this class.  It
 * provides the access methods to your CUDA kernels that were compiled
 * into object files by nvcc.
 */
class CudaModuleScene : public ApexCudaObjManager
{
public:
	CudaModuleScene(NiApexScene& scene, NxModule& module, const char* modulePrefix = "");
	virtual ~CudaModuleScene() {}

	void destroy(NiApexScene& scene);

	void*	getHeadCudaObj()
	{
		return ApexCudaObjManager::getObjListHead();
	}

	physx::PhysXGpuIndicator*		mPhysXGpuIndicator;

	//ApexCudaObjManager
	virtual void onBeforeLaunchApexCudaFunc(const ApexCudaFunc& func, CUstream stream);
	virtual void onAfterLaunchApexCudaFunc(const ApexCudaFunc& func, CUstream stream);

protected:
	ApexCudaModule* getCudaModule(int modIndex);

	void initCudaObj(ApexCudaTexRef& obj);
	void initCudaObj(ApexCudaVar& obj);
	void initCudaObj(ApexCudaFunc& obj);
	void initCudaObj(ApexCudaSurfRef& obj);

	physx::Array<ApexCudaModule> mCudaModules;

	NiApexScene& mNiApexScene;

private:
	CudaModuleScene& operator=(const CudaModuleScene&);
};

}
} // namespace physx::apex

#endif
#endif

#endif // __CUDA_MODULE_SCENE__
