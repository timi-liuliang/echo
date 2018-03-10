/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */



#include "PvdObjectModel.h"
#include "PvdObjectModelBaseTypes.h"
#include "PvdObjectModelInternalTypes.h"
#include "PvdObjectModelMetaData.h"
#include "PvdFoundation.h"
#include "PsMutex.h"
#include "PvdByteStreams.h"
#include "PvdInternalByteStreams.h"
#include "PvdMarshalling.h"

using namespace physx::debugger;

namespace {

	struct PropDescImpl : public PropertyDescription
	{	
		ForwardingArray<NamedValue> mValueNames;
		PropDescImpl( const PropertyDescription& inBase, PxAllocatorCallback& alloc, StringTable& table ) 
			: PropertyDescription( inBase ) 
			, mValueNames( alloc, "NamedValue" )
		{
			mName = table.registerStr( mName );
		}
		PropDescImpl( PxAllocatorCallback& alloc )
			: mValueNames( alloc, "NamedValue" )
		{
		}

		template<typename TSerializer>
		void serialize( TSerializer& serializer )
		{
			serializer.streamify( mOwnerClassName );
			serializer.streamify( mOwnerClassId );
			serializer.streamify( mSemantic );
			serializer.streamify( mDatatype );
			serializer.streamify( mDatatypeName );
			serializer.streamify( mPropertyType );
			serializer.streamify( mPropertyId );
			serializer.streamify( m32BitOffset );
			serializer.streamify( m64BitOffset );
			serializer.streamify( mValueNames );
			serializer.streamify( mName );
		}
	};

	struct ClassDescImpl : public ClassDescription
	{
		ForwardingArray<PropDescImpl*>	mPropImps;
		ForwardingArray<PtrOffset>		m32OffsetArray;
		ForwardingArray<PtrOffset>		m64OffsetArray;
		ClassDescImpl(const ClassDescription& inBase, PxAllocatorCallback& alloc) 
			: ClassDescription( inBase )
			, mPropImps( alloc, "PropDescImpl*" ) 
			, m32OffsetArray( alloc, "ClassDescImpl::m32OffsetArray" )
			, m64OffsetArray( alloc, "ClassDescImpl::m64OffsetArray" )
		{
			PVD_FOREACH( idx, m32BitSizeInfo.mPtrOffsets.size() )
				m32OffsetArray.pushBack( m32BitSizeInfo.mPtrOffsets[idx] );
			PVD_FOREACH( idx, m64BitSizeInfo.mPtrOffsets.size() )
				m64OffsetArray.pushBack( m64BitSizeInfo.mPtrOffsets[idx] );
		}
		ClassDescImpl( PxAllocatorCallback& alloc ) 
			: mPropImps( alloc, "PropDescImpl*" )
			, m32OffsetArray( alloc, "ClassDescImpl::m32OffsetArray" )
			, m64OffsetArray( alloc, "ClassDescImpl::m64OffsetArray" )
		{}
		PropDescImpl* findProperty( String name )
		{
			PVD_FOREACH( idx, mPropImps.size() )
			{
				if ( safeStrEq( mPropImps[idx]->mName, name ) )
					return mPropImps[idx];
			}
			return NULL;
		}
		void addProperty( PropDescImpl* prop )
		{
			mPropImps.pushBack( prop );
		}

		void addPtrOffset( PtrOffsetType::Enum type, PxU32 offset32, PxU32 offset64 )
		{
			m32OffsetArray.pushBack( PtrOffset( type, offset32 ) );
			m64OffsetArray.pushBack( PtrOffset( type, offset64 ) );
			m32BitSizeInfo.mPtrOffsets = DataRef<PtrOffset>( m32OffsetArray.begin(), m32OffsetArray.end() );
			m64BitSizeInfo.mPtrOffsets = DataRef<PtrOffset>( m64OffsetArray.begin(), m64OffsetArray.end() );
		}

		template<typename TSerializer>
		void serialize( TSerializer& serializer )
		{
			serializer.streamify( mName );
			serializer.streamify( mClassId );
			serializer.streamify( mBaseClass );
			serializer.streamify( mPackedUniformWidth );
			serializer.streamify( mPackedClassType );
			serializer.streamify( mLocked );
			serializer.streamify( mRequiresDestruction );
			serializer.streamify( m32BitSizeInfo.mByteSize );
			serializer.streamify( m32BitSizeInfo.mDataByteSize );
			serializer.streamify( m32BitSizeInfo.mAlignment );
			serializer.streamify( m64BitSizeInfo.mByteSize );
			serializer.streamify( m64BitSizeInfo.mDataByteSize );
			serializer.streamify( m64BitSizeInfo.mAlignment );
			serializer.streamifyLinks( mPropImps );
			serializer.streamify( m32OffsetArray );
			serializer.streamify( m64OffsetArray );
			m32BitSizeInfo.mPtrOffsets = DataRef<PtrOffset>( m32OffsetArray.begin(), m32OffsetArray.end() );
			m64BitSizeInfo.mPtrOffsets = DataRef<PtrOffset>( m64OffsetArray.begin(), m64OffsetArray.end() );
		}
	};

	class StringTableImpl : public StringTable
	{
		PxAllocatorCallback& mAlloc;
		ForwardingHashMap<const char*, char*>	mStrings;
		PxU32 mNextStrHandle;
		ForwardingHashMap<PxU32, char*>			mHandleToStr;
		ForwardingHashMap<const char*, PxU32>	mStrToHandle;
	public:
		StringTableImpl( PxAllocatorCallback& callback )
			: mAlloc( callback )
			, mStrings( callback, "StringTableImpl::mStrings" )
			, mNextStrHandle( 1 )
			, mHandleToStr( callback, "StringTableImpl::mHandleToStr" )
			, mStrToHandle( callback, "StringTableImpl::mStrToHandle" )
		{
		}
		PxU32 nextHandleValue() { return mNextStrHandle++; }
		virtual ~StringTableImpl()
		{
			for ( ForwardingHashMap<const char*,char*>::Iterator iter = mStrings.getIterator(); !iter.done(); ++iter )
				mAlloc.deallocate( iter->second );
			mStrings.clear();
		}
		virtual PxU32 getNbStrs() { return (PxU32)mStrings.size(); }
		virtual PxU32 getStrs( const char** outStrs, PxU32 bufLen, PxU32 startIdx = 0 )
		{
			startIdx = PxMin( getNbStrs(), startIdx );
			PxU32 numStrs( PxMin( getNbStrs() - startIdx, bufLen ) );
			ForwardingHashMap<const char*,char*>::Iterator iter( mStrings.getIterator() );
			for ( PxU32 idx = 0; idx < startIdx; ++idx, ++iter );
			for ( PxU32 idx = 0; idx < numStrs && !iter.done(); ++idx, ++iter )
				outStrs[idx] = iter->second;
			return numStrs;
		}
		void addStringHandle( char* str, PxU32 hdl )
		{
			mHandleToStr.insert( hdl, str );
			mStrToHandle.insert( str, hdl );
		}
		
		PxU32 addStringHandle( char* str )
		{
			PxU32 newHandle = nextHandleValue();
			addStringHandle( str, newHandle );
			return newHandle;
		}
		const char* doRegisterStr( const char* str, bool& outAdded )
		{
			PX_ASSERT( isMeaningful( str ) );
			const ForwardingHashMap<const char*,char*>::Entry* entry( mStrings.find( str ) );
			if ( entry == NULL )
			{
				outAdded = true;
				char* retval( copyStr( mAlloc, str, __FILE__, __LINE__ ) );
				mStrings.insert( retval, retval );
				return retval;
			}
			return entry->second;

		}
		virtual const char* registerStr( const char* str, bool& outAdded )
		{
			outAdded = false;
			if ( isMeaningful( str ) == false ) return "";
			const char* retval = doRegisterStr( str, outAdded );
			if ( outAdded )
				addStringHandle( const_cast<char*>( retval ) );
			return retval;
		}

