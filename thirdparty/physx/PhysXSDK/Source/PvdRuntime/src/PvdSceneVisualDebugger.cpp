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

#include "PvdSceneVisualDebugger.h"
#include "PvdDataStream.h"

#include "ScPhysics.h"
#include "NpScene.h"
#include "PsFoundation.h"

#include "ScBodyCore.h"
#include "ScBodySim.h"
#include "ScConstraintSim.h"
#include "ScParticleSystemSim.h"

#include "NpRigidDynamic.h"
#include "NpRigidStatic.h"

#include "NpArticulation.h"
#include "NpArticulationLink.h"
#include "NpArticulationJoint.h"

#include "NpParticleFluid.h"
#include "CmEventProfiler.h"

#include "ScbCloth.h"
#include "NpCloth.h"

#include "NpAggregate.h"

#include "PvdConnection.h"
#include "PvdTypeNames.h"

#include "PvdUserRenderer.h"


namespace physx
{
namespace Pvd
{

#define UPDATE_PVD_PROPERTIES_CHECK() { if ( !isConnectedAndSendingDebugInformation() ) return; }

PX_FORCE_INLINE static NpScene* getNpScene(Scb::Scene* scbScene) 
{ 
	return static_cast<NpScene*>(scbScene->getPxScene());
}

PX_FORCE_INLINE static const NpRigidDynamic* getNpRigidDynamic(const Scb::Body* scbBody) 
{ 
	size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<NpRigidDynamic*>(0)->getScbActorFast()));
	return reinterpret_cast<const NpRigidDynamic*>(reinterpret_cast<const char*>(scbBody)-offset);
}

PX_FORCE_INLINE static NpRigidDynamic* getNpRigidDynamic(Scb::Body* scbBody) 
{ 
	size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<NpRigidDynamic*>(0)->getScbActorFast()));
	return reinterpret_cast<NpRigidDynamic*>(reinterpret_cast<char*>(scbBody)-offset);
}

PX_FORCE_INLINE static const NpRigidStatic* getNpRigidStatic(const Scb::RigidStatic* scbRigidStatic) 
{ 
	size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<NpRigidStatic*>(0)->getScbActorFast()));
	return reinterpret_cast<const NpRigidStatic*>(reinterpret_cast<const char*>(scbRigidStatic)-offset);
}

PX_FORCE_INLINE static NpRigidStatic* getNpRigidStatic(Scb::RigidStatic* scbRigidStatic) 
{ 
	size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<NpRigidStatic*>(0)->getScbActorFast()));
	return reinterpret_cast<NpRigidStatic*>(reinterpret_cast<char*>(scbRigidStatic)-offset);
}

PX_FORCE_INLINE static NpShape* getNpShape(Scb::Shape* scbShape) 
{ 
	size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<NpShape*>(0)->getScbShape()));
	return reinterpret_cast<NpShape*>(reinterpret_cast<char*>(scbShape)-offset);
}
PX_FORCE_INLINE static NpShape* getNpShape(const Scb::Shape* scbShape) 
{ 
	return getNpShape( const_cast< Scb::Shape* >( scbShape ) );
}

#if PX_USE_PARTICLE_SYSTEM_API

PX_FORCE_INLINE static const NpParticleSystem* getNpParticleSystem(const Scb::ParticleSystem* scbParticleSystem) 
{ 
	size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<NpParticleSystem*>(0)->getScbActor()));
	return reinterpret_cast<const NpParticleSystem*>(reinterpret_cast<const char*>(scbParticleSystem)-offset);
}

PX_FORCE_INLINE static const NpParticleFluid* getNpParticleFluid(const Scb::ParticleSystem* scbParticleSystem) 
{ 
	size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<NpParticleFluid*>(0)->getScbActor()));
	return reinterpret_cast<const NpParticleFluid*>(reinterpret_cast<const char*>(scbParticleSystem)-offset);
}

#endif

PX_FORCE_INLINE static const NpArticulationLink* getNpArticulationLink(const Scb::Body* scbArticulationLink) 
{ 
	size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<NpArticulationLink*>(0)->getScbActorFast()));
	return reinterpret_cast<const NpArticulationLink*>(reinterpret_cast<const char*>(scbArticulationLink)-offset);
}

PX_FORCE_INLINE static NpArticulationLink* getNpArticulationLink(Scb::Body* scbArticulationLink) 
{ 
	size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<NpArticulationLink*>(0)->getScbActorFast()));
	return reinterpret_cast<NpArticulationLink*>(reinterpret_cast<char*>(scbArticulationLink)-offset);
}

PX_FORCE_INLINE static NpArticulation* getNpArticulation(Scb::Articulation* scbArticulation) 
{ 
	size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<NpArticulation*>(0)->getArticulation()));
	return reinterpret_cast<NpArticulation*>(reinterpret_cast<char*>(scbArticulation)-offset);
}

PX_FORCE_INLINE static NpArticulationJoint* getNpArticulationJoint(Scb::ArticulationJoint* scbArticulationJoint) 
{ 
	size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<NpArticulationJoint*>(0)->getScbArticulationJoint()));
	return reinterpret_cast<NpArticulationJoint*>(reinterpret_cast<char*>(scbArticulationJoint)-offset);
}

PX_FORCE_INLINE static NpConstraint* getNpConstraint(Sc::ConstraintCore* scConstraint) 
{ 
	size_t scOffset = reinterpret_cast<size_t>(&(reinterpret_cast<Scb::Constraint*>(0)->getScConstraint()));
	size_t scbOffset = reinterpret_cast<size_t>(&(reinterpret_cast<NpConstraint*>(0)->getScbConstraint()));
	return reinterpret_cast<NpConstraint*>(reinterpret_cast<char*>(scConstraint)-scOffset-scbOffset);
}

#if PX_USE_PARTICLE_SYSTEM_API

PX_FORCE_INLINE static Scb::ParticleSystem* getScbParticleSystem(Sc::ParticleSystemCore* scParticleSystem) 
{ 
	size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<Scb::ParticleSystem*>(0)->getScParticleSystem()));
	return reinterpret_cast<Scb::ParticleSystem*>(reinterpret_cast<char*>(scParticleSystem)-offset);
}

#endif

#if PX_USE_CLOTH_API

PX_FORCE_INLINE static NpCloth* backptr(Scb::Cloth* cloth) 
{ 
	size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<NpCloth*>(0)->getScbCloth()));
	return reinterpret_cast<NpCloth*>(reinterpret_cast<char*>(cloth)-offset);
}

#endif

