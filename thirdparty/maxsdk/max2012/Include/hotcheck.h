/**********************************************************************
 *<
	FILE:  hotcheck.h

	DESCRIPTION:  Video Color check utilities

	CREATED BY: Dan Silva

	HISTORY: created 26 December 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "coreexp.h"
// forward declarations
struct Color48;

#define VID_NTSC 0
#define VID_PAL 1

// Methods
#define HOT_FLAG 0
#define HOT_SCALE_LUM 1
#define HOT_SCALE_SAT 2


CoreExport void  BuildHotTable( int video_type = VID_NTSC );
// do video color check
CoreExport int HotLimit(Color48* thepix, int method = HOT_SCALE_LUM);

