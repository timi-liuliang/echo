/**********************************************************************
 *<
	FILE: arcdlg.h

	DESCRIPTION:

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "maxheap.h"
#include "quat.h"

/*! \sa  Class ArcballCallback.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
An instace of this class is created when the global function <b>ArcballDialog
*CreateArcballDialog()</b> is called. The <b>DeleteThis()</b> method of this
class is used to free the memory allocated by <b>CreateArcballDialog()</b>.
Call it when you are done using the dialog box.  */
class ArcballDialog: public MaxHeapOperators {
	public:
	/*! \remarks Destructor. */
	virtual ~ArcballDialog() {}
	/*! \remarks This method is called to delete the instance of the class. */
	virtual void DeleteThis()=0;  
	};

/*! \sa  Class ArcballDialog, Class Quat.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class provides methods to work with a general arcball dialog box for doing
3D rotations. This dialog appears below: \n
\n \image html "dlgarcr.gif"
To use these APIs you'll need to <b>#include "arcdlg.h"</b>\n\n
All methods of this class are virtual.\n\n
Sample code can be found in <b>/MAXSDK/SAMPLES/HOWTO/CUSTCTRL/CUSTCTRL.CPP</b>.
*/
class ArcballCallback: public MaxHeapOperators {
	public:
	/*! \remarks Destructor. */
	virtual ~ArcballCallback() {;}
	/*! \remarks This method is called when a drag operation begins. The
	developer may want to save the start state at this point. */
	virtual void StartDrag()=0;   // called when drag begins (may want to save state at this point)
	/*! \remarks This method is called when a drag operation ends. */
	virtual void EndDrag()=0;   // called when drag ends
	/*! \remarks This method is called during a drag operation.
	\par Parameters:
	<b>Quat q</b>\n\n
	The relative rotation from the start rotation.\n\n
	<b>BOOL buttonUp</b>\n\n
	If TRUE this indicates if the mouse button is up (has been released); if
	FALSE the button is down. */
	virtual void Drag(Quat q, BOOL buttonUp)=0;  // called during drag, with q=relative rotation from start
	/*! \remarks This method is called when the right mouse button is clicked
	during a drag operation to cancel it. */
	virtual void CancelDrag()=0;  // called when right button clicked during drag
	/*! \remarks This method is called if the dialog box window was closed.
	Note that developers should <b>not</b> call
	<b>ArcballDialog::DeleteThis()</b> from inside this method. */
	virtual	void BeingDestroyed()=0;  // called if the window was closed
	};

/*! \remarks This global function is provided by 3ds Max and is used to create
the arcball dialog box. Then the methods of your callback class are called
based on the user's use of the dialog. Be sure to call the
<b>ArcballDialog::DeleteThis()</b> method when done.
\par Parameters:
<b>ArcballCallback *cb</b>\n\n
The callback whose methods are called based on the user's interaction with the
dialog controls.\n\n
<b>HWND hwndOwner</b>\n\n
The window handle of the dialog owner.\n\n
<b>MCHAR* title=NULL</b>\n\n
The title string to be displayed in the dialog.
\return  A new instance of the <b>ArcballDialog</b> class. Be sure to call its
<b>DeleteThis()</b> method when done.  */
CoreExport ArcballDialog* CreateArcballDialog(ArcballCallback* cb, HWND hwndOwner, MCHAR* title = NULL);


