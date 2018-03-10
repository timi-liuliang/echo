/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "RTdef.h"
#if RT_COMPILE
#include "PolygonTriangulatorBase.h"
#include <PxAssert.h>

namespace physx
{
namespace fracture
{
namespace base
{

// ------ singleton pattern -----------------------------------------------------------

//static PolygonTriangulator *gPolygonTriangulator = NULL;
//
//PolygonTriangulator* PolygonTriangulator::getInstance() 
//{
//	if (gPolygonTriangulator == NULL) {
//		gPolygonTriangulator = PX_NEW(PolygonTriangulator)();
//	}
//	return gPolygonTriangulator;
//}
//
//void PolygonTriangulator::destroyInstance() 
//{
//	if (gPolygonTriangulator != NULL) {
//		PX_DELETE(gPolygonTriangulator);
//	}
//	gPolygonTriangulator = NULL;
//}

// -------------------------------------------------------------------------------------
PolygonTriangulator::PolygonTriangulator(SimScene* scene):
	mScene(scene)
{
}

// -------------------------------------------------------------------------------------
PolygonTriangulator::~PolygonTriangulator() 
{
}

// -------------------------------------------------------------------------------------
float PolygonTriangulator::cross(const PxVec3 &p0, const PxVec3 &p1)
{
	return p0.x * p1.y - p0.y * p1.x;
}

// -------------------------------------------------------------------------------------
bool PolygonTriangulator::inTriangle(const PxVec3 &p, const PxVec3 &p0, const PxVec3 &p1, const PxVec3 &p2)
{
	float d0 = cross(p1 - p0, p - p0);
	float d1 = cross(p2 - p1, p - p1);
	float d2 = cross(p0 - p2, p - p2);
	return (d0 >= 0.0f && d1 >= 0.0f && d2 >= 0.0f) ||
		(d0 <= 0.0f && d1 <= 0.0f && d2 <= 0.0f);
}

// -------------------------------------------------------------------------------------
void PolygonTriangulator::triangulate(const PxVec3 *points, int numPoints, const int *indices, PxVec3 *planeNormal)
{
	mIndices.clear();

	if (numPoints < 3)
		return;
	if (numPoints == 3) {
		if (indices == NULL) {
			mIndices.pushBack(0);
			mIndices.pushBack(1);
			mIndices.pushBack(2);
		}
		else {
			mIndices.pushBack(indices[0]);
			mIndices.pushBack(indices[1]);
			mIndices.pushBack(indices[2]);
		}
		return;
	}

	bool isConvex;
	importPoints(points, numPoints, indices, planeNormal, isConvex);

	if (isConvex) {		// fast path
		mIndices.resize(3*((physx::PxU32)numPoints-2));
		for (int i = 0; i < numPoints-2; i++) {
			mIndices[3*(physx::PxU32)i]   = 0;
			mIndices[3*(physx::PxU32)i+1] = i+1;
			mIndices[3*(physx::PxU32)i+2] = i+2;
		}
	}
	else
		clipEars();

	if (indices != NULL) {
		for (physx::PxU32 i = 0; i < mIndices.size(); i++) {
			mIndices[i] = indices[(physx::PxU32)mIndices[i]];
		}
	}
}

// -------------------------------------------------------------------------------------
void PolygonTriangulator::importPoints(const PxVec3 *points, int numPoints, const int *indices, PxVec3 *planeNormal, bool &isConvex)
{
	// find projection 3d -> 2d;
	PxVec3 n;

	isConvex = true;

	if (planeNormal) 
		n = *planeNormal;
	else {
		PX_ASSERT(numPoints >= 3);
		n = PxVec3(0.0f, 0.0f, 0.0f);

		for (int i = 1; i < numPoints-1; i++) {
			int i0 = 0;
			int i1 = i;
			int i2 = i+1;
			if (indices) {
				i0 = indices[i0];
				i1 = indices[i1];
				i2 = indices[i2];
			}
			const PxVec3 &p0 = points[i0];
			const PxVec3 &p1 = points[i1];
			const PxVec3 &p2 = points[i2];
			PxVec3 ni = (p1-p0).cross(p2-p0);
			if (i > 1 && ni.dot(n) < 0.0f)
				isConvex = false;
			n += ni;
		}
	}

	n.normalize();
	PxVec3 t0,t1;

	if (fabs(n.x) < fabs(n.y) && fabs(n.x) < fabs(n.z))
		t0 = PxVec3(1.0f, 0.0f, 0.0f);
	else if (fabs(n.y) < fabs(n.z))
		t0 = PxVec3(0.0f, 1.0f, 0.0f);
	else
		t0 = PxVec3(0.0f, 0.0f, 1.0f);
	t1 = n.cross(t0);
	t1.normalize();
	t0 = t1.cross(n);
	
	mPoints.resize((physx::PxU32)numPoints);
	if (indices == NULL) {
		for (physx::PxU32 i = 0; i < (physx::PxU32)numPoints; i++) 
			mPoints[i] = PxVec3(points[i].dot(t0), points[i].dot(t1), 0.0f);
	}
	else {
		for (physx::PxU32 i = 0; i < (physx::PxU32)numPoints; i++) {
			const PxVec3 &p = points[(physx::PxU32)indices[i]];
			mPoints[i] = PxVec3(p.dot(t0), p.dot(t1), 0.0f);
		}
	}
}

// -------------------------------------------------------------------------------------
void PolygonTriangulator::clipEars()
{
	// init
	physx::PxI32 num = (physx::PxI32)mPoints.size();

	mCorners.resize((physx::PxU32)num);

	for (int i = 0; i < num; i++) {
		Corner &c = mCorners[(physx::PxU32)i];
		c.prev = (i > 0) ? i-1 : num-1;
		c.next = (i < num-1) ? i + 1 : 0;
		c.isEar = false;
		c.angle = 0.0f;
		PxVec3 &p0 = mPoints[(physx::PxU32)c.prev];
		PxVec3 &p1 = mPoints[(physx::PxU32)i];
		PxVec3 &p2 = mPoints[(physx::PxU32)c.next];
		PxVec3 n1 = p1-p0;
		PxVec3 n2 = p2-p1;
		if (cross(n1, n2) > 0.0f) {
			n1.normalize();
			n2.normalize();
			c.angle = n1.dot(n2);

			c.isEar = true;
			int nr = (i+2) % num;
			for (int j = 0; j < num-3; j++) {
				if (inTriangle(mPoints[(physx::PxU32)nr], p0,p1,p2)) {
					c.isEar = false;
					break;
				}
				nr = (nr+1) % num;
			}
		}
	}

	int firstCorner = 0;
	int numCorners = num;

	while (numCorners > 3) {

		// find best ear
		float minAng = FLT_MAX;
		int minNr = -1;

		int nr = firstCorner;
		for (int i = 0; i < numCorners; i++) {
			Corner &c = mCorners[(physx::PxU32)nr];
			if (c.isEar && c.angle < minAng) {
				minAng = c.angle;
				minNr = nr;
			}
			nr = c.next;
		}

		// cut ear
//		assert(minNr >= 0);
if (minNr < 0)
break;
		Corner &cmin = mCorners[(physx::PxU32)minNr];
		mIndices.pushBack(cmin.prev);
		mIndices.pushBack(minNr);
		mIndices.pushBack(cmin.next);
		mCorners[(physx::PxU32)cmin.prev].next = cmin.next;
		mCorners[(physx::PxU32)cmin.next].prev = cmin.prev;

		if (firstCorner == minNr)
			firstCorner = cmin.next;
		numCorners--;
		if (numCorners == 3)
			break;

		// new ears?
		for (int i = 0; i < 2; i++) {
			physx::PxU32 i1 = physx::PxU32((i == 0) ? cmin.prev : cmin.next);
			physx::PxU32 i0 = (physx::PxU32)mCorners[i1].prev;
			physx::PxU32 i2 = (physx::PxU32)mCorners[i1].next;

			PxVec3 &p0 = mPoints[i0];
			PxVec3 &p1 = mPoints[i1];
			PxVec3 &p2 = mPoints[i2];
			PxVec3 n1 = p1-p0;
			PxVec3 n2 = p2-p1;
			if (cross(n1, n2) > 0.0f) {
				n1.normalize();
				n2.normalize();
				mCorners[i1].angle = n1.dot(n2);
				
				mCorners[i1].isEar = true;
				int nr = mCorners[i2].next;
				for (int j = 0; j < numCorners-3; j++) {
					if (inTriangle(mPoints[(physx::PxU32)nr], p0,p1,p2)) {
						mCorners[i1].isEar = false;
						break;
					}
					nr = mCorners[(physx::PxU32)nr].next;
				}
			}
		}
	}
	int id = firstCorner;
	mIndices.pushBack(id);
	id = mCorners[(physx::PxU32)id].next;
	mIndices.pushBack(id);
	id = mCorners[(physx::PxU32)id].next;
	mIndices.pushBack(id);
}

}
}
}
#endif