/*=============================================================================
	LMBuild.h: Build options as InitLMCore function
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

namespace Lightmass
{

/**
 * DO_GUARD_SLOW is enabled for debug builds which enables checkSlow, appErrorfDebug, ...
 **/
#ifdef _DEBUG
#define DO_GUARD_SLOW	1
#else
#define DO_GUARD_SLOW	0
#endif

// check/checkf
#define DO_CHECK			1
// checkSlow/ checkSlowf
#define DO_CHECK_SLOW		(0 && DO_CHECK)

// debugf
#define DO_LOG				1
// debugfSlow
#define DO_LOG_SLOW			(0 && DO_LOG)

/**
 * Allowing debug files means debugging files can be created, and CreateFileWriter does not need
 * a max file size it can grow to (needed for PS3 HD caching for now, may use for Xbox caching)
 */
#define ALLOW_DEBUG_FILES 1

}
