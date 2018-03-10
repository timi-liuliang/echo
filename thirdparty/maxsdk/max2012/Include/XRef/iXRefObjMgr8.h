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
// DESCRIPTION: Object XRef SDK. Should be used instead of IObjXRefManager (see iXRefObjMgr.h)
// AUTHOR: Attila Szabo - created Dec.10.2004
//***************************************************************************/

#pragma once
#include "..\iFnPub.h"
#include "..\maxapi.h"
#include "iXRefObjRecord.h"

// Forward declarations
class IXRefItem;
class IXRefProxy;
class IXRefObject8;

//! Interface id for IObjXRefManager8
#define IID_OBJ_XREF_MGR8 Interface_ID(0x38575e56, 0x3b852a63)

//! \brief Class for managing object xrefs 
/*! This interface allows for creating, destroying and managing xref records and xref items.
Scene entities such as objects and materials must be xrefed using methods of this
interface. Information related to xref operation and the xref items (IXRefItem) themselves
are stored in xref records (IObjXRefRecord). The object xref manager must also be used 
for modifying and deleting xref items and records, in order to ensure the consistency and 
validity of the remaining xref data.
This interface is implemented by the system (3ds max). To obtain a pointer 
to the object xref manager, call IObjXRefManager8::GetInstace()

Terms used with xrefs:
\li Source file: The file from which scene entities are xrefed
\li Master file: The file into which scene entities of a source file are xrefed
\li XRef item: A master file scene entity that xrefs a specific source file scene entity 
\li Source item: A source file scene entity being xrefed into a master file
\li XRef record: Data structure within the master file that holds xref items and other
data describing an object xref operation or transaction

\see IObjXRefManager, IObjXRefRecord, IXRefItem
\remarks This interface should be used instead of IObjXRefManager.
*/
class IObjXRefManager8 : public FPStaticInterface 
{
public:
	//! \name XRef Record Management
	//@{
	//! \brief Retrieves the number of object xref records 
	/*! \return - the number of object xref records 
	*/
	virtual unsigned int RecordCount() const = 0;
	//! \brief Retrieves the i-th xref record
	/*! \param [in] i - The index of the xref record to retrieve. 
	\return - Pointer to the i-th xref record. NULL if parameter i is greater than the number of currently stored xref records.
	*/
	virtual IObjXRefRecord* GetRecord(unsigned int i) const = 0;

	//! \brief Retrieves an xref record based on its unique handle.
	/*! \param [in] xrefRecHandle - The xref record handle based on which to retrieve an xref record.
	\return - The record with the specified handle, or NULL if no record with the specified handle exists.
	\see XRefRecordHandle
	*/
	virtual IObjXRefRecord* FindRecord(XRefRecordHandle xrefRecHandle) const = 0;

	//! \brief Determines whether two xref records can be combined.
	/*! The specified xref records must meet the following criteria:
	\li Must be top-level (non-nested) xref records (see IObjXRefRecord::GetParentRecords), 
	\li Have the same source file (see IObjXRefRecord::GetSrcFile), 
	\li Have the same creation flags (see IObjXRefRecord::GetXRefOptions)
	The Include All, Auto-Update and Enabled flags of the xref records that are combined
	do not need to be the same, since these flags can be modified any time.
	\param [in] firstRecord - The first record to be combined with the second one
	\param [in] secondRecord - The second record to be combined with the first one
	\return true if the two records can be combined, false otherwise.
	*/
	virtual bool CanCombineRecords(
		const IObjXRefRecord& firstRecord, 
		const IObjXRefRecord& secondRecord) const = 0;

	//! \brief Combines several xref records into one single record.
	/*! The xref items of all xref records that meet the criteria described at 
	IObjXRefManager8::CanCombineRecords will be combined into the first xref record in the 
	list that is a top level xref record. All resulting empty xref records will be deleted.
	\param [in] xrefRecords - The xref records to be combined.
	\return The xref record that holds the xref items of all combined records, 
	or NULL if none of the records was combined.
	*/
	virtual IObjXRefRecord* CombineRecords(Tab<IObjXRefRecord*>& xrefRecords) = 0;

	//! \brief Removes an xref record from the scene
	/*! All xref items of this record are removed from the scene and the record is destroyed.
	\param [in] xrefRecord - The xref record to be removed
	\return - true if the operation was successful, false otherwise
	*/
	virtual bool RemoveRecordFromScene(const IObjXRefRecord& xrefRecord) = 0;

