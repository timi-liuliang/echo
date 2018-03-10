/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __NI_MODULE_IOFX_H__
#define __NI_MODULE_IOFX_H__

#include "NiModule.h"

namespace physx
{
namespace apex
{

class NiIofxManagerDesc;
class NiIofxManager;
class NxApexScene;

class NiModuleIofx : public NiModule
{
public:
	virtual NiIofxManager*	createActorManager(const NxApexScene& scene, const physx::apex::NxIofxAsset& asset, const NiIofxManagerDesc& desc) = 0;
};

}
} // end namespace physx::apex

#endif // __NI_MODULE_IOFX_H__
