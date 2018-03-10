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

#ifndef GU_GJKRAYCAST_H
#define GU_GJKRAYCAST_H

#include "GuGJKSimplex.h"
#include "GuConvexSupportTable.h"
#include "GuGJKPenetration.h"
#include "GuEPA.h"


namespace physx
{


namespace Gu
{

#ifndef	__SPU__


	/*
		ConvexA is in the local space of ConvexB
		lambda			:	the time of impact(TOI)
		initialLambda	:	the start time of impact value (disable)
		s				:	the sweep ray origin
		r				:	the normalized sweep ray direction scaled by the sweep distance. r should be in ConvexB's space
		normal			:	the contact normal
		inflation		:	the amount by which we inflate the swept shape. If the inflated shapes aren't initially-touching, 
							the TOI will return the time at which both shapes are at a distance equal to inflation separated. If inflation is 0
							the TOI will return the time at which both shapes are touching.
	
	*/
	template<class ConvexA, class ConvexB>
	bool _gjkLocalRayCast(ConvexA& a, ConvexB& b, const Ps::aos::FloatVArg initialLambda, const Ps::aos::Vec3VArg s, const Ps::aos::Vec3VArg r, Ps::aos::FloatV& lambda, Ps::aos::Vec3V& normal, Ps::aos::Vec3V& closestA, const PxReal _inflation)
	{
		PX_UNUSED(initialLambda);

		using namespace Ps::aos;

		const FloatV inflation = FLoad(_inflation);
		const Vec3V zeroV = V3Zero();
		const FloatV zero = FZero();
		const FloatV one = FOne();
		const BoolV bTrue = BTTTT();

		const FloatV maxDist = FLoad(PX_MAX_REAL);
	
		FloatV _lambda = zero;//initialLambda;
		Vec3V x = V3ScaleAdd(r, _lambda, s);
		PxU32 size=1;
		
		const Vec3V dir = V3Sub(a.getCenter(), b.getCenter());
		const Vec3V _initialSearchDir = V3Sel(FIsGrtr(V3Dot(dir, dir), FEps()), dir, V3UnitX());
		const Vec3V initialSearchDir = V3Normalize(_initialSearchDir);

		const Vec3V initialSupportA(a.supportSweepLocal(V3Neg(initialSearchDir)));
		const Vec3V initialSupportB( b.supportSweepLocal(initialSearchDir));
		 
		Vec3V Q[4] = {V3Sub(initialSupportA, initialSupportB), zeroV, zeroV, zeroV}; //simplex set
		Vec3V A[4] = {initialSupportA, zeroV, zeroV, zeroV}; //ConvexHull a simplex set
		Vec3V B[4] = {initialSupportB, zeroV, zeroV, zeroV}; //ConvexHull b simplex set
		 

		Vec3V closest = Q[0];
		Vec3V supportA = initialSupportA;
		Vec3V supportB = initialSupportB;
		Vec3V support = Q[0];
	

		const FloatV minMargin = FMin(a.getSweepMargin(), b.getSweepMargin());
		const FloatV eps1 = FMul(minMargin, FLoad(0.1f));
		const FloatV inflationPlusEps(FAdd(eps1, inflation));
		const FloatV eps2 = FMul(eps1, eps1);

		const FloatV inflation2 = FMul(inflationPlusEps, inflationPlusEps);

		//Vec3V closA(initialSupportA), closB(initialSupportB);
		FloatV sDist = V3Dot(closest, closest);
		FloatV minDist = sDist;
		
		BoolV bNotTerminated = FIsGrtr(sDist, eps2);
		BoolV bCon = bTrue;

		Vec3V nor = closest;
		Vec3V prevClosest = closest;
		
		while(BAllEq(bNotTerminated, bTrue))
		{
			
			minDist = sDist;
			prevClosest = closest;

			const Vec3V vNorm = V3Neg(V3Normalize(closest));
			//const Vec3V nvNorm = V3Neg(vNorm);

			supportA=a.supportSweepLocal(vNorm);
			supportB=V3Add(x, b.supportSweepLocal(V3Neg(vNorm)));
		
			//calculate the support point
			support = V3Sub(supportA, supportB);
			const Vec3V w = V3Neg(support);
			const FloatV vw = FSub(V3Dot(vNorm, w), inflationPlusEps);
			const FloatV vr = V3Dot(vNorm, r);
			if(FAllGrtr(vw, zero))
			{
	
				if(FAllGrtrOrEq(vr, zero))
				{
					return false;
				}
				else
				{
					const FloatV _oldLambda = _lambda;
					_lambda = FSub(_lambda, FDiv(vw, vr));
					if(FAllGrtr(_lambda, _oldLambda))
					{
						if(FAllGrtr(_lambda, one))
						{
							return false;
						}
						const Vec3V bPreCenter = x;
						x = V3ScaleAdd(r, _lambda, s);
						
						const Vec3V offSet =V3Sub(x, bPreCenter);
						const Vec3V b0 = V3Add(B[0], offSet);
						const Vec3V b1 = V3Add(B[1], offSet);
						const Vec3V b2 = V3Add(B[2], offSet);
					
						B[0] = b0;
						B[1] = b1;
						B[2] = b2;

						Q[0]=V3Sub(A[0], b0);
						Q[1]=V3Sub(A[1], b1);
						Q[2]=V3Sub(A[2], b2);

						supportB = V3Add(x, b.supportSweepLocal(V3Neg(vNorm)));
						support = V3Sub(supportA, supportB);
						minDist = maxDist;
						nor = closest;
						//size=0;
					}
				}
			}

			PX_ASSERT(size < 4);
			A[size]=supportA;
			B[size]=supportB;
			Q[size++]=support;
	
			//calculate the closest point between two convex hull
			closest = GJKCPairDoSimplex(Q, A, B, support, size);
			sDist = V3Dot(closest, closest);
			
			bCon = FIsGrtr(minDist, sDist);
			bNotTerminated = BAnd(FIsGrtr(sDist, inflation2), bCon);
		}

		const BoolV aQuadratic = a.isMarginEqRadius();
		//ML:if the Minkowski sum of two objects are too close to the original(eps2 > sDist), we can't take v because we will lose lots of precision. Therefore, we will take
		//previous configuration's normal which should give us a reasonable approximation. This effectively means that, when we do a sweep with inflation, we always keep v because
		//the shapes converge separated. If we do a sweep without inflation, we will usually use the previous configuration's normal.
		nor = V3Sel(BAnd(FIsGrtr(sDist, eps2), bCon), closest, nor);
		nor =  V3Normalize(nor);
		normal = nor;
		lambda = _lambda;
		const Vec3V closestP = V3Sel(bCon, closest, prevClosest);
		Vec3V closA = zeroV, closB = zeroV;
		getClosestPoint(Q, A, B, closestP, closA, closB, size);
		closestA = V3Sel(aQuadratic, V3NegScaleSub(nor, a.getMargin(), closA), closA);  
		
		return true;
	}

	
	/*
	
		lambda			:	the time of impact(TOI)
		initialLambda	:	the start time of impact value (disable)
		s				:	the sweep ray origin in ConvexB's space
		r				:	the normalized sweep ray direction scaled by the sweep distance. r should be in ConvexB's space
		normal			:	the contact normal in ConvexB's space
		closestA		:	the tounching contact in ConvexB's space
		inflation		:	the amount by which we inflate the swept shape. If the inflated shapes aren't initially-touching, 
							the TOI will return the time at which both shapes are at a distance equal to inflation separated. If inflation is 0
							the TOI will return the time at which both shapes are touching.
	*/
	template<class ConvexA, class ConvexB>
	bool _gjkRelativeRayCast(ConvexA& a, ConvexB& b, const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg initialLambda, const Ps::aos::Vec3VArg s, const Ps::aos::Vec3VArg r, Ps::aos::FloatV& lambda, Ps::aos::Vec3V& normal, Ps::aos::Vec3V& closestA, const PxReal _inflation/*, const bool initialOverlap*/)
	{
		PX_UNUSED(initialLambda);

		using namespace Ps::aos;

		const FloatV inflation = FLoad(_inflation);
		const Vec3V zeroV = V3Zero();
		const FloatV zero = FZero();
		const FloatV eps = FEps();
		const FloatV one = FOne();
		const BoolV bTrue = BTTTT();

		const FloatV maxDist = FLoad(PX_MAX_REAL);
	
		FloatV _lambda = zero;//initialLambda;
		Vec3V x = V3ScaleAdd(r, _lambda, s);
		PxU32 size=1;
	
		const Vec3V _initialSearchDir = V3Sel(FIsGrtr(V3Dot(aToB.p, aToB.p), eps), aToB.p, V3UnitX());
		const Vec3V initialSearchDir = V3Normalize(_initialSearchDir);
		

		const Vec3V initialSupportA(a.supportSweepRelative(V3Neg(initialSearchDir), aToB));
		const Vec3V initialSupportB( b.supportSweepLocal(initialSearchDir));
		 
		Vec3V Q[4] = {V3Sub(initialSupportA, initialSupportB), zeroV, zeroV, zeroV}; //simplex set
		Vec3V A[4] = {initialSupportA, zeroV, zeroV, zeroV}; //ConvexHull a simplex set
		Vec3V B[4] = {initialSupportB, zeroV, zeroV, zeroV}; //ConvexHull b simplex set
		 

		Vec3V closest = Q[0];
		Vec3V supportA = initialSupportA;
		Vec3V supportB = initialSupportB;
		Vec3V support = Q[0];

		const FloatV minMargin = FMin(a.getSweepMargin(), b.getSweepMargin());
		const FloatV eps1 = FMul(minMargin, FLoad(0.1f));
		const FloatV inflationPlusEps(FAdd(eps1, inflation));
		const FloatV eps2 = FMul(eps1, eps1);

		const FloatV inflation2 = FMul(inflationPlusEps, inflationPlusEps);

		FloatV sDist = V3Dot(closest, closest);
		FloatV minDist = sDist;
		
		
		BoolV bNotTerminated = FIsGrtr(sDist, eps2);
		BoolV bCon = bTrue;

		Vec3V prevClosest = closest;

		Vec3V nor = closest;
		
		while(BAllEq(bNotTerminated, bTrue))
		{
			
			minDist = sDist;
			prevClosest = closest;

			const Vec3V vNorm = V3Neg(V3Normalize(closest));

			supportA=a.supportSweepRelative(vNorm, aToB);
			supportB=V3Add(x, b.supportSweepLocal(V3Neg(vNorm)));
		
			//calculate the support point
			support = V3Sub(supportA, supportB);
			const Vec3V w = V3Neg(support);
			const FloatV vw = FSub(V3Dot(vNorm, w), inflationPlusEps);
			const FloatV vr = V3Dot(vNorm, r);
			if(FAllGrtr(vw, zero))
			{
	
				if(FAllGrtrOrEq(vr, zero))
				{
					return false;
				}
				else
				{
					const FloatV _oldLambda = _lambda;
					_lambda = FSub(_lambda, FDiv(vw, vr));
					if(FAllGrtr(_lambda, _oldLambda))
					{
						if(FAllGrtr(_lambda, one))
						{
							return false;
						}
						const Vec3V bPreCenter = x;
						x = V3ScaleAdd(r, _lambda, s);
			
						const Vec3V offSet =V3Sub(x, bPreCenter);
						const Vec3V b0 = V3Add(B[0], offSet);
						const Vec3V b1 = V3Add(B[1], offSet);
						const Vec3V b2 = V3Add(B[2], offSet);
					
						B[0] = b0;
						B[1] = b1;
						B[2] = b2;

						Q[0]=V3Sub(A[0], b0);
						Q[1]=V3Sub(A[1], b1);
						Q[2]=V3Sub(A[2], b2);

						supportB = V3Add(x, b.supportSweepLocal(V3Neg(vNorm)));
				
						support = V3Sub(supportA, supportB);
						minDist = maxDist;
						nor = closest;
						//size=0;
					}
				}
			}

			PX_ASSERT(size < 4);
			A[size]=supportA;
			B[size]=supportB;
			Q[size++]=support;
	
			//calculate the closest point between two convex hull
			closest = GJKCPairDoSimplex(Q, A, B, support, size);
			sDist = V3Dot(closest, closest);

			bCon = FIsGrtr(minDist, sDist);
			bNotTerminated = BAnd(FIsGrtr(sDist, inflation2), bCon);
		}

		const BoolV aQuadratic = a.isMarginEqRadius();
		//ML:if the Minkowski sum of two objects are too close to the original(eps2 > sDist), we can't take v because we will lose lots of precision. Therefore, we will take
		//previous configuration's normal which should give us a reasonable approximation. This effectively means that, when we do a sweep with inflation, we always keep v because
		//the shapes converge separated. If we do a sweep without inflation, we will usually use the previous configuration's normal.
		nor = V3Sel(BAnd(FIsGrtr(sDist, eps2), bCon), closest, nor);
		nor =  V3Normalize(nor);
		normal = nor;
		lambda = _lambda;
		const Vec3V closestP = V3Sel(bCon, closest, prevClosest);
		Vec3V closA = zeroV, closB = zeroV;
		getClosestPoint(Q, A, B, closestP, closA, closB, size);
		closestA = V3Sel(aQuadratic, V3NegScaleSub(nor, a.getMargin(), closA), closA);  
		
		return true;
	}


	
	/*
		ConvexA is in the local space of ConvexB
		lambda			:	the time of impact(TOI)
		initialLambda	:	the start time of impact value (disable)
		s				:	the sweep ray origin in ConvexB's space
		r				:	the normalized sweep ray direction scaled by the sweep distance. r should be in ConvexB's space
		normal			:	the contact normal in ConvexB's space
		closestA		:	the tounching contact in ConvexB's space
		inflation		:	the amount by which we inflate the swept shape. If the inflated shapes aren't initially-touching, 
							the TOI will return the time at which both shapes are at a distance equal to inflation separated. If inflation is 0
							the TOI will return the time at which both shapes are touching.
	
	*/
	template<class ConvexA, class ConvexB, class ShrunkConvexA, class ShrunkConvexB>
	bool gjkLocalRayCast(ConvexA& a, ConvexB& b, const Ps::aos::FloatVArg initialLambda, const Ps::aos::Vec3VArg s, const Ps::aos::Vec3VArg r, Ps::aos::FloatV& lambda, Ps::aos::Vec3V& normal, Ps::aos::Vec3V& closestA, const PxReal _inflation, const bool initialOverlap)
	{
		using namespace Ps::aos;
		Vec3V closA;
		Vec3V norm; 
		FloatV _lambda;
		if(_gjkLocalRayCast(a, b, initialLambda, s, r, _lambda, norm, closA, _inflation))
		{
			const FloatV zero = FZero();
			lambda = _lambda;
		
			if(FAllEq(_lambda, zero) && initialOverlap)
			{
				//time of impact is zero, the sweep shape is intesect, use epa to get the normal and contact point
				const FloatV contactDist = getSweepContactEps(a.getMargin(), b.getMargin());

				Vec3V closAA;
				Vec3V closBB;
				FloatV sDist = zero;

				PxU8 aIndices[4];
				PxU8 bIndices[4];
				PxU8 size=0;
				PxGJKStatus status = gjkLocalPenetration((ShrunkConvexA&)a, (ShrunkConvexB&)b, contactDist, closAA, closBB, norm, sDist, aIndices, bIndices, size, false);
				if(status == GJK_CONTACT)
				{
					closA = closAA;
				}
				else if(status == EPA_CONTACT)
				{
					EPASupportMapPairLocalImpl<ConvexA, ConvexB> supportMap(a, b);
					status = epaPenetration(a, b, &supportMap, aIndices, bIndices, size, closAA, closBB, norm, sDist, false);
					if(status == EPA_CONTACT || status == EPA_DEGENERATE)
					{
						closA = closAA;
					}
					else
					{
						//ML: if EPA fail, we will use the ray direction as the normal and set pentration to be zero
						norm = V3Normalize(V3Neg(r));
						closA = closAA;
						sDist = zero;
					}
				}
				else
				{
					//ML:: this will be gjk degenerate case, we will take the normal and sDist from the gjkLocalPenetration
					closA = closAA;
				}
				lambda = FMin(zero, sDist);
			}
			closestA = closA;
			normal = norm;

			return true;
		}
		return false;
	}


