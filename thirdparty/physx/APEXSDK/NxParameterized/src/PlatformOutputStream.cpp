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

#include "PlatformOutputStream.h"

using namespace NxParameterized;

#ifndef WITHOUT_APEX_SERIALIZATION

Reloc::Reloc(RelocType type_, physx::PxU32 ptrPos_, const PlatformOutputStream &parent)
	: type(type_),
	ptrPos(ptrPos_),
	traits(parent.mTraits)
{
	ptrData = reinterpret_cast<PlatformOutputStream *>(traits->alloc(sizeof(PlatformOutputStream)));
	PX_PLACEMENT_NEW(ptrData, PlatformOutputStream)(parent.mTargetParams, parent.mTraits, parent.dict);
}

Reloc::Reloc(const Reloc &cinfo): type(cinfo.type), ptrPos(cinfo.ptrPos), traits(cinfo.traits)
{
	Reloc &info = (Reloc &)cinfo;

	//Take ownership of stream to avoid slow recursive copies (especially when reallocating array elements)
	ptrData = info.ptrData;
	info.ptrData = 0;
}

Reloc::~Reloc()
{
	if( ptrData )
	{
		ptrData->~PlatformOutputStream();
		traits->free(ptrData);
	}
}

PlatformOutputStream::PlatformOutputStream(const PlatformABI &targetParams, Traits *traits, Dictionary &dict_)
	: PlatformStream(targetParams, traits),
	data(traits),
	mRelocs(Traits::Allocator(traits)),
	mStrings(Traits::Allocator(traits)),
	mMerges(Traits::Allocator(traits)),
	dict(dict_),
	mTotalAlign(1)
{}

PlatformOutputStream::PlatformOutputStream(const PlatformOutputStream &s)
	: PlatformStream(s),
	data(s.data),
	mRelocs(Traits::Allocator(s.mTraits)),
	mStrings(Traits::Allocator(s.mTraits)),
	mMerges(Traits::Allocator(s.mTraits)),
	dict(s.dict),
	mTotalAlign(1)
{
	mRelocs.reserve(s.mRelocs.size());
	for(physx::PxU32 i = 0; i < s.mRelocs.size(); ++i)
		mRelocs.pushBack(s.mRelocs[i]);

	mStrings.reserve(s.mStrings.size());
	for(physx::PxU32 i = 0; i < s.mStrings.size(); ++i)
		mStrings.pushBack(s.mStrings[i]);

	mMerges.reserve(s.mMerges.size());
	for(physx::PxU32 i = 0; i < s.mMerges.size(); ++i)
		mMerges.pushBack(s.mMerges[i]);
}

#ifndef NDEBUG
void PlatformOutputStream::dump() const
{
	PlatformStream::dump();

	dumpBytes(data, size());

	fflush(stdout);
	for(physx::PxU32 i = 0; i < mRelocs.size(); ++i)
	{
		printf("Relocation %d at %x:\n", (int)i, mRelocs[i].ptrPos);
		mRelocs[i].ptrData->dump();
	}

	fflush(stdout);
}
#endif

void PlatformOutputStream::storeU32At(physx::PxU32 x, physx::PxU32 i)
{
	if( mCurParams.endian != mTargetParams.endian )
		SwapBytes(reinterpret_cast<char *>(&x), 4U, TYPE_U32);

	*reinterpret_cast<physx::PxU32 *>(&data[i]) = x;
}

physx::PxU32 PlatformOutputStream::storeString(const char *s)
{
	physx::PxU32 off = storeSimple((physx::PxU8)*s);
	while( *s++ )
		storeSimple((physx::PxU8)*s);

	return off;
}

physx::PxU32 PlatformOutputStream::storeBytes(const char *s, physx::PxU32 n)
{
	if( !n )
		return size();

	physx::PxU32 off = storeSimple((physx::PxU8)s[0]);
	for(physx::PxU32 i = 1; i < n; ++i)
		storeSimple((physx::PxU8)s[i]);

	return off;
}

physx::PxU32 PlatformOutputStream::beginStruct(physx::PxU32 align_, physx::PxU32 pad_)
{
	physx::PxU32 off = size();
	mStack.pushBack(Agregate(Agregate::STRUCT, pad_));
	align(align_); // Align _after_ we push struct to avoid ignored align() when inside array
	return off;
}