	//! \brief Merges an xref record into the scene
	/*! All xref items of this record are merged into the scene and the record is destroyed.
	\param [in] xrefRecord - The xref record to be merged
	\return - true if the operation was successful, false otherwise
	*/
	virtual bool MergeRecordIntoScene(const IObjXRefRecord& xrefRecord) = 0;

	//! Sets a new file path and name for the specified xref record
	/*! Call this method in order to change the source file of all xref items within a
	top-level (non-nested) xref record.
	\param [in] xrefRecord - The top-level xref record to be changed. This xref record becomes invalid 
	once this method returns, thus client code should not try to use it.
	\param [in] srcFileName - The path and name of the new source file. Must be a non-empty string.
	This method checks that the supplied file path and name is valid for xrefing.
	\return - true if the xref record has been successfully updated, false otherwise. 
	*/
	virtual bool SetRecordSrcFile(IObjXRefRecord& xrefRecord, const MaxSDK::AssetManagement::AssetUser& srcFileName) = 0;

	//! \brief Updates all xref records
	/*! This method updates the up-to-date state of all xref records that are 
	enabled and not auto-updating. Then it reloads all the records that are out-of-date,
	bringing them all up-to-date.	Nested records are also reloaded. 
	\return - true if at least one xref record has been reloaded, false otherwise.
	*/
	virtual bool UpdateAllRecords() = 0;
	//@}

	//! \name XRef Item Management
	//@{
	//! \brief XRefs objects from the specified source file.
	/*!	This method allows for xrefing scene entities from the specified source file. 
	The resulting xref items are stored in a new xref record.	The xrefing of scene entities
	is driven by object names. Currently only objects and materials applied to objects can be xrefed. 
	In order to xref objects from the source file, their names need to be specified via the objNames parameter. 
	In order to xref all objects from the specified file, turn off prompting for objects to be xrefed and 
	pass no object names (pass NULL as objNames or an empty list). 
	This method uses the path resolution rules implemented by IPathConfigMgr::GetFullFilePath to
	resolve the supplied path for the source file name. It also checks whether the source file
	is valid for xrefing: it is not xrefed already directly or indirectly (as a nested xref record).
	\param [in] srcFileName - The path and name of the file the objects to be xrefed live in.
	\param [in] promptObjNames - If true, the user can interactively specify the objects to get xrefed, 
	and the objNames parameter is ignored. If false, the objNames parameter controls what gets xrefed.
	\param [in] objNames - A list with names of objects (nodes) to xref. If no names are specified
	or this parameter is NULL, the promptObjNames parameter controls the behaviour of the method.
	\param [in] xrefOptions - Options gouverning how objects are xrefed. See \ref XRefObjectOptions
	for a full list of available options.
	\return - Pointer to the newly created xref record, or NULL if the operation was not successful.
	\see IPathConfigMgr::GetFullFilePath
	*/
	virtual IObjXRefRecord* AddXRefItemsFromFile(
		const MaxSDK::AssetManagement::AssetUser& srcFileName, 
		bool promptObjNames,
		const Tab<MCHAR*>* objNames,
		unsigned int xrefOptions) = 0;

	//! \brief Adds new xref items to an existent xref record from the record's source file.
	/*!	In order to xref only certain objects, specify their name via the objNames parameter.
	In order to xref all objects not xrefed yet, leave the objNames parameter NULL. The xref 
	options and the source file name of the specified xref record are used to create the new xref objects.
	New xref items can be added only to top-level xref records that are resolved (not unresolved). 
	See IObjXRefRecord for a discussion of nested xref records.
	\param [in] xrefRecord- The xref record to which the new xref objects will be added
	\param [in] promptObjNames - If true, the user can interactively specify the objects to get xrefed, 
	and the objNames parameter is ignored. If false, the objNames parameter controls what gets xrefed.
	\param [in] objNames - A list with names of objects (nodes) to xref. If no names are specified
	or this parameter is NULL, all objects in the source file that aren't xrefed yet will get xrefed.
	Otherwise, the specified objects are xrefed only, whether already xrefed or not.
	\return - true if the operation was successful, false otherwise.
	\see IObjXRefRecord::XRefModOptions, IPathConfigMgr::GetFullFilePath
	*/
	virtual bool AddXRefItemsToXRefRecord(
		IObjXRefRecord& xrefRecord,
		bool promptObjNames,
		const Tab<MCHAR*>* objNames) = 0;

