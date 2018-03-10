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

#ifndef GU_SWEEP_MTD_H
#define GU_SWEEP_MTD_H


namespace physx
{
namespace Gu
{
	class Sphere;
	class Capsule;

	bool computeCapsule_TriangleMeshMTD(const PxTriangleMeshGeometry& triMeshGeom, const PxTransform& pose, Gu::CapsuleV& capsuleV, const PxReal inflatedRadius, const PxReal distance, const bool isDoubleSided,  PxSweepHit& hit);

	bool computeCapsule_HeightFieldMTD(const PxHeightFieldGeometry& heightFieldGeom, const PxTransform& pose, Gu::CapsuleV& capsuleV, const PxReal inflatedRadius, const PxReal distance, const bool isDoubleSided, const PxU32 flags, PxSweepHit& hit);

	bool computeBox_TriangleMeshMTD(const PxTriangleMeshGeometry& triMeshGeom, const PxTransform& pose, const Gu::Box& box, const PxTransform& boxTransform, const PxReal inflation,
						  const PxReal distance, const bool isDoubleSided,  PxSweepHit& hit);    

	bool computeBox_HeightFieldMTD(const PxHeightFieldGeometry& heightFieldGeom, const PxTransform& pose, const Gu::Box& box, const PxTransform& boxTransform, const PxReal inflation, 
						  const PxReal distance, const bool isDoubleSided, const PxU32 flags, PxSweepHit& hit);

	bool computeConvex_TriangleMeshMTD(const PxTriangleMeshGeometry& triMeshGeom, const PxTransform& pose, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexTransform, const PxReal inflation,
						  const PxReal distance, const bool isDoubleSided, PxSweepHit& hit);

	bool computeConvex_HeightFieldMTD(const PxHeightFieldGeometry& heightFieldGeom, const PxTransform& pose, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexTransform, const PxReal inflation,
						  const PxReal distance, const bool isDoubleSided, const PxU32 flags, PxSweepHit& hit);

	bool computeSphere_SphereMTD(const Sphere& sphere0, const Sphere& sphere1, PxSweepHit& hit);
	bool computeSphere_CapsuleMTD(const Sphere& sphere, const Capsule& capsule, PxSweepHit& hit);

	bool computeCapsule_CapsuleMTD(const Capsule& capsule0, const Capsule& capsule1, PxSweepHit& hit);

	bool computePlane_CapsuleMTD(const PxPlane& plane, const Capsule& capsule, PxSweepHit& hit);
	bool computePlane_BoxMTD(const PxPlane& plane, const Box& box, PxSweepHit& hit);
	bool computePlane_ConvexMTD(const PxPlane& plane, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexPose, PxSweepHit& hit);
}

}


#endif
