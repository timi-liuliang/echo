/*! \file IPFIntegrator.h
    \brief Interface for time-integrating PF Operator
				 PF has a default implementation of such interface
				 The interface is used to "advance" particles in time
				 according to some rules. The default implementation
				 uses classical mechanics physics rules for a body
				 Position and speed is calculated according to
				 the current speed and acceleration. That applies to
				 linear and angular movements.
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

// interface ID
#define PFINTEGRATOR_INTERFACE Interface_ID(0x74f93d03, 0x1eb34500) 

#define GetPFIntegratorInterface(obj) ((IPFIntegrator*)obj->GetInterface(PFINTEGRATOR_INTERFACE)) 

// function IDs
enum {	kPFIntegrator_proceedSync,
		kPFIntegrator_proceedASync
}; 

class IPFIntegrator : public FPMixinInterface
{

BEGIN_FUNCTION_MAP
FN_5(kPFIntegrator_proceedSync, TYPE_bool, ProceedSync, TYPE_IOBJECT, TYPE_TIMEVALUE, TYPE_FLOAT, TYPE_bool, TYPE_BITARRAY);
FN_5(kPFIntegrator_proceedASync, TYPE_bool, ProceedASync, TYPE_IOBJECT, TYPE_TIMEVALUE_TAB, TYPE_FLOAT_TAB, TYPE_bool, TYPE_BITARRAY);

END_FUNCTION_MAP

public:

    /** @defgroup IPFIntegrator IPFIntegrator.h
    *  @{
    */

    /*! \fn virtual bool	Proceed(IObject* pCont, PreciseTimeValue time, int index) = 0;
    *  \brief Returns true if the operation has been proceeded successfully
	 Proceed a single particle with the given index to the given time.
	 If pCont is NULL then the method works with previously given container.
	 It will expedite the call since the method won't acquire particle channels
	 from the container but uses the previously acquired channels.
	 When working with individual particles from the same container, the good
	 practice is to make the call for the first particle with the container specified,
	 and then for all other particles with the container set to NULL.
	 \param pCont: particleContainer
	 \param time: the time for particles to come to; each particle may have its
	 				own current valid time; the parameter sets the time for all particles to synchronize to
 	 \param times: time value for a particle to come to; particles may have different time values to come to
 	 \param selected: bitArray to define which particles to proceed
    */
	virtual bool	Proceed(IObject* pCont, PreciseTimeValue time, int index) = 0;

    /*! \fn virtual bool	Proceed(IObject* pCont, PreciseTimeValue time) = 0;
    *  \brief Proceed all particles in the container to the given time
    */
	virtual bool	Proceed(IObject* pCont, PreciseTimeValue time) = 0;

    /*! \fn virtual bool	Proceed(IObject* pCont, Tab<PreciseTimeValue>& times) = 0;
    *  \brief Proceed all particles in the container to the given times (maybe different for each particle)
    */
	virtual bool	Proceed(IObject* pCont, Tab<PreciseTimeValue>& times) = 0;

    /*! \fn virtual bool	Proceed(IObject* pCont, PreciseTimeValue time, BitArray& selected) = 0;
    *  \brief Proceed selected particles only to the given time
    */
	virtual bool	Proceed(IObject* pCont, PreciseTimeValue time, BitArray& selected) = 0;

    /*! \fn virtual bool	Proceed(IObject* pCont, Tab<PreciseTimeValue>& times, BitArray& selected) = 0;
    *  \brief Proceed selected particles only to the given times (maybe different for each particle)
    */
	virtual bool	Proceed(IObject* pCont, Tab<PreciseTimeValue>& times, BitArray& selected) = 0;

    /*! \fn PFExport bool ProceedSync(IObject* pCont, TimeValue timeTick, float timeFraction,
								bool isSelected, BitArray* selected);
    *  \brief "function-publishing" hook-up for the methods above
	 \param pCont: particleContainer
	 \param timeTick, timeFraction, timeTicks, timeFractions:
					time for particles to come to. All particles may have the same time value to
					come to, in that case the method with timeTick and timeFraction have to
					used. Sum of timeTick and timeFraction define the exact time
					value (without TimeValue granularity). If particles have different time values
					to come to than use the second method with Tabs of timeTicks and timeFractions
	 \param isSelected, selected: if only part of particles need to be time-advanced then
					isSelected is true, and "selected" BitArray have to be defined
    */
	PFExport bool ProceedSync(IObject* pCont, TimeValue timeTick, float timeFraction,
								bool isSelected, BitArray* selected);

    /*! \fn PFExport bool ProceedASync(IObject* pCont, Tab<TimeValue> *timeTicks, Tab<float> *timeFractions,
								bool isSelected, BitArray* selected);
    *  \brief "function-publishing" hook-up for the methods above
	 \param pCont: particleContainer
	 \param timeTick, timeFraction, timeTicks, timeFractions:
					time for particles to come to. All particles may have the same time value to
					come to, in that case the method with timeTick and timeFraction have to
					used. Sum of timeTick and timeFraction define the exact time
					value (without TimeValue granularity). If particles have different time values
					to come to than use the second method with Tabs of timeTicks and timeFractions
	 \param isSelected, selected: if only part of particles need to be time-advanced then
					isSelected is true, and "selected" BitArray have to be defined
    */
	PFExport bool ProceedASync(IObject* pCont, Tab<TimeValue> *timeTicks, Tab<float> *timeFractions,
								bool isSelected, BitArray* selected);

    /*! \fn virtual IPFIntegrator* GetEncapsulatedIntegrator() { return NULL; }
    *  \brief An integrator may rely on another integrator to calculation some particle proceedings.
	 For example, a Collision test has IPFIntegrator interface to proceed particles according
	 to collision approximations. At the same time a Collision test doesn't proceed angular
	 velocity of a particle. For that the test relies on encapsulated standard integrator.
    */
	virtual IPFIntegrator* GetEncapsulatedIntegrator() { return NULL; }

    /*! \fn PFExport bool HasEncapsulatedIntegrator(IPFIntegrator* integrator);
    *  \brief Check if the current integrator has the given integrator encapsulated inside
	 the encapsulated integrator can be more than one level down
    */
	PFExport bool HasEncapsulatedIntegrator(IPFIntegrator* integrator);

    /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PFINTEGRATOR_INTERFACE); }
    *  \brief 
    */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PFINTEGRATOR_INTERFACE); }

    /*! \fn friend PFExport IPFIntegrator* GetPFIntegrator();
    *  \brief Friend function declarations
    */
	friend PFExport IPFIntegrator* GetPFIntegrator();

    /*! \fn friend PFExport void ReleasePFIntegrator(IPFIntegrator*);
    *  \brief Friend function declarations
    */
	friend PFExport void ReleasePFIntegrator(IPFIntegrator*);

protected:
	static Tab<IObject*> m_objs;
};
// this method gets a system PF Integrator interface
PFExport IPFIntegrator* GetPFIntegrator();
// when the developer is done with the PFIntegrator interface acquired via GetPFIntegrator() 
// they should call this method to release it.
PFExport void ReleasePFIntegrator(IPFIntegrator* integrator);


