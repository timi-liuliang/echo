/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexMeshContractor.h"
#include "NiApexSDK.h"
#include "foundation/PxBounds3.h"
#include "ApexBinaryHeap.h"
#include "ApexSharedUtils.h"

#include "PsSort.h"

namespace physx
{
namespace apex
{

#define EDGE_PRESERVATION 1

struct TriangleEdge
{
	void init(physx::PxU32 v0, physx::PxU32 v1, physx::PxU32 edgeNr, physx::PxU32 triNr)
	{
		PX_ASSERT(v0 != v1);
		this->v0 = physx::PxMin(v0, v1);
		this->v1 = physx::PxMax(v0, v1);
		this->edgeNr = edgeNr;
		this->triNr = triNr;
	}
	bool operator < (const TriangleEdge& e) const
	{
		if (v0 < e.v0)
		{
			return true;
		}
		if (v0 > e.v0)
		{
			return false;
		}
		return v1 < e.v1;
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
		return a.v1 < b.v1;
	}
	bool operator == (const TriangleEdge& e) const
	{
		if (v0 == e.v0 && v1 == e.v1)
		{
			return true;
		}
		PX_ASSERT(!(v0 == e.v1 && v1 == e.v0));
		//if (v0 == e.v1 && v1 == e.v0) return true;
		return false;
	}
	physx::PxU32 v0, v1;
	physx::PxU32 edgeNr;
	physx::PxU32 triNr;
};



struct CellCoord
{
	physx::PxU32 xi, yi, zi;
	physx::PxF32 value;
	bool operator < (CellCoord& c) const
	{
		return value < c.value;
	}
};



ApexMeshContractor::ApexMeshContractor() :
	mCellSize(1.0f),
	mOrigin(0.0f, 0.0f, 0.0f),
	mNumX(0), mNumY(0), mNumZ(0),
	mInitialVolume(0.0f),
	mCurrentVolume(0.0f)
{
	PX_COMPILE_TIME_ASSERT(sizeof(ContractorCell) == 12);
}



void ApexMeshContractor::registerVertex(const physx::PxVec3& pos)
{
	mVertices.pushBack(pos);
}



void ApexMeshContractor::registerTriangle(physx::PxU32 v0, physx::PxU32 v1, physx::PxU32 v2)
{
	mIndices.pushBack(v0);
	mIndices.pushBack(v1);
	mIndices.pushBack(v2);
}



bool ApexMeshContractor::endRegistration(physx::PxU32 subdivision, IProgressListener* progressListener)
{
	HierarchicalProgressListener progress(100, progressListener);
	progress.setSubtaskWork(30, "Compute Neighbors");
	computeNeighbours();

	physx::PxBounds3 bounds;
	bounds.setEmpty();
	for (unsigned i = 0; i < mVertices.size(); i++)
	{
		bounds.include(mVertices[i]);
	}

	mCellSize = (bounds.maximum - bounds.minimum).magnitude() / subdivision;
	if (mCellSize == 0.0f)
	{
		mCellSize = 1.0f;
	}
	PX_ASSERT(!bounds.isEmpty());
	bounds.fattenFast(2.0f * mCellSize);

	mOrigin = bounds.minimum;
	float invH = 1.0f / mCellSize;
	mNumX = (physx::PxU32)((bounds.maximum.x - bounds.minimum.x) * invH) + 1;
	mNumY = (physx::PxU32)((bounds.maximum.y - bounds.minimum.y) * invH) + 1;
	mNumZ = (physx::PxU32)((bounds.maximum.z - bounds.minimum.z) * invH) + 1;
	unsigned num = mNumX * mNumY * mNumZ;
	mGrid.resize(num, ContractorCell());

	progress.completeSubtask();

	progress.setSubtaskWork(60, "Compute Signed Distance Field");
	computeSignedDistanceField();
	progress.completeSubtask();

	progress.setSubtaskWork(10, "Computing Volume");
	physx::PxF32 area = 0;
	computeAreaAndVolume(area, mInitialVolume);
	progress.completeSubtask();

	return true;
}



physx::PxU32 ApexMeshContractor::contract(physx::PxI32 steps, physx::PxF32 abortionRatio, physx::PxF32& volumeRatio, IProgressListener* progressListener)
{
	if (steps == -1 && (abortionRatio <= 0 || abortionRatio > 1))
	{
		APEX_INTERNAL_ERROR("Invalid abortionRatio when doing infinite steps (%f)", abortionRatio);
		return 0;
	}

	HierarchicalProgressListener progress(100, progressListener);
	progress.setSubtaskWork(100, "Contract");

	bool abort = false;
	physx::PxI32 currStep = 0;
	for (; (steps < 0 || currStep < steps) && !abort; currStep++)
	{
		{
			physx::PxI32 percent = 100 * currStep / steps;
			progress.setProgress(percent);
		}
		if (!abort)
		{
			contractionStep();
			subdivide(1.5f * mCellSize);
			collapse(0.3f * mCellSize);

			physx::PxF32 area;
			computeAreaAndVolume(area, mCurrentVolume);

			volumeRatio = mCurrentVolume / mInitialVolume;
			abort = volumeRatio < abortionRatio;
		}
	}

	progress.completeSubtask();
	return (physx::PxU32)currStep;
}



void ApexMeshContractor::expandBorder()
{
	const physx::PxF32 localRadius = 2.0f * mCellSize;
	//const physx::PxF32 minDist = 1.0f * mCellSize;
	physx::PxF32 maxDot = 0.0f;

	if (mNeighbours.size() != mIndices.size())
	{
		return;
	}

	const physx::PxU32 numTris = mIndices.size() / 3;
	const physx::PxU32 numVerts = mVertices.size();

	void* memory = PX_ALLOC(sizeof(physx::PxU32) * numTris + sizeof(physx::PxVec3) * numTris, PX_DEBUG_EXP("ApexMeshContractor"));
	physx::PxU32* triMarks = (physx::PxU32*)memory;
	physx::PxVec3* triComs = (physx::PxVec3*)(triMarks + numTris);
	memset(triMarks, -1, sizeof(physx::PxU32) * numTris);

	physx::Array<physx::PxVec3> dispField;
	dispField.resize(numVerts);

	physx::Array<physx::PxF32> dispWeights;
	dispWeights.resize(numVerts);

	for (physx::PxU32 i = 0; i < numVerts; i++)
	{
		dispField[i] = physx::PxVec3(0.0f);
		dispWeights[i] = 0.0f;
	}

	physx::Array<physx::PxI32> localTris;
	physx::Array<physx::PxF32> localDists;

	for (physx::PxU32 i = 0; i < numTris; i++)
	{
		triComs[i] = physx::PxVec3(0.0f);

		// is there a border edge?
		physx::PxU32 i0 = mIndices[3 * i];
		physx::PxU32 i1 = mIndices[3 * i + 1];
		physx::PxU32 i2 = mIndices[3 * i + 2];

		const physx::PxVec3& p0 = mVertices[i0];
		const physx::PxVec3& p1 = mVertices[i1];
		const physx::PxVec3& p2 = mVertices[i2];

		const physx::PxVec3 ci = (p0 + p1 + p2) / 3.0f;

		physx::PxVec3 ni = (p1 - p0).cross(p2 - p0);
		ni.normalize();

		bool edgeFound = false;
		for (physx::PxU32 j = 0; j < 3; j++)
		{
			physx::PxI32 adj = mNeighbours[3 * i + j];
			if (adj < 0)
			{
				continue;
			}

			physx::PxVec3& q0 = mVertices[mIndices[3 * adj + j]];
			physx::PxVec3& q1 = mVertices[mIndices[3 * adj + (j + 1) % 3]];
			physx::PxVec3& q2 = mVertices[mIndices[3 * adj + (j + 2) % 3]];

			physx::PxVec3 nadj = (q1 - q0).cross(q2 - q0);
			nadj.normalize();

			if (ni.dot(nadj) < maxDot)
			{
				edgeFound = true;
				break;
			}
		}

		if (!edgeFound)
		{
			continue;
		}

		collectNeighborhood((physx::PxI32)i, localRadius, i, localTris, localDists, triMarks);
		physx::PxU32 numLocals = localTris.size();

		// is the neighborhood double sided?
		physx::PxF32 doubleArea = 0.0f;
		physx::PxVec3 com(0.0f, 0.0f, 0.0f);
		physx::PxF32 totalW = 0.0f;

		for (physx::PxU32 j = 0; j < numLocals; j++)
		{
			physx::PxU32 triJ = (physx::PxU32)localTris[j];
			const physx::PxVec3& p0 = mVertices[mIndices[3 * triJ + 0]];
			const physx::PxVec3& p1 = mVertices[mIndices[3 * triJ + 1]];
			const physx::PxVec3& p2 = mVertices[mIndices[3 * triJ + 2]];

			physx::PxVec3 nj = (p1 - p0).cross(p2 - p0);
			const physx::PxF32 area = nj.normalize();
			const physx::PxF32 w = area;
			totalW += w;
			com += (p0 + p1 + p2) / 3.0f * w;

			for (physx::PxU32 k = 0; k < numLocals; k++)
			{
				physx::PxU32 triK = (physx::PxU32)localTris[k];

				const physx::PxVec3& q0 = mVertices[mIndices[3 * triK + 0]];
				const physx::PxVec3& q1 = mVertices[mIndices[3 * triK + 1]];
				const physx::PxVec3& q2 = mVertices[mIndices[3 * triK + 2]];

				physx::PxVec3 nk = (q1 - p0).cross(q2 - q0);
				nk.normalize();

				if (nj.dot(nk) < -0.999f)
				{
					doubleArea += area;
					break;
				}
			}
		}

		physx::PxF32 totalArea = PxPi * localRadius * localRadius;
		if (doubleArea < 0.5f * totalArea)
		{
			continue;
		}

		if (totalW > 0.0f)
		{
			com /= totalW;
		}

		triComs[i] = com;

		// update displacement field
		physx::PxVec3 disp = ci - com;
		disp.normalize();
		disp *= 2.0f * mCellSize;

		physx::PxF32 minT = findMin(ci, disp);
		disp *= minT;

		physx::PxF32 maxDist = 0.0f;
		for (physx::PxU32 j = 0; j < numLocals; j++)
		{
			maxDist = physx::PxMax(maxDist, localDists[j]);
		}

		for (physx::PxU32 j = 0; j < numLocals; j++)
		{
			physx::PxI32 triJ = localTris[j];

			physx::PxF32 w = 1.0f;
			if (maxDist != 0.0f)
			{
				w = 1.0f - localDists[j] / maxDist;
			}

			for (physx::PxU32 k = 0; k < 3; k++)
			{
				physx::PxU32 v = mIndices[3 * triJ + k];
				dispField[v] += disp * w * w;
				dispWeights[v] += w;
			}
		}
	}

	for (physx::PxU32 i = 0; i < mVertices.size(); i++)
	{
		const physx::PxF32 w = dispWeights[i];
		if (w == 0.0f)
		{
			continue;
		}

		mVertices[i] += dispField[i] / w;
	}

	PX_FREE(memory);
}




void ApexMeshContractor::computeNeighbours()
{
	physx::Array<TriangleEdge> edges;
	edges.reserve(mIndices.size());

	mNeighbours.resize(mIndices.size());
	for (physx::PxU32 i = 0; i < mNeighbours.size(); i++)
	{
		mNeighbours[i] = -1;
	}

	PX_ASSERT(mIndices.size() % 3 == 0);
	const physx::PxU32 numTriangles = mIndices.size() / 3;
	for (physx::PxU32 i = 0; i < numTriangles; i++)
	{
		physx::PxU32 i0 = mIndices[3 * i  ];
		physx::PxU32 i1 = mIndices[3 * i + 1];
		physx::PxU32 i2 = mIndices[3 * i + 2];
		TriangleEdge edge;
		edge.init(i0, i1, 0, i);
		edges.pushBack(edge);
		edge.init(i1, i2, 1, i);
		edges.pushBack(edge);
		edge.init(i2, i0, 2, i);
		edges.pushBack(edge);
	}

	shdfnd::sort(edges.begin(), edges.size(), TriangleEdge());

	physx::PxU32 i = 0;
	const physx::PxU32 numEdges = edges.size();
	while (i < numEdges)
	{
		const TriangleEdge& e0 = edges[i];
		i++;
		while (i < numEdges && edges[i] == e0)
		{
			const TriangleEdge& e1 = edges[i];
			PX_ASSERT(mNeighbours[e0.triNr * 3 + e0.edgeNr] == -1);///?
			mNeighbours[e0.triNr * 3 + e0.edgeNr] = (physx::PxI32)e1.triNr;
			PX_ASSERT(mNeighbours[e1.triNr * 3 + e1.edgeNr] == -1);///?
			mNeighbours[e1.triNr * 3 + e1.edgeNr] = (physx::PxI32)e0.triNr;
			i++;
		}
	}
}



void ApexMeshContractor::computeSignedDistanceField()
{
	// init
	for (physx::PxU32 i = 0; i < mGrid.size(); i++)
	{
		PX_ASSERT(mGrid[i].distance == PX_MAX_F32);
		PX_ASSERT(mGrid[i].inside == 0);
	}

	PX_ASSERT(mIndices.size() % 3 == 0);
	physx::PxU32 numTris = mIndices.size() / 3;

	for (physx::PxU32 i = 0; i < numTris; i++)
	{
		physx::PxVec3 p0 = mVertices[mIndices[3 * i  ]] - mOrigin;
		physx::PxVec3 p1 = mVertices[mIndices[3 * i + 1]] - mOrigin;
		physx::PxVec3 p2 = mVertices[mIndices[3 * i + 2]] - mOrigin;
		addTriangle(p0, p1, p2);
	}


	// fast marching, see J.A. Sethian "Level Set Methods and Fast Marching Methods"
	// create front
	ApexBinaryHeap<CellCoord> heap;
	//CellCoord cc;

	//int xi,yi,zi;

	for (physx::PxU32 xi = 0; xi < mNumX - 1; xi++)
	{
		CellCoord cc;
		cc.xi = xi;
		for (physx::PxU32 yi = 0; yi < mNumY - 1; yi++)
		{
			cc.yi = yi;
			for (physx::PxU32 zi = 0; zi < mNumZ - 1; zi++)
			{
				cc.zi = zi;
				const ContractorCell& c = cellAt((physx::PxI32)xi, (physx::PxI32)yi, (physx::PxI32)zi);
				if (!c.marked)
				{
					continue;
				}
				cc.value = c.distance;
				heap.push(cc);
			}
		}
	}

	while (!heap.isEmpty())
	{
		CellCoord cc;

		// Make compiler happy
		cc.xi = cc.yi = cc.zi = 0;
		cc.value = 0.0f;

		heap.pop(cc);

		ContractorCell& c = cellAt((physx::PxI32)cc.xi, (physx::PxI32)cc.yi, (physx::PxI32)cc.zi);
		c.marked = true;
		for (int i = 0; i < 6; i++)
		{
			CellCoord ncc = cc;
			switch (i)
			{
			case 0:
				if (ncc.xi < 1)
				{
					continue;
				}
				else
				{
					ncc.xi--;
				}
				break;
			case 1:
				if (ncc.xi >= mNumX - 1)
				{
					continue;
				}
				else
				{
					ncc.xi++;
				}
				break;
			case 2:
				if (ncc.yi < 1)
				{
					continue;
				}
				else
				{
					ncc.yi--;
				}
				break;
			case 3:
				if (ncc.yi >= mNumY - 1)
				{
					continue;
				}
				else
				{
					ncc.yi++;
				}
				break;
			case 4:
				if (ncc.zi < 1)
				{
					continue;
				}
				else
				{
					ncc.zi--;
				}
				break;
			case 5:
				if (ncc.zi >= mNumZ - 1)
				{
					continue;
				}
				else
				{
					ncc.zi++;
				}
				break;
			}

			ContractorCell& cn = cellAt((physx::PxI32)ncc.xi, (physx::PxI32)ncc.yi, (physx::PxI32)ncc.zi);
			if (cn.marked)
			{
				continue;
			}
			if (!updateDistance(ncc.xi, ncc.yi, ncc.zi))
			{
				continue;
			}
			ncc.value = cn.distance;
			heap.push(ncc);
		}
	}

	setInsideOutside();
	for (unsigned i = 0; i < mGrid.size(); i++)
	{
		if (mGrid[i].inside < 3)	// majority vote
		{
			mGrid[i].distance = -mGrid[i].distance;
		}
	}
}



void ApexMeshContractor::contractionStep()
{
#if EDGE_PRESERVATION
	unsigned numTris = mIndices.size() / 3;
	mVertexCurvatures.resize(mVertices.size());
	for (unsigned i = 0; i < mVertexCurvatures.size(); i++)
	{
		mVertexCurvatures[i] = 0.0f;
	}

	for (unsigned i = 0; i < numTris; i++)
	{
		for (unsigned j = 0; j < 3; j++)
		{
			unsigned vertNr = mIndices[3 * i + j];
			if (mVertexCurvatures[vertNr] == 0.0f)
			{
				float curv = curvatureAt((physx::PxI32)i, (physx::PxI32)vertNr);
				mVertexCurvatures[vertNr] = curv;
			}
		}
	}
#endif

	float s = 0.1f * mCellSize;
	for (unsigned i = 0; i < mVertices.size(); i++)
	{
		physx::PxVec3& p = mVertices[i];
		physx::PxVec3 grad;
		interpolateGradientAt(p, grad);

#if EDGE_PRESERVATION
		p += grad * s  * (1.0f - mVertexCurvatures[i]);
#else
		p += grad * s;
#endif
	}
}



void ApexMeshContractor::computeAreaAndVolume(physx::PxF32& area, physx::PxF32& volume)
{
	area = volume = 0.0f;

	for (physx::PxU32 i = 0; i < mIndices.size(); i += 3)
	{
		physx::PxVec3& d0 = mVertices[mIndices[i]];
		physx::PxVec3 d1 = mVertices[mIndices[i + 1]] - mVertices[mIndices[i]];
		physx::PxVec3 d2 = mVertices[mIndices[i + 2]] - mVertices[mIndices[i]];
		physx::PxVec3 n = d1.cross(d2);
		area += n.magnitude();
		volume += n.dot(d0);
	}

	area /= 2.0f;
	volume /= 6.0f;
}



void ApexMeshContractor::addTriangle(const physx::PxVec3& p0, const physx::PxVec3& p1, const physx::PxVec3& p2)
{
	physx::PxBounds3 bounds;
	bounds.setEmpty();
	bounds.include(p0);
	bounds.include(p1);
	bounds.include(p2);
	PX_ASSERT(!bounds.isEmpty());
	bounds.fattenFast(0.01f * mCellSize);

	physx::PxVec3 n = (p1 - p0).cross(p2 - p0);

	physx::PxF32 d_big = n.dot(p0);
	physx::PxF32 invH = 1.0f / mCellSize;

	physx::PxI32 min[3];
	min[0] = (physx::PxI32)PxFloor(bounds.minimum.x * invH) + 1;
	min[1] = (physx::PxI32)PxFloor(bounds.minimum.y * invH) + 1;
	min[2] = (physx::PxI32)PxFloor(bounds.minimum.z * invH) + 1;
	physx::PxI32 max[3];
	max[0] = (physx::PxI32)PxFloor(bounds.maximum.x * invH);
	max[1] = (physx::PxI32)PxFloor(bounds.maximum.y * invH);
	max[2] = (physx::PxI32)PxFloor(bounds.maximum.z * invH);

	if (min[0] < 0)
	{
		min[0] = 0;
	}
	if (min[1] < 0)
	{
		min[1] = 0;
	}
	if (min[2] < 0)
	{
		min[2] = 0;
	}
	if (max[0] >= (physx::PxI32)mNumX)
	{
		max[0] = (physx::PxI32)mNumX - 1;
	}
	if (max[1] >= (physx::PxI32)mNumY)
	{
		max[1] = (physx::PxI32)mNumY - 1;
	}
	if (max[2] >= (physx::PxI32)mNumZ)
	{
		max[2] = (physx::PxI32)mNumZ - 1;
	}

	physx::PxI32 num[3] = { (physx::PxI32)mNumX, (physx::PxI32)mNumY, (physx::PxI32)mNumZ };

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
				physx::PxF32 axis1 = coord[dim1] * mCellSize;
				physx::PxF32 axis2 = coord[dim2] * mCellSize;
				/// \todo prevent singular cases when the same spacing was used in previous steps ????
				axis1 += 0.00017f * mCellSize;
				axis2 += 0.00017f * mCellSize;

				// does the ray go through the triangle?
				if (n[dim0] > 0.0f)
				{
					if ((p1[dim1] - p0[dim1]) * (axis2 - p0[dim2]) - (axis1 - p0[dim1]) * (p1[dim2] - p0[dim2]) < 0.0f)
					{
						continue;
					}
					if ((p2[dim1] - p1[dim1]) * (axis2 - p1[dim2]) - (axis1 - p1[dim1]) * (p2[dim2] - p1[dim2]) < 0.0f)
					{
						continue;
					}
					if ((p0[dim1] - p2[dim1]) * (axis2 - p2[dim2]) - (axis1 - p2[dim1]) * (p0[dim2] - p2[dim2]) < 0.0f)
					{
						continue;
					}
				}
				else
				{
					if ((p1[dim1] - p0[dim1]) * (axis2 - p0[dim2]) - (axis1 - p0[dim1]) * (p1[dim2] - p0[dim2]) > 0.0f)
					{
						continue;
					}
					if ((p2[dim1] - p1[dim1]) * (axis2 - p1[dim2]) - (axis1 - p1[dim1]) * (p2[dim2] - p1[dim2]) > 0.0f)
					{
						continue;
					}
					if ((p0[dim1] - p2[dim1]) * (axis2 - p2[dim2]) - (axis1 - p2[dim1]) * (p0[dim2] - p2[dim2]) > 0.0f)
					{
						continue;
					}
				}

				physx::PxF32 pos = (d_big - axis1 * n[dim1] - axis2 * n[dim2]) / n[dim0];
				coord[dim0] = (int)PxFloor(pos * invH);
				if (coord[dim0] < 0 || coord[dim0] >= num[dim0])
				{
					continue;
				}

				ContractorCell& cell = cellAt(coord[0], coord[1], coord[2]);
				cell.numCuts[dim0]++;

				physx::PxF32 d = physx::PxAbs(pos - coord[dim0] * mCellSize) * invH;
				if (d < cell.distance)
				{
					cell.distance = d;
					cell.marked = true;
				}

				if (coord[dim0] < num[dim0] - 1)
				{
					int adj[3] = { coord[0], coord[1], coord[2] };
					adj[dim0]++;
					ContractorCell& ca = cellAt(adj[0], adj[1], adj[2]);
					d = 1.0f - d;
					if (d < ca.distance)
					{
						ca.distance = d;
						ca.marked = true;
					}
				}
			}
		}
	}
}



