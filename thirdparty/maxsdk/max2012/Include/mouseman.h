//
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.  
//

#pragma once

#include "coreexp.h"
#include "baseinterface.h"
#include "ipoint2.h"

// forward declarations
class MouseManager;

/** Allows the developer to capture and process the mouse events entered by
 * the user. To create a mouse call back, derive a sub class of
 * this class and implement the proc() function.  
 * \see CommandMode */
class MouseCallBack : public BaseInterfaceServer {
		MouseManager * mouseMan;
	public:
		virtual ~MouseCallBack() {}

		/** Called to handle the mouse event processing whenever a mouse event happens.
		 * \param hwnd The window handle of the window in which the user has clicked. It is 
		 * one of the viewports to which an interface can be obtained from the system.
		 * To get such interface, the window handle should be passes to the functions 
		 * Interface::GetViewport() as the input. Developers should call
		 * Interface::ReleaseViewport() once they are done with the viewport interface.
		 * \param msg This message describes the type of event that occurred. See
		 * \ref Mouse_Call_Back_Flags.
		 * \param point The point number. this is 0 for the first click, 1 for the second,	etc.
		 * \param flags These flags describe the state of the mouse buttons. See \ref 
		 * Mouse_Call_Back_Flags.
		 * \param m The 2D screen point on which that the user has clicked. Methods in the viewport
		 * interface allow this point to be converted into a world space ray or a
		 * 3D view space point. A world space ray can be intersected with the
		 * active construction plane which results in a point on the active
		 * construction plane. See Class ViewExp.
		 * \return  TRUE, indicating the proc should continue to process points;
		 * or FALSE, meaning to stop processing points. If a plug-in supplied a large
		 * number of points in a command mode that uses this mouse proc, FALSE can
		 * be returned to abort the processing before all the points have been
		 * entered. */
		CoreExport virtual int proc( 
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
		virtual void pan(IPoint2 offset) { UNUSED_PARAM(offset); }

		/** Override the default drag mode. Most plug-ins will not need to 
		 * replace the default implementation of this method.
		 * This function changes the way the messages are sent based on the sequence of mouse 
		 * inputs by the user.
		 * The normal flow of the messages is described as follows: First the user clicks the 
		 * mouse button and a MOUSE_POINT message is generated. The user then drags the
		 * mouse with the button down, resulting in a series of MOUSE_MOVE messages
		 * to be sent. When the user releases the mouse button, a MOUSE_POINT
		 * messages is generated. Then the mouse is moved again, resulting in a new series of
		 * MOUSE_MOVE messages to be sent again. Unlike the first time, when the user 
		 * clicks the mouse button here, a point message is NOT generated at this point until the button
		 * is released. All future points are then only sent after the mouse
		 * button has been pressed and released. The default implementation is 
		 * { return mode; }, and a sample program using the override method 
		 * (using CLICK_DOWN_POINT) can be found in 
		   /MAXSDK/SAMPLES/OBJECTS/SPLINE.CPP. 
		 * \param mode The current drag mode from the list \ref Mouse_drag_modes. 
		 * \return  The given mouse drag mode.
		 */ 
		virtual int override(int mode) { return mode; }		

		/** \internal This method is used internally. */
		void setMouseManager(MouseManager *mm)  { mouseMan = mm; }

		/** \internal This method is used internally. */
		MouseManager *getMouseManager()  { return mouseMan; }

		/**	Transform Gizmo Interface. Returns TRUE if the mouse proc 
		 * supports a transform gizmo, FALSE if not.
		 * This method and DeactivateTransformGizmo() are normally
		 * implemented by the selection processor and the existing mouse procs.
		 * However, special implementation is possible as well. For that,
		 * because the transform gizmo depends on the Command Mode, the
		 * MouseCallback itself decides if it supports the use of the transform
		 * gizmo or not. When the node axis (or transform gizmo) is redrawn, the system
		 * will ask the command mode's mouse proc whether or not it supports transform gizmos.
		 * If it does, it will draw a gizmo instead of the regular node axis.
		 * The same procedure happens with the main selection processor in 3ds Max.
		 * When the mouse is moved, the selection processor itself asks if the
		 * MouseCallback supports transform gizmos or not. If so, it will hit
		 * test the gizmo in a MOUSE_FREEMOVE or MOUSE_POINT message. If any of 
		 * the transform gizmos hit test flags are passed into
		 * the mouse procs hit tester, the transform gizmo should be hit tested as
		 * well (using Interface::HitTestTransformGizmo()). The default Implementation
		 * of this function is {return FALSE;}.
		 * When hit testing the gizmo, different flags will be passed in:
		 * \li HIT_TRANSFORMGIZMO is passed in on a MOUSE_FREEMOVE
		 * message so that the axis is hit tested and it highlights if it is hit,
		 * but it doesn't actually switch the transform mode.
		 * \li In case of a MOUSE_POINT, the flag will be HIT_SWITCH_GIZMO, 
		 * and if the axis is hit, the 'hit' transform mode will be pushed on the 
		 * transform mode stack.
		 * \li When the mouse is released (MOUSE_POINT, pt==1 or MOUSE_ABORT), 
		 * then the axis constraint should pop back to the
		 * existing one, and DeactivateTransformGizmo() is called. Inside
		 * DeactivateTransformGizmo() the normal implementation is to pop
		 * the axis mode back. It should also maintain a flag (set it if
		 * HitTestTransformGizmo() returns TRUE and the HIT_SWITCH_GIZMO is set, 
		 * and clear it in DeactivateTransformGizmo()). The flag is needed because you
		 * should not call Interface::PopAxisMode() unless the axis mode
		 * was previously popped. */
		virtual BOOL SupportTransformGizmo() { return FALSE; }

		/** Deactivates the transform gizmo. See the note
		 * in SupportTransformGizmo(). */
		virtual void DeactivateTransformGizmo() {}
		// End of Transform Gizmo Interface

		/** Returns FALSE by default, and needs to be overridden by the mouse procs 
		 * wishing to utilize the AutoGrid feature. If overridden, it should return TRUE
		 * and also make the appropriate calls to the ViewExp::TrackImpliciGrid(),
		 * ViewExp::CommitImplicitGrid() and ViewExp::ReleaseImplicitGrid() from the
		 * body of their classes proc() method. For sample code see
		 * /MAXSDK/SAMPLES/MODIFIERS/SURFWRAP/SURFWRAP.CPP. */
		virtual BOOL SupportAutoGrid(){return FALSE;}

		/** Returns FALSE by default. Should return TRUE if Ortho Mode makes sense 
		 * for this creation, FALSE otherwise. In general it returns TRUE only  
		 * for splines and such. */
		virtual BOOL TolerateOrthoMode(){return FALSE; }
	};


/// \defgroup Mouse_Callback_Messages Mouse Callback Messages
//@{
/** Sent when the user aborts a mouse procedure. 
An examle is when the user is dragging the mouse and he right clicks. */
#define MOUSE_ABORT 		0
/** \internal This message is used internally. */
#define MOUSE_IDLE			0	
/** Sent when the user has clicked a point.*/
#define MOUSE_POINT			1	
/** Sent when the mouse input is captured and the user moved the mouse. 
When mouse input is captured all mouse events continue to go to the 
current window even when the mouse is move outside the limits of the 
window. This is when the user is 'dragging'. */
#define MOUSE_MOVE			2
/** sent when the user has double clicked the mouse.*/
#define MOUSE_DBLCLICK		3
/** Sent when the mouse proc is plugged in as the current mouse proc. 
If a plug-in needed to perform some kind of initialization when it was 
first became current this message could be processed.*/
#define MOUSE_INIT			4
/** sent when the mouse proc is un-plugged as the current mouse proc.*/
#define MOUSE_UNINIT		5
/** Similar to a MOUSE_MOVE message except it is not called when 
the mouse is in a 'drag' session. This means that mouse input is 
not captured. If mouse input is not captured and the mouse is moved 
outside the current window, the current window will no longer 
receive the mouse messages.*/
#define MOUSE_FREEMOVE		6
/** \deprecated This is not used. Keyboard input is processed by 
 * registering an accelerator table. See Class Interface for the 
 * methods to register a keyboard accelerator. Also see the 
 * section Keyboard Accelerators and Dialog Messages.*/
#define MOUSE_KEYBOARD		7
 /** Sent on a right click, when nothing is selected, and the user 
  * is not over any selectable object. For example, this is how 
  * the unfreeze-by-hit pick mode knows to abort if the user presses 
  * the right mouse button. Note that this is different than if you 
  * right click while dragging - in that case you get a MOUSE_ABORT message.*/
#define MOUSE_PROPCLICK		8
#define MOUSE_SNAPCLICK		9
//@}

/// \defgroup Mouse_drag_modes Mouse Drag Modes
//@{
#define CLICK_MODE_DEFAULT	0	//!< Returned by CreateMouseCallBack to indicate use of system mouse mode
#define CLICK_DRAG_CLICK	1	//!< The default behaviour as described in override()
/** In this mode, the first point is entered by clicking the mouse button
* down and then letting it up. This generates point 0. In other words, a
* MOUSE_POINT message is only generated after the user has pressed
* and released the mouse button. */
#define CLICK_MOVE_CLICK	2
#define CLICK_DOWN_POINT	3	//!< In this mode, point messages are sent on mouse-down only.
//@}


/// \defgroup Button_Constants Button Constants
//@{
#define LEFT_BUTTON			0	//!< Left button
#define MIDDLE_BUTTON		1	//!< Middle button
#define RIGHT_BUTTON		2	//!< Right button
//@}

/// \defgroup Mouse_Call_Back_Flags Mouse Call Back Flags
//@{
#define MOUSE_SHIFT			(1<<0)	//!< Indicates the Shift key is pressed.
#define MOUSE_CTRL			(1<<1)	//!< Indicates the Ctrl key is pressed.
#define MOUSE_ALT			(1<<2)	//!< Indicates the Alt key is pressed.
#define MOUSE_LBUTTON		(1<<3)	//!< Indicates the Left button is down.
#define MOUSE_MBUTTON		(1<<4)	//!< Indicates the Middle button is down.
#define MOUSE_RBUTTON		(1<<5)	//!< Indicates the Right button is down.
//@}

class MouseOverride;

// aszabo | Nov.14.2000
// Although this class is not supposed to be instantiated directly
// by 3rd party code, there's code in the Modifier samples that 
// uses it directly rather then through MouseManager* Interface::GetMouseManager()
//
/** \internal Represents the interface to the mouse manager and handling system
 * and is used internally only. This class should not be instanced directly into 
 * plugin code. To retrieve a pointer to the mouse manager you can use the method 
 * Interface::GetMouseManager(). 
 * \see BaseInterfaceServer, MouseCallBack, Interface*/
class MouseManager : public BaseInterfaceServer {
	private:
		static int clickDragMode;
		
