/*! \file IParticleChannelShape.h
    \brief Channel-specific interfaces for ParticleChannelShape
				 The channel is used to store particle representation
				 in mesh form for viewports and during render
				 The channel is a wrap around ParticleChannelMesh
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 12-04-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "IParticleChannelMesh.h"

// standard particle channel "Shape"
// interface ID
#define PARTICLECHANNELSHAPER_INTERFACE Interface_ID(0x74f93b0c, 0x1eb34500)
#define PARTICLECHANNELSHAPEW_INTERFACE Interface_ID(0x74f93b0c, 0x1eb34501)

#define GetParticleChannelShapeRInterface(obj) ((IParticleChannelMeshR*)obj->GetInterface(PARTICLECHANNELSHAPER_INTERFACE))
#define GetParticleChannelShapeWInterface(obj) ((IParticleChannelMeshW*)obj->GetInterface(PARTICLECHANNELSHAPEW_INTERFACE))


