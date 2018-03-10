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

#ifndef GU_SWEEP_TESTS_H
#define GU_SWEEP_TESTS_H

#include "CmPhysXCommon.h"
#include "PxQueryReport.h"

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
class PxTriangle;
struct PxSweepHit;

//namespace Cm
//{
//	class RenderOutput;
//}

namespace Gu
{
	class Capsule;
	class Box;

#define GU_CAPSULE_SWEEP_FUNC_PARAMS	const PxGeometry& geom,	const PxTransform& pose, \
		const Gu::Capsule& lss, const PxVec3& unitDir, const PxReal distance, \
		PxSweepHit& sweepHit, PxHitFlags hintFlags, const PxReal inflation 

#define GU_BOX_SWEEP_FUNC_PARAMS	const PxGeometry& geom,	const PxTransform& pose,   \
		const Gu::Box& box, const PxVec3& unitDir, const PxReal distance,    \
		PxSweepHit& sweepHit, PxHitFlags hintFlags, const PxReal inflation

	//PX_PHYSX_COMMON_API void setDebugRender(Cm::RenderOutput& output);

	typedef bool (*SweepCapsuleFunc) (GU_CAPSULE_SWEEP_FUNC_PARAMS);
	typedef bool (*SweepBoxFunc) (GU_BOX_SWEEP_FUNC_PARAMS);
	typedef bool (*SweepConvexFunc) (const PxGeometry&, const PxTransform&, const PxConvexMeshGeometry&,
									const PxTransform&, const PxVec3& unitDir, const PxReal distance, PxSweepHit&,
									PxHitFlags hintFlags, const PxReal inflation);
	typedef SweepCapsuleFunc	GeomSweepCapsuleTable7[7];
	typedef SweepBoxFunc		GeomSweepBoxTable7[7];
	typedef SweepConvexFunc		GeomSweepConvexTable7[7];

	struct GeomSweepFuncs
	{
		GeomSweepCapsuleTable7	capsuleMap;
		GeomSweepCapsuleTable7	cctCapsuleMap;
		GeomSweepBoxTable7		boxMap;
		GeomSweepBoxTable7		cctBoxMap;
		GeomSweepConvexTable7	convexMap;
	};

	PX_PHYSX_COMMON_API const GeomSweepFuncs& GetGeomSweepFuncs();

	// For sweep vs. triangle list: PxGeometryQuery::sweep()
	bool SweepCapsuleTriangles(	PxU32 nbTris, const PxTriangle* triangles, bool doubleSided,
								const PxCapsuleGeometry& capsuleGeom, const PxTransform& capsulePose,
								const PxVec3& unitDir, const PxReal distance, const PxU32* cachedIndex, PxVec3& hit,
								PxVec3& normal, PxReal& d, PxU32& index, const PxReal inflation, PxHitFlags hintFlags);

	// For sweep vs. triangle list: PxGeometryQuery::sweep()
	bool SweepBoxTriangles(PxU32 nbTris, const PxTriangle* triangles, bool doubleSided,
							const PxBoxGeometry& boxGeom, const PxTransform& boxPose, const PxVec3& unitDir, const PxReal distance, 
							PxVec3& _hit, PxVec3& _normal, float& _d, PxU32& _index, const PxU32* cachedIndex, const PxReal inflation, PxHitFlags hintFlags);

	bool sweepCCTBoxTriangles(	PxU32 nbTris, const PxTriangle* triangles, bool doubleSided,
								const PxBoxGeometry& boxGeom, const PxTransform& boxPose, const PxVec3& unitDir, const PxReal distance, 
								PxVec3& _hit, PxVec3& _normal, float& _d, PxU32& _index, const PxU32* cachedIndex, const PxReal inflation,
								PxHitFlags hintFlags);

}  // namespace Gu

}

#endif
