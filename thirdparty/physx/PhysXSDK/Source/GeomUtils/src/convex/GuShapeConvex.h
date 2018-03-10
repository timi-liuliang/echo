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

#ifndef GU_SHAPECONVEX_H
#define GU_SHAPECONVEX_H

#include "GuConvexMeshData.h"
#include "CmScaling.h"

namespace physx
{
namespace Gu
{
	// More PS3 friendly version
	struct PolygonalData;
	typedef void	(*HullPrefetchCB)		(PxU32 numVerts, const PxVec3* PX_RESTRICT verts);
	typedef void	(*HullProjectionCB)		(const PolygonalData& data, const PxVec3& dir, const Cm::Matrix34& world2hull, const Cm::FastVertex2ShapeScaling& scaling, PxReal& minimum, PxReal& maximum);
	typedef PxU32	(*SelectClosestEdgeCB)	(const PolygonalData& data, const Cm::FastVertex2ShapeScaling& scaling, const PxVec3& localDirection);

	struct PolygonalData
	{
		// Data
		PxVec3								mCenter;
		PxU32								mNbVerts;
		PxU32								mNbPolygons;
		PxU32								mNbEdges;
		const Gu::HullPolygonData*			mPolygons;
		const PxVec3*						mVerts;
		const PxU8*							mPolygonVertexRefs;
		const PxU8*							mFacesByEdges;
		Gu::InternalObjectsData				mInternal;
		union
		{
			const Gu::BigConvexRawData*		mBigData;	// Only for big convexes
			const PxVec3*					mHalfSide;	// Only for boxes
		};

		// Code
		HullPrefetchCB						mPrefetchHull;
		HullProjectionCB					mProjectHull;
		SelectClosestEdgeCB					mSelectClosestEdgeCB;

		PX_FORCE_INLINE const PxU8*	getPolygonVertexRefs(const Gu::HullPolygonData& poly)	const
		{
			return mPolygonVertexRefs + poly.mVRef8;
		}
	};

#if defined(PX_VC) 
    #pragma warning(push)
	#pragma warning( disable : 4251 ) // class needs to have dll-interface to be used by clients of class
#endif
	// PT: keep this one virtual-free for SPU please
	class PX_PHYSX_COMMON_API PolygonalBox
	{
	public:
									PolygonalBox(const PxVec3& halfSide);

			void					getPolygonalData(PolygonalData* PX_RESTRICT dst)	const;

			const PxVec3&			mHalfSide;
			PxVec3					mVertices[8];
			Gu::HullPolygonData		mPolygons[6];
	private:
			PolygonalBox& operator=(const PolygonalBox&);
	};
#if defined(PX_VC) 
     #pragma warning(pop) 
#endif

	PX_PHYSX_COMMON_API void getPolygonalData_Convex(PolygonalData* PX_RESTRICT dst, const Gu::ConvexHullData* PX_RESTRICT src, const Cm::FastVertex2ShapeScaling& scaling);
}
}

#endif
