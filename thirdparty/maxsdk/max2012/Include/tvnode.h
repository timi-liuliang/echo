/**********************************************************************
 *<
	FILE: tvnode.h

	DESCRIPTION: Track View Node Class

	CREATED BY: Rolf Berteig

	HISTORY: 11-14-96

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/


#pragma once
#include "maxheap.h"
#include "ref.h"
// forward declarations
class Control;

#define TVNODE_CLASS_ID	Class_ID(0x8d73b8aa, 0x90f2ee71)

// Default position for appending
#define TVNODE_APPEND -1


// TrackViewNodes can contain one or more sub nodes or controllers.
// Sub-nodes and controllers are identified by a unique ID in the
// form of a Class_ID variable. This does not necessarily have to
// be the class ID of an existing plug-in, however plug-ins may
// wish to use thier class ID for any items they add to be sure they
// are unique.
//
// The Interface class provides access to the root track view node.
// From this node, new nodes can be added. There are two defined
// sub nodes:

#define GLOBAL_VAR_TVNODE_CLASS_ID			Class_ID(0xb27e9f2a, 0x73fad370)
#define VIDEO_POST_TVNODE_CLASS_ID			Class_ID(0x482b8d30, 0xb72c8511)
#define TRACK_SELECTION_SET_MGR_CLASS_ID	Class_ID(0x77a71ca2, 0x670c632d)
#define TRACK_SELECTION_SET_CLASS_ID		Class_ID(0x6eb33def, 0x20344f6a)

// These can be retreived by calling GetNode() on the track view root
// node and passing in one of the above IDs.


// These can be registered with a TVNode to intercept reference notifications
/*! \sa  Class ITrackViewNode, Class UndoNotify.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class is the callback object for
<b>ITrackViewNode::RegisterTVNodeNotify()</b>. Developers should derive their
class from this class and implement the <b>NotifyRefChanged()</b> method. This
allows the Track %View Node to intercept reference notifications when they use
a Track %View Node.\n\n
For an example of this class in use by <b>ImageFilter</b> plug-ins see class
<b>UndoNotify</b> in <b>/MAXSDK/INCLUDE/FILTERS.H</b>. It is sub-classed from
this class and provides an implementation of <b>NotifyRefChanged()</b>.\n\n
All methods of this class are virtual.  */
class TVNodeNotify: public MaxHeapOperators {
	public:
		/*! \remarks A plug-in which makes references must implement this
		method to receive and respond to messages broadcast by its dependents.
		\par Parameters:
		<b>Interval changeInt</b>\n\n
		This is the interval of time over which the message is active.
		Currently, all plug-ins will receive <b>FOREVER</b> for this
		interval.\n\n
		<b>RefTargetHandle hTarget</b>\n\n
		This is the handle of the reference target the message was sent by. The
		reference maker uses this handle to know specifically which reference
		target sent the message.\n\n
		<b>PartID\& partID</b>\n\n
		This contains information specific to the message passed in. Some
		messages don't use the <b>partID</b> at all. See the section
		\ref Reference_Messages and \ref partids for more information about the meaning of the
		<b>partID</b> for some common messages.\n\n
		<b>RefMessage message</b>\n\n
		The <b>message</b> parameters passed into this method is the specific
		message which needs to be handled. See \ref Reference_Messages.
		\return  The return value from this method is of type <b>RefResult</b>.
		This is usually <b>REF_SUCCEED</b> indicating the message was
		processed. Sometimes, the return value may be <b>REF_STOP</b>. This
		return value is used to stop the message from being propagated to the
		dependents of the item. */
		virtual RefResult NotifyRefChanged(
			Interval changeInt, RefTargetHandle hTarget, 
			PartID& partID,  RefMessage message)=0;
	};
 
