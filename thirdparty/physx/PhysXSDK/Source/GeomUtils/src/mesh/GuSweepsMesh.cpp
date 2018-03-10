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

#include "GuSweepTests.h"
#include "GuSPUHelpers.h"
#include "GuMidphase.h"
#include "GuSweepSharedTests.h"
#include "GuConvexUtilsInternal.h"
#include "CmScaling.h"
#include "GuVecBox.h"
#include "GuSweepMTD.h"
#include "GuVecCapsule.h"
#include "GuSweepBoxTriangle_SAT.h"
#include "GuSweepTriangleUtils.h"
#include "GuSweepCapsuleTriangle.h"
#include "GuDistancePointTriangle.h"
#include "GuSweepSphereTriangle.h"
#include "CmMidphaseTrace.h"

using namespace physx;
using namespace Gu;
using namespace Cm;
using namespace physx::shdfnd::aos;

#include "GuSweepConvexTri.h"

static bool sweepSphereTriangle(const PxTriangle& tri,															// Triangle data
								const PxVec3& sphereCenter, float sphereRadius,									// Sphere data
								const PxVec3& unitDir, const PxReal distance,									// Ray data
								PxHitFlags& outFlags, PxF32& t, PxVec3& normal, PxVec3& hit, PxU32& hitIndex,	// Results
								PxVec3& _triNormal,
								PxHitFlags hintFlags, bool isDoubleSided)										// Query modifiers
{
	const bool meshBothSides = hintFlags & PxHitFlag::eMESH_BOTH_SIDES;

	if(!(hintFlags & PxHitFlag::eASSUME_NO_INITIAL_OVERLAP))
	{
		// PT: test if shapes initially overlap
		// PT: add culling here for now, but could be made more efficiently...

		// Create triangle normal
		PxVec3 denormalizedNormal;
		tri.denormalizedNormal(denormalizedNormal);

		// Backface culling
		const bool doBackfaceCulling = !isDoubleSided && !meshBothSides;
		if(doBackfaceCulling && (denormalizedNormal.dot(unitDir) > 0.0f))
			return false;

		float s_unused, t_unused;
		const PxVec3 cp = closestPtPointTriangle(sphereCenter, tri.verts[0], tri.verts[1], tri.verts[2], s_unused, t_unused);
		const PxReal dist2 = (cp - sphereCenter).magnitudeSquared();
		if(dist2<=sphereRadius*sphereRadius)
		{
			hitIndex	= 0;
			t			= 0.0f;
			normal		= -unitDir;
			outFlags	= PxHitFlag::eDISTANCE|PxHitFlag::eNORMAL;
			_triNormal	= denormalizedNormal.getNormalized();
			return true;
		}
	}
	return sweepSphereTriangles(1, &tri, sphereCenter, sphereRadius, unitDir, distance, NULL, hit, normal, t, hitIndex, outFlags, _triNormal, isDoubleSided, meshBothSides);
}

struct SweepCapsuleCallback : MeshHitCallback<PxRaycastHit>
{		
	PxSweepHit&			hit;
	const Matrix34&		vertexToWorldSkew;
	PxReal				trueSweepDistance;
	PxReal				trueSweepShrunkDistance;
	PxReal				distCoeff;		// dist coeff from unscaled to scaled distance
	bool				bDoubleSide;		
	const Capsule&		capsule;
	const PxVec3&		unitDir;
	const PxHitFlags&	hintFlags;
	PxReal				mostOpposingHitDot, bestDist;
	bool				status;
	bool				initialOverlapStatus;
	bool				isSphere;

	SweepCapsuleCallback(
		PxSweepHit& sweepHit, const Matrix34& worldMatrix, PxReal distance, PxReal distCoeff_, bool doubleSide, 
		const Capsule& capsule_, const PxVec3& unitDir_, const PxHitFlags& hintFlags_)
		:	MeshHitCallback<PxRaycastHit>(CallbackMode::eMULTIPLE),
			hit(sweepHit), vertexToWorldSkew(worldMatrix), trueSweepDistance(distance), trueSweepShrunkDistance(distance),
			distCoeff(distCoeff_), bDoubleSide(doubleSide), capsule(capsule_), unitDir(unitDir_),
			hintFlags(hintFlags_),
			mostOpposingHitDot(2.0f), bestDist(PX_MAX_REAL),
			status(false), initialOverlapStatus(false)
	{
		hit.distance = PX_MAX_REAL;
		isSphere = capsule_.p0 == capsule_.p1;
	}

