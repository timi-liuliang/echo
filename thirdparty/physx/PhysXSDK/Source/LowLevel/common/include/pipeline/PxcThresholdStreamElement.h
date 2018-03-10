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


#ifndef PXS_THRESHOLDSTREAMELEMENT_H
#define PXS_THRESHOLDSTREAMELEMENT_H

namespace physx
{

class PxsRigidBody;

struct PxcThresholdStreamElement
{
	PxsRigidBody *	body0;
	PxsRigidBody *	body1;
	PxReal			normalForce;
	PxReal			threshold;
};
#if !defined(PX_P64)
PX_COMPILE_TIME_ASSERT(0==(sizeof(PxcThresholdStreamElement) & 0x0f));
#endif

}

#endif
