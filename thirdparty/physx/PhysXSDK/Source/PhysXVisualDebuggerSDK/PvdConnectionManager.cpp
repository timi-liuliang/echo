/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
#include "PvdConnectionManager.h"
#include "PvdCommStreamTypes.h"
#include "PvdFoundation.h"
#include "PvdObjectModelMetaData.h"
#include "PxProfileZoneManager.h"
#include "PxProfileMemoryEventTypes.h"
#include "PxProfileMemoryEventRecorder.h"

using namespace physx::debugger;
using namespace physx::debugger::comm;
using namespace physx;

#ifdef PX_VC
#pragma warning (disable : 4371) //layout of class may have changed from a previous version of the compiler due to better packing of member
#endif

namespace {
	struct InstanceData
	{
		Option<bool>		mPickable;
		Option<PvdColor>	mColor;
		Option<bool>		mIsTopLevel;
	};
	struct ConnectionManager : public PvdConnectionManager, public PvdConnectionListener, public PxAllocationListener
	{
		typedef MutexT<ForwardingAllocator>::ScopedLock TScopedLockType;
		typedef void (ConnectionManager::*TAllocationHandler)( size_t size, const char* typeName, const char* filename, int line, void* allocatedMemory );
		typedef void (ConnectionManager::*TDeallocationHandler)( void* allocatedMemory );

		PxAllocatorCallback&						mAllocator;
		PxAllocatorCallback&						mNonBroadcastingAllocator;
		PvdRefPtr<PvdCommStreamInternalConnection>	mConnection;
		//Any changes to the connection's ref count need to mConnection
		//Need to happen with this mutex locked.
		MutexT<ForwardingAllocator>					mMutex;
		StringTable&								mStringTable;

		ForwardingHashMap<const void*,InstanceData>	mInstanceMap;
		ForwardingHashMap<String,SetCamera>		mCameraMap;
		ForwardingArray<PvdConnectionHandler*>	mHandlers;

		PxProfileMemoryEventRecorder*			mRecorder;
		TAllocationHandler						mAllocationHandler;
		TDeallocationHandler					mDeallocationHandler;
		MutexT<ForwardingAllocator>				mAllocationMutex;

		PxProfileZoneManager*					mProfileZoneManager;

		ConnectionManager( PxAllocatorCallback& alloc, PxAllocatorCallback& nonBroadcastingAlloc, bool inRecordMemoryEvents )
			: mAllocator( alloc )
			, mNonBroadcastingAllocator( nonBroadcastingAlloc )
			, mConnection( NULL )
			, mMutex( ForwardingAllocator( alloc, "ConnectionManager::mMutex" ) )
			, mStringTable( StringTable::create( alloc ) )
			, mInstanceMap( alloc, "ConnectionManager::mInstanceMap" )
			, mCameraMap( alloc, "ConnectionManager::mCameraMap" )
			, mHandlers( alloc, "ConnectionManager::mHandlers" )
			, mRecorder( NULL )
			, mAllocationMutex( ForwardingAllocator( alloc, "ConnectionManager::mAllocationMutex" ) )
			, mProfileZoneManager( NULL )
		{
			if ( inRecordMemoryEvents )
			{
				mRecorder = &PxProfileMemoryEventRecorder::createRecorder( &Foundation::getInstance() );
				mAllocationHandler = &ConnectionManager::recordAllocationHandler;
				mDeallocationHandler = &ConnectionManager::recordDeallocationHandler;
			}
			else
			{
				mAllocationHandler = &ConnectionManager::nullAllocationHandler;
				mDeallocationHandler = &ConnectionManager::nullDeallocationHandler;
			}
			Foundation::getInstance().getAllocator().registerAllocationListener( *this );
		}

		virtual ~ConnectionManager()
		{
			Foundation::getInstance().getAllocator().deregisterAllocationListener( *this );
			if ( mRecorder )
			{
				mRecorder->setListener( NULL );
				mRecorder->release();
			}
			disconnect();
			mStringTable.release();
		}

		InstanceData& getInstanceData( const void* instance )
		{
			const ForwardingHashMap<const void*,InstanceData>::Entry* entry( mInstanceMap.find( instance ) );
			if ( entry ) return const_cast<InstanceData&>(entry->second);
			mInstanceMap.insert( instance, InstanceData() );
			return const_cast<InstanceData&>( mInstanceMap.find( instance )->second );
		}
		
		virtual void setProfileZoneManager( physx::PxProfileZoneManager& profileMgr )
		{
			mProfileZoneManager = &profileMgr;
		}
		
