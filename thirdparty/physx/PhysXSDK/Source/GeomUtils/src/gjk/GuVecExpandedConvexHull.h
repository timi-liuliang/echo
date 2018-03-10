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

#ifndef GU_VEC_EXPANDED_CONVEX_HULL_H
#define GU_VEC_EXPANDED_CONVEX_HULL_H

#include "PxPhysXCommonConfig.h"
#include "GuVecConvex.h"
#include "GuConvexMeshData.h"
#include "GuBigConvexData.h"
#include "GuConvexSupportTable.h"
#include "GuCubeIndex.h"
#include "GuVecConvexHull.h"
#include "PsFPU.h"

  
namespace physx
{
namespace Gu
{

	class ExpandedConvexHullV : public ConvexHullV
	{


		public:
		/**
		\brief Constructor
		*/
		PX_SUPPORT_INLINE ExpandedConvexHullV(): ConvexHullV()
		{
		}

		PX_SUPPORT_INLINE ExpandedConvexHullV(const Gu::ConvexHullData* _hullData, const Ps::aos::Vec3VArg _center, const Ps::aos::Vec3VArg scale, const Ps::aos::QuatVArg scaleRot):
													ConvexHullV(_hullData, _center, scale, scaleRot)
		{
		}


		PX_SUPPORT_INLINE ExpandedConvexHullV(const Gu::ConvexHullData* _hullData, const Ps::aos::Vec3VArg _center, const Ps::aos::FloatVArg _margin, const Ps::aos::FloatVArg _minMargin, const Ps::aos::Vec3VArg scale, const Ps::aos::QuatVArg scaleRot) : 
													ConvexHullV(_hullData, _center, _margin, _minMargin, scale, scaleRot)
		{
		}

	

		PX_FORCE_INLINE Ps::aos::Vec3V supportPoint(const PxI32 index)const
		{
			using namespace Ps::aos;
			return M33MulV3(vertex2Shape, V3LoadU(verts[index]));
		}

	
		//dir is in the shape space
		PX_SUPPORT_INLINE Ps::aos::Vec3V supportLocal(const Ps::aos::Vec3VArg dir)const
		{
			using namespace Ps::aos;
			//scale dir and put it in the vertex space
			const Vec3V n = V3Normalize(dir);
			const Vec3V _dir = M33TrnspsMulV3(vertex2Shape, dir);
			//const Vec3V maxPoint = supportVertex(_dir);
			const PxU32 maxIndex = supportVertexIndex(_dir);
			const Vec3V p=M33MulV3(vertex2Shape, V3LoadU(verts[maxIndex]));
			return V3ScaleAdd(n, margin, p);
		}
		

		PX_SUPPORT_INLINE Ps::aos::Vec3V supportRelative(const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aTob)const
		{
			using namespace Ps::aos;
		
			//transform dir into the shape space
			const Vec3V _dir = aTob.rotateInv(dir);//relTra.rotateInv(dir);
			const Vec3V maxPoint =supportLocal(_dir);
			//translate maxPoint from shape space of a back to the b space
			return aTob.transform(maxPoint);//relTra.transform(maxPoint);
		}

		
		//dir is in the shape space
		PX_SUPPORT_INLINE Ps::aos::Vec3V supportLocal(const Ps::aos::Vec3VArg dir, Ps::aos::Vec3V& support, PxI32& index)const
		{
			using namespace Ps::aos;
			//scale dir and put it in the vertex space
			const Vec3V n = V3Normalize(dir);
			const Vec3V _dir = M33TrnspsMulV3(vertex2Shape, dir);
			//const Vec3V maxPoint = supportVertex(_dir);
			const PxU32 maxIndex = supportVertexIndex(_dir);
			index = maxIndex;

			//transfrom the vertex from vertex space to shape space
			const Vec3V pInShape=M33MulV3(vertex2Shape, V3LoadU(verts[maxIndex]));
			const Vec3V p=V3ScaleAdd(n, margin, pInShape);
			support = p;
			return p;
		}


		PX_SUPPORT_INLINE Ps::aos::Vec3V supportRelative(const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aTob, Ps::aos::Vec3V& support, PxI32& index)const
		{
			using namespace Ps::aos;
		
			//transform dir into the shape space
			const Vec3V _dir = aTob.rotateInv(dir);//relTra.rotateInv(dir);

			const Vec3V maxPoint =supportLocal(_dir, support, index);

			//translate maxPoint from shape space of a back to the b space
			const Vec3V ret=aTob.transform(maxPoint);//relTra.transform(maxPoint);

			support = ret;
			return ret;
		}
	};


}

}

#endif	// 
