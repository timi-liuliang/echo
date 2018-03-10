/**********************************************************************
 *<
	FILE: iurvctl.h

	DESCRIPTION:	CurveControl Interface

	CREATED BY:		Nikolai Sander, Kinetix

	HISTORY:		Created 10/15/98

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/
#pragma once
#include <Wtypes.h>   // Required before commctrl.h
#include <CommCtrl.h> // for HIMAGELIST
#include "maxheap.h"
#include "ref.h"

#define CURVE_CONTROL_CLASS_ID Class_ID(0x14585773, 0x483a7dcf)

#define I_RESMAKER_INTERFACE 0x2474334a

#define WM_CC_SEL_CURVEPT         WM_USER+0x2b70 // lParam = ICurve * , LOWORD(wParam) = number of points selected
#define WM_CC_CHANGE_CURVEPT      WM_USER+0x2b71 // lParam = ICurve * , LOWORD(wParam) = point index
#define WM_CC_CHANGE_CURVETANGENT WM_USER+0x2b72 // lParam = ICurve * , LOWORD(wParam) = point index, HIWORD(wParam) & IN_CURVETANGENT_CHANGED->InTangent, HIWORD(wParam)&OUT_CURVETANGENT_CHANGED->OutTangent
#define WM_CC_DEL_CURVEPT         WM_USER+0x2b73 // lParam = ICurve * , LOWORD(wParam) = point index
#define WM_CC_INSERT_CURVEPT      WM_USER+0x2b74 // lParam = ICurve * , LOWORD(wParam) = point index  (added by AF (6/26/2000) )
#define WM_CC_LBUTTONDOWN         WM_USER+0x2b75 // lParam = ICurve * , LOWORD(wParam) = point index  (added by AF (10/31/2000) )
#define WM_CC_RBUTTONDOWN         WM_USER+0x2b76 // lParam = ICurve * , LOWORD(wParam) = point index  (added by AF (10/31/2000) )
#define WM_CC_LBUTTONUP	          WM_USER+0x2b77 // lParam = ICurve * , LOWORD(wParam) = point index  (added by AF (10/31/2000) )

#define IN_CURVETANGENT_CHANGED  (1<<0)
#define OUT_CURVETANGENT_CHANGED (1<<1)

#define IN_CURVETANGENT_CHANGED  (1<<0)
#define OUT_CURVETANGENT_CHANGED (1<<1)

/*! \defgroup customCurveControlFlags Custom Curve Control Flags
These flags control various display aspects of the custom curve control which includes window
gemometry and features. See ICurveCtl::SetCCFlags() and ICurveCtl::GetCCFlags().\n\n
Developers can  review these by running the curve control utility CCUtil in \\MAXSDK\\SAMPLES\\UTILITIES\\CCUTIL.
\sa Class ICurveCtl.
*/
//@{
#define CC_DRAWBG				(1<<0) //!< Draw the white background in the graph window.
#define CC_DRAWGRID				(1<<1) //!< Draw the grid lines and coordinates in the graph window.
#define CC_DRAWUTOOLBAR			(1<<2) //!< Draw the upper toolbar above the control.
#define CC_SHOWRESET			(1<<3) //!< Display the Reset button in the upper toolbar.
#define CC_DRAWLTOOLBAR			(1<<4) //!< Draw the lower toolbar beneath the control.
#define CC_DRAWSCROLLBARS		(1<<5) //!< Draw the horizontal and vertical scroll bars for the control.
/*!  Auto scroll happens when you drag a CurvePoint out of the currently visible range. In case
you're zoomed in, the window will automtically scroll. */
#define CC_AUTOSCROLL			(1<<6) //!< Do auto scrolling.
#define CC_DRAWRULER			(1<<7) //!< Draw a small moveable ruler window that measures horizontal coordinates.
/*! Show the window as popup window. If this is not set, it is important to set 
SetCustomParentWnd(HWND hParent)=0 for the parent of the CurveControl Window. */
#define CC_ASPOPUP				(1<<8) //!< Create the control as a pop-up with a title string.
#define CC_CONSTRAIN_Y			(1<<9) //!< No points (or handles) can be moved out of the value that is set by SetYRange().
/*! If this is not set, the disabled curves will be drawn by the color set by SetDisabledPenProperty(). */
#define CC_HIDE_DISABLED_CURVES (1<<10) //!< Disabled curves won't be displayed at all.

