/*! \file PFSimpleActionState.h
    \brief Implementation of ActionState that has only a single
				 random generator.
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 10-25-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "IPFActionState.h"
#include "PFClassIDs.h"
#include "..\RandGenerator.h"
#include "..\ifnpub.h"

class PFSimpleActionState:	public IObject,
							public IPFActionState
{
public:
	// from IObject interface
	int NumInterfaces() const { return 1; }
	BaseInterface* GetInterfaceAt(int index) const { return ((index == 0) ? (IPFActionState*)this : NULL); }
	BaseInterface* GetInterface(Interface_ID id);
	void DeleteIObject();

	// From IPFActionState
	Class_ID GetClassID() { return PFSimpleActionState_Class_ID; }
	ULONG GetActionHandle() const { return actionHandle(); }
	void SetActionHandle(ULONG handle) { _actionHandle() = handle; }
	IOResult Save(ISave* isave) const;
	IOResult Load(ILoad* iload);

public:
		// const access to class members
		const RandGenerator*	randGen()		const	{ return &m_randGen; }
		// access to class members
		RandGenerator*			_randGen()				{ return &m_randGen; }

protected:
		// const access to class members
		ULONG					actionHandle()	const	{ return m_actionHandle; }
		// access to class members
		ULONG&					_actionHandle()			{ return m_actionHandle; }

private:
		ULONG m_actionHandle;
		RandGenerator m_randGen;
};

