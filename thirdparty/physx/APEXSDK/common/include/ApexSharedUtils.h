/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEXSHAREDUTILS_H
#define APEXSHAREDUTILS_H

#include "NxApexUsingNamespace.h"
#include "NxApexDefs.h"
#include "NxApexUserProgress.h"
#include "NxRenderMeshAsset.h"

#include "NxFromPx.h"

#include "ApexString.h"
#include "PsArray.h"
#include "PsUtilities.h"
#include "NxApexUtils.h"
#include "foundation/PxPlane.h"

#include "ConvexHullParameters.h"

#include "NiCof44.h"

#if NX_SDK_VERSION_MAJOR == 2
class NxConvexMesh;
typedef NxConvexMesh	ConvexMesh;
#elif NX_SDK_VERSION_MAJOR == 3
namespace physx
{
class PxConvexMesh;
}
typedef physx::PxConvexMesh	ConvexMesh;
#endif

namespace physx
{
namespace apex
{

PX_INLINE physx::PxPlane toPxPlane(const ConvexHullParametersNS::Plane_Type& plane)
{
	return physx::PxPlane(plane.normal.x, plane.normal.y, plane.normal.z, plane.d);
}

/*
File-local functions and definitions
*/


/*
Global utilities
*/

// Diagonalize a symmetric 3x3 matrix.  Returns the eigenvectors in the first parameter, eigenvalues as the return value.
PxVec3 diagonalizeSymmetric(PxMat33& eigenvectors, const PxMat33& m);



PX_INLINE bool worldToLocalRay(physx::PxVec3& localorig, physx::PxVec3& localdir,
                               const physx::PxVec3& worldorig, const physx::PxVec3& worlddir,
                               const physx::PxMat34Legacy& localToWorldRT, const physx::PxVec3& scale)
{
	// Invert scales
	const physx::PxF32 detS = scale.x * scale.y * scale.z;
	if (detS == 0.0f)
	{
		return false;	// Not handling singular TMs
	}
	const physx::PxF32 recipDetS = 1.0f / detS;

	// Is it faster to do a bunch of multiplies, or a few divides?
	const physx::PxVec3 invS(scale.y * scale.z * recipDetS, scale.z * scale.x * recipDetS, scale.x * scale.y * recipDetS);

	// Create hull-local ray
	localToWorldRT.multiplyByInverseRT(worldorig, localorig);
	localorig = invS.multiply(localorig);
	localToWorldRT.M.multiplyByTranspose(worlddir, localdir);
	localdir = invS.multiply(localdir);

	return true;
}

// barycentric utilities
/**
This function starts recording the number of cycles elapsed.
\param		pa		[in] first vertex of triangle
\param		pb		[in] second vertex of triangle
\param		pc		[in] third vertex of triangle
\param		p		[in] vertex to generate barycentric coordinates for
\param		s		[out] the first barycentric coordinate
\param		t		[out] the second barycentric coordinate
\note the third barycentric coordinate is defined as (1 - s - t)
\see		EndProfile
*/
void generateBarycentricCoordinatesTri(const physx::PxVec3& pa, const physx::PxVec3& pb, const physx::PxVec3& pc, const physx::PxVec3& p, physx::PxF32& s, physx::PxF32& t);
void generateBarycentricCoordinatesTet(const physx::PxVec3& pa, const physx::PxVec3& pb, const physx::PxVec3& pc, const physx::PxVec3& pd, const physx::PxVec3& p, physx::PxVec3& bary);

struct OverlapLineSegmentAABBCache
{
	physx::PxVec3	sgnDir;
	physx::PxVec3	invDir;
};

PX_INLINE void computeOverlapLineSegmentAABBCache(OverlapLineSegmentAABBCache& cache, const physx::PxVec3& segmentDisp)
{
	cache.sgnDir = physx::PxVec3((physx::PxF32)(1 - (((int)(segmentDisp.x < 0.0f)) << 1)), (physx::PxF32)(1 - (((int)(segmentDisp.y < 0.0f)) << 1)), (physx::PxF32)(1 - (((int)(segmentDisp.z < 0.0f)) << 1)));
	physx::PxVec3 absDir = cache.sgnDir.multiply(segmentDisp);
	absDir += physx::PxVec3(PX_EPS_F32);	// To avoid divide-by-zero
	cache.invDir = physx::PxVec3(absDir.y * absDir.z, absDir.z * absDir.x, absDir.x * absDir.y);
	cache.invDir *= 1.0f / (absDir.x * cache.invDir.x);
}

PX_INLINE bool overlapLineSegmentAABBCached(const physx::PxVec3& segmentOrig, const OverlapLineSegmentAABBCache& cache, const physx::PxBounds3& aabb)
{
	const physx::PxVec3 center = 0.5f * (aabb.maximum + aabb.minimum);
	const physx::PxVec3 radii = 0.5f * (aabb.maximum - aabb.minimum);
	physx::PxVec3 disp = (center - segmentOrig).multiply(cache.sgnDir);
	physx::PxVec3 tMin = (disp - radii).multiply(cache.invDir);
	physx::PxVec3 tMax = (disp + radii).multiply(cache.invDir);
	int maxMinIndex = tMin.y > tMin.x;
	const int maxMinIndexIs2 = tMin.z > tMin[maxMinIndex];
	maxMinIndex = (maxMinIndex | maxMinIndexIs2) << maxMinIndexIs2;
	int minMaxIndex = tMax.y < tMax.x;
	const int minMaxIndexIs2 = tMax.z > tMax[minMaxIndex];
	minMaxIndex = (minMaxIndex | minMaxIndexIs2) << minMaxIndexIs2;
	const physx::PxF32 tIn = tMin[maxMinIndex];
	const physx::PxF32 tOut = tMax[minMaxIndex];
	return tIn < tOut && tOut > 0.0f && tIn < 1.0f;
}

PX_INLINE bool overlapLineSegmentAABB(const physx::PxVec3& segmentOrig, const physx::PxVec3& segmentDisp, const physx::PxBounds3& aabb)
{
	OverlapLineSegmentAABBCache cache;
	computeOverlapLineSegmentAABBCache(cache, segmentDisp);
	return overlapLineSegmentAABBCached(segmentOrig, cache, aabb);
}

struct IntPair
{
	void	set(physx::PxI32 _i0, physx::PxI32 _i1)
	{
		i0 = _i0;
		i1 = _i1;
	}

	physx::PxI32	i0, i1;

	static	int	compare(const void* a, const void* b)
	{
		const physx::PxI32 diff0 = ((IntPair*)a)->i0 - ((IntPair*)b)->i0;
		return diff0 ? diff0 : (((IntPair*)a)->i1 - ((IntPair*)b)->i1);
	}
};

PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, IntPair& p)
{
	p.i0 = (physx::PxI32)stream.readDword();
	p.i1 = (physx::PxI32)stream.readDword();
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const IntPair& p)
{
	stream.storeDword((physx::PxU32)p.i0);
	stream.storeDword((physx::PxU32)p.i1);
	return stream;
}

struct BoundsRep
{
	BoundsRep() : type(0)
	{
		aabb.setEmpty();
	}

	physx::PxBounds3 aabb;
	physx::PxU32	 type;	// By default only reports if subtypes are the same, configurable.  Valid range {0...7}
};

struct BoundsInteractions
{
	BoundsInteractions() : bits(0x8040201008040201ULL) {}
	BoundsInteractions(bool setAll) : bits(setAll ? 0xFFFFFFFFFFFFFFFFULL : 0x0000000000000000ULL) {}

	bool	set(unsigned group1, unsigned group2, bool interacts)
	{
		if (group1 >= 8 || group2 >= 8)
		{
			return false;
		}
		const physx::PxU64 mask = (physx::PxU64)1 << ((group1 << 3) + group2) | (physx::PxU64)1 << ((group2 << 3) + group1);
		if (interacts)
		{
			bits |= mask;
		}
		else
		{
			bits &= ~mask;
		}
		return true;
	}

	physx::PxU64 bits;
};

enum Bounds3Axes
{
	Bounds3X =		1,
	Bounds3Y =		2,
	Bounds3Z =		4,

	Bounds3XY =		Bounds3X | Bounds3Y,
	Bounds3YZ =		Bounds3Y | Bounds3Z,
	Bounds3ZX =		Bounds3Z | Bounds3X,

	Bounds3XYZ =	Bounds3X | Bounds3Y | Bounds3Z
};

void boundsCalculateOverlaps(physx::Array<IntPair>& overlaps, Bounds3Axes axesToUse, const BoundsRep* bounds, physx::PxU32 boundsCount, physx::PxU32 boundsByteStride,
                             const BoundsInteractions& interactions = BoundsInteractions(), bool append = false);


/*
Descriptor for building a ConvexHull, below
*/
class ConvexHullDesc
{
public:
	const void*		vertices;
	physx::PxU32	numVertices;
	physx::PxU32	vertexStrideBytes;
	physx::PxU32*	indices;
	physx::PxU32	numIndices;
	physx::PxU8*	faceIndexCounts;
	physx::PxU32	numFaces;

	ConvexHullDesc() :
		vertices(NULL),
		numVertices(0),
		vertexStrideBytes(0),
		indices(NULL),
		numIndices(0),
		faceIndexCounts(NULL),
		numFaces(0)
	{
	}

	bool isValid() const
	{
		return
			vertices != NULL &&
			numVertices != 0 && 
			vertexStrideBytes != 0 &&
			indices != NULL &&
			numIndices != 0 &&
			faceIndexCounts != NULL &&
			numFaces != 0;
	}
};

/*
ConvexHull - precomputed (redundant) information about a convex hull: vertices, hull planes, etc.
*/
class ConvexHull
{
public:
	struct Separation
	{
		physx::PxPlane	plane;
		physx::PxF32	min0, max0, min1, max1;

		physx::PxF32 getDistance()
		{
			return PxMax(min0 - max1, min1 - max0);
		}
	};

	ConvexHull();
	ConvexHull(const ConvexHull& hull) : mParams(NULL), mOwnsParams(false)
	{
		*this = hull;
	}
	virtual ~ConvexHull();

	// If params == NULL, this will build (and own) its own internal parameters
	void			init(NxParameterized::Interface* params = NULL);

	ConvexHull&		operator = (const ConvexHull& hull)
	{
		mParams = hull.mParams;
		mOwnsParams = false;
		return *this;
	}

	// Only returns non-NULL value if this object owns its parameters.
	NxParameterized::Interface*	giveOwnersipOfNxParameters();

	// Releases parameters if it owns them
	void			term();

	void			buildFromDesc(const ConvexHullDesc& desc);
	void			buildFromPoints(const void* points, physx::PxU32 numPoints, physx::PxU32 pointStrideBytes);
	void			buildFromPlanes(const physx::PxPlane* planes, physx::PxU32 numPlanes, physx::PxF32 eps);
	void			buildFromConvexMesh(const ConvexMesh* mesh);
	void			buildFromAABB(const physx::PxBounds3& aabb);
	void			buildKDOP(const void* points, physx::PxU32 numPoints, physx::PxU32 pointStrideBytes, const physx::PxVec3* directions, physx::PxU32 numDirections);

	void			intersectPlaneSide(const physx::PxPlane& plane);
	void			intersectHull(const ConvexHull& hull);

	// If the distance between the hulls exceeds maxDistance, false is returned.
	// Otherwise, true is returned.  In this case, if 'separation' is not NULL, then separation plane
	// and projected extents are returned in *separation.
	static	bool	hullsInProximity(const ConvexHull& hull0, const physx::PxMat34Legacy& localToWorldRT0, const physx::PxVec3& scale0,
	                                 const ConvexHull& hull1, const physx::PxMat34Legacy& localToWorldRT1, const physx::PxVec3& scale1,
	                                 physx::PxF32 maxDistance, Separation* separation = NULL);

