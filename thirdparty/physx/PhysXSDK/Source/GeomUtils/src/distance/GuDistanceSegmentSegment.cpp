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

#include "GuDistanceSegmentSegment.h"
#include "GuDistanceSegmentSegmentSIMD.h"

using namespace physx;

// ptchernev: 
// The Magic Software code uses a relative error test for parallel case.
// The Novodex code does not presumably as an optimization. 
// Since the Novodex code is working in the trunk I see no reason 
// to reintroduce the relative error test here.

// PT: this might just be because the relative error test has been added
// after we grabbed the code. I don't remember making this change. A good
// idea would be NOT to refactor Magic's code, to easily grab updated
// versions from the website.............................................

//Uses an old Wild Magic function, which suffers from some problems addressed 
//in the new version, see TTP 4617. However, some of the colliders seems to work 
//badly with the new version (e.g. Capsule/Mesh), so we keep the old one around 
//for a while.
PxReal Gu::distanceSegmentSegmentSquaredOLD(const PxVec3& origin0, const PxVec3& extent0,
											const PxVec3& origin1, const PxVec3& extent1,
											PxReal* s, PxReal* t)
{
	// PT: "OLD" function re-routed to new one in 3.3. Will be cleaned up/refactored in 3.4.
	return distanceSegmentSegmentSquared2(	Gu::Segment(origin0, origin0+extent0),
											Gu::Segment(origin1, origin1+extent1),
											s, t);
}

static const float ZERO_TOLERANCE = 1e-06f;

