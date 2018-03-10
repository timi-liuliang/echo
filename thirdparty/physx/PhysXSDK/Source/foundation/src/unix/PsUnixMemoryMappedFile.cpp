/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  
#include "PsMemoryMappedFile.h"

namespace physx
{
	namespace shdfnd
	{

class MemoryMappedFileImpl
{
public:
	void *  mHeader;
};

MemoryMappedFile::MemoryMappedFile(const char *mappingObject,unsigned int mapSize)
{
	mImpl = (MemoryMappedFileImpl *)PX_ALLOC(sizeof(MemoryMappedFileImpl), PX_DEBUG_EXP("MemoryMappedFileImpl"));
	mImpl->mHeader = 0;
	PX_ASSERT(0); // not implemented

}

MemoryMappedFile::~MemoryMappedFile(void)
{
	PX_FREE(mImpl);
}

void * MemoryMappedFile::getBaseAddress(void)
{
	return mImpl->mHeader;
}

} // end of namespace
} // end of namespace