bool ApexMeshContractor::updateDistance(physx::PxU32 xi, physx::PxU32 yi, physx::PxU32 zi)
{
	ContractorCell& ci = cellAt((physx::PxI32)xi, (physx::PxI32)yi, (physx::PxI32)zi);
	if (ci.marked)
	{
		return false;
	}

	// create quadratic equation from the stencil
	physx::PxF32 a = 0.0f;
	physx::PxF32 b = 0.0f;
	physx::PxF32 c = -1.0f;
	for (physx::PxU32 i = 0; i < 3; i++)
	{
		physx::PxF32 min = PX_MAX_F32;
		physx::PxI32 x = (physx::PxI32)xi, y = (physx::PxI32)yi, z = (physx::PxI32)zi;
		switch (i)
		{
		case 0:
			x--;
			break;
		case 1:
			y--;
			break;
		case 2:
			z--;
			break;
		}

		if (x >= 0 && y >= 0 && z >= 0)
		{
			ContractorCell& c = cellAt(x, y, z);
			if (c.marked)
			{
				min = c.distance;
			}
		}

		switch (i)
		{
		case 0:
			x += 2;
			break;
		case 1:
			y += 2;
			break;
		case 2:
			z += 2;
			break;
		}

		if (x < (physx::PxI32)mNumX && y < (physx::PxI32)mNumY && z < (physx::PxI32)mNumZ)
		{
			ContractorCell& c = cellAt(x, y, z);
			if (c.marked && c.distance < min)
			{
				min = c.distance;
			}
		}

		if (min != PX_MAX_F32)
		{
			a += 1.0f;
			b -= 2.0f * min;
			c += min * min;
		}
	}

	// solve the equation, the larger solution is the right one (distances grow as we proceed)
	if (a == 0.0f)
	{
		return false;
	}

	physx::PxF32 d = b * b - 4.0f * a * c;

	if (d < 0.0f)
	{
		d = 0.0f;    // debug
	}

	physx::PxF32 distance = (-b + physx::PxSqrt(d)) / (2.0f * a);

	if (distance < ci.distance)
	{
		ci.distance = distance;
		return true;
	}

	return false;
}
void ApexMeshContractor::setInsideOutside()
{
	physx::PxI32 num[3] = { (physx::PxI32)mNumX, (physx::PxI32)mNumY, (physx::PxI32)mNumZ };

	physx::PxI32 coord[3];
	for (physx::PxU32 dim0 = 0; dim0 < 3; dim0++)
	{
		const physx::PxU32 dim1 = (dim0 + 1) % 3;
		const physx::PxU32 dim2 = (dim0 + 2) % 3;
		for (coord[dim1] = 0; coord[dim1] < num[dim1]; coord[dim1]++)
		{
			for (coord[dim2] = 0; coord[dim2] < num[dim2]; coord[dim2]++)
			{
				bool inside = false;
				for (coord[dim0] = 0; coord[dim0] < num[dim0]; coord[dim0]++)
				{
					ContractorCell& cell = cellAt(coord[0], coord[1], coord[2]);
					if (inside)
					{
						cell.inside++;
					}
					if ((cell.numCuts[dim0] % 2) == 1)
					{
						inside = !inside;
					}
				}

				inside = false;
				for (coord[dim0] = num[dim0] - 1; coord[dim0] >= 0; coord[dim0]--)
				{
					ContractorCell& cell = cellAt(coord[0], coord[1], coord[2]);
					if ((cell.numCuts[dim0] % 2) == 1)
					{
						inside = !inside;
					}
					if (inside)
					{
						cell.inside++;
					}
				}
			}
		}
	}
}



