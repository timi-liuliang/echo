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
#include "GuHeightFieldUtil.h"
#include "GuEntityReport.h"
#include "GuVecCapsule.h"
#include "GuSweepMTD.h"
#include "GuSweepSharedTests.h"
#include "GuVecBox.h"
#include "CmScaling.h"
#include "GuConvexUtilsInternal.h"
#include "GuSweepCapsuleTriangle.h"

using namespace physx;
using namespace Gu;
using namespace Cm;
using namespace physx::shdfnd::aos;

#include "GuSweepConvexTri.h"

struct CapsuleTraceSegmentReport : public EntityReport<PxU32>
{
	CapsuleTraceSegmentReport() : status(false), initialOverlap(false)
	{

	}

	bool underFaceHit(
		const Gu::HeightFieldUtil&, const PxVec3&,
		const PxVec3&, PxF32, PxF32, PxF32, PxU32)
	{
		return true;
	}

	bool faceHit(const Gu::HeightFieldUtil&, const PxVec3&, PxU32)
	{
		return true;
	}

	virtual PxAgain onEvent(PxU32 nb, PxU32* indices)
	{		
		PxU8 tribuf[HF_SWEEP_REPORT_BUFFER_SIZE*sizeof(PxTriangle)];
		PxTriangle* tmpT = (PxTriangle*)tribuf;
		for(PxU32 i=0; i<nb; i++)
		{
			PxU32 triangleIndex = indices[i];
			hfUtil->getTriangle(*pose, tmpT[i], NULL, NULL, triangleIndex, true);
		}

		PxU32 faceIndex = 0xFFFFFFFF;
		PxVec3 normal, impact, triNormal;
		PxF32 t = PX_MAX_F32;

		bool statusl = sweepCapsuleTriangles(nb, tmpT, *inflatedCapsule, *unitDir, distance, NULL, sweepHit->flags, t, normal, impact, faceIndex, triNormal, *hintFlags, isDoubleSided);
		//PX_PRINTF("face %d t=%f\n",indices[faceIndex],t);
		if(statusl && (t <= sweepHit->distance))
		{
			sweepHit->faceIndex = indices[faceIndex];
			sweepHit->normal = normal;
			sweepHit->position = impact;
			sweepHit->distance = t;
			this->status = statusl;
			if(t == 0.f)
			{
				initialOverlap = true;
				return false; // abort traversal, initial overlap handled separate
			}
			if (isAnyHit)
				return false; // abort traversal, any hit we dont need to parse additional triangles
		}
		return true;
	}

	const Capsule* inflatedCapsule;
	const PxVec3* unitDir;
	PxReal distance;
	PxSweepHit* sweepHit;
	PxHitFlags* hintFlags;
	bool status;
	bool initialOverlap;
	const PxTransform* pose;
	HeightFieldUtil* hfUtil;
	bool isDoubleSided;
	bool isAnyHit;
};

bool sweepCapsule_HeightFieldGeom(GU_CAPSULE_SWEEP_FUNC_PARAMS)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eHEIGHTFIELD);
	const PxHeightFieldGeometry& heightFieldGeom = static_cast<const PxHeightFieldGeometry&>(geom);

	const Capsule inflatedCapsule(lss.p0, lss.p1, lss.radius + inflation);

	// Compute swept box
	Box capsuleBox;
	computeBoxAroundCapsule(inflatedCapsule, capsuleBox);

	const PxVec3 capsuleAABBExtents = capsuleBox.computeAABBExtent();
	const PxU32 flags = PxHfQueryFlags::eWORLD_SPACE;

    CapsuleTraceSegmentReport myReport;

#ifdef __SPU__
	PX_ALIGN_PREFIX(16)  PxU8 heightFieldBuffer[sizeof(HeightField)+32] PX_ALIGN_SUFFIX(16);
	HeightField* heightField = memFetchAsync<HeightField>(heightFieldBuffer, (uintptr_t)(heightFieldGeom.heightField), sizeof(HeightField), 1);
	memFetchWait(1);

	g_sampleCache.init((uintptr_t)(heightField->getData().samples), heightField->getData().tilesU);

	const_cast<PxHeightFieldGeometry&>(heightFieldGeom).heightField = heightField;
