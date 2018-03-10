/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_ISO_MESH_H
#define APEX_ISO_MESH_H

#include "NxApexUsingNamespace.h"
#include "PsShare.h"
#include "PsUserAllocated.h"
#include "PsArray.h"
#include "foundation/PxBounds3.h"

namespace physx
{
namespace apex
{

class IProgressListener;

class ApexIsoMesh : public physx::UserAllocated
{
public:
	ApexIsoMesh(physx::PxU32 isoGridSubdivision, PxU32 keepNBiggestMeshes, bool discardInnerMeshes);
	~ApexIsoMesh();

	void setBound(const physx::PxBounds3& bound);
	void clear();
	void clearTemp();
	void addTriangle(const PxVec3& v0, const PxVec3& v1, const PxVec3& v2);
	bool update(IProgressListener* progress);


	physx::PxU32 getNumVertices() const
	{
		return mIsoVertices.size();
	}
	const physx::PxVec3& getVertex(physx::PxU32 index) const
	{
		PX_ASSERT(index < mIsoVertices.size());
		return mIsoVertices[index];
	}

	physx::PxU32 getNumTriangles() const
	{
		return mIsoTriangles.size();
	}
	void getTriangle(physx::PxU32 index, physx::PxU32& v0, physx::PxU32& v1, physx::PxU32& v2) const;
private:
	// settable parameters
	PxU32 mIsoGridSubdivision;
	PxU32 mKeepNBiggestMeshes;
	bool  mDiscardInnerMeshes;
	PxBounds3 mBound;

	bool generateMesh(IProgressListener* progress);
	bool interpolate(float d0, float d1, const physx::PxVec3& pos0, const physx::PxVec3& pos1, physx::PxVec3& pos);
	bool findNeighbors(IProgressListener* progress);
	void removeLayers();
	PxU32 floodFill(physx::PxU32 triangleNr, physx::PxU32 groupNr);

	void removeTrisAndVerts();

	// non-settable parameters (deducted from the ones you can set)
	physx::PxF32 mCellSize;
	physx::PxF32 mThickness;
	physx::PxVec3 mOrigin;
	physx::PxI32 mNumX, mNumY, mNumZ;
	const physx::PxF32 mIsoValue;


	struct IsoCell
	{
		void init()
		{
			density = 0.0f;
			vertNrX = -1;
			vertNrY = -1;
			vertNrZ = -1;
			firstTriangle = -1;
			numTriangles = 0;
		}
		physx::PxF32 density;
		physx::PxI32 vertNrX;
		physx::PxI32 vertNrY;
		physx::PxI32 vertNrZ;
		physx::PxI32 firstTriangle;
		physx::PxI32 numTriangles;
	};
	physx::Array<IsoCell> mGrid;
	inline IsoCell& cellAt(int xi, int yi, int zi)
	{
		physx::PxU32 index = (physx::PxU32)(((xi * mNumY) + yi) * mNumZ + zi);
		PX_ASSERT(index < mGrid.size());
		return mGrid[index];
	}


	struct IsoTriangle
	{
		void init()
		{
			vertexNr[0] = -1;
			vertexNr[1] = -1;
			vertexNr[2] = -1;
			adjTriangles[0] = -1;
			adjTriangles[1] = -1;
			adjTriangles[2] = -1;
			groupNr = -1;
			deleted = false;
		}
		void set(physx::PxI32 v0, physx::PxI32 v1, physx::PxI32 v2, physx::PxI32 cubeX, physx::PxI32 cubeY, physx::PxI32 cubeZ)
		{
			init();
			vertexNr[0] = v0;
			vertexNr[1] = v1;
			vertexNr[2] = v2;
			this->cubeX = cubeX;
			this->cubeY = cubeY;
			this->cubeZ = cubeZ;
		}
		void addNeighbor(physx::PxI32 triangleNr)
		{
			if (adjTriangles[0] == -1)
			{
				adjTriangles[0] = triangleNr;
			}
			else if (adjTriangles[1] == -1)
			{
				adjTriangles[1] = triangleNr;
			}
			else if (adjTriangles[2] == -1)
			{
				adjTriangles[2] = triangleNr;
			}
		}

		physx::PxI32 vertexNr[3];
		physx::PxI32 cubeX, cubeY, cubeZ;
		physx::PxI32 adjTriangles[3];
		physx::PxI32 groupNr;
		bool deleted;
	};

	struct IsoEdge
	{
		void set(int newV0, int newV1, int newTriangle)
		{
			if (newV0 < newV1)
			{
				v0 = newV0;
				v1 = newV1;
			}
			else
			{
				v0 = newV1;
				v1 = newV0;
			}
			triangleNr = newTriangle;
		}
		PX_INLINE bool operator < (const IsoEdge& e) const
		{
			if (v0 < e.v0)
			{
				return true;
			}
			if (v0 > e.v0)
			{
				return false;
			}
			return (v1 < e.v1);
		}

		PX_INLINE bool operator()(const IsoEdge& e1, const IsoEdge& e2) const
		{
			return e1 < e2;
		}

		PX_INLINE bool operator == (const IsoEdge& e) const
		{
			return v0 == e.v0 && v1 == e.v1;
		}

		int v0, v1;
		int triangleNr;
	};

	physx::Array<PxVec3> mIsoVertices;
	physx::Array<IsoTriangle> mIsoTriangles;
	physx::Array<IsoEdge> mIsoEdges;

	// evil, should not be used
	ApexIsoMesh& operator=(const ApexIsoMesh&);
};

}
} // end namespace physx::apex

#endif // APEX_ISO_MESH_H
