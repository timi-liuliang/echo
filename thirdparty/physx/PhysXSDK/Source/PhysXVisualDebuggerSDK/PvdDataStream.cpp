/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
#include "PvdDataStream.h"
#include "PvdFoundation.h"
#include "PvdCommStreamEvents.h"
#include "PvdCommStreamEventSink.h"
#include "PsTime.h"
#include "PvdCommStreamTypes.h"
#include "PvdDataStreamHelpers.h"
#include "PvdObjectModelInternalTypes.h"

using namespace physx::debugger;
using namespace physx::debugger::comm;

namespace 
{

	struct ScopedMetaData
	{
		PvdOMMetaDataProvider&	mProvider;
		PvdObjectModelMetaData& mMeta;
		ScopedMetaData( PvdOMMetaDataProvider& provider )
			: mProvider( provider )
			, mMeta( provider.lock() )
		{
		}
		~ScopedMetaData()
		{
			mProvider.unlock();
		}
		PvdObjectModelMetaData* operator->() { return &mMeta; }
	private:
		ScopedMetaData& operator=(const ScopedMetaData&);
	};
	
	struct PropertyDefinitionHelper : public PvdPropertyDefinitionHelper
	{
		PvdDataStream*						mStream;
		PvdOMMetaDataProvider&				mProvider;
		ForwardingArray<char>				mNameBuffer;
		ForwardingArray<PxU32>				mNameStack;
		ForwardingArray<NamedValue>			mNamedValues;
		ForwardingArray<PropertyMessageArg>	mPropertyMessageArgs;

		PropertyDefinitionHelper( PxAllocatorCallback& alloc, PvdOMMetaDataProvider& provider )
			: mStream( NULL )
			, mProvider( provider )
			, mNameBuffer( alloc, "PropertyDefinitionHelper::mNameBuffer" )
			, mNameStack( alloc, "PropertyDefinitionHelper::mNameStack" )
			, mNamedValues( alloc, "PropertyDefinitionHelper::mNamedValues" )
			, mPropertyMessageArgs( alloc, "PropertyDefinitionHelper::mPropertyMessageArgs" )
		{
		}
		void setStream( PvdDataStream* stream ) { mStream = stream; }

		inline void appendStrToBuffer( const char* str )
		{
			if ( str == NULL )
				return;
			size_t strLen = strlen( str );
			size_t endBufOffset = mNameBuffer.size();
			size_t resizeLen = endBufOffset;
			//account for null
			if ( mNameBuffer.empty() )
				resizeLen += 1;
			else
				endBufOffset -=1;

			mNameBuffer.resize( static_cast<PxU32>( resizeLen + strLen ) );
			char* endPtr = mNameBuffer.begin() + endBufOffset;
			memcpy( endPtr, str, strLen );
		}

		virtual void pushName( const char* nm, const char* appender = "." ) 
		{ 
			size_t nameBufLen = mNameBuffer.size();
			mNameStack.pushBack( static_cast<PxU32>( nameBufLen ) );
			if ( mNameBuffer.empty() == false )
				appendStrToBuffer( appender );
			appendStrToBuffer( nm );
			mNameBuffer.back() = 0;
		}
		
		virtual void pushBracketedName( const char* inName, const char* leftBracket = "[", const char* rightBracket = "]" )
		{
			size_t nameBufLen = mNameBuffer.size();
			mNameStack.pushBack( static_cast<PxU32>( nameBufLen ) );
			appendStrToBuffer( leftBracket );
			appendStrToBuffer( inName );
			appendStrToBuffer( rightBracket );
			mNameBuffer.back() = 0;
		}

		virtual void popName()
		{
			if ( mNameStack.empty() )
				return;
			mNameBuffer.resize( static_cast<PxU32>( mNameStack.back() ) );
			mNameStack.popBack();
			if ( mNameBuffer.empty() == false )
				mNameBuffer.back() = 0;
		}

		virtual const char* getTopName()
		{
			if ( mNameBuffer.size() )
				return mNameBuffer.begin();
			return "";
		}
		virtual void clearNameStack()
		{
			mNameBuffer.clear();
			mNameStack.clear();
		}
		
		virtual void addNamedValue( const char* name, PxU32 value )
		{
			mNamedValues.pushBack( NamedValue( name, value ) );
		}
		virtual void clearNamedValues() { mNamedValues.clear(); }

