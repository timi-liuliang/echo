/* 
 *    iDragAndDrop.h - SDK API to desktop drag-and-drop services in MAX
 *
 *       Copyright (c) Autodesk, Inc, 2000.  John Wainwright.
 *
 */

#pragma once

#include "maxheap.h"
#include "iFnPub.h"
#include "iMacroScript.h"
#include "GetCOREInterface.h"

// forward declarations
class IDragAndDropMgr; 
class DragAndDropHandler;
class DropType;

// Core interface to the OLE drag-and-drop manager

// utility Tab<> specialization to hold URL strings
/*! \sa  Class DragAndDropHandler\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
The URLTab class is a <b>Tab\<MCHAR*\></b> utility class that is used by
certain components in the Drag and Drop manager to hold and pass around
packages of file URLs. The class manages its own local copies of URL strings.
This class represents the additional API support by URLTab, over-and-above that
provided by any <b>Tab\<\></b> template instantiation.
\par Data Members:
protected:\n\n
<b>BOOL downloaded;</b>\n\n
This flag is set to indicate the URL package has been downloaded and names will
reflect local copies.  */
class URLTab : public Tab<MCHAR*>
{
public:
   BOOL downloaded;  // set to indicate URL package has been downloaded & names now reflect local copies

   /*! \remarks Constructor.
   \par Default Implementation:
   <b>{ downloaded = FALSE; }</b> */
   URLTab()  { downloaded = FALSE; }
   /*! \remarks Destructor.\n\n
   URLTabs manage their own local string element copies, the destructor frees
   all these strings.
   \par Default Implementation:
   <b>{ Clear(); }</b> */
   ~URLTab() { Clear(); }
	/*! \remarks Assignment operator. */
   CoreExport URLTab& operator=(const Tab<MCHAR*>& tb);
   /*! \remarks Assignment operator. */
   CoreExport URLTab& operator=(const URLTab& tb);
   /*! \remarks This method adds a URL string to the package. A local copy of
   the string will be made.
   \par Parameters:
   <b>MCHAR* url</b>\n\n
   The URL string to add. */
   CoreExport void Add(MCHAR* url);
   /*! \remarks This method replaces the i'th element by deletes the old
   string, taking a local copy of the new one. This is used by the various
   loaders to replace a URL with its local copy path name upon download.
   \par Parameters:
   <b>int i</b>\n\n
   The index of the URL to replace.\n\n
   <b>MCHAR* url</b>\n\n
   The new URL string. */
   CoreExport void Change(int i, MCHAR* url);
   /*! \remarks This method clears the package (deletes all the strings),
   zeros the Tab\<\> and resets 'downloaded' to FALSE. */
   CoreExport void Clear();
};

// drag and drop Manager interface ID
#define DND_MGR_INTERFACE   Interface_ID(0x51163ddb, 0x2a4f1619)
/*! \remarks This function, which is not part of the class, allows you to
retrieve a pointer to the Drag and Drop Manager interface. This will return a
pointer to the drag and drop manager interface from the core interface.  */
inline IDragAndDropMgr* GetDragAndDropMgr() { return (IDragAndDropMgr*)GetCOREInterface(DND_MGR_INTERFACE); }


//! Function IDs for drag and drop
enum
{
	dndmgr_enableDandD, 
	dndmgr_globalEnableDnD, 
	dndmgr_isEnabled, 
	dndmgr_dropPackage, 
	dndmgr_downloadDirectory, 
	dndmgr_downloadPackage,
	dndmgr_downloadUrlToDisk,
	dndmgr_importContextNode,
}; 

/*! \brief Manager class for Drag and Drop
	The Drag and Drop system is managed through a Core FP interface
	(DND_MGR_INTERFACE), defined by this class. It provides control 
	over the drag and drop system, manages handler registration and 
	exposes some useful utility methods for downloading URL's, simulating
	drops, etc.\n\n
	
	The DragAndDropMgr supports multiple DragAndDropHandler's registered 
	against a single HWND window. This allows new components and 3rd-party
	developers to incrementally add handlers for new dropTypes to common 
	windows such as viewports, that the default handlers don't know how to 
	handle.\n\n

	The IDragAndDropMgr::EnableDandD(HWND hwnd, BOOL flag, DragAndDropHandler* handler) 
	method can be called multiple times on the same window with different handler 
	instances. The drag and drop manager keeps track of all the DragAndDropHandlers 
	on each window and will call their drag and drop event methods as needed, 
	in order of registration, until one of them returns <b>S_OK</b>. For
	example, on a viewport, the DefaultDragAndDropHandler is registered by default.
	The event methods (such as DragEnter, DragOver, Drop, etc.) return
	<b>E_FAIL</b> if the dropping IDataObject or DropType is not recognized by
	them. Then, if a new component registers its own handler to deal with some new
	droptype, the default handler will fail to recognize the new droptype
	and so the new handler will be called to process the new drop type.\n\n
	
	Note that this means DragAndDropHandler event methods must correctly return
	<b>S_OK</b> or <b>E_FAIL</b> depending on whether they handle the IDataObject
	or DropType currently dropping, so that the appropriate handler is found and
	called by the DnDMgr.\n\n

	\Note In Max 2011, the management of DragAndDropHandler memory lifetime has 
	been changed. In previous versions, it was up to 3rd party developers to 'unregister' 
	(Passing in FALSE to IDragAndDropMgr::EnableDandD) their drag and drop 
	handlers (DragAndDropHandler*), before max was shutdown. If they didn't max 
	would crash on shutdown. 
	Now, unitialization has been moved up before 3rd party plugins are unloaded. 
	Now, the manager automatically 'unregisters' all DragAndDropHandler pointers during 
	a broadcast of NOTIFY_SYSTEM_SHUTDOWN. This means that the manager calls 
	DragAndDropHandler::Release on all DragAndDropHandler pointers registered with it.
	After the manager cleans up during the NOTIFY_SYSTEM_SHUTDOWN broadcast, any subsequent
	calls to register a drag and drop handler will return FALSE. Also any subsequent
	calls to 'unregister' a handler will just silently do nothing.
	\sa  Class FPStaticInterface,  Class DragAndDropHandler,  Class URLTab
*/
class IDragAndDropMgr : public FPStaticInterface 
{
public:
	/*!	\brief Globally enables or disables the drag and drop interface.
	 *	\param flag -  TRUE to enable, FALSE to disable. */
	virtual void    EnableDandD(BOOL flag)=0;

	/*! \brief returns TRUE if the global drag and drop interface is
	 *	enabled, otherwise FALSE. */
	virtual BOOL   IsEnabled()=0;

	//! \brief Enables drag and drop for a given window (and its children).
	/*! If no custom DragAndDropHandler is supplied, a default one is
	 *  used that will accept dropped scene files for opening and scripts for
	 *  running. When the application is shutting down, it is prohibited to enable
	 *  or disable a handler. Therefore after a NOTIFY_SYSTEM_SHUTDOWN broadcast 
	 *  this method will always return FALSE.
	 *  \param hwnd - A handle to the window you wish to enable or disable drag and drop for.
	 *  \param flag - TRUE to enable, FALSE to disable.
	 *  \param handler - A pointer to a custom drag and drop handler, or NULL to accept a default one.
	 *  \return TRUE if the method was successful, otherwise FALSE. */
	virtual BOOL   EnableDandD(HWND hwnd, BOOL flag, DragAndDropHandler* handler = NULL)=0;

	/*! \brief This method allows the simulation of a package of files into a
	 *	window at a given point. 
	 *	A package of files, specified as a list of URL strings is the common 
	 *	form of DropType data from iDrop sources and files dragged from the 
	 *	Windows desktop. The entire package is downloaded, as needed, but only 
	 *	the first file in the list is actually dropped into 3ds Max. 
	 *	The other files in the package are presumed to be support files, such as
	 *	texmaps or xref sources, for the main drop file.\n\n
	 *	After the drop, the URL strings in the URLTab are converted to
	 *	fully-specified path names to local file copies, if any had to be downloaded
	 *	from the web.
	 *	\param hwnd - A handle to the window. If this is set to NULL, the default 
	 *		3ds Max window	is used.
	 *	\param point - The point at which to drop.
	 *	\param package - A reference to the local copies of the URL strings.
	 *	\return  TRUE if the drop was successful, otherwise FALSE. */
	virtual BOOL   DropPackage(HWND hwnd, POINT& point, URLTab& package)=0; 
	
