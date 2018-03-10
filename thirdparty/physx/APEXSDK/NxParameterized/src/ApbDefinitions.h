/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APB_DEFINITIONS_H_
#define APB_DEFINITIONS_H_

// This file contains definitions of various parts of APB file format

// WARNING: before doing any changes to this file
// check comments at the head of BinSerializer.cpp

#include "SerializerCommon.h"
#include "BinaryHelper.h"

#pragma warning(push)
#pragma warning(disable:4127)

namespace NxParameterized
{

#define APB_MAGIC 0x5A5B5C5D

namespace BinVersions
{
	static const physx::PxU32 Initial = 0x00010000,
		AllRefsCounted = 0x00010001,
		WithAlignment = 0x00010002,
		WithExtendedHeader = 0x00010003;
}

// Type of relocation in binary file
enum RelocType
{
	//Raw bytes
	RELOC_ABS_RAW = 0,

	//NxParameterized (will be initialized on deserialization)
	RELOC_ABS_REF,

	RELOC_LAST
};

// Relocation record in binary file
struct BinaryReloc
{
	physx::PxU32 type;
	physx::PxU32 off;
};

// Format of data in binary file; only BINARY_TYPE_PLAIN is used for now
enum BinaryType
{
	BINARY_TYPE_PLAIN = 0,
	BINARY_TYPE_XML_GZ,
	BINARY_TYPE_LAST
};

// Some dummy version control systems insert '\r' before '\n'.
// This short string in header is used to catch this.
// We also use 0xff byte to guarantee that we have non-printable chars
// (s.t. VCS can detect that file is binary).
#define VCS_SAFETY_FLAGS "ab\n\xff"

// File header
#pragma pack(push,1) // For cross-platform compatibility!

// Convert to platform-independent format
static void CanonizeArrayOfU32s(char *data, physx::PxU32 len)
{
	PX_ASSERT(len % 4U == 0);

	if( IsBigEndian() )
		return;

	for(physx::PxU32 i = 0; i < len; i += 4U)
		SwapBytes(data + i, 4U, TYPE_U32);
}

// Main binary header
struct BinaryHeader
{
	physx::PxU32 magic;
	physx::PxU32 type;
	physx::PxU32 version;
	physx::PxI32 numObjects;

	physx::PxU32 fileLength;
	physx::PxU32 dictOffset;
	physx::PxU32 dataOffset;
	physx::PxU32 relocOffset;

	physx::PxU32 metadataOffset;
	physx::PxU32 archType;
	physx::PxU32 compilerType;
	physx::PxU32 compilerVer;

	physx::PxU32 osType;
	physx::PxU32 osVer;
	physx::PxU32 numMetadata;
	physx::PxU32 alignment;

	static bool CheckAlignment()
	{
		bool isPushPackOk = 4 != offsetof(BinaryHeader, type);
		if( isPushPackOk )
		{
			DEBUG_ASSERT( 0 && "PX_PUSH_PACK failed!" );
			return false;
		}

		if( sizeof(BinaryHeader) % 16 != 0 )
		{
			return false;
		}

		return true;
	}

	Serializer::ErrorType getPlatform(SerializePlatform &platform) const
	{
		if( archType >= SerializePlatform::ARCH_LAST
				|| compilerType >= SerializePlatform::COMP_LAST
				|| osType >= SerializePlatform::OS_LAST )
		{
			DEBUG_ALWAYS_ASSERT();
			return Serializer::ERROR_INVALID_PLATFORM;
		}
		
		platform = SerializePlatform(
			static_cast<SerializePlatform::ArchType>(archType),
			static_cast<SerializePlatform::CompilerType>(compilerType),
			compilerVer,
			static_cast<SerializePlatform::OsType>(osType),
			osVer
		);

		return Serializer::ERROR_NONE;
	}

	void canonize()
	{
		CanonizeArrayOfU32s((char*)this, sizeof(BinaryHeader));
	}

	void decanonize() { canonize(); }
};

// Extended header (only in new versions)
struct BinaryHeaderExt
{
	physx::PxU32 vcsSafetyFlags;
	physx::PxU32 res[3 + 8]; // Pad to multiple of 16 byte

	void canonize()
	{
		// vcsSafetyFlags should be stored as-is
	}

	void decanonize()
	{
		// vcsSafetyFlags should be stored as-is
	}
};

#pragma pack(pop)

// NxParameterized object header
struct ObjHeader
{
	physx::PxU32 dataOffset;
	const char *className;
	const char *name;
	bool isIncluded;
	physx::PxU32 version;
	physx::PxU32 checksumSize;
	const physx::PxU32 *checksum;
};

// Element of root references table
struct ObjectTableEntry
{
	Interface *obj;
	const char *className;
	const char *name;
	const char *filename;
};

} // namespace NxParameterized

#pragma warning(pop)

#endif