PX_FORCE_INLINE static const PxActor* getPxActor(const Scb::Actor* scbActor)
{
	PxActorType::Enum type = scbActor->getActorCore().getActorCoreType();
	if(type == PxActorType::eRIGID_DYNAMIC)
	{
		return getNpRigidDynamic(static_cast<const Scb::Body*>(scbActor));
	}
	else if(type == PxActorType::eRIGID_STATIC)
	{
		return getNpRigidStatic(static_cast<const Scb::RigidStatic*>(scbActor));
	}
#if PX_USE_PARTICLE_SYSTEM_API
	else if (type == PxActorType::ePARTICLE_SYSTEM)
	{
		return getNpParticleSystem(static_cast<const Scb::ParticleSystem*>(scbActor));
	}
	else if (type == PxActorType::ePARTICLE_FLUID)
	{
		return getNpParticleFluid(static_cast<const Scb::ParticleSystem*>(scbActor));
	}
#endif
	else if (type == PxActorType::eARTICULATION_LINK)
	{
		return getNpArticulationLink(static_cast<const Scb::Body*>(scbActor));
	}
#if PX_USE_CLOTH_API
	else if (type == PxActorType::eCLOTH)
	{
		return backptr(const_cast<Scb::Cloth*>(static_cast<const Scb::Cloth*>(scbActor)));
	}
#endif
	
	return NULL;
}

PX_FORCE_INLINE static NpAggregate* getNpAggregate(Scb::Aggregate* aggregate) 
{ 
	size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<NpAggregate*>(0)->getScbAggregate()));
	return reinterpret_cast<NpAggregate*>(reinterpret_cast<char*>(aggregate)-offset);
}

namespace {
	struct PvdConstraintVisualizer : public PxConstraintVisualizer
	{
		physx::debugger::renderer::PvdUserRenderer& mRenderer;
		PvdConstraintVisualizer( const void* id, physx::debugger::renderer::PvdUserRenderer& r )
			: mRenderer( r )
		{
			mRenderer.setInstanceId( id );
		}
		virtual void visualizeJointFrames( const PxTransform& parent, const PxTransform& child )
		{
			mRenderer.visualizeJointFrames( parent, child );
		}

		virtual void visualizeLinearLimit( const PxTransform& t0, const PxTransform& t1, PxReal value, bool active )
		{
			mRenderer.visualizeLinearLimit( t0, t1, static_cast<PxF32>( value ), active );
		}

		virtual void visualizeAngularLimit( const PxTransform& t0, PxReal lower, PxReal upper, bool active)
		{
			mRenderer.visualizeAngularLimit( t0, static_cast<PxF32>( lower ), static_cast<PxF32>( upper ), active );
		}

		virtual void visualizeLimitCone( const PxTransform& t, PxReal ySwing, PxReal zSwing, bool active)
		{
			mRenderer.visualizeLimitCone( t, static_cast<PxF32>( ySwing ), static_cast<PxF32>( zSwing ), active );
		}

		virtual void visualizeDoubleCone( const PxTransform& t, PxReal angle, bool active)
		{
			mRenderer.visualizeDoubleCone( t, static_cast<PxF32>( angle ), active );
		}

	private:
		PvdConstraintVisualizer& operator=(const PvdConstraintVisualizer&);
	};
}



SceneVisualDebugger::SceneVisualDebugger(Scb::Scene& s)
: mPvdDataStream(NULL)
, mUserRenderer( NULL )
, mScbScene(s)
, mConnectionType( 0 )
{
}

void SceneVisualDebugger::detach()
{
	setCreateContactReports(false);
}

SceneVisualDebugger::~SceneVisualDebugger()
{
	if(isConnected())
		releasePvdInstance();

	if (mUserRenderer)
		mUserRenderer->release();
	if(mPvdDataStream)
		mPvdDataStream->release();
}

namespace 
{
	static Pvd::VisualDebugger &getSdkPvd()
	{
		return *static_cast<Pvd::VisualDebugger*>(NpPhysics::getInstance().getVisualDebugger());
	}
}

physx::debugger::comm::PvdDataStream* SceneVisualDebugger::getPvdDataStream() const
{
	return mPvdDataStream;
}


void SceneVisualDebugger::setPvdConnection(physx::debugger::comm::PvdDataStream* c, PxU32 inConnectionType)
{
	if ( mUserRenderer )
		mUserRenderer->release();
	mUserRenderer = NULL;
	if(mPvdDataStream)
		mPvdDataStream->release();
	mConnectionType = inConnectionType;

	mPvdDataStream = c;

	if(mPvdDataStream)
		c->addRef();
	else
		mProfileZoneIdList.clear();		
}

void SceneVisualDebugger::setCreateContactReports(bool s)
{
	mScbScene.getScScene().setCreateContactReports(s);
}

bool SceneVisualDebugger::isConnected(bool useCachedStatus) const
{
	return useCachedStatus ? getSdkPvd().isConnected(true) 
		:  (mPvdDataStream && mPvdDataStream->isConnected());
}

bool SceneVisualDebugger::isConnectedAndSendingDebugInformation()
{
	return isConnected()
			&& ( mConnectionType & physx::debugger::PvdConnectionType::eDEBUG );
}

