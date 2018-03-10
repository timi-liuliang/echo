//-----------------------------------------------------------------------------
// ----------------
// File ....: gup.h
// ----------------
// Author...: Gus Grubba
// Date ....: September 1998
//
// History .: Sep, 30 1998 - Started
//
//-----------------------------------------------------------------------------
#pragma once

#ifdef GUP_EXPORTS
	#define GUPExport __declspec( dllexport )
#else
	#define GUPExport __declspec( dllimport )
#endif

#include "maxheap.h"
#include "gupapi.h"
#include "strbasic.h"

// forward declarations
class BitmapManager;
class GUP;
class ClassDesc;
class ISave;
class ILoad;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-- Plug-Ins Handler
//
	
/*! \sa  Class GUP.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class is used to collect and maintain GUP type plug-ins and is for
internal use only. */
class GUPHandler : public InterfaceServer {

		//-- DLL Handler ----------------------------------
		
		ClassDesc*	cd;
		Class_ID	id;
		GUP*		instance;
		
	public:

								GUPHandler		( );

		GUPExport	void		SetCD			( ClassDesc *dll )	{ cd = dll;	}
		GUPExport	ClassDesc*	GetCD			( )					{ return cd;}
		GUPExport	void		SetID			( Class_ID cid )	{ id = cid;	}
		GUPExport	Class_ID	GetID			( )					{ return id;}
		GUPExport	void		SetInstance		( GUP *ins )		{ instance = ins;  }
		GUPExport	GUP*		GetInstance		( )					{ return instance; }

		GUPExport	GUP*		Gup				( )					{ return instance; }

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-- List of Loaded Plug-Ins
//
	
/*! \sa  Class GUP,  Class GUPManager, Template Class Tab, Class GUPHandler.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class can be used to create instance of GUP plug-ins on the fly, if
needed. It is accessed through the GUPManager object. It keeps a list of
available GUP plug-ins. An example of this can be found in
<b>/MAXSDK/SAMPLES/UTILITIES/COMSRV</b>. This is a standard Utility plug-in
which is used to give the GUP plug-in COMSRV a user interface.  */
class GUPList: public Tab<GUPHandler> {

	public:

		/*! \remarks Constructor. */
		GUPExport			GUPList				( )	{ ; }
		/*! \remarks Returns the zero based index of the specified GUP plug-in
		in the list maintain by this class. If not found -1 is returned.
		\par Parameters:
		<b>const Class_ID id</b>\n\n
		Identifies the plug-in to find using its unique Class_ID. */
		GUPExport	int		FindGup				( const Class_ID id );
		/*! \remarks This method is used to create a new instance of the GUP
		plug-in whose Class_ID is specified. If it succeeds, it returns a newly
		created instance of a GUP object. A developer may then use the
		<b>GUP::Control()</b> method to communicate with the GUP object.
		\par Parameters:
		<b>const Class_ID id</b>\n\n
		Identifies the plug-in to create. */
		GUPExport	GUP*	CreateInstance		( const Class_ID id );

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-- Global Utility Plug-In Class
//

/*! \sa  Class Interface, Class DllDir, Class BitmapManager, Class ITreeEnumProc, Class ILoad,  Class ISave.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This is the base class for the creation of Global Utility Plug-Ins.\n\n
These plug-ins work as follows: At 3ds Max startup time, before 3ds Max begins
its message loop and after all its subsystems are initialized, the GUP Manager
will scan the plug-in directory and load all plug-ins with the "<b>*.gup</b>"
extension. One by one, the GUP Manager will call the plug-in's <b>Start()</b>
method. At that point the plug-in would do its initialization and decide if it
wants to remain loaded or if it can be discarded. As an option, it is also
possible for a GUP plug-in for make 3ds Max fail and abort all together. If a
plug-in wishes to remain loaded (after returning a <b>GUPRESULT_KEEP</b> result
code from the <b>Start()</b> method described below), it should start a new
thread for its code as there is no other call from 3ds Max.\n\n
Unlike other 3ds Max plug-ins, GUP's do not provide a user interface. If
developers of GUP plug-ins desire to present an interface, they can develop a
standard 3ds Max utility plug-in to do so. See
Class UtilityObj. There is some sample
code using this technique availalble in
<b>/MAXSDK/SAMPLES/GUP/COMSRV/MSCOM.CPP</b>. This Utility plug-in
(<b>COMSRV.DLU</b>) accesses the COM/DCOM plug-in and allows the user to
"register" or "unregister" the COM interface. See the Advanced Topics section
<a href="ms-its:3dsmaxsdk.chm::/com_dcom_interface.html">COM/DCOM
Interface</a>.
\par Plug-In Information:
Class Defined In GUP.H\n\n
Super Class ID GUP_CLASS_ID\n\n
Standard File Name Extension GUP\n\n
Extra Include File Needed None  */
class GUP : public InterfaceServer {
	
