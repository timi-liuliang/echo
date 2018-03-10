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

#include "maxheap.h"
#include "point3.h"
#include "matrix3.h"

/*! \sa  Class BaseObject, Class Control\n\n
\par Description:
The callback is used by the two methods <b>GetSubObjectCenters()</b> and
<b>GetSubObjectTMs()</b> found in the classes <b>BaseObject</b> and
<b>Control</b>.  */
class SubObjAxisCallback: public MaxHeapOperators {
	public:
		/*! \remarks Destructor. */
		virtual ~SubObjAxisCallback() {}
		/*! \remarks Implemented by the System.\n\n
		This method is called to specify the individual coordinate system
		center for the axes whose <b>id</b> is passed.
		\param c The center point.
		\param id The id of the axis. */
		virtual void Center(Point3 c,int id)=0;
		/*! \remarks Implemented by the System.\n\n
		This method is called to specify the individual coordinate system
		transformation for the axes whose <b>id</b> is passed.
		\param tm The transformation matrix.
		\param id The id of the axis. */
		virtual void TM(Matrix3 tm,int id)=0;
		/*! \remarks Implemented by the System.\n\n
		The user has three options for center of the coordinate system, center
		of the selection set, or pivot. For center of the coordinate system the
		system does not need to call <b>GetSubObjCenters()</b>. The plug-in may
		call this method to determine which center option it is returning the
		Centers and TMs for.
		\return  One of the following values:\n\n
		<b>SO_CENTER_SELECTION</b>\n\n
		<b>SO_CENTER_PIVOT</b> */
		virtual int Type()=0;
	};

// Values returned by Type();
#define SO_CENTER_SELECTION	1 
#define SO_CENTER_PIVOT		2