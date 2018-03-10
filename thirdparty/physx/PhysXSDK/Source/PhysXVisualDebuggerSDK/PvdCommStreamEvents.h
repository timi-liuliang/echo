/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PVD_COMM_STREAM_EVENTS_H
#define PVD_COMM_STREAM_EVENTS_H
#include "PvdObjectModelBaseTypes.h"
#include "PxFlags.h"
#include "PsTime.h"
#include "PxVec3.h"
#include "PvdConnection.h"

namespace physx { namespace debugger { namespace comm {

	struct CommStreamFlagTypes
	{
		enum Enum
		{
			PtrIs64Bit = 1
		};
	};

	typedef PxFlags<CommStreamFlagTypes::Enum, PxU32> CommStreamFlags;

	template<typename TDataType> struct PvdCommVariableSizedEventCheck { bool variable_size_check; };

	//Pick out the events that are possibly very large.
	//This helps us keep our buffers close to the size the user requested.
#define DECLARE_TYPE_VARIABLE_SIZED( type ) \
	template<> struct PvdCommVariableSizedEventCheck<type> { PxU32 variable_size_check; };

	struct NameHandleValue;
	struct StreamPropMessageArg;
	

	struct PvdCommStreamEventTypes
	{
		enum Enum
		{
			Unknown = 0,
#define DECLARE_PVD_COMM_STREAM_EVENT( x ) x,
#define DECLARE_PVD_COMM_STREAM_EVENT_NO_COMMA( x ) x
#include "PvdCommStreamEventTypes.h"
#undef DECLARE_PVD_COMM_STREAM_EVENT_NO_COMMA 
#undef DECLARE_PVD_COMM_STREAM_EVENT
		};
	};

	struct StreamNamespacedName
	{
		StringHandle mNamespace; //StringHandle handles
		StringHandle mName;
		StreamNamespacedName( StringHandle ns = 0, StringHandle nm = 0 ) : mNamespace( ns ), mName( nm ) {}
	};

	class EventSerializeable;

	class EventSerializer
	{
	protected:
		virtual ~EventSerializer(){}
	public:
		
		virtual void streamify( PxU8& val ) = 0;
		virtual void streamify( PxU16& val ) = 0;
		virtual void streamify( PxU32& val ) = 0;
		virtual void streamify( PxF32& val ) = 0;
		virtual void streamify( PxU64& val ) = 0;
		virtual void streamify( String& val ) = 0;
		virtual void streamify( DataRef<const PxU8>& data ) = 0;
		virtual void streamify( DataRef<NameHandleValue>& data ) = 0;
		virtual void streamify( DataRef<StreamPropMessageArg>& data ) = 0;
		virtual void streamify( DataRef<StringHandle>& data ) = 0;

		void streamify( StringHandle& hdl )
		{
			streamify( hdl.mHandle );
		}
		void streamify( CommStreamFlags& flags )
		{
			PxU32 val(flags);
			streamify( val );
			flags = CommStreamFlags( val );
		}

		void streamify( PvdCommStreamEventTypes::Enum& val) 
		{
			PxU8 detyped = static_cast<PxU8>( val );
			streamify( detyped );
			val = static_cast<PvdCommStreamEventTypes::Enum>( detyped );
		}
		void streamify( PropertyType::Enum& val )
		{
			PxU8 detyped = static_cast<PxU8>( val );
			streamify( detyped );
			val = static_cast<PropertyType::Enum>( detyped );
		}
		
		void streamify( bool& val )
		{
			PxU8 detyped = PxU8(val ? 1 : 0);
			streamify( detyped );
			val = detyped ? true : false;
		}

		void streamify( StreamNamespacedName& name )
		{
			streamify( name.mNamespace );
			streamify( name.mName );
		} 

		void streamify( PvdColor& color )
		{
			streamify( color.r );
			streamify( color.g );
			streamify( color.b );
			streamify( color.a );
		}

		void streamify( PxVec3& vec )
		{
			streamify( vec.x );
			streamify( vec.y );
			streamify( vec.z );
		}
		
		static PxU32 measure( const EventSerializeable& evt );
	};

	class EventSerializeable
	{
	protected:
		virtual ~EventSerializeable(){}
	public:

		virtual void serialize( EventSerializer& serializer ) = 0;
	};

	/** Numbers generated from random.org
	129919156	17973702	401496246	144984007	336950759
	907025328	837150850	679717896	601529147	269478202
	*/
	struct StreamInitialization : public EventSerializeable
	{
		static PxU32 getStreamId() { return 837150850; }
		static PxU32 getStreamVersion() { return 1; }