	public:
	
		/*! \remarks Constructor. */
		GUPExport	GUP								( );
		/*! \remarks Destructor. */
		GUPExport virtual	~GUP							( );

		//-----------------------------------------------------------
		//-- Plug-In Service Methods
		
		/*! \remarks Implemented by the System.\n\n
		Returns the application instance handle of 3ds Max itself. */
		GUPExport virtual	HINSTANCE		MaxInst			( );
		/*! \remarks Implemented by the System.\n\n
		Returns the window handle of 3ds Max's main window. */
		GUPExport virtual	HWND			MaxWnd			( );
		/*! \remarks Implemented by the System.\n\n
		Returns a pointer to an instance of a class which provides access to
		the DLL Directory. This is a list of every DLL loaded in 3ds Max */
		GUPExport virtual	DllDir*			MaxDllDir		( );
		/*! \remarks Implemented by the System.\n\n
		Returns an interface pointer for calling methods provided by 3ds Max.
		*/
		GUPExport virtual	Interface*		Max				( );
		/*! \remarks Implemented by the System.\n\n
		Returns a pointer to the bitmap manager which may be used to manage the
		use of bitmaps within 3ds Max. */
		GUPExport virtual	BitmapManager*	Bmi				( );
		/*! \remarks Implemented by the System.\n\n
		This may be called to enumerate every INode in the scene.
		\par Parameters:
		<b>ITreeEnumProc *proc</b>\n\n
		This callback object is called once for each INode in the scene.
		\return  Nonzero if the process was aborted by the callback
		(<b>TREE_ABORT</b>); otherwise 0. */
		GUPExport virtual	int				EnumTree		( ITreeEnumProc *proc );

		//-----------------------------------------------------------
		//-- Plug-In Service Methods (Scripting)
		
		/*! \remarks Implemented by the System.\n\n
		This method will execute the specified MAXScript command. If a
		developer needs to ask 3ds Max to do something and this "something" is
		not implemented within the COM interface, it is possible to send
		MAXScript commands through this method (and <b>ExecuteFileScript()</b>
		below). This method will execute whatever string is specified, for
		instance <b>ExecuteStringScript("open \"MyScene.max\"")</b>.
		\par Parameters:
		<b>MCHAR *string</b>\n\n
		Points to the MAXScript command to execute.
		\return  TRUE indicates if the command was successfully sent to
		MAXScript; FALSE if it was not sent. Note that this does not reflect
		the success of the embedded command. */
		GUPExport virtual	bool			ExecuteStringScript	( MCHAR *string );
		/*! \remarks Implemented by the System.\n\n
		This method will execute the specified MAXScript file.
		\par Parameters:
		<b>MCHAR *file</b>\n\n
		The file name for the script file.
		\return  TRUE indicates if the script was successfully sent to
		MAXScript; FALSE if it was not sent. Note that this does not reflect
		the result of the script. */
		GUPExport virtual	bool			ExecuteFileScript	( MCHAR *file );

		//-----------------------------------------------------------
		//-----------------------------------------------------------
		//-- Plug-In Implementation Methods
		
		//-----------------------------------------------------------
		// Start() is called at boot time. If the plug-in
		// desires to remain loaded, it returns GUPRESULT_KEEP. If
		// it returns GUPRESULT_NOKEEP, it will be discarded. If it
		// returns GUPRESULT_ABORT MAX will be shut down.

		#define GUPRESULT_KEEP		0x00
		#define GUPRESULT_NOKEEP	0x01
		#define GUPRESULT_ABORT		0x03

		/*! \remarks This method is called at boot time. At that point the plug-in should do
		its initialization and decide if it wants to remain loaded or if it can
		be discarded. As an option, it is also possible for a GUP plug-in for
		make 3ds Max fail and abort all together. Obviously this should be used
		with caution and plenty of documentation from the part of the developer
		of the plug-in. If a plug-in wishes to remain loaded (after returning a
		<b>GUPRESULT_KEEP</b> result code), it should start a new thread for
		its code as there is no other call from 3ds Max.
		\return  One of the following values:\n\n
		<b>GUPRESULT_KEEP</b>\n\n
		Return this to value to have the plug-in remain loaded.\n\n
		<b>GUPRESULT_NOKEEP</b>\n\n
		Return this value to discard.\n\n
		<b>GUPRESULT_ABORT</b>\n\n
		Return this value to cause 3ds Max to shut down. */
		GUPExport virtual	DWORD			Start		( ) = 0;
		
