/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "PsShare.h"
#ifndef WITHOUT_PVD

#include "NxApexDefs.h"
#include "NiApexSDK.h"
#if NX_SDK_VERSION_MAJOR == 2

#include "PVD28Binding.h"

#include "NxRemoteDebugger.h"
#include "PVDBindingErrorStream.h"
#include "PvdDataStream.h"
#include "PvdErrorCodes.h"
#include "PvdConnectionManager.h"
#include "PsUserAllocated.h"
#include "PxProfileZoneManager.h"
#include "PxProfileMemoryBuffer.h"
#include "PxProfileFoundationWrapper.h"

using namespace PVD;
using namespace physx;
using namespace physx::debugger;
using namespace physx::debugger::comm;
using namespace physx::debugger::renderer;
using namespace physx::profile;
using namespace physx::shdfnd;

// Note: This is only created if foundation creation was requested
struct SLocalFoundation : public PxAllocatorCallback
{
	PVDBindingErrorStream mErrorStream;
	SLocalFoundation()
	{
		using namespace physx::shdfnd;
		PxCreateFoundation(PX_PHYSICS_VERSION, *this, mErrorStream);
	}
	~SLocalFoundation()
	{
		using namespace physx::shdfnd;
		Foundation::destroyInstance();
	}

	virtual void* allocate(size_t size, const char* typeName, const char* filename, int line)
	{
		PX_UNUSED(filename);
		PX_UNUSED(line);
		PX_UNUSED(typeName);
		return PX_ALLOC(size, typeName);
	}

	virtual void deallocate(void* ptr)
	{
		PX_FREE(ptr);
	}
};

template<physx::PxU32 CacheSize = 0x2000>
class CPVDRDOutStream : public PvdNetworkOutStream
{
	physx::profile::FoundationWrapper mWrapper;
	NxRemoteDebugger* mDebugger;
	physx::profile::MemoryBuffer<physx::profile::WrapperReflectionAllocator<PxU8> > mTempStorage;
	PxU32		mRefCount;

public:
	CPVDRDOutStream(){}
	CPVDRDOutStream( NxRemoteDebugger* inDebugger )
		: mWrapper( Foundation::getInstance().getAllocatorCallback() ) 
		, mDebugger( inDebugger )
		, mTempStorage( physx::profile::WrapperReflectionAllocator<PxU8>( mWrapper ) )
	{
		if ( mDebugger->isConnected() )
		{
			mDebugger->createObject(this, NX_DBG_OBJECTTYPE_GENERIC, "PVDCommLayerTunnel", NX_DBG_EVENTGROUP_BASIC_OBJECTS);
		}
		performInitialWrite();
	}
	void performInitialWrite()
	{
		mTempStorage.clear();
		PxU32 theWord = 0;
		mTempStorage.write( theWord );
	}
	virtual PvdError write( const physx::PxU8* inBytes, physx::PxU32 inLength )
	{
		if ( !isConnected() )
			return PvdErrorType::NetworkError;

		while ( inLength )
		{
			physx::PxU32 amountToWrite = physx::PxMin( CacheSize - mTempStorage.size(), inLength );
			mTempStorage.write( inBytes, amountToWrite );
			inLength -= amountToWrite;
			inBytes += amountToWrite;
			if ( mTempStorage.size() == CacheSize )
				FlushData();
		}
		return PvdErrorType::Success;
	}
	virtual PvdError flush() 
	{ 
		if ( mDebugger == NULL ) return PvdErrorType::NetworkError;
		FlushData(); return PvdErrorType::Success;
	}
	virtual PxU64 getWrittenDataSize()
	{
		return 0;
	}
	virtual bool isConnected() const { if ( mDebugger ) return mDebugger->isConnected(); return false; }
	virtual void release() { mDebugger = NULL; }
	virtual void disconnect() { mDebugger = NULL; }

	inline void FlushData()
	{
		if ( mDebugger == NULL ) return;

		physx::PxU32 messageSize( static_cast<physx::PxU32>(mTempStorage.size() ) );
		physx::PxU8* theLen = reinterpret_cast< physx::PxU8* >( &messageSize );
		physx::PxU8* theWriteBuffer = mTempStorage.begin();

		theWriteBuffer[0] = theLen[0];
		theWriteBuffer[1] = theLen[1];
		theWriteBuffer[2] = theLen[2];
		theWriteBuffer[3] = theLen[3];
		mDebugger->writeParameter(theWriteBuffer, this, true, "Tunnel", NX_DBG_EVENTGROUP_BASIC_OBJECTS_DYNAMIC_DATA);
		performInitialWrite();
	}
};


class Pvd28BindingImpl : public NxRemoteDebuggerEventListener, public Pvd28Binding, public UserAllocated
{
	 Pvd28BindingImpl& operator=(const Pvd28BindingImpl&);

