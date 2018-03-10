/**********************************************************************
 *<
	FILE: XTCObject.h
				  
	DESCRIPTION:  Defines the Extension Channel Object

	CREATED BY: Nikolai Sander

	HISTORY: created 3 March 2000

 *>     Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "maxapi.h"
#include "plugapi.h"
#include "iFnPub.h"
#include "channels.h"
#include "object.h"
// forward declarations
class ObjectState;


/*------------------------------------------------------------------- 
  XTCObject:  
---------------------------------------------------------------------*/

/*! \sa  Class InterfaceServer, Class Class_ID, Class ModContext, Class ObjectState, Class INode,  Class Object,  Class Modifier, Class GraphicsWindow, Class FPInterface, List of Channel Bits, Class XTCContainer, Class IXTCAccess\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This is the base class for an Extension Channel plug-in. These plug-ins are
used to allow a developer defined object to flow down the geometry pipeline.
This class provides an interface to the extension object. This is the virtual
base class that developers can derive objects from, that should be inserted
into the extension channel of the object flowing up the stack. Extension
Channels will expand the geometry pipeline by allowing one to add a custom
object to the pipeline object that can flow down the pipeline. This object will
get notified whenever something in the pipeline changes. For example, if you
want to indicate when a certain object becomes invalid for export to their game
engine, invalid skin-vertex assignments, bound patches etc. By inserting an
Extension Channel Object (XTCObject, for short) into the pipeline you can
accomplish this, by constantly checking the structure of the object and
displaying wrong faces/vertices etc. in the viewport.\n\n
You can specify which other channels it depends on using <b>DependsOn()</b>.
The extension object has callback methods that get called before and after a
modifier modifies a channel that the extension object depends on using
<b>PreChanChangedNotify()</b> and <b>PostChanChangedNotify()</b>. The extension
object can declare additional channels that it modifies using
<b>ChannelsChanged()</b>, so that it can make any changes to the mesh before
and after the modification by the modifier.\n\n
In general, the Extension Channel is a transient data structure that gets
recreated on every pipeline evaluation. So the object that adds an extension
channel to the modifier stack automatically makes it persistent. However, when
the user collapses the stack, the user might want the Extension Channel to be
preserved as well. In order to accomplish that, please refer to the
Class BaseObject and the methods
<b>NotifyPreCollapse()</b> and <b>NotifyPostCollapse()</b>. These methods will
be called by the collapse code. It will give the modifier or BaseObject, that
adds an XTC object to the stack the possibility to apply a modifier, that
inserts these XTC objects onto the stack after the collapse. Through this
mechanism, the XTC will survive a stack collapse. The Pre and Post
notifications will be called through a pipeline enumeration downstream (for
more info see Class GeomPipelineEnumProc). Developers, who are collapsing the stack
programmatically, have to call this method. In case this method is not called,
the XTC objects will by default be copied as well, since they are part of the
object in the wsCache. However, they won't survive a save/load operation. In
addition to all this, XTC objects also have the possibility to display their
data in the viewports. Any Extension Channel Object can disable the display of
the object itself and take over the entire display itself, by returning true in
the method <b>SuspendObjectDisplay()</b>.\n\n
Note: Modifiers which change the type of object that flows up the stack have to
copy the Extension Channel from the old object into the new one using
<b>CopyAdditionalChannels()</b> (e.g. the extrude modifier has to copy the XTC
from the incoming spline to the Mesh, Patch or NURBS object).\n\n
Note: Compound objects have to merge the Extension Channel of the branched
pipelines into the resulting pipeline. This is in general a simple copy of the
Extension Channel Object into the new Extension Channel. When the
CompoundObject evaluates is branches it would call
<b>CopyAdditionalChannels(os-\>obj)</b>, so that the Extension Channels of the
branches are copied over. In the <b>ConvertToType()</b> method it then has to
copy the Extension Channels from itself to the converted object using
<b>obj-\>CopyAdditionalChannels(this)</b>.\n\n
Also note that the Extension Channel itself is implemented in Class Object.
This means, that it will be available for all pipeline objects that get
implemented in 3ds Max. For additional methods related to extension objects see
the methods in Class Object -\>
<a href="class_object.html#A_GM_obj_extension_chnl">Extension Channel
Access</a>.\n\n
All methods of this class are virtual. Default implementations are shown. <br>
*/
class XTCObject : public InterfaceServer {
public:
	// Unique identifier
	/*! \remarks This method returns the unique identifier for the object. */
	virtual Class_ID ExtensionID()=0;
	/*! \remarks This method is called to create a cloned copy of the object.
	The object should create a copy of itself and return a pointer to it. */
	virtual XTCObject *Clone()=0;
	
