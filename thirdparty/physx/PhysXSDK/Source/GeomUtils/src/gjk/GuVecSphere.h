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

#ifndef GU_VEC_SPHERE_H
#define GU_VEC_SPHERE_H
/** \addtogroup geomutils
@{
*/

#include "GuVecConvex.h"
#include "GuConvexSupportTable.h"

/**
\brief Represents a sphere defined by its center point and radius.
*/
namespace physx
{
namespace Gu
{
	class SphereV : public ConvexV
	{
	public:
		/**
		\brief Constructor
		*/
		PX_INLINE SphereV(): ConvexV(E_SPHERE)
		{
			radius = Ps::aos::FZero();
			bMarginIsRadius = Ps::aos::BTTTT();
		}

		PX_INLINE SphereV(const Ps::aos::Vec3VArg _center, const Ps::aos::FloatV _radius): ConvexV(E_SPHERE, _center)
		{
			radius = _radius;
			margin = _radius;
			minMargin = _radius;
			bMarginIsRadius = Ps::aos::BTTTT();
		}


		/*
			Margin should be the same as radius
		*/
		PX_INLINE SphereV(const Ps::aos::Vec3VArg _center, const Ps::aos::FloatVArg _radius, const Ps::aos::FloatVArg _margin): ConvexV(E_SPHERE, _center, _margin)
		{
			radius = _radius;
			minMargin =  _margin;
			bMarginIsRadius = Ps::aos::BTTTT();
		}

		/**
		\brief Copy constructor
		*/
		PX_INLINE SphereV(const SphereV& sphere) : ConvexV(E_SPHERE, sphere.center, sphere.margin), radius(sphere.radius)
		{
			minMargin = sphere.margin;
			bMarginIsRadius = Ps::aos::BTTTT();
		}

		/**
		\brief Destructor
		*/
		PX_INLINE ~SphereV()
		{
		}

		PX_INLINE	void	setV(const Ps::aos::Vec3VArg _center, const Ps::aos::FloatVArg _radius)		
		{ 
			center = _center;
			radius = _radius;
		}

		/**
		\brief Checks the sphere is valid.

		\return		true if the sphere is valid
		*/
		PX_INLINE bool isValid() const
		{
			// Consistency condition for spheres: Radius >= 0.0f
			using namespace Ps::aos;
			return BAllEq(FIsGrtrOrEq(radius, FZero()), BTTTT()) != 0;
		}

		/**
		\brief Tests if a point is contained within the sphere.

		\param[in] p the point to test
		\return	true if inside the sphere
		*/
		PX_INLINE bool contains(const Ps::aos::Vec3VArg p) const
		{
			using namespace Ps::aos;
			const FloatV rr = FMul(radius, radius);
			const FloatV cc =  V3LengthSq(V3Sub(center, p));
			return FAllGrtrOrEq(rr, cc) != 0;
		}

		/**
		\brief Tests if a sphere is contained within the sphere.

		\param		sphere	[in] the sphere to test
		\return		true if inside the sphere
		*/
		PX_INLINE bool contains(const SphereV& sphere)	const
		{
			using namespace Ps::aos;
			
			const Vec3V centerDif= V3Sub(center, sphere.center);
			const FloatV radiusDif = FSub(radius, sphere.radius);
			const FloatV cc = V3Dot(centerDif, centerDif);
			const FloatV rr = FMul(radiusDif, radiusDif); 

			const BoolV con0 = FIsGrtrOrEq(radiusDif, FZero());//might contain
			const BoolV con1 = FIsGrtr(rr, cc);//return true
			return BAllEq(BAnd(con0, con1), BTTTT())==1;
		}

		/**
		\brief Tests if a box is contained within the sphere.

		\param		minimum		[in] minimum value of the box
		\param		maximum		[in] maximum value of the box
		\return		true if inside the sphere
		*/
		PX_INLINE bool contains(const Ps::aos::Vec3VArg minimum, const Ps::aos::Vec3VArg maximum) const
		{
		
			//compute the sphere which wrap around the box
			using namespace Ps::aos;
			const FloatV zero = FZero();
			const FloatV half = FHalf();

			const Vec3V boxSphereCenter = V3Scale(V3Add(maximum, minimum), half);
			const Vec3V v = V3Scale(V3Sub(maximum, minimum), half);
			const FloatV boxSphereR = V3Length(v);

			const Vec3V w = V3Sub(center, boxSphereCenter);
			const FloatV wLength = V3Length(w);
			const FloatV dif = FSub(FSub(radius, wLength), boxSphereR); 

			return FAllGrtrOrEq(dif, zero) != 0;
		}           
                  
		/**
		\brief Tests if the sphere intersects another sphere

		\param		sphere	[in] the other sphere
		\return		true if spheres overlap
		*/
		PX_INLINE bool intersect(const SphereV& sphere) const
		{
			using namespace Ps::aos;
			const Vec3V centerDif = V3Sub(center, sphere.center);
			const FloatV cc = V3Dot(centerDif, centerDif);
			const FloatV r = FAdd(radius, sphere.radius);
			const FloatV rr = FMul(r, r);
			return FAllGrtrOrEq(rr, cc) != 0;
		}

		//return point in local space
		PX_FORCE_INLINE Ps::aos::Vec3V getPoint(const PxU8)
		{
			return Ps::aos::V3Zero();
		}
  //  
		//sweep code need to have full version
		PX_FORCE_INLINE Ps::aos::Vec3V supportSweep(const Ps::aos::Vec3VArg dir)const
		{
			using namespace Ps::aos;
			const Vec3V _dir = V3Normalize(dir);
			return V3ScaleAdd(_dir, radius, center);  
		}

		//make the support function the same as support margin
		PX_FORCE_INLINE Ps::aos::Vec3V support(const Ps::aos::Vec3VArg)const
		{
			return center;//_margin is the same as radius
		}
  

		PX_FORCE_INLINE Ps::aos::Vec3V supportMargin(const Ps::aos::Vec3VArg dir, const Ps::aos::FloatVArg _margin, Ps::aos::Vec3V& support)const
		{
			PX_UNUSED(_margin);
			PX_UNUSED(dir);

			support = center;
			return center;//_margin is the same as radius
		}

		PX_FORCE_INLINE Ps::aos::BoolV isMarginEqRadius()const
		{
			return Ps::aos::BTTTT();
		}

		PX_FORCE_INLINE Ps::aos::FloatV getSweepMargin() const
		{
			return Ps::aos::FZero();
		}


		Ps::aos::FloatV radius;		//!< Sphere's center, w component is radius

	};
}

}

#endif