// S0 = origin + extent * dir;
// S1 = origin - extent * dir;
static PxReal distanceSegmentSegmentSquaredNEW(const PxVec3& origin0,
										 const PxVec3& dir0,
										 PxReal extent0,

										 const PxVec3& origin1,
										 const PxVec3& dir1,
										 PxReal extent1,

										 PxReal* param0, 
										 PxReal* param1)
{
    const PxVec3 kDiff	= origin0 - origin1;
    const PxReal fA01	= -dir0.dot(dir1);
    const PxReal fB0	= kDiff.dot(dir0);
    const PxReal fB1	= -kDiff.dot(dir1);
	const PxReal fC		= kDiff.magnitudeSquared();
	const PxReal fDet	= PxAbs(1.0f - fA01*fA01);
    PxReal fS0, fS1, fSqrDist, fExtDet0, fExtDet1, fTmpS0, fTmpS1;

    if (fDet >= ZERO_TOLERANCE)
    {
        // segments are not parallel
        fS0 = fA01*fB1-fB0;
        fS1 = fA01*fB0-fB1;
        fExtDet0 = extent0*fDet;
        fExtDet1 = extent1*fDet;

        if (fS0 >= -fExtDet0)
        {
            if (fS0 <= fExtDet0)
            {
                if (fS1 >= -fExtDet1)
                {
                    if (fS1 <= fExtDet1)  // region 0 (interior)
                    {
                        // minimum at two interior points of 3D lines
                        PxReal fInvDet = 1.0f/fDet;
                        fS0 *= fInvDet;
                        fS1 *= fInvDet;
                        fSqrDist = fS0*(fS0+fA01*fS1+2.0f*fB0) +
                            fS1*(fA01*fS0+fS1+2.0f*fB1)+fC;
                    }
                    else  // region 3 (side)
                    {
                        fS1 = extent1;
                        fTmpS0 = -(fA01*fS1+fB0);
                        if (fTmpS0 < -extent0)
                        {
                            fS0 = -extent0;
                            fSqrDist = fS0*(fS0-2.0f*fTmpS0) +
                                fS1*(fS1+2.0f*fB1)+fC;
                        }
                        else if (fTmpS0 <= extent0)
                        {
                            fS0 = fTmpS0;
                            fSqrDist = -fS0*fS0+fS1*(fS1+2.0f*fB1)+fC;
                        }
                        else
                        {
                            fS0 = extent0;
                            fSqrDist = fS0*(fS0-2.0f*fTmpS0) +
                                fS1*(fS1+2.0f*fB1)+fC;
                        }
                    }
                }
                else  // region 7 (side)
                {
                    fS1 = -extent1;
                    fTmpS0 = -(fA01*fS1+fB0);
                    if (fTmpS0 < -extent0)
                    {
                        fS0 = -extent0;
                        fSqrDist = fS0*(fS0-2.0f*fTmpS0) +
                            fS1*(fS1+2.0f*fB1)+fC;
                    }
                    else if (fTmpS0 <= extent0)
                    {
                        fS0 = fTmpS0;
                        fSqrDist = -fS0*fS0+fS1*(fS1+2.0f*fB1)+fC;
                    }
                    else
                    {
                        fS0 = extent0;
                        fSqrDist = fS0*(fS0-2.0f*fTmpS0) +
                            fS1*(fS1+2.0f*fB1)+fC;
                    }
                }
            }
            else
            {
                if (fS1 >= -fExtDet1)
                {
                    if (fS1 <= fExtDet1)  // region 1 (side)
                    {
                        fS0 = extent0;
                        fTmpS1 = -(fA01*fS0+fB1);
                        if (fTmpS1 < -extent1)
                        {
                            fS1 = -extent1;
                            fSqrDist = fS1*(fS1-2.0f*fTmpS1) +
                                fS0*(fS0+2.0f*fB0)+fC;
                        }
                        else if (fTmpS1 <= extent1)
                        {
                            fS1 = fTmpS1;
                            fSqrDist = -fS1*fS1+fS0*(fS0+2.0f*fB0)+fC;
                        }
                        else
                        {
                            fS1 = extent1;
                            fSqrDist = fS1*(fS1-2.0f*fTmpS1) +
                                fS0*(fS0+2.0f*fB0)+fC;
                        }
                    }
                    else  // region 2 (corner)
                    {
                        fS1 = extent1;
                        fTmpS0 = -(fA01*fS1+fB0);
                        if (fTmpS0 < -extent0)
                        {
                            fS0 = -extent0;
                            fSqrDist = fS0*(fS0-2.0f*fTmpS0) +
                                fS1*(fS1+2.0f*fB1)+fC;
                        }
                        else if (fTmpS0 <= extent0)
                        {
                            fS0 = fTmpS0;
                            fSqrDist = -fS0*fS0+fS1*(fS1+2.0f*fB1)+fC;
                        }
                        else
                        {
                            fS0 = extent0;
                            fTmpS1 = -(fA01*fS0+fB1);
                            if (fTmpS1 < -extent1)
                            {
                                fS1 = -extent1;
                                fSqrDist = fS1*(fS1-2.0f*fTmpS1) +
                                    fS0*(fS0+2.0f*fB0)+fC;
                            }
                            else if (fTmpS1 <= extent1)
                            {
                                fS1 = fTmpS1;
                                fSqrDist = -fS1*fS1+fS0*(fS0+2.0f*fB0)
                                    + fC;
                            }
                            else
                            {
                                fS1 = extent1;
                                fSqrDist = fS1*(fS1-2.0f*fTmpS1) +
                                    fS0*(fS0+2.0f*fB0)+fC;
                            }
                        }
                    }
                }
                else  // region 8 (corner)
                {
                    fS1 = -extent1;
                    fTmpS0 = -(fA01*fS1+fB0);
                    if (fTmpS0 < -extent0)
                    {
                        fS0 = -extent0;
                        fSqrDist = fS0*(fS0-2.0f*fTmpS0) +
                            fS1*(fS1+2.0f*fB1)+fC;
                    }
                    else if (fTmpS0 <= extent0)
                    {
                        fS0 = fTmpS0;
                        fSqrDist = -fS0*fS0+fS1*(fS1+2.0f*fB1)+fC;
                    }
                    else
                    {
                        fS0 = extent0;
                        fTmpS1 = -(fA01*fS0+fB1);
                        if (fTmpS1 > extent1)
                        {
                            fS1 = extent1;
                            fSqrDist = fS1*(fS1-2.0f*fTmpS1) +
                                fS0*(fS0+2.0f*fB0)+fC;
                        }
                        else if (fTmpS1 >= -extent1)
                        {
                            fS1 = fTmpS1;
                            fSqrDist = -fS1*fS1+fS0*(fS0+2.0f*fB0)
                                + fC;
                        }
                        else
                        {
                            fS1 = -extent1;
                            fSqrDist = fS1*(fS1-2.0f*fTmpS1) +
                                fS0*(fS0+2.0f*fB0)+fC;
                        }
                    }
                }
            }
        }
        else 
        {
            if (fS1 >= -fExtDet1)
            {
                if (fS1 <= fExtDet1)  // region 5 (side)
                {
                    fS0 = -extent0;
                    fTmpS1 = -(fA01*fS0+fB1);
                    if (fTmpS1 < -extent1)
                    {
                        fS1 = -extent1;
                        fSqrDist = fS1*(fS1-2.0f*fTmpS1) +
                            fS0*(fS0+2.0f*fB0)+fC;
                    }
                    else if (fTmpS1 <= extent1)
                    {
                        fS1 = fTmpS1;
                        fSqrDist = -fS1*fS1+fS0*(fS0+2.0f*fB0)+fC;
                    }
                    else
                    {
                        fS1 = extent1;
                        fSqrDist = fS1*(fS1-2.0f*fTmpS1) +
                            fS0*(fS0+2.0f*fB0)+fC;
                    }
                }
                else  // region 4 (corner)
                {
                    fS1 = extent1;
                    fTmpS0 = -(fA01*fS1+fB0);
                    if (fTmpS0 > extent0)
                    {
                        fS0 = extent0;
                        fSqrDist = fS0*(fS0-2.0f*fTmpS0) +
                            fS1*(fS1+2.0f*fB1)+fC;
                    }
                    else if (fTmpS0 >= -extent0)
                    {
                        fS0 = fTmpS0;
                        fSqrDist = -fS0*fS0+fS1*(fS1+2.0f*fB1)+fC;
                    }
                    else
                    {
                        fS0 = -extent0;
                        fTmpS1 = -(fA01*fS0+fB1);
                        if (fTmpS1 < -extent1)
                        {
                            fS1 = -extent1;
                            fSqrDist = fS1*(fS1-2.0f*fTmpS1) +
                                fS0*(fS0+2.0f*fB0)+fC;
                        }
                        else if (fTmpS1 <= extent1)
                        {
                            fS1 = fTmpS1;
                            fSqrDist = -fS1*fS1+fS0*(fS0+2.0f*fB0)
                                + fC;
                        }
                        else
                        {
                            fS1 = extent1;
                            fSqrDist = fS1*(fS1-2.0f*fTmpS1) +
                                fS0*(fS0+2.0f*fB0)+fC;
                        }
                    }
                }
            }
            else   // region 6 (corner)
            {
                fS1 = -extent1;
                fTmpS0 = -(fA01*fS1+fB0);
                if (fTmpS0 > extent0)
                {
                    fS0 = extent0;
                    fSqrDist = fS0*(fS0-2.0f*fTmpS0) +
                        fS1*(fS1+2.0f*fB1)+fC;
                }
                else if (fTmpS0 >= -extent0)
                {
                    fS0 = fTmpS0;
                    fSqrDist = -fS0*fS0+fS1*(fS1+2.0f*fB1)+fC;
                }
                else
                {
                    fS0 = -extent0;
                    fTmpS1 = -(fA01*fS0+fB1);
                    if (fTmpS1 < -extent1)
                    {
                        fS1 = -extent1;
                        fSqrDist = fS1*(fS1-2.0f*fTmpS1) +
                            fS0*(fS0+2.0f*fB0)+fC;
                    }
                    else if (fTmpS1 <= extent1)
                    {
                        fS1 = fTmpS1;
                        fSqrDist = -fS1*fS1+fS0*(fS0+2.0f*fB0)
                            + fC;
                    }
                    else
                    {
                        fS1 = extent1;
                        fSqrDist = fS1*(fS1-2.0f*fTmpS1) +
                            fS0*(fS0+2.0f*fB0)+fC;
                    }
                }
            }
        }
    }
    else
    {
		// The segments are parallel.
		PxReal fE0pE1 = extent0 + extent1;
		PxReal fSign = (fA01 > 0.0f ? -1.0f : 1.0f);
		PxReal b0Avr = 0.5f*(fB0 - fSign*fB1);
		PxReal fLambda = -b0Avr;
		if(fLambda < -fE0pE1)
		{
			fLambda = -fE0pE1;
		}
		else if(fLambda > fE0pE1)
		{
			fLambda = fE0pE1;
		}

		fS1 = -fSign*fLambda*extent1/fE0pE1;
		fS0 = fLambda + fSign*fS1;
		fSqrDist = fLambda*(fLambda + 2.0f*b0Avr) + fC;
    }

	if(param0)
		*param0 = fS0;
	if(param1)
		*param1 = fS1;

	// account for numerical round-off error
	return physx::intrinsics::selectMax(0.0f, fSqrDist);
}

    // The segment is represented as P+t*D, where P is the segment origin,
    // D is a unit-length direction vector and |t| <= e.  The value e is
    // referred to as the extent of the segment.  The end points of the
    // segment are P-e*D and P+e*D.  The user must ensure that the direction
    // vector is unit-length.  The representation for a segment is analogous
    // to that for an oriented bounding box.  P is the center, D is the
    // axis direction, and e is the extent.

