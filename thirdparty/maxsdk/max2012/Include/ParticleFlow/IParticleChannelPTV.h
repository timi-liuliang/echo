/*! \file IParticleChannelPTV.h
    \brief Channel-generic interfaces for particle channels that keep time data.
				 For precision reasons the time is kept in two
				 components: TimeValue FrameTime and float FractionTime ]-0.5, 0.5]
				 The result time is a sum of these two components.
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 11-29-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "..\ifnpub.h"
#include "PreciseTimeValue.h"

// generic particle channel "PTV" : PreciseTimeValue
// interface ID
#define PARTICLECHANNELPTVR_INTERFACE Interface_ID(0x74f93c07, 0x1eb34500) 
#define PARTICLECHANNELPTVW_INTERFACE Interface_ID(0x74f93c07, 0x1eb34501) 

// since it's a "type" channel there is no "GetChannel" defines
//#define GetParticleChannelPTVRInterface(obj) ((IParticleChannelPTVR*)obj->GetInterface(PARTICLECHANNELPTVR_INTERFACE)) 
//#define GetParticleChannelPTVWInterface(obj) ((IParticleChannelPTVW*)obj->GetInterface(PARTICLECHANNELPTVW_INTERFACE)) 


class IParticleChannelPTVR : public FPMixinInterface
{
public:

	// function IDs Read
	enum {	kGetTick, 
			kGetFraction,
			kGetTickFraction,
			kIsSync
	}; 

	// Function Map for Function Publish System 
	//***********************************
	BEGIN_FUNCTION_MAP

	FN_1(kGetTick, TYPE_TIMEVALUE, GetTick, TYPE_INT);
	FN_1(kGetFraction, TYPE_FLOAT, GetFraction, TYPE_INT);
	VFN_3(kGetTickFraction, GetTickFraction, TYPE_INT, TYPE_TIMEVALUE_BR, TYPE_FLOAT_BR);
	FN_0(kIsSync, TYPE_bool, IsSync);

	END_FUNCTION_MAP

    /** @defgroup IParticleChannelPTV IParticleChannelPTV.h
    *  @{
    */

    /*! \fn virtual TimeValue	GetTick(int index) const = 0;
    *  \brief Get particle property
    */
	virtual TimeValue	GetTick(int index) const = 0;

    /*! \fn virtual float		GetFraction(int index) const = 0;
    *  \brief Returns true if "index" particle exists
    */
	virtual float		GetFraction(int index) const = 0;

    /*! \fn virtual void		GetTickFraction(int index, TimeValue& tick, float& fraction) const = 0;
    *  \brief 
    */
	virtual void		GetTickFraction(int index, TimeValue& tick, float& fraction) const = 0;

    /*! \fn virtual bool		IsSync() const = 0; 
    *  \brief Returns true if all particles have the same time
    */
	virtual bool		IsSync() const = 0;  
	
    /*! \fn virtual const PreciseTimeValue&	GetValue(int index) const = 0;
    *  \brief Methods not in the mapped interface
    */
	virtual const PreciseTimeValue&	GetValue(int index) const = 0;

    /*! \fn virtual const PreciseTimeValue& GetValue() const = 0;
    *  \brief Get synchronized time for all particles; if timing isn't
	synchronized then return time for the first particle in the container
    */
	virtual const PreciseTimeValue& GetValue() const = 0;

    /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELPTVR_INTERFACE); }
    *  \brief 
    */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELPTVR_INTERFACE); }
/*@}*/

};

class IParticleChannelPTVW : public FPMixinInterface
{
public:

	// function IDs Write
	enum {	kSetTick,
			kSetFraction,
			kSetTickFraction,
	}; 


	BEGIN_FUNCTION_MAP

	VFN_2(kSetTick, SetTick, TYPE_INT, TYPE_TIMEVALUE);
	VFN_2(kSetFraction, SetFraction, TYPE_INT, TYPE_FLOAT);
	VFN_3(kSetTickFraction, SetTickFraction, TYPE_INT, TYPE_TIMEVALUE, TYPE_FLOAT);

	END_FUNCTION_MAP

	// set particle property
	virtual void	SetTick(int index, TimeValue time) = 0;
	virtual void	SetFraction(int index, float time) = 0;
	virtual void	SetTickFraction(int index, TimeValue tick, float fraction) = 0;

// methods not in the mapped interface
	virtual void	SetValue(int index, const PreciseTimeValue& time) = 0;
	// set synchronized time for all particles
	virtual void	SetValue(const PreciseTimeValue& time) = 0;

	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELPTVW_INTERFACE); }
};

