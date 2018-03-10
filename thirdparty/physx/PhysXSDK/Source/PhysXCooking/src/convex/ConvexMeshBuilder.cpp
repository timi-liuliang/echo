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


#include "GuConvexMesh.h"
#include "PsFoundation.h"
#include "PsFastMemory.h"
#include "PsMathUtils.h"

#include "GuHillClimbing.h"
#include "GuBigConvexData2.h"
#include "GuGeomUtilsInternal.h"
#include "GuSerialize.h"
#include "VolumeIntegration.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "VolumeIntegration.h"
#include "ConvexHullBuilder.h"
#include "ConvexMeshBuilder.h"
#include "BigConvexDataBuilder.h"
#include "InternalTriangleMeshBuilder.h"
#include "GuGeomUtilsInternal.h"

#include "PxGaussMapLimit.h"
#include "CmUtils.h"

using namespace physx;
using namespace Gu;
using namespace Gu;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ConvexMeshBuilder::ConvexMeshBuilder() : hullBuilder(&mHullData)
{
}

ConvexMeshBuilder::~ConvexMeshBuilder()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ConvexMeshBuilder::loadFromDesc(const PxConvexMeshDesc& desc, PxPlatform::Enum	targetPlatform)
{
	if(!desc.isValid())
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "Gu::ConvexMesh::loadFromDesc: desc.isValid() failed!");
		return false;
	}

	if(!loadConvexHull(desc))
		return false;

	// Compute local bounds (*after* hull has been created)
	computeBoundsAroundVertices(mHullData.mAABB, mHullData.mNbHullVertices, hullBuilder.mHullDataHullVertices);

	if(mHullData.mNbHullVertices > PxGetGaussMapVertexLimitForPlatform(targetPlatform)) 
		if (!computeGaussMaps())
			return false;

// TEST_INTERNAL_OBJECTS
	computeInternalObjects();