	// If the distance between this hull and the given sphere exceeds maxDistance, false is returned.
	// Otherwise, true is returned.  In this case, if 'separation' is not NULL, then separation plane
	// and projected extents are returned in *separation.  The '0' values will correspond to the hull,
	// and the '1' values to the sphere.
	bool			sphereInProximity(const physx::PxMat34Legacy& hullLocalToWorldRT, const physx::PxVec3& hullScale,
									  const physx::PxVec3& sphereWorldCenter, physx::PxF32 sphereRadius,
									  physx::PxF32 maxDistance, Separation* separation = NULL);

#if NX_SDK_VERSION_MAJOR == 2
	// localToWorldRT and scale apply to this.
	bool			intersects(const NxShape& shape, const physx::PxMat34Legacy& localToWorldRT, const physx::PxVec3& scale, physx::PxF32 padding) const;
#elif NX_SDK_VERSION_MAJOR == 3
	bool			intersects(const PxShape& shape, const physx::PxMat34Legacy& localToWorldRT, const physx::PxVec3& scale, physx::PxF32 padding) const;
#endif

	// worldRay.dir need not be normalized.  in & out times are relative to worldRay.dir length
	// N.B.  in & out are both input and output variables:
	// input:	in = minimum possible ray intersect time
	//			out = maximum possible ray intersect time
	// output:	in = time ray enters hull
	//			out = time ray exits hull
	bool			rayCast(physx::PxF32& in, physx::PxF32& out, const physx::PxVec3& orig, const physx::PxVec3& dir,
	                        const physx::PxMat34Legacy& localToWorldRT, const physx::PxVec3& scale, physx::PxVec3* normal = NULL) const;

	// in & out times are relative to worldDisp length
	// N.B.  in & out are both input and output variables:
	// input:	in = minimum possible ray intersect time
	//			out = maximum possible ray intersect time
	// output:	in = time ray enters hull
	//			out = time ray exits hull
	bool			obbSweep(physx::PxF32& in, physx::PxF32& out, const physx::PxVec3& worldBoxCenter, const physx::PxVec3& worldBoxExtents, const physx::PxVec3 worldBoxAxes[3],
	                         const physx::PxVec3& worldDisp, const physx::PxMat34Legacy& localToWorldRT, const physx::PxVec3& scale, physx::PxVec3* normal = NULL) const;

	// Returns the min and max dot product of the vertices with the given normal
	void			extent(physx::PxF32& min, physx::PxF32& max, const physx::PxVec3& normal) const;

	void			fill(physx::Array<physx::PxVec3>& outPoints, const physx::PxMat34Legacy& localToWorldRT, const physx::PxVec3& scale,
	                     physx::PxF32 spacing, physx::PxF32 jitter, physx::PxU32 maxPoints, bool adjustSpacing) const;

	void			setEmpty()
	{
		NxParameterized::Handle handle(*mParams);
		mParams->getParameterHandle("vertices", handle);
		mParams->resizeArray(handle, 0);
		mParams->getParameterHandle("uniquePlanes", handle);
		mParams->resizeArray(handle, 0);
		mParams->getParameterHandle("widths", handle);
		mParams->resizeArray(handle, 0);
		mParams->getParameterHandle("edges", handle);
		mParams->resizeArray(handle, 0);
		mParams->bounds.setEmpty();
		mParams->volume = 0.0f;
		mParams->uniqueEdgeDirectionCount = 0;
		mParams->planeCount = 0;
	}

	bool			isEmpty() const
	{
		PX_ASSERT(mParams->bounds.isEmpty() == (mParams->vertices.arraySizes[0] == 0));
		PX_ASSERT(mParams->bounds.isEmpty() == (mParams->uniquePlanes.arraySizes[0] == 0));
		PX_ASSERT(mParams->bounds.isEmpty() == (mParams->widths.arraySizes[0] == 0));
		PX_ASSERT(mParams->bounds.isEmpty() == (mParams->edges.arraySizes[0] == 0));
		PX_ASSERT(mParams->bounds.isEmpty() == (mParams->volume == 0.0f));
		return mParams->bounds.isEmpty();
	}

	physx::PxU32	getVertexCount() const
	{
		return (physx::PxU32)mParams->vertices.arraySizes[0];
	}
	const physx::PxVec3&	getVertex(physx::PxU32 index) const
	{
		return mParams->vertices.buf[index];
	}

	physx::PxU32	getPlaneCount() const
	{
		return mParams->planeCount;
	}
	physx::PxU32	getUniquePlaneNormalCount() const
	{
		return (physx::PxU32)mParams->uniquePlanes.arraySizes[0];
	}
	physx::PxPlane	getPlane(physx::PxU32 index) const
	{
		PX_ASSERT(index < getPlaneCount());
		if (index < (physx::PxU32)mParams->uniquePlanes.arraySizes[0])
		{
			return toPxPlane(mParams->uniquePlanes.buf[index]);
		}
		index -= mParams->uniquePlanes.arraySizes[0];
		physx::PxPlane plane = toPxPlane(mParams->uniquePlanes.buf[index]);
		plane.n = -plane.n;
		plane.d = -plane.d - mParams->widths.buf[index];
		return plane;
	}

	physx::PxU32	getWidthCount() const
	{
		return (physx::PxU32)mParams->widths.arraySizes[0];
	}
	physx::PxF32	getWidth(physx::PxU32 index) const
	{
		return mParams->widths.buf[index];
	}

	physx::PxU32	getEdgeCount() const
	{
		return (physx::PxU32)mParams->edges.arraySizes[0];
	}
	physx::PxU32	getEdgeEndpointIndex(physx::PxU32 edgeIndex, physx::PxU32 endpointIndex) const	// endpointIndex = 0 or 1
	{
		PX_ASSERT(edgeIndex < getEdgeCount());
		PX_ASSERT((endpointIndex & 1) == endpointIndex);
		endpointIndex &= 1;
		const physx::PxU32 edge = mParams->edges.buf[edgeIndex];
		return (endpointIndex & 1) ? (edge & 0x0000FFFF) : (edge >> 16);
	}
	physx::PxU32	getEdgeAdjacentFaceIndex(physx::PxU32 edgeIndex, physx::PxU32 adjacencyIndex) const	// adjacencyIndex = 0 or 1
	{
		PX_ASSERT(edgeIndex < getEdgeCount());
		PX_ASSERT((adjacencyIndex & 1) == adjacencyIndex);
		adjacencyIndex &= 1;
		const physx::PxU32 adj = mParams->adjacentFaces.buf[edgeIndex];
		return (adjacencyIndex & 1) ? (adj & 0x0000FFFF) : (adj >> 16);
	}
	physx::PxU32	getUniqueEdgeDirectionCount() const
	{
		return mParams->uniqueEdgeDirectionCount;
	}
	physx::PxVec3	getEdgeDirection(physx::PxU32 index) const
	{
		PX_ASSERT(index < getEdgeCount());
		physx::PxU32 edge = mParams->edges.buf[index];
		return mParams->vertices.buf[edge & 0xFFFF] - mParams->vertices.buf[edge >> 16];
	}

	const physx::PxBounds3&	getBounds() const
	{
		return mParams->bounds;
	}
	physx::PxF32			getVolume() const
	{
		return (physx::PxF32)mParams->volume;
	}

	// transform may include an arbitrary 3x3 block and a translation
	void					applyTransformation(const physx::PxMat44& tm)	
	{
		PX_ASSERT(mParams);

		const physx::PxReal det3 = PxMat33(tm.getBasis(0), tm.getBasis(1), tm.getBasis(2)).getDeterminant();
		PX_ASSERT(det3 > 0.0f); // mirroring or degeneracy won't work well here

		// planes and slab widths
		const NiCof44 cof(tm);
		const physx::PxU32 numPlanes = (physx::PxU32)mParams->uniquePlanes.arraySizes[0];
		ConvexHullParametersNS::Plane_Type* planes = mParams->uniquePlanes.buf;
		PX_ASSERT(planes);
		PX_ASSERT(numPlanes == (physx::PxU32)mParams->widths.arraySizes[0]);
		physx::PxF32* widths = mParams->widths.buf;
		PX_ASSERT(widths);
		for (PxU32 i = 0; i < numPlanes; i++)
		{
			physx::PxPlane src(planes[i].normal, planes[i].d);
			physx::PxPlane dst;
			cof.transform(src, dst);
			planes[i].normal = dst.n;
			planes[i].d = dst.d;
			const physx::PxReal n2 = dst.n.magnitudeSquared();
			if (n2 > 0.0f)
			{
				const physx::PxReal recipN = physx::PxRecipSqrt(n2);
				planes[i].normal *= recipN;
				planes[i].d *= recipN;
				widths[i] *= det3*recipN;
			}
		}

		// vertices
		const physx::PxU32 numVertices = (physx::PxU32)mParams->vertices.arraySizes[0];
		physx::PxVec3* vertices = mParams->vertices.buf;
		PX_ASSERT(vertices);

		mParams->bounds.setEmpty();
		for (PxU32 i = 0; i < numVertices; i++)
		{
			vertices[i] = tm.transform(vertices[i]);
			mParams->bounds.include(vertices[i]);
		}

		// volume
		mParams->volume *= det3;
	}

	// Special case - transformation must be a pure rotation plus translation, and we only allow a positive, uniform scale
	// Note, we could implement this with applyTransformation(const physx::PxMat44& tm), above, but we will keep this
	// old implementation to ensure that behavior doesn't change
	void					applyTransformation(const physx::PxMat44& transformation, physx::PxF32 scale)
	{
		PX_ASSERT(mParams);
		PX_ASSERT(scale > 0.0f); // negative scale won't work well here

		// planes
		const physx::PxU32 numPlanes = (physx::PxU32)mParams->uniquePlanes.arraySizes[0];
		ConvexHullParametersNS::Plane_Type* planes = mParams->uniquePlanes.buf;
		PX_ASSERT(planes);
		for (PxU32 i = 0; i < numPlanes; i++)
		{
			planes[i].normal = transformation.rotate(planes[i].normal);
			planes[i].d *= scale;
		}

		// slab widths
		const physx::PxU32 numWidths = (physx::PxU32)mParams->widths.arraySizes[0];
		physx::PxF32* widths = mParams->widths.buf;
		PX_ASSERT(widths);
		for (PxU32 i = 0; i < numWidths; i++)
		{
			widths[i] *= scale;
		}

		// vertices
		const physx::PxU32 numVertices = (physx::PxU32)mParams->vertices.arraySizes[0];
		physx::PxVec3* vertices = mParams->vertices.buf;
		PX_ASSERT(vertices);

		mParams->bounds.setEmpty();
		for (PxU32 i = 0; i < numVertices; i++)
		{
			vertices[i] = transformation.transform(vertices[i]) * scale;	// Works since scale is uniform
			mParams->bounds.include(vertices[i]);
		}

		// volume
		mParams->volume *= scale*scale*scale;
	}

	// Returns the number of vertices and faces of the cooked mesh.  If inflated = false,
	// these should be the same as the values returned by getVertexCount() and getPlaneCount().
	// However, the numerical properties of the cooker could result in different values.  If inflated = true,
	// then sharp edges will be beveled by the cooker, resulting in more vertices and faces.
	// Note: the number of edges E may be calculated from the number of vertices V and faces F using E = V + F - 2.
	// Return value = size in bytes of the cooked convex mesh
	physx::PxU32 calculateCookedSizes(physx::PxU32& vertexCount, physx::PxU32& faceCount, bool inflated) const;

	// Removes vertices from the hull until the bounds given in the function's parameters are met.
	// If inflated = true, then the maximum counts given are compared with the cooked hull, which may have higher counts due to beveling.
	// Note: a value of zero indicates no limit, effectively infinite.
	// Return value: true if successful, i.e. the limits were met.  False otherwise.
	bool reduceHull(physx::PxU32 maxVertexCount, physx::PxU32 maxEdgeCount, physx::PxU32 maxFaceCount, bool inflated);