/*    // construction
    Segment3 ();  // uninitialized
    Segment3 (const Vector3<PxReal>& rkOrigin, const Vector3<PxReal>& rkDirection,
        PxReal fExtent);

    // end points
    Vector3<PxReal> GetPosEnd () const;  // P+e*D
    Vector3<PxReal> GetNegEnd () const;  // P-e*D

    Vector3<PxReal> Origin, Direction;
    PxReal Extent;*/

/*
static PX_FORCE_INLINE PxReal convertSegment(PxVec3& center, PxVec3& extent, const Gu::Segment& segment)
{
	extent = segment.computeDirection();
	center = segment.p0 + extent*0.5f;
	const PxReal length = extent.magnitude();	//AM: change to make it work for degenerate (zero length) segments.
	if(length != 0.0f)
	{
		extent /= length;
		length *= 0.5f;
	}
	return length;
}*/

PxReal Gu::distanceSegmentSegmentSquared2(	const Gu::Segment& segment0,
											const Gu::Segment& segment1,
											PxReal* param0, 
											PxReal* param1)
{
	// Some conversion is needed between the old & new code
	// Old:
	// segment (s0, s1)
	// origin = s0
	// extent = s1 - s0
	//
	// New:
	// s0 = origin + extent * dir;
	// s1 = origin - extent * dir;

	// dsequeira: is this really sensible? We use a highly optimized Wild Magic routine, 
	// then use a segment representation that requires an expensive conversion to/from...

	PxVec3 extent0 = segment0.computeDirection();
	const PxVec3 center0 = segment0.p0 + extent0*0.5f;
	PxReal length0 = extent0.magnitude();	//AM: change to make it work for degenerate (zero length) segments.
	const bool b0 = length0 != 0.0f;
	PxReal oneOverLength0(0.0f);
	if(b0)
	{
		oneOverLength0 = 1.0f / length0;
		extent0 *= oneOverLength0;
		length0 *= 0.5f;
	}

	PxVec3 extent1 = segment1.computeDirection();
	const PxVec3 center1 = segment1.p0 + extent1*0.5f;
	PxReal length1 = extent1.magnitude();
	const bool b1 = length1 != 0.0f;
	PxReal oneOverLength1(0.0f);
	if(b1)
	{
		oneOverLength1 = 1.0f / length1;
		extent1 *= oneOverLength1;
		length1 *= 0.5f;
	}
/*
	PxVec3 center0, extent0;
	const PxReal length0 = convertSegment(center0, extent0, segment0);

	PxVec3 center1, extent1;
	const PxReal length1 = convertSegment(center1, extent1, segment1);
*/
	// the return param vals have -extent = s0, extent = s1

	const PxReal returnValue = distanceSegmentSegmentSquaredNEW(	center0, extent0, length0,
																	center1, extent1, length1,
																	param0, param1);

	//ML : This is wrong for some reason, I guess it has precision issue
	//// renormalize into the 0 = s0, 1 = s1 range
	//if (param0)
	//	*param0 = b0 ? ((*param0) * oneOverLength0 * 0.5f + 0.5f) : 0.0f;
	//if (param1)
	//	*param1 = b1 ? ((*param1) * oneOverLength1 * 0.5f + 0.5f) : 0.0f;

	if(param0)
		*param0 = b0 ? ((length0 + (*param0))*oneOverLength0) : 0.0f;
	if(param1)
		*param1 = b1 ? ((length1 + (*param1))*oneOverLength1) : 0.0f;

	return returnValue;
}
//
///*
//	S0 = origin + extent * dir;
//	S1 = origin + extent * dir;
//	dir is the vector from start to end point
//	p1 is the start point of segment1
//	q1 is the end point of segment1
//	p2 is the start point of segment2
//	q2 is the end point of segment2
//*/
//
//Ps::aos::FloatV Gu::distanceSegmentSegmentSquared(const Ps::aos::Vec3VArg p1, 
//														const Ps::aos::Vec3VArg q1,
//														const Ps::aos::Vec3VArg p2, 
//														const Ps::aos::Vec3VArg q2,
//														Ps::aos::FloatV& s, 
//														Ps::aos::FloatV& t)
//{
//	using namespace Ps::aos;
//	const FloatV zero = FZero();
//	const FloatV one = FOne();
//	const FloatV eps = FEps();
//	const FloatV parallelTolerance  = FloatV_From_F32(PX_PARALLEL_TOLERANCE);
//
//	const Vec3V d1 = V3Sub(q1, p1); //direction vector of segment1
//	const Vec3V d2 = V3Sub(q2, p2); //direction vector of segment2
//	const Vec3V r = V3Sub(p1, p2);
//	const FloatV a = V3Dot(d1, d1);//squared length of segment1
//	const FloatV e = V3Dot(d2, d2);//squared length of segment2
//	const FloatV aRecip = FRecip(a);
//	const FloatV eRecip = FRecip(e);
//	const FloatV f = V3Dot(d2, r);
//	const FloatV c = V3Dot(d1, r);
//	const FloatV nc = FNeg(c);
//	
//
//	//check if either or both segments degenerate into points
//	const BoolV con0 = FIsGrtr(eps, a);
//	const BoolV con1 = FIsGrtr(eps, e);
//
//	PxU32 v0, v1; 
//	Store_From_BoolV(con0, &v0);
//	Store_From_BoolV(con1, &v1);
//
//	if(v0&v1)
//	{
//		s = zero;
//		t = zero;
//		return V3Dot(r, r);
//	}
//	
//	if(v0)
//	{
//		//First segment degenerates into a point
//		s = zero;
//		const FloatV temp = FMul(f, eRecip);
//		t= FClamp(temp, zero, one);
//	}
//	else
//	{
//		const FloatV d = FMul(nc, aRecip);
//		if(v1)//second segment degenerates into a point
//		{
//			s = zero;
//			t = FClamp(d, zero, one);
//		}
//		else
//		{
//			//the general nondegenerate case
//			const FloatV b = V3Dot(d1, d2);
//			const FloatV denom = FSub(FMul(a, e), FMul(b, b));
//			const FloatV denomRecip = FDiv(one, denom);
//			const FloatV temp = FSub(FMul(b, f), FMul(c, e));
//			const FloatV s0 = FMul(temp, denomRecip);
//			
//			//if segments not parallell, compute closest point on two segments and clamp to segment1, else pick arbitrary param0
//			const BoolV con2 = FIsGrtrOrEq( parallelTolerance, FAbs(denom));
//			PxU32 v2; 
//			Store_From_BoolV(con2, &v2);
//			if(v2)
//			{
//				//segment is parallel
//				
//				const FloatV a01 = FNeg(V3Dot(d1, d2));
//				const FloatV a01Recip = FRecip(a01);
//
//				const FloatV tmp0 = FNeg(FAdd(a, c));
//				const FloatV tmp1 = FMul(nc, a);
//				const FloatV tmp2 = FMul(nc, aRecip);
//				const FloatV tmp3 = FMul(tmp0, a01Recip);
//				const FloatV tmp4 = FMul(nc, a01Recip);
//
//				const BoolV con00 = FIsGrtr(a01, zero);
//				PxU32 v00; 
//				Store_From_BoolV(con00, &v00);
//
//				if ( v00)
//				{
//					const BoolV con01 = FIsGrtrOrEq(c, zero);
//					const BoolV con02 = FIsGrtr(a, nc);
//					const BoolV con3 = BOr(con01, con02);
//					//const FloatV tmp0 = FNeg(FAdd(a, c));
//					s = FSel(con01, zero, FSel(con02,tmp1, one));
//					//t = FSel(con01, zero, FSel(con02, zero, FSel(FIsGrtrOrEq(tmp0, a01), one, FDiv(tmp0, a01))));
//					t = FSel(con3, zero, FSel(FIsGrtrOrEq(tmp0, a01), one, tmp3));
//
//				}
//				else
//				{
//					const BoolV con03 = FIsGrtrOrEq(nc, a);
//					const BoolV con04 = FIsGrtr(zero, c);
//					const BoolV con5 = BOr(con03, con04);
//
//					s = FSel(con03, one, FSel(con04, tmp2, zero));
//					//t = FSel(con03, zero, FSel(con04, zero, FSel(FIsGrtrOrEq(c, FNeg(a01)), one,  FDiv(nc, a01)) ));
//					t = FSel(con5, zero, FSel(FIsGrtrOrEq(c, FNeg(a01)), one,  tmp4));
//				}
//
//			}
//			else
//			{
//
//				s = FSel(con2, zero, FClamp(s0, zero, one));
//			
//				//compute point on segment2 closest to segment1
//				t = FMul(FAdd(FMul(b, s), f), eRecip);
//
//				//if param1 is in [zero, one], done. otherwise clamp param1 and recompute param0 for the new value
//				const BoolV con3 = FIsGrtrOrEq(zero, t);
//				const BoolV con4 = FIsGrtr(t, one);
//
//				const FloatV par0 = FClamp(d, zero, one);
//				const FloatV par1 = FClamp(FMul(FSub(b, c), aRecip), zero, one);
//
//				t = FSel(con3, zero, FSel(con4, one,  t));
//				s = FSel(con3, par0, FSel(con4, par1, s));
//			}
//
//		}
//	}
//
//	const Vec3V closest1 = V3Add(p1, V3Scale(d1, s));
//	const Vec3V closest2 = V3Add(p2, V3Scale(d2, t));
//	const Vec3V vv = V3Sub(closest1, closest2);
//	return V3Dot(vv, vv);
//
//}



