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

#include "GuIntersectionTriangleBox.h"
#include "GuIntersectionTriangleBoxSIMD.h"
#include "PxVec3.h"
#include "CmMatrix34.h"

using namespace physx;

//!!! Delete counterparts in ContactBoxConvex.cpp (and OPC_CommonColliderOverlap.h)

/********************************************************/
/* AABB-triangle overlap test code                      */
/* by Tomas Akenine-M?r									*/
/* Function: int triBoxOverlap(float boxcenter[3],      */
/*          float boxhalfsize[3],float triverts[3][3]); */
/* History:                                             */
/*   2001-03-05: released the code in its first version */
/*   2001-06-18: changed the order of the tests, faster */
/*                                                      */
/* Acknowledgement: Many thanks to Pierre Terdiman for  */
/* suggestions and discussions on how to optimize code. */
/* Thanks to David Hunt for finding a ">="-bug!         */
/********************************************************/

#define CROSS(dest,v1,v2)		\
	dest.x=v1.y*v2.z-v1.z*v2.y;	\
	dest.y=v1.z*v2.x-v1.x*v2.z;	\
	dest.z=v1.x*v2.y-v1.y*v2.x; 

#define DOT(v1,v2) (v1.x*v2.x+v1.y*v2.y+v1.z*v2.z)

#define FINDMINMAX(x0, x1, x2, minimum, maximum)			\
	minimum = physx::intrinsics::selectMin(x0, x1);			\
	maximum = physx::intrinsics::selectMax(x0, x1);			\
	minimum = physx::intrinsics::selectMin(minimum, x2);	\
	maximum = physx::intrinsics::selectMax(maximum, x2);

static PX_FORCE_INLINE Ps::IntBool planeBoxOverlap(const PxVec3& normal, PxReal d, const PxVec3& maxbox)
{
	PxVec3 vmin,vmax;

	if (normal.x>0.0f)
	{
		vmin.x = -maxbox.x;
		vmax.x = maxbox.x;
	}
	else
	{
		vmin.x = maxbox.x;
		vmax.x = -maxbox.x;
	}

	if (normal.y>0.0f)
	{
		vmin.y = -maxbox.y;
		vmax.y = maxbox.y;
	}
	else
	{
		vmin.y = maxbox.y;
		vmax.y = -maxbox.y;
	}

	if (normal.z>0.0f)
	{
		vmin.z = -maxbox.z;
		vmax.z = maxbox.z;
	}
	else
	{
		vmin.z = maxbox.z;
		vmax.z = -maxbox.z;
	}

	if( normal.dot(vmin) + d >  0.0f) return Ps::IntFalse;
	if( normal.dot(vmax) + d >= 0.0f) return Ps::IntTrue;
	return Ps::IntFalse;
}

/*======================== X-tests ========================*/
#define AXISTEST_X01(a, b, fa, fb)							\
	p0 = a*v0.y - b*v0.z;									\
	p2 = a*v2.y - b*v2.z;									\
	minimum = physx::intrinsics::selectMin(p0, p2);			\
	maximum = physx::intrinsics::selectMax(p0, p2);			\
	rad = fa * extents.y + fb * extents.z;					\
	if(minimum>rad || maximum<-rad) return Ps::IntFalse;

#define AXISTEST_X2(a, b, fa, fb)							\
	p0 = a*v0.y - b*v0.z;									\
	p1 = a*v1.y - b*v1.z;									\
	minimum = physx::intrinsics::selectMin(p0, p1);			\
	maximum = physx::intrinsics::selectMax(p0, p1);			\
	rad = fa * extents.y + fb * extents.z;					\
	if(minimum>rad || maximum<-rad) return Ps::IntFalse;

/*======================== Y-tests ========================*/
#define AXISTEST_Y02(a, b, fa, fb)							\
	p0 = -a*v0.x + b*v0.z;									\
	p2 = -a*v2.x + b*v2.z;									\
	minimum = physx::intrinsics::selectMin(p0, p2);			\
	maximum = physx::intrinsics::selectMax(p0, p2);			\
	rad = fa * extents.x + fb * extents.z;					\
	if(minimum>rad || maximum<-rad) return Ps::IntFalse;

