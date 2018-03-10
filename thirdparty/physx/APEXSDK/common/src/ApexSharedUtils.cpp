/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "NxApex.h"
#include "ApexSharedUtils.h"
#include "ApexRand.h"
#include "PxMemoryBuffer.h"

#if NX_SDK_VERSION_MAJOR == 2
#include <NxConvexMeshDesc.h>
#include <NxCooking.h>
#include <NxPhysicsSDK.h>
#include <NxShape.h>
#elif NX_SDK_VERSION_MAJOR == 3
#include "PxPhysics.h"
#include "cooking/PxConvexMeshDesc.h"
#include "cooking/PxCooking.h"
#include <PxShape.h>
#endif

#include "PsSort.h"

#include "NiApexSDK.h"

#include "NiCof44.h"
#include "foundation/PxPlane.h"

#include "PsVecMath.h"
#include "PsMathUtils.h"

#include "../../shared/general/stan_hull/include/StanHull.h"

#define REDUCE_CONVEXHULL_INPUT_POINT_SET	0

namespace physx
{
namespace apex
{
// Local utilities

// copied from //sw/physx/PhysXSDK/3.3/trunk/Samples/SampleBase/RenderClothActor.cpp
PxReal det(PxVec4 v0, PxVec4 v1, PxVec4 v2, PxVec4 v3)
{
	const PxVec3& d0 = reinterpret_cast<const PxVec3&>(v0);
	const PxVec3& d1 = reinterpret_cast<const PxVec3&>(v1);
	const PxVec3& d2 = reinterpret_cast<const PxVec3&>(v2);
	const PxVec3& d3 = reinterpret_cast<const PxVec3&>(v3);

	return v0.w * d1.cross(d2).dot(d3)
		- v1.w * d0.cross(d2).dot(d3)
		+ v2.w * d0.cross(d1).dot(d3)
		- v3.w * d0.cross(d1).dot(d2);
}

PX_INLINE PxReal det(const physx::PxMat44& m)
{
	return det(m.column0, m.column1, m.column2, m.column3);
}

PX_INLINE physx::PxF32 extentDistance(physx::PxF32 min0, physx::PxF32 max0, physx::PxF32 min1, physx::PxF32 max1)
{
	return PxMax(min0 - max1, min1 - max0);
}

PX_INLINE physx::PxF32 extentDistanceAndNormalDirection(physx::PxF32 min0, physx::PxF32 max0, physx::PxF32 min1, physx::PxF32 max1, physx::PxF32& midpoint, bool& normalPointsFrom0to1)
{
	const physx::PxF32 d01 = min1 - max0;
	const physx::PxF32 d10 = min0 - max1;

	normalPointsFrom0to1 = d01 > d10;

	if (normalPointsFrom0to1)
	{
		midpoint = 0.5f*(min1 + max0);
		return d01;
	}
	else
	{
		midpoint = 0.5f*(min0 + max1);
		return d10;
	}
}

PX_INLINE void extentOverlapTimeInterval(physx::PxF32& tIn, physx::PxF32& tOut, physx::PxF32 vel0, physx::PxF32 min0, physx::PxF32 max0, physx::PxF32 min1, physx::PxF32 max1)
{
	if (physx::PxAbs(vel0) < PX_EPS_F32)
	{
		// Not moving
		if (extentDistance(min0, max0, min1, max1) < 0.0f)
		{
			// Always overlap
			tIn = -PX_MAX_F32;
			tOut = PX_MAX_F32;
		}
		else
		{
			// Never overlap
			tIn = PX_MAX_F32;
			tOut = -PX_MAX_F32;
		}
		return;
	}

	const physx::PxF32 recipVel0 = 1.0f / vel0;

	if (vel0 > 0.0f)
	{
		tIn = (min1 - max0) * recipVel0;
		tOut = (max1 - min0) * recipVel0;
	}
	else
	{
		tIn = (max1 - min0) * recipVel0;
		tOut = (min1 - max0) * recipVel0;
	}
}

PX_INLINE bool updateTimeIntervalAndNormal(physx::PxF32& in, physx::PxF32& out, physx::PxF32& tNormal, physx::PxVec3* normal, physx::PxF32 tIn, physx::PxF32 tOut, const physx::PxVec3& testNormal)
{
	if (tIn >= out || tOut <= in)
	{
		return false;	// No intersection will occur
	}

	if (normal != NULL && tIn > tNormal)
	{
		tNormal = tIn;
		*normal = testNormal;
	}

	in = physx::PxMax(tIn, in);
	out = physx::PxMin(tOut, out);

	return true;
}

PX_INLINE void getExtent(physx::PxF32& min, physx::PxF32& max, const void* points, physx::PxU32 pointCount, const physx::PxU32 pointByteStride, const physx::PxVec3& normal)
{
	min = PX_MAX_F32;
	max = -PX_MAX_F32;
	const physx::PxVec3* p = (const physx::PxVec3*)points;
	for (physx::PxU32 i = 0; i < pointCount; ++i, p = (const physx::PxVec3*)(((const physx::PxU8*)p) + pointByteStride))
	{
		const physx::PxF32 d = normal.dot(*p);
		if (d < min)
		{
			min = d;
		}
		if (d > max)
		{
			max = d;
		}
	}
}

PX_INLINE bool intersectPlanes(physx::PxVec3& point, const physx::PxPlane& p0, const physx::PxPlane& p1, const physx::PxPlane& p2)
{
	const physx::PxVec3 p1xp2 = p1.n.cross(p2.n);
	const physx::PxF32 det = p0.n.dot(p1xp2);
	if (physx::PxAbs(det) < 1.0e-18)
	{
		return false;	// No intersection
	}
	point = (-p0.d * p1xp2 - p1.d * (p2.n.cross(p0.n)) - p2.d * (p0.n.cross(p1.n))) / det;
	return true;
}

PX_INLINE bool pointInsidePlanes(const physx::PxVec3& point, const physx::PxPlane* planes, physx::PxU32 numPlanes, physx::PxF32 eps)
{
	for (physx::PxU32 i = 0; i < numPlanes; ++i)
	{
		const physx::PxPlane& plane = planes[i];
		if (plane.distance(point) > eps)
		{
			return false;
		}
	}
	return true;
}

static void findPrincipleComponents(physx::PxVec3& mean, physx::PxVec3& variance, physx::PxMat33& axes, const physx::PxVec3* points, physx::PxU32 pointCount)
{
	// Find the average of the points
	mean = physx::PxVec3(0.0f);
	for (physx::PxU32 i = 0; i < pointCount; ++i)
	{
		mean += points[i];
	}

	physx::PxMat33 cov = physx::PxMat33::createZero();

	if (pointCount > 0)
	{
		mean *= 1.0f/pointCount;

		// Now subtract the mean and add to the covariance matrix cov
		for (physx::PxU32 i = 0; i < pointCount; ++i)
		{
			const physx::PxVec3 relativePoint = points[i] - mean;
			for (physx::PxU32 j = 0; j < 3; ++j)
			{
				for (physx::PxU32 k = 0; k < 3; ++k)
				{
					cov(j,k) += relativePoint[j]*relativePoint[k];
				}
			}
		}
		cov *= 1.0f/pointCount;
	}

	// Now diagonalize cov
	variance = diagonalizeSymmetric(axes, cov);
}

#if 0
/* John's k-means clustering function, slightly specialized */
static physx::PxU32 kmeans_cluster(const physx::PxVec3* input,
								   physx::PxU32 inputCount,
								   physx::PxVec3* clusters,
								   physx::PxU32 clumpCount,
								   physx::PxF32 threshold = 0.01f, // controls how long it works to converge towards a least errors solution.
								   physx::PxF32 collapseDistance = 0.0001f) // distance between clumps to consider them to be essentially equal.
{
	physx::PxU32 convergeCount = 64; // maximum number of iterations attempting to converge to a solution..
	physx::Array<physx::PxU32> counts(clumpCount);
	physx::PxF32 error = 0.0f;
	if (inputCount <= clumpCount) // if the number of input points is less than our clumping size, just return the input points.
	{
		clumpCount = inputCount;
		for (physx::PxU32 i = 0; i < inputCount; ++i)
		{
			clusters[i] = input[i];
			counts[i] = 1;
		}
	}
	else
	{
		physx::Array<physx::PxVec3> centroids(clumpCount);

		// Take a sampling of the input points as initial centroid estimates.
		for (physx::PxU32 i = 0; i < clumpCount; ++i)
		{
			physx::PxU32 index = (i*inputCount)/clumpCount;
			PX_ASSERT( index < inputCount );
			clusters[i] = input[index];
		}

		// Here is the main convergence loop
		physx::PxF32 old_error = PX_MAX_F32;	// old and initial error estimates are max physx::PxF32
		error = PX_MAX_F32;
		do
		{
			old_error = error;	// preserve the old error
			// reset the counts and centroids to current cluster location
			for (physx::PxU32 i = 0; i < clumpCount; ++i)
			{
				counts[i] = 0;
				centroids[i] = physx::PxVec3(0.0f);
			}
			error = 0.0f;
			// For each input data point, figure out which cluster it is closest too and add it to that cluster.
			for (physx::PxU32 i = 0; i < inputCount; ++i)
			{
				physx::PxF32 min_distance2 = PX_MAX_F32;
				physx::PxU32 j_nearest = 0;
				// find the nearest clump to this point.
				for (physx::PxU32 j = 0; j < clumpCount; ++j)
				{
					const physx::PxF32 distance2 = (input[i]-clusters[j]).magnitudeSquared();
					if (distance2 < min_distance2)
					{
						min_distance2 = distance2;
						j_nearest = j; // save which clump this point indexes
					}
				}
				centroids[j_nearest] += input[i];
				++counts[j_nearest];	// increment the counter indicating how many points are in this clump.
				error += min_distance2; // save the error accumulation
			}
			// Now, for each clump, compute the mean and store the result.
			for (physx::PxU32 i = 0; i < clumpCount; ++i)
			{
				if (counts[i]) // if this clump got any points added to it...
				{
					centroids[i] /= (physx::PxF32)counts[i];	// compute the average center of the points in this clump.
					clusters[i] = centroids[i]; // store it as the new cluster.
				}
			}
			// decrement the convergence counter and bail if it is taking too long to converge to a solution.
			--convergeCount;
			if (convergeCount == 0)
			{
				break;
			}
			if (error < threshold) // early exit if our first guess is already good enough (if all input points are the same)
			{
				break;
			}
		} while (physx::PxAbs(error - old_error) > threshold); // keep going until the error is reduced by this threshold amount.
	}

	// ok..now we prune the clumps if necessary.
	// The rules are; first, if a clump has no 'counts' then we prune it as it's unused.
	// The second, is if the centroid of this clump is essentially  the same (based on the distance tolerance)
	// as an existing clump, then it is pruned and all indices which used to point to it, now point to the one
	// it is closest too.
	physx::PxU32 outCount = 0; // number of clumps output after pruning performed.
	physx::PxF32 d2 = collapseDistance*collapseDistance; // squared collapse distance.
	for (physx::PxU32 i = 0; i < clumpCount; ++i)
	{
		if (counts[i] == 0) // if no points ended up in this clump, eliminate it.
		{
			continue;
		}
		// see if this clump is too close to any already accepted clump.
		bool add = true;
		for (physx::PxU32 j = 0; j < outCount; ++j)
		{
			if ((clusters[i]-clusters[j]).magnitudeSquared() < d2)
			{
				add = false; // we do not add this clump
				break;
			}
		}
		if (add)
		{
			clusters[outCount++] = clusters[i];
		}
	}

	clumpCount = outCount;

	return clumpCount;
};
#endif

// barycentric utilities
void generateBarycentricCoordinatesTri(const physx::PxVec3& pa, const physx::PxVec3& pb, const physx::PxVec3& pc, const physx::PxVec3& p, physx::PxF32& s, physx::PxF32& t)
{
	// pe = pb-ba, pf = pc-pa
	// Barycentric coordinates: (s,t) -> pa + s*pe + t*pf
	// Minimize: [s*pe + t*pf - (p-pa)]^2
	// Minimize: a s^2 + 2 b s t + c t^2 + 2 d s + 2 e t + f
	// Minimization w.r.t s and t :
	//    as + bt = -d
	//    bs + ct = -e

	physx::PxVec3 pe = pb - pa;
	physx::PxVec3 pf = pc - pa;
	physx::PxVec3 pg = pa - p;

	physx::PxF32 a = pe.dot(pe);
	physx::PxF32 b = pe.dot(pf);
	physx::PxF32 c = pf.dot(pf);
	physx::PxF32 d = pe.dot(pg);
	physx::PxF32 e = pf.dot(pg);

	physx::PxF32 det = a * c - b * b;
	if (det != 0.0f)
	{
		s = (b * e - c * d) / det;
		t = (b * d - a * e) / det;
	}
}

void generateBarycentricCoordinatesTet(const physx::PxVec3& pa, const physx::PxVec3& pb, const physx::PxVec3& pc, const physx::PxVec3& pd, const physx::PxVec3& p, physx::PxVec3& bary)
{
	physx::PxVec3 q  = p - pd;
	physx::PxVec3 q0 = pa - pd;
	physx::PxVec3 q1 = pb - pd;
	physx::PxVec3 q2 = pc - pd;
	physx::PxMat33Legacy m;
	m.setColumn(0, q0);
	m.setColumn(1, q1);
	m.setColumn(2, q2);
	float det = m.determinant();
	m.setColumn(0, q);
	bary.x = m.determinant();
	m.setColumn(0, q0);
	m.setColumn(1, q);
	bary.y = m.determinant();
	m.setColumn(1, q1);
	m.setColumn(2, q);
	bary.z = m.determinant();
	if (det != 0.0f)
	{
		bary /= det;
	}
}

/* TriangleFrame */

size_t TriangleFrame::s_offsets[TriangleFrame::VertexFieldCount];

static TriangleFrameBuilder sTriangleFrameBuilder;

/*
	File-local functions and definitions
 */

struct Marker
{
	physx::PxF32	pos;
	physx::PxU32	id;	// lsb = type (0 = max, 1 = min), other bits used for object index

	void	set(physx::PxF32 _pos, physx::PxI32 _id)
	{
		pos = _pos;
		id = (physx::PxU32)_id;
	}
};

static int compareMarkers(const void* A, const void* B)
{
	// Sorts by value.  If values equal, sorts min types greater than max types, to reduce the # of overlaps
	const physx::PxF32 delta = ((Marker*)A)->pos - ((Marker*)B)->pos;
	return delta != 0 ? (delta < 0 ? -1 : 1) : ((int)(((Marker*)A)->id & 1) - (int)(((Marker*)B)->id & 1));
}

struct EdgeWithFace
{
	EdgeWithFace() {}
	EdgeWithFace(physx::PxU32 v0, physx::PxU32 v1, physx::PxU32 face, physx::PxU32 oppositeFace, bool sort = false) : faceIndex(face), v0Index(v0), v1Index(v1), opposite(oppositeFace)
	{
		if (sort)
		{
			if (v1Index < v0Index)
			{
				physx::swap(v0Index, v1Index);
			}
		}
	}

	bool operator == (const EdgeWithFace& e)
	{
		return v0Index == e.v0Index && v1Index == e.v1Index;
	}

