/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexGeneralizedMarchingCubes.h"

#include "ApexGeneralizedCubeTemplates.h"

#include "foundation/PxMath.h"

#include "ApexSharedUtils.h"

#include "PsSort.h"

namespace physx
{
namespace apex
{

struct TriangleEdge
{
	void init(physx::PxI32 v0, physx::PxI32 v1, physx::PxI32 edgeNr, physx::PxI32 triNr)
	{
		this->v0 = physx::PxMin(v0, v1);
		this->v1 = physx::PxMax(v0, v1);
		this->edgeNr = edgeNr;
		this->triNr = triNr;
	}
	bool operator == (const TriangleEdge& e) const
	{
		if (v0 == e.v0 && v1 == e.v1)
		{
			return true;
		}
		if (v0 == e.v1 && v1 == e.v0)
		{
			return true;
		}
		return false;
	}
	bool operator()(const TriangleEdge& a, const TriangleEdge& b) const
	{
		if (a.v0 < b.v0)
		{
			return true;
		}
		if (a.v0 > b.v0)
		{
			return false;
		}
		if (a.v1 < b.v1)
		{
			return true;
		}
		if (a.v1 > b.v1)
		{
			return false;
		}
		return a.triNr < b.triNr;
	}
	physx::PxI32 v0, v1;
	physx::PxI32 edgeNr;
	physx::PxI32 triNr;
};



struct BorderEdge
{
	void set(int vertNr, int prev = -1, int depth = 0)
	{
		this->vertNr = vertNr;
		this->prev = prev;
		this->depth = depth;
	}
	physx::PxI32 vertNr;
	physx::PxI32 prev;
	physx::PxI32 depth;
};




ApexGeneralizedMarchingCubes::ApexGeneralizedMarchingCubes(const physx::PxBounds3& bound, physx::PxU32 subdivision) :
	mTemplates(NULL)
{
	mCubes.clear();
	mBound = bound;
	PX_ASSERT(subdivision > 0);
	mSpacing = (bound.maximum - bound.minimum).magnitude() / (physx::PxF32)subdivision;
	mInvSpacing = 1.0f / mSpacing;

	memset(mFirstCube, -1, sizeof(physx::PxI32) * HASH_INDEX_SIZE);

	mVertices.clear();
	mIndices.clear();
}


ApexGeneralizedMarchingCubes::~ApexGeneralizedMarchingCubes()
{
	if (mTemplates != NULL)
	{
		delete mTemplates;
		mTemplates = NULL;
	}
}





void ApexGeneralizedMarchingCubes::registerTriangle(const physx::PxVec3& p0, const physx::PxVec3& p1, const physx::PxVec3& p2)
{
	physx::PxBounds3 bounds;
	bounds.setEmpty();
	bounds.include(p0);
	bounds.include(p1);
	bounds.include(p2);
	PX_ASSERT(!bounds.isEmpty());
	bounds.fattenFast(0.001f * mSpacing);
	physx::PxVec3 n, vertPos, q0, q1, qt;
	n = (p1 - p0).cross(p2 - p0);
	float d = n.dot(p0);

	physx::PxI32 min[3] = { (physx::PxI32)PxFloor(bounds.minimum.x* mInvSpacing), (physx::PxI32)PxFloor(bounds.minimum.y* mInvSpacing), (physx::PxI32)PxFloor(bounds.minimum.z* mInvSpacing) };
	physx::PxI32 max[3] = { (physx::PxI32)PxFloor(bounds.maximum.x* mInvSpacing) + 1, (physx::PxI32)PxFloor(bounds.maximum.y* mInvSpacing) + 1, (physx::PxI32)PxFloor(bounds.maximum.z* mInvSpacing) + 1 };

	physx::PxI32 coord[3];

	for (physx::PxU32 dim0 = 0; dim0 < 3; dim0++)
	{
		if (n[dim0] == 0.0f)
		{
			continue;    // singular case
		}

		const physx::PxU32 dim1 = (dim0 + 1) % 3;
		const physx::PxU32 dim2 = (dim1 + 1) % 3;

		for (coord[dim1] = min[dim1]; coord[dim1] <= max[dim1]; coord[dim1]++)
		{
			for (coord[dim2] = min[dim2]; coord[dim2] <= max[dim2]; coord[dim2]++)
			{
				//const physx::PxF32 axis0 = coord[dim0] * mSpacing;
				const physx::PxF32 axis1 = coord[dim1] * mSpacing;
				const physx::PxF32 axis2 = coord[dim2] * mSpacing;

				// does the ray go through the triangle?
				bool intersection = true;
				if (n[dim0] > 0.0f)
				{
					if ((p1[dim1] - p0[dim1]) * (axis2 - p0[dim2]) - (axis1 - p0[dim1]) * (p1[dim2] - p0[dim2]) < 0.0f)
					{
						intersection = false;
					}
					if ((p2[dim1] - p1[dim1]) * (axis2 - p1[dim2]) - (axis1 - p1[dim1]) * (p2[dim2] - p1[dim2]) < 0.0f)
					{
						intersection = false;
					}
					if ((p0[dim1] - p2[dim1]) * (axis2 - p2[dim2]) - (axis1 - p2[dim1]) * (p0[dim2] - p2[dim2]) < 0.0f)
					{
						intersection = false;
					}
				}
				else
				{
					if ((p1[dim1] - p0[dim1]) * (axis2 - p0[dim2]) - (axis1 - p0[dim1]) * (p1[dim2] - p0[dim2]) > 0.0f)
					{
						intersection = false;
					}
					if ((p2[dim1] - p1[dim1]) * (axis2 - p1[dim2]) - (axis1 - p1[dim1]) * (p2[dim2] - p1[dim2]) > 0.0f)
					{
						intersection = false;
					}
					if ((p0[dim1] - p2[dim1]) * (axis2 - p2[dim2]) - (axis1 - p2[dim1]) * (p0[dim2] - p2[dim2]) > 0.0f)
					{
						intersection = false;
					}
				}

				if (intersection)
				{
					const physx::PxF32 pos = (d - axis1 * n[dim1] - axis2 * n[dim2]) / n[dim0];
					coord[dim0] = (physx::PxI32)PxFloor(pos * mInvSpacing);

					physx::PxU32 nr = (physx::PxU32)createCube(coord[0], coord[1], coord[2]);
					GeneralizedCube& cube = mCubes[nr];

					if (cube.vertRefs[dim0].vertNr < 0)
					{
						vertPos = physx::PxVec3(coord[0] * mSpacing, coord[1] * mSpacing, coord[2] * mSpacing);
						vertPos[dim0] = pos;
						cube.vertRefs[dim0].vertNr = (physx::PxI32)mVertices.size();
						mVertices.pushBack(vertPos);
					}
				}
			}
		}
	}

	// does a triangle edge cut a cube face?
	physx::PxBounds3 cellBounds;
	for (physx::PxI32 xi = min[0]; xi <= max[0]; xi++)
	{
		for (physx::PxI32 yi = min[1]; yi <= max[1]; yi++)
		{
			for (physx::PxI32 zi = min[2]; zi <= max[2]; zi++)
			{
				cellBounds.minimum = physx::PxVec3(xi * mSpacing, yi * mSpacing, zi * mSpacing);
				cellBounds.maximum = physx::PxVec3((xi + 1) * mSpacing, (yi + 1) * mSpacing, (zi + 1) * mSpacing);

				for (physx::PxU32 i = 0; i < 3; i++)
				{
					switch (i)
					{
					case 0 :
						q0 = p0;
						q1 = p1;
						break;
					case 1 :
						q0 = p1;
						q1 = p2;
						break;
					case 2 :
						q0 = p2;
						q1 = p0;
						break;
					default: // Make compiler happy
						q0 = q1 = physx::PxVec3(0.0f);
						break;
					}

					if (q0.x != q1.x)
					{
						const physx::PxF32 t = (cellBounds.minimum.x - q0.x) / (q1.x - q0.x);
						if (0.0f <= t && t <= 1.0f)
						{
							qt = q0 + (q1 - q0) * t;
							if (cellBounds.minimum.y <= qt.y && qt.y <= cellBounds.maximum.y && cellBounds.minimum.z <= qt.z && qt.z <= cellBounds.maximum.z)
							{
								GeneralizedCube& cube = mCubes[(physx::PxU32)createCube(xi, yi, zi)];
								cube.sideBounds[0].include(qt);
							}
						}
					}
					if (q0.y != q1.y)
					{
						const physx::PxF32 t = (cellBounds.minimum.y - q0.y) / (q1.y - q0.y);
						if (0.0f <= t && t <= 1.0f)
						{
							qt = q0 + (q1 - q0) * t;
							if (cellBounds.minimum.z <= qt.z && qt.z <= cellBounds.maximum.z && cellBounds.minimum.x <= qt.x && qt.x <= cellBounds.maximum.x)
							{
								GeneralizedCube& cube = mCubes[(physx::PxU32)createCube(xi, yi, zi)];
								cube.sideBounds[1].include(qt);
							}
						}
					}
					if (q0.z != q1.z)
					{
						const physx::PxF32 t = (cellBounds.minimum.z - q0.z) / (q1.z - q0.z);
						if (0.0f <= t && t <= 1.0f)
						{
							qt = q0 + (q1 - q0) * t;
							if (cellBounds.minimum.x <= qt.x && qt.x <= cellBounds.maximum.x && cellBounds.minimum.y <= qt.y && qt.y <= cellBounds.maximum.y)
							{
								GeneralizedCube& cube = mCubes[(physx::PxU32)createCube(xi, yi, zi)];
								cube.sideBounds[2].include(qt);
							}
						}
					}
				}
			}
		}
	}
}



bool ApexGeneralizedMarchingCubes::endRegistration(physx::PxU32 bubleSizeToRemove, IProgressListener* progressListener)
{
	HierarchicalProgressListener progress(100, progressListener);

	progress.setSubtaskWork(20, "Complete Cells");
	completeCells();
	progress.completeSubtask();

	progress.setSubtaskWork(20, "Create Triangles");
	for (int i = 0; i < (int)mCubes.size(); i++)
	{
		createTrianglesForCube(i);
	}
	progress.completeSubtask();

	progress.setSubtaskWork(20, "Create Neighbor Info");
	createNeighbourInfo();
	progress.completeSubtask();

	physx::PxU32 numTris = mIndices.size() / 3;

	mTriangleDeleted.resize(numTris);
	for (physx::PxU32 i = 0; i < numTris; i++)
	{
		mTriangleDeleted[i] = 0;
	}

	if (bubleSizeToRemove > 0)
	{
		progress.setSubtaskWork(10, "Remove Bubbles");
		determineGroups();
		removeBubbles((physx::PxI32)bubleSizeToRemove);
		progress.completeSubtask();
	}
	progress.setSubtaskWork(20, "Fix Orientation");
	determineGroups();
	fixOrientations();
	progress.completeSubtask();

	progress.setSubtaskWork(-1, "Compress");
	compress();
	progress.completeSubtask();

	return true;
}


physx::PxI32 ApexGeneralizedMarchingCubes::createCube(physx::PxI32 xi, physx::PxI32 yi, physx::PxI32 zi)
{
	physx::PxI32 nr = findCube(xi, yi, zi);
	if (nr >= 0)
	{
		return nr;
	}

	GeneralizedCube newCube;
	newCube.init(xi, yi, zi);
	physx::PxI32 h = hashFunction(xi, yi, zi);
	newCube.next = mFirstCube[h];
	mFirstCube[h] = (physx::PxI32)mCubes.size();
	nr = (physx::PxI32)mCubes.size();
	mCubes.pushBack(newCube);
	return nr;
}



physx::PxI32 ApexGeneralizedMarchingCubes::findCube(physx::PxI32 xi, physx::PxI32 yi, physx::PxI32 zi)
{
	physx::PxI32 h = hashFunction(xi, yi, zi);
	physx::PxI32 i = mFirstCube[h];

	while (i >= 0)
	{
		GeneralizedCube& c = mCubes[(physx::PxU32)i];
		if (!c.deleted && c.xi == xi && c.yi == yi && c.zi == zi)
		{
			return i;
		}
		i = mCubes[(physx::PxU32)i].next;
	}
	return -1;
}



void ApexGeneralizedMarchingCubes::completeCells()
{
	// make sure we have the boarder cells as well
	physx::PxU32 numCubes = mCubes.size();
	for (physx::PxU32 i = 0; i < numCubes; i++)
	{
		physx::PxI32 xi = mCubes[i].xi;
		physx::PxI32 yi = mCubes[i].yi;
		physx::PxI32 zi = mCubes[i].zi;
		//createCube(xi-1,yi,  zi);
		//createCube(xi,  yi-1,zi);
		//createCube(xi,  yi,  zi-1);

		createCube(xi - 1, yi,   zi);
		createCube(xi - 1, yi - 1, zi);
		createCube(xi,   yi - 1, zi);
		createCube(xi,   yi,   zi - 1);
		createCube(xi - 1, yi,   zi - 1);
		createCube(xi - 1, yi - 1, zi - 1);
		createCube(xi,   yi - 1, zi - 1);
	}
}



void ApexGeneralizedMarchingCubes::createTrianglesForCube(physx::PxI32 cellNr)
{
	const int sideEdges[6][4] =
	{
		{3, 7, 8, 11}, {1, 5, 9, 10}, {0, 4, 8, 9}, {2, 6, 10, 11}, {0, 1, 2, 3}, {4, 5, 6, 7}
	};
	const int adjVerts[12][8] =
	{
		{16, 1, 2, 3, 14, 4, 8, 9}, {16, 0, 2, 3, 13, 5, 9, 10}, {16, 0, 1, 3, 15, 6, 10, 11}, {16, 0, 1, 2, 12, 7, 8, 11},
		{17, 5, 6, 7, 14, 0, 8, 9}, {17, 4, 6, 7, 13, 1, 9, 10}, {17, 4, 5, 7, 15, 2, 10, 11}, {17, 4, 5, 6, 12, 3, 8, 11},
		{12, 3, 7, 11, 14, 0, 4, 9}, {14, 0, 4, 8, 13, 1, 5, 10}, {13, 1, 5, 9, 15, 2, 6, 11}, {15, 2, 6, 10, 12, 3, 7, 8}
	};

	physx::PxI32 groups[8];
	physx::PxI32 vertNrs[19];

	memset(vertNrs, -1, sizeof(physx::PxI32) * 19);

	// get edge vertices
	GeneralizedVertRef* vertRefs[12];
	getCubeEdgesAndGroups(cellNr, vertRefs, groups);

	physx::PxU32 numCuts = 0;
	for (physx::PxU32 i = 0; i < 12; i++)
	{
		if (vertRefs[i] != NULL)
		{
			vertNrs[i] = vertRefs[i]->vertNr;
			if (vertNrs[i] >= 0)
			{
				numCuts++;
			}
		}
	}

	if (numCuts == 0)
	{
		return;
	}

	GeneralizedCube& c = mCubes[(physx::PxU32)cellNr];

	int startFace = -1, endFace = -1;

	PX_UNUSED(endFace); // Make compiler happy

	// create side vertices if necessary
	for (physx::PxU32 i = 0; i < 6; i++)
	{
		physx::PxU32 faceNr = 12 + i;
		physx::PxI32* faceVertNr = NULL;
		physx::PxBounds3* b = NULL;
		switch (faceNr)
		{
		case 12:
			faceVertNr = &c.sideVertexNr[0];
			b = &c.sideBounds[0];
			break;
		case 13:
		{
			physx::PxI32 nr = findCube(c.xi + 1, c.yi, c.zi);
			if (nr >= 0)
			{
				faceVertNr = &mCubes[(physx::PxU32)nr].sideVertexNr[0];
				b = &mCubes[(physx::PxU32)nr].sideBounds[0];
			}
		}
		break;
		case 14:
			faceVertNr = &c.sideVertexNr[1];
			b = &c.sideBounds[1];
			break;
		case 15:
		{
			physx::PxI32 nr = findCube(c.xi, c.yi + 1, c.zi);
			if (nr >= 0)
			{
				faceVertNr = &mCubes[(physx::PxU32)nr].sideVertexNr[1];
				b = &mCubes[(physx::PxU32)nr].sideBounds[1];
			}
		}
		break;
		case 16:
			faceVertNr = &c.sideVertexNr[2];
			b = &c.sideBounds[2];
			break;
		case 17:
		{
			physx::PxI32 nr = findCube(c.xi, c.yi, c.zi + 1);
			if (nr >= 0)
			{
				faceVertNr = &mCubes[(physx::PxU32)nr].sideVertexNr[2];
				b = &mCubes[(physx::PxU32)nr].sideBounds[2];
			}
		}
		break;
		}

		physx::PxVec3 pos(0.0f, 0.0f, 0.0f);
		physx::PxU32 num = 0;

		for (physx::PxU32 j = 0; j < 4; j++)
		{
			physx::PxI32 edgeVertNr = vertNrs[sideEdges[i][j]];
			if (edgeVertNr < 0)
			{
				continue;
			}

			pos += mVertices[(physx::PxU32)edgeVertNr];
			num++;
		}

		if (num == 0 || num == 2)
		{
			continue;
		}

		pos = pos / (physx::PxF32)num;

		if (num == 1)
		{
			if (startFace < 0)
			{
				startFace = (physx::PxI32)faceNr;
			}
			else
			{
				endFace = (physx::PxI32)faceNr;
			}

			if (!(b->isEmpty()))
			{
				if (physx::PxAbs(pos.x - b->minimum.x) > physx::PxAbs(pos.x - b->maximum.x))
				{
					pos.x = b->minimum.x;
				}
				else
				{
					pos.x = b->maximum.x;
				}
				if (physx::PxAbs(pos.y - b->minimum.y) > physx::PxAbs(pos.y - b->maximum.y))
				{
					pos.y = b->minimum.y;
				}
				else
				{
					pos.y = b->maximum.y;
				}
				if (physx::PxAbs(pos.z - b->minimum.z) > physx::PxAbs(pos.z - b->maximum.z))
				{
					pos.z = b->minimum.z;
				}
				else
				{
					pos.z = b->maximum.z;
				}
			}
			else
			{
				continue;
			}
		}

		if (*faceVertNr < 0)
		{
			*faceVertNr = (physx::PxI32)mVertices.size();
			mVertices.pushBack(pos);
		}
		vertNrs[faceNr] = *faceVertNr;
	}

	physx::PxI32 maxGroup = groups[0];
	for (physx::PxU32 i = 1; i < 8; i++)
	{
		if (groups[i] > maxGroup)
		{
			maxGroup = groups[i];
		}
	}

	// boundary cell
	physx::Array<BorderEdge> queue;
	if (startFace >= 0)
	{
		BorderEdge edge;
		queue.clear();
		if (queue.capacity() < 20)
		{
			queue.reserve(20);
		}

		physx::PxI32 prev[19];
		bool visited[19];
		for (physx::PxU32 i = 0; i < 19; i++)
		{
			prev[i] = -1;
			visited[i] = false;
		}

		edge.set(startFace);
		queue.pushBack(edge);

		physx::PxI32 maxVert = -1;
		physx::PxI32 maxDepth = -1;

		while (!queue.empty())
		{
			edge = queue[queue.size() - 1];
			queue.popBack();

			physx::PxI32 v = edge.vertNr;
			if (visited[v])
			{
				continue;
			}

			visited[v] = true;
			prev[v] = edge.prev;
			edge.prev = v;
			edge.depth++;

			if (edge.depth > maxDepth)
			{
				maxDepth = edge.depth;
				maxVert = v;
			}
			//			if (v == endFace) break;

			if (v < 12)
			{
				for (physx::PxU32 i = 0; i < 8; i += 4)
				{
					edge.vertNr = adjVerts[v][i];
					if (vertNrs[edge.vertNr] >= 0)
					{
						if (!visited[edge.vertNr])
						{
							queue.pushBack(edge);
						}
					}
					else
					{
						for (physx::PxU32 j = i + 1; j < i + 4; j++)
						{
							edge.vertNr = adjVerts[v][j];
							if (vertNrs[edge.vertNr] >= 0 && !visited[edge.vertNr])
							{
								queue.pushBack(edge);
							}
						}
					}
				}
			}
			else
			{
				for (physx::PxU32 i = 0; i < 4; i++)
				{
					edge.vertNr = sideEdges[v - 12][i];
					if (!visited[edge.vertNr] && vertNrs[edge.vertNr] >= 0)
					{
						queue.pushBack(edge);
					}
				}
			}
		}

		physx::PxI32 chain[14];
		physx::PxU32 chainLen = 0;
		physx::PxI32 v = maxVert;

		if (vertNrs[v] >= 0)
		{
			chain[chainLen++] = v;
		}

		v = prev[v];
		while (v >= 0)
		{
			if (vertNrs[v] >= 0)
			{
				chain[chainLen++] = v;
			}
			v = prev[v];
		}

		physx::PxI32 numTris = (physx::PxI32)chainLen - 2;

		c.firstTriangle = (physx::PxI32)mIndices.size() / 3;
		for (int i = 0; i < numTris; i++)
		{
			mIndices.pushBack((physx::PxU32)vertNrs[(physx::PxU32)chain[0]]);
			mIndices.pushBack((physx::PxU32)vertNrs[(physx::PxU32)chain[i + 1]]);
			mIndices.pushBack((physx::PxU32)vertNrs[(physx::PxU32)chain[i + 2]]);
			c.numTriangles++;
		}
		return;
	}

	// inner cell
	if (mTemplates == NULL)
	{
		mTemplates = PX_NEW(ApexGeneralizedCubeTemplates)();
	}
	mTemplates->getTriangles(groups, mGeneralizedTriangles);

	// create face and inner vertices if necessary
	for (physx::PxU32 i = 0; i < mGeneralizedTriangles.size(); i++)
	{
		physx::PxI32 localNr = mGeneralizedTriangles[i];

		if (vertNrs[localNr] < 0)
		{
			if (localNr < 12)
			{
				return;    // impossible to create triangles, border cube
			}

			if (localNr == 18 && vertNrs[18] < 0)
			{
				// center vertex, not shared with other cells
				vertNrs[18] = (physx::PxI32)mVertices.size();
				physx::PxVec3 pos(0.0f);
				physx::PxF32 num = 0.0f;
				for (int j = 0; j < 12; j++)
				{
					if (vertNrs[j] >= 0)
					{
						pos += mVertices[(physx::PxU32)vertNrs[j]];
						num = num + 1.0f;
					}
				}
				mVertices.pushBack(pos / num);
			}
		}
	}

	c.firstTriangle = (physx::PxI32)mIndices.size() / 3;
	const physx::PxU32 numTris = mGeneralizedTriangles.size() / 3;
	for (physx::PxU32 i = 0; i < numTris; i++)
	{
		PX_ASSERT(vertNrs[mGeneralizedTriangles[3 * i + 0]] != vertNrs[mGeneralizedTriangles[3 * i + 1]]);
		PX_ASSERT(vertNrs[mGeneralizedTriangles[3 * i + 0]] != vertNrs[mGeneralizedTriangles[3 * i + 2]]);
		PX_ASSERT(vertNrs[mGeneralizedTriangles[3 * i + 1]] != vertNrs[mGeneralizedTriangles[3 * i + 2]]);
		mIndices.pushBack((physx::PxU32)vertNrs[mGeneralizedTriangles[3 * i + 0]]);
		mIndices.pushBack((physx::PxU32)vertNrs[mGeneralizedTriangles[3 * i + 1]]);
		mIndices.pushBack((physx::PxU32)vertNrs[mGeneralizedTriangles[3 * i + 2]]);
		c.numTriangles++;
	}
}



void ApexGeneralizedMarchingCubes::createNeighbourInfo()
{
	physx::Array<TriangleEdge> edges;
	edges.reserve(mIndices.size());

	mFirstNeighbour.resize(mIndices.size());
	for (physx::PxU32 i = 0; i < mFirstNeighbour.size(); i++)
	{
		mFirstNeighbour[i] = -1;
	}

	mNeighbours.clear();

	const physx::PxU32 numTriangles = mIndices.size() / 3;
	for (physx::PxU32 i = 0; i < numTriangles; i++)
	{
		TriangleEdge edge;
		physx::PxI32 i0 = (physx::PxI32)mIndices[3 * i];
		physx::PxI32 i1 = (physx::PxI32)mIndices[3 * i + 1];
		physx::PxI32 i2 = (physx::PxI32)mIndices[3 * i + 2];
		PX_ASSERT(i0 != i1);
		PX_ASSERT(i0 != i2);
		PX_ASSERT(i1 != i2);
		edge.init(i0, i1, 0, (physx::PxI32)i);
		edges.pushBack(edge);
		edge.init(i1, i2, 1, (physx::PxI32)i);
		edges.pushBack(edge);
		edge.init(i2, i0, 2, (physx::PxI32)i);
		edges.pushBack(edge);
	}
	shdfnd::sort(edges.begin(), edges.size(), TriangleEdge());

	physx::PxU32 i = 0;
	const physx::PxU32 numEdges = edges.size();
	while (i < numEdges)
	{
		const TriangleEdge& e0 = edges[i];
		const physx::PxI32 first = (physx::PxI32)mNeighbours.size();
		PX_ASSERT(mFirstNeighbour[(physx::PxU32)(e0.triNr * 3 + e0.edgeNr)] == -1);
		mFirstNeighbour[(physx::PxU32)(e0.triNr * 3 + e0.edgeNr)] = first;
		mNeighbours.pushBack(e0.triNr);
		i++;

		while (i < numEdges && edges[i] == e0)
		{
			const TriangleEdge& e1 = edges[i];
			PX_ASSERT(mFirstNeighbour[(physx::PxU32)(e1.triNr * 3 + e1.edgeNr)] == -1);
			mFirstNeighbour[(physx::PxU32)(e1.triNr * 3 + e1.edgeNr)] = first;
			mNeighbours.pushBack(e1.triNr);
			i++;
		}
		mNeighbours.pushBack(-1);	// end marker
	}
}



void ApexGeneralizedMarchingCubes::getCubeEdgesAndGroups(physx::PxI32 cellNr, GeneralizedVertRef* vertRefs[12], physx::PxI32 groups[8])
{
	const physx::PxI32 adjCorners[8][3] =
	{
		{1, 3, 4}, {0, 2, 5}, {1, 3, 6},  {0, 2, 7},  {0, 5, 7}, {1, 4, 6}, {2, 5, 7},  {3, 4, 6}
	};

	const physx::PxI32 adjEdges[8][3] =
	{
		{0, 3, 8}, {0, 1, 9}, {1, 2, 10}, {3, 2, 11}, {8, 4, 7}, {9, 4, 5}, {10, 5, 6}, {11, 7, 6}
	};

	// collect edge vertices
	GeneralizedCube& c = mCubes[(physx::PxU32)cellNr];
	for (physx::PxU32 i = 0; i < 12; i++)
	{
		vertRefs[i] = NULL;
	}

	vertRefs[0] = &c.vertRefs[0];
	vertRefs[3] = &c.vertRefs[1];
	vertRefs[8] = &c.vertRefs[2];

	physx::PxI32 nr = findCube(c.xi + 1, c.yi, c.zi);
	if (nr >= 0)
	{
		vertRefs[1] = &mCubes[(physx::PxU32)nr].vertRefs[1];
		vertRefs[9] = &mCubes[(physx::PxU32)nr].vertRefs[2];
	}
	nr = findCube(c.xi, c.yi + 1, c.zi);
	if (nr >= 0)
	{
		vertRefs[2] = &mCubes[(physx::PxU32)nr].vertRefs[0];
		vertRefs[11] = &mCubes[(physx::PxU32)nr].vertRefs[2];
	}
	nr = findCube(c.xi, c.yi, c.zi + 1);
	if (nr >= 0)
	{
		vertRefs[4] = &mCubes[(physx::PxU32)nr].vertRefs[0];
		vertRefs[7] = &mCubes[(physx::PxU32)nr].vertRefs[1];
	}
	nr = findCube(c.xi + 1, c.yi + 1, c.zi);
	if (nr >= 0)
	{
		vertRefs[10] = &mCubes[(physx::PxU32)nr].vertRefs[2];
	}
	nr = findCube(c.xi, c.yi + 1, c.zi + 1);
	if (nr >= 0)
	{
		vertRefs[6] = &mCubes[(physx::PxU32)nr].vertRefs[0];
	}
	nr = findCube(c.xi + 1, c.yi, c.zi + 1);
	if (nr >= 0)
	{
		vertRefs[5] = &mCubes[(physx::PxU32)nr].vertRefs[1];
	}

	// assign groups using flood fill on the cube edges
	for (physx::PxU32 i = 0; i < 8; i++)
	{
		groups[i] = -1;
	}

	int groupNr = -1;
	for (physx::PxU32 i = 0; i < 8; i++)
	{
		if (groups[i] >= 0)
		{
			continue;
		}

		groupNr++;
		mCubeQueue.clear();
		mCubeQueue.pushBack((physx::PxI32)i);
		while (!mCubeQueue.empty())
		{
			physx::PxI32 cornerNr = mCubeQueue[mCubeQueue.size() - 1];
			mCubeQueue.popBack();

			if (groups[cornerNr] >= 0)
			{
				continue;
			}

			groups[cornerNr] = groupNr;

			for (physx::PxU32 j = 0; j < 3; j++)
			{
				physx::PxI32 adjCorner = adjCorners[cornerNr][j];
				physx::PxI32 adjEdge   = adjEdges[cornerNr][j];
				if (vertRefs[adjEdge] != NULL && vertRefs[adjEdge]->vertNr >= 0)	// edge blocked by vertex
				{
					continue;
				}

				if (groups[adjCorner] < 0)
				{
					mCubeQueue.pushBack(adjCorner);
				}
			}
		}
	}
}



void ApexGeneralizedMarchingCubes::determineGroups()
{
	const physx::PxU32 numTris = mIndices.size() / 3;

	mTriangleGroup.resize(numTris);
	for (physx::PxU32 i = 0; i < numTris; i++)
	{
		mTriangleGroup[i] = -1;
	}

	mGroupFirstTriangle.clear();
	mGroupTriangles.clear();

	for (physx::PxU32 i = 0; i < numTris; i++)
	{
		if (mTriangleDeleted[i])
		{
			continue;
		}
		if (mTriangleGroup[i] >= 0)
		{
			continue;
		}

		physx::PxI32 group = (physx::PxI32)mGroupFirstTriangle.size();
		mGroupFirstTriangle.pushBack((physx::PxI32)mGroupTriangles.size());

		mCubeQueue.clear();
		mCubeQueue.pushBack((physx::PxI32)i);
		while (!mCubeQueue.empty())
		{
			const physx::PxU32 t = (physx::PxU32)mCubeQueue[mCubeQueue.size() - 1];
			mCubeQueue.popBack();

			if (mTriangleGroup[t] >= 0)
			{
				continue;
			}

			mTriangleGroup[t] = group;
			mGroupTriangles.pushBack((physx::PxI32)t);

			for (physx::PxU32 j = 0; j < 3; j++)
			{
				physx::PxU32 first = (physx::PxU32)mFirstNeighbour[3 * t + j];
				physx::PxU32 num = 0;
				physx::PxI32 n = -1;
				while (mNeighbours[first] >= 0)
				{
					if (mNeighbours[first] != (physx::PxI32)t)
					{
						n = mNeighbours[first];
					}

					first++;
					num++;
				}
				if (num == 2 && n >= 0 && mTriangleGroup[(physx::PxU32)n] < 0)
				{
					if (!mTriangleDeleted[(physx::PxU32)n])
					{
						mCubeQueue.pushBack(n);
					}
				}
			}
		}
	}
}



void ApexGeneralizedMarchingCubes::removeBubbles(physx::PxI32 minGroupSize)
{
	const physx::PxU32 numGroups = mGroupFirstTriangle.size();
	for (physx::PxU32 i = 0; i < numGroups; i++)
	{
		physx::PxI32 firstTri = mGroupFirstTriangle[i];
		physx::PxI32 lastTri  = i < numGroups - 1 ? mGroupFirstTriangle[i + 1] : (physx::PxI32)mGroupTriangles.size();

		if (lastTri - firstTri > minGroupSize)
		{
			continue;
		}

		bool OK = true;

		for (physx::PxI32 j = firstTri; j < lastTri; j++)
		{
			physx::PxI32 t = mGroupTriangles[(physx::PxU32)j];
			for (physx::PxU32 k = 0; k < 3; k++)
			{
				physx::PxU32 first = (physx::PxU32)mFirstNeighbour[3 * t + k];
				physx::PxU32 num = 0;
				physx::PxI32 n = -1;
				while (mNeighbours[first] >= 0)
				{
					if (mNeighbours[first] != t)
					{
						n = mNeighbours[first];
					}

					num++;
					first++;
				}
				if (num == 2 && mTriangleGroup[(physx::PxU32)n] != (physx::PxI32)i)
				{
					OK = false;
					break;
				}
			}
			if (!OK)
			{
				break;
			}
		}


		if (!OK)
		{
			continue;
		}

		for (physx::PxI32 j = firstTri; j < lastTri; j++)
		{
			physx::PxU32 t = (physx::PxU32)mGroupTriangles[(physx::PxU32)j];
			// remove neighbor info
			for (int k = 0; k < 3; k++)
			{
				physx::PxU32 first = (physx::PxU32)mFirstNeighbour[3 * t + k];
				physx::PxI32 pos = -1;
				while (mNeighbours[first] >= 0)
				{
					if (mNeighbours[first] == (physx::PxI32)t)
					{
						PX_ASSERT(pos == -1);
						pos = (physx::PxI32)first;
					}

					first++;
				}
				if (pos >= 0)
				{
					mNeighbours[(physx::PxU32)pos] = mNeighbours[first - 1];
					mNeighbours[first - 1] = -1;
				}
			}
			// remove triangle
			mTriangleDeleted[t] = true;

			/*
			// debugging only
			mDebugLines.pushBack(mVertices[mIndices[t * 3 + 0]]);
			mDebugLines.pushBack(mVertices[mIndices[t * 3 + 1]]);
			mDebugLines.pushBack(mVertices[mIndices[t * 3 + 1]]);
			mDebugLines.pushBack(mVertices[mIndices[t * 3 + 2]]);
			mDebugLines.pushBack(mVertices[mIndices[t * 3 + 2]]);
			mDebugLines.pushBack(mVertices[mIndices[t * 3 + 0]]);
			*/
		}
	}
}



void ApexGeneralizedMarchingCubes::fixOrientations()
{
	if (mIndices.size() == 0)
	{
		return;
	}

	const physx::PxU32 numTris = mIndices.size() / 3;

	physx::Array<physx::PxU8> marks;
	marks.resize(numTris);

	// 0 = non visited, 1 = visited, 2 = visited, to be flipped
	for (physx::PxU32 i = 0; i < numTris; i++)
	{
		marks[i] = 0;
	}

	physx::Array<physx::PxI32> queue;
	for (physx::PxU32 i = 0; i < numTris; i++)
	{
		if (mTriangleDeleted[i])
		{
			continue;
		}

		if (marks[i] != 0)
		{
			continue;
		}

		queue.clear();

		marks[i] = 1;
		queue.pushBack((physx::PxI32)i);
		while (!queue.empty())
		{
			physx::PxU32 triNr = (physx::PxU32)queue[queue.size() - 1];
			queue.popBack();

			for (physx::PxU32 j = 0; j < 3; j++)
			{
				physx::PxU32 first = (physx::PxU32)mFirstNeighbour[3 * triNr + j];
				physx::PxU32 num = 0;
				physx::PxI32 adjNr = -1;

				while (mNeighbours[first] >= 0)
				{
					if (mNeighbours[first] != (physx::PxI32)triNr)
					{
						adjNr = mNeighbours[first];
					}

					first++;
					num++;
				}

				if (num != 2)
				{
					continue;
				}
				if (marks[(physx::PxU32)adjNr] != 0)
				{
					continue;
				}

				queue.pushBack(adjNr);

				physx::PxU32 i0, i1;
				if (marks[(physx::PxU32)triNr] == 1)
				{
					i0 = mIndices[3 * triNr + j];
					i1 = mIndices[3 * triNr + ((j + 1) % 3)];
				}
				else
				{
					i1 = mIndices[3 * triNr + j];
					i0 = mIndices[3 * triNr + ((j + 1) % 3)];
				}

				// don't swap here because this would corrupt the neighbor information
				marks[(physx::PxU32)adjNr] = 1;
				if (mIndices[3 * (physx::PxU32)adjNr + 0] == i0 && mIndices[3 * (physx::PxU32)adjNr + 1] == i1)
				{
					marks[(physx::PxU32)adjNr] = 2;
				}
				if (mIndices[3 * (physx::PxU32)adjNr + 1] == i0 && mIndices[3 * (physx::PxU32)adjNr + 2] == i1)
				{
					marks[(physx::PxU32)adjNr] = 2;
				}
				if (mIndices[3 * (physx::PxU32)adjNr + 2] == i0 && mIndices[3 * (physx::PxU32)adjNr + 0] == i1)
				{
					marks[(physx::PxU32)adjNr] = 2;
				}
			}
		}
	}

	for (physx::PxU32 i = 0; i < numTris; i++)
	{
		if (marks[i] == 2)
		{
			physx::PxU32 i0 = mIndices[3 * i];
			mIndices[3 * i] = mIndices[3 * i + 1];
			mIndices[3 * i + 1] = i0;
		}
	}
}



void ApexGeneralizedMarchingCubes::compress()
{
	PX_ASSERT(mTriangleDeleted.size() * 3 == mIndices.size());

	physx::PxU32 writePos = 0;

	for (physx::PxU32 i = 0; i < mTriangleDeleted.size(); i++)
	{
		if (mTriangleDeleted[i] == 1)
		{
			continue;
		}
		for (physx::PxU32 j = 0; j < 3; j++)
		{
			mIndices[3 * writePos + j] = mIndices[3 * i + j];
		}
		writePos++;
	}

	mIndices.resize(3 * writePos);

	mTriangleDeleted.clear();
	mTriangleDeleted.reset();

	mNeighbours.clear();
	mNeighbours.reset();

	mFirstNeighbour.clear();
	mFirstNeighbour.reset();
}

}
} // end namespace physx::apex
