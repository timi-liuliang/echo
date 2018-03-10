/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PLATFORM_OUTPUT_STREAM_H_
#define PLATFORM_OUTPUT_STREAM_H_

// WARNING: before doing any changes to this file
// check comments at the head of BinSerializer.cpp

#include "PlatformStream.h"
#include "ApbDefinitions.h"

#ifndef WITHOUT_APEX_SERIALIZATION

namespace NxParameterized
{

class PlatformOutputStream;

//Info about pointed data (and its subdata)
struct Reloc
{
	RelocType type;
	physx::PxU32 ptrPos;
	PlatformOutputStream *ptrData;
	Traits *traits;

	Reloc(RelocType type_, physx::PxU32 ptrPos_, const PlatformOutputStream &parent);

	Reloc(const Reloc &cinfo);

	~Reloc();
};

//Info about pointed string
struct StringReloc
{
	physx::PxU32 ptrPos;
	const char *s;

	StringReloc(physx::PxU32 ptrPos_, const char *s_): ptrPos(ptrPos_), s(s_) {}
};

//Info about reloc which was already merged
struct MergedReloc
{
	physx::PxU32 ptrPos;
	physx::PxU32 targetPos;
	RelocType type;
	bool isExtern;
};

//ABI-aware output stream
class PlatformOutputStream: public PlatformStream
{
	friend struct Reloc;
public:
	PlatformOutputStream(const PlatformABI &targetParams, Traits *traits, Dictionary &dict_);

	//Array's copy constructor is broken so we implement it by hand
	PlatformOutputStream(const PlatformOutputStream &s);

#ifndef NDEBUG
	void dump() const;
#endif

	PX_INLINE StringBuf &getData() { return data; }

	physx::PxU32 size() const { return data.size(); }

	//Update physx::PxU32 at given position
	void storeU32At(physx::PxU32 x, physx::PxU32 i);

	physx::PxU32 storeString(const char *s);

	//Serialize raw bytes
	physx::PxU32 storeBytes(const char *s, physx::PxU32 n);

	//Serialize value of primitive type with proper alignment
	//(this is overriden for bools below)
	template <typename T> PX_INLINE physx::PxU32 storeSimple(T x);

	//Serialize array of primitive type with proper alignment (slow path)
	template <typename T> PX_INLINE physx::PxI32 storeSimpleArraySlow(Handle &handle);

	//Serialize array of structs of primitive types with proper alignment
	physx::PxU32 storeSimpleStructArray(Handle &handle);

	//Serialize array of primitive type with proper alignment
	template <typename T> PX_INLINE physx::PxU32 storeSimpleArray(Handle &handle);

	//Serialize header of NxParameterized object (see wiki for details)
	physx::PxU32 storeObjHeader(const NxParameterized::Interface &obj, bool isIncluded = true);

	//Serialize NxParameters-part of NxParameterized object
	physx::PxU32 beginObject(const NxParameterized::Interface &obj, bool /*isRoot*/, const Definition *pd);

	//Insert tail padding bytes for NxParameterized object
	void closeObject() { closeStruct(); }

	//Align current offset according to supplied alignment and padding
	physx::PxU32 beginStruct(physx::PxU32 align_, physx::PxU32 pad_);

	//Align current offset according to supplied alignment (padding = alignment)
	physx::PxU32 beginStruct(physx::PxU32 align_);

	//Align current offset according to supplied DataType
	physx::PxU32 beginStruct(const Definition *pd);

	//Insert tail padding
	void closeStruct();

	//beginStruct for DummyStringStruct
	physx::PxU32 beginString();

	//closeStruct for DummyStringStruct
	void closeString();

	//beginStruct for arrays
	physx::PxU32 beginArray(const Definition *pd);

	//beginStruct for arrays
	physx::PxU32 beginArray(physx::PxU32 align_);

	//closeStruct for arrays
	void closeArray();

	void skipBytes(physx::PxU32 nbytes);

	//Align offset to be n*border
	void align(physx::PxU32 border);

	//Align offset to be n * alignment of T
	template <typename T> PX_INLINE void align();

	//Add data (including relocations) from another stream
	physx::PxU32 merge(const PlatformOutputStream &mergee);

	//Merge pointed data (including strings) thus flattening the stream
	void flatten();

	//Create relocation table
	physx::PxU32 writeRelocs();

	//Serialize dictionary data
	void mergeDict();

	physx::PxU32 storeNullPtr();

	Reloc &storePtr(RelocType type, const Definition *pd);

	Reloc &storePtr(RelocType type, physx::PxU32 align);

	void storeStringPtr(const char *s);

	PX_INLINE physx::PxU32 alignment() const { return mTotalAlign; }

	PX_INLINE void setAlignment(physx::PxU32 newAlign) { mTotalAlign = newAlign; }

private:

	//Byte stream
	StringBuf data;

	//Generic relocations
	physx::Array<Reloc, Traits::Allocator> mRelocs;

	//String relocations
	physx::Array<StringReloc, Traits::Allocator> mStrings;

	//Keep info about relocations after flattening
	physx::Array<MergedReloc, Traits::Allocator> mMerges;

	Dictionary &dict;

	physx::PxU32 mTotalAlign;
};

#include "PlatformOutputStream.inl"

}

#endif
#endif
