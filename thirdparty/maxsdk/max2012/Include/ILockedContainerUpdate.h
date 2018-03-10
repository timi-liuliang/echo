/*==============================================================================
Copyright 2010 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
or which otherwise accompanies this software in either electronic or hard copy form.   
*****************************/
// DESCRIPTION: Interfaces for handling local edits during container update.
// AUTHOR: Michael Zyracki created 2009
//***************************************************************************/
#pragma once

#include "inode.h"

//! \brief The interface ID for an object that implements ILockedContainerUpdate
//! The id that's passed into BaseInterface * Animatable::GetInterface(Interface_ID id) when querying for an ILockedContainerUpdate object
#define IID_LOCKED_CONTAINER_UPDATE Interface_ID(0x5e323e2, 0x2add7cb8)
 
class IContainerUpdateReplacedNode;
/*! This class provides an interface for classes that may need to augment the new container update process that was introduced in 3ds Max 2011.
During this update process, locked items from the source are merged into the master file and replace their
corresponding locked counterpart in the master if one exists. This process is automatic but sometimes the item in question needs to augment 
this process since some information may get lost during this merge and copy process.  Note there may be some other cases where the source item 
is merged in, for example unlocked modifiers may get copied in from the source if that modifier doesn't exist in the master.
An example of a class that needs to implement this is the skin modifier. When a skin modifier is put onto the modifier stack on some node,
any existing weight information on the specific vertices are automatically lost, since they are stored in the local mod data for modifier instance,
even though that skin modifier has the correct nodes in its node list, which is the case during the container update process. 
So what the skin modifier needs to do is provide an implementation of ILockedContainerUpdate interface. In the ILockedContainerUpdate::PreReplacement
function it copies and saves out it's weight information, and then in the ILockedContainerUpdate::PostReplacement function it pastes these
weights back onto itself, since this function is called after the replacement has happened. You can see this example at maxsdk\samples\bondesdef\.

Note that the container update process does handle the transferring of node references during the process, that's why the skin modifier
doesn't need to worry about having the correct nodes it's node list. However external non-node references aren't automatically handled so
an item has one and this reference isn't locked, it will need to make sure that the correct one is found and the reference replaced. For example
we do this internally with our wire parameters, if the object that's controlling the wire isn't locked we make sure that this existing master
item replaces the merged item that's coming in from the source.
\see ILockedTracksMan
*/

class ILockedContainerUpdate : public BaseInterface
{
public:
	//! Deconstructor
	virtual ~ILockedContainerUpdate(){};

	//! \brief Return the Interface_ID
	//! \return The default implementation returns IID_LOCKED_CONTAINER_UPDATE
	virtual Interface_ID	GetID() { return IID_LOCKED_CONTAINER_UPDATE; }

	//! \brief Function that's called before the source item replaces the master item during a container update.
	//! \param[in] sourceNode The merged node from the source, that may be needed to find a modifier's ModContext, etc..  Note that the
	//! sourceNode doesn't replace the masterNode, only the sourceAnim replaces the masterAnim on the masterNode.
	//! \param[in] sourceanim The merged Animatable from the source that replaces the masterAnim parameter. Note that this value may be the same as the 
	//! object that implements this function.
	//! \param[in] masterNode The masterNode that the masterAnim exists under. It may be needed to find a modifier's ModContext, etc.. Note that the
	//! sourceNode doesn't replace the masterNode, only the sourceAnim replaces the masterAnim on the masterNode.
	//! \param[in] masterAnim The Animatable that will get replaced. 
	//! \param[in] man An IContainerUpdateReplacedNode interface that lets you query for a node currently in the scene what node, if any, will be replacing 
	//! it after the update process. As noted the references to these nodes are automatically transferred but it still may be useful to the client
	//! to know exactly what's getting replaced, in case for some reason they aren't using reference to keep track of a node.
	//! \param[in,out] log A string to place any pertinent error messages that the container update system may output as an error log during the update process.
	//! \return Returns true if the function succeeds, false otherwise.
	virtual bool PreReplacement(INode *sourceNode, Animatable *sourceAnim, INode *masterNode, Animatable *masterAnim, IContainerUpdateReplacedNode *man, MSTR &log)   = 0;


	//! \brief Function that's called after the source item replaces the master item during a container update.
	//! \param[in] sourceNode The merged node from the source, that may be needed to find a modifier's ModContext, etc..  Note that the
	//! sourceNode doesn't replace the masterNode, only the sourceAnim replaces the masterAnim on the masterNode.
	//! \param[in] sourceanim The merged Animatable from the source that replaces the masterAnim parameter. Note that this value may be the same as the 
	//! object that implements this function.
	//! \param[in] masterNode The masterNode that the master anim exists under. It may be needed to find a modifier's ModContext, etc.. Note that the
	//! sourceNode doesn't replace the masterNode.
	//! \param[in] man An IContainerUpdateReplacedNode interface that lets you query for a node currently in the scene what node, if any, will be replacing 
	//! it after the update process. As noted the references to these nodes are automatically transferred but it still may be useful to the client
	//! to know exactly what's getting replaced, in case for some reason they aren't using reference to keep track of a node.
	//! \param[in,out] log A string to place any pertinent error messages that the container update system may output as an error log during the update process.
	//! \return Returns true if the function succeeds, false otherwise.
	virtual bool PostReplacement(INode *sourceNode, Animatable *sourceAnim, INode *masterNode, IContainerUpdateReplacedNode *man, MSTR &log) = 0;
};


/*! \brief This class provides an interface to retrieve what new node will replace an existing one during the local edit process.
An instance of this class is passed in the function ILockedContainerUpdate::PreReplacement and ILockedContainerUpdate::PostReplacement. \see ILockedContainerUpdate
Here's a possible example of a fake class that maintains the distance between two nodes that needs to recalculate this distance during the container update 
process.
/code
class ReplacedNodeExample : public ILockedContainerUpdate
{
...
	INode *mNode1; //one node
	INode *mNode2; //another node
	float mDistanceBetween; //distance between them that's calculated once the nodes get set.
}
void ReplacedNodeExample::PreReplacement(INode *sourceNode, Animatable *sourceAnim, INode *masterNode, Animatable *masterAnim, IContainerUpdateReplacedNode *man,
										 MSTR &log)
{
	INode *newNode1 = man->NewNode(mNode1);
	//if mNode1 is a reference no need to do mNode1 = newNode1 or SetReference(newNode1)...
	INode *newNode2 = man->NewNode(mNode2);
	//calculate new distance between.
	if(newNode1 && newNode2) //they may have gotten deleted
		mDistanceBetween = Distance(newNode1.GetTrans() - newNode2.GetTrans());
}
/endcode
*/
class IContainerUpdateReplacedNode : public MaxHeapOperators
{
public:
	//! Deconstructor
	virtual ~IContainerUpdateReplacedNode(){};
	//! \brief This functions returns what node will replace the master node in the scene during the container update process.
	//! \param[in] masterNode The node that we are querying to see how it will get replaced.
	//! \return The node that replaces the master node. If it's the same as the masterNode parameter then that node isn't replaced, and if returns NULL the node
	//! is deleted, otherwise it will return a different node than the masterNode parameter.
	virtual INode *NewNode(INode *masterNode) = 0;
};