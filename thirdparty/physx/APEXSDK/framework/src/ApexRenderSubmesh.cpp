/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexRenderSubmesh.h"

//#include "ApexStream.h"
//#include "ApexSharedSerialization.h"
#include "NiApexSDK.h"


namespace physx
{
namespace apex
{

PX_INLINE physx::PxU32 findIndexedNeighbors(physx::PxU32 indexedNeighbors[3], physx::PxU32 triangleIndex,
        const physx::PxU32* indexBuffer, const physx::PxU32* vertexTriangleRefs, const physx::PxU32* vertexToTriangleMap)
{
	physx::PxU32 indexedNeighborCount = 0;
	const physx::PxU32* triangleVertexIndices = indexBuffer + 3 * triangleIndex;
	for (physx::PxU32 v = 0; v < 3; ++v)
	{
		const physx::PxU32 vertexIndex = triangleVertexIndices[v];
		const physx::PxU32 prevVertexIndex = triangleVertexIndices[(3 >> v) ^ 1];
		// Find all other triangles which have this vertex
		const physx::PxU32 mapStart = vertexTriangleRefs[vertexIndex];
		const physx::PxU32 mapStop = vertexTriangleRefs[vertexIndex + 1];
		for (physx::PxU32 i = mapStart; i < mapStop; ++i)
		{
			const physx::PxU32 neighborTriangleIndex = vertexToTriangleMap[i];
			// See if the previous vertex on the triangle matches the next vertex on the neighbor.  (This will
			// automatically exclude the triangle itself, so no check to exclude a self-check is made.)
			const physx::PxU32* neighborTriangleVertexIndices = indexBuffer + 3 * neighborTriangleIndex;
			const physx::PxU8 indexMatch = (physx::PxU8)((physx::PxU8)(neighborTriangleVertexIndices[0] == vertexIndex) |
													     (physx::PxU8)(neighborTriangleVertexIndices[1] == vertexIndex) << 1 |
													     (physx::PxU8)(neighborTriangleVertexIndices[2] == vertexIndex) << 2);
			const physx::PxU32 nextNeighborVertexIndex = neighborTriangleVertexIndices[indexMatch & 3];
			if (nextNeighborVertexIndex == prevVertexIndex)
			{
				// Found a neighbor
				indexedNeighbors[indexedNeighborCount++] = neighborTriangleIndex;
			}
		}
	}

	return indexedNeighborCount;
}



void ApexRenderSubmesh::applyPermutation(const Array<PxU32>& old2new, const Array<PxU32>& new2old)
{
	if (mParams->vertexPartition.arraySizes[0] == 2)
	{
		mVertexBuffer.applyPermutation(new2old);
	}

	const PxU32 numIndices = (physx::PxU32)mParams->indexBuffer.arraySizes[0];
	for (PxU32 i = 0; i < numIndices; i++)
	{
		PX_ASSERT(mParams->indexBuffer.buf[i] < old2new.size());
		mParams->indexBuffer.buf[i] = old2new[mParams->indexBuffer.buf[i]];
	}
}



bool ApexRenderSubmesh::createFromParameters(SubmeshParameters* params)
{
	mParams = params;

	if (mParams->vertexBuffer == NULL)
	{
		NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();
		mParams->vertexBuffer = traits->createNxParameterized(VertexBufferParameters::staticClassName());
	}
	mVertexBuffer.setParams(static_cast<VertexBufferParameters*>(mParams->vertexBuffer));

	return true;
}



void ApexRenderSubmesh::setParams(SubmeshParameters* submeshParams, VertexBufferParameters* vertexBufferParams)
{

	if (vertexBufferParams == NULL && submeshParams != NULL)
	{
		vertexBufferParams = static_cast<VertexBufferParameters*>(submeshParams->vertexBuffer);
		PX_ASSERT(vertexBufferParams != NULL);
	}
	else if (submeshParams != NULL && submeshParams->vertexBuffer == NULL)
	{
		submeshParams->vertexBuffer = vertexBufferParams;
	}
	else if (mParams == NULL)
	{
		// Only emit this warning if mParams is empty yet (not on destruction of the object)
		APEX_INTERNAL_ERROR("Confliciting parameterized objects!");
	}
	mParams = submeshParams;

	mVertexBuffer.setParams(vertexBufferParams);
}



void ApexRenderSubmesh::addStats(NxRenderMeshAssetStats& stats) const
{
	stats.vertexCount += mVertexBuffer.getVertexCount();
	stats.indexCount += mParams->indexBuffer.arraySizes[0];

	const PxU32 submeshVertexBytes = mVertexBuffer.getAllocationSize();
	stats.vertexBufferBytes += submeshVertexBytes;
	stats.totalBytes += submeshVertexBytes;

	const PxU32 submeshIndexBytes = mParams->indexBuffer.arraySizes[0] * sizeof(physx::PxU32);
	stats.indexBufferBytes += submeshIndexBytes;
	stats.totalBytes += submeshIndexBytes;

	stats.totalBytes += mParams->smoothingGroups.arraySizes[0] * sizeof(physx::PxU32);
}



void ApexRenderSubmesh::buildVertexBuffer(const NxVertexFormat& format, PxU32 vertexCount)
{
	mVertexBuffer.build(format, vertexCount);
}


} // namespace apex
} // namespace physx
