/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "SnSerialUtils.h"
#include "PsString.h"
#include "PxSerialization.h"

using namespace physx;

namespace
{

#define SN_NUM_BINARY_PLATFORMS 16
const PxU32 sBinaryPlatformTags[SN_NUM_BINARY_PLATFORMS] =
{
	PX_MAKE_FOURCC('W','_','3','2'),
	PX_MAKE_FOURCC('W','_','6','4'),
	PX_MAKE_FOURCC('L','_','3','2'),
	PX_MAKE_FOURCC('L','_','6','4'),
	PX_MAKE_FOURCC('M','_','3','2'),
	PX_MAKE_FOURCC('M','_','6','4'),
	PX_MAKE_FOURCC('M','O','C','A'),
	PX_MAKE_FOURCC('P','S','_','3'),
	PX_MAKE_FOURCC('X','3','6','0'),
	PX_MAKE_FOURCC('A','N','D','R'),
	PX_MAKE_FOURCC('A','I','O','S'),
	PX_MAKE_FOURCC('P','S','P','2'),
	PX_MAKE_FOURCC('W','A','R','M'),
	PX_MAKE_FOURCC('W','I','I','U'),
	PX_MAKE_FOURCC('X','O','N','E'),
	PX_MAKE_FOURCC('A','A','6','4')
};

const char* sBinaryPlatformNames[SN_NUM_BINARY_PLATFORMS] =
{
	"win32",
	"win64",
	"linux32",
	"linux64",
	"macOSX32",
	"macOSX64",
	"ps4",
	"ps3",
	"xbox360",
	"android",
	"ios",
	"psp2",
	"win8arm"
	"wiiu",
	"xboxone",
	"ios64"
};

#define SN_NUM_BINARY_COMPATIBLE_VERSIONS 4

//
// Important: if you adjust the following structure, please adjust the comment for PX_BINARY_SERIAL_VERSION as well
//
const Ps::Pair<PxU32, PxU32> sBinaryCompatibleVersions[SN_NUM_BINARY_COMPATIBLE_VERSIONS] =
{
	Ps::Pair<PxU32, PxU32>(PX_PHYSICS_VERSION, PX_BINARY_SERIAL_VERSION),
	Ps::Pair<PxU32, PxU32>(0x3030300, 0),
	Ps::Pair<PxU32, PxU32>(0x3030200, 0),
	Ps::Pair<PxU32, PxU32>(0x3030100, 0)
};

}

namespace physx { namespace Sn {

PxU32 getBinaryPlatformTag()
{
#if defined(PX_WINDOWS) && defined(PX_X86)
	return sBinaryPlatformTags[0];
#elif defined(PX_WINDOWS) && defined(PX_X64)
	return sBinaryPlatformTags[1];
#elif defined(PX_LINUX) && defined(PX_X86)
	return sBinaryPlatformTags[2];
#elif defined(PX_LINUX) && defined(PX_X64)
	return sBinaryPlatformTags[3];
#elif defined(PX_APPLE) && defined(PX_X86)
	return sBinaryPlatformTags[4];
#elif defined(PX_APPLE) && defined(PX_X64)
	return sBinaryPlatformTags[5];
#elif defined(PX_PS4)
	return sBinaryPlatformTags[6];
#elif defined(PX_PS3)
	return sBinaryPlatformTags[7];
#elif defined(PX_X360)
	return sBinaryPlatformTags[8];
#elif defined(PX_ANDROID)
	return sBinaryPlatformTags[9];
#elif defined(PX_APPLE_IOS) && defined(PX_ARM)
	return sBinaryPlatformTags[10];
#elif defined(PX_PSP2)
	return sBinaryPlatformTags[11];
#elif defined(PX_WINMODERN) && defined(PX_ARM)
	return sBinaryPlatformTags[12];
#elif defined(PX_WIIU)
	return sBinaryPlatformTags[13];
#elif defined(PX_XBOXONE)
	return sBinaryPlatformTags[14];
#elif defined(PX_APPLE_IOS) && defined(PX_A64)
	return sBinaryPlatformTags[15];
#else
	#error Unknown binary platform
#endif
}

bool isBinaryPlatformTagValid(physx::PxU32 platformTag)
{
	PxU32 platformIndex = 0;
	while (platformIndex < SN_NUM_BINARY_PLATFORMS && platformTag != sBinaryPlatformTags[platformIndex]) platformIndex++;
	return platformIndex < SN_NUM_BINARY_PLATFORMS;
}

const char* getBinaryPlatformName(physx::PxU32 platformTag)
{
	PxU32 platformIndex = 0;
	while (platformIndex < SN_NUM_BINARY_PLATFORMS && platformTag != sBinaryPlatformTags[platformIndex]) platformIndex++;
	return (platformIndex == SN_NUM_BINARY_PLATFORMS) ? "unknown" : sBinaryPlatformNames[platformIndex];
}

bool checkCompatibility(const PxU32 version, const PxU32 binaryVersion)
{		
	for(PxU32 i =0; i<SN_NUM_BINARY_COMPATIBLE_VERSIONS; i++)
	{
		if(version == sBinaryCompatibleVersions[i].first && binaryVersion == sBinaryCompatibleVersions[i].second)
			return true;
	}
	return false;
}

void getCompatibilityVersionsStr(char* buffer, PxU32 lenght)
{
	size_t len = 0;
	for(PxU32 i =0; i<SN_NUM_BINARY_COMPATIBLE_VERSIONS; i++)
	{
		physx::string::sprintf_s(buffer + len,  lenght - len, "%x-%d\n", sBinaryCompatibleVersions[i].first, sBinaryCompatibleVersions[i].second);	
		len = strlen(buffer);
	}	
}

} // Sn
} // physx

