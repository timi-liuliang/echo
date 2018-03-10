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


#ifndef PX_SPU_COMMON
#define PX_SPU_COMMON


// AP: Need to map from SPU_RAYCAST_MODULE to IS_SPU_RAYCAST and such because xpj doesn't support macros with values AFAIK
// These macros enable different query types in NpBatchQuery::execute; this in turn pulls in all the other necessary code.
// All unused code will then be stripped by the linker, so this is the only place where we differentiate between the 3 SPU modules.
#ifdef SPU_RAYCAST_MODULE
#define IS_SPU_RAYCAST 1
#else
#define IS_SPU_RAYCAST 0
#endif
#ifdef SPU_OVERLAP_MODULE
#define IS_SPU_OVERLAP 1
#else
#define IS_SPU_OVERLAP 0
#endif
#ifdef SPU_SWEEP_MODULE
#define IS_SPU_SWEEP 1
#else
#define IS_SPU_SWEEP 0
#endif


#endif