void SceneVisualDebugger::sendEntireScene()
{
	if(!isConnected())
		return;

	NpScene* npScene = getNpScene(&mScbScene);

	if( npScene->getFlagsFast() & PxSceneFlag::eREQUIRE_RW_LOCK )
			npScene->lockRead(__FILE__, __LINE__);

	createPvdInstance();
	
	VisualDebugger& sdkPvd = getSdkPvd();
	// materials:
	{
		PxsMaterialManager* manager = mScbScene.getScScene().getMaterialManager();
		PxsMaterialManagerIterator iter(*manager);
		PxsMaterialCore* mat;
		while(iter.getNextMaterial(mat))
			sdkPvd.increaseReference(mat->getNxMaterial());
	}

	if ( isConnectedAndSendingDebugInformation() )
	{		
		Ps::Array<PxActor*> actorArray;		

		// RBs
		// static:
		{
			PxU32 numActors = npScene->getNbActors(PxActorTypeFlag::eRIGID_STATIC | PxActorTypeFlag::eRIGID_DYNAMIC );
			actorArray.resize(numActors);
			npScene->getActors(PxActorTypeFlag::eRIGID_STATIC | PxActorTypeFlag::eRIGID_DYNAMIC, actorArray.begin(), actorArray.size());
			for(PxU32 i = 0; i < numActors; i++)
			{
				PxActor* pxActor = actorArray[i];
				if ( pxActor->is<PxRigidStatic>() )
					mMetaDataBinding.createInstance( *mPvdDataStream, *static_cast<PxRigidStatic*>(pxActor), *npScene, sdkPvd );
				else
					mMetaDataBinding.createInstance( *mPvdDataStream, *static_cast<PxRigidDynamic*>(pxActor), *npScene, sdkPvd );
			}
		}
		// articulations & links
		{
			Ps::Array<PxArticulation*> articulations;
			PxU32 numArticulations = npScene->getNbArticulations();
			articulations.resize(numArticulations);
			npScene->getArticulations(articulations.begin(), articulations.size());
			for(PxU32 i = 0; i < numArticulations; i++)
				mMetaDataBinding.createInstance( *mPvdDataStream, *articulations[i], *npScene, sdkPvd );
		}

#if PX_USE_PARTICLE_SYSTEM_API
		// particle systems & fluids:
		{
			PxU32 nbParticleSystems = mScbScene.getScScene().getNbParticleSystems();
			Sc::ParticleSystemCore** particleSystems = mScbScene.getScScene().getParticleSystems();
			for(PxU32 i = 0; i < nbParticleSystems; i++)
			{
				Scb::ParticleSystem* scbParticleSystem = getScbParticleSystem(particleSystems[i]);
				createPvdInstance(scbParticleSystem);
			}
		}
#endif

#if PX_USE_CLOTH_API
		//cloth 
		{
			Ps::Array<PxActor*> cloths;
			PxU32 numActors = npScene->getNbActors(PxActorTypeFlag::eCLOTH);
			cloths.resize(numActors);
			npScene->getActors(PxActorTypeFlag::eCLOTH, cloths.begin(), cloths.size());
			for(PxU32 i = 0; i < numActors; i++)
			{
				Scb::Cloth* scbCloth = &static_cast<NpCloth*>(cloths[i])->getScbCloth();
				createPvdInstance(scbCloth);
			}
		}
#endif

		// joints
		{
			Sc::ConstraintCore** constraints = mScbScene.getScScene().getConstraints();
			PxU32 nbConstraints = mScbScene.getScScene().getNbConstraints();
			for(PxU32 i=0;i<nbConstraints;i++)
			{
				updateConstraint(*constraints[i], PxPvdUpdateType::CREATE_INSTANCE);
				updateConstraint(*constraints[i], PxPvdUpdateType::UPDATE_ALL_PROPERTIES);
			}
		}
	}

	if( npScene->getFlagsFast() & PxSceneFlag::eREQUIRE_RW_LOCK )
		npScene->unlockRead();

	mPvdDataStream->flush();
}


void SceneVisualDebugger::frameStart(PxReal simElapsedTime)
{
	if(!isConnected())
		return;
	mMetaDataBinding.sendBeginFrame( *mPvdDataStream, mScbScene.getPxScene(), simElapsedTime );
	mPvdDataStream->flush();
}


void SceneVisualDebugger::frameEnd()
{
	if(!isConnected())
		return;
	
	const PxScene* theScene = mScbScene.getPxScene();

	//Send the statistics for last frame.
	mMetaDataBinding.sendStats( *mPvdDataStream, theScene  );

	if(isConnectedAndSendingDebugInformation())
	{
#if PX_USE_PARTICLE_SYSTEM_API
		// particle systems & fluids:
		{
			CM_PROFILE_ZONE_WITH_SUBSYSTEM( mScbScene,PVD,updatePariclesAndFluids );
			PxU32 nbParticleSystems = mScbScene.getScScene().getNbParticleSystems();
			Sc::ParticleSystemCore** particleSystems = mScbScene.getScScene().getParticleSystems();
			for(PxU32 i = 0; i < nbParticleSystems; i++)
			{
				sendStateDatas(particleSystems[i]);
			}
		}
#endif

#if PX_USE_CLOTH_API
		{
			CM_PROFILE_ZONE_WITH_SUBSYSTEM( mScbScene,PVD,updateCloths);
			mMetaDataBinding.updateCloths( *mPvdDataStream, *theScene );
		}
#endif
	}


	//Flush the outstanding memory events.  PVD in some situations tracks memory events
	//and can display graphs of memory usage at certain points.  They have to get flushed
	//at some point...
	//Also note that PVD is a consumer of the profiling system events.  This ensures
	//that PVD gets a view of the profiling events that pertained to the last frame.
	physx::debugger::PvdScopedItem<physx::debugger::comm::PvdConnection> connection( NpPhysics::getInstance().getPvdConnectionManager()->getAndAddRefCurrentConnection() );
	if ( connection ) 
	{
		//Flushes memory and profiling events out of any buffered areas.
		connection->flush();
	}
	

	//End the frame *before* we send the dynamic object current data.
	//This ensures that contacts end up synced with the rest of the system.
	//Note that contacts were sent much earler in NpScene::fetchResults.
	mMetaDataBinding.sendEndFrame(*mPvdDataStream, mScbScene.getPxScene() );
	//flush our data to the main connection
	mPvdDataStream->flush(); 


	
	VisualDebugger& sdkPvd = getSdkPvd();


	if(isConnectedAndSendingDebugInformation())
	{
		const bool visualizeJoints = ( sdkPvd.getVisualDebuggerFlags() & PxVisualDebuggerFlag::eTRANSMIT_CONSTRAINTS ) != 0;
	
		if ( visualizeJoints && mUserRenderer == NULL )
		{
			NpPhysics& physics( NpPhysics::getInstance() );
			physx::debugger::comm::PvdConnectionManager& mgr( *physics.getPvdConnectionManager() );
			physx::debugger::comm::PvdConnection* connection1( mgr.getAndAddRefCurrentConnection() );
			if ( connection1 )
			{
				mUserRenderer = &connection1->createRenderer();
				mUserRenderer->addRef();
				connection1->release();
			}
		}
		{
			PvdVisualizer* vizualizer = NULL;
			if ( visualizeJoints ) vizualizer = this;
			CM_PROFILE_ZONE_WITH_SUBSYSTEM( mScbScene,PVD,sceneUpdate );
			mMetaDataBinding.updateDynamicActorsAndArticulations( *mPvdDataStream, theScene, vizualizer );
		}

		// frame end moved to update contacts to have them in the previous frame.
	}
}

void SceneVisualDebugger::originShift(PxVec3 shift)
{
	if(!isConnected())
		return;
	
	const PxScene* theScene = mScbScene.getPxScene();
	mMetaDataBinding.originShift(*mPvdDataStream, theScene, shift);	
	mPvdDataStream->flush();

}