	physx::PxU32	faceIndex;
	physx::PxU32	v0Index;
	physx::PxU32	v1Index;
	physx::PxU32	opposite;
};

/*
	Global utilities
 */

void boundsCalculateOverlaps(physx::Array<IntPair>& overlaps, Bounds3Axes axesToUse, const BoundsRep* bounds, physx::PxU32 boundsCount, physx::PxU32 boundsByteStride,
                             const BoundsInteractions& interactions, bool append)
{
	if (!append)
	{
		overlaps.reset();
	}

	physx::PxU32 D = 0;
	physx::PxU32 axisNums[3];
	for (unsigned i = 0; i < 3; ++i)
	{
		if ((axesToUse >> i) & 1)
		{
			axisNums[D++] = i;
		}
	}

	if (D == 0 || D > 3)
	{
		return;
	}

	physx::Array< physx::Array<Marker> > axes;
	axes.resize(D);
	physx::PxU32 overlapCount[3];

	for (physx::PxU32 n = 0; n < D; ++n)
	{
		const physx::PxU32 axisNum = axisNums[n];
		physx::Array<Marker>& axis = axes[n];
		overlapCount[n] = 0;
		axis.resize(2 * boundsCount);
		physx::PxU8* boundsPtr = (physx::PxU8*)bounds;
		for (physx::PxU32 i = 0; i < boundsCount; ++i, boundsPtr += boundsByteStride)
		{
			const BoundsRep& boundsRep = *(const BoundsRep*)boundsPtr;
			const physx::PxBounds3& box = boundsRep.aabb;
			physx::PxF32 min = box.minimum[axisNum];
			physx::PxF32 max = box.maximum[axisNum];
			if (min >= max)
			{
				const physx::PxF32 mid = 0.5f * (min + max);
				physx::PxF32 pad = 0.000001f * fabsf(mid);
				min = mid - pad;
				max = mid + pad;
			}
			axis[i << 1].set(min, (physx::PxI32)i << 1 | 1);
			axis[i << 1 | 1].set(max, (physx::PxI32)i << 1);
		}
		qsort(axis.begin(), axis.size(), sizeof(Marker), compareMarkers);
		physx::PxU32 localOverlapCount = 0;
		for (physx::PxU32 i = 0; i < axis.size(); ++i)
		{
			Marker& marker = axis[i];
			if (marker.id & 1)
			{
				overlapCount[n] += localOverlapCount;
				++localOverlapCount;
			}
			else
			{
				--localOverlapCount;
			}
		}
	}

	int axis0;
	int axis1;
	int axis2;
	int maxBin;
	if (D == 1)
	{
		maxBin = 0;
		axis0 = (physx::PxI32)axisNums[0];
		axis1 = axis0;
		axis2 = axis0;
	}
	else if (D == 2)
	{
		if (overlapCount[0] < overlapCount[1])
		{
			maxBin = 0;
			axis0 = (physx::PxI32)axisNums[0];
			axis1 = (physx::PxI32)axisNums[1];
			axis2 = axis0;
		}
		else
		{
			maxBin = 1;
			axis0 = (physx::PxI32)axisNums[1];
			axis1 = (physx::PxI32)axisNums[0];
			axis2 = axis0;
		}
	}
	else
	{
		maxBin = overlapCount[0] < overlapCount[1] ? (overlapCount[0] < overlapCount[2] ? 0 : 2) : (overlapCount[1] < overlapCount[2] ? 1 : 2);
		axis0 = (physx::PxI32)axisNums[maxBin];
		axis1 = (axis0 + 1) % 3;
		axis2 = (axis0 + 2) % 3;
	}

	const physx::PxU64 interactionBits = interactions.bits;

	NxIndexBank<physx::PxU32> localOverlaps(boundsCount);
	physx::Array<Marker>& axis = axes[(physx::PxU32)maxBin];
	physx::PxF32 boxMin1 = 0.0f;
	physx::PxF32 boxMax1 = 0.0f;
	physx::PxF32 boxMin2 = 0.0f;
	physx::PxF32 boxMax2 = 0.0f;

	for (physx::PxU32 i = 0; i < axis.size(); ++i)
	{
		Marker& marker = axis[i];
		const physx::PxU32 index = marker.id >> 1;
		if (marker.id & 1)
		{
			const BoundsRep& boundsRep = *(const BoundsRep*)((physx::PxU8*)bounds + index*boundsByteStride);
			const physx::PxU8 interaction = (physx::PxU8)((interactionBits >> (boundsRep.type << 3)) & 0xFF);
			const physx::PxBounds3& box = boundsRep.aabb;
			// These conditionals compile out with optimization:
			if (D > 1)
			{
				boxMin1 = box.minimum[axis1];
				boxMax1 = box.maximum[axis1];
				if (D == 3)
				{
					boxMin2 = box.minimum[axis2];
					boxMax2 = box.maximum[axis2];
				}
			}
			const physx::PxU32 localOverlapCount = localOverlaps.usedCount();
			const physx::PxU32* localOverlapIndices = localOverlaps.usedIndices();
			for (physx::PxU32 j = 0; j < localOverlapCount; ++j)
			{
				const physx::PxU32 overlapIndex = localOverlapIndices[j];
				const BoundsRep& overlapBoundsRep = *(const BoundsRep*)((physx::PxU8*)bounds + overlapIndex*boundsByteStride);
				if ((interaction >> overlapBoundsRep.type) & 1)
				{
					const physx::PxBounds3& overlapBox = overlapBoundsRep.aabb;
					// These conditionals compile out with optimization:
					if (D > 1)
					{
						if (boxMin1 >= overlapBox.maximum[axis1] || boxMax1 <= overlapBox.minimum[axis1])
						{
							continue;
						}
						if (D == 3)
						{
							if (boxMin2 >= overlapBox.maximum[axis2] || boxMax2 <= overlapBox.minimum[axis2])
							{
								continue;
							}
						}
					}
					// Add overlap
					IntPair& pair = overlaps.insert();
					pair.i0 = (physx::PxI32)index;
					pair.i1 = (physx::PxI32)overlapIndex;
				}
			}
			PX_ASSERT(localOverlaps.isValid(index));
			PX_ASSERT(!localOverlaps.isUsed(index));
			localOverlaps.use(index);
		}
		else
		{
			// Remove local overlap
			PX_ASSERT(localOverlaps.isValid(index));
			localOverlaps.free(index);
		}
	}
}


/*
	ConvexHull functions
 */

ConvexHull::ConvexHull() : mParams(NULL), mOwnsParams(false)
{
}

ConvexHull::~ConvexHull()
{
	term();
}

// If params == NULL, ConvexHull will build (and own) its own internal parameters
void ConvexHull::init(NxParameterized::Interface* params)
{
	term();

	if (params != NULL)
	{
		mParams = DYNAMIC_CAST(ConvexHullParameters*)(params);
		if (mParams == NULL)
		{
			PX_ASSERT(!"params must be ConvexHullParameters");
		}
	}
	else
	{
		NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();
		mParams = DYNAMIC_CAST(ConvexHullParameters*)(traits->createNxParameterized(ConvexHullParameters::staticClassName()));
		PX_ASSERT(mParams != NULL);
		if (mParams != NULL)
		{
			mOwnsParams = true;
			setEmpty();
		}
	}
}


NxParameterized::Interface*	ConvexHull::giveOwnersipOfNxParameters()
{
	if (mOwnsParams)
	{
		mOwnsParams = false;
		return mParams;
	}

	return NULL;
}

// Releases parameters if it owns them
void ConvexHull::term()
{
	if (mParams != NULL && mOwnsParams)
	{
		mParams->destroy();
	}
	mParams = NULL;
	mOwnsParams = false;
}

static int compareEdgesWithFaces(const void* A, const void* B)
{
	const EdgeWithFace& eA = *(const EdgeWithFace*)A;
	const EdgeWithFace& eB = *(const EdgeWithFace*)B;
	if (eA.v0Index != eB.v0Index)
	{
		return (int)eA.v0Index - (int)eB.v0Index;
	}
	if (eA.v1Index != eB.v1Index)
	{
		return (int)eA.v1Index - (int)eB.v1Index;
	}
	return (int)eA.faceIndex - (int)eB.faceIndex;
}

void ConvexHull::buildFromDesc(const ConvexHullDesc& desc)
{
	if (!desc.isValid())
	{
		setEmpty();
		return;
	}

	setEmpty();

	Array<physx::PxPlane>	uniquePlanes;
	Array<PxU32>	edges;
	Array<PxF32>	widths;

	NxParameterized::Handle handle(*mParams);

	// Copy vertices and compute bounds
	mParams->getParameterHandle("vertices", handle);
	mParams->resizeArray(handle, (physx::PxI32)desc.numVertices);
	const physx::PxU8* vertPointer = (const physx::PxU8*)desc.vertices;
	for (physx::PxU32 i = 0; i < desc.numVertices; ++i, vertPointer += desc.vertexStrideBytes)
	{
		mParams->vertices.buf[i] = *(const physx::PxVec3*)vertPointer;
		mParams->bounds.include(mParams->vertices.buf[i]);
	}

	physx::PxVec3 center;
	center = mParams->bounds.getCenter();

	// Find faces and compute volume
	physx::Array<EdgeWithFace> fEdges;
	const physx::PxU32* indices = desc.indices;
	mParams->volume = 0.0f;
	for (physx::PxU32 fI = 0; fI < desc.numFaces; ++fI)
	{
		const physx::PxU32 indexCount = (const physx::PxU32)desc.faceIndexCounts[fI];
		const physx::PxVec3& vI0 = mParams->vertices.buf[indices[0]];
		physx::PxVec3 normal(0.0f);
		for (physx::PxU32 pI = 1; pI < indexCount-1; ++pI)
		{
			const physx::PxU32 i1 = indices[pI];
			const physx::PxU32 i2 = indices[pI+1];
			const physx::PxVec3& vI1 = mParams->vertices.buf[i1];
			const physx::PxVec3& vI2 = mParams->vertices.buf[i2];
			const physx::PxVec3 eI1 = vI1 - vI0;
			const physx::PxVec3 eI2 = vI2 - vI0;
			physx::PxVec3 normalI = eI1.cross(eI2);
			mParams->volume += (normalI.dot(vI0 - center));
			normal += normalI;
		}

		const bool normalValid = normal.normalize() > 0.0f;

		physx::PxF32 d = 0.0f;
		for (physx::PxU32 pI = 0; pI < indexCount; ++pI)
		{
			d -= normal.dot(mParams->vertices.buf[indices[pI]]);
		}
		d /= indexCount;

		physx::PxU32 oppositeFace = 0;
		physx::PxU32 faceN = uniquePlanes.size();	// unless we find an opposite

		if (normalValid)
		{
			// See if this face is opposite an existing one
			for (physx::PxU32 j = 0; j < uniquePlanes.size(); ++j)
			{
				if (normal.dot(uniquePlanes[j].n) < -0.999999f && widths[j] == PX_MAX_F32)
				{
					oppositeFace = 1;
					faceN = j;
					widths[j] = -d - uniquePlanes[j].d;
					break;
				}
			}
		}

		if (!oppositeFace)
		{
			uniquePlanes.pushBack(physx::PxPlane(normal, d));
			widths.pushBack(PX_MAX_F32);
		}

		for (physx::PxU32 pI = 0; pI < indexCount; ++pI)
		{
			fEdges.pushBack(EdgeWithFace(indices[pI], indices[(pI+1)%indexCount], faceN, oppositeFace, true));
		}

		indices += indexCount;
	}

	// Create a map from current plane indices to the arrangement we'll have after the following loop
	// This map will be its inverse
	physx::Array<physx::PxU32> invMap;
	invMap.resize(2*widths.size());
	for (physx::PxU32 i = 0; i < invMap.size(); ++i)
	{
		invMap[i] = i;
	}

	// This ensures that all the slabs are represented first in the planes list
	physx::PxU32 slabCount = widths.size();
	for (physx::PxU32 i = widths.size(); i--;)
	{
		if (widths[i] == PX_MAX_F32)
		{
			--slabCount;
			physx::swap(widths[i], widths[slabCount]);
			physx::swap(uniquePlanes[i], uniquePlanes[slabCount]);
			physx::swap(invMap[i], invMap[slabCount]);
		}
	}

	// Now invert invMap to get the map
	physx::Array<physx::PxU32> map;
	map.resize(invMap.size());
	for (physx::PxU32 i = 0; i < map.size(); ++i)
	{
		map[invMap[i]] = i;
	}

	// Plane indices have been rearranged, need to make corresponding rearrangements to fEdges' faceIndex value.
	for (physx::PxU32 i = 0; i < fEdges.size(); ++i)
	{
		fEdges[i].faceIndex = map[fEdges[i].faceIndex];
		if (fEdges[i].opposite)
		{
			fEdges[i].faceIndex += uniquePlanes.size();
		}
	}

	// Total plane count, with non-unique (back-facing) normals
	mParams->planeCount = uniquePlanes.size() + slabCount;

	// Fill in remaining widths
	for (physx::PxU32 fI = mParams->planeCount - uniquePlanes.size(); fI < widths.size(); ++fI)
	{
		widths[fI] = 0.0f;
		for (physx::PxU32 vI = 0; vI < desc.numVertices; ++vI)
		{
			const physx::PxF32 vDepth = -uniquePlanes[fI].distance(mParams->vertices.buf[vI]);
			if (vDepth > widths[fI])
			{
				widths[fI] = vDepth;
			}
		}
	}

	// Record faceIndex pairs in adjacentFaces
	physx::Array<physx::PxU32> adjacentFaces;

	// Eliminate redundant edges
	qsort(fEdges.begin(), fEdges.size(), sizeof(EdgeWithFace), compareEdgesWithFaces);
	for (physx::PxU32 eI = 0; eI < fEdges.size();)
	{
		physx::PxU32 i0 = fEdges[eI].v0Index;
		physx::PxU32 i1 = fEdges[eI].v1Index;
		PX_ASSERT(i0 < 65536 && i1 < 65536);
		if (eI < fEdges.size() - 1 && fEdges[eI] == fEdges[eI + 1])
		{
			if (fEdges[eI].faceIndex != fEdges[eI + 1].faceIndex)
			{
				edges.pushBack(i0 << 16 | i1);
				adjacentFaces.pushBack(fEdges[eI].faceIndex << 16 | fEdges[eI + 1].faceIndex);
			}
			eI += 2;
		}
		else
		{
			edges.pushBack(i0 << 16 | i1);
			adjacentFaces.pushBack(0xFFFF0000 | fEdges[eI].faceIndex);
			++eI;
		}
	}

	// Find unique edge directions, put them at the front of the edge list
	const physx::PxU32 edgeCount = edges.size();
	if (edgeCount > 0)
	{
		mParams->uniqueEdgeDirectionCount = 1;
		for (physx::PxU32 testEdgeIndex = 1; testEdgeIndex < edgeCount; ++testEdgeIndex)
		{
			const physx::PxU32 testEdgeEndpointIndices = edges[testEdgeIndex];
			const physx::PxVec3 testEdge = mParams->vertices.buf[testEdgeEndpointIndices & 0xFFFF] - mParams->vertices.buf[testEdgeEndpointIndices >> 16];
			const physx::PxF32 testEdge2 = testEdge.magnitudeSquared();
			physx::PxU32 uniqueEdgeIndex = 0;
			for (; uniqueEdgeIndex < mParams->uniqueEdgeDirectionCount; ++uniqueEdgeIndex)
			{
				const physx::PxU32 uniqueEdgeEndpointIndices = edges[uniqueEdgeIndex];
				const physx::PxVec3 uniqueEdge = mParams->vertices.buf[uniqueEdgeEndpointIndices & 0xFFFF] - mParams->vertices.buf[uniqueEdgeEndpointIndices >> 16];
				if (uniqueEdge.cross(testEdge).magnitudeSquared() < testEdge2 * uniqueEdge.magnitudeSquared()*PX_EPS_F32 * PX_EPS_F32)
				{
					break;
				}
			}
			if (uniqueEdgeIndex == mParams->uniqueEdgeDirectionCount)
			{
				physx::swap(edges[mParams->uniqueEdgeDirectionCount], edges[testEdgeIndex]);
				physx::swap(adjacentFaces[mParams->uniqueEdgeDirectionCount], adjacentFaces[testEdgeIndex]);
				++mParams->uniqueEdgeDirectionCount;
			}
		}
	}

	mParams->volume *= 0.1666666667f;

	// Transfer from temporary arrays into mParams

	// uniquePlanes
	mParams->getParameterHandle("uniquePlanes", handle);
	mParams->resizeArray(handle, (physx::PxI32)uniquePlanes.size());
	for (physx::PxU32 i = 0; i < uniquePlanes.size(); ++i)
	{
		physx::PxPlane& plane = uniquePlanes[i];
		ConvexHullParametersNS::Plane_Type& paramPlane = mParams->uniquePlanes.buf[i];
		paramPlane.normal = plane.n;
		paramPlane.d = plane.d;
	}

	// edges
	mParams->getParameterHandle("edges", handle);
	mParams->resizeArray(handle, (physx::PxI32)edges.size());
	mParams->setParamU32Array(handle, edges.begin(), (physx::PxI32)edges.size());

	// adjacentFaces
	mParams->getParameterHandle("adjacentFaces", handle);
	mParams->resizeArray(handle, (physx::PxI32)adjacentFaces.size());
	mParams->setParamU32Array(handle, adjacentFaces.begin(), (physx::PxI32)adjacentFaces.size());

	// widths
	mParams->getParameterHandle("widths", handle);
	mParams->resizeArray(handle, (physx::PxI32)widths.size());
	mParams->setParamF32Array(handle, widths.begin(), (physx::PxI32)widths.size());

}

void ConvexHull::buildFromPoints(const void* points, physx::PxU32 numPoints, physx::PxU32 pointStrideBytes)
{
	if (numPoints < 4)
	{
		setEmpty();
		return;
	}

	// First try stanhull

	// We will transform the points of the hull such that they fit well into a rotated 2x2x2 cube.
	// To find a suitable set of axes for this rotated cube, we will find the  of the point distribution.

	// Create an array of points which we will transform
	physx::Array<physx::PxVec3> transformedPoints(numPoints);
	physx::PxU8* pointPtr = (physx::PxU8*)points;
	for (physx::PxU32 i = 0; i < numPoints; ++i, pointPtr += pointStrideBytes)
	{
		transformedPoints[i] = *(physx::PxVec3*)pointPtr;
	}

	// Optionally reduce the point set
#if REDUCE_CONVEXHULL_INPUT_POINT_SET
	const physx::PxU32 maxSetSize = 400;
	physx::Array<physx::PxVec3> reducedPointSet(numPoints);
	const physx::PxU32 reducedSetSize = kmeans_cluster(&transformedPoints[0], numPoints, &reducedPointSet[0], maxSetSize);
	PX_ASSERT(reducedSetSize <= 400 && reducedSetSize >= 4);
	transformedPoints = reducedPointSet;
#endif // REDUCE_CONVEXHULL_INPUT_POINT_SET

	physx::PxVec3 mean;
	physx::PxVec3 variance;
	physx::PxMat33 axes;
	findPrincipleComponents(mean, variance, axes, &transformedPoints[0], numPoints);

	// Now subtract the mean from the points and rotate the points into the frame of the axes
	for (physx::PxU32 i = 0; i < numPoints; ++i)
	{
		transformedPoints[i] = axes.transformTranspose(transformedPoints[i] - mean);
	}

	// Finally find a scale such that the maximum absolute coordinate on each axis is 1
	physx::PxVec3 scale(0.0f);
	for (physx::PxU32 i = 0; i < numPoints; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			scale[j] = physx::PxMax(scale[j], physx::PxAbs(transformedPoints[i][j]));
		}
	}
	if (scale.x*scale.y*scale.z == 0.0f)
	{
		// We have a degeneracy, planar (or colinear or coincident) points
		setEmpty();
		return;
	}

	// Now scale the points
	const physx::PxVec3 recipScale(1.0f/scale.x, 1.0f/scale.y, 1.0f/scale.z);
	for (physx::PxU32 i = 0; i < numPoints; ++i)
	{
		transformedPoints[i] = transformedPoints[i].multiply(recipScale);
	}

	// The points are transformed, and for completeness let us now build the transform which restores them
	const physx::PxMat44 tm(axes.column0*scale.x, axes.column1*scale.y, axes.column2*scale.z, mean);

#if 0
	// Now find the convex hull of the transformed points
	physx::stanhull::HullDesc hullDesc;
	hullDesc.mVertices = (const physx::PxF32*)&transformedPoints[0];
	hullDesc.mVcount = numPoints;
	hullDesc.mVertexStride = sizeof(physx::PxVec3);
	hullDesc.mSkinWidth = 0.0f;

	physx::stanhull::HullLibrary hulllib;
	physx::stanhull::HullResult hull;
	if (physx::stanhull::QE_OK == hulllib.CreateConvexHull(hullDesc, hull))
	{	// Success
		ConvexHullDesc desc;
		desc.vertices = hull.mOutputVertices;
		desc.numVertices = hull.mNumOutputVertices;
		desc.vertexStrideBytes = 3*sizeof(physx::PxF32);
		desc.indices = hull.mIndices;
		desc.numIndices = hull.mNumIndices;
		desc.faceIndexCounts = hull.mFaces;
		desc.numFaces = hull.mNumFaces;
		buildFromDesc(desc);
		hulllib.ReleaseResult(hull);

		// Transform our hull back before returning
		applyTransformation(tm);

		return;
	}
#endif

	// Stanhull failed, try physx sdk cooker
	NxApexSDK* sdk = NxGetApexSDK();
	if (sdk && sdk->getCookingInterface() && sdk->getPhysXSDK())
	{
#if NX_SDK_VERSION_MAJOR == 2
		NxConvexMeshDesc convexMeshDesc;
		convexMeshDesc.numVertices = numPoints;
		convexMeshDesc.points = &transformedPoints[0];
		convexMeshDesc.pointStrideBytes = pointStrideBytes;
		convexMeshDesc.flags = NX_CF_COMPUTE_CONVEX;

		PxMemoryBuffer stream;
		stream.setEndianMode(physx::PxFileBuf::ENDIAN_NONE);
		NxFromPxStream nxs(stream);
		if (sdk->getCookingInterface()->NxCookConvexMesh(convexMeshDesc, nxs))
		{
			NxConvexMesh* mesh = sdk->getPhysXSDK()->createConvexMesh(nxs);
			if (mesh)
			{
				buildFromConvexMesh(mesh);
				sdk->getPhysXSDK()->releaseConvexMesh(*mesh);
				// Transform our hull back before returning
				applyTransformation(tm);
				return;
			}
		}
#elif NX_SDK_VERSION_MAJOR == 3
		PxConvexMeshDesc convexMeshDesc;
		convexMeshDesc.points.count = numPoints;
		convexMeshDesc.points.data = &transformedPoints[0];
		convexMeshDesc.points.stride = pointStrideBytes;
		convexMeshDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

		PxMemoryBuffer stream;
		stream.setEndianMode(physx::PxFileBuf::ENDIAN_NONE);
		NxFromPxStream nxs(stream);
		if (sdk->getCookingInterface()->cookConvexMesh(convexMeshDesc, nxs))
		{
			PxConvexMesh* mesh = sdk->getPhysXSDK()->createConvexMesh(nxs);
			if (mesh)
			{
				buildFromConvexMesh(mesh);
				mesh->release();
				// Transform our hull back before returning
				applyTransformation(tm);
				return;
			}
		}
#endif
	}

