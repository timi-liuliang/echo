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


#ifndef PXS_SOLVERCORESIMD_H
#define PXS_SOLVERCORESIMD_H

#include "PxsSolverCore.h"

namespace physx
{

class PxsSolverCoreSIMD : public PxsSolverCore
{
public:
	static PxsSolverCoreSIMD* create();

	// Implements PxsSolverCore
	virtual void destroyV();

	//! Solve
	virtual void solveV
		(const PxReal dt, const PxU32 positionIterations, const PxU32 velocityIterations, 
		PxcSolverBody* PX_RESTRICT atomListStart, PxcSolverBodyData* PX_RESTRICT atomDataList, const PxU32 solverBodyOffset, const PxU32 atomListSize,
		PxcArticulationSolverDesc *PX_RESTRICT articulationListStart, const PxU32 articulationListSize,
		PxcSolverConstraintDesc* PX_RESTRICT constraintList, const PxU32 constraintListSize,
		PxcSolverConstraintDesc* PX_RESTRICT frictionConstraintList, const PxU32 frictionConstraintConstraintListSize,
		Cm::SpatialVector* PX_RESTRICT motionVelocityArray) 
		const;

	//~Implements PxsSolverCore
};

}

#endif
