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
// FILE:        ref.h
// DESCRIPTION: Defines reference system classes
// AUTHOR:      Rolf Berteig & Dan Silva
// HISTORY:     created 9 September 1994
//**************************************************************************/

#pragma once
#include "BuildWarnings.h"
#include "maxheap.h"
#include "strbasic.h"
#include "noncopyable.h"
#include "autoptr.h"
#include <WTypes.h> // for ULONG_PTR
#include "CoreExport.h"
#include "Animatable.h"
#include "AnimatableFlags.h"
#include "hold.h" // for RestoreObj
#include "channels.h" // for ALL_CHANNELS

/* This module implements a first run at the concept of references.
Some other stuff is in here too like time and intervals, but
these are implemented only to get the reference classes working. */

typedef void* ReferenceData;
typedef unsigned int TargetParam;

//==============================================================================
//	Part ID's
//==============================================================================

/** 
	\defgroup partids PartID 
	This section describes the PartID parameter used to pass information that supports messages 
	received by Reference Makers.
 */
 
//@{
	
/** This is NOT just a bitmask, the parameter is also used to stuff pointers, hence the ULONG_PTR. */
typedef ULONG_PTR PartID;

/** \defgroup partid_object_channel Object Channel PartIDs
OBJECT STATE: SUB-PARTS These are the 4 sub-channels in the object state. Don't change these defines.
\sa channels.h for define of TOPO_CHANNEL, etc. */
//@{
//! \brief See \ref TOPO_CHANNEL
#define PART_TOPO			TOPO_CHANNEL
//! \brief See \ref GEOM_CHANNEL  
#define PART_GEOM			GEOM_CHANNEL 
//! \brief See \ref TEXMAP_CHANNEL
#define PART_TEXMAP			TEXMAP_CHANNEL
//! \brief See \ref MTL_CHANNEL
#define PART_MTL			MTL_CHANNEL  
//! \brief See \ref SELECT_CHANNEL
#define PART_SELECT			SELECT_CHANNEL
//! \brief See \ref SUBSEL_TYPE_CHANNEL
#define PART_SUBSEL_TYPE 	SUBSEL_TYPE_CHANNEL
//! \brief See \ref DISP_ATTRIB_CHANNEL
#define PART_DISPLAY    	DISP_ATTRIB_CHANNEL
//! \brief See \ref VERTCOLOR_CHANNEL
#define PART_VERTCOLOR		VERTCOLOR_CHANNEL
//! \brief See \ref GFX_DATA_CHANNEL
#define PART_GFX_DATA		GFX_DATA_CHANNEL
//! \brief See \ref DISP_APPROX_CHANNEL
#define PART_DISP_APPROX	DISP_APPROX_CHANNEL
//! \brief See \ref EXTENSION_CHANNEL
#define PART_EXTENSION		EXTENSION_CHANNEL
//! \todo describe this define
#define PART_TM_CHAN   		TM_CHANNEL
//! \todo describe this define
#define PART_MTL_CHAN		GLOBMTL_CHANNEL
//! \todo describe this define
#define PART_OBJECT_TYPE   	(1<<11)      
//! \brief node transform matrix
#define PART_TM  			(1<<12)
//! \todo describe this define
#define PART_OBJ  			(PART_TOPO|PART_GEOM)
//! \todo describe this define
#define PART_ALL			(ALL_CHANNELS|PART_TM)
//@}

/*! \defgroup partid_node_global_illumination_property Node Global Illumination Property PartIDs
	Part IDs sent with the \ref REFMSG_NODE_GI_PROP_CHANGED message. 
	\todo Document individual defines */
//@{
#define PART_GI_EXCLUDED				(1<<0)
#define PART_GI_OCCLUDER				(1<<1)
#define PART_GI_RECEIVER				(1<<2)
#define PART_GI_DIFFUSE				(1<<3)
#define PART_GI_SPECULAR				(1<<4)
#define PART_GI_NBREFINESTEPS			(1<<5)
#define PART_GI_MESHSIZE                (1<<6)
#define PART_GI_MESHINGENABLED          (1<<7)
#define PART_GI_USEGLOBALMESHING        (1<<8)
#define PART_GI_EXCLUDEFROMREGATHERING	(1<<9)
#define PART_GI_STOREILLUMMESH			(1<<10)
#define PART_GI_RAYMULT					(1<<11)
#define PART_GI_USE_ADAPTIVE_SUBDIVISION (1<<12)
#define PART_GI_MIN_MESH_SIZE			(1<<13)
#define PART_GI_INITIAL_MESH_SIZE		(1<<14)
#define PART_GI_CONTRAST_THRESHOLD		(1<<15)
//@}

// ASzabo|june.04.03
/*! \defgroup partid_node_render_property Node Render Property PartIDs
 Sent with \ref REFMSG_NODE_RENDERING_PROP_CHANGED 
 \todo Document individual defines */
//@{
#define PART_REND_PROP_RENDERABLE					(1<<0)
#define PART_REND_PROP_CAST_SHADOW					(1<<1)
#define PART_REND_PROP_RCV_SHADOW					(1<<2)
#define PART_REND_PROP_RENDER_OCCLUDED				(1<<3)
#define PART_REND_PROP_VISIBILITY					(1<<4)
#define PART_REND_PROP_INHERIT_VIS					(1<<5)
#define PART_REND_PROP_PRIMARY_INVISIBILITY			(1<<6)
#define PART_REND_PROP_SECONDARY_INVISIBILITY		(1<<7)
//@}

/*! \defgroup partid_disp_props PartIDs for Changing Display Properties
PartIDs sent with the \ref REFMSG_NODE_DISPLAY_PROP_CHANGED message.
 \todo Document individual defines */
//@{
#define PART_DISP_PROP_IS_HIDDEN					(1<<0)
#define PART_DISP_PROP_IS_FROZEN					(1<<1)
#define PART_DISP_PROP_SHOW_FRZN_WITH_MTL 			(1<<2)
#define PART_DISP_PROP_BOX_MODE 					(1<<3)
#define PART_DISP_PROP_BACKCULL 					(1<<4)
#define PART_DISP_PROP_ALL_EDGES 					(1<<5)
#define PART_DISP_PROP_BONE_ONLY 					(1<<6)
#define PART_DISP_PROP_BONE_AS_LINE 				(1<<7)
#define PART_DISP_PROP_SHOW_PATH 					(1<<8)
#define PART_DISP_PROP_VERT_TICKS 					(1<<9)
#define PART_DISP_PROP_XRAY_MTL 					(1<<10)
#define PART_DISP_PROP_IGNORE_EXTENTS 				(1<<11)
#define PART_DISP_PROP_COLOR_VERTS 					(1<<12)
//@}

/** \defgroup partid_misc Miscellaneous PartIDs */
//@{
//! \brief Special part ID that prevents the radiosity engine from processing the change
#define PART_EXCLUDE_RADIOSITY (1<<29)
//! \brief Special part ID sent by visibility controllers 
/** Sent when they change the hidden in viewport state. Sent with \ref REFMSG_CHANGE message. */
#define PART_HIDESTATE		(1<<30)
//! \brief Special part ID sent by MAXScript when it changes an object's parameter. 
/** Sent with \ref REFMSG_CHANGE message */
#define PART_MXS_PROPCHANGE			(1<<16)

/*! \name PartIDs when notify dependents is called with REFMSG_FLAGDEPENDENTS 
\sa \ref REFMSG_FLAGDEPENDENTS */
//@{
//! \todo describe this define
#define PART_PUT_IN_FG				(1<<0)
//! \todo describe this define
#define PART_SHOW_DEPENDENCIES		(1<<1)
//! \todo describe this define
#define PART_SHOWDEP_ON				(1<<2)
//@}

//@}

//@}      ************** END OF PartIDs

/** 
	\defgroup Reference_Messages Reference Messages 

This section describes some of the common messages used by references and the
meaning of the \ref partids parameter associated with these messages. Some messages are
sent by the system while others are sent by the plugin. Each method may need to 
pass along additional information so the reference maker may process the message.
This information is passed in the \ref partids parameter. The meaning of the 
information stored in the \ref partids is specific to the message sent along with it.

\note Not all messages use the \ref partids parameter (in fact most don't). In 
these cases the \ref partids will be set to 0. If the plugin is sending the message, it
should set the \ref partids to 0 if not used. In the cases where the \ref partids is used, it
is documented below.\n

Developers who define their own reference messages should do so using a value greater than the following:
\code
#define REFMSG_USER 0x00010000
\endcode
The system uses numbers less than this value.

\sa \ref partid_object_channel

 */

//@{

//! \brief The message passed to notify and evaluate.
typedef unsigned int RefMessage;

//! \brief This tests for a cyclic reference. It will return REF_FAIL if there is a loop.
#define REFMSG_LOOPTEST 				0x00000010

//! \brief This message is sent when a ReferenceTarget is deleted. 
/*! In response to this notification, a ReferenceMaker that makes a weak reference 
to the ReferenceTarget or if the target is a node (INode), must set its reference  
storage (data member) to NULL.
Note that the ReferenceMaker should not call ReferenceMaker::DeleteReference() or
ReferenceMaker::ReplaceReference() to set its reference handle to NULL. Also, the
ReferenceMaker should not delete itself in response to this notification.
*/
#define REFMSG_TARGET_DELETED 			0x00000020
//! \brief Used Internally
#define REFMSG_MODAPP_DELETING			0x00000021
//! \brief Used Internally
#define REFMSG_EVAL  					0x00000030
//! \brief Used Internally
#define REFMSG_RESET_ORIGIN 			0x00000040

//! \brief Sent to dependents of a ReferenceTarget that has changed in some way.
/*! Any time a reference target changes in a way that may affect items which 
reference it, this message should be sent. Note the following for the \ref partids 
that are sent during this message:
-# \ref PART_HIDESTATE This is a special \ref partids sent by visibility controllers
	when they change the hidden in viewport state.
-# \ref PART_TM This is passed in \ref partids when the reference is to a node in the
	scene and its transformation matrix has changed.
-# \ref PART_OBJECT_TYPE This is sent if the object type changes.

Objects and Modifier set the \ref partids to the channel which changed. See the section
on the Geometric Pipeline for more information on channels. 
There are several specific \ref partids referring to channels. See \ref partid_object_channel. */
#define REFMSG_CHANGE 					0x00000050

//! \brief Used Internally
#define REFMSG_FLAGDEPENDENTS			0x00000070
//! \brief Used Internally
#define REFMSG_TARGET_SELECTIONCHANGE	0x00000080

//! \brief This is used by modifiers to indicate when they are beginning an edit. 
/*! For example, in SimpleMod::BeginEditParams() this message is sent. */
#define REFMSG_BEGIN_EDIT				0x00000090
//! \brief This is used by modifiers to indicate when they are ending an edit. 
/*! For example in SimpleMod::EndEditParams() this message is sent. Typically 
what a modifier will do while it is being edited it will have its LocalValidity()
return NEVER so that a cache is built before it. This will ensure it is more 
interactive while it is being edited. When this message is sent to indicate 
the edit is finished the system can discard the cache. */
#define REFMSG_END_EDIT					0x000000A0

//! \brief Used Internally
#define REFMSG_DISABLE					0x000000B0
//! \brief Used Internally
#define REFMSG_ENABLE					0x000000C0
//! \brief Used Internally
#define REFMSG_TURNON					0x000000D0
//! \brief Used Internally
#define REFMSG_TURNOFF					0x000000E0
//! \brief Used Internally
#define REFMSG_LOOKAT_TARGET_DELETED  	0x000000F0 
//! \brief Used Internally
#define REFMSG_INVALIDATE_IF_BG			0x000000F1

//! \brief This is used by modifiers to indicate that their apparatus (gizmo) is displayed.
/*! For example in SimpleMod::BeginEditParams() this message is sent. */
#define REFMSG_MOD_DISPLAY_ON			0x000000F2
//! \brief This is used by modifiers to indicate that their apparatus (gizmo) is no 
//! longer displayed.
#define REFMSG_MOD_DISPLAY_OFF			0x000000F3

//! \brief Modifier uses this to tell Modapps to call their Eval() procedure.
/*! This is sent by a modifier to cause its ModApp to call Eval() on the modifier. 
If a modifier wants its ModifyObject() method to be called it can send this message.
The \ref partids should contain the bits that specify which channels are to be evaluated, 
for example PART_GEOM|PART_TOPO or ALL_CHANNELS. The interval passed should be set 
to Interval(t, t), where t is the time the to evaluate. Note that before 
NotifyDependents() returns, ModifyObject() will be called. */
#define REFMSG_MOD_EVAL					0x000000F4

//! \brief Ask if it is ok to change topology.  
/*! If any dependents have made topology-dependent modifications, they should return REF_FAIL:
	\li A return of REF_SUCCEED means that the answer is YES, it is okay to change 
	the topology. 
	\li A return of REF_FAIL means that the answer is NO, it is not okay to change 
	the topology. */
#define REFMSG_IS_OK_TO_CHANGE_TOPOLOGY	0x000000F5


// This main purpose of these notifications is to cause the tree
// view to update when one of these events takes place.

//! \brief Sent by a node when it has a child linked to it or unlinked from it.
#define REFMSG_NODE_LINK				0x000000F6

//! \brief Sent by a node when it's name has been changed. 
/*! For example, the path controller displays the name of the node in the scene 
which it follows. It responds to this message by changing the name displayed in 
the UI. */
#define REFMSG_NODE_NAMECHANGE			0x000000F7

//! <b>DEPRECATED</b> Sent by a node (or derived object) when the object it references changes.
/*! \note This message is no longer used. It is basically synonymous with 
	\ref REFMSG_SUBANIM_STRUCTURE_CHANGED. */
#define REFMSG_OBREF_CHANGE				0x000000F8

//! \brief Sent by a derived object when a modifier is a added or deleted.
#define REFMSG_MODIFIER_ADDED			0x000000F9

//! \brief Sent when an animatable switches controllers for one of it's parameters.
#define REFMSG_CONTROLREF_CHANGE		0x000000FA

//! \brief A parameter block sends the message to it's client when it needs the 
//! anim name of the ith parameter. 
//*! \ref partids is set to a pointer to a GetParamName structure defined in iparamb.h */
#define REFMSG_GET_PARAM_NAME			0x000000FB

//! \brief A parameter block sends this message to it's client when it needs to know
//! the dimension type of the ith parameter.
/*! \ref partids is set to a pointer to a GetParamDim structure  defined in iparamb.h */
#define REFMSG_GET_PARAM_DIM			0x000000FC

//! \brief A controller can send this to it's client to get it's param dimension.
//! It should set \ref partids to a ParamDimension.
#define REFMSG_GET_CONTROL_DIM			0x000000FD

//! \brief Sent when a Nodes transform matrix (TM) has changed in a different time. 
/*! This message is sent by a node when it's TM has changed because it was evaluated
	at a different time. Normally this isn't necessary - anyone depending on the
	node's TM would have a validity interval that reflected the validity of the
	TM. The axis system doesn't store a validity interval (it probably should) so this
	message is needed for it. */