	// No luck
	setEmpty();
}

void ConvexHull::buildFromPlanes(const physx::PxPlane* planes, physx::PxU32 numPlanes, physx::PxF32 eps)
{
	physx::Array<physx::PxVec3> points;
	points.reserve((numPlanes * (numPlanes - 1) * (numPlanes - 2)) / 6);
	for (physx::PxU32 i = 0; i < numPlanes; ++i)
	{
		const physx::PxPlane& planeI = planes[i];
		for (physx::PxU32 j = i + 1; j < numPlanes; ++j)
		{
			const physx::PxPlane& planeJ = planes[j];
			for (physx::PxU32 k = j + 1; k < numPlanes; ++k)
			{
				const physx::PxPlane& planeK = planes[k];
				physx::PxVec3 point;
				if (intersectPlanes(point, planeI, planeJ, planeK))
				{
					if (pointInsidePlanes(point, planes, i, eps) &&
					        pointInsidePlanes(point, planes + i + 1, j - (i + 1), eps) &&
					        pointInsidePlanes(point, planes + j + 1, k - (j + 1), eps) &&
					        pointInsidePlanes(point, planes + k + 1, numPlanes - (k + 1), eps))
					{
						physx::PxU32 m = 0;
						for (; m < points.size(); ++m)
						{
							if ((point - points[m]).magnitudeSquared() < eps)
							{
								break;
							}
						}
						if (m == points.size())
						{
							points.pushBack(point);
						}
					}
				}
			}
		}
	}

	buildFromPoints(points.begin(), points.size(), sizeof(physx::PxVec3));
}

#if NX_SDK_VERSION_MAJOR == 2
void ConvexHull::buildFromConvexMesh(const ConvexMesh* mesh)
{
	setEmpty();

	if (!mesh)
	{
		return;
	}

	PX_ASSERT(mesh->getFormat(0, NX_ARRAY_TRIANGLES) == NX_FORMAT_INT);
	PX_ASSERT(mesh->getStride(0, NX_ARRAY_TRIANGLES) == sizeof(physx::PxU32) * 3);
	PX_ASSERT(mesh->getStride(0, NX_ARRAY_VERTICES) == sizeof(physx::PxVec3));

	const physx::PxU32 numVerts = mesh->getCount(0, NX_ARRAY_VERTICES);
	const physx::PxVec3* verts = (const physx::PxVec3*)mesh->getBase(0, NX_ARRAY_VERTICES);
	const physx::PxU32 numTris = mesh->getCount(0, NX_ARRAY_TRIANGLES);
	const physx::PxU32* index = (const physx::PxU32*)mesh->getBase(0, NX_ARRAY_TRIANGLES);

	Array<physx::PxPlane>	uniquePlanes;
	Array<PxU32>	edges;
	Array<PxF32>	widths;

	NxParameterized::Handle handle(*mParams);

	// Copy vertices and compute bounds
	mParams->getParameterHandle("vertices", handle);
	mParams->resizeArray(handle, numVerts);
	for (physx::PxU32 i = 0; i < numVerts; ++i)
	{
		mParams->vertices.buf[i] = verts[i];
		mParams->bounds.include(verts[i]);
	}

	physx::PxVec3 center;
	center = mParams->bounds.getCenter();
	mParams->volume = 0.0f;

	// Find planes and compute volume
	physx::Array<EdgeWithFace> fEdges;
	const physx::PxU32 numIndices = 3 * numTris;
	for (physx::PxU32 vI = 0; vI < numIndices; vI += 3)
	{
		const physx::PxVec3& vI0 = verts[index[vI]];
		const physx::PxVec3& vI1 = verts[index[vI + 1]];
		const physx::PxVec3& vI2 = verts[index[vI + 2]];
		const physx::PxVec3 eI1 = vI1 - vI0;
		const physx::PxVec3 eI2 = vI2 - vI0;
		physx::PxVec3 normalI = eI1.cross(eI2);
		mParams->volume += (normalI.dot(vI0 - center));
		if (normalI.magnitudeSquared() > 0.0f)
		{
			normalI.normalize();
			physx::PxU32 j;
			physx::PxU32 oppositeFace = 0;
			for (j = 0; j < uniquePlanes.size(); ++j)
			{
				const physx::PxF32 cosTheta = normalI.dot(uniquePlanes[j].n);
				oppositeFace = (physx::PxU32)(cosTheta < 0.0f);
				if (cosTheta * cosTheta > 0.999999f)
				{
					break;
				}
			}
			if (j == uniquePlanes.size())
			{
				uniquePlanes.pushBack(physx::PxPlane(vI0, normalI));
				widths.pushBack(PX_MAX_F32);
				oppositeFace = 0;
			}
			else if (oppositeFace && widths[j] == PX_MAX_F32)
			{
				// New slab
				widths[j] = -uniquePlanes[j].distance(vI0);
			}
			fEdges.pushBack(EdgeWithFace(index[vI], index[vI + 1], j, oppositeFace, true));
			fEdges.pushBack(EdgeWithFace(index[vI + 1], index[vI + 2], j, oppositeFace, true));
			fEdges.pushBack(EdgeWithFace(index[vI + 2], index[vI], j, oppositeFace, true));
		}
	}

	// Create a map from current plane indices to the arrangement we'll have after the following loop
	// This map will be its inverse
	physx::Array<physx::PxU32> invMap;
	invMap.resize(2*widths.size());
	for (physx::PxU32 i = 0; i < invMap.size(); ++i)
	{
		invMap[i] = i;
	}

	// This ensures that all the slabs are represented first in the planes list
	physx::PxU32 slabCount = widths.size();
	for (physx::PxU32 i = widths.size(); i--;)
	{
		if (widths[i] == PX_MAX_F32)
		{
			--slabCount;
			physx::swap(widths[i], widths[slabCount]);
			physx::swap(uniquePlanes[i], uniquePlanes[slabCount]);
			physx::swap(invMap[i], invMap[slabCount]);
		}
	}

	// Now invert invMap to get the map
	physx::Array<physx::PxU32> map;
	map.resize(invMap.size());
	for (physx::PxU32 i = 0; i < map.size(); ++i)
	{
		map[invMap[i]] = i;
	}

	// Plane indices have been rearranged, need to make corresponding rearrangements to fEdges' faceIndex value.
	for (physx::PxU32 i = 0; i < fEdges.size(); ++i)
	{
		fEdges[i].faceIndex = map[fEdges[i].faceIndex];
		if (fEdges[i].opposite)
		{
			fEdges[i].faceIndex += uniquePlanes.size();
		}
	}

	// Total plane count, with non-unique (back-facing) normals
	mParams->planeCount = uniquePlanes.size() + slabCount;

	// Fill in remaining widths
	for (physx::PxU32 pI = mParams->planeCount - uniquePlanes.size(); pI < widths.size(); ++pI)
	{
		widths[pI] = 0.0f;
		for (physx::PxU32 vI = 0; vI < numIndices; ++vI)
		{
			const physx::PxF32 vDepth = -uniquePlanes[pI].distance(verts[index[vI]]);
			if (vDepth > widths[pI])
			{
				widths[pI] = vDepth;
			}
		}
	}

	// Record faceIndex pairs in adjacentFaces
	physx::Array<physx::PxU32> adjacentFaces;

	// Eliminate redundant edges
	qsort(fEdges.begin(), fEdges.size(), sizeof(EdgeWithFace), compareEdgesWithFaces);
	for (physx::PxU32 eI = 0; eI < fEdges.size();)
	{
		physx::PxU32 i0 = fEdges[eI].v0Index;
		physx::PxU32 i1 = fEdges[eI].v1Index;
		PX_ASSERT(i0 < 65536 && i1 < 65536);
		if (eI < fEdges.size() - 1 && fEdges[eI] == fEdges[eI + 1])
		{
			if (fEdges[eI].faceIndex != fEdges[eI + 1].faceIndex)
			{
				edges.pushBack(i0 << 16 | i1);
				adjacentFaces.pushBack(fEdges[eI].faceIndex << 16 | fEdges[eI + 1].faceIndex);
			}
			eI += 2;
		}
		else
		{
			edges.pushBack(i0 << 16 | i1);
			adjacentFaces.pushBack(0xFFFF0000 | fEdges[eI].faceIndex);
			++eI;
		}
	}

	// Find unique edge directions, put them at the front of the edge list
	const physx::PxU32 edgeCount = edges.size();
	if (edgeCount > 0)
	{
		mParams->uniqueEdgeDirectionCount = 1;
		for (physx::PxU32 testEdgeIndex = 1; testEdgeIndex < edgeCount; ++testEdgeIndex)
		{
			const physx::PxU32 testEdgeEndpointIndices = edges[testEdgeIndex];
			const physx::PxVec3 testEdge = mParams->vertices.buf[testEdgeEndpointIndices & 0xFFFF] - mParams->vertices.buf[testEdgeEndpointIndices >> 16];
			const physx::PxF32 testEdge2 = testEdge.magnitudeSquared();
			physx::PxU32 uniqueEdgeIndex = 0;
			for (; uniqueEdgeIndex < mParams->uniqueEdgeDirectionCount; ++uniqueEdgeIndex)
			{
				const physx::PxU32 uniqueEdgeEndpointIndices = edges[uniqueEdgeIndex];
				const physx::PxVec3 uniqueEdge = mParams->vertices.buf[uniqueEdgeEndpointIndices & 0xFFFF] - mParams->vertices.buf[uniqueEdgeEndpointIndices >> 16];
				if (uniqueEdge.cross(testEdge).magnitudeSquared() < testEdge2 * uniqueEdge.magnitudeSquared()*PX_EPS_F32 * PX_EPS_F32)
				{
					break;
				}
			}
			if (uniqueEdgeIndex == mParams->uniqueEdgeDirectionCount)
			{
				physx::swap(edges[mParams->uniqueEdgeDirectionCount], edges[testEdgeIndex]);
				physx::swap(adjacentFaces[mParams->uniqueEdgeDirectionCount], adjacentFaces[testEdgeIndex]);
				++mParams->uniqueEdgeDirectionCount;
			}
		}
	}

	mParams->volume *= 0.1666666667f;

	// Transfer from temporary arrays into mParams

	// uniquePlanes
	mParams->getParameterHandle("uniquePlanes", handle);
	mParams->resizeArray(handle, uniquePlanes.size());
	for (physx::PxU32 i = 0; i < uniquePlanes.size(); ++i)
	{
		physx::PxPlane& plane = uniquePlanes[i];
		ConvexHullParametersNS::Plane_Type& paramPlane = mParams->uniquePlanes.buf[i];
		paramPlane.normal = plane.n;
		paramPlane.d = plane.d;
	}

	// edges
	mParams->getParameterHandle("edges", handle);
	mParams->resizeArray(handle, edges.size());
	mParams->setParamU32Array(handle, edges.begin(), edges.size());

	// adjacentFaces
	mParams->getParameterHandle("adjacentFaces", handle);
	mParams->resizeArray(handle, adjacentFaces.size());
	mParams->setParamU32Array(handle, adjacentFaces.begin(), adjacentFaces.size());

	// widths
	mParams->getParameterHandle("widths", handle);
	mParams->resizeArray(handle, widths.size());
	mParams->setParamF32Array(handle, widths.begin(), widths.size());
}
#elif NX_SDK_VERSION_MAJOR == 3


void ConvexHull::buildFromConvexMesh(const ConvexMesh* mesh)
{
	setEmpty();

	if (!mesh)
	{
		return;
	}

	const physx::PxU32 numVerts = mesh->getNbVertices();
	const physx::PxVec3* verts = (const physx::PxVec3*)mesh->getVertices();
	const physx::PxU32 numPolygons = mesh->getNbPolygons();
	const physx::PxU8* index = (const physx::PxU8*)mesh->getIndexBuffer();


	Array<physx::PxPlane>	uniquePlanes;
	Array<PxU32>	edges;
	Array<PxF32>	widths;

	NxParameterized::Handle handle(*mParams);

	// Copy vertices and compute bounds
	mParams->getParameterHandle("vertices", handle);
	mParams->resizeArray(handle, (physx::PxI32)numVerts);
	for (physx::PxU32 i = 0; i < numVerts; ++i)
	{
		mParams->vertices.buf[i] = verts[i];
		mParams->bounds.include(verts[i]);
	}

	physx::PxVec3 center;
	center = mParams->bounds.getCenter();

	PxMat33 localInertia;
	PxVec3 localCenterOfMass;
	mesh->getMassInformation(mParams->volume, localInertia, localCenterOfMass);

	// Find planes and compute volume
	physx::Array<EdgeWithFace> fEdges;

	for (PxU32 i = 0; i < numPolygons; i++)
	{
		PxHullPolygon data;
		bool status = mesh->getPolygonData(i, data);
		PX_ASSERT(status);
		if (!status)
		{
			continue;
		}

		const physx::PxU32 numIndices = data.mNbVerts;
		for (physx::PxU32 vI = 0; vI < numIndices; ++vI)
		{
			const physx::PxU16 i0 = (physx::PxU16)index[data.mIndexBase + vI];
			const physx::PxU16 i1 = (physx::PxU16)index[data.mIndexBase + ((vI+1)%numIndices)];
			const physx::PxVec3& vI0 = verts[i0];
			const physx::PxVec3 normal(data.mPlane[0], data.mPlane[1], data.mPlane[2]);
			PX_ASSERT(physx::PxEquals(normal.magnitudeSquared(), 1.0f, 0.000001f));
			physx::PxU32 faceIndex;
			physx::PxU32 oppositeFace = 0;
			for (faceIndex = 0; faceIndex < uniquePlanes.size(); ++faceIndex)
			{
				const physx::PxF32 cosTheta = normal.dot(uniquePlanes[faceIndex].n);
				oppositeFace = (physx::PxU32)(cosTheta < 0.0f);
				if (cosTheta * cosTheta > 0.999999f)
				{
					break;
				}
			}
			if (faceIndex == uniquePlanes.size())
			{
				uniquePlanes.pushBack(physx::PxPlane(vI0, normal));
				widths.pushBack(PX_MAX_F32);
				oppositeFace = 0;
			}
			else if (oppositeFace && widths[faceIndex] == PX_MAX_F32)
			{
				// New slab
				widths[faceIndex] = -uniquePlanes[faceIndex].distance(vI0);
			}
			fEdges.pushBack(EdgeWithFace(i0, i1, faceIndex, oppositeFace, true));
		}
	}

	// Create a map from current plane indices to the arrangement we'll have after the following loop
	// This map will be its inverse
	physx::Array<physx::PxU32> invMap;
	invMap.resize(2*widths.size());
	for (physx::PxU32 i = 0; i < invMap.size(); ++i)
	{
		invMap[i] = i;
	}

	// This ensures that all the slabs are represented first in the planes list
	physx::PxU32 slabCount = widths.size();
	for (physx::PxU32 i = widths.size(); i--;)
	{
		if (widths[i] == PX_MAX_F32)
		{
			--slabCount;
			physx::swap(widths[i], widths[slabCount]);
			physx::swap(uniquePlanes[i], uniquePlanes[slabCount]);
			physx::swap(invMap[i], invMap[slabCount]);
		}
	}

	// Now invert invMap to get the map
	physx::Array<physx::PxU32> map;
	map.resize(invMap.size());
	for (physx::PxU32 i = 0; i < map.size(); ++i)
	{
		map[invMap[i]] = i;
	}

	// Plane indices have been rearranged, need to make corresponding rearrangements to fEdges' faceIndex value.
	for (physx::PxU32 i = 0; i < fEdges.size(); ++i)
	{
		fEdges[i].faceIndex = map[fEdges[i].faceIndex];
		if (fEdges[i].opposite)
		{
			fEdges[i].faceIndex += uniquePlanes.size();
		}
	}

	// Total plane count, with non-unique (back-facing) normals
	mParams->planeCount = uniquePlanes.size() + slabCount;

	// Fill in remaining widths
	for (physx::PxU32 pI = mParams->planeCount - uniquePlanes.size(); pI < widths.size(); ++pI)
	{
		widths[pI] = 0.0f;
		for (PxU32 i = 0; i < numPolygons; i++)
		{
			PxHullPolygon data;
			bool status = mesh->getPolygonData(i, data);
			PX_ASSERT(status);
			if (!status)
			{
				continue;
			}

			const physx::PxU32 numIndices = (physx::PxU32)data.mNbVerts - 2;
			for (physx::PxU32 vI = 0; vI < numIndices; ++vI)
			{
				const physx::PxF32 vDepth = -uniquePlanes[pI].distance(verts[index[data.mIndexBase + vI]]);
				if (vDepth > widths[pI])
				{
					widths[pI] = vDepth;
				}
			}
		}
	}

	// Record faceIndex pairs in adjacentFaces
	physx::Array<physx::PxU32> adjacentFaces;

	// Eliminate redundant edges
	qsort(fEdges.begin(), fEdges.size(), sizeof(EdgeWithFace), compareEdgesWithFaces);
	for (physx::PxU32 eI = 0; eI < fEdges.size();)
	{
		physx::PxU32 i0 = fEdges[eI].v0Index;
		physx::PxU32 i1 = fEdges[eI].v1Index;
		PX_ASSERT(i0 < 65536 && i1 < 65536);
		if (eI < fEdges.size() - 1 && fEdges[eI] == fEdges[eI + 1])
		{
			if (fEdges[eI].faceIndex != fEdges[eI + 1].faceIndex)
			{
				edges.pushBack(i0 << 16 | i1);
				adjacentFaces.pushBack(fEdges[eI].faceIndex << 16 | fEdges[eI + 1].faceIndex);
			}
			eI += 2;
		}
		else
		{
			edges.pushBack(i0 << 16 | i1);
			adjacentFaces.pushBack(0xFFFF0000 | fEdges[eI].faceIndex);
			++eI;
		}
	}

	// Find unique edge directions, put them at the front of the edge list
	const physx::PxU32 edgeCount = edges.size();
	if (edgeCount > 0)
	{
		mParams->uniqueEdgeDirectionCount = 1;
		for (physx::PxU32 testEdgeIndex = 1; testEdgeIndex < edgeCount; ++testEdgeIndex)
		{
			const physx::PxVec3 testEdge = verts[edges[testEdgeIndex] & 0xFFFF] - verts[edges[testEdgeIndex] >> 16];
			const physx::PxF32 testEdge2 = testEdge.magnitudeSquared();
			physx::PxU32 uniqueEdgeIndex = 0;
			for (; uniqueEdgeIndex < mParams->uniqueEdgeDirectionCount; ++uniqueEdgeIndex)
			{
				const physx::PxVec3 uniqueEdge = verts[edges[uniqueEdgeIndex] & 0xFFFF] - verts[edges[uniqueEdgeIndex] >> 16];
				if (uniqueEdge.cross(testEdge).magnitudeSquared() < testEdge2 * uniqueEdge.magnitudeSquared()*PX_EPS_F32 * PX_EPS_F32)
				{
					break;
				}
			}
			if (uniqueEdgeIndex == mParams->uniqueEdgeDirectionCount)
			{
				physx::swap(edges[mParams->uniqueEdgeDirectionCount], edges[testEdgeIndex]);
				physx::swap(adjacentFaces[mParams->uniqueEdgeDirectionCount], adjacentFaces[testEdgeIndex]);
				++mParams->uniqueEdgeDirectionCount;
			}
		}
	}

	// Transfer from temporary arrays into mParams

	// uniquePlanes
	mParams->getParameterHandle("uniquePlanes", handle);
	mParams->resizeArray(handle, (physx::PxI32)uniquePlanes.size());
	for (physx::PxU32 i = 0; i < uniquePlanes.size(); ++i)
	{
		physx::PxPlane& plane = uniquePlanes[i];
		ConvexHullParametersNS::Plane_Type& paramPlane = mParams->uniquePlanes.buf[i];
		paramPlane.normal = plane.n;
		paramPlane.d = plane.d;
	}

	// edges
	mParams->getParameterHandle("edges", handle);
	mParams->resizeArray(handle, (physx::PxI32)edges.size());
	mParams->setParamU32Array(handle, edges.begin(), (physx::PxI32)edges.size());

	// adjacentFaces
	mParams->getParameterHandle("adjacentFaces", handle);
	mParams->resizeArray(handle, (physx::PxI32)adjacentFaces.size());
	mParams->setParamU32Array(handle, adjacentFaces.begin(), (physx::PxI32)adjacentFaces.size());

	// widths
	mParams->getParameterHandle("widths", handle);
	mParams->resizeArray(handle, (physx::PxI32)widths.size());
	mParams->setParamF32Array(handle, widths.begin(), (physx::PxI32)widths.size());
}
#endif



void ConvexHull::buildFromAABB(const physx::PxBounds3& aabb)
{
	physx::PxVec3 center, extent;
	center = aabb.getCenter();
	extent = aabb.getExtents();

	NxParameterized::Handle handle(*mParams);

	// Copy vertices and compute bounds
	mParams->getParameterHandle("vertices", handle);
	mParams->resizeArray(handle, 8);
	for (int i = 0; i < 8; ++i)
	{
		mParams->vertices.buf[i] = center + physx::PxVec3((2.0f * (i & 1) - 1.0f) * extent.x, ((physx::PxF32)(i & 2) - 1.0f) * extent.y, (0.5f * (i & 4) - 1.0f) * extent.z);
	}

	mParams->getParameterHandle("uniquePlanes", handle);
	mParams->resizeArray(handle, 3);
	mParams->getParameterHandle("widths", handle);
	mParams->resizeArray(handle, 3);
	for (int i = 0; i < 3; ++i)
	{
		ConvexHullParametersNS::Plane_Type& plane = mParams->uniquePlanes.buf[i];
		plane.normal = physx::PxVec3(0.0f);
		plane.normal[i] = -1.0f;
		plane.d = -aabb.maximum[i];
		mParams->widths.buf[i] = aabb.maximum[i] - aabb.minimum[i];
	}
	mParams->planeCount = 6;

	mParams->getParameterHandle("edges", handle);
	mParams->resizeArray(handle, 12);
	mParams->getParameterHandle("adjacentFaces", handle);
	mParams->resizeArray(handle, 12);
	for (physx::PxU32 i = 0; i < 3; ++i)
	{
		physx::PxU32 i1 = ((i + 1) % 3);
		physx::PxU32 i2 = ((i + 2) % 3);
		physx::PxU32 vOffset1 = 1u << i1;
		physx::PxU32 vOffset2 = 1u << i2;
		for (int j = 0; j < 4; ++j)
		{
			physx::PxU32 v0 = 0;
			physx::PxU32 v1 = 1u << i;
			physx::PxU32 f0 = i1;
			physx::PxU32 f1 = i2;
			if (j & 1)
			{
				v0 += vOffset1;
				v1 += vOffset1;
				f0 += 3;
			}
			if (j & 2)
			{
				v0 += vOffset2;
				v1 += vOffset2;
				f1 += 3;
			}
			if (j == 1 || j == 2)
			{
				physx::swap(f0, f1);
			}
			physx::PxU32 index = i + 3 * j;
			mParams->edges.buf[index] = v0 << 16 | v1;
			mParams->adjacentFaces.buf[i] = f0 << 16 | f1;
		}
	}
	mParams->uniqueEdgeDirectionCount = 3;

	mParams->bounds = aabb;

	const physx::PxVec3 diag = mParams->bounds.maximum - mParams->bounds.minimum;
	mParams->volume = diag.x * diag.y * diag.z;
}

void ConvexHull::buildKDOP(const void* points, physx::PxU32 numPoints, physx::PxU32 pointStrideBytes, const physx::PxVec3* directions, physx::PxU32 numDirections)
{
	setEmpty();

	if (numPoints == 0)
	{
		return;
	}

	physx::PxF32 size = 0;

	physx::Array<physx::PxPlane> planes;
	planes.reserve(2 * numDirections);
	for (physx::PxU32 i = 0; i < numDirections; ++i)
	{
		physx::PxVec3 dir = directions[i];
		dir.normalize();
		physx::PxF32 min, max;
		getExtent(min, max, points, numPoints, pointStrideBytes, dir);
		planes.pushBack(physx::PxPlane(dir, -max));
		planes.pushBack(physx::PxPlane(-dir, min));
		size = physx::PxMax(size, max - min);
	}

	buildFromPlanes(planes.begin(), planes.size(), 0.00001f * size);
}

void ConvexHull::intersectPlaneSide(const physx::PxPlane& plane)
{
	const physx::PxF32 size = (mParams->bounds.maximum - mParams->bounds.minimum).magnitude();
	const physx::PxF32 eps = 0.00001f * size;

	const physx::PxU32 oldVertexCount = (physx::PxU32)mParams->vertices.arraySizes[0];

	Array<PxVec3> vertices;
	vertices.resize(oldVertexCount);
	NxParameterized::Handle handle(*mParams);
	mParams->getParameterHandle("vertices", handle);
	mParams->getParamVec3Array(handle, vertices.begin(), (physx::PxI32)vertices.size());

	// Throw out all vertices on the + side of the plane
	for (physx::PxU32 i = oldVertexCount; i--;)
	{
		if (plane.distance(vertices[i]) > eps)
		{
			vertices.replaceWithLast(i);
		}
	}

	if (vertices.size() == 0)
	{
		// plane excludes this whole hull.  Delete everything.
		setEmpty();
		return;
	}

	if (vertices.size() == oldVertexCount)
	{
		// plane includes this whole hull.  Do nothing.
		return;
	}

	// Intersect new plane with all pairs of old planes.
	const physx::PxU32 planeCount = getPlaneCount();
	for (physx::PxU32 j = 1; j < planeCount; ++j)
	{
		const physx::PxPlane planeJ = getPlane(j);
		for (physx::PxU32 i = 0; i < j; ++i)
		{
			const physx::PxPlane planeI = getPlane(i);
			physx::PxVec3 point;
			if (intersectPlanes(point, plane, planeI, planeJ))
			{
				// See if point is excluded by any of the other planes
				physx::PxU32 k;
				for (k = 0; k < planeCount; ++k)
				{
					if (k != i && k != j && getPlane(k).distance(point) > eps)
					{
						break;
					}
				}
				if (k == planeCount)
				{
					// Point is part of the new intersected hull
					physx::PxU32 m = 0;
					for (; m < vertices.size(); ++m)
					{
						if ((point - vertices[m]).magnitudeSquared() < eps)
						{
							break;
						}
					}
					if (m == vertices.size())
					{
						vertices.pushBack(point);
					}
				}
			}
		}
	}

	buildFromPoints(vertices.begin(), vertices.size(), sizeof(vertices[0]));
}

void ConvexHull::intersectHull(const ConvexHull& hull)
{
	if (hull.getPlaneCount() == 0)
	{
		setEmpty();
		return;
	}

	physx::Array<physx::PxPlane> planes;
	planes.reserve(getPlaneCount() + hull.getPlaneCount());
	for (physx::PxU32 planeN = 0; planeN < getPlaneCount(); ++planeN)
	{
		planes.pushBack(getPlane(planeN));
	}
	for (physx::PxU32 planeN = 0; planeN < hull.getPlaneCount(); ++planeN)
	{
		planes.pushBack(hull.getPlane(planeN));
	}

	const physx::PxF32 size = (mParams->bounds.maximum - mParams->bounds.minimum).magnitude();

	buildFromPlanes(planes.begin(), planes.size(), 0.00001f * size);
}

// Returns true iff distance does not exceed result and maxDistance
PX_INLINE bool updateResultAndSeparation(bool& updated, bool& normalPointsFrom0to1, physx::PxF32& result, ConvexHull::Separation* separation,
										 physx::PxF32 min0, physx::PxF32 max0, physx::PxF32 min1, physx::PxF32 max1, const physx::PxVec3& n, physx::PxF32 maxDistance)
{
	physx::PxF32 midpoint;
	const physx::PxF32 dist = extentDistanceAndNormalDirection(min0, max0, min1, max1, midpoint, normalPointsFrom0to1);
	updated = dist > result;
	if (updated)
	{
		if (dist > maxDistance)
		{
			return false;
		}
		result = dist;
		if (separation != NULL)
		{
			if (normalPointsFrom0to1)
			{
				separation->plane = physx::PxPlane(n, -midpoint);
				separation->min0 = min0;
				separation->max0 = max0;
				separation->min1 = min1;
				separation->max1 = max1;
			}
			else
			{
				separation->plane = physx::PxPlane(-n, midpoint);
				separation->min0 = -max0;
				separation->max0 = -min0;
				separation->min1 = -max1;
				separation->max1 = -min1;
			}
		}
	}
	return true;
}


namespace GjkInternal
{
using namespace physx::aos;

PX_NOALIAS PX_FORCE_INLINE BoolV PointOutsideOfPlane4(const Vec3VArg _a, const Vec3VArg _b, const Vec3VArg _c, const Vec3VArg _d)
{
	// this is not 0 because of the following scenario:
	// All the points lie on the same plane and the plane goes through the origin (0,0,0).
	// On the Wii U, the math below has the problem that when point A gets projected on the
	// plane cumputed by A, B, C, the distance to the plane might not be 0 for the mentioned
	// scenario but a small positive or negative value. This can lead to the wrong boolean
	// results. Using a small negative value as threshold is more conservative but safer.
	const Vec4V zero = V4Load(-1e-6);

	const Vec3V ab = V3Sub(_b, _a);
	const Vec3V ac = V3Sub(_c, _a);
	const Vec3V ad = V3Sub(_d, _a);
	const Vec3V bd = V3Sub(_d, _b);
	const Vec3V bc = V3Sub(_c, _b);

	const Vec3V v0 = V3Cross(ab, ac);
	const Vec3V v1 = V3Cross(ac, ad);
	const Vec3V v2 = V3Cross(ad, ab);
	const Vec3V v3 = V3Cross(bd, bc);

	const FloatV signa0 = V3Dot(v0, _a);
	const FloatV signa1 = V3Dot(v1, _a);
	const FloatV signa2 = V3Dot(v2, _a);
	const FloatV signd3 = V3Dot(v3, _a);

	const FloatV signd0 = V3Dot(v0, _d);
	const FloatV signd1 = V3Dot(v1, _b);
	const FloatV signd2 = V3Dot(v2, _c);
	const FloatV signa3 = V3Dot(v3, _b);

	const Vec4V signa = V4Merge(signa0, signa1, signa2, signa3);
	const Vec4V signd = V4Merge(signd0, signd1, signd2, signd3);
	return V4IsGrtrOrEq(V4Mul(signa, signd), zero);//same side, outside of the plane
}

PX_NOALIAS PX_FORCE_INLINE Vec3V closestPtPointSegment(const Vec3VArg a, const Vec3VArg b)
{
	const FloatV zero = FZero();
	const FloatV one = FOne();

	//Test degenerated case
	const Vec3V ab = V3Sub(b, a);
	const FloatV denom = V3Dot(ab, ab);
	const Vec3V ap = V3Neg(a);//V3Sub(origin, a);
	const FloatV nom = V3Dot(ap, ab);
	const BoolV con = FIsEq(denom, zero);
	const FloatV tValue = FClamp(FDiv(nom, denom), zero, one);
	const FloatV t = FSel(con, zero, tValue);

	return V3Sel(con, a, V3ScaleAdd(ab, t, a));
}

PX_NOALIAS PX_FORCE_INLINE Vec3V closestPtPointSegment(const Vec3VArg Q0, const Vec3VArg Q1, const Vec3VArg A0, const Vec3VArg A1,
	const Vec3VArg B0, const Vec3VArg B1, PxU32& size, Vec3V& closestA, Vec3V& closestB)
{
	const Vec3V a = Q0;
	const Vec3V b = Q1;

	const BoolV bTrue = BTTTT();
	const FloatV zero = FZero();
	const FloatV one = FOne();

	//Test degenerated case
	const Vec3V ab = V3Sub(b, a);
	const FloatV denom = V3Dot(ab, ab);
	const Vec3V ap = V3Neg(a);//V3Sub(origin, a);
	const FloatV nom = V3Dot(ap, ab);
	const BoolV con = FIsEq(denom, zero);

	if (BAllEq(con, bTrue))
	{
		size = 1;
		closestA = A0;
		closestB = B0;
		return Q0;
	}

	const Vec3V v = V3Sub(A1, A0);
	const Vec3V w = V3Sub(B1, B0);
	const FloatV tValue = FClamp(FDiv(nom, denom), zero, one);
	const FloatV t = FSel(con, zero, tValue);

	const Vec3V tempClosestA = V3ScaleAdd(v, t, A0);
	const Vec3V tempClosestB = V3ScaleAdd(w, t, B0);
	closestA = tempClosestA;
	closestB = tempClosestB;
	return V3Sub(tempClosestA, tempClosestB);
}

PX_NOALIAS Vec3V closestPtPointSegmentTesselation(const Vec3VArg Q0, const Vec3VArg Q1, const Vec3VArg A0, const Vec3VArg A1,
	const Vec3VArg B0, const Vec3VArg B1, PxU32& size, Vec3V& closestA, Vec3V& closestB)
{
	const FloatV half = FHalf();

	const FloatV targetSegmentLengthSq = FLoad(10000.f);//100 unit

	Vec3V q0 = Q0;
	Vec3V q1 = Q1;
	Vec3V a0 = A0;
	Vec3V a1 = A1;
	Vec3V b0 = B0;
	Vec3V b1 = B1;

	do
	{
		const Vec3V midPoint = V3Scale(V3Add(q0, q1), half);
		const Vec3V midA = V3Scale(V3Add(a0, a1), half);
		const Vec3V midB = V3Scale(V3Add(b0, b1), half);

		const Vec3V v = V3Sub(midPoint, q0);
		const FloatV sqV = V3Dot(v, v);
		if (FAllGrtr(targetSegmentLengthSq, sqV))
			break;
		//split the segment into half
		const Vec3V tClos0 = closestPtPointSegment(q0, midPoint);
		const FloatV sqDist0 = V3Dot(tClos0, tClos0);

		const Vec3V tClos1 = closestPtPointSegment(q1, midPoint);
		const FloatV sqDist1 = V3Dot(tClos1, tClos1);
		//const BoolV con = FIsGrtr(sqDist0, sqDist1);
		if (FAllGrtr(sqDist0, sqDist1))
		{
			//segment [m, q1]
			q0 = midPoint;
			a0 = midA;
			b0 = midB;
		}
		else
		{
			//segment [q0, m]
			q1 = midPoint;
			a1 = midA;
			b1 = midB;
		}

	} while (1);

	return closestPtPointSegment(q0, q1, a0, a1, b0, b1, size, closestA, closestB);
}

PX_NOALIAS Vec3V closestPtPointTriangleTesselation(const Vec3V* PX_RESTRICT Q, const Vec3V* PX_RESTRICT A, const Vec3V* PX_RESTRICT B, const PxU32* PX_RESTRICT indices, PxU32& size, Vec3V& closestA, Vec3V& closestB)
{
	size = 3;
	const FloatV zero = FZero();
	const FloatV eps = FEps();
	const FloatV half = FHalf();
	const BoolV bTrue = BTTTT();
	const FloatV four = FLoad(4.f);
	const FloatV sixty = FLoad(100.f);

	const PxU32 ind0 = indices[0];
	const PxU32 ind1 = indices[1];
	const PxU32 ind2 = indices[2];

	const Vec3V a = Q[ind0];
	const Vec3V b = Q[ind1];
	const Vec3V c = Q[ind2];

	Vec3V ab_ = V3Sub(b, a);
	Vec3V ac_ = V3Sub(c, a);
	Vec3V bc_ = V3Sub(b, c);

	const FloatV dac_ = V3Dot(ac_, ac_);
	const FloatV dbc_ = V3Dot(bc_, bc_);
	if (FAllGrtrOrEq(eps, FMin(dac_, dbc_)))
	{
		//degenerate
		size = 2;
		return closestPtPointSegment(Q[ind0], Q[ind1], A[ind0], A[ind1], B[ind0], B[ind1], size, closestA, closestB);
	}

	Vec3V ap = V3Neg(a);
	Vec3V bp = V3Neg(b);
	Vec3V cp = V3Neg(c);

	FloatV d1 = V3Dot(ab_, ap); //  snom
	FloatV d2 = V3Dot(ac_, ap); //  tnom
	FloatV d3 = V3Dot(ab_, bp); // -sdenom
	FloatV d4 = V3Dot(ac_, bp); //  unom = d4 - d3
	FloatV d5 = V3Dot(ab_, cp); //  udenom = d5 - d6
	FloatV d6 = V3Dot(ac_, cp); // -tdenom
	/*	FloatV unom = FSub(d4, d3);
	FloatV udenom = FSub(d5, d6);*/

	FloatV va = FNegScaleSub(d5, d4, FMul(d3, d6));//edge region of BC
	FloatV vb = FNegScaleSub(d1, d6, FMul(d5, d2));//edge region of AC
	FloatV vc = FNegScaleSub(d3, d2, FMul(d1, d4));//edge region of AB

	//check if p in vertex region outside a
	const BoolV con00 = FIsGrtrOrEq(zero, d1); // snom <= 0
	const BoolV con01 = FIsGrtrOrEq(zero, d2); // tnom <= 0
	const BoolV con0 = BAnd(con00, con01); // vertex region a
	if (BAllEq(con0, bTrue))
	{
		//size = 1;
		closestA = A[ind0];
		closestB = B[ind0];
		return Q[ind0];
	}

	//check if p in vertex region outside b
	const BoolV con10 = FIsGrtrOrEq(d3, zero);
	const BoolV con11 = FIsGrtrOrEq(d3, d4);
	const BoolV con1 = BAnd(con10, con11); // vertex region b
	if (BAllEq(con1, bTrue))
	{
		/*size = 1;
		indices[0] = ind1;*/
		closestA = A[ind1];
		closestB = B[ind1];
		return Q[ind1];
	}


	//check if p in vertex region outside of c
	const BoolV con20 = FIsGrtrOrEq(d6, zero);
	const BoolV con21 = FIsGrtrOrEq(d6, d5);
	const BoolV con2 = BAnd(con20, con21); // vertex region c
	if (BAllEq(con2, bTrue))
	{
		closestA = A[ind2];
		closestB = B[ind2];
		return Q[ind2];
	}

	//check if p in edge region of AB
	const BoolV con30 = FIsGrtrOrEq(zero, vc);
	const BoolV con31 = FIsGrtrOrEq(d1, zero);
	const BoolV con32 = FIsGrtrOrEq(zero, d3);
	const BoolV con3 = BAnd(con30, BAnd(con31, con32));

	if (BAllEq(con3, bTrue))
	{
		//size = 2;
		//p in edge region of AB, split AB
		return closestPtPointSegmentTesselation(Q[ind0], Q[ind1], A[ind0], A[ind1], B[ind0], B[ind1], size, closestA, closestB);
	}

	//check if p in edge region of BC
	const BoolV con40 = FIsGrtrOrEq(zero, va);
	const BoolV con41 = FIsGrtrOrEq(d4, d3);
	const BoolV con42 = FIsGrtrOrEq(d5, d6);
	const BoolV con4 = BAnd(con40, BAnd(con41, con42));

	if (BAllEq(con4, bTrue))
	{
		//p in edge region of BC, split BC
		return closestPtPointSegmentTesselation(Q[ind1], Q[ind2], A[ind1], A[ind2], B[ind1], B[ind2], size, closestA, closestB);
	}

	//check if p in edge region of AC
	const BoolV con50 = FIsGrtrOrEq(zero, vb);
	const BoolV con51 = FIsGrtrOrEq(d2, zero);
	const BoolV con52 = FIsGrtrOrEq(zero, d6);
	const BoolV con5 = BAnd(con50, BAnd(con51, con52));

	if (BAllEq(con5, bTrue))
	{
		//p in edge region of AC, split AC
		return closestPtPointSegmentTesselation(Q[ind0], Q[ind2], A[ind0], A[ind2], B[ind0], B[ind2], size, closestA, closestB);
	}

	size = 3;

	Vec3V q0 = Q[ind0];
	Vec3V q1 = Q[ind1];
	Vec3V q2 = Q[ind2];
	Vec3V a0 = A[ind0];
	Vec3V a1 = A[ind1];
	Vec3V a2 = A[ind2];
	Vec3V b0 = B[ind0];
	Vec3V b1 = B[ind1];
	Vec3V b2 = B[ind2];

	do
	{

		const Vec3V ab = V3Sub(q1, q0);
		const Vec3V ac = V3Sub(q2, q0);
		const Vec3V bc = V3Sub(q2, q1);

		const FloatV dab = V3Dot(ab, ab);
		const FloatV dac = V3Dot(ac, ac);
		const FloatV dbc = V3Dot(bc, bc);

		const FloatV fMax = FMax(dab, FMax(dac, dbc));
		const FloatV fMin = FMin(dab, FMin(dac, dbc));

		const Vec3V w = V3Cross(ab, ac);

		const FloatV area = V3Length(w);
		const FloatV ratio = FDiv(FSqrt(fMax), FSqrt(fMin));
		if (FAllGrtr(four, ratio) && FAllGrtr(sixty, area))
			break;

		//calculate the triangle normal
		const Vec3V triNormal = V3Normalize(w);

		PX_ASSERT(V3AllEq(triNormal, V3Zero()) == 0);


		//split the longest edge
		if (FAllGrtrOrEq(dab, dac) && FAllGrtrOrEq(dab, dbc))
		{
			//split edge q0q1
			const Vec3V midPoint = V3Scale(V3Add(q0, q1), half);
			const Vec3V midA = V3Scale(V3Add(a0, a1), half);
			const Vec3V midB = V3Scale(V3Add(b0, b1), half);

			const Vec3V v = V3Sub(midPoint, q2);
			const Vec3V n = V3Normalize(V3Cross(v, triNormal));

			const FloatV d = FNeg(V3Dot(n, midPoint));
			const FloatV dp = FAdd(V3Dot(n, q0), d);
			const FloatV sum = FMul(d, dp);

			if (FAllGrtr(sum, zero))
			{
				//q0 and origin at the same side, split triangle[q0, m, q2]
				q1 = midPoint;
				a1 = midA;
				b1 = midB;
			}
			else
			{
				//q1 and origin at the same side, split triangle[m, q1, q2]
				q0 = midPoint;
				a0 = midA;
				b0 = midB;
			}

		}
		else if (FAllGrtrOrEq(dac, dbc))
		{
			//split edge q0q2
			const Vec3V midPoint = V3Scale(V3Add(q0, q2), half);
			const Vec3V midA = V3Scale(V3Add(a0, a2), half);
			const Vec3V midB = V3Scale(V3Add(b0, b2), half);

			const Vec3V v = V3Sub(midPoint, q1);
			const Vec3V n = V3Normalize(V3Cross(v, triNormal));

			const FloatV d = FNeg(V3Dot(n, midPoint));
			const FloatV dp = FAdd(V3Dot(n, q0), d);
			const FloatV sum = FMul(d, dp);

			if (FAllGrtr(sum, zero))
			{
				//q0 and origin at the same side, split triangle[q0, q1, m]
				q2 = midPoint;
				a2 = midA;
				b2 = midB;
			}
			else
			{
				//q2 and origin at the same side, split triangle[m, q1, q2]
				q0 = midPoint;
				a0 = midA;
				b0 = midB;
			}
		}
		else
		{
			//split edge q1q2
			const Vec3V midPoint = V3Scale(V3Add(q1, q2), half);
			const Vec3V midA = V3Scale(V3Add(a1, a2), half);
			const Vec3V midB = V3Scale(V3Add(b1, b2), half);

			const Vec3V v = V3Sub(midPoint, q0);
			const Vec3V n = V3Normalize(V3Cross(v, triNormal));

			const FloatV d = FNeg(V3Dot(n, midPoint));
			const FloatV dp = FAdd(V3Dot(n, q1), d);
			const FloatV sum = FMul(d, dp);

			if (FAllGrtr(sum, zero))
			{
				//q1 and origin at the same side, split triangle[q0, q1, m]
				q2 = midPoint;
				a2 = midA;
				b2 = midB;
			}
			else
			{
				//q2 and origin at the same side, split triangle[q0, m, q2]
				q1 = midPoint;
				a1 = midA;
				b1 = midB;
			}


		}
	} while (1);

	//P must project inside face region. Compute Q using Barycentric coordinates
	ab_ = V3Sub(q1, q0);
	ac_ = V3Sub(q2, q0);
	ap = V3Neg(q0);
	bp = V3Neg(q1);
	cp = V3Neg(q2);

	d1 = V3Dot(ab_, ap); //  snom
	d2 = V3Dot(ac_, ap); //  tnom
	d3 = V3Dot(ab_, bp); // -sdenom
	d4 = V3Dot(ac_, bp); //  unom = d4 - d3
	d5 = V3Dot(ab_, cp); //  udenom = d5 - d6
	d6 = V3Dot(ac_, cp); // -tdenom

	va = FNegScaleSub(d5, d4, FMul(d3, d6));//edge region of BC
	vb = FNegScaleSub(d1, d6, FMul(d5, d2));//edge region of AC
	vc = FNegScaleSub(d3, d2, FMul(d1, d4));//edge region of AB

	const FloatV toRecipD = FAdd(va, FAdd(vb, vc));
	const FloatV denom = FRecip(toRecipD);//V4GetW(recipTmp);
	const Vec3V v0 = V3Sub(a1, a0);
	const Vec3V v1 = V3Sub(a2, a0);
	const Vec3V w0 = V3Sub(b1, b0);
	const Vec3V w1 = V3Sub(b2, b0);

	const FloatV t = FMul(vb, denom);
	const FloatV w = FMul(vc, denom);
	const Vec3V vA1 = V3Scale(v1, w);
	const Vec3V vB1 = V3Scale(w1, w);
	const Vec3V tempClosestA = V3Add(a0, V3ScaleAdd(v0, t, vA1));
	const Vec3V tempClosestB = V3Add(b0, V3ScaleAdd(w0, t, vB1));
	closestA = tempClosestA;
	closestB = tempClosestB;
	return V3Sub(tempClosestA, tempClosestB);
}

PX_NOALIAS Vec3V closestPtPointTetrahedronTesselation(Vec3V* PX_RESTRICT Q, Vec3V* PX_RESTRICT A, Vec3V* PX_RESTRICT B, PxU32& size, Vec3V& closestA, Vec3V& closestB)
{
	const FloatV eps = FEps();
	const Vec3V zeroV = V3Zero();
	PxU32 tempSize = size;

	FloatV bestSqDist = FLoad(PX_MAX_REAL);
	const Vec3V a = Q[0];
	const Vec3V b = Q[1];
	const Vec3V c = Q[2];
	const Vec3V d = Q[3];
	const BoolV bTrue = BTTTT();
	const BoolV bFalse = BFFFF();

	//degenerated
	const Vec3V ad = V3Sub(d, a);
	const Vec3V bd = V3Sub(d, b);
	const Vec3V cd = V3Sub(d, c);
	const FloatV dad = V3Dot(ad, ad);
	const FloatV dbd = V3Dot(bd, bd);
	const FloatV dcd = V3Dot(cd, cd);
	const FloatV fMin = FMin(dad, FMin(dbd, dcd));
	if (FAllGrtr(eps, fMin))
	{
		size = 3;
		PxU32 tempIndices[] = { 0, 1, 2 };
		return closestPtPointTriangleTesselation(Q, A, B, tempIndices, size, closestA, closestB);
	}

	Vec3V _Q[] = { Q[0], Q[1], Q[2], Q[3] };
	Vec3V _A[] = { A[0], A[1], A[2], A[3] };
	Vec3V _B[] = { B[0], B[1], B[2], B[3] };

	PxU32 indices[3] = { 0, 1, 2 };

	const BoolV bIsOutside4 = PointOutsideOfPlane4(a, b, c, d);

	if (BAllEq(bIsOutside4, bFalse))
	{
		//origin is inside the tetrahedron, we are done
		return zeroV;
	}

	Vec3V result = zeroV;
	Vec3V tempClosestA, tempClosestB;

	if (BAllEq(BGetX(bIsOutside4), bTrue))
	{

		PxU32 tempIndices[] = { 0, 1, 2 };
		PxU32 _size = 3;

		result = closestPtPointTriangleTesselation(_Q, _A, _B, tempIndices, _size, tempClosestA, tempClosestB);

		const FloatV sqDist = V3Dot(result, result);
		bestSqDist = sqDist;

		indices[0] = tempIndices[0];
		indices[1] = tempIndices[1];
		indices[2] = tempIndices[2];

		tempSize = _size;
		closestA = tempClosestA;
		closestB = tempClosestB;
	}

	if (BAllEq(BGetY(bIsOutside4), bTrue))
	{

		PxU32 tempIndices[] = { 0, 2, 3 };

		PxU32 _size = 3;

		const Vec3V q = closestPtPointTriangleTesselation(_Q, _A, _B, tempIndices, _size, tempClosestA, tempClosestB);

		const FloatV sqDist = V3Dot(q, q);
		const BoolV con = FIsGrtr(bestSqDist, sqDist);
		if (BAllEq(con, bTrue))
		{
			result = q;
			bestSqDist = sqDist;
			indices[0] = tempIndices[0];
			indices[1] = tempIndices[1];
			indices[2] = tempIndices[2];

			tempSize = _size;
			closestA = tempClosestA;
			closestB = tempClosestB;
		}
	}

	if (BAllEq(BGetZ(bIsOutside4), bTrue))
	{

		PxU32 tempIndices[] = { 0, 3, 1 };
		PxU32 _size = 3;

		const Vec3V q = closestPtPointTriangleTesselation(_Q, _A, _B, tempIndices, _size, tempClosestA, tempClosestB);

		const FloatV sqDist = V3Dot(q, q);
		const BoolV con = FIsGrtr(bestSqDist, sqDist);
		if (BAllEq(con, bTrue))
		{
			result = q;
			bestSqDist = sqDist;
			indices[0] = tempIndices[0];
			indices[1] = tempIndices[1];
			indices[2] = tempIndices[2];
			tempSize = _size;
			closestA = tempClosestA;
			closestB = tempClosestB;
		}

	}

	if (BAllEq(BGetW(bIsOutside4), bTrue))
	{

		PxU32 tempIndices[] = { 1, 3, 2 };
		PxU32 _size = 3;

		const Vec3V q = closestPtPointTriangleTesselation(_Q, _A, _B, tempIndices, _size, tempClosestA, tempClosestB);

		const FloatV sqDist = V3Dot(q, q);
		const BoolV con = FIsGrtr(bestSqDist, sqDist);

		if (BAllEq(con, bTrue))
		{
			result = q;
			bestSqDist = sqDist;

			indices[0] = tempIndices[0];
			indices[1] = tempIndices[1];
			indices[2] = tempIndices[2];

			tempSize = _size;
			closestA = tempClosestA;
			closestB = tempClosestB;
		}
	}

	A[0] = _A[indices[0]]; A[1] = _A[indices[1]]; A[2] = _A[indices[2]];
	B[0] = _B[indices[0]]; B[1] = _B[indices[1]]; B[2] = _B[indices[2]];
	Q[0] = _Q[indices[0]]; Q[1] = _Q[indices[1]]; Q[2] = _Q[indices[2]];


	size = tempSize;
	return result;
}

PX_NOALIAS PX_FORCE_INLINE Vec3V doTesselation(Vec3V* PX_RESTRICT Q, Vec3V* PX_RESTRICT A, Vec3V* PX_RESTRICT B,
	const Vec3VArg support, const Vec3VArg supportA, const Vec3VArg supportB, PxU32& size, Vec3V& closestA, Vec3V& closestB)
{
	switch (size)
	{
	case 1:
	{
		closestA = supportA;
		closestB = supportB;
		return support;
	}
	case 2:
	{
		return closestPtPointSegmentTesselation(Q[0], support, A[0], supportA, B[0], supportB, size, closestA, closestB);
	}
	case 3:
	{

		PxU32 tempIndices[3] = { 0, 1, 2 };
		return closestPtPointTriangleTesselation(Q, A, B, tempIndices, size, closestA, closestB);
	}
	case 4:
	{
		return closestPtPointTetrahedronTesselation(Q, A, B, size, closestA, closestB);
	}
	default:
		PX_ASSERT(0);
	}
	return support;
}

struct ConvexV
{
	void calcExtent(const Vec3V& dir, PxF32& minOut, PxF32& maxOut) const
	{
		// Expand 
		const Vec4V x = Vec4V_From_FloatV(V3GetX(dir));
		const Vec4V y = Vec4V_From_FloatV(V3GetY(dir));
		const Vec4V z = Vec4V_From_FloatV(V3GetZ(dir));

		const Vec4V* src = mAovVertices;
		const Vec4V* end = src + mNumAovVertices * 3;

		// Do first step
		Vec4V max = V4MulAdd(x, src[0], V4MulAdd(y, src[1], V4Mul(z, src[2])));
		Vec4V min = max;
		src += 3;
		// Do the rest
		for (; src < end; src += 3)
		{
			const Vec4V dot = V4MulAdd(x, src[0], V4MulAdd(y, src[1], V4Mul(z, src[2])));
			max = V4Max(dot, max);
			min = V4Min(dot, min);
		}
		maxOut =  FStore(V4ExtractMax(max));
		minOut = FStore(V4ExtractMin(min));
	}
	Vec3V calcSupport(const Vec3V& dir) const
	{
		// Expand 
		const Vec4V x = Vec4V_From_FloatV(V3GetX(dir));
		const Vec4V y = Vec4V_From_FloatV(V3GetY(dir));
		const Vec4V z = Vec4V_From_FloatV(V3GetZ(dir));

		PX_ALIGN(16, static const PxF32 index4const[]) = { 0.0f, 1.0f, 2.0f, 3.0f };
		Vec4V index4 = *(const Vec4V*)index4const;
		PX_ALIGN(16, static const PxF32 delta4const[]) = { 4.0f, 4.0f, 4.0f, 4.0f };
		const Vec4V delta4 = *(const Vec4V*)delta4const;

		const Vec4V* src = mAovVertices;
		const Vec4V* end = src + mNumAovVertices * 3;

		// Do first step
		Vec4V max = V4MulAdd(x, src[0], V4MulAdd(y, src[1], V4Mul(z, src[2])));
		Vec4V maxIndex = index4;
		index4 = V4Add(index4, delta4);
		src += 3;
		// Do the rest
		for (; src < end; src += 3)
		{
			const Vec4V dot = V4MulAdd(x, src[0], V4MulAdd(y, src[1], V4Mul(z, src[2])));
			const BoolV cmp = V4IsGrtr(dot, max);
			max = V4Max(dot, max);
			maxIndex = V4Sel(cmp, index4, maxIndex);
			index4 = V4Add(index4, delta4);
		}
		Vec4V horiMax = Vec4V_From_FloatV(V4ExtractMax(max));
		PxU32 mask = BGetBitMask(V4IsEq(horiMax, max));
		const PxU32 simdIndex = (0x12131210 >> (mask + mask)) & PxU32(3);

		/// NOTE! Could be load hit store
		/// Would be better to have all simd. 
		PX_ALIGN(16, PxF32 f[4]);
		V4StoreA(maxIndex, f);
		PxU32 index = PxU32(PxI32(f[simdIndex]));

		const Vec4V* aovIndex = (mAovVertices + (index >> 2) * 3);
		const PxF32* aovOffset = ((const PxF32*)aovIndex) + (index & 3);

		return Vec3V_From_Vec4V(Vec4VLoadXYZW(aovOffset[0], aovOffset[4], aovOffset[8], 1.0f));
	}

