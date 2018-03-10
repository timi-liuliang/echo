/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


// WARNING: before doing any changes to this file
// check comments at the head of BinSerializer.cpp

#include "PlatformABI.h"
#include "NxSerializerInternal.h"
#include "SerializerCommon.h"

using namespace NxParameterized;

//Returns ABI for predefined platforms
Serializer::ErrorType PlatformABI::GetPredefinedABI(const SerializePlatform &platform, PlatformABI &params)
{
	//Most common parameters
	params.endian = LITTLE;
	params.sizes.Char = 1;
	params.sizes.Bool = 1;
	params.sizes.pointer = 4;
	params.sizes.real = 4; //physx::PxF32
	params.aligns.Char = 1;
	params.aligns.pointer = 4;
	params.aligns.Bool = 1;
	params.aligns.i8 = 1;
	params.aligns.i16 = 2;
	params.aligns.i32 = 4;
	params.aligns.i64 = 8;
	params.aligns.f32 = 4;
	params.aligns.f64 = 8;
	params.aligns.real = params.aligns.f32;
	params.doReuseParentPadding = false;
	params.doEbo = true;

	SerializePlatform knownPlatform;

	//TODO: all those GetPlatforms are ugly

	NX_BOOL_ERR_CHECK_RETURN( GetPlatform("VcWin32", knownPlatform), Serializer::ERROR_UNKNOWN );
	if( knownPlatform == platform )
	{
		//Default params are ok
		return Serializer::ERROR_NONE;
	}

	NX_BOOL_ERR_CHECK_RETURN( GetPlatform("VcWin64", knownPlatform), Serializer::ERROR_UNKNOWN );
	if( knownPlatform == platform )
	{
		params.sizes.pointer = params.aligns.pointer = 8;
		return Serializer::ERROR_NONE;
	}

	NX_BOOL_ERR_CHECK_RETURN( GetPlatform("VcXbox360", knownPlatform), Serializer::ERROR_UNKNOWN );
	if( knownPlatform == platform )
	{
		//Pointers remain 32-bit
		params.endian = BIG;
		return Serializer::ERROR_NONE;
	}

	// Snc says that it's binary compatible with Gcc...
	NX_BOOL_ERR_CHECK_RETURN( GetPlatform("GccPs3", knownPlatform), Serializer::ERROR_UNKNOWN );
	if( knownPlatform == platform )
	{
		//Pointers remain 32-bit
		params.doReuseParentPadding = true;
		params.endian = BIG;
		return Serializer::ERROR_NONE;
	}

	// Same as ps3 but little endian
	NX_BOOL_ERR_CHECK_RETURN( GetPlatform("AndroidARM", knownPlatform), Serializer::ERROR_UNKNOWN );
	if( knownPlatform == platform )
	{
		//Pointers remain 32-bit
		params.doReuseParentPadding = true;
		params.endian = LITTLE;
		return Serializer::ERROR_NONE;
	}

	NX_BOOL_ERR_CHECK_RETURN( GetPlatform("GccLinux32", knownPlatform), Serializer::ERROR_UNKNOWN );
	if( knownPlatform == platform )
	{
		return Serializer::ERROR_NONE;
	}

	NX_BOOL_ERR_CHECK_RETURN( GetPlatform("GccLinux64", knownPlatform), Serializer::ERROR_UNKNOWN );
	if( knownPlatform == platform )
	{
		params.sizes.pointer = params.aligns.pointer = 8;
		return Serializer::ERROR_NONE;
	}

	// FIXME: true ABI is much more complicated (sizeof(bool) is 4, etc.)
	NX_BOOL_ERR_CHECK_RETURN( GetPlatform("GccOsX32", knownPlatform), Serializer::ERROR_UNKNOWN );
	if( knownPlatform == platform )
	{
		params.doReuseParentPadding = true; // TODO (JPB): Is this correct?
		return Serializer::ERROR_NONE;
	}

	NX_BOOL_ERR_CHECK_RETURN( GetPlatform("Pib", knownPlatform), Serializer::ERROR_UNKNOWN );
	if( knownPlatform == platform )
	{
		params.endian = BIG;

		params.sizes.Char = params.sizes.Bool = 1;
		params.sizes.pointer = 4;
		params.sizes.real = 4; //physx::PxF32

		//All alignments are 1 to minimize space
		physx::PxU32 *aligns = (physx::PxU32 *)&params.aligns;
		for(physx::PxU32 i = 0; i < sizeof(params.aligns)/sizeof(physx::PxU32); ++i)
			aligns[i] = 1;

		return Serializer::ERROR_NONE;
	}

	NX_BOOL_ERR_CHECK_RETURN( GetPlatform("VcXboxOne", knownPlatform), Serializer::ERROR_UNKNOWN );
	if( knownPlatform == platform )
	{
		params.sizes.pointer = params.aligns.pointer = 8;
		return Serializer::ERROR_NONE;
	}

	NX_BOOL_ERR_CHECK_RETURN( GetPlatform("GccPs4", knownPlatform), Serializer::ERROR_UNKNOWN );
	if( knownPlatform == platform )
	{
		// if you don't set this then the in-place binary files could contain padding
		// between the NxParameters class and the parameterized data.
		params.doReuseParentPadding = true;
		params.sizes.pointer = params.aligns.pointer = 8;
		return Serializer::ERROR_NONE;
	}

	//Add new platforms here

	return Serializer::ERROR_INVALID_PLATFORM_NAME;
}

