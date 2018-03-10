/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_APEX_DEFS_H
#define NX_APEX_DEFS_H

/*!
\file
\brief Version identifiers and other macro definitions

	This file is intended to be usable without picking up the entire
	public APEX API, so it explicitly does not include NxApex.h
*/

#include "NxPhysXSDKVersion.h"

/*!
 \def NX_APEX_SDK_VERSION
 \brief APEX Framework API version

 Used for making sure you are linking to the same version of the SDK files
 that you have included.  Should be incremented with every API change.

 \def NX_APEX_SDK_RELEASE
 \brief APEX SDK Release version

 Used for conditionally compiling user code based on the APEX SDK release version.

 \def DYNAMIC_CAST
 \brief Determines use of dynamic_cast<> by APEX modules

 \def APEX_USE_PARTICLES
 \brief Determines use of particle-related APEX modules

 \def NX_APEX_DEFAULT_NO_INTEROP_IMPLEMENTATION
 \brief Provide API stubs with no CUDA interop support

 Use this to add default implementations of interop-related interfaces for NxUserRenderer.
*/

#include "foundation/PxPreprocessor.h"

#define NX_APEX_SDK_VERSION 1
#define NX_APEX_SDK_RELEASE 0x01030300

//#if defined(PX_WINDOWS) && !defined(PX_VC14)
//#define APEX_CUDA_SUPPORT 1
//#endif


#if USE_RTTI
#define DYNAMIC_CAST(type) dynamic_cast<type>
#else
#define DYNAMIC_CAST(type) static_cast<type>
#endif

#if defined(PX_WINDOWS) && !defined(PX_VC14)
#define APEX_USE_PARTICLES 0
#else
#define APEX_USE_PARTICLES 0
#endif

#define NX_APEX_DEFAULT_NO_INTEROP_IMPLEMENTATION 1

#endif // NX_APEX_DEFS_H
