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

#ifndef GU_VEC_SHRUNK_BOX_H
#define GU_VEC_SHRUNK_BOX_H

/** \addtogroup geomutils
@{
*/
#include "PxPhysXCommonConfig.h"
#include "GuVecConvex.h"
#include "PsVecTransform.h"
#include "GuConvexSupportTable.h"



namespace physx
{

namespace Gu
{

	/**
	\brief Represents an oriented bounding box. 

	As a center point, extents(radii) and a rotation. i.e. the center of the box is at the center point, 
	the box is rotated around this point with the rotation and it is 2*extents in width, height and depth.
	*/

	/**
	Box geometry

	The rot member describes the world space orientation of the box.
	The center member gives the world space position of the box.
	The extents give the local space coordinates of the box corner in the positive octant.
	Dimensions of the box are: 2*extent.
	Transformation to world space is: worldPoint = rot * localPoint + center
	Transformation to local space is: localPoint = T(rot) * (worldPoint - center)
	Where T(M) denotes the transpose of M.
	*/

	class ShrunkBoxV : public BoxV
	{
	public:

		/**
		\brief Constructor
		*/
		PX_INLINE ShrunkBoxV() : BoxV()
		{
		}

		/**
		\brief Constructor

		\param _center Center of the OBB
		\param _extents Extents/radii of the obb.
		\param _rot rotation to apply to the obb.
		*/

		PX_FORCE_INLINE ShrunkBoxV(const Ps::aos::Vec3VArg origin, const Ps::aos::Vec3VArg extent) : 
																				BoxV(origin, extent)
		{
			using namespace Ps::aos;
			//calculate the marginDif
			const FloatV sqMargin = FMul(margin, margin);
			const FloatV tempMarginDif =  FSqrt(FAdd(sqMargin, FAdd(sqMargin, sqMargin)));
			marginDif = FSub(tempMarginDif, margin);
		}
		
		/**
		\brief Destructor
		*/
		PX_INLINE ~ShrunkBoxV()
		{
		}

		//! Assignment operator
		PX_INLINE const ShrunkBoxV& operator=(const ShrunkBoxV& other)
		{
			rot		= other.rot;
			center	= other.center;
			extents	= other.extents;
			margin =  other.margin;
			minMargin = other.minMargin;
			return *this;
		}


		PX_FORCE_INLINE Ps::aos::Vec3V supportPoint(const PxI32 index)const
		{
			using namespace Ps::aos;
			const Vec3V _extents = V3Sub(extents,  V3Splat(margin));
			const BoolV con = boxVertexTable[index];
			return V3Sel(con, _extents, V3Neg(_extents));
		}  

		//local space point
		PX_FORCE_INLINE Ps::aos::Vec3V supportLocal(const Ps::aos::Vec3VArg dir,  Ps::aos::Vec3V& support, PxI32& index)const
		{
			using namespace Ps::aos;
		
			const Vec3V zero = V3Zero();
			const Vec3V _extents = V3Sub(extents,  V3Splat(margin));
			const BoolV comp = V3IsGrtr(dir, zero);
			getIndex(comp, index);
			const Vec3V p = V3Sel(comp, _extents, V3Neg(_extents));
			support = p;
			return p;
		}


		PX_FORCE_INLINE Ps::aos::Vec3V supportRelative(const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aTob, Ps::aos::Vec3V& support, PxI32& index)const
		{
			using namespace Ps::aos;
		
			const Vec3V zero = V3Zero();
			const Vec3V _extents = V3Sub(extents,  Vec3V_From_FloatV(margin));
			//transfer dir into the local space of the box
			const Vec3V _dir =aTob.rotateInv(dir);//relTra.rotateInv(dir);
			const BoolV comp = V3IsGrtr(_dir, zero);
			getIndex(comp, index);
			const Vec3V p = V3Sel(comp, _extents, V3Neg(_extents));
			const Vec3V ret = aTob.transform(p);//relTra.transform(p);//V3Add(center, M33MulV3(rot, p));
			support = ret;
			return ret;
		}

	};
}	

}

/** @} */
#endif
