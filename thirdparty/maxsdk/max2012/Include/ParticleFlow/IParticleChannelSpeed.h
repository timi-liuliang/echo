/*! \file IParticleChannelSpeed.h
    \brief Channel-specific interface for ParticleChannelSpeed.
				 Particle speed is stored in absolute world coordianates.
				 The speed is in units per tick.
 				  The channel is a wrap around ParticleChannelPoint3.
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 10-19-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "IParticleChannelPoint3.h"

// standard particle channel "Speed"
// interface ID
#define PARTICLECHANNELSPEEDR_INTERFACE Interface_ID(0x74f93b07, 0x1eb34500) 
#define PARTICLECHANNELSPEEDW_INTERFACE Interface_ID(0x74f93b07, 0x1eb34501) 

#define GetParticleChannelSpeedRInterface(obj) ((IParticleChannelPoint3R*)obj->GetInterface(PARTICLECHANNELSPEEDR_INTERFACE)) 
#define GetParticleChannelSpeedWInterface(obj) ((IParticleChannelPoint3W*)obj->GetInterface(PARTICLECHANNELSPEEDW_INTERFACE)) 


