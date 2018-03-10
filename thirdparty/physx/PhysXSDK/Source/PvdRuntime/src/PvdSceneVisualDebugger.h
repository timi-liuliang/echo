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


#ifndef PVD_SCENEVISUALDEBUGGER_H
#define PVD_SCENEVISUALDEBUGGER_H

#if PX_SUPPORT_VISUAL_DEBUGGER

#include "PsUserAllocated.h"
#include "PsArray.h"
#include "CmPhysXCommon.h"
#include "PxMetaDataPvdBinding.h"
#include "CmBitMap.h"

namespace physx { namespace debugger {
	namespace comm {
		class PvdDataStream;
	}
	namespace renderer
	{
		class PvdUserRenderer;
	}
}}

namespace physx
{

class PxSimulationStatistics;
class PxGeometry;
template<typename T>
  class PxStrideIterator;

namespace profile
{	
	class PxProfileZone;
}

namespace Scb
{
	class Scene;
	class Actor;
	class Body;
	class RigidStatic;
	class RigidObject;
	class Shape;
	//class Material;
	class ParticleSystem;
	class Constraint;
	class Articulation;
	class ArticulationJoint;
	class Cloth;
	class Aggregate;
}

namespace Sc
{
	class MaterialCore;
	class RigidCore;
	class ConstraintCore;
	class ParticleSystemCore;
}

namespace Pvd
{

struct SceneGroups
{
	enum Enum
	{
		RigidDynamics = 1, // 0 is reserved for SimulationStatistics
		RigidStatics,
		Joints,
		Articulations,
		ParticleSystems,
		Materials,
		ProfileZones,
		Cloths,
		NUM_ELEMENTS
	};
};

//////////////////////////////////////////////////////////////////////////
/*!
RemoteDebug supplies functionality for writing debug info to a stream
to be read by the remote debugger application.
*/
//////////////////////////////////////////////////////////////////////////
class SceneVisualDebugger: public Ps::UserAllocated, public PvdVisualizer
{
public:
	SceneVisualDebugger(Scb::Scene&);
	virtual ~SceneVisualDebugger();
	
	// finish with the scene
	void	detach();

	void setPvdConnection(physx::debugger::comm::PvdDataStream* c, PxU32 inConnectionType);
	physx::debugger::comm::PvdDataStream* getPvdDataStream() const;


	// internal methods
	void sendClassDescriptions();
	
	// useCachedStaus: 
	//    1> When useCachedStaus is false, isConnected() checks the lowlevel network status.
	//       This can be slow because it needs to lock the lowlevel network stream. If isConnected() is 
	//       called frequently, the expense of locking can be significant.
	//    2> When useCachedStaus is true, isConnected() checks the highlevel cached status with atomic access.
	//       It is faster than locking, but the status may be different from the lowlevel network with latency of up to one frame.
	//       The reason for this is that the cached status is changed inside PvdConnectionHandler listener, which is not called immediately
	//       when the lowlevel conntion status changes. 
	bool isConnected(bool useCachedStatus = false) const;

	bool isConnectedAndSendingDebugInformation();

	void sendEntireScene();
	void frameStart(PxReal simulateElapsedTime);
	void frameEnd();
	void originShift(PxVec3 shift);

	void createPvdInstance();
	void updatePvdProperties();
	void releasePvdInstance();

	bool isInstanceValid(void* instance);
	
	void createPvdInstanceIfInvalid(const PxActor* actor); //temporary for articulation link
	void createPvdInstance(const PxActor* actor); // temporary for deformables and particle systems
	void updatePvdProperties(const PxActor* actor);
	void releasePvdInstance(const PxActor* actor); // temporary for deformables and particle systems

	void createPvdInstance(Scb::Actor* scbActor); // temporary for deformables and particle systems
	void updatePvdProperties(Scb::Actor* scbActor);
	void releasePvdInstance(Scb::Actor* scbActor); // temporary for deformables and particle systems

	void createPvdInstance(Scb::Body* scbBody);
	void updatePvdProperties(Scb::Body* scbBody);
	void updateKinematicTarget(Scb::Body* scbBody,const PxTransform& p);