#define REFMSG_TM_CHANGE 				0x000000FE

//! A node sends this message when it's animation range changes.
#define REFMSG_RANGE_CHANGE				0x000000FF

//! Sent to the tree view when an animatable's line height changes.
#define REFMSG_LINEHEIGHT_CHANGE		0x00000100

//! \brief A controller should send this message to the track view when it becomes animated.
/*! If the user has the animated only filter on then the track view will display this item. */
#define REFMSG_BECOMING_ANIMATED		0x00000101

//! \brief Used in the TrackView to regenerate an objects sub components.
/*! This is intended mainly for the TrackView to tell it to regenerate it's view
	below the message sender's level. If a plugin has a variable number of 
	parameters this message may be used. */
#define REFMSG_SUBANIM_STRUCTURE_CHANGED 0x00000102

//! \brief A target has had a reference deleted.
/* Materials use this to tell the material whether or not the number of node
references has gone to zero. */
#define REFMSG_REF_DELETED			0x00000103

//! \brief A target has had a reference added. 
/* Materials use this to tell the material whether or not the number of node
references are non zero.*/
#define REFMSG_REF_ADDED			0x00000104

//! \brief Sent by an object that provides branching in the history to notify that
//! the structure of the branches changed.
#define REFMSG_BRANCHED_HISTORY_CHANGED	0x00000105

//! \brief The selection set sends this notification when it receives \ref REFMSG_CHANGE
//! from an item in the selection set. 
/*! The selection set doesn't propagate the \ref REFMSG_CHANGE message. */
#define REFMSG_NODEINSELSET_CHANGED	0x00000106

//! \brief This method is used to see if this reference target depends on something.
/*! If the \ref partids is nonzero, the dependency test will 
include child nodes. Otherwise, child nodes will not be considered dependents. See
ReferenceTarget::BeginDependencyTest(). 
 */
#define REFMSG_TEST_DEPENDENCY	0x00000107

//! \brief Queries whether to display "Parameters" in the track view.
/*!	A Parameter block sends this to its client to ask if it should display a 
	distinct "Parameters" level in the track view hierarchy. A pointer to a 
	boolean is passed in for \ref partids: set this to the desired answer. The default
	is NO -- in this case the message doesn't need to be responded to. */
#define REFMSG_WANT_SHOWPARAMLEVEL 	0x00000108

//! \brief Sent before a paste has been done.
/*! Sent as \ref partids is a pointer to a data structure containing three 
	RefTargetHandle's: the reference maker, the old target,  and the new target. The 
	message is sent to the reference maker initially. */
#define REFMSG_BEFORE_PASTE 	0x00000109
//! \brief Sent after a paste has been done.
/*! Sent as \ref partids is a pointer to a data structure containing three 
RefTargetHandle's: the reference maker, the old target,  and the new target. The 
message is sent to the reference maker initially. */
#define REFMSG_NOTIFY_PASTE 	0x0000010A

//! \brief Sent when a UV Generator changes symmetry, so interactive texture display 
//! updates.
#define REFMSG_UV_SYM_CHANGE    0x0000010B

//! \brief Gets the node's name
/*! The first node that gets this message will fill in the MSTR, which \ref partids 
points to, with its name and stop the message from propagating. */
#define REFMSG_GET_NODE_NAME			0x0000010C

//! Sent by the selection set whenever it has just deleted nodes
#define REFMSG_SEL_NODES_DELETED		0x0000010D

//! Sent before a reference target is pasted. Sent by the target about to be replaced.
#define REFMSG_PRENOTIFY_PASTE 	0x0000010E

//! \brief Sent when a shape starts changing.
/*! Sent when a shape enters a state where it'll be changing a lot and it
	would be a good idea for anybody using it for mesh generation to suppress
	updates. */
#define REFMSG_SHAPE_START_CHANGE	0x0000010F

//! \brief Sent when a shape stops changing.
#define REFMSG_SHAPE_END_CHANGE		0x00000110

//! \brief A texture map has been removed. 
/*! This tells the material editor to remove it from the viewport if it is active. */
#define REFMSG_TEXMAP_REMOVED	0x00000111

//! \brief Sent by an unselected node to see if any selected nodes depend on it.
/*! The \ref partids param points to a boolean. If a selected node receives this 
message it should set the boolean to true and return REF_STOP. */
#define REFMSG_FLAG_NODES_WITH_SEL_DEPENDENTS	0x00000112

//! \brief Sent by objects which contain shapes when the shape position changes.
#define REFMSG_CONTAINED_SHAPE_POS_CHANGE 0x00000120
//! \brief Sent by objects which contain shapes when the shape position changes.
#define REFMSG_CONTAINED_SHAPE_SEL_CHANGE 0x00000121
//! \brief Sent by objects which contain shapes when general changes occur.
#define REFMSG_CONTAINED_SHAPE_GENERAL_CHANGE 0x00000122

//! \brief Select sub-object branch.
/*! When an object receives this message it should do what ever it needs
	to do (usually select the appropriate sub-object) to make the dependent
	object be the object returned from GetPipeBranch().
	The \ref partids will point to an INode pointer that will be filled in by
	the first node to receive this message. */
#define REFMSG_SELECT_BRANCH	0x00000130

//! \brief Sent when a user begins a mouse operation in the viewport
/*! These messages are sent to dependents of the transform matrix controllers
for selected objects when the user begins and ends a mouse transformation in the
viewports (move/rotate/scale). */
#define REFMSG_MOUSE_CYCLE_STARTED		0x00000140

//! \brief Sent when a user ends a mouse operation in the viewport
#define REFMSG_MOUSE_CYCLE_COMPLETED	0x00000150

//! \brief Sent when linking nodes
/*! Sent by a node to other nodes (which depend on that node) when the
user attempts to link another node to a node. The \ref partids parameter contains a 
pointer to the new parent node. */
#define REFMSG_CHECK_FOR_INVALID_BIND	0x00000161

//! \brief Sent when a cache is dumped in the pipeline. 
/*! Sent when a cache is dumped in the pipeline. A \ref REFMSG_CHANGE message used to
be sent, however that was misleading since the object itself didn't change even 
though any old object pointer has become invalid. For example, if a path 
controller depends on a spline object and that object dumps some caches in the 
pipeline, the path controller hasn't actually changed. 
Also modifiers that reference INode to only get its transformation data should
stop the propagation of this message. For example: 
\code
case REFMSG_OBJECT_CACHE_DUMPED:
	return REF_STOP;
	break;
\endcode

*/

#define REFMSG_OBJECT_CACHE_DUMPED	0x00000162

//! \brief Sent by Atmospheric or Effect when it makes or deletes a reference to a node.
/*! When Atmospherics or Effects add or delete a gizmo they should send this message 
via NotifyDependents(). */
#define REFMSG_SFX_CHANGE 		0x00000170

//! \brief For internal use only. 
/*! Sent when updating object xrefs. \ref partids contains new material. When a node receives this message
it will set its material to the new one. */
#define REFMSG_OBJXREF_UPDATEMAT	0x00000180
//! \brief For internal use only. 
/*! Sent when updating object xrefs. \ref partids contains new controller. When a node receives this message
it will set its controller to the new one. */
#define REFMSG_OBJXREF_UPDATECTRL	0x00000181

//! \name Defines XRef Messages
//! \brief Sent to build a list of nodes which use a particular XRef object.
//@{
//! \brief For Internal use only.
#define REFMSG_OBJXREF_GETNODES			0x00000190
//! \brief For internal use only.
/*! PartID points to a table of base node pointers. */
#define REFMSG_CTRLXREF_GETNODES		0x00000191
//! \brief For internal use only.
/*! PartID points to a table of base node pointers. */
#define REFMSG_MTLXREF_GETNODES			0x00000192
//@}

//! \brief Sent when objects are replaced from another scene (File->Replace). 
/*! Other objects referencing the object that is replaced may want to perform 
some validity checking; this message is more specific than \ref REFMSG_SUBANIM_STRUCTURE_CHANGED.
*/ 
#define REFMSG_OBJECT_REPLACED		0x00000200

//! \brief Sent when nodes wireframe color is changed
#define REFMSG_NODE_WIRECOLOR_CHANGED 0x00000210

//! \brief Indicates that the subobject types have changed and that the StackView should be updated.
#define REFMSG_NUM_SUBOBJECTTYPES_CHANGED 0x00000211

//! \brief Returns a unique (per session) node handle integer
/*! The partID contains a pointer to a ULONG. The first node that gets
this message will assign its node handle to this ULONG, and will return
REF_STOP to terminate further propagation of the message. */
#define REFMSG_GET_NODE_HANDLE			0x00000220

//! \brief This will cause EndEditParams to be called on the object displayed in the modify panel
#define REFMSG_END_MODIFY_PARAMS	0x00000230
//! \brief This will cause BeginEditParams to be called on the object displayed in the modify panel
#define REFMSG_BEGIN_MODIFY_PARAMS 0x00000231

//! \brief Sent when a Tab<> is deleted.
/*! Sent by a ParamBlock2 to its owner whenever a reftarg element in a Tab<> 
	parameter is forcibly deleted and the reference set to NULL (typically for 
	INODE_TABs when a scene node is deleted in the viewport. */
#define REFMSG_TAB_ELEMENT_NULLED 0x00000232

//! \brief Sent to merged objects so that they can convert node handles.
/*! After merging nodes into the scene, all merged objects will receive this reference 
notification. The PartID will be a pointer to a merge manager interface that you 
can use to see if a specific handle was converted and convert between the old and  
the new handle.
\code
IMergeManager* pMergeManager = (IMergeManager*)partID; 
\endcode
Node handles can change when a scene is merged and if you keep track of nodes
using their handles you need to intercept this message. The PartID will be a
pointer to an IMergeManager object that you can use to map between the old 
and new handle. */
#define REFMSG_NODE_HANDLE_CHANGED 0x00000233

//! \brief The pipeline was reevaluated and the wscache was updated.
/*! Sent from the node (without propagation) whenever the world state cache gets updated
 (e.g. when the pipeline gets reevaluated). */
#define REFMSG_NODE_WSCACHE_UPDATED 0x00000234

//! This notification is sent after a new material was assigned to a node
#define REFMSG_NODE_MATERIAL_CHANGED    0x00000235

//! \brief This notification is sent to dependents when a subanim's changes order.
/*!  It is used by things like scripted plugin's and custom attributes to tell 
	expression and wire controllers when the user redefines the ordering of 
	parameters so these controllers can keep pointing at the correct parameter.  
	The PartID is a Tab<DWORD>* in which each DWORD contains an old-to-new mapping
	with the LOWORD() = old subanim number and the HIWORD() = new subanim number.
	A new subanim ID of -1 implies the subanim was removed.   
	See maxsdk/samples/control/exprctrl.cpp for example use. NOTE: If you send 
	this message, the 'propagate' argument of NotifyDependents must be false. 
	Otherwise, dependents of dependents think that their ref's subAnim numbering
	is changing.*/
#define REFMSG_SUBANIM_NUMBER_CHANGED    0x00000236   

//! \brief 
/*! \todo Document this define */
#define REFMSG_NODE_FLAGOMB_RENDER 0x00000237 

// 
//! \brief For Internal use only.
//! Notification sent AFTER the Global Illumination (radiosity) properties of a node changed.
/*! The PartId will contain information about the property that has changed. */
#define REFMSG_NODE_GI_PROP_CHANGED	0x00000238

//! \brief Sent when key selection changes.
#define REFMSG_KEY_SELECTION_CHANGED 0x00000239

// ASzabo|june.04.03
/** Sent AFTER the Node Rendering Properties have changed
  The part id will contain information about the property that has changed.
  \sa partid_change_render_properties  */
#define REFMSG_NODE_RENDERING_PROP_CHANGED	0x00000240
//! \brief Notification sent AFTER the Node Display Properties have changed
/*! The part id will contain information about the property that has changed. */
#define REFMSG_NODE_DISPLAY_PROP_CHANGED	0x00000241

//! \brief Message type propagated to dependents of scripted Custom Attribute (CA) when CA gets a \ref REFMSG_CHANGE
/*! Scripted custom attributes sit on an object, but do not directly affect the output of that object.
If a \ref REFMSG_CHANGE message is propagated to the dependents of the CA, the \ref REFMSG_CHANGE message 
will be propagated to the dependents of the object, and those dependents will think that the object
has changed. This can result in caches being thrown away needlessly. In r8 and r9.0, when a scripted
CA received a \ref REFMSG_CHANGE, a REF_STOP result was returned stopping propagation to dependents. This
affected things like the trackbar, where moving keys on the CA wasn't causing the trackbar to update.
In r9.1, a \ref REFMSG_CHANGE notification to the scripted custom attribute causes a \ref REFMSG_MXS_CUSTATTRIB_CHANGE notification to be sent to the dependents of the CA. */
#define REFMSG_MXS_CUSTATTRIB_CHANGE 0x00000250

//! \brief This message is sent immediately prior to a node being deleted. 
/*! This allows the reference maker to handle this condition if it depends on 
the deleted node. At the time this message is sent, the node's children have not
been detached nor has the node been disconnected from its parent. This message is 
sent to only the immediate dependents of the node. This message is sent immediately
after the NOTIFY_SCENE_PRE_DELETED_NODE Broadcast Notification
*/
#define REFMSG_NODE_PRE_DELETE 			0x00000255


//! \brief This message is sent after an item has been locked. \see ILockedTracksMan
#define REFMSG_LOCKED 				0x00000260

//! \brief This message is sent after an item has been unlocked. \see ILockedTracksMan
#define REFMSG_UNLOCKED 				0x00000261

//! \brief This message is sent immediately before an object instance is updated to a new object definition
/*! MAXScript allows scripted plugin and scripted custom attribute definitions to be updated by re-evaluating 
the definition script. The new definition can add or remove local variables, parameter blocks, parameter block
items, rollouts, and rollout controls. After the new definition is evaluated, existing plugin instances are 
converted to this new definition. This message is sent to the dependents of a scripted plugin instance
immediately before the instance is converted to the new definition. The partID passed is a ReferenceTarget pointer 
to the plugin instance.
A dependent may use this notification to, for example, rebuild the rollout display for the instance.
\sa NOTIFY_OBJECT_DEFINITION_CHANGE_BEGIN
*/
#define REFMSG_OBJECT_DEFINITION_CHANGE_BEGIN 				0x00000270

//! \brief This message is sent immediately after an object instance is updated to a new object definition
/*! MAXScript allows scripted plugin and scripted custom attribute definitions to be updated by re-evaluating 
the definition script. The new definition can add or remove local variables, parameter blocks, parameter block
items, rollouts, and rollout controls. After the new definition is evaluated, existing plugin instances are 
converted to this new definition. This message is sent to the dependents of a scripted plugin instance
immediately after the instance is converted to the new definition. The partID passed is a ReferenceTarget pointer 
to the plugin instance.
A dependent may use this notification to, for example, rebuild the rollout display for the instance.
\sa NOTIFY_OBJECT_DEFINITION_CHANGE_END
*/
#define REFMSG_OBJECT_DEFINITION_CHANGE_END 				0x00000271

