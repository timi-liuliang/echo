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


#ifndef PXS_GPU_RIGID_BODY_ACCESS_H
#define PXS_GPU_RIGID_BODY_ACCESS_H

#include "PxRigidBodyAccessGpu.h"
#include "PsUserAllocated.h"

namespace physx
{

class PxsBodyTransformVault;

class PxsRigidBodyAccessGpu : public Ps::UserAllocated, public PxRigidBodyAccessGpu
{
public:
	virtual void copyShapeProperties(ShapeProperties& shapeProperties, const PxvShapeHandle shape, const PxvBodyHandle body) const;
	virtual void copyBodyProperties(BodyProperties& bodyProperties, const PxvBodyHandle* bodies, PxU32 numBodies) const;
	
public:
	PxsRigidBodyAccessGpu(const PxsBodyTransformVault& transformVault) : mTransformVault(transformVault) {}
	virtual ~PxsRigidBodyAccessGpu() {}
	
private:
	PxsRigidBodyAccessGpu& operator=(const PxsRigidBodyAccessGpu&);
	const PxsBodyTransformVault& mTransformVault;
};

}

#endif // PXS_GPU_RIGID_BODY_ACCESS_H
