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
// DESCRIPTION: Interface for working with xref records
// AUTHOR: Attila Szabo - created Dec.10.2004
//***************************************************************************/

#pragma once

#include "..\iFnPub.h"
#include "iXrefItem.h"
#include "..\tabtypes.h"

// --- Forward decls
class ReferenceTarget;
class ClassDesc;

// --- Type definitions
//! \brief Xref record handle type. 
/*! Xref record pointers could be invalidated by operations performed on xref records or xref items,
thus it is not recommended to cache them for later use. Xref record handles overcome this
limitation. They can be cached and used for retrieving xref records in a safe way.
Xref record handles are guaranteed to be unique for all xref records within the current 
scene, but they are not persisted so any handles cached by client code become invalid 
after a new file is loaded.  Client code should not persist these handles.
*/
typedef unsigned long XRefRecordHandle;

//! \brief This represents the value of an invalid xref record handle
const XRefRecordHandle kNullXRefRecordHandle = 0;

//! Id for IObjXRefRecord interface
#define IID_XREF_OBJ_RECORD Interface_ID(0x1a457bd1, 0x7584ff0)

//! \brief Class for managing an object xref record
/*! Xref records store xref items and information regarding the xref operation/transaction 
that created them. Xref records are created by and can be retrieved from the Object Xref Manager 
(see class IObjXRefManager8). Xref records are not created when xrefing a whole scene via the 
scene xref feature.
*/
class IObjXRefRecord : public FPMixinInterface
{
public:
	//! \name XRef Item Access
	//@{
	//! \brief Retrieves the number of xref items stored in this record. 
	/*! \param [in] itemType - One of the predefined xref item types IXRefItem::XRefItemType 
	\return - The number of xrefed items of the specified type. 
	*/
	virtual unsigned int ItemCount(IXRefItem::XRefItemType itemType) const = 0;
	
	//! \brief Retrieves all the xref items of the specified type.
	/*! Note that any xref atmospheric item returned by this method will become invalid
	if all other types of items are removed from this record. Also, any of of the xref items returned 
	by this method could become invalid after an operation that modifies the contents of the xref record.
	\param [in] itemType - A combination of predefined xref item types IXRefItem::XRefItemType
	\param [in, out] xrefItems - An array where the xref items will be returned.
	The array is not cleared, so its existent elements will be preserved.
	\return - The number of returned xref items 
	*/
	virtual unsigned int GetItems(IXRefItem::XRefItemType itemType, Tab<ReferenceTarget*>& xrefItems) const = 0;

 	//! \brief Retrieves the i-th xref item of the specified type.
	/*! \param [in] i - The index of the xref item to retrieve. 
	\param [in] itemType - One of the predefined xref item types IXRefItem::XRefItemType
	\return - Pointer to the xref item. NULL if the supplied index (i) is out of range, 
	or if the supplied item type is unsupported
	*/
	virtual ReferenceTarget* GetItem(unsigned int i, IXRefItem::XRefItemType itemType) const = 0;

	//! \brief Allows to verify whether this record contains any xref items.
	/*! Note that empty xref records are automatically deleted by the system, so
	this method should never return true, except when called before the system had
	a chance to delete empty xref records. Also note that xref records containing 
	only xref atmospherics will be deleted automatically by the system.
	\return - true if the record does not contain any xref items of any type, otherwise false
	*/
	virtual bool IsEmpty() const = 0;
	//@}

