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

#ifndef GU_VEC_CAPSULE_H
#define GU_VEC_CAPSULE_H

/** \addtogroup geomutils
@{
*/

#include "GuVecConvex.h"   
#include "GuConvexSupportTable.h"


namespace physx
{
namespace Gu
{

	PX_FORCE_INLINE Ps::aos::FloatV CalculateCapsuleMinMargin(const Ps::aos::FloatVArg radius)
	{
		using namespace Ps::aos;
		const FloatV ratio = Ps::aos::FLoad(0.05f);
		return FMul(radius, ratio);
	}

	class CapsuleV : public ConvexV  
	{
	public:
		/**
		\brief Constructor
		*/

		PX_INLINE CapsuleV():ConvexV(E_CAPSULE)
		{
			bMarginIsRadius = Ps::aos::BTTTT();
		}

		//constructor for sphere
		PX_INLINE CapsuleV(const Ps::aos::Vec3VArg p, const Ps::aos::FloatVArg _radius) : ConvexV(E_CAPSULE)
		{
			using namespace Ps::aos;
			center = p;
			radius = _radius;
			p0 = p;
			p1 = p;
			margin = _radius;
			minMargin = _radius;
			bMarginIsRadius = Ps::aos::BTTTT(); 
		}

		PX_INLINE CapsuleV(const Ps::aos::Vec3VArg _center, const Ps::aos::Vec3VArg _v, const Ps::aos::FloatVArg _radius) : 
											ConvexV(E_CAPSULE, _center)
		{
			using namespace Ps::aos;
			radius = _radius;
			p0 = V3Add(_center, _v);
			p1 = V3Sub(_center, _v);
			margin = _radius;
			minMargin = _radius;
			bMarginIsRadius = Ps::aos::BTTTT();
		}


		PX_INLINE CapsuleV(const Ps::aos::Vec3VArg _center, const Ps::aos::Vec3VArg _v, const Ps::aos::FloatVArg _radius, const Ps::aos::FloatVArg _margin) : 
													ConvexV(E_CAPSULE, _center, _margin)
		{
			using namespace Ps::aos;
			radius = _radius;
			p0 = V3Add(_center, _v);
			p1 = V3Sub(_center, _v);
			bMarginIsRadius = Ps::aos::BTTTT();
		}

		/**
		\brief Constructor

		\param _radius Radius of the capsule.
		*/

		/**
		\brief Destructor
		*/
		PX_INLINE ~CapsuleV()
		{
		}

		PX_FORCE_INLINE void initialize(const Ps::aos::Vec3VArg _p0, const Ps::aos::Vec3VArg _p1, const Ps::aos::FloatVArg _radius)
		{
			using namespace Ps::aos;
			radius = _radius;
			p0 = _p0;
			p1 = _p1;
			margin = _radius;
			minMargin = _radius;
			center = V3Scale(V3Add(_p0, _p1), FHalf());
		}   

		PX_INLINE Ps::aos::Vec3V computeDirection() const
		{
			return Ps::aos::V3Sub(p1, p0);
		}

		
		PX_FORCE_INLINE	Ps::aos::FloatV	getRadius()	const
		{
			return radius;
		}


		PX_FORCE_INLINE Ps::aos::Vec3V supportPoint(const PxI32 index)const
		{
			return (&p0)[1-index];
		}


		PX_FORCE_INLINE void getIndex(const Ps::aos::BoolV con, PxI32& index)const
		{
			using namespace Ps::aos;
			const VecI32V v = VecI32V_From_BoolV(con);
			const VecI32V t = VecI32V_And(v, VecI32V_One());
			PxI32_From_VecI32V(t, &index);
		}

		PX_FORCE_INLINE void setCenter(const Ps::aos::Vec3VArg _center)
		{
			using namespace Ps::aos;
			Vec3V offset = V3Sub(_center, center);
			center = _center;

			p0 = V3Add(p0, offset);
			p1 = V3Add(p1, offset);

		}

		PX_FORCE_INLINE Ps::aos::Vec3V supportSweepLocal(const Ps::aos::Vec3VArg dir)const
		{
	
			return supportLocal(dir);
		}

		PX_FORCE_INLINE Ps::aos::Vec3V supportSweepRelative(const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aToB)const
		{
			return supportRelative(dir, aToB);
		}