	/*! Used to download a package of URLs to the specified directory. 
	 *	If the hwnd argument is supplied, any progress or other messages are centered over that
	 *  window.
	 *  \param package - A reference to the local copies of the URL strings.
	 *  \param directory - The directory path string to download to.
	 *  \param hwnd - A handle to the window. If this is set to NULL, the default window is used.
	 *  \param showProgress - The download progress dialog can be displayed by passing true.
	 *  \return  TRUE if the download was successful, otherwise FALSE. */
	virtual BOOL   DownloadPackage(URLTab& package, MCHAR* directory, HWND hwnd = NULL, bool showProgress = false)=0; 

	/*! \brief Returns the fully-specified path to the directory in which package 
	 *	drops are downloaded. */
	virtual MCHAR* GetDownloadDirectory()=0;
	
	/*! \brief Gets the number of handlers associated with the given window.
	 *  \param hwnd -  A handle to the window. */
	virtual int    NumHandlers(HWND hwnd)=0;        // iterate handlers for given window
	
	/*! \brief Returns a pointer to a specified drag and drop hander of a specified window.
	 *	\param hwnd - A handle to the window.
	 *	\param i - The I-th handler. */
	virtual DragAndDropHandler* GetHandler(HWND hwnd, int i)=0;

	/*! \brief Downloads the file referenced by the URL to disk.
	 *	\param hwnd - A handle to the window.
	 *	\param url - The URL string of the file to download.
	 *	\param fileName - The filename string of the URL to store on disk.
	 *	\param dlgflags - Additional controls to the download behavior. 
	 *		Currently only one flag is supported, DOWNLOADDLG_NOPLACE, which 
	 *		hides an option in the progress dialog that allows the user to 
	 *		place (move) a dropped object immediately after being dropped.
	 *	\return  TRUE if the download was successful, otherwise FALSE. */
	virtual bool   DownloadUrlToDisk(HWND hwnd, MCHAR* url, MCHAR* fileName, DWORD dlgflags=0)=0;

	/*! Returns a pointer to the import context node. */
	virtual INode* ImportContextNode()=0;
}; 

#pragma warning(push)
#pragma warning(disable:4100)

/*! \brief The base class from which specialized drag-and-drop handlers 
	should be derived.  */
/*	Instances of these classes can be registered with the drag and drop manager 
	when enabling a window for drag and drop activity. Virtual methods on these
	instances called to handle the various drag and drop events. Use 
	\code 
	IDragAndDropMgr::EnableDnD(hwnd, flag, &myHandler)
	\endcode 
	to enable drag and drop in a window and specify the DragAndDropHandler instance
	that will handle drag and drop events for that window. Note that one 
	DragAndDropHandler instance can potentially be shared among many HWND windows. 
	Further, a parent window can be enabled for drag and drop and this effectively
	causes all child windows to be enabled and handled by the given handler (unless 
	overriden by an explicit <b>EnableDnD</b> with a different handler on a child).\n\n
	
	There are actually two sets of drag and drop event handler virtual methods in the base
	class, a low-level set that equates to the methods on the OLE IDropTarget
	interface that take raw IDataObject drop data and a high-level set that take
	fully-parsed DropType drop data. The low-level methods have default implementations
	provided that do this parsing and call the corresponding high-level method, 
	so in most cases you only need to provide implementations for the high-level methods. 
	You would provide implementations of the low-level methods if custom parsing of the 
	IDataObject is required.
	\sa  Class IDragAndDropMgr, Class DropType
*/
class DragAndDropHandler : public InterfaceServer
{
public:
	//! constructor
	DragAndDropHandler() 
		: current_droptype(NULL)
	{
		if (dndMgr == NULL)
			dndMgr = GetDragAndDropMgr();
	}

	/*! \brief Gets low level control over drag and drop operations.
		Override this method in your DragAndDropHandler subclass to 
		get low-level control over drag and drop operations. This is just a
		redirect of the identical method called on the OLE IDropTarget
		interface, see MSDN docs for details.\n\n
	
		The default implementation for this methods use the DropClipFormat
		and DropType classes to recognize and parse the incoming
		IDataObject into a DropType instance and hand this to the
		associated high-level drag and drop handler methods desribed next.\n\n
		
		As an example, here is the default DragEnter() implementation which 
		does the initial parsing on entry to a window:\n\n

	\code
	HRESULT DragAndDropHandler::DragEnter(HWND hWnd, IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
	{
		current_droptype = NULL;
		// look for one of our accepted clip formats
		DropClipFormat* cf = DropClipFormat::FindClipFormat(pDataObject);
		if (cf != NULL)
		{
			// have one, get it to parse it into a DropType subclass
			current_droptype = cf->ParseDataObject(pDataObject);
			if (current_droptype != NULL)
			{
				// got recognizable drop data,
				// pass on to high-level method
				if(pdwEffect)
					*pdwEffect = DROPEFFECT_LINK|DROPEFFECT_COPY;

				POINT p = { pt.x, pt.y };
				DragEnter(hWnd, current_droptype, grfKeyState, p, pdwEffect);
				return S_OK;
			}
		}
		// nothing for us
		if(pdwEffect)
			*pdwEffect = DROPEFFECT_NONE;
		return S_OK;
	}
	\endcode 
	\param window - The specified handle to the window in which the drag and drop 
		event is occuring. This is one of the windows that was enabled via 
		a IDragAndDropMgr::EnabledDnD() call, so it may be the parent of the
		lowest-level window that the mouse is actually over.
	\param pDataObject - The incoming IDataObject.
	\param grfKeyState - The specified current state of the keyboard modifier 
		keys on the keyboard. Valid values can be a combination of any of the 
		flags MK_CONTROL, MK_SHIFT, MK_ALT, MK_BUTTON, MK_LBUTTON, MK_MBUTTON, and
		MK_RBUTTON.
	\param pt - The specified current cursor coordinates in the coordinate space of the
		drop-target window.
	\param pdwEffect - On entry, pointer to the value of the pdwEffect parameter of the
		DoDragDrop function. On return, must contain one of the effect flags
		from the Win32 DROPEFFECT enumeration, which indicates what the
		result of the drop operation would be.
	\return  Standard return values of E_OUTOFMEMORY, E_INVALIDARG, F_UNEXPECTED, and E_FAIL, S_OK. */
	CoreExport virtual HRESULT DragEnter(HWND window, IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);

