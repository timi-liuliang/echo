/**********************************************************************
 *<
	FILE: winutil.h

	DESCRIPTION: Misc. windows related functions

	CREATED BY: Rolf Berteig

	HISTORY: 1-6-95 file created

 *>	Copyright (c) 1994-2003 All Rights Reserved.
 **********************************************************************/

#pragma once

#include "maxheap.h"
#include <limits>
#include <locale.h>
#include "coreexp.h"
#include <wtypes.h>
#include <CommCtrl.h> // for HIMAGELIST
#include "strclass.h"
#include "ipoint2.h"
#include "box2.h"

CoreExport float GetWindowFloat(HWND hwnd,BOOL* valid = NULL);
CoreExport int GetWindowInt(HWND hwnd,BOOL* valid = NULL);
CoreExport BOOL SetWindowTextInt( HWND hwnd, int i );
CoreExport BOOL SetWindowTextFloat( HWND hwnd, float f, int precision = 3 );
CoreExport BOOL SetDlgItemFloat( HWND hwnd, int idControl, float val );
CoreExport float GetDlgItemFloat( HWND hwnd, int idControl, BOOL* valid = NULL );
CoreExport void SetDlgFont( HWND hDlg, HFONT hFont );
CoreExport void SlideWindow( HWND hwnd, int x, int y );
CoreExport void StretchWindow( HWND hwnd, int w, int h );
CoreExport BOOL CenterWindow(HWND hWndChild, HWND hWndParent);
CoreExport void GetClientRectP( HWND hwnd, Rect* rect );
CoreExport void DrawIconButton( HDC hdc, HBITMAP hBitmap, Rect& wrect, Rect& brect, BOOL in );
CoreExport int GetListHieght( HWND hList );
CoreExport void ShadedVertLine( HDC hdc, int x, int y0, int y1, BOOL in );
CoreExport void ShadedHorizLine( HDC hdc, int y, int x0, int x1, BOOL in );
CoreExport void ShadedRect( HDC hdc, RECT& rect );
CoreExport void Rect3D( HDC hdc, RECT& rect, BOOL in );
CoreExport void WhiteRect3D( HDC hdc, RECT rect, BOOL in );
CoreExport void DrawButton( HDC hdc, RECT rect, BOOL in );
//! \brief Sets a pathname in an edit control, truncating with an ellipses if the pathname is too long.
CoreExport void SetPathWithEllipses(HWND hwnd, const MCHAR* path, HFONT hFont);

//! \brief Updates the width of a combo-box drop-down window to make all its items fit
/*! Call this function if some strings in your combo-box are longer than the actual
 * combo-box control's width. This function will update the width of the combo-box
 * to make the widest combo-box item fit.
 * \param hComboBox The handle of the combobox to update
 * \param vScroll true if the width of a vertical scroll bar should be taken into account
 * \return -1 if the combo-box could not be updated successfully, the new width of
 * the combo-box drop down otherwise.
 */
CoreExport int UpdateComboBoxDropDownWidth(HWND hComboBox, bool vScroll = true);


//! \brief XOR drawing method of rectangle
/*! This method will draw a dotted or solid rectangle onto the specified window. This method
can provide better compatibility with Vista Aero than using GDI function directly.

\param[in] hWnd Window handle of the window on which the drawing will occur.
\param[in] p0 One corner of the rectangle.
\param[in] p1 p0's diagonal in the rectangle.
\param[in] solidToRight If zero, the rectangle will be dotted. If less than zero, and p0.x >= p1.x,
the line will be solid, otherwize dotted. If greater than zero, and p0.x <= p1.x, the line will be solid,
otherwize dotted.
\param[in] bErase Whether to erase the foreground and use the background to fill the affected pixels,
this is only valid for the window handle associated with graphic window.
\param[in] bDelayUpdate if true, the result image will be stored on the back buffer and won't be presented to 
screen immediately. If next drawing function is invoked with bDelayUpdate being false, then the image 
will be presented combined with the result of previous drawing function. If the drawing result is not 
necessary to be seen immediately, passing true to this parameter will reduce the times of back buffer presenting, 
thus optimize the performance. This is only valid for window handle associated with graphic window.
*/
CoreExport void XORDottedRect( HWND hwnd, IPoint2 p0, IPoint2 p1, int solidToRight = 0,
							   bool bErase = false, bool bDelayUpdate = false);