	/*! \name Nested XRef Record Management
	When xrefing scene entities that are themselves xrefs, nested xref records are created. 
	The following methods allow finding and working with nested xref records.
	*/
	//@{
	//! \brief Retrieves all child xref records of this record.
	/*! \param [out] childRecords - Array of xref records where the child records are retured. 
	The supplied array is not	cleared out before the xref records are added to it. 
	The array is guaranteed not to contain duplicate xref records.
	\return - The number of child xref records. 
	*/
	virtual unsigned int GetChildRecords(Tab<IObjXRefRecord*>& childRecords) const = 0;
	//! \brief Retrieves all parent xref records of this record.
	/*! \param [out] parentRecords - Array of xref records where the parent records are returned. 
	The supplied array is not	cleared out before the xref records are added to it. 
	The array is guaranteed not to contain duplicate xref records.
	\return - The number of parent xref records. 
	*/
	virtual unsigned int GetParentRecords(Tab<IObjXRefRecord*>& parentRecords) const = 0;
	//! \brief Retrieves all root (top-level) xref records of this record.
	/*! \param [out] rootRecords - Array of xref records where the root records are returned. 
	The supplied array cleared out before the xref records are added to it. 
	The array is guaranteed not to contain duplicate xref records.
	\return - The number of root xref records. 
	*/
	virtual unsigned int GetRootRecords(Tab<IObjXRefRecord*>& rootRecords) const = 0;

	//! \brief Retrives whether this xref record has parent records or not.
	/*! This method uses a more efficient way of determining whether this xref record
	is nested or not than calling GetParentRecords.
	\return - true if this xref record is nested, i.e. has at least one parent record, otherwise false.
	*/
	virtual bool IsNested() const = 0;
	//@}

	/*! \name XRef Record State Access
	The following methods allow for setting and retrieving options used when xrefing scene entities.
	These options apply to the whole xrefing operation rather than to individual xref items.
	*/
	//! \brief Retrives the options used to create this xref record.
	/*! These options cannot be modifed once the xref record has been created.
	\return The options used to create the xref record. 
	See \ref XRefObjectOptions for a full list of available options.
	\see IObjXRefManager8::XRefAllObjectsFromFile
	*/
	virtual unsigned int GetXRefOptions() const = 0;

	//! \brief Retrieves whether all scene entities are xrefed from this record's source file 
	/*! When this option is turned on, the next update operation of this xref record
	will xref all scene entities that have not been xrefed yet from the its source file.
	\return - true if this should be xrefing all scene entities from its source file, otherwise false
	*/
	virtual bool IsIncludeAll() const = 0;
	//! \brief Sets the xrefing of all scene entities from this record's source file.
	/*! \param [in] bEnable - If true, the include-all option is enabled, otherwise it's disabled
	*/
	virtual void SetIncludeAll(bool bEnable) = 0;

	//! \brief Retrieves the enabled state of this xref record.
	/*! Enabled xref records can be updated, while disabled xref records cannot.
	\return - true if this xref record is enabled, otherwise false
	\see IObjXRefRecord::Update
	*/
	virtual bool IsEnabled() const = 0;
	//! \brief Sets the enabled state of this xref record.
	/*! When disabling an xref record, the source items of its xref items are un-loaded
	from memory. Note that opening a master file containing disabled xref records results 
	in unresolved xref items. Re-enabling and updating the record, re-resolves the xref items.
	\param [in] bEnable - If true, the xref record is enabled, otherwise it's disabled
	*/
	virtual void SetEnable(bool bEnable) = 0;

	//! \brief Retrieves whether auto-update is enabled for this xref record.
	/*! When Auto-update is enabled, the xref record is automatically updated 
	as soon as the source file changes.
	\return - true if this xref record is has auto-update turned on, otherwise false
	*/
	virtual bool IsAutoUpdate() const = 0;
	//! \brief Sets the state of the auto-upate for this xref record.
	/*! \param [in] bEnable - If true, auto-update is enabled for this xref record, 
	otherwise it's disabled.
	*/
	virtual void SetAutoUpdate(bool bEnable) = 0;