	virtual PxAgain processHit( // all reported coords are in mesh local space including hit.position
		const PxRaycastHit& aHit, const PxVec3& v0, const PxVec3& v1, const PxVec3& v2, PxReal& shrunkMaxT, const PxU32*)
	{
		const PxTriangle tmpt(	vertexToWorldSkew.transform(v0),
								vertexToWorldSkew.transform(v1),
								vertexToWorldSkew.transform(v2));

		float localDist;
		PxVec3 localPos, localNormal, triNormal;
		PxU32 localIndex;
		const PxReal distEpsilon = GU_EPSILON_SAME_DISTANCE; // pick a farther hit within distEpsilon that is more opposing than the previous closest hit
		// PT: PxMin is here to ensure we discard hits further away than user's max dist (captured in trueSweepDistance).
		// For example if we first hit a triangle at maxDist-eps/2 = localDist, next time trueSweepShrunkDistance+eps would equal
		// maxDist-eps/2+eps = maxDist+eps/2, which is too much.
		const float minD = PxMin(trueSweepDistance, trueSweepShrunkDistance+distEpsilon);
		if(isSphere)
		{
			if(!::sweepSphereTriangle(	tmpt, capsule.p0, capsule.radius, unitDir, minD, hit.flags,
										localDist, localNormal, localPos, localIndex, triNormal, hintFlags, bDoubleSide))
			{
				return true;
			}
		}
		else
		{
			if(!sweepCapsuleTriangles(	1, &tmpt, capsule, unitDir, minD, NULL, hit.flags,
										localDist, localNormal, localPos, localIndex, triNormal, hintFlags, bDoubleSide, NULL))
			{
				return true;
			}
		}

		const PxReal hitDot1 = computeAlignmentValue(triNormal, unitDir);
		if(keepTriangle(localDist, hitDot1, bestDist, mostOpposingHitDot, distEpsilon))
		{
			mostOpposingHitDot = hitDot1; // arbitrary bias. works for hitDot1=-1, prevHitDot=0

			// AP: need to shrink the sweep distance passed into sweepCapsuleTriangles for correctness so that next sweep is closer
			trueSweepShrunkDistance = localDist;
			shrunkMaxT = localDist * distCoeff; // shrunkMaxT is scaled

			// PT: TODO: isn't 'bestDist' the same as 'hit.distance' and 'trueSweepShrunkDistance'?
			bestDist = PxMin(bestDist, localDist); // exact lower bound
			hit.distance = localDist; // approximate lower bound within +/-distEpsilon
			hit.normal = localNormal;
			hit.position = localPos;
			hit.faceIndex = aHit.faceIndex;
			status = true;
			//ML:this is the initial overlap condition
			if(localDist == 0.f)
			{
				initialOverlapStatus = true;
				return false;
			}
			if(hintFlags & PxHitFlag::eMESH_ANY)
				return false; // abort traversal
		}
		return true;
	}
	void operator=(const SweepCapsuleCallback&) {}


	bool finalize(
		PxSweepHit& sweepHit, const Capsule& lss, const PxTriangleMeshGeometry& triMeshGeom,
		const PxTransform& pose, bool isDoubleSided, PxReal distance)
	{
		if(status)
		{
			if(initialOverlapStatus)
			{
				if((!PX_IS_SPU) && (hintFlags & PxHitFlag::eMTD))
				{
					const Vec3V p0 = V3LoadU(capsule.p0);
					const Vec3V p1 = V3LoadU(capsule.p1);
					const FloatV radius = FLoad(lss.radius);
					CapsuleV capsuleV;
					capsuleV.initialize(p0, p1, radius);

					//we need to calculate the MTD
					const bool hasContacts = computeCapsule_TriangleMeshMTD(
						triMeshGeom, pose, capsuleV, capsule.radius,  distance, isDoubleSided, sweepHit);

					sweepHit.flags = PxHitFlag::eDISTANCE | PxHitFlag::eNORMAL;
					//ML: the center of mass is below the surface, we won't have MTD contact generate
					if(!hasContacts)
					{
						sweepHit.distance	= 0.0f;
						sweepHit.normal		= -unitDir;
					}
					else
					{
						sweepHit.flags |= PxHitFlag::ePOSITION;
					}
				}
				else
				{	
					sweepHit.distance	= 0.0f;
					sweepHit.normal		= -unitDir;
					sweepHit.flags		= PxHitFlag::eDISTANCE | PxHitFlag::eNORMAL;
				}
			}
			else // if (initialOverlapStatus)
			{
				sweepHit.flags = PxHitFlag::eDISTANCE | PxHitFlag::eNORMAL | PxHitFlag::ePOSITION;
			}

			return true;
		} // if (status)

		return false;
	}
};

