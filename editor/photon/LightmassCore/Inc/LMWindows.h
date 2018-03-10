/*=============================================================================
	LMWindows.h: Gathers all the Windows headers/compile time checks
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

/*----------------------------------------------------------------------------
	Include needed Windows headers
----------------------------------------------------------------------------*/

// Disable the warning that the pack size is changed in this header.
#pragma warning(push)
#pragma warning(disable:4103)

#pragma pack(push,8)

#ifndef STRICT
#define STRICT
#endif

#ifdef _WINDOWS_
#pragma message ( " " )
#pragma message ( "You have included windows.h before MinWindows.h" )
#pragma message ( "All useless stuff from the windows headers won't be excluded !!!" )
#pragma message ( " " )
#endif // _WINDOWS_

// Define the following to exclude some unused services from the windows headers.
// For information on what the following defenitions will exclude, look in the windows.h header file.
#define NOGDICAPMASKS
#define NOMENUS
#define NORASTEROPS
#define NOATOM
#define NODRAWTEXT
#define NOKERNEL
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NOCRYPT

// Define these for MFC projects
#define NOTAPE
#define NOIMAGE
#define NOPROXYSTUB
#define NORPC

// Also define WIN32_LEAN_AND_MEAN to exclude rarely-used services from windows headers.
#define WIN32_LEAN_AND_MEAN

// Finally now we can include windows.h
#include <windows.h>
//#include <new>
#include <tchar.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <psapi.h>
#include <TlHelp32.h>				// For module info.
#include <DbgHelp.h>				// For stack walker.

// restore the pack warning (4103)
#pragma warning(pop)

#pragma pack(pop)


// SIMD intrinsics
// @dawntodo: This was not inside a pack pragma in UE3, is that correct?
#include <mmintrin.h>
#include <intrin.h>


// Undo any Windows defines.
#undef BYTE
#undef WORD
#undef DWORD
#undef INT
#undef FLOAT
#undef MAXBYTE
#undef MAXWORD
#undef MAXDWORD
#undef MAXINT
#undef CDECL
#undef PF_MAX
#undef PlaySound
#undef DrawText


/**
 * Performs a compile-time assertion. Similar in functionality to Loki or Boost STATIC_CHECK.
 * This particular syntax was chosen over others because it allows the assertion to be used 
 * at namespace, class, or block scope (which Loki does not) and supports a custom error 
 * message (which Boost does not).
 *
 * The expression is carefully phrased to produce a compiler error that contains the msg provided.
 * In VC8 the expression checkAtCompileTime(false, MsgHere) will display:
 *
 *     error C2087: 'COMPILE_ERROR_MsgHere' : missing subscript
 *
 * NOTE: This doesn't currently work in gcc as its error message does not display the type.
 * Several variants of Loki or Boost techniques failed to achieve all of the goals desired,
 * so an implementation was chosen that resembles the VC version as closely as possible.
 *
 * For reasons like this static_assert will be added to C++0x:
 *   http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2004/n1720.html
 *
 * @param expr		Must be evaluatable at compile time.
 * @param msg		Message to be displayed. Somewhat obscured because we coerce
 *                  the compiler into displaying an error with msg present.
 */
#if _MSC_VER
	#define checkAtCompileTime(expr, msg)  typedef char COMPILE_ERROR_##msg[1][(expr)]
#else
	// gcc seems to ignore zero-sized arrays (which is non-conforming), but they are necessary 
	// to get VC8 to print out the error msg. gcc won't print out the error msg anyway.
	#define checkAtCompileTime(expr, msg)  typedef char COMPILE_ERROR_##msg[1][(expr)?1:-1]
#endif

/**
 * 32-bit and 64-bit checks
 */
#if defined(__x86_64__) || defined(_M_X64) || defined(__LP64__) || defined(__POWERPC64__)
	#define PLATFORM_64BITS 1
	checkAtCompileTime(sizeof (void *) == 8, 64BitPlatformsShouldHave8BytePointers);
#elif defined(__i386__) || defined(_M_IX86) || defined(_M_PPC) || defined(__LP32__) || defined(__POWERPC__)
	#define PLATFORM_32BITS 1
	checkAtCompileTime(sizeof (void *) == 4, 32BitPlatformsShouldHave4BytePointers);
#else
	#error Please define your platform.
#endif


/**
 * Simple check for VC8. Since the check for SP1 is more indirect, check directly for this first.
 */
checkAtCompileTime(_MSC_VER >= 1400, VisualStudio2005_SP1_Required);

/**
 * We require at least VC8/ Visual Studio 2005 SP1 so check for it here. See:
 * http://msmvps.com/blogs/vandooren/archive/2007/01/18/detection-of-the-vc2005-compiler-version-at-compile-time-rtm-or-sp1.aspx
 * for details.
 */
