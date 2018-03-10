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

#ifndef GU_VEC_CONVEX_H
#define GU_VEC_CONVEX_H

#include "CmPhysXCommon.h"
#include "PsVecMath.h"


#if defined(PX_X360) || defined(PX_PS3) || defined(__SPU__)
#define PX_USE_BRANCHLESS_SUPPORT
#endif


#if (defined __SPU__ )|| (defined(PX_PS3) && !(defined(__SNC__)) && defined _DEBUG)
#define PX_SUPPORT_INLINE __attribute__((noinline))
#define PX_SUPPORT_FORCE_INLINE PX_INLINE
#else
#define PX_SUPPORT_INLINE PX_FORCE_INLINE
#define PX_SUPPORT_FORCE_INLINE PX_FORCE_INLINE
#endif

namespace physx
{
namespace Gu
{
//to avoid LHS
#define E_CONVEXTYPE PxU32
#define E_CONVEXHULL 0
#define	E_CONVEXHULLNOSCALE	1
#define E_SPHERE 2
#define E_BOX 3
#define E_CAPSULE 4
#define E_CONE 5
#define E_CYLINDER 6
#define E_TRIANGLE 7
#define E_EXTRUDED_TRIANGLE 8
	
	class ConvexV
	{
	public:

		PX_FORCE_INLINE ConvexV(const E_CONVEXTYPE _type) : type(_type)
		{
			using namespace Ps::aos;
			const FloatV zero = FZero();
			margin = zero;
			minMargin = zero;
			marginDif = zero;
			bMarginIsRadius = BFFFF();

		}

		PX_FORCE_INLINE ConvexV(const E_CONVEXTYPE _type, const Ps::aos::Vec3VArg _center) 
		{
			using namespace Ps::aos;
			const FloatV zero = FZero();
			center = _center;
			type = _type;
			margin = zero;
			minMargin = zero;
			marginDif = zero;
			bMarginIsRadius = BFFFF();
		}

		PX_FORCE_INLINE ConvexV(const E_CONVEXTYPE _type, const Ps::aos::Vec3VArg _center, const Ps::aos::FloatVArg _margin)
		{
			using namespace Ps::aos;
			const FloatV ratio = Ps::aos::FLoad(0.25f);
			minMargin = FMul(_margin, ratio);
			center = _center;
			margin = _margin;
			marginDif = FZero();
			type = _type;
			bMarginIsRadius = BFFFF();
		}

		PX_FORCE_INLINE ConvexV(const E_CONVEXTYPE _type, const Ps::aos::Vec3VArg _center, const Ps::aos::FloatVArg _margin, const Ps::aos::FloatVArg _minMargin)
		{
			using namespace Ps::aos;
			minMargin = _minMargin;
			center = _center;
			margin = _margin;
			marginDif = FZero();
			type = _type;
			bMarginIsRadius = Ps::aos::BFFFF();
		}

		PX_FORCE_INLINE ConvexV(const E_CONVEXTYPE _type, const Ps::aos::Vec3VArg _center, const Ps::aos::FloatVArg _margin, const Ps::aos::FloatVArg _minMargin, const Ps::aos::FloatVArg _marginDif)
		{
			using namespace Ps::aos;
			minMargin = _minMargin;
			center = _center;
			margin = _margin;
			marginDif = _marginDif;
			type = _type;
			bMarginIsRadius = Ps::aos::BFFFF();
		}

		//everytime when someone transform the object, they need to up
		void setCenter(const Ps::aos::Vec3VArg _center)
		{
			center = _center;
		}

		void setMargin(const PxF32 _margin)
		{
			margin = Ps::aos::FLoad(_margin);
		}

		void setMargin(const Ps::aos::FloatVArg _margin)
		{
			margin = _margin;
		}

		void setMarginDif(const Ps::aos::FloatVArg _marginDif)
		{
			marginDif =_marginDif;
		}


		PX_FORCE_INLINE Ps::aos::Vec3V getCenter()const 
		{
			return center;
		}

		PX_FORCE_INLINE Ps::aos::FloatV getMargin() const
		{
			return margin;
		}

		PX_FORCE_INLINE Ps::aos::FloatV getMinMargin() const
		{
			return minMargin;
		}

		PX_FORCE_INLINE Ps::aos::FloatV getSweepMargin() const
		{
			return minMargin;
		}

		PX_FORCE_INLINE Ps::aos::FloatV getMarginDif()const 
		{
			return marginDif;
		}


		void getMargin(PxF32& _marginF) const
		{
			Ps::aos::FStore(margin, &_marginF);
		}


		E_CONVEXTYPE getType() const
		{
			return type;
		}


		PX_FORCE_INLINE Ps::aos::BoolV isMarginEqRadius()const
		{
			return bMarginIsRadius;
		}


	protected:
		~ConvexV(){}
		Ps::aos::Vec3V center;
		Ps::aos::FloatV margin;
		Ps::aos::FloatV minMargin;
		mutable Ps::aos::FloatV marginDif;
		Ps::aos::BoolV bMarginIsRadius;
		E_CONVEXTYPE	type;
		
	};

	PX_FORCE_INLINE Ps::aos::FloatV getContactEps(const Ps::aos::FloatV& _marginA, const Ps::aos::FloatV& _marginB)
	{
		using namespace Ps::aos;

		const FloatV ratio = FLoad(0.25f);
		const FloatV minMargin = FMin(_marginA, _marginB);
		
		return FMul(minMargin, ratio);
	}

	PX_FORCE_INLINE Ps::aos::FloatV getSweepContactEps(const Ps::aos::FloatV& _marginA, const Ps::aos::FloatV& _marginB)
	{
		using namespace Ps::aos;

		const FloatV ratio = FLoad(100.f);
		const FloatV minMargin = FAdd(_marginA, _marginB);
		
		return FMul(minMargin, ratio);
	}
}

}

#endif
