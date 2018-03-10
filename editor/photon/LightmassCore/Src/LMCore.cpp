/*=============================================================================
	LMCore.h: Some implementation of LMCore functionality
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "stdafx.h"
#include <stdio.h>
#include <objbase.h>
#include "LMCore.h"

namespace Lightmass
{

/*-----------------------------------------------------------------------------
	Validate that our sizes/alignments are the same as the Base versions in the public header
-----------------------------------------------------------------------------*/
checkAtCompileTime(sizeof(FGuid) == sizeof(FGuidBase), LMTypeDifferentSizeThanBase);
checkAtCompileTime(sizeof(FMatrix) == sizeof(FMatrixBase), LMTypeDifferentSizeThanBase);
checkAtCompileTime(sizeof(FVector2D) == sizeof(FVector2DBase), LMTypeDifferentSizeThanBase);
checkAtCompileTime(sizeof(FVector4) == sizeof(FVector4Base), LMTypeDifferentSizeThanBase);
checkAtCompileTime(sizeof(FPlane) == sizeof(FVector4Base), LMTypeDifferentSizeThanBase);
checkAtCompileTime(sizeof(FColor) == sizeof(FColorBase), LMTypeDifferentSizeThanBase);
checkAtCompileTime(sizeof(FPackedNormal) == sizeof(FPackedNormalBase), LMTypeDifferentSizeThanBase);
checkAtCompileTime(sizeof(FBox) == sizeof(FBoxBase), LMTypeDifferentSizeThanBase);
checkAtCompileTime(sizeof(FFloat16) == sizeof(FFloat16Base), LMTypeDifferentSizeThanBase);
checkAtCompileTime(sizeof(FQuantizedSHVector) == sizeof(FQuantizedSHVectorBase), LMTypeDifferentSizeThanBase);
checkAtCompileTime(sizeof(FQuantizedSHVectorRGB) == sizeof(FQuantizedSHVectorRGBBase), LMTypeDifferentSizeThanBase);

checkAtCompileTime(__alignof(FGuid) == __alignof(FGuidBase), LMTypeDifferentAlignmentThanBase);
checkAtCompileTime(__alignof(FMatrix) == __alignof(FMatrixBase), LMTypeDifferentAlignmentThanBase);
checkAtCompileTime(__alignof(FVector2D) == __alignof(FVector2DBase), LMTypeDifferentAlignmentThanBase);
checkAtCompileTime(__alignof(FVector4) == __alignof(FVector4Base), LMTypeDifferentAlignmentThanBase);
checkAtCompileTime(__alignof(FPlane) == __alignof(FVector4Base), LMTypeDifferentAlignmentThanBase);
checkAtCompileTime(__alignof(FColor) == __alignof(FColorBase), LMTypeDifferentAlignmentThanBase);
checkAtCompileTime(__alignof(FPackedNormal) == __alignof(FPackedNormalBase), LMTypeDifferentAlignmentThanBase);
checkAtCompileTime(__alignof(FBox) == __alignof(FBoxBase), LMTypeDifferentAlignmentThanBase);
checkAtCompileTime(__alignof(FFloat16) == __alignof(FFloat16Base), LMTypeDifferentAlignmentThanBase);
checkAtCompileTime(__alignof(FQuantizedSHVector) == __alignof(FQuantizedSHVectorBase), LMTypeDifferentAlignmentThanBase);
checkAtCompileTime(__alignof(FQuantizedSHVectorRGB) == __alignof(FQuantizedSHVectorRGBBase), LMTypeDifferentAlignmentThanBase);

/**
 * Make sure we are compiling against the DXSDK we are expecting to,
 * Which is the June 2010 D3DX SDK.
 */
const INT REQUIRED_D3DX_SDK_VERSION = 43;
//checkAtCompileTime(D3DX_SDK_VERSION == REQUIRED_D3DX_SDK_VERSION, D3DX_SDK_VERSION_DoesNotMatchRequiredVersion);

/*-----------------------------------------------------------------------------
	Lightmass globals
-----------------------------------------------------------------------------*/

	
// single instance of a CRC table	
FCRCTable GCRCTable;
// number of seconds in 1 clock cycle
DOUBLE GSecondsPerCycle = 0.0f;

/** Whether the executable is guarded with a __try/__catch. */
UBOOL GIsGuarded = FALSE;

// logger
FLogInternal* FLog::Log = NULL;

