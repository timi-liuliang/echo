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

#include "buildver.h"

// BeginEditParams flags values
#define BEGIN_EDIT_CREATE  		(1<<0)
#define BEGIN_EDIT_MOTION		(1<<1)	// Controller is being edited in the motion branch
#define BEGIN_EDIT_HIERARCHY	(1<<2)	// Same as BEGIN_EDIT_IK
#define BEGIN_EDIT_IK			(1<<2)  // Controller is being edited in the IK subtask of the hierarchy branch
#define BEGIN_EDIT_LINKINFO		(1<<3)  // Controller is being edited in the Link Info  subtask of the hierarchy branch
#define BEGIN_EDIT_SHAPE_NO_RENDPARAM (1<<5) // Used by LoftSpline

// EndEditParams flags values
#define END_EDIT_REMOVEUI  (1<<0)