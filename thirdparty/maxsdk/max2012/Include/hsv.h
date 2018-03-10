/**********************************************************************
 *<
	FILE: hsv.h

	DESCRIPTION:

	CREATED BY: Dan Silva

	HISTORY:

	NH 16|05|03 - Added AColor support.

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "maxheap.h"
#include "coreexp.h"
#include <WTypes.h>
#include "acolor.h"
#include "ipoint2.h"
#include "baseinterface.h"
#include "strbasic.h"

#define MAXCOLORS 16


// This callback proc gets called after every mouse button up to tell you the
// new color, if you want to do interactive update.

/*! \sa  Class ColorPicker, <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_color_conversion_utilities.html">List of Color Conversion Utilities</a>.\n\n
\par Description:
This class provides methods that are called when using the modeless color
picker. All methods of this class are virtual.  */
#pragma warning(push)
#pragma warning(disable:4100)

class HSVCallback : public MaxHeapOperators {
	public:
		/*! \remarks Destructor. */
		virtual ~HSVCallback() {;}
		/*! \remarks This method is called when the user has pressed the mouse button.
		\par Default Implementation:
		<b>{}</b> */
		virtual	void ButtonDown() {}
		/*! \remarks This method is called when the user has released the mouse button.
		\par Parameters:
		<b>BOOL accept</b>\n\n
		TRUE if the mouse button was released normally; FALSE if the user
		canceled.
		\par Default Implementation:
		<b>{}</b> */
		virtual	void ButtonUp(BOOL accept) {}
		/*! \remarks This callback proc gets called after the user changes the color.
		Implement this method to handle interactive updates.
		\par Parameters:
		<b>DWORD col</b>\n\n
		The new color.\n\n
		<b>BOOL buttonUp</b>\n\n
		Indicates if the mouse button has been released (is up). TRUE if the
		button is up; FALSE if it is down. */
		virtual	void ColorChanged(DWORD col, BOOL buttonUp) {}
		/*! \remarks This callback proc gets called after the user changes the color.
		Implement this method to handle interactive updates.
		\par Parameters:
		<b>AColor col</b>\n\n
		The new color.\n\n
		<b>BOOL buttonUp</b>\n\n
		Indicates if the mouse button has been released (is up). TRUE if the
		button is up; FALSE if it is down. */
		virtual void ColorChanged(AColor col, BOOL ButtonUp) {}
		/*! \remarks This callback proc gets called when the color picker is closed:
		\par Parameters:
		<b>IPoint2 pos</b>\n\n
		The last screen position of the color picker before it was closed. */
		virtual	void BeingDestroyed(IPoint2 pos)=0;	// gets called when picker is closed: 
		/*! \remarks This callback proc gets called when user click OK to confirm
		the color modification.
		Default Implementation:
		<b>Do Nothing</b> */
		CoreExport virtual void OnOK();
		/*! \remarks This callback proc gets called when user click Cancel to cancel
		the color modification.
		Default Implementation:
		<b>Do Nothing</b> */
		CoreExport virtual void OnCancel();
	};

// Put up the Modal dialog.
/*! \remarks This method puts up the modal HSV color picker dialog box.
This dialog appears below: \n\n
\image html "dlgcsel.gif"
\par Parameters:
<b>HWND hwndOwner</b>\n\n
Owner window handle.\n\n
<b>DWORD *lpc</b>\n\n
Pointer to color to be edited. See
<a href="ms-its:listsandfunctions.chm::/idx_R_colorref.html">COLORREF</a>.\n\n
<b>IPoint2 *spos</b>\n\n
The starting position of the upper left corner of the dialog window. This is
set to the ending position when the user is done. You may pass NULL to get the
default location.\n\n
<b>HSVCallback *callBack</b>\n\n
Callback object whose procs are called when the color changes. See
Class HSVCallback.\n\n
<b>MCHAR *name</b>\n\n
The name of color being edited to appear in the title bar.
\return  Returns TRUE if the user exists the dialog with OK, otherwise returns
FALSE. */
extern CoreExport INT_PTR HSVDlg_Do(
							 HWND hwndOwner, 		// owning window
							 DWORD*   lpc,			// pointer to color to be edited
							 IPoint2*   spos, 		// starting position, set to ending position
							 HSVCallback* callBack,	// called when color changes
							 MCHAR* name			// name of color being edited
							 );

