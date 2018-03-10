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


#ifndef PXD_CONFIG_H
#define PXD_CONFIG_H

/*! \file internal top level include file for lowlevel. */

#include "CmPhysXCommon.h"
#include "PxPhysXConfig.h"

/************************************************************************/
/* Compiler workarounds                                                 */
/************************************************************************/
#ifdef PX_VC
#pragma warning(disable: 4355 )	// "this" used in base member initializer list
#pragma warning(disable: 4146 ) // unary minus operator applied to unsigned type.
#endif

namespace physx
{

// CA: Cell stuff
#ifdef PX_PS3
	#define USE_NEW_ISLAND_GEN
	#define CELL_SPU_ISLAND_GEN_MAX_ATOMS (20000) // Should be always smaller or equal CELL_SPU_ISLAND_GEN_MAX_CMS
	#define CELL_SPU_ISLAND_GEN_MAX_CMS   (40000)
	#define CELL_SPU_MAX_SLABS_PER_POOL   (256) // Check with elements per slab in active bodies and active contact managers
	#define CELL_SPU_UNCONST_ISLAND_FLAG_MASK (0x8000)
	#define CELL_SPU_UNCONST_ISLAND_MAX_SIZE (512)	// Crashes if > 512, 512 seems OK
	#define CELL_SPU_MERGED_ISLANDS_MAX_SIZE (32)
	#define CELL_SPU_MERGE_CANDIDATE_MAX_SIZE (16)
	#define CELL_SPU_MERGE_ISLANDS

	#define	SPU_NEW_CONVEX_VS_MESH
#endif //PX_PS3

}

#endif