		virtual DataRef<NamedValue> getNamedValues() { return DataRef<NamedValue>( mNamedValues.begin(), mNamedValues.size() ); }
		
		virtual void createProperty( const NamespacedName& clsName, const char* inSemantic, const NamespacedName& dtypeName
									, PropertyType::Enum propType )
		{
			mStream->createProperty( clsName, getTopName(), inSemantic, dtypeName, propType, getNamedValues() );
			clearNamedValues();
		}
		const char* registerStr( const char* str )
		{
			ScopedMetaData scopedProvider( mProvider );
			return scopedProvider->getStringTable().registerStr( str );
		}
		virtual void addPropertyMessageArg( const NamespacedName& inDatatype, PxU32 inOffset, PxU32 inSize )
		{
			mPropertyMessageArgs.pushBack( PropertyMessageArg( registerStr( getTopName() ), inDatatype, inOffset, inSize ) );
		}
		virtual void addPropertyMessage( const NamespacedName& clsName, const NamespacedName& msgName, PxU32 inStructSizeInBytes )
		{
			if ( mPropertyMessageArgs.empty() ) { PX_ASSERT( false ); return; }
			mStream->createPropertyMessage( clsName, msgName, DataRef<PropertyMessageArg>( mPropertyMessageArgs.begin(), mPropertyMessageArgs.size() ), inStructSizeInBytes );
		}
		virtual void clearPropertyMessageArgs() { mPropertyMessageArgs.clear(); }
	private:
		PropertyDefinitionHelper& operator=(const PropertyDefinitionHelper&);
	};

	class PvdMemPool
	{
		PxAllocatorCallback&			mAllocator;
		//Link List
		ForwardingArray<PxU8*>			mMemBuffer;
		PxU32							mLength;
		PxU32							mBufIndex;

		//4k for one page
		static const int BUFFER_LENGTH = 4096;
		PX_NOCOPY(PvdMemPool)
	public:

		PvdMemPool(PxAllocatorCallback& alloc, const char* bufDataName)
			:mAllocator(alloc)
			, mMemBuffer( alloc, bufDataName )
			, mLength(0)
			, mBufIndex(0)
		{
			grow();
		}

		~PvdMemPool()
		{
			for(PxU32 i = 0; i < mMemBuffer.size(); i++)
			{
				mAllocator.deallocate(mMemBuffer[i]);
			}
		}
		
		void grow()
		{
			if(mBufIndex + 1 < mMemBuffer.size())
			{
				mBufIndex++;
			}
			else
			{
				PxU8* Buf = (PxU8*)mAllocator.allocate(BUFFER_LENGTH, "PvdMemPool::mMemBuffer.buf", __FILE__, __LINE__);
				mMemBuffer.pushBack(Buf);
				mBufIndex = mMemBuffer.size() - 1;
			}
			mLength = 0;
		}

		void* allocate(PxU32 length)
		{
			if(length > (PxU32)BUFFER_LENGTH)
				return NULL;

			if(length + mLength > (PxU32)BUFFER_LENGTH)
				grow();

			void* mem = (void*)&mMemBuffer[mBufIndex][mLength];
			mLength += length;
			return mem;
		}

		void clear()
		{
			mLength = 0;
			mBufIndex = 0;
		}

	};
	struct DataStream : public PvdCommStreamInternalDataStream
	{
		PxAllocatorCallback&			mAllocator;
		ForwardingHashMap<String,PxU32> mStringHashMap;
		PvdOMMetaDataProvider&			mMetaDataProvider;
		PxU32							mRefCount;
		ForwardingArray<PxU8>			mTempBuffer;
		PropertyDefinitionHelper		mPropertyDefinitionHelper;
		DataStreamState::Enum			mStreamState;
		
		PxU32							mBufferCutoff;
		ForwardingMemoryBuffer			mBuffer;
		ClassDescription				mSPVClass;
		PropertyMessageDescription		mMessageDesc;
		//Set property value and SetPropertyMessage calls require
		//us to write the data out to a separate buffer
		//when strings are involved.
		ForwardingMemoryBuffer			mSPVBuffer;
		PxU32							mEventCount;
		PxU32							mPropertyMessageSize;
		PvdCommStreamEventBufferClient&	mClient;
		bool							mConnected;
		PxU64							mStreamId;
		bool							mCacheEnabled;
		ForwardingArray<PvdCommand*>
										mPvdCommandArray;
		PvdMemPool						mPvdCommandPool;
		

