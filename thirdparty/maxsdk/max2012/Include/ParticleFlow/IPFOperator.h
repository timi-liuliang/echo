/*! \file IPFOperator.h
    \brief Operator-generic interface IPFOperator
				This is a part of every PF Operator
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
#include "..\ifnpub.h"
#include "..\object.h"
// forward declarations
class IPFIntegrator;

PFExport Object* GetPFObject(Object* obj);
// interface ID
#define PFOPERATOR_INTERFACE Interface_ID(0x74f93d01, 0x1eb34500) 
#define GetPFOperatorInterface(obj) ((IPFOperator*)((GetPFObject(obj))->GetInterface(PFOPERATOR_INTERFACE))) 


class IPFOperator : public FPMixinInterface
{
public:

	// function IDs
	enum {	kProceed 
	}; 

	// Function Map for Function Publish System 
	//***********************************
	BEGIN_FUNCTION_MAP

	FN_7(kProceed, TYPE_bool, Proceed, TYPE_IOBJECT, TYPE_TIMEVALUE, TYPE_TIMEVALUE_BR, TYPE_OBJECT, TYPE_INODE, TYPE_INODE, TYPE_INTERFACE);

	END_FUNCTION_MAP

      /** @defgroup IPFOperator IPFOperator.h
      *  @{
      */

      /*! \fn Proceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd, Object* pSystem, INode* pNode, INode* actioNode, IPFIntegrator* integrator) = 0;
      *  \brief Returns true if the operation has been proceeded successfully.
	methods' signatures are presented in two forms: one is compact, and the other one
	is for function-publishing where PreciseTimeValue is presented as a pair (int,float).
	\param pCont: particleContainer timeStart: the time for particle to start from the simulation. Each particle
					may have its own current valid time greater than timeStart. In this
					case the operator should consider the current particle time and 
					timeEnd parameter. Some operators (like Birth) may not consider
					current particle time since it works with timeStart and timeEnd
					parameters only.
	\param timeEnd: the time for particles to come to; each particle may have its
					own current valid time; the parameter sets the time for all
					particles to synchronize to.
					If current particle time is greater than timeEnd it means that the
					particle doesn't require processing at all; the particle has been already 
					updated beyond the interval ]timeStart, timeEnd] of the current operator.
						Sometimes the operator may not be able to proceed all the 
      				particles to the given timeEnd. In this case the operator proceeds 
					the particles as far as possible and set the parameter to the 
      				time achieved. All the following operators will be given the new
					timeEnd value as a time value for particles to come to, 
					and the particle system will run another round of operator stack 
    					modification in the attempt to proceed all the particles to the 
    					initially set time.
	\param pSystem: the particle system that generates the particle stream; 
					the same operator may get calls from different particle systems; if the 
					result varies according to the particle system that generates
					particles, then this parameter is useful.
	\param pNode:INode of the particle system that generates the particles.
	\param actionNode: INode of the operator (if any; can be NULL)
	\param integrator: an operator to proceed particles according to "inertia" rules.
					The operator updates time, speed, position, spin, orientation channels
					on the basis of time, acceleration, speed, angular acceleration and
					spin channels. Each particle system has a default inertia operator.
					When an operator want to update current time for particles, it
					uses the "inertia" operator. An Action in the ActionList stack my "switch"
					the suggested "inertia" operator by its own if the Action support
					the "inertial" operator interface. The particle system recognizes
					such an Action as a holder of "inertia" interface, and use it
					for "inertia" purposes later on.
					If the parameter is NULL then the particle system doesn't allow 
					the operator to advance particles in the time channel.
      */
	virtual bool	Proceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd, Object* pSystem, INode* pNode, INode* actioNode, IPFIntegrator* integrator) = 0;	

      /*! \fn PFExport bool	Proceed(IObject* pCont, TimeValue timeStart, TimeValue& timeEnd, Object* pSystem, INode* pNode, INode* actionNode, FPInterface* integrator);
      *  \brief "function publishing" alternative for the method above
	  */
	PFExport bool	Proceed(IObject* pCont, TimeValue timeStart, TimeValue& timeEnd, Object* pSystem, INode* pNode, INode* actionNode, FPInterface* integrator);
#pragma warning(push)
#pragma warning(disable:4100)
      /*! \fn virtual bool	HasPreProceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd) { return false; }
      *  \brief Three methods apply the Proceed method as a pre-procedure before the
	regular proceed for all operators starts. It is not recommended to modify particle container.
	The call is mostly used to give an operator opportunity to snapshot the state of particle container
	before any operator modifies it.
	The Proceed method is called in between PretProceedBegin and PreProceedEnd methods if 
	HasPreProceed method returns "true".
	Those methods are not supported in MaxScript intentionally.
      */
	virtual bool	HasPreProceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd) { return false; }

      /*! \fn virtual void	PreProceedBegin(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd) { ; }
      *  \brief See virtual bool HasPreProceed(}
      */
	virtual void	PreProceedBegin(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd) { ; }

      /*! \fn virtual void	PreProceedEnd(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd) { ; }
      *  \brief See virtual bool HasPreProceed(}
      */
	virtual void	PreProceedEnd(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd) { ; }

      /*! \fn virtual bool	HasPostProceed(IObject* pCont, PreciseTimeValue time) { return false; }
      *  \brief Three methods apply the Proceed method as a post procedure when the
	amount of particles in all particle groups has been established and there won't be any
	particle transfers from one particle group to another.
	The Proceed method is called again in between PostProceedBegin and PostProceedEnd methods if 
	HasPostProceed method returns "true".
	Since the post proceed methods are applied after everything has established, it is applied to a "time" moment, not
	[timeStart, timeEnt] interval. The Proceed method is called with the same value for timeStart and timeEnd.
	Those methods are not supported in MaxScript intentionally.
      */
	virtual bool	HasPostProceed(IObject* pCont, PreciseTimeValue time) { return false; }

      /*! \fn virtual void	PostProceedBegin(IObject* pCont, PreciseTimeValue time) { ; }
      *  \brief See virtual bool HasPostProceed(}
      */
	virtual void	PostProceedBegin(IObject* pCont, PreciseTimeValue time) { ; }

      /*! \fn virtual void	PostProceedEnd(IObject* pCont, PreciseTimeValue time) { ; }
      *  \brief See virtual bool HasPostProceed(}
      */
	virtual void	PostProceedEnd(IObject* pCont, PreciseTimeValue time) { ; }
#pragma warning(pop)
      /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PFOPERATOR_INTERFACE); }
      *  \brief 
      */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PFOPERATOR_INTERFACE); }

/*@}*/

};

inline IPFOperator* PFOperatorInterface(Object* obj) {
	return ((obj == NULL) ? NULL : GetPFOperatorInterface(obj));
};

inline IPFOperator* PFOperatorInterface(INode* node) {
	return ((node == NULL) ? NULL : PFOperatorInterface(node->GetObjectRef()));
};

