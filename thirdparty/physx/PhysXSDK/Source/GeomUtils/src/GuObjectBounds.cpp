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

#include "GuGeometryUnion.h"
#include "PxBounds3.h"
#include "PsMathUtils.h"
#include "GuConvexMeshData.h"
#include "GuTriangleMeshData.h"
#include "GuHeightField.h"
#include "GuHeightFieldData.h"
#include "CmMemFetch.h"
#include "CmMatrix34.h"

using namespace physx;

// PT: specialized version only computing the extent. TODO: refactor
static PX_FORCE_INLINE void basisExtent(PxVec3& w, const PxMat33& basis, const PxVec3& extent)
{
	// extended basis vectors
	const PxVec3 c0 = basis.column0 * extent.x;
	const PxVec3 c1 = basis.column1 * extent.y;
	const PxVec3 c2 = basis.column2 * extent.z;

	// find combination of base vectors that produces max. distance for each component = sum of abs()
	w.x = PxAbs(c0.x) + PxAbs(c1.x) + PxAbs(c2.x);
	w.y = PxAbs(c0.y) + PxAbs(c1.y) + PxAbs(c2.y);
	w.z = PxAbs(c0.z) + PxAbs(c1.z) + PxAbs(c2.z);
}

static PX_FORCE_INLINE void basisExtent(PxVec3& w, const Cm::Matrix34& basis, const PxVec3& extent)
{
	// extended basis vectors
	const PxVec3 c0 = basis.base0 * extent.x;
	const PxVec3 c1 = basis.base1 * extent.y;
	const PxVec3 c2 = basis.base2 * extent.z;

	// find combination of base vectors that produces max. distance for each component = sum of abs()
	w.x = PxAbs(c0.x) + PxAbs(c1.x) + PxAbs(c2.x);
	w.y = PxAbs(c0.y) + PxAbs(c1.y) + PxAbs(c2.y);
	w.z = PxAbs(c0.z) + PxAbs(c1.z) + PxAbs(c2.z);
}

// PT: this one is not used here but we should revisit the original code, looks like there's a useless conversion
/*
static PX_FORCE_INLINE void transformNoEmptyTest(PxVec3& c, PxVec3& ext, const PxTransform& t, const PxBounds3& bounds)
{
//PX_INLINE PxBounds3 PxBounds3::basisExtent(const PxVec3& center, const PxMat33& basis, const PxVec3& extent)

	const PxVec3 boundsCenter = bounds.getCenter();
	c = t.transform(boundsCenter);
	const PxMat33 basis(t.q);
	PxVec3 c2 = (basis * boundsCenter) + t.p;
//	return PxBounds3::basisExtent(center, basis, bounds.getExtents());
	basisExtent(ext, basis, bounds.getExtents());
}
*/

// PT: no "isEmpty" test in that one.
static PX_FORCE_INLINE void transformNoEmptyTest(PxVec3& c, PxVec3& ext, const Cm::Matrix34& matrix, const PxBounds3& bounds)
{
	const PxVec3 boundsCenter = bounds.getCenter();
	c = matrix.transform( boundsCenter );
	basisExtent(ext, matrix, bounds.getExtents());
}

// PT: this version shows what is really going on here, and it's not pretty. Still, is has no FCMP and an order of magnitude less LHS than the original code.
#define IEEE_1_0				0x3f800000					//!< integer representation of 1.0
static PX_FORCE_INLINE void transformNoEmptyTest(PxVec3& c, PxVec3& ext, const PxTransform& transform, const PxMeshScale& scale, const PxBounds3& bounds)
{
/*	PT: so the decomposition is:

	const PxMat33Legacy tmp(transform.q);	// quat-to-matrix

	const PxMat33 tmp2 = scale.toMat33();	// quat-to-matrix + transpose + matrix multiply ==> could be precomputed

	const PxMat34Legacy legacy(tmp * tmp2, transform.p);	// matrix conversion + legacy matrix multiply

	transformNoEmptyTest(c, ext, legacy, bounds);

	Writing it with a single line has less LHS, so:
*/
#ifdef _XBOX
	if(PX_IR(scale.scale.x)==IEEE_1_0 && PX_IR(scale.scale.y)==IEEE_1_0 && PX_IR(scale.scale.z)==IEEE_1_0)
#else
	if(scale.isIdentity())
#endif
	{
		transformNoEmptyTest(c, ext, Cm::Matrix34(transform), bounds);
	}
	else
	{
		transformNoEmptyTest(c, ext, Cm::Matrix34(PxMat33(transform.q) * scale.toMat33(), transform.p), bounds);
	}
}