#define AXISTEST_Y1(a, b, fa, fb)							\
	p0 = -a*v0.x + b*v0.z;									\
	p1 = -a*v1.x + b*v1.z;									\
	minimum = physx::intrinsics::selectMin(p0, p1);			\
	maximum = physx::intrinsics::selectMax(p0, p1);			\
	rad = fa * extents.x + fb * extents.z;					\
	if(minimum>rad || maximum<-rad) return Ps::IntFalse;

/*======================== Z-tests ========================*/
#define AXISTEST_Z12(a, b, fa, fb)							\
	p1 = a*v1.x - b*v1.y;									\
	p2 = a*v2.x - b*v2.y;									\
	minimum = physx::intrinsics::selectMin(p1, p2);			\
	maximum = physx::intrinsics::selectMax(p1, p2);			\
	rad = fa * extents.x + fb * extents.y;					\
	if(minimum>rad || maximum<-rad) return Ps::IntFalse;

#define AXISTEST_Z0(a, b, fa, fb)							\
	p0 = a*v0.x - b*v0.y;									\
	p1 = a*v1.x - b*v1.y;									\
	minimum = physx::intrinsics::selectMin(p0, p1);			\
	maximum = physx::intrinsics::selectMax(p0, p1);			\
	rad = fa * extents.x + fb * extents.y;					\
	if(minimum>rad || maximum<-rad) return Ps::IntFalse;

Ps::IntBool Gu::intersectTriangleBox(const PxVec3& boxcenter, const PxVec3& extents, const PxVec3& tp0, const PxVec3& tp1, const PxVec3& tp2)
{
	/*    use separating axis theorem to test overlap between triangle and box */
	/*    need to test for overlap in these directions: */
	/*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
	/*       we do not even need to test these) */
	/*    2) normal of the triangle */
	/*    3) crossproduct(edge from tri, {x,y,z}-directin) */
	/*       this gives 3x3=9 more tests */

	// This is the fastest branch on Sun - move everything so that the boxcenter is in (0,0,0)
	const PxVec3 v0 = tp0 - boxcenter;
	const PxVec3 v1 = tp1 - boxcenter;
	const PxVec3 v2 = tp2 - boxcenter;

	// compute triangle edges
	const PxVec3 e0 = v1 - v0;	// tri edge 0
	const PxVec3 e1 = v2 - v1;	// tri edge 1
	const PxVec3 e2 = v0 - v2;	// tri edge 2

	float minimum,maximum,rad,p0,p1,p2;  

	// Bullet 3: test the 9 tests first (this was faster)
	float fex = PxAbs(e0.x); 
	float fey = PxAbs(e0.y);
	float fez = PxAbs(e0.z);
	AXISTEST_X01(e0.z, e0.y, fez, fey);
	AXISTEST_Y02(e0.z, e0.x, fez, fex);
	AXISTEST_Z12(e0.y, e0.x, fey, fex);

	fex = PxAbs(e1.x);
	fey = PxAbs(e1.y);
	fez = PxAbs(e1.z);
	AXISTEST_X01(e1.z, e1.y, fez, fey);
	AXISTEST_Y02(e1.z, e1.x, fez, fex);
	AXISTEST_Z0(e1.y, e1.x, fey, fex);

	fex = PxAbs(e2.x);
	fey = PxAbs(e2.y);
	fez = PxAbs(e2.z);
	AXISTEST_X2(e2.z, e2.y, fez, fey);
	AXISTEST_Y1(e2.z, e2.x, fez, fex);
	AXISTEST_Z12(e2.y, e2.x, fey, fex);

	// Bullet 1:
	//  first test overlap in the {x,y,z}-directions
	//  find minimum, maximum of the triangle each direction, and test for overlap in
	//  that direction -- this is equivalent to testing a minimal AABB around
	//  the triangle against the AABB

	// test in X-direction
	FINDMINMAX(v0.x, v1.x, v2.x, minimum, maximum);
	if(minimum>extents.x || maximum<-extents.x) return Ps::IntFalse;

	// test in Y-direction
	FINDMINMAX(v0.y, v1.y, v2.y, minimum, maximum);
	if(minimum>extents.y || maximum<-extents.y) return Ps::IntFalse;

	// test in Z-direction
	FINDMINMAX(v0.z, v1.z, v2.z, minimum, maximum);
	if(minimum>extents.z || maximum<-extents.z) return Ps::IntFalse;

	// Bullet 2:
	//  test if the box intersects the plane of the triangle
	//  compute plane equation of triangle: normal*x+d=0
	PxVec3 normal;
	CROSS(normal,e0,e1);
	const float d=-DOT(normal,v0);	// plane eq: normal.x+d=0
	if(!planeBoxOverlap(normal, d, extents)) return Ps::IntFalse;

	return Ps::IntTrue;	// box and triangle overlaps
}

