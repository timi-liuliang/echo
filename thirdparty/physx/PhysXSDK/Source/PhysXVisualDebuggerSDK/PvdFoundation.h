/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */



#ifndef PVD_FOUNDATION_H
#define PVD_FOUNDATION_H
#include "foundation/PxAllocatorCallback.h"
#include "foundation/PxMemory.h"
#include "PsArray.h"
#include "PsHashMap.h"
#include "PsHashSet.h"
#include "PsPool.h"
#include "physxvisualdebuggersdk/PvdObjectModelBaseTypes.h"
#include "PvdByteStreams.h"
#include "PxVec3.h"
#include "PxTransform.h"
#include "PxBounds3.h"
#include "PxFiltering.h"
#include "PsString.h"

namespace physx { namespace debugger {
	using namespace physx::shdfnd;

	struct ForwardingAllocator
	{
		PxAllocatorCallback& mAllocator;
		const char* mTypeName;

		ForwardingAllocator( PxAllocatorCallback& alloc, const char* typeName )
			: mAllocator( alloc )
			, mTypeName( typeName )
		{
		}
		ForwardingAllocator( PxAllocatorCallback* alloc = NULL )
			: mAllocator( *alloc )
			, mTypeName( "__error__" )
		{
			PX_ASSERT(false);
		}
		ForwardingAllocator( const ForwardingAllocator& other ) 
			: mAllocator( other.mAllocator )
			, mTypeName( other.mTypeName ) 
		{
		}
		ForwardingAllocator& operator=( const ForwardingAllocator& )
		{
			return *this;
		}
		PxAllocatorCallback& getAllocator() { return mAllocator; }
		void* allocate(size_t size, const char* filename, int line)
		{
			return getAllocator().allocate(size, mTypeName, filename, line);
		}
		void deallocate(void* ptr)
		{
			getAllocator().deallocate(ptr);
		}
		virtual ~ForwardingAllocator(){}
	};


	template<class T>
	struct ForwardingArray : public Array<T, ForwardingAllocator >
	{
		typedef ForwardingAllocator TAllocatorType;

		ForwardingArray( PxAllocatorCallback& alloc, const char* typeName )
			: Array<T, TAllocatorType >( TAllocatorType( alloc, typeName ) )
		{
		}
		
		ForwardingArray( const ForwardingArray< T >& inOther )
			: Array<T, TAllocatorType >( inOther, inOther )
		{
		}
	};

	template<typename TKeyType, typename TValueType, typename THashType=Hash<TKeyType> >
	struct ForwardingHashMap : public HashMap<TKeyType, TValueType, THashType, ForwardingAllocator >
	{
		typedef HashMap<TKeyType, TValueType, THashType, ForwardingAllocator > THashMapType;
		typedef ForwardingAllocator TAllocatorType;
		ForwardingHashMap( PxAllocatorCallback& alloc, const char* typeName )
			: THashMapType( TAllocatorType( alloc, typeName ) )
		{
		}
		ForwardingHashMap( const ForwardingHashMap<TKeyType, TValueType, THashType>& inOther )
			: THashMapType( inOther )
		{
			for ( typename ForwardingHashMap<TKeyType, TValueType, THashType>::Iterator iter = const_cast<ForwardingHashMap<TKeyType, TValueType, THashType>&>(inOther).getIterator();
				iter.done() == false; ++iter )
				insert( iter->first, iter->second );
		}
		ForwardingHashMap<TKeyType,TValueType,THashType>& operator=( const ForwardingHashMap<TKeyType, TValueType, THashType>& inOther )
		{
			if ( this != &inOther )
			{
				HashMap<TKeyType, TValueType, THashType, ForwardingAllocator >::clear();
				for ( typename ForwardingHashMap<TKeyType, TValueType, THashType>::Iterator iter = const_cast<ForwardingHashMap<TKeyType, TValueType, THashType>&>(inOther).getIterator();
					iter.done() == false; ++iter )
					insert( iter->first, iter->second );
			}
			return *this;
		}
	};

	template<typename TValueType, typename THashType=Hash<TValueType> >
	struct ForwardingHashSet : public HashSet<TValueType, THashType, ForwardingAllocator >
	{
		typedef HashSet<TValueType, THashType, ForwardingAllocator > THashSetType;
		typedef ForwardingAllocator TAllocatorType;
		ForwardingHashSet( PxAllocatorCallback& alloc, const char* typeName )
			: THashSetType( TAllocatorType( alloc, typeName ) )
		{
		}
	};