		PxU32			mStreamId;
		PxU32			mStreamVersion;
		PxU64			mTimestampNumerator;
		PxU64			mTimestampDenominator;
		CommStreamFlags mStreamFlags;
		StreamInitialization()
			: mStreamId( getStreamId() )
			, mStreamVersion( getStreamVersion() )
			, mTimestampNumerator( physx::shdfnd::Time::getCounterFrequency().mNumerator * 10 )
			, mTimestampDenominator( physx::shdfnd::Time::getCounterFrequency().mDenominator )
			, mStreamFlags( sizeof( void* ) == 4 ? 0 : 1 )
		{
		}

		void serialize( EventSerializer& s )
		{
			s.streamify( mStreamId );
			s.streamify( mStreamVersion );
			s.streamify( mTimestampNumerator );
			s.streamify( mTimestampDenominator );
			s.streamify( mStreamFlags );
		}
	};

	struct EventGroup : public EventSerializeable
	{
		PxU32 mDataSize; //in bytes, data directly follows this header
		PxU32 mNumEvents;
		PxU64 mStreamId;
		PxU64 mTimestamp;

		EventGroup( PxU32 dataSize = 0, PxU32 numEvents = 0, PxU64 streamId = 0, PxU64 ts = 0 )
			: mDataSize( dataSize )
			, mNumEvents( numEvents )
			, mStreamId( streamId )
			, mTimestamp( ts )
		{
		}
		
		
		void serialize( EventSerializer& s )
		{
			s.streamify( mDataSize );
			s.streamify( mNumEvents );
			s.streamify( mStreamId );
			s.streamify( mTimestamp );
		}
	};

	struct StringHandleEvent : public EventSerializeable
	{
		String mString;
		PxU32  mHandle;
		StringHandleEvent( String str, PxU32 hdl ) : mString( str ), mHandle( hdl ) {}
		StringHandleEvent(){}
		
		void serialize( EventSerializer& s )
		{
			s.streamify( mString );
			s.streamify( mHandle );
		}
	};

	DECLARE_TYPE_VARIABLE_SIZED( StringHandleEvent )

	
	typedef PxU64 Timestamp;

	//virtual PvdError createClass( StreamNamespacedName nm ) = 0;
	struct CreateClass : public EventSerializeable
	{
		StreamNamespacedName mName;
		CreateClass( StreamNamespacedName nm ) : mName( nm ) {}
		CreateClass(){}
		
		void serialize( EventSerializer& s )
		{
			s.streamify( mName );
		}
	};

	//virtual PvdError deriveClass( StreamNamespacedName parent, StreamNamespacedName child ) = 0;
	struct DeriveClass : public EventSerializeable
	{
		StreamNamespacedName mParent;
		StreamNamespacedName mChild;

		DeriveClass( StreamNamespacedName p, StreamNamespacedName c ) : mParent( p ), mChild( c ) {}
		DeriveClass(){}
		
		
		void serialize( EventSerializer& s )
		{
			s.streamify( mParent );
			s.streamify( mChild );
		}
	};

	struct NameHandleValue : public EventSerializeable
	{
		StringHandle	mName;
		PxU32			mValue;
		NameHandleValue( StringHandle name, PxU32 val )
			: mName( name ), mValue( val ) {}
		NameHandleValue(){}

		void serialize( EventSerializer& s )
		{
			s.streamify( mName );
			s.streamify( mValue );
		}
	};
	/*virtual PvdError createProperty( StreamNamespacedName clsName, StringHandle name, StringHandle semantic
										, StreamNamespacedName dtypeName, PropertyType::Enum propertyType
										, DataRef<NamedValue> values = DataRef<NamedValue>() ) = 0; */
	struct CreateProperty : public EventSerializeable
	{
		StreamNamespacedName		mClass;
		StringHandle				mName; 
		StringHandle				mSemantic;
		StreamNamespacedName		mDatatypeName;
		PropertyType::Enum			mPropertyType;
		DataRef<NameHandleValue>	mValues;

		CreateProperty( StreamNamespacedName cls, StringHandle name, StringHandle semantic, StreamNamespacedName dtypeName, PropertyType::Enum ptype, DataRef<NameHandleValue> values )
			: mClass( cls ), mName( name ), mSemantic( semantic ), mDatatypeName( dtypeName ), mPropertyType( ptype ), mValues( values ) {}
		CreateProperty(){}
		
		
		void serialize( EventSerializer& s )
		{
			s.streamify( mClass );
			s.streamify( mName );
			s.streamify( mSemantic );
			s.streamify( mDatatypeName );
			s.streamify( mPropertyType );
			s.streamify( mValues );
		}
	};

