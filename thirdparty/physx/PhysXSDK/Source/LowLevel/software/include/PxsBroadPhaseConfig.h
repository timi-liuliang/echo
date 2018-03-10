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


#ifndef PXS_BROADPHASE_CONFIG_H
#define PXS_BROADPHASE_CONFIG_H

#include "CmPhysXCommon.h"
#include "PsVecMath.h"

namespace physx
{

using namespace Ps::aos;

///////////////////////////
//ICE SAP
///////////////////////////

//Maximum number of aabbs (static + dynamic) allowed for spu sap.
//Revert to ppu for greater numbers of aabbs.
#define MAX_NUM_BP_SPU_SAP_AABB 4096 //512 for spu debug
#define MAX_NUM_BP_SPU_SAP_END_POINTS (2*MAX_NUM_BP_SPU_SAP_AABB+2)
//Maximum number of overlaps allowed on spu.
#define MAX_NUM_BP_SPU_SAP_OVERLAPS 8192 //1024 for spu debug

//////////////////////////
//AGGREGATE MANAGER
//////////////////////////

#define MAX_AGGREGATE_BOUND_SIZE 128	//Max number of bounds in an aggregate bound.
#define MAX_AGGREGATE_BITMAP_SIZE 2048 //(128 * 128 / 8)
#define MAX_AGGREGATE_WORD_COUNT 512 //(128*128 / (32))

//////////////////////////
//DEFAULT BUFFER SIZES
//////////////////////////

#define PX_DEFAULT_BOX_ARRAY_CAPACITY 64
#define	PX_DEFAULT_AGGREGATE_CAPACITY 0

}

#endif //PXS_BROADPHASE_CONFIG_H
