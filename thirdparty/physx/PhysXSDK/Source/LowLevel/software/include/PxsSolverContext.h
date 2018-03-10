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


#ifndef PXS_SOLVERCONTEXT_H
#define PXS_SOLVERCONTEXT_H

namespace physx
{

struct PxcThresholdStreamElement;

struct PxcSolverContext
{
	bool doFriction;
	bool writeBackIteration;

	// for threshold stream output
	PxcThresholdStreamElement*		mThresholdStream;
	PxU32							mThresholdStreamIndex;
	PxU32							mThresholdStreamLength;
	PxcSolverBodyData*				solverBodyArray;
};

}

#endif //PXS_SOLVERCONTEXT_H