	/*
		ConvexA is in the local space of ConvexB
		lambda			:	the time of impact(TOI)
		initialLambda	:	the start time of impact value (disable)
		s				:	the sweep ray origin in ConvexB's space
		r				:	the normalized sweep ray direction scaled by the sweep distance. r should be in ConvexB's space
		normal			:	the contact normal in ConvexB's space
		closestA		:	the tounching contact in ConvexB's space
		inflation		:	the amount by which we inflate the swept shape. If the inflated shapes aren't initially-touching, 
							the TOI will return the time at which both shapes are at a distance equal to inflation separated. If inflation is 0
							the TOI will return the time at which both shapes are touching.
	
	*/
	template<class ConvexA, class ConvexB, class ShrunkConvexA, class ShrunkConvexB>
	bool gjkRelativeRayCast(ConvexA& a, ConvexB& b, const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg initialLambda, const Ps::aos::Vec3VArg s, const Ps::aos::Vec3VArg r, Ps::aos::FloatV& lambda, Ps::aos::Vec3V& normal, Ps::aos::Vec3V& closestA, const PxReal _inflation, const bool initialOverlap)
	{
		using namespace Ps::aos;
		Vec3V closA;
		Vec3V norm; 
		FloatV _lambda;
		
		if(_gjkRelativeRayCast(a, b, aToB, initialLambda, s, r, _lambda, norm, closA, _inflation))
		{
			const FloatV zero = FZero();
			lambda = _lambda;
			if(FAllEq(_lambda, zero) && initialOverlap)
			{

				//time of impact is zero, the sweep shape is intesect, use epa to get the normal and contact point
				const FloatV contactDist = getSweepContactEps(a.getMargin(), b.getMargin());

				Vec3V closAA;
				Vec3V closBB;
				FloatV sDist;
				PxU8 aIndices[4];
				PxU8 bIndices[4];
				PxU8 size=0;
				PxGJKStatus status = gjkRelativePenetration((ShrunkConvexA&)a, (ShrunkConvexB&)b, aToB, contactDist, closAA, closBB, norm, sDist, aIndices, bIndices, size);
				//norm = V3Neg(norm);
				if(status == GJK_CONTACT)
				{
					closA = closAA;//V3Sel(aQuadratic, V3Add(closAA, V3Scale(norm, a.getMargin())),closAA);
				}
				else if(status == EPA_CONTACT)
				{
					//const BoolV bQuadratic = b.isMarginEqRadius();
					EPASupportMapPairRelativeImpl<ConvexA, ConvexB> supportMap(a, b, aToB);
					status = epaPenetration(a, b, &supportMap, aIndices, bIndices, size, closAA, closBB, norm, sDist);
					if(status == EPA_CONTACT || status == EPA_DEGENERATE)
					{
						closA = closAA;
					}
					else
					{
						//ML: if EPA fail, we will use the ray direction as the normal and set pentration to be zero
						norm = V3Normalize(V3Neg(r));
						closA = closAA;
						sDist = zero;//FNeg(V3Length(V3Sub(closBB, closAA)));
					}
				}
				else
				{
					//ML:: this will be gjk degenerate case, we will take the normal and sDist from the gjkRelativePenetration
					closA = closAA;
				}
				lambda = FMin(zero, sDist);
			}
			closestA = closA;
			normal = norm;
			
			

			return true;
		}
		return false;
	}

#else

