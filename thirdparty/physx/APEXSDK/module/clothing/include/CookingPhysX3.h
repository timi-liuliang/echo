/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef COOKING_PHYSX3_H
#define COOKING_PHYSX3_H

#include "CookingAbstract.h"

#include <PxVec3.h>
#include <PsArray.h>

// Tae - 301 -> 302: fiberless cooker change 
#define COOKED_DATA_VERSION 302

namespace physx
{
namespace apex
{
namespace clothing
{

class ClothingCookedPhysX3Param;

class CookingPhysX3 : public CookingAbstract
{
public:
	CookingPhysX3(bool withFibers) : mWithFibers(withFibers) {}

	virtual NxParameterized::Interface* execute();
	static PxU32 getCookingVersion()
	{
		return COOKED_DATA_VERSION;
	}

private:
	ClothingCookedPhysX3Param* trivialCooker(PxU32 subMeshIndex) const;
	ClothingCookedPhysX3Param* fiberCooker(PxU32 subMeshIndex) const;

	void computeUniqueEdges(PxU32 subMeshIndex);
	void refineUniqueEdges(PxU32 physicalMeshIndex);
	void computeVertexWeights(ClothingCookedPhysX3Param* cookedData, PxU32 subMeshIndex) const;
	void createVirtualParticles(ClothingCookedPhysX3Param* cookedData, PxU32 subMeshIndex);
	void createSelfcollisionIndices(ClothingCookedPhysX3Param* cookedData, PxU32 subMeshIndex) const;
	bool verifyValidity(const ClothingCookedPhysX3Param* cookedData, PxU32 subMeshIndex);
	void fillOutSetsDesc(ClothingCookedPhysX3Param* cookedData);
	void groupPhases(ClothingCookedPhysX3Param* cookedData, PxU32 subMeshIndex, PxU32 startIndex, PxU32 endIndex, Array<PxU32>& phaseEnds) const;

	void dumpObj(const char* filename, PxU32 subMeshIndex) const;
	void dumpApx(const char* filename, const NxParameterized::Interface* data) const;

	bool mWithFibers;

	static bool mTetraWarning;

	struct Edge
	{
		Edge();
		Edge(PxU32 v0, PxU32 v1, PxU32 v2);

		PxU32 vertex0, vertex1;
		PxU32 vertex2, vertex3;
		PxF32 maxAngle;
		bool isQuadDiagonal;
		bool isUsed;

		PX_FORCE_INLINE bool operator()(const Edge& e1, const Edge& e2) const
		{
			return e1 < e2;
		}

		PX_FORCE_INLINE bool operator!=(const Edge& other) const
		{
			return vertex0 != other.vertex0 || vertex1 != other.vertex1;
		}
		PX_FORCE_INLINE bool operator==(const Edge& other) const
		{
			return vertex0 == other.vertex0 && vertex1 == other.vertex1;
		}
		PX_FORCE_INLINE bool operator<(const Edge& other) const
		{
			if (vertex0 != other.vertex0)
			{
				return vertex0 < other.vertex0;
			}

			return vertex1 < other.vertex1;
		}

		PX_FORCE_INLINE PxU32 largestIndex() const
		{
			PxU32 largest = PxMax(vertex0, vertex1);
			largest = PxMax(largest, vertex2);
			if (vertex3 != 0xffffffff)
			{
				largest = PxMax(largest, vertex3);
			}
			return largest;
		}
	};

	struct SortHiddenEdges
	{
		SortHiddenEdges(shdfnd::Array<Edge>& uniqueEdges) : mUniqueEdges(uniqueEdges) {}

		bool operator()(PxU32 a, PxU32 b) const
		{
			return mUniqueEdges[a].maxAngle < mUniqueEdges[b].maxAngle;
		}

	private:
		SortHiddenEdges& operator=(const SortHiddenEdges&);

		shdfnd::Array<Edge>& mUniqueEdges;
	};

	shdfnd::Array<Edge> mUniqueEdges;
	PxU32 findUniqueEdge(PxU32 index1, PxU32 index2) const;

	struct VirtualParticle
	{
		VirtualParticle(PxU32 i0, PxU32 i1, PxU32 i2)
		{
			indices[0] = i0;
			indices[1] = i1;
			indices[2] = i2;
			tableIndex = 0;
		}

		void rotate(PxU32 count)
		{
			while (count--)
			{
				const PxU32 temp = indices[2];
				indices[2] = indices[1];
				indices[1] = indices[0];
				indices[0] = temp;
			}
		}

		PxU32 indices[3];
		PxU32 tableIndex;
	};

	struct EdgeAndLength
	{
		EdgeAndLength(PxU32 edgeNumber, PxF32 length) : mEdgeNumber(edgeNumber), mLength(length) {}
		PxU32 mEdgeNumber;
		PxF32 mLength;

		bool operator<(const EdgeAndLength& other) const
		{
			return mLength < other.mLength;
		}
	};
};

}
}
}


#endif // COOKING_PHYSX3_H
