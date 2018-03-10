/*=============================================================================
	LMMath.h: Some implementation of LM math functionality
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

namespace Lightmass
{

extern FString InstigatorUserName;

/**
 * The platform that this is running on.  This mask is also used by UFunction::PlatformFlags to determine which platforms
 * a native function can be bound for.
 */
enum EPlatformType
{
	PLATFORM_Unknown	=	0x00000000,
	PLATFORM_Windows	=	0x00000001,
	PLATFORM_Xenon		=	0x00000004,
	PLATFORM_PS3		=	0x00000008,
	PLATFORM_Linux		=	0x00000010,
	PLATFORM_MacOSX		=	0x00000020,


	// Combination Masks
	/** PC platform types */
	PLATFORM_PC			=	PLATFORM_Windows|PLATFORM_Linux|PLATFORM_MacOSX,

	/** Console platform types */
	PLATFORM_Console	=	PLATFORM_Xenon|PLATFORM_PS3,

	/** These flags will be inherited from parent classes */
	PLATFORM_Inherit	=	PLATFORM_Console|PLATFORM_Linux|PLATFORM_MacOSX,
};

/**
 * Determines which platform we are currently running on
 */
inline EPlatformType appGetPlatformType()
{
#if _MSC_VER && !XBOX
	return PLATFORM_Windows;
#elif XBOX
	return PLATFORM_Xenon;
#elif PS3
	return PLATFORM_PS3;
#elif PLATFORM_LINUX
	return PLATFORM_Linux;
#elif PLATFORM_MACOSX
	return PLATFORM_MacOSX;
#else
#error Please define your platform.
#endif
}

/**
* This is used to capture all of the module information needed to load pdb's.
*/
struct FModuleInfo
{
	QWORD BaseOfImage;
	DWORD ImageSize;
	DWORD TimeDateStamp;
	TCHAR ModuleName[32];
	TCHAR ImageName[256];
	TCHAR LoadedImageName[256];
	DWORD PdbSig;
	DWORD PdbAge;
#if XBOX || WIN32
	GUID PdbSig70;
#else
	struct
	{
		unsigned long  Data1;
		unsigned short Data2;
		unsigned short Data3;
		unsigned char  Data4[8];
	} PdbSig70;
#endif
};

/**
 * Symbol information associated with a program counter.
 */
struct FProgramCounterSymbolInfo
{
	/** Module name.					*/
	ANSICHAR	ModuleName[1024];
	/** Function name.					*/
	ANSICHAR	FunctionName[1024];
	/** Filename.						*/
	ANSICHAR	Filename[1024];
	/** Line number in file.			*/
	INT			LineNumber;
	/** Symbol displacement of address.	*/
	INT			SymbolDisplacement;
};

/** @name ObjectFlags
* Flags used to control the output from stack tracing
*/
typedef DWORD EVerbosityFlags;

#define VF_DISPLAY_BASIC		0x00000000
#define VF_DISPLAY_FILENAME		0x00000001
#define VF_DISPLAY_MODULE		0x00000002
#define VF_DISPLAY_ALL			0xffffffff

/**
 * Returns the number of modules loaded by the currently running process.
 */
INT appGetProcessModuleCount();

/**
 * Gets the signature for every module loaded by the currently running process.
 *
 * @param	ModuleSignatures		An array to retrieve the module signatures.
 * @param	ModuleSignaturesSize	The size of the array pointed to by ModuleSignatures.
 *
 * @return	The number of modules copied into ModuleSignatures
 */
INT appGetProcessModuleSignatures(FModuleInfo *ModuleSignatures, const INT ModuleSignaturesSize);

/**
 * Initializes stack traversal and symbol. Must be called before any other stack/symbol functions. Safe to reenter.
 */
UBOOL appInitStackWalking();

/**
 * Converts the passed in program counter address to a human readable string and appends it to the passed in one.
 * @warning: The code assumes that HumanReadableString is large enough to contain the information.
 *
 * @param	ProgramCounter			Address to look symbol information up for
 * @param	HumanReadableString		String to concatenate information with
 * @param	HumanReadableStringSize size of string in characters
 * @param	VerbosityFlags			Bit field of requested data for output. -1 for all output.
 */ 
void appProgramCounterToHumanReadableString( QWORD ProgramCounter, ANSICHAR* HumanReadableString, SIZE_T HumanReadableStringSize, EVerbosityFlags VerbosityFlags = VF_DISPLAY_ALL );

/**
 * Converts the passed in program counter address to a symbol info struct, filling in module and filename, line number and displacement.
 * @warning: The code assumes that the destination strings are big enough
 *
 * @param	ProgramCounter			Address to look symbol information up for
 * @return	symbol information associated with program counter
 */
FProgramCounterSymbolInfo appProgramCounterToSymbolInfo( QWORD ProgramCounter );

/**
 * Capture a stack backtrace and optionally use the passed in exception pointers.
 *
 * @param	BackTrace			[out] Pointer to array to take backtrace
 * @param	MaxDepth			Entries in BackTrace array
 * @param	Context				Optional thread context information
 */
void appCaptureStackBackTrace( QWORD* BackTrace, DWORD MaxDepth, CONTEXT* Context = NULL );

}
