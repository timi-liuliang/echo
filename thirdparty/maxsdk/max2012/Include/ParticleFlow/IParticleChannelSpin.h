/*! \file IParticleChannelSpin.h
    \brief Channel-specific interface for ParticleChannelSpin.
				 Particle spin rate is stored in angle/axis form.
				 The rate is in radians per tick.
				  The channel is a wrap around ParticleChannelAngAxis.
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 01-09-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "IParticleChannelAngAxis.h"

// standard particle channel "Spin"
// interface ID
#define PARTICLECHANNELSPINR_INTERFACE Interface_ID(0x74f93b0a, 0x1eb34500) 
#define PARTICLECHANNELSPINW_INTERFACE Interface_ID(0x74f93b0a, 0x1eb34501) 

#define GetParticleChannelSpinRInterface(obj) ((IParticleChannelAngAxisR*)obj->GetInterface(PARTICLECHANNELSPINR_INTERFACE)) 
#define GetParticleChannelSpinWInterface(obj) ((IParticleChannelAngAxisW*)obj->GetInterface(PARTICLECHANNELSPINW_INTERFACE)) 