	const Vec4V* mAovVertices;			///< Vertices storex x,x,x,x, y,y,y,y, z,z,z,z
	PxU32 mNumAovVertices;			///< Number of groups of 4 of vertices
};

struct Output
{
		/// Get the normal to push apart in direction from A to B
	PX_FORCE_INLINE Vec3V getNormal() const { return V3Normalize(V3Sub(mClosestB, mClosestA)); }
	Vec3V mClosestA;				///< Closest point on A
	Vec3V mClosestB;				///< Closest point on B
	FloatV mDistSq;
};

enum Status
{
	STATUS_NON_INTERSECT,
	STATUS_CONTACT,
	STATUS_DEGENERATE,
};

Status Collide(const Vec3V& initialDir, const ConvexV& convexA, const Mat34V& bToA, const ConvexV& convexB, Output& out)
{
	Vec3V Q[4];
	Vec3V A[4];
	Vec3V B[4];

	Mat33V aToB = M34Trnsps33(bToA);

	PxU32 size = 0;

	const Vec3V zeroV = V3Zero();
	const BoolV bTrue = BTTTT();
	
	//Vec3V v = V3UnitX();
	Vec3V v = V3Sel(FIsGrtr(V3Dot(initialDir, initialDir), FZero()), initialDir, V3UnitX());

	//const FloatV minMargin = zero;
	//const FloatV eps2 = FMul(minMargin, FLoad(0.01f));
	//FloatV eps2 = zero;
	FloatV eps2 = FLoad(1e-6f);
	const FloatV epsRel = FLoad(0.000225f);

	Vec3V closA(zeroV), closB(zeroV);
	FloatV sDist = FMax();
	FloatV minDist = sDist;
	Vec3V closAA = zeroV;
	Vec3V closBB = zeroV;

	BoolV bNotTerminated = bTrue;
	BoolV bCon = bTrue;

	do
	{
		minDist = sDist;
		closAA = closA;
		closBB = closB;

		PxU32 index = size++;
		PX_ASSERT(index < 4);

		const Vec3V supportA = convexA.calcSupport(V3Neg(v));
		const Vec3V supportB = M34MulV3(bToA, convexB.calcSupport(M33MulV3(aToB, v)));
		const Vec3V support = V3Sub(supportA, supportB);
		
		A[index] = supportA;
		B[index] = supportB;
		Q[index] = support;

		const FloatV signDist = V3Dot(v, support);
		const FloatV tmp0 = FSub(sDist, signDist);
		if (FAllGrtr(FMul(epsRel, sDist), tmp0))
		{
			out.mClosestA = closA;
			out.mClosestB = closB;
			out.mDistSq = sDist;
			return STATUS_NON_INTERSECT;
		}

		//calculate the closest point between two convex hull
		v = doTesselation(Q, A, B, support, supportA, supportB, size, closA, closB);
		sDist = V3Dot(v, v);
		bCon = FIsGrtr(minDist, sDist);

		bNotTerminated = BAnd(FIsGrtr(sDist, eps2), bCon);
	} while (BAllEq(bNotTerminated, bTrue));

	out.mClosestA = V3Sel(bCon, closA, closAA);
	out.mClosestB = V3Sel(bCon, closB, closBB);
	out.mDistSq = FSel(bCon, sDist, minDist);
	return Status(BAllEq(bCon, bTrue) == 1 ? STATUS_CONTACT : STATUS_DEGENERATE);
}

Status CollidePoint(const ConvexV& convexA, const Vec3V& pointB, Output& out)
{
	Vec3V Q[4];
	Vec3V A[4];
	Vec3V B[4];

	PxU32 size = 0;

	const Vec3V zeroV = V3Zero();
	//const FloatV zero = FZero();
	const BoolV bTrue = BTTTT();

	Vec3V v = V3UnitX();

	//const FloatV minMargin = zero;
	//const FloatV eps2 = FMul(minMargin, FLoad(0.01f));
	//FloatV eps2 = zero;
	FloatV eps2 = FLoad(1e-6f);
	const FloatV epsRel = FLoad(0.000225f);

	Vec3V closA(zeroV), closB(zeroV);
	FloatV sDist = FMax();
	FloatV minDist = sDist;
	Vec3V closAA = zeroV;
	Vec3V closBB = zeroV;

	BoolV bNotTerminated = bTrue;
	BoolV bCon = bTrue;

	do
	{
		minDist = sDist;
		closAA = closA;
		closBB = closB;

		PxU32 index = size++;
		PX_ASSERT(index < 4);

		const Vec3V supportA = convexA.calcSupport(V3Neg(v));
		const Vec3V supportB = pointB; 
		const Vec3V support = V3Sub(supportA, supportB);

		A[index] = supportA;
		B[index] = supportB;
		Q[index] = support;

		const FloatV signDist = V3Dot(v, support);
		const FloatV tmp0 = FSub(sDist, signDist);
		if (FAllGrtr(FMul(epsRel, sDist), tmp0))
		{
			out.mClosestA = closA;
			out.mClosestB = closB;
			out.mDistSq = sDist;
			return STATUS_NON_INTERSECT;
		}

		//calculate the closest point between two convex hull
		v = doTesselation(Q, A, B, support, supportA, supportB, size, closA, closB);
		sDist = V3Dot(v, v);
		bCon = FIsGrtr(minDist, sDist);

		bNotTerminated = BAnd(FIsGrtr(sDist, eps2), bCon);
	} while (BAllEq(bNotTerminated, bTrue));

	out.mClosestA = V3Sel(bCon, closA, closAA);
	out.mClosestB = V3Sel(bCon, closB, closBB);
	out.mDistSq = FSel(bCon, sDist, minDist);
	return Status(BAllEq(bCon, bTrue) == 1 ? STATUS_CONTACT : STATUS_DEGENERATE);
}


} // namespace GjkInternal

static void _arrayVec3ToVec4(const PxVec3* src, physx::aos::Vec4V* dst, PxU32 num)
{
	using namespace physx::aos;
	const PxU32 num4 = num >> 2;
	for (PxU32 i = 0; i < num4; i++, dst += 3, src += 4)
	{
		Vec3V v0 = V3LoadU(&src[0].x);
		Vec3V v1 = V3LoadU(&src[1].x);
		Vec3V v2 = V3LoadU(&src[2].x);
		Vec3V v3 = V3LoadU(&src[3].x);
		// Transpose
		V4Transpose(v0, v1, v2, v3);
		// Save 
		dst[0] = v0;
		dst[1] = v1;
		dst[2] = v2;
	}
	const PxU32 remain = num & 3;
	if (remain)
	{
		Vec3V work[4];
		PxU32 i = 0;
		for (; i < remain; i++) work[i] = V3LoadU(&src[i].x);
		for (; i < 4; i++) work[i] = work[remain - 1];
		V4Transpose(work[0], work[1], work[2], work[3]);
		dst[0] = work[0];
		dst[1] = work[1];
		dst[2] = work[2];
	}
}

static void _arrayVec3ToVec4(const PxVec3* src, const physx::aos::Vec3V& scale, physx::aos::Vec4V* dst, PxU32 num)
{
	using namespace physx::aos;
	
	// If no scale - use the faster version
	if (V3AllEq(scale, V3One()))
	{
		return _arrayVec3ToVec4(src, dst, num);
	}

	const PxU32 num4 = num >> 2;
	for (PxU32 i = 0; i < num4; i++, dst += 3, src += 4)
	{
		Vec3V v0 = V3Mul(scale, V3LoadU(&src[0].x));
		Vec3V v1 = V3Mul(scale, V3LoadU(&src[1].x));
		Vec3V v2 = V3Mul(scale, V3LoadU(&src[2].x));
		Vec3V v3 = V3Mul(scale, V3LoadU(&src[3].x));
		// Transpose
		V4Transpose(v0, v1, v2, v3);
		// Save 
		dst[0] = v0;
		dst[1] = v1;
		dst[2] = v2;
	}
	const PxU32 remain = num & 3;
	if (remain)
	{
		Vec3V work[4];
		PxU32 i = 0;
		for (; i < remain; i++) work[i] = V3Mul(scale, V3LoadU(&src[i].x));
		for (; i < 4; i++) work[i] = work[remain - 1];
		V4Transpose(work[0], work[1], work[2], work[3]);
		dst[0] = work[0];
		dst[1] = work[1];
		dst[2] = work[2];
	}
}

static void _calcSeparation(const GjkInternal::ConvexV& convexA, const physx::PxMat34Legacy& aToWorldIn, const physx::aos::Mat34V& bToA, const GjkInternal::ConvexV& convexB, const GjkInternal::Output& out, ConvexHull::Separation& sep)
{
	using namespace physx::aos;

	Mat33V aToB = M34Trnsps33(bToA);
	Vec3V normalA = out.getNormal();

	convexA.calcExtent(normalA, sep.min0, sep.max0);
	Vec3V normalB = M33MulV3(aToB, normalA);
	convexB.calcExtent(normalB, sep.min1, sep.max1);

	{
		// Offset the min max taking into account transform
		// Distance of origin from B's space in As space in direction of the normal in As space should fix it...
		PxF32 fix = FStore(V3Dot(bToA.col3, normalA));
		sep.min1 += fix;
		sep.max1 += fix;
	}

	// Looks like it's the plane at the midpoint
	Vec3V center = V3Scale(V3Add(out.mClosestA, out.mClosestB), FLoad(0.5f));
	// Transform to world space
	Mat34V aToWorld;
	aToWorldIn.getColumnMajor44((PxF32*)&aToWorld);
	// Put the normal in world space
	Vec3V worldCenter = M34MulV3(aToWorld, center);
	Vec3V worldNormal = M34Mul33V3(aToWorld, normalA);

	FloatV dist = V3Dot(worldNormal, worldCenter);
	V3StoreU(worldNormal, sep.plane.n);
	sep.plane.d = -FStore(dist);
}

bool ConvexHull::hullsInProximity(const ConvexHull& hull0, const physx::PxMat34Legacy& localToWorldRT0In, const physx::PxVec3& scale0In,
	const ConvexHull& hull1, const physx::PxMat34Legacy& localToWorldRT1In, const physx::PxVec3& scale1In,
	physx::PxF32 maxDistance, Separation* separation)
{
	using namespace physx::aos;

	const PxU32 numVerts0 = hull0.getVertexCount();
	const PxU32 numVerts1 = hull1.getVertexCount();
	const PxU32 numAov0 = (numVerts0 + 3) >> 2;
	const PxU32 numAov1 = (numVerts1 + 3) >> 2;

	Vec4V* verts0 = (Vec4V*)PxAlloca((numAov0 + numAov1) * sizeof(Vec4V) * 3);
	// Make sure it's aligned
	PX_ASSERT((size_t(verts0) & 0xf) == 0);

	Vec4V* verts1 = verts0 + (numAov0 * 3);

	const Vec3V scale0 = V3LoadU(&scale0In.x);
	const Vec3V scale1 = V3LoadU(&scale1In.x);

	_arrayVec3ToVec4(hull0.mParams->vertices.buf, scale0, verts0, numVerts0);
	_arrayVec3ToVec4(hull1.mParams->vertices.buf, scale1, verts1, numVerts1);

	PxMat34Legacy trans1To0(false);
	trans1To0.multiplyInverseRTLeft(localToWorldRT0In, localToWorldRT1In);

	// Load into simd mat
	Mat34V bToA;
	trans1To0.getColumnMajor44((PxF32*)&bToA);

	GjkInternal::ConvexV convexA;
	GjkInternal::ConvexV convexB;

	convexA.mNumAovVertices = numAov0;
	convexA.mAovVertices = verts0;

	convexB.mNumAovVertices = numAov1;
	convexB.mAovVertices = verts1;

	// Take the origin of B in As space as the inital direction as it is 'the difference in transform origins B-A in A's space'
	// Should be a good first guess
	const Vec3V initialDir = bToA.col3;

	GjkInternal::Output output;
	GjkInternal::Status status = GjkInternal::Collide(initialDir, convexA, bToA, convexB, output);
	
	if (status == GjkInternal::STATUS_DEGENERATE)
	{
		// Calculate the tolerance from the extents
		const PxVec3 extents0 = hull0.getBounds().getExtents();
		const PxVec3 extents1 = hull1.getBounds().getExtents();

		const FloatV tolerance0 = V3ExtractMin(V3Mul(V3LoadU(&extents0.x), scale0));
		const FloatV tolerance1 = V3ExtractMin(V3Mul(V3LoadU(&extents1.x), scale1));

		const FloatV tolerance = FMul(FAdd(tolerance0, tolerance1), FLoad(0.01f));
		const FloatV sqTolerance = FMul(tolerance, tolerance);

		status = FAllGrtr(sqTolerance, output.mDistSq) ? GjkInternal::STATUS_CONTACT : GjkInternal::STATUS_NON_INTERSECT;
	}

	switch (status)
	{
		case GjkInternal::STATUS_CONTACT:
		{ 
			if (separation)
			{
				_calcSeparation(convexA, localToWorldRT0In, bToA, convexB, output, *separation);
			}
			return true;
		}
		default:
		case GjkInternal::STATUS_NON_INTERSECT:
		{
			if (separation)
			{
				_calcSeparation(convexA, localToWorldRT0In, bToA, convexB, output, *separation);
			}
			return FStore(output.mDistSq) < (maxDistance * maxDistance);
		}
	}
}

static void _calcSeparation(const GjkInternal::ConvexV& convex, const physx::PxMat34Legacy& hullLocalToWorldRT, const PxVec3& sphereCenterWorld, PxF32 sphereRadius, const GjkInternal::Output& gjkOut, ConvexHull::Separation& sep)
{
	using namespace physx::aos;

	const Vec3V normal = gjkOut.getNormal();

	// Calc the plane normal (in world space)
	V3StoreU(normal, sep.plane.n);
	sep.plane.n = hullLocalToWorldRT.M * sep.plane.n;

	const PxF32 originOffset = sep.plane.n.dot(hullLocalToWorldRT.t);
	convex.calcExtent(normal, sep.min0, sep.max0);
	// The min and mix are in local space -> fix by taking into account world space origin
	sep.min0 += originOffset;
	sep.max0 += originOffset;

	// Calculate the sphere radius
	const PxF32 centerDist = sep.plane.n.dot(sphereCenterWorld);
	// Work out the spheres 
	sep.min1 = centerDist - sphereRadius;
	sep.max1 = centerDist + sphereRadius;
	// Set the plane distance
	sep.plane.d = (sep.max0 + sep.min1) * -0.5f;
}

bool ConvexHull::sphereInProximity(const physx::PxMat34Legacy& hullLocalToWorldRT, const physx::PxVec3& hullScaleIn,
								   const physx::PxVec3& sphereWorldCenterIn, physx::PxF32 sphereRadius,
								   physx::PxF32 maxDistance, Separation* separation)
{
	using namespace physx::aos;

	const PxU32 numVerts = getVertexCount();
	const PxU32 numAov = (numVerts + 3) >> 2;
	Vec4V* verts = (Vec4V*)PxAlloca(numAov * sizeof(Vec4V) * 3);
	// Make sure it's aligned
	PX_ASSERT((size_t(verts) & 0xf) == 0);

	const Vec3V hullScale = V3LoadU(&hullScaleIn.x);

	_arrayVec3ToVec4(mParams->vertices.buf, hullScale, verts, numVerts);

	// Put the spheres center in the hulls space
	Vec3V sphereCenterLocal = V3LoadU(hullLocalToWorldRT.multiplyByInverseRT(sphereWorldCenterIn));

	// Load into simd mat
	GjkInternal::ConvexV convex;

	convex.mNumAovVertices = numAov;
	convex.mAovVertices = verts;

	// Calculate distance to center of sphere
	GjkInternal::Output gjkOutput;
	GjkInternal::Status status = GjkInternal::CollidePoint(convex, sphereCenterLocal, gjkOutput);

	if (status == GjkInternal::STATUS_DEGENERATE)
	{
		FloatV sphereRadiusV = FLoad(sphereRadius);
		status = FAllGrtr(gjkOutput.mDistSq, FMul(sphereRadiusV, sphereRadiusV)) ? GjkInternal::STATUS_NON_INTERSECT : GjkInternal::STATUS_CONTACT;
	}

	switch (status)
	{
		case GjkInternal::STATUS_CONTACT:
		{
			if (separation)
			{
				_calcSeparation(convex, hullLocalToWorldRT, sphereWorldCenterIn, sphereRadius, gjkOutput, *separation);
			}
			return true;
		}
		default:
		case GjkInternal::STATUS_NON_INTERSECT:
		{
			if (separation)
			{
				_calcSeparation(convex, hullLocalToWorldRT, sphereWorldCenterIn, sphereRadius, gjkOutput, *separation);
			}
			const PxF32 maxTotal = maxDistance + sphereRadius;
			return FStore(gjkOutput.mDistSq) < (maxTotal * maxTotal);
		}
	}
}

#if NX_SDK_VERSION_MAJOR == 2
bool ConvexHull::intersects(const NxShape& shape, const physx::PxMat34Legacy& localToWorldRT, const physx::PxVec3& scale, physx::PxF32 padding) const
{
	PX_UNUSED(localToWorldRT);
	PX_UNUSED(scale);
	PX_UNUSED(padding);

	switch (shape.getType())
	{
	case NX_SHAPE_PLANE:	// xxx todo - implement
		return true;
	case NX_SHAPE_SPHERE:	// xxx todo - implement
		return true;
	case NX_SHAPE_BOX:	// xxx todo - implement
		return true;
	case NX_SHAPE_CAPSULE:	// xxx todo - implement
		return true;
	case NX_SHAPE_CONVEX:	// xxx todo - implement
		return true;
	case NX_SHAPE_MESH:	// xxx todo - implement
		return true;
	case NX_SHAPE_HEIGHTFIELD:	// xxx todo - implement
		return true;
	default:
		return false;
	}
}
#elif NX_SDK_VERSION_MAJOR == 3
bool ConvexHull::intersects(const PxShape& shape, const physx::PxMat34Legacy& localToWorldRT, const physx::PxVec3& scale, physx::PxF32 padding) const
{
	PX_UNUSED(localToWorldRT);
	PX_UNUSED(scale);
	PX_UNUSED(padding);

	switch (shape.getGeometryType())
	{
	case PxGeometryType::ePLANE:	// xxx todo - implement
		return true;
	case PxGeometryType::eSPHERE:	// xxx todo - implement
		return true;
	case PxGeometryType::eBOX:	// xxx todo - implement
		return true;
	case PxGeometryType::eCAPSULE:	// xxx todo - implement
		return true;
	case PxGeometryType::eCONVEXMESH:	// xxx todo - implement
		return true;
	case PxGeometryType::eTRIANGLEMESH:	// xxx todo - implement
		return true;
	case PxGeometryType::eHEIGHTFIELD:	// xxx todo - implement
		return true;
	default:
		return false;
	}
}
#endif



bool ConvexHull::rayCast(physx::PxF32& in, physx::PxF32& out, const physx::PxVec3& orig, const physx::PxVec3& dir, const physx::PxMat34Legacy& localToWorldRT, const physx::PxVec3& scale, physx::PxVec3* normal) const
{
	const physx::PxU32 numSlabs = getUniquePlaneNormalCount();

	if (numSlabs == 0)
	{
		return false;
	}

	// Create hull-local ray
	physx::PxVec3 localorig, localdir;
	if (!worldToLocalRay(localorig, localdir, orig, dir, localToWorldRT, scale))
	{
		return false;
	}

	// Intersect with hull - local and world intersect 'times' are equal

	if (normal != NULL)
	{
		*normal = physx::PxVec3(0.0f);	// This will be the value if the ray origin is within the hull
	}

	const physx::PxF32 tol2 = (float)1.0e-14 * localdir.magnitudeSquared();

	for (physx::PxU32 slabN = 0; slabN < numSlabs; ++slabN)
	{
		ConvexHullParametersNS::Plane_Type& paramPlane = mParams->uniquePlanes.buf[slabN];
		const physx::PxPlane plane(paramPlane.normal, paramPlane.d);
		const physx::PxF32 num0 = -plane.distance(localorig);
		const physx::PxF32 num1 = mParams->widths.buf[slabN] - num0;
		const physx::PxF32 den = localdir.dot(plane.n);
		if (den* den <= tol2)	// Needs to be <=, so that localdir = (0,0,0) will act as a point check
		{
			if (num0 < 0 || num1 < 0)
			{
				return false;
			}
		}
		else
		{
			if (den > 0)
			{
				if (num0 < in * den || num1 < out * (-den))
				{
					return false;
				}
				const physx::PxF32 recipDen = 1.0f / den;
				const physx::PxF32 slabIn = -num1 * recipDen;
				if (slabIn > in)
				{
					in = slabIn;
					if (normal != NULL)
					{
						*normal = -plane.n;
					}
				}
				out = physx::PxMin(num0 * recipDen, out);
			}
			else
			{
				if (num0 < out * den || num1 < in * (-den))
				{
					return false;
				}
				const physx::PxF32 recipDen = 1.0f / den;
				const physx::PxF32 slabIn = num0 * recipDen;
				if (slabIn > in)
				{
					in = slabIn;
					if (normal != NULL)
					{
						*normal = plane.n;
					}
				}
				out = physx::PxMin(-num1 * recipDen, out);
			}
		}
	}

	if (normal != NULL)
	{
		NiCof44 cof(localToWorldRT, scale);
		*normal = cof.getBlock33().transform(*normal);
		normal->normalize();
	}

	return true;
}

bool ConvexHull::obbSweep(physx::PxF32& in, physx::PxF32& out, const physx::PxVec3& worldBoxCenter, const physx::PxVec3& worldBoxExtents, const physx::PxVec3 worldBoxAxes[3],
                          const physx::PxVec3& worldDisp, const physx::PxMat34Legacy& localToWorldRT, const physx::PxVec3& scale, physx::PxVec3* normal) const
{
	physx::PxF32 tNormal = -PX_MAX_F32;

	// Leave hull untransformed
	const physx::PxU32 numHullSlabs = getUniquePlaneNormalCount();
	const physx::PxU32 numHullVerts = getVertexCount();
	ConvexHullParametersNS::Plane_Type* paramPlanes = mParams->uniquePlanes.buf;
	const physx::PxVec3* hullVerts = mParams->vertices.buf;
	const physx::PxF32* hullWidths = mParams->widths.buf;

	// Create inverse transform for box and displacement
	const physx::PxF32 detS = scale.x * scale.y * scale.z;
	if (detS == 0.0f)
	{
		return false;	// Not handling singular TMs
	}
	const physx::PxF32 recipDetS = 1.0f / detS;
	const physx::PxVec3 invS(scale.y * scale.z * recipDetS, scale.z * scale.x * recipDetS, scale.x * scale.y * recipDetS);

	// Create box directions and displacement - the box will become a parallelepiped in general.  For brevity we'll still call it a box.
	physx::PxVec3 disp;
	localToWorldRT.M.multiplyByTranspose(worldDisp, disp);
	disp = invS.multiply(disp);

	physx::PxVec3 boxCenter;
	localToWorldRT.multiplyByInverseRT(worldBoxCenter, boxCenter);
	boxCenter = invS.multiply(boxCenter);

	physx::PxVec3 boxAxes[3];	// Will not be orthonormal in general
	for (physx::PxU32 i = 0; i < 3; ++i)
	{
		localToWorldRT.M.multiplyByTranspose(worldBoxAxes[i], boxAxes[i]);
		boxAxes[i] *= worldBoxExtents[i];
		boxAxes[i] = invS.multiply(boxAxes[i]);
	}

	physx::PxVec3 boxFaceNormals[3];
	physx::PxF32 boxRadii[3];
	const physx::PxF32 octantVol = boxAxes[0].dot(boxAxes[1].cross(boxAxes[2]));
	for (physx::PxU32 i = 0; i < 3; ++i)
	{
		boxFaceNormals[i] = boxAxes[(1 << i) & 3].cross(boxAxes[(3 >> i) ^ 1]);
		const physx::PxF32 norm = physx::PxRecipSqrt(boxFaceNormals[i].magnitudeSquared());
		boxFaceNormals[i] *= norm;
		boxRadii[i] = octantVol * norm;
	}

	// Test box against slabs of hull
	for (physx::PxU32 slabIndex = 0; slabIndex < numHullSlabs; ++slabIndex)
	{
		ConvexHullParametersNS::Plane_Type& paramPlane = paramPlanes[slabIndex];
		const physx::PxPlane plane(paramPlane.normal, paramPlane.d);
		const physx::PxF32 projectedRadius = physx::PxAbs(plane.n.dot(boxAxes[0])) + physx::PxAbs(plane.n.dot(boxAxes[1])) + physx::PxAbs(plane.n.dot(boxAxes[2]));
		const physx::PxF32 projectedCenter = plane.n.dot(boxCenter);
		const physx::PxF32 vel0 = disp.dot(plane.n);
		physx::PxF32 tIn, tOut;
		extentOverlapTimeInterval(tIn, tOut, vel0, projectedCenter - projectedRadius, projectedCenter + projectedRadius, -plane.d - hullWidths[slabIndex], -plane.d);
		if (!updateTimeIntervalAndNormal(in, out, tNormal, normal, tIn, tOut, (-PxSign(vel0))*plane.n))
		{
			return false;	// No intersection within the input time interval
		}
	}

	// Test hull against box face directions
	for (physx::PxU32 faceIndex = 0; faceIndex < 3; ++faceIndex)
	{
		const physx::PxVec3& faceNormal = boxFaceNormals[faceIndex];
		physx::PxF32 min, max;
		getExtent(min, max, hullVerts, numHullVerts, sizeof(physx::PxVec3), faceNormal);
		const physx::PxF32 projectedRadius = boxRadii[faceIndex];
		const physx::PxF32 projectedCenter = faceNormal.dot(boxCenter);
		const physx::PxF32 vel0 = disp.dot(faceNormal);
		physx::PxF32 tIn, tOut;
		extentOverlapTimeInterval(tIn, tOut, vel0, projectedCenter - projectedRadius, projectedCenter + projectedRadius, min, max);
		physx::PxVec3 testFace = (-PxSign(vel0)) * faceNormal;
		if (!updateTimeIntervalAndNormal(in, out, tNormal, normal, tIn, tOut, testFace))
		{
			return false;	// No intersection within the input time interval
		}
	}

	// Test hulls against cross-edge planes
	const physx::PxU32 numHullEdges = getUniqueEdgeDirectionCount();
	for (physx::PxU32 hullEdgeIndex = 0; hullEdgeIndex < numHullEdges; ++hullEdgeIndex)
	{
		const physx::PxVec3 hullEdge = getEdgeDirection(hullEdgeIndex);
		for (physx::PxU32 boxEdgeIndex = 0; boxEdgeIndex < 3; ++boxEdgeIndex)
		{
			physx::PxVec3 n = hullEdge.cross(boxAxes[boxEdgeIndex]);
			const physx::PxF32 n2 = n.magnitudeSquared();
			if (n2 < PX_EPS_F32 * PX_EPS_F32)
			{
				continue;
			}
			n *= physx::recipSqrtFast(n2);
			physx::PxF32 vel0 = disp.dot(n);
			// Choose normal direction such that the normal component of velocity is negative
			if (vel0 > 0.0f)
			{
				vel0 = -vel0;
				n *= -1.0f;
			}
			const physx::PxF32 projectedRadius = physx::PxAbs(n.dot(boxAxes[(1 << boxEdgeIndex) & 3])) +
			                                     physx::PxAbs(n.dot(boxAxes[(3 >> boxEdgeIndex) ^ 1]));
			const physx::PxF32 projectedCenter = n.dot(boxCenter);
			physx::PxF32 min, max;
			getExtent(min, max, hullVerts, numHullVerts, sizeof(physx::PxVec3), n);
			physx::PxF32 tIn, tOut;
			extentOverlapTimeInterval(tIn, tOut, vel0, projectedCenter - projectedRadius, projectedCenter + projectedRadius, min, max);
			if (!updateTimeIntervalAndNormal(in, out, tNormal, normal, tIn, tOut, n))
			{
				return false;	// No intersection within the input time interval
			}
		}
	}

	if (normal != NULL)
	{
		NiCof44 cof(localToWorldRT, scale);
		*normal = cof.getBlock33().transform(*normal);
		normal->normalize();
	}

	return true;
}

void ConvexHull::extent(physx::PxF32& min, physx::PxF32& max, const physx::PxVec3& normal) const
{
	getExtent(min, max, mParams->vertices.buf, (physx::PxU32)mParams->vertices.arraySizes[0], sizeof(physx::PxVec3), normal);
}

void ConvexHull::fill(physx::Array<physx::PxVec3>& outPoints, const physx::PxMat34Legacy& localToWorldRT, const physx::PxVec3& scale,
                      physx::PxF32 spacing, physx::PxF32 jitter, physx::PxU32 maxPoints, bool adjustSpacing) const
{
	if (!maxPoints)
	{
		return;
	}

	const physx::PxU32 numPlanes = getPlaneCount();
	physx::PxPlane* hull = (physx::PxPlane*)PxAlloca(numPlanes * sizeof(physx::PxPlane));
	physx::PxF32* recipDens = (physx::PxF32*)PxAlloca(numPlanes * sizeof(physx::PxF32));

	const NiCof44 cof(localToWorldRT, scale);	// matrix of cofactors to correctly transform planes
	physx::PxMat34Legacy srt = localToWorldRT;
	srt.M.multiplyDiagonal(scale);

	physx::PxBounds3 bounds;
	bounds.setEmpty();
	for (physx::PxU32 vertN = 0; vertN < getVertexCount(); ++vertN)
	{
		physx::PxVec3 worldVert;
		srt.multiply(getVertex(vertN), worldVert);
		bounds.include(worldVert);
	}

	physx::PxVec3 center, extents;
	center = bounds.getCenter();
	extents = bounds.getExtents();

	const physx::PxVec3 areas(extents[1]*extents[2], extents[2]*extents[0], extents[0]*extents[1]);
	const physx::PxU32 axisN = areas[0] < areas[1] ? (areas[0] < areas[2] ? 0u : 2u) : (areas[1] < areas[2] ? 1u : 2u);
	const physx::PxU32 axisN1 = (axisN + 1) % 3;
	const physx::PxU32 axisN2 = (axisN + 2) % 3;

	if (adjustSpacing)
	{
		const physx::PxF32 boxVolume = 8 * extents[0] * areas[0];
		const physx::PxF32 cellVolume = spacing * spacing * spacing;
		if (boxVolume > maxPoints * cellVolume)
		{
			spacing = physx::PxPow(boxVolume / maxPoints, 0.333333333f);
		}
	}

	for (physx::PxU32 planeN = 0; planeN < numPlanes; ++planeN)
	{
		cof.transform(getPlane(planeN), hull[planeN]);
		recipDens[planeN] = physx::PxAbs(hull[planeN].n[axisN]) > 1.0e-7 ? 1.0f / hull[planeN].n[axisN] : 0.0f;
	}

	const physx::PxF32 recipSpacing = 1.0f / spacing;
	const physx::PxI32 num1 = (physx::PxI32)(extents[axisN1] * recipSpacing);
	const physx::PxI32 num2 = (physx::PxI32)(extents[axisN2] * recipSpacing);

	QDSRand rnd;
	const physx::PxF32 scaledJitter = jitter * spacing;

	physx::PxVec3 orig;
	for (physx::PxI32 i1 = -num1; i1 <= num1; ++i1)
	{
		orig[axisN1] = i1 * spacing + center[axisN1];
		for (physx::PxI32 i2 = -num2; i2 <= num2; ++i2)
		{
			orig[axisN2] = i2 * spacing + center[axisN2];

			physx::PxF32 out = extents[axisN];
			physx::PxF32 in = -out;

			orig[axisN] = center[axisN];

			physx::PxU32 planeN;
			for (planeN = 0; planeN < numPlanes; ++planeN)
			{
				const physx::PxPlane& plane = hull[planeN];
				const physx::PxF32 recipDen = recipDens[planeN];
				const physx::PxF32 num = -plane.distance(orig);
				if (recipDen == 0.0f)
				{
					if (num < 0)
					{
						break;
					}
				}
				else
				{
					const physx::PxF32 t = num * recipDen;
					if (recipDen > 0)
					{
						if (t < in)
						{
							break;
						}
						out = physx::PxMin(t, out);
					}
					else
					{
						if (t > out)
						{
							break;
						}
						in = physx::PxMax(t, in);
					}
				}
			}

			if (planeN == numPlanes)
			{
				const physx::PxF32 depth = out - in;
				const physx::PxF32 stop = orig[axisN] + out;
				orig[axisN] += in + 0.5f * (depth - spacing * (int)(depth * recipSpacing));
				do
				{
					outPoints.pushBack(orig + scaledJitter * physx::PxVec3(rnd.getNext(), rnd.getNext(), rnd.getNext()));
					if (!--maxPoints)
					{
						return;
					}
				}
				while ((orig[axisN] += spacing) <= stop);
			}
		}
	}
}

physx::PxU32 ConvexHull::calculateCookedSizes(physx::PxU32& vertexCount, physx::PxU32& faceCount, bool inflated) const
{
	vertexCount = 0;
	faceCount = 0;
	PxMemoryBuffer memStream;
	memStream.setEndianMode(physx::PxFileBuf::ENDIAN_NONE);
	NxFromPxStream nxs(memStream);
#if NX_SDK_VERSION_MAJOR == 2
	NxConvexMeshDesc meshDesc;
	meshDesc.numVertices = getVertexCount();
	meshDesc.points = &getVertex(0);
	meshDesc.pointStrideBytes = sizeof(physx::PxVec3);
	meshDesc.flags = NX_CF_COMPUTE_CONVEX | NX_CF_USE_UNCOMPRESSED_NORMALS;
	const physx::PxF32 skinWidth = NxGetApexSDK()->getCookingInterface() != NULL ? NxGetApexSDK()->getCookingInterface()->NxGetCookingParams().skinWidth : 0.0f;
	if (inflated && skinWidth > 0.0f)
	{
		meshDesc.flags |= NX_CF_INFLATE_CONVEX;
	}
	if (NxGetApexSDK()->getCookingInterface()->NxCookConvexMesh(meshDesc, nxs))
	{
		NxConvexMesh* convexMesh = NxGetApexSDK()->getPhysXSDK()->createConvexMesh(nxs);
		if (convexMesh != NULL)
		{
			vertexCount = convexMesh->getCount(0, NX_ARRAY_VERTICES);
			faceCount = convexMesh->getCount(0, NX_ARRAY_HULL_POLYGONS);
		}
		NxGetApexSDK()->getPhysXSDK()->releaseConvexMesh(*convexMesh);
	}		
#elif NX_SDK_VERSION_MAJOR == 3
	PxConvexMeshDesc meshDesc;
	meshDesc.points.count = getVertexCount();
	meshDesc.points.data = &getVertex(0);
	meshDesc.points.stride = sizeof(physx::PxVec3);
	meshDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;
	const physx::PxF32 skinWidth = NxGetApexSDK()->getCookingInterface() != NULL ? NxGetApexSDK()->getCookingInterface()->getParams().skinWidth : 0.0f;
	if (inflated && skinWidth > 0.0f)
	{
		meshDesc.flags |= PxConvexFlag::eINFLATE_CONVEX;
	}
	if (NxGetApexSDK()->getCookingInterface()->cookConvexMesh(meshDesc, nxs))
	{
		PxConvexMesh* convexMesh = NxGetApexSDK()->getPhysXSDK()->createConvexMesh(nxs);
		if (convexMesh != NULL)
		{
			vertexCount = convexMesh->getNbVertices();
			faceCount = convexMesh->getNbPolygons();
		}
		convexMesh->release();
	}
#endif
	return memStream.getFileLength();
}

bool ConvexHull::reduceByCooking()
{
	bool reduced = false;
	PxMemoryBuffer memStream;
	memStream.setEndianMode(physx::PxFileBuf::ENDIAN_NONE);
	NxFromPxStream nxs(memStream);
#if NX_SDK_VERSION_MAJOR == 2
	NxConvexMeshDesc meshDesc;
	meshDesc.numVertices = getVertexCount();
	meshDesc.points = &getVertex(0);
	meshDesc.pointStrideBytes = sizeof(physx::PxVec3);
	meshDesc.flags = NX_CF_COMPUTE_CONVEX;
	if (NxGetApexSDK()->getCookingInterface()->NxCookConvexMesh(meshDesc, nxs))
	{
		NxConvexMesh* convexMesh = NxGetApexSDK()->getPhysXSDK()->createConvexMesh(nxs);
		if (convexMesh != NULL)
		{
			const physx::PxU32 vertexCount = convexMesh->getCount(0, NX_ARRAY_VERTICES);
			reduced = vertexCount < getVertexCount();
			if (reduced)
			{
				buildFromConvexMesh(convexMesh);
			}
		}
		NxGetApexSDK()->getPhysXSDK()->releaseConvexMesh(*convexMesh);
	}		
#elif NX_SDK_VERSION_MAJOR == 3
	PxConvexMeshDesc meshDesc;
	meshDesc.points.count = getVertexCount();
	meshDesc.points.data = &getVertex(0);
	meshDesc.points.stride = sizeof(physx::PxVec3);
	meshDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;
	if (NxGetApexSDK()->getCookingInterface()->cookConvexMesh(meshDesc, nxs))
	{
		PxConvexMesh* convexMesh = NxGetApexSDK()->getPhysXSDK()->createConvexMesh(nxs);
		if (convexMesh != NULL)
		{
			const physx::PxU32 vertexCount = convexMesh->getNbVertices();
			reduced = vertexCount < getVertexCount();
			if (reduced)
			{
				buildFromConvexMesh(convexMesh);
			}
		}
		convexMesh->release();
	}
#endif
	return reduced;
}

struct IndexedAngle
{
	physx::PxF32 angle;
	physx::PxU32 index;