/*
	S0 = origin + extent * dir;
	S1 = origin + extent * dir;
	dir is the vector from start to end point
	p1 is the start point of segment1
	d1 is the direction vector(q1 - p1)
	p2 is the start point of segment2
	d2 is the direction vector(q2 - p2) 
*/

Ps::aos::FloatV Gu::distanceSegmentSegmentSquared(const Ps::aos::Vec3VArg p1, 
														const Ps::aos::Vec3VArg d1,
														const Ps::aos::Vec3VArg p2, 
														const Ps::aos::Vec3VArg d2,
														Ps::aos::FloatV& s, 
														Ps::aos::FloatV& t)
{
	using namespace Ps::aos;
	const FloatV zero = FZero();
	const FloatV one = FOne();
	const FloatV half = FLoad(0.5f);
	const FloatV eps = FEps();
	//const FloatV parallelTolerance  = FloatV_From_F32(PX_PARALLEL_TOLERANCE);

	//const Vec3V d1 = V3Sub(q1, p1); //direction vector of segment1
	//const Vec3V d2 = V3Sub(q2, p2); //direction vector of segment2
	const Vec3V r = V3Sub(p1, p2);
	//const FloatV a = V3Dot(d1, d1);//squared length of segment1
	//const FloatV e = V3Dot(d2, d2);//squared length of segment2
	//const FloatV b = V3Dot(d1, d2);
	//const FloatV c = V3Dot(d1, r);
	//const FloatV aRecip = FRecip(a);
	//const FloatV eRecip = FRecip(e);

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
	
	//if segments not parallell, compute closest point on two segments and clamp to segment1
	const FloatV denom = FSub(FMul(a, e), FMul(b, b));
	const FloatV temp = FSub(FMul(b, f), FMul(c, e));
	const FloatV s0 = FClamp(FDiv(temp, denom), zero, one);
	
	//if segment is parallel, demon < eps
	const BoolV con2 = FIsGrtr(eps, denom);//FIsEq(denom, zero);
	const FloatV sTmp = FSel(con2, half, s0);
	
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

	const Vec3V closest1 = V3ScaleAdd(d1, s2, p1);//V3Add(p1, V3Scale(d1, tempS));
	const Vec3V closest2 = V3ScaleAdd(d2, t2, p2);//V3Add(p2, V3Scale(d2, tempT));
	const Vec3V vv = V3Sub(closest1, closest2);
	return V3Dot(vv, vv);
}



