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
// deletion. If the NodeMonitor instance is saved as part of a partial save, or loaded as a partial load, 
// the node pointed to by the NodeMonitor is not forced to be saved or loaded.
// AUTHOR: Larry.Minton - created Jan.11.2006
//***************************************************************************/

#pragma once

#include "iFnPub.h"
#include "IRefTargMonitor.h"

//! ClassID for NodeMonitor
#define NODEMONITOR_CLASS_ID			Class_ID(0x18f81903, 0x19033fd2)

//! Id for INodeMonitor interface
#define IID_NODEMONITOR				Interface_ID(0x18f81903, 0x19033fd2)

//! RefMessage sent by NodeMonitor's ProcessRefTargMonitorMsg when the monitored node is deleted
//! The hTarg argument is the node being deleted
#define REFMSG_NODEMONITOR_TARGET_DELETED				REFMSG_USER + 0x18f81903

//! RefMessage sent by NodeTransformMonitor's ProcessRefTargMonitorMsg when the monitored node is set or set to NULL,which may happened during an Undo/Redo.
#define REFMSG_NODEMONITOR_TARGET_SET REFMSG_REFTARGMONITOR_TARGET_SET 

//! \brief The interface for working with NodeMonitor class.
/*! The NodeMonitor class (defined in ctrl.dlc) is used to monitor a node for its  
\ref REFMSG_TARGET_DELETED messages. The NodeMonitor class creates a RefTargMonitorRefMaker instance
pointing at a node, and allows only the \ref REFMSG_TARGET_DELETED message to propagate to the NodeMonitor's 
dependents. To prevent circular message loops, the messages are not propagated if that message type 
is already being propagated.
*/
class INodeMonitor : public FPMixinInterface  {
public:
	//! \brief Retrieves the node being watched.
	/*! \returns INode pointer to the node being watched.
	*/
	virtual INode* GetNode() = 0;
	//! \brief Sets the node being watched.
	/*! \param theNode - Pointer to node to be watched. Can be NULL.
	*/
	virtual void SetNode(INode *theNode) = 0;

	// --- Function publishing
	FPInterfaceDesc* GetDesc();    // <-- must implement 
	virtual Interface_ID GetID() { return IID_NODEMONITOR; }

	enum {  
		kfpGetnode, kfpSetnode,
	};

	BEGIN_FUNCTION_MAP
		PROP_FNS(kfpGetnode, GetNode, kfpSetnode, SetNode, TYPE_INODE);
	END_FUNCTION_MAP

};