	template<typename TDataType>
	struct ForwardingPool : public Pool<TDataType, ForwardingAllocator >
	{
		ForwardingPool( const ForwardingAllocator& allocator )
			: Pool<TDataType, ForwardingAllocator >( allocator )
		{
		}
	};
	

	template<typename TAllocator = typename AllocatorTraits<PxU8>::Type >
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
		const char* cStr()
		{
			if(mEnd && (*mEnd != 0) )
			   write( (PxU8)0 ); 
			return reinterpret_cast< const char* >(mBegin);
		}
		void write( PxU8 inValue )
		{
			*growBuf( 1 ) = inValue;
		}

		template<typename TDataType>
		void write( const TDataType& inValue )
		{
			const PxU8* __restrict readPtr = reinterpret_cast< const PxU8* >( &inValue );
			PxU8* __restrict writePtr = growBuf( sizeof( TDataType ) );
			for ( PxU32 idx = 0; idx < sizeof(TDataType); ++idx ) writePtr[idx] = readPtr[idx];
		}
		
		template<typename TDataType>
		void write( const TDataType* inValue, PxU32 inLength )
		{
			if ( inValue && inLength )
			{
				PxU32 writeSize = inLength * sizeof( TDataType );
				PxMemCopy( growBuf( writeSize ), inValue, writeSize );
			}
			if ( inLength && !inValue )
			{
				PX_ASSERT( false );
				//You can't not write something, because that will cause
				//the receiving end to crash.
				PxU32 writeSize = inLength * sizeof( TDataType );
				for( PxU32 idx = 0; idx < writeSize; ++idx )
					write( (PxU8)0 );
			}
		}
		
		void writeStrided( const PxU8* __restrict inData, PxU32 inItemSize, PxU32 inLength, PxU32 inStride )
		{
			if ( inStride == 0 || inStride == inItemSize )
				write( inData, inLength * inItemSize );
			else if ( inData && inLength )
			{
				PxU32 writeSize = inLength * inItemSize;
				PxU8* __restrict writePtr = growBuf( writeSize );
				for ( PxU32 idx =0; idx < inLength; ++idx, writePtr += inItemSize, inData += inStride )
					PxMemCopy( writePtr, inData, inItemSize );
			}
		}
		PxU8* growBuf( PxU32 inAmount )
		{
			PxU32 offset = size();
			PxU32 newSize = offset + inAmount;
			reserve( newSize );
			mEnd += inAmount;
			return mBegin + offset;
		}
		void writeZeros( PxU32 inAmount )
		{
			PxU32 offset = size();
			growBuf( inAmount );
			physx::PxMemZero( begin() + offset, inAmount );
		}
		void reserve( PxU32 newSize )
		{
			PxU32 currentSize = size();
			if ( newSize && newSize >= capacity() )
			{
				PxU32 newDataSize = newSize * 2;
				if ( newDataSize > 8192 )
					newDataSize = (PxU32)((PxF32)newSize * 1.2f);
				PxU8* newData = static_cast<PxU8*>( TAllocator::allocate( newDataSize, __FILE__, __LINE__ ) );				
				if ( mBegin )
				{
					PxMemCopy( newData, mBegin, currentSize );
					TAllocator::deallocate( mBegin );
				}
				mBegin = newData;
				mEnd = mBegin + currentSize;
				mCapacityEnd = mBegin + newDataSize;				
			}
		}
	};

	struct ForwardingMemoryBuffer : public MemoryBuffer<ForwardingAllocator>
	{
		ForwardingMemoryBuffer( PxAllocatorCallback& callback, const char* bufDataName )
			: MemoryBuffer<ForwardingAllocator>( ForwardingAllocator( callback, bufDataName ) )
		{
		}

		ForwardingMemoryBuffer& operator << ( const char* inString )
		{
			if ( inString && *inString )
			{
				PxU32 len( strLen( inString ) );
				write( inString, len );
			}
			return *this;
		}		

		template<typename TDataType>
		inline ForwardingMemoryBuffer& toStream( const char* inFormat, const TDataType inData )
		{
			char buffer[128] = { 0 };
			string::sprintf_s( buffer, 128, inFormat, inData );
			*this << buffer;
			return *this;
		}

