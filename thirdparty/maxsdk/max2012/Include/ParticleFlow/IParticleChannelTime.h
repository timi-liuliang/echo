/*! \file IParticleChannelTime.h
    \brief Channel-specific interfaces for ParticleChannelTime.
				 The channel is used to trace current particle time.
				 The channel is a wrap around ParticleChannelPTV.
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 10-04-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "IParticleChannelPTV.h"

// standard particle channel "Time"
// interface ID
#define PARTICLECHANNELTIMER_INTERFACE Interface_ID(0x74f93b03, 0x1eb34500) 
#define PARTICLECHANNELTIMEW_INTERFACE Interface_ID(0x74f93b03, 0x1eb34501) 

#define GetParticleChannelTimeRInterface(obj) ((IParticleChannelPTVR*)obj->GetInterface(PARTICLECHANNELTIMER_INTERFACE)) 
#define GetParticleChannelTimeWInterface(obj) ((IParticleChannelPTVW*)obj->GetInterface(PARTICLECHANNELTIMEW_INTERFACE)) 