		NamespacedName registerName( const NamespacedName& nm )
		{
			return NamespacedName( registerStr( nm.mNamespace ), registerStr( nm.mName ) );
		}
		const char* registerStr( const char* str ) {bool ignored; return registerStr( str, ignored );}

		
		virtual StringHandle strToHandle( const char* str )
		{
			if ( isMeaningful( str ) == false ) return 0;
			const ForwardingHashMap<const char*, PxU32>::Entry* entry( mStrToHandle.find( str ) );
			if ( entry ) return entry->second;
			bool added = false;
			const char* registeredStr = doRegisterStr( str, added );
			PxU32 newHandle = addStringHandle( const_cast<char*>( registeredStr ) );
			PX_ASSERT( mStrToHandle.find( str ) );
			PX_ASSERT( added );
			return newHandle;
		}

		virtual const char* handleToStr( PxU32 hdl )
		{
			if ( hdl == 0 ) return "";
			const ForwardingHashMap<PxU32, char*>::Entry* entry( mHandleToStr.find( hdl ) );
			if ( entry ) return entry->second;
			//unregistered handle...
			return "";
		}

		void write( PvdOutputStream& stream )
		{
			PxU32 numStrs = static_cast<PxU32>( mHandleToStr.size() );
			stream << numStrs;
			stream << mNextStrHandle;
			for ( ForwardingHashMap<PxU32, char*>::Iterator iter = mHandleToStr.getIterator(); !iter.done(); ++iter )
			{
				stream << iter->first;
				PxU32 len = static_cast<PxU32>( strlen( iter->second ) + 1 );
				stream << len;
				stream.write( reinterpret_cast<PxU8*>( iter->second ), len );
			}
		}

		template<typename TReader>
		void read( TReader& stream )
		{
			mHandleToStr.clear();
			mStrToHandle.clear();
			PxU32 numStrs;
			stream >> numStrs;
			stream >> mNextStrHandle;
			ForwardingArray<PxU8> readBuffer( mAlloc, "StringTable::read::readBuffer" );
			PxU32 bufSize = 0;
			for( PxU32 idx = 0; idx < numStrs; ++idx )
			{
				PxU32 handleValue;
				PxU32 bufLen;
				stream >> handleValue;
				stream >> bufLen;
				if ( bufSize < bufLen )
					readBuffer.resize( bufLen );
				bufSize = PxMax( bufSize, bufLen );
				stream.read( readBuffer.begin(), bufLen );
				bool ignored;
				const char* newStr = doRegisterStr( (const char*)readBuffer.begin(), ignored );
				addStringHandle( const_cast<char*>( newStr ), handleValue );
			}
		}

		virtual void release() 
		{
			PVD_DELETE( mAlloc, this );
		}
	private:
		StringTableImpl& operator=(const StringTableImpl&);
	};

	struct NamespacedNameHasher
	{
		PxU32 operator()( const NamespacedName& nm )
		{
			return Hash<const char*>()( nm.mNamespace ) ^ Hash<const char*>()( nm.mName );
		}
		bool operator()( const NamespacedName& lhs, const NamespacedName& rhs )
		{
			return safeStrEq( lhs.mNamespace, rhs.mNamespace ) && safeStrEq( lhs.mName, rhs.mName );
		}
	};

	struct ClassPropertyName
	{
		NamespacedName mName;
		String mPropName;
		ClassPropertyName( const NamespacedName& name = NamespacedName(), String propName = "")
			: mName( name )
			, mPropName( propName )
		{
		}
	};

	struct ClassPropertyNameHasher
	{
		PxU32 operator()( const ClassPropertyName& nm )
		{
			return NamespacedNameHasher()(nm.mName ) ^ Hash<const char*>()( nm.mPropName );
		}
		bool operator()( const ClassPropertyName& lhs, const ClassPropertyName& rhs )
		{
			return NamespacedNameHasher()( lhs.mName, rhs.mName ) && safeStrEq( lhs.mPropName, rhs.mPropName );
		}
	};

	struct PropertyMessageEntryImpl : public PropertyMessageEntry
	{
		PropertyMessageEntryImpl( const PropertyMessageEntry& data )
			: PropertyMessageEntry( data )
		{
		}
		PropertyMessageEntryImpl()
		{
		}
		template<typename TSerializerType>
		void serialize( TSerializerType& serializer )
		{
			serializer.streamify( mDatatypeName );
			serializer.streamify( mDatatypeId );
			serializer.streamify( mMessageOffset );
			serializer.streamify( mByteSize );
			serializer.streamify( mDestByteSize );
			serializer.streamify( mProperty );
		}
	};

	struct PropertyMessageDescriptionImpl : public PropertyMessageDescription
	{
		PxAllocatorCallback&						mAlloc;
		ForwardingArray<PropertyMessageEntryImpl>	mEntryImpls;
		ForwardingArray<PropertyMessageEntry>		mEntries;
		ForwardingArray<PxU32>						mStringOffsetArray;
		PropertyMessageDescriptionImpl( const PropertyMessageDescription& data, PxAllocatorCallback& alloc )
			: PropertyMessageDescription( data )
			, mAlloc( alloc )
			, mEntryImpls( alloc, "PropertyMessageDescriptionImpl::mEntryImpls" )
			, mEntries( alloc, "PropertyMessageDescriptionImpl::mEntries" )
			, mStringOffsetArray( alloc, "PropertyMessageDescriptionImpl::mStringOffsets" )
		{
		}
		PropertyMessageDescriptionImpl( PxAllocatorCallback& alloc ) 
			: mAlloc( alloc ) 
			, mEntryImpls( alloc, "PropertyMessageDescriptionImpl::mEntryImpls" )
			, mEntries( alloc, "PropertyMessageDescriptionImpl::mEntries" )
			, mStringOffsetArray( alloc, "PropertyMessageDescriptionImpl::mStringOffsets" )
		{}

		~PropertyMessageDescriptionImpl()
		{
		}

		void addEntry( const PropertyMessageEntryImpl& entry )
		{
			mEntryImpls.pushBack( entry );
			mEntries.pushBack( entry );
			mProperties = DataRef<PropertyMessageEntry>( mEntries.begin(), mEntries.end() );
		}

		template<typename TSerializerType>
		void serialize( TSerializerType& serializer )
		{
			serializer.streamify( mClassName );
			serializer.streamify( mClassId );		//No other class has this id, it is DB-unique
			serializer.streamify( mMessageName );
			serializer.streamify( mMessageId );
			serializer.streamify( mMessageByteSize );
			serializer.streamify( mEntryImpls );
			serializer.streamify( mStringOffsetArray );
			if ( mEntries.size() != mEntryImpls.size() )
			{
				mEntries.clear();
				PxU32 numEntries = static_cast<PxU32>( mEntryImpls.size() );
				for ( PxU32 idx = 0; idx < numEntries; ++idx )
					mEntries.pushBack( mEntryImpls[idx] );
			}
			mProperties = DataRef<PropertyMessageEntry>( mEntries.begin(), mEntries.end() );
			mStringOffsets = DataRef<PxU32>( mStringOffsetArray.begin(), mStringOffsetArray.end() );
		}

	private:
		PropertyMessageDescriptionImpl& operator=(const PropertyMessageDescriptionImpl&);
	};

	struct PvdObjectModelMetaDataImpl : public PvdObjectModelMetaData
	{
		typedef ForwardingHashMap<NamespacedName, ClassDescImpl*, NamespacedNameHasher>						TNameToClassMap;
		typedef ForwardingHashMap<ClassPropertyName,	PropDescImpl*, ClassPropertyNameHasher>				TNameToPropMap;
		typedef ForwardingHashMap<NamespacedName, PropertyMessageDescriptionImpl*, NamespacedNameHasher>	TNameToPropertyMessageMap;