	typedef CPVDRDOutStream<>	TOutStreamType;
	SLocalFoundation*			mFoundation;
	NxRemoteDebugger*			mDebugger;
	PvdBinding&					mBinding;
	TConnectionFlagsType		mNextConnectionType;
	TOutStreamType*				mOutStream;
	FoundationWrapper			mWrapper;

protected:
	
	virtual ~Pvd28BindingImpl()
	{	
		mBinding.release();
		releaseOutStream();
	}

	PxAllocatorCallback& getAllocator()
	{
		return mWrapper.getAllocator();
	}

	void releaseOutStream()
	{
		if ( mOutStream )
		{
			PX_PROFILE_DELETE( getAllocator(), mOutStream );
			mOutStream = NULL;
		}
	}

public:

	Pvd28BindingImpl(NxRemoteDebugger* inDebugger, bool inSetupFoundation)
		: mFoundation( inSetupFoundation ? new SLocalFoundation() : NULL )
		, mDebugger( inDebugger )
		, mBinding( *PvdBinding::create( true ) )
		, mNextConnectionType( defaultConnectionFlags() )
		, mOutStream( NULL )
		, mWrapper( Foundation::getInstance().getAllocatorCallback() )
	{
		mDebugger->registerEventListener( this );
	}

	virtual bool isConnected() { return mBinding.isConnected() != NULL; }
	virtual TConnectionFlagsType getConnectionType() { return mBinding.getConnectionType(); }
	virtual PvdConnectionManager& getConnectionManager() { return mBinding.getConnectionManager(); }
	virtual PxProfileZoneManager& getProfileManager() { return mBinding.getProfileManager(); }
	virtual PvdDataStream* getDataStream(){ return mBinding.getDataStream(); }
	virtual PvdUserRenderer* getRenderer() { return mBinding.getRenderer(); }
	virtual void initPvdClasses() { mBinding.initPvdClasses(); }
	virtual void initPvdInstances() { mBinding.initPvdInstances(); }
	virtual void initPvdClasses(const NxParameterized::Definition& paramsHandle, const char* pvdClassName)
	{
		mBinding.initPvdClasses(paramsHandle, pvdClassName);
	}
	virtual void updatePvd(const void* pvdInstance, NxParameterized::Interface& params, PvdAction::Enum pvdAction = PvdAction::UPDATE)
	{
		mBinding.updatePvd(pvdInstance, params, pvdAction);
	}
	virtual void lock() { mBinding.lock(); }
	virtual void unlock() { mBinding.unlock(); }
	virtual void beginFrame( void* inInstanceId ) { mBinding.beginFrame( inInstanceId ); }
	virtual void endFrame( void* inInstanceId ) { mBinding.endFrame( inInstanceId ); if ( mOutStream != NULL ) mOutStream->flush(); }

	virtual void connect( const char* inHost
							, int inPort
							, unsigned int inTimeoutInMilliseconds
							//If we are using the debug connection type, we tunnel through the
							//older debugger.  Else we don't and connect directly to pvd without
							//using the older debugger.
							, TConnectionFlagsType inConnectionType )
	{
		disconnect();
		mNextConnectionType = inConnectionType;
		if ( mNextConnectionType & PvdConnectionType::eDEBUG )
		{
			if ( mNextConnectionType != PvdConnectionType::eDEBUG )
			{
				APEX_DEBUG_WARNING( "PVD: APEX + PhysX 2.8.4 does not support simultaneous PVD profile and debug connections.\n"
				                    "       The debug connection takes precedence." );
			}
			if( mDebugger != NULL )
				mDebugger->connect( inHost, inPort );
		}
		else
		{
			mBinding.connect( inHost, inPort, inTimeoutInMilliseconds, inConnectionType );
		}
	}

	virtual void disconnect()
	{
		if ( mDebugger->isConnected() )
			mDebugger->disconnect();
		else
			mBinding.disconnect();
		releaseOutStream();
	}

	virtual void release() { disconnect(); PX_DELETE(this); }

	virtual void onConnect()
	{
		releaseOutStream();
		TOutStreamType* theOutStream = PX_PROFILE_NEW( getAllocator(), TOutStreamType )( mDebugger );
		mOutStream = theOutStream;
		getConnectionManager().connect( NULL, *theOutStream, mNextConnectionType, false );
	}
	/**
	This is called right before the SDK is disconnected from the Visual Remote Debugger application.
	*/
	virtual void onDisconnect() 
	{ 
		getConnectionManager().disconnect(); 
		releaseOutStream();
	}
};

Pvd28Binding& Pvd28Binding::attach( NxRemoteDebugger& inDebugger, bool inSetupFoundation )
{
	Pvd28BindingImpl* retval = PX_NEW(Pvd28BindingImpl)(&inDebugger, inSetupFoundation);
	if ( inDebugger.isConnected() && 
		retval->isConnected() == false )
		retval->onConnect();
	return *retval;
}

#endif // NX_SDK_VERSION_MAJOR == 2

#endif // WITHOUT_PVD