	// Channels that the XTCObject depends on. If a modifier changes a channel, 
	// that a XTCObject depends on, its Pre- and PostChanChangedNotify
	// methods will be called
	/*! \remarks This method returns a ChannelMask which specifies the
	channels that the XTCObject depends on. If a modifier changes a channel
	that a XTCObject depends on, its <b>PreChanChangedNotify()</b> and
	<b>PostChanChangedNotify()</b> methods will be called.
	\return  See \ref ObjectChannels.
	\par Default Implementation:
	<b>{ return 0; }</b> */
	virtual ChannelMask DependsOn(){return 0;}
	
	// These are the channels, that the extension object changes in the Pre- 
	// or PostChanChangedNotify methods
	/*! \remarks	This method returns a ChannelMask which specifies the channels that the
	extension object changes in the <b>PreChanChangedNotify()</b> and
	<b>PostChanChangedNotify()</b> methods.
	\return  See \ref ObjectChannels.
	\par Default Implementation:
	<b>{ return 0; }</b> */
	virtual ChannelMask ChannelsChanged(){return 0;}

	// These are the channels, that the extension object changes in the Pre- 
	// or PostChanChangedNotify methods
	/*! \remarks	This method returns a ChannelMask which specifies the channels that the
	extension object uses in the <b>PreChanChangedNotify()</b> and
	<b>PostChanChangedNotify()</b> methods.
	\return  See \ref ObjectChannels.
	\par Default Implementation:
	<b>{ return 0; }</b> */
	virtual ChannelMask ChannelsUsed(){return 0;}
	
	// If an XTCObject wants to display itself in the viewport, it can 
	// overwrite this method
	/*! \remarks If an XTCObject wants to display itself in the viewport it
	can overwrite this method.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which the object is to be displayed.\n\n
	<b>INode* inode</b>\n\n
	Points to the node for the object.\n\n
	<b>ViewExp *vpt</b>\n\n
	Points to the viewport interface for the object.\n\n
	<b>int flags</b>\n\n
	See <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_display_flags.html">List of Display
	Flags</a>.\n\n
	<b>Object *pObj</b>\n\n
	Points to the object that the extension object is a part of.
	\return  The return value is not currently used.
	\par Default Implementation:
	<b>{ return 0; }</b> */
	#pragma warning(push)
	#pragma warning(disable:4100)
	virtual int  Display(TimeValue t, INode* inode, ViewExp *vpt, int flags,Object *pObj){return 0;};
	
	// This method will be called before a modifier is applied, that 
	// changes a channel, that the XTCObject depends on. In case the 
	// modifier is the last in the stack, bEndOfPipleine is true,
	// otherwise false

	/*! \remarks This method is called before a modifier is applied that
	changes a channel that the XTCObject depends on.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which the channel will be modified.\n\n
	<b>ModContext \&mc</b>\n\n
	The modifier context.\n\n
	<b>ObjectState* os</b>\n\n
	The objectstate of the object.\n\n
	<b>INode *node</b>\n\n
	A pointer to the node.\n\n
	<b>Modifier *mod</b>\n\n
	A pointer to the modifier being applied.\n\n
	<b>bool bEndOfPipeline</b>\n\n
	TRUE to indicate that this is the last change before the wsCache.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void PreChanChangedNotify(TimeValue t, ModContext &mc, ObjectState* os, INode *node,Modifier *mod, bool bEndOfPipeline){};

	// This method will be called after a modifier is applied, that 
	// changes a channel, that the XTC object depends on. In case the 
	// modifier is the last in the stack, bEndOfPipleine is true
	// otherwise false
	/*! \remarks This method will be called after a modifier is applied that
	changes a channel that the XTC object depends on.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which the channel will be modified.\n\n
	<b>ModContext \&mc</b>\n\n
	The modifier context.\n\n
	<b>ObjectState* os</b>\n\n
	The objectstate of the object.\n\n
	<b>INode *node</b>\n\n
	A pointer to the node.\n\n
	<b>Modifier *mod</b>\n\n
	A pointer to the modifier being applied.\n\n
	<b>bool bEndOfPipeline</b>\n\n
	TRUE to indicate that this is the last change before the wsCache.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void PostChanChangedNotify(TimeValue t, ModContext &mc, ObjectState* os, INode *node,Modifier *mod, bool bEndOfPipeline){};
	
	// If the XTC object returns true from this method, the object 
	// is not displayed in the viewport
	/*! \remarks If the XTCObject returns TRUE from this method the object is
	not displayed in the viewport; if FALSE is returned the <b>Display()</b>
	method will be called to display the object.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual BOOL SuspendObjectDisplay(){ return false; }
	/*! \remarks This method is called to delete the extension object. */
	virtual void DeleteThis()=0;
	