/*! \sa  Class ReferenceTarget, Class Interface, Class Control, Class ImageFilter, Class TVNodeNotify.\n\n
\par Description:
This class provides an interface to Track %View Nodes. A Track %View Node is
simpy a class that has zero or more sub-track view nodes and zero or more
sub-controllers. This is mainly used to provide a place for Global Variable
tracks (labeled "Global Tracks" in Track %View) and Video Post tracks (labelled
"Video Post" in Track %View).\n\n
The TrackViewNode sub-nodes and sub-controllers are identified by a unique ID
in the form of a Class_ID variable. This does not necessarily have to be the
Class_ID of an existing plug-in, however plug-ins may wish to use their
Class_ID for any items they add to be sure they are unique.\n\n
The Interface class provides access to the root track view node:\n\n
<b>virtual ITrackViewNode *GetTrackViewRootNode()=0;</b>\n\n
From the root track view node, new nodes may be added. There are two defined
sub nodes identified by the following <b>#defined</b> Class_IDs:\n\n
<b>#define GLOBAL_VAR_TVNODE_CLASS_ID Class_ID(0xb27e9f2a, 0x73fad370)</b>\n\n
<b>#define VIDEO_POST_TVNODE_CLASS_ID Class_ID(0x482b8d30, 0xb72c8511)</b>\n\n
These can be retreived by calling <b>GetNode()</b> on the track view root node
and passing in one of the above IDs.\n\n
All methods of this class are implemented by the system.\n\n
Note: Developers can also create their own track view node using the following
global function:\n\n
  */
