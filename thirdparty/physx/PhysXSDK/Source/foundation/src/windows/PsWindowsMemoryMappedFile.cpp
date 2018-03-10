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
#include "windows/PsWindowsInclude.h"

namespace physx
{
	namespace shdfnd
	{

class MemoryMappedFileImpl
{
public:
	HANDLE	mMapFile;
	void *  mHeader;
};

MemoryMappedFile::MemoryMappedFile(const char *mappingObject,unsigned int mapSize)
{
	mImpl = (MemoryMappedFileImpl *)PX_ALLOC(sizeof(MemoryMappedFileImpl), PX_DEBUG_EXP("MemoryMappedFileImpl"));
	mImpl->mHeader = NULL;
#ifndef PX_WINMODERN
   	mImpl->mMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS,FALSE,mappingObject);
	if ( mImpl->mMapFile == NULL )
	{
		mImpl->mMapFile = CreateFileMappingA(
      				INVALID_HANDLE_VALUE,    // use paging file
       				NULL,                    // default security
       				PAGE_READWRITE,          // read/write access
       				0,                       // maximum object size (high-order DWORD)
       				mapSize,                // maximum object size (low-order DWORD)
       				mappingObject);
	}
	if ( mImpl->mMapFile )
	{
		mImpl->mHeader = MapViewOfFile(mImpl->mMapFile,FILE_MAP_ALL_ACCESS,0,0,mapSize);
	}
#else
	// convert to unicode
	const static int BUFFERSIZE = 256;
	WCHAR buffer[BUFFERSIZE];
	int succ = MultiByteToWideChar(CP_ACP, 0, mappingObject, -1, buffer, BUFFERSIZE);
	// validate
	if (succ < 0)
		succ = 0;
	if (succ < BUFFERSIZE)
		buffer[succ] = 0;
	else if (buffer[BUFFERSIZE-1])
		buffer[0] = 0;

	mImpl->mMapFile = (succ > 0) ? CreateFileMappingFromApp(
		INVALID_HANDLE_VALUE,    // use paging file
       	NULL,                    // default security
       	PAGE_READWRITE,          // read/write access
       	ULONG64(mapSize),        // maximum object size (low-order DWORD)
       	buffer) 
		: NULL;
#endif
}

MemoryMappedFile::~MemoryMappedFile(void)
{

	if ( mImpl->mHeader )
   	{
   		UnmapViewOfFile(mImpl->mHeader);
   		if ( mImpl->mMapFile )
   		{
   			CloseHandle(mImpl->mMapFile);
   		}
	}

	PX_FREE(mImpl);
}

void * MemoryMappedFile::getBaseAddress(void)
{
	return mImpl->mHeader;
}

}; // end of namespace
}; // end of namespace