		inline ForwardingMemoryBuffer& operator << (  bool inData ) { *this << (inData ? "true" : "false"); return *this; }
		inline ForwardingMemoryBuffer& operator << (  PxI32 inData ) { return toStream( "%d",  inData ); }
		inline ForwardingMemoryBuffer& operator << (  PxU16 inData ) {	return toStream( "%u", (PxU32)inData ); }
		inline ForwardingMemoryBuffer& operator << (  PxU8 inData ) {	return toStream( "%u", (PxU32)inData ); }
		inline ForwardingMemoryBuffer& operator << (  char inData ) {	return toStream( "%c", inData ); }
		inline ForwardingMemoryBuffer& operator << (  PxU32 inData ) {	return toStream( "%u", inData ); }
		inline ForwardingMemoryBuffer& operator << (  PxU64 inData ) {	return toStream( "%I64u", inData ); }
		inline ForwardingMemoryBuffer& operator << (  PxI64 inData ) { return toStream( "%I64d",  inData ); }
		inline ForwardingMemoryBuffer& operator << (  const void* inData ) { return  *this << static_cast<PxU64>(reinterpret_cast<size_t>( inData )); }
		inline ForwardingMemoryBuffer& operator << (  PxF32 inData ) { return toStream( "%g", (PxF64)inData ); }
		inline ForwardingMemoryBuffer& operator << (  PxF64 inData ) { return toStream( "%g", inData ); }
		inline ForwardingMemoryBuffer& operator << (  const PxVec3& inData ) 
		{ 
			*this << inData[0];
			*this << " ";
			*this << inData[1];
			*this << " ";
			*this << inData[2];
			return *this;
		}

		inline ForwardingMemoryBuffer& operator << ( const PxQuat& inData ) 
		{
			*this << inData.x;
			*this << " ";
			*this << inData.y;
			*this << " ";
			*this << inData.z;
			*this << " ";
			*this << inData.w;
			return *this;
		}

		inline ForwardingMemoryBuffer& operator << ( const PxTransform& inData ) 
		{
			*this << inData.q;
			*this << " ";
			*this << inData.p;
			return *this;
		}

		inline ForwardingMemoryBuffer& operator << ( const PxBounds3& inData )
		{
			*this << inData.minimum;
			*this << " ";
			*this << inData.maximum;
			return *this;
		}

		inline ForwardingMemoryBuffer& operator << ( const PxFilterData& inData )
		{
			*this << inData.word0 << " " << inData.word1 << " " << inData.word2 << " " << inData.word3;
			return *this;
		}
	};
	

	template< typename TObjectType
			, typename TGetSetIndexOp
			, typename TSetSetIndexOp>
	class InvasiveSet
	{
		ForwardingArray<TObjectType*> mSet;

		InvasiveSet( const InvasiveSet& other );
		InvasiveSet& operator=( const InvasiveSet& other );

	public:

		InvasiveSet( PxAllocatorCallback& callback, const char* allocName ) 
			: mSet( callback, allocName )
		{
		}

		bool insert( TObjectType& inObject )
		{
			PxU32 currentIdx = TGetSetIndexOp()( inObject );
			if ( currentIdx == PX_MAX_U32 )
			{
				TSetSetIndexOp()( inObject, mSet.size() );
				mSet.pushBack( &inObject );
				return true;
			}
			return false;
		}

		bool remove( TObjectType& inObject )
		{
			PxU32 currentIdx = TGetSetIndexOp()( inObject );
			if ( currentIdx != PX_MAX_U32 )
			{
				TObjectType* theEnd = mSet.back();
				TObjectType* theObj = &inObject;
				if ( theEnd != theObj )
				{
					TSetSetIndexOp()( *theEnd, currentIdx );
					mSet[currentIdx] = theEnd;
				}
				mSet.popBack();
				TSetSetIndexOp()( inObject, PX_MAX_U32 );
				return true;
			}
			return false;
		}

		bool contains( TObjectType& inObject ) { return TGetSetIndexOp()( inObject ) != PX_MAX_U32; }

		void clear()
		{
			for ( PxU32 idx = 0; idx < mSet.size(); ++idx )
				TSetSetIndexOp()( *(mSet[idx]), PX_MAX_U32 );
			mSet.clear();
		}
		