	/*! \remarks This method will parse the dropped dataObject.
	\par Parameters:
	<b>HWND window</b>\n\n
	The specified handle to the window in which the drag and drop event is occuring. This
	is one of the windows that was enabled via a
	<b>IDragAndDropMgr::EnabledDnD()</b> call, so it may be the parent of the
	lowest-level window that the mouse is actually over.\n\n
	<b>IDataObject* pDataObject</b>\n\n
	The incoming IDataObject.\n\n
	<b>DWORD grfKeyState</b>\n\n
	The specified current state of the keyboard modifier keys on the keyboard.
	Valid values can be a combination of any of the flags <b>MK_CONTROL,
	MK_SHIFT, MK_ALT, MK_BUTTON, MK_LBUTTON, MK_MBUTTON</b>, and
	<b>MK_RBUTTON</b>.\n\n
	<b>POINTL pt</b>\n\n
	The specified current cursor coordinates in the coordinate space of the
	drop-target window.\n\n
	<b>DWORD* pdwEffect</b>\n\n
	On entry, pointer to the value of the <b>pdwEffect</b> parameter of the
	<b>DoDragDrop</b> function. On return, must contain one of the effect flags
	from the Win32 <b>DROPEFFECT</b> enumeration, which indicates what the
	result of the drop operation would be.
	\return  Standard return values of <b>E_OUTOFMEMORY, E_INVALIDARG,
	F_UNEXPECTED</b>, and <b>E_FAIL</b>, <b>S_OK</b>. */
	CoreExport virtual HRESULT Drop(HWND window, IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
	/*! \remarks This method handles the process of dragging over a drop
	target.
	\par Parameters:
	<b>HWND window</b>\n\n
	The specified handle to the window in which the drag and drop event is occuring. This
	is one of the windows that was enabled via a
	<b>IDragAndDropMgr::EnabledDnD()</b> call, so it may be the parent of the
	lowest-level window that the mouse is actually over.\n\n
	<b>DWORD grfKeyState</b>\n\n
	The specified current state of the keyboard modifier keys on the keyboard.
	Valid values can be a combination of any of the flags <b>MK_CONTROL,
	MK_SHIFT, MK_ALT, MK_BUTTON, MK_LBUTTON, MK_MBUTTON</b>, and
	<b>MK_RBUTTON</b>.\n\n
	<b>POINTL pt</b>\n\n
	The specified current cursor coordinates in the coordinate space of the
	drop-target window.\n\n
	<b>DWORD* pdwEffect</b>\n\n
	On entry, pointer to the value of the <b>pdwEffect</b> parameter of the
	<b>DoDragDrop</b> function. On return, must contain one of the effect flags
	from the Win32 <b>DROPEFFECT</b> enumeration, which indicates what the
	result of the drop operation would be.
	\return  Standard return values of <b>E_OUTOFMEMORY, E_INVALIDARG,
	F_UNEXPECTED</b>, and <b>E_FAIL</b>, <b>S_OK</b>. */
	CoreExport virtual HRESULT DragOver(HWND window, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
	/*! \remarks This method handles the process of the drag operation leaving
	the drop target window.
	\par Parameters:
	<b>HWND window</b>\n\n
	The specified handle to the window in which the drag and drop event is occuring. This
	is one of the windows that was enabled via a
	<b>IDragAndDropMgr::EnabledDnD()</b> call, so it may be the parent of the
	lowest-level window that the mouse is actually over.
	\return  Standard return values of <b>E_OUTOFMEMORY, E_INVALIDARG,
	F_UNEXPECTED</b>, and <b>E_FAIL</b>, <b>S_OK</b>.
	\par Default Implementation:
	<b>{ return E_FAIL; }</b> */
	CoreExport virtual HRESULT DragLeave(HWND window) { return E_FAIL; }

	// high-level drag-and-drop operations, the dataObject has been decoded and the drop-type parsed
	/*! \remarks This is the high-level method called to handle drag and drop events with
	already recognized and parsed data object. Override the above methods as
	needed in your <b>DragAndDropHandler</b> subclass to handle drag and drop events.
	\par Parameters:
	<b>HWND window</b>\n\n
	The specified handle to the window in which the drag and drop event is occuring. This
	is one of the windows that was enabled via a
	<b>IDragAndDropMgr::EnabledDnD()</b> call, so it may be the parent of the
	lowest-level window that the mouse is actually over.\n\n
	<b>DropType* type</b>\n\n
	The specified Pointer to the <b>DropType</b> instance that corresponds to
	the data in the dropped <b>IDataObject</b>. You can use <b>the
	DropType::TypeCode()</b> method to determine the droptype (see the built-in
	codes in the <b>DropType</b> section). Each <b>DropType</b> subclass
	instance has utility methods and public data members containing the parsed
	drop data. See each subclass definition for details.\n\n
	<b>DWORD grfKeyState</b>\n\n
	The specified current state of the keyboard modifier keys on the keyboard.
	Valid values can be a combination of any of the flags <b>MK_CONTROL,
	MK_SHIFT, MK_ALT, MK_BUTTON, MK_LBUTTON, MK_MBUTTON</b>, and
	<b>MK_RBUTTON</b>.\n\n
	<b>POINT\& pt</b>\n\n
	The specified current cursor coordinates in the coordinate space of the
	drop-target window.\n\n
	<b>DWORD* pdwEffect</b>\n\n
	On entry, pointer to the value of the <b>pdwEffect</b> parameter of the
	<b>DoDragDrop</b> function. On return, must contain one of the effect flags
	from the Win32 <b>DROPEFFECT</b> enumeration, which indicates what the
	result of the drop operation would be.
	\return  Standard return values of <b>E_OUTOFMEMORY, E_INVALIDARG,
	F_UNEXPECTED</b>, and <b>E_FAIL</b>, <b>S_OK</b>.
	\par Default Implementation:
	<b>{ return E_FAIL; }</b> */
	virtual HRESULT DragEnter(HWND window, DropType* type, DWORD grfKeyState, POINT& pt, DWORD* pdwEffect) { return E_FAIL; }
	/*! \remarks This method will parse the dropped dataObject.\n\n
	Here's an example implementation of Drop() in the default handler:\n\n
	\code
	HRESULT DefaultDragAndDropHandler::Drop(HWND hwnd, DropType* type, DWORD grfKeyState, POINT& pt, DWORD* pdwEffect)
	{
	// This could take a while, set wait cursor
	HCURSOR hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
	HRESULT result = S_OK;

	// load the dropped data if needed
	if (type->Load()) {
	// see if dropped on a viewport, if so adjust point
	// to be vp-relative
	HWND vpwin = FindDropViewport(hwnd, pt);

	// Handle the drop depending on drop type
	BOOL bRet;
	switch (type->TypeCode()) {
	case SCENEFILE_DROPTYPE:
	bRet = HandleDroppedGeom(
	hwnd, vpwin, pt,
	sceneFileDropType.current_package[0]);
	break;
	case IMAGEFILE_DROPTYPE:
	bRet = HandleDroppedBitmap(
	hwnd, vpwin, pt,
	imageFileDropType.current_package[0]);
	break;
	case DROPSCRIPTEFILE_DROPTYPE:
	bRet = HandleDroppedDropScript(
	hwnd, vpwin, pt,
	dropScriptFileDropType.current_package[0]);
	break;
	}
	result = bRet ? S_OK : E_FAIL;
	}
	// restore cursor
	SetCursor(hOldCursor);
	return result;
	}
	\endcode 
	\par Parameters:
	<b>HWND window</b>\n\n
	The specified handle to the window in which the drag and drop event is occuring. This
	is one of the windows that was enabled via a
	<b>IDragAndDropMgr::EnabledDnD()</b> call, so it may be the parent of the
	lowest-level window that the mouse is actually over.\n\n
	<b>DropType* type</b>\n\n
	The specified Pointer to the <b>DropType</b> instance that corresponds to
	the data in the dropped <b>IDataObject</b>. You can use <b>the
	DropType::TypeCode()</b> method to determine the droptype (see the built-in
	codes in the <b>DropType</b> section). Each <b>DropType</b> subclass
	instance has utility methods and public data members containing the parsed
	drop data. See each subclass definition for details.\n\n
	<b>DWORD grfKeyState</b>\n\n
	The specified current state of the keyboard modifier keys on the keyboard.
	Valid values can be a combination of any of the flags <b>MK_CONTROL,
	MK_SHIFT, MK_ALT, MK_BUTTON, MK_LBUTTON, MK_MBUTTON</b>, and
	<b>MK_RBUTTON</b>.\n\n
	<b>POINT\& pt</b>\n\n
	The specified current cursor coordinates in the coordinate space of the
	drop-target window.\n\n
	<b>DWORD* pdwEffect</b>\n\n
	On entry, pointer to the value of the <b>pdwEffect</b> parameter of the
	<b>DoDragDrop</b> function. On return, must contain one of the effect flags
	from the Win32 <b>DROPEFFECT</b> enumeration, which indicates what the
	result of the drop operation would be.
	\return  Standard return values of <b>E_OUTOFMEMORY, E_INVALIDARG,
	F_UNEXPECTED</b>, and <b>E_FAIL</b>, <b>S_OK</b>.
	\par Default Implementation:
	<b>{ return E_FAIL; }</b> */
	virtual HRESULT Drop(HWND window, DropType* type, DWORD grfKeyState, POINT& pt, DWORD* pdwEffect) { return E_FAIL; }
	/*! \remarks This method handles the process of dragging over a drop
	target.
	\par Parameters:
	<b>HWND window</b>\n\n
	The specified handle to the window in which the drag and drop event is occuring. This
	is one of the windows that was enabled via a
	<b>IDragAndDropMgr::EnabledDnD()</b> call, so it may be the parent of the
	lowest-level window that the mouse is actually over.\n\n
	<b>DWORD grfKeyState</b>\n\n
	The specified current state of the keyboard modifier keys on the keyboard.
	Valid values can be a combination of any of the flags <b>MK_CONTROL,
	MK_SHIFT, MK_ALT, MK_BUTTON, MK_LBUTTON, MK_MBUTTON</b>, and
	<b>MK_RBUTTON</b>.\n\n
	<b>POINT\& pt</b>\n\n
	The specified current cursor coordinates in the coordinate space of the
	drop-target window.\n\n
	<b>DWORD* pdwEffect</b>\n\n
	On entry, pointer to the value of the <b>pdwEffect</b> parameter of the
	<b>DoDragDrop</b> function. On return, must contain one of the effect flags
	from the Win32 <b>DROPEFFECT</b> enumeration, which indicates what the
	result of the drop operation would be.\n\n

	\return  Standard return values of <b>E_OUTOFMEMORY, E_INVALIDARG,
	F_UNEXPECTED</b>, and <b>E_FAIL</b>, <b>S_OK</b>.
	\par Default Implementation:
	<b>{ return E_FAIL; }</b> */
	virtual HRESULT DragOver(HWND window, DWORD grfKeyState, POINT& pt, DWORD * pdwEffect) { return E_FAIL; }

	/*! \remarks This method is called when the drag and drop manager starts managing a window
	for this handler.
	managing drag and drop events for a particular window for this handler. You can
	provide an implementation if you need to keep track of extant uses of the
	handler (say, by ref-counting) or to do handler-specific cleanup.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void Acquire() { }; 
	//! \brief This method is called called during a when the drag and drop manager stops
	//! managing drag and drop events for a particular window for this handler. 
	/*! By default, the drag and drop manager will call this method on all registered
	DragAndDropHandler's during a NOTIFY_SYSTEM_SHUTDOWN broadcast. 
	You should provide an implementation if you need to keep track of 
	extant uses of the handler (say, by ref-counting) or to do handler-specific cleanup.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void Release() { }; 

protected:
	/*! Cache for the currently parsed DropType. This is usually filled in during
		DragEnter() processing in the DragAndDropHandler for the current window. */
	DropType*  current_droptype;
	//! Cached pointer to drag and drop manager
	CoreExport static IDragAndDropMgr* dndMgr; 
};

// class DropClipFormat
// base class for the various supported clipboard formats.
// Distinguished instance of each subclass represents a
// particular IDataObject clip format or package of related formats
// that can be accepted by MAX's various windows.  Prime responsibility
// of each is to recognize its presence in a dropped (pasted?) IDataObject
// and to parse the data object into one of the acceptable DropTypes.
// 
/*! \sa  Class DropType, Class VIZableClipFmt, Class DropScriptClipFmt,  \ref dropTypes.\n
\par Description:
This class is available in release 4.0 and later only.\n\n
<b>DropClipFormat</b> is the base class for the various supported clipboard
formats contained in a dropped <b>IDataObject</b>. Subclasses represent
particular <b>IDataObject</b> clip format or package of related formats that
can be accepted by various windows in 3ds Max. The prime responsibility of each
is to recognize its presence in a dropped <b>IDataObject</b> and to parse the
data object into one of the supported <b>DropTypes</b>. Each subclass should
have a singleton instance created. This is automatically registered with the
drag and drop system for use in the clipform recognition routines.
\par Data Members:
protected:\n\n
<b>static Tab\<DropClipFormat*\> clipFmts;</b>\n\n
The table of supported clip formats.  */
class DropClipFormat : public InterfaceServer
{
protected:
   CoreExport static Tab<DropClipFormat*> clipFmts;  // table of supported clip formats
public:
   // each DropClimFormat instance created is kept in the clipFmts table
   /*! \remarks Constructor.\n\n
   Each <b>DropClimFormat</b> instance created is kept in the <b>clipFmts</b>
   table */
   DropClipFormat() { DropClipFormat* cf = this; clipFmts.Append(1, &cf); }

   // used primarily by the DragEnter() handler to find the DropClipFormat
   // corresponding to the currently dropping IDataObject
   /*! \remarks This method returns a pointer to the <b>DropClipFormat</b>
   (singleton) corresponding to the clip format in the given
   <b>IDataObject</b>, or NULL if the <b>IDataObject</b> contains no
   recognizable formats. This is primarily used by the low-level default
   <b>DragEnter()</b> function in drag and drop manager.
   \par Parameters:
   <b>IDataObject* pDataObject</b>\n\n
   The data object you wish to return the clip format for. */
   CoreExport static DropClipFormat* FindClipFormat(IDataObject* pDataObject);

   // specialized by individual clipformats to detect its format(s) present in 
   // the given IDataObject
   /*! \remarks This method should be implemented by each subclass to detect
   the presence of its clipformat(s) in the given <b>IDataObject</b>. See
   <b>ParseDataObject()</b> below for a detailed example.
   \par Parameters:
   <b>IDataObject* pDataObject</b>\n\n
   The data object.
   \return  TRUE if the data was queries successfully, otherwise FALSE.
   \par Default Implementation:
   <b>{ return false; }</b> */
   virtual bool CheckClipFormat(IDataObject* pDataObject) { return false; }
   
   // specialized by individual clip format types to parse IDataObject
   // into appropriate DropType
   /*! \remarks This method should be implemented by each subclass to parse
   its clipformat(s) in the given <b>IDataObject</b> into the corresponding
   <b>DropType</b> subclass instance. For example, the <b>DropClipFormats</b>
   that accept dropped files will typically return one of the
   <b>FileDropType</b> subclasses depending on the filename suffix. A list of
   built-in clipformats:\n\n
   <b>IDropPackageClipFmt</b>  iDrop XML package\n
   <b>VIZableClipFmt</b>   VIZable file URL\n
   <b>DropScriptClipFmt  </b>internal dropScript\n\n
   Here's an example (simplified) <b>VIZableClipFmt</b> implementation, which
   accepts a custom <b>CF_MAXURL</b> clip format containing the URL of a file.
   <b>CheckClipFormat()</b> returns true if it finds the <b>CF_MAXURL</b>
   clipboard format present in the given <b>IDataObject</b>. Because this is a
   dropping file, <b>ParseDataObject()</b> clears the current droptype data
   (the <b>FileDropType::Init()</b>, extracts the file name from the
   <b>IDataObject</b> and installs it into the <b>FileDropType
   current_package</b> variable. It then asks the <b>FileDropType</b> class to
   recognize the actual file type being dropped and return the corresponding
   <b>FileDropType</b> subclass instance (using
   <b>FileDropType::FindDropType()</b>).\n\n
	\code
	bool VIZableClipFmt::CheckClipFormat(IDataObject* pDataObject)
	{
	// accept CF_MAXURL clip formats
		FORMATETC fmt = { NULL, NULL, DVASPECT_CONTENT, -1, NULL };
		fmt.cfFormat = RegisterClipboardFormat(_M("CF_MAXURL"));
		fmt.tymed = TYMED_HGLOBAL;
		return SUCCEEDED(pDataObject->QueryGetData(&fmt)) ==
			TRUE;
	}
	
	DropType* VIZableClipFmt::ParseDataObject(IDataObject* pDataObject)
	{
	// parse a CF_MAXURL clipformat into one of the FileDropTypes
		&
	// fill in the FileDropType::current_packge URLTab
			HRESULT hr;
		FORMATETC fmt = { NULL, NULL, DVASPECT_CONTENT, -1, NULL };
		STGMEDIUM stg = { TYMED_NULL, NULL, NULL };
	
		fmt.tymed = TYMED_HGLOBAL;
		fmt.cfFormat = RegisterClipboardFormat(_M("CF_MAXURL"));
	
	// clear out the file drop current data
		FileDropType::Init();
	
	// look for CF_MAXURL formats
		hr = pDataObject->GetData(&fmt, &stg);
		if(SUCCEEDED(hr)) {
	// found, get the max file name
			MCHAR szUrl[MAX_PATH];
			ZeroMemory(szUrl, sizeof(szUrl));
			wcstombs(szUrl,
				reinterpret_cast<wchar_t*>(GlobalLock(stg.hGlobal)),
				MAX_PATH-1);
			GlobalUnlock(stg.hGlobal);
			ReleaseStgMedium(&stg);
	// add it to the current_package URLTab
			FileDropType::current_package.Add(szUrl);
		}
	
	// if we have a non-NULL package, get the appropriate
	// FileDropType (usually based on file extension), by asking the
	// utility DropType finder in FileDropType
		if (FileDropType::current_package.Count() > 0)
			return FileDropType::FindDropType(
				FileDropType::current_package[0], pDataObject);
		else
			return NULL;
	}
	\endcode 
   \par Parameters:
   <b>IDataObject* pDataObject</b>\n\n
   The data object.
   \return  A pointer to the drop-type.
   \par Default Implementation:
   <b>{ return NULL; }</b> */
   virtual DropType* ParseDataObject(IDataObject* pDataObject) { return NULL; }
};

// built-in clip formats

// iDrop package URL
/*! \sa  Class DropClipFormat\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the iDrop XML package. */
class IDropPackageClipFmt : public DropClipFormat
{
public:
   bool CheckClipFormat(IDataObject* pDataObject);
   DropType* ParseDataObject(IDataObject* pDataObject);
};

// VIZable file URL
/*! \sa  Class DropClipFormat\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the VIZable File URL. */
class VIZableClipFmt : public DropClipFormat
{
public:
   bool CheckClipFormat(IDataObject* pDataObject);
   DropType* ParseDataObject(IDataObject* pDataObject);
};

// internal dropScript
/*! \sa  Class DropScriptDropType, Class DropClipFormat\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the internal drop script format. */
class DropScriptClipFmt : public DropClipFormat
{
public:
   bool CheckClipFormat(IDataObject* pDataObject);
   DropType* ParseDataObject(IDataObject* pDataObject);
};


// class DropType
// base class for dropable content types
// Distinguished instances of subclasses represent different types of drop content, 
// such as a file distinguished by file suffix or a scene object
// The active DropClipFormat parses dropped IDataObject into one
// of these dropped types, filling its data members with appropriate
// guff from the data object.
/*! \sa  Class DropClipFormat, Class FileDropType, Class DropScriptDropType, \ref dropTypes.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This is the base class for droppable content types. Distinguished instances of
subclasses represent different types of drop content, such as a file
distinguished by file suffix or a scene object The active <b>DropClipFormat</b>
parses dropped <b>IDataObject</b> into one of these dropped types, filling its
data members with appropriate guff from the data object.\n\n
Each <b>DropClipFormat</b> can encompass multiple possible types of dropped
data. For example, the <b>iDrop</b> package can drop max files, image files,
script files, etc. The <b>DropClipFormat</b> classes parse raw dropped
clipboard data into one of the <b>DropType</b> family of classes. These contain
recognizer predicates, extracted data from the current drop, and utilities for
working with the data, such as URL downloads, script compile \& execution, etc.
Custom drop-types can be created by subclassing one of the <b>DropType</b> base
classes. The built-in <b>DropTypes</b> are listed in \ref dropTypes.
\par Data Members:
protected:\n\n
<b>static IDragAndDropMgr* dndMgr;</b>\n\n
Cached pointer to the drag and drop manager.\n\n
<b>static bool dragging;</b>\n\n
The drop source state.\n\n
<b>static POINT startPt;</b>\n\n
The drag and drop starting point.\n\n
<b>static WPARAM startKeyState;</b>\n\n
They starting state of the keyboard.\n\n
<b>static HWND startWnd;</b>\n\n
The handle to the starting window.\n\n
<b>static bool loaded;</b>\n\n
Flags if the current packages is already downloaded.\n\n
<b>static IDataObject* current_dataobject;</b>\n\n
Currently dropping <b>IDataObject</b>. Filled in by the low-level
<b>DragEnter()</b> code.  */
class DropType : public MaxHeapOperators, public IDropSource, public IDataObject
{
protected:
   CoreExport static IDragAndDropMgr* dndMgr;   // cached pointer to drag and drop manager
   CoreExport static bool dragging;       // drop source state...
   CoreExport static POINT startPt;
   CoreExport static WPARAM startKeyState;
   CoreExport static HWND startWnd;
   CoreExport static bool loaded;            // flags if curent package already downloaded

public:
   // currently dropping data object
   CoreExport static IDataObject* current_dataobject;

   /*! \remarks Constructor. */
   DropType() { if (dndMgr == NULL) dndMgr = GetDragAndDropMgr(); }
	 /*! \remarks Destructor. */
	 CoreExport virtual ~DropType();
   
   // clears current parsed drop data
   /*! \remarks This method clears the currently-parsed drop data.
   \par Default Implementation:
   <b>{ current_dataobject = NULL; loaded = false; }</b> */
   static void Init() { current_dataobject = NULL; loaded = false; }

   // DropType code access, provides an integer code specific
   // to the droptype
   /*! \remarks This method returns the typecode of the DropType. */
   virtual int TypeCode()=0;
   /*! \remarks This method returns TRUE if the DropType is of the specified
   DropType code, otherwise FALSE.\n\n
   Each <b>DropType</b> subclass is given a unique integer code that can be
   used for type-testing, switching, etc. The <b>TypeCode()</b> method must be
   implemented to return this code and <b>isDropType()</b> to test against the
   given code (this is provided to that intermediate <b>DropType</b> base
   classes with codes can effectively support superclass testing). The codes
   for the built-in <b>DropTypes</b> are given by the following defined symbols
   which are listed in \ref dropTypes.</a>
   \par Parameters:
   <b>int code</b>\n\n
   The DropType code.
   \par Default Implementation:
   <b>{ return code == TypeCode(); }</b> */
   virtual bool IsDropType(int code) { return code == TypeCode(); }
   
   // ------- drop target methods & members --------------

   // perform any droptype-specific load prior to using the data, eg 
   // downloading URLs to local machine
   /*! \remarks Subclasses should implement this method if they need to
   perform any droptype-specific loading prior to clipformat data use. For
   example, the URL package types all download any web-resident files in this
   method. Control any implemented progress dialog with the showProgress
   parameter.
   \par Parameters:
   <b>bool showProgress = true</b>\n\n
   The download progress dialog can be displayed by passing true.
   \par Default Implementation:
   <b>{ return true; }</b> */
   virtual bool Load(bool showProgress = true) { return true; }

   // dropeffect currently supported by accepted dropping type
   /*! \remarks This method returns the dropeffect currently supported by the
   accepted dropping type.
   \par Default Implementation:
   <b>{ return DROPEFFECT_MOVE; }</b> */
   virtual DWORD DropEffect() { return DROPEFFECT_MOVE; }

   // -------- drop source methods and members -----------------

   // from IUnknown
   CoreExport STDMETHODIMP QueryInterface(REFIID iid, void** ppvObject);
   CoreExport STDMETHODIMP_(ULONG) AddRef(void)  { return 1; }
   CoreExport STDMETHODIMP_(ULONG) Release(void) { return 1; }
   
   // from IDataObject
   CoreExport STDMETHODIMP GetData(FORMATETC* pFormatetc, STGMEDIUM* pmedium) { return E_UNEXPECTED; }
   CoreExport STDMETHODIMP GetDataHere(FORMATETC* pFormatetc, STGMEDIUM* pmedium) { return E_UNEXPECTED; }
   CoreExport STDMETHODIMP QueryGetData(FORMATETC* pFormatetc) { return E_UNEXPECTED; }
   CoreExport STDMETHODIMP GetCanonicalFormatEtc(FORMATETC* pFormatetcIn, FORMATETC* pFormatetcOut);
   CoreExport STDMETHODIMP SetData(FORMATETC* pFormatetc, STGMEDIUM* pmedium, BOOL fRelease);
   CoreExport STDMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatetc);
   CoreExport STDMETHODIMP DAdvise(FORMATETC* pFormatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection);
   CoreExport STDMETHODIMP DUnadvise(DWORD dwConnection);
   CoreExport STDMETHODIMP EnumDAdvise(IEnumSTATDATA** ppenumAdvise);

   // from IDropSource
   CoreExport STDMETHODIMP QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);
   CoreExport STDMETHODIMP GiveFeedback(DWORD dwEffect) { return DRAGDROP_S_USEDEFAULTCURSORS; }