//! \brief Message numbers above this value can be defined for use by sub-classes, below are reserved.
#define REFMSG_USER		0x00010000


// JBW 9.9.00 
//! \name Messages sent from ReferenceMaker to ReferenceTarget
/*! \brief Codes used by a ReferenceMaker to send 'reverse' notification messages to a RefTarget.
Developers who define their own reference target messages should do so using a value greater than:
\code
#define TARGETMSG_USER 0x00010000.
\endcode 
*/
//@{
//! \brief Target notify message IDs above this value can be used by other plugins
/*! Best if large random IDs */
#define TARGETMSG_USER				0x00010000
//! \brief Send to a Node's ObjectRef when the node is attaching the object to itself	
#define TARGETMSG_ATTACHING_NODE	0x00000010		
//! \brief Send to a Node's ObjectRef when the node is about to be explicitly deleted
#define TARGETMSG_DELETING_NODE		0x00000020		
//! \brief Send to a Node's ObjectRef when the node is detaching the object from itself
/*! If your plugin utilizes this new mechanism, be 
sure that your clients are aware that they must run your plugin with 3ds max
version 4.2 or higher. */
#define TARGETMSG_DETACHING_NODE	0x00000030
//@}

//@} END OF Reference_Messages


//! \brief Return codes for reference handling and message processing methods.
enum RefResult {
	//! The operation failed.
	REF_FAIL = 0,
	//! Return this from your implementation of ReferenceMaker::NotifyRefChanged() in order 
	//! to immediately stop sending the reference message to dependents of the RefereneTarget
	//! the message originates from.
	REF_HALT = 0,
	//! The operation succeeded.
	REF_SUCCEED,
	//! The results of the operation don't matter.
	REF_DONTCARE,
	//! Return this from your implementation of ReferenceMaker::NotifyRefChanged() 
	//! to stop propagating the reference message to your dependents, but allow it
	//! to be sent to the other dependents of the ReferenceTarget the message originates from.
	REF_STOP,
	//! Attempts to delete an invalid reference will return this value.
	REF_INVALID,
	//! Return this from your implementation of ReferenceMaker::NotifyRefChanged() 
	//! in response to a REFMGS_TAGET_DELETED message to indicate that all the 
	//! reference targets you depended on have been deleted, and you also wish to be
	//! deleted from the scene.
	REF_AUTO_DELETE
};

class ReferenceTarget;
/*! \brief Creates a typename for ReferenceTarget pointers */
typedef ReferenceTarget* RefTargetHandle;

class ReferenceMaker;
/*! \brief Creates a typename for ReferenceMaker pointers */
typedef ReferenceMaker* RefMakerHandle;

// This replaces the .Valid() method of handles.
#define VALID(x) (x)

//! If this super class is passed to NotifyDependents() all dependents will be notified
#define NOTIFY_ALL		0xfffffff0

class PostPatchProc;

//! \brief For remapping references during a Clone.
/*! \sa CloneRefHierarchy(), ReferenceTarget::Clone()
	This class is used for remapping references during a Clone. It is used when 
	cloning items that are instanced so that the plugin can maintain the same 
	instance relationship within the clone. All methods of this class are 
	implemented by the system. */
class RemapDir: public MaxHeapOperators {
	public:
//! \name Clone Methods
//@{
		//! \brief Returns a pointer to a target's clone if it was cloned or NULL otherwise.
		virtual	RefTargetHandle FindMapping(RefTargetHandle from) = 0;

		//! \brief Creates a clone of a reference target only if it hasn't been cloned already.
		/*! This method will clone the item if was not already cloned, register 
			the item and its clone with the RemapDir, and return a pointer to 
			the clone. If the item was previously cloned, a pointer to the
			clone is returned. 
			In the ReferenceTarget::Clone() procedure when an item is cloning 
			itself it should clone all its references. Instead of calling Clone()
			on its references it should instead call this method passing 
			it the item to clone. This ensures that all objects are cloned
			at most one time. For example:
			\code 
				class MyDerivedPlugin
					: public MyBasePlugin
				{
					const int MY_REFERENCE = 1;

					ReferenceTarget* Clone(RemapDir& remap)
					{
						ReferenceTarget* result = new MyDerivedPlugin();
						BaseClone(this, result, remap);
						return result;
					}

					void BaseClone(ReferenceTarget* from, ReferenceTarget* to, RemapDir& remap)
					{
						if (!to || !from || from == to)
							return;    
						MyBasePlugin::BaseClone(from, to, remap);
						to->ReplaceReference(MY_REFERENCE, remap->CloneRef(from->GetReference(MY_REFERENCE)));
					}
				};
			\endcode
			\param  oldTarg - This is the item that is to be cloned.
			\return A pointer to the clone of the item.
		*/
		virtual RefTargetHandle CloneRef(RefTargetHandle oldTarg);

		//! \brief This method is used to back patch the pointer for cloned items.
		/*!  If oldTarg has already been cloned, then the RefTargetHandle variable pointed 
		to by patchThis is set to the pointer of the clone. If oldTarg has not been cloned, 
		a back patch procedure is registered that is run after all cloning is completed but 
		before the PostPatchProcs are run. The back patch procedure checks to see if 
		oldTarg has been cloned, and if so the the RefTargetHandle variable pointed to
		by patchThis is set to the pointer of the clone. If oldTarg has not been cloned, 
		the RefTargetHandle variable pointed to by patchThis is set to oldTarg.
		Because there may be a delayed write to the RefTargetHandle variable, this variable 
		should not be a stack variable.
		Note that this method is primarily used when an object being cloned holds a raw 
		pointer to a ReferenceTarget (i.e., it does not hold a reference to the 
		ReferenceTarget). In general, this is not safe due to the potential for a 
		dangling pointer when that ReferenceTarget is deleted. It is better to use a
		SingleRefMaker or a SingleWeakRefMaker member variable to hold the pointer
		as it will hold a reference to the ReferenceTarget and properly NULL the pointer
		when/if the ReferenceTarget is deleted.
		\see SingleRefMaker, SingleWeakRefMaker
		\param  patchThis - The RefTargetHandle variable to be set.
		\param  oldTarg - The original target.
		*/
		virtual void PatchPointer(RefTargetHandle* patchThis, RefTargetHandle oldTarg) = 0;
		
		//! \brief Adds a Procedure that is called after cloning and back patching.
		/*! This method is used for adding a PostPatchProc whose procedure method
		    is called after the reference hierarchy has been cloned and any 
		    back patching has occurred. These will be called in the order that
		    they are registered. The PostPatchProc procs are called from 
		    the RemapDirImp::Backpatch(). More information can be found at the
		    RefTargMonitorRefMaker::Proc method. 
		    See NodeMonitor::Clone for an example.
		    \param  proc - Points to the callback object.
		    \param  toDelete - If true, the callback object is deleted when the RemapDir is deleted.
		*/
		virtual void AddPostPatchProc(PostPatchProc* proc, bool toDelete) = 0;
//@}

//! \name Internal methods
//@{
		//! \brief Registers a mapping of original object to clone
		/* This method is used to register the mapping of an object to the clone
		of the object. This method is called by CloneRef and by 
		ReferenceTarget::BaseClone, and so does not normally need to be
		called from plugins. One exception is for code similar to the following:
		\code 
		PRSControl& PRSControl::operator=(const PRSControl& ctrl) {
			RemapDir *remap = NewRemapDir();
			ReplaceReference(PRS_POS_REF,remap->CloneRef(ctrl.pos));
			ReplaceReference(PRS_ROT_REF,remap->CloneRef(ctrl.rot));
			ReplaceReference(PRS_SCL_REF,remap->CloneRef(ctrl.scl));

			// Make sure that if any sub-controller references this 
			// controller that it will get backpatched correctly  
			remap->AddEntry(ctrl,this);
			remap->DeleteThis();
			mLocked = ctrl.mLocked;
			return(*this);
		}
		\endcode 

		\param  hfrom - The item that was cloned.
		\param  hto - The clone of the item.
		*/
		virtual void AddEntry(RefTargetHandle hfrom, RefTargetHandle hto)=0;
		//! \brief Runs back patch procedures and PostPatchProcs
		/* This method first runs any back patch procedures the RemapDir registered
		(see PatchPointer) and then any PostPatchProcs that were registered (see
		AddPostPatchProc).
		This method is run by the RemapDir as it is being deleted, and so does not 
		normally need to be called from plugins.
		*/
		virtual void Backpatch()=0;
		//! \brief Used internally
		virtual bool BackpatchPending()=0;
		//! \brief Used internally
		virtual void Clear()=0;
		//! \brief Used internally
		virtual void ClearBackpatch()=0;
		//! \brief Used internally
		virtual	void DeleteThis()=0;
		//! \brief Used internally
		virtual	~RemapDir() {}
//@}
};

class PostPatchProc: public MaxHeapOperators {
public:
	virtual ~PostPatchProc(){}
    virtual int Proc(RemapDir& remap) = 0;
};

class DeleteRefRestore;
class MakeRefRestore;
class ParamBlock;
class ISave;
class ILoad;
class ILoadImp;
class DependentIterator;

//! \name Defines enumeration return values
//! \brief Possible return values for DependentEnumProc::proc()
//@{
//! Continues enumeration
#define DEP_ENUM_CONTINUE	0
//! Halts enumeration
#define DEP_ENUM_HALT		1
//! Skip enumeration
#define DEP_ENUM_SKIP		2	
//@}

//! \brief A callback class for enumerating dependents.
/*! This class is a callback object for the ReferenceMaker::DoEnumDependentsImpl()
and ReferenceMaker::DoEnumDependents() methods. The proc() method is called by the system. */
class DependentEnumProc: public MaxHeapOperators {

	friend class ReferenceTarget;
	friend class ReferenceMaker;

	class DependentEnumProcImplData;
	const MaxSDK::Util::AutoPtr<DependentEnumProcImplData> mDependentEnumProcImplData;

protected:
	CoreExport DependentEnumProc();
	CoreExport virtual ~DependentEnumProc();

	// Hide it - no copy constructor or assignment to the public.
	DependentEnumProc(const DependentEnumProc&); // not implemented
	DependentEnumProc& operator=(const DependentEnumProc& rhs); // not implemented

public:
		//! \brief This is the method called by system from ReferenceTarget::DoEnumDependentsImpl().
		/*! 
			\param  rmaker - A pointer to the reference maker
			\return One of the following values:
			\li <b>DEP_ENUM_CONTINUE</b>: This continues the enumeration
			\li <b>DEP_ENUM_HALT</b>: This stops the enumeration.
			\li <b>DEP_ENUM_SKIP</b>: Reference Targets can
			have multiple Reference Makers (dependents). In certain instances 
			when DoEnumDependents() is used to enumerate them you may not want to 
			travel up all of the "branches". By returning DEP_ENUM_SKIP from this
			method you tell the enumerator to not enumerate the current Reference
			Maker's dependents but not to halt the enumeration completely.
			\see ReferenceTarget::DoEnumDependentsImpl(DependentEnumProc* dep)
			\see ReferenceTarget::DoEnumDependents(DependentEnumProc* dep)
		*/
		virtual	int proc(ReferenceMaker *rmaker)=0;

		//! \brief This method sets and checks whether a ReferenceMaker was visited
		/*! This method is used to check whether a ReferenceMaker was previously visited
			by this callback object, and registers it as having been visited if not.
			This is used to only call the proc on each ReferenceMaker once. If you 
			override ReferenceTarget::EnumDependentsImp, you should use this method
			to process your instance only if it was not previously visited. 
			\param  rmaker - A pointer to the reference maker
			\return Returns true of the rmaker was previously visited, false if not.
			\see ReferenceTarget::DoEnumDependentsImpl(DependentEnumProc* dep)
		*/
		CoreExport bool CheckIfAndSetAsVisited(ReferenceMaker *rmaker);

	};

//! \brief A callback class for saving dependents
/*! Instances of this class are passed to ReferenceMaker::SaveEum. Typically, 
terminate is called passing in the ReferenceMaker instance, and if terminate returns
FALSE SaveEnum is called on ReferenceMaker's custom attribute container, its
direct references, and its indirect references. Finally, proc is called passing in 
the ReferenceMaker instance. If terminate returns TRUE, SaveEnum typically 
immediately returns.
*/
class SaveEnumProc: public MaxHeapOperators {
	public:
		//! \brief This is the method allows processing the passed ReferenceMaker.
		/*! Any processing needing to be performed on the passed RefenceMaker after enumerating the references held 
			by the ReferenceMaker would be done in this method.
			\param  rmaker - A pointer to the reference maker
		*/
		virtual	void proc(ReferenceMaker *rmaker)=0;
		//! \brief This is the method specifies whether to continue processing the passed ReferenceMaker.
		/*! This method specifies whether to continue processing the passed ReferenceMaker. Typically the method
			would check to see if the ReferenceMaker has already been processed and return TRUE if it has. Any 
			processing needing to be performed before enumerating the references held by the ReferenceMaker would
			be done in this method.
			\param  rmaker - A pointer to the reference maker
			\return FALSE if to continue enumerating the references held by the ReferenceMaker and call proc on the 
			ReferenceMaker, TRUE to terminate processing of the ReferenceMaker.
		*/
		virtual int terminate(ReferenceMaker *rmaker)=0; 
	};


//! \brief Implements a default version of a RemapDir.
/*! \sa CloneRefHierarchy(), ReferenceTarget::Clone()
	Instances of this class are used as the default argument to the 
	ReferenceTarget::Clone method. 
	When the boolean use_A_WORK2_flag is set to TRUE, 
	the remap directory will set this flag on all entries in the remap directory, 
	and will assume that any object that DOESN'T have this flag set is not in 
	the remap directory. This avoids the search through the directory and speeds
	up things greatly. When using this feature, you must first clear A_WORK2 on 
	all of the objects being cloned. DS: 3/17/00.
	When the instance is deleted, the Backpatch() method is automatically called 
	if backpatch operations were added but never performed.
	\pre First clear the A_WORK2 flag on all objects being cloned.
	\post Flag is set on all entries in remap directory.
	\param  use_A_WORK2_flag - used to set this flag on all entries in the 
	remap directory.
*/
class DefaultRemapDir: public RemapDir
{
private:
	const MaxSDK::Util::AutoPtr<RemapDir> mRemapDir;
	// Copy construction and assignment of objects of this class is disallowed
	DefaultRemapDir(const DefaultRemapDir& aDefaultRemapDir); // not implemented
	DefaultRemapDir& operator=(const DefaultRemapDir& rhs); // not implemented
public:
	CoreExport DefaultRemapDir(BOOL use_A_WORK2_flag = FALSE);
	CoreExport ~DefaultRemapDir();
	RefTargetHandle FindMapping(RefTargetHandle from);
	void AddEntry(RefTargetHandle hfrom, RefTargetHandle hto);
	void PatchPointer(RefTargetHandle* patchThis, RefTargetHandle oldTarg);
	void Backpatch();
	bool BackpatchPending();
	void Clear();
	void ClearBackpatch();
	void DeleteThis();
	void AddPostPatchProc(PostPatchProc* proc, bool toDelete);
};

