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

#ifndef GU_CONTACT_CONVEX_MESH_COMMON_H
#define GU_CONTACT_CONVEX_MESH_COMMON_H

#include "GuTriangleVertexPointers.h"
#include "CmScaling.h"
#include "GuTriangleCache.h"

namespace physx
{
	namespace Gu
	{
		class Container;
	}

namespace Gu
{

	struct PolygonalData;
	class ContactBuffer;

	PX_FORCE_INLINE void getVertices(
		PxVec3* PX_RESTRICT localPoints, const Gu::InternalTriangleMeshData* meshDataLS,
		const Cm::FastVertex2ShapeScaling& meshScaling, PxU32 triangleIndex, bool idtMeshScale)
	{
		PxVec3 v0, v1, v2;
		Gu::TriangleVertexPointers::getTriangleVerts(meshDataLS, triangleIndex, v0, v1, v2);

		if(idtMeshScale)
		{
			localPoints[0] = v0;
			localPoints[1] = v1;
			localPoints[2] = v2;
		}
		else
		{
			localPoints[0] = meshScaling * v0;
			localPoints[1] = meshScaling * v1;
			localPoints[2] = meshScaling * v2;
		}
	}
}
}

#endif
