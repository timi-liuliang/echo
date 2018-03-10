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

#include "GuIntersectionEdgeEdge.h"
#include "GuDistanceSegmentTriangle.h"
#include "GuIntersectionRayTriangle.h"
#include "GuTriangleVertexPointers.h"
#include "GuIntersectionTriangleBox.h"
#include "GuGeomUtilsInternal.h"
#include "GuContactMethodImpl.h"
#include "GuFeatureCode.h"
#include "GuContactBuffer.h"
#include "GuMidphase.h"
#include "CmScaling.h"
#include "GuEntityReport.h"
#include "GuHeightFieldUtil.h"
#include "GuConvexEdgeFlags.h"
#include "GuGeometryUnion.h"

using namespace physx;
using namespace Gu;

#define DEBUG_RENDER_MESHCONTACTS 0

#if DEBUG_RENDER_MESHCONTACTS
#include "PxPhysics.h"
#include "PxScene.h"
#endif

#ifdef __SPU__
extern unsigned char HeightFieldBuffer[sizeof(HeightField)+16];
#include "CmMemFetch.h"
#endif

#define USE_AABB_TRI_CULLING

//#define USE_CAPSULE_TRI_PROJ_CULLING
//#define USE_CAPSULE_TRI_SAT_CULLING
#define VISUALIZE_TOUCHED_TRIS	0
#define VISUALIZE_CULLING_BOX	0

#if VISUALIZE_TOUCHED_TRIS
#include "CmRenderOutput.h"
#include "PxsContactManager.h"
#include "PxsContext.h"
static void gVisualizeLine(const PxVec3& a, const PxVec3& b, PxcNpThreadContext& context, PxU32 color=0xffffff)
{
	PxMat44 m = PxMat44::identity();

	Cm::RenderOutput& out = context.mRenderOutput;
	out << color << m << Cm::RenderOutput::LINES << a << b;
}
static void gVisualizeTri(const PxVec3& a, const PxVec3& b, const PxVec3& c, PxcNpThreadContext& context, PxU32 color=0xffffff)
{
	PxMat44 m = PxMat44::identity();

	Cm::RenderOutput& out = context.mRenderOutput;
	out << color << m << Cm::RenderOutput::TRIANGLES << a << b << c;
}

static PxU32 gColors[8] = { 0xff0000ff, 0xff00ff00, 0xffff0000,
							0xff00ffff, 0xffff00ff, 0xffffff00,
							0xff000080, 0xff008000};
#endif

static const float fatBoxEdgeCoeff = 0.01f;

static bool PxcTestAxis(const PxVec3& axis, const Segment& segment, PxReal radius, 
						const PxVec3* PX_RESTRICT triVerts, PxReal& depth)
{
	// Project capsule
	PxReal min0 = segment.p0.dot(axis);
	PxReal max0 = segment.p1.dot(axis);
	if(min0>max0)	Ps::swap(min0, max0);
	min0 -= radius;
	max0 += radius;

	// Project triangle
	float Min1, Max1;
	{
		Min1 = Max1 = triVerts[0].dot(axis);
		const PxReal dp1 = triVerts[1].dot(axis);
		Min1 = physx::intrinsics::selectMin(Min1, dp1);
		Max1 = physx::intrinsics::selectMax(Max1, dp1);
		const PxReal dp2 = triVerts[2].dot(axis);
		Min1 = physx::intrinsics::selectMin(Min1, dp2);
		Max1 = physx::intrinsics::selectMax(Max1, dp2);
	}

	// Test projections
	if(max0<Min1 || Max1<min0)
		return false;

	const PxReal d0 = max0 - Min1;
	PX_ASSERT(d0>=0.0f);
	const PxReal d1 = Max1 - min0;
	PX_ASSERT(d1>=0.0f);
	depth = physx::intrinsics::selectMin(d0, d1);
	return true;
}