	struct StreamPropMessageArg : public EventSerializeable
	{
		StringHandle			mPropertyName;
		StreamNamespacedName	mDatatypeName;
		PxU32					mMessageOffset; 
		PxU32					mByteSize;
		StreamPropMessageArg( StringHandle pname, StreamNamespacedName dtypeName, PxU32 offset, PxU32 byteSize )
			: mPropertyName( pname ), mDatatypeName( dtypeName ), mMessageOffset( offset ), mByteSize( byteSize ) {}

		StreamPropMessageArg(){}
		
		void serialize( EventSerializer& s )
		{
			s.streamify( mPropertyName );
			s.streamify( mDatatypeName );
			s.streamify( mMessageOffset );
			s.streamify( mByteSize );
		}

	};

	/*
		virtual PvdError createPropertyMessage( StreamNamespacedName cls, StreamNamespacedName msgName
													, DataRef<PropertyMessageArg> entries, PxU32 messageSizeInBytes ) = 0;*/
	struct CreatePropertyMessage : public EventSerializeable
	{
		StreamNamespacedName				mClass;
		StreamNamespacedName				mMessageName;
		DataRef<StreamPropMessageArg>		mMessageEntries;
		PxU32								mMessageByteSize;

		CreatePropertyMessage( StreamNamespacedName cls, StreamNamespacedName msgName, DataRef<StreamPropMessageArg> propArg, PxU32 messageByteSize )
			: mClass( cls ), mMessageName( msgName ), mMessageEntries( propArg ), mMessageByteSize( messageByteSize ) {}
		CreatePropertyMessage(){}
		
		void serialize( EventSerializer& s )
		{
			s.streamify( mClass );
			s.streamify( mMessageName );
			s.streamify( mMessageEntries );
			s.streamify( mMessageByteSize );
		}
	};

	/**Changing immediate data on instances*/



	
	//virtual PvdError createInstance( StreamNamespacedName cls, PxU64 instance ) = 0;
	struct CreateInstance : public EventSerializeable
	{
		StreamNamespacedName	mClass;
		PxU64					mInstanceId;

		CreateInstance( StreamNamespacedName cls, PxU64 streamId )
			: mClass( cls ), mInstanceId( streamId ) {}
		CreateInstance(){}

		
		void serialize( EventSerializer& s )
		{
			s.streamify( mClass );
			s.streamify( mInstanceId );
		}
	};


	//virtual PvdError setPropertyValue( PxU64 instance, StringHandle name, DataRef<const PxU8> data, StreamNamespacedName incomingTypeName ) = 0;
	struct SetPropertyValue : public EventSerializeable
	{
		PxU64					mInstanceId;
		StringHandle			mPropertyName;
		DataRef<const PxU8>		mData;
		StreamNamespacedName	mIncomingTypeName;
		PxU32					mNumItems;

		SetPropertyValue( PxU64 instance, StringHandle name, DataRef<const PxU8> data, StreamNamespacedName incomingTypeName, PxU32 numItems )
			: mInstanceId( instance ), mPropertyName( name ), mData( data ), mIncomingTypeName( incomingTypeName ), mNumItems( numItems ) {}
		
		SetPropertyValue(){}

		void serializeBeginning( EventSerializer& s )
		{
			s.streamify( mInstanceId );
			s.streamify( mPropertyName );
			s.streamify( mIncomingTypeName );
			s.streamify( mNumItems );
		}

		
		void serialize( EventSerializer& s )
		{
			serializeBeginning( s );
			s.streamify( mData );
		}
	};
	
	DECLARE_TYPE_VARIABLE_SIZED( SetPropertyValue )

	
	//virtual PvdError beginSetPropertyValue( PxU64 instance, StringHandle name, StreamNamespacedName incomingTypeName ) = 0;
	struct BeginSetPropertyValue : public EventSerializeable
	{
		PxU64					mInstanceId;
		StringHandle			mPropertyName;
		StreamNamespacedName	mIncomingTypeName;


