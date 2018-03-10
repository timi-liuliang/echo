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

#ifndef GU_MTD_H
#define GU_MTD_H

#include "PxPhysXCommonConfig.h"

namespace physx
{
	class PxConvexMeshGeometry;
	class PxTriangleMeshGeometry;
	class PxPlane;
	class PxGeometry;
	class PxTransform;
	class PxHeightFieldGeometry;

namespace Gu
{
	class Box;
	class Sphere;
	class Capsule;

	// PT: the depenetration vector D is equal to mtd * depth. It should be applied to the
	// first object, to get out of the second object.

	// PT: the function names should follow the order in which the PxGeometryTypes are listed,
	// i.e. computeMTD_Type0Type1 with Type0<=Type1. This is to guarantee that the proper results
	// (following the desired convention) are returned from the PxGeometryQuery-level call.

	PX_PHYSX_COMMON_API	bool	computeMTD_SphereSphere		(PxVec3& mtd, PxF32& depth, const Sphere& sphere0, const Sphere& sphere1);
	PX_PHYSX_COMMON_API	bool	computeMTD_SphereCapsule	(PxVec3& mtd, PxF32& depth, const Sphere& sphere, const Capsule& capsule);
	PX_PHYSX_COMMON_API	bool	computeMTD_SphereBox		(PxVec3& mtd, PxF32& depth, const Sphere& sphere, const Box& box);
	PX_PHYSX_COMMON_API	bool	computeMTD_CapsuleCapsule	(PxVec3& mtd, PxF32& depth, const Capsule& capsule0, const Capsule& capsule1);
	PX_PHYSX_COMMON_API	bool	computeMTD_CapsuleBox		(PxVec3& mtd, PxF32& depth, const Capsule& capsule, const Box& box);
	PX_PHYSX_COMMON_API	bool	computeMTD_BoxBox			(PxVec3& mtd, PxF32& depth, const Box& box0, const Box& box1);

	PX_PHYSX_COMMON_API	bool	computeMTD_SphereConvex		(PxVec3& mtd, PxF32& depth, const Sphere& sphere, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexPose);
	PX_PHYSX_COMMON_API	bool	computeMTD_BoxConvex		(PxVec3& mtd, PxF32& depth, const Box& box, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexPose);
	PX_PHYSX_COMMON_API	bool	computeMTD_CapsuleConvex	(PxVec3& mtd, PxF32& depth, const Capsule& capsule, const PxTransform& capsulePose, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexPose);
	PX_PHYSX_COMMON_API	bool	computeMTD_ConvexConvex		(PxVec3& mtd, PxF32& depth, const PxConvexMeshGeometry& convexGeom0, const PxTransform& convexPose0, const PxConvexMeshGeometry& convexGeom1, const PxTransform& convexPose1);

	PX_PHYSX_COMMON_API	bool	computeMTD_SpherePlane		(PxVec3& mtd, PxF32& depth, const Sphere& sphere, const PxPlane& plane);
	PX_PHYSX_COMMON_API	bool	computeMTD_PlaneBox			(PxVec3& mtd, PxF32& depth, const PxPlane& plane, const Box& box);
	PX_PHYSX_COMMON_API	bool	computeMTD_PlaneCapsule		(PxVec3& mtd, PxF32& depth, const PxPlane& plane, const Capsule& capsule);
	PX_PHYSX_COMMON_API	bool	computeMTD_PlaneConvex		(PxVec3& mtd, PxF32& depth, const PxPlane& plane, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexPose);

	PX_PHYSX_COMMON_API	bool	computeMTD_SphereMesh		(PxVec3& mtd, PxF32& depth, const Sphere& sphere, const PxTriangleMeshGeometry& meshGeom, const PxTransform& meshPose);
	PX_PHYSX_COMMON_API	bool	computeMTD_BoxMesh			(PxVec3& mtd, PxF32& depth, const Box& box, const PxTriangleMeshGeometry& meshGeom, const PxTransform& meshPose);
	PX_PHYSX_COMMON_API	bool	computeMTD_CapsuleMesh		(PxVec3& mtd, PxF32& depth, const Capsule& capsule, const PxTriangleMeshGeometry& meshGeom, const PxTransform& meshPose);
	PX_PHYSX_COMMON_API	bool	computeMTD_ConvexMesh		(PxVec3& mtd, PxF32& depth, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexPose, const PxTriangleMeshGeometry& meshGeom, const PxTransform& meshPose);

	PX_PHYSX_COMMON_API	bool	computeMTD_SphereHeightField(PxVec3& mtd, PxF32& depth, const Sphere& sphere, const PxHeightFieldGeometry& meshGeom, const PxTransform& meshPose);
	PX_PHYSX_COMMON_API	bool	computeMTD_CapsuleHeightField(PxVec3& mtd, PxF32& depth, const Capsule& sphere, const PxHeightFieldGeometry& meshGeom, const PxTransform& meshPose);
	PX_PHYSX_COMMON_API bool	computeMTD_BoxHeightField	(PxVec3& mtd, PxF32& depth, const Box& box, const PxHeightFieldGeometry& meshGeom, const PxTransform& meshPose);
	PX_PHYSX_COMMON_API bool	computeMTD_ConvexHeightField(PxVec3& mtd, PxF32& depth, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexPose, const PxHeightFieldGeometry& meshGeom, const PxTransform& meshPose);


	#define GEOM_MTD_CALLBACK_PARAMS	PxVec3& mtd, PxF32& depth, const PxGeometry& geom0, const PxTransform& transform0, const PxGeometry& geom1, const PxTransform& transform1
	typedef bool (*GeomMTDFunc)	(GEOM_MTD_CALLBACK_PARAMS);
	extern GeomMTDFunc	gGeomMTDMethodTable[][7];
}
}

#endif
