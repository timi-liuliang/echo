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

#ifndef PS_INLINE_AOS_H
#define PS_INLINE_AOS_H

#include "foundation/PxPreprocessor.h"

#if defined PX_WINDOWS
	#include "windows/PsWindowsTrigConstants.h"
	#include "windows/PsWindowsInlineAoS.h"
#elif defined PX_X360
	#include "xbox360/PsXbox360InlineAoS.h"
#elif (defined PX_LINUX || defined PX_ANDROID || defined PX_APPLE || defined PX_PS4 || (defined PX_WINMODERN && defined PX_ARM_NEON))
	#include "unix/PsUnixTrigConstants.h"
	#include "unix/PsUnixInlineAoS.h"
#elif defined PX_PS3
	#include "ps3/PsPS3InlineAoS.h"
#elif defined PX_PSP2
	#include "psp2/PsPSP2InlineAoS.h"
#elif defined PX_XBOXONE
	#include "XboxOne/PsXboxOneTrigConstants.h"
	#include "XboxOne/PsXboxOneInlineAoS.h"
#else
	#error "Platform not supported!"
#endif

#endif

