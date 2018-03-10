//*****************************************************************************
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise accompanies 
// this software in either electronic or hard copy form.   
//
//*****************************************************************************
//
//	FILE: IContainerObject.h
//  DESCRIPTION: Describes the container object interface 
//	CREATED BY:	Nicolas Leonard
//
//	HISTORY:	July 9th, 2008	Creation
//*****************************************************************************

#pragma once

#include "iFnPub.h"
#include "AssetManagement\iassetmanager.h"
#include "Animatable.h"
// forward declarations
class INodeTab;


///////////////////////////////////////////////////////////////////////////////
//	Container ClassIDs and Interface IDs   
///////////////////////////////////////////////////////////////////////////////

#define CONTAINER_CLASS_ID	Class_ID(0x6E34B493, 0xE3C48E41)

#define IID_CONTAINER Interface_ID(0xB314E0F8, 0x4B104959)
#define IID_CONTAINER13 Interface_ID(0x6f9826aa, 0x75a12aa6)

///////////////////////////////////////////////////////////////////////////////
//!
//! Container Interface
//!
//! \brief IContainer provides access to the container object functionality. 
/*! 
A container is a helper object in the scene.  Once created, the user can choose its content, 
that is, nodes that are meant to be grouped together.  Containers should be self-contained blocks
without references from the content to nodes outside of the container, otherwise the references 
will be lost during operation of the container.  Containers hold the content, and  moving or 
deleting a container will affect the content, but containers and content are not the same thing.  
For example, content can be unloaded, while the container remains in the scene.  The container 
is associated to a container definition file, which is a Max file that stores only the container and 
its contents.  Container definitions are tracked as file assets.

There are two types of containers:

-#	Local Containers:  Containers can be created locally and used to organize scenes in ways that 
are similar to the uses of groups or layers.  This provides a way to collect a bunch of nodes and work 
with them as one, or to section part of your scene to share with others.  If a local container  
has no content that is inherited from any source (including nested containers, that is, containers 
inside containers), we can call it a unique container.  If content is inherited at all, it is  
not a Unique container.

-#	Inherited Containers:  Containers can also be inherited.  When you inherit a container from 
someone else, you get its contents.  If the author allows, you can open the container and add  
changes, while still inheriting the author's changes.  This allows teams to work collaboratively.

Content nodes of containers are all children of the container.  It is possible to have objects linked 
to containers or contents of containers without being inside, but this is not recommended.  It is not   
possible to instance containers, but they can be copied, and then changes to one of the copies can be 
saved, and all containers can be reloaded, and all copies will inherit the changes.  Many containers 
can point to the same definition.

In addition to the content, containers also govern what users of a container are allowed to do 
with the content if they inherit the container.  This is called the container rules.  At the moment, 
the only options are either no access (the user can only read in the content but make no modification)
or edit in-place, which allows someone inheriting the container to modify the content and save the
result, overwriting the original definition.

Containers can be closed, making the content uneditable.  When a container is closed the content behaves 
like an xreffed scene, displayed and rendered but not selectable or editable.

*/
///////////////////////////////////////////////////////////////////////////////

class IContainerObject: public FPMixinInterface
{
public:



	FPInterfaceDesc* GetDesc(); 

	//! Returns the node containing this container object.
	/*! \remarks There can only be one node since we cannot create instances of container objects */
	virtual INode*		GetContainerNode() const = 0;
	//! \name Content queries and modifications
	//@{ 
	//! \brief Returns the content nodes for this container.
	//! \param[in] nestedContainerNodes - if true include nested content
	//! \param[out] contentNodes - returned content nodes
	virtual void 		GetContentNodes(bool nestedContainerNodes, INodeTab& contentNodes) const = 0;
	//! \brief Checks for content membership.
	//! \return - true if the node is part of the content.
	//! \param[in] in_node - node to check for membership
	//! \param[in] nestedContainerNodes - include nested containers child nodes when checking
	virtual	bool		IsNodeInContent(INode* in_node, bool nestedContainerNodes) const = 0;
	//! \brief Checks for inherited content membership.
	//! \return - true if the node is part of the inherited content.
	//! \param[in] nestedContainerNodes - check content of nested containers
	virtual	bool		IsNodeInInheritedContent(INode* in_node, bool nestedContainerNodes) const = 0;
	//! \brief Adds a node and its children to the container.
	/*! The container node becomes the parent of the content node.  Some nodes are automatically added 
		to the local content along with the input node.  The children of the node are added.
		If the node is part of a system [like biped or bones], all nodes in the system are also added.   
		If the node has a target [as in INode::GetTarget() - for instance a camera target], then its  
		target is added and if it is a target, the targeting node is also added */
	//! \see RemoveNodeFromContent
	//! \return - true if the node was added.
	//! \param[in] in_node - node to add to the container
	virtual	bool		AddNodeToContent(INode* in_node) = 0;			
	//! \brief Adds several nodes to the container.
	//! \see AddNodeToContent
	//! \return - true if all nodes were added.
	//! \param[in] in_nodeTab - array of nodes to add to the container
	virtual	bool		AddNodesToContent(INodeTab& in_nodeTab) = 0;
	//! \brief Removes a node from the container.
	/*! Some nodes are automatically removed along with the input node.  The children of the node are removed.
	If the node is part of a system, all nodes in the system are also removed.  If the node has a target, 
	its target is removed and if it is a target, the targeting node is also removed */
	//! \see AddNodeToContent
	//! \return - true if the node was removed from local content.
	//! \param[in] in_node - node to remove from the container
	//! \param[in] detach - whether or not the removed node should also be unlinked from its parent
	virtual	bool		RemoveNodeFromContent(INode* in_node, bool detach = true) = 0;
	//@}

	//! \name Definition files.  
	//! Definition files have the format of a max file, but use a different file extension: .MAXC.  
	//! This is to allow users to distinguish container definitions from regular max scenes when browsing folders, 
	//! and prevent mistaken edits of container definitions.  It is possible to rename the container definition and 
	//! open it as a MAX file.
	//! Local containers use a local definition file.  Inherited containers reference a source definition file.
	//@{ 
	/*! Gets the local definition file for the container.  If the container is inherited, the name is empty */
	virtual MaxSDK::AssetManagement::AssetUser	GetLocalDefinitionFileName() const = 0;
	/*! Sets the local definition file for the container */
	virtual bool		SetLocalDefinitionFileName(const MaxSDK::AssetManagement::AssetUser &val) = 0;

	/*! Gets the source definition file for the container.  If the container is local, the name is empty */
	virtual MaxSDK::AssetManagement::AssetUser	GetSourceDefinitionFileName() const = 0;
	/*! Sets the source definition file for the container */
	virtual bool		SetSourceDefinitionFileName(const MaxSDK::AssetManagement::AssetUser &val) = 0;
	//@}

	//! \name Definition file operations and content management
	//@{ 
	//! \brief Saves the current content and rules as the new definition.
	//! If there is no definition file name, the user will be prompted for one.
	//! \param[in] newFileName - if true, prompt the user for a definition file name
	virtual	bool		SaveContainer(bool newFileName = false) = 0;
	//! Updates the content from its definition, preserving local modifications.
	virtual	bool		UpdateContainer() = 0;
	//! Saves the local definition and deletes the content from the scene.
	virtual	bool		UnloadContainer() = 0;
	//! Reloads content from its definition and local modifications. Only available when unloaded.
	virtual	bool		LoadContainer() = 0;
	//! Reloads content from its definition and local modifications. Same as load but is always available. 
	//! Discards unsaved local changes.
	virtual	bool		ReloadContainer() = 0;
	//! Turns an inherited container into an unsaved local container, merging its content into the scene.
	virtual bool		MergeSource() = 0;
	//! Merges a container source, and recursively merges any nested container among its content
	virtual bool		MakeUnique() = 0;
	//@}

	//! \name Container status and rules.
	//@{ 
	//! Returns whether the container is inherited (or local)
	virtual bool		IsInherited() const = 0;

	//! Returns whether the container is open
	virtual bool		IsOpen() const = 0;
	/*! \brief Opens or closes the container.  Depending on the container state, this may not be allowed */
	//! \return - the open state of the container after the operation.
	virtual bool		SetOpen(bool isOpen) = 0;

	//! Returns whether the open status of the container can be changed in its current state
	virtual bool		IsContainerOpenableOrClosable() const = 0;

	//! Returns whether the container is unloaded, that is, whether its content has been temporarily deleted
	virtual bool		IsUnloaded() const = 0;

	//! Returns whether the container is unique, that is, whether it is local as well as any container it may contain
	virtual bool		IsUnique() const = 0;

	//! \brief Sets whether the container, when inherited, will allow editing in place
	/*! Editing in place is a container rule that allows a user to temporarily turn an inherited container 
	    into a local container and change its definition.  You can now set more rules than just edit in place.
		\see IContainerObject13::SetAccessType*/
	//! \param[in] allowEdit - whether the container, when inherited, will allow editing in place
	virtual void		AllowInPlaceEdit(bool allowEdit) = 0;
	//! Returns whether the container, when inherited, will allow editing in place
	virtual bool		GetAllowInPlaceEdit() const = 0;

	//! Temporarily makes an inherited container that allows it to act like a local container
	virtual void		SetEditInPlace(bool edit) = 0;
	//! Returns whether the container is currently editing in place
	virtual bool		IsInPlaceEditing() const = 0;

	//! For an imported container, returns whether it allows editing in place
	virtual bool		CanEditInPlace() const = 0;

	//! \brief Sets whether the container will automatically update if it is closed and its definition is updated.
	/*! Otherwise a manual Update or Reload is needed. */
	virtual void		SetAutoUpdateClosed(bool setAutoUpdate) = 0;
	//! Returns whether the container will automatically update if it is closed and its definition is updated
	virtual bool		AutoUpdateClosed() const = 0;
	//@}

	//! \name Display properties of the container helper.
	//@{ 
	//! Sets whether the container displays its name in the viewport
	virtual	void		SetLabelDisplay(bool display) = 0;
	//! Returns whether the container displays its name in the viewport
	virtual	bool		GetLabelDisplay() const = 0;

	//! Sets whether the bounding box for the container object will be extended to encompass all of its content
	virtual void		UseContentBoundingBox(bool useContent) = 0;
	//! Returns whether the bounding box for the container object will be extended to encompass all of its content
	virtual bool		IsUsingContentBoundingBox() const = 0;

	//! Sets whether the container overrides the object properties (wire color, hidden status etc) of its content
	virtual void		SetOverrideNodeProperties(bool setOverride) = 0;
	//! Returns whether the container overrides the object properties (wire color, hidden status etc) of its content
	virtual bool		OverrideNodeProperties() const = 0;

	//! Sets the size of the container helper object
	virtual void		SetSize(float size, TimeValue t = 0) = 0;
	//! Returns the size of the container helper
	virtual float		GetSize(TimeValue t = 0) const = 0;
	//@}


	//! If the Animatable supports the IContainerObject interface, returns a pointer to it.
	//! \param[in] anim - Animatable to be tested for support of the IContainer interface
	//! \return - a pointer to an IContainer interface, or NULL.
	static inline IContainerObject* GetInterface(Animatable* anim) { 
		return anim? static_cast<IContainerObject*>(anim->GetInterface(IID_CONTAINER)): NULL;}
};


///////////////////////////////////////////////////////////////////////////////
//!
//! IContainer13 Interface
//!
//! \brief IContainer13 extends container object functionality.
//! In particular it provides support for new access rules that inherited containers may follow.
/*! 
Previously, containers would only allow for no access when inherited or access to edit in place so that
a user can save over the original definition.  Now in addition a container can grant access to only add new objects
over the inherited container or get full freedom to access anything that was unlocked on the original source definition.
For the latter case the user can specify additional locks over certain items, such as all materials, all modifiers, all
transforms or all objects, to restrict what's accessible further.
*/
class IContainerObject13  : public IContainerObject
{
public:

	//! \brief Access possibilities for when a container is inherited.  
	enum 
		AccessType{ 
			//! \brief Nothing is accessible,acts like a scene xref
			eNoAccess = 0x0, 
			//! \brief The only access allowed is the ability to edit in place
			eOnlyEditInPlace , 
			//! \brief All inherited content is locked but you can add new nodes 
			eOnlyAddNewObjects,  
			//! \brief Access to all of the unlocked tracks on the inherited container
			eAnythingUnlocked};
	
	//! \brief Possibilities for items that may get locked when the AccessType is eAnythingUnlocked.
	enum LockedContents {
		//! \brief All materials are locked
		eLockAllMaterials =0x1, 
		//! \brief All modifiers are locked
		eLockAllModifiers = 0x2 , 
		//! \brief All transforms are locked
		eLockAllTransforms =0x4, 
		//! \brief All objects are locked
		eLockAllObjects= 0x8};

	enum ProxyType {
		//! \brief No proxy is used (the container uses its definition)
		PROXYTYPE_NONE,
		//! \brief An alternate definitions is used
		PROXYTYPE_ALTERNATE
	};

	//! \name Access Type
	//@{ 
	//! \brief Set the type of access for this container when it gets inherited by another container.
	//! This replaces IContainerObject::SetEditInPlace since different levels of access are now supported.
	//! \param[in] accessType - \see IContainerObject13::AccessType to see what levels of access you can specify. 
	virtual void SetAccessType(AccessType accessType) =0;

	//! Returns the type of access that the container will provide when inherited.
	//! \return Returns the enum AccessType flag specifying what type of access is allowed. \see IContainerObject13::AccessType
	virtual AccessType GetAccessType() const = 0;

	//! Returns the type of access that the container has based upon the access provided by its inherited source.  
	//! If it has no inherited source it will default to return the same value as IContainerObject13::GetAccessType.
	//! \return Returns the enum AccessType flag specifying what type of access is allowed. \see IContainerObject13::AccessType
	virtual AccessType InheritedAccessType() const = 0;
	//@}

	//! \name Locked Contents
	//@{ 
	//! \brief Set what will be automatically locked when the access type is set to AccessType::eAnythingUnlocked.
	//! param[in] content - Type of content that will be automatically locked. \see IContainerObject13::LockedContents.
	//! \param[in] clear - If true then clear any other locked content enum that was set, and only set the specified one. 
	//! If false, then keep the other specified locked content types also.
	virtual void SetLockedContents(LockedContents content, bool clear) =0;

	//! \brief Clear the locked content rules so that nothing is automatically locked.
	virtual void ClearLockedContents() =0;

	//! \brief Get if this content will be automatically locked when the access type is set to AccessType::eAnythingUnlocked.
	//! param[in] - content The content type that we are checking to see if it will be locked. \see IContainerObject13::LockedContents.
	//! \return Returns true if that content type specified by the LockedContents enum will be locked, false otherwise.
	virtual bool IsLockedContents(LockedContents content)const =0;
	//@}

	//! \brief Saves the content and rules as the new definition into a format compatible with the specified 3ds Max version
	//! If there is no definition file name, the user will be prompted for one.
	//! \param[in] newFileName - if true, prompt the user for a definition file name
	//! \param [in] saveAsVersion - the version of 3ds Max in which the file is to be saved. 
	//! Currently it supports saving to the current version of 3ds Max (MAX_RELEASE) and to 3ds Max 2010 (MAX_RELEASE_R12).
	//! \return true if the operation was successful; false otherwise
	virtual	bool SaveContainerAsVersion(bool newFileName, unsigned long saveAsVersion) = 0;

	//! \brief Returns a status string for the container
	virtual MCHAR*		GetStatusString() = 0;

	//! \brief Sets whether the container displays its status in the 3D viewports
	virtual	void		SetStatusDisplay(bool status) = 0;
	//! \brief Returns whether the container displays its status in the 3D viewports
	virtual	bool		GetStatusDisplay() const = 0;

	//! \brief Returns the user name currently editing the definition for the container
	virtual const MCHAR* GetEditingUser() const = 0;

	//! \brief Returns whether the container is out of date with its definition
	virtual bool		IsUpdateNeeded() const = 0;

	////////////////////////////////////////////////////////////////////////////
	//! \name Container proxies
	//! Proxies offer the possibility to represent container content by something 
	//! other than their definition.  The following are the proxy types currently 
	//! supported: 
	//!		PROXYTYPE_NONE: (default value) the container displays its definition
	//!		PROXYTYPE_ALTERNATE: the container will display one of possibly several
	//!							alternate definitions, one of which is current.
	////////////////////////////////////////////////////////////////////////////
	//@{ 
	//! \brief Returns the proxy type used by the container.  \see IContainerObject13::ProxyType
	virtual ProxyType	GetProxyType() const = 0;
	//! \brief Sets the proxy type used by the container.  \see IContainerObject13::ProxyType
	virtual void		SetProxyType(ProxyType type) = 0;

	//! \brief Returns the number of alternate definitions for the container.
	virtual int			GetAlternateDefinitionCount() const = 0;
	//! \brief Returns the alternate definition for the container from an index.
	//! \param[in] defIndex - zero-based index of the alternate definition file to retrieve
	virtual MaxSDK::AssetManagement::AssetUser	GetAlternateDefinition(int defIndex) const = 0;
	//! \brief Sets an alternate definition for the container.
	//! \param[in] defIndex - zero-based index of the alternate definition file to set.
	//!		The index must be less than the current number of alternate definitions
	//! \param[in] val - an alternate definition file: either an empty asset or an asset of type kContainerAsset 
	//! \return - true if the alternate definition was successfully set.
	virtual bool		SetAlternateDefinition(int defIndex, const MaxSDK::AssetManagement::AssetUser &val) = 0;
	//! \brief Appends one alternate definition to the container alternate definitions.
	// Appending an alternate definition does not set the current alternate to the added asset
	//! \param[in] val - an alternate definition file: either an empty asset or an asset of type kContainerAsset 
	//! \return - true if the alternate definition was successfully appended.
	virtual bool		AppendAlternateDefinition(const MaxSDK::AssetManagement::AssetUser &val) = 0;
	//! \brief Removes one alternate definition from the container alternate definitions.
	//! \param[in] defIndex - zero-based index of the alternate definition file to remove.
	//! \return - true if the alternate definition was successfully removed.
	virtual bool		RemoveAlternateDefinition(int defIndex) = 0;

	//! \brief Returns the zero-based index of the current alternate definition of the container
	virtual int			GetCurrentAlternateDefinitionIndex() const = 0;
	//! \brief Sets the current alternate definition of the container
	//! \param[in] defIndex - zero-based index of the alternate definition to set as current.
	//!		The index must be less than the current number of alternate definitions
	virtual void		SetCurrentAlternateDefinitionIndex(int defIndex) = 0;
	//! \brief Returns the current alternate definition of the container
	virtual MaxSDK::AssetManagement::AssetUser	GetCurrentAlternateDefinition() const = 0;
	//@}

	//! \brief Returns whether the container is both inherited and its content is not currently local
	virtual bool		IsInheritedClosed() const = 0;

	//! If the Animatable supports the IContainerObject13 interface, returns a pointer to it.
	//! \param[in] anim - Animatable to be tested for support of the IContainer interface.
	//! \return - a pointer to an IContainerObject13 interface, or NULL.
	static inline IContainerObject13* GetInterface(Animatable* anim) { 
		return anim? static_cast<IContainerObject13*>(anim->GetInterface(IID_CONTAINER13)): NULL;}
};

