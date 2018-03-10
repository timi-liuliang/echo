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


#ifndef PX_COLLISION_CONVEXMESHBUILDER
#define PX_COLLISION_CONVEXMESHBUILDER

#include "GuConvexMesh.h"
#include "PxCooking.h"
#include "ConvexHullBuilder.h"

namespace physx
{

	class ConvexMeshBuilder : public Gu::ConvexMesh
	{
	public:
									ConvexMeshBuilder();
									~ConvexMeshBuilder();

				bool				loadFromDesc(const PxConvexMeshDesc&, PxPlatform::Enum	targetPlatform);
				bool				save(PxOutputStream& stream, bool platformMismatch)		const;

				bool				loadConvexHull(const PxConvexMeshDesc&);
				bool				computeHullPolygons(const PxU32& nbVerts,const PxVec3* verts, const PxU32& nbTriangles, const PxU32* triangles, PxAllocatorCallback& inAllocator,
										 PxU32& outNbVerts, PxVec3*& outVertices, PxU32& nbIndices, PxU32*& indices, PxU32& nbPolygons, PxHullPolygon*& polygons);
				bool				computeGaussMaps();
				void				computeMassInfo();
// TEST_INTERNAL_OBJECTS
				void				computeInternalObjects();
//~TEST_INTERNAL_OBJECTS

		mutable	ConvexHullBuilder	hullBuilder;
	};

}

#endif
