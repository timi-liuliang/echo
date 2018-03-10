/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_TETRAHEDRALIZER_H
#define APEX_TETRAHEDRALIZER_H

#include "NxApexDefs.h"
#include "foundation/PxBounds3.h"

#include "PsShare.h"
#include "PsArray.h"
#include "PsUserAllocated.h"

#define TETRAHEDRALIZER_DEBUG_RENDERING 0

namespace physx
{
namespace apex
{

class ApexMeshHash;
class IProgressListener;

class ApexTetrahedralizer : public physx::UserAllocated
{
public:
	ApexTetrahedralizer(physx::PxU32 subdivision);
	~ApexTetrahedralizer();


	void registerVertex(const physx::PxVec3& v);
	void registerTriangle(physx::PxU32 i0, physx::PxU32 i1, physx::PxU32 i2);
	void endRegistration(IProgressListener* progress);

	physx::PxU32 getNumVertices() const
	{
		return mVertices.size();
	}
	void getVertices(physx::PxVec3* data);
	physx::PxU32 getNumIndices() const
	{
		return mTetras.size() * 4;
	}
	void getIndices(physx::PxU32* data);

private:
	struct FullTetrahedron;

	void weldVertices();
	void delaunayTetrahedralization(IProgressListener* progress);
	physx::PxI32 findSurroundingTetra(physx::PxU32 startTetra, const physx::PxVec3& p) const;
	physx::PxF32 retriangulate(const physx::PxU32 tetraNr, physx::PxU32 vertexNr);
	physx::PxU32 swapTetrahedra(physx::PxU32 startTet, IProgressListener* progress);
	bool swapEdge(physx::PxU32 v0, physx::PxU32 v1);
	bool removeOuterTetrahedra(IProgressListener* progress);

	void updateCircumSphere(FullTetrahedron& tetra) const;
	bool pointInCircumSphere(FullTetrahedron& tetra, const physx::PxVec3& p) const;
	bool pointInTetra(const FullTetrahedron& tetra, const physx::PxVec3& p) const;

	physx::PxF32 getTetraVolume(const FullTetrahedron& tetra) const;
	physx::PxF32 getTetraVolume(physx::PxI32 v0, physx::PxI32 v1, physx::PxI32 v2, physx::PxI32 v3) const;
	physx::PxF32 getTetraQuality(const FullTetrahedron& tetra) const;
	physx::PxF32 getTetraLongestEdge(const FullTetrahedron& tetra) const;

	bool triangleContainsVertexNr(physx::PxU32* triangle, physx::PxU32* vertexNumber, physx::PxU32 nbVertices);

	void compressTetrahedra(bool trashNeighbours);
	void compressVertices();

	inline bool isFarVertex(physx::PxU32 vertNr) const
	{
		return mFirstFarVertex <= vertNr && vertNr <= mLastFarVertex;
	}

	ApexMeshHash* mMeshHash;

	physx::PxU32 mSubdivision;

	physx::PxBounds3 mBound;
	physx::PxF32 mBoundDiagonal;

	struct TetraVertex
	{
		inline void init(const physx::PxVec3& pos, physx::PxU32  flags)
		{
			this->pos = pos;
			this->flags = flags;
		}
		inline bool isDeleted() const
		{
			return flags == (physx::PxU32)0xdeadf00d;
		}
		inline void markDeleted()
		{
			flags = 0xdeadf00d;
		}

		physx::PxVec3 pos;
		physx::PxU32 flags;
	};
	class LessInOneAxis
	{
		physx::PxU32 mAxis;
	public:
		LessInOneAxis(physx::PxU32 axis) : mAxis(axis) {}
		bool operator()(const TetraVertex& v1, const TetraVertex& v2) const
		{
			return v1.pos[mAxis] < v2.pos[mAxis];
		}
	};

	struct TetraEdge
	{
		void init(physx::PxI32 v0, physx::PxI32 v1, physx::PxI32 tetra, int neighborNr = -1)
		{
			this->tetraNr = (physx::PxU32)tetra;
			this->neighborNr = neighborNr;
			PX_ASSERT(v0 != v1);
			vNr0 = (physx::PxU32)physx::PxMin(v0, v1);
			vNr1 = (physx::PxU32)physx::PxMax(v0, v1);
		}
		PX_INLINE bool operator <(const TetraEdge& e) const
		{
			if (vNr0 < e.vNr0)
			{
				return true;
			}
			if (vNr0 > e.vNr0)
			{
				return false;
			}
			if (vNr1 < e.vNr1)
			{
				return true;
			}
			if (vNr1 > e.vNr1)
			{
				return false;
			}
			return (neighborNr < e.neighborNr);
		}
		PX_INLINE bool operator()(const TetraEdge& e1, const TetraEdge& e2) const
		{
			return e1 < e2;
		}
		bool operator ==(TetraEdge& e) const
		{
			return vNr0 == e.vNr0 && vNr1 == e.vNr1;
		}