		DataStream( PxAllocatorCallback& alloc, PvdOMMetaDataProvider& provider, PxU32 bufferSize, PvdCommStreamEventBufferClient& client, PxU64 streamId )
			: mAllocator( alloc )
			, mStringHashMap( alloc, "DataStream::mStringHashMap" )
			, mMetaDataProvider( provider )
			, mRefCount( 0 )
			, mTempBuffer( alloc, "DataStream::mTempBuffer" )
			, mPropertyDefinitionHelper( alloc, provider )
			, mStreamState( DataStreamState::Open )
			, mBufferCutoff( bufferSize )
			, mBuffer( alloc, "PvdCommStreamBufferedEventSink::mBuffer" )
			, mSPVBuffer( alloc, "PvdCommStreamBufferedEventSink::mSPVBuffer" )
			, mEventCount( 0 )
			, mPropertyMessageSize( 0 )
			, mClient( client )
			, mConnected( true )
			, mStreamId( streamId )
			, mCacheEnabled( true )
			, mPvdCommandArray( alloc, "PvdCommStreamBufferedEventSink::mPvdCommandArray" )
			, mPvdCommandPool( alloc, "PvdCommStreamBufferedEventSink::mPvdCommandPool" )
		{
			mMetaDataProvider.addRef();
			mClient.addRef();
			mPropertyDefinitionHelper.setStream( this );
		}
		virtual ~DataStream()
		{
			mMetaDataProvider.release();
			mClient.release();
		}

		virtual void addRef() { ++mRefCount; }
		virtual void release()
		{
			if ( mRefCount ) --mRefCount;
			if ( !mRefCount ) PVD_DELETE( mAllocator, this );
		}

		StringHandle toStream( String nm )
		{
			if ( nm == NULL || *nm == 0 ) return 0;
			const ForwardingHashMap<String,PxU32>::Entry* entry( mStringHashMap.find( nm ) );
			if ( entry ) return entry->second;
			ScopedMetaData meta( mMetaDataProvider );
			StringHandle hdl = meta->getStringTable().strToHandle( nm );
			nm = meta->getStringTable().handleToStr( hdl );
			handlePvdEvent( StringHandleEvent( nm, hdl ) );
			mStringHashMap.insert( nm, hdl );
			return hdl;
		}

		StreamNamespacedName toStream( const NamespacedName& nm )
		{
			return StreamNamespacedName( toStream( nm.mNamespace ), toStream( nm.mName ) );
		}


		//You will notice that some functions are #pragma'd out throughout this file.
		//This is because they are only called from asserts which means they aren't
		//called in release.  This causes warnings when building using snc which break
		//the build.
#ifdef PX_DEBUG

		bool classExists( const NamespacedName& nm )
		{
			ScopedMetaData meta( mMetaDataProvider );
			return meta->findClass( nm ).hasValue();
		}

#endif

		bool createMetaClass( const NamespacedName& nm )
		{
			ScopedMetaData meta( mMetaDataProvider );
			meta->getOrCreateClass( nm );
			return true;
		}

		bool deriveMetaClass( const NamespacedName& parent, const NamespacedName& child )
		{
			ScopedMetaData meta( mMetaDataProvider );
			return meta->deriveClass( parent, child );
		}

#ifdef PX_DEBUG

		bool propertyExists( const NamespacedName& nm, String pname )
		{
			ScopedMetaData meta( mMetaDataProvider );
			return meta->findProperty( nm, pname ).hasValue();
		}

#endif

		PvdError boolToError( bool val ) { if ( val ) return PvdErrorType::Success; return PvdErrorType::NetworkError; } 

		//PvdMetaDataStream
		virtual PvdError createClass( const NamespacedName& nm )
		{
			PX_ASSERT( mStreamState == DataStreamState::Open );
#ifdef PX_DEBUG
			PX_ASSERT( classExists( nm ) == false );
#endif
			createMetaClass( nm );
			return boolToError( handlePvdEvent( CreateClass( toStream( nm ) ) ) );
		}

