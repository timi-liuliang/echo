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

#include "PxsRigidBody.h"
#include "PxsContext.h"
#include "PxsAABBManager.h"
#include "PxsCCD.h"
#include "GuGJKWrapper.h"
#include "GuVecSphere.h"
#include "GuVecConvexHull.h"
#include "GuVecBox.h"
#include "GuVecCapsule.h"
#include "GuVecTriangle.h"
#include "GuTriangleVertexPointers.h"
#include "GuSweepSharedTests.h"
#include "GuConvexUtilsInternal.h"
#include "GuEntityReport.h"
#include "GuHeightFieldUtil.h"
#include "GuDistancePointTriangle.h"
#include "GuMidphase.h"
#include "GuGeomUtilsInternal.h"
#include "GuContainer.h"

#if DEBUG_RENDER_CCD
#include "PxPhysics.h"
#include "PxScene.h"
#include "GuTriangleVertexPointers.h"
#endif

#ifdef PX_WIIU  
#pragma ghs nowarning 1656 //within a function using alloca or VLAs, alignment of local variables
#endif

using namespace physx;
using namespace Gu;

PxF32 sweepAABBAABB(const PxVec3& centerA, const PxVec3& extentsA, const PxVec3& centerB, const PxVec3& extentsB, const PxVec3& trA, const PxVec3& trB);

namespace physx
{

//Utility class to help accessing triangle information. Intended to store a set of convex triangles but currently used only to reference a single triangle
class PxcConvexTriangles
{
public:
										PxcConvexTriangles(const PxTriangleMeshGeometryLL& shapeMesh, 
														   const Cm::FastVertex2ShapeScaling& skew, // object is not copied, beware!
														   const PxU32* trigsInGroup, 
														   PxU32 numTrigsInGroup, 
														   PxU32* trigIndexDestBuffer);//trigIndexDestBuffer should be at least  numTrigsInGroup long.

	/*virtual*/ void						getBounds(PxBounds3& bounds, const physx::PxTransform& transform)							const;
//	virtual bool						getInnerSphere(PxVec3& center, PxReal& radius) const;

	//non-virtuals:
	PX_FORCE_INLINE	const Gu::InternalTriangleMeshData*	getMeshData() const			{ return shapeMesh.meshData; }
	PX_FORCE_INLINE	PxU32								getNumTrigs() const			{ return numTrigsInGroup; }
	PX_FORCE_INLINE	const PxU32*						getTrigs()	const			{ return trigsInGroup; }

	PX_FORCE_INLINE	PxU32*								getTrigIndexDestBuffer() const { return trigIndexDestBuffer; }
//	PxU32												selectClosestPolygon(const PxVec3& localDirection)	const;
	PX_FORCE_INLINE	const Gu::HullPolygonData&			getPolygon()	const		{ return selectedPolygon; }

	PxVec3 getPolygonNormal(PxU32 index) const;

	const Cm::FastVertex2ShapeScaling&	getVertex2ShapeScaling() const { return mVertex2ShapeSkew; }
	

private:
	PxcConvexTriangles& operator=(const PxcConvexTriangles&);
	void calcCenterAndBounds(const physx::PxTransform& transform) const;

	const PxTriangleMeshGeometryLL&		shapeMesh;
	const Cm::FastVertex2ShapeScaling&	mVertex2ShapeSkew;
	const PxU32*						trigsInGroup;
	PxU32								numTrigsInGroup;
	PxU32*								trigIndexDestBuffer;
	mutable Gu::HullPolygonData			selectedPolygon;

