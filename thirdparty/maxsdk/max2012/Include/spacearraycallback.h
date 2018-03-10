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
// FILE:        SpaceArrayCallback.H
// DESCRIPTION: SpaceArrayCallback class definition
// AUTHOR:      Peter Sauerbrei
// HISTORY:     - created January 1998
//	        - changed object type to CloneType 01/28/98 (PJS)
//	        - changed CloneType back to OTYPE 01/30/98 (PJS)
//	        - added proc to act as callback 02/02/98 (PJS)
//	        - added new context modes 02/17/98 (PJS)
//**************************************************************************/



#pragma once

#include "point3.h"
#include <limits.h>
#include "CoreExport.h"
#include "maxheap.h"
#include "strbasic.h"

// forward declarations
class ShapeObject;
class Spline3D;


// needed types
enum STYPE { SPACE_CENTER, SPACE_EDGE };
enum OTYPE { NODE_CPY, NODE_INST, NODE_REF };
enum CTYPE { CTXT_FREE, CTXT_CNTRCOUNT, CTXT_CNTRSPACE,
				CTXT_END, CTXT_ENDCOUNT, CTXT_ENDSPACE,
				CTXT_START, CTXT_STARTCOUNT, CTXT_STARTSPACE,
				CTXT_FULLSPACE, CTXT_FULLCOUNT, CTXT_ENDLOCK,
				CTXT_ENDLOCKCOUNT, CTXT_ENDLOCKSPACE, CTXT_STARTLOCK,
				CTXT_STARTLOCKCOUNT, CTXT_STARTLOCKSPACE,
				CTXT_FULLLOCKSPACE, CTXT_FULLLOCKCOUNT,};



////////////////////////////////////////////////////////////////////////////////
// SpaceArracyCallback
//
/*! \sa  Class Interface, Class ShapeObject, Class Spline3D, Class Point3.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This is the callback object for the method
<b>Interface::DoSpaceArrayDialog(SpaceArrayCallback *sacb=NULL)=0</b>\n\n
This is the method which brings up the Spacing tool. This lets the user
distribute objects based on the current selection along a path defined by a
spline or a pair of points.\n\n
The callback is mostly for use for plug-ins that need to use the spacing tool
as a way of generating spacing information. If one wants to customize the
default behaviour of the dialog, they derive a class from this one and
implement the virtual methods. The non-virtuals simply provide access to the
spacing information. This is both for setting the defaults before calling the
spacing tool as well as getting out the information after using the spacing
tool.  */
class SpaceArrayCallback: public MaxHeapOperators
{
private:
	ShapeObject * path;
	float start, end, space;
	int count;
	OTYPE oType;
	bool follow;
	STYPE sType;
	CTYPE context;
	float width;
	char buffer[100];
	int countLimit;

public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b>path = NULL; start = end = space = 0.0f; count = 1; oType = NODE_CPY;
	sType = SPACE_CENTER; follow = false; context = CTXT_FREE; width = 0.0f;
	countLimit = INT_MAX;</b> */
	SpaceArrayCallback() { path = NULL; start = end = space = 0.0f; count = 1; oType = NODE_CPY; sType = SPACE_CENTER; follow = false; 
							context = CTXT_FREE; width = 0.0f; countLimit = INT_MAX; }
	/*! \remarks Destructor. */
	virtual ~SpaceArrayCallback() {}

