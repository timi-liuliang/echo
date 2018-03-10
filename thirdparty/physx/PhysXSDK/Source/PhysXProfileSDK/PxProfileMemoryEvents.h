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


#ifndef PX_PHYSX_MEMORY_EVENTS_H
#define PX_PHYSX_MEMORY_EVENTS_H
#include "PxProfileEvents.h"

//Memory events define their own event stream

namespace physx { namespace profile {
	struct MemoryEventTypes
	{
		enum Enum
		{
			Unknown = 0,
			StringTableEvent, //introduce a new mapping of const char* -> integer
			AllocationEvent,
			DeallocationEvent,
			FullAllocationEvent
		};
	};

	template<unsigned numBits, typename TDataType>
	inline unsigned char convertToNBits( TDataType inType )
	{
		PxU8 conversion = static_cast<PxU8>( inType );
		PX_ASSERT( conversion < (1 << numBits) );
		return conversion;
	}

	template<typename TDataType>
	inline unsigned char convertToTwoBits( TDataType inType )
	{
		return convertToNBits<2>( inType );
	}

	template<typename TDataType>
	inline unsigned char convertToFourBits( TDataType inType )
	{
		return convertToNBits<4>( inType );
	}

	inline EventStreamCompressionFlags::Enum fromNumber( PxU8 inNum ) { return static_cast<EventStreamCompressionFlags::Enum>( inNum ); } 
	
	template<unsigned lhs, unsigned rhs>
	inline void compileCheckSize()
	{
		PX_COMPILE_TIME_ASSERT( lhs <= rhs );
	}

	//Used for predictable bit fields.
	template<typename TDataType
			, PxU8 TNumBits
			, PxU8 TOffset
			, typename TInputType>
	struct BitMaskSetter
	{
		//Create a mask that masks out the orginal value shift into place
		static TDataType createOffsetMask() { return TDataType(createMask() << TOffset); }
		//Create a mask of TNumBits number of tis
		static TDataType createMask() { return static_cast<TDataType>((1 << TNumBits) - 1); }
		void setValue( TDataType& inCurrent, TInputType inData )
		{
			PX_ASSERT( inData < ( 1 << TNumBits ) );
			
			//Create a mask to remove the current value.
			TDataType theMask = TDataType(~(createOffsetMask()));
			//Clear out current value.
			inCurrent = TDataType(inCurrent & theMask);
			//Create the new value.
			TDataType theAddition = (TDataType)( inData << TOffset );
			//or it into the existing value.
			inCurrent = TDataType(inCurrent | theAddition);
		}

		TInputType getValue( TDataType inCurrent )
		{
			return static_cast<TInputType>( ( inCurrent >> TOffset ) & createMask() );
		}
	};


	struct MemoryEventHeader
	{
		PxU16 mValue;

		typedef BitMaskSetter<PxU16, 4, 0, PxU8> TTypeBitmask;
		typedef BitMaskSetter<PxU16, 2, 4, PxU8> TAddrCompressBitmask;
		typedef BitMaskSetter<PxU16, 2, 6, PxU8> TTypeCompressBitmask;
		typedef BitMaskSetter<PxU16, 2, 8, PxU8> TFnameCompressBitmask;
		typedef BitMaskSetter<PxU16, 2, 10, PxU8> TSizeCompressBitmask;
		typedef BitMaskSetter<PxU16, 2, 12, PxU8> TLineCompressBitmask;

		//That leaves size as the only thing not compressed usually.

		MemoryEventHeader( MemoryEventTypes::Enum inType = MemoryEventTypes::Unknown ) 
			: mValue( 0 )
		{
			PxU8 defaultCompression( convertToTwoBits( EventStreamCompressionFlags::U64 ) );
			TTypeBitmask().setValue( mValue, convertToFourBits( inType ) );
			TAddrCompressBitmask().setValue( mValue, defaultCompression );
			TTypeCompressBitmask().setValue( mValue, defaultCompression );
			TFnameCompressBitmask().setValue( mValue, defaultCompression );
			TSizeCompressBitmask().setValue( mValue, defaultCompression );
			TLineCompressBitmask().setValue( mValue, defaultCompression );
		}

		MemoryEventTypes::Enum getType() const { return static_cast<MemoryEventTypes::Enum>( TTypeBitmask().getValue( mValue ) ); }

#define DEFINE_MEMORY_HEADER_COMPRESSION_ACCESSOR( name )																			\
	void set##name( EventStreamCompressionFlags::Enum inEnum ) { T##name##Bitmask().setValue( mValue, convertToTwoBits( inEnum ) ); }	\
		EventStreamCompressionFlags::Enum get##name() const { return fromNumber( T##name##Bitmask().getValue( mValue ) ); }