   // drop start checking methods 
   CoreExport virtual void InitDragDropCheck(LPARAM mousePt, WPARAM keyState, HWND hwnd);
   CoreExport virtual void CancelDragDropCheck();
   CoreExport virtual bool DragDropCheck(LPARAM mousePt, WPARAM keyState, DWORD allowedEffects);
   CoreExport virtual bool ReadyToDrag(){ return false; }
};

/*! \defgroup dropTypes DropTypes
The following is the list of built-in DropTypes provided with 3ds Max. The DropTypes are listed by
their class name and DropType code. This type code can be obtained by using the
DropType::TypeCode() method. Third-party DropType subclasses should use random codes above
0x1000000.
\sa Class DropType, Class DragAndDropHandler
*/
//@{
#define FILE_DROPTYPE            0x00000001		//!< A package of file names or URL's.
#define DROPSCRIPT_DROPTYPE         0x00000002	//!< A dropScript.
#define SCENEFILE_DROPTYPE       0x00000003		//!< The *.max scene file.
#define IMAGEFILE_DROPTYPE       0x00000004		//!< The image files (.bmp, .tga, etc.).
#define IMPORTFILE_DROPTYPE         0x00000005	//!< The importable files (.3ds, .dxf, etc.).
#define SCRIPTFILE_DROPTYPE         0x00000006	//!< The script files (.ms, .mse, .mcr).
#define DROPSCRIPTFILE_DROPTYPE     0x00000007	//!< The drop script files (.ds, .dse).
#define BITMAP_DROPTYPE          0x00000008		//!< The bitmap type.
#define MSZIPFILE_DROPTYPE       0x00000009		//!< The script zip package files (.mzp).
#define PATHFILE_DROPTYPE        0x0000000a		//!< The path type.
//@}
/*! \sa  Class DropType, Class URLTab, Class SceneFileDropType, Class ImageFileDropType, Class ImportFileDropType, Class ScriptFileDropType,  \ref dropTypes.\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This is an intermediate base class for drop content that comes in the form of a
package of filenames or URLS. This class maintains a list of all its subclass
singleton instances and provides utility methods for finding an appropriate
subclass instance, based on the dropped filename, and for download URL
packages.
\par Data Members:
protected:\n\n
<b>static Tab\<FileDropType*\> fileDropTypes;</b>\n\n
The table of FileDropTypes.\n\n
<b>static MSTR download_directory;</b>\n\n
Cache for current default URL package download directory .\n\n
public\n\n
<b>static URLTab current_package;</b>\n\n
Currently dropping URL package. Filled in by the active <b>DropClipFormat</b>
in its <b>ParseDataObject()</b> method .  */
class FileDropType : public DropType
{
protected:
   CoreExport static Tab<FileDropType*> fileDropTypes;  // table of supported file drop types
	CoreExport static MSTR download_directory;			  // cache for default download directory

