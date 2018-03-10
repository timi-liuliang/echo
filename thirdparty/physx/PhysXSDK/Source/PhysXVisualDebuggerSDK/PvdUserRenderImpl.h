/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
#ifndef PVD_IMMEDIATE_RENDER_IMPL_H
#define PVD_IMMEDIATE_RENDER_IMPL_H
#include "PvdUserRenderer.h"


namespace physx { namespace debugger { namespace renderer {


	struct PvdUserRenderTypes
	{
		enum Enum
		{
			Unknown = 0,
#define DECLARE_PVD_IMMEDIATE_RENDER_TYPE( type ) type,
#define DECLARE_PVD_IMMEDIATE_RENDER_TYPE_NO_COMMA( type ) type
#include "PvdUserRenderTypes.h"
#undef DECLARE_PVD_IMMEDIATE_RENDER_TYPE_NO_COMMA 
#undef DECLARE_PVD_IMMEDIATE_RENDER_TYPE
		};
	};
	
	class RenderSerializer
	{
	protected:
		virtual ~RenderSerializer(){}
	public:
		virtual void streamify( PxU64& val ) = 0;
		virtual void streamify( PxF32& val ) = 0;
		virtual void streamify( PxU8& val ) = 0;
		virtual void streamify( DataRef<PxU8>& val ) = 0;
		virtual void streamify( DataRef<PvdPoint>& val ) = 0;
		virtual void streamify( DataRef<PvdLine>& val ) = 0;
		virtual void streamify( DataRef<PvdTriangle>& val ) = 0;
		virtual bool isGood() = 0;
		virtual PxU32 hasData() = 0;

		void streamify( PvdUserRenderTypes::Enum& val )
		{
			PxU8 data = static_cast<PxU8>( val );
			streamify( data );
			val = static_cast<PvdUserRenderTypes::Enum>( data );
		}
		void streamify( PxVec3& val )
		{
			streamify( val[0] );
			streamify( val[1] );
			streamify( val[2] );
		}

		void streamify( PvdColor& val )
		{
			streamify( val.r );
			streamify( val.g );
			streamify( val.b );
			streamify( val.a );
		}
		void streamify( PxTransform& val )
		{
			streamify( val.q.x );
			streamify( val.q.y );
			streamify( val.q.z );
			streamify( val.q.w );
			streamify( val.p.x );
			streamify( val.p.y );
			streamify( val.p.z );
		}
		void streamify( bool& val )
		{
			PxU8 tempVal = PxU8(val ? 1 : 0);
			streamify( tempVal );
			val = tempVal ? true : false;
		}
	};

	template<typename TBulkRenderType>
	struct BulkRenderEvent
	{
		DataRef<TBulkRenderType>	mData;
		BulkRenderEvent( const TBulkRenderType* data, PxU32 count )
			: mData( data, count )
		{
		}
		BulkRenderEvent(){}
		void serialize( RenderSerializer& serializer )
		{
			serializer.streamify( mData );
		}
	};
	struct SetInstanceIdRenderEvent
	{
		PxU64 mInstanceId;
		SetInstanceIdRenderEvent( PxU64 iid ) : mInstanceId( iid ) {}
		SetInstanceIdRenderEvent(){}
		void serialize( RenderSerializer& serializer )
		{
			serializer.streamify( mInstanceId );
		}
	};
	struct PointsRenderEvent : BulkRenderEvent<PvdPoint>
	{
		PointsRenderEvent( const PvdPoint* data, PxU32 count ) : BulkRenderEvent<PvdPoint>( data, count ) {}
		PointsRenderEvent(){}
	};
	struct LinesRenderEvent : BulkRenderEvent<PvdLine>
	{
		LinesRenderEvent( const PvdLine* data, PxU32 count ) : BulkRenderEvent<PvdLine>( data, count ) {}
		LinesRenderEvent(){}
	};
	struct TrianglesRenderEvent : BulkRenderEvent<PvdTriangle>
	{
		TrianglesRenderEvent( const PvdTriangle* data, PxU32 count ) : BulkRenderEvent<PvdTriangle>( data, count ) {}
		TrianglesRenderEvent(){}
	};

	struct TextRenderEvent 
	{
		DataRef<PxU8> mData;
		PxVec3 mPos;
        PvdColor mColor;
		TextRenderEvent( const char* text, PxVec3 pos, PvdColor color ) : mData((const PxU8*)text,PxU32(strlen(text))+1),mPos(pos),mColor(color) {}
		TextRenderEvent(){}
		void serialize( RenderSerializer& serializer )
		{
			serializer.streamify( mData );
			serializer.streamify( mPos );
			serializer.streamify( mColor );
		}
	};
	