	//! \brief Removes the specified xref items from the scene.
	/*! This method simplifies the task of removing several xref items at once. 
	\li Removing an xref object from the scene, removes all the nodes that reference it. 
	\li Xref Atmospherics referencing xref gizmos are removed from the scene when all their gizmos are removed.\n
	\li An xref material is not removed from the scene by this method; it is rather merged into the scene. 
	It is not necessary to pass as parameters all nested xref items wrapping a source scene entity. 
	It sufices to pass only one xref item in the chain of	nested xref items. 
	This method will remove all parent and child items of the	specified xref item.
	\param [in] xrefItems - The xref items to be removed from the scene
	\return - True if the operation was successful, false otherwise
	*/
	virtual bool RemoveXRefItemsFromScene(const Tab<ReferenceTarget*>& xrefItems) = 0;

	//! \brief Merges the specified xref items into the scene.
	/*! This method merges the source scene entities of the specified xref items into the current scene by. 
	\li Merging an xref object into the scene, leaves all the nodes
	that reference it into the scene and the object can be edited in the modifier panel.
	\li Xref Atmospherics referencing xrefed gizmos that are merged, are also merged into	the scene. 
	\li Merging xref materials into the current scene leaves the source material on the nodes
	to which the xref material was applied.\n
	It is not necessary to pass as parameters all nested xref items wrapping a source scene entity. 
	It sufices to pass only one xref item in the chain of	nested xref items. 
	This method will merge all parent and child items of the	specified xref item.
	\param [in] xrefItems - The xref items to be merged into the scene
	\return - True if the operation was successful, false otherwise
	*/
	virtual bool MergeXRefItemsIntoScene(const Tab<ReferenceTarget*>& xrefItems) = 0;

	//! \brief Applies or re-applies an xref material to a list of xref objects.
	/*! This method iterates through the list of xref objects and applies/re-applies an xref
	material on each node that references each object. The xref material that is applied will
	reference the material on the xref object's source. If an appropriate xref material already exists
	in the record that contains the xref object, it is re-used; otherwise, a new xref material is
	created and added to the record. 
	Note: If the source object has no material, then this method will _remove_ any existing material 
	on the xref object's nodes.
	\param [in] objectXRefItems - The xref objects on which the xref material is to be applied.
	*/
	virtual void ApplyXRefMaterialsToXRefObjects(const Tab<ReferenceTarget*>& objectXRefItems) = 0;

	//! \brief Applies or re-applies an xref controller to a list of xref objects.
	/*! This method iterates through the list of xref objects and applies/re-applies an xref
	controller on each node that references each object. The xref controller that is applied will
	reference the controller on the xref object's source. If an appropriate xref controller already exists
	in the record that contains the xref object, it is re-used; otherwise, a new xref controller is
	created and added to the record. 
	\param objectXRefItems - The xref objects on which the xref controller is to be applied.
	*/
	virtual void ApplyXRefControllersToXRefObjects(const Tab<ReferenceTarget*>& objectXRefItems) = 0;

	//! \brief Determines whether an Item can have its Offset matrix being reset
	/*! For this to be true, the given xref item must meet the following criteria:
	\li Must be a controller, 
	\li Must be from a top-level (non-nested) xref record (see IObjXRefRecord::GetParentRecords) 
	\param inXRefItem - The XRef item we want to test against.
	\return true if the given item is an XRef controller for which the offset can be reset, false otherwise.
	*/
	virtual bool CanResetXRefControllersPRSOffset(ReferenceTarget& inXRefItem) const = 0;

	//! \brief Set XRef controllers' offset matrix back to identity, and if any,
	//! set the corresponding source controller's transformation matrix accordingly.
	/*! This method iterates through the list of XRef items, and for any non-nested XRef controller
	that is found, it will set its offset matrix back to identity. If the XRef controller is resolved,
	i.e. it has a source controller, it will also set this one's transformation matrix accordingly.
	\param inXRefItems - Collected set of XRef items.
	\return true if at least one XRef controller has its offset been reset, false otherwise.
	*/
	virtual bool ResetXRefControllersPRSOffset(const Tab<ReferenceTarget*>& inXRefItems) = 0;