//! \brief Gets a pointer to a default version of a RemapDir.
/*! \sa CloneRefHierarchy(), ReferenceTarget::Clone()
	You must delete the RemapDir returned by this function when done using it via the DeleteThis method. 
	When the boolean use_A_WORK2_flag is set to TRUE, 
	the remap directory will set this flag on all entries in the remap directory, 
	and will assume that any object that DOESN'T have this flag set is not in 
	the remap directory. This avoids the search through the directory and speeds
	up things greatly. When using this feature, you must first clear A_WORK2 on 
	all of the objects being cloned. DS: 3/17/00.
	When the instance is deleted, the Backpatch() method is automatically called 
	if backpatch operations were added but never performed.
	\pre First clear the A_WORK2 flag on all objects being cloned.
	\post Flag is set on all entries in remap directory.
	\param  use_A_WORK2_flag - used to set this flag on all entries in the 
	remap directory.
*/
CoreExport RemapDir* NewRemapDir(BOOL use_A_WORK2_flag=FALSE); 

//! \brief A ULONG Interface Id to determine if an Animatable is a Reference Maker or Target
#define REFERENCE_MAKER_INTERFACE 0x2f96f73
//! \brief A ULONG Interface Id to determine if an Animatable is a Reference Maker or Target
#define REFERENCE_TARGET_INTERFACE 0x66b073ea

class RefEnumProc;
class ReferenceSaveManager;

//! \brief A scene entity that owns other scene entities and listens to messages from them.
/*! \sa Class SingleRefMaker, Class IRefTargContainer
	Any scene entity that makes references must be derived from this class. 
	A reference creates a record of the dependency between a ReferenceMaker and a 
	ReferenceTarget. 3ds Max uses a messaging system to notify dependent entities
	about changes. This class has a method that receives the notifications its 
	targets send when they change. It has methods that return the number of 
	references if has, and methods to get and set these references. Also, there 
	are methods for creating, replacing and deleting the references. File 
	input and output is handled via methods of this class (Load() and Save()).
	
	A reference link is similar to the relationship that exists 
	between an observer and its subjects in an observer design patters, except that
	reference links also mean ownership, unless otherwise specified.

	Note that copying ReferenceMaker instances would not correctly establish reference 
	hierarchy relationships between this destination object and the references held 
	by the source object. Therefore copying of ReferenceMakers is prohibited. 
	For more information, see ReferenceMaker::Clone().

	Note that RefMakerHandle is a typename for pointers to an object of type ReferenceMaker.

	See the section on <a href="ms-its:3dsmaxsdk.chm::/refs_root.html">References</a>
	for an overview of the 3ds Max reference architecture.
*/
class ReferenceMaker : public Animatable {
	friend class DeleteRefRestore;
	friend class MakeRefRestore;
	friend class ReferenceTarget;
	friend class ParamBlock;
	friend class RootNode;
	friend class BaseNode;
	friend class ILoadImp;
	friend class RefMakerPLC;
	friend class RefList;
	
	// temporary friend declarations until figure out what the code there is doing
	friend class LayerManagerRestore; 
	friend class RepMtlRest;  
	friend class MAXMaterialLibrary;  

//! \name Construction, deletion of instances
//@{
	protected:
		//! \brief Destructor
		/*! Instances of class ReferenceMaker and classes derived from it should be deleted by 
		calling ReferenceMaker::DeleteMe() on them, rather then calling the delete operator. 
		This will ensure that the object drops all references it makes to other objects.
		*/
		CoreExport virtual ~ReferenceMaker() = 0;

	public:
		//! \brief Constructor
		CoreExport ReferenceMaker();

		//! \brief Deletes an instance of this class.
		/*! Deletes all references to and from this object, sends REFMSG_TARGET_DELETED 
		messages, and deletes the object. If the undo system is engaged (holding) it
		ensures that the operation is undo/redo-able.
		Note that this method should only be called by plugins when they need
		to delete plugin objects via ReferenceMaker pointers or in case the plugin to be
		deleted does not allow the system to automatically delete it.
		See ReferenceTarget::MaybeAutoDelete() and ReferenceTarget::AutoDelete() for more information.
		*/
		CoreExport void DeleteMe();
	//@}

		virtual void GetClassName(MSTR& s) { s = _M("ReferenceMaker"); }
		CoreExport virtual SClass_ID SuperClassID();

//! \name Making / Replacing / Deleting References
//@{
		//! \brief Used when cloning reference makers.
		/*! This routine is used when cloning reference makers, to delete old 
		reference and make a new one.
		In max r9, additional checks have have been added to ReplaceReference. 
		If parameter which is < 0, REF_FAIL will be immediately returned. 
		If parameter which is >= NumRefs,	an attempt will be made to set the reference, 
		and then another check will is made to see if which is >= NumRefs, and if so 
		REF_FAIL is returned.
			There are 2 additional tests that can be enabled in order to ensure that 
		plugins set up their references correctly. Both are executed at run-time when 
		a plugin calls ReplaceReference:\n\n
			1. The first test ensures that all references of a plugin are initialized to 
		NULL before it establishes its first reference. Whether to perform this check 
		defaults to TRUE for Hybrid and Debug builds, false for Release builds. 
		The 3DSMAX_REFCHECK_ENABLED environment variable can be used to overwrite the 
		default behaviour. See bool UtilityInterface::GetBoolEnvironmentVariable() 
		for the values this environment variable can take.\n\n
			2. The second test ensures that all references of a plugin have been set up 
		correctly. This test can also detect "dangling" references, i.e. references that 
		are non-NULL but point to non-existent reference targets. Whether to perform 
		this check defaults to FALSE for all build configurations (Debug, Hybrid and Release). 
		The 3DSMAX_REFCHECK_PARANOID_ENABLED environment variable, can be used to overwrite 
		the default behaviour. See bool UtilityInterface::GetBoolEnvironmentVariable() 
		for the values this environment variable can take.
		\param  which   - The virtual array index of the reference to replace.
		\param  newtarg - The new reference target
		\param  delOld  - If TRUE, the old reference is deleted.
		\return This is usually REF_SUCCEED indicating the reference was 
		replaced, otherwise REF_FAIL.
		*/ 
		CoreExport RefResult ReplaceReference(
			int which, 
			RefTargetHandle newtarg, 
			BOOL delOld = TRUE);

		//! \brief Deletes all references from this ReferenceMaker.
		/*! Implemented by the System. 
			\return This is always REF_SUCCEED indicating the references were deleted.
		*/
		CoreExport RefResult DeleteAllRefsFromMe();
			
		//! \brief Deletes all refs to this RefMaker/RefTarget.
		/*! Implemented by the System. This method deletes all the references to this 
		reference maker/reference target. This also sends the \ref REFMSG_TARGET_DELETED 
		message to all dependents.
		\return This is \ref REF_SUCCEED if the references were deleted; otherwise it
		is REF_FAIL.
		*/
		virtual RefResult DeleteAllRefsToMe() { return REF_SUCCEED; }
			
		//! \brief Deletes all references both to and from this item.
		/*! Implemented by the System. Deletes all references both to and from this item.
		\return This is \ref REF_SUCCEED if the references were deleted; otherwise it is \ref REF_FAIL.
		*/
		CoreExport RefResult DeleteAllRefs();

		//! \brief Deletes the specified reference
		/*! Implemented by the System. This method deletes the reference whose 
		virtual array index is passed. The other reference indices are not 
		affected, i.e. the number of references is not reduced nor are they 
		reordered in any way. Note the system calls SetReference(i, NULL) to set
		that reference to NULL. Also, if this is the last reference to the item,
		the item itself is deleted by calling its DeleteThis() method.
		\param  i - The virtual array index of the reference to delete.
		\return This is \ref REF_SUCCEED if the reference was deleted; otherwise it is \ref REF_FAIL.
		*/
		CoreExport RefResult DeleteReference( int i);

		//! \brief Tells whether this reference can be transfered.
		/*! A ReferenceMaker can choose not to let ReferenceTarget::TransferReferences() 
		affect it. Note that plugins probably should not use this. It is used by certain 
		system objects that have references.
		\param  i - Currently not used.
		\return Default to return TRUE
		*/
		virtual BOOL CanTransferReference(int i) { UNUSED_PARAM(i); return TRUE; }
//@} end group

//! \name Reference Access	
//! 3ds Max manages the access to an items references by using a virtual 
//! array. ALL classes that make references MUST implement these three 
//! methods to handle access to their references
//@{
		//! \brief Returns the total number of references this ReferenceMaker can hold.
		/*! The plugin implements this method to indicate the total number of 
			of references it can make. This includes all references whether they are NULL 
			(inactive) or non-NULL (active) at the time when this method is called. 
			A plugin can hold a variable number of references, thus the return value of 
			this method is not to be cached and reused by client code.
		\return The total number of references this plugin can hold. The default 
			implementation is return 0.
		*/
		CoreExport virtual int NumRefs();

		//! \brief Returns the 'i-th' reference
		/*! The plugin implements this method to return its 'i-th' reference. 
		The plug-in simply keeps track of its references using an integer index for 
		each one. This method is normally called by the system. 
		\param  i - The index of the reference to retrieve. Valid values are from 0 to NumRefs()-1.
		\return The reference handle of the 'i-th' reference. Note that different calls 
		to this method with the same 'i' value can result in different reference handles 
		being retrieved, as the plugin changes the scene objects it references as its 'i-th' reference.
		*/
		CoreExport virtual RefTargetHandle GetReference(int i);
protected:	
		//! \brief Stores a ReferenceTarget as its 'i-th' reference`.
		/*! The plugin implements this method to store the reference handle passed to 
		it as its 'i-th' reference. In its implementation of this method, the plugin 
		should simply assign the reference handle passed in as a parameter to the member 
		variable that holds the 'i-th' reference. Other reference handling methods such as
		ReferenceMaker::DeleteReference(), or ReferenceMaker::ReplaceReference() should
		not be called from within this method.
		The plugin itself or other plugins should not call this method directly. 
		The system will call this method when a new reference is created or an existing 
		one is replaced by calling ReferenceMaker::ReplaceReference().
		\param  i - The index of the reference to store. Valid values are from 0 to NumRefs()-1.
		\param  rtarg - The reference handle to store.
		*/
		CoreExport virtual void SetReference(int i, RefTargetHandle rtarg);
//@} End Group

public:
//! \name Loading and Saving Methods
//@{
		//! \brief Access the ReferenceSaveManager of this ReferenceMaker.
		/*! The ReferenceSaveManager is used to specify and enumerate the save reference hierarchy
		for the ReferenceMaker. If a plugin needed to specify a save reference hierarchy
		different than its normal reference hierarchy, it would implement SpecifySaveReferences()
		and specify the save reference hierarchy through the ReferenceSaveManager in that
		implementation.
		\return the ReferenceSaveManager for the ReferenceMaker.
		*/
		CoreExport ReferenceSaveManager& GetReferenceSaveManager();

		//! \brief Called for saving data.
		/*! Called by the system to allow the plugin to save 
		its data.
		\param  isave - This pointer may be used to call methods to write
		data to disk. See the section on <a href="ms-its:3dsmaxsdk.chm::/loading_and_saving.html">Loading and Saving</a>
		for an overview of the load/save process.
		\return The default implementation is return IO_OK.
		\li IO_OK means the result was acceptable, with no errors.
		\li IO_ERROR This should be returned if an error occurred.
		*/
		CoreExport virtual IOResult Save(ISave *isave);
		
		//! \brief Called for loading data.
		/*! Called by the system to allow the plug-in to load 
		its data. See the section on <a href="ms-its:3dsmaxsdk.chm::/loading_and_saving.html">Loading and Saving</a>
                for an overview of the load - save process.
		\param  iload - This interface pointer may be used to call methods
		to read data from disk.
		\return The default implementation is return IO_OK.
		\li IO_OK means the result was acceptable, with no errors.
		\li IO_ERROR This should be returned if an error occurred.
		*/
		CoreExport virtual IOResult Load(ILoad *iload);

		//! \brief Used to load old files with references
		/*! Implement this if you have added or deleted references and are 
		loading an old file that needs to have its references remapped.		
		This method is used when you have modified a ReferenceMaker to add or
		delete references, and are loading old files. It gets called during 
		the reference mapping process, after the Load() method is called. You
		determine what version is loading in the Load(), and store the 
		version in a variable which you can look at in RemapRefOnLoad() to 
		determine how to remap references. The default implementation of this
		method just returns the same value it is passed, so you don't need to
		implement it unless you have added or deleted references from your 
		class. This method makes it a lot easier to load old files when the 
		reference topology has changed. 
		\param  iref - The input index of the reference.
		\return The output index of the reference.
		*/
		virtual int RemapRefOnLoad(int iref) { return iref; }

		//! \brief Rescale size of all world units in reference hierarchy.
		/*! This method is available in release 2.0 and later only. Must call 
		ClearAFlagInHierarchy(rm, A_WORK1) or ClearAFlagInAllAnimatables(A_WORK1) before 
		doing this on a reference hierarchy.
		This may be implemented to rescale the size of all world units in 
		a reference hierarchy. Developers must call 
		\code 
		if (TestAFlag(A_WORK1))
			return;
		SetAFlag(A_WORK1);
		\endcode
		before doing this on a reference hierarchy. 
		\param  f - The scale factor.
		*/
		CoreExport virtual void RescaleWorldUnits(float f);
//@} end Group

//! \name Dependent Notification Methods
//@{ 
		//! \brief Send a notification all classes that reference this class
		/*! Since a ReferenceMaker cannot have dependents, ReferenceMaker::NotifyDependents() is implemented to simply return REF_SUCCEED.
			\sa ReferenceTarget::NotifyDependents for a detailed description of this method. */
		CoreExport virtual RefResult NotifyDependents(
			Interval changeInt, 
			PartID partID, 
			RefMessage message, 
			SClass_ID sclass = NOTIFY_ALL,
			BOOL propagate = TRUE, 
			RefTargetHandle hTarg = NULL);
//@} end group

//! \name Enumeration Methods
//@{ 
		//! \brief Enumerate auxiliary files (e.g. bitmaps)
		/*!  Enumerate auxiliary files (e.g. bitmaps). The default implementation
		just calls itself on all references and calls Animatable::EnumAuxFiles to
		pick up Custom Attributes Entities which actually need to load aux files
		must implement this, possibly calling ReferenceMaker::EnumAuxFiles also 
		to recurse. If you don't call ReferenceMaker::EnumAuxFiles call 
		Animatable::EnumAuxFiles. 
		\param  assetEnum - The Callback object that gets called on all
		Auxiliary files.
		\param  flags - Flags.
		*/ 
		CoreExport virtual void EnumAuxFiles(AssetEnumCallback& assetEnum, DWORD flags);

