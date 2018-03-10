/*! \file IParticleChannelAcceleration.h
    \brief Channel-specific interface for ParticleChannelAcceleration
				 Acceleration is stored in absolute world coordianates
				 The acceleration is in units per tick
				  The channel is a wrap around ParticleChannelPoint3
				  The acceleration values are set to zero (Point3::Origin)
				  after each integration step. Therefore an operator that
				  modifies the channel has to repeat the modification on
				  every 'Proceed' call even if the value to be set is the
				  same.
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 10-19-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "IParticleChannelPoint3.h"

// standard particle channel "Acceleration"
// interface ID
#define PARTICLECHANNELACCELERATIONR_INTERFACE Interface_ID(0x74f93b08, 0x1eb34500)
#define PARTICLECHANNELACCELERATIONW_INTERFACE Interface_ID(0x74f93b08, 0x1eb34501)

#define GetParticleChannelAccelerationRInterface(obj) ((IParticleChannelPoint3R*)obj->GetInterface(PARTICLECHANNELACCELERATIONR_INTERFACE))
#define GetParticleChannelAccelerationWInterface(obj) ((IParticleChannelPoint3W*)obj->GetInterface(PARTICLECHANNELACCELERATIONW_INTERFACE))