		int 			mouseMode;
		int 			curPoint;
		int         	curButton;
		MouseCallBack 	*TheMouseProc[3];
		int 			numPoints[3];
		int				buttonState[3];
		int				mouseProcReplaced;
		int 			inMouseProc;
#ifdef _OSNAP
		UINT			m_msg;
#endif
		HWND			captureWnd;

    // This wonds a callback that plug-ins can register to get the raw
    // windows messages for the mouse and keyboard for a viewport.
        WNDPROC mpMouseWindProc;

	public:

		friend class MouseManagerStateInterface;
		friend class MouseOverride; //internal
		// Constructor/Destructor
		CoreExport MouseManager();
		CoreExport ~MouseManager();

		CoreExport int SetMouseProc( MouseCallBack *mproc, int button, int numPoints=2 );
		CoreExport int SetDragMode( int mode );
		CoreExport int GetDragMode( );
		CoreExport int SetNumPoints( int numPoints, int button );
		CoreExport int ButtonFlags();
		CoreExport void Pan(IPoint2 p);
		CoreExport LRESULT CALLBACK MouseWinProc( 
			HWND hwnd, 
			UINT message, 
			WPARAM wParam, 
			LPARAM lParam );
		
		// RB 4-3-96: Resets back to the MOUSE_IDLE state
		CoreExport void Reset();
		int GetMouseMode() {return mouseMode;}
#ifdef _OSNAP
		UINT GetMouseMsg() {return m_msg;}
		int GetMousePoint() {return curPoint;}
#endif

