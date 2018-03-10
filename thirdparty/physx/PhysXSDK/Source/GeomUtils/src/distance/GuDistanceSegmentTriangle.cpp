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

#include "PsIntrinsics.h"
#include "GuDistanceSegmentTriangle.h"
#include "GuDistanceSegmentTriangleSIMD.h"
#include "GuDistancePointTriangle.h"
#include "GuDistancePointTriangleSIMD.h"
#include "GuDistanceSegmentSegment.h"
#include "GuDistanceSegmentSegmentSIMD.h"
#include "GuBarycentricCoordinates.h"

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

// ptchernev:
// The code has been modified to use a relative error test since the absolute
// test would break down for small geometries. (TTP 4021)

PxReal Gu::distanceSegmentTriangleSquared(	const PxVec3& origin, const PxVec3& dir,
											const PxVec3& p0, const PxVec3& triEdge0, const PxVec3& triEdge1,
											PxReal* t, PxReal* u, PxReal* v)
{
	const PxReal fA00 = dir.magnitudeSquared();
	if (fA00 < 1e-6f*1e-6f)
	{
		if (t)
			*t = 0.0f;
		return Gu::distancePointTriangleSquared(origin, p0, triEdge0, triEdge1, u, v);
	}
	const PxVec3 kDiff = p0 - origin;
	const PxReal fA01 = -(dir.dot(triEdge0));
	const PxReal fA02 = -(dir.dot(triEdge1));
	const PxReal fA11 = triEdge0.magnitudeSquared();
	const PxReal fA12 = triEdge0.dot(triEdge1);
	const PxReal fA22 = triEdge1.dot(triEdge1);
	const PxReal fB0  = -(kDiff.dot(dir));
	const PxReal fB1  = kDiff.dot(triEdge0);
	const PxReal fB2  = kDiff.dot(triEdge1);
	const PxReal fCof00 = fA11*fA22-fA12*fA12;
	const PxReal fCof01 = fA02*fA12-fA01*fA22;
	const PxReal fCof02 = fA01*fA12-fA02*fA11;
	const PxReal fDet = fA00*fCof00+fA01*fCof01+fA02*fCof02;

	PxVec3 kTriSegOrig;
	PxVec3 kTriSegDir;	
	PxVec3 kPt;
	PxReal fSqrDist, fSqrDist0, fR, fS, fT, fR0, fS0, fT0;

	// Set up for a relative error test on the angle between ray direction
	// and triangle normal to determine parallel/nonparallel status.
	const PxVec3 kNormal = triEdge0.cross(triEdge1);
	const PxReal fDot = kNormal.dot(dir);
	if ( fDot*fDot >= 1e-6*dir.magnitudeSquared()*kNormal.magnitudeSquared())
	{
		const PxReal fCof11 = fA00*fA22-fA02*fA02;
		const PxReal fCof12 = fA02*fA01-fA00*fA12;
		const PxReal fCof22 = fA00*fA11-fA01*fA01;
		const PxReal fInvDet = 1.0f/fDet;
		const PxReal fRhs0 = -fB0*fInvDet;
		const PxReal fRhs1 = -fB1*fInvDet;
		const PxReal fRhs2 = -fB2*fInvDet;

		fR = fCof00*fRhs0+fCof01*fRhs1+fCof02*fRhs2;
		fS = fCof01*fRhs0+fCof11*fRhs1+fCof12*fRhs2;
		fT = fCof02*fRhs0+fCof12*fRhs1+fCof22*fRhs2;

		if ( fR < 0.0f )
		{
			if ( fS+fT <= 1.0f )
			{
				if ( fS < 0.0f )
				{
					if ( fT < 0.0f )  // region 4m
					{
						// minimum on face s=0 or t=0 or r=0
						kTriSegOrig = p0;
						kTriSegDir = triEdge1;
						fSqrDist = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR, &fT);
						fS = 0.0f;
						kTriSegOrig = p0;
						kTriSegDir = triEdge0;
						fSqrDist0 = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR0, &fS0);
						fT0 = 0.0f;
						if ( fSqrDist0 < fSqrDist )
						{
							fSqrDist = fSqrDist0;
							fR = fR0;
							fS = fS0;
							fT = fT0;
						}
						fSqrDist0 = Gu::distancePointTriangleSquared(origin, p0, triEdge0, triEdge1, &fS0, &fT0);
						fR0 = 0.0f;
						if ( fSqrDist0 < fSqrDist )
						{
							fSqrDist = fSqrDist0;
							fR = fR0;
							fS = fS0;
							fT = fT0;
						}
					}
					else  // region 3m
					{
						// minimum on face s=0 or r=0
						kTriSegOrig = p0;
						kTriSegDir = triEdge1;
						fSqrDist = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR, &fT);
						fS = 0.0f;
						fSqrDist0 = Gu::distancePointTriangleSquared(origin, p0, triEdge0, triEdge1, &fS0, &fT0);
						fR0 = 0.0f;
						if ( fSqrDist0 < fSqrDist )
						{
							fSqrDist = fSqrDist0;
							fR = fR0;
							fS = fS0;
							fT = fT0;
						}
					}
				}
				else if ( fT < 0.0f )  // region 5m
				{
					// minimum on face t=0 or r=0
					kTriSegOrig = p0;
					kTriSegDir = triEdge0;
					fSqrDist = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR, &fS);
					fT = 0.0f;
					fSqrDist0 = Gu::distancePointTriangleSquared(origin, p0, triEdge0, triEdge1, &fS0, &fT0);
					fR0 = 0.0f;
					if ( fSqrDist0 < fSqrDist )
					{
						fSqrDist = fSqrDist0;
						fR = fR0;
						fS = fS0;
						fT = fT0;
					}
				}
				else  // region 0m
				{
					// minimum on face r=0
					fSqrDist = Gu::distancePointTriangleSquared(origin, p0, triEdge0, triEdge1, &fS, &fT);
					fR = 0.0f;
				}
			}
			else
			{
				if ( fS < 0.0f )  // region 2m
				{
					// minimum on face s=0 or s+t=1 or r=0
					kTriSegOrig = p0;
					kTriSegDir = triEdge1;
					fSqrDist = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR, &fT);
					fS = 0.0f;
					kTriSegOrig = p0+triEdge0;
					kTriSegDir = triEdge1-triEdge0;
					fSqrDist0 = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR0, &fT0);
					fS0 = 1.0f-fT0;
					if ( fSqrDist0 < fSqrDist )
					{
						fSqrDist = fSqrDist0;
						fR = fR0;
						fS = fS0;
						fT = fT0;
					}
					fSqrDist0 = Gu::distancePointTriangleSquared(origin, p0, triEdge0, triEdge1, &fS0, &fT0);
					fR0 = 0.0f;
					if ( fSqrDist0 < fSqrDist )
					{
						fSqrDist = fSqrDist0;
						fR = fR0;
						fS = fS0;
						fT = fT0;
					}
				}
				else if ( fT < 0.0f )  // region 6m
				{
					// minimum on face t=0 or s+t=1 or r=0
					kTriSegOrig = p0;
					kTriSegDir = triEdge0;
					fSqrDist = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR, &fS);
					fT = 0.0f;
					kTriSegOrig = p0+triEdge0;
					kTriSegDir = triEdge1-triEdge0;
					fSqrDist0 = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR0, &fT0);
					fS0 = 1.0f-fT0;
					if ( fSqrDist0 < fSqrDist )
					{
						fSqrDist = fSqrDist0;
						fR = fR0;
						fS = fS0;
						fT = fT0;
					}
					fSqrDist0 = Gu::distancePointTriangleSquared(origin, p0, triEdge0, triEdge1, &fS0, &fT0);
					fR0 = 0.0f;
					if ( fSqrDist0 < fSqrDist )
					{
						fSqrDist = fSqrDist0;
						fR = fR0;
						fS = fS0;
						fT = fT0;
					}
				}
				else  // region 1m
				{
					// minimum on face s+t=1 or r=0
					kTriSegOrig = p0+triEdge0;
					kTriSegDir = triEdge1-triEdge0;
					fSqrDist = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR, &fT);
					fS = 1.0f-fT;
					fSqrDist0 = Gu::distancePointTriangleSquared(origin, p0, triEdge0, triEdge1, &fS0, &fT0);
					fR0 = 0.0f;
					if ( fSqrDist0 < fSqrDist )
					{
						fSqrDist = fSqrDist0;
						fR = fR0;
						fS = fS0;
						fT = fT0;
					}
				}
			}
		}
		else if ( fR <= 1.0f )
		{
			if ( fS+fT <= 1.0f )
			{
				if ( fS < 0.0f )
				{
					if ( fT < 0.0f )  // region 4
					{
						// minimum on face s=0 or t=0
						kTriSegOrig = p0;
						kTriSegDir = triEdge1;
						fSqrDist = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR, &fT);
						fS = 0.0f;
						kTriSegOrig = p0;
						kTriSegDir = triEdge0;
						fSqrDist0 = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR0, &fS0);
						fT0 = 0.0f;
						if ( fSqrDist0 < fSqrDist )
						{
							fSqrDist = fSqrDist0;
							fR = fR0;
							fS = fS0;
							fT = fT0;
						}
					}
					else  // region 3
					{
						// minimum on face s=0
						kTriSegOrig = p0;
						kTriSegDir = triEdge1;
						fSqrDist = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR, &fT);
						fS = 0.0f;
					}
				}
				else if ( fT < 0.0f )  // region 5
				{
					// minimum on face t=0
					kTriSegOrig = p0;
					kTriSegDir = triEdge0;
					fSqrDist = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR, &fS);
					fT = 0.0f;
				}
				else  // region 0
				{
					// PT: should be zero, according to Gang. I think he's right.
					// global minimum is interior, done
/*					fSqrDist = fR*(fA00*fR+fA01*fS+fA02*fT+2.0f*fB0)
						+fS*(fA01*fR+fA11*fS+fA12*fT+2.0f*fB1)
						+fT*(fA02*fR+fA12*fS+fA22*fT+2.0f*fB2)
						+kDiff.magnitudeSquared();*/
					fSqrDist = 0.0f;
				}
			}
			else
			{
				if ( fS < 0.0f )  // region 2
				{
					// minimum on face s=0 or s+t=1
					kTriSegOrig = p0;
					kTriSegDir = triEdge1;
					fSqrDist = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR, &fT);
					fS = 0.0f;
					kTriSegOrig = p0+triEdge0;
					kTriSegDir = triEdge1-triEdge0;
					fSqrDist0 = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR0, &fT0);
					fS0 = 1.0f-fT0;
					if ( fSqrDist0 < fSqrDist )
					{
						fSqrDist = fSqrDist0;
						fR = fR0;
						fS = fS0;
						fT = fT0;
					}
				}
				else if ( fT < 0.0f )  // region 6
				{
					// minimum on face t=0 or s+t=1
					kTriSegOrig = p0;
					kTriSegDir = triEdge0;
					fSqrDist = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR, &fS);
					fT = 0.0f;
					kTriSegOrig = p0+triEdge0;
					kTriSegDir = triEdge1-triEdge0;
					fSqrDist0 = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR0, &fT0);
					fS0 = 1.0f-fT0;
					if ( fSqrDist0 < fSqrDist )
					{
						fSqrDist = fSqrDist0;
						fR = fR0;
						fS = fS0;
						fT = fT0;
					}
				}
				else  // region 1
				{
					// minimum on face s+t=1
					kTriSegOrig = p0+triEdge0;
					kTriSegDir = triEdge1-triEdge0;
					fSqrDist = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR, &fT);
					fS = 1.0f-fT;
				}
			}
		}
		else  // fR > 1
		{
			if ( fS+fT <= 1.0f )
			{
				if ( fS < 0.0f )
				{
					if ( fT < 0.0f )  // region 4p
					{
						// minimum on face s=0 or t=0 or r=1
						kTriSegOrig = p0;
						kTriSegDir = triEdge1;
						fSqrDist = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR, &fT);
						fS = 0.0f;
						kTriSegOrig = p0;
						kTriSegDir = triEdge0;
						fSqrDist0 = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR0, &fS0);
						fT0 = 0.0f;
						if ( fSqrDist0 < fSqrDist )
						{
							fSqrDist = fSqrDist0;
							fR = fR0;
							fS = fS0;
							fT = fT0;
						}
						kPt = origin+dir;
						fSqrDist0 = Gu::distancePointTriangleSquared(kPt, p0, triEdge0, triEdge1, &fS0, &fT0);
						fR0 = 1.0f;
						if ( fSqrDist0 < fSqrDist )
						{
							fSqrDist = fSqrDist0;
							fR = fR0;
							fS = fS0;
							fT = fT0;
						}
					}
					else  // region 3p
					{
						// minimum on face s=0 or r=1
						kTriSegOrig = p0;
						kTriSegDir = triEdge1;
						fSqrDist = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR, &fT);
						fS = 0.0f;
						kPt = origin+dir;
						fSqrDist0 = Gu::distancePointTriangleSquared(kPt, p0, triEdge0, triEdge1, &fS0, &fT0);
						fR0 = 1.0f;
						if ( fSqrDist0 < fSqrDist )
						{
							fSqrDist = fSqrDist0;
							fR = fR0;
							fS = fS0;
							fT = fT0;
						}
					}
				}
				else if ( fT < 0.0f )  // region 5p
				{
					// minimum on face t=0 or r=1
					kTriSegOrig = p0;
					kTriSegDir = triEdge0;
					fSqrDist = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR, &fS);
					fT = 0.0f;
					kPt = origin+dir;
					fSqrDist0 = Gu::distancePointTriangleSquared(kPt, p0, triEdge0, triEdge1, &fS0, &fT0);
					fR0 = 1.0f;
					if ( fSqrDist0 < fSqrDist )
					{
						fSqrDist = fSqrDist0;
						fR = fR0;
						fS = fS0;
						fT = fT0;
					}
				}
				else  // region 0p
				{
					// minimum face on r=1
					kPt = origin+dir;
					fSqrDist = Gu::distancePointTriangleSquared(kPt, p0, triEdge0, triEdge1, &fS, &fT);
					fR = 1.0f;
				}
			}
			else
			{
				if ( fS < 0.0f )  // region 2p
				{
					// minimum on face s=0 or s+t=1 or r=1
					kTriSegOrig = p0;
					kTriSegDir = triEdge1;
					fSqrDist = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR, &fT);
					fS = 0.0f;
					kTriSegOrig = p0+triEdge0;
					kTriSegDir = triEdge1-triEdge0;
					fSqrDist0 = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR0, &fT0);
					fS0 = 1.0f-fT0;
					if ( fSqrDist0 < fSqrDist )
					{
						fSqrDist = fSqrDist0;
						fR = fR0;
						fS = fS0;
						fT = fT0;
					}
					kPt = origin+dir;
					fSqrDist0 = Gu::distancePointTriangleSquared(kPt, p0, triEdge0, triEdge1, &fS0, &fT0);
					fR0 = 1.0f;
					if ( fSqrDist0 < fSqrDist )
					{
						fSqrDist = fSqrDist0;
						fR = fR0;
						fS = fS0;
						fT = fT0;
					}
				}
				else if ( fT < 0.0f )  // region 6p
				{
					// minimum on face t=0 or s+t=1 or r=1
					kTriSegOrig = p0;
					kTriSegDir = triEdge0;
					fSqrDist = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR, &fS);
					fT = 0.0f;
					kTriSegOrig = p0+triEdge0;
					kTriSegDir = triEdge1-triEdge0;
					fSqrDist0 = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR0, &fT0);
					fS0 = 1.0f-fT0;
					if ( fSqrDist0 < fSqrDist )
					{
						fSqrDist = fSqrDist0;
						fR = fR0;
						fS = fS0;
						fT = fT0;
					}
					kPt = origin+dir;
					fSqrDist0 = Gu::distancePointTriangleSquared(kPt, p0, triEdge0, triEdge1, &fS0, &fT0);
					fR0 = 1.0f;
					if ( fSqrDist0 < fSqrDist )
					{
						fSqrDist = fSqrDist0;
						fR = fR0;
						fS = fS0;
						fT = fT0;
					}
				}
				else  // region 1p
				{
					// minimum on face s+t=1 or r=1
					kTriSegOrig = p0+triEdge0;
					kTriSegDir = triEdge1-triEdge0;
					fSqrDist = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR, &fT);
					fS = 1.0f-fT;
					kPt = origin+dir;
					fSqrDist0 = Gu::distancePointTriangleSquared(kPt, p0, triEdge0, triEdge1, &fS0, &fT0);
					fR0 = 1.0f;
					if ( fSqrDist0 < fSqrDist )
					{
						fSqrDist = fSqrDist0;
						fR = fR0;
						fS = fS0;
						fT = fT0;
					}
				}
			}
		}
	}
	else
	{
		// segment and triangle are parallel
		kTriSegOrig = p0;
		kTriSegDir = triEdge0;
		fSqrDist = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR, &fS);
		fT = 0.0f;

		kTriSegDir = triEdge1;
		fSqrDist0 = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR0, &fT0);
		fS0 = 0.0f;
		if ( fSqrDist0 < fSqrDist )
		{
			fSqrDist = fSqrDist0;
			fR = fR0;
			fS = fS0;
			fT = fT0;
		}

		kTriSegOrig = p0+triEdge0;
		kTriSegDir = triEdge1 - triEdge0;
		fSqrDist0 = Gu::distanceSegmentSegmentSquaredOLD(origin, dir, kTriSegOrig, kTriSegDir, &fR0, &fT0);
		fS0 = 1.0f-fT0;
		if ( fSqrDist0 < fSqrDist )
		{
			fSqrDist = fSqrDist0;
			fR = fR0;
			fS = fS0;
			fT = fT0;
		}

		fSqrDist0 = Gu::distancePointTriangleSquared(origin, p0, triEdge0, triEdge1, &fS0, &fT0);
		fR0 = 0.0f;
		if ( fSqrDist0 < fSqrDist )
		{
			fSqrDist = fSqrDist0;
			fR = fR0;
			fS = fS0;
			fT = fT0;
		}

		kPt = origin+dir;
		fSqrDist0 = Gu::distancePointTriangleSquared(kPt, p0, triEdge0, triEdge1, &fS0, &fT0);
		fR0 = 1.0f;
		if ( fSqrDist0 < fSqrDist )
		{
			fSqrDist = fSqrDist0;
			fR = fR0;
			fS = fS0;
			fT = fT0;
		}
	}

	if(t)	*t = fR;
	if(u)	*u = fS;
	if(v)	*v = fT;

	// account for numerical round-off error
	return physx::intrinsics::selectMax(0.0f, fSqrDist);
}

