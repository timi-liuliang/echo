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

#ifndef GU_OVERLAP_TESTS_H
#define GU_OVERLAP_TESTS_H

#include "PxPhysXCommonConfig.h"
#include "CmPhysXCommon.h"
#include "PxVec3.h"

namespace physx
{
class PxGeometry;
class PxBoxGeometry;
class PxCapsuleGeometry;
class PxSphereGeometry;
class PxPlaneGeometry;
class PxConvexMeshGeometry;
class PxTriangleMeshGeometry;
class PxHeightFieldGeometry;
class PxMeshScale;
class PxPlane;

namespace Gu
{
	class Capsule;
	class Plane;
	class Sphere;
	class Box;
	class HeightFieldUtil;
	class ConvexMesh;
	class RTreeMidphase;

	PX_PHYSX_COMMON_API bool intersectPlaneBox 		(const PxPlane& plane, const Gu::Box& box);
	PX_PHYSX_COMMON_API bool intersectPlaneCapsule	(const Gu::Capsule& capsule, const PxPlane& plane);
	PX_PHYSX_COMMON_API bool intersectSphereSphere	(const Gu::Sphere& sphere0, const Gu::Sphere& sphere1);
	PX_PHYSX_COMMON_API bool intersectSphereCapsule	(const Gu::Sphere& sphere, const Gu::Capsule& capsule);
	PX_PHYSX_COMMON_API bool intersectSphereBox		(const Gu::Sphere& sphere, const Gu::Box& box);
	PX_PHYSX_COMMON_API bool intersectBoxCapsule	(const Gu::Box& box, const Gu::Capsule& capsule);

//	PX_PHYSX_COMMON_API bool intersectSphereConvex	(const PxSphereGeometry& sphereGeom, const PxTransform& sphereGlobalPose, const Gu::ConvexMesh& mesh, const PxMeshScale& meshScale, const PxTransform& convexGlobalPose, PxVec3* cachedSepAxis);
	PX_PHYSX_COMMON_API bool intersectSphereConvex	(const Gu::Sphere& sphere,			const Gu::ConvexMesh& mesh, const PxMeshScale& meshScale, const PxTransform& convexGlobalPose, PxVec3* cachedSepAxis);
	PX_PHYSX_COMMON_API bool intersectCapsuleConvex	(const PxCapsuleGeometry& capsGeom,	const PxTransform& capsGlobalPose, const Gu::ConvexMesh& mesh, const PxMeshScale& meshScale, const PxTransform& convexGlobalPose, PxVec3* cachedSepAxis);
	PX_PHYSX_COMMON_API bool intersectBoxConvex		(const PxBoxGeometry& boxGeom,		const PxTransform& boxGlobalPose, const Gu::ConvexMesh& mesh, const PxMeshScale& meshScale, const PxTransform& convexGlobalPose, PxVec3* cachedSepAxis);

	PX_PHYSX_COMMON_API PxU32 findOverlapSphereMesh	(const Gu::Sphere& worldSphere,		const Gu::RTreeMidphase& meshModel, const PxTransform& meshTransform, const PxMeshScale& scaling, PxU32* PX_RESTRICT results, PxU32 maxResults, PxU32 startIndex, bool& overflow);
	PX_PHYSX_COMMON_API PxU32 findOverlapCapsuleMesh(const Gu::Capsule& worldCapsule,	const Gu::RTreeMidphase& meshModel, const PxTransform& meshTransform, const PxMeshScale& scaling, PxU32* PX_RESTRICT results, PxU32 maxResults, PxU32 startIndex, bool& overflow);
	PX_PHYSX_COMMON_API PxU32 findOverlapOBBMesh	(const Gu::Box& worldOBB,			const Gu::RTreeMidphase& meshModel, const PxTransform& meshTransform, const PxMeshScale& scaling, PxU32* PX_RESTRICT results, PxU32 maxResults, PxU32 startIndex, bool& overflow);

	PX_PHYSX_COMMON_API bool intersectHeightFieldSphere		(const Gu::HeightFieldUtil& hfUtil, const Gu::Sphere& sphereInHfShape);
	PX_PHYSX_COMMON_API bool intersectHeightFieldCapsule	(const Gu::HeightFieldUtil& hfUtil, const Gu::Capsule& capsuleInHfShape);
	PX_PHYSX_COMMON_API bool intersectHeightFieldBox		(const Gu::HeightFieldUtil& hfUtil, const Gu::Box& boxInHfShape);
	PX_PHYSX_COMMON_API bool intersectHeightFieldConvex		(const Gu::HeightFieldUtil& hfUtil, const PxTransform& hfAbsPose, const Gu::ConvexMesh& convexMesh, const PxTransform& convexAbsPose, const PxMeshScale& convexMeshScaling);