		virtual PvdError deriveClass( const NamespacedName& parent, const NamespacedName& child )
		{
			PX_ASSERT( mStreamState == DataStreamState::Open );
#ifdef PX_DEBUG
			PX_ASSERT( classExists( parent ) );
			PX_ASSERT( classExists( child ) );
#endif
			deriveMetaClass( parent, child );
			return boolToError( handlePvdEvent( DeriveClass( toStream( parent ), toStream( child ) ) ) );
		}

		template<typename TDataType>
		TDataType* allocTemp( PxU32 numItems )
		{
			PxU32 desiredBytes = numItems * sizeof( TDataType );
			if ( desiredBytes > mTempBuffer.size() )
				mTempBuffer.resize( desiredBytes );
			TDataType* retval = reinterpret_cast<TDataType*>( mTempBuffer.begin() );
			if ( numItems )
			{
				PVD_FOREACH( idx, numItems ) new ( retval + idx ) TDataType();
			}
			return retval;
		}
		
#ifdef PX_DEBUG


		//Property datatypes need to be uniform.
		//At this point, the data stream cannot handle properties that
		//A struct with a float member and a char member would work.
		//A struct with a float member and a long member would work (more efficiently).
		bool isValidPropertyDatatype( const NamespacedName& dtypeName )
		{
			ScopedMetaData meta( mMetaDataProvider );
			ClassDescription clsDesc( meta->findClass( dtypeName ) );
			return clsDesc.mRequiresDestruction == false;
		}

#endif

		NamespacedName createMetaProperty( const NamespacedName& clsName, String name, String semantic
										, const NamespacedName& dtypeName, PropertyType::Enum propertyType )
		{
			ScopedMetaData meta( mMetaDataProvider );
			NonNegativeInteger dtypeType = meta->findClass( dtypeName )->mClassId;
			NamespacedName typeName = dtypeName;
			if ( dtypeType == getPvdTypeForType<String>() )
			{
				dtypeType = getPvdTypeForType<StringHandle>();
				typeName = getPvdNamespacedNameForType<StringHandle>();
			}
			Option<PropertyDescription> propOpt = meta->createProperty( meta->findClass( clsName )->mClassId, name, semantic, dtypeType, propertyType );
			PX_ASSERT( propOpt.hasValue() );
			PX_UNUSED(propOpt);
			return typeName;
		}

		virtual PvdError createProperty( const NamespacedName& clsName, String name, String semantic
										, const NamespacedName& incomingDtypeName, PropertyType::Enum propertyType
										, DataRef<NamedValue> values )
		{
			PX_ASSERT( mStreamState == DataStreamState::Open );
#ifdef PX_DEBUG
			PX_ASSERT( classExists( clsName ) );
			PX_ASSERT( propertyExists( clsName, name ) == false );
#endif
			NamespacedName dtypeName( incomingDtypeName );
			if ( safeStrEq( dtypeName.mName, "VoidPtr" ) )
				dtypeName.mName = "ObjectRef";
#ifdef PX_DEBUG
			PX_ASSERT( classExists( dtypeName ) );
			PX_ASSERT( isValidPropertyDatatype( dtypeName ) );
#endif
			NamespacedName typeName = createMetaProperty( clsName, name, semantic, dtypeName, propertyType );
			//Can't have arrays of strings or arrays of string handles due to the difficulty
			//of quickly dealing with them on the network receiving side.
			if ( propertyType == PropertyType::Array && safeStrEq( typeName.mName, "StringHandle" ) )
			{
				PX_ASSERT( false );
				return PvdErrorType::ArgumentError;
			}
			PxU32 numItems = values.size();
			NameHandleValue* streamValues = allocTemp<NameHandleValue>( numItems );
			PVD_FOREACH( idx, numItems )
				streamValues[idx] = NameHandleValue( toStream( values[idx].mName ), values[idx].mValue );
			CreateProperty evt( toStream( clsName ), toStream( name ), toStream( semantic ),
							toStream( typeName ), propertyType, DataRef<NameHandleValue>( streamValues, numItems ) );
			return boolToError( handlePvdEvent( evt ) );
		}

