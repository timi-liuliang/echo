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

#ifndef GU_GJK_H
#define GU_GJK_H


#include "GuConvexSupportTable.h"
#include "GuGJKWrapper.h"
#include "GuGJKSimplex.h"
#include "GuGJKSimplexTesselation.h"
#include "GuGJKFallBack.h"




namespace physx
{
namespace Gu
{

	class ConvexV;



#ifndef	__SPU__


	template<class ConvexA, class ConvexB>
	PxGJKStatus gjkRelative(const ConvexA& a, const ConvexB& b, const Ps::aos::PsMatTransformV& aToB, Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& sqDist)
	{
		PX_SIMD_GUARD;

		using namespace Ps::aos;
		Vec3V Q[4];
		Vec3V A[4];
		Vec3V B[4];

		const FloatV zero = FZero();
		const BoolV bTrue = BTTTT();
		PxU32 size=0;

		const Vec3V _initialSearchDir = aToB.p;
		Vec3V closest = V3Sel(FIsGrtr(V3Dot(_initialSearchDir, _initialSearchDir), zero), _initialSearchDir, V3UnitX());

		
		// ML: eps2 is the square value of an epsilon value which applied in the termination condition for two shapes overlap.
		// GJK will terminate based on sq(v) < eps2 and indicate that two shapes are overlapping.
		// we calculate the eps2 based on 10% of the minimum margin of two shapes
		const FloatV tenPerc = FLoad(0.1f);
		const FloatV minMargin = FMin(a.getMinMargin(), b.getMinMargin());
		const FloatV _eps2 = FMul(minMargin, tenPerc);
		const FloatV eps2 = FMul(_eps2, _eps2);

		// ML:epsRel is square value of 1.5% which applied to the square distance of a closest point(v) to the origin.
		// If sq(v)- v.dot(w) < epsRel*sq(v),
		// two shapes are clearly separate, GJK terminate and return non intersect.
		// This adjusts the termination condition based on the length of v
		// which avoids ill-conditioned terminations. 
		const FloatV epsRel = FLoad(0.000225f);//1.5%.

		FloatV sDist = FMax();
		FloatV minDist = sDist;
		Vec3V prevClosest = closest;
		
		BoolV bNotTerminated = bTrue;
		BoolV bCon = bTrue;
		
		do
		{
			minDist = sDist;
			prevClosest = closest;

			const Vec3V supportA=a.supportRelative(V3Neg(closest), aToB);
			const Vec3V supportB=b.supportLocal(closest);
			
			//calculate the support point
			const Vec3V support = V3Sub(supportA, supportB);
			const FloatV signDist = V3Dot(closest, support);
			const FloatV tmp0 = FSub(sDist, signDist);

			PX_ASSERT(size < 4);
			A[size]=supportA;
			B[size]=supportB;
			Q[size]=support;   
	
			if(FAllGrtr(FMul(epsRel, sDist), tmp0))
			{
				getClosestPoint(Q, A, B, closest, closestA, closestB, size);
				const Vec3V n = V3Normalize(V3Neg(closest));
				sqDist = sDist;
				normal = n;
				return GJK_NON_INTERSECT;
			}  
			size++;

			//calculate the closest point between two convex hull
			closest = GJKCPairDoSimplex(Q, A, B, support, size);
			sDist = V3Dot(closest, closest);
			bCon = FIsGrtr(minDist, sDist);
			bNotTerminated = BAnd(FIsGrtr(sDist, eps2), bCon);
		}while(BAllEq(bNotTerminated, bTrue));
		
		getClosestPoint(Q, A, B, prevClosest, closestA, closestB, size);
		normal = V3Normalize(V3Neg(prevClosest));
		sqDist = FSel(bCon, sDist, minDist);
		
		return PxGJKStatus(BAllEq(bCon, bTrue) == 1 ? GJK_CONTACT : GJK_DEGENERATE);
		
	}

