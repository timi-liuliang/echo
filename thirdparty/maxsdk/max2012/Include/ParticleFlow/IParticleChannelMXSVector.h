/*! \file IParticleChannelMXSVector.h
    \brief Channel-specific interface for ParticleChannelMXSVector
				 The channel stores Point3 value for script operators/tests
 				 The channel is a wrap around ParticleChannelPoint3
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 07-27-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "IParticleChannelPoint3.h"

// standard particle channel "MXSVector"
// interface ID
#define PARTICLECHANNELMXSVECTORR_INTERFACE Interface_ID(0x74f93b16, 0x1eb34500)
#define PARTICLECHANNELMXSVECTORW_INTERFACE Interface_ID(0x74f93b16, 0x1eb34501)

#define GetParticleChannelMXSVectorRInterface(obj) ((IParticleChannelPoint3R*)obj->GetInterface(PARTICLECHANNELMXSVECTORR_INTERFACE))
#define GetParticleChannelMXSVectorWInterface(obj) ((IParticleChannelPoint3W*)obj->GetInterface(PARTICLECHANNELMXSVECTORW_INTERFACE))