//! \brief XOR drawing method of circle
/*! This method will draw a dotted or solid circle onto the specified window. This method
can provide better compatibility with Vista Aero than using GDI function directly.

\param[in] hWnd Window handle of the window on which the drawing will occur.
\param[in] p0 One corner of the circle's bounding rectangle.
\param[in] p1 p0's diagonal in the rectangle.
\param[in] solidToRight If zero, the circle will be dotted. If less than zero, and p0.x >= p1.x,
the line will be solid, otherwize dotted. If greater than zero, and p0.x <= p1.x, the line will be solid,
otherwize dotted.
\param[in] bErase Whether to erase the foreground and use the background to fill the affected pixels,
this is only valid for the window handle associated with graphic window.
\param[in] bDelayUpdate if true,the result image will be stored on the back buffer and won't be presented to 
screen immediately. If next drawing function is invoked with bDelayUpdate being false, then the image 
will be presented combined with the result of previous drawing function. If the drawing result is not 
necessary to be seen immediately, passing true to this parameter will reduce the times of back buffer presenting, 
thus optimize the performance. This is only valid for window handle associated with graphic window.
*/
CoreExport void XORDottedCircle( HWND hwnd, IPoint2 p0, IPoint2 p1, int solidToRight = 0,
								 bool bErase = false, bool bDelayUpdate = false);
//! \brief XOR drawing method of polylines
/*! This method will draw dotted or solid polylines onto the specified window. This method
can provide better compatibility with Vista Aero than using GDI function directly.

\param[in] hWnd Window handle of the window on which the drawing will occur.
\param[in] count The count of corners of the polylines.
\param[in] pts Pointer to the corners.
\param[in] solid If nonzero, the polylines will be solid, otherwize dotted.
\param[in] bErase Whether to erase the foreground and use the background to fill the affected pixels,
this is only valid for the window handle associated with graphic window.
\param[in] bDelayUpdate if true, the result image will be stored on the back buffer and won't be presented to 
screen immediately. If next drawing function is invoked with bDelayUpdate being false, then the image 
will be presented combined with the result of previous drawing function. If the drawing result is not 
necessary to be seen immediately, passing true to this parameter will reduce the times of back buffer presenting, 
thus optimize the performance. This is only valid for window handle associated with graphic window.
*/
CoreExport void XORDottedPolyline( HWND hwnd, int count, IPoint2 *pts, int solid = 0,
								   bool bErase = false, bool bDelayUpdate = false);

CoreExport void XORRect(HDC hdc, RECT& r, int border = 1);
//! \brief Vista Aero related method.
/*! This method will tell whether the Vista Aero is turned on or not.
\return TRUE if Vista Aero is turned on, otherwize FALSE. On Window XP or previous Windows, it will always return FALSE.
*/
CoreExport BOOL IsVistaAeroEnabled();

//! \brief If theme is supported on current OS
/*! Specifically, any Windows versions above Windows XP (Windows 5.1)should be able to support OS theme.
\return TRUE if the OS supports theme(Windows XP or later), otherwise FALSE.
*/
CoreExport BOOL IsThemeSupported();

//! \brief If the mouse cursor is currently hovering over a window
/*! This method will not differentiate whether the cursor is actually hovering over a window, or it's blocked by some other window.
/*! It only does a point-rectangle containment test.
\param[in] hWnd The handle of the window.
\return TRUE if the mouse cursor is currently hovering the window, otherwise FALSE.
*/
CoreExport BOOL IsHovering(HWND hWnd);

//! \brief If the cursor position specified is currently confined in a window's rectangle
/*! This method will not differentiate whether the cursor is actually hovering over a window, or it's blocked by some other window.
/*! It only does a point-rectangle containment test.
\param[in] hWnd The handle of the window.
\param[in] x The x-coordinate of the cursor. The coordinate is relative to the upper-left corner of the client area.
\param[in] y The y-coordinate of the cursor. The coordinate is relative to the upper-left corner of the client area.
\return TRUE if the mouse cursor is currently hovering the window, otherwise FALSE.
*/
CoreExport BOOL IsHovering(HWND hWnd, int x, int y);