struct FCheckForVS2005SP1
{ 
public: 
	static char Dummy[10];
	enum { RequiredSize = 
		#if defined(_WIN64)
			8
		#else
			4
		#endif
	};
};
//checkAtCompileTime(sizeof(&FCheckForVS2005SP1::Dummy) == FCheckForVS2005SP1::RequiredSize, VisualStudio2005_SP1_Required);
checkAtCompileTime(_MSC_VER >= 1600 && _MSC_FULL_VER >= 160040219, VisualStudio2010_SP1_Required);


/**
 * Future-proofing the min version check so we keep bumping it whenever we upgrade.
 */ 
#if _MSC_VER > 1800
	#pragma message("Detected compiler newer than Visual Studio 2010, please update min version checking in LMWindows.h")
#endif


// If C++ exception handling is disabled, force guarding to be off.
#ifndef _CPPUNWIND
	#error "Bad VCC option: C++ exception handling must be enabled" //lint !e309 suppress as lint doesn't have this defined
#endif

// Make sure characters are unsigned.
#ifdef _CHAR_UNSIGNED
	#error "Bad VC++ option: Characters must be signed" //lint !e309 suppress as lint doesn't have this defined
#endif






// Unwanted VC++ level 4 warnings to disable.
#pragma warning(disable : 4100) // unreferenced formal parameter										
#pragma warning(disable : 4127) // Conditional expression is constant									
#pragma warning(disable : 4200) // Zero-length array item at end of structure, a VC-specific extension	
#pragma warning(disable : 4201) // nonstandard extension used : nameless struct/union	
#pragma warning(disable : 4244) // conversion to float, possible loss of data						
#pragma warning(disable : 4245) // 'initializing': conversion from 'type' to 'type', signed/unsigned mismatch 
#pragma warning(disable : 4291) // typedef-name '' used as synonym for class-name ''                    
#pragma warning(disable : 4324) // structure was padded due to __declspec(align())						
#pragma warning(disable : 4355) // this used in base initializer list                                   
#pragma warning(disable : 4389) // signed/unsigned mismatch                                             
#pragma warning(disable : 4511) // copy constructor could not be generated                              
#pragma warning(disable : 4512) // assignment operator could not be generated                           

#pragma warning(disable : 4514) // unreferenced inline function has been removed						
#pragma warning(disable : 4699) // creating precompiled header											
#pragma warning(disable : 4702) // unreachable code in inline expanded function							
#pragma warning(disable : 4710) // inline function not expanded											
#pragma warning(disable : 4711) // function selected for autmatic inlining								
#pragma warning(disable : 4714) // __forceinline function not expanded									
#pragma warning(disable : 4482) // nonstandard extension used: enum 'enum' used in qualified name (having hte enum name helps code readability and should be part of TR1 or TR2)
#pragma warning(disable : 4748)	// /GS can not protect parameters and local variables from local buffer overrun because optimizations are disabled in function

// NOTE: _mm_cvtpu8_ps will generate this falsely if it doesn't get inlined
#pragma warning(disable : 4799)	// Warning: function 'ident' has no EMMS instruction


// all of the /Wall warnings that we are able to enable
// @todo:  once we have 2005 working check:  http://msdn2.microsoft.com/library/23k5d385(en-us,vs.80).aspx
#pragma warning(default : 4191) // 'operator/operation' : unsafe conversion from 'type of expression' to 'type required'
#pragma warning(disable : 4217) // 'operator' : member template functions cannot be used for copy-assignment or copy-construction
#pragma warning(disable : 4242) // 'variable' : conversion from 'type' to 'type', possible loss of data
#pragma warning(default : 4254) // 'operator' : conversion from 'type1' to 'type2', possible loss of data
#pragma warning(default : 4255) // 'function' : no function prototype given: converting '()' to '(void)'
#pragma warning(disable : 4263) // 'function' : member function does not override any base class virtual member function
#pragma warning(default : 4287) // 'operator' : unsigned/negative constant mismatch
#pragma warning(default : 4289) // nonstandard extension used : 'var' : loop control variable declared in the for-loop is used outside the for-loop scope
#pragma warning(default : 4302) // 'conversion' : truncation from 'type 1' to 'type 2'
#pragma warning(disable : 4339) // 'type' : use of undefined type detected in CLR meta-data - use of this type may lead to a runtime exception
#pragma warning(disable : 4347) // behavior change: 'function template' is called instead of 'function
#pragma warning(disable : 4514) // unreferenced inline/local function has been removed
#pragma warning(default : 4529) // 'member_name' : forming a pointer-to-member requires explicit use of the address-of operator ('&') and a qualified name
#pragma warning(default : 4536) // 'type name' : type-name exceeds meta-data limit of 'limit' characters
#pragma warning(default : 4545) // expression before comma evaluates to a function which is missing an argument list
#pragma warning(default : 4546) // function call before comma missing argument list
#pragma warning(default : 4547) // 'operator' : operator before comma has no effect; expected operator with side-effect
#pragma warning(default : 4548) // expression before comma has no effect; expected expression with side-effect  (needed as xlocale does not compile cleanly)
#pragma warning(default : 4549) // 'operator' : operator before comma has no effect; did you intend 'operator'?
#pragma warning(disable : 4555) // expression has no effect; expected expression with side-effect
#pragma warning(default : 4557) // '__assume' contains effect 'effect'
#pragma warning(disable : 4623) // 'derived class' : default constructor could not be generated because a base class default constructor is inaccessible
#pragma warning(disable : 4625) // 'derived class' : copy constructor could not be generated because a base class copy constructor is inaccessible
#pragma warning(disable : 4626) // 'derived class' : assignment operator could not be generated because a base class assignment operator is inaccessible
#pragma warning(default : 4628) // digraphs not supported with -Ze. Character sequence 'digraph' not interpreted as alternate token for 'char'
#pragma warning(disable : 4640) // 'instance' : construction of local static object is not thread-safe
#pragma warning(disable : 4668) // 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
#pragma warning(default : 4682) // 'parameter' : no directional parameter attribute specified, defaulting to [in]
#pragma warning(default : 4686) // 'user-defined type' : possible change in behavior, change in UDT return calling convention
#pragma warning(disable : 4710) // 'function' : function not inlined / The given function was selected for inline expansion, but the compiler did not perform the inlining.
#pragma warning(default : 4786) // 'identifier' : identifier was truncated to 'number' characters in the debug information
#pragma warning(default : 4793) // native code generated for function 'function': 'reason'

