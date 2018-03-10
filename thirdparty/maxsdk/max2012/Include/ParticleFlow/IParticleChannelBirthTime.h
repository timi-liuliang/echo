/*! \file IParticleChannelBirthTime.h
    \brief Channel-specific interfaces for ParticleChannelBirthTime
				 The channel is used to store time of birth for a particle
				 The channel is a wrap around ParticleChannelPTV
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 11-29-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "IParticleChannelPTV.h"

// standard particle channel "BirthTime"
// interface ID
#define PARTICLECHANNELBIRTHTIMER_INTERFACE Interface_ID(0x74f93b04, 0x1eb34500)
#define PARTICLECHANNELBIRTHTIMEW_INTERFACE Interface_ID(0x74f93b04, 0x1eb34501)

#define GetParticleChannelBirthTimeRInterface(obj) ((IParticleChannelPTVR*)obj->GetInterface(PARTICLECHANNELBIRTHTIMER_INTERFACE))
#define GetParticleChannelBirthTimeWInterface(obj) ((IParticleChannelPTVW*)obj->GetInterface(PARTICLECHANNELBIRTHTIMEW_INTERFACE))