void ApexMeshContractor::interpolateGradientAt(const physx::PxVec3& pos, physx::PxVec3& grad)
{
	// the derivatives of the distances are defined on the center of the edges
	// from there they are interpolated trilinearly
	const physx::PxVec3 p = pos - mOrigin;
	const physx::PxF32 h = mCellSize;
	const physx::PxF32 h1 = 1.0f / h;
	const physx::PxF32 h2 = 0.5f * h;
	physx::PxI32 x0 = (physx::PxI32)PxFloor(p.x * h1);
	const physx::PxF32 dx0 = (p.x - h * x0) * h1;
	const physx::PxF32 ex0 = 1.0f - dx0;
	physx::PxI32 y0 = (physx::PxI32)PxFloor(p.y * h1);
	const physx::PxF32 dy0 = (p.y - h * y0) * h1;
	const physx::PxF32 ey0 = 1.0f - dy0;
	physx::PxI32 z0 = (physx::PxI32)PxFloor(p.z * h1);
	const physx::PxF32 dz0 = (p.z - h * z0) * h1;
	const physx::PxF32 ez0 = 1.0f - dz0;

	if (x0 < 0)
	{
		x0 = 0;
	}
	if (x0 + 1 >= (physx::PxI32)mNumX)
	{
		x0 = (physx::PxI32)mNumX - 2;    // todo: handle boundary correctly
	}
	if (y0 < 0)
	{
		y0 = 0;
	}
	if (y0 + 1 >= (physx::PxI32)mNumY)
	{
		y0 = (physx::PxI32)mNumY - 2;
	}
	if (z0 < 0)
	{
		z0 = 0;
	}
	if (z0 + 1 >= (physx::PxI32)mNumZ)
	{
		z0 = (physx::PxI32)mNumZ - 2;
	}

	// the following coefficients are used on the axis of the component that is computed
	// everything is shifted there because the derivatives are defined at the center of edges
	//float dx2,dy2,dz2,ex2,ey2,ez2;
	physx::PxI32 x2 = (physx::PxI32)PxFloor((p.x - h2) * h1);
	const physx::PxF32 dx2 = (p.x - h2 - h * x2) * h1;
	const physx::PxF32 ex2 = 1.0f - dx2;
	physx::PxI32 y2 = (physx::PxI32)PxFloor((p.y - h2) * h1);
	const physx::PxF32 dy2 = (p.y - h2 - h * y2) * h1;
	const physx::PxF32 ey2 = 1.0f - dy2;
	physx::PxI32 z2 = (physx::PxI32)PxFloor((p.z - h2) * h1);
	const physx::PxF32 dz2 = (p.z - h2 - h * z2) * h1;
	const physx::PxF32 ez2 = 1.0f - dz2;

	if (x2 < 0)
	{
		x2 = 0;
	}
	if (x2 + 2 >= (physx::PxI32)mNumX)
	{
		x2 = (physx::PxI32)mNumX - 3;    // todo: handle boundary correctly
	}
	if (y2 < 0)
	{
		y2 = 0;
	}
	if (y2 + 2 >= (physx::PxI32)mNumY)
	{
		y2 = (physx::PxI32)mNumY - 3;
	}
	if (z2 < 0)
	{
		z2 = 0;
	}
	if (z2 + 2 >= (physx::PxI32)mNumZ)
	{
		z2 = (physx::PxI32)mNumZ - 3;
	}

	physx::PxF32 d, d0, d1, d2, d3, d4, d5, d6, d7;
	d = cellAt(x2 + 1, y0  , z0).distance;
	d0 = d - cellAt(x2, y0,  z0).distance;
	d4 = cellAt(x2 + 2, y0,  z0).distance - d;
	d = cellAt(x2 + 1, y0 + 1, z0).distance;
	d1 = d - cellAt(x2, y0 + 1, z0).distance;
	d5 = cellAt(x2 + 2, y0 + 1, z0).distance - d;
	d = cellAt(x2 + 1, y0 + 1, z0 + 1).distance;
	d2 = d - cellAt(x2, y0 + 1, z0 + 1).distance;
	d6 = cellAt(x2 + 2, y0 + 1, z0 + 1).distance - d;
	d = cellAt(x2 + 1, y0,  z0 + 1).distance;
	d3 = d - cellAt(x2, y0,  z0 + 1).distance;
	d7 = cellAt(x2 + 2, y0,  z0 + 1).distance - d;

	grad.x = ex2 * (d0 * ey0 * ez0 + d1 * dy0 * ez0 + d2 * dy0 * dz0 + d3 * ey0 * dz0)
	         + dx2 * (d4 * ey0 * ez0 + d5 * dy0 * ez0 + d6 * dy0 * dz0 + d7 * ey0 * dz0);

	d = cellAt(x0,  y2 + 1, z0).distance;
	d0 = d - cellAt(x0,  y2, z0).distance;
	d4 = cellAt(x0,  y2 + 2, z0).distance - d;
	d = cellAt(x0,  y2 + 1, z0 + 1).distance;
	d1 = d - cellAt(x0,  y2, z0 + 1).distance;
	d5 = cellAt(x0,  y2 + 2, z0 + 1).distance - d;
	d = cellAt(x0 + 1, y2 + 1, z0 + 1).distance;
	d2 = d - cellAt(x0 + 1, y2, z0 + 1).distance;
	d6 = cellAt(x0 + 1, y2 + 2, z0 + 1).distance - d;
	d = cellAt(x0 + 1, y2 + 1, z0).distance;
	d3 = d - cellAt(x0 + 1, y2, z0).distance;
	d7 = cellAt(x0 + 1, y2 + 2, z0).distance - d;

	grad.y = ey2 * (d0 * ez0 * ex0 + d1 * dz0 * ex0 + d2 * dz0 * dx0 + d3 * ez0 * dx0)
	         + dy2 * (d4 * ez0 * ex0 + d5 * dz0 * ex0 + d6 * dz0 * dx0 + d7 * ez0 * dx0);

	d = cellAt(x0,  y0  , z2 + 1).distance;
	d0 = d - cellAt(x0,  y0  , z2).distance;
	d4 = cellAt(x0,  y0  , z2 + 2).distance - d;
	d = cellAt(x0 + 1, y0,  z2 + 1).distance;
	d1 = d - cellAt(x0 + 1, y0,  z2).distance;
	d5 = cellAt(x0 + 1, y0,  z2 + 2).distance - d;
	d = cellAt(x0 + 1, y0 + 1, z2 + 1).distance;
	d2 = d - cellAt(x0 + 1, y0 + 1, z2).distance;
	d6 = cellAt(x0 + 1, y0 + 1, z2 + 2).distance - d;
	d = cellAt(x0,  y0 + 1, z2 + 1).distance;
	d3 = d - cellAt(x0,  y0 + 1, z2).distance;
	d7 = cellAt(x0,  y0 + 1, z2 + 2).distance - d;

	grad.z = ez2 * (d0 * ex0 * ey0 + d1 * dx0 * ey0 + d2 * dx0 * dy0 + d3 * ex0 * dy0)
	         + dz2 * (d4 * ex0 * ey0 + d5 * dx0 * ey0 + d6 * dx0 * dy0 + d7 * ex0 * dy0);
}