   // URL download utilities
   CoreExport static bool CheckForCachedFile(MCHAR* filename);
   CoreExport static bool IsInternetCachedFile(const MCHAR* filename);
   CoreExport static bool  AppendUrlFilename(const MCHAR* szUrl, MCHAR* szPathname, bool& is_URL);

public:
   CoreExport static URLTab current_package;

   /*! \remarks Constructor. */
   FileDropType() { FileDropType* dt = this; fileDropTypes.Append(1, &dt); }
   /*! \remarks This method clears the currently-parsed drop data.
   \par Default Implementation:
   <b>{ current_package.Clear(); DropType::Init(); }</b> */
   static void Init() { current_package.Clear(); DropType::Init(); }

   // From DropType
   /*! \remarks This method returns the typecode of the DropType.
   \par Default Implementation:
   <b>{ return FILE_DROPTYPE; }</b> */
   int TypeCode() { return FILE_DROPTYPE; }
   /*! \remarks This method returns TRUE if the DropType is of the specified
   DropType code, otherwise FALSE.
   \par Parameters:
   <b>int code</b>\n\n
   The DropType code.
   \par Default Implementation:
   <b>{ return code == TypeCode() || code == FILE_DROPTYPE; }</b> */
   bool IsDropType(int code) { return code == TypeCode() || code == FILE_DROPTYPE; }