/** Logger: Safe to use at any time (even before main or any global constructor). */
FLog GLog;

/**
 * A helper class that just does one time initialization of Lightmass related
 * functionality. It is done at an unknown time before main() is called so
 * no other globals should use Lightmass functionality
 */
class FLightmassGlobalInit
{
public:
	FLightmassGlobalInit()
	{
		// initialize GSecondsPerCycle
		LARGE_INTEGER Frequency;
		verify( QueryPerformanceFrequency(&Frequency) );
		GSecondsPerCycle = 1.0 / Frequency.QuadPart;

		// initialize threading managers
		GSynchronizeFactory = new FSynchronizeFactoryWin();
		GThreadFactory = new FThreadFactoryWin();
		GThreadPool = new FQueuedThreadPoolWin();
	}
};

/**
 * Cleanup code at exit. This is not called if there is a crash.
 */
void LightmassExit()
{
	// Tear down threading managers in reverse order of initialization.
	delete GThreadPool;
	GThreadPool = NULL;
	delete GThreadFactory;
	GThreadFactory = NULL;
	delete GSynchronizeFactory;
	GSynchronizeFactory = NULL;
}


// single instance of the initializer
static FLightmassGlobalInit GLightmassGlobalInitObject;

/** CPU frequency for stats, only used for inner loop timing with rdtsc. */
DOUBLE GCPUFrequency = 3000000000.0;

/** Number of CPU clock cycles per second (as counted by __rdtsc). */
DOUBLE GSecondPerCPUCycle = 1.0 / 3000000000.0;

struct FInitCPUFrequency
{
	unsigned __int64 StartCPUTime;
	unsigned __int64 EndCPUTime;
	DOUBLE StartTime;
	DOUBLE EndTime;
};
static FInitCPUFrequency GInitCPUFrequency;

/** Start initializing CPU frequency (as counted by __rdtsc). */
void StartInitCPUFrequency()
{
	GInitCPUFrequency.StartTime		= appSeconds();
	GInitCPUFrequency.StartCPUTime	= __rdtsc();
}

/** Finish initializing CPU frequency (as counted by __rdtsc), and set up CPUFrequency and CPUCyclesPerSecond. */
void FinishInitCPUFrequency()
{
	GInitCPUFrequency.EndTime		= appSeconds();
	GInitCPUFrequency.EndCPUTime	= __rdtsc();
	DOUBLE NumSeconds				= GInitCPUFrequency.EndTime - GInitCPUFrequency.StartTime;
	GCPUFrequency					= DOUBLE(GInitCPUFrequency.EndCPUTime - GInitCPUFrequency.StartCPUTime) / NumSeconds;
	GSecondPerCPUCycle				= NumSeconds / DOUBLE(GInitCPUFrequency.EndCPUTime - GInitCPUFrequency.StartCPUTime);
	debugf(TEXT("Measured CPU frequency: %.2f GHz"), GCPUFrequency/1000000000.0);
}


/*-----------------------------------------------------------------------------
	Misc helper functions.
-----------------------------------------------------------------------------*/

/**
 * Create a new globally unique identifier.
 */
FGuid appCreateGuid()
{
	FGuid Result(0,0,0,0);
	verify( CoCreateGuid( (GUID*)&Result )==S_OK );
	return Result;
}

/**
 * Returns the system time.
 */
void appSystemTime( INT& Year, INT& Month, INT& DayOfWeek, INT& Day, INT& Hour, INT& Min, INT& Sec, INT& MSec )
{
	SYSTEMTIME st;
	GetLocalTime( &st );

	Year		= st.wYear;
	Month		= st.wMonth;
	DayOfWeek	= st.wDayOfWeek;
	Day			= st.wDay;
	Hour		= st.wHour;
	Min			= st.wMinute;
	Sec			= st.wSecond;
	MSec		= st.wMilliseconds;
}

/**
 * Returns a string with a unique timestamp (useful for creating log filenames)
 */
FString appSystemTimeString()
{
	// Create string with system time to create a unique filename.
	INT Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec;

	appSystemTime( Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec );
	FString	CurrentTime = FString::Printf( TEXT( "%i.%02i.%02i-%02i.%02i.%02i" ), Year, Month, Day, Hour, Min, Sec );

	return( CurrentTime );
}

/**
 * Returns the file path in front of the filename, including the trailing path separator (e.g. '\').
 */
