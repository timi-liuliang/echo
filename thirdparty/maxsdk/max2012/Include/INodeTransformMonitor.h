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
// DESCRIPTION: Defines an interface that uses the RefTargMonitorRefMaker class to monitor a node for 
// its PART_TM messages. If the NodeTransformMonitor instance is saved as part of a partial save, or 
// loaded as a partial load, the node pointed to by the NodeTransformMonitor is also saved or loaded.
// AUTHOR: Larry.Minton - created May.14.2004
//***************************************************************************/

#pragma once

#include "iFnPub.h"
#include "IRefTargMonitor.h"

//! ClassID for NodeTransformMonitor
#define NODETRANSFORMMONITOR_CLASS_ID			Class_ID(0x18f81902, 0x19033fd2)

//! Id for INodeTransformMonitor interface
#define IID_NODETRANSFORMMONITOR				Interface_ID(0x18f81902, 0x19033fd2)

//! RefMessage sent by NodeTransformMonitor's ProcessRefTargMonitorMsg when the monitored node is deleted
//! The hTarg argument is the node being deleted
#define REFMSG_NODETRANSFORMMONITOR_TARGET_DELETED				REFMSG_USER + 0x18f81902

//! RefMessage sent by NodeTransformMonitor's ProcessRefTargMonitorMsg when the monitored node is set or set to NULL,which may happened during an Undo/Redo.
#define REFMSG_NODETRANSFORMMONITOR_TARGET_SET REFMSG_REFTARGMONITOR_TARGET_SET 

//! \brief The interface for working with NodeTransformMonitor class.
/*! The NodeTransformMonitor class (defined in ctrl.dlc) is used to monitor a node for its \ref REFMSG_CHANGE / 
PART_TM and \ref REFMSG_TARGET_DELETED messages. The NodeTransformMonitor class creates a RefTargMonitorRefMaker
instance pointing at a node, and allows only the \ref REFMSG_TARGET_DELETED message and the REFMSG_CHANGE message when
PartID is PART_TM to propagate to the NodeTransformMonitor's dependents. To prevent circular message 
loops, the messages are not propagated if that message type is already being propagated.
*/
class INodeTransformMonitor : public FPMixinInterface  {
public:
	//! \brief Retrieves the node being watched.
	/*! \returns INode pointer to the node being watched.
	*/
	virtual INode* GetNode() = 0;
	//! \brief Sets the node being watched.
	/*! \param theNode - Pointer to node to be watched. Can be NULL.
	*/
	virtual void SetNode(INode *theNode) = 0;

	//! \brief Retrieves whether to monitor for \ref REFMSG_CHANGE / PART_TM messages.
	/*! \returns True if to monitor for \ref REFMSG_CHANGE / PART_TM messages.
	*/
	virtual bool GetForwardTransformChangeMsgs() = 0;
	//! \brief Sets whether to monitor for \ref REFMSG_CHANGE / PART_TM messages.
	/*! \param state - True if to monitor for \ref REFMSG_CHANGE / PART_TM messages.
	*/
	virtual void SetForwardTransformChangeMsgs(bool state) = 0;

	//! \brief Retrieves whether to monitor for \ref REFMSG_FLAG_NODES_WITH_SEL_DEPENDENTS messages.
	/*! \returns True if to monitor for \ref REFMSG_FLAG_NODES_WITH_SEL_DEPENDENTS messages.
	*/
	virtual bool GetForwardFlagNodesMsgs() = 0;
	//! \brief Sets whether to monitor for \ref REFMSG_FLAG_NODES_WITH_SEL_DEPENDENTS messages.
	/*! \param state - True if to monitor for \ref REFMSG_FLAG_NODES_WITH_SEL_DEPENDENTS messages.
	*/
	virtual void SetForwardFlagNodesMsgs(bool state) = 0;

	//! \brief Retrieves whether to monitor for EnumDependentImp calls.
	/*! \returns True if to monitor for EnumDependentImp calls.
	*/
	virtual bool GetForwardEnumDependentsCalls() = 0;
	//! \brief Sets whether to monitor for EnumDependentImp calls.
	/*! \param state - True if to monitor for EnumDependentImp calls.
	*/
	virtual void SetForwardEnumDependentsCalls(bool state) = 0;

	// --- Function publishing
	FPInterfaceDesc* GetDesc();    // <-- must implement 
	virtual Interface_ID GetID() { return IID_NODETRANSFORMMONITOR; }

	enum {  
		kfpGetnode, kfpSetnode,
		kfpGetforwardPartTM, kfpSetforwardPartTM,
		kfpGetforwardFlagNodes, kfpSetforwardFlagNodes,
		kfpGetforwardEnumDependents, kfpSetforwardEnumDependents,
	};

	BEGIN_FUNCTION_MAP
		PROP_FNS(kfpGetnode, GetNode, kfpSetnode, SetNode, TYPE_INODE);
		PROP_FNS(kfpGetforwardPartTM, GetForwardTransformChangeMsgs, kfpSetforwardPartTM, SetForwardTransformChangeMsgs, TYPE_bool);
		PROP_FNS(kfpGetforwardFlagNodes, GetForwardFlagNodesMsgs, kfpSetforwardFlagNodes, SetForwardFlagNodesMsgs, TYPE_bool);
		PROP_FNS(kfpGetforwardEnumDependents, GetForwardEnumDependentsCalls, kfpSetforwardEnumDependents, SetForwardEnumDependentsCalls, TYPE_bool);
	END_FUNCTION_MAP

};