		PxAllocatorCallback&								mAlloc;
		TNameToClassMap										mNameToClasses;
		TNameToPropMap										mNameToProperties;
		ForwardingArray<ClassDescImpl*>						mClasses;
		ForwardingArray<PropDescImpl*>						mProperties;
		StringTableImpl&									mStringTable; 
		TNameToPropertyMessageMap							mPropertyMessageMap;
		ForwardingArray<PropertyMessageDescriptionImpl*>	mPropertyMessages;
		PxI32												mNextClassId;
		PxU32												mRefCount;



		PvdObjectModelMetaDataImpl( PxAllocatorCallback& alloc )
			: mAlloc( alloc )
			, mNameToClasses( alloc, "NamespacedName->ClassDescImpl*" )
			, mNameToProperties( alloc, "ClassPropertyName->PropDescImpl*" )
			, mClasses( alloc,  "ClassDescImpl*" )
			, mProperties( alloc, "PropDescImpl*" )
			, mStringTable( *(PVD_NEW(alloc, StringTableImpl)(alloc)) )
			, mPropertyMessageMap( alloc, "PropertyMessageMap" )
			, mPropertyMessages( alloc, "PvdObjectModelMetaDataImpl::mPropertyMessages" )
			, mNextClassId( 1 )
			, mRefCount( 0 )
		{
		}

	private:
		PvdObjectModelMetaDataImpl& operator=(const PvdObjectModelMetaDataImpl&);

	public:

