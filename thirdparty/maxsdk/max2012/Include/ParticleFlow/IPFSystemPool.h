/*! \file IPFSystemPool.h
    \brief The interface and a method are used to access a list
				 of all PF particle systems in the current scene.
				 It is possible to register notification with the object
				 thus getting notification each time when the set of
				 all PF particle systems is changing (create, clone, delete).
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 08-20-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "PFExport.h"
#include "..\ifnpub.h"
#include "..\notify.h"

// interface ID
#define PFSYSTEMPOOL_INTERFACE Interface_ID(0x74f93d0a, 0x1eb34500)

#define GetPFSystemPoolInterface(obj) ((IPFSystemPool*)obj->GetInterface(PFSYSTEMPOOL_INTERFACE))

class IPFSystemPool : public FPMixinInterface
{
public:

	// function IDs
	enum {	kNumPFSystems,
			kGetPFSystem,
			kHasPFSystem,
			kHasPFSystemByHandle,
			kRegisterNotification,
			kUnRegisterNotification
	};

	BEGIN_FUNCTION_MAP
	FN_0(kNumPFSystems, TYPE_INT, NumPFSystems );
	FN_1(kGetPFSystem, TYPE_INODE, GetPFSystem, TYPE_INDEX );
	FN_1(kHasPFSystem, TYPE_bool, HasPFSystem, TYPE_INODE );
	FN_1(kHasPFSystemByHandle, TYPE_bool, HasPFSystemByHandle, TYPE_INT );
	END_FUNCTION_MAP

    /** @defgroup IPFSystemPool IPFSystemPool.h
    *  @{
    */

      /*! \fn virtual int NumPFSystems() = 0;
	*  \brief  returns number of PFSystems in the current scene
      */
	virtual int NumPFSystems() = 0;

	/*! \fn virtual INode* GetPFSystem(int i) = 0;
	*  \brief  Returns i-th PFSystem in the scene
	*  \param i: PFSystem index
      */
	virtual INode* GetPFSystem(int i) = 0;

	/*! \fn virtual bool HasPFSystem(INode* node) = 0;
	*  \brief  Checks if the given PFSystem node is present in the scene
	*  \param node: PFSystem node
      */
	virtual bool HasPFSystem(INode* node) = 0;

	/*! \fn virtual bool HasPFSystemByHandle(int handle) = 0;
	*  \brief Check if the scene has a PFSystem with the given node handle
      */
	virtual bool HasPFSystemByHandle(int handle) = 0;

	/*! \fn virtual int RegisterNotification(NOTIFYPROC proc, void *param) = 0;
	*  \brief  Register notification with the PFSystem pool.
	   The given function is called each time the set of all PFSystems is changing.
	   Returns nonzero if the event was registered; otherwise zero.
	*  \param proc: The callback function called when the event occurs.
      */
	virtual int RegisterNotification(NOTIFYPROC proc, void *param) = 0;

	/*! \fn virtual int UnRegisterNotification(NOTIFYPROC proc, void *param) = 0;
	*  \brief  Unregister notification with the PFSystem pool.
	   This function is called to break the connection between the event and the callback.
	   After this function executes the callback is no longer invoked when the event occurs.
	   Returns nonzero if the event was unregistered; otherwise zero.
	*  \param proc: The callback function called when the event occurs.
      */
	virtual int UnRegisterNotification(NOTIFYPROC proc, void *param) = 0;

	/*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PFSYSTEMPOOL_INTERFACE); }
	*  \brief  
      */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PFSYSTEMPOOL_INTERFACE); }

	/*! \fn friend PFExport IPFSystemPool* GetPFSystemPool();
	*  \brief  Friend function declarations
      */
    friend PFExport IPFSystemPool* GetPFSystemPool();

protected:
	PFExport IPFSystemPool();

	static IPFSystemPool* m_pool; // the one and only PFSystem Pool in the scene
};

/*! \fn PFExport IPFSystemPool* GetPFSystemPool();
*  \brief Gets PFSystemPool from the scene
*/
PFExport IPFSystemPool* GetPFSystemPool();

/*@}*/