	// Replaces vertices with cooked, un-inflated vertices, if the latter set is smaller.  Returns true if the number of vertices is reduced.
	bool reduceByCooking();

	// Utility function
	static bool createKDOPDirections(physx::Array<physx::PxVec3>& directions, NxConvexHullMethod::Enum method);

//		DeclareArray(physx::PxVec3)	vertices;
//		DeclareArray(physx::PxPlane)	uniquePlanes;	// These are the unique face directions.  If there is an opposite face, the corresponding widths[i] will give its distance
//		physx::Array<physx::PxF32>	widths;			// Same size as uniquePlanes.  Gives width of hull in uniquePlane direction
//		physx::Array<physx::PxU32>	edges;			// Vertex indices stored in high/low words.  The first uniqueEdgeDirectionCount elements give the unique directions.
//		physx::PxBounds3			bounds;
//		physx::PxF32				volume;
//		physx::PxU32				uniqueEdgeDirectionCount;
//		physx::PxU32				planeCount;		// Total number of faces.  Greater than or equal to size of uniquePlanes.

	ConvexHullParameters*		mParams;
	bool						mOwnsParams;
};


/*
ConvexMeshBuilder - creates triangles for a convex hull defined by a set of planes. Copied from physx samples (RenderClothActor)
*/
struct ConvexMeshBuilder
{
	ConvexMeshBuilder(const PxVec4* planes)
		: mPlanes(planes)
	{}

	void operator()(PxU32 mask, float scale=1.0f);

	const PxVec4* mPlanes;
	shdfnd::Array<PxVec3> mVertices;
	shdfnd::Array<PxU16> mIndices;
};


// Fast implementation for sse
PX_INLINE physx::PxF32	NxRecipSqrt(physx::PxF32 x)
{
#if defined( APEX_SUPPORT_SSE )
	const float three = 3.0f;
	const float oneHalf = 0.5f;
	float y;
	_asm
	{
		movss	xmm2, three;
		rsqrtss	xmm0, x
		movss	xmm1, xmm0
		mulss	xmm1, oneHalf
		mulss	xmm0, xmm0
		mulss	xmm0, x
		subss	xmm2, xmm0
		mulss	xmm1, xmm2
		movss	y, xmm1
	}
	return y;
#else
	return 1.0f / sqrtf(x);
#endif
}

/*
	Array find utility
 */

// If t is found in array, index is set to the array element and the function returns true
// If t is not found in the array, index is not modified and the function returns false
template<class T>
bool arrayFind(physx::PxU32& index, const T& t, const physx::Array<T>& array)
{
	const physx::PxU32 size = array.size();
	for (physx::PxU32 i = 0; i < size; ++i)
	{
		if (array[i] == t)
		{
			index = i;
			return true;
		}
	}
	return false;
}

#include "PsShare.h"
#ifdef PX_X64
#pragma warning(push)
#pragma warning(disable: 4324) // 'NxIndexBank<IndexType>' : structure was padded due to __declspec(align())
#endif

/*
	Index bank - double-sided free list for O(1) borrow/return of unique IDs

	Type IndexType should be an unsigned integer type or something that can be cast to and from
	an integer
 */
template <class IndexType>
class NxIndexBank
{
	public:
	NxIndexBank<IndexType>(physx::PxU32 capacity = 0) : indexCount(0), capacityLocked(false)
	{
		maxCapacity = calculateMaxCapacity();
		reserve_internal(capacity);
	}

	// Copy constructor
	NxIndexBank<IndexType>(const NxIndexBank<IndexType>& other)
	{
		*this = other;
	}

	virtual				~NxIndexBank<IndexType>() {}

	// Assignment operator
	NxIndexBank<IndexType>& operator = (const NxIndexBank<IndexType>& other)
	{
		indices = other.indices;
		ranks = other.ranks;
		maxCapacity = other.maxCapacity;
		indexCount = other.indexCount;
		capacityLocked = other.capacityLocked;
		return *this;
	}

	void				setIndicesAndRanks(physx::PxU16* indicesIn, physx::PxU16* ranksIn, physx::PxU32 capacityIn, physx::PxU32 usedCountIn)
	{
		indexCount = usedCountIn;
		reserve_internal(capacityIn);
		for (PxU32 i = 0; i < capacityIn; ++i)
		{
			indices[i] = indicesIn[i];
			ranks[i]   = ranksIn[i];
		}
	}

	void				clear(physx::PxU32 capacity = 0, bool used = false)
	{
		capacityLocked = false;
		indices.reset();
		ranks.reset();
		reserve_internal(capacity);
		if (used)
		{
			indexCount = capacity;
			indices.resize(capacity);
			for (IndexType i = (IndexType)0; i < (IndexType)capacity; ++i)
			{
				indices[i] = i;
			}
		}
		else
		{
			indexCount = 0;
		}
	}

	// Equivalent to calling freeLastUsed() until the used list is empty.
	void				clearFast()
	{
		indexCount = 0;
	}

	// This is the reserve size.  The bank can only grow, due to shuffling of indices
	virtual void		reserve(physx::PxU32 capacity)
	{
		reserve_internal(capacity);
	}

	// If lock = true, keeps bank from automatically resizing
	void				lockCapacity(bool lock)
	{
		capacityLocked = lock;
	}

	bool				isCapacityLocked() const
	{
		return capacityLocked;
	}

	void				setMaxCapacity(physx::PxU32 inMaxCapacity)
	{
		// Cannot drop below current capacity, nor above max set by data types
		maxCapacity = physx::PxClamp(inMaxCapacity, capacity(), calculateMaxCapacity());
	}

	physx::PxU32		capacity() const
	{
		return indices.size();
	}
	physx::PxU32		usedCount() const
	{
		return indexCount;
	}
	physx::PxU32		freeCount() const
	{
		return capacity() - usedCount();
	}

	// valid from [0] to [size()-1]
	const IndexType* 	usedIndices() const
	{
		return indices.begin();
	}

	// valid from [0] to [free()-1]
	const IndexType* 	freeIndices() const
	{
		return indices.begin() + usedCount();
	}

	bool				isValid(IndexType index) const
	{
		return index < (IndexType)capacity();
	}
	bool				isUsed(IndexType index) const
	{
		return isValid(index) && (ranks[index] < (IndexType)usedCount());
	}
	bool				isFree(IndexType index) const
	{
		return isValid(index) && !isUsed();
	}

	IndexType			getRank(IndexType index) const
	{
		return ranks[index];
	}

	// Gets the next available index, if any
	bool				useNextFree(IndexType& index)
	{
		if (freeCount() == 0)
		{
			if (capacityLocked)
			{
				return false;
			}
			if (capacity() >= maxCapacity)
			{
				return false;
			}
			reserve(physx::PxClamp(capacity() * 2, (physx::PxU32)1, maxCapacity));
			PX_ASSERT(freeCount() > 0);
		}
		index = indices[indexCount++];
		return true;
	}

	// Frees the last used index, if any
	bool				freeLastUsed(IndexType& index)
	{
		if (usedCount() == 0)
		{
			return false;
		}
		index = indices[--indexCount];
		return true;
	}

	// Requests a particular index.  If that index is available, it is borrowed and the function
	// returns true.  Otherwise nothing happens and the function returns false.
	bool				use(IndexType index)
	{
		if (!indexIsValidForUse(index))
		{
			return false;
		}
		IndexType oldRank;
		placeIndexAtRank(index, (IndexType)indexCount++, oldRank);
		return true;
	}

	bool				free(IndexType index)
	{
		if (!indexIsValidForFreeing(index))
		{
			return false;
		}
		IndexType oldRank;
		placeIndexAtRank(index, (IndexType)--indexCount, oldRank);
		return true;
	}

	bool				useAndReturnRanks(IndexType index, IndexType& newRank, IndexType& oldRank)
	{
		if (!indexIsValidForUse(index))
		{
			return false;
		}
		newRank = (IndexType)indexCount++;
		placeIndexAtRank(index, newRank, oldRank);
		return true;
	}

	bool				freeAndReturnRanks(IndexType index, IndexType& newRank, IndexType& oldRank)
	{
		if (!indexIsValidForFreeing(index))
		{
			return false;
		}
		newRank = (IndexType)--indexCount;
		placeIndexAtRank(index, newRank, oldRank);
		return true;
	}

	protected:

	bool				indexIsValidForUse(IndexType index)
	{
		if (!isValid(index))
		{
			if (capacityLocked)
			{
				return false;
			}
			if (capacity() >= maxCapacity)
			{
				return false;
			}
			reserve(physx::PxClamp(2*(physx::PxU32)index, (physx::PxU32)1, maxCapacity));
			PX_ASSERT(isValid(index));
		}
		return !isUsed(index);
	}

	bool				indexIsValidForFreeing(IndexType index)
	{
		if (!isValid(index))
		{
			// Invalid index
			return false;
		}
		return isUsed(index);
	}

	// This is the reserve size.  The bank can only grow, due to shuffling of indices
	void				reserve_internal(physx::PxU32 capacity)
	{
		capacity = physx::PxMin(capacity, maxCapacity);
		const physx::PxU32 oldCapacity = indices.size();
		if (capacity > oldCapacity)
		{
			indices.resize(capacity);
			ranks.resize(capacity);
			for (IndexType i = (IndexType)oldCapacity; i < (IndexType)capacity; ++i)
			{
				indices[i] = i;
				ranks[i] = i;
			}
		}
	}

	private:

	void				placeIndexAtRank(IndexType index, IndexType newRank, IndexType& oldRank)	// returns old rank
	{
		const IndexType replacementIndex = indices[newRank];
		oldRank = ranks[index];
		indices[oldRank] = replacementIndex;
		indices[newRank] = index;
		ranks[replacementIndex] = oldRank;
		ranks[index] = newRank;
	}

	physx::PxU32				calculateMaxCapacity()
	{
#pragma warning(push)
#pragma warning(disable: 4127) // conditional expression is constant
		if (sizeof(IndexType) >= sizeof(physx::PxU32))
		{
			return 0xFFFFFFFF;	// Limited by data type we use to report capacity
		}
		else
		{
			return (1u << (8 * physx::PxMin((physx::PxU32)sizeof(IndexType), 3u))) - 1;	// Limited by data type we use for indices
		}
#pragma warning(pop)
	}

	protected:

	Array<IndexType>		indices;
	Array<IndexType>		ranks;
	PxU32					maxCapacity;
	PxU32					indexCount;
	bool					capacityLocked;
};

#ifdef PX_X64
#pragma warning(pop)
#endif

/*
	Bank - Index bank of type IndexType with an associated object array of type T
 */
template <class T, class IndexType>
class NxBank : public NxIndexBank<IndexType>
{
	public:
	NxBank<T, IndexType>(physx::PxU32 capacity = 0) : NxIndexBank<IndexType>(capacity)
	{
		objects = (T*)PX_ALLOC(NxIndexBank<IndexType>::indices.size() * sizeof(T), PX_DEBUG_EXP("NxBank"));
		if (objects != NULL)
		{
			PX_ASSERT(memset(objects, 0, NxIndexBank<IndexType>::indices.size() * sizeof(T)));
		}
	}
	NxBank<T, IndexType>(const NxBank<T, IndexType>& bank) : objects(NULL)
	{
		*this = bank;
	}

	~NxBank<T, IndexType>()
	{
		clear();
	}