bool sweepCapsule_MeshGeom(GU_CAPSULE_SWEEP_FUNC_PARAMS)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eTRIANGLEMESH);
	const PxTriangleMeshGeometry& triMeshGeom = static_cast<const PxTriangleMeshGeometry&>(geom);

	GU_FETCH_MESH_DATA(triMeshGeom);

	// Collide OBB against current mesh
	const PxU32 meshBothSides = hintFlags & PxHitFlag::eMESH_BOTH_SIDES;
	const bool isDoubleSided = (triMeshGeom.meshFlags & PxMeshGeometryFlag::eDOUBLE_SIDED) || meshBothSides;

	RTreeMidphaseData data;
	meshData->mCollisionModel.getRTreeMidphaseData(data);

	const Capsule inflatedCapsule(lss.p0, lss.p1, lss.radius + inflation);

	// compute sweptAABB
	PxVec3 localP0 = pose.transformInv(inflatedCapsule.p0), localP1 = pose.transformInv(inflatedCapsule.p1);
	PxVec3 sweepOrigin = (localP0+localP1)*0.5f;
	PxVec3 sweepDir = pose.rotateInv(unitDir);
	PxVec3 sweepExtents = PxVec3(inflatedCapsule.radius) + (localP0-localP1).abs()*0.5f;
	PxReal distance1 = distance;
	PxReal distCoeff = 1.0f;
	Matrix34 poseWithScale;
	if(!triMeshGeom.scale.isIdentity())
	{
		poseWithScale = pose * triMeshGeom.scale;
		Cm::FastVertex2ShapeScaling meshScaling(triMeshGeom.scale.getInverse()); // shape to vertex transform
		PxBounds3 originBounds = meshScaling.transformBounds(PxBounds3::centerExtents(sweepOrigin, sweepExtents));
		PxBounds3 endBounds = meshScaling.transformBounds(PxBounds3::centerExtents(sweepOrigin + sweepDir*distance, sweepExtents));
		sweepOrigin = originBounds.getCenter();
		sweepExtents = originBounds.getExtents();
		sweepDir = endBounds.getCenter() - sweepOrigin;
		distance1 = sweepDir.normalizeSafe();
		distCoeff = distance1 / distance;
	} else
		poseWithScale = Matrix34(pose);

	SweepCapsuleCallback callback(sweepHit, poseWithScale, distance, distCoeff, isDoubleSided, inflatedCapsule, unitDir, hintFlags);
	MPT_SET_CONTEXT("swcm", pose, triMeshGeom.scale);
	MeshRayCollider::collide<1,1>(sweepOrigin, sweepDir, distance1, true, data, callback, &sweepExtents);

	return callback.finalize(sweepHit, inflatedCapsule, triMeshGeom, pose, isDoubleSided, distance);
}

	// PT: class to make sure we can safely V4Load Matrix34's last column
	class Matrix34Padded : public Matrix34
	{
		public:
			PX_FORCE_INLINE	Matrix34Padded(const Matrix34& src) : Matrix34(src)	{}
			PX_FORCE_INLINE	Matrix34Padded()									{}
			PX_FORCE_INLINE	~Matrix34Padded()									{}
			PxU32	padding;
	};
	PX_COMPILE_TIME_ASSERT(0==(sizeof(Matrix34Padded)==16));

	// same as 'mat.transform(p)' but using SIMD
	static PX_FORCE_INLINE Vec4V transformV(const Vec4V p, const Matrix34Padded& mat)
	{
		Vec4V ResV = V4Scale(V4LoadU(&mat.base0.x), V4GetX(p));
		ResV = V4ScaleAdd(V4LoadU(&mat.base1.x), V4GetY(p), ResV);
		ResV = V4ScaleAdd(V4LoadU(&mat.base2.x), V4GetZ(p), ResV);
		ResV = V4Add(ResV, V4LoadU(&mat.base3.x));	// PT: this load is safe thanks to padding
		return ResV;
	}

#if defined(PX_VC) 
    #pragma warning(push)
	#pragma warning( disable : 4324 ) // Padding was added at the end of a structure because of a __declspec(align) value.
#endif