static void computeMeshBounds(const PxTransform& pose, const PxBounds3* PX_RESTRICT localSpaceBounds, const PxMeshScale& meshScale, PxVec3& origin, PxVec3& extent)
{
	Ps::prefetchLine(localSpaceBounds);	// PT: this one helps reducing L2 misses in transformNoEmptyTest
	transformNoEmptyTest(origin, extent, pose, meshScale, *localSpaceBounds);
}

static void computePlaneBounds(PxBounds3& bounds, const PxTransform& pose, float contactOffset)
{
	// PT: A plane is infinite, so usually the bounding box covers the whole world.
	// Now, in particular cases when the plane is axis-aligned, we can take
	// advantage of this to compute a smaller bounding box anyway.

	// PT: we use PX_MAX_BOUNDS_EXTENTS to be compatible with PxBounds3::setMaximal,
	// and to make sure that the value doesn't collide with the BP's sentinels.
	const PxF32 bigValue = PX_MAX_BOUNDS_EXTENTS;
//	const PxF32 bigValue = 1000000.0f;
	PxVec3 minPt = PxVec3(-bigValue, -bigValue, -bigValue);
	PxVec3 maxPt = PxVec3(bigValue, bigValue, bigValue);

	const PxVec3 planeNormal = pose.q.getBasisVector0();
	const PxPlane plane(pose.p, planeNormal);

	const float nx = PxAbs(planeNormal.x);
	const float ny = PxAbs(planeNormal.y);
	const float nz = PxAbs(planeNormal.z);
	const float epsilon = 1e-6f;
	const float oneMinusEpsilon = 1.0f - epsilon;
	if(nx>oneMinusEpsilon && ny<epsilon && nz<epsilon)
	{
		if(planeNormal.x>0.0f)	maxPt.x = -plane.d;
		else					minPt.x = plane.d;
	}
	else if(nx<epsilon && ny>oneMinusEpsilon && nz<epsilon)
	{
		if(planeNormal.y>0.0f)	maxPt.y = -plane.d;
		else					minPt.y = plane.d;
	}
	else if(nx<epsilon && ny<epsilon && nz>oneMinusEpsilon)
	{
		if(planeNormal.z>0.0f)	maxPt.z = -plane.d;
		else					minPt.z = plane.d;
	}

	// PT: it is important to compute the min/max form directly without going through the
	// center/extents intermediate form. With PX_MAX_BOUNDS_EXTENTS, those back-and-forth
	// computations destroy accuracy.
	const PxVec3 offset(contactOffset);
	bounds.minimum = minPt - offset;
	bounds.maximum = maxPt + offset;
}