FString appGetFilePath( FString& FilePath )
{
	INT Pos = FilePath.InStr(PATH_SEPARATOR, TRUE);

	// in case we are using slashes on a platform that uses backslashes
	Pos = Max(Pos, FilePath.InStr(TEXT("/"), TRUE));

	// in case we are using backslashes on a platform that doesn't use backslashes
	Pos = Max(Pos, FilePath.InStr(TEXT("\\"), TRUE));
	if ( Pos != INDEX_NONE )
	{
		return FilePath.Left(Pos);
	}

	return FString();
}

/**
 * Returns the filename, including file extension.
 */
FString appGetCleanFilename( FString& FilePath )
{
	INT Pos = FilePath.InStr(PATH_SEPARATOR, TRUE);

	// in case we are using slashes on a platform that uses backslashes
	Pos = Max(Pos, FilePath.InStr(TEXT("/"), TRUE));

	// in case we are using backslashes on a platform that doesn't use backslashes
	Pos = Max(Pos, FilePath.InStr(TEXT("\\"), TRUE));

	if ( Pos != INDEX_NONE )
	{
		return FilePath.Mid(Pos + 1);
	}

	return FilePath;
}

/**
 * Returns the filename, not including file extension or the period.
 */
FString appGetBaseFilename( FString& FilePath, UBOOL bRemovePath )
{
	FString Wk = bRemovePath ? appGetCleanFilename(FilePath) : FilePath;

	// remove the extension
	INT Pos = Wk.InStr(TEXT("."), TRUE);
	if ( Pos != INDEX_NONE )
	{
		return Wk.Left(Pos);
	}

	return Wk;
}

//
// Convert an integer to a string.
//
// Faster Itoa that also appends to a string
void appItoaAppend( INT InNum,FString &NumberString )
{
	SQWORD	Num					= InNum; // This avoids having to deal with negating -MAXINT-1
	TCHAR*	NumberChar[11]		= { TEXT("0"), TEXT("1"), TEXT("2"), TEXT("3"), TEXT("4"), TEXT("5"), TEXT("6"), TEXT("7"), TEXT("8"), TEXT("9"), TEXT("-") };
	UBOOL	bIsNumberNegative	= FALSE;
	TCHAR	TempNum[16];		// 16 is big enough
	INT		TempAt				= 16; // fill the temp string from the top down.

	// Correctly handle negative numbers and convert to positive integer.
	if( Num < 0 )
	{
		bIsNumberNegative = TRUE;
		Num = -Num;
	}

	TempNum[--TempAt] = 0; // NULL terminator

	// Convert to string assuming base ten and a positive integer.
	do 
	{
		TempNum[--TempAt] = *NumberChar[Num % 10];
		Num /= 10;
	} while( Num );

	// Append sign as we're going to reverse string afterwards.
	if( bIsNumberNegative )
	{
		TempNum[--TempAt] = *NumberChar[10];
	}

	NumberString += TempNum + TempAt;
}

FString appItoa( INT InNum )
{
	FString NumberString;
	appItoaAppend(InNum,NumberString );
	return NumberString;
}

/** Get computer name.  NOTE: Only one return value is valid at a time! */
const TCHAR* appComputerName()
{
	static TCHAR Result[256]=TEXT("");
	if( !Result[0] )
	{
		unsigned long  Size=ARRAY_COUNT(Result);
		GetComputerName( Result, &Size );
	}
	return Result;
}

/** Get user name.  NOTE: Only one return value is valid at a time! */
const TCHAR* appUserName()
{
	static TCHAR Result[256]=TEXT("");
	if( !Result[0] )
	{
		unsigned long  Size=ARRAY_COUNT(Result);
		GetUserName( Result, &Size );
		TCHAR *c, *d;
		for( c=Result, d=Result; *c!=0; c++ )
			if( appIsAlnum(*c) )
				*d++ = *c;
		*d++ = 0;
	}
	return Result;
}

/** Command-line */
TCHAR GCmdLine[16384]=TEXT("");

/** Returns the executable command line. */
const TCHAR* appCmdLine()
{
	return GCmdLine;
}

/** Returns the startup directory (the directory this executable was launched from).  NOTE: Only one return value is valid at a time! */
const TCHAR* appBaseDir()
{
	static TCHAR Result[MAX_PATH]=TEXT("");
	GetCurrentDirectory(MAX_PATH, Result);
	return Result;
}

