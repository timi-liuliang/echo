/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexCustomBufferIterator.h"

namespace physx
{
namespace apex
{

ApexCustomBufferIterator::ApexCustomBufferIterator() :
	mData(NULL),
	mElemSize(0),
	mMaxTriangles(0)
{
}

void ApexCustomBufferIterator::setData(void* data, physx::PxU32 elemSize, physx::PxU32 maxTriangles)
{
	mData = (physx::PxU8*)data;
	mElemSize = elemSize;
	mMaxTriangles = maxTriangles;
}

void ApexCustomBufferIterator::addCustomBuffer(const char* name, NxRenderDataFormat::Enum format, physx::PxU32 offset)
{
	CustomBuffer buffer;
	buffer.name = name;
	buffer.offset = offset;
	buffer.format = format;

	mCustomBuffers.pushBack(buffer);
}
void* ApexCustomBufferIterator::getVertex(physx::PxU32 triangleIndex, physx::PxU32 vertexIndex) const
{
	if (mData == NULL || triangleIndex >= mMaxTriangles)
	{
		return NULL;
	}

	return mData + mElemSize * (triangleIndex * 3 + vertexIndex);
}
physx::PxI32 ApexCustomBufferIterator::getAttributeIndex(const char* attributeName) const
{
	if (attributeName == NULL || attributeName[0] == 0)
	{
		return -1;
	}

	for (physx::PxU32 i = 0; i < mCustomBuffers.size(); i++)
	{
		if (strcmp(mCustomBuffers[i].name, attributeName) == 0)
		{
			return (physx::PxI32)i;
		}
	}
	return -1;
}
void* ApexCustomBufferIterator::getVertexAttribute(physx::PxU32 triangleIndex, physx::PxU32 vertexIndex, const char* attributeName, NxRenderDataFormat::Enum& outFormat) const
{
	outFormat = NxRenderDataFormat::UNSPECIFIED;

	physx::PxU8* elementData = (physx::PxU8*)getVertex(triangleIndex, vertexIndex);
	if (elementData == NULL)
	{
		return NULL;
	}


	for (physx::PxU32 i = 0; i < mCustomBuffers.size(); i++)
	{
		if (strcmp(mCustomBuffers[i].name, attributeName) == 0)
		{
			outFormat = mCustomBuffers[i].format;
			return elementData + mCustomBuffers[i].offset;
		}
	}
	return NULL;
}

void* ApexCustomBufferIterator::getVertexAttribute(physx::PxU32 triangleIndex, physx::PxU32 vertexIndex, physx::PxU32 attributeIndex, NxRenderDataFormat::Enum& outFormat, const char*& outName) const
{
	outFormat = NxRenderDataFormat::UNSPECIFIED;
	outName = NULL;

	physx::PxU8* elementData = (physx::PxU8*)getVertex(triangleIndex, vertexIndex);
	if (elementData == NULL || attributeIndex >= mCustomBuffers.size())
	{
		return NULL;
	}

	outName = mCustomBuffers[attributeIndex].name;
	outFormat = mCustomBuffers[attributeIndex].format;
	return elementData + mCustomBuffers[attributeIndex].offset;
}

}
} // end namespace physx::apex
