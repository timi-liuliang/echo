/*! \file IParticleChannelInt.h
    \brief	Channel-generic interfaces for particle channels that store int data.
*/
/**********************************************************************
 *<
	CREATED BY:		Chung-An Lin

	HISTORY:		created 01-16-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "..\ifnpub.h"

// generic particle channel "Int"
// interface ID
#define PARTICLECHANNELINTR_INTERFACE Interface_ID(0x74f93c01, 0x1eb34500)
#define PARTICLECHANNELINTW_INTERFACE Interface_ID(0x74f93c01, 0x1eb34501)

// since it's a "type" channel there is no "GetChannel" defines
//#define GetParticleChannelIntRInterface(obj) ((IParticleChannelIntR*)obj->GetInterface(PARTICLECHANNELINTR_INTERFACE))
//#define GetParticleChannelIntWInterface(obj) ((IParticleChannelIntW*)obj->GetInterface(PARTICLECHANNELINTW_INTERFACE))


class IParticleChannelIntR : public FPMixinInterface
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

	FN_1(kGetValue, TYPE_INT, GetValue, TYPE_INT);
	FN_0(kIsGlobal, TYPE_bool, IsGlobal);
	FN_0(kGetValueGlobal, TYPE_INT, GetValue);

	END_FUNCTION_MAP

      /** @defgroup IParticleChannelInt IParticleChannelInt.h
      *  @{
      */

      /*! \fn virtual int		GetValue(int index) const = 0;
      *  \brief get property for particle with index
      */
	virtual int		GetValue(int index) const = 0;

      /*! \fn virtual bool	IsGlobal() const = 0;
      *  \brief verify if the channel is global
      */
	virtual bool	IsGlobal() const = 0;

      /*! \fn virtual int		GetValue() const = 0;
      *  \brief if channel is global returns the global value. if channel is not global returns value of the first particle
      */
	virtual int		GetValue() const = 0;

      /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELINTR_INTERFACE); }
      *  \brief 
      */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELINTR_INTERFACE); }
};

class IParticleChannelIntW : public FPMixinInterface
{
public:

	// function IDs Write
	enum {	kSetValue,
			kSetValueGlobal,
	};

	// Function Map for Function Publish System
	//***********************************
	BEGIN_FUNCTION_MAP

	VFN_2(kSetValue, SetValue, TYPE_INT, TYPE_INT);
	VFN_1(kSetValueGlobal, SetValue, TYPE_INT);

	END_FUNCTION_MAP

      /** @defgroup IParticleChannelInt IParticleChannelInt.h
      *  @{
      */

      /*! \fn virtual void	SetValue(int index, int value) = 0;
      *  \brief set property for particle with index
      */
	virtual void	SetValue(int index, int value) = 0;

      /*! \fn virtual void	SetValue(int value) = 0;
      *  \brief set property for all particles at once thus making the channel global
      */
	virtual void	SetValue(int value) = 0;

      /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELINTW_INTERFACE); }
      *  \brief 
      */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELINTW_INTERFACE); }
};