   // ------- drop target methods & members --------------

   /*! \remarks This method will load the URLTab in current_package (filled in
   by the current DropClipFormat).
   \par Parameters:
   <b>bool showProgress = true</b>\n\n
   The download progress dialog can be displayed by passing true. */
   CoreExport bool Load(bool showProgress = true);
   
   // global finder of assoicated droptype given filename (or pDataObject)
   /*! \remarks This method finds and returns the <b>FileDropType</b> subclass
   corresponding to the given filename suffix. See \ref dropTypes for more details.
   \par Parameters:
   <b>MCHAR* filename</b>\n\n
   The filename suffix.\n\n
   <b>IDataObject* pDataObject = NULL</b>\n\n
   A pointer to the IDataObject.
   \return  The FileDropType that corresponds to the filename suffix. This
   could be one of the following; <b>sceneFileDropType, imageFileDropType,
   importFileDropType, dropScriptFileDropType</b>. */
   CoreExport static FileDropType* FindDropType(MCHAR* filename, IDataObject* pDataObject = NULL);

   // specialize this for each droppable file type to provide type detector
   /*! \remarks Subclasses should implement this method to recognize the file
   types associated with this drop type. This is used by <b>FindDropType()</b>.
   \par Parameters:
   <b>MCHAR* filename</b>\n\n
   The filename suffix.
   \return  TRUE if the filename suffix checks out, otherwise FALSE.
   \par Default Implementation:
   <b>{ return false; }</b> */
   virtual bool     CheckDropType(MCHAR* filename) { return false; } 

   // package download utilities
   /*! \remarks This method serves as a utility function that can be used to
   download a package of URLs to the specified directory. If the <b>hwnd</b>
   argument is supplied, any progress or other messages are centered over that
   window.
   \par Parameters:
   <b>URLTab\& package</b>\n\n
   A reference to the local copies of the URL strings.\n\n
   <b>MCHAR* directory</b>\n\n
   The directory path string to download to.\n\n
   <b>HWND hwnd = NULL</b>\n\n
   A handle to the window. If this is set to NULL, the default 3ds Max window
   is used.\n\n
   <b>bool showProgress = false</b>\n\n
   The download progress dialog can be displayed by passing true.
   \return  TRUE if the download was successful, otherwise FALSE. */
   CoreExport static bool   DownloadPackage(URLTab& package, MCHAR* szDirectory, HWND hwnd = NULL, bool showProgress = true);
   /*! \remarks This method returns the fully-specified path to the directory
   in which package drops are downloaded. */
   CoreExport static MCHAR* GetDownloadDirectory();
   /*! \remarks This method allows you to download the file referenced by the
   URL to disk.
   \par Parameters:
   <b>HWND hwnd = NULL</b>\n\n
   A handle to the window.\n\n
   <b>MCHAR* url</b>\n\n
   The URL string of the file to download.\n\n
   <b>MCHAR* fileName</b>\n\n
   The filename string of the URL to store on disk.\n\n
   <b>DWORD flags=0</b>\n\n
   Additional controls to the download behavior. Currently only one flag is
   supported, <b>DOWNLOADDLG_NOPLACE</b>, which hides an option in the progress
   dialog that allows the user to place (move) a dropped object immediately
   after being dropped.
   \return  TRUE if the download was successful, otherwise FALSE. */
   CoreExport static bool DownloadUrlToDisk(HWND hwnd, MCHAR* szUrl, MCHAR* szPathname, DWORD flags=0);

   // -------- drop source methods and members -----------------

   // from IDataObject
   STDMETHODIMP GetData(FORMATETC* pFormatetc, STGMEDIUM* pmedium) { return E_UNEXPECTED; }
   STDMETHODIMP GetDataHere(FORMATETC* pFormatetc, STGMEDIUM* pmedium) { return E_UNEXPECTED; }
   STDMETHODIMP QueryGetData(FORMATETC* pFormatetc) { return E_UNEXPECTED; }

};

#pragma warning(pop)