	// methods
	/*! \remarks Implemented by the Plug-in.\n\n
	If this method returns false, the dialog is presented as a modeless dialog.
	\par Default Implementation:
	<b>{ return true; }</b> */
	virtual bool isModal(void) { return true; }
	/*! \remarks Implemented by the Plug-in.\n\n
	If this method returns true, the path picking buttons are turned on.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool doPickPath(void) { return false; }
	/*! \remarks Implemented by the Plug-in.\n\n
	Returns the title for the dialog (e.g. Space Array Tool).
	\par Default Implementation:
	<b>{ return _M(""); }</b> */
	virtual MCHAR * dialogTitle(void) { return _M(""); }
	/*! \remarks Implemented by the Plug-in.\n\n
	Returns the message to be displayed in the static display right after the
	dialog comes up.
	\par Default Implementation:
	<b>{ return _M(""); }</b> */
	virtual MCHAR * startMessage(void) { return _M(""); }
	/*! \remarks Implemented by the Plug-in.\n\n
	Returns the button text.
	\par Default Implementation:
	<b>{ return _M("OK"); }</b> */
	virtual MCHAR * buttonText(void) { return _M("OK"); }
	/*! \remarks Implemented by the Plug-in.\n\n
	If this method returns true, the spacing tool generates points, but the
	actual dialog is not presented to the user. This is currently used, for
	example, by the Stairs in VIZ to generate spacing information for the mesh.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool isSilent(void) { return false; }
	/*! \remarks Implemented by the Plug-in.\n\n
	If this method returns false, then the Instance, Copy, Reference radio
	buttons are disabled (greyed out).
	\par Default Implementation:
	<b>{ return true; }</b> */
	virtual bool doObjectType(void) { return true; }
	/*! \remarks Implemented by the Plug-in.\n\n
	This method is called after the spacing information is generated. It is
	here that a developer using this callback could get the updated values,
	perform some calculation of their own, etc.
	\par Default Implementation:
	<b>{}</b> */
	virtual void proc(void) {}
	