struct SweepBoxMeshHitCallback : MeshHitCallback<PxRaycastHit>
{		
	const Matrix34Padded&	meshToBox;
	PxReal				dist, dist0, distCoeff; // dist coeff from unscaled to scaled distance
	FloatV				distV;
	bool				bDoubleSide;		
	const Box&			box;
	float				boxRadius;
	const PxVec3&		localMotion;
	const PxVec3&		localDir;
	const PxVec3&		worldUnitDir;
	const PxHitFlags&	hintFlags;
	bool				status;
	bool				initialOverlap;
	PxReal				inflation;
	PxTriangle			hitTriangle;
	Vec3V				minClosestA;
	Vec3V				minNormal;
	Vec3V				localDirV;
	Vec3V				localMotionV;
	PxU32				minTriangleIndex;
	PxVec3				oneOverDir;

	SweepBoxMeshHitCallback(
		CallbackMode::Enum _mode, const Matrix34Padded& _meshToBox, PxReal distance, PxReal distCoeff_, bool doubleSide, 
		const Box& _box, const PxVec3& _localMotion, const PxVec3& _localDir, const PxVec3& unitDir,
		const PxHitFlags& _hintFlags, const PxReal _inflation) 
		:	MeshHitCallback<PxRaycastHit>(_mode), meshToBox(_meshToBox), dist(distance), distCoeff(distCoeff_),
			bDoubleSide(doubleSide), box(_box), localMotion(_localMotion), localDir(_localDir),
			worldUnitDir(unitDir), hintFlags(_hintFlags), status(false), initialOverlap(false), inflation(_inflation)
	{
		//pxPrintf("SweepBoxMeshHitCallback mtb=%x\n", PxU32(&meshToBox));
		localDirV = V3LoadU(localDir);
		localMotionV = V3LoadU(localMotion);
		distV = FLoad(distance);
		dist0 = distance;
		oneOverDir = PxVec3(
			localDir.x!=0.0f ? 1.0f/(localDir.x) : 0.0f,
			localDir.y!=0.0f ? 1.0f/(localDir.y) : 0.0f,
			localDir.z!=0.0f ? 1.0f/(localDir.z) : 0.0f);
	}

	virtual ~SweepBoxMeshHitCallback() {}

	virtual PxAgain processHit( // all reported coords are in mesh local space including hit.position
		const PxRaycastHit& meshHit, const PxVec3& lp0, const PxVec3& lp1, const PxVec3& lp2, PxReal& shrinkMaxT, const PxU32*)
	{
		if (!PX_IS_SPU && (hintFlags & PxHitFlag::ePRECISE_SWEEP))
		{
			const PxTriangle currentTriangle(	// in world space
					meshToBox.transform(lp0),
					meshToBox.transform(lp1),
					meshToBox.transform(lp2));
			PxF32 t = PX_MAX_REAL; // PT: could be better!
			if(triBoxSweepTestBoxSpace(currentTriangle, box.extents, localDir, oneOverDir, dist, t, !bDoubleSide))
			{
				if(t <= dist)
				{
					// PT: test if shapes initially overlap
					dist				= t;
					shrinkMaxT			= t * distCoeff;   // shrinkMaxT is unscaled
					minClosestA			= V3LoadU(currentTriangle.verts[0]); // PT: this is arbitrary
					minNormal			= V3LoadU(-worldUnitDir);
					status				= true;
					minTriangleIndex	= meshHit.faceIndex;
					hitTriangle			= currentTriangle;
					if (t == 0.0f)
					{
						initialOverlap = true;
						return false; // abort traversal
					}
				}
			}
		}
		else
		{
			const FloatV zero = FZero();

			// Move to box space
			//
			// PT: SIMD code similar to:
			//	const Vec3V triV0 = V3LoadU(meshToBox.transform(lp0));
			//	const Vec3V triV1 = V3LoadU(meshToBox.transform(lp1));
			//	const Vec3V triV2 = V3LoadU(meshToBox.transform(lp2));
			//
			// SIMD version works but we need to ensure all loads are safe.
			// For incoming vertices they should either come from the vertex array or from a binary deserialized file.
			// For the vertex array we can just allocate one more vertex. For the binary file it should be ok as soon
			// as vertices aren't the last thing serialized in the file.
			// For the matrix only the last column is a problem, and we can easily solve that with some padding in the local class.
			const Vec3V triV0 = Vec3V_From_Vec4V(transformV(V4LoadU(&lp0.x), meshToBox));
			const Vec3V triV1 = Vec3V_From_Vec4V(transformV(V4LoadU(&lp1.x), meshToBox));
			const Vec3V triV2 = Vec3V_From_Vec4V(transformV(V4LoadU(&lp2.x), meshToBox));

			if(!bDoubleSide)
			{
				const Vec3V triNormal = V3Cross(V3Sub(triV2, triV1),V3Sub(triV0, triV1)); 
				if(FAllGrtrOrEq(V3Dot(triNormal, localMotionV), zero))
					return true;
			}

			const Vec3V zeroV = V3Zero();
			const Vec3V boxExtents = V3LoadU(box.extents);
			BoxV boxV(zeroV, boxExtents);

			TriangleV triangleV(triV0, triV1, triV2);
	
			FloatV lambda;   
			Vec3V closestA, normal;//closestA and normal is in the local space of convex hull
			//pxPrintf("calling gjk raycast\n");
			bool gjkHit  = GJKLocalRayCast(triangleV, boxV, zero, zeroV, localMotionV, lambda, normal, closestA, inflation, false); 
			//pxPrintf("done gjk raycast\n");
			if(gjkHit)
			{
				status = true;
				minClosestA = closestA;
				minTriangleIndex = meshHit.faceIndex;
				if(FAllGrtrOrEq(zero, lambda)) // lambda < 0? => initial overlap
				{
					initialOverlap = true;
					shrinkMaxT = 0.0f;
					distV = zero;
					dist = 0.0f;
					minNormal = V3LoadU(-worldUnitDir);
					return false;
				}

				dist = FStore(lambda)*dist; // shrink dist
				localMotionV = V3Scale(localMotionV, lambda); // shrink localMotion
				distV = FMul(distV,lambda); // shrink distV
				minNormal = normal;
				if (dist * distCoeff < shrinkMaxT) // shrink shrinkMaxT
					shrinkMaxT = dist * distCoeff; // shrinkMaxT is scaled
			}
		}

		//pxPrintf("returning from processHits\n");
		return true;
	}

private:
	SweepBoxMeshHitCallback& operator=(const SweepBoxMeshHitCallback&);
};	

