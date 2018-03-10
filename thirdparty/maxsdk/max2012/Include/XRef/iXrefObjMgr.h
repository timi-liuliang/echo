//**************************************************************************/
// Copyright (c) 1998-2005 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Legacy Object Xref SDK
// AUTHOR: Nikolai Sander - created July.07.2000
//***************************************************************************/
#pragma once

#include "..\ifnpub.h"
#include "..\GetCOREInterface.h"
// forward declarations
class IXRefObject;
namespace MaxSDK
{
	namespace AssetManagement
	{
		class AssetUser;
	}
}

//! \brief Legacy interface for managing object xrefs
/*! \remarks This interface should be considered obsolete. Please use IObjXRefManager8 instead.
Note that this interface does not provide full support for multiple object xref records/links 
created from the same source file. 
\see IObjXRefManager8
*/
/*! <h2>Note:</h2> In 3ds Max 8 this class was replaced with
class IObjXRefManager8. It is
still documented for backwards compatibility.  
\sa
Class IXRefObject, Class FPStaticInterface, \ref Reference_Messages \n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the interface to the Object XRef Manager. You can obtain
a pointer to the Object XRef Manager interface using; <b>IObjXRefManager*
GetObjXRefManager()</b>. This macro will return\n\n
<b>(IObjXRefManager*)GetCOREInterface(OBJXREFMANAGER_INTERFACE).</b>\n\n
All methods of this class are Implemented by the System.  */
class IObjXRefManager : public FPStaticInterface 
{
	public:
	
	// function IDs 
	enum { 
		fnIdAddXRefObject,
		fnIdGetNumXRefObjects,
		fnIdGetXRefObject,
		fnIdGetNumFiles,
		fnIdGetFileName,
		fnIdReloadFile,
		fnIdIsFileUnresolved,
		fnIdIsFileDisabled,
		fnIdGetAllXRefObjects,
	};
	
	/*! \remarks  This method allows you to add an XRef object to the scene by
	providing the file and object name.
	\par Parameters:
	<b>const MaxSDK::AssetManagement::AssetUser &asset</b>\n\n
	The file asset of the scene containing the object to add.\n\n
	<b>MCHAR *obname</b>\n\n
	The name of the object in the scene.\n\n
	\return  A pointer to the newly added xref object, or NULL in case the file or
	name were invalid. */
	virtual IXRefObject *AddXRefObject(const MaxSDK::AssetManagement::AssetUser& asset, MCHAR *obname, int xFlags = 0)=0;

	/*! \remarks  This method returns the number if IXRefObjects that reference
	a certain file name. It is possible to iterate over the IXRefObjects of a
	certain file by using GetXRefObject.
	\par Parameters:
	<b>const MaxSDK::AssetManagement::AssetUser &asset</b>\n\n
	The file name you wish to check references for. */
	virtual int GetNumXRefObjects(const MaxSDK::AssetManagement::AssetUser &asset)=0;

	/*! \remarks  This method returns a pointer to the I-th IXRefObject that
	references a certain file.
	\par Parameters:
	<b>const MaxSDK::AssetManagement::AssetUser &asset</b>\n\n
	The file you wish to check references for.\n\n
	<b>int i</b>\n\n
	The index of the object. */
	virtual IXRefObject *GetXRefObject(const MaxSDK::AssetManagement::AssetUser &asset, int i)=0;
	
	/*! \remarks  This method will return the number of files that contain
	object XRef's. */
	virtual int GetNumFiles()=0;
	/*! \remarks  This method returns the file asset of the file that contains
	the specified object XRef.
	\par Parameters:
	<b>int fidx</b>\n\n
	The index of the object XRef for which to get the file. */
	virtual const MaxSDK::AssetManagement::AssetUser &GetFile(int fidx)=0;
	/*! \remarks  This method will reload the file represented by the specified asset.
	\par Parameters:
	<b>const MaxSDK::AssetManagement::AssetUser &asset</b>\n\n
	The file name to reload.
	\return  TRUE if the reload was successful, otherwise FALSE. */
	virtual BOOL ReloadFile(const MaxSDK::AssetManagement::AssetUser &asset)=0;
	
	/*! \remarks  This method returns whether or not the specified file asset is
	unresolved.
	\par Parameters:
	<b>const MaxSDK::AssetManagement::AssetUser &asset</b>\n\n
	The file name to check.
	\return  TRUE if the file is unresolved, otherwise FALSE. */
	virtual BOOL IsFileUnresolved(const MaxSDK::AssetManagement::AssetUser &asset)=0;

	/*! \remarks  This method returns whether or not the specified file name is
	disabled.
	\par Parameters:
	<b>const MaxSDK::AssetManagement::AssetUser &asset</b>\n\n
	The file name to check.
	\return  TRUE if the file is disabled, otherwise FALSE. */
	virtual BOOL IsFileDisabled(const MaxSDK::AssetManagement::AssetUser &asset)=0;
	/*! \remarks  This method fills the IXRefObject table with all XrefObjects that
	are in the scene.
	\par Parameters:
	<b>Tab\<IXRefObject*\> \&objs</b>\n\n
	The table in which to receive all IXRefObject's in the scene. */
	virtual void GetAllXRefObjects(Tab<IXRefObject*> &objs)=0;
};

#define OBJXREFMANAGER_INTERFACE Interface_ID(0x7ede1c65, 0x353d271f)
inline IObjXRefManager* GetObjXRefManager () { return (IObjXRefManager*)GetCOREInterface(OBJXREFMANAGER_INTERFACE); }