///*
//	closest0 is the closest point on segment pq
//	closest1 is the closest point on triangle abc
//*/
//Ps::aos::FloatV Gu::distanceSegmentTriangleSquared(	const Ps::aos::Vec3VArg p, const Ps::aos::Vec3VArg q,
//													const Ps::aos::Vec3VArg a, const Ps::aos::Vec3VArg b, const Ps::aos::Vec3VArg c,
//													Ps::aos::Vec3V& closest0, Ps::aos::Vec3V& closest1)
//{
//	using namespace Ps::aos;
//	const FloatV zero = FZero();
//	const FloatV one = FOne();
//	const FloatV parallelTolerance  = FloatV_From_F32(PX_PARALLEL_TOLERANCE);
//
//	const Vec3V pq = V3Sub(q, p);
//	const Vec3V ab = V3Sub(b, a);
//	const Vec3V ac = V3Sub(c, a);
//	const Vec3V bc = V3Sub(c, b);
//	const Vec3V ap = V3Sub(p, a);
//	const Vec3V aq = V3Sub(q, a);
//
//	const Vec3V n =V3Normalise(V3Cross(ab, ac)); // normalize vector
//
//	//compute the closest point of p and triangle plane abc
//	const FloatV dist3 = V3Dot(ap, n);
//	const FloatV sqDist3 = FMul(dist3, dist3);
//
//
//	//compute the closest point of q and triangle plane abc
//	const FloatV dist4 = V3Dot(aq, n);
//	const FloatV sqDist4 = FMul(dist4, dist4);
//	const FloatV dMul = FMul(dist3, dist4);
//	const BoolV con = FIsGrtr(zero, dMul);
//	PxU32 ww;
//	Store_From_BoolV(con, &ww);
//
//
//	if(ww) // intersect with the plane
//	{
//		//compute the intersect point
//		const FloatV nom = FNeg(V3Dot(n, p));
//		const FloatV denom = FRecip(V3Dot(n, pq));
//		const FloatV t = FMul(nom, denom);
//		const Vec3V ip = FAdd(p, V3Scale(t, pq));
//		FloatV v0, w0; // p = a + v*ab + w*ac;
//		barycentricCoordinates(ab, ac, FSub(ip, a), v0, w0);
//		const BoolV con00 = BAnd(FIsGrtrOrEq(v0, zero), FIsGrtr(one, v0));
//		const BoolV con01 = BAnd(FIsGrtrOrEq(w0, zero), FIsGrtr(one, w0));
//		const BoolV con02 = FIsGrtr(one, FAdd(v0, w0));
//		const BoolV con0 = BAnd(con00, BAnd(con01, con02));
//		PxU32 ww;
//		Store_From_BoolV(con0, &ww);
//		if(ww)
//		{
//			closest0 = closest1 = ip;
//			return zero;
//		}
//	}
//	
//	//compute the closest point between segment pq and triangle edge AB
//	FloatV t00, t01;
//	const FloatV sqDist0 = distanceSegmentSegmentSquared(p,q,a,b, t00, t01);
//	const Vec3V closestP00 = V3Add(p, V3Scale(pq, t00));
//	const Vec3V closestP01 = V3Add(a, V3Scale(ab, t01));
//
//	//compute the closest point between segment pq and triangle edge BC
//	FloatV t10, t11;
//	const FloatV sqDist1 = distanceSegmentSegmentSquared(p,q, b,c, t10, t11);
//	const Vec3V closestP10 = V3Add(p, V3Scale(pq, t10));
//	const Vec3V closestP11 = V3Add(b, V3Scale(bc, t11));
//	 
//	//compute the closest point between segment pq and triangle edge ac
//	FloatV t20, t21;
//	const FloatV sqDist2 = distanceSegmentSegmentSquared(p,q, a,c, t20, t21);
//	const Vec3V closestP20 = V3Add(p, V3Scale(pq, t20));
//	const Vec3V closestP21 = V3Add(a, V3Scale(ac, t21));
//
//	//Get the closest point of all edges
//	const BoolV con20 = FIsGrtr(sqDist1, sqDist0);
//	const BoolV con21 = FIsGrtr(sqDist2, sqDist0);
//	const BoolV con2 = BAnd(con20,con21);
//	const BoolV con30 = FIsGrtrOrEq(sqDist0, sqDist1);
//	const BoolV con31 = FIsGrtr(sqDist2, sqDist1);
//	const BoolV con3 = BAnd(con30, con31);
//	const FloatV sqDistPE = FSel(con2, sqDist0, FSel(con3, sqDist1, sqDist2));
//	//const FloatV tValue = FSel(con2, t00, FSel(con3, t10, t20));
//	const Vec3V closestPE0 =  FSel(con2, closestP00, FSel(con3, closestP10, closestP20)); // closestP on segment
//	const Vec3V closestPE1 =  FSel(con2, closestP01, FSel(con3, closestP11, closestP21)); // closestP on triangle
//
//	const Vec3V closestP31 = FSub(p, V3Scale(n, dist3));
//	const Vec3V closestP30 = p;
//
//	//check closestP3 is inside the triangle
//	FloatV v0, w0; // p = a + v*ab + w*ac;
//	barycentricCoordinates(ab, ac, FSub(closestP31, a), v0, w0);
//
//	const BoolV con00 = BAnd(FIsGrtrOrEq(v0, zero), FIsGrtrOrEq(one, v0));
//	const BoolV con01 = BAnd(FIsGrtrOrEq(w0, zero), FIsGrtrOrEq(one, w0));
//	const BoolV con02 = FIsGrtr(one, FAdd(v0, w0));
//	const BoolV con0 = BAnd(con00, BAnd(con01, con02));
//
//	
//	const Vec3V closestP41 = FSub(q, V3Scale(n, dist4));
//	const Vec3V closestP40 = q;
//
//	
//	FloatV v1, w1;
//	barycentricCoordinates(ab, ac, FSub(closestP41, a), v1, w1);
//
//	const BoolV con10 = BAnd(FIsGrtrOrEq(v1, zero), FIsGrtrOrEq(one, v1));
//	const BoolV con11 = BAnd(FIsGrtrOrEq(w1, zero), FIsGrtrOrEq(one, w1));
//	const BoolV con12 = FIsGrtr(one, FAdd(v1, w1));
//	const BoolV con1 = BAnd(con10, BAnd(con11, con12));
//
//	/*
//		p is interior point but not q
//	*/
//	const BoolV d0 = FIsGrtr(sqDistPE, sqDist3);
//	const Vec3V c00 = V3Sel(d0, closestP30, closestPE0);
//	const Vec3V c01 = V3Sel(d0, closestP31, closestPE1);
//
//	/*
//		q is interior point but not p
//	*/
//	const BoolV d1 = FIsGrtr(sqDistPE, sqDist4);
//	const Vec3V c10 = V3Sel(d1, closestP40, closestPE0);
//	const Vec3V c11 = V3Sel(d1, closestP41, closestPE1);
//
//	/*
//		p and q are interior point 
//	*/
//	const BoolV d2 = FIsGrtr(sqDist4, sqDist3);
//	const Vec3V c20 = V3Sel(d2, closestP30, closestP40);
//	const Vec3V c21 = V3Sel(d2, closestP31, closestP41);
//
//	const BoolV cond0 = BAndNot(con1, con0);
//	const BoolV cond1 = BAndNot(con0, con1);
//	const BoolV cond2 = BAnd(con0, con1);
//
//	const Vec3V closestP0 = V3Sel(cond0, c00, V3Sel(cond1, c10, V3Sel(cond2, c20, closestPE0)));
//	const Vec3V closestP1 = V3Sel(cond0, c01, V3Sel(cond1, c11, V3Sel(cond2, c21, closestPE1)));
//
//	const Vec3V vv = FSub(closestP1, closestP0);
//	closest0 = closestP0;
//	closest1 = closestP1;
//	return V3Dot(vv, vv);
//
//}

