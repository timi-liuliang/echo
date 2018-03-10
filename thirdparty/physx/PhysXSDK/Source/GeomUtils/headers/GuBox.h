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

#ifndef GU_BOX_H
#define GU_BOX_H

/** \addtogroup geomutils
@{
*/

#include "PxPhysXCommonConfig.h"
#include "PxTransform.h"
#include "CmPhysXCommon.h"
#include "foundation/PxMat33.h"

namespace physx
{
namespace Gu
{
	class Capsule;

	void PX_PHYSX_COMMON_API computeOBBPoints(PxVec3* PX_RESTRICT pts, const PxVec3& center, const PxVec3& extents, const PxVec3& base0, const PxVec3& base1, const PxVec3& base2);


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
#if defined(PX_VC) 
    #pragma warning(push)
	#pragma warning( disable : 4251 ) // class needs to have dll-interface to be used by clients of class
#endif
	class PX_PHYSX_COMMON_API Box
	{
	public:
		/**
		\brief Constructor
		*/
		PX_FORCE_INLINE Box()
		{
		}

		/**
		\brief Constructor

		\param _center Center of the OBB
		\param _extents Extents/radii of the obb.
		\param _rot rotation to apply to the obb.
		*/
		//! Construct from center, extent and rotation
		PX_FORCE_INLINE Box(const PxVec3& origin, const PxVec3& extent, const PxMat33& base) : rot(base), center(origin), extents(extent)
		{}

		//! Copy constructor
		PX_FORCE_INLINE Box(const Box& other) : rot(other.rot), center(other.center), extents(other.extents)
		{}

		/**
		\brief Destructor
		*/
		PX_FORCE_INLINE ~Box()
		{
		}

		//! Assignment operator
		PX_FORCE_INLINE const Box& operator=(const Box& other)
		{
			rot		= other.rot;
			center	= other.center;
			extents	= other.extents;
			return *this;
		}

		/**
		\brief Setups an empty box.
		*/
		PX_INLINE void setEmpty()
		{
			center = PxVec3(0);
			extents = PxVec3(-PX_MAX_REAL, -PX_MAX_REAL, -PX_MAX_REAL);
			rot = PxMat33(PxIdentity);
		}

		/**
		\brief Checks the box is valid.

		\return	true if the box is valid
		*/
		PX_INLINE bool isValid() const
		{
			// Consistency condition for (Center, Extents) boxes: Extents >= 0.0f
			if(extents.x < 0.0f)	return false;
			if(extents.y < 0.0f)	return false;
			if(extents.z < 0.0f)	return false;
			return true;
		}

/////////////
		PX_FORCE_INLINE	void	setAxes(const PxVec3& axis0, const PxVec3& axis1, const PxVec3& axis2)
		{
			rot.column0 = axis0;
			rot.column1 = axis1;
			rot.column2 = axis2;
		}

		PX_FORCE_INLINE	PxVec3	rotate(const PxVec3& src)	const
		{
			return rot * src;
		}

		PX_FORCE_INLINE	PxVec3	rotateInv(const PxVec3& src)	const
		{
			return rot.transformTranspose(src);
		}

		PX_FORCE_INLINE	PxVec3	transform(const PxVec3& src)	const
		{
			return rot * src + center;
		}

		PX_FORCE_INLINE	PxTransform getTransform()	const
		{
			return PxTransform(center, PxQuat(rot));
		}

		PX_INLINE PxVec3 computeAABBExtent() const
		{
			const PxReal a00 = PxAbs(rot[0][0]);
			const PxReal a01 = PxAbs(rot[0][1]);
			const PxReal a02 = PxAbs(rot[0][2]);

			const PxReal a10 = PxAbs(rot[1][0]);
			const PxReal a11 = PxAbs(rot[1][1]);
			const PxReal a12 = PxAbs(rot[1][2]);

			const PxReal a20 = PxAbs(rot[2][0]);
			const PxReal a21 = PxAbs(rot[2][1]);
			const PxReal a22 = PxAbs(rot[2][2]);

			const PxReal ex = extents.x;
			const PxReal ey = extents.y;
			const PxReal ez = extents.z;

			return PxVec3(	a00 * ex + a10 * ey + a20 * ez,
							a01 * ex + a11 * ey + a21 * ez,
							a02 * ex + a12 * ey + a22 * ez);
		}

		/**
		Computes the obb points.
		\param		pts	[out] 8 box points
		\return		true if success
		*/
		// AP: ok on SPU - doesn't seem to inline with PX_INLINE
		PX_INLINE void computeBoxPoints(PxVec3* PX_RESTRICT pts) const
		{
			Gu::computeOBBPoints(pts, center, extents, rot.column0, rot.column1, rot.column2);
		}

		void create(const Gu::Capsule& capsule);

		PxMat33	rot;
		PxVec3	center;
		PxVec3	extents;
	};
	PX_COMPILE_TIME_ASSERT(sizeof(Gu::Box) == 60);

#if defined(PX_VC) 
     #pragma warning(pop) 
#endif

}

}

/** @} */
#endif