		PxI32 nextClassId() { return mNextClassId++; }
		void initialize()
		{
			//Create the default classes.
			{
				ClassDescImpl& aryData = getOrCreateClassImpl( getPvdNamespacedNameForType<ArrayData>(), DataTypeToPvdTypeMap<ArrayData>::BaseTypeEnum );
				aryData.m32BitSizeInfo.mByteSize = sizeof( ArrayData );
				aryData.m32BitSizeInfo.mAlignment = sizeof( void* );
				aryData.m64BitSizeInfo.mByteSize = sizeof( ArrayData );
				aryData.m64BitSizeInfo.mAlignment = sizeof( void* );
				aryData.mLocked = true;
			}
#define CREATE_BASIC_PVD_CLASS( type )	\
			{							\
				ClassDescImpl& cls = getOrCreateClassImpl( getPvdNamespacedNameForType<type>(), getPvdTypeForType<type>() );	\
				cls.m32BitSizeInfo.mByteSize = sizeof( type );			\
				cls.m32BitSizeInfo.mAlignment = sizeof( type );			\
				cls.m64BitSizeInfo.mByteSize = sizeof( type );			\
				cls.m64BitSizeInfo.mAlignment = sizeof( type );			\
				cls.mLocked = true;										\
				cls.mPackedUniformWidth = sizeof( type );				\
				cls.mPackedClassType = getPvdTypeForType<type>();	\
			}
			CREATE_BASIC_PVD_CLASS( PxI8 )
			CREATE_BASIC_PVD_CLASS( PxU8 )
			CREATE_BASIC_PVD_CLASS( PvdBool )
			CREATE_BASIC_PVD_CLASS( PxI16 )
			CREATE_BASIC_PVD_CLASS( PxU16 )
			CREATE_BASIC_PVD_CLASS( PxI32 )
			CREATE_BASIC_PVD_CLASS( PxU32 )
			CREATE_BASIC_PVD_CLASS( PxU32 )
			CREATE_BASIC_PVD_CLASS( PxI64 )
			CREATE_BASIC_PVD_CLASS( PxU64 )
			CREATE_BASIC_PVD_CLASS( PxF32 )
			CREATE_BASIC_PVD_CLASS( PxF64 )
#undef CREATE_BASIC_PVD_CLASS
			
#define CREATE_PTR_TYPE_PVD_CLASS( type, ptrType )	\
			{										\
				ClassDescImpl& cls = getOrCreateClassImpl( getPvdNamespacedNameForType<type>(), getPvdTypeForType<type>() );	\
				cls.m32BitSizeInfo.mByteSize = 4;			     \
				cls.m32BitSizeInfo.mAlignment = 4;			     \
				cls.m64BitSizeInfo.mByteSize = 8;			     \
				cls.m64BitSizeInfo.mAlignment = 8;				 \
				cls.mLocked = true;								 \
				cls.addPtrOffset( PtrOffsetType::ptrType, 0, 0 );\
			}
			
			CREATE_PTR_TYPE_PVD_CLASS( String, StringOffset )
			CREATE_PTR_TYPE_PVD_CLASS( VoidPtr, VoidPtrOffset )
			CREATE_PTR_TYPE_PVD_CLASS( StringHandle, StringOffset )
			CREATE_PTR_TYPE_PVD_CLASS( ObjectRef, VoidPtrOffset )

#undef CREATE_64BIT_ADJUST_PVD_CLASS

			PxI32 fltClassType = getPvdTypeForType<PxF32>();
			PxI32 u32ClassType = getPvdTypeForType<PxU32>();
			PxI32 v3ClassType = getPvdTypeForType<PxVec3>();
			PxI32 v4ClassType = getPvdTypeForType<PxVec4>();
			PxI32 qtClassType = getPvdTypeForType<PxQuat>();
			{
				ClassDescImpl& cls = getOrCreateClassImpl( getPvdNamespacedNameForType<PvdColor>(), getPvdTypeForType<PvdColor>() );
				createProperty( cls.mClassId, "r", "", getPvdTypeForType<PxU8>(), PropertyType::Scalar );
				createProperty( cls.mClassId, "g", "", getPvdTypeForType<PxU8>(), PropertyType::Scalar );
				createProperty( cls.mClassId, "b", "", getPvdTypeForType<PxU8>(), PropertyType::Scalar );
				createProperty( cls.mClassId, "a", "", getPvdTypeForType<PxU8>(), PropertyType::Scalar );
				PX_ASSERT( cls.m32BitSizeInfo.mAlignment == 1 );
				PX_ASSERT( cls.m32BitSizeInfo.mByteSize == 4 );
				PX_ASSERT( cls.m64BitSizeInfo.mAlignment == 1 );
				PX_ASSERT( cls.m64BitSizeInfo.mByteSize == 4 );
				PX_ASSERT( cls.mPackedUniformWidth == NonNegativeInteger( 1 ) );
				PX_ASSERT( cls.mPackedClassType == getPvdTypeForType<PxU8>() );
				cls.mLocked = true;
			}

			{
				ClassDescImpl& cls = getOrCreateClassImpl( getPvdNamespacedNameForType<PxVec2>(), getPvdTypeForType<PxVec2>() );
				createProperty( cls.mClassId, "x", "", fltClassType, PropertyType::Scalar );
				createProperty( cls.mClassId, "y", "", fltClassType, PropertyType::Scalar );
				PX_ASSERT( cls.m32BitSizeInfo.mAlignment == 4 );
				PX_ASSERT( cls.m32BitSizeInfo.mByteSize == 8 );
				PX_ASSERT( cls.m64BitSizeInfo.mAlignment == 4 );
				PX_ASSERT( cls.m64BitSizeInfo.mByteSize == 8 );
				PX_ASSERT( cls.mPackedUniformWidth == NonNegativeInteger( 4 ) );
				PX_ASSERT( cls.mPackedClassType == NonNegativeInteger( fltClassType ) );
				cls.mLocked = true;
			}
			{
				ClassDescImpl& cls = getOrCreateClassImpl( getPvdNamespacedNameForType<PxVec3>(), getPvdTypeForType<PxVec3>() );
				createProperty( cls.mClassId, "x", "", fltClassType, PropertyType::Scalar );
				createProperty( cls.mClassId, "y", "", fltClassType, PropertyType::Scalar );
				createProperty( cls.mClassId, "z", "", fltClassType, PropertyType::Scalar );
				PX_ASSERT( cls.m32BitSizeInfo.mAlignment == 4 );
				PX_ASSERT( cls.m32BitSizeInfo.mByteSize == 12 );
				PX_ASSERT( cls.m64BitSizeInfo.mAlignment == 4 );
				PX_ASSERT( cls.m64BitSizeInfo.mByteSize == 12 );
				PX_ASSERT( cls.mPackedUniformWidth == NonNegativeInteger( 4 ) );
				PX_ASSERT( cls.mPackedClassType == NonNegativeInteger( fltClassType ) );
				cls.mLocked = true;
			}
			{
				ClassDescImpl& cls = getOrCreateClassImpl( getPvdNamespacedNameForType<PxVec4>(), getPvdTypeForType<PxVec4>() );
				createProperty( cls.mClassId, "x", "", fltClassType, PropertyType::Scalar );
				createProperty( cls.mClassId, "y", "", fltClassType, PropertyType::Scalar );
				createProperty( cls.mClassId, "z", "", fltClassType, PropertyType::Scalar );
				createProperty( cls.mClassId, "w", "", fltClassType, PropertyType::Scalar );
				PX_ASSERT( cls.m32BitSizeInfo.mAlignment == 4 );
				PX_ASSERT( cls.m32BitSizeInfo.mByteSize == 16 );
				PX_ASSERT( cls.m64BitSizeInfo.mAlignment == 4 );
				PX_ASSERT( cls.m64BitSizeInfo.mByteSize == 16 );
				PX_ASSERT( cls.mPackedUniformWidth == NonNegativeInteger( 4 ) );
				PX_ASSERT( cls.mPackedClassType == NonNegativeInteger( fltClassType ) );
				cls.mLocked = true;
			}
			
			{
				ClassDescImpl& cls = getOrCreateClassImpl( getPvdNamespacedNameForType<PxQuat>(), getPvdTypeForType<PxQuat>() );
				createProperty( cls.mClassId, "x", "", fltClassType, PropertyType::Scalar );
				createProperty( cls.mClassId, "y", "", fltClassType, PropertyType::Scalar );
				createProperty( cls.mClassId, "z", "", fltClassType, PropertyType::Scalar );
				createProperty( cls.mClassId, "w", "", fltClassType, PropertyType::Scalar );
				PX_ASSERT( cls.m32BitSizeInfo.mAlignment == 4 );
				PX_ASSERT( cls.m32BitSizeInfo.mByteSize == 16 );
				PX_ASSERT( cls.m64BitSizeInfo.mAlignment == 4 );
				PX_ASSERT( cls.m64BitSizeInfo.mByteSize == 16 );
				PX_ASSERT( cls.mPackedUniformWidth == NonNegativeInteger( 4 ) );
				PX_ASSERT( cls.mPackedClassType == NonNegativeInteger( fltClassType ) );
				cls.mLocked = true;
			}
			
			{
				ClassDescImpl& cls = getOrCreateClassImpl( getPvdNamespacedNameForType<PxBounds3>(), getPvdTypeForType<PxBounds3>() );
				createProperty( cls.mClassId, "minimum", "", v3ClassType, PropertyType::Scalar );
				createProperty( cls.mClassId, "maximum", "", v3ClassType, PropertyType::Scalar );
				PX_ASSERT( cls.m32BitSizeInfo.mAlignment == 4 );
				PX_ASSERT( cls.m32BitSizeInfo.mByteSize == 24 );
				PX_ASSERT( cls.mPackedUniformWidth == NonNegativeInteger( 4 ) );
				PX_ASSERT( cls.mPackedClassType == NonNegativeInteger( fltClassType ) );
				cls.mLocked = true;
			}
			
			{
				ClassDescImpl& cls = getOrCreateClassImpl( getPvdNamespacedNameForType<PxTransform>(), getPvdTypeForType<PxTransform>() );
				createProperty( cls.mClassId, "q", "", qtClassType, PropertyType::Scalar );
				createProperty( cls.mClassId, "p", "", v3ClassType, PropertyType::Scalar );
				PX_ASSERT( cls.m32BitSizeInfo.mAlignment == 4 );
				PX_ASSERT( cls.m32BitSizeInfo.mByteSize == 28 );
				PX_ASSERT( cls.mPackedUniformWidth == NonNegativeInteger( 4 ) );
				PX_ASSERT( cls.mPackedClassType == NonNegativeInteger( fltClassType ) );
				cls.mLocked = true;
			}

			{
				ClassDescImpl& cls = getOrCreateClassImpl( getPvdNamespacedNameForType<PxMat33>(), getPvdTypeForType<PxMat33>() );
				createProperty( cls.mClassId, "column0", "", v3ClassType, PropertyType::Scalar );
				createProperty( cls.mClassId, "column1", "", v3ClassType, PropertyType::Scalar );
				createProperty( cls.mClassId, "column2", "", v3ClassType, PropertyType::Scalar );
				PX_ASSERT( cls.m32BitSizeInfo.mAlignment == 4 );
				PX_ASSERT( cls.m32BitSizeInfo.mByteSize == 36 );
				PX_ASSERT( cls.mPackedUniformWidth == NonNegativeInteger( 4 ) );
				PX_ASSERT( cls.mPackedClassType == NonNegativeInteger( fltClassType ) );
				cls.mLocked = true;
			}
			

			{
				ClassDescImpl& cls = getOrCreateClassImpl( getPvdNamespacedNameForType<PxMat44>(), getPvdTypeForType<PxMat44>() );
				createProperty( cls.mClassId, "column0", "", v4ClassType, PropertyType::Scalar );
				createProperty( cls.mClassId, "column1", "", v4ClassType, PropertyType::Scalar );
				createProperty( cls.mClassId, "column2", "", v4ClassType, PropertyType::Scalar );
				createProperty( cls.mClassId, "column3", "", v4ClassType, PropertyType::Scalar );
				PX_ASSERT( cls.m32BitSizeInfo.mAlignment == 4 );
				PX_ASSERT( cls.m32BitSizeInfo.mByteSize == 64 );
				PX_ASSERT( cls.mPackedUniformWidth == NonNegativeInteger( 4 ) );
				PX_ASSERT( cls.mPackedClassType == NonNegativeInteger( fltClassType ) );
				cls.mLocked = true;
			}
			
			{
				ClassDescImpl& cls = getOrCreateClassImpl( getPvdNamespacedNameForType<PxMat34Legacy>(), getPvdTypeForType<PxMat34Legacy>() );
				createProperty( cls.mClassId, "column0", "", v3ClassType, PropertyType::Scalar );
				createProperty( cls.mClassId, "column1", "", v3ClassType, PropertyType::Scalar );
				createProperty( cls.mClassId, "column2", "", v3ClassType, PropertyType::Scalar );
				createProperty( cls.mClassId, "column3", "", v3ClassType, PropertyType::Scalar );
				PX_ASSERT( cls.m32BitSizeInfo.mAlignment == 4 );
				PX_ASSERT( cls.m32BitSizeInfo.mByteSize == 48 );
				PX_ASSERT( cls.mPackedUniformWidth == NonNegativeInteger( 4 ) );
				PX_ASSERT( cls.mPackedClassType == NonNegativeInteger( fltClassType ) );
				cls.mLocked = true;
			}
			{
				ClassDescImpl& cls = getOrCreateClassImpl( getPvdNamespacedNameForType<U32Array4>(), getPvdTypeForType<U32Array4>() );
				createProperty( cls.mClassId, "d0", "", u32ClassType, PropertyType::Scalar );
				createProperty( cls.mClassId, "d1", "", u32ClassType, PropertyType::Scalar );
				createProperty( cls.mClassId, "d2", "", u32ClassType, PropertyType::Scalar );
				createProperty( cls.mClassId, "d3", "", u32ClassType, PropertyType::Scalar );
				cls.mLocked = true;
			}
		}
		virtual ~PvdObjectModelMetaDataImpl()
		{
			mStringTable.release();
			PVD_FOREACH( idx, mClasses.size() ) { if(mClasses[idx] != NULL ) PVD_DELETE( mAlloc, mClasses[idx] ); }
			mClasses.clear();
			PVD_FOREACH( idx, mProperties.size() ) PVD_DELETE( mAlloc, mProperties[idx] );
			mProperties.clear();
			PVD_FOREACH( idx, mPropertyMessages.size() ) PVD_DELETE( mAlloc, mPropertyMessages[idx] );
			mPropertyMessages.clear();
		}