#endif

	HeightFieldUtil hfUtil(heightFieldGeom);

	myReport.pose = &pose;
	myReport.hfUtil = &hfUtil;
	myReport.inflatedCapsule = &inflatedCapsule;
	myReport.unitDir = &unitDir;
	myReport.distance = distance;
	myReport.sweepHit = &sweepHit;
	myReport.hintFlags = &hintFlags;
	myReport.isAnyHit = hintFlags.isSet(PxHitFlag::eMESH_ANY);
	const PxU32 meshBothSides = hintFlags & PxHitFlag::eMESH_BOTH_SIDES;
	myReport.isDoubleSided = (heightFieldGeom.heightFieldFlags & PxMeshGeometryFlag::eDOUBLE_SIDED) || meshBothSides;
	sweepHit.distance = PX_MAX_F32;


#ifdef __SPU__
	Box sweptBox;
	computeSweptBox(sweptBox, capsuleBox.extents, capsuleBox.center, capsuleBox.rot, unitDir, distance);

	//### Temp hack until we can directly collide the OBB against the HF
	const PxTransform sweptBoxTR = sweptBox.getTransform();
	const PxBounds3 bounds = PxBounds3::poseExtent(sweptBoxTR, sweptBox.extents);

	hfUtil.overlapAABBTriangles(pose, bounds, flags, &myReport);
#else
	// need hf local space stuff	
	const PxTransform inversePose = pose.getInverse();
	const PxVec3 centerLocalSpace = inversePose.transform(capsuleBox.center);
	const PxVec3 sweepDirLocalSpace = inversePose.rotate(unitDir);
	const PxVec3 capsuleAABBBExtentHfLocalSpace = PxBounds3::basisExtent(centerLocalSpace, PxMat33(inversePose.q), capsuleAABBExtents).getExtents();	
	MPT_SET_CONTEXT("swch", pose, PxMeshScale());
	hfUtil.traceSegment<CapsuleTraceSegmentReport,false,false,true>(
		centerLocalSpace,centerLocalSpace + sweepDirLocalSpace*distance,&myReport,&capsuleAABBBExtentHfLocalSpace);
#endif

	if(myReport.status)
	{
		if(myReport.initialOverlap)
		{

			sweepHit.flags		= PxHitFlag::eDISTANCE | PxHitFlag::eNORMAL;

			if(!PX_IS_SPU && (hintFlags & PxHitFlag::eMTD))
			{

				const Vec3V p0 = V3LoadU(lss.p0);
				const Vec3V p1 = V3LoadU(lss.p1);
				const FloatV radius = FLoad(lss.radius);
				CapsuleV capsuleV;
				capsuleV.initialize(p0, p1, radius);

				//calculate MTD
				const bool hasContacts =  computeCapsule_HeightFieldMTD(heightFieldGeom, pose, capsuleV, inflatedCapsule.radius, distance, myReport.isDoubleSided, flags, sweepHit);

				//ML: the center of mass is below the surface, we won't have MTD contact generate
				if(!hasContacts)
				{
					sweepHit.distance	= 0.0f;
					sweepHit.normal		= -unitDir;
				}
				else
				{
					sweepHit.flags  |= PxHitFlag::ePOSITION;
				}
				return true;
			}
			else
			{
				sweepHit.distance = 0.f;
				sweepHit.normal = -unitDir;
			}
		}
		else
		{
			sweepHit.flags	= PxHitFlag::eDISTANCE | PxHitFlag::eNORMAL| PxHitFlag::ePOSITION;
		}
		return true;
	}
	return false;
}
class ConvexVsHeightfieldSweep : public EntityReport<PxU32>
{
public:
		ConvexVsHeightfieldSweep(
			HeightFieldUtil& hfUtil,
			const ConvexHullData& hull,
			const PxMeshScale& convexScale,
			const PxTransform& convexTrans,
			const PxTransform& hightFieldTrans,
			const PxVec3& unitDir, const PxReal distance, PxHitFlags hintFlags, const PxReal inflation,
			const bool aAnyHit, bool isDoubleSided) :
				mHfUtil			(hfUtil),
				mUnitDir		(unitDir),
				mInflation		(inflation),
				mHintFlags		(hintFlags),
				mAnyHit			(aAnyHit),
				mIsDoubleSided	(isDoubleSided),
				mInitialOverlap (false),
				mStatus			(false)