#undef CROSS
#undef DOT
#undef FINDMINMAX
#undef AXISTEST_X01
#undef AXISTEST_X2
#undef AXISTEST_Y02
#undef AXISTEST_Y1
#undef AXISTEST_Z12
#undef AXISTEST_Z0

//bool Gu::intersectTriangleBox(const Ps::aos::Vec3VArg boxCenter, const Ps::aos::Vec3VArg extents, const Ps::aos::Vec3VArg tp0, const Ps::aos::Vec3V& tp1, const Ps::aos::Vec3VArg tp2)
//{
//	using namespace Ps::aos;
//	const FloatV zero = FZero();
//	const Vec3V nExtents = V3Neg(extents);
//	// This is the fastest branch on Sun - move everything so that the boxcenter is in (0,0,0)
//	const Vec3V v0 = V3Sub(tp0, boxCenter);
//	const Vec3V v1 = V3Sub(tp1, boxCenter);
//	const Vec3V v2 = V3Sub(tp2, boxCenter);
//
//	// compute triangle edges
//	const Vec3V f0 = V3Sub(v1, v0);	// tri edge 0
//	const Vec3V f1 = V3Sub(v2, v1);	// tri edge 1
//	const Vec3V f2 = V3Sub(v0, v2);	// tri edge 2
//
//	//9 separating axis
//	const Vec3V a00 = V3Merge(zero, FNeg(V3GetZ(f0)), V3GetY(f0));
//	const Vec3V a01 = V3Merge(zero, FNeg(V3GetZ(f1)), V3GetY(f1));
//	const Vec3V a02 = V3Merge(zero, FNeg(V3GetZ(f2)), V3GetY(f2));
//
//	const Vec3V a10 = V3Merge(V3GetZ(f0), zero, FNeg(V3GetX(f0)));
//	const Vec3V a11 = V3Merge(V3GetZ(f1), zero, FNeg(V3GetX(f1)));
//	const Vec3V a12 = V3Merge(V3GetZ(f2), zero, FNeg(V3GetX(f2)));
//
//	const Vec3V a20 = V3Merge(FNeg(V3GetY(f0)), V3GetX(f0), zero);
//	const Vec3V a21 = V3Merge(FNeg(V3GetY(f1)), V3GetX(f1), zero);
//	const Vec3V a22 = V3Merge(FNeg(V3GetY(f2)), V3GetX(f2), zero);
//
//	//Axis a00
//	//triangle projection
//	const FloatV p000 = V3Dot(a00, v0); // p001 == p000
//	const FloatV p002 = V3Dot(a00, v2);
//	//box projection
//	const FloatV r00 = V3Dot(V3Abs(a00), extents);
//
//	const FloatV p00Min = FMin(p000, p002);
//	const FloatV p00Max = FMax(p000, p002);
//	const BoolV c00 = BOr(FIsGrtr(p00Min, r00), FIsGrtr(FNeg(r00), p00Max));
//
//	if(BAllEq(c00, BTTTT()))
//		return false;
//
//	//Axis a01
//	const FloatV p010 = V3Dot(a01, v0);
//	const FloatV p011 = V3Dot(a01, v1);//p011 == p012
//	
//	//box projection
//	const FloatV r01 = V3Dot(V3Abs(a01), extents);
//
//	const FloatV p01Min = FMin(p010, p011);
//	const FloatV p01Max = FMax(p010, p011);
//	const BoolV c01 = BOr(FIsGrtr(p01Min, r01), FIsGrtr(FNeg(r01), p01Max));
//
//	if(BAllEq(c01, BTTTT()))
//		return false;
//
//	//Axis a02
//	const FloatV p020 = V3Dot(a02, v0);//p022 == p020
//	const FloatV p021 = V3Dot(a02, v1);
//	
//	//box projection
//	const FloatV r02 = V3Dot(V3Abs(a02), extents);
//
//	const FloatV p02Min = FMin(p020, p021);
//	const FloatV p02Max = FMax(p020, p021);
//	const BoolV c02 = BOr(FIsGrtr(p02Min, r02), FIsGrtr(FNeg(r02), p02Max));
//
//	if(BAllEq(c02, BTTTT()))
//		return false;
//
//	//Axis a10
//	//triangle projection
//	const FloatV p100 = V3Dot(a10, v0); // p100 == p102
//	const FloatV p102 = V3Dot(a10, v2);
//	
//	//box projection
//	const FloatV r10 = V3Dot(V3Abs(a10), extents);
//
//	const FloatV p10Min = FMin(p100, p102);
//	const FloatV p10Max = FMax(p100, p102);
//	const BoolV c10 = BOr(FIsGrtr(p10Min, r10), FIsGrtr(FNeg(r10), p10Max));
//
//	if(BAllEq(c10, BTTTT()))
//		return false;
//
//
//	//Axis a11
//	//triangle projection
//	const FloatV p110 = V3Dot(a11, v0); 
//	const FloatV p111 = V3Dot(a11, v1);// p111 == p112
//	//box projection
//	const FloatV r11 = V3Dot(V3Abs(a11), extents);
//
//	const FloatV p11Min = FMin(p110, p111);
//	const FloatV p11Max = FMax(p110, p111);
//	const BoolV c11 = BOr(FIsGrtr(p11Min, r11), FIsGrtr(FNeg(r11), p11Max));
//
//	if(BAllEq(c11, BTTTT()))
//		return false;
//	
//
//	//Axis a12
//	//triangle projection
//	const FloatV p120 = V3Dot(a12, v0); // p120 == p122
//	const FloatV p121 = V3Dot(a12, v1);
//	//box projection
//	const FloatV r12 = V3Dot(V3Abs(a12), extents);
//
//	const FloatV p12Min = FMin(p120, p121);
//	const FloatV p12Max = FMax(p120, p121);
//	const BoolV c12 = BOr(FIsGrtr(p12Min, r12), FIsGrtr(FNeg(r12), p12Max));
//
//	if(BAllEq(c12, BTTTT()))
//		return false;
//
//	//Axis a20
//	//triangle projection
//	const FloatV p200 = V3Dot(a20, v0); // p200 == p201
//	const FloatV p202 = V3Dot(a20, v2);
//	//box projection
//	const FloatV r20 = V3Dot(V3Abs(a20), extents);
//
//	const FloatV p20Min = FMin(p200, p202);
//	const FloatV p20Max = FMax(p200, p202);
//	const BoolV c20 = BOr(FIsGrtr(p20Min, r20), FIsGrtr(FNeg(r20), p20Max));
//
//	if(BAllEq(c20, BTTTT()))
//		return false;
//
//
//	//Axis a21
//	//triangle projection
//	const FloatV p210 = V3Dot(a21, v0); 
//	const FloatV p211 = V3Dot(a21, v1);// p211 == p212
//	//box projection
//	const FloatV r21 = V3Dot(V3Abs(a21), extents);
//
//	const FloatV p21Min = FMin(p210, p211);
//	const FloatV p21Max = FMax(p210, p211);
//	const BoolV c21 = BOr(FIsGrtr(p21Min, r21), FIsGrtr(FNeg(r21), p21Max));
//
//	if(BAllEq(c21, BTTTT()))
//		return false;
//	
//
//	//Axis a22
//	//triangle projection
//	const FloatV p220 = V3Dot(a22, v0); // p220 == p222
//	const FloatV p221 = V3Dot(a22, v1);
//	//box projection
//	const FloatV r22 = V3Dot(V3Abs(a22), extents);
//
//	const FloatV p22Min = FMin(p220, p221);
//	const FloatV p22Max = FMax(p220, p221);
//	const BoolV c22 = BOr(FIsGrtr(p22Min, r22), FIsGrtr(FNeg(r22), p22Max));
//
//	if(BAllEq(c22, BTTTT()))
//		return false;
//
//
//	//test e0(1, 0, 0), e1(0,1,0), e2(0,0,1)
//	const Vec3V min = V3Min(v0, V3Min(v1, v2));
//	const Vec3V max = V3Max(v0, V3Max(v1, v2));
//	const BoolV c = BAnyTrue3(BOr(V3IsGrtr(min, extents), V3IsGrtr(nExtents, max)));
//	if(BAllEq(c, BTTTT()))
//		return false;
//
//	//triangle face normal
//	const Vec3V normal = V3Cross(f0, f1);
//	const FloatV d = FNeg(V3Dot(normal, v0));
//	const BoolV fc = V3IsGrtr(normal, zero);
//	const Vec3V pmin = V3Sel(fc, nExtents, extents );
//	const Vec3V pmax = V3Sel(fc, extents,  nExtents);
//	const FloatV minsigndist = FAdd(V3Dot(normal, pmin), d);
//	const FloatV maxsigndist = FAdd(V3Dot(normal, pmax), d);
//	const FloatV sign = FMul(minsigndist, maxsigndist);
//
//	return (BAllEq(FIsGrtr(zero, sign), BFFFF()) == 0);
//
//}

