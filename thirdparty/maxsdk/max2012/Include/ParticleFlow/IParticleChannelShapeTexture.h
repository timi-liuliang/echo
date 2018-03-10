/*! \file IParticleChannelShapeTexture.h
    \brief Channel-specific interfaces for ParticleChannelShapeTexture
				 The channel is used to store particle mapping
				 The channel is a wrap around ParticleChannelMeshMap
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 06-21-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "IParticleChannelMeshMap.h"

// standard particle channel "Shape Texture"
// interface ID
#define PARTICLECHANNELSHAPETEXTURER_INTERFACE Interface_ID(0x74f93b10, 0x1eb34500)
#define PARTICLECHANNELSHAPETEXTUREW_INTERFACE Interface_ID(0x74f93b10, 0x1eb34501)

#define GetParticleChannelShapeTextureRInterface(obj) ((IParticleChannelMeshMapR*)obj->GetInterface(PARTICLECHANNELSHAPETEXTURER_INTERFACE))
#define GetParticleChannelShapeTextureWInterface(obj) ((IParticleChannelMeshMapW*)obj->GetInterface(PARTICLECHANNELSHAPETEXTUREW_INTERFACE))