void SceneVisualDebugger::createPvdInstance()
{
	mPvdDataStream->createInstance( mScbScene.getPxScene() );
	updatePvdProperties();
}


void SceneVisualDebugger::updatePvdProperties()
{
	PxScene* theScene = mScbScene.getPxScene();
	mMetaDataBinding.sendAllProperties( *mPvdDataStream, *theScene  );
}


void SceneVisualDebugger::releasePvdInstance()
{
	mPvdDataStream->destroyInstance(mScbScene.getPxScene());
}

template<typename TOperator>
inline void ActorTypeOperation( const PxActor* actor, TOperator op )
{
	switch( actor->getType() )
	{
	case PxActorType::eRIGID_STATIC: op( *static_cast<const PxRigidStatic*>( actor ) ); break;
	case PxActorType::eRIGID_DYNAMIC: op( *static_cast<const PxRigidDynamic*>( actor ) ); break;
	case PxActorType::eARTICULATION_LINK: op( *static_cast<const PxArticulationLink*>( actor ) ); break;
#if PX_USE_PARTICLE_SYSTEM_API
	case PxActorType::ePARTICLE_SYSTEM: op( *static_cast<const PxParticleSystem*>( actor ) ); break;
	case PxActorType::ePARTICLE_FLUID: op( *static_cast<const PxParticleFluid*>( actor ) ); break;
#endif
#if PX_USE_CLOTH_API
	case PxActorType::eCLOTH: op( *static_cast<const PxCloth*>( actor ) ); break;
#endif
	case PxActorType::eACTOR_COUNT:
	case PxActorType::eACTOR_FORCE_DWORD:
	default:
		PX_ASSERT( false );
		break;
	};
}

struct CreateOp
{
	CreateOp& operator=( const CreateOp& );
	physx::debugger::comm::PvdDataStream&		mStream;
	PvdMetaDataBinding& mBinding;
	BufferRegistrar&	mRegistrar;
	PxScene&			mScene;
	CreateOp( physx::debugger::comm::PvdDataStream& str, PvdMetaDataBinding& bind, BufferRegistrar& reg, PxScene&		scene )
		: mStream( str ), mBinding( bind ), mRegistrar(reg), mScene( scene ) {}
	template<typename TDataType>
	void operator()( const TDataType& dtype ) {	mBinding.createInstance( mStream, dtype, mScene, mRegistrar ); }
	void operator()( const PxArticulationLink&) {}
#if PX_USE_PARTICLE_SYSTEM_API
	void operator()( const PxParticleSystem& dtype ) { mBinding.createInstance( mStream, dtype, mScene ); }
	void operator()( const PxParticleFluid& dtype ) { mBinding.createInstance( mStream, dtype, mScene ); }
#endif
};

struct UpdateOp
{
	CreateOp& operator=( const CreateOp& );
	physx::debugger::comm::PvdDataStream&		mStream;
	PvdMetaDataBinding& mBinding;
	UpdateOp( physx::debugger::comm::PvdDataStream& str, PvdMetaDataBinding& bind )
		: mStream( str ), mBinding( bind ) {}
	template<typename TDataType>
	void operator()( const TDataType& dtype ) {	mBinding.sendAllProperties( mStream, dtype ); }
private:
	UpdateOp& operator=(const UpdateOp&);
};

struct DestroyOp
{
	CreateOp& operator=( const CreateOp& );
	physx::debugger::comm::PvdDataStream&		mStream;
	PvdMetaDataBinding& mBinding;
	PxScene&			mScene;
	DestroyOp( physx::debugger::comm::PvdDataStream& str, PvdMetaDataBinding& bind, PxScene&		scene )
		: mStream( str ), mBinding( bind ), mScene( scene ) {}
	template<typename TDataType>
	void operator()( const TDataType& dtype ) {	mBinding.destroyInstance( mStream, dtype, mScene ); }
	void operator()( const PxArticulationLink& dtype ) { mBinding.destroyInstance( mStream, dtype ); }
#if PX_USE_PARTICLE_SYSTEM_API
	void operator()( const PxParticleSystem& dtype ) { mBinding.destroyInstance( mStream, dtype, mScene ); }
	void operator()( const PxParticleFluid& dtype ) { mBinding.destroyInstance( mStream, dtype, mScene ); }
#endif

private:
	DestroyOp& operator=(const DestroyOp&);
};

bool SceneVisualDebugger::isInstanceValid(void* instance)
{
	if ( isConnectedAndSendingDebugInformation() == false ) return false;
	return mPvdDataStream->isInstanceValid(instance);
}

void SceneVisualDebugger::createPvdInstanceIfInvalid(const PxActor* actor)
{
	if ( isConnectedAndSendingDebugInformation() == false ) return;
	if(actor->getType() == PxActorType::eARTICULATION_LINK)
	{
		VisualDebugger& sdkPvd = getSdkPvd();
		if(mPvdDataStream->isInstanceValid(actor) == false)
			mMetaDataBinding.createInstance(*mPvdDataStream, *(const PxArticulationLink* )actor, sdkPvd );
	}
}

void SceneVisualDebugger::createPvdInstance(const PxActor* scbActor)
{
	if ( isConnectedAndSendingDebugInformation() == false ) return;
	VisualDebugger& sdkPvd = getSdkPvd();
	PxScene* theScene = mScbScene.getPxScene();
	ActorTypeOperation( scbActor, CreateOp( *mPvdDataStream, mMetaDataBinding, sdkPvd, *theScene ) );
}

void SceneVisualDebugger::updatePvdProperties(const PxActor* scbActor)
{
	if ( isConnectedAndSendingDebugInformation() == false ) return;
	ActorTypeOperation( scbActor, UpdateOp( *mPvdDataStream, mMetaDataBinding ) );
}

void SceneVisualDebugger::releasePvdInstance(const PxActor* scbActor)
{
	if ( isConnectedAndSendingDebugInformation() == false ) return;
	//VisualDebugger& sdkPvd = NpPhysics::getInstance().getPhysics()->getVisualDebugger();
	PxScene* theScene = mScbScene.getPxScene();
	ActorTypeOperation( scbActor, DestroyOp( *mPvdDataStream, mMetaDataBinding, *theScene ) );
	//The flush is here because when memory allocation systems have aggressive reuse policies
	//then we have issues.  The one case that reproduces some of those issues reliably is
	//SdkUnitTests --gtest_filter="*.createRandomConvex"
	//The address of the rigiddynamic is reused for the convex mesh.  Since the npphysics data stream
	//was flush but this wasn't the release mechanism for the rigiddynamic looked to PVD like we were
	//trying to remove shapes from a newly created convex mesh.
	//These considerations are extremely important where people are streaming information
	//into and out of a running simulation and they are very very difficult to keep correct.
	mPvdDataStream->flush();
}


