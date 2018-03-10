/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEX_CUSTOM_BUFFER_ITERARTOR_H__
#define __APEX_CUSTOM_BUFFER_ITERARTOR_H__

#include "NxApexCustomBufferIterator.h"
#include <PsUserAllocated.h>
#include <PsArray.h>
#include <PsShare.h>

namespace physx
{
namespace apex
{

class ApexCustomBufferIterator : public NxApexCustomBufferIterator, public physx::UserAllocated
{
public:
	ApexCustomBufferIterator();

	// NxApexCustomBufferIterator methods

	virtual void		setData(void* data, physx::PxU32 elemSize, physx::PxU32 maxTriangles);

	virtual void		addCustomBuffer(const char* name, NxRenderDataFormat::Enum format, physx::PxU32 offset);

	virtual void*		getVertex(physx::PxU32 triangleIndex, physx::PxU32 vertexIndex) const;

	virtual physx::PxI32		getAttributeIndex(const char* attributeName) const;

	virtual void*		getVertexAttribute(physx::PxU32 triangleIndex, physx::PxU32 vertexIndex, const char* attributeName, NxRenderDataFormat::Enum& outFormat) const;

	virtual void*		getVertexAttribute(physx::PxU32 triangleIndex, physx::PxU32 vertexIndex, physx::PxU32 attributeIndex, NxRenderDataFormat::Enum& outFormat, const char*& outName) const;

private:
	physx::PxU8* mData;
	physx::PxU32 mElemSize;
	physx::PxU32 mMaxTriangles;
	struct CustomBuffer
	{
		const char* name;
		physx::PxU32 offset;
		NxRenderDataFormat::Enum format;
	};
	physx::Array<CustomBuffer> mCustomBuffers;
};

}
} // end namespace physx::apex


#endif // __APEX_CUSTOM_BUFFER_ITERARTOR_H__