	PX_PHYSX_COMMON_API bool checkOverlapSphere_boxGeom			(const PxGeometry& boxGeom,		const PxTransform& pose, const Gu::Sphere& sphere);
	PX_PHYSX_COMMON_API bool checkOverlapSphere_sphereGeom		(const PxGeometry& sphereGeom,	const PxTransform& pose, const Gu::Sphere& sphere);
	PX_PHYSX_COMMON_API bool checkOverlapSphere_capsuleGeom		(const PxGeometry& capsuleGeom,	const PxTransform& pose, const Gu::Sphere& sphere);
	PX_PHYSX_COMMON_API bool checkOverlapSphere_planeGeom		(const PxGeometry& planeGeom,	const PxTransform& pose, const Gu::Sphere& sphere);
	PX_PHYSX_COMMON_API bool checkOverlapSphere_convexGeom		(const PxGeometry& cvGeom,		const PxTransform& pose, const Gu::Sphere& sphere);
	PX_PHYSX_COMMON_API bool checkOverlapSphere_heightFieldGeom	(const PxGeometry& hfGeom,		const PxTransform& pose, const Gu::Sphere& sphere);

	PX_PHYSX_COMMON_API bool checkOverlapOBB_boxGeom			(const PxGeometry& boxGeom,		const PxTransform& pose, const Gu::Box& box);
	PX_PHYSX_COMMON_API bool checkOverlapOBB_sphereGeom			(const PxGeometry& sphereGeom,	const PxTransform& pose, const Gu::Box& box);
	PX_PHYSX_COMMON_API bool checkOverlapOBB_capsuleGeom		(const PxGeometry& capsuleGeom,	const PxTransform& pose, const Gu::Box& box);
	PX_PHYSX_COMMON_API bool checkOverlapOBB_planeGeom			(const PxGeometry& planeGeom,	const PxTransform& pose, const Gu::Box& box);
	PX_PHYSX_COMMON_API bool checkOverlapOBB_convexGeom			(const PxGeometry& cvGeom,		const PxTransform& pose, const Gu::Box& box);
	PX_PHYSX_COMMON_API bool checkOverlapOBB_triangleGeom		(const PxGeometry& triGeom,		const PxTransform& pose, const Gu::Box& box);
	PX_PHYSX_COMMON_API bool checkOverlapOBB_heightFieldGeom	(const PxGeometry& hfGeom,		const PxTransform& pose, const Gu::Box& box);

	PX_PHYSX_COMMON_API bool checkOverlapCapsule_boxGeom		(const PxGeometry& boxGeom,		const PxTransform& pose, const Gu::Capsule& worldCapsule);
	PX_PHYSX_COMMON_API bool checkOverlapCapsule_sphereGeom		(const PxGeometry& sphereGeom,	const PxTransform& pose, const Gu::Capsule& worldCapsule);
	PX_PHYSX_COMMON_API bool checkOverlapCapsule_capsuleGeom	(const PxGeometry& capsuleGeom,	const PxTransform& pose, const Gu::Capsule& worldCapsule);
	PX_PHYSX_COMMON_API bool checkOverlapCapsule_planeGeom		(const PxGeometry& planeGeom,	const PxTransform& pose, const Gu::Capsule& worldCapsule);
	PX_PHYSX_COMMON_API bool checkOverlapCapsule_convexGeom		(const PxGeometry& cvGeom,		const PxTransform& pose, const Gu::Capsule& worldCapsule);
	PX_PHYSX_COMMON_API bool checkOverlapCapsule_heightFieldGeom(const PxGeometry& hfGeom,		const PxTransform& pose, const Gu::Capsule& worldCapsule);

	// PT: this is just a shadow of what it used to be. We currently don't use TRIGGER_INSIDE anymore, but I leave it for now,
	// since I really want to put this back the way it was before.
	enum TriggerStatus
	{
		TRIGGER_DISJOINT,
		TRIGGER_INSIDE,
		TRIGGER_OVERLAP
	};

	// PT: currently only used for convex triggers
	struct TriggerCache
	{
		PxVec3	dir;
		PxU16	state;
		PxU16	gjkState; //gjk succeed or fail
	};

	// PT: this is used both for Gu overlap queries and for triggers. Please do not duplicate that code.
	#define GEOM_OVERLAP_CALLBACK_PARAMS \
		const PxGeometry& geom0, const PxTransform& transform0, const PxGeometry& geom1, const PxTransform& transform1, \
		Gu::TriggerCache* cache


	typedef bool (*GeomOverlapFunc)	(GEOM_OVERLAP_CALLBACK_PARAMS);

	typedef GeomOverlapFunc GeomOverlapTableEntry7[7];
	// not const because HFs are dynamically registered in this
	PX_PHYSX_COMMON_API GeomOverlapTableEntry7* GetGeomOverlapMethodTable();


	// dynamic registration of height fields
	PX_PHYSX_COMMON_API void registerHeightFields();
}  // namespace Gu

}

#endif