/*
	segment pq and segment p02q02
	segment pq and segment p12q12
	segment pq and segment p22q22
	segment pq and segment p32q32
*/
//Ps::aos::Vec4V Gu::distanceSegmentSegmentSquared4(		const Ps::aos::Vec3VArg p, const Ps::aos::Vec3VArg d0, 
//														const Ps::aos::Vec3VArg p02, const Ps::aos::Vec3VArg d02, 
//                                                        const Ps::aos::Vec3VArg p12, const Ps::aos::Vec3VArg d12, 
//														const Ps::aos::Vec3VArg p22, const Ps::aos::Vec3VArg d22,
//                                                        const Ps::aos::Vec3VArg p32, const Ps::aos::Vec3VArg d32,
//                                                        Ps::aos::Vec4V& s, Ps::aos::Vec4V& t)
//{
//      using namespace Ps::aos;
//      const Vec4V zero = V4Zero();
//      const Vec4V one = V4One();
//      const Vec4V eps = V4Eps();
//      const Vec4V parallelTolerance  = Vec4V_From_F32(PX_PARALLEL_TOLERANCE);
//
//      const Vec3V r0 = V3Sub(p, p02);
//      const Vec3V r1 = V3Sub(p, p12);
//      const Vec3V r2 = V3Sub(p, p22);
//      const Vec3V r3 = V3Sub(p, p32);
//
//	  //TODO - store this in a transposed state and avoid so many dot products?
//
//	  const FloatV dd = V3Dot(d0, d0);
//
//
//      const FloatV _e[4] = { V3Dot(d02, d02), V3Dot(d12, d12), V3Dot(d22, d22), V3Dot(d32, d32) };
//
//      const FloatV _b[4] = { V3Dot(d0, d02), V3Dot(d0, d12), V3Dot(d0, d22), V3Dot(d0, d32) };
//      const FloatV _c[4] = { V3Dot(d0, r0), V3Dot(d0, r1), V3Dot(d0, r2), V3Dot(d0, r3) };
//      const FloatV _f[4] = { V3Dot(d02, r0), V3Dot(d12, r1), V3Dot(d22, r2), V3Dot(d32, r3) };
//
//
//      const Vec4V a(V4Splat(dd));
//      const Vec4V e(V4Merge(_e));
//
//      const Vec4V b(V4Merge(_b));
//      const Vec4V c(V4Merge(_c));
//
//	  /*const Vec4V b(M44MulV4(m0T, d0));
//	  const Vec4V c(M44MulV4(m1T, d0));*/
//
//      const Vec4V f(V4Merge(_f));
//
//      const Vec4V aRecip(V4Recip(a));
//      const Vec4V eRecip(V4Recip(e));
//
//      const Vec4V nc(V4Neg(c));
//      const Vec4V d(V4Mul(nc, aRecip));
//      const Vec4V cd(V4Clamp(d, zero, one));
//      
//      
//      const Vec4V g = V4Mul(f, eRecip);
//      const Vec4V cg = V4Clamp(g, zero, one); //when s = zero
//      const Vec4V h = V4Mul(nc, a);
//      
//
//      //check if either or both segments degenerate into points
//      const BoolV con0 = V4IsGrtr(eps, a);
//      const BoolV con1 = V4IsGrtr(eps, e);
//            
//      //the general nondegenerate case
//
//      //if segments not parallell, compute closest point on two segments and clamp to segment1
//      const Vec4V denom = V4Sub(V4Mul(a, e), V4Mul(b, b));
//      const Vec4V denomRecip = V4Recip(denom);
//      const Vec4V temp = V4Sub(V4Mul(b, f), V4Mul(c, e));
//      const Vec4V s0 = V4Clamp(V4Mul(temp, denomRecip), zero, one);
//      
//      //if segment is parallel
//      const BoolV con00 = V4IsGrtr(b, zero);//segment p1q1 and p2q2 point to the same direction
//      const BoolV con01 = V4IsGrtrOrEq(c, zero);
//      const BoolV con02 = V4IsGrtr(a, nc);
//
//      const Vec4V s01 = V4Sel(con01, zero, V4Sel(con02, h, one));
//      const Vec4V s02 = V4Sel(con02, V4Sel(con01, zero, d), one);
//      const Vec4V s1 = V4Sel(con00, s02, s01);
//
//      
//      const BoolV con2 = V4IsGrtrOrEq( parallelTolerance, V4Abs(denom));
//      const Vec4V sTmp = V4Sel(con2, s1, s0);
//      
//      //compute point on segment2 closest to segment1
//      const Vec4V tTmp = V4Mul(V4Add(V4Mul(b, sTmp), f), eRecip);
//
//      //if t is in [zero, one], done. otherwise clamp t
//      const Vec4V t2 = V4Clamp(tTmp, zero, one);
//
//      //recompute s for the new value
//      const Vec4V comp = V4Mul(V4Sub(V4Mul(b,t2), c), aRecip);
//      const Vec4V s2 = V4Clamp(comp, zero, one);
//
//      s = V4Sel(con0, zero, V4Sel(con1, cd, s2));
//      t = V4Sel(con1, zero, V4Sel(con0, cg, t2));
//
//	  const Vec3V closest01 = V3ScaleAdd(d0, V4GetX(s), p);
//      const Vec3V closest02 = V3ScaleAdd(d02, V4GetX(t), p02);
//      const Vec3V closest11 = V3ScaleAdd(d0, V4GetY(s), p);
//      const Vec3V closest12 = V3ScaleAdd(d12, V4GetY(t), p12);
//      const Vec3V closest21 = V3ScaleAdd(d0, V4GetZ(s), p);
//      const Vec3V closest22 = V3ScaleAdd(d22, V4GetZ(t), p22);
//      const Vec3V closest31 = V3ScaleAdd(d0, V4GetW(s), p);
//      const Vec3V closest32 = V3ScaleAdd(d32, V4GetW(t), p32);
//
//      const Vec3V vv0 = V3Sub(closest01, closest02);
//      const Vec3V vv1 = V3Sub(closest11, closest12);
//      const Vec3V vv2 = V3Sub(closest21, closest22);
//      const Vec3V vv3 = V3Sub(closest31, closest32);
//
//      const FloatV vd[4] = {V3Dot(vv0, vv0), V3Dot(vv1, vv1), V3Dot(vv2, vv2), V3Dot(vv3, vv3)};
//      return V4Merge(vd);
//}




/*
	segment (p, d) and segment (p02, d02)
	segment (p, d) and segment (p12, d12)
	segment (p, d) and segment (p22, d22)
	segment (p, d) and segment (p32, d32)
*/

