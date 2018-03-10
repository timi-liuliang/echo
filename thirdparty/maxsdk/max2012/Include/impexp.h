/**********************************************************************
 *<
	FILE: impexp.h

	DESCRIPTION: Includes for importing and exporting geometry files

	CREATED BY:	Tom Hudson

	HISTORY: Created 26 December 1994

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/


#pragma once

#include <WTypes.h>
#include "maxheap.h"
#include "buildver.h"
#include "strbasic.h"

// forward declarations
class ImpInterface;
class ExpInterface;
class Interface;

// Returned by DoImport, DoExport
#define IMPEXP_FAIL 0
#define IMPEXP_SUCCESS 1
#define IMPEXP_CANCEL 2

// SceneImport::ZoomExtents return values
#define ZOOMEXT_NOT_IMPLEMENTED	-1		// The default (uses Preferences value)
#define ZOOMEXT_YES				TRUE	// Zoom extents after import
#define ZOOMEXT_NO				FALSE	// No zoom extents

// The scene import/export classes.  Right now, these are very similar, but this may change as things develop

/*! \sa  Class ImpInterface, Class Interface.\n\n
\par Description:
This is a base class for creating file import plug-ins. The plug-in implements
methods of this class to describe the properties of the import plug-in and a
method that handles the actual import process.  */
class SceneImport : public MaxHeapOperators
{
public:
	/*! \remarks Constructor. */
	SceneImport() {};
	/*! \remarks Destructor. */
	virtual	~SceneImport() {};

	/*! \remarks Returns the number of file name extensions supported by the plug-in. */
	virtual int				ExtCount() = 0;

	/*! \remarks Returns the 'i-th' file name extension (i.e. "3DS").
	\par Parameters:
	<b>int i</b>\n\n
	The index of the file name extension to return. */
	virtual const MCHAR *	Ext(int n) = 0;					// Extension #n (i.e. "3DS")
	/*! \remarks Returns a long ASCII description of the file type being imported (i.e.
	"Autodesk 3D Studio File"). */
	virtual const MCHAR *	LongDesc() = 0;					// Long ASCII description (i.e. "Autodesk 3D Studio File")
	/*! \remarks Returns a short ASCII description of the file type being imported (i.e. "3D
	Studio"). */
	virtual const MCHAR *	ShortDesc() = 0;				// Short ASCII description (i.e. "3D Studio")
	/*! \remarks Returns the ASCII Author name. */
	virtual const MCHAR *	AuthorName() = 0;				// ASCII Author name

	/*! \remarks Returns the ASCII Copyright message for the plug-in. */
	virtual const MCHAR *	CopyrightMessage() = 0;			// ASCII Copyright message
	/*! \remarks Returns the first message string that is displayed. */
	virtual const MCHAR *	OtherMessage1() = 0;			// Other message #1
	/*! \remarks Returns the second message string that is displayed. */
	virtual const MCHAR *	OtherMessage2() = 0;			// Other message #2
	/*! \remarks Returns the version number of the import plug-in. The format is the version
	number * 100 (i.e. v3.01 = 301). */
	virtual unsigned int	Version() = 0;					// Version number * 100 (i.e. v3.01 = 301)

	/*! \remarks This method is called to have the plug-in display its "About..." box.
	\par Parameters:
	<b>HWND hWnd</b>\n\n
	The parent window handle for the dialog. */
	virtual void			ShowAbout(HWND hWnd) = 0;		// Show DLL's "About..." box

	/*! \remarks This method actually performs the file import.
	\par Parameters:
	<b>const MCHAR *name</b>\n\n
	The file name chosen by the user to import.\n\n
	<b>ImpInterface *ii</b>\n\n
	An import interface pointer that may be used to create objects and nodes in
	the scene.\n\n
	<b>Interface *i</b>\n\n
	Pass the 3ds Max interface pointer here.\n\n
	<b>BOOL suppressPrompts=FALSE</b>\n\n
	This parameter is available in release 2.0 and later only.\n\n
	When TRUE, the plug-in must not display any dialogs requiring user input.
	It is up to the plug-in as to how to handle error conditions or situations
	requiring user input. This is an option set up for the 3ds Max API in order
	for plug-in developers to create batch import plugins which operate
	unattended. See <b>Interface::ImportFromFile()</b>.
	\return  One of the following three values should be returned\n\n
	<b>#define IMPEXP_FAIL 0</b>\n\n
	<b>#define IMPEXP_SUCCESS 1</b>\n\n
	<b> #define IMPEXP_CANCEL 2</b> */
	virtual int				DoImport(const MCHAR *name,ImpInterface *ii,Interface *i, BOOL suppressPrompts=FALSE) = 0;	// Import file

	/*! \remarks	This method is used to control the zoom extents done after the import is
	accomplished. It returns a value that indicates if the plug-in should
	override the user preference setting.\n\n
	Also see the method <b>Interface::GetImportZoomExtents()</b> which returns
	the state of the system zoom extents flag.
	\return  One of the following values:\n\n
	<b>ZOOMEXT_NOT_IMPLEMENTED</b>\n\n
	Indicates to use the preference setting.\n\n
	<b>ZOOMEXT_YES</b>\n\n
	Indicates to do a zoom extents after import regardless of the preference
	setting.\n\n
	<b>ZOOMEXT_NO</b>\n\n
	Indicates to <b>not</b> do a zoom extents regardless of the preference
	setting.
	\par Default Implementation:
	<b>{ return ZOOMEXT_NOT_IMPLEMENTED; }</b> */
	virtual int				ZoomExtents() { return ZOOMEXT_NOT_IMPLEMENTED; }	// Override this for zoom extents control
	};

