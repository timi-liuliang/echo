/*********************************************************************
 *<
	FILE: nodeDisp.h

	DESCRIPTION: Interface for node display callbacks

	CREATED BY:	Cleve Ard

	HISTORY: Created April 3, 2000

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "baseinterface.h"
#include "strclass.h"

// forward declarations
class ViewExp;
class INode;
class Object;
class Box3;
class IPoint2;

// A callback to allow plug-ins that aren't actually objects (such as utilities)
// to control a Node's display
/*! \sa  Class InterfaceServer, Class INodeDisplayControl, Class INode, Class ViewExp, Class IPoint2.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
A callback to allow plug-ins that aren't actually objects (such as utilities)
to control a Node's display.\n\n
This class enables you to display extra information on top of a node in a
viewport. Once activated, a plug-in will control the display (on/off) of a
node's world space representation as well as add data in a viewport on a per
node basis. This approach allows you to replace the drawing code of every node
without adding modifiers on top of each of them.  */
class NodeDisplayCallback : public InterfaceServer {
	public:

		// StartDisplay is called right before the tree of nodes is drawn
      /*! \remarks This method is called just before 3ds Max draws the nodes
      in the scene.
      \par Parameters:
      <b>TimeValue t</b>\n\n
      The time at which the nodes are being drawn.\n\n
      <b>ViewExp *vpt</b>\n\n
      Points to an interface for the viewport the node is being drawn in.\n\n
      <b>int flags</b>\n\n
      These flags are used internally. */
      virtual void StartDisplay(TimeValue t, ViewExp *vpt, int flags)=0;
		// EndDisplay is called right after the tree of nodes is drawn
       /*! \remarks This method is called just after 3ds Max draws the nodes
       in the scene.
       \par Parameters:
       <b>TimeValue t</b>\n\n
       The time at which the nodes were drawn.\n\n
       <b>ViewExp *vpt</b>\n\n
       Points to an interface for the viewport the node is being drawn in.\n\n
       <b>int flags</b>\n\n
       These flags are used internally. */
       virtual void EndDisplay(TimeValue t, ViewExp *vpt, int flags)=0;
		// Display: Called for every node
		 /*! \remarks This method is called for every node to allow it display itself.
		 \par Parameters:
		 <b>TimeValue t</b>\n\n
		 The time at which the node is to be drawn.\n\n
		 <b>ViewExp *vpt</b>\n\n
		 Points to an interface for the viewport in which the node is being drawn
		 in.\n\n
		 <b>int flags</b>\n\n
		 The display flags, which are;\n\n
		 <b>USE_DAMAGE_RECT</b>\n\n
		 If this flag is set, only the damaged area needs to be displayed. The damaged
		 rectangle may be retrieved using <b>INode::GetDamagedRect()</b>. See
		 Class INode.\n\n
		 <b>DISP_SHOWSUBOBJECT</b>\n\n
		 This indicates if an item should display its sub-object selection state. The
		 system will set this flag is the item is selected, the user is in the modify
		 branch, and the item is in sub-object selection mode.\n\n
		 <b>INode *node</b>\n\n
		 Points to the node being drawn.
		 \return  true if displayed, otherwise false. */
		virtual bool Display(TimeValue t, ViewExp *vpt, int flags, INode *node,Object *pObj)=0;		
		// HideObject: Queries if the normal node mesh should be displayed
		/*! \remarks This method is called to determine if the node mesh
		should be displayed. It should return true; otherwise return false.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at which to check if the node should be displayed.\n\n
		<b>INode *node</b>\n\n
		The node to check. */
		virtual bool SuspendObjectDisplay(TimeValue t,INode *node)=0;
		// AddNodeCallbackBox: Asks the callback to participate in the bounding box calculation
		/*! \remarks This method will ask the callback to participate in the bounding
		box calculation.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at which to calculate the bounding box.\n\n
		<b>INode *node</b>\n\n
		The node to calculate the bounding box for.\n\n
		<b>ViewExp *vpt</b>\n\n
		Points to an interface for the viewport in which the node is being drawn
		in.\n\n
		<b>Box3\& box</b>\n\n
		A reference to the bounding box. */
		virtual void AddNodeCallbackBox(TimeValue t, INode *node, ViewExp *vpt, Box3& box,Object *pObj)=0;
		// HitTest: Hit testing on the callback's mesh
		/*! \remarks This method hit tests the callback's mesh.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at which to hit test.\n\n
		<b>INode *node</b>\n\n
		A pointer to the node to test.\n\n
		<b>int type</b>\n\n
		The type of hit testing to perform. See \ref SceneAndNodeHitTestTypes. \n\n
		<b>int crossing</b>\n\n
		The state of the crossing setting. If TRUE crossing selection is on.\n\n
		<b>int flags</b>\n\n
		The hit test flags. See \ref SceneAndNodeHitTestFlags for details.\n\n
		<b>IPoint2 *p</b>\n\n
		The screen point to test.\n\n
		<b>ViewExp* vpt</b>\n\n
		An interface pointer that may be used to call methods associated with the
		viewports.
		\return  true if the item was hit, otherwise false. */
		virtual bool HitTest(TimeValue t, INode *node, int type, int crossing, int flags, IPoint2 *p, ViewExp* vpt,Object *pObj)=0;
		// Activate: Called when the callback gets activated, it's up to the callback to invalidate the screen
		/*! \remarks This method is called when the callback gets activated.
		It is up to the callback to invalidate the screen. */
		virtual void Activate()=0;
		// Deactivate: Called when the callback is deactivated
		/*! \remarks This method is called when the callback is deactivated.
		*/
		virtual void Deactivate()=0;
		// GetName: Queries the name of the callback. (Used for display in the menu)
		/*! \remarks This method returns the name of the callback which is
		used for display in the menu. The user must delete the string returned.
		*/
		virtual MSTR GetName() const = 0;// user must delete the string

	};

