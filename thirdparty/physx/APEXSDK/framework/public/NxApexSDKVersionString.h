/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_APEX_SDKVERSION_STRING_H
#define NX_APEX_SDKVERSION_STRING_H

/*!
\file
\brief APEX SDK versioning definitions
*/

#include "foundation/PxPreprocessor.h"
#include "NxApexDefs.h"

//! \brief physx namespace
namespace physx
{
//! \brief apex namespace
namespace apex
{

/**
\brief These values are used to select version string NxGetApexSDKVersionString function should return
*/
enum NxApexSDKVersionString
{
	/**
	\brief APEX version
	*/
	VERSION = 0,
	/**
	\brief APEX build changelist
	*/
	CHANGELIST = 1,
	/**
	\brief APEX tools build changelist
	*/
	TOOLS_CHANGELIST = 2,
	/**
	\brief APEX branch used to create build 
	*/
	BRANCH = 3,
	/**
	\brief Time at which the build was created
	*/
	BUILD_TIME = 4,
	/**
	\brief Author of the build
	*/
	AUTHOR = 5,
	/**
	\brief Reason to prepare the build
	*/
	REASON = 6
};

/**
\def NXAPEX_API
\brief Export the function declaration from its DLL
*/

/**
\def NX_CALL_CONV
\brief Use C calling convention, required for exported functions
*/

#ifdef NX_CALL_CONV
#undef NX_CALL_CONV
#endif

#if defined(PX_WINDOWS)
#define NXAPEX_API extern "C" __declspec(dllexport)
#define NX_CALL_CONV __cdecl
#else
#define NXAPEX_API extern "C"
#define NX_CALL_CONV /* void */
#endif

/**
\brief Returns version strings
*/
NXAPEX_API const char*	NX_CALL_CONV NxGetApexSDKVersionString(NxApexSDKVersionString versionString);

}
} // end namespace physx::apex

#endif // NX_APEX_SDKVERSION_STRING_H
