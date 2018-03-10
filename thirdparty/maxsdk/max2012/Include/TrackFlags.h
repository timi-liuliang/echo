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

#include "strbasic.h"

// Flags passed to MapKeys and DeleteKeys
#define TRACK_DOSEL			(1<<0)
#define TRACK_DOALL			(1<<1)  // ignore selection
#define TRACK_SLIDEUNSEL	(1<<2)  // Slide unselected keys to the right
#define TRACK_RIGHTTOLEFT	(1<<3)  // Enumerate right to left. If TRACK_SLIDEUNSEL is set, keys will slide to the left.
#define TRACK_DOSUBANIMS	(1<<4)  // This flag is only passed to MapKeys
#define TRACK_DOCHILDNODES	(1<<5)	// This flag is only passed to MapKeys
#define TRACK_MAPRANGE		(1<<6)	// The range, if not locked to first and last key, should be mapped as well.
#define TRACK_DOSOFTSELECT	(1<<7)	// Apply soft selection weights to the results of Time map.  This flag is only passed to MapKeys
#define TRACK_INSERTKEYS	(1<<8)	// Unselected keys after the insertion point get moved to the right
#define TRACK_REPLACEKEYS	(1<<9)	// Unselected keys within selected keys' range get deleted

// Flags passed to EditTimeRange
#define EDITRANGE_LINKTOKEYS	(1<<0)	// This means if one of the ends of the interval is at a key, link it to the key so that if the key moves, the interval moves.

// Flags passed to hit test tracks and fcurves.
#define HITTRACK_SELONLY		(1<<0)
#define HITTRACK_UNSELONLY		(1<<1)
#define HITTRACK_ABORTONHIT		(1<<2)
#define HITCURVE_TESTTANGENTS	(1<<3)
#define HITTRACK_SUBTREEMODE	(1<<4)  // Subtree mode is on so the anim is being asked to hittest itself in one of its ancestor's tracks
#define HITCURVE_TESTALLTANGENTS (1<<5) // If PAINTCURVE_SHOWALLTANGENTS is set then you also want to hit test all tangent handles

// Flags passed to SelectKeys
// Either SELECT, DESELECT, or a combination of CLEARKEYS and CLEARCURVE
// will be specified.
#define SELKEYS_SELECT			(1<<0)	
#define SELKEYS_DESELECT		(1<<1)
#define SELKEYS_CLEARKEYS		(1<<2)		 
#define SELKEYS_CLEARCURVE		(1<<3) 
#define SELKEYS_FCURVE			(1<<4) 	// indicates that were operating on keys of a function curve, not a track  


// Flags passed to GetTimeRange
#define TIMERANGE_SELONLY		(1<<0)	// The bounding interval of selected keys
#define TIMERANGE_ALL			(1<<1)  // Whatever the channel's time range is - usually the bunding interval of all keys.
#define TIMERANGE_CHILDNODES	(1<<2)  // A node's time range should include child nodes.
#define TIMERANGE_CHILDANIMS	(1<<3)  // A animatable's child anim ranges should be included

// Passed to the functions that modify a time range such as copy,paste,delete,reverse
#define TIME_INCLEFT		(1<<10)  // Include left endpoint
#define TIME_INCRIGHT		(1<<11)  // Include right endpoint
#define TIME_NOSLIDE		(1<<12)  // Delete any keys in the interval but don't actually remove the block of time.

// In addition to the TIME_ flags above, the following flag may be passed to PasteTrck()
#define PASTE_RELATIVE		(1<<20)	// Add the keys relative to existing keys

// Flags passed to AddKey
#define ADDKEY_SELECT		(1<<0)  // Select the new key and deselect any other selected keys
#define ADDKEY_INTERP		(1<<1)  // Init the new key to the interpolated value at that time. Otherwise, init the key to the value of the previous key.
#define ADDKEY_FLAGGED		(1<<2)  // Flag the newly created key as if FlagKey() was called for it

// Flags passed to CopyKeysFromTime()
#define COPYKEY_POS			(1<<0)	// These filter flags are passed to a tm controller. The tm
#define COPYKEY_ROT			(1<<1)	// can decide what to do with them... they have obvious meaning
#define COPYKEY_SCALE		(1<<2)	// For the PRS controller.

// Flags passed to GetNextKeyTime()
#define NEXTKEY_LEFT		(1<<0)	// Search to the left.
#define NEXTKEY_RIGHT		(1<<1)  // Search to the right.
#define NEXTKEY_POS			(1<<2)
#define NEXTKEY_ROT			(1<<3)
#define NEXTKEY_SCALE		(1<<4)

// Flags passed to IsKeyAtTime
#define KEYAT_POSITION		(1<<0)
#define KEYAT_ROTATION		(1<<1)
#define KEYAT_SCALE			(1<<2)