		bool createMetaPropertyMessage( const NamespacedName& cls, const NamespacedName& msgName
													, DataRef<PropertyMessageArg> entries, PxU32 messageSizeInBytes )
		{
			ScopedMetaData meta( mMetaDataProvider );
			return meta->createPropertyMessage( cls, msgName, entries, messageSizeInBytes ).hasValue();
		}
#ifdef PX_DEBUG


		bool messageExists( const NamespacedName& msgName )
		{
			ScopedMetaData meta( mMetaDataProvider );
			return meta->findPropertyMessage( msgName ).hasValue();
		}

#endif

		virtual PvdError createPropertyMessage( const NamespacedName& cls, const NamespacedName& msgName
													, DataRef<PropertyMessageArg> entries, PxU32 messageSizeInBytes )
		{
			PX_ASSERT( mStreamState == DataStreamState::Open );
#ifdef PX_DEBUG
			PX_ASSERT( classExists( cls ) );
			PX_ASSERT( messageExists( msgName ) == false );
#endif
			createMetaPropertyMessage( cls, msgName, entries, messageSizeInBytes );
			PxU32 numItems = entries.size();
			StreamPropMessageArg* streamValues = allocTemp<StreamPropMessageArg>( numItems );
			PVD_FOREACH( idx, numItems )
				streamValues[idx] = StreamPropMessageArg( toStream(entries[idx].mPropertyName), toStream( entries[idx].mDatatypeName ), entries[idx].mMessageOffset, entries[idx].mByteSize );
			CreatePropertyMessage evt( toStream( cls ), toStream( msgName ), DataRef<StreamPropMessageArg>( streamValues, numItems ), messageSizeInBytes );
			return boolToError( handlePvdEvent( evt ) );
		}
		
		PxU64 toStream( const void* instance ) { return PVD_POINTER_TO_U64( instance ); }
		virtual PvdError createInstance( const NamespacedName& cls, const void* instance )
		{
			PX_ASSERT( isInstanceValid( instance ) == false );
			PX_ASSERT( mStreamState == DataStreamState::Open );
			bool success = mMetaDataProvider.createInstance( cls, instance );
			PX_ASSERT( success );
			(void)success;
			return boolToError( handlePvdEvent( CreateInstance( toStream( cls ), toStream( instance ) ) ) );
		}

		virtual bool isInstanceValid( const void* instance ) { return mMetaDataProvider.isInstanceValid( instance ); }
		
#ifdef PX_DEBUG

		//If the property will fit or is already completely in memory
		bool checkPropertyType( const void* instance, String name, const NamespacedName& incomingType )
		{
			NonNegativeInteger instType = mMetaDataProvider.getInstanceClassType( instance );
			ScopedMetaData meta( mMetaDataProvider );
			Option<PropertyDescription> prop = meta->findProperty( instType, name );
			if ( prop.hasValue() == false ) return false;
			NonNegativeInteger propType = prop->mDatatype;
			NonNegativeInteger incomingTypeId = meta->findClass( incomingType )->mClassId;
			if ( incomingTypeId != getPvdTypeForType<VoidPtr>() )
			{
				MarshalQueryResult result = meta->checkMarshalling( incomingTypeId, propType );
				bool possible = result.needsMarshalling == false || result.canMarshal;
				return possible;
			}
			else
			{
				if ( propType != getPvdTypeForType<ObjectRef>() )
					return false;
			}
			return true;
		}

#endif

		DataRef<const PxU8> bufferPropertyValue( ClassDescriptionSizeInfo info, DataRef<const PxU8> data )
		{
			PxU32 realSize = info.mByteSize;
			PxU32 numItems = data.size() / realSize;
			if ( info.mPtrOffsets.size() != 0 )
			{
				mSPVBuffer.clear();
				PVD_FOREACH( item, numItems )
				{
					const PxU8* itemPtr = data.begin() + item * realSize;
					mSPVBuffer.write( itemPtr, realSize );
					PVD_FOREACH( stringIdx, info.mPtrOffsets.size() )
					{
						PtrOffset offset( info.mPtrOffsets[stringIdx] );
						if ( offset.mOffsetType == PtrOffsetType::VoidPtrOffset )
							continue;
						const char* strPtr;
						PxMemCopy( &strPtr, itemPtr + offset.mOffset, sizeof( char* ) );
						strPtr = nonNull( strPtr );
						PxU32 len = safeStrLen( strPtr ) + 1;
						mSPVBuffer.write( strPtr, len );
					}
				}
				data = DataRef<const PxU8>( mSPVBuffer.begin(), mSPVBuffer.size() );
			}
			return data;
		}

