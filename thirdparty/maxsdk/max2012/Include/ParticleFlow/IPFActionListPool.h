/*! \file IPFActionListPool.h
    \brief The interface and a method are used to access a list
				 of all ActionList objects in the current scene.
				 It is possible to register notification with the object
				 thus getting notification each time when the set of
				 all ActionLists is changing (create, clone, delete).
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 08-20-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "..\notify.h"
#include "..\ifnpub.h"


// interface ID
#define PFACTIONLISTPOOL_INTERFACE Interface_ID(0x74f93d09, 0x1eb34500)

#define GetPFActionListPoolInterface(obj) ((IPFActionListPool*)obj->GetInterface(PFACTIONLISTPOOL_INTERFACE))

class IPFActionListPool : public FPMixinInterface
{
public:

	// function IDs
	enum {	kNumActionLists,
			kGetActionList,
			kHasActionList,
			kHasActionListByHandle,
			kRegisterNotification,
			kUnRegisterNotification
	};

	BEGIN_FUNCTION_MAP
	FN_0(kNumActionLists, TYPE_INT, NumActionLists );
	FN_1(kGetActionList, TYPE_INODE, GetActionList, TYPE_INDEX );
	FN_1(kHasActionList, TYPE_bool, HasActionList, TYPE_INODE );
	FN_1(kHasActionListByHandle, TYPE_bool, HasActionListByHandle, TYPE_INT );
	END_FUNCTION_MAP

    /** @defgroup IPFActionListPool IPFActionListPool.h 
    *  @{
    */

	/*! \fn virtual int NumActionLists() = 0;
	*  \brief Returns number of ActionLists in the current scene
	*/
	virtual int NumActionLists() = 0;

	/*! \fn virtual INode* GetActionList(int i) = 0;
	*  \brief Returns i-th action lists in the scene
	   \param i: action list index
	*/
	virtual INode* GetActionList(int i) = 0;

	/*! \fn virtual bool HasActionList(INode* node) = 0;
	*  \brief Checks if the given aciton list node is present in the scene
	   \param node: action list node
	*/
	virtual bool HasActionList(INode* node) = 0;

	/*! \fn virtual bool HasActionListByHandle(int handle) = 0;
	*  \brief  check if the scene has an action list with the given node handle
	*/
	virtual bool HasActionListByHandle(int handle) = 0;

	/*! \fn virtual int RegisterNotification(NOTIFYPROC proc, void *param) = 0;
	*  \brief  Register notification with the action list pool.
	   The given function is called each time the set of all action lists is changing
	   Returns nonzero if the event was registered; otherwise zero.
	   \param proc: The callback function called when the event occurs.
	   \param param: A pointer to a parameter which will be passed to the callback function.
	*/
	virtual int RegisterNotification(NOTIFYPROC proc, void *param) = 0;

	/*! \fn virtual int UnRegisterNotification(NOTIFYPROC proc, void *param) = 0;
	*  \brief  Unregister notification with the action list pool.
	   This function is called to break the connection between the event and the callback.
	   After this function executes the callback is no longer invoked when the event occurs.
	   Returns nonzero if the event was unregistered; otherwise zero.
	   \param proc: The callback function called when the event occurs.
	   \param param: A pointer to a parameter which will be passed to the callback function.
	*/
	virtual int UnRegisterNotification(NOTIFYPROC proc, void *param) = 0;

	/*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PFACTIONLISTPOOL_INTERFACE); }
	*  \brief  
	*/
	FPInterfaceDesc* GetDesc() { return GetDescByID(PFACTIONLISTPOOL_INTERFACE); }

	/*! \fn friend PFExport IPFActionListPool* GetPFActionListPool();
	*  \brief  Friend function declarations
	*/
    friend PFExport IPFActionListPool* GetPFActionListPool();

protected:
	PFExport IPFActionListPool();

	static IPFActionListPool* m_pool; // the one and only PFActionList Pool in the scene
};

/*! \fn PFExport IPFActionListPool* GetPFActionListPool();
*  \brief Gets PFActionListPool from the scene 
*/
PFExport IPFActionListPool* GetPFActionListPool();

/*@}*/