// class DropScriptDropType
//   intermediate base class for drop content in the form of a 
//   dropScript 
/*! \sa  Class DropType, Class DragAndDropHandler, Class FPParams, Class MacroEntry,  \ref dropTypes.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class is an intermediate base class for drop content that comes in the
form of a dropScript. This is a special kind of macroScript that implements
dropScript event handlers (see the DropScript documentation for details.) The
prime subclass is <b>DropScriptFileDropType</b> which recognizes files of type
.ds. The parsed data for this type is a single parsed macroScript, represented
as a <b>MacroEntry</b>pointer. The <b>DropScriptDropType</b> class provides
utility methods for compiling a .ds file into the <b>current_dropscript</b>
slot and for running the drag and drop-associated handlers in the current dropScript.\n\n
The methods <b>RunDropScriptDragEnter(FPParams* params)</b>,
<b>RunDropScriptDragOver(FPParams* params)</b> and
<b>RunDropScriptDrop(FPParams* params)</b> take care of the 'on droppable'
handler in the <b>current_dropscript</b>, if supplied. The
<b>DragAndDropHandler::DragEnter</b> call is usualy made once on initial entry
to a registered drag and drop target window and <b>DragAndDropHandler::DragOver</b> is
usually called as the mouse moves over this window. In both cases, the handler
returns true or false to indicate whether the dropping dropScript will be
accepted. If a handler is not supplied, the dropScript is always deemed
droppable. If the handler returns false, the not-droppable cursor is shown.\n\n
The handler is called with a set of arguments, supplied by the
<b>DragAndDropHandler</b>, that usually depends on the window currently under
the mouse pointer. For example, over a viewport, the current mouse coordinates,
scene node under the mouse, slot number in a list window, etc. By convention,
the first argument is positional and always a window type name, such as
"Viewport" or "MaterialEditor", and all the others are keyword arguments, since
they will vary from window to window. They are delivered to the
RunDropScriptXXX methods in a Function Publishing <b>FPParam</b> object, so
that handler code needs to deal as little as possible with the MAXScript SDK.
Here's an example code fragment from the default drop handler:\n\n
\code
FPParams params (6,
TYPE_NAME, (vpwin ? _M("viewport") : _M("max")),
TYPE_KEYARG_MARKER,
TYPE_NAME, _M("node"),
TYPE_INODE, m_nodectx,
TYPE_NAME, _M("point"),
TYPE_POINT, &pt);
// run the dragEnter handler & set dropeffect based on result
if (dropScriptFileDropType.RunDropScriptDragEnter(&params))
	*pdwEffect = DROPEFFECT_COPY;
else
	*pdwEffect = DROPEFFECT_NONE;
\endcode  
In the above code, the handler is called with 3 actual arguments, one position
and two keyword. They are loaded into the 'params' instance with the
<b>FPParams varargs</b> constructor. The first is the positional window name,
in this case either #viewport or #max, then comes a special
<b>TYPE_KEYARG_MARKER</b> signalling that the following arguments are keyword.
The keyword args are given in pairs, name then value, in this case node: and
point:. See the Function Publishing system documentation for full details on
using the <b>FPParams</b> class for passing parameter blocks. An example
droppable handler might be as follows:\n\n
<b>on droppable window node: do</b>\n\n
<b>return window == #viewport and superclassOf node == Shape</b>\n\n
This handler effectively makes the dropScript droppable if the mouse is over a
Shape object in a viewport window. Notice that the function only looks at the
node: keyword argument in this definition; arguments delivered as keyword
arguments can vary from call to call and the called function can choose to look
at only subset of them.
\par Data Members:
public\n\n
<b>static MacroEntry* current_dropscript;</b>\n\n
Cache for current macroScript. See <b>/MAXSDK/INCLUDE/iMacroScript.h</b> for
the MacroScript manager public API. There are also utility methods in
DropScriptDropType that do all the necessary drag and drop compiling \& running of
macroScripts, so you only have to deal with the MacroScript manager for special
processing.  */
class DropScriptDropType : public DropType
{
public:
   CoreExport static MacroEntry* current_dropscript;

   // From DropType
   /*! \remarks This method returns the typecode of the DropType.
   \par Default Implementation:
   <b>{ return DROPSCRIPT_DROPTYPE; }</b> */
   int TypeCode() { return DROPSCRIPT_DROPTYPE; }
   /*! \remarks This method returns TRUE if the DropType is of the specified
   DropType code, otherwise FALSE.
   \par Parameters:
   <b>int code</b>\n\n
   The DropType code.
   \par Default Implementation:
   <b>{ return code == TypeCode() || code == DROPSCRIPT_DROPTYPE; }</b> */
   bool IsDropType(int code) { return code == TypeCode() || code == DROPSCRIPT_DROPTYPE; }
   /*! \remarks This method returns the dropeffect currently supported by the
   accepted dropping type.
   \par Default Implementation:
   <b>{ return DROPEFFECT_MOVE; }</b>\n\n
   The following methods provide assistance for developing custom drag-and-drop
   handlers that want to accept dropScripts. They work on the shared
   current_dropscript static data member in <b>DropScriptDropType</b>. */
   DWORD DropEffect() { return DROPEFFECT_MOVE; }
   
   // ------- drop target methods & members --------------

   // compile & run support methods
   /*! \remarks This method parses the given file, looking for a single
   macroScript definition. If successful, interns the macroScript and places
   its corresponding <b>MacroEntry*</b> in the <b>current_dropscript</b> static
   data member. Note that if there is more code than just a single macroScript
   in the file, only the last macroScript definition is taken; the other code
   is NOT executed, so you cannot include auxiliary global functions and other
   prep code in the file. These should be inside the body of the macroScript,
   as local data and functions.
   \par Parameters:
   <b>MCHAR* filename</b>\n\n
   The filename of the script.
   \return  TRUE if successfully compiled, otherwise FALSE. */
   CoreExport BOOL CompileDropScript(MCHAR* filename);
   /*! \remarks This methods takes care of the 'on droppable' handler in the
   <b>current_dropscript</b>, if supplied. If the handler returns false, the
   not-droppable cursor is shown.
   \par Parameters:
   <b>FPParams* params</b>\n\n
   The set of arguments for the handler.
   \return  TRUE if droppable script will be accepted, otherwise FALSE. */
   CoreExport BOOL RunDropScriptDragEnter(FPParams* params);
   /*! \remarks This methods takes care of the 'on droppable' handler in the
   <b>current_dropscript</b>, if supplied, during the process of dragging
   contents over the drop target. If the handler returns false, the
   not-droppable cursor is shown.
   \par Parameters:
   <b>FPParams* params</b>\n\n
   The set of arguments for the handler.
   \return  TRUE if droppable script will be accepted, otherwise FALSE. */
   CoreExport BOOL RunDropScriptDragOver(FPParams* params);
   /*! \remarks This methods takes care of the 'on droppable' handler in the
   <b>current_dropscript</b>, if supplied and handles the parsing of the
   dropped script. If the handler returns false, the not-droppable cursor is
   shown.
   \par Parameters:
   <b>FPParams* params</b>\n\n
   The set of arguments for the handler.
   \return  TRUE if droppable script will be accepted, otherwise FALSE. */
   CoreExport BOOL RunDropScriptDrop(FPParams* params);

   // -------- drop source methods and members -----------------

   // from IDataObject
   CoreExport STDMETHODIMP GetData(FORMATETC* pFormatetc, STGMEDIUM* pmedium);
   CoreExport STDMETHODIMP GetDataHere(FORMATETC* pFormatetc, STGMEDIUM* pmedium);
   CoreExport STDMETHODIMP QueryGetData(FORMATETC* pFormatetc);

   // drop start checking methods 
   /*! \remarks This method will initialize a drag and drop check.
   \par Parameters:
   <b>MacroEntry* dropscript</b>\n\n
   The drop script macro entry.\n\n
   <b>LPARAM mousePt</b>\n\n
   The initial mouse cursor position.\n\n
   <b>WPARAM keyState</b>\n\n
   They initial state of the keyboard.\n\n
   <b>HWND hwnd</b>\n\n
   The handle to the initial start window. */
   void InitDragDropCheck(MacroEntry* dropscript, LPARAM mousePt, WPARAM keyState, HWND hwnd)
   {
      DropType::InitDragDropCheck(mousePt, keyState, hwnd);
      current_dropscript = dropscript;
   }
   /*! \remarks This method returns TRUE if the system is ready to drag,
   otherwise FALSE.
   \par Default Implementation:
   <b>{ return current_dropscript != NULL; }</b> */
   bool ReadyToDrag() { return current_dropscript != NULL; }
};

