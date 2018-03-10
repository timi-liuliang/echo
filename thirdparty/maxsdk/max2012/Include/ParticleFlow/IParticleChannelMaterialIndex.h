/*! \file IParticleChannelMaterialIndex.h
    \brief Channel-specific interfaces for ParticleChannelMaterialIndex
				 The channel is used to store material index information
				 The channel is a wrap around ParticleChannelInt
				 If the channel is present in the particle container
				 then the index information overrides any material index information
				 in mesh channel with one exception: if index value is -1 then
				 the mesh channel information is kept intact.
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 01-23-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "IParticleChannelInt.h"

// standard particle channel "MaterialIndex"
// interface ID
#define PARTICLECHANNELMTLINDEXR_INTERFACE Interface_ID(0x74f93b12, 0x1eb34500)
#define PARTICLECHANNELMTLINDEXW_INTERFACE Interface_ID(0x74f93b12, 0x1eb34501)

#define GetParticleChannelMtlIndexRInterface(obj) ((IParticleChannelIntR*)obj->GetInterface(PARTICLECHANNELMTLINDEXR_INTERFACE))
#define GetParticleChannelMtlIndexWInterface(obj) ((IParticleChannelIntW*)obj->GetInterface(PARTICLECHANNELMTLINDEXW_INTERFACE))


