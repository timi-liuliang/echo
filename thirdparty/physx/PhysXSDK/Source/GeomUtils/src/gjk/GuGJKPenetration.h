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

#ifndef GU_GJK_PENETRATION_H
#define GU_GJK_PENETRATION_H


#include "GuConvexSupportTable.h"
#include "GuGJKSimplex.h"
#include "GuVecShrunkConvexHullNoScale.h"
#include "GuVecConvexHullNoScale.h"
#include "GuGJKUtil.h"
#include "PsUtilities.h"

#define	GJK_VALIDATE 0


namespace physx
{
namespace Gu
{

	class ConvexV;


	PX_FORCE_INLINE void assignWarmStartValue(PxU8* __restrict aIndices, PxU8* __restrict bIndices, PxU8& _size, PxI32* __restrict aInd, PxI32* __restrict bInd,  PxU32 size )
	{
		if(aIndices)
		{
			PX_ASSERT(bIndices);
			_size = Ps::to8(size);
			for(PxU32 i=0; i<size; ++i)
			{
				aIndices[i] = Ps::to8(aInd[i]);
				bIndices[i] = Ps::to8(bInd[i]);
			}
		}
	}


#ifndef	__SPU__

	PX_FORCE_INLINE void validateDuplicateVertex(const Ps::aos::Vec3V* Q, const Ps::aos::Vec3VArg support, const PxU32 size)
	{
		using namespace Ps::aos;

		const FloatV eps = FEps();
		const BoolV bFalse = BFFFF();
		const BoolV bTrue = BTTTT();
		//Get rid of the duplicate point
		BoolV match = bFalse;
		for(PxU32 na = 0; na < size; ++na)
		{
			Vec3V dif = V3Sub(Q[na], support);
			match = BOr(match, FIsGrtr(eps, V3Dot(dif, dif)));	
		}

		//we have duplicate code
		if(BAllEq(match, bTrue))
		{
			PX_ASSERT(0);
		}
	}

	//relative space, warm start

