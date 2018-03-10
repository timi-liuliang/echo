/*=============================================================================
	LMCore.h: Most basic include file for LMCore 
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

#include "LMWindows.h"			// interface to MS Windows (outside the LM namespace)

#include "LMBuild.h"			// build options, as well as InitLMCore function definition

// recommended that these are included by all files (ie, leave in LMCore.h, and maybe include LMCore.h in stdafx.h)
#include "LMTypes.h"			// variable types
#include "LMHelpers.h"			// helper #defines, etc
#include "LMStringConv.h"		// converting strings between standards (ANSI, UNICODE, etc.)
#include "LMMemory.h"			// memory allocation functionality
#include "LMTemplates.h"		// basic template functionality
#include "LMArray.h"			// TArray implementation
#include "LMQueue.h"			// TQueue implementation
#include "LMSet.h"				// TSet implementation
#include "LMMap.h"				// TMap implementation
#include "LMSorting.h"			// Sorting implementation
#include "LMString.h"			// string (FString/C strings) routines
#include "LMMath.h"				// math functions and classes
#include "LMThreading.h"		// threading functionality
#include "LMStats.h"			// Stats

// these can be moved out and just included per .cpp file
#include "LMColor.h"			// color (FColor/FLinearColor)
#include "LMSHMath.h"			// spherical harmonic math functions and classes
#include "LMGuid.h"				// GUID functionality
#include "LMOctree.h"			// TOctree functionality
#include "LMkDOP.h"				// TkDOP functionality
#include "LMCollision.h"		// Collision functionality