		virtual PvdError setPropertyValue( const void* instance, String name, DataRef<const PxU8> data, const NamespacedName& incomingTypeName ) 
		{

			PX_ASSERT( isInstanceValid( instance ) );
#ifdef PX_DEBUG
			PX_ASSERT( classExists( incomingTypeName ) );
#endif
			PX_ASSERT( mStreamState == DataStreamState::Open );
			ClassDescription clsDesc;
			{
				ScopedMetaData meta( mMetaDataProvider );
				clsDesc = meta->findClass( incomingTypeName );
			} 
			PxU32 realSize = clsDesc.getNativeSize();
			PxU32 numItems = data.size() / realSize;
			data = bufferPropertyValue( clsDesc.getNativeSizeInfo(), data );
			SetPropertyValue evt( toStream( instance ), toStream( name ), data, toStream( incomingTypeName ), numItems );
			return boolToError( handlePvdEvent( evt ) );
		}

		//Else if the property is very large (contact reports) you can send it in chunks.
		virtual PvdError beginSetPropertyValue( const void* instance, String name, const NamespacedName& incomingTypeName )
		{
			PX_ASSERT( isInstanceValid( instance ) );
#ifdef PX_DEBUG
			PX_ASSERT( classExists( incomingTypeName ) );
			PX_ASSERT( checkPropertyType( instance, name, incomingTypeName ) );
#endif
			PX_ASSERT( mStreamState == DataStreamState::Open );
			mStreamState = DataStreamState::SetPropertyValue;
			{
				ScopedMetaData meta( mMetaDataProvider );
				mSPVClass = meta->findClass( incomingTypeName );
			}
			BeginSetPropertyValue evt( toStream( instance ), toStream( name ), toStream( incomingTypeName ) );
			return boolToError( handlePvdEvent( evt ) );
		}

		virtual PvdError appendPropertyValueData( DataRef<const PxU8> data )
		{
			PxU32 realSize = mSPVClass.getNativeSize();
			PxU32 numItems = data.size() / realSize;
			data = bufferPropertyValue( mSPVClass.getNativeSizeInfo(), data );
			PX_ASSERT( mStreamState == DataStreamState::SetPropertyValue );
			return boolToError( handlePvdEvent( AppendPropertyValueData( data, numItems ) ) );
		}
		virtual PvdError endSetPropertyValue()
		{
			PX_ASSERT( mStreamState == DataStreamState::SetPropertyValue );
			mStreamState = DataStreamState::Open;
			return boolToError( handlePvdEvent( EndSetPropertyValue() ) );
		}

#ifdef PX_DEBUG


		bool checkPropertyMessage( const void* instance, const NamespacedName& msgName )
		{
			NonNegativeInteger clsId = mMetaDataProvider.getInstanceClassType( instance );
			ScopedMetaData meta( mMetaDataProvider );
			PropertyMessageDescription desc(meta->findPropertyMessage( msgName ));
			bool retval = meta->isDerivedFrom( clsId, desc.mClassId );
			return retval;
		}

#endif

		DataRef<const PxU8> bufferPropertyMessage( const PropertyMessageDescription& desc, DataRef<const PxU8> data )
		{
			if ( desc.mStringOffsets.size() )
			{
				mSPVBuffer.clear();
				mSPVBuffer.write( data.begin(), data.size() );
				PVD_FOREACH( idx, desc.mStringOffsets.size() )
				{
					const char* strPtr;
					PxMemCopy( &strPtr, data.begin() + desc.mStringOffsets[idx], sizeof( char* ) );
					strPtr = nonNull( strPtr );
					PxU32 len = safeStrLen( strPtr ) + 1;
					mSPVBuffer.write( strPtr, len );
				}
				data = DataRef<const PxU8>( mSPVBuffer.begin(), mSPVBuffer.end() );
			}
			return data;
		}

