/*! \file IParticleChannelOrientation.h
    \brief Channel-specific interface for ParticleChannelOrientation
				 Particle orientation is stored in quaternion form
				  The channel is a wrap around ParticleChannelQuat
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 01-09-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "IParticleChannelQuat.h"

// standard particle channel "Orientation"
// interface ID
#define PARTICLECHANNELORIENTATIONR_INTERFACE Interface_ID(0x74f93b09, 0x1eb34500)
#define PARTICLECHANNELORIENTATIONW_INTERFACE Interface_ID(0x74f93b09, 0x1eb34501)

#define GetParticleChannelOrientationRInterface(obj) ((IParticleChannelQuatR*)obj->GetInterface(PARTICLECHANNELORIENTATIONR_INTERFACE))
#define GetParticleChannelOrientationWInterface(obj) ((IParticleChannelQuatW*)obj->GetInterface(PARTICLECHANNELORIENTATIONW_INTERFACE))


