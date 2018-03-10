/**
 * Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
 */
// UnrealLightmass.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CPUSolver.h"
#include "UnitTest.h"
#include "LightmassSwarm.h"
#include <io.h> // for _findclose, weird, eh?
#include "../Lord/LordExport.h"


namespace Lightmass
{
	char GLordSavePath[512];

/**
 * Compare the output results from 2 lighting results
 *
 * @param Dir1 First directory of mapping file dumps to compare
 * @param Dir2 Seconds directory of mapping file dumps to compare
 */
void CompareLightingResults(const TCHAR* Dir1, const TCHAR* Dir2, FLOAT ErrorThreshold);

/**
 * @return the number of processors in this machine
 */
INT GetNumProcessors()
{
	SYSTEM_INFO SI;
	GetSystemInfo(&SI);
	return SI.dwNumberOfProcessors;
}

DOUBLE GStartupTime = 0.0f;

/**
 * 入口函数
 */
int LightmassMain(int argc, _TCHAR* argv[])
{
#if _DEBUG
//  	while (TRUE)
//  	{
//  		int a = 10;
//  	}
#endif

	GStartupTime = appSeconds();

	// Remember command-line.
	extern TCHAR GCmdLine[16384];
	appStrncpy( GCmdLine, GetCommandLine(), ARRAY_COUNT(GCmdLine) );

#if _WIN64
	const TCHAR* BinaryType = TEXT("WIN64");
#else
	const TCHAR* BinaryType = TEXT("WIN32");
#endif
	debugf( TEXT("Lightmass %s started on: %s. Command-line: %s"), BinaryType, appComputerName(), appCmdLine() );

	// parse commandline options
	UBOOL bRunUnitTest = FALSE;
	UBOOL bDumpTextures = FALSE;
	UBOOL bLordExport	= FALSE;		// 导出Lord使用的光照图格式
	FGuid SceneGuid(0x0123, 0x4567, 0x89AB, 0xCDEF); // default scene guid if none specified
	INT NumThreads = GetNumProcessors(); // default to the number of processors
	UBOOL bCompareFiles = FALSE;
	TCHAR* File1 = NULL;
	TCHAR* File2 = NULL;
	FString SceneFile = NULL;
	FLOAT ErrorThreshold = 0.000001f; // default error tolerance to allow in lighting comparisons

	// Override 'NumThreads' with the environment variable, if it's set.
	{
		TCHAR* SwarmMaxCoresVariable = new TCHAR[32768];
		DWORD StrLength = GetEnvironmentVariable( TEXT("Swarm_MaxCores"), SwarmMaxCoresVariable, 32768 );
		SwarmMaxCoresVariable[StrLength] = 0;
		INT SwarmMaxCores = appAtoi( SwarmMaxCoresVariable );
		if ( SwarmMaxCores >= 1 && SwarmMaxCores < 128 )
		{
			NumThreads = SwarmMaxCores;
		}
		delete [] SwarmMaxCoresVariable;
	}

	for (INT ArgIndex = 1; ArgIndex < argc; ArgIndex++)
	{
		if ((appStricmp(argv[ArgIndex], TEXT("-help")) == 0) || (appStricmp(argv[ArgIndex], TEXT("-?")) == 0))
		{
			debugf(TEXT("Usage:\n  UnrealLightmass\n\t[SceneGuid]\n\t[-debug]\n\t[-unittest]\n\t[-dumptex]\n\t[-numthreads N]\n\t[-compare Dir1 Dir2 [-error N]]"));
			debugf(TEXT(""));
			debugf(TEXT("  SceneGuid : Guid of a scene file. 0x0000012300004567000089AB0000CDEF is the default"));
			debugf(TEXT("  -debug : Processes all mappings in the scene, instead of getting tasks from Swarm Coordinator"));
			debugf(TEXT("  -unittest : Runs a series of validations, then quits"));
			debugf(TEXT("  -dumptex : Outputs .bmp files to the current directory of 2D lightmap/shadowmap results"));
			debugf(TEXT("  -compare : Compares the binary dumps created by UnrealEd to compare UE3 vs LM lighting runs"));
			debugf(TEXT("  -error : Controls the threshold that an error is counted when comparing with -compare"));
			return 0;
		}
		else if (appStricmp(argv[ArgIndex], TEXT("-unittest")) == 0)
		{
			bRunUnitTest = TRUE;
		}
		else if (appStricmp(argv[ArgIndex], TEXT("-dumptex")) == 0)
		{
			bDumpTextures = TRUE;
		}
		else if( appStricmp(argv[ArgIndex], TEXT("-lord"))==0)
		{
			bLordExport = TRUE;
		}
		else if (appStricmp(argv[ArgIndex], TEXT("-debug")) == 0)
		{
			GDebugMode = TRUE;
		}
		else if (appStricmp(argv[ArgIndex], TEXT("-stats")) == 0)
		{
			GReportDetailedStats = TRUE;
		}
		else if (appStricmp(argv[ArgIndex], TEXT("-numthreads")) == 0)
		{
			// use the next parameter as the number of threads (it must exist, or we fail)
			NumThreads = 0;
			if (ArgIndex < argc - 1)
			{
				NumThreads = appAtoi(argv[++ArgIndex]);
			}

			// validate it
			if (NumThreads == 0)
			{
				debugf(TEXT("The number of threads was not specified properly, use \"-numthreads N\""));
				return 1;
			}
		}
		else if (appStricmp(argv[ArgIndex], TEXT("-compare")) == 0)
		{
			bCompareFiles = TRUE;

			if (ArgIndex >= argc - 2)
			{
				debugf(TEXT("-compare requires two directories to compare (-compare Dir1 Dir2)"));
				return 1;
			}
			// cache the files to compare
			File1 = argv[++ArgIndex];
			File2 = argv[++ArgIndex];
		}
		else if( appStricmp( argv[ArgIndex], TEXT("-scene")) == 0)
		{
			// cache the file to bake
			SceneFile = argv[++ArgIndex];
		}
		else if( appStricmp( argv[ArgIndex], TEXT("-savepath"))==0)
		{
			strcpy( GLordSavePath, argv[++ArgIndex]); 
		}
		else if (appStricmp(argv[ArgIndex], TEXT("-error")) == 0)
		{
			// use the next parameter as the number of threads (it must exist, or we fail)
			if (ArgIndex >= argc - 1)
			{
				debugf(TEXT("-error requires an error value following (-error N)"));
				return 1;
			}

			ErrorThreshold = appAtof(argv[++ArgIndex]);
		}
		// look for just a Guid on the commandline
		else if (appStrlen(argv[ArgIndex]) == 32)
		{
			// break up the string into 4 components
			FString Arg(argv[ArgIndex]);

			// we use _tcstoul to import base 16
			SceneGuid.A = _tcstoul(*Arg.Mid(0, 8), NULL, 16);
			SceneGuid.B = _tcstoul(*Arg.Mid(8, 8), NULL, 16);
			SceneGuid.C = _tcstoul(*Arg.Mid(16, 8), NULL, 16);
			SceneGuid.D = _tcstoul(*Arg.Mid(24, 8), NULL, 16);
		}
		else if (appStricmp(argv[ArgIndex], TEXT("-trisperleaf")) == 0)
		{
			// use the next parameter as the maximum number of triangles per leaf for the kdop tree (it must exist, or we fail)
			if (ArgIndex >= argc - 1)
			{
				debugf(TEXT("-error requires an error value following (-trisperleaf N)"));
				return 1;
			}

			GKDOPMaxTrisPerLeaf = appAtoi(argv[++ArgIndex]);
		}
	}

	// if we want to run the unit test, do that, then nothing else
	if (bRunUnitTest)
	{
		// this is an ongoing compiler/runtime test for all templates and whatnot
		TestLightmass();
		return 0;
	}

	if (bCompareFiles)
	{
		CompareLightingResults(File1, File2, ErrorThreshold);
		return 0;
	}
	// Start the static lighting processing
//	debugf( TEXT("+++"));
	debugf( TEXT("Processing scene GUID: %08X%08X%08X%08X with %d threads"), SceneGuid.A, SceneGuid.B, SceneGuid.C, SceneGuid.D, NumThreads );
	INT num = BuildStaticLighting( SceneFile, NumThreads, bDumpTextures);
	// 保存文件
	LORDSaveLightMgr(num);

	// 规定的退出协议消息
	debugf( TEXT("---"));

	return 0;
}

extern void LightmassExit();


/**
 * Compare the output results from 2 lighting results
 *
 * @param Filename1 First mapping dump to compare
 * @param Filename2 First mapping dump to compare
 * @param ErrorThreshold Any error less than this is ignored
 *
 * @return Output information, or empty if no differences
 */
FString CompareLightingFiles(const TCHAR* Filename1, const TCHAR* Filename2, FLOAT ErrorThreshold)
{
	// open the files and verify they exist
	HANDLE File1 = CreateFile(Filename1, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (File1 == INVALID_HANDLE_VALUE)
	{
		return FString::Printf(TEXT("File '%s' does not exist!"), Filename1);
	}

	HANDLE File2 = CreateFile(Filename2, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (File2 == INVALID_HANDLE_VALUE)
	{
		CloseHandle(File1);
		return FString::Printf(TEXT("File '%s' does not exist!"), Filename2);
	}

	// get file sizes
	INT Size1 = GetFileSize(File1, NULL);
	INT Size2 = GetFileSize(File2, NULL);

	// they must match
	if (Size1 != Size2)
	{
		CloseHandle(File1);
		CloseHandle(File2);
		return FString::Printf(TEXT("Files are a different size!"));
	}

	// read in the files
	FLOAT* Buf1 = (FLOAT*)appMalloc(Size1);
	FLOAT* Buf2 = (FLOAT*)appMalloc(Size1);

	unsigned long  BytesRead;
	ReadFile(File1, Buf1, Size1, &BytesRead, NULL);
	ReadFile(File2, Buf2, Size1, &BytesRead, NULL);

	CloseHandle(File1);
	CloseHandle(File2);

	// compute the number of floats in the buffers
	INT NumFloats = Size1 / sizeof(FLOAT);

	DOUBLE TotalError = 0;
	FLOAT BiggestError = 0;
	INT NumErrors = 0;
	// compute error over all matches
	for (INT Index = 0; Index < NumFloats; Index++)
	{
		// get diff between 2 lighting values
		FLOAT Error = Abs(Buf1[Index] - Buf2[Index]);

		// does this error pass our threshold?
		if (Error > ErrorThreshold)
		{
			// add it to the running total
			TotalError += Error;
			NumErrors++;

			// look for biggest
			if (Error > BiggestError)
			{
				BiggestError = Error;
			}
		}
	}

	appFree(Buf1);
	appFree(Buf2);

	// return the output if we had any errors
	if (NumErrors > 0)
	{
		return FString::Printf(TEXT("    Error: %0.6f / %d samples, %0.6f avg / %d errors, %0.6f biggest"), TotalError, NumFloats, NumErrors ? TotalError / NumErrors : 0, NumErrors, BiggestError);
	}

	// otherwise, just an empty string
	return TEXT("");
}


/**
 * Compare the output results from 2 lighting results
 *
 * @param Dir1 First directory of mapping file dumps to compare
 * @param Dir2 Seconds directory of mapping file dumps to compare
 * @param ErrorThreshold Any error less than this is ignored
 */
void CompareLightingResults(const TCHAR* Dir1, const TCHAR* Dir2, FLOAT ErrorThreshold)
{
	// go through each file in the first directory
	_finddata_t FindData;

	INT NumDifferentFiles = 0;
	INT TotalFiles = 0;

	debugf(TEXT(""));
	debugf(TEXT("Comparing '%s' vs '%s'"), Dir1, Dir2);

	// ramp up the find files
	intptr_t Find = _findfirst(*FString::Printf(TEXT("%s\\*.bin"), Dir1), &FindData);
	if (Find != -1L)
	{
		do
		{
			// do the comparison
			FString Output = CompareLightingFiles(
								*FString::Printf(TEXT("%s\\%s"), Dir1, FindData.name),
								*FString::Printf(TEXT("%s\\%s"), Dir2, FindData.name),
								ErrorThreshold);

			TotalFiles++;
			// if there was any interesting output, show it
			if (Output != TEXT(""))
			{
				debugf(TEXT("\n  %s:\n%s"), FindData.name, *Output);

				NumDifferentFiles++;
			}

		} 
		while (_findnext(Find, &FindData) == 0);
		_findclose(Find);
	}

	debugf(TEXT("\nFound %d issues (out of %d mappings)..."), NumDifferentFiles, TotalFiles);
}

extern INT WinCreateMiniDump( LPEXCEPTION_POINTERS ExceptionInfo );

void CriticalErrorCallback()
{
	// Try to notify Swarm about the critical error.
	const FString& CrashReporterURL = appGetCrashReporterURL();
	if( TRUE)
	{
		debugf( TEXT("*** CRITICAL ERROR! Machine: %s"), appComputerName() );
		debugf( TEXT("*** CRITICAL ERROR! Logfile: %s"), GLog->GetLogFilename() );
		debugf( TEXT("*** CRITICAL ERROR! Crash report: %s"), *CrashReporterURL );
		debugf( GLog->GetLogFilename() );
	}
	else
	{
		debugf( TEXT("--- Critical Error! Machine: %s. Logfile: %s. Crash report: %s. ---"), appComputerName(), GLog->GetLogFilename(), *CrashReporterURL );
	}
}

/**
 * Verifies that the correct version of DirectX is installed.
 * @return	TRUE if everything looks correct
 */
UBOOL VerifyD3D()
{
// 	UBOOL bD3DInstalledCorrectly = FALSE;
// 	IDirect3D9* D3D = NULL;
// 	__try
// 	{
// 		D3D = Direct3DCreate9(D3D_SDK_VERSION);
// 		bD3DInstalledCorrectly = (D3D != NULL) && D3DXCheckVersion(D3D_SDK_VERSION, D3DX_SDK_VERSION);
// 	}
// 	__except( EXCEPTION_EXECUTE_HANDLER )
// 	{
// 		bD3DInstalledCorrectly = FALSE;
// 	}
// 	if ( !bD3DInstalledCorrectly )
// 	{
// 		debugf( TEXT("DirectX run-time isn't installed or it's using the incorrect version!\nLightmass requires D3D_SDK_VERSION %d and D3DX_SDK_VERSION %d."), D3D_SDK_VERSION, D3DX_SDK_VERSION );
// 		return FALSE;
// 	}
// 	D3D->Release();
	return TRUE;
}

UBOOL VerifyDLL( const TCHAR* DLLFilename )
{
	HMODULE DbgHelpDll = LoadLibrary( DLLFilename );
	if ( DbgHelpDll == NULL )
	{
		debugf( TEXT("Failed to load %s!"), DLLFilename );
		return FALSE;
	}
	return TRUE;
}

}


int _tmain(int argc, _TCHAR* argv[])
{
	Lightmass::GStatistics.TotalTimeStart = Lightmass::appSeconds();

	INT ErrorLevel = 0;

	// Set the error mode to avoid popping up dialog boxes on crashes
	SetErrorMode( SEM_NOGPFAULTERRORBOX | SEM_NOGPFAULTERRORBOX );

	Lightmass::appSetCriticalErrorCallback( Lightmass::CriticalErrorCallback );

	// Verify the installed DirectX run-time and other required DLLs
	if ( !Lightmass::VerifyD3D() ||
		 !Lightmass::VerifyDLL(TEXT("dbghelp.dll")) )
	{
		return 1;
	}

#ifdef _DEBUG
 	if ( TRUE )
#else
	if ( appIsDebuggerPresent() )
#endif
	{
		// Don't use exception handling when a debugger is attached to exactly trap the crash.
		ErrorLevel = Lightmass::LightmassMain(argc, argv);
		Lightmass::LightmassExit();
	}
	else
	{
		// Use structured exception handling to trap any crashes, walk the the stack and display a crash dialog box.
		__try
		{
			__try
			{
				__try
				{
					Lightmass::GIsGuarded = TRUE;
					// Run the guarded code.
					ErrorLevel = Lightmass::LightmassMain(argc, argv);
					Lightmass::LightmassExit();
					Lightmass::GIsGuarded = FALSE;
				}
				__except( Lightmass::WinCreateMiniDump( GetExceptionInformation() ) )
				{
					printf( "Exception handled in main, crash report generated, re-throwing exception\n" );

					// With the crash report created, propagate the error
					throw( 1 );
				}
			}
			__except( EXCEPTION_EXECUTE_HANDLER )
			{
				printf( "Exception handled in main, calling appHandleCriticalError\n" );

				// Crashed
				ErrorLevel = 1;
				Lightmass::appHandleCriticalError();
			}
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
			// Do nothing except prevent the crash
			printf( "Exception handled in main, attempting to prevent application crash\n" );
		}
	}

	Lightmass::GStatistics.TotalTimeEnd = Lightmass::appSeconds();
	return ErrorLevel;
}