		virtual PvdError setPropertyMessage( const void* instance, const NamespacedName& msgName, DataRef<const PxU8> data )
		{
			ScopedMetaData meta( mMetaDataProvider );
			PX_ASSERT( isInstanceValid( instance ) );
#ifdef PX_DEBUG
			PX_ASSERT( messageExists( msgName ) );
			PX_ASSERT( checkPropertyMessage( instance, msgName ) );
#endif
			PropertyMessageDescription desc(meta->findPropertyMessage( msgName ));
			if ( data.size() < desc.mMessageByteSize ) { PX_ASSERT( false ); return PvdErrorType::ArgumentError; }
			data = bufferPropertyMessage( desc, data );
			PX_ASSERT( mStreamState == DataStreamState::Open );
			return boolToError( handlePvdEvent( SetPropertyMessage( toStream( instance ), toStream( msgName ), data ) ) );
		}

#ifdef PX_DEBUG


		bool checkBeginPropertyMessageGroup( const NamespacedName& msgName )
		{
			ScopedMetaData meta( mMetaDataProvider );
			PropertyMessageDescription desc( meta->findPropertyMessage( msgName ));
			return desc.mStringOffsets.size() == 0;
		}

#endif
		//If you need to send of lot of identical messages, this avoids a hashtable lookup per message.
		virtual PvdError beginPropertyMessageGroup( const NamespacedName& msgName )
		{
#ifdef PX_DEBUG
			PX_ASSERT( messageExists( msgName ) );
			PX_ASSERT( checkBeginPropertyMessageGroup( msgName ) );
#endif
			PX_ASSERT( mStreamState == DataStreamState::Open );
			mStreamState = DataStreamState::PropertyMessageGroup;
			ScopedMetaData meta( mMetaDataProvider );
			mMessageDesc = meta->findPropertyMessage( msgName );
			return boolToError( handlePvdEvent( BeginPropertyMessageGroup( toStream( msgName ) ) ) );
		}

		virtual PvdError sendPropertyMessageFromGroup( const void* instance, DataRef<const PxU8> data )
		{
			PX_ASSERT( mStreamState == DataStreamState::PropertyMessageGroup );
			PX_ASSERT( isInstanceValid( instance ) );
#ifdef PX_DEBUG
			PX_ASSERT( checkPropertyMessage( instance, mMessageDesc.mMessageName ) );
#endif
			if ( mMessageDesc.mMessageByteSize != data.size() ) { PX_ASSERT( false ); return PvdErrorType::ArgumentError; }
			if ( data.size() < mMessageDesc.mMessageByteSize ) return PvdErrorType::ArgumentError;
			data = bufferPropertyMessage( mMessageDesc, data );
			return boolToError( handlePvdEvent( SendPropertyMessageFromGroup( toStream( instance ), data ) ) );
		}
		virtual PvdError endPropertyMessageGroup()
		{
			PX_ASSERT( mStreamState == DataStreamState::PropertyMessageGroup );
			mStreamState = DataStreamState::Open;
			return boolToError( handlePvdEvent( EndPropertyMessageGroup() ) );
		}
		virtual PvdError pushBackObjectRef( const void* instance, String propName, const void* data )
		{
			PX_ASSERT( isInstanceValid( instance ) );
			PX_ASSERT( isInstanceValid( data ) );
			PX_ASSERT( mStreamState == DataStreamState::Open );
			return boolToError( handlePvdEvent( PushBackObjectRef( toStream( instance ), toStream( propName ), toStream( data ) ) ) );
		}
		virtual PvdError removeObjectRef( const void* instance, String propName, const void* data )
		{
			PX_ASSERT( isInstanceValid( instance ) );
			PX_ASSERT( isInstanceValid( data ) );
			PX_ASSERT( mStreamState == DataStreamState::Open );
			return boolToError( handlePvdEvent( RemoveObjectRef( toStream( instance ), toStream( propName ), toStream( data ) ) ) );
		}
		//Instance elimination.
		virtual PvdError destroyInstance( const void* instance )
		{
			PX_ASSERT( isInstanceValid( instance ) );
			PX_ASSERT( mStreamState == DataStreamState::Open );
			mMetaDataProvider.destroyInstance( instance );
			return boolToError( handlePvdEvent( DestroyInstance( toStream( instance ) ) ) );
		}
		
		//Profiling hooks
		virtual PvdError beginSection( const void* instance, String name )
		{
			PX_ASSERT( mStreamState == DataStreamState::Open );
			return boolToError( handlePvdEvent( BeginSection( toStream( instance ), toStream( name ), physx::shdfnd::Time::getCurrentCounterValue() ) ) );
		}