bool sweepBox_MeshGeom(GU_BOX_SWEEP_FUNC_PARAMS)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eTRIANGLEMESH);
	const PxTriangleMeshGeometry& triMeshGeom = static_cast<const PxTriangleMeshGeometry&>(geom);

	Matrix34 meshToWorldSkew;
	PxVec3 sweptAABBMeshSpaceExtents, meshSpaceOrigin, meshSpaceDir;

	// Input sweep params: geom, pose, box, unitDir, distance
	// We convert the origin from world space to mesh local space
	// and convert the box+pose to mesh space AABB
	if(triMeshGeom.scale.isIdentity())
	{
		meshToWorldSkew = Matrix34(pose);
		PxMat33 worldToMeshRot(pose.q.getConjugate()); // extract rotation matrix from pose.q
		meshSpaceOrigin = worldToMeshRot.transform(box.center - pose.p);
		meshSpaceDir = worldToMeshRot.transform(unitDir) * distance;
		PxMat33 boxToMeshRot = worldToMeshRot * box.rot;
		sweptAABBMeshSpaceExtents = boxToMeshRot.column0.abs() * box.extents.x + 
						   boxToMeshRot.column1.abs() * box.extents.y + 
						   boxToMeshRot.column2.abs() * box.extents.z;
	}
	else
	{
		meshToWorldSkew = pose * triMeshGeom.scale;
		const PxMat33 meshToWorldSkew_Rot = PxMat33(pose.q) * triMeshGeom.scale.toMat33();
		const PxVec3& meshToWorldSkew_Trans = pose.p;

		PxMat33 worldToVertexSkew_Rot;
		PxVec3 worldToVertexSkew_Trans;
		getInverse(worldToVertexSkew_Rot, worldToVertexSkew_Trans, meshToWorldSkew_Rot, meshToWorldSkew_Trans);

		//make vertex space OBB
		Box vertexSpaceBox1;
		const Matrix34 worldToVertexSkew(worldToVertexSkew_Rot, worldToVertexSkew_Trans);
		vertexSpaceBox1 = transform(worldToVertexSkew, box);
		// compute swept aabb
		sweptAABBMeshSpaceExtents = vertexSpaceBox1.computeAABBExtent();

		meshSpaceOrigin = worldToVertexSkew.transform(box.center);
		meshSpaceDir = worldToVertexSkew.rotate(unitDir*distance); // also applies scale to direction/length
	}

	sweptAABBMeshSpaceExtents += PxVec3(inflation); // inflate the bounds with additive inflation
	sweptAABBMeshSpaceExtents *= 1.01f; // fatten the bounds to account for numerical discrepancies

	GU_FETCH_MESH_DATA(triMeshGeom);

	RTreeMidphaseData hmd;
	const RTreeMidphase& collisionModel = meshData->mCollisionModel;
	collisionModel.getRTreeMidphaseData(hmd);

	PxReal dirLen = PxMax(meshSpaceDir.magnitude(), 1e-5f);

	PxReal distCoeff = 1.0f; 
	if(!triMeshGeom.scale.isIdentity())
		distCoeff = dirLen / distance;

	// Move to AABB space
	Matrix34 worldToBox;
	computeWorldToBoxMatrix(worldToBox, box);

	bool status = false;
	const PxU32 meshBothSides = hintFlags & PxHitFlag::eMESH_BOTH_SIDES;
	const bool isDoubleSided = (triMeshGeom.meshFlags & PxMeshGeometryFlag::eDOUBLE_SIDED) || meshBothSides;

	const Matrix34Padded meshToBox = worldToBox*meshToWorldSkew;
	const PxTransform boxTransform = box.getTransform();
	const Vec3V p0 = V3LoadU(&boxTransform.p.x);
	const QuatV q0 = QuatVLoadU(&boxTransform.q.x);
	const PsTransformV boxPos(p0, q0);

	const PxVec3 localDir = worldToBox.rotate(unitDir);
	const PxVec3 localDirDist = localDir*distance;
	SweepBoxMeshHitCallback callback( // using eMULTIPLE with shrinkMaxT
		CallbackMode::eMULTIPLE, meshToBox, distance, distCoeff, isDoubleSided, box, localDirDist, localDir, unitDir, hintFlags, inflation);

	//pxPrintf("mesh collider begin\n");
	MPT_SET_CONTEXT("swbm", pose, triMeshGeom.scale);
	MeshRayCollider::collide<1, 1>(meshSpaceOrigin, meshSpaceDir/dirLen, dirLen, isDoubleSided, hmd, callback, &sweptAABBMeshSpaceExtents);
	//pxPrintf("mesh collider done\n");

	status = callback.status;
	Vec3V minClosestA = callback.minClosestA;
	Vec3V minNormal = callback.minNormal;
	PxU32 minTriangleIndex = callback.minTriangleIndex;

	if(status)
	{
		if(callback.initialOverlap)
		{
			if(!(PX_IS_SPU) && (hintFlags & PxHitFlag::eMTD))
			{
				const bool hasContacts = computeBox_TriangleMeshMTD(triMeshGeom, pose, box, boxTransform, inflation, distance, isDoubleSided,  sweepHit);

				sweepHit.flags = PxHitFlag::eDISTANCE | PxHitFlag::eNORMAL;
				//ML: the center of mass is below the surface, we won't have MTD contact generate
				if(!hasContacts)
				{
					sweepHit.distance	= 0.0f;
					sweepHit.normal		= -unitDir;
				}
				else
				{
					sweepHit.flags |= PxHitFlag::ePOSITION;
				}
			}
			else
			{
				sweepHit.flags		= PxHitFlag::eDISTANCE | PxHitFlag::eNORMAL;
				sweepHit.distance	= 0.0f;
				sweepHit.normal		= -unitDir;
			}
			sweepHit.faceIndex = minTriangleIndex;
		}
		else
		{
			sweepHit.faceIndex	= minTriangleIndex;
			sweepHit.distance = callback.dist;
			sweepHit.flags = PxHitFlag::eDISTANCE | PxHitFlag::ePOSITION | PxHitFlag::eNORMAL;

			if ((hintFlags & PxHitFlag::ePRECISE_SWEEP) && !PX_IS_SPU)
			{
				const PxVec3 oneOverMotion(
					localDir.x!=0.0f ? 1.0f/localDirDist.x : 0.0f,
					localDir.y!=0.0f ? 1.0f/localDirDist.y : 0.0f,
					localDir.z!=0.0f ? 1.0f/localDirDist.z : 0.0f);

				computeBoxTriImpactData(sweepHit.position, sweepHit.normal, box.extents, localDir, localDirDist, oneOverMotion, callback.hitTriangle);

				sweepHit.normal.normalize();

				// computeImpactData computes sweepHit
				V3StoreU(boxPos.rotate(V3LoadU(sweepHit.normal)), sweepHit.normal);
				V3StoreU(boxPos.transform(V3LoadU(sweepHit.position)), sweepHit.position);
			}
			else
			{
				const Vec3V destNormal = V3Neg(boxPos.rotate(minNormal));
				const Vec3V destWorldPointA = boxPos.transform(minClosestA);
				V3StoreU(destNormal, sweepHit.normal);
				V3StoreU(destWorldPointA, sweepHit.position);
				sweepHit.faceIndex = minTriangleIndex;
			}
		}
		return true;
	}
	return false;
}