//~TEST_INTERNAL_OBJECTS

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PX_COMPILE_TIME_ASSERT(sizeof(PxMaterialTableIndex)==sizeof(PxU16));
bool ConvexMeshBuilder::save(PxOutputStream& stream, bool platformMismatch) const
{
	// Export header
	if(!writeHeader('C', 'V', 'X', 'M', PX_CONVEX_VERSION, platformMismatch, stream))
		return false;

	// Export serialization flags
	PxU32 serialFlags = 0;

	writeDword(serialFlags, platformMismatch, stream);

	if(!hullBuilder.Save(stream, platformMismatch))
		return false;

	// Export local bounds
//	writeFloat(geomEpsilon, platformMismatch, stream);
	writeFloat(0.0f, platformMismatch, stream);
	writeFloat(mHullData.mAABB.minimum.x, platformMismatch, stream);
	writeFloat(mHullData.mAABB.minimum.y, platformMismatch, stream);
	writeFloat(mHullData.mAABB.minimum.z, platformMismatch, stream);
	writeFloat(mHullData.mAABB.maximum.x, platformMismatch, stream);
	writeFloat(mHullData.mAABB.maximum.y, platformMismatch, stream);
	writeFloat(mHullData.mAABB.maximum.z, platformMismatch, stream);

	// Export mass info
	writeFloat(mMass, platformMismatch, stream);
	writeFloatBuffer((const PxF32*)&mInertia, 9, platformMismatch, stream);
	writeFloatBuffer(&mHullData.mCenterOfMass.x, 3, platformMismatch, stream);

	// Export gaussmaps
	if(mBigConvexData)
	{
		writeFloat(1.0f, platformMismatch, stream);		//gauss map flag true
		BigConvexDataBuilder SVMB(&mHullData, mBigConvexData, hullBuilder.mHullDataHullVertices);
		SVMB.Save(stream, platformMismatch, hullBuilder.GetNbFaces(), hullBuilder.GetFaces());
	}
	else
		writeFloat(-1.0f, platformMismatch, stream);	//gauss map flag false

// TEST_INTERNAL_OBJECTS
	writeFloat(mHullData.mInternal.mRadius, platformMismatch, stream);
	writeFloat(mHullData.mInternal.mExtents[0], platformMismatch, stream);
	writeFloat(mHullData.mInternal.mExtents[1], platformMismatch, stream);
	writeFloat(mHullData.mInternal.mExtents[2], platformMismatch, stream);
//~TEST_INTERNAL_OBJECTS
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ConvexMeshBuilder::computeMassInfo()
{
	if(mMass <= 0.0f)		//not yet computed.
	{
		PxIntegrals integrals;
		PxSimpleTriangleMesh simpleMesh;

		simpleMesh.points.count		= mHullData.mNbHullVertices;
		simpleMesh.triangles.count	= hullBuilder.GetNbFaces();
		simpleMesh.points.stride	= sizeof(PxVec3);
		simpleMesh.triangles.stride	= sizeof(PxU32) * 3;
		simpleMesh.points.data		= hullBuilder.mHullDataHullVertices;
		simpleMesh.triangles.data	= hullBuilder.GetFaces();

		// using the centroid of the convex for the volume integration solved accuracy issues in cases where the inertia tensor
		// ended up close to not being positive definite and after a few further transforms the diagonalized inertia tensor ended
		// up with negative values.
		PxVec3 mean(0.0f);
		for(PxU32 i=0; i < simpleMesh.points.count; i++)
			mean += hullBuilder.mHullDataHullVertices[i];
		mean *= (1.0f / mHullData.mNbHullVertices);

		bool status = computeVolumeIntegralsEberly(simpleMesh, 1.0f, integrals, mean);
		if(status)	
		{

			integrals.getOriginInertia(reinterpret_cast<PxMat33&>(mInertia));
			mHullData.mCenterOfMass = integrals.COM;

			//note: the mass will be negative for an inside-out mesh!
			if(mInertia.column0.isFinite() && mInertia.column1.isFinite() && mInertia.column2.isFinite() 
				&& mHullData.mCenterOfMass.isFinite() && PxIsFinite((PxReal)integrals.mass))
			{
				if (integrals.mass < 0)
				{
					Ps::getFoundation().error(PX_WARN, "Gu::ConvexMesh: Mesh has a negative volume! Is it open or do (some) faces have reversed winding? (Taking absolute value.)");
					integrals.mass = -integrals.mass;
					mInertia = -mInertia;
				}

				mMass = PxReal(integrals.mass);	//set mass to valid value.
				return;
			}
		}
		Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "Gu::ConvexMesh: Error computing mesh mass properties!\n");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma warning(push)
#pragma warning(disable:4996)	// permitting use of gatherStrided until we have a replacement.

bool ConvexMeshBuilder::loadConvexHull(const PxConvexMeshDesc& desc)
{
	// The user provided his own hull

	PxVec3* geometry = (PxVec3*)PxAlloca(sizeof(PxVec3)*desc.points.count);
	Ps::gatherStrided(desc.points.data, geometry, desc.points.count, sizeof(PxVec3), desc.points.stride);

	PxU32* topology = NULL;
	
	if(desc.triangles.data)
	{
		topology = (PxU32*)PxAlloca(sizeof(PxU32)*3*desc.triangles.count);
		if (desc.flags & PxConvexFlag::e16_BIT_INDICES)
		{
			// conversion; 16 bit index -> 32 bit index & stride
			PxU32* dest = topology;
			const PxU32* pastLastDest = topology + 3*desc.triangles.count;
			const PxU8* source = (const PxU8 *)desc.triangles.data;
			while (dest < pastLastDest)
			{
				const PxU16 * trig16 = (const PxU16 *)source;
				*dest++ = trig16[0];
				*dest++ = trig16[1];
				*dest++ = trig16[2];
				source += desc.triangles.stride;
			}
		}
		else
		{
			Ps::gatherStrided(desc.triangles.data, topology, desc.triangles.count, sizeof(PxU32) * 3, desc.triangles.stride);
		}
	}

	// store the indices into topology if we have the polygon data
	if(desc.indices.data)
	{
		topology = (PxU32*)PxAlloca(sizeof(PxU32)*desc.indices.count);
		if (desc.flags & PxConvexFlag::e16_BIT_INDICES)
		{
			// conversion; 16 bit index -> 32 bit index & stride
			PxU32* dest = topology;
			const PxU32* pastLastDest = topology + desc.indices.count;
			const PxU8* source = (const PxU8 *)desc.indices.data;
			while (dest < pastLastDest)
			{
				const PxU16 * trig16 = (const PxU16 *)source;
				*dest++ = trig16[0];
				*dest++ = trig16[1];
				*dest++ = trig16[2];
				source += desc.indices.stride;
			}
		}
		else
		{
			Ps::gatherStrided(desc.indices.data, topology, desc.indices.count, sizeof(PxU32), desc.indices.stride);
		}
	}

	PxHullPolygon* hullPolygons = NULL;
	if(desc.polygons.data)
	{
		hullPolygons = (PxHullPolygon*) PxAlloca(sizeof(PxHullPolygon)*desc.polygons.count);
		Ps::gatherStrided(desc.polygons.data,hullPolygons,desc.polygons.count,sizeof(PxHullPolygon),desc.polygons.stride);
	}

  	if(!hullBuilder.Init(desc.points.count, geometry, desc.triangles.count, topology, desc.polygons.count, hullPolygons))
  	{
		if(hullPolygons)
			Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "Gu::ConvexMesh::loadConvexHull: convex hull init failed!");
		else
  			Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "Gu::ConvexMesh::loadConvexHull: convex hull init failed! Try to use the PxConvexFlag::eINFLATE_CONVEX flag. (see PxToolkit::createConvexMeshSafe)");
  		return false;
  	}
	computeMassInfo();
  
	return true;
}

