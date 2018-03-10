/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef PX_CONVX_COMMON_H
#define PX_CONVX_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

//#pragma warning(disable:4512)	// assignment operator could not be generated
#pragma warning(disable:4121)	// alignment of a member was sensitive to packing

#define DELETESINGLE(x)	if(x){ delete x;	x = NULL; }
#define DELETEARRAY(x)	if(x){ delete []x;	x = NULL; }

#include "common/PxPhysXCommonConfig.h"      

#define	inline_	PX_FORCE_INLINE
#define PsArray physx::shdfnd::Array

#endif
