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
#include "GuBoxConversion.h"
#include "GuCapsule.h"
#include "GuGeomUtilsInternal.h"
#include "CmMatrix34.h"

using namespace physx;

static PX_FORCE_INLINE void computeBasis2(const PxVec3& dir, PxVec3& right, PxVec3& up)
{
	PxU32 Coord = Ps::closestAxis(dir);
	if(Coord==0)
	{
		// P = (0,0,1)
		right.x = - dir.y;
		right.y = dir.x;
		right.z = 0.0f;

		up.x = - dir.z * dir.x;
		up.y = - dir.z * dir.y;
		up.z = dir.x * dir.x + dir.y * dir.y;
	}
	else if(Coord==1)
	{
		// P = (1,0,0)
		right.x = 0.0f;
		right.y = - dir.z;
		right.z = dir.y;

		up.x = dir.y * dir.y + dir.z * dir.z;
		up.y = - dir.x * dir.y;
		up.z = - dir.x * dir.z;
	}
	else //if(dir.closestAxis()==2)
	{
		// P = (0,1,0)
		right.x = dir.z;
		right.y = 0.0f;
		right.z = - dir.x;

		up.x = - dir.y * dir.x;
		up.y = dir.z * dir.z + dir.x * dir.x;
		up.z = - dir.y * dir.z;
	}
right.normalize();	// ### added after above fix, to do better
}


void Gu::Box::create(const Gu::Capsule& capsule)
{
	// Box center = center of the two LSS's endpoints
	center = capsule.computeCenter();

	PxVec3 dir = capsule.p1 - capsule.p0;
	const float d = dir.magnitude();
	rot.column0 = dir / d;

	// Box extents
	extents.x = capsule.radius + (d * 0.5f);
	extents.y = capsule.radius;
	extents.z = capsule.radius;

	// Box orientation
	computeBasis2(rot.column0, rot.column1, rot.column2);
}


/**
Returns edges.
\return		24 indices (12 edges) indexing the list returned by ComputePoints()
*/
const PxU8* Gu::getBoxEdges()
{
	//     7+------+6			0 = ---
	//     /|     /|			1 = +--
	//    / |    / |			2 = ++-
	//   / 4+---/--+5			3 = -+-
	// 3+------+2 /    y   z	4 = --+
	//  | /    | /     |  /		5 = +-+
	//  |/     |/      |/		6 = +++
	// 0+------+1      *---x	7 = -++

	static PxU8 Indices[] = {
		0, 1,	1, 2,	2, 3,	3, 0,
		7, 6,	6, 5,	5, 4,	4, 7,
		1, 5,	6, 2,
		3, 7,	4, 0
	};
	return Indices;
}


void Gu::computeOBBPoints(PxVec3* PX_RESTRICT pts, const PxVec3& center, const PxVec3& extents, const PxVec3& base0, const PxVec3& base1, const PxVec3& base2)
{
	PX_ASSERT(pts);

	// "Rotated extents"
	const PxVec3 axis0 = base0 * extents.x;
	const PxVec3 axis1 = base1 * extents.y;
	const PxVec3 axis2 = base2 * extents.z;

	//     7+------+6			0 = ---
	//     /|     /|			1 = +--
	//    / |    / |			2 = ++-
	//   / 4+---/--+5			3 = -+-
	// 3+------+2 /    y   z	4 = --+
	//  | /    | /     |  /		5 = +-+
	//  |/     |/      |/		6 = +++
	// 0+------+1      *---x	7 = -++

	// Original code: 24 vector ops
	/*	pts[0] = box.center - Axis0 - Axis1 - Axis2;
	pts[1] = box.center + Axis0 - Axis1 - Axis2;
	pts[2] = box.center + Axis0 + Axis1 - Axis2;
	pts[3] = box.center - Axis0 + Axis1 - Axis2;
	pts[4] = box.center - Axis0 - Axis1 + Axis2;
	pts[5] = box.center + Axis0 - Axis1 + Axis2;
	pts[6] = box.center + Axis0 + Axis1 + Axis2;
	pts[7] = box.center - Axis0 + Axis1 + Axis2;*/

	// Rewritten: 12 vector ops
	pts[0] = pts[3] = pts[4] = pts[7] = center - axis0;
	pts[1] = pts[2] = pts[5] = pts[6] = center + axis0;

	PxVec3 tmp = axis1 + axis2;
	pts[0] -= tmp;
	pts[1] -= tmp;
	pts[6] += tmp;
	pts[7] += tmp;

	tmp = axis1 - axis2;
	pts[2] += tmp;
	pts[3] += tmp;
	pts[4] -= tmp;
	pts[5] -= tmp;
}