	NxBank<T, IndexType>&	operator = (const NxBank<T, IndexType>& bank)
	{
		if (&bank == this)
		{
			return *this;
		}

		this->clear();

		this->indices = bank.indices;
		this->ranks = bank.ranks;
		this->maxCapacity = bank.maxCapacity;
		this->indexCount = bank.indexCount;
		this->capacityLocked = bank.capacityLocked;

		if (this->indices.size())
		{
			objects = (T*)PX_ALLOC(NxIndexBank<IndexType>::indices.size() * sizeof(T), PX_DEBUG_EXP("NxBank"));
			PX_ASSERT(memset(objects, 0, NxIndexBank<IndexType>::indices.size() * sizeof(T)));
			for (physx::PxU32 i = 0; i < this->indexCount; ++i)
			{
				physx::PxU32 index = this->indices[i];
				new(objects + index) T();
				objects[index] = bank.objects[index];
			}
		}
		return *this;
	}

	// This is the reserve size.  The bank can only grow, due to shuffling of indices
	virtual	void	reserve(physx::PxU32 capacity)
	{
		const physx::PxU32 oldSize = NxIndexBank<IndexType>::indices.size();
		NxIndexBank<IndexType>::reserve_internal(capacity);
		if (NxIndexBank<IndexType>::indices.size() > oldSize)
		{
			T* nb = (T*)PX_ALLOC(NxIndexBank<IndexType>::indices.size() * sizeof(T), PX_DEBUG_EXP("NxBank"));
			if (nb)
			{
				PX_ASSERT(memset(nb, 0, NxIndexBank<IndexType>::indices.size() * sizeof(T)));

				const IndexType* usedIndices = NxIndexBank<IndexType>::usedIndices();
				physx::PxU32 numIndices = NxIndexBank<IndexType>::usedCount();

				// this copy needs to be correct for nonPOD type T's
				for (physx::PxI32 i = (physx::PxI32)numIndices - 1; i >= 0; i--)
				{
					IndexType index = usedIndices[i];
					new(nb + index) T(objects[index]);
					objects[index].~T();
				}
				//memcpy( nb, objects, NxIndexBank<IndexType>::indices.size()*sizeof(T) );
			}
			PX_FREE(objects);
			objects = nb;
		}
	}

	// Indirect array accessors: rank in [0,usedCount()-1] returns all "used" indexed objects
	const T& 		getUsed(IndexType rank) const
	{
		return objects[ NxIndexBank<IndexType>::indices[rank] ];
	}
	T& 				getUsed(IndexType rank)
	{
		return objects[ NxIndexBank<IndexType>::indices[rank] ];
	}

	// Direct array accessors
	const T& 		direct(IndexType index) const
	{
		return objects[index];
	}
	T& 				direct(IndexType index)
	{
		return objects[index];
	}

	// Wrappers for base class, which call appropriate constructors and destructors of objects
	bool			useNextFree(IndexType& index)
	{
		if (NxIndexBank<IndexType>::useNextFree(index))
		{
			new(objects + index) T();
			return true;
		}
		return false;
	}

	bool			freeLastUsed(IndexType& index)
	{
		if (NxIndexBank<IndexType>::freeLastUsed(index))
		{
			objects[index].~T();
			return true;
		}
		return false;
	}

	bool			use(IndexType index)
	{
		if (NxIndexBank<IndexType>::use(index))
		{
			new(objects + index) T();
			return true;
		}
		return false;
	}

	bool			free(IndexType index)
	{
		if (NxIndexBank<IndexType>::free(index))
		{
			objects[index].~T();
			return true;
		}
		return false;
	}

	bool			useAndReturnRanks(IndexType index, IndexType& newRank, IndexType& oldRank)
	{
		if (NxIndexBank<IndexType>::useAndReturnRanks(index,  newRank,  oldRank))
		{
			new(objects + index) T();
			return true;
		}
		return false;
	}

	bool			freeAndReturnRanks(IndexType index, IndexType& newRank, IndexType& oldRank)
	{
		if (NxIndexBank<IndexType>::freeAndReturnRanks(index,  newRank,  oldRank))
		{
			objects[index].~T();
			return true;
		}
		return false;
	}

	// Erases all object, index, and rank arrays (complete deallocation)
	void			clear()
	{
		const IndexType* usedIndices = NxIndexBank<IndexType>::usedIndices();
		physx::PxU32 numIndices = NxIndexBank<IndexType>::usedCount();

		for (physx::PxI32 i = (physx::PxI32)numIndices - 1; i >= 0; i--)
		{
			bool test = free(usedIndices[i]);
			PX_UNUSED(test);
			PX_ASSERT(test);
		}

		NxIndexBank<IndexType>::clear();
		PX_FREE(objects);
		objects = NULL;
	}

	// Re-arranges objects internally into rank-order, afterwards rank = index
	void			clean()
	{
		for (IndexType i = 0; i < NxIndexBank<IndexType>::capacity(); ++i)
		{
			const IndexType index = NxIndexBank<IndexType>::indices[i];
			if (index != i)
			{
				physx::swap(objects[i], objects[index]);
				const IndexType displacedRank = NxIndexBank<IndexType>::ranks[i];
				NxIndexBank<IndexType>::indices[i] = i;
				NxIndexBank<IndexType>::ranks[i] = i;
				NxIndexBank<IndexType>::indices[displacedRank] = index;
				NxIndexBank<IndexType>::ranks[index] = displacedRank;
			}
		}
	}

	protected:
	T*	objects;
};


/*
	Ring buffer
*/
template <class T>
class NxRingBuffer
{
	public:
	NxRingBuffer() : frontIndex(0), backIndex(0xFFFFFFFF), usedSize(0), bufferSize(0), buffer(NULL) {}
	~NxRingBuffer()
	{
		erase();
	}

	physx::PxU32	size()	const
	{
		return usedSize;
	}

	T&		operator [](physx::PxU32 i)
	{
		PX_ASSERT(i < usedSize);
		i += frontIndex;
		return buffer[ i < bufferSize ? i : i - bufferSize ];
	}

	const T&	operator [](physx::PxU32 i) const
	{
		return (const T&)(const_cast<NxRingBuffer<T>*>(this)->operator[](i));
	}

	T&		back()	const
	{
		return buffer[backIndex];
	}
	T&		front()	const
	{
		return buffer[frontIndex];
	}

	T&		pushBack()
	{
		if (bufferSize == usedSize)
		{
			reserve(2 * (bufferSize + 1));
		}
		++usedSize;
		if (++backIndex == bufferSize)
		{
			backIndex = 0;
		}
		T& back = buffer[backIndex];
		PX_PLACEMENT_NEW(&back, T)();
		return back;
	}

	void	popBack()
	{
		PX_ASSERT(size() != 0);
		if (size() == 0)
		{
			return;
		}
		buffer[backIndex].~T();
		--usedSize;
		if (backIndex-- == 0)
		{
			backIndex += bufferSize;
		}
	}

	T&		pushFront()
	{
		if (bufferSize == usedSize)
		{
			reserve(2 * (bufferSize + 1));
		}
		++usedSize;
		if (frontIndex-- == 0)
		{
			frontIndex += bufferSize;
		}
		T& front = buffer[frontIndex];
		PX_PLACEMENT_NEW(&front, T)();
		return front;
	}

	void	popFront()
	{
		PX_ASSERT(size() != 0);
		if (size() == 0)
		{
			return;
		}
		buffer[frontIndex].~T();
		--usedSize;
		if (++frontIndex == bufferSize)
		{
			frontIndex = 0;
		}
	}

	void	clear()
	{
		while (size() != 0)
		{
			popBack();
		}
		frontIndex = 0;
		backIndex = 0xFFFFFFFF;
	}

	void	erase()
	{
		clear();
		if (buffer != NULL)
		{
			PX_FREE(buffer);
			buffer = NULL;
		}
		bufferSize = 0;
	}

	void	reserve(physx::PxU32 newBufferSize)
	{
		if (newBufferSize <= bufferSize)
		{
			return;
		}
		T* newBuffer = (T*)PX_ALLOC(newBufferSize * sizeof(T), PX_DEBUG_EXP("NxRingBuffer"));
		const physx::PxU32 lastIndex = frontIndex + usedSize;
		if (lastIndex <= bufferSize)
		{
			for (physx::PxU32 i = 0; i < usedSize; i++)
			{
				PX_PLACEMENT_NEW(newBuffer + i, T)(buffer[i]);
				buffer[i].~T();
			}
			//memcpy( newBuffer, buffer+frontIndex, usedSize*sizeof( T ) );
		}
		else
		{
			for (physx::PxU32 i = 0; i < (bufferSize - frontIndex); i++)
			{
				PX_PLACEMENT_NEW(newBuffer + i, T)(buffer[i + frontIndex]);
				buffer[i + frontIndex].~T();
			}
			//memcpy( newBuffer, buffer+frontIndex, (bufferSize-frontIndex)*sizeof( T ) );

			for (physx::PxU32 i = 0; i < (lastIndex - bufferSize); i++)
			{
				PX_PLACEMENT_NEW(newBuffer + i + (bufferSize - frontIndex), T)(buffer[i]);
				buffer[i].~T();
			}
			//memcpy( newBuffer + (bufferSize-frontIndex), buffer, (lastIndex-bufferSize)*sizeof( T ) );
		}
		bufferSize = newBufferSize;
		frontIndex = 0;
		backIndex = frontIndex + usedSize - 1;
		if (buffer)
		{
			PX_FREE(buffer);
		}
		buffer = newBuffer;
	}

	class It
	{
		public:
		It(const NxRingBuffer<T>& buffer) :
		m_bufferStart(buffer.buffer), m_bufferStop(buffer.buffer + buffer.bufferSize),
		m_current(buffer.usedSize > 0 ? buffer.buffer + buffer.frontIndex : NULL), m_remaining(buffer.usedSize) {}

		operator T* ()	const
		{
			return m_current;
		}
		T*		operator ++ ()
		{
			inc();
			return m_current;
		}
		T*		operator ++ (int)
		{
			T* prev = m_current;
			inc();
			return prev;
		}

		private:
		void	inc()
		{
			if (m_remaining > 1)
			{
				--m_remaining;
				if (++m_current == m_bufferStop)
				{
					m_current = m_bufferStart;
				}
			}
			else
			{
				m_remaining = 0;
				m_current = NULL;
			}
		}

		T*		m_bufferStart;
		T*		m_bufferStop;
		T*		m_current;
		physx::PxU32	m_remaining;
	};

	friend class It;

	protected:
	physx::PxU32	frontIndex;
	physx::PxU32	backIndex;
	physx::PxU32	usedSize;
	physx::PxU32	bufferSize;
	T*		buffer;
};


template<class T>
class Pool
{
	enum { DefaultBlockSizeInBytes = 1024 };	// This must be positive

	public:
	Pool(physx::PxU32 objectsPerBlock = 0) : m_head(NULL), m_inUse(0)
	{
		PX_ASSERT(sizeof(T) >= sizeof(void*));
		setBlockSize(objectsPerBlock);
	}

	~Pool()
	{
		empty();
	}

	void	setBlockSize(physx::PxU32 objectsPerBlock)
	{
		m_objectsPerBlock = objectsPerBlock > 0 ? objectsPerBlock : ((physx::PxU32)DefaultBlockSizeInBytes + sizeof(T) - 1) / sizeof(T);
	}

	/* Gives a single object, allocating if necessary */
	T*		borrow()
	{
		if (m_head == NULL)
		{
			allocateBlock();
		}
		T* ptr = (T*)m_head;
		m_head = *(void**)m_head;
		new(ptr) T();
		++m_inUse;
		return ptr;
	}

	/* Return a single object */
	void	replace(T* ptr)
	{
		if (ptr != NULL)
		{
			ptr->~T();
			*(void**)ptr = m_head;
			m_head = (void*)ptr;
			--m_inUse;
		}
	}

	void	allocateBlock()
	{
		T* block = (T*)PX_ALLOC(sizeof(T) * m_objectsPerBlock, PX_DEBUG_EXP("ApexSharedUtils::Pool"));
		m_blocks.pushBack(block);
		for (T* ptr = block + m_objectsPerBlock; ptr-- != block;)
		{
			*(void**)ptr = m_head;
			m_head = (void*)ptr;
		}
	}