		ClassDescImpl& getOrCreateClassImpl( const NamespacedName& nm, PxI32 idx )
		{
			ClassDescImpl* impl( getClassImpl( idx ) );
			if ( impl ) return *impl;
			NamespacedName safeName( mStringTable.registerStr( nm.mNamespace ), mStringTable.registerStr( nm.mName ) );
			while( idx >= (PxI32)mClasses.size() )
				mClasses.pushBack( NULL );
			mClasses[(PxU32)idx] = PVD_NEW( mAlloc, ClassDescImpl )( ClassDescription( safeName, idx), mAlloc );
			mNameToClasses.insert( nm, mClasses[(PxU32)idx] );
			mNextClassId = PxMax( mNextClassId, idx + 1 );
			return *mClasses[(PxU32)idx];
		}

		ClassDescImpl& getOrCreateClassImpl( const NamespacedName& nm )
		{
			ClassDescImpl* retval = findClassImpl( nm );
			if ( retval ) return *retval;
			return getOrCreateClassImpl( nm, nextClassId() );
		}
		virtual ClassDescription getOrCreateClass( const NamespacedName& nm )
		{
			return getOrCreateClassImpl( nm );
		}
		//get or create parent, lock parent. deriveFrom getOrCreatechild.
		virtual bool deriveClass( const NamespacedName& parent, const NamespacedName& child )
		{
			ClassDescImpl& p( getOrCreateClassImpl(parent) );
			ClassDescImpl& c( getOrCreateClassImpl(child) );
			
			if ( c.mBaseClass.hasValue() )
			{
				PX_ASSERT( c.mBaseClass.getValue() == p.mClassId.getValue() );
				return false;
			}
			p.mLocked = true;
			c.mBaseClass = p.mClassId;
			c.m32BitSizeInfo = p.m32BitSizeInfo;
			c.m64BitSizeInfo = p.m64BitSizeInfo;
			c.mPackedClassType = p.mPackedClassType;
			c.mPackedUniformWidth = p.mPackedUniformWidth;
			c.mRequiresDestruction = p.mRequiresDestruction;
			c.m32OffsetArray = p.m32OffsetArray;
			c.m64OffsetArray = p.m64OffsetArray;
			//Add all the parent propertes to this class in the global name map.
			for ( ClassDescImpl* parent0 = &p; parent0 != NULL; parent0 = getClassImpl( parent0->mBaseClass ) )
			{
				PVD_FOREACH( idx, parent0->mPropImps.size() ) 
					mNameToProperties.insert( ClassPropertyName( c.mName, parent0->mPropImps[idx]->mName ), parent0->mPropImps[idx] );
					
				if ( parent0->mBaseClass.hasValue() == false )
					break;
			}

			return true;
		}
		ClassDescImpl* findClassImpl( const NamespacedName& nm ) const
		{
			const TNameToClassMap::Entry* entry( mNameToClasses.find( nm ) );
			if ( entry )
				return entry->second;
			return NULL;
		}
		virtual Option<ClassDescription> findClass( const NamespacedName& nm ) const
		{
			ClassDescImpl* retval = findClassImpl( nm );
			if ( retval ) return *retval;
			return Option<ClassDescription>();
		}
		
		ClassDescImpl* getClassImpl( NonNegativeInteger classId ) const
		{
			if ( classId.hasValue() == false )
				return NULL;
			PxI32 idx = classId;
			if ( classId < (PxI32)mClasses.size() )
				return mClasses[(PxU32)idx];
			return NULL;
		}
	
		virtual Option<ClassDescription> getClass( NonNegativeInteger classId ) const
		{
			ClassDescImpl* impl( getClassImpl( classId ) );
			if ( impl ) return *impl;
			return None();
		}
		
		virtual Option<ClassDescription> getParentClass( NonNegativeInteger classId ) const 
		{
			ClassDescImpl* impl( getClassImpl( classId ) );
			if ( impl == NULL ) return None();
			return getClass( impl->mBaseClass );
		}

		virtual void lockClass( NonNegativeInteger classId ) 
		{
			ClassDescImpl* impl( getClassImpl( classId ) );
			PX_ASSERT( impl );
			if( impl ) impl->mLocked = true;
		}
		virtual PxU32 getNbClasses() const 
		{ 
			PxU32 total = 0;
			PVD_FOREACH( idx, mClasses.size() ) if ( mClasses[idx] ) ++total;
			return total;
		}

		virtual PxU32 getClasses( ClassDescription* outClasses, PxU32 requestCount, PxU32 startIndex = 0 ) const
		{
			PxU32 classCount( getNbClasses() );
			startIndex = PxMin( classCount, startIndex );
			PxU32 retAmount = PxMin( requestCount, classCount - startIndex );

			PxU32 idx = 0;
			while( startIndex )
			{
				if ( mClasses[idx] != NULL )
					--startIndex;
				++idx;
			}

			PxU32 inserted = 0;
			PxU32 classesSize = static_cast<PxU32>( mClasses.size() );
			while( inserted < retAmount && idx < classesSize )
			{
				if ( mClasses[idx] != NULL ) 
				{
					outClasses[inserted] = *mClasses[idx];
					++inserted;
				}
				++idx;
			}
			return inserted;
		}

		PxU32 updateByteSizeAndGetPropertyAlignment( ClassDescriptionSizeInfo& dest, const ClassDescriptionSizeInfo& src )
		{
			PxU32 alignment = src.mAlignment;
			dest.mAlignment = PxMax( dest.mAlignment, alignment );
			PxU32 offset = align( dest.mDataByteSize, alignment );
			dest.mDataByteSize = offset + src.mByteSize;
			dest.mByteSize = align( dest.mDataByteSize, dest.mAlignment );
			return offset;
		}

		void transferPtrOffsets( ClassDescriptionSizeInfo& destInfo, ForwardingArray<PtrOffset>& destArray, const ForwardingArray<PtrOffset>& src, PxU32 offset )
		{
			PVD_FOREACH( idx, src.size() )
				destArray.pushBack( PtrOffset( src[idx].mOffsetType, src[idx].mOffset + offset ) );
			destInfo.mPtrOffsets = DataRef<PtrOffset>( destArray.begin(), destArray.end() );
		}

