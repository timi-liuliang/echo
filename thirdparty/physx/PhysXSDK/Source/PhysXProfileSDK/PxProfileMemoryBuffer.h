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


#ifndef PX_PHYSX_PROFILE_MEMORY_BUFFER_H
#define PX_PHYSX_PROFILE_MEMORY_BUFFER_H
#include "PxProfileBase.h"
#include "PsAllocator.h"
#include "PxMemory.h"

namespace physx { namespace profile {

	template<typename TAllocator = typename shdfnd::AllocatorTraits<PxU8>::Type >
	class MemoryBuffer : public TAllocator
	{
		PxU8* mBegin;
		PxU8* mEnd;
		PxU8* mCapacityEnd;

	public:
		MemoryBuffer( const TAllocator& inAlloc = TAllocator() ) : TAllocator( inAlloc ), mBegin( 0 ), mEnd( 0 ), mCapacityEnd( 0 ) {}
		~MemoryBuffer()
		{
			if ( mBegin ) TAllocator::deallocate( mBegin );
		}
		PxU32 size() const { return static_cast<PxU32>( mEnd - mBegin ); }
		PxU32 capacity() const { return static_cast<PxU32>( mCapacityEnd - mBegin ); }
		PxU8* begin() { return mBegin; }
		PxU8* end() { return mEnd; }
		const PxU8* begin() const { return mBegin; }
		const PxU8* end() const { return mEnd; }
		void clear() { mEnd = mBegin; }
		void write( PxU8 inValue )
		{
			growBuf( 1 );
			*mEnd = inValue;
			++mEnd;
		}

		template<typename TDataType>
		void write( const TDataType& inValue )
		{
			growBuf( sizeof( TDataType ) );
			const PxU8* __restrict readPtr = reinterpret_cast< const PxU8* >( &inValue );
			PxU8* __restrict writePtr = mEnd;
			for ( PxU32 idx = 0; idx < sizeof(TDataType); ++idx ) writePtr[idx] = readPtr[idx];
			mEnd += sizeof(TDataType);
		}
		
		template<typename TDataType>
		void write( const TDataType* inValue, PxU32 inLength )
		{
			if ( inValue && inLength )
			{
				PxU32 writeSize = inLength * sizeof( TDataType );
				growBuf( writeSize );
				PxMemCopy( mBegin + size(), inValue, writeSize );
				mEnd += writeSize;
			}
		}
		
		void writeStrided( const PxU8* __restrict inData, PxU32 inItemSize, PxU32 inLength, PxU32 inStride )
		{
			if ( inStride == 0 || inStride == inItemSize )
				write( inData, inLength * inItemSize );
			else if ( inData && inLength )
			{
				PxU32 writeSize = inLength * inItemSize;
				growBuf( writeSize );
				PxU8* __restrict writePtr = mBegin + size();
				for ( PxU32 idx =0; idx < inLength; ++idx, writePtr += inItemSize, inData += inStride )
					PxMemCopy( writePtr, inData, inItemSize );
				mEnd += writeSize;
			}
		}
		void growBuf( PxU32 inAmount )
		{
			PxU32 newSize = size() + inAmount;
			reserve( newSize );
		}
		void resize( PxU32 inAmount )
		{
			reserve( inAmount );
			mEnd = mBegin + inAmount;
		}
		void reserve( PxU32 newSize )
		{
			PxU32 currentSize = size();
			if ( newSize >= capacity() )
			{
				PxU8* newData = static_cast<PxU8*>( TAllocator::allocate( newSize * 2, __FILE__, __LINE__ ) );
				if ( mBegin )
				{
					PxMemCopy( newData, mBegin, currentSize );
					TAllocator::deallocate( mBegin );
				}
				mBegin = newData;
				mEnd = mBegin + currentSize;
				mCapacityEnd = mBegin + newSize * 2;
			}
		}
	};
}}
#endif
