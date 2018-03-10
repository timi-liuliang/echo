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

#ifndef GU_PCM_SHAPE_CONVEX_H
#define GU_PCM_SHAPE_CONVEX_H

#include "GuConvexSupportTable.h"
#include "GuPersistentContactManifold.h"
#include "GuShapeConvex.h"

//#define PCM_LOW_LEVEL_DEBUG

#ifdef PCM_LOW_LEVEL_DEBUG
#ifndef __SPU__
extern physx::Gu::PersistentContactManifold* gManifold;
#endif
#endif

namespace physx
{

namespace Gu
{
	class GeometryUnion;

	extern const PxU8 gPCMBoxPolygonData[24];
	
	class PCMPolygonalBox
	{
	public:
									PCMPolygonalBox(const PxVec3& halfSide);

									void					getPolygonalData(Gu::PolygonalData* PX_RESTRICT dst)	const;

			const PxVec3&			mHalfSide;
			PxVec3					mVertices[8];
			Gu::HullPolygonData		mPolygons[6];
	private:
			PCMPolygonalBox& operator=(const PCMPolygonalBox&);
	};


	void getPCMConvexData(const Gu::ConvexHullV& convexHull,  const bool idtScale, Gu::PolygonalData& polyData);  
	bool getPCMConvexData(const Gu::GeometryUnion& shape, Cm::FastVertex2ShapeScaling& scaling, PxBounds3& bounds, Gu::PolygonalData& polyData);

}
}

#endif