		virtual void setPickable( const void* instance, bool pickable )
		{
			{	
				TScopedLockType locker( mMutex );
				getInstanceData( instance ).mPickable = pickable;
			}
			PvdScopedItem<PvdCommStreamInternalConnection> connection(getAndAddRefCurrentConnectionInternal());
			if ( connection ) connection->setPickable( instance, pickable );
		}
		virtual void setColor( const void* instance, const PvdColor& color )
		{
			{
				TScopedLockType locker( mMutex );
				getInstanceData( instance ).mColor = color;
			}
			PvdScopedItem<PvdCommStreamInternalConnection> connection(getAndAddRefCurrentConnectionInternal());
			if ( connection ) connection->setColor( instance, color );
		}
		virtual void setIsTopLevelUIElement( const void* instance, bool isTopLevel )
		{
			{
				TScopedLockType locker( mMutex );
				getInstanceData( instance ).mIsTopLevel = isTopLevel;
			}
			PvdScopedItem<PvdCommStreamInternalConnection> connection(getAndAddRefCurrentConnectionInternal());
			if ( connection ) connection->setIsTopLevelUIElement( instance, isTopLevel );
		}
		virtual void setCamera( String name, const PxVec3& position, const PxVec3& up, const PxVec3& target )
		{
			{
				TScopedLockType locker( mMutex );
				const ForwardingHashMap<String,SetCamera>::Entry* entry = mCameraMap.find( name );
				if ( entry == NULL )
				{
					name = mStringTable.registerStr( name );
					mCameraMap.insert( name, SetCamera( name, position, up, target ) );
					entry = mCameraMap.find( name );
				}
				SetCamera& cam = const_cast<SetCamera&>( entry->second );
				cam = SetCamera( cam.mName, position, up, target );
			}
			PvdScopedItem<PvdCommStreamInternalConnection> connection(getAndAddRefCurrentConnectionInternal());
			if ( connection ) connection->setCamera( name, position, up, target );
		}

		//Send error message to PVD
		virtual void sendErrorMessage(PxErrorCode::Enum code, String message, String file, PxU32 line)
		{
			PvdScopedItem<PvdCommStreamInternalConnection> connection(getAndAddRefCurrentConnectionInternal());
			if ( connection )
			{
				connection->sendErrorMessage( code, message, file, line );
				connection->flush();
			}
		}

		virtual void sendStreamEnd()
		{
			PvdScopedItem<PvdCommStreamInternalConnection> connection(getAndAddRefCurrentConnectionInternal());
			if ( connection ) connection->sendStreamEnd();
		}

		/**
		 *	Handler will be notified every time there is a new connection.
		 */
		virtual void addHandler( PvdConnectionHandler& inHandler )
		{
			TScopedLockType locker( mMutex );
			mHandlers.pushBack( &inHandler );
			PvdScopedItem<PvdCommStreamInternalConnection> connection(getAndAddRefCurrentConnectionInternal());
			if ( connection ) 
			{
				inHandler.onPvdSendClassDescriptions( *connection );
				inHandler.onPvdConnected( *connection );
			}
		}
		/**
		 *	Handler will be notified when a connection is destroyed.
		 */
		virtual void removeHandler( PvdConnectionHandler& inHandler )
		{
			TScopedLockType locker( mMutex );
			PVD_FOREACH( idx, mHandlers.size() )
			{
				if ( mHandlers[idx] == &inHandler )
				{
					mHandlers.replaceWithLast( idx );
					break;
				}
			}
		}

		virtual PvdConnection* connectAddRef( PvdNetworkInStream* inInStream
												, PvdNetworkOutStream& inOutStream
												, TConnectionFlagsType inConnectionType
												, bool doubleBuffered )
		{
			TScopedLockType __locker( mMutex );
			internalDisconnect();
			mConnection = &PvdCommStreamInternalConnection::create( mAllocator, mNonBroadcastingAllocator, inOutStream, inInStream, inConnectionType, *this, mAllocationMutex, doubleBuffered );
			PvdRefPtr<PvdCommStreamInternalConnection> connection(mConnection.mObj);
			connection->sendStreamInitialization();

			bool isMemoryConnection = inConnectionType & PvdConnectionType::eMEMORY;
			if ( isMemoryConnection )
			{
				TScopedLockType lock( mAllocationMutex );
				if ( mRecorder != NULL )
					mRecorder->setListener( connection );
				else
				{
					mAllocationHandler = &ConnectionManager::connectionAllocationHandler;
					mDeallocationHandler = &ConnectionManager::connectionDeallocationHandler;
				}
			}

			if ( mProfileZoneManager && inConnectionType & PvdConnectionType::ePROFILE )
				mProfileZoneManager->addProfileZoneHandler( *connection );

			PxI32 callbackIdx = getFoundation().getErrorHandler().registerErrorCallback( *connection );
			if( callbackIdx < 0 )
				getFoundation().error( PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "Failed to register pvd error handler to foundation." );
			
			for ( ForwardingHashMap<const void*,InstanceData>::Iterator iter = mInstanceMap.getIterator();
				iter.done() == false; ++iter )
			{
				if ( iter->second.mColor.hasValue() )
					connection->setColor( iter->first, iter->second.mColor );
				if ( iter->second.mPickable.hasValue() )
					connection->setPickable( iter->first, iter->second.mPickable );
				if ( iter->second.mIsTopLevel.hasValue() )
					connection->setIsTopLevelUIElement( iter->first, iter->second.mIsTopLevel );
			}

			for ( ForwardingHashMap<String,SetCamera>::Iterator iter = mCameraMap.getIterator();
				iter.done() == false; ++iter )
				connection->setCamera( iter->second.mName, iter->second.mPosition, iter->second.mUp, iter->second.mTarget );

			PVD_FOREACH( idx, mHandlers.size() )
				mHandlers[idx]->onPvdSendClassDescriptions( *connection );
			//The crazy thing is that one of these handlers could call disconnect.
			//or inadvertently cause it to be called.
			//Thus we need to use the local variable and not the mConnection variable.
			PVD_FOREACH( idx, mHandlers.size() )
				mHandlers[idx]->onPvdConnected( *connection );

			mConnection.addRef();
			return mConnection.mObj;
		}