// Rightclick menu
#define CC_RCMENU_MOVE_XY	    (1<<11) //!< Right-click menu item.
#define CC_RCMENU_MOVE_X	    (1<<12) //!< Right-click menu item. 
#define CC_RCMENU_MOVE_Y	    (1<<13) //!< Right-click menu item.
#define CC_RCMENU_SCALE			(1<<14) //!< Right-click menu item. 
#define CC_RCMENU_INSERT_CORNER	(1<<15) //!< Right-click menu item. 
#define CC_RCMENU_INSERT_BEZIER	(1<<16) //!< Right-click menu item. 
#define CC_RCMENU_DELETE		(1<<17) //!< Right-click menu item. 
//watje
/*! You loose the automatic switch from insert to move mode when right-clicking, if a RightClick menu is active. Also, pressing the Control key while inserting keys will insert the opposite type
of key as what's currently selected (bezier or corner). And, pressing the Shift key while moving a
tangent converts a bezier key into a bezier corner key. */
#define CC_SHOW_CURRENTXVAL		(1<<18) //!< Allows a vertical bar to be drawn over the graph to show the current X value.
/*! Normally, if several points are stacked in area, clicking on the area you get all of them.
With this flag you get the first one only. */
#define CC_SINGLESELECT			(1<<19) //!< Allows the user to single select a point
/*!  This is useful when there are many curves and you want to manage the display yourself. */
#define CC_NOFILTERBUTTONS		(1<<20) //!< Turns off the curve visible/editable toggle in the top of the menu bar.
#define CC_ALL_RCMENU (CC_RCMENU_MOVE_XY|CC_RCMENU_MOVE_X|CC_RCMENU_MOVE_Y|CC_RCMENU_SCALE|CC_RCMENU_INSERT_CORNER|CC_RCMENU_INSERT_BEZIER|CC_RCMENU_DELETE) //!< Entire right click menu.
#define CC_ALL (CC_DRAWBG|CC_DRAWGRID|CC_DRAWUTOOLBAR|CC_SHOWRESET|CC_DRAWLTOOLBAR|CC_DRAWSCROLLBARS|CC_AUTOSCROLL|CC_DRAWRULER|CC_ASPOPUP|CC_CONSTRAIN_Y|CC_HIDE_DISABLED_CURVES| CC_ALL_RCMENU )  //!< 
#define CC_NONE 0 //!< 
//@}

#define CID_CC_MOVE_XY				0
#define CID_CC_MOVE_X				1
#define CID_CC_MOVE_Y				2
#define CID_CC_SCALE				3
#define CID_CC_INSERT_CORNER		4
#define CID_CC_INSERT_BEZIER		5

//Curve out of range types
#define CURVE_EXTRAPOLATE_LINEAR	0
#define CURVE_EXTRAPOLATE_CONSTANT	1

// IPoint flags
#define CURVEP_BEZIER		(1<<0)
#define CURVEP_CORNER		(1<<1)
#define CURVEP_LOCKED_Y		(1<<2)
#define CURVEP_LOCKED_X		(1<<3)
#define CURVEP_SELECTED		(1<<4)
#define CURVEP_ENDPOINT		(1<<8)	// It's a constrained endpoint on the curve
#define CURVEP_NO_X_CONSTRAINT (1<<9)  //Added by AF (6/26/2000)

// Flags passed to SelectPts
#define SELPTS_SELECT			(1<<0)	
#define SELPTS_DESELECT			(1<<1)
#define SELPTS_CLEARPTS			(1<<2)		 

#define IS_CORNER(flags)        ( ( (flags) & CURVEP_CORNER) && !((flags) & CURVEP_BEZIER) )
#define IS_BEZIERSMOOTH(flags)  ( ( (flags) & CURVEP_BEZIER) && !((flags) & CURVEP_CORNER) )
#define IS_BEZIERCORNER(flags)  ( (flags) & (CURVEP_BEZIER | CURVEP_CORNER) )



class ICurve;

/*! \sa  Class ReferenceTarget, Class ICurve, Class CurvePoint, Class ResourceMakerCallback, Class BitArray, Class Interval.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class is an interface to the curve custom control. An example of this
control in the 3ds Max user interface can be seen in the Color Map section of
the Output rollup of a 2D Texture map. Sample code using these APIs is
available in <b>/MAXSDK/SAMPLES/UTILITIES/CCUTIL/CCUTIL.CPP</b>.  */
class ICurveCtl : public ReferenceTarget {
public:
	