class ITrackViewNode : public ReferenceTarget {
	public:
		/*! \remarks This method is used to add a track view node.
		Note: Modifying the track view node list both from a context in which the undo 
		system is On (holding) and Off (not holding) is unsafe. Since 3ds Max can turn 
		on or off the undo system as needed, adding or removing track view nodes should 
		be considered safe only when the undo system is Off.
		\par Parameters:
		<b>ITrackViewNode *node</b>\n\n
		Points to the Track %View Node to add.\n\n
		<b>MCHAR *name</b>\n\n
		The name for the node that appears in Track %View.\n\n
		<b>Class_ID cid</b>\n\n
		The Class_ID which identifies the plug-in that added the node.\n\n
		<b>int pos=TVNODE_APPEND</b>\n\n
		The position in the list of nodes where this one is added. If this
		defaults to <b>TVNODE_APPEND</b> the node is added at the end of the
		list. */
		virtual void AddNode(ITrackViewNode *node, MCHAR *name, Class_ID cid, int pos=TVNODE_APPEND)=0;
		/*! \remarks This method is used to add a track view controller.
		Note: Modifying the track view node list both from a context in which the undo 
		system is On (holding) and Off (not holding) is unsafe. Since 3ds Max can turn 
		on or off the undo system as needed, adding or removing track view nodes should 
		be considered safe only when the undo system is Off.
		\par Parameters:
		<b>Control *c</b>\n\n
		Points to the controller to add.\n\n
		<b>MCHAR *name</b>\n\n
		The name that will appear in Track %View.\n\n
		<b>Class_ID cid</b>\n\n
		The Class_ID of the plug-in that adds the controller.\n\n
		<b>int pos=TVNODE_APPEND</b>\n\n
		The position in the list where the controller is added. If this
		defaults to <b>TVNODE_APPEND</b> the controller is added at the end of
		the list. */
		virtual void AddController(Control* c, MCHAR* name, Class_ID cid, int pos=TVNODE_APPEND)=0;
		/*! \remarks A Track %View Node maintains a table that contains the
		sub-nodes and sub-controllers. This method returns the index into the
		table of the node or controller whose Class_ID is passed. If the
		Class_ID could not be found then -1 is returned.
		\par Parameters:
		<b>Class_ID cid</b>\n\n
		The Class_ID to find. */
		virtual int FindItem(Class_ID cid)=0;
		/*! \remarks A Track %View Node maintains a table that contains the
		sub-nodes and sub-controllers. This method removes the 'i-th' sub-node
		or sub-controller of the table.
		Note: Modifying the track view node list both from a context in which the undo 
		system is On (holding) and Off (not holding) is unsafe. Since 3ds Max can turn 
		on or off the undo system as needed, adding or removing track view nodes should 
		be considered safe only when the undo system is Off.
		\par Parameters:
		<b>int i</b>\n\n
		The zero based index into the table of the item to remove. */
		virtual void RemoveItem(int i)=0;
		/*! \remarks A Track %View Node maintains a table that contains the
		sub-nodes and sub-controllers. This method removes the sub-node or
		sub-controller whose Class_ID is passed from the table.
		Note: Modifying the track view node list both from a context in which the undo 
		system is On (holding) and Off (not holding) is unsafe. Since 3ds Max can turn 
		on or off the undo system as needed, adding or removing track view nodes should 
		be considered safe only when the undo system is Off.
		\par Parameters:
		<b>Class_ID cid</b>\n\n
		The Class_ID used when the node or controller was added. */
		virtual void RemoveItem(Class_ID cid)=0;
		/*! \remarks This method returns a pointer to the 'I-th'
		sub-controller.
		\par Parameters:
		<b>int i</b>\n\n
		The zero based index of the sub-controller. */
		virtual Control *GetController(int i)=0;
		/*! \remarks This method returns a pointer to the sub-controller whose
		Class_ID is passed.
		\par Parameters:
		<b>Class_ID cid</b>\n\n
		The Class_ID used when the controller was added. */
		virtual Control *GetController(Class_ID cid)=0;
		/*! \remarks This method returns a pointer to the 'i-th' sub-node.
		\par Parameters:
		<b>int i</b>\n\n
		The zero based index of the sub-node. */
		virtual ITrackViewNode *GetNode(int i)=0;
		/*! \remarks This method returns a pointer to the sub-node whose
		Class_ID is passed.
		\par Parameters:
		<b>Class_ID cid</b>\n\n
		The Class_ID used when the controller was added. */
		virtual ITrackViewNode *GetNode(Class_ID cid)=0;
		/*! \remarks This method returns the total number of sub-nodes and/or
		sub-controllers in the table. */
		virtual int NumItems()=0;
		/*! \remarks This method is used to rearrange the elements in the
		table so item <b>i1</b> is where <b>i2</b> was and <b>i2</b> is where
		<b>i1</b> was.
		Note: Modifying the track view node list both from a context in which the undo 
		system is On (holding) and Off (not holding) is unsafe. Since 3ds Max can turn 
		on or off the undo system as needed, adding or removing track view nodes should 
		be considered safe only when the undo system is Off.
		\par Parameters:
		<b>int i1</b>\n\n
		The zero based index into the table of one of the items to swap.\n\n
		<b>int i2</b>\n\n
		The zero based index into the table of the other item to swap. */
		virtual void SwapPositions(int i1, int i2)=0;
		/*! \remarks Returns the name of the 'i-th' sub-node or
		sub-controller.
		\par Parameters:
		<b>int i</b>\n\n
		The zero based index into the table of the item whose name to return.
		*/
		virtual MCHAR *GetName(int i)=0;
		/*! \remarks Sets the name of the 'i-th' sub-node or sub-controller to
		the name passed.
		\par Parameters:
		<b>int i</b>\n\n
		The zero based index into the table of the item whose name to set.\n\n
		<b>MCHAR *name</b>\n\n
		The new name for the sub-node or sub-controller. */
		virtual void SetName(int i,MCHAR *name)=0;
		/*! \remarks Registers the track view notify callback object so it
		recieves reference messages.
		\par Parameters:
		<b>TVNodeNotify *notify</b>\n\n
		Points to the callback object to register. */
		virtual void RegisterTVNodeNotify(TVNodeNotify *notify)=0;
		/*! \remarks Un-Registers the track view notify callback object.
		\par Parameters:
		<b>TVNodeNotify *notify</b>\n\n
		Points to the callback object to register. */
		virtual void UnRegisterTVNodeNotify(TVNodeNotify *notify)=0;
//watje this will prevent children from showing up in the TV
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This may be used to prevent child nodes from showing up in Track %View.
		\par Parameters:
		<b>BOOL chide</b>\n\n
		Pass TRUE to have children hidden; FALSE to have them visible.\n\n
		  */
		virtual void HideChildren(BOOL chide)=0;
	};

/*! \remarks This method is used to add a track view node.
\par Parameters:
<b>BOOL hidden=FALSE</b>\n\n
If FALSE the node is hidden; otherwise it is visible in the viewports.  */
CoreExport ITrackViewNode *CreateITrackViewNode(BOOL hidden=FALSE);

