/*=============================================================================
	LMMath.h: Some implementation of LM math functionality
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "stdafx.h"
#include "LMCore.h"
#include "LMDebug.h"

#pragma pack(push,8)
#include <DbgHelp.h>
#pragma pack(pop)


namespace Lightmass
{

typedef BOOL  (WINAPI *TFEnumProcesses)( ::DWORD * lpidProcess, ::DWORD cb, ::DWORD * cbNeeded);
typedef BOOL  (WINAPI *TFEnumProcessModules)(HANDLE hProcess, HMODULE *lphModule, ::DWORD cb, LPDWORD lpcbNeeded);
typedef ::DWORD (WINAPI *TFGetModuleBaseName)(HANDLE hProcess, HMODULE hModule, LPSTR lpBaseName, ::DWORD nSize);
typedef ::DWORD (WINAPI *TFGetModuleFileNameEx)(HANDLE hProcess, HMODULE hModule, LPSTR lpFilename, ::DWORD nSize);
typedef BOOL  (WINAPI *TFGetModuleInformation)(HANDLE hProcess, HMODULE hModule, LPMODULEINFO lpmodinfo, ::DWORD cb);

static TFEnumProcesses			FEnumProcesses;
static TFEnumProcessModules		FEnumProcessModules;
static TFGetModuleBaseName		FGetModuleBaseName;
static TFGetModuleFileNameEx	FGetModuleFileNameEx;
static TFGetModuleInformation	FGetModuleInformation;

/** Latest error message. */
TCHAR GErrorHist[16384] = TEXT("");

FString InstigatorUserName;

void* appGetDllHandle( const TCHAR* Filename )
{
	check(Filename);	
	return LoadLibrary(Filename);
}

//
// Free a DLL.
//
void appFreeDllHandle( void* DllHandle )
{
	check(DllHandle);
	FreeLibrary( (HMODULE)DllHandle );
}

//
// Lookup the address of a DLL function.
//
void* appGetDllExport( void* DllHandle, const ANSICHAR* ProcName )
{
	check(DllHandle);
	check(ProcName);
	return (void*)GetProcAddress( (HMODULE)DllHandle, ProcName );
}



/**
 * Loads modules for current process.
 */ 
static void LoadProcessModules()
{
	INT			ErrorCode = 0;	
	HANDLE		ProcessHandle = GetCurrentProcess(); 
	const INT	MAX_MOD_HANDLES = 1024;
	HMODULE		ModuleHandleArray[MAX_MOD_HANDLES];
	HMODULE*	ModuleHandlePointer = ModuleHandleArray;
	unsigned long 		BytesRequired;
	MODULEINFO	ModuleInfo;

	// Enumerate process modules.
	UBOOL bEnumProcessModulesSucceeded = FEnumProcessModules( ProcessHandle, ModuleHandleArray, sizeof(ModuleHandleArray), &BytesRequired );
	if( !bEnumProcessModulesSucceeded )
	{
		ErrorCode = GetLastError();
		return;
	}

	// Static array isn't sufficient so we dynamically allocate one.
	UBOOL bNeedToFreeModuleHandlePointer = FALSE;
	if( BytesRequired > sizeof( ModuleHandleArray ) )
	{
		// Keep track of the fact that we need to free it again.
		bNeedToFreeModuleHandlePointer = TRUE;
		ModuleHandlePointer = (HMODULE*) appMalloc( BytesRequired );
		FEnumProcessModules( ProcessHandle, ModuleHandlePointer, sizeof(ModuleHandleArray), &BytesRequired );
	}

	// Find out how many modules we need to load modules for.
	INT	ModuleCount = BytesRequired / sizeof( HMODULE );

	// Load the modules.
	for( INT ModuleIndex=0; ModuleIndex<ModuleCount; ModuleIndex++ )
	{
		ANSICHAR ModuleName[1024];
		ANSICHAR ImageName[1024];
		FGetModuleInformation( ProcessHandle, ModuleHandleArray[ModuleIndex], &ModuleInfo,sizeof( ModuleInfo ) );
		FGetModuleFileNameEx( ProcessHandle, ModuleHandleArray[ModuleIndex], ImageName, 1024 );
		FGetModuleBaseName( ProcessHandle, ModuleHandleArray[ModuleIndex], ModuleName, 1024 );

		// Load module.
		if( !SymLoadModule64( ProcessHandle, ModuleHandleArray[ModuleIndex], ImageName, ModuleName, (DWORD64) ModuleInfo.lpBaseOfDll, (DWORD) ModuleInfo.SizeOfImage ) )
		{
			ErrorCode = GetLastError();
		}
	} 

	// Free the module handle pointer allocated in case the static array was insufficient.
	if( bNeedToFreeModuleHandlePointer )
	{
		appFree( ModuleHandlePointer );
	}
}

/**
 * Returns the number of modules loaded by the currently running process.
 */
