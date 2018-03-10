/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_SERIALIZE_BINARY_HELPER_H
#define NX_SERIALIZE_BINARY_HELPER_H

// WARNING: before doing any changes to this file
// check comments at the head of BinSerializer.cpp

#include <PsShare.h>
#include <PsArray.h>

#include "NxParameterized.h"
#include "NxParameterizedTraits.h"

namespace NxParameterized
{

template<typename T> static PX_INLINE T PxMax3(T x, T y, T z)
{
	return physx::PxMax<T>(x, physx::PxMax<T>(y, z));
}

#ifndef offsetof
#	define offsetof(StructType, field) reinterpret_cast<size_t>(&((StructType *)0)->field)
#endif

// Alignment calculator
template<typename T> class GetAlignment {
	struct TestStruct {
		char _;
		T x;
	};

public:
	static const size_t value = offsetof(struct TestStruct, x);
};

// Maps C type to NxParameterized::DataType
template<typename T> struct GetDataType {
	static const DataType value = NxParameterized::TYPE_UNDEFINED;
};

// Currently we only need to distinguish PxVec2 from other 64-bit stuff
// (like PxU64, PxI64, PxF64, pointer) in SwapBytes

template<> struct GetDataType <physx::PxVec2> {
	static const DataType value = NxParameterized::TYPE_VEC2;
};

//Copied from NvApexStream
PX_INLINE static bool IsBigEndian()
{
	physx::PxU32 i = 1;
	return 0 == *(char *)&i;
}

PX_INLINE static void SwapBytes(char *data, physx::PxU32 size, NxParameterized::DataType type)
{
	// XDK compiler does not like switch here
	if( 1 == size )
	{
		// Do nothing
	}
	else if( 2 == size )
	{
		char one_byte;
		one_byte = data[0]; data[0] = data[1]; data[1] = one_byte;
	}
	else if( 4 == size )
	{
		char one_byte;
		one_byte = data[0]; data[0] = data[3]; data[3] = one_byte;
		one_byte = data[1]; data[1] = data[2]; data[2] = one_byte;
	}
	else if( 8 == size )
	{
		//Handling of PxVec2 agregate is different from 64-bit atomic types
		if( TYPE_VEC2 == type )
		{
			//PxVec2 => swap each field separately
			SwapBytes(data + 0, 4, TYPE_F32);
			SwapBytes(data + 4, 4, TYPE_F32);
		}
		else
		{
			char one_byte;
			one_byte = data[0]; data[0] = data[7]; data[7] = one_byte;
			one_byte = data[1]; data[1] = data[6]; data[6] = one_byte;
			one_byte = data[2]; data[2] = data[5]; data[5] = one_byte;
			one_byte = data[3]; data[3] = data[4]; data[4] = one_byte;
		}
	}
	else
	{
		//Generic algorithm for containers of physx::PxReal

		const size_t elemSize = sizeof(physx::PxReal); //We assume that PxReal sizes match on both platforms
		PX_ASSERT( elemSize >= GetAlignment<physx::PxReal>::value ); //If alignment is non-trivial below algorithm will not work
		PX_ASSERT( size > elemSize );

		//Just swap all PxReals
		for(size_t i = 0; i < size; i += elemSize)
			SwapBytes(data + i, elemSize, TYPE_F32);
	}
}

//Convert value to platform-independent format (network byte order)
template<typename T> PX_INLINE static T Canonize(T x)
{
	if( !IsBigEndian() )
		SwapBytes((char *)&x, sizeof(T), GetDataType<T>::value);

	return x;
}

//Convert value to native format (from network byte order)
template<typename T> PX_INLINE static T Decanonize(T x)
{
	return Canonize(x);
}

//Read platform-independent value from stream and convert it to native format
template<typename T> static PX_INLINE T readAndConvert(const char *&p)
{
	T val;
	memcpy((char *)&val, p, sizeof(T));
	val = Decanonize(val);
	p += sizeof(T);

	return val;
}

//Byte array used for data serialization
//TODO: replace this with Array?
class StringBuf
{
	Traits *mTraits;
	char *mData;
	physx::PxU32 mSize, mCapacity;

	PX_INLINE void internalAppend(const char *data, physx::PxU32 size, bool doCopy = true)
	{
		if( 0 == size )
			return;

		if( mCapacity < mSize + size )
			reserve(physx::PxMax(mSize + size, 3 * mCapacity / 2));

		if( doCopy )
			memcpy(mData + mSize, data, size);
		else
			memset(mData + mSize, 0, size); //We want padding bytes filled with 0

		mSize += size;
	}

public:

	PX_INLINE StringBuf(Traits *traits)
		: mTraits(traits), mData(0), mSize(0), mCapacity(0)
	{}

	PX_INLINE StringBuf(const StringBuf &s)
		: mTraits(s.mTraits), mSize(s.mSize), mCapacity(s.mSize)
	{
		mData = (char *)mTraits->alloc(mSize);
		memcpy(mData, s.mData, mSize);
	}

	PX_INLINE ~StringBuf() { mTraits->free((void *)mData); }

	PX_INLINE void reserve(physx::PxU32 newCapacity)
	{
		if( mCapacity >= newCapacity )
			return;

		char *newData = (char *)mTraits->alloc(newCapacity);
		PX_ASSERT(newData);

		if( mData )
		{
			memcpy(newData, mData, mSize);
			mTraits->free(mData);
		}

		mData = newData;
		mCapacity = newCapacity;
	}

	PX_INLINE char *getBuffer()
	{
		char *data = mData;

		mSize = mCapacity = 0;
		mData = 0;

		return data;
	}

	PX_INLINE physx::PxU32 size() const { return mSize; }

	template< typename T > PX_INLINE void append(T x)
	{
		internalAppend((char *)&x, sizeof(T));
	}

	template< typename T > PX_INLINE void append(T *x)
	{
		PX_UNUSED(x);
		PX_ASSERT(0 && "Unable to append pointer");
	}

	PX_INLINE void appendBytes(const char *data, physx::PxU32 size) { internalAppend(data, size); }

	PX_INLINE void skipBytes(physx::PxU32 size) { internalAppend(0, size, false); }

	PX_INLINE char &operator [](physx::PxU32 i)
	{
		PX_ASSERT( i < mSize );
		return mData[i];
	}

	PX_INLINE operator char *() { return mData; }

	PX_INLINE operator const char *() const { return mData; }
};

//Dictionary of strings used for binary serialization
class Dictionary
{
	struct Entry
	{
		const char *s;
		physx::PxU32 offset;
	};

	physx::Array<Entry, Traits::Allocator> entries; //TODO: use hash map after DE402 is fixed

public:
	Dictionary(Traits *traits_): entries(Traits::Allocator(traits_)) {}

	physx::PxU32 put(const char *s);

	void setOffset(const char *s, physx::PxU32 off);

	PX_INLINE void setOffset(physx::PxU32 i, physx::PxU32 off) { setOffset(get(i), off); }

	physx::PxU32 getOffset(const char *s) const;

	physx::PxU32 getOffset(physx::PxU32 i) const { return getOffset(get(i)); }

	const char *get(physx::PxU32 i) const { return entries[i].s; }

	PX_INLINE physx::PxU32 size() const { return entries.size(); }

	void serialize(StringBuf &res) const;
};

//Binary file pretty-printer (mimics xxd)
void dumpBytes(const char *data, physx::PxU32 nbytes);

};

#endif