		//-------------------------------------------------
		// Stop is called prior to discarding the plug-in
		// for persistent plug-ins (i.e. only those that 
		// returned GUPRESULT_KEEP for Start() above).
		
		/*! \remarks The <b>Stop()</b> method is called when 3ds Max is going down. The GUP
		Manager will call this methods for all GUP plug-ins that were
		maintained in memory right before it discards them. This method is
		called only for plug-ins that returned <b>GUPRESULT_KEEP</b> for the
		<b>Start()</b> method above. */
		GUPExport virtual	void			Stop		( ) = 0;

		//-------------------------------------------------
		// Control is an entry point for external access to
		// GUP plug-ins. For instance, Utility plugins can
		// invoke their GUP plugin counterpart and have 
		// direct access to them.
		//
      // SR NOTE64: This can return pointer-sized data, so 
      //            it has to be a DWORD_PTR.
		/*! \remarks This method is an entry point for external access to GUP plug-in. For instance,
		Utility plug-ins can invoke their GUP plugin counterpart and have direct access
		to them.
		\par Parameters:
		<b>DWORD parameter</b>\n\n
		The meaning of this parameter is defined by the plug-in.
		\return  The return value meaning is also defined by the plug-in.
		\par Default Implementation:
		<b>{ return 0;}</b> */
		GUPExport virtual	DWORD_PTR		Control		( DWORD parameter ) { UNUSED_PARAM(parameter); return 0;}

		//-------------------------------------------------
		// Optional Methods for saving the plug-in state
		// within a scene file.

		/*! \remarks This method is called to save any data the plug-in may
		have into the 3ds Max file.
		\par Parameters:
		<b>ISave *isave</b>\n\n
		An interface used for saving data. */
		GUPExport virtual	IOResult		Save		( ISave *isave );
		/*! \remarks This method is called to load any data the plug-in may
		have from the 3ds Max file.
		\par Parameters:
		<b>ILoad *iload</b>\n\n
		An interface used for loading data. */
		GUPExport virtual	IOResult		Load		( ILoad *iload );
		
		// RK: 06/28/00, added it to support static instances
		GUPExport virtual	void			DeleteThis	( ) { }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-- Main GUP Manager Class
//
//

/*! \sa  Class GUPList, Class GUPInterface.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
The GUP library exports a pointer variable which is used to access the GUP
Manager object (a global instance of this class):\n\n
<b>GUPManager* gupManager;</b>\n\n
The only methods of this class of interest are the instance of <b>GUPList</b>
and the <b>Interface()</b> method. All other methods are for internal use only.
\par Data Members:
All data members are public.\n\n
<b>GUPList gupList</b>\n\n
The list of available GUP plug-ins.  */
class GUPManager: public MaxHeapOperators    {
	
		GUPInterface*	iface;

		int				InitList		( );
		bool			listed;
		
	public:
	
		/*! \remarks Constructor. */
		GUPExport					GUPManager		( GUPInterface *i );
		/*! \remarks Destructor. */
		GUPExport					~GUPManager		( );
		
		GUPList		gupList;
		
		/*! \remarks This method is for internal use only. */
		GUPExport	bool			Ready			( );
		/*! \remarks This method is for internal use only. */
		GUPExport	bool			Init			( );
		/*! \remarks This method gives GUP plug-ins access to 3ds Max main
		Inteface class. Plug-ins don't need to access GUPInterface directly as
		all the functionality is exposed in the GUP class itself. */
		GUPExport	GUPInterface*	Interface		( ) { return iface; }
		GUPExport	IOResult		Save			( ISave *isave );
		GUPExport	IOResult		Load			( ILoad *iload );

};

//-----------------------------------------------------------------------------
//-- Exported
//

extern GUPExport	void			OpenGUP			(  GUPInterface *i );
extern GUPExport	void			CloseGUP		(  );
extern GUPExport	GUPManager*		gupManager; 
extern GUPExport	GUP*			OpenGupPlugIn	( const Class_ID id); 

//-----------------------------------------------------------------------------
//-- Cleanup


//-- EOF: gup.h ---------------------------------------------------------------