	mutable PxBounds3					bounds;
	mutable PxVec3						mCenter;			//average of vertices rather than center of bounds!
	mutable bool						haveCenterAndBounds;
};

PxcConvexTriangles::PxcConvexTriangles(const PxTriangleMeshGeometryLL& md, 
									   const Cm::FastVertex2ShapeScaling& skew,
									   const PxU32* tg, PxU32 ntg, PxU32 * tb) 
: shapeMesh(md), mVertex2ShapeSkew(skew), trigsInGroup(tg), numTrigsInGroup(ntg), trigIndexDestBuffer(tb), bounds(PxBounds3::empty()), mCenter(0.0f), haveCenterAndBounds(false)
{
//	selectedPolygon.mFlags = 0xff;
}


void PxcConvexTriangles::getBounds(PxBounds3& b, const physx::PxTransform& transform) const
{
	calcCenterAndBounds(transform);
	b = bounds;
}

void PxcConvexTriangles::calcCenterAndBounds(const physx::PxTransform& transform) const	//computes bounds in shape space
{
	//NOTE: we have code that does this in a loop inside PxcContactHullMeshPenetrationFallback --  a relatively expensive weighted average of the faces.
	//see if we really need to be that expensive!

	//shound be done in ctor:
	PX_ASSERT(bounds.isEmpty());
	PX_ASSERT(mCenter.isZero());

	for (PxU32 i = 0; i < numTrigsInGroup; i++)
	{
		const PxU32 triangleIndex = trigsInGroup[i];
		Gu::TriangleVertexPointers T(*getMeshData(), triangleIndex);

		//TODO: this does a lot of redundant work because shared vertices get tested multiple times.
		//Still, its not a lot of work so any overhead of optimized data access may not be worth it.

		//gotta take bounds in shape space because building it in vertex space and transforming it out would skew it.
		
		//unrolled loop of 3
		const PxVec3 v0 = transform.transform(mVertex2ShapeSkew * T[0]);
		mCenter += v0;
		bounds.include(v0);

		const PxVec3 v1 = transform.transform(mVertex2ShapeSkew * T[1]);
		mCenter += v1;
		bounds.include(v1);

		const PxVec3 v2 = transform.transform(mVertex2ShapeSkew * T[2]);
		mCenter += v2;
		bounds.include(v2);
	}

	mCenter *= 1.0f / (numTrigsInGroup * 3);
	haveCenterAndBounds = true;
}

PxVec3 PxcConvexTriangles::getPolygonNormal(PxU32 index) const
{
	PX_ASSERT(index < numTrigsInGroup);
	const PxU32 triangleIndex = trigsInGroup[index];
	Gu::TriangleVertexPointers T(*getMeshData(), triangleIndex);
	const PxVec3 t0 = mVertex2ShapeSkew * T[0];
	const PxVec3 t1 = mVertex2ShapeSkew * T[1];
	const PxVec3 t2 = mVertex2ShapeSkew * T[2];
	const PxVec3 v0 = t0 - t1;
	const PxVec3 v1 = t0 - t2;
	const PxVec3 nor = v0.cross(v1);
	return nor.getNormalized();
}


typedef bool (*PxcTriangleSweepMethod) (TRIANGLE_SWEEP_METHOD_ARGS);

typedef PxReal (*PxcTriangleSweepMethod2) (TRIANGLE_SWEEP_METHOD_ARGS);

static PxReal PxcSweepSphereTriangles(TRIANGLE_SWEEP_METHOD_ARGS);
static PxReal PxcSweepCapsuleTriangles(TRIANGLE_SWEEP_METHOD_ARGS);
static PxReal PxcSweepBoxTriangles(TRIANGLE_SWEEP_METHOD_ARGS);
static PxReal PxcSweepConvexTriangles(TRIANGLE_SWEEP_METHOD_ARGS);
static PxReal PxcUnimplementedTriangleSweep(TRIANGLE_SWEEP_METHOD_ARGS)
{
	PX_UNUSED(shape0);
	PX_UNUSED(shape1);
	PX_UNUSED(transform0);
	PX_UNUSED(transform1);
	PX_UNUSED(lastTm0);
	PX_UNUSED(lastTm1);
	PX_UNUSED(restDistance);
	PX_UNUSED(worldNormal);
	PX_UNUSED(worldPoint);
	PX_UNUSED(cache);
	PX_UNUSED(context);
	PX_UNUSED(meshScaling);
	PX_UNUSED(triangle);
	PX_UNUSED(toiEstimate);

	return 1e10f;	//no impact
}

const PxcTriangleSweepMethod2 g_TriangleSweepMethodTable[7] = 
{
	PxcSweepSphereTriangles,		//PxGeometryType::eSPHERE
	PxcUnimplementedTriangleSweep,	//PxGeometryType::ePLANE
	PxcSweepCapsuleTriangles,		//PxGeometryType::eCAPSULE
	PxcSweepBoxTriangles,			//PxGeometryType::eBOX
	PxcSweepConvexTriangles,		//PxGeometryType::eCONVEXMESH
	PxcUnimplementedTriangleSweep,	//PxGeometryType::eTRIANGLEMESH
	PxcUnimplementedTriangleSweep,	//PxGeometryType::eHEIGHTFIELD
};



/**
\brief Templated utility function to sweep 2 convex shapes against each-other
This function sweeps 2 shapes against each-other.
*/
template<class ConvexA, class ConvexB>
bool CCDSweep(ConvexA& a, ConvexB& b,  const PsMatTransformV& aToB, const PsTransformV& transB, const Ps::aos::Vec3VArg translationA, const Ps::aos::FloatVArg initialLambda, const Ps::aos::Vec3VArg s, const Ps::aos::Vec3VArg r, Ps::aos::FloatV& lambda, Ps::aos::Vec3V& normal, Ps::aos::Vec3V& closestA,
			  PxReal inflation = 0.f)
{
	using namespace Ps::aos;

	FloatV _lambda;
	//Vec3V _closestA;
	Vec3V _closestA, _normal;
	if(Gu::GJKRelativeRayCast(a, b, aToB, initialLambda, s, r, _lambda, _normal, _closestA, inflation))
	{
		//Adjust closestA because it will be on the surface of convex a in its initial position (s). If the TOI > 0, we need to move 
		//the point along the sweep direction to get the world-space hit position.
		closestA = V3ScaleAdd(translationA, FMax(_lambda, FZero()), transB.transform(_closestA));
		normal = transB.rotate(_normal);

		lambda = _lambda;
		return true;
	}
	return false;
}

/**
\brief This code performs a conservative estimate of the TOI of a shape v mesh.
*/
PxReal PxcSweepEstimateAnyShapeMesh(const PxsCCDShape& shape0, const PxsCCDShape& shape1,
									const PxsRigidBody* atom0, const PxsRigidBody* atom1,
									const PxTransform& /*transform0*/, const PxTransform& transform1,
									PxReal restDistance)
{
	// this is the trimesh midphase for convex vs mesh sweep. shape0 is the convex shape.
	// Get actual shape data
	const Gu::GeometryUnion& shape1data = shape1.mShapeCore->geometry;
	const PxTriangleMeshGeometryLL& shapeMesh = (shape1data).get<const PxTriangleMeshGeometryLL>();

	const Cm::FastVertex2ShapeScaling meshScaling(shapeMesh.scale);


	/*---------------------------------------------------*\
	|
	| STEP1: OPCODE Geometry collection
	|
	\*---------------------------------------------------*/

	PxVec3 trA = shape0.mCurrentTransform.p - shape0.mPrevTransform.p;
	PxVec3 trB = shape1.mCurrentTransform.p - shape1.mPrevTransform.p;

	PxVec3 relTr = trA - trB;
	PxVec3 unitDir = relTr;
	PxReal length = unitDir.normalize();

	PxMat33 matRot(shape0.mPrevTransform.q);

	//1) Compute the swept bounds
	Gu::Box sweptBox;
	computeSweptBox(sweptBox, shape0.mExtents, shape0.mCenter, matRot, unitDir, length);

	Gu::Box vertexSpaceBox;
	computeVertexSpaceOBB(vertexSpaceBox, sweptBox, shape1.mCurrentTransform, shapeMesh.scale);

	vertexSpaceBox.extents += PxVec3(restDistance);

	// TODO: implement a cached mode that fetches the trigs from a cache rather than per opcode if there is little motion.

	struct CB : Gu::MeshHitCallback<PxRaycastHit>
	{
		PxReal minTOI;
		PxReal sumFastMovingThresh;
		const PxTriangleMeshGeometryLL& shapeMesh;
		const Cm::FastVertex2ShapeScaling& meshScaling;
		const PxVec3& relTr;
		const PxVec3& trA;
		const PxVec3& trB;
		const PxTransform& transform1;
		const PxVec3& origin;
		const PxVec3& extent;

		CB(PxReal aSumFast, const PxTriangleMeshGeometryLL& aShapeMesh, const Cm::FastVertex2ShapeScaling& aMeshScaling,
			const PxVec3& aRelTr, const PxVec3& atrA, const PxVec3& atrB, const PxTransform& aTransform1, const PxVec3& aOrigin, const PxVec3& aExtent)
			:	Gu::MeshHitCallback<PxRaycastHit>(CallbackMode::eMULTIPLE),
				sumFastMovingThresh(aSumFast), shapeMesh(aShapeMesh), meshScaling(aMeshScaling), relTr(aRelTr), trA(atrA), trB(atrB),
				transform1(aTransform1), origin(aOrigin), extent(aExtent)
		{
			minTOI = PX_MAX_REAL;
		}

		virtual PxAgain processHit( // all reported coords are in mesh local space including hit.position
			const PxRaycastHit& hit, const PxVec3&, const PxVec3&, const PxVec3&, PxReal& shrunkMaxT, const PxU32*)
		{
			PxU32 unused;
			PxcConvexTriangles convexPartOfMesh1(shapeMesh, meshScaling, &hit.faceIndex, 1, &unused);
			PxVec3 resultNormal = -transform1.rotate(convexPartOfMesh1.getPolygonNormal(0));
			if(relTr.dot(resultNormal) >= sumFastMovingThresh)
			{
				PxBounds3 bounds;
				convexPartOfMesh1.getBounds(bounds, transform1);
				//OK, we have all 3 vertices, now calculate bounds...

				PX_ASSERT(trB.x == 0.f);
				PX_ASSERT(trB.y == 0.f);
				PX_ASSERT(trB.z == 0.f);
				PxF32 toi = sweepAABBAABB(
					origin, extent * 1.1f, bounds.getCenter(), (bounds.getExtents() + PxVec3(0.01f, 0.01f, 0.01f)) * 1.1f, trA, trB);

				minTOI = PxMin(minTOI, toi);
				shrunkMaxT = minTOI;
			}

			return (minTOI > 0.0f); // stop traversal if minTOI == 0.0f
		}

		void operator=(const CB&) {}
	};

	PxVec3 origin = shape0.mCenter;
	PxVec3 extent = shape0.mExtents + PxVec3(restDistance);
	const PxReal fastMovingThresh0 = atom0 ? shape0.mFastMovingThreshold : 0.f;
	const PxReal fastMovingThresh1 = atom1 ? shape1.mFastMovingThreshold : 0.f;
	const PxReal sumFastMovingThresh =
		PxMax(fastMovingThresh0, fastMovingThresh1) * PxMin(atom0 ? atom0->mCCD->mTimeLeft : 1, atom1 ? atom1->mCCD->mTimeLeft : 1);

	CB callback(sumFastMovingThresh, shapeMesh, meshScaling, relTr, trA, trB, transform1, origin, extent);

	RTreeMidphaseData hmd;
	shapeMesh.meshData->mCollisionModel.getRTreeMidphaseData(hmd);
	MPT_SET_CONTEXT("ccdm", transform1, meshScaling);
	MeshRayCollider::collideOBB(vertexSpaceBox, true, hmd, callback);

	return callback.minTOI;
}


static PxReal PxcSweepAnyShapeMesh(	const PxsCCDShape& shape0, const PxsCCDShape& shape1, 
									const PxsRigidBody* atom0, const PxsRigidBody* atom1,
									const PxTransform& transform0, const PxTransform& transform1,
									PxReal restDistance, PxVec3& worldNormal, PxVec3& worldPoint,
									PxcNpCache& cache, PxcNpThreadContext& context)
{
	// this is the trimesh midphase for convex vs mesh sweep. shape0 is the convex shape.

	// Get actual shape data
	const Gu::GeometryUnion& shape1data = shape1.mShapeCore->geometry;
	const PxTriangleMeshGeometryLL& shapeMesh = (shape1data).get<const PxTriangleMeshGeometryLL>();

	const Cm::FastVertex2ShapeScaling meshScaling(shapeMesh.scale);

	/*---------------------------------------------------*\
	|
	| STEP1: OPCODE Geometry collection
	|
	\*---------------------------------------------------*/

	PxVec3 trA = shape0.mCurrentTransform.p - shape0.mPrevTransform.p;
	PxVec3 trB = shape1.mCurrentTransform.p - shape1.mPrevTransform.p;

	PxVec3 relTr = trA - trB;
	PxVec3 unitDir = relTr;
	PxReal length = unitDir.normalize();

	PxMat33 matRot(shape0.mPrevTransform.q);


	//1) Compute the swept bounds
	Gu::Box sweptBox;
	computeSweptBox(sweptBox, shape0.mExtents, shape0.mCenter, matRot, unitDir, length);

	Gu::Box vertexSpaceBox;
	if (shapeMesh.scale.isIdentity())
		vertexSpaceBox = transformBoxOrthonormal(sweptBox, shape1.mCurrentTransform.getInverse());
	else
		computeVertexSpaceOBB(vertexSpaceBox, sweptBox, shape1.mCurrentTransform, shapeMesh.scale);


	vertexSpaceBox.extents += PxVec3(restDistance);

	// TODO: implement a cached mode that fetches the trigs from a cache rather than per opcode if there is little motion.
	// Setup the collider
	RTreeMidphaseData hmd;
	shapeMesh.meshData->mCollisionModel.getRTreeMidphaseData(hmd);

	LocalContainer(tempContainer, 64);
	struct AccumCallback : MeshHitCallback<PxRaycastHit>
	{
		Gu::Container& mResult;
		AccumCallback(Gu::Container& result)
			:	MeshHitCallback<PxRaycastHit>(CallbackMode::eMULTIPLE),
				mResult(result)
		{
		}

		virtual PxAgain processHit( // all reported coords are in mesh local space including hit.position
			const PxRaycastHit& hit, const PxVec3&, const PxVec3&, const PxVec3&, PxReal&, const PxU32*)
		{
			mResult.Add(hit.faceIndex);
			return true;
		}

		void operator=(const AccumCallback&) {}
	} callback(tempContainer);

	// AP scaffold: early out opportunities, should probably use fat raycast
	MPT_SET_CONTEXT("ccdx", transform1, meshScaling);
	MeshRayCollider::collideOBB(vertexSpaceBox, true, hmd, callback);

	if (tempContainer.GetNbEntries() == 0)
		return PX_MAX_REAL;

	// Intersection found, fetch triangles
	PxU32 numTrigs = tempContainer.GetNbEntries();
	const PxU32* triangleIndices = tempContainer.GetEntries();

	// fetch some vars now that we will not early out:
	PxGeometryType::Enum type0 = shape0.mShapeCore->geometry.getType();
	PX_ALIGN(16, PxTransform) lastTransform0;
	PX_ALIGN(16, PxTransform) lastTransform1;

	lastTransform0 = shape0.mPrevTransform;	
	lastTransform1 = transform1;

	if (atom1 && !atom1->isKinematic())	//don't treat kinematics as moving because we can't block them so they need to be tested at their final pose.
		lastTransform1 = shape1.mPrevTransform;

	PxVec3 origin = shape0.mCenter;
	PxVec3 extent = shape0.mExtents + PxVec3(restDistance);

	LocalContainer(orderedContainer, 64);
	orderedContainer.Reserve(tempContainer.GetNbEntries());

	LocalContainer(distanceEntries, 64);
	distanceEntries.Reserve(tempContainer.GetNbEntries());

	PxU32* orderedList = orderedContainer.GetEntries();
	PxF32* distances = (PxF32*)distanceEntries.GetEntries();

	PxReal minTOI = PX_MAX_REAL;

	const PxReal fastMovingThresh0 = atom0 ? shape0.mFastMovingThreshold : 0.f;
	const PxReal fastMovingThresh1 = atom1 ? shape1.mFastMovingThreshold : 0.f;
	const PxReal ratio = PxMin(atom0 ? atom0->mCCD->mTimeLeft : 1.f, atom1 ? atom1->mCCD->mTimeLeft : 1.f);

	const PxReal sumFastMovingThresh = PxMax(fastMovingThresh0, fastMovingThresh1)*ratio;

	PxU32 count = 0;
	for(PxU32 a = 0; a < numTrigs; ++a)
	{
		PxU32 unused;
		PxcConvexTriangles convexPartOfMesh1(shapeMesh, meshScaling, &triangleIndices[a], 1, &unused);

		PxVec3 resultNormal = -transform1.rotate(convexPartOfMesh1.getPolygonNormal(0));

		if(relTr.dot(resultNormal) >= sumFastMovingThresh)
		{
			PxBounds3 bounds;
			convexPartOfMesh1.getBounds(bounds, lastTransform1);
			//OK, we have all 3 vertices, now calculate bounds...

			PxF32 toi = sweepAABBAABB(origin, extent, bounds.getCenter(), bounds.getExtents() + PxVec3(0.02f, 0.02f, 0.02f), trA, trB);

			PxU32 index = 0;
			if(toi <= 1.f)
			{
				for(PxU32 b = count; b > 0; --b)
				{
					if(distances[b-1] <= toi)
					{
						//shuffle down and swap
						index = b;
						break;
					}
					PX_ASSERT(b > 0);
					PX_ASSERT(b < numTrigs);
					distances[b] = distances[b-1];
					orderedList[b] = orderedList[b-1];
				}
				PX_ASSERT(index < numTrigs);
				orderedList[index] = triangleIndices[a];
				distances[index] = toi;
				count++;
			}
		}
	}



	PxVec3 tempWorldNormal(0.f), tempWorldPoint(0.f);

	Cm::FastVertex2ShapeScaling idScale;
	context.mCCDFaceIndex = PXC_CONTACT_NO_FACE_INDEX;

	PxVec3 sphereCenter(shape0.mPrevTransform.p);
	PxF32 inSphereRadius = shape0.mFastMovingThreshold;
	//PxF32 inRadSq = inSphereRadius * inSphereRadius;

	PxVec3 sphereCenterInTransform1 = transform1.transformInv(sphereCenter);

	PxVec3 sphereCenterInTransform0p = transform1.transformInv(lastTransform0.p);


	for (PxU32 ti = 0; ti < count /*&& PxMax(minTOI, 0.f) >= distances[ti]*/; ti++)
	{
		PxU32 unused;
		PxcConvexTriangles convexPartOfMesh1(shapeMesh, meshScaling, &orderedList[ti], 1, &unused);

		PxVec3 resultNormal, resultPoint;

		Gu::TriangleVertexPointers T(*shapeMesh.meshData, orderedList[ti]);

		PxVec3 v0 = meshScaling * T[0];
		PxVec3 v1 = meshScaling * T[1];
		PxVec3 v2 = meshScaling * T[2];

		Gu::TriangleV triangle(V3LoadU(v0), V3LoadU(v1), V3LoadU(v2));


		//do sweep
		const PxcTriangleSweepMethod2 sweepMethod = g_TriangleSweepMethodTable[type0];
		PxReal res = sweepMethod(
			shape0, shape1, transform0, transform1, lastTransform0, lastTransform1, restDistance,
			resultNormal, resultPoint, cache, context, Cm::FastVertex2ShapeScaling(), triangle,
			0.f);

		if(res <= 0.f)
		{
			//res = 0.f;

			const PxVec3 vv0 = v1 - v0 ;
			const PxVec3 vv1 = v2 - v0;
			const PxVec3 nor = vv0.cross(vv1);

			//Now we have a 0 TOI, lets see if the in-sphere hit it!

			PxF32 distanceSq = Gu::distancePointTriangleSquared( sphereCenterInTransform1, v0, vv0, vv1);

			PxReal inflatedRadius = restDistance + inSphereRadius;

			if(distanceSq < (inflatedRadius*inflatedRadius))
			{
				const PxF32 distance = PxSqrt(distanceSq);
				res = distance - inflatedRadius;
				const PxF32 d = nor.dot(v0);
				const PxF32 dd = nor.dot(sphereCenterInTransform0p);
				if((dd - d) > 0.f)
				{
					//back side, penetration 
					res = -(2.f * inflatedRadius - distance);
				}
			}	
			PX_ASSERT(PxIsFinite(res));
		}

		if (res < minTOI)
		{
			tempWorldNormal = convexPartOfMesh1.getPolygonNormal(0);//transform1.rotate(convexPartOfMesh1.getPolygonNormal(0));
			tempWorldPoint = resultPoint;
			minTOI = res;
			context.mCCDFaceIndex = orderedList[ti];
		}
		
	}

	worldNormal = transform1.rotate(tempWorldNormal);
	worldPoint = tempWorldPoint;
	return minTOI;
}

struct EntityReportContainerCallback : public Gu::EntityReport<PxU32>
{
	Gu::Container& container;
	EntityReportContainerCallback(Gu::Container& container_) : container(container_)
	{
		container.Reset();
	}
	virtual ~EntityReportContainerCallback() {}

