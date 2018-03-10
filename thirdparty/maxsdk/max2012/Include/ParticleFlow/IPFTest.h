/*! \file IPFTest.h
    \brief Test-generic interface IPFTest. This is a part of every PF Test.
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 10-12-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "PreciseTimeValue.h"
#include "..\object.h"
// forward declarations
class IPFIntegrator;

PFExport Object* GetPFObject(Object* obj);
// interface ID
#define PFTEST_INTERFACE Interface_ID(0x74f93d02, 0x1eb34500) 
#define GetPFTestInterface(obj) ((IPFTest*)((GetPFObject(obj))->GetInterface(PFTEST_INTERFACE))) 

class IPFTest : public FPMixinInterface
{
public:

	// function IDs
	enum {	kProceedStep1,
			kProceedStep2,
			kGetNextActionList,
			kSetNextActionList,
			kSetLinkActive,
			kClearNextActionList
	}; 

	// Function Map for Function Publish System 
	//***********************************
	BEGIN_FUNCTION_MAP

	VFN_5(kProceedStep1, ProceedStep1, TYPE_IOBJECT, TYPE_OBJECT, TYPE_INODE, TYPE_INODE, TYPE_INTERFACE);
	FN_6(kProceedStep2, TYPE_bool, ProceedStep2, TYPE_TIMEVALUE, TYPE_FLOAT, TYPE_TIMEVALUE_BR, TYPE_FLOAT_BR, TYPE_BITARRAY_BR, TYPE_FLOAT_TAB_BR);
	FN_2(kGetNextActionList, TYPE_INODE, GetNextActionList, TYPE_INODE, TYPE_bool_BP );
	FN_2(kSetNextActionList, TYPE_bool, SetNextActionListMXS, TYPE_INODE, TYPE_INODE);
	FN_2(kSetLinkActive, TYPE_bool, SetLinkActive, TYPE_bool, TYPE_INODE);
	FN_1(kClearNextActionList, TYPE_bool, ClearNextActionList, TYPE_INODE);

	END_FUNCTION_MAP

      /** @defgroup IPFTest IPFTest.h
      *  @{
      */

      /*! \fn PFExport IPFTest();
      *  \brief 
      */
	PFExport IPFTest();

      /*! \fn virtual bool	Proceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd, 
							Object* pSystem, INode* pNode, INode* actionNode, IPFIntegrator* integrator, 
							BitArray& testResult, Tab<float>& testTime) = 0;
      *  \brief Returns true if the test has been proceeded successfully. Verifies the test condition and may change particle channels;
	particle (that satisfy the condition) may be advanced to the time of the condition 
	satisfaction according to the  inertia operator "integrator". If ptr to the integrator 
	is set to NULL then particles are not advanced in time. 
	\param timeStart: the time for a particle to start the simulation. Each particle
					may have its own current valid time greater than timeStart. In this
					case the test should consider the current particle time and 
					timeEnd parameter.
	\param timeEnd: the time for particles to come to; each particle may have its
					own current valid time; the parameter sets the time for all
					particles to synchronize to.
					Sometimes the test may not be able to proceed all the 
    					particles to the given timeEnd. In this case the operator proceeds 
					the particles as far as possible and set the parameter to the 
    					time achieved. All the following actions will be given the new
					timeEnd value as a time value for particles to come to, 
					and the particle system will run another round of action stack 
    					modification in the attempt to proceed all the particles to the 
  					initially set time.
	\param pSystem: the particle system that generates the particle stream; 
					the same operator may get calls from different particle systems; if the 
					result varies according to the particle system that generates
					particles, then this parameter is useful
	\param pNode:	INode of the particle system that generates the particles.
	\param actionNode: INode of the test
	\param integrator: an operator to proceed particles according to "inertia" rules.
					The operator updates time, speed, position, spin, orientation channels
					on the basis of time, acceleration, speed, angular acceleration and
					spin channels. Each particle system has a default inertia operator.
					When an action want to update current time for particles, it
					uses the "inertia" operator. An Action in the ActionList stack my "switch"
					the suggested "inertia" operator by its own if the Action support
					the "inertial" operator interface. The particle system recognizes
					such an Action as a holder of "inertia" interface, and use it
					for "inertia" purposes later on.
					If the parameter is NULL then the particle system doesn't allow 
					the test to advance particles in the time channel
	\param testResult: a bit array to indicate which particles satisfy the test condition
	\param testTime: for particles that satisfy the condition, the list of time moments when
					a particle satisfies the condifion. The tab has the same number of entry as
					particles in the container. Only for particles that satisfy the condition
					the entry is set.
      */
	virtual bool	Proceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd, 
							Object* pSystem, INode* pNode, INode* actionNode, IPFIntegrator* integrator, 
							BitArray& testResult, Tab<float>& testTime) = 0;

      /*! \fn virtual void ProceedStep1(IObject* pCont, Object* pSystem, INode* pNode, INode* actionNode, FPInterface* integrator) = 0;
      *  \brief FnPub alternative to virtual bool Proceed() above. FnPub doesn't allow having more than 7 parameters; therefore the method consists of two parts. The methods
	should be called successively: Step1 and then Step2.
      */
	virtual void ProceedStep1(IObject* pCont, Object* pSystem, INode* pNode, INode* actionNode, FPInterface* integrator) = 0;

      /*! \fn virtual bool ProceedStep2(TimeValue timeStartTick, float timeStartFraction, 
							TimeValue& timeEndTick, float& timeEndFraction, 
							BitArray& testResult, Tab<float>& testTime) = 0;
      *  \brief FnPub alternative to virtual bool Proceed() above. FnPub doesn't allow having more than 7 parameters; therefore the method consists of two parts. The methods
	should be called successively: Step1 and then Step2.
      */
	virtual bool ProceedStep2(TimeValue timeStartTick, float timeStartFraction, 
							TimeValue& timeEndTick, float& timeEndFraction, 
							BitArray& testResult, Tab<float>& testTime) = 0;

      /*! \fn PFExport INode* GetNextActionList(INode* test, bool* linkActive=NULL);
      *  \brief Gets INode of the next ActionList. If particle satisfies the test then it is directed
	to this ActionList. Since a test may have several INode instances with different next events,
	it is necessary to define the INode the method returns activity status of the link to the next ActionList.
      */
	PFExport INode* GetNextActionList(INode* test, bool* linkActive=NULL);

      /*! \fn PFExport bool SetNextActionList(INode* nextAL, INode* test);
      *  \brief Modifies the next ActionList for the test. Returns true if the next actionList has been set successfully.
	The nextAL node can be rejected if it is not real actionList.
      */
	PFExport bool SetNextActionList(INode* nextAL, INode* test);

      /*! \fn PFExport bool SetNextActionListMXS(INode* nextAL, INode* test);
      *  \brief FnPub alternative for the method above; used by maxscript to set the next action list
      */
	PFExport bool SetNextActionListMXS(INode* nextAL, INode* test);

      /*! \fn PFExport bool SetLinkActive(bool active, INode* test);
      *  \brief Modifies activity status for the link to the next ActionList. Returns true if the modification has been done successfully
      */
	PFExport bool SetLinkActive(bool active, INode* test);

      /*! \fn PFExport bool ClearNextActionList(INode* test);
      *  \brief Deletes the wire going to the nextActionList for the test. Returns true if the wire was deleted successfully.
	The test node can be rejected if it is not a real test action.
      */
	PFExport bool ClearNextActionList(INode* test);

      /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PFTEST_INTERFACE); }
      *  \brief 
      */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PFTEST_INTERFACE); }

      /*! \fn inline void updateNextActionListValue(INode* nextAL) { m_nextActionList = nextAL; }
      *  \brief for internal use only
      */
	inline void updateNextActionListValue(INode* nextAL) { m_nextActionList = nextAL; }

      /*! \fn inline void updateArrowValue(Object* arrow) { m_arrow = arrow; }
      *  \brief for internal use only
      */
	inline void updateArrowValue(Object* arrow) { m_arrow = arrow; }

      /*! \fn inline void updateLinkActiveValue(bool linkActive) { m_linkActive = linkActive; }
      *  \brief for internal use only
      */
	inline void updateLinkActiveValue(bool linkActive) { m_linkActive = linkActive; }

protected:
	INode*	m_nextActionList; // an actionList node for the next event.
	Object* m_arrow; // an arrow object that controls the gate to the next ActionList
	bool	m_linkActive; // status of the link to the next ActionList
};

inline IPFTest* PFTestInterface(Object* obj) {
	return ((obj == NULL) ? NULL : GetPFTestInterface(obj));
};

inline IPFTest* PFTestInterface(INode* node) {
	return ((node == NULL) ? NULL : PFTestInterface(node->GetObjectRef()));
};

