/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "NxApexDefs.h"
#include "NiApexSDK.h"
#include "ModulePerfScope.h"
#include "ProfilerCallback.h"

#pragma warning(disable:4100)

namespace MODULE_NAMESPACE
{

void initModuleProfiling(physx::apex::NiApexSDK* sdk, const char*)
{
	PX_UNUSED(sdk);
#ifdef PHYSX_PROFILE_SDK
#ifdef PX_WINDOWS
	gProfileZone = sdk->getProfileZone();
#endif
#endif
}

void releaseModuleProfiling()
{
#ifdef PHYSX_PROFILE_SDK
#ifdef PX_WINDOWS
	gProfileZone = NULL;
#endif
#endif
}


} // end namespace MODULE_NAMESPACE

