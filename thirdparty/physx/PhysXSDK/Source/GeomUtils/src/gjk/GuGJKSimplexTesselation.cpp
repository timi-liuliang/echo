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

#include "GuGJKSimplexTesselation.h"

namespace physx
{
namespace Gu
{
	PX_NOALIAS PX_FORCE_INLINE Ps::aos::Vec3V closestPtPointSegment(const Ps::aos::Vec3VArg a, const Ps::aos::Vec3VArg b)
	{
		using namespace Ps::aos;
	
		const FloatV zero = FZero();
		const FloatV one = FOne();

		//Test degenerated case
		const Vec3V ab = V3Sub(b, a);
		const FloatV denom = V3Dot(ab, ab);
		const Vec3V ap = V3Neg(a);//V3Sub(origin, a);
		const FloatV nom = V3Dot(ap, ab);
		const BoolV con = FIsEq(denom, zero);
		const FloatV tValue = FClamp(FDiv(nom, denom), zero, one);
		const FloatV t = FSel(con, zero, tValue);

		return V3Sel(con, a, V3ScaleAdd(ab, t, a));
	}

	PX_NOALIAS PX_FORCE_INLINE Ps::aos::Vec3V closestPtPointSegment(const Ps::aos::Vec3VArg Q0, const Ps::aos::Vec3VArg Q1, const Ps::aos::Vec3VArg A0, const Ps::aos::Vec3VArg A1,
		const Ps::aos::Vec3VArg B0, const Ps::aos::Vec3VArg B1, PxU32& size, Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB)
	{
		using namespace Ps::aos;
		const Vec3V a = Q0;
		const Vec3V b = Q1;

		const BoolV bTrue = BTTTT();
		const FloatV zero = FZero();
		const FloatV one = FOne();

		//Test degenerated case
		const Vec3V ab = V3Sub(b, a);
		const FloatV denom = V3Dot(ab, ab);
		const Vec3V ap = V3Neg(a);//V3Sub(origin, a);
		const FloatV nom = V3Dot(ap, ab);
		const BoolV con = FIsEq(denom, zero);
		
		if(BAllEq(con, bTrue))
		{
			size = 1;
			closestA = A0;
			closestB = B0;
			return Q0;
		}

		const Vec3V v = V3Sub(A1, A0);
		const Vec3V w = V3Sub(B1, B0);
		const FloatV tValue = FClamp(FDiv(nom, denom), zero, one);
		const FloatV t = FSel(con, zero, tValue);

		const Vec3V tempClosestA = V3ScaleAdd(v, t, A0);
		const Vec3V tempClosestB = V3ScaleAdd(w, t, B0);
		closestA = tempClosestA;
		closestB = tempClosestB;
		return V3Sub(tempClosestA, tempClosestB);
	}

	PX_NOALIAS Ps::aos::Vec3V closestPtPointSegmentTesselation(const Ps::aos::Vec3VArg Q0, const Ps::aos::Vec3VArg Q1, const Ps::aos::Vec3VArg A0, const Ps::aos::Vec3VArg A1,
		const Ps::aos::Vec3VArg B0, const Ps::aos::Vec3VArg B1, PxU32& size, Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB)
	{
		using namespace Ps::aos;

		const FloatV half = FHalf();

		const FloatV targetSegmentLengthSq = FLoad(10000.f);//100 unit

		Vec3V q0 = Q0;
		Vec3V q1 = Q1;
		Vec3V a0 = A0;
		Vec3V a1 = A1;
		Vec3V b0 = B0;
		Vec3V b1 = B1;

		do
		{
			const Vec3V midPoint = V3Scale(V3Add(q0, q1), half);
			const Vec3V midA = V3Scale(V3Add(a0, a1), half);
			const Vec3V midB = V3Scale(V3Add(b0, b1), half);

			const Vec3V v = V3Sub(midPoint, q0);
			const FloatV sqV = V3Dot(v, v);
			if(FAllGrtr(targetSegmentLengthSq, sqV))
				break;
			//split the segment into half
			const Vec3V tClos0 = closestPtPointSegment(q0, midPoint);
			const FloatV sqDist0 = V3Dot(tClos0, tClos0);

			const Vec3V tClos1 = closestPtPointSegment(q1, midPoint);
			const FloatV sqDist1 = V3Dot(tClos1, tClos1);
			//const BoolV con = FIsGrtr(sqDist0, sqDist1);
			if(FAllGrtr(sqDist0, sqDist1))
			{
				//segment [m, q1]
				q0 = midPoint;
				a0 = midA;
				b0 = midB;
			}
			else
			{
				//segment [q0, m]
				q1 = midPoint;
				a1 = midA;
				b1 = midB;
			}

		}while(1);

		return closestPtPointSegment(q0, q1, a0, a1, b0, b1, size, closestA, closestB);
	}