		BeginSetPropertyValue( PxU64 instance, StringHandle name, StreamNamespacedName incomingTypeName )
			: mInstanceId( instance ), mPropertyName( name ), mIncomingTypeName( incomingTypeName ) {}
		BeginSetPropertyValue(){}
		
		
		void serialize( EventSerializer& s )
		{
			s.streamify( mInstanceId );
			s.streamify( mPropertyName );
			s.streamify( mIncomingTypeName );
		}
	};

		
	//virtual PvdError appendPropertyValueData( DataRef<const PxU8> data ) = 0;
	struct AppendPropertyValueData : public EventSerializeable
	{
		DataRef<const PxU8>		mData;
		PxU32					mNumItems;
		AppendPropertyValueData( DataRef<const PxU8> data, PxU32 numItems ) : mData( data ), mNumItems( numItems ) {}
		AppendPropertyValueData() {}
		
		
		void serialize( EventSerializer& s )
		{
			s.streamify( mData );
			s.streamify( mNumItems );
		}
	};
	
	DECLARE_TYPE_VARIABLE_SIZED( AppendPropertyValueData )

	//virtual PvdError endSetPropertyValue() = 0;
	struct EndSetPropertyValue : public EventSerializeable
	{
		EndSetPropertyValue(){}
		
		void serialize( EventSerializer& )
		{
		}
	};
	
	//virtual PvdError setPropertyMessage( PxU64 instance, StreamNamespacedName msgName, DataRef<const PxU8> data ) = 0;
	struct SetPropertyMessage : public EventSerializeable
	{
		PxU64					mInstanceId;
		StreamNamespacedName	mMessageName;
		DataRef<const PxU8>		mData;

		SetPropertyMessage( PxU64 instance, StreamNamespacedName msgName, DataRef<const PxU8> data )
			: mInstanceId( instance ), mMessageName( msgName ), mData( data ) {}

		SetPropertyMessage(){}
		
		void serialize( EventSerializer& s)
		{
			s.streamify( mInstanceId );
			s.streamify( mMessageName );
			s.streamify( mData );
		}
	};
	
	DECLARE_TYPE_VARIABLE_SIZED( SetPropertyMessage )

	//virtual PvdError beginPropertyMessageGroup( StreamNamespacedName msgName ) = 0;
	struct BeginPropertyMessageGroup : public EventSerializeable
	{
		StreamNamespacedName mMsgName;
		BeginPropertyMessageGroup( StreamNamespacedName msgName ) : mMsgName( msgName ) {}
		BeginPropertyMessageGroup(){}

		
		void serialize( EventSerializer& s)
		{
			s.streamify( mMsgName );
		}
	};

	//virtual PvdError sendPropertyMessageFromGroup( PxU64 instance, DataRef<const PxU8*> data ) = 0;
	struct SendPropertyMessageFromGroup : public EventSerializeable
	{
		PxU64					mInstance;
		DataRef<const PxU8>		mData;

		SendPropertyMessageFromGroup( PxU64 instance, DataRef<const PxU8> data )
			: mInstance( instance ), mData( data ) {}
		SendPropertyMessageFromGroup(){}
		
		
		void serialize( EventSerializer& s)
		{
			s.streamify( mInstance );
			s.streamify( mData );
		}
	};
	
	DECLARE_TYPE_VARIABLE_SIZED( SendPropertyMessageFromGroup )

	//virtual PvdError endPropertyMessageGroup() = 0;
	struct EndPropertyMessageGroup : public EventSerializeable
	{
		EndPropertyMessageGroup(){}
		
		void serialize( EventSerializer& )
		{
		}
	};
	
	struct PushBackObjectRef : public EventSerializeable
	{
		PxU64					mInstanceId;
		StringHandle			mProperty;
		PxU64					mObjectRef;

		PushBackObjectRef( PxU64 instId, StringHandle prop, PxU64 objRef )
			: mInstanceId( instId ), mProperty( prop ), mObjectRef( objRef ) {}

		PushBackObjectRef(){}
		
		void serialize( EventSerializer& s)
		{
			s.streamify( mInstanceId );
			s.streamify( mProperty );
			s.streamify( mObjectRef );
		}
	};

	struct RemoveObjectRef  : public EventSerializeable
	{
		PxU64					mInstanceId;
		StringHandle			mProperty;
		PxU64					mObjectRef;

		RemoveObjectRef( PxU64 instId, StringHandle prop, PxU64 objRef )
			: mInstanceId( instId ), mProperty( prop ), mObjectRef( objRef ) {}

		RemoveObjectRef(){}
		
		void serialize( EventSerializer& s)
		{
			s.streamify( mInstanceId );
			s.streamify( mProperty );
			s.streamify( mObjectRef );
		}
	};