	/*! \remarks This method indicates if the dialog box for the control is up
	or not. Returns TRUE if active; otherwise FALSE. */
	virtual BOOL  IsActive()=0;
	/*! \remarks This method is used to bring up or close the dialog box.
	\par Parameters:
	<b>BOOL sw</b>\n\n
	TRUE to open; FALSE to close. */
	virtual void  SetActive(BOOL sw)=0;
	/*! \remarks Returns the window handle of the control. */
	virtual HWND  GetHWND()=0;
	/*! Sets the number of curves used in this control.
		Note, that this method actually create the curves, which means,
		that the NewCurveCreatedCallback method will be called. However, this happens only 
		if a ResourceMakerCallback is registered already.
		Thus it is important,that the ResourceMakerCallback is registered BEFORE this method
		is called.
		\param iNum - The number of curves to use.
		\param doUndo - This will cause the function to register an Restore Object if set to TRUE. */
	virtual void  SetNumCurves(int iNum, BOOL doUndo=FALSE)=0;
	/*! Deletes all curves associated with the curve control. */
	virtual void DeleteAllCurves() = 0;
	/*! \remarks Returns the numbers of curves used by the control. */
	virtual int   GetNumCurves()=0;
	/*! \remarks Sets the absolute position of the first and last CurvePoints.
	\par Parameters:
	<b>float min</b>\n\n
	The minimum value.\n\n
	<b>float max</b>\n\n
	The maximum value.\n\n
	<b>BOOL rescaleKeys = TRUE</b>\n\n
	This parameter is available in release 4.0 and later only.\n\n
	This parameter controls whether changing the X range will rescale the keys
	and key tangents or not. When this is TRUE, the default, the keys and
	tangents get resinced to the total X range. Setting this to FALSE allow
	developers to have move control over the exact placement of keys and
	tangents when the X range changes. This also allow developers to set ranges
	from within the points changed message handler without getting into a loop.
	*/
	virtual void  SetXRange(float min, float max, BOOL rescaleKeys = TRUE)=0;	// Determines the first and last key for all curves
	/*! \remarks This determines the absolute upper and lower Y-constraint.
	This method only has an effect if the <b>CC_CONSTRAIN_Y</b> flag is set.
	\par Parameters:
	<b>float min</b>\n\n
	The minimum value.\n\n
	<b>float max</b>\n\n
	The maximum value. */
	virtual void  SetYRange(float min, float max)=0;				// Determines the upper and lower limits, if the Flag CC_CONSTRAIN_Y is set
	/*! \remarks	Returns the X Range. */
	virtual Point2 GetXRange()=0;									//Returns the X range as a Point2 (added by AF (6/26/2000) )
	/*! \remarks	Returns the Y Range. */
	virtual Point2 GetYRange()=0;									//Returns the Y range as a Point2 (added by AF (6/26/2000) )							
	
	/*! \remarks This methods registers a callback object used to handle
	certain aspects of the control. The callback object will be used for
	updating of the display button image list and tool tip text for the curve
	control. It also gets called when the user executes a Reset or creates a
	new control.\n\n
	This registers a reference maker which implements the method
	<b>Animatable::GetInterface()</b> for the id <b>I_RESMAKER_INTERFACE</b> by
	returning an object derived from class <b>ResourceMakerCallback</b>.
	\par Parameters:
	<b>ReferenceMaker *rmak</b>\n\n
	Points to the reference maker which returns the callback object. */
	virtual void  RegisterResourceMaker(ReferenceMaker *rmak)=0;	// This registers a rmaker, which has to implement GetInterface 
																	// for I_RESMAKER_INTERFACE by returning an object derived from 
																	// ResourceMakerCallback
	