//
// Creates a new process and its primary thread. The new process runs the
// specified executable file in the security context of the calling process.
//
void *appCreateProc( const TCHAR* URL, const TCHAR* Parms )
{
	debugf( TEXT("CreateProc %s %s"), URL, Parms );

	FString CommandLine = FString::Printf(TEXT("%s %s"), URL, Parms);

	PROCESS_INFORMATION ProcInfo;
	SECURITY_ATTRIBUTES Attr;
	Attr.nLength = sizeof(SECURITY_ATTRIBUTES);
	Attr.lpSecurityDescriptor = NULL;
	Attr.bInheritHandle = TRUE;

	STARTUPINFO StartupInfo = { sizeof(STARTUPINFO), NULL, NULL, NULL,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, NULL, NULL, SW_HIDE, NULL, NULL,
		NULL, NULL, NULL };
	if( !CreateProcess( NULL, CommandLine.GetCharArray().GetTypedData(), &Attr, &Attr, TRUE, DETACHED_PROCESS | NORMAL_PRIORITY_CLASS,
		NULL, NULL, &StartupInfo, &ProcInfo ) )
	{
		return NULL;
	}
	CloseHandle( ProcInfo.hThread );
	return (void*)ProcInfo.hProcess;
}

/**
 * Copy text to clipboard.
 */
void appClipboardCopy( const TCHAR* Str )
{
	if( OpenClipboard(GetActiveWindow()) )
	{
		verify(EmptyClipboard());
		HGLOBAL GlobalMem;
		INT StrLen = appStrlen(Str);
		GlobalMem = GlobalAlloc( GMEM_DDESHARE | GMEM_MOVEABLE, sizeof(TCHAR)*(StrLen+1) );
		check(GlobalMem);
		TCHAR* Data = (TCHAR*) GlobalLock( GlobalMem );
		appStrcpy( Data, (StrLen+1), Str );
		GlobalUnlock( GlobalMem );
		if( SetClipboardData( CF_UNICODETEXT, GlobalMem ) == NULL )
			appErrorf(TEXT("SetClipboardData failed with error code %i"), GetLastError() );
		verify(CloseClipboard());
	}
}

/**
 * Paste text from clipboard into an FString.
 * @return	FString containing the text from the clipboard
 */
FString appClipboardPaste()
{
	FString Result;
	if( OpenClipboard(GetActiveWindow()) )
	{
		HGLOBAL GlobalMem = NULL;
		UBOOL Unicode = 0;
		GlobalMem = GetClipboardData( CF_UNICODETEXT );
		Unicode = 1;
		if( !GlobalMem )
		{
			GlobalMem = GetClipboardData( CF_TEXT );
			Unicode = 0;
		}
		if( !GlobalMem )
		{
			Result = TEXT("");
		}
		else
		{
			void* Data = GlobalLock( GlobalMem );
			check( Data );	
			if( Unicode )
				Result = (TCHAR*) Data;
			else
			{
				ANSICHAR* ACh = (ANSICHAR*) Data;
				INT i;
				for( i=0; ACh[i]; i++ );
				TArray<TCHAR> Ch(i+1);
				for( i=0; i<Ch.Num(); i++ )
					Ch(i)=FromAnsi(ACh[i]);
				Result = &Ch(0);
			}
			GlobalUnlock( GlobalMem );
		}
		verify(CloseClipboard());
	}
	else Result=TEXT("");

	return Result;
}

/**
 * Returns a pretty-string for a time given in seconds. (I.e. "4:31 min", "2:16:30 hours", etc)
 * @param Seconds	Time in seconds
 * @return			Time in a pretty formatted string
 */
