/**********************************************************************
 *<
    FILE: MouseProc.h

    DESCRIPTION:  Declares DataEntryMouseProc class

    CREATED BY: Scott Morrison

    HISTORY: created 7 December, 1998

 *> Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "mouseman.h"
#include "point3.h"
#include "matrix3.h"
#include "maxapi.h"

// forward declarations
class Object;
class ViewExp;

/*! \sa  Class MouseCallBack, Class ViewExp, Class Object, Class Interface, Class Point3, Class Matrtix3, Class CreateMouseCallBack.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This mouse proc allows drawing in multiple viewports, offsetting from the
contruction plane, and orthogonal and angle snapping. This allows developers to
support orthogonal snapping and angle snapping on creation (as the Bezier line
tool does). If the user presses Shift while dragging the mouse, the point is
snapped to the nearest quadrant (ortho snapping). If the Alt key is held, the
point is snapped using the setting of the angle snap system.\n\n
The typical control flow of this class is that the <b>OnPointSelected()</b>
method is called every time the user clicks in the viewport, and
<b>OnMouseAbort()</b> is called when the user right clicks to finish the curve.
<b>RemoveLastPoint()</b> is called when backspace is pressed, and
<b>OnMouseMove(Point3\& p)</b> is called every time the mouse moves (this lets
the developer update the curve continuously).\n\n
This class is a sub-class of MouseCallBack, but it can also be used as a
CreateMouseCallBack to create curves from the creation panel. To do this you
embed a DataEntryMouseProc in a CreateMouseCallBack as show below. Notice the
implementation of the virtual member StartNewCreation(). This is a new virtual
method on CreateMouseCallBack that tells the system whether the mouse proc is
in a state ready to create a new object. This was required, becase the "proc"
function now always returns "CREATE_STOP" in order to implement multi-viewport
input.\n\n
\code
class TopCVCurveCreateMouseProc : public Em3DDataEntryMouseProc
{
	TopCVCurveCreateMouseProc() :
	Em3DDataEntryMouseProc() {}
	virtual BOOL OnPointSelected();
	virtual void OnMouseMove(Point3& p);
	virtual BOOL AllowAnyViewport();
	virtual void RemoveLastPoint();
	virtual int OnMouseAbort();
	virtual BOOL PerformRedraw() { return FALSE; }
	void SetObj(EditableCVCurve* o) { mpOb = o; }
	virtual BOOL StartNewCreation() { return mMouseClick == 0; }
	friend class EditableCVCurve;
	private:
		EditableCVCurve * mpOb;
};
class EditableCVCurveCreateCallBack: public CreateMouseCallBack
{
	EditableCVCurveCreateCallBack() {}
	virtual int proc( ViewExp* vpt,int msg, int point, int flags, IPoint2 m,
		Matrix3& mat );
	friend class CVBackspaceUser;
	friend class EditableCVCurve;
	virtual BOOL StartNewCreation() {
		return mMouseProc.StartNewCreation();
	}
	private:
		void RemoveLastPoint();
		TopCVCurveCreateMouseProc mMouseProc;
};
int EditableCVCurveCreateCallBack::proc(ViewExp* vpt,int msg, int point, int
flags, IPoint2 m, Matrix3& mat)
{
	spTransformMat = &mat;
	return mMouseProc.proc(vpt->GetHWnd(), msg, point, flags, m);
}

static EditableCVCurveCreateCallBack nsCreateCB;
CreateMouseCallBack* EditableCVCurve::GetCreateMouseCallBack()
{
	nsCreateCB.mMouseProc.SetObj(this);
	nsCreateCB.mMouseProc.SetParams(hInstance, mpEM, 0);
	return(&nsCreateCB);
}
\endcode 
\par Data Members:
protected:\n\n
<b>Object* mpObject;</b>\n\n
This a pointer to the object that is using the mouse proc.\n\n
<b>int mMouseClick;</b>\n\n
The number of clicks (i.e. selected points) the user has entered in creating
this object. It is like the "point" parameter to "proc".\n\n
<b>Tab\<Point3\> mPoints;</b>\n\n
These are the 3D values of the points the user has selected.\n\n
<b>Tab\<IPoint2\> mClickPoints;</b>\n\n
These are the 2D viewport coordinates the user selected.\n\n
<b>BOOL mLiftOffCP;</b>\n\n
TRUE when in the mode where we lift off the construction plane  */
class DataEntryMouseProc : public MouseCallBack {
public:
    /*! \remarks Constructor. The data members are initialized as follows:\n\n
    <b>mpObject = pObj;</b>\n\n
    <b>mCursor = cursor;</b>\n\n
    <b>mInstance = hInst;</b>\n\n
    <b>mMouseClick = 0;</b>\n\n
    <b>mDoNotDouble = TRUE;</b>\n\n
    <b>mLiftOffCP = FALSE;</b>\n\n
    <b>mPreviousFlags = 0;</b> */
    CoreExport DataEntryMouseProc(Object* pObj, int cursor, HINSTANCE hInst);

