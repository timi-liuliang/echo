/*=============================================================================
	LMHelpers.h: Mostly defines brought over from UE3 for familiarity
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

namespace Lightmass
{

	
// Function type macros.
#define VARARGS			__cdecl					/* Functions with variable arguments */
#define CDECL			__cdecl					/* Standard C function */
#define STDCALL			__stdcall				/* Standard calling convention */
#define FORCEINLINE		__forceinline			/* Force code to be inline */
#define FORCENOINLINE	__declspec(noinline)	/* Force code to NOT be inline */
#define ZEROARRAY			                    /* Zero-length arrays in structs */
#define RESTRICT		__restrict


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
INT appGetVarArgs( TCHAR* Dest, SIZE_T DestSize, INT Count, const TCHAR*& Fmt, va_list ArgPtr );

#define GET_VARARGS(msg,msgsize,len,lastarg,fmt) { va_list ap; va_start(ap,lastarg);appGetVarArgs(msg,msgsize,len,fmt,ap); }
#define GET_VARARGS_ANSI(msg,msgsize,len,lastarg,fmt) { va_list ap; va_start(ap,lastarg);appGetVarArgsAnsi(msg,msgsize,len,fmt,ap); }
#define GET_VARARGS_RESULT(msg,msgsize,len,lastarg,fmt,result) { va_list ap; va_start(ap,lastarg); result = appGetVarArgs(msg,msgsize,len,fmt,ap); }
#define GET_VARARGS_RESULT_ANSI(msg,msgsize,len,lastarg,fmt,result) { va_list ap; va_start(ap,lastarg); result = appGetVarArgsAnsi(msg,msgsize,len,fmt,ap); }

#define VARARG_NONE
#define VARARG_EXTRA(A) A,
#define VARARG_PURE =0
#define VARARG_DECL( FuncRet, StaticFuncRet, Return, FuncName, Pure, FmtType, ExtraDecl, ExtraCall )	\
	FuncRet FuncName( ExtraDecl FmtType Fmt, ... ) Pure
#define VARARG_BODY( FuncRet, FuncName, FmtType, ExtraDecl )		\
	FuncRet FuncName( ExtraDecl FmtType Fmt, ... )

#define LINE_TERMINATOR TEXT("\r\n")
#define PATH_SEPARATOR TEXT("\\")

// Alignment.
#define GCC_PACK(n)
#define GCC_ALIGN(n)
#define GCC_BITFIELD_MAGIC
#define MS_ALIGN(n) __declspec(align(n))



enum {MAXBYTE		= 0xff       };
enum {MAXWORD		= 0xffffU    };
enum {MAXDWORD		= 0xffffffffU};
enum {MAXSBYTE		= 0x7f       };
enum {MAXSWORD		= 0x7fff     };
enum {MAXINT		= 0x7fffffff };
enum {INDEX_NONE	= -1         };
enum {UNICODE_BOM   = 0xfeff     };
enum EEventParm				{EC_EventParm};
enum ENoInit				{E_NoInit = 0};
enum EInit					{E_Init = 0};
enum { DEFAULT_ALIGNMENT = 16 }; // Default boundary to align memory allocations on. Needs to be multiple of 16 for SSE.


// debug/assert functionality
#define appDebugBreak()			( appIsDebuggerPresent() ? (DebugBreak(),1) : 1 )

void VARARGS appFailAssertFunc( const ANSICHAR* Expr, const ANSICHAR* File, INT Line, const TCHAR* Format=TEXT(""), ... );
void VARARGS appFailAssertFuncDebug( const ANSICHAR* Expr, const ANSICHAR* File, INT Line, const TCHAR* Format=TEXT(""), ... );

#define appFailAssert(expr,file,line,...) { if (appIsDebuggerPresent()) Lightmass::appFailAssertFuncDebug(expr, file, line, __VA_ARGS__); appDebugBreak(); Lightmass::appFailAssertFunc(expr, file, line, __VA_ARGS__); }

/**
 * Wrapper object that will encapsulate printing to stdout and to a log file
 */
class FLogInternal
{
public:
	/**
	 * Varargs logging
	 */
	VARARG_DECL( void, void, {}, Logf, VARARG_NONE, const TCHAR*, VARARG_NONE, VARARG_NONE );

	/**
	 * Logs a text string.
	 */
	void Log( const TCHAR* Text );

