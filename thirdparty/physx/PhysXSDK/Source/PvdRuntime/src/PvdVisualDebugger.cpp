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

// suppress LNK4221
#include "PxPreprocessor.h"
PX_DUMMY_SYMBOL

#include "PxVisualDebugger.h"
#if PX_SUPPORT_VISUAL_DEBUGGER

#include "PxPhysXCommonConfig.h"
#include "PxProfileBase.h"

#include "PvdVisualDebugger.h"
#include "PvdDataStream.h"
#include "PvdConnection.h"

#include "ScPhysics.h"
#include "NpScene.h"
#include "PsFoundation.h"

#include "ScBodyCore.h"

#include "NpRigidDynamic.h"
#include "NpRigidStatic.h"
#include "PxHeightFieldSample.h"
#include "PvdTypeNames.h"

namespace physx
{
namespace Pvd
{

VisualDebugger::VisualDebugger()
: mPvdDataStream(NULL)
, mPvdConnection(NULL)
, mFlags(0)
, mIsConnected(0)
{
}


VisualDebugger::~VisualDebugger()
{
	disconnect();
}


void VisualDebugger::disconnect()
{
	NpPhysics& npPhysics = NpPhysics::getInstance();
	if ( npPhysics.getPvdConnectionManager() )
		npPhysics.getPvdConnectionManager()->disconnect();
}


physx::debugger::comm::PvdConnection* VisualDebugger::getPvdConnection()
{
	return mPvdConnection;
}

physx::debugger::comm::PvdDataStream* VisualDebugger::getPvdDataStream(const PxScene& scene)
{
	const NpScene& npScene = static_cast<const NpScene&>(scene);
	return npScene.getScene().getSceneVisualDebugger().getPvdDataStream();
}


void VisualDebugger::updateCamera(const char* name, const PxVec3& origin, const PxVec3& up, const PxVec3& target)
{
	NpPhysics& npPhysics = NpPhysics::getInstance();
	npPhysics.getPvdConnectionManager()->setCamera( name, origin, up, target );
}

void VisualDebugger::sendErrorMessage(PxErrorCode::Enum code, const char* message, const char* file, PxU32 line)
{
	if( mPvdConnection == NULL )
		return;

	NpPhysics& npPhysics = NpPhysics::getInstance();
	npPhysics.getPvdConnectionManager()->sendErrorMessage( code, message, file, line );
}

void VisualDebugger::setCreateContactReports(bool value)
{
	if ( isConnected() )
	{
		NpPhysics& npPhysics = NpPhysics::getInstance();
		PxU32 numScenes = npPhysics.getNbScenes();
		for(PxU32 i = 0; i < numScenes; i++)
		{
			NpScene* npScene = npPhysics.getScene(i);
			Scb::Scene& scbScene = npScene->getScene();
			scbScene.getSceneVisualDebugger().setCreateContactReports(value);
		}
	}
}

void VisualDebugger::setVisualDebuggerFlag(PxVisualDebuggerFlag::Enum flag, bool value)
{
	if(value)
		mFlags |= PxU32(flag);
	else
		mFlags &= ~PxU32(flag);
	//This has been a bug against the debugger for some time,
	//changing this flag doesn't always change the sending-contact-reports behavior.
	if ( flag == PxVisualDebuggerFlag::eTRANSMIT_CONTACTS )
	{
		setCreateContactReports( value );
	}
}

void VisualDebugger::setVisualDebuggerFlags(PxVisualDebuggerFlags flags)
{
	bool oldContactFlag = mFlags & PxVisualDebuggerFlag::eTRANSMIT_CONTACTS;
	bool newContactFlag = flags & PxVisualDebuggerFlag::eTRANSMIT_CONTACTS;

	mFlags = PxU32(flags);
	

	//This has been a bug against the debugger for some time,
	//changing this flag doesn't always change the sending-contact-reports behavior.
	if ( oldContactFlag != newContactFlag )
	{
		setCreateContactReports( newContactFlag );
	}
}

PxU32 VisualDebugger::getVisualDebuggerFlags()
{
	return mFlags;
}

bool VisualDebugger::isConnected(bool useCachedStatus)
{
	return useCachedStatus ? (Ps::atomicCompareExchange(&mIsConnected, 1, 1) == 1)
		: (mPvdConnection && mPvdConnection->isConnected());
}

void VisualDebugger::checkConnection()
{
	if ( mPvdConnection != NULL ) mPvdConnection->checkConnection();
}


void VisualDebugger::updateScenesPvdConnection()
{
	NpPhysics& npPhysics = NpPhysics::getInstance();
	PxU32 numScenes = npPhysics.getNbScenes();
	for(PxU32 i = 0; i < numScenes; i++)
	{
		NpScene* npScene = npPhysics.getScene(i);
		Scb::Scene& scbScene = npScene->getScene();
		setupSceneConnection(scbScene);
		npScene->getSingleSqCollector().clearGeometryArrays();
		npScene->getBatchedSqCollector().clearGeometryArrays();
	}
}

void VisualDebugger::setupSceneConnection(Scb::Scene& s)
{
	physx::debugger::comm::PvdDataStream* conn = mPvdConnection ? &mPvdConnection->createDataStream() : NULL;
	if(conn)
	s.getSceneVisualDebugger().setPvdConnection(conn, mPvdConnection ? mPvdConnection->getConnectionType() : physx::debugger::TConnectionFlagsType(0));
	s.getSceneVisualDebugger().setCreateContactReports(conn ? getTransmitContactsFlag() : false);
}


void VisualDebugger::sendClassDescriptions()
{
	if(!isConnected())
		return;
	mMetaDataBinding.registerSDKProperties(*mPvdDataStream);
	mPvdDataStream->flush();
}

void VisualDebugger::onPvdSendClassDescriptions( physx::debugger::comm::PvdConnection& inFactory )
{
	physx::debugger::comm::PvdConnection* cf( &inFactory );
	if(!cf)
		return;

	if(mPvdDataStream && mPvdConnection)
		disconnect();

	mPvdConnection = cf;
	mPvdDataStream = &mPvdConnection->createDataStream();
	if(!mPvdDataStream)
		return;
	mPvdDataStream->addRef();
	sendClassDescriptions();
}

void VisualDebugger::onPvdConnected( physx::debugger::comm::PvdConnection& /*inFactory */)
{
	if(!mPvdDataStream)
		return;
	updateScenesPvdConnection();
	sendEntireSDK();
	Ps::atomicExchange(&mIsConnected, 1);
}
void VisualDebugger::onPvdDisconnected( physx::debugger::comm::PvdConnection& /*inFactory */)
{
	Ps::atomicExchange(&mIsConnected, 0);
	if(mPvdDataStream)
	{
//		NpPhysics& npPhysics = NpPhysics::getInstance();
		if(mPvdDataStream->isConnected())
		{
			mPvdDataStream->destroyInstance(&NpPhysics::getInstance());
			mPvdDataStream->flush();
		}
		mPvdDataStream->release();
		mPvdDataStream = NULL;
		mPvdConnection = NULL;
		updateScenesPvdConnection();
		mRefCountMapLock.lock();
		mRefCountMap.clear();
		mRefCountMapLock.unlock();
	}
}

template<typename TDataType> void VisualDebugger::doMeshFactoryBufferRelease( const TDataType* type )
{
	if ( mPvdConnection != NULL 
		&& mPvdDataStream != NULL
		&& mPvdConnection->getConnectionType() & physx::debugger::PvdConnectionType::eDEBUG )
	{
		Ps::Mutex::ScopedLock locker(mRefCountMapLock);
		if ( mRefCountMap.find( type ) )
		{
			mRefCountMap.erase( type );
			destroyPvdInstance( type );
			mPvdDataStream->flush();
		}
	}
}

void VisualDebugger::onGuMeshFactoryBufferRelease(const PxBase* object, PxType typeID, bool memRelease)
{
	PX_UNUSED(memRelease);

	switch(typeID)
	{
		case PxConcreteType::eHEIGHTFIELD:
			doMeshFactoryBufferRelease( static_cast<const PxHeightField*>(object) );
		break;

		case PxConcreteType::eCONVEX_MESH:
			doMeshFactoryBufferRelease( static_cast<const PxConvexMesh*>(object) );
		break;

		case PxConcreteType::eTRIANGLE_MESH:
			doMeshFactoryBufferRelease( static_cast<const PxTriangleMesh*>(object) );
		break;

		default:
		break;
	}	
}
#if PX_USE_CLOTH_API
void VisualDebugger::onNpFactoryBufferRelease(PxClothFabric& data)
{
	doMeshFactoryBufferRelease( &data );
}
#endif

void VisualDebugger::sendEntireSDK()
{
	NpPhysics& npPhysics = NpPhysics::getInstance();
	mPvdDataStream->createInstance( (PxPhysics*)&npPhysics );
	npPhysics.getPvdConnectionManager()->setIsTopLevelUIElement( &npPhysics, true );
	mMetaDataBinding.sendAllProperties( *mPvdDataStream, npPhysics );

#define SEND_BUFFER_GROUP( type, name ) {					\
		Ps::Array<type*> buffers;							\
		PxU32 numBuffers = npPhysics.getNb##name();			\
		buffers.resize(numBuffers);							\
		npPhysics.get##name(buffers.begin(), numBuffers);	\
		for(PxU32 i = 0; i < numBuffers; i++)				\
			increaseReference(buffers[i]);					\
	}