	bool gjkRayCast(ConvexV& a, ConvexV& b, GJKSupportMapPair* gjkPair, EPASupportMapPair* epaPair, const Ps::aos::Vec3VArg initialDir, const Ps::aos::FloatVArg initialLambda, const Ps::aos::Vec3VArg s, const Ps::aos::Vec3VArg r, Ps::aos::FloatV& lambda, Ps::aos::Vec3V& normal, Ps::aos::Vec3V& closestA, const PxReal inflation, const bool initialOverlap);

	template<class ConvexA, class ConvexB, class ShrunkConvexA, class ShrunkConvexB>
	bool gjkRelativeRayCast(ConvexA& a, ConvexB& b, const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg initialLambda, const Ps::aos::Vec3VArg s, const Ps::aos::Vec3VArg r, Ps::aos::FloatV& lambda, Ps::aos::Vec3V& normal, Ps::aos::Vec3V& closestA, const PxReal inflation, const bool initialOverlap)
	{
		GJKSupportMapPairRelativeImpl<ConvexA, ConvexB> gjkPair(a, b, aToB);
		EPASupportMapPairRelativeImpl<ConvexA, ConvexB> epaPair(a, b, aToB);
		return gjkRayCast(a, b, &gjkPair, &epaPair, aToB.p, initialLambda, s, r, lambda, normal, closestA, inflation, initialOverlap);
	}

	template<class ConvexA, class ConvexB, class ShrunkConvexA, class ShrunkConvexB>
	bool gjkLocalRayCast(ConvexA& a, ConvexB& b, const Ps::aos::FloatVArg initialLambda, const Ps::aos::Vec3VArg s, const Ps::aos::Vec3VArg r, Ps::aos::FloatV& lambda, Ps::aos::Vec3V& normal, Ps::aos::Vec3V& closestA, const PxReal inflation, const bool initialOverlap)
	{
		using namespace Ps::aos;
		GJKSupportMapPairLocalImpl<ConvexA, ConvexB> gjkPair(a, b);
		EPASupportMapPairLocalImpl<ConvexA, ConvexB> epaPair(a, b);
		const Vec3V initialDir = V3Sub(a.getCenter(), b.getCenter());
		return gjkRayCast(a, b, &gjkPair, &epaPair, initialDir, initialLambda, s, r, lambda, normal, closestA, inflation, initialOverlap);
	}


	
#endif

}
}

#endif
