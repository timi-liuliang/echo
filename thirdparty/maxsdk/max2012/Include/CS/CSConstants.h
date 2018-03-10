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

// transition focus types
#define FOCUS_AUTO	0
#define FOCUS_COM	1
#define FOCUS_LFOOT	2
#define FOCUS_RFOOT	3
#define FOCUS_BFEET	4

// track types
#define TRANSTRACK	1
#define LAYERTRACK	2

// transition track levels
#define TOP_ROW	1
#define BOT_ROW	2

// load options
#define RE_SOURCE_CLIP_ONLY	1
#define RE_SOURCE_INSTANCES	2
#define RE_SOURCE_FILEGROUP	3

// Animation Fit Options
#define FIT_ANIM_TO_CLIP	1
#define FIT_CLIP_TO_ANIM	2
#define TRIM_ANIM_TO_CLIP	3

//string length defines
#define MAXNAME	MAX_PATH
#define MAXSCRIPTNAME 64 
#define MAXTRANNOTE 256