PX_FORCE_INLINE static PxVec3 PxcComputeTriangleNormal(const PxVec3* PX_RESTRICT triVerts)
{
	return ((triVerts[0]-triVerts[1]).cross(triVerts[0]-triVerts[2])).getNormalized();
}

PX_FORCE_INLINE static PxVec3 PxcComputeTriangleCenter(const PxVec3* PX_RESTRICT triVerts)
{
	static const PxReal inv3 = 1.0f / 3.0f;
	return (triVerts[0] + triVerts[1] + triVerts[2]) * inv3;
}

static bool PxcCapsuleTriOverlap3(PxU8 edgeFlags, const Segment& segment, PxReal radius, const PxVec3* PX_RESTRICT triVerts,
								  PxReal* PX_RESTRICT t=NULL, PxVec3* PX_RESTRICT pp=NULL)
{
	PxReal penDepth = PX_MAX_REAL;

	// Test normal
	PxVec3 sep = PxcComputeTriangleNormal(triVerts);
	if(!PxcTestAxis(sep, segment, radius, triVerts, penDepth))
		return false;

	// Test edges
	// PT: are those flags correct? Shouldn't we use ETD_CONVEX_EDGE_01/etc ?
	//const PxU8 ignoreEdgeFlag[] = {1, 4, 2};  // for edges 0-1, 1-2, 2-0 (see InternalTriangleMeshData::mExtraTrigData)
	// ML:: use the active edge flag instead of the concave flag
	const PxU32 activeEdgeFlag[] = {ETD_CONVEX_EDGE_01, ETD_CONVEX_EDGE_12, ETD_CONVEX_EDGE_20};
	const PxVec3 capsuleAxis = (segment.p1 - segment.p0).getNormalized();
	for(PxU32 i=0;i<3;i++)
	{
		//bool active =((edgeFlags & ignoreEdgeFlag[i]) == 0);
		
		if(edgeFlags & activeEdgeFlag[i])
		{
		
			const PxVec3 e0 = triVerts[i];
//			const PxVec3 e1 = triVerts[(i+1)%3];
			const PxVec3 e1 = triVerts[Ps::getNextIndex3(i)];
			const PxVec3 edge = e0 - e1;

			PxVec3 cross = capsuleAxis.cross(edge);
			if(!isAlmostZero(cross))
			{
				cross = cross.getNormalized();
				PxReal d;
				if(!PxcTestAxis(cross, segment, radius, triVerts, d))
					return false;
				if(d<penDepth)
				{
					penDepth = d;
					sep = cross;
				}
			}
		}
	}

	const PxVec3 capsuleCenter = segment.computeCenter();
	const PxVec3 triCenter = PxcComputeTriangleCenter(triVerts);
	const PxVec3 witness = capsuleCenter - triCenter;

	if(sep.dot(witness) < 0.0f)
		sep = -sep;

	if(t)	*t = penDepth;
	if(pp)	*pp = sep;

	return true;
}

static void PxcGenerateVFContacts(	const Cm::Matrix34& meshAbsPose, ContactBuffer& contactBuffer, const Segment& segment,
									const PxReal radius, const PxVec3* PX_RESTRICT triVerts, const PxVec3& normal, 
									PxU32 triangleIndex, PxReal contactDistance)
{
	const PxVec3* PX_RESTRICT Ptr = &segment.p0;
	for(PxU32 i=0;i<2;i++)
	{
		const PxVec3& Pos = Ptr[i];
		PxReal t,u,v;
		if(intersectRayTriangleCulling(Pos, -normal, triVerts[0], triVerts[1], triVerts[2], t, u, v, 1e-3f) && t < radius + contactDistance)
		{
			const PxVec3 Hit = meshAbsPose.transform(Pos - t * normal);
			const PxVec3 wn = meshAbsPose.rotate(normal);
			
			contactBuffer.contact(Hit, wn, t-radius, PXC_CONTACT_NO_FACE_INDEX, triangleIndex);
			#if DEBUG_RENDER_MESHCONTACTS
			PxScene *s; PxGetPhysics().getScenes(&s, 1, 0);
			Cm::RenderOutput((Cm::RenderBuffer&)s->getRenderBuffer()) << Cm::RenderOutput::LINES << PxDebugColor::eARGB_BLUE // red
				<< Hit << (Hit + wn * 10.0f);
			#endif
		}
	}
}