CoreExport void MakeButton2State(HWND hCtrl);
CoreExport void MakeButton3State(HWND hCtrl);
CoreExport int GetCheckBox(HWND hw, int id);
CoreExport void SetCheckBox(HWND hw, int id, BOOL b);

//! \brief Detects whether the file specified by the full, absolute filename exists
/*! Detects whether specified file exists, optionally excluding files that are directories.
Note that if the filename is a guid string, the guid string is converted to a AssetUser, and then 
the AssetUser's filename is tested to see if it exists.
* \param file The null-terminated filename or guid string
* \param allowDirectory If true, returns TRUE if the filename is found and is either a file or a directory. If false,
returns TRUE if the filename is found and is a file but not a directory.
* \return TRUE if the file exists.
*/
CoreExport BOOL DoesFileExist(const MCHAR* file, bool allowDirectory = true);

/** Retrieves a file's or directory's creation, last access and last modification times.
\param [in] filePath - absolute path to the file 
\param [out] creationTime - pointer to a FILETIME structure to receive the date and 
time the file or directory was created. This parameter can be NULL.
\param [out] lastAccessTime - pointer to a FILETIME structure to receive the date and 
time the file or directory was last accessed. This parameter can be NULL.
\param [out] lastWriteTime- pointer to a FILETIME structure to receive the date and 
time the file or directory was last modified. This parameter can be NULL.
\return True if the operation was successful, false otherwise. */
CoreExport bool GetFileTime(const MCHAR* filePath, FILETIME* creationTime, FILETIME* lastAccessTime, FILETIME* lastWriteTime);

/** Converts a FILETIME value to a string representation.
The FILETIME is converted into local time and date, and then to a string.
The format of the time and date string is given by the default user locale.
\param fileTime - the file time to convert to string
\return A string representation of the file time, or empty string if an error occurred. */
CoreExport MSTR FileTimeToString(const FILETIME& fileTime);

CoreExport int GetBitsPerPixel();

// Delete superfluous zeros from float string: 1.2300000 -> 1.23
CoreExport void StripTrailingZeros(MCHAR* buf);

template<class T> void LimitValue( T& value, T min, T max )
{
	if ( value < min ) 
		value = min;
	if ( value > max )
		value = max;
}

//! Safely casts double to float. Valid flag will indicate overflow
inline float Dbl2Flt(double val, BOOL *valid = NULL)
{
	if ( val < 0.0f )
	{
		if ( val < -FLT_MAX )
		{
			if (valid) *valid = FALSE;
			return -FLT_MAX;
		}
		if ( val > -FLT_MIN )
		{
			if (valid) *valid = FALSE;
			return -FLT_MIN;
		}
		if (valid) *valid = TRUE;
		return (float)val;
	}

	if ( val > FLT_MAX )
	{
		if (valid) *valid = FALSE;
		return FLT_MAX;
	}
	if ( val < FLT_MIN && val != 0.0 )
	{
		if (valid) *valid = FALSE;
		return FLT_MIN;
	}
	if (valid) *valid = TRUE;
	return (float)val;
}

//! Safely casts double to int. Valid flag will indicate overflow
inline int Dbl2Int(double val, BOOL* valid = NULL)
{
	if ( val > INT_MAX )
	{
		if (valid) *valid = FALSE;
		return INT_MAX;
	}
	if ( val < INT_MIN )
	{
		if (valid) *valid = FALSE;
		return INT_MIN;
	}
	if (valid) *valid = TRUE;
	return (int)val;
}

#define MAKEPOINT( lparam, pt ) { pt.x = (short)LOWORD(lparam); pt.y = (short)HIWORD(lparam); }

// The following two functions extend list boxes. Set the list box to be
// owner draw and then call these two methods in response to the
// WM_MEASUREITEM and WM_DRAWITEM messages.
// 

// Flags to pass to CustListDrawItem
#define CUSTLIST_DISABLED		(1<<0)		// Text is gray
#define CUSTLIST_MED_DISABLED		(1<<1)		// Test is darker gray
#define CUSTLIST_SEPARATOR		(1<<2)		// Draws a separator instead of text
#define CUSTLIST_DBL_SERPARATOR	(1<<3)		// Draw a double line separator
#define CUSTLIST_RED			(1<<4)		// Text is red