void ApexMeshContractor::subdivide(physx::PxF32 spacing)
{
	const physx::PxF32 min2 = spacing * spacing;
	const physx::PxU32 numTris = mIndices.size() / 3;
	//const physx::PxU32 numVerts = mVertices.size();

	for (physx::PxU32 i = 0; i < numTris; i++)
	{
		const physx::PxU32 triNr = i;
		for (physx::PxU32 j = 0; j < 3; j++)
		{
			physx::PxU32 k = (j + 1) % 3;
			physx::PxU32 v0 = mIndices[3 * triNr + j];
			physx::PxU32 v1 = mIndices[3 * triNr + k];

			physx::PxVec3& p0 = mVertices[v0];
			physx::PxVec3& p1 = mVertices[v1];
			if ((p1 - p0).magnitudeSquared() < min2)
			{
				continue;
			}

			physx::PxU32 newVert = mVertices.size();
			mVertices.pushBack((p1 + p0) * 0.5f);

			physx::PxI32 adj;
			physx::PxI32 t0, t1, t2, t3;
			getButterfly(triNr, v0, v1, adj, t0, t1, t2, t3);

			physx::PxU32 newTri  = mIndices.size() / 3;
			int newAdj  = -1;

			int v = getOppositeVertex((physx::PxI32)triNr, v0, v1);
			PX_ASSERT(v >= 0);
			mIndices.pushBack(newVert);
			mIndices.pushBack(v1);
			mIndices.pushBack((physx::PxU32)v);
			if (adj >= 0)
			{
				v = getOppositeVertex(adj, v0, v1);
				PX_ASSERT(v >= 0);
				newAdj = (physx::PxI32)mIndices.size() / 3;
				mIndices.pushBack(newVert);
				mIndices.pushBack((physx::PxU32)v);
				mIndices.pushBack(v1);
			}

			replaceVertex((physx::PxI32)triNr, v1, newVert);
			replaceVertex(adj, v1, newVert);

			replaceNeighbor((physx::PxI32)triNr, t1, newTri);
			replaceNeighbor(adj, t3, (physx::PxU32)newAdj);
			replaceNeighbor(t1, (physx::PxI32)triNr, newTri);
			replaceNeighbor(t3, adj, (physx::PxU32)newAdj);

			mNeighbours.pushBack(newAdj);
			mNeighbours.pushBack(t1);
			mNeighbours.pushBack((physx::PxI32)triNr);
			if (adj >= 0)
			{
				mNeighbours.pushBack(adj);
				mNeighbours.pushBack(t3);
				mNeighbours.pushBack((physx::PxI32)newTri);
			}
		}
	}
}