	struct LT
	{
		PX_INLINE bool operator()(const IndexedAngle& a, const IndexedAngle& b) const
		{
			return a.angle < b.angle;
		}
	};
};

bool ConvexHull::reduceHull(physx::PxU32 maxVertexCount, physx::PxU32 maxEdgeCount, physx::PxU32 maxFaceCount, bool inflated)
{
	// Translate max = 0 => max = "infinity"
	if (maxVertexCount == 0)
	{
		maxVertexCount = 0xFFFFFFFF;
	}

	if (maxEdgeCount == 0)
	{
		maxEdgeCount = 0xFFFFFFFF;
	}

	if (maxFaceCount == 0)
	{
		maxFaceCount = 0xFFFFFFFF;
	}

	while (reduceByCooking()) {}

	// Calculate sizes
	physx::PxU32 cookedVertexCount;
	physx::PxU32 cookedFaceCount;
	calculateCookedSizes(cookedVertexCount, cookedFaceCount, inflated);
	physx::PxU32 cookedEdgeCount = cookedVertexCount + cookedFaceCount - 2;

	// Return successfully if we've met the required limits
	if (cookedVertexCount <= maxVertexCount && cookedEdgeCount <= maxEdgeCount && cookedFaceCount <= maxFaceCount)
	{
		return true;
	}

	NxParameterized::Handle handle(*mParams, "vertices");
	physx::PxU32 vertexCount = 0;
	mParams->getArraySize(handle, (physx::PxI32&)vertexCount);
	if (vertexCount < 4)
	{
		return false;
	}
	physx::Array<physx::PxVec3> vertices(vertexCount);
	mParams->getParamVec3Array(handle, &vertices[0], (physx::PxI32)vertexCount);

	// We have at least four vertices in our hull.  Find the tetrahedron with the largest volume.
	physx::PxMat44 tetrahedron;
	physx::PxF32 maxTetVolume = 0.0f;
	physx::PxU32 tetIndices[4] = {0,1,2,3};
	for (physx::PxU32 i = 0; i < vertexCount-3; ++i)
	{
		tetrahedron.column0 = physx::PxVec4(vertices[i], 1.0f);
		for (physx::PxU32 j = i+1; j < vertexCount-2; ++j)
		{
			tetrahedron.column1 = physx::PxVec4(vertices[j], 1.0f);
			for (physx::PxU32 k = j+1; k < vertexCount-1; ++k)
			{
				tetrahedron.column2 = physx::PxVec4(vertices[k], 1.0f);
				for (physx::PxU32 l = k+1; l < vertexCount; ++l)
				{
					tetrahedron.column3 = physx::PxVec4(vertices[l], 1.0f);
					const physx::PxF32 v = physx::PxAbs(det(tetrahedron));
					if (v > maxTetVolume)
					{
						maxTetVolume = v;
						tetIndices[0] = i;
						tetIndices[1] = j;
						tetIndices[2] = k;
						tetIndices[3] = l;
					}
				}
			}
		}
	}

	// Remove tetradhedral vertices from vertices array, put in reducedVertices array
	physx::Array<physx::PxVec3> reducedVertices(4);
	for (physx::PxU32 vNum = 4; vNum--;)
	{
		reducedVertices[vNum] = vertices[tetIndices[vNum]];
		vertices.replaceWithLast(tetIndices[vNum]);	// Works because tetIndices[i] < tetIndices[j] if i < j
	}
	buildFromPoints(&reducedVertices[0], 4, sizeof(physx::PxVec3));

	calculateCookedSizes(cookedVertexCount, cookedFaceCount, inflated);
	cookedEdgeCount = cookedVertexCount + cookedFaceCount - 2;
	if (cookedVertexCount > maxVertexCount || cookedEdgeCount > maxEdgeCount || cookedFaceCount > maxFaceCount)
	{
		return false;	// Even a tetrahedron exceeds our limits
	}

	physx::Array<physx::PxU32> faceEdges;
	while (vertices.size())
	{
		physx::PxF32 maxVolumeIncrease = 0.0f;
		physx::PxU32 bestVertexIndex = 0;
		for (physx::PxU32 testVertexIndex = 0; testVertexIndex < vertices.size(); ++testVertexIndex)
		{
			const physx::PxVec3& testVertex = vertices[testVertexIndex];
			physx::PxF32 volumeIncrease = 0.0f;
			physx::PxMat44 addedTet;
			addedTet.column0 = physx::PxVec4(testVertex, 1.0f);
			for (physx::PxU32 planeIndex = 0; planeIndex < getPlaneCount(); ++planeIndex)
			{
				const physx::PxPlane plane = getPlane(planeIndex);
				if (plane.distance(testVertex) <= 0.0f)
				{
					continue;
				}
				faceEdges.resize(0);
				physx::PxVec3 faceCenter(0.0f);
				for (physx::PxU32 edgeIndex = 0; edgeIndex < getEdgeCount(); ++edgeIndex)
				{
					if (getEdgeAdjacentFaceIndex(edgeIndex, 0) == planeIndex || getEdgeAdjacentFaceIndex(edgeIndex, 1) == planeIndex)
					{
						faceCenter += getVertex(getEdgeEndpointIndex(edgeIndex, 0)) + getVertex(getEdgeEndpointIndex(edgeIndex, 1));
						faceEdges.pushBack(edgeIndex);
					}
				}
				if (faceEdges.size())
				{
					faceCenter *= 0.5f/(physx::PxF32)faceEdges.size();
				}
				addedTet.column1 = physx::PxVec4(faceCenter, 1.0f);
				for (physx::PxU32 edgeNum = 0; edgeNum < faceEdges.size(); ++edgeNum)
				{
					const physx::PxU32 edgeIndex = faceEdges[edgeNum];
					addedTet.column2 = physx::PxVec4(getVertex(getEdgeEndpointIndex(edgeIndex,0)), 1.0f);
					addedTet.column3 = physx::PxVec4(getVertex(getEdgeEndpointIndex(edgeIndex,1)), 1.0f);
					volumeIncrease += physx::PxAbs(det(addedTet));
				}
			}
			if (volumeIncrease > maxVolumeIncrease)
			{
				maxVolumeIncrease = volumeIncrease;
				bestVertexIndex = testVertexIndex;
			}
		}

		reducedVertices.pushBack(vertices[bestVertexIndex]);
		vertices.replaceWithLast(bestVertexIndex);
		buildFromPoints(&reducedVertices[0], reducedVertices.size(), sizeof(physx::PxVec3));

		calculateCookedSizes(cookedVertexCount, cookedFaceCount, inflated);
		cookedEdgeCount = cookedVertexCount + cookedFaceCount - 2;
		if (cookedVertexCount > maxVertexCount || cookedEdgeCount > maxEdgeCount || cookedFaceCount > maxFaceCount)
		{
			// Exceeded limits, remove last
			reducedVertices.popBack();
			buildFromPoints(&reducedVertices[0], reducedVertices.size(), sizeof(physx::PxVec3));
			break;
		}
	}

	return true;
}

bool ConvexHull::createKDOPDirections(physx::Array<physx::PxVec3>& directions, NxConvexHullMethod::Enum method)
{
	physx::PxU32 dirs;
	switch (method)
	{
	case NxConvexHullMethod::USE_6_DOP:
		dirs = 0;
		break;
	case NxConvexHullMethod::USE_10_DOP_X:
		dirs = 1;
		break;
	case NxConvexHullMethod::USE_10_DOP_Y:
		dirs = 2;
		break;
	case NxConvexHullMethod::USE_10_DOP_Z:
		dirs = 4;
		break;
	case NxConvexHullMethod::USE_14_DOP_XY:
		dirs = 3;
		break;
	case NxConvexHullMethod::USE_14_DOP_YZ:
		dirs = 6;
		break;
	case NxConvexHullMethod::USE_14_DOP_ZX:
		dirs = 5;
		break;
	case NxConvexHullMethod::USE_18_DOP:
		dirs = 7;
		break;
	case NxConvexHullMethod::USE_26_DOP:
		dirs = 15;
		break;
	default:
		return false;
	}
	directions.reset();
	directions.pushBack(physx::PxVec3(1, 0, 0));
	directions.pushBack(physx::PxVec3(0, 1, 0));
	directions.pushBack(physx::PxVec3(0, 0, 1));
	if (dirs & 1)
	{
		directions.pushBack(physx::PxVec3(0, 1, 1));
		directions.pushBack(physx::PxVec3(0, -1, 1));
	}
	if (dirs & 2)
	{
		directions.pushBack(physx::PxVec3(1, 0, 1));
		directions.pushBack(physx::PxVec3(1, 0, -1));
	}
	if (dirs & 4)
	{
		directions.pushBack(physx::PxVec3(1, 1, 0));
		directions.pushBack(physx::PxVec3(-1, 1, 0));
	}
	if (dirs & 8)
	{
		directions.pushBack(physx::PxVec3(1, 1, 1));
		directions.pushBack(physx::PxVec3(-1, 1, 1));
		directions.pushBack(physx::PxVec3(1, -1, 1));
		directions.pushBack(physx::PxVec3(1, 1, -1));
	}
	return true;
}


void createIndexStartLookup(physx::Array<physx::PxU32>& lookup, physx::PxI32 indexBase, physx::PxU32 indexRange, physx::PxI32* indexSource, physx::PxU32 indexCount, physx::PxU32 indexByteStride)
{
	if (indexRange == 0)
	{
		lookup.resize(physx::PxMax(indexRange + 1, 2u));
		lookup[0] = 0;
		lookup[1] = indexCount;
	}
	else
	{
		lookup.resize(indexRange + 1);
		physx::PxU32 indexPos = 0;
		for (physx::PxU32 i = 0; i < indexRange; ++i)
		{
			for (; indexPos < indexCount; ++indexPos, indexSource = (physx::PxI32*)((uintptr_t)indexSource + indexByteStride))
			{
				if (*indexSource >= (physx::PxI32)i + indexBase)
				{
					lookup[i] = indexPos;
					break;
				}
			}
			if (indexPos == indexCount)
			{
				lookup[i] = indexPos;
			}
		}
		lookup[indexRange] = indexCount;
	}
}

void findIslands(physx::Array< physx::Array<physx::PxU32> >& islands, const physx::Array<IntPair>& overlaps, physx::PxU32 indexRange)
{
	// symmetrize the overlap list
	physx::Array<IntPair> symOverlaps;
	symOverlaps.reserve(2 * overlaps.size());
	for (physx::PxU32 i = 0; i < overlaps.size(); ++i)
	{
		const IntPair& pair = overlaps[i];
		symOverlaps.pushBack(pair);
		IntPair& reversed = symOverlaps.insert();
		reversed.set(pair.i1, pair.i0);
	}
	// Sort symmetrized list
	qsort(symOverlaps.begin(), symOverlaps.size(), sizeof(IntPair), IntPair::compare);
	// Create overlap look-up
	physx::Array<physx::PxU32> overlapStarts;
	createIndexStartLookup(overlapStarts, 0, indexRange, &symOverlaps.begin()->i0, symOverlaps.size(), sizeof(IntPair));
	// Find islands
	NxIndexBank<physx::PxU32> objectIndices(indexRange);
	objectIndices.lockCapacity(true);
	physx::PxU32 seedIndex = PX_MAX_U32;
	while (objectIndices.useNextFree(seedIndex))
	{
		physx::Array<physx::PxU32>& island = islands.insert();
		island.pushBack(seedIndex);
		for (physx::PxU32 i = 0; i < island.size(); ++i)
		{
			const physx::PxU32 index = island[i];
			for (physx::PxU32 j = overlapStarts[index]; j < overlapStarts[index + 1]; ++j)
			{
				PX_ASSERT(symOverlaps[j].i0 == (physx::PxI32)index);
				const physx::PxU32 overlapIndex = (physx::PxU32)symOverlaps[j].i1;
				if (objectIndices.use(overlapIndex))
				{
					island.pushBack(overlapIndex);
				}
			}
		}
	}
}


// Neighbor-finding utility class

void NeighborLookup::setBounds(const BoundsRep* bounds, physx::PxU32 boundsCount, physx::PxU32 boundsByteStride)
{
	m_neighbors.resize(0);
	m_firstNeighbor.resize(0);

	if (bounds != NULL && boundsCount > 0 && boundsByteStride >= sizeof(BoundsRep))
	{
		physx::Array<IntPair> overlaps;
		boundsCalculateOverlaps(overlaps, Bounds3XYZ, bounds, boundsCount, boundsByteStride);
		// symmetrize the overlap list
		const physx::PxU32 oldSize = overlaps.size();
		overlaps.resize(2 * oldSize);
		for (physx::PxU32 i = 0; i < oldSize; ++i)
		{
			const IntPair& pair = overlaps[i];
			overlaps[i+oldSize].set(pair.i1, pair.i0);
		}
		// Sort symmetrized list
		qsort(overlaps.begin(), overlaps.size(), sizeof(IntPair), IntPair::compare);
		// Create overlap look-up
		if (overlaps.size() > 0)
		{
			createIndexStartLookup(m_firstNeighbor, 0, boundsCount, &overlaps[0].i0, overlaps.size(), sizeof(IntPair));
			m_neighbors.resize(overlaps.size());
			for (physx::PxU32 i = 0; i < overlaps.size(); ++i)
			{
				m_neighbors[i] = (physx::PxU32)overlaps[i].i1;
			}
		}
	}
}

physx::PxU32 NeighborLookup::getNeighborCount(const physx::PxU32 index) const
{
	if (index + 1 >= m_firstNeighbor.size())
	{
		return 0;	// Invalid neighbor list or index
	}

	return m_firstNeighbor[index+1] - m_firstNeighbor[index];
}

const physx::PxU32* NeighborLookup::getNeighbors(const physx::PxU32 index) const
{
	if (index + 1 >= m_firstNeighbor.size())
	{
		return 0;	// Invalid neighbor list or index
	}

	return &m_neighbors[m_firstNeighbor[index]];
}


// Data conversion macros

#define copyBuffer( _DstFormat, _SrcFormat ) \
	{ \
		PxU32 _numVertices = numVertices; \
		PxI32* _invMap = invMap; \
		_DstFormat##_TYPE* _dst = (_DstFormat##_TYPE*)dst; \
		const _SrcFormat##_TYPE* _src = (const _SrcFormat##_TYPE*)src; \
		if( _invMap == NULL ) \
		{ \
			while( _numVertices-- ) \
			{ \
				convert_##_DstFormat##_from_##_SrcFormat( *_dst, *_src ); \
				((PxU8*&)_dst) += dstStride; \
				((const PxU8*&)_src) += srcStride; \
			} \
		} \
		else \
		{ \
			while( _numVertices-- ) \
			{ \
				const PxI32 invMapValue = *_invMap++; \
				if (invMapValue >= 0) \
				{ \
					_DstFormat##_TYPE* _dstElem = (_DstFormat##_TYPE*)((PxU8*)_dst + invMapValue*dstStride); \
					convert_##_DstFormat##_from_##_SrcFormat( *_dstElem, *_src ); \
				} \
				((const PxU8*&)_src) += srcStride; \
			} \
		} \
	}


#define HANDLE_COPY1( _DstFormat, _SrcFormat ) \
	case NxRenderDataFormat::_DstFormat : \
		if( srcFormat == NxRenderDataFormat::_SrcFormat ) \
		{ \
			copyBuffer( _DstFormat, _SrcFormat ); \
		} \
		break;

#define HANDLE_COPY2( _DstFormat, _SrcFormat1, _SrcFormat2 ) \
	case NxRenderDataFormat::_DstFormat : \
		if( srcFormat == NxRenderDataFormat::_SrcFormat1 ) \
		{ \
			copyBuffer( _DstFormat, _SrcFormat1 ); \
		} \
		else if( srcFormat == NxRenderDataFormat::_SrcFormat2 ) \
		{ \
			copyBuffer( _DstFormat, _SrcFormat2 ); \
		} \
		break;

#define HANDLE_COPY3( _DstFormat, _SrcFormat1, _SrcFormat2, _SrcFormat3 ) \
	case NxRenderDataFormat::_DstFormat : \
		if( srcFormat == NxRenderDataFormat::_SrcFormat1 ) \
		{ \
			copyBuffer( _DstFormat, _SrcFormat1 ); \
		} \
		else if( srcFormat == NxRenderDataFormat::_SrcFormat2 ) \
		{ \
			copyBuffer( _DstFormat, _SrcFormat2 ); \
		} \
		else if( srcFormat == NxRenderDataFormat::_SrcFormat3 ) \
		{ \
			copyBuffer( _DstFormat, _SrcFormat3 ); \
		} \
		break;

// ... etc.


bool copyRenderVertexBuffer(void* dst, NxRenderDataFormat::Enum dstFormat, PxU32 dstStride, PxU32 dstStart, const void* src, NxRenderDataFormat::Enum srcFormat, PxU32 srcStride, PxU32 srcStart, PxU32 numVertices, PxI32* invMap)
{
	const PxU32 dstDataSize = NxRenderDataFormat::getFormatDataSize(dstFormat);
	if (dstStride == 0)
	{
		dstStride = dstDataSize;
	}

	if (dstStride < dstDataSize)
	{
		return false;
	}

	// advance src pointer
	((const PxU8*&)src) += srcStart * srcStride;

	PX_ASSERT((dstStart == 0) || (invMap == NULL)); // can only be one of them, won't work if its both!

	if (dstFormat == srcFormat)
	{
		if (dstFormat != NxRenderDataFormat::UNSPECIFIED)
		{
			// Direct data copy

			// advance dst pointer
			((const PxU8*&)dst) += dstStart * dstStride;

			if (invMap == NULL)
			{
				while (numVertices--)
				{
					memcpy(dst, src, dstDataSize);
					((PxU8*&)dst) += dstStride;
					((const PxU8*&)src) += srcStride;
				}
			}
			else
			{
				while (numVertices--)
				{
					const PxI32 invMapValue = *invMap++;
					if (invMapValue >= 0)
					{
						void* mappedDst = (void*)((PxU8*)dst + dstStride * (invMapValue));
						memcpy(mappedDst, src, dstDataSize);
					}
					((const PxU8*&)src) += srcStride;
				}
			}
		}

		return true;
	}

	// advance dst pointer
	((const PxU8*&)dst) += dstStart * dstStride;

	switch (dstFormat)
	{
	case NxRenderDataFormat::UNSPECIFIED:
		// Handle unspecified by doing nothing (still no error!)
		break;

		// Put format converters here
		HANDLE_COPY1(USHORT1, UINT1)
		HANDLE_COPY1(USHORT2, UINT2)
		HANDLE_COPY1(USHORT3, UINT3)
		HANDLE_COPY1(USHORT4, UINT4)

		HANDLE_COPY1(UINT1, USHORT1)
		HANDLE_COPY1(UINT2, USHORT2)
		HANDLE_COPY1(UINT3, USHORT3)
		HANDLE_COPY1(UINT4, USHORT4)

		HANDLE_COPY1(BYTE_SNORM1, FLOAT1)
		HANDLE_COPY1(BYTE_SNORM2, FLOAT2)
		HANDLE_COPY1(BYTE_SNORM3, FLOAT3)
		HANDLE_COPY1(BYTE_SNORM4, FLOAT4)
		HANDLE_COPY1(BYTE_SNORM4_QUATXYZW, FLOAT4_QUAT)
		HANDLE_COPY1(SHORT_SNORM1, FLOAT1)
		HANDLE_COPY1(SHORT_SNORM2, FLOAT2)
		HANDLE_COPY1(SHORT_SNORM3, FLOAT3)
		HANDLE_COPY1(SHORT_SNORM4, FLOAT4)
		HANDLE_COPY1(SHORT_SNORM4_QUATXYZW, FLOAT4_QUAT)

		HANDLE_COPY2(FLOAT1, BYTE_SNORM1, SHORT_SNORM1)
		HANDLE_COPY2(FLOAT2, BYTE_SNORM2, SHORT_SNORM2)
		HANDLE_COPY2(FLOAT3, BYTE_SNORM3, SHORT_SNORM3)
		HANDLE_COPY2(FLOAT4, BYTE_SNORM4, SHORT_SNORM4)
		HANDLE_COPY2(FLOAT4_QUAT, BYTE_SNORM4_QUATXYZW, SHORT_SNORM4_QUATXYZW)

		HANDLE_COPY3(R8G8B8A8, B8G8R8A8, R32G32B32A32_FLOAT, B32G32R32A32_FLOAT)
		HANDLE_COPY3(B8G8R8A8, R8G8B8A8, R32G32B32A32_FLOAT, B32G32R32A32_FLOAT)
		HANDLE_COPY3(R32G32B32A32_FLOAT, R8G8B8A8, B8G8R8A8, B32G32R32A32_FLOAT)
		HANDLE_COPY3(B32G32R32A32_FLOAT, R8G8B8A8, B8G8R8A8, R32G32B32A32_FLOAT)

	default:
	{
		PX_ALWAYS_ASSERT();	// Format conversion not handled
		return false;
	}
	}


	return true;
}



/************************************************************************/
// Convex Hull from Planes
/************************************************************************/

// copied from //sw/physx/PhysXSDK/3.2/trunk/Source/Common/src/CmMathUtils.cpp
PxQuat PxShortestRotation(const PxVec3& v0, const PxVec3& v1)
{
	const PxReal d = v0.dot(v1);
	const PxVec3 cross = v0.cross(v1);

	PxQuat q = d>-1 ? PxQuat(cross.x, cross.y, cross.z, 1+d) 
		: PxAbs(v0.x)<0.1f ? PxQuat(0.0f, v0.z, -v0.y, 0.0f) : PxQuat(v0.y, -v0.x, 0.0f, 0.0f);

	return q.getNormalized();
}

PxTransform PxTransformFromPlaneEquation(const PxPlane& plane)
{
	PxPlane p = plane; 
	p.normalize();

	// special case handling for axis aligned planes
	const PxReal halfsqrt2 = PxReal(0.707106781);
	PxQuat q;
	if(2 == (p.n.x == 0.0f) + (p.n.y == 0.0f) + (p.n.z == 0.0f)) // special handling for axis aligned planes
	{
		if(p.n.x > 0)		q = PxQuat::createIdentity();
		else if(p.n.x < 0)	q = PxQuat(0, 0, 1, 0);
		else				q = PxQuat(0.0f, -p.n.z, p.n.y, 1) * halfsqrt2;
	}
	else q = PxShortestRotation(PxVec3(1,0,0), p.n);

	return PxTransform(-p.n * p.d, q);
}



PxVec3 intersect(PxVec4 p0, PxVec4 p1, PxVec4 p2)
{
	const PxVec3& d0 = reinterpret_cast<const PxVec3&>(p0);
	const PxVec3& d1 = reinterpret_cast<const PxVec3&>(p1);
	const PxVec3& d2 = reinterpret_cast<const PxVec3&>(p2);

	return (p0.w * d1.cross(d2) 
		  + p1.w * d2.cross(d0) 
		  + p2.w * d0.cross(d1))
		  / d0.dot(d2.cross(d1));
}

const PxU16 sInvalid = PxU16(-1);

// restriction: only supports a single patch per vertex.
struct HalfedgeMesh
{
	struct Halfedge
	{
		Halfedge(PxU16 vertex = sInvalid, PxU16 face = sInvalid, 
			PxU16 next = sInvalid, PxU16 prev = sInvalid)
			: mVertex(vertex), mFace(face), mNext(next), mPrev(prev)
		{}

