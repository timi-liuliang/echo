//**************************************************************************/
// Copyright (c) 1998-2008 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// AUTHOR: Benjamin Cecchetto
// DATE: 2008-03-14
//***************************************************************************/

#pragma once

#ifndef STRICT
#define STRICT
#endif

#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target Windows Vista or later.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target Windows Vista or later.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows XP or later.
#define _WIN32_WINDOWS 0x0501 
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 5.01 or later.
#define _WIN32_IE 0x0501	// Change this to the appropriate value to target IE 5.5 or later.
#endif

#include <windows.h>

