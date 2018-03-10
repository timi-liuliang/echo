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

#include "PxQueryReport.h"

#include "GuHeightFieldUtil.h"
#include "GuEntityReport.h"
#include "CmScaling.h"
#include "PsUtilities.h"
#include "PxGeometryQuery.h"

#include "PxConvexMeshGeometry.h"
#include "PxTriangleMeshGeometry.h"
#include "GuConvexMesh.h"

#include "GuSweepSharedTests.h"  

#include "GuCapsule.h"
#include "PsAlloca.h"
#include "GuConvexUtilsInternal.h"
#include "GuTriangleMesh.h"
#include "GuSPUHelpers.h"
#include "GuVecSphere.h"
#include "GuVecCapsule.h"
#include "GuVecBox.h"
#include "GuVecTriangle.h"
#include "GuVecConvexHull.h"
#include "GuVecConvexHullNoScale.h"

#include "GuMidphase.h" // for inflated raycast
#include "GuPCMContactConvexCommon.h"
#include "GuSweepMTD.h"
#include "GuPersistentContactManifold.h"
#include "GuPCMShapeConvex.h"
#include "GuDistanceSegmentSegment.h"
#include "GuDistancePointSegment.h"

#ifdef PX_WIIU  
#pragma ghs nowarning 1656 //within a function using alloca or VLAs, alignment of local variables
#endif


using namespace physx;
using namespace Gu;


#define	BATCH_TRIANGLE_NUMBER	32u

struct MTDTriangle : public PxTriangle
{
public:
	PxU8 extraTriData;//active edge flag data
};

struct MeshMTDGenerationCallback : MeshHitCallback<PxRaycastHit>
{
public:
	
	Container&								container;

	MeshMTDGenerationCallback(Container& tempContainer)
	:	MeshHitCallback<PxRaycastHit>(CallbackMode::eMULTIPLE), container(tempContainer)
	{
	}

	virtual PxAgain processHit(
		const PxRaycastHit& hit, const PxVec3&, const PxVec3&, const PxVec3&, PxReal&, const PxU32*)
	{
		container.Add(hit.faceIndex);

		return true;
	}

	void operator=(const MeshMTDGenerationCallback&) {}
};

static bool getMTDPerTriangle(const Gu::MeshPersistentContact* manifoldContacts, const PxU32 numContacts, const PxU32 triangleIndex, Ps::aos::Vec3V& normal, Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, PxU32& faceIndex, Ps::aos::FloatV& deepestPen)
{
	using namespace Ps::aos;

	FloatV deepest =  V4GetW(manifoldContacts[0].mLocalNormalPen);
	PxU32 index = 0;
	for(PxU32 k=1; k<numContacts; ++k)
	{
		const FloatV pen = V4GetW(manifoldContacts[k].mLocalNormalPen);
		if(FAllGrtr(deepest, pen))
		{
			deepest = pen;
			index = k;
		}
	}

	if(FAllGrtr(deepestPen, deepest))
	{
		PX_ASSERT(triangleIndex == manifoldContacts[index].mFaceIndex);
		faceIndex = triangleIndex;
		deepestPen = deepest;
		normal = Vec3V_From_Vec4V(manifoldContacts[index].mLocalNormalPen);
		closestA = manifoldContacts[index].mLocalPointB;
		closestB = manifoldContacts[index].mLocalPointA;
		return true;
	}

	return false;
}
static void midPhaseQuery(const PxTriangleMeshGeometry& triMeshGeom, const PxTransform& pose, Gu::Box& bound, Container& tempContainer)
{
	GU_FETCH_MESH_DATA(triMeshGeom);

	const bool idtScaleMesh =  triMeshGeom.scale.isIdentity();

	Cm::FastVertex2ShapeScaling meshScaling;
	if(!idtScaleMesh)
		meshScaling.init( triMeshGeom.scale);

	Gu::Box vertexSpaceBox;
	computeVertexSpaceOBB(vertexSpaceBox, bound, pose, triMeshGeom.scale);
	

	RTreeMidphaseData hmd;
	const Gu::RTreeMidphase& collisionModel = meshData->mCollisionModel;
	collisionModel.getRTreeMidphaseData(hmd);

	MeshMTDGenerationCallback callback(tempContainer);

	MPT_SET_CONTEXT("mtdm", pose, triMeshGeom.scale);
	MeshRayCollider::collideOBB(vertexSpaceBox, true, hmd, callback);
}

struct MidPhaseQueryLocalReport : Gu::EntityReport<PxU32>
{
	MidPhaseQueryLocalReport(Container& _container) : container(_container)
	{

	}
	virtual bool onEvent(PxU32 nb, PxU32* indices)
	{

		for(PxU32 i=0; i<nb; i++)
		{
			PxU32 triangleIndex = indices[i];
			container.Add(triangleIndex);
		}
		
		return true;
	}

	Container& container;

private:
	MidPhaseQueryLocalReport operator=(MidPhaseQueryLocalReport& report);

} ;

static void midPhaseQuery(const Gu::HeightFieldUtil& hfUtil, const PxTransform& pose, PxBounds3& bounds,  Container& tempContainer, PxU32 flags)
{
	MidPhaseQueryLocalReport localReport(tempContainer);
	MPT_SET_CONTEXT("mtdh", pose, PxMeshScale());
	hfUtil.overlapAABBTriangles(pose, bounds, flags, &localReport);
}