void Gu::GeometryUnion::computeBounds(
	PxBounds3& bounds, const PxTransform& pose, float contactOffset,
	const PxBounds3* PX_RESTRICT localSpaceBounds) const
{
	// Box, Convex, Mesh and HeightField will compute local bounds and pose to world space.
	// Sphere, Capsule & Plane will compute world space bounds directly.

	switch (getType())
	{
	case PxGeometryType::eSPHERE:
		{
			PX_ASSERT(!localSpaceBounds);

			const PxSphereGeometry& shape = get<const PxSphereGeometry>();
			const PxVec3 extents(shape.radius+contactOffset);
			bounds.minimum = pose.p - extents;
			bounds.maximum = pose.p + extents;
		}
		break;

	case PxGeometryType::ePLANE:
		{
			PX_ASSERT(!localSpaceBounds);

			computePlaneBounds(bounds, pose, contactOffset);
		}
		break;

	case PxGeometryType::eCAPSULE:
		{
			PX_ASSERT(!localSpaceBounds);

			const PxCapsuleGeometry& shape = get<const PxCapsuleGeometry>();
			const PxVec3 d = pose.q.getBasisVector0();
			PxVec3 extents;
			for(PxU32 ax = 0; ax<3; ax++)
				extents[ax] = PxAbs(d[ax]) * shape.halfHeight + shape.radius + contactOffset;
			bounds.minimum = pose.p - extents;
			bounds.maximum = pose.p + extents;
		}
		break;

	case PxGeometryType::eBOX:
		{
			PX_ASSERT(!localSpaceBounds);

			const PxBoxGeometry& shape = get<const PxBoxGeometry>();

			PxVec3 extents;
			::basisExtent(extents, PxMat33(pose.q), shape.halfExtents);

			const PxVec3 offset(contactOffset);	// PT: TODO: pass this directly to basisExtent?
			bounds.minimum = pose.p - extents - offset;
			bounds.maximum = pose.p + extents + offset;
		}
		break;

	case PxGeometryType::eCONVEXMESH:
		{
			const PxConvexMeshGeometryLL& shape = get<const PxConvexMeshGeometryLL>();

			PxVec3 origin, extents;

			if(localSpaceBounds)
			{
				computeMeshBounds(pose, localSpaceBounds, shape.scale, origin, extents);
			}
			else
			{
				Cm::MemFetchBufferA<ConvexHullData> cvxData;
				ConvexHullData* data = Cm::memFetchAsync<ConvexHullData>(Cm::MemFetchPtr(shape.hullData), 1, cvxData);
				Cm::memFetchWait(1);

				computeMeshBounds(pose, &data->mAABB, shape.scale, origin, extents);
			}

			const PxVec3 offset(contactOffset);	// PT: TODO: pass this directly to computeMeshBounds?
			bounds.minimum = origin - extents - offset;
			bounds.maximum = origin + extents + offset;
		}
		break;

	case PxGeometryType::eTRIANGLEMESH:
		{
			const PxTriangleMeshGeometryLL& shape = get<const PxTriangleMeshGeometryLL>();
			PxVec3 origin, extents;
			if(localSpaceBounds)
			{
				computeMeshBounds(pose, localSpaceBounds, shape.scale, origin, extents);
			}
			else
			{
				Cm::MemFetchBufferA<Gu::InternalTriangleMeshData> dataBuffer;
				Gu::InternalTriangleMeshData* data = Cm::memFetchAsync<Gu::InternalTriangleMeshData>(Cm::MemFetchPtr(shape.meshData), 1, dataBuffer);
				Cm::memFetchWait(1);

				computeMeshBounds(pose, &data->mAABB, shape.scale, origin, extents);
			}
			const PxVec3 offset(contactOffset);	// PT: TODO: pass this directly to computeMeshBounds?
			bounds.minimum = origin - extents - offset;
			bounds.maximum = origin + extents + offset;
		}
		break;

	case PxGeometryType::eHEIGHTFIELD:
		{
			const PxHeightFieldGeometryLL& shape = get<const PxHeightFieldGeometryLL>();
			const PxMeshScale scale(PxVec3(shape.rowScale, shape.heightScale, shape.columnScale), PxQuat(PxIdentity));
			PxVec3 origin, extents;
			if(localSpaceBounds)
			{
				PxBounds3 aabb = *localSpaceBounds;
				const PxF32 thickness = shape.heightFieldData->thickness;
				const PxReal thicknessScaled = thickness/shape.heightScale;
				if(thicknessScaled < 0.f)
					aabb.minimum.y += thicknessScaled;
				else
					aabb.maximum.y += thicknessScaled;

				computeMeshBounds(pose, &aabb, scale, origin, extents);
			}
			else
			{
				Cm::MemFetchBufferA<HeightFieldData> dataBuffer;
				HeightFieldData* data = Cm::memFetchAsync<HeightFieldData>(Cm::MemFetchPtr(shape.heightFieldData), 1, dataBuffer);
				Cm::memFetchWait(1);

				PxBounds3 aabb = data->mAABB;
				const PxF32 thickness = shape.heightFieldData->thickness;
				const PxReal thicknessScaled = thickness/shape.heightScale;
				if(thicknessScaled < 0.f)
					aabb.minimum.y += thicknessScaled;
				else
					aabb.maximum.y += thicknessScaled;

				computeMeshBounds(pose, &aabb, scale, origin, extents);
			}
			const PxVec3 offset(contactOffset);	// PT: TODO: pass this directly to computeMeshBounds?
			bounds.minimum = origin - extents - offset;
			bounds.maximum = origin + extents + offset;
		}
		break;
	case PxGeometryType::eGEOMETRY_COUNT:
	case PxGeometryType::eINVALID:
	default:
		{
			PX_ASSERT(0);		
#ifndef __SPU__
			Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "Gu::GeometryUnion::computeBounds: Unknown shape type.");
#endif
		}
	}
}