	//virtual PvdError destroyInstance( PxU64 key ) = 0;
	struct DestroyInstance : public EventSerializeable
	{
		PxU64 mInstanceId;
		DestroyInstance( PxU64 instance ) : mInstanceId( instance ) {}
		DestroyInstance(){}
		
		void serialize( EventSerializer& s)
		{
			s.streamify( mInstanceId );
		}
	};

	//virtual PvdError beginSection( PxU64 sectionId, StringHandle name ) = 0;
	struct BeginSection : public EventSerializeable
	{
		PxU64			mSectionId;
		StringHandle	mName;
		Timestamp		mTimestamp;
		BeginSection( PxU64 sectionId, StringHandle name, PxU64 timestamp ) 
			: mSectionId( sectionId ), mName( name ), mTimestamp( timestamp ) {}
		BeginSection() {}
		
		void serialize( EventSerializer& s)
		{
			s.streamify( mSectionId );
			s.streamify( mName );
			s.streamify( mTimestamp );
		}

	};
	//virtual PvdError endSection( PxU64 sectionId, StringHandle name ) = 0;
	struct EndSection : public EventSerializeable
	{
		PxU64			mSectionId;
		StringHandle	mName;
		Timestamp		mTimestamp;
		EndSection( PxU64 sectionId, StringHandle name, PxU64 timestamp ) 
			: mSectionId( sectionId ), mName( name ), mTimestamp( timestamp ) {}
		EndSection() {}
		
		void serialize( EventSerializer& s)
		{
			s.streamify( mSectionId );
			s.streamify( mName );
			s.streamify( mTimestamp );
		}
	};

	
	//virtual void setPickable( void* instance, bool pickable ) = 0;
	struct SetPickable : public EventSerializeable
	{
		PxU64	mInstanceId;
		bool	mPickable;
		SetPickable( PxU64 instId, bool pick ) 
			: mInstanceId( instId ), mPickable( pick ) {}
		SetPickable(){}
		
		
		void serialize( EventSerializer& s)
		{
			s.streamify( mInstanceId );
			s.streamify( mPickable );
		}
	};
	//virtual void setColor( void* instance, const PvdColor& color ) = 0;
	struct SetColor : public EventSerializeable
	{
		PxU64		mInstanceId;
		PvdColor	mColor;
		SetColor( PxU64 instId, PvdColor color ) 
			: mInstanceId( instId ), mColor( color ) {}
		SetColor(){}
		
		
		void serialize( EventSerializer& s)
		{
			s.streamify( mInstanceId );
			s.streamify( mColor );
		}
	};

	//virtual void setColor( void* instance, const PvdColor& color ) = 0;
	struct SetIsTopLevel : public EventSerializeable
	{
		PxU64		mInstanceId;
		bool		mIsTopLevel;

		SetIsTopLevel( PxU64 instId, bool topLevel ) 
			: mInstanceId( instId ), mIsTopLevel( topLevel ) {}
		SetIsTopLevel() : mIsTopLevel( false ) {}
		
		void serialize( EventSerializer& s)
		{
			s.streamify( mInstanceId );
			s.streamify( mIsTopLevel );
		}
	};

	struct SetCamera : public EventSerializeable
	{
		String		 mName;
		PxVec3		 mPosition;
		PxVec3		 mUp;
		PxVec3		 mTarget;
		SetCamera( String name, const PxVec3& pos, const PxVec3& up, const PxVec3& target )
			: mName( name ), mPosition( pos ), mUp( up ), mTarget( target ) {}
		SetCamera() : mName( NULL ) {}

		void serialize( EventSerializer& s)
		{
			s.streamify( mName );
			s.streamify( mPosition );
			s.streamify( mUp );
			s.streamify( mTarget );
		}
	};

	struct ErrorMessage : public EventSerializeable
	{
		PxU32			mCode;
		String			mMessage;
		String			mFile;
		PxU32			mLine;

		ErrorMessage( PxU32 code, String message, String file, PxU32 line )
			: mCode( code ), mMessage( message ), mFile( file ), mLine( line )
		{}

		ErrorMessage() : mMessage( NULL ), mFile( NULL ) {}

		void serialize( EventSerializer& s)
		{
			s.streamify( mCode );
			s.streamify( mMessage );
			s.streamify( mFile );
			s.streamify( mLine );
		}
	};

