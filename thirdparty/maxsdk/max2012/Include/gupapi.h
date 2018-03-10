//-----------------------------------------------------------------------------
// -------------------
// File ....: gupapi.h
// -------------------
// Author...: Gus Grubba
// Date ....: September 1998
//
// History .: Sep, 30 1998 - Started
//
//-----------------------------------------------------------------------------
#pragma once

#include "baseinterface.h"
// forward declarations
class DllDir;
class Interface;
class ITreeEnumProc;

/*! \sa  Class DllDir, Class Interface, Class ITreeEnumProc.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This is an interface class used by GUPs (Global Utility Plug-Ins).\n\n
All methods of this class are Implemented by the System.  */
class GUPInterface : public InterfaceServer {

	public:

		/*! \remarks Returns the application instance handle of 3ds Max
		itself. */
		virtual HINSTANCE	AppInst			() = 0;
		/*! \remarks Returns the window handle of 3ds Max's main window. */
		virtual HWND		AppWnd			() = 0;
		/*! \remarks Returns a pointer to an instance of a class which
		provides access to the DLL Directory. This is a list of every DLL
		loaded in 3ds Max */
		virtual DllDir*		AppDllDir		() = 0;
		/*! \remarks Returns an interface pointer for calling methods provided
		by 3ds Max. */
		virtual Interface*	Max				() = 0;
		/*! \remarks This may be called to enumerate every INode in the scene.
		\par Parameters:
		<b>ITreeEnumProc *proc</b>\n\n
		This callback object is called once for each INode in the scene.
		\return  Nonzero if the process was aborted by the callback
		(TREE_ABORT); otherwise 0. */
		virtual int			EnumTree		( ITreeEnumProc* proc ) = 0;

};