// NH 16|05|03 Added this method to support AColor.
/*! \remarks This method puts up the modal HSV color picker dialog box.
\par Parameters:
<b>HWND hwndOwner</b>\n\n
Owner window handle.\n\n
<b>AColor *lpc</b>\n\n
Pointer to color to be edited.\n\n
<b>IPoint2 *spos</b>\n\n
The starting position of the upper left corner of the dialog window. This is
set to the ending position when the user is done. You may pass NULL to get the
default location.\n\n
<b>HSVCallback *callBack</b>\n\n
Callback object whose procs are called when the color changes. See
Class HSVCallback.\n\n
<b>MCHAR *name</b>\n\n
The name of color being edited to appear in the title bar.
\return  Returns TRUE if the user exists the dialog with OK, otherwise returns
FALSE. */
extern CoreExport INT_PTR HSVDlg_Do(
									HWND hwndOwner, 		// owning window
									AColor *lpc,				// pointer to color to be edited
									IPoint2 *spos, 			// starting position, set to ending position
									HSVCallback *callBack,	// called when color changes
									MCHAR *name				// name of color being edited
									);

CoreExport void RGBtoHSV (DWORD rgb, int *ho, int *so, int *vo);
CoreExport DWORD HSVtoRGB (int H, int S, int V);
CoreExport void HSVtoHWBt (int h, int s, int v, int *ho, int *w, int *bt);
CoreExport void HWBttoHSV (int h, int w, int bt, int *ho, int *s, int *v);

// RB: Added floating point versions
class Color;
CoreExport Color RGBtoHSV(Color rgb);
CoreExport Color HSVtoRGB(Color hsv);

// MODELESS Version

/*! \sa  Class HSVCallback, <a href="ms-its:listsandfunctions.chm::/idx_R_colorref.html">COLORREF - DWORD</a>, Class IPoint2.\n\n
\par Description:
This class allows a plug-in to create a <b>modeless</b> color picker dialog
box.\n\n
Developers may also create a <b>modal</b> version of this dialog box. The
function HSVDlg_Do is defined for this purpose. Note that this is not a
class method but a global function.\n\n
To use these APIs you need to <b>#include "hsv.h"</b>.\n\n
*/
class ColorPicker : public InterfaceServer {
	protected:
	/*! \remarks Destructor. */
	~ColorPicker() {}
	public:
		/*! \remarks Constructor. */
		ColorPicker() {}
		/*! \remarks This method changes the current color in the color
		picker, but does not change the "reset" color.
		\par Parameters:
		<b>DWORD color</b>\n\n
		The current color. */
		virtual void ModifyColor (DWORD color)=0;
		/*! \remarks Sets a new color as current in the dialog.
		\par Parameters:
		<b>DWORD color</b>\n\n
		The color to set.\n\n
		<b>MCHAR *name</b>\n\n
		A new name to display in the title bar. */
		virtual void SetNewColor (DWORD color, MCHAR *name)=0;  
		/*! \remarks Returns the current color. */
		virtual DWORD GetColor()=0;
		/*! \remarks Returns the screen position of the upper left corner of
		the dialog as a IPoint2. */
		virtual IPoint2 GetPosition()=0;
		/*! \remarks This is called when the parent is going away. */
		virtual void Destroy()=0;  // remove window and delete ColorPicker.
		/*! \remarks This method is used to add a different callback, set a
		new initial color and update the title string.
		\par Parameters:
		<b>DWORD col</b>\n\n
		The new initial color.\n\n
		<b>HSVCallback *pcb</b>\n\n
		The new callback.\n\n
		<b>MCHAR *name</b>\n\n
		The new title string. */
		virtual void InstallNewCB(DWORD col, HSVCallback *pcb, MCHAR *name)=0;
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method is called when the display gamma changes. */
		virtual void RefreshUI() {}  // Called when display gamma changes


		// NH 16|05|03 Added this method to support AColor. 
		/*! \remarks This method changes the current color in the color
		picker, but does not change the "reset" color.
		\par Parameters:
		<b>AColor color</b>\n\n
		The current color. */
		virtual void ModifyColor(AColor color){}
		// NH 16|05|03 Added this method to support AColor. 
		/*! \remarks Sets a new color as current in the dialog.
		\par Parameters:
		<b>AColor color</b>\n\n
		The color to set.\n\n
		<b>MCHAR *name</b>\n\n
		A new name to display in the title bar. */
		virtual void SetNewColor(AColor, MCHAR * name){}
		// NH 16|05|03 Added this method to support AColor. THis will return the AColor from the picker.
		/*! \remarks Returns the current  color as an Acolor from the color picker.
		Default implementation returns AColor(0,0,0,0). */
		virtual AColor GetAColor(){return AColor(0,0,0,0);}
		// NH 16|05|03 Added this method to support AColor. 
		/*! \remarks This method is used to add a different callback, set a new
		initial color and update the title string.
		\par Parameters:
		<b>Acolor col</b>\n\n
		The new initial color.\n\n
		<b>HSVCallback *pcb</b>\n\n
		The new callback.\n\n
		<b>MCHAR *name</b>\n\n
		The new title string. */
		virtual void InstallNewCB(AColor col, HSVCallback *pcb, MCHAR *name){}
		//Brian Duan 22|05|08 Add this method to support Managed ColorPicker 
		//in Scene Explorer and Material Explorer.
		/*! \remarks Call this function to get the rectangle of current dialog of color picker.
		\return  Default implementation returns RECT(0,0,0,0). */
		CoreExport virtual RECT GetRect()const;
		