bool calculateMTD(const Gu::CapsuleV& capsuleV, const Ps::aos::FloatVArg inflatedRadiusV, const bool isDoubleSide, const MTDTriangle* triangles, const PxU32 nbTriangles, const PxU32 startIndex, Gu::MeshPersistentContact* manifoldContacts, 
				  PxU32& numContacts, Ps::aos::Vec3V& normal, Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, PxU32& faceIndex, Ps::aos::FloatV& mtd)
{
	using namespace Ps::aos;
	const FloatV zero = FZero();
	bool hadContacts = false;
	FloatV deepestPen = mtd;

	for(PxU32 j=0; j<nbTriangles; ++j)
	{
		numContacts = 0;

		const MTDTriangle& curTri = triangles[j];
		TriangleV triangleV;
		triangleV.verts[0] = V3LoadU(curTri.verts[0]);
		triangleV.verts[1] = V3LoadU(curTri.verts[1]);
		triangleV.verts[2] = V3LoadU(curTri.verts[2]);
		const PxU8 triFlag = curTri.extraTriData;

		const Vec3V triangleNormal = triangleV.normal();
		const Vec3V v = V3Sub(capsuleV.getCenter(), triangleV.verts[0]);
		const FloatV dotV = V3Dot(triangleNormal, v);

		// Backface culling
		const bool culled = !isDoubleSide && (FAllGrtr(zero, dotV));
		if(culled)
			continue;
		
		Gu::PCMCapsuleVsMeshContactGeneration::processTriangle(triangleV, j+startIndex, capsuleV, inflatedRadiusV, triFlag, manifoldContacts, numContacts);

		if(numContacts ==0)
			continue;

		hadContacts = true;

		getMTDPerTriangle(manifoldContacts, numContacts, j + startIndex, normal, closestA, closestB, faceIndex, deepestPen);


	}

	mtd = deepestPen;
	return hadContacts;
}

bool physx::Gu::computeCapsule_TriangleMeshMTD(const PxTriangleMeshGeometry& triMeshGeom, const PxTransform& pose, Gu::CapsuleV& capsuleV, const PxReal inflatedRadius, 
						  const PxReal distance, const bool isDoubleSided,  PxSweepHit& hit)
{

	using namespace Ps::aos;


	const Vec3V zeroV = V3Zero();

	Gu::TriangleMesh* triMesh = static_cast<Gu::TriangleMesh*>(triMeshGeom.triangleMesh);
	const PxU8* extraTrigData = triMesh->mMesh.mData.mExtraTrigData;
	
	Gu::MeshPersistentContact manifoldContacts[64];   
	PxU32 numContacts = 0;
	FloatV distV = FLoad(distance);
	//inflated the capsule by 15% in case of some disgreement between sweep and mtd calculation.If sweep said initial overlap, but mtd has a positive seperation,
	//we are still be able to return a valid normal but we should zero the distance.
	const FloatV inflatedRadiusV = FLoad(inflatedRadius*1.15f);
	
	bool foundInitial = false;
	static PxU32 iterations = 4;

	const Cm::Matrix34 vertexToWorldSkew = pose * triMeshGeom.scale;

	LocalContainer(tempContainer, 128);


	Vec3V closestA=zeroV, closestB=zeroV, normal=zeroV;
	PxU32 triangleIndex = 0xfffffff;

	Vec3V translation = zeroV;
	FloatV mtd;
	for(PxU32 i=0; i<iterations; ++i)
	{
		tempContainer.Reset();
		Gu::Capsule inflatedCapsule;
		V3StoreU(capsuleV.p0, inflatedCapsule.p0);
		V3StoreU(capsuleV.p1, inflatedCapsule.p1);
		inflatedCapsule.radius = inflatedRadius;

		Box capsuleBox;
		computeBoxAroundCapsule(inflatedCapsule, capsuleBox);
		midPhaseQuery(triMeshGeom, pose, capsuleBox, tempContainer);

		// Get results
		PxU32 nbTriangles = tempContainer.GetNbEntries();

		if(!nbTriangles)
			break;


		PxU32* indices = tempContainer.GetEntries();

		bool hadContacts = false;


		PxU32 nbBatches = (nbTriangles + BATCH_TRIANGLE_NUMBER - 1)/BATCH_TRIANGLE_NUMBER;
		mtd = FMax();
		MTDTriangle triangles[BATCH_TRIANGLE_NUMBER];
		for(PxU32 a = 0; a < nbBatches; ++a)
		{
			PxU32 startIndex = a * BATCH_TRIANGLE_NUMBER;
			PxU32 nbTrigs = PxMin(nbTriangles - startIndex, BATCH_TRIANGLE_NUMBER);
			for(PxU32 k=0; k<nbTrigs; k++)
			{
				//triangle world space
				const PxU32 triangleIndex1 = indices[startIndex+k];
				::getScaledTriangle(triMeshGeom, vertexToWorldSkew, triangles[k], triangleIndex1);
				triangles[k].extraTriData = extraTrigData[triangleIndex1];
			}

			//ML: mtd has back face culling, so if the capsule's center is below the triangle, we won't generate any contacts
			hadContacts = calculateMTD(capsuleV, inflatedRadiusV, isDoubleSided, triangles, nbTrigs, startIndex, manifoldContacts, numContacts, normal, closestA, closestB, triangleIndex, mtd) || hadContacts;

		}
	
	
		if(!hadContacts)
			break;

		triangleIndex = indices[triangleIndex];

		foundInitial = true;

		//move the capsule to depenerate it
		
		distV = FSub(mtd, capsuleV.radius);
		if(FAllGrtrOrEq(FZero(), distV))
		{
			Vec3V center = capsuleV.getCenter();
			Vec3V t = V3Scale(normal, distV);
			translation = V3Sub(translation, t);
			center = V3Sub(center, t);
			capsuleV.setCenter(center);
		}
		else
		{
			if(i == 0)
			{
				//First iteration so keep this normal
				hit.distance = 0.f;
				V3StoreU(closestA, hit.position);
				V3StoreU(normal, hit.normal);
				hit.faceIndex = triangleIndex;
				return true;
			}
			break;
		}
	}

	normal = V3Normalize(translation);
	distV = FNeg(V3Length(translation));
	
	if(foundInitial)
	{
		FStore(distV, &hit.distance);
		V3StoreU(closestA, hit.position);
		V3StoreU(normal, hit.normal);
		hit.faceIndex = triangleIndex;
	}

	return foundInitial;

}