	/*! \remarks Returns the current zoom values.
	\par Parameters:
	<b>float *h</b>\n\n
	Points to storage for the horizontal zoom value.\n\n
	<b>float *v</b>\n\n
	Points to storage for the vertical zoom value.
	\return  TRUE if valid values were returned; otherwise FALSE. */
	virtual BOOL  GetZoomValues(float *h, float *v)=0;				// Returns the current zoom values
	/*! \remarks Sets the zoom values. To determine the values to use
	developers should use the CCUtil plug-in, create a pop-up window as big as
	is appropriate for the final control, adjust the zoom and scroll values
	interactively, and then simply read out the values from the CCUtil user
	interface (GetZoom and GetScroll).
	\par Parameters:
	<b>float h</b>\n\n
	The horizontal value to set.\n\n
	<b>float v</b>\n\n
	The vertical value to set. */
	virtual void  SetZoomValues(float h, float v)=0;				// Sets the zoom values
	/*! \remarks Returns the horizontal and vertical scroll values.
	\par Parameters:
	<b>int *h</b>\n\n
	Points to storage for the horizontal scroll value.\n\n
	<b>int *v</b>\n\n
	Points to storage for the vertical scroll value.
	\return  TRUE if valid values were returned; otherwise FALSE. */
	virtual BOOL  GetScrollValues(int *h, int *v)=0;				// Returns the current Scroll Values
	/*! \remarks Sets the scroll values. To determine the values to use
	developers should use the CCUtil plug-in, create a pop-up window as big as
	is appropriate for the final control, adjust the zoom and scroll values
	interactively, and then simply read out the values from the CCUtil user
	interface (GetZoom and GetScroll).
	\par Parameters:
	<b>int h</b>\n\n
	The horizontal scroll value to set.\n\n
	<b>int v</b>\n\n
	The vertical scroll value to set. */
	virtual void  SetScrollValues(int h, int v)=0;					// Sets the scroll values
	/*! \remarks	Performs a zoom extents operation to the curve view. */
	virtual void  ZoomExtents()=0;
	/*! \remarks Sets the title of the dialog box to the specified string.
	This is only used if the <b>CC_ASPOPUP</b> is used to create a pop-up
	dialog.
	\par Parameters:
	<b>MCHAR *str</b>\n\n
	The title string to display. */
	virtual void  SetTitle(MCHAR *str)=0;							// Sets the title of the dialog box
	/*! \remarks Returns and interface to the specified curve. This interface
	allows you to set the color of the curve and retrieve the Y value of the
	curve given an X value.
	\par Parameters:
	<b>int numCurve</b>\n\n
	The zero based index of the curve. */
	virtual ICurve *GetControlCurve(int numCurve)=0;				// Returns and interface to the numCurve'th curve
	/*! \remarks Determines which curves are toggled on.
	\par Parameters:
	<b>BitArray \&mode</b>\n\n
	The BitArray to control curve visibility -- one bit for each curve. If the
	bit is set the curve is toggled on; otherwise it is toggled off. */
	virtual void  SetDisplayMode(BitArray &mode)=0;					// Determines which curves are toggled on
	/*! \remarks Returns a BitArray which indicates which curves are toggled
	on or off. */
	virtual BitArray GetDisplayMode()=0;							// Returns which curves are toggled on
	/*! \remarks Sets the curve control flags to those passed.
	\par Parameters:
	<b>DWORD flags</b>\n\n
	See \ref customCurveControlFlags. */
	virtual void  SetCCFlags(DWORD flags)=0;
	/*! \remarks Returns the curve control flags. See \ref customCurveControlFlags. */
	virtual DWORD GetCCFlags()=0;
	/*! \remarks Sets the parent window for the control if the controll is not
	a popup window.
	\par Parameters:
	<b>HWND hParent</b>\n\n
	The window handle of the parent. */
	virtual void  SetCustomParentWnd(HWND hParent)=0;				// Parent Window, if CurveControl is no popup window
	/*! \remarks Call this method and the following window messages will be
	sent to the window whose handle is passed. The contents of the
	<b>lParam</b> and <b>wParam</b> parameters passed to the window proc are
	shown.\n\n
	<b>WM_CC_SEL_CURVEPT</b>\n\n
	Sent when a point is selected or deselected.\n\n
	<b>lParam</b> = <b>ICurve *</b> , <b>LOWORD(wParam)</b> = The number of
	points which are selected.\n\n
	<b>WM_CC_CHANGE_CURVEPT</b>\n\n
	Sent when a point is changed.\n\n
	<b>lParam</b> = <b>ICurve *</b> , <b>LOWORD(wParam)</b> = The zero based
	index of the changed point.\n\n
	<b>WM_CC_CHANGE_CURVETANGENT</b>\n\n
	Sent when a point's in or out tangent is changed.\n\n
	<b>lParam = ICurve * , LOWORD(wParam)</b>= The zero based index of the
	changed point,\n\n
	<b>HIWORD(wParam)</b> contains a flag, that indicates if the changed
	tangent is the in, or out tangent. You can check these flags as
	<b>HIWORD(wParam) \& IN_CURVETANGENT_CHANGED</b> and <b>HIWORD(wParam) \&
	OUT_CURVETANGENT_CHANGED</b>, for the in and out tangent respectively.\n\n
	<b>WM_CC_DEL_CURVEPT</b>\n\n
	Sent when a point is deleted.\n\n
	<b>lParam</b> = <b>ICurve *</b> , <b>LOWORD(wParam)</b> = The zero based
	index of the deleted point.\n\n
	<b>WM_CC_INSERT_CURVEPT</b>\n\n
	This option is available in release 4.0 and later only.\n\n
	<b>lParam = ICurve *, LOWORD(wParam) =</b> The zero based index of the
	inserted point.
	\par Parameters:
	<b>HWND hWnd</b>\n\n
	The handle of the window which will receive the messages. */
	virtual void  SetMessageSink(HWND hWnd)=0;						// WM_CC_CHANGE_CURVEPT, WM_CC_CHANGE_CURVETANGENT and WM_CC_DEL_CURVEPT will be sent to this window
	/*! \remarks Sets the command mode in use by the control.
	\par Parameters:
	<b>int ID</b>\n\n
	One of the following values (which correspond directly to toolbar buttons
	in the UI. See the 2D texture map Output rollup for example):\n\n
	<b>CID_CC_MOVE_XY</b>\n\n
	<b>CID_CC_MOVE_X </b>\n\n
	<b>CID_CC_MOVE_Y </b>\n\n
	<b>CID_CC_SCALE</b>\n\n
	<b>CID_CC_INSERT_CORNER</b>\n\n
	<b>CID_CC_INSERT_BEZIER</b> */
	virtual void  SetCommandMode(int ID)=0;
	/*! \remarks Returns a value which indicates the current command mode. One
	of the following values (which correspond directly to toolbar buttons in
	the UI. See the 2D texture map Output rollup for example):\n\n
	<b>CID_CC_MOVE_XY</b>\n\n
	<b>CID_CC_MOVE_X </b>\n\n
	<b>CID_CC_MOVE_Y </b>\n\n
	<b>CID_CC_SCALE</b>\n\n
	<b>CID_CC_INSERT_CORNER</b>\n\n
	<b>CID_CC_INSERT_BEZIER</b> */
	virtual int   GetCommandMode()=0;
	/*! \remarks This method redraws the custom curve control. */
	virtual void  Redraw()=0;
	/*! \remarks Returns an <b>Interval</b> which reflects the validity of
	every point of every curve used by the curve control.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time about which the interval is computed. */
	virtual Interval	GetValidity(TimeValue t)=0;
	/*! \remarks Updates the validity interval passed with the overall
	validity of the curves in the control. This simply does:\n\n
	 <b>valid \&= GetValidity(t);</b>
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time about which the interval is computed.\n\n
	<b>Interval\& valid</b>\n\n
	The interval which is updated. */
	virtual void Update(TimeValue t, Interval& valid)=0;
//watje
//this draws a vertical bar at the current value
	/*! \remarks	This sets the position of the vertical line drawn over the graph showing
	the current X value. See flag <b>CC_SHOW_CURRENTXVAL</b> in \ref customCurveControlFlags.
	\par Parameters:
	<b>float val</b>\n\n
	The value to set. */
	virtual void SetCurrentXValue(float val)=0;
//this turns on/off the display code.  It is useful when you are doing lots of changes and don't
//want the window to continually redraw
	/*! \remarks	This method allows you to turn on/off the display code. It is useful when
	you are doing lots of changes and don't want the window to continually
	redraw.
	\par Parameters:
	<b>BOOL enable</b>\n\n
	TRUE to enable; FALSE to disable. */
	virtual void EnableDraw(BOOL enable)=0;