		bool allEqual(TetraEdge& e) const
		{
			return (vNr0 == e.vNr0) && (vNr1 == e.vNr1) && (neighborNr == e.neighborNr);
		}
		physx::PxU32 vNr0, vNr1;
		physx::PxU32 tetraNr;
		physx::PxI32 neighborNr;
	};


	struct TetraEdgeList
	{
		void add(TetraEdge& edge)
		{
			mEdges.pushBack(edge);
		}
		void insert(physx::PxU32 pos, TetraEdge& edge);
		physx::PxU32 numEdges()
		{
			return mEdges.size();
		}
		void sort();
		int  findEdge(int v0, int v1);
		int  findEdgeTetra(int v0, int v1, int tetraNr);
		TetraEdge& operator[](unsigned i)
		{
			return mEdges[i];
		}
		const TetraEdge& operator[](unsigned i) const
		{
			return mEdges[i];
		}

		physx::Array<TetraEdge> mEdges;
	};


	struct FullTetrahedron
	{
		void init()
		{
			vertexNr[0] = vertexNr[1] = vertexNr[2] = vertexNr[3] = -1;
			neighborNr[0] = neighborNr[1] = neighborNr[2] = neighborNr[3] = -1;
			center = physx::PxVec3(0.0f);
			radiusSquared = 0.0f;
			quality = 0;
			bCircumSphereDirty = 1;
			bDeleted = 0;
		}
		void set(physx::PxI32 v0, physx::PxI32 v1, physx::PxI32 v2, physx::PxI32 v3)
		{
			vertexNr[0] = v0;
			vertexNr[1] = v1;
			vertexNr[2] = v2;
			vertexNr[3] = v3;
			neighborNr[0] = neighborNr[1] = neighborNr[2] = neighborNr[3] = -1;
			center = physx::PxVec3(0.0f);
			radiusSquared = 0.0f;
			quality = 0;
			bCircumSphereDirty = 1;
			bDeleted = 0;
		}
		bool operator==(const FullTetrahedron& t) const
		{
			return
			    (vertexNr[0] == t.vertexNr[0]) &&
			    (vertexNr[1] == t.vertexNr[1]) &&
			    (vertexNr[2] == t.vertexNr[2]) &&
			    (vertexNr[3] == t.vertexNr[3]);
		}