	template<class ConvexA, class ConvexB>
	PxGJKStatus gjkLocal(const ConvexA& a, const ConvexB& b, Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& sqDist)
	{
		PX_SIMD_GUARD;

		using namespace Ps::aos;
		Vec3V Q[4];
		Vec3V A[4];
		Vec3V B[4];
		const FloatV zero = FZero();
		const BoolV bTrue = BTTTT();
		PxU32 size=0;

		const Vec3V _initialSearchDir = V3Sub(a.getCenter(), b.getCenter());
		Vec3V v = V3Sel(FIsGrtr(V3Dot(_initialSearchDir, _initialSearchDir), zero), _initialSearchDir, V3UnitX());


		//ML: eps2 is the square value of an epsilon value which applied in the termination condition for two shapes overlap. GJK will terminate based on sq(v) < eps2 and indicate that two shapes are overlap. 
		//we calculate the eps2 based on 10% of the minimum margin of two shapes
		const FloatV tenPerc = FLoad(0.1f);
		const FloatV minMargin = FMin(a.getMinMargin(), b.getMinMargin());
		const FloatV _eps2 = FMul(minMargin, tenPerc);
		const FloatV eps2 = FMul(_eps2, _eps2);
		//ML:epsRel is square value of 1.5% which applied to the square distance of a closest point(v) to the origin. If sq(v)- v.dot(w) < epsRel*sq(v),
		//two shapes are clearly separate, GJK terminate and return non intersect. This adjusts the termination condition based on the length of v
		//which avoids ill-conditioned terminations. 
		const FloatV epsRel = FLoad(0.000225f);//1.5%.
		
		FloatV sDist = FMax();
		FloatV minDist = sDist;

		
		BoolV bNotTerminated = bTrue;
		BoolV bCon = bTrue;
		Vec3V prevV = v;
		
		do
		{
			minDist = sDist;
			prevV = v;

			const Vec3V supportA=a.supportLocal(V3Neg(v));
			const Vec3V supportB=b.supportLocal(v);
			
			//calculate the support point
			const Vec3V support = V3Sub(supportA, supportB);
			const FloatV signDist = V3Dot(v, support);
			const FloatV tmp0 = FSub(sDist, signDist);

			PX_ASSERT(size < 4);
			A[size]=supportA;
			B[size]=supportB;
			Q[size]=support;
	
			if(FAllGrtr(FMul(epsRel, sDist), tmp0))
			{
				getClosestPoint(Q, A, B, v, closestA, closestB, size);
				normal = V3Normalize(V3Neg(v));
				sqDist = sDist;
				return GJK_NON_INTERSECT;
			}

			size++;

			//calculate the closest point between two convex hull
			const Vec3V tempV = GJKCPairDoSimplex(Q, A, B, support, size);
			v = tempV;
			sDist = V3Dot(v, v);
			bCon = FIsGrtr(minDist, sDist);
			bNotTerminated = BAnd(FIsGrtr(sDist, eps2), bCon);
		}while(BAllEq(bNotTerminated, bTrue));


		getClosestPoint(Q, A, B, v, closestA, closestB, size);
		normal = V3Normalize(V3Neg(prevV));
		sqDist = FSel(bCon, sDist, minDist);
		return PxGJKStatus(BAllEq(bCon, bTrue) == 1 ? GJK_CONTACT : GJK_DEGENERATE);
	}


	template<class ConvexA, class ConvexB>
	PxGJKStatus gjkRelativeTesselation(const ConvexA& a, const ConvexB& b, const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg sqTolerance, Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& sqDist)
	{
		using namespace Ps::aos;
		PxGJKStatus status = gjkRelative(a, b, aToB, closestA, closestB, normal, sqDist);
		if(status != GJK_CONTACT)
		{
			status = FAllGrtr(sqTolerance, sqDist)? (PxGJKStatus)GJK_CONTACT : status == GJK_DEGENERATE ? (PxGJKStatus)GJK_DEGENERATE : (PxGJKStatus)GJK_NON_INTERSECT;
			if(status == GJK_DEGENERATE)
			{
				SupportMapRelativeImpl<ConvexA> map0(a, aToB);
				SupportMapLocalImpl<ConvexB>	map1(b);
				status = gjkRelativeFallback(a, b, &map0, &map1, aToB.p, closestA, closestB, normal, sqDist);
				if(status == GJK_DEGENERATE)
				{
					status = PxGJKStatus(FAllGrtr(sqTolerance, sqDist)? GJK_CONTACT : GJK_NON_INTERSECT);
				}
			}
		}
		return status;
	}


#else
	

	PxGJKStatus gjk(const ConvexV& a, const ConvexV& b, SupportMap* map1, SupportMap* map2, const Ps::aos::Vec3VArg initialDir,  Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& sqDist);
	
	template<class ConvexA, class ConvexB>
	PxGJKStatus gjkRelative(const ConvexA& a, const ConvexB& b, const Ps::aos::PsMatTransformV& aToB, Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& sqDist)
	{
		SupportMapRelativeImpl<ConvexA> map1(a, aToB);
		SupportMapLocalImpl<ConvexB> map2(b);
		return gjk(a, b, &map1, &map2, aToB.p, closestA, closestB, normal, sqDist );

	}

	template<class ConvexA, class ConvexB>
	PxGJKStatus gjkLocal(const ConvexA& a, const ConvexB& b, Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& sqDist)
	{
		using namespace Ps::aos;
	
		SupportMapLocalImpl<ConvexA> map1(a);
		SupportMapLocalImpl<ConvexB> map2(b);
		const Vec3V initialDir = V3Sub(a.getCenter(), b.getCenter());
		return gjk(a, b, &map1, &map2, initialDir, closestA, closestB, normal, sqDist);
	}

	template<class ConvexA, class ConvexB>
	PxGJKStatus gjkRelativeTesselation(const ConvexA& a, const ConvexB& b, const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg sqTolerance, Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& sqDist)
	{
		using namespace Ps::aos;
		SupportMapRelativeImpl<ConvexA> map1(a, aToB);
		SupportMapLocalImpl<ConvexB> map2(b);
		PxGJKStatus status = gjk(a, b, &map1, &map2, aToB.p, closestA, closestB, normal, sqDist);
		if(status == GJK_DEGENERATE)
		{
			status = FAllGrtr(sqTolerance, sqDist)? GJK_CONTACT : GJK_NON_INTERSECT;
			if(status == GJK_NON_INTERSECT)
			{
				status = gjkRelativeFallback(a, b, &map1, &map2, aToB.p, closestA, closestB, normal, sqDist);
				if(status == GJK_DEGENERATE)
				{
					status = FAllGrtr(sqTolerance, sqDist)? GJK_CONTACT : GJK_NON_INTERSECT;
				}
			}
		}
		return status;
	}

#endif

}

}

#endif
