/*=============================================================================
	FMallocProfiler.h: Memory profiling support.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
#ifndef FMALLOC_PROFILER_H
#define FMALLOC_PROFILER_H

/** Set USE_MALLOC_PROFILER to 1 to support memory profiling. */
#ifndef USE_MALLOC_PROFILER
	#define USE_MALLOC_PROFILER				0
#endif

#if USE_MALLOC_PROFILER

namespace Lightmass
{

/*=============================================================================
	FBufferedFileWriter
=============================================================================*/

/**
 * Special buffered file writer, used to serialize data before GFileManager is initialized.
 */
class FMallocProfilerBufferedFileWriter
{
public:
	/** Internal file writer used to serialize to HDD.							*/
	HANDLE			FileHandle;
	/** Buffered data being serialized before GFileManager has been set up.		*/
	TArray<BYTE>	BufferedData;
	/** Timestamped filename with path.											*/
	FString			FullFilepath;
	/** File number. Index 0 is the base .mprof file.							*/
	INT				FileNumber;

	/**
	 * Constructor. Called before GMalloc is initialized!!!
	 */
	FMallocProfilerBufferedFileWriter();

	/**
	 * Constructor. Opens an existing file for append.
	 */
	FMallocProfilerBufferedFileWriter( FString& FilePath, INT InFileNumber );

	/**
	 * Destructor, cleaning up FileWriter.
	 */
	virtual ~FMallocProfilerBufferedFileWriter();

	/**
	 * Intitalize the file.
	 */
	void Init( UBOOL bReopenExistingFile );

	/**
	 * Whether we are currently buffering to memory or directly writing to file.
	 *
	 * @return	TRUE if buffering to memory, FALSE otherwise
	 */
	UBOOL IsBufferingToMemory();

	/**
	 * Splits the file and creates a new one with different extension.
	 */
	void Split();

	// FArchive interface.
	virtual void Serialize( void* V, INT Length );
	virtual void Seek( INT InPos );
	virtual UBOOL Close();
	virtual INT Tell();
};

/*=============================================================================
	FMallocProfiler
=============================================================================*/

/**
 * Memory profiling malloc, routing allocations to real malloc and writing information on all 
 * operations to a file for analysis by a standalone tool.
 */
class FMallocProfiler : public FMalloc
{
public:
	/**
	 * Constructor, initializing all member variables and potentially loading symbols.
	 *
	 * @param	InMalloc	The allocator wrapped by FMallocProfiler that will actually do the allocs/deallocs.
	 */
	FMallocProfiler(FMalloc* InMalloc);

	static FMallocProfiler& StaticInit(FMalloc* InMalloc);

	virtual void* Malloc( DWORD Size, DWORD Alignment )
	{
		FScopeLock Lock( &CriticalSection );
		void* Ptr = UsedMalloc->Malloc( Size, Alignment );
		TrackMalloc( Ptr, Size );
		return Ptr;
	}
	virtual void* Realloc( void* OldPtr, DWORD NewSize, DWORD Alignment )
	{
		FScopeLock Lock( &CriticalSection );
		void* NewPtr = UsedMalloc->Realloc( OldPtr, NewSize, Alignment );
		TrackRealloc( OldPtr, NewPtr, NewSize );
		return NewPtr;
	}
	virtual void Free( void* Ptr )
	{
		FScopeLock Lock( &CriticalSection );
		UsedMalloc->Free( Ptr );
		TrackFree( Ptr );
	}

	/**
	 * Embeds token into stream to snapshot memory at this point.
	 */
	virtual void SnapshotMemory();

	/**
	 * Writes out the profiling data to disk.
	 */
	virtual void	WriteProfilingData();

private:
	/** Critical section to sequence tracking.														*/
	FCriticalSection	CriticalSection;
	/** Malloc we're based on, aka using under the hood												*/
	FMalloc*			UsedMalloc;
	/** Whether operations should be tracked. FALSE e.g. in tracking internal functions.			*/
	UBOOL				bShouldTrackOperations;
	/** Whether or not EndProfiling()  has been Ended.  Once it has been ended it has ended most operations are no longer valid **/
	UBOOL               bEndProfilingHasBeenCalled;
	/** Time malloc profiler was created. Used to convert arbitrary DOUBLE time to relative FLOAT.	*/
	DOUBLE				StartTime;
	/** File writer used to serialize data. Safe to call before GFileManager has been initialized.	*/
	FMallocProfilerBufferedFileWriter BufferedFileWriter;

	/** Mapping from program counter to index in program counter array.								*/
	TMap<QWORD,INT>		ProgramCounterToIndexMap;
	/** Array of unique call stack address infos.													*/
	TArray<struct FCallStackAddressInfo> CallStackAddressInfoArray;

	/** Mapping from callstack CRC to index in call stack info array.								*/
	TMap<DWORD,INT>		CRCToCallStackIndexMap;
	/** Array of unique call stack infos.															*/
	TArray<struct FCallStackInfo> CallStackInfoArray;

	/** Mapping from name to index in name array.													*/
	TMap<FString,INT>	NameToNameTableIndexMap;
	/** Array of unique names.																		*/
	TArray<FString>		NameArray;

	/** 
	 * Returns index of callstack, captured by this function into array of callstack infos. If not found, adds it.
	 *
	 * @return index into callstack array
	 */
	INT GetCallStackIndex();

	/**
	 * Returns index of passed in program counter into program counter array. If not found, adds it.
	 *
	 * @param	ProgramCounter	Program counter to find index for
	 * @return	Index of passed in program counter if it's not NULL, INDEX_NONE otherwise
	 */
	INT GetProgramCounterIndex( QWORD ProgramCounter );

	/**
	 * Returns index of passed in name into name array. If not found, adds it.
	 *
	 * @param	Name	Name to find index for
	 * @return	Index of passed in name
	 */
	INT GetNameTableIndex( const FString& Name );

	/**
	 * Tracks malloc operation.
	 *
	 * @param	Ptr		Allocated pointer 
	 * @param	Size	Size of allocated pointer
	 */
	void TrackMalloc( void* Ptr, DWORD Size );
	
	/**
	 * Tracks free operation
	 *
	 * @param	Ptr		Freed pointer
	 */
	void TrackFree( void* Ptr );
	
	/**
	 * Tracks realloc operation
	 *
	 * @param	OldPtr	Previous pointer
	 * @param	NewPtr	New pointer
	 * @param	NewSize	New size of allocation
	 */
	void TrackRealloc( void* OldPtr, void* NewPtr, DWORD NewSize );

	/**
	 * Ends profiling operation and closes file.
	 */
	void EndProfiling();

	/** 
	 * Checks whether file is too big and will create new files with different extension but same base name.
	 */
	void PossiblySplitMprof();
};

}	//namespace Lightmass

#endif //USE_MALLOC_PROFILER

#endif	//#ifndef FMALLOC_PROFILER_H