    /*! \remarks Constructor. The data members are initialized as follows:\n\n
    <b>mpObject = NULL;</b>\n\n
    <b>mpIp = NULL;</b>\n\n
    <b>mMouseClick = 0;</b>\n\n
    <b>mDoNotDouble = TRUE;</b>\n\n
    <b>mCursor = 0;</b>\n\n
    <b>mLiftOffCP = FALSE;</b>\n\n
    <b>mPreviousFlags = 0;</b>\n\n
    <b>mInstance = 0;</b> */
    CoreExport DataEntryMouseProc();

    // Called when a point is selected
    /*! \remarks This method is called every time the user clicks in the viewport to enter
    data. This is the method in NURBS curves, for example, that adds a new CV
    or point to the curve. The method can query the <b>mMouseClick</b> member
    to see which point this is in the sequence (like the "point" parameter to
    traditional MouseCallback classes), and the 3D value of the point can be
    determined from <b>mPoints[mMouseClick]</b>. The data member <b>mPoints</b>
    contains all the 3D points selected, and <b>mClickPoints</b> is a table of
    the 2d points where the user clicked in the viewport.
    \return  The return value is used to determine whether the creation should
    continue or not. If it returns TRUE, more points are selected. If it
    returns FALSE, then the creation is done. In the case of NURBS, this is
    used to implement the feature that asks users if they want to close a curve
    when they click on the same point where they started the curve. If the
    answer is yes, this method returns FALSE, otherwise it always return TRUE.
    \par Default Implementation:
    <b>{return TRUE; }</b> */
    CoreExport virtual BOOL OnPointSelected()      {return TRUE; }
    // Called on every mouse move event
    /*! \remarks This method is called on every mouse move event.
    \par Parameters:
    <b>Point3\& p</b>\n\n
    The current point in world space of the mouse position.
    \par Default Implementation:
    <b>{}</b> */
    CoreExport virtual void OnMouseMove(Point3& p) { UNUSED_PARAM(p); }
    // Tells the system when to allow drawing in multiple viewports
    /*! \remarks This method tells the system when to allow drawing in multiple viewports.
    \return  TRUE to allow drawing between viewports; FALSE to not allow
    drawing between viewports.
    \par Default Implementation:
    <b>{ return TRUE; }</b> */
    CoreExport virtual BOOL AllowAnyViewport()     { return TRUE; }
    // Called when backspace is pressed
    /*! \remarks This method is called when the backspace key is pressed. Typically this
    deletes the last point entered by the user so they may correct its entry.
    \par Default Implementation:
    <b>{}</b> */
    CoreExport virtual void RemoveLastPoint()      {}
    // Called when the creation is finished
    /*! \remarks This method is called when the creation is finished.
    \return  Return one of the following value to indicate the state of the
    creation process:\n\n
    <b>CREATE_CONTINUE</b>\n\n
    The creation process should continue.\n\n
    <b>CREATE_STOP</b>\n\n
    The creation process has terminated normally.\n\n
    <b>CREATE_ABORT</b>\n\n
    The creation process has been aborted. The system will delete the created
    object and node.
    \par Default Implementation:
    <b>{ return CREATE_ABORT; }</b> */
    CoreExport virtual int OnMouseAbort()          { return CREATE_ABORT; }
    // Says whether the mouse proc should perform redraws
    // When used in a CreateMouseCallBack, this should return FALSE
    /*! \remarks This method indicates whether the mouse proc should perform redraws. When
    used in a <b>CreateMouseCallBack</b>, this should return FALSE.
    \return  TRUE to have the mouse proc perform redraws; otherwise FALSE.
    \par Default Implementation:
    <b>{ return TRUE; }</b> */
    CoreExport virtual BOOL PerformRedraw()        { return TRUE; }

    // These methods need to be implemented to get the offset line drawn