		{
			using namespace Ps::aos;
			mHit.faceIndex = 0xFFFFffff;
			mHit.distance = distance;
			const Vec3V worldDir = V3LoadU(unitDir);
			const FloatV dist = FLoad(distance);
			const QuatV q0 = QuatVLoadU(&hightFieldTrans.q.x);
			const Vec3V p0 = V3LoadU(&hightFieldTrans.p.x);

			const QuatV q1 = QuatVLoadU(&convexTrans.q.x);
			const Vec3V p1 = V3LoadU(&convexTrans.p.x);

			const PsTransformV meshTransf(p0, q0);
			const PsTransformV convexTransf(p1, q1);

			mMeshToConvex = convexTransf.transformInv(meshTransf);
			mConvexPoseV = convexTransf;
			mConvexSpaceDir = convexTransf.rotateInv(V3Neg(V3Scale(worldDir, dist)));
			mDistance = dist;

			const Vec3V vScale = V3LoadU(convexScale.scale);
			const QuatV vQuat = QuatVLoadU(&convexScale.rotation.x);

			mMeshSpaceUnitDir = hightFieldTrans.rotateInv(unitDir);
			mConvexHull.initialize(&hull, V3Zero(), vScale, vQuat);
		}

		virtual PxAgain onEvent(PxU32 nbEntities, PxU32* entities)
		{
			const PxTransform idt = PxTransform(PxIdentity);
			for(PxU32 i = 0; i < nbEntities; i++)
			{
				PxTriangle tri;
				mHfUtil.getTriangle(idt, tri, NULL, NULL, entities[i], false, false);  // First parameter not needed if local space triangle is enough

				// use mHit.distance as max sweep distance so far, mHit.distance will be clipped by this function
				if(sweepConvexVsTriangle(tri.verts[0], tri.verts[1], tri.verts[2], mConvexHull, mMeshToConvex, mConvexPoseV,
					mConvexSpaceDir, mUnitDir, mMeshSpaceUnitDir, mDistance, mHintFlags, mHit.distance, mHit, mIsDoubleSided, mInflation, mInitialOverlap))
				{
					mStatus = true;
					mHit.faceIndex = entities[i]; // update faceIndex
					if (mAnyHit || mHit.distance == 0.f)
						return false; // abort traversal
				}
			}
			return true; // continue traversal
		}

		bool underFaceHit(
			const Gu::HeightFieldUtil&, const PxVec3&,
			const PxVec3&, PxF32, PxF32, PxF32, PxU32)
		{
			return true;
		}

		bool faceHit(const Gu::HeightFieldUtil&, const PxVec3&, PxU32)
		{
			return true;
		}

		HeightFieldUtil&		mHfUtil;
		PsMatTransformV			mMeshToConvex;
		PsTransformV			mConvexPoseV;
		ConvexHullV				mConvexHull;
		PxSweepHit				mHit;
		Vec3V					mConvexSpaceDir;
		FloatV					mDistance;
		PxVec3					mUnitDir;
		PxVec3					mMeshSpaceUnitDir;
		PxReal					mInflation;
		PxU32					mHintFlags;
		const bool				mAnyHit;
		bool					mIsDoubleSided;
		bool					mInitialOverlap;
		bool					mStatus;

private:
	ConvexVsHeightfieldSweep& operator=(const ConvexVsHeightfieldSweep&);
};

bool sweepConvex_HeightFieldGeom(
	const PxGeometry& geom, const PxTransform& pose, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexPose,
	const PxVec3& unitDir, const PxReal distance, PxSweepHit& sweepHit, PxHitFlags hintFlags, const PxReal inflation)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eHEIGHTFIELD);
	const PxHeightFieldGeometry& hfGeom = static_cast<const PxHeightFieldGeometry&>(geom);

	const Matrix34 convexTM(convexPose);
	const Matrix34 meshTM(pose);

	FETCH_CONVEX_HULL_DATA(convexGeom)

//	const TriangleMesh* tm = static_cast<TriangleMesh*>(meshGeom.triangleMesh);

	const bool idtScaleConvex = convexGeom.scale.isIdentity();

	FastVertex2ShapeScaling convexScaling;
	if(!idtScaleConvex)
		convexScaling.init(convexGeom.scale);

	FastVertex2ShapeScaling meshScaling;
//	if(!idtScaleMesh)
//		meshScaling.init(meshGeom.scale);

	PX_ASSERT(!convexMesh->getLocalBoundsFast().isEmpty());
	const PxBounds3 hullAABBLocalSpace = PxBounds3::transformFast(convexScaling.getVertex2ShapeSkew(), convexMesh->getLocalBoundsFast());