		virtual Option<PropertyDescription> createProperty( NonNegativeInteger classId, String name, String semantic, NonNegativeInteger datatype, PropertyType::Enum propertyType )
		{
			ClassDescImpl* cls( getClassImpl( classId ) );
			PX_ASSERT( cls ); if( !cls ) return None();
			if ( cls->mLocked ) { PX_ASSERT( false ); return None(); }
			PropDescImpl* impl( cls->findProperty( name ) );
			//duplicate property definition
			if ( impl ) { PX_ASSERT( false ); return None(); }
			if ( datatype == getPvdTypeForType<String>() ) { PX_ASSERT( false ); return None(); }
			//The datatype for this property has not been declared.
			ClassDescImpl* propDType( getClassImpl( datatype ) );
			PX_ASSERT( propDType );
			if ( !propDType ) return None();
			NamespacedName propClsName( propDType->mName );
			NonNegativeInteger propPackedWidth = propDType->mPackedUniformWidth;
			NonNegativeInteger propPackedType = propDType->mPackedClassType;
			//The implications of properties being complex types aren't major 
			//*until* you start trying to undue a property event that set values
			//of those complex types.  Then things just get too complex.
			if ( propDType->mRequiresDestruction ) { PX_ASSERT( false ); return None(); }
			bool requiresDestruction = propDType->mRequiresDestruction || cls->mRequiresDestruction;

			if ( propertyType == PropertyType::Array )
			{
				PxI32 tempId = DataTypeToPvdTypeMap<ArrayData>::BaseTypeEnum;
				propDType = getClassImpl( tempId );
				PX_ASSERT( propDType );
				if ( !propDType ) return None();
				requiresDestruction = true;
			}
			PxU32 offset32 = updateByteSizeAndGetPropertyAlignment( cls->m32BitSizeInfo, propDType->m32BitSizeInfo );
			PxU32 offset64 = updateByteSizeAndGetPropertyAlignment( cls->m64BitSizeInfo, propDType->m64BitSizeInfo );
			transferPtrOffsets( cls->m32BitSizeInfo, cls->m32OffsetArray, propDType->m32OffsetArray, offset32 );
			transferPtrOffsets( cls->m64BitSizeInfo, cls->m64OffsetArray, propDType->m64OffsetArray, offset64 );
			propDType->mLocked = true; //Can't add members to the property type.
			cls->mRequiresDestruction = requiresDestruction;
			PxI32 propId((PxI32)mProperties.size());
			PropertyDescription newDesc( cls->mName, cls->mClassId, name, semantic, datatype, propClsName, propertyType, propId, offset32, offset64 );
			mProperties.pushBack( PVD_NEW( mAlloc, PropDescImpl )( newDesc, mAlloc, mStringTable ) );
			mNameToProperties.insert( ClassPropertyName( cls->mName, mProperties.back()->mName), mProperties.back() );
			cls->addProperty( mProperties.back() );
			bool firstProp = cls->mPropImps.size() == 1;
			

			if ( firstProp )
			{
				cls->mPackedUniformWidth = propPackedWidth;
				cls->mPackedClassType = propPackedType;
			}
			else 
			{
				bool packed = propPackedWidth.hasValue() && ( cls->m32BitSizeInfo.mDataByteSize % propPackedWidth ) == 0;
				if ( cls->mPackedClassType.hasValue() ) //maybe uncheck packed class type
				{
					if ( propPackedType.hasValue() == false
						|| cls->mPackedClassType != propPackedType
						//Object refs require conversion from stream to db id
						|| datatype == getPvdTypeForType<ObjectRef>()
						//Strings also require conversion from stream to db id.
						|| datatype == getPvdTypeForType<StringHandle>()
						|| packed == false )
						cls->mPackedClassType = -1;
				}
				if ( cls->mPackedUniformWidth.hasValue() ) //maybe uncheck packed class width
				{
					if ( propPackedWidth.hasValue() == false 
						|| cls->mPackedUniformWidth != propPackedWidth
						//object refs, because they require special treatment during parsing,
						//cannot be packed
						|| datatype == getPvdTypeForType<ObjectRef>()
						//Likewise, string handles are special because the data needs to be sent *after*
						//the 
						|| datatype == getPvdTypeForType<StringHandle>()
						|| packed == false )
						cls->mPackedUniformWidth = -1; //invalid packed width.
				}
			}
			return *mProperties.back();
		}

		PropDescImpl* findPropImpl( const NamespacedName& clsName, String prop ) const
		{
			const TNameToPropMap::Entry* entry = mNameToProperties.find( ClassPropertyName( clsName, prop ) );
			if ( entry ) return entry->second;
			return NULL;
		}
		virtual Option<PropertyDescription> findProperty( const NamespacedName& cls, String propName ) const
		{
			PropDescImpl* prop( findPropImpl( cls, propName ) );
			if ( prop ) return *prop;
			return None();
		}

		virtual Option<PropertyDescription> findProperty( NonNegativeInteger clsId, String propName )  const
		{
			ClassDescImpl* cls( getClassImpl( clsId ) );
			PX_ASSERT( cls ); if( !cls ) return None();
			PropDescImpl* prop( findPropImpl( cls->mName, propName ) );
			if ( prop ) return *prop;
			return None();
		}
		
		PropDescImpl* getPropertyImpl( NonNegativeInteger propId ) const
		{
			PX_ASSERT( propId.hasValue() );
			if ( propId.hasValue() == false ) return NULL;
			PxI32 val = propId.getValue();
			if ( val >= (PxI32)mProperties.size() ) { PX_ASSERT( false ); return NULL; }
			return mProperties[(PxU32)val];
		}

		virtual Option<PropertyDescription> getProperty( NonNegativeInteger propId ) const
		{
			PropDescImpl* impl( getPropertyImpl( propId ) );
			if ( impl ) return *impl;
			return None();
		}

		virtual void setNamedPropertyValues(DataRef<NamedValue> values, NonNegativeInteger propId )
		{
			PropDescImpl* impl( getPropertyImpl( propId ) );
			if ( impl )
			{
				impl->mValueNames.resize( values.size() );
				PVD_FOREACH( idx, values.size() ) impl->mValueNames[idx] = values[idx];
			}
		}

		virtual DataRef<NamedValue> getNamedPropertyValues( NonNegativeInteger propId ) const
		{
			PropDescImpl* impl( getPropertyImpl( propId ) );
			if ( impl ){ return toDataRef( impl->mValueNames ); }
			return DataRef<NamedValue>();
		}

		virtual PxU32 getNbProperties(NonNegativeInteger classId) const 
		{ 
			PxU32 retval = 0;
			for ( ClassDescImpl* impl( getClassImpl( classId ) ); impl; impl = getClassImpl( impl->mBaseClass ) )
			{
				retval += impl->mPropImps.size();
				if ( impl->mBaseClass.hasValue() == false )
					break;
			}
			return retval;
		}

		//Properties need to be returned in base class order, so this requires a recursive function.
		PxU32 getPropertiesImpl( NonNegativeInteger classId, PropertyDescription*& outBuffer, PxU32 &numItems, PxU32 &startIdx ) const
		{
			ClassDescImpl* impl( getClassImpl( classId ) );
			if ( impl )
			{
				PxU32 retval = 0;
				if ( impl->mBaseClass.hasValue() )
					retval = getPropertiesImpl( impl->mBaseClass, outBuffer, numItems, startIdx );
				
				PxU32 localStart = PxMin( impl->mPropImps.size(), startIdx );
				PxU32 localNumItems = PxMin( numItems, impl->mPropImps.size() - localStart );
				PVD_FOREACH( idx, localNumItems ) { outBuffer[idx] = *impl->mPropImps[localStart + idx]; }

				startIdx -= localStart;
				numItems -= localNumItems;
				outBuffer += localNumItems;
				return retval + localNumItems;
			}
			return 0;


		}
		
		virtual PxU32 getProperties( NonNegativeInteger classId, PropertyDescription* outBuffer, PxU32 numItems, PxU32 startIdx ) const
		{
			return getPropertiesImpl( classId, outBuffer, numItems, startIdx );
		}
		
