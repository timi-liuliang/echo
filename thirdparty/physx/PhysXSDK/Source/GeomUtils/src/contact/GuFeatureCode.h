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

#ifndef GU_FEATURE_CODE_H
#define GU_FEATURE_CODE_H

#include "CmPhysXCommon.h"

namespace physx
{
namespace Gu
{
	enum FeatureCode
	{
		FC_VERTEX0,
		FC_VERTEX1,
		FC_VERTEX2,
		FC_EDGE01,
		FC_EDGE12,
		FC_EDGE20,
		FC_FACE,

		FC_UNDEFINED
	};

	bool		selectNormal(PxU8 data, PxReal u, PxReal v);
}
}

#endif
