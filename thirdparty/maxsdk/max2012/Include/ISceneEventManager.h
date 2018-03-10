//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// FILE:        SceneEventListener.h
// DESCRIPTION: Scene Event Notification Manager
// AUTHOR:      Michaelson Britt
// HISTORY:     Created 5 February 2007
//**************************************************************************/

#pragma once


#include "MaxTypes.h"
#include "Tab.h"
#include "BaseInterface.h"
#include "Export.h"
#include "CoreExp.h"
#include "plugapi.h"
#include "Animatable.h"

// forward declarations
class INode;

#define ISCENEEVENTCALLBACK_INTERFACE_ID	Interface_ID(0x36cd4b33, 0x36ef01e4)

class INodeEventCallback;
#define INODEEVENTCALLBACK_INTERFACE_ID		Interface_ID(0x6a173454, 0x4b420bbc)

class ISceneEventManager;
#define ISCENEEVENTMANAGER_INTERFACE_ID		Interface_ID(0x3f375083, 0x63ea3a86)




//===========================================================================
//
// Namespace SceneEventNamespace
//
//===========================================================================

namespace SceneEventNamespace {
	typedef DWORD CallbackKey;
};


//===========================================================================
//
// Namespace NodeEventNamespace
//
//===========================================================================

namespace NodeEventNamespace {
	typedef AnimHandle NodeKey;
	typedef Tab<NodeKey> NodeKeyTab;
	typedef SceneEventNamespace::CallbackKey CallbackKey;

	// ---------- Helper Functions ----------
	//! \brief Returns the NodeKey for a node object.
	//! Returns a node's key value, which is the AnimHandle for the node object.
	//! \param[in] node The node object.
	//! \return The node's key value.
	inline NodeKey GetKeyByNode( INode* node ) {
		if( node!=NULL )
			 return Animatable::GetHandleByAnim((Animatable*)node);
		else return 0;
	}
	//! \brief Returns a node object from a NodeKey.
	//! Returns a node object from its key value, which is the AnimHandle for the node object.
	//! \param[in] nodeKey The node's key value, which is the AnimHandle for the node object.
	//! \return The node object.
	inline INode* GetNodeByKey( NodeKey nodeKey ) {
		Animatable* anim = Animatable::GetAnimByHandle(nodeKey);
		if( (anim!=NULL) && (anim->SuperClassID()==BASENODE_CLASS_ID) )
			 return (INode*)anim;
		else return NULL;
	}
};


//===========================================================================
//
// Class SceneEventCallback
// Base class class for all EventCallback types
//
//===========================================================================
class ISceneEventCallback : public BaseInterface {
	public:
		//! \brief Casts to an INodeEventCallback if possible
		//! If this object is an INodeEventCallback, this method downcasts and returns a pointer.  Otherwise this returns NULL.
		//! \return INodeEventCallback pointer if the object is an INodeEventCallback; otherwise NULL.
		INodeEventCallback* GetINodeEventCallback()					{ return (INodeEventCallback*)(this->GetInterface(INODEEVENTCALLBACK_INTERFACE_ID)); }
};


//===========================================================================
//
// Class NodeEventCallback
//
//===========================================================================

class INodeEventCallback : public ISceneEventCallback {
	public:
		typedef NodeEventNamespace::NodeKeyTab NodeKeyTab;
		#pragma warning(push)
		#pragma warning(disable:4100)
		// ---------- Hierarchy Events ----------
		//! \defgroup HierarchyEventMethods Hierarchy Events
		//! Changes to the scene structure of an object
		//@{
		//! \brief Nodes added to the scene
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! \see NodeEventNamespace::GetNodeByKey()
		virtual void Added( NodeKeyTab& nodes )						{}
		//! \brief Nodes deleted from the scene
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! Node that, if already flushed from the undo buffer, nodes pointers will not be available via GetNodeByKey().
		//! \see NodeEventNamespace::GetNodeByKey()
		virtual void Deleted( NodeKeyTab& nodes )					{}
		//! \brief Node linked or unlinked from another parent node.
		//! Only the linked or unlinked child nodes are listed in the notification, not the parent nodes.
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! \see NodeEventNamespace::GetNodeByKey()
		virtual void LinkChanged( NodeKeyTab& nodes )				{}
		//! \brief Nodes added or removed from a layer, or moved between layers
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! \see NodeEventNamespace::GetNodeByKey()
		virtual void LayerChanged( NodeKeyTab& nodes )				{}
		//! \brief Nodes added or removed from a group, or its group was opened or closed
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! \see NodeEventNamespace::GetNodeByKey()
		virtual void GroupChanged( NodeKeyTab& nodes )				{}
		//! \brief All other change to the scene structure of nodes
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! \see NodeEventNamespace::GetNodeByKey()
		virtual void HierarchyOtherEvent( NodeKeyTab& nodes )		{}
		//@}

		// ---------- Model Events ----------
		//! \defgroup ModelEventMethods Model Events
		//! Changes to the geometry or parameters of an object
		//@{
		//! \brief Nodes with modifiers added or deleted, or modifier stack branched
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! \see NodeEventNamespace::GetNodeByKey()
		virtual void ModelStructured( NodeKeyTab& nodes )			{}
		//! \brief Nodes changed in their geometry channel
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! \see NodeEventNamespace::GetNodeByKey()
		virtual void GeometryChanged( NodeKeyTab& nodes )			{}
		//! \brief Nodes changed in their topology channel
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! \see NodeEventNamespace::GetNodeByKey()
		virtual void TopologyChanged( NodeKeyTab& nodes )			{}
		//! \brief Nodes changed in their UV mapping channel, or vertex color channel.
		//! Also includes Material ID changes.
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! \see NodeEventNamespace::GetNodeByKey()
		virtual void MappingChanged( NodeKeyTab& nodes )			{}
		//! \brief Nodes changed in any of their extention channels
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! \see NodeEventNamespace::GetNodeByKey()
		virtual void ExtentionChannelChanged( NodeKeyTab& nodes )	{}
		//! \brief All other change to the geometry or parameters of an object.
		//! This includes settings on lights, cameras and helper objects.
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! \see NodeEventNamespace::GetNodeByKey()
		virtual void ModelOtherEvent( NodeKeyTab& nodes )			{}
		//@}

		// ---------- Material Events ----------
		//! \defgroup MaterialEventMethods Material Events
		//! Changes to the material on an object
		//@{
		//! \brief Node materials applied, unapplied or switched, or sub-texture structure of materials changed
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! \see NodeEventNamespace::GetNodeByKey()
		virtual void MaterialStructured( NodeKeyTab& nodes )		{}
		//! \brief All other change to the settings of a node's material.
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! \see NodeEventNamespace::GetNodeByKey()
		virtual void MaterialOtherEvent( NodeKeyTab& nodes )		{}
		//@}

		// ---------- Controller Events ----------
		//! \defgroup ControllerEventMethods Controller Events
		//! Changes to the controller on an object
		//@{
		//! \brief Node transform controllers applied, unapplied or switched.
		//! Pertains only to a node's transform controller, not to other animated aspects of the object.
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! \see NodeEventNamespace::GetNodeByKey()
		virtual void ControllerStructured( NodeKeyTab& nodes )		{}
		//! \brief All other changes to node transform controller values, including nodes moved/rotated/scaled, or transform animation keys set.
		//! Pertains only to a node's transform controller, not to other animated aspects of the object.
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! \see NodeEventNamespace::GetNodeByKey()
		virtual void ControllerOtherEvent( NodeKeyTab& nodes )		{}
		//@}

		// ---------- Property Events ----------
		//! \defgroup PropertyEventMethods Property Events
		//! Changes to object properties
		//@{
		//! \brief Node names changed.
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! \see NodeEventNamespace::GetNodeByKey()
		virtual void NameChanged( NodeKeyTab& nodes )				{}
		//! \brief Node wire color changed.
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! \see NodeEventNamespace::GetNodeByKey()
		virtual void WireColorChanged( NodeKeyTab& nodes )			{}
		//! \brief Node render-related object properties changed.
		//! From the Object Properties dialog, this includes Rendering Control and Motion Blur items, and the Advanced Lighting and mental ray tabs.
		//! Also includes changes to displacement approximation and render elements.
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! \see NodeEventNamespace::GetNodeByKey()
		virtual void RenderPropertiesChanged( NodeKeyTab& nodes )	{} // Includes PART_GI_xxx, PART_REND_xxx, PART_DISP_APPROX, and render elements
		//! \brief Node display-related object properties changed.
		//! From the Object Properties dialog, this includes Interactivity and Display Properties items.
		//! Also includes changes to viewport shader data, as per PART_GFX_DATA.
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! \see NodeEventNamespace::GetNodeByKey()
		//! \see PART_GFX_DATA
		virtual void DisplayPropertiesChanged( NodeKeyTab& nodes )	{} // Includes PART_DISP_xxx, PART_GFX_DATA
		//! \brief Node used-defined object properties changed.
		//! From the Object Properties dialog, this is the User Defined tab.
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! \see NodeEventNamespace::GetNodeByKey()
		virtual void UserPropertiesChanged( NodeKeyTab& nodes )		{}
		//! \brief All other changes to node property values.
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! \see NodeEventNamespace::GetNodeByKey()
		virtual void PropertiesOtherEvent( NodeKeyTab& nodes )		{} // All other property events
		//@}

		// ---------- Display/Interaction Events ----------
		//! \defgroup DisplayEventMethods Display Events
		//! Changes to the display or interactivity state of an object
		//@{
		//! \brief Subobject selection changed.
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! \see NodeEventNamespace::GetNodeByKey()
		virtual void SubobjectSelectionChanged( NodeKeyTab& nodes )	{}
		//! \brief Nodes selected or deselected.
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! \see NodeEventNamespace::GetNodeByKey()
		virtual void SelectionChanged( NodeKeyTab& nodes )			{}
		//! \brief Nodes hidden or unhidden.
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! \see NodeEventNamespace::GetNodeByKey()
		virtual void HideChanged( NodeKeyTab& nodes )				{}
		//! \brief Nodes frozen or unfrozen.
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! \see NodeEventNamespace::GetNodeByKey()
		virtual void FreezeChanged( NodeKeyTab& nodes )				{}
		//! \brief All other display or interaction node events.
		//! For example, turning on trajectory mode or link display mode.
		//! \param[in] nodes The node list, provided as AnimHandles.
		//! \see NodeEventNamespace::GetNodeByKey()
		virtual void DisplayOtherEvent( NodeKeyTab& nodes )			{}
		//@}
		#pragma warning(pop)
		// ---------- Other callback methods ----------
		//! \brief Called when messages are being triggered for the callback, before event methods
		virtual void CallbackBegin()								{}
		//! \brief Called when messages are being triggered for the callback, after all event methods
		virtual void CallbackEnd()									{}

		// ---------- Callback Settings ----------
		//! \brief Verbose Deleted mode, for extra information about deleted object
		//! If verbose is false, messages are never sent for objects which are deleted from the scene (and on the undo stack).
		//! Also the Add or Delete messages are treated as all-encompassing messages, and will never be accompanied by other messages.
		//! If verbose is true, all messages are sent, even for deleted objects, and even accompanying Add and Delete messages.
		//! Do not change this value dynamically. The method is called only once by the system, when the callback is registered.
		//! \return The Verbose Deleted setting to use for the callback
		virtual BOOL VerboseDeleted()								{return FALSE;}

		// ---------- Support for Future Expansion ----------
		Interface_ID GetID()										{ return INODEEVENTCALLBACK_INTERFACE_ID; }
		BaseInterface* GetInterface(Interface_ID id)
			{ if (id == INODEEVENTCALLBACK_INTERFACE_ID) return this; else return BaseInterface::GetInterface(id); }
};


//===========================================================================
//
// Class SceneEventManager
//
//===========================================================================
class ISceneEventManager : public BaseInterface {
	public:
		typedef SceneEventNamespace::CallbackKey CallbackKey;

		//! \brief Registers a scene event callback, including NodeEventCallback objects.
		//! \param[in] callback The callback object.  This may be any scene event callback type, including NodeEventCallback.
		//! \param[in] polling In polling mode, messages are sent only when TriggerMessages() is called.
		//! Otherwise, messages are sent immediately after the given delay and mouseUp conditions are met.
		//! Polling mode cannot be used if a delay or mouseUp is specified.
		//! \param[in] delayMilliseconds If delay is zero, messages are triggered immediately when an event is completed.
		//! If delay is nonzero, messages are delayed until the specified time passes with no events (an uninterrupted period with no event noise).
		//! \param[in] mouseUp If mouseUp is true, messages are delayed until the mouse is up.
		//! \return The callback key ID (zero on failure)
		virtual CallbackKey RegisterCallback( ISceneEventCallback* callback, BOOL polling=FALSE, DWORD delayMilliseconds=0, BOOL mouseUp=FALSE ) = 0;
		//! \brief Unregister a callback object.
		//! \param[in] callbackKey The callback key ID
		virtual void UnRegisterCallback( CallbackKey callbackKey ) = 0;
		//! \brief Returns a callback object, given its key ID.
		//! \param[in] callbackKey The callback key ID
		//! \return The callback object
		virtual ISceneEventCallback* GetCallbackByKey( CallbackKey callbackKey ) = 0;
		//! \brief Returns a callback key ID, given a registered callback object.
		//! \param[in] callback The callback object.
		//! \return The callback key ID (zero on failure)
		virtual CallbackKey GetKeyByCallback( ISceneEventCallback* callback ) = 0;

		//! \brief Triggers messages about all events gathered for a callback, immediately
		//! This may be used with callbacks in any mode, but is particularly necessary
		//! to trigger messages for callbacks in polling mode.
		//! \param[in] callbackKey The callback key ID
		virtual void TriggerMessages( CallbackKey callbackKey ) = 0;
		//! \brief Clears out all gathered messages for a callback
		//! The currently gathered events will not be included in the next batch of messages for the callback.
		//! \param[in] callbackKey The callback key ID
		virtual void ResetMessages( CallbackKey callbackKey ) = 0;
};

CoreExport ISceneEventManager* GetISceneEventManager();


