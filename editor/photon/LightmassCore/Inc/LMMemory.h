/*=============================================================================
	LMMemory.h: Memory allocation functionality
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

namespace Lightmass
{

/** @name Memory functions */
//@{
/** Copies count bytes of characters from Src to Dest. If some regions of the source
 * area and the destination overlap, memmove ensures that the original source bytes
 * in the overlapping region are copied before being overwritten.  NOTE: make sure
 * that the destination buffer is the same size or larger than the source buffer!
 */
void* appMemmove( void* Dest, const void* Src, SIZE_T Count );
INT appMemcmp( const void* Buf1, const void* Buf2, SIZE_T Count );
UBOOL appMemIsZero( const void* V, SIZE_T Count );
DWORD appMemCrc( const void* Data, SIZE_T Length, DWORD CRC=0 );
void appMemswap( void* Ptr1, void* Ptr2, SIZE_T Size );
void* appMalloc(SIZE_T Size);
void* appRealloc(void* Ptr, SIZE_T Size);
void appFree(void* Ptr);

/**
 * Sets the first Count chars of Dest to the character C.
 */
#define appMemset( Dest, C, Count )		memset( Dest, C, Count )
#define appMemcpy( Dest, Src, Count )	memcpy( Dest, Src, Count )
#define appMemzero( Dest, Count )		memset( Dest, 0, Count )
#define appAlloca(size)					((size==0) ? 0 : _alloca((size+7)&~7))

//
// C style memory allocation stubs that fall back to C runtime
//
#ifndef appSystemMalloc
#define appSystemMalloc		malloc
#endif
#ifndef appSystemFree
#define appSystemFree		free
#endif
#ifndef appSystemRealloc
#define appSystemRealloc	realloc
#endif

/** The global memory allocator's interface. */
class FMalloc
{
public:
	virtual void*	Malloc( SIZE_T Count, DWORD Alignment=DEFAULT_ALIGNMENT ) = 0;
	virtual void*	Realloc( void* Original, SIZE_T Count, DWORD Alignment=DEFAULT_ALIGNMENT ) = 0;
	virtual void	Free( void* Original ) = 0;
	virtual void	SnapshotMemory() {}
	virtual void	WriteProfilingData() {}

protected:
	/** Total number of calls Malloc, if implemented by derived class. */
	static QWORD TotalMallocCalls;
	/** Total number of calls Malloc, if implemented by derived class. */
	static QWORD TotalFreeCalls;
	/** Total number of calls Malloc, if implemented by derived class. */
	static QWORD TotalReallocCalls;
	/** Total number of calls to PhysicalAlloc, if implemented by derived class. */
	static QWORD TotalPhysicalAllocCalls;
	/** Total number of calls to PhysicalFree, if implemented by derived class. */
	static QWORD TotalPhysicalFreeCalls;
};

class FMallocPtr
{
public:
	FMallocPtr()
	:	Allocator(NULL)
	,	bReentry(FALSE)
	{
	}
	UBOOL		IsInitialized() const
	{
		return Allocator != NULL;
	}
	FMalloc*	operator->()
	{
		if ( Allocator == NULL )
		{
			if ( bReentry )
			{
				return GetFallbackAllocator();
			}
			else
			{
				bReentry = TRUE;
				CreateAllocator();
				bReentry = FALSE;
			}
		}
		return Allocator;
	}
private:
	void		CreateAllocator();
	FMalloc*	GetFallbackAllocator();

	FMalloc*	Allocator;
	UBOOL		bReentry;
};

/** The global allocator. */
extern FMallocPtr	GMalloc;


}

//
// C++ style memory allocation.
//
FORCEINLINE void* operator new( size_t Size )
{
	return Lightmass::appMalloc( Size );
}
FORCEINLINE void* operator new( size_t Size, void* Ptr )
{
	return Ptr;
}
FORCEINLINE void operator delete( void* Ptr )
{
	Lightmass::appFree( Ptr );
}
FORCEINLINE void* operator new[]( size_t Size )
{
	return Lightmass::appMalloc( Size );
}
FORCEINLINE void operator delete[]( void* Ptr )
{
	Lightmass::appFree( Ptr );
}