void SceneVisualDebugger::createPvdInstance(Scb::Actor* scbActor ) { createPvdInstance( getPxActor( scbActor ) ); }
void SceneVisualDebugger::updatePvdProperties(Scb::Actor* scbActor) { updatePvdProperties( getPxActor( scbActor ) ); }
void SceneVisualDebugger::releasePvdInstance(Scb::Actor* scbActor) { releasePvdInstance( getPxActor( scbActor ) ); }

template<typename TOperator>
inline void BodyTypeOperation( Scb::Body* scbBody, TOperator op )
{
	bool isArticulationLink = scbBody->getActorType() == PxActorType::eARTICULATION_LINK;
	if(isArticulationLink)
	{
		NpArticulationLink* link( getNpArticulationLink( scbBody ) );
		op( *static_cast<PxArticulationLink*>( link ) );
	}
	else
	{
		NpRigidDynamic* npRigidDynamic = getNpRigidDynamic(scbBody);
		op( *static_cast<PxRigidDynamic*>( npRigidDynamic ) );
	}
}

void SceneVisualDebugger::createPvdInstance(Scb::Body* scbBody)
{
	if ( isConnectedAndSendingDebugInformation() == false ) return;
	VisualDebugger& sdkPvd = getSdkPvd();
	PxScene* theScene = mScbScene.getPxScene();
	bool isArticulationLink = scbBody->getActorType() == PxActorType::eARTICULATION_LINK;
	if ( !isArticulationLink )
		BodyTypeOperation( scbBody, CreateOp( *mPvdDataStream, mMetaDataBinding, sdkPvd, *theScene ) );
}

void SceneVisualDebugger::updatePvdProperties(Scb::Body* scbBody)
{
	if ( isConnectedAndSendingDebugInformation() == false ) return;
	BodyTypeOperation( scbBody, UpdateOp( *mPvdDataStream, mMetaDataBinding ) );
}

void SceneVisualDebugger::updateKinematicTarget( Scb::Body* scbBody, const PxTransform& target )
{
	if ( isConnectedAndSendingDebugInformation() == false ) return;
	NpRigidDynamic* npRigidDynamic = getNpRigidDynamic(scbBody);
	mPvdDataStream->setPropertyValue( npRigidDynamic, "KinematicTarget", target );
}

void SceneVisualDebugger::createPvdInstance(Scb::RigidStatic* scbRigidStatic)
{
	if ( isConnectedAndSendingDebugInformation() == false ) return;
	VisualDebugger& sdkPvd = getSdkPvd();
	PxScene* theScene = mScbScene.getPxScene();
	PxRigidStatic* npRigidStatic = getNpRigidStatic(scbRigidStatic);
	mMetaDataBinding.createInstance( *mPvdDataStream, *npRigidStatic, *theScene, sdkPvd );
}


void SceneVisualDebugger::updatePvdProperties(Scb::RigidStatic* scbRigidStatic)
{	
	UPDATE_PVD_PROPERTIES_CHECK();
	PxRigidStatic& rs( *getNpRigidStatic( scbRigidStatic ) );
	mMetaDataBinding.sendAllProperties( *mPvdDataStream, rs );
}

void SceneVisualDebugger::releasePvdInstance(Scb::RigidObject* scbRigidObject) 
{ 
	releasePvdInstance( getPxActor( scbRigidObject ) ); 
}

void SceneVisualDebugger::createPvdInstance(const Scb::Shape* scbShape, PxActor& pxActor)
{
	UPDATE_PVD_PROPERTIES_CHECK();
	PxShape* npShape = getNpShape(scbShape);
	VisualDebugger& sdkPvd = getSdkPvd();
	mMetaDataBinding.createInstance( *mPvdDataStream, *npShape,  static_cast<PxRigidActor&>( pxActor ), sdkPvd );
}

void SceneVisualDebugger::updatePvdProperties(const Scb::Shape* scbShape)
{
	UPDATE_PVD_PROPERTIES_CHECK();
	mMetaDataBinding.sendAllProperties( *mPvdDataStream, *getNpShape( const_cast<Scb::Shape*>( scbShape ) ) );
}


void SceneVisualDebugger::releaseAndRecreateGeometry( const Scb::Shape* scbShape )
{
	UPDATE_PVD_PROPERTIES_CHECK();
	VisualDebugger& sdkPvd = getSdkPvd();
	mMetaDataBinding.releaseAndRecreateGeometry( *mPvdDataStream, *getNpShape( const_cast<Scb::Shape*>( scbShape ) ), NpPhysics::getInstance(), sdkPvd );
}

void SceneVisualDebugger::updateMaterials(const Scb::Shape* scbShape)
{
	UPDATE_PVD_PROPERTIES_CHECK();
	VisualDebugger& sdkPvd = getSdkPvd();
	mMetaDataBinding.updateMaterials( *mPvdDataStream, *getNpShape( const_cast<Scb::Shape*>( scbShape ) ), sdkPvd );
}
void SceneVisualDebugger::releasePvdInstance(const Scb::Shape* scbShape, PxActor& pxActor)
{
	UPDATE_PVD_PROPERTIES_CHECK();
	PxShape* npShape = getNpShape(scbShape);
	mMetaDataBinding.destroyInstance( *mPvdDataStream, *npShape,  static_cast<PxRigidActor&>( pxActor ) );

	PxU32 numMaterials = npShape->getNbMaterials();
	PX_ALLOCA(materialPtr, PxMaterial*, numMaterials);
	npShape->getMaterials( materialPtr, numMaterials );

	for ( PxU32 idx = 0; idx < numMaterials; ++idx )
	{
		releasePvdInstance(&(static_cast<NpMaterial*>( materialPtr[idx] )->getScMaterial()));
	}
}
void  SceneVisualDebugger::createPvdInstance(const Sc::MaterialCore* mat)//(const Scb::Material* scbMat)
{
	UPDATE_PVD_PROPERTIES_CHECK();
	VisualDebugger& sdkPvd = getSdkPvd();
	const PxMaterial* theMaterial( mat->getNxMaterial() );
	sdkPvd.increaseReference( theMaterial );
}

