/*! \file PFSimpleTest.h
    \brief PF SimpleTest (abstract class) header.
				 Features that are common for a simplest form of
				 PF Test. SimpleTest doesn't have a
				 visual representation in viewports therefore it
				 cannot depend on its own icon to determine parameters
				 of the operation. All parameters are defined through
				 ParamBlock2.
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 05-28-2002

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "PFSimpleAction.h"
#include "IPFTest.h"
#include "..\iparamb2.h"

class PFSimpleTest:	public PFSimpleAction, 
					public IPFTest
{
public:
	// constructor: inherited as a base class constructor
	PFSimpleTest() { _activeIcon() = _trueIcon() = _falseIcon() = NULL; }
	PFExport virtual ~PFSimpleTest();

    	/** @defgroup PFSimpleTest PFSimpleTest.h
    	*  @{
    	*/

	/*! \fn PFExport virtual BaseInterface* GetInterface(Interface_ID id);
	*  \brief From InterfaceServer
	*/
	PFExport virtual BaseInterface* GetInterface(Interface_ID id);
#pragma warning(push)
#pragma warning(disable:4100)
	/*! \fn virtual bool	Proceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd, Object* pSystem, INode* pNode, INode* actionNode, IPFIntegrator* integrator, BitArray& testResult, Tab<float>& testTime) { return false; }
	*  \brief Method must be implemented by the derived class. Has a default virtual implementation to 
	ease PFExport implementation of Clone(...) method. From IPFTest interface.
	*/
	virtual bool	Proceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd, 
							Object* pSystem, INode* pNode, INode* actionNode, IPFIntegrator* integrator, 
							BitArray& testResult, Tab<float>& testTime) { return false; }
#pragma warning(pop)
	/*! \fn PFExport void ProceedStep1(IObject* pCont, Object* pSystem, INode* pNode, INode* actionNode, FPInterface* integrator);
	*  \brief Method must be implemented by the derived class. Has a default virtual implementation to 
	ease PFExport implementation of Clone(...) method.
	*/
	PFExport void ProceedStep1(IObject* pCont, Object* pSystem, INode* pNode, INode* actionNode, FPInterface* integrator);

	/*! \fn PFExport bool ProceedStep2(TimeValue timeStartTick, float timeStartFraction, TimeValue& timeEndTick, float& timeEndFraction, BitArray& testResult, Tab<float>& testTime);
	*  \brief Method must be implemented by the derived class. Has a default virtual implementation to 
	ease PFExport implementation of Clone(...) method.
	*/
	PFExport bool ProceedStep2(TimeValue timeStartTick, float timeStartFraction, TimeValue& timeEndTick, float& timeEndFraction, BitArray& testResult, Tab<float>& testTime);

/*@}*/

protected:
		// const access to class members
		const IObject*		containerFnPub()		const	{ return m_containerFnPub; }
		const Object*		particleSystemFnPub()	const	{ return m_particleSystemFnPub; }
		const INode*		particleNodeFnPub()		const	{ return m_particleNodeFnPub; }
		const INode*		actionNodeFnPub()		const	{ return m_actionNodeFnPub; }
		const FPInterface*	integratorFnPub()		const	{ return m_integratorFnPub; }
		HBITMAP				activeIcon()			const	{ return m_activeIcon; }
		HBITMAP				trueIcon()				const	{ return m_trueIcon; }
		HBITMAP				falseIcon()				const	{ return m_falseIcon; }

		// access to class members
		IObject*&		_containerFnPub()		{ return m_containerFnPub; }
		Object*&		_particleSystemFnPub()	{ return m_particleSystemFnPub; }
		INode*&			_particleNodeFnPub()	{ return m_particleNodeFnPub; }
		INode*&			_actionNodeFnPub()		{ return m_actionNodeFnPub; }
		FPInterface*&	_integratorFnPub()		{ return m_integratorFnPub; }
		HBITMAP&		_activeIcon()			{ return m_activeIcon; }
		HBITMAP&		_trueIcon()				{ return m_trueIcon; }
		HBITMAP&		_falseIcon()			{ return m_falseIcon; }

private:
		// to support FnPub two-step Proceed
		IObject* m_containerFnPub;
		Object* m_particleSystemFnPub;
		INode* m_particleNodeFnPub;
		INode* m_actionNodeFnPub;
		FPInterface* m_integratorFnPub;

		// for custom icons
		HBITMAP m_activeIcon;
		HBITMAP m_trueIcon;
		HBITMAP m_falseIcon;
};