void ApexMeshContractor::collapse(float spacing)
{
	const physx::PxF32 min2 = spacing * spacing;
	const physx::PxU32 numTris = mIndices.size() / 3;
	const physx::PxU32 numVerts = mVertices.size();

	const physx::PxU32 size = sizeof(physx::PxI32) * numVerts + sizeof(physx::PxI32) * numTris;
	void* memory = PX_ALLOC(size, PX_DEBUG_EXP("ApexMeshContractor"));
	memset(memory, 0, size);

	physx::PxI32* old2newVerts = (physx::PxI32*)memory;
	physx::PxI32* old2newTris = old2newVerts + numVerts;

	// collapse edges
	for (physx::PxU32 i = 0; i < numTris; i++)
	{
		const physx::PxU32 triNr = i;

		if (old2newTris[triNr] < 0)
		{
			continue;
		}

		for (physx::PxU32 j = 0; j < 3; j++)
		{
			const physx::PxU32 k = (j + 1) % 3;
			const physx::PxU32 v0 = mIndices[3 * triNr + j];
			const physx::PxU32 v1 = mIndices[3 * triNr + k];

			physx::PxVec3& p0 = mVertices[v0];
			physx::PxVec3& p1 = mVertices[v1];
			if ((p1 - p0).magnitudeSquared() > min2)
			{
				continue;
			}

			//if (iterNr == 14 && triNr == 388 && j == 0) {
			//	int foo = 0;
			//	checkConsistency();
			//}

			if (!legalCollapse((physx::PxI32)triNr, v0, v1))
			{
				continue;
			}

			physx::PxI32 adj, t0, t1, t2, t3;
			getButterfly(triNr, v0, v1, adj, t0, t1, t2, t3);

			mVertices[v0] = (p1 + p0) * 0.5f;

			physx::PxI32 prev = (physx::PxI32)triNr;
			physx::PxI32 t = t1;
			while (t >= 0)
			{
				replaceVertex(t, v1, v0);
				advanceAdjTriangle(v1, t, prev);
				if (t == (physx::PxI32)triNr)
				{
					break;
				}
			}

			for (physx::PxU32 k = 0; k < 3; k++)
			{
				if (t0 >= 0 && mNeighbours[3 * (physx::PxU32)t0 + k] == (physx::PxI32)triNr)
				{
					mNeighbours[3 * (physx::PxU32)t0 + k] = t1;
				}
				if (t1 >= 0 && mNeighbours[3 * (physx::PxU32)t1 + k] == (physx::PxI32)triNr)
				{
					mNeighbours[3 * (physx::PxU32)t1 + k] = t0;
				}
				if (t2 >= 0 && mNeighbours[3 * (physx::PxU32)t2 + k] == adj)
				{
					mNeighbours[3 * t2 + k] = t3;
				}
				if (t3 >= 0 && mNeighbours[3 * (physx::PxU32)t3 + k] == adj)
				{
					mNeighbours[3 * (physx::PxU32)t3 + k] = t2;
				}
			}

			old2newVerts[v1] = -1;
			old2newTris[triNr] = -1;
			if (adj >= 0)
			{
				old2newTris[adj] = -1;
			}

			//checkConsistency();

			break;
		}
	}

	// compress mesh
	physx::PxU32 vertNr = 0;
	for (physx::PxU32 i = 0; i < numVerts; i++)
	{
		if (old2newVerts[i] != -1)
		{
			old2newVerts[i] = (physx::PxI32)vertNr;
			mVertices[vertNr] = mVertices[i];
			vertNr++;
		}
	}
	mVertices.resize(vertNr);

	physx::PxU32 triNr = 0;
	for (physx::PxU32 i = 0; i < numTris; i++)
	{
		if (old2newTris[i] != -1)
		{
			old2newTris[i] = (physx::PxI32)triNr;
			for (physx::PxU32 j = 0; j < 3; j++)
			{
				mIndices[3 * triNr + j] = (physx::PxU32)old2newVerts[mIndices[3 * i + j]];
				mNeighbours[3 * triNr + j] = mNeighbours[3 * i + j];
			}
			triNr++;
		}
	}
	mIndices.resize(triNr * 3);
	mNeighbours.resize(triNr * 3);
	for (physx::PxU32 i = 0; i < mNeighbours.size(); i++)
	{
		PX_ASSERT(mNeighbours[i] != -1);
		PX_ASSERT(old2newTris[mNeighbours[i]] != -1);
		mNeighbours[i] = old2newTris[mNeighbours[i]];
	}

	PX_FREE(memory);
}