		virtual MarshalQueryResult checkMarshalling( NonNegativeInteger srcClsId, NonNegativeInteger dstClsId ) const
		{
			Option<ClassDescription> propTypeOpt( getClass( dstClsId ) );
			if ( propTypeOpt.hasValue() == false ) { PX_ASSERT( false ); return MarshalQueryResult(); }
			const ClassDescription& propType( propTypeOpt );

			Option<ClassDescription> incomingTypeOpt( getClass( srcClsId ) );
			if ( incomingTypeOpt.hasValue() == false ) { PX_ASSERT( false ); return MarshalQueryResult(); }
			const ClassDescription& incomingType( incomingTypeOpt );
			//Can only marshal simple things at this point in time.
			bool needsMarshalling = false;
			bool canMarshal = false;
			TSingleMarshaller single = NULL;
			TBlockMarshaller block = NULL;
			if ( incomingType.mClassId != propType.mClassId )
			{
				//Check that marshalling is even possible.
				if ( ( incomingType.mPackedUniformWidth.hasValue() && propType.mPackedUniformWidth.hasValue() ) == false ) { PX_ASSERT( false ); return MarshalQueryResult(); }

				PxI32 srcType = incomingType.mPackedClassType;
				PxI32 dstType = propType.mPackedClassType;

				PxI32 srcWidth = incomingType.mPackedUniformWidth;
				PxI32 dstWidth = propType.mPackedUniformWidth;
				canMarshal = getMarshalOperators( single, block, srcType, dstType );
				if ( srcWidth == dstWidth )
					needsMarshalling = canMarshal; //If the types are the same width, we assume we can convert between some of them seamlessly (PxU16, PxI16)
				else
				{
					needsMarshalling = true;
					//If we can't marshall and we have to then we can't set the property value.
					//This indicates that the src and dest are different properties and we don't
					//know how to convert between them.
					if ( !canMarshal ) { PX_ASSERT( false ); return MarshalQueryResult(); }
				}
			}
			return MarshalQueryResult( srcClsId, dstClsId, canMarshal, needsMarshalling, block );
		}

		PropertyMessageDescriptionImpl* findPropertyMessageImpl( const NamespacedName& messageName ) const
		{
			const TNameToPropertyMessageMap::Entry* entry = mPropertyMessageMap.find( messageName );
			if ( entry ) return entry->second;
			return NULL;
		}

		PropertyMessageDescriptionImpl* getPropertyMessageImpl( NonNegativeInteger msg ) const
		{
			PxI32 msgCount = (PxI32)mPropertyMessages.size();
			if ( msg.hasValue() && msg.getValue() < msgCount )
				return mPropertyMessages[(PxU32)msg];
			return NULL;
		}

		
		virtual Option<PropertyMessageDescription> createPropertyMessage( const NamespacedName& clsName, const NamespacedName& messageName, DataRef<PropertyMessageArg> entries, PxU32 messageSize )
		{
			PropertyMessageDescriptionImpl* existing( findPropertyMessageImpl( messageName ) );
			if ( existing ) { PX_ASSERT( false ); return None(); }
			ClassDescImpl* cls = findClassImpl( clsName );
			PX_ASSERT( cls );
			if ( !cls ) return None();
			PxI32 msgId = (PxI32)mPropertyMessages.size();
			PropertyMessageDescriptionImpl* newMessage = PVD_NEW( mAlloc, PropertyMessageDescriptionImpl )( PropertyMessageDescription( mStringTable.registerName( clsName ), cls->mClassId, mStringTable.registerName( messageName ), msgId, messageSize ), mAlloc );
			PxU32 calculatedSize = 0;
			PVD_FOREACH( idx, entries.size() )
			{
				PropertyMessageArg entry(entries[idx]);
				ClassDescImpl* dtypeCls = findClassImpl( entry.mDatatypeName );
				if ( dtypeCls == NULL ) { PX_ASSERT( false ); goto DestroyNewMessage; }
				ClassDescriptionSizeInfo dtypeInfo( dtypeCls->m32BitSizeInfo );
				PxU32 incomingSize = dtypeInfo.mByteSize;
				if ( entry.mByteSize < incomingSize ) { PX_ASSERT( false ); goto DestroyNewMessage; }

				calculatedSize = PxMax( calculatedSize, entry.mMessageOffset + entry.mByteSize );
				if ( calculatedSize > messageSize ) { PX_ASSERT( false ); goto DestroyNewMessage; }

				Option<PropertyDescription> propName( findProperty( cls->mClassId, entry.mPropertyName ) );
				if ( propName.hasValue() == false ) { PX_ASSERT( false ); goto DestroyNewMessage; }

				Option<ClassDescription> propCls( getClass( propName.getValue().mDatatype ) );
				if ( propCls.hasValue() == false ) { PX_ASSERT( false ); goto DestroyNewMessage; }

				PropertyMessageEntryImpl newEntry( PropertyMessageEntry( propName, dtypeCls->mName, dtypeCls->mClassId, entry.mMessageOffset, incomingSize, dtypeInfo.mByteSize ) );
				newMessage->addEntry( newEntry );
				
				if ( newEntry.mDatatypeId == getPvdTypeForType<String>() ) newMessage->mStringOffsetArray.pushBack( entry.mMessageOffset );

				//property messages cannot be marshalled at this time.
				if ( newEntry.mDatatypeId != getPvdTypeForType<String>() && newEntry.mDatatypeId != getPvdTypeForType<VoidPtr>() )
				{
					MarshalQueryResult marshalInfo = checkMarshalling( newEntry.mDatatypeId, newEntry.mProperty.mDatatype );
					if ( marshalInfo.needsMarshalling ) { PX_ASSERT( false ); goto DestroyNewMessage; }
				}
			}

			if ( newMessage )
			{
				newMessage->mStringOffsets = DataRef<PxU32>( newMessage->mStringOffsetArray.begin(), newMessage->mStringOffsetArray.end() );
				mPropertyMessages.pushBack( newMessage );
				mPropertyMessageMap.insert( messageName, newMessage );
				return *newMessage;
			}

DestroyNewMessage:
			if ( newMessage )
				PVD_DELETE( mAlloc, newMessage );

			return None();
		}
		virtual Option<PropertyMessageDescription> findPropertyMessage( const NamespacedName& msgName ) const
		{
			PropertyMessageDescriptionImpl* desc(findPropertyMessageImpl( msgName ) );
			if ( desc ) return *desc;
			return None();
		}

		virtual Option<PropertyMessageDescription> getPropertyMessage( NonNegativeInteger msgId ) const 
		{
			PropertyMessageDescriptionImpl* desc(getPropertyMessageImpl( msgId ) );
			if ( desc ) return *desc;
			return None();
		}

		virtual PxU32 getNbPropertyMessages() const
		{
			return (PxU32)mPropertyMessages.size();
		}

		virtual PxU32 getPropertyMessages( PropertyMessageDescription* msgBuf, PxU32 bufLen, PxU32 startIdx = 0 ) const
		{
			startIdx = PxMin( startIdx, getNbPropertyMessages() );
			bufLen = PxMin( bufLen, getNbPropertyMessages() - startIdx );
			PVD_FOREACH( idx, bufLen ) msgBuf[idx] = *mPropertyMessages[idx + startIdx];
			return bufLen;
		}


		struct MetaDataWriter
		{
			const PvdObjectModelMetaDataImpl& mMetaData;
			PvdOutputStream&				mStream;
			MetaDataWriter( const PvdObjectModelMetaDataImpl& meta, PvdOutputStream& stream )
				: mMetaData( meta ), mStream( stream )
			{
			}

