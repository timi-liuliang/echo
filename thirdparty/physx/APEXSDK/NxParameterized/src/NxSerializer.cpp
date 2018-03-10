/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


/*!
\file
\brief NxParameterized serializer implementation
*/

#include <string.h>
#include <ctype.h>
#include <new> // for placement new

#include "foundation/PxPreprocessor.h"

#include "NxSerializer.h"
#include "XmlSerializer.h"
#include "BinSerializer.h"
#include "NxParameterizedTraits.h"

namespace NxParameterized
{

const SerializePlatform &GetCurrentPlatform()
{
	static bool isInitialized = false;
	static SerializePlatform platform;

	if( isInitialized )
		return platform;

	platform.osVer = SerializePlatform::ANY_VERSION; //Do we need this at all???

	//Determine compiler
#	if defined PX_VC
		platform.compilerType = SerializePlatform::COMP_VC;
		platform.compilerVer = _MSC_VER;
#	elif defined PX_XBOXONE
		platform.compilerType = SerializePlatform::COMP_VC;
		platform.compilerVer = _MSC_VER;
#	elif defined PX_PS4
		platform.compilerType = SerializePlatform::COMP_GCC;
		platform.compilerVer = SerializePlatform::ANY_VERSION;
#	elif defined PX_GNUC
		platform.compilerType =SerializePlatform:: COMP_GCC;
		platform.compilerVer = __GNUC__ << 16 + __GNUC_MINOR__;
#	elif defined PX_CW
		platform.compilerType = SerializePlatform::COMP_MW;
#		error "TODO: define version of Metrowerks compiler"
#	else
#		error "Unknown compiler"
#	endif

	//Determine OS
#	if defined PX_WINDOWS
		platform.osType = SerializePlatform::OS_WINDOWS;
#	elif defined PX_APPLE
		platform.osType = SerializePlatform::OS_MACOSX;
#	elif defined PX_PS3
		platform.osType = SerializePlatform::OS_LV2;
#	elif defined PX_X360
		platform.osType = SerializePlatform::OS_XBOX;
		platform.osVer = _XBOX_VER;
#	elif defined PX_XBOXONE
		platform.osType = SerializePlatform::OS_XBOXONE;
#	elif defined PX_PS4
	platform.osType = SerializePlatform::OS_PS4;
#	elif defined PX_ANDROID
		platform.osType = SerializePlatform::OS_ANDROID;
#	elif defined PX_LINUX
		platform.osType = SerializePlatform::OS_LINUX;
#	else
#		error "Undefined OS"
#	endif

	//Determine arch
#	if defined PX_X86
		platform.archType = SerializePlatform::ARCH_X86;
#	elif defined PX_APPLE
		platform.archType = SerializePlatform::ARCH_X86;
#	elif defined PX_X64
		platform.archType = SerializePlatform::ARCH_X86_64;
#	elif defined PX_PPC
	platform.archType = SerializePlatform::ARCH_PPC;
#	elif defined PX_PS3
	platform.archType = SerializePlatform::ARCH_CELL;
#	elif defined PX_ARM
	platform.archType = SerializePlatform::ARCH_ARM;
#	else
#		error "Unknown architecture"
#	endif

	isInitialized = true;
	return platform;
}

bool GetPlatform(const char *name, SerializePlatform &platform)
{
	platform.osVer = platform.compilerVer = SerializePlatform::ANY_VERSION;

	if( 0 == strcmp("VcXbox", name) || 0 == strcmp("VcXbox360", name))
	{
		platform.archType = SerializePlatform::ARCH_PPC;
		platform.compilerType = SerializePlatform::COMP_VC;
		platform.osType = SerializePlatform::OS_XBOX;
	}
	else if( 0 == strcmp("VcXboxOne", name) )
	{
		platform.archType = SerializePlatform::ARCH_X86_64;
		platform.compilerType = SerializePlatform::COMP_VC;
		platform.osType = SerializePlatform::OS_XBOXONE;
	}
	else if( 0 == strcmp("GccPs4", name) )
	{
		platform.archType = SerializePlatform::ARCH_X86_64;
		platform.compilerType = SerializePlatform::COMP_GCC;
		platform.osType = SerializePlatform::OS_PS4;
	}
	else if( 0 == strcmp("VcWin32", name) )
	{
		platform.archType = SerializePlatform::ARCH_X86;
		platform.compilerType = SerializePlatform::COMP_VC;
		platform.osType = SerializePlatform::OS_WINDOWS;
	}
	else if( 0 == strcmp("VcWin64", name) )
	{
		platform.archType = SerializePlatform::ARCH_X86_64;
		platform.compilerType = SerializePlatform::COMP_VC;
		platform.osType = SerializePlatform::OS_WINDOWS;
	}
	else if( 0 == strcmp("GccPs3", name) )
	{
		platform.archType = SerializePlatform::ARCH_CELL;
		platform.compilerType = SerializePlatform::COMP_GCC;
		platform.osType = SerializePlatform::OS_LV2;
	}
	else if( 0 == strcmp("GccOsX32", name) )
	{
		platform.archType = SerializePlatform::ARCH_X86;
		platform.compilerType = SerializePlatform::COMP_GCC;
		platform.osType = SerializePlatform::OS_MACOSX;
	}
	else if( 0 == strcmp("AndroidARM", name) )
	{
		platform.archType = SerializePlatform::ARCH_ARM;
		platform.compilerType = SerializePlatform::COMP_GCC;
		platform.osType = SerializePlatform::OS_ANDROID;
	}
	else if( 0 == strcmp("GccLinux32", name) )
	{
		platform.archType = SerializePlatform::ARCH_X86;
		platform.compilerType = SerializePlatform::COMP_GCC;
		platform.osType = SerializePlatform::OS_LINUX;
	}
	else if( 0 == strcmp("GccLinux64", name) )
	{
		platform.archType = SerializePlatform::ARCH_X86_64;
		platform.compilerType = SerializePlatform::COMP_GCC;
		platform.osType = SerializePlatform::OS_LINUX;
	}
	else if( 0 == strcmp("Pib", name) ) //Abstract platform for platform-independent serialization
	{
		platform.archType = SerializePlatform::ARCH_GEN;
		platform.compilerType = SerializePlatform::COMP_GEN;
		platform.osType = SerializePlatform::OS_GEN;
	}
	else
		return false;

	return true;
}

const char *GetPlatformName(const SerializePlatform &platform)
{
	static const char *unknown = "<Unknown>";

	switch( platform.osType )
	{
	case SerializePlatform::OS_XBOX:
		return SerializePlatform::COMP_VC == platform.compilerType
			? "VcXbox360" : unknown;

	case SerializePlatform::OS_XBOXONE:
		return SerializePlatform::COMP_VC == platform.compilerType
			? "VcXboxOne" : unknown;

	case SerializePlatform::OS_PS4:
		return SerializePlatform::COMP_GCC == platform.compilerType
			? "GccPs4" : unknown;

	case SerializePlatform::OS_WINDOWS:
		if( SerializePlatform::COMP_VC != platform.compilerType )
			return unknown;

		switch( platform.archType )
		{
		case SerializePlatform::ARCH_X86:
			return "VcWin32";

		case SerializePlatform::ARCH_X86_64:
			return "VcWin64";

		default:
			return unknown;
		}

	case SerializePlatform::OS_MACOSX:
		return SerializePlatform::COMP_GCC == platform.compilerType
			? "GccOsX32" : unknown;

	case SerializePlatform::OS_LV2:
		return SerializePlatform::COMP_GCC == platform.compilerType
			? "GccPs3" : unknown;

	case SerializePlatform::OS_GEN:
		return "Pib";

	case SerializePlatform::OS_ANDROID:
		return SerializePlatform::ARCH_ARM == platform.archType
			? "AndroidARM" : 0;

	case SerializePlatform::OS_LINUX:
		if( SerializePlatform::COMP_GCC != platform.compilerType )
			return unknown;

		switch( platform.archType )
		{
		case SerializePlatform::ARCH_X86:
			return "GccLinux32";

		case SerializePlatform::ARCH_X86_64:
			return "GccLinux64";

		default:
			return unknown;
		}

	default:
		return unknown;
	}
}

Serializer *internalCreateSerializer(Serializer::SerializeType type, Traits *traits)
{
	switch ( type )
	{
		case Serializer::NST_XML:
			{
				void *buf = serializerMemAlloc(sizeof(XmlSerializer), traits);
				return buf ? PX_PLACEMENT_NEW(buf, XmlSerializer)(traits) : 0;
			}
		case Serializer::NST_BINARY:
			{
				void *buf = serializerMemAlloc(sizeof(BinSerializer), traits);
				return buf ? PX_PLACEMENT_NEW(buf, BinSerializer)(traits) : 0;
			}
		default:
			NX_PARAM_TRAITS_WARNING(
				traits,
				"Unknown serializer type: %d",
				(int)type );
			break;
	}

	return 0;
}

Serializer::SerializeType Serializer::peekSerializeType(physx::PxFileBuf &stream)
{
	return isBinaryFormat(stream) ? Serializer::NST_BINARY
		: isXmlFormat(stream) ? Serializer::NST_XML : Serializer::NST_LAST;
}

Serializer::ErrorType Serializer::peekPlatform(physx::PxFileBuf &stream, SerializePlatform &platform)
{
	if( isBinaryFormat(stream) )
		return peekBinaryPlatform(stream, platform);
	
	//Xml has no native platform
	platform = GetCurrentPlatform();
	return Serializer::ERROR_NONE;
}

Serializer::ErrorType Serializer::deserializeMetadata(physx::PxFileBuf & /*stream*/, DeserializedMetadata & /*desData*/)
{
	//Xml currently does not implement this
	return Serializer::ERROR_NOT_IMPLEMENTED;
}

Serializer::ErrorType Serializer::deserialize(physx::PxFileBuf &stream, Serializer::DeserializedData &desData)
{
	bool tmp;
	return deserialize(stream, desData, tmp);
}

Serializer::ErrorType Serializer::deserializeInplace(void *data, physx::PxU32 dataLen, Serializer::DeserializedData &desData)
{
	bool tmp;
	return deserializeInplace(data, dataLen, desData, tmp);
}

}; // end of namespace
