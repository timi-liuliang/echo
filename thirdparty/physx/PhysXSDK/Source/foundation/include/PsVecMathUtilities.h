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


#ifndef PX_PHYSICS_COMMON_VECMATHUTILITIES_H
#define PX_PHYSICS_COMMON_VECMATHUTILITIES_H

#include "PsVecMath.h"

namespace physx
{
namespace shdfnd
{
namespace aos
{
	/*!
		Extend an edge along its length by a factor
		*/
	PX_FORCE_INLINE void makeFatEdge(Vec3V& p0, Vec3V& p1, const FloatVArg fatCoeff)
	{
		const Vec3V delta = V3Sub(p1, p0);
		const FloatV m = V3Length(delta);
		const BoolV con = FIsGrtr(m, FZero());
		const Vec3V fatDelta = V3Scale(V3ScaleInv(delta, m), fatCoeff);
		p0 = V3Sel(con, V3Sub(p0, fatDelta), p0);
		p1 = V3Sel(con, V3Add(p1, fatDelta), p1);
	}
}
}
}

#endif