INT appGetProcessModuleCount()
{
	appInitStackWalking();

	HANDLE		ProcessHandle = GetCurrentProcess(); 
	const INT	MAX_MOD_HANDLES = 1024;
	HMODULE		ModuleHandleArray[MAX_MOD_HANDLES];
	HMODULE*	ModuleHandlePointer = ModuleHandleArray;
	unsigned long 		BytesRequired;

	// Enumerate process modules.
	UBOOL bEnumProcessModulesSucceeded = FEnumProcessModules( ProcessHandle, ModuleHandleArray, sizeof(ModuleHandleArray), &BytesRequired );
	if( !bEnumProcessModulesSucceeded )
	{
		return 0;
	}

	// Static array isn't sufficient so we dynamically allocate one.
	UBOOL bNeedToFreeModuleHandlePointer = FALSE;
	if( BytesRequired > sizeof( ModuleHandleArray ) )
	{
		// Keep track of the fact that we need to free it again.
		bNeedToFreeModuleHandlePointer = TRUE;
		ModuleHandlePointer = (HMODULE*) appMalloc( BytesRequired );
		FEnumProcessModules( ProcessHandle, ModuleHandlePointer, sizeof(ModuleHandleArray), &BytesRequired );
	}

	// Find out how many modules we need to load modules for.
	INT	ModuleCount = BytesRequired / sizeof( HMODULE );

	// Free the module handle pointer allocated in case the static array was insufficient.
	if( bNeedToFreeModuleHandlePointer )
	{
		appFree( ModuleHandlePointer );
	}

	return ModuleCount;
}

/**
 * Gets the signature for every module loaded by the currently running process.
 *
 * @param	ModuleSignatures		An array to retrieve the module signatures.
 * @param	ModuleSignaturesSize	The size of the array pointed to by ModuleSignatures.
 *
 * @return	The number of modules copied into ModuleSignatures
 */
INT appGetProcessModuleSignatures(FModuleInfo *ModuleSignatures, const INT ModuleSignaturesSize)
{
	appInitStackWalking();

	HANDLE		ProcessHandle = GetCurrentProcess(); 
	const INT	MAX_MOD_HANDLES = 1024;
	HMODULE		ModuleHandleArray[MAX_MOD_HANDLES];
	HMODULE*	ModuleHandlePointer = ModuleHandleArray;
	unsigned long 		BytesRequired;
	MODULEINFO	ModuleInfo;

	// Enumerate process modules.
	UBOOL bEnumProcessModulesSucceeded = FEnumProcessModules( ProcessHandle, ModuleHandleArray, sizeof(ModuleHandleArray), &BytesRequired );
	if( !bEnumProcessModulesSucceeded )
	{
		return 0;
	}

	// Static array isn't sufficient so we dynamically allocate one.
	UBOOL bNeedToFreeModuleHandlePointer = FALSE;
	if( BytesRequired > sizeof( ModuleHandleArray ) )
	{
		// Keep track of the fact that we need to free it again.
		bNeedToFreeModuleHandlePointer = TRUE;
		ModuleHandlePointer = (HMODULE*) appMalloc( BytesRequired );
		FEnumProcessModules( ProcessHandle, ModuleHandlePointer, sizeof(ModuleHandleArray), &BytesRequired );
	}

	// Find out how many modules we need to load modules for.
	INT	ModuleCount = BytesRequired / sizeof( HMODULE );
	IMAGEHLP_MODULE64 Img;
	Img.SizeOfStruct = sizeof(Img);

	INT SignatureIndex = 0;

	// Load the modules.
	for( INT ModuleIndex = 0; ModuleIndex < ModuleCount && SignatureIndex < ModuleSignaturesSize; ModuleIndex++ )
	{
		ANSICHAR ModuleName[1024];
		ANSICHAR ImageName[1024];
		FGetModuleInformation( ProcessHandle, ModuleHandleArray[ModuleIndex], &ModuleInfo,sizeof( ModuleInfo ) );
		FGetModuleFileNameEx( ProcessHandle, ModuleHandleArray[ModuleIndex], ImageName, 1024 );
		FGetModuleBaseName( ProcessHandle, ModuleHandleArray[ModuleIndex], ModuleName, 1024 );

		// Load module.
		if(SymGetModuleInfo64(ProcessHandle, (DWORD64)ModuleInfo.lpBaseOfDll, &Img))
		{
			FModuleInfo Info;
			Info.BaseOfImage = Img.BaseOfImage;
			appStrcpy(Info.ImageName, Img.ImageName);
			Info.ImageSize = Img.ImageSize;
			appStrcpy(Info.LoadedImageName, Img.LoadedImageName);
			appStrcpy(Info.ModuleName, Img.ModuleName);
			Info.PdbAge = Img.PdbAge;
			Info.PdbSig = Img.PdbSig;
			Info.PdbSig70 = Img.PdbSig70;
			Info.TimeDateStamp = Img.TimeDateStamp;

			ModuleSignatures[SignatureIndex] = Info;
			++SignatureIndex;
		}
	}

	// Free the module handle pointer allocated in case the static array was insufficient.
	if( bNeedToFreeModuleHandlePointer )
	{
		appFree( ModuleHandlePointer );
	}

	return SignatureIndex;
}

/**
 * Initializes the symbol engine if needed.
 */ 