// PT: PxcGenerateEEContacts2 uses a segment-triangle distance function, which breaks when the segment
// intersects the triangle, in which case you need to switch to a penetration-depth computation.
// If you don't do this thin capsules don't work.
static void PxcGenerateEEContacts(	const Cm::Matrix34& meshAbsPose, ContactBuffer& contactBuffer, const Segment& segment, const PxReal radius,
									const PxVec3* PX_RESTRICT triVerts, const PxVec3& normal, PxU32 triangleIndex)
{
	PxVec3 s0 = segment.p0;
	PxVec3 s1 = segment.p1;
	Ps::makeFatEdge(s0, s1, fatBoxEdgeCoeff);

	for(PxU32 i=0;i<3;i++)
	{
		PxReal dist;
		PxVec3 ip;
		if(intersectEdgeEdge(triVerts[i], triVerts[Ps::getNextIndex3(i)], -normal, s0, s1, dist, ip))
		{
			ip = meshAbsPose.transform(ip);
			const PxVec3 wn = meshAbsPose.rotate(normal);

			contactBuffer.contact(ip, wn, - (radius + dist), PXC_CONTACT_NO_FACE_INDEX, triangleIndex);
			#if DEBUG_RENDER_MESHCONTACTS
			PxScene *s; PxGetPhysics().getScenes(&s, 1, 0);
			Cm::RenderOutput((Cm::RenderBuffer&)s->getRenderBuffer()) << Cm::RenderOutput::LINES << PxDebugColor::eARGB_BLUE // red
				<< ip << (ip + wn * 10.0f);
			#endif
		}
	}
}

static void PxcGenerateEEContacts2(	const Cm::Matrix34& meshAbsPose, ContactBuffer& contactBuffer, const Segment& segment, const PxReal radius,
									const PxVec3* PX_RESTRICT triVerts, const PxVec3& normal, PxU32 triangleIndex, PxReal contactDistance)
{
	PxVec3 s0 = segment.p0;
	PxVec3 s1 = segment.p1;
	Ps::makeFatEdge(s0, s1, fatBoxEdgeCoeff);

	for(PxU32 i=0;i<3;i++)
	{
		PxReal dist;
		PxVec3 ip;
		if(intersectEdgeEdge(triVerts[i], triVerts[Ps::getNextIndex3(i)], normal, s0, s1, dist, ip) && dist < radius+contactDistance)
		{
			ip = meshAbsPose.transform(ip);
			const PxVec3 wn = meshAbsPose.rotate(normal);

			contactBuffer.contact(ip, wn, dist - radius, PXC_CONTACT_NO_FACE_INDEX, triangleIndex);
			#if DEBUG_RENDER_MESHCONTACTS
			PxScene *s; PxGetPhysics().getScenes(&s, 1, 0);
			Cm::RenderOutput((Cm::RenderBuffer&)s->getRenderBuffer()) << Cm::RenderOutput::LINES << PxDebugColor::eARGB_BLUE // red
				<< ip << (ip + wn * 10.0f);
			#endif
		}
	}
}

namespace
{
struct CapsuleMeshContactGeneration
{
	ContactBuffer&		mContactBuffer;
	const Cm::Matrix34	mMeshAbsPose;
	const Segment&		mMeshCapsule;
#ifdef USE_AABB_TRI_CULLING
	PxVec3				mBC;
	PxVec3				mBE;
#endif
	PxReal				mInflatedRadius;
	PxReal				mContactDistance;
	PxReal				mShapeCapsuleRadius;