		/** 
		* \brief Is this picker in color sampling (eyedropper) mode.
		* 
		* Default implementation returns false.
		* \return true if this picker is in sampling mode, false otherwise.
		*/
		CoreExport virtual bool IsColorSamplerMode()const;
	};

// Create A Modeless Color Picker
/*! \remarks Call this function to bring up the modeless color picker.
\par Parameters:
<b>HWND hwndOwner</b>\n\n
The owner window handle.\n\n
<b>DWORD initColor</b>\n\n
The initial color for the color picker.\n\n
<b>IPoint2* spos</b>\n\n
The initial screen position of the upper left corner. NULL may be passed for
the default location.\n\n
<b>HSVCallback *pcallback</b>\n\n
The callback object to respond to color change events.\n\n
<b>MCHAR *name</b>\n\n
The title string in the dialog.\n\n
<b>int objClr=0</b>\n\n
A BOOLEAN used to indicate that the ColorPicker is being used to set the object
color from the control panel. In all other cases, the default value of 0 should
be used.
\return  A pointer to a ColorPicker object. */
CoreExport ColorPicker *CreateColorPicker(HWND hwndOwner, DWORD initColor,
	 IPoint2* spos, HSVCallback *pcallback, MCHAR *name, int objClr=0);


// Create A Modeless Color Picker
// NH 16|05|03 Added this method to support AColor.
CoreExport ColorPicker *CreateColorPicker(HWND hwndOwner, AColor initColor,
	 IPoint2* spos, HSVCallback *pcallback, MCHAR *name, int objClr=0);
	 
/*! \remarks Establishes the color picker initial screen position.
\par Parameters:
<b>IPoint2 \&pos</b>\n\n
The upper left corner screen coordinate for the color picker. */
CoreExport void SetCPInitPos(IPoint2 &pos);
/*! \remarks Retrieves the color picker initial screen position.
\return  The screen coordinates of the color picker. This is the coordinate of
the upper left corner.  */
CoreExport IPoint2 GetCPInitPos(void);	

#define WM_ADD_COLOR	(WM_USER+2321)	// wParam = color

//--------------------------------------------------------------------------
// Pluggable color picker class ( COLPICK_CLASS_ID )
//--------------------------------------------------------------------------


/*! \sa  Class ColorPicker, Class HSVCallback, Class Class_ID, Class IPoint2, <a href="ms-its:listsandfunctions.chm::/idx_R_colorref.html">DWORD--COLORREF Color Format</a>..\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This is the base class for the creation of plug-in color selectors. The list of
available color pickers appear in the 3ds Max user interface in the General
page of the Preferences dialog. The choosen picker will be called whenever a
user clicks on a 3ds Max color swatch control.
\par Plug-In Information:
Class Defined In HSV.H\n\n
Super Class ID COLPICK_CLASS_ID\n\n
Standard File Name Extension DLU\n\n
Extra Include File Needed HSV.H */
class ColPick : public InterfaceServer {
	public:
	// Do Modal dialog
	virtual INT_PTR ModalColorPicker(
		HWND hwndOwner, 		// owning window
		DWORD *lpc,				// pointer to color to be edited
	    IPoint2 *spos, 			// starting position, set to ending position
	    HSVCallback *callBack,	// called when color changes
		MCHAR *name				// name of color being edited
	    )=0;
	/*! \remarks This method is called to bring up the modal color picker.
	\par Parameters:
	<b>HWND hwndOwner</b>\n\n
	The owning window handle\n\n
	<b>DWORD *lpc</b>\n\n
	A pointer to the color to be edited. See <a href="ms-its:listsandfunctions.chm::/idx_R_colorref.html">DWORD
	COLORREF Format</a>.\n\n
	<b>IPoint2 *spos</b>\n\n
	The starting position of the dialog. This is set to ending position on
	return.\n\n
	<b>HSVCallback *callBack</b>\n\n
	This callback is called whenever color changes.\n\n
	<b>MCHAR *name</b>\n\n
	The name of color being edited
	\return  TRUE if the user pressed OK; FALSE on cancel. */
	virtual INT_PTR ModalColorPicker(
		HWND hwndOwner, 		// owning window
		AColor *lpc,				// pointer to color to be edited
		IPoint2 *spos, 			// starting position, set to ending position
		HSVCallback *callBack,	// called when color changes
		MCHAR *name				// name of color being edited
		){return 0;}