bool physx::Gu::computeCapsule_HeightFieldMTD(const PxHeightFieldGeometry& heightFieldGeom, const PxTransform& pose, Gu::CapsuleV& capsuleV, const PxReal inflatedRadius, const PxReal distance, const bool isDoubleSided, const PxU32 flags, PxSweepHit& hit)
{

	using namespace Ps::aos;

	const Vec3V zeroV = V3Zero();

	Gu::MeshPersistentContact manifoldContacts[64]; 
	PxU32 numContacts = 0;
	FloatV distV = FLoad(distance);
	//inflated the capsule by 1% in case of some disgreement between sweep and mtd calculation.If sweep said initial overlap, but mtd has a positive seperation,
	//we are still be able to return a valid normal but we should zero the distance.
	const FloatV inflatedRadiusV = FLoad(inflatedRadius*1.01f);  
	
	bool foundInitial = false;
	static PxU32 iterations = 4;

	LocalContainer(tempContainer, 128);

	Gu::HeightFieldUtil hfUtil(heightFieldGeom);


	Vec3V closestA=zeroV, closestB=zeroV, normal=zeroV;
	PxU32 triangleIndex = 0xfffffff;

	Vec3V translation = zeroV;
	FloatV mtd;
	for(PxU32 i=0; i<iterations; ++i)
	{
		tempContainer.Reset();
		Gu::Capsule inflatedCapsule;
		V3StoreU(capsuleV.p0, inflatedCapsule.p0);
		V3StoreU(capsuleV.p1, inflatedCapsule.p1);
		inflatedCapsule.radius = inflatedRadius;

		Box capsuleBox;
		computeBoxAroundCapsule(inflatedCapsule, capsuleBox);
		const PxTransform capsuleBoxTransform = capsuleBox.getTransform();
		PxBounds3 bounds = PxBounds3::poseExtent(capsuleBoxTransform, capsuleBox.extents);
		midPhaseQuery(hfUtil, pose, bounds, tempContainer, flags);

		// Get results
		PxU32 nbTriangles = tempContainer.GetNbEntries();

		if(!nbTriangles)
			break;


		PxU32* indices = tempContainer.GetEntries();

		bool hadContacts = false;

		PxU32 nbBatches = (nbTriangles + BATCH_TRIANGLE_NUMBER - 1)/BATCH_TRIANGLE_NUMBER;
		mtd = FMax();
		MTDTriangle triangles[BATCH_TRIANGLE_NUMBER];
		for(PxU32 a = 0; a < nbBatches; ++a)
		{
			PxU32 startIndex = a * BATCH_TRIANGLE_NUMBER;
			PxU32 nbTrigs = PxMin(nbTriangles - startIndex, BATCH_TRIANGLE_NUMBER);
			for(PxU32 k=0; k<nbTrigs; k++)
			{
				//triangle vertext space
				const PxU32 triangleIndex1 = indices[startIndex+k];
				hfUtil.getTriangle(pose, triangles[k], NULL, NULL, triangleIndex1, true);
				triangles[k].extraTriData = 0x38;
			}

			//ML: mtd has back face culling, so if the capsule's center is below the triangle, we won't generate any contacts
			hadContacts = calculateMTD(capsuleV, inflatedRadiusV, isDoubleSided, triangles, nbTrigs, startIndex, manifoldContacts, numContacts, normal, closestA, closestB, triangleIndex, mtd) || hadContacts;
		}


		if(!hadContacts)
			break;

		triangleIndex = indices[triangleIndex];

		foundInitial = true;

		distV = FSub(mtd, capsuleV.radius);
		if(FAllGrtrOrEq(FZero(), distV))
		{
			//move the capsule to depenerate it
			Vec3V center = capsuleV.getCenter();
			Vec3V t = V3Scale(normal, distV);
			translation = V3Sub(translation, t);
			center = V3Sub(center, t);
			capsuleV.setCenter(center);
		}
		else
		{
			if(i == 0)
			{
				//First iteration so keep this normal
				hit.distance = 0.f;
				V3StoreU(closestA, hit.position);
				V3StoreU(normal, hit.normal);
				hit.faceIndex = triangleIndex;
				return true;
			}
			break;
		}
	}

	normal = V3Normalize(translation);
	distV = FNeg(V3Length(translation));
	if(foundInitial)
	{
		FStore(distV, &hit.distance);
		V3StoreU(closestA, hit.position);
		V3StoreU(normal, hit.normal);
		hit.faceIndex = triangleIndex;
	}

	return foundInitial;


}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool calculateMTD(const Gu::PolygonalData& polyData, SupportLocal* polyMap, Ps::aos::PsTransformV& convexTransform, const Ps::aos::PsMatTransformV& meshToConvex, const bool isDoubleSided, const Ps::aos::FloatVArg inflation, const MTDTriangle* triangles, const PxU32 nbTriangles,  const PxU32 startIndex, 
				  Gu::MeshPersistentContact* manifoldContacts, PxU32& numContacts, Ps::aos::Vec3V& normal, Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, PxU32& faceIndex, Ps::aos::FloatV& mtd)
{
	using namespace Ps::aos;

	bool hadContacts = false;
	FloatV deepestPen = mtd;
	
	for(PxU32 j=0; j<nbTriangles; ++j)
	{
		numContacts = 0;
		const MTDTriangle& curTri = triangles[j];
		const PxU8 triFlag = curTri.extraTriData;
		
		Gu::PCMConvexVsMeshContactGeneration::processTriangle(polyData, polyMap, curTri.verts,  j+startIndex, triFlag, inflation, isDoubleSided, convexTransform, meshToConvex, manifoldContacts, numContacts);

		if(numContacts ==0)
			continue;

		hadContacts = true;
		getMTDPerTriangle(manifoldContacts, numContacts, j+startIndex, normal, closestA, closestB, faceIndex, deepestPen);
	}

	mtd = deepestPen;

	return hadContacts;
}