	struct JointFramesRenderEvent
	{
		PxTransform parent;
		PxTransform child;
		JointFramesRenderEvent( const PxTransform& p, const PxTransform& c) : parent( p ), child( c ) {}
		JointFramesRenderEvent(){}
		void serialize( RenderSerializer& serializer )
		{
			serializer.streamify( parent );
			serializer.streamify( child );
		}
	};
	struct LinearLimitRenderEvent
	{
		PxTransform t0;
		PxTransform t1;
		PxF32 value;
		bool active;
		LinearLimitRenderEvent( const PxTransform& _t0, const PxTransform& _t1, PxF32 _value, bool _active )
			: t0( _t0 ), t1( _t1 ), value( _value ), active( _active ) {}
		LinearLimitRenderEvent(){}
		void serialize( RenderSerializer& serializer )
		{
			serializer.streamify( t0 );
			serializer.streamify( t1 );
			serializer.streamify( value );
			serializer.streamify( active );
		}
	};
	struct AngularLimitRenderEvent
	{
		PxTransform t0;
		PxF32 lower;
		PxF32 upper;
		bool active;
		AngularLimitRenderEvent( const PxTransform& _t0, PxF32 _lower, PxF32 _upper, bool _active )
			: t0( _t0 ), lower( _lower ), upper( _upper ), active( _active ) {}
		AngularLimitRenderEvent(){}
		void serialize( RenderSerializer& serializer )
		{
			serializer.streamify( t0 );
			serializer.streamify( lower );
			serializer.streamify( upper );
			serializer.streamify( active );
		}
	};
	struct LimitConeRenderEvent
	{
		PxTransform t;
		PxF32 ySwing;
		PxF32 zSwing;
		bool active;
		LimitConeRenderEvent( const PxTransform& _t, PxF32 _ySwing, PxF32 _zSwing, bool _active )
			: t( _t ), ySwing( _ySwing ), zSwing( _zSwing ), active( _active ) {}
		LimitConeRenderEvent(){}
		void serialize( RenderSerializer& serializer )
		{
			serializer.streamify( t );
			serializer.streamify( ySwing );
			serializer.streamify( zSwing );
			serializer.streamify( active );
		}
	};
	struct DoubleConeRenderEvent
	{
		PxTransform t;
		PxF32 angle;
		bool active;
		DoubleConeRenderEvent( const PxTransform& _t, PxF32 _angle, bool _active )
			: t( _t ), angle( _angle ), active( _active ) {}
		DoubleConeRenderEvent(){}
		void serialize( RenderSerializer& serializer )
		{
			serializer.streamify( t );
			serializer.streamify( angle );
			serializer.streamify( active );
		}
	};

	template<typename TDataType>
	struct RenderSerializerMap
	{
		void serialize( RenderSerializer& s, TDataType& d )
		{
			d.serialize( s );
		}
	};
	template<> struct RenderSerializerMap<PxU8>
	{
		void serialize( RenderSerializer& s, PxU8& d )
		{
			s.streamify( d );			
		}
	};
	template<> struct RenderSerializerMap<PvdPoint>
	{
		void serialize( RenderSerializer& s, PvdPoint& d )
		{
			s.streamify( d.pos );
			s.streamify( d.color );
		}
	};
	template<> struct RenderSerializerMap<PvdLine>
	{
		void serialize( RenderSerializer& s, PvdLine& d )
		{
			s.streamify( d.pos0 );
			s.streamify( d.color0 );
			s.streamify( d.pos1 );
			s.streamify( d.color1 );
		}
	};
	template<> struct RenderSerializerMap<PvdTriangle>
	{
		void serialize( RenderSerializer& s, PvdTriangle& d )
		{
			s.streamify( d.pos0 );
			s.streamify( d.color0 );
			s.streamify( d.pos1 );
			s.streamify( d.color1 );
			s.streamify( d.pos2 );
			s.streamify( d.color2 );
		}
	};

	template<typename TDataType>
	struct PvdTypeToRenderType
	{
		bool compile_error;
	};

#define DECLARE_PVD_IMMEDIATE_RENDER_TYPE( type ) \
	template<> struct PvdTypeToRenderType<type##RenderEvent> { enum Enum { EnumVal = PvdUserRenderTypes::type }; };

#include "PvdUserRenderTypes.h"
#undef DECLARE_PVD_IMMEDIATE_RENDER_TYPE

	template<typename TDataType>
	PvdUserRenderTypes::Enum getPvdRenderTypeFromType() 
	{ return static_cast<PvdUserRenderTypes::Enum>( PvdTypeToRenderType<TDataType>::EnumVal ); }

	class PvdUserRenderHandler
	{
	protected:
		virtual ~PvdUserRenderHandler(){}

	public:
#define DECLARE_PVD_IMMEDIATE_RENDER_TYPE( type ) \
	virtual void handleRenderEvent( const type##RenderEvent& evt ) = 0;

#include "PvdUserRenderTypes.h"
#undef DECLARE_PVD_IMMEDIATE_RENDER_TYPE
	};

	class PvdUserRenderParser
	{
	protected:
		virtual ~PvdUserRenderParser(){}
	public:
		virtual void release() = 0;
		virtual void parseData( DataRef<const PxU8> data, PvdUserRenderHandler& handler ) = 0;

		static PvdUserRenderParser& create( PxAllocatorCallback& alloc, bool swapBytes );
	};

}}}


#endif
