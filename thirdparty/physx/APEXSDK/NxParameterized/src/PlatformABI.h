/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PLATFORM_ABI_H_
#define PLATFORM_ABI_H_

// WARNING: before doing any changes to this file
// check comments at the head of BinSerializer.cpp

#include "foundation/PxMath.h"

#include <PxMat33Legacy.h>
#include <PxMat34Legacy.h>

#include "BinaryHelper.h"
#include "NxSerializer.h"

namespace NxParameterized
{

//Describes platform ABI (endian, alignment, etc.)
struct PlatformABI
{
	enum Endian
	{
		LITTLE,
		BIG
	};
	
	Endian endian;

	//Sizes of basic types
	struct
	{
		physx::PxU32 Char,
			Bool,
			pointer,
			real;
	} sizes;

	//Alignments of basic types
	struct
	{
		physx::PxU32 Char,
			Bool,
			pointer,
			real,
			i8,
			i16,
			i32,
			i64,
			f32,
			f64;
	} aligns;

	//Does child class reuse tail padding of parent? (google for "overlaying tail padding")
	bool doReuseParentPadding;

	//Are empty base classes eliminated? (google for "empty base class optimization")
	//We may need this in future
	bool doEbo;

	//Get ABI of platform
	static Serializer::ErrorType GetPredefinedABI(const SerializePlatform &platform, PlatformABI &params);

	//Get alignment of (complex) NxParameterized data type
	physx::PxU32 getAlignment(const Definition *pd) const;

	//Get padding of (complex) NxParameterized data type
	physx::PxU32 getPadding(const Definition *pd) const;

	//Get size of (complex) NxParameterized data type
	physx::PxU32 getSize(const Definition *pd) const;

	//Helper function which calculates aligned value
	PX_INLINE static physx::PxU32 align(physx::PxU32 len, physx::PxU32 border);

	//Verifying that platforms are going to work with our serializer
	PX_INLINE bool isNormal() const;
	static bool VerifyCurrentPlatform();

	//Alignment of metadata table entry in metadata section
	//TODO: find better place for this
	PX_INLINE physx::PxU32 getMetaEntryAlignment() const;

	//Alignment of metadata info in metadata section
	//TODO: find better place for this
	PX_INLINE physx::PxU32 getMetaInfoAlignment() const;

	//Alignment of hint in metadata section
	//TODO: find better place for this
	PX_INLINE physx::PxU32 getHintAlignment() const;

	//Alignment of hint value (union of PxU32, PxU64, const char *) in metadata section
	//TODO: find better place for this
	PX_INLINE physx::PxU32 getHintValueAlignment() const;

	//Size of hint value (union of PxU32, PxU64, const char *) in metadata section
	//TODO: find better place for this
	PX_INLINE physx::PxU32 getHintValueSize() const;

	//Template for getting target alignment of T
	template <typename T> PX_INLINE physx::PxU32 getAlignment() const;

	//Template for getting target size of T
	template <typename T> PX_INLINE physx::PxU32 getSize() const;

private:
	physx::PxU32 getNatAlignment(const Definition *pd) const;
};

#include "PlatformABI.inl"

}

#endif
