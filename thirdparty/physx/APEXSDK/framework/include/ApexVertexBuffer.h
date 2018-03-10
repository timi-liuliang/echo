/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_VERTEX_BUFFER_H
#define APEX_VERTEX_BUFFER_H

#include "NiApexRenderMeshAsset.h"
#include "ApexVertexFormat.h"
#include "VertexBufferParameters.h"
#include <NxParameterized.h>
#include "ApexSharedUtils.h"
#include "ApexInteropableBuffer.h"

namespace physx
{
namespace apex
{

class ApexVertexBuffer : public NiApexVertexBuffer, public ApexInteropableBuffer, public NxParameterized::SerializationCallback
{
public:
	ApexVertexBuffer();
	~ApexVertexBuffer();

	// from NxVertexBuffer
	const NxVertexFormat&	getFormat() const
	{
		return mFormat;
	}
	physx::PxU32			getVertexCount() const
	{
		return mParams->vertexCount;
	}
	void*					getBuffer(physx::PxU32 bufferIndex);
	void*					getBufferAndFormatWritable(NxRenderDataFormat::Enum& format, physx::PxU32 bufferIndex)
	{
		return getBufferAndFormat(format, bufferIndex);
	}

	void*					getBufferAndFormat(NxRenderDataFormat::Enum& format, physx::PxU32 bufferIndex)
	{
		format = getFormat().getBufferFormat(bufferIndex);
		return getBuffer(bufferIndex);
	}
	bool					getBufferData(void* dstBuffer, physx::NxRenderDataFormat::Enum dstBufferFormat, physx::PxU32 dstBufferStride, physx::PxU32 bufferIndex,
	                                      physx::PxU32 startVertexIndex, physx::PxU32 elementCount) const;
	PX_INLINE const void*	getBuffer(physx::PxU32 bufferIndex) const
	{
		return (const void*)((ApexVertexBuffer*)this)->getBuffer(bufferIndex);
	}
	PX_INLINE const void*	getBufferAndFormat(NxRenderDataFormat::Enum& format, physx::PxU32 bufferIndex) const
	{
		return (const void*)((ApexVertexBuffer*)this)->getBufferAndFormat(format, bufferIndex);
	}

	// from NiApexVertexBuffer
	void					build(const NxVertexFormat& format, physx::PxU32 vertexCount);

	NxVertexFormat&			getFormatWritable()
	{
		return mFormat;
	}
	void					applyTransformation(const physx::PxMat34Legacy& transformation);
	void					applyScale(physx::PxF32 scale);
	bool					mergeBinormalsIntoTangents();

	void					copy(physx::PxU32 dstIndex, physx::PxU32 srcIndex, ApexVertexBuffer* srcBufferPtr = NULL);
	void					resize(physx::PxU32 vertexCount);

	// from NxParameterized::SerializationCallback

	void					preSerialize(void* userData_);

	void					setParams(VertexBufferParameters* param);
	VertexBufferParameters* getParams()
	{
		return mParams;
	}

	physx::PxU32			getAllocationSize() const;

	void					applyPermutation(const Array<PxU32>& permutation);

protected:
	VertexBufferParameters*			mParams;

	ApexVertexFormat				mFormat;	// Wrapper class for mParams->vertexFormat
};


} // namespace apex
} // namespace physx


#endif // APEX_VERTEX_BUFFER_H