	CapsuleMeshContactGeneration(ContactBuffer& contactBuffer, const PxTransform& transform1, const Segment& meshCapsule, PxReal inflatedRadius, PxReal contactDistance, PxReal shapeCapsuleRadius) :
		mContactBuffer		(contactBuffer),
		mMeshAbsPose		(Cm::Matrix34(transform1)),
		mMeshCapsule		(meshCapsule),
		mInflatedRadius		(inflatedRadius),
		mContactDistance	(contactDistance),
		mShapeCapsuleRadius	(shapeCapsuleRadius)
	{
		PX_ASSERT(contactBuffer.count==0);
#ifdef USE_AABB_TRI_CULLING
		mBC = (meshCapsule.p0 + meshCapsule.p1)*0.5f;
		const PxVec3 be = (meshCapsule.p0 - meshCapsule.p1)*0.5f;
		mBE.x = fabsf(be.x) + inflatedRadius;
		mBE.y = fabsf(be.y) + inflatedRadius;
		mBE.z = fabsf(be.z) + inflatedRadius;
#endif
	}

	void processTriangle(PxU32 triangleIndex, const PxVec3* triVerts, PxU8 extraData/*, const PxU32* vertInds*/)
	{
#ifdef USE_AABB_TRI_CULLING
	#if VISUALIZE_CULLING_BOX
		{
			Cm::RenderOutput& out = context.mRenderOutput;
			PxTransform idt = PxTransform(PxIdentity);
			out << idt;
			out << 0xffffffff;
			out << Cm::DebugBox(mBC, mBE, true);
		}
	#endif
#endif

#ifdef USE_AABB_TRI_CULLING
		if(!intersectTriangleBox(mBC, mBE, triVerts[0], triVerts[1], triVerts[2]))
			return;
#endif

#ifdef USE_CAPSULE_TRI_PROJ_CULLING
		PxVec3 triCenter = (triVerts[0] + triVerts[1] + triVerts[2])*0.33333333f;
		PxVec3 delta = mBC - triCenter;

		PxReal depth;
		if(!PxcTestAxis(delta, mMeshCapsule, mInflatedRadius, triVerts, depth))
			return;
#endif

#if VISUALIZE_TOUCHED_TRIS
		gVisualizeTri(triVerts[0], triVerts[1], triVerts[2], context, PxDebugColor::eARGB_RED);
#endif

#ifdef USE_CAPSULE_TRI_SAT_CULLING
		PxVec3 SepAxis;
		if(!PxcCapsuleTriOverlap3(extraData, mMeshCapsule, mInflatedRadius, triVerts, NULL, &SepAxis)) 
			return;
#endif

		const PxVec3& p0 = triVerts[0];
		const PxVec3& p1 = triVerts[1];
		const PxVec3& p2 = triVerts[2];

		PxReal t,u,v;
		const PxReal squareDist = distanceSegmentTriangleSquared(mMeshCapsule, p0, p1-p0, p2-p0, &t, &u, &v);

		// PT: do cheaper test first!
		if(squareDist >= mInflatedRadius*mInflatedRadius) 
			return;

		// Cull away back-facing triangles   
		// PT: TODO: p1-p0 and p2-p0 have already been computed above
		const PxPlane localPlane(p0, p1, p2);
		if(localPlane.distance(mBC) < 0.0f)
			return;

		if(squareDist > 0.001f*0.001f)
		{
			// Contact information
			PxVec3 normal;
			if(selectNormal(extraData, u, v))
			{
				normal = localPlane.n;
			}
			else
			{
				const PxVec3 pointOnTriangle = Ps::computeBarycentricPoint(p0, p1, p2, u, v);

				const PxVec3 pointOnSegment = mMeshCapsule.getPointAt(t);
				normal = pointOnSegment - pointOnTriangle;
				const PxReal l = normal.magnitude();
				if(l == 0.0f)
					return;
				normal = normal / l;
			}

			PxcGenerateEEContacts2(mMeshAbsPose, mContactBuffer, mMeshCapsule, mShapeCapsuleRadius, triVerts, normal, triangleIndex, mContactDistance);
			PxcGenerateVFContacts(mMeshAbsPose, mContactBuffer, mMeshCapsule, mShapeCapsuleRadius, triVerts, normal, triangleIndex, mContactDistance);
		}
		else
		{
			PxVec3 SepAxis;
			if(!PxcCapsuleTriOverlap3(extraData, mMeshCapsule, mInflatedRadius, triVerts, NULL, &SepAxis)) 
				return;

			PxcGenerateEEContacts(mMeshAbsPose, mContactBuffer, mMeshCapsule, mShapeCapsuleRadius, triVerts, SepAxis, triangleIndex);
			PxcGenerateVFContacts(mMeshAbsPose, mContactBuffer, mMeshCapsule, mShapeCapsuleRadius, triVerts, SepAxis, triangleIndex, mContactDistance);
		}
	}

private:
	CapsuleMeshContactGeneration& operator=(const CapsuleMeshContactGeneration&);
};

struct CapsuleMeshContactGenerationCallback : MeshHitCallback<PxRaycastHit>
{
	CapsuleMeshContactGeneration		mGeneration;
	const Cm::FastVertex2ShapeScaling&	mScaling;
	bool								mIdtMeshScale;
	const InternalTriangleMeshData*		mMeshData;

