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


#include "PsAllocator.h"
#include "GuVecBox.h"
#include "GuVecCapsule.h"
#include "GuVecConvexHull.h"
#include "GuGJKSimplex.h"
#include "GuGJKFallBack.h"
#include "GuGJKSimplexTesselation.h"


namespace physx
{
namespace Gu
{
	struct Simplex
	{
		Ps::aos::Vec3V mA[4];
		Ps::aos::Vec3V mB[4];
		Ps::aos::Vec3V mQ[4];
		PxU32 mSize;
	};

	struct SimplexStack
	{
		static const PxU32 MaxStackSize = 64;
		Simplex mSimplices[MaxStackSize];
		PxU32 stackSize;

		SimplexStack() : stackSize(0)
		{
		}

		void pushSimplex(Ps::aos::Vec3V* Q, Ps::aos::Vec3V* A, Ps::aos::Vec3V* B, PxU32 size)
		{
			PX_ASSERT(size < MaxStackSize);
			for(PxU32 a = 0; a < size; ++a)
			{
				mSimplices[stackSize].mA[a] = A[a];
				mSimplices[stackSize].mB[a] = B[a];
				mSimplices[stackSize].mQ[a] = Q[a];
				mSimplices[stackSize].mSize = size;
			}
			stackSize++;
		}

		PxU32 popSimplex(Ps::aos::Vec3V* Q, Ps::aos::Vec3V* A, Ps::aos::Vec3V* B)
		{
			--stackSize;
			for(PxU32 a = 0; a < mSimplices[stackSize].mSize; ++a)
			{
				Q[a] = mSimplices[stackSize].mQ[a];
				A[a] = mSimplices[stackSize].mA[a];
				B[a] = mSimplices[stackSize].mB[a];
			}
			return mSimplices[stackSize].mSize;
		}

		PxU32 getSize()
		{
			return stackSize;
		}

	};


	Ps::aos::Vec3V doSupportMappingPair(const ConvexV&, const ConvexV&, SupportMap* map1, SupportMap* map2, /*const Ps::aos::PsMatTransformV& aToB, SupportLocal aSupportLocal, SupportLocal bSupportLocal,*/ const Ps::aos::Vec3V v, Ps::aos::Vec3V* Q, 
		Ps::aos::Vec3V* A, Ps::aos::Vec3V* B, PxU32 size, SimplexStack& stack)
	{
		using namespace Ps::aos;
		const FloatV fEps = FLoad(1e-2f);
		const Vec3V eps = Vec3V_From_FloatV(fEps);


		const Vec3V negV = V3Neg(v);
		Vec3V supportA = map1->doSupport(negV);
		Vec3V supportB = map2->doSupport(v);
		Vec3V support = V3Sub(supportA, supportB);


		PxU32 aDegen = FAllGrtr(fEps, V3ExtractMin(V3Abs(negV)));
		PxU32 bDegen = FAllGrtr(fEps, V3ExtractMin(V3Abs(v)));

		//Check for degenerates!!!!

		Vec3V degenA = V3Zero(), degenB = V3Zero();

		if(aDegen)
		{
			const BoolV isDegen = V3IsGrtr(eps, V3Abs(negV));
			const Vec3V degenerateDir = V3Sel(isDegen, v, negV);
			degenA = map1->doSupport(degenerateDir);
		}

		if(bDegen)
		{
			const BoolV isDegen = V3IsGrtr(eps, V3Abs(v));
			const Vec3V degenerateDir = V3Sel(isDegen, negV, v);
			degenB = map2->doSupport(degenerateDir);//bSupportLocal(b, degenerateDir);
			
		}

		if(aDegen && bDegen)
		{
			A[size] = degenA;
			B[size] = degenB;
			Q[size] = V3Sub(degenA, degenB);
			stack.pushSimplex(Q, A, B, size+1);
		}
		if(aDegen)
		{
			A[size] = degenA;
			B[size] = supportB;
			Q[size] = V3Sub(degenA, supportB);
			stack.pushSimplex(Q, A, B, size+1);
		}
		if(bDegen)
		{
			A[size] = supportA;
			B[size] = degenB;
			Q[size] = V3Sub(supportA, degenB);
			stack.pushSimplex(Q, A, B, size+1);
		}

		A[size] = supportA;
		B[size] = supportB;
		Q[size] = support;
		
		//calculate the support point
		return support;
	}


	PxGJKStatus gjkRelativeFallback(const ConvexV& a, const ConvexV& b, SupportMap* map1, SupportMap* map2, const Ps::aos::Vec3VArg initialDir, Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& sqDist)
	{
		using namespace Ps::aos;
		Vec3V Q[4];
		Vec3V A[4];
		Vec3V B[4];

		const Vec3V zeroV = V3Zero();
		const FloatV zero = FZero();
		const BoolV bTrue = BTTTT();
		PxU32 size=0;

		Vec3V v = V3Sel(FIsGrtr(V3Dot(initialDir, initialDir), zero), initialDir, V3UnitX());

		const FloatV minMargin = FMin(a.getMinMargin(), b.getMinMargin());
		const FloatV eps2 = FMul(minMargin, FLoad(0.01f));
		const FloatV epsRel = FLoad(0.000225f);//1.5%
	
		Vec3V closA(zeroV), closB(zeroV);
		FloatV sDist = FMax();
		FloatV minDist = sDist;
		Vec3V closAA = zeroV;   
		Vec3V closBB = zeroV;

		
		BoolV bNotTerminated = bTrue;
		BoolV bCon = bTrue;
	
		do
		{
			minDist = sDist; 
			closAA = closA;
			closBB = closB;

			const Vec3V supportA = map1->doSupport(V3Neg(v));
			const Vec3V supportB = map2->doSupport(v);
			const Vec3V support = V3Sub(supportA, supportB);
			PX_ASSERT(size < 4);
			A[size]=supportA;
			B[size]=supportB;
			Q[size++]=support;

			
			//calculate the support point
			
			const FloatV signDist = V3Dot(v, support);
			const FloatV tmp0 = FSub(sDist, signDist);

	
			if(FAllGrtr(FMul(epsRel, sDist), tmp0))
			{
				const Vec3V n = V3Normalize(V3Sub(closB, closA));
				closestA = closA;
				closestB = closB;
				sqDist = sDist;
				normal = n;
				return GJK_NON_INTERSECT;
			}

			//calculate the closest point between two convex hull
			v = GJKCPairDoSimplexTesselation(Q, A, B, support, supportA, supportB, size, closA, closB);
			sDist = V3Dot(v, v);
			bCon = FIsGrtr(minDist, sDist);

			bNotTerminated = BAnd(FIsGrtr(sDist, eps2), bCon);

		}while(BAllEq(bNotTerminated, bTrue));

		closA = V3Sel(bCon, closA, closAA);
		closB = V3Sel(bCon, closB, closBB);
		closestA = closA;
		closestB = closB;
		normal = V3Normalize(V3Sub(closB, closA));
		sqDist = FSel(bCon, sDist, minDist);

		return PxGJKStatus(BAllEq(bCon, bTrue) == 1 ? GJK_CONTACT : GJK_DEGENERATE);
	}
}


}