physx::PxU32 PlatformABI::getNatAlignment(const Definition *pd) const
{
	switch( pd->type() )
	{
	case TYPE_ARRAY:
		return pd->arraySizeIsFixed()
			? getAlignment(pd->child(0)) //Array alignment = mermber alignment
			: PxMax3(aligns.pointer, aligns.Bool, aligns.i32); //Dynamic array is DummyDynamicArrayStruct

	case TYPE_STRUCT:
		{
			//Struct alignment is max of fields' alignment
			physx::PxU32 align = 1;
			for(physx::PxI32 i = 0; i < pd->numChildren(); ++i)
				align = physx::PxMax(align, getAlignment(pd->child(i)));

			return align;
		}

	case TYPE_STRING:
		return physx::PxMax(aligns.pointer, aligns.Bool); //String = DummyDynamicStringStruct

	case TYPE_I8:
	case TYPE_U8:
		return aligns.i8;

	case TYPE_I16:
	case TYPE_U16:
		return aligns.i16;

	case TYPE_I32:
	case TYPE_U32:
		return aligns.i32;

	case TYPE_I64:
	case TYPE_U64:
		return aligns.i64;

	case TYPE_F32:
	case TYPE_VEC2:
	case TYPE_VEC3:
	case TYPE_VEC4:
	case TYPE_QUAT:
	case TYPE_MAT33:
	case TYPE_MAT34:
	case TYPE_BOUNDS3:
	case TYPE_MAT44:
	case TYPE_TRANSFORM:
		return aligns.f32;

	case TYPE_F64:
		return aligns.f64;

	case TYPE_ENUM:
	case TYPE_REF:
	case TYPE_POINTER:
		return aligns.pointer;

	case TYPE_BOOL:
		return aligns.Bool;

	default:
		PX_ASSERT( 0 && "Unexpected type" );
	}

	return PX_MAX_U32;
}

//Returns alignment of given DataType
physx::PxU32 PlatformABI::getAlignment(const Definition *pd) const
{
	physx::PxU32 natAlign = getNatAlignment(pd),
		customAlign = pd->alignment();

	// Alignment of dynamic array means alignment of dynamic memory
	return !customAlign || ( TYPE_ARRAY == pd->type() && !pd->arraySizeIsFixed() )
		? natAlign
		: physx::PxMax(natAlign, customAlign);
}

//Returns alignment of given DataType
physx::PxU32 PlatformABI::getPadding(const Definition *pd) const
{
	physx::PxU32 natAlign = getNatAlignment(pd),
		customPad = pd->padding();

	// Alignment of dynamic array means alignment of dynamic memory
	return !customPad || ( TYPE_ARRAY == pd->type() && !pd->arraySizeIsFixed() )
		? natAlign
		: physx::PxMax(natAlign, customPad);
}