void SceneVisualDebugger::updatePvdProperties(const Sc::MaterialCore* mat)//( const Scb::Material* material )
{
	UPDATE_PVD_PROPERTIES_CHECK();
	mMetaDataBinding.sendAllProperties( *mPvdDataStream, *mat->getNxMaterial() );
}


void SceneVisualDebugger::releasePvdInstance(const Sc::MaterialCore* mat)//(const Scb::Material* scbMat)
{
	UPDATE_PVD_PROPERTIES_CHECK();
	VisualDebugger& sdkPvd = getSdkPvd();
	sdkPvd.decreaseReference( mat->getNxMaterial() );
}

void SceneVisualDebugger::createPvdInstance(Scb::Articulation* articulation)
{
	UPDATE_PVD_PROPERTIES_CHECK();
	VisualDebugger& sdkPvd = getSdkPvd();
	PxScene* theScene = mScbScene.getPxScene();
	NpArticulation* npArticulation = getNpArticulation(articulation);
	mMetaDataBinding.createInstance( *mPvdDataStream, *npArticulation, *theScene, sdkPvd );
}


void SceneVisualDebugger::updatePvdProperties(Scb::Articulation* articulation)
{
	UPDATE_PVD_PROPERTIES_CHECK();
	NpArticulation* npArticulation = getNpArticulation(articulation);
	mMetaDataBinding.sendAllProperties( *mPvdDataStream, *npArticulation );
}


void SceneVisualDebugger::releasePvdInstance(Scb::Articulation* articulation)
{
	UPDATE_PVD_PROPERTIES_CHECK();
	PxScene* theScene = mScbScene.getPxScene();
	NpArticulation* npArticulation = getNpArticulation(articulation);
	mMetaDataBinding.destroyInstance( *mPvdDataStream, *npArticulation, *theScene );
}

void SceneVisualDebugger::updatePvdProperties(Scb::ArticulationJoint* articulationJoint)
{
	UPDATE_PVD_PROPERTIES_CHECK();
	NpArticulationJoint* joint = getNpArticulationJoint( articulationJoint );
	mMetaDataBinding.sendAllProperties( *mPvdDataStream, *joint );
}

#if PX_USE_PARTICLE_SYSTEM_API
void SceneVisualDebugger::createPvdInstance(Scb::ParticleSystem* scbParticleSys)
{
	UPDATE_PVD_PROPERTIES_CHECK();
	createPvdInstance(getPxActor(scbParticleSys)); 
}

template<typename TPropertyType>
void SceneVisualDebugger::sendArray(const void* instance, const char* propName, const Cm::BitMap* bitMap, PxU32 nbValidParticles, PxStrideIterator<const TPropertyType>& iterator)
{			
		PX_ASSERT( nbValidParticles > 0);
		if(!iterator.ptr() )
			return;
		
		// setup the pvd array  PxParticleFlags
		debugger::DataRef<const PxU8> propData;		
		Array<PxU8> mTempU8Array;
		mTempU8Array.resize(nbValidParticles * sizeof(TPropertyType));
		TPropertyType* tmpArray  = reinterpret_cast<TPropertyType*>(mTempU8Array.begin());
		propData = debugger::DataRef<const PxU8>( mTempU8Array.begin(), mTempU8Array.size() );
	
		PxU32 tIdx = 0;
		Cm::BitMap::Iterator it(*bitMap);
		for(PxU32 index = it.getNext(); index != Cm::BitMap::Iterator::DONE; index = it.getNext())
		{
			tmpArray[tIdx++] = iterator[index];	
		}	
		PX_ASSERT(tIdx == nbValidParticles);
	
		mPvdDataStream->setPropertyValue( instance, propName, propData, debugger::getPvdNamespacedNameForType<TPropertyType>() );

}

void SceneVisualDebugger::sendStateDatas(Sc::ParticleSystemCore* psCore)
{
	UPDATE_PVD_PROPERTIES_CHECK();
	
	Scb::ParticleSystem* scbParticleSystem = getScbParticleSystem(psCore);
	bool doProcess = scbParticleSystem->getFlags() & PxParticleBaseFlag::eENABLED;
#if PX_SUPPORT_GPU_PHYSX
	doProcess &= (scbParticleSystem->getDeviceExclusiveAccessGpu() == NULL); 
#endif
	if(doProcess)
	{		
		Sc::ParticleSystemSim* particleSystem = psCore->getSim();
		PxvParticleSystemStateDataDesc stateData;
		particleSystem->getParticleState().getParticlesV(stateData, true, false);
		PxvParticleSystemSimDataDesc simParticleData;
		particleSystem->getSimParticleData(simParticleData, false);
	
		const PxActor* pxActor = getPxActor(scbParticleSystem);

		//mPvdDataStream->setPropertyValue( pxActor, "WorldBounds", psCore->getWorldBounds());
		mPvdDataStream->setPropertyValue( pxActor, "NbParticles", stateData.numParticles);
		mPvdDataStream->setPropertyValue( pxActor, "ValidParticleRange", stateData.validParticleRange);
		
		if(stateData.validParticleRange > 0)
		{
			mPvdDataStream->setPropertyValue( pxActor, "ValidParticleBitmap", stateData.bitMap->getWords(),  (stateData.validParticleRange >> 5)+1 );
			sendArray<PxVec3>(pxActor, "Positions", stateData.bitMap, stateData.numParticles, stateData.positions);
			sendArray<PxVec3>(pxActor, "Velocities", stateData.bitMap, stateData.numParticles, stateData.velocities);
			sendArray<PxF32>(pxActor, "RestOffsets", stateData.bitMap, stateData.numParticles, stateData.restOffsets);
			sendArray<PxVec3>(pxActor, "CollisionNormals", stateData.bitMap, stateData.numParticles, simParticleData.collisionNormals);
			sendArray<PxF32>(pxActor, "Densities", stateData.bitMap, stateData.numParticles, simParticleData.densities);
			//todo: twoway data if need more particle retrieval

			{//send PxParticleFlags, we have PxvParticleFlags here
				debugger::DataRef<const PxU8> propData;		
				Array<PxU8> mTempU8Array;
				mTempU8Array.resize(stateData.numParticles * sizeof(PxU16));
				PxU16* tmpArray  = reinterpret_cast<PxU16*>(mTempU8Array.begin());
				propData = debugger::DataRef<const PxU8>( mTempU8Array.begin(), mTempU8Array.size() );
	
				PxU32 tIdx = 0;
				PxStrideIterator<const PxvParticleFlags>& iterator = stateData.flags;
				Cm::BitMap::Iterator it(*stateData.bitMap);
				for(PxU32 index = it.getNext(); index != Cm::BitMap::Iterator::DONE; index = it.getNext())
				{
					tmpArray[tIdx++] = iterator[index].api;	
				}
					
				mPvdDataStream->setPropertyValue( pxActor, "Flags", propData, debugger::getPvdNamespacedNameForType<PxU16>() );
			}
		}					
	}	
}

