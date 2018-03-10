/*! \file IParticleChannelAngAxis.h
    \brief Channel-generic interfaces for particle channels that
					store angle/axis data.
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

// generic particle channel "AngAxis"
// interface ID
#define PARTICLECHANNELANGAXISR_INTERFACE Interface_ID(0x74f93c0d, 0x1eb34500)
#define PARTICLECHANNELANGAXISW_INTERFACE Interface_ID(0x74f93c0d, 0x1eb34501)

// since it's a "type" channel there is no "GetChannel" defines
//#define GetParticleChannelAngAxisRInterface(obj) ((IParticleChannelAngAxisR*)obj->GetInterface(PARTICLECHANNELANGAXISR_INTERFACE))
//#define GetParticleChannelAngAxisWInterface(obj) ((IParticleChannelAngAxisW*)obj->GetInterface(PARTICLECHANNELANGAXISW_INTERFACE))


class IParticleChannelAngAxisR : public FPMixinInterface
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

	FN_1(kGetValue, TYPE_ANGAXIS_BR, GetValue, TYPE_INT);
	FN_0(kIsGlobal, TYPE_bool, IsGlobal);
	FN_0(kGetValueGlobal, TYPE_ANGAXIS_BR, GetValue);

	END_FUNCTION_MAP

      /** @defgroup IParticleChannelAngAxis IParticleChannelAngAxis.h
      *  @{
      */

      /*! \fn virtual const AngAxis&	GetValue(int index) const = 0;
      *  \brief Get property for particle with index
      */
	virtual const AngAxis&	GetValue(int index) const = 0;

      /*! \fn virtual bool			IsGlobal() const = 0;
      *  \brief Verify if the channel is global
      */
	virtual bool			IsGlobal() const = 0;

      /*! \fn virtual const AngAxis&	GetValue() const = 0;
      *  \brief If channel is global returns the global value. If channel is not global returns value of the first particle.
	   Returns bounding box for all particles.
      */
	virtual const AngAxis&	GetValue() const = 0;

      /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELANGAXISR_INTERFACE); }
      *  \brief 
      */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELANGAXISR_INTERFACE); }
};

class IParticleChannelAngAxisW : public FPMixinInterface
{
public:

	// function IDs Write
	enum {	kSetValue,
			kSetValueGlobal,
	};

	// Function Map for Function Publish System
	//***********************************
	BEGIN_FUNCTION_MAP

	VFN_2(kSetValue, SetValue, TYPE_INT, TYPE_ANGAXIS_BR);
	VFN_1(kSetValueGlobal, SetValue, TYPE_ANGAXIS_BR);

	END_FUNCTION_MAP

      /** @defgroup IParticleChannelAngAxis IParticleChannelAngAxis.h
      *  @{
      */

      /*! \fn virtual void	SetValue(int index, const AngAxis& v) = 0;
      *  \brief Set property for particle with index
      */
	virtual void	SetValue(int index, const AngAxis& v) = 0;

      /*! \fn virtual void	SetValue(const AngAxis& v) = 0;
      *  \brief Set property for all particles at once thus making the channel global
      */
	virtual void	SetValue(const AngAxis& v) = 0;

      /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELANGAXISW_INTERFACE); }
      *  \brief 
      */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELANGAXISW_INTERFACE); }
};