Ps::aos::Vec4V Gu::distanceSegmentSegmentSquared4(		const Ps::aos::Vec3VArg p, const Ps::aos::Vec3VArg d0, 
														const Ps::aos::Vec3VArg p02, const Ps::aos::Vec3VArg d02, 
                                                        const Ps::aos::Vec3VArg p12, const Ps::aos::Vec3VArg d12, 
														const Ps::aos::Vec3VArg p22, const Ps::aos::Vec3VArg d22,
                                                        const Ps::aos::Vec3VArg p32, const Ps::aos::Vec3VArg d32,
                                                        Ps::aos::Vec4V& s, Ps::aos::Vec4V& t)
{
      using namespace Ps::aos;
      const Vec4V zero = V4Zero();
      const Vec4V one = V4One();
      const Vec4V eps = V4Eps();
	  const Vec4V half = V4Splat(FHalf());
     // const Vec4V parallelTolerance  = Vec4V_From_F32(PX_PARALLEL_TOLERANCE);

	  const Vec4V d0X = V4Splat(V3GetX(d0));
	  const Vec4V d0Y = V4Splat(V3GetY(d0));
	  const Vec4V d0Z = V4Splat(V3GetZ(d0));
	  const Vec4V pX  = V4Splat(V3GetX(p));
	  const Vec4V pY  = V4Splat(V3GetY(p));
	  const Vec4V pZ  = V4Splat(V3GetZ(p));

	  Vec4V d024 = Vec4V_From_Vec3V(d02);
	  Vec4V d124 = Vec4V_From_Vec3V(d12);
	  Vec4V d224 = Vec4V_From_Vec3V(d22);
	  Vec4V d324 = Vec4V_From_Vec3V(d32);

	  Vec4V p024 = Vec4V_From_Vec3V(p02);
	  Vec4V p124 = Vec4V_From_Vec3V(p12);
	  Vec4V p224 = Vec4V_From_Vec3V(p22);
	  Vec4V p324 = Vec4V_From_Vec3V(p32);

	  Vec4V d0123X, d0123Y, d0123Z;
	  Vec4V p0123X, p0123Y, p0123Z;

	  PX_TRANSPOSE_44_34(d024, d124, d224, d324, d0123X, d0123Y, d0123Z);
	  PX_TRANSPOSE_44_34(p024, p124, p224, p324, p0123X, p0123Y, p0123Z);

	  const Vec4V rX = V4Sub(pX, p0123X);
	  const Vec4V rY = V4Sub(pY, p0123Y);
	  const Vec4V rZ = V4Sub(pZ, p0123Z);


	  //TODO - store this in a transposed state and avoid so many dot products?

	  const FloatV dd = V3Dot(d0, d0);

	  const Vec4V e = V4MulAdd(d0123Z, d0123Z, V4MulAdd(d0123X, d0123X, V4Mul(d0123Y, d0123Y)));
	  const Vec4V b = V4MulAdd(d0Z, d0123Z, V4MulAdd(d0X, d0123X, V4Mul(d0Y, d0123Y)));
	  const Vec4V c = V4MulAdd(d0Z, rZ, V4MulAdd(d0X, rX, V4Mul(d0Y, rY)));
	  const Vec4V f = V4MulAdd(d0123Z, rZ, V4MulAdd(d0123X, rX, V4Mul(d0123Y, rY))); 
	 

	  const Vec4V a(V4Splat(dd));

      const Vec4V aRecip(V4Recip(a));
      const Vec4V eRecip(V4Recip(e));


      //if segments not parallell, compute closest point on two segments and clamp to segment1
      const Vec4V denom = V4Sub(V4Mul(a, e), V4Mul(b, b));
      const Vec4V temp = V4Sub(V4Mul(b, f), V4Mul(c, e));
      const Vec4V s0 = V4Clamp(V4Div(temp, denom), zero, one);
      
  
	  //test whether segments are parallel
	  const BoolV con2 = V4IsGrtrOrEq(eps, denom);     
	  const Vec4V sTmp = V4Sel(con2, half, s0);
      
      //compute point on segment2 closest to segment1
      const Vec4V tTmp = V4Mul(V4Add(V4Mul(b, sTmp), f), eRecip);

      //if t is in [zero, one], done. otherwise clamp t
      const Vec4V t2 = V4Clamp(tTmp, zero, one);

      //recompute s for the new value
      const Vec4V comp = V4Mul(V4Sub(V4Mul(b,t2), c), aRecip);
	  const BoolV aaNearZero = V4IsGrtrOrEq(eps, a); // check if aRecip is valid (aa>eps)
      const Vec4V s2 = V4Sel(aaNearZero, V4Zero(), V4Clamp(comp, zero, one));

    /*  s = V4Sel(con0, zero, V4Sel(con1, cd, s2));
      t = V4Sel(con1, zero, V4Sel(con0, cg, t2));  */
	  s = s2;
	  t = t2;

	  const Vec4V closest1X = V4MulAdd(d0X, s2, pX);
	  const Vec4V closest1Y = V4MulAdd(d0Y, s2, pY);
	  const Vec4V closest1Z = V4MulAdd(d0Z, s2, pZ);

	  const Vec4V closest2X = V4MulAdd(d0123X, t2, p0123X);
	  const Vec4V closest2Y = V4MulAdd(d0123Y, t2, p0123Y);
	  const Vec4V closest2Z = V4MulAdd(d0123Z, t2, p0123Z);

	  const Vec4V vvX = V4Sub(closest1X, closest2X);
	  const Vec4V vvY = V4Sub(closest1Y, closest2Y);
	  const Vec4V vvZ = V4Sub(closest1Z, closest2Z);

	  const Vec4V vd = V4MulAdd(vvX, vvX, V4MulAdd(vvY, vvY, V4Mul(vvZ, vvZ)));

	  return vd;
}

