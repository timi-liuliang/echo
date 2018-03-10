/*! \file IParticleChannelDeathTime.h
    \brief Channel-specific interfaces for ParticleChannelDeathTime
				 The channel is used to store time of death for a particle
				 The channel is a wrap around ParticleChannelPTV
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 09-27-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "IParticleChannelPTV.h"

// standard particle channel "DeathTime"
// interface ID
#define PARTICLECHANNELDEATHTIMER_INTERFACE Interface_ID(0x74f93b18, 0x1eb34500)
#define PARTICLECHANNELDEATHTIMEW_INTERFACE Interface_ID(0x74f93b18, 0x1eb34501)

#define GetParticleChannelDeathTimeRInterface(obj) ((IParticleChannelPTVR*)obj->GetInterface(PARTICLECHANNELDEATHTIMER_INTERFACE))
#define GetParticleChannelDeathTimeWInterface(obj) ((IParticleChannelPTVW*)obj->GetInterface(PARTICLECHANNELDEATHTIMEW_INTERFACE))