	virtual void DeleteCurve(int index)=0;							//Added by AF (08/17/00) (used by reactor)

};

/*! \sa  Class ICurve.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class stores data about a single point on a curve used by the custom curve
control.
\par Data Members:
<b>Point2 p;</b>\n\n
The curve point.\n\n
<b>Point2 in;</b>\n\n
The in tangent, relative to <b>p</b>.\n\n
<b>Point2 out;</b>\n\n
The out tangent, relative to <b>p</b>.\n\n
<b>int flags;</b>\n\n
One or more of the following values which describes the type of curve
point:\n\n
<b>CURVEP_BEZIER</b>\n\n
Indicates the point is a bezier smooth point.\n\n
<b>CURVEP_CORNER</b>\n\n
Indicates the point is a corner point.\n\n
To get a Bezier Corner use:\n\n
<b>CURVEP_CORNER \& CURVEP_BEZIER</b>\n\n
<b>CURVEP_LOCKED_Y</b>\n\n
Indicates the point is locked in Y.\n\n
<b>CURVEP_LOCKED_X</b>\n\n
Indicates the point is locked in X.\n\n
<b>CURVEP_SELECTED</b>\n\n
Indicates the point is selected.\n\n
<b>CURVEP_ENDPOINT</b>\n\n
Indicates a constrained endpoint on the curve.\n\n
<b>CURVEP_NO_X_CONSTRAINT</b>\n\n
Indicates a point should not be constrained to X. */
class CurvePoint: public MaxHeapOperators
{
public:
	Point2 p;
	Point2 in;
	Point2 out;
	int flags;
	CurvePoint()
		: flags(0)
	{
		p.x = 0.0f;
		p.y = 0.0f;
		in.x = 0.0f;
		in.y = 0.0f;
		out.x = 0.0f;
		out.y = 0.0f;
	}
	CurvePoint& operator=(const CurvePoint& rhs)
	{
		p		= rhs.p;
		in		= rhs.in;
		out		= rhs.out;
		flags	= rhs.flags;
		return *this;
	}
};


/*! \sa  Class ReferenceTarget, Class ICurveCtl, Class CurvePoint, <a href="ms-its:listsandfunctions.chm::/idx_R_colorref.html">COLORREF</a>,
Class BitArray.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class is an interface to a single curve used by a <b>ICurveCtl</b>. A
pointer to one of these is returned from the method
<b>ICurveCtl::GetControlCurve()</b>.\n\n
All methods of this class are implemented by the system.  */
#pragma warning(push)
#pragma warning(disable:4239)

