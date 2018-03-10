/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexIsoMesh.h"
#include "ApexCollision.h"
#include "ApexMarchingCubes.h"
#include "ApexSharedUtils.h"

#include "NiApexSDK.h"

#include "PsSort.h"

#if NX_SDK_VERSION_MAJOR == 2
#include "NxTriangle.h"
#include "NxFromPx.h"
#endif

namespace physx
{
namespace apex
{

ApexIsoMesh::ApexIsoMesh(PxU32 isoGridSubdivision, PxU32 keepNBiggestMeshes, bool discardInnerMeshes) :
	mIsoGridSubdivision(isoGridSubdivision),
	mKeepNBiggestMeshes(keepNBiggestMeshes),
	mDiscardInnerMeshes(discardInnerMeshes),
	mCellSize(0.0f),
	mThickness(0.0f),
	mOrigin(0.0f, 0.0f, 0.0f),
	mNumX(0), mNumY(0), mNumZ(0),
	mIsoValue(0.5f)
{
	if (mIsoGridSubdivision == 0)
	{
		APEX_INVALID_PARAMETER("isoGridSubdivision must be bigger than 0, setting it to 10");
		mIsoGridSubdivision = 10;
	}
	mBound.setEmpty();
}



ApexIsoMesh::~ApexIsoMesh()
{
}



void ApexIsoMesh::setBound(const PxBounds3& bound)
{
	mBound = bound;
	mCellSize = (mBound.maximum - mBound.minimum).magnitude() / mIsoGridSubdivision;
	if (mCellSize == 0.0f)
	{
		mCellSize = 1.0f;
	}
	mThickness = mCellSize * 1.5f;
	PX_ASSERT(!mBound.isEmpty());
	mBound.fattenFast(2.0f * mThickness);

	mOrigin = mBound.minimum;
	float invH = 1.0f / mCellSize;
	mNumX = (PxI32)((mBound.maximum.x - mBound.minimum.x) * invH) + 1;
	mNumY = (PxI32)((mBound.maximum.y - mBound.minimum.y) * invH) + 1;
	mNumZ = (PxI32)((mBound.maximum.z - mBound.minimum.z) * invH) + 1;
}



void ApexIsoMesh::clear()
{
	mCellSize = 0.0f;
	mThickness = 0.0f;
	mOrigin = PxVec3(0.0f);
	mNumX = mNumY = mNumZ = 0;
	mBound.setEmpty();

	clearTemp();

	mIsoVertices.clear();
	mIsoTriangles.clear();
	mIsoEdges.clear();
}



void ApexIsoMesh::clearTemp()
{
	mGrid.clear();
	mGrid.reset();
}



void ApexIsoMesh::addTriangle(const PxVec3& v0, const PxVec3& v1, const PxVec3& v2)
{
	PxI32 num = mNumX * mNumY * mNumZ;
	if (mGrid.size() != (PxU32) num)
	{
		IsoCell cell;
		cell.init();
		mGrid.resize((physx::PxU32)num, cell);
	}

	PX_ASSERT(mThickness != 0.0f);

	PxBounds3 bounds;
	bounds.minimum = bounds.maximum = v0;
	bounds.include(v1);
	bounds.include(v2);
	PX_ASSERT(!bounds.isEmpty());
	bounds.fattenFast(mThickness);

	PxF32 h = mCellSize;
	PxF32 invH = 1.0f / h;
	PxF32 invT = 1.0f / mThickness;

	PxI32 x0 = PxMax(0, (PxI32)((bounds.minimum.x - mOrigin.x) * invH));
	PxI32 x1 = PxMin(mNumX - 1, (PxI32)((bounds.maximum.x - mOrigin.x) * invH));
	PxI32 y0 = PxMax(0, (PxI32)((bounds.minimum.y - mOrigin.y) * invH));
	PxI32 y1 = PxMin(mNumY - 1, (PxI32)((bounds.maximum.y - mOrigin.y) * invH));
	PxI32 z0 = PxMax(0, (PxI32)((bounds.minimum.z - mOrigin.z) * invH));
	PxI32 z1 = PxMin(mNumZ - 1, (PxI32)((bounds.maximum.z - mOrigin.z) * invH));

	for (PxI32 xi = x0; xi <= x1; xi++)
	{
		PxVec3 pos;
		pos.x = mOrigin.x + h * xi;
		for (PxI32 yi = y0; yi <= y1; yi++)
		{
			pos.y = mOrigin.y + h * yi;
			for (PxI32 zi = z0; zi <= z1; zi++)
			{
				pos.z = mOrigin.z + h * zi;
				Triangle triangle(v0, v1, v2);
				PxF32 dist = PxSqrt(APEX_pointTriangleSqrDst(triangle, pos));
				if (dist > mThickness)
				{
					continue;
				}

				IsoCell& cell = cellAt(xi, yi, zi);
				float density = 1.0f - dist * invT;
				PX_ASSERT(density >= 0);
				cell.density = PxMax(cell.density, density); ///< \todo, is this right?
			}
		}
	}
}



bool ApexIsoMesh::update(IProgressListener* progressListener)
{
	HierarchicalProgressListener progress(100, progressListener);
	progress.setSubtaskWork(90, "Generating first mesh");

	if (generateMesh(&progress))
	{

		progress.completeSubtask();
		progress.setSubtaskWork(5, "Finding Neighbors");

		if (findNeighbors(&progress))
		{

			if (mKeepNBiggestMeshes > 0 || mDiscardInnerMeshes)
			{
				progress.completeSubtask();
				progress.setSubtaskWork(3, "Removing layers");
				removeLayers();
				//	removeSide(2);
			}
			progress.completeSubtask();
			progress.setSubtaskWork(2, "Removing Triangles and Vertices");

			removeTrisAndVerts();
		}
	}
	progress.completeSubtask();

	return true;
}



void ApexIsoMesh::getTriangle(PxU32 index, PxU32& v0, PxU32& v1, PxU32& v2) const
{
	PX_ASSERT(index < mIsoTriangles.size());
	const IsoTriangle& t = mIsoTriangles[index];
	v0 = (physx::PxU32)t.vertexNr[0];
	v1 = (physx::PxU32)t.vertexNr[1];
	v2 = (physx::PxU32)t.vertexNr[2];
}



bool ApexIsoMesh::generateMesh(IProgressListener* progressListener)
{
	mIsoVertices.clear();

	int xi, yi, zi;
	float h = mCellSize;
	PxVec3 pos, vPos;

	PxU32 maximum = (physx::PxU32)(mNumX * mNumY * mNumZ);
	PxU32 progressCounter = 0;

	HierarchicalProgressListener progress(100, progressListener);
	progress.setSubtaskWork(80, "Generating Vertices");

	// generate vertices
	for (xi = 0; xi < mNumX; xi++)
	{
		pos.x = mOrigin.x + h * xi;
		for (yi = 0; yi < mNumY; yi++)
		{
			pos.y = mOrigin.y + h * yi;
			for (zi = 0; zi < mNumZ; zi++)
			{

				if ((progressCounter++ & 0xff) == 0)
				{
					const PxI32 curr = ((xi * mNumY) + yi) * mNumZ + zi;
					const PxI32 percent = 100 * curr / (physx::PxI32)maximum;
					progress.setProgress(percent);
				}

				pos.z = mOrigin.z + h * zi;
				IsoCell& cell = cellAt(xi, yi, zi);
				float d  = cell.density;
				if (xi < mNumX - 1)
				{
					float dx = cellAt(xi + 1, yi, zi).density;
					if (interpolate(d, dx, pos, pos + PxVec3(h, 0.0f, 0.0f), vPos))
					{
						cell.vertNrX = (physx::PxI32)mIsoVertices.size();
						mIsoVertices.pushBack(vPos);
					}
				}
				if (yi < mNumY - 1)
				{
					float dy = cellAt(xi, yi + 1, zi).density;
					if (interpolate(d, dy, pos, pos + PxVec3(0.0f, h, 0.0f), vPos))
					{
						cell.vertNrY = (physx::PxI32)mIsoVertices.size();
						mIsoVertices.pushBack(vPos);
					}
				}
				if (zi < mNumZ - 1)
				{
					float dz = cellAt(xi, yi, zi + 1).density;
					if (interpolate(d, dz, pos, pos + PxVec3(0.0f, 0.0f, h), vPos))
					{
						cell.vertNrZ = (physx::PxI32)mIsoVertices.size();
						mIsoVertices.pushBack(vPos);
					}
				}
			}
		}
	}

	progress.completeSubtask();
	progress.setSubtaskWork(20, "Generating Faces");
	progressCounter = 0;

	mIsoTriangles.clear();
	// generate triangles
	int edges[12];
	IsoTriangle triangle;

	for (xi = 0; xi < mNumX - 1; xi++)
	{
		pos.x = mOrigin.x + h * xi;
		for (yi = 0; yi < mNumY - 1; yi++)
		{
			pos.y = mOrigin.y + h * yi;
			for (zi = 0; zi < mNumZ - 1; zi++)
			{

				if ((progressCounter++ & 0xff) == 0)
				{
					const PxI32 curr = ((xi * mNumY) + yi) * mNumZ + zi;
					const PxI32 percent = 100 * curr / (physx::PxI32)maximum;
					progress.setProgress(percent);
				}
				pos.z = mOrigin.z + h * zi;
				int code = 0;
				if (cellAt(xi,  yi,  zi).density > mIsoValue)
				{
					code |= 1;
				}
				if (cellAt(xi + 1, yi,  zi).density > mIsoValue)
				{
					code |= 2;
				}
				if (cellAt(xi + 1, yi + 1, zi).density > mIsoValue)
				{
					code |= 4;
				}
				if (cellAt(xi,  yi + 1, zi).density > mIsoValue)
				{
					code |= 8;
				}
				if (cellAt(xi,  yi,  zi + 1).density > mIsoValue)
				{
					code |= 16;
				}
				if (cellAt(xi + 1, yi,  zi + 1).density > mIsoValue)
				{
					code |= 32;
				}
				if (cellAt(xi + 1, yi + 1, zi + 1).density > mIsoValue)
				{
					code |= 64;
				}
				if (cellAt(xi,  yi + 1, zi + 1).density > mIsoValue)
				{
					code |= 128;
				}
				if (code == 0 || code == 255)
				{
					continue;
				}

				edges[ 0] = cellAt(xi,  yi,  zi).vertNrX;
				edges[ 1] = cellAt(xi + 1, yi,  zi).vertNrY;
				edges[ 2] = cellAt(xi,  yi + 1, zi).vertNrX;
				edges[ 3] = cellAt(xi,  yi,  zi).vertNrY;

				edges[ 4] = cellAt(xi,  yi,  zi + 1).vertNrX;
				edges[ 5] = cellAt(xi + 1, yi,  zi + 1).vertNrY;
				edges[ 6] = cellAt(xi,  yi + 1, zi + 1).vertNrX;
				edges[ 7] = cellAt(xi,  yi,  zi + 1).vertNrY;

				edges[ 8] = cellAt(xi,  yi,  zi).vertNrZ;
				edges[ 9] = cellAt(xi + 1, yi,  zi).vertNrZ;
				edges[10] = cellAt(xi + 1, yi + 1, zi).vertNrZ;
				edges[11] = cellAt(xi,  yi + 1, zi).vertNrZ;

				IsoCell& c = cellAt(xi, yi, zi);
				c.firstTriangle = (physx::PxI32)mIsoTriangles.size();

				const int* v = MarchingCubes::triTable[code];
				while (*v >= 0)
				{
					int v0 = edges[*v++];
					PX_ASSERT(v0 >= 0);
					int v1 = edges[*v++];
					PX_ASSERT(v1 >= 0);
					int v2 = edges[*v++];
					PX_ASSERT(v2 >= 0);
					triangle.set(v0, v1, v2, xi, yi, zi);
					mIsoTriangles.pushBack(triangle);
					c.numTriangles++;
				}
			}
		}
	}
	progress.completeSubtask();

	return true;
}



bool ApexIsoMesh::interpolate(float d0, float d1, const PxVec3& pos0, const PxVec3& pos1, PxVec3& pos)
{
	if ((d0 < mIsoValue && d1 < mIsoValue) || (d0 > mIsoValue && d1 > mIsoValue))
	{
		return false;
	}

	float s = (mIsoValue - d0) / (d1 - d0);
	s = PxClamp(s, 0.01f, 0.99f);	// safety not to produce vertices at the same location
	pos = pos0 * (1.0f - s) + pos1 * s;
	return true;
}



bool ApexIsoMesh::findNeighbors(IProgressListener* progress)
{
	mIsoEdges.clear();
	IsoEdge edge;

	for (PxI32 i = 0; i < (physx::PxI32)mIsoTriangles.size(); i++)
	{
		IsoTriangle& t = mIsoTriangles[(physx::PxU32)i];
		edge.set(t.vertexNr[0], t.vertexNr[1], i);
		mIsoEdges.pushBack(edge);
		edge.set(t.vertexNr[1], t.vertexNr[2], i);
		mIsoEdges.pushBack(edge);
		edge.set(t.vertexNr[2], t.vertexNr[0], i);
		mIsoEdges.pushBack(edge);
	}

	progress->setProgress(30);

	shdfnd::sort(mIsoEdges.begin(), mIsoEdges.size(), IsoEdge());

	progress->setProgress(60);

	PxU32 i = 0;
	while (i < mIsoEdges.size())
	{
		PxU32 j = i + 1;
		while (j < mIsoEdges.size() && mIsoEdges[j] == mIsoEdges[i])
		{
			j++;
		}
		if (j > i + 1)
		{
			mIsoTriangles[(physx::PxU32)mIsoEdges[i  ].triangleNr].addNeighbor(mIsoEdges[j - 1].triangleNr);
			mIsoTriangles[(physx::PxU32)mIsoEdges[j - 1].triangleNr].addNeighbor(mIsoEdges[i  ].triangleNr);
		}
		i = j;
	}

	progress->setProgress(100);
	return true;
}

struct GroupAndSize
{
	PxI32 group;
	PxU32 size;
	bool deleteMesh;

