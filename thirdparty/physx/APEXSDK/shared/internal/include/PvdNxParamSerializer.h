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

#ifndef PVD_NXPARAM_SERIALIZER
#define PVD_NXPARAM_SERIALIZER
#include "foundation/PxSimpleTypes.h"
#include "NxParameterized.h"

namespace PVD
{
class PvdDataStream;
}

namespace NxParameterized
{
class Interface;
}

namespace PvdNxParamSerializer
{

NxParameterized::ErrorType
traverseParamDefTree(NxParameterized::Interface& obj,
                     PVD::PvdDataStream* remoteDebugger,
                     void* curPvdObj,
                     NxParameterized::Handle& handle);

}; // namespacePvdNxParamSerializer

#endif // #ifndef PVD_NXPARAM_SERIALIZER

#endif