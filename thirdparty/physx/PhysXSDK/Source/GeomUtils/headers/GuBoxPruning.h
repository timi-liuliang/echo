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

#ifndef GU_BOXPRUNING_H
#define GU_BOXPRUNING_H

#include "PxBounds3.h"
#include "PsArray.h"
#include "CmPhysXCommon.h"
#include "PxPhysXCommonConfig.h"
#include "GuAxes.h"

namespace physx
{

namespace Gu
{
	PX_PHYSX_COMMON_API bool CompleteBoxPruning(const PxBounds3* bounds, PxU32 nb, Ps::Array<PxU32>& pairs, const Gu::Axes& axes);
	PX_PHYSX_COMMON_API bool BipartiteBoxPruning(const PxBounds3* bounds0, PxU32 nb0, const PxBounds3* bounds1, PxU32 nb1, Ps::Array<PxU32>& pairs, const Gu::Axes& axes);
}
}
	
#endif // GU_BOXPRUNING_H
