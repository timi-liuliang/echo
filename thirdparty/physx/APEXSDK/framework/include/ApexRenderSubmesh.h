/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_RENDER_SUBMESH_H
#define APEX_RENDER_SUBMESH_H

#include "NiApexRenderMeshAsset.h"
#include "ApexVertexBuffer.h"
#include "SubmeshParameters.h"

namespace physx
{
namespace apex
{

class ApexRenderSubmesh : public NiApexRenderSubmesh, public UserAllocated
{
public:
	ApexRenderSubmesh() : mParams(NULL) {}
	~ApexRenderSubmesh() {}

	// from NxRenderSubmesh
	virtual physx::PxU32				getVertexCount(physx::PxU32 partIndex) const
	{
		return mParams->vertexPartition.buf[partIndex + 1] - mParams->vertexPartition.buf[partIndex];
	}

	virtual const NiApexVertexBuffer&	getVertexBuffer() const
	{
		return mVertexBuffer;
	}

	virtual physx::PxU32				getFirstVertexIndex(physx::PxU32 partIndex) const
	{
		return mParams->vertexPartition.buf[partIndex];
	}

	virtual physx::PxU32				getIndexCount(physx::PxU32 partIndex) const
	{
		return mParams->indexPartition.buf[partIndex + 1] - mParams->indexPartition.buf[partIndex];
	}

	virtual const physx::PxU32*			getIndexBuffer(physx::PxU32 partIndex) const
	{
		return mParams->indexBuffer.buf + mParams->indexPartition.buf[partIndex];
	}

	virtual const physx::PxU32*			getSmoothingGroups(physx::PxU32 partIndex) const
	{
		return mParams->smoothingGroups.buf != NULL ? (mParams->smoothingGroups.buf + mParams->indexPartition.buf[partIndex]/3) : NULL;
	}


	// from NiApexRenderSubmesh
	virtual NiApexVertexBuffer&			getVertexBufferWritable()
	{
		return mVertexBuffer;
	}

	virtual PxU32*						getIndexBufferWritable(PxU32 partIndex)
	{
		return mParams->indexBuffer.buf + mParams->indexPartition.buf[partIndex];
	}

	virtual void						applyPermutation(const Array<PxU32>& old2new, const Array<PxU32>& new2old);

	// own methods

	physx::PxU32						getTotalIndexCount() const
	{
		return (physx::PxU32)mParams->indexBuffer.arraySizes[0];
	}

	physx::PxU32*						getIndexBufferWritable(physx::PxU32 partIndex) const
	{
		return mParams->indexBuffer.buf + mParams->indexPartition.buf[partIndex];
	}

	bool								createFromParameters(SubmeshParameters* params);

	void								setParams(SubmeshParameters* submeshParams, VertexBufferParameters* vertexBufferParams);

	void								addStats(NxRenderMeshAssetStats& stats) const;

	void								buildVertexBuffer(const NxVertexFormat& format, PxU32 vertexCount);

	SubmeshParameters*  mParams;

private:
	ApexVertexBuffer    mVertexBuffer;

	// No assignment
	ApexRenderSubmesh&					operator = (const ApexRenderSubmesh&);
};

} // namespace apex
} // namespace physx

#endif // APEX_RENDER_SUBMESH_H
