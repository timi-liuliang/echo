/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_COLLISION_H
#define APEX_COLLISION_H

#include "NxApexDefs.h"

#include "PsShare.h"
#include "foundation/PxVec3.h"
#include "foundation/PxMat33.h"

#if NX_SDK_VERSION_MAJOR == 2
class NxCapsule;
class NxBox;
#endif


namespace physx
{
namespace apex
{



struct Segment
{
	PxVec3 p0;
	PxVec3 p1;
};

struct Capsule : Segment
{
	PxF32 radius;
};

struct Triangle
{
	Triangle(const PxVec3& a, const PxVec3& b, const PxVec3& c) : v0(a), v1(b), v2(c) {}
	PxVec3 v0;
	PxVec3 v1;
	PxVec3 v2;
};

struct Box
{
	PxVec3	center;
	PxVec3	extents;
	PxMat33	rot;
};


#if NX_SDK_VERSION_MAJOR == 2
bool capsuleCapsuleIntersection(const NxCapsule& worldCaps0, const NxCapsule& worldCaps1, PxF32 tolerance = 1.2);
bool boxBoxIntersection(const NxBox& worldBox0, const NxBox& worldBox1);
PxU32 APEX_RayCapsuleIntersect(const PxVec3& origin, const PxVec3& dir, const NxCapsule& capsule, PxF32 s[2]);
#endif

bool capsuleCapsuleIntersection(const Capsule& worldCaps0, const Capsule& worldCaps1, PxF32 tolerance = 1.2);
bool boxBoxIntersection(const Box& worldBox0, const Box& worldBox1);

PxF32 APEX_pointTriangleSqrDst(const Triangle& triangle, const PxVec3& position);
PxF32 APEX_segmentSegmentSqrDist(const Segment& seg0, const Segment& seg1, PxF32* s, PxF32* t);
PxF32 APEX_pointSegmentSqrDist(const Segment& seg, const PxVec3& point, PxF32* param = 0);
PxU32 APEX_RayCapsuleIntersect(const PxVec3& origin, const PxVec3& dir, const Capsule& capsule, PxF32 s[2]);

bool APEX_RayTriangleIntersect(const PxVec3& orig, const PxVec3& dir, const PxVec3& a, const PxVec3& b, const PxVec3& c, PxF32& t, PxF32& u, PxF32& v);

} // namespace apex
} // namespace physx


#endif // APEX_COLLISION_H