		DEFINE_MEMORY_HEADER_COMPRESSION_ACCESSOR( AddrCompress )
		DEFINE_MEMORY_HEADER_COMPRESSION_ACCESSOR( TypeCompress )
		DEFINE_MEMORY_HEADER_COMPRESSION_ACCESSOR( FnameCompress )
		DEFINE_MEMORY_HEADER_COMPRESSION_ACCESSOR( SizeCompress )
		DEFINE_MEMORY_HEADER_COMPRESSION_ACCESSOR( LineCompress )

#undef DEFINE_MEMORY_HEADER_COMPRESSION_ACCESSOR

		bool operator==( const MemoryEventHeader& inOther ) const 
		{ 
			return mValue == inOther.mValue; 
		}
		template<typename TStreamType>
		void streamify( TStreamType& inStream ) 
		{ 
			inStream.streamify( "Header", mValue );
		}
	};
	
	//Declaration of type level getMemoryEventType function that maps enumeration event types to datatypes
	template<typename TDataType>
	inline MemoryEventTypes::Enum getMemoryEventType() { PX_ASSERT( false ); return MemoryEventTypes::Unknown; }

	inline bool safeStrEq( const char* lhs, const char* rhs )
	{
		if ( lhs == rhs )
			return true;
		//If they aren't equal, and one of them is null,
		//then they can't be equal.
		//This is assuming that the null char* is not equal to
		//the empty "" char*.
		if ( !lhs || !rhs )
			return false;

		return strcmp( lhs, rhs ) == 0;
	}

	struct StringTableEvent
	{
		const char* mString;
		PxU32		mHandle;

		void init( const char* inStr = "", PxU32 inHdl = 0 )
		{
			mString = inStr;
			mHandle = inHdl;
		}

		void init( const StringTableEvent& inData )
		{
			mString = inData.mString;
			mHandle = inData.mHandle;
		}

		bool operator==( const StringTableEvent& inOther ) const
		{
			return mHandle == inOther.mHandle
				&& safeStrEq( mString, inOther.mString );
		}
		
		void setup( MemoryEventHeader& ) const {}

		template<typename TStreamType>
		void streamify( TStreamType& inStream, const MemoryEventHeader& )
		{
			inStream.streamify( "String", mString );
			inStream.streamify( "Handle", mHandle );
		}
	};
	template<> inline MemoryEventTypes::Enum getMemoryEventType<StringTableEvent>() { return MemoryEventTypes::StringTableEvent; }

	struct MemoryEventData
	{
		PxU64 mAddress;
		void init( PxU64 addr )
		{
			mAddress = addr;
		}

		void init( const MemoryEventData& inData)
		{
			mAddress = inData.mAddress;
		}

		bool operator==( const MemoryEventData& inOther ) const
		{
			return mAddress == inOther.mAddress;
		}
		
		void setup( MemoryEventHeader& inHeader ) const
		{
			inHeader.setAddrCompress( findCompressionValue( mAddress ) );
		}

		template<typename TStreamType>
		void streamify( TStreamType& inStream, const MemoryEventHeader& inHeader )
		{
			inStream.streamify( "Address", mAddress, inHeader.getAddrCompress() );
		}
	};

	struct AllocationEvent : public MemoryEventData
	{
		PxU32 mSize;
		PxU32 mType;
		PxU32 mFile;
		PxU32 mLine;
		void init( size_t size = 0, PxU32 type = 0, PxU32 file = 0, PxU32 line = 0, PxU64 addr = 0 )
		{
			MemoryEventData::init( addr );
			mSize = static_cast<PxU32>( size );
			mType = type;
			mFile = file;
			mLine = line;
		}

		void init( const AllocationEvent& inData )
		{
			MemoryEventData::init( inData );
			mSize = inData.mSize;
			mType = inData.mType;
			mFile = inData.mFile;
			mLine = inData.mLine;
		}

		bool operator==( const AllocationEvent& inOther ) const
		{
			return MemoryEventData::operator==( inOther )
				&& mSize == inOther.mSize
				&& mType == inOther.mType
				&& mFile == inOther.mFile
				&& mLine == inOther.mLine;
		}

		void setup( MemoryEventHeader& inHeader ) const
		{
			inHeader.setTypeCompress( findCompressionValue( mType ) );
			inHeader.setFnameCompress( findCompressionValue( mFile ) );
			inHeader.setSizeCompress( findCompressionValue( mSize ) );
			inHeader.setLineCompress( findCompressionValue( mLine ) );
			MemoryEventData::setup( inHeader );
		}

