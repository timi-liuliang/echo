/*! \file IPFActionListSet.h
    \brief Interface for PF ActionList Set
				 It's a set of ActionLists. PF System implements the
				 interface to keep track of all initial ActionLists.
				Part of public SDK
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 11-09-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "..\ifnpub.h"
#include "..\object.h"

PFExport Object* GetPFObject(Object* obj);
// interface ID
#define PFACTIONLISTSET_INTERFACE Interface_ID(0x7216b6f, 0x634253c5) 
#define GetPFActionListSetInterface(obj) ((IPFActionListSet*)((GetPFObject(obj))->GetInterface(PFACTIONLISTSET_INTERFACE))) 

class IPFActionListSet : public FPMixinInterface
{
public:
	// function IDs
	enum {	kGetNumActionLists,	kGetActionList,		kAppendActionList,
			kInsertActionList,	kRemoveActionList,	kHasActionList,
			kIsActionList
	}; 

	// Function Map for Function Publish System 
	//***********************************
	BEGIN_FUNCTION_MAP

	FN_0(	kGetNumActionLists,	TYPE_INT,	NumActionLists	);
	FN_1(	kGetActionList,		TYPE_INODE, GetActionList,		TYPE_INDEX);
	FN_1(	kAppendActionList,	TYPE_bool,	AppendActionList,	TYPE_INODE);
	FN_2(	kInsertActionList,	TYPE_bool,	InsertActionList,	TYPE_INODE, TYPE_INDEX);
	FN_1(	kRemoveActionList,	TYPE_bool,	RemoveActionList,	TYPE_INDEX);
	FN_2(	kHasActionList,		TYPE_bool,	HasActionList,		TYPE_INODE, TYPE_INDEX_BR);
	FN_1(	kIsActionList,		TYPE_bool,	IsActionList,		TYPE_INODE);

	END_FUNCTION_MAP

	// must implement GetDescByID method !
	FPInterfaceDesc* GetDesc() { return GetDescByID(PFACTIONLISTSET_INTERFACE); }
	//End of Function Publishing system code 
	//***********************************

      /** @defgroup IPFActionListSet IPFActionListSet.h
      *  @{
      */

      /*! \fn virtual int		NumActionLists() const = 0;
      *  \brief 
      */
	virtual int		NumActionLists() const = 0;

      /*! \fn virtual INode*	GetActionList(int index) const = 0;
      *  \brief 
      */
	virtual INode*	GetActionList(int index) const = 0;

      /*! \fn virtual bool	AppendActionList(INode* alist) = 0;
      *  \brief 
      */
	virtual bool	AppendActionList(INode* alist) = 0;

      /*! \fn virtual bool	InsertActionList(INode* alist, int indexAt) = 0; 
      *  \brief 
      */
	virtual bool	InsertActionList(INode* alist, int indexAt) = 0;

      /*! \fn virtual bool	RemoveActionList(int index) = 0;
      *  \brief 
      */
	virtual bool	RemoveActionList(int index) = 0;

      /*! \fn virtual bool	HasActionList(INode* alist, int& index) const = 0;
      *  \brief 
      */
	virtual bool	HasActionList(INode* alist, int& index) const = 0;

      /*! \fn PFExport static bool IsActionList(INode* alist);
      *  \brief 
      */
	PFExport static bool IsActionList(INode* alist);
};

inline IPFActionListSet* PFActionListSetInterface(Object* obj) {
	return ((obj == NULL) ? NULL : GetPFActionListSetInterface(obj));
};

inline IPFActionListSet* PFActionListSetInterface(INode* node) {
	return ((node == NULL) ? NULL : PFActionListSetInterface(node->GetObjectRef()));
};


