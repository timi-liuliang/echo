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


#ifndef PXV_GEOMETRY_H
#define PXV_GEOMETRY_H

#include "PxVec3.h"
#include "PxQuat.h"
#include "PxTransform.h"
#include "PxGeometry.h"
#include "PxConvexMeshGeometry.h"
#include "PxTriangleMeshGeometry.h"
#include "PxHeightFieldGeometry.h"

#include "PxvConfig.h"

namespace physx
{

class PxsRigidBody;
class PxvParticleSystem;

namespace Gu
{
	struct ConvexHullData;
	struct InternalTriangleMeshData;
	struct HeightFieldData;
}

}

/*!
\file
Geometry interface
*/

/************************************************************************/
/* Shapes                                                               */
/************************************************************************/

// moved to
#include "GuGeometryUnion.h"

namespace physx
{

struct PxsShapeCore
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================

// PX_SERIALIZATION
	PxsShapeCore()										{}
	PxsShapeCore(const PxEMPTY&) : geometry(PxEmpty)	{}
//~PX_SERIALIZATION

	PX_ALIGN_PREFIX(16)
	PxTransform					transform PX_ALIGN_SUFFIX(16);
	PxReal						contactOffset;
	PxU8						mShapeFlags;			// !< API shape flags	// PT: TODO: use PxShapeFlags here. Needs to move flags to separate file.
	PxU8						padding[1];
	PxU16						materialIndex;
	Gu::GeometryUnion			geometry;
};

PX_COMPILE_TIME_ASSERT( (sizeof(PxsShapeCore)&0xf) == 0);

}

#endif