	// This method allows the object to enlarge its viewport rectangle,
	// if it wants to. The system will call this method for all XTCObjects 
	// when calculating the viewport rectangle; the XTCObject can enlarge the 
	// rectangle if desired
	/*! \remarks This method allows the object to enlarge its viewport
	rectangle if it wants to. The system will call this method for all
	XTCObjects when calculating the viewport rectangle; the XTCObject can
	enlarge the rectangle if desired.
	\par Parameters:
	<b>GraphicsWindow *gw</b>\n\n
	Points to the GraphicsWindow instance associated with the viewport the
	object is displayed in.\n\n
	<b>Rect \&rect</b>\n\n
	The viewport rectangle for the object which may be modified.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void MaybeEnlargeViewportRect(GraphicsWindow *gw, Rect &rect){}

	// by default the existing XTCObjects will be deleted, if a branch updates. 
	// In case the XTCObject wants to do more intelligent branching (not just 
	// deleted and add), it might want to return false to this method, so that 
	// it can later (see MergeXTCObject) copy the data from this and other 
	// branches into an existing XTCObject.
	/*! \remarks By default the existing XTCObjects will be deleted if a
	branch updates In case the XTCObject wants to do more intelligent branching
	(not simply delete and add), it can return false from this method so that
	it can later (see <b>MergeXTCObject()</b> below) copy the data from this
	and other branches into an existing XTCObject.
	\par Parameters:
	<b>Object *obFrom</b>\n\n
	Points to the source object.\n\n
	<b>Object *obTo</b>\n\n
	Points to the destination object.
	\return  Returns true if the object will be deleted; false to do more
	processing via MergeXTCObject.
	\par Default Implementation:
	<b>{ return true; }</b> */
	virtual bool RemoveXTCObjectOnMergeBranches(Object *obFrom, Object *obTo) { return true; }
	

	// The default implementation just adds the XTCObject to the to object
	// In case the XTCObject should do a more intelligent merge with already
	// existing XTCObjects in the obTo, it has to overwrite this method
	/*! \remarks The default implementation just adds the XTCObject to the to
	object. In case the XTCObject should do a more intelligent merge with
	already existing XTCObjects in the obTo, it has to overwrite this method.
	\par Parameters:
	<b>Object *obFrom</b>\n\n
	Points to the source object.\n\n
	<b>Object *obTo</b>\n\n
	Points to the destination object.\n\n
	<b>int prio</b>\n\n
	The priority to set.\n\n
	<b>int branchID</b>\n\n
	The branch identifier to set.
	\return  TRUE if successful, otherwise FALSE.
	\par Default Implementation:
	<b>{ obTo-\>AddXTCObject(this,prio,branchID); return true;}</b> */
	virtual bool MergeXTCObject(Object *obFrom, Object *obTo, int prio, int branchID) { obTo->AddXTCObject(this,prio,branchID); return true;}

	// In case a branch of a compound object is deleted the XTCObject will be asked,
	// if the XTCObject should be deleted as well. In case the XTCObject represents a
	// merge of all branches the TCObject might want to return false to this
	// method and reassign itself to another branch, so that the merged information is 
	// not lost.
	/*! \remarks In case a branch of a compound object is deleted the
	XTCObject will be asked if the XTCObject should be deleted as well. In case
	the XTCObject represents a merge of all branches the XTCObject might want
	to return false to this method and reassign itself to another branch, so
	that the merged information is not lost.
	\par Parameters:
	<b>Object *ComObj</b>\n\n
	A pointer to the compound object.\n\n
	<b>int branchID</b>\n\n
	The branch identifier to set.\n\n
	<b>bool branchWillBeReordered</b>\n\n
	TRUE if the branch should be reordered, otherwise FALSE.
	\return  TRUE if successful, otherwise FALSE.
	\par Default Implementation:
	<b>{ return true; }</b> */
	virtual bool RemoveXTCObjectOnBranchDeleted(Object *ComObj,int branchID, bool branchWillBeReordered) { return true; }
	#pragma warning(pop)
};

