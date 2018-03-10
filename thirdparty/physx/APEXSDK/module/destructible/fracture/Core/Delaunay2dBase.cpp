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
#include "Delaunay2dBase.h"
#include "PxBounds3.h"
#include <PxMath.h>

namespace physx
{
namespace fracture
{
namespace base
{

// -------------------------------------------------------------------------------------
Delaunay2d::Delaunay2d(SimScene* scene):
	mScene(scene)
{
}

// -------------------------------------------------------------------------------------
Delaunay2d::~Delaunay2d() 
{
}

// -------------------------------------------------------------------------------------
void Delaunay2d::clear()
{
	mVertices.clear();
	mIndices.clear();
	mTriangles.clear();
	mFirstFarVertex = 0;

	mConvexVerts.clear();
	mConvexes.clear();
	mConvexNeighbors.clear();
}

// -------------------------------------------------------------------------------------
void Delaunay2d::triangulate(const PxVec3 *vertices, int numVerts, int byteStride, bool removeFarVertices)
{
	clear();

	PxU8 *vp = (PxU8*)vertices;
	mVertices.resize((physx::PxU32)numVerts);
	for (physx::PxU32 i = 0; i < (physx::PxU32)numVerts; i++) {
		mVertices[i] = (*(PxVec3*)vp);
		mVertices[i].z = 0.0f;
		vp += byteStride;
	}

	delaunayTriangulation();

	for (physx::PxU32 i = 0; i < mTriangles.size(); i++) {
		Triangle &t = mTriangles[i];
		if (!removeFarVertices || (t.p0 < mFirstFarVertex && t.p1 < mFirstFarVertex && t.p2 < mFirstFarVertex)) {
			mIndices.pushBack(t.p0);
			mIndices.pushBack(t.p1);
			mIndices.pushBack(t.p2);
		}
	}

	if (removeFarVertices)
		mVertices.resize((physx::PxU32)mFirstFarVertex);
}


// -------------------------------------------------------------------------------------
void Delaunay2d::delaunayTriangulation()
{
	PxBounds3 bounds;
	bounds.setEmpty();

	for (physx::PxU32 i = 0; i < mVertices.size(); i++) 
		bounds.include(mVertices[i]);

	bounds.fattenSafe(bounds.getDimensions().magnitude());

	// start with two triangles
	//float scale = 10.0f;
	PxVec3 p0(bounds.minimum.x, bounds.minimum.y, 0.0f);
	PxVec3 p1(bounds.maximum.x, bounds.minimum.y, 0.0f);
	PxVec3 p2(bounds.maximum.x, bounds.maximum.y, 0.0f);
	PxVec3 p3(bounds.minimum.x, bounds.maximum.y, 0.0f);

	mFirstFarVertex = (physx::PxI32)mVertices.size();
	mVertices.pushBack(p0);
	mVertices.pushBack(p1);
	mVertices.pushBack(p2);
	mVertices.pushBack(p3);

	mTriangles.clear();
	addTriangle(mFirstFarVertex, mFirstFarVertex+1, mFirstFarVertex+2);
	addTriangle(mFirstFarVertex, mFirstFarVertex+2, mFirstFarVertex+3);

	// insert other points
	for (physx::PxU32 i = 0; i < (physx::PxU32)mFirstFarVertex; i++) {
		mEdges.clear();
		physx::PxU32 j = 0;
		while (j < mTriangles.size()) {
			Triangle &t = mTriangles[j];
			if ((t.center - mVertices[i]).magnitudeSquared() > t.circumRadiusSquared) {
				j++;
				continue;
			}
			Edge e0(t.p0, t.p1);
			Edge e1(t.p1, t.p2);
			Edge e2(t.p2, t.p0);
			bool found0 = false;
			bool found1 = false;
			bool found2 = false;

			physx::PxU32 k = 0;
			while (k < mEdges.size()) {
				Edge &e = mEdges[k];
				bool found = false;
				if (e == e0) { found0 = true; found = true; }
				if (e == e1) { found1 = true; found = true; }
				if (e == e2) { found2 = true; found = true; }
				if (found) {
					mEdges[k] = mEdges[mEdges.size()-1];
					mEdges.popBack();
				}
				else k++;
			}
			if (!found0) mEdges.pushBack(e0);
			if (!found1) mEdges.pushBack(e1);
			if (!found2) mEdges.pushBack(e2);
			mTriangles[j] = mTriangles[mTriangles.size()-1];
			mTriangles.popBack();
		}
		for (j = 0; j < mEdges.size(); j++) {
			Edge &e = mEdges[j];
			addTriangle(e.p0, e.p1, (physx::PxI32)i);
		}
	}
}

// -------------------------------------------------------------------------------------
void Delaunay2d::addTriangle(int p0, int p1, int p2)
{
	Triangle triangle;
	triangle.p0 = p0;
	triangle.p1 = p1;
	triangle.p2 = p2;
	getCircumSphere(mVertices[(physx::PxU32)p0], mVertices[(physx::PxU32)p1], mVertices[(physx::PxU32)p2], triangle.center, triangle.circumRadiusSquared);
	mTriangles.pushBack(triangle);
}

// -------------------------------------------------------------------------------------
void Delaunay2d::getCircumSphere(const PxVec3 &p0, const PxVec3 &p1, const PxVec3 &p2,
		PxVec3 &center, float &radiusSquared)
{
	float x1 = p1.x - p0.x;
	float y1 = p1.y - p0.y;
	float x2 = p2.x - p0.x;
	float y2 = p2.y - p0.y;

	float det = x1 * y2 - x2 * y1;
	if (det == 0.0f) {
		center = p0; radiusSquared = 0.0f;
		return;
	}
	det = 0.5f / det;
	float len1 = x1*x1 + y1*y1;
	float len2 = x2*x2 + y2*y2;
	float cx = (len1 * y2 - len2 * y1) * det;
	float cy = (len2 * x1 - len1 * x2) * det;
	center.x = p0.x + cx;
	center.y = p0.y + cy;
	center.z = 0.0f;
	radiusSquared = cx * cx + cy * cy;
}

// -------------------------------------------------------------------------------------
void Delaunay2d::computeVoronoiMesh()
{
	mConvexes.clear(); 
	mConvexVerts.clear(); 
	mConvexNeighbors.clear(); 

	physx::PxU32 numVerts = mVertices.size();
	physx::PxU32 numTris = mIndices.size() / 3;

	// center positions
	shdfnd::Array<PxVec3> centers(numTris);
	for (physx::PxU32 i = 0; i < numTris; i++) {
		PxVec3 &p0 = mVertices[(physx::PxU32)mIndices[3*i]];
		PxVec3 &p1 = mVertices[(physx::PxU32)mIndices[3*i+1]];
		PxVec3 &p2 = mVertices[(physx::PxU32)mIndices[3*i+2]];
		float r2;
		getCircumSphere(p0,p1,p2, centers[i], r2);
	}

	// vertex -> triangles links
	shdfnd::Array<int> firstVertTri(numVerts+1, 0);
	shdfnd::Array<int> vertTris;

	for (physx::PxU32 i = 0; i < numTris; i++) {
		firstVertTri[(physx::PxU32)mIndices[3*i]]++;
		firstVertTri[(physx::PxU32)mIndices[3*i+1]]++;
		firstVertTri[(physx::PxU32)mIndices[3*i+2]]++;
	}

	int numLinks = 0;
	for (physx::PxU32 i = 0; i < numVerts; i++) {
		numLinks += firstVertTri[i];
		firstVertTri[i] = numLinks;
	}
	firstVertTri[numVerts] = numLinks;
	vertTris.resize((physx::PxU32)numLinks);

	for (physx::PxU32 i = 0; i < numTris; i++) {
		physx::PxU32 &i0 = (physx::PxU32&)firstVertTri[(physx::PxU32)mIndices[3*i]];
		physx::PxU32 &i1 = (physx::PxU32&)firstVertTri[(physx::PxU32)mIndices[3*i+1]];
		physx::PxU32 &i2 = (physx::PxU32&)firstVertTri[(physx::PxU32)mIndices[3*i+2]];
		i0--; vertTris[i0] = (physx::PxI32)i;
		i1--; vertTris[i1] = (physx::PxI32)i;
		i2--; vertTris[i2] = (physx::PxI32)i;
	}

	// convexes
	Convex c;
	shdfnd::Array<int> nextVert(numVerts, -1);
	shdfnd::Array<int> vertVisited(numVerts, -1);
	shdfnd::Array<int> triOfVert(numVerts, -1);
	shdfnd::Array<int> convexOfVert(numVerts, -1);

	for (physx::PxU32 i = 0; i < numVerts; i++) {
		int first = firstVertTri[i];
		int last = firstVertTri[i+1];
		int num = last - first;
		if (num < 3)
			continue;

		int start = -1;

		for (int j = first; j < last; j++) {
			int triNr = vertTris[(physx::PxU32)j];

			int k = 0;
			while (k < 3 && mIndices[physx::PxU32(3*triNr+k)] != (physx::PxI32)i)
				k++;

			int j0 = mIndices[physx::PxU32(3*triNr + (k+1)%3)];
			int j1 = mIndices[physx::PxU32(3*triNr + (k+2)%3)];

			if (j == first)
				start = j0;

			nextVert[(physx::PxU32)j0] = j1;
			vertVisited[(physx::PxU32)j0] = 2*(physx::PxI32)i;
			triOfVert[(physx::PxU32)j0] = triNr;
		}

		c.firstNeighbor = (physx::PxI32)mConvexNeighbors.size();
		c.firstVert = (physx::PxI32)mConvexVerts.size();
		c.numVerts = num;
		bool rollback = false;
		int id = start;
		do {
			if (vertVisited[(physx::PxU32)id] != 2*(physx::PxI32)i) {
				rollback = true;
				break;
			}
			vertVisited[(physx::PxU32)id] = 2*(physx::PxI32)i+1;

			mConvexVerts.pushBack(centers[(physx::PxU32)triOfVert[(physx::PxU32)id]]);
			mConvexNeighbors.pushBack(id);
			id = nextVert[(physx::PxU32)id];
		} while (id != start);

		if (rollback) {
			mConvexVerts.resize((physx::PxU32)c.firstVert);
			mConvexNeighbors.resize((physx::PxU32)c.firstNeighbor);
			continue;
		}

		c.numVerts = (physx::PxI32)mConvexVerts.size() - c.firstVert;
		c.numNeighbors = (physx::PxI32)mConvexNeighbors.size() - c.firstNeighbor;
		convexOfVert[i] = (physx::PxI32)mConvexes.size();
		mConvexes.pushBack(c);
	}

	// compute neighbors
	physx::PxU32 newPos = 0;
	for (physx::PxU32 i = 0; i < mConvexes.size(); i++) {
		Convex &c = mConvexes[i];
		physx::PxU32 pos = (physx::PxU32)c.firstNeighbor;
		physx::PxU32 num = (physx::PxU32)c.numNeighbors;
		c.firstNeighbor = (physx::PxI32)newPos;
		c.numNeighbors = 0;
		for (physx::PxU32 j = 0; j < num; j++) {
			int n = convexOfVert[(physx::PxU32)mConvexNeighbors[pos+j]];
			if (n >= 0) {
				mConvexNeighbors[newPos] = n;
				newPos++;
				c.numNeighbors++;
			}
		}
	}
	mConvexNeighbors.resize(newPos);
}

}
}
}
#endif