		//! \brief The default save enumeration.
		/*! This method is used internally. */
		CoreExport virtual void SaveEnum(SaveEnumProc& sep, BOOL isNodeCall = 0);

		//! \brief Used to specify reference slot remapping during scene file save.
		/*! Plugins that want to add, remove, or change the references held by the 
		ReferenceMaker as stored to the scene file implement this method. The method 
		implementation would modify the stored reference hierarchy through the specified
		ReferenceSaveManager. Plugins that implement this method must call SpecifySaveReferences() 
		on their parent class after they have finished performing operations on the 
		ReferenceSaveManager. The ReferenceSaveManager modifies the save hierarchy in the order 
		that operations are performed on it, so operations on higher-indexed reference target 
		slots should occur before operations on lower-indexed reference target slots. Otherwise, 
		the indexing on the higher-indexed reference target slots may point at an incorrect slot. 
		For example, if the plugin currently had slots 1,2,3 and then inserted slot 4 at position 3 and 
		then removed slot 2, the plugin would end up with slots 1,4,3. If the order of the 
		operations was reversed, the plugin would end up with slots 1,3,4. 
		The ReferenceMaker level implementation finalizes the preparation of the ReferenceSaveManager 
		to support the scene file save reference slot remapping.
		\code
			// If using CopyParamBlock2ToParamBlock copy of values/controllers from pb2 to a new pb1
			bool Swirl::SpecifySaveReferences(ReferenceSaveManager& referenceSaveManager)
			{
				// if saving to previous version that used pb1 instead of pb2...
				DWORD saveVersion = GetSavingVersion();
				if (saveVersion != 0 && saveVersion <= MAX_RELEASE_R13)
				{
					// create the pb1 instance
					IParamBlock* paramBlock1 = CreateParameterBlock( pbdesc,swirl_num_params_ver1,1);
					DbgAssert(paramBlock1 != NULL);
					if (paramBlock1)
					{
						// copy data from the pb2 to the pb1
						int numParamsCopied = CopyParamBlock2ToParamBlock(pblock,paramBlock1,pbdesc,swirl_num_params_ver1);
						DbgAssert(numParamsCopied == swirl_num_params_ver1);
						// register the reference slot replacement
						referenceSaveManager.ReplaceReferenceSlot(PBLOCK_REF,paramBlock1);
					}
				}
				return Texmap::SpecifySaveReferences(referenceSaveManager);
			}

			// If doing manual copy of values/controllers from pb2 to a new pb1
			bool Swirl::SpecifySaveReferences(ReferenceSaveManager& referenceSaveManager)
			{
				// if saving to previous version that used pb1 instead of pb2...
				DWORD saveVersion = GetSavingVersion();
				if (saveVersion != 0 && saveVersion <= MAX_RELEASE_R13)
				{
					SuspendAll suspendAll(TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE);
					// create the pb1 instance
					IParamBlock* paramBlock1 = CreateParameterBlock( pbdesc,swirl_num_params_ver1,1);
					DbgAssert(paramBlock1);
					if (paramBlock1)
					{
						// copy over each parameter value/controller
						int color_params[] = {swirl_col1, swirl_col2};
						for (int i = 0; i < _countof(color_params); i++)
						{
							int id = color_params[i];
							Control* pb2Controller = pblock->GetController(ParamID(id));
							if (pb2Controller)
								paramBlock1->SetController(id, pb2Controller, FALSE);
							else
							{
								// Note: Swirl pb2 params are TYPE_FRGBA, PB1 params are TYPE_RGBA
								AColor pb2Color = pblock->GetAColor(id);
								Color pb1Color = pb2Color;
								paramBlock1->SetValue(id, 0, pb1Color);
							}
						}
						int float_params[] = {swirl_hs, swirl_vs, swirl_cv, swirl_vg, swirl_hg, swirl_ls, swirl_rs};
						for (int i = 0; i < _countof(float_params); i++)
						{
							int id = float_params[i];
							Control* pb2Controller = pblock->GetController(ParamID(id));
							if (pb2Controller)
								paramBlock1->SetController(id, pb2Controller, FALSE);
							else
							{
								float pb2Float = pblock->GetFloat(id);
								paramBlock1->SetValue(id, 0, pb2Float);
							}
						}
						int int_params[] = {swirl_h, swirl_lock};
						for (int i = 0; i < _countof(int_params); i++)
						{
							int id = int_params[i];
							Control* pb2Controller = pblock->GetController(ParamID(id));
							if (pb2Controller)
								paramBlock1->SetController(id, pb2Controller, FALSE);
							else
							{
								int pb2Int = pblock->GetInt(id);
								paramBlock1->SetValue(id, 0, pb2Int);
							}
						}

						// register the reference slot replacement
						referenceSaveManager.ReplaceReferenceSlot(PBLOCK_REF,paramBlock1);
					}
				}
				return Texmap::SpecifySaveReferences(referenceSaveManager);
			}

		\endcode
		\param referenceSaveManager The ReferenceSaveManager for the object
		\return true if no errors processing any add, remove, or change operations registered with the ReferenceSaveManager. 
		An error will occur if the reference slot index specified for an operation is invalid after performing any 
		previously registered operations.
		*/
		CoreExport virtual bool SpecifySaveReferences(ReferenceSaveManager& referenceSaveManager);

		//! \brief Begins an enumeration that searches back in the dependency network.
		/*! Implemented by the System. See the documentation for ReferenceTarget
		for more details. When called on instances that derive from ReferenceTarget,
		the call is passed along to the ReferenceTarget::DoEnumDependents method. Otherwise
		this method returns 0.
		\see ReferenceTarget::DoEnumDependents(DependentEnumProc* dep)
		*/
		//! \param  dep - The callback object called for each dependent.
		//! \return Return 1 to stop the enumeration and 0 to continue. The
		//! default implementation for ReferenceMaker is return 0;
		CoreExport int DoEnumDependents(DependentEnumProc* dep);

		//! \brief Method to perform an enumeration on a ReferenceTarget.
		/*! Implemented by the System. See the documentation for ReferenceTarget
		for more details.
		\param  dep - The callback object called for each dependent.
		\return Return 1 to stop the enumeration and 0 to continue.
		\see ReferenceTarget::DoEnumDependentsImpl(DependentEnumProc* dep)
		*/
		CoreExport virtual int DoEnumDependentsImpl(DependentEnumProc* dep);

		//! \brief This method provides a general purpose reference enumerator.
		/*! This method provides a general purpose reference enumerator that calls 
		RefEnumProc::proc() on each element in a reference hierarchy. This function 
		walks down the reference hierarchy, recursively calling RefEnumProc::proc() 
		on the references held by a reference, This function ensures that 
		RefEnumProc::proc() is called only once on each reference. Processing each 
		reference only once improves efficiency, and prevents potential infinite 
		recursive loops when processing indirect references. To enumerate the up 
		the reference hierarchy, see ReferenceTarget::DoEnumDependents.
		\param  proc - The callback object whose proc() method is called for each element.
		\param  includeCustAttribs - Added in 3ds Max 6 SDK. Defaults to true. 
		With includeCustAttribs set to true, RefEnumProc::proc will be called for the 
		custom attributes applied to the reference maker.
		\param  includeIndirectRefs - Added in 3ds Max 9 SDK. Defaults to true. 
		With includeIndirectRefs set to true, RefEnumProc::proc will be called for the 
		indirect references held by the reference maker.
		\param  includeNonPersistentRefs - Added in 3ds Max 9 SDK. Defaults to true. 
		With includeNonPersistentRefs set to false, RefEnumProc::proc will not be called 
		on direct references from the reference	maker unless IsRealDependency or 
		ShouldPersistWeakRef returns true for that reference; and the proc method will 
		not be called on indirect references from the reference maker unless 
		ShouldPersistIndirectRef returns true for that indirect reference.
		\param  preventDuplicatesViaFlag - Added in 3ds Max 9 SDK. Defaults to true. 
		With preventDuplicatesViaFlag set to true, processing of a duplicate reference 
		is detected by requesting and clearing a flag bit on all animatables, and then 
		testing/setting the flag bit prior to calling the proc on each reference maker. 
		When set to false, a list of reference makers visited is maintained. If you 
		expect to enumerate only a small number	of references, this argument should be 
		set to false. Note however that you do not know what references are held by the 
		references you may enumerate into, so in most cases this argument should be true.
		\return Returns false if the enumeration was terminated because the RefEnumProc's 
		proc returned REF_ENUM_HALT, true otherwise
		\see Class RefEnumProc
		\see ReferenceMaker::IsRealDependency(ReferenceTarget *rtarg)
		\see ReferenceMaker::ShouldPersistWeakRef(RefTargetHandle rtarg)
		\see IIndirectReferenceMaker::ShouldPersistIndirectRef(RefTargetHandle rtarg)
		\see ReferenceTarget::DoEnumDependents(DependentEnumProc* dep)
		*/
		CoreExport bool EnumRefHierarchy(
			RefEnumProc &proc, 
			bool includeCustAttribs = true, 
			bool includeIndirectRefs = true, 
			bool includeNonPersistentRefs = true, 
			bool preventDuplicatesViaFlag = true);
//@}

//! \name Finding and Checking Reference Targets
//@{

		//! \brief Get the index of the ReferenceTarget
		/*! Implemented by the System. This method returns the virtual array 
		index of the reference target passed.
		\param  rtarg - The reference target to find the index of.
		\return The virtual array index of the reference target to find. If the 
		reference target is not found, -1 is returned.
		*/
		CoreExport int FindRef(RefTargetHandle rtarg);

		//! \brief Tells whether it is a ReferenceTarget
		/*! This function differentiates things sub classed from ReferenceMaker 
		from subclasses of ReferenceTarget. The implementation of this method 
		(in ReferenceMaker) returns FALSE and its implementation in ReferenceTarget 
		returns TRUE. This can be useful when tracing back up the reference hierarchy, 
		to know when you run into something that was sub classed directly off of 
		ReferenceMaker, and hence to stop the traversal at that point.
		\return Default of FALSE.
		*/
		virtual BOOL IsRefTarget() { return FALSE; }

		//! \brief Tells whether it is a ReferenceMaker
		/*! This function differentiates things sub classed from Animatable 
		from subclasses of ReferenceMaker. The implementation of this method 
		(in Animatable) returns FALSE and its implementation in ReferenceMaker 
		returns TRUE.
		\return Returns TRUE.
		*/
		virtual BOOL IsRefMaker() { return TRUE; }

		//! \brief Returns whether this is a "real" (strong) dependency or not.
		/*! Used Internally. When a reference target's last "real" (strong) 
		reference is deleted the target is deleted. Any leftover "non-real" 
		(weak) reference makers will receive a \ref REFMSG_TARGET_DELETED 
		notification. This method returns TRUE if the reference dependency 
		is "real" (strong). Otherwise it returns FALSE. Certain references 
		are not considered "real" (strong) dependencies. For instance, 
		internally there are certain reference makers such as the object that
		handles editing key information in the motion branch. This object 
		implements this method to return FALSE because it is not a "real" 
		strong) reference dependency. It's just needed while the editing is 
		taking place. Plugin developers don't need to concern themselves 
		with this method because it is used internally.
		\param  rtarg - A pointer to the reference target.
		\return TRUE if the reference dependency is "real". Otherwise it returns
		FALSE. Default implementation is TRUE.
		*/
		virtual BOOL IsRealDependency(ReferenceTarget *rtarg) { UNUSED_PARAM(rtarg); return TRUE;}

		//! \brief Specifies whether a weak reference is to be persisted on a 
		//! partial load or save. 
		/*! This method specifies the partial load/save behavior of a weak 
		reference. This method will only be called if IsRealDependency returns 
		FALSE. If this method returns true, and this ref maker is loaded/saved, 
		the weak reference will be forced to be loaded/saved. If false, the 
		reference will not be forced to be loaded/saved, but will be hooked back
		up if it is loaded.

		The default implementation is to return FALSE. Otherwise, it is possible
		that on a partial load that this reference maker would hold the only 
		reference to the target. If something else temporarily referenced the 
		target, then the target will be deleted when that reference is dropped. 
		From the user's perspective, this is a randomly occurring event.

		Typical cases where an implementation of this method would return TRUE is
		when post load callbacks are used to check and process the references, 
		checking for things like owner-less parameter blocks.

		\param rtarg - The weak reference. 
		\returns Whether to force the load/save of the weak reference if this 
		reference maker is saved.
		*/
		virtual BOOL ShouldPersistWeakRef(RefTargetHandle rtarg) { UNUSED_PARAM(rtarg); return FALSE; }
//@} end group

//! \name Methods introduced in 3ds Max 4.2
//! Methods inherited from Animatable. If your plugin utilizes this new 
//! mechanism, be sure that your clients are aware that they must run 
//! your plugin with 3ds max version 4.2 or higher.

//@{
		//! \brief Returns a pointer to the interface
		/*! 
			\param  id - The ID of the interface
		*/
		CoreExport virtual void* GetInterface(ULONG id);

		//! \brief Returns a pointer to the Base Interface.
		/*! Returns a pointer to the Base Interface for the interface ID passed.
			\param  id - The unique ID of the interface to get
		*/
		CoreExport virtual BaseInterface* GetInterface(Interface_ID id);
//@}

	protected:
//! \name Internal Methods
//! \brief Implemented by the system. These method are used internally.
//@{	
		void BlockEval()	{   SetAFlag(A_EVALUATING); }
		void UnblockEval()	{ ClearAFlag(A_EVALUATING); }		
		int Evaluating()	{ return TestAFlag(A_EVALUATING); }		
		
		CoreExport RefResult StdNotifyRefChanged(
			Interval changeInt, 
			RefTargetHandle hTarget, 
			PartID partID, 
			RefMessage message, 
			BOOL propagate = TRUE);
//@}

//! \name Dependent Notification
//! Methods used to respond to notifications 
//@{
		//! \brief Receives and responds to messages
		/*! A plugin which makes references must implement a method to receive 
			and respond to messages broadcast by its dependents. This is done by
			implementing NotifyRefChanged().
			The plugin developer usually implements this method as a switch 
			statement where each case is one of the messages the plugin needs to
			respond to.
			The Method StdNotifyRefChanged calls this, which can change the 
			partID to new value. If it doesn't depend on the particular message&
			partID, it should return REF_DONTCARE.
		\li For developer that need to update a dialog box with data about 
			an object you reference note the following related to this method: 
			This method may be called many times. For instance, say you have a 
			dialog box that displays data about an object you reference. This 
			method will get called many time during the drag operations on that 
			object. If you updated the display every time you'd wind up with a 
			lot of 'flicker' in the dialog box. Rather than updating the dialog 
			box each time, you should just invalidate the window in response to 
			the NotifyRefChanged() call. Then, as the user drags the mouse your 
			window will still receive paint messages. If the scene is complex 
			the user may have to pause (but not let up on the mouse) to allow 
			the paint message to go through since they have a low priority. This
			is the way many windows in 3ds Max work.

		\param  changeInt - This is the interval of time over which the 
			message is active. Currently, all plug-ins will receive FOREVER for 
			this interval.
		\param  hTarget - This is the handle of the reference target the 
			message was sent by. The reference maker uses this handle to know 
			specifically which reference target sent the message.
		\param  partID - This contains information specific to the message 
			passed in. Some messages don't use the partID at all. See the 
			section List of Reference Messages for more information about the 
			meaning of the partID for some common messages.
		\param  message - The message parameters passed into this method is 
			the specific message which needs to be handled.
		\return The return value from this method is of type RefResult. This is 
			usually REF_SUCCEED indicating the message was processed. Sometimes,
			the return value may be REF_STOP. This return value is used to stop 
			the message from being propagated to the dependents of the item.
		*/
			virtual RefResult NotifyRefChanged(
				Interval changeInt, 
				RefTargetHandle hTarget, 
				PartID& partID,  
				RefMessage message) = 0;
//@} end group