class ICurve : public ReferenceTarget 
{
public:
	/*! \remarks Sets the pen properties of a curve
	\par Parameters:
	<b>COLORREF color</b>\n\n
	The color for the curve lines.\n\n
	<b>int width = 0</b>\n\n
	The width of the lines in pixels.\n\n
	<b>int style = PS_SOLID</b>\n\n
	The pen style to use. One of the following types may be used. See the Win32
	API Reference for more information on pen styles.\n\n
	<b>PS_SOLID</b>\n\n
	<b>PS_DASH</b>\n\n
	<b>PS_DOT</b>\n\n
	<b>PS_DASHDOT</b>\n\n
	<b>PS_DASHDOTDOT</b>\n\n
	<b>PS_NULL</b>\n\n
	<b>PS_INSIDEFRAME</b> */
	virtual void  SetPenProperty(COLORREF color, int width = 0, int style = PS_SOLID)=0; // Sets the pen properties of a curve 
	/*! \remarks Retrieves the color, width and style of a curve.
	\par Parameters:
	<b>COLORREF \&color</b>\n\n
	The color in use.\n\n
	<b>int \&width</b>\n\n
	The width in use.\n\n
	<b>int \&style</b>\n\n
	The style in use. One of the following types:\n\n
	<b>PS_SOLID</b>\n\n
	<b>PS_DASH</b>\n\n
	<b>PS_DOT</b>\n\n
	<b>PS_DASHDOT</b>\n\n
	<b>PS_DASHDOTDOT</b>\n\n
	<b>PS_NULL</b>\n\n
	<b>PS_INSIDEFRAME</b> */
	virtual void  GetPenProperty(COLORREF &color, int &width, int &style)=0;				 // Gets the color of a curve
	/*! \remarks Sets the pen properties of a curve if it is disabled.
	\par Parameters:
	<b>COLORREF color</b>\n\n
	The color to set.\n\n
	<b>int width = 0</b>\n\n
	The width to set.\n\n
	<b>int style = PS_SOLID</b>\n\n
	The style to set. One of the following types:\n\n
	<b>PS_SOLID</b>\n\n
	<b>PS_DASH</b>\n\n
	<b>PS_DOT</b>\n\n
	<b>PS_DASHDOT</b>\n\n
	<b>PS_DASHDOTDOT</b>\n\n
	<b>PS_NULL</b>\n\n
	<b>PS_INSIDEFRAME</b> */
	virtual void  SetDisabledPenProperty(COLORREF color, int width = 0, int style = PS_SOLID)=0; // Sets the pen properties of a curve if it is disabled
	/*! \remarks Retrieves the color of a curve if it is disabled.
	\par Parameters:
	<b>COLORREF \&color</b>\n\n
	The color in use.\n\n
	<b>int \&width</b>\n\n
	The width in use.\n\n
	<b>int \&style</b>\n\n
	The style in use. One of the following types:\n\n
	<b>PS_SOLID</b>\n\n
	<b>PS_DASH</b>\n\n
	<b>PS_DOT</b>\n\n
	<b>PS_DASHDOT</b>\n\n
	<b>PS_DASHDOTDOT</b>\n\n
	<b>PS_NULL</b>\n\n
	<b>PS_INSIDEFRAME</b> */
	virtual void  GetDisabledPenProperty(COLORREF &color, int &width, int &style)=0;				 // Gets the color of a curve if it is disabled
	/*! \remarks Returns the Y-value for a given X-Value of the curve. Note
	that values outside the X-range are extrapolated from the curve using a
	straight line based on the tangents of the first or last point.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the value.\n\n
	<b>float fX</b>\n\n
	The input X value.\n\n
	<b>Interval \&ivalid = FOREVER</b>\n\n
	The validity interval which is updated by this method to reflect the
	validity of the curve.\n\n
	<b>BOOL UseLookupTable = FALSE</b>\n\n
	If TRUE a lookup table is used to get the value (for speed). If FALSE the
	value is computed.\n\n
	This is used to speed up value access. The default value for the lookup
	table size is 1000. The lookup table will be calculated whenever this
	method is called with TRUE and the current lookup table is not initialized
	yet, or invalid (it will be invalidated, when a point or tangent is moved,
	or the time has changed in case it is animated). */
	virtual float GetValue(TimeValue t, float fX, Interval &ivalid = FOREVER, BOOL UseLookupTable = FALSE)=0;			 // Returns the Y-value for a given X-Value
	/*! \remarks Sets if the curve can be animated or not.
	\par Parameters:
	<b>BOOL Animated</b>\n\n
	TRUE if it can be animated; FALSE if it can't. */
	virtual void  SetCanBeAnimated(BOOL Animated)=0;
	/*! \remarks Returns TRUE if the curve can be animated; otherwise FALSE.
	*/
	virtual BOOL  GetCanBeAnimated()=0;

	/*! \remarks Returns nonzero if the specified point is animated; otherwise
	zero.
	\par Parameters:
	<b>int index</b>\n\n
	The zero based index of the curve to check. */
	virtual int   IsAnimated(int index)=0;
	