bool Gu::intersectTriangleBox(const Ps::aos::Vec3VArg boxCenter, const Ps::aos::Vec3VArg extents, const Ps::aos::Vec3VArg tp0, const Ps::aos::Vec3V& tp1, const Ps::aos::Vec3VArg tp2)
{
	using namespace Ps::aos;
	const FloatV zero = FZero();
	const Vec3V nExtents = V3Neg(extents);
	// This is the fastest branch on Sun - move everything so that the boxcenter is in (0,0,0)
	const Vec3V v0 = V3Sub(tp0, boxCenter);
	const Vec3V v1 = V3Sub(tp1, boxCenter);
	const Vec3V v2 = V3Sub(tp2, boxCenter);

	// compute triangle edges
	const Vec3V f0 = V3Sub(v1, v0);	// tri edge 0
	const Vec3V f1 = V3Sub(v2, v1);	// tri edge 1
	const Vec3V f2 = V3Sub(v0, v2);	// tri edge 2

	const FloatV x0 = V3GetX(f0);
	const FloatV y0 = V3GetY(f0);
	const FloatV z0 = V3GetZ(f0);

	const FloatV x1 = V3GetX(f1);
	const FloatV y1 = V3GetY(f1);
	const FloatV z1 = V3GetZ(f1);

	const FloatV x2 = V3GetX(f2);
	const FloatV y2 = V3GetY(f2);
	const FloatV z2 = V3GetZ(f2);
	//9 separating axis
	const Vec3V a00 = V3Merge(zero, FNeg(z0), y0);
	const Vec3V a01 = V3Merge(zero, FNeg(z1), y1);
	const Vec3V a02 = V3Merge(zero, FNeg(z2), y2);

	const Vec3V a10 = V3Merge(z0, zero, FNeg(x0));
	const Vec3V a11 = V3Merge(z1, zero, FNeg(x1));
	const Vec3V a12 = V3Merge(z2, zero, FNeg(x2));

	const Vec3V a20 = V3Merge(FNeg(y0), x0, zero);
	const Vec3V a21 = V3Merge(FNeg(y1), x1, zero);
	const Vec3V a22 = V3Merge(FNeg(y2), x2, zero);

	//Axis a00
	//triangle projection
	const FloatV p000 = V3Dot(a00, v0); // p001 == p000
	const FloatV p002 = V3Dot(a00, v2);
	const FloatV r00 = V3Dot(V3Abs(a00), extents);//box projection

	//Axis a01
	const FloatV p010 = V3Dot(a01, v0);
	const FloatV p011 = V3Dot(a01, v1);//p011 == p012
	const FloatV r01 = V3Dot(V3Abs(a01), extents);//box projection

	//Axis a02
	const FloatV p020 = V3Dot(a02, v0);//p022 == p020
	const FloatV p021 = V3Dot(a02, v1);
	const FloatV r02 = V3Dot(V3Abs(a02), extents);//box projection

	const Vec3V pp0 = V3Merge(p000, p010, p020);
	const Vec3V pp1 = V3Merge(p002, p011, p021);
	const Vec3V rr0 = V3Merge(r00, r01, r02);
	const Vec3V nrr0 = V3Neg(rr0);

	const Vec3V pmin0 = V3Min(pp0, pp1);
	const Vec3V pmax0 = V3Max(pp0, pp1);

	const BoolV con0 = BAnyTrue3(BOr(V3IsGrtr(pmin0, rr0), V3IsGrtr(nrr0, pmax0)));
	if(BAllEq(con0, BTTTT()))
		return false;


	//Axis a10
	//triangle projection
	const FloatV p100 = V3Dot(a10, v0); // p100 == p102
	const FloatV p102 = V3Dot(a10, v2);
	const FloatV r10 = V3Dot(V3Abs(a10), extents);//box projection

	//Axis a11
	//triangle projection
	const FloatV p110 = V3Dot(a11, v0); 
	const FloatV p111 = V3Dot(a11, v1);// p111 == p112
	const FloatV r11 = V3Dot(V3Abs(a11), extents);	//box projection

	//Axis a12
	//triangle projection
	const FloatV p120 = V3Dot(a12, v0); // p120 == p122
	const FloatV p121 = V3Dot(a12, v1);
	const FloatV r12 = V3Dot(V3Abs(a12), extents);//box projection

	const Vec3V pp10 = V3Merge(p100, p110, p120);
	const Vec3V pp11 = V3Merge(p102, p111, p121);
	const Vec3V rr1 = V3Merge(r10, r11, r12);
	const Vec3V nrr1 = V3Neg(rr1);
 
	const Vec3V pmin1 = V3Min(pp10, pp11);
	const Vec3V pmax1 = V3Max(pp10, pp11);

	const BoolV con1 = BAnyTrue3(BOr(V3IsGrtr(pmin1, rr1), V3IsGrtr(nrr1, pmax1)));
	if(BAllEq(con1, BTTTT()))
		return false;

	//Axis a20
	//triangle projection
	const FloatV p200 = V3Dot(a20, v0); // p200 == p201
	const FloatV p202 = V3Dot(a20, v2);
	const FloatV r20 = V3Dot(V3Abs(a20), extents);//box projection

	//Axis a21
	//triangle projection
	const FloatV p210 = V3Dot(a21, v0); 
	const FloatV p211 = V3Dot(a21, v1);// p211 == p212
	const FloatV r21 = V3Dot(V3Abs(a21), extents);	//box projection

	//Axis a22
	//triangle projection
	const FloatV p220 = V3Dot(a22, v0); // p220 == p222
	const FloatV p221 = V3Dot(a22, v1);
	const FloatV r22 = V3Dot(V3Abs(a22), extents);//box projection

	const Vec3V pp20 = V3Merge(p200, p210, p220);
	const Vec3V pp21 = V3Merge(p202, p211, p221);
	const Vec3V rr2 = V3Merge(r20, r21, r22);
	const Vec3V nrr2 = V3Neg(rr2);
 
	const Vec3V pmin2 = V3Min(pp20, pp21);
	const Vec3V pmax2 = V3Max(pp20, pp21);

	const BoolV con2 = BAnyTrue3(BOr(V3IsGrtr(pmin2, rr2), V3IsGrtr(nrr2, pmax2)));
	if(BAllEq(con2, BTTTT()))
		return false;

	//test e0(1, 0, 0), e1(0,1,0), e2(0,0,1)
	const Vec3V min = V3Min(v0, V3Min(v1, v2));
	const Vec3V max = V3Max(v0, V3Max(v1, v2));
	const BoolV c = BAnyTrue3(BOr(V3IsGrtr(min, extents), V3IsGrtr(nExtents, max)));

	//triangle face normal
	const Vec3V normal = V3Cross(f0, f1);
	const FloatV d = FNeg(V3Dot(normal, v0));
	const BoolV fc = V3IsGrtr(normal, V3Zero());
	const Vec3V pmin = V3Sel(fc, nExtents, extents );
	const Vec3V pmax = V3Sel(fc, extents,  nExtents);
	const FloatV minsigndist = FAdd(V3Dot(normal, pmin), d);
	const FloatV maxsigndist = FAdd(V3Dot(normal, pmax), d);
	const FloatV sign = FMul(minsigndist, maxsigndist);

	const BoolV bNotIntersect = BOr(c, FIsGrtr(sign, zero));

	return BAllEq(bNotIntersect, BTTTT()) == 0;


}