void SceneVisualDebugger::updatePvdProperties(Scb::ParticleSystem* scbParticleSys)
{
	updatePvdProperties( getPxActor(scbParticleSys) );
}

void SceneVisualDebugger::releasePvdInstance(Scb::ParticleSystem* scbParticleSys)
{
	releasePvdInstance( getPxActor(scbParticleSys) );
}
#endif // PX_USE_PARTICLE_SYSTEM_API

#if PX_USE_CLOTH_API
static inline PxCloth* toPx( Scb::Cloth* cloth )
{
	NpCloth* realCloth( backptr( cloth ) );
	return static_cast<PxCloth*>( realCloth );
}

void SceneVisualDebugger::createPvdInstance(Scb::Cloth* scbCloth)
{
	UPDATE_PVD_PROPERTIES_CHECK();
	NpCloth* realCloth( backptr( scbCloth ) );
	VisualDebugger& sdkPvd = getSdkPvd();
	PxScene* theScene = mScbScene.getPxScene();
	mMetaDataBinding.createInstance( *mPvdDataStream, *realCloth, *theScene, sdkPvd );
	mPvdDataStream->flush();
}

void SceneVisualDebugger::sendSimpleProperties( Scb::Cloth* cloth )
{
	if ( isConnectedAndSendingDebugInformation() )
		mMetaDataBinding.sendSimpleProperties( *mPvdDataStream, *toPx( cloth ) );
}
void SceneVisualDebugger::sendMotionConstraints( Scb::Cloth* cloth )
{
	if ( isConnectedAndSendingDebugInformation() )
		mMetaDataBinding.sendMotionConstraints( *mPvdDataStream, *toPx( cloth ) );
}
void SceneVisualDebugger::sendSelfCollisionIndices( Scb::Cloth* cloth)
{
	if ( isConnectedAndSendingDebugInformation() )
		mMetaDataBinding.sendSelfCollisionIndices( *mPvdDataStream, *toPx( cloth ) );
}
void SceneVisualDebugger::sendRestPositions( Scb::Cloth* cloth )
{
	if ( isConnectedAndSendingDebugInformation() )
		mMetaDataBinding.sendRestPositions( *mPvdDataStream, *toPx( cloth ) );
}
void SceneVisualDebugger::sendSeparationConstraints( Scb::Cloth* cloth )
{
	if ( isConnectedAndSendingDebugInformation() )
		mMetaDataBinding.sendSeparationConstraints( *mPvdDataStream, *toPx( cloth ) );
}
void SceneVisualDebugger::sendCollisionSpheres( Scb::Cloth* cloth )
{
	if ( isConnectedAndSendingDebugInformation() )
		mMetaDataBinding.sendCollisionSpheres( *mPvdDataStream, *toPx( cloth ) );
}

void SceneVisualDebugger::sendCollisionCapsules( Scb::Cloth* cloth )
{
	if ( isConnectedAndSendingDebugInformation() )
		mMetaDataBinding.sendCollisionSpheres( *mPvdDataStream, *toPx( cloth ), true );
}

void SceneVisualDebugger::sendCollisionTriangles( Scb::Cloth* cloth )
{
	if (isConnectedAndSendingDebugInformation() )
		mMetaDataBinding.sendCollisionTriangles( *mPvdDataStream, *toPx( cloth ) );
}
void SceneVisualDebugger::sendParticleAccelerations( Scb::Cloth* cloth )
{
	if (isConnectedAndSendingDebugInformation() )
		mMetaDataBinding.sendParticleAccelerations( *mPvdDataStream, *toPx( cloth ) );
}
void SceneVisualDebugger::sendVirtualParticles( Scb::Cloth* cloth )
{
	if ( isConnectedAndSendingDebugInformation() )
		mMetaDataBinding.sendVirtualParticles( *mPvdDataStream, *toPx( cloth ) );
}

void SceneVisualDebugger::releasePvdInstance(Scb::Cloth* cloth)
{
	UPDATE_PVD_PROPERTIES_CHECK();
	mMetaDataBinding.destroyInstance( *mPvdDataStream, *toPx(cloth), *mScbScene.getPxScene() );
	mPvdDataStream->flush();
}
#endif // PX_USE_CLOTH_API

bool SceneVisualDebugger::updateConstraint(const Sc::ConstraintCore& scConstraint, PxU32 updateType)
{
	if ( isConnectedAndSendingDebugInformation() == false ) return false;
	PxConstraintConnector* conn;
	bool success = false;
	if( (conn = scConstraint.getPxConnector()) != NULL
		&& isConnectedAndSendingDebugInformation() )
	{
		success = conn->updatePvdProperties(*mPvdDataStream, scConstraint.getPxConstraint(), PxPvdUpdateType::Enum(updateType));
	}
	return success;
}


void SceneVisualDebugger::createPvdInstance(Scb::Constraint* constraint)
{
	UPDATE_PVD_PROPERTIES_CHECK();
	updateConstraint(constraint->getScConstraint(), PxPvdUpdateType::CREATE_INSTANCE);
}


void SceneVisualDebugger::updatePvdProperties(Scb::Constraint* constraint)
{
	UPDATE_PVD_PROPERTIES_CHECK();
	updateConstraint(constraint->getScConstraint(), PxPvdUpdateType::UPDATE_ALL_PROPERTIES);
}


void SceneVisualDebugger::releasePvdInstance(Scb::Constraint* constraint)
{
	UPDATE_PVD_PROPERTIES_CHECK();
	PxConstraintConnector* conn;
	Sc::ConstraintCore& scConstraint = constraint->getScConstraint();
	if( (conn = scConstraint.getPxConnector()) != NULL )
	{
		conn->updatePvdProperties(*mPvdDataStream, scConstraint.getPxConstraint(), PxPvdUpdateType::RELEASE_INSTANCE);
	}
}

void SceneVisualDebugger::createPvdInstance(Scb::Aggregate* aggregate)
{	
	UPDATE_PVD_PROPERTIES_CHECK();
	NpAggregate* npAggregate = getNpAggregate( aggregate );
	VisualDebugger& sdkPvd = getSdkPvd();
	PxScene* theScene = mScbScene.getPxScene();
	mMetaDataBinding.createInstance( *mPvdDataStream, *npAggregate, *theScene, sdkPvd );
	mPvdDataStream->flush();
}

