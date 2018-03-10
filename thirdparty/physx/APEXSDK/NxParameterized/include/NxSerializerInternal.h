/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_SERIALIZER_INTERNAL_H
#define NX_SERIALIZER_INTERNAL_H

/*!
\file
\brief NxParameterized serializer factory
*/


#include "NxSerializer.h"
#include "NxParameterizedTraits.h"

namespace NxParameterized
{

PX_PUSH_PACK_DEFAULT

/**
\brief A factory function to create an instance of the Serializer class
\param [in] type serializer type (binary, xml, etc.)
\param [in] traits traits-object to do memory allocation, legacy version conversions, callback calls, etc.
*/
Serializer *internalCreateSerializer(Serializer::SerializeType type, Traits *traits);


// Query the current platform
const SerializePlatform &GetCurrentPlatform();
bool GetPlatform(const char *name, SerializePlatform &platform);
const char *GetPlatformName(const SerializePlatform &platform);


PX_POP_PACK

}; // end of namespace

#endif