	struct AddProfileZone : public EventSerializeable
	{
		PxU64 mInstanceId;
		String mName;
		AddProfileZone( PxU64 iid, String nm )
			: mInstanceId( iid ), mName( nm ) {}
		AddProfileZone() : mName( NULL ) {}

		void serialize( EventSerializer& s)
		{
			s.streamify( mInstanceId );
			s.streamify( mName );
		}
	};

	struct AddProfileZoneEvent : public EventSerializeable
	{
		PxU64	mInstanceId;
		String	mName;
		PxU16	mEventId;
		bool	mCompileTimeEnabled;
		AddProfileZoneEvent( PxU64 iid, String nm, PxU16 eid, bool cte )
			: mInstanceId( iid ), mName( nm ), mEventId( eid ), mCompileTimeEnabled( cte )
		{
		}
		AddProfileZoneEvent(){}

		void serialize( EventSerializer& s)
		{
			s.streamify( mInstanceId );
			s.streamify( mName );
			s.streamify( mEventId );
			s.streamify( mCompileTimeEnabled );
		}
	};

	struct StreamEndEvent : public EventSerializeable
	{
		String		 mName;		
		StreamEndEvent() : mName( "StreamEnd" ) {}

		void serialize( EventSerializer& s)
		{
			s.streamify( mName );
		}
	};

	struct OriginShift : public EventSerializeable
	{
		PxU64		 mInstanceId;
		PxVec3		 mShift;

		OriginShift( PxU64 iid, const PxVec3& shift )
			: mInstanceId( iid ), mShift( shift ) {}
		OriginShift(){}

		void serialize( EventSerializer& s)
		{
			s.streamify( mInstanceId );
			s.streamify( mShift );
		}
	};


	template<typename TDataType> struct DatatypeToCommEventType { bool compile_error; };
	template<PvdCommStreamEventTypes::Enum TEnumType> struct CommEventTypeToDatatype { bool compile_error; };

#define DECLARE_PVD_COMM_STREAM_EVENT( x )																			\
	template<> struct DatatypeToCommEventType<x> { enum Enum { EEventTypeMap = PvdCommStreamEventTypes::x }; };	\
	template<> struct CommEventTypeToDatatype<PvdCommStreamEventTypes::x> { typedef x TEventType; };
#define DECLARE_PVD_COMM_STREAM_EVENT_NO_COMMA( x )																			\
template<> struct DatatypeToCommEventType<x> { enum Enum { EEventTypeMap = PvdCommStreamEventTypes::x }; };	\
template<> struct CommEventTypeToDatatype<PvdCommStreamEventTypes::x> { typedef x TEventType; };

#include "PvdCommStreamEventTypes.h"
#undef DECLARE_PVD_COMM_STREAM_EVENT_NO_COMMA 
#undef DECLARE_PVD_COMM_STREAM_EVENT


	template<typename TDataType> PvdCommStreamEventTypes::Enum getCommStreamEventType() 
	{ return static_cast<PvdCommStreamEventTypes::Enum>( DatatypeToCommEventType<TDataType>::EEventTypeMap ); }
	
	template<typename TOperator>
	void visitCommEventType( PvdCommStreamEventTypes::Enum value, TOperator& op )
	{
		switch( value )
		{
#define DECLARE_PVD_COMM_STREAM_EVENT( x ) case PvdCommStreamEventTypes::x: op.handle##x(); return;
#include "PvdCommStreamEventTypes.h"
#undef DECLARE_PVD_COMM_STREAM_EVENT

			default: PX_ASSERT( false );
		}
		PX_ASSERT( false );
	}

	//SDK event types, events sent from PVDUI to the SDK.  PVD takes care of ensuring they
	//are ordered byte-appropriately.
	struct PvdSdkEventTypes
	{
		enum Enum
		{
#define DECLARE_PVD_COMM_STREAM_SDK_EVENT( name ) name
#include "PvdCommStreamSDKEventTypes.h"
#undef DECLARE_PVD_COMM_STREAM_SDK_EVENT
		};
	};

	struct SetPauseStateSdkEvent
	{
		PxU32 mPauseState;
		SetPauseStateSdkEvent( PvdConnectionState::Enum state = PvdConnectionState::eIDLE )
			: mPauseState( static_cast<PxU32>( state ) ) {}

		PvdConnectionState::Enum getConnectionState() { return static_cast<PvdConnectionState::Enum>( mPauseState ); }
		
		template<typename TSerializer>
		void serialize( TSerializer& s)
		{
			s.streamify( mPauseState );
		}
	};


}}}
#endif