	CapsuleMeshContactGenerationCallback(
		ContactBuffer& contactBuffer,
		const PxTransform& transform1, const Segment& meshCapsule,
		PxReal inflatedRadius, const Cm::FastVertex2ShapeScaling& scaling, PxReal contactDistance,
		PxReal shapeCapsuleRadius, const InternalTriangleMeshData* meshData, bool idtMeshScale
	)	:
		MeshHitCallback<PxRaycastHit>	(CallbackMode::eMULTIPLE),
		mGeneration						(contactBuffer, transform1, meshCapsule, inflatedRadius, contactDistance, shapeCapsuleRadius),
		mScaling						(scaling),
		mIdtMeshScale					(idtMeshScale),
		mMeshData						(meshData)
	{
		PX_ASSERT(contactBuffer.count==0);
	}

	virtual PxAgain processHit(
		const PxRaycastHit& hit, const PxVec3& v0, const PxVec3& v1, const PxVec3& v2, PxReal&, const PxU32* /*vInds*/)
	{
		PxVec3 v[3];
		if(mIdtMeshScale)
		{
			// PT: TODO: revisit this, avoid the copy
			v[0] = v0;
			v[1] = v1;
			v[2] = v2;
		}
		else
		{
			v[0] = mScaling * v0;
			v[1] = mScaling * v1;
			v[2] = mScaling * v2;
		}

		const PxU32 triangleIndex = hit.faceIndex;

		//ML::set all the edges to be active, if the mExtraTrigData exist, we overwrite this flag
		PxU8 extraData = ETD_CONVEX_EDGE_01|ETD_CONVEX_EDGE_12|ETD_CONVEX_EDGE_20;
		if(mMeshData->mExtraTrigData)
		{
			extraData = Cm::memFetch<PxU8>(
				Cm::MemFetchPtr(mMeshData->mExtraTrigData)+triangleIndex*sizeof(mMeshData->mExtraTrigData[0]), 5);
			Cm::memFetchWait(5);
		}

		mGeneration.processTriangle(triangleIndex, v, extraData);
		return true;
	}

private:
	CapsuleMeshContactGenerationCallback& operator=(const CapsuleMeshContactGenerationCallback&);
};
}

