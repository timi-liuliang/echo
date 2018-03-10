/*! \file IParticleChannelQuat.h
    \brief Channel-generic interfaces for particle channels that store quaternion data.

*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 01-09-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "..\ifnpub.h"

// generic particle channel "Quat"
// interface ID
#define PARTICLECHANNELQUATR_INTERFACE Interface_ID(0x74f93c0e, 0x1eb34500) 
#define PARTICLECHANNELQUATW_INTERFACE Interface_ID(0x74f93c0e, 0x1eb34501) 

// since it's a "type" channel there is no "GetChannel" defines
//#define GetParticleChannelQuatRInterface(obj) ((IParticleChannelQuatR*)obj->GetInterface(PARTICLECHANNELQUATR_INTERFACE)) 
//#define GetParticleChannelQuatWInterface(obj) ((IParticleChannelQuatW*)obj->GetInterface(PARTICLECHANNELQUATW_INTERFACE)) 


class IParticleChannelQuatR : public FPMixinInterface
{
public:

	// function IDs Read
	enum {	kGetValue,
			kIsGlobal,
			kGetValueGlobal
	}; 

	// Function Map for Function Publish System 
	//***********************************
	BEGIN_FUNCTION_MAP

	FN_1(kGetValue, TYPE_QUAT_BR, GetValue, TYPE_INT);
	FN_0(kIsGlobal, TYPE_bool, IsGlobal);
	FN_0(kGetValueGlobal, TYPE_QUAT_BR, GetValue);

	END_FUNCTION_MAP

    /** @defgroup IParticleChannelQuat IParticleChannelQuat.h
    *  @{
    */

    /*! \fn virtual const Quat&	GetValue(int index) const = 0;
    *  \brief Get property for particle with index
    */
	virtual const Quat&	GetValue(int index) const = 0;

    /*! \fn virtual bool			IsGlobal() const = 0;
    *  \brief Verify if the channel is global
    */
	virtual bool			IsGlobal() const = 0;

    /*! \fn virtual const Quat&	GetValue() const = 0;
    *  \brief If channel is global, returns the global value. If channel is not global, returns value of the first particle.
    */
	virtual const Quat&	GetValue() const = 0;

	// returns bounding box for all particles

    /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELQUATR_INTERFACE); } 
    *  \brief 
    */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELQUATR_INTERFACE); }
};

class IParticleChannelQuatW : public FPMixinInterface
{
public:

	// function IDs Write
	enum {	kSetValue,
			kSetValueGlobal,
	}; 

	// Function Map for Function Publish System 
	//***********************************
	BEGIN_FUNCTION_MAP

	VFN_2(kSetValue, SetValue, TYPE_INT, TYPE_QUAT_BR);
	VFN_1(kSetValueGlobal, SetValue, TYPE_QUAT_BR);

	END_FUNCTION_MAP

    /** @defgroup IParticleChannelQuat IParticleChannelQuat.h
    *  @{
    */

    /*! \fn virtual void	SetValue(int index, const Quat& v) = 0;
    *  \brief Set property for particle with index
    */
	virtual void	SetValue(int index, const Quat& v) = 0;

    /*! \fn virtual void	SetValue(const Quat& v) = 0; 
    *  \brief Set property for all particles at once thus making the channel global
    */
	virtual void	SetValue(const Quat& v) = 0;

    /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELQUATW_INTERFACE); } 
    *  \brief 
    */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELQUATW_INTERFACE); }

/*@}*/

};

