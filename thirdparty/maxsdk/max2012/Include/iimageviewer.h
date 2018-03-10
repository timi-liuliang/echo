/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: iImageViewer.h

	 DESCRIPTION: abstract class for image viewers

	 CREATED BY: michael malone (mjm)

	 HISTORY: created April 13, 2000

   	 Copyright (c) 2000, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */
#pragma once

#include "maxheap.h"
#include "maxtypes.h"
#include "bitmap.h"

// forward declarations
class DADMgr;
class CropCallback;

#ifdef IMAGE_VIEW_EXPORTS
#define IMAGE_VIEW_API __declspec(dllexport)
#else
#define IMAGE_VIEW_API __declspec(dllimport)
#endif

// sent by a docked bitmap viewer window to its parent when the user right clicks on dead area of the toolbar
// mouse points are relative to the client area of the schematic view window
//
// LOWORD(wParam) = mouse x
// HIWORD(wParam) = mouse y
// lparam         = bitmap viewer window handle
#define WM_BV_TOOLBAR_RIGHTCLICK	WM_USER + 0x8ac4

#define MAX_BITMAP_VIEWER_CLASS _M("MaxBitmapViewerClass") // windows class name


// aszabo | Nov.16.2000
// For the exported abstract classes in this header I am just adding
// a GetInterface method to them instead of deriving them from InterfaceServer
// in order to make it ready for extending it later with additional interfaces
class BaseInterface;

// -----------------------------------------------------------------------------
/*! \sa  Class DADMgr , Class BaseInterface, Class IMaxBitmapViewer\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents an abstract interface class for an image viewer.  */

class IMAGE_VIEW_API IImageViewer: public MaxHeapOperators
{
public:
	enum DisplayStyle { IV_FLOATING = 0, IV_DOCKED };

	enum WindowPosition
	{
		WPos_NULL = 0,
		WPos_UpperLeft,
		WPos_LowerLeft,
		WPos_UpperRight,
		WPos_LowerRight,
		WPos_Center,

		// the following are used for automatic save and restore
		WPos_Renderer = 10,
		WPos_VideoPostPrimary,
		WPos_VideoPostSecondary,
	};
	
	// -----------------------------------------------------------------------------
	//    Abstract class (Interface) for a callback that allows post-display access
	//    to an image viewer
	#pragma warning(push)
	#pragma warning(disable:4100)
	class PostDisplayCallback : public MaxHeapOperators
	{
	public:
		/*! \remarks The PostDisplayCB method of the PostDisplayCallback class is
		called after an image is displayed in the image viewer. Developers can use this
		to do any post-display related work.
		\par Parameters:
		<b>HWND hWnd</b>\n\n
		The handle of the image viewer window. */
		virtual void PostDisplayCB(HWND hWnd) = 0;

		// provides a way for extending it with interfaces
		/*! \remarks This method provides a way to extend the class with
		interfaces.
		\par Parameters:
		<b>Interface_ID id</b>\n\n
		The interface ID.
		\par Default Implementation:
		<b>{ return NULL; }</b> */

		virtual		BaseInterface* GetInterface	(Interface_ID id) { return NULL; }
	};


	// -----------------------------------------------------------------------------
	//    Abstract class (Interface) for a callback that allows pre-event handler
	//    access to an image viewer
	class PreEventHandlerCallback : public MaxHeapOperators
	{
	public:
		/*! \remarks The EventHanderCB method of the PreEventHandlerCallback class
		allows you to intercept window events prior to them being passed through.
		\par Parameters:
		<b>HWND hWnd</b>\n\n
		The handle of the image viewer window.\n\n
		<b>UINT message</b>\n\n
		The message identifier.\n\n
		<b>WPARAM wParam</b>\n\n
		The WPARAM value.\n\n
		<b>LPARAM lParam</b>\n\n
		The LPARAM value.\n\n
		<b>bool \&propagate</b>\n\n
		TRUE if the message and event are to be propagated, otherwise FALSE. */
		virtual LRESULT EventHandlerCB(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool &propagate) = 0;

		// provides a way for extending it with interfaces
		/*! \remarks This method provides a way to extend the class with interfaces.
		\par Parameters:
		<b>Interface_ID id</b>\n\n
		The interface ID.
		\par Default Implementation:
		<b>{ return NULL; }</b> */
		virtual		BaseInterface* GetInterface	(Interface_ID id) { return NULL; }
	};
	#pragma warning(pop)
	// shows/hides the viewer window
	/*! \remarks This method will show the image viewer. In the
	<b>IMaxBitmapViewer</b> class, the <b>Display()</b> method must initially
	be used to display the viewer; this method only displays the viewer after
	using <b>Hide()</b>. */
	virtual void Show() = 0;
	/*! \remarks This method will hide the image viewer. In the
	<b>IMaxBitmapViewer</b> class, the <b>UnDisplay()</b> method should be used
	to close the viewer when done; use <b>Hide()</b> to temporarily hide the
	window. */
	virtual void Hide() = 0;