	template<class ConvexA, class ConvexB>
	PX_GJK_FORCE_INLINE PxGJKStatus gjkRelativePenetration(const ConvexA& a, const ConvexB& b,  const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg _contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict aIndices, PxU8* __restrict bIndices, PxU8& _size)
	{
		using namespace Ps::aos;
		const FloatV zero = FZero();
		const Vec3V zeroV = V3Zero();
		const BoolV bTrue = BTTTT();
		const BoolV bFalse = BFFFF();
		const FloatV marginA = a.getMargin();
		const FloatV marginB = b.getMargin();
		
		//This might change
		FloatV contactDist = _contactDist;
		
		  
		const FloatV minMargin = FMin(a.getMinMargin(), b.getMinMargin());
		const FloatV _eps2 = FMul(minMargin, FLoad(0.1f));
		//ML: eps2 is the threshold that uses to detemine whether two (shrunk) shapes overlap. We calculate eps2 based on 10% of the minimum margin of two shapes
		const FloatV eps2 = FMul(_eps2, _eps2);
		//ML: epsRel2 is the square of 1.5%. This is used to scale the the sqaure distance of a closet point to origin to detemine whether two shrunk shapes overlap in the margin, but
		//they don't overlap.
		const FloatV epsRel2 = FLoad(GJK_RELATIVE_EPSILON);//FMax(FLoad(0.0001), eps2);
		
		const FloatV sumOriginalMargin = FAdd(marginA, marginB);

		FloatV sDist = FMax();
		FloatV minDist= sDist;

		BoolV bNotTerminated = bTrue;
		BoolV bCon = bTrue;

		Vec3V Q[4];
		Vec3V A[4];
		Vec3V B[4];
		PxI32 aInd[4];
		PxI32 bInd[4];

		PxU32 size = 0;//_size;
		
		Vec3V closest;
		
		//ML: if _size!=0, which means we pass in the previous frame simplex so that we can warm-start the simplex. 
		//In this case, GJK will normally terminate in one iteration
		if(_size != 0)
		{
			Vec3V support = zeroV;
			
			//we should not have duplicate vertexes
			for(PxU32 i=0; i<_size; ++i)
			{
				aInd[i] = aIndices[i];
				bInd[i] = bIndices[i];
				Vec3V supportA = aToB.transform(a.supportPoint(aIndices[i]));
				Vec3V supportB = b.supportPoint(bIndices[i]);
				support = V3Sub(supportA, supportB);

#if	GJK_VALIDATE
				validateDuplicateVertex(Q, support, size);
#endif
				A[size] = supportA;
				B[size] = supportB;
				Q[size++] = support;

			}

			//run simplex solver to determine whether the point is closest enough so that gjk can terminate
			closest = GJKCPairDoSimplex(Q, A, B, aInd, bInd, support, size);

			sDist = V3Dot(closest, closest);
			minDist = sDist;

			bNotTerminated = FIsGrtr(sDist, eps2);
		}
		else
		{
			const Vec3V _initialSearchDir = aToB.p;//V3Sub(a.getCenter(), b.getCenter());
			closest = V3Sel(FIsGrtr(V3Dot(_initialSearchDir, _initialSearchDir), zero), _initialSearchDir, V3UnitX());
		}

		Vec3V prevClosest = closest;
		
		// ML : termination condition
		//(1)two (shrunk)shapes overlap. GJK will terminate based on sq(v) < eps2 and indicate that two shapes are overlapping.
		//(2)two (shrunk + margin)shapes separate. If sq(vw) > sqMargin * sq(v), which means the original objects do not intesect, GJK terminate with GJK_NON_INTERSECT. 
		//(3)two (shrunk) shapes don't overlap. However, they interect within margin distance. if sq(v)- vw < epsRel2*sq(v), this means the shrunk shapes interect in the margin, 
		//   GJK terminate with GJK_CONTACT.
		while(BAllEq(bNotTerminated, bTrue))
		{
			//minDist, tempClosA, tempClosB are used to store the previous iteration's closest points(in A and B space) and the square distance from the closest point
			//to origin in Mincowski space
			minDist = sDist;
			prevClosest = closest;

			const Vec3V supportA=a.supportRelative(V3Neg(closest), aToB, A[size], aInd[size]);
			const Vec3V supportB=b.supportLocal(closest, B[size], bInd[size]);

			//calculate the support point
			const Vec3V support = V3Sub(supportA, supportB);
			Q[size]=support;

			//ML: because we shrink the shapes by plane shifting(box and convexhull), the distance from the "shrunk" vertices to the original vertices may be larger than contact distance. 
			//therefore, we need to take the largest of these 2 values into account so that we don't incorrectly declare shapes to be disjoint. If we don't do this, there is
			//an inherent inconsistency between fallback SAT tests and GJK tests that may result in popping due to SAT discovering deep penetrations that were not detected by
			//GJK operating on a shrunk shape.
			const FloatV maxMarginDif = FMax(a.getMarginDif(), b.getMarginDif());
			contactDist = FMax(contactDist, maxMarginDif);
			const FloatV sumMargin = FAdd(sumOriginalMargin, contactDist);
			const FloatV sqMargin = FMul(sumMargin, sumMargin);
			const FloatV tmp = FMul(sDist, sqMargin);//FMulAdd(sDist, sqMargin, eps3);

			const FloatV vw = V3Dot(closest, support);
			const FloatV sqVW = FMul(vw, vw);

			
			const BoolV bTmp1 = FIsGrtr(vw, zero);
			const BoolV bTmp2 = FIsGrtr(sqVW, tmp);
			BoolV con = BAnd(bTmp1, bTmp2);//this is the non intersect condition


			const FloatV tmp1 = FSub(sDist, vw);
			const FloatV tmp2 = FMul(epsRel2, sDist);
			const BoolV conGrtr = FIsGrtrOrEq(tmp2, tmp1);//this is the margin intersect condition

			const BoolV conOrconGrtr(BOr(con, conGrtr));

			if(BAllEq(conOrconGrtr, bTrue))
			{
				//PX_ASSERT(BAllEq(_conGrtr, conGrtr));
				assignWarmStartValue(aIndices, bIndices, _size, aInd, bInd, size);
			
				//size--; if you want to get the correct size, this line need to be on
				if(BAllEq(con, bFalse)) //must be true otherwise we wouldn't be in here...
				{

					Vec3V closA, closB;
					getClosestPoint(Q, A, B, closest, closA, closB, size);

					const FloatV dist = FSqrt(sDist);
					PX_ASSERT(FAllGrtr(dist, FEps()));
					const Vec3V n = V3ScaleInv(closest, dist);//normalise
					contactA = V3NegScaleSub(n, marginA, closA);
					contactB = V3ScaleAdd(n, marginB, closB);
					penetrationDepth = FSub(dist, sumOriginalMargin);
					normal = n; 
					return GJK_CONTACT;
					
				}
				else
				{ 
					return GJK_NON_INTERSECT;
				}
			}

			size++;
			PX_ASSERT(size <= 4);

			//calculate the closest point between two convex hull
			closest = GJKCPairDoSimplex(Q, A, B, aInd, bInd, support, size);

			sDist = V3Dot(closest, closest);

			bCon = FIsGrtr(minDist, sDist);
			bNotTerminated = BAnd(FIsGrtr(sDist, eps2), bCon);
		}

		if(BAllEq(bCon, bFalse))
		{
			assignWarmStartValue(aIndices, bIndices, _size, aInd, bInd, size-1);

			//Reset back to older closest point 
			closest = prevClosest;
			Vec3V closA, closB;
			getClosestPoint(Q, A, B, closest, closA, closB, size);
			sDist = minDist;
			const FloatV dist = FSqrt(sDist);
			PX_ASSERT(FAllGrtr(dist, FEps()));
			const Vec3V n = V3ScaleInv(closest, dist);//normalise
			contactA = V3NegScaleSub(n, marginA, closA);
			contactB = V3ScaleAdd(n, marginB, closB);
			penetrationDepth = FSub(dist, sumOriginalMargin);
			normal = n;
			return GJK_DEGENERATE; 
		}
		else 
		{	
			//this two shapes are deeply intersected with each other, we need to use EPA algorithm to calculate MTD
			assignWarmStartValue(aIndices, bIndices, _size, aInd, bInd, size);
			return EPA_CONTACT;     
		}
	}


