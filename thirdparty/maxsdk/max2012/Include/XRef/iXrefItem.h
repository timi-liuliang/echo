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
// DESCRIPTION: Interface to any xref scene entity
// AUTHOR: Attila Szabo - created Feb.09.2005
//***************************************************************************/

#pragma once

#include "..\iFnPub.h"
#include "..\tab.h"
#include "..\assetmanagement\AssetUser.h"

// Foreward declarations
class IObjXRefRecord;
class ReferenceMaker;
class ReferenceTarget;
class INodeTab;

//! Id for IXRefItem interface
#define IID_XREF_ITEM Interface_ID(0x5fa551f4, 0x442f057c)

//! \brief Generic interface that represents an xrefed scene entity
/*! This interface exposes services common to all xref scene entities.
3rd party plugins do not need to derive from this interface. Support for xrefing 
different plugin categories is provided by the system (3ds Max). Each plugin 
category that supports xrefing is modeled as a class that implements this interface. 

Xref items can be created and modified via the Object XRef Manager (IObjXRefManager8), 
and live in xref records (IObjXRefRecord). An xref item can live in one and only one xref record.

Sub-object animation of a scene entity is maintained when the scene entity is xrefed.
Transform animation of a scene entity such as a node is brought into the master file,
but is not updated when the xref record is updated.

In order to retrieve this interface from a scene entity, one must call either 
IXRefItem::GetInterface with a reference to a scene entity (Object, Atmospherics, etc.), 
\see IObjXRefManager8, IObjXRefRecord
*/
class IXRefItem : public FPMixinInterface
{
public:
	//! \brief List of xref item types or categories of xref-able scene entities. 
	/*! Use the following values when retrieving xref items from xref records. 
	*/
	enum XRefItemType
	{
		//! \brief Xref object type
		kType_Object			= 1<<0, 
		//! \brief Xref material type
		kType_Material		= 1<<1, 
		// Xref controller type - Unsupported
		kType_Controller	= 1<<2, 
		//! \brief Xref atmospheric type
		kType_Atmospheric	= 1<<3, 
		//! \brief All xref item types
		kType_All					= (kType_Object | 
												kType_Material | 
												kType_Controller | 
												kType_Atmospheric),
		//! \brief All xref item types, except atmospherics
		kType_All_No_Atmospherics	= (kType_Object | 
												kType_Material | 
												kType_Controller),
	};

	/*! \name XRef Item Property Access 
	The following methods give access to the properties of xref items. These properties
	are allowed to be modified only via methods of IObjXRefManager8 in order to ensure
	the integrity of xref items and records where they are stored.
	*/
	//@{
	//! \brief Retrieves the file asset that the source scene entity comes from.
	/*! \return - The file asset that the source scene entity comes from.
	\see IObjXRefManager8::SetSrcFile
	*/
	virtual const MaxSDK::AssetManagement::AssetUser& GetSrcFile() const = 0;

	//! \brief Retrieves the name of the source scene entity. 
	/*! \return - Pointer to the name of the source scene entity or NULL if 
	there's no source item name specified.
	\see IObjXRefManager8::SetXRefItemSrcName
	*/
	virtual const MCHAR* GetSrcItemName() const = 0;
	
	//! \brief Retrieves the source scene entity xrefed by this xref item.
	/*! \param [in] resolveNested - If true, the method looks for the first non-xref 
	source item. This could be a direct or indirect source item.
	\return - Pointer to the source scene entity xrefed by this xref item.
	The source entity can be itself an xref item or a derived object if the supplied 
	parameter is false. If the source scene entity is NULL, the xref item is unresolved.
	\see 
	*/
	virtual ReferenceTarget* GetSrcItem(bool resolveNested = false) const = 0;

	//! \brief Allows to verify whether the xref item is unresolved. 
	/*! An xref item is unresolved if the source scene entity is invalid. 
	This can happen for example if the source scene entity does not exist in the 
	source file or the xref item has been pointed to a source file
	that does not exist. Unresolved xref items will not render, but have a default 
	viewport representation. Unresolved xref items may get re-resolved on a subsequent 
	reload of the xref item's record.
	\return - true if this is an unresolved xref item
	\see IObjXRefRecord::Update
	*/
	virtual bool IsUnresolved() const = 0;

	//! \brief Retrieves the xref record this xref item belongs to.
	/*! \return The xref record this xref item belongs to. An xref item can belong to 
	one and only one xref record at any one time. A return value of NULL, indicates 
	that the xref item is not part of an xref record yet.
	*/
	virtual IObjXRefRecord* GetXRefRecord() const = 0;

	//! \brief Retrieves the nodes associated with this xref item.
	/*! This method retrieves all the nodes that reference this xref item. If this
	xref item is an xref atmospheric, no nodes will be reported. If this xref item 
	is nested, this method retrieves the nodes that reference it indirectly.
	Note that nodes that are not in the scene could also be returned by this method.
	\param [in, out] xrefNodes - The tab of nodes that reference this xref item. 
	The tab is not cleared by this method. The tab is guaranteed to not contain duplicated nodes.
	*/
	virtual void GetNodes(INodeTab& xrefNodes) const = 0;
	//@}