	// accessors
	/*! \remarks Implemented by the System.\n\n
	Returns a pointer to the path ShapeObject. */
	ShapeObject * getPath(void)	{ return path; }
	/*! \remarks Implemented by the System.\n\n
	Sets the path used (as a ShapeObject).
	\par Parameters:
	<b>ShapeObject *p</b>\n\n
	Points to the shape object to use as a path. */
	void setPath(ShapeObject * p) { path = p; }
 	/*! \remarks Implemented by the System.\n\n
 	Sets the points for the path.
 	\par Parameters:
 	<b>Point3 pt1</b>\n\n
 	One of the endpoints.\n\n
 	<b>Point3 pt2</b>\n\n
 	The other endpoint. */
 	CoreExport void setPath(Point3 pt1, Point3 pt2);       
 	/*! \remarks Implemented by the System.\n\n
 	Sets the path used.
 	\par Parameters:
 	<b>Spline3D *s</b>\n\n
 	Points to the path to use. */
 	CoreExport void setPath(Spline3D *s);    
	/*! \remarks Implemented by the System.\n\n
	Returns the start offset. */
	float getStart(void)	{ return start; }
	/*! \remarks Implemented by the System.\n\n
	Returns the end offset. */
	float getEnd(void)		{ return end; }
	/*! \remarks Implemented by the System.\n\n
	Returns the spacing. */
	float getSpace(void)	{ return space; }
	/*! \remarks Implemented by the System.\n\n
	Returns the object count. */
	int getCount(void)		{ return count; }
	/*! \remarks Implemented by the System.\n\n
	Returns the context. See <b>setContext()</b> above. */
	int getContext(void)	{ return context; }
	/*! \remarks Implemented by the System.\n\n
	Returns the width. This is the width of the object to be arrayed. It is a
	single value so can be calculated any way the user wishes. In the default
	spacing tool this is calculated based on the x size of the bounding box. */
	float getWidth(void) { return width; }
	/*! \remarks Implemented by the System.\n\n
	Sets the start offset.
	\par Parameters:
	<b>float f</b>\n\n
	The start offset to set. */
	void setStart(float f)	{ start = f; }
	/*! \remarks Implemented by the System.\n\n
	Sets the end offset.
	\par Parameters:
	<b>float f</b>\n\n
	The end offset to set. */
	void setEnd(float f)	{ end = f; }
	/*! \remarks Implemented by the System.\n\n
	Sets the spacing.
	\par Parameters:
	<b>float f</b>\n\n
	The spacing to set. */
	void setSpace(float f)	{ space = f; }
	/*! \remarks Implemented by the System.\n\n
	Sets the count.
	\par Parameters:
	<b>int n</b>\n\n
	The count to set. */
	void setCount(int n)	{ count = n > countLimit ? countLimit : n; }
	/*! \remarks Implemented by the System.\n\n
	Returns true if Follow is set (checked); otherwise false. */
	bool getFollow(void)	{ return follow; }
	/*! \remarks Implemented by the System.\n\n
	Sets the Follow state (checkbox).
	\par Parameters:
	<b>bool t</b>\n\n
	Use true for checked; false for un-checked. */
	void setFollow(bool t)	{ follow = t; }
  /*! \remarks Implemented by the System.\n\n
  Sets the context.
  \par Parameters:
  <b>CTYPE c</b>\n\n
  One of the following values:\n\n
  <b>CTXT_FREE</b> - Free Center\n\n
  <b>CTXT_CNTRCOUNT</b> - Divide Evenly, Objects at Ends\n\n
  <b>CTXT_CNTRSPACE</b> - Centered, Specify Spacing\n\n
  <b>CTXT_END</b> - End Offset\n\n
  <b>CTXT_ENDCOUNT</b> - End Offset, Divide Evenly\n\n
  <b>CTXT_ENDSPACE</b> - End Offset, Specify Spacing\n\n
  <b>CTXT_START</b> - Start Offset\n\n
  <b>CTXT_STARTCOUNT</b> - Start Offset, Divide Evenly\n\n
  <b>CTXT_STARTSPACE</b> - Start Offset, Specify Spacing\n\n
  <b>CTXT_FULLSPACE</b> - Specify Offset and Spacing\n\n
  <b>CTXT_FULLCOUNT</b> - Specify Offsets, Divide Evenly\n\n
  <b>CTXT_ENDLOCK</b> - Space from End, Unbounded\n\n
  <b>CTXT_ENDLOCKCOUNT</b> - Space from End, Specify Number\n\n
  <b>CTXT_ENDLOCKSPACE</b> - Space from End, Specify Spacing\n\n
  <b>CTXT_STARTLOCK</b> - Space from Start, Unbounded\n\n
  <b>CTXT_STARTLOCKCOUNT</b> - Space from Start, Specify Number\n\n
  <b>CTXT_STARTLOCKSPACE</b> - Space from Start, Specify Spacing\n\n
  <b>CTXT_FULLLOCKSPACE</b> - Specify Spacing, Matching Offsets\n\n
  <b>CTXT_FULLLOCKCOUNT</b> - Divide Evenly, No Objects at Ends */
  void setContext(CTYPE c)	{ context = c; }
	/*! \remarks Implemented by the System.\n\n
	Returns the object creation type. One of the following values:\n\n
	<b>NODE_CPY</b> -- Copy\n\n
	<b>NODE_INST</b> -- Instance\n\n
	<b>NODE_REF</b> -- Reference */
	OTYPE getObjectCreationType(void)	{ return oType; }
	/*! \remarks Implemented by the System.\n\n
	Sets the object creation type. One of the following values:\n\n
	<b>NODE_CPY</b> -- Copy\n\n
	<b>NODE_INST</b> -- Instance\n\n
	<b>NODE_REF</b> -- Reference */
	void setObjectCreationType(OTYPE t) { oType = t; }
	/*! \remarks Implemented by the System.\n\n
	Returns the spacing type. One of the following values:\n\n
	<b>SPACE_CENTER</b>\n\n
	<b>SPACE_EDGE</b> */
	STYPE getSpacingType(void){ return sType; }
	/*! \remarks Implemented by the System.\n\n
	Sets the spacing type.
	\par Parameters:
	<b>STYPE s</b>\n\n
	One of the following values:\n\n
	<b>SPACE_CENTER</b>\n\n
	<b>SPACE_EDGE</b> */
	void setSpacingType(STYPE s) { sType = s; }
	/*! \remarks Implemented by the System.\n\n
	Sets the message string. */
	CoreExport void setMessage(char * buf);
	/*! \remarks Implemented by the System.\n\n
	Sets the width. See <b>getWidth()</b> above.
	\par Parameters:
	<b>float nWidth</b>\n\n
	The width to set. */
	void setWidth(float nWidth) { width = nWidth; }
	/*! \remarks Implemented by the System.\n\n
	Sets a limit on the count.
	\par Parameters:
	<b>int limit</b>\n\n
	The limit to set. */
	void setCountLimit(int limit) { countLimit = limit; }
	/*! \remarks Implemented by the System.\n\n
	Returns the limit on the count. */
	int getCountLimit(void) { return countLimit; }
};

