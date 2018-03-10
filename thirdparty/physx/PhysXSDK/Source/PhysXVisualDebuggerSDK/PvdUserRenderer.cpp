/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
#include "PvdUserRenderer.h"
#include "PvdUserRenderImpl.h"
#include "PvdFoundation.h"
#include "PvdInternalByteStreams.h"
#include "PvdBits.h"

using namespace physx;
using namespace physx::profile; 
using namespace physx::debugger;
using namespace physx::debugger::renderer;


namespace {

	template<typename TStreamType>
	struct RenderWriter : public RenderSerializer
	{
		TStreamType& mStream;
		RenderWriter( TStreamType& stream )
			: mStream( stream ) {}
		template<typename TDataType>
		void write( const TDataType* val, PxU32 count )
		{
			PxU32 numBytes = count * sizeof( TDataType );
			mStream.write( reinterpret_cast<const PxU8*>( val ), numBytes );
		}
		template<typename TDataType>
		void write( const TDataType& val ) { write( &val, 1 ); }

		template<typename TDataType>
		void writeRef( DataRef<TDataType>& val )
		{
			PxU32 amount = val.size();
			write( amount );
			if ( amount )
				write( val.begin(), amount );
		}
		
		virtual void streamify( PxU64& val ) { write( val ); }
		virtual void streamify( PxF32& val ) { write( val ); }
		virtual void streamify( PxU8& val )  { write( val ); }
        virtual void streamify( DataRef<PxU8>& val ) { writeRef( val ); }
		virtual void streamify( DataRef<PvdPoint>& val ) { writeRef( val ); }
		virtual void streamify( DataRef<PvdLine>& val ) { writeRef( val ); }
		virtual void streamify( DataRef<PvdTriangle>& val ) { writeRef( val ); }
		virtual PxU32 hasData() { return false; }
		virtual bool isGood() { return true; }

	private:
		RenderWriter& operator=(const RenderWriter&);
		
	};

	struct UserRenderer : public PvdUserRenderer
	{

		PxAllocatorCallback& mAllocator;
		ForwardingMemoryBuffer mBuffer;
		PxU32 mBufferCapacity;
		PxU32 mRefCount;
		ForwardingArray<PxProfileEventBufferClient*> mClients;
		UserRenderer( PxAllocatorCallback& alloc, PxU32 bufferFullAmount )
			: mAllocator( alloc )
			, mBuffer( alloc, "UserRenderBuffer" )
			, mBufferCapacity( bufferFullAmount )
			, mRefCount( 0 )
			, mClients( alloc, "PvdUserRenderer::Clients" )
		{
		}
		virtual ~UserRenderer()
		{
			PVD_FOREACH( idx, mClients.size() )
				mClients[idx]->handleClientRemoved();
			mClients.clear();
		}
		virtual void addRef() { ++mRefCount; }
		virtual void release() 
		{ 
			if ( mRefCount ) --mRefCount;
			if ( !mRefCount ) PVD_DELETE( mAllocator, this ); 
		}

		template<typename TEventType>
		void handleEvent( TEventType evt )
		{
			RenderWriter<ForwardingMemoryBuffer> _writer( mBuffer );
			RenderSerializer& writer( _writer );

			PvdUserRenderTypes::Enum evtType( getPvdRenderTypeFromType<TEventType>() );
			writer.streamify( evtType );
			evt.serialize( writer );
			if ( mBuffer.size() >= mBufferCapacity )
				flushRenderEvents();
		}
		virtual void setInstanceId( const void* iid )
		{
			handleEvent( SetInstanceIdRenderEvent( PVD_POINTER_TO_U64( iid ) ) );
		}
		//Draw these points associated with this instance
		virtual void drawPoints( const PvdPoint* points, PxU32 count )
		{
			handleEvent( PointsRenderEvent( points, count ) );
		}
		//Draw these lines associated with this instance
		virtual void drawLines( const PvdLine* lines, PxU32 count )
		{
			handleEvent( LinesRenderEvent( lines, count ) );
		}
		//Draw these triangles associated with this instance
		virtual void drawTriangles( const PvdTriangle* triangles, PxU32 count )
		{
			handleEvent( TrianglesRenderEvent( triangles, count ) );
		}

