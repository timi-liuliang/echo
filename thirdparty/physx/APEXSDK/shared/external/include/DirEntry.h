/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef DIR_ENTRY_INCLUDE_H
#define DIR_ENTRY_INCLUDE_H

#include "foundation/PxPreprocessor.h"

#if (defined PX_WINDOWS)
	#include "windows/DirEntry.h"
#elif defined PX_X360
	#include "xbox360/DirEntry.h"
#elif defined PX_XBOXONE
	#include "xboxone/DirEntry.h"
#elif (defined PX_LINUX || defined PX_APPLE || defined PX_ANDROID)
	#include "linux/DirEntry.h"
#elif defined PX_PS3
	#include "ps3/DirEntry.h"
#elif defined PX_PS4
	#include "ps4/DirEntry.h"
#else
	#error "Platform not supported!"
#endif

#endif // DIR_ENTRY_INCLUDE_H
