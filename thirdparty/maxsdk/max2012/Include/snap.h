//
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.  
//
//

#pragma once

#include "maxheap.h"
#include <WTypes.h>
#include "matrix3.h"
#include "point2.h"


/// \defgroup Snap_types Snap types 
//@{
#define SNAP_2D	1		//!< 2 dimensional Snap
#define SNAP_25D	2	//!< 2.5 dimensional Snap
#define SNAP_3D	3		//!< 3 dimensional Snap
//@}

/// \defgroup Snap_modes Snap modes
/** Snap mode values used by Interface::SetSnapMode(), Interface::GetSnapMode(). */
//@{
#define SNAPMODE_RELATIVE	0	//!< Relative snap mode.
#define SNAPMODE_ABSOLUTE	1	//!< Absolute snap mode.
//@}

/// \defgroup Snap_Flags Snap Flags
//@{
#define SNAP_IN_3D								(0)			//!< Snap to all points.
#define SNAP_IN_PLANE							(1<<0)		//!< Snap only to points on the construction (or optionally specified) plane.
#define SNAP_UNSEL_OBJS_ONLY			(1<<1)				//!< Ignore selected nodes when considering snap points.
#define SNAP_SEL_OBJS_ONLY				(1<<2)				//!< Ignore unselected nodes when considering snap points.
#define SNAP_UNSEL_SUBOBJ_ONLY		(1<<3)					//!< Ignore selected sub-object geometry when considering snap points.
#define SNAP_SEL_SUBOBJ_ONLY			(1<<4)				//!< Ignore unselected sub-object geometry when considering snap points.
#define SNAP_FORCE_3D_RESULT			(1<<5)				//!< Override user settings to force snap in 3D
/**
* \internal Used internally to snap only to points off the plane. 
* Many of the objects call GetCPDisp() when they want to snap lenghts etc. 
* This method knew nothing about the snapping that was added in 3ds Max 2.0 
* so it had to be retrofited to call SnapPoint(). This is where 
* SNAP_OFF_PLANE is called. It forces the osnapmanager to only consider 
* points which are OFF the current construction plane.
*/
#define SNAP_OFF_PLANE						(1<<6)			
/**
 * \internal Used internally to suppresses any display in the viewports. 
 * This can be used to provide a way of snapping to arbitrary screen points 
 * without giving feedback.*/
#define SNAP_TRANSPARENTLY				(1<<7)				
#define SNAP_APPLY_CONSTRAINTS		(1<<8)					//!< \internal Used internally to suppresses any display in the viewports.
#define SNAP_PROJ_XAXIS						(1<<9)			//!< \internal Used internally to suppresses any display in the viewports.
#define SNAP_PROJ_YAXIS						(1<<10)			//!< \internal Used internally to suppresses any display in the viewports.
#define SNAP_PROJ_ZAXIS						(1<<11)			//!< \internal Used internally to suppresses any display in the viewports.
#define SNAP_XFORM_AXIS						(1<<12)			//!< Informs the osnapmanager to invalidate the com axis
#define SNAP_BEGIN_SEQ						(1<<13)	
#define SNAP_END_SEQ						(1<<14)	

/** If set, the the origin of the selection's transform matrix 
 * is used as start snap point. No other snap point candidates 
 * that fall into the snap point radius is displayed or used 
 * as start snap point. */
#define SNAP_USE_XFORM_AXIS_AS_START_SNAP_POINT (1<<16)	
//@}

/** Describes the snap settings used for snap operations.
 * \see Osnap, IOsnapManager, BaseObject::Snap(), Mesh::Snap()
 */
struct SnapInfo: public MaxHeapOperators 
{
	int snapType;			//!< Snap settings. \see snap_types
	int strength;			//!< Maximum snap distance.
	int vertPriority;		//!< Geometry vertex priority.
	int edgePriority;		//!< Geometry edge priority.
	int gIntPriority;		//!< Grid intersection priority.
	int gLinePriority;		//!< Grid line priority.
	DWORD flags;			//!< Snap flags. \see snap_flags
	Matrix3 plane;			//!< Plane to use for snap computations.
	Point3 bestWorld;		//!< Best snap point found in world space.
	Point2 bestScreen;		//!< Best snap point found in screen space.
	int bestDist;			//!< Best snap point distance.
	int priority;			//!< Best snap point's priority
};

