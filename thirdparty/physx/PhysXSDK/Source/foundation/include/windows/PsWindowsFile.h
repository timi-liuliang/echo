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


#ifndef PX_FOUNDATION_PX_WINDOWS_FILE_H
#define PX_FOUNDATION_PX_WINDOWS_FILE_H

#include "foundation/Px.h"

#include <stdio.h>

namespace physx
{
namespace shdfnd
{
	PX_INLINE errno_t fopen_s( FILE ** _File, const char * _Filename, const char * _Mode) 
	{
		return ::fopen_s( _File, _Filename, _Mode);
	};

} // namespace shdfnd
} // namespace physx

#endif