#pragma warning(default : 4905) // wide string literal cast to 'LPSTR'
#pragma warning(default : 4906) // string literal cast to 'LPWSTR'
#pragma warning(disable : 4917) // 'declarator' : a GUID cannot only be associated with a class, interface or namespace ( ocid.h breaks this)
#pragma warning(default : 4931) // we are assuming the type library was built for number-bit pointers
#pragma warning(default : 4946) // reinterpret_cast used between related classes: 'class1' and 'class2'

#pragma warning(default : 4928) // illegal copy-initialization; more than one user-defined conversion has been implicitly applied
#if !USE_SECURE_CRT
#pragma warning(disable : 4996) // 'function' was was declared deprecated  (needed for the secure string functions)
#else
#pragma warning(default : 4996)	// enable deprecation warnings
#endif

// interesting ones to turn on and off at times
#pragma warning(disable : 4264) // 'virtual_function' : no override available for virtual member function from base 'class'; function is hidden
#pragma warning(disable : 4265) // 'class' : class has virtual functions, but destructor is not virtual
#pragma warning(disable : 4266) // '' : no override available for virtual member function from base ''; function is hidden

#pragma warning(disable : 4296) // 'operator' : expression is always true / false

#pragma warning(disable : 4820) // 'bytes' bytes padding added after member 'member'

// Mixing MMX/SSE intrinsics will cause this warning, even when it's done correctly.
#pragma warning(disable : 4730) //mixing _m64 and floating point expressions may result in incorrect code

// It'd be nice to turn these on, but at the moment they can't be used in DEBUG due to the varargs stuff.	
#pragma warning(disable : 4189) // local variable is initialized but not referenced 
#pragma warning(disable : 4505) // unreferenced local function has been removed		


// we know PTRINT will be big enough in 64-bit, so it's okay
#pragma warning(disable : 4311) //'type cast' : pointer truncation from 'const void *' to 'PTRINT'
#pragma warning(disable : 4302) //'type cast' : truncation from 'const void *' to 'PTRINT'
#pragma warning(disable : 4312) //'reinterpret_cast' : conversion from 'const Lightmass::PTRINT' to 'const Lightmass::DWORD *' of greater size



// Optimization macros (preceeded by #pragma).
#define DISABLE_OPTIMIZATION optimize("",off)
#ifdef _DEBUG
	#define ENABLE_OPTIMIZATION  optimize("",off)
#else
	#define ENABLE_OPTIMIZATION  optimize("",on)
#endif

/**
 * NOTE: The objects these macros declare have very short lifetimes. They are
 * meant to be used as parameters to functions. You cannot assign a variable
 * to the contents of the converted string as the object will go out of
 * scope and the string released.
 *
 * NOTE: The parameter you pass in MUST be a proper string, as the parameter
 * is typecast to a pointer. If you pass in a char, not char* it will compile
 * and then crash at runtime.
 *
 * Usage:
 *
 *		SomeApi(TCHAR_TO_ANSI(SomeUnicodeString));
 *
 *		const char* SomePointer = TCHAR_TO_ANSI(SomeUnicodeString); <--- Bad!!!
 */
#define TCHAR_TO_ANSI(str) (ANSICHAR*)FTCHARToANSI((const TCHAR*)str)
#define TCHAR_TO_OEM(str) (ANSICHAR*)FTCHARToOEM((const TCHAR*)str)
#define ANSI_TO_TCHAR(str) (TCHAR*)FANSIToTCHAR((const ANSICHAR*)str)

/** In Debug builds, returns non-zero (TRUE) if a debugger is attached. */
#ifdef _DEBUG
#define appIsDebuggerPresent()	IsDebuggerPresent()
#else
#define appIsDebuggerPresent()	IsDebuggerPresent()
#endif