CoreExport void CustListMeasureItem(HWND hList,WPARAM wParam, LPARAM lParam);
CoreExport void CustListDrawItem(HWND hList,WPARAM wParam, LPARAM lParam,DWORD flags);


// MAX extended message box functionality  DB 7/98
//! Add "Hold" button
#define MAX_MB_HOLD			0x0001
//! Add "Don't show this dialog again" check box
#define MAX_MB_DONTSHOWAGAIN	0x0002

// The first four parameters are just like the Win32 MessageBox routine (but not
// all MessageBox functionality is supported!)
// The last two optional args add the functionality listed above -- exType is used
// for adding the additional buttons, and exRet is used for getting the extra
// return info.  For example, if exType includes MAX_MB_DONTSHOWAGAIN, and exRet
// is non-NULL, then exRet will have MAX_MB_DONTSHOWAGAIN set if that checkbox was
// checked by the user.
CoreExport INT_PTR MaxMsgBox(HWND hWnd, LPCMSTR lpText, LPCMSTR lpCaption, UINT type, UINT exType=0, DWORD *exRet=NULL);

/**********************************************************************
 *
 * alpha blended icon support...
 *
 **********************************************************************/
#include "plugapi.h"

enum LoadMAXIconErrors
{
	LMI_Ok,
	LMI_ResourceNotFound,
	LMI_ResourceLoadFailed,
	LMI_ImageAndMaskNotCompatible,
};

CoreExport LoadMAXIconErrors LoadMAXIcon(HINSTANCE hInstance,
								LPCMSTR    resID,
								LPCMSTR    resMaskID, 
								COLORREF   bkColor,
								HIMAGELIST imageList,
								int        imageIndex,
								int        preMultAlpha = TRUE);

// returns index of first image into existing imageList
CoreExport int LoadMAXIconFromBMI(LPBITMAPINFOHEADER biImage,
						    LPBITMAPINFOHEADER biMask,
						    COLORREF bkColor,
						    MCHAR* pFilePrefix,
						    int preMultAlpha = TRUE,
						    HIMAGELIST* pImageList = NULL);

CoreExport BITMAPINFOHEADER *LoadBitmapFromFile(const MCHAR *filename);

CoreExport void DrawMAXIcon(HDC hDC, Rect& r, HIMAGELIST hList32, HIMAGELIST hList16, int index);

// Compute a good color to use for drawing XOR lines over a particular background color
CoreExport COLORREF ComputeXORDrawColor(COLORREF bkgColor);
// Compute a good color to use for drawing XOR lines over a viewport
CoreExport COLORREF ComputeViewportXORDrawColor();


/**********************************************************************

Dialog resizing and positioning. A generic class for resizing a dialog.
Includes functions to save and restore the initial size and position
of the dialog.

This class makes it easy to add resizability to a dialog. You'll need
one instance of this class for each instance of a dialog. Set up
various parameters during your dialog's WM_INITDIALOG processing,
then let this class handle all WM_SIZE events.

Include library: core.lib

HOW TO USE THIS CLASS:

1. Create either a static instance in your DlgProc, or a live instance
   in the object that is driving the dialog.
2. In your WM_INITDIALOG handling, call Initialize().
3. Optionally, call SetMinimumDlgSize(). However, the default minimum size
   for a dialog is it's size at the time Initialize() is called.
4. For each control in your dialog, call SetControlInfo(); you only need 
   to do this if you want to override the default behavior for the control.
   For most, the default is to be locked to the top left corner, with a
   fixed size. Some controls may default to resizing; SysListView32 has
   some specialized behaviors you may not need to override.
5. Process the WM_SIZING message, and call Process_WM_SIZING().
6. Process the WM_SIZE message, and call Process_WM_SIZE().

**********************************************************************/

class DialogItemSizeData: public MaxHeapOperators
{
public:
	HWND hwnd;
	RECT rect;
	DWORD flags;

	DialogItemSizeData(HWND h, DWORD f = 0)
		:	hwnd(h), flags(f)
	{
		WINDOWPLACEMENT wpControl;
		wpControl.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(hwnd, &wpControl);
		rect = wpControl.rcNormalPosition;
	}
	// Compiler-generated destructor, copy constructor, and assignment 
	// operator should all be fine.
private:
	// Ensure no default constructor is generated or used.
	DialogItemSizeData();
};