	/**
	 * Flushes all previously written logs.
	 */
	void Flush();

	/**
	 * Returns the filename of the log file.
	 */
	const TCHAR* GetLogFilename() const
	{
		return Filename;
	}

	friend class FLog;
protected:
	/**
	 * Constructor. Will open up a file to write output to, to simulate UE3's logging
	 */
	FLogInternal();

	/**
	 * Destructor. Closes the file.
	 */
	~FLogInternal();

private:
	/** Handle to log file */
	FILE*	File;

	/** Filename of the log file. */
	TCHAR	Filename[ MAX_PATH ];
};

/**
 * Singleton helper class, to make sure FLog is construction upon first use.
 */
class FLog
{
public:
	/**
	 * Accessor for FLog. Safe to call at any time from any thread (even before main or any global constructor).
	 * Constructs an FLog upon first use.
	 * @return	FLogInternal object
	 */
	FLogInternal* operator->();

private:
	/** Private pointer to global instance of the logger. */
	static FLogInternal*	Log;
};

/** Logger: Safe to use at any time from any thread (even before main or any global constructor). */
extern FLog GLog;

// UE3-standard writing to screen and log file
#if DO_LOG
	#define debugf GLog->Logf
	#if DO_LOG_SLOW
		#define debugfSlow debugf
	#else
		#define debugfSlow __noop
	#endif
#else
	#define debugf __noop
	#define debugfSlow __noop
#endif