	physx::PxI32	empty()
	{
		while (m_blocks.size())
		{
			PX_FREE(m_blocks.back());
			m_blocks.popBack();
		}
		m_blocks.reset();
		m_head = NULL;
		const physx::PxI32 inUse = m_inUse;
		m_inUse = 0;
		return inUse;
	}

	protected:

	void*			m_head;
	physx::PxU32	m_objectsPerBlock;
	physx::Array<T*>m_blocks;
	physx::PxI32	m_inUse;
};


// Progress listener implementation for hierarchical progress reporting
class HierarchicalProgressListener : public IProgressListener
{
	public:
	HierarchicalProgressListener(int totalWork, IProgressListener* parent) :
	m_work(0), m_subtaskWork(1), m_totalWork(physx::PxMax(totalWork, 1)), m_taskName(NULL), m_parent(parent) {}

	void	setSubtaskWork(int subtaskWork, const char* taskName = NULL)
	{
		if (subtaskWork < 0)
		{
			subtaskWork = m_totalWork - m_work;
		}

		m_subtaskWork = subtaskWork;
		PX_ASSERT(m_work + m_subtaskWork <= m_totalWork);
		m_taskName = taskName;
		setProgress(0, m_taskName);
	}

	void	completeSubtask()
	{
		setProgress(100, m_taskName);
		m_work += m_subtaskWork;
	}

	void	setProgress(int progress, const char* taskName = NULL)
	{
		PX_ASSERT(progress >= 0);
		PX_ASSERT(progress <= 100);

		if (taskName == NULL)
		{
			taskName = m_taskName;
		}

		if (m_parent != NULL)
		{
			const int parentProgress = m_totalWork > 0 ? (m_work * 100 + m_subtaskWork * progress) / m_totalWork : 100;
			m_parent->setProgress(physx::PxClamp(parentProgress, 0, 100), taskName);
		}
	}

	protected:
	int m_work;
	int m_subtaskWork;
	int m_totalWork;
	const char* m_taskName;
	IProgressListener* m_parent;
};

void createIndexStartLookup(physx::Array<physx::PxU32>& lookup, physx::PxI32 indexBase, physx::PxU32 indexRange, physx::PxI32* indexSource, physx::PxU32 indexCount, physx::PxU32 indexByteStride);

void findIslands(physx::Array< physx::Array<physx::PxU32> >& islands, const physx::Array<IntPair>& overlaps, physx::PxU32 indexRange);

// Neighbor-finding utility class
class NeighborLookup
{
public:
	void				setBounds(const BoundsRep* bounds, physx::PxU32 boundsCount, physx::PxU32 boundsByteStride);

	physx::PxU32		getNeighborCount(const physx::PxU32 index) const;
	const physx::PxU32*	getNeighbors(const physx::PxU32 index) const;

protected:
	physx::Array<physx::PxU32>	m_neighbors;
	physx::Array<physx::PxU32>	m_firstNeighbor;
};


// TriangleFrame - calculates interpolation data for triangle quantities
class TriangleFrame
{
	public:

	enum VertexField
	{
//		Position_x,	Position_y, Position_z,	// Not interpolating positions
		Normal_x, Normal_y, Normal_z,
		Tangent_x, Tangent_y, Tangent_z,
		Binormal_x, Binormal_y, Binormal_z,
		UV0_u, UV0_v, UV1_u, UV1_v, UV2_u, UV2_v, UV3_u, UV3_v,
		Color_r, Color_g, Color_b, Color_a,

		VertexFieldCount
	};

	TriangleFrame() : m_fieldMask(0)																							{}
	TriangleFrame(const NxExplicitRenderTriangle& tri, physx::PxU64 fieldMask = 0xFFFFFFFFFFFFFFFFULL)
	{
		setFromTriangle(tri, fieldMask);
	}
	TriangleFrame(const physx::PxMat34Legacy& tm, const physx::PxVec2& uvScale, const physx::PxVec2& uvOffset, physx::PxU64 fieldMask = 0xFFFFFFFFFFFFFFFFULL)
	{
		setFlat(tm, uvScale, uvOffset, fieldMask);
	}

	PX_INLINE void	setFromTriangle(const NxExplicitRenderTriangle& tri, physx::PxU64 fieldMask = 0xFFFFFFFFFFFFFFFFULL);
	PX_INLINE void	setFlat(const physx::PxMat34Legacy& tm, const physx::PxVec2& uvScale, const physx::PxVec2& uvOffset, physx::PxU64 fieldMask = 0xFFFFFFFFFFFFFFFFULL);

	PX_INLINE void	interpolateVertexData(NxVertex& vertex) const;

	private:

	static size_t	s_offsets[VertexFieldCount];
	physx::PxPlane	m_frames[VertexFieldCount];
	physx::PxU64	m_fieldMask;

