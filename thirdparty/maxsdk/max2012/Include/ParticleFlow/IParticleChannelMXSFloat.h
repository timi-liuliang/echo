/*! \file IParticleChannelMXSFloat.h
    \brief Channel-specific interface for ParticleChannelMXSFloat
				 The channel stores float value for script operators/tests
 				 The channel is a wrap around ParticleChannelFloat
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 07-27-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "IParticleChannelFloat.h"

// standard particle channel "MXSFloat"
// interface ID
#define PARTICLECHANNELMXSFLOATR_INTERFACE Interface_ID(0x74f93b15, 0x1eb34500)
#define PARTICLECHANNELMXSFLOATW_INTERFACE Interface_ID(0x74f93b15, 0x1eb34501)

#define GetParticleChannelMXSFloatRInterface(obj) ((IParticleChannelFloatR*)obj->GetInterface(PARTICLECHANNELMXSFLOATR_INTERFACE))
#define GetParticleChannelMXSFloatWInterface(obj) ((IParticleChannelFloatW*)obj->GetInterface(PARTICLECHANNELMXSFLOATW_INTERFACE))


