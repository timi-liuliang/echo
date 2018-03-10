/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "PVDBinding.h"
#include "PsShare.h"
#ifndef WITHOUT_PVD

#include "NxModule.h"
#include "NiApexSDK.h"
#include "NiModule.h"

#include "PxProfileZoneManager.h"
#include "PvdDataStream.h"
#include "PvdUserRenderer.h"
#include "PvdConnectionManager.h"
#include "PvdConnection.h"

#include "foundation/PxAllocatorCallback.h"
#include "PsUserAllocated.h"

#include "FrameworkParamClasses.h"

#include "PVDParameterizedHandler.h"

#define INIT_PVD_CLASSES_PARAMETERIZED( parameterizedClassName ) { \
	pvdStream.createClass(NamespacedName(APEX_PVD_NAMESPACE, #parameterizedClassName)); \
	parameterizedClassName* params = DYNAMIC_CAST(parameterizedClassName*)(NiGetApexSDK()->getParameterizedTraits()->createNxParameterized(#parameterizedClassName)); \
	mParameterizedHandler->initPvdClasses(*params->rootParameterDefinition(), #parameterizedClassName); \
	params->destroy(); }

// NOTE: the PvdDataStream is not threadsafe.

using namespace PVD;
using namespace physx;
using namespace physx::shdfnd;
using namespace physx::debugger;
using namespace physx::debugger::comm;
using namespace physx::debugger::renderer;

struct SimpleAllocator : public PxAllocatorCallback 
{
	virtual void* allocate(size_t size, const char* typeName, const char* filename, int line)
	{
		PX_UNUSED(filename);
		PX_UNUSED(line);
		PX_UNUSED(typeName);
		
		// Ensure that we don't use a tracking allocation so that we don't get infinite recursion
		return Foundation::getInstance().getAllocatorCallback().allocate(size, typeName, filename, line);
	}

	virtual void deallocate(void* ptr)
	{
		Foundation::getInstance().getAllocatorCallback().deallocate(ptr);
	}
};
static SimpleAllocator sAllocator;

class PvdBindingImpl : public UserAllocated, public PvdBinding, public PvdConnectionHandler, public PxAllocatorCallback
{
	PvdConnectionManager*		mConnectionManager;
	PxProfileZoneManager*		mProfileManager;
	bool						mReleaseManagers;
	Array<void*>				mInstances;
	PvdDataStream*				mDataStream;
	PvdUserRenderer*			mRenderer;
	PvdParameterizedHandler*	mParameterizedHandler;
	Mutex						mPvdLock;
	bool						mIsLocked;
	TConnectionFlagsType		mConnectionType;
	

public:
	static const char* getPvdBindingNamespace() { return "PVD.PvdBinding"; }
	static const NamespacedName getPvdBindingNamespacedName()
	{
		static NamespacedName instanceNamespace(APEX_PVD_NAMESPACE, "scene");
		return instanceNamespace;
	}

	PvdBindingImpl( PvdConnectionManager& inConnectionManager, physx::PxProfileZoneManager& inProfileManager )
		: mConnectionManager( &inConnectionManager )
		, mProfileManager( &inProfileManager )
		, mReleaseManagers( false )
		, mDataStream( NULL )
		, mRenderer( NULL )
		, mParameterizedHandler( NULL )
		, mIsLocked( false )
	{
		init();
	}

	PvdBindingImpl(bool inRecordMemoryEvents)
		// Using "this" as the allocator argument to PvdConnectionManager::create was not kosher
		: mConnectionManager( &PvdConnectionManager::create( sAllocator, sAllocator, inRecordMemoryEvents ) )
		, mProfileManager( &PxProfileZoneManager::createProfileZoneManager( &Foundation::getInstance() ) )
		, mReleaseManagers( true )
		, mDataStream( NULL )
		, mRenderer( NULL )
		, mParameterizedHandler( NULL )
		, mIsLocked( false )
	{
		mConnectionManager->setProfileZoneManager( *mProfileManager );
		init();
	}

	void init()
	{
		getConnectionManager().addHandler( *this );
	}

	virtual ~PvdBindingImpl()
	{
		if ( mReleaseManagers )
		{
			mProfileManager->release();
			mConnectionManager->release();
		}
		mConnectionManager = NULL;
		mProfileManager = NULL;
	}
	
	virtual PvdConnectionManager& getConnectionManager() 
	{ 
		return *mConnectionManager; 
	}

	virtual physx::PxProfileZoneManager& getProfileManager() 
	{ 
		return *mProfileManager; 
	}

	virtual bool isConnected() 
	{
		return mConnectionManager->isConnected();
	}

	virtual TConnectionFlagsType getConnectionType()
	{
		return mConnectionType;
	}
	
	virtual void disconnect() 
	{
		mConnectionManager->disconnect();
	}

	void* ensureInstance( void* inInstance )
	{
		PxU32 instCount = mInstances.size();
		for ( PxU32 idx = 0; idx < instCount; ++idx )
		{
			if ( mInstances[idx] == inInstance )
				return inInstance;
		}
		if ( mDataStream )
		{
			mDataStream->createInstance(getPvdBindingNamespacedName(), inInstance);
		}
		mInstances.pushBack( inInstance );
		return inInstance;
	}

	virtual void beginFrame( void* inInstance )
	{
		if ( mDataStream == NULL ) return;
		mDataStream->beginSection( ensureInstance( inInstance ), "ApexFrame");
	}

	virtual void endFrame( void* inInstance )
	{
		if ( mDataStream == NULL ) return;
		//Flush the outstanding memory events.  PVD in some situations tracks memory events
		//and can display graphs of memory usage at certain points.  They have to get flushed
		//at some point...
		
		//getConnectionManager().flushMemoryEvents();
		
		//Also note that PVD is a consumer of the profiling system events.  This ensures
		//that PVD gets a view of the profiling events that pertained to the last frame.
		
		//getProfileManager().flushProfileEvents();

		//mDataStream->setPropertyValue( ensureInstance( inInstance ), 1, createSection( SectionType::End ) );
		physx::debugger::PvdScopedItem<physx::debugger::comm::PvdConnection> connection( mConnectionManager->getAndAddRefCurrentConnection() );
		if ( connection ) 
		{
			//Flushes memory and profiling events out of any buffered areas.
			connection->flush();
		}

		mDataStream->endSection( ensureInstance( inInstance ), "ApexFrame");

		//flush our data to the main connection
		//and flush the main connection.
		//This could be an expensive call.
		mDataStream->flush();
		mRenderer->flushRenderEvents();
	}

	virtual void connect( const char* inHost
							, int inPort
							, unsigned int inTimeoutInMilliseconds
							//If we are using the debug connection type, we tunnel through the
							//older debugger.  Else we don't and connect directly to pvd without
							//using the older debugger.
							, physx::debugger::TConnectionFlagsType inConnectionType )
	{
		//check the debugger information in this case.
#if defined(PX_ANDROID)
		//getConnectionManager().connect(	*this, "/sdcard/media/out.pxd2", inConnectionType );
#else
		getConnectionManager().connect(	*this, inHost, inPort, inTimeoutInMilliseconds, inConnectionType );
#endif
	}
	
	virtual void onPvdSendClassDescriptions( PvdConnection& connection )
	{
		PX_UNUSED(connection);
	}

	virtual void onPvdConnected( PvdConnection& connection )
	{
		lock();
		mConnectionType = connection.getConnectionType();

		PX_ASSERT(mRenderer == NULL);
		mRenderer = &connection.createRenderer();
		mRenderer->addRef();

		PX_ASSERT(mDataStream == NULL);
		mDataStream = &connection.createDataStream();
		mDataStream->addRef();

		mParameterizedHandler = PX_NEW(PvdParameterizedHandler)(*mDataStream);

		if (mConnectionType & PvdConnectionType::eDEBUG)
		{
			initPvdClasses();
			initPvdInstances();
		}

		//Setting the namespace ensure that our class definition *can't* collide with
		//someone else's.  It doesn't protect against instance ids colliding which is why
		//people normally use memory addresses casted to unsigned 64 bit numbers for those.
		
		//mDataStream->setNamespace( getPvdBindingNamespace() );
		//mDataStream->createClass( "PvdBinding", 1 );
		//mDataStream->defineProperty( 1, "Frame", "", PvdCommLayerDatatype::Section, 1 );
		mDataStream->createClass( getPvdBindingNamespacedName() );
		mDataStream->flush();

		unlock();
	}

	virtual void onPvdDisconnected( PvdConnection& )
	{
		lock();
		if ( mParameterizedHandler != NULL )
		{
			PX_DELETE(mParameterizedHandler);
			mParameterizedHandler = NULL;
		}

		if ( mDataStream != NULL )
		{
			mDataStream->release();
			mDataStream = NULL;
		}

		if ( mRenderer != NULL )
		{
			mRenderer->release();
			mRenderer = NULL;
		}

		mInstances.clear();

		mConnectionType = PvdConnectionType::eDEBUG;
		unlock();
	}

	//destroy this instance;
	virtual void release() 
	{ 
		disconnect(); 
		PX_DELETE( this ); 
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

	virtual PvdDataStream* getDataStream()
	{
		PX_ASSERT(mIsLocked);
		return mDataStream;
	}

	virtual PvdUserRenderer* getRenderer()
	{
		PX_ASSERT(mIsLocked);
		return mRenderer;
	}

	virtual void initPvdClasses()
	{
		PX_ASSERT(mIsLocked);

		// NxApexSDK
		NamespacedName apexSdkName = NamespacedName(APEX_PVD_NAMESPACE, "NxApexSDK");
		mDataStream->createClass(apexSdkName);
		mDataStream->createProperty(apexSdkName, "Platform", "", getPvdNamespacedNameForType<const char*>(), physx::debugger::PropertyType::Scalar);
		mDataStream->createProperty(apexSdkName, "NxModules", "", getPvdNamespacedNameForType<ObjectRef>(), physx::debugger::PropertyType::Array);

		// init framework parameterized classes
		PvdDataStream& pvdStream = *mDataStream;
		INIT_PVD_CLASSES_PARAMETERIZED(VertexFormatParameters);
		INIT_PVD_CLASSES_PARAMETERIZED(VertexBufferParameters);
		INIT_PVD_CLASSES_PARAMETERIZED(SurfaceBufferParameters);
		INIT_PVD_CLASSES_PARAMETERIZED(SubmeshParameters);
		INIT_PVD_CLASSES_PARAMETERIZED(RenderMeshAssetParameters);
		INIT_PVD_CLASSES_PARAMETERIZED(BufferU8x1);
		INIT_PVD_CLASSES_PARAMETERIZED(BufferU8x2);
		INIT_PVD_CLASSES_PARAMETERIZED(BufferU8x3);
		INIT_PVD_CLASSES_PARAMETERIZED(BufferU8x4);
		INIT_PVD_CLASSES_PARAMETERIZED(BufferU16x1);
		INIT_PVD_CLASSES_PARAMETERIZED(BufferU16x2);
		INIT_PVD_CLASSES_PARAMETERIZED(BufferU16x3);
		INIT_PVD_CLASSES_PARAMETERIZED(BufferU16x4);
		INIT_PVD_CLASSES_PARAMETERIZED(BufferU32x1);
		INIT_PVD_CLASSES_PARAMETERIZED(BufferU32x2);
		INIT_PVD_CLASSES_PARAMETERIZED(BufferU32x3);
		INIT_PVD_CLASSES_PARAMETERIZED(BufferU32x4);
		INIT_PVD_CLASSES_PARAMETERIZED(BufferF32x1);
		INIT_PVD_CLASSES_PARAMETERIZED(BufferF32x2);
		INIT_PVD_CLASSES_PARAMETERIZED(BufferF32x3);
		INIT_PVD_CLASSES_PARAMETERIZED(BufferF32x4);

		// module classes
		NiApexSDK* niApexSDK = NiGetApexSDK();
		PxU32 numModules = niApexSDK->getNbModules();
		for (PxU32 i = 0; i < numModules; ++i)
		{
			NiModule* niModule = niApexSDK->getNiModules()[i];
			NxModule* nxModule = niApexSDK->getModules()[i];

			NamespacedName moduleName;
			moduleName.mNamespace = APEX_PVD_NAMESPACE;
			moduleName.mName = nxModule->getName();
			mDataStream->createClass(moduleName);

			niModule->initPvdClasses(*mDataStream);
		}
	}

	virtual void initPvdInstances()
	{
		PX_ASSERT(mIsLocked);

		NxApexSDK* apexSdk = NxGetApexSDK();
		mDataStream->createInstance( NamespacedName(APEX_PVD_NAMESPACE, "NxApexSDK"), apexSdk);

		// set platform name
		NxParameterized::SerializePlatform platform;
		apexSdk->getCurrentPlatform(platform);
		const char* platformName = apexSdk->getPlatformName(platform);
		mDataStream->setPropertyValue(apexSdk, "Platform", platformName);

		mConnectionManager->setIsTopLevelUIElement(apexSdk, true);

		// add module instances
		PxU32 numModules = apexSdk->getNbModules();
		NiApexSDK* niApexSDK = NiGetApexSDK();
		for (PxU32 i = 0; i < numModules; ++i)
		{
			// init pvd instances
			NxModule* nxModule = apexSdk->getModules()[i];
			NiModule* niModule = niApexSDK->getNiModules()[i];

			NamespacedName moduleName;
			moduleName.mNamespace = APEX_PVD_NAMESPACE;
			moduleName.mName = nxModule->getName();
			mDataStream->createInstance(moduleName, nxModule);
			mDataStream->pushBackObjectRef(apexSdk, "NxModules", nxModule);

			niModule->initPvdInstances(*mDataStream);
		}
	}


	virtual void initPvdClasses(const NxParameterized::Definition& paramsHandle, const char* pvdClassName)
	{
		PX_ASSERT(mIsLocked);

		if (mParameterizedHandler != NULL)
		{
			mParameterizedHandler->initPvdClasses(paramsHandle, pvdClassName);
		}
	}


	virtual void updatePvd(const void* pvdInstance, NxParameterized::Interface& params, PvdAction::Enum pvdAction)
	{
		PX_ASSERT(mIsLocked);

		if (mParameterizedHandler != NULL)
		{
			NxParameterized::Handle paramsHandle(params);
			mParameterizedHandler->updatePvd(pvdInstance, paramsHandle, pvdAction);
		}
	}
		

	virtual void lock()
	{
		mPvdLock.lock();
		mIsLocked = true;
	}


	virtual void unlock()
	{
		mIsLocked = false;
		mPvdLock.unlock();
	}
};


PvdBinding* PvdBinding::create( physx::debugger::comm::PvdConnectionManager& inConnectionManager, physx::PxProfileZoneManager& inProfileManager )
{
	return PX_NEW( PvdBindingImpl) ( inConnectionManager, inProfileManager );
}

PvdBinding* PvdBinding::create( bool inRecordMemoryEvents )
{
	return PX_NEW( PvdBindingImpl ) ( inRecordMemoryEvents );
}

#else

PVD::PvdBinding* PVD::PvdBinding::create( physx::debugger::comm::PvdConnectionManager&, physx::PxProfileZoneManager& )
{
	return NULL;
}

PVD::PvdBinding* PVD::PvdBinding::create( bool )
{
	return NULL;
}

#endif // WITHOUT_PVD