FString appPrettyTime( DOUBLE Seconds )
{
	if ( Seconds < 1.0 )
	{
		return FString::Printf( TEXT("%d ms"), appTrunc(Seconds*1000) );
	}
	else if ( Seconds < 10.0 )
	{
		INT Sec = appTrunc(Seconds);
		INT Ms = appTrunc(Seconds*1000) - Sec*1000;
		return FString::Printf( TEXT("%d.%02d sec"), Sec, Ms/10 );
	}
	else if ( Seconds < 60.0 )
	{
		INT Sec = appTrunc(Seconds);
		INT Ms = appTrunc(Seconds*1000) - Sec*1000;
		return FString::Printf( TEXT("%d.%d sec"), Sec, Ms/100 );
	}
	else if ( Seconds < 60.0*60.0 )
	{
		INT Min = appTrunc(Seconds/60.0);
		INT Sec = appTrunc(Seconds) - Min*60;
		return FString::Printf( TEXT("%d:%02d min"), Min, Sec );
	}
	else
	{
		INT Hr = appTrunc(Seconds/60.0/60.0);
		INT Min = appTrunc(Seconds/60.0) - Hr*60*60;
		INT Sec = appTrunc(Seconds) - Hr*60*60*60 - Min*60;
		return FString::Printf( TEXT("%d:%02d:%02d hours"), Hr, Min, Sec );
	}
}

/**
* Helper function to write formatted output using an argument list
*
* @param Dest - destination string buffer
* @param DestSize - size of destination buffer
* @param Count - number of characters to write (not including null terminating character)
* @param Fmt - string to print
* @param Args - argument list
* @return number of characters written or -1 if truncated
*/
INT appGetVarArgs( TCHAR* Dest, SIZE_T DestSize, INT Count, const TCHAR*& Fmt, va_list ArgPtr )
{
	INT Result = _vsntprintf_s(Dest,DestSize,Count,Fmt,ArgPtr);
	va_end( ArgPtr );
	return Result;
}


VARARG_BODY( FString, FString::Printf, const TCHAR*, VARARG_NONE )
{
	INT		BufferSize	= 1024;
	TCHAR*	Buffer		= NULL;
	INT		Result		= -1;

	while(Result == -1)
	{
		Buffer = (TCHAR*) appRealloc( Buffer, BufferSize * sizeof(TCHAR) );
		GET_VARARGS_RESULT( Buffer, BufferSize, BufferSize-1, Fmt, Fmt, Result );
		BufferSize *= 2;
	};
	Buffer[Result] = 0;

	FString ResultString(Buffer);
	appFree( Buffer );

	return ResultString;
}


/*-----------------------------------------------------------------------------
	Logging functionality
-----------------------------------------------------------------------------*/

/**
 * Constructor. Will open up a file to write output to, to simulate UE3's logging
 */
FLogInternal::FLogInternal()
{
	Filename[0] = 0;

	// Create a Guid for this run.
	FGuid Guid = appCreateGuid();

	// get the app name to base the log name off of
	TCHAR ExeName[MAX_PATH];
	GetModuleFileName(NULL, ExeName, MAX_PATH);

	// Extract filename part and add "-[guid].log"
	INT ExeNameLen = appStrlen( ExeName );
	INT PathSeparatorPos = ExeNameLen;
	while ( PathSeparatorPos >= 0 && ExeName[PathSeparatorPos] != TEXT('\\') )
	{
		PathSeparatorPos--;
	}
	//FString LogName( ExeNameLen - PathSeparatorPos - 5, ExeName + PathSeparatorPos + 1 );
	//LogName += TEXT("_");
	//LogName += appComputerName();
	//LogName += TEXT("_");
	//LogName += Guid.String();
	//LogName += TEXT(".log");
	//appStrncpy( Filename, *LogName, MAX_PATH );

	FString LogName( "./Lightmass.log");

	// open the file for writing
	File = fopen(*LogName, "w");

	// mark the file to be unicode
	if (File)
	{
		//WORD UnicodeBOM = 0xfeff;

		//fwrite( &UnicodeBOM, sizeof(WORD), 1, File);
	}
	else
	{
		// print to the screen that we failed to open the file
		printf(TEXT("\nFailed to open the log file '%s' for writing\n\n"), *LogName);
	}
}

/**
 * Destructor. Closes the file.
 */
FLogInternal::~FLogInternal()
{
	fflush( File);
	fclose( File);
}

/**
 * Varargs logging
 */
VARARG_BODY( void, FLogInternal::Logf, const TCHAR*, VARARG_NONE )
{
	INT		BufferSize	= 1024;
	TCHAR*	Buffer		= NULL;
	INT		Result		= -1;

	// do the usual VARARGS shenanigans
	while(Result == -1)
	{
		appFree(Buffer);
		Buffer = (TCHAR*) appMalloc( BufferSize * sizeof(TCHAR) );
		GET_VARARGS_RESULT( Buffer, BufferSize, BufferSize-1, Fmt, Fmt, Result );
		BufferSize *= 2;
	};
	Buffer[Result] = 0;

	Log( Buffer );

	appFree( Buffer );
}

