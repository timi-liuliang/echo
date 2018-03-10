//
// Copyright 2009 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.  
//
//

#pragma once

#include "GetResourceString.h"

// The Microsoft linker provides the __ImageBase pseudo-variable to point
// to the start of a module in memory.  It is equivalent to HINSTANCE.
EXTERN_C IMAGE_DOS_HEADER __ImageBase;

namespace MaxSDK
{

/**
 * \brief Get the calling module's HINSTANCE.
 *
 * \return Handle pointing to the start of the calling module.
 */
inline HINSTANCE GetHInstance()
{
	return reinterpret_cast<HINSTANCE>(&__ImageBase);
}

/**
 * \brief Extract a resource from the calling module's string table.
 *
 * GetResourceString is essentially a wrapper around the Win32 API's LoadString,
 * but it saves client code from the bother of dealing with its HINSTANCE, 
 * of maintaining a buffer, and reduces duplicated code. 
 *
 * Note that this uses a static buffer internally, and is therefore not thread-
 * safe.  Furthermore, if a client needs multiple resource strings 
 * simultaneously, it must copy GetResourceString's result locally before
 * making a subsequent call.
 *
 * \param resourceId Identifier for the desired string resource within the
 * calling module's string table.
 * \return Pointer to a null-terminated character string, or null if the 
 * requested resource is not found.
 */
inline const MCHAR* GetResourceString(UINT resourceId)
{
	return MaxSDK::GetResourceString(GetHInstance(), resourceId);
}

/**
 * \brief Extract a resource from the calling module's string table.
 *
 * GetResourceStringAsMSTR is essentially a thread-safe wrapper around the Win32 API's LoadString,
 * but it saves client code from the bother of dealing with its HINSTANCE, 
 * of maintaining a buffer, and reduces duplicated code. 
 *
 * \param resourceId Identifier for the desired string resource within the
 * calling module's string table.
 * \return An MSTR containing the resource string, empty if the requested resource is not found.
 */
inline  MSTR GetResourceStringAsMSTR(UINT resourceId)
{
	return MaxSDK::GetResourceStringAsMSTR(GetHInstance(), resourceId);
}

/**
 * \brief Extract a resource from the calling module's string table.
 *
 * GetResourceStringAsMSTR is essentially a thread-safe wrapper around the Win32 API's LoadString,
 * but it saves client code from the bother of dealing with its HINSTANCE, 
 * of maintaining a buffer, and reduces duplicated code. 
 *
 * \param resourceId Identifier for the desired string resource within the
 * calling module's string table.
 * \param resourceString Updated to contain the resource string if the requested resource is not found.
 * \return true if the requested resource is found, false if not
 */
inline  bool GetResourceStringAsMSTR(UINT resourceId, MSTR& resourceString)
{
	return MaxSDK::GetResourceStringAsMSTR(GetHInstance(), resourceId, resourceString);

}


}
