/*! \file IParticleChannelScale.h
    \brief Channel-specific interface for ParticleChannelScale
				 The channel stores scaling factor for each particle as
				 Point3 value.
 				 The channel is a wrap around ParticleChannelPoint3
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 01-04-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "IParticleChannelPoint3.h"

// standard particle channel "Scale"
// interface ID
#define PARTICLECHANNELSCALER_INTERFACE Interface_ID(0x74f93b0b, 0x1eb34500)
#define PARTICLECHANNELSCALEW_INTERFACE Interface_ID(0x74f93b0b, 0x1eb34501)

#define GetParticleChannelScaleRInterface(obj) ((IParticleChannelPoint3R*)obj->GetInterface(PARTICLECHANNELSCALER_INTERFACE))
#define GetParticleChannelScaleWInterface(obj) ((IParticleChannelPoint3W*)obj->GetInterface(PARTICLECHANNELSCALEW_INTERFACE))


