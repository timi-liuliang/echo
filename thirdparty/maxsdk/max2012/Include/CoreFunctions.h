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
#pragma once

#include <WTypes.h>
#include "CoreExport.h"
// forward declarations
class MacroRecorder;
class MacroDir;

CoreExport void SetLockFailureLevel(int level);
CoreExport int GetLockFailureLevel();

//! For the macro-recorder to establish itself with CORE
CoreExport void SetMacroRecorderInterface(MacroRecorder* mri);
//! To set the CUI macro scrit directory ref in CORE
CoreExport void SetMacroScriptInterface(MacroDir* msd);
						

// \name System Setting Flags
// \brief Values to pass to GetSystemSetting():
//@{
//! Are editable meshes enabled?
#define SYSSET_ENABLE_EDITABLEMESH		1
/*! When GetSystemSetting is called with this the undo buffer is
	cleared. GetSystemSetting will return 0. */
#define SYSSET_CLEAR_UNDO				2
//! Are keyboard accelerators enabled for the editable mesh.
#define SYSSET_EDITABLEMESH_ENABLE_KEYBOARD_ACCEL	3
//! Is the edit meh modifier enabled?
#define SYSSET_ENABLE_EDITMESHMOD	4
//@}

// This API allows plug-in to query various system settings.
CoreExport int GetSystemSetting(int id);

// Returns the state of the VERSION_3DSMAX #define from PLUGAPI.H
// when the running version of MAX was compiled.
CoreExport DWORD Get3DSMAXVersion();


//! \name Motion Capture INI Flags
// Special access to the MAX INI file for motion capture
//@{
#define MCAP_INI_CHANNEL	1
#define MCAP_INI_PRESET		2
#define MCAP_INI_STOP		3
#define MCAP_INI_PLAY		4
#define MCAP_INI_RECORD		5
#define MCAP_INI_SSENABLE	6
//@}

CoreExport int GetMotionCaptureINISetting(int ID);
CoreExport void SetMotionCaptureINISetting(int ID, int val);

// CoreExecute: generic expansion capability
CoreExport INT_PTR CoreExecute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0);


