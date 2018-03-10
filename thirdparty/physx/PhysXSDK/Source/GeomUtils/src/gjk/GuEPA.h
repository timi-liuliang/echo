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

#ifndef GU_EPA_H
#define GU_EPA_H

#include "CmPhysXCommon.h"
#include "PsVecMath.h"
#include "GuGJKUtil.h"
#include "GuConvexSupportTable.h"

namespace physx
{
  
namespace Gu
{
	class ConvexV;
	//ML: The main entry point for EPA.
	// 
	//This function returns one of three status codes:
	//(1)EPA_FAIL:	the algorithm failed to create a valid polytope(the origin wasn't inside the polytope) from the input simplex.
	//(2)EPA_CONTACT : the algorithm found the MTD and converged successfully.
	//(3)EPA_DEGENERATE: the algorithm cannot make further progress and the result is unknown.

	PxGJKStatus epaPenetration(const ConvexV& a, const ConvexV& b, 					// two convexes, in the same space	
							   EPASupportMapPair* pair,
							   PxU8* PX_RESTRICT aInd, PxU8* PX_RESTRICT bInd,			// warm start index points to create an initial simplex that EPA will work on
							   PxU8 _size,												// count of warm-start indices
							   Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB,		// a point on each body: when B is translated by normal*penetrationDepth, these are coincident
							   Ps::aos::Vec3V& normal, Ps::aos::FloatV& depth,			// MTD normal & penetration depth							    
							   const bool takeCoreShape = false);						// indicates whether we take support point from the core shape of the convexes
}

}

#endif
