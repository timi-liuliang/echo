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

#ifndef GU_CONVEX_EDGE_FLAGS_H
#define GU_CONVEX_EDGE_FLAGS_H

namespace physx
{
namespace Gu
{
	enum ExtraTrigDataFlag
	{
		ETD_CONVEX_EDGE_01	= (1<<3),	// PT: important value, don't change
		ETD_CONVEX_EDGE_12	= (1<<4),	// PT: important value, don't change
		ETD_CONVEX_EDGE_20	= (1<<5)	// PT: important value, don't change
	};
}
}

#endif