// the built-in type classes

// first the file types, usually sourced by the iDrop active-X control on a 
// web page or by draggin files from the Windows desktop/explorer...

// .max scene file
/*! \sa  Class FileDropType,  \ref dropTypes.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the scene file (*.max) DropType and is available through
the built-in type instance <b>sceneFileDropType</b>.  */
class SceneFileDropType : public FileDropType
{
public:
   // From DropType
   /*! \remarks This method returns the typecode of the DropType.
   \par Default Implementation:
   <b>{ return SCENEFILE_DROPTYPE; }</b> */
   int TypeCode() { return SCENEFILE_DROPTYPE; }
   
   // From FileDropType
   /*! \remarks This method checks if the file type associated with the
   DropType is recognized.
   \par Parameters:
   <b>MCHAR* filename</b>\n\n
   The filename suffix.
   \return  TRUE if the filename suffix checks out, otherwise FALSE. */
   CoreExport bool CheckDropType(MCHAR* filename);
};

// image files (.bmp, .tga, etc.)
/*! \sa  Class FileDropType,  \ref dropTypes.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the image file (*.bmp, *.tga, etc.) DropType and is
available through the built-in type instance <b>imageFileDropType</b>.  */
class ImageFileDropType : public FileDropType
{
public:
   // From DropType
   /*! \remarks This method returns the typecode of the DropType.
   \par Default Implementation:
   <b>{ return IMAGEFILE_DROPTYPE; }</b> */
   int TypeCode() { return IMAGEFILE_DROPTYPE; }
   
   // From FileDropType
   /*! \remarks This method checks if the file type associated with the
   DropType is recognized.
   \par Parameters:
   <b>MCHAR* filename</b>\n\n
   The filename suffix.
   \return  TRUE if the filename suffix checks out, otherwise FALSE. */
   CoreExport bool CheckDropType(MCHAR* filename);
};

// importable files (.3ds, .dxf, etc.)
/*! \sa  Class FileDropType,  \ref dropTypes.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the import file (*.3ds, *.dxf, etc.) DropType and is
available through the built-in type instance <b>importFileDropType</b>.  */
class ImportFileDropType : public FileDropType
{
public:
   // From DropType
   /*! \remarks This method returns the typecode of the DropType.
   \par Default Implementation:
   <b>{ return IMPORTFILE_DROPTYPE; }</b> */
   int TypeCode() { return IMPORTFILE_DROPTYPE; }
   
   // From FileDropType
   /*! \remarks This method checks if the file type associated with the
   DropType is recognized.
   \par Parameters:
   <b>MCHAR* filename</b>\n\n
   The filename suffix.
   \return  TRUE if the filename suffix checks out, otherwise FALSE. */
   CoreExport bool CheckDropType(MCHAR* filename);
};

// script files (.ms, .mse, .mcr)
/*! \sa  Class FileDropType,  \ref dropTypes.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the script file (*.ms, *.mse, *.mcr) DropType and is
available through the built-in type instance <b>scriptFileDropType</b>.  */
class ScriptFileDropType : public FileDropType
{
public:
   // From DropType
   /*! \remarks This method returns the typecode of the DropType.
   \par Default Implementation:
   <b>{ return SCRIPTFILE_DROPTYPE; }</b> */
   int TypeCode() { return SCRIPTFILE_DROPTYPE; }
   
   // From FileDropType
   /*! \remarks This method checks if the file type associated with the
   DropType is recognized.
   \par Parameters:
   <b>MCHAR* filename</b>\n\n
   The filename suffix.
   \return  TRUE if the filename suffix checks out, otherwise FALSE. */
   CoreExport bool CheckDropType(MCHAR* filename);
};

// drop script files (.ds, .dse)
/*! \sa  Class FileDropType,  \ref dropTypes,
Class DropScriptDropType\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the drop script file (*.ds, *.dse) DropType and is
available through the built-in type instance <b>dropScriptFileDropType</b>.
 */
class DropScriptFileDropType : public FileDropType, public DropScriptDropType
{
public:
   // From DropType
   /*! \remarks This method returns the typecode of the DropType.
   \par Default Implementation:
   <b>{ return DROPSCRIPTFILE_DROPTYPE; }</b> */
   int TypeCode() { return DROPSCRIPTFILE_DROPTYPE; }
   /*! \remarks This method will perform any droptype-specific loading.
   \par Parameters:
   <b>bool showProgress = true</b>\n\n
   The download progress dialog can be displayed by passing true. */
   bool Load(bool showProgress = true);
   
   // From FileDropType
   /*! \remarks This method checks if the file type associated with the
   DropType is recognized.
   \par Parameters:
   <b>MCHAR* filename</b>\n\n
   The filename suffix.
   \return  TRUE if the filename suffix checks out, otherwise FALSE. */
   CoreExport bool CheckDropType(MCHAR* filename);
};

// script zip package files (.mzp)
/*! \sa  Class FileDropType, List of DropTypes,
Class DropScriptDropType\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the script zip package file (*.mzp) DropType and is
available through the built-in type instance <b>msZipPackageFileDropType</b>.
\par Data Members:
<b>MSTR extract_dir;</b>\n\n
The path string representing the directory to extract into.\n\n
<b>MSTR drop_file;</b>\n\n
The drop file string.\n\n
<b>DropType* drop_file_type;</b>\n\n
The DropType if the <b>drop_file</b> is not a dropScript.  */
class MSZipPackageFileDropType : public FileDropType, public DropScriptDropType
{
public:
	MSTR extract_dir;
	MSTR drop_file; 
   DropType* drop_file_type;   // if drop_file is not a dropScript

   // From DropType
   /*! \remarks This method returns the typecode of the DropType.
   \par Default Implementation:
   <b>{ return MSZIPFILE_DROPTYPE; }</b> */
   int TypeCode() { return MSZIPFILE_DROPTYPE; }
   /*! \remarks This method will perform any droptype-specific loading.
   \par Parameters:
   <b>bool showProgress = true</b>\n\n
   The download progress dialog can be displayed by passing true. */
   bool Load(bool showProgress = true);
   
   // From FileDropType
   /*! \remarks This method checks if the file type associated with the
   DropType is recognized.
   \par Parameters:
   <b>MCHAR* filename</b>\n\n
   The filename suffix.
   \return  TRUE if the filename suffix checks out, otherwise FALSE. */
   CoreExport bool CheckDropType(MCHAR* filename);
};

// MAX-internal types

// bitmap
/*! \sa  Class FileDropType, \ref dropTypes. \n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the bitmap DropType and is available through the built-in
type instance <b>bitmapDropType</b>.  */
class BitmapDropType : public DropType
{
public:
   // From DropType
   /*! \remarks This method returns the typecode of the DropType.
   \par Default Implementation:
   <b>{ return BITMAP_DROPTYPE; }</b> */
   int TypeCode() { return BITMAP_DROPTYPE; }
   
};

//! \brief A DropType class specific to path configuration files

//! Used by the DragAndDropManager to help determine the 
//! type of operation that should be performed given a specific
//! drop operation.
class PathConfigDropType : public FileDropType
{
public:
   //! \brief Returns the drop type ID.

   int TypeCode() { return PATHFILE_DROPTYPE; }

   //! \brief Checks to see if the passed in file is handled by this drop type.

    //! This methods checks if this is a path configuration file, by
   //! examining the file's extension.  ( == .mxp)
    //! \param[in] filename The filename of the file dropped.
    //! \return \b true if this is a .mxp file
   CoreExport bool CheckDropType(MCHAR* filename);
};

// built-in type instances
extern CoreExport SceneFileDropType sceneFileDropType; 
extern CoreExport ImageFileDropType imageFileDropType; 
extern CoreExport ScriptFileDropType scriptFileDropType; 
extern CoreExport DropScriptFileDropType dropScriptFileDropType; 
extern CoreExport DropScriptDropType dropScriptDropType; 
extern CoreExport BitmapDropType bitmapDropType; 
extern CoreExport MSZipPackageFileDropType msZipPackageFileDropType; 
extern CoreExport PathConfigDropType pathConfigDropType;