PxF32 Gu::GeometryUnion::computeBoundsWithCCDThreshold(PxVec3& origin, PxVec3& extent, const PxTransform& pose, const PxBounds3* PX_RESTRICT localSpaceBounds) const
{
	// Box, Convex, Mesh and HeightField will compute local bounds and pose to world space.
	// Sphere, Capsule & Plane will compute world space bounds directly.

	const PxReal inSphereRatio = 0.75f;

	//The CCD thresholds are as follows:
	//(1) sphere = inSphereRatio * radius
	//(2) plane = inf (we never need CCD against this shape)
	//(3) capsule = inSphereRatio * radius
	//(4) box = inSphereRatio * (box minimum extent axis)
	//(5) convex = inSphereRatio * convex in-sphere * min scale
	//(6) triangle mesh = 0.f (polygons have 0 thickness)
	//(7) heightfields = 0.f (polygons have 0 thickness)

	//The decision to enter CCD depends on the sum of the shapes' CCD thresholds. One of the 2 shapes must be a 
	//sphere/capsule/box/convex so the sum of the CCD thresholds will be non-zero.

	switch (getType())
	{
	case PxGeometryType::eSPHERE:
		{
			PX_ASSERT(!localSpaceBounds);

			const PxSphereGeometry& shape = get<const PxSphereGeometry>();
			origin = pose.p;
			extent = PxVec3(shape.radius, shape.radius, shape.radius);
			return shape.radius*inSphereRatio;
		}
	case PxGeometryType::ePLANE:
		{
			PX_ASSERT(!localSpaceBounds);

			PxBounds3 bounds;
			computePlaneBounds(bounds, pose, 0.0f);
			origin = bounds.getCenter();
			extent = bounds.getExtents();
			return PX_MAX_REAL;
		}
	case PxGeometryType::eCAPSULE:
		{
			PX_ASSERT(!localSpaceBounds);

			const PxCapsuleGeometry& shape = get<const PxCapsuleGeometry>();
			origin = pose.p;
			const PxVec3 d = pose.q.getBasisVector0();
			for(PxU32 ax = 0; ax<3; ax++)
				extent[ax] = PxAbs(d[ax]) * shape.halfHeight + shape.radius;
			return shape.radius * inSphereRatio;
		}

	case PxGeometryType::eBOX:
		{
			PX_ASSERT(!localSpaceBounds);

			const PxBoxGeometry& shape = get<const PxBoxGeometry>();

			::basisExtent(extent, PxMat33(pose.q), shape.halfExtents);
			origin = pose.p;

			return PxMin(PxMin(shape.halfExtents.x, shape.halfExtents.y), shape.halfExtents.z)*inSphereRatio;
		}

	case PxGeometryType::eCONVEXMESH:
		{
			const PxConvexMeshGeometryLL& shape = get<const PxConvexMeshGeometryLL>();

			//KS - we need to fetch in data *all the time* because we need to get access to the in-sphere to be able to return the 
			//CCD threshold. If this fails on SPU, we may incorrectly disable CCD.
			Cm::MemFetchBufferA<ConvexHullData> cvxData;
			ConvexHullData* data = Cm::memFetchAsync<ConvexHullData>(Cm::MemFetchPtr(shape.hullData), 1, cvxData);
			Cm::memFetchWait(1);
			if(localSpaceBounds)
			{
				computeMeshBounds(pose, localSpaceBounds, shape.scale, origin, extent);

			}
			else
			{
				computeMeshBounds(pose, &data->mAABB, shape.scale, origin, extent);
			}

			return PxMin(shape.scale.scale.z, PxMin(shape.scale.scale.x, shape.scale.scale.y)) * data->mInternal.mRadius * inSphereRatio;
		}
	case PxGeometryType::eTRIANGLEMESH:
		{
			const PxTriangleMeshGeometryLL& shape = get<const PxTriangleMeshGeometryLL>();

			if(localSpaceBounds)
			{
				computeMeshBounds(pose, localSpaceBounds, shape.scale, origin, extent);
			}
			else
			{
				Cm::MemFetchBufferA<Gu::InternalTriangleMeshData> dataBuffer;
				Gu::InternalTriangleMeshData* data = Cm::memFetchAsync<Gu::InternalTriangleMeshData>(Cm::MemFetchPtr(shape.meshData), 1, dataBuffer);
				Cm::memFetchWait(1);

				computeMeshBounds(pose, &data->mAABB, shape.scale, origin, extent);
			}
			return 0.0f;
		}

	case PxGeometryType::eHEIGHTFIELD:
		{
			const PxHeightFieldGeometryLL& shape = get<const PxHeightFieldGeometryLL>();
			const PxMeshScale scale(PxVec3(shape.rowScale, shape.heightScale, shape.columnScale), PxQuat(PxIdentity));

			if(localSpaceBounds)
			{
				computeMeshBounds(pose, localSpaceBounds, scale, origin, extent);
			}
			else
			{
				Cm::MemFetchBufferA<HeightFieldData> dataBuffer;
				HeightFieldData* data = Cm::memFetchAsync<HeightFieldData>(Cm::MemFetchPtr(shape.heightFieldData), 1, dataBuffer);
				Cm::memFetchWait(1);

				computeMeshBounds(pose, &data->mAABB, scale, origin, extent);
			}
			return 0.f;
		}
	case PxGeometryType::eGEOMETRY_COUNT:
	case PxGeometryType::eINVALID:
	default:
		{
			PX_ASSERT(0);		
#ifndef __SPU__
			Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "Gu::GeometryUnion::computeBounds: Unknown shape type.");
#endif
		}
	}
	return PX_MAX_REAL;
}
