//
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form.   
//
//

#pragma once

//! Preprocessor switch for the DLL's interface
#ifdef DATA_BRIDGE_MODULE
#define DataBridgeAPI __declspec( dllexport )
#else
#define DataBridgeAPI __declspec( dllimport )
#endif