	//! Sets a new scene entity for the specified xref item
	/*! Call this method in order to change the source scene entity being xrefed by	an xref item. 
	The specified scene entity will be xrefed from the scene the xref record owning the
	the specified xref item is pointing to.
	\note XRef Atmospherics are should not be modified using this method since they are
	xrefed and updated when their xrefed gizmos are modified. Thus, this method ignores 
	XRef Atmospherics and return false.
	\param [in] xrefItem - The xref item to be changed. XRef Atmospherics should not be passed as parameter.
	\param [in] srcItemName - The name of the new source entity. Must be a non-empty string
	\return - true if the xref item has been successfully updated, false otherwise. 
	If the xref item has not been successfully updated, IXRefItem::GetSrcItem will return NULL.
	\see IObjXRefRecord::Update, IXRefItem::GetSrcItem
	*/
	virtual bool SetXRefItemSrcName(ReferenceTarget& xrefItem, const MCHAR* srcItemName) = 0;

	//! Sets a new scene entity for the specified xref proxy
	/*! Call this method in order to change the proxy scene entity being xrefed by an xref proxy. 
	The specified scene entity will be xrefed from the scene the xref record owning the
	the specified xref proxy is pointing to.
	\param [in] xrefProxy - The xref proxy to be changed. 
	\param [in] proxyName - The name of the new proxy scene entity. Must be a non-empty string
	\return - true if the proxy has been successfully updated, false otherwise. 
	If the proxy has not been successfully updated, IXRefItem::GetSrcItem will return NULL.
	\see IObjXRefRecord::Update, IXRefItem::GetSrcItem
	*/
	virtual bool SetProxyItemSrcName(ReferenceTarget& xrefProxy, const MCHAR* proxyName) = 0;

	//! Sets a new file for the specified xref item
	/*! Call this method in order to change the source file a scene entity is xrefed from.
	Xref records could be removed if they become empty as a result of executing this method.
	\param [in] xrefItem - The xref item to be changed. 
	\param [in] srcAsset - The new source file. This method checks that the supplied file is valid for xrefing.
	\return - true if the source item has been successfully updated, false otherwise. 
	If the source item has not been successfully updated, IXRefItem::GetSrcItem will return NULL.
	\see IObjXRefRecord::Update, IXRefItem::GetSrcItem
	*/
	virtual bool SetXRefItemSrcFile(ReferenceTarget& xrefItem, const MaxSDK::AssetManagement::AssetUser& srcAsset) = 0;

	//! Sets a new file for the specified xref proxy
	/*! Call this method in order to change the file a proxy scene entity is xrefed from.
	Xref records could be removed if they become empty as a result of executing this method.
	\param [in] xrefProxy - The xref proxy to be changed. 
	\param [in] proxyAsset - The new proxy file. This method checks that the supplied file is valid for xrefing.
	\return - true if the xref proxy has been successfully updated, false otherwise. 
	If the source item has not been successfully updated, IXRefItem::GetSrcItem will return NULL.
	\see IObjXRefRecord::Update, IXRefItem::GetSrcItem
	*/
	virtual bool SetProxyItemSrcFile(ReferenceTarget& xrefProxy, const MaxSDK::AssetManagement::AssetUser& proxyAsset) = 0;
	//@}

	//! \name Miscellaneous
	//@{
	//! \brief Gets the action taken during xrefing when a source object has a 
	//! name that is already used by a master scene object.
	/*! \return One of the action values listed under \ref DupNodeNameActions "Duplicate Node Name Actions"
	*/
	virtual unsigned int GetDupObjNameAction() const = 0;
	//! \brief Sets the action to be taken during xrefing when a source object has a 
	//! name that is already used by a master scene object.
	/*! \param [in] action - One of the action values listed under \ref DupNodeNameActions "Duplicate Node Name Actions"
	*/
	virtual void SetDupObjNameAction(unsigned int action) = 0;

	//! \brief Gets the action taken during xrefing when a source material has a 
	//! name that is already used by a master scene material.
	/*! \return One of the action values listed under \ref DupMtlNameActions "Duplicate Material Name Actions"
	*/
	virtual unsigned int GetDupMtlNameAction() const = 0;
	//! \brief Gets the action taken during xrefing when a source material has a 
	//! name that is already used by a master scene material.
	/*! \param [in] action - One of the action values listed under \ref DupMtlNameActions "Duplicate Material Name Actions"
	*/
	virtual void SetDupMtlNameAction(unsigned int action) = 0;