	void createPvdInstance(Scb::RigidStatic* scbRigidStatic);
	void updatePvdProperties(Scb::RigidStatic* scbRigidStatic);

	void releasePvdInstance(Scb::RigidObject* scbRigidObject);

	
	void createPvdInstance(const Scb::Shape* scbShape, PxActor& owner);
	void updateMaterials(const Scb::Shape* scbShape);
	void updatePvdProperties(const Scb::Shape* scbShape);
	void releaseAndRecreateGeometry( const Scb::Shape* scbShape );
	void releasePvdInstance(const Scb::Shape* scbShape, PxActor& owner);

	//void createPvdInstance(const Scb::Material* scbMat);
	//void updatePvdProperties( const Scb::Material* material );
	//void releasePvdInstance(const Scb::Material* scbMat);

	void createPvdInstance(const Sc::MaterialCore* scbMat);
	void updatePvdProperties( const Sc::MaterialCore* material );
	void releasePvdInstance(const Sc::MaterialCore* scbMat);

	void createPvdInstance(Scb::Articulation* articulation);
	void updatePvdProperties(Scb::Articulation* articulation);
	void releasePvdInstance(Scb::Articulation* articulation);

	void updatePvdProperties(Scb::ArticulationJoint* articulationJoint);

	void createPvdInstance(Scb::ParticleSystem* scbParticleSys);
	void updatePvdProperties(Scb::ParticleSystem* scbParticleSys);
	void releasePvdInstance(Scb::ParticleSystem* scbParticleSys);
	template<typename TPropertyType>
	void sendArray(const void* instance, const char* propName, const Cm::BitMap* bitMap, PxU32 nbValidParticles, PxStrideIterator<const TPropertyType>& iterator);
	void sendStateDatas(Sc::ParticleSystemCore* psCore);
	
	void createPvdInstance(Scb::Cloth* scbCloth);
	void sendSimpleProperties( Scb::Cloth* cloth );
	void sendMotionConstraints( Scb::Cloth* cloth );
	void sendCollisionSpheres( Scb::Cloth* cloth );
	void sendCollisionCapsules( Scb::Cloth* cloth );
	void sendCollisionTriangles( Scb::Cloth* cloth );
	void sendVirtualParticles( Scb::Cloth* cloth );
	void sendSeparationConstraints( Scb::Cloth* cloth );
	void sendParticleAccelerations( Scb::Cloth* cloth );
	void sendSelfCollisionIndices( Scb::Cloth* cloth );
	void sendRestPositions( Scb::Cloth* cloth );
	void releasePvdInstance(Scb::Cloth* scbCloth);

	void createPvdInstance(Scb::Constraint* constraint);
	void updatePvdProperties(Scb::Constraint* constraint);
	void releasePvdInstance(Scb::Constraint* constraint);

	void createPvdInstance(Scb::Aggregate* aggregate);
	void updatePvdProperties(Scb::Aggregate* aggregate);
	void attachAggregateActor(Scb::Aggregate* aggregate, Scb::Actor* actor);
	void detachAggregateActor(Scb::Aggregate* aggregate, Scb::Actor* actor);
	void releasePvdInstance(Scb::Aggregate* aggregate);

	void updateContacts();
	void updateSceneQueries();
	void setCreateContactReports(bool);

	void updateJoints();

	//PvdVisualizer
	virtual void visualize( PxArticulationLink& link );

	void flushPendingCommands();
	
private:
	SceneVisualDebugger& operator=(const SceneVisualDebugger&);
	bool updateConstraint(const Sc::ConstraintCore& scConstraint, PxU32 updateType);

	physx::debugger::comm::PvdDataStream*					mPvdDataStream;
	physx::debugger::renderer::PvdUserRenderer*		mUserRenderer;
	Scb::Scene&							mScbScene;
	Ps::Array<PxU64>					mProfileZoneIdList;
	PxU32								mConnectionType;
	PvdMetaDataBinding					mMetaDataBinding;
};

} // namespace Pvd

}

#endif // PX_SUPPORT_VISUAL_DEBUGGER

#endif // VISUALDEBUGGER_H