			void streamify( NamespacedName& type ) 
			{ 
				mStream << mMetaData.mStringTable.strToHandle( type.mNamespace ); 
				mStream << mMetaData.mStringTable.strToHandle( type.mName );
			}
			void streamify( String& type ) { mStream << mMetaData.mStringTable.strToHandle( type ); }
			void streamify( NonNegativeInteger& type ) { mStream << type; }
			void streamify( PxU32& type ) { mStream << type; }
			void streamify( PxU8 type ) { mStream << type; }
			void streamify( bool type ) { streamify( type ? (PxU8)1 : (PxU8)0 ); }
			void streamify( PropertyType::Enum type ) { PxU32 val = static_cast<PxU32>( type ); mStream << val; }
			void streamify( NamedValue& type ) { streamify( type.mValue ); streamify( type.mName ); }
			void streamifyLinks( PropDescImpl* prop ) { streamify( prop->mPropertyId ); }
			void streamify( PropertyDescription& prop ) { streamify( prop.mPropertyId ); }
			void streamify( PropertyMessageEntryImpl& prop ) { prop.serialize( *this ); }
			void streamify( PtrOffset& off )
			{
				PxU32 type = off.mOffsetType;
				mStream << type;
				mStream << off.mOffset;
			}
			template<typename TDataType>
			void streamify( TDataType* type ) 
			{ 
				PxU32 existMarker = type ? (PxU32)1 : 0;
				mStream << existMarker;
				if(type) type->serialize( *this ); 
			}
			template<typename TArrayType>
			void streamify( const ForwardingArray<TArrayType>& type )
			{
				mStream << static_cast<PxU32>( type.size() );
				PVD_FOREACH( idx, type.size() ) streamify( const_cast<TArrayType&>( type[(PxU32)idx] ) );
			}
			template<typename TArrayType>
			void streamifyLinks( const ForwardingArray<TArrayType>& type )
			{
				mStream << static_cast<PxU32>( type.size() );
				PVD_FOREACH( idx, type.size() ) streamifyLinks( const_cast<TArrayType&>( type[(PxU32)idx] ) );
			}

		private:
			MetaDataWriter& operator=(const MetaDataWriter&);
		};
		
		
		template<typename TStreamType>
		struct MetaDataReader
		{
			PvdObjectModelMetaDataImpl& mMetaData;
			TStreamType&				mStream;
			MetaDataReader( PvdObjectModelMetaDataImpl& meta, TStreamType& stream )
				: mMetaData( meta ), mStream( stream )
			{
			}

			void streamify( NamespacedName& type ) 
			{ 
				streamify( type.mNamespace );
				streamify( type.mName );
			}

			void streamify( String& type ) 
			{ 
				PxU32 handle;
				mStream >> handle;
				type = mMetaData.mStringTable.handleToStr( handle );
			}
			void streamify( NonNegativeInteger& type ) { PxI32 val; streamify( val ); type = val; }
			void streamify( PxI32& type ) { mStream >> type; }
			void streamify( PxU32& type ) { mStream >> type; }
			void streamify( bool& type ) 
			{ 
				PxU8 data;
				mStream >> data;
				type = data ? true : false;
			}

			void streamify( PropertyType::Enum& type ) 
			{ 
				PxU32 val;
				mStream >> val;
				type = static_cast<PropertyType::Enum>( val );
			}
			void streamify( NamedValue& type ) { streamify( type.mValue ); streamify( type.mName ); }
			void streamify( PropertyMessageEntryImpl& type ) { type.serialize( *this ); }
			void streamify( PtrOffset& off )
			{
				PxU32 type;
				mStream >> type;
				mStream >> off.mOffset;
				off.mOffsetType = static_cast<PtrOffsetType::Enum>( type );
			}
			void streamifyLinks( PropDescImpl*& prop ) 
			{ 
				NonNegativeInteger propId;
				streamify( propId );
				prop = mMetaData.getPropertyImpl( propId );
			}
			void streamify( PropertyDescription& prop ) 
			{ 
				streamify( prop.mPropertyId ); 
				prop = mMetaData.getProperty( prop.mPropertyId );
			}
			template<typename TDataType>
			void streamify( TDataType*& type ) 
			{ 
				PxU32 existMarker;
				mStream >> existMarker;
				if ( existMarker )
				{
					TDataType* newType = PVD_NEW( mMetaData.mAlloc, TDataType )(mMetaData.mAlloc);
					newType->serialize( *this ); 
					type = newType;
				}
				else
					type = NULL;
			}
			template<typename TArrayType>
			void streamify( ForwardingArray<TArrayType>& type )
			{
				PxU32 typeSize;
				mStream >> typeSize;
				type.resize( typeSize );
				PVD_FOREACH( idx, type.size() ) streamify( type[idx] );
			}
			template<typename TArrayType>
			void streamifyLinks( ForwardingArray<TArrayType>& type )
			{
				PxU32 typeSize;
				mStream >> typeSize;
				type.resize( typeSize );
				PVD_FOREACH( idx, type.size() ) streamifyLinks( type[idx] );
			}
		private:
			MetaDataReader& operator=(const MetaDataReader&);
		};

		virtual void write( PvdOutputStream& stream ) const
		{
			stream << getCurrentPvdObjectModelVersion();
			stream << mNextClassId;
			mStringTable.write( stream );
			MetaDataWriter writer( *this, stream );
			writer.streamify( mProperties );
			writer.streamify( mClasses );
			writer.streamify( mPropertyMessages );
		}

		template<typename TReaderType>
		void read( TReaderType& stream )
		{
			PxU32 version;
			stream >> version;
			stream >> mNextClassId;
			mStringTable.read( stream );
			MetaDataReader<TReaderType> reader( *this, stream );
			reader.streamify( mProperties );
			reader.streamify( mClasses );
			reader.streamify( mPropertyMessages );
			
			mNameToClasses.clear();
			mNameToProperties.clear();
			mPropertyMessageMap.clear();
			PVD_FOREACH( i, mClasses.size() ) 
			{
				ClassDescImpl* cls( mClasses[i] );
				if ( cls == NULL ) continue;
				mNameToClasses.insert( cls->mName, mClasses[i] );
				PxU32 propCount = getNbProperties( cls->mClassId );
				PropertyDescription descs[16];
				PxU32 offset = 0;
				for( PxU32 idx = 0; idx < propCount; idx = offset )
				{
					PxU32 numProps = getProperties( cls->mClassId, descs, 16, offset );
					offset += numProps;
					for ( PxU32 propIdx = 0; propIdx < numProps; ++propIdx )
					{
						PropDescImpl* prop = getPropertyImpl( descs[propIdx].mPropertyId );
						if ( prop )
							mNameToProperties.insert( ClassPropertyName( cls->mName, prop->mName ), prop);
					}
				}
			}
			PVD_FOREACH( idx, mPropertyMessages.size() )
				mPropertyMessageMap.insert( mPropertyMessages[idx]->mMessageName, mPropertyMessages[idx] );
		}
		
		virtual PvdObjectModelMetaData& clone() const
		{
			MemPvdOutputStream tempStream( mAlloc, "PvdObjectModelMetaData::clone" );
			write( tempStream );
			MemPvdInputStream inStream( tempStream );
			return create( mAlloc, inStream );
		}

		virtual StringTable& getStringTable() const { return mStringTable; }
		virtual void addRef()
		{ 
			++mRefCount; 
		}
		virtual void release() 
		{ 
			if ( mRefCount ) --mRefCount;
			if ( !mRefCount ) PVD_DELETE( mAlloc, this ); 
		}

	};
}

PxU32 PvdObjectModelMetaData::getCurrentPvdObjectModelVersion() { return 1; }

PvdObjectModelMetaData& PvdObjectModelMetaData::create( PxAllocatorCallback& alloc )
{
	PvdObjectModelMetaDataImpl& retval( *PVD_NEW( alloc, PvdObjectModelMetaDataImpl )( alloc ) );
	retval.initialize();
	return retval;
}

PvdObjectModelMetaData& PvdObjectModelMetaData::create( PxAllocatorCallback& alloc, PvdInputStream& stream )
{
	PvdObjectModelMetaDataImpl& retval( *PVD_NEW( alloc, PvdObjectModelMetaDataImpl )( alloc ) );
	retval.read( stream );
	return retval;
}


StringTable& StringTable::create( PxAllocatorCallback& alloc )
{
	return *PVD_NEW( alloc, StringTableImpl )( alloc );
}