//Returns size of given DataType
physx::PxU32 PlatformABI::getSize(const Definition *pd) const
{
	switch( pd->type() )
	{
	case TYPE_ARRAY:
		if( pd->arraySizeIsFixed() )
		{
			//Size of static array = number of elements * size of element
			const Definition *elemPd = pd;
			physx::PxU32 totalSize = 1;
			for(physx::PxI32 i = 0; i < pd->arrayDimension(); ++i)
			{
				// Currently no nested dynamic arrays
				NX_BOOL_ERR_CHECK_RETURN( elemPd->arraySizeIsFixed(), Serializer::ERROR_NOT_IMPLEMENTED );

				physx::PxI32 size = elemPd->arraySize();
				totalSize *= size;

				elemPd = elemPd->child(0);
			}
			return totalSize * getSize(elemPd);
		}
		else
		{
			//Dynamic array = DummyDynamicArrayStruct

			physx::PxU32 totalAlign = PxMax3(aligns.pointer, aligns.Bool, aligns.i32);

			physx::PxU32 size = sizes.pointer; //buf
			size = align(size, aligns.Bool) + 1U; //isAllocated
			size = align(size, aligns.i32) + 4U; //elementSize
			size = align(size, aligns.i32) + pd->arrayDimension() * 4U; //arraySizes

			physx::PxU32 paddedSize = align(size, totalAlign);

			return paddedSize;
		}

	case TYPE_STRUCT:
		{
			if( !pd->numChildren() )
				return 1;

			//Size of struct = sum of member sizes + sum of padding bytes + tail padding

			physx::PxU32 totalAlign = 1, size = 0;
			for(physx::PxI32 i = 0; i < pd->numChildren(); ++i)
			{
				physx::PxU32 childAlign = getAlignment(pd->child(i));
				totalAlign = physx::PxMax(totalAlign, childAlign);
				size = align(size, childAlign) + getSize(pd->child(i));
			}

			physx::PxU32 customPad = pd->padding();
			if( customPad )
				totalAlign = physx::PxMax(totalAlign, customPad);

			return align(size, totalAlign); //Tail padding bytes
		}

	case TYPE_STRING:
		{
			//String = DummyDynamicStringStruct

			physx::PxU32 totalAlign = physx::PxMax(aligns.pointer, aligns.Bool);

			physx::PxU32 size = sizes.pointer; //buf
			size = align(size, aligns.Bool) + 1U; //isAllocated

			physx::PxU32 paddedSize = align(size, totalAlign);

			return paddedSize;
		}

	case TYPE_I8:
	case TYPE_U8:
		return 1;

	case TYPE_I16:
	case TYPE_U16:
		return 2;

	case TYPE_I32:
	case TYPE_U32:
	case TYPE_F32:
		return 4;

	case TYPE_I64:
	case TYPE_U64:
	case TYPE_F64:
		return 8;

	// Vectors and matrices are structs so we need tail padding

	case TYPE_VEC2:
		return align(2 * sizes.real, aligns.real);
	case TYPE_VEC3:
		return align(3 * sizes.real, aligns.real);
	case TYPE_VEC4:
		return align(4 * sizes.real, aligns.real);
	case TYPE_QUAT:
		return align(4 * sizes.real, aligns.real);
	case TYPE_MAT33:
		return align(9 * sizes.real, aligns.real);
	case TYPE_MAT34:
		return align(12 * sizes.real, aligns.real);
	case TYPE_MAT44:
		return align(16 * sizes.real, aligns.real);
	case TYPE_BOUNDS3:
		return align(6 * sizes.real, aligns.real);
	case TYPE_TRANSFORM:
		return align(7 * sizes.real, aligns.real);

	case TYPE_ENUM:
	case TYPE_REF:
	case TYPE_POINTER:
		return sizes.pointer;

	case TYPE_BOOL:
		return sizes.Bool;

	default:
		PX_ASSERT( 0 && "Unexpected type" );
	}

	return PX_MAX_U32; //This is never reached
}

bool PlatformABI::VerifyCurrentPlatform()
{
	//See PlatformABI::isNormal

	physx::PxU8 one = 1,
		zero = 0;

	struct Empty {};

	return 1 == GetAlignment<bool>::value && 1 == sizeof(bool)
		&& 1 == GetAlignment<physx::PxU8>::value
		&& 1 == GetAlignment<char>::value
		&& 4 == sizeof(physx::PxReal)
		&& *(bool *)&one //We assume that 0x1 corresponds to true internally
		&& !*(bool *)&zero //We assume that 0x0 corresponds to false internally
		&& 1 == sizeof(Empty); // Serializer expects sizeof empty struct to be 1
}