	//! \brief Allows to verify whether the xref record is up-to-date.
	/*! The xref record is up-to-date if it's been updated with the source items from 
	the latest version of the source file. This method will report the current 
	value of the up-to-date state of this xref record. To trigger a verification 
	of whether the source file of this xref record has changed, use CheckSrcFileChanged.
	\return - true if the record is up-to-date, otherwise false
	*/
	virtual bool GetUpToDate() const = 0;
	//! \brief Allows to set an xref record as out-of-date. 
	/*! Only out-of-date records are processed by an update request.
	\param [in] bUpToDate - false to set the record as ou-of-date, otherwise true
	*/
	virtual void SetUpToDate(bool bUpToDate) = 0;
	//! \brief Verifies whether the source file of this xref record has changed since 
	//! the record was last updated. 
	/*! Calling this method checks whether the source file is unresolved. If it isn't, 
	it updates the up-to-date flag of this xref record by comparing the date and time of
	the source file with the date and time the xref record was last updated.
	\return - true if the unresolved state, or the up-to-date state have changed, 
	false if neither have changed
	*/
	virtual bool CheckSrcFileChanged() = 0;
	
	//! \brief Allows to verify whether the xref record is unresolved. 
	/*! An xref record is unresolved if the source file name and path is not valid. 
	Unresolved xref records are not discarded by the system because they may get
	re-resolved on a subsequent update of the xref record if the source file 
	path and name becomes valid.
	\return - true if this is an unresolved xref record, otherwise false
	*/
	virtual bool IsUnresolved() const = 0;

	//! \brief Retrieves the asset Id of the source file.
	/*! The source file of an xref record is the file from where the source scene entities 
	xrefed via this xref record come from.
	\return - The asset user of the file that the source scene entity comes 
	from or or NULL if there's no source file specified.
	*/
	virtual const MaxSDK::AssetManagement::AssetUser& GetSrcFile() const = 0;

	//! \brief Returns the parent Xref record source file name
	//! \return the parent source file name.
	virtual const MSTR& GetParentSrcFileName()const = 0;
	//@}

	//! \name XRef Record Management
	//@{
	//! \brief Updates the xref record by reloading its xref items from the source file.
	/*! The xref entities are reloaded according to the currently set options. For example, 
	the xref record will not be updated if it is disabled.	
	\note Updating an xref record will trigger the updating of all nested xref records.
	\note Updating an xref record with the Include All flag On for itself and all its child 
	records, will not result in xrefing scene entities newly added to nested source files.
	The child xref records saved with the intermediate master files must be update first, 
	starting with the deepest child or intermediate file.
	\return - true if the operation succeeded, otherwise false.
	*/
	virtual bool Update() = 0;

	//! \brief Retrieves the record's unique handle.
	/*! Each xref record is identified by a unique handle. The handle value is 
	unique only within the current scene. Reseting the scene is considered to 
	start a new scene. The handle of the xref records is not persisted. 
	\return - The handle of this xref record.
	\see XRefRecordHandle
	*/
	virtual XRefRecordHandle GetHandle() const = 0;
	//@}
	
	//! \brief Retrieves the IObjXRefRecord interface from an InterfaceServer.
	/*! \param [in] is - Pointer to InterfaceServer
	\return - Pointer to the IObjXRefRecord interface of the scene entity or NULL if it's not an xref record
	*/
	static IObjXRefRecord* GetInterface(InterfaceServer* is) {
		DbgAssert(is != NULL);
		return static_cast<IObjXRefRecord*>(is->GetInterface(IID_XREF_OBJ_RECORD));
	};

	// --- From FPMixinInterface
	// Retrieves the unique if of this interface class.
	virtual Interface_ID GetID() { return IID_XREF_OBJ_RECORD; }
	// Provides access to the metadata used to expose this interface class to maxscript
	virtual FPInterfaceDesc* GetDesc() { return &mFPInterfaceDesc; }

	// IDs of function published methods
	enum FPFuncIDs 	
	{ 
		kfpItemCount,
		kfpIsEmpty,
		kfpGetItem,
		kfpGetItems,
		kfpGetChildRecords, 
		kfpGetParentRecords,
		kfpGetRootRecords,
		kfpIsNested,
		kfpGetIncludeAll, kfpSetIncludeAll,
		kfpGetAutoUpdate, kfpSetAutoUpdate,
		kfpGetEnabled, kfpSetEnabled,
		kfpGetUpToDate, kfpSetUpToDate,
		kfpCheckSrcFileChanged,
		kfpIsUnresolved,
		kfpGetSrcFileName, kfpSetSrcFileName,
		kfpGetSrcFileAssetId, kfpSetSrcFileAssetId,
		kfpUpdate,
		kfpGetRecordId,
		kfpGetHandle,
		kfpGetXRefOptions,
	};

	// IDs of enumeration types used by function published methods
	enum FPEnums 	
	{ 
		kfpXRefItemTypes,
		kfpXRefItemTypesNoAll,
		kfpXrefOptions,
	};
#pragma warning(push)
#pragma warning(disable:4238)
	BEGIN_FUNCTION_MAP
		FN_1(kfpItemCount,	TYPE_DWORD,	FPItemCount,	TYPE_ENUM_TAB_BR);
		FN_2(kfpGetItem,	TYPE_REFTARG,	FPGetItem,	TYPE_INDEX, TYPE_ENUM);
		FN_2(kfpGetItems,	TYPE_DWORD,	FPGetItems,	TYPE_ENUM_TAB_BR, TYPE_REFTARG_TAB_BR);
		FN_1(kfpGetChildRecords,	TYPE_DWORD,	FPGetChildRecords,	  TYPE_INTERFACE_TAB_BR);
		FN_1(kfpGetParentRecords,	TYPE_DWORD,	FPGetParentRecords,	  TYPE_INTERFACE_TAB_BR);
		FN_1(kfpGetRootRecords,	TYPE_DWORD,	FPGetRootRecords,	  TYPE_INTERFACE_TAB_BR);
		FN_0(kfpCheckSrcFileChanged,	TYPE_bool,	CheckSrcFileChanged);
		FN_0(kfpUpdate,	TYPE_bool,	Update);
		PROP_FNS(kfpGetIncludeAll, IsIncludeAll, kfpSetIncludeAll, SetIncludeAll, TYPE_bool);
		PROP_FNS(kfpGetAutoUpdate, IsAutoUpdate, kfpSetAutoUpdate, SetAutoUpdate, TYPE_bool);
		PROP_FNS(kfpGetEnabled, IsEnabled, kfpSetEnabled, SetEnable, TYPE_bool);
		PROP_FNS(kfpGetUpToDate, GetUpToDate, kfpSetUpToDate, SetUpToDate, TYPE_bool);
		RO_PROP_FN(kfpIsEmpty, IsEmpty, TYPE_bool);
		RO_PROP_FN(kfpIsUnresolved, IsUnresolved, TYPE_bool);
		PROP_FNS(kfpGetSrcFileName, FPGetSrcFileName, kfpSetSrcFileName, FPSetSrcFileName, TYPE_FILENAME);
		RO_PROP_FN(kfpGetHandle, GetHandle, TYPE_DWORD);
		RO_PROP_FN(kfpGetXRefOptions, FPGetXRefOptions, TYPE_ENUM_TAB_BV);
		RO_PROP_FN(kfpIsNested, IsNested, TYPE_bool);
	END_FUNCTION_MAP
#pragma warning(pop)
private:
	static FPInterfaceDesc mFPInterfaceDesc;
	unsigned int FPItemCount(IntTab& itemTypes) const;
	ReferenceTarget* FPGetItem(size_t i, unsigned int itemType) const;
	unsigned int FPGetItems(IntTab& itemTypes, Tab<ReferenceTarget*>& xrefItems) const;
	unsigned int FPGetChildRecords(Tab<FPInterface*>& childRecords) const;
	unsigned int FPGetParentRecords(Tab<FPInterface*>& parentRecords) const;
	unsigned int FPGetRootRecords(Tab<FPInterface*>& rootRecords) const;
	IntTab IObjXRefRecord::FPGetXRefOptions() const;
	void FPSetSrcFileName(const MCHAR* fileName);
	const MCHAR*  FPGetSrcFileName();
};