	PX_NOALIAS Ps::aos::Vec3V closestPtPointTriangleTesselation(Ps::aos::Vec3V* PX_RESTRICT Q, Ps::aos::Vec3V* PX_RESTRICT A, Ps::aos::Vec3V* PX_RESTRICT B, PxU32* PX_RESTRICT indices, PxU32& size, Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB)
	{
		using namespace Ps::aos;

		size = 3;
		const FloatV zero = FZero();
		const FloatV eps = FEps();
		const FloatV half = FHalf();
		const BoolV bTrue = BTTTT();
		const FloatV four = FLoad(4.f);
		const FloatV sixty = FLoad(100.f);

		const PxU32 ind0 = indices[0];
		const PxU32 ind1 = indices[1];
		const PxU32 ind2 = indices[2];

		const Vec3V a = Q[ind0];
		const Vec3V b = Q[ind1];
		const Vec3V c = Q[ind2];

		Vec3V ab_ = V3Sub(b, a);
		Vec3V ac_ = V3Sub(c, a);
		Vec3V bc_ = V3Sub(b, c);

		const FloatV dac_ = V3Dot(ac_, ac_);
		const FloatV dbc_ = V3Dot(bc_, bc_);
		if(FAllGrtrOrEq(eps, FMin(dac_, dbc_)))
		{
			//degenerate
			size = 2;
			return closestPtPointSegment(Q[ind0], Q[ind1], A[ind0], A[ind1], B[ind0], B[ind1], size, closestA, closestB);
		}
		
		Vec3V ap = V3Neg(a);
		Vec3V bp = V3Neg(b);
		Vec3V cp = V3Neg(c);

		FloatV d1 = V3Dot(ab_, ap); //  snom
		FloatV d2 = V3Dot(ac_, ap); //  tnom
		FloatV d3 = V3Dot(ab_, bp); // -sdenom
		FloatV d4 = V3Dot(ac_, bp); //  unom = d4 - d3
		FloatV d5 = V3Dot(ab_, cp); //  udenom = d5 - d6
		FloatV d6 = V3Dot(ac_, cp); // -tdenom
	/*	FloatV unom = FSub(d4, d3);
		FloatV udenom = FSub(d5, d6);*/

		FloatV va = FNegScaleSub(d5, d4, FMul(d3, d6));//edge region of BC
		FloatV vb = FNegScaleSub(d1, d6, FMul(d5, d2));//edge region of AC
		FloatV vc = FNegScaleSub(d3, d2, FMul(d1, d4));//edge region of AB

		//check if p in vertex region outside a
		const BoolV con00 = FIsGrtrOrEq(zero, d1); // snom <= 0
		const BoolV con01 = FIsGrtrOrEq(zero, d2); // tnom <= 0
		const BoolV con0 = BAnd(con00, con01); // vertex region a
		if(BAllEq(con0, bTrue))
		{
			//size = 1;
			closestA = A[ind0];
			closestB = B[ind0];
			return Q[ind0];
		}

		//check if p in vertex region outside b
		const BoolV con10 = FIsGrtrOrEq(d3, zero);
		const BoolV con11 = FIsGrtrOrEq(d3, d4);
		const BoolV con1 = BAnd(con10, con11); // vertex region b
		if(BAllEq(con1, bTrue))
		{
			/*size = 1;
			indices[0] = ind1;*/
			closestA = A[ind1];
			closestB = B[ind1];
			return Q[ind1];
		}


		//check if p in vertex region outside of c
		const BoolV con20 = FIsGrtrOrEq(d6, zero);
		const BoolV con21 = FIsGrtrOrEq(d6, d5); 
		const BoolV con2 = BAnd(con20, con21); // vertex region c
		if(BAllEq(con2, bTrue))
		{
			closestA = A[ind2];
			closestB = B[ind2];
			return Q[ind2];
		}

		//check if p in edge region of AB
		const BoolV con30 = FIsGrtrOrEq(zero, vc);
		const BoolV con31 = FIsGrtrOrEq(d1, zero);
		const BoolV con32 = FIsGrtrOrEq(zero, d3);
		const BoolV con3 = BAnd(con30, BAnd(con31, con32));

		if(BAllEq(con3, bTrue))
		{
			//size = 2;
			//p in edge region of AB, split AB
			return closestPtPointSegmentTesselation(Q[ind0], Q[ind1], A[ind0], A[ind1], B[ind0], B[ind1], size, closestA, closestB);
		}

		//check if p in edge region of BC
		const BoolV con40 = FIsGrtrOrEq(zero, va);
		const BoolV con41 = FIsGrtrOrEq(d4, d3);
		const BoolV con42 = FIsGrtrOrEq(d5, d6);
		const BoolV con4 = BAnd(con40, BAnd(con41, con42)); 

		if(BAllEq(con4, bTrue))
		{
			//p in edge region of BC, split BC
			return closestPtPointSegmentTesselation(Q[ind1], Q[ind2], A[ind1], A[ind2], B[ind1], B[ind2], size, closestA, closestB);
		}



		//check if p in edge region of AC
		const BoolV con50 = FIsGrtrOrEq(zero, vb);
		const BoolV con51 = FIsGrtrOrEq(d2, zero);
		const BoolV con52 = FIsGrtrOrEq(zero, d6);
		const BoolV con5 = BAnd(con50, BAnd(con51, con52));

		if(BAllEq(con5, bTrue))
		{
			//p in edge region of AC, split AC
			return closestPtPointSegmentTesselation(Q[ind0], Q[ind2], A[ind0], A[ind2], B[ind0], B[ind2], size, closestA, closestB);
		}
		
		size = 3;

		Vec3V q0 = Q[ind0];
		Vec3V q1 = Q[ind1];
		Vec3V q2 = Q[ind2];
		Vec3V a0 = A[ind0];
		Vec3V a1 = A[ind1];
		Vec3V a2 = A[ind2];
		Vec3V b0 = B[ind0];
		Vec3V b1 = B[ind1];
		Vec3V b2 = B[ind2];

		do
		{

			const Vec3V ab = V3Sub(q1, q0);
			const Vec3V ac = V3Sub(q2, q0);
			const Vec3V bc = V3Sub(q2, q1);

			const FloatV dab = V3Dot(ab, ab);
			const FloatV dac = V3Dot(ac, ac);
			const FloatV dbc = V3Dot(bc, bc);

			const FloatV fMax = FMax(dab, FMax(dac, dbc));
			const FloatV fMin = FMin(dab, FMin(dac, dbc));

			const Vec3V w = V3Cross(ab, ac);

			const FloatV area = V3Length(w);
			const FloatV ratio = FDiv(FSqrt(fMax), FSqrt(fMin));
			if(FAllGrtr(four, ratio) && FAllGrtr(sixty , area))
				break;

			//calculate the triangle normal
			const Vec3V triNormal = V3Normalize(w);

			PX_ASSERT(V3AllEq(triNormal, V3Zero()) == 0);

			
			//split the longest edge
			if(FAllGrtrOrEq(dab, dac) && FAllGrtrOrEq(dab, dbc))
			{
				//split edge q0q1
				const Vec3V midPoint = V3Scale(V3Add(q0, q1), half);
				const Vec3V midA = V3Scale(V3Add(a0, a1), half);
				const Vec3V midB = V3Scale(V3Add(b0, b1), half);

				const Vec3V v = V3Sub(midPoint, q2);
				const Vec3V n = V3Normalize(V3Cross(v, triNormal));

				const FloatV d = FNeg(V3Dot(n, midPoint));
				const FloatV dp = FAdd(V3Dot(n, q0), d);
				const FloatV sum = FMul(d, dp);

				if(FAllGrtr(sum, zero))
				{
					//q0 and origin at the same side, split triangle[q0, m, q2]
					q1 = midPoint;
					a1 = midA;
					b1 = midB;
				}
				else
				{
					//q1 and origin at the same side, split triangle[m, q1, q2]
					q0 = midPoint;
					a0 = midA;
					b0 = midB;
				}
			
			}
			else if(FAllGrtrOrEq(dac, dbc))
			{
				//split edge q0q2
				const Vec3V midPoint = V3Scale(V3Add(q0, q2), half);
				const Vec3V midA = V3Scale(V3Add(a0, a2), half);
				const Vec3V midB = V3Scale(V3Add(b0, b2), half);

				const Vec3V v = V3Sub(midPoint, q1);
				const Vec3V n = V3Normalize(V3Cross(v, triNormal));

				const FloatV d = FNeg(V3Dot(n, midPoint));
				const FloatV dp = FAdd(V3Dot(n, q0), d);
				const FloatV sum = FMul(d, dp);

				if(FAllGrtr(sum, zero))
				{
					//q0 and origin at the same side, split triangle[q0, q1, m]
					q2 = midPoint;
					a2 = midA;
					b2 = midB;
				}
				else
				{
					//q2 and origin at the same side, split triangle[m, q1, q2]
					q0 = midPoint;
					a0 = midA;
					b0 = midB;
				}
			}
			else
			{
				//split edge q1q2
				const Vec3V midPoint = V3Scale(V3Add(q1, q2), half);
				const Vec3V midA = V3Scale(V3Add(a1, a2), half);
				const Vec3V midB = V3Scale(V3Add(b1, b2), half);

				const Vec3V v = V3Sub(midPoint, q0);
				const Vec3V n = V3Normalize(V3Cross(v, triNormal));

				const FloatV d = FNeg(V3Dot(n, midPoint));
				const FloatV dp = FAdd(V3Dot(n, q1), d);
				const FloatV sum = FMul(d, dp);

				if(FAllGrtr(sum, zero))
				{
					//q1 and origin at the same side, split triangle[q0, q1, m]
					q2 = midPoint;
					a2 = midA;
					b2 = midB;
				}
				else
				{
					//q2 and origin at the same side, split triangle[q0, m, q2]
					q1 = midPoint;
					a1 = midA;
					b1 = midB;
				}

			
			}
		}while(1);

		//P must project inside face region. Compute Q using Barycentric coordinates
		ab_ = V3Sub(q1, q0);
		ac_ = V3Sub(q2, q0);
		ap = V3Neg(q0);
		bp = V3Neg(q1);
		cp = V3Neg(q2);

		d1 = V3Dot(ab_, ap); //  snom
		d2 = V3Dot(ac_, ap); //  tnom
		d3 = V3Dot(ab_, bp); // -sdenom
		d4 = V3Dot(ac_, bp); //  unom = d4 - d3
		d5 = V3Dot(ab_, cp); //  udenom = d5 - d6
		d6 = V3Dot(ac_, cp); // -tdenom

		va = FNegScaleSub(d5, d4, FMul(d3, d6));//edge region of BC
		vb = FNegScaleSub(d1, d6, FMul(d5, d2));//edge region of AC
		vc = FNegScaleSub(d3, d2, FMul(d1, d4));//edge region of AB

		const FloatV toRecipD = FAdd(va, FAdd(vb, vc));
		const FloatV denom = FRecip(toRecipD);//V4GetW(recipTmp);
		const Vec3V v0 = V3Sub(a1, a0);
		const Vec3V v1 = V3Sub(a2, a0);
		const Vec3V w0 = V3Sub(b1, b0);
		const Vec3V w1 = V3Sub(b2, b0);

		const FloatV t = FMul(vb, denom);
		const FloatV w = FMul(vc, denom);
		const Vec3V vA1 = V3Scale(v1, w);
		const Vec3V vB1 = V3Scale(w1, w);
		const Vec3V tempClosestA = V3Add(a0, V3ScaleAdd(v0, t, vA1));
		const Vec3V tempClosestB = V3Add(b0, V3ScaleAdd(w0, t, vB1));
		closestA = tempClosestA;
		closestB = tempClosestB;
		return V3Sub(tempClosestA, tempClosestB);

	}


