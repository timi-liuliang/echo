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


#ifndef PX_PHYSICS_COMMON_VECTOR
#define PX_PHYSICS_COMMON_VECTOR

#include "PxVec3.h"
#include "CmPhysXCommon.h"

/*!
Combination of two R3 vectors.
*/

namespace physx
{
namespace Cm
{
	PX_ALIGN_PREFIX(16)
	class SpatialVector
	{
	public:
		//! Default constructor
		PX_FORCE_INLINE SpatialVector()
		{}

		//! Construct from two PxcVectors
		PX_FORCE_INLINE SpatialVector(const PxVec3& lin, const PxVec3& ang)
			: linear(lin), pad0(0.0f), angular(ang), pad1(0.0f)
		{
		}

		PX_FORCE_INLINE ~SpatialVector()
		{}



		// PT: this one is very important. Without it, the Xbox compiler generates weird "float-to-int" and "int-to-float" LHS
		// each time we copy a SpatialVector (see for example PIX on "solveSimpleGroupA" without this operator).
		PX_FORCE_INLINE	void	operator = (const SpatialVector& v)
		{
			linear = v.linear;
			pad0 = 0.0f;
			angular = v.angular;
			pad1 = 0.0f;
		}


		static PX_FORCE_INLINE SpatialVector zero() {	return SpatialVector(PxVec3(0),PxVec3(0)); }

		PX_FORCE_INLINE SpatialVector operator+(const SpatialVector& v) const
		{
			return SpatialVector(linear+v.linear,angular+v.angular);
		}

		PX_FORCE_INLINE SpatialVector operator-(const SpatialVector& v) const
		{
			return SpatialVector(linear-v.linear,angular-v.angular);
		}

		PX_FORCE_INLINE SpatialVector operator-() const
		{
			return SpatialVector(-linear,-angular);
		}


		PX_FORCE_INLINE SpatialVector operator *(PxReal s) const
		{	
			return SpatialVector(linear*s,angular*s);	
		}
		
		PX_FORCE_INLINE void operator+=(const SpatialVector& v)
		{
			linear+=v.linear;
			angular+=v.angular;
		}

		PX_FORCE_INLINE void operator-=(const SpatialVector& v)
		{
			linear-=v.linear;
			angular-=v.angular;
		}

		PX_FORCE_INLINE PxReal magnitude()	const
		{
			return angular.magnitude() + linear.magnitude();
		}

		PX_FORCE_INLINE PxReal dot(const SpatialVector& v) const
		{
			return linear.dot(v.linear) + angular.dot(v.angular);
		}
		
		PX_FORCE_INLINE bool isFinite() const
		{
			return linear.isFinite() && angular.isFinite();
		}

		PX_FORCE_INLINE Cm::SpatialVector scale(PxReal l, PxReal a) const
		{
			return Cm::SpatialVector(linear*l, angular*a);
		}

		PxVec3 linear;
		PxReal pad0;
		PxVec3 angular;
		PxReal pad1;
	}
	PX_ALIGN_SUFFIX(16);

} // namespace Cm

PX_COMPILE_TIME_ASSERT(sizeof(Cm::SpatialVector) == 32);

}

#endif
