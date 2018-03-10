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


#ifndef PX_FOUNDATION_PXFILE_H
#define PX_FOUNDATION_PXFILE_H

#include "foundation/PxPreprocessor.h"
#include "Ps.h"

#if (defined PX_WINDOWS || defined PX_WINMODERN)
	#include "windows/PsWindowsFile.h"
#elif defined PX_X360
	#include "xbox360/PsXbox360File.h"
#elif (defined PX_LINUX || defined PX_ANDROID || defined PX_APPLE || defined PX_PS4)
	#include "unix/PsUnixFile.h"
#elif defined PX_PS3
	#include "ps3/PsPS3File.h"
#elif defined PX_PSP2
	#include "psp2/PsPSP2File.h"  
#elif defined PX_WIIU
	#include "wiiu/PsWiiUFile.h"
#elif defined PX_XBOXONE
	#include "XboxOne/PsXboxOneFile.h"
#else
	#error "Platform not supported!"
#endif


#endif