	private:
//! \name Internal Methods
//! \brief Implemented by the system. These method are used internally.
//@{	
		void CheckForNonNullRefsOnInitialization(int ignoreRef = -1);		

		//! \brief Validates a given reference link between this reference maker and its reference target
		/* \param which - the index of the reference link to verify
		\return - Returns true if either the reference is NULL or correctly set up.
		Returns false if the reference target does not know of this reference maker.
		*/
		bool ValidateExistingReference(int which);		

		//! \brief Implemented by the system. Used internally.
		bool EnumRefHierarchyImpl(
			RefEnumProc &proc, 
			bool includeCustAttribs, 
			bool includeIndirectRefs, 
			bool includeNonPersistentRefs);

		//! \brief Registers a reference with this refmaker when the reference link is established
		void AddReference(AnimHandle hTarget);
		//! \brief Unregisters a reference with this refmaker when the reference link is destroyed
		void RemoveReference(AnimHandle hTarget);

//@}

		class ReferenceMakerImplData;
		const MaxSDK::Util::AutoPtr<ReferenceMakerImplData> mImplData;
};

#pragma warning(push)
#pragma warning(disable:4239)

//! \brief A scene entity that is being owned and listened to by other scene entities.
/*! ReferenceTarget is the base class for scene entities that allow to be referenced by 
	instances of class ReferenceMaker. Most plugins are derived from this class. 
	
	This class exposes methods for sending notification messages to its dependent 
	ReferenceMaker objects, enumerating their dependents, etc.

	Note that RefTargetHandle is a typename for pointers to an object of type ReferenceTarget.
*/
class ReferenceTarget : public ReferenceMaker {
	friend class DependentIterator;
	friend class DeleteRefRestore;
	friend class MakeRefRestore;
	friend class ReferenceMaker;
	class RefList;

	//! \name Construction, destruction of instances
	//@{
	protected:
		//! \brief Destructor
		/*! Instances of class ReferenceTarget and classes derived from it should be deleted by 
		calling ReferenceTarget::MaybeAutoDelete() or ReferenceMaker::DeleteMe() on 
		them, rather then calling the delete operator. This will ensure that the 
		object drops all references made to it and the references it makes to other objects.
		*/
		CoreExport virtual ~ReferenceTarget() = 0; 

	public:
		//! \brief Constructor
		CoreExport ReferenceTarget();

		//! \brief Deletes the object when it has no more real dependents.
		/*! This function is called by 3ds Max in order to delete a plugin object 
		when its last real dependent has been deleted. It also allows derived classes
		to control the lifetime of their instances.
		The default implementation of this method will drop the references this 
		object makes to other objects, and if the undo system is engaged (holding) 
		it will ensure that the deletion is undo/redo-able.
		Override this method only if you don't want instances of your plugin to be 
		automatically deleted when the last reference to them is deleted. 
		Most subclasses of class ReferenceTarget will not need to override this method. 
		Plugins should call ReferenceTarget::MaybeAutoDelete() or ReferenceMaker::DeleteMe() 
		to delete instances of plugin objects. For more information on how 3ds Max 
		deletes reference targets, see ReferenceTarget::MaybeAutoDelete()
		\return Default implementation always returns REF_SUCCEED. Overwrites should
		also always return REF_SUCCEED.
		\note Classes that overwrite this method to prevent deletion of their instances, 
		should be deleted by calling ReferenceMaker::DeleteMe() on them.
		*/
		CoreExport virtual RefResult AutoDelete();

		//! \brief Deletes the object when it has no more real dependents.
		/*! The system calls this method to check if it can delete objects that had 
		a reference to them deleted (dropped). If there are no more real dependents on 
		this object, this function will ask the object to auto-delete itself. 
		See ReferenceTarget::AutoDelete() for more information on this.
		Note that "weak" references will not prevent the deletion of the object. 
		Plugins should also call this method when they need to delete plugin instances. 
		See ReferenceTarget::AutoDelete() for more information on deleting plugin instances.
		\return REF_SUCCEED if this object was deleted successfully, or REF_FAIL if 
		the object has outstanding real dependents to it and as a result it cannot be deleted.
		\note Objects can prevent their own deletion by overwriting ReferenceTarget::AutoDelete().
		Examples of objects that may want to do this are singletons such as class MtlLib.
		*/
		CoreExport RefResult MaybeAutoDelete();
//@}

		CoreExport virtual void GetClassName(MSTR& s);  
		CoreExport virtual SClass_ID SuperClassID();

//! \name Testing for Class / Cyclic References/ Dependencies
//@{
		//! \brief Checks if this is a ReferenceTarget
		/*! This function differentiates things subclassed from ReferenceMaker 
		from subclasses of ReferenceTarget.
		\return Returns TRUE.
		*/
		virtual BOOL IsRefTarget() { return TRUE; }

		//! \brief Tests for a cyclical reference.
		/*! Implemented by the System.
		This method may be called to test for cyclical references.
		\param  refInterval - This interval is reserved for future use. 
		Currently any plugin should specify FOREVER for this interval.
		\param  hmaker - The reference maker performing the loop test.
		\return REF_SUCCEED if a cyclic reference would be created; otherwise REF_FAIL.
		*/
		CoreExport RefResult TestForLoop( Interval refInterval, RefMakerHandle hmaker);

		//! \brief Checks if a ReferenceTarget has references.
		/*! Implemented by the System. 
		\return Returns 1 if the reference target has items that reference it, 
		otherwise 0.
		*/
		CoreExport BOOL HasDependents();

		//! \brief Checks if this has Real (Strong) Dependents
		/*! This function goes through it's list of dependents or ReferenceMakers
		and queries the results of their IsRealDependency Functions.
		\return TRUE if any one of them are, FALSE otherwise.
		*/
		CoreExport BOOL HasRealDependents();  // not counting tree view

		//! \brief Starts Dependency Test
		/*! To see if this reference target depends on something:
		\li first call BeginDependencyTest()
		\li then call NotifyDependents() on the thing with the \ref REFMSG_TEST_DEPENDENCY
		If EndDependencyTest() returns TRUE this target is dependent on the thing.
		\return TRUE if this target is dependent on the thing.
		*/
		void BeginDependencyTest() { ClearAFlag(A_DEPENDENCY_TEST); }

		//! \brief Ends Dependency Test
		/*! To see if this reference target depends on something:
		\li first call BeginDependencyTest()
		\li then call NotifyDependents() on the thing with the \ref REFMSG_TEST_DEPENDENCY
		If EndDependencyTest() returns TRUE this target is dependent on the thing.
		\return TRUE if this target is dependent on the thing.
		*/
		BOOL EndDependencyTest() { return TestAFlag(A_DEPENDENCY_TEST); }
//@}

//! \name Adding / Deleting / Transferring References
//@{
		//! \brief Called after a reference is made to a target.
		/*! This is called after a reference is made to this. If the target (this)
		needs to know that a reference to made to it, the target (this) can override this function.
		\param  rm - The ReferenceMaker creating the reference.
		*/
		virtual void RefAdded(RefMakerHandle rm) { UNUSED_PARAM(rm); }

		//! \brief Called after a reference is made to a target because of undo 
		//! or redo.
		/*! This method is available in release 2.0 and later only.
		Called when reference is added because of and undo or a redo. 
		Otherwise	it is similar to RefAdded.
		\param  rm - The ReferenceMaker creating the reference.
		*/
		virtual void RefAddedUndoRedo(RefMakerHandle rm) { UNUSED_PARAM(rm); }

		// This is called after deleting a reference to a ref target,
		// in the case that the target was not deleted. If target needs
		// to know, it should override this method.
		//! \brief Called after a references to this is deleted.
		/*! This is called after deleting a reference to a reference target, 
		in the case that the target was not deleted. When the last strong 
		reference to a ReferenceTarget is removed, the ReferenceTarget is deleted,
		the destructor is called, and the memory cleared.
		*/
		virtual void RefDeleted() {}

		//! \brief Called after a references to this is deleted because of undo 
		//! or redo.
		/*! This is called after deleting a reference to a reference target, 
		in the case that the target was not deleted. When the last strong 
		reference to a ReferenceTarget is removed, the ReferenceTarget is deleted,
		the destructor is called, and the memory cleared.
		*/
		virtual void RefDeletedUndoRedo() {}
		
		//! \brief Deletes all references to this ReferenceTarget.
		/*! implemented by the System. Deletes all references to this ReferenceTarget.
		\return always returns REF_SUCCEED.
		*/
		CoreExport RefResult DeleteAllRefsToMe();

		//! \brief Transfers all the references from oldTarget to this 
		/*! Implemented by the System. This method is used to transfer all the 
		references from oldTarget to this reference target.
			\param  oldTarget - The previous reference target.
			\param  delOld - If this is TRUE the previous reference target is deleted.
			\return Always returns REF_SUCCEED.
		*/
		CoreExport RefResult TransferReferences(
			RefTargetHandle oldTarget, 
			BOOL delOld = FALSE);
//@}	

//! \name Dependent Enumeration
//@{		
		// 
		//! \brief Begins an enumeration that searches back in the dependency network.
		/*! Implemented by the System. This method is called to initiate an enumeration of all the 
		references to the ReferenceTarget. This method prepares the DependentEnumProc for enumeration, 
		and calls DoEnumDependentsImpl on the ReferenceTarget.
		To enumerate the down the reference hierarchy, see EnumRefHierarchy.
			\param  dep - The callback object called for each dependent.
			\return Return 1 to stop the enumeration and 0 to continue.
			\see EnumRefHierarchy
		*/
		CoreExport int DoEnumDependents(DependentEnumProc* dep);	
//@}

//@{ \name Cloning
		//! \brief This method is used by 3ds Max to clone an object 
		/*! \sa CloneRefHierarchy(), class RemapDir
		This method is called by 3ds Max to have the plugin clone 
		itself. The plug-in's implementation of this method should copy both 
		the data structure and all the data residing in the data structure of 
		this reference target. The plugin should clone all its references as well. 
		Also, the plug-in's implementation of this method must call BaseClone().
		In order for classes derived from this class to clone cleanly, the
		Clone method should just create the new instance, and then call an
		implementation of BaseClone that clones the references and copies any
		other necessary data. For example:
		\code 
			class MyDerivedPlugin
				: public MyBasePlugin
			{
				const int MY_REFERENCE = 1;

				ReferenceTarget* Clone(RemapDir& remap)
				{
					ReferenceTarget* result = new MyDerivedPlugin();
					BaseClone(this, result, remap);
					return result;
				}

				void BaseClone(ReferenceTarget* from, ReferenceTarget* to, RemapDir& remap)
				{
					if (!to || !from || from == to)
						return;    
					MyBasePlugin::BaseClone(from, to, remap);
					to->ReplaceReference(MY_REFERENCE, remap->CloneRef(from->GetReference(MY_REFERENCE)));
				}
			};
		\endcode

		This method should not be directly called by plug-ins. Instead, either RemapDir::CloneRef()
		or CloneRefHierachy() should be used to perform cloning. These methods
		ensure that the mapping from the original object to the clone is added to
		the RemapDir used for cloning, which may be used during backpatch operations
		\note See the remarks in method BaseClone() below.
		\param  remap - A RemapDir instance used for remapping references during a Clone.
		\return A pointer to the cloned item. */                    
		CoreExport virtual RefTargetHandle Clone(RemapDir& remap);

		//! \brief This method copies base class data from an object to its clone.
		/*! This method is available in release 4.0 and later only. Virtual method.
		\note All plugins that implement a Clone() method have to call this BaseClone() 
		method from that Clone() method with the old and the new object as parameters. The 
		ordering in regards to when this method is called is unimportant, however
		this method must, of course, be called after the cloned object is created. 
		This method allows base classes to copy their data into a new object 
		created by the clone operation. As described in the Clone method, the Clone method 
		should just create a new instance and then call the BaseClone method. The BaseClone 
		method should then clones any references and sets any other necessary data. This 
		allows classes that derive from this class to clone cleanly.  See the Clone method
		documentation for a code example.
		All overrides of BaseClone() must call the base class implementation. The base class 
		implementation copies the CustAttrib objects into the newly created object.
		\param  from - Points to the old object to clone.
		\param  to - Points to the new object created.
		\param  remap - This class is used for remapping references during a Clone.
		*/
		CoreExport virtual void BaseClone(
			ReferenceTarget *from, 
			ReferenceTarget *to,
			RemapDir &remap);
		//@}
		
//! \name Dependent Notification
//@{
		//! \brief Notify all dependent RefMakers concerned with the message 
		/*! Implemented by the System.
		This method broadcasts the message specified by the message parameter
		to all the items which reference this item.
		Note the following on how reference messages propagate (that is, travel 
		to the dependents):
		- When a plugin sends a message via NotifyDependents(), the message
			-# Propagates to ALL the items that reference it. 
			-# And also to all the items which reference those items. 
			-# And so on. 
		- The only exceptions to this are as follows:
		-# The propagate parameter passed is FALSE. In that case the message 
			only goes to the immediate dependents.
		-# If the SClass_ID sclass = NOTIFY_ALL parameter limits the dependents
			to a certain specified Super Class.
		-# If one of the items that references the plugin, processes the message
			inside its NotifyRefChanged() and returns REF_STOP instead of 
			REF_SUCCEED. In this case, the message is not further propagated.

		Also, whenever a message propagates, the hTarget parameter received in 
		NotifyRefChanged() is reset to the this pointer of the immediate dependent
		(not the originator) who propagates the message.

		\param  changeInt - Currently all plug-ins must pass FOREVER for this
			interval. This indicates the interval of time over which the change 
			reported by the message is in effect.
		\param  partID - This parameter is used to pass message specific 
			information to the items which will receive the message. See the 
			ReferenceMaker::NotifiyRefChanged() method for more details.
		\param  message - The message to broadcast to all dependents. See
			the ReferenceMaker::NotifiyRefChanged() method for more details.
		\param  sclass - This parameter defaults to NOTIFY_ALL. If this 
			value is passed to NotifyDependents() all dependents will be notified.
			Other super class values may be passed to only send the message to 
			certain items whose SuperClassID matches the one passed.
		\param  propagate - This parameter defaults to TRUE. This indicates 
			that the message should be sent to all 'nested' dependencies. If 
			passed as FALSE, this parameter indicates the message should only be
			sent to first level dependents. Normally this should be left to 
			default to TRUE.
		\param  hTarg - This parameter must always default to NULL.
		\return This method always returns REF_SUCCEED.
		*/
		CoreExport virtual RefResult NotifyDependents(
			Interval changeInt, 
			PartID partID, 
			RefMessage message, 
			SClass_ID sclass = NOTIFY_ALL,
			BOOL propagate = TRUE, 
			RefTargetHandle hTarg = NULL);
 
