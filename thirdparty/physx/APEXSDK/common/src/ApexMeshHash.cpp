/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexMeshHash.h"

#include "foundation/PxBounds3.h"

#include "PsSort.h"

namespace physx
{
namespace apex
{

ApexMeshHash::ApexMeshHash() :
	mHashIndex(NULL)
{
	mHashIndex = (MeshHashRoot*)PX_ALLOC(sizeof(MeshHashRoot) * HashIndexSize, PX_DEBUG_EXP("ApexMeshHash"));
	mTime = 1;
	for (physx::PxU32 i = 0; i < HashIndexSize; i++)
	{
		mHashIndex[i].first = -1;
		mHashIndex[i].timeStamp = 0;
	}
	mSpacing = 0.25f;
	mInvSpacing = 1.0f / mSpacing;
}



ApexMeshHash::~ApexMeshHash()
{
	if (mHashIndex != NULL)
	{
		PX_FREE(mHashIndex);
		mHashIndex = NULL;
	}
}



void ApexMeshHash::setGridSpacing(physx::PxF32 spacing)
{
	mSpacing = spacing;
	mInvSpacing = 1.0f / spacing;
	reset();
}



void ApexMeshHash::reset()
{
	mTime++;
	mEntries.clear();
}



void ApexMeshHash::add(const physx::PxBounds3& bounds, physx::PxU32 itemIndex)
{
	physx::PxI32 x1, y1, z1;
	physx::PxI32 x2, y2, z2;
	cellCoordOf(bounds.minimum, x1, y1, z1);
	cellCoordOf(bounds.maximum, x2, y2, z2);
	MeshHashEntry entry;
	entry.itemIndex = itemIndex;

	for (physx::PxI32 x = x1; x <= x2; x++)
	{
		for (physx::PxI32 y = y1; y <= y2; y++)
		{
			for (physx::PxI32 z = z1; z <= z2; z++)
			{
				physx::PxU32 h = hashFunction(x, y, z);
				MeshHashRoot& r = mHashIndex[h];
				physx::PxU32 n = mEntries.size();
				if (r.timeStamp != mTime || r.first < 0)
				{
					entry.next = -1;
				}
				else
				{
					entry.next = r.first;
				}
				r.first = (physx::PxI32)n;
				r.timeStamp = mTime;
				mEntries.pushBack(entry);
			}
		}
	}
}



void ApexMeshHash::add(const physx::PxVec3& pos, physx::PxU32 itemIndex)
{
	int x, y, z;
	cellCoordOf(pos, x, y, z);
	MeshHashEntry entry;
	entry.itemIndex = itemIndex;

	physx::PxU32 h = hashFunction(x, y, z);
	MeshHashRoot& r = mHashIndex[h];
	physx::PxU32 n = mEntries.size();
	if (r.timeStamp != mTime || r.first < 0)
	{
		entry.next = -1;
	}
	else
	{
		entry.next = r.first;
	}
	r.first = (physx::PxI32)n;
	r.timeStamp = mTime;
	mEntries.pushBack(entry);
}



void ApexMeshHash::query(const physx::PxBounds3& bounds, physx::Array<physx::PxU32>& itemIndices, physx::PxI32 maxIndices)
{
	physx::PxI32 x1, y1, z1;
	physx::PxI32 x2, y2, z2;
	cellCoordOf(bounds.minimum, x1, y1, z1);
	cellCoordOf(bounds.maximum, x2, y2, z2);
	itemIndices.clear();

	for (physx::PxI32 x = x1; x <= x2; x++)
	{
		for (physx::PxI32 y = y1; y <= y2; y++)
		{
			for (physx::PxI32 z = z1; z <= z2; z++)
			{
				physx::PxU32 h = hashFunction(x, y, z);
				MeshHashRoot& r = mHashIndex[h];
				if (r.timeStamp != mTime)
				{
					continue;
				}
				physx::PxI32 i = r.first;
				while (i >= 0)
				{
					MeshHashEntry& entry = mEntries[(physx::PxU32)i];
					itemIndices.pushBack(entry.itemIndex);
					if (maxIndices >= 0 && itemIndices.size() >= (physx::PxU32)maxIndices)
					{
						return;
					}
					i = entry.next;
				}
			}
		}
	}
}



void ApexMeshHash::queryUnique(const physx::PxBounds3& bounds, physx::Array<physx::PxU32>& itemIndices, physx::PxI32 maxIndices)
{
	query(bounds, itemIndices, maxIndices);
	compressIndices(itemIndices);
}



void ApexMeshHash::query(const physx::PxVec3& pos, physx::Array<physx::PxU32>& itemIndices, physx::PxI32 maxIndices)
{
	physx::PxI32 x, y, z;
	cellCoordOf(pos, x, y, z);
	itemIndices.clear();

	physx::PxU32 h = hashFunction(x, y, z);
	MeshHashRoot& r = mHashIndex[h];
	if (r.timeStamp != mTime)
	{
		return;
	}
	physx::PxI32 i = r.first;
	while (i >= 0)
	{
		MeshHashEntry& entry = mEntries[(physx::PxU32)i];
		itemIndices.pushBack(entry.itemIndex);
		if (maxIndices >= 0 && itemIndices.size() >= (physx::PxU32)maxIndices)
		{
			return;
		}
		i = entry.next;
	}
}



void ApexMeshHash::queryUnique(const physx::PxVec3& pos, physx::Array<physx::PxU32>& itemIndices, physx::PxI32 maxIndices)
{
	query(pos, itemIndices, maxIndices);
	compressIndices(itemIndices);
}


class U32Less
{
public:
	bool operator()(PxU32 u1, PxU32 u2) const
	{
		return u1 < u2;
	}
};


void ApexMeshHash::compressIndices(physx::Array<physx::PxU32>& itemIndices)
{
	if (itemIndices.empty())
	{
		return;
	}

	shdfnd::sort(itemIndices.begin(), itemIndices.size(), U32Less());

	// mark duplicates
	physx::PxU32 i = 0;
	while (i < itemIndices.size())
	{
		physx::PxU32 j = i + 1;
		while (j < itemIndices.size() && itemIndices[i] == itemIndices[j])
		{
			itemIndices[j] = (physx::PxU32) - 1;
			j++;
		}
		i = j;
	}

	// remove duplicates
	i = 0;
	while (i < itemIndices.size())
	{
		if (itemIndices[i] == (physx::PxU32)-1)
		{
			itemIndices.replaceWithLast(i);
		}
		else
		{
			i++;
		}
	}
}

physx::PxI32 ApexMeshHash::getClosestPointNr(const physx::PxVec3* points, physx::PxU32 numPoints, physx::PxU32 pointStride, const physx::PxVec3& pos)
{
	PX_ASSERT(numPoints > 0);
	physx::PxBounds3 queryBounds;
	queryBounds.minimum = pos;
	queryBounds.maximum = pos;
	PX_ASSERT(!queryBounds.isEmpty());
	queryBounds.fattenFast(mSpacing);
	query(queryBounds, mTempIndices);

	// remove false positives due to hash collisions
	physx::PxU32 next = 0;
	for (physx::PxU32 i = 0; i < mTempIndices.size(); i++)
	{
		physx::PxU32 pointNr = mTempIndices[i];
		const physx::PxVec3* p = (physx::PxVec3*)((physx::PxU8*)points + (pointNr * pointStride));
		if (pointNr < numPoints && queryBounds.contains(*p))
		{
			mTempIndices[next++] = pointNr;
		}
	}
	mTempIndices.resize(next);
	bool fallBack = mTempIndices.size() == 0;
	physx::PxU32 numRes = fallBack ? numPoints : mTempIndices.size();

	float min2 = 0.0f;
	int minNr = -1;
	for (physx::PxU32 j = 0; j < numRes; j++)
	{
		physx::PxU32 k = fallBack ? j : mTempIndices[j];
		const physx::PxVec3* p = (physx::PxVec3*)((physx::PxU8*)points + (k * pointStride));
		float d2 = (pos - *p).magnitudeSquared();
		if (minNr < 0 || d2 < min2)
		{
			min2 = d2;
			minNr = (physx::PxI32)k;
		}
	}
	return minNr;
}

}
} // end namespace physx::apex
