/*! \file IParticleChannelBool.h
    \brief	Channel-generic interfaces for particle channels that store bool data.
*/
/**********************************************************************
 *<
	CREATED BY:		Chung-An Lin

	HISTORY:		created 02-20-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "..\ifnpub.h"

// generic particle channel "bool"
// interface ID
#define PARTICLECHANNELBOOLR_INTERFACE Interface_ID(0x74f93c04, 0x1eb34500)
#define PARTICLECHANNELBOOLW_INTERFACE Interface_ID(0x74f93c04, 0x1eb34501)

// since it's a "type" channel there is no "GetChannel" defines
//#define GetParticleChannelBoolRInterface(obj) ((IParticleChannelBoolR*)obj->GetInterface(PARTICLECHANNELBOOLR_INTERFACE))
//#define GetParticleChannelBoolWInterface(obj) ((IParticleChannelBoolW*)obj->GetInterface(PARTICLECHANNELBOOLW_INTERFACE))


class IParticleChannelBoolR : public FPMixinInterface
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

	FN_1(kGetValue, TYPE_bool, GetValue, TYPE_INT);
	FN_0(kIsGlobal, TYPE_bool, IsGlobal);
	FN_0(kGetValueGlobal, TYPE_bool, GetValue);

	END_FUNCTION_MAP

      /** @defgroup IParticleChannelBool IParticleChannelBool.h
      *  @{
      */

	/*! \fn virtual bool	GetValue(int index) const = 0;
	*  \brief get property for particle with index
	*/
	virtual bool	GetValue(int index) const = 0;

	/*! \fn virtual bool	IsGlobal() const = 0;
	*  \brief verify if the channel is global
	*/
	virtual bool	IsGlobal() const = 0;

	/*! \fn virtual bool	GetValue() const = 0;
	*  \brief if channel is global returns the global value. if channel is not global returns value of the first particle
	*/
	virtual bool	GetValue() const = 0;

	/*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELBOOLR_INTERFACE); }
	*  \brief 
	*/
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELBOOLR_INTERFACE); }
};

class IParticleChannelBoolW : public FPMixinInterface
{
public:

	// function IDs Write
	enum {	kSetValue,
			kSetValueGlobal,
	};

	// Function Map for Function Publish System
	//***********************************
	BEGIN_FUNCTION_MAP

	VFN_2(kSetValue, SetValue, TYPE_INT, TYPE_bool);
	VFN_1(kSetValueGlobal, SetValue, TYPE_bool);

	END_FUNCTION_MAP

      /** @defgroup IParticleChannelBool IParticleChannelBool.h
      *  @{
      */

	/*! \fn virtual void	SetValue(int index, bool value) = 0;
	*  \brief set property for particle with index
	*/
	virtual void	SetValue(int index, bool value) = 0;

	/*! \fn virtual void	SetValue(bool value) = 0;
	*  \brief set property for all particles at once thus making the channel global
	*/
	virtual void	SetValue(bool value) = 0;

	/*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELBOOLW_INTERFACE); }
	*  \brief 
	*/
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELBOOLW_INTERFACE); }
};