		//dir, p0 and p1 are in the local space of dir
		PX_FORCE_INLINE Ps::aos::Vec3V supportLocal(const Ps::aos::Vec3VArg dir)const
		{
			using namespace Ps::aos;
			//const Vec3V _dir = V3Normalize(dir);
			const FloatV dist0 = V3Dot(p0, dir);
			const FloatV dist1 = V3Dot(p1, dir);
			return V3Sel(FIsGrtr(dist0, dist1), p0, p1);
		}

	
		PX_FORCE_INLINE Ps::aos::Vec3V supportRelative(const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aToB)const
		{
			using namespace Ps::aos;
			//transform dir into the local space of a
			const Vec3V _dir = aToB.rotateInv(dir);
			const Vec3V p = supportLocal(_dir);
			//transform p back to the local space of b
			return aToB.transform(p);
		}

		//dir, p0 and p1 are in the local space of dir
		PX_FORCE_INLINE Ps::aos::Vec3V supportLocal(const Ps::aos::Vec3VArg dir, PxI32& index)const
		{
			using namespace Ps::aos;
			
			const FloatV dist0 = V3Dot(p0, dir);
			const FloatV dist1 = V3Dot(p1, dir);
			const BoolV comp = FIsGrtr(dist0, dist1);
			getIndex(comp, index);
			return V3Sel(comp, p0, p1);
		}

	
		PX_FORCE_INLINE Ps::aos::Vec3V supportRelative(const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aToB, PxI32& index)const
		{
			using namespace Ps::aos;
			//transform dir into the local space of a
			const Vec3V _dir = aToB.rotateInv(dir);

			const Vec3V p = supportLocal(_dir, index);
			//transform p back to the local space of b
			return aToB.transform(p);
		}



		//dir, p0 and p1 are in the local space of dir
		PX_FORCE_INLINE Ps::aos::Vec3V supportLocal(const Ps::aos::Vec3VArg dir, Ps::aos::Vec3V& support, PxI32& index)const 
		{
			using namespace Ps::aos;

			const Vec3V ret = supportLocal(dir, index);
			support = ret;
			return ret;
		}

		PX_FORCE_INLINE Ps::aos::BoolV supportLocalIndex(const Ps::aos::Vec3VArg dir, PxI32& index)const
		{
			using namespace Ps::aos;
			//scale dir and put it in the vertex space
			const FloatV dist0 = V3Dot(p0, dir);
			const FloatV dist1 = V3Dot(p1, dir);
			const BoolV comp = FIsGrtr(dist0, dist1);
			getIndex(comp, index);
			return comp;
		}

		PX_FORCE_INLINE Ps::aos::Vec3V supportLocal(Ps::aos::Vec3V& support, const PxI32& index, const Ps::aos::BoolV comp)const
		{
			PX_UNUSED(index);

			using namespace Ps::aos;
			const Vec3V p = V3Sel(comp, p0, p1);
			support = p;
			return p;
		}

		
		PX_FORCE_INLINE Ps::aos::Vec3V supportRelative(const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aToB, Ps::aos::Vec3V& support, PxI32& index)const 
		{
			using namespace Ps::aos;
			const Vec3V ret = supportRelative(dir, aToB, index);
			support = ret;
			return ret;
		}

		PX_FORCE_INLINE Ps::aos::BoolV supportRelativeIndex(const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aTob, PxI32& index)const
		{
			using namespace Ps::aos;
			//scale dir and put it in the vertex space
			const Vec3V _dir =aTob.rotateInv(dir);//relTra.rotateInv(dir);
			return supportLocalIndex(_dir, index);
		}

		PX_FORCE_INLINE Ps::aos::Vec3V supportRelative(Ps::aos::Vec3V& support, const Ps::aos::PsMatTransformV& aTob, const PxI32& index, const Ps::aos::BoolV comp)const
		{
			using namespace Ps::aos;

			PX_UNUSED(index);

			const Vec3V p = V3Sel(comp, p0, p1);
			//transfer p into the b space
			const Vec3V ret = aTob.transform(p);//relTra.transform(p);//V3Add(center, M33MulV3(rot, p));
			support = ret;
			return ret;
		}

		PX_FORCE_INLINE Ps::aos::FloatV getSweepMargin() const
		{
			return Ps::aos::FZero();
		}

		//don't change the order of p0 and p1, the getPoint function depend on the order
		Ps::aos::Vec3V	p0;		//!< Start of segment
		Ps::aos::Vec3V	p1;		//!< End of segment
		Ps::aos::FloatV	radius;
	};
}

}

#endif