		bool containsVertex(physx::PxI32 nr) const
		{
			return (vertexNr[0] == nr || vertexNr[1] == nr || vertexNr[2] == nr || vertexNr[3] == nr);
		}
		void replaceVertex(int nr, int newNr)
		{
			if (vertexNr[0] == nr)
			{
				vertexNr[0] = newNr;
			}
			else if (vertexNr[1] == nr)
			{
				vertexNr[1] = newNr;
			}
			else if (vertexNr[2] == nr)
			{
				vertexNr[2] = newNr;
			}
			else if (vertexNr[3] == nr)
			{
				vertexNr[3] = newNr;
			}
			else
			{
				PX_ASSERT(0);
			}
		}
		void get2OppositeVertices(int vNr0, int vNr1, int& vNr2, int& vNr3)
		{
			int v[4], p = 0;
			if (vertexNr[0] != vNr0 && vertexNr[0] != vNr1)
			{
				v[p++] = vertexNr[0];
			}
			if (vertexNr[1] != vNr0 && vertexNr[1] != vNr1)
			{
				v[p++] = vertexNr[1];
			}
			if (vertexNr[2] != vNr0 && vertexNr[2] != vNr1)
			{
				v[p++] = vertexNr[2];
			}
			if (vertexNr[3] != vNr0 && vertexNr[3] != vNr1)
			{
				v[p++] = vertexNr[3];
			}
			PX_ASSERT(p == 2);
			vNr2 = v[0];
			vNr3 = v[1];
		}
		void get3OppositeVertices(int vNr, int& vNr0, int& vNr1, int& vNr2)
		{
			if (vNr == vertexNr[0])
			{
				vNr0 = vertexNr[1];
				vNr1 = vertexNr[2];
				vNr2 = vertexNr[3];
			}
			else if (vNr == vertexNr[1])
			{
				vNr0 = vertexNr[2];
				vNr1 = vertexNr[0];
				vNr2 = vertexNr[3];
			}
			else if (vNr == vertexNr[2])
			{
				vNr0 = vertexNr[0];
				vNr1 = vertexNr[1];
				vNr2 = vertexNr[3];
			}
			else if (vNr == vertexNr[3])
			{
				vNr0 = vertexNr[2];
				vNr1 = vertexNr[1];
				vNr2 = vertexNr[0];
			}
			else
			{
				PX_ASSERT(0);
			}
		}
		int getOppositeVertex(int vNr0, int vNr1, int vNr2)
		{
			if (vertexNr[0] != vNr0 && vertexNr[0] != vNr1 && vertexNr[0] != vNr2)
			{
				return vertexNr[0];
			}
			if (vertexNr[1] != vNr0 && vertexNr[1] != vNr1 && vertexNr[1] != vNr2)
			{
				return vertexNr[1];
			}
			if (vertexNr[2] != vNr0 && vertexNr[2] != vNr1 && vertexNr[2] != vNr2)
			{
				return vertexNr[2];
			}
			if (vertexNr[3] != vNr0 && vertexNr[3] != vNr1 && vertexNr[3] != vNr2)
			{
				return vertexNr[3];
			}
			PX_ASSERT(0);
			return -1;
		}
		int sideOf(int vNr0, int vNr1, int vNr2)
		{
			if (vertexNr[0] != vNr0 && vertexNr[0] != vNr1 && vertexNr[0] != vNr2)
			{
				return 0;
			}
			if (vertexNr[1] != vNr0 && vertexNr[1] != vNr1 && vertexNr[1] != vNr2)
			{
				return 1;
			}
			if (vertexNr[2] != vNr0 && vertexNr[2] != vNr1 && vertexNr[2] != vNr2)
			{
				return 2;
			}
			if (vertexNr[3] != vNr0 && vertexNr[3] != vNr1 && vertexNr[3] != vNr2)
			{
				return 3;
			}
			PX_ASSERT(0);
			return -1;
		}
		inline int neighborNrOf(int vNr0, int vNr1, int vNr2)
		{
			PX_ASSERT(containsVertex(vNr0));
			PX_ASSERT(containsVertex(vNr1));
			PX_ASSERT(containsVertex(vNr2));
			if (vertexNr[0] != vNr0 && vertexNr[0] != vNr1 && vertexNr[0] != vNr2)
			{
				return 0;
			}
			if (vertexNr[1] != vNr0 && vertexNr[1] != vNr1 && vertexNr[1] != vNr2)
			{
				return 1;
			}
			if (vertexNr[2] != vNr0 && vertexNr[2] != vNr1 && vertexNr[2] != vNr2)
			{
				return 2;
			}
			if (vertexNr[3] != vNr0 && vertexNr[3] != vNr1 && vertexNr[3] != vNr2)
			{
				return 3;
			}
			PX_ASSERT(0);
			return 0;
		}
		inline int& neighborOf(int vNr0, int vNr1, int vNr2)
		{
			return neighborNr[neighborNrOf(vNr0, vNr1, vNr2)];
		}

		inline bool onSurface()
		{
			return neighborNr[0] < 0 || neighborNr[1] < 0 || neighborNr[2] < 0 || neighborNr[3] < 0;
		}
		// representation
		physx::PxVec3 center;
		physx::PxI32	vertexNr[4];
		physx::PxI32	neighborNr[4];
		physx::PxF32	radiusSquared;
		physx::PxU32	quality : 10;
		physx::PxU32	bDeleted : 1;
		physx::PxU32	bCircumSphereDirty : 1;

		// static
		static const physx::PxU32 sideIndices[4][3];
	};

	physx::Array<FullTetrahedron> mTetras;

	physx::PxU32 mFirstFarVertex;
	physx::PxU32 mLastFarVertex;

	physx::Array<TetraVertex> mVertices;
	physx::Array<physx::PxU32> mIndices;

	// temporary indices, that way we don't allocate this buffer all the time
	physx::Array<physx::PxU32> mTempItemIndices;

#if TETRAHEDRALIZER_DEBUG_RENDERING
public:
	physx::Array<physx::PxVec3> debugLines;
	physx::Array<physx::PxVec3> debugBounds;
	physx::Array<physx::PxVec3> debugTetras;
#endif
};

}
} // end namespace physx::apex

#endif // APEX_TETRAHEDRALIZER_H