	PX_NOALIAS Ps::aos::Vec3V closestPtPointTetrahedronTesselation(Ps::aos::Vec3V* PX_RESTRICT Q, Ps::aos::Vec3V* PX_RESTRICT A, Ps::aos::Vec3V* PX_RESTRICT B, PxU32& size, Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB)
	{
		using namespace Ps::aos;
		const FloatV eps = FEps();
		const Vec3V zeroV = V3Zero();
		PxU32 tempSize = size;
	
		
		FloatV bestSqDist = FLoad(PX_MAX_REAL);
		const Vec3V a = Q[0];
		const Vec3V b = Q[1];
		const Vec3V c = Q[2];
		const Vec3V d = Q[3];
		const BoolV bTrue = BTTTT();
		const BoolV bFalse = BFFFF();

		//degenerated
		const Vec3V ad = V3Sub(d, a);
		const Vec3V bd = V3Sub(d, b);
		const Vec3V cd = V3Sub(d, c);
		const FloatV dad = V3Dot(ad, ad);
		const FloatV dbd = V3Dot(bd, bd);
		const FloatV dcd = V3Dot(cd, cd);
		const FloatV fMin = FMin(dad, FMin(dbd, dcd));
		if(FAllGrtr(eps, fMin))
		{
			size = 3;
			PxU32 tempIndices[] = {0,1,2};
			return closestPtPointTriangleTesselation(Q, A, B, tempIndices, size, closestA, closestB);
		}

		Vec3V _Q[] = {Q[0], Q[1], Q[2], Q[3]};
		Vec3V _A[] = {A[0], A[1], A[2], A[3]};
		Vec3V _B[] = {B[0], B[1], B[2], B[3]};

		PxU32 indices[3] = {0, 1, 2};

		const BoolV bIsOutside4 = PointOutsideOfPlane4(a, b, c, d);

		if(BAllEq(bIsOutside4, bFalse))
		{
			//origin is inside the tetrahedron, we are done
			return zeroV;
		}
	
		Vec3V result = zeroV;
		Vec3V tempClosestA, tempClosestB;

		if(BAllEq(BGetX(bIsOutside4), bTrue))
		{

			PxU32 tempIndices[] = {0,1,2};
			PxU32 _size = 3;

			result = closestPtPointTriangleTesselation(_Q, _A, _B,  tempIndices, _size, tempClosestA, tempClosestB);

			const FloatV sqDist = V3Dot(result, result);
			bestSqDist = sqDist;
			
			indices[0] = tempIndices[0];
			indices[1] = tempIndices[1];
			indices[2] = tempIndices[2];
			
			tempSize = _size;
			closestA = tempClosestA;
			closestB = tempClosestB;
		}

		if(BAllEq(BGetY(bIsOutside4), bTrue))
		{

			PxU32 tempIndices[] = {0,2,3};

			PxU32 _size = 3;

			const Vec3V q = closestPtPointTriangleTesselation(_Q, _A, _B,  tempIndices, _size, tempClosestA, tempClosestB);

			const FloatV sqDist = V3Dot(q, q);
			const BoolV con = FIsGrtr(bestSqDist, sqDist);
			if(BAllEq(con, bTrue))
			{
				result = q;
				bestSqDist = sqDist;
				indices[0] = tempIndices[0];
				indices[1] = tempIndices[1];
				indices[2] = tempIndices[2];

				tempSize = _size;
				closestA = tempClosestA;
				closestB = tempClosestB;
			}
		}

		if(BAllEq(BGetZ(bIsOutside4), bTrue))
		{
			
			PxU32 tempIndices[] = {0,3,1};
			PxU32 _size = 3;

			const Vec3V q = closestPtPointTriangleTesselation(_Q, _A, _B,  tempIndices, _size, tempClosestA, tempClosestB);

			const FloatV sqDist = V3Dot(q, q);
			const BoolV con = FIsGrtr(bestSqDist, sqDist);
			if(BAllEq(con, bTrue))
			{
				result = q;
				bestSqDist = sqDist;
				indices[0] = tempIndices[0];
				indices[1] = tempIndices[1];
				indices[2] = tempIndices[2];
				tempSize = _size;
				closestA = tempClosestA;
				closestB = tempClosestB;
			}

		}

		if(BAllEq(BGetW(bIsOutside4), bTrue))
		{
	
			PxU32 tempIndices[] = {1,3,2};
			PxU32 _size = 3;

			const Vec3V q = closestPtPointTriangleTesselation(_Q, _A, _B,  tempIndices, _size, tempClosestA, tempClosestB);

			const FloatV sqDist = V3Dot(q, q);
			const BoolV con = FIsGrtr(bestSqDist, sqDist);

			if(BAllEq(con, bTrue))
			{
				result = q;
				bestSqDist = sqDist;

				indices[0] = tempIndices[0];
				indices[1] = tempIndices[1];
				indices[2] = tempIndices[2];

				tempSize = _size;
				closestA = tempClosestA;
				closestB = tempClosestB;
			}
		}

		A[0] = _A[indices[0]]; A[1] = _A[indices[1]]; A[2] = _A[indices[2]];
		B[0] = _B[indices[0]]; B[1] = _B[indices[1]]; B[2] = _B[indices[2]];
		Q[0] = _Q[indices[0]]; Q[1] = _Q[indices[1]]; Q[2] = _Q[indices[2]];


		size = tempSize;
		return result;
	}


}

}
