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


#ifndef PS_MEMORY_MAPPED_FILE_H
#define PS_MEMORY_MAPPED_FILE_H

#include "Ps.h"
#include "foundation/PxSimpleTypes.h"
#include "PsUserAllocated.h"

namespace physx
{
	namespace shdfnd
	{

class MemoryMappedFile : public UserAllocated
{
public:

	MemoryMappedFile(const char *mappingObject,unsigned int mapSize);
	~MemoryMappedFile(void);
	void * getBaseAddress(void);
private:
	class	MemoryMappedFileImpl	*mImpl;
};
	} // end of namespace
} // end of namespace

#endif
