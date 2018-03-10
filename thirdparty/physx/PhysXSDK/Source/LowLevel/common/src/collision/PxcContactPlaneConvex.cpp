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

#include "GuConvexMeshData.h"
#include "PxcContactMethodImpl.h"
#include "PxcNpCache.h"
#include "GuContactBuffer.h"
#include "CmScaling.h"

using namespace physx;

namespace physx
{
bool PxcContactPlaneConvex(CONTACT_METHOD_ARGS)
{
	PX_UNUSED(npCache);
	PX_UNUSED(shape0);

	// Get actual shape data
	//const PxPlaneGeometry& shapePlane = shape.get<const PxPlaneGeometry>();
	const PxConvexMeshGeometryLL& shapeConvex = shape1.get<const PxConvexMeshGeometryLL>();

	const PxVec3* PX_RESTRICT hullVertices = shapeConvex.hullData->getHullVertices();
	PxU32 numHullVertices = shapeConvex.hullData->mNbHullVertices;
//	Ps::prefetch128(hullVertices);

	// Plane is implicitly <1,0,0> 0 in localspace
	Cm::Matrix34 convexToPlane (transform0.transformInv(transform1));
	PxMat33 convexToPlane_rot(convexToPlane[0], convexToPlane[1], convexToPlane[2] );

	bool idtScale = shapeConvex.scale.isIdentity();
	Cm::FastVertex2ShapeScaling convexScaling;	// PT: TODO: remove default ctor
	if(!idtScale)
		convexScaling.init(shapeConvex.scale);

	convexToPlane = Cm::Matrix34( convexToPlane_rot * convexScaling.getVertex2ShapeSkew(), convexToPlane[3] );

	//convexToPlane = context.mVertex2ShapeSkew[1].getVertex2WorldSkew(convexToPlane);

	const Cm::Matrix34 planeToW (transform0);

	// This is rather brute-force
	
	bool status = false;

	const PxVec3 contactNormal = -planeToW.base0;

	while(numHullVertices--)
	{
		const PxVec3& vertex = *hullVertices++;
//		if(numHullVertices)
//			Ps::prefetch128(hullVertices);

		const PxVec3 pointInPlane = convexToPlane.transform(vertex);		//TODO: this multiply could be factored out!
		if(pointInPlane.x <= contactDistance)
		{
//			const PxVec3 pointInW = planeToW.transform(pointInPlane);
//			contactBuffer.contact(pointInW, -planeToW.base0, pointInPlane.x);
			status = true;
			Gu::ContactPoint* PX_RESTRICT pt = contactBuffer.contact();
			if(pt)
			{
				pt->normal				= contactNormal;
				pt->point				= planeToW.transform(pointInPlane);
				pt->separation			= pointInPlane.x;
				pt->internalFaceIndex0	= PXC_CONTACT_NO_FACE_INDEX;
				pt->internalFaceIndex1	= PXC_CONTACT_NO_FACE_INDEX;
			}
		}
	}
	return status;
}
}
