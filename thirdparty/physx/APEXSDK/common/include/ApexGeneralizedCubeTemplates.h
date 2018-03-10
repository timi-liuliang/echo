/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEX_GENERALIZED_CUBE_TEMPLATES_H__
#define __APEX_GENERALIZED_CUBE_TEMPLATES_H__

#include "PsShare.h"
#include "PsUserAllocated.h"
#include "PsArray.h"

#include "foundation/PxVec3.h"

namespace physx
{
namespace apex
{

class ApexGeneralizedCubeTemplates : public physx::UserAllocated
{
public:
	ApexGeneralizedCubeTemplates();

	void getTriangles(const int groups[8], physx::Array<physx::PxI32> &indices);

private:

	enum AllConsts
	{
		GEN_NUM_SUB_CELLS	= 6,
		SUB_GRID_LEN		= GEN_NUM_SUB_CELLS,
		SUB_GRID_LEN_2		= GEN_NUM_SUB_CELLS * GEN_NUM_SUB_CELLS,
		NUM_SUB_CELLS		= GEN_NUM_SUB_CELLS * GEN_NUM_SUB_CELLS * GEN_NUM_SUB_CELLS,
		NUM_CUBE_VERTS		= 19,
		NUM_CASES_3			= 6561, // 2^8
	};
	struct GenSubCell
	{
		inline void init()
		{
			group = -1;
			marked = false;
		}
		physx::PxI32 group;
		bool marked;
	};

	struct GenCoord
	{
		void init(physx::PxI32 xi, physx::PxI32 yi, physx::PxI32 zi)
		{
			this->xi = xi;
			this->yi = yi;
			this->zi = zi;
		}
		bool operator == (const GenCoord& c) const
		{
			return xi == c.xi && yi == c.yi && zi == c.zi;
		}

		physx::PxI32 xi, yi, zi;
	};


	void createLookupTable3();
	void setCellGroups(const physx::PxI32 groups[8]);
	void splitDisconnectedGroups();
	void findVertices();
	void createTriangles(physx::Array<physx::PxI32>& currentIndices);
	bool isEdge(const GenCoord& c, physx::PxI32 dim, physx::PxI32 group0, physx::PxI32 group1);


	inline physx::PxU32 cellNr(physx::PxU32 x, physx::PxU32 y, physx::PxU32 z)
	{
		return x * SUB_GRID_LEN_2 + y * SUB_GRID_LEN + z;
	}

	inline physx::PxI32 groupAt(physx::PxI32 x, physx::PxI32 y, physx::PxI32 z)
	{
		if (x < 0 || x >= SUB_GRID_LEN || y < 0 || y >= SUB_GRID_LEN || z < 0 || z >= SUB_GRID_LEN)
		{
			return -1;
		}
		return mSubGrid[x * SUB_GRID_LEN_2 + y * SUB_GRID_LEN + z].group;
	}

	inline bool vertexMarked(const GenCoord& c)
	{
		if (c.xi < 0 || c.xi > SUB_GRID_LEN || c.yi < 0 || c.yi > SUB_GRID_LEN || c.zi < 0 || c.zi > SUB_GRID_LEN)
		{
			return false;
		}
		return mVertexMarked[c.xi][c.yi][c.zi];
	}

	inline void markVertex(const GenCoord& c)
	{
		if (c.xi < 0 || c.xi > SUB_GRID_LEN || c.yi < 0 || c.yi > SUB_GRID_LEN || c.zi < 0 || c.zi > SUB_GRID_LEN)
		{
			return;
		}
		mVertexMarked[c.xi][c.yi][c.zi] = true;
	}




	physx::PxF32 mBasis[NUM_SUB_CELLS][8];
	physx::PxVec3 mVertPos[NUM_CUBE_VERTS];
	int mVertexAt[SUB_GRID_LEN + 1][SUB_GRID_LEN + 1][SUB_GRID_LEN + 1];
	bool mVertexMarked[SUB_GRID_LEN + 1][SUB_GRID_LEN + 1][SUB_GRID_LEN + 1];

	GenSubCell mSubGrid[NUM_SUB_CELLS];
	physx::PxI32 mFirst3[NUM_CASES_3];	// 3^8

	physx::Array<physx::PxI32> mLookupIndices3;

	physx::PxI32 mFirstPairVertex[8][8];
	GenCoord mFirstPairCoord[8][8];

};

}
} // end namespace physx::apex

#endif