#ifdef __SPU__
	PX_ALIGN_PREFIX(16)  PxU8 heightFieldBuffer[sizeof(HeightField)+32] PX_ALIGN_SUFFIX(16);
	HeightField* heightField = memFetchAsync<HeightField>(heightFieldBuffer, (uintptr_t)(hfGeom.heightField), sizeof(HeightField), 1);
	memFetchWait(1);
	g_sampleCache.init((uintptr_t)(heightField->getData().samples), heightField->getData().tilesU);

	const_cast<PxHeightFieldGeometry&>(hfGeom).heightField = heightField;
#endif

	const PxU32 flags = PxHfQueryFlags::eWORLD_SPACE;
	const bool isMtd = hintFlags & PxHitFlag::eMTD;

	const PxU32 meshBothSides = hintFlags & PxHitFlag::eMESH_BOTH_SIDES;
	const bool isDoubleSided = (hfGeom.heightFieldFlags & PxMeshGeometryFlag::eDOUBLE_SIDED) || meshBothSides;

	HeightFieldUtil hfUtil(hfGeom);
	ConvexVsHeightfieldSweep entityReport(
		hfUtil, convexMesh->getHull(), convexGeom.scale, convexPose, pose, -unitDir, distance, hintFlags, inflation,
		hintFlags.isSet(PxHitFlag::eMESH_ANY), isDoubleSided);

#ifdef __SPU__
	//	const bool idtScaleMesh = meshGeom.scale.isIdentity();
	const bool idtScaleMesh = true;

	Box hullOBB;
	computeHullOBB(hullOBB, hullAABBLocalSpace, 0.0f, convexPose, convexTM, meshTM, meshScaling, idtScaleMesh);

	hullOBB.extents.x += inflation;
	hullOBB.extents.y += inflation;
	hullOBB.extents.z += inflation;
	// Now create temporal bounds
	Box querySweptBox;
	computeSweptBox(querySweptBox, hullOBB.extents, hullOBB.center, hullOBB.rot, pose.rotateInv(unitDir), distance);

	// from MeshQuery::findOverlapHeightField
	const PxBounds3 bounds = PxBounds3::basisExtent(querySweptBox.center, querySweptBox.rot, querySweptBox.extents);
	hfUtil.overlapAABBTriangles(pose, bounds, 0, &entityReport);
#else
	// need hf local space stuff	
	const PxBounds3 hullAABB = PxBounds3::transformFast(convexPose, hullAABBLocalSpace);
	const PxVec3 aabbExtents = hullAABB.getExtents() + PxVec3(inflation, inflation, inflation);
	const PxTransform inversePose = pose.getInverse();
	const PxVec3 centerLocalSpace = inversePose.transform(hullAABB.getCenter());
	const PxVec3 sweepDirLocalSpace = inversePose.rotate(unitDir);
	const PxVec3 convexAABBExtentHfLocalSpace = PxBounds3::basisExtent(centerLocalSpace, PxMat33(inversePose.q), aabbExtents).getExtents();	
	MPT_SET_CONTEXT("swxh", pose, PxMeshScale());
	hfUtil.traceSegment<ConvexVsHeightfieldSweep,false,false,true>(
		centerLocalSpace,centerLocalSpace + sweepDirLocalSpace*distance,&entityReport,&convexAABBExtentHfLocalSpace);
#endif

	if(entityReport.mStatus)
	{
		if(entityReport.mInitialOverlap)
		{
			if(!(PX_IS_SPU) && isMtd)
			{
				const bool hasContacts = computeConvex_HeightFieldMTD(hfGeom,  pose, convexGeom, convexPose, inflation, distance, isDoubleSided, flags, sweepHit);

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
				sweepHit.faceIndex	= entityReport.mHit.faceIndex; // hit index must be set to closest for IO
			}
		}
		else
		{
			sweepHit = entityReport.mHit;
			sweepHit.normal = -sweepHit.normal;
			sweepHit.normal.normalize();
			sweepHit.faceIndex = entityReport.mHit.faceIndex;
		}
		return true;
	}
	return false;
}

#if defined(PX_VC) 
    #pragma warning(push)
	#pragma warning( disable : 4324 ) // Padding was added at the end of a structure because of a __declspec(align) value.
#endif