#pragma warning(pop)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ConvexMeshBuilder::computeHullPolygons(const PxU32& nbVerts,const PxVec3* verts, const PxU32& nbTriangles, const PxU32* triangles, PxAllocatorCallback& inAllocator,
	PxU32& outNbVerts, PxVec3*& outVertices , PxU32& nbIndices, PxU32*& indices, PxU32& nbPolygons, PxHullPolygon*& polygons)
{
	if(!hullBuilder.ComputeHullPolygons(nbVerts,verts,nbTriangles,triangles))
	{
		Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "ConvexMeshBuilder::computeHullPolygons: compute convex hull polygons failed. Provided triangles dont form a convex hull.");
		return false;
	}

	outNbVerts = hullBuilder.mHull->mNbHullVertices;
	nbPolygons = hullBuilder.mHull->mNbPolygons; 

	outVertices = (PxVec3*) inAllocator.allocate(outNbVerts*sizeof(PxVec3),"PxVec3",__FILE__,__LINE__);
	PxMemCopy(outVertices,hullBuilder.mHullDataHullVertices,outNbVerts*sizeof(PxVec3));

	nbIndices = 0;
	for (PxU32 i = 0; i < nbPolygons; i++)
	{
		nbIndices += hullBuilder.mHullDataPolygons[i].mNbVerts;
	}

	indices = (PxU32*) inAllocator.allocate(nbIndices*sizeof(PxU32),"PxU32",__FILE__,__LINE__);
	for (PxU32 i = 0; i < nbIndices; i++)
	{
		indices[i] = hullBuilder.mHullDataVertexData8[i];
	}

	polygons = (PxHullPolygon*) inAllocator.allocate(nbPolygons*sizeof(PxHullPolygon),"PxHullPolygon",__FILE__,__LINE__);

	for (PxU32 i = 0; i < nbPolygons; i++)
	{
		const Gu::HullPolygonData& polygonData = hullBuilder.mHullDataPolygons[i];
		PxHullPolygon& outPolygon = polygons[i];
		outPolygon.mPlane[0] = polygonData.mPlane.n.x;
		outPolygon.mPlane[1] = polygonData.mPlane.n.y;
		outPolygon.mPlane[2] = polygonData.mPlane.n.z;
		outPolygon.mPlane[3] = polygonData.mPlane.d;

		outPolygon.mNbVerts = polygonData.mNbVerts;
		outPolygon.mIndexBase = polygonData.mVRef8;

		for (PxU32 j = 0; j < polygonData.mNbVerts; j++)
		{
			PX_ASSERT(indices[outPolygon.mIndexBase + j] == hullBuilder.mHullDataVertexData8[polygonData.mVRef8+j]);
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ConvexMeshBuilder::computeGaussMaps()
{
	// The number of polygons is limited to 256 because the gaussmap encode 256 polys maximum

	PxU32 density = 16;
	//	density = 64;
	//	density = 8;
	//	density = 2;

	PX_DELETE(mBigConvexData);	
	PX_NEW_SERIALIZED(mBigConvexData,BigConvexData);	
	BigConvexDataBuilder SVMB(&mHullData, mBigConvexData, hullBuilder.mHullDataHullVertices);
	SVMB.Precompute(density);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TEST_INTERNAL_OBJECTS
static void ComputeAABBNoTransform(PxVec3& c, PxVec3& e, PxU32 nbVerts, const PxVec3* verts)
{
	float MinX = FLT_MAX;
	float MinY = FLT_MAX;
	float MinZ = FLT_MAX;
	float MaxX = -FLT_MAX;
	float MaxY = -FLT_MAX;
	float MaxZ = -FLT_MAX;
	for(PxU32 i=0; i<nbVerts; i++)
	{
		const PxVec3& pt = verts[i];
		if(pt.x<MinX)	MinX = pt.x;
		if(pt.x>MaxX)	MaxX = pt.x;
		if(pt.y<MinY)	MinY = pt.y;
		if(pt.y>MaxY)	MaxY = pt.y;
		if(pt.z<MinZ)	MinZ = pt.z;
		if(pt.z>MaxZ)	MaxZ = pt.z;
	}
	c = PxVec3(MaxX+MinX, MaxY+MinY, MaxZ+MinZ);
	e = PxVec3(MaxX-MinX, MaxY-MinY, MaxZ-MinZ);
}

static bool TestContainment(const PxVec3& localCenter, const float extents[3], PxU32 nbPolys, const Gu::HullPolygonData* polygons)
{
	for(PxU32 p=0;p<8;p++)
	{
		PxVec3 LocalPt;
		if(p==0)		LocalPt = localCenter + PxVec3(extents[0], extents[1], extents[2]);
		else if(p==1)	LocalPt = localCenter + PxVec3(extents[0], extents[1], -extents[2]);
		else if(p==2)	LocalPt = localCenter + PxVec3(extents[0], -extents[1], extents[2]);
		else if(p==3)	LocalPt = localCenter + PxVec3(extents[0], -extents[1], -extents[2]);
		else if(p==4)	LocalPt = localCenter + PxVec3(-extents[0], extents[1], extents[2]);
		else if(p==5)	LocalPt = localCenter + PxVec3(-extents[0], extents[1], -extents[2]);
		else if(p==6)	LocalPt = localCenter + PxVec3(-extents[0], -extents[1], extents[2]);
		else if(p==7)	LocalPt = localCenter + PxVec3(-extents[0], -extents[1], -extents[2]);

		for(PxU32 i=0;i<nbPolys;i++)
		{
			const PxVec3 Normal=polygons[i].mPlane.n;
			const float d = LocalPt.dot(Normal) + polygons[i].mPlane.d;
			if(d>0.0f)
				return false;
		}
	}
	return true;
}

void ConvexMeshBuilder::computeInternalObjects()
{
	const Gu::HullPolygonData* hullPolys = hullBuilder.mHullDataPolygons;
	const PxVec3* hullVerts = hullBuilder.mHullDataHullVertices;

	Gu::ConvexHullData& data = mHullData;
	data.mInternal.mRadius = FLT_MAX;
	for(PxU32 i=0;i<data.mNbPolygons;i++)
	{
		const float dist = fabsf(data.mCenterOfMass.dot(hullPolys[i].mPlane.n) + hullPolys[i].mPlane.d);
		if(dist<data.mInternal.mRadius)
			data.mInternal.mRadius = dist;
	}

	PxVec3 c,e;
	ComputeAABBNoTransform(c, e, data.mNbHullVertices, hullVerts);

	const float r = data.mInternal.mRadius / sqrtf(3.0f);
//	const int LargestExtent = e.maxAxis();
	const PxU32 LargestExtent = Ps::largestAxis(e);
	const float step1 = (e[LargestExtent]*0.5f - r)/1024.0f;
	data.mInternal.mExtents[0] = data.mInternal.mExtents[1] = data.mInternal.mExtents[2] = r;
	data.mInternal.mExtents[LargestExtent] = e[LargestExtent]*0.5f;
	bool FoundBox = false;
	for(PxU32 j=0;j<1024;j++)
	{
		if(TestContainment(data.mCenterOfMass, data.mInternal.mExtents, data.mNbPolygons, hullPolys))
		{
			FoundBox = true;
			break;
		}

		data.mInternal.mExtents[LargestExtent] -= step1;
	}
	if(!FoundBox)
	{
		data.mInternal.mExtents[0] = data.mInternal.mExtents[1] = data.mInternal.mExtents[2] = r;
	}
	else
	{
		// Refine the box
		const float Step = (data.mInternal.mRadius - r)/1024.0f;
		const PxU32 e0 = PxU32((1<<LargestExtent) & 3);
		const PxU32 e1 = PxU32((1<<e0) & 3);

		for(PxU32 j=0;j<1024;j++)
		{
			const float Saved0 = data.mInternal.mExtents[e0];
			const float Saved1 = data.mInternal.mExtents[e1];
			data.mInternal.mExtents[e0] += Step;
			data.mInternal.mExtents[e1] += Step;

			if(!TestContainment(data.mCenterOfMass, data.mInternal.mExtents, data.mNbPolygons, hullPolys))
			{
				data.mInternal.mExtents[e0] = Saved0;
				data.mInternal.mExtents[e1] = Saved1;
				break;
			}
		}
	}
}

//~TEST_INTERNAL_OBJECTS