UBOOL appInitStackWalking()
{
	static UBOOL SymEngInitialized = FALSE;
	
	// Only initialize once.
	if( !SymEngInitialized )
	{
		void* DllHandle = appGetDllHandle(TEXT("PSAPI.DLL"));
		if( DllHandle == NULL )
		{
			return FALSE;
		}

		// Load dynamically linked PSAPI routines.
		FEnumProcesses			= (TFEnumProcesses)			appGetDllExport( DllHandle,"EnumProcesses");
		FEnumProcessModules		= (TFEnumProcessModules)	appGetDllExport( DllHandle,"EnumProcessModules");
		FGetModuleFileNameEx	= (TFGetModuleFileNameEx)	appGetDllExport( DllHandle,"GetModuleFileNameExA");
		FGetModuleBaseName		= (TFGetModuleBaseName)		appGetDllExport( DllHandle,"GetModuleBaseNameA");
		FGetModuleInformation	= (TFGetModuleInformation)	appGetDllExport( DllHandle,"GetModuleInformation");

		// Abort if we can't look up the functions.
		if( !FEnumProcesses || !FEnumProcessModules || !FGetModuleFileNameEx || !FGetModuleBaseName || !FGetModuleInformation )
		{
			return FALSE;
		}

		// Set up the symbol engine.
		DWORD SymOpts = SymGetOptions();
		SymOpts |= SYMOPT_LOAD_LINES ;
		SymOpts |= SYMOPT_DEBUG;
		SymSetOptions ( SymOpts );

		// Initialize the symbol engine.
#ifdef _DEBUG
		SymInitialize ( GetCurrentProcess(), "Lib/Debug;.", TRUE );
#else
		SymInitialize ( GetCurrentProcess(), "Lib/Release;.", TRUE );
#endif
		LoadProcessModules();

		SymEngInitialized = TRUE;
	}
	return SymEngInitialized;
}

/**
 * Helper function performing the actual stack walk. This code relies on the symbols being loaded for best results
 * walking the stack albeit at a significant performance penalty.
 *
 * This helper function is designed to be called within a structured exception handler.
 *
 * @param	BackTrace			Array to write backtrace to
 * @param	MaxDepth			Maxium depth to walk - needs to be less than or equal to array size
 * @param	Context				Thread context information
 * @return	EXCEPTION_EXECUTE_HANDLER
 */