	SEND_BUFFER_GROUP( PxMaterial, Materials );
	SEND_BUFFER_GROUP( PxTriangleMesh, TriangleMeshes );
	SEND_BUFFER_GROUP( PxConvexMesh, ConvexMeshes );
	SEND_BUFFER_GROUP( PxHeightField, HeightFields );

#if PX_USE_CLOTH_API
	SEND_BUFFER_GROUP( PxClothFabric, ClothFabrics );
#endif

	mPvdDataStream->flush();
	PxU32 numScenes = npPhysics.getNbScenes();
	for(PxU32 i = 0; i < numScenes; i++)
	{
		NpScene* npScene = npPhysics.getScene(i);
		Scb::Scene& scbScene = npScene->getScene();

		scbScene.getSceneVisualDebugger().sendEntireScene();
	}
}


void VisualDebugger::createPvdInstance(const PxTriangleMesh* triMesh)
{
	mMetaDataBinding.createInstance( *mPvdDataStream, *triMesh, NpPhysics::getInstance() );
}

void VisualDebugger::destroyPvdInstance(const PxTriangleMesh* triMesh)
{
	mMetaDataBinding.destroyInstance( *mPvdDataStream, *triMesh, NpPhysics::getInstance() );
}


void VisualDebugger::createPvdInstance(const PxConvexMesh* convexMesh)
{
	mMetaDataBinding.createInstance( *mPvdDataStream, *convexMesh, NpPhysics::getInstance() );
}

void VisualDebugger::destroyPvdInstance(const PxConvexMesh* convexMesh)
{
	mMetaDataBinding.destroyInstance( *mPvdDataStream, *convexMesh, NpPhysics::getInstance() );
}

void VisualDebugger::createPvdInstance(const PxHeightField* heightField)
{
	mMetaDataBinding.createInstance( *mPvdDataStream, *heightField, NpPhysics::getInstance() );
}

void VisualDebugger::destroyPvdInstance(const PxHeightField* heightField)
{
	mMetaDataBinding.destroyInstance( *mPvdDataStream, *heightField, NpPhysics::getInstance() );
}

void VisualDebugger::createPvdInstance(const PxMaterial* mat)
{
	mMetaDataBinding.createInstance( *mPvdDataStream, *mat, NpPhysics::getInstance() );
}

void VisualDebugger::updatePvdProperties(const PxMaterial* mat)
{
	mMetaDataBinding.sendAllProperties( *mPvdDataStream, *mat );
}

void VisualDebugger::destroyPvdInstance(const PxMaterial* mat)
{
	mMetaDataBinding.destroyInstance( *mPvdDataStream, *mat, NpPhysics::getInstance() );
}

#if PX_USE_CLOTH_API
void VisualDebugger::createPvdInstance(const PxClothFabric* fabric)
{
	NpPhysics* npPhysics = &NpPhysics::getInstance();
	mMetaDataBinding.createInstance( *mPvdDataStream, *fabric, *npPhysics );
	//Physics level buffer object update, must be flushed immediatedly. 
	flush();
}

void VisualDebugger::destroyPvdInstance(const PxClothFabric* fabric)
{
	NpPhysics* npPhysics = &NpPhysics::getInstance();
	mMetaDataBinding.destroyInstance( *mPvdDataStream, *fabric, *npPhysics );
	//Physics level buffer object update, must be flushed immediatedly. 
	flush();
}
#endif

void VisualDebugger::flush() 
{
	mPvdDataStream->flush();
}


} // namespace Pvd

}

#endif  // PX_SUPPORT_VISUAL_DEBUGGER