	/*! \remarks Returns the number of points in the curve. */
	virtual int   GetNumPts()=0;
	/*! \remarks Sets the number of points in the curve.
	\par Parameters:
	<b>int count</b>\n\n
	The number to set. */
	virtual void  SetNumPts(int count)=0;
	/*! \remarks Returns a BitArray which contains the selected / de-selected
	state for each point. The BitArray is <b>GetNumPts()</b> in size where the
	0-th bit corresponds to the 0-th point. */
	virtual	BitArray GetSelectedPts()=0;
	/*! \remarks Sets the selected state of the points in the curve using the
	BitArray passed. Bits which are are affected as specified by the flags.
	\par Parameters:
	<b>BitArray \&sel</b>\n\n
	Specifies which points are affected. The 0-th bit corresponds to the 0-th
	point.\n\n
	<b>int flags</b>\n\n
	One or more of the following values:\n\n
	<b>SELPTS_SELECT</b>\n\n
	Select the points.\n\n
	<b>SELPTS_DESELECT</b>\n\n
	De-select the points.\n\n
	<b>SELPTS_CLEARPTS</b>\n\n
	If this bit is set the method de-selects (clears) all the points before
	performing the operation as specified by the flags above. */
	virtual	void  SetSelectedPts(BitArray &sel, int flags)=0;

	/*! \remarks Sets the specified point at the time passed.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the point.\n\n
	<b>int index</b>\n\n
	The zero based index of the point in the curve.\n\n
	<b>CurvePoint *point</b>\n\n
	Points to the curve point to set.\n\n
	<b>BOOL CheckConstraints = TRUE</b>\n\n
	When you're setting a point, it checks the in and outtan handles to prevent
	them from going beyond the previous or next point's location, since that
	would create an invalid curve. Thus the Curve Control will adjust the
	tangents due to the constraints. However, if you set the first point, there
	is no next point, to check the tangent location against, thus you have to
	have a possibility to turn CheckConstraints off, so the constraints won't
	be checked (developers have to make sure that they are inserting valid
	points/tangents. )\n\n
	<b>BOOL notify = TRUE</b>\n\n
	This parameter is available in release 4.0 and later only.\n\n
	This allows developers to control whether the command sends windows
	messages or not. When set to FALSE windows messages are not sent to the
	message handler. This lets developers constrain points when the user
	attempts to move them without getting into a message loop. When TRUE
	messages are sent. */
	virtual	void  SetPoint(TimeValue t, int index, CurvePoint *point, BOOL CheckConstraints = TRUE, BOOL notify = TRUE)=0;
	/*! \remarks Retrieves data about the specfied point and updates the
	validity interval to reflect the validity of the point's controller.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the data.\n\n
	<b>int index</b>\n\n
	The zero based index of the point.\n\n
	<b>Interval \&valid = FOREVER</b>\n\n
	The validity interval which is updated. */
	virtual CurvePoint	GetPoint(TimeValue t, int index, Interval &valid = FOREVER)=0;

	//Added by AF (10/26/00)
	//Currently only supports CURVE_EXTRAPOLATE_LINEAR and CURVE_EXTRAPOLATE_CONSTANT
	/*! \remarks	This method allows you to set the out of range type.
	\par Parameters:
	<b>int type</b>\n\n
	Currently these types are supported;\n\n
	<b>CURVE_EXTRAPOLATE_LINEAR</b>\n\n
	<b>CURVE_EXTRAPOLATE_CONSTANT</b> */
	virtual void  SetOutOfRangeType(int type)=0;
	/*! \remarks	This method returns the out of range type.
	\return  One of the following; <b>CURVE_EXTRAPOLATE_LINEAR,
	CURVE_EXTRAPOLATE_CONSTANT</b> */
	virtual int   GetOutOfRangeType()=0;
	
	/*! \remarks Inserts the specified point at the location passed.
	\par Parameters:
	<b>int where</b>\n\n
	This value becomes the new index of the point.\n\n
	<b>CurvePoint \&p</b>\n\n
	The point to insert.
	\return  Nonzero if the point was inserted; otherwise zero. */
	virtual int	  Insert(int where, CurvePoint & p)=0;
//watje
//this is identical to the Insert above but allows you to turn off/on the hold that occurs.
//this is useful when you are doing interactive inserts and moves from code, the original Insert hold
//would often get in the way
	/*! \remarks	This is identical to the Insert above but allows you to turn off/on the
	hold that occurs.this is useful when you are doing interactive inserts and
	moves from code, the original Insert hold would often get in the way
	\par Parameters:
	<b>int where</b>\n\n
	This value becomes the new index of the point.\n\n
	<b>CurvePoint \&p</b>\n\n
	The point to insert.\n\n
	<b>BOOL do_not_hold</b>\n\n
	TRUE in order not to hold; otherwise FALSE.
	\return  Nonzero if the point was inserted; otherwise zero. */
	virtual int	  Insert(int where, CurvePoint& p, BOOL do_not_hold)=0;


	/*! \remarks Deletes the point whose index is passed.
	\par Parameters:
	<b>int index</b>\n\n
	The zero based index of the point to delete. */
	virtual void  Delete(int index)=0;

