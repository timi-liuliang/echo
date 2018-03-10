/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#if TODO_PVD_NXPARAM_SERIALIZER

#ifndef __PVD_COMM_LAYER_NX_DEBUGGER_STREAM__
#define __PVD_COMM_LAYER_NX_DEBUGGER_STREAM__
#include "PVDCommLayerErrors.h"
#include "PVDCommLayerTypes.h"
#include "foundation/PxVec3.h"
#include "foundation/PxQuat.h"
#include "foundation/PxMat33.h"
#include "PxMat34Legacy.h"

class NxRemoteDebugger;

namespace PVD
{
class PvdConnection;
class PvdDataStream;

inline physx::PxU64 PtrToPVD(void* inPtr)
{
	return static_cast<physx::PxU64>(reinterpret_cast<size_t>(inPtr));
}

PvdCommLayerError CreateObject(PvdDataStream* inConnection, physx::PxU64 inInstanceKey, const char* inTypeName, PVD::EInstanceUIFlags inFlags = PVD::EInstanceUIFlags::None);
PvdCommLayerError SetPropertyValue(PvdDataStream* inConnection, physx::PxU64 inInstanceKey, const PvdCommLayerValue& inValue, bool inCreate, const char* inName);

template< typename TValueType >
inline PvdCommLayerValue CreateCommLayerValue(TValueType inValue)
{
	return PvdCommLayerValue(inValue);
}

template<>
inline PvdCommLayerValue CreateCommLayerValue(const char* inValue)
{
	return PvdCommLayerValue(createString(inValue));
}

template<>
inline PvdCommLayerValue CreateCommLayerValue(physx::PxMat33 inValue)
{
	return PvdCommLayerValue(createMat33(reinterpret_cast<const float*>(&inValue)));
}

template<>
inline PvdCommLayerValue CreateCommLayerValue(physx::PxMat34Legacy inValue)
{
	return PvdCommLayerValue(createFrame(reinterpret_cast<const float*>(&inValue)));
}

}

#endif

#endif

