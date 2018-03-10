/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

//#ifdef _MANAGED
//#pragma managed(push, off)
//#endif

#include <stdarg.h>
#include "NxApex.h"
#include "ApexSharedUtils.h"

#include "authoring/Fracturing.h"
#include "authoring/ApexGSA.h"
#include "PsUserAllocated.h"
#include "ApexRand.h"
#include "foundation/PxErrorCallback.h"
#include "PsString.h"
#include "PsShare.h"
#include "foundation/PxPlane.h"
#include "PsMathUtils.h"
#include "PsAlloca.h"
#include "ConvexDecomposition.h"
#include "Noise.h"
#include "NxDestructibleAsset.h"
#include "Link.h"
#include "NiApexRenderDebug.h"
#include "PsSort.h"

#define INCREMENTAL_GSA	0

/////////////////////////////////////////////////////////////////////////////

#if NX_SDK_VERSION_MAJOR == 2
#include "NxPlane.h"
#endif

#include <stdio.h>

#ifndef WITHOUT_APEX_AUTHORING
#include "ApexSharedSerialization.h"


using namespace physx;	// !?  Need to do this for PX_ALLOCA!?


#define MAX_ALLOWED_ESTIMATED_CHUNK_TOTAL	10000

#define CUTOUT_MAP_BOUNDS_TOLERANCE	0.0001f
#define MESH_INSTANACE_TOLERANCE	0.025f

static ApexCSG::BSPBuildParameters
gDefaultBuildParameters;

static bool gIslandGeneration = false;
static unsigned gMicrogridSize = 65536;
static NxBSPOpenMode::Enum gMeshMode = NxBSPOpenMode::Automatic;
static int	gVerbosity = 0;


static physx::NxCollisionVolumeDesc getVolumeDesc(const physx::NxCollisionDesc& collisionDesc, unsigned depth)
{
	return collisionDesc.mDepthCount > 0 ? collisionDesc.mVolumeDescs[physx::PxMin(depth, collisionDesc.mDepthCount-1)] : physx::NxCollisionVolumeDesc();
}

PX_INLINE physx::PxF32 extentDistance(physx::PxF32 min0, physx::PxF32 max0, physx::PxF32 min1, physx::PxF32 max1)
{
	return PxMax(min0 - max1, min1 - max0);
}


static physx::PxMat44 randomRotationMatrix(physx::PxVec3 zAxis, physx::QDSRand& rnd)
{
	physx::PxMat44 rot;
	zAxis.normalize();
	physx::PxU32 maxDir =  physx::PxAbs(zAxis.x) > physx::PxAbs(zAxis.y) ?
		(physx::PxAbs(zAxis.x) > physx::PxAbs(zAxis.z) ? 0u : 2u) :
		(physx::PxAbs(zAxis.y) > physx::PxAbs(zAxis.z) ? 1u : 2u);
	physx::PxVec3 xAxis = physx::PxMat33::createIdentity()[(maxDir + 1) % 3];
	physx::PxVec3 yAxis = zAxis.cross(xAxis);
	yAxis.normalize();
	xAxis = yAxis.cross(zAxis);

	const physx::PxF32 angle = rnd.getScaled(-physx::PxPi, physx::PxPi);
	const physx::PxF32 c = physx::PxCos(angle);
	const physx::PxF32 s = physx::PxSin(angle);

	rot.column0 = physx::PxVec4(c*xAxis + s*yAxis, 0.0f);
	rot.column1 = physx::PxVec4(c*yAxis - s*xAxis, 0.0f);
	rot.column2 = physx::PxVec4(zAxis, 0.0f);
	rot.column3 = physx::PxVec4(0.0f, 0.0f, 0.0f, 1.0f);

	return rot;
}

PX_INLINE physx::PxVec3 randomPositionInTriangle(const physx::PxVec3& v1, const physx::PxVec3& v2, const physx::PxVec3& v3, physx::QDSRand& rnd)
{
	const physx::PxVec3 d1 = v2 - v1;
	const physx::PxVec3 d2 = v3 - v1;
	physx::PxF32 c1 = rnd.getUnit();
	physx::PxF32 c2 = rnd.getUnit();
	const physx::PxF32 d = 1.0f - (c1+c2);
	if (d < 0.0f)
	{
		c1 += d;
		c2 += d;
	}
	return v1 + c1*d1 + c2*d2;
}


// Used by VoronoiCellPlaneIterator
class ReciprocalSitePairLink : public physx::Link
{
public:
	ReciprocalSitePairLink() : physx::Link(), m_recip(NULL)
	{
	}
	ReciprocalSitePairLink(const ReciprocalSitePairLink& other) : physx::Link()
	{
		index0 = other.index0;
		index1 = other.index1;
		plane = other.plane;
		m_recip = NULL;
	}
	~ReciprocalSitePairLink()
	{
		remove();
	}

	void					setRecip(ReciprocalSitePairLink& recip)
	{
		PX_ASSERT(m_recip == NULL && recip.m_recip == NULL);
		m_recip = &recip;
		recip.m_recip = this;
	}

	ReciprocalSitePairLink* getRecip() const
	{
		return m_recip;
	}

	ReciprocalSitePairLink*	getAdj(physx::PxU32 which) const
	{
		return static_cast<ReciprocalSitePairLink*>(physx::Link::getAdj(which));
	}

	void					remove()
	{
		if (m_recip)
		{
			m_recip->m_recip = NULL;
			m_recip = NULL;
		}
		physx::Link::remove();
	}

	physx::PxU32	index0, index1;
	physx::PxPlane	plane;

private:
	ReciprocalSitePairLink*	m_recip;
};


struct SiteMidPlaneIteratorInit
{
	SiteMidPlaneIteratorInit() : first(NULL), stop(NULL) {}

	ReciprocalSitePairLink*	first;
	ReciprocalSitePairLink* stop;
};

class SiteMidPlaneIterator
{
public:
	SiteMidPlaneIterator(const SiteMidPlaneIteratorInit& listBounds) : current(listBounds.first), stop(listBounds.stop) {}

	bool	valid() const
	{
		return current != stop;
	}

	void	inc()
	{
		current = current->getAdj(1);
	}

	ApexCSG::Plane	plane() const
	{
		const physx::PxPlane& midPlane = current->plane;
		ApexCSG::Plane plane(ApexCSG::Dir((ApexCSG::Real)midPlane.n.x, (ApexCSG::Real)midPlane.n.y, (ApexCSG::Real)midPlane.n.z), (ApexCSG::Real)midPlane.d);
		plane.normalize();
		return plane;
	}

private:
	ReciprocalSitePairLink* current;
	ReciprocalSitePairLink* stop;
};

class SiteMidPlaneIntersection : public ApexCSG::GSA::StaticConvexPolyhedron<SiteMidPlaneIterator, SiteMidPlaneIteratorInit>
{
public:
	void	setPlanes(ReciprocalSitePairLink* first, ReciprocalSitePairLink* stop)
	{
		m_initValues.first = first;
		m_initValues.stop = stop;
	}

	void	replacePlanes(ReciprocalSitePairLink* first, ReciprocalSitePairLink* stop, const physx::PxPlane& oldFlipPlane, const physx::PxPlane& newFlipPlane)
	{
		m_initValues.first = first;
		m_initValues.stop = stop;
#if INCREMENTAL_GSA
		const ApexCSG::Plane oldFlipGSAPlane = ApexCSG::Plane(ApexCSG::Dir((ApexCSG::Real)oldFlipPlane.n.x, (ApexCSG::Real)oldFlipPlane.n.y, (ApexCSG::Real)oldFlipPlane.n.z), (ApexCSG::Real)oldFlipPlane.d);
		const ApexCSG::Plane newFlipGSAPlane = ApexCSG::Plane(ApexCSG::Dir((ApexCSG::Real)newFlipPlane.n.x, (ApexCSG::Real)newFlipPlane.n.y, (ApexCSG::Real)newFlipPlane.n.z), (ApexCSG::Real)newFlipPlane.d);
		for (int i = 0; i < 4; ++i)
		{
			if (m_S(0,i) == oldFlipGSAPlane(0) && m_S(1,i) == oldFlipGSAPlane(1) && m_S(2,i) == oldFlipGSAPlane(2) && m_S(3,i) == oldFlipGSAPlane(3))
			{
				m_S.setCol(i, -oldFlipGSAPlane);
			}
			if (m_S(0,i) == newFlipGSAPlane(0) && m_S(1,i) == newFlipGSAPlane(1) && m_S(2,i) == newFlipGSAPlane(2) && m_S(3,i) == newFlipGSAPlane(3))
			{
				m_S.setCol(i, -newFlipGSAPlane);
			}
		}
#else
		(void)oldFlipPlane;
		(void)newFlipPlane;
#endif
	}

	void	resetPlanes()
	{
		m_initValues.first = NULL;
		m_initValues.stop = NULL;
	}
};

// Voronoi decomposition utility
class VoronoiCellPlaneIterator
{
public:
							VoronoiCellPlaneIterator(const physx::PxVec3* sites, physx::PxU32 siteCount, const physx::PxPlane* boundPlanes = NULL, physx::PxU32 boundPlaneCount = 0, physx::PxU32 startSiteIndex = 0);

	bool					valid() const
							{
								return m_valid;
							}

	physx::PxU32			cellIndex() const
							{
								return m_valid ? m_cellIndex : 0xFFFFFFFF;
							}

	const physx::PxPlane*	cellPlanes() const
							{
								return m_valid ? m_cellPlanes.begin() : NULL;
							}

	physx::PxU32			cellPlaneCount() const
							{
								return m_valid ? m_cellPlanes.size() : 0;
							}

	void					inc()
							{
								if (m_valid)
								{
									if (m_startPair != &m_listRoot)
									{
										prepareOutput();
									}
									else
									{
										m_valid = false;
									}
								}
							}

private:
	void					prepareOutput();

	// Input
	const physx::PxVec3*					m_sites;
	physx::PxU32							m_siteCount;
	physx::PxU32							m_boundPlaneCount;

	// State and intermediate data
	physx::Array<ReciprocalSitePairLink>	m_sitePairs;	// A symmetric array of site pairs and their bisector planes, in order (i major, j minor), with the diagonal removed
	SiteMidPlaneIntersection				m_test;			// Used to see if a plane is necessary for a cell
	ReciprocalSitePairLink*					m_startPair;	// Current start site pair
	ReciprocalSitePairLink					m_listRoot;		// A stopping node

	// Output
	bool									m_valid;
	physx::PxU32							m_cellIndex;
	physx::Array<physx::PxPlane>			m_cellPlanes;
};

VoronoiCellPlaneIterator::VoronoiCellPlaneIterator(const physx::PxVec3* sites, physx::PxU32 siteCount, const physx::PxPlane* boundPlanes, physx::PxU32 boundPlaneCount, physx::PxU32 startSiteIndex)
{
	m_valid = false;

	if (sites == NULL || startSiteIndex >= siteCount)
	{
		return;
	}

	m_valid = true;

	m_sites = sites;
	m_siteCount = siteCount;
	m_cellIndex = startSiteIndex;
	m_boundPlaneCount = boundPlanes != NULL ? boundPlaneCount : 0;

	// Add the bound planes
	m_cellPlanes.reserve(m_boundPlaneCount);
	for (physx::PxU32 boundPlaneNum = 0; boundPlaneNum < m_boundPlaneCount; ++boundPlaneNum)
	{
		m_cellPlanes.pushBack(boundPlanes[boundPlaneNum]);
	}

	// This should mean m_siteCount = 1.  In this case, there are no planes (besides the bound planes)
	if (m_siteCount < 2)
	{
		m_startPair = &m_listRoot;	// Causes termination after one iteration
		return;
	}

	// Fill in the pairs
	m_sitePairs.resize(m_siteCount*(m_siteCount-1));
	physx::PxU32 pairIndex = 0;
	for (physx::PxU32 i = 0; i < m_siteCount; ++i)
	{
		for (physx::PxU32 j = 0; j < m_siteCount; ++j)
		{
			if (j == i)
			{
				continue;
			}
			ReciprocalSitePairLink& pair = m_sitePairs[pairIndex];
			if (j > i)
			{
				pair.setRecip(m_sitePairs[pairIndex+(j-i)*(m_siteCount-2)+1]);
			}
			pair.index0 = i;
			pair.index1 = j;
			pair.plane = physx::PxPlane(0.5f*(m_sites[j] + m_sites[i]), (m_sites[j] - m_sites[i]).getNormalized());
			// Link together into a single loop
			if (pairIndex > 0)
			{
				m_sitePairs[pairIndex-1].setAdj(1, &pair);
			}

			++pairIndex;
		}
	}

	// Start with the first pair in the array
	m_startPair = &m_sitePairs[0];

	// Create a list root
	m_listRoot.setAdj(1, m_startPair);

	// Find first pair with the desired index0
	while (m_startPair->index0 != startSiteIndex && m_startPair != &m_listRoot)
	{
		m_startPair = m_startPair->getAdj(1);
	}

	prepareOutput();
}

void VoronoiCellPlaneIterator::prepareOutput()
{
	if (!m_valid)
	{
		return;
	}

	m_cellIndex = m_startPair->index0;

	// Find the first pair with a different first site index, which is our end-marker
	ReciprocalSitePairLink* stopPair = m_startPair->getAdj(1);
	while (stopPair != &m_listRoot && stopPair->index0 == m_cellIndex)
	{
		stopPair = stopPair->getAdj(1);
	}

	PX_ASSERT(stopPair == &m_listRoot || stopPair->index0 == m_startPair->index0+1);

	// Reset planes (keeping bound planes)
	m_cellPlanes.resize(m_boundPlaneCount);

	// Now iterate through this subset of the list, flipping one plane each time
	ReciprocalSitePairLink* testPlanePair = m_startPair;
	bool firstGSAUse = true;
	physx::PxPlane lastPlane;
	do
	{
		ReciprocalSitePairLink* nextPlanePair = testPlanePair->getAdj(1);
		testPlanePair->plane = physx::PxPlane(-testPlanePair->plane.n, -testPlanePair->plane.d);	// Flip
		if (firstGSAUse)
		{
			m_test.setPlanes(m_startPair, stopPair);
#if INCREMENTAL_GSA
			firstGSAUse = false;
#endif
		}
		else
		{
			m_test.replacePlanes(m_startPair, stopPair, lastPlane, testPlanePair->plane);
		}
		lastPlane = testPlanePair->plane;
		const bool keep = (1 == ApexCSG::GSA::vs3d_test(m_test));
		testPlanePair->plane = physx::PxPlane(-testPlanePair->plane.n, -testPlanePair->plane.d);	// Flip back
		if (keep)
		{
			// This is a bounding plane
			m_cellPlanes.pushBack(testPlanePair->plane);
		}
		else
		{
			// Flipping this plane results in an empty set intersection.  It is non-essential, so remove it
			// And its reciprocal
			if (testPlanePair->getRecip() == stopPair)
			{
				stopPair = stopPair->getAdj(1);
			}
			testPlanePair->getRecip()->remove();
			if (testPlanePair == m_startPair)
			{
				m_startPair = m_startPair->getAdj(1);
			}
			testPlanePair->remove();
		}
		testPlanePair = nextPlanePair;
	} while (testPlanePair != stopPair);

	m_startPair = stopPair;
}


PX_INLINE bool segmentOnBorder(const physx::PxVec3& v0, const physx::PxVec3& v1, physx::PxF32 width, physx::PxF32 height)
{
	return
	    (v0.x < -0.5f && v1.x < -0.5f) ||
	    (v0.y < -0.5f && v1.y < -0.5f) ||
	    (v0.x >= width - 0.5f && v1.x >= width - 0.5f) ||
	    (v0.y >= height - 0.5f && v1.y >= height - 0.5f);
}

class Random : public ApexCSG::UserRandom
{
public:
	physx::PxU32	getInt()
	{
		return m_rnd.nextSeed();
	}
	physx::PxF32	getReal(physx::PxF32 min, physx::PxF32 max)
	{
		return m_rnd.getScaled(min, max);
	}

	physx::QDSRand	m_rnd;
} userRnd;


typedef ApexCSG::Vec<PxF32,2> Vec2Float;
typedef ApexCSG::Vec<PxF32,3> Vec3Float;

// TODO: Provide configurable octave parameter
ApexCSG::PerlinNoise<physx::PxF32, 64, 2, Vec2Float > userPerlin2D(userRnd, 2, 1.5f, 2.5f);
ApexCSG::PerlinNoise<physx::PxF32, 64, 3, Vec3Float > userPerlin3D(userRnd, 2, 1.5f, 2.5f);

PX_INLINE bool edgesOverlap(const physx::PxVec3& pv00, const physx::PxVec3& pv01, const physx::PxVec3& pv10, const physx::PxVec3& pv11, physx::PxF32 eps)
{
	physx::PxVec3 e0 = pv01 - pv00;
	physx::PxVec3 e1 = pv11 - pv10;

	if (e0.dot(e1) < 0)
	{
		return false;
	}

	physx::PxF32 l0 = e0.normalize();
	e1.normalize();

	const physx::PxVec3 disp0 = pv10 - pv00;
	const physx::PxVec3 disp1 = pv11 - pv00;

	const physx::PxF32 d10 = disp0.dot(e0);

	const physx::PxF32 d11 = disp1.dot(e0);

	if (d11 < -eps)
	{
		return false;
	}

	if (d10 > l0 + eps)
	{
		return false;
	}

	const physx::PxF32 disp02 = disp0.dot(disp0);
	if (disp02 - d10 * d10 > eps * eps)
	{
		return false;
	}

	const physx::PxF32 disp12 = disp1.dot(disp1);
	if (disp12 - d11 * d11 > eps * eps)
	{
		return false;
	}

	return true;
}

PX_INLINE bool trianglesOverlap(const physx::PxVec3& pv00, const physx::PxVec3& pv01, const physx::PxVec3& pv02, const physx::PxVec3& pv10, const physx::PxVec3& pv11, const physx::PxVec3& pv12, physx::PxF32 eps)
{
	return	edgesOverlap(pv00, pv02, pv10, pv11, eps) || edgesOverlap(pv00, pv02, pv11, pv12, eps) || edgesOverlap(pv00, pv02, pv12, pv10, eps) ||
	        edgesOverlap(pv01, pv00, pv10, pv11, eps) || edgesOverlap(pv01, pv00, pv11, pv12, eps) || edgesOverlap(pv01, pv00, pv12, pv10, eps) ||
	        edgesOverlap(pv02, pv01, pv10, pv11, eps) || edgesOverlap(pv02, pv01, pv11, pv12, eps) || edgesOverlap(pv02, pv01, pv12, pv10, eps);
}


// Returns a point uniformly distributed on the "polar cap" in +axisN direction, of azimuthal size range (in radians)
PX_INLINE physx::PxVec3	randomNormal(physx::PxU32 axisN, physx::PxF32 range)
{
	physx::PxVec3 result;
	const physx::PxF32 cosTheta = 1.0f - (1.0f - physx::PxCos(range)) * userRnd.getReal(0.0f, 1.0f);
	const physx::PxF32 sinTheta = physx::PxSqrt(1.0f - cosTheta * cosTheta);
	physx::PxF32 cosPhi, sinPhi;
	physx::sincos(userRnd.getReal(-physx::PxPi, physx::PxPi), sinPhi, cosPhi);
	result[axisN % 3] = cosTheta;
	result[(axisN + 1) % 3] = cosPhi * sinTheta;
	result[(axisN + 2) % 3] = sinPhi * sinTheta;
	return result;
}

void calculatePartition(int partition[3], const unsigned requestedSplits[3], const physx::PxVec3& extent, const float* targetProportions)
{
	partition[0] = (physx::PxI32)requestedSplits[0] + 1;
	partition[1] = (physx::PxI32)requestedSplits[1] + 1;
	partition[2] = (physx::PxI32)requestedSplits[2] + 1;

	if (targetProportions != NULL)
	{
		physx::PxVec3 n(extent[0] / targetProportions[0], extent[1] / targetProportions[1], extent[2] / targetProportions[2]);
		n *= physx::PxVec3((physx::PxF32)partition[0], (physx::PxF32)partition[1], (physx::PxF32)partition[2]).dot(n) / n.magnitudeSquared();
		// n now contains the # of partitions per axis closest to the desired # of partitions
		// which give the correct target proportions. However, the numbers will not (in general)
		// be integers, so round:
		partition[0] = physx::PxMax(1, (int)(n[0] + 0.5f));
		partition[1] = physx::PxMax(1, (int)(n[1] + 0.5f));
		partition[2] = physx::PxMax(1, (int)(n[2] + 0.5f));
	}
}

static void outputMessage(const char* message, physx::PxErrorCode::Enum errorCode = physx::PxErrorCode::eNO_ERROR, int verbosity = 0)	// Lower # = higher priority
{
	if (verbosity > gVerbosity)
	{
		return;
	}

	physx::PxErrorCallback* outputStream = physx::NxGetApexSDK()->getErrorCallback();
	if (outputStream)
	{
		outputStream->reportError(errorCode, message, __FILE__, __LINE__);
	}
}

struct ChunkIndexer
{
	physx::ExplicitHierarchicalMesh::Chunk*	chunk;
	physx::PxI32							parentIndex;
	physx::PxI32							index;

	static int compareParentIndices(const void* A, const void* B)
	{
		const int diff = ((const ChunkIndexer*)A)->parentIndex - ((const ChunkIndexer*)B)->parentIndex;
		if (diff)
		{
			return diff;
		}
		return ((const ChunkIndexer*)A)->index - ((const ChunkIndexer*)B)->index;
	}
};

static physx::PxBounds3 boundTriangles(const physx::Array<physx::NxExplicitRenderTriangle>& triangles, const physx::PxMat34Legacy& interiorTM)
{
	physx::PxBounds3 bounds;
	bounds.setEmpty();
	for (physx::PxU32 triangleN = 0; triangleN < triangles.size(); ++triangleN)
	{
		for (int v = 0; v < 3; ++v)
		{
			physx::PxVec3 localVert;
			interiorTM.multiplyByInverseRT(triangles[triangleN].vertices[v].position, localVert);
			bounds.include(localVert);
		}
	}
	return bounds;
}

PX_INLINE void generateSliceAxes(physx::PxU32 sliceAxes[3], physx::PxU32 sliceAxisNum)
{
	switch (sliceAxisNum)
	{
	case 0:
		sliceAxes[1] = 2;
		sliceAxes[0] = 1;
		break;
	case 1:
		sliceAxes[1] = 2;
		sliceAxes[0] = 0;
		break;
	default:
	case 2:
		sliceAxes[1] = 1;
		sliceAxes[0] = 0;
	}
	sliceAxes[2] = sliceAxisNum;
}

PX_INLINE physx::PxVec3 createAxis(physx::PxU32 axisNum)
{
	return physx::PxMat33::createIdentity()[axisNum];
}

PX_INLINE void 	getCutoutSliceAxisAndSign(physx::PxU32& sliceAxisNum, physx::PxU32& sliceSignNum, physx::PxU32 sliceDirIndex)
{
	sliceAxisNum  = sliceDirIndex >> 1;
	sliceSignNum  = sliceDirIndex & 1;
}

typedef physx::PxF32(*NoiseFn)(physx::PxF32 x, physx::PxF32 y, physx::PxF32 z, physx::PxF32& xGrad, physx::PxF32& yGrad, physx::PxF32& zGrad);

static physx::PxF32 planeWave(physx::PxF32 x, physx::PxF32 y, physx::PxF32, physx::PxF32& xGrad, physx::PxF32& yGrad, physx::PxF32& zGrad)
{
	physx::PxF32 c, s;
	physx::sincos(x + y, s, c);
	xGrad = c;
	yGrad = 0.0f;
	zGrad = 0.0f;
	return s;
}

static physx::PxF32 perlin2D(physx::PxF32 x, physx::PxF32 y, physx::PxF32, physx::PxF32& xGrad, physx::PxF32& yGrad, physx::PxF32& zGrad)
{
	const PxF32 xy[] = {x, y};
	physx::PxF32 s = userPerlin2D.sample(Vec2Float(xy));
	// TODO: Implement
	xGrad = 0.0f;
	yGrad = 0.0f;
	zGrad = 0.0f;
	return s;
}

static physx::PxF32 perlin3D(physx::PxF32 x, physx::PxF32 y, physx::PxF32 z, physx::PxF32& xGrad, physx::PxF32& yGrad, physx::PxF32& zGrad)
{
	const PxF32 xyz[] = {x, y, z};
	physx::PxF32 s = userPerlin3D.sample(Vec3Float(xyz));
	// TODO: Implement
	xGrad = 0.0f;
	yGrad = 0.0f;
	zGrad = 0.0f;
	return s;
}

static NoiseFn noiseFns[] =
{
	planeWave,
	perlin2D,
	perlin3D
};

static int noiseFnCount = PX_ARRAY_SIZE(noiseFns);

static void buildNoise(physx::Array<physx::PxF32>& f, physx::Array<physx::PxVec3>* n,
					   physx::IntersectMesh::GridPattern pattern, physx::PxF32 cornerX, physx::PxF32 cornerY, physx::PxF32 xSpacing, physx::PxF32 ySpacing, physx::PxU32 numX, physx::PxU32 numY,
					   float noiseAmplitude, float relativeFrequency, float xPeriod, float yPeriod,
					   int noiseType, int noiseDir)
{
	const physx::PxU32 gridSize = (numX + 1) * (numY + 1);
	
	if( f.size() != gridSize) 
		f.resize(gridSize, 0.);
	
	if( n && n->size() != gridSize) 
		n->resize(gridSize, physx::PxVec3(0,0,0));

	noiseType = physx::PxClamp(noiseType, 0 , noiseFnCount - 1);
	NoiseFn noiseFn = noiseFns[noiseType];

	// This differentiation between wave planes and perlin is rather arbitrary, but works alright
	const physx::PxU32 numModes = noiseType == NxFractureSliceDesc::NoiseWavePlane ? 20u : 4u;
	const physx::PxF32 amplitude = noiseAmplitude / physx::PxSqrt((physx::PxF32)numModes);	// Scale by frequency?
	for (physx::PxU32 i = 0; i < numModes; ++i)
	{
		physx::PxF32 phase     = userRnd.getReal(-3.14159265f, 3.14159265f);
		physx::PxF32 freqShift = userRnd.getReal(0.0f, 3.0f);
		physx::PxF32 kx, ky;
		switch (noiseDir)
		{
		case 0:
			kx = physx::PxPow(2.0f, freqShift) * relativeFrequency / xSpacing;
			ky = 0.0f;
			break;
		case 1:
			kx = 0.0f;
			ky = physx::PxPow(2.0f, freqShift) * relativeFrequency / ySpacing;
			break;
		default:
			{
				const physx::PxF32 f = physx::PxPow(2.0f, freqShift) * relativeFrequency;
				const physx::PxF32 theta = userRnd.getReal(-3.14159265f, 3.14159265f);
				const physx::PxF32 c = physx::PxCos(theta);
				const physx::PxF32 s = physx::PxSin(theta);
				kx = c * f / xSpacing;
				ky = s * f / ySpacing;
			}
		}

		if (xPeriod != 0.0f)
		{
			const float cx = (2.0f * 3.14159265f) / xPeriod;
			const int nx = (int)physx::PxSign(kx) * (int)(physx::PxAbs(kx) / cx + 0.5f);	// round
			kx = nx * cx;
		}

		if (yPeriod != 0.0f)
		{
			// Make sure the wavenumbers are integers
			const float cy = (2.0f * 3.14159265f) / yPeriod;
			const int ny = (int)physx::PxSign(ky) * (int)(physx::PxAbs(ky) / cy + 0.5f);	// round
			ky = ny * cy;
		}

		physx::PxU32 pointN = 0;
		physx::PxF32 y = cornerY;
		for (physx::PxU32 iy = 0; iy <= numY; ++iy, y += ySpacing)
		{
			physx::PxF32 x = cornerX;
			for (physx::PxU32 ix = 0; ix <= numX; ++ix, x += xSpacing, ++pointN)
			{
				if (pattern == physx::IntersectMesh::Equilateral && (((iy & 1) == 0 && ix == numX) || ((iy & 1) != 0 && ix == 1)))
				{
					x -= 0.5f * xSpacing;
				}
				physx::PxF32 xGrad, yGrad, zGrad;
				// TODO: Find point in 3D space for use with NoisePerlin3D
				f[pointN] += amplitude * noiseFn(x * kx - phase, y * ky - phase, 0, xGrad, yGrad, zGrad);
				if (n) (*n)[pointN] += physx::PxVec3(-xGrad * kx * amplitude, -yGrad * ky * amplitude, 0.0f);
			}
		}
	}

}

// noiseDir = 0 => X
// noiseDir = 1 => Y
// noiseDir = -1 => userRnd
void physx::IntersectMesh::build(GridPattern pattern, const physx::PxPlane& plane,
                                 physx::PxF32 cornerX, physx::PxF32 cornerY, physx::PxF32 xSpacing, physx::PxF32 ySpacing, physx::PxU32 numX, physx::PxU32 numY,
                                 const physx::PxMat34Legacy& tm, float noiseAmplitude, float relativeFrequency, float xPeriod, float yPeriod,
                                 int noiseType, int noiseDir, physx::PxU32 submeshIndex, physx::PxU32 frameIndex, const physx::TriangleFrame& triangleFrame, bool forceGrid)
{
	m_pattern = pattern;
	m_plane = plane;
	m_cornerX = cornerX;
	m_cornerY = cornerY;
	m_xSpacing = xSpacing;
	m_ySpacing = ySpacing;
	m_numX = numX;
	m_numY = numY;
	m_tm = tm;

	if (relativeFrequency == 0.0f)
	{
		// 0 frequency only provides a plane offset
		m_plane.d += userRnd.getReal(-noiseAmplitude, noiseAmplitude);
		noiseAmplitude = 0.0f;
	}

	if (!forceGrid && noiseAmplitude == 0.0f)
	{
		// Without noise, we only need one triangle
		m_pattern = Equilateral;
		m_vertices.resize(3);
		m_triangles.resize(1);

		const physx::PxF32 rX = 0.5f * (xSpacing * numX);
		const physx::PxF32 rY = 0.5f * (ySpacing * numY);
		const physx::PxF32 centerX = cornerX + rX;
		const physx::PxF32 centerY = cornerY + rY;

		// Circumscribe rectangle
		const physx::PxF32 R = physx::PxSqrt(rX * rX + rY * rY);

		// Fit equilateral triangle around circle
		const physx::PxF32 x = 1.73205081f * R;
		tm.multiply(physx::PxVec3(centerX, centerY + 2 * R, 0), m_vertices[0].position);
		tm.multiply(physx::PxVec3(centerX - x, centerY - R, 0), m_vertices[1].position);
		tm.multiply(physx::PxVec3(centerX + x, centerY - R, 0), m_vertices[2].position);

		for (physx::PxU32 i = 0; i < 3; ++i)
		{
			m_vertices[i].normal = m_plane.n;
			m_vertices[i].tangent = tm.M.getColumn(0);
			m_vertices[i].binormal = tm.M.getColumn(1);
			m_vertices[i].color = PxColorRGBA(255, 255, 255, 255);
		}

		NxExplicitRenderTriangle& triangle = m_triangles[0];
		for (physx::PxU32 v = 0; v < 3; ++v)
		{
			NxVertex& gridVertex = m_vertices[v];
			triangle.vertices[v] = gridVertex;
			triangleFrame.interpolateVertexData(triangle.vertices[v]);
			// Only really needed to interpolate u,v... replace normals and tangents with proper ones
			triangle.vertices[v].normal = gridVertex.normal;
			triangle.vertices[v].tangent = gridVertex.tangent;
			triangle.vertices[v].binormal = gridVertex.binormal;
		}
		triangle.extraDataIndex = frameIndex;
		triangle.smoothingMask = 0;
		triangle.submeshIndex = (physx::PxI32)submeshIndex;

		return;
	}

	///////////////////////////////////////////////////////////////////////////

	physx::PxVec3 corner;
	m_tm.multiply(physx::PxVec3(m_cornerX, m_cornerY, 0), corner);
	const physx::PxVec3 localX = m_tm.M.getColumn(0) * m_xSpacing;
	const physx::PxVec3 localY = m_tm.M.getColumn(1) * m_ySpacing;
	const physx::PxVec3 localZ = m_tm.M.getColumn(2);

	// Vertices:
	m_vertices.resize((m_numX + 1) * (m_numY + 1));
	const physx::PxVec3 halfLocalX = 0.5f * localX;
	physx::PxU32 pointN = 0;
	physx::PxVec3 side = corner;
	for (physx::PxU32 iy = 0; iy <= m_numY; ++iy, side += localY)
	{
		physx::PxVec3 point = side;
		for (physx::PxU32 ix = 0; ix <= m_numX; ++ix, point += localX)
		{
			if (m_pattern == physx::IntersectMesh::Equilateral && (((iy & 1) == 0 && ix == m_numX) || ((iy & 1) != 0 && ix == 1)))
			{
				point -= halfLocalX;
			}
			NxVertex& vertex = m_vertices[pointN++];
			vertex.position = point;
			vertex.normal = physx::PxVec3(0.0f);
		}
	}

	// Build noise
	physx::Array<physx::PxF32>  f(m_vertices.size(), 0.);
	physx::Array<physx::PxVec3> n(m_vertices.size(), PxVec3(0,0,0));
	buildNoise(f, &n, pattern, m_cornerX, m_cornerY, m_xSpacing, m_ySpacing, m_numX, m_numY, 
	           noiseAmplitude, relativeFrequency, xPeriod, yPeriod, noiseType, noiseDir);
	pointN = 0;
	for (physx::PxU32 iy = 0; iy <= m_numY; ++iy)
	{
		for (physx::PxU32 ix = 0; ix <= m_numX; ++ix, ++pointN)
		{
			NxVertex& vertex = m_vertices[pointN];
			vertex.position += localZ * f[pointN];
			vertex.normal   += n[pointN];
		}
	}

	// Normalize normals and put in correct frame
	for (pointN = 0; pointN < m_vertices.size(); pointN++)
	{
		NxVertex& vertex = m_vertices[pointN];
		vertex.normal.z = 1.0f;
		vertex.normal.normalize();
		m_tm.M.multiply(vertex.normal, vertex.normal);
		vertex.tangent = m_tm.M.getColumn(1).cross(vertex.normal);
		vertex.tangent.normalize();
		vertex.color = PxColorRGBA(255, 255, 255, 255);
		vertex.binormal = vertex.normal.cross(vertex.tangent);
	}

	m_triangles.resize(2 * m_numX * m_numY);
	physx::PxU32 triangleN = 0;
	physx::PxU32 index = 0;
	const physx::PxU32 tpattern[12] = { 0, m_numX + 2, m_numX + 1, 0, 1, m_numX + 2, 0, 1, m_numX + 1, 1, m_numX + 2, m_numX + 1 };
	for (physx::PxU32 iy = 0; iy < m_numY; ++iy)
	{
		const physx::PxU32* yPattern = tpattern + (iy & 1) * 6;
		for (physx::PxU32 ix = 0; ix < m_numX; ++ix, ++index)
		{
			const physx::PxU32* ytPattern = yPattern;
			for (physx::PxU32 it = 0; it < 2; ++it, ytPattern += 3)
			{
				NxExplicitRenderTriangle& triangle = m_triangles[triangleN++];
				for (physx::PxU32 v = 0; v < 3; ++v)
				{
					NxVertex& gridVertex = m_vertices[index + ytPattern[v]];
					triangle.vertices[v] = gridVertex;
					triangleFrame.interpolateVertexData(triangle.vertices[v]);
					// Only really needed to interpolate u,v... replace normals and tangents with proper ones
					triangle.vertices[v].normal = gridVertex.normal;
					triangle.vertices[v].tangent = gridVertex.tangent;
					triangle.vertices[v].binormal = gridVertex.binormal;
				}
				triangle.extraDataIndex = frameIndex;
				triangle.smoothingMask = 0;
				triangle.submeshIndex = (physx::PxI32)submeshIndex;
			}
		}
		++index;
	}
}

static const int gSliceDirs[6][3] =
{
	{0, 1, 2},	// XYZ
	{1, 2, 0},	// YZX
	{2, 0, 1},	// ZXY
	{2, 1, 0},	// ZYX
	{1, 0, 2},	// YXZ
	{0, 2, 1}	// XZY
};

struct GridParameters
{
	GridParameters() :
		sizeScale(1.0f),
		xPeriod(0.0f),
		yPeriod(0.0f),
		interiorSubmeshIndex(0xFFFFFFFF),
		materialFrameIndex(0xFFFFFFFF),
		forceGrid(false)
	{
	}

	physx::Array< physx::NxExplicitRenderTriangle >*	level0Mesh;
	physx::PxF32										sizeScale;
	physx::NxNoiseParameters								noise;
	physx::PxF32										xPeriod;
	physx::PxF32										yPeriod;
	physx::PxU32										interiorSubmeshIndex;
	physx::PxU32										materialFrameIndex;
	physx::TriangleFrame								triangleFrame;
	bool												forceGrid;
};


//////////////////////////////////////////////////////////////////////////////

static PX_INLINE physx::PxU32 nearestPowerOf2(physx::PxU32 v)
{
	v = v > 0 ? v - 1 : 0;

	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	++v;

	return v;
}

physx::DisplacementMapVolume::DisplacementMapVolume() 
: width(0),
  height(0),
  depth(0)
{

}

void physx::DisplacementMapVolume::init(const NxFractureSliceDesc& desc)
{
	PX_UNUSED(desc);

	// Compute the number of slices for each plane
	physx::PxU32 slices[3]; 
	slices[0] = slices[1] = slices[2] = 0;
	physx::PxU32 maxGridSize = 0;
	for (physx::PxU32 i = 0; i < 3; ++i)
	{
		for (physx::PxU32 j = 1; j < desc.maxDepth; ++j)
		{
			if (slices[i] == 0) 
				slices[i]  = desc.sliceParameters[j].splitsPerPass[i];
			else
				slices[i] *= desc.sliceParameters[j].splitsPerPass[i];
		}
		for (physx::PxU32 j = 0; j < desc.maxDepth; ++j)
		{
			if (desc.sliceParameters[j].noise[i].gridSize > (int)maxGridSize)
				maxGridSize = (physx::PxU32)desc.sliceParameters[j].noise[i].gridSize;
		}
	}

	width  = 4 * nearestPowerOf2(physx::PxMax(maxGridSize, physx::PxMax(slices[0], slices[1])));
	height = width;
	depth  = 4 * nearestPowerOf2(physx::PxMax(maxGridSize, slices[2]));
}


void physx::DisplacementMapVolume::getData(physx::PxU32& w, physx::PxU32& h, physx::PxU32& d, physx::PxU32& size, unsigned char const** ppData) const
{
	PX_ASSERT(ppData);
	if(data.size() == 0) 
		buildData();

	w       = width;
	h       = height;
	d       = depth;
	size    = data.size();
	*ppData = data.begin();
}

template<typename T, typename U>
class Conversion
{
public:
	static PX_INLINE U convert(T x)
	{
		return (U)PxClamp(x, (T)-1, (T)1);
	}
};

template<typename T>
class Conversion<T, unsigned char>
{
public:
	static PX_INLINE unsigned char convert(T x)
	{
		unsigned char value = (unsigned char)((PxClamp(x, (T)-1, (T)1) + 1) * .5 * 255);
		return value;
	}
};

void physx::DisplacementMapVolume::buildData(physx::PxVec3 scale) const
{
	// For now, we forgo use of the scaling parameter
	PX_UNUSED(scale);

	const PxU32 numChannels = 4;  // ZYX -> BGRA
	const PxU32 channelSize = sizeof(unsigned char);
	const PxU32 stride      = numChannels * channelSize;
	const PxU32 size        = width * depth * height * stride;
	data.resize(size);

	const PxF32 dX = width  > 1 ? 1.0f/(width - 1) : 0.0f;
	const PxF32 dY = height > 1 ? 1.0f/(height- 1) : 0.0f;
	const PxF32 dZ = depth  > 1 ? 1.0f/(depth - 1) : 0.0f;

	PxU32 index  = 0;
	PxF32 z      = 0.0f;
	for (PxU32 i = 0; i < depth; ++i, z+=dZ)
	{
		PxF32 y = 0.0f;
		for (PxU32 j = 0; j < height; ++j, y+=dY)
		{
			PxF32 x = 0.0f;
			for (PxU32 k = 0; k < width; ++k, x+=dX, index+=stride)
			{
				const PxF32 xyz[] = {x, y ,z};
				const PxF32 yzx[] = {y, z, x};

				// Random offsets in x and y, with the z offset as a combination of the two
				//   As long as we're consistent here in our ordering, noise will be a smooth vector function of position
				PxF32 xOffset = userPerlin3D.sample(Vec3Float(xyz));
				PxF32 yOffset = userPerlin3D.sample(Vec3Float(yzx));
				PxF32 zOffset = (xOffset + yOffset) * 0.5f;

				// ZXY -> RGB
				data[index]   = Conversion<PxF32, unsigned char>::convert(zOffset);
				data[index+1] = Conversion<PxF32, unsigned char>::convert(yOffset);
				data[index+2] = Conversion<PxF32, unsigned char>::convert(xOffset);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

static void buildIntersectMesh(physx::IntersectMesh& mesh,
							   const physx::PxPlane& plane,
							   const physx::NxMaterialFrame& materialFrame,
							   int noiseType = NxFractureSliceDesc::NoiseWavePlane,
							   const GridParameters* gridParameters = NULL)
{
	if (!gridParameters)
	{
		mesh.build(plane);
		return;
	}

	physx::PxMat34Legacy tm = materialFrame.mCoordinateSystem;

	physx::PxBounds3 localPlaneBounds = boundTriangles(*gridParameters->level0Mesh, tm);

	const physx::PxVec3 diameter = localPlaneBounds.maximum - localPlaneBounds.minimum;
	const physx::PxF32 planeDiameter = physx::PxMax(diameter.x, diameter.y);
	// No longer fattening - the BSP does not have side boundaries, so we will not shave off any of the mesh.
	//	localPlaneBounds.fatten( 0.005f*planeDiameter );	// To ensure we get the whole mesh
	const physx::PxF32 gridSpacing = planeDiameter / gridParameters->noise.gridSize;

	physx::PxVec3 center = localPlaneBounds.getCenter();
	physx::PxVec3 extent = localPlaneBounds.getExtents();

#if 0	// Equilateral
	const physx::PxF32 offset = 0.5f;
	const physx::PxF32 yRatio = 0.866025404f;
	const physx::IntersectMesh::GridPattern pattern = physx::IntersectMesh::Equilateral;
	const physx::PxF32 xSpacing = gridSpacing;
	const physx::PxF32 numX = PxCeil(2 * extent.x / xSpacing + offset);
	const physx::PxF32 cornerX = center.x - 0.5f * (numX - offset) * xSpacing;
	const physx::PxF32 ySpacing = yRatio * gridSpacing;
	const physx::PxF32 numY = PxCeil(2 * extent.y / ySpacing);
	const physx::PxF32 cornerY = center.y - 0.5f * numY * ySpacing;
#else	// Right
	const physx::IntersectMesh::GridPattern pattern = physx::IntersectMesh::Right;
	const physx::PxF32 numX = gridParameters->xPeriod != 0.0f ? gridParameters->noise.gridSize : physx::PxCeil(2 * extent.x / gridSpacing);
	const physx::PxF32 xSpacing = 2 * extent.x / numX;
	const physx::PxF32 cornerX = center.x - extent.x;
	const physx::PxF32 numY = gridParameters->yPeriod != 0.0f ? gridParameters->noise.gridSize : physx::PxCeil(2 * extent.y / gridSpacing);
	const physx::PxF32 ySpacing = 2 * extent.y / numY;
	const physx::PxF32 cornerY = center.y - extent.y;
#endif

	const physx::PxF32 noiseAmplitude = gridParameters->sizeScale * gridParameters->noise.amplitude;

	mesh.build(pattern, plane, cornerX, cornerY, xSpacing, ySpacing, (physx::PxU32)numX, (physx::PxU32)numY, tm,
	           noiseAmplitude, gridParameters->noise.frequency, gridParameters->xPeriod, gridParameters->yPeriod, noiseType, -1,
	           gridParameters->interiorSubmeshIndex, gridParameters->materialFrameIndex, gridParameters->triangleFrame, gridParameters->forceGrid);
}

PX_INLINE physx::PxPlane createSlicePlane(const physx::PxVec3& center, const physx::PxVec3& extent, int sliceDir, int sliceDirNum,
                                        const physx::PxF32 sliceWidths[3], const physx::PxF32 linearNoise[3], const physx::PxF32 angularNoise[3])
{
	// Orient the plane (+apply the angular noise) and compute the d parameter (+apply the linear noise)
	physx::PxVec3 slicePoint = center;
	slicePoint[sliceDir] += (sliceDirNum + 1) * sliceWidths[sliceDir] - extent[sliceDir];
	const physx::PxVec3 normal = randomNormal((physx::PxU32)sliceDir, angularNoise[sliceDir]);
	return physx::PxPlane(normal, -normal.dot(slicePoint) + sliceWidths[sliceDir] * linearNoise[sliceDir] * userRnd.getReal(-0.5f, 0.5f));
}

static void buildSliceBSP(ApexCSG::IApexBSP& sliceBSP, physx::ExplicitHierarchicalMesh& hMesh, const physx::NxNoiseParameters& noise,
                          const physx::PxVec3& extent, int sliceDir, int sliceDepth, const physx::PxPlane planes[3], 
						  const physx::NxFractureMaterialDesc& materialDesc, int noiseType = NxFractureSliceDesc::NoiseWavePlane, bool useDisplacementMaps = false)
{
	// Build grid and slice BSP
	physx::IntersectMesh grid;
	GridParameters gridParameters;
	gridParameters.interiorSubmeshIndex = materialDesc.interiorSubmeshIndex;
	// Defer noise generation if we're using displacement maps
	gridParameters.noise = useDisplacementMaps ? physx::NxNoiseParameters() : noise;
	gridParameters.level0Mesh = &hMesh.mParts[0]->mMesh;
	gridParameters.sizeScale = extent[sliceDir];
	gridParameters.materialFrameIndex = hMesh.addMaterialFrame();
	physx::NxMaterialFrame materialFrame = hMesh.getMaterialFrame(gridParameters.materialFrameIndex);
	materialFrame.buildCoordinateSystemFromMaterialDesc(materialDesc, planes[sliceDir]);
	materialFrame.mFractureMethod = physx::NxFractureMethod::Slice;
	materialFrame.mFractureIndex = sliceDir;
	materialFrame.mSliceDepth = (physx::PxU32)sliceDepth;
	hMesh.setMaterialFrame(gridParameters.materialFrameIndex, materialFrame);
	gridParameters.triangleFrame.setFlat(materialFrame.mCoordinateSystem, materialDesc.uvScale, materialDesc.uvOffset);
	buildIntersectMesh(grid, planes[sliceDir], materialFrame, noiseType, &gridParameters);
	
	ApexCSG::BSPTolerances bspTolerances = ApexCSG::gDefaultTolerances;
	bspTolerances.linear = 1.e-9f;
	bspTolerances.angular = 0.00001f;
	sliceBSP.setTolerances(bspTolerances);

	ApexCSG::BSPBuildParameters bspBuildParams = gDefaultBuildParameters;
	bspBuildParams.rnd = &userRnd;
	bspBuildParams.internalTransform = sliceBSP.getInternalTransform();

	if(useDisplacementMaps)
	{
		// Displacement map generation is deferred until the end of fracturing
		// This used to be where a slice would populate a displacement map with 
		//  offsets along the plane, but no longer
	}

	sliceBSP.fromMesh(&grid.m_triangles[0], grid.m_triangles.size(), bspBuildParams);
}

PX_INLINE ApexCSG::IApexBSP* createFractureBSP(physx::PxPlane slicePlanes[3], ApexCSG::IApexBSP*& sliceBSP, ApexCSG::IApexBSP& sourceBSP,
        physx::ExplicitHierarchicalMesh& hMesh, physx::PxF32& childVolume, physx::PxF32 minVolume,
        const physx::PxVec3& center, const physx::PxVec3& extent, int sliceDir, int sliceDirNum, int sliceDepth,
        const physx::PxF32 sliceWidths[3], const physx::PxF32 linearNoise[3], const physx::PxF32 angularNoise[3],
		const physx::NxNoiseParameters& noise, const physx::NxFractureMaterialDesc& materialDesc, int noiseType, bool useDisplacementMaps)
{
	const physx::PxPlane oldSlicePlane = slicePlanes[sliceDir];
	slicePlanes[sliceDir] = createSlicePlane(center, extent, sliceDir, sliceDirNum, sliceWidths, linearNoise, angularNoise);
	if (sliceBSP == NULL)
	{
		sliceBSP = createBSP(hMesh.mBSPMemCache, sourceBSP.getInternalTransform());
		buildSliceBSP(*sliceBSP, hMesh, noise, extent, sliceDir, sliceDepth, slicePlanes, materialDesc, noiseType, useDisplacementMaps);
	}
	sourceBSP.combine(*sliceBSP);
	ApexCSG::IApexBSP* bsp = createBSP(hMesh.mBSPMemCache, sourceBSP.getInternalTransform());
	bsp->op(sourceBSP, ApexCSG::Operation::Intersection);
#if 1	// Eliminating volume calculation here, for performance.  May introduce it later once the mesh is calculated.
	sourceBSP.op(sourceBSP, ApexCSG::Operation::A_Minus_B);
	if (minVolume <= 0 || (bsp->getType() != ApexCSG::BSPType::Empty_Set && sourceBSP.getType() != ApexCSG::BSPType::Empty_Set))
	{
		childVolume = 1.0f;
	}
	else
	{
		// We will ignore this slice
		if (sourceBSP.getType() != ApexCSG::BSPType::Empty_Set)
		{
			// chunk bsp volume too small
			slicePlanes[sliceDir] = oldSlicePlane;
			bsp->release();
			bsp = NULL;
			childVolume = 0.0f;
		}
		else
		{
			// remainder is too small.  Terminate slicing along this direction
			childVolume = 1.0f;
		}
	}
#else
	physx::PxF32 bspArea, bspVolume;
	bsp->getSurfaceAreaAndVolume(bspArea, bspVolume, true);
	physx::PxF32 remainingBSPArea, remainingBSPVolume;
	sourceBSP.getSurfaceAreaAndVolume(remainingBSPArea, remainingBSPVolume, true, ApexCSG::Operation::A_Minus_B);
	if (minVolume <= 0 || (bspVolume >= minVolume && remainingBSPVolume >= minVolume))
	{
		sourceBSP.op(sourceBSP, ApexCSG::Operation::A_Minus_B);
		childVolume = bspVolume;
	}
	else
	{
		// We will ignore this slice
		if (remainingBSPVolume >= minVolume)
		{
			// chunk bsp volume too small
			slicePlanes[sliceDir] = oldSlicePlane;
			bsp->release();
			bsp = NULL;
			sourceBSP.op(sourceBSP, ApexCSG::Operation::Set_A);
			childVolume = 0.0f;
		}
		else
		{
			// remainder is too small.  Terminate slicing along this direction
			bsp->op(sourceBSP, ApexCSG::Operation::Set_A);
			sourceBSP.op(sourceBSP, ApexCSG::Operation::Empty_Set);
			childVolume = bspVolume + remainingBSPVolume;
		}
	}
#endif
	return bsp;
}

static bool hierarchicallySplitChunkInternal
(
 physx::ExplicitHierarchicalMesh& hMesh,
 physx::PxU32 chunkIndex,
 physx::PxU32 relativeSliceDepth,
 physx::PxPlane chunkTrailingPlanes[3],
 physx::PxPlane chunkLeadingPlanes[3],
 const ApexCSG::IApexBSP& chunkBSP,
 physx::PxF32 chunkVolume,
 const physx::NxFractureSliceDesc& desc,
 const physx::NxCollisionDesc& collisionDesc,
 physx::IProgressListener& progressListener,
 volatile bool* cancel
 );

static bool createChunk
(
	physx::ExplicitHierarchicalMesh& hMesh,
	physx::PxU32 chunkIndex,
	physx::PxU32 relativeSliceDepth,
	physx::PxPlane trailingPlanes[3],
	physx::PxPlane leadingPlanes[3],
	physx::PxF32 chunkVolume,
	const physx::NxFractureSliceDesc& desc,
	const ApexCSG::IApexBSP& parentBSP,
	const ApexCSG::IApexBSP& fractureBSP,
	const physx::NxSliceParameters& sliceParameters,
	const physx::NxCollisionDesc& collisionDesc,
	physx::IProgressListener& progressListener,
	volatile bool* cancel
)
{
	bool canceled = false;
	ApexCSG::IApexBSP* chunkBSP = createBSP(hMesh.mBSPMemCache);
#if 0
	chunkBSP->copy(parentBSP);
	chunkBSP->combine(fractureBSP);
#else
	chunkBSP->copy(fractureBSP);
	chunkBSP->combine(parentBSP);
#endif
	chunkBSP->op(*chunkBSP, ApexCSG::Operation::Intersection);

	if (chunkBSP->getType() == ApexCSG::BSPType::Empty_Set)
	{
		return true;
	}

	if (gIslandGeneration)
	{
		chunkBSP = chunkBSP->decomposeIntoIslands();
	}

	physx::NxCollisionVolumeDesc volumeDesc = getVolumeDesc(collisionDesc, hMesh.depth(chunkIndex)+1);

	const physx::PxVec3 minimumExtents = hMesh.chunkBounds(0).getExtents().multiply(physx::PxVec3(desc.minimumChunkSize[0], desc.minimumChunkSize[1], desc.minimumChunkSize[2]));

	while (chunkBSP != NULL)
	{
		if (!canceled)
		{
			// Create a mesh with chunkBSP (or its islands)
			const physx::PxU32 newPartIndex = hMesh.addPart();
			const physx::PxU32 newChunkIndex = hMesh.addChunk();
			chunkBSP->toMesh(hMesh.mParts[newPartIndex]->mMesh);
			hMesh.buildMeshBounds(newPartIndex);
			hMesh.buildCollisionGeometryForPart(newPartIndex, volumeDesc);
			hMesh.mChunks[newChunkIndex]->mParentIndex = (physx::PxI32)chunkIndex;
			hMesh.mChunks[newChunkIndex]->mPartIndex = (physx::PxI32)newPartIndex;
			if (hMesh.mParts[(physx::PxU32)hMesh.mChunks[chunkIndex]->mPartIndex]->mFlags & ExplicitHierarchicalMesh::Part::MeshOpen)
			{
				hMesh.mParts[newPartIndex]->mFlags |= ExplicitHierarchicalMesh::Part::MeshOpen;
			}
			// Trim hull in directions where splitting is noisy
			for (physx::PxU32 i = 0; i < 3; ++i)
			{
				if ((sliceParameters.noise[i].amplitude != 0.0f || volumeDesc.mHullMethod != physx::NxConvexHullMethod::WRAP_GRAPHICS_MESH) &&
					volumeDesc.mHullMethod != physx::NxConvexHullMethod::CONVEX_DECOMPOSITION)
				{
					for (physx::PxU32 hullIndex = 0; hullIndex < hMesh.mParts[newPartIndex]->mCollision.size(); ++hullIndex)
					{
						physx::PartConvexHullProxy& hull = *hMesh.mParts[newPartIndex]->mCollision[hullIndex];
						physx::PxF32 min, max;
						hull.impl.extent(min, max, trailingPlanes[i].n);
						if (max > min)
						{
							physx::PxPlane clipPlane = trailingPlanes[i];
							clipPlane.d = physx::PxMin(clipPlane.d, -(0.8f * (max - min) + min));	// 20% clip bound
							hull.impl.intersectPlaneSide(clipPlane);
						}
						hull.impl.extent(min, max, leadingPlanes[i].n);
						if (max > min)
						{
							physx::PxPlane clipPlane = leadingPlanes[i];
							clipPlane.d = physx::PxMin(clipPlane.d, -(0.8f * (max - min) + min));	// 20% clip bound
							hull.impl.intersectPlaneSide(clipPlane);
						}
					}
				}
			}
			if (hMesh.mParts[newPartIndex]->mMesh.size() > 0 && hMesh.mParts[newPartIndex]->mCollision.size() > 0 && // We have a mesh and collision hulls
				(hMesh.chunkBounds(newChunkIndex).getExtents() - minimumExtents).minElement() >= 0.0f)	// Chunk is large enough
			{
				// Proper chunk
				hMesh.mParts[newPartIndex]->mMeshBSP->copy(*chunkBSP);
				if (relativeSliceDepth < desc.maxDepth)
				{
					// Recurse
					canceled = !hierarchicallySplitChunkInternal(hMesh, newChunkIndex, relativeSliceDepth, trailingPlanes, leadingPlanes, *chunkBSP, chunkVolume, desc, collisionDesc, progressListener, cancel);
				}
			}
			else
			{
				// No mesh, no colision, or too small.  Eliminate.
				hMesh.removeChunk(newChunkIndex);
				hMesh.removePart(newPartIndex);
			}
		}
		if (chunkBSP == &parentBSP)
		{
			// No islands were generated; break from loop
			break;
		}
		// Get next bsp in island decomposition
		ApexCSG::IApexBSP* nextBSP = chunkBSP->getNext();
		chunkBSP->release();
		chunkBSP = nextBSP;
	}

	return !canceled;
}

static bool hierarchicallySplitChunkInternal
(
    physx::ExplicitHierarchicalMesh& hMesh,
    physx::PxU32 chunkIndex,
    physx::PxU32 relativeSliceDepth,
    physx::PxPlane chunkTrailingPlanes[3],
    physx::PxPlane chunkLeadingPlanes[3],
    const ApexCSG::IApexBSP& chunkBSP,
    physx::PxF32 chunkVolume,
    const physx::NxFractureSliceDesc& desc,
	const physx::NxCollisionDesc& collisionDesc,
    physx::IProgressListener& progressListener,
    volatile bool* cancel
)
{
	if (relativeSliceDepth >= desc.maxDepth)
	{
		return true;	// No slice parameters at this depth
	}

	const physx::PxBounds3 bounds = hMesh.chunkBounds(chunkIndex);

	if (chunkIndex >= hMesh.chunkCount() || bounds.isEmpty())
	{
		return true;	// Done, nothing in chunk
	}

	bool canceled = false;	// our own copy of *cancel

	physx::PxVec3 center = bounds.getCenter();
	physx::PxVec3 extent = bounds.getExtents();

	if (relativeSliceDepth == 0)
	{
		chunkTrailingPlanes[0] = physx::PxPlane(-1, 0, 0, bounds.minimum[0]);
		chunkTrailingPlanes[1] = physx::PxPlane(0, -1, 0, bounds.minimum[1]);
		chunkTrailingPlanes[2] = physx::PxPlane(0, 0, -1, bounds.minimum[2]);
		chunkLeadingPlanes[0] = physx::PxPlane(1, 0, 0, -bounds.maximum[0]);
		chunkLeadingPlanes[1] = physx::PxPlane(0, 1, 0, -bounds.maximum[1]);
		chunkLeadingPlanes[2] = physx::PxPlane(0, 0, 1, -bounds.maximum[2]);
	}

	// Get parameters for this depth
	const physx::NxSliceParameters& sliceParameters = desc.sliceParameters[relativeSliceDepth++];

	// Determine slicing at this level
	int partition[3];
	calculatePartition(partition, sliceParameters.splitsPerPass, extent, desc.useTargetProportions ? desc.targetProportions : NULL);

	// Slice volume rejection ratio, perhaps should be exposed
	const physx::PxF32 volumeRejectionRatio = 0.1f;
	// Resulting slices must have at least this volume
	const physx::PxF32 minChildVolume = volumeRejectionRatio * chunkVolume / (partition[0] * partition[1] * partition[2]);

	const bool slicingThrough = sliceParameters.order >= 6;

	const physx::PxU32 sliceDirOrder = slicingThrough ? 0u : (physx::PxU32)sliceParameters.order;
	const physx::PxU32 sliceDir0 = (physx::PxU32)gSliceDirs[sliceDirOrder][0];
	const physx::PxU32 sliceDir1 = (physx::PxU32)gSliceDirs[sliceDirOrder][1];
	const physx::PxU32 sliceDir2 = (physx::PxU32)gSliceDirs[sliceDirOrder][2];
	const physx::PxF32 sliceWidths[3] = { 2.0f * extent[0] / partition[0], 2.0f * extent[1] / partition[1], 2.0f * extent[2] / partition[2] };

	physx::HierarchicalProgressListener localProgressListener(physx::PxMax(partition[0]*partition[1]*partition[2], 1), &progressListener);

	// If we are slicing through, then we need to cache the slice BSPs in the 2nd and 3rd directions
	physx::Array<ApexCSG::IApexBSP*> sliceBSPs1;
	physx::Array<ApexCSG::IApexBSP*> sliceBSPs2;
	if (slicingThrough)
	{
		sliceBSPs1.resize((physx::PxU32)partition[(physx::PxU32)gSliceDirs[sliceDirOrder][1]] - 1, NULL);
		sliceBSPs2.resize((physx::PxU32)partition[(physx::PxU32)gSliceDirs[sliceDirOrder][2]] - 1, NULL);
	}

	// If we are not slicingb through, we can re-use this sliceBSP
	ApexCSG::IApexBSP* reusedSliceBSP = NULL;

	physx::PxPlane trailingPlanes[3];
	physx::PxPlane leadingPlanes[3];

	physx::PxF32 childVolume = 0.0f;

	ApexCSG::IApexBSP* fractureBSP0 = createBSP(hMesh.mBSPMemCache, chunkBSP.getInternalTransform());

	const int sliceDepth = (int)hMesh.depth(chunkIndex) + 1;

	trailingPlanes[sliceDir0] = chunkTrailingPlanes[sliceDir0];
	leadingPlanes[sliceDir0] = physx::PxPlane(-trailingPlanes[sliceDir0].n, -trailingPlanes[sliceDir0].d);
	for (int sliceDir0Num = 0; sliceDir0Num < partition[sliceDir0] && !canceled; ++sliceDir0Num)
	{
		ApexCSG::IApexBSP* fractureBSP1 = fractureBSP0;	// This is the default; if there is a need to slice it will be replaced below.
		if (sliceDir0Num + 1 < partition[sliceDir0])
		{
			// Slice off piece in the 0 direction
			fractureBSP1 = createFractureBSP(leadingPlanes, reusedSliceBSP, *fractureBSP0, hMesh, childVolume, 0, center, extent, (physx::PxI32)sliceDir0, sliceDir0Num, sliceDepth, sliceWidths,
			                                 sliceParameters.linearVariation, sliceParameters.angularVariation, sliceParameters.noise[sliceDir0],
											 desc.materialDesc[sliceDir0], (physx::PxI32)desc.noiseMode, desc.useDisplacementMaps);
			reusedSliceBSP->release();
			reusedSliceBSP = NULL;
		}
		else
		{
			leadingPlanes[sliceDir0] = chunkLeadingPlanes[sliceDir0];
		}
		trailingPlanes[sliceDir1] = chunkTrailingPlanes[sliceDir1];
		leadingPlanes[sliceDir1] = physx::PxPlane(-trailingPlanes[sliceDir1].n, -trailingPlanes[sliceDir1].d);
		for (int sliceDir1Num = 0; sliceDir1Num < partition[sliceDir1] && !canceled; ++sliceDir1Num)
		{
			ApexCSG::IApexBSP* fractureBSP2 = fractureBSP1;	// This is the default; if there is a need to slice it will be replaced below.
			if (sliceDir1Num + 1 < partition[sliceDir1])
			{
				// Slice off piece in the 1 direction
				ApexCSG::IApexBSP*& sliceBSP = !slicingThrough ? reusedSliceBSP : sliceBSPs1[(physx::PxU32)sliceDir1Num];
				fractureBSP2 = createFractureBSP(leadingPlanes, sliceBSP, *fractureBSP1, hMesh, childVolume, 0, center, extent, (physx::PxI32)sliceDir1, sliceDir1Num, sliceDepth,
				                                 sliceWidths, sliceParameters.linearVariation, sliceParameters.angularVariation, sliceParameters.noise[sliceDir1],
												 desc.materialDesc[sliceDir1], (physx::PxI32)desc.noiseMode, desc.useDisplacementMaps);
				if (sliceBSP == reusedSliceBSP)
				{
					reusedSliceBSP->release();
					reusedSliceBSP = NULL;
				}
			}
			else
			{
				leadingPlanes[sliceDir1] = chunkLeadingPlanes[sliceDir1];
			}
			trailingPlanes[sliceDir2] = chunkTrailingPlanes[sliceDir2];
			leadingPlanes[sliceDir2] = physx::PxPlane(-trailingPlanes[sliceDir2].n, -trailingPlanes[sliceDir2].d);
			for (int sliceDir2Num = 0; sliceDir2Num < partition[sliceDir2] && !canceled; ++sliceDir2Num)
			{
				ApexCSG::IApexBSP* fractureBSP3 = fractureBSP2;	// This is the default; if there is a need to slice it will be replaced below.
				if (sliceDir2Num + 1 < partition[sliceDir2])
				{
					// Slice off piece in the 2 direction
					ApexCSG::IApexBSP*& sliceBSP = !slicingThrough ? reusedSliceBSP : sliceBSPs2[(physx::PxU32)sliceDir2Num];
					fractureBSP3 = createFractureBSP(leadingPlanes, sliceBSP, *fractureBSP2, hMesh, childVolume, minChildVolume, center, extent, (physx::PxI32)sliceDir2, sliceDir2Num, sliceDepth,
					                                 sliceWidths, sliceParameters.linearVariation, sliceParameters.angularVariation, sliceParameters.noise[sliceDir2],
													 desc.materialDesc[sliceDir2], (physx::PxI32)desc.noiseMode, desc.useDisplacementMaps);
					if (sliceBSP == reusedSliceBSP)
					{
						reusedSliceBSP->release();
						reusedSliceBSP = NULL;
					}
				}
				else
				{
					leadingPlanes[sliceDir2] = chunkLeadingPlanes[sliceDir2];
				}
				if (fractureBSP3 != NULL)
				{
					if (hMesh.mParts[(physx::PxU32)hMesh.mChunks[chunkIndex]->mPartIndex]->mFlags & ExplicitHierarchicalMesh::Part::MeshOpen)
					{
						fractureBSP3->deleteTriangles();	// Don't use interior triangles on an open mesh
					}
					canceled = !createChunk(hMesh, chunkIndex, relativeSliceDepth, trailingPlanes, leadingPlanes, childVolume, desc, chunkBSP, *fractureBSP3, sliceParameters, collisionDesc, localProgressListener, cancel);
				}
				localProgressListener.completeSubtask();
				// We no longer need fractureBSP3
				if (fractureBSP3 != NULL && fractureBSP3 != fractureBSP2)
				{
					fractureBSP3->release();
					fractureBSP3 = NULL;
				}
				trailingPlanes[sliceDir2] = physx::PxPlane(-leadingPlanes[sliceDir2].n, -leadingPlanes[sliceDir2].d);
				// Check for cancellation
				if (cancel != NULL && *cancel)
				{
					canceled = true;
				}
			}
			// We no longer need fractureBSP2
			if (fractureBSP2 != NULL && fractureBSP2 != fractureBSP1)
			{
				fractureBSP2->release();
				fractureBSP2 = NULL;
			}
			trailingPlanes[sliceDir1] = physx::PxPlane(-leadingPlanes[sliceDir1].n, -leadingPlanes[sliceDir1].d);
			// Check for cancellation
			if (cancel != NULL && *cancel)
			{
				canceled = true;
			}
		}
		// We no longer need fractureBSP1
		if (fractureBSP1 != NULL && fractureBSP1 != fractureBSP0)
		{
			fractureBSP1->release();
			fractureBSP1 = NULL;
		}
		trailingPlanes[sliceDir0] = physx::PxPlane(-leadingPlanes[sliceDir0].n, -leadingPlanes[sliceDir0].d);
		// Check for cancellation
		if (cancel != NULL && *cancel)
		{
			canceled = true;
		}
	}
	fractureBSP0->release();

	while (sliceBSPs2.size())
	{
		if (sliceBSPs2.back() != NULL)
		{
			sliceBSPs2.back()->release();
		}
		sliceBSPs2.popBack();
	}
	while (sliceBSPs1.size())
	{
		if (sliceBSPs1.back() != NULL)
		{
			sliceBSPs1.back()->release();
		}
		sliceBSPs1.popBack();
	}

	return !canceled;
}


struct TriangleLockInfo
{
	TriangleLockInfo() : lockedVertices(0), lockedEdges(0), originalTriangleIndex(0xFFFFFFFF) {}

	physx::PxU16 lockedVertices;	// (lockedVertices>>N)&1 => vertex N is locked
	physx::PxU16 lockedEdges;		// (lockedEdges>>M)&1 => edge M is locked
	physx::PxU32 originalTriangleIndex;
};

PX_INLINE physx::PxF32 square(physx::PxF32 x)
{
	return x*x;
}

// Returns edge of triangle, and position on edge (in pointOnEdge) if an edge is split, otherwise returns -1
// If a valid edge index is returned, also returns distance squared from the point to the edge in perp2
PX_INLINE physx::PxI32 pointOnAnEdge(physx::PxVec3& pointOnEdge, physx::PxF32& perp2, const physx::PxVec3& point, const physx::NxExplicitRenderTriangle& triangle, physx::PxF32 paraTol2, physx::PxF32 perpTol2)
{
	physx::PxI32 edgeIndex = -1;
	physx::PxF32 closestPerp2E2 = 1.0f;
	physx::PxF32 closestE2 = 0.0f;

	for (physx::PxU32 i = 0; i < 3; ++i)
	{
		const physx::PxVec3& v0 = triangle.vertices[i].position;
		const physx::PxVec3& v1 = triangle.vertices[(i+1)%3].position;
		const physx::PxVec3 e = v1 - v0;
		const physx::PxF32 e2 = e.magnitudeSquared();
		const physx::PxF32 perpTol2e2 = perpTol2*e2;
		const physx::PxVec3 d0 = point - v0;
		const physx::PxF32 d02 = d0.magnitudeSquared();
		if (d02 < paraTol2)
		{
			return -1;
		}
		if (e2 <= 4.0f*paraTol2)
		{
			continue;
		}
		const physx::PxF32 d0e = d0.dot(e);
		if (d0e < 0.0f || d0e > e2)
		{
			continue;	// point does not project down onto the edge
		}
		const physx::PxF32 perp2e2 = d0.cross(e).magnitudeSquared();
		if (perp2e2 > perpTol2e2)
		{
			continue;	// Point too far from edge
		}
		// Point is close to an edge.  Consider it if it's the closest.
		if (perp2e2*closestE2 < closestPerp2E2*e2)
		{
			closestPerp2E2 = perp2e2;
			closestE2 = e2;
			edgeIndex = (physx::PxI32)i;
		}
	}

	if (edgeIndex < 0 || closestE2 == 0.0f)
	{
		return -1;
	}

	const physx::PxVec3& v0 = triangle.vertices[edgeIndex].position;
	const physx::PxVec3& v1 = triangle.vertices[(edgeIndex+1)%3].position;

	if ((point-v0).magnitudeSquared() < paraTol2 || (point-v1).magnitudeSquared() < paraTol2)
	{
		return -1;
	}

	pointOnEdge = point;
	perp2 = closestE2 > 0.0f ? closestPerp2E2/closestE2 : 0.0f;

	return edgeIndex;
}

// Returns shared edge of triangleA if an edge is shared, otherwise returns -1
PX_INLINE physx::PxI32 trianglesShareEdge(const physx::NxExplicitRenderTriangle& triangleA, const physx::NxExplicitRenderTriangle& triangleB, physx::PxF32 tol2)
{
	for (physx::PxU32 i = 0; i < 3; ++i)
	{
		const physx::PxVec3 eA = triangleA.vertices[(i+1)%3].position - triangleA.vertices[i].position;
		const physx::PxF32 eA2 = eA.magnitudeSquared();
		const physx::PxF32 tol2eA2 = tol2*eA2;
		// We will search for edges pointing in the opposite direction only
		for (physx::PxU32 j = 0; j < 3; ++j)
		{
			const physx::PxVec3 d0 = triangleB.vertices[j].position - triangleA.vertices[i].position;
			const physx::PxF32 d0A = d0.dot(eA);
			if (d0A <= 0.0f)
			{
				continue;	// edge on B starts before edge on A
			}
			const physx::PxVec3 d1 = triangleB.vertices[(j+1)%3].position - triangleA.vertices[i].position;
			const physx::PxF32 d1A = d1.dot(eA);
			if (d1A >= eA2)
			{
				continue;	// edge on B ends after edge on A
			}
			if (d0A <= d1A)
			{
				continue;	// edges don't point in opposite directions
			}
			if (d0.cross(eA).magnitudeSquared() > tol2eA2)
			{
				continue;	// one vertex on B is not close enough to the edge of A
			}
			if (d1.cross(eA).magnitudeSquared() > tol2eA2)
			{
				continue;	// other vertex on B is not close enough to the edge of A
			}
			// These edges appear to have an overlap, to within tolerance
			return (physx::PxI32)i;
		}
	}

	return -1;
}

// Positive tol means that interference will be registered even if the triangles are a small distance apart.
// Negative tol means that interference will not be registered even if the triangles have a small overlap.
PX_INLINE bool trianglesInterfere(const physx::NxExplicitRenderTriangle& triangleA, const physx::NxExplicitRenderTriangle& triangleB, physx::PxF32 tol)
{
	// Check extent of B relative to plane of A
	const physx::PxPlane planeA(0.333333333f*(triangleA.vertices[0].position + triangleA.vertices[1].position + triangleA.vertices[2].position), triangleA.calculateNormal().getNormalized());
	physx::PxVec3 dispB(planeA.distance(triangleB.vertices[0].position), planeA.distance(triangleB.vertices[1].position), planeA.distance(triangleB.vertices[2].position));
	if (extentDistance(dispB.minElement(), dispB.maxElement(), 0.0f, 0.0f) > tol)
	{
		return false;
	}

	// Check extent of A relative to plane of B
	const physx::PxPlane planeB(0.333333333f*(triangleB.vertices[0].position + triangleB.vertices[1].position + triangleB.vertices[2].position), triangleB.calculateNormal().getNormalized());
	physx::PxVec3 dispA(planeB.distance(triangleA.vertices[0].position), planeB.distance(triangleA.vertices[1].position), planeB.distance(triangleA.vertices[2].position));
	if (extentDistance(dispA.minElement(), dispA.maxElement(), 0.0f, 0.0f) > tol)
	{
		return false;
	}

	for (physx::PxU32 i = 0; i < 3; ++i)
	{
		physx::PxVec3 eA = triangleA.vertices[(i+1)%3].position - triangleA.vertices[i].position;
		eA.normalize();
		for (physx::PxU32 j = 0; j < 3; ++j)
		{
			physx::PxVec3 eB = triangleB.vertices[(j+1)%3].position - triangleB.vertices[j].position;
			eB.normalize();
			physx::PxVec3 n = eA.cross(eB);
			if (n.normalize() > 0.00001f)
			{
				dispA = physx::PxVec3(n.dot(triangleA.vertices[0].position), n.dot(triangleA.vertices[1].position), n.dot(triangleA.vertices[2].position));
				dispB = physx::PxVec3(n.dot(triangleB.vertices[0].position), n.dot(triangleB.vertices[1].position), n.dot(triangleB.vertices[2].position));
				if (extentDistance(dispA.minElement(), dispA.maxElement(), dispB.minElement(), dispB.maxElement()) > tol)
				{
					return false;
				}
			}
		}
	}

	return true;
}

PX_INLINE bool segmentIntersectsTriangle(const physx::PxVec3 orig, const physx::PxVec3 dest, const physx::NxExplicitRenderTriangle& triangle, physx::PxF32 tol)
{
	// Check extent of segment relative to plane of triangle
	const physx::PxPlane plane(0.333333333f*(triangle.vertices[0].position + triangle.vertices[1].position + triangle.vertices[2].position), triangle.calculateNormal().getNormalized());
	const physx::PxF32 dist0 = plane.distance(orig);
	const physx::PxF32 dist1 = plane.distance(dest);
	if (extentDistance(physx::PxMin(dist0, dist1), physx::PxMax(dist0, dist1), 0.0f, 0.0f) > tol)
	{
		return false;
	}

	// Test to see if the segment goes through the triangle
	const physx::PxF32 signDist0 = physx::PxSign(dist0);
	const physx::PxVec3 disp = dest-orig;
	const physx::PxVec3 relV[3] = {triangle.vertices[0].position - orig, triangle.vertices[1].position - orig, triangle.vertices[2].position - orig};
	for (physx::PxU32 v = 0; v < 3; ++v)
	{
		if (physx::PxSign(relV[v].cross(relV[(v+1)%3]).dot(disp)) == signDist0)
		{
			return false;
		}
	}

	return true;
}

struct VertexRep
{
	BoundsRep		bounds;
	physx::PxVec3	position;	// Not necessarily the center of bounds, after we snap vertices
	physx::PxVec3	normal;
};

class MeshProcessor
{
public:
	class FreeVertexIt
	{
	public:
		FreeVertexIt(MeshProcessor& meshProcessor) : mMeshProcessor(meshProcessor), mVertexRep(NULL)
		{
			mTriangleIndex = meshProcessor.mTrianglePartition;
			mVertexIndex = 0;
			advanceToNextValidState();
		}

		PX_INLINE bool valid() const
		{
			return mTriangleIndex < mMeshProcessor.mMesh->size();
		}

		PX_INLINE void inc()
		{
			if (valid())
			{
				++mVertexIndex;
				advanceToNextValidState();
			}
		}

		PX_INLINE physx::PxU32 triangleIndex() const
		{
			return mTriangleIndex;
		}

		PX_INLINE physx::PxU32 vertexIndex() const
		{
			return mVertexIndex;
		}

		PX_INLINE VertexRep& vertexRep() const
		{
			return *mVertexRep;
		}

	private:
		FreeVertexIt& operator=(const FreeVertexIt&);

		PX_INLINE void advanceToNextValidState()
		{
			for (; valid(); ++mTriangleIndex, mVertexIndex = 0)
			{
				for (; mVertexIndex < 3; ++mVertexIndex)
				{
					const physx::PxU32 relativeTriangleIndex = mTriangleIndex-mMeshProcessor.mTrianglePartition;
					if (!((mMeshProcessor.mTriangleInfo[relativeTriangleIndex].lockedVertices >> mVertexIndex)&1))
					{
						mVertexRep = &mMeshProcessor.mVertexBounds[3*relativeTriangleIndex + mVertexIndex];
						return;
					}
				}
			}
		}

		MeshProcessor&	mMeshProcessor;
		physx::PxU32	mTriangleIndex;
		physx::PxU32	mVertexIndex;
		VertexRep*		mVertexRep;
	};

	class FreeNeighborVertexIt
	{
	public:
		FreeNeighborVertexIt(MeshProcessor& meshProcessor, physx::PxU32 triangleIndex, physx::PxU32 vertexIndex) 
			: mMeshProcessor(meshProcessor)
			, mTriangleIndex(triangleIndex)
			, mVertexIndex(vertexIndex)
			, mVertexRep(NULL)
		{
			const physx::PxU32 vertexRepIndex = 3*(triangleIndex - mMeshProcessor.mTrianglePartition) + vertexIndex;
			mNeighbors = meshProcessor.mVertexNeighborhoods.getNeighbors(vertexRepIndex);
			mNeighborStop = mNeighbors + meshProcessor.mVertexNeighborhoods.getNeighborCount(vertexRepIndex);
			advanceToNextValidState();
		}

		PX_INLINE bool valid() const
		{
			return mNeighbors < mNeighborStop;
		}

		PX_INLINE void inc()
		{
			if (valid())
			{
				++mNeighbors;
				advanceToNextValidState();
			}
		}

		PX_INLINE physx::PxU32 triangleIndex() const
		{
			return mTriangleIndex;
		}

		PX_INLINE physx::PxU32 vertexIndex() const
		{
			return mVertexIndex;
		}

		PX_INLINE VertexRep& vertexRep() const
		{
			return *mVertexRep;
		}

	private:
		FreeNeighborVertexIt& operator=(const FreeNeighborVertexIt&);

		PX_INLINE void advanceToNextValidState()
		{
			for (; valid(); ++mNeighbors)
			{
				const physx::PxU32 neighbor = *mNeighbors;
				const physx::PxU32 relativeTriangleIndex = neighbor/3;
				mTriangleIndex = mMeshProcessor.mTrianglePartition + relativeTriangleIndex;
				mVertexIndex = neighbor - 3*relativeTriangleIndex;
				mVertexRep = &mMeshProcessor.mVertexBounds[neighbor];
				if (!((mMeshProcessor.mTriangleInfo[relativeTriangleIndex].lockedVertices >> mVertexIndex)&1))
				{
					return;
				}
			}
		}

		MeshProcessor&		mMeshProcessor;
		const physx::PxU32*	mNeighbors;
		const physx::PxU32*	mNeighborStop;
		physx::PxU32		mTriangleIndex;
		physx::PxU32		mVertexIndex;
		VertexRep*			mVertexRep;
	};

	MeshProcessor()
	{
		reset();
	}

	// Removes any triangles with a width less than minWidth
	static void removeSlivers(physx::Array<physx::NxExplicitRenderTriangle>& mesh, physx::PxF32 minWidth)
	{
		const physx::PxF32 minWidth2 = minWidth*minWidth;
		for (physx::PxU32 i = mesh.size(); i--;)
		{
			physx::NxExplicitRenderTriangle& triangle = mesh[i];
			for (physx::PxU32 j = 0; j < 3; ++j)
			{
				const physx::PxVec3 edge = (triangle.vertices[(j+1)%3].position - triangle.vertices[j].position).getNormalized();
				if ((triangle.vertices[(j+2)%3].position - triangle.vertices[j].position).cross(edge).magnitudeSquared() < minWidth2)
				{
					mesh.replaceWithLast(i);
					break;
				}
			}
		}
	}

	// trianglePartition is a point in the part mesh where we want to start processing.  We will assume that triangles
	// before this index are locked, and vertices in triangles after this index will be locked if they coincide with
	// locked triangles
	void setMesh(physx::Array<physx::NxExplicitRenderTriangle>& mesh, physx::Array<physx::NxExplicitRenderTriangle>* parentMesh, physx::PxU32 trianglePartition, physx::PxF32 tol)
	{
		reset();

		if (mesh.size() == 0)
		{
			return;
		}

		mMesh = &mesh;
		mTrianglePartition = physx::PxMin(trianglePartition, mesh.size());
		mTol = physx::PxAbs(tol);
		mPadding = 2*mTol;

		mParentMesh = parentMesh;

		// Find part triangle neighborhoods, expanding triangle bounds by some padding factor
		mOriginalTriangleBounds.resize(mesh.size());
		for (physx::PxU32 i = 0; i < mesh.size(); ++i)
		{
			physx::NxExplicitRenderTriangle& triangle = mesh[i];
			mOriginalTriangleBounds[i].aabb.setEmpty();
			mOriginalTriangleBounds[i].aabb.include(triangle.vertices[0].position);
			mOriginalTriangleBounds[i].aabb.include(triangle.vertices[1].position);
			mOriginalTriangleBounds[i].aabb.include(triangle.vertices[2].position);
			mOriginalTriangleBounds[i].aabb.fattenFast(mPadding);
		}
		mOriginalTriangleNeighborhoods.setBounds(&mOriginalTriangleBounds[0], mOriginalTriangleBounds.size(), sizeof(mOriginalTriangleBounds[0]));

		// Create additional triangle info.  This will parallel the mesh after the partition
		mTriangleInfo.resize(mesh.size()-mTrianglePartition);
		// Also create triangle interpolators from the original triangles
		mTriangleFrames.resize(mesh.size()-mTrianglePartition);
		// As well as child triangle info
		mTriangleChildLists.resize(mesh.size()-mTrianglePartition);
		for (physx::PxU32 i = mTrianglePartition; i < mesh.size(); ++i)
		{
			mTriangleInfo[i-mTrianglePartition].originalTriangleIndex = i;	// Use the original triangles' neighborhood info
			mTriangleFrames[i-mTrianglePartition].setFromTriangle(mesh[i]);
			mTriangleChildLists[i-mTrianglePartition].pushBack(i);	// These are all of the triangles that use the corresponding triangle frame, and will be represented by the corresponding bounds for spatial lookup
		}
	}

	void lockBorderVertices()
	{
		physx::Array<physx::NxExplicitRenderTriangle>& mesh = *mMesh;

		const physx::PxF32 tol2 = mTol*mTol;

		for (physx::PxU32 i = mTrianglePartition; i < mesh.size(); ++i)
		{
			// Use neighbor info to find out if we should lock any of the vertices
			physx::NxExplicitRenderTriangle& triangle = mesh[i];
			const physx::PxU32 neighborCount = mOriginalTriangleNeighborhoods.getNeighborCount(i);
			const physx::PxU32* neighbors = mOriginalTriangleNeighborhoods.getNeighbors(i);
			for (physx::PxU32 j = 0; j < neighborCount; ++j)
			{
				const physx::PxU32 neighbor = *neighbors++;
				if (neighbor < mTrianglePartition)
				{	
					// Neighbor is not a process triangle - if it shares an edge then lock vertices
					const physx::PxI32 sharedEdge = trianglesShareEdge(triangle, mesh[neighbor], tol2);
					if (sharedEdge >= 0)
					{
						mTriangleInfo[i-mTrianglePartition].lockedVertices |= (1<<sharedEdge) | (1<<((sharedEdge+1)%3));
						mTriangleInfo[i-mTrianglePartition].lockedEdges |= 1<<sharedEdge;
					}
					// Check triangle vertices against neighbor's edges as well
					for (physx::PxU32 v = 0; v < 3; ++v)
					{
						const physx::PxVec3& point = triangle.vertices[v].position;
						physx::PxVec3 pointOnEdge;
						physx::PxF32 perp2;
						if (0 <= pointOnAnEdge(pointOnEdge, perp2, point, mesh[neighbor], 0.0f, tol2))
						{
							mTriangleInfo[i-mTrianglePartition].lockedVertices |= 1<<v;
						}
					}
				}
			}
		}
	}

	void removeTJunctions()
	{
		physx::Array<physx::NxExplicitRenderTriangle>& mesh = *mMesh;

		const physx::PxF32 tol2 = mTol*mTol;

		const physx::PxU32 originalMeshSize = mesh.size();
		for (physx::PxU32 i = mTrianglePartition; i < originalMeshSize; ++i)
		{
			const physx::PxU32 neighborCount = mOriginalTriangleNeighborhoods.getNeighborCount(i);
			for (physx::PxU32 v = 0; v < 3; ++v)
			{
				const physx::PxVec3& point = mesh[i].vertices[v].position;
				physx::PxI32 neighborToSplit = -1;
				physx::PxI32 edgeToSplit = -1;
				physx::PxF32 leastPerp2 = PX_MAX_F32;
				const physx::PxU32* neighbors = mOriginalTriangleNeighborhoods.getNeighbors(i);
				for (physx::PxU32 j = 0; j < neighborCount; ++j)
				{
					const physx::PxU32 originalNeighbor = *neighbors++;
					if (originalNeighbor >= mTrianglePartition)
					{
						physx::Array<physx::PxU32>& triangleChildList = mTriangleChildLists[originalNeighbor - mTrianglePartition];
						const physx::PxU32 triangleChildListSize = triangleChildList.size();
						for (physx::PxU32 k = 0; k < triangleChildListSize; ++k)
						{
							const physx::PxU32 neighbor = triangleChildList[k];
							// Neighbor is a process triangle - split it at this triangle's vertices
							const physx::PxVec3& point = mesh[i].vertices[v].position;
							physx::PxVec3 pointOnEdge;
							physx::PxF32 perp2 = 0.0f;
							const physx::PxI32 edge = pointOnAnEdge(pointOnEdge, perp2, point, mesh[neighbor], tol2, tol2);
							if (edge >= 0 && !((mTriangleInfo[neighbor - mTrianglePartition].lockedEdges >> edge)&1))
							{
								if (perp2 < leastPerp2)
								{
									neighborToSplit = (physx::PxI32)neighbor;
									edgeToSplit = edge;
									leastPerp2 = perp2;
								}
							}
						}
					}
				}
				if (neighborToSplit >= 0 && edgeToSplit >= 0)
				{
					splitTriangle((physx::PxU32)neighborToSplit, (physx::PxU32)edgeToSplit, point);
				}
			}
		}
	}

	void subdivide(physx::PxF32 maxEdgeLength)
	{
		physx::Array<physx::NxExplicitRenderTriangle>& mesh = *mMesh;
		const physx::PxF32 maxEdgeLength2 = maxEdgeLength*maxEdgeLength;

		const physx::PxF32 tol2 = mTol*mTol;

		// Pass through list and split edges that are too long
		bool splitDone;
		do
		{
			splitDone = false;
			for (physx::PxU32 i = mTrianglePartition; i < mesh.size(); ++i)	// this array (as well as info) might grow during the loop
			{
				physx::NxExplicitRenderTriangle& triangle = mesh[i];
				// Find the longest edge of this triangle
				const physx::PxF32 edgeLengthSquared[3] =
				{
					(triangle.vertices[1].position - triangle.vertices[0].position).magnitudeSquared(),
					(triangle.vertices[2].position - triangle.vertices[1].position).magnitudeSquared(),
					(triangle.vertices[0].position - triangle.vertices[2].position).magnitudeSquared()
				};
				const int longestEdge = edgeLengthSquared[1] > edgeLengthSquared[0] ? (edgeLengthSquared[2] > edgeLengthSquared[1] ? 2 : 1) : (edgeLengthSquared[2] > edgeLengthSquared[0] ? 2 : 0);
				if (edgeLengthSquared[longestEdge] > maxEdgeLength2)
				{
					// Split this edge
					const physx::NxExplicitRenderTriangle oldTriangle = triangle;	// Save off old triangle for neighbor edge check
					const physx::PxVec3 newVertexPosition = 0.5f*(triangle.vertices[longestEdge].position + triangle.vertices[(longestEdge + 1)%3].position);
					splitTriangle(i, (physx::PxU32)longestEdge, newVertexPosition);
					// Now split neighbor edges
					const physx::PxU32 neighborCount = mOriginalTriangleNeighborhoods.getNeighborCount(i);
					const physx::PxU32* neighbors = mOriginalTriangleNeighborhoods.getNeighbors(i);
					for (physx::PxU32 j = 0; j < neighborCount; ++j)
					{
						const physx::PxU32 originalNeighbor = *neighbors++;
						if (originalNeighbor >= mTrianglePartition)
						{
							physx::Array<physx::PxU32>& triangleChildList = mTriangleChildLists[originalNeighbor - mTrianglePartition];
							for (physx::PxU32 k = 0; k < triangleChildList.size(); ++k)
							{
								const physx::PxU32 neighbor = triangleChildList[k];
								if (neighbor >= mTrianglePartition)
								{	
									// Neighbor is a process triangle - split it too, if the neighbor shares an edge, and the split point is on the shared edge
									const physx::PxI32 sharedEdge = trianglesShareEdge(oldTriangle, mesh[neighbor], tol2);
									if (sharedEdge >= 0)
									{
										physx::PxVec3 pointOnEdge;
										physx::PxF32 perp2;
										const physx::PxI32 edgeToSplit = pointOnAnEdge(pointOnEdge, perp2, newVertexPosition, mesh[neighbor], tol2, tol2);
										if (edgeToSplit == sharedEdge && !((mTriangleInfo[neighbor - mTrianglePartition].lockedEdges >> edgeToSplit)&1))
										{
											splitTriangle(neighbor, (physx::PxU32)edgeToSplit, pointOnEdge);
										}
									}
								}
							}
						}
					}
					splitDone = true;
				}
			}
		} while (splitDone);
	}

	void snapVertices(physx::PxF32 snapTol)
	{
		physx::Array<physx::NxExplicitRenderTriangle>& mesh = *mMesh;

		// Create a small bounding cube for each vertex
		const physx::PxU32 vertexCount = 3*(mesh.size()-mTrianglePartition);
		mVertexBounds.resize(vertexCount);

		if (mVertexBounds.size() == 0)
		{
			return;
		}

		VertexRep* vertexRep = &mVertexBounds[0];
		for (physx::PxU32 i = mTrianglePartition; i < mesh.size(); ++i)
		{
			const physx::PxVec3 normal = mesh[i].calculateNormal();
			for (physx::PxU32 j = 0; j < 3; ++j, ++vertexRep)
			{
				vertexRep->bounds.aabb.include(mesh[i].vertices[j].position);
				vertexRep->bounds.aabb.fattenFast(snapTol);
				vertexRep->position = mesh[i].vertices[j].position;
				vertexRep->normal = normal;
			}
		}

		// Generate neighbor info
		mVertexNeighborhoods.setBounds(&mVertexBounds[0].bounds, vertexCount, sizeof(VertexRep));

		const physx::PxF32 snapTol2 = snapTol*snapTol;

		// Run through all free vertices, look for neighbors that are free, and snap them together
		for (MeshProcessor::FreeVertexIt it(*this); it.valid(); it.inc())
		{
			NxVertex& vertex = mesh[it.triangleIndex()].vertices[it.vertexIndex()];
			VertexRep& vertexRep = it.vertexRep();
			physx::PxU32 N = 1;
			const physx::PxVec3 oldVertexPosition = vertex.position;
			for (MeshProcessor::FreeNeighborVertexIt nIt(*this, it.triangleIndex(), it.vertexIndex()); nIt.valid(); nIt.inc())
			{
				NxVertex& neighborVertex = mesh[nIt.triangleIndex()].vertices[nIt.vertexIndex()];
				if ((neighborVertex.position-oldVertexPosition).magnitudeSquared() < snapTol2)
				{
					vertex.position += neighborVertex.position;
					vertexRep.normal += nIt.vertexRep().normal;
					++N;
				}
			}
			vertex.position *= 1.0f/N;
			vertexRep.position = vertex.position;
			vertexRep.normal.normalize();
			for (MeshProcessor::FreeNeighborVertexIt nIt(*this, it.triangleIndex(), it.vertexIndex()); nIt.valid(); nIt.inc())
			{
				NxVertex& neighborVertex = mesh[nIt.triangleIndex()].vertices[nIt.vertexIndex()];
				if ((neighborVertex.position-oldVertexPosition).magnitudeSquared() < snapTol2)
				{
					neighborVertex.position = vertex.position;
					nIt.vertexRep().position = vertex.position;
					nIt.vertexRep().normal = vertexRep.normal;
				}
			}
		}
	}

	void resolveIntersections(physx::PxF32 relaxationFactor = 0.5f, physx::PxU32 maxIterations = 10)
	{
		physx::Array<physx::NxExplicitRenderTriangle>& mesh = *mMesh;

		if (mesh.size() == 0 || mParentMesh == NULL)
		{
			return;
		}

		physx::Array<physx::NxExplicitRenderTriangle>& parentMesh = *mParentMesh;

		// Find neighborhoods for the new active triangles, the inactive triangles, and the face mesh triangles
		const physx::PxU32 parentMeshSize = parentMesh.size();
		physx::Array<BoundsRep> triangleBounds;
		triangleBounds.resize(parentMeshSize + mesh.size());
		// Triangles from the face mesh
		for (physx::PxU32 i = 0; i < parentMeshSize; ++i)
		{
			physx::NxExplicitRenderTriangle& triangle = parentMesh[i];
			BoundsRep& boundsRep = triangleBounds[i];
			boundsRep.aabb.setEmpty();
			for (physx::PxU32 v = 0; v < 3; ++v)
			{
				boundsRep.aabb.include(triangle.vertices[v].position);
			}
			boundsRep.aabb.fattenFast(mPadding);
		}
		// Triangles from the part mesh
		for (physx::PxU32 i = 0; i < mesh.size(); ++i)
		{
			physx::NxExplicitRenderTriangle& triangle = mesh[i];
			BoundsRep& boundsRep = triangleBounds[i+parentMeshSize];
			boundsRep.aabb.setEmpty();
			for (physx::PxU32 v = 0; v < 3; ++v)
			{
				boundsRep.aabb.include(triangle.vertices[v].position);
				// Also include the triangle's original vertices if it's a process triangle, so we can check for tunneling
				if (i >= mTrianglePartition)
				{
					VertexRep& vertexRep = mVertexBounds[3*(i-mTrianglePartition) + v];
					boundsRep.aabb.include(vertexRep.position);
				}
			}
			boundsRep.aabb.fattenFast(mPadding);
		}

		NeighborLookup triangleNeighborhoods;
		triangleNeighborhoods.setBounds(&triangleBounds[0], triangleBounds.size(), sizeof(triangleBounds[0]));

		const physx::NxExplicitRenderTriangle* parentMeshStart = parentMeshSize ? &parentMesh[0] : NULL;

		// Find interfering pairs of triangles
		physx::Array<IntPair> interferingPairs;
		for (physx::PxU32 repIndex = mTrianglePartition+parentMeshSize; repIndex < mesh.size()+parentMeshSize; ++repIndex)
		{
			const physx::PxU32 neighborCount = triangleNeighborhoods.getNeighborCount(repIndex);
			const physx::PxU32* neighborRepIndices = triangleNeighborhoods.getNeighbors(repIndex);
			for (physx::PxU32 j = 0; j < neighborCount; ++j)
			{
				const physx::PxU32 neighborRepIndex = *neighborRepIndices++;
				if (repIndex > neighborRepIndex)	// Only count each pair once
				{
					if (trianglesInterfere(repIndex, neighborRepIndex, parentMeshStart, parentMeshSize, -mTol))
					{
						IntPair& pair = interferingPairs.insert();
						pair.set((physx::PxI32)repIndex, (physx::PxI32)neighborRepIndex);
					}
				}
			}
		}

		// Now run through the interference list, pulling the vertices in the offending triangles back to
		// their original positions.  Iterate until there are no more interfering triangles, or the maximum
		// number of iterations is reached.
		physx::Array<bool> handled;
		handled.resize(mesh.size() - mTrianglePartition, false);
		for (physx::PxU32 iterN = 0; iterN < maxIterations && interferingPairs.size(); ++iterN)
		{
			for (physx::PxU32 pairN = 0; pairN < interferingPairs.size(); ++pairN)
			{
				const IntPair& pair = interferingPairs[pairN];
				const physx::PxU32 i0 = (physx::PxU32)pair.i0;
				const physx::PxU32 i1 = (physx::PxU32)pair.i1;
				if (i0 >= mTrianglePartition + parentMeshSize && !handled[i0 - mTrianglePartition - parentMeshSize])
				{
					relaxTriangleFreeVertices(i0 - parentMeshSize, relaxationFactor);
					handled[i0 - mTrianglePartition - parentMeshSize] = true;
				}
				if (i1 >= mTrianglePartition + parentMeshSize && !handled[i1 - mTrianglePartition - parentMeshSize])
				{
					relaxTriangleFreeVertices(i1 - parentMeshSize, relaxationFactor);
					handled[i1 - mTrianglePartition - parentMeshSize] = true;
				}
			}
			// We've given the vertices a relaxation pass.  Reset the handled list, and remove pairs that no longer interfere
			for (physx::PxU32 pairN = interferingPairs.size(); pairN--;)
			{
				const IntPair& pair = interferingPairs[pairN];
				const physx::PxU32 i0 = (physx::PxU32)pair.i0;
				const physx::PxU32 i1 = (physx::PxU32)pair.i1;
				if (i0 >= mTrianglePartition + parentMeshSize)
				{
					handled[i0 - mTrianglePartition - parentMeshSize] = false;
				}
				if (i1 >= mTrianglePartition + parentMeshSize)
				{
					handled[i1 - mTrianglePartition - parentMeshSize] = false;
				}
				if (!trianglesInterfere(i0, i1, parentMeshStart, parentMeshSize, -mTol))
				{
					interferingPairs.replaceWithLast(pairN);
				}
			}
		}
	}

private:
	void reset()
	{
		mMesh = NULL;
		mParentMesh = NULL;
		mTrianglePartition = 0;
		mOriginalTriangleBounds.resize(0);
		mTol = 0.0f;
		mPadding = 0.0f;
		mOriginalTriangleNeighborhoods.setBounds(NULL, 0, 0);
		mTriangleInfo.resize(0);
		mTriangleFrames.resize(0);
		mTriangleChildLists.resize(0);
		mVertexBounds.resize(0);
		mVertexNeighborhoods.setBounds(NULL, 0, 0);
	}

	PX_INLINE void splitTriangle(physx::PxU32 triangleIndex, physx::PxU32 edgeIndex, const physx::PxVec3& newVertexPosition)
	{
		physx::Array<physx::NxExplicitRenderTriangle>& mesh = *mMesh;
		physx::NxExplicitRenderTriangle& triangle = mesh[triangleIndex];
		const unsigned nextEdge = (edgeIndex + 1)%3;
		physx::NxExplicitRenderTriangle newTriangle = triangle;
		TriangleLockInfo& info = mTriangleInfo[triangleIndex-mTrianglePartition];
		TriangleLockInfo newInfo = info;
		const bool splitEdgeIsLocked = ((info.lockedEdges>>edgeIndex)&1) != 0;
		info.lockedEdges &= ~(physx::PxU16)(1<<((edgeIndex + 2)%3));	// New edge is not locked
		if (!splitEdgeIsLocked)
		{
			info.lockedVertices &= ~(physx::PxU16)(1<<edgeIndex);	// New vertex is not locked if split edge is not locked
		}
		newInfo.lockedEdges &= ~(physx::PxU16)(1<<nextEdge);	// New edge is not locked
		if (!splitEdgeIsLocked)
		{
			newInfo.lockedVertices &= ~(physx::PxU16)(1<<nextEdge);	// New vertex is not locked if split edge is not locked
		}
		const physx::TriangleFrame& triangleFrame = mTriangleFrames[newInfo.originalTriangleIndex-mTrianglePartition];
		triangle.vertices[edgeIndex].position = newVertexPosition;
		triangleFrame.interpolateVertexData(triangle.vertices[edgeIndex]);
		newTriangle.vertices[nextEdge]= triangle.vertices[edgeIndex];
		const physx::PxU32 newTriangleIndex = mesh.size();
		mesh.pushBack(newTriangle);
		mTriangleInfo.pushBack(newInfo);
		mTriangleChildLists[newInfo.originalTriangleIndex-mTrianglePartition].pushBack(newTriangleIndex);
	}

	PX_INLINE void relaxTriangleFreeVertices(physx::PxU32 triangleIndex, physx::PxF32 relaxationFactor)
	{
		const physx::PxF32 tol2 = mTol*mTol;
		physx::Array<physx::NxExplicitRenderTriangle>& mesh = *mMesh;
		const physx::PxU32 relativeIndex = triangleIndex - mTrianglePartition;
		TriangleLockInfo& info = mTriangleInfo[relativeIndex];
		for (physx::PxU32 v = 0; v < 3; ++v)
		{
			if (!((info.lockedVertices >> v)&1))
			{
				NxVertex& vertex = mesh[triangleIndex].vertices[v];
				VertexRep& vertexRep = mVertexBounds[3*relativeIndex + v];
				const physx::PxVec3 oldVertexPosition = vertex.position;
				vertex.position = (1.0f - relaxationFactor)*vertex.position + relaxationFactor*vertexRep.position;
				for (MeshProcessor::FreeNeighborVertexIt nIt(*this, triangleIndex, v); nIt.valid(); nIt.inc())
				{
					NxVertex& neighborVertex = mesh[nIt.triangleIndex()].vertices[nIt.vertexIndex()];
					if ((neighborVertex.position-oldVertexPosition).magnitudeSquared() < tol2)
					{
						neighborVertex.position = vertex.position;
					}
				}
			}
		}
	}

	PX_INLINE bool trianglesInterfere(physx::PxU32 indexA, physx::PxU32 indexB, const physx::NxExplicitRenderTriangle* parentMeshStart, physx::PxU32 parentMeshSize, physx::PxF32 tol)
	{
		physx::Array<physx::NxExplicitRenderTriangle>& mesh = *mMesh;
		const physx::NxExplicitRenderTriangle& triangleA = indexA >= parentMeshSize ? mesh[indexA - parentMeshSize] : parentMeshStart[indexA];
		const physx::NxExplicitRenderTriangle& triangleB = indexB >= parentMeshSize ? mesh[indexB - parentMeshSize] : parentMeshStart[indexB];

		// Check for static interference
		if (::trianglesInterfere(triangleA, triangleB, tol))
		{
			return true;
		}

		// See if one of the vertices of A swept through B
		if (indexA >= mTrianglePartition + parentMeshSize)
		{
			for (physx::PxU32 v = 0; v < 3; ++v)
			{
				VertexRep& vertexRep = mVertexBounds[3*(indexA-mTrianglePartition-parentMeshSize) + v];
				if (segmentIntersectsTriangle(vertexRep.position, triangleA.vertices[v].position, triangleB, tol))
				{
					return true;
				}
			}
		}

		// See if one of the vertices of B swept through A
		if (indexB >= mTrianglePartition + parentMeshSize)
		{
			for (physx::PxU32 v = 0; v < 3; ++v)
			{
				VertexRep& vertexRep = mVertexBounds[3*(indexB-mTrianglePartition-parentMeshSize) + v];
				if (segmentIntersectsTriangle(vertexRep.position, triangleB.vertices[v].position, triangleA, tol))
				{
					return true;
				}
			}
		}

		// No interference found
		return false;
	}


	physx::Array<physx::NxExplicitRenderTriangle>*	mMesh;
	physx::Array<physx::NxExplicitRenderTriangle>*	mParentMesh;
	physx::PxU32									mTrianglePartition;
	physx::Array<BoundsRep>							mOriginalTriangleBounds;
	physx::PxF32									mTol;
	physx::PxF32									mPadding;
	NeighborLookup									mOriginalTriangleNeighborhoods;
	physx::Array<TriangleLockInfo>					mTriangleInfo;
	physx::Array<physx::TriangleFrame>				mTriangleFrames;
	physx::Array< physx::Array<physx::PxU32> >		mTriangleChildLists;
	physx::Array<VertexRep>							mVertexBounds;
	NeighborLookup									mVertexNeighborhoods;

	friend class FreeVertexIt;
	friend class FreeNeighborVertexIt;
};


PX_INLINE bool triangleIsPartOfActiveSet(const physx::NxExplicitRenderTriangle& triangle, const physx::ExplicitHierarchicalMesh& hMesh)
{
	if (triangle.extraDataIndex >= hMesh.mMaterialFrames.size())
	{
		return false;
	}

	const physx::NxMaterialFrame& materialFrame = hMesh.mMaterialFrames[triangle.extraDataIndex];

	return materialFrame.mFractureIndex == -1;
}


static void applyNoiseToChunk
(
 physx::ExplicitHierarchicalMesh& hMesh,
 physx::PxU32 parentPartIndex,
 physx::PxU32 partIndex,
 const physx::NxNoiseParameters& noise,
 physx::PxF32 gridScale
 )
{
	if (partIndex >= hMesh.mParts.size() || parentPartIndex >= hMesh.mParts.size())
	{
		return;
	}

	// Mesh and mesh size
	physx::PxF32 level0Size = hMesh.mParts[(physx::PxU32)hMesh.mChunks[0]->mPartIndex]->mBounds.getExtents().magnitude();
	physx::Array<physx::NxExplicitRenderTriangle>& partMesh = hMesh.mParts[partIndex]->mMesh;
	physx::Array<physx::NxExplicitRenderTriangle>& parentPartMesh = hMesh.mParts[parentPartIndex]->mMesh;

	// Grid parameters
	const physx::PxF32 gridSize = physx::PxAbs(gridScale) / physx::PxMax(2, noise.gridSize);
	if (gridSize == 0.0f)
	{
		return;
	}

	const physx::PxF32 tol = 0.0001f*gridSize;

	//	MeshProcessor::removeSlivers(partMesh, 0.5f*tol);

	// Sort triangles based upon whether or not they are part of the active group.
	// Put the active triangles last in the list, so we only need traverse them when splitting
	physx::PxU32 inactiveTriangleCount = 0;
	physx::PxU32 highMark = partMesh.size();
	while (inactiveTriangleCount < highMark)
	{
		if (!triangleIsPartOfActiveSet(partMesh[inactiveTriangleCount], hMesh))
		{
			++inactiveTriangleCount;
		}
		else
			if (triangleIsPartOfActiveSet(partMesh[highMark-1], hMesh))
			{
				--highMark;
			}
			else
			{
				physx::swap(partMesh[inactiveTriangleCount++], partMesh[--highMark]);
			}
	}
	PX_ASSERT(inactiveTriangleCount == highMark);

	MeshProcessor chunkMeshProcessor;
	chunkMeshProcessor.setMesh(partMesh, &parentPartMesh, inactiveTriangleCount, tol);
	chunkMeshProcessor.lockBorderVertices();
	chunkMeshProcessor.removeTJunctions();
	chunkMeshProcessor.subdivide(gridSize);
	chunkMeshProcessor.snapVertices(4*tol);

	// Now create and apply noise field
	const physx::PxU32 rndSeedSave = userRnd.m_rnd.seed();
	userRnd.m_rnd.setSeed(0);
	const physx::PxF32 scaledAmplitude = noise.amplitude*level0Size;
	const physx::PxU32 numModes = 10;
	const physx::PxF32 amplitude = scaledAmplitude / physx::PxSqrt((physx::PxF32)numModes);	// Scale by frequency?
	const physx::PxF32 spatialFrequency = noise.frequency*(physx::PxTwoPi/gridSize);
	physx::PxF32 phase[numModes][3];
	physx::PxVec3 k[numModes][3];
	for (physx::PxU32 n = 0; n < numModes; ++n)
	{
		for (physx::PxU32 i = 0; i < 3; ++i)
		{
			phase[n][i] = userRnd.getReal(-physx::PxPi, physx::PxPi);
			k[n][i] = physx::PxVec3(userRnd.getReal(-1.0f, 1.0f), userRnd.getReal(-1.0f, 1.0f), userRnd.getReal(-1.0f, 1.0f));
			k[n][i].normalize();	// Not a uniform spherical distribution, but it's ok
			k[n][i] *= spatialFrequency;
		}
	}
	userRnd.m_rnd.setSeed(rndSeedSave);

	for (MeshProcessor::FreeVertexIt it(chunkMeshProcessor); it.valid(); it.inc())
	{
		physx::PxVec3& r = partMesh[it.triangleIndex()].vertices[it.vertexIndex()].position;
		physx::PxVec3 field(0.0f);
		physx::PxMat33 gradient(physx::PxVec3(0.0f), physx::PxVec3(0.0f), physx::PxVec3(0.0f));
		for (physx::PxU32 n = 0; n < numModes; ++n)
		{
			for (physx::PxU32 i = 0; i < 3; ++i)
			{
				const physx::PxF32 phi = k[n][i].dot(r) + phase[n][i];
				field[i] += amplitude*physx::PxSin(phi);
				for (physx::PxU32 j = 0; j < 3; ++j)
				{
					gradient(i,j) += amplitude*k[n][i][j]*physx::PxCos(phi);
				}
			}
		}
		r += field.dot(it.vertexRep().normal)*it.vertexRep().normal;
		physx::PxVec3 g = gradient.transformTranspose(it.vertexRep().normal);
		physx::PxVec3& n = partMesh[it.triangleIndex()].vertices[it.vertexIndex()].normal;
		n += g.dot(n)*n - g;
		n.normalize();
		physx::PxVec3& t = partMesh[it.triangleIndex()].vertices[it.vertexIndex()].tangent;
		t -= t.dot(n)*n;
		t.normalize();
		partMesh[it.triangleIndex()].vertices[it.vertexIndex()].binormal = n.cross(t);
	}

	// Fix up any mesh intersections that may have resulted from the application of noise
	chunkMeshProcessor.resolveIntersections();
}


static bool voronoiSplitChunkInternal
(
	physx::ExplicitHierarchicalMesh& hMesh,
	physx::PxU32 chunkIndex,
	const ApexCSG::IApexBSP& chunkBSP,
	const physx::NxFractureVoronoiDesc& desc,
	const NxCollisionDesc& collisionDesc,
	physx::IProgressListener& progressListener,
	volatile bool* cancel
)
{
	bool canceled = false;

	physx::Array<physx::PxVec3> sitesForChunk;
	const physx::PxVec3* sites = desc.sites;
	physx::PxU32 siteCount = desc.siteCount;
	if (desc.chunkIndices != NULL)
	{
		for (physx::PxU32 siteN = 0; siteN < desc.siteCount; ++siteN)
		{
			if (desc.chunkIndices[siteN] == chunkIndex)
			{
				sitesForChunk.pushBack(desc.sites[siteN]);
			}
		}
		siteCount = sitesForChunk.size();
		sites = siteCount > 0 ? &sitesForChunk[0] : NULL;
	}

	if (siteCount < 2)
	{
		return !canceled;	// Don't want to generate a single child which is a duplicate of the parent, when siteCount == 1
	}

	HierarchicalProgressListener progress((physx::PxI32)physx::PxMax(siteCount, 1u), &progressListener);

	const physx::PxF32 minimumRadius2 = hMesh.chunkBounds(0).getExtents().magnitudeSquared()*desc.minimumChunkSize*desc.minimumChunkSize;

	for (VoronoiCellPlaneIterator i(sites, siteCount); i.valid(); i.inc())
	{
		// Create a voronoi cell for this site
		ApexCSG::IApexBSP* cellBSP = createBSP(hMesh.mBSPMemCache, chunkBSP.getInternalTransform());	// BSPs start off representing all space
		ApexCSG::IApexBSP* planeBSP = createBSP(hMesh.mBSPMemCache, chunkBSP.getInternalTransform());
		const physx::PxPlane* planes = i.cellPlanes();
		for (physx::PxU32 planeN = 0; planeN < i.cellPlaneCount(); ++planeN)
		{
			const physx::PxPlane& plane = planes[planeN];

			// Create single-plane slice BSP
			physx::IntersectMesh grid;
			GridParameters gridParameters;
			gridParameters.interiorSubmeshIndex = desc.materialDesc.interiorSubmeshIndex;
			// Defer noise generation if we're using displacement maps
			gridParameters.noise = physx::NxNoiseParameters();
			gridParameters.level0Mesh = &hMesh.mParts[0]->mMesh;
			gridParameters.materialFrameIndex = hMesh.addMaterialFrame();
			physx::NxMaterialFrame materialFrame = hMesh.getMaterialFrame(gridParameters.materialFrameIndex);
			materialFrame.buildCoordinateSystemFromMaterialDesc(desc.materialDesc, plane);
			materialFrame.mFractureMethod = physx::NxFractureMethod::Voronoi;
			materialFrame.mSliceDepth = hMesh.depth(chunkIndex) + 1;
			// Leaving the materialFrame.mFractureMethod at the default of -1, since voronoi cutout faces are not be associated with a direction index
			hMesh.setMaterialFrame(gridParameters.materialFrameIndex, materialFrame);
			gridParameters.triangleFrame.setFlat(materialFrame.mCoordinateSystem, desc.materialDesc.uvScale, desc.materialDesc.uvOffset);
			buildIntersectMesh(grid, plane, materialFrame, (physx::PxI32)desc.noiseMode, &gridParameters);

			ApexCSG::BSPBuildParameters bspBuildParams = gDefaultBuildParameters;
			bspBuildParams.internalTransform = chunkBSP.getInternalTransform();
			bspBuildParams.rnd = &userRnd;

			if(desc.useDisplacementMaps)
			{
				// Displacement map generation is deferred until the end of fracturing
				// This used to be where a slice would populate a displacement map with 
				//  offsets along the plane, but no longer
			}

			planeBSP->fromMesh(&grid.m_triangles[0], grid.m_triangles.size(), bspBuildParams);
			cellBSP->combine(*planeBSP);
			cellBSP->op(*cellBSP, ApexCSG::Operation::Intersection);
		}
		planeBSP->release();

		if (hMesh.mParts[(physx::PxU32)hMesh.mChunks[chunkIndex]->mPartIndex]->mFlags & ExplicitHierarchicalMesh::Part::MeshOpen)
		{
			cellBSP->deleteTriangles();	// Don't use interior triangles on an open mesh
		}

		ApexCSG::IApexBSP* bsp = createBSP(hMesh.mBSPMemCache);
		bsp->copy(*cellBSP);
		bsp->combine(chunkBSP);
		bsp->op(*bsp, ApexCSG::Operation::Intersection);
		cellBSP->release();

		if (gIslandGeneration)
		{
			bsp = bsp->decomposeIntoIslands();
		}

		while (bsp != NULL)
		{
			if (cancel != NULL && *cancel)
			{
				canceled = true;
			}

			if (!canceled)
			{
				// Create a mesh with chunkBSP (or its islands)
				const physx::PxU32 newPartIndex = hMesh.addPart();
				const physx::PxU32 newChunkIndex = hMesh.addChunk();
				bsp->toMesh(hMesh.mParts[newPartIndex]->mMesh);
				hMesh.mParts[newPartIndex]->mMeshBSP->copy(*bsp);
				hMesh.buildMeshBounds(newPartIndex);
				hMesh.buildCollisionGeometryForPart(newPartIndex, getVolumeDesc(collisionDesc, hMesh.depth(chunkIndex)+1));
				hMesh.mChunks[newChunkIndex]->mParentIndex = (physx::PxI32)chunkIndex;
				hMesh.mChunks[newChunkIndex]->mPartIndex = (physx::PxI32)newPartIndex;
				if (hMesh.mParts[(physx::PxU32)hMesh.mChunks[chunkIndex]->mPartIndex]->mFlags & ExplicitHierarchicalMesh::Part::MeshOpen)
				{
					hMesh.mParts[newPartIndex]->mFlags |= ExplicitHierarchicalMesh::Part::MeshOpen;
				}
				if (hMesh.mParts[newPartIndex]->mMesh.size() == 0 || hMesh.mParts[newPartIndex]->mCollision.size() == 0 ||
					hMesh.chunkBounds(newChunkIndex).getExtents().magnitudeSquared() < minimumRadius2)
				{
					// Either no mesh, no collision, or too small.  Eliminate.
					hMesh.removeChunk(newChunkIndex);
					hMesh.removePart(newPartIndex);
				}
				else
				{
					// Apply graphical noise to new part, if requested
					if (desc.faceNoise.amplitude > 0.0f){
						const physx::PxU32 parentPartIndex = (physx::PxU32)*hMesh.partIndex(chunkIndex);
						applyNoiseToChunk(hMesh, parentPartIndex, newPartIndex, desc.faceNoise, hMesh.meshBounds(newPartIndex).getExtents().magnitude());
					}
				}
			}
			// Get next bsp in island decomposition
			ApexCSG::IApexBSP* nextBSP = bsp->getNext();
			bsp->release();
			bsp = nextBSP;
		}

		progress.completeSubtask();
	}

	return !canceled;
}

namespace FractureTools
{

void setBSPTolerances
(
	physx::PxF32 linearTolerance,
	physx::PxF32 angularTolerance,
	physx::PxF32 baseTolerance,
	physx::PxF32 clipTolerance,
	physx::PxF32 cleaningTolerance
)
{
	ApexCSG::gDefaultTolerances.linear = linearTolerance;
	ApexCSG::gDefaultTolerances.angular = angularTolerance;
	ApexCSG::gDefaultTolerances.base = baseTolerance;
	ApexCSG::gDefaultTolerances.clip = clipTolerance;
	ApexCSG::gDefaultTolerances.cleaning = cleaningTolerance;
}

void setBSPBuildParameters
(
	physx::PxF32 logAreaSigmaThreshold,
	physx::PxU32 testSetSize,
	physx::PxF32 splitWeight,
	physx::PxF32 imbalanceWeight
)
{
	gDefaultBuildParameters.logAreaSigmaThreshold = logAreaSigmaThreshold;
	gDefaultBuildParameters.testSetSize = testSetSize;
	gDefaultBuildParameters.splitWeight = splitWeight;
	gDefaultBuildParameters.imbalanceWeight = imbalanceWeight;
}

static void trimChunkHulls(physx::ExplicitHierarchicalMesh& hMesh, physx::PxU32* chunkIndexArray, physx::PxU32 chunkIndexArraySize, physx::PxF32 maxTrimFraction)
{
	// Outer array is indexed by chunk #, and is of size chunkIndexArraySize
	// Middle array is indexed by hull # for chunkIndexArray[chunk #], is of the same size as the part mCollision array associated with the chunk
	// Inner array is a list of trim planes to be applied to each hull
	physx::Array< physx::Array< physx::Array<physx::PxPlane> > > chunkHullTrimPlanes;

	// Initialize arrays
	chunkHullTrimPlanes.resize(chunkIndexArraySize);
	for (physx::PxU32 chunkNum = 0; chunkNum < chunkIndexArraySize; ++chunkNum)
	{
		physx::Array< physx::Array<physx::PxPlane> >& hullTrimPlanes = chunkHullTrimPlanes[chunkNum];
		const physx::PxU32 chunkIndex = chunkIndexArray[chunkNum];
		const physx::PxU32 partIndex = (physx::PxU32)hMesh.mChunks[chunkIndex]->mPartIndex;
		const physx::PxU32 hullCount = hMesh.mParts[partIndex]->mCollision.size();
		hullTrimPlanes.resize(hullCount);
	}

	const physx::PxVec3 identityScale(1.0f);

	// Compare each chunk's hulls against other chunk hulls, building up list of trim planes.  O(N^2), but so far this is only used for multi-fbx level 1 chunks, so N shouldn't be too large.
	for (physx::PxU32 chunkNum0 = 0; chunkNum0 < chunkIndexArraySize; ++chunkNum0)
	{
		const physx::PxU32 chunkIndex0 = chunkIndexArray[chunkNum0];
		const physx::PxU32 partIndex0 = (physx::PxU32)hMesh.mChunks[chunkIndex0]->mPartIndex;
		const physx::PxMat44 tm0(physx::PxMat33::createIdentity(), hMesh.mChunks[chunkIndex0]->mInstancedPositionOffset);
		const physx::PxU32 hullCount0 = hMesh.mParts[partIndex0]->mCollision.size();
		physx::Array< physx::Array<physx::PxPlane> >& hullTrimPlanes0 = chunkHullTrimPlanes[chunkNum0];
		for (physx::PxU32 hullIndex0 = 0; hullIndex0 < hullCount0; ++hullIndex0)
		{
			physx::PartConvexHullProxy* hull0 = hMesh.mParts[partIndex0]->mCollision[hullIndex0];
			physx::Array<physx::PxPlane>& trimPlanes0 = hullTrimPlanes0[hullIndex0];

			// Inner set of loops for other chunks
			for (physx::PxU32 chunkNum1 = chunkNum0+1; chunkNum1 < chunkIndexArraySize; ++chunkNum1)
			{
				const physx::PxU32 chunkIndex1 = chunkIndexArray[chunkNum1];
				const physx::PxU32 partIndex1 = (physx::PxU32)hMesh.mChunks[chunkIndex1]->mPartIndex;
				const physx::PxMat44 tm1(physx::PxMat33::createIdentity(), hMesh.mChunks[chunkIndex1]->mInstancedPositionOffset);
				const physx::PxU32 hullCount1 = hMesh.mParts[partIndex1]->mCollision.size();
				physx::Array< physx::Array<physx::PxPlane> >& hullTrimPlanes1 = chunkHullTrimPlanes[chunkNum1];
				for (physx::PxU32 hullIndex1 = 0; hullIndex1 < hullCount1; ++hullIndex1)
				{
					physx::PartConvexHullProxy* hull1 = hMesh.mParts[partIndex1]->mCollision[hullIndex1];
					physx::Array<physx::PxPlane>& trimPlanes1 = hullTrimPlanes1[hullIndex1];

					// Test overlap
					ConvexHull::Separation separation;
					if (ConvexHull::hullsInProximity(hull0->impl, tm0, identityScale, hull1->impl, tm1, identityScale, 0.0f, &separation))
					{
						// Add trim planes if there's an overlap
						physx::PxPlane& trimPlane0 = trimPlanes0.insert();
						trimPlane0 = separation.plane;
						trimPlane0.d = physx::PxMin(trimPlane0.d, maxTrimFraction*(separation.max0-separation.min0) - separation.max0);	// Bound clip distance
						trimPlane0.d += trimPlane0.n.dot(tm0.getPosition());	// Transform back into part local space
						physx::PxPlane& trimPlane1 = trimPlanes1.insert();
						trimPlane1 = physx::PxPlane(-separation.plane.n, -separation.plane.d);
						trimPlane1.d = physx::PxMin(trimPlane1.d, maxTrimFraction*(separation.max1-separation.min1) + separation.min1);	// Bound clip distance
						trimPlane1.d += trimPlane1.n.dot(tm1.getPosition());	// Transform back into part local space
					}
				}
			}
		}
	}

	// Now traverse trim plane list and apply it to the chunks's hulls
	for (physx::PxU32 chunkNum = 0; chunkNum < chunkIndexArraySize; ++chunkNum)
	{
		const physx::PxU32 chunkIndex = chunkIndexArray[chunkNum];
		const physx::PxU32 partIndex = (physx::PxU32)hMesh.mChunks[chunkIndex]->mPartIndex;
		const physx::PxU32 hullCount = hMesh.mParts[partIndex]->mCollision.size();
		physx::Array< physx::Array<physx::PxPlane> >& hullTrimPlanes = chunkHullTrimPlanes[chunkNum];
		for (physx::PxU32 hullIndex = hullCount; hullIndex--;)	// Traverse backwards, in case we need to delete empty hulls
		{
			physx::PartConvexHullProxy* hull = hMesh.mParts[partIndex]->mCollision[hullIndex];
			physx::Array<physx::PxPlane>& trimPlanes = hullTrimPlanes[hullIndex];
			for (physx::PxU32 planeIndex = 0; planeIndex < trimPlanes.size(); ++planeIndex)
			{
				hull->impl.intersectPlaneSide(trimPlanes[planeIndex]);
				if (hull->impl.isEmpty())
				{
					PX_DELETE(hMesh.mParts[partIndex]->mCollision[hullIndex]);
					hMesh.mParts[partIndex]->mCollision.replaceWithLast(hullIndex);
					break;
				}
			}
		}
		// Make sure we haven't deleted every collision hull
		if (hMesh.mParts[partIndex]->mCollision.size() == 0)
		{
			physx::NxCollisionVolumeDesc collisionVolumeDesc;
			collisionVolumeDesc.mHullMethod = NxConvexHullMethod::WRAP_GRAPHICS_MESH;	// Should we use something simpler, like a box?
			hMesh.buildCollisionGeometryForPart(partIndex, collisionVolumeDesc);
		}
	}
}

bool buildExplicitHierarchicalMesh
(
    physx::NxExplicitHierarchicalMesh& iHMesh,
    const physx::NxExplicitRenderTriangle* meshTriangles,
    physx::PxU32 meshTriangleCount,
    const physx::NxExplicitSubmeshData* submeshData,
    physx::PxU32 submeshCount,
    physx::PxU32* meshPartition,
    physx::PxU32 meshPartitionCount,
	physx::PxI32* parentIndices,
	physx::PxU32 parentIndexCount
)
{
	bool flatDepthOne = parentIndexCount == 0;

	const bool havePartition = meshPartition != NULL && meshPartitionCount > 1;

	if (!havePartition)
	{
		flatDepthOne = true;	// This only makes sense if we have a partition
	}

	if (parentIndices == NULL)
	{
		parentIndexCount = 0;
	}

	physx::ExplicitHierarchicalMesh& hMesh = *(physx::ExplicitHierarchicalMesh*)&iHMesh;
	hMesh.clear();
	hMesh.addPart();
	hMesh.mParts[0]->mMesh.reset();
	const physx::PxU32 part0Size = !flatDepthOne ? meshPartition[0] : meshTriangleCount;	// Build level 0 part out of all of the triangles if flatDepthOne = true
	hMesh.mParts[0]->mMesh.reserve(part0Size);
	physx::PxU32 nextTriangle = 0;
	for (physx::PxU32 i = 0; i < part0Size; ++i)
	{
		hMesh.mParts[0]->mMesh.pushBack(meshTriangles[nextTriangle++]);
	}
	hMesh.buildMeshBounds(0);
	hMesh.addChunk();
	hMesh.mChunks[0]->mParentIndex = -1;
	hMesh.mChunks[0]->mPartIndex = 0;

	if (flatDepthOne)
	{
		nextTriangle = 0;	// reset
	}

	physx::Array<bool> hasChildren(meshPartitionCount+1, false);

	if (havePartition)
	{
		// We have a partition - build hierarchy
		physx::PxU32 partIndex = 1;
		const physx::PxU32 firstLevel1Part = !flatDepthOne ? 1u : 0u;
		for (physx::PxU32 i = firstLevel1Part; i < meshPartitionCount; ++i, ++partIndex)
		{
			hMesh.addPart();
			hMesh.mParts[partIndex]->mMesh.reset();
			hMesh.mParts[partIndex]->mMesh.reserve(meshPartition[i] - nextTriangle);
			while (nextTriangle < meshPartition[i])
			{
				hMesh.mParts[partIndex]->mMesh.pushBack(meshTriangles[nextTriangle++]);
			}
			hMesh.buildMeshBounds(partIndex);
			hMesh.addChunk();
			hMesh.mChunks[partIndex]->mParentIndex = partIndex < parentIndexCount ? parentIndices[partIndex] : 0;	// partIndex = chunkIndex here
			if (hMesh.mChunks[partIndex]->mParentIndex >= 0)
			{
				hasChildren[(physx::PxU32)hMesh.mChunks[partIndex]->mParentIndex] = true;
			}
			hMesh.mChunks[partIndex]->mPartIndex = (physx::PxI32)partIndex;	// partIndex = chunkIndex here
		}
	}

	// Submesh data
	hMesh.mSubmeshData.reset();
	hMesh.mSubmeshData.reserve(submeshCount);
	for (physx::PxU32 i = 0; i < submeshCount; ++i)
	{
		hMesh.mSubmeshData.pushBack(submeshData[i]);
	}

	for (physx::PxU32 i = 0; i < hMesh.mChunks.size(); ++i)
	{
		hMesh.mChunks[i]->mPrivateFlags |= ExplicitHierarchicalMesh::Chunk::Root;
		if (!hasChildren[i])
		{
			hMesh.mChunks[i]->mPrivateFlags |= ExplicitHierarchicalMesh::Chunk::RootLeaf;
		}
	}

	hMesh.mRootSubmeshCount = submeshCount;

	hMesh.sortChunks();

	return true;
}

// If destructibleAsset == NULL, no hierarchy is assumed and we must have only one part in the render mesh.
static bool buildExplicitHierarchicalMeshFromApexAssetsInternal(physx::ExplicitHierarchicalMesh& hMesh, const physx::apex::NxRenderMeshAsset& renderMeshAsset,
																const physx::apex::NxDestructibleAsset* destructibleAsset, physx::PxU32 maxRootDepth = PX_MAX_U32)
{
	if (renderMeshAsset.getPartCount() == 0)
	{
		return false;
	}

	if (destructibleAsset == NULL && renderMeshAsset.getPartCount() != 1)
	{
		return false;
	}

	hMesh.clear();

	// Create parts
	for (physx::PxU32 partIndex = 0; partIndex < renderMeshAsset.getPartCount(); ++partIndex)
	{
		const physx::PxU32 newPartIndex = hMesh.addPart();
		PX_ASSERT(newPartIndex == partIndex);
		ExplicitHierarchicalMesh::Part* part = hMesh.mParts[newPartIndex];
		// Fill in fields except for mesh (will be done in submesh loop below)
		// part->mMeshBSP is NULL, that's OK
		part->mBounds = renderMeshAsset.getBounds(partIndex);
		if (destructibleAsset != NULL)
		{
			// Get collision data from destructible asset
			part->mCollision.reserve(destructibleAsset->getPartConvexHullCount(partIndex));
			for (physx::PxU32 hullIndex = 0; hullIndex < destructibleAsset->getPartConvexHullCount(partIndex); ++hullIndex)
			{
				NxParameterized::Interface* hullParams = destructibleAsset->getPartConvexHullArray(partIndex)[hullIndex];
				if (hullParams != NULL)
				{
					physx::PartConvexHullProxy* newHull = PX_NEW(physx::PartConvexHullProxy)();
					part->mCollision.pushBack(newHull);
					newHull->impl.mParams->copy(*hullParams);
				}
			}
		}
	}

	// Deduce root and interior submesh info
	hMesh.mRootSubmeshCount = 0;	// Incremented below

	// Fill in mesh and get submesh data
	hMesh.mSubmeshData.reset();
	hMesh.mSubmeshData.reserve(renderMeshAsset.getSubmeshCount());
	for (physx::PxU32 submeshIndex = 0; submeshIndex < renderMeshAsset.getSubmeshCount(); ++submeshIndex)
	{
		const physx::NxRenderSubmesh& submesh = renderMeshAsset.getSubmesh(submeshIndex);

		// Submesh data
		physx::NxExplicitSubmeshData& submeshData = hMesh.mSubmeshData.pushBack(physx::NxExplicitSubmeshData());
		physx::string::strcpy_s(submeshData.mMaterialName, physx::NxExplicitSubmeshData::MaterialNameBufferSize, renderMeshAsset.getMaterialName(submeshIndex));
		submeshData.mVertexFormat.mBonesPerVertex = 1;

		// Mesh
		const physx::NxVertexBuffer& vb = submesh.getVertexBuffer();
		const physx::NxVertexFormat& vbFormat = vb.getFormat();
		const physx::PxU32 submeshVertexCount = vb.getVertexCount();
		if (submeshVertexCount == 0)
		{
			continue;
		}

		// Get vb data:
		physx::Array<physx::PxVec3> positions;
		physx::Array<physx::PxVec3> normals;
		physx::Array<physx::PxVec4> tangents;	// Handle vec4 tangents
		physx::Array<physx::PxVec3> binormals;
		physx::Array<physx::PxColorRGBA> colors;
		physx::Array<physx::NxVertexUV> uvs[NxVertexFormat::MAX_UV_COUNT];

		// Positions
		const physx::PxI32 positionBufferIndex = vbFormat.getBufferIndexFromID(vbFormat.getSemanticID(NxRenderVertexSemantic::POSITION));
		positions.resize(submeshVertexCount);
		submeshData.mVertexFormat.mHasStaticPositions = vb.getBufferData(&positions[0], physx::NxRenderDataFormat::FLOAT3, sizeof(physx::PxVec3), 
																		(physx::PxU32)positionBufferIndex, 0, submeshVertexCount);
		if (!submeshData.mVertexFormat.mHasStaticPositions)
		{
			return false;	// Need a position buffer!
		}

		// Normals
		const physx::PxI32 normalBufferIndex = vbFormat.getBufferIndexFromID(vbFormat.getSemanticID(NxRenderVertexSemantic::NORMAL));
		normals.resize(submeshVertexCount);
		submeshData.mVertexFormat.mHasStaticNormals = vb.getBufferData(&normals[0], physx::NxRenderDataFormat::FLOAT3, sizeof(physx::PxVec3), 
																		(physx::PxU32)normalBufferIndex, 0, submeshVertexCount);
		if (!submeshData.mVertexFormat.mHasStaticNormals)
		{
			::memset(&normals[0], 0, submeshVertexCount*sizeof(physx::PxVec3));	// Fill with zeros
		}

		// Tangents
		const physx::PxI32 tangentBufferIndex = vbFormat.getBufferIndexFromID(vbFormat.getSemanticID(NxRenderVertexSemantic::TANGENT));
		tangents.resize(submeshVertexCount, physx::PxVec4(physx::PxVec3(0.0f), 1.0f));	// Fill with (0,0,0,1), in case we read 3-component tangents
		switch (vbFormat.getBufferFormat((physx::PxU32)tangentBufferIndex))
		{
		case physx::NxRenderDataFormat::BYTE_SNORM3:
		case physx::NxRenderDataFormat::SHORT_SNORM3:
		case physx::NxRenderDataFormat::FLOAT3:
			submeshData.mVertexFormat.mHasStaticTangents = vb.getBufferData(&tangents[0], physx::NxRenderDataFormat::FLOAT3, sizeof(physx::PxVec4), (physx::PxU32)tangentBufferIndex, 0, submeshVertexCount);
			break;
		case physx::NxRenderDataFormat::BYTE_SNORM4:
		case physx::NxRenderDataFormat::SHORT_SNORM4:
		case physx::NxRenderDataFormat::FLOAT4:
			submeshData.mVertexFormat.mHasStaticTangents = vb.getBufferData(&tangents[0], physx::NxRenderDataFormat::FLOAT4, sizeof(physx::PxVec4), (physx::PxU32)tangentBufferIndex, 0, submeshVertexCount);
			break;
		default:
			submeshData.mVertexFormat.mHasStaticTangents = false;
			break;
		}

		// Binormals
		const physx::PxI32 binormalBufferIndex = vbFormat.getBufferIndexFromID(vbFormat.getSemanticID(NxRenderVertexSemantic::BINORMAL));
		binormals.resize(submeshVertexCount);
		submeshData.mVertexFormat.mHasStaticBinormals = vb.getBufferData(&binormals[0], physx::NxRenderDataFormat::FLOAT3, sizeof(physx::PxVec3), 
																		(physx::PxU32)binormalBufferIndex, 0, submeshVertexCount);
		if (!submeshData.mVertexFormat.mHasStaticBinormals)
		{
			submeshData.mVertexFormat.mHasStaticBinormals = submeshData.mVertexFormat.mHasStaticNormals && submeshData.mVertexFormat.mHasStaticTangents;
			for (physx::PxU32 i = 0; i < submeshVertexCount; ++i)
			{
				binormals[i] = physx::PxSign(tangents[i][3])*normals[i].cross(tangents[i].getXYZ());	// Build from normals and tangents.  If one of these doesn't exist we'll get (0,0,0)'s
			}
		}

		// Colors
		const physx::PxI32 colorBufferIndex = vbFormat.getBufferIndexFromID(vbFormat.getSemanticID(NxRenderVertexSemantic::COLOR));
		colors.resize(submeshVertexCount);
		submeshData.mVertexFormat.mHasStaticColors = vb.getBufferData(&colors[0], physx::NxRenderDataFormat::B8G8R8A8, sizeof(physx::PxColorRGBA), 
																		(physx::PxU32)colorBufferIndex, 0, submeshVertexCount);
		if (!submeshData.mVertexFormat.mHasStaticColors)
		{
			::memset(&colors[0], 0xFF, submeshVertexCount*sizeof(physx::PxColorRGBA));	// Fill with 0xFF
		}

		// UVs
		submeshData.mVertexFormat.mUVCount = 0;
		physx::PxU32 uvNum = 0;
		for (; uvNum < NxVertexFormat::MAX_UV_COUNT; ++uvNum)
		{
			uvs[uvNum].resize(submeshVertexCount);
			const PxI32 uvBufferIndex = vbFormat.getBufferIndexFromID(vbFormat.getSemanticID((NxRenderVertexSemantic::Enum)(NxRenderVertexSemantic::TEXCOORD0 + uvNum)));
			if (!vb.getBufferData(&uvs[uvNum][0], physx::NxRenderDataFormat::FLOAT2, sizeof(physx::NxVertexUV), 
																		(physx::PxU32)uvBufferIndex, 0, submeshVertexCount))
			{
				break;
			}
		}
		submeshData.mVertexFormat.mUVCount = uvNum;
		for (; uvNum < NxVertexFormat::MAX_UV_COUNT; ++uvNum)
		{
			uvs[uvNum].resize(submeshVertexCount);
			::memset(&uvs[uvNum][0], 0, submeshVertexCount*sizeof(physx::NxVertexUV));	// Fill with zeros
		}

		// Now create triangles
		bool rootChunkHasTrianglesWithThisSubmesh = false;
		for (physx::PxU32 partIndex = 0; partIndex < renderMeshAsset.getPartCount(); ++partIndex)
		{
			ExplicitHierarchicalMesh::Part* part = hMesh.mParts[partIndex];
			physx::Array<physx::NxExplicitRenderTriangle>& triangles = part->mMesh;
			const physx::PxU32* indexBuffer = submesh.getIndexBuffer(partIndex);
			const physx::PxU32* smoothingGroups = submesh.getSmoothingGroups(partIndex);
			const physx::PxU32 indexCount = submesh.getIndexCount(partIndex);
			PX_ASSERT((indexCount%3) == 0);
			const physx::PxU32 triangleCount = indexCount/3;
			triangles.reserve(triangles.size() + triangleCount);
			if (triangleCount > 0 && destructibleAsset != NULL)
			{
				for (physx::PxU32 chunkIndex = 0; chunkIndex < destructibleAsset->getChunkCount(); ++chunkIndex)
				{
					if (destructibleAsset->getPartIndex(chunkIndex) == partIndex)
					{
						// This part is in a root chunk.  Make sure we've accounted for all of its submeshes
						rootChunkHasTrianglesWithThisSubmesh = true;
						break;
					}
				}
			}
			for (physx::PxU32 triangleNum = 0; triangleNum < triangleCount; ++triangleNum)
			{
				physx::NxExplicitRenderTriangle& triangle = triangles.pushBack(physx::NxExplicitRenderTriangle());
				triangle.extraDataIndex = 0xFFFFFFFF;
				triangle.smoothingMask = smoothingGroups != NULL ? smoothingGroups[triangleNum] : 0;
				triangle.submeshIndex = (physx::PxI32)submeshIndex;
				for (unsigned v = 0; v < 3; ++v)
				{
					const physx::PxU32 index = *indexBuffer++;
					physx::NxVertex& vertex = triangle.vertices[v];
					vertex.position = positions[index];
					vertex.normal = normals[index];
					vertex.tangent = tangents[index].getXYZ();
					vertex.binormal = binormals[index];
					vertex.color = NxVertexColor(PxColorRGBA(colors[index]));
					for (physx::PxU32 uvNum = 0; uvNum < NxVertexFormat::MAX_UV_COUNT; ++uvNum)
					{
						vertex.uv[uvNum] = uvs[uvNum][index];
					}
					vertex.boneIndices[0] = (physx::PxU16)partIndex;
				}
			}
		}

		if (rootChunkHasTrianglesWithThisSubmesh)
		{
			hMesh.mRootSubmeshCount = submeshIndex+1;
		}
	}

	// Create chunks
	if (destructibleAsset != NULL)
	{
		physx::Array<bool> hasRootChildren(destructibleAsset->getChunkCount(), false);
		for (physx::PxU32 chunkIndex = 0; chunkIndex < destructibleAsset->getChunkCount(); ++chunkIndex)
		{
			const physx::PxU32 newChunkIndex = hMesh.addChunk();
			PX_ASSERT(newChunkIndex == chunkIndex);
			ExplicitHierarchicalMesh::Chunk* chunk = hMesh.mChunks[newChunkIndex];
			// Fill in fields of chunk
			chunk->mParentIndex = destructibleAsset->getChunkParentIndex(chunkIndex);
			chunk->mFlags = destructibleAsset->getChunkFlags(chunkIndex);
			chunk->mPartIndex = (physx::PxI32)destructibleAsset->getPartIndex(chunkIndex);
			chunk->mInstancedPositionOffset = destructibleAsset->getChunkPositionOffset(chunkIndex);
			chunk->mInstancedUVOffset = destructibleAsset->getChunkUVOffset(chunkIndex);
			if (destructibleAsset->getChunkDepth(chunkIndex) <= maxRootDepth)
			{
				chunk->mPrivateFlags |= ExplicitHierarchicalMesh::Chunk::Root;	// We will assume every chunk is a root chunk
				if (chunk->mParentIndex >= 0 && chunk->mParentIndex < (physx::PxI32)destructibleAsset->getChunkCount())
				{
					hasRootChildren[(physx::PxU32)chunk->mParentIndex] = true;
				}
			}
		}

		// See which root chunks have no children; these are root leaves
		for (physx::PxU32 chunkIndex = 0; chunkIndex < destructibleAsset->getChunkCount(); ++chunkIndex)
		{
			ExplicitHierarchicalMesh::Chunk* chunk = hMesh.mChunks[chunkIndex];
			if (chunk->isRootChunk() && !hasRootChildren[chunkIndex])
			{
				chunk->mPrivateFlags |= ExplicitHierarchicalMesh::Chunk::RootLeaf;
			}
		}
	}
	else
	{
		// No destructible asset, there's just one chunk
		const physx::PxU32 newChunkIndex = hMesh.addChunk();
		PX_ASSERT(newChunkIndex == 0);
		ExplicitHierarchicalMesh::Chunk* chunk = hMesh.mChunks[newChunkIndex];
		// Fill in fields of chunk
		chunk->mParentIndex = -1;
		chunk->mFlags = 0;	// Can't retrieve this
		chunk->mPartIndex = 0;
		chunk->mInstancedPositionOffset = physx::PxVec3(0.0f);
		chunk->mInstancedUVOffset = physx::PxVec2(0.0f);
		chunk->mPrivateFlags |= (ExplicitHierarchicalMesh::Chunk::Root | ExplicitHierarchicalMesh::Chunk::RootLeaf);
	}

	return true;
}

PX_INLINE bool trianglesTouch(const physx::NxExplicitRenderTriangle& t1, const physx::NxExplicitRenderTriangle& t2)
{
	PX_UNUSED(t1);
	PX_UNUSED(t2);
	return true;	// For now, just keep AABB test.  May want to do better.
}

static void partitionMesh(physx::Array<physx::PxU32>& partition, physx::NxExplicitRenderTriangle* mesh, physx::PxU32 meshTriangleCount, physx::PxF32 padding)
{
	// Find triangle neighbors
	physx::Array<physx::BoundsRep> triangleBounds;
	triangleBounds.reserve(meshTriangleCount);
	for (physx::PxU32 i = 0; i < meshTriangleCount; ++i)
	{
		physx::NxExplicitRenderTriangle& triangle = mesh[i];
		physx::BoundsRep& rep = triangleBounds.insert();
		for (int j = 0; j < 3; ++j)
		{
			rep.aabb.include(triangle.vertices[j].position);
		}
		rep.aabb.fattenFast(padding);
	}

	NeighborLookup triangleNeighborhoods;
	triangleNeighborhoods.setBounds(&triangleBounds[0], triangleBounds.size(), sizeof(triangleBounds[0]));

	// Re-ordering the mesh in-place will make the neighborhoods invalid, so we re-map
	physx::Array<physx::PxU32> triangleRemap(meshTriangleCount);
	physx::Array<physx::PxU32> triangleRemapInv(meshTriangleCount);
	for (physx::PxU32 i = 0; i < meshTriangleCount; ++i)
	{
		triangleRemap[i] = i;
		triangleRemapInv[i] = i;
	}

	partition.resize(0);
	physx::PxU32 nextTriangle = 0;
	while (nextTriangle < meshTriangleCount)
	{
		physx::PxU32 partitionStop = nextTriangle+1;
		do
		{
			const physx::PxU32 r = triangleRemap[nextTriangle];
			const physx::PxU32 neighborCount = triangleNeighborhoods.getNeighborCount(r);
			const physx::PxU32* neighbors = triangleNeighborhoods.getNeighbors(r);
			for (physx::PxU32 n = 0; n < neighborCount; ++n)
			{
				const physx::PxU32 s = triangleRemapInv[neighbors[n]];
				if (s <= partitionStop || !trianglesTouch(mesh[nextTriangle], mesh[s]))
				{
					continue;
				}
				physx::swap(triangleRemapInv[triangleRemap[partitionStop]], triangleRemapInv[triangleRemap[s]]);
				physx::swap(triangleRemap[partitionStop], triangleRemap[s]);
				physx::swap(mesh[partitionStop], mesh[s]);
				++partitionStop;
			}
		} while(nextTriangle++ < partitionStop);
		partition.pushBack(nextTriangle);
	}
}

physx::PxU32 partitionMeshByIslands
(
	physx::NxExplicitRenderTriangle* mesh,
	physx::PxU32 meshTriangleCount,
    physx::PxU32* meshPartition,
    physx::PxU32 meshPartitionMaxCount,
	physx::PxF32 padding
)
{
	// Adjust padding for mesh size
	physx::PxBounds3 bounds;
	bounds.setEmpty();
	for (physx::PxU32 i = 0; i < meshTriangleCount; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			bounds.include(mesh[i].vertices[j].position);
		}
	}
	padding *= bounds.getExtents().magnitude();

	physx::Array<physx::PxU32> partition;
	partitionMesh(partition, mesh, meshTriangleCount, padding);

	for (physx::PxU32 i = 0; i < meshPartitionMaxCount && i < partition.size(); ++i)
	{
		meshPartition[i] = partition[i];
	}

	return partition.size();
}

bool buildExplicitHierarchicalMeshFromRenderMeshAsset(physx::NxExplicitHierarchicalMesh& iHMesh, const physx::apex::NxRenderMeshAsset& renderMeshAsset, physx::PxU32 maxRootDepth)
{
	return buildExplicitHierarchicalMeshFromApexAssetsInternal(*(physx::ExplicitHierarchicalMesh*)&iHMesh, renderMeshAsset, NULL, maxRootDepth);
}

bool buildExplicitHierarchicalMeshFromDestructibleAsset(physx::NxExplicitHierarchicalMesh& iHMesh, const physx::apex::NxDestructibleAsset& destructibleAsset, physx::PxU32 maxRootDepth)
{
	if (destructibleAsset.getChunkCount() == 0)
	{
		return false;
	}

	const physx::NxRenderMeshAsset* renderMeshAsset = destructibleAsset.getRenderMeshAsset();
	if (renderMeshAsset == NULL)
	{
		return false;
	}

	return buildExplicitHierarchicalMeshFromApexAssetsInternal(*(physx::ExplicitHierarchicalMesh*)&iHMesh, *renderMeshAsset, &destructibleAsset, maxRootDepth);
}


class MeshSplitter
{
public:
	virtual bool validate(physx::ExplicitHierarchicalMesh& hMesh) = 0;

	virtual void initialize(physx::ExplicitHierarchicalMesh& hMesh) = 0;

	virtual bool process
	(
		physx::ExplicitHierarchicalMesh& hMesh,
		physx::PxU32 chunkIndex,
		const ApexCSG::IApexBSP& chunkBSP,
		const NxCollisionDesc& collisionDesc,
		physx::IProgressListener& progressListener,
		volatile bool* cancel
	) = 0;

	virtual bool finalize(physx::ExplicitHierarchicalMesh& hMesh) = 0;
};

static bool splitMeshInternal
(
	physx::NxExplicitHierarchicalMesh& iHMesh,
	physx::NxExplicitHierarchicalMesh& iHMeshCore,
	bool exportCoreMesh,
	physx::PxI32 coreMeshImprintSubmeshIndex,
	const NxMeshProcessingParameters& meshProcessingParams,
	MeshSplitter& splitter,
	const NxCollisionDesc& collisionDesc,
	physx::PxU32 randomSeed,
	physx::IProgressListener& progressListener,
	volatile bool* cancel
)
{
	physx::ExplicitHierarchicalMesh& hMesh = *(physx::ExplicitHierarchicalMesh*)&iHMesh;
	physx::ExplicitHierarchicalMesh& hMeshCore = *(physx::ExplicitHierarchicalMesh*)&iHMeshCore;

	if (hMesh.partCount() == 0)
	{
		return false;
	}

	bool rootDepthIsZero = hMesh.mChunks[0]->isRootChunk();	// Until proven otherwise
	for (physx::PxU32 chunkIndex = 1; rootDepthIsZero && chunkIndex < hMesh.chunkCount(); ++chunkIndex)
	{
		rootDepthIsZero = !hMesh.mChunks[chunkIndex]->isRootChunk();
	}

	if (!rootDepthIsZero && hMeshCore.partCount() > 0 && exportCoreMesh)
	{
		char message[1000];
		sprintf(message, "Warning: cannot export core mesh with multiple-mesh root mesh.  Will not export core.");
		outputMessage(message, physx::PxErrorCode::eDEBUG_WARNING);
		exportCoreMesh = false;
	}

	if (!splitter.validate(hMesh))
	{
		return false;
	}

	// Save state if cancel != NULL
	physx::PxFileBuf* save = NULL;
	class NullEmbedding : public physx::NxExplicitHierarchicalMesh::NxEmbedding
	{
		void	serialize(physx::PxFileBuf& stream, NxEmbedding::DataType type) const
		{
			(void)stream;
			(void)type;
		}
		void	deserialize(physx::PxFileBuf& stream, NxEmbedding::DataType type, physx::PxU32 version)
		{
			(void)stream;
			(void)type;
			(void)version;
		}
	} embedding;
	if (cancel != NULL)
	{
		save = physx::NxGetApexSDK()->createMemoryWriteStream();
		if (save != NULL)
		{
			hMesh.serialize(*save, embedding);			
		}
	}
	bool canceled = false;

	hMesh.buildCollisionGeometryForPart(0, getVolumeDesc(collisionDesc, 0));

	userRnd.m_rnd.setSeed(randomSeed);

	// Call initialization callback
	splitter.initialize(hMesh);

	// Make sure we've got BSPs at root depth
	for (physx::PxU32 i = 0; i < hMesh.chunkCount(); ++i)
	{
		if (!hMesh.mChunks[i]->isRootLeafChunk())
		{
			continue;
		}
		physx::PxU32 partIndex = (physx::PxU32)*hMesh.partIndex(i);
		if (hMesh.mParts[partIndex]->mMeshBSP->getType() != ApexCSG::BSPType::Nontrivial)
		{
			outputMessage("Building mesh BSP...");
			progressListener.setProgress(0);
			if (hMesh.calculatePartBSP(partIndex, randomSeed, meshProcessingParams.microgridSize, meshProcessingParams.meshMode, &progressListener, cancel))
			{
				outputMessage("Mesh BSP completed.");
			}
			else
			{
				outputMessage("Mesh BSP failed.");
				canceled = true;
			}
			userRnd.m_rnd.setSeed(randomSeed);
		}
	}

#if 0	// Debugging aid - uses BSP mesh generation to replace level 0 mesh
	hMesh.mParts[*hMesh.partIndex(0)]->mMeshBSP->toMesh(hMesh.mParts[0]->mMesh);
#endif

	hMesh.clear(true);

	physx::ExplicitHierarchicalMesh tempCoreMesh;

	physx::PxU32 coreChunkIndex = 0xFFFFFFFF;
	physx::PxU32 corePartIndex = 0xFFFFFFFF;
	if (hMeshCore.partCount() > 0 && !canceled)
	{
		// We have a core mesh.
		tempCoreMesh.set(iHMeshCore);

		if (exportCoreMesh)
		{
			// Use it as our first split
			// Core starts as original mesh
			coreChunkIndex = hMesh.addChunk();
			corePartIndex = hMesh.addPart();
			hMesh.mChunks[coreChunkIndex]->mPartIndex = (physx::PxI32)corePartIndex;
			hMesh.mParts[corePartIndex]->mMesh = hMeshCore.mParts[0]->mMesh;
			hMesh.buildMeshBounds(corePartIndex);
			hMesh.buildCollisionGeometryForPart(corePartIndex, getVolumeDesc(collisionDesc, 1));
			hMesh.mChunks[coreChunkIndex]->mParentIndex = 0;
		}

		// Add necessary submesh data to hMesh from core.
		physx::Array<physx::PxU32> submeshMap(tempCoreMesh.mSubmeshData.size());
		if (exportCoreMesh || coreMeshImprintSubmeshIndex < 0)
		{
			for (physx::PxU32 i = 0; i < tempCoreMesh.mSubmeshData.size(); ++i)
			{
				physx::NxExplicitSubmeshData& coreSubmeshData = tempCoreMesh.mSubmeshData[i];
				submeshMap[i] = hMesh.mSubmeshData.size();
				for (physx::PxU32 j = 0; j < hMesh.mSubmeshData.size(); ++j)
				{
					physx::NxExplicitSubmeshData& submeshData = hMesh.mSubmeshData[j];
					if (0 == strcmp(submeshData.mMaterialName, coreSubmeshData.mMaterialName))
					{
						submeshMap[i] = j;
						break;
					}
				}
				if (submeshMap[i] == hMesh.mSubmeshData.size())
				{
					hMesh.mSubmeshData.pushBack(coreSubmeshData);
				}
			}
		}

		if (coreMeshImprintSubmeshIndex >= (physx::PxI32)hMesh.mSubmeshData.size())
		{
			coreMeshImprintSubmeshIndex = 0;
		}

		for (physx::PxU32 i = 0; i < tempCoreMesh.chunkCount(); ++i)
		{
			if (!tempCoreMesh.mChunks[i]->isRootChunk())
			{
				continue;
			}

			// Remap materials
			physx::PxU32 partIndex = (physx::PxU32)*tempCoreMesh.partIndex(i);
			for (physx::PxU32 j = 0; j < tempCoreMesh.mParts[partIndex]->mMesh.size(); ++j)
			{
				physx::NxExplicitRenderTriangle& tri = tempCoreMesh.mParts[partIndex]->mMesh[j];
				if (tri.submeshIndex >= 0 && tri.submeshIndex < (physx::PxI32)submeshMap.size())
				{
					tri.submeshIndex = coreMeshImprintSubmeshIndex < 0 ? (physx::PxI32)submeshMap[(physx::PxU32)tri.submeshIndex] : coreMeshImprintSubmeshIndex;
					if (exportCoreMesh && i == 0)
					{
						hMesh.mParts[corePartIndex]->mMesh[j].submeshIndex = (physx::PxI32)submeshMap[(physx::PxU32)hMesh.mParts[corePartIndex]->mMesh[j].submeshIndex];
					}
				}
				else
				{
					tri.submeshIndex = coreMeshImprintSubmeshIndex;
				}
			}

			// Make sure we've got BSPs up to hMesh.mRootDepth
			if (tempCoreMesh.mParts[partIndex]->mMeshBSP->getType() != ApexCSG::BSPType::Nontrivial)
			{
				outputMessage("Building core mesh BSP...");
				progressListener.setProgress(0);
				if(tempCoreMesh.calculatePartBSP(partIndex, randomSeed, meshProcessingParams.microgridSize, meshProcessingParams.meshMode, &progressListener, cancel)) 
				{
					outputMessage("Core mesh BSP completed.");
				}
				else
				{
					outputMessage("Core mesh BSP calculation failed.");
					canceled = true;
				}
				userRnd.m_rnd.setSeed(randomSeed);
			}
		}
	}

	gIslandGeneration = meshProcessingParams.islandGeneration;
	gMicrogridSize = meshProcessingParams.microgridSize;
	gVerbosity = meshProcessingParams.verbosity;

	for (physx::PxU32 chunkIndex = 0; chunkIndex < hMesh.mChunks.size() && !canceled; ++chunkIndex)
	{
		const physx::PxU32 depth = hMesh.depth(chunkIndex);

		if (!hMesh.mChunks[chunkIndex]->isRootLeafChunk())
		{
			continue;	// Only process core leaf chunk
		}

		if (chunkIndex == coreChunkIndex)
		{
			continue;	// Ignore core chunk
		}

		physx::PxU32 partIndex = (physx::PxU32)*hMesh.partIndex(chunkIndex);

		ApexCSG::IApexBSP* seedBSP = createBSP(hMesh.mBSPMemCache);
		seedBSP->copy(*hMesh.mParts[partIndex]->mMeshBSP);

		// Subtract out core
		bool partModified = false;
		for (physx::PxU32 i = 0; i < tempCoreMesh.chunkCount(); ++i)
		{
			if (!tempCoreMesh.mChunks[i]->isRootLeafChunk())
			{
				continue;
			}
			physx::PxU32 corePartIndex = (physx::PxU32)*tempCoreMesh.partIndex(i);
			if (tempCoreMesh.mParts[corePartIndex]->mMeshBSP != NULL)
			{
				ApexCSG::IApexBSP* rescaledCoreMeshBSP = createBSP(hMesh.mBSPMemCache);
				rescaledCoreMeshBSP->copy(*tempCoreMesh.mParts[corePartIndex]->mMeshBSP, physx::PxMat44::createIdentity(), seedBSP->getInternalTransform());
				seedBSP->combine(*rescaledCoreMeshBSP);
				rescaledCoreMeshBSP->release();
				seedBSP->op(*seedBSP, ApexCSG::Operation::A_Minus_B);
				partModified = true;
			}
		}

		if (partModified && depth > 0)
		{
			// Create part from modified seedBSP (unless it's at level 0)
			seedBSP->toMesh(hMesh.mParts[partIndex]->mMesh);
			if (hMesh.mParts[partIndex]->mMesh.size() != 0)
			{
				hMesh.mParts[partIndex]->mMeshBSP->copy(*seedBSP);
				hMesh.buildCollisionGeometryForPart(partIndex, getVolumeDesc(collisionDesc, depth));
			}
		}

#if 0	// Should always have been true
		if (depth == hMesh.mRootDepth)
#endif
		{
			// At hMesh.mRootDepth - split
			outputMessage("Splitting...");
			progressListener.setProgress(0);
			canceled = !splitter.process(hMesh, chunkIndex, *seedBSP, collisionDesc, progressListener, cancel);
			outputMessage("splitting completed.");
		}

		seedBSP->release();
	}

	// Restore if canceled
	if (canceled && save != NULL)
	{
		physx::PxU32 len;
		const void* mem = physx::NxGetApexSDK()->getMemoryWriteBuffer(*save, len);
		physx::PxFileBuf* load = physx::NxGetApexSDK()->createMemoryReadStream(mem, len);
		if (load != NULL)
		{
			hMesh.deserialize(*load, embedding);
			physx::NxGetApexSDK()->releaseMemoryReadStream(*load);
		}
	}

	if (save != NULL)
	{
		physx::NxGetApexSDK()->releaseMemoryReadStream(*save);
	}

	if (canceled)
	{
		return false;
	}

	if (meshProcessingParams.removeTJunctions && hMesh.mParts.size())
	{
		MeshProcessor meshProcessor;
		const physx::PxF32 size = hMesh.mParts[0]->mBounds.getExtents().magnitude();
		for (physx::PxU32 i = 0; i < hMesh.partCount(); ++i)
		{
			meshProcessor.setMesh(hMesh.mParts[i]->mMesh, NULL, 0, 0.0001f*size);
			meshProcessor.removeTJunctions();
		}
	}

	physx::Array<physx::PxU32> remap;
	hMesh.sortChunks(&remap);

	hMesh.createPartSurfaceNormals();

	if (corePartIndex < hMesh.partCount())
	{
		// Create reasonable collision hulls when there is a core mesh
		coreChunkIndex = remap[coreChunkIndex];
		const physx::PxMat34Legacy idTM;
		const physx::PxVec3 idScale(1.0f);
		for (physx::PxU32 coreHullIndex = 0; coreHullIndex < hMesh.mParts[corePartIndex]->mCollision.size(); ++coreHullIndex)
		{
			const physx::PartConvexHullProxy& coreHull = *hMesh.mParts[corePartIndex]->mCollision[coreHullIndex];
			for (physx::PxU32 i = 1; i < hMesh.partCount(); ++i)
			{
				if (i == coreChunkIndex)
				{
					continue;
				}
				for (physx::PxU32 hullIndex = 0; hullIndex < hMesh.mParts[i]->mCollision.size(); ++hullIndex)
				{
					physx::PartConvexHullProxy& hull = *hMesh.mParts[i]->mCollision[hullIndex];
					physx::ConvexHull::Separation separation;
					if (physx::ConvexHull::hullsInProximity(coreHull.impl, idTM, idScale, hull.impl, idTM, idScale, 0.0f, &separation))
					{
						const physx::PxF32 hullWidth = separation.max1 - separation.min1;
						const physx::PxF32 overlap = separation.max0 - separation.min1;
						if (overlap < 0.25f * hullWidth)
						{
							// Trim the hull
							hull.impl.intersectPlaneSide(physx::PxPlane(-separation.plane.n, -separation.max0));
						}
					}
				}
			}
		}
	}

	return splitter.finalize(hMesh);
}

// Note: chunks must be in breadth-first order
static void deleteChunkChildren
(
 physx::ExplicitHierarchicalMesh& hMesh,
 physx::PxU32 chunkIndex,
 bool deleteChunk = false
 )
{
	for (physx::PxU32 index = hMesh.chunkCount(); index-- > chunkIndex+1;)
	{
		if (hMesh.mChunks[index]->mParentIndex == (physx::PxI32)chunkIndex)
		{
			deleteChunkChildren(hMesh, index, true);
		}
	}

	if (deleteChunk)
	{
		const physx::PxI32 partIndex = hMesh.mChunks[chunkIndex]->mPartIndex;
		hMesh.removeChunk(chunkIndex);
		bool partIndexUsed = false;
		for (physx::PxU32 index = 0; index < hMesh.chunkCount(); ++index)
		{
			if (hMesh.mChunks[index]->mPartIndex == partIndex)
			{
				partIndexUsed = true;
				break;
			}
		}
		if (!partIndexUsed)
		{
			hMesh.removePart((physx::PxU32)partIndex);
		}
	}
}

static bool splitChunkInternal
(
	physx::NxExplicitHierarchicalMesh& iHMesh,
	physx::PxU32 chunkIndex,
	const FractureTools::NxMeshProcessingParameters& meshProcessingParams,
	MeshSplitter& splitter,
	const NxCollisionDesc& collisionDesc,
	physx::PxU32* randomSeed,
	IProgressListener& progressListener,
	volatile bool* cancel
)
{
	const physx::PxI32* partIndexPtr = iHMesh.partIndex(chunkIndex);
	if (partIndexPtr == NULL)
	{
		return true;
	}
	const physx::PxU32 partIndex = (physx::PxU32)*partIndexPtr;

	gIslandGeneration = meshProcessingParams.islandGeneration;
	gMicrogridSize = meshProcessingParams.microgridSize;
	gVerbosity = meshProcessingParams.verbosity;

	outputMessage("Splitting...");

	// Save state if cancel != NULL
	physx::PxFileBuf* save = NULL;
	class NullEmbedding : public physx::NxExplicitHierarchicalMesh::NxEmbedding
	{
		void	serialize(physx::PxFileBuf& stream, NxEmbedding::DataType type) const
		{
			(void)stream;
			(void)type;
		}
		void	deserialize(physx::PxFileBuf& stream, NxEmbedding::DataType type, physx::PxU32 version)
		{
			(void)stream;
			(void)type;
			(void)version;
		}
	} embedding;

	physx::ExplicitHierarchicalMesh& hMesh = *(physx::ExplicitHierarchicalMesh*)&iHMesh;
	
	if (cancel != NULL)
	{
		save = physx::NxGetApexSDK()->createMemoryWriteStream();
		if (save != NULL)
		{
			hMesh.serialize(*save, embedding);			
		}
	}
	bool canceled = false;

	progressListener.setProgress(0);

	// Delete chunk children
	deleteChunkChildren(hMesh, chunkIndex);

	// Reseed if requested
	if (randomSeed != NULL)
	{
		userRnd.m_rnd.setSeed(*randomSeed);
	}
	const physx::PxU32 seed = userRnd.m_rnd.seed();

	// Fracture chunk
	ApexCSG::IApexBSP* chunkMeshBSP = hMesh.mParts[partIndex]->mMeshBSP;

	// Make sure we've got a BSP.  If this is a root chunk, it may not have been created yet.
	if (chunkMeshBSP->getType() != ApexCSG::BSPType::Nontrivial)
	{
		if (!hMesh.mChunks[chunkIndex]->isRootChunk())
		{
			outputMessage("Warning: Building a BSP for a non-root mesh.  This should have been created by a splitting process.");
		}
		outputMessage("Building mesh BSP...");
		progressListener.setProgress(0);
		hMesh.calculatePartBSP(partIndex, seed, meshProcessingParams.microgridSize, meshProcessingParams.meshMode, &progressListener);
		outputMessage("Mesh BSP completed.");
		userRnd.m_rnd.setSeed(seed);
	}

	const physx::PxU32 oldPartCount = hMesh.mParts.size();

	canceled = !splitter.process(hMesh, chunkIndex, *chunkMeshBSP, collisionDesc, progressListener, cancel);

	// Restore if canceled
	if (canceled && save != NULL)
	{
		physx::PxU32 len;
		const void* mem = physx::NxGetApexSDK()->getMemoryWriteBuffer(*save, len);
		physx::PxFileBuf* load = physx::NxGetApexSDK()->createMemoryReadStream(mem, len);
		if (load != NULL)
		{
			hMesh.deserialize(*load, embedding);
			physx::NxGetApexSDK()->releaseMemoryReadStream(*load);
		}
	}

	if (save != NULL)
	{
		physx::NxGetApexSDK()->releaseMemoryReadStream(*save);
	}

	if (canceled)
	{
		return false;
	}

	if (meshProcessingParams.removeTJunctions)
	{
		MeshProcessor meshProcessor;
		const physx::PxF32 size = hMesh.mParts[partIndex]->mBounds.getExtents().magnitude();
		for (physx::PxU32 i = oldPartCount; i < hMesh.partCount(); ++i)
		{
			meshProcessor.setMesh(hMesh.mParts[i]->mMesh, NULL, 0, 0.0001f*size);
			meshProcessor.removeTJunctions();
		}
	}

	hMesh.sortChunks();

	hMesh.createPartSurfaceNormals();

	return true;
}


static physx::PxU32 createVoronoiSitesInsideMeshInternal
(
	physx::ExplicitHierarchicalMesh& hMesh,
	const physx::PxU32* chunkIndices,
	physx::PxU32 chunkCount,
	physx::PxVec3* siteBuffer,
	physx::PxU32* siteChunkIndices,
	physx::PxU32 siteCount,
	physx::PxU32* randomSeed,
	physx::PxU32* microgridSize,
	NxBSPOpenMode::Enum meshMode,
	physx::IProgressListener& progressListener
)
{
	if (randomSeed != NULL)
	{
		userRnd.m_rnd.setSeed(*randomSeed);
	}

	const physx::PxU32 microgridSizeToUse = microgridSize != NULL ? *microgridSize : gMicrogridSize;

	// Make sure we've got BSPs for all chunks
	for (physx::PxU32 chunkNum = 0; chunkNum < chunkCount; ++chunkNum)
	{
		const physx::PxU32 chunkIndex = chunkIndices[chunkNum];
		physx::PxU32 partIndex = (physx::PxU32)*hMesh.partIndex(chunkIndex);
		if (hMesh.mParts[partIndex]->mMeshBSP->getType() != ApexCSG::BSPType::Nontrivial)
		{
			outputMessage("Building mesh BSP...");
			progressListener.setProgress(0);
			if (randomSeed == NULL)
			{
				outputMessage("Warning: no random seed given in createVoronoiSitesInsideMeshInternal but BSP must be built.  Using seed = 0.", physx::PxErrorCode::eDEBUG_WARNING);
			}
			hMesh.calculatePartBSP(partIndex, (randomSeed != NULL ? *randomSeed : 0), microgridSizeToUse, meshMode, &progressListener);
			outputMessage("Mesh BSP completed.");
			if (randomSeed != NULL)
			{
				userRnd.m_rnd.setSeed(*randomSeed);
			}
		}
	}

	// Come up with distribution that is weighted by chunk volume, but also tries to ensure each chunk gets at least one site.
	physx::PxF32 totalVolume = 0.0f;
	physx::Array<physx::PxF32> volumes(chunkCount);
	physx::Array<physx::PxU32> siteCounts(chunkCount);
	for (physx::PxU32 chunkNum = 0; chunkNum < chunkCount; ++chunkNum)
	{
		const physx::PxU32 chunkIndex = chunkIndices[chunkNum];
		const physx::PxBounds3 bounds = hMesh.chunkBounds(chunkIndex);
		const physx::PxVec3 extents = bounds.getExtents();
		volumes[chunkNum] = extents.x*extents.y*extents.z;
		totalVolume += volumes[chunkNum];
		siteCounts[chunkNum] = 0;
	}

	// Now fill in site counts
	if (totalVolume <= 0.0f)
	{
		totalVolume = 1.0f;	// To avoid divide-by-zero
	}

	// Make site count proportional to volume, within error due to quantization.  Ensure at least one site per chunk, even if "zero volume"
	// is recorded (it might be an open-meshed chunk).  We distinguish between zero and one sites per chunk, even though they have the same
	// effect on a chunk, since using one site per chunk will reduce the number of sites available for other chunks.  The aim is to have
	// control over the number of chunks generated, so we will avoid using zero sites per chunk.
	physx::PxU32 totalSiteCount = 0;
	for (physx::PxU32 chunkNum = 0; chunkNum < chunkCount; ++chunkNum)
	{
		siteCounts[chunkNum] = physx::PxMax(1U, (physx::PxU32)(siteCount*volumes[chunkNum]/totalVolume));
		totalSiteCount += siteCounts[chunkNum];
	}

	// Add sites if we need to.  This can happen due to the rounding.
	while (totalSiteCount < siteCount)
	{
		physx::PxU32 chunkToAddSite = 0;
		physx::PxF32 greatestDeficit = -PX_MAX_F32;
		for (physx::PxU32 chunkNum = 0; chunkNum < chunkCount; ++chunkNum)
		{
			const physx::PxF32 defecit = siteCount*volumes[chunkNum]/totalVolume - (physx::PxF32)siteCounts[chunkNum];
			if (defecit > greatestDeficit)
			{
				greatestDeficit = defecit;
				chunkToAddSite = chunkNum;
			}
		}
		++siteCounts[chunkToAddSite];
		++totalSiteCount;
	}

	// Remove sites if necessary.  This is much more likely.
	while (totalSiteCount > siteCount)
	{
		physx::PxU32 chunkToRemoveSite = 0;
		physx::PxF32 greatestSurplus = -PX_MAX_F32;
		for (physx::PxU32 chunkNum = 0; chunkNum < chunkCount; ++chunkNum)
		{
			const physx::PxF32 surplus = (physx::PxF32)siteCounts[chunkNum] - siteCount*volumes[chunkNum]/totalVolume;
			if (surplus > greatestSurplus)
			{
				greatestSurplus = surplus;
				chunkToRemoveSite = chunkNum;
			}
		}
		--siteCounts[chunkToRemoveSite];
		--totalSiteCount;
	}

	// Now generate the actual sites
	physx::PxU32 totalSitesGenerated = 0;
	for (physx::PxU32 chunkNum = 0; chunkNum < chunkCount; ++chunkNum)
	{
		const physx::PxU32 chunkIndex = chunkIndices[chunkNum];
		physx::PxU32 partIndex = (physx::PxU32)*hMesh.partIndex(chunkIndex);
		ApexCSG::IApexBSP* meshBSP = hMesh.mParts[partIndex]->mMeshBSP;
		const physx::PxBounds3 bounds = hMesh.chunkBounds(chunkIndex);
		physx::PxU32 sitesGenerated = 0;
		physx::PxU32 attemptsLeft = 100000;
		while (	sitesGenerated < siteCounts[chunkNum])
		{
			const physx::PxVec3 site(userRnd.getReal(bounds.minimum.x, bounds.maximum.x), userRnd.getReal(bounds.minimum.y, bounds.maximum.y), userRnd.getReal(bounds.minimum.z, bounds.maximum.z));
			if (!attemptsLeft || meshBSP->pointInside(site - *hMesh.instancedPositionOffset(chunkIndex)))
			{
				siteBuffer[totalSitesGenerated] = site;
				if (siteChunkIndices != NULL)
				{
					siteChunkIndices[totalSitesGenerated] = chunkIndex;
				}
				++sitesGenerated;
				++totalSitesGenerated;
			}
			if (attemptsLeft)
			{
				--attemptsLeft;
			}
		}
	}

	return totalSitesGenerated;
}

class HierarchicalMeshSplitter : public MeshSplitter
{
private:
	HierarchicalMeshSplitter& operator=(const HierarchicalMeshSplitter&);

public:
	HierarchicalMeshSplitter(const NxFractureSliceDesc& desc) : mDesc(desc)
	{
	}

	bool validate(physx::ExplicitHierarchicalMesh& hMesh)
	{
		// Try to see if we're going to generate too many chunks
		physx::PxU32 estimatedTotalChunkCount = 0;
		for (physx::PxU32 chunkIndex = 0; chunkIndex < hMesh.chunkCount(); ++chunkIndex)
		{
			if (!hMesh.mChunks[chunkIndex]->isRootLeafChunk())
			{
				continue;
			}
			physx::PxU32 partIndex = (physx::PxU32)*hMesh.partIndex(chunkIndex);
			physx::PxU32 estimatedLevelChunkCount = 1;
			physx::PxVec3 estimatedExtent = hMesh.mParts[partIndex]->mBounds.getExtents();
			for (physx::PxU32 chunkDepth = 0; chunkDepth < mDesc.maxDepth; ++chunkDepth)
			{
				// Get parameters for this depth
				const physx::NxSliceParameters& sliceParameters = mDesc.sliceParameters[chunkDepth];
				int partition[3];
				calculatePartition(partition, sliceParameters.splitsPerPass, estimatedExtent, mDesc.useTargetProportions ? mDesc.targetProportions : NULL);
				estimatedLevelChunkCount *= partition[0] * partition[1] * partition[2];
				estimatedTotalChunkCount += estimatedLevelChunkCount;
				if (estimatedTotalChunkCount > MAX_ALLOWED_ESTIMATED_CHUNK_TOTAL)
				{
					char message[1000];
					physx::string::sprintf_s(message,1000, "Slicing chunk count is estimated to be %d chunks, exceeding the maximum allowed estimated total of %d chunks.  Aborting.  Try using fewer slices, or a smaller fracture depth.",
						estimatedTotalChunkCount, (int)MAX_ALLOWED_ESTIMATED_CHUNK_TOTAL);
					outputMessage(message, physx::PxErrorCode::eINTERNAL_ERROR);
					return false;
				}
				estimatedExtent[0] /= partition[0];
				estimatedExtent[1] /= partition[1];
				estimatedExtent[2] /= partition[2];
			}
		}

		return true;
	}

	void initialize(physx::ExplicitHierarchicalMesh& hMesh)
	{
		if (mDesc.useDisplacementMaps)
		{
			hMesh.initializeDisplacementMapVolume(mDesc);
		}

		for (int i = 0; i < 3; ++i)
		{
			hMesh.mSubmeshData.resize(physx::PxMax(hMesh.mRootSubmeshCount, mDesc.materialDesc[i].interiorSubmeshIndex + 1));
		}
	}

	bool process
	(
		physx::ExplicitHierarchicalMesh& hMesh,
		physx::PxU32 chunkIndex,
		const ApexCSG::IApexBSP& chunkBSP,
		const NxCollisionDesc& collisionDesc,
		physx::IProgressListener& progressListener,
		volatile bool* cancel
	)
	{
		physx::PxPlane trailingPlanes[3];	// passing in depth = 0 will initialize these
		physx::PxPlane leadingPlanes[3];
#if 1	// Eliminating volume calculation here, for performance.  May introduce it later once the mesh is calculated.
		const physx::PxF32 chunkVolume = 1.0f;
#else
		physx::PxF32 chunkArea, chunkVolume;
		chunkBSP.getSurfaceAreaAndVolume(chunkArea, chunkVolume, true);
#endif
		return hierarchicallySplitChunkInternal(hMesh, chunkIndex, 0, trailingPlanes, leadingPlanes, chunkBSP, chunkVolume, mDesc, collisionDesc, progressListener, cancel);
	}

	bool finalize(physx::ExplicitHierarchicalMesh& hMesh)
	{
		if (mDesc.instanceChunks)
		{
			for (physx::PxU32 i = 0; i < hMesh.partCount(); ++i)
			{
				hMesh.mChunks[i]->mFlags |= physx::apex::NxDestructibleAsset::ChunkIsInstanced;
			}
		}

		return true;
	}

protected:
	const NxFractureSliceDesc& mDesc;
};

bool createHierarchicallySplitMesh
(
    physx::NxExplicitHierarchicalMesh& iHMesh,
    physx::NxExplicitHierarchicalMesh& iHMeshCore,
    bool exportCoreMesh,
	physx::PxI32 coreMeshImprintSubmeshIndex,	// If this is < 0, use the core mesh materials (was applyCoreMeshMaterialToNeighborChunks).  Otherwise, use the given submesh.
    const NxMeshProcessingParameters& meshProcessingParams,
    const NxFractureSliceDesc& desc,
    const NxCollisionDesc& collisionDesc,
    physx::PxU32 randomSeed,
    physx::IProgressListener& progressListener,
    volatile bool* cancel
)
{
	HierarchicalMeshSplitter splitter(desc);

	return splitMeshInternal(
		iHMesh,
		iHMeshCore,
		exportCoreMesh,
		coreMeshImprintSubmeshIndex,
		meshProcessingParams,
		splitter,
		collisionDesc,
		randomSeed,
		progressListener,
		cancel);
}

bool hierarchicallySplitChunk
(
	physx::NxExplicitHierarchicalMesh& iHMesh,
	physx::PxU32 chunkIndex,
	const FractureTools::NxMeshProcessingParameters& meshProcessingParams,
	const FractureTools::NxFractureSliceDesc& desc,
	const NxCollisionDesc& collisionDesc,
	physx::PxU32* randomSeed,
	IProgressListener& progressListener,
	volatile bool* cancel
)
{
	HierarchicalMeshSplitter splitter(desc);

	return splitChunkInternal(iHMesh, chunkIndex, meshProcessingParams, splitter, collisionDesc, randomSeed, progressListener, cancel);
}

PX_INLINE physx::PxMat34Legacy createCutoutFrame(const physx::PxVec3& center, const physx::PxVec3& extents, physx::PxU32 sliceAxes[3], physx::PxU32 sliceSignNum, const NxFractureCutoutDesc& desc, bool& invertX)
{
	const physx::PxU32 sliceDirIndex = sliceAxes[2] * 2 + sliceSignNum;
	const physx::PxF32 sliceSign = sliceSignNum ? -1.0f : 1.0f;
	physx::PxVec3 n = createAxis(sliceAxes[2]) * sliceSign;
	physx::PxMat34Legacy cutoutTM;
	cutoutTM.M.setColumn(2, n);
	physx::PxF32 applySign;
	switch (sliceAxes[2])
	{
	case 0:
		applySign = 1.0f;
		break;
	case 1:
		applySign = -1.0f;
		break;
	default:
	case 2:
		applySign = 1.0f;
	}
	const physx::PxVec3 p = createAxis(sliceAxes[1]);
	const physx::PxF32 cutoutPadding = desc.tileFractureMap ? 0.0f : 0.0001f;
	cutoutTM.M.setColumn(1, p);
	cutoutTM.M.setColumn(0, p.cross(n));
	physx::PxF32 cutoutWidth = 2 * extents[sliceAxes[0]] * (1.0f + cutoutPadding);
	physx::PxF32 cutoutHeight = 2 * extents[sliceAxes[1]] * (1.0f + cutoutPadding);
	cutoutWidth *= (desc.cutoutWidthInvert[sliceDirIndex] ? -1.0f : 1.0f) * desc.cutoutWidthScale[sliceDirIndex];
	cutoutHeight *= (desc.cutoutHeightInvert[sliceDirIndex] ? -1.0f : 1.0f) * desc.cutoutHeightScale[sliceDirIndex];
	cutoutTM.M.multiplyDiagonal(physx::PxVec3(cutoutWidth / desc.cutoutSizeX, cutoutHeight / desc.cutoutSizeY, 1.0f));
	cutoutTM.t = physx::PxVec3(0.0f);
	physx::PxF32 sign = applySign * sliceSign;
	invertX = sign < 0.0f;
	cutoutTM.t[sliceAxes[0]] = center[sliceAxes[0]] - sign * (0.5f * cutoutWidth + desc.cutoutWidthOffset[sliceDirIndex] * extents[sliceAxes[0]]);
	cutoutTM.t[sliceAxes[1]] = center[sliceAxes[1]] - 0.5f * cutoutHeight + desc.cutoutHeightOffset[sliceDirIndex] * extents[sliceAxes[1]];

	return cutoutTM;
}

static bool createCutoutChunk(physx::ExplicitHierarchicalMesh& hMesh, ApexCSG::IApexBSP& cutoutBSP, /*IApexBSP& remainderBSP,*/
							  const ApexCSG::IApexBSP& sourceBSP, physx::PxU32 sourceIndex,
                              const physx::NxCollisionVolumeDesc& volumeDesc, volatile bool* cancel)
{
//	remainderBSP.combine( cutoutBSP );
//	remainderBSP.op( remainderBSP, Operation::A_Minus_B );
	cutoutBSP.combine(sourceBSP);
	cutoutBSP.op(cutoutBSP, ApexCSG::Operation::Intersection);
	// BRG - should apply island generation here
//	if( gIslandGeneration )
//	{
//	}
	const physx::PxU32 newPartIndex = hMesh.addPart();
	hMesh.mParts[newPartIndex]->mMeshBSP = &cutoutBSP;	// Save off and own this
	cutoutBSP.toMesh(hMesh.mParts[newPartIndex]->mMesh);
	if (hMesh.mParts[newPartIndex]->mMesh.size() > 0)
	{
		hMesh.mParts[newPartIndex]->mMeshBSP->copy(cutoutBSP);
		hMesh.buildMeshBounds(newPartIndex);
		hMesh.buildCollisionGeometryForPart(newPartIndex, volumeDesc);
		const physx::PxU32 newChunkIndex = hMesh.addChunk();
		hMesh.mChunks[newChunkIndex]->mParentIndex = (physx::PxI32)sourceIndex;
		hMesh.mChunks[newChunkIndex]->mPartIndex = (physx::PxI32)newPartIndex;
	}
	else
	{
		hMesh.removePart(newPartIndex);
	}

	return cancel == NULL || !(*cancel);
}

static void addQuad(physx::ExplicitHierarchicalMesh& hMesh, physx::Array<physx::NxExplicitRenderTriangle>& mesh, physx::PxU32 sliceDepth, physx::PxU32 submeshIndex,
					const physx::PxVec2& interiorUVScale, const physx::PxVec3& v0, const physx::PxVec3& v1, const physx::PxVec3& v2, const physx::PxVec3& v3)
{
	// Create material frame TM
	const physx::PxU32 materialIndex = hMesh.addMaterialFrame();
	physx::NxMaterialFrame materialFrame = hMesh.getMaterialFrame(materialIndex);

	physx::NxFractureMaterialDesc materialDesc;

	/* BRG: these should be obtained from an alternative set of material descs (one for each cutout direction), which describe the UV layout around the chunk cutout. */
	materialDesc.uAngle = 0.0f;
	materialDesc.uvOffset = physx::PxVec2(0.0f);
	materialDesc.uvScale = interiorUVScale;

	materialDesc.tangent = v1 - v0;
	materialDesc.tangent.normalize();
	physx::PxVec3 normal = materialDesc.tangent.cross(v3 - v0);
	normal.normalize();
	const physx::PxPlane plane(v0, normal);

	materialFrame.buildCoordinateSystemFromMaterialDesc(materialDesc, plane);
	materialFrame.mFractureMethod = physx::NxFractureMethod::Cutout;
	materialFrame.mFractureIndex = -1;	// Signifying that it's a cutout around the chunk, so we shouldn't make assumptions about the face direction
	materialFrame.mSliceDepth = sliceDepth;

	hMesh.setMaterialFrame(materialIndex, materialFrame);

	// Create interpolator for triangle quantities

	physx::TriangleFrame triangleFrame(materialFrame.mCoordinateSystem, interiorUVScale, physx::PxVec2(0.0f));

	// Fill one triangle
	physx::NxExplicitRenderTriangle& tri0 = mesh.insert();
	memset(&tri0, 0, sizeof(physx::NxExplicitRenderTriangle));
	tri0.submeshIndex = (physx::PxI32)submeshIndex;
	tri0.extraDataIndex = materialIndex;
	tri0.smoothingMask = 0;
	tri0.vertices[0].position = v0;
	tri0.vertices[1].position = v1;
	tri0.vertices[2].position = v2;
	for (int i = 0; i < 3; ++i)
	{
		triangleFrame.interpolateVertexData(tri0.vertices[i]);
	}

	// ... and another
	physx::NxExplicitRenderTriangle& tri1 = mesh.insert();
	memset(&tri1, 0, sizeof(physx::NxExplicitRenderTriangle));
	tri1.submeshIndex = (physx::PxI32)submeshIndex;
	tri1.extraDataIndex = materialIndex;
	tri1.smoothingMask = 0;
	tri1.vertices[0].position = v2;
	tri1.vertices[1].position = v3;
	tri1.vertices[2].position = v0;
	for (int i = 0; i < 3; ++i)
	{
		triangleFrame.interpolateVertexData(tri1.vertices[i]);
	}
}


static bool createCutout(
	physx::ExplicitHierarchicalMesh& hMesh,
	physx::PxU32 faceChunkIndex,
	ApexCSG::IApexBSP& faceBSP,	// May be modified
	const physx::Cutout& cutout,
	const physx::PxMat34Legacy& cutoutTM,
	const physx::NxFractureCutoutDesc& desc,
	const physx::NxNoiseParameters& edgeNoise,
	physx::PxF32 cutoutDepth,
	const physx::NxFractureMaterialDesc& materialDesc,
	const physx::NxCollisionVolumeDesc& volumeDesc,
	const physx::PxPlane& minPlane,
	const physx::PxPlane& maxPlane,
	physx::HierarchicalProgressListener& localProgressListener,
	volatile bool* cancel)
{
	bool canceled = false;

	const physx::PxF32 cosSmoothingThresholdAngle = physx::PxCos(desc.facetNormalMergeThresholdAngle * physx::PxPi / 180.0f);

	physx::Array<physx::PxPlane> trimPlanes;
	const physx::PxU32 oldPartCount = hMesh.partCount();
	localProgressListener.setSubtaskWork(1);
	const physx::PxU32 loopCount = desc.splitNonconvexRegions ? cutout.convexLoops.size() : 1;

	const bool ccw = cutoutTM.M.determinant() > (physx::PxF32)0;

	const physx::PxU32 facePartIndex = (physx::PxU32)*hMesh.partIndex(faceChunkIndex);

	const physx::PxU32 sliceDepth = hMesh.depth(faceChunkIndex) + 1;

	for (physx::PxU32 j = 0; j < loopCount && !canceled; ++j)
	{
		const physx::PxU32 loopSize = desc.splitNonconvexRegions ? cutout.convexLoops[j].polyVerts.size() : cutout.vertices.size();
		if (desc.splitNonconvexRegions)
		{
			trimPlanes.reset();
		}
		// Build mesh which surrounds the cutout
		physx::Array<physx::NxExplicitRenderTriangle> loopMesh;
		for (physx::PxU32 k = 0; k < loopSize; ++k)
		{
			physx::PxU32 kPrime = ccw ? k : loopSize - 1 - k;
			physx::PxU32 kPrimeNext = ccw ? ((kPrime + 1) % loopSize) : (kPrime == 0 ? (loopSize - 1) : (kPrime-1));
			const physx::PxU32 vertexIndex0 = desc.splitNonconvexRegions ? cutout.convexLoops[j].polyVerts[kPrime].index : kPrime;
			const physx::PxU32 vertexIndex1 = desc.splitNonconvexRegions ? cutout.convexLoops[j].polyVerts[kPrimeNext].index : kPrimeNext;
			const physx::PxVec3& v0 = cutout.vertices[vertexIndex0];
			const physx::PxVec3& v1 = cutout.vertices[vertexIndex1];
			const physx::PxVec3 v0World = cutoutTM * v0;
			const physx::PxVec3 v1World = cutoutTM * v1;
			const physx::PxVec3 quad0 = minPlane.project(v0World);
			const physx::PxVec3 quad1 = minPlane.project(v1World);
			const physx::PxVec3 quad2 = maxPlane.project(v1World);
			const physx::PxVec3 quad3 = maxPlane.project(v0World);
			addQuad(hMesh, loopMesh, sliceDepth, materialDesc.interiorSubmeshIndex, materialDesc.uvScale, quad0, quad1, quad2, quad3);
			if (cutout.convexLoops.size() == 1 || desc.splitNonconvexRegions)
			{
				physx::PxVec3 planeNormal = (quad2 - quad0).cross(quad3 - quad1);
				planeNormal.normalize();
				trimPlanes.pushBack(physx::PxPlane(0.25f * (quad0 + quad1 + quad2 + quad3), planeNormal));
			}
		}
		// Smooth the mesh's normals and tangents
		PX_ASSERT(loopMesh.size() == 2 * loopSize);
		if (loopMesh.size() == 2 * loopSize)
		{
			for (physx::PxU32 k = 0; k < loopSize; ++k)
			{
				const physx::PxU32 triIndex0 = 2 * k;
				const physx::PxU32 frameIndex = loopMesh[triIndex0].extraDataIndex;
				PX_ASSERT(frameIndex == loopMesh[triIndex0 + 1].extraDataIndex);
				physx::PxMat44& frame = hMesh.mMaterialFrames[frameIndex].mCoordinateSystem;
				const physx::PxU32 triIndex2 = 2 * ((k + 1) % loopSize);
				const physx::PxU32 nextFrameIndex = loopMesh[triIndex2].extraDataIndex;
				PX_ASSERT(nextFrameIndex == loopMesh[triIndex2 + 1].extraDataIndex);
				physx::PxMat44& nextFrame = hMesh.mMaterialFrames[nextFrameIndex].mCoordinateSystem;
				const physx::PxVec3 normalK = frame.column2.getXYZ();
				const physx::PxVec3 normalK1 = nextFrame.column2.getXYZ();
				if (normalK.dot(normalK1) < cosSmoothingThresholdAngle)
				{
					continue;
				}
				physx::PxVec3 normal = normalK + normalK1;
				normal.normalize();
				loopMesh[triIndex0].vertices[1].normal = normal;
				loopMesh[triIndex0].vertices[2].normal = normal;
				loopMesh[triIndex0 + 1].vertices[0].normal = normal;
				loopMesh[triIndex2].vertices[0].normal = normal;
				loopMesh[triIndex2 + 1].vertices[1].normal = normal;
				loopMesh[triIndex2 + 1].vertices[2].normal = normal;
			}
			for (physx::PxU32 k = 0; k < loopMesh.size(); ++k)
			{
				physx::NxExplicitRenderTriangle& tri = loopMesh[k];
				for (physx::PxU32 v = 0; v < 3; ++v)
				{
					physx::NxVertex& vert = tri.vertices[v];
					vert.tangent = vert.binormal.cross(vert.normal);
				}
			}
		}
		// Create loop cutout BSP
		ApexCSG::IApexBSP* loopBSP = createBSP(hMesh.mBSPMemCache);
		ApexCSG::BSPBuildParameters bspBuildParams = gDefaultBuildParameters;
		bspBuildParams.rnd = &userRnd;
		bspBuildParams.internalTransform = faceBSP.getInternalTransform();
		loopBSP->fromMesh(&loopMesh[0], loopMesh.size(), bspBuildParams);
		const physx::PxU32 oldSize = hMesh.partCount();
		// loopBSP will be modified and owned by the new chunk.
		canceled = !createCutoutChunk(hMesh, *loopBSP, faceBSP, /**cutoutSource,*/ faceChunkIndex, volumeDesc, cancel);
		for (physx::PxU32 partN = oldSize; partN < hMesh.partCount(); ++partN)
		{
			// Apply graphical noise to new parts, if requested
			if (edgeNoise.amplitude > 0.0f)
			{
				applyNoiseToChunk(hMesh, facePartIndex, partN, edgeNoise, cutoutDepth);
			}
			// Trim new part collision hulls
			for (physx::PxU32 trimN = 0; trimN < trimPlanes.size(); ++trimN)
			{
				for (physx::PxU32 hullIndex = 0; hullIndex < hMesh.mParts[partN]->mCollision.size(); ++hullIndex)
				{
					hMesh.mParts[partN]->mCollision[hullIndex]->impl.intersectPlaneSide(trimPlanes[trimN]);
				}
			}
		}
		localProgressListener.completeSubtask();
	}
	// Trim hulls
	if (!canceled)
	{
		for (physx::PxU32 partN = oldPartCount; partN < hMesh.partCount(); ++partN)
		{
			for (physx::PxU32 hullIndex = 0; hullIndex < hMesh.mParts[partN]->mCollision.size(); ++hullIndex)
			{
				physx::ConvexHull& hull = hMesh.mParts[partN]->mCollision[hullIndex]->impl;
				if (!desc.splitNonconvexRegions)
				{
					for (physx::PxU32 trimN = 0; trimN < trimPlanes.size(); ++trimN)
					{
						hull.intersectPlaneSide(trimPlanes[trimN]);
					}
				}
//				hull.intersectHull(hMesh.mParts[faceChunkIndex]->mCollision.impl);	// Do we need this?
			}
		}
	}

	return !canceled;
}

static void instanceChildren(physx::ExplicitHierarchicalMesh& hMesh, physx::PxU32 instancingChunkIndex, physx::PxU32 instancedChunkIndex)
{
	for (physx::PxU32 chunkIndex = 0; chunkIndex < hMesh.chunkCount(); ++chunkIndex)
	{
		if (hMesh.mChunks[chunkIndex]->mParentIndex == (physx::PxI32)instancingChunkIndex)
		{
			// Found a child.  Instance.
			const physx::PxU32 instancedChildIndex = hMesh.addChunk();
			hMesh.mChunks[instancedChildIndex]->mFlags |= physx::apex::NxDestructibleAsset::ChunkIsInstanced;
			hMesh.mChunks[instancedChildIndex]->mParentIndex = (physx::PxI32)instancedChunkIndex;
			hMesh.mChunks[instancedChildIndex]->mPartIndex = hMesh.mChunks[chunkIndex]->mPartIndex;	// Same part as instancing child
			hMesh.mChunks[instancedChildIndex]->mInstancedPositionOffset = hMesh.mChunks[instancedChunkIndex]->mInstancedPositionOffset;	// Same offset as parent
			hMesh.mChunks[instancedChildIndex]->mInstancedUVOffset = hMesh.mChunks[instancedChunkIndex]->mInstancedUVOffset;	// Same offset as parent
			instanceChildren(hMesh, chunkIndex, instancedChildIndex);	// Recurse
		}
	}
}

static bool createFaceCutouts
(
    physx::ExplicitHierarchicalMesh& hMesh,
    physx::PxU32 faceChunkIndex,
    ApexCSG::IApexBSP& faceBSP,	// May be modified
    const physx::NxFractureCutoutDesc& desc,
	const physx::NxNoiseParameters& edgeNoise,
	physx::PxF32 cutoutDepth,
	const physx::NxFractureMaterialDesc& materialDesc,
    const physx::CutoutSet& cutoutSet,
	const physx::PxMat34Legacy& cutoutTM,
	const physx::PxF32 mapXLow,
	const physx::PxF32 mapYLow,
	const physx::NxCollisionDesc& collisionDesc,
    const physx::NxFractureSliceDesc& sliceDesc,
	const physx::NxFractureVoronoiDesc& voronoiDesc,
	const physx::PxPlane& facePlane,
    const physx::PxVec3& localCenter,
    const physx::PxVec3& localExtents,
    physx::IProgressListener& progressListener,
    volatile bool* cancel
)
{
	physx::NxFractureVoronoiDesc cutoutVoronoiDesc;
	physx::Array<physx::PxVec3> perChunkSites;

	switch (desc.chunkFracturingMethod)
	{
	case NxFractureCutoutDesc::VoronoiFractureCutoutChunks:
		{
			cutoutVoronoiDesc = voronoiDesc;
			perChunkSites.resize(voronoiDesc.siteCount);
			cutoutVoronoiDesc.sites = voronoiDesc.siteCount > 0 ? &perChunkSites[0] : NULL;
			cutoutVoronoiDesc.siteCount = voronoiDesc.siteCount;
		}
		break;
	}

	//	IApexBSP* cutoutSource = createBSP( hMesh.mBSPMemCache );
	//	cutoutSource->copy( faceBSP );

	const physx::PxVec3 faceNormal = facePlane.n;

	// "Sandwich" planes
	const physx::PxF32 centerDisp = -facePlane.d - localExtents[2];
	const physx::PxF32 paddedExtent = 1.01f * localExtents[2];
	const physx::PxPlane maxPlane(faceNormal, -centerDisp - paddedExtent);
	const physx::PxPlane minPlane(faceNormal, -centerDisp + paddedExtent);

	bool canceled = false;

	// Tiling bounds
	const physx::PxF32 xTol = CUTOUT_MAP_BOUNDS_TOLERANCE*localExtents[0];
	const physx::PxF32 yTol = CUTOUT_MAP_BOUNDS_TOLERANCE*localExtents[1];
	const physx::PxF32 mapWidth = cutoutTM.M.getColumn(0).magnitude()*cutoutSet.getDimensions()[0];
	const physx::PxF32 mapHeight = cutoutTM.M.getColumn(1).magnitude()*cutoutSet.getDimensions()[1];
	const physx::PxF32 boundsXLow = localCenter[0] - localExtents[0];
	const physx::PxF32 boundsWidth = 2*localExtents[0];
	const physx::PxF32 boundsYLow = localCenter[1] - localExtents[1];
	const physx::PxF32 boundsHeight = 2*localExtents[1];
	physx::PxI32 ixStart = desc.tileFractureMap ? (physx::PxI32)physx::PxFloor((boundsXLow - mapXLow)/mapWidth + xTol) : 0;
	physx::PxI32 ixStop = desc.tileFractureMap ? (physx::PxI32)physx::PxCeil((boundsXLow - mapXLow + boundsWidth)/mapWidth - xTol) : 1;
	physx::PxI32 iyStart = desc.tileFractureMap ? (physx::PxI32)physx::PxFloor((boundsYLow - mapYLow)/mapHeight + yTol) : 0;
	physx::PxI32 iyStop = desc.tileFractureMap ? (physx::PxI32)physx::PxCeil((boundsYLow - mapYLow + boundsHeight)/mapHeight - yTol) : 1;

	// Find UV map

	const physx::PxVec3 xDir = cutoutTM.M.getColumn(0).getNormalized();
	const physx::PxVec3 yDir = cutoutTM.M.getColumn(1).getNormalized();

	// First find a good representative face triangle
	const physx::PxF32 faceDiffTolerance = 0.001f;
	physx::PxU32 uvMapTriangleIndex = 0;
	physx::PxF32 uvMapTriangleIndexFaceDiff = PX_MAX_F32;
	physx::PxF32 uvMapTriangleIndexArea = 0.0f;
	const physx::PxU32 facePartIndex = (physx::PxU32)*hMesh.partIndex(faceChunkIndex);
	const physx::PxU32 facePartTriangleCount = hMesh.meshTriangleCount(facePartIndex);
	const physx::NxExplicitRenderTriangle* facePartTriangles = hMesh.meshTriangles(facePartIndex);
	for (physx::PxU32 triN = 0; triN < facePartTriangleCount; ++triN)
	{
		const physx::NxExplicitRenderTriangle& tri = facePartTriangles[triN];
		physx::PxVec3 triNormal = (tri.vertices[1].position - tri.vertices[0].position).cross(tri.vertices[2].position - tri.vertices[0].position);
		const physx::PxF32 triArea = triNormal.normalize();	// Actually twice the area, but it's OK
		const physx::PxF32 triFaceDiff = (faceNormal-triNormal).magnitude();
		if (triFaceDiff < uvMapTriangleIndexFaceDiff - faceDiffTolerance || (triFaceDiff < uvMapTriangleIndexFaceDiff + faceDiffTolerance && triArea > uvMapTriangleIndexArea))
		{	// Significantly better normal, or normal is close and the area is bigger
			uvMapTriangleIndex = triN;
			uvMapTriangleIndexFaceDiff = triFaceDiff;
			uvMapTriangleIndexArea = triArea;
		}
	}

	// Set up interpolation for UV channel 0
	physx::TriangleFrame uvMapTriangleFrame(facePartTriangles[uvMapTriangleIndex], (physx::PxU64)1<<physx::TriangleFrame::UV0_u | (physx::PxU64)1<<physx::TriangleFrame::UV0_v);

	if (cutoutSet.isPeriodic())
	{
		--ixStart;
		++ixStop;
		--iyStart;
		++iyStop;
	}

#define FORCE_INSTANCING 0
#if !FORCE_INSTANCING
	const physx::PxF32 volumeTol = physx::PxMax(localExtents[0]*localExtents[1]*localExtents[2]*MESH_INSTANACE_TOLERANCE*MESH_INSTANACE_TOLERANCE*MESH_INSTANACE_TOLERANCE, (physx::PxF32)1.0e-15);
//	const physx::PxF32 areaTol = physx::PxMax((localExtents[0]*localExtents[1]+localExtents[1]*localExtents[2]+localExtents[2]*localExtents[0])*MESH_INSTANACE_TOLERANCE*MESH_INSTANACE_TOLERANCE, (physx::PxF32)1.0e-10);
#endif

	const bool instanceCongruentChunks = desc.instancingMode == NxFractureCutoutDesc::InstanceCongruentChunks || desc.instancingMode == NxFractureCutoutDesc::InstanceAllChunks;

	// Estimate total work for progress
	physx::PxU32 totalWork = 0;
	for (physx::PxU32 i = 0; i < cutoutSet.cutouts.size(); ++i)
	{
		totalWork += cutoutSet.cutouts[i].convexLoops.size();
	}
	totalWork *= (ixStop-ixStart)*(iyStop-iyStart);
	physx::HierarchicalProgressListener localProgressListener(physx::PxMax((int)totalWork, 1), &progressListener);

	physx::Array<physx::PxU32> unhandledChunks;

	if (cutoutDepth == 0.0f)
	{
		cutoutDepth = 2*localExtents[2];	// handle special case of all-the-way-through cutout.  cutoutDepth is only used for noise grid calculations
	}

	const unsigned cutoutChunkDepth = hMesh.depth(faceChunkIndex) + 1;

	// Loop over cutouts on the outside loop.  For each cutout, create all tiled (potential) clones
	for (physx::PxU32 i = 0; i < cutoutSet.cutouts.size() && !canceled; ++i)
	{
		// Keep track of starting chunk count.  We will process the newly created chunks below.
		const physx::PxU32 oldChunkCount = hMesh.chunkCount();

		for (physx::PxI32 iy = iyStart; iy < iyStop && !canceled; ++iy)
		{
			for (physx::PxI32 ix = ixStart; ix < ixStop && !canceled; ++ix)
			{
				physx::PxVec3 offset = (physx::PxF32)ix*mapWidth*xDir + (physx::PxF32)iy*mapHeight*yDir;
				physx::NxVertex interpolation;
				interpolation.position = offset;
				uvMapTriangleFrame.interpolateVertexData(interpolation);
				// BRG - note bizarre need to flip v...
				physx::PxVec2 uvOffset(interpolation.uv[0].u, -interpolation.uv[0].v);
				const physx::PxMat34Legacy offsetCutoutTM(cutoutTM.M, cutoutTM.t + offset);
				const physx::PxU32 newChunkIndex = hMesh.chunkCount();
				canceled = !createCutout(hMesh, faceChunkIndex, faceBSP, cutoutSet.cutouts[i], offsetCutoutTM, desc, edgeNoise, cutoutDepth, materialDesc, getVolumeDesc(collisionDesc, cutoutChunkDepth), minPlane, maxPlane, localProgressListener, cancel);
				if (!canceled && instanceCongruentChunks && newChunkIndex < hMesh.chunkCount())
				{
					PX_ASSERT(newChunkIndex + 1 == hMesh.chunkCount());
					if (newChunkIndex + 1 == hMesh.chunkCount())
					{
						hMesh.mChunks[newChunkIndex]->mInstancedPositionOffset = offset;
						hMesh.mChunks[newChunkIndex]->mInstancedUVOffset = uvOffset;
					}
				}
			}
		}

		// Keep track of which chunks we've checked for congruence
		const physx::PxU32 possibleCongruentChunkCount = hMesh.chunkCount()-oldChunkCount;

		unhandledChunks.resize(possibleCongruentChunkCount);
		physx::PxU32 index = hMesh.chunkCount();
		for (physx::PxU32 i = 0; i < possibleCongruentChunkCount; ++i)
		{
			unhandledChunks[i] = --index;
		}

		physx::PxU32 unhandledChunkCount = possibleCongruentChunkCount;
		while (unhandledChunkCount > 0)
		{
			// Have a fresh chunk to test for instancing.
			const physx::PxU32 chunkIndex = unhandledChunks[--unhandledChunkCount];
			ExplicitHierarchicalMesh::Chunk* chunk = hMesh.mChunks[chunkIndex];

			// Record its offset and rebase
			const physx::PxVec3 instancingBaseOffset = chunk->mInstancedPositionOffset;
			const physx::PxVec2 instancingBaseUVOffset = chunk->mInstancedUVOffset;
			chunk->mInstancedPositionOffset = physx::PxVec3(0.0f);
			chunk->mInstancedUVOffset = physx::PxVec2(0.0f);

			// If this option is selected, slice regions further
			switch (desc.chunkFracturingMethod)
			{
			case NxFractureCutoutDesc::SliceFractureCutoutChunks:
				{
					// Split hierarchically
					physx::PxPlane trailingPlanes[3];	// passing in depth = 0 will initialize these
					physx::PxPlane leadingPlanes[3];
#if 1	// Eliminating volume calculation here, for performance.  May introduce it later once the mesh is calculated.
					const physx::PxF32 bspVolume = 1.0f;
#else
					physx::PxF32 bspArea, bspVolume;
					chunkBSP->getSurfaceAreaAndVolume(bspArea, bspVolume, true);
#endif
					canceled = !hierarchicallySplitChunkInternal(hMesh, chunkIndex, 0, trailingPlanes, leadingPlanes, *hMesh.mParts[(physx::PxU32)chunk->mPartIndex]->mMeshBSP, bspVolume, sliceDesc, collisionDesc, localProgressListener, cancel);
				}
				break;
			case NxFractureCutoutDesc::VoronoiFractureCutoutChunks:
				{
					// Voronoi split
					cutoutVoronoiDesc.siteCount = createVoronoiSitesInsideMeshInternal(hMesh, &chunkIndex, 1, voronoiDesc.siteCount > 0 ? &perChunkSites[0] : NULL, NULL, voronoiDesc.siteCount, NULL, &gMicrogridSize, gMeshMode, progressListener );
					canceled = !voronoiSplitChunkInternal(hMesh, chunkIndex, *hMesh.mParts[(physx::PxU32)chunk->mPartIndex]->mMeshBSP, cutoutVoronoiDesc, collisionDesc, localProgressListener, cancel);
				}
				break;
			}

			// Now see if we can instance this chunk
			if (unhandledChunkCount > 0)
			{
				bool congruentChunkFound = false;
				physx::PxU32 testChunkCount = unhandledChunkCount;
				while (testChunkCount > 0)
				{
					const physx::PxU32 testChunkIndex = unhandledChunks[--testChunkCount];
					ExplicitHierarchicalMesh::Chunk* testChunk = hMesh.mChunks[testChunkIndex];
					const physx::PxU32 testPartIndex = (physx::PxU32)testChunk->mPartIndex;
					ExplicitHierarchicalMesh::Part* testPart = hMesh.mParts[testPartIndex];

					// Create a shifted BSP of the test chunk
					ApexCSG::IApexBSP* combinedBSP = createBSP(hMesh.mBSPMemCache);
					const physx::PxMat44 tm(physx::PxMat33::createIdentity(), instancingBaseOffset-testChunk->mInstancedPositionOffset);
					combinedBSP->copy(*testPart->mMeshBSP, tm);
					combinedBSP->combine(*hMesh.mParts[(physx::PxU32)chunk->mPartIndex]->mMeshBSP);
					physx::PxF32 xorArea, xorVolume;
					combinedBSP->getSurfaceAreaAndVolume(xorArea, xorVolume, true, ApexCSG::Operation::Exclusive_Or);
					combinedBSP->release();
					if (xorVolume <= volumeTol)
					{
						// XOR of the two volumes is nearly zero.  Consider these chunks to be congruent, and instance.
						congruentChunkFound = true;
						testChunk->mInstancedPositionOffset -= instancingBaseOffset;	// Correct offset
						testChunk->mInstancedUVOffset -= instancingBaseUVOffset;		// Correct offset
						testChunk->mFlags |= physx::apex::NxDestructibleAsset::ChunkIsInstanced;		// Set instance flag
						hMesh.removePart((physx::PxU32)testChunk->mPartIndex);	// Remove part for this chunk, since we'll be instancing another part
						testChunk->mPartIndex = chunk->mPartIndex;
						instanceChildren(hMesh, chunkIndex, testChunkIndex);	// Recursive
						--unhandledChunkCount;	// This chunk is handled now
						physx::swap(unhandledChunks[unhandledChunkCount],unhandledChunks[testChunkCount]);	// Keep the unhandled chunk array packed
					}
				}
				
				// If the chunk is instanced, then mark it so
				if (congruentChunkFound)
				{
					chunk->mFlags |= physx::apex::NxDestructibleAsset::ChunkIsInstanced;
				}
			}
		}

		// Second pass at cutout chunks
		for (physx::PxU32 j = 0; j < possibleCongruentChunkCount && !canceled; ++j)
		{
			ExplicitHierarchicalMesh::Chunk* chunk = hMesh.mChunks[oldChunkCount+j];
			if ((chunk->mFlags & physx::apex::NxDestructibleAsset::ChunkIsInstanced) == 0)
			{
				// This chunk will not be instanced.  Zero its offsets.
				chunk->mInstancedPositionOffset = physx::PxVec3(0.0f);
				chunk->mInstancedUVOffset = physx::PxVec2(0.0f);
			}
		}
	}

//	cutoutSource->release();

	return !canceled;
}

static bool cutoutFace
(
	physx::ExplicitHierarchicalMesh& hMesh,
	physx::Array<physx::PxPlane>& faceTrimPlanes,
	ApexCSG::IApexBSP* coreBSP,
	physx::PxU32& coreChunkIndex,	// This may be changed if the original core chunk is sliced away completely
	const physx::NxFractureCutoutDesc& desc,
	const physx::NxNoiseParameters& backfaceNoise,
	const physx::NxNoiseParameters& edgeNoise,
	const physx::NxFractureMaterialDesc& materialDesc,
	const physx::PxI32 fractureIndex,
	const physx::PxPlane& facePlane,
	const physx::NxCutoutSet& iCutoutSet,
	const physx::PxMat34Legacy& cutoutTM,
	const physx::PxF32 mapXLow,
	const physx::PxF32 mapYLow,
	const physx::PxBounds3& localBounds,
	const physx::PxF32 cutoutDepth,
	const physx::NxFractureSliceDesc& sliceDesc,
	const physx::NxFractureVoronoiDesc& voronoiDesc,
	const NxCollisionDesc& collisionDesc,
	physx::IProgressListener& progressListener,
	bool& stop,
	volatile bool* cancel
)
{
	physx::HierarchicalProgressListener localProgressListener(physx::PxMax((physx::PxI32)iCutoutSet.getCutoutCount(), 1), &progressListener);

	const physx::PxVec3 localExtents = localBounds.getExtents();
	const physx::PxVec3 localCenter = localBounds.getCenter();

	const physx::PxF32 sizeScale = physx::PxMax(physx::PxMax(localExtents.x, localExtents.y), localExtents.z);

	physx::PxU32 corePartIndex = (physx::PxU32)hMesh.mChunks[coreChunkIndex]->mPartIndex;

	const physx::PxU32 oldSize = hMesh.chunkCount();
	ApexCSG::IApexBSP* faceBSP = createBSP(hMesh.mBSPMemCache);	// face BSP defaults to all space
	physx::PxU32 faceChunkIndex = 0xFFFFFFFF;
	if (cutoutDepth > 0.0f)	// (depth = 0) => slice all the way through
	{
		physx::IntersectMesh grid;

		const physx::PxF32 mapWidth = cutoutTM.M.getColumn(0).magnitude()*iCutoutSet.getDimensions()[0];
		const physx::PxF32 mapHeight = cutoutTM.M.getColumn(1).magnitude()*iCutoutSet.getDimensions()[1];

		// Create faceBSP from grid
		GridParameters gridParameters;
		gridParameters.interiorSubmeshIndex = materialDesc.interiorSubmeshIndex;
		gridParameters.noise = backfaceNoise;
		gridParameters.level0Mesh = &hMesh.mParts[0]->mMesh;	// must be set each time, since this can move with array resizing
		gridParameters.sizeScale = sizeScale;
		if (desc.instancingMode != NxFractureCutoutDesc::DoNotInstance)
		{
			gridParameters.xPeriod = mapWidth;
			gridParameters.yPeriod = mapHeight;
		}
		// Create the slicing plane
		physx::PxPlane slicePlane = facePlane;
		slicePlane.d += cutoutDepth;
		gridParameters.materialFrameIndex = hMesh.addMaterialFrame();
		physx::NxMaterialFrame materialFrame = hMesh.getMaterialFrame(gridParameters.materialFrameIndex);
		materialFrame.buildCoordinateSystemFromMaterialDesc(materialDesc, slicePlane);
		materialFrame.mFractureMethod = physx::NxFractureMethod::Cutout;
		materialFrame.mFractureIndex = fractureIndex;
		materialFrame.mSliceDepth = hMesh.depth(coreChunkIndex) + 1;
		hMesh.setMaterialFrame(gridParameters.materialFrameIndex, materialFrame);
		gridParameters.triangleFrame.setFlat(materialFrame.mCoordinateSystem, materialDesc.uvScale, materialDesc.uvOffset);
		buildIntersectMesh(grid, slicePlane, materialFrame, (physx::PxI32)sliceDesc.noiseMode, &gridParameters);
		ApexCSG::BSPTolerances bspTolerances = ApexCSG::gDefaultTolerances;
		bspTolerances.linear = 0.00001f;
		bspTolerances.angular = 0.00001f;
		faceBSP->setTolerances(bspTolerances);
		ApexCSG::BSPBuildParameters bspBuildParams = gDefaultBuildParameters;
		bspBuildParams.rnd = &userRnd;
		bspBuildParams.internalTransform = coreBSP->getInternalTransform();
		faceBSP->fromMesh(&grid.m_triangles[0], grid.m_triangles.size(), bspBuildParams);
		coreBSP->combine(*faceBSP);
		faceBSP->op(*coreBSP, ApexCSG::Operation::A_Minus_B);
		coreBSP->op(*coreBSP, ApexCSG::Operation::Intersection);
		physx::PxU32 facePartIndex = hMesh.addPart();
		faceChunkIndex = hMesh.addChunk();
		hMesh.mChunks[faceChunkIndex]->mPartIndex = (physx::PxI32)facePartIndex;
		faceBSP->toMesh(hMesh.mParts[facePartIndex]->mMesh);
		physx::NxCollisionVolumeDesc volumeDesc = getVolumeDesc(collisionDesc, hMesh.depth(coreChunkIndex) + 1);
		if (hMesh.mParts[facePartIndex]->mMesh.size() != 0)
		{
			hMesh.mParts[facePartIndex]->mMeshBSP->copy(*faceBSP);
			hMesh.buildMeshBounds(facePartIndex);
			hMesh.buildCollisionGeometryForPart(facePartIndex, volumeDesc);
			if (desc.trimFaceCollisionHulls && (gridParameters.noise.amplitude != 0.0f || volumeDesc.mHullMethod != physx::NxConvexHullMethod::WRAP_GRAPHICS_MESH))
			{
				// Trim backface
				for (physx::PxU32 hullIndex = 0; hullIndex < hMesh.mParts[facePartIndex]->mCollision.size(); ++hullIndex)
				{
					physx::ConvexHull& hull = hMesh.mParts[facePartIndex]->mCollision[hullIndex]->impl;
					hull.intersectPlaneSide(physx::PxPlane(-slicePlane.n, -slicePlane.d));
					faceTrimPlanes.pushBack(slicePlane);
				}
			}
			hMesh.mChunks[faceChunkIndex]->mParentIndex = 0;
		}
		else
		{
			hMesh.removePart(facePartIndex);
			hMesh.removeChunk(faceChunkIndex);
			faceChunkIndex = 0xFFFFFFFF;
			facePartIndex = 0xFFFFFFFF;
		}
	}
	else
	{
		// Slicing goes all the way through
		faceBSP->copy(*coreBSP);
		if (oldSize == coreChunkIndex + 1)
		{
			// Core hasn't been split yet.  We don't want a copy of the original mesh at level 1, so remove it.
			hMesh.removePart(corePartIndex--);
			hMesh.removeChunk(coreChunkIndex--);
		}
		faceChunkIndex = coreChunkIndex;
		// This will break us out of both loops (only want to slice all the way through once):
		stop = true;
	}

	localProgressListener.setSubtaskWork(1);

	bool canceled = false;

	if (faceChunkIndex < hMesh.chunkCount())
	{
		// We have a face chunk.  Create cutouts
		canceled = !createFaceCutouts(hMesh, faceChunkIndex, *faceBSP, desc, edgeNoise, cutoutDepth, materialDesc, *(const physx::CutoutSet*)&iCutoutSet, cutoutTM, mapXLow, mapYLow, collisionDesc,
									  sliceDesc, voronoiDesc, facePlane, localCenter, localExtents, localProgressListener, cancel);
		// If there is anything left in the face, attach it as unfracturable
		// Volume rejection ratio, perhaps should be exposed
#if 0	// BRG - to do : better treatment of face leftover
		const physx::PxF32 volumeRejectionRatio = 0.0001f;
		if (faceBSP->getVolume() >= volumeRejectionRatio * faceVolumeEstimate)
		{
			const physx::PxU32 newPartIndex = hMesh.addPart();
			faceBSP->toMesh(hMesh.mParts[newPartIndex]->mMesh);
			if (hMesh.mParts[newPartIndex]->mMesh.size() != 0)
			{
				hMesh.mParts[newPartIndex]->mMeshBSP->copy(*faceBSP);
				hMesh.buildMeshBounds(newPartIndex);
				hMesh.mParts[newPartIndex]->mCollision.setEmpty();	// BRG - to do : better treatment of face leftover
				hMesh.mParts[newPartIndex]->mParentIndex = faceChunkIndex;
				chunkFlags.resize(hMesh.partCount(), 0);
			}
			else
			{
				hMesh.removePart(newPartIndex);
			}
		}
#endif

		localProgressListener.completeSubtask();
	}
	faceBSP->release();

	return !canceled;
}

bool createChippedMesh
(
    physx::NxExplicitHierarchicalMesh& iHMesh,
    const physx::NxMeshProcessingParameters& meshProcessingParams,
    const physx::NxFractureCutoutDesc& desc,
    const physx::NxCutoutSet& iCutoutSet,
    const physx::NxFractureSliceDesc& sliceDesc,
	const physx::NxFractureVoronoiDesc& voronoiDesc,
    const NxCollisionDesc& collisionDesc,
    physx::PxU32 randomSeed,
    physx::IProgressListener& progressListener,
    volatile bool* cancel
)
{
	physx::ExplicitHierarchicalMesh& hMesh = *(physx::ExplicitHierarchicalMesh*)&iHMesh;

	if (hMesh.partCount() == 0)
	{
		return false;
	}

	outputMessage("Chipping...");
	progressListener.setProgress(0);

	// Save state if cancel != NULL
	physx::PxFileBuf* save = NULL;
	class NullEmbedding : public physx::NxExplicitHierarchicalMesh::NxEmbedding
	{
		void	serialize(physx::PxFileBuf& stream, NxEmbedding::DataType type) const
		{
			(void)stream;
			(void)type;
		}
		void	deserialize(physx::PxFileBuf& stream, NxEmbedding::DataType type, physx::PxU32 version)
		{
			(void)stream;
			(void)type;
			(void)version;
		}
	} embedding;
	if (cancel != NULL)
	{
		save = physx::NxGetApexSDK()->createMemoryWriteStream();
		if (save != NULL)
		{
			hMesh.serialize(*save, embedding);
		}
	}

	hMesh.buildCollisionGeometryForPart(0, getVolumeDesc(collisionDesc, 0));

	userRnd.m_rnd.setSeed(randomSeed);

	if (hMesh.mParts[0]->mMeshBSP->getType() != ApexCSG::BSPType::Nontrivial)
	{
		outputMessage("Building mesh BSP...");
		progressListener.setProgress(0);
		hMesh.calculateMeshBSP(randomSeed, &progressListener, &meshProcessingParams.microgridSize, meshProcessingParams.meshMode);
		outputMessage("Mesh BSP completed.");
		userRnd.m_rnd.setSeed(randomSeed);
	}

	gIslandGeneration = meshProcessingParams.islandGeneration;
	gMicrogridSize = meshProcessingParams.microgridSize;
	gVerbosity = meshProcessingParams.verbosity;

	if (hMesh.mParts[0]->mBounds.isEmpty())
	{
		return false;	// Done, nothing in mesh
	}

	hMesh.clear(true);

	for (int i = 0; i < NxFractureCutoutDesc::DirectionCount; ++i)
	{
		if ((desc.directions >> i) & 1)
		{
			hMesh.mSubmeshData.resize(physx::PxMax(hMesh.mRootSubmeshCount, desc.cutoutParameters[i].materialDesc.interiorSubmeshIndex + 1));
		}
	}
	switch (desc.chunkFracturingMethod)
	{
	case NxFractureCutoutDesc::SliceFractureCutoutChunks:
		for (int i = 0; i < 3; ++i)
		{
			hMesh.mSubmeshData.resize(physx::PxMax(hMesh.mRootSubmeshCount, sliceDesc.materialDesc[i].interiorSubmeshIndex + 1));
		}
		break;
	case NxFractureCutoutDesc::VoronoiFractureCutoutChunks:
		hMesh.mSubmeshData.resize(physx::PxMax(hMesh.mRootSubmeshCount, voronoiDesc.materialDesc.interiorSubmeshIndex + 1));
		break;
	}

	// Count directions
	physx::PxU32 directionCount = 0;
	physx::PxU32 directions = desc.directions;
	while (directions)
	{
		directions = (directions - 1)&directions;
		++directionCount;
	}

	if (directionCount == 0 && desc.userDefinedDirection.isZero())	// directions = 0 is the way we invoke the user-supplied normal "UV-based" cutout fracturing
	{
		return true; // Done, no split directions
	}

	// Validate direction ordering
	bool dirUsed[NxFractureCutoutDesc::DirectionCount];
	memset(dirUsed, 0, PX_ARRAY_SIZE(dirUsed));
	for (physx::PxU32 dirIndex = 0; dirIndex < NxFractureCutoutDesc::DirectionCount; ++dirIndex)
	{
		// The direction must be one found in NxFractureCutoutDesc::Directions
		//    and must not be used twice, if it is enabled
		if ((directions  & desc.directionOrder[dirIndex]) &&
			(!isPowerOfTwo(desc.directionOrder[dirIndex])                  ||
			 desc.directionOrder[dirIndex] <= 0                            ||
			 desc.directionOrder[dirIndex] > NxFractureCutoutDesc::PositiveZ ||
			 dirUsed[lowestSetBit(desc.directionOrder[dirIndex])]))
		{
			outputMessage("Invalid direction ordering, each direction may be used just once, "
				          "and must correspond to a direction defined in NxFractureCutoutDesc::Directions.",
				physx::PxErrorCode::eINTERNAL_ERROR);
			return false;
		}
		dirUsed[dirIndex] = true;
	}

	physx::HierarchicalProgressListener localProgressListener(physx::PxMax((physx::PxI32)directionCount, 1), &progressListener);

	// Core starts as original mesh
	physx::PxU32 corePartIndex = hMesh.addPart();
	physx::PxU32 coreChunkIndex = hMesh.addChunk();
	hMesh.mParts[corePartIndex]->mMesh = hMesh.mParts[0]->mMesh;
	hMesh.buildMeshBounds(0);
	hMesh.mChunks[coreChunkIndex]->mParentIndex = 0;
	hMesh.mChunks[coreChunkIndex]->mPartIndex = (physx::PxI32)corePartIndex;

	ApexCSG::IApexBSP* coreBSP = createBSP(hMesh.mBSPMemCache);
	coreBSP->copy(*hMesh.mParts[0]->mMeshBSP);

	physx::Array<physx::PxPlane> faceTrimPlanes;

	const physx::PxBounds3& worldBounds = hMesh.mParts[0]->mBounds;
	const physx::PxVec3& extents = worldBounds.getExtents();
	const physx::PxVec3& center = worldBounds.getCenter();

	NxSliceParameters* sliceParametersAtDepth = (NxSliceParameters*)PxAlloca(sizeof(NxSliceParameters) * sliceDesc.maxDepth);

	bool canceled = false;
	bool stop = false;
	for (physx::PxU32 dirNum = 0; dirNum < NxFractureCutoutDesc::DirectionCount && !stop && !canceled; ++dirNum)
	{
		const physx::PxU32 sliceDirIndex = lowestSetBit(desc.directionOrder[dirNum]);
		physx::PxU32 sliceAxisNum, sliceSignNum;
		getCutoutSliceAxisAndSign(sliceAxisNum, sliceSignNum, sliceDirIndex);
		{
			if ((desc.directions >> sliceDirIndex) & 1)
			{
				physx::PxU32 sliceAxes[3];
				generateSliceAxes(sliceAxes, sliceAxisNum);

				localProgressListener.setSubtaskWork(1);

				physx::PxPlane facePlane;
				facePlane.n = physx::PxVec3(0, 0, 0);
				facePlane.n[sliceAxisNum] = sliceSignNum ? -1.0f : 1.0f;
				facePlane.d = -(facePlane.n[sliceAxisNum] * center[sliceAxisNum] + extents[sliceAxisNum]);	// coincides with depth = 0

				bool invertX;
				const physx::PxMat34Legacy cutoutTM = createCutoutFrame(center, extents, sliceAxes, sliceSignNum, desc, invertX);

				// Tiling bounds
				const physx::PxF32 mapWidth = cutoutTM.M.getColumn(0).magnitude()*iCutoutSet.getDimensions()[0];
				const physx::PxF32 mapXLow = cutoutTM.t[sliceAxes[0]] - ((invertX != desc.cutoutWidthInvert[sliceDirIndex])? mapWidth : 0.0f);
				const physx::PxF32 mapHeight = cutoutTM.M.getColumn(1).magnitude()*iCutoutSet.getDimensions()[1];
				const physx::PxF32 mapYLow = cutoutTM.t[sliceAxes[1]] - (desc.cutoutHeightInvert[sliceDirIndex] ? mapHeight : 0.0f);

				physx::PxBounds3 localBounds;
				for (int i = 0; i < 3; ++i)
				{
					localBounds.minimum[i] = worldBounds.minimum[sliceAxes[i]];
					localBounds.maximum[i] = worldBounds.maximum[sliceAxes[i]];
				}

				// Slice desc, if needed
				physx::NxFractureSliceDesc cutoutSliceDesc;
				// Create a sliceDesc based off of the GUI slice desc's X and Y components, applied to the
				// two axes appropriate for this cutout direction.
				cutoutSliceDesc = sliceDesc;
				cutoutSliceDesc.sliceParameters = sliceParametersAtDepth;
				for (unsigned depth = 0; depth < sliceDesc.maxDepth; ++depth)
				{
					cutoutSliceDesc.sliceParameters[depth] = sliceDesc.sliceParameters[depth];
				}
				for (physx::PxU32 axisN = 0; axisN < 3; ++axisN)
				{
					cutoutSliceDesc.targetProportions[sliceAxes[axisN]] = sliceDesc.targetProportions[axisN];
					for (physx::PxU32 depth = 0; depth < sliceDesc.maxDepth; ++depth)
					{
						cutoutSliceDesc.sliceParameters[depth].splitsPerPass[sliceAxes[axisN]] = sliceDesc.sliceParameters[depth].splitsPerPass[axisN];
						cutoutSliceDesc.sliceParameters[depth].linearVariation[sliceAxes[axisN]] = sliceDesc.sliceParameters[depth].linearVariation[axisN];
						cutoutSliceDesc.sliceParameters[depth].angularVariation[sliceAxes[axisN]] = sliceDesc.sliceParameters[depth].angularVariation[axisN];
						cutoutSliceDesc.sliceParameters[depth].noise[sliceAxes[axisN]] = sliceDesc.sliceParameters[depth].noise[axisN];
					}
				}

				canceled = !cutoutFace(hMesh, faceTrimPlanes, coreBSP, coreChunkIndex, desc, desc.cutoutParameters[sliceDirIndex].backfaceNoise, desc.cutoutParameters[sliceDirIndex].edgeNoise,
									   desc.cutoutParameters[sliceDirIndex].materialDesc, (physx::PxI32)sliceDirIndex, facePlane, iCutoutSet, cutoutTM, mapXLow, mapYLow, localBounds,
									   desc.cutoutParameters[sliceDirIndex].depth, cutoutSliceDesc, voronoiDesc, collisionDesc, localProgressListener, stop, cancel);

				localProgressListener.completeSubtask();
			}
		}
	}

	if (desc.directions == 0) // user-supplied normal "UV-based" cutout fracturing
	{
		localProgressListener.setSubtaskWork(1);

		// Create cutout transform from user's supplied mapping and direction
		const physx::PxVec3 userNormal = desc.userDefinedDirection.getNormalized();

		physx::PxMat34Legacy cutoutTM;
		cutoutTM.M.setColumn(0, desc.userUVMapping.column0/(physx::PxF32)desc.cutoutSizeX);
		cutoutTM.M.setColumn(1, desc.userUVMapping.column1/(physx::PxF32)desc.cutoutSizeY);
		cutoutTM.M.setColumn(2, userNormal);
		cutoutTM.t = desc.userUVMapping.column2;

		// Also create a local frame to get the local bounds for the mesh
		const physx::PxMat33 globalToLocal = physx::PxMat33(desc.userUVMapping.column0.getNormalized(), desc.userUVMapping.column1.getNormalized(), userNormal).getTranspose();

		physx::Array<physx::NxExplicitRenderTriangle>& mesh = hMesh.mParts[0]->mMesh;
		physx::PxBounds3 localBounds;
		localBounds.setEmpty();
		for (physx::PxU32 i = 0; i < mesh.size(); ++i)
		{
			physx::NxExplicitRenderTriangle& tri = mesh[i];
			for (int v = 0; v < 3; ++v)
			{
				localBounds.include(globalToLocal*tri.vertices[v].position);
			}
		}

		physx::PxPlane facePlane;
		facePlane.n = userNormal;
		facePlane.d = -localBounds.maximum[2];	// coincides with depth = 0

		// Tiling bounds
		const physx::PxVec3 localOrigin = globalToLocal*cutoutTM.t;
		const physx::PxF32 mapXLow = localOrigin[0];
		const physx::PxF32 mapYLow = localOrigin[1];

		canceled = !cutoutFace(hMesh, faceTrimPlanes, coreBSP, coreChunkIndex, desc, desc.userDefinedCutoutParameters.backfaceNoise, desc.userDefinedCutoutParameters.edgeNoise,
							   desc.userDefinedCutoutParameters.materialDesc, 6, facePlane, iCutoutSet, cutoutTM, mapXLow, mapYLow, localBounds, desc.userDefinedCutoutParameters.depth,
							   sliceDesc, voronoiDesc, collisionDesc, localProgressListener, stop, cancel);

		localProgressListener.completeSubtask();
	}

	if (!canceled && coreChunkIndex != 0)
	{
		coreBSP->toMesh(hMesh.mParts[corePartIndex]->mMesh);
		if (hMesh.mParts[corePartIndex]->mMesh.size() != 0)
		{
			hMesh.mParts[corePartIndex]->mMeshBSP->copy(*coreBSP);
			hMesh.buildCollisionGeometryForPart(coreChunkIndex, getVolumeDesc(collisionDesc, hMesh.depth(coreChunkIndex)));
			for (physx::PxU32 i = 0; i < faceTrimPlanes.size(); ++i)
			{
				for (physx::PxU32 hullIndex = 0; hullIndex < hMesh.mParts[corePartIndex]->mCollision.size(); ++hullIndex)
				{
					physx::ConvexHull& hull = hMesh.mParts[corePartIndex]->mCollision[hullIndex]->impl;
					hull.intersectPlaneSide(faceTrimPlanes[i]);
				}
			}
		}
		else
		{
			// Remove core mesh and chunk
			if (corePartIndex < hMesh.mParts.size())
			{
				hMesh.removePart(corePartIndex);
			}
			if (coreChunkIndex < hMesh.mChunks.size())
			{
				hMesh.removeChunk(coreChunkIndex);
			}
			coreChunkIndex = 0xFFFFFFFF;
			corePartIndex = 0xFFFFFFFF;
		}
	}

	coreBSP->release();

	// Restore if canceled
	if (canceled && save != NULL)
	{
		physx::PxU32 len;
		const void* mem = physx::NxGetApexSDK()->getMemoryWriteBuffer(*save, len);
		physx::PxFileBuf* load = physx::NxGetApexSDK()->createMemoryReadStream(mem, len);
		if (load != NULL)
		{
			hMesh.deserialize(*load, embedding);
			physx::NxGetApexSDK()->releaseMemoryReadStream(*load);
		}
	}

	if (save != NULL)
	{
		physx::NxGetApexSDK()->releaseMemoryReadStream(*save);
	}

	if (canceled)
	{
		return false;
	}

	if (meshProcessingParams.removeTJunctions)
	{
		MeshProcessor meshProcessor;
		for (physx::PxU32 i = 0; i < hMesh.partCount(); ++i)
		{
			meshProcessor.setMesh(hMesh.mParts[i]->mMesh, NULL, 0, 0.0001f*extents.magnitude());
			meshProcessor.removeTJunctions();
		}
	}

	hMesh.sortChunks();

	hMesh.createPartSurfaceNormals();

	if (desc.instancingMode == NxFractureCutoutDesc::InstanceAllChunks)
	{
		for (physx::PxU32 i = 0; i < hMesh.chunkCount(); ++i)
		{
			hMesh.mChunks[i]->mFlags |= physx::apex::NxDestructibleAsset::ChunkIsInstanced;
		}
	}

	outputMessage("chipping completed.");

	return true;
}

class VoronoiMeshSplitter : public MeshSplitter
{
private:
	VoronoiMeshSplitter& operator=(const VoronoiMeshSplitter&);

public:
	VoronoiMeshSplitter(const NxFractureVoronoiDesc& desc) : mDesc(desc)
	{
	}

	bool validate(physx::ExplicitHierarchicalMesh& hMesh)
	{
		if (hMesh.chunkCount() == 0)
		{
			return false;
		}

		if (mDesc.siteCount == 0)
		{
			return false;
		}

		return true;
	}

	void initialize(physx::ExplicitHierarchicalMesh& hMesh)
	{
		hMesh.mSubmeshData.resize(physx::PxMax(hMesh.mRootSubmeshCount, mDesc.materialDesc.interiorSubmeshIndex + 1));

		// Need to split out DM parameters
//		if (mDesc.useDisplacementMaps)
//		{
//			hMesh.initializeDisplacementMapVolume(mDesc);
//		}
	}

	bool process
	(
		physx::ExplicitHierarchicalMesh& hMesh,
		physx::PxU32 chunkIndex,
		const ApexCSG::IApexBSP& chunkBSP,
		const NxCollisionDesc& collisionDesc,
		physx::IProgressListener& progressListener,
		volatile bool* cancel
	)
	{
		return voronoiSplitChunkInternal(hMesh, chunkIndex, chunkBSP, mDesc, collisionDesc, progressListener, cancel);
	}

	bool finalize(physx::ExplicitHierarchicalMesh& hMesh)
	{
		if (mDesc.instanceChunks)
		{
			for (physx::PxU32 i = 0; i < hMesh.partCount(); ++i)
			{
				hMesh.mChunks[i]->mFlags |= physx::apex::NxDestructibleAsset::ChunkIsInstanced;
			}
		}

		return true;
	}

protected:
	const NxFractureVoronoiDesc& mDesc;
};

bool createVoronoiSplitMesh
(
	physx::NxExplicitHierarchicalMesh& iHMesh,
	physx::NxExplicitHierarchicalMesh& iHMeshCore,
	bool exportCoreMesh,
	physx::PxI32 coreMeshImprintSubmeshIndex,	// If this is < 0, use the core mesh materials (was applyCoreMeshMaterialToNeighborChunks).  Otherwise, use the given submesh.
	const NxMeshProcessingParameters& meshProcessingParams,
	const NxFractureVoronoiDesc& desc,
	const NxCollisionDesc& collisionDesc,
	physx::PxU32 randomSeed,
	physx::IProgressListener& progressListener,
	volatile bool* cancel
)
{
	VoronoiMeshSplitter splitter(desc);

	return splitMeshInternal(
		iHMesh,
		iHMeshCore,
		exportCoreMesh,
		coreMeshImprintSubmeshIndex,
		meshProcessingParams,
		splitter,
		collisionDesc,
		randomSeed,
		progressListener,
		cancel);
}

bool voronoiSplitChunk
(
	physx::NxExplicitHierarchicalMesh& iHMesh,
	physx::PxU32 chunkIndex,
	const FractureTools::NxMeshProcessingParameters& meshProcessingParams,
	const FractureTools::NxFractureVoronoiDesc& desc,
	const NxCollisionDesc& collisionDesc,
	physx::PxU32* randomSeed,
	IProgressListener& progressListener,
	volatile bool* cancel
)
{
	VoronoiMeshSplitter splitter(desc);

	return splitChunkInternal(iHMesh, chunkIndex, meshProcessingParams, splitter, collisionDesc, randomSeed, progressListener, cancel);
}

physx::PxU32 createVoronoiSitesInsideMesh
(
	physx::NxExplicitHierarchicalMesh& iHMesh,
	physx::PxVec3* siteBuffer,
	physx::PxU32* siteChunkIndices,
	physx::PxU32 siteCount,
	physx::PxU32* randomSeed,
	physx::PxU32* microgridSize,
	NxBSPOpenMode::Enum meshMode,
	physx::IProgressListener& progressListener,
	physx::PxU32 chunkIndex
)
{
	physx::ExplicitHierarchicalMesh& hMesh = *(physx::ExplicitHierarchicalMesh*)&iHMesh;

	physx::Array<PxU32> chunkList;

	if (hMesh.mChunks.size() == 0)
	{
		return 0;
	}

	if (chunkIndex >= hMesh.chunkCount())
	{
		// Find root-depth chunks
		for (physx::PxU32 chunkIndex = 0; chunkIndex < hMesh.chunkCount(); ++chunkIndex)
		{
			if (hMesh.mChunks[chunkIndex]->isRootLeafChunk())
			{
				chunkList.pushBack(chunkIndex);
			}
		}

		if (chunkList.size() > 0)
		{
			return createVoronoiSitesInsideMeshInternal(hMesh, &chunkList[0], chunkList.size(), siteBuffer, siteChunkIndices, siteCount, randomSeed, microgridSize, meshMode, progressListener);
		}

		return 0;	// This means we didn't find a root leaf chunk
	}

	return createVoronoiSitesInsideMeshInternal(hMesh, &chunkIndex, 1, siteBuffer, siteChunkIndices, siteCount, randomSeed, microgridSize, meshMode, progressListener);
}

// Defining these structs here, so as not to offend gnu's sensibilities
struct TriangleData
{
	physx::PxU16							chunkIndex;
	physx::PxVec3							triangleNormal;
	physx::PxF32							summedAreaWeight;
	const physx::NxExplicitRenderTriangle*	triangle;
};

struct InstanceInfo
{
	physx::PxU8		meshIndex;
	physx::PxI32	chunkIndex;	// Using a PxI32 so that the createIndexStartLookup can do its thing
	physx::PxMat44	relativeTransform;

	struct ChunkIndexLessThan
	{
		PX_INLINE bool operator()(const InstanceInfo& x, const InstanceInfo& y) const
		{
			return x.chunkIndex < y.chunkIndex;
		}
	};
};

physx::PxU32	createScatterMeshSites
(
	physx::PxU8*						meshIndices,
	physx::PxMat44*						relativeTransforms,
	physx::PxU32*						chunkMeshStarts,
	physx::PxU32						scatterMeshInstancesBufferSize,
	physx::NxExplicitHierarchicalMesh&	iHMesh,
	physx::PxU32						targetChunkCount,
	const physx::PxU16*					targetChunkIndices,
	physx::PxU32*						randomSeed,
	physx::PxU32						scatterMeshAssetCount,
	physx::NxRenderMeshAsset**			scatterMeshAssets,
	const physx::PxU32*					minCount,
	const physx::PxU32*					maxCount,
	const physx::PxF32*					minScales,
	const physx::PxF32*					maxScales,
	const physx::PxF32*					maxAngles
)
{
	physx::ExplicitHierarchicalMesh& hMesh = *(physx::ExplicitHierarchicalMesh*)&iHMesh;

	// Cap asset count to 1-byte range
	if (scatterMeshAssetCount > 255)
	{
		scatterMeshAssetCount = 255;
	}

	// Set random seed if requested
	if (randomSeed != NULL)
	{
		userRnd.m_rnd.setSeed(*randomSeed);
	}

	// Counts for each scatter mesh asset
	physx::Array<physx::PxU32> counts(scatterMeshAssetCount, 0);

	// Create convex hulls for each scatter mesh and add up valid weights
	physx::Array<PxVec3> vertices;	// Reusing this array for convex hull building
	physx::Array<PartConvexHullProxy> hulls(scatterMeshAssetCount);
	physx::PxU32 scatterMeshInstancesRequested = 0;
	for (physx::PxU32 scatterMeshAssetIndex = 0; scatterMeshAssetIndex < scatterMeshAssetCount; ++scatterMeshAssetIndex)
	{
		hulls[scatterMeshAssetIndex].impl.setEmpty();
		const physx::NxRenderMeshAsset* rma = scatterMeshAssets[scatterMeshAssetIndex];
		if (rma != NULL)
		{
			vertices.resize(0);
			for (physx::PxU32 submeshIndex = 0; submeshIndex < rma->getSubmeshCount(); ++submeshIndex)
			{
				const physx::NxRenderSubmesh& submesh = rma->getSubmesh(submeshIndex);
				const physx::NxVertexBuffer& vertexBuffer = submesh.getVertexBuffer();
				if (vertexBuffer.getVertexCount() > 0)
				{
					const physx::NxVertexFormat& vertexFormat = vertexBuffer.getFormat();
					const physx::PxI32 posBufferIndex = vertexFormat.getBufferIndexFromID(vertexFormat.getSemanticID(physx::NxRenderVertexSemantic::POSITION));
					const physx::PxU32 oldVertexCount = vertices.size();
					vertices.resize(oldVertexCount + vertexBuffer.getVertexCount());
					if (!vertexBuffer.getBufferData(&vertices[oldVertexCount], physx::NxRenderDataFormat::FLOAT3, sizeof(physx::PxVec3), (physx::PxU32)posBufferIndex, 0, vertexBuffer.getVertexCount()))
					{
						vertices.resize(oldVertexCount);	// Operation failed, revert vertex array size
					}
				}
			}
			if (vertices.size() > 0)
			{
				physx::Array<physx::PxVec3> directions;
				physx::ConvexHull::createKDOPDirections(directions, physx::NxConvexHullMethod::USE_6_DOP);
				hulls[scatterMeshAssetIndex].impl.buildKDOP(&vertices[0], vertices.size(), sizeof(vertices[0]), &directions[0], directions.size());
				if (!hulls[scatterMeshAssetIndex].impl.isEmpty())
				{
					counts[scatterMeshAssetIndex] = (physx::PxU32)userRnd.m_rnd.getScaled((physx::PxF32)minCount[scatterMeshAssetIndex], (physx::PxF32)maxCount[scatterMeshAssetIndex] + 1.0f);
					scatterMeshInstancesRequested += counts[scatterMeshAssetIndex];
				}
			}
		}
	}

	// Cap at buffer size
	if (scatterMeshInstancesRequested > scatterMeshInstancesBufferSize)
	{
		scatterMeshInstancesRequested = scatterMeshInstancesBufferSize;
	}

	// Return if no instances requested
	if (scatterMeshInstancesRequested == 0)
	{
		return 0;
	}

	// Count the interior triangles in all of the target chunks, and add up their areas
	// Build an area-weighted lookup table for the various triangles (also reference the chunks)
	physx::Array<TriangleData> triangleTable;
	physx::PxF32 summedAreaWeight = 0.0f;
	for (physx::PxU32 chunkNum = 0; chunkNum < targetChunkCount; ++chunkNum)
	{
		const physx::PxU16 chunkIndex = targetChunkIndices[chunkNum];
		if (chunkIndex >= hMesh.chunkCount())
		{
			continue;
		}
		const physx::PxU32 partIndex = (physx::PxU32)*hMesh.partIndex(chunkIndex);
		const physx::NxExplicitRenderTriangle* triangles = hMesh.meshTriangles(partIndex);
		const physx::PxU32 triangleCount = hMesh.meshTriangleCount(partIndex);
		for (physx::PxU32 triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
		{
			const NxExplicitRenderTriangle& triangle = triangles[triangleIndex];
			if (triangle.extraDataIndex != 0xFFFFFFFF)	// See if this is an interior triangle
			{

				TriangleData& triangleData = triangleTable.insert();
				triangleData.chunkIndex = chunkIndex;
				triangleData.triangleNormal = triangle.calculateNormal();
				summedAreaWeight += triangleData.triangleNormal.normalize();
				triangleData.summedAreaWeight = summedAreaWeight;
				triangleData.triangle = &triangle;
			}
		}
	}

	// Normalize summed area table
	if (summedAreaWeight <= 0.0f)
	{
		return 0;	// Non-normalizable
	}
	const physx::PxF32 recipSummedAreaWeight = 1.0f/summedAreaWeight;
	for (physx::PxU32 triangleNum = 0; triangleNum < triangleTable.size()-1; ++triangleNum)
	{
		triangleTable[triangleNum].summedAreaWeight *= recipSummedAreaWeight;
	}
	triangleTable[triangleTable.size()-1].summedAreaWeight = 1.0f;	// Just to be sure

	// Reserve instance info
	physx::Array<InstanceInfo> instanceInfo;
	instanceInfo.reserve(scatterMeshInstancesRequested);

	// Add scatter meshes
	ApexCSG::IApexBSP* hullBSP = createBSP(hMesh.mBSPMemCache);
	if (hullBSP == NULL)
	{
		return 0;
	}

	physx::Array<physx::PxPlane> planes;	// Reusing this array for bsp building

	for (physx::PxU32 scatterMeshAssetIndex = 0; scatterMeshAssetIndex < scatterMeshAssetCount && instanceInfo.size() < scatterMeshInstancesRequested; ++scatterMeshAssetIndex)
	{
		bool success = true;
		for (physx::PxU32 count = 0; success && count < counts[scatterMeshAssetIndex] && instanceInfo.size() < scatterMeshInstancesRequested; ++count)
		{
			success = false;
			for (physx::PxU32 trial = 0; !success && trial < 1000; ++trial)
			{
				// Pick triangle
				const TriangleData* triangleData = NULL;
				const physx::PxF32 unitRndForTriangle = userRnd.m_rnd.getUnit();
				for (physx::PxU32 triangleNum = 0; triangleNum < triangleTable.size(); ++triangleNum)
				{
					if (triangleTable[triangleNum].summedAreaWeight > unitRndForTriangle)
					{
						triangleData = &triangleTable[triangleNum];
						break;
					}
				}
				if (triangleData == NULL)
				{
					continue;
				}

				// pick scale, angle, and position and build transform
				const physx::PxF32 scale = physx::PxExp(userRnd.m_rnd.getScaled(physx::PxLog(minScales[scatterMeshAssetIndex]), physx::PxLog(maxScales[scatterMeshAssetIndex])));
				const physx::PxF32 angle = (physx::PxPi/180.0f)*userRnd.m_rnd.getScaled(0.0f, maxAngles[scatterMeshAssetIndex]);
				// random position in triangle
				const NxVertex* vertices = triangleData->triangle->vertices;
				const physx::PxVec3 position = randomPositionInTriangle(vertices[0].position, vertices[1].position, vertices[2].position, userRnd.m_rnd);
				physx::PxVec3 zAxis = triangleData->triangleNormal;
				// Rotate z axis into arbitrary vector in triangle plane
				physx::PxVec3 para = vertices[1].position - vertices[0].position;
				if (para.normalize() > 0.0f)
				{
					physx::PxF32 cosPhi, sinPhi;
					physx::sincos(angle, sinPhi, cosPhi);
					zAxis = cosPhi*zAxis + sinPhi*para;
				}
				physx::PxMat44 tm = randomRotationMatrix(zAxis, userRnd.m_rnd);
				tm.setPosition(position);
				tm.scale(physx::PxVec4(physx::PxVec3(scale), 1.0f));

				const physx::PxI32 parentIndex = *hMesh.parentIndex(triangleData->chunkIndex);
				if (parentIndex >= 0)
				{
					const physx::PxU32 parentPartIndex = (physx::PxU32)*hMesh.partIndex((physx::PxU32)parentIndex);
					ApexCSG::IApexBSP* parentPartBSP = hMesh.mParts[parentPartIndex]->mMeshBSP;
					if (parentPartBSP != NULL)
					{
						// Create BSP from hull and transform
						PartConvexHullProxy& hull = hulls[scatterMeshAssetIndex];
						planes.resize(hull.impl.getPlaneCount());
						for (physx::PxU32 planeIndex = 0; planeIndex < hull.impl.getPlaneCount(); ++planeIndex)
						{
							planes[planeIndex] = hull.impl.getPlane(planeIndex);
						}
						hullBSP->fromConvexPolyhedron(&planes[0], planes.size(), parentPartBSP->getInternalTransform());
						hullBSP->copy(*hullBSP, tm);

						// Now combine with chunk parent bsp, and see if the mesh hull bsp lies within the parent bsp
						hullBSP->combine(*hMesh.mParts[parentPartIndex]->mMeshBSP);
						hullBSP->op(*hullBSP, ApexCSG::Operation::A_Minus_B);
						if (hullBSP->getType() == ApexCSG::BSPType::Empty_Set)	// True if the hull lies entirely within the parent chunk
						{
							success = true;
							InstanceInfo& info = instanceInfo.insert();
							info.meshIndex = (physx::PxU8)scatterMeshAssetIndex;
							info.chunkIndex = (physx::PxI32)triangleData->chunkIndex;
							info.relativeTransform = tm;
						}
					}
				}
			}
		}
	}

	hullBSP->release();

	// Now sort the instance info by chunk index
	if (instanceInfo.size() > 1)
	{
		physx::sort<InstanceInfo, InstanceInfo::ChunkIndexLessThan>(instanceInfo.begin(), instanceInfo.size(), InstanceInfo::ChunkIndexLessThan());
	}

	// Write the info to the output arrays
	for (physx::PxU32 instanceNum = 0; instanceNum < instanceInfo.size() && instanceNum < scatterMeshInstancesBufferSize; ++instanceNum)	// Second condition instanceNum < scatterMeshInstancesBufferSize should not be necessary
	{
		const InstanceInfo& info = instanceInfo[instanceNum];
		meshIndices[instanceNum] = info.meshIndex;
		relativeTransforms[instanceNum] = info.relativeTransform;
	}

	// Finally create an indexed lookup
	if (instanceInfo.size() > 0)
	{
		physx::Array<physx::PxU32> lookup;
		createIndexStartLookup(lookup, 0, hMesh.chunkCount(), &instanceInfo[0].chunkIndex, instanceInfo.size(), sizeof(InstanceInfo));

		// .. and copy it into the output lookup table
		for (physx::PxU32 chunkLookup = 0; chunkLookup <= hMesh.chunkCount(); ++chunkLookup)	// <= is intentional
		{
			chunkMeshStarts[chunkLookup] = lookup[chunkLookup];
		}
	}

	return instanceInfo.size();
}

PX_INLINE bool intersectPlanes(physx::PxVec3& pos, physx::PxVec3& dir, const physx::PxPlane& plane0, const physx::PxPlane& plane1)
{
	dir = plane0.n.cross(plane1.n);

	if(dir.normalize() < PX_EPS_F32)
	{
		return false;
	}

	pos = physx::PxVec3(0.0f);

	for (int iter = 3; iter--;)
	{
		// Project onto plane0:
		pos = plane0.project(pos);

		// Raycast to plane1:
		const physx::PxVec3 b = dir.cross(plane0.n);
		pos -= (plane1.distance(pos)/(b.dot(plane1.n)))*b;
	}

	return true;
}

PX_INLINE void renderConvex(physx::NxApexRenderDebug& debugRender, const physx::PxPlane* planes, physx::PxU32 planeCount, physx::PxU32 color, physx::PxF32 tolerance)
{
	debugRender.setCurrentColor(color);

	physx::Array<physx::PxVec3> endpoints;

	const physx::PxF32 tol2 = tolerance*tolerance;

	for (physx::PxU32 i = 0; i < planeCount; ++i)
	{
		// We'll be drawing polygons in this plane
		const physx::PxPlane& plane_i = planes[i];
		endpoints.resize(0);
		for (physx::PxU32 j = 0; j < planeCount; ++j)
		{
			if (j == i)
			{
				continue;
			}
			const physx::PxPlane& plane_j = planes[j];
			// Find potential edge from intersection if plane_i and plane_j
			physx::PxVec3 orig;
			physx::PxVec3 edgeDir;
			if (!intersectPlanes(orig, edgeDir, plane_i, plane_j))
			{
				continue;
			}
			physx::PxF32 minS = -PX_MAX_F32;
			physx::PxF32 maxS = PX_MAX_F32;
			bool intersectionFound = true;
			// Clip to planes
			for (physx::PxU32 k = 0; k < planeCount; ++k)
			{
				if (k == i || i == j)
				{
					continue;
				}
				const physx::PxPlane& plane_k = planes[k];
				const physx::PxF32 num = -plane_k.distance(orig);
				const physx::PxF32 den = edgeDir.dot(plane_k.n);
				if (physx::PxAbs(den) > 10*PX_EPS_F32)
				{
					const physx::PxF32 s = num/den;
					if (den > 0.0f)
					{
						maxS = physx::PxMin(maxS, s);
					}
					else
					{
						minS = physx::PxMax(minS, s);
					}
					if (maxS <= minS)
					{
						intersectionFound = false;
						break;
					}
				}
				else
				if (num < -tolerance)
				{
					intersectionFound = false;
					break;
				}
			}
			if (intersectionFound)
			{
				endpoints.pushBack(orig + minS * edgeDir);
				endpoints.pushBack(orig + maxS * edgeDir);
			}
		}
		if (endpoints.size() > 2)
		{
			physx::Array<physx::PxVec3> verts;
			verts.pushBack(endpoints[endpoints.size()-2]);
			verts.pushBack(endpoints[endpoints.size()-1]);
			endpoints.popBack();
			endpoints.popBack();
			while (endpoints.size())
			{
				physx::PxU32 closestN = 0;
				physx::PxF32 closestDist2 = PX_MAX_F32;
				for (physx::PxU32 n = 0; n < endpoints.size(); ++n)
				{
					const physx::PxF32 dist2 = (endpoints[n] - verts[verts.size()-1]).magnitudeSquared();
					if (dist2 < closestDist2)
					{
						closestDist2 = dist2;
						closestN = n;
					}
				}
				if ((endpoints[closestN^1] - verts[0]).magnitudeSquared() < tol2)
				{
					break;
				}
				verts.pushBack(endpoints[closestN^1]);
				endpoints.replaceWithLast(closestN^1);
				endpoints.replaceWithLast(closestN);
			}
			if (verts.size() > 2)
			{
				if (((verts[1]-verts[0]).cross(verts[2]-verts[0])).dot(plane_i.n) < 0.0f)
				{
					for (physx::PxU32 n = verts.size()/2; n--;)
					{
						physx::swap(verts[n], verts[verts.size()-1-n]);
					}
				}
				debugRender.debugPolygon(verts.size(), &verts[0]);
			}
		}
	}
}

void visualizeVoronoiCells
(
	physx::NxApexRenderDebug& debugRender,
	const physx::PxVec3* sites,
	physx::PxU32 siteCount,
	const physx::PxU32* cellColors,
	physx::PxU32 cellColorCount,
	const physx::PxBounds3& bounds,
	physx::PxU32 cellIndex /* = 0xFFFFFFFF */
)
{
	// Rendering tolerance
	const physx::PxF32 tolerance = 1.0e-5f*bounds.getDimensions().magnitude();

	// Whether or not to use cellColors
	const bool useCellColors = cellColors != NULL && cellColorCount > 0;

	// Whether to draw a single cell or all cells
	const bool drawSingleCell = cellIndex < siteCount;

	// Create bound planes
	physx::Array<physx::PxPlane> boundPlanes;
	boundPlanes.reserve(6);
	boundPlanes.pushBack(physx::PxPlane(-1.0f, 0.0f, 0.0f, bounds.minimum.x));
	boundPlanes.pushBack(physx::PxPlane(1.0f, 0.0f, 0.0f, -bounds.maximum.x));
	boundPlanes.pushBack(physx::PxPlane(0.0f, -1.0f, 0.0f, bounds.minimum.y));
	boundPlanes.pushBack(physx::PxPlane(0.0f, 1.0f, 0.0f, -bounds.maximum.y));
	boundPlanes.pushBack(physx::PxPlane(0.0f, 0.0f, -1.0f, bounds.minimum.z));
	boundPlanes.pushBack(physx::PxPlane(0.0f, 0.0f, 1.0f, -bounds.maximum.z));

	// Iterate over cells
	for (VoronoiCellPlaneIterator i(sites, siteCount, boundPlanes.begin(), boundPlanes.size(), drawSingleCell ? cellIndex : 0); i.valid(); i.inc())
	{
		const physx::PxU32 cellColor = useCellColors ? cellColors[i.cellIndex()%cellColorCount] : 0xFFFFFFFF;
		renderConvex(debugRender, i.cellPlanes(), i.cellPlaneCount(), cellColor, tolerance);
		if (drawSingleCell)
		{
			break;
		}
	}
}

bool buildSliceMesh
(
	physx::IntersectMesh& intersectMesh,
	physx::NxExplicitHierarchicalMesh& referenceMesh,
	const physx::PxPlane& slicePlane,
	const FractureTools::NxNoiseParameters& noiseParameters,
	physx::PxU32 randomSeed
)
{
	if( referenceMesh.chunkCount() == 0 )
	{
		return false;
	}

	physx::ExplicitHierarchicalMesh& hMesh = *(physx::ExplicitHierarchicalMesh*)&referenceMesh;

	GridParameters gridParameters;
	gridParameters.interiorSubmeshIndex = 0;
	gridParameters.noise = noiseParameters;
	const physx::PxU32 partIndex = (physx::PxU32)hMesh.mChunks[0]->mPartIndex;
	gridParameters.level0Mesh = &hMesh.mParts[partIndex]->mMesh;
	physx::PxVec3 extents = hMesh.mParts[partIndex]->mBounds.getExtents();
	gridParameters.sizeScale = physx::PxAbs(extents.x*slicePlane.n.x) + physx::PxAbs(extents.y*slicePlane.n.y) + physx::PxAbs(extents.z*slicePlane.n.z);
	gridParameters.materialFrameIndex = hMesh.addMaterialFrame();
	physx::NxMaterialFrame materialFrame = hMesh.getMaterialFrame(gridParameters.materialFrameIndex );
	physx::NxFractureMaterialDesc materialDesc;
	materialFrame.buildCoordinateSystemFromMaterialDesc(materialDesc, slicePlane);
	materialFrame.mFractureMethod = physx::NxFractureMethod::Unknown;	// This is only a slice preview
	hMesh.setMaterialFrame(gridParameters.materialFrameIndex, materialFrame);
	gridParameters.triangleFrame.setFlat(materialFrame.mCoordinateSystem, physx::PxVec2(1.0f), physx::PxVec2(0.0f));
	gridParameters.forceGrid = true;
	userRnd.m_rnd.setSeed(randomSeed);
	buildIntersectMesh(intersectMesh, slicePlane, materialFrame, 0, &gridParameters);

	return true;
}

} // namespace FractureTools

namespace physx
{
namespace apex
{

void buildCollisionGeometry(physx::Array<physx::PartConvexHullProxy*>& volumes, const NxCollisionVolumeDesc& desc,
	const physx::PxVec3* vertices, physx::PxU32 vertexCount, physx::PxU32 vertexByteStride,
	const physx::PxU32* indices, physx::PxU32 indexCount)
{
	NxConvexHullMethod::Enum hullMethod = desc.mHullMethod;

	do 
	{
		if (hullMethod == physx::NxConvexHullMethod::CONVEX_DECOMPOSITION)
		{
			resizeCollision(volumes, 0);

			CONVEX_DECOMPOSITION::ConvexDecomposition* decomposer = CONVEX_DECOMPOSITION::createConvexDecomposition();
			if (decomposer != NULL)
			{
				CONVEX_DECOMPOSITION::DecompDesc decompDesc;
				decompDesc.mCpercent = desc.mConcavityPercent;
	//TODO:JWR			decompDesc.mPpercent = desc.mMergeThreshold;
				decompDesc.mDepth = desc.mRecursionDepth;

				decompDesc.mVcount = vertexCount;
				decompDesc.mVertices = (physx::PxF32*)vertices;
				decompDesc.mTcount = indexCount / 3;
				decompDesc.mIndices = indices;

				physx::PxU32 hullCount = decomposer->performConvexDecomposition(decompDesc);
				resizeCollision(volumes, hullCount);
				for (physx::PxU32 hullIndex = 0; hullIndex < hullCount; ++hullIndex)
				{
					CONVEX_DECOMPOSITION::ConvexResult* result = decomposer->getConvexResult(hullIndex,false);
					volumes[hullIndex]->buildFromPoints(result->mHullVertices, result->mHullVcount, 3 * sizeof(physx::PxF32));
					if (volumes[hullIndex]->impl.isEmpty())
					{
						// fallback
						physx::Array<physx::PxVec3> directions;
						physx::ConvexHull::createKDOPDirections(directions, physx::NxConvexHullMethod::USE_26_DOP);
						volumes[hullIndex]->impl.buildKDOP(result->mHullVertices, result->mHullVcount, 3 * sizeof(physx::PxF32), directions.begin(), directions.size());
					}
				}
				decomposer->release();
			}

			if(volumes.size() > 0)
			{
				break;
			}

			// fallback
			hullMethod = physx::NxConvexHullMethod::WRAP_GRAPHICS_MESH;
		}

		resizeCollision(volumes, 1);

		if (hullMethod == physx::NxConvexHullMethod::WRAP_GRAPHICS_MESH)
		{
			volumes[0]->buildFromPoints(vertices, vertexCount, vertexByteStride);
			if (!volumes[0]->impl.isEmpty())
			{
				break;
			}

			// fallback
			hullMethod = physx::NxConvexHullMethod::USE_26_DOP;
		}

		physx::Array<physx::PxVec3> directions;
		physx::ConvexHull::createKDOPDirections(directions, hullMethod);
		volumes[0]->impl.buildKDOP(vertices, vertexCount, vertexByteStride, directions.begin(), directions.size());
	} while(0);

	// Reduce hulls
	for (physx::PxU32 hullIndex = 0; hullIndex < volumes.size(); ++hullIndex)
	{
		// First try uninflated, then try with inflation.  This may find a better reduction
		volumes[hullIndex]->reduceHull(desc.mMaxVertexCount, desc.mMaxEdgeCount, desc.mMaxFaceCount, false);
		volumes[hullIndex]->reduceHull(desc.mMaxVertexCount, desc.mMaxEdgeCount, desc.mMaxFaceCount, true);
	}
}


// Serialization of NxExplicitSubmeshData


void serialize(physx::PxFileBuf& stream, const NxExplicitSubmeshData& d)
{
	ApexSimpleString materialName(d.mMaterialName);
	apex::serialize(stream, materialName);
	stream << d.mVertexFormat.mWinding;
	stream << d.mVertexFormat.mHasStaticPositions;
	stream << d.mVertexFormat.mHasStaticNormals;
	stream << d.mVertexFormat.mHasStaticTangents;
	stream << d.mVertexFormat.mHasStaticBinormals;
	stream << d.mVertexFormat.mHasStaticColors;
	stream << d.mVertexFormat.mHasStaticSeparateBoneBuffer;
	stream << d.mVertexFormat.mHasStaticDisplacements;
	stream << d.mVertexFormat.mHasDynamicPositions;
	stream << d.mVertexFormat.mHasDynamicNormals;
	stream << d.mVertexFormat.mHasDynamicTangents;
	stream << d.mVertexFormat.mHasDynamicBinormals;
	stream << d.mVertexFormat.mHasDynamicColors;
	stream << d.mVertexFormat.mHasDynamicSeparateBoneBuffer;
	stream << d.mVertexFormat.mHasDynamicDisplacements;
	stream << d.mVertexFormat.mUVCount;
	stream << d.mVertexFormat.mBonesPerVertex;
}

void deserialize(physx::PxFileBuf& stream, physx::PxU32 apexVersion, physx::PxU32 meshVersion, NxExplicitSubmeshData& d)
{
	ApexSimpleString materialName;
	apex::deserialize(stream, apexVersion, materialName);
	physx::string::strcpy_s(d.mMaterialName, NxExplicitSubmeshData::MaterialNameBufferSize, materialName.c_str());

	if (apexVersion >= ApexStreamVersion::CleanupOfApexRenderMesh)
	{
		stream >> d.mVertexFormat.mWinding;
		stream >> d.mVertexFormat.mHasStaticPositions;
		stream >> d.mVertexFormat.mHasStaticNormals;
		stream >> d.mVertexFormat.mHasStaticTangents;
		stream >> d.mVertexFormat.mHasStaticBinormals;
		stream >> d.mVertexFormat.mHasStaticColors;
		stream >> d.mVertexFormat.mHasStaticSeparateBoneBuffer;
		if (meshVersion >= ExplicitHierarchicalMesh::DisplacementData)
			stream >> d.mVertexFormat.mHasStaticDisplacements;
		stream >> d.mVertexFormat.mHasDynamicPositions;
		stream >> d.mVertexFormat.mHasDynamicNormals;
		stream >> d.mVertexFormat.mHasDynamicTangents;
		stream >> d.mVertexFormat.mHasDynamicBinormals;
		stream >> d.mVertexFormat.mHasDynamicColors;
		stream >> d.mVertexFormat.mHasDynamicSeparateBoneBuffer;
		if (meshVersion >= ExplicitHierarchicalMesh::DisplacementData)
			stream >> d.mVertexFormat.mHasDynamicDisplacements;
		stream >> d.mVertexFormat.mUVCount;
		if (apexVersion < ApexStreamVersion::RemovedTextureTypeInformationFromVertexFormat)
		{
			// Dead data
			physx::PxU32 textureTypes[NxVertexFormat::MAX_UV_COUNT];
			for (physx::PxU32 i = 0; i < NxVertexFormat::MAX_UV_COUNT; ++i)
			{
				stream >> textureTypes[i];
			}
		}
		stream >> d.mVertexFormat.mBonesPerVertex;
	}
	else
	{
#if 0	// BRG - to do, implement conversion
		bool	hasPosition;
		bool	hasNormal;
		bool	hasTangent;
		bool	hasBinormal;
		bool	hasColor;
		physx::PxU32	numBonesPerVertex;
		physx::PxU32	uvCount;
		NxRenderCullMode::Enum winding = NxRenderCullMode::CLOCKWISE;

		// PH: assuming position and normal as the default dynamic flags
		physx::PxU32 dynamicFlags = NxVertexFormatFlag::POSITION | NxVertexFormatFlag::NORMAL;

		if (version >= ApexStreamVersion::AddedRenderCullModeToRenderMeshAsset)
		{
			//stream.readBuffer( &winding, sizeof(winding) );
			stream >> winding;
		}
		if (version >= ApexStreamVersion::AddedDynamicVertexBufferField)
		{
			stream >> dynamicFlags;
		}
		if (version >= ApexStreamVersion::AddingTextureTypeInformationToVertexFormat)
		{
			stream >> hasPosition;
			stream >> hasNormal;
			stream >> hasTangent;
			stream >> hasBinormal;
			stream >> hasColor;
			if (version >= ApexStreamVersion::RenderMeshAssetRedesign)
			{
				stream >> numBonesPerVertex;
			}
			else
			{
				bool hasBoneIndex;
				stream >> hasBoneIndex;
				numBonesPerVertex = hasBoneIndex ? 1 : 0;
			}
			stream >> uvCount;
			if (version < ApexStreamVersion::RemovedTextureTypeInformationFromVertexFormat)
			{
				// Dead data
				physx::PxU32 textureTypes[NxVertexFormat::MAX_UV_COUNT];
				for (physx::PxU32 i = 0; i < NxVertexFormat::MAX_UV_COUNT; ++i)
				{
					stream >> textureTypes[i];
				}
			}
		}
		else
		{
			physx::PxU32 data;
			stream >> data;
			hasPosition = (data & (1 << 8)) != 0;
			hasNormal = (data & (1 << 9)) != 0;
			hasTangent = (data & (1 << 10)) != 0;
			hasBinormal = (data & (1 << 11)) != 0;
			hasColor = (data & (1 << 12)) != 0;
			numBonesPerVertex = (data & (1 << 13)) != 0 ? 1 : 0;
			uvCount = data & 0xFF;
		}

		d.mVertexFormat.mWinding = winding;
		d.mVertexFormat.mHasStaticPositions = hasPosition;
		d.mVertexFormat.mHasStaticNormals = hasNormal;
		d.mVertexFormat.mHasStaticTangents = hasTangent;
		d.mVertexFormat.mHasStaticBinormals = hasBinormal;
		d.mVertexFormat.mHasStaticColors = hasColor;
		d.mVertexFormat.mHasStaticSeparateBoneBuffer = false;
		d.mVertexFormat.mHasDynamicPositions = (dynamicFlags & NxVertexFormatFlag::POSITION) != 0;
		d.mVertexFormat.mHasDynamicNormals = (dynamicFlags & NxVertexFormatFlag::NORMAL) != 0;
		d.mVertexFormat.mHasDynamicTangents = (dynamicFlags & NxVertexFormatFlag::TANGENT) != 0;
		d.mVertexFormat.mHasDynamicBinormals = (dynamicFlags & NxVertexFormatFlag::BINORMAL) != 0;
		d.mVertexFormat.mHasDynamicColors = (dynamicFlags & NxVertexFormatFlag::COLOR) != 0;
		d.mVertexFormat.mHasDynamicSeparateBoneBuffer = (dynamicFlags & NxVertexFormatFlag::SEPARATE_BONE_BUFFER) != 0;
		d.mVertexFormat.mUVCount = uvCount;
		d.mVertexFormat.mBonesPerVertex = numBonesPerVertex;

		if (version >= ApexStreamVersion::RenderMeshAssetRedesign)
		{
			physx::PxU32 customBufferCount;
			stream >> customBufferCount;
			for (physx::PxU32 i = 0; i < customBufferCount; i++)
			{
				physx::PxU32 stringLength;
				stream >> stringLength;
				PX_ASSERT(stringLength < 254);
				char buf[256];
				stream.read(buf, stringLength);
				buf[stringLength] = 0;
				physx::PxU32 format;
				stream >> format;
			}
		}
#endif
	}
}


// Serialization of physx::NxMaterialFrame


void serialize(physx::PxFileBuf& stream, const physx::NxMaterialFrame& f)
{
	// f.mCoordinateSystem
	PxMat34Legacy m34;
	m34.M.setColumn(0, f.mCoordinateSystem.column0.getXYZ());
	m34.M.setColumn(1, f.mCoordinateSystem.column1.getXYZ());
	m34.M.setColumn(2, f.mCoordinateSystem.column2.getXYZ());
	m34.t = f.mCoordinateSystem.getPosition();
	stream << m34.M(0, 0) << m34.M(0, 1) << m34.M(0, 2)
		<< m34.M(1, 0) << m34.M(1, 1) << m34.M(1, 2)
		<< m34.M(2, 0) << m34.M(2, 1) << m34.M(2, 2) << m34.t;

	// Other fields of f
	stream << f.mUVPlane << f.mUVScale << f.mUVOffset << f.mFractureMethod << f.mFractureIndex << f.mSliceDepth;
}

void deserialize(physx::PxFileBuf& stream, physx::PxU32 apexVersion, physx::PxU32 meshVersion, physx::NxMaterialFrame& f)
{
	PX_UNUSED(apexVersion);

	f.mSliceDepth = 0;

	if (meshVersion >= ExplicitHierarchicalMesh::ChangedMaterialFrameToIncludeFracturingMethodContext)	// First version in which this struct exists
	{
		// f.mCoordinateSystem
		PxMat34Legacy m34;
		stream >> m34.M(0, 0) >> m34.M(0, 1) >> m34.M(0, 2)
			>> m34.M(1, 0) >> m34.M(1, 1) >> m34.M(1, 2)
			>> m34.M(2, 0) >> m34.M(2, 1) >> m34.M(2, 2) >> m34.t;
		f.mCoordinateSystem.column0 = PxVec4(m34.M.getColumn(0), 0.0f);
		f.mCoordinateSystem.column1 = PxVec4(m34.M.getColumn(1), 0.0f);
		f.mCoordinateSystem.column2 = PxVec4(m34.M.getColumn(2), 0.0f);
		f.mCoordinateSystem.setPosition(m34.t);

		// Other fields of f
		stream >> f.mUVPlane >> f.mUVScale >> f.mUVOffset >> f.mFractureMethod >> f.mFractureIndex;

		if (meshVersion >= ExplicitHierarchicalMesh::AddedSliceDepthToMaterialFrame)
		{
			stream >> f.mSliceDepth;
		}
	}
}


// ExplicitHierarchicalMesh

ExplicitHierarchicalMesh::ExplicitHierarchicalMesh()
{
	mBSPMemCache = ApexCSG::createBSPMemCache();
	mRootSubmeshCount = 0;
}

ExplicitHierarchicalMesh::~ExplicitHierarchicalMesh()
{
	clear();
	mBSPMemCache->release();
}

physx::PxU32 ExplicitHierarchicalMesh::addPart()
{
	const physx::PxU32 index = mParts.size();
	mParts.insert();
	Part* part = PX_NEW(Part);
	part->mMeshBSP = createBSP(mBSPMemCache);
	mParts.back() = part;
	return index;
}

bool ExplicitHierarchicalMesh::removePart(physx::PxU32 index)
{
	if (index >= partCount())
	{
		return false;
	}

	for (physx::PxU32 i = 0; i < mChunks.size(); ++i)
	{
		if (mChunks[i]->mPartIndex == (physx::PxI32)index)
		{
			mChunks[i]->mPartIndex = -1;
		}
		else if (mChunks[i]->mPartIndex > (physx::PxI32)index)
		{
			--mChunks[i]->mPartIndex;
		}
	}

	delete mParts[index];
	mParts.remove(index);

	return true;
}

physx::PxU32 ExplicitHierarchicalMesh::addChunk()
{
	const physx::PxU32 index = mChunks.size();
	mChunks.insert();
	mChunks.back() = PX_NEW(Chunk);
	return index;
}

bool ExplicitHierarchicalMesh::removeChunk(physx::PxU32 index)
{
	if (index >= chunkCount())
	{
		return false;
	}

	for (physx::PxU32 i = 0; i < mChunks.size(); ++i)
	{
		if (mChunks[i]->mParentIndex == (physx::PxI32)index)
		{
			mChunks[i]->mParentIndex = -1;
		}
		else if (mChunks[i]->mParentIndex > (physx::PxI32)index)
		{
			--mChunks[i]->mParentIndex;
		}
	}

	delete mChunks[index];
	mChunks.remove(index);

	return true;
}

void ExplicitHierarchicalMesh::serialize(physx::PxFileBuf& stream, NxEmbedding& embedding) const
{
	stream << (physx::PxU32)ExplicitHierarchicalMesh::Current;
	stream << (physx::PxU32)ApexStreamVersion::Current;
	stream << mParts.size();
	for (physx::PxU32 i = 0; i < mParts.size(); ++i)
	{
		stream << mParts[i]->mBounds;
		apex::serialize(stream, mParts[i]->mMesh);
		stream.storeDword(mParts[i]->mCollision.size());
		for (physx::PxU32 j = 0; j < mParts[i]->mCollision.size(); ++j)
		{
			apex::serialize(stream, mParts[i]->mCollision[j]->impl);
		}
		if (mParts[i]->mMeshBSP != NULL)
		{
			stream << (physx::PxU32)1;
			mParts[i]->mMeshBSP->serialize(stream);
		}
		else
		{
			stream << (physx::PxU32)0;
		}
		stream << mParts[i]->mFlags;
	}
	stream << mChunks.size();
	for (physx::PxU32 i = 0; i < mChunks.size(); ++i)
	{
		stream << mChunks[i]->mParentIndex;
		stream << mChunks[i]->mFlags;
		stream << mChunks[i]->mPartIndex;
		stream << mChunks[i]->mInstancedPositionOffset;
		stream << mChunks[i]->mInstancedUVOffset;
		stream << mChunks[i]->mPrivateFlags;
	}
	apex::serialize(stream, mSubmeshData);
	apex::serialize(stream, mMaterialFrames);
	embedding.serialize(stream, NxEmbedding::MaterialLibrary);
	stream << mRootSubmeshCount;
}

void ExplicitHierarchicalMesh::deserialize(physx::PxFileBuf& stream, NxEmbedding& embedding)
{
	clear();

	physx::PxU32 meshStreamVersion;
	stream >> meshStreamVersion;
	physx::PxU32 apexStreamVersion;
	stream >> apexStreamVersion;

	if (meshStreamVersion < ExplicitHierarchicalMesh::RemovedExplicitHMesh_mMaxDepth)
	{
		physx::PxI32 maxDepth;
		stream >> maxDepth;
	}

	if (meshStreamVersion >= ExplicitHierarchicalMesh::InstancingData)
	{
		physx::PxU32 partCount;
		stream >> partCount;
		mParts.resize(partCount);
		for (physx::PxU32 i = 0; i < partCount; ++i)
		{
			mParts[i] = PX_NEW(Part);
			stream >> mParts[i]->mBounds;
			apex::deserialize(stream, apexStreamVersion, mParts[i]->mMesh);
			resizeCollision(mParts[i]->mCollision, stream.readDword());
			for (physx::PxU32 hullNum = 0; hullNum < mParts[i]->mCollision.size(); ++hullNum)
			{
				apex::deserialize(stream, apexStreamVersion, mParts[i]->mCollision[hullNum]->impl);
			}
			mParts[i]->mMeshBSP = createBSP(mBSPMemCache);
			physx::PxU32 createMeshBSP;
			stream >> createMeshBSP;
			if (createMeshBSP)
			{
				mParts[i]->mMeshBSP->deserialize(stream);
			}
			else
			{
				ApexCSG::BSPBuildParameters bspBuildParameters = gDefaultBuildParameters;
				bspBuildParameters.internalTransform = physx::PxMat44::createIdentity();
				bspBuildParameters.rnd = &userRnd;
				userRnd.m_rnd.setSeed(0);
				mParts[i]->mMeshBSP->fromMesh(&mParts[i]->mMesh[0], mParts[i]->mMesh.size(), bspBuildParameters);
			}
			if (meshStreamVersion >= ExplicitHierarchicalMesh::ReaddedFlagsToPart)
			{
				stream >> mParts[i]->mFlags;
			}
		}

		physx::PxU32 chunkCount;
		stream >> chunkCount;
		mChunks.resize(chunkCount);
		for (physx::PxU32 i = 0; i < chunkCount; ++i)
		{
			mChunks[i] = PX_NEW(Chunk);
			stream >> mChunks[i]->mParentIndex;
			stream >> mChunks[i]->mFlags;
			stream >> mChunks[i]->mPartIndex;
			stream >> mChunks[i]->mInstancedPositionOffset;
			if (meshStreamVersion >= ExplicitHierarchicalMesh::UVInstancingData)
			{
				stream >> mChunks[i]->mInstancedUVOffset;
			}
			if (meshStreamVersion >= ExplicitHierarchicalMesh::IntroducingChunkPrivateFlags)
			{
				stream >> mChunks[i]->mPrivateFlags;
			}
		}
	}
	else
	{
		if (meshStreamVersion >= ExplicitHierarchicalMesh::UsingExplicitPartContainers)
		{
			physx::PxU32 partCount;
			stream >> partCount;
			mParts.resize(partCount);
			mChunks.resize(partCount);
			for (physx::PxU32 i = 0; i < partCount; ++i)
			{
				mParts[i] = PX_NEW(Part);
				mChunks[i] = PX_NEW(Chunk);
				stream >> mChunks[i]->mParentIndex;
				if (meshStreamVersion >= ExplicitHierarchicalMesh::SerializingMeshBounds)
				{
					stream >> mParts[i]->mBounds;
				}
				apex::deserialize(stream, apexStreamVersion, mParts[i]->mMesh);
				if (meshStreamVersion < ExplicitHierarchicalMesh::SerializingMeshBounds)
				{
					buildMeshBounds(i);
				}
				if (meshStreamVersion >= ExplicitHierarchicalMesh::MultipleConvexHullsPerChunk)
				{
					resizeCollision(mParts[i]->mCollision, stream.readDword());
				}
				else
				{
					resizeCollision(mParts[i]->mCollision, 1);
				}
				for (physx::PxU32 hullNum = 0; hullNum < mParts[i]->mCollision.size(); ++hullNum)
				{
					apex::deserialize(stream, apexStreamVersion, mParts[i]->mCollision[hullNum]->impl);
				}
				if (meshStreamVersion >= ExplicitHierarchicalMesh::PerPartMeshBSPs)
				{
					mParts[i]->mMeshBSP = createBSP(mBSPMemCache);
					physx::PxU32 createMeshBSP;
					stream >> createMeshBSP;
					if (createMeshBSP)
					{
						mParts[i]->mMeshBSP->deserialize(stream);
					}
					else
					{
						ApexCSG::BSPBuildParameters bspBuildParameters = gDefaultBuildParameters;
						bspBuildParameters.internalTransform = physx::PxMat44::createIdentity();
						mParts[i]->mMeshBSP->fromMesh(&mParts[i]->mMesh[0], mParts[i]->mMesh.size(), bspBuildParameters);
					}
				}
				if (meshStreamVersion >= ExplicitHierarchicalMesh::AddedFlagsFieldToPart)
				{
					stream >> mChunks[i]->mFlags;
				}
			}
		}
		else
		{
			physx::Array<physx::PxI32> parentIndices;
			physx::Array< physx::Array< NxExplicitRenderTriangle > > meshes;
			physx::Array< ConvexHull > meshHulls;
			apex::deserialize(stream, apexStreamVersion, parentIndices);
			apex::deserialize(stream, apexStreamVersion, meshes);
			apex::deserialize(stream, apexStreamVersion, meshHulls);
			PX_ASSERT(parentIndices.size() == meshes.size() && meshes.size() == meshHulls.size());
			physx::PxU32 partCount = physx::PxMin(parentIndices.size(), physx::PxMin(meshes.size(), meshHulls.size()));
			mParts.resize(partCount);
			mChunks.resize(partCount);
			for (physx::PxU32 i = 0; i < partCount; ++i)
			{
				mParts[i] = PX_NEW(Part);
				mChunks[i] = PX_NEW(Chunk);
				mChunks[i]->mParentIndex = parentIndices[i];
				mParts[i]->mMesh = meshes[i];
				resizeCollision(mParts[i]->mCollision, 1);
				mParts[i]->mCollision[0]->impl = meshHulls[i];
				buildMeshBounds(i);
			}
		}
		for (physx::PxU32 i = 0; i < mChunks.size(); ++i)
		{
			mChunks[i]->mPartIndex = (physx::PxI32)i;
		}
	}

	if (meshStreamVersion >= ExplicitHierarchicalMesh::SerializingMeshBSP && meshStreamVersion < ExplicitHierarchicalMesh::PerPartMeshBSPs)
	{
		mParts[0]->mMeshBSP = createBSP(mBSPMemCache);
		physx::PxU32 createMeshBSP;
		stream >> createMeshBSP;
		if (createMeshBSP)
		{
			mParts[0]->mMeshBSP->deserialize(stream);
		}
		else
		{
			ApexCSG::BSPBuildParameters bspBuildParameters = gDefaultBuildParameters;
			bspBuildParameters.internalTransform = physx::PxMat44::createIdentity();
			mParts[0]->mMeshBSP->fromMesh(&mParts[0]->mMesh[0], mParts[0]->mMesh.size(), bspBuildParameters);
		}
	}

	if (meshStreamVersion >= ExplicitHierarchicalMesh::IncludingVertexFormatInSubmeshData)
	{
		apex::deserialize(stream, apexStreamVersion, meshStreamVersion, mSubmeshData);
	}
	else
	{
		physx::Array<ApexSimpleString> materialNames;
		apex::deserialize(stream, apexStreamVersion, materialNames);
		mSubmeshData.resize(0);	// Make sure the next resize calls constructors
		mSubmeshData.resize(materialNames.size());
		for (physx::PxU32 i = 0; i < materialNames.size(); ++i)
		{
			physx::string::strcpy_s(mSubmeshData[i].mMaterialName, NxExplicitSubmeshData::MaterialNameBufferSize, materialNames[i].c_str());
			mSubmeshData[i].mVertexFormat.mHasStaticPositions = true;
			mSubmeshData[i].mVertexFormat.mHasStaticNormals = true;
			mSubmeshData[i].mVertexFormat.mHasStaticTangents = true;
			mSubmeshData[i].mVertexFormat.mHasStaticBinormals = true;
			mSubmeshData[i].mVertexFormat.mHasStaticColors = true;
			mSubmeshData[i].mVertexFormat.mHasStaticDisplacements = false;
			mSubmeshData[i].mVertexFormat.mUVCount = 1;
			mSubmeshData[i].mVertexFormat.mBonesPerVertex = 1;
		}
	}

	if (meshStreamVersion >= ExplicitHierarchicalMesh::AddedMaterialFramesToHMesh_and_NoiseType_and_GridSize_to_Cleavage)
	{
		if (meshStreamVersion >= ExplicitHierarchicalMesh::ChangedMaterialFrameToIncludeFracturingMethodContext)
		{
			apex::deserialize(stream, apexStreamVersion, meshStreamVersion, mMaterialFrames);
		}
		else
		{
			const PxU32 size = stream.readDword();
			mMaterialFrames.resize(size);
			for (PxU32 i = 0; i < size; ++i)
			{
				PxMat34Legacy m34;
				stream >> m34.M(0, 0) >> m34.M(0, 1) >> m34.M(0, 2)
					>> m34.M(1, 0) >> m34.M(1, 1) >> m34.M(1, 2)
					>> m34.M(2, 0) >> m34.M(2, 1) >> m34.M(2, 2) >> m34.t;
				mMaterialFrames[i].mCoordinateSystem.column0 = PxVec4(m34.M.getColumn(0), 0.0f);
				mMaterialFrames[i].mCoordinateSystem.column1 = PxVec4(m34.M.getColumn(1), 0.0f);
				mMaterialFrames[i].mCoordinateSystem.column2 = PxVec4(m34.M.getColumn(2), 0.0f);
				mMaterialFrames[i].mCoordinateSystem.setPosition(m34.t);
				mMaterialFrames[i].mUVPlane = physx::PxPlane(m34.t, m34.M.getColumn(2));
				mMaterialFrames[i].mUVScale = physx::PxVec2(1.0f);
				mMaterialFrames[i].mUVOffset = physx::PxVec2(0.0f);
				mMaterialFrames[i].mFractureMethod = physx::NxFractureMethod::Unknown;
				mMaterialFrames[i].mFractureIndex = -1;
			}
		}
	}
	else
	{
		mMaterialFrames.resize(0);
	}

	if (meshStreamVersion >= ExplicitHierarchicalMesh::AddedMaterialLibraryToMesh)
	{
		embedding.deserialize(stream, NxEmbedding::MaterialLibrary, meshStreamVersion);
	}

	if (meshStreamVersion >= ExplicitHierarchicalMesh::AddedCacheChunkSurfaceTracesAndInteriorSubmeshIndex && meshStreamVersion < ExplicitHierarchicalMesh::RemovedInteriorSubmeshIndex)
	{
		physx::PxI32 interiorSubmeshIndex;
		stream >> interiorSubmeshIndex;
	}


	if (meshStreamVersion < ExplicitHierarchicalMesh::IntroducingChunkPrivateFlags)
	{
		physx::PxU32 rootDepth = 0;
		if (meshStreamVersion >= ExplicitHierarchicalMesh::PerPartMeshBSPs)
		{
			stream >> rootDepth;
		}

		for (physx::PxU32 i = 0; i < mChunks.size(); ++i)
		{
			mChunks[i]->mPrivateFlags = 0;
			const physx::PxU32 chunkDepth = depth(i);
			if (chunkDepth <= rootDepth)
			{
				mChunks[i]->mPrivateFlags |= Chunk::Root;
				if (chunkDepth == rootDepth)
				{
					mChunks[i]->mPrivateFlags |= Chunk::RootLeaf;
				}
			}
		}
	}

	if (meshStreamVersion >= ExplicitHierarchicalMesh::StoringRootSubmeshCount)
	{
		stream >> mRootSubmeshCount;
	}
	else
	{
		mRootSubmeshCount = mSubmeshData.size();
	}

	if (meshStreamVersion < ExplicitHierarchicalMesh::RemovedNxChunkAuthoringFlag)
	{
		/* Need to translate flags:
			IsCutoutFaceSplit =	(1U << 0),
			IsCutoutLeftover =	(1U << 1),
			Instance =			(1U << 31)
		*/
		for (physx::PxU32 chunkIndex = 0; chunkIndex < mChunks.size(); ++chunkIndex)
		{
			// IsCutoutFaceSplit and IsCutoutLeftover are no longer used.
			// Translate Instance flag:
			if (mChunks[chunkIndex]->mFlags & (1U << 31))
			{
				mChunks[chunkIndex]->mFlags = physx::apex::NxDestructibleAsset::ChunkIsInstanced;
			}
		}
	}
}

physx::PxI32 ExplicitHierarchicalMesh::maxDepth() const
{
	physx::PxI32 max = -1;
	physx::PxI32 index = (physx::PxI32)chunkCount()-1;
	while (index >= 0)
	{
		index = mChunks[(physx::PxU32)index]->mParentIndex;
		++max;
	}
	return max;
}

physx::PxU32 ExplicitHierarchicalMesh::partCount() const
{
	return mParts.size();
}

physx::PxU32 ExplicitHierarchicalMesh::chunkCount() const
{
	return mChunks.size();
}

physx::PxI32* ExplicitHierarchicalMesh::parentIndex(physx::PxU32 chunkIndex)
{
	return chunkIndex < chunkCount() ? &mChunks[chunkIndex]->mParentIndex : NULL;
}

physx::PxU64 ExplicitHierarchicalMesh::chunkUniqueID(physx::PxU32 chunkIndex)
{
	return chunkIndex < chunkCount() ? mChunks[chunkIndex]->getEUID() : (physx::PxU64)0;
}

physx::PxI32* ExplicitHierarchicalMesh::partIndex(physx::PxU32 chunkIndex)
{
	return chunkIndex < chunkCount() ? &mChunks[chunkIndex]->mPartIndex : NULL;
}

physx::PxVec3* ExplicitHierarchicalMesh::instancedPositionOffset(physx::PxU32 chunkIndex)
{
	return chunkIndex < chunkCount() ? &mChunks[chunkIndex]->mInstancedPositionOffset : NULL;
}

physx::PxVec2* ExplicitHierarchicalMesh::instancedUVOffset(physx::PxU32 chunkIndex)
{
	return chunkIndex < chunkCount() ? &mChunks[chunkIndex]->mInstancedUVOffset : NULL;
}

physx::PxU32 ExplicitHierarchicalMesh::depth(physx::PxU32 chunkIndex) const
{
	if (chunkIndex >= mChunks.size())
	{
		return 0;
	}

	PxU32 depth = 0;
	PxI32 index = (PxI32)chunkIndex;
	while ((index = mChunks[(physx::PxU32)index]->mParentIndex) >= 0)
	{
		++depth;
	}

	return depth;
}

physx::PxU32 ExplicitHierarchicalMesh::meshTriangleCount(physx::PxU32 partIndex) const
{
	return partIndex < partCount() ? mParts[partIndex]->mMesh.size() : 0;
}

NxExplicitRenderTriangle* ExplicitHierarchicalMesh::meshTriangles(physx::PxU32 partIndex)
{
	return partIndex < partCount() ? mParts[partIndex]->mMesh.begin() : NULL;
}

physx::PxBounds3 ExplicitHierarchicalMesh::meshBounds(physx::PxU32 partIndex) const
{
	physx::PxBounds3 bounds;
	bounds.setEmpty();
	if (partIndex < partCount())
	{
		bounds = mParts[partIndex]->mBounds;
	}
	return bounds;
}

physx::PxBounds3 ExplicitHierarchicalMesh::chunkBounds(physx::PxU32 chunkIndex) const
{
	physx::PxBounds3 bounds;
	bounds.setEmpty();

	if (chunkIndex < chunkCount())
	{
		bounds = mParts[(physx::PxU32)mChunks[chunkIndex]->mPartIndex]->mBounds;
		bounds.minimum += mChunks[chunkIndex]->mInstancedPositionOffset;
		bounds.maximum += mChunks[chunkIndex]->mInstancedPositionOffset;
	}
	return bounds;
}

physx::PxU32* ExplicitHierarchicalMesh::chunkFlags(physx::PxU32 chunkIndex) const
{
	if (chunkIndex < chunkCount())
	{
		return &mChunks[chunkIndex]->mFlags;
	}
	return NULL;
}

physx::PxU32 ExplicitHierarchicalMesh::convexHullCount(physx::PxU32 partIndex) const
{
	if (partIndex < partCount())
	{
		return mParts[partIndex]->mCollision.size();
	}
	return 0;
}

const ExplicitHierarchicalMesh::NxConvexHull** ExplicitHierarchicalMesh::convexHulls(physx::PxU32 partIndex) const
{
	if (partIndex < partCount())
	{
		Part* part = mParts[partIndex];
		return part->mCollision.size() > 0 ? (const ExplicitHierarchicalMesh::NxConvexHull**)&part->mCollision[0] : NULL;
	}
	return NULL;
}

physx::PxVec3* ExplicitHierarchicalMesh::surfaceNormal(physx::PxU32 partIndex)
{
	if (partIndex < partCount())
	{
		Part* part = mParts[partIndex];
		return &part->mSurfaceNormal;
	}
	return NULL;
}

const NxDisplacementMapVolume& ExplicitHierarchicalMesh::displacementMapVolume() const
{
	return mDisplacementMapVolume;
}

physx::PxU32 ExplicitHierarchicalMesh::submeshCount() const
{
	return mSubmeshData.size();
}

NxExplicitSubmeshData* ExplicitHierarchicalMesh::submeshData(physx::PxU32 submeshIndex)
{
	return submeshIndex < mSubmeshData.size() ? mSubmeshData.begin() + submeshIndex : NULL;
}

physx::PxU32 ExplicitHierarchicalMesh::addSubmesh(const NxExplicitSubmeshData& submeshData)
{
	const physx::PxU32 index = mSubmeshData.size();
	mSubmeshData.pushBack(submeshData);
	return index;
}

physx::PxU32 ExplicitHierarchicalMesh::getMaterialFrameCount() const
{
	return mMaterialFrames.size();
}

physx::NxMaterialFrame ExplicitHierarchicalMesh::getMaterialFrame(physx::PxU32 index) const
{
	return mMaterialFrames[index];
}

void ExplicitHierarchicalMesh::setMaterialFrame(physx::PxU32 index, const physx::NxMaterialFrame& materialFrame)
{
	mMaterialFrames[index] = materialFrame;
}

physx::PxU32 ExplicitHierarchicalMesh::addMaterialFrame()
{
	mMaterialFrames.insert();
	return mMaterialFrames.size()-1;
}

void ExplicitHierarchicalMesh::clear(bool keepRoot)
{
	physx::PxU32 newPartCount = 0;
	physx::PxU32 index = chunkCount();
	while (index-- > 0)
	{
		if (!keepRoot || !mChunks[index]->isRootChunk())
		{
			removeChunk(index);
		}
		else
		{
			newPartCount = physx::PxMax(newPartCount, (physx::PxU32)(mChunks[index]->mPartIndex+1));
		}
	}

	while (newPartCount < partCount())
	{
		removePart(partCount()-1);
	}

	mMaterialFrames.resize(0);

	if (!keepRoot)
	{
		mSubmeshData.reset();
		mBSPMemCache->clearAll();
		mRootSubmeshCount = 0;
	}
}

void ExplicitHierarchicalMesh::sortChunks(physx::Array<physx::PxU32>* indexRemap)
{
	if (mChunks.size() <= 1)
	{
		return;
	}

	// Sort by original parent index
	physx::Array<ChunkIndexer> chunkIndices(mChunks.size());
	for (physx::PxU32 i = 0; i < mChunks.size(); ++i)
	{
		chunkIndices[i].chunk = mChunks[i];
		chunkIndices[i].parentIndex = mChunks[i]->mParentIndex;
		chunkIndices[i].index = (physx::PxI32)i;
	}
	qsort(chunkIndices.begin(), chunkIndices.size(), sizeof(ChunkIndexer), ChunkIndexer::compareParentIndices);

	// Now arrange in depth order
	physx::Array<physx::PxU32> parentStarts;
	createIndexStartLookup(parentStarts, -1, chunkIndices.size() + 1, &chunkIndices[0].parentIndex, chunkIndices.size(), sizeof(ChunkIndexer));

	physx::Array<ChunkIndexer> newChunkIndices;
	newChunkIndices.reserve(mChunks.size());
	physx::PxI32 parentIndex = -1;
	physx::PxU32 nextPart = 0;
	while (newChunkIndices.size() < mChunks.size())
	{
		const physx::PxU32 start = parentStarts[(physx::PxU32)parentIndex + 1];
		const physx::PxU32 stop = parentStarts[(physx::PxU32)parentIndex + 2];
		for (physx::PxU32 index = start; index < stop; ++index)
		{
			newChunkIndices.pushBack(chunkIndices[index]);
		}
		parentIndex = newChunkIndices[nextPart++].index;
	}

	// Remap the parts and parent indices
	physx::Array<physx::PxU32> internalRemap;
	physx::Array<physx::PxU32>& remap = indexRemap != NULL ? *indexRemap : internalRemap;
	remap.resize(newChunkIndices.size());
	for (physx::PxU32 i = 0; i < newChunkIndices.size(); ++i)
	{
		mChunks[i] = newChunkIndices[i].chunk;
		remap[(physx::PxU32)newChunkIndices[i].index] = i;
	}
	for (physx::PxU32 i = 0; i < mChunks.size(); ++i)
	{
		if (mChunks[i]->mParentIndex >= 0)
		{
			mChunks[i]->mParentIndex = (physx::PxI32)remap[(physx::PxU32)mChunks[i]->mParentIndex];
		}
	}
}

void ExplicitHierarchicalMesh::createPartSurfaceNormals()
{
	for (physx::PxU32 partIndex = 0; partIndex < mParts.size(); ++partIndex)
	{
		Part* part = mParts[partIndex];
		physx::Array<NxExplicitRenderTriangle>& mesh = part->mMesh;
		physx::PxVec3 normal(0.0f);
		for (physx::PxU32 triangleIndex = 0; triangleIndex < mesh.size(); ++triangleIndex)
		{
			NxExplicitRenderTriangle& triangle = mesh[triangleIndex];
			if (triangle.extraDataIndex == 0xFFFFFFFF)
			{
				normal += (triangle.vertices[1].position - triangle.vertices[0].position).cross(triangle.vertices[2].position - triangle.vertices[0].position);
			}
		}
		part->mSurfaceNormal = normal.getNormalized();
	}
}

void ExplicitHierarchicalMesh::set(const NxExplicitHierarchicalMesh& mesh)
{
	const ExplicitHierarchicalMesh& m = (const ExplicitHierarchicalMesh&)mesh;
	clear();
	mParts.resize(0);
	mParts.reserve(m.mParts.size());
	for (physx::PxU32 i = 0; i < m.mParts.size(); ++i)
	{
		const physx::PxU32 newPartIndex = addPart();
		PX_ASSERT(newPartIndex == i);
		mParts[newPartIndex]->mBounds = m.mParts[i]->mBounds;
		mParts[newPartIndex]->mMesh = m.mParts[i]->mMesh;
		PX_ASSERT(m.mParts[i]->mMeshBSP != NULL);
		mParts[newPartIndex]->mMeshBSP->copy(*m.mParts[i]->mMeshBSP);
		resizeCollision(mParts[newPartIndex]->mCollision, m.mParts[i]->mCollision.size());
		for (physx::PxU32 j = 0; j < mParts[newPartIndex]->mCollision.size(); ++j)
		{
			mParts[newPartIndex]->mCollision[j]->impl = m.mParts[i]->mCollision[j]->impl;
		}
		mParts[newPartIndex]->mFlags = m.mParts[i]->mFlags;
	}
	mChunks.resize(m.mChunks.size());
	for (physx::PxU32 i = 0; i < mChunks.size(); ++i)
	{
		mChunks[i] = PX_NEW(Chunk);
		mChunks[i]->mParentIndex = m.mChunks[i]->mParentIndex;
		mChunks[i]->mFlags = m.mChunks[i]->mFlags;
		mChunks[i]->mPartIndex = m.mChunks[i]->mPartIndex;
		mChunks[i]->mInstancedPositionOffset = m.mChunks[i]->mInstancedPositionOffset;
		mChunks[i]->mInstancedUVOffset = m.mChunks[i]->mInstancedUVOffset;
		mChunks[i]->mPrivateFlags = m.mChunks[i]->mPrivateFlags;
	}
	mSubmeshData = m.mSubmeshData;
	mMaterialFrames = m.mMaterialFrames;
	mRootSubmeshCount = m.mRootSubmeshCount;
}

static void buildCollisionGeometryForPartInternal(physx::Array<physx::PartConvexHullProxy*>& volumes, ExplicitHierarchicalMesh::Part* part, const NxCollisionVolumeDesc& desc, physx::PxF32 inflation = 0.0f)
{
	physx::PxU32 vertexCount = part->mMesh.size() * 3;
	if (inflation > 0.0f)
	{
		vertexCount *= 7;	// Will add vertices
	}
	physx::Array<physx::PxVec3> vertices(vertexCount);
	physx::PxU32 vertexN = 0;
	for (physx::PxU32 i = 0; i < part->mMesh.size(); ++i)
	{
		physx::NxExplicitRenderTriangle& triangle = part->mMesh[i];
		for (int v = 0; v < 3; ++v)
		{
			const physx::PxVec3& position = triangle.vertices[v].position;
			vertices[vertexN++] = position;
			if (inflation > 0.0f)
			{
				for (int j = 0; j < 3; ++j)
				{
					physx::PxVec3 offset(0.0f);
					offset[j] = inflation;
					for (int k = 0; k < 2; ++k)
					{
						vertices[vertexN++] = position + offset;
						offset[j] *= -1.0f;
					}
				}
			}
		}
	}

	// Identity index buffer
	PX_ALLOCA(indices, PxU32, vertices.size());
	for (physx::PxU32 i = 0; i < vertices.size(); ++i)
	{
		indices[i] = i;
	}

	buildCollisionGeometry(volumes, desc, vertices.begin(), vertices.size(), sizeof(physx::PxVec3), indices, vertices.size());
}

bool ExplicitHierarchicalMesh::calculatePartBSP(physx::PxU32 partIndex, physx::PxU32 randomSeed, physx::PxU32 microgridSize, NxBSPOpenMode::Enum meshMode, IProgressListener* progressListener, volatile bool* cancel)
{
	if (partIndex >= mParts.size())
	{
		return false;
	}

	PX_ASSERT(mParts[partIndex]->mMeshBSP != NULL);

	ApexCSG::BSPBuildParameters bspBuildParameters = gDefaultBuildParameters;
	bspBuildParameters.snapGridSize = microgridSize;
	bspBuildParameters.internalTransform = physx::PxMat44::createZero();
	bspBuildParameters.rnd = &userRnd;
	userRnd.m_rnd.setSeed(randomSeed);
	bool ok = mParts[partIndex]->mMeshBSP->fromMesh(&mParts[partIndex]->mMesh[0], mParts[partIndex]->mMesh.size(), bspBuildParameters, progressListener, cancel);
	if (!ok)
	{
		return false;
	}

	// Check for open mesh
	if (meshMode == NxBSPOpenMode::Closed)
	{
		return true;
	}

	for (physx::PxU32 chunkIndex = 0; chunkIndex < chunkCount(); ++chunkIndex)
	{
		// Find a chunk which uses this part
		if ((physx::PxU32)mChunks[chunkIndex]->mPartIndex == partIndex)
		{
			// If the chunk is a root chunk, test for openness
			if (mChunks[chunkIndex]->isRootChunk())
			{
				physx::PxF32 area, volume;
				if (meshMode == NxBSPOpenMode::Open || !mParts[partIndex]->mMeshBSP->getSurfaceAreaAndVolume(area, volume, true))
				{
					// Mark the mesh as open
					mParts[partIndex]->mFlags |= Part::MeshOpen;
					// Instead of using this mesh's BSP, use the convex hull
					physx::Array<physx::PartConvexHullProxy*> volumes;
					NxCollisionVolumeDesc collisionDesc;
					collisionDesc.mHullMethod = physx::NxConvexHullMethod::WRAP_GRAPHICS_MESH;
					buildCollisionGeometryForPartInternal(volumes, mParts[partIndex], collisionDesc, mParts[partIndex]->mBounds.getExtents().magnitude()*0.01f);
					PX_ASSERT(volumes.size() == 1);
					if (volumes.size() > 0)
					{
						PartConvexHullProxy& hull = *volumes[0];
						physx::Array<physx::PxPlane> planes;
						planes.resize(hull.impl.getPlaneCount());
						const physx::PxVec3 extents = hull.impl.getBounds().getExtents();
						const physx::PxF32 padding = 0.001f*extents.magnitude();
						for (physx::PxU32 planeIndex = 0; planeIndex < hull.impl.getPlaneCount(); ++planeIndex)
						{
							planes[planeIndex] = hull.impl.getPlane(planeIndex);
							planes[planeIndex].d -= padding;
						}
						physx::PxMat44 internalTransform = physx::PxMat44::createIdentity();
						const physx::PxVec3 scale(1.0f/extents[0], 1.0f/extents[1], 1.0f/extents[2]);
						internalTransform.scale(physx::PxVec4(scale, 1.0f));
						internalTransform.setPosition(-scale.multiply(hull.impl.getBounds().getCenter()));
						mParts[partIndex]->mMeshBSP->fromConvexPolyhedron(&planes[0], planes.size(), internalTransform, &mParts[partIndex]->mMesh[0], mParts[partIndex]->mMesh.size());
					}
				}
			}
			break;
		}
	}

	return true;
}

void ExplicitHierarchicalMesh::replaceInteriorSubmeshes(physx::PxU32 partIndex, physx::PxU32 frameCount, physx::PxU32* frameIndices, physx::PxU32 submeshIndex)
{
	if (partIndex >= mParts.size())
	{
		return;
	}

	Part* part = mParts[partIndex];

	// Replace render mesh submesh indices
	for (physx::PxU32 triangleIndex = 0; triangleIndex < part->mMesh.size(); ++triangleIndex)
	{
		NxExplicitRenderTriangle& triangle = part->mMesh[triangleIndex];
		for (physx::PxU32 frameNum = 0; frameNum < frameCount; ++frameNum)
		{
			if (triangle.extraDataIndex == frameIndices[frameNum])
			{
				triangle.submeshIndex = (physx::PxI32)submeshIndex;
			}
		}
	}

	// Replace BSP mesh submesh indices
	part->mMeshBSP->replaceInteriorSubmeshes(frameCount, frameIndices, submeshIndex);
}

void ExplicitHierarchicalMesh::calculateMeshBSP(physx::PxU32 randomSeed, IProgressListener* progressListener, const physx::PxU32* microgridSize, NxBSPOpenMode::Enum meshMode)
{
	if (partCount() == 0)
	{
		outputMessage("No mesh, cannot calculate BSP.", physx::PxErrorCode::eDEBUG_WARNING);
		return;
	}

	physx::PxU32 bspCount = 0;
	for (physx::PxU32 chunkIndex = 0; chunkIndex < mChunks.size(); ++chunkIndex)
	{
		if (mChunks[chunkIndex]->isRootLeafChunk())
		{
			++bspCount;
		}
	}

	if (bspCount == 0)
	{
		outputMessage("No parts at root depth, no BSPs to calculate", physx::PxErrorCode::eDEBUG_WARNING);
		return;
	}

	HierarchicalProgressListener progress(physx::PxMax((physx::PxI32)bspCount, 1), progressListener);

	const physx::PxU32 microgridSizeToUse = microgridSize != NULL ? *microgridSize : gMicrogridSize;

	for (physx::PxU32 chunkIndex = 0; chunkIndex < mChunks.size(); ++chunkIndex)
	{
		if (mChunks[chunkIndex]->isRootLeafChunk())
		{
			physx::PxU32 chunkPartIndex = (physx::PxU32)*partIndex(chunkIndex);
			calculatePartBSP(chunkPartIndex, randomSeed, microgridSizeToUse, meshMode, &progress);
			progress.completeSubtask();
		}
	}
}

void ExplicitHierarchicalMesh::visualize(NxApexRenderDebug& debugRender, physx::PxU32 flags, physx::PxU32 index) const
{
#ifdef WITHOUT_DEBUG_VISUALIZE
	PX_UNUSED(debugRender);
	PX_UNUSED(flags);
	PX_UNUSED(index);
#else
	physx::PxU32 bspMeshFlags = 0;
	if (flags & VisualizeMeshBSPInsideRegions)
	{
		bspMeshFlags |= ApexCSG::BSPVisualizationFlags::InsideRegions;
	}
	if (flags & VisualizeMeshBSPOutsideRegions)
	{
		bspMeshFlags |= ApexCSG::BSPVisualizationFlags::OutsideRegions;
	}
	if (flags & VisualizeMeshBSPSingleRegion)
	{
		bspMeshFlags |= ApexCSG::BSPVisualizationFlags::SingleRegion;
	}
	for (physx::PxU32 partIndex = 0; partIndex < mParts.size(); ++partIndex)
	{
		if (mParts[partIndex]->mMeshBSP != NULL)
		{
			mParts[partIndex]->mMeshBSP->visualize(debugRender, bspMeshFlags, index);
		}
	}
#endif
}

void ExplicitHierarchicalMesh::release()
{
	delete this;
}

void ExplicitHierarchicalMesh::buildMeshBounds(physx::PxU32 partIndex)
{
	if (partIndex < partCount())
	{
		physx::PxBounds3& bounds = mParts[partIndex]->mBounds;
		bounds.setEmpty();
		const physx::Array<NxExplicitRenderTriangle>& mesh = mParts[partIndex]->mMesh;
		for (physx::PxU32 i = 0; i < mesh.size(); ++i)
		{
			bounds.include(mesh[i].vertices[0].position);
			bounds.include(mesh[i].vertices[1].position);
			bounds.include(mesh[i].vertices[2].position);
		}
	}
}

void ExplicitHierarchicalMesh::buildCollisionGeometryForPart(physx::PxU32 partIndex, const NxCollisionVolumeDesc& desc)
{
	if (partIndex < partCount())
	{
		Part* part = mParts[partIndex];
		buildCollisionGeometryForPartInternal(part->mCollision, part, desc);
	}
}

void ExplicitHierarchicalMesh::aggregateCollisionHullsFromRootChildren(physx::PxU32 chunkIndex)
{
	physx::InlineArray<physx::PxU32,16> rootChildren;
	for (physx::PxU32 i = 0; i < mChunks.size(); ++i)
	{
		if (mChunks[i]->mParentIndex == (physx::PxI32)chunkIndex && mChunks[i]->isRootChunk())
		{
			rootChildren.pushBack(i);
		}
	}

	if (rootChildren.size() != 0)
	{
		physx::PxU32 newHullCount = 0;
		for (physx::PxU32 rootChildNum = 0; rootChildNum < rootChildren.size(); ++rootChildNum)
		{
			const physx::PxU32 rootChild = rootChildren[rootChildNum];
			aggregateCollisionHullsFromRootChildren(rootChild);
			const physx::PxU32 childPartIndex = (physx::PxU32)mChunks[rootChild]->mPartIndex;
			newHullCount += mParts[childPartIndex]->mCollision.size();
		}
		const physx::PxU32 partIndex = (physx::PxU32)mChunks[chunkIndex]->mPartIndex;
		resizeCollision(mParts[partIndex]->mCollision, newHullCount);
		newHullCount = 0;
		for (physx::PxU32 rootChildNum = 0; rootChildNum < rootChildren.size(); ++rootChildNum)
		{
			const physx::PxU32 rootChild = rootChildren[rootChildNum];
			const physx::PxU32 childPartIndex = (physx::PxU32)mChunks[rootChild]->mPartIndex;
			for (physx::PxU32 hullN = 0; hullN < mParts[childPartIndex]->mCollision.size(); ++hullN)
			{
				*mParts[partIndex]->mCollision[newHullCount++] = *mParts[childPartIndex]->mCollision[hullN];
			}
		}
		PX_ASSERT(newHullCount == mParts[partIndex]->mCollision.size());
	}
}

void ExplicitHierarchicalMesh::buildCollisionGeometryForRootChunkParts(const NxCollisionDesc& desc, bool aggregateRootChunkParentCollision)
{
	// This helps keep the loops small if there are a lot of child chunks
	physx::PxU32 rootChunkStop = 0;

	for (physx::PxU32 chunkIndex = 0; chunkIndex < chunkCount(); ++chunkIndex)
	{
		if (mChunks[chunkIndex]->isRootChunk())
		{
			rootChunkStop = chunkIndex+1;
			const physx::PxU32 partIndex = (physx::PxU32)mChunks[chunkIndex]->mPartIndex;
			if (partIndex < mParts.size())
			{
				resizeCollision(mParts[partIndex]->mCollision, 0);
			}
		}
	}

	for (physx::PxU32 chunkIndex = 0; chunkIndex < rootChunkStop; ++chunkIndex)
	{
		if (mChunks[chunkIndex]->isRootLeafChunk() || (mChunks[chunkIndex]->isRootChunk() && !aggregateRootChunkParentCollision))
		{
			const physx::PxU32 partIndex = (physx::PxU32)mChunks[chunkIndex]->mPartIndex;
			if (partIndex < mParts.size() && mParts[partIndex]->mCollision.size() == 0)
			{
				NxCollisionVolumeDesc volumeDesc = getVolumeDesc(desc, depth(chunkIndex));
				volumeDesc.mMaxVertexCount = volumeDesc.mMaxEdgeCount = volumeDesc.mMaxFaceCount = 0;	// Don't reduce hulls until the very end
				buildCollisionGeometryForPart(partIndex, volumeDesc);
			}
		}
	}

	if (aggregateRootChunkParentCollision)
	{
		// Aggregate collision volumes from root depth chunks to their parents, recursing to depth 0
		aggregateCollisionHullsFromRootChildren(0);
	}

	if (desc.mMaximumTrimming > 0.0f)
	{
		// Trim hulls up to root depth
		for (physx::PxU32 processDepth = 1; (physx::PxI32)processDepth <= maxDepth(); ++processDepth)
		{
			physx::Array<physx::PxU32> chunkIndexArray;
			for (physx::PxU32 chunkIndex = 0; chunkIndex < rootChunkStop; ++chunkIndex)
			{
				if (mChunks[chunkIndex]->isRootChunk() && depth(chunkIndex) == processDepth)
				{
					chunkIndexArray.pushBack(chunkIndex);
				}
			}
			if (chunkIndexArray.size() > 0)
			{
				trimChunkHulls(*this, &chunkIndexArray[0], chunkIndexArray.size(), desc.mMaximumTrimming);
			}
		}
	}

	// Finally reduce the hulls
	reduceHulls(desc, true);
}

void ExplicitHierarchicalMesh::reduceHulls(const physx::NxCollisionDesc& desc, bool inflated)
{
	physx::Array<bool> partReduced(mParts.size(), false);

	for (physx::PxU32 chunkIndex = 0; chunkIndex < mChunks.size(); ++chunkIndex)
	{
		physx::PxU32 partIndex = (PxU32)mChunks[chunkIndex]->mPartIndex;
		if (partReduced[partIndex])
		{
			continue;
		}
		Part* part = mParts[partIndex];
		NxCollisionVolumeDesc volumeDesc = getVolumeDesc(desc, depth(chunkIndex));
		for (physx::PxU32 hullIndex = 0; hullIndex < part->mCollision.size(); ++hullIndex)
		{
			// First try uninflated, then try with inflation (if requested).  This may find a better reduction
			part->mCollision[hullIndex]->reduceHull(volumeDesc.mMaxVertexCount, volumeDesc.mMaxEdgeCount, volumeDesc.mMaxFaceCount, false);
			if (inflated)
			{
				part->mCollision[hullIndex]->reduceHull(volumeDesc.mMaxVertexCount, volumeDesc.mMaxEdgeCount, volumeDesc.mMaxFaceCount, true);
			}
		}
		partReduced[partIndex] = true;
	}
}

void ExplicitHierarchicalMesh::initializeDisplacementMapVolume(const physx::NxFractureSliceDesc& desc)
{
	mDisplacementMapVolume.init(desc);
}

}
} // namespace physx::apex

namespace FractureTools
{
physx::NxExplicitHierarchicalMesh* createExplicitHierarchicalMesh()
{
	return PX_NEW(physx::ExplicitHierarchicalMesh)();
}

physx::NxExplicitHierarchicalMesh::NxConvexHull*	createExplicitHierarchicalMeshConvexHull()
{
	return PX_NEW(physx::PartConvexHullProxy)();
}
} // namespace FractureTools

#endif // !defined(WITHOUT_APEX_AUTHORING)

//#ifdef _MANAGED
//#pragma managed(pop)
//#endif