		template<typename TStreamType>
		void streamify( TStreamType& inStream, const MemoryEventHeader& inHeader )
		{
			inStream.streamify( "Size", mSize, inHeader.getSizeCompress() );
			inStream.streamify( "Type", mType, inHeader.getTypeCompress() );
			inStream.streamify( "File", mFile, inHeader.getFnameCompress() );
			inStream.streamify( "Line", mLine, inHeader.getLineCompress() );
			MemoryEventData::streamify( inStream, inHeader );
		}
	};
	template<> inline MemoryEventTypes::Enum getMemoryEventType<AllocationEvent>() { return MemoryEventTypes::AllocationEvent; }
	

	struct FullAllocationEvent : public MemoryEventData
	{
		size_t mSize;
		const char* mType;
		const char* mFile;
		PxU32 mLine;
		void init( size_t size, const char* type, const char* file, PxU32 line, PxU64 addr )
		{
			MemoryEventData::init( addr );
			mSize = size;
			mType = type;
			mFile = file;
			mLine = line;
		}

		void init( const FullAllocationEvent& inData )
		{
			MemoryEventData::init( inData );
			mSize = inData.mSize;
			mType = inData.mType;
			mFile = inData.mFile;
			mLine = inData.mLine;
		}

		bool operator==( const FullAllocationEvent& inOther ) const
		{
			return MemoryEventData::operator==( inOther )
				&& mSize == inOther.mSize
				&& safeStrEq( mType, inOther.mType )
				&& safeStrEq( mFile, inOther.mFile )
				&& mLine == inOther.mLine;
		}
			
		void setup( MemoryEventHeader& ) const {}
	};

	template<> inline MemoryEventTypes::Enum getMemoryEventType<FullAllocationEvent>() { return MemoryEventTypes::FullAllocationEvent; }

	struct DeallocationEvent : public MemoryEventData
	{
		void init( PxU64 addr = 0 ) { MemoryEventData::init( addr ); }
		void init( const DeallocationEvent& inData ) { MemoryEventData::init( inData ); }
	};
	
	template<> inline MemoryEventTypes::Enum getMemoryEventType<DeallocationEvent>() { return MemoryEventTypes::DeallocationEvent; }

	class MemoryEvent
	{
	public:
		typedef UNION_5(StringTableEvent, AllocationEvent, DeallocationEvent, FullAllocationEvent, PxU8) EventData;

	private:
		MemoryEventHeader mHeader;
		EventData mData;
	public:
		
		MemoryEvent() {}
		MemoryEvent( MemoryEventHeader inHeader, const EventData& inData = EventData() )
			: mHeader( inHeader )
			, mData( inData )
		{
		}

		template<typename TDataType>
		MemoryEvent( const TDataType& inType )
			: mHeader( getMemoryEventType<TDataType>() )
			, mData( inType )
		{
			//set the appropriate compression bits.
			inType.setup( mHeader );
		}
		const MemoryEventHeader& getHeader() const { return mHeader; }
		const EventData& getData() const { return mData; }

		template<typename TDataType>
		const TDataType& getValue() const { PX_ASSERT( mHeader.getType() == getMemoryEventType<TDataType>() ); return mData.toType<TDataType>(); }

		template<typename TDataType>
		TDataType& getValue() { PX_ASSERT( mHeader.getType() == getMemoryEventType<TDataType>() ); return mData.toType<TDataType>(); }

		template<typename TRetVal, typename TOperator>
		inline TRetVal visit( TOperator inOp ) const;

		bool operator==( const MemoryEvent& inOther ) const
		{
			if ( !(mHeader == inOther.mHeader ) ) return false;
			if ( mHeader.getType() )
				return inOther.visit<bool>( EventDataEqualOperator<EventData>( mData ) );
			return true;
		}
	};

	template<typename TRetVal, typename TOperator>
	inline TRetVal visit( MemoryEventTypes::Enum inEventType, const MemoryEvent::EventData& inData, TOperator inOperator )
	{
		switch( inEventType )
		{
		case MemoryEventTypes::StringTableEvent:		return inOperator( inData.toType( Type2Type<StringTableEvent>() ) );
		case MemoryEventTypes::AllocationEvent:			return inOperator( inData.toType( Type2Type<AllocationEvent>() ) );
		case MemoryEventTypes::DeallocationEvent:		return inOperator( inData.toType( Type2Type<DeallocationEvent>() ) );
		case MemoryEventTypes::FullAllocationEvent:		return inOperator( inData.toType( Type2Type<FullAllocationEvent>() ) );
		case MemoryEventTypes::Unknown:
		default: 										return inOperator( static_cast<PxU8>( inEventType ) );
		}
	}

	template<typename TRetVal, typename TOperator>
	inline TRetVal MemoryEvent::visit( TOperator inOp ) const
	{ 
		return physx::profile::visit<TRetVal>( mHeader.getType(), mData, inOp ); 
	}
}}

#endif