void SceneVisualDebugger::updatePvdProperties(Scb::Aggregate* aggregate)
{
	UPDATE_PVD_PROPERTIES_CHECK();
	NpAggregate* npAggregate = getNpAggregate( aggregate );
	mMetaDataBinding.sendAllProperties( *mPvdDataStream, *npAggregate );
}

void SceneVisualDebugger::attachAggregateActor(Scb::Aggregate* aggregate, Scb::Actor* actor)
{
	UPDATE_PVD_PROPERTIES_CHECK();
	NpAggregate* npAggregate = getNpAggregate( aggregate );
	mMetaDataBinding.attachAggregateActor( *mPvdDataStream, *npAggregate, *getPxActor(actor) );
}

void SceneVisualDebugger::detachAggregateActor(Scb::Aggregate* aggregate, Scb::Actor* actor)
{
	UPDATE_PVD_PROPERTIES_CHECK();
	NpAggregate* npAggregate = getNpAggregate( aggregate );
	mMetaDataBinding.detachAggregateActor( *mPvdDataStream, *npAggregate, *getPxActor(actor) );
}

void SceneVisualDebugger::releasePvdInstance(Scb::Aggregate* aggregate)
{
	UPDATE_PVD_PROPERTIES_CHECK();
	NpAggregate* npAggregate = getNpAggregate( aggregate );
	mMetaDataBinding.destroyInstance( *mPvdDataStream, *npAggregate, *mScbScene.getPxScene() );
	mPvdDataStream->flush();
}

void SceneVisualDebugger::updateContacts()
{
	if(!isConnectedAndSendingDebugInformation())
		return;

	// if contacts are disabled, send empty array and return
	VisualDebugger& sdkPvd = getSdkPvd();
	const PxScene* theScene( mScbScene.getPxScene() );
	if(!sdkPvd.getTransmitContactsFlag())
	{
		mMetaDataBinding.sendContacts( *mPvdDataStream, *theScene );
		return;
	}

	CM_PROFILE_ZONE_WITH_SUBSYSTEM( mScbScene,PVD,updateContacts );
	Sc::ContactIterator contactIter;
	mScbScene.getScScene().initContactsIterator(contactIter);
	mMetaDataBinding.sendContacts( *mPvdDataStream, *theScene, contactIter );
}


void SceneVisualDebugger::updateSceneQueries()
{
	if(!isConnectedAndSendingDebugInformation())
		return;

	// if contacts are disabled, send empty array and return
	VisualDebugger& sdkPvd = getSdkPvd();
	const PxScene* theScene( mScbScene.getPxScene() );

	mMetaDataBinding.sendSceneQueries( *mPvdDataStream, *theScene, sdkPvd.getTransmitSceneQueriesFlag() );
}


void SceneVisualDebugger::visualize( PxArticulationLink& link )
{
#if PX_ENABLE_DEBUG_VISUALIZATION
	NpArticulationLink& npLink = static_cast<NpArticulationLink&>( link );
	const void* itemId = npLink.getInboundJoint();
	if ( itemId != NULL && mUserRenderer != NULL )
	{
		PvdConstraintVisualizer viz( itemId, *mUserRenderer );
		npLink.visualizeJoint( viz );
	}
#endif
}

void SceneVisualDebugger::updateJoints()
{
	if(!isConnected())
		return;
	
	VisualDebugger& sdkPvd = getSdkPvd();

	if(isConnectedAndSendingDebugInformation())
	{
		const bool visualizeJoints = sdkPvd.isVisualizingConstraints();
		if ( visualizeJoints && mUserRenderer == NULL )
		{
			NpPhysics& physics( NpPhysics::getInstance() );
			physx::debugger::comm::PvdConnectionManager& mgr( *physics.getPvdConnectionManager() );
			physx::debugger::comm::PvdConnection* connection( mgr.getAndAddRefCurrentConnection() );
			if ( connection )
			{
				mUserRenderer = &connection->createRenderer();
				mUserRenderer->addRef();
				connection->release();
			}
		}

			// joints
		{
			CM_PROFILE_ZONE_WITH_SUBSYSTEM( mScbScene,PVD,updateJoints );
			Sc::ConstraintCore** constraints = mScbScene.getScScene().getConstraints();
			PxU32 nbConstraints = mScbScene.getScScene().getNbConstraints();
			PxI64 constraintCount = 0;

			for(PxU32 i = 0; i<nbConstraints; i++)
			{
				Sc::ConstraintCore* constraint = constraints[i];
				PxPvdUpdateType::Enum updateType = getNpConstraint(constraint)->isDirty() ? PxPvdUpdateType::UPDATE_ALL_PROPERTIES : PxPvdUpdateType::UPDATE_SIM_PROPERTIES;
				updateConstraint(*constraint, updateType);
				PxConstraintConnector* conn = constraint->getPxConnector();
				//visualization is updated here
				{
					PxU32 typeId = 0;
					void* joint = NULL;
					if ( conn != NULL )
						joint = conn->getExternalReference( typeId );
					// visualize:
					Sc::ConstraintSim* sim = constraint->getSim();
					if(visualizeJoints 
						&& 
						sim 
						&& sim->getConstantsLL() 
						&& joint
						&& constraint->getVisualize()  )
					{
						Sc::BodySim* b0 = sim->getBody(0);
						Sc::BodySim* b1 = sim->getBody(1);
						PxTransform t0 = b0 ? b0->getBody2World() : PxTransform(PxIdentity);
						PxTransform t1 = b1 ? b1->getBody2World() : PxTransform(PxIdentity);
						PvdConstraintVisualizer viz( joint, *mUserRenderer );
						(*constraint->getVisualize())(viz, sim->getConstantsLL(), t0, t1, 0xffffFFFF);
					}
				}
				++constraintCount;
			}
			if ( mUserRenderer != NULL )
				mUserRenderer->flushRenderEvents();
			CM_PROFILE_VALUE( mScbScene,PVD,updateJoints, constraintCount );
		}
	}
}

void SceneVisualDebugger::flushPendingCommands()
{
	if(!isConnectedAndSendingDebugInformation())
		return;

	PX_ASSERT(mPvdDataStream);

	mPvdDataStream->flushPvdCommand();
}
} // namespace Pvd

}

#endif  // PX_SUPPORT_VISUAL_DEBUGGER