		PxU16 mVertex; // to
		PxU16 mFace; // left
		PxU16 mNext; // ccw
		PxU16 mPrev; // cw
	};

	HalfedgeMesh() : mNumTriangles(0) {}

	PxU16 findHalfedge(PxU16 v0, PxU16 v1)
	{
		PxU16 h = mVertices[v0], start = h;
		while(h != sInvalid && mHalfedges[h].mVertex != v1)
		{
			h = mHalfedges[(physx::PxU32)h ^ 1].mNext;
			if(h == start) 
				return sInvalid;
		}
		return h;
	}

	void connect(PxU16 h0, PxU16 h1)
	{
		mHalfedges[h0].mNext = h1;
		mHalfedges[h1].mPrev = h0;
	}

	void addTriangle(PxU16 v0, PxU16 v1, PxU16 v2)
	{
		// add new vertices
		PxU16 n = (PxU16)(PxMax(v0, PxMax(v1, v2))+1);
		if(mVertices.size() < n)
			mVertices.resize(n, sInvalid);

		// collect halfedges, prev and next of triangle
		PxU16 verts[] = { v0, v1, v2 };
		PxU16 handles[3], prev[3], next[3];
		for(PxU16 i=0; i<3; ++i)
		{
			PxU16 j = (PxU16)((i+1)%3);
			PxU16 h = findHalfedge(verts[i], verts[j]);
			if(h == sInvalid)
			{
				// add new edge
				PX_ASSERT(mHalfedges.size() <= PX_MAX_U16);
				h = (PxU16)mHalfedges.size();
				mHalfedges.pushBack(Halfedge(verts[j]));
				mHalfedges.pushBack(Halfedge(verts[i]));
			}
			handles[i] = h;
			prev[i] = mHalfedges[h].mPrev;
			next[i] = mHalfedges[h].mNext;
		}

		// patch connectivity
		for(PxU16 i=0; i<3; ++i)
		{
			PxU16 j = (PxU16)((i+1)%3);

			PX_ASSERT(mFaces.size() <= PX_MAX_U16);
			mHalfedges[handles[i]].mFace = (PxU16)mFaces.size();

			// connect prev and next
			connect(handles[i], handles[j]);

			if(next[j] == sInvalid) // new next edge, connect opposite
				connect((PxU32)(handles[j]^1), next[i]!=sInvalid ? next[i] : (PxU32)(handles[i]^1));

			if(prev[i] == sInvalid) // new prev edge, connect opposite
				connect(prev[j]!=sInvalid ? prev[j] : (PxU32)(handles[j]^1), (PxU32)(handles[i]^1));

			// prev is boundary, update middle vertex
			if(mHalfedges[(PxU32)(handles[i]^1)].mFace == sInvalid)
				mVertices[verts[j]] = (PxU32)(handles[i]^1);
		}

		mFaces.pushBack(handles[2]);
		++mNumTriangles;
	}

