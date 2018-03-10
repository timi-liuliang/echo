//**************************************************************************/
// Copyright 2009 Autodesk, Inc.  All rights reserved.
// Use of this software is subject to the terms of the Autodesk license 
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form. 
//**************************************************************************/
#pragma once

#ifdef BLD_PARAMBLK2
#	define PB2Export __declspec( dllexport )
#else
#	define PB2Export __declspec( dllimport )
#endif