#define IID_NODE_DISPLAY_CALLBACK_EX Interface_ID(0x5c583214, 0x4a3b44e7)

//we're adding an overload of SuspendObjectDisplay which gets a viewport and the object cache
class NodeDisplayCallbackEx : public BaseInterface, public NodeDisplayCallback{
	public:

		// HideObject: Queries if the normal node mesh should be displayed
		virtual bool SuspendObjectDisplay(TimeValue t, ViewExp *vpt,  INode *node,Object *pObj )=0;
	};

#define I_NODEDISPLAYCONTROL  0x00001000	

// Gets a pointer to the INodeDisplayControl interface, the caller should pass a pointer to "Interface"
#define GetNodeDisplayControl(i)  ((INodeDisplayControl*)i->GetInterface(I_NODEDISPLAYCONTROL))

// An interface that is used to register the node display callback.
/*! \sa  Class InterfaceServer, Class NodeDisplayCallback, Class INode.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class is an interface that is used to register the node display callback.
To get a pointer to this interface the developer should use the following
macro:\n\n
<b>#define GetNodeDisplayControl(i)
((INodeDisplayControl*)i-\>GetInterface(I_NODEDISPLAYCONTROL))</b>  */
class INodeDisplayControl : public InterfaceServer
{
	public:

		// Register a NodeDisplayCallback
		/*! \remarks Register a node display callback which can be used to
		control the display of nodes in the scene.
		\par Parameters:
		<b>NodeDisplayCallback *cb</b>\n\n
		Points to the callback object. */
		virtual void RegisterNodeDisplayCallback(NodeDisplayCallback *cb)=0;
		/*! \remarks Un-registers the node display callback.
		\par Parameters:
		<b>NodeDisplayCallback *cb</b>\n\n
		Points to the callback object. */
		virtual void UnRegisterNodeDisplayCallback(NodeDisplayCallback *cb)=0;

		//Set and get the current current callback, the callback must be registered
		/*! \remarks Sets the current current callback. The callback must be
		previously registered.
		\par Parameters:
		<b>NodeDisplayCallback* hook</b>\n\n
		Points to the node display callback to set.
		\return  Returns true if the callback was set; otherwise false. */
		virtual bool SetNodeCallback(NodeDisplayCallback* hook)=0;
		/*! \remarks Returns a pointer to the current current callback. The
		callback must be previously registered. */
		virtual NodeDisplayCallback* GetNodeCallback()=0;


		// Viewport refresh routine
		// this function only invalidates the display, it's up to the callback to select the correct redraw technique.
		/*! \remarks Viewport refresh routine. Tthis function only invalidates
		the display, it's up to the callback to select the correct redraw
		technique. */
		virtual void InvalidateNodeDisplay()=0;

};

