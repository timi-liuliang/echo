/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */



#ifndef PVD_BYTE_STREAMS_H
#define PVD_BYTE_STREAMS_H
#include "physxvisualdebuggersdk/PvdObjectModelBaseTypes.h"

namespace physx { namespace debugger {

	static inline PxU32 strLen( const char* inStr )
	{
		PxU32 len = 0;
		if ( inStr )
		{
			while ( *inStr )
			{
				++len;
				++inStr;
			}
		}
		return len;
	}

	class PvdInputStream
	{
	protected:
		virtual ~PvdInputStream(){}
	public:
		
		//Return false if you can't write the number of bytes requested
		//But make an absolute best effort to read the data...
		virtual bool read( PxU8* buffer, PxU32& len ) = 0;

		template<typename TDataType>
		bool read( TDataType* buffer, PxU32 numItems )
		{
			PxU32 expected = numItems;
			PxU32 amountToRead = numItems * sizeof( TDataType );
			read( reinterpret_cast<PxU8*>( buffer ), amountToRead );
			numItems = amountToRead / sizeof( TDataType );
			PX_ASSERT( numItems == expected );
			return expected == numItems;
		}

		template<typename TDataType>
		PvdInputStream& operator >> ( TDataType& data )
		{
			PxU32 dataSize = static_cast<PxU32>( sizeof( TDataType ) );
			bool success = read( reinterpret_cast<PxU8*>( &data ), dataSize );
			//PX_ASSERT( success );
			//PX_ASSERT( dataSize == sizeof( data ) );
			(void)success;
			return *this;
		}
	};

	struct ByteSwappingPvdInputStream
	{
	protected:
		ByteSwappingPvdInputStream& operator=( ByteSwappingPvdInputStream& other );
	public:
		PvdInputStream& mStream;
		ByteSwappingPvdInputStream( PvdInputStream& stream ) : mStream( stream ) {}

		template<typename TDataType>
		bool read( TDataType* buffer, PxU32& numItems )
		{
			bool retval = mStream.read( buffer, numItems );
			for ( PxU32 idx = 0; idx < numItems; ++idx )
				swapBytes( buffer[idx] );
			return retval;
		}

		template<typename TDataType>
		ByteSwappingPvdInputStream& operator >> ( TDataType& data )
		{
			mStream >> data;
			swapBytes( data );
			return *this;
		}
	};

	class PvdOutputStream
	{
	protected:
		virtual ~PvdOutputStream(){}
	public:
		//Return false if you can't write the number of bytes requested
		//But make an absolute best effort to write the data...
		virtual bool write( const PxU8* buffer, PxU32 len ) = 0;
		virtual bool directCopy( PvdInputStream& inStream, PxU32 len ) = 0;

		template<typename TDataType>
		bool write( const TDataType* buffer, PxU32 numItems )
		{
			return write( reinterpret_cast<const PxU8*>( buffer ), numItems * sizeof( TDataType ) );
		}

		template<typename TDataType>
		PvdOutputStream& operator << ( const TDataType& data )
		{
			bool success = write( reinterpret_cast< const PxU8* >( &data ), sizeof( data ) );
			PX_ASSERT( success );
			(void)success;
			return *this;
		}

		PvdOutputStream& operator << ( const char* inString )
		{
			if ( inString && *inString )
			{
				PxU32 len( strLen( inString ) );
				write( inString, len );
			}
			return *this;
		}		
	};
}}
#endif
