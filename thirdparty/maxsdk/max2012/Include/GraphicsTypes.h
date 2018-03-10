//
// Copyright 2010 Autodesk, Inc.  All rights reserved. 
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law.  They may
// not be disclosed to, copied or used by any third party without the prior
// written consent of Autodesk, Inc.
#pragma once

#include <WTypes.h>

typedef BOOL	(*HitFunc)(int, int, void *);
typedef void (*GFX_ESCAPE_FN)(void *);

/** Camera Types */
enum CameraType
{
	PERSP_CAM,			//!< Perspective view camera type.
	ORTHO_CAM			//!< Orthographic projection camera type. 
};

/** Color types (used by setColor) */
enum ColorType
{
	LINE_COLOR,			//!< Line drawing color.
	FILL_COLOR,			//!< Polygon fill color.
	TEXT_COLOR,			//!< Text drawing color.
	CLEAR_COLOR			//!< The color that the viewport is cleared to when you call clearScreen()
};

/** Marker types. \see GraphicsWindow::hMarker() */
enum MarkerType
{
	POINT_MRKR,			//!< A single pixel on the display.
	HOLLOW_BOX_MRKR,	//!< A small box centered on the point.
	PLUS_SIGN_MRKR,		//!< A plug sign (+) at the point.
	ASTERISK_MRKR,		//!< An asterisk (*) at the point.
	X_MRKR,				//!< An X at the point.
	BIG_BOX_MRKR,		//!< A large box centered on the point.
	CIRCLE_MRKR,		//!< A circle at the point.
	TRIANGLE_MRKR,		//!< A triangle centered on the point.
	DIAMOND_MRKR,		//!< A diamond centered on the point.
	SM_HOLLOW_BOX_MRKR,	//!< A hollow box at the point.
	SM_CIRCLE_MRKR,		//!< A small circle at the point.
	SM_TRIANGLE_MRKR,	//!< A small triangle centered on the point.
	SM_DIAMOND_MRKR,	//!< A small diamond centered on the point.
	DOT_MRKR,			//!< A large dot.
	SM_DOT_MRKR,		//!< A smaller dot.
	BOX2_MRKR,
	BOX3_MRKR,
	BOX4_MRKR,
	BOX5_MRKR,
	BOX6_MRKR,
	BOX7_MRKR,
	DOT2_MRKR,
	DOT3_MRKR,
	DOT4_MRKR,
	DOT5_MRKR,
	DOT6_MRKR,
	DOT7_MRKR
};

/** Facing type */
enum FacingType
{
	kFrontFacing,
	kSideFacing,
	kBackFacing
};