class DialogResizer: public MaxHeapOperators
{
public:
	DialogResizer() : mhDlg(NULL)
	{
		mMinDlgSize.left = mMinDlgSize.top = 0;
		mMinDlgSize.right = mMinDlgSize.bottom = 50;
		mOriginalClientRect.left = mOriginalClientRect.top = 0;
		mOriginalClientRect.right = mOriginalClientRect.bottom = 50;
	}
	// Compiler-generated destructor, copy constructor, and assignment 
	// operator should all be fine.

	CoreExport void Initialize(HWND hDlg);
	CoreExport void SetMinimumDlgSize(LONG wid, LONG ht);

	// If you have a control you want to have fixed width and height, and locked to 
	// top left, you do not have to call SetControlInfo, as that is the default 
	// behavior.
	enum PositionControl
		{ kLockToTopLeft=1, kLockToTopRight, kLockToBottomLeft, kLockToBottomRight, kPositionsOnly=0xff };
	enum ControlFlags
		{ kDefaultBehavior=0, kWidthChangesWithDialog=1<<8, kHeightChangesWithDialog=1<<9 };
	CoreExport void SetControlInfo(int resID, PositionControl pos, DWORD flags = kDefaultBehavior);
	CoreExport void SetControlInfo(HWND hwnd, PositionControl pos, DWORD flags = kDefaultBehavior);

	CoreExport void Process_WM_SIZING(WPARAM wParam, LPARAM lParam);
	CoreExport void Process_WM_SIZE(WPARAM wParam, LPARAM lParam);

	// Static methods to let you save and restore the size and position of your dialog
	// (whether it's resizable or not).
	// If the section name is not specified, it will default to
	// [DialogResizer_SizeAndPositions].
	// If the ini filename is not specified, it will default to the main application ini
	// file, typically 3dsmax.ini.
	CoreExport static void SaveDlgPosition(HWND hDlg, const MCHAR* keyname, 
				const MCHAR* section = NULL, const MCHAR* inifn = NULL);
	CoreExport static void LoadDlgPosition(HWND hDlg, const MCHAR* keyname, 
				const MCHAR* section = NULL, const MCHAR* inifn = NULL);

	friend static BOOL CALLBACK GetInitialPositionECP(HWND hwnd, LPARAM lParam);

private:
	Tab<DialogItemSizeData> mControls;
	RECT mMinDlgSize;
	RECT mOriginalClientRect;
	HWND mhDlg;
};

//! \brief MaxLocaleHandler handles Globalization

//! This is a helper class that switches the Locale settings to a temporary
//! value and restores the previous locale setting in the destructor.
//! To use this class simply instanciate a variable of it's class at the beginning
//! of a method. The given locale setting will be set until the method is left and
//! the destructor of the class is called automatically.
class MaxLocaleHandler: public MaxHeapOperators {

public:

	//! \brief Constructor
	//!
	//! \param[in] category - The category affected by locale. See _tsetlocale in MSDN help for more info
	//! \param[in] localeSetting - The locale name. See _tsetlocale in MSDN help for more info
	CoreExport MaxLocaleHandler(int category = LC_ALL, const MCHAR* localeSetting = _M("C"));
	//! \brief Destructor, resets locale setting to original value
	CoreExport ~MaxLocaleHandler();

   //! \brief Resets locale setting to original value, before the object goes out of scope.
   CoreExport void RestoreLocale();

private:

	int m_category;
    MSTR m_oldLocale;
};



/**********************************************************************
 *
 * ToolTip Extender
 *
 **********************************************************************/

class ToolTipExtenderImpl; //! Internal use only. Hidden internal implementation for ToolTipExtender

/*! \sa  Class ToolTipExtender.\n\n
\par Description:
This class allows a developer to add tooltips to one or more window controls.  Controls of all types are supported.
One extender object can support all tooltips for a dialog, it's not necessary to create one extender per tooltip.
This is useful for control types which do not have native tooltip support.
*/
class ToolTipExtender : public MaxHeapOperators
{
	public:
		/*! \brief Class constructor. No heap memory is allocated until tooltips are actually set. */
		CoreExport ToolTipExtender();