struct ConvexVsMeshSweepCallback : MeshHitCallback<PxRaycastHit>
{
	ConvexVsMeshSweepCallback(
		const ConvexHullData& hull, const PxMeshScale& convexScale, const FastVertex2ShapeScaling& meshScale,
		const PxTransform& convexPose, const PxTransform& meshPose,
		const PxVec3& unitDir, const PxReal distance, PxHitFlags hintFlags, const bool isDoubleSided, const PxReal inflation,
		const bool anyHit)
		:	MeshHitCallback<PxRaycastHit>(CallbackMode::eMULTIPLE),
			mMeshScale		(meshScale),
			mUnitDir		(unitDir),			
			mInflation		(inflation),
			mHintFlags		(hintFlags),
			mAnyHit			(anyHit),
			mIsDoubleSided	(isDoubleSided),
			mInitialOverlap (false)
			
	{
		mHit.distance = distance; // this will be shrinking progressively as we sweep and clip the sweep length
		mHit.faceIndex = 0xFFFFFFFF;

		mMeshSpaceUnitDir = meshPose.rotateInv(unitDir);
	
		const Vec3V worldDir = V3LoadU(unitDir);
		const FloatV dist = FLoad(distance);
		const QuatV q0 = QuatVLoadU(&meshPose.q.x);
		const Vec3V p0 = V3LoadU(&meshPose.p.x);

		const QuatV q1 = QuatVLoadU(&convexPose.q.x);
		const Vec3V p1 = V3LoadU(&convexPose.p.x);

		const PsTransformV meshPoseV(p0, q0);
		const PsTransformV convexPoseV(p1, q1);

		mMeshToConvex = convexPoseV.transformInv(meshPoseV);
		mConvexPoseV = convexPoseV;
		mConvexSpaceDir = convexPoseV.rotateInv(V3Neg(V3Scale(worldDir, dist)));
		mInitialDistance = dist;

		const Vec3V vScale = V3LoadU(convexScale.scale);
		const QuatV vQuat = QuatVLoadU(&convexScale.rotation.x);
		mConvexHull.initialize(&hull, V3Zero(), vScale, vQuat);

		mStatus = false;
	}
	virtual ~ConvexVsMeshSweepCallback()	{}

