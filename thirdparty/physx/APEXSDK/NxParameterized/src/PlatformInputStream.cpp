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

#include "PlatformInputStream.h"

using namespace NxParameterized;

PlatformInputStream::PlatformInputStream(physx::PxFileBuf &stream, const PlatformABI &targetParams, Traits *traits)
	: PlatformStream(targetParams, traits), mStream(stream), mPos(traits), mStartPos(stream.tellRead())
{}

Serializer::ErrorType PlatformInputStream::skipBytes(physx::PxU32 nbytes)
{
	physx::PxU32 newPos = mStream.tellRead() + nbytes;

	if( newPos >= mStream.getFileLength() )
	{
		DEBUG_ALWAYS_ASSERT();
		return Serializer::ERROR_INVALID_INTERNAL_PTR;
	}

	mStream.seekRead(newPos);

	return Serializer::ERROR_NONE;
}

Serializer::ErrorType PlatformInputStream::pushPos(physx::PxU32 newPos)
{
	if( !newPos )
	{
		//Trying to access NULL
		DEBUG_ALWAYS_ASSERT();
		return Serializer::ERROR_INVALID_INTERNAL_PTR;
	}

	if( mStartPos + newPos >= mStream.getFileLength() )
	{
		DEBUG_ALWAYS_ASSERT();
		return Serializer::ERROR_INVALID_INTERNAL_PTR;
	}

	mPos.pushBack(mStream.tellRead());
	mStream.seekRead(mStartPos + newPos);

	return Serializer::ERROR_NONE;
}

void PlatformInputStream::popPos()
{
	mStream.seekRead(mPos.back());
	mPos.popBack();
}

physx::PxU32 PlatformInputStream::getPos() const
{
	return mStream.tellRead() - mStartPos;
}

Serializer::ErrorType PlatformInputStream::readString(physx::PxU32 off, const char *&s)
{
	if( !off )
	{
		s = 0;
		return Serializer::ERROR_NONE;
	}

	NX_ERR_CHECK_RETURN( pushPos(off) );

	StringBuf buf(mTraits);
	buf.reserve(10);

	physx::PxI8 c;
	do
	{
		NX_ERR_CHECK_RETURN( read(c) );
		buf.append((char)c);
	} while( c );

	popPos();

	s = buf.getBuffer();
	return Serializer::ERROR_NONE;
}

Serializer::ErrorType PlatformInputStream::readObjHeader(ObjHeader &hdr)
{
	physx::PxU32 align_ = PxMax3(mTargetParams.aligns.Bool, mTargetParams.aligns.i32, mTargetParams.aligns.pointer);

	beginStruct(align_);

	physx::PxU32 prevPos = getPos();

	physx::PxU32 relOffset;
	NX_ERR_CHECK_RETURN(read(relOffset));

	hdr.dataOffset = prevPos + relOffset; //Convert relative offset to absolute

	physx::PxU32 classNameOff;
	NX_ERR_CHECK_RETURN( readPtr(classNameOff) );
	NX_ERR_CHECK_RETURN( readString(classNameOff, hdr.className) );

	physx::PxU32 nameOff;
	NX_ERR_CHECK_RETURN( readPtr(nameOff) );
	NX_ERR_CHECK_RETURN( readString(nameOff, hdr.name) );

	NX_ERR_CHECK_RETURN( read(hdr.isIncluded) );
	NX_ERR_CHECK_RETURN( read(hdr.version) );
	NX_ERR_CHECK_RETURN( read(hdr.checksumSize) );

	physx::PxU32 chkOff;
	NX_ERR_CHECK_RETURN( readPtr(chkOff) );

	if( !chkOff )
		hdr.checksum = 0;
	else
	{
		NX_ERR_CHECK_RETURN( pushPos(chkOff) );

		//TODO: usually we can avoid memory allocation here because checksumSize is 4
		hdr.checksum = (physx::PxU32 *)mTraits->alloc(4 * hdr.checksumSize);
		for(physx::PxU32 i = 0; i < hdr.checksumSize; ++i)
			NX_ERR_CHECK_RETURN( read( const_cast<physx::PxU32 &>(hdr.checksum[i]) ) );

		popPos();
	}

	closeStruct();

	NX_ERR_CHECK_RETURN( pushPos(hdr.dataOffset) );

	//Skip NxParameters fields
	//WARN: this depends on implementation of NxParameters

	beginStruct(physx::PxMax(mTargetParams.aligns.pointer, mTargetParams.aligns.Bool));

	physx::PxU32 tmp;
	for(physx::PxU32 i = 0; i < 8; readPtr(tmp), ++i); //vtable and other fields

	bool mDoDeallocateSelf;
	NX_ERR_CHECK_RETURN( read(mDoDeallocateSelf) );

	bool mDoDeallocateName;
	NX_ERR_CHECK_RETURN( read(mDoDeallocateName) );

	bool mDoDeallocateClassName;
	NX_ERR_CHECK_RETURN( read(mDoDeallocateClassName) );

	//See note in beginObject
	if( !mTargetParams.doReuseParentPadding )
		closeStruct();

	hdr.dataOffset = getPos();

	popPos();

	return Serializer::ERROR_NONE;
}

