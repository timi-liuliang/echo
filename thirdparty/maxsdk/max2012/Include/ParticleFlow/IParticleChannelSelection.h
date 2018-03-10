/*! \file IParticleChannelSelection.h
    \brief Channel-specific interfaces for ParticleChannelSelection
				 The channel is used to track selection status of particles
				 The channel is a wrap around ParticleChannelBool
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 02-19-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "IParticleChannelBool.h"

// standard particle channel "Selection"
// interface ID
#define PARTICLECHANNELSELECTIONR_INTERFACE Interface_ID(0x74f93b13, 0x1eb34500)
#define PARTICLECHANNELSELECTIONW_INTERFACE Interface_ID(0x74f93b13, 0x1eb34501)

#define GetParticleChannelSelectionRInterface(obj) ((IParticleChannelBoolR*)obj->GetInterface(PARTICLECHANNELSELECTIONR_INTERFACE))
#define GetParticleChannelSelectionWInterface(obj) ((IParticleChannelBoolW*)obj->GetInterface(PARTICLECHANNELSELECTIONW_INTERFACE))