void ApexMeshContractor::getButterfly(physx::PxU32 triNr, physx::PxU32 v0, physx::PxU32 v1, physx::PxI32& adj, physx::PxI32& t0, physx::PxI32& t1, physx::PxI32& t2, physx::PxI32& t3) const
{
	t0 = -1;
	t1 = -1;
	t2 = -1;
	t3 = -1;
	adj = -1;
	for (physx::PxU32 i = 0; i < 3; i++)
	{
		physx::PxI32 n = mNeighbours[triNr * 3 + i];

		if (n < 0)
		{
			continue;
		}

		if (triangleContains(n, v0) && triangleContains(n, v1))
		{
			adj = n;
		}
		else if (triangleContains(n, v0))
		{
			t0 = n;
		}
		else if (triangleContains(n, v1))
		{
			t1 = n;
		}
	}
	if (adj >= 0)
	{
		for (physx::PxU32 i = 0; i < 3; i++)
		{
			physx::PxI32 n = mNeighbours[adj * 3 + i];

			if (n < 0 || (physx::PxU32)n == triNr)
			{
				continue;
			}

			if (triangleContains(n, v0))
			{
				t2 = n;
			}
			else if (triangleContains(n, v1))
			{
				t3 = n;
			}
		}
	}
}



physx::PxI32 ApexMeshContractor::getOppositeVertex(physx::PxI32 t, physx::PxU32 v0, physx::PxU32 v1) const
{
	if (t < 0 || 3 * t + 2 >= (physx::PxI32)mIndices.size())
	{
		return -1;
	}
	const physx::PxU32 i = 3 * (physx::PxU32)t;
	if (mIndices[i + 0] != v0 && mIndices[i + 0] != v1)
	{
		return (physx::PxI32)mIndices[i + 0];
	}
	if (mIndices[i + 1] != v0 && mIndices[i + 1] != v1)
	{
		return (physx::PxI32)mIndices[i + 1];
	}
	if (mIndices[i + 2] != v0 && mIndices[i + 2] != v1)
	{
		return (physx::PxI32)mIndices[i + 2];
	}
	return -1;
}