		TObjectType* operator[]( PxU32 idx ) { return mSet[idx]; }
		const TObjectType* operator[] ( PxU32 idx ) const { return mSet[idx]; }
		PxU32 size() const { return mSet.size(); }
		TObjectType** begin() { return mSet.begin(); }
		TObjectType** end() { return mSet.end(); }
		const TObjectType** begin() const { return mSet.begin(); }
		const TObjectType** end() const { return mSet.end(); }
		const TObjectType* back() const { return mSet.back(); }
		TObjectType* back() { return mSet.back(); }
	};

	template<typename TDataType>
	inline TDataType* PvdAllocate( PxAllocatorCallback& allocator, const char* typeName, const char* file, int line )
	{
		return reinterpret_cast<TDataType*>( allocator.allocate( sizeof( TDataType ), typeName, file, line ) );
	}

	template<typename TDataType>
	inline void PvdDeleteAndDeallocate( PxAllocatorCallback& allocator, TDataType* inDType )
	{
		if ( inDType )
		{
			inDType->~TDataType();
			allocator.deallocate( inDType );
		}
	}
}}

#define PVD_NEW( allocator, dtype ) new (PvdAllocate<dtype>( allocator, #dtype, __FILE__, __LINE__ )) dtype
#define PVD_DELETE( allocator, obj ) PvdDeleteAndDeallocate( allocator, obj );
//Construct containers that tell us what they contain.
#define PVD_CONSTRUCT_CONTAINER( name, allocator, type ) name( allocator, #type )
#define PVD_FOREACH( varname, stop ) for( PxU32 varname = 0; varname < stop; ++varname )

namespace physx { namespace debugger {
	
	template<typename TKeyType, typename TValueType, typename THashType, typename TBufType, typename TOperator>
	PxU32 getMapKeysOp( ForwardingHashMap<TKeyType,TValueType,THashType>& map, TBufType* buffer, PxU32 bufSize, PxU32 startIdx, TOperator op )
	{
		PxU32 numItems = static_cast<PxU32>( map.size() );
		if ( numItems == 0 || bufSize == 0 ) return 0;

		startIdx = PxMin( numItems - 1, startIdx );
		PxU32 retval = 0;
		for ( typename ForwardingHashMap<TKeyType,TValueType,THashType>::Iterator iter = map.getIterator();
			iter.done() == false && bufSize; ++iter )
		{
			if ( startIdx ) --startIdx;
			else
			{
				buffer[retval] = op(iter->first);
				--bufSize;
				++retval;
			}
		}
		return retval;
	}

	struct IdOp
	{
		template<typename TDataType> TDataType operator()( const TDataType& item ) { return item; }
	};

	template<typename TKeyType, typename TValueType, typename THashType>
	PxU32 getMapKeys( ForwardingHashMap<TKeyType,TValueType,THashType>& map, TKeyType* buffer, PxU32 bufSize, PxU32 startIdx )
	{
		return getMapKeysOp( map, buffer, bufSize, startIdx, IdOp() );
	}

	struct DerefOp
	{
		template<typename TDataType> TDataType operator()( const TDataType* item ) { return *item; }
	};
	
	template<typename TKeyType, typename TValueType, typename TBufType, typename TOp>
	PxU32 getMapValues( ForwardingHashMap<TKeyType,TValueType>& map, TBufType* buffer, PxU32 bufSize, PxU32 startIdx, TOp op )
	{
		PxU32 numItems = static_cast<PxU32>( map.size() );
		if ( numItems == 0 || bufSize == 0 ) return 0;

		startIdx = PxMin( numItems - 1, startIdx );
		PxU32 retval = 0;
		for ( typename ForwardingHashMap<TKeyType,TValueType>::Iterator iter = map.getIterator();
			iter.done() == false && bufSize; ++iter )
		{
			if ( startIdx ) --startIdx;
			else
			{
				buffer[retval] = op(iter->second);
				--bufSize;
				++retval;
			}
		}
		return retval;
	}

	template<typename TValueType, typename TBufType>
	PxU32 getArrayEntries( ForwardingArray<TValueType>& data, TBufType* buffer, PxU32 bufSize, PxU32 startIdx )
	{
		PxU32 numItems = static_cast<PxU32>( data.size() );
		if ( numItems == 0 || bufSize == 0 ) return 0;

		startIdx = PxMin( numItems - 1, startIdx );
		PxU32 available = PxMin( numItems - startIdx, bufSize );
		PVD_FOREACH( idx, available )
			buffer[idx] = data[idx + startIdx];
		return available;
	}
}}

#endif