	/*! \remarks This method sets the size of the Curve Control lookup table.
	The lookup table allows users of the Curve Control to easily speed up their
	value access. The default value for the lookup table size is 1000. The
	lookup table will be calculated whenever <b>GetValue()</b> is called with
	the parameter <b>UseLookupTable==TRUE</b> and the current LookupTable is
	not initialized yet, or invalid (it will be invalidated, when a point or
	tangent is moved, or the time has changed in case it is animated).
	\par Parameters:
	<b>int size</b>\n\n
	The size to set. */
	virtual void  SetLookupTableSize(int size)=0;
	/*! \remarks Returns the current size of the lookup table. See
	<b>SetLookupTableSize()</b> above. */
	virtual int   GetLookupTableSize()=0;
};

#pragma warning(pop)
/*! \sa  Class ICurveCtl.\n\n
class ResourceMakerCallback\n\n

\par Description:
This class is available in release 3.0 and later only.\n\n
This callback object may be used to handle custom bitmaps and tooltips for the
display buttons of a curve control. There are also methods which get called to
handle the situations when a curve is reset or a new curve is created.\n\n
All methods of this class are virtual.  */
class ResourceMakerCallback: public MaxHeapOperators
{
public:
	/*! \remarks This method is called to set the image list.\n\n
	This callback is used to update the HIMAGELIST to handle custom bitmaps on
	the display buttons. The image list has to have 2*NumCurves 16x15 or 16x16 bitmaps.
	The format for the first set of images is for Out\&In Enabled. The second
	set is for Out\&In Disabled (which are not yet used).
	\par Parameters:
	<b>HIMAGELIST \&hCTools</b>\n\n
	A reference to the image list to set. An HIMAGELIST is a Win32 data type
	that is a handle to an image list.\n\n
	<b>ICurveCtl *pCCtl</b>\n\n
	This pointer can be used to determine which ICurveCtl calls the callback,
	in case the plugin uses many CurveControls and want to set different
	bitmaps for different CurveControls.
	\return  If the image list was assigned the callback should return TRUE. If
	it returns FALSE, the default bitmaps will be used.
	\par Default Implementation:
	<b>{return FALSE;}</b> */
	#pragma warning(push)
	#pragma warning(disable:4100)
	virtual BOOL SetCustomImageList(HIMAGELIST& hCTools, ICurveCtl* pCCtl){return FALSE;} 
	
	// This callback allows the developer to assign custom ToolTips to the display buttons. He simply has to assing a 
	// MSTR to the ToolTip parameter in regards to the button number. The pCCtl pointer can be used to determine 
	// which ICurveCtl calls the callback, in case the plugin uses many CurveControls and want to set different 
	// Tooltips for different CurveControls

	/*! \remarks This callback allows the developer to assign custom ToolTips
	to the display buttons. One simply has to assing a string to the ToolTip
	parameter for the specified button number.
	\par Parameters:
	<b>int iButton</b>\n\n
	The zero based index of the button whose tool tip text to retrieve.\n\n
	<b>MSTR \&ToolTip</b>\n\n
	The string for the tool tip text.\n\n
	<b>ICurveCtl *pCCtl</b>\n\n
	This pointer can be used to determine which ICurveCtl calls the callback,
	in case the plugin uses many CurveControls and want to set different
	Tooltips for different CurveControls.
	\return  TRUE if the method is implemented to use custom tooltips;
	otherwise FALSE.
	\par Default Implementation:
	<b>{return FALSE;}</b> */
	virtual BOOL GetToolTip(int iButton, MSTR &ToolTip,ICurveCtl *pCCtl){return FALSE;}

	/*! \remarks This methods gets called when the user preses the Reset
	button (cross symbol) in the user interface (if the control is using the
	upper toolbar).
	\par Parameters:
	<b>int curvenum</b>\n\n
	The zero based index of the curve.\n\n
	<b>ICurveCtl *pCCtl</b>\n\n
	Points to the interface for the custom curve control.
	\par Default Implementation:
	<b>{}</b> */
	virtual void ResetCallback(int curvenum, ICurveCtl *pCCtl){}
	/*! \remarks This method gets called after the curve control creates a new
	curve. The developer can set the default values here for the new curve.
	This call will be a result of a call to <b>SetNumCurves(i)</b> where the
	new size is bigger than the old size.
	\par Parameters:
	<b>int curvenum</b>\n\n
	The zero based index of the curve.\n\n
	<b>ICurveCtl *pCCtl</b>\n\n
	Points to the interface for the custom curve control.
	\par Default Implementation:
	<b>{}</b> */
	virtual void NewCurveCreatedCallback(int curvenum, ICurveCtl *pCCtl){}
	
	virtual void* GetInterface(ULONG id) {return NULL;}
	#pragma warning(pop)
};

