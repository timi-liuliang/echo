/*! \file IPFArrow.h
    \brief Interface for PF Arrow
				 Directs particles from Test to the next ActionList
				 Part of public SDK
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 10-15-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "..\ifnpub.h"
#include "..\object.h"

PFExport Object* GetPFObject(Object* obj);
// interface ID
#define PFARROW_INTERFACE Interface_ID(0x74f93d05, 0x1eb34500) 
#define GetPFArrowInterface(obj) ((IPFArrow*)(GetPFObject(obj))->GetInterface(PFARROW_INTERFACE)) 


class IPFArrow : public FPMixinInterface
{
public:

	// function IDs
	enum {	kSetTest,
			kSetActionList,
			kGetTest,
			kGetActionList,
			kActivate,
			kDeactivate,
			kIsActive
	}; 

	// Function Map for Function Publish System 
	//***********************************
	BEGIN_FUNCTION_MAP

	FN_1(kSetTest, TYPE_bool, SetTest, TYPE_INODE);
	FN_1(kSetActionList, TYPE_bool, SetActionList, TYPE_INODE);
	FN_0(kGetTest, TYPE_INODE, GetTest);
	FN_0(kGetActionList, TYPE_INODE, GetActionList);
	FN_0(kActivate, TYPE_bool, Activate);
	FN_0(kDeactivate, TYPE_bool, Deactivate);
	FN_0(kIsActive, TYPE_bool, IsActive);

	END_FUNCTION_MAP

    /** @defgroup IPFArrow IPFArrow.h 
    *  @{
    */

    /*! \fn virtual bool SetTest(INode* test) = 0;
    *  \brief Set test the particles are coming from.
	 Returns true if has been set successfully
	 The INode can be rejected if it is not a real test (doesn't have IPFTest interface).
    */
	virtual bool SetTest(INode* test) = 0;

    /*! \fn virtual bool SetActionList(INode* actionList) = 0;
    *  \brief Set actionList for particles to come to.
	 Returns true if has been set successfully.
	 The INode can be rejected if it is not a real actionList (doesn't have IPFActionList interface).
    */
	virtual bool SetActionList(INode* actionList) = 0;

    /*! \fn virtual INode* GetTest() const = 0;
    *  \brief 
    */
	virtual INode* GetTest() const = 0;

    /*! \fn virtual INode* GetActionList() const = 0;
    *  \brief 
    */
	virtual INode* GetActionList() const = 0;

    /*! \fn virtual bool Activate() = 0;
    *  \brief 
    */
	virtual bool Activate() = 0;

    /*! \fn virtual bool Deactivate() = 0;
    *  \brief 
    */
	virtual bool Deactivate() = 0;

    /*! \fn virtual bool IsActive() const = 0;
    *  \brief 
    */
	virtual bool IsActive() const = 0;

    /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PFARROW_INTERFACE); }
    *  \brief 
    */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PFARROW_INTERFACE); }

    /*! \fn PFExport void updateNextActionListValueInTestFrom(INode* nextAL, INode* testFrom);
    *  \brief For internal use
    */
	PFExport void updateNextActionListValueInTestFrom(INode* nextAL, INode* testFrom);

    /*! \fn PFExport void updateArrowValueInTestFrom(Object* arrow, INode* testFrom);
    *  \brief For internal use
    */
	PFExport void updateArrowValueInTestFrom(Object* arrow, INode* testFrom);

    /*! \fn PFExport void updateLinkActiveValueInTestFrom(bool active, INode* testFrom);
    *  \brief For internal use
    */
	PFExport void updateLinkActiveValueInTestFrom(bool active, INode* testFrom);
};

inline IPFArrow* PFArrowInterface(Object* obj) {
	return ((obj == NULL) ? NULL : GetPFArrowInterface(obj));
};

inline IPFArrow* PFArrowInterface(INode* node) {
	return ((node == NULL) ? NULL : PFArrowInterface(node->GetObjectRef()));
};

/*@}*/

