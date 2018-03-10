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

#ifndef GU_PCM_CONTACT_GEN_H
#define GU_PCM_CONTACT_GEN_H


#include "GuConvexSupportTable.h"
#include "GuPersistentContactManifold.h"
#include "GuShapeConvex.h"
#include "GuSeparatingAxes.h"


#ifdef	PCM_LOW_LEVEL_DEBUG
#ifndef __SPU__
extern physx::Gu::PersistentContactManifold* gManifold;
#endif
#endif

namespace physx
{

namespace Gu
{


	//full contact gen code for box/convexhull vs convexhull
	bool generateFullContactManifold(Gu::PolygonalData& polyData0, Gu::PolygonalData& polyData1, Gu::SupportLocal* map0, Gu::SupportLocal* map1, Gu::PersistentContact* manifoldContacts, PxU32& numContacts,
		const Ps::aos::FloatVArg contactDist, const Ps::aos::Vec3VArg normal, const bool doOverlapTest);

	//full contact gen code for capsule vs convexhulll
	bool generateFullContactManifold(const Gu::CapsuleV& capsule, Gu::PolygonalData& polyData, Gu::SupportLocal* map, const Ps::aos::PsMatTransformV& aToB,  Gu::PersistentContact* manifoldContacts, PxU32& numContacts,
		const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& normal, const bool doOverlapTest);

	//full contact gen code for capsule vs box
	bool generateCapsuleBoxFullContactManifold(const Gu::CapsuleV& capsule, Gu::PolygonalData& polyData, Gu::SupportLocal* map, const Ps::aos::PsMatTransformV& aToB, Gu::PersistentContact* manifoldContacts, PxU32& numContacts,
		const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& normal, const bool doOverlapTest);

	//MTD code for box/convexhull vs box/convexhull
	bool computeMTD(Gu::PolygonalData& polyData0, Gu::PolygonalData& polyData1,  SupportLocal* map0, SupportLocal* map1, Ps::aos::FloatV& penDepth, Ps::aos::Vec3V& normal);
	
	//MTD code for capsule vs box/convexhull
	bool computeMTD(const Gu::CapsuleV& capsule, Gu::PolygonalData& polyData, Gu::SupportLocal* map, Ps::aos::FloatV& penDepth, Ps::aos::Vec3V& normal);

	void buildPartialHull(const Gu::PolygonalData& polyData, SupportLocal* map, Gu::SeparatingAxes& validAxes, const Ps::aos::Vec3VArg v, const Ps::aos::Vec3VArg _dir);

	//full contact gen code for sphere vs convexhull
	bool generateSphereFullContactManifold(const Gu::CapsuleV& capsule, Gu::PolygonalData& polyData, Gu::SupportLocal* map, Gu::PersistentContact* manifoldContacts, PxU32& numContacts,
		const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& normal, const bool doOverlapTest);

}  
}

#endif
