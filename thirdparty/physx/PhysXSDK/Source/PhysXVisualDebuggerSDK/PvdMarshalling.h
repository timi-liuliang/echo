/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef PVD_MARSHALLING_H
#define PVD_MARSHALLING_H
#include "PvdObjectModelBaseTypes.h"
#include "PsIntrinsics.h"
#include "PvdBits.h"
namespace physx { namespace debugger {
	using namespace physx::shdfnd;
	
//Define marshalling

template<typename TSmallerType, typename TLargerType>
struct PvdMarshalling
{
	bool canMarshal;
	PvdMarshalling() : canMarshal( false ) {}
};



template<typename smtype, typename lgtype>
static inline void marshalSingleT( const PxU8* srcData, PxU8* destData )
{
	smtype incoming;

	PxMemCopy( &incoming, srcData, sizeof( smtype ) );
	lgtype outgoing = static_cast<lgtype>( incoming );
	PxMemCopy( destData, &outgoing, sizeof( lgtype ) );
}

template<typename smtype, typename lgtype>
static inline void marshalBlockT( const PxU8* srcData, PxU8* destData, PxU32 numBytes )
{										
	for( const PxU8* item = srcData, *end = srcData + numBytes;
			item < end;
			item += sizeof(smtype), destData += sizeof(lgtype) )
		marshalSingleT<smtype,lgtype>( item, destData );
}


#define PVD_TYPE_MARSHALLER(smtype, lgtype)												\
	template<> struct PvdMarshalling<smtype,lgtype>										\
	{																					\
		PxU32 canMarshal;																\
		static void marshalSingle( const PxU8* srcData, PxU8* destData )				\
		{ marshalSingleT<smtype,lgtype>( srcData, destData ); }							\
		static void marshalBlock( const PxU8* srcData, PxU8* destData, PxU32 numBytes )	\
		{ marshalBlockT<smtype,lgtype>( srcData, destData, numBytes ); }				\
	};

//define marshalling tables.
PVD_TYPE_MARSHALLER(PxI8,PxI16)
PVD_TYPE_MARSHALLER(PxI8,PxU16)
PVD_TYPE_MARSHALLER(PxI8,PxI32)
PVD_TYPE_MARSHALLER(PxI8,PxU32)
PVD_TYPE_MARSHALLER(PxI8,PxI64)
PVD_TYPE_MARSHALLER(PxI8,PxU64)
PVD_TYPE_MARSHALLER(PxI8,PxF32)
PVD_TYPE_MARSHALLER(PxI8,PxF64)

PVD_TYPE_MARSHALLER(PxU8,PxI16)
PVD_TYPE_MARSHALLER(PxU8,PxU16)
PVD_TYPE_MARSHALLER(PxU8,PxI32)
PVD_TYPE_MARSHALLER(PxU8,PxU32)
PVD_TYPE_MARSHALLER(PxU8,PxI64)
PVD_TYPE_MARSHALLER(PxU8,PxU64)
PVD_TYPE_MARSHALLER(PxU8,PxF32)
PVD_TYPE_MARSHALLER(PxU8,PxF64)

PVD_TYPE_MARSHALLER(PxI16,PxI32)
PVD_TYPE_MARSHALLER(PxI16,PxU32)
PVD_TYPE_MARSHALLER(PxI16,PxI64)
PVD_TYPE_MARSHALLER(PxI16,PxU64)
PVD_TYPE_MARSHALLER(PxI16,PxF32)
PVD_TYPE_MARSHALLER(PxI16,PxF64)

PVD_TYPE_MARSHALLER(PxU16,PxI32)
PVD_TYPE_MARSHALLER(PxU16,PxU32)
PVD_TYPE_MARSHALLER(PxU16,PxI64)
PVD_TYPE_MARSHALLER(PxU16,PxU64)
PVD_TYPE_MARSHALLER(PxU16,PxF32)
PVD_TYPE_MARSHALLER(PxU16,PxF64)

PVD_TYPE_MARSHALLER(PxI32,PxI64)
PVD_TYPE_MARSHALLER(PxI32,PxU64)
PVD_TYPE_MARSHALLER(PxI32,PxF64)
PVD_TYPE_MARSHALLER(PxI32,PxF32)

PVD_TYPE_MARSHALLER(PxU32,PxI64)
PVD_TYPE_MARSHALLER(PxU32,PxU64)
PVD_TYPE_MARSHALLER(PxU32,PxF64)
PVD_TYPE_MARSHALLER(PxU32,PxF32)

PVD_TYPE_MARSHALLER(PxF32,PxF64)
PVD_TYPE_MARSHALLER(PxF32,PxU32)
PVD_TYPE_MARSHALLER(PxF32,PxI32)


PVD_TYPE_MARSHALLER(PxU64,PxF64)
PVD_TYPE_MARSHALLER(PxI64,PxF64)
PVD_TYPE_MARSHALLER(PxF64,PxU64)
PVD_TYPE_MARSHALLER(PxF64,PxI64)


template<typename TMarshaller> 
static inline bool getMarshalOperators( TSingleMarshaller&, TBlockMarshaller&, TMarshaller&, bool )
{
	return false;
}

template<typename TMarshaller> 
static inline bool getMarshalOperators( TSingleMarshaller& single, TBlockMarshaller& block, TMarshaller&, PxU32 )
{
	single = TMarshaller::marshalSingle;
	block = TMarshaller::marshalBlock;
	return true;
}

template<typename smtype, typename lgtype> 
static inline bool getMarshalOperators( TSingleMarshaller& single, TBlockMarshaller& block )
{
	single = NULL;
	block = NULL;
	PvdMarshalling<smtype,lgtype> marshaller = PvdMarshalling<smtype,lgtype>();
	return getMarshalOperators( single, block, marshaller, marshaller.canMarshal );
}

template <typename smtype>
static inline bool getMarshalOperators( TSingleMarshaller& single, TBlockMarshaller& block, PxI32 lgtypeId )
{
	switch( lgtypeId )
	{
	case PvdBaseType::PxI8: return getMarshalOperators<smtype,PxI8>( single, block );
	case PvdBaseType::PxU8: return getMarshalOperators<smtype,PxU8>( single, block );
	case PvdBaseType::PxI16: return getMarshalOperators<smtype,PxI16>( single, block );
	case PvdBaseType::PxU16: return getMarshalOperators<smtype,PxU16>( single, block );
	case PvdBaseType::PxI32: return getMarshalOperators<smtype,PxI32>( single, block );
	case PvdBaseType::PxU32: return getMarshalOperators<smtype,PxU32>( single, block );
	case PvdBaseType::PxI64: return getMarshalOperators<smtype,PxI64>( single, block );
	case PvdBaseType::PxU64: return getMarshalOperators<smtype,PxU64>( single, block );
	case PvdBaseType::PxF32: return getMarshalOperators<smtype,PxF32>( single, block );
	case PvdBaseType::PxF64: return getMarshalOperators<smtype,PxF64>( single, block );
	}
	return false;
}

static inline bool getMarshalOperators( TSingleMarshaller& single, TBlockMarshaller& block, PxI32 smtypeId, PxI32 lgtypeId )
{
	switch( smtypeId )
	{
	case PvdBaseType::PxI8: return getMarshalOperators<PxI8>( single, block, lgtypeId );
	case PvdBaseType::PxU8: return getMarshalOperators<PxU8>( single, block, lgtypeId );
	case PvdBaseType::PxI16: return getMarshalOperators<PxI16>( single, block, lgtypeId );
	case PvdBaseType::PxU16: return getMarshalOperators<PxU16>( single, block, lgtypeId );
	case PvdBaseType::PxI32: return getMarshalOperators<PxI32>( single, block, lgtypeId );
	case PvdBaseType::PxU32: return getMarshalOperators<PxU32>( single, block, lgtypeId );
	case PvdBaseType::PxI64: return getMarshalOperators<PxI64>( single, block, lgtypeId );
	case PvdBaseType::PxU64: return getMarshalOperators<PxU64>( single, block, lgtypeId );
	case PvdBaseType::PxF32: return getMarshalOperators<PxF32>( single, block, lgtypeId );
	case PvdBaseType::PxF64: return getMarshalOperators<PxF64>( single, block, lgtypeId );
	}
	return false;	
}

}}


#endif