Serializer::ErrorType PlatformInputStream::readSimpleStructArray(Handle &handle)
{
	physx::PxI32 n;
	handle.getArraySize(n);

	const NxParameterized::Definition *pdStruct = handle.parameterDefinition()->child(0);
	physx::PxI32 nfields = pdStruct->numChildren();

	physx::PxU32 align_ = getTargetAlignment(pdStruct),
		pad_ = getTargetPadding(pdStruct); 

	align(align_);

	for(physx::PxI32 i = 0; i < n; ++i)
	{
		beginStruct(align_, pad_);
		handle.set(i);

		for(physx::PxI32 j = 0; j < nfields; ++j)
		{
			handle.set(j);

			const Definition *pdField = pdStruct->child(j);

			if( pdField->alignment() )
				align( pdField->alignment() );

			if( pdField->hint("DONOTSERIALIZE") )
			{
				//Simply skip bytes
				align(getTargetAlignment(pdField));
				NX_ERR_CHECK_RETURN( skipBytes(getTargetSize(pdField)) );
			}
			else
			{
				//No need to align structs because of tail padding
				switch( pdField->type() )
				{
				case TYPE_MAT34:
					{
						physx::PxMat34Legacy val;
						NX_ERR_CHECK_RETURN(read(val));
						handle.setParamMat34((physx::PxMat44)val);
						break;
					}

#				define NX_PARAMETERIZED_TYPES_NO_LEGACY_TYPES
#				define NX_PARAMETERIZED_TYPES_ONLY_SIMPLE_TYPES
#				define NX_PARAMETERIZED_TYPES_NO_STRING_TYPES
#				define NX_PARAMETERIZED_TYPE(type_name, enum_name, c_type) \
				case TYPE_##enum_name: \
					{ \
						c_type val; \
						NX_ERR_CHECK_RETURN(read(val)); \
						handle.setParam##type_name(val); \
						break; \
					}
#					include "NxParameterized_types.h"

				default:
					DEBUG_ASSERT( 0 && "Unexpected type" );
					return Serializer::ERROR_UNKNOWN;
				}
			}

			handle.popIndex();
		} //j

		handle.popIndex();
		closeStruct();
	} //i

	return Serializer::ERROR_NONE;
}

void PlatformInputStream::beginStruct(physx::PxU32 align_, physx::PxU32 pad_)
{
	mStack.pushBack(Agregate(Agregate::STRUCT, pad_));
	align(align_); // Align _after_ we push array because otherwise align() would be ignored
}

void PlatformInputStream::beginStruct(physx::PxU32 align_)
{
	beginStruct(align_, align_);
}

void PlatformInputStream::beginStruct(const Definition *pd)
{
	beginStruct(getTargetAlignment(pd), getTargetPadding(pd));
}

void PlatformInputStream::closeStruct()
{
	PX_ASSERT(mStack.size() > 0);

	align(mStack.back().align); //Tail padding
	mStack.popBack();
}

void PlatformInputStream::beginString()
{
	beginStruct(physx::PxMax(mTargetParams.aligns.pointer, mTargetParams.aligns.Bool));
}

void PlatformInputStream::closeString()
{
	closeStruct();
}

void PlatformInputStream::beginArray(const Definition *pd)
{
	physx::PxU32 align_ = getTargetAlignment(pd);
	align(align_);  // Align _before_ we push array because otherwise align() would be ignored
	mStack.pushBack(Agregate(Agregate::ARRAY, align_));
}

void PlatformInputStream::closeArray()
{
	// No padding in arrays
	mStack.popBack();
}

void PlatformInputStream::align(physx::PxU32 border)
{
	mStream.seekRead(mStartPos + getAlign(getPos(), border));
}

Serializer::ErrorType PlatformInputStream::readPtr(physx::PxU32 &val)
{
	//Do not align on PxU32 or PxU64 boundary (already aligned at pointer boundary)
	align(mTargetParams.aligns.pointer);
	if( 4 == mTargetParams.sizes.pointer )
	{
		return read(val, false);
	}
	else
	{
		PX_ASSERT( 8 == mTargetParams.sizes.pointer );

		physx::PxU64 val64;
		NX_ERR_CHECK_RETURN( read(val64, false) );

		val = (physx::PxU32)val64;
		PX_ASSERT( val == val64 );

		return Serializer::ERROR_NONE;
	}
}