	PxU16 removeTriangle(PxU16 f)
	{
		PxU16 result = sInvalid;

		for(PxU16 i=0, h = mFaces[f]; i<3; ++i)
		{
			PxU16 v0 = mHalfedges[(PxU32)(h^1)].mVertex;
			PxU16 v1 = mHalfedges[h].mVertex;

			mHalfedges[h].mFace = sInvalid;

			if(mHalfedges[(PxU32)(h^1)].mFace == sInvalid) // was boundary edge, remove
			{
				PxU16 v0Prev = mHalfedges[h  ].mPrev;
				PxU16 v0Next = mHalfedges[(PxU32)(h^1)].mNext;
				PxU16 v1Prev = mHalfedges[(PxU32)(h^1)].mPrev;
				PxU16 v1Next = mHalfedges[h  ].mNext;

				// update halfedge connectivity
				connect(v0Prev, v0Next);
				connect(v1Prev, v1Next);

				// update vertex boundary or delete
				mVertices[v0] = (v0Prev^1) == v0Next ? sInvalid : v0Next;
				mVertices[v1] = (v1Prev^1) == v1Next ? sInvalid : v1Next;
			} 
			else 
			{
				mVertices[v0] = h; // update vertex boundary
				result = v1;
			}

			h = mHalfedges[h].mNext;
		}

		mFaces[f] = sInvalid;
		--mNumTriangles;

		return result;
	}