static /*PX_FORCE_INLINE*/ Segment computeLocalCapsule(const PxTransform& transform0, const PxTransform& transform1, const PxCapsuleGeometry& shapeCapsule)
{
	// PT: TODO: avoid world-space here
	Segment worldCapsule;
	getCapsuleSegment(transform0, shapeCapsule, worldCapsule);

	//to avoid transforming all the trig vertices to world space, transform the capsule to the mesh's space instead
	return Segment(	// Capsule in mesh space
		transform1.transformInv(worldCapsule.p0),
		transform1.transformInv(worldCapsule.p1));
}

bool Gu::contactCapsuleMesh(GU_CONTACT_METHOD_ARGS)
{
	PX_UNUSED(cache);

	const PxCapsuleGeometry& shapeCapsule = shape0.get<const PxCapsuleGeometry>();
	const PxTriangleMeshGeometryLL& shapeMesh = shape1.get<const PxTriangleMeshGeometryLL>();

	const bool idtMeshScale = shapeMesh.scale.isIdentity();

	Cm::FastVertex2ShapeScaling meshScaling;	// PT: TODO: remove default ctor
	if(!idtMeshScale)
		meshScaling.init(shapeMesh.scale);

	const PxReal inflatedRadius = shapeCapsule.radius + contactDistance;		//AM: inflate!
	const Segment meshCapsule = computeLocalCapsule(transform0, transform1, shapeCapsule);

	const InternalTriangleMeshData* meshData = shapeMesh.meshData;
#ifdef __SPU__
	// fetch meshData to temp storage
	PX_ALIGN_PREFIX(16) char meshDataBuf[sizeof(InternalTriangleMeshData)] PX_ALIGN_SUFFIX(16);
	Cm::memFetchAlignedAsync(Cm::MemFetchPtr(meshDataBuf), Cm::MemFetchPtr(shapeMesh.meshData), sizeof(InternalTriangleMeshData), 5);
	Cm::memFetchWait(5);
	meshData = reinterpret_cast<const InternalTriangleMeshData*>(meshDataBuf);
#endif
	CapsuleMeshContactGenerationCallback callback(contactBuffer, transform1, meshCapsule,
		inflatedRadius, meshScaling, contactDistance, shapeCapsule.radius, meshData, idtMeshScale);

	//switched from capsuleCollider to boxCollider so we can support nonuniformly scaled meshes by scaling the query region:

	//bound the capsule in shape space by an OBB:
	Box queryBox;
	{
		const Capsule queryCapsule(meshCapsule, inflatedRadius);
		queryBox.create(queryCapsule);
	}

	//apply the skew transform to the box:
	if(!idtMeshScale)
		meshScaling.transformQueryBounds(queryBox.center, queryBox.extents, queryBox.rot);

	RTreeMidphaseData hmd;
	meshData->mCollisionModel.getRTreeMidphaseData(hmd);

	MPT_SET_CONTEXT("cocm", transform1, shapeMesh.scale);
	MeshRayCollider::collideOBB(queryBox, true, hmd, callback);

	return contactBuffer.count > 0;
}

namespace
{
struct CapsuleHeightfieldContactGenerationCallback : EntityReport<PxU32>
{
	CapsuleMeshContactGeneration	mGeneration;
	HeightFieldUtil&				mHfUtil;
	const PxTransform&				mTransform1;

	CapsuleHeightfieldContactGenerationCallback(
		ContactBuffer& contactBuffer,
		const PxTransform& transform1, HeightFieldUtil& hfUtil, const Segment& meshCapsule,
		PxReal inflatedRadius, PxReal contactDistance, PxReal shapeCapsuleRadius
	) : 
		mGeneration	(contactBuffer, transform1, meshCapsule, inflatedRadius, contactDistance, shapeCapsuleRadius),
		mHfUtil		(hfUtil),
		mTransform1	(transform1)
	{
		PX_ASSERT(contactBuffer.count==0);
	}

