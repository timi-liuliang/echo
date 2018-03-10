/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_TRAITS_INTERNAL_H
#define NX_TRAITS_INTERNAL_H

#include "NxParameterizedTraits.h"
#include "PsString.h"

namespace NxParameterized
{

PX_PUSH_PACK_DEFAULT

#ifndef WITHOUT_APEX_SERIALIZATION
#if !defined(PX_PS4)
#pragma warning(disable:4127) // conditional expression is constant
#endif
#define NX_PARAM_TRAITS_WARNING(_traits, _format, ...) do { \
		char _tmp[256]; \
		physx::string::sprintf_s(_tmp, sizeof(_tmp), _format, ##__VA_ARGS__); \
		_traits->traitsWarn(_tmp); \
	} while(0)

#else
#define NX_PARAM_TRAITS_WARNING(...)
#endif

// Determines how default converter handles included references
struct RefConversionMode
{
	enum Enum
	{
		// Simply move it to new object (and remove from old)
		REF_CONVERT_COPY = 0,

		// Same as REF_CONVERT_COPY but also update legacy references
		REF_CONVERT_UPDATE,

		// Skip references
		REF_CONVERT_SKIP,

		REF_CONVERT_LAST
	};
};

// Specifies preferred version for element at position specified in longName
struct PrefVer
{
	const char *longName;
	physx::PxU32 ver;
};

// A factory function to create an instance of default conversion.
Conversion *internalCreateDefaultConversion(Traits *traits, const PrefVer *prefVers = 0, RefConversionMode::Enum refMode = RefConversionMode::REF_CONVERT_COPY);

PX_POP_PACK

};

#endif