/*
	segment p(p+d0) and segment p01(p02 + d1)
	segment p(p+d0) and segment p12(p12 + d1)
	segment p(p+d0) and segment p22(p22 + d1)
	segment p(p+d0) and segment p32(p32 + d1)
*/
//void Gu::distanceSegmentSegmentSquared4(		const Ps::aos::Vec3VArg p, const Ps::aos::Vec3VArg d0, 
//														const Ps::aos::Vec3VArg p02,  
//                                                        const Ps::aos::Vec3VArg p12,  
//														const Ps::aos::Vec3VArg p22, 
//                                                        const Ps::aos::Vec3VArg p32, 
//														const Ps::aos::Vec3VArg d1,
//                                                        Ps::aos::FloatV* s, Ps::aos::FloatV* t,
//														Ps::aos::Vec3V* closest,
//														Ps::aos::FloatV* sqDist)
//{
//
//      using namespace Ps::aos;
//      const Vec4V zero = V4Zero();
//      const Vec4V one = V4One();
//      const Vec4V eps = V4Eps();
//      const Vec4V parallelTolerance  = Vec4V_From_F32(PX_PARALLEL_TOLERANCE);
//
//      const Vec3V r0 = V3Sub(p, p02);
//      const Vec3V r1 = V3Sub(p, p12);
//      const Vec3V r2 = V3Sub(p, p22);
//      const Vec3V r3 = V3Sub(p, p32);
//
//	  //TODO - store this in a transposed state and avoid so many dot products?
//
//	  const FloatV dd0 = V3Dot(d0, d0);
//	  const FloatV dd1 = V3Dot(d1, d1);
//	  const FloatV d0d1 = V3Dot(d0, d1);
//	
//      const FloatV _c[4] = { V3Dot(d0, r0), V3Dot(d0, r1), V3Dot(d0, r2), V3Dot(d0, r3) };
//      const FloatV _f[4] = { V3Dot(d1, r0), V3Dot(d1, r1), V3Dot(d1, r2), V3Dot(d1, r3) };
//
//
//      const Vec4V a(V4Splat(dd0));
//      const Vec4V e(V4Splat(dd1));
//
//      const Vec4V b(V4Splat(d0d1));
//      const Vec4V c(V4Merge(_c));
//      const Vec4V f(V4Merge(_f));
//
//      const Vec4V aRecip(V4Recip(a));
//      const Vec4V eRecip(V4Recip(e));
//
//      const Vec4V nc(V4Neg(c));
//      const Vec4V d(V4Mul(nc, aRecip));
//      const Vec4V cd(V4Clamp(d, zero, one));
//      
//      
//      const Vec4V g = V4Mul(f, eRecip);
//      const Vec4V cg = V4Clamp(g, zero, one); //when s = zero
//      const Vec4V h = V4Mul(nc, a);
//      
//
//      //check if either or both segments degenerate into points
//      const BoolV con0 = V4IsGrtr(eps, a);
//      const BoolV con1 = V4IsGrtr(eps, e);
//            
//      //the general nondegenerate case
//
//      //if segments not parallell, compute closest point on two segments and clamp to segment1
//      const Vec4V denom = V4Sub(V4Mul(a, e), V4Mul(b, b));
//      const Vec4V denomRecip = V4Recip(denom);
//      const Vec4V temp = V4Sub(V4Mul(b, f), V4Mul(c, e));
//      const Vec4V s0 = V4Clamp(V4Mul(temp, denomRecip), zero, one);
//      
//      //if segment is parallel
//      const BoolV con00 = V4IsGrtr(b, zero);//segment p1q1 and p2q2 point to the same direction
//      const BoolV con01 = V4IsGrtrOrEq(c, zero);
//      const BoolV con02 = V4IsGrtr(a, nc);
//
//      const Vec4V s01 = V4Sel(con01, zero, V4Sel(con02, h, one));
//      const Vec4V s02 = V4Sel(con02, V4Sel(con01, zero, d), one);
//      const Vec4V s1 = V4Sel(con00, s02, s01);
//
//      
//      const BoolV con2 = V4IsGrtrOrEq( parallelTolerance, V4Abs(denom));
//      const Vec4V sTmp = V4Sel(con2, s1, s0);
//      
//      //compute point on segment2 closest to segment1
//      const Vec4V tTmp = V4Mul(V4Add(V4Mul(b, sTmp), f), eRecip);
//
//      //if t is in [zero, one], done. otherwise clamp t
//      const Vec4V t2 = V4Clamp(tTmp, zero, one);
//
//      //recompute s for the new value
//      const Vec4V comp = V4Mul(V4Sub(V4Mul(b,t2), c), aRecip);
//      const Vec4V s2 = V4Clamp(comp, zero, one);
//
//      const Vec4V sv = V4Sel(con0, zero, V4Sel(con1, cd, s2));
//      const Vec4V tv = V4Sel(con1, zero, V4Sel(con0, cg, t2));
//	  s[0] = V4GetX(sv);
//	  s[1] = V4GetY(sv);
//	  s[2] = V4GetZ(sv);
//	  s[3] = V4GetW(sv);
//
//	  t[0] = V4GetX(tv);
//	  t[1] = V4GetY(tv);
//	  t[2] = V4GetZ(tv);
//	  t[3] = V4GetW(tv);
//
//	  const Vec3V closest01 = V3ScaleAdd(d0, s[0], p);
//      const Vec3V closest11 = V3ScaleAdd(d0, s[1], p);
//	  const Vec3V closest21 = V3ScaleAdd(d0, s[2], p);
//	  const Vec3V closest31 = V3ScaleAdd(d0, s[3], p);
//
//	  const Vec3V closest02 = V3ScaleAdd(d1, t[0], p02);
//      const Vec3V closest12 = V3ScaleAdd(d1, t[1], p12);
//      const Vec3V closest22 = V3ScaleAdd(d1, t[2], p22);
//      const Vec3V closest32 = V3ScaleAdd(d1, t[3], p32);
//
//	  closest[0] = closest02;
//	  closest[1] = closest12;
//	  closest[2] = closest22;
//	  closest[3] = closest32;
//
//      const Vec3V vv0 = V3Sub(closest01, closest02);
//      const Vec3V vv1 = V3Sub(closest11, closest12);
//      const Vec3V vv2 = V3Sub(closest21, closest22);
//      const Vec3V vv3 = V3Sub(closest31, closest32);
//
//	  sqDist[0] = V3Dot(vv0, vv0);
//	  sqDist[1] = V3Dot(vv1, vv1);
//	  sqDist[2] = V3Dot(vv2, vv2);
//	  sqDist[3] = V3Dot(vv3, vv3);
//}