#if DO_CHECK
	#define checkCode( Code )		do { Code } while ( false );
	#define checkMsg(expr,msg)		{ if(!(expr)) {      appFailAssert( #expr " : " #msg , __FILE__, __LINE__ ); }  }
    #define checkFunc(expr,func)	{ if(!(expr)) {func; appFailAssert( #expr, __FILE__, __LINE__ ); }              }
	#define verify(expr)			{ if(!(expr)) appFailAssert( #expr, __FILE__, __LINE__ ); }
	#define check(expr)				{ if(!(expr)) appFailAssert( #expr, __FILE__, __LINE__ ); }

	/**
	 * Denotes codepaths that should never be reached.
	 */
	#define checkNoEntry()       { appFailAssert( "Enclosing block should never be called", __FILE__, __LINE__ ); }

	/**
	 * Denotes codepaths that should not be executed more than once.
	 */
	#define checkNoReentry()     { static bool s_beenHere##__LINE__ = false;                                         \
	                               checkMsg( !s_beenHere##__LINE__, Enclosing block was called more than once );   \
								   s_beenHere##__LINE__ = true; }

	class FRecursionScopeMarker
	{
	public: 
		FRecursionScopeMarker(WORD &InCounter) : Counter( InCounter ) { ++Counter; }
		~FRecursionScopeMarker() { --Counter; }
	private:
		WORD& Counter;
	};

	/**
	 * Denotes codepaths that should never be called recursively.
	 */
	#define checkNoRecursion()  static WORD RecursionCounter##__LINE__ = 0;                                            \
	                            checkMsg( RecursionCounter##__LINE__ == 0, Enclosing block was entered recursively );  \
	                            const FRecursionScopeMarker ScopeMarker##__LINE__( RecursionCounter##__LINE__ )

#define verifyf(expr, ...)				{ if(!(expr)) appFailAssert( #expr, __FILE__, __LINE__, ##__VA_ARGS__ ); }
#define checkf(expr, ...)				{ if(!(expr)) appFailAssert( #expr, __FILE__, __LINE__, ##__VA_ARGS__ ); }

#else
	// MS compilers support noop which discards everything inside the parens
	#define checkCode(Code)			{}
	#define check					__noop
	#define checkf					__noop
	#define checkMsg				__noop
	#define checkFunc				__noop
	#define checkNoEntry			__noop
	#define checkNoReentry			__noop
	#define checkNoRecursion		__noop
	#define verify(expr)			{ if(!(expr)){} }
	#define verifyf(expr, ...)		{ if(!(expr)){} }
#endif

//
// Check for development only.
//
#if DO_GUARD_SLOW
#define checkSlow(expr, ...)   {if(!(expr)) appFailAssert( #expr, __FILE__, __LINE__ );}
#define checkfSlow(expr, ...)	{ if(!(expr)) appFailAssert( #expr, __FILE__, __LINE__, __VA_ARGS__ ); }
#define verifySlow(expr)  {if(!(expr)) appFailAssert( #expr, __FILE__, __LINE__ );}
#else
// MS compilers support noop which discards everything inside the parens
#define checkSlow					__noop
#define checkfSlow					__noop
#define verifySlow(expr)			if(expr){}
#endif


// this is always active (not dependent on DO_CHECK)
//#define appErrorf(...)			( (appIsDebuggerPresent() ? appFailAssertFuncDebug("appErrorf", __FILE__, __LINE__, ##__VA_ARGS__),1 : 1), appDebugBreak(), GError->Logf(__VA_ARGS__), 1 )
#define appErrorf(...)				appFailAssert( "appErrorf was called", __FILE__, __LINE__, ##__VA_ARGS__ )

/**
 * Sets GErrorHist, outputs it to the log and debugger Output Window,
 * and stops the program execution.
 */
VARARG_DECL( void, void, {}, appCriticalError, VARARG_NONE, const TCHAR*, VARARG_NONE, VARARG_NONE );

/**
 * Sets a callback function that will be called after a critical error has been detected and handled.
 * @param CriticalErrorCallback	User-specified callback function
 */
void appSetCriticalErrorCallback( void (*CriticalErrorCallback)() );

/**
 * Handles critical error. The call only performs anything on the first call.
 */
void appHandleCriticalError();

/** Returns the latest error message. */
const TCHAR* appGetError();


/*-----------------------------------------------------------------------------
	Lightmass globals
-----------------------------------------------------------------------------*/

/** Whether the executable is guarded with a __try/__catch. */
extern UBOOL GIsGuarded;

/*-----------------------------------------------------------------------------
	Misc helper functions.
-----------------------------------------------------------------------------*/

inline TCHAR    FromAnsi   ( ANSICHAR In ) { return (BYTE)In;                        }
inline TCHAR    FromUnicode( UNICHAR In  ) { return In;                              }
inline ANSICHAR ToAnsi     ( TCHAR In    ) { return (WORD)In<0x100 ? In : MAXSBYTE;  }
inline UNICHAR  ToUnicode  ( TCHAR In    ) { return In;                              }

/**
* Current high resolution cycle counter. Origin is arbitrary.
*
* @return current value of high resolution cycle counter - origin is arbitrary
*/
FORCEINLINE QWORD appCycles()
{
	LARGE_INTEGER Cycles;
	QueryPerformanceCounter(&Cycles);
	return Cycles.QuadPart;
}

/**
 * Returns time in seconds. Origin is arbitrary.
 *
 * @return time in seconds (arbitrary origin)
 */
FORCEINLINE DOUBLE appSeconds()
{
	extern DOUBLE GSecondsPerCycle;
	checkSlow(GSecondsPerCycle != 0.0f); // TEXT("InitLMCore has not been called before using appSeconds()!"));

	// Add big number to make bugs apparent where return value is being passed to FLOAT
	return appCycles() * GSecondsPerCycle + 16777216.0;
}

/**
 * Returns the system time.
 */
void appSystemTime( INT& Year, INT& Month, INT& DayOfWeek, INT& Day, INT& Hour, INT& Min, INT& Sec, INT& MSec );

/** Get computer name.  NOTE: Only one return value is valid at a time! */
const TCHAR* appComputerName();
/** Get user name.  NOTE: Only one return value is valid at a time! */
const TCHAR* appUserName();
/** Returns the executable command line. */
const TCHAR* appCmdLine();
/** Returns the startup directory (the directory this executable was launched from).  NOTE: Only one return value is valid at a time! */
const TCHAR* appBaseDir();

//
// Creates a new process and its primary thread. The new process runs the
// specified executable file in the security context of the calling process.
//
void *appCreateProc( const TCHAR* URL, const TCHAR* Parms );

/**
 * Returns the crash reporter URL after appHandleCriticalError() has been called.
 */
const class FString& appGetCrashReporterURL();

/*-----------------------------------------------------------------------------
	Character type functions.
-----------------------------------------------------------------------------*/
#define UPPER_LOWER_DIFF	32

/** @name Character functions */
//@{
inline TCHAR appToUpper( TCHAR c )
{
	// compiler generates incorrect code if we try to use TEXT('char') instead of the numeric values directly
	// TEXT('char') comments removed as they cannot be represented in non latin-1 codepages, causing compile errors on non-US windows.
	//@hack - ideally, this would be data driven or use some sort of lookup table
	// some special cases
	switch (UNICHAR(c))
	{
	// these special chars are not 32 apart
	//case 255: return 159; // diaeresis y
	//case 156: return 140; // digraph ae


	// characters within the 192 - 255 range which have no uppercase/lowercase equivalents
	case 240: return c;
	case 208: return c;
	case 223: return c;
	case 247: return c;
	}

	if ( (c >= TEXT('a') && c <= TEXT('z')) || (c > 223 && c < 255) )
	{
		return c - UPPER_LOWER_DIFF;
	}

	// no uppercase equivalent
	return c;
}
inline TCHAR appToLower( TCHAR c )
{
	// compiler generates incorrect code if we try to use TEXT('char') instead of the numeric values directly
	// TEXT('char') comments removed as they cannot be represented in non latin-1 codepages, causing compile errors on non-US windows.
	// some special cases
	switch (UNICHAR(c))
	{
	// these are not 32 apart
	//case 159: return 255; // diaeresis y
	//case 140: return 156; // digraph ae

	// characters within the 192 - 255 range which have no uppercase/lowercase equivalents
	case 240: return c;
	case 208: return c;
	case 223: return c;
	case 247: return c;
	}

	if ( (c >= 192 && c < 223) || (c >= TEXT('A') && c <= TEXT('Z')) )
	{
		return c + UPPER_LOWER_DIFF;
	}

	// no lowercase equivalent
	return c;
}
inline UBOOL appIsUpper( TCHAR cc )
{
	UNICHAR c(cc);
	// compiler generates incorrect code if we try to use TEXT('char') instead of the numeric values directly
	// TEXT('char') comments removed as they cannot be represented in non latin-1 codepages, causing compile errors on non-US windows.
	return (c==159) || (c==140)	// these are outside the standard range
		|| (c==240) || (c==247)	// these have no lowercase equivalents
		|| (c>=TEXT('A') && c<=TEXT('Z')) || (c >= 192 && c <= 223);
}
inline UBOOL appIsLower( TCHAR cc )
{
	UNICHAR c(cc);
	// compiler generates incorrect code if we try to use TEXT('char') instead of the numeric values directly
	// TEXT('char') comments removed as they cannot be represented in non latin-1 codepages, causing compile errors on non-US windows.
	return (c==156) 															// outside the standard range
		|| (c==215) || (c==208) || (c==223)	// these have no lower-case equivalents
		|| (c>=TEXT('a') && c<=TEXT('z')) || (c >=224 && c <=  255);
}

inline UBOOL appIsAlpha( TCHAR cc )
{
	UNICHAR c(cc);
	// compiler generates incorrect code if we try to use TEXT('char') instead of the numeric values directly
	// TEXT('char') comments removed as they cannot be represented in non latin-1 codepages, causing compile errors on non-US windows.
	return (c>=TEXT('A') && c<=TEXT('Z')) 
		|| (c>=192 && c<= 255)
		|| (c>=TEXT('a') && c<=TEXT('z')) 
		|| (c==159) || (c==140) || (c==156);	// these are outside the standard range
}
inline UBOOL appIsDigit( TCHAR c )
{
	return c>=TEXT('0') && c<=TEXT('9');
}
inline UBOOL appIsAlnum( TCHAR c )
{
	return appIsAlpha(c) || (c>=TEXT('0') && c<=TEXT('9'));
}
inline UBOOL appIsWhitespace( TCHAR c )
{
	return c == TEXT(' ') || c == TEXT('\t');
}
inline UBOOL appIsLinebreak( TCHAR c )
{
	//@todo - support for language-specific line break characters
	return c == TEXT('\n');
}

/** Returns nonzero if character is a space character. */
inline UBOOL appIsSpace( TCHAR c )
{
    return( iswspace(c) != 0 );
}

inline UBOOL appIsPunct( TCHAR c )
{
	return( iswpunct( c ) != 0 );
}
//@}


/*-----------------------------------------------------------------------------
	DLLs.
-----------------------------------------------------------------------------*/

/** @name DLL access */
//@{
void* appGetDllHandle( const TCHAR* DllName );
/** Frees a DLL. */
void appFreeDllHandle( void* DllHandle );
/** Looks up the address of a DLL function. */
void* appGetDllExport( void* DllHandle, const TCHAR* ExportName );
//@}

}
