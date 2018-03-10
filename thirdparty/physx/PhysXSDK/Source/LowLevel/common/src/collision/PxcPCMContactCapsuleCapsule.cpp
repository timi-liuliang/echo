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

#include "GuVecCapsule.h"
#include "GuGeometryUnion.h"

#include "PxcNpCache.h"
#include "PxcNpThreadContext.h"
#include "PxcContactMethodImpl.h"


namespace physx
{

/*
	S0 = origin + extent * dir;
	S1 = origin + extent * dir;
	dir is the vector from start to end point
	p1 is the start point of segment1
	d1 is the direction vector(q1 - p1)
	p2 is the start point of segment2
	d2 is the direction vector(q2 - p2) 
*/

Ps::aos::FloatV pcmDistanceSegmentSegmentSquared(const Ps::aos::Vec3VArg p1, const Ps::aos::Vec3VArg d1,const Ps::aos::Vec3VArg p2, const Ps::aos::Vec3VArg d2,
									Ps::aos::FloatV& s, Ps::aos::FloatV& t)
{
	using namespace Ps::aos;
	const FloatV zero = FZero();
	const FloatV one = FOne();
	const FloatV eps = FEps();

	const Vec3V r = V3Sub(p1, p2);
	//const FloatV a = V3Dot(d1, d1);//squared length of segment1
	//const FloatV e = V3Dot(d2, d2);//squared length of segment2
	//const FloatV b = V3Dot(d1, d2);
	//const FloatV c = V3Dot(d1, r);
	const Vec4V combinedDot = V3Dot4(d1, d1, d2, d2, d1, d2, d1, r);
	const Vec4V combinedRecip = V4Sel(V4IsGrtr(combinedDot, V4Splat(eps)), V4Recip(combinedDot), V4Splat(zero));
	const FloatV a = V4GetX(combinedDot);
	const FloatV e = V4GetY(combinedDot);
	const FloatV b = V4GetZ(combinedDot);
	const FloatV c = V4GetW(combinedDot);
	const FloatV aRecip = V4GetX(combinedRecip);//FSel(FIsGrtr(a, eps), FRecip(a), zero);
	const FloatV eRecip = V4GetY(combinedRecip);//FSel(FIsGrtr(e, eps), FRecip(e), zero);

	const FloatV f = V3Dot(d2, r);

	/*
		s = (b*f - c*e)/(a*e - b*b);
		t = (a*f - b*c)/(a*e - b*b);

		s = (b*t - c)/a;
		t = (b*s + f)/e;
	*/
	
	//if segments not parallel, the general non-degenerated case, compute closest point on two segments and clamp to segment1
	const FloatV denom = FSub(FMul(a, e), FMul(b, b));
	const FloatV temp = FSub(FMul(b, f), FMul(c, e));
	//const FloatV s0 = FClamp(FMul(temp, denomRecip), zero, one);
	const FloatV s0 = FClamp(FDiv(temp, denom), zero, one);

	//check whether the segments are parallel
	const BoolV con2 = FIsGrtr(eps, denom);//FIsEq(denom, zero);
	
	const FloatV sTmp = FSel(con2, FHalf(), s0);
	
	//compute point on segment2 closest to segment1
	//const FloatV tTmp = FMul(FAdd(FMul(b, sTmp), f), eRecip);
	const FloatV tTmp = FMul(FScaleAdd(b, sTmp, f), eRecip);

	//if t is in [zero, one], done. otherwise clamp t
	const FloatV t2 = FClamp(tTmp, zero, one);

	//recompute s for the new value
	const FloatV comp = FMul(FSub(FMul(b,t2), c), aRecip);

	const FloatV s2 = FClamp(comp, zero, one);

	s = s2;
	t = t2;

	const Vec3V c1 = V3ScaleAdd(d1, s2, p1);//V3Add(p1, V3Scale(d1, tempS));
	const Vec3V c2 = V3ScaleAdd(d2, t2, p2);//V3Add(p2, V3Scale(d2, tempT));
	const Vec3V v = V3Sub(c1, c2);
	return V3Dot(v, v);
}

//Ps::aos::FloatV PCMDistanceSegmentSegmentSquared(const Ps::aos::Vec3VArg p1, 
//														const Ps::aos::Vec3VArg d1,
//														const Ps::aos::Vec3VArg p2, 
//														const Ps::aos::Vec3VArg d2,
//														Ps::aos::FloatV& s, 
//														Ps::aos::FloatV& t)
//{
//	using namespace Ps::aos;
//	const FloatV zero = FZero();
//	const FloatV one = FOne();
//	const FloatV half = FloatV_From_F32(0.5f);
//	const FloatV eps = FEps();
//	//const FloatV parallelTolerance  = FloatV_From_F32(PX_PARALLEL_TOLERANCE);
//
//	//const Vec3V d1 = V3Sub(q1, p1); //direction vector of segment1
//	//const Vec3V d2 = V3Sub(q2, p2); //direction vector of segment2
//	const Vec3V r = V3Sub(p1, p2);
//	const FloatV a = V3Dot(d1, d1);//squared length of segment1
//	const FloatV e = V3Dot(d2, d2);//squared length of segment2
//	const FloatV aRecip = FRecip(a);
//	const FloatV eRecip = FRecip(e);
//	const FloatV b = V3Dot(d1, d2);
//	const FloatV c = V3Dot(d1, r);
//	const FloatV nc = FNeg(c);
//	const FloatV d = FMul(nc, aRecip);//when t = zero
//	const FloatV cd = FClamp(d, zero, one);
//	const FloatV f = V3Dot(d2, r);
//	const FloatV g = FMul(f, eRecip);
//	const FloatV cg = FClamp(g, zero, one); //when s = zero
//	//const FloatV h = FMul(nc, a);
//
//	/*
//		s = (b*f - c*e)/(a*e - b*b);
//		t = (a*f - b*c)/(a*e - b*b);
//
//		s = (b*t - c)/a;
//		t = (b*s + f)/e;
//	*/
//	
//
//	//check if either or both segments degenerate into points
//	const BoolV con0 = FIsGrtr(eps, a);
//	const BoolV con1 = FIsGrtr(eps, e);
//		
//	//the general nondegenerate case
//
//	//if segments not parallell, compute closest point on two segments and clamp to segment1
//	const FloatV denom = FSub(FMul(a, e), FMul(b, b));
//	const FloatV denomRecip = FRecip(denom);
//	const FloatV temp = FSub(FMul(b, f), FMul(c, e));
//	const FloatV s0 = FClamp(FMul(temp, denomRecip), zero, one);
//	
//	//if segment is parallel
//	//const BoolV con00 = FIsGrtr(b, zero);//segment p1q1 and p2q2 point to the same direction
//	//const BoolV con01 = FIsGrtrOrEq(c, zero);
//	//const BoolV con02 = FIsGrtr(a, nc);
//
//	//const FloatV s01 = FSel(con01, zero, FSel(con02, h, one));
//	//const FloatV s02 = FSel(con02, FSel(con01, zero, d), one);
//	//const FloatV s1 = FSel(con00, s02, s01);
//
//	
//	//const BoolV con2 = FIsGrtrOrEq( parallelTolerance, FAbs(denom));
//	const BoolV con2 = FIsEq(denom, zero);
//	const FloatV sTmp = FSel(con2, half, s0);
//	
//	//compute point on segment2 closest to segment1
//	const FloatV tTmp = FMul(FAdd(FMul(b, sTmp), f), eRecip);
//
//	//if t is in [zero, one], done. otherwise clamp t
//	const FloatV t2 = FClamp(tTmp, zero, one);
//
//	//recompute s for the new value
//	const FloatV comp = FMul(FSub(FMul(b,t2), c), aRecip);
//	const FloatV s2 = FClamp(comp, zero, one);
//
//	const FloatV tempS = FSel(con0, zero, FSel(con1, cd, s2));
//	const FloatV tempT = FSel(con1, zero, FSel(con0, cg, t2));
//
//	s = tempS;
//	t = tempT;
//
//	const Vec3V closest1 = V3Add(p1, V3Scale(d1, tempS));
//	const Vec3V closest2 = V3Add(p2, V3Scale(d2, tempT));
//	const Vec3V vv = V3Sub(closest1, closest2);
//	return V3Dot(vv, vv);
//}

static Ps::aos::Vec4V pcmDistancePointSegmentTValue22(const Ps::aos::Vec3VArg a0, const Ps::aos::Vec3VArg b0, 
																	const Ps::aos::Vec3VArg a1, const Ps::aos::Vec3VArg b1,
																	const Ps::aos::Vec3VArg p0, const Ps::aos::Vec3VArg p1,
																	const Ps::aos::Vec3VArg p2, const Ps::aos::Vec3VArg p3)
{
	using namespace Ps::aos;
	const Vec4V zero = V4Zero();
	const Vec3V ap00 = V3Sub(p0, a0);
	const Vec3V ap10 = V3Sub(p1, a0);
	const Vec3V ap01 = V3Sub(p2, a1);
	const Vec3V ap11 = V3Sub(p3, a1);

	const Vec3V ab0 = V3Sub(b0, a0);
	const Vec3V ab1 = V3Sub(b1, a1);

/*	const FloatV nom00 = V3Dot(ap00, ab0);
	const FloatV nom10 = V3Dot(ap10, ab0);
	const FloatV nom01 = V3Dot(ap01, ab1);
	const FloatV nom11 = V3Dot(ap11, ab1);*/

	const Vec4V combinedDot = V3Dot4(ap00, ab0, ap10, ab0, ap01, ab1, ap11, ab1);
	const FloatV nom00 = V4GetX(combinedDot);
	const FloatV nom10 = V4GetY(combinedDot);
	const FloatV nom01 = V4GetZ(combinedDot);
	const FloatV nom11 = V4GetW(combinedDot);
	
	const FloatV denom0 = V3Dot(ab0, ab0);
	const FloatV denom1 = V3Dot(ab1, ab1);

	const Vec4V nom = V4Merge(nom00, nom10, nom01, nom11);
	const Vec4V denom = V4Merge(denom0, denom0, denom1, denom1);

	const Vec4V tValue = V4Div(nom, denom);
	return V4Sel(V4IsEq(denom, zero), zero, tValue);
}

//static PxU32 generateContact(const Ps::aos::Vec3VArg a, const Ps::aos::Vec3VArg ab, const Ps::aos::FloatVArg t, const Ps::aos::Vec3VArg q, const Ps::aos::FloatVArg inflatedSumSquared,
//							  const Ps::aos::FloatVArg sumRadius, const Ps::aos::FloatVArg radius, const Ps::aos::Vec3VArg positionOffset, Gu::ContactBuffer& contactBuffer)
//{
//	using namespace Ps::aos;
//
//	const FloatV eps = FloatV_From_F32(1e-6);//FEps();
//	const BoolV bTrue = BTTTT();
//	const Vec3V point = V3ScaleAdd(ab, t, a);
//	const Vec3V v = V3Sub(point, q);
//	const FloatV sqDist = V3Dot(v, v);
//	const BoolV bCon = BAnd(FIsGrtr(sqDist, eps), FIsGrtr(inflatedSumSquared, sqDist));
//	
//	if(BAllEq(bCon, bTrue))
//	{
//		const FloatV dist = FSqrt(sqDist);
//		const FloatV pen =  FSub(dist, sumRadius);
//		const Vec3V normal = V3ScaleInv(v, dist);
//		PX_ASSERT(isFiniteVec3V(normal));
//		const Vec3V _p = V3NegScaleSub(normal, radius, point);
//		const Vec3V p = V3Add(_p, positionOffset);
//		//const Vec3V p = V3ScaleAdd(normal, r0, projS1);
//		contactBuffer.contact(p, normal, pen);
//		return 1;
//	}
//	return 0;
//}

bool PxcPCMContactCapsuleCapsule(CONTACT_METHOD_ARGS)
{
	PX_UNUSED(npCache);

	using namespace Ps::aos;
	// Get actual shape data
	const PxCapsuleGeometry& shapeCapsule0 = shape0.get<const PxCapsuleGeometry>();
	const PxCapsuleGeometry& shapeCapsule1 = shape1.get<const PxCapsuleGeometry>();

	PX_ASSERT(transform1.q.isSane());
	PX_ASSERT(transform0.q.isSane());

	const Vec3V _p0 = V3LoadA(&transform0.p.x);
	const QuatV q0 = QuatVLoadA(&transform0.q.x);

	const Vec3V _p1 = V3LoadA(&transform1.p.x);
	const QuatV q1 = QuatVLoadA(&transform1.q.x);

	/*PsTransformV transf0(p0, q0);
	PsTransformV transf1(p1, q1);*/

	
	const FloatV r0 = FLoad(shapeCapsule0.radius);
	const FloatV halfHeight0 = FLoad(shapeCapsule0.halfHeight);

	const FloatV r1 = FLoad(shapeCapsule1.radius);
	const FloatV halfHeight1 = FLoad(shapeCapsule1.halfHeight);

	const FloatV cDist = FLoad(contactDistance);

	const Vec3V positionOffset = V3Scale(V3Add(_p0, _p1),  FHalf());
	const Vec3V p0 = V3Sub(_p0, positionOffset);
	const Vec3V p1 = V3Sub(_p1, positionOffset);

	const FloatV zero = FZero();
	//const FloatV one = FOne();
	const Vec3V zeroV = V3Zero();
	

	/*const Vec3V positionOffset = V3Scale(V3Add(transf0.p, transf1.p), FloatV_From_F32(0.5f));
	transf0.p = V3Sub(transf0.p, positionOffset);
	transf1.p = V3Sub(transf1.p, positionOffset);*/

	const Vec3V basisVector0 = QuatGetBasisVector0(q0);
	const Vec3V tmp0 = V3Scale(basisVector0, halfHeight0);
	const Vec3V s0 = V3Add(p0, tmp0);
	const Vec3V e0 = V3Sub(p0, tmp0);
	const Vec3V d0 = V3Sub(e0, s0);

	const Vec3V basisVector1 = QuatGetBasisVector0(q1);
	const Vec3V tmp1 = V3Scale(basisVector1, halfHeight1);
	const Vec3V s1 = V3Add(p1, tmp1);
	const Vec3V e1 = V3Sub(p1, tmp1); 

	const Vec3V d1 = V3Sub(e1, s1);

	const FloatV sumRadius = FAdd(r0, r1);
	const FloatV inflatedSum = FAdd(sumRadius, cDist);
	const FloatV inflatedSumSquared = FMul(inflatedSum, inflatedSum);
	const FloatV a = V3Dot(d0, d0);//squared length of segment1
	const FloatV e = V3Dot(d1, d1);//squared length of segment2
	const FloatV eps = FLoad(1e-6);//FEps();

	FloatV t0, t1;
	const FloatV sqDist0 = pcmDistanceSegmentSegmentSquared(s0, d0, s1, d1, t0, t1);

	if(FAllGrtrOrEq(inflatedSumSquared, sqDist0))
	{
		const Vec4V zeroV4 = V4Zero();
		const Vec4V oneV4 = V4One();
		const BoolV bTrue = BTTTT();
		//check to see whether these two capsule are paralle
		const FloatV parallelTolerance  = FLoad(0.9998f);
		
		
		const BoolV con0 = FIsGrtr(eps, a);
		const BoolV con1 = FIsGrtr(eps, e);
		const Vec3V dir0 = V3Sel(con0, zeroV, V3ScaleInv(d0, FSqrt(a)));
		const Vec3V dir1 = V3Sel(con1, zeroV, V3ScaleInv(d1, FSqrt(e)));

		const FloatV cos = FAbs(V3Dot(dir0, dir1));
		if(FAllGrtr(cos, parallelTolerance))//paralle
		{
			//project s, e into s1e1
			const Vec4V t= pcmDistancePointSegmentTValue22(s0, e0, s1, e1,
															s1, e1, s0, e0);

			const BoolV con = BAnd(V4IsGrtrOrEq(t, zeroV4), V4IsGrtrOrEq(oneV4, t));
			const BoolV con00 = BGetX(con);
			const BoolV con01 = BGetY(con);
			const BoolV con10 = BGetZ(con);
			const BoolV con11 = BGetW(con);

		/*	PX_ALIGN(16, PxU32 conditions[4]);
			F32Array_Aligned_From_Vec4V(con, (PxF32*)conditions);*/

			
			PxU32 numContact=0;

			if(BAllEq(con00, bTrue))
			{
				const Vec3V projS1 = V3ScaleAdd(d0, V4GetX(t), s0);
				const Vec3V v = V3Sub(projS1, s1);
				const FloatV sqDist = V3Dot(v, v);
				const BoolV bCon = BAnd(FIsGrtr(sqDist, eps), FIsGrtr(inflatedSumSquared, sqDist));
				
				if(BAllEq(bCon, bTrue))
				{
					const FloatV dist = FSqrt(sqDist);
					const FloatV pen =  FSub(dist, sumRadius);
					const Vec3V normal = V3ScaleInv(v, dist);
					PX_ASSERT(isFiniteVec3V(normal));
					const Vec3V _p = V3NegScaleSub(normal, r0, projS1);
					const Vec3V p = V3Add(_p, positionOffset);
					//const Vec3V p = V3ScaleAdd(normal, r0, projS1);
					contactBuffer.contact(p, normal, pen);
					numContact++;
				}
			}
			if(BAllEq(con01, bTrue))
			{
				const Vec3V projE1 = V3ScaleAdd(d0, V4GetY(t), s0);
				const Vec3V v = V3Sub(projE1, e1);
				const FloatV sqDist = V3Dot(v, v);
				const BoolV bCon = BAnd(FIsGrtr(sqDist, eps), FIsGrtr(inflatedSumSquared, sqDist));
				
				if(BAllEq(bCon, bTrue))
				{
					const FloatV dist = FSqrt(sqDist);
					const FloatV pen =  FSub(dist, sumRadius);
					const Vec3V normal =  V3ScaleInv(v, dist);
					PX_ASSERT(isFiniteVec3V(normal));
					const Vec3V _p = V3NegScaleSub(normal, r0, projE1);
					const Vec3V p = V3Add(_p, positionOffset);
					contactBuffer.contact(p, normal, pen);
					numContact++;
				}
			}

			if(BAllEq(con10, bTrue))
			{
				const Vec3V projS0 = V3ScaleAdd(d1, V4GetZ(t), s1);
				const Vec3V v = V3Sub(s0, projS0);
				const FloatV sqDist = V3Dot(v, v);
				const BoolV bCon = BAnd(FIsGrtr(sqDist, eps), FIsGrtr(inflatedSumSquared, sqDist));
				
				if(BAllEq(bCon, bTrue))
				{
					const FloatV dist = FSqrt(sqDist);
					const FloatV pen =  FSub(dist, sumRadius);
					const Vec3V normal = V3ScaleInv(v, dist);
				 	PX_ASSERT(isFiniteVec3V(normal));
					const Vec3V _p = V3NegScaleSub(normal, r0, s0);
					const Vec3V p = V3Add(_p, positionOffset);
					//const Vec3V p = V3ScaleAdd(normal, r0, s0);
					contactBuffer.contact(p, normal, pen);
					numContact++;
				}
			}

			if(BAllEq(con11, bTrue))
			{
				const Vec3V projE0 = V3ScaleAdd(d1, V4GetW(t), s1);
				const Vec3V v = V3Sub(e0, projE0);
				const FloatV sqDist = V3Dot(v, v);
				const BoolV bCon = BAnd(FIsGrtr(sqDist, eps), FIsGrtr(inflatedSumSquared, sqDist));
				
				if(BAllEq(bCon, bTrue))
				{
					const FloatV dist = FSqrt(sqDist);
					const FloatV pen =  FSub(dist, sumRadius);
					const Vec3V normal = V3ScaleInv(v, dist);
					PX_ASSERT(isFiniteVec3V(normal));
					const Vec3V _p = V3NegScaleSub(normal, r0, e0);
					const Vec3V p = V3Add(_p, positionOffset);
					//const Vec3V p = V3ScaleAdd(normal, r0, e0);
					contactBuffer.contact(p, normal, pen);
					numContact++;
				}
			}

			if(numContact)
				return true;

		}  

		const Vec3V closestA = V3ScaleAdd(d0, t0, s0);
		const Vec3V closestB = V3ScaleAdd(d1, t1, s1);
		
		const BoolV con = FIsGrtr(eps, sqDist0);
		//const Vec3V normal = V3Sel(FIsEq(dist, zero), V3Sel(FIsGrtr(a, eps), V3Normalise(d0), V3Scale(V3Sub(closestA, closestB), FRecip(dist)));
		const Vec3V _normal = V3Sel(con, V3Sel(FIsGrtr(a, eps), d0, V3UnitX()), V3Sub(closestA, closestB));
		const Vec3V normal = V3Normalize(_normal);
		PX_ASSERT(isFiniteVec3V(normal));
		const Vec3V _point = V3NegScaleSub(normal, r0, closestA);
		const Vec3V point = V3Add(_point, positionOffset);
		const FloatV dist = FSel(con, zero, FSqrt(sqDist0));
		const FloatV pen = FSub(dist, sumRadius);
		//PX_ASSERT(FAllGrtrOrEq(zero, pen));
		contactBuffer.contact(point, normal, pen);
		return true;
		
	}

	return false;
}
}
