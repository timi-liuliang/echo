/*****************************************************************************
* FILE:              BuildWarnings.h
* DESCRIPTION: Enables\Disables build warnings
* CREATED BY:     Attila Szabo
* HISTORY:  
* - 2003.Oct.14 - created 
******************************************************************************
* Copyright (c) 2003 Autodesk, All Rights Reserved.
******************************************************************************/

#pragma once

// The recommended way to disable\enable warnings, or to change their level
// is via the following files 
// maxsdk\ProjectSettings\AdditionalCompilerOptions.txt - for x32 build targets
// maxsdk\ProjectSettings\AdditionalCompilerOptions64.txt - for x64 build targets

// Macro used to shield from changes to the syntax of the deprecated keyword, 
// and to make the deprecated declarations more succinct.
#define MAX_DEPRECATED __declspec(deprecated)

// Macro used to avoid the C4100 compiler warning. Use this for function parameters
// that are not used.
#define UNUSED_PARAM(x) (x)