void ApexMeshContractor::replaceVertex(physx::PxI32 t, physx::PxU32 vOld, physx::PxU32 vNew)
{
	if (t < 0 || 3 * t + 2 >= (physx::PxI32)mIndices.size())
	{
		return;
	}
	const physx::PxU32 i = 3 * (physx::PxU32)t;
	if (mIndices[i + 0] == vOld)
	{
		mIndices[i + 0] = vNew;
	}
	if (mIndices[i + 1] == vOld)
	{
		mIndices[i + 1] = vNew;
	}
	if (mIndices[i + 2] == vOld)
	{
		mIndices[i + 2] = vNew;
	}
}



void ApexMeshContractor::replaceNeighbor(physx::PxI32 t, physx::PxI32 nOld, physx::PxU32 nNew)
{
	if (t < 0 || 3 * t + 2 >= (physx::PxI32)mNeighbours.size())
	{
		return;
	}
	const physx::PxU32 i = 3 * (physx::PxU32)t;
	if (mNeighbours[i + 0] == nOld)
	{
		mNeighbours[i + 0] = (physx::PxI32)nNew;
	}
	if (mNeighbours[i + 1] == nOld)
	{
		mNeighbours[i + 1] = (physx::PxI32)nNew;
	}
	if (mNeighbours[i + 2] == nOld)
	{
		mNeighbours[i + 2] = (physx::PxI32)nNew;
	}
}


bool ApexMeshContractor::triangleContains(physx::PxI32 t, physx::PxU32 v) const
{
	if (t < 0 || 3 * t + 2 >= (physx::PxI32)mIndices.size())
	{
		return false;
	}
	const physx::PxU32 i = 3 * (physx::PxU32)t;
	return mIndices[i] == v || mIndices[i + 1] == v || mIndices[i + 2] == v;
}



bool ApexMeshContractor::legalCollapse(physx::PxI32 triNr, physx::PxU32 v0, physx::PxU32 v1) const
{
	physx::PxI32 adj, t0, t1, t2, t3;
	getButterfly((physx::PxU32)triNr, v0, v1, adj, t0, t1, t2, t3);

	// both of the end vertices must be completely surrounded by triangles
	physx::PxI32 prev = triNr;
	physx::PxI32 t = t0;
	while (t >= 0)
	{
		advanceAdjTriangle(v0, t, prev);
		if (t == triNr)
		{
			break;
		}
	}

	if (t != triNr)
	{
		return false;
	}

	prev = triNr;
	t = t1;
	while (t >= 0)
	{
		advanceAdjTriangle(v1, t, prev);
		if (t == triNr)
		{
			break;
		}
	}
	if (t != triNr)
	{
		return false;
	}

	// not legal if there exists v != v0,v1 with (v0,v) and (v1,v) edges
	// but (v,v0,v1) is not a triangle

	prev = triNr;
	t = t1;
	int adjV = getOppositeVertex(triNr, v0, v1);
	while (t >= 0)
	{
		if (t != t1)
		{
			int prev2 = prev;
			int t2 = t;
			while (t2 >= 0)
			{
				if (triangleContains(t2, v0))
				{
					return false;
				}
				advanceAdjTriangle((physx::PxU32)adjV, t2, prev2);
				if (t2 == t)
				{
					break;
				}
			}
		}
		adjV = getOppositeVertex(t, v1, (physx::PxU32)adjV);
		advanceAdjTriangle(v1, t, prev);
		if (t == triNr || t == adj)
		{
			break;
		}
	}

	if (areNeighbors(t0, t1))
	{
		return false;
	}
	if (areNeighbors(t2, t3))
	{
		return false;
	}
	return true;
}



void ApexMeshContractor::advanceAdjTriangle(physx::PxU32 v, physx::PxI32& t, physx::PxI32& prev) const
{
	physx::PxI32 next = -1;
	for (physx::PxU32 i = 0; i < 3; i++)
	{
		physx::PxI32 n = mNeighbours[3 * t + i];
		if (n >= 0 && n != prev && triangleContains(n, v))
		{
			next = n;
		}
	}
	prev = t;
	t = next;
}



bool ApexMeshContractor::areNeighbors(physx::PxI32 t0, physx::PxI32 t1) const
{
	if (t0 < 0 || 3 * t0 + 2 >= (physx::PxI32)mNeighbours.size())
	{
		return false;
	}
	const physx::PxU32 i = 3 * (physx::PxU32)t0;
	return mNeighbours[i] == t1 || mNeighbours[i + 1] == t1 || mNeighbours[i + 2] == t1;
}