namespace physx { namespace Gu {

OBBTriangleTest::OBBTriangleTest(const Gu::Box& box)
{
	mBoxExtents				= box.extents;

	Cm::Matrix34 WorldB;
	WorldB.base0 = box.rot.column0;
	WorldB.base1 = box.rot.column1;
	WorldB.base2 = box.rot.column2;
	WorldB.base3 = box.center;

	Cm::Matrix34 ModelToBox	= WorldB.getInverseRT();

	mRModelToBox.column0	= ModelToBox.base0;
	mRModelToBox.column1	= ModelToBox.base1;
	mRModelToBox.column2	= ModelToBox.base2;
	mTModelToBox			= ModelToBox.base3;

	mRBoxToModel.column0	= WorldB.base0;
	mRBoxToModel.column1	= WorldB.base1;
	mRBoxToModel.column2	= WorldB.base2;
	mTBoxToModel			= WorldB.base3;
}

Ps::IntBool OBBTriangleTest::obbTriTest(const PxVec3& vert0, const PxVec3& vert1, const PxVec3& vert2) const
{
	using namespace Ps::aos;
	Vec3V rot_0 = V3LoadU(mRModelToBox[0]);
	Vec3V rot_1 = V3LoadU(mRModelToBox[1]);
	Vec3V rot_2 = V3LoadU(mRModelToBox[2]);

	Vec3V tran = V3LoadU(mTModelToBox);
	//AP: needed on SPU to fix a precision failure in --gtest_filter=SqTestSweep.verify_Big_QueryShape_Small_SceneShape
	Vec3V extents = PX_IS_SPU ? V3LoadU(mBoxExtents+PxVec3(1e-6f)) : V3LoadU(mBoxExtents);

	Mat33V rot(rot_0, rot_1, rot_2);
	BoolV ffff = BFFFF();

	//transform obb to origin
	Vec3V v0 = V3LoadU(vert0);
	Vec3V v1 = V3LoadU(vert1);
	Vec3V v2 = V3LoadU(vert2);

	//Note this transform is different to the LSS loose version.
	v0 = M33MulV3AddV3(rot, v0, tran);
	v1 = M33MulV3AddV3(rot, v1, tran);
	v2 = M33MulV3AddV3(rot, v2, tran);

	Vec3V minusExtents =  V3Neg(extents);

	// Test triangle AABB
	Vec3V triMin = V3Min(v0, V3Min(v1, v2));
	Vec3V triMax = V3Max(v0, V3Max(v1, v2));

	BoolV mask = BOr(V3IsGrtr(triMin, extents), V3IsGrtr(minusExtents, triMax));
	if(!BAllEq(mask, ffff))
		return Ps::IntFalse;

	// test tri plane.
	Vec3V edge0 = V3Sub(v1, v0);
	Vec3V edge2 = V3Sub(v0, v2);

	Vec3V normal = V3Cross(edge2, edge0);
	FloatV dist = V3Dot(normal, v0);
	FloatV maxDist = V3Dot(extents, V3Abs(normal));

	if(FOutOfBounds(dist, maxDist))
		return Ps::IntFalse;


	//////// test edge axis
	Vec3V radius = V3Add(extents, extents);

	Vec3V axis;
	FloatV p0, p1, p2, pMin, pMax, axisRadius;

	/*
		y*other.z - z*other.y,
		z*other.x - x*other.z,
		x*other.y - y*other.x

		0 - 0
		0 - e0.z
		e0.y - 0
	*/


	// axis == [1,0,0] x e0 == [0, -e0.z, e0.y]
	// x, y, z, w,    x, y, z, w
	// 0, 1, 2, 3,    4, 5, 6, 7

	Vec3V minusEdge0 = V3Neg(edge0);

	axis = V3Perm_Zero_1Z_0Y(edge0, minusEdge0);
	p0 = V3Dot(v0, axis);
	p2 = V3Dot(v2, axis);
	axisRadius = V3Dot(radius, V3Abs(axis));

	pMin = FAdd(p0, p2);
	pMax = FAdd(FAbs(FSub(p0, p2)), axisRadius);
	
	if(FOutOfBounds(pMin, pMax))
		return Ps::IntFalse;
	

	Vec3V edge1 = V3Sub(v2, v1);
	Vec3V minusEdge1 = V3Neg(edge1);
	
	
	// axis == [1,0,0] x e1 == [0, -e1.z, e1.y]
	axis = V3Perm_Zero_1Z_0Y(edge1, minusEdge1);
	p0 = V3Dot(v0, axis);
	p1 = V3Dot(v1, axis);
	axisRadius = V3Dot(radius, V3Abs(axis));
	pMin = FAdd(p0, p1);
	pMax = FAdd(FAbs(FSub(p0, p1)), axisRadius);
	
	if(FOutOfBounds(pMin, pMax))
		return Ps::IntFalse;
	

	Vec3V minusEdge2 = V3Neg(edge2);

	
	// axis == [1,0,0] x e2 == [0, -e2.z, e2.y]
	axis = V3Perm_Zero_1Z_0Y(edge2, minusEdge2);
	p0 = V3Dot(v0, axis);
	p1 = V3Dot(v1, axis);
	axisRadius = V3Dot(radius, V3Abs(axis));

	pMin = FAdd(p0, p1);
	pMax = FAdd(FAbs(FSub(p0, p1)), axisRadius);

	if(FOutOfBounds(pMin, pMax))
		return Ps::IntFalse;
	

	/*
		y*other.z - z*other.y,
		z*other.x - x*other.z,
		x*other.y - y*other.x

		e0.z - 0
		0 - 0
		0 - e0.x
	*/
	// axis == [0,1,0] x e0 == [e0.z, 0, -e0.x]
	// x, y, z, w,    x, y, z, w
	// 0, 1, 2, 3,    4, 5, 6, 7

	axis = V3Perm_0Z_Zero_1X(edge0, minusEdge0);
	p0 = V3Dot(v0, axis);
	p2 = V3Dot(v2, axis);
	axisRadius = V3Dot(radius, V3Abs(axis));
	pMin = FAdd(p0, p2);
	pMax = FAdd(FAbs(FSub(p0, p2)), axisRadius);

	if(FOutOfBounds(pMin, pMax))
		return Ps::IntFalse;

	// axis == [0,1,0] x e1 == [e1.z, 0, -e1.x]
	axis = V3Perm_0Z_Zero_1X(edge1, minusEdge1);
	p0 = V3Dot(v0, axis);
	p1 = V3Dot(v1, axis);
	axisRadius = V3Dot(radius, V3Abs(axis));

	pMin = FAdd(p0, p1);
	pMax = FAdd(FAbs(FSub(p0, p1)), axisRadius);

	if(FOutOfBounds(pMin, pMax))
		return Ps::IntFalse;

	// axis == [0, 1, 0] x e2 == [e2.z, 0, -e2.x]
	axis = V3Perm_0Z_Zero_1X(edge2, minusEdge2);
	p0 = V3Dot(v0, axis);
	p1 = V3Dot(v1, axis);
	axisRadius = V3Dot(radius, V3Abs(axis));

	pMin = FAdd(p0, p1);
	pMax = FAdd(FAbs(FSub(p0, p1)), axisRadius);

	if(FOutOfBounds(pMin, pMax))
		return Ps::IntFalse;


	/*
		y*other.z - z*other.y,
		z*other.x - x*other.z,
		x*other.y - y*other.x

		0 - e0.y
		e0.x - 0
		0 - 0
	*/

	// axis == [0, 0, 1] x e0 == [-e0.y, e0.x, 0]
	// x, y, z, w,    x, y, z, w
	// 0, 1, 2, 3,    4, 5, 6, 7

	axis = V3Perm_1Y_0X_Zero(edge0, minusEdge0);
	p0 = V3Dot(v0, axis);
	p2 = V3Dot(v2, axis);
	axisRadius = V3Dot(radius, V3Abs(axis));

	pMin = FAdd(p0, p2);
	pMax = FAdd(FAbs(FSub(p0, p2)), axisRadius);

	if(FOutOfBounds(pMin, pMax))
		return Ps::IntFalse;


	// axis == [0, 0, 1] x e1 == [-e1.y, e1.x, 0]
	axis = V3Perm_1Y_0X_Zero(edge1, minusEdge1);
	p0 = V3Dot(v0, axis);
	p1 = V3Dot(v1, axis);
	axisRadius = V3Dot(radius, V3Abs(axis));

	pMin = FAdd(p0, p1);
	pMax = FAdd(FAbs(FSub(p0,p1)), axisRadius);

	if(FOutOfBounds(pMin, pMax))
		return Ps::IntFalse;

	// axis == [0, 0, 1] x e2 == [-e2.y, e2.x, 0]
	axis = V3Perm_1Y_0X_Zero(edge2, minusEdge2);
	p0 = V3Dot(v0, axis);
	p1 = V3Dot(v1, axis);
	axisRadius = V3Dot(radius, V3Abs(axis));

	pMin = FAdd(p0, p1);
	pMax = FAdd(FAbs(FSub(p0, p1)), axisRadius);

	if(FOutOfBounds(pMin, pMax))
		return Ps::IntFalse;

	return Ps::IntTrue;
}

} } // namespace physx, Gu
