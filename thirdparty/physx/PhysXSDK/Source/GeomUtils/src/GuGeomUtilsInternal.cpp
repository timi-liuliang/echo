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
#include "GuGeomUtilsInternal.h"
#include "GuVecPlane.h"
#include "PsMathUtils.h"
#include "PxCapsuleGeometry.h"

using namespace physx;

// ----------------------------------------------------------------------------------------

/**
Computes the aabb points.
\param		pts	[out] 8 box points
*/
void Gu::computeBoxPoints(const PxBounds3& bounds, PxVec3* PX_RESTRICT pts)
{
	PX_ASSERT(pts);

	// Get box corners
	const PxVec3& minimum = bounds.minimum;
	const PxVec3& maximum = bounds.maximum;

	//     7+------+6			0 = ---
	//     /|     /|			1 = +--
	//    / |    / |			2 = ++-
	//   / 4+---/--+5			3 = -+-
	// 3+------+2 /    y   z	4 = --+
	//  | /    | /     |  /		5 = +-+
	//  |/     |/      |/		6 = +++
	// 0+------+1      *---x	7 = -++

	// Generate 8 corners of the bbox
	pts[0] = PxVec3(minimum.x, minimum.y, minimum.z);
	pts[1] = PxVec3(maximum.x, minimum.y, minimum.z);
	pts[2] = PxVec3(maximum.x, maximum.y, minimum.z);
	pts[3] = PxVec3(minimum.x, maximum.y, minimum.z);
	pts[4] = PxVec3(minimum.x, minimum.y, maximum.z);
	pts[5] = PxVec3(maximum.x, minimum.y, maximum.z);
	pts[6] = PxVec3(maximum.x, maximum.y, maximum.z);
	pts[7] = PxVec3(minimum.x, maximum.y, maximum.z);
}

//---------------------------------------------------------------------------

PxPlane Gu::getPlane(const PxTransform& pose)
{ 
	const PxVec3 n = pose.q.getBasisVector0();
	return PxPlane(n, -pose.p.dot(n)); 
}

PxTransform Gu::getCapsuleTransform(const Gu::Capsule& capsule, PxReal& halfHeight)
{
	PxTransform capsuleTransform;
	PxVec3 dir = capsule.computeDirection();
	capsuleTransform.p = capsule.getOrigin() + dir * 0.5f;
	halfHeight = 0.5f * dir.normalize();

	if (halfHeight > PX_EPS_F32)	//else it is just a sphere.
	{
	//angle axis representation is the rotation from the world x axis to dir
		PxVec3 t1, t2;
		Ps::normalToTangents(dir, t1, t2);

		const PxMat33 x(dir, t1, t2);
		capsuleTransform.q = PxQuat(x);
	}
	else
	{
		capsuleTransform.q = PxQuat(PxIdentity);
	}
	//this won't ever be exactly the same thing as the original because we lost some info on a DOF

	return capsuleTransform;

	// Instead we could do the following:
	//
	//if (halfHeight > PX_EPS_F32)	//else it is just a sphere.
	//{
	//	PxVec3 axis = dir.cross(PxVec3(1.0f, 0.0f, 0.0f));
	//	PxReal sinTheta = axis.normalize();
	//	PxReal cosTheta = dir.x;	//dir|PxVec3(1.0f, 0.0f, 0.0f)
	//
	//	To build the quaternion we need sin(a/2), cos(a/2)
	//  The formulas are:
	//
	//	sin(a/2) = +/- sqrt([1 - cos(a)] / 2)
	//	cos(a/2) = +/- sqrt([1 + cos(a)] / 2)
	//
	// The problem is to find the correct signs based on the spatial relation between
	// the capsule direction and (1,0,0)
	// Not sure this is more efficient and anyway, we want to ditch this method as soon as possible
	//
	//	quat.x = axis.x * sin(a/2)
	//	quat.y = axis.y * sin(a/2)
	//	quat.z = axis.z * sin(a/2)
	//	quat.w = cos(a/2)
}

void Gu::computeBoundsAroundVertices(PxBounds3& bounds, PxU32 nbVerts, const PxVec3* PX_RESTRICT verts)
{
	bounds.setEmpty();
	while(nbVerts--)
		bounds.include(*verts++);
}

void Gu::computeSweptBox(Gu::Box& dest, const PxVec3& extents, const PxVec3& center, const PxMat33& rot, const PxVec3& unitDir, const PxReal distance)
{
	PxVec3 R1, R2;
	Ps::computeBasis(unitDir, R1, R2);

	PxReal dd[3];
	dd[0] = PxAbs(rot.column0.dot(unitDir));
	dd[1] = PxAbs(rot.column1.dot(unitDir));
	dd[2] = PxAbs(rot.column2.dot(unitDir));
	PxReal dmax = dd[0];
	PxU32 ax0=1;
	PxU32 ax1=2;
	if(dd[1]>dmax)
	{
		dmax=dd[1];
		ax0=0;
		ax1=2;
	}
	if(dd[2]>dmax)
	{
		dmax=dd[2];
		ax0=0;
		ax1=1;
	}
	if(dd[ax1]<dd[ax0])
		Ps::swap(ax0, ax1);

	R1 = rot[ax0];
	R1 -= (R1.dot(unitDir))*unitDir;	// Project to plane whose normal is dir
	R1.normalize();
	R2 = unitDir.cross(R1);

	dest.setAxes(unitDir, R1, R2);

	PxReal offset[3];
	offset[0] = distance;
	offset[1] = distance*(unitDir.dot(R1));
	offset[2] = distance*(unitDir.dot(R2));

	for(PxU32 r=0; r<3; r++)
	{
		const PxVec3& R = dest.rot[r];
		dest.extents[r] = offset[r]*0.5f + PxAbs(rot.column0.dot(R))*extents.x + PxAbs(rot.column1.dot(R))*extents.y + PxAbs(rot.column2.dot(R))*extents.z;
	}

	dest.center = center + unitDir*distance*0.5f;
}
