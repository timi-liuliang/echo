/*! \file PFSimpleOperator.h
    \brief PF SimpleOperator (abstract class) header
				 Features that are common for a simplest form of
				 PF Operator. SimpleOperator doesn't have a
				 visual representation in viewports therefore it
				 cannot depend on its own icon to determine parameters
				 of the operation. All parameters are defined through
				 ParamBlock2
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 10-23-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "PFSimpleAction.h"
#include "IPFOperator.h"
#include "..\iparamb2.h"

class PFSimpleOperator:	public PFSimpleAction,
						public IPFOperator
{
public:
	// constructor: inherited as a base class constructor
	PFSimpleOperator() { _activeIcon() = _inactiveIcon() = NULL; }
	PFExport virtual ~PFSimpleOperator();

	// From InterfaceServer
	PFExport virtual BaseInterface* GetInterface(Interface_ID id);

	// --- These methods MUST be implemented by the derived class --- //
	// --- These methods have a default virtual implementation to --- //
	// --- ease PFExport implementation of Clone(...) method     --- //

	// From IPFOperator interface
#pragma warning(push)
#pragma warning(disable:4100)
	virtual bool Proceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd, Object* pSystem, INode* node, IObject* integrator) { return false; }
#pragma warning(pop)
protected:
		// const access to class members
		HBITMAP		activeIcon()	const	{ return m_activeIcon; }
		HBITMAP		inactiveIcon()	const	{ return m_inactiveIcon; }

		// access to class members
		HBITMAP&	_activeIcon()			{ return m_activeIcon; }
		HBITMAP&	_inactiveIcon()			{ return m_inactiveIcon; }

private:
	HBITMAP m_activeIcon;
	HBITMAP m_inactiveIcon;
};