bool physx::Gu::computeBox_TriangleMeshMTD(const PxTriangleMeshGeometry& triMeshGeom, const PxTransform& pose, const Gu::Box& _box, const PxTransform& boxTransform, const PxReal inflation, const PxReal distance, const bool isDoubleSided,  PxSweepHit& hit)
{

	using namespace Ps::aos;

	Gu::TriangleMesh* triMesh = static_cast<Gu::TriangleMesh*>(triMeshGeom.triangleMesh);
	const PxU8* extraTrigData = triMesh->mMesh.mData.mExtraTrigData;

	const Vec3V zeroV = V3Zero();
	
	Gu::MeshPersistentContact manifoldContacts[64]; 
	PxU32 numContacts = 0;
	FloatV distV = FLoad(distance);
	
	
	bool foundInitial = false;
	static PxU32 iterations = 4;

	LocalContainer(tempContainer, 128);

	Vec3V closestA = zeroV, closestB = zeroV, normal = zeroV;
	Vec3V worldNormal=zeroV, worldContactA=zeroV;//, worldContactB=zeroV;
	PxU32 triangleIndex = 0xfffffff;

	Vec3V translation = zeroV;

	Gu::Box box = _box;
	
	const QuatV q0 = QuatVLoadU(&boxTransform.q.x);
	const Vec3V p0 = V3LoadU(&boxTransform.p.x);

	const Vec3V boxExtents = V3LoadU(box.extents);
	const FloatV minMargin = Gu::CalculatePCMBoxMargin(boxExtents);
	const FloatV inflationV = FAdd(FLoad(inflation), minMargin);
	PxReal boundInflation;
	FStore(inflationV, &boundInflation);

	box.extents.x += boundInflation;
	box.extents.y += boundInflation;
	box.extents.z += boundInflation;
	Gu::BoxV boxV(zeroV, boxExtents);

	Vec3V boxCenter = V3LoadU(box.center);

	//create the polyData based on the original data
	Gu::PolygonalData polyData;
	Gu::PCMPolygonalBox polyBox(_box.extents);
	polyBox.getPolygonalData(&polyData);

	Mat33V identity =  M33Identity();

	Cm::Matrix34 meshToWorldSkew = pose * triMeshGeom.scale;

	PsTransformV boxTransformV(p0, q0);//box
	
	FloatV mtd;
	
	
	for(PxU32 i=0; i<iterations; ++i)
	{
	
		
		tempContainer.Reset();

		midPhaseQuery(triMeshGeom, pose, box, tempContainer);

		// Get results
		PxU32 nbTriangles = tempContainer.GetNbEntries();

		if(!nbTriangles)
			break;

		boxTransformV.p = boxCenter;
		SupportLocalImpl<Gu::BoxV> boxMap(boxV, boxTransformV, identity, identity, true);

		boxMap.setShapeSpaceCenterofMass(zeroV);
		// Move to AABB space
		Cm::Matrix34 WorldToBox;
		computeWorldToBoxMatrix(WorldToBox, box);
		const Cm::Matrix34 meshToBox = WorldToBox*meshToWorldSkew;

		Ps::aos::Mat33V rot(V3LoadU(meshToBox.base0), V3LoadU(meshToBox.base1), V3LoadU(meshToBox.base2));
		Ps::aos::PsMatTransformV meshToConvex(V3LoadU(meshToBox.base3), rot);

		PxU32* indices = tempContainer.GetEntries();
		bool hadContacts = false;


		PxU32 nbBatches = (nbTriangles + BATCH_TRIANGLE_NUMBER - 1)/BATCH_TRIANGLE_NUMBER;
		mtd = FMax();
		MTDTriangle triangles[BATCH_TRIANGLE_NUMBER];
		for(PxU32 a = 0; a < nbBatches; ++a)
		{
			PxU32 startIndex = a * BATCH_TRIANGLE_NUMBER;
			PxU32 nbTrigs = PxMin(nbTriangles - startIndex, BATCH_TRIANGLE_NUMBER);
			for(PxU32 k=0; k<nbTrigs; k++)
			{
				//triangle vertext space
				const PxU32 triangleIndex1 = indices[startIndex+k];
				triMesh->getLocalTriangle(triangles[k], triangleIndex1);
				triangles[k].extraTriData = extraTrigData[triangleIndex1];
			}

			//ML: mtd has back face culling, so if the capsule's center is below the triangle, we won't generate any contacts
			hadContacts = calculateMTD(polyData, &boxMap, boxTransformV, meshToConvex,  isDoubleSided, inflationV,  triangles, nbTrigs, startIndex, manifoldContacts, numContacts, normal, closestA, closestB, triangleIndex, mtd) || hadContacts;
		}
	
		if(!hadContacts)
			break;

		triangleIndex = indices[triangleIndex];

		foundInitial = true;

		distV =mtd;
		worldNormal = boxTransformV.rotate(normal);
		worldContactA = boxTransformV.transform(closestA);
		if(FAllGrtrOrEq(FZero(), distV))
		{
			Vec3V t = V3Scale(worldNormal, mtd);
			translation = V3Sub(translation, t);
			boxCenter = V3Sub(boxCenter, t);
			V3StoreU(boxCenter, box.center);
		}
		else
		{
			if(i == 0)
			{
				//First iteration so keep this normal
				hit.distance = 0.f;
				V3StoreU(worldContactA, hit.position);
				V3StoreU(worldNormal, hit.normal);
				hit.faceIndex = triangleIndex;
				return true;
			}
			break;
		}
	}

	worldNormal = V3Normalize(translation);
	distV = FNeg(V3Length(translation));
	if(foundInitial)
	{
		//transform closestA to world space
		FStore(distV, &hit.distance);
		V3StoreU(worldContactA, hit.position);
		V3StoreU(worldNormal, hit.normal);
		hit.faceIndex = triangleIndex;
	}

	return foundInitial;

}