physx::PxU32 PlatformOutputStream::beginStruct(physx::PxU32 align_)
{
	return beginStruct(align_, align_);
}

physx::PxU32 PlatformOutputStream::beginStruct(const Definition *pd)
{
	return beginStruct(getTargetAlignment(pd), getTargetPadding(pd));
}

void PlatformOutputStream::closeStruct()
{
	PX_ASSERT(mStack.size() > 0);

	//Tail padding
	align(mStack.back().align);// Align _before_ we pop struct to avoid ignored align() when inside array
	mStack.popBack();
}

physx::PxU32 PlatformOutputStream::beginString()
{
	return beginStruct(physx::PxMax(mTargetParams.aligns.pointer, mTargetParams.aligns.Bool));
}

void PlatformOutputStream::closeString()
{
	closeStruct();
}

physx::PxU32 PlatformOutputStream::beginArray(const Definition *pd)
{
	return beginArray(getTargetAlignment(pd));
}

physx::PxU32 PlatformOutputStream::beginArray(physx::PxU32 align_)
{
	align(align_); // Align _before_ we push array because otherwise align() would be ignored
	physx::PxU32 off = size();
	mStack.pushBack(Agregate(Agregate::ARRAY, align_));
	return off;
}

void PlatformOutputStream::closeArray()
{
	// No tail padding when in array
	mStack.popBack();
}

void PlatformOutputStream::skipBytes(physx::PxU32 nbytes)
{
	data.skipBytes(nbytes);
}

void PlatformOutputStream::align(physx::PxU32 border)
{
	bool isAligned;
	physx::PxU32 newSize = getAlign(size(), border, isAligned);

	if( isAligned )
		mTotalAlign = physx::PxMax(mTotalAlign, border);

	data.skipBytes(newSize - size());
}

void PlatformOutputStream::mergeDict()
{
	for(physx::PxU32 i = 0; i < dict.size(); ++i)
	{
		const char *s = dict.get(i);
		physx::PxU32 off = storeString(s);
		dict.setOffset(s, off);
	}
}

physx::PxU32 PlatformOutputStream::storeNullPtr()
{
	//Do not align on PxU32 or PxU64 boundary (already aligned at pointer boundary)
	align(mTargetParams.aligns.pointer);
	physx::PxU32 off = size();
	if( 4 == mTargetParams.sizes.pointer )
		data.skipBytes(4);
	else
	{
		PX_ASSERT( 8 == mTargetParams.sizes.pointer );
		data.skipBytes(8);
	}
	return off;
}

Reloc &PlatformOutputStream::storePtr(RelocType type, physx::PxU32 align)
{
	physx::PxU32 off = storeNullPtr();
	mRelocs.pushBack(Reloc(type, off, *this));
	mRelocs.back().ptrData->setAlignment(align);
	return mRelocs.back();
}

Reloc &PlatformOutputStream::storePtr(RelocType type, const Definition *pd)
{
	return storePtr(type, getTargetAlignment(pd));
}

void PlatformOutputStream::storeStringPtr(const char *s)
{
	physx::PxU32 off = storeNullPtr();
	if( s )
	{
		mStrings.pushBack(StringReloc(off, s));
		dict.put(s);
	}
}

physx::PxU32 PlatformOutputStream::storeSimpleStructArray(Handle &handle)
{
	physx::PxI32 n;
	handle.getArraySize(n);

	const NxParameterized::Definition *pdStruct = handle.parameterDefinition()->child(0);
	physx::PxI32 nfields = pdStruct->numChildren();

	physx::PxU32 align_ = getTargetAlignment(pdStruct),
		size_ = getTargetSize(pdStruct),
		pad_ = getTargetPadding(pdStruct);

	align(align_);
	physx::PxU32 off = size();

	data.reserve(size() + n * physx::PxMax(align_, size_));

	char *p = data;
	p += data.size();

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
				skipBytes(getTargetSize(pdField));
			}
			else
			{
				//No need to align structs because of tail padding
				switch( pdField->type() )
				{
				case TYPE_MAT34:
					{
						physx::PxMat44 val;
						handle.getParamMat34(val);
						storeSimple(physx::PxMat34Legacy(val));
						break;
					}

#				define NX_PARAMETERIZED_TYPES_NO_LEGACY_TYPES
#				define NX_PARAMETERIZED_TYPES_ONLY_SIMPLE_TYPES
#				define NX_PARAMETERIZED_TYPES_NO_STRING_TYPES
#				define NX_PARAMETERIZED_TYPE(type_name, enum_name, c_type) \
					case TYPE_##enum_name: \
					{ \
						c_type val; \
						handle.getParam##type_name(val); \
						storeSimple<c_type>(val); \
						break; \
					}
#					include "NxParameterized_types.h"

				default:
					DEBUG_ASSERT( 0 && "Unexpected type" );
					return PX_MAX_U32;
				}
			}

			handle.popIndex();
		} //j

		handle.popIndex();
		closeStruct();
	} //i

	return off;
}

