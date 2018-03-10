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

#ifndef GU_PCM_TRIANGLE_CONTACT_GEN_H
#define GU_PCM_TRIANGLE_CONTACT_GEN_H

#include "GuPCMContactGenUtil.h"
#include "GuConvexSupportTable.h"
#include "GuPersistentContactManifold.h"
#include "GuShapeConvex.h"
#include "GuTriangleCache.h"


#ifdef PCM_LOW_LEVEL_DEBUG
#ifndef __SPU__
extern physx::Gu::PersistentContactManifold* gManifold;
#endif
#endif

namespace physx
{
	struct PxTriangleMeshGeometryLL;
	class PxHeightFieldGeometry;

namespace Gu
{

	bool PCMContactConvexMesh(const Gu::PolygonalData& polyData0, 
						Gu::SupportLocal* polyMap, 
						const Ps::aos::FloatVArg minMargin, 
						const PxBounds3& hullAABB, 
						const PxTriangleMeshGeometryLL& shapeMesh,
						const PxTransform& transform0, const PxTransform& transform1,
						PxReal contactDistance, Gu::ContactBuffer& contactBuffer,
						const Cm::FastVertex2ShapeScaling& convexScaling, const Cm::FastVertex2ShapeScaling& meshScaling,
						bool idtConvexScale, bool idtMeshScale, Gu::MultiplePersistentContactManifold& multiManifold);

	bool PCMContactConvexHeightfield(const Gu::PolygonalData& polyData0, 
						Gu::SupportLocal* polyMap, 
						const Ps::aos::FloatVArg minMargin, 
						const PxBounds3& hullAABB, 
						const PxHeightFieldGeometry& shapeHeightfield,
						const PxTransform& transform0, const PxTransform& transform1,
						PxReal contactDistance, Gu::ContactBuffer& contactBuffer,
						const Cm::FastVertex2ShapeScaling& convexScaling, bool idtConvexScale, Gu::MultiplePersistentContactManifold& multiManifold);


}
}

#endif