	virtual PxAgain processHit( // all reported coords are in mesh local space including hit.position
		const PxRaycastHit& hit, const PxVec3& av0, const PxVec3& av1, const PxVec3& av2, PxReal&, const PxU32*)
	{
		const PxVec3 v0 = mMeshScale * av0;
		const PxVec3 v1 = mMeshScale * av1;
		const PxVec3 v2 = mMeshScale * av2;

		// mHit will be updated if sweep distance is < input mHit.distance
		const PxReal oldDist = mHit.distance;
		if (sweepConvexVsTriangle(
			v0, v1, v2, mConvexHull, mMeshToConvex, mConvexPoseV, mConvexSpaceDir,
			mUnitDir, mMeshSpaceUnitDir, mInitialDistance, mHintFlags, oldDist, mHit, mIsDoubleSided, mInflation, mInitialOverlap))
		{

			mStatus = true;
			mHit.faceIndex = hit.faceIndex; // record the triangle index
			if (mAnyHit)
				return false; // abort traversal
			
			if(mHit.distance == 0.f)
				return false;
		}

		return true; // continue traversal
	}

	ConvexHullV						mConvexHull;
	PsMatTransformV					mMeshToConvex;
	PsTransformV					mConvexPoseV;
	const FastVertex2ShapeScaling&	mMeshScale;
	PxSweepHit						mHit; // stores either the closest or any hit depending on value of mAnyHit
	FloatV							mInitialDistance;
	Vec3V							mConvexSpaceDir; // convexPose.rotateInv(-unit*distance)
	PxVec3							mUnitDir;
	PxVec3							mMeshSpaceUnitDir;
	PxReal							mInflation;	
	PxHitFlags						mHintFlags;
	const bool						mAnyHit;
	const bool						mIsDoubleSided;
	bool							mInitialOverlap;
	bool							mStatus;

private:
	ConvexVsMeshSweepCallback& operator=(const ConvexVsMeshSweepCallback&);
};

