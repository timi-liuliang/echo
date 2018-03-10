/*! \file IParticleChannelPosition.h
    \brief Channel-specific interface for ParticleChannelPosition
				 Particle position is stored in absolute world coordianates
				  The channel is a wrap around ParticleChannelPoint3
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 10-19-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "IParticleChannelPoint3.h"

// standard particle channel "Position"
// interface ID
#define PARTICLECHANNELPOSITIONR_INTERFACE Interface_ID(0x74f93b06, 0x1eb34500)
#define PARTICLECHANNELPOSITIONW_INTERFACE Interface_ID(0x74f93b06, 0x1eb34501)

#define GetParticleChannelPositionRInterface(obj) ((IParticleChannelPoint3R*)obj->GetInterface(PARTICLECHANNELPOSITIONR_INTERFACE))
#define GetParticleChannelPositionWInterface(obj) ((IParticleChannelPoint3W*)obj->GetInterface(PARTICLECHANNELPOSITIONW_INTERFACE))