/*
	closest0 is the closest point on segment pq
	closest1 is the closest point on triangle abc
*/
Ps::aos::FloatV Gu::distanceSegmentTriangleSquared(	const Ps::aos::Vec3VArg p, const Ps::aos::Vec3VArg q,
													const Ps::aos::Vec3VArg a, const Ps::aos::Vec3VArg b, const Ps::aos::Vec3VArg c,
													Ps::aos::Vec3V& closest0, Ps::aos::Vec3V& closest1)
{
	using namespace Ps::aos;
	const FloatV zero = FZero();
	const BoolV bTrue = BTTTT();
	//const FloatV one = FOne();
	//const FloatV parallelTolerance  = FloatV_From_F32(PX_PARALLEL_TOLERANCE);

	const Vec3V pq = V3Sub(q, p);
	const Vec3V ab = V3Sub(b, a);
	const Vec3V ac = V3Sub(c, a);
	const Vec3V bc = V3Sub(c, b);
	const Vec3V ap = V3Sub(p, a);
	const Vec3V aq = V3Sub(q, a);

	//This is used to calculate the barycentric coordinate
	const FloatV d00 = V3Dot(ab,ab);
	const FloatV d01 = V3Dot(ab, ac);
	const FloatV d11 = V3Dot(ac, ac);
	const FloatV bdenom = FRecip(FSub(FMul(d00,d11), FMul(d01, d01)));

	const Vec3V n =V3Normalize(V3Cross(ab, ac)); // normalize vector

	//compute the closest point of p and triangle plane abc
	const FloatV dist3 = V3Dot(ap, n);
	const FloatV sqDist3 = FMul(dist3, dist3);


	//compute the closest point of q and triangle plane abc
	const FloatV dist4 = V3Dot(aq, n);
	const FloatV sqDist4 = FMul(dist4, dist4);
	const FloatV dMul = FMul(dist3, dist4);
	const BoolV con = FIsGrtr(zero, dMul);
	//PxU32 ww;
	//Store_From_BoolV(con, &ww);


	//if(ww) // intersect with the plane
	if(BAllEq(con, bTrue))
	{
		//compute the intersect point
		const FloatV nom = FNeg(V3Dot(n, ap));
		const FloatV denom = FRecip(V3Dot(n, pq));
		const FloatV t = FMul(nom, denom);
		const Vec3V ip = V3ScaleAdd(pq, t, p);//V3Add(p, V3Scale(pq, t));
		const Vec3V v2 = V3Sub(ip, a);
		const FloatV d20 = V3Dot(v2, ab);
		const FloatV d21 = V3Dot(v2, ac);
		const FloatV v0 = FMul(FSub(FMul(d11, d20), FMul(d01, d21)), bdenom);
		const FloatV w0 = FMul(FSub(FMul(d00, d21), FMul(d01, d20)), bdenom);
		const BoolV con0 = isValidTriangleBarycentricCoord(v0, w0);
		if(BAllEq(con0,bTrue))
		{
			closest0 = closest1 = ip;
			return zero;
		}
	}
	

	//compute the closest point between segment pq and triangle edge AB
	//FloatV t00, t01;
	//const FloatV sqDist0 = distanceSegmentSegmentSquared(p,pq,a,ab, t00, t01);

	//const Vec3V closestP00 = V3Add(p, V3Scale(pq, t00));
	//const Vec3V closestP01 = V3Add(a, V3Scale(ab, t01));

	////compute the closest point between segment pq and triangle edge BC
	//FloatV t10, t11;
	//const FloatV sqDist1 = distanceSegmentSegmentSquared(p,pq, b, bc, t10, t11);
	//const Vec3V closestP10 = V3Add(p, V3Scale(pq, t10));
	//const Vec3V closestP11 = V3Add(b, V3Scale(bc, t11));
	// 
	////compute the closest point between segment pq and triangle edge ac
	//FloatV t20, t21;
	//const FloatV sqDist2 = distanceSegmentSegmentSquared(p,pq, a, ac, t20, t21);
	//const Vec3V closestP20 = V3Add(p, V3Scale(pq, t20));
	//const Vec3V closestP21 = V3Add(a, V3Scale(ac, t21));

	Vec4V t40, t41;
	const Vec4V sqDist44 = distanceSegmentSegmentSquared4(p,pq,a,ab, b,bc, a,ac, a,ab, t40, t41);  

	const FloatV t00 = V4GetX(t40);
	const FloatV t10 = V4GetY(t40);
	const FloatV t20 = V4GetZ(t40);

	const FloatV t01 = V4GetX(t41);
	const FloatV t11 = V4GetY(t41);
	const FloatV t21 = V4GetZ(t41);

	const FloatV sqDist0(V4GetX(sqDist44));
	const FloatV sqDist1(V4GetY(sqDist44));
	const FloatV sqDist2(V4GetZ(sqDist44));

	const Vec3V closestP00 = V3ScaleAdd(pq, t00, p);
	const Vec3V closestP01 = V3ScaleAdd(ab, t01, a);

	const Vec3V closestP10 = V3ScaleAdd(pq, t10, p);
	const Vec3V closestP11 = V3ScaleAdd(bc, t11, b);

	const Vec3V closestP20 = V3ScaleAdd(pq, t20, p);
	const Vec3V closestP21 = V3ScaleAdd(ac, t21, a);


	//Get the closest point of all edges
	const BoolV con20 = FIsGrtr(sqDist1, sqDist0);
	const BoolV con21 = FIsGrtr(sqDist2, sqDist0);
	const BoolV con2 = BAnd(con20,con21);
	const BoolV con30 = FIsGrtrOrEq(sqDist0, sqDist1);
	const BoolV con31 = FIsGrtr(sqDist2, sqDist1);
	const BoolV con3 = BAnd(con30, con31);
	const FloatV sqDistPE = FSel(con2, sqDist0, FSel(con3, sqDist1, sqDist2));
	//const FloatV tValue = FSel(con2, t00, FSel(con3, t10, t20));
	const Vec3V closestPE0 =  V3Sel(con2, closestP00, V3Sel(con3, closestP10, closestP20)); // closestP on segment
	const Vec3V closestPE1 =  V3Sel(con2, closestP01, V3Sel(con3, closestP11, closestP21)); // closestP on triangle


	const Vec3V closestP31 = V3NegScaleSub(n, dist3, p);//V3Sub(p, V3Scale(n, dist3));
	const Vec3V closestP30 = p;

	//Compute the barycentric coordinate for project point of q
	const Vec3V pV20 = V3Sub(closestP31, a);
	const FloatV pD20 = V3Dot(pV20, ab);
	const FloatV pD21 = V3Dot(pV20, ac);
	const FloatV v0 = FMul(FSub(FMul(d11, pD20), FMul(d01, pD21)), bdenom);
	const FloatV w0 = FMul(FSub(FMul(d00, pD21), FMul(d01, pD20)), bdenom);

	//check closestP3 is inside the triangle
	const BoolV con0 = isValidTriangleBarycentricCoord(v0, w0);


	
	const Vec3V closestP41 = V3NegScaleSub(n, dist4, q);// V3Sub(q, V3Scale(n, dist4));
	const Vec3V closestP40 = q;

	//Compute the barycentric coordinate for project point of q
	const Vec3V qV20 = V3Sub(closestP41, a);
	const FloatV qD20 = V3Dot(qV20, ab);
	const FloatV qD21 = V3Dot(qV20, ac);
	const FloatV v1 = FMul(FSub(FMul(d11, qD20), FMul(d01, qD21)), bdenom);
	const FloatV w1 = FMul(FSub(FMul(d00, qD21), FMul(d01, qD20)), bdenom);


	const BoolV con1 = isValidTriangleBarycentricCoord(v1, w1);

	//if(BAllEq(con0, bTrue) && BAllEq(con1,bTrue))
	//{
	//	//both points are the interior points
	//	const BoolV condition = FIsGrtr(sqDist4, sqDist3);
	//	const Vec3V temp0 = V3Sel(condition, closestP30, closestP40);
	//	const Vec3V temp1 = V3Sel(condition, closestP31, closestP41);
	//	const Vec3V v = V3Sub(temp1, temp0);
	//	closest0 = temp0;
	//	closest1 = temp1;
	//	return V3Dot(v, v);
	//}

	//if(BAllEq(con0, bTrue))
	//{
	//	//p is interior point but not q
	//	const BoolV condition = FIsGrtr(sqDistPE, sqDist3);
	//	const Vec3V temp0 = V3Sel(condition, closestP30, closestPE0);
	//	const Vec3V temp1 = V3Sel(condition, closestP31, closestPE1);
	//	const Vec3V v = V3Sub(temp1, temp0);
	//	closest0 = temp0;
	//	closest1 = temp1;
	//	return V3Dot(v, v);

	//}

	//if(BAllEq(con1, bTrue))
	//{
	//	//q is interior point but not p
	//	const BoolV condition = FIsGrtr(sqDistPE, sqDist4);
	//	const Vec3V temp0 = V3Sel(condition, closestP40, closestPE0);
	//	const Vec3V temp1 = V3Sel(condition, closestP41, closestPE1);
	//	const Vec3V v = V3Sub(temp1, temp0);
	//	closest0 = temp0;
	//	closest1 = temp1;
	//	return V3Dot(v, v);
	//}

	//const Vec3V temp0 = closestPE0;
	//const Vec3V temp1 = closestPE1;
	//const Vec3V v = V3Sub(temp1, temp0);
	//closest0 = temp0;
	//closest1 = temp1;
	//return V3Dot(v, v);

	/*
		p is interior point but not q
	*/
	const BoolV d0 = FIsGrtr(sqDistPE, sqDist3);
	const Vec3V c00 = V3Sel(d0, closestP30, closestPE0);
	const Vec3V c01 = V3Sel(d0, closestP31, closestPE1);

	/*
		q is interior point but not p
	*/
	const BoolV d1 = FIsGrtr(sqDistPE, sqDist4);
	const Vec3V c10 = V3Sel(d1, closestP40, closestPE0);
	const Vec3V c11 = V3Sel(d1, closestP41, closestPE1);

	/*
		p and q are interior point 
	*/
	const BoolV d2 = FIsGrtr(sqDist4, sqDist3);
	const Vec3V c20 = V3Sel(d2, closestP30, closestP40);
	const Vec3V c21 = V3Sel(d2, closestP31, closestP41);

	const BoolV cond2 = BAnd(con0, con1);

	//const Vec3V closestP0 = V3Sel(cond0, c00, V3Sel(cond1, c10, V3Sel(cond2, c20, closestPE0)));
	//const Vec3V closestP1 = V3Sel(cond0, c01, V3Sel(cond1, c11, V3Sel(cond2, c21, closestPE1)));

	const Vec3V closestP0 = V3Sel(cond2, c20, V3Sel(con0, c00, V3Sel(con1, c10, closestPE0)));
	const Vec3V closestP1 = V3Sel(cond2, c21, V3Sel(con0, c01, V3Sel(con1, c11, closestPE1)));

	const Vec3V vv = V3Sub(closestP1, closestP0);
	closest0 = closestP0;
	closest1 = closestP1;
	return V3Dot(vv, vv);

}