struct BoxTraceSegmentReport : public EntityReport<PxU32>
{
	virtual PxAgain onEvent(PxU32 nb, PxU32* indices)
	{
		const FloatV zero=FZero();
		const Vec3V zeroV = V3Zero();
		const Vec3V dir = V3LoadU(localMotion);
		//FloatV minToi = FMax();
		FloatV toi;
		Vec3V closestA, normal;//closestA and normal is in the local space of box

		for(PxU32 i=0; i<nb; i++)
		{
			const PxU32 triangleIndex = indices[i];

			PxTriangle currentTriangle;	// in world space
			hfUtil->getTriangle(*pose, currentTriangle, NULL, NULL, triangleIndex, true, true);

			const Vec3V localV0 = V3LoadU(currentTriangle.verts[0]);
			const Vec3V localV1 = V3LoadU(currentTriangle.verts[1]);
			const Vec3V localV2 = V3LoadU(currentTriangle.verts[2]);

			const Vec3V triV0 = WorldToBoxV->transform(localV0);
			const Vec3V triV1 = WorldToBoxV->transform(localV1);
			const Vec3V triV2 = WorldToBoxV->transform(localV2);

			if(!isDoubleSided)
			{
				const Vec3V triNormal = V3Cross(V3Sub(triV2, triV1),V3Sub(triV0, triV1)); 
				if(FAllGrtrOrEq(V3Dot(triNormal, dir), zero))
					continue;
			}

			TriangleV triangle(triV0, triV1, triV2);

			////move triangle to box space
			//const Vec3V localV0 = Vec3V_From_PxVec3(WorldToBox.transform(currentTriangle.verts[0]));
			//const Vec3V localV1 = Vec3V_From_PxVec3(WorldToBox.transform(currentTriangle.verts[1]));
			//const Vec3V localV2 = Vec3V_From_PxVec3(WorldToBox.transform(currentTriangle.verts[2]));

			//TriangleV triangle(localV0, localV1, localV2);

			bool ok  = GJKLocalRayCast(triangle, *box, zero, zeroV, dir, toi, normal, closestA, inflation, false);

			if(ok)
			{
				status	= true;
				if(FAllGrtr(toi, zero))
				{
					if(FAllGrtr(minToi, toi))
					{
						minToi = toi;
						FStore(toi, &hit->distance);
						V3StoreU(normal, hit->normal);
						V3StoreU(closestA, hit->position);
						hit->faceIndex		= triangleIndex;

						if(isAnyHit)
							break; // break we do compute the normal at the end, then early exit
					}
				}
				else
				{
					hit->distance = 0.f;
					hit->faceIndex	= triangleIndex;
					initialOverlap = true;
					return false;  // early exit, intialoverlap is handled separate
				}
			}
		}
		if(status)
		{
			hit->normal.normalize();
			if((hit->normal.dot(localMotion))>0.0f)
				hit->normal = -hit->normal;

			if(isAnyHit)
				return false; // abort traversal, any hit we dont need to parse additional triangles
		}
		return true;
	}

	bool underFaceHit(
		const Gu::HeightFieldUtil&, const PxVec3&,
		const PxVec3&, PxF32, PxF32, PxF32, PxU32)
	{
		return true;
	}

	bool faceHit(const Gu::HeightFieldUtil&, const PxVec3&, PxU32)
	{
		return true;
	}

	//PxTransform WorldToBox;
	const PsTransformV*	WorldToBoxV;
	const PxTransform* pose;
	HeightFieldUtil* hfUtil;
	BoxV* box;

	FloatV minToi;
	PxVec3 localMotion;
	PxSweepHit* hit;
	PxReal inflation;
	bool status;
	bool isDoubleSided;
	bool initialOverlap;
	bool isAnyHit;
};

#if defined(PX_VC) 
     #pragma warning(pop) 
#endif

bool sweepBox_HeightFieldGeom(GU_BOX_SWEEP_FUNC_PARAMS)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eHEIGHTFIELD);
	const PxHeightFieldGeometry& heightFieldGeom = static_cast<const PxHeightFieldGeometry&>(geom);

	PX_UNUSED(hintFlags);

	PxVec3 boxAABBExtent = box.computeAABBExtent();
	boxAABBExtent.x += inflation;	
	boxAABBExtent.y += inflation;
	boxAABBExtent.z += inflation;

	const PxU32 flags = PxHfQueryFlags::eWORLD_SPACE;
	const bool isDoubleSided = heightFieldGeom.heightFieldFlags & PxMeshGeometryFlag::eDOUBLE_SIDED;

	// Move to AABB space
	const PxTransform BoxToWorld = box.getTransform();
	PX_ALIGN_PREFIX(16) PxTransform WorldToBox PX_ALIGN_SUFFIX(16); WorldToBox = BoxToWorld.getInverse();

	const QuatV q1 = QuatVLoadA(&WorldToBox.q.x);
	const Vec3V p1 = V3LoadA(&WorldToBox.p.x);
	const PsTransformV WorldToBoxV(p1, q1);

	const PxVec3 motion = unitDir * distance;
	const PxVec3 localMotion = WorldToBox.rotate(motion);

	BoxV boxV(V3Zero(), V3LoadU(box.extents));

	sweepHit.distance = PX_MAX_F32;

    BoxTraceSegmentReport myReport;