bool physx::Gu::computeBox_HeightFieldMTD(const PxHeightFieldGeometry& heightFieldGeom, const PxTransform& pose, const Gu::Box& _box, const PxTransform& boxTransform, const PxReal inflation,
						  const PxReal distance, const bool isDoubleSided, const PxU32 flags, PxSweepHit& hit)
{
	using namespace Ps::aos;

	const Vec3V zeroV = V3Zero();
	Gu::MeshPersistentContact manifoldContacts[64]; 
	PxU32 numContacts = 0;
	bool foundInitial = false;
	static PxU32 iterations = 4;

	LocalContainer(tempContainer, 128);
	const Gu::HeightFieldUtil hfUtil(heightFieldGeom);


	Vec3V closestA = zeroV, closestB = zeroV, normal = zeroV;
	Vec3V worldNormal=zeroV, worldContactA=zeroV;//, worldContactB=zeroV;
	PxU32 triangleIndex = 0xfffffff;

	Vec3V translation =zeroV;

	Gu::Box box = _box;
	
	FloatV distV = FLoad(distance);
	const QuatV q0 = QuatVLoadU(&boxTransform.q.x);
	const Vec3V p0 = V3LoadU(&boxTransform.p.x);

	const Vec3V boxExtents = V3LoadU(box.extents);
	const FloatV minMargin = Gu::CalculatePCMBoxMargin(boxExtents);
	const FloatV inflationV = FAdd(FLoad(inflation), minMargin);
	//const FloatV inflationV = FLoad(inflation);

	PxReal boundInflation;
	FStore(inflationV, &boundInflation);
	box.extents.x += boundInflation;
	box.extents.y += boundInflation;
	box.extents.z += boundInflation;
	
	Gu::BoxV boxV(zeroV, boxExtents);

	Vec3V boxCenter = V3LoadU(box.center);

	//create the polyData based on the original box
	Gu::PolygonalData polyData;
	Gu::PCMPolygonalBox polyBox(_box.extents);
	polyBox.getPolygonalData(&polyData);

	Mat33V identity =  M33Identity();

	Cm::Matrix34 meshToWorldSkew(pose);

	PsTransformV boxTransformV(p0, q0);//box
	
	FloatV mtd;
	
	for(PxU32 i=0; i<iterations; ++i)
	{


		tempContainer.Reset();

		PxBounds3 bounds = PxBounds3::poseExtent(box.getTransform(), box.extents);
		midPhaseQuery(hfUtil, pose, bounds, tempContainer, flags);

		// Get results
		PxU32 nbTriangles = tempContainer.GetNbEntries();

		if(!nbTriangles)
			break;

		boxTransformV.p = boxCenter;
		SupportLocalImpl<Gu::BoxV> boxMap(boxV, boxTransformV, identity, identity, true);
		boxMap.setShapeSpaceCenterofMass(zeroV);
		// Move to AABB space
		Cm::Matrix34 WorldToBox;
		computeWorldToBoxMatrix(WorldToBox, box);
		const Cm::Matrix34 meshToBox = WorldToBox*meshToWorldSkew;

		Ps::aos::Mat33V rot(V3LoadU(meshToBox.base0), V3LoadU(meshToBox.base1), V3LoadU(meshToBox.base2));
		Ps::aos::PsMatTransformV meshToConvex(V3LoadU(meshToBox.base3), rot);

		PxU32* indices = tempContainer.GetEntries();
		bool hadContacts = false;

		PxU32 nbBatches = (nbTriangles + BATCH_TRIANGLE_NUMBER-1)/BATCH_TRIANGLE_NUMBER;
		mtd = FMax();
		MTDTriangle triangles[BATCH_TRIANGLE_NUMBER];
		for(PxU32 a = 0; a < nbBatches; ++a)
		{
			PxU32 startIndex = a * BATCH_TRIANGLE_NUMBER;
			PxU32 nbTrigs = PxMin(nbTriangles - startIndex, BATCH_TRIANGLE_NUMBER);
			for(PxU32 k=0; k<nbTrigs; k++)
			{
				//triangle vertext space
				const PxU32 triangleIndex1 = indices[startIndex+k];
				hfUtil.getTriangle(pose, triangles[k], NULL, NULL, triangleIndex1, false, false);
				triangles[k].extraTriData = 0x38;
			}

			//ML: mtd has back face culling, so if the box's center is below the triangle, we won't generate any contacts
			hadContacts = calculateMTD(polyData, &boxMap, boxTransformV, meshToConvex,  isDoubleSided, inflationV,  triangles, nbTrigs, startIndex, manifoldContacts, numContacts, normal, closestA, closestB, triangleIndex, mtd) || hadContacts;
		}


		if(!hadContacts)
			break;

		triangleIndex = indices[triangleIndex];

		foundInitial = true;

		distV =mtd;
		worldNormal = boxTransformV.rotate(normal);
		worldContactA = boxTransformV.transform(closestA);
		if(FAllGrtrOrEq(FZero(), distV))
		{
			//worldContactB = boxTransformV.transform(closestB);
			Vec3V t = V3Scale(worldNormal, mtd);
			translation = V3Sub(translation, t);
			boxCenter = V3Sub(boxCenter, t);
			V3StoreU(boxCenter, box.center);
		}
		else
		{
			if(i == 0)
			{
				//First iteration so keep this normal
				hit.distance = 0.f;
				V3StoreU(worldContactA, hit.position);
				V3StoreU(worldNormal, hit.normal);
				hit.faceIndex = triangleIndex;
				return true;
			}
			break;
		}
	}

	worldNormal = V3Normalize(translation);
	distV = FNeg(V3Length(translation));
	
	if(foundInitial)
	{
		FStore(distV, &hit.distance);
		V3StoreU(worldContactA, hit.position);
		V3StoreU(worldNormal, hit.normal);
		hit.faceIndex = triangleIndex;
	}


	return foundInitial;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool physx::Gu::computeConvex_TriangleMeshMTD(const PxTriangleMeshGeometry& triMeshGeom, const PxTransform& pose, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexPose, const PxReal inflation, 
						  const PxReal distance, const bool isDoubleSided, PxSweepHit& hit)
{

	using namespace Ps::aos;

	const Vec3V zeroV = V3Zero();

	Gu::TriangleMesh* triMesh = static_cast<Gu::TriangleMesh*>(triMeshGeom.triangleMesh);
	Gu::ConvexMesh* cm = static_cast<Gu::ConvexMesh*>(convexGeom.convexMesh);
	const PxU8* extraTrigData = triMesh->mMesh.mData.mExtraTrigData;

	
	Gu::MeshPersistentContact manifoldContacts[64]; 
	PxU32 numContacts = 0;
	FloatV distV = FLoad(distance);
	
	bool foundInitial = false;
	static PxU32 iterations = 2;

	const bool idtScaleMesh = triMeshGeom.scale.isIdentity();
	Cm::FastVertex2ShapeScaling meshScaling;
	if(!idtScaleMesh)
		meshScaling.init(triMeshGeom.scale);

	Gu::ConvexHullData* hullData = &cm->getHull();


	const bool idtScaleConvex = convexGeom.scale.isIdentity();
	
	 Cm::FastVertex2ShapeScaling convexScaling;
	if(!idtScaleConvex)
			convexScaling.init(convexGeom.scale);

	const PxVec3 _shapeSpaceCenterOfMass = convexScaling * hullData->mCenterOfMass;
	const Vec3V shapeSpaceCenterOfMass = V3LoadU(_shapeSpaceCenterOfMass);

	const QuatV q0 = QuatVLoadU(&convexPose.q.x);
	const Vec3V p0 = V3LoadU(&convexPose.p.x);
	PsTransformV convexTransformV(p0, q0);

	const Vec3V vScale = V3LoadU(convexGeom.scale.scale);
	const QuatV vQuat = QuatVLoadU(&convexGeom.scale.rotation.x);
	Gu::ConvexHullV convexHull(hullData, V3Zero(), vScale, vQuat);
	PX_ALIGN(16, PxU8 convexBuff[sizeof(SupportLocalImpl<ConvexHullV>)]);
	
	const FloatV convexMargin = Gu::CalculatePCMConvexMargin(hullData, vScale);
	const FloatV inflationV = FAdd(FLoad(inflation), convexMargin);
	PxReal boundInflation;
	FStore(inflationV, &boundInflation);

	LocalContainer(tempContainer, 128);


	Vec3V closestA = zeroV, closestB = zeroV, normal = zeroV;
	PxU32 triangleIndex = 0xfffffff;

	Vec3V translation = zeroV;

	Cm::Matrix34 meshToWorldSkew = pose * triMeshGeom.scale;

	Gu::PolygonalData polyData;
	getPCMConvexData(convexHull, idtScaleConvex, polyData);

	
	FloatV mtd;
	Vec3V center = p0;
	PxTransform tempConvexPose = convexPose;
	Vec3V worldNormal= zeroV, worldContactA=zeroV;//, worldContactB=zeroV;
	
	for(PxU32 i=0; i<iterations; ++i)
	{

		tempContainer.Reset();

		//ML:: construct convex hull data
		V3StoreU(center, tempConvexPose.p);
		convexTransformV.p = center;
		SupportLocal* convexMap = (idtScaleConvex ? (SupportLocal*)PX_PLACEMENT_NEW(convexBuff, SupportLocalImpl<ConvexHullNoScaleV>)((ConvexHullNoScaleV&)convexHull, convexTransformV, convexHull.vertex2Shape, convexHull.shape2Vertex, idtScaleConvex) : 
		(SupportLocal*)PX_PLACEMENT_NEW(convexBuff, SupportLocalImpl<ConvexHullV>)(convexHull, convexTransformV, convexHull.vertex2Shape, convexHull.shape2Vertex, idtScaleConvex));

		convexMap->setShapeSpaceCenterofMass(shapeSpaceCenterOfMass);
		
		
		Gu::Box hullOBB;
		Gu::computeOBBAroundConvex(hullOBB, convexGeom, cm, tempConvexPose);

		hullOBB.extents.x += boundInflation;
		hullOBB.extents.y += boundInflation;
		hullOBB.extents.z += boundInflation;

		midPhaseQuery(triMeshGeom, pose, hullOBB, tempContainer);

		// Get results
		PxU32 nbTriangles = tempContainer.GetNbEntries();

		if(!nbTriangles)
			break;
	
		// Move to AABB space
		Cm::Matrix34 worldToConvex(tempConvexPose.getInverse());
		const Cm::Matrix34 meshToConvex = worldToConvex*meshToWorldSkew;

		Ps::aos::Mat33V rot(V3LoadU(meshToConvex.base0), V3LoadU(meshToConvex.base1), V3LoadU(meshToConvex.base2));
		Ps::aos::PsMatTransformV meshToConvexV(V3LoadU(meshToConvex.base3), rot);


		PxU32* indices = tempContainer.GetEntries();
		bool hadContacts = false;

		PxU32 nbBatches = (nbTriangles + BATCH_TRIANGLE_NUMBER-1)/BATCH_TRIANGLE_NUMBER;
		mtd = FMax();
		MTDTriangle triangles[BATCH_TRIANGLE_NUMBER];
		for(PxU32 a = 0; a < nbBatches; ++a)
		{
			PxU32 startIndex = a * BATCH_TRIANGLE_NUMBER;
			PxU32 nbTrigs = PxMin(nbTriangles - startIndex, BATCH_TRIANGLE_NUMBER);
			for(PxU32 k=0; k<nbTrigs; k++)
			{
				//triangle vertext space
				const PxU32 triangleIndex1 = indices[startIndex+k];
				triMesh->getLocalTriangle(triangles[k], triangleIndex1);
				triangles[k].extraTriData = extraTrigData[triangleIndex1];
			}

			//ML: mtd has back face culling, so if the capsule's center is below the triangle, we won't generate any contacts
			hadContacts = calculateMTD(polyData, convexMap, convexTransformV, meshToConvexV,  isDoubleSided, inflationV,  triangles, nbTrigs, startIndex, manifoldContacts, numContacts, normal, closestA, closestB, triangleIndex, mtd) || hadContacts;
		}

	
		if(!hadContacts)
			break;

		triangleIndex = indices[triangleIndex];

		foundInitial = true;

		distV =mtd;
		worldNormal = convexTransformV.rotate(normal);
		worldContactA = convexTransformV.transform(closestA);
		if(FAllGrtrOrEq(FZero(), distV))
		{
			Vec3V t = V3Scale(worldNormal, mtd);
			translation = V3Sub(translation, t);
			center = V3Sub(center, t);
		}
		else
		{
			if(i == 0)
			{
				//First iteration so keep this normal
				hit.distance = 0.f;
				V3StoreU(worldContactA, hit.position);
				V3StoreU(worldNormal, hit.normal);
				hit.faceIndex = triangleIndex;
				return true;
			}
			break;
		}
	}

	worldNormal = V3Normalize(translation);
	distV = FNeg(V3Length(translation));
	if(foundInitial)
	{
		//transform closestA to world space
		FStore(distV, &hit.distance);
		V3StoreU(worldContactA, hit.position);
		V3StoreU(worldNormal, hit.normal);
		hit.faceIndex = triangleIndex;
	}

	return foundInitial;
}


bool physx::Gu::computeConvex_HeightFieldMTD(const PxHeightFieldGeometry& heightFieldGeom, const PxTransform& pose, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexPose, const PxReal inflation,
						  const PxReal distance, const bool isDoubleSided, const PxU32 flags, PxSweepHit& hit)
{
	using namespace Ps::aos;

	const Gu::HeightFieldUtil hfUtil(heightFieldGeom);

	
	const Vec3V zeroV = V3Zero();
	Gu::MeshPersistentContact manifoldContacts[64]; 
	PxU32 numContacts = 0;
	FloatV distV = FLoad(distance);
	
	
	bool foundInitial = false;
	static PxU32 iterations = 2;

	Gu::ConvexMesh* cm = static_cast<Gu::ConvexMesh*>(convexGeom.convexMesh);

	Gu::ConvexHullData* hullData = &cm->getHull();

	const bool idtScaleConvex = convexGeom.scale.isIdentity();
	
	 Cm::FastVertex2ShapeScaling convexScaling;
	if(!idtScaleConvex)
			convexScaling.init(convexGeom.scale);

	const PxVec3 _shapeSpaceCenterOfMass = convexScaling * hullData->mCenterOfMass;
	const Vec3V shapeSpaceCenterOfMass = V3LoadU(_shapeSpaceCenterOfMass);

	const QuatV q0 = QuatVLoadU(&convexPose.q.x);
	const Vec3V p0 = V3LoadU(&convexPose.p.x);
	PsTransformV convexTransformV(p0, q0);

	const Vec3V vScale = V3LoadU(convexGeom.scale.scale);
	const QuatV vQuat = QuatVLoadU(&convexGeom.scale.rotation.x);
	Gu::ConvexHullV convexHull(hullData, zeroV, vScale, vQuat);
	PX_ALIGN(16, PxU8 convexBuff[sizeof(SupportLocalImpl<ConvexHullV>)]);

	const FloatV convexMargin = Gu::CalculatePCMConvexMargin(hullData, vScale);
	const FloatV inflationV = FAdd(FLoad(inflation), convexMargin);
	PxReal boundInflation;
	FStore(inflationV, &boundInflation);


	LocalContainer(tempContainer, 128);


	Vec3V closestA = zeroV, closestB = zeroV, normal = zeroV;
	Vec3V worldNormal=zeroV, worldContactA=zeroV;//, worldContactB=zeroV;
	PxU32 triangleIndex = 0xfffffff;

	Vec3V translation = zeroV;

	Gu::PolygonalData polyData;
	getPCMConvexData(convexHull, idtScaleConvex, polyData);

	FloatV mtd;
	Vec3V center = p0;
	PxTransform tempConvexPose = convexPose;
	Cm::Matrix34 meshToWorldSkew(pose);
	
	
	for(PxU32 i=0; i<iterations; ++i)
	{

		tempContainer.Reset();

		//ML:: construct convex hull data
	
		V3StoreU(center, tempConvexPose.p);
		convexTransformV.p = center;

		SupportLocal* convexMap = (idtScaleConvex ? (SupportLocal*)PX_PLACEMENT_NEW(convexBuff, SupportLocalImpl<ConvexHullNoScaleV>)((ConvexHullNoScaleV&)convexHull, convexTransformV, convexHull.vertex2Shape, convexHull.shape2Vertex, idtScaleConvex) : 
		(SupportLocal*)PX_PLACEMENT_NEW(convexBuff, SupportLocalImpl<ConvexHullV>)(convexHull, convexTransformV, convexHull.vertex2Shape, convexHull.shape2Vertex, idtScaleConvex));

		convexMap->setShapeSpaceCenterofMass(shapeSpaceCenterOfMass);
		
		Gu::Box hullOBB;
		Gu::computeOBBAroundConvex(hullOBB, convexGeom, cm, tempConvexPose);
		hullOBB.extents.x += boundInflation;
		hullOBB.extents.y += boundInflation;
		hullOBB.extents.z += boundInflation;

		PxBounds3 bounds = PxBounds3::basisExtent(hullOBB.center, hullOBB.rot, hullOBB.extents);

		midPhaseQuery(hfUtil, pose, bounds, tempContainer, flags);

		// Get results
		PxU32 nbTriangles = tempContainer.GetNbEntries();

		if(!nbTriangles)
			break;
	
		// Move to AABB space
		Cm::Matrix34 worldToConvex(tempConvexPose.getInverse());
		const Cm::Matrix34 meshToConvex = worldToConvex*meshToWorldSkew;

		Ps::aos::Mat33V rot(V3LoadU(meshToConvex.base0), V3LoadU(meshToConvex.base1), V3LoadU(meshToConvex.base2));
		Ps::aos::PsMatTransformV meshToConvexV(V3LoadU(meshToConvex.base3), rot);


		PxU32* indices = tempContainer.GetEntries();

		bool hadContacts = false;

		PxU32 nbBatches = (nbTriangles + BATCH_TRIANGLE_NUMBER-1)/BATCH_TRIANGLE_NUMBER;
		mtd = FMax();
		MTDTriangle triangles[BATCH_TRIANGLE_NUMBER];
		for(PxU32 a = 0; a < nbBatches; ++a)
		{
			PxU32 startIndex = a * BATCH_TRIANGLE_NUMBER;   
			PxU32 nbTrigs = PxMin(nbTriangles - startIndex, BATCH_TRIANGLE_NUMBER);
			for(PxU32 k=0; k<nbTrigs; k++)
			{
				//triangle vertext space
				const PxU32 triangleIndex1 = indices[startIndex+k];
				hfUtil.getTriangle(pose, triangles[k], NULL, NULL, triangleIndex1, false, false);
				triangles[k].extraTriData = 0x38;
			}

			//ML: mtd has back face culling, so if the capsule's center is below the triangle, we won't generate any contacts
			hadContacts = calculateMTD(polyData, convexMap, convexTransformV, meshToConvexV, isDoubleSided, inflationV,  triangles, nbTrigs, startIndex, manifoldContacts, numContacts, normal, closestA, closestB, triangleIndex, mtd) || hadContacts;
		}
	
		if(!hadContacts)
			break;

		triangleIndex = indices[triangleIndex];

		foundInitial = true;

		distV =mtd;
		worldNormal = convexTransformV.rotate(normal);
		worldContactA = convexTransformV.transform(closestA);

		if(FAllGrtrOrEq(FZero(), distV))
		{
			Vec3V t = V3Scale(worldNormal, mtd);
			translation = V3Sub(translation, t);
			center = V3Sub(center, t);
		}
		else
		{
			if(i == 0)
			{
				//First iteration so keep this normal
				hit.distance = 0.f;
				V3StoreU(worldContactA, hit.position);
				V3StoreU(worldNormal, hit.normal);
				hit.faceIndex = triangleIndex;
				return true;
			}
			break;
		}
	}

	worldNormal = V3Normalize(translation);
	distV = FNeg(V3Length(translation));
	if(foundInitial)
	{
		//transform closestA to world space
		FStore(distV, &hit.distance);
		V3StoreU(worldContactA, hit.position);
		V3StoreU(worldNormal, hit.normal);
		hit.faceIndex = triangleIndex;
	}

	return foundInitial;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool physx::Gu::computeSphere_SphereMTD(const Sphere& sphere0, const Sphere& sphere1, PxSweepHit& hit)
{
	const PxVec3 delta = sphere1.center - sphere0.center;
	const PxReal d2 = delta.magnitudeSquared();
	const PxReal radiusSum = sphere0.radius + sphere1.radius;

	const PxReal d = PxSqrt(d2);
	hit.normal = delta / d;
	hit.distance = d - radiusSum ;
	hit.position = sphere0.center + hit.normal * sphere0.radius;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool physx::Gu::computeSphere_CapsuleMTD( const Sphere& sphere, const Capsule& capsule, PxSweepHit& hit)
{
	const PxReal radiusSum = sphere.radius + capsule.radius;

	PxReal u;
	distancePointSegmentSquared(capsule, sphere.center, &u);

	const PxVec3 normal = capsule.getPointAt(u) -  sphere.center;
	
	const PxReal lenSq = normal.magnitudeSquared();
	const PxF32 d = PxSqrt(lenSq);
	hit.normal = normal / d;
	hit.distance = d - radiusSum;
	hit.position = sphere.center + hit.normal * sphere.radius;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool physx::Gu::computeCapsule_CapsuleMTD(const Capsule& capsule0, const Capsule& capsule1, PxSweepHit& hit)
{
	using namespace Ps::aos;

	PxReal s,t;
	distanceSegmentSegmentSquared2(capsule0, capsule1, &s, &t);


	const PxReal radiusSum = capsule0.radius + capsule1.radius;

	const PxVec3 pointAtCapsule0 = capsule0.getPointAt(s);
	const PxVec3 pointAtCapsule1 = capsule1.getPointAt(t);

	const PxVec3 normal = pointAtCapsule0 - pointAtCapsule1;
	const PxReal lenSq = normal.magnitudeSquared();
	const PxF32 len = PxSqrt(lenSq);
	hit.normal = normal / len;
	hit.distance = len - radiusSum;
	hit.position = pointAtCapsule1 + hit.normal * capsule1.radius;
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool physx::Gu::computePlane_CapsuleMTD(const PxPlane& plane, const Capsule& capsule, PxSweepHit& hit)
{
	const PxReal d0 = plane.distance(capsule.p0);
	const PxReal d1 = plane.distance(capsule.p1);
	PxReal dmin;
	PxVec3 point;
	if(d0 < d1)
	{
		dmin = d0;
		point = capsule.p0;
	}
	else
	{
		dmin = d1;
		point = capsule.p1;
	}

	hit.normal		= plane.n;
	hit.distance	= dmin - capsule.radius;
	hit.position	= point - hit.normal * dmin;
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool physx::Gu::computePlane_BoxMTD(const PxPlane& plane, const Box& box, PxSweepHit& hit)
{
	PxVec3 pts[8];
	box.computeBoxPoints(pts);

	PxReal dmin = plane.distance(pts[0]);
	PxU32 index = 0;
	for(PxU32 i=1;i<8;i++)
	{
		const PxReal d = plane.distance(pts[i]);
		if(dmin > d)
		{
			index = i;
			dmin = d;
		}
	}
	hit.normal		= plane.n;
	hit.distance	= dmin;
	hit.position	= pts[index] - plane.n*dmin;
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool physx::Gu::computePlane_ConvexMTD(const PxPlane& plane, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexPose, PxSweepHit& hit)
{
	const ConvexMesh* convexMesh = static_cast<const ConvexMesh*>(convexGeom.convexMesh);
	const Cm::FastVertex2ShapeScaling convexScaling(convexGeom.scale);
	PxU32 nbVerts = convexMesh->getNbVerts();
	const PxVec3* PX_RESTRICT verts = convexMesh->getVerts();

	PxVec3 worldPointMin = convexPose.transform(convexScaling * verts[0]);
	PxReal dmin = plane.distance(worldPointMin);
	for(PxU32 i=1;i<nbVerts;i++)
	{
		const PxVec3 worldPoint = convexPose.transform(convexScaling * verts[i]);
		const PxReal d = plane.distance(worldPoint);
		if(dmin > d)
		{
			dmin = d;
			worldPointMin = worldPoint;
		}
	}

	hit.normal		= plane.n;
	hit.distance	= dmin;
	hit.position	= worldPointMin - plane.n * dmin;
	return true;
}


