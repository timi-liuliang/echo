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

#pragma once

#include "utilexp.h"

namespace MaxSDK
{
	namespace DebuggingTools
	{
		/*! A method to call a debug break during runtime.
		 * If called while running a release build, it will throw an exception and generate a Crash Error Report. 
		 * If called while running a debug build under the debugger, it will pause execution 
		 */
		UtilExport void DebugBreak();
	}
}