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


#ifndef PX_RIGID_BODY_ACCESS_GPU_H
#define PX_RIGID_BODY_ACCESS_GPU_H

#include "PxStrideIterator.h"
#include "PxTransform.h"
#include "GuGeometryUnion.h"

namespace physx
{

typedef size_t PxvShapeHandle;
typedef size_t PxvBodyHandle;

class PxRigidBodyAccessGpu
{
public:

	struct ShapeProperties
	{
		PxTransform* ownerToWorld;
		PxTransform* shapeToOwner;
		Gu::GeometryUnion* geometry;
	};

	struct BodyProperties
	{
		PxStrideIterator<PxTransform>		currentTransforms;
		PxStrideIterator<PxTransform>		previousTransforms;
		PxStrideIterator<PxVec3>			linearVelocities;
		PxStrideIterator<PxVec3>			angularVelocities;
		PxStrideIterator<PxTransform>		body2ActorTransforms;
		PxStrideIterator<PxvBodyHandle>		cpuBodyHandle;
	};

	virtual void copyShapeProperties(ShapeProperties& shapeProperties, const PxvShapeHandle shape, const PxvBodyHandle body)	const = 0;
	virtual void copyBodyProperties(BodyProperties& bodyProperties, const PxvBodyHandle* bodies, PxU32 numBodies)		const = 0;
};

}

#endif // PX_RIGID_BODY_ACCESS_GPU_H