#if defined(PX_VC) 
     #pragma warning(pop) 
#endif

bool sweepConvex_MeshGeom(
	const PxGeometry& aMeshGeom, const PxTransform& meshPose, const PxConvexMeshGeometry& convexGeom,
	const PxTransform& convexPose, const PxVec3& unitDir, const PxReal distance,
	PxSweepHit& sweepHit, PxHitFlags hintFlags, const PxReal inflation)
{
	PX_ASSERT(aMeshGeom.getType() == PxGeometryType::eTRIANGLEMESH);
	const PxTriangleMeshGeometry& meshGeom = static_cast<const PxTriangleMeshGeometry&>(aMeshGeom);

	FETCH_CONVEX_HULL_DATA(convexGeom)

	GU_FETCH_MESH_DATA(meshGeom);

	const bool idtScaleConvex = convexGeom.scale.isIdentity();
	const bool idtScaleMesh = meshGeom.scale.isIdentity();

	FastVertex2ShapeScaling convexScaling;
	if(!idtScaleConvex)
		convexScaling.init(convexGeom.scale);

	FastVertex2ShapeScaling meshScaling;
	if(!idtScaleMesh)
		meshScaling.init(meshGeom.scale);

	PX_ASSERT(!convexMesh->getLocalBoundsFast().isEmpty());
	PxBounds3 hullAABB = PxBounds3::transformFast(convexScaling.getVertex2ShapeSkew(), convexMesh->getLocalBoundsFast());

	Box hullOBB;
	computeHullOBB(hullOBB, hullAABB, 0.0f, convexPose, Matrix34(convexPose), Matrix34(meshPose), meshScaling, idtScaleMesh);
	//~PT: TODO: this part similar to convex-vs-overlap test, refactor

	hullOBB.extents.x += inflation;
	hullOBB.extents.y += inflation;
	hullOBB.extents.z += inflation;

	// Now create temporal bounds
	Box querySweptBox;
	computeSweptBox(querySweptBox, hullOBB.extents, hullOBB.center, hullOBB.rot, meshPose.rotateInv(unitDir), distance);

	const PxU32 meshBothSides = hintFlags & PxHitFlag::eMESH_BOTH_SIDES;
	const bool isDoubleSided = (meshGeom.meshFlags & PxMeshGeometryFlag::eDOUBLE_SIDED) || meshBothSides;
	const bool isMtd = hintFlags & PxHitFlag::eMTD;
	ConvexVsMeshSweepCallback callback(
		convexMesh->getHull(), convexGeom.scale, meshScaling, convexPose, meshPose, -unitDir, distance, hintFlags,
		isDoubleSided, inflation, hintFlags.isSet(PxHitFlag::eMESH_ANY));

	// AP: careful with changing the template params - can negatively affect SPU_Sweep module size
	RTreeMidphaseData rtreeData;
	meshData->mCollisionModel.getRTreeMidphaseData(rtreeData);
	MPT_SET_CONTEXT("swxm", meshPose, meshGeom.scale);
	MeshRayCollider::collideOBB(querySweptBox, true, rtreeData, callback);

	if(callback.mStatus)
	{
		if(callback.mInitialOverlap)
		{
			if(!(PX_IS_SPU) && isMtd)
			{
				const bool hasContacts = computeConvex_TriangleMeshMTD(meshGeom,  meshPose, convexGeom, convexPose, inflation, distance, isDoubleSided, sweepHit);

				sweepHit.flags = PxHitFlag::eDISTANCE | PxHitFlag::eNORMAL;
				if(!hasContacts)
				{
					sweepHit.distance	= 0.0f;
					sweepHit.normal		= -unitDir;
				}
				else
				{
					sweepHit.flags |= PxHitFlag::ePOSITION;
				}
			}
			else
			{
				sweepHit.flags = PxHitFlag::eDISTANCE | PxHitFlag::eNORMAL;
				sweepHit.distance	= 0.0f;
				sweepHit.normal		= -unitDir;
			}
			sweepHit.faceIndex = callback.mHit.faceIndex;
		}
		else
		{
			sweepHit = callback.mHit;
			//sweepHit.position += unitDir * sweepHit.distance;
			sweepHit.normal = -sweepHit.normal;
			sweepHit.normal.normalize();
			if(isDoubleSided)
			{
				// PT: make sure the normal is properly oriented when we hit a back-triangle
				if(sweepHit.normal.dot(unitDir)>0.0f)
					sweepHit.normal = -sweepHit.normal;
			}
		}
		return true;
	}
	return false;
}
