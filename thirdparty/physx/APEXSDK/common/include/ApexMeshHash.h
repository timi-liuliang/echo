/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_MESH_HASH_H
#define APEX_MESH_HASH_H

#include "NxApexDefs.h"

#include "PsShare.h"
#include "PsUserAllocated.h"
#include "PsArray.h"

#include "foundation/PxVec3.h"

namespace physx
{
namespace apex
{

struct MeshHashRoot
{
	physx::PxI32 first;
	physx::PxU32 timeStamp;
};

struct MeshHashEntry
{
	physx::PxI32 next;
	physx::PxU32 itemIndex;
};


class ApexMeshHash : public physx::UserAllocated
{
public:
	ApexMeshHash();
	~ApexMeshHash();

	void   setGridSpacing(physx::PxF32 spacing);
	physx::PxF32 getGridSpacing()
	{
		return 1.0f / mInvSpacing;
	}
	void reset();
	void add(const physx::PxBounds3& bounds, physx::PxU32 itemIndex);
	void add(const physx::PxVec3& pos, physx::PxU32 itemIndex);

	void query(const physx::PxBounds3& bounds, physx::Array<physx::PxU32>& itemIndices, physx::PxI32 maxIndices = -1);
	void queryUnique(const physx::PxBounds3& bounds, physx::Array<physx::PxU32>& itemIndices, physx::PxI32 maxIndices = -1);

	void query(const physx::PxVec3& pos, physx::Array<physx::PxU32>& itemIndices, physx::PxI32 maxIndices = -1);
	void queryUnique(const physx::PxVec3& pos, physx::Array<physx::PxU32>& itemIndices, physx::PxI32 maxIndices = -1);

	// applied functions, only work if inserted objects are points!
	physx::PxI32 getClosestPointNr(const physx::PxVec3* points, physx::PxU32 numPoints, physx::PxU32 pointStride, const physx::PxVec3& pos);

private:
	enum
	{
		HashIndexSize = 170111
	};

	void compressIndices(physx::Array<physx::PxU32>& itemIndices);
	physx::PxF32 mSpacing;
	physx::PxF32 mInvSpacing;
	physx::PxU32 mTime;

	inline physx::PxU32  hashFunction(physx::PxI32 xi, physx::PxI32 yi, physx::PxI32 zi)
	{
		physx::PxU32 h = (physx::PxU32)((xi * 92837111) ^(yi * 689287499) ^(zi * 283923481));
		return h % HashIndexSize;
	}

	inline void cellCoordOf(const physx::PxVec3& v, int& xi, int& yi, int& zi)
	{
		xi = (int)(v.x * mInvSpacing);
		if (v.x < 0.0f)
		{
			xi--;
		}
		yi = (int)(v.y * mInvSpacing);
		if (v.y < 0.0f)
		{
			yi--;
		}
		zi = (int)(v.z * mInvSpacing);
		if (v.z < 0.0f)
		{
			zi--;
		}
	}

	MeshHashRoot* mHashIndex;
	physx::Array<MeshHashEntry> mEntries;

	physx::Array<physx::PxU32> mTempIndices;
};

}
} // end namespace physx::apex

#endif