static INT CaptureStackTraceHelper( QWORD *BackTrace, DWORD MaxDepth, CONTEXT* Context )
{
	STACKFRAME64		StackFrame64;
	HANDLE				ProcessHandle;
	HANDLE				ThreadHandle;
	unsigned long		LastError;
	UBOOL				bStackWalkSucceeded	= TRUE;
	DWORD				CurrentDepth		= 0;
	DWORD				MachineType			= IMAGE_FILE_MACHINE_I386;

	__try
	{
		// Get context, process and thread information.
		ProcessHandle	= GetCurrentProcess();
		ThreadHandle	= GetCurrentThread();

		// Zero out stack frame.
		memset( &StackFrame64, 0, sizeof(StackFrame64) );

		// Initialize the STACKFRAME structure.
		StackFrame64.AddrPC.Mode         = AddrModeFlat;
		StackFrame64.AddrStack.Mode      = AddrModeFlat;
		StackFrame64.AddrFrame.Mode      = AddrModeFlat;
#ifdef _WIN64
		StackFrame64.AddrPC.Offset       = Context->Rip;
		StackFrame64.AddrStack.Offset    = Context->Rsp;
		StackFrame64.AddrFrame.Offset    = Context->Rbp;
		MachineType                      = IMAGE_FILE_MACHINE_AMD64;
#else
		StackFrame64.AddrPC.Offset       = Context->Eip;
		StackFrame64.AddrStack.Offset    = Context->Esp;
		StackFrame64.AddrFrame.Offset    = Context->Ebp;
#endif

		// Walk the stack one frame at a time.
		while( bStackWalkSucceeded && (CurrentDepth < MaxDepth) )
		{
			bStackWalkSucceeded = StackWalk64(  MachineType, 
												ProcessHandle, 
												ThreadHandle, 
												&StackFrame64,
												Context,
												NULL,
												SymFunctionTableAccess64,
												SymGetModuleBase64,
												NULL );

			BackTrace[CurrentDepth++] = StackFrame64.AddrPC.Offset;

			if( !bStackWalkSucceeded  )
			{
				// StackWalk failed! give up.
				LastError = GetLastError( );
				break;
			}

			if( StackFrame64.AddrFrame.Offset == 0 )
			{
				// This frame offset is not valid.
				break;
			}
		}
		
	} 
	__except ( EXCEPTION_EXECUTE_HANDLER )
	{
		// We need to catch any execptions within this function so they dont get sent to 
		// the engine's error handler, hence causing an infinite loop.
		return EXCEPTION_EXECUTE_HANDLER;
	} 

	// NULL out remaining entries.
	for ( ; CurrentDepth<MaxDepth; CurrentDepth++ )
	{
		BackTrace[CurrentDepth] = NULL;
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

#pragma warning(push)
#pragma warning(disable:4740)	// warning C4740: flow in or out of inline asm code suppresses global optimization

/**
 * Capture a stack backtrace and optionally use the passed in exception pointers.
 *
 * @param	BackTrace			[out] Pointer to array to take backtrace
 * @param	MaxDepth			Entries in BackTrace array
 * @param	Context				Optional thread context information
 */
void appCaptureStackBackTrace( QWORD* BackTrace, DWORD MaxDepth, CONTEXT* Context )
{
	// Make sure we have place to store the information before we go through the process of raising
	// an exception and handling it.
	if( BackTrace == NULL || MaxDepth == 0 )
	{
		return;
	}

	if( Context )
	{
		CaptureStackTraceHelper( BackTrace, MaxDepth, Context );
	}
	else
	{
#ifdef _WIN64
		CONTEXT HelperContext;
		RtlCaptureContext( &HelperContext );

		// Capture the back trace.
		CaptureStackTraceHelper( BackTrace, MaxDepth, &HelperContext );
#else
		// Use a bit of inline assembly to capture the information relevant to stack walking which is
		// basically EIP and EBP.
		CONTEXT HelperContext;
		memset( &HelperContext, 0, sizeof(CONTEXT) );
		HelperContext.ContextFlags = CONTEXT_FULL;

		// Use a fake function call to pop the return address and retrieve EIP.
		__asm
		{
			call FakeFunctionCall
		FakeFunctionCall: 
			pop eax
			mov HelperContext.Eip, eax
			mov HelperContext.Ebp, ebp
			mov HelperContext.Esp, esp
		}

		// Capture the back trace.
		CaptureStackTraceHelper( BackTrace, MaxDepth, &HelperContext );
#endif
	}
}

#pragma warning(pop)


/**
 * Converts the passed in program counter address to a human readable string and appends it to the passed in one.
 * @warning: The code assumes that HumanReadableString is large enough to contain the information.
 *
 * @param	ProgramCounter			Address to look symbol information up for
 * @param	HumanReadableString		String to concatenate information with
 * @param	HumanReadableStringSize size of string in characters
 * @param	VerbosityFlags			Bit field of requested data for output.
 */ 
void appProgramCounterToHumanReadableString( QWORD ProgramCounter, ANSICHAR* HumanReadableString, SIZE_T HumanReadableStringSize, EVerbosityFlags VerbosityFlags /*= VF_DISPLAY_ALL*/ )
{
	ANSICHAR			SymbolBuffer[sizeof(IMAGEHLP_SYMBOL64) + 512];
	PIMAGEHLP_SYMBOL64	Symbol;
	unsigned long 				SymbolDisplacement		= 0;
	DWORD64				SymbolDisplacement64	= 0;
	DWORD				LastError;
	
	HANDLE				ProcessHandle = GetCurrentProcess();

	// Initialize stack walking as it loads up symbol information which we require.
	appInitStackWalking();

	// Initialize symbol.
	Symbol					= (PIMAGEHLP_SYMBOL64) SymbolBuffer;
	Symbol->SizeOfStruct	= sizeof(SymbolBuffer);
	Symbol->MaxNameLength	= 512;

	// Get symbol from address.
	if( SymGetSymFromAddr64( ProcessHandle, ProgramCounter, &SymbolDisplacement64, Symbol ) )
	{
		ANSICHAR			FunctionName[1024];

		// Skip any funky chars in the beginning of a function name.
		INT Offset = 0;
		while( Symbol->Name[Offset] < 32 || Symbol->Name[Offset] > 127 )
		{
			Offset++;
		}

		// Write out function name if there is sufficient space.
		sprintf( FunctionName,  ("%s() "), (const ANSICHAR*)(Symbol->Name + Offset) );
		strcat_s( HumanReadableString, HumanReadableStringSize, FunctionName );
	}
	else
	{
		// No symbol found for this address.
		LastError = GetLastError( );
	}

	if( VerbosityFlags & VF_DISPLAY_FILENAME )
	{
		IMAGEHLP_LINE64		ImageHelpLine;
		ANSICHAR			FileNameLine[1024];

		// Get Line from address
		ImageHelpLine.SizeOfStruct = sizeof( ImageHelpLine );
		if( SymGetLineFromAddr64( ProcessHandle, ProgramCounter, &SymbolDisplacement, &ImageHelpLine) )
		{
			sprintf( FileNameLine, ("0x%-8x + %d bytes [File=%s:%d] "), (DWORD) ProgramCounter, SymbolDisplacement, (const ANSICHAR*)(ImageHelpLine.FileName), ImageHelpLine.LineNumber );
		}
		else    
		{
			// No line number found.  Print out the logical address instead.
			sprintf( FileNameLine, "Address = 0x%-8x (filename not found) ", (DWORD) ProgramCounter );
		}
		strcat_s( HumanReadableString, HumanReadableStringSize, FileNameLine );
	}

	if( VerbosityFlags & VF_DISPLAY_MODULE )
	{
		IMAGEHLP_MODULE64	ImageHelpModule;
		ANSICHAR			ModuleName[1024];

		// Get module information from address.
		ImageHelpModule.SizeOfStruct = sizeof( ImageHelpModule );
		if( SymGetModuleInfo64( ProcessHandle, ProgramCounter, &ImageHelpModule) )
		{
			// Write out Module information if there is sufficient space.
			sprintf( ModuleName, "[in %s]", (const ANSICHAR*)(ImageHelpModule.ImageName) );
			strcat_s( HumanReadableString, HumanReadableStringSize, ModuleName );
		}
		else
		{
			LastError = GetLastError( );
		}
	}
}

//#pragma ENABLE_OPTIMIZATION

/**
 * Converts the passed in program counter address to a symbol info struct, filling in module and filename, line number and displacement.
 * @warning: The code assumes that the destination strings are big enough
 *
 * @param	ProgramCounter			Address to look symbol information up for
 * @return	symbol information associated with program counter
 */
FProgramCounterSymbolInfo appProgramCounterToSymbolInfo( QWORD ProgramCounter )
{
	// Create zeroed out return value.
	FProgramCounterSymbolInfo	SymbolInfo;
	appMemzero( &SymbolInfo, sizeof(SymbolInfo) );

	ANSICHAR			SymbolBuffer[sizeof(IMAGEHLP_SYMBOL64) + 512];
	PIMAGEHLP_SYMBOL64	Symbol;
	unsigned long 				SymbolDisplacement		= 0;
	DWORD64				SymbolDisplacement64	= 0;
	DWORD				LastError;
	
	HANDLE				ProcessHandle = GetCurrentProcess();

	// Initialize stack walking as it loads up symbol information which we require.
	appInitStackWalking();

	// Initialize symbol.
	Symbol					= (PIMAGEHLP_SYMBOL64) SymbolBuffer;
	Symbol->SizeOfStruct	= sizeof(SymbolBuffer);
	Symbol->MaxNameLength	= 512;

	// Get symbol from address.
	if( SymGetSymFromAddr64( ProcessHandle, ProgramCounter, &SymbolDisplacement64, Symbol ) )
	{
		// Skip any funky chars in the beginning of a function name.
		INT Offset = 0;
		while( Symbol->Name[Offset] < 32 || Symbol->Name[Offset] > 127 )
		{
			Offset++;
		}

		// Write out function name.
		appStrcpyANSI( SymbolInfo.FunctionName, Symbol->Name + Offset );
	}
	else
	{
		// No symbol found for this address.
		LastError = GetLastError( );
	}

	// Get Line from address
	IMAGEHLP_LINE64	ImageHelpLine;
	ImageHelpLine.SizeOfStruct = sizeof( ImageHelpLine );
	if( SymGetLineFromAddr64( ProcessHandle, ProgramCounter, &SymbolDisplacement, &ImageHelpLine) )
	{
		appStrcpyANSI( SymbolInfo.Filename, ImageHelpLine.FileName );
		SymbolInfo.LineNumber			= ImageHelpLine.LineNumber;
		SymbolInfo.SymbolDisplacement	= SymbolDisplacement;
	}
	else    
	{
		// No line number found.
		appStrcatANSI( SymbolInfo.Filename, "Unknown" );
		SymbolInfo.LineNumber			= 0;
		SymbolDisplacement				= 0;
	}

	// Get module information from address.
	IMAGEHLP_MODULE64 ImageHelpModule;
	ImageHelpModule.SizeOfStruct = sizeof( ImageHelpModule );
	if( SymGetModuleInfo64( ProcessHandle, ProgramCounter, &ImageHelpModule) )
	{
		// Write out Module information.
		appStrcpyANSI( SymbolInfo.ModuleName, ImageHelpModule.ImageName );
	}
	else
	{
		LastError = GetLastError( );
	}

	return SymbolInfo;
}

/**
 * Walks the stack and appends the human readable string to the passed in one.
 * @warning: The code assumes that HumanReadableString is large enough to contain the information.
 *
 * @param	HumanReadableString	String to concatenate information with
 * @param	HumanReadableStringSize size of string in characters
 * @param	IgnoreCount			Number of stack entries to ignore (some are guaranteed to be in the stack walking code)
 * @param	Context				Optional thread context information
 */ 
void appStackWalkAndDump( ANSICHAR* HumanReadableString, SIZE_T HumanReadableStringSize, INT IgnoreCount, CONTEXT* Context = NULL )
{	
	// Initialize stack walking... loads up symbol information.
	appInitStackWalking();

	// Temporary memory holding the stack trace.
	#define MAX_DEPTH 100
	DWORD64 StackTrace[MAX_DEPTH];
	memset( StackTrace, 0, sizeof(StackTrace) );

	// Capture stack backtrace.
	appCaptureStackBackTrace( StackTrace, MAX_DEPTH, Context );

	// Skip the first two entries as they are inside the stack walking code.
	INT CurrentDepth = IgnoreCount;
	while( StackTrace[CurrentDepth] )
	{
		appProgramCounterToHumanReadableString( StackTrace[CurrentDepth], HumanReadableString, HumanReadableStringSize );
		strcat_s( HumanReadableString, HumanReadableStringSize, "\r\n" );
		CurrentDepth++;
	}
}

//
// Failed assertion handler.
//warning: May be called at library startup time.
//
void VARARGS appFailAssertFunc( const ANSICHAR* Expr, const ANSICHAR* File, INT Line, const TCHAR* Format/*=TEXT("")*/, ... )
{
	TCHAR TempStr[4096];
	GET_VARARGS( TempStr, ARRAY_COUNT(TempStr), ARRAY_COUNT(TempStr)-1, Format, Format );

	// Note: If debugger is present, appFailAssertFuncDebug() has already printed the message.
	if ( appIsDebuggerPresent() == FALSE )
	{
		// If we're running guarded, the callstack will be printed by the exception handler.
		if ( GIsGuarded )
		{
			appCriticalError( TEXT("Assertion failed: %s [File:%s] [Line: %i]\n%s"), ANSI_TO_TCHAR(Expr), ANSI_TO_TCHAR(File), Line, TempStr );
		}
		else
		{
			const SIZE_T StackTraceSize = 65535;
			ANSICHAR* StackTrace = (ANSICHAR*) appSystemMalloc( StackTraceSize );
			StackTrace[0] = 0;
			// Walk the stack and dump it to the allocated memory.
			appStackWalkAndDump( StackTrace, StackTraceSize, 3 );

			appCriticalError( TEXT("Assertion failed: %s [File:%s] [Line: %i]\n%s\nStack:\n%s"), ANSI_TO_TCHAR(Expr), ANSI_TO_TCHAR(File), Line, TempStr, ANSI_TO_TCHAR(StackTrace) );

			appSystemFree( StackTrace );
		}
	}
}


//
// Failed assertion handler.  This version only calls appOutputDebugString.
//
void VARARGS appFailAssertFuncDebug( const ANSICHAR* Expr, const ANSICHAR* File, INT Line, const TCHAR* Format/*=TEXT("")*/, ... )
{
	TCHAR TempStr[4096];
	GET_VARARGS( TempStr, ARRAY_COUNT(TempStr), ARRAY_COUNT(TempStr)-1, Format, Format );
	debugf( TEXT("%s(%i): Assertion failed: %s\n%s\n"), ANSI_TO_TCHAR(File), Line, ANSI_TO_TCHAR(Expr), TempStr );
}

/**
 * Sets GErrorHist, outputs it to the log and debugger Output Window,
 * and stops the program execution.
 */
VARARG_BODY( void, appCriticalError, const TCHAR*, VARARG_NONE )
{
	GET_VARARGS( GErrorHist, ARRAY_COUNT(GErrorHist), ARRAY_COUNT(GErrorHist)-1, Fmt, Fmt );
	debugf( GErrorHist );

	if( GIsGuarded )
	{
		// Propagate error so structured exception handler can perform necessary work.
		throw( 1 );
	}
	else
	{
		// We crashed outside the guarded code (e.g. appExit).
		appHandleCriticalError();
		exit( 1 );
	}
}


/** Critical section to block other threads until a mini-dump has been created. */
static FCriticalSection GMiniDumpCriticalSection;

/** Filename of the minidump file. */
TCHAR GMiniDumpFilenameW[1024] = TEXT("");

/**
 * Creates a mini-dump file in the current folder on the hard disk.
 * Thread-safe (blocking). Only the first call will create the mini-dump.
 */
INT WinCreateMiniDump( LPEXCEPTION_POINTERS ExceptionInfo )
{
	// Block other threads until a mini-dump has been created.
	FScopeLock Lock( &GMiniDumpCriticalSection );

	// Only create a mini-dump the first time WinCreateMiniDump() is called.
	static INT bAlreadyCreatedMiniDump = FALSE;
	if ( bAlreadyCreatedMiniDump == FALSE )
	{
		bAlreadyCreatedMiniDump = TRUE;

		static TCHAR ExeName[MAX_PATH] = TEXT("");

		GetModuleFileName(NULL, ExeName, MAX_PATH);
		INT ExeNameLen = appStrlen( ExeName );
		INT PathSeparatorPos = ExeNameLen;
		while ( PathSeparatorPos >= 0 && ExeName[PathSeparatorPos] != TEXT('\\') )
		{
			PathSeparatorPos--;
		}
		appStrcpy( GMiniDumpFilenameW, *FString::Printf( TEXT("%s-%s.dmp"), ExeName + PathSeparatorPos + 1, *appSystemTimeString() ) );

		// Try to create file for minidump.
		HANDLE FileHandle	= CreateFile( GMiniDumpFilenameW, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

		// Write a minidump.
		if( FileHandle != INVALID_HANDLE_VALUE )
		{
			MINIDUMP_EXCEPTION_INFORMATION DumpExceptionInfo;

			DumpExceptionInfo.ThreadId			= GetCurrentThreadId();
			DumpExceptionInfo.ExceptionPointers	= ExceptionInfo;
			DumpExceptionInfo.ClientPointers	= true;

			MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), FileHandle, MiniDumpNormal, &DumpExceptionInfo, NULL, NULL );
			CloseHandle( FileHandle );
		}

		const SIZE_T StackTraceSize = 65535;
		ANSICHAR* StackTrace = (ANSICHAR*) appSystemMalloc( StackTraceSize );
		StackTrace[0] = 0;
		// Walk the stack and dump it to the allocated memory.
		appStackWalkAndDump( StackTrace, StackTraceSize, 0, ExceptionInfo->ContextRecord );
		appStrncat( GErrorHist, ANSI_TO_TCHAR(StackTrace), ARRAY_COUNT(GErrorHist) - 1 );
		appSystemFree( StackTrace );
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

/** User-specified callback function that will be called after a critical error has been detected and handled. */
static void (*GCriticalErrorCallback)() = NULL;

/**
 * Sets a callback function that will be called after a critical error has been detected and handled.
 * @param CriticalErrorCallback	User-specified callback function
 */
void appSetCriticalErrorCallback( void (*CriticalErrorCallback)() )
{
	GCriticalErrorCallback = CriticalErrorCallback;
}

/** Returns the latest error message. */
const TCHAR* appGetError()
{
	return GErrorHist;
}

/** Crash reporter URL, as set by AutoReporter.exe after being launched by appHandleCriticalError(). */
static FString GCrashReporterURL;

/**
 * Returns the crash reporter URL after appHandleCriticalError() has been called.
 */
const FString& appGetCrashReporterURL()
{
	return GCrashReporterURL;
}

/** Critical section to block other threads until a mini-dump has been created. */
static FCriticalSection GCriticalErrorCriticalSection;

/**
 * Handles critical error. The call only performs anything on the first call.
 */
void appHandleCriticalError()
{
	// Block other threads until the critical error has been handled.
	FScopeLock Lock( &GMiniDumpCriticalSection );

	/** Only handle the first critical error. */
	static INT bAlreadyHandledCriticalError = FALSE;
	if ( bAlreadyHandledCriticalError == FALSE )
	{
		bAlreadyHandledCriticalError = TRUE;

		GCrashReporterURL.Empty();

		// Dump the error and flush the log.
		debugf(TEXT("=== Critical error: ===") LINE_TERMINATOR TEXT("%s"), GErrorHist);
		GLog->Flush();

	// Create an AutoReporter report.
#ifndef _DEBUG
		{
			TCHAR ReportDumpVersion[] = TEXT("3");
			TCHAR ReportDumpFilename[] = TEXT("UE3AutoReportDump.txt");
			TCHAR AutoReportExe[] = TEXT("AutoReporter.exe");
			TCHAR IniDumpFilename[] = TEXT("UE3AutoReportIniDump.txt");

			HANDLE AutoReportFile = CreateFile(ReportDumpFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (AutoReportFile != INVALID_HANDLE_VALUE)
			{
				TCHAR CompName[256];
				appStrcpy(CompName, appComputerName());
				TCHAR UserName[256];
				if (InstigatorUserName.Len() > 0 && InstigatorUserName.InStr(appUserName(), FALSE, TRUE) == INDEX_NONE)
				{
					// Override the current machine's username with the instigator's username, 
					// So that it's easy to track crashes on remote machines back to the person launching the lighting build.
					appStrcpy(UserName, *InstigatorUserName);
				}
				else
				{
					appStrcpy(UserName, appUserName());
				}
				TCHAR GameName[256];
				appStrcpy(GameName, TEXT("Lightmass"));
				TCHAR PlatformName[32];
#if _WIN64
				appStrcpy(PlatformName, TEXT("PC 64-bit"));
#else
				appStrcpy(PlatformName, TEXT("PC 32-bit"));
#endif
				TCHAR LangExt[10];
				appStrcpy(LangExt, TEXT("English"));
				TCHAR SystemTime[256];
				appStrcpy(SystemTime, *appSystemTimeString());
				TCHAR EngineVersionStr[32];
				appStrcpy(EngineVersionStr, *appItoa(1));

				TCHAR ChangelistVersionStr[32];
				appStrcpy(ChangelistVersionStr, *appItoa(0));

				TCHAR CmdLine[2048];
				appStrncpy(CmdLine, appCmdLine(), ARRAY_COUNT(CmdLine));
				appStrncat(CmdLine, TEXT(" -unattended"), ARRAY_COUNT(CmdLine));
				TCHAR BaseDir[260];
				appStrncpy(BaseDir, appBaseDir(), ARRAY_COUNT(BaseDir));
				TCHAR separator = 0;

				TCHAR EngineMode[64];
				appStrcpy(EngineMode, TEXT("Tool"));

				//build the report dump file
				::DWORD NumBytesWritten = 0;
				WriteFile( AutoReportFile, ReportDumpVersion, appStrlen(ReportDumpVersion) * sizeof(TCHAR), &NumBytesWritten, NULL );
				WriteFile( AutoReportFile, &separator, sizeof(TCHAR), &NumBytesWritten, NULL );
				WriteFile( AutoReportFile, CompName, appStrlen(CompName) * sizeof(TCHAR), &NumBytesWritten, NULL );
				WriteFile( AutoReportFile, &separator, sizeof(TCHAR), &NumBytesWritten, NULL );
				WriteFile( AutoReportFile, UserName, appStrlen(UserName) * sizeof(TCHAR), &NumBytesWritten, NULL );
				WriteFile( AutoReportFile, &separator, sizeof(TCHAR), &NumBytesWritten, NULL );
				WriteFile( AutoReportFile, GameName, appStrlen(GameName) * sizeof(TCHAR), &NumBytesWritten, NULL );
				WriteFile( AutoReportFile, &separator, sizeof(TCHAR), &NumBytesWritten, NULL );
				WriteFile( AutoReportFile, PlatformName, appStrlen(PlatformName) * sizeof(TCHAR), &NumBytesWritten, NULL );
				WriteFile( AutoReportFile, &separator, sizeof(TCHAR), &NumBytesWritten, NULL );
				WriteFile( AutoReportFile, LangExt, appStrlen(LangExt) * sizeof(TCHAR), &NumBytesWritten, NULL );
				WriteFile( AutoReportFile, &separator, sizeof(TCHAR), &NumBytesWritten, NULL );
				WriteFile( AutoReportFile, SystemTime, appStrlen(SystemTime) * sizeof(TCHAR), &NumBytesWritten, NULL );
				WriteFile( AutoReportFile, &separator, sizeof(TCHAR), &NumBytesWritten, NULL );
				WriteFile( AutoReportFile, EngineVersionStr, appStrlen(EngineVersionStr) * sizeof(TCHAR), &NumBytesWritten, NULL );
				WriteFile( AutoReportFile, &separator, sizeof(TCHAR), &NumBytesWritten, NULL );
				WriteFile( AutoReportFile, ChangelistVersionStr, appStrlen(ChangelistVersionStr) * sizeof(TCHAR), &NumBytesWritten, NULL );
				WriteFile( AutoReportFile, &separator, sizeof(TCHAR), &NumBytesWritten, NULL );
				WriteFile( AutoReportFile, CmdLine, appStrlen(CmdLine) * sizeof(TCHAR), &NumBytesWritten, NULL );
				WriteFile( AutoReportFile, &separator, sizeof(TCHAR), &NumBytesWritten, NULL );
				WriteFile( AutoReportFile, BaseDir, appStrlen(BaseDir) * sizeof(TCHAR), &NumBytesWritten, NULL );
				WriteFile( AutoReportFile, &separator, sizeof(TCHAR), &NumBytesWritten, NULL );
				WriteFile( AutoReportFile, GErrorHist, appStrlen(GErrorHist) * sizeof(TCHAR), &NumBytesWritten, NULL );
				WriteFile( AutoReportFile, &separator, sizeof(TCHAR), &NumBytesWritten, NULL );
				WriteFile( AutoReportFile, EngineMode, appStrlen(EngineMode) * sizeof(TCHAR), &NumBytesWritten, NULL );
				WriteFile( AutoReportFile, &separator, sizeof(TCHAR), &NumBytesWritten, NULL );
				FlushFileBuffers( AutoReportFile );
				CloseHandle( AutoReportFile );

				FString UserLogFile( GLog->GetLogFilename() );
				extern TCHAR GMiniDumpFilenameW[1024];
				//start up the auto reporting app, passing the report dump file path, the games' log file, the ini dump path and the minidump path
				//protect against spaces in paths breaking them up on the commandline
				FString CallingCommandLine = FString::Printf(TEXT("\"%s\" \"%s\" \"%s\" \"%s\" -unattended"), ReportDumpFilename, *UserLogFile, IniDumpFilename, GMiniDumpFilenameW);
				void* ProcHandlePtr = appCreateProc(AutoReportExe, *CallingCommandLine);
				if ( ProcHandlePtr )
				{
					HANDLE ProcHandle = (HANDLE)ProcHandlePtr;
					if ( WaitForSingleObject( ProcHandle, 10000 ) == WAIT_OBJECT_0 )
					{
						// Read the URL from the crash report log file
						FILE *GeneratedAutoReportFile;
						if ( fopen_s( &GeneratedAutoReportFile, "AutoReportLog.txt", "r" ) == 0 )
						{
							// Read each line, looking for the URL
							const unsigned int LineBufferSize = 1024;
							char LineBuffer[LineBufferSize];
							char* URLSearchText = "CrashReport url = ";
							char* URLFoundText = NULL;
							while( fgets( LineBuffer, LineBufferSize, GeneratedAutoReportFile ) != NULL )
							{
								if( ( URLFoundText = strstr( LineBuffer, URLSearchText ) ) != NULL )
								{
									URLFoundText += strlen( URLSearchText );
									char ConvertedURLText[LineBufferSize] ="liuliang";
									//if( MultiByteToWideChar( CP_ACP, 0, URLFoundText, -1, ConvertedURLText, LineBufferSize ) > 0 )
									{
										GCrashReporterURL = ConvertedURLText;
										break;
									}
								}
							}
							fclose( GeneratedAutoReportFile );
						}
						else
						{
							GCrashReporterURL = TEXT("Not found (unable to open log file)!");
						}
					}
					CloseHandle( ProcHandle );
				}
				else
				{
					debugf(TEXT("Couldn't start up the Auto Reporting process!"));
				}
			}
		}
#endif	//#ifndef _DEBUG

		// Call the critical error callback function, if it has been set by the user.
		if ( GCriticalErrorCallback )
		{
			GCriticalErrorCallback();
		}
	}
}

}	//namespace Lightmass
