/*! \file IParticleChannelMXSInteger.h
    \brief Channel-specific interface for ParticleChannelMXSInteger
				 The channel stores integer value for script operators/tests
 				 The channel is a wrap around ParticleChannelInt
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 07-27-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "IParticleChannelInt.h"

// standard particle channel "MXSInteger"
// interface ID
#define PARTICLECHANNELMXSINTEGERR_INTERFACE Interface_ID(0x74f93b14, 0x1eb34500)
#define PARTICLECHANNELMXSINTEGERW_INTERFACE Interface_ID(0x74f93b14, 0x1eb34501)

#define GetParticleChannelMXSIntegerRInterface(obj) ((IParticleChannelIntR*)obj->GetInterface(PARTICLECHANNELMXSINTEGERR_INTERFACE))
#define GetParticleChannelMXSIntegerWInterface(obj) ((IParticleChannelIntW*)obj->GetInterface(PARTICLECHANNELMXSINTEGERW_INTERFACE))


