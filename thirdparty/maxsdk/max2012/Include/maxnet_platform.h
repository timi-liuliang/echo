//-----------------------------------------------------------------------------
// ----------------------------
// File ....: maxnet_platform.h
// ----------------------------
// Author...: Gus J Grubba
// Date ....: February 2000
//
// Descr....: 3D Studio MAX Network Interface - Platform Dependency
//
// History .: Feb, 07 2000 - Started
//            
//-----------------------------------------------------------------------------

#pragma once

#include <windows.h>

#define mn_malloc(s)	LocalAlloc(LPTR,s)
#define mn_free			LocalFree