		//! \brief This sends the \ref REFMSG_FLAGDEPENDENTS message up the pipeline.
		/*! This sends the \ref REFMSG_FLAGDEPENDENTS message up the pipeline. There 
		are two reasons to flag dependents:
		-# To put the node in the FG plane. (\ref PART_PUT_IN_FG)
		-# To set the node's mesh color to green to indicate it is a dependent. 
		(\ref PART_SHOW_DEPENDENCIES). 
		If the \ref PART_SHOWDEP_ON bit is set, the dependency display is turned on, 
		otherwise it is turned off.
		\param  t - Currently all plug-ins must pass FOREVER for this interval.
		\param  which - The part id that is sent with \ref REFMSG_FLAGDEPENDENTS 
		*/
		void FlagDependents( TimeValue t, PartID which=PART_PUT_IN_FG ) { 
			NotifyDependents( Interval(t,t), which,	REFMSG_FLAGDEPENDENTS );
		}

		//! \brief This method is called to flag dependents into the FG.
		/*! This method is called to flag dependents into the FG. 
		(Note that the above method is obsolete)
		The default implementation just sends out the notification 
		\ref REFMSG_FLAGDEPENDENTS with \ref PART_PUT_IN_FG as the partID. In particular, 
		a slave controller could override this method and call its master's 
		version of this method
		*/
		virtual void NotifyForeground(TimeValue t) {
			NotifyDependents(Interval(t,t),PART_PUT_IN_FG,REFMSG_FLAGDEPENDENTS);
		}

		//! \brief Sends messages to ReferenceTargets
		/*! This method is available in release 4.0 and later only.
		Used by a ReferenceMaker to send 'reverse' notification messages to its 
		ReferenceTargets, or to this ReferenceTarget.
		\param  message - The message sent to the reference target.
		\param  hMaker - The ReferenceMaker sending the message.
		*/  
		virtual void NotifyTarget(int message, ReferenceMaker* hMaker) { UNUSED_PARAM(message); UNUSED_PARAM(hMaker); }
//@}

//! \name Methods introduced in 3DS Max version 4.2
//@{	
		//! \brief Inherited from Animatable
		/*! Returns a pointer to the interface.
			\param  id - The id of the interface.
			\return A Pointer to the Interface
		*/
		CoreExport virtual void* GetInterface(ULONG id);

		//! \brief Inherited from Animatable
		/*! Returns a pointer to the Base Interface for the interface ID passed.
		\param  id - The unique ID of the interface to get
		\return A Pointer to the Interface
		*/
		CoreExport virtual BaseInterface* GetInterface(Interface_ID id);
//@}

	protected:
		// 
		//! \brief Method to perform an enumeration on a ReferenceTarget.
		/*! Implemented by the System. This method is initially called by DoEnumDependents. 
		This method allows a ReferenceTarget to call the given callback object's proc 
		on itself, and then optionally enumerate all references to it. How to continue 
		enumeration is based on the return value from the callback object's proc. 
		All ReferenceTargets have a list of back pointers to entities that directly 
		reference it. The default implementation of this method first checks to see 
		if this ReferenceTarget was previously processed as part of the enumeration, 
		and if so immediately returns 0. If the ReferenceTarget was not previously 
		processed, it calls the callback object's proc on itself and then, dependent 
		on the return value from the callback object's proc, enumerates those back 
		pointers calling DoEnumDependentsImpl once per dependent.
		Normally you will not need to override this method. If you do override this 
		method, you should use DependentEnumProc::CheckIfAndSetAsVisited to process 
		your instance and its dependents only if it was not previously processed. 
		\param  dep - The callback object called for each dependent.
		\return Return 1 to stop the enumeration and 0 to continue.
		\see DependentEnumProc::CheckIfAndSetAsVisited(ReferenceMaker *rmaker)
		\code
		int RefTargMonitorRefMaker::DoEnumDependentsImpl(DependentEnumProc* dep) {
			// If already processed this reftarg once, no need to process it or its dependents again
			if (dep->CheckIfAndSetAsVisited(this)) 
				return 0;
			int res = dep->proc(this);
			if (res == DEP_ENUM_SKIP)
				return 0;
			else if (res)
				return 1;
			return mOwner.ProcessEnumDependents(dep);
		}
		\endcode
		*/
		CoreExport virtual int DoEnumDependentsImpl(DependentEnumProc* dep);

	private:
		// Redeclared as private because it should not be called directly by plugins
		using ReferenceMaker::NotifyRefChanged;

		//! \brief Used to create references.
		/*! 
		Method used internally for creating References. ReplaceReference calls this method. 
		\param  refInterval - Currently must always pass FOREVER for this interval.
		\param  hmaker - The handle to the ReferenceMaker.
		\param  whichRef - which reference to set. If -1, the reference link is not 
		created and the method returns REF_FAIL.
		\return Returns REF_SUCCEED if successful. */ 
		CoreExport RefResult MakeReference(
			Interval refInterval,
			RefMakerHandle hmaker,
			int whichRef = -1);

	private:
		//! \brief Reference list link-node
		/*! A linked list of these objects constitute the list of ReferenceMakers that a
		ReferenceTarget keeps internally. These RefListItems are not accessed directly
		by the reference target, but are used in the RefList class. */
		class RefListItem: public MaxSDK::Util::Noncopyable 
		{
		public:
			ReferenceMaker* mMaker;
			RefListItem* mNext;
			//! Constructor
			RefListItem( ReferenceMaker* maker, RefListItem *list );
			~RefListItem();
		};

		//! \brief Stores the list of dependents for a ReferenceTargets, i.e. the list of references to a ReferenceTarget
		/*! Each Reference target uses one object of this class to store it's dependents
		(reference makers). Each item in the list is pointed to by a RefListItem which
		are associated in a linked list. */
		class RefList: public MaxSDK::Util::Noncopyable 
		{
			//! \brief Helper class used internally
			friend class ReferenceTarget;
			friend class DependentIterator;
		public:
			//! \brief Constructor
			RefList();

			//! \brief Destructor
			~RefList();

			//! \brief Gets the first item in the reference list
			RefListItem* FirstItem() const;

			//! \brief Deletes the specified item from the list.
			/*! \param  maker - The item to delete.
			\param  eval - If nonzero then when inside of NotifyDependents(),
			just set maker to NULL.   
			\return If the item was deleted REF_SUCCEED is returned; 
			otherwise REF_INVALID is returned. */
			RefResult DeleteItem(ReferenceMaker* maker, int eval);

			//! \brief Adds an item to the list by creating a RefListItem pointer.
			/*! The new	pointer is placed at the start of the list.
			\param  maker - Handle to the ReferenceMaker
			\return Always returns REF_SUCCEED. */
			RefResult AddItem(ReferenceMaker* maker);

			//! \brief Returns true if the reflist is empty
			bool Empty() const;

		private: // methods
			//! \brief Sets the owner of this reference list.
			void SetOwner(ReferenceTarget& owner);

			//! \brief Removes list items that have a NULL ReferenceMaker pointer
			void RemoveEmptyListItems();

			//! \brief Deletes the references the ReferenceMakers in this list have to 
			// the owner of this list.
			bool CleanupReferences();

		private: // data members
			//! \brief Head of the reference list (RefList)
			RefListItem* mFirst;	
			//! \brief The owner of the RefList
			AnimHandle mOwnerRefTarget;
			//! \brief When true, the RefList contains at least one item with a NULL ReferenceMaker
			bool mHasEmptyListItems;
		};

		//! \brief The list of dependents that reference this ReferenceTarget.
		RefList* mRefs;
};

#pragma warning(pop)

// Forward declaration
namespace MaxSDK
{
	class SingleWeakRefMaker;
};

//! \brief Iterates through all direct dependents of a given ReferenceTarget.
/*! Client code can simply instantiate an object of this type with the ReferenceTarget
	instance whose direct dependents need to be iterated on. Calling method DependentIterator::Next()
	will return the next ReferenceMaker that depends on the given ReferenceTarget. 
	For instance, you can count the number of ReferenceMakers that depend on a ReferenceTarget 
	using the following code:
	\code
	int CountRefs(ReferenceTarget *rt) {
		DependentIterator di(rt);
		int nrefs = 0;
		ReferenceMaker* rm = NULL;
		while (NULL!=(rm=di.Next())) {
			nrefs++;
		}
		return nrefs;   
	}
	\endcode
	All methods of this class are implemented by the system.
	\note Adding or deleting dependents to the ReferenceTarget after the 
	DependentIterator instance has been created can invalidate the iterator
	and lead to undefined behaviour.
*/
class DependentIterator: public MaxSDK::Util::Noncopyable 
{
public:
	//! \brief Constructor
	/*! \param  rtarg - Points to the RefereceTarget whose dependents are iterated through. */
	CoreExport DependentIterator(ReferenceTarget* rtarg);
	//! \brief Destructor.
	CoreExport ~DependentIterator();
	//! \brief Returns a pointer to the next ReferenceMaker or NULL when there are no more.
	CoreExport ReferenceMaker* Next();
	//! \brief Resets the iterator object so it starts at the beginning again 
	//! with the original ReferenceTarget passed.
	CoreExport void Reset(); 
	
private:
	// No default construction allowed
	DependentIterator();

private:
	ReferenceTarget* mTarget;
	ReferenceTarget::RefListItem* mNext;
};

class DeletedRestore : public RestoreObj
{
	RefMakerHandle anim, svanim;
public:
	CoreExport DeletedRestore();
	CoreExport DeletedRestore(RefMakerHandle an);
	CoreExport ~DeletedRestore();
	CoreExport void Restore(int isUndo);
	CoreExport void Redo();
	CoreExport MSTR Description();
};

//! \name Defines enumeration return values
//! \brief Possible return values for RefEnumProc::proc()
//@{
//! Continues enumeration
#define REF_ENUM_CONTINUE	0
//! Halts enumeration
#define REF_ENUM_HALT		1
//! Skip enumeration
#define REF_ENUM_SKIP		2	
//@}

//! \brief A callback class for ReferenceMaker::EnumRefHierarchy.
/*! This is the callback object for ReferenceMaker::EnumRefHierarchy. This class's 
	proc() method is called for each element in the reference hierarchy.
	When enumerating indirect references, it is possible to enter an infinite recursive 
	loop unless references are not processed more than once. The system prevents 
	processing duplicates by either maintaining a list of all references processed, 
	or by clearing a flag bit on all references and then testing/setting the flag 
	bit as references are processed. In the ReferenceMaker::EnumRefHierarchy method, 
	this object's BeginEnumeration method is called	before starting an enumeration, 
	and EndEnumeration at the end. These methods increment and decrement an internal
	counter. When the counter is decremented to 0, the list of visited references is
	cleared. This ensures that the list of visited references is maintained during 
	recursive calls to EnumRefHierarchy, but cleared when the outer most call is 
	exited. In some cases however, it is desirable to maintain the list of visited 
	references across	calls to EnumRefHierarchy. In these cases, call this object's 
	BeginEnumeration method prior to the calls to EnumRefHierarchy, and 
	EndEnumeration when done. For example:
\code
	FreeMapsRefEnum freeEnum(this);
	for ( rmaker = <enum over some refmaker set> )
		-- list of visited references cleared after each call
		EnumRefHierarchy(rmaker, freeEnum); 

	versus

	FreeMapsRefEnum freeEnum(this);
	freeEnum.BeginEnumeration()
	for ( rmaker = <enum over some refmaker set> )
		-- list of visited references maintained across each call
		EnumRefHierarchy(rmaker, freeEnum); 
	freeEnum.EndEnumeration()
\endcode
*/
class RefEnumProc: public MaxHeapOperators
{
	friend class ReferenceTarget;
	class RefEnumProcImplData;
	const MaxSDK::Util::AutoPtr<RefEnumProcImplData> mRefEnumProcImplData;

protected:
	CoreExport RefEnumProc();
	CoreExport virtual ~RefEnumProc();

	// Hide it - no copy constructor or assignment to the public.
	RefEnumProc(const RefEnumProc&); // not implemented
	RefEnumProc& operator=(const RefEnumProc& rhs); // not implemented

	public:
	//! \brief This method is called once for each element in the reference hierarchy.
	/*! This method is called once for each element in the reference hierarchy. 
	The return value specifies whether continue processing the references, indirect 
	references, and custom attributes of the refmaker; whether to skip processing of 
	those and continue to the next element, or whether to terminate enumeration. 
	Typically the return value will be REF_ENUM_CONTINUE.
	\param  rm - A pointer to the reference maker to this item. 
	\return One of the following values:
	\li <b>REF_ENUM_CONTINUE</b>: This continues the enumeration
	\li <b>REF_ENUM_HALT</b>: This stops the enumeration.
	\li <b>REF_ENUM_SKIP</b>: Reference Makers can hold multiple References. 
	In certain instances you may not want to travel down all of the "branches". 
	By returning REF_ENUM_SKIP from this method you tell the enumerator to not 
	enumerate the current Reference Maker's references but not to halt the enumeration 
	completely.
	*/
	virtual int proc(ReferenceMaker *rm)=0;

	//! \brief This method sets and checks whether a ReferenceMaker was visited
	/*! Implemented by system. This method is used to check whether a ReferenceMaker 
	was previously visited by this callback object, and registers it as having been 
	visited if not. This method is used to ensure we call the proc only once on each 
	ReferenceMaker. 
	\param  rmaker - A pointer to the reference maker
	\return Returns true of the rmaker was previously visited, false if not.
	*/
	CoreExport virtual bool CheckIfAndSetAsVisited(ReferenceMaker *rmaker);

	//! \brief This method prepares the RefEnumProc instance for enumeration
	/*! Implemented by system. This method is used to initialize the RefEnumProc 
	instance for tracking the reference makers that have been visited. Normally, 
	you do not need to call this method as ReferenceMaker::EnumRefHierarchy calls 
	this method prior to enumerating the specified reference maker. In some cases 
	though, you may want to have RefEnumProc instance track the reference makers 
	visited over multiple calls to EnumRefHierarchy. For example, you may want to 
	call RefEnumProc::proc() on each node in a selection set, tracking the reference 
	makers that have been visited across the entire selection set. Note that each 
	call this method must be paired with a call to EndEnumeration.
	\param  preventDuplicatesViaFlag - Defaults to true. With preventDuplicatesViaFlag 
	set to true, processing of a duplicate reference is detected by requesting and 
	clearing a flag bit on all animatables, and then testing/setting the flag bit 
	prior to calling the proc on each reference maker. When set to false, a list of 
	reference makers visited is maintained. If you expect to enumerate only a small 
	number of references, this argument should be set to false. Note however that 
	you do not know what references are held by the references you may enumerate into, 
	so in most cases this argument should be true. Note that the setting specified 
	here overrides the preventDuplicatesViaFlag argument setting in the EnumRefHierarchy call.
	\see ReferenceMaker::EnumRefHierarchy
	*/
	CoreExport void BeginEnumeration(bool preventDuplicatesViaFlag = true);

	//! \brief This method finalizes the RefEnumProc instance after enumeration
	/*! Implemented by system. This method is used to finalize the RefEnumProc 
	instance after enumeration, freeing any memory or max system resources used to 
	track the reference makers that have been visited. This method should only, 
	and must be, called if you called the BeginEnumeration method.
	*/
	CoreExport void EndEnumeration();
};

//! \brief This function is used to clone a single ReferenceTarget
/*! \sa Class RemapDir, ReferenceTarget::Clone()
	Implemented by system. This function should be used by plug-ins to clone a single ReferenceTarget instead of ReferenceTarget::Clone(RemapDir*)
	Calling
	ReferenceTarget::Clone() does not add the old/new ReferenceTargets to the RemapDir passed to Clone, and any backpointers in the reference hierarchy
	under the original ReferenceTarget that points back to that ReferenceTarget will fail to backpatch correctly. This function takes care of
	setting up the RemapDir, calling Clone, adding the old/new ReferenceTargets to the RemapDir, and having the RemapDir perform backpatching.
	So instead of:

	\code
	ReplaceReference(IPBLOCK,ipblock->Clone(DefaultRemapDir()));
	\endcode

	you should say:

	\code
	ReplaceReference(IPBLOCK,CloneRefHierarchy(ipblock));
	\endcode

	If you are cloning multiple ReferenceTargets, you should be creating a RemapDir and cloning the ReferenceTargets
	using RemapDir::CloneRef. Otherwise, any common ReferenceTarget under the ReferenceTargets being cloned would be
	cloned as separate ReferenceTargets, rather than a single ReferenceTarget that is shared. So, for example:

	\code
	RemapDir *remap = NewRemapDir();
	ReplaceReference(ES_MASTER_CONTROL_REF, remap->CloneRef(from.masterCont));
	ReplaceReference(ES_SLAVE_CONTROL_REF, remap->CloneRef(from.slaveCont));
	remap->Backpatch();
	remap->DeleteThis();
	\endcode

	\param  rm - A pointer to the reference target to clone. 
	\return A pointer to the new reference target
*/
CoreExport ReferenceTarget *CloneRefHierarchy(ReferenceTarget *rm);


//! \brief This class lets you tap in to ALL reference messages in the entire
//! system. 
/*! Once registered, the NotifyRefChanged() method will be called once for every
	time NotifyRefChanged() is called on a regular ReferenceTarget effectively 
	allowing you to wire tap the entire reference network.\n
	<b>WARNING</b> This should be used with extreme care. NotifyRefChange() will
	be called MANY MANY times so it is important to do very little processing 
	within this method. This most that should probably be done is to set a dirty bit.
*/
class GlobalReferenceMaker : public MaxHeapOperators
{
public:
	virtual ~GlobalReferenceMaker() {}
	class GlobalReferenceDisableGuard;

	//! \brief Callback function to receive messages
	/*! This method is implemented to receive and respond to messages 
	broadcast by all the Dependants in the entire system.
	\param  iv - This is the interval of time over which the message 
	is active. Currently, all plugins will receive FOREVER for this 
	interval.
	\param  hTarg - This is the handle of the reference target the 
	message was sent by. The reference maker uses this handle to know 
	specifically which reference target sent the message.
	\param  partID - This contains information specific to the message
	passed in. Some messages don't use the partID at all. 
	\param  msg - The msg parameters passed into this method is the
	specific message which needs to be handled.
	\return The return value from this method is of type RefResult. This
	is usually REF_SUCCEED indicating the message was processed. Sometimes,
	the return value may be REF_STOP. This return value is used to stop 
	the message from being propagated to the dependents of the item.
	*/
	virtual RefResult NotifyRefChanged(
		Interval iv, 
		RefTargetHandle hTarg,
		PartID& partID, 
		RefMessage msg) = 0;		
};

//! \brief This class is used to temporarily disable global reference makers.
/*! Implements the guard pattern to disable global reference makers. While disabled,
	none of the reference messages are sent to the global reference makers
	that were registered through RegisterGlobalReference().
	<br>
	The class correctly handles nested disables by using a counter which is
	incremented in the constructor and decremented in the destructor.
	<br>
	An example of a case where it is useful to disable global reference makers
	is an operation which creates a lot of temporary references, such as
	temporary materials. The global reference makers may not need to be
	aware of the existence of these temporary references, so they may be disabled
	for performance reasons. */
class GlobalReferenceMaker::GlobalReferenceDisableGuard : public MaxHeapOperators
{
public:

	//! Determines whether global reference makers are currently disabled.
	//! \return true if global reference makers are disabled; false otherwise.
	CoreExport static bool IsDisabled();

	//! Constructor; disables global reference makers.
	/*! A boolean parameter may be used to conditionally have this class
	do nothing.
	\param  doDisable - When true, this class will disable the global
	reference makers. When false, this class does nothing. */
	CoreExport GlobalReferenceDisableGuard(bool doDisable = true);

	//! Destructor; re-enables global reference makers.
	/*! Global reference makers are only re-enabled if they were disabled
	in the constructor. */
	CoreExport ~GlobalReferenceDisableGuard();

private:

	//! Counter used to keep track of the number of disables that have occurred.
	static int m_disableCount;

	//! Set to true if and only if global reference makers were disabled in the
	//! constructor.
	bool m_doDisable;
};

//! \brief Registers an instance of this class whose NotifyRefChanged() method will 
//! receive the messages. 
/*! \param  maker - Points to an instance of this class whose NotifyRefChanged() 
	method will receive the messages
*/
CoreExport void RegisterGlobalReference(GlobalReferenceMaker *maker);

//! \brief Unregisters the class so it won't be called anymore
/*! \param  maker - Points to an instance of this class whose NotifyRefChanged() 
	method will no longer receive the messages
*/
CoreExport void UnRegisterGlobalReference(GlobalReferenceMaker *maker);

//! \brief Clears the specified Animatable Flag bits in a reference hierarchy
/*! This method calls ClearAFlag on the ReferenceMaker and all references from it, recursively,
with the specified mask
\param  rm -  The handle to the root ReferenceMaker to enumerate from
\param  mask The bits to turn off in the Animatable flags
*/
CoreExport void ClearAFlagInHierarchy(RefMakerHandle rm, DWORD mask);

//! \brief Clears the specified Animatable Flag bits on all Animatables
/*! This method calls ClearAFlag on all Animatables with the specified mask
\param  mask The bits to turn off in the Animatable flags
*/
CoreExport void ClearAFlagInAllAnimatables(DWORD mask);

//! \brief Checks if there is a path of references from a ReferenceMaker to a 
//! ReferenceTarget.
/*!	\param  mkr - The handle to the ReferenceMaker
	\param  targ - The handle to the ReferenceTarget
	\return TRUE if there is a path of references from the to targ. This returns
	TRUE if mkr==targ)
*/
CoreExport BOOL DependsOn(RefMakerHandle mkr, RefMakerHandle targ);

//! \brief Function to find out if we are saving an old version of a .MAX file.  
/*! \return 
	\li If this returns 0, then either we are not in a save or we are saving the current version. 
	\li If it returns non-zero, it is the 3ds Max release number being saved, such as MAX_RELEASE_R12, etc
	This function can be used in NumRefs() and GetRef() to
	make an objects references appear as they did in the old Max version.
	\see ISave::SavingVersion()
*/
CoreExport DWORD GetSavingVersion(); 

//! \brief Function used internally to maintain the SavingVersion number, which 
//! should not be called by plugins.
CoreExport DWORD SetSavingVersion(DWORD version); 

//! \brief Disable reference messages globally.
/*! <b>Beware</b> Use with extreme caution.  Used for speeding up File/Reset, but not
	recommended anywhere else. (DS 3/16/00) 
	Note: the following reference notification messages are not disabled when reference messages are globally disabled: 
	REFMSG_GET_PARAM_NAME and REFMSG_TARGET_DELETED
*/
CoreExport void DisableRefMsgs();
//! \brief Enable reference messages globally.
/*! <b>Beware:</b> Use with extreme caution.  Used for speeding up File/Reset, but not
recommended anywhere else. (DS 3/16/00) */
CoreExport void EnableRefMsgs();

//! \brief Reference Maker to single entity
/*! \sa Class ReferenceMaker, Class IRefTargContainer
	This class encapsulates the management of one single reference.
	It is entirely implemented by 3ds Max. Plugins can use SingleRefMaker as is,
	or derive from it when they need to respond to reference 
	messages other than REFMSG_TARGET_DELETED.
	\par Examples: 
		See Standard Material shaders switch maxsdk\samples\materials\stdmtl2.cpp for sample usage.\n
		See also maxsdk\samples\mesh\editablemesh.
*/
class SingleRefMaker: public ReferenceMaker {
protected:
	//! Handle to the single reference
	RefTargetHandle rtarget;
public:
	//! \brief Constructor
	CoreExport SingleRefMaker();
	//! \brief Destructor
	/*! Suspended from Undo system */
	CoreExport ~SingleRefMaker();

	//! \brief Set the Reference
	/*! Suspended from Undo system */
	CoreExport void SetRef(RefTargetHandle rt);

	//! \brief Get the Reference
	CoreExport RefTargetHandle GetRef() const;

	//! \brief By default handles \ref REFMSG_TARGET_DELETED message only
	/*! This function should be overridden by plugins that need to respond to
			reference messages.  If this is done, it is still recommended to call
			the original implementation
			\code
			NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message )
			{
				switch(message) { 
					// Handle our messages
				}
				// Handles REFMSG_TARGET_DELETED
				return SingleRefMaker::NotifyRefChanged(changeInt, hTarget, partID, message);
			}
			\endcode					
			See ReferenceMaker::NotifyRefChanged for complete documentation */
	CoreExport RefResult NotifyRefChanged(
		Interval changeInt,
		RefTargetHandle hTarget, 
		PartID& partID, 
		RefMessage message );
	CoreExport void DeleteThis();
	CoreExport	SClass_ID  SuperClassID();

	// Overrides From ReferenceMaker
	CoreExport	int NumRefs();
	CoreExport	RefTargetHandle GetReference(int i);
protected:
	CoreExport	virtual void SetReference(int i, RefTargetHandle rtarg);
public:
	CoreExport	BOOL CanTransferReference(int i);
};

//! \brief Suspends the "animate" mode in an exception safe manner
/*! The system is in "animate" mode when animation keys are being created.
Create an instance of this class to suspend the "animate" mode. 
When the instance is destructed the "animate" mode is resumed. 
It's a good idea to suspend the "animate" mode whenever new instances of a plugin 
are being created, or their parameters are modified programatically. 
Examples of when the "animate" mode is suspended include assigning controllers and
copying key frames from a controller to another, cloning objects, etc.
\note AnimateSuspend accomplishes the same thing as a call to SuspendAnimation(), 
but in a manner that is safe when exceptions are thrown.
*/
class CoreExport AnimateSuspend: public MaxHeapOperators {
private:
	BOOL animateSuspended; 
	BOOL setKeyModeSuspended;
	BOOL macroRecSuspended;
	BOOL refMsgsSuspended;
public:
	//! Constructor
	/*! Suspends the "animate" mode, and optionally some other system states
	\param  macroRecToo - If TRUE, macro recording is also suspended, 
	otherwise it's not changed. See MacroRecorder::Disable() for more information.
	\param  setKeyModeToo - If TRUE, "setkey" mode is also suspended, 
	otherwise it's not changed. See SuspendSetKeyMode() for more information.
	\param  refMsgsToo - If TRUE, passing of reference messages is also suspended, 
	otherwise it's not changed. See DisableRefMsgs() for more information.
	*/
	AnimateSuspend(
		BOOL setKeyModeToo = FALSE, 
		BOOL macroRecToo = FALSE, 
		BOOL refMsgsToo = FALSE );
	//! \brief Restores "animate" mode to its initial state.
	void Resume();
	//! \brief Destructor - restores "animate" mode to its initial state.
	~AnimateSuspend() {Resume ();}
};					  

//! \brief Suspends various system states in an exception safe manner.
/*! In certain cases, it is desirable to suspend animation, undo/redo, etc. while
your plugin executes certain operations that should not create animation keys, 
be undo-able, etc. In these cases, create an instance of this class to suspend 
the desired system states. When the instance is destructed, the states will automatically
be restored to their previous value, in a way that is safe in the presence of exceptions.
*/
class CoreExport SuspendAll: public MaxHeapOperators {
private:
	BOOL holdSuspended;
	BOOL macroRecSuspended;
	BOOL animateSuspended;
	BOOL setKeyModeSuspended;
	BOOL autoBackupSuspended;
	BOOL saveRequiredState;
	BOOL refMsgsSuspended;
	BOOL m_previousSaveRequiredState;
	BOOL m_previousAutoBackupState;
public:
	//! Constructor
	/*! Suspends all system states that are requested to be suspended
	\param  holdState - If TRUE, disables the hold system (undo/redo), 
	otherwise it's not changed. See theHold.Suspend() for more information.
	\param  macroRecorder - If TRUE, macro recording is suspended, 
	otherwise it's not changed. See MacroRecorder::Disable() for more information.
	\param  animateMode - If TRUE, creating animation keys is suspended, 
	otherwise it's not changed. See class AnimateSuspend for more information.
	\param  setKeyMode - If TRUE, "setkey" mode is suspended, 
	otherwise it's not changed. See SuspendSetKeyMode() for more information.
	\param  autoBackup - If TRUE, auto-backup is suspended, 
	otherwise it's not changed. See Interface::EnableAutoBackup() for more information.
	\param  saveRequired - If TRUE, backs up the "save required" flag, otherwise it doesn't.
	See IsSaveRequired() for more information.
	\param  refMsgs - If TRUE, passing of reference messages is suspended, 
	otherwise it's not changed. See DisableRefMsgs() for more information.
	*/
	SuspendAll(
		BOOL holdState = FALSE, 
		BOOL macroRecorder = FALSE, 
		BOOL animateMode = FALSE, 
		BOOL setKeyMode = FALSE, 
		BOOL autoBackup = FALSE, 
		BOOL saveRequired = FALSE, 
		BOOL refMsgs = FALSE );
	//! \brief Restores the suspended system states.
	void Resume();
	//! \brief Destructor - restores the suspended system states
	~SuspendAll() {Resume ();}
};               

