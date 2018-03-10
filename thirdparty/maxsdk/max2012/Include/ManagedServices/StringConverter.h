//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// AUTHOR: Nicolas Desjardins
// DATE: 2007-01-08 
//***************************************************************************/

#pragma once

#pragma managed(push, off)
#include "..\WindowsDefines.h"
#include "..\strclass.h"
#pragma managed(pop)

#pragma managed(push, on)
#include <vcclr.h>

namespace ManagedServices
{

// Note that this documentation does not use "///" for extraction by the
// compiler.  It will fail to be processed correctly.  Here's Microsoft's
// explanation:

// In the current release, code comments are not processed on templates or 
// anything containing a template type (for example, a function taking a 
// parameter as a template). Doing so will result in undefined behavior.
// [Copied from web MSDN on 2007-08-31.]
	
// <summary>
// Converts a managed System::String to the template parameter character type:
// char or wchar_t.
// </summary>
// <remarks>
// Uses CStr and WStr to perform the conversion.  It is assumed that these 
// converters are allocated on the stack.  This is important for the internal 
// CStr or WStr memory to be managed correctly.
// </remarks>
// <typeparam name="char_type">
// The destination character type, char or wchar_t, to which to convert 
// the input String.
// </typeparam>
template<typename char_type> struct StringConverter
{
	// <summary>
	// The destination character type, char or wchar_t, to which to convert 
	// the input String.
	// </summary>
	typedef char_type* converted_type;

	// <summary>
	// Convert the given String to the char_type destination character type.
	// </summary>
	// <remarks>
	// This uses an internal CStr or WStr to perform the actual conversion and
	// manage the destination string memory.  This memory will be overwritten
	// by any subsequent call to Convert and will be cleaned up when this
	// StringConverter goes out of scope.
	// </remarks>
	// <param name="in">
	// Source input string to be converted.
	// </param>
	// <returns>
	// Pointer to the resulting null-terminated character string.  This memory
	// is owned by this StringConverter.  Ownership is not given to client code.
	// </returns>
	char_type* Convert(System::String^ in)
	{
		// This should all be const, but INode methods are not const-correct
		// Rather than jump through hoops to use const_casting wrapper functors,
		// I'm stripping off the const here.

		pin_ptr<const wchar_t> pinned( PtrToStringChars(in) );
		mString = static_cast<const wchar_t*>(pinned);
		return const_cast<char_type*>(mString.data());
	}

private:
	// internal template to pick the appropriate type of MSTR depending on the
	// char_type template parameter
	template <typename T> struct TstrPicker {};
	template <>	struct TstrPicker<char> { typedef CStr type; };
	template <> struct TstrPicker<wchar_t> { typedef WStr type; };

	// <summary>
	// Internal storage for the resulting converted string.
	// </summary>
	typename TstrPicker<char_type>::type mString;
};

}

#pragma managed(pop)