	// gets the handle to display window
	/*! \remarks This method returns the handle to the display window of the
	image viewer. */
	virtual HWND GetHDisplayWindow() = 0;

	// sets the screen
	/*! \remarks This method allows you to set the position of the image
	viewer window.
	\par Parameters:
	<b>int x, y</b>\n\n
	The position of the window.\n\n
	<b>int w, h</b>\n\n
	The width and height of the window. */
	virtual void SetPos(int x, int y, int w, int h) = 0;

	// gets the display style
	/*! \remarks This method will return the display style used by the image
	viewer window, which is either <b>IV_FLOATING</b> or <b>IV_DOCKED</b>. */
	virtual DisplayStyle GetDisplayStyle() const = 0;

	// sets/gets the context help id
	/*! \remarks This method allows you to set the context help identifier for
	the image viewer.
	\par Parameters:
	<b>DWORD helpID</b>\n\n
	The help identifier. */
	virtual void SetContextHelpId(DWORD helpID) = 0;
	/*! \remarks This method returns the context help identifier. */
	virtual DWORD GetContextHelpId() const = 0;

	// sets/gets the DADMgr (allows override of DADMgr methods)
	/*! \remarks This method allows you to set the drag and drop manager which
	should be used for the image viewer.
	\par Parameters:
	<b>DADMgr *pDADMgr</b>\n\n
	A pointer to the drag and drop manager. */
	virtual void SetDADMgr(DADMgr *pDADMgr) = 0;
	virtual DADMgr *GetDADMgr() const = 0;

	// sets/gets the pre-event handler callback
	/*! \remarks This method allows you to set the pre-event handler callback
	method.
	\par Parameters:
	<b>PreEventHandlerCallback* pPreEventHandlerCB</b>\n\n
	A pointer to the callback function. */
	virtual void SetPreEventHandlerCallback(PreEventHandlerCallback* pPreEventHandlerCB) = 0;
	/*! \remarks This method returns a pointer to the pre-event handler
	callback function. */
	virtual PreEventHandlerCallback* GetPreEventHandlerCallback() const = 0;

	// sets/gets the post-display callback
	/*! \remarks This method allows you to set the post display callback
	function.
	\par Parameters:
	<b>PostDisplayCallback* pPostDisplayCB</b>\n\n
	A pointer to the callback function. */
	virtual void SetPostDisplayCallback(PostDisplayCallback* pPostDisplayCB) = 0;
	/*! \remarks This method returns a pointer to the post display callback
	function. */
	virtual PostDisplayCallback* GetPostDisplayCallback() const = 0;

	// provides a way for extending it with interfaces
	/*! \remarks This method provides a way to extend the class with
	interfaces.
	\par Parameters:
	<b>Interface_ID id</b>\n\n
	The interface ID.
	\par Default Implementation:
	<b>{ return NULL; }</b> */
	virtual		BaseInterface* GetInterface	(Interface_ID id) { UNUSED_PARAM(id); return NULL; }
};


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/*! \sa  Class IImageViewer, Class Bitmap, Class CropCallback\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents an abstract interface class for a default bitmap
viewer.\n\n
The following functions are available for use but are not part of the class
IMaxBitmapViewer.\n\n
Note that the minimum size of the floating window is 390 x 325. */
class IMAGE_VIEW_API IMaxBitmapViewer : public IImageViewer

//  PURPOSE:
//    Abstract class (Interface) for an default max bitmap viewer
//
//  NOTES:
//    created:  04.13.00 - mjm
//
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

