//
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form.   
//
//

#pragma once

#ifdef GRAPHICS_DRIVER_EXPORTS
#	define GraphicsDriverAPI __declspec( dllexport )
#else
#	define GraphicsDriverAPI __declspec( dllimport )
#endif