		/*! \brief Class destructor. Removes all tooltips and releases heap memory. */
		CoreExport virtual ~ToolTipExtender();

		/*! \brief Sets a tooltip for an arbitrary UI control.
		Calling this twice for a single control is safe, and will remove the first tooltip.
		Calling this for multiple controls is allowed, as this class supports an unlimited number of tooltips.
		Note that tooltip resources are not automatically freed when a control is destroyed.
		Resources are freed when the extender is destroyed, or by calling RemoveToolTip() or Reset().
		\param hWnd Handle to the window control.
		\param toolTipIn ToolTip string. The string is copied, so the caller may safely delete this. */
		CoreExport virtual void SetToolTip( HWND hWnd, const MSTR& toolTipIn );

		/*! \brief Gets the tooltip for an arbitrary UI control.
		This only functions for tooltips set with this class. Returns an empty string if no tooltip is set.
		\param hWnd Handle to the window control.
		\param toolTipOut ToolTipOutput string. The method will resize the string as necessary. */
		CoreExport virtual void GetToolTip( HWND hWnd, MSTR& toolTipOut );

		/*! \brief Removes the tooltip from an arbitrary UI control.
		This only functions for tooltips set with this class.
		Note that tooltip resources are not automatically freed when a control is destroyed.
		Resources are freed when the extender is destroyed, or by calling this function or Reset().
		\param hWnd Handle to the window control. */
		CoreExport virtual void RemoveToolTip( HWND hWnd );

		/*! \brief Removes all tooltips and releases heap memory. */
		CoreExport virtual void RemoveToolTips();

		/*! \brief Returns a handle to the operating system's ToolTip object.
		This provides access to extended tooltip functionality.
		See the operating system's documentation for TOOLTIPS_CLASS. */
		CoreExport virtual HWND GetToolTipHWND(); // ToolTips window object, of type TOOLTIPS_CLASS

	protected:
		//! Internal use only.  This implementation object is created when the first tooltip is set.
		ToolTipExtenderImpl* impl;
};

/**********************************************************************
 *
 * Utility Functions
 *
 **********************************************************************/

namespace MaxSDK
{

//! \brief Multiple character search for ComboBoxes
/*! This method will select an entry in the ComboBox based on the
multiple key entries over a set time period.  The functionality mimics the 
standard windows ListView behavior, including the documented one second
time period for key entries.  It is intended to be used in conjunction with
the WM_CHAR message in the ComboBox's callback.

Example usage:
\code
	case WM_CHAR:
		return MaxSDK::SearchComboBox( hWnd, (MCHAR)wParam, _M(" "));
\endcode

\param[in] hWnd Window handle of ComboBox
\param[in] key Character entered by user
\param[in] szTrim Optional parameter.  Default is NULL.  Any leading characters in 
the ComboBox entries that are contained in szTrim will be removed prior to 
comparing to the search string.
\return true if match found, false if not
*/
CoreExport bool SearchComboBox( HWND hWnd, MCHAR key, const MCHAR* szTrim = NULL );
//! \brief Multiple character search for ListBoxes
/*! This method will select an entry in the ListBox based on the
multiple key entries over a set time period.  The functionality mimics the 
standard windows ListView behavior, including the documented one second
time period for key entries.  It is intended to be used in conjunction with
the WM_CHAR message in the ListBox's callback.

Example usage:
\code
	case WM_CHAR:
		return MaxSDK::SearchListBox( hWnd, (MCHAR)wParam, _M(" "));
\endcode

\param[in] hWnd Window handle of ListBox
\param[in] key Character entered by user
\param[in] szTrim Optional parameter.  Default is NULL.  Any leading characters in 
the ListBox entries that are contained in szTrim will be removed prior to 
comparing to the search string.
\return true if match found, false if not
*/
CoreExport bool SearchListBox( HWND hWnd, MCHAR key, const MCHAR* szTrim = NULL );

/*! \remarks Returns the default maximum width, in pixels, for tooltip windows in Max.
Tooltips wider than the maximum are broken into multiple lines. 
*/
CoreExport int GetDefaultToolTipMaxWidth();

};