	virtual bool onEvent(PxU32 nb, PxU32* indices)
	{
		const PxU8 nextInd[] = {2,0,1};

		while(nb--)
		{
			const PxU32 triangleIndex = *indices++;

			PxU32 vertIndices[3];
			PxTriangle currentTriangle;	// in world space
			PxU32 adjInds[3];
			mHfUtil.getTriangle(mTransform1, currentTriangle, vertIndices, adjInds, triangleIndex, false, false);

			PxVec3 normal;
			currentTriangle.normal(normal);

			PxU8 triFlags = 0; //KS - temporary until we can calculate triFlags for HF

			for(PxU32 a = 0; a < 3; ++a)
			{
				if(adjInds[a] != 0xFFFFFFFF)
				{
					PxTriangle adjTri;
					mHfUtil.getTriangle(mTransform1, adjTri, NULL, NULL, adjInds[a], false, false);
					//We now compare the triangles to see if this edge is active

					PxVec3 adjNormal;
					adjTri.denormalizedNormal(adjNormal);
					PxU32 otherIndex = nextInd[a];
					PxF32 projD = adjNormal.dot(currentTriangle.verts[otherIndex] - adjTri.verts[0]);
					if(projD < 0.f)
					{
						adjNormal.normalize();

						PxF32 proj = adjNormal.dot(normal);

						if(proj < 0.999f)
						{
							triFlags |= 1 << (a+3);
						}
					}
				}
				else
				{
					triFlags |= 1 << (a+3);
				}
			}

			mGeneration.processTriangle(triangleIndex, currentTriangle.verts, triFlags);
		}
		return true;
	}

private:
	CapsuleHeightfieldContactGenerationCallback& operator=(const CapsuleHeightfieldContactGenerationCallback&);
};
}

bool Gu::contactCapsuleHeightfield(GU_CONTACT_METHOD_ARGS)
{
	PX_UNUSED(cache);

	const PxCapsuleGeometry& shapeCapsule = shape0.get<const PxCapsuleGeometry>();
	const PxHeightFieldGeometryLL& shapeMesh = shape1.get<const PxHeightFieldGeometryLL>();

	const PxReal inflatedRadius = shapeCapsule.radius + contactDistance;		//AM: inflate!
	const Segment meshCapsule = computeLocalCapsule(transform0, transform1, shapeCapsule);

	// We must be in local space to use the cache

#ifdef __SPU__
	const HeightField& hf = *Cm::memFetchAsync<const HeightField>(HeightFieldBuffer, Cm::MemFetchPtr(static_cast<HeightField*>(shapeMesh.heightField)), sizeof(HeightField), 1);
	Cm::memFetchWait(1);
#if HF_TILED_MEMORY_LAYOUT
	g_sampleCache.init((uintptr_t)(hf.getData().samples), hf.getData().tilesU);
#endif
#else
	const HeightField& hf = *static_cast<HeightField*>(shapeMesh.heightField);
#endif
	HeightFieldUtil hfUtil(shapeMesh, hf);

	CapsuleHeightfieldContactGenerationCallback callback(
		contactBuffer, transform1, hfUtil, meshCapsule, inflatedRadius, contactDistance, shapeCapsule.radius);

	//switched from capsuleCollider to boxCollider so we can support nonuniformly scaled meshes by scaling the query region:

	//bound the capsule in shape space by an OBB:

	PxBounds3 bounds;
	bounds.maximum = PxVec3(shapeCapsule.halfHeight + inflatedRadius, inflatedRadius, inflatedRadius);
	bounds.minimum = -bounds.maximum;

	bounds = PxBounds3::transformFast(transform1.transformInv(transform0), bounds);

	MPT_SET_CONTEXT("coch", PxTransform(), PxMeshScale());
	hfUtil.overlapAABBTriangles(transform1, bounds, 0, &callback);

	return contactBuffer.count > 0;
}
