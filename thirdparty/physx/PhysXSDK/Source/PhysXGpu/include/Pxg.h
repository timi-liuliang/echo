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


#ifndef PX_GPU_NX
#define PX_GPU_NX

#include "Px.h"

#if PX_SUPPORT_GPU_PHYSX

// define API function declaration
#if defined(PX_WINDOWS) 
	#if defined PX_PHYSX_GPU_EXPORTS
	#define PX_PHYSX_GPU_API __declspec(dllexport)
	#else
	#define PX_PHYSX_GPU_API __declspec(dllimport)
	#endif
#elif defined(PX_UNIX)
	#define PX_PHYSX_GPU_API PX_UNIX_EXPORT
#else
	#define PX_PHYSX_GPU_API
#endif

#ifndef PX_C_EXPORT
#define PX_C_EXPORT extern "C"
#endif

#else // PX_SUPPORT_GPU_PHYSX
#pragma message("Pxg.h is only supported on GPGPU platforms!")
#endif // PX_SUPPORT_GPU_PHYSX

#endif // PX_GPU_NX

