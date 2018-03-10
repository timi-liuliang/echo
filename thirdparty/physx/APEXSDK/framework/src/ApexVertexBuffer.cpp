/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexVertexBuffer.h"

#include "NiApexSDK.h"

#include "VertexFormatParameters.h"

#include <NxParamArray.h>

#include "PxMemoryBuffer.h"
#include "NiCof44.h"

#include "BufferU8x1.h"
#include "BufferU8x2.h"
#include "BufferU8x3.h"
#include "BufferU8x4.h"
#include "BufferU16x1.h"
#include "BufferU16x2.h"
#include "BufferU16x3.h"
#include "BufferU16x4.h"
#include "BufferU32x1.h"
#include "BufferU32x2.h"
#include "BufferU32x3.h"
#include "BufferU32x4.h"
#include "BufferF32x1.h"
#include "BufferF32x2.h"
#include "BufferF32x3.h"
#include "BufferF32x4.h"

#include "ApexPermute.h"

namespace physx
{
namespace apex
{

#ifdef _DEBUG
#define VERIFY_PARAM(_A) PX_ASSERT(_A == NxParameterized::ERROR_NONE)
#else
#define VERIFY_PARAM(_A) _A
#endif

// Transform Vec3 by PxMat34Legacy
PX_INLINE void transform_FLOAT3_by_PxMat34Legacy(FLOAT3_TYPE& dst, const FLOAT3_TYPE& src, const physx::PxMat34Legacy& m)
{
	(physx::PxVec3&)dst = m * (const physx::PxVec3&)src;
}

// Transform Vec3 by PxMat33Legacy
PX_INLINE void transform_FLOAT3_by_PxMat33Legacy(FLOAT3_TYPE& dst, const FLOAT3_TYPE& src, const physx::PxMat33Legacy& m)
{
	(physx::PxVec3&)dst = m * (const physx::PxVec3&)src;
}

// Transform Vec4 (tangent) by PxMat33Legacy, ignoring tangent.w
PX_INLINE void transform_FLOAT4_by_PxMat33Legacy(FLOAT4_TYPE& dst, const FLOAT4_TYPE& src, const physx::PxMat33Legacy& m)
{
	const PxVec4 source = (const PxVec4&)src;
	(physx::PxVec4&)dst = PxVec4(m * source.getXYZ(), PxSign(m.determinant()) * source.w);
}

// Transform Quat by PxMat33Legacy
PX_INLINE void transform_FLOAT4_QUAT_by_PxMat33Legacy(FLOAT4_QUAT_TYPE& dst, const FLOAT4_QUAT_TYPE& src, const physx::PxMat33Legacy& m)
{
	*((physx::PxVec3*)&dst) = m * (*(const physx::PxVec3*)&src.x);
}

// Multiply Vec3 by scalar
PX_INLINE void transform_FLOAT3_by_PxF32(FLOAT3_TYPE& dst, const FLOAT3_TYPE& src, const physx::PxF32& s)
{
	(physx::PxVec3&)dst = s * (const physx::PxVec3&)src;
}

// Transform signed normalized byte 3-vector by PxMat34Legacy
PX_INLINE void transform_BYTE_SNORM3_by_PxMat34Legacy(BYTE_SNORM3_TYPE& dst, const BYTE_SNORM3_TYPE& src, const physx::PxMat34Legacy& m)
{
	physx::PxVec3 v;
	convert_FLOAT3_from_BYTE_SNORM3((FLOAT3_TYPE&)v, src);
	transform_FLOAT3_by_PxMat34Legacy((FLOAT3_TYPE&)v, (const FLOAT3_TYPE&)v, m);
	convert_BYTE_SNORM3_from_FLOAT3(dst, (const FLOAT3_TYPE&)v);
}

// Transform signed normalized byte 3-vector by PxMat33Legacy
PX_INLINE void transform_BYTE_SNORM3_by_PxMat33Legacy(BYTE_SNORM3_TYPE& dst, const BYTE_SNORM3_TYPE& src, const physx::PxMat33Legacy& m)
{
	physx::PxVec3 v;
	convert_FLOAT3_from_BYTE_SNORM3((FLOAT3_TYPE&)v, src);
	transform_FLOAT3_by_PxMat33Legacy((FLOAT3_TYPE&)v, (const FLOAT3_TYPE&)v, m);
	convert_BYTE_SNORM3_from_FLOAT3(dst, (const FLOAT3_TYPE&)v);
}

// Transform signed normalized byte 4-vector by PxMat33Legacy
PX_INLINE void transform_BYTE_SNORM4_by_PxMat33Legacy(BYTE_SNORM4_TYPE& dst, const BYTE_SNORM4_TYPE& src, const physx::PxMat33Legacy& m)
{
	physx::PxVec4 v;
	convert_FLOAT4_from_BYTE_SNORM4((FLOAT4_TYPE&)v, src);
	transform_FLOAT4_by_PxMat33Legacy((FLOAT4_TYPE&)v, (const FLOAT4_TYPE&)v, m);
	convert_BYTE_SNORM4_from_FLOAT4(dst, (const FLOAT4_TYPE&)v);
}

// Multiply signed normalized byte 3-vector by scalar
PX_INLINE void transform_BYTE_SNORM3_by_PxF32(BYTE_SNORM3_TYPE& dst, const BYTE_SNORM3_TYPE& src, const physx::PxF32& s)
{
	physx::PxVec3 v;
	convert_FLOAT3_from_BYTE_SNORM3((FLOAT3_TYPE&)v, src);
	transform_FLOAT3_by_PxF32((FLOAT3_TYPE&)v, (const FLOAT3_TYPE&)v, s);
	convert_BYTE_SNORM3_from_FLOAT3(dst, (const FLOAT3_TYPE&)v);
}

// Transform signed normalized byte quat by PxMat33Legacy
PX_INLINE void transform_BYTE_SNORM4_QUATXYZW_by_PxMat33Legacy(BYTE_SNORM4_QUATXYZW_TYPE& dst, const BYTE_SNORM4_QUATXYZW_TYPE& src, const physx::PxMat33Legacy& m)
{
	transform_BYTE_SNORM3_by_PxMat33Legacy(*(BYTE_SNORM3_TYPE*)&dst, *(const BYTE_SNORM3_TYPE*)&src, m);
}

// Transform signed normalized short 3-vector by PxMat34Legacy
PX_INLINE void transform_SHORT_SNORM3_by_PxMat34Legacy(SHORT_SNORM3_TYPE& dst, const SHORT_SNORM3_TYPE& src, const physx::PxMat34Legacy& m)
{
	physx::PxVec3 v;
	convert_FLOAT3_from_SHORT_SNORM3((FLOAT3_TYPE&)v, src);
	transform_FLOAT3_by_PxMat34Legacy((FLOAT3_TYPE&)v, (const FLOAT3_TYPE&)v, m);
	convert_SHORT_SNORM3_from_FLOAT3(dst, (const FLOAT3_TYPE&)v);
}

// Transform signed normalized short 3-vector by PxMat33Legacy
PX_INLINE void transform_SHORT_SNORM3_by_PxMat33Legacy(SHORT_SNORM3_TYPE& dst, const SHORT_SNORM3_TYPE& src, const physx::PxMat33Legacy& m)
{
	physx::PxVec3 v;
	convert_FLOAT3_from_SHORT_SNORM3((FLOAT3_TYPE&)v, src);
	transform_FLOAT3_by_PxMat33Legacy((FLOAT3_TYPE&)v, (const FLOAT3_TYPE&)v, m);
	convert_SHORT_SNORM3_from_FLOAT3(dst, (const FLOAT3_TYPE&)v);
}

// Transform signed normalized short 4-vector by PxMat33Legacy
PX_INLINE void transform_SHORT_SNORM4_by_PxMat33Legacy(SHORT_SNORM4_TYPE& dst, const SHORT_SNORM4_TYPE& src, const physx::PxMat33Legacy& m)
{
	physx::PxVec4 v;
	convert_FLOAT4_from_SHORT_SNORM4((FLOAT4_TYPE&)v, src);
	transform_FLOAT4_by_PxMat33Legacy((FLOAT4_TYPE&)v, (const FLOAT4_TYPE&)v, m);
	convert_SHORT_SNORM4_from_FLOAT4(dst, (const FLOAT4_TYPE&)v);
}

// Multiply signed normalized short 3-vector by scalar
PX_INLINE void transform_SHORT_SNORM3_by_PxF32(SHORT_SNORM3_TYPE& dst, const SHORT_SNORM3_TYPE& src, const physx::PxF32& s)
{
	physx::PxVec3 v;
	convert_FLOAT3_from_SHORT_SNORM3((FLOAT3_TYPE&)v, src);
	transform_FLOAT3_by_PxF32((FLOAT3_TYPE&)v, (const FLOAT3_TYPE&)v, s);
	convert_SHORT_SNORM3_from_FLOAT3(dst, (const FLOAT3_TYPE&)v);
}

// Transform signed normalized short quat by PxMat33Legacy
PX_INLINE void transform_SHORT_SNORM4_QUATXYZW_by_PxMat33Legacy(SHORT_SNORM4_QUATXYZW_TYPE& dst, const SHORT_SNORM4_QUATXYZW_TYPE& src, const physx::PxMat33Legacy& m)
{
	transform_SHORT_SNORM3_by_PxMat33Legacy(*(SHORT_SNORM3_TYPE*)&dst, *(const SHORT_SNORM3_TYPE*)&src, m);
}

#define SAME(x) x

#define HANDLE_TRANSFORM( _DataType, _OpType ) \
	case NxRenderDataFormat::SAME(_DataType): \
	while( numVertices-- ) \
	{ \
		transform_##_DataType##_by_##_OpType( *(_DataType##_TYPE*)dst, *(const _DataType##_TYPE*)src, op ); \
		((PxU8*&)dst) += sizeof( _DataType##_TYPE ); \
		((const PxU8*&)src) += sizeof( _DataType##_TYPE ); \
	} \
	return;

void transformRenderBuffer(void* dst, const void* src, NxRenderDataFormat::Enum format, PxU32 numVertices, const PxMat34Legacy& op)
{
	switch (format)
	{
		// Put transform handlers here
		HANDLE_TRANSFORM(FLOAT3, PxMat34Legacy)
		HANDLE_TRANSFORM(BYTE_SNORM3, PxMat34Legacy)
		HANDLE_TRANSFORM(SHORT_SNORM3, PxMat34Legacy)
	default:
		break;
	}

	PX_ALWAYS_ASSERT();	// Unhandled format
}

void transformRenderBuffer(void* dst, const void* src, NxRenderDataFormat::Enum format, PxU32 numVertices, const PxMat33Legacy& op)
{
	switch (format)
	{
		// Put transform handlers here
		HANDLE_TRANSFORM(FLOAT3, PxMat33Legacy)
		HANDLE_TRANSFORM(FLOAT4, PxMat33Legacy)
		HANDLE_TRANSFORM(FLOAT4_QUAT, PxMat33Legacy)
		HANDLE_TRANSFORM(BYTE_SNORM3, PxMat33Legacy)
		HANDLE_TRANSFORM(BYTE_SNORM4, PxMat33Legacy)
		HANDLE_TRANSFORM(BYTE_SNORM4_QUATXYZW, PxMat33Legacy)
		HANDLE_TRANSFORM(SHORT_SNORM3, PxMat33Legacy)
		HANDLE_TRANSFORM(SHORT_SNORM4, PxMat33Legacy)
		HANDLE_TRANSFORM(SHORT_SNORM4_QUATXYZW, PxMat33Legacy)
	default:
		break;
	}

	PX_ALWAYS_ASSERT();	// Unhandled format
}

void transformRenderBuffer(void* dst, const void* src, NxRenderDataFormat::Enum format, PxU32 numVertices, const PxF32& op)
{
	switch (format)
	{
		// Put transform handlers here
		HANDLE_TRANSFORM(FLOAT3, PxF32)
		HANDLE_TRANSFORM(BYTE_SNORM3, PxF32)
	default:
		break;
	}

	PX_ALWAYS_ASSERT();	// Unhandled format
}


ApexVertexBuffer::ApexVertexBuffer() : mParams(NULL), mFormat(NULL)
{
}

ApexVertexBuffer::~ApexVertexBuffer()
{
	PX_ASSERT(mParams == NULL);
}

void ApexVertexBuffer::build(const NxVertexFormat& format, physx::PxU32 vertexCount)
{
	const ApexVertexFormat* apexVertexFormat = DYNAMIC_CAST(const ApexVertexFormat*)(&format);
	if (apexVertexFormat)
	{
		mFormat.copy(*apexVertexFormat);
	}

	NxParameterized::Handle handle(*mParams);
	VERIFY_PARAM(mParams->getParameterHandle("buffers", handle));
	VERIFY_PARAM(mParams->resizeArray(handle, mFormat.mParams->bufferFormats.arraySizes[0]));

	resize(vertexCount);
}

void ApexVertexBuffer::applyTransformation(const physx::PxMat34Legacy& transformation)
{
	NxRenderDataFormat::Enum format;
	void* buf;
	PxU32 index;

	// Positions
	index = (physx::PxU32)getFormat().getBufferIndexFromID(getFormat().getSemanticID(NxRenderVertexSemantic::POSITION));
	buf = getBuffer(index);
	if (buf)
	{
		format = getFormat().getBufferFormat(index);
		transformRenderBuffer(buf, buf, format, getVertexCount(), transformation);
	}

	// Normals
	index = (physx::PxU32)getFormat().getBufferIndexFromID(getFormat().getSemanticID(NxRenderVertexSemantic::NORMAL));
	buf = getBuffer(index);
	if (buf)
	{
		// PH: the Cofactor matrix now also handles negative determinants, so it does the same as multiplying with the inverse transpose of transformation.M.
		const NiCof44 cof(transformation);
		format = getFormat().getBufferFormat(index);
		transformRenderBuffer(buf, buf, format, getVertexCount(), cof.getBlock33());
	}

	// Tangents
	index = (physx::PxU32)getFormat().getBufferIndexFromID(getFormat().getSemanticID(NxRenderVertexSemantic::TANGENT));
	buf = getBuffer(index);
	if (buf)
	{
		format = getFormat().getBufferFormat(index);
		transformRenderBuffer(buf, buf, format, getVertexCount(), transformation.M);
	}

	// Binormals
	index = (physx::PxU32)getFormat().getBufferIndexFromID(getFormat().getSemanticID(NxRenderVertexSemantic::BINORMAL));
	buf = getBuffer(index);
	if (buf)
	{
		format = getFormat().getBufferFormat(index);
		transformRenderBuffer(buf, buf, format, getVertexCount(), transformation.M);
	}
}



void ApexVertexBuffer::applyScale(physx::PxF32 scale)
{
	PxU32 index = (physx::PxU32)getFormat().getBufferIndexFromID(getFormat().getSemanticID(NxRenderVertexSemantic::POSITION));
	void* buf = getBuffer(index);
	NxRenderDataFormat::Enum format = getFormat().getBufferFormat(index);
	transformRenderBuffer(buf, buf, format, getVertexCount(), scale);
}



bool ApexVertexBuffer::mergeBinormalsIntoTangents()
{
	const PxU32 numBuffers = mFormat.getBufferCount();

	PxI32 normalBufferIndex = -1;
	PxI32 tangentBufferIndex = -1;
	PxI32 binormalBufferIndex = -1;
	for (PxU32 i = 0; i < numBuffers; i++)
	{
		const NxRenderVertexSemantic::Enum semantic  = mFormat.getBufferSemantic(i);
		const NxRenderDataFormat::Enum format = mFormat.getBufferFormat(i);
		if (semantic == NxRenderVertexSemantic::NORMAL && format == NxRenderDataFormat::FLOAT3)
		{
			normalBufferIndex = (physx::PxI32)i;
		}
		else if (semantic == NxRenderVertexSemantic::TANGENT && format == NxRenderDataFormat::FLOAT3)
		{
			tangentBufferIndex = (physx::PxI32)i;
		}
		else if (semantic == NxRenderVertexSemantic::BINORMAL && format == NxRenderDataFormat::FLOAT3)
		{
			binormalBufferIndex = (physx::PxI32)i;
		}
	}

	if (normalBufferIndex != -1 && tangentBufferIndex != -1 && binormalBufferIndex != -1)
	{
		// PH: This gets dirty. modifying the parameterized object directly
		BufferF32x3* normalsBuffer = static_cast<BufferF32x3*>(mParams->buffers.buf[normalBufferIndex]);
		BufferF32x3* oldTangentsBuffer = static_cast<BufferF32x3*>(mParams->buffers.buf[tangentBufferIndex]);
		BufferF32x3* oldBinormalsBuffer = static_cast<BufferF32x3*>(mParams->buffers.buf[binormalBufferIndex]);
		BufferF32x4* newTangentsBuffer = static_cast<BufferF32x4*>(NiGetApexSDK()->getParameterizedTraits()->createNxParameterized("BufferF32x4"));

		if (normalsBuffer != NULL && oldTangentsBuffer != NULL && oldBinormalsBuffer != NULL && newTangentsBuffer != NULL)
		{
			const PxU32 numElements = (physx::PxU32)oldTangentsBuffer->data.arraySizes[0];

			PX_ASSERT(oldTangentsBuffer->data.arraySizes[0] == oldBinormalsBuffer->data.arraySizes[0]);
			{
				// resize the array
				NxParameterized::Handle handle(*newTangentsBuffer, "data");
				PX_ASSERT(handle.isValid());
				handle.resizeArray((physx::PxI32)numElements);
			}
			PX_ASSERT(oldTangentsBuffer->data.arraySizes[0] == newTangentsBuffer->data.arraySizes[0]);

			const PxVec3* normals = normalsBuffer->data.buf;
			const PxVec3* oldTangents = oldTangentsBuffer->data.buf;
			const PxVec3* oldBinormals = oldBinormalsBuffer->data.buf;
			PxVec4* newTangents = (PxVec4*)newTangentsBuffer->data.buf;

			for (PxU32 i = 0; i < numElements; i++)
			{
				const PxF32 binormal = PxSign(normals[i].cross(oldTangents[i]).dot(oldBinormals[i]));
				newTangents[i] = PxVec4(oldTangents[i], binormal);
			}

			// Ok, real dirty now
			mParams->buffers.buf[(physx::PxU32)tangentBufferIndex] = newTangentsBuffer;
			for (PxU32 i = (physx::PxU32)binormalBufferIndex + 1; i < numBuffers; i++)
			{
				mParams->buffers.buf[i - 1] = mParams->buffers.buf[i];
			}
			mParams->buffers.buf[numBuffers - 1] = NULL;
			{
				NxParameterized::Handle handle(*mParams, "buffers");
				PX_ASSERT(handle.isValid());
				handle.resizeArray((physx::PxI32)numBuffers - 1);
			}
			oldTangentsBuffer->destroy();
			oldBinormalsBuffer->destroy();

			// and make same change to the format too
			VertexFormatParameters* format = static_cast<VertexFormatParameters*>(mParams->vertexFormat);
			PX_ASSERT(format->bufferFormats.buf[tangentBufferIndex].semantic == NxRenderVertexSemantic::TANGENT);
			PX_ASSERT(format->bufferFormats.buf[tangentBufferIndex].format == NxRenderDataFormat::FLOAT3);
			format->bufferFormats.buf[tangentBufferIndex].format = NxRenderDataFormat::FLOAT4;

			VertexFormatParametersNS::BufferFormat_Type binormalBuffer = format->bufferFormats.buf[binormalBufferIndex];
			for (PxU32 i = (physx::PxU32)binormalBufferIndex + 1; i < numBuffers; i++)
			{
				format->bufferFormats.buf[i - 1] = format->bufferFormats.buf[i];
			}

			// swap it to the last such that it gets released properly
			format->bufferFormats.buf[numBuffers - 1] = binormalBuffer;
			{
				NxParameterized::Handle handle(*format, "bufferFormats");
				PX_ASSERT(handle.isValid());
				handle.resizeArray((physx::PxI32)numBuffers - 1);
			}

			return true;
		}
	}
	return false;
}



void ApexVertexBuffer::copy(physx::PxU32 dstIndex, physx::PxU32 srcIndex, ApexVertexBuffer* srcBufferPtr)
{
	ApexVertexBuffer& srcVB = srcBufferPtr != NULL ? *srcBufferPtr : *this;
	ApexVertexFormat& srcVF = srcVB.mFormat;

	if (mParams->buffers.arraySizes[0] != srcVB.mParams->buffers.arraySizes[0])
	{
		PX_ALWAYS_ASSERT();
		return;
	}

	for (physx::PxU32 i = 0; i < (physx::PxU32)mParams->buffers.arraySizes[0]; i++)
	{
		NxRenderDataFormat::Enum dstFormat = mFormat.getBufferFormat(i);
		NxVertexFormat::BufferID id = mFormat.getBufferID(i);
		const physx::PxI32 srcBufferIndex = srcVF.getBufferIndexFromID(id);
		if (srcBufferIndex >= 0)
		{
			NxRenderDataFormat::Enum srcFormat = srcVF.getBufferFormat((physx::PxU32)srcBufferIndex);
			NxParameterized::Interface* dstInterface = mParams->buffers.buf[i];
			NxParameterized::Interface* srcInterface = srcVB.mParams->buffers.buf[(physx::PxU32)srcBufferIndex];
			// BRG: Using PH's reasoning: Technically all those CustomBuffer* classes should have the same struct, so I just use the first one
			BufferU8x1& srcBuffer = *static_cast<BufferU8x1*>(srcInterface);
			BufferU8x1& dstBuffer = *static_cast<BufferU8x1*>(dstInterface);
			PX_ASSERT(dstIndex < (physx::PxU32)dstBuffer.data.arraySizes[0]);
			PX_ASSERT(srcIndex < (physx::PxU32)srcBuffer.data.arraySizes[0]);
			copyRenderVertexData(dstBuffer.data.buf, dstFormat, dstIndex, srcBuffer.data.buf, srcFormat, srcIndex);
		}
	}
}

void ApexVertexBuffer::resize(physx::PxU32 vertexCount)
{
	mParams->vertexCount = vertexCount;

	NxParameterized::Handle handle(*mParams);

	VERIFY_PARAM(mParams->getParameterHandle("buffers", handle));
	physx::PxI32 buffersSize = 0;
	VERIFY_PARAM(mParams->getArraySize(handle, buffersSize));

	for (physx::PxI32 i = 0; i < buffersSize; i++)
	{
		NxRenderDataFormat::Enum outFormat = mFormat.getBufferFormat((physx::PxU32)i);

		NxParameterized::Handle elementHandle(*mParams);
		VERIFY_PARAM(handle.getChildHandle(i, elementHandle));

		NxParameterized::Interface* currentReference = NULL;
		VERIFY_PARAM(mParams->getParamRef(elementHandle, currentReference));

		// BUFFER_FORMAT_ADD This is just a bookmark for places where to add buffer formats
		if (currentReference == NULL && vertexCount > 0)
		{
			const char* className = NULL;

			switch (outFormat)
			{
			case NxRenderDataFormat::UBYTE1:
			case NxRenderDataFormat::BYTE_UNORM1:
			case NxRenderDataFormat::BYTE_SNORM1:
				className = BufferU8x1::staticClassName();
				break;
			case NxRenderDataFormat::UBYTE2:
			case NxRenderDataFormat::BYTE_UNORM2:
			case NxRenderDataFormat::BYTE_SNORM2:
				className = BufferU8x2::staticClassName();
				break;
			case NxRenderDataFormat::UBYTE3:
			case NxRenderDataFormat::BYTE_UNORM3:
			case NxRenderDataFormat::BYTE_SNORM3:
				className = BufferU8x3::staticClassName();
				break;
			case NxRenderDataFormat::UBYTE4:
			case NxRenderDataFormat::BYTE_UNORM4:
			case NxRenderDataFormat::BYTE_SNORM4:
			case NxRenderDataFormat::R8G8B8A8:
			case NxRenderDataFormat::B8G8R8A8:
				className = BufferU8x4::staticClassName();
				break;
			case NxRenderDataFormat::SHORT1:
			case NxRenderDataFormat::USHORT1:
			case NxRenderDataFormat::SHORT_UNORM1:
			case NxRenderDataFormat::SHORT_SNORM1:
			case NxRenderDataFormat::HALF1:
				className = BufferU16x1::staticClassName();
				break;
			case NxRenderDataFormat::SHORT2:
			case NxRenderDataFormat::USHORT2:
			case NxRenderDataFormat::SHORT_UNORM2:
			case NxRenderDataFormat::SHORT_SNORM2:
			case NxRenderDataFormat::HALF2:
				className = BufferU16x2::staticClassName();
				break;
			case NxRenderDataFormat::SHORT3:
			case NxRenderDataFormat::USHORT3:
			case NxRenderDataFormat::SHORT_UNORM3:
			case NxRenderDataFormat::SHORT_SNORM3:
			case NxRenderDataFormat::HALF3:
				className = BufferU16x3::staticClassName();
				break;
			case NxRenderDataFormat::SHORT4:
			case NxRenderDataFormat::USHORT4:
			case NxRenderDataFormat::SHORT_UNORM4:
			case NxRenderDataFormat::SHORT_SNORM4:
			case NxRenderDataFormat::HALF4:
				className = BufferU16x4::staticClassName();
				break;
			case NxRenderDataFormat::UINT1:
				className = BufferU32x1::staticClassName();
				break;
			case NxRenderDataFormat::UINT2:
				className = BufferU32x2::staticClassName();
				break;
			case NxRenderDataFormat::UINT3:
				className = BufferU32x3::staticClassName();
				break;
			case NxRenderDataFormat::UINT4:
				className = BufferU32x4::staticClassName();
				break;
			case NxRenderDataFormat::FLOAT1:
				className = BufferF32x1::staticClassName();
				break;
			case NxRenderDataFormat::FLOAT2:
				className = BufferF32x2::staticClassName();
				break;
			case NxRenderDataFormat::FLOAT3:
				className = BufferF32x3::staticClassName();
				break;
			case NxRenderDataFormat::FLOAT4:
			case NxRenderDataFormat::R32G32B32A32_FLOAT:
			case NxRenderDataFormat::B32G32R32A32_FLOAT:
				className = BufferF32x4::staticClassName();
				break;
			default:
				PX_ALWAYS_ASSERT();
				break;
			}

			if (className != NULL)
			{
				currentReference = NiGetApexSDK()->getParameterizedTraits()->createNxParameterized(className);
			}

			if (currentReference != NULL)
			{
				NxParameterized::Handle arrayHandle(*currentReference);
				VERIFY_PARAM(currentReference->getParameterHandle("data", arrayHandle));
				PX_ASSERT(arrayHandle.isValid());
				VERIFY_PARAM(arrayHandle.resizeArray((physx::PxI32)vertexCount));

				mParams->setParamRef(elementHandle, currentReference);
			}
		}
		else if (vertexCount > 0)
		{
			NxParameterized::Interface* oldReference = currentReference;
			PX_ASSERT(oldReference != NULL);
			currentReference = NiGetApexSDK()->getParameterizedTraits()->createNxParameterized(oldReference->className());
			if (currentReference != NULL)
			{
				VERIFY_PARAM(currentReference->copy(*oldReference));

				NxParameterized::Handle arrayHandle(*currentReference);
				VERIFY_PARAM(currentReference->getParameterHandle("data", arrayHandle));
				VERIFY_PARAM(arrayHandle.resizeArray((physx::PxI32)vertexCount));
			}
			VERIFY_PARAM(mParams->setParamRef(elementHandle, currentReference));
			oldReference->destroy();
		}
		else if (vertexCount == 0)
		{
			VERIFY_PARAM(mParams->setParamRef(elementHandle, NULL));

			if (currentReference != NULL)
			{
				currentReference->destroy();
			}
		}
	}
}



void ApexVertexBuffer::preSerialize(void*)
{
	PX_ASSERT((physx::PxI32)mFormat.getBufferCount() == mParams->buffers.arraySizes[0]);
	NxParamArray<NxParameterized::Interface*> buffers(mParams, "buffers", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->buffers));
	for (physx::PxU32 i = 0; i < mFormat.getBufferCount(); i++)
	{
		if (!mFormat.getBufferSerialize(i))
		{
			// [i] no longer needs to be destroyed because the resize will handle it
			buffers.replaceWithLast(i);
			mFormat.bufferReplaceWithLast(i);
			i--;
		}
	}

	PX_ASSERT((physx::PxI32)mFormat.getBufferCount() == mParams->buffers.arraySizes[0]);
}

bool ApexVertexBuffer::getBufferData(void* dstBuffer, physx::NxRenderDataFormat::Enum dstBufferFormat, physx::PxU32 dstBufferStride, physx::PxU32 bufferIndex,
                                     physx::PxU32 startIndex, physx::PxU32 elementCount) const
{
	const void* data = getBuffer(bufferIndex);
	if (data == NULL)
	{
		return false;
	}
	physx::NxRenderDataFormat::Enum srcFormat = getFormat().getBufferFormat(bufferIndex);
	return copyRenderVertexBuffer(dstBuffer, dstBufferFormat, dstBufferStride, 0, data, srcFormat, NxRenderDataFormat::getFormatDataSize(srcFormat), startIndex, elementCount);
}

void* ApexVertexBuffer::getBuffer(physx::PxU32 bufferIndex)
{
	if (bufferIndex < (physx::PxU32)mParams->buffers.arraySizes[0])
	{
		NxParameterized::Interface* buffer = mParams->buffers.buf[bufferIndex];
		if (buffer != NULL)
		{
			BufferU8x1* particularBuffer = DYNAMIC_CAST(BufferU8x1*)(buffer);
			return particularBuffer->data.buf;
		}
	}

	return NULL;
}

physx::PxU32 ApexVertexBuffer::getAllocationSize() const
{
	physx::PxU32 size = sizeof(ApexVertexBuffer);

	for (physx::PxU32 index = 0; (physx::PxI32)index < mParams->buffers.arraySizes[0]; ++index)
	{
		PX_ASSERT(index < getFormat().getBufferCount());
		if (index >= getFormat().getBufferCount())
		{
			break;
		}
		const physx::PxU32 dataSize = NxRenderDataFormat::getFormatDataSize(getFormat().getBufferFormat(index));
		NxParameterized::Interface* buffer = mParams->buffers.buf[index];
		if (buffer != NULL)
		{
			BufferU8x1* particularBuffer = DYNAMIC_CAST(BufferU8x1*)(buffer);
			size += particularBuffer->data.arraySizes[0] * dataSize;
		}
	}

	return size;
}

void ApexVertexBuffer::setParams(VertexBufferParameters* param)
{
	if (mParams != param)
	{
		if (mParams != NULL)
		{
			mParams->setSerializationCallback(NULL);
		}

		mParams = param;

		if (mParams != NULL)
		{
			NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();
			if (mParams->vertexFormat != NULL)
			{
				if (mFormat.mParams && mFormat.mParams != (VertexFormatParameters*)mParams->vertexFormat)
				{
					mFormat.mParams->destroy();
				}
			}
			else
			{
				mParams->vertexFormat = DYNAMIC_CAST(VertexFormatParameters*)(traits->createNxParameterized(VertexFormatParameters::staticClassName()));
			}
		}

		mFormat.mParams = mParams != NULL ? static_cast<VertexFormatParameters*>(mParams->vertexFormat) : NULL;
		mFormat.mOwnsParams = false;

		if (mParams != NULL)
		{
			mParams->setSerializationCallback(this);
		}
	}
}



void ApexVertexBuffer::applyPermutation(const Array<PxU32>& permutation)
{
	const PxU32 numVertices = mParams->vertexCount;
	PX_ASSERT(numVertices == permutation.size());
	for (PxU32 i = 0; i < (physx::PxU32)mParams->buffers.arraySizes[0]; i++)
	{
		NxParameterized::Interface* bufferInterface = mParams->buffers.buf[i];
		NxRenderDataFormat::Enum format = getFormat().getBufferFormat(i);
		switch(format)
		{
			// all 1 byte
		case NxRenderDataFormat::UBYTE1:
		case NxRenderDataFormat::BYTE_UNORM1:
		case NxRenderDataFormat::BYTE_SNORM1:
			{
				BufferU8x1* byte1 = static_cast<BufferU8x1*>(bufferInterface);
				PX_ASSERT(numVertices == (physx::PxU32)byte1->data.arraySizes[0]);
				ApexPermute(byte1->data.buf, permutation.begin(), numVertices);
			}
			break;

			// all 2 byte
		case NxRenderDataFormat::UBYTE2:
		case NxRenderDataFormat::USHORT1:
		case NxRenderDataFormat::SHORT1:
		case NxRenderDataFormat::BYTE_UNORM2:
		case NxRenderDataFormat::SHORT_UNORM1:
		case NxRenderDataFormat::BYTE_SNORM2:
		case NxRenderDataFormat::SHORT_SNORM1:
		case NxRenderDataFormat::HALF1:
			{
				BufferU16x1* short1 = static_cast<BufferU16x1*>(bufferInterface);
				PX_ASSERT(numVertices == (physx::PxU32)short1->data.arraySizes[0]);
				ApexPermute(short1->data.buf, permutation.begin(), numVertices);
			}
			break;

			// all 3 byte
		case NxRenderDataFormat::UBYTE3:
		case NxRenderDataFormat::BYTE_UNORM3:
		case NxRenderDataFormat::BYTE_SNORM3:
			{
				BufferU8x3* byte3 = static_cast<BufferU8x3*>(bufferInterface);
				PX_ASSERT(numVertices == (physx::PxU32)byte3->data.arraySizes[0]);
				ApexPermute(byte3->data.buf, permutation.begin(), numVertices);
			}
			break;

			// all 4 byte
		case NxRenderDataFormat::UBYTE4:
		case NxRenderDataFormat::USHORT2:
		case NxRenderDataFormat::SHORT2:
		case NxRenderDataFormat::UINT1:
		case NxRenderDataFormat::R8G8B8A8:
		case NxRenderDataFormat::B8G8R8A8:
		case NxRenderDataFormat::BYTE_UNORM4:
		case NxRenderDataFormat::SHORT_UNORM2:
		case NxRenderDataFormat::BYTE_SNORM4:
		case NxRenderDataFormat::SHORT_SNORM2:
		case NxRenderDataFormat::HALF2:
		case NxRenderDataFormat::FLOAT1:
		case NxRenderDataFormat::BYTE_SNORM4_QUATXYZW:
		case NxRenderDataFormat::SHORT_SNORM4_QUATXYZW:
			{
				BufferU32x1* int1 = static_cast<BufferU32x1*>(bufferInterface);
				PX_ASSERT(numVertices == (physx::PxU32)int1->data.arraySizes[0]);
				ApexPermute(int1->data.buf, permutation.begin(), numVertices);
			}
			break;

			// all 6 byte
		case NxRenderDataFormat::USHORT3:
		case NxRenderDataFormat::SHORT3:
		case NxRenderDataFormat::SHORT_UNORM3:
		case NxRenderDataFormat::SHORT_SNORM3:
		case NxRenderDataFormat::HALF3:
			{
				BufferU16x3* short3 = static_cast<BufferU16x3*>(bufferInterface);
				PX_ASSERT(numVertices == (physx::PxU32)short3->data.arraySizes[0]);
				ApexPermute(short3->data.buf, permutation.begin(), numVertices);
			}
			break;

			// all 8 byte
		case NxRenderDataFormat::USHORT4:
		case NxRenderDataFormat::SHORT4:
		case NxRenderDataFormat::SHORT_UNORM4:
		case NxRenderDataFormat::SHORT_SNORM4:
		case NxRenderDataFormat::UINT2:
		case NxRenderDataFormat::HALF4:
		case NxRenderDataFormat::FLOAT2:
			{
				BufferU32x2* int2 = static_cast<BufferU32x2*>(bufferInterface);
				PX_ASSERT(numVertices == (physx::PxU32)int2->data.arraySizes[0]);
				ApexPermute(int2->data.buf, permutation.begin(), numVertices);
			}
			break;

			// all 12 byte
		case NxRenderDataFormat::UINT3:
		case NxRenderDataFormat::FLOAT3:
			{
				BufferU32x3* int3 = static_cast<BufferU32x3*>(bufferInterface);
				PX_ASSERT(numVertices == (physx::PxU32)int3->data.arraySizes[0]);
				ApexPermute(int3->data.buf, permutation.begin(), numVertices);
			}
			break;

			// all 16 byte
		case NxRenderDataFormat::UINT4:
		case NxRenderDataFormat::R32G32B32A32_FLOAT:
		case NxRenderDataFormat::B32G32R32A32_FLOAT:
		case NxRenderDataFormat::FLOAT4:
		case NxRenderDataFormat::FLOAT4_QUAT:
			{
				BufferU32x4* int4 = static_cast<BufferU32x4*>(bufferInterface);
				PX_ASSERT(numVertices == (physx::PxU32)int4->data.arraySizes[0]);
				ApexPermute(int4->data.buf, permutation.begin(), numVertices);
			}
			break;

			// all 36 byte
		case NxRenderDataFormat::FLOAT3x3:
			{
				BufferF32x1* float1 = static_cast<BufferF32x1*>(bufferInterface);
				PX_ASSERT(numVertices == (physx::PxU32)float1->data.arraySizes[0]);
				ApexPermute((PxMat33*)float1->data.buf, permutation.begin(), numVertices);
			}
			break;

			// all 48 byte
		case NxRenderDataFormat::FLOAT3x4:
			{
				BufferF32x1* float1 = static_cast<BufferF32x1*>(bufferInterface);
				PX_ASSERT(numVertices == (physx::PxU32)float1->data.arraySizes[0]);
				ApexPermute((PxMat34Legacy*)float1->data.buf, permutation.begin(), numVertices);
			}
			break;

		// fix gcc warnings
		case NxRenderDataFormat::UNSPECIFIED:
		case NxRenderDataFormat::NUM_FORMATS:
			break;
		}
	}
}



}
} // end namespace physx::apex