	// true if vertex v is in front of face f
	bool visible(PxU16 v, PxU16 f)
	{
		PxU16 h = mFaces[f];
		if(h == sInvalid)
			return false;

		PxU16 v0 = mHalfedges[h].mVertex;
		h = mHalfedges[h].mNext;
		PxU16 v1 = mHalfedges[h].mVertex;
		h = mHalfedges[h].mNext;
		PxU16 v2 = mHalfedges[h].mVertex;
		h = mHalfedges[h].mNext;

		return det(mPoints[v], mPoints[v0], mPoints[v1], mPoints[v2]) < -1e-5f;
	}

	/*
	void print() const
	{
		for(PxU32 i=0; i<mFaces.size(); ++i)
		{
			printf("f%u: ", i);
			PxU16 h = mFaces[i];
			if(h == sInvalid)
			{
				printf("deleted\n");
				continue;
			}

			for(int j=0; j<3; ++j)
			{
				printf("h%u -> v%u -> ", PxU32(h), PxU32(mHalfedges[h].mVertex));
				h = mHalfedges[h].mNext;
			}

			printf("\n");
		}

		for(PxU32 i=0; i<mVertices.size(); ++i)
		{
			printf("v%u: ", i);
			PxU16 h = mVertices[i];
			if(h == sInvalid)
			{
				printf("deleted\n");
				continue;
			}
			
			PxU16 start = h;
			do {
				printf("h%u -> v%u, ", PxU32(h), PxU32(mHalfedges[h].mVertex));
				h = mHalfedges[h^1].mNext;
			} while (h != start);

			printf("\n");
		}

		for(PxU32 i=0; i<mHalfedges.size(); ++i)
			printf("h%u: v%u, f%u, p%u, n%u\n", i, PxU32(mHalfedges[i].mVertex),
				PxU32(mHalfedges[i].mFace), PxU32(mHalfedges[i].mPrev), PxU32(mHalfedges[i].mNext));
	}
	*/

	shdfnd::Array<Halfedge> mHalfedges;
	shdfnd::Array<PxU16> mVertices; // vertex -> (boundary) halfedge
	shdfnd::Array<PxU16> mFaces; // face -> halfedge
	shdfnd::Array<PxVec4> mPoints;
	PxU16 mNumTriangles;
};



void ConvexMeshBuilder::operator()(PxU32 planeMask, float scale)
{
	PxU32 numPlanes = shdfnd::bitCount(planeMask);

	if (numPlanes == 1)
	{
		PxTransform t = physx::apex::PxTransformFromPlaneEquation(reinterpret_cast<const PxPlane&>(mPlanes[lowestSetBit(planeMask)]));

		if (!t.isValid())
			return;

		const PxU16 indices[] = { 0, 1, 2, 0, 2, 3 };
		const PxVec3 vertices[] = { 
			PxVec3(0.0f,  scale,  scale), 
			PxVec3(0.0f, -scale,  scale),
			PxVec3(0.0f, -scale, -scale),			
			PxVec3(0.0f,  scale, -scale) };

			PX_ASSERT(mVertices.size() <= PX_MAX_U16);
			PxU16 baseIndex = (PxU16)mVertices.size();

			for (PxU32 i=0; i < 4; ++i)
				mVertices.pushBack(t.transform(vertices[i]));

			for (PxU32 i=0; i < 6; ++i)
				mIndices.pushBack((PxU16)(indices[i] + baseIndex));

			return;
	}

	if(numPlanes < 4)
		return; // todo: handle degenerate cases

	HalfedgeMesh mesh;

	// gather points (planes, that is)
	mesh.mPoints.reserve(numPlanes);
	for(; planeMask; planeMask &= planeMask-1)
		mesh.mPoints.pushBack(mPlanes[shdfnd::lowestSetBit(planeMask)]);

	// initialize to tetrahedron
	mesh.addTriangle(0, 1, 2);
	mesh.addTriangle(0, 3, 1);
	mesh.addTriangle(1, 3, 2);
	mesh.addTriangle(2, 3, 0);

	// flip if inside-out
	if(mesh.visible(3, 0))
		shdfnd::swap(mesh.mPoints[0], mesh.mPoints[1]);

	// iterate through remaining points
	for(PxU16 i=4; i<mesh.mPoints.size(); ++i)
	{
		// remove any visible triangle
		PxU16 v0 = sInvalid;
		for(PxU16 j=0; j<mesh.mFaces.size(); ++j)
		{
			if(mesh.visible(i, j))
				v0 = PxMin(v0, mesh.removeTriangle(j));
		}

		if(v0 == sInvalid)
			continue; // no triangle removed

		if(!mesh.mNumTriangles)
			return; // empty mesh

		// find non-deleted boundary vertex
		for(PxU16 h=0; mesh.mVertices[v0] == sInvalid; h+=2)
		{
			if ((mesh.mHalfedges[h  ].mFace == sInvalid) ^ 
				(mesh.mHalfedges[(PxU32)(h+1)].mFace == sInvalid))
			{
				v0 = mesh.mHalfedges[h].mVertex;
			}
		}

		// tesselate hole
		PxU16 start = v0;
		do {
			PxU16 h = mesh.mVertices[v0];
			PxU16 v1 = mesh.mHalfedges[h].mVertex;
			if(mesh.mFaces.size() == PX_MAX_U16)
				break; // safety net
			mesh.addTriangle(v0, v1, i);
			v0 = v1;
		} while(v0 != start);

		bool noHole = true;
		for(PxU16 h=0; noHole && h < mesh.mHalfedges.size(); h+=2)
		{
			if ((mesh.mHalfedges[h  ].mFace == sInvalid) ^ 
				(mesh.mHalfedges[(PxU32)(h+1)].mFace == sInvalid))
				noHole = false;
		}

		if(!noHole || mesh.mFaces.size() == PX_MAX_U16)
		{
			mesh.mFaces.resize(0);
			mesh.mVertices.resize(0);
			break;
		}
	}

	// convert triangles to vertices (intersection of 3 planes)
	shdfnd::Array<PxU32> face2Vertex(mesh.mFaces.size());
	for(PxU32 i=0; i<mesh.mFaces.size(); ++i)
	{
		face2Vertex[i] = mVertices.size();

		PxU16 h = mesh.mFaces[i];
		if(h == sInvalid)
			continue;

		PxU16 v0 = mesh.mHalfedges[h].mVertex;
		h = mesh.mHalfedges[h].mNext;
		PxU16 v1 = mesh.mHalfedges[h].mVertex;
		h = mesh.mHalfedges[h].mNext;
		PxU16 v2 = mesh.mHalfedges[h].mVertex;

		mVertices.pushBack(intersect(mesh.mPoints[v0], mesh.mPoints[v1], mesh.mPoints[v2]));
	}

	// convert vertices to polygons (face one-ring)
	for(PxU32 i=0; i<mesh.mVertices.size(); ++i)
	{
		PxU16 h = mesh.mVertices[i];
		if(h == sInvalid)
			continue;

		PxU32 v0 = face2Vertex[mesh.mHalfedges[h].mFace];
		h = (PxU16)(mesh.mHalfedges[h].mPrev^1);
		PxU32 v1 = face2Vertex[mesh.mHalfedges[h].mFace];

		while(true)
		{
			h = (PxU16)(mesh.mHalfedges[h].mPrev^1);
			PxU32 v2 = face2Vertex[mesh.mHalfedges[h].mFace];

			if(v0 == v2) 
				break;
			
			PX_ASSERT(v0 <= PX_MAX_U16);
			PX_ASSERT(v1 <= PX_MAX_U16);
			PX_ASSERT(v2 <= PX_MAX_U16);
			mIndices.pushBack((PxU16)v0);
			mIndices.pushBack((PxU16)v2);
			mIndices.pushBack((PxU16)v1);

			v1 = v2; 
		}

	}
}

// Diagonalize a symmetric 3x3 matrix.  Returns the eigenvectors in the first parameter, eigenvalues as the return value.
PxVec3 diagonalizeSymmetric(PxMat33& eigenvectors, const PxMat33& m)
{
	// jacobi rotation using quaternions (from an idea of Stan Melax, with fix for precision issues)

	const PxU32 MAX_ITERS = 24;

	PxQuat q = PxQuat::createIdentity();

	PxMat33 d;
	for(PxU32 i=0; i < MAX_ITERS;i++)
	{
		eigenvectors = PxMat33(q);
		d = eigenvectors.getTranspose() * m * eigenvectors;

		const PxReal d0 = PxAbs(d[1][2]), d1 = PxAbs(d[0][2]), d2 = PxAbs(d[0][1]);
		const PxU32 a = d0 > d1 && d0 > d2 ? 0u : d1 > d2 ? 1u : 2u;						// rotation axis index, from largest off-diagonal element

		const PxU32 a1 = (a+1)%3;
		const PxU32 a2 = (a+2)%3;											
		if(d[a1][a2] == 0.0f || PxAbs(d[a1][a1]-d[a2][a2]) > 2e6*PxAbs(2.0f*d[a1][a2]))
		{
			break;
		}

		const PxReal w = (d[a1][a1]-d[a2][a2]) / (2.0f*d[a1][a2]);					// cot(2 * phi), where phi is the rotation angle
		const PxReal absw = PxAbs(w);

		PxReal c, s;
		if(absw>1000)
		{
			c = 1;
			s = 1/(4*w);														// h will be very close to 1, so use small angle approx instead
		}
		else
		{
			PxReal t = 1 / (absw + PxSqrt(w*w+1));								// absolute value of tan phi
			PxReal h = 1 / PxSqrt(t*t+1);										// absolute value of cos phi
			PX_ASSERT(h!=1);													// |w|<1000 guarantees this with typical IEEE754 machine eps (approx 6e-8)
			c = PxSqrt((1+h)/2);
			s = PxSqrt((1-h)/2) * PxSign(w);
		}

		PxQuat r(0,0,0,c);
		reinterpret_cast<PxVec4&>(r)[a] = s;

		q = (q*r).getNormalized();
	}

	return PxVec3(d.column0.x, d.column1.y, d.column2.z);
}



}
} // end namespace physx::apex