	virtual bool onEvent(PxU32 nb, PxU32* indices)
	{
		container.Add(indices, nb);
		return true;
	}
private:
	EntityReportContainerCallback& operator=(const EntityReportContainerCallback&);
};



PxReal PxcSweepEstimateAnyShapeHeightfield(	const PxsCCDShape& shape0, const PxsCCDShape& shape1,
									const PxsRigidBody* atom0, const PxsRigidBody* atom1,
									const PxTransform& /*transform0*/, const PxTransform& transform1,
									PxReal restDistance)
{
	Gu::HeightFieldUtil hfUtil((physx::PxHeightFieldGeometry&)shape1.mShapeCore->geometry.get<const physx::PxHeightFieldGeometryLL>());

	LocalContainer(tempContainer, 64);

	EntityReportContainerCallback callback(tempContainer);

	PxVec3 trA = shape0.mCurrentTransform.p - shape0.mPrevTransform.p;
	PxVec3 trB = shape1.mCurrentTransform.p - shape1.mPrevTransform.p;

	PxVec3 relTr = trA - trB;
	PxVec3 halfRelTr = relTr * 0.5f;

	const PxVec3 extents = shape0.mExtents + halfRelTr.abs() + PxVec3(restDistance);
	const PxVec3 center = shape0.mCenter + halfRelTr;


	PxBounds3 bounds0(center - extents, center + extents);


	MPT_SET_CONTEXT("ccdh", transform1, PxMeshScale());
	hfUtil.overlapAABBTriangles(transform1, bounds0, PxHfQueryFlags::eWORLD_SPACE, &callback);
	
	PxVec3 origin = shape0.mCenter;
	PxVec3 extent = shape0.mExtents;

	PxReal minTOI = PX_MAX_REAL;

	const PxReal fastMovingThresh0 = atom0 ? shape0.mFastMovingThreshold : 0.f;
	const PxReal fastMovingThresh1 = atom1 ? shape1.mFastMovingThreshold : 0.f;

	const PxReal sumFastMovingThresh = PxMax(fastMovingThresh0, fastMovingThresh1)* PxMin(atom0 ? atom0->mCCD->mTimeLeft : 1, atom1 ? atom1->mCCD->mTimeLeft : 1);

	PxU32 numTrigs = tempContainer.GetNbEntries();
	PxU32* trianglesIndices = tempContainer.GetEntries();

	for(PxU32 a = 0; a < numTrigs; ++a)
	{

		PxTriangle tri;
		hfUtil.getTriangle(shape1.mPrevTransform, tri, 0, 0, trianglesIndices[a], true, true);



		PxVec3 resultNormal = -(tri.verts[1]-tri.verts[0]).cross(tri.verts[2]-tri.verts[0]);
		resultNormal.normalize();

		if(relTr.dot(resultNormal) >= sumFastMovingThresh)
		{

			PxBounds3 bounds;
			bounds.setEmpty();
			bounds.include(tri.verts[0]);
			bounds.include(tri.verts[1]);
			bounds.include(tri.verts[2]);

			PxF32 toi = sweepAABBAABB(origin, extent * 1.1f, bounds.getCenter(), (bounds.getExtents() + PxVec3(0.01f, 0.01f, 0.01f)) * 1.1f, trA, trB);

			minTOI = PxMin(minTOI, toi);
		}
	}



	//OK, we have a list of triangle indices
	return minTOI;

}


static PxReal PxcSweepAnyShapeHeightfield(	const PxsCCDShape& shape0, const PxsCCDShape& shape1, 
									const PxsRigidBody* atom0, const PxsRigidBody* atom1,
									const PxTransform& transform0, const PxTransform& transform1,
									PxReal restDistance, PxVec3& worldNormal, PxVec3& worldPoint,
									PxcNpCache& cache, PxcNpThreadContext& context)
{
	Gu::HeightFieldUtil hfUtil((PxHeightFieldGeometry&)shape1.mShapeCore->geometry.get<const physx::PxHeightFieldGeometryLL>());

	LocalContainer(tempContainer, 64);

	EntityReportContainerCallback callback(tempContainer);

	PxVec3 trA = shape0.mCurrentTransform.p - shape0.mPrevTransform.p;
	PxVec3 trB = shape1.mCurrentTransform.p - shape1.mPrevTransform.p;

	PxVec3 relTr = trA - trB;
	PxVec3 halfRelTr = relTr * 0.5f;

	const PxVec3 extents = shape0.mExtents + halfRelTr.abs() + PxVec3(restDistance);
	const PxVec3 center = shape0.mCenter + halfRelTr;


	PxBounds3 bounds0(center - extents, center + extents);

	MPT_SET_CONTEXT("ccdh", transform1, PxMeshScale());
	hfUtil.overlapAABBTriangles(transform1, bounds0, PxHfQueryFlags::eWORLD_SPACE, &callback);


	LocalContainer(orderedContainer, 64);
	orderedContainer.Reserve(tempContainer.GetNbEntries());

	LocalContainer(distanceEntries, 64);
	distanceEntries.Reserve(tempContainer.GetNbEntries());

	PxU32* orderedList = orderedContainer.GetEntries();
	PxF32* distances = (PxF32*)distanceEntries.GetEntries();
	
	PxVec3 origin = shape0.mCenter;
	PxVec3 extent = shape0.mExtents + PxVec3(restDistance);

	PxReal minTOI = PX_MAX_REAL;

	const PxReal fastMovingThresh0 = atom0 ? shape0.mFastMovingThreshold : 0.f;
	const PxReal fastMovingThresh1 = atom1 ? shape1.mFastMovingThreshold : 0.f;

	const PxReal sumFastMovingThresh = PxMax(fastMovingThresh0, fastMovingThresh1)* PxMin(atom0 ? atom0->mCCD->mTimeLeft : 1, atom1 ? atom1->mCCD->mTimeLeft : 1);

	PxU32 numTrigs = tempContainer.GetNbEntries();
	PxU32* trianglesIndices = tempContainer.GetEntries();

	PxU32 count = 0;
	for(PxU32 a = 0; a < numTrigs; ++a)
	{
		PxTriangle tri;
		hfUtil.getTriangle(shape1.mPrevTransform, tri, 0, 0, trianglesIndices[a], true, true);

		PxVec3 resultNormal = -(tri.verts[1]-tri.verts[0]).cross(tri.verts[2]-tri.verts[0]);
		resultNormal.normalize();

		if(relTr.dot(resultNormal) >= sumFastMovingThresh)
		{

			PxBounds3 bounds;
			bounds.setEmpty();
			bounds.include(tri.verts[0]);
			bounds.include(tri.verts[1]);
			bounds.include(tri.verts[2]);

			PxF32 toi = sweepAABBAABB(origin, extent * 1.1f, bounds.getCenter(), (bounds.getExtents() + PxVec3(0.01f, 0.01f, 0.01f)) * 1.1f, trA, trB);

			PxU32 index = 0;
			if(toi <= 1.f)
			{
				for(PxU32 b = count; b > 0; --b)
				{
					if(distances[b-1] <= toi)
					{
						//shuffle down and swap
						index = b;
						break;
					}
					PX_ASSERT(b > 0);
					PX_ASSERT(b < numTrigs);
					distances[b] = distances[b-1];
					orderedList[b] = orderedList[b-1];
				}
				PX_ASSERT(index < numTrigs);
				orderedList[index] = trianglesIndices[a];
				distances[index] = toi;
				count++;
			}
		}
	}



	worldNormal = PxVec3(PxReal(0));
	worldPoint = PxVec3(PxReal(0));
	Cm::FastVertex2ShapeScaling idScale;
	context.mCCDFaceIndex = PXC_CONTACT_NO_FACE_INDEX;

	PxVec3 sphereCenter(shape0.mPrevTransform.p);
	PxF32 inSphereRadius = shape0.mFastMovingThreshold;
	PxF32 inRadSq = inSphereRadius * inSphereRadius;

	PxGeometryType::Enum type0 = shape0.mShapeCore->geometry.getType();

	PX_ALIGN(16, PxTransform) lastTransform0;
	PX_ALIGN(16, PxTransform) lastTransform1;

	lastTransform0 = shape0.mPrevTransform;	
	lastTransform1 = transform1;

	if (atom1 && !atom1->isKinematic())	//don't treat kinematics as moving because we can't block them so they need to be tested at their final pose.
		lastTransform1 = shape1.mPrevTransform;

	PxVec3 sphereCenterInTr1 = transform1.transformInv(sphereCenter);
	PxVec3 sphereCenterInTr1T0 = transform1.transformInv(lastTransform0.p);


	PxVec3 tempWorldNormal(0.f), tempWorldPoint(0.f);

	for (PxU32 ti = 0; ti < count; ti++)
	{
		PxTriangle tri;
		hfUtil.getTriangle(shape1.mPrevTransform, tri, 0, 0, orderedList[ti], false, false);

		PxVec3 resultNormal, resultPoint;

		Gu::TriangleV triangle(V3LoadU(tri.verts[0]), V3LoadU(tri.verts[1]), V3LoadU(tri.verts[2]));

		//do sweep
		const PxcTriangleSweepMethod2 sweepMethod = g_TriangleSweepMethodTable[type0];
		PxReal res = sweepMethod(
			shape0, shape1, transform0, transform1, lastTransform0, lastTransform1, restDistance,
			resultNormal, resultPoint, cache, context, Cm::FastVertex2ShapeScaling(), triangle,
			0.f);

		if(res <= 0.f)
		{
			res = 0.f;

			const PxVec3 v0 = tri.verts[1] - tri.verts[0] ;
			const PxVec3 v1 = tri.verts[2] - tri.verts[0];

			//Now we have a 0 TOI, lets see if the in-sphere hit it!

			PxF32 distanceSq = Gu::distancePointTriangleSquared( sphereCenterInTr1, tri.verts[0], v0, v1);

			if(distanceSq < inRadSq)
			{
				const PxVec3 nor = v0.cross(v1);
				const PxF32 distance = PxSqrt(distanceSq);
				res = distance - inSphereRadius;
				const PxF32 d = nor.dot(tri.verts[0]);
				const PxF32 dd = nor.dot(sphereCenterInTr1T0);
				if((dd - d) > 0.f)
				{
					//back side, penetration 
					res = -(2.f * inSphereRadius - distance);
				}
			}			
		}

		if (res < minTOI)
		{
			const PxVec3 v0 = tri.verts[1] - tri.verts[0] ;
			const PxVec3 v1 = tri.verts[2] - tri.verts[0];

			PxVec3 resultNormal1 = v0.cross(v1);
			resultNormal1.normalize();
			//if(norDotRel > 1e-6f)
			{
				tempWorldNormal = resultNormal1;
				tempWorldPoint = resultPoint;
				minTOI = res;
				context.mCCDFaceIndex = orderedList[ti];
			}
		}
		
	}

	worldNormal = transform1.rotate(tempWorldNormal);
	worldPoint = tempWorldPoint;

	return minTOI;
}


PxReal PxcSweepSphereSphere(SWEEP_METHOD_ARGS)
{
	PX_UNUSED(context);
	PX_UNUSED(cache);
	PX_UNUSED(atom0);
	PX_UNUSED(atom1);

	const PxSphereGeometry& shapeSpecific0 = shape0.mShapeCore->geometry.get<const PxSphereGeometry>();		
	const PxSphereGeometry& shapeSpecific1 = shape1.mShapeCore->geometry.get<const PxSphereGeometry>();

	const Vec3V zeroV = V3Zero();
	Gu::CapsuleV capsule0(zeroV, FLoad(shapeSpecific0.radius));
	Gu::CapsuleV capsule1(zeroV, FLoad(shapeSpecific1.radius));

	const QuatV q0 = QuatVLoadA(&transform0.q.x);
	const Vec3V p0 = V3LoadA(&lastTm0.p.x);

	const QuatV q1 = QuatVLoadA(&transform1.q.x);
	const Vec3V p1 = V3LoadA(&lastTm1.p.x);

	const PsTransformV capTrans0(p0, q0);
	const PsTransformV capTrans1(p1, q1);

	const PsMatTransformV aToB(capTrans1.transformInv(capTrans0));

	
	const FloatV toi = FLoad(toiEstimate);

	const Vec3V trans0p = V3LoadU(transform0.p);
	const Vec3V trans1p = V3LoadU(transform1.p);
	const Vec3V trA = V3Sub(trans0p, p0);
	const Vec3V trB = V3Sub(trans1p, p1);
	const Vec3V relTr = capTrans1.rotateInv(V3Sub(trB, trA));

	FloatV lambda;
	Vec3V normal, closestA;

	if(CCDSweep(capsule0, capsule1, aToB, capTrans1, trA, toi, zeroV, relTr, 
		lambda, normal, closestA, shapeSpecific0.radius + shapeSpecific1.radius + restDistance))
	{
		//closestA = V3NegScaleSub(normal, capsule0.radius, closestA);
		PxF32 res;
		FStore(lambda, &res);
		V3StoreU(normal, worldNormal);
		V3StoreU(closestA, worldPoint);
		return res;
	}
	return PX_MAX_REAL;
}

PxReal PxcSweepSphereBox(SWEEP_METHOD_ARGS)
{
	PX_UNUSED(context);
	PX_UNUSED(cache);
	PX_UNUSED(atom0);
	PX_UNUSED(atom1);

	const PxSphereGeometry& shapeSpecific0 = shape0.mShapeCore->geometry.get<const PxSphereGeometry>();		
	const PxBoxGeometry& shapeSpecific1 = shape1.mShapeCore->geometry.get<const PxBoxGeometry>();

	const Vec3V zeroV = V3Zero();
	const Vec3V boxExtents = V3LoadU(shapeSpecific1.halfExtents);

	const QuatV q0 = QuatVLoadA(&transform0.q.x);
	const Vec3V p0 = V3LoadA(&lastTm0.p.x);

	const QuatV q1 = QuatVLoadA(&transform1.q.x);
	const Vec3V p1 = V3LoadA(&lastTm1.p.x);

	const PsTransformV capTrans(p0, q0);
	const PsTransformV boxTrans(p1, q1);

	const PsMatTransformV aToB(boxTrans.transformInv(capTrans));


	Gu::CapsuleV capsule(zeroV, FLoad(shapeSpecific0.radius));
	Gu::BoxV box(zeroV, boxExtents);

	const FloatV toi = FLoad(toiEstimate);

	const Vec3V trans0p = V3LoadU(transform0.p);
	const Vec3V trans1p = V3LoadU(transform1.p);
	const Vec3V trA = V3Sub(trans0p, p0);
	const Vec3V trB = V3Sub(trans1p, p1);
	const Vec3V relTr = boxTrans.rotateInv(V3Sub(trB, trA));


	FloatV lambda;
	Vec3V normal, closestA;

	if(CCDSweep(capsule, box, aToB, boxTrans, trA, toi, zeroV, relTr, lambda, normal, closestA, shapeSpecific0.radius+restDistance))
	{
		//closestA = V3NegScaleSub(normal, capsule.radius, closestA);
		PxF32 res;
		FStore(lambda, &res);
		V3StoreU(normal, worldNormal);
		V3StoreU(closestA, worldPoint);
		return res;
	}
	return PX_MAX_REAL;
}


PxReal PxcSweepCapsuleCapsule(SWEEP_METHOD_ARGS)
{
	PX_UNUSED(context);
	PX_UNUSED(cache);
	PX_UNUSED(restDistance);
	PX_UNUSED(atom0);
	PX_UNUSED(atom1);

	const PxCapsuleGeometry& shapeSpecific0 = shape0.mShapeCore->geometry.get<const PxCapsuleGeometry>();		
	const PxCapsuleGeometry& shapeSpecific1 = shape1.mShapeCore->geometry.get<const PxCapsuleGeometry>();
	
	const Vec3V zeroV = V3Zero();
	const FloatV capsuleRadius0 = FLoad(shapeSpecific0.radius);
	const FloatV halfHeight0 = FLoad(shapeSpecific0.halfHeight);

	const FloatV capsuleRadius1 = FLoad(shapeSpecific1.radius);
	const FloatV halfHeight1 = FLoad(shapeSpecific1.halfHeight);
	
	const QuatV q0 = QuatVLoadA(&transform0.q.x);
	const Vec3V p0 = V3LoadA(&lastTm0.p.x);

	const QuatV q1 = QuatVLoadA(&transform1.q.x);
	const Vec3V p1 = V3LoadA(&lastTm1.p.x);

	const PsTransformV capTrans0(p0, q0);
	const PsTransformV capTrans1(p1, q1);

	const PsMatTransformV aToB(capTrans1.transformInv(capTrans0));

	Gu::CapsuleV capsule0(zeroV, V3Scale(V3UnitX(), halfHeight0), capsuleRadius0);
	Gu::CapsuleV capsule1(zeroV, V3Scale(V3UnitX(), halfHeight1), capsuleRadius1);

	const FloatV toi = FLoad(toiEstimate);

	const Vec3V trans0p = V3LoadU(transform0.p);
	const Vec3V trans1p = V3LoadU(transform1.p);
	const Vec3V trA = V3Sub(trans0p, p0);
	const Vec3V trB = V3Sub(trans1p, p1);
	const Vec3V relTr = capTrans1.rotateInv(V3Sub(trB, trA));

	FloatV lambda;
	Vec3V normal, closestA;
	
	if(CCDSweep(capsule0, capsule1, aToB, capTrans1, trA, toi, zeroV, relTr, lambda, normal, closestA, shapeSpecific0.radius+ shapeSpecific1.radius + restDistance))
	{
		//closestA = V3NegScaleSub(normal, capsule0.radius, closestA);
		PxF32 res;  
		FStore(lambda, &res);
		V3StoreU(normal, worldNormal);
		V3StoreU(closestA, worldPoint);
		return res;
	}
	
	return PX_MAX_REAL;


}

PxReal PxcSweepCapsuleBox(SWEEP_METHOD_ARGS)
{
	PX_UNUSED(context);
	PX_UNUSED(cache);
	PX_UNUSED(restDistance);
	PX_UNUSED(atom0);
	PX_UNUSED(atom1);

	const PxCapsuleGeometry& shapeSpecific0 = shape0.mShapeCore->geometry.get<const PxCapsuleGeometry>();		
	const PxBoxGeometry& shapeSpecific1 = shape1.mShapeCore->geometry.get<const PxBoxGeometry>();
	
	const Vec3V zeroV = V3Zero();
	const FloatV capsuleRadius = FLoad(shapeSpecific0.radius);
	const FloatV halfHeight = FLoad(shapeSpecific0.halfHeight);
	const Vec3V boxExtents = V3LoadU(shapeSpecific1.halfExtents);

	Gu::CapsuleV capsule(zeroV, V3Scale(V3UnitX(), halfHeight), capsuleRadius);
	Gu::BoxV box(zeroV, boxExtents);

	const QuatV q0 = QuatVLoadA(&transform0.q.x);
	const Vec3V p0 = V3LoadA(&lastTm0.p.x);

	const QuatV q1 = QuatVLoadA(&transform1.q.x);
	const Vec3V p1 = V3LoadA(&lastTm1.p.x);

	const PsTransformV capTrans(p0, q0);
	const PsTransformV boxTrans(p1, q1);

	const PsMatTransformV aToB(boxTrans.transformInv(capTrans));

	const FloatV toi = FLoad(toiEstimate);

	const Vec3V trans0p = V3LoadU(transform0.p);
	const Vec3V trans1p = V3LoadU(transform1.p);
	const Vec3V trA = V3Sub(trans0p, p0);
	const Vec3V trB = V3Sub(trans1p, p1);
	const Vec3V relTr = boxTrans.rotateInv(V3Sub(trB, trA));
	FloatV lambda;
	Vec3V normal, closestA;
	
	if(CCDSweep(capsule, box, aToB, boxTrans, trA, toi, zeroV, relTr, lambda, normal, closestA, shapeSpecific0.radius+restDistance))
	{
		

		//closestA = V3NegScaleSub(normal, capsule.radius, closestA);
		PxF32 res;
		FStore(lambda, &res);
		V3StoreU(normal, worldNormal);
		V3StoreU(closestA, worldPoint);
		return res;
	}
	
	return PX_MAX_REAL;

}

PxReal PxcSweepCapsuleConvex(SWEEP_METHOD_ARGS)
{
	PX_UNUSED(context);
	PX_UNUSED(cache);
	PX_UNUSED(atom0);
	PX_UNUSED(atom1);

	const Gu::GeometryUnion& shapeData0 = shape0.mShapeCore->geometry;
	const Gu::GeometryUnion& shapeData1 = shape1.mShapeCore->geometry;

	const PxCapsuleGeometry& shapeSpecific0 = shapeData0.get<const PxCapsuleGeometry>();		
	const PxConvexMeshGeometryLL& shapeSpecific1 = shapeData1.get<const PxConvexMeshGeometryLL>();
	
	const Vec3V zeroV = V3Zero();
	const FloatV capsuleRadius = FLoad(shapeSpecific0.radius);
	const FloatV halfHeight = FLoad(shapeSpecific0.halfHeight);

	const Vec3V vScale = V3LoadU(shapeSpecific1.scale.scale);
	const QuatV vQuat = QuatVLoadU(&shapeSpecific1.scale.rotation.x);

	const Gu::ConvexHullData* hullData = shapeSpecific1.hullData;

	Gu::CapsuleV capsule(zeroV, V3Scale(V3UnitX(), halfHeight), capsuleRadius);
	Gu::ConvexHullV convexHull(hullData, zeroV, vScale, vQuat);

	const QuatV q0 = QuatVLoadA(&transform0.q.x);
	const Vec3V p0 = V3LoadA(&lastTm0.p.x);

	const QuatV q1 = QuatVLoadA(&transform1.q.x);
	const Vec3V p1 = V3LoadA(&lastTm1.p.x);

	const PsTransformV capTrans(p0, q0);
	const PsTransformV convexTrans(p1, q1);

	const PsMatTransformV aToB(convexTrans.transformInv(capTrans));

	const FloatV toi = FLoad(toiEstimate);

	const Vec3V trans0p = V3LoadU(transform0.p);
	const Vec3V trans1p = V3LoadU(transform1.p);
	const Vec3V trA = V3Sub(trans0p, p0);
	const Vec3V trB = V3Sub(trans1p, p1);
	const Vec3V relTr = convexTrans.rotateInv(V3Sub(trB, trA));

	FloatV lambda;
	Vec3V normal, closestA;

	if(CCDSweep(capsule, convexHull, aToB, convexTrans, trA, toi, zeroV, relTr, lambda, normal, closestA, shapeSpecific0.radius+restDistance))
	{
		//closestA = V3NegScaleSub(normal, capsule.radius, closestA);
		PxF32 res;
		FStore(lambda, &res);
		V3StoreU(normal, worldNormal);
		V3StoreU(closestA, worldPoint);
		return res;
	}

	return PX_MAX_REAL;


}

PxReal PxcSweepBoxBox(SWEEP_METHOD_ARGS)
{
	PX_UNUSED(context);
	PX_UNUSED(cache);
	PX_UNUSED(atom0);
	PX_UNUSED(atom1);

	const PxBoxGeometry& shapeSpecific0 = shape0.mShapeCore->geometry.get<const PxBoxGeometry>();		
	const PxBoxGeometry& shapeSpecific1 = shape1.mShapeCore->geometry.get<const PxBoxGeometry>();
	const Vec3V zeroV = V3Zero();
	const Vec3V boxExtents0 = V3LoadU(shapeSpecific0.halfExtents);
	const Vec3V boxExtents1 = V3LoadU(shapeSpecific1.halfExtents);

	Gu::BoxV box0(zeroV, boxExtents0);
	Gu::BoxV box1(zeroV, boxExtents1);

	const QuatV q0 = QuatVLoadA(&transform0.q.x);
	const Vec3V p0 = V3LoadA(&lastTm0.p.x);

	const QuatV q1 = QuatVLoadA(&transform1.q.x);
	const Vec3V p1 = V3LoadA(&lastTm1.p.x);

	const PsTransformV boxTrans0(p0, q0);
	const PsTransformV boxTrans1(p1, q1);

	const PsMatTransformV aToB(boxTrans1.transformInv(boxTrans0));

	const FloatV toi = FLoad(toiEstimate);

	const Vec3V trans0p = V3LoadU(transform0.p);
	const Vec3V trans1p = V3LoadU(transform1.p);
	const Vec3V trA = V3Sub(trans0p, p0);
	const Vec3V trB = V3Sub(trans1p, p1);
	const Vec3V relTr = boxTrans1.rotateInv(V3Sub(trB, trA));

	FloatV lambda;
	Vec3V normal, closestA;
	if(CCDSweep(box0, box1, aToB, boxTrans1, trA, toi, zeroV, relTr, lambda, normal, closestA, restDistance))
	{
		PxF32 res;
		FStore(lambda, &res);
		V3StoreU(normal, worldNormal);
		V3StoreU(closestA, worldPoint);
		return res;
	}

	return PX_MAX_REAL;
}

PxReal PxcSweepBoxConvex(SWEEP_METHOD_ARGS)
{
	PX_UNUSED(context);
	PX_UNUSED(cache);
	PX_UNUSED(atom0);
	PX_UNUSED(atom1);

	using namespace Ps::aos;
	const Gu::GeometryUnion& shapeData0 = shape0.mShapeCore->geometry;
	const Gu::GeometryUnion& shapeData1 = shape1.mShapeCore->geometry;

	const PxBoxGeometry& shapeSpecific0 = shapeData0.get<const PxBoxGeometry>();		
	const PxConvexMeshGeometryLL& shapeSpecific1 = shapeData1.get<const PxConvexMeshGeometryLL>();

	const Vec3V zeroV= V3Zero();
	const Vec3V vScale = V3LoadU(shapeSpecific1.scale.scale);
	const QuatV vQuat = QuatVLoadU(&shapeSpecific1.scale.rotation.x);

	const Vec3V boxExtents = V3LoadU(shapeSpecific0.halfExtents);
	const Gu::ConvexHullData* hullData = shapeSpecific1.hullData;

	Gu::BoxV box(zeroV, boxExtents);
	Gu::ConvexHullV convexHull(hullData, zeroV, vScale, vQuat);

	const QuatV q0 = QuatVLoadA(&transform0.q.x);
	const Vec3V p0 = V3LoadA(&lastTm0.p.x);

	const QuatV q1 = QuatVLoadA(&transform1.q.x);
	const Vec3V p1 = V3LoadA(&lastTm1.p.x);

	const PsTransformV boxTrans(p0, q0);
	const PsTransformV convexTrans(p1, q1);

	const PsMatTransformV aToB(convexTrans.transformInv(boxTrans));

	const FloatV toi = FLoad(toiEstimate);

	const Vec3V trans0p = V3LoadU(transform0.p);
	const Vec3V trans1p = V3LoadU(transform1.p);
	const Vec3V trA = V3Sub(trans0p, p0);
	const Vec3V trB = V3Sub(trans1p, p1);
	const Vec3V relTr = convexTrans.rotateInv(V3Sub(trB, trA));

	FloatV lambda;
	Vec3V normal, closestA;

	if(CCDSweep(box, convexHull, aToB, convexTrans, trA, toi, zeroV, relTr, lambda, normal, closestA, restDistance))
	{
		PxF32 res;
		FStore(lambda, &res);
		V3StoreU(normal, worldNormal);
		V3StoreU(closestA, worldPoint);
		return res;
	}

	return PX_MAX_REAL;
}

PxReal PxcSweepConvexConvex(SWEEP_METHOD_ARGS)
{
	PX_UNUSED(context);
	PX_UNUSED(cache);
	PX_UNUSED(atom0);
	PX_UNUSED(atom1);

	using namespace Ps::aos;
	const Gu::GeometryUnion& shapeData0 = shape0.mShapeCore->geometry;
	const Gu::GeometryUnion& shapeData1 = shape1.mShapeCore->geometry;

	const PxConvexMeshGeometryLL& shapeSpecific0 = shapeData0.get<const PxConvexMeshGeometryLL>();		
	const PxConvexMeshGeometryLL& shapeSpecific1 = shapeData1.get<const PxConvexMeshGeometryLL>();

	const Vec3V zeroV = V3Zero();
	const Vec3V vScale0 = V3LoadU(shapeSpecific0.scale.scale);
	const QuatV vQuat0 = QuatVLoadU(&shapeSpecific0.scale.rotation.x);
	const Vec3V vScale1 = V3LoadU(shapeSpecific1.scale.scale);
	const QuatV vQuat1 = QuatVLoadU(&shapeSpecific1.scale.rotation.x);

	const Gu::ConvexHullData* hullData0 = shapeSpecific0.hullData;
	const Gu::ConvexHullData* hullData1 = shapeSpecific1.hullData;

	Gu::ConvexHullV convexHull0(hullData0, zeroV, vScale0, vQuat0);
	Gu::ConvexHullV convexHull1(hullData1, zeroV, vScale1, vQuat1);

	const QuatV q0 = QuatVLoadA(&transform0.q.x);
	const Vec3V p0 = V3LoadA(&lastTm0.p.x);

	const QuatV q1 = QuatVLoadA(&transform1.q.x);
	const Vec3V p1 = V3LoadA(&lastTm1.p.x);

	const PsTransformV convexTrans0(p0, q0);
	const PsTransformV convexTrans1(p1, q1);
	const PsMatTransformV aToB(convexTrans1.transformInv(convexTrans0));

	const FloatV toi = FLoad(toiEstimate);

	const Vec3V trans0p = V3LoadU(transform0.p);
	const Vec3V trans1p = V3LoadU(transform1.p);
	const Vec3V trA = V3Sub(trans0p, p0);
	const Vec3V trB = V3Sub(trans1p, p1);
	const Vec3V relTr = convexTrans1.rotateInv(V3Sub(trB, trA));

	FloatV lambda;
	Vec3V normal, closestA;

	if(CCDSweep(convexHull0, convexHull1, aToB, convexTrans1, trA, toi, zeroV, relTr, lambda, normal, closestA, restDistance))
	{
		PxF32 res;
		FStore(lambda, &res);
		V3StoreU(normal, worldNormal);
		V3StoreU(closestA, worldPoint);
		return res;
	}

	return PX_MAX_REAL;
}

PxReal PxcSweepConvexMesh(SWEEP_METHOD_ARGS)
{
	PX_UNUSED(toiEstimate);
	PX_UNUSED(lastTm1);
	PX_UNUSED(lastTm0);

	return PxcSweepAnyShapeMesh(
		shape0, shape1, atom0, atom1, transform0, transform1, restDistance,
		worldNormal, worldPoint, cache, context);
}

PxReal PxcSweepConvexHeightfield(SWEEP_METHOD_ARGS)
{
	PX_UNUSED(toiEstimate);
	PX_UNUSED(lastTm1);
	PX_UNUSED(lastTm0);

	return PxcSweepAnyShapeHeightfield(
		shape0, shape1, atom0, atom1, transform0, transform1, restDistance,
		worldNormal, worldPoint, cache, context);
}

static PxReal PxcSweepCapsuleTriangles(TRIANGLE_SWEEP_METHOD_ARGS)
{
	PX_UNUSED(meshScaling);
	PX_UNUSED(context);
	PX_UNUSED(cache);
	PX_UNUSED(shape1);

	const PxCapsuleGeometry& shapeSpecific0 = shape0.mShapeCore->geometry.get<const PxCapsuleGeometry>();		

	const Vec3V zeroV = V3Zero();
	const FloatV capsuleRadius = FLoad(shapeSpecific0.radius);
	const FloatV halfHeight = FLoad(shapeSpecific0.halfHeight);

	Gu::CapsuleV capsule0(zeroV, V3Scale(V3UnitX(), halfHeight), capsuleRadius);

	const QuatV q0 = QuatVLoadA(&transform0.q.x);
	const Vec3V p0 = V3LoadA(&lastTm0.p.x);

	const QuatV q1 = QuatVLoadA(&transform1.q.x);
	const Vec3V p1 = V3LoadA(&lastTm1.p.x);

	const PsTransformV capsuleTrans(p0, q0);
	const PsTransformV triangleTrans(p1, q1);

	const PsMatTransformV aToB(capsuleTrans.transformInv(triangleTrans));

	const FloatV toi = FLoad(toiEstimate);

	const Vec3V trans0p = V3LoadU(transform0.p);
	const Vec3V trans1p = V3LoadU(transform1.p);
	const Vec3V trA = V3Sub(trans0p, p0);
	const Vec3V trB = V3Sub(trans1p, p1);
	const Vec3V relTr = capsuleTrans.rotateInv(V3Sub(trA, trB));

	FloatV lambda;
	Vec3V normal, closestA;

	if(CCDSweep(triangle, capsule0, aToB, capsuleTrans, trB, toi, zeroV, relTr, 
		lambda, normal, closestA, shapeSpecific0.radius+restDistance))
	{
		PxF32 res;
		FStore(lambda, &res);
		V3StoreU(normal, worldNormal);
		V3StoreU(closestA, worldPoint);
		return res;
	}
	return PX_MAX_REAL;
}


static PxReal PxcSweepSphereTriangles(TRIANGLE_SWEEP_METHOD_ARGS)
{
	PX_UNUSED(meshScaling);
	PX_UNUSED(context);
	PX_UNUSED(cache);
	PX_UNUSED(shape1);

	const PxSphereGeometry& shapeSpecific0 = shape0.mShapeCore->geometry.get<const PxSphereGeometry>();		

	const Vec3V zeroV = V3Zero();
	Gu::CapsuleV capsule(zeroV, FLoad(shapeSpecific0.radius));

	const QuatV q0 = QuatVLoadA(&transform0.q.x);
	const Vec3V p0 = V3LoadA(&lastTm0.p.x);

	const QuatV q1 = QuatVLoadA(&transform1.q.x);
	const Vec3V p1 = V3LoadA(&lastTm1.p.x);

	const PsTransformV capsuleTrans(p0, q0);
	const PsTransformV triangleTrans(p1, q1);

	const PsMatTransformV aToB(capsuleTrans.transformInv(triangleTrans));

	const FloatV toi = FLoad(toiEstimate);

	const Vec3V trans0p = V3LoadU(transform0.p);
	const Vec3V trans1p = V3LoadU(transform1.p);
	const Vec3V trA = V3Sub(trans0p, p0);
	const Vec3V trB = V3Sub(trans1p, p1);
	const Vec3V relTr = capsuleTrans.rotateInv(V3Sub(trA, trB));
	
	
	FloatV lambda;
	Vec3V normal, closestA;

	if(CCDSweep(triangle, capsule, aToB, capsuleTrans, trB, toi, zeroV, relTr, 
		lambda, normal, closestA, shapeSpecific0.radius+restDistance))
	{
		PxF32 res;
		FStore(lambda, &res);
		V3StoreU(normal, worldNormal);
		V3StoreU(closestA, worldPoint);
		return res;
	}
	return PX_MAX_REAL;

}


static PxReal PxcSweepBoxTriangles(TRIANGLE_SWEEP_METHOD_ARGS)
{
	PX_UNUSED(meshScaling);
	PX_UNUSED(context);
	PX_UNUSED(cache);
	PX_UNUSED(shape1);

	const PxBoxGeometry& shapeSpecific0 = shape0.mShapeCore->geometry.get<const PxBoxGeometry>();		

	const Vec3V zeroV = V3Zero();
	Gu::BoxV box0(zeroV, V3LoadU(shapeSpecific0.halfExtents));

	const QuatV q0 = QuatVLoadA(&transform0.q.x);
	const Vec3V p0 = V3LoadA(&lastTm0.p.x);

	const QuatV q1 = QuatVLoadA(&transform1.q.x);
	const Vec3V p1 = V3LoadA(&lastTm1.p.x);

	const PsTransformV boxTrans(p0, q0);
	const PsTransformV triangleTrans(p1, q1);
	const PsMatTransformV aToB(boxTrans.transformInv(triangleTrans));

	const FloatV toi = FLoad(toiEstimate);

	const Vec3V trans0p = V3LoadU(transform0.p);
	const Vec3V trans1p = V3LoadU(transform1.p);
	const Vec3V trA = V3Sub(trans0p, p0);
	const Vec3V trB = V3Sub(trans1p, p1);
	const Vec3V relTr = boxTrans.rotateInv(V3Sub(trA, trB));

	FloatV lambda;
	Vec3V normal, closestA;
	
	if(CCDSweep(triangle, box0, aToB, boxTrans, trB, toi, zeroV, relTr, 
		lambda, normal, closestA, restDistance))
	{
		PxF32 res;
		FStore(lambda, &res);
		V3StoreU(normal, worldNormal);
		V3StoreU(closestA, worldPoint);
		return res;
	}
	return PX_MAX_REAL;
}

static PxReal PxcSweepConvexTriangles(TRIANGLE_SWEEP_METHOD_ARGS)
{
	PX_UNUSED(meshScaling);
	PX_UNUSED(context);
	PX_UNUSED(cache);
	PX_UNUSED(shape1);

	const PxConvexMeshGeometryLL& shapeSpecific0 = shape0.mShapeCore->geometry.get<const PxConvexMeshGeometryLL>();		
	
	const Vec3V zeroV = V3Zero();
	const Vec3V vScale0 = V3LoadU(shapeSpecific0.scale.scale);
	const QuatV vQuat0 = QuatVLoadU(&shapeSpecific0.scale.rotation.x);

	const Gu::ConvexHullData* hullData0 = shapeSpecific0.hullData;
	Gu::ConvexHullV convexHull0(hullData0, zeroV, vScale0, vQuat0);

	const QuatV q0 = QuatVLoadA(&transform0.q.x);
	const Vec3V p0 = V3LoadA(&lastTm0.p.x);

	const QuatV q1 = QuatVLoadA(&transform1.q.x);
	const Vec3V p1 = V3LoadA(&lastTm1.p.x);

	const PsTransformV convexTrans(p0, q0);
	const PsTransformV triangleTrans(p1, q1);

	const PsMatTransformV aToB(convexTrans.transformInv(triangleTrans));

	const FloatV toi = FLoad(toiEstimate);

	const Vec3V trans0p = V3LoadU(transform0.p);
	const Vec3V trans1p = V3LoadU(transform1.p);
	const Vec3V trA = V3Sub(trans0p, p0);
	const Vec3V trB = V3Sub(trans1p, p1);
	const Vec3V relTr = convexTrans.rotateInv(V3Sub(trA, trB));

	FloatV lambda;
	Vec3V normal, closestA;

	
	if(CCDSweep(triangle, convexHull0, aToB, convexTrans, trB, toi, zeroV, relTr, lambda, normal, closestA, restDistance))
	{
	
		PxF32 res;
		FStore(lambda, &res);
		V3StoreU(normal, worldNormal);
		V3StoreU(closestA, worldPoint);
		return res;
	}
	return PX_MAX_REAL;
}

}