{
public:
	// sets/gets the bitmap
	/*! \remarks This method allows you to set the bitmap which should be
	displayed in the bitmap viewer.\n\n
	Note: This is for internal use only.
	\par Parameters:
	<b>Bitmap* pBitmap</b>\n\n
	A pointer to the bitmap to display. */
	virtual void SetBitmap(Bitmap* pBitmap) = 0;
	/*! \remarks This method returns a pointer to the bitmap that is being
	displayed by the bitmap viewer. */
	virtual Bitmap* GetBitmap() const = 0;

	// sets/gets the crop callback
	/*! \remarks This method allows you to set the crop callback function
	which will assist in interactive adjustments of the bitmap cropping
	rectangle. This method should not be called after the window has been
	displayed. Preferably the callback should be set before the window is
	displayed or after the window has been destroyed.
	\par Parameters:
	<b>CropCallback* pCropCallback</b>\n\n
	A pointer to the callback function to set. */
	virtual void SetCropCB(CropCallback* pCropCallback) = 0;
	/*! \remarks This method returns a pointer to the crop callback function
	used by the bitmap viewer. */
	virtual CropCallback* GetCropCB() const = 0;

	// sets/gets the autonomous state
	/*! \remarks This method allows you to set whether the bitmap viewer is
	autonomous or not. When the bitmap viewer is set in autonomous mode, the
	viewer will not have a clone button, although the viewer will not display a
	clone button unless the save button is also displayed. Changes will not
	take effect until the next time the viewer is displayed using
	<b>Display()</b>.
	\par Parameters:
	<b>bool isAutonomous</b>\n\n
	TRUE to set the viewer to autonomous, otherwise FALSE. */
	virtual void SetAutonomous(bool isAutonomous) = 0;
	/*! \remarks This method returns TRUE if the viewer is autonomous,
	otherwise FALSE. When the bitmap viewer is set in autonomous mode, the
	viewer will not have a clone button, although the viewer will not display a
	clone button unless the save button is also displayed. */
	virtual bool GetAutonomous() const = 0;

	// sets/gets the current position
	/*! \remarks A call to this method will never impact the current opened
	window. This function should be used prior to calling <b>Display()</b> in
	order to specify the position of the next created window. */
	virtual void SetCurrentPosition(WindowPosition currentPosition) = 0;
	/*! \remarks This method returns the current position of the bitmap
	viewer. */
	virtual WindowPosition GetCurrentPosition() const = 0;

	// sets/gets whether to show the save button
	/*! \remarks This method allows you to set whether or not the save button
	should be shown as part of the bitmap viewer window. A change will not take
	effect until the next time the viewer is opened via <b>Display()</b>.
	\par Parameters:
	<b>bool showSaveButton</b>\n\n
	TRUE to show; FALSE to hide. */
	virtual void SetShowSaveButton(bool showSaveButton) = 0;
	/*! \remarks This method returns TRUE if the save button is shown as part
	of the bitmap viewer window, otherwise FALSE. */
	virtual bool GetShowSaveButton() const = 0;

	// (un)displays the viewer
	/*! \remarks This method will display the bitmap image viewer. Note that
	the <b>Show()</b> and <b>Hide()</b> methods have no effect until the window
	is initialized by a call to <b>Display()</b>. This method should not be
	called when the window is already open and this method can't be used to
	create a docked viewer.\n\n
	You should also not call this twice on this interface. The second time the
	function is called, the viewer will loose all references to the previoulsy
	opened window. To switch images using the same bitmap viewer, a call to
	"UnDisplay" should be made to close the previous window before calling this
	method again.\n\n

	\par Parameters:
	<b>MCHAR *title</b>\n\n
	The title of the window to set.\n\n
	<b>WindowPosition position = WPos_Center</b>\n\n
	The window position you wish to set, which is one of the following enum
	values; <b>WPos_NULL, WPos_UpperLeft, WPos_LowerLeft, WPos_UpperRight,
	WPos_LowerRight, WPos_Center</b>.
	\return  TRUE if successful, otherwise FALSE. */
	virtual bool Display(MCHAR *title, WindowPosition position = WPos_Center) = 0;
	// parameters x, y, w, & h will be ignored unless 'Current Position' is WPos_NULL
	/*! \remarks This method will display the bitmap image viewer. Note that
	the <b>Show()</b> and <b>Hide()</b> methods have no effect until the window
	is initialized by a call to <b>Display()</b>. For docked viewers, the
	<b>hParent</b> parameter is the window into which te viewer will be docked.
	For floating viewers, the parameter has no effect.\n\n
	You should also not call this twice on this interface. The second time the
	function is called, the viewer will loose all references to the previoulsy
	opened window. To switch images using the same bitmap viewer, a call to
	"UnDisplay" should be made to close the previous window before calling this
	method again.
	\par Parameters:
	<b>MCHAR *title</b>\n\n
	The title of the window to set.\n\n
	<b>HWND hParent</b>\n\n
	A handle to the parent window.\n\n
	<b>int x, y, h, w</b>\n\n
	The position and dimensions of the window.\n\n
	Note that these parameters will be ignored unless 'Current Position' is
	<b>WPos_NULL</b>.
	\return  TRUE if successful, otherwise FALSE. */
	virtual bool Display(MCHAR *title, HWND hParent, int x, int y, int w, int h) = 0;
	/*! \remarks This method will undisplay the current bitmap image. The
	<b>Show()</b> and <b>Hide()</b> methods will no longer function after the
	window is destroyed. */
	virtual bool UnDisplay() = 0;
	/*! \remarks This method will clear the bitmap image viewer window
	contents. This will not delete the window as it would using the Delete
	button on the viewer toolbar which actually deletes the contents of the
	bitmap. */
	virtual void ClearScreen() = 0;

	// transforms point/rect between window's client coords and bitmap's coords
	/*! \remarks This method will transform a specified point between the
	window's client coordinates and the bitmap coordinates and return the
	result.
	\par Parameters:
	<b>const POINT \&pt</b>\n\n
	The point on screen. */
	virtual POINT XFormScreenToBitmap(const POINT &pt) const = 0;
	/*! \remarks This method will transform a specified point between the
	window's client coordinates and the bitmap coordinates and return the
	result.
	\par Parameters:
	<b>const POINT \&pt</b>\n\n
	The point on the bitmap. */
	virtual POINT XFormBitmapToScreen(const POINT &pt) const = 0;
	/*! \remarks This method will transform a specified rectangle between the
	window's client coordinates and the bitmap coordinates and return the
	result.
	\par Parameters:
	<b>const Rect \&rect</b>\n\n
	The screen rectangle. */
	virtual Rect XFormScreenToBitmap(const Rect &rect) const = 0;
	/*! \remarks This method will transform a specified rectangle between the
	window's client coordinates and the bitmap coordinates and return the
	result.
	\par Parameters:
	<b>const Rect \&rect</b>\n\n
	The bitmap rectangle. */
	virtual Rect XFormBitmapToScreen(const Rect &rect) const = 0;

	// shows/hides/toggles the toolbar
	/*! \remarks This method allows you to set whether the toolbar should be
	shown as part of the bitmap image viewer window.
	\par Parameters:
	<b>bool show</b>\n\n
	TRUE to show; FALSE to hide. */
	virtual void ShowToolbar(bool show) = 0;
	/*! \remarks This method allows you to toggle the toolbar of the bitmap
	image viewer window on and off. */
	virtual void ToggleToolbar() = 0;

	// gets the portion of the window's client area that is safe to draw in (in client coordinates)
	/*! \remarks This method will retrieve the portion of the window's client
	area that is safe to draw in (in client coordinates and below the toolbar).
	This method can return a value larger than the displayed bitmap if the
	window size is large enough.
	\par Parameters:
	<b>Rect\& drawableRect</b>\n\n
	The client area. */
	virtual void GetDrawableRect(Rect& drawableRect) = 0;

	// refreshes the region of the window, or the entire window if region is NULL
	/*! \remarks This method will refresh the region of the window, or the
	entire window if region is NULL
	\par Parameters:
	<b>Rect* pRefreshRegion = NULL</b>\n\n
	The region of the window you wish to refresh. */
	virtual void RefreshWindow(Rect* pRefreshRegion = NULL) = 0;

	// provides a way for extending it with interfaces
	/*! \remarks This method provides a way to extend the class with
	interfaces.
	\par Parameters:
	<b>Interface_ID id</b>\n\n
	The interface ID.
	\par Default Implementation:
	<b>{ return NULL; }</b> */
	virtual		BaseInterface* GetInterface	(Interface_ID id) { UNUSED_PARAM(id); return NULL; }
};


/*! \remarks This function allows you to create a new bitmap viewer as either
a floating window or docked in a viewport.
\par Parameters:
<b>Bitmap* pBitmap</b>\n\n
The bitmap to use with the viewer. This should not be NULL.\n\n
<b>IImageViewer::DisplayStyle displayStyle</b>\n\n
The display style for the viewer, which is either <b>IV_FLOATING</b> or
<b>IV_DOCKED</b>.
\return  A pointer to a new IMaxBitmapViewer or NULL if the viewer could not be
created. */
IMAGE_VIEW_API IMaxBitmapViewer* CreateIMaxBitmapViewer(Bitmap* pBitmap, IImageViewer::DisplayStyle displayStyle);
/*! \remarks This method will delete and release the specified bitmap viewer.
This method should not be used while the viewer is being displayed. Use
<b>UnDisplay()</b> or <b>Hide()</b> before calling this method.
\par Parameters:
<b>IMaxBitmapViewer *</b>\n\n
A pointer to the viewer.  */
IMAGE_VIEW_API void ReleaseIMaxBitmapViewer(IMaxBitmapViewer *);