physx::PxU32 PlatformOutputStream::storeObjHeader(const NxParameterized::Interface &obj, bool isIncluded)
{
	physx::PxU32 align_ = PxMax3(mTargetParams.aligns.Bool, mTargetParams.aligns.i32, mTargetParams.aligns.pointer);

	physx::PxU32 off = beginStruct(align_);

	physx::PxU32 hdrOff = data.size();
	PX_ASSERT( hdrOff % sizeof(physx::PxU32) == 0 );

	storeSimple(physx::PxU32(0)); //Data offset

	//className
	storeStringPtr(obj.className());

	//name
	storeStringPtr(obj.name());

	//isIncluded
	storeSimple(isIncluded);

	//version
	storeSimple<physx::PxU32>(obj.version());

	//checksum size
	physx::PxU32 bits = (physx::PxU32)-1;
	const physx::PxU32 *checksum = obj.checksum(bits);
	PX_ASSERT( bits % 32 == 0 ); //32 bits in physx::PxU32
	physx::PxU32 i32s = bits / 32;
	storeSimple(i32s);

	//checksum pointer
	Reloc &reloc = storePtr(RELOC_ABS_RAW, mTargetParams.aligns.i32);
	for(physx::PxU32 i = 0; i < i32s; ++i)
		reloc.ptrData->storeSimple(checksum[i]);

	closeStruct();

	//We force alignment to calculate dataOffset
	//(when object is inserted no additional padding will be inserted)

	const Definition *pd = obj.rootParameterDefinition();
	physx::PxU32 customAlign = pd ? getTargetAlignment(pd) : 1;
	align(physx::PxMax(16U, customAlign)); //16 for safety

	storeU32At(data.size() - hdrOff, hdrOff); //Now we know object data offset

	return off;
}

physx::PxU32 PlatformOutputStream::beginObject(const NxParameterized::Interface &obj, bool /*isRoot*/, const Definition *pd)
{
	//NxParameterized objects is derived from NxParameters so we need to store its fields as well.

	//WARN: this implementation _heavily_ depends on implementation of NxParameters

	//Alignment of NxParameters
	physx::PxU32 parentAlign = physx::PxMax(mTargetParams.aligns.pointer, mTargetParams.aligns.Bool),
		childAlign = pd ? getTargetAlignment(pd) : 1,
		totalAlign = physx::PxMax(parentAlign, childAlign);

	physx::PxU32 off = beginStruct(totalAlign);

	//NxParameters fields
	for(physx::PxU32 i = 0; i < 6; storeNullPtr(), ++i); //vtable and other fields
	storeStringPtr(obj.name()); //mName
	storeStringPtr(obj.className()); //mClassName
	storeSimple(true); //mDoDeallocateSelf (all objects are responsible for memory deallocation)
	storeSimple(false); //mDoDeallocateName
	storeSimple(false); //mDoDeallocateClassName

	//Some general theory of alignment handling
	//Imagine that we have class:
	//class A: A1, A2, ... An
	//{
	//	T1 f1;
	//	T2 f2;
	//};
	//Then all Ai/fi are aligned on natural boundary. Whether or not padding bytes for Ai are inserted
	//and whether or not Ai+1 or fi may reuse those bytes depends on compiler;
	//we store this info in PlatformABI's doReusePadding flag.
	//doReusePadding == false means that padding bytes are always there.
	//doReusePadding == true means that padding bytes are _not_ inserted for non-POD Ai but are inserted for POD Ai.
	//(I have yet to see compiler that does not insert padding bytes for POD base class!).

	//Compilers may handle derived classes in two different ways.
	//Say we have
	//	class B {int x; char y; };
	//	class A: public B {char z; };
	//Then in pure C code this may look like either as
	//	struct A { struct { int x; char y; } b; char z; };
	//or as
	//	struct A { int x; char y; char z; };
	//(the latter is usual if B is not POD).
	//Take care of that here (NxParameters is not POD!).
	if( !mTargetParams.doReuseParentPadding )
		align(parentAlign); //Insert tail padding for NxParameters

	//ParametersStruct is aligned on natural boundary
	align(childAlign);

	return off;
}