// Flags passed to PaintTrack and PaintFCurves
#define PAINTTRACK_SHOWSEL			(1<<0)
#define PAINTTRACK_SHOWSTATS		(1<<1)	// Show for selected keys
#define PAINTCURVE_SHOWTANGENTS		(1<<2)	// Show for selected keys
#define PAINTCURVE_FROZEN			(1<<3)  // Curve is in a frozen state
#define PAINTCURVE_GENCOLOR			(1<<4)	// Draw curve in generic color
#define PAINTCURVE_XCOLOR			(1<<5)	// Draw curve in red
#define PAINTCURVE_YCOLOR			(1<<6)	// Draw curve in green
#define PAINTCURVE_ZCOLOR			(1<<7)	// Draw curve in blue
#define PAINTTRACK_SUBTREEMODE		(1<<8)  // Subtree mode is on so the anim is being asked to paint itself in one of its ancestor's tracks
#define PAINTTRACK_HIDESTATICVALUES (1<<9)  // Indicates that static values shouldn't be displayed in tracks.
#define PAINTCURVE_FROZENKEYS		(1<<10) // Show keys on frozen tracks
#define PAINTCURVE_SHOWALLTANGENTS	(1<<11) // Show tangents for unselected keys as well
#define PAINTCURVE_SOFTSELECT		(1<<12) // Draw curves using Soft Selection
#define PAINTCURVE_WCOLOR			(1<<13)	// Draw curve in yellow

// Flags passed to GetFCurveExtents
#define EXTENTS_SHOWTANGENTS		(1<<0) // Tangents are visible for selected keys
#define EXTENTS_SHOWALLTANGENTS		(1<<1) // Tangents are visible for all keys
#define EXTENTS_FITINRANGE			(1<<2) // Fit only keys with a value falling within the currently displayed time range
#define EXTENTS_SELECTED_KEYS		(1<<3) // Fit only selected keys

// Values returned from PaintTrack, PaintFCurve and HitTestTrack
#define TRACK_DONE			1		// Track was successfully painted/hittested
#define TRACK_DORANGE		2		// This anim has no track. Draw/hittest the bounding range of it's subanims
#define TRACK_ASKCLIENT		3		// Ask client anim to paint the track
#define TRACK_DOSTANDARD	4		// Have the system draw and hit test the track for you -- Version 4.5 and later only

// Values returned from HitTestFCurve
#define HITCURVE_KEY		1	// Hit one or more keys
#define HITCURVE_WHOLE		2   // Hit the curve (anywhere)
#define HITCURVE_TANGENT	3	// Hit a tangent handle
#define HITCURVE_NONE		4	// Didn't hit squat
#define HITCURVE_ASKCLIENT	5	// Ask client to hit test fcurve.

// These flags are passed into PaintFCurves, HitTestFCurves, and GetFCurveExtnents
// They are filters for controllers with more than one curve
// NOTE: RGBA controllers interpret X as red, Y as green, Z as blue, and A as yellow.
#define DISPLAY_XCURVE		(1<<28)
#define DISPLAY_YCURVE		(1<<29)
#define DISPLAY_ZCURVE		(1<<30)
#define DISPLAY_WCURVE		(1<<31)

// Values returned from GetSelKeyCoords()
#define KEYS_NONESELECTED	(1<<0)
#define KEYS_MULTISELECTED	(1<<1)
#define KEYS_COMMONTIME		(1<<2)  // Both of these last two bits
#define KEYS_COMMONVALUE	(1<<3)  // could be set.

// Flags passed to GetSelKeyCoords()
#define KEYCOORDS_TIMEONLY		(1<<0)
#define KEYCOORDS_VALUEONLY		(1<<1)

// Variable definitions for SetSelKeyCoordsExpr()
#define KEYCOORDS_TIMEVAR	_M("n")
#define KEYCOORDS_VALVAR	_M("n")

// Return values from SetSelKeyCoordsExpr()
#define KEYCOORDS_EXPR_UNSUPPORTED	0	// Don't implement this method
#define	KEYCOORDS_EXPR_ERROR		1   // Error in expression
#define KEYCOORDS_EXPR_OK			2   // Expression evaluated

// Returned from NumKeys() if the animatable is not keyframeable
#define NOT_KEYFRAMEABLE	-1

// Flags passed to AdjustTangents
#define ADJTAN_LOCK		(1<<0)
#define ADJTAN_BREAK	(1<<1)

// Flags passed to EditTrackParams
#define EDITTRACK_FCURVE	(1<<0)	// The user is in the function curve editor.
#define EDITTRACK_TRACK		(1<<1) 	// The user is in one of the track views.
#define EDITTRACK_SCENE		(1<<2)	// The user is editing a path in the scene.
#define EDITTRACK_BUTTON	(1<<3)	// The user invoked by choosing the properties button. In this case the time parameter is NOT valid.
#define EDITTRACK_MOUSE		(1<<4)	// The user invoked by right clicking with the mouse. In this case the time parameter is valid.


// These are returned from TrackParamsType(). They define how the track parameters are invoked.
#define TRACKPARAMS_NONE	0	// Has no track parameters
#define TRACKPARAMS_KEY		1	// Entered by right clicking on a selected key
#define TRACKPARAMS_WHOLE	2	// Entered by right clicking anywhere in the track.

// Flags passed into RenderBegin
#define RENDERBEGIN_IN_MEDIT   1   // Indicates that the render is occuring in the material editor.

