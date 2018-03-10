/*=============================================================================
	Scene.cpp: PCH master include file.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#include <stdio.h>
#include <tchar.h>

// basic stuff needed by everybody
#pragma warning( disable: 4799 )		// function '...' has no EMMS instruction)
#pragma warning( disable: 4819 )
#pragma warning( disable: 4316 )

#define LORD

#include "LMCore.h"
#include "Lighting.h"
#include "3DVisualizer.h"

