/*! \file IParticleChannelMXSMatrix.h
    \brief Channel-specific interface for ParticleChannelMXSMatrix
				 The channel stores Matrix3 value for script operators/tests
 				 The channel is a wrap around ParticleChannelMatrix3
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 07-27-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "IParticleChannelMatrix3.h"

// standard particle channel "MXSMatrix"
// interface ID
#define PARTICLECHANNELMXSMATRIXR_INTERFACE Interface_ID(0x74f93b17, 0x1eb34500)
#define PARTICLECHANNELMXSMATRIXW_INTERFACE Interface_ID(0x74f93b17, 0x1eb34501)

#define GetParticleChannelMXSMatrixRInterface(obj) ((IParticleChannelMatrix3R*)obj->GetInterface(PARTICLECHANNELMXSMATRIXR_INTERFACE))
#define GetParticleChannelMXSMatrixWInterface(obj) ((IParticleChannelMatrix3W*)obj->GetInterface(PARTICLECHANNELMXSMATRIXW_INTERFACE))