	friend class TriangleFrameBuilder;
};

PX_INLINE void
TriangleFrame::setFromTriangle(const NxExplicitRenderTriangle& tri, physx::PxU64 fieldMask)
{
	m_fieldMask = fieldMask;

	physx::PxVec3 p0, p1, p2;
	p0 = tri.vertices[0].position;
	p1 = tri.vertices[1].position;
	p2 = tri.vertices[2].position;
	const physx::PxVec3 p1xp2 = p1.cross(p2);
	const physx::PxVec3 p2xp0 = p2.cross(p0);
	const physx::PxVec3 p0xp1 = p0.cross(p1);
	const physx::PxVec3 n = p1xp2 + p2xp0 + p0xp1;
	const physx::PxF32 n2 = n.dot(n);
	if (n2 < PX_EPS_F32 * PX_EPS_F32)
	{
		for (physx::PxU32 i = 0; fieldMask != 0 && i < VertexFieldCount; ++i, (fieldMask >>= 1))
		{
			if (fieldMask & 1)
			{
				m_frames[i] = physx::PxPlane(0, 0, 0, 0);
			}
		}
		return;
	}

	// Calculate inverse 4x4 matrix (only need first three columns):
	const physx::PxVec3 nP = n / n2;	// determinant is -n2
	const physx::PxVec3 Q0(nP.z * (p1.y - p2.y) - nP.y * (p1.z - p2.z), nP.z * (p2.y - p0.y) - nP.y * (p2.z - p0.z), nP.z * (p0.y - p1.y) - nP.y * (p0.z - p1.z));
	const physx::PxVec3 Q1(nP.x * (p1.z - p2.z) - nP.z * (p1.x - p2.x), nP.x * (p2.z - p0.z) - nP.z * (p2.x - p0.x), nP.x * (p0.z - p1.z) - nP.z * (p0.x - p1.x));
	const physx::PxVec3 Q2(nP.y * (p1.x - p2.x) - nP.x * (p1.y - p2.y), nP.y * (p2.x - p0.x) - nP.x * (p2.y - p0.y), nP.y * (p0.x - p1.x) - nP.x * (p0.y - p1.y));
	const physx::PxVec3 r(nP.dot(p1xp2), nP.dot(p2xp0), nP.dot(p0xp1));

	for (physx::PxU32 i = 0; fieldMask != 0 && i < VertexFieldCount; ++i, (fieldMask >>= 1))
	{
		if (fieldMask & 1)
		{
			const size_t offset = s_offsets[i];
			const physx::PxVec3 vi(*(physx::PxF32*)(((physx::PxU8*)&tri.vertices[0]) + offset), *(physx::PxF32*)(((physx::PxU8*)&tri.vertices[1]) + offset), *(physx::PxF32*)(((physx::PxU8*)&tri.vertices[2]) + offset));
			m_frames[i] = physx::PxPlane(Q0.dot(vi), Q1.dot(vi), Q2.dot(vi), r.dot(vi));
		}
	}
}

PX_INLINE void
TriangleFrame::setFlat(const physx::PxMat34Legacy& tm, const physx::PxVec2& uvScale, const physx::PxVec2& uvOffset, physx::PxU64 fieldMask)
{
	m_fieldMask = fieldMask;

	// Local z ~ normal = tangents[2], x ~ u and tangent = tangents[0], y ~ v and binormal = tangents[1]
	if ((fieldMask >> Normal_x) & 1)
	{
		m_frames[Normal_x] = physx::PxPlane(PxVec3((physx::PxF32)0), tm.M(0, 2));
	}
	if ((fieldMask >> Normal_y) & 1)
	{
		m_frames[Normal_y] = physx::PxPlane(PxVec3((physx::PxF32)0), tm.M(1, 2));
	}
	if ((fieldMask >> Normal_z) & 1)
	{
		m_frames[Normal_z] = physx::PxPlane(PxVec3((physx::PxF32)0), tm.M(2, 2));
	}
	if ((fieldMask >> Tangent_x) & 1)
	{
		m_frames[Tangent_x] = physx::PxPlane(PxVec3((physx::PxF32)0), tm.M(0, 0));
	}
	if ((fieldMask >> Tangent_y) & 1)
	{
		m_frames[Tangent_y] = physx::PxPlane(PxVec3((physx::PxF32)0), tm.M(1, 0));
	}
	if ((fieldMask >> Tangent_z) & 1)
	{
		m_frames[Tangent_z] = physx::PxPlane(PxVec3((physx::PxF32)0), tm.M(2, 0));
	}
	if ((fieldMask >> Binormal_x) & 1)
	{
		m_frames[Binormal_x] = physx::PxPlane(PxVec3((physx::PxF32)0), tm.M(0, 1));
	}
	if ((fieldMask >> Binormal_y) & 1)
	{
		m_frames[Binormal_y] = physx::PxPlane(PxVec3((physx::PxF32)0), tm.M(1, 1));
	}
	if ((fieldMask >> Binormal_z) & 1)
	{
		m_frames[Binormal_z] = physx::PxPlane(PxVec3((physx::PxF32)0), tm.M(2, 1));
	}
	const physx::PxVec3 psu = (uvScale[0] ? 1 / uvScale[0] : (physx::PxF32)0) * tm.M.getColumn(0);
	const physx::PxVec3 psv = (uvScale[1] ? 1 / uvScale[1] : (physx::PxF32)0) * tm.M.getColumn(1);
	if ((fieldMask >> UV0_u) & 1)
	{
		m_frames[UV0_u] = physx::PxPlane(psu, uvOffset[0]);
	}
	if ((fieldMask >> UV0_v) & 1)
	{
		m_frames[UV0_v] = physx::PxPlane(psv, uvOffset[1]);
	}
	if ((fieldMask >> UV1_u) & 1)
	{
		m_frames[UV1_u] = physx::PxPlane(psu, uvOffset[0]);
	}
	if ((fieldMask >> UV1_v) & 1)
	{
		m_frames[UV1_v] = physx::PxPlane(psv, uvOffset[1]);
	}
	if ((fieldMask >> UV2_u) & 1)
	{
		m_frames[UV2_u] = physx::PxPlane(psu, uvOffset[0]);
	}
	if ((fieldMask >> UV2_v) & 1)
	{
		m_frames[UV2_v] = physx::PxPlane(psv, uvOffset[1]);
	}
	if ((fieldMask >> UV3_u) & 1)
	{
		m_frames[UV3_u] = physx::PxPlane(psu, uvOffset[0]);
	}
	if ((fieldMask >> UV3_v) & 1)
	{
		m_frames[UV3_v] = physx::PxPlane(psv, uvOffset[1]);
	}
	if ((fieldMask >> Color_r) & 1)
	{
		m_frames[Color_r] = physx::PxPlane(PxVec3((physx::PxF32)0), (physx::PxF32)1);
	}
	if ((fieldMask >> Color_g) & 1)
	{
		m_frames[Color_g] = physx::PxPlane(PxVec3((physx::PxF32)0), (physx::PxF32)1);
	}
	if ((fieldMask >> Color_b) & 1)
	{
		m_frames[Color_b] = physx::PxPlane(PxVec3((physx::PxF32)0), (physx::PxF32)1);
	}
	if ((fieldMask >> Color_a) & 1)
	{
		m_frames[Color_a] = physx::PxPlane(PxVec3((physx::PxF32)0), (physx::PxF32)1);
	}
}

PX_INLINE void
TriangleFrame::interpolateVertexData(NxVertex& vertex) const
{
	physx::PxU64 fieldMask = m_fieldMask;
	for (physx::PxU32 i = 0; fieldMask != 0 && i < VertexFieldCount; ++i, (fieldMask >>= 1))
	{
		if (fieldMask & 1)
		{
			physx::PxF32& value = *(physx::PxF32*)(((physx::PxU8*)&vertex) + s_offsets[i]);
			value = m_frames[i].distance(vertex.position);
		}
	}
}

class TriangleFrameBuilder
{
	public:
	TriangleFrameBuilder()
	{
#define CREATE_TF_OFFSET( field, element )				(size_t)((uintptr_t)&vertex.field.element-(uintptr_t)&vertex)
#define CREATE_TF_OFFSET_IDX( field, element, index )	(size_t)((uintptr_t)&vertex.field[index].element-(uintptr_t)&vertex)

		NxVertex vertex;
//		TriangleFrame::s_offsets[TriangleFrame::Position_x] =	CREATE_TF_OFFSET( position, x );
//		TriangleFrame::s_offsets[TriangleFrame::Position_y] =	CREATE_TF_OFFSET( position, y );
//		TriangleFrame::s_offsets[TriangleFrame::Position_z] =	CREATE_TF_OFFSET( position, z );
		TriangleFrame::s_offsets[TriangleFrame::Normal_x] =		CREATE_TF_OFFSET(normal, x);
		TriangleFrame::s_offsets[TriangleFrame::Normal_y] =		CREATE_TF_OFFSET(normal, y);
		TriangleFrame::s_offsets[TriangleFrame::Normal_z] =		CREATE_TF_OFFSET(normal, z);
		TriangleFrame::s_offsets[TriangleFrame::Tangent_x] =	CREATE_TF_OFFSET(tangent, x);
		TriangleFrame::s_offsets[TriangleFrame::Tangent_y] =	CREATE_TF_OFFSET(tangent, y);
		TriangleFrame::s_offsets[TriangleFrame::Tangent_z] =	CREATE_TF_OFFSET(tangent, z);
		TriangleFrame::s_offsets[TriangleFrame::Binormal_x] =	CREATE_TF_OFFSET(binormal, x);
		TriangleFrame::s_offsets[TriangleFrame::Binormal_y] =	CREATE_TF_OFFSET(binormal, y);
		TriangleFrame::s_offsets[TriangleFrame::Binormal_z] =	CREATE_TF_OFFSET(binormal, z);
		TriangleFrame::s_offsets[TriangleFrame::UV0_u] =		CREATE_TF_OFFSET_IDX(uv, u, 0);
		TriangleFrame::s_offsets[TriangleFrame::UV0_v] =		CREATE_TF_OFFSET_IDX(uv, v, 0);
		TriangleFrame::s_offsets[TriangleFrame::UV1_u] =		CREATE_TF_OFFSET_IDX(uv, u, 1);
		TriangleFrame::s_offsets[TriangleFrame::UV1_v] =		CREATE_TF_OFFSET_IDX(uv, v, 1);
		TriangleFrame::s_offsets[TriangleFrame::UV2_u] =		CREATE_TF_OFFSET_IDX(uv, u, 2);
		TriangleFrame::s_offsets[TriangleFrame::UV2_v] =		CREATE_TF_OFFSET_IDX(uv, v, 2);
		TriangleFrame::s_offsets[TriangleFrame::UV3_u] =		CREATE_TF_OFFSET_IDX(uv, u, 3);
		TriangleFrame::s_offsets[TriangleFrame::UV3_v] =		CREATE_TF_OFFSET_IDX(uv, v, 3);
		TriangleFrame::s_offsets[TriangleFrame::Color_r] =		CREATE_TF_OFFSET(color, r);
		TriangleFrame::s_offsets[TriangleFrame::Color_g] =		CREATE_TF_OFFSET(color, g);
		TriangleFrame::s_offsets[TriangleFrame::Color_b] =		CREATE_TF_OFFSET(color, b);
		TriangleFrame::s_offsets[TriangleFrame::Color_a] =		CREATE_TF_OFFSET(color, a);
	}
};


// Format conversion utilities

// Explicit data layouts, used for data conversion

typedef physx::PxU8 UBYTE1_TYPE;
typedef physx::PxU8 UBYTE2_TYPE[2];
typedef physx::PxU8 UBYTE3_TYPE[3];
typedef physx::PxU8 UBYTE4_TYPE[4];

typedef physx::PxU16 USHORT1_TYPE;
typedef physx::PxU16 USHORT2_TYPE[2];
typedef physx::PxU16 USHORT3_TYPE[3];
typedef physx::PxU16 USHORT4_TYPE[4];

typedef physx::PxI16 SHORT1_TYPE;
typedef physx::PxI16 SHORT2_TYPE[2];
typedef physx::PxI16 SHORT3_TYPE[3];
typedef physx::PxI16 SHORT4_TYPE[4];

typedef physx::PxU32 UINT1_TYPE;
typedef physx::PxU32 UINT2_TYPE[2];
typedef physx::PxU32 UINT3_TYPE[3];
typedef physx::PxU32 UINT4_TYPE[4];

struct R8G8B8A8_TYPE
{
	physx::PxU8 r, g, b, a;
};
struct B8G8R8A8_TYPE
{
	physx::PxU8 b, g, r, a;
};
struct R32G32B32A32_FLOAT_TYPE
{
	physx::PxF32 r, g, b, a;
};
struct B32G32R32A32_FLOAT_TYPE
{
	physx::PxF32 b, g, r, a;
};

typedef physx::PxU8 BYTE_UNORM1_TYPE;
typedef physx::PxU8 BYTE_UNORM2_TYPE[2];
typedef physx::PxU8 BYTE_UNORM3_TYPE[3];
typedef physx::PxU8 BYTE_UNORM4_TYPE[4];

typedef physx::PxU16 SHORT_UNORM1_TYPE;
typedef physx::PxU16 SHORT_UNORM2_TYPE[2];
typedef physx::PxU16 SHORT_UNORM3_TYPE[3];
typedef physx::PxU16 SHORT_UNORM4_TYPE[4];

typedef physx::PxI8 BYTE_SNORM1_TYPE;
typedef physx::PxI8 BYTE_SNORM2_TYPE[2];
typedef physx::PxI8 BYTE_SNORM3_TYPE[3];
typedef physx::PxI8 BYTE_SNORM4_TYPE[4];

typedef physx::PxI16 SHORT_SNORM1_TYPE;
typedef physx::PxI16 SHORT_SNORM2_TYPE[2];
typedef physx::PxI16 SHORT_SNORM3_TYPE[3];
typedef physx::PxI16 SHORT_SNORM4_TYPE[4];

typedef physx::PxU16 HALF1_TYPE;
typedef physx::PxU16 HALF2_TYPE[2];
typedef physx::PxU16 HALF3_TYPE[3];
typedef physx::PxU16 HALF4_TYPE[4];

typedef physx::PxF32 FLOAT1_TYPE;
typedef physx::PxF32 FLOAT2_TYPE[2];
typedef physx::PxF32 FLOAT3_TYPE[3];
typedef physx::PxF32 FLOAT4_TYPE[4];

typedef physx::PxMat34Legacy	FLOAT3x4_TYPE;
typedef physx::PxMat33			FLOAT3x3_TYPE;

typedef physx::PxQuat	FLOAT4_QUAT_TYPE;
typedef physx::PxI8		BYTE_SNORM4_QUATXYZW_TYPE[4];
typedef physx::PxI16	SHORT_SNORM4_QUATXYZW_TYPE[4];


// Data converters

// USHORT1_TYPE -> UINT1_TYPE
PX_INLINE void convert_UINT1_from_USHORT1(UINT1_TYPE& dst, const USHORT1_TYPE& src)
{
	dst = (physx::PxU32)src;
}

// USHORT2_TYPE -> UINT2_TYPE
PX_INLINE void convert_UINT2_from_USHORT2(UINT2_TYPE& dst, const USHORT2_TYPE& src)
{
	convert_UINT1_from_USHORT1(dst[0], src[0]);
	convert_UINT1_from_USHORT1(dst[1], src[1]);
}

// USHORT3_TYPE -> UINT3_TYPE
PX_INLINE void convert_UINT3_from_USHORT3(UINT3_TYPE& dst, const USHORT3_TYPE& src)
{
	convert_UINT1_from_USHORT1(dst[0], src[0]);
	convert_UINT1_from_USHORT1(dst[1], src[1]);
	convert_UINT1_from_USHORT1(dst[2], src[2]);
}

// USHORT4_TYPE -> UINT4_TYPE
PX_INLINE void convert_UINT4_from_USHORT4(UINT4_TYPE& dst, const USHORT4_TYPE& src)
{
	convert_UINT1_from_USHORT1(dst[0], src[0]);
	convert_UINT1_from_USHORT1(dst[1], src[1]);
	convert_UINT1_from_USHORT1(dst[2], src[2]);
	convert_UINT1_from_USHORT1(dst[3], src[3]);
}

// UINT1_TYPE -> USHORT1_TYPE
PX_INLINE void convert_USHORT1_from_UINT1(USHORT1_TYPE& dst, const UINT1_TYPE& src)
{
	dst = (physx::PxU16)src;
}

// UINT2_TYPE -> USHORT2_TYPE
PX_INLINE void convert_USHORT2_from_UINT2(USHORT2_TYPE& dst, const UINT2_TYPE& src)
{
	convert_USHORT1_from_UINT1(dst[0], src[0]);
	convert_USHORT1_from_UINT1(dst[1], src[1]);
}

// UINT3_TYPE -> USHORT3_TYPE
PX_INLINE void convert_USHORT3_from_UINT3(USHORT3_TYPE& dst, const UINT3_TYPE& src)
{
	convert_USHORT1_from_UINT1(dst[0], src[0]);
	convert_USHORT1_from_UINT1(dst[1], src[1]);
	convert_USHORT1_from_UINT1(dst[2], src[2]);
}

// UINT4_TYPE -> USHORT4_TYPE
PX_INLINE void convert_USHORT4_from_UINT4(USHORT4_TYPE& dst, const UINT4_TYPE& src)
{
	convert_USHORT1_from_UINT1(dst[0], src[0]);
	convert_USHORT1_from_UINT1(dst[1], src[1]);
	convert_USHORT1_from_UINT1(dst[2], src[2]);
	convert_USHORT1_from_UINT1(dst[3], src[3]);
}

// BYTE_SNORM1_TYPE -> FLOAT1_TYPE
PX_INLINE void convert_FLOAT1_from_BYTE_SNORM1(FLOAT1_TYPE& dst, const BYTE_SNORM1_TYPE& src)
{
	dst = (physx::PxF32)src / 127.0f;
}

// BYTE_SNORM2_TYPE -> FLOAT2_TYPE
PX_INLINE void convert_FLOAT2_from_BYTE_SNORM2(FLOAT2_TYPE& dst, const BYTE_SNORM2_TYPE& src)
{
	convert_FLOAT1_from_BYTE_SNORM1(dst[0], src[0]);
	convert_FLOAT1_from_BYTE_SNORM1(dst[1], src[1]);
}

// BYTE_SNORM3_TYPE -> FLOAT3_TYPE
PX_INLINE void convert_FLOAT3_from_BYTE_SNORM3(FLOAT3_TYPE& dst, const BYTE_SNORM3_TYPE& src)
{
	convert_FLOAT1_from_BYTE_SNORM1(dst[0], src[0]);
	convert_FLOAT1_from_BYTE_SNORM1(dst[1], src[1]);
	convert_FLOAT1_from_BYTE_SNORM1(dst[2], src[2]);
}

// BYTE_SNORM4_TYPE -> FLOAT4_TYPE
PX_INLINE void convert_FLOAT4_from_BYTE_SNORM4(FLOAT4_TYPE& dst, const BYTE_SNORM4_TYPE& src)
{
	convert_FLOAT1_from_BYTE_SNORM1(dst[0], src[0]);
	convert_FLOAT1_from_BYTE_SNORM1(dst[1], src[1]);
	convert_FLOAT1_from_BYTE_SNORM1(dst[2], src[2]);
	convert_FLOAT1_from_BYTE_SNORM1(dst[3], src[3]);
}

// BYTE_SNORM4_QUATXYZW_TYPE -> FLOAT4_QUAT_TYPE
PX_INLINE void convert_FLOAT4_QUAT_from_BYTE_SNORM4_QUATXYZW(FLOAT4_QUAT_TYPE& dst, const BYTE_SNORM4_QUATXYZW_TYPE& src)
{
	convert_FLOAT1_from_BYTE_SNORM1(dst.x, src[0]);
	convert_FLOAT1_from_BYTE_SNORM1(dst.y, src[1]);
	convert_FLOAT1_from_BYTE_SNORM1(dst.z, src[2]);
	convert_FLOAT1_from_BYTE_SNORM1(dst.w, src[3]);
}

// SHORT_SNORM1_TYPE -> FLOAT1_TYPE
PX_INLINE void convert_FLOAT1_from_SHORT_SNORM1(FLOAT1_TYPE& dst, const SHORT_SNORM1_TYPE& src)
{
	dst = (physx::PxF32)src / 32767.0f;
}

// SHORT_SNORM2_TYPE -> FLOAT2_TYPE
PX_INLINE void convert_FLOAT2_from_SHORT_SNORM2(FLOAT2_TYPE& dst, const SHORT_SNORM2_TYPE& src)
{
	convert_FLOAT1_from_SHORT_SNORM1(dst[0], src[0]);
	convert_FLOAT1_from_SHORT_SNORM1(dst[1], src[1]);
}

// SHORT_SNORM3_TYPE -> FLOAT3_TYPE
PX_INLINE void convert_FLOAT3_from_SHORT_SNORM3(FLOAT3_TYPE& dst, const SHORT_SNORM3_TYPE& src)
{
	convert_FLOAT1_from_SHORT_SNORM1(dst[0], src[0]);
	convert_FLOAT1_from_SHORT_SNORM1(dst[1], src[1]);
	convert_FLOAT1_from_SHORT_SNORM1(dst[2], src[2]);
}

// SHORT_SNORM4_TYPE -> FLOAT4_TYPE
PX_INLINE void convert_FLOAT4_from_SHORT_SNORM4(FLOAT4_TYPE& dst, const SHORT_SNORM4_TYPE& src)
{
	convert_FLOAT1_from_SHORT_SNORM1(dst[0], src[0]);
	convert_FLOAT1_from_SHORT_SNORM1(dst[1], src[1]);
	convert_FLOAT1_from_SHORT_SNORM1(dst[2], src[2]);
	convert_FLOAT1_from_SHORT_SNORM1(dst[3], src[3]);
}

// SHORT_SNORM4_QUATXYZW_TYPE -> FLOAT4_QUAT_TYPE
PX_INLINE void convert_FLOAT4_QUAT_from_SHORT_SNORM4_QUATXYZW(FLOAT4_QUAT_TYPE& dst, const SHORT_SNORM4_QUATXYZW_TYPE& src)
{
	convert_FLOAT1_from_SHORT_SNORM1(dst.x, src[0]);
	convert_FLOAT1_from_SHORT_SNORM1(dst.y, src[1]);
	convert_FLOAT1_from_SHORT_SNORM1(dst.z, src[2]);
	convert_FLOAT1_from_SHORT_SNORM1(dst.w, src[3]);
}

// FLOAT1_TYPE -> BYTE_SNORM1_TYPE
PX_INLINE void convert_BYTE_SNORM1_from_FLOAT1(BYTE_SNORM1_TYPE& dst, const FLOAT1_TYPE& src)
{
	dst = (physx::PxI8)((physx::PxI16)(src * 127.0f + 127.5f) - 127);	// Doing it this way to avoid nonuniform mapping near zero
}

// FLOAT2_TYPE -> BYTE_SNORM2_TYPE
PX_INLINE void convert_BYTE_SNORM2_from_FLOAT2(BYTE_SNORM2_TYPE& dst, const FLOAT2_TYPE& src)
{
	convert_BYTE_SNORM1_from_FLOAT1(dst[0], src[0]);
	convert_BYTE_SNORM1_from_FLOAT1(dst[1], src[1]);
}

// FLOAT3_TYPE -> BYTE_SNORM3_TYPE
PX_INLINE void convert_BYTE_SNORM3_from_FLOAT3(BYTE_SNORM3_TYPE& dst, const FLOAT3_TYPE& src)
{
	convert_BYTE_SNORM1_from_FLOAT1(dst[0], src[0]);
	convert_BYTE_SNORM1_from_FLOAT1(dst[1], src[1]);
	convert_BYTE_SNORM1_from_FLOAT1(dst[2], src[2]);
}

// FLOAT4_TYPE -> BYTE_SNORM4_TYPE
PX_INLINE void convert_BYTE_SNORM4_from_FLOAT4(BYTE_SNORM4_TYPE& dst, const FLOAT4_TYPE& src)
{
	convert_BYTE_SNORM1_from_FLOAT1(dst[0], src[0]);
	convert_BYTE_SNORM1_from_FLOAT1(dst[1], src[1]);
	convert_BYTE_SNORM1_from_FLOAT1(dst[2], src[2]);
	convert_BYTE_SNORM1_from_FLOAT1(dst[3], src[3]);
}

// FLOAT4_QUAT_TYPE -> BYTE_SNORM4_QUATXYZW_TYPE
PX_INLINE void convert_BYTE_SNORM4_QUATXYZW_from_FLOAT4_QUAT(BYTE_SNORM4_QUATXYZW_TYPE& dst, const FLOAT4_QUAT_TYPE& src)
{
	convert_BYTE_SNORM1_from_FLOAT1(dst[0], src.x);
	convert_BYTE_SNORM1_from_FLOAT1(dst[1], src.y);
	convert_BYTE_SNORM1_from_FLOAT1(dst[2], src.z);
	convert_BYTE_SNORM1_from_FLOAT1(dst[3], src.w);
}

// FLOAT1_TYPE -> SHORT_SNORM1_TYPE
PX_INLINE void convert_SHORT_SNORM1_from_FLOAT1(SHORT_SNORM1_TYPE& dst, const FLOAT1_TYPE& src)
{
	dst = (physx::PxI16)((physx::PxI32)(src * 32767.0f + 32767.5f) - 32767);	// Doing it this way to avoid nonuniform mapping near zero
}

// FLOAT2_TYPE -> SHORT_SNORM2_TYPE
PX_INLINE void convert_SHORT_SNORM2_from_FLOAT2(SHORT_SNORM2_TYPE& dst, const FLOAT2_TYPE& src)
{
	convert_SHORT_SNORM1_from_FLOAT1(dst[0], src[0]);
	convert_SHORT_SNORM1_from_FLOAT1(dst[1], src[1]);
}

// FLOAT3_TYPE -> SHORT_SNORM3_TYPE
PX_INLINE void convert_SHORT_SNORM3_from_FLOAT3(SHORT_SNORM3_TYPE& dst, const FLOAT3_TYPE& src)
{
	convert_SHORT_SNORM1_from_FLOAT1(dst[0], src[0]);
	convert_SHORT_SNORM1_from_FLOAT1(dst[1], src[1]);
	convert_SHORT_SNORM1_from_FLOAT1(dst[2], src[2]);
}

// FLOAT4_TYPE -> SHORT_SNORM4_TYPE
PX_INLINE void convert_SHORT_SNORM4_from_FLOAT4(SHORT_SNORM4_TYPE& dst, const FLOAT4_TYPE& src)
{
	convert_SHORT_SNORM1_from_FLOAT1(dst[0], src[0]);
	convert_SHORT_SNORM1_from_FLOAT1(dst[1], src[1]);
	convert_SHORT_SNORM1_from_FLOAT1(dst[2], src[2]);
	convert_SHORT_SNORM1_from_FLOAT1(dst[3], src[3]);
}

// FLOAT4_QUAT_TYPE -> SHORT_SNORM4_QUATXYZW_TYPE
PX_INLINE void convert_SHORT_SNORM4_QUATXYZW_from_FLOAT4_QUAT(SHORT_SNORM4_QUATXYZW_TYPE& dst, const FLOAT4_QUAT_TYPE& src)
{
	convert_SHORT_SNORM1_from_FLOAT1(dst[0], src.x);
	convert_SHORT_SNORM1_from_FLOAT1(dst[1], src.y);
	convert_SHORT_SNORM1_from_FLOAT1(dst[2], src.z);
	convert_SHORT_SNORM1_from_FLOAT1(dst[3], src.w);
}

// Color format conversions
PX_INLINE void convert_B8G8R8A8_from_R8G8B8A8(B8G8R8A8_TYPE& dst, const R8G8B8A8_TYPE& src)
{
	dst.r = src.r;
	dst.g = src.g;
	dst.b = src.b;
	dst.a = src.a;
}

PX_INLINE void convert_R8G8B8A8_from_B8G8R8A8(R8G8B8A8_TYPE& dst, const B8G8R8A8_TYPE& src)
{
	dst.r = src.r;
	dst.g = src.g;
	dst.b = src.b;
	dst.a = src.a;
}

PX_INLINE void convert_R32G32B32A32_FLOAT_from_R8G8B8A8(R32G32B32A32_FLOAT_TYPE& dst, const R8G8B8A8_TYPE& src)
{
	(NxVertexColor&)dst = NxVertexColor((const PxColorRGBA&)src);
}

PX_INLINE void convert_R8G8B8A8_from_R32G32B32A32_FLOAT(R8G8B8A8_TYPE& dst, const R32G32B32A32_FLOAT_TYPE& src)
{
	(PxColorRGBA&)dst = ((const NxVertexColor&)src).toColorRGBA();
}

PX_INLINE void convert_B32G32R32A32_FLOAT_from_R8G8B8A8(B32G32R32A32_FLOAT_TYPE& dst, const R8G8B8A8_TYPE& src)
{
	(NxVertexColor&)dst = NxVertexColor((const PxColorRGBA&)src);
	physx::PxF32 t = dst.r;
	dst.r = dst.b;
	dst.b = t;
}

PX_INLINE void convert_R8G8B8A8_from_B32G32R32A32_FLOAT(R8G8B8A8_TYPE& dst, const B32G32R32A32_FLOAT_TYPE& src)
{
	(PxColorRGBA&)dst = ((const NxVertexColor&)src).toColorRGBA();
	physx::PxU8 t = dst.r;
	dst.r = dst.b;
	dst.b = t;
}

PX_INLINE void convert_R32G32B32A32_FLOAT_from_B8G8R8A8(R32G32B32A32_FLOAT_TYPE& dst, const B8G8R8A8_TYPE& src)
{
	(NxVertexColor&)dst = NxVertexColor((const PxColorRGBA&)src);
	physx::PxF32 t = dst.r;
	dst.r = dst.b;
	dst.b = t;
}

PX_INLINE void convert_B8G8R8A8_from_R32G32B32A32_FLOAT(B8G8R8A8_TYPE& dst, const R32G32B32A32_FLOAT_TYPE& src)
{
	(PxColorRGBA&)dst = ((const NxVertexColor&)src).toColorRGBA();
	physx::PxU8 t = dst.r;
	dst.r = dst.b;
	dst.b = t;
}

PX_INLINE void convert_B32G32R32A32_FLOAT_from_B8G8R8A8(B32G32R32A32_FLOAT_TYPE& dst, const B8G8R8A8_TYPE& src)
{
	(NxVertexColor&)dst = NxVertexColor((const PxColorRGBA&)src);
}

PX_INLINE void convert_B8G8R8A8_from_B32G32R32A32_FLOAT(B8G8R8A8_TYPE& dst, const B32G32R32A32_FLOAT_TYPE& src)
{
	(PxColorRGBA&)dst = ((const NxVertexColor&)src).toColorRGBA();
}

PX_INLINE void convert_B32G32R32A32_FLOAT_from_R32G32B32A32_FLOAT(B32G32R32A32_FLOAT_TYPE& dst, const R32G32B32A32_FLOAT_TYPE& src)
{
	dst.r = src.r;
	dst.g = src.g;
	dst.b = src.b;
	dst.a = src.a;
}

PX_INLINE void convert_R32G32B32A32_FLOAT_from_B32G32R32A32_FLOAT(R32G32B32A32_FLOAT_TYPE& dst, const B32G32R32A32_FLOAT_TYPE& src)
{
	dst.r = src.r;
	dst.g = src.g;
	dst.b = src.b;
	dst.a = src.a;
}

// Data conversion macros
#define HANDLE_CONVERT1( _DstFormat, _SrcFormat ) \
	case NxRenderDataFormat::_DstFormat : \
		if( srcFormat == NxRenderDataFormat::_SrcFormat ) \
		{ \
			convert_##_DstFormat##_from_##_SrcFormat( ((_DstFormat##_TYPE*)dst)[dstIndex], ((const _SrcFormat##_TYPE*)src)[srcIndex] ); \
		} \
		break;

#define HANDLE_CONVERT2( _DstFormat, _SrcFormat1, _SrcFormat2 ) \
	case NxRenderDataFormat::_DstFormat : \
		if( srcFormat == NxRenderDataFormat::_SrcFormat1 ) \
		{ \
			convert_##_DstFormat##_from_##_SrcFormat1( ((_DstFormat##_TYPE*)dst)[dstIndex], ((const _SrcFormat1##_TYPE*)src)[srcIndex] ); \
		} \
		else if( srcFormat == NxRenderDataFormat::_SrcFormat2 ) \
		{ \
			convert_##_DstFormat##_from_##_SrcFormat2( ((_DstFormat##_TYPE*)dst)[dstIndex], ((const _SrcFormat2##_TYPE*)src)[srcIndex] ); \
		} \
		break;

#define HANDLE_CONVERT3( _DstFormat, _SrcFormat1, _SrcFormat2, _SrcFormat3 ) \
	case NxRenderDataFormat::_DstFormat : \
		if( srcFormat == NxRenderDataFormat::_SrcFormat1 ) \
		{ \
			convert_##_DstFormat##_from_##_SrcFormat1( ((_DstFormat##_TYPE*)dst)[dstIndex], ((const _SrcFormat1##_TYPE*)src)[srcIndex] ); \
		} \
		else if( srcFormat == NxRenderDataFormat::_SrcFormat2 ) \
		{ \
			convert_##_DstFormat##_from_##_SrcFormat2( ((_DstFormat##_TYPE*)dst)[dstIndex], ((const _SrcFormat2##_TYPE*)src)[srcIndex] ); \
		} \
		else if( srcFormat == NxRenderDataFormat::_SrcFormat3 ) \
		{ \
			convert_##_DstFormat##_from_##_SrcFormat3( ((_DstFormat##_TYPE*)dst)[dstIndex], ((const _SrcFormat3##_TYPE*)src)[srcIndex] ); \
		} \
		break;

// ... etc.

PX_INLINE bool copyRenderVertexData(void* dst, NxRenderDataFormat::Enum dstFormat, PxU32 dstIndex, const void* src, NxRenderDataFormat::Enum srcFormat, PxU32 srcIndex)
{
	if (dstFormat == srcFormat)
	{
		// Direct data copy
		if (dstFormat != NxRenderDataFormat::UNSPECIFIED)
		{
			PxU8* srcPtr = (PxU8*)src;
			PxU8* dstPtr = (PxU8*)dst;

			const PxU32 size = NxRenderDataFormat::getFormatDataSize(dstFormat);
			memcpy(dstPtr + (dstIndex * size), srcPtr + (srcIndex * size), size);
		}
		return true;
	}

	switch (dstFormat)
	{
	case NxRenderDataFormat::UNSPECIFIED:
			break; // The simplest case, do nothing

		// Put format converters here

		HANDLE_CONVERT1(USHORT1, UINT1)
		HANDLE_CONVERT1(USHORT2, UINT2)
		HANDLE_CONVERT1(USHORT3, UINT3)
		HANDLE_CONVERT1(USHORT4, UINT4)

		HANDLE_CONVERT1(UINT1, USHORT1)
		HANDLE_CONVERT1(UINT2, USHORT2)
		HANDLE_CONVERT1(UINT3, USHORT3)
		HANDLE_CONVERT1(UINT4, USHORT4)

		HANDLE_CONVERT1(BYTE_SNORM1, FLOAT1)
		HANDLE_CONVERT1(BYTE_SNORM2, FLOAT2)
		HANDLE_CONVERT1(BYTE_SNORM3, FLOAT3)
		HANDLE_CONVERT1(BYTE_SNORM4, FLOAT4)
		HANDLE_CONVERT1(BYTE_SNORM4_QUATXYZW, FLOAT4_QUAT)
		HANDLE_CONVERT1(SHORT_SNORM1, FLOAT1)
		HANDLE_CONVERT1(SHORT_SNORM2, FLOAT2)
		HANDLE_CONVERT1(SHORT_SNORM3, FLOAT3)
		HANDLE_CONVERT1(SHORT_SNORM4, FLOAT4)
		HANDLE_CONVERT1(SHORT_SNORM4_QUATXYZW, FLOAT4_QUAT)

		HANDLE_CONVERT2(FLOAT1, BYTE_SNORM1, SHORT_SNORM1)
		HANDLE_CONVERT2(FLOAT2, BYTE_SNORM2, SHORT_SNORM2)
		HANDLE_CONVERT2(FLOAT3, BYTE_SNORM3, SHORT_SNORM3)
		HANDLE_CONVERT2(FLOAT4, BYTE_SNORM4, SHORT_SNORM4)
		HANDLE_CONVERT2(FLOAT4_QUAT, BYTE_SNORM4_QUATXYZW, SHORT_SNORM4_QUATXYZW)

		HANDLE_CONVERT3(R8G8B8A8, B8G8R8A8, R32G32B32A32_FLOAT, B32G32R32A32_FLOAT)
		HANDLE_CONVERT3(B8G8R8A8, R8G8B8A8, R32G32B32A32_FLOAT, B32G32R32A32_FLOAT)
		HANDLE_CONVERT3(R32G32B32A32_FLOAT, R8G8B8A8, B8G8R8A8, B32G32R32A32_FLOAT)
		HANDLE_CONVERT3(B32G32R32A32_FLOAT, R8G8B8A8, B8G8R8A8, R32G32B32A32_FLOAT)

	default:
		{
		    PX_ALWAYS_ASSERT();	// Format conversion not handled
		    return false;
		}
		}

	return true;
}

bool copyRenderVertexBuffer(void* dst, NxRenderDataFormat::Enum dstFormat, PxU32 dstStride, PxU32 dstStart,
                            const void* src, NxRenderDataFormat::Enum srcFormat, PxU32 srcStride, PxU32 srcStart,
                            PxU32 numVertices, PxI32* invMap = NULL);

/*
	Local utilities
 */
PX_INLINE bool vertexSemanticFormatValid(NxRenderVertexSemantic::Enum semantic, NxRenderDataFormat::Enum format)
{
	switch (semantic)
	{
	case NxRenderVertexSemantic::POSITION:
			return format == NxRenderDataFormat::FLOAT3;
	case NxRenderVertexSemantic::NORMAL:
			case NxRenderVertexSemantic::BINORMAL:
					return format == NxRenderDataFormat::FLOAT3 || format == NxRenderDataFormat::BYTE_SNORM3;
	case NxRenderVertexSemantic::TANGENT:
		return	format == NxRenderDataFormat::FLOAT3 || format == NxRenderDataFormat::BYTE_SNORM3 ||
				format == NxRenderDataFormat::FLOAT4 || format == NxRenderDataFormat::BYTE_SNORM4;
	case NxRenderVertexSemantic::COLOR:
			return format == NxRenderDataFormat::R8G8B8A8 || format == NxRenderDataFormat::B8G8R8A8;
	case NxRenderVertexSemantic::TEXCOORD0:
		case NxRenderVertexSemantic::TEXCOORD1:
			case NxRenderVertexSemantic::TEXCOORD2:
				case NxRenderVertexSemantic::TEXCOORD3:
						return format == NxRenderDataFormat::FLOAT2;	// Not supporting other formats yet
	case NxRenderVertexSemantic::DISPLACEMENT_TEXCOORD:
		return format == NxRenderDataFormat::FLOAT2 || format == NxRenderDataFormat::FLOAT3;
	case NxRenderVertexSemantic::DISPLACEMENT_FLAGS:
		return format == NxRenderDataFormat::UINT1 || format == NxRenderDataFormat::USHORT1;
	case NxRenderVertexSemantic::BONE_INDEX:
			return	format == NxRenderDataFormat::USHORT1 ||
			        format == NxRenderDataFormat::USHORT2 ||
			        format == NxRenderDataFormat::USHORT3 ||
			        format == NxRenderDataFormat::USHORT4;	// Not supporting other formats yet
	case NxRenderVertexSemantic::BONE_WEIGHT:
			return	format == NxRenderDataFormat::FLOAT1 ||
			        format == NxRenderDataFormat::FLOAT2 ||
			        format == NxRenderDataFormat::FLOAT3 ||
			        format == NxRenderDataFormat::FLOAT4;	// Not supporting other formats yet
	default:
		return false;
	}
}

PX_INLINE PxU32 vertexSemanticFormatElementCount(NxRenderVertexSemantic::Enum semantic, NxRenderDataFormat::Enum format)
{
	switch (semantic)
	{
	case NxRenderVertexSemantic::CUSTOM:
		case NxRenderVertexSemantic::POSITION:
			case NxRenderVertexSemantic::NORMAL:
				case NxRenderVertexSemantic::TANGENT:
					case NxRenderVertexSemantic::BINORMAL:
						case NxRenderVertexSemantic::COLOR:
							case NxRenderVertexSemantic::TEXCOORD0:
								case NxRenderVertexSemantic::TEXCOORD1:
									case NxRenderVertexSemantic::TEXCOORD2:
										case NxRenderVertexSemantic::TEXCOORD3:
											case NxRenderVertexSemantic::DISPLACEMENT_TEXCOORD:
												case NxRenderVertexSemantic::DISPLACEMENT_FLAGS:
													return 1;
	case NxRenderVertexSemantic::BONE_INDEX:
			switch (format)
			{
			case NxRenderDataFormat::USHORT1:
					return 1;
			case NxRenderDataFormat::USHORT2:
					return 2;
			case NxRenderDataFormat::USHORT3:
					return 3;
			case NxRenderDataFormat::USHORT4:
					return 4;
			default:
				break;
			}
			return 0;
	case NxRenderVertexSemantic::BONE_WEIGHT:
			switch (format)
			{
			case NxRenderDataFormat::FLOAT1:
					return 1;
			case NxRenderDataFormat::FLOAT2:
					return 2;
			case NxRenderDataFormat::FLOAT3:
					return 3;
			case NxRenderDataFormat::FLOAT4:
					return 4;
			default:
				break;
			}
			return 0;
	default:
		return 0;
	}
}


}
} // end namespace apex


#endif	// __APEXSHAREDUTILS_H__