	/*! \name Nested XRef Item Management
	When xrefing scene entities that are themselves xrefs, nested xref items are created. 
	The following methods allow working with nested xref items.
	*/
	//@{
	//! \brief Retrieves the xref items under which this xref item is nested.
	/*! A nested xref item can have multiple parent items	(when for example a nested 
	xref item is cloned), and all of them have the same type as their child item.
	\param [out] parentItems - Array where the parent xref items of this item are returned. 
	The parent xref items are added to the array without clearing it out first. 
	\return - The number of parent xref items returned. 
	*/
	virtual unsigned int GetParentItems(Tab<ReferenceTarget*>& parentItems) const = 0;
	//! \brief Returns the list of every xref item for which GetParentItem() would return 'this'.	
	/*! A child xref item is a xref item encountered when walking down any given path in the
	reference hierarchy of this xref item. An xref item may have several child xref items. 
	\param [out] childItems - Array where the child xref items of this item are returned. 
	The child xref items are added to the array without clearing it out first. 
	\return - The number of child xref items returned. 
	*/
	virtual unsigned int GetChildItems(Tab<ReferenceTarget*>& childItems) const = 0;
	//! \brief Retrieves the root (top-level) xref items of this xref item.
	/*! This method retrieves all top-level parent xref items of this item.
	A top-level xref item is an item that does not have an ancestor. The top-level xref 
	item of an item that doesn't have ancestors is itself.
	\param rootItems - Array where the top-level (root) xref items of this item are returned. 
	This array is guaranteed to contain at least this xref item.
	\return - The number of top-level (root) xref items returned. 
	*/
	virtual unsigned int GetRootItems(Tab<ReferenceTarget*>& rootItems) const = 0;
	//! \brief Checks whether this xref item is a nested one.
	/*! This method offers a more efficient way of finding out if this xref item is
	nested than calling GetParentItems.
	\return - true if this xref item is a nested one.
	*/
	virtual bool IsNested() const = 0;
	//@}


	//! \brief Allows to determine whether a scene entity implements this interface 
	/*! \param [in] is - Reference to scene entity
	\return - true if the scene entity implements the IXRefItem interface, false otherwise
	 */
	static bool IsIXRefItem(InterfaceServer& is) {
		return (is.GetInterface(IID_XREF_ITEM) != NULL);
	};

	using FPMixinInterface::GetInterface;
	//! \brief Retrieves the IXRefItem interface from a scene entity 
	/*! \param [in] is - Reference to scene entity
	\return - Pointer to the IXRefItem interface of the scene entity or NULL if it's not an xref item
	*/
	static IXRefItem* GetInterface(InterfaceServer& is) {
		return static_cast<IXRefItem*>(is.GetInterface(IID_XREF_ITEM));
	};

	// --- Function publishing
	virtual Interface_ID GetID() { return IID_XREF_ITEM; }

	enum {
		kfpGetSrcFileName, kfpSetSrcFileName,
		kfpGetSrcItemName, kfpSetSrcItemName,
		kfpGetSrcItem,
		kfpGetParentItems,
		kfpGetChildItems,
		kfpGetRootItems,
		kfpGetXRefRecord,
		kfpIsUnresolved,
		kfpIsNested,
		kfpGetNodes,
		kfpLastID,
	};

	BEGIN_FUNCTION_MAP
		PROP_FNS(kfpGetSrcFileName, FPGetSrcFileName, kfpSetSrcFileName, FPSetSrcFileName, TYPE_FILENAME);
		PROP_FNS(kfpGetSrcItemName, GetSrcItemName, kfpSetSrcItemName, FPSetSrcItemName, TYPE_STRING);
		RO_PROP_FN(kfpGetXRefRecord, GetXRefRecord, TYPE_INTERFACE);
		RO_PROP_FN(kfpIsUnresolved, IsUnresolved, TYPE_bool);
		RO_PROP_FN(kfpIsNested, IsNested, TYPE_bool);
		FN_1(kfpGetSrcItem, TYPE_REFTARG, GetSrcItem, TYPE_bool);
		FN_1(kfpGetParentItems, TYPE_DWORD, GetParentItems, TYPE_REFTARG_TAB_BR);
		FN_1(kfpGetRootItems, TYPE_DWORD, GetRootItems, TYPE_REFTARG_TAB_BR);
		FN_1(kfpGetChildItems, TYPE_DWORD, GetChildItems, TYPE_REFTARG_TAB_BR);
		VFN_1(kfpGetNodes, FPGetNodes, TYPE_INODE_TAB_BR);
	END_FUNCTION_MAP

	// From FPMixinInterface
	virtual FPInterfaceDesc* GetDesc() { return &mFPInterfaceDesc; }
	static FPInterfaceDesc mFPInterfaceDesc;

	protected:
		//! \name Function Publishing methods 
		//@{
		//! \brief Sets file name of xref. Implemented by the system.
		virtual void FPSetSrcFileName(const MCHAR* fileName) = 0;
		//! \brief Gets file name of xref. Implemented by the system.
		virtual const MCHAR* FPGetSrcFileName() = 0;
		//! \brief Sets item name of xref. Implemented by the system.
		virtual void FPSetSrcItemName(const MCHAR* itemName) = 0;
		//@}

	private:
		void FPGetNodes(Tab<INode*>& xrefNodes);
};