	//! \brief Determines if a node is xrefed
	/*! A node is considered to be xrefed if any part of it's geometry pipeline is xrefed.
	\param [in] node - The node to be checked whether it's an xref
	\return - Pointer to the first xref object in the geometry pipeline of the node.
	Use IXRefObject8::GetInterface(InterfaceServer& is) to get an IXRefObject8 pointer
	from the returned interface pointer.
	*/
	virtual IXRefObject8* IsNodeXRefed(INode& node) const = 0;
	//@}

	//! \name XRef options
	//! These functions affect the default settings for creating records through the UI.
	//! SDK or maxscript methods to create xrefs (\see AddXRefItemsFromFile) are unaffected.
	//@{
	//! \brief Sets the option of merging the transform controller of nodes when xrefing objects. 
	/*!	Merged controllers (as opposed to xref'd controllers) will be accessible and
		modifiable in the master scene.  Some references between merged and xref'd items
		of xref'd nodes may be lost.
		\param[in] in_merge - whether nodes will have merged controllers in the master scene.
		*/
	virtual void SetMergeTransforms(bool in_merge) = 0;
	//! \brief Returns the current setting of whether the transform controllers of nodes are merged when xrefing objects. 
	/* \see SetMergeTransforms */
	virtual bool GetMergeTransforms() const = 0;

	//! \brief Sets the option of merging the materials of nodes when xrefing objects. 
	/*!	Merged materials (as opposed to xref'd materials) will be accessible and
		modifiable in the master scene.  Some references between merged and xref'd items
		of xref'd nodes may be lost.
		\param[in] in_merge - whether nodes will have merged materials in the master scene.
		*/
	virtual void SetMergeMaterials(bool in_merge) = 0;
	//! \brief Returns the option of merging the material of nodes when xrefing objects. 
	/* \see SetMergeMaterials */
	virtual bool GetMergeMaterials() const = 0;

	//! \brief Sets the option of merging the manipulators of nodes when xrefing objects. 
	/*!	Merged manipulators (as opposed to xrefed manipulators) will be accessible and
		modifiable in the master scene.  Some references between merged and xrefed items
		of xref'd nodes may be lost.
		\param[in] in_merge - whether nodes will have merged manipulators in the master scene.
		*/
	virtual void SetMergeManipulators(bool in_merge) = 0;
	//! \brief Returns the option of merging the material of nodes when xrefing objects. 
	/* \see SetMergeMaterials */
	virtual bool GetMergeManipulators() const = 0;

	//! \brief Sets the option of merging the modifiers of nodes when xrefing objects. 
	/*!	Merged modifiers (as opposed to xref'd modifiers) will be accessible and
		modifiable in the master scene.  Some references between merged and xref'd items
		of xref'd nodes may be lost.
		\param[in] in_merge - how modifiers will appear in the master scene.  
		Possible values are (\ref XRefObjectOptions ):
		- XREF_XREF_MODIFIERS: modifiers will be contained within the XRef object.  They will not be accessible 
		in the modifier stack and therefore will not be editable. 
		- XREF_MERGE_MODIFIERS: modifiers on the XRef object (in the original file) are merged into the scene. 
		They can be changed in the modifier stack. However, these changes are not reflected back into 
		the XRef file.
		- XREF_DROP_MODIFIERS: any modifiers assigned to the XRef object (in the original file) are disregarded 
		and the base object is brought into the scene as an XRef object
		*/
	virtual void SetMergeModifiers(int in_merge) = 0;
	//! \brief Sets the handling of modifiers of nodes when xrefing objects. 
	/* \see SetMergeModifiers */
	virtual int GetMergeModifiers() const = 0;

	//! \brief Sets whether all nodes of a file will be added to a new xref record. 
	/*!	Sets whether all nodes of a file will be added to a new xref record, or if the user will pick 
		among the nodes in that scene through a dialog.
		\param[in] in_include - whether all nodes will be added to the record.
		*/
	virtual void SetIncludeAll(bool in_include) = 0;
	//! \brief Returns the option of merging the material of nodes when xrefing objects. 
	/* \see SetMergeMaterials */
	virtual bool GetIncludeAll() const = 0;


