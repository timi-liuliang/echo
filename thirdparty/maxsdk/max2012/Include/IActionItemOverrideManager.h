//**************************************************************************/
// Copyright (c) 1998-2007 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
/**************************************************************************
	FILE: IActionItemOverrideManager.h

	DESCRIPTION: Declares a public header for a function published core interface
				 for managing Action Item Overrides.
	AUTHOR: Michael Zyracki - created 01.02.2007
/***************************************************************************/
#pragma once
#include "MaxHeap.h"
#include <WTypes.h>
#include "ifnpub.h"

class ActionItem;

//! \brief Class used to specify action item overrides.

//! The IActionItemOverride class is an abstract base class to be used to create an action item override. An action item override is 
//! an action item that will only execute as long as the action item keyboard shortcut is held. Once released the override will revert back to the state it
//! was in before the action item executed.  In order for an override to occur instead of an action item, the client needs to create an 
//! instance of a class that implements the virtual functions defined in this base class, and then activate it by
//! calling IActionItemOverrideManager::ActivateActionItemOverride.  
//! \see IActionItemOverrideManager
class IActionItemOverride : public MaxHeapOperators
{
public:
	//! \brief Virtual Deconstructor that allows the IctionItemOverrideManager to delete IActionItemOverride objects correctly.
	virtual ~IActionItemOverride(){};

	//! \brief Function that returns whether or not we this override is active.
	//! \return TRUE if the override is active, FALSE if otherwise.
	virtual BOOL IsOverrideActive() = 0;

	//! \brief Function that's called when we start the action item override.

	//! Function that's called when we start the action item override.  Usually the subclass that overrides this method will store the state(s) that
	//! the override will change and then perform the action, usually as if ActionItem::ExecuteAction was called.
	//! \return TRUE if we started the override correctly, FALSE otherwise
	virtual BOOL StartOverride() = 0;


	//! \brief Function that's called when we end the action item override.

	//! Function that's called when we end the action item override.  Usually the subclass that overrides this method will restore the state(s) that
	//! were stored when IActionItemOverride::StartOverride() was cas called.
	//! \return TRUE if we ended the override correctly, FALSE otherwise
	virtual BOOL EndOverride() = 0;
};

//! \brief The unique interface ID for the SimpleFaceData Manager.
#define IACTIONITEMOVERRIDEMANAGER_INTERFACE Interface_ID(0x5e1b66eb, 0x4bc034de)

//! \brief The interface for managing action item overrides. 
/*! This manager allows action item overrides to be activated, deactivated and retrieved. This static 
function-published interface is registered as a core interface.  You retrieve this interface by calling
static_cast<IActionItemOverrideManager*>(GetCOREInterface(IACTIONITEMOVERRIDEMANAGER_INTERFACE )). Note that 
once an IActionOverrideItem is managed by this interface by activating it, it then will get deleted by this interface.
\see IActionItemOverride
*/
class IActionItemOverrideManager : public FPStaticInterface
{
public:

	//! \brief Function to set action item overrides active.

	//! \param[in] active  If TRUE then action item overrides may be active, if FALSE then all action overrides will be disabled.
	virtual void SetOverridesActive(BOOL active) = 0;
	
	//! \brief Get whether or not action item overrides are globally active or not.
	
	//! \return TRUE if action item overrides may be active, FALSE if otherwise.
	virtual BOOL GetOverridesActive(void)const = 0;

	//! \brief Function to set how long a key must be pressed before a suitable action item becomes an override.

	//! \param[in] secs  The number of seconds that a key must be pressed and held before an action item becomes an override.
	virtual void SetTimeToStartOverride(float secs) =0;

	//! \brief How much time, in seconds, that we need to hold a key before a suitable ction item becomes an override.
	
	//! \return Returns how many seconds that we need to hold a key before an action time becomes an override.
	virtual float GetTimeToStartOverride()const = 0;


	//! \brief Function to register an action item as a possible override.

	//! Function to register an action item as a possible override.  The purpose of this is to inform the system of a possible override so that
	//! the action item may get highlighted in the customize UI dialog.  
	//! \param[in] aItem The action item that may have an override.
	//! \param[in] overrideDescription Text description of what the override does.
	virtual void RegisterActionItemOverride(ActionItem *aItem,MSTR &overrideDescription) =0;

	//! \brief Function to unregister an action item as a possible override.

	//! Function to unregister an action item as a possible override.  The client should call this when either an action item will no longer
	//! be overridable of if the action item will get removed from the action table. \see ActionTable::DeleteOperation.
	//! \param[in] aItem The action item that we want to unregister. Nothing happens if the action item was never registered.
	virtual void UnregisterActionItemOverride(ActionItem *aItem) =0;

	//! \brief  Function to see if an action item has been registered, and thus may eventually be activated.
	//! \param[in] aItem   The aciton item that may have an override.
	//! \param[out] description The text description of the override.
	//! \return Return TRUE if the ActionItem is registered, otherwise return FALSE.
	virtual BOOL IsActionItemRegistered(ActionItem *aItem,MSTR &description) = 0;
	//! \brief Function to activate a particular action item with a particular override, which then allows the action item to work like an override also.

	//! Function to activate a particular action item with a particular override.  This function must be called in order for a particular action item
	//! to work like an override.
	//! \param[in] aItem  The action item that will be associated with an override.
	//! \param[in] aItemOverride The action item override that will override the action item when a key is held.  Note that once a IActionItemOverride object
	//! is activated the deletion of this object is now the responsibility of the IActionItemOverrideManager, and so the client shouldn't delete it also.
	//! Usually the action item override will be activated after IActionManger::ActivateActionTable is called.
	virtual void ActivateActionItemOverride(ActionItem *aItem,IActionItemOverride*  aItemOverride)=0;
	
	//! \brief Function to deactivate an action item override.

	//! Function to unactive an action item with a corresponding action item override. Note that the activated IActionItemOverride instance will get deleted
	//! when this occurs. For efficiency, a user should unactivate their action item and action item overrides whenever they would also deactivate an action table via
	//! IActionManager::DeactiveActionTable, usually within a Animatable::EndEditParams function call.
	//! \see IActionManager::DeactivateActionTable
	//! \param[in] aItem  The action item that will be unactivated.  The corresponding IActionActionItemOverride will also get removed and deleted.
	virtual void DeactivateActionItemOverride(ActionItem* aItem)=0;

	//! \brief Find a particular activated IActionItemOverride given a particular ActionItem.
	
	//! \return Returns the IActionItemOverride object that's been activated with IActionItemOverrideManager::ActivateActionItemOverride. If none exists,
	//! it will return NULL.
	virtual IActionItemOverride * FindOverride(ActionItem * aItem)=0 ;


	//! \brief Returns the action item that is currently being overriden.

	//! \return Returns a pointer to the ActionItem that is currently being overriden.  If no action item is being overriden, it return NULL.
	virtual ActionItem* GetOverridingActionItem()const = 0;

};


