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


#ifndef PX_CONVEXHULLBUILDER_H
#define PX_CONVEXHULLBUILDER_H

#include "GuConvexMeshData.h"

namespace physx
{
	namespace Gu
	{
		struct EdgeDescData;
	} // namespace Gu

	struct HullTriangleData
	{
		PxU32	mRef[3];
	};

	class ConvexHullBuilder : public Ps::UserAllocated
	{
		public:
												ConvexHullBuilder(Gu::ConvexHullData* hull);
												~ConvexHullBuilder();

					bool						Init(PxU32 nbVerts, const PxVec3* verts, PxU32 nbTris, const PxU32* indices, const PxU32 nbPolygons=0, const PxHullPolygon* hullPolygons=NULL);

					bool						Save(PxOutputStream& stream, bool platformMismatch)	const;

					bool						CreateEdgeList();
					bool						CreatePolygonData();
					bool						CheckHullPolygons()	const;
					bool						CreateTrianglesFromPolygons();
					bool						ComputeGeomCenter(PxVec3& center) const;

					bool						CalculateVertexMapTable(PxU32 nbPolygons, bool userPolygons = false);

					bool						ComputeHullPolygons(const PxU32& nbVerts,const PxVec3* verts, const PxU32& nbTriangles, const PxU32* triangles);

		PX_INLINE	PxU32						ComputeNbPolygons()		const
												{
													if(!mHull->mNbPolygons)	const_cast<ConvexHullBuilder*>(this)->CreatePolygonData();	// "mutable method"
													return mHull->mNbPolygons;
												}

		PX_INLINE	PxU32						GetNbFaces()const	{ return mNbHullFaces;	}
		PX_INLINE	const HullTriangleData*		GetFaces()	const	{ return mFaces;		}

					PxVec3*						mHullDataHullVertices;
					Gu::HullPolygonData*		mHullDataPolygons;
					PxU8*						mHullDataVertexData8;
					PxU8*						mHullDataFacesByEdges8;
					PxU8*						mHullDataFacesByVertices8;

					Gu::ConvexHullData*			mHull;
	private:
					Gu::EdgeDescData*			mEdgeToTriangles;
					PxU16*						mEdgeData16;	//!< Edge indices indexed by hull polygons
					PxU32						mNbHullFaces;	//!< Number of faces in the convex hull
					HullTriangleData*			mFaces;			//!< Triangles.
	};
}

#endif	// PX_CONVEXHULLBUILDER_H