		CoreExport void SetCapture(HWND hWnd);
		CoreExport HWND HasCapture();
		CoreExport void ReleaseCapture();
		CoreExport void RestoreCapture();

        CoreExport void SetMouseWindProcCallback(WNDPROC pMouseWindProc)
        { mpMouseWindProc = pMouseWindProc; }
// The following member been added
// in 3ds max 4.2.  If your plugin utilizes this new
// mechanism, be sure that your clients are aware that they
// must run your plugin with 3ds max version 4.2 or higher.
		WNDPROC GetMouseWindProcCallback() const
        { return mpMouseWindProc; }
// End of 3ds max 4.2 Extension
	};


#define WM_MOUSEABORT	(WM_USER + 7834)

/** Indicates if any mouse proc is currently in the process of
 * aborting a mouse proc. This function is not part of this class 
 * but is available for use. It returns TRUE if any mouse proc is 
 * currently in the process of aborting a mouse proc, FALSE otherwise.
 * For instance, a developer may be using the Animatable::MouseCycleCompleted()
 * method to put up a dialog box, but needs to not put it up if the 
 * mouse proc was aborted. This method provides a way to know if indeed 
 * the mouse proc is aborting so the dialog won't be displayed inside 
 * MouseCycleCompleted().
 * \return  TRUE if aborting, FALSE otherwise. */
CoreExport BOOL GetInMouseAbort();


