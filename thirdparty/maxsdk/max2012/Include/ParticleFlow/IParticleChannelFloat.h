/*! \file IParticleChannelFloat.h
    \brief	Channel-generic interfaces for particle channels that store float data.
*/
/**********************************************************************
 *<
	CREATED BY:		Chung-An Lin

	HISTORY:		created 01-24-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "..\ifnpub.h"

// generic particle channel "Float"
// interface ID
#define PARTICLECHANNELFLOATR_INTERFACE Interface_ID(0x74f93c00, 0x1eb34500)
#define PARTICLECHANNELFLOATW_INTERFACE Interface_ID(0x74f93c00, 0x1eb34501)

// since it's a "type" channel there is no "GetChannel" defines
//#define GetParticleChannelFloatRInterface(obj) ((IParticleChannelFloatR*)obj->GetInterface(PARTICLECHANNELFLOATR_INTERFACE))
//#define GetParticleChannelFloatWInterface(obj) ((IParticleChannelFloatW*)obj->GetInterface(PARTICLECHANNELFLOATW_INTERFACE))


class IParticleChannelFloatR : public FPMixinInterface
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

	FN_1(kGetValue, TYPE_FLOAT, GetValue, TYPE_INT);
	FN_0(kIsGlobal, TYPE_bool, IsGlobal);
	FN_0(kGetValueGlobal, TYPE_FLOAT, GetValue);

	END_FUNCTION_MAP

      /** @defgroup IParticleChannelFloat IParticleChannelFloat.h
      *  @{
      */

	/*! \fn virtual float	GetValue(int index) const = 0;
	*  \brief get property for particle with index
	*/
	virtual float	GetValue(int index) const = 0;

	/*! \fn virtual bool	IsGlobal() const = 0;
	*  \brief verify if the channel is global
	*/
	virtual bool	IsGlobal() const = 0;

	/*! \fn virtual float	GetValue() const = 0;
	*  \brief if channel is global returns the global value. if channel is not global returns value of the first particle.
	*/
	virtual float	GetValue() const = 0;

	/*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELFLOATR_INTERFACE); }
	*  \brief 
	*/
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELFLOATR_INTERFACE); }
};

class IParticleChannelFloatW : public FPMixinInterface
{
public:

	// function IDs Write
	enum {	kSetValue,
			kSetValueGlobal,
	};

	// Function Map for Function Publish System
	//***********************************
	BEGIN_FUNCTION_MAP

	VFN_2(kSetValue, SetValue, TYPE_INT, TYPE_FLOAT);
	VFN_1(kSetValueGlobal, SetValue, TYPE_FLOAT);

	END_FUNCTION_MAP

      /** @defgroup IParticleChannelFloat IParticleChannelFloat.h
      *  @{
      */

	/*! \fn virtual void	SetValue(int index, float value) = 0;
	*  \brief  set property for particle with index
	*/
	virtual void	SetValue(int index, float value) = 0;

	/*! \fn virtual void	SetValue(float value) = 0;
	*  \brief set property for all particles at once thus making the channel global
	*/
	virtual void	SetValue(float value) = 0;

	/*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELFLOATW_INTERFACE); }
	*  \brief 
	*/
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELFLOATW_INTERFACE); }
};

