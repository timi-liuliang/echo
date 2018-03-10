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
// DESCRIPTION: Defines an interface that uses the RefTargMonitorRefMaker class to monitor a RefTarg for 
// deletion. If the RefTargMonitor instance is saved as part of a partial save, or loaded as a partial load, 
// the RefTarg pointed to by the RefTargMonitor is not forced to be saved or loaded.
// AUTHOR: Larry.Minton - created Jan.11.2006
//***************************************************************************/

#pragma once

#include "iFnPub.h"
#include "IRefTargMonitor.h"

//! ClassID for RefTargMonitor
#define REFTARGMONITOR_CLASS_ID			Class_ID(0x1af82963, 0x29663cd2)

//! Id for IRefTargMonitorClass interface
#define IID_REFTARGMONITOR_CLASS		Interface_ID(0x1af82963, 0x29663cd2)

//! RefMessage sent by RefTargMonitor's ProcessRefTargMonitorMsg when the monitored RefTarg is deleted
//! The hTarg argument is the RefTarg being deleted
#define REFMSG_REFTARGMONITOR_TARGET_DELETED				REFMSG_USER + 0x18f81904

//! \brief The interface for working with RefTargMonitor class.
/*! The RefTargMonitor class (defined in ctrl.dlc) is used to monitor a RefTarg for its  
\ref REFMSG_TARGET_DELETED messages. The RefTargMonitor class creates a RefTargMonitorRefMaker instance
pointing at a RefTarg, and allows only the \ref REFMSG_TARGET_DELETED message to propagate to the RefTargMonitor's 
dependents. To prevent circular message loops, the messages are not propagated if that message type 
is already being propagated.
*/
class IRefTargMonitorClass : public FPMixinInterface  {
public:
	//! \brief Retrieves the RefTarg being watched.
	/*! \returns  The RefTarg being watched.
	*/
	virtual RefTargetHandle GetRefTarg() = 0;
	//! \brief Sets the RefTarg being watched.
	/*! \param theRefTarg - The RefTarg to be watched. Can be NULL.
	*/
	virtual void SetRefTarg(RefTargetHandle theRefTarg) = 0;

	//! \brief Retrieves whether to force persistence of the target on partial load/save.
	/*! \returns  True if force persistence in on.
	*/
	virtual bool GetPersist() = 0;
	//! \brief Sets whether to force persistence of the target on partial load/save.
	/*! \param persist - True to force persistence.
	*/
	virtual void SetPersist(bool persist) = 0;

	// --- Function publishing
	FPInterfaceDesc* GetDesc();    // <-- must implement 
	virtual Interface_ID GetID() { return IID_REFTARGMONITOR_CLASS; }

	enum {  
		kfpGetRefTarg, kfpSetRefTarg,
		kfpGetPersist, kfpSetPersist,
	};

	BEGIN_FUNCTION_MAP
		PROP_FNS(kfpGetRefTarg, GetRefTarg, kfpSetRefTarg, SetRefTarg, TYPE_REFTARG);
		PROP_FNS(kfpGetPersist, GetPersist, kfpSetPersist, SetPersist, TYPE_bool);
	END_FUNCTION_MAP

};