	//! \brief Sets whether the content of the xref record will update on source file update. 
	/*!	Sets whether the content of the xref record will update on source file update or only on demand.
		\param[in] in_autoUpdate - whether the record udpdates automatically or on demand.
		*/
	virtual void SetAutoUpdate(bool in_autoUpdate) = 0;
	//! \brief Returns the option of merging the material of nodes when xrefing objects. 
	/* \see SetMergeMaterials */
	virtual bool GetAutoUpdate() const = 0;

	//@}
	//! \brief Retrieves a pointer to the IObjXRefManager8 interface
	/*! \return IObjXRefManager8* - pointer to the object xref manager 
	*/
	static IObjXRefManager8* GetInstance() {
		return static_cast<IObjXRefManager8*>(GetCOREInterface(IID_OBJ_XREF_MGR8)); 
	}
	

	//! Function IDs used by function publishing
	enum { 
		kfpRecordCount,
		kfpGetRecord,
		kfpFindRecord,
		kfpRemoveRecordFromScene,
		kfpMergeRecordIntoScene,
		kfpSetRecordSrcFile,
		kfpUpdateAllRecords,
		kfpSetXRefItemSrcName,
		kfpSetProxyItemSrcName,
		kfpSetXRefItemSrcFile,
		kfpSetProxyItemSrcFile,
		kfpRemoveXRefsFromScene,
		kfpMergeXRefsIntoScene,
		kfpAddXRefItemsToXRefRecord,
		kfpAddXRefItemsFromFile,
		kfpCanCombineRecords,
		kfpCombineRecords,
		kfpApplyXRefMaterialsToXRefObjects,
		kfpGetDupObjNameAction,	kfpSetDupObjNameAction,
		kfpGetDupMtlNameAction,	kfpSetDupMtlNameAction,
		kfpIsNodeXRefed,
		kfpApplyXRefControllersToXRefObjects,
		kfpResetXRefControllersPRSOffset,
		kfpCanResetXRefControllersPRSOffset,

		// Nicolas Leonard April 5, 06
		kfpGetMergeTransforms, kfpSetMergeTransforms,
		kfpGetMergeMaterials, kfpSetMergeMaterials,
		kfpGetMergeManipulators, kfpSetMergeManipulators,
		kfpGetMergeModifiers, kfpSetMergeModifiers,
		kfpGetIncludeAll, kfpSetIncludeAll,
		kfpGetAutoUpdate, kfpSetAutoUpdate,
	};

	//! IDs of enumeration types used by function published methods
	enum FPEnums 	
	{ 
		kfpXrefOptions,
		kfpDupNodeNameActions,
		kfpDupMaterialNameActions,
		kfpXrefModifierOptions
	};