/**
 * Logs a text string.
 */
void FLogInternal::Log( const TCHAR* Text )
{
	// write it out to disk
	if (File)
	{
		fputs( Text, File);
		fputs( "\r\n", File);
		//fwrite( Text, strlen(Text)+1, 1, File);
		//fwrite( "\r\n", 2*sizeof(TCHAR), 1, File);
		fflush( File);
	}

	// also print it to the screen and debugger output
	printf(TEXT("%s\n"), Text);
	fflush( stdout );

	if( appIsDebuggerPresent() )
	{
		OutputDebugString( Text );
		OutputDebugString( TEXT("\n") );
		fflush( stderr );
	}
}

/**
 * Flushes all previously written logs.
 */
void FLogInternal::Flush()
{
	FlushFileBuffers( File );
}


/**
 * Accessor for FLog. Safe to call at any time from any thread (even before main or any global constructor).
 * Constructs an FLog upon first use.
 * @return	FLog object
 */
FLogInternal* FLog::operator->()
{
	// First lock the critical section.
	static FCriticalSection CriticalSection;
	FScopeLock Lock( &CriticalSection );

	// Construct the FLogInternal upon first use.
	static FLogInternal LocalLog;

	// Do some initialization upon first use.
	static UBOOL bInitialized = FALSE;
	if ( !bInitialized )
	{
		// Setup a pointer to the FLogInternal, just so that it can be inspected in the debugger.
		Log = &LocalLog;
		Log->Logf(TEXT("Log file created: %s"), Log->GetLogFilename());
		bInitialized = TRUE;
	}
	return Log;
}


/*-----------------------------------------------------------------------------
	String functions.
-----------------------------------------------------------------------------*/

/** 
* Copy a string with length checking. Behavior differs from strncpy in that last character is zeroed. 
*
* @param Dest - destination buffer to copy to
* @param Src - source buffer to copy from
* @param MaxLen - max characters in the buffer (including null-terminator)
* @return pointer to resulting string buffer
*/
TCHAR* appStrncpy( TCHAR* Dest, const TCHAR* Src, INT MaxLen )
{
	check(MaxLen>0);
	// length of string must be strictly < total buffer length so use (MaxLen-1)
	_tcsncpy_s(Dest,MaxLen,Src,MaxLen-1);	
	return Dest;
}

/** 
* Concatenate a string with length checking.
*
* @param Dest - destination buffer to append to
* @param Src - source buffer to copy from
* @param MaxLen - max length of the buffer (including null-terminator)
* @return pointer to resulting string buffer
*/
TCHAR* appStrncat( TCHAR* Dest, const TCHAR* Src, INT MaxLen )
{
	INT Len = appStrlen(Dest);
	TCHAR* NewDest = Dest + Len;
	if( (MaxLen-=Len) > 0 )
	{
		appStrncpy( NewDest, Src, MaxLen );
	}
	return Dest;
}

/** 
 * Finds string in string, case insensitive 
 * @param Str The string to look through
 * @param Find The string to find inside Str
 * @return Position in Str if Find was found, otherwise, NULL
 */
const TCHAR* appStristr(const TCHAR* Str, const TCHAR* Find)
{
	// both strings must be valid
	if( Find == NULL || Str == NULL )
	{
		return NULL;
	}
	// get upper-case first letter of the find string (to reduce the number of full strnicmps)
	TCHAR FindInitial = appToUpper(*Find);
	// get length of find string, and increment past first letter
	INT   Length = appStrlen(Find++) - 1;
	// get the first letter of the search string, and increment past it
	TCHAR StrChar = *Str++;
	// while we aren't at end of string...
	while (StrChar)
	{
		// make sure it's upper-case
		StrChar = appToUpper(StrChar);
		// if it matches the first letter of the find string, do a case-insensitive string compare for the length of the find string
		if (StrChar == FindInitial && !appStrnicmp(Str, Find, Length))
		{
			// if we found the string, then return a pointer to the beginning of it in the search string
			return Str-1;
		}
		// go to next letter
		StrChar = *Str++;
	}

	// if nothing was found, return NULL
	return NULL;
}
TCHAR* appStristr(TCHAR* Str, const TCHAR* Find)
{
	return (TCHAR*)appStristr((const TCHAR*)Str, Find);
}


}


