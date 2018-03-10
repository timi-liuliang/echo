/*! \file IParticleChannelLifespan.h
    \brief Channel-specific interfaces for ParticleChannelLifespan.
				 The channel is used to store maximal age a particle can live up to.
				 The channel is a wrap around ParticleChannelPTV.
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 01-24-03

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "IParticleChannelPTV.h"

// standard particle channel "Lifespan"
// interface ID
#define PARTICLECHANNELLIFESPANR_INTERFACE Interface_ID(0x74f93b19, 0x1eb34500) 
#define PARTICLECHANNELLIFESPANW_INTERFACE Interface_ID(0x74f93b19, 0x1eb34501) 

#define GetParticleChannelLifespanRInterface(obj) ((IParticleChannelPTVR*)obj->GetInterface(PARTICLECHANNELLIFESPANR_INTERFACE)) 
#define GetParticleChannelLifespanWInterface(obj) ((IParticleChannelPTVW*)obj->GetInterface(PARTICLECHANNELLIFESPANW_INTERFACE)) 