	BEGIN_FUNCTION_MAP
		RO_PROP_FN(kfpRecordCount,	RecordCount, TYPE_DWORD);
		PROP_FNS(kfpGetDupObjNameAction, GetDupObjNameAction, kfpSetDupObjNameAction, SetDupObjNameAction, TYPE_ENUM);
		PROP_FNS(kfpGetDupMtlNameAction, GetDupMtlNameAction, kfpSetDupMtlNameAction, SetDupMtlNameAction, TYPE_ENUM);
		FN_1(kfpGetRecord,	TYPE_INTERFACE,	GetRecord,	TYPE_INDEX);
		FN_1(kfpFindRecord,	TYPE_INTERFACE,	FindRecord,	TYPE_DWORD);
		FN_1(kfpRemoveRecordFromScene,	TYPE_bool,	FPRemoveRecordFromScene,	TYPE_INTERFACE);
		FN_1(kfpMergeRecordIntoScene,	TYPE_bool,	FPMergeRecordIntoScene,	TYPE_INTERFACE);
		FN_2(kfpSetRecordSrcFile,	TYPE_bool,	FPSetRecordSrcFile,	TYPE_INTERFACE, TYPE_FILENAME);
		FN_0(kfpUpdateAllRecords,	TYPE_bool,	UpdateAllRecords);
		FN_2(kfpSetXRefItemSrcName,	TYPE_bool,	SetXRefItemSrcName,	TYPE_REFTARG_BR, TYPE_STRING);
		FN_2(kfpSetProxyItemSrcName,	TYPE_bool,	SetProxyItemSrcName,	TYPE_REFTARG_BR, TYPE_STRING);
		FN_2(kfpSetXRefItemSrcFile,	TYPE_bool,	FPSetXRefItemSrcFile,	TYPE_REFTARG_BR, TYPE_FILENAME);
		FN_2(kfpSetProxyItemSrcFile,	TYPE_bool,	FPSetProxyItemSrcFile,	TYPE_REFTARG_BR, TYPE_FILENAME);
		FN_1(kfpRemoveXRefsFromScene,	TYPE_bool,	RemoveXRefItemsFromScene,	TYPE_REFTARG_TAB_BR);
		FN_1(kfpMergeXRefsIntoScene,	TYPE_bool,	MergeXRefItemsIntoScene,	TYPE_REFTARG_TAB_BR);
		FN_3(kfpAddXRefItemsToXRefRecord, TYPE_bool, FPAddXRefItemsToXRefRecord,	TYPE_INTERFACE, TYPE_bool, TYPE_STRING_TAB);
		FN_4(kfpAddXRefItemsFromFile, TYPE_INTERFACE, FPAddXRefItemsFromFile,	TYPE_FILENAME, TYPE_bool, TYPE_STRING_TAB, TYPE_ENUM_TAB_BR);
		FN_2(kfpCanCombineRecords,	TYPE_bool,	FPCanCombineRecords,	TYPE_INTERFACE, TYPE_INTERFACE);
		FN_1(kfpCombineRecords,	TYPE_INTERFACE,	FPCombineRecords,	TYPE_INTERFACE_TAB_BR);
		VFN_1(kfpApplyXRefMaterialsToXRefObjects, ApplyXRefMaterialsToXRefObjects, TYPE_REFTARG_TAB_BR); 
		FN_1(kfpIsNodeXRefed,	TYPE_INTERFACE,	FPIsNodeXRefed,	TYPE_INODE);
		VFN_1(kfpApplyXRefControllersToXRefObjects, ApplyXRefControllersToXRefObjects, TYPE_REFTARG_TAB_BR);
		FN_1(kfpResetXRefControllersPRSOffset, TYPE_bool, ResetXRefControllersPRSOffset, TYPE_REFTARG_TAB_BR);
		FN_1(kfpCanResetXRefControllersPRSOffset, TYPE_bool, CanResetXRefControllersPRSOffset, TYPE_REFTARG_BR);
		PROP_FNS(kfpGetMergeTransforms, GetMergeTransforms, 
			kfpSetMergeTransforms, SetMergeTransforms, TYPE_bool);
		PROP_FNS(kfpGetMergeMaterials, GetMergeMaterials, 
			kfpSetMergeMaterials, SetMergeMaterials, TYPE_bool);
		PROP_FNS(kfpGetMergeManipulators, GetMergeManipulators, 
			kfpSetMergeManipulators, SetMergeManipulators, TYPE_bool);
		PROP_FNS(kfpGetMergeModifiers, GetMergeModifiers, 
			kfpSetMergeModifiers, SetMergeModifiers, TYPE_ENUM);
		PROP_FNS(kfpGetIncludeAll, GetIncludeAll, 
			kfpSetIncludeAll, SetIncludeAll, TYPE_bool);
		PROP_FNS(kfpGetAutoUpdate, GetAutoUpdate, 
			kfpSetAutoUpdate, SetAutoUpdate, TYPE_bool);
	END_FUNCTION_MAP

	private:
		bool FPSetRecordSrcFile(FPInterface* xrefRecord, const MCHAR* srcFileName);
		bool FPAddXRefItemsToXRefRecord(
			FPInterface* xrefRecord,
			bool promptObjNames,
			Tab<MCHAR*>* objNames);
		IObjXRefRecord* FPAddXRefItemsFromFile(
			const MCHAR* srcFileName, 
			bool promptObjNames,
			Tab<MCHAR*>* objNames,
			IntTab& xrefOptions);
		bool FPCanCombineRecords(FPInterface* firstXRefRec, FPInterface* secondXRefRec);
		IObjXRefRecord* FPCombineRecords(Tab<FPInterface*>& xrefRecords);
		IXRefItem* FPIsNodeXRefed(INode* node);
		bool FPRemoveRecordFromScene(FPInterface* xrefRecord);
		bool FPMergeRecordIntoScene(FPInterface* xrefRecord);
		virtual bool FPSetXRefItemSrcFile(ReferenceTarget& xrefItem, const MCHAR* fname)=0;
		virtual bool FPSetProxyItemSrcFile(ReferenceTarget& xrefProxy, const MCHAR* fname)=0;
}; 