#ifdef __SPU__
	PX_ALIGN_PREFIX(16)  PxU8 heightFieldBuffer[sizeof(HeightField)+32] PX_ALIGN_SUFFIX(16);
	HeightField* heightField = memFetchAsync<HeightField>(heightFieldBuffer, (uintptr_t)(heightFieldGeom.heightField), sizeof(HeightField), 1);
	memFetchWait(1);

	g_sampleCache.init((uintptr_t)(heightField->getData().samples), heightField->getData().tilesU);

	const_cast<PxHeightFieldGeometry&>(heightFieldGeom).heightField = heightField;
#endif

	HeightFieldUtil hfUtil(heightFieldGeom);

	myReport.WorldToBoxV = &WorldToBoxV;
	myReport.status = false;
	myReport.initialOverlap = false;
	myReport.pose = &pose;
	myReport.hfUtil = &hfUtil;
	myReport.box = &boxV;
	myReport.localMotion = localMotion;
	myReport.hit = &sweepHit;
	myReport.inflation = inflation;
	myReport.minToi = FMax();
	myReport.isAnyHit = hintFlags.isSet(PxHitFlag::eMESH_ANY);

	const PxU32 meshBothSides = hintFlags & PxHitFlag::eMESH_BOTH_SIDES;
	myReport.isDoubleSided = (heightFieldGeom.heightFieldFlags & PxMeshGeometryFlag::eDOUBLE_SIDED) || meshBothSides;

#ifdef __SPU__
	PxVec3 inflationBound(box.extents.x + inflation, box.extents.y + inflation, box.extents.z + inflation);
	// Compute swept box
	Box sweptBox;
	computeSweptBox(sweptBox, inflationBound, box.center, box.rot, unitDir, distance);

	//### Temp hack until we can directly collide the OBB against the HF
	const PxTransform sweptBoxTR = sweptBox.getTransform();
	const PxBounds3 bounds = PxBounds3::poseExtent(sweptBoxTR, sweptBox.extents);
	hfUtil.overlapAABBTriangles(pose, bounds, flags, &myReport);
#else
	// need hf local space stuff	
	const PxTransform inversePose = pose.getInverse();
	const PxVec3 centerLocalSpace = inversePose.transform(box.center);
	const PxVec3 sweepDirLocalSpace = inversePose.rotate(unitDir);
	const PxVec3 boxAABBExtentInHfLocalSpace = PxBounds3::basisExtent(centerLocalSpace, PxMat33(inversePose.q), boxAABBExtent).getExtents();
	MPT_SET_CONTEXT("swbh", pose, PxMeshScale());
	hfUtil.traceSegment<BoxTraceSegmentReport,false,false,true>(
		centerLocalSpace, centerLocalSpace + sweepDirLocalSpace*distance,&myReport,&boxAABBExtentInHfLocalSpace);
#endif

	if(myReport.status)
	{
		if(myReport.initialOverlap)
		{
			sweepHit.flags		= PxHitFlag::eDISTANCE | PxHitFlag::eNORMAL;

			if(!(PX_IS_SPU) && (hintFlags & PxHitFlag::eMTD))
			{
				bool hasContacts = computeBox_HeightFieldMTD(heightFieldGeom, pose, box, BoxToWorld, inflation, distance, isDoubleSided, flags, sweepHit);
				
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
			}
		}
		else
		{
			sweepHit.distance *= distance;  // stored as toi [0,1] during computation -> scale
			sweepHit.normal = BoxToWorld.rotate(sweepHit.normal);
			sweepHit.position = BoxToWorld.transform(sweepHit.position);
			sweepHit.flags = PxHitFlag::eDISTANCE | PxHitFlag::ePOSITION | PxHitFlag::eNORMAL;
		}
		return true;
	}
	return false;
}