    // Tells the object to draw offset lines
    /*! \remarks This method is called to tell the object to draw offset lines. This is
    called passing TRUE when the system enters the mode where points are lifted
    off the construction plane. It is telling the object that it needs to draw
    a line between the points supplied by <b>SetConstructionLine(int i, Point3
    p)</b>. It is called passing FALSE when the offset procedure is
    complete.\n\n
    To see an example of how this is used, create a NURBS Point curve, and
    press the Control key while laying down a point. It enters a mode that lets
    you lift the point off the construction plane, and draws a red dotted line
    back to the CP to give some visual feedback.
    \par Parameters:
    <b>BOOL useLine</b>\n\n
    TRUE if the mode is beginning; FALSE if it is ending. */
    CoreExport virtual void SetUseConstructionLine(BOOL useLine) = 0;
    // Sets the endpoints of the line (0 is start, 1 is end)
    /*! \remarks These methods need to be implemented to get the offset line
    drawn\n\n
    This method is called with i==0 for the start point and with i==1 for the
    end point.
    \par Parameters:
    <b>int i</b>\n\n
    The point index: 0 for the start or 1 for the end.\n\n
    <b>Point3 p</b>\n\n
    The point to draw to or from. */
    CoreExport virtual void SetConstructionLine(int i, Point3 p) = 0;
    
    // The mouse callback function
    /*! \remarks Implemented by the System.\n\n
    This is the method where the developer defines the user / mouse interaction
    that takes place.
    \par Parameters:
    <b>HWND hwnd</b>\n\n
    The window handle of the window the user clicked in. This is one of the
    viewports.\n\n
    <b>int msg</b>\n\n
    This message describes the type of event that occurred. See
    <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_mouse_callback_messages.html">List of Mouse Callback
    Messages</a>.\n\n
    <b>int point</b>\n\n
    The point number. This is 0 for the first click, 1 for the second, etc.\n\n
    <b>int flags</b>\n\n
    These flags describe the state of the mouse buttons. See
    <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_mouse_callback_flags.html">List of Mouse Callback
    Flags</a>.\n\n
    <b>IPoint2 m</b>\n\n
    The 2D screen point that the user clicked on.
    \return  <b>CREATE_STOP</b> is returned.\n\n
    Note: Notice the implementation of the virtual member
    <b>StartNewCreation()</b>. This is a virtual method on
    <b>CreateMouseCallBack</b> that tells the system whether the mouse proc is
    in a state ready to creat a new object. This was required, becase this
    method now always returns <b>CREATE_STOP</b> in order to implement the
    multi-viewport input */
    CoreExport int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m );
    
    friend class DataEntryBackspaceUser;
    
    /*! \remarks Implemented by the System.\n\n
    This method clears the creation parameters. The data members are reset as
    follows:\n\n
    <b>mMouseClick = 0;</b>\n\n
    <b>mPoints.SetCount(0);</b>\n\n
    <b>mClickPoints.SetCount(0);</b>\n\n
    <b>mLiftOffCP = FALSE;</b>\n\n
    <b>mPreviousFlags = 0;</b> */
    CoreExport void ClearCreationParams();
    
    /*! \remarks Implemented by the System.\n\n
    This method sets the parameters as follows:\n\n
    <b>mpObject = pObj;</b>\n\n
    <b>mCursor = cursor;</b>\n\n
    <b>mInstance = hInst;</b> */
    CoreExport void SetParams(HINSTANCE hInst, Object* pObj, int cursor);
    
private:
    Point3 GetPoint(IPoint2 m, HWND hWnd, ViewExp* pVpt);
    void SetOffsetBase(IPoint2 m, HWND hWnd, ViewExp* pVpt);
    BOOL GetNodeTM(Matrix3& tm);
    IPoint2 ProjectPointToViewport(ViewExp *pVpt, Point3 fp);
    IPoint2 AngleSnapPoint(Point3 in3, ViewExp* pVpt);

    // The inverse of the transform on the node (or viewport transform,
    // for creation mouse procs)
    Matrix3        mTM;

    // Indicates when to ignore upclicks close to down clicks
    BOOL           mDoNotDouble;
    // The resource id of the cursor to use
    int            mCursor;
    // The instance of the dll using the mouse proc
    HINSTANCE      mInstance;

    // State for off-construction plane creation
    Point3         mSnappedPoint;
    Matrix3        mOriginalViewTM;

    int            mPreviousFlags;

protected:
    // The object using the mouse proc
    Object*        mpObject;

    // The number of points selected so far.
    int            mMouseClick;
    // The 3D coordinates of the points, in the local coordinate system
    Tab<Point3>    mPoints;
    // The 2D points the user selected in the viewport.
    Tab<IPoint2>   mClickPoints;
    // TRUE when in the mode where we lift off the construction plane
    BOOL           mLiftOffCP;
    // The last window we had an event in
    HWND           mHwnd;
    IPoint2        mLastMovePoint;
};