	bool operator()(const GroupAndSize& a, const GroupAndSize& b) const
	{
		return a.size > b.size;
	}
};

struct TimeAndDot
{
	PxF32 time;
	PxF32 dot;

	bool operator()(const TimeAndDot& a, const TimeAndDot& b) const
	{
		return a.time > b.time;
	}
};

void ApexIsoMesh::removeLayers()
{
	// mark regions
	shdfnd::Array<GroupAndSize> triangleGroups;

	for (PxU32 i = 0; i < mIsoTriangles.size(); i++)
	{
		if (mIsoTriangles[i].groupNr >= 0)
		{
			continue;
		}

		GroupAndSize gas;
		gas.size = 0;
		gas.group = (physx::PxI32)triangleGroups.size();
		gas.deleteMesh = false;

		gas.size = floodFill(i, (physx::PxU32)gas.group);

		triangleGroups.pushBack(gas);
	}

	if (triangleGroups.size() == 0)
	{
		return;
	}

	// clear regions
	if (mDiscardInnerMeshes)
	{
		shdfnd::Array<TimeAndDot> raycastHits;
		for (PxU32 group = 0; group < triangleGroups.size(); group++)
		{
			// see if this group is an inner mesh
			raycastHits.clear();

			const PxI32 groupNumber = triangleGroups[group].group;

			PxU32 start = 0;
			const PxU32 numTriangles = mIsoTriangles.size();
			for (PxU32 i = 0; i < numTriangles; i++)
			{
				if (mIsoTriangles[i].groupNr == groupNumber)
				{
					start = i;
					break;
				}
			}

			// raycast from the start triangle
			const PxVec3 rayOrig =   (mIsoVertices[(physx::PxU32)mIsoTriangles[start].vertexNr[0]] 
									+ mIsoVertices[(physx::PxU32)mIsoTriangles[start].vertexNr[1]] 
									+ mIsoVertices[(physx::PxU32)mIsoTriangles[start].vertexNr[2]]) / 3.0f;
			PxVec3 rayDir(1.0f, 1.0f, 1.0f);
			rayDir.normalize(); // can really be anything.

			// Find all ray hits
			for (PxU32 i = start; i < numTriangles; i++)
			{
				if (mIsoTriangles[i].groupNr != groupNumber)
				{
					continue;
				}

				PxF32 t, u, v;
				PxVec3 verts[3] =
				{
					mIsoVertices[(physx::PxU32)mIsoTriangles[i].vertexNr[0]],
					mIsoVertices[(physx::PxU32)mIsoTriangles[i].vertexNr[1]],
					mIsoVertices[(physx::PxU32)mIsoTriangles[i].vertexNr[2]],
				};

				if (APEX_RayTriangleIntersect(rayOrig, rayDir, verts[0], verts[1], verts[2], t, u, v))
				{
					TimeAndDot hit;
					hit.time = t;

					PxVec3 faceNormal = (verts[1] - verts[0]).cross(verts[2] - verts[0]);
					faceNormal.normalize();

					hit.dot = faceNormal.dot(rayDir);

					raycastHits.pushBack(hit);
				}
			}

			if (raycastHits.size() > 0)
			{
				shdfnd::sort(raycastHits.begin(), raycastHits.size(), TimeAndDot());

				triangleGroups[group].deleteMesh = raycastHits[0].dot > 0.0f;
			}
		}
	}

	if (mKeepNBiggestMeshes > 0)
	{
		shdfnd::sort(triangleGroups.begin(), triangleGroups.size(), GroupAndSize());

		for (PxU32 i = mKeepNBiggestMeshes; i < triangleGroups.size(); i++)
		{
			triangleGroups[i].deleteMesh = true;
		}
	}


	for (PxU32 i = 0; i < triangleGroups.size(); i++)
	{
		if (!triangleGroups[i].deleteMesh)
		{
			continue;
		}

		const PxI32 group = triangleGroups[i].group;

		const PxU32 size = mIsoTriangles.size();
		for (PxU32 j = 0; j < size; j++)
		{
			if (mIsoTriangles[j].groupNr == group)
			{
				mIsoTriangles[j].deleted = true;
			}
		}
	}
}



PxU32 ApexIsoMesh::floodFill(PxU32 triangleNr, PxU32 groupNr)
{
	PxU32 numTriangles = 0;
	shdfnd::Array<PxU32> queue;
	queue.pushBack(triangleNr);

	while (!queue.empty())
	{
		IsoTriangle& t = mIsoTriangles[queue.back()];
		queue.popBack();
		if (t.groupNr == PxI32(groupNr))
		{
			continue;
		}

		t.groupNr = (physx::PxI32)groupNr;
		numTriangles++;

		PxI32 adj = t.adjTriangles[0];
		if (adj >= 0 && mIsoTriangles[(physx::PxU32)adj].groupNr < 0)
		{
			queue.pushBack((physx::PxU32)adj);
		}
		adj = t.adjTriangles[1];
		if (adj >= 0 && mIsoTriangles[(physx::PxU32)adj].groupNr < 0)
		{
			queue.pushBack((physx::PxU32)adj);
		}
		adj = t.adjTriangles[2];
		if (adj >= 0 && mIsoTriangles[(physx::PxU32)adj].groupNr < 0)
		{
			queue.pushBack((physx::PxU32)adj);
		}
	}

	return numTriangles;
}



void ApexIsoMesh::removeTrisAndVerts()
{
	for (PxI32 i = (physx::PxI32)mIsoTriangles.size() - 1; i >= 0; i--)
	{
		if (mIsoTriangles[(physx::PxU32)i].deleted)
		{
			mIsoTriangles.replaceWithLast((physx::PxU32)i);
		}
	}

	Array<PxI32> oldToNew;
	Array<PxVec3> newVertices;

	oldToNew.resize(mIsoVertices.size(), -1);

	for (PxU32 i = 0; i < mIsoTriangles.size(); i++)
	{
		IsoTriangle& t = mIsoTriangles[i];
		for (PxU32 j = 0; j < 3; j++)
		{
			PxU32 vNr = (physx::PxU32)t.vertexNr[j];
			if (oldToNew[vNr] < 0)
			{
				oldToNew[vNr] = (physx::PxI32)newVertices.size();
				newVertices.pushBack(mIsoVertices[vNr]);
			}
			t.vertexNr[j] = oldToNew[vNr];
		}
	}

	mIsoVertices.clear();
	mIsoVertices.reserve(newVertices.size());

	for (PxU32 i = 0; i < newVertices.size(); i++)
	{
		mIsoVertices.pushBack(newVertices[i]);
	}
}

} // end namespace apex
} // end namespace physx
