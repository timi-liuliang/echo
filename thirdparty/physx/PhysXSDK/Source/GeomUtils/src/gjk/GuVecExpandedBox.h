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

#ifndef GU_VEC_EXPANDED_BOX_H
#define GU_VEC_EXPANDED_BOX_H

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

	/*
		ML:
		This class is just used in EPA code
	*/
	class ExpandedBoxV : public BoxV
	{
	public:

		/**
		\brief Constructor
		*/
		PX_INLINE ExpandedBoxV() : BoxV()
		{
		}

		/**
		\brief Constructor

		\param _center Center of the OBB
		\param _extents Extents/radii of the obb.
		\param _rot rotation to apply to the obb.
		*/


		//! Construct from center, extent and rotation
		PX_FORCE_INLINE ExpandedBoxV(const Ps::aos::Vec3VArg origin, const Ps::aos::Vec3VArg extent, const Ps::aos::Mat33V& base) : 
																											BoxV(origin, extent, base)
		{
		}

		PX_FORCE_INLINE ExpandedBoxV(const Ps::aos::Vec3VArg origin, const Ps::aos::Vec3VArg extent) : 
																				BoxV(origin, extent)
		{
		}

		PX_FORCE_INLINE ExpandedBoxV(const Ps::aos::Vec3VArg origin, const Ps::aos::Vec3VArg extent, 
			const Ps::aos::Vec3VArg col0, const Ps::aos::Vec3VArg col1, const Ps::aos::Vec3VArg col2) : 
																									BoxV(origin, extent, col0, col1, col2)
		{
		}


		//! Copy constructor
		PX_FORCE_INLINE ExpandedBoxV(const ExpandedBoxV& other) : BoxV(other)
		{
		}

		
		/**
		\brief Destructor
		*/
		PX_INLINE ~ExpandedBoxV()
		{
		}

		//! Assignment operator
		PX_INLINE const ExpandedBoxV& operator=(const ExpandedBoxV& other)
		{
			rot		= other.rot;
			center	= other.center;
			extents	= other.extents;
			margin =  other.margin;
			minMargin = other.minMargin;
			return *this;
		}

		//this is used in the warm start to get the closest feature point out
		PX_FORCE_INLINE Ps::aos::Vec3V supportPoint(const PxI32 index)const
		{
			using namespace Ps::aos;
			const Vec3V _extents = V3Add(extents,  V3Splat(margin));
			const BoolV con = boxVertexTable[index];
			return V3Sel(con, _extents, V3Neg(_extents));
		}
		
		//local space point
		PX_FORCE_INLINE Ps::aos::Vec3V supportLocal(const Ps::aos::Vec3VArg dir)const
		{
			using namespace Ps::aos;
		
			const Vec3V zero = V3Zero();
			const Vec3V _extents = V3Add(extents,  V3Splat(margin));
			const BoolV comp = V3IsGrtr(dir, zero);
			return V3Sel(comp, _extents, V3Neg(_extents));
		}


		PX_FORCE_INLINE Ps::aos::Vec3V supportRelative(const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aTob)const  
		{

			//a is the current object, b is the other object, dir is in the local space of b
			using namespace Ps::aos;

			const Vec3V _dir = aTob.rotateInv(dir);//relTra.rotateInv(dir);//from b to a
			const Vec3V p =  supportLocal(_dir);
			//transfer p into the b space
			return aTob.transform(p);//relTra.transform(p);
		}
	
	};
}	//PX_COMPILE_TIME_ASSERT(sizeof(Gu::BoxV) == 96);

}

/** @} */
#endif