physx::PxU32 PlatformOutputStream::merge(const PlatformOutputStream &mergee)
{
	//All structs should be closed
	PX_ASSERT( !mergee.mStack.size() );

	align(mergee.alignment());

	physx::PxU32 base = data.size();

	data.appendBytes(mergee.data, mergee.data.size());

	//Update relocations

	mRelocs.reserve(mRelocs.size() + mergee.mRelocs.size());
	for(physx::PxU32 i = 0; i < mergee.mRelocs.size(); ++i)
	{
		mRelocs.pushBack(mergee.mRelocs[i]);
		mRelocs.back().ptrPos += base;
	}

	mStrings.reserve(mStrings.size() + mergee.mStrings.size());
	for(physx::PxU32 i = 0; i < mergee.mStrings.size(); ++i)
	{
		mStrings.pushBack(mergee.mStrings[i]);
		mStrings.back().ptrPos += base;
	}

	mMerges.reserve(mMerges.size() + mergee.mMerges.size());
	for(physx::PxU32 i = 0; i < mergee.mMerges.size(); ++i)
	{
		mMerges.pushBack(mergee.mMerges[i]);

		mMerges.back().ptrPos += base;
		if( !mergee.mMerges[i].isExtern )
			mMerges.back().targetPos += base;
	}

	return base;
}

void PlatformOutputStream::flatten()
{
	//It's very important that data for child objects is stored
	//after current object to allow safe initialization

	//Generic pointers
	for(physx::PxU32 i = 0; i < mRelocs.size(); ++i)
	{
		Reloc &reloc = mRelocs[i];

		//Recursively add data which is pointed-to

		align( reloc.ptrData->alignment() );

		MergedReloc m = { reloc.ptrPos, data.size(), reloc.type, false };
		mMerges.pushBack(m);

		merge(*reloc.ptrData); //Internal pointers are recursively added here
	}

	//String pointers
	for(physx::PxU32 i = 0; i < mStrings.size(); ++i)
	{
		//String pointers are external and absolute
		MergedReloc m = { mStrings[i].ptrPos, dict.getOffset(mStrings[i].s), RELOC_ABS_RAW, true };
		mMerges.pushBack(m);
	}

	mRelocs.clear();
	mStrings.clear();
}

physx::PxU32 PlatformOutputStream::writeRelocs()
{
	physx::PxU32 ptrOff = storeSimple<physx::PxU32>(mMerges.size()); //Offset of relocation table

	data.reserve(mMerges.size() * 2 * physx::PxMax(4U, mTargetParams.aligns.i32));

	for(physx::PxU32 i = 0; i < mMerges.size(); ++i)
	{
		char *ptr = &data[mMerges[i].ptrPos];
		if( 4 == mTargetParams.sizes.pointer )
		{
			physx::PxU32 *ptrAsInt = reinterpret_cast<physx::PxU32 *>(ptr);
			*ptrAsInt = mMerges[i].targetPos;
			if( mTargetParams.endian != mCurParams.endian )
				SwapBytes(ptr, 4U, TYPE_U32);
		}
		else
		{
			PX_ASSERT(8 == mTargetParams.sizes.pointer);

			physx::PxU64 *ptrAsInt = reinterpret_cast<physx::PxU64 *>(ptr);
			*ptrAsInt = mMerges[i].targetPos;
			if( mTargetParams.endian != mCurParams.endian )
				SwapBytes(ptr, 8U, TYPE_U64);
		}

		//BinaryReloc struct
		beginStruct(mTargetParams.aligns.i32);
		storeSimple<physx::PxU32>(static_cast<physx::PxU32>(mMerges[i].type));
		storeSimple<physx::PxU32>(mMerges[i].ptrPos);
		closeStruct();
	}

	mMerges.clear();

	return ptrOff;
}

#endif
