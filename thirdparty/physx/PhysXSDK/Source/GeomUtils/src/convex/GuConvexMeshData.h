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

#ifndef GU_CONVEX_MESH_DATA_H
#define GU_CONVEX_MESH_DATA_H

#include "CmPhysXCommon.h"
#include "PxBounds3.h"
#include "PsIntrinsics.h"
#include "PxPlane.h"
#include "GuSerialize.h"

// Data definition

namespace physx
{
namespace Gu
{
	struct BigConvexRawData;

	struct HullPolygonData
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================

		//Note: this structure can not be used with PX_NEW because Plane has a constructor but isn't a sub-class of PxAllocatable.
		//		please do not assume that any data types added to this class will automatically call their constructor in cooking. 
		PxPlane	mPlane;			//!< Plane equation for this polygon	//Could drop 4th elem as it can be computed from any vertex as: d = - p.dot(n);
		PxU16	mVRef8;			//!< Offset of vertex references in hull vertex data (CS: can we assume indices are tightly packed and offsets are ascending?? DrawObjects makes and uses this assumption)
		PxU8	mNbVerts;		//!< Number of vertices/edges in the polygon
		PxU8	mMinIndex;		//!< Index of the polygon vertex that has minimal projection along this plane's normal.

		PX_FORCE_INLINE	PxReal getMin(const PxVec3* PX_RESTRICT hullVertices) const	//minimum of projection of the hull along this plane normal
		{ 
			return mPlane.n.dot(hullVertices[mMinIndex]);
		}

		PX_FORCE_INLINE	PxReal getMax() const		{ return -mPlane.d; }	//maximum of projection of the hull along this plane normal
	};

	PX_FORCE_INLINE void flipData(Gu::HullPolygonData& data)
	{
		flip(data.mPlane.n.x);
		flip(data.mPlane.n.y);
		flip(data.mPlane.n.z);
		flip(data.mPlane.d);
		flip(data.mVRef8);
	}
	// PT: if this one breaks, please make sure the 'flipData' function is properly updated.
	PX_COMPILE_TIME_ASSERT(sizeof(Gu::HullPolygonData) == 20);

// TEST_INTERNAL_OBJECTS
	struct InternalObjectsData
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================
		PxReal	mRadius;
		PxReal	mExtents[3];

		PX_FORCE_INLINE	void reset()
		{
			mRadius = 0.0f;
			mExtents[0] = 0.0f;
			mExtents[1] = 0.0f;
			mExtents[2] = 0.0f;
		}
	};
	PX_COMPILE_TIME_ASSERT(sizeof(Gu::InternalObjectsData) == 16);
//~TEST_INTERNAL_OBJECTS

	struct ConvexHullData
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================

		PxBounds3			mAABB;				//!< bounds TODO: compute this on the fly from first 6 vertices in the vertex array.  We'll of course need to sort the most extreme ones to the front.
		PxVec3				mCenterOfMass;		//in local space of mesh!

		// PT: WARNING: mNbHullVertices *must* appear before mBigConvexRawData for ConvX to be able to do "big raw data" surgery
		PxU16				mNbEdges;
		PxU8				mNbHullVertices;	//!< Number of vertices in the convex hull
		PxU8				mNbPolygons;		//!< Number of planar polygons composing the hull

		HullPolygonData*	mPolygons;			//!< Array of mNbPolygons structures
		BigConvexRawData*	mBigConvexRawData;	//!< Hill climbing data, only for large convexes! else NULL.

// TEST_INTERNAL_OBJECTS
		InternalObjectsData	mInternal;
//~TEST_INTERNAL_OBJECTS

		PX_FORCE_INLINE	const PxVec3* getHullVertices()	const	//!< Convex hull vertices
		{
			const char* tmp = reinterpret_cast<const char*>(mPolygons);
			tmp += sizeof(Gu::HullPolygonData) * mNbPolygons;
			return reinterpret_cast<const PxVec3*>(tmp);
		}

		PX_FORCE_INLINE	const PxU8* getFacesByEdges8()	const	//!< for each edge, gives 2 adjacent polygons; used by convex-convex code to come up with all the convex' edge normals.  
		{
			const char* tmp = reinterpret_cast<const char*>(mPolygons);
			tmp += sizeof(Gu::HullPolygonData) * mNbPolygons;
			tmp += sizeof(PxVec3) * mNbHullVertices; 
			return reinterpret_cast<const PxU8*>(tmp);
		}

		PX_FORCE_INLINE	const PxU8* getFacesByVertices8()	const	//!< for each edge, gives 2 adjacent polygons; used by convex-convex code to come up with all the convex' edge normals.  
		{
			const char* tmp = reinterpret_cast<const char*>(mPolygons);
			tmp += sizeof(Gu::HullPolygonData) * mNbPolygons;
			tmp += sizeof(PxVec3) * mNbHullVertices;
			tmp += sizeof(PxU8) * mNbEdges * 2;
			return reinterpret_cast<const PxU8*>(tmp);
		}


		PX_FORCE_INLINE	const PxU8*	getVertexData8()	const	//!< Vertex indices indexed by hull polygons
		{
			const char* tmp = reinterpret_cast<const char*>(mPolygons);
			tmp += sizeof(Gu::HullPolygonData) * mNbPolygons;
			tmp += sizeof(PxVec3) * mNbHullVertices;
			tmp += sizeof(PxU8) * mNbEdges * 2;
			tmp += sizeof(PxU8) * mNbHullVertices * 3;
			return reinterpret_cast<const PxU8*>(tmp);
		}

	};
	#if defined(PX_P64)
	PX_COMPILE_TIME_ASSERT(sizeof(Gu::ConvexHullData) == 72);
	#else
	PX_COMPILE_TIME_ASSERT(sizeof(Gu::ConvexHullData) == 64);
	#endif
} // namespace Gu

}

//#pragma PX_POP_PACK

#endif