		virtual PvdError endSection( const void* instance, String name )
		{
			PX_ASSERT( mStreamState == DataStreamState::Open );
			return boolToError( handlePvdEvent( EndSection( toStream( instance ), toStream( name ), physx::shdfnd::Time::getCurrentCounterValue() ) ) );
		}

		virtual PvdError originShift( const void* scene, PxVec3 shift )
		{
			PX_ASSERT( mStreamState == DataStreamState::Open );
			return boolToError( handlePvdEvent( OriginShift( toStream( scene ), shift ) ) );
		}

		virtual void addProfileZone( void* zone, const char* name )
		{
			handlePvdEvent( AddProfileZone( toStream( zone ), name ) );
		}
		virtual void addProfileZoneEvent( void* zone, const char* name, PxU16 eventId, bool compileTimeEnabled )
		{
			handlePvdEvent( AddProfileZoneEvent( toStream( zone ), name, eventId, compileTimeEnabled ) );
		}
		
		//add a variable sized event
		void addEvent( const EventSerializeable& evt, PvdCommStreamEventTypes::Enum evtType )
		{
			if ( mConnected )
			{
				PxU32 available = mBufferCutoff - mBuffer.size();
				MeasureStream measure;
				PvdCommStreamEventSink::writeEvent( evt, evtType, measure );
				if ( measure.mSize > available )
					flush();
				//Events may be huge.  So sometimes we have to direct write
				//the event out.
				if ( measure.mSize > mBufferCutoff || mCacheEnabled == false )
				{
					PvdNetworkOutStream& stream( mClient.lock( measure.mSize, 1, mStreamId ) );
					PvdCommStreamEventSink::writeEvent( evt, evtType, stream );
					mClient.unlock();
					mConnected = mClient.isConnected();
				}
				else
				{
					PvdCommStreamEventSink::writeEvent( evt, evtType, mBuffer );
					++mEventCount;
				}
			}
		}

		template<typename TEventType>
		bool handlePvdEvent( const TEventType& evt )
		{
			addEvent( evt, getCommStreamEventType<TEventType>() );
			return mConnected;
		}

		virtual PvdError flush() 
		{
			if ( mBuffer.size() && mConnected )
				mConnected = mClient.sendEventBuffer( DataRef<const PxU8>( mBuffer.begin(), mBuffer.size() ), mEventCount, mStreamId );
			mBuffer.clear();
			mEventCount = 0;
			return boolToError( mConnected );
		}

		virtual PvdPropertyDefinitionHelper& getPropertyDefinitionHelper() 
		{
			mPropertyDefinitionHelper.clearBufferedData();
			return mPropertyDefinitionHelper; 
		}

		virtual bool isConnected()
		{
			if ( mConnected )
				mConnected = mClient.isConnected();
			return mConnected;
		}

		virtual void enableCache() { mCacheEnabled = true; }
		virtual void disableCache() { mCacheEnabled = false; flush(); }

		virtual void* allocateMemForCmd( PxU32 length )
		{
			return mPvdCommandPool.allocate(length);
		}

		virtual void pushPvdCommand(PvdCommand& cmd)
		{
			mPvdCommandArray.pushBack(&cmd);
		}

		virtual void flushPvdCommand()
		{
			PxU32 cmdQueueSize = mPvdCommandArray.size();
			for(PxU32 i= 0; i < cmdQueueSize; i++)
			{
				if(mPvdCommandArray[i])
				{
					//if(mPvdCommandArray[i]->canRun(*this))
						mPvdCommandArray[i]->run(*this);
					mPvdCommandArray[i]->~PvdCommand();
				}
			}
			mPvdCommandArray.clear();
			mPvdCommandPool.clear();
		}

		PX_NOCOPY(DataStream)
	};
}

PvdCommStreamInternalDataStream& PvdCommStreamInternalDataStream::create( PxAllocatorCallback& alloc, PvdOMMetaDataProvider& metaDataProvider, PxU32 bufferSize, PvdCommStreamEventBufferClient& client, PxU64 streamId )
{
	return *PVD_NEW( alloc, DataStream )( alloc, metaDataProvider, bufferSize, client, streamId );
}