		PvdCommStreamInternalConnection* getAndAddRefCurrentConnectionInternal()
		{
			TScopedLockType __locker( mMutex );
			if ( mConnection.mObj != NULL )
				mConnection.mObj->addRef();
			return mConnection.mObj;
		}

		/**
		 *	Return the factory representing the current connection to PVD, if any.
		 */
		virtual PvdConnection* getAndAddRefCurrentConnection() 
		{ 
			return getAndAddRefCurrentConnectionInternal();
		}

		virtual bool isConnected() 
		{ 
			TScopedLockType __locker( mMutex );
			return mConnection.mObj != NULL && mConnection.mObj->isConnected(); 
		}

		//Should only be called with mMutex held.
		void internalDisconnect()
		{
			if ( mConnection ) mConnection->disconnect();
		}

		/**
		 *	If there is a current connection, disconnect from the factory.
		 */
		virtual void disconnect() 
		{
			TScopedLockType __locker( mMutex );
			internalDisconnect();
		}

		virtual void release() { PVD_DELETE( mAllocator, this ); }

		
		virtual void onDisconnect( PvdCommStreamInternalConnection& )
		{
			if ( mConnection == NULL )
			{
				return;
			}
			
			TScopedLockType locker( mMutex );
			if ( mConnection )
			{
				if ( mRecorder )
					mRecorder->setListener( NULL );
				else
				{
					mAllocationHandler = &ConnectionManager::nullAllocationHandler;
					mDeallocationHandler = &ConnectionManager::nullDeallocationHandler;
				}
				PvdRefPtr<PvdCommStreamInternalConnection> connection(mConnection.mObj);
				mConnection = NULL;
				if ( mProfileZoneManager )
					mProfileZoneManager->removeProfileZoneHandler( *connection );

				getFoundation().getErrorHandler().unRegisterErrorCallback( *connection );

				PVD_FOREACH( idx, mHandlers.size() ) mHandlers[idx]->onPvdDisconnected( *connection );
			}
		}
		virtual void onInstanceDestroyed( const void* instance )
		{
			TScopedLockType locker( mMutex );
			mInstanceMap.erase( instance );
		}
		
		virtual void onAllocation( size_t size, const char* typeName, const char* filename, int line, void* allocatedMemory )
		{
			(this->*mAllocationHandler)( size, typeName, filename, line, allocatedMemory );	
		}
		virtual void onDeallocation( void* addr )
		{
			(this->*mDeallocationHandler)(addr);
		}
		
		void recordAllocationHandler( size_t size, const char* typeName, const char* filename, int line, void* allocatedMemory )
		{
			TScopedLockType lock( mAllocationMutex );
			mRecorder->onAllocation( size, typeName, filename, line, allocatedMemory );
		}

		void recordDeallocationHandler( void* addr )
		{
			TScopedLockType lock( mAllocationMutex );
			mRecorder->onDeallocation( addr );
		}

		void connectionAllocationHandler( size_t size, const char* typeName, const char* filename, int line, void* allocatedMemory )
		{
			TScopedLockType lock( mAllocationMutex );
			mConnection->onAllocation( size, typeName, filename, line, allocatedMemory );
		}
		
		void connectionDeallocationHandler( void* addr )
		{
			TScopedLockType lock( mAllocationMutex );
			mConnection->onDeallocation( addr );
		}
		
		void nullAllocationHandler( size_t , const char* , const char* , int , void* )
		{
		}

		void nullDeallocationHandler( void* )
		{
		}

	private:
		ConnectionManager& operator=(const ConnectionManager&);
		ConnectionManager(const ConnectionManager &);
	};
}

PvdConnectionManager& PvdConnectionManager::create( PxAllocatorCallback& allocator, PxAllocatorCallback& nonBroadcastingAlloc, bool trackMemoryEvents )
{
	return *PVD_NEW( allocator, ConnectionManager )( allocator, nonBroadcastingAlloc, trackMemoryEvents );
}
