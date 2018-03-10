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

#ifndef GU_CONVEXHELPER_H
#define GU_CONVEXHELPER_H

#include "PsFPU.h"
#include "PxVec3.h"
#include "GuShapeConvex.h"

namespace physx
{
namespace Gu
{
	class GeometryUnion;

	PX_FORCE_INLINE void copyPlane(PxPlane* PX_RESTRICT dst, const Gu::HullPolygonData* PX_RESTRICT src)
	{
		// PT: "as usual" now, the direct copy creates LHS that are avoided by the IR macro...
#ifdef PX_PS3
		// AP: IR macro causes compiler warnings (dereferencing type-punned pointer will break strict-aliasing rules)
		dst->n = src->mPlane.n;
		dst->d = src->mPlane.d;
#else
		PX_IR(dst->n.x) = PX_IR(src->mPlane.n.x);
		PX_IR(dst->n.y) = PX_IR(src->mPlane.n.y);
		PX_IR(dst->n.z) = PX_IR(src->mPlane.n.z);
		PX_IR(dst->d) = PX_IR(src->mPlane.d);
#endif
	}

	///////////////////////////////////////////////////////////////////////////

	PX_PHYSX_COMMON_API void getScaledConvex(	PxVec3*& scaledVertices, PxU8*& scaledIndices, PxVec3* dstVertices, PxU8* dstIndices,
												bool idtConvexScale, const PxVec3* srcVerts, const PxU8* srcIndices, PxU32 nbVerts, const Cm::FastVertex2ShapeScaling& convexScaling);

	// PT: calling this correctly isn't trivial so let's macroize it. At least we limit the damage since it immediately calls a real function.
	#define GET_SCALEX_CONVEX(scaledVertices, stackIndices, idtScaling, nbVerts, scaling, srcVerts, srcIndices)	\
	getScaledConvex(scaledVertices, stackIndices,																\
					idtScaling ? NULL : (PxVec3*)PxAlloca(nbVerts * sizeof(PxVec3)),							\
					idtScaling ? NULL : (PxU8*)PxAlloca(nbVerts * sizeof(PxU8)),								\
					idtScaling, srcVerts, srcIndices, nbVerts, scaling);

	PX_PHYSX_COMMON_API bool getConvexData(const Gu::GeometryUnion& shape, Cm::FastVertex2ShapeScaling& scaling, PxBounds3& bounds, PolygonalData& polyData);

	struct ConvexEdge
	{
		PxU8	vref0;
		PxU8	vref1;
		PxVec3	normal;	// warning: non-unit vector!
	};

	PX_PHYSX_COMMON_API PxU32 findUniqueConvexEdges(PxU32 maxNbEdges, ConvexEdge* PX_RESTRICT edges, PxU32 numPolygons, const Gu::HullPolygonData* PX_RESTRICT polygons, const PxU8* PX_RESTRICT vertexData);
}
}

#endif