Ps::aos::Vec4V Gu::distanceSegmentSegmentSquared4(		const Ps::aos::Vec3VArg p, const Ps::aos::Vec3VArg d0, 
														const Ps::aos::Vec3VArg p02,  
                                                        const Ps::aos::Vec3VArg p12,  
														const Ps::aos::Vec3VArg p22, 
                                                        const Ps::aos::Vec3VArg p32, 
														const Ps::aos::Vec3VArg d1,
                                                        Ps::aos::Vec4V& s, Ps::aos::Vec4V& t,
														Ps::aos::Vec3V* closest)
{

      using namespace Ps::aos;
      const Vec4V zero = V4Zero();
      const Vec4V one = V4One();
      const Vec4V eps = V4Eps();
      const Vec4V parallelTolerance  = V4Load(PX_PARALLEL_TOLERANCE);

	  const Vec4V d0X = V4Splat(V3GetX(d0));
	  const Vec4V d0Y = V4Splat(V3GetY(d0));
	  const Vec4V d0Z = V4Splat(V3GetZ(d0));

	  const Vec4V d1X = V4Splat(V3GetX(d1));
	  const Vec4V d1Y = V4Splat(V3GetY(d1));
	  const Vec4V d1Z = V4Splat(V3GetZ(d1));

	  const Vec4V pX  = V4Splat(V3GetX(p));
	  const Vec4V pY  = V4Splat(V3GetY(p));
	  const Vec4V pZ  = V4Splat(V3GetZ(p));

	  Vec4V p024 = Vec4V_From_Vec3V(p02);
	  Vec4V p124 = Vec4V_From_Vec3V(p12);
	  Vec4V p224 = Vec4V_From_Vec3V(p22);
	  Vec4V p324 = Vec4V_From_Vec3V(p32);

	  Vec4V p0123X, p0123Y, p0123Z;

	  PX_TRANSPOSE_44_34(p024, p124, p224, p324, p0123X, p0123Y, p0123Z);

	  const Vec4V rX = V4Sub(pX, p0123X);
	  const Vec4V rY = V4Sub(pY, p0123Y);
	  const Vec4V rZ = V4Sub(pZ, p0123Z);



     /* const Vec3V r0 = V3Sub(p, p02);
      const Vec3V r1 = V3Sub(p, p12);
      const Vec3V r2 = V3Sub(p, p22);
      const Vec3V r3 = V3Sub(p, p32);*/

	  //TODO - store this in a transposed state and avoid so many dot products?

	  const FloatV dd0 = V3Dot(d0, d0);
	  const FloatV dd1 = V3Dot(d1, d1);
	  const FloatV d0d1 = V3Dot(d0, d1);
	
     // const FloatV _c[4] = { V3Dot(d0, r0), V3Dot(d0, r1), V3Dot(d0, r2), V3Dot(d0, r3) };
      //const FloatV _f[4] = { V3Dot(d1, r0), V3Dot(d1, r1), V3Dot(d1, r2), V3Dot(d1, r3) };

	  const Vec4V c = V4MulAdd(d0X, rX, V4MulAdd(d0Y, rY, V4Mul(d0Z, rZ)));
	  const Vec4V f = V4MulAdd(d1X, rX, V4MulAdd(d1Y, rY, V4Mul(d1Z, rZ)));


      const Vec4V a(V4Splat(dd0));
      const Vec4V e(V4Splat(dd1));

      const Vec4V b(V4Splat(d0d1));

      const Vec4V aRecip(V4Recip(a));
      const Vec4V eRecip(V4Recip(e));

      const Vec4V nc(V4Neg(c));
      const Vec4V d(V4Mul(nc, aRecip));
      const Vec4V cd(V4Clamp(d, zero, one));
      
      
      const Vec4V g = V4Mul(f, eRecip);
      const Vec4V cg = V4Clamp(g, zero, one); //when s = zero
      const Vec4V h = V4Mul(nc, a);
      

      //check if either or both segments degenerate into points
      const BoolV con0 = V4IsGrtr(eps, a);
      const BoolV con1 = V4IsGrtr(eps, e);
            
      //the general nondegenerate case

      //if segments not parallell, compute closest point on two segments and clamp to segment1
      const Vec4V denom = V4Sub(V4Mul(a, e), V4Mul(b, b));
      const Vec4V denomRecip = V4Recip(denom);
      const Vec4V temp = V4Sub(V4Mul(b, f), V4Mul(c, e));
      const Vec4V s0 = V4Clamp(V4Mul(temp, denomRecip), zero, one);
      
      //if segment is parallel
      const BoolV con00 = V4IsGrtr(b, zero);//segment p1q1 and p2q2 point to the same direction
      const BoolV con01 = V4IsGrtrOrEq(c, zero);
      const BoolV con02 = V4IsGrtr(a, nc);

      const Vec4V s01 = V4Sel(con01, zero, V4Sel(con02, h, one));
      const Vec4V s02 = V4Sel(con02, V4Sel(con01, zero, d), one);
      const Vec4V s1 = V4Sel(con00, s02, s01);

      
      const BoolV con2 = V4IsGrtrOrEq( parallelTolerance, V4Abs(denom));
      const Vec4V sTmp = V4Sel(con2, s1, s0);
      
      //compute point on segment2 closest to segment1
      const Vec4V tTmp = V4Mul(V4Add(V4Mul(b, sTmp), f), eRecip);

      //if t is in [zero, one], done. otherwise clamp t
      const Vec4V t2 = V4Clamp(tTmp, zero, one);

      //recompute s for the new value
      const Vec4V comp = V4Mul(V4Sub(V4Mul(b,t2), c), aRecip);
      const Vec4V s2 = V4Clamp(comp, zero, one);

      const Vec4V sv = V4Sel(con0, zero, V4Sel(con1, cd, s2));
      const Vec4V tv = V4Sel(con1, zero, V4Sel(con0, cg, t2));
	  s = sv;

	  t = tv;

	  const Vec4V closest1X = V4MulAdd(d0X, sv, pX);
	  const Vec4V closest1Y = V4MulAdd(d0Y, sv, pY);
	  const Vec4V closest1Z = V4MulAdd(d0Z, sv, pZ);

	  Vec4V closest2X = V4MulAdd(d1X, tv, p0123X);
	  Vec4V closest2Y = V4MulAdd(d1Y, tv, p0123Y);
	  Vec4V closest2Z = V4MulAdd(d1Z, tv, p0123Z);

	  const Vec4V vvX = V4Sub(closest1X, closest2X);
	  const Vec4V vvY = V4Sub(closest1Y, closest2Y);
	  const Vec4V vvZ = V4Sub(closest1Z, closest2Z);

	  const Vec4V vd = V4MulAdd(vvX, vvX, V4MulAdd(vvY, vvY, V4Mul(vvZ, vvZ)));

	  Vec4V closest02, closest12, closest22, closest32;

	  PX_TRANSPOSE_34_44(closest2X, closest2Y, closest2Z, closest02, closest12, closest22, closest32);

	  closest[0] = Vec3V_From_Vec4V(closest02);
	  closest[1] = Vec3V_From_Vec4V(closest12);
	  closest[2] = Vec3V_From_Vec4V(closest22);
	  closest[3] = Vec3V_From_Vec4V(closest32);

	  return vd;
}