	// Create Modeless dialog.
	/*! \remarks This method is called to create and return a ColorPicker object for the
	modeless color picker.
	\par Parameters:
	<b>HWND hwndOwner</b>\n\n
	The owning window handle.\n\n
	<b>DWORD initColor</b>\n\n
	The inital value of the color. See <a href="ms-its:listsandfunctions.chm::/idx_R_colorref.html">DWORD COLORREF
	Format</a>.\n\n
	<b>IPoint2* spos</b>\n\n
	The starting position of dialog.\n\n
	<b>HSVCallback *pcallback</b>\n\n
	This callback is called whenever color changes.\n\n
	<b>MCHAR *name</b>\n\n
	The name of color being edited.\n\n
	<b>BOOL isObjectColor=FALSE</b>\n\n
	This indicates the color picker is being used for the object color in the
	command panel, and the color picker then displays the Add Color button */
	virtual	ColorPicker *CreateColorPicker(
		HWND hwndOwner,   // owning window
		DWORD initColor,  // inital value of color
		IPoint2* spos,    // starting position of dialog
		HSVCallback *pcallback, // call back when color changes
		MCHAR *name, 	  // name of color being edited
		BOOL isObjectColor=FALSE)=0;

	/*! \remarks This method is called to create and return a ColorPicker object for the
	modeless color picker.
	\par Parameters:
	<b>HWND hwndOwner</b>\n\n
	The owning window handle.\n\n
	<b>AColor initColor</b>\n\n
	The inital value of the color.\n\n
	<b>IPoint2* spos</b>\n\n
	The starting position of dialog.\n\n
	<b>HSVCallback *pcallback</b>\n\n
	This callback is called whenever color changes.\n\n
	<b>MCHAR *name</b>\n\n
	The name of color being edited.\n\n
	<b>BOOL isObjectColor=FALSE</b>\n\n
	This indicates the color picker is being used for the object color in the
	command panel, and the color picker then displays the Add Color button */
	virtual	ColorPicker *CreateColorPicker(
		HWND hwndOwner,   // owning window
		AColor initColor,  // inital value of color
		IPoint2* spos,    // starting position of dialog
		HSVCallback *pcallback, // call back when color changes
		MCHAR *name, 	  // name of color being edited
		BOOL isObjectColor=FALSE){return NULL;}

	/*! \remarks Returns the name of the class. This name appears in the drop down list of
	color picker choices. */
	virtual const MCHAR* ClassName()=0;
	/*! \remarks Returns the unique ClassID of this plug-in. The Class_ID for the default
	color picker is <b>Class_ID(DEFAULT_COLPICK_CLASS_ID,0)</b>. */
	virtual Class_ID ClassID()=0;
	/*! \remarks This method is called to delete this instance of the plug-in class. */
	virtual void DeleteThis()=0;
	/*! \remarks This method is used for future expansion and is currently not used. */
	virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0)=0; 
	};

#pragma warning(pop)
//--------------------------------------------------------------------------
// the class id for the default color picker is Class_ID(DEFAULT_COLPICK_CLASS_ID,0)

#define DEFAULT_COLPICK_CLASS_ID 1

//--------------------------------------------------------------------------
// These are used by the MAX to plug in the current color picker.  
// Developers should not need to access these.
/*! \remarks This function is available in release 3.0 and later only.\n\n
This global function is used internally to establish the current color picker
used.
\par Parameters:
<b>ColPick *colpick</b>\n\n
Points to the color picker to use.
\return  A pointer to the current color picker. */
CoreExport ColPick *SetCurColPick(ColPick *colpick);
/*! \remarks This function is available in release 3.0 and later only.\n\n
Returns a pointer to the current color picker.  */
CoreExport ColPick *GetCurColPick();
//--------------------------------------------------------------------------
   
//--------------------------------------------------------------------------