	//ML: if we are using gjk local which means one of the object will be sphere/capsule, in that case, if we define takeCoreShape is true, we just need to return the closest point as the sphere center or a point in the capsule segment. This will increase the stability
	//for the manifold recycling code
	template<class ConvexA, class ConvexB>
	PX_GJK_FORCE_INLINE PxGJKStatus gjkLocalPenetration(const ConvexA& a, const ConvexB& b, const Ps::aos::FloatVArg _contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict aIndices, PxU8* __restrict bIndices, PxU8& _size, bool takeCoreShape)
	{
		using namespace Ps::aos;

	
		FloatV contactDist = _contactDist;
		const FloatV zero = FZero();

		const FloatV marginA = a.getMargin();
		const FloatV marginB = b.getMargin();

		const FloatV minMargin = FMin(a.getMinMargin(), b.getMinMargin());
		const FloatV _eps2 = FMul(minMargin, FLoad(0.1f));
		//ML: eps2 is the threshold that uses to detemine whether two (shrunk) shapes overlap. We calculate eps2 based on 10% of the minimum margin of two shapes
		const FloatV eps2 = FMul(_eps2, _eps2);
		//ML: epsRel2 is the square of 1.5%. This is used to scale the the sqaure distance of a closet point to origin to detemine whether two shrunk shapes overlap in the margin, but
		//they don't overlap.
		const FloatV epsRel2 = FLoad(GJK_RELATIVE_EPSILON); 
		
		const Vec3V zeroV = V3Zero();
		const BoolV bTrue = BTTTT();
		const BoolV bFalse = BFFFF();
		
		const FloatV sumOrignalMargin = FAdd(marginA, marginB);

		FloatV sDist = FMax();
		FloatV minDist= sDist;

		BoolV bNotTerminated = bTrue;
		BoolV bCon = bTrue;
		Vec3V closest;
		

		Vec3V Q[4];
		Vec3V A[4];
		Vec3V B[4];
		PxI32 aInd[4];
		PxI32 bInd[4];
		Vec3V supportA = zeroV, supportB = zeroV, support=zeroV;

		PxU32 size = 0;//_size;
		
		//ML: if _size!=0, which means we pass in the previous frame simplex so that we can warm-start the simplex. 
		//In this case, GJK will normally terminate in one iteration
		if(_size != 0)
		{
			for(PxU32 i=0; i<_size; ++i)
			{
				aInd[i] = aIndices[i];
				bInd[i] = bIndices[i];
				supportA = a.supportPoint(aIndices[i]);
				supportB = b.supportPoint(bIndices[i]);
				support = V3Sub(supportA, supportB);

#if	GJK_VALIDATE
				//ML: this is used to varify whether we will have duplicate vertices in the warm-start value. If this function get triggered,
				//this means something isn't right and we need to investigate
				validateDuplicateVertex(Q, support, size);
#endif
				A[size] = supportA;
				B[size] =  supportB;
				Q[size++] =	support;
			}

			//run simplex solver to determine whether the point is closest enough so that gjk can terminate
			closest = GJKCPairDoSimplex(Q, A, B, aInd, bInd, support, size);

			sDist = V3Dot(closest, closest);
			minDist = sDist;
			
			bNotTerminated = FIsGrtr(sDist, eps2);
		}
		else
		{
			const Vec3V _initialSearchDir = V3Sub(a.getCenter(), b.getCenter());
			closest = V3Sel(FIsGrtr(V3Dot(_initialSearchDir, _initialSearchDir), zero), _initialSearchDir, V3UnitX());
		}

		Vec3V prevClosest = closest;
		
		// ML : termination condition
		//(1)two (shrunk)shapes overlap. GJK will terminate based on sq(v) < eps2 and indicate that two shapes are overlapping.
		//(2)two (shrunk + margin)shapes separate. If sq(vw) > sqMargin * sq(v), which means the original objects do not intesect, GJK terminate with GJK_NON_INTERSECT. 
		//(3)two (shrunk) shapes don't overlap. However, they interect within margin distance. if sq(v)- vw < epsRel2*sq(v), this means the shrunk shapes interect in the margin, 
		//   GJK terminate with GJK_CONTACT.
		while(BAllEq(bNotTerminated, bTrue))
		{
			//minDist, tempClosA, tempClosB are used to store the previous iteration's closest points(in A and B space) and the square distance from the closest point
			//to origin in Mincowski space
			minDist = sDist;
			prevClosest = closest;

			supportA=a.supportLocal(V3Neg(closest), A[size], aInd[size]);
			supportB=b.supportLocal(closest, B[size], bInd[size]);
			
		
			//calculate the support point
			support = V3Sub(supportA, supportB);
			Q[size]=support;

			//ML: because we shrink the shapes by plane shifting(box and convexhull), the distance from the "shrunk" vertices to the original vertices may be larger than contact distance. 
			//therefore, we need to take the largest of these 2 values into account so that we don't incorrectly declare shapes to be disjoint. If we don't do this, there is
			//an inherent inconsistency between fallback SAT tests and GJK tests that may result in popping due to SAT discovering deep penetrations that were not detected by
			//GJK operating on a shrunk shape.
			const FloatV maxMarginDif = FMax(a.getMarginDif(), b.getMarginDif());
			contactDist = FMax(contactDist, maxMarginDif);
			//contactDist = FSel(FIsGrtr(contactDist, maxMarginDif), contactDist, maxMarginDif); 
			const FloatV sumMargin = FAdd(sumOrignalMargin, contactDist);
			const FloatV sqMargin = FMul(sumMargin, sumMargin);
			const FloatV tmp = FMul(sDist, sqMargin);//FMulAdd(sDist, sqMargin, eps3);

			const FloatV vw = V3Dot(closest, support);
			const FloatV sqVW = FMul(vw, vw);

			
			const BoolV bTmp1 = FIsGrtr(vw, zero);
			const BoolV bTmp2 = FIsGrtr(sqVW, tmp);
			BoolV con = BAnd(bTmp1, bTmp2);//this is the non intersect condition


			const FloatV tmp1 = FSub(sDist, vw);
			const FloatV tmp2 = FMul(epsRel2, sDist);
			const BoolV conGrtr = FIsGrtrOrEq(tmp2, tmp1);//this is the margin intersect condition

			const BoolV conOrconGrtr(BOr(con, conGrtr));

			if(BAllEq(conOrconGrtr, bTrue))
			{
				//store warm start indices
				assignWarmStartValue(aIndices, bIndices, _size, aInd, bInd, size-1);
				//size--; if you want to get the correct size, this line need to be on
				if(BAllEq(con, bFalse)) //must be true otherwise we wouldn't be in here...
				{
					const FloatV dist = FSqrt(sDist);
					PX_ASSERT(FAllGrtr(dist, FEps()));
					const Vec3V n = V3ScaleInv(closest, dist);//normalise
					normal = n; 
					Vec3V closA, closB;
					getClosestPoint(Q, A, B, closest, closA, closB, size);
					if(takeCoreShape)
					{
						const BoolV aQuadratic = a.isMarginEqRadius();
						const BoolV bQuadratic = b.isMarginEqRadius();
						const FloatV shrunkFactorA = FSel(aQuadratic, zero, marginA);
						const FloatV shrunkFactorB = FSel(bQuadratic, zero, marginB);
						const FloatV sumShrunkFactor = FAdd(shrunkFactorA, shrunkFactorB);
						contactA = V3NegScaleSub(n, shrunkFactorA, closA);
						contactB = V3ScaleAdd(n, shrunkFactorB, closB);
						penetrationDepth = FSub(dist, sumShrunkFactor);
					
					}
					else
					{
						contactA = V3NegScaleSub(n, marginA, closA);
						contactB = V3ScaleAdd(n, marginB, closB);
						penetrationDepth = FSub(dist, sumOrignalMargin);
					}
					
					return GJK_CONTACT;
					
				}
				else
				{
					return GJK_NON_INTERSECT;
				}
			}

			size++;
			PX_ASSERT(size <= 4);

			//calculate the closest point between two convex hull
			closest = GJKCPairDoSimplex(Q, A, B, aInd, bInd, support, size);

			sDist = V3Dot(closest, closest);

			bCon = FIsGrtr(minDist, sDist);
			bNotTerminated = BAnd(FIsGrtr(sDist, eps2), bCon);
		}

		if(BAllEq(bCon, bFalse))
		{
			sDist = minDist;
			assignWarmStartValue(aIndices, bIndices, _size, aInd, bInd, size-1);
			const FloatV sumExpandedMargin = FAdd(sumOrignalMargin, contactDist);
			const FloatV sqExpandedMargin = FMul(sumExpandedMargin, sumExpandedMargin);
			//Reset back to older closest point
			closest = prevClosest;//V3Sub(closA, closB);
			Vec3V closA, closB;
			getClosestPoint(Q, A, B, closest, closA, closB, size);

			sDist = minDist;

			const FloatV dist = FSqrt(sDist);
			PX_ASSERT(FAllGrtr(dist, FEps()));
			const Vec3V n = V3ScaleInv(closest, dist);//normalise

			if(takeCoreShape)
			{
				const BoolV aQuadratic = a.isMarginEqRadius();
				const BoolV bQuadratic = b.isMarginEqRadius();
				const FloatV shrunkFactorA = FSel(aQuadratic, zero, marginA);
				const FloatV shrunkFactorB = FSel(bQuadratic, zero, marginB);
				const FloatV sumShrunkFactor = FAdd(shrunkFactorA, shrunkFactorB);
				contactA = V3NegScaleSub(n, shrunkFactorA, closA);
				contactB = V3ScaleAdd(n, shrunkFactorB, closB);
				penetrationDepth = FSub(dist, sumShrunkFactor);
			}
			else
			{
				contactA = V3NegScaleSub(n, marginA, closA);
				contactB = V3ScaleAdd(n, marginB, closB);
				penetrationDepth = FSub(dist, sumOrignalMargin);
			}
			normal = n;
			if(FAllGrtrOrEq(sqExpandedMargin, sDist))
			{
				
				return GJK_CONTACT;  
			}
			return GJK_DEGENERATE;  

		}
		else 
		{
			//this two shapes are deeply intersected with each other, we need to use EPA algorithm to calculate MTD
			assignWarmStartValue(aIndices, bIndices, _size, aInd, bInd, size);
			return EPA_CONTACT;
			
		}
	}
 

#else


