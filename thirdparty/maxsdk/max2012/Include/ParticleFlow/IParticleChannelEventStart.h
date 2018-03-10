/*! \file IParticleChannelEventStart.h
    \brief Channel-specific interfaces for ParticleChannelEventStart
				 The channel is used to store a moment of entering current Event
				 The channel is a wrap around ParticleChannelPTV
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 11-30-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "IParticleChannelPTV.h"

// standard particle channel "EventStart"
// interface ID
#define PARTICLECHANNELEVENTSTARTR_INTERFACE Interface_ID(0x74f93b05, 0x1eb34500)
#define PARTICLECHANNELEVENTSTARTW_INTERFACE Interface_ID(0x74f93b05, 0x1eb34501)

#define GetParticleChannelEventStartRInterface(obj) ((IParticleChannelPTVR*)obj->GetInterface(PARTICLECHANNELEVENTSTARTR_INTERFACE))
#define GetParticleChannelEventStartWInterface(obj) ((IParticleChannelPTVW*)obj->GetInterface(PARTICLECHANNELEVENTSTARTW_INTERFACE))