		virtual void drawText( PxVec3 pos, PvdColor color, const char* text, ...)
		{
			va_list va;
			va_start(va, text);
			const int bufSize = 1024;
		    static char stringBuffer[bufSize+1];

		    vsnprintf(stringBuffer, (size_t)bufSize, text, va);
		    stringBuffer[bufSize] = 0; // make sure there is a null termination character on all platforms

			handleEvent( TextRenderEvent( stringBuffer, pos, color ) );

			va_end(va);
		}
	
		//Constraint visualization routines
		virtual void visualizeJointFrames( const PxTransform& parent, const PxTransform& child )
		{
			handleEvent( JointFramesRenderEvent( parent, child ) );
		}
		virtual void visualizeLinearLimit( const PxTransform& t0, const PxTransform& t1, PxF32 value, bool active )
		{
			handleEvent( LinearLimitRenderEvent( t0, t1, value, active ) );
		}
		virtual void visualizeAngularLimit( const PxTransform& t0, PxF32 lower, PxF32 upper, bool active)
		{
			handleEvent( AngularLimitRenderEvent( t0, lower, upper, active ) );
		}
		virtual void visualizeLimitCone( const PxTransform& t, PxF32 ySwing, PxF32 zSwing, bool active)
		{
			handleEvent( LimitConeRenderEvent( t, ySwing, zSwing, active ) );
		}
		virtual void visualizeDoubleCone( const PxTransform& t, PxF32 angle, bool active)
		{
			handleEvent( DoubleConeRenderEvent( t, angle, active ) );
		}
		//Clear the immedate buffer.
		virtual void flushRenderEvents()
		{
			if ( mClients.size() )
			{
				PVD_FOREACH( idx, mClients.size() )
					mClients[idx]->handleBufferFlush( mBuffer.begin(), mBuffer.size() );
			}
			mBuffer.clear();
		}
		virtual void addClient( PxProfileEventBufferClient& inClient ) 
		{
			mClients.pushBack( &inClient );
		}
		virtual void removeClient( PxProfileEventBufferClient& inClient ) 
		{
			PVD_FOREACH( idx, mClients.size() )
			{
				if ( mClients[idx] == &inClient )
				{
					inClient.handleClientRemoved();
					mClients.replaceWithLast( idx );
					break;
				}
			}
		}
		virtual bool hasClients() const
		{
			return mClients.size() > 0;
		}
	private:
		UserRenderer& operator=(const UserRenderer&);
	};

	
	template<bool swapBytes>
	struct RenderReader : public RenderSerializer
	{
		MemPvdInputStream			mStream;
		ForwardingMemoryBuffer&		mBuffer;

		
		RenderReader( ForwardingMemoryBuffer& buf ) : mBuffer ( buf ) {}
		void setData( DataRef<const PxU8> data )
		{
			mStream.setup( const_cast<PxU8*>( data.begin() ), const_cast<PxU8*>( data.end() ) );
		}
		virtual void streamify( PxU64& val ) { mStream >> val; }
		virtual void streamify( PxF32& val ) { mStream >> val; }
		virtual void streamify( PxU8& val )  { mStream >> val; }
		template<typename TDataType>
		void readRef( DataRef<TDataType>& val )
		{
			PxU32 count;
			mStream >> count;
			PxU32 numBytes = sizeof( TDataType ) * count;
			
			TDataType* dataPtr = reinterpret_cast<TDataType*>( mBuffer.growBuf(numBytes) );
			mStream.read( reinterpret_cast<PxU8*>(dataPtr), numBytes );
			val = DataRef<TDataType>( dataPtr, count );
		}
		virtual void streamify( DataRef<PvdPoint>& val ) 
		{
			readRef( val );
		}
		virtual void streamify( DataRef<PvdLine>& val )
		{
			readRef( val );
		}
		virtual void streamify( DataRef<PvdTriangle>& val )
		{
			readRef( val );
		}
		virtual void streamify( DataRef<PxU8>& val ) 
		{
			readRef( val );
		}
		virtual bool isGood() { return mStream.isGood(); }
		virtual PxU32 hasData() { return PxU32(mStream.size() > 0); }
	private:
		RenderReader& operator=(const RenderReader&);
	};