	PxGJKStatus gjkPenetration(const ConvexV& a, const ConvexV& b, GJKSupportMapPair* pair, const Ps::aos::Vec3VArg initialDir,  const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices, PxU8& _size, const bool useDegeneratedCase, const bool takeCoreShape);
	
	template<typename ConvexA, typename ConvexB>
	PxGJKStatus gjkRelativePenetration(const ConvexA& a, const ConvexB& b, const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth)
	{
		GJKSupportMapPairRelativeImpl<ConvexA, ConvexB> supportMap(a, b, aToB);
		return gjkPenetration(a, b,  &supportMap, aToB.p, contactDist, contactA, contactB, normal, penetrationDepth, NULL, NULL, 0, false, false);
	}


	template<class ConvexA, class ConvexB>
	PxGJKStatus gjkRelativePenetration(const ConvexA& a, const ConvexB& b, const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices, PxU8& _size)
	{
		GJKSupportMapPairRelativeImpl<ConvexA, ConvexB> supportMap(a, b, aToB);
		return gjkPenetration(a, b, &supportMap, aToB.p, contactDist, contactA, contactB, normal, penetrationDepth, aIndices, bIndices, _size, false, false);
	}


	template<class ConvexA, class ConvexB>
	PxGJKStatus gjkLocalPenetration(const ConvexA& a, const ConvexB& b, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penetrationDepth,
		PxU8* __restrict  aIndices, PxU8* __restrict bIndices, PxU8& _size, const bool takeCoreShape)
	{
		using namespace Ps::aos;

		GJKSupportMapPairLocalImpl<ConvexA, ConvexB> supportMap(a, b);
		const Vec3V initialDir = V3Sub(a.getCenter(), b.getCenter());
		return gjkPenetration(a, b, &supportMap, initialDir, contactDist, contactA, contactB, normal, penetrationDepth, aIndices, bIndices, _size, true, takeCoreShape);
	}
	

#endif

}//Gu

}//physx

#endif