physx::PxF32 ApexMeshContractor::findMin(const physx::PxVec3& p, const physx::PxVec3& maxDisp) const
{
	const physx::PxU32 numSteps = 20;
	const physx::PxF32 dt = 1.0f / numSteps;

	physx::PxF32 minDist = physx::PxAbs(interpolateDistanceAt(p));
	float minT = 0.0f;

	for (physx::PxU32 i = 1; i < numSteps; i++)
	{
		const physx::PxF32 t = i * dt;
		physx::PxF32 dist = physx::PxAbs(interpolateDistanceAt(p + maxDisp * t));
		if (dist < minDist)
		{
			minT = t;
			// PH: isn't this missing?
			//minDist = dist;
		}
	}
	return minT;
}



physx::PxF32 ApexMeshContractor::interpolateDistanceAt(const physx::PxVec3& pos) const
{
	const physx::PxVec3 p = pos - mOrigin;
	const physx::PxF32 h = mCellSize;
	const physx::PxF32 h1 = 1.0f / h;

	physx::PxI32 x = (physx::PxI32)PxFloor(p.x * h1);
	const physx::PxF32 dx = (p.x - h * x) * h1;
	const physx::PxF32 ex = 1.0f - dx;
	physx::PxI32 y = (physx::PxI32)PxFloor(p.y * h1);
	const physx::PxF32 dy = (p.y - h * y) * h1;
	const physx::PxF32 ey = 1.0f - dy;
	physx::PxI32 z = (physx::PxI32)PxFloor(p.z * h1);
	const physx::PxF32 dz = (p.z - h * z) * h1;
	const physx::PxF32 ez = 1.0f - dz;

	if (x < 0)
	{
		x = 0;
	}
	if (x + 1 >= (physx::PxI32)mNumX)
	{
		x = (physx::PxI32)mNumX - 2;    // todo: handle boundary correctly
	}
	if (y < 0)
	{
		y = 0;
	}
	if (y + 1 >= (physx::PxI32)mNumY)
	{
		y = (physx::PxI32)mNumY - 2;
	}
	if (z < 0)
	{
		z = 0;
	}
	if (z + 1 >= (physx::PxI32)mNumZ)
	{
		z = (physx::PxI32)mNumZ - 2;
	}

	physx::PxF32 d0, d1, d2, d3, d4, d5, d6, d7;
	d0 = constCellAt(x, y,  z).distance;
	d4 = constCellAt(x + 1, y,  z).distance;
	d1 = constCellAt(x, y + 1, z).distance;
	d5 = constCellAt(x + 1, y + 1, z).distance;
	d2 = constCellAt(x, y + 1, z + 1).distance;
	d6 = constCellAt(x + 1, y + 1, z + 1).distance;
	d3 = constCellAt(x, y,  z + 1).distance;
	d7 = constCellAt(x + 1, y,  z + 1).distance;

	physx::PxF32 dist = ex * (d0 * ey * ez + d1 * dy * ez + d2 * dy * dz + d3 * ey * dz)
	                    + dx * (d4 * ey * ez + d5 * dy * ez + d6 * dy * dz + d7 * ey * dz);

	return dist;
}




struct DistTriangle
{
	physx::PxI32 triNr;
	physx::PxF32 dist;
	bool operator < (const DistTriangle& t) const
	{
		return dist > t.dist;
	}
};

void ApexMeshContractor::collectNeighborhood(physx::PxI32 triNr, physx::PxF32 radius, physx::PxU32 newMark, physx::Array<physx::PxI32> &tris, physx::Array<physx::PxF32> &dists, physx::PxU32* triMarks) const
{
	tris.clear();
	dists.clear();
	ApexBinaryHeap<DistTriangle> heap;

	DistTriangle dt;
	dt.triNr = triNr;
	dt.dist = 0.0f;
	heap.push(dt);

	while (!heap.isEmpty())
	{
		heap.pop(dt);

		if (triMarks[dt.triNr] == newMark)
		{
			continue;
		}
		triMarks[dt.triNr] = newMark;

		tris.pushBack(dt.triNr);
		dists.pushBack(-dt.dist);
		physx::PxVec3 center;
		getTriangleCenter(dt.triNr, center);

		for (physx::PxU32 i = 0; i < 3; i++)
		{
			physx::PxI32 adj = mNeighbours[3 * dt.triNr + i];
			if (adj < 0)
			{
				continue;
			}
			if (triMarks[adj] == newMark)
			{
				continue;
			}

			physx::PxVec3 adjCenter;
			getTriangleCenter(adj, adjCenter);
			DistTriangle adjDt;
			adjDt.triNr = adj;
			adjDt.dist = dt.dist - (adjCenter - center).magnitude();	// it is a max heap, we need the smallest dist first
			//adjDt.dist = adjCenter.distance(center) - dt.dist;	// PH: inverting heap distance, now it's a min heap
			if (-adjDt.dist > radius)
			{
				continue;
			}

			heap.push(adjDt);
		}
	}
}



void ApexMeshContractor::getTriangleCenter(physx::PxI32 triNr, physx::PxVec3& center) const
{
	const physx::PxVec3& p0 = mVertices[mIndices[3 * (physx::PxU32)triNr + 0]];
	const physx::PxVec3& p1 = mVertices[mIndices[3 * (physx::PxU32)triNr + 1]];
	const physx::PxVec3& p2 = mVertices[mIndices[3 * (physx::PxU32)triNr + 2]];
	center = (p0 + p1 + p2) / 3.0f;
}

//------------------------------------------------------------------------------------
float ApexMeshContractor::curvatureAt(int triNr, int v)
{
	int prev = -1;
	int t = triNr;
	physx::PxVec3 n0, n1;
	float minDot = 1.0f;
	while (t >= 0)
	{
		advanceAdjTriangle((physx::PxU32)v, t, prev);
		if (t < 0 || t == triNr)
		{
			break;
		}

		physx::PxVec3& p0 = mVertices[mIndices[3 * (physx::PxU32)prev]];
		physx::PxVec3& p1 = mVertices[mIndices[3 * (physx::PxU32)prev + 1]];
		physx::PxVec3& p2 = mVertices[mIndices[3 * (physx::PxU32)prev + 2]];
		n0 = (p1 - p0).cross(p2 - p0);
		n0.normalize();

		physx::PxVec3& q0 = mVertices[mIndices[3 * (physx::PxU32)t]];
		physx::PxVec3& q1 = mVertices[mIndices[3 * (physx::PxU32)t + 1]];
		physx::PxVec3& q2 = mVertices[mIndices[3 * (physx::PxU32)t + 2]];
		n1 = (q1 - q0).cross(q2 - q0);
		n1.normalize();
		float dot = n0.dot(n1);
		if (dot < minDot)
		{
			minDot = dot;
		}
	}
	return (1.0f - minDot) * 0.5f;
}

}
} // end namespace physx::apex