	template<> struct RenderReader<true> : public RenderSerializer
	{
		MemPvdInputStream			mStream;
		ForwardingMemoryBuffer&		mBuffer;
		RenderReader( ForwardingMemoryBuffer& buf ) : mBuffer ( buf ) {}
		void setData( DataRef<const PxU8> data )
		{
			mStream.setup( const_cast<PxU8*>( data.begin() ), const_cast<PxU8*>( data.end() ) );
		}

		template<typename TDataType>
		void read( TDataType& val ) { mStream >> val; swapBytes( val ); }
		virtual void streamify( PxU64& val ) { read( val ); }
		virtual void streamify( PxF32& val ) { read( val ); }
		virtual void streamify( PxU8& val )  { read( val ); }
		template<typename TDataType>
		void readRef( DataRef<TDataType>& val )
		{
			PxU32 count;
			mStream >> count;
			swapBytes( count );
			PxU32 numBytes = sizeof( TDataType ) * count;
			
			TDataType* dataPtr = reinterpret_cast<TDataType*>( mBuffer.growBuf(numBytes) );
			PVD_FOREACH( idx, count )
				RenderSerializerMap<TDataType>().serialize( *this, dataPtr[idx] );
			val = DataRef<TDataType>( dataPtr, count );
		}
		virtual void streamify( DataRef<PvdPoint>& val ) 
		{
			readRef( val );
		}
		virtual void streamify( DataRef<PvdLine>& val )
		{
			readRef( val );
		}
		virtual void streamify( DataRef<PvdTriangle>& val )
		{
			readRef( val );
		}
		virtual void streamify( DataRef<PxU8>& val ) 
		{
			readRef( val );
		}
		virtual bool isGood() { return mStream.isGood(); }
		virtual PxU32 hasData() { return PxU32(mStream.size() > 0); }
	private:
		RenderReader& operator=(const RenderReader&);
	};

	template<bool swapBytes>
	struct Parser : public PvdUserRenderParser
	{
		PxAllocatorCallback&	mAllocator;
		ForwardingMemoryBuffer	mBuffer;
		RenderReader<swapBytes> mReader;
		Parser( PxAllocatorCallback& alloc )
			: mAllocator( alloc )
			, mBuffer( alloc, "PvdUserRenderParser::mBuffer" )
			, mReader( mBuffer )
		{
		}
		
		void release() { PVD_DELETE( mAllocator, this ); }
		void parseData( DataRef<const PxU8> data, PvdUserRenderHandler& handler ) 
		{
			mReader.setData( data );
			RenderSerializer& serializer( mReader );
			while( serializer.isGood() && serializer.hasData() )
			{
				mReader.mBuffer.clear();
				PvdUserRenderTypes::Enum evtType = PvdUserRenderTypes::Unknown;
				serializer.streamify( evtType );
				switch( evtType )
				{
	#define DECLARE_PVD_IMMEDIATE_RENDER_TYPE( type )		\
				case PvdUserRenderTypes::type:         \
					{										\
						type##RenderEvent evt;				\
						evt.serialize( serializer );		\
						handler.handleRenderEvent( evt );	\
					}										\
					break;
	#include "PvdUserRenderTypes.h"
	#undef DECLARE_PVD_IMMEDIATE_RENDER_TYPE
				case PvdUserRenderTypes::Unknown:
				default: PX_ASSERT( false ); return;

				}
			}
			PX_ASSERT( serializer.isGood() );
			return;
		}

		PX_NOCOPY(Parser<swapBytes>)
	};
}

PvdUserRenderer& PvdUserRenderer::create( PxAllocatorCallback& alloc, PxU32 bufferSize )
{
	return *PVD_NEW( alloc, UserRenderer)( alloc, bufferSize ) ;
}
PvdUserRenderParser& PvdUserRenderParser::create( PxAllocatorCallback& alloc, bool swapBytes )
{
	if ( swapBytes ) return *PVD_NEW( alloc, Parser<true> )( alloc );
	else return *PVD_NEW( alloc, Parser<false> )( alloc );
}