// SceneExport::DoExport options flags:
#define SCENE_EXPORT_SELECTED (1<<0)

/*! \sa  Class ExpInterface, Class Interface.\n\n
\par Description:
This is a base class for creating file export plug-ins. The plug-in implements
methods of this class to describe the properties of the export plug-in and a
method that handles the actual export process.  */
class SceneExport : public MaxHeapOperators
{
public:
	/*! \remarks Constructor. */
	SceneExport() {};
	/*! \remarks Destructor. */
	virtual	~SceneExport() {};
	/*! \remarks Returns the number of file name extensions supported by the plug-in. */
	virtual int				ExtCount() = 0;

	/*! \remarks Returns the 'i-th' file name extension (i.e. "3DS").
	\par Parameters:
	<b>int i</b>\n\n
	The index of the file name extension to return. */
	virtual const MCHAR *	Ext(int n) = 0;					// Extension #n (i.e. "3DS")
	/*! \remarks Returns a long ASCII description of the file type being exported (i.e.
	"Autodesk 3D Studio File"). */
	virtual const MCHAR *	LongDesc() = 0;					// Long ASCII description (i.e. "Autodesk 3D Studio File")

	/*! \remarks Returns a short ASCII description of the file type being exported (i.e. "3D
	Studio"). */
	virtual const MCHAR *	ShortDesc() = 0;				// Short ASCII description (i.e. "3D Studio")

	/*! \remarks Returns the ASCII Author name. */
	virtual const MCHAR *	AuthorName() = 0;				// ASCII Author name
	/*! \remarks Returns the ASCII Copyright message for the plug-in. */
	virtual const MCHAR *	CopyrightMessage() = 0;			// ASCII Copyright message
	/*! \remarks Returns the first message string that is displayed. */
	virtual const MCHAR *	OtherMessage1() = 0;			// Other message #1

	/*! \remarks Returns the second message string that is displayed. */
	virtual const MCHAR *	OtherMessage2() = 0;			// Other message #2
	/*! \remarks Returns the version number of the export plug-in. The format is the version
	number * 100 (i.e. v3.01 = 301). */
	virtual unsigned int	Version() = 0;					// Version number * 100 (i.e. v3.01 = 301)
	/*! \remarks This method is called to have the plug-in display its "About..." box.
	\par Parameters:
	<b>HWND hWnd</b>\n\n
	The parent window handle for the dialog. */
	virtual void			ShowAbout(HWND hWnd) = 0;		// Show DLL's "About..." box

	/*! \remarks This method is called for the plug-in to perform its file export.
	\par Parameters:
	<b>const MCHAR *name</b>\n\n
	The export file name.\n\n
	<b>ExpInterface *ei</b>\n\n
	A pointer the plug-in may use to call methods to enumerate the scene.\n\n
	<b>Interface *i</b>\n\n
	An interface pointer the plug-in may use to call methods of 3ds Max.\n\n
	<b>BOOL suppressPrompts=FALSE</b>\n\n
	This parameter is available in release 2.0 and later only.\n\n
	When TRUE, the plug-in must not display any dialogs requiring user input.
	It is up to the plug-in as to how to handle error conditions or situations
	requiring user input. This is an option set up for the 3ds Max API in order
	for plug-in developers to create batch export plugins which operate
	unattended. See <b>Interface::ExportToFile()</b>.\n\n
	<b>DWORD options=0</b>\n\n
	This parameter is available in release 3.0 and later only.\n\n
	In order to support export of selected objects (as well as future
	enhancements), this method now has this additional parameter. The only
	currently defined option is:\n\n
	<b>SCENE_EXPORT_SELECTED</b>\n\n
	When this bit is set the export module should only export the selected
	nodes.
	\return  One of the following three values should be returned\n\n
	<b>#define IMPEXP_FAIL 0</b>\n\n
	<b>#define IMPEXP_SUCCESS 1</b>\n\n
	<b> #define IMPEXP_CANCEL 2</b> */
	virtual int				DoExport(const MCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0) = 0;	// Export file

	/*! \remarks	This method is called by 3ds Max to determine if one or more export options
	are supported by a plug-in for a given extension. It should return TRUE if
	all option bits set are supported for this extension; otherwise FALSE.\n\n
	Note that the method has a default implementation defined in order to
	provide easy backward compatibility. It returns FALSE, indicating that no
	options are supported.
	\par Parameters:
	<b>int ext</b>\n\n
	This parameter indicates which extension the options are being queried for,
	based on the number of extensions returned by the
	<b>SceneExport::ExtCount()</b> method. This index is zero based.\n\n
	<b>DWORD options</b>\n\n
	This parameter specifies which options are being queried, and may have more
	than one option specified. At present, the only export option is
	<b>SCENE_EXPORT_SELECTED</b>, but this may change in the future. If more
	than one option is specified in this field, the plugin should only return
	TRUE if all of the options are supported. If one or more of the options are
	not supported, the plugin should return FALSE.
	\par Default Implementation:
	<b>{return FALSE;}</b> */
	virtual BOOL			SupportsOptions(int ext, DWORD options) { UNUSED_PARAM(ext); UNUSED_PARAM(options); return FALSE;} 
};


