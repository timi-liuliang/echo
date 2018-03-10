/*=============================================================================
	LMMemory.cpp: Memory allocation functionality
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "stdafx.h"
#include "LMCore.h"
#include "LMMallocProfiler.h"

namespace Lightmass
{

/** The global allocator. */
FMallocPtr	GMalloc;

/** Total number of calls Malloc, if implemented by derived class. */
QWORD FMalloc::TotalMallocCalls = 0;
/** Total number of calls Malloc, if implemented by derived class. */
QWORD FMalloc::TotalFreeCalls = 0;
/** Total number of calls Malloc, if implemented by derived class. */
QWORD FMalloc::TotalReallocCalls = 0;
/** Total number of calls to PhysicalAlloc, if implemented by derived class. */
QWORD FMalloc::TotalPhysicalAllocCalls = 0;
/** Total number of calls to PhysicalFree, if implemented by derived class. */
QWORD FMalloc::TotalPhysicalFreeCalls = 0;


class FMallocWin : public FMalloc
{
	virtual void* Malloc( SIZE_T Size, DWORD Alignment=DEFAULT_ALIGNMENT )
	{
		TotalMallocCalls++;
		return _aligned_malloc( Size, DEFAULT_ALIGNMENT );
	}
	virtual void* Realloc( void* Ptr, SIZE_T Size, DWORD Alignment=DEFAULT_ALIGNMENT )
	{
		TotalReallocCalls++;
		return _aligned_realloc( Ptr, Size, Alignment );
	}
	virtual void Free( void* Ptr )
	{
		TotalFreeCalls++;
		_aligned_free( Ptr );
	}
};

void FMallocPtr::CreateAllocator()
{
#if USE_MALLOC_PROFILER
	static FMallocProfiler& WinAlloc( FMallocProfiler::StaticInit(GetFallbackAllocator()) );
	Allocator = &WinAlloc;
#else
	Allocator = GetFallbackAllocator();
#endif
}

FMalloc* FMallocPtr::GetFallbackAllocator()
{
	static FMallocWin WinAlloc;
	return &WinAlloc;
}


/*-----------------------------------------------------------------------------
	Memory functions.
-----------------------------------------------------------------------------*/
TCHAR GOutOfMemoryString[4096];
#define	CONVERT_TO_MB	(1.0f / (1024.0f*1024.0f))
const TCHAR* appOutOfMemoryMessage(const TCHAR* AttemptedOperation, SIZE_T RequestedSize)
{
	MEMORYSTATUSEX	MemStatus;

	MemStatus.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&MemStatus);

	_stprintf(GOutOfMemoryString, 
		TEXT("OUT OF MEMORY: %8s: %12u bytes (%12.2fMB)  requested, %12.2f of %12.2f MB of virtual memory free, %12.2f of %12.2f MB of physical memory free, %12.2f of %12.2f MB of page file free"), 
		AttemptedOperation, 
		RequestedSize, RequestedSize * CONVERT_TO_MB, 
		MemStatus.ullAvailVirtual * CONVERT_TO_MB, MemStatus.ullTotalVirtual * CONVERT_TO_MB,
		MemStatus.ullAvailPhys * CONVERT_TO_MB, MemStatus.ullTotalPhys * CONVERT_TO_MB,
		MemStatus.ullAvailPageFile * CONVERT_TO_MB, MemStatus.ullTotalPageFile * CONVERT_TO_MB
		);

	return GOutOfMemoryString;
}

void* appMalloc( SIZE_T Size )
{
	void* Ptr = GMalloc->Malloc(Size);
	checkf(Ptr, appOutOfMemoryMessage(TEXT("Malloc"), Size));
	return Ptr;
}

void* appRealloc( void* Ptr, SIZE_T Size )
{
	void* NewPtr = GMalloc->Realloc(Ptr, Size, DEFAULT_ALIGNMENT);
	checkf(NewPtr || Size==0, appOutOfMemoryMessage(TEXT("Realloc"), Size));
	return NewPtr;
}

void appFree( void* Ptr )
{
	GMalloc->Free( Ptr );
}

void* appMemmove( void* Dest, const void* Src, SIZE_T Count )
{
	return memmove( Dest, Src, Count );
}

void appMemswap( void* Ptr1, void* Ptr2, SIZE_T Size )
{
	void* Temp = appAlloca(Size);
	appMemcpy( Temp, Ptr1, Size );
	appMemcpy( Ptr1, Ptr2, Size );
	appMemcpy( Ptr2, Temp, Size );
}

//
// CRC32 computer based on CRC32_POLY.
//
DWORD appMemCrc( const void* InData, SIZE_T Length, DWORD CRC )
{
	BYTE* Data = (BYTE*)InData;
	CRC = ~CRC;
	for( SIZE_T i=0; i<Length; i++ )
		CRC = (CRC << 8) ^ GCRCTable.CRC[(CRC >> 24) ^ Data[i]];
	return ~CRC;
}


}
