/*! \file IParticleChannelVoid.h
    \brief	Channel-generic interfaces for particle channels that store generic pointers.
*/
/**********************************************************************
 *<
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 07-08-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "..\ifnpub.h"

// generic particle channel "VoidPtr"
// interface ID
#define PARTICLECHANNELVOIDR_INTERFACE Interface_ID(0x74f93c0b, 0x1eb34500)
#define PARTICLECHANNELVOIDW_INTERFACE Interface_ID(0x74f93c0b, 0x1eb34501)

// since it's a "type" channel there is no "GetChannel" defines
//#define GetParticleChannelVoidRInterface(obj) ((IParticleChannelVoidR*)obj->GetInterface(PARTICLECHANNELVOIDR_INTERFACE))
//#define GetParticleChannelVoidWInterface(obj) ((IParticleChannelVoidW*)obj->GetInterface(PARTICLECHANNELVOIDW_INTERFACE))


class IParticleChannelVoidR : public FPMixinInterface
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

	FN_1(kGetValue, TYPE_VALUE, GetValue, TYPE_INT);
	FN_0(kIsGlobal, TYPE_bool, IsGlobal);
	FN_0(kGetValueGlobal, TYPE_VALUE, GetValue);

	END_FUNCTION_MAP

    /** @defgroup IParticleChannelVoid IParticleChannelVoid.h
    *  @{
    */

    /*! \fn virtual void*		GetValue(int index) const = 0;
    *  \brief  get property for particle with index
    */
	virtual void*		GetValue(int index) const = 0;

    /*! \fn virtual bool	IsGlobal() const = 0;
    *  \brief verify if the channel is global
    */
	virtual bool	IsGlobal() const = 0;

    /*! \fn virtual void*		GetValue() const = 0;
    *  \brief if channel is global returns the global value. if channel is not global returns value of the first particle
    */
	virtual void*		GetValue() const = 0;

    /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELVOIDR_INTERFACE); }
    *  \brief 
    */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELVOIDR_INTERFACE); }
};

class IParticleChannelVoidW : public FPMixinInterface
{
public:

	// function IDs Write
	enum {	kSetValue,
			kSetValueGlobal,
	};

	// Function Map for Function Publish System
	//***********************************
	BEGIN_FUNCTION_MAP

	VFN_2(kSetValue, SetValue, TYPE_INT, TYPE_VALUE);
	VFN_1(kSetValueGlobal, SetValue, TYPE_VALUE);

	END_FUNCTION_MAP

    /** @defgroup IParticleChannelVoid IParticleChannelVoid.h
    *  @{
    */

    /*! \fn virtual void	SetValue(int index, void* value) = 0;
    *  \brief set property for particle with index
    */
	virtual void	SetValue(int index, void* value) = 0;

    /*! \fn virtual void	SetValue(void* value) = 0;
    *  \brief set property for all particles at once thus making the channel global
    */
	virtual void	SetValue(void* value) = 0;

    /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELVOIDW_INTERFACE); }
    *  \brief 
    */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELVOIDW_INTERFACE); }
};

