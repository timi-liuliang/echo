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


#include "NpRigidDynamic.h"

using namespace physx;

NpRigidDynamic::NpRigidDynamic(const PxTransform& bodyPose)
:	NpRigidDynamicT(PxConcreteType::eRIGID_DYNAMIC, PxBaseFlag::eOWNS_MEMORY | PxBaseFlag::eIS_RELEASABLE, PxActorType::eRIGID_DYNAMIC, bodyPose)
{}

NpRigidDynamic::~NpRigidDynamic()
{
}

// PX_SERIALIZATION
void NpRigidDynamic::requires(PxProcessPxBaseCallback& c)
{
	NpRigidDynamicT::requires(c);
}

NpRigidDynamic* NpRigidDynamic::createObject(PxU8*& address, PxDeserializationContext& context)
{
	NpRigidDynamic* obj = new (address) NpRigidDynamic(PxBaseFlag::eIS_RELEASABLE);
	address += sizeof(NpRigidDynamic);	
	obj->importExtraData(context);
	obj->resolveReferences(context);
	return obj;
}
//~PX_SERIALIZATION

void NpRigidDynamic::release()
{
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));

	NpPhysics::getInstance().notifyDeletionListenersUserRelease(this, userData);

	Scb::Scene* s = mBody.getScbSceneForAPI();

	bool noSim = mBody.isSimDisabledInternally();
	// important to check the non-buffered flag because it tells what the current internal state of the object is
	// (someone might switch to non-simulation and release all while the sim is running).
	if (s && noSim)
	{
		// need to do it here because the Np-shape buffer will not be valid anymore after the release below
		// and unlike simulation objects, there is no shape buffer in the simulation controller
		getShapeManager().clearShapesOnRelease(*s, *this);
	}

	NpRigidDynamicT::release();

	// Remove from scene should happen at the end because the base class still needs to know whether the object was in the scene (implicit constraint removal etc.)
	if (s)
	{
		s->removeRigidBody(mBody, true, noSim);
		static_cast<NpScene*>(s->getPxScene())->removeFromRigidActorList(mIndex);
	}

	mBody.destroy();
}


void NpRigidDynamic::setGlobalPose(const PxTransform& pose, bool autowake)
{
	NpScene* scene = NpActor::getAPIScene(*this);

#ifdef PX_CHECKED
	if(scene)
		scene->checkPositionSanity(*this, pose, "PxRigidDynamic::setGlobalPose");
#endif

	PX_CHECK_AND_RETURN(pose.isSane(), "NpRigidDynamic::setGlobalPose: pose is not valid.");
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));

	PxTransform p = pose.getNormalized();

	if(scene)
	{
		updateDynamicSceneQueryShapes(mShapeManager, scene->getSceneQueryManagerFast());
	}

	PxTransform newPose = p;
	newPose.q.normalize(); //AM: added to fix 1461 where users read and write orientations for no reason.
	
	Scb::Body& b = getScbBodyFast();
	PxTransform body2World = newPose * b.getBody2Actor();
	b.setBody2World(body2World, false);

	if(scene && autowake && !(b.getActorFlags() & PxActorFlag::eDISABLE_SIMULATION))
		wakeUpInternal();
}


PX_FORCE_INLINE void NpRigidDynamic::setKinematicTargetInternal(const PxTransform& targetPose)
{
	// The target is actor related. Transform to body related target
	PxTransform bodyTarget = targetPose * getScbBodyFast().getBody2Actor();

	Scb::Body& b = getScbBodyFast();
	b.setKinematicTarget(bodyTarget);

	NpScene* scene = NpActor::getAPIScene(*this);
	if ((b.getFlags() & PxRigidBodyFlag::eUSE_KINEMATIC_TARGET_FOR_SCENE_QUERIES) && scene)
	{
		updateDynamicSceneQueryShapes(mShapeManager, scene->getSceneQueryManagerFast());
	}
}


void NpRigidDynamic::setKinematicTarget(const PxTransform& destination)
{
	PX_CHECK_AND_RETURN(destination.isSane(), "NpRigidDynamic::setKinematicTarget: destination is not valid.");

	NpScene* scene = NpActor::getAPIScene(*this);
	PX_UNUSED(scene);
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));

#ifdef PX_CHECKED
	if(scene)
		scene->checkPositionSanity(*this, destination, "PxRigidDynamic::setKinematicTarget");
#endif

	Scb::Body& b = getScbBodyFast();
	PX_UNUSED(b);

	//0) make sure this is kinematic
	PX_CHECK_AND_RETURN((b.getFlags() & PxRigidBodyFlag::eKINEMATIC), "RigidDynamic::setKinematicTarget: Body must be kinematic!");
	PX_CHECK_AND_RETURN(scene, "RigidDynamic::setKinematicTarget: Body must be in a scene!");
	PX_CHECK_AND_RETURN(!(b.getActorFlags() & PxActorFlag::eDISABLE_SIMULATION), "RigidDynamic::setKinematicTarget: Not allowed if PxActorFlag::eDISABLE_SIMULATION is set!");
	
	setKinematicTargetInternal(destination.getNormalized());
}


bool NpRigidDynamic::getKinematicTarget(PxTransform& target)
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));

	Scb::Body& b = getScbBodyFast();
	if (b.getFlags() & PxRigidBodyFlag::eKINEMATIC)
	{
		PxTransform bodyTarget;
		if (b.getKinematicTarget(bodyTarget))
		{
			// The internal target is body related. Transform to actor related target
			target = bodyTarget * b.getBody2Actor().getInverse();

			return true;
		}
		else
			return false;
	}
	else
		return false;
}


void NpRigidDynamic::setCMassLocalPose(const PxTransform& pose)
{
	PX_CHECK_AND_RETURN(pose.isSane(), "NpRigidDynamic::setCMassLocalPose pose is not valid.");

	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));

	PxTransform p = pose.getNormalized();

	PxTransform oldBody2Actor = getScbBodyFast().getBody2Actor();

	NpRigidDynamicT::setCMassLocalPoseInternal(p);

	Scb::Body& b = getScbBodyFast();
	if (b.getFlags() & PxRigidBodyFlag::eKINEMATIC)
	{
		PxTransform bodyTarget;
		if (b.getKinematicTarget(bodyTarget))
		{
			PxTransform actorTarget = bodyTarget * oldBody2Actor.getInverse();  // get old target pose for the actor from the body target
			setKinematicTargetInternal(actorTarget);
		}
	}
}


void NpRigidDynamic::setLinearDamping(PxReal linearDamping)
{
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));
	PX_CHECK_AND_RETURN(PxIsFinite(linearDamping), "NpRigidDynamic::setLinearDamping: invalid float");
	PX_CHECK_AND_RETURN(linearDamping >=0, "RigidDynamic::setLinearDamping: The linear damping must be nonnegative!");


	getScbBodyFast().setLinearDamping(linearDamping);
}


PxReal NpRigidDynamic::getLinearDamping() const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));

	return getScbBodyFast().getLinearDamping();
}


void NpRigidDynamic::setAngularDamping(PxReal angularDamping)
{
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));
	PX_CHECK_AND_RETURN(PxIsFinite(angularDamping), "NpRigidDynamic::setAngularDamping: invalid float");
	PX_CHECK_AND_RETURN(angularDamping>=0, "RigidDynamic::setAngularDamping: The angular damping must be nonnegative!")
	
	getScbBodyFast().setAngularDamping(angularDamping);
}


PxReal NpRigidDynamic::getAngularDamping() const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));

	return getScbBodyFast().getAngularDamping();
}


void NpRigidDynamic::setLinearVelocity(const PxVec3& velocity, bool autowake)
{
	NpScene* scene = NpActor::getAPIScene(*this);

	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));
	PX_CHECK_AND_RETURN(velocity.isFinite(), "NpRigidDynamic::setLinearVelocity: velocity is not valid.");
	PX_CHECK_AND_RETURN(!(getScbBodyFast().getFlags() & PxRigidBodyFlag::eKINEMATIC), "RigidDynamic::setLinearVelocity: Body must be non-kinematic!");
	PX_CHECK_AND_RETURN(!(getScbBodyFast().getActorFlags() & PxActorFlag::eDISABLE_SIMULATION), "RigidDynamic::setLinearVelocity: Not allowed if PxActorFlag::eDISABLE_SIMULATION is set!");
	
	Scb::Body& b = getScbBodyFast();
	b.setLinearVelocity(velocity);

	if (scene)
		wakeUpInternalNoKinematicTest(b, (!velocity.isZero()), autowake);
}


void NpRigidDynamic::setAngularVelocity(const PxVec3& velocity, bool autowake)
{
	NpScene* scene = NpActor::getAPIScene(*this);

	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));
	PX_CHECK_AND_RETURN(velocity.isFinite(), "NpRigidDynamic::setAngularVelocity: velocity is not valid.");
	PX_CHECK_AND_RETURN(!(getScbBodyFast().getFlags() & PxRigidBodyFlag::eKINEMATIC), "RigidDynamic::setAngularVelocity: Body must be non-kinematic!");
	PX_CHECK_AND_RETURN(!(getScbBodyFast().getActorFlags() & PxActorFlag::eDISABLE_SIMULATION), "RigidDynamic::setAngularVelocity: Not allowed if PxActorFlag::eDISABLE_SIMULATION is set!");

	Scb::Body& b = getScbBodyFast();
	b.setAngularVelocity(velocity);

	if (scene)
		wakeUpInternalNoKinematicTest(b, (!velocity.isZero()), autowake);
}


void NpRigidDynamic::setMaxAngularVelocity(PxReal maxAngularVelocity)
{
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));
	PX_CHECK_AND_RETURN(PxIsFinite(maxAngularVelocity), "NpRigidDynamic::setMaxAngularVelocity: invalid float");
	PX_CHECK_AND_RETURN(maxAngularVelocity>=0.0f, "RigidDynamic::setMaxAngularVelocity: threshold must be non-negative!");

	getScbBodyFast().setMaxAngVelSq(maxAngularVelocity * maxAngularVelocity);		
}


PxReal NpRigidDynamic::getMaxAngularVelocity() const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));

	return PxSqrt(getScbBodyFast().getMaxAngVelSq());
}


void NpRigidDynamic::addForce(const PxVec3& force, PxForceMode::Enum mode, bool autowake)
{
	Scb::Body& b = getScbBodyFast();

	PX_CHECK_AND_RETURN(force.isFinite(), "NpRigidDynamic::addForce: force is not valid.");
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));
	PX_CHECK_AND_RETURN(NpActor::getAPIScene(*this), "RigidDynamic::addForce: Body must be in a scene!");
	PX_CHECK_AND_RETURN(!(b.getFlags() & PxRigidBodyFlag::eKINEMATIC), "RigidDynamic::addForce: Body must be non-kinematic!");
	PX_CHECK_AND_RETURN(!(b.getActorFlags() & PxActorFlag::eDISABLE_SIMULATION), "RigidDynamic::addForce: Not allowed if PxActorFlag::eDISABLE_SIMULATION is set!");

	addSpatialForce(&force, 0, mode);

	wakeUpInternalNoKinematicTest(b, (!force.isZero()), autowake);
}


void NpRigidDynamic::addTorque(const PxVec3& torque, PxForceMode::Enum mode, bool autowake)
{
	Scb::Body& b = getScbBodyFast();

	PX_CHECK_AND_RETURN(torque.isFinite(), "NpRigidDynamic::addTorque: torque is not valid.");
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));
	PX_CHECK_AND_RETURN(NpActor::getAPIScene(*this), "RigidDynamic::addTorque: Body must be in a scene!");
	PX_CHECK_AND_RETURN(!(b.getFlags() & PxRigidBodyFlag::eKINEMATIC), "RigidDynamic::addTorque: Body must be non-kinematic!");
	PX_CHECK_AND_RETURN(!(b.getActorFlags() & PxActorFlag::eDISABLE_SIMULATION), "RigidDynamic::addTorque: Not allowed if PxActorFlag::eDISABLE_SIMULATION is set!");

	addSpatialForce(0, &torque, mode);

	wakeUpInternalNoKinematicTest(b, (!torque.isZero()), autowake);
}

void NpRigidDynamic::clearForce(PxForceMode::Enum mode)
{
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));
	PX_CHECK_AND_RETURN(NpActor::getAPIScene(*this), "RigidDynamic::clearForce: Body must be in a scene!");
	PX_CHECK_AND_RETURN(!(getScbBodyFast().getFlags() & PxRigidBodyFlag::eKINEMATIC), "RigidDynamic::clearForce: Body must be non-kinematic!");
	PX_CHECK_AND_RETURN(!(getScbBodyFast().getActorFlags() & PxActorFlag::eDISABLE_SIMULATION), "RigidDynamic::clearForce: Not allowed if PxActorFlag::eDISABLE_SIMULATION is set!");

	clearSpatialForce(mode, true, false);
}


void NpRigidDynamic::clearTorque(PxForceMode::Enum mode)
{
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));
	PX_CHECK_AND_RETURN(NpActor::getAPIScene(*this), "RigidDynamic::clearTorque: Body must be in a scene!");
	PX_CHECK_AND_RETURN(!(getScbBodyFast().getFlags() & PxRigidBodyFlag::eKINEMATIC), "RigidDynamic::clearTorque: Body must be non-kinematic!");
	PX_CHECK_AND_RETURN(!(getScbBodyFast().getActorFlags() & PxActorFlag::eDISABLE_SIMULATION), "RigidDynamic::clearTorque: Not allowed if PxActorFlag::eDISABLE_SIMULATION is set!");

	clearSpatialForce(mode, false, true);
}


bool NpRigidDynamic::isSleeping() const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));
	PX_CHECK_AND_RETURN_VAL(NpActor::getAPIScene(*this), "NpRigidDynamic::isSleeping: Body must be in a scene.", true);

	return getScbBodyFast().isSleeping();
}


void NpRigidDynamic::setSleepThreshold(PxReal threshold)
{
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));
	PX_CHECK_AND_RETURN(PxIsFinite(threshold), "NpRigidDynamic::setSleepThreshold: invalid float.");
	PX_CHECK_AND_RETURN(threshold>=0.0f, "RigidDynamic::setSleepThreshold: threshold must be non-negative!");

	getScbBodyFast().setSleepThreshold(threshold);
}


PxReal NpRigidDynamic::getSleepThreshold() const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));

	return getScbBodyFast().getSleepThreshold();
}

void NpRigidDynamic::setStabilizationThreshold(PxReal threshold)
{
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));
	PX_CHECK_AND_RETURN(PxIsFinite(threshold), "NpRigidDynamic::setSleepThreshold: invalid float.");
	PX_CHECK_AND_RETURN(threshold>=0.0f, "RigidDynamic::setSleepThreshold: threshold must be non-negative!");

	getScbBodyFast().setFreezeThreshold(threshold);
}


PxReal NpRigidDynamic::getStabilizationThreshold() const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));

	return getScbBodyFast().getFreezeThreshold();
}


void NpRigidDynamic::setWakeCounter(PxReal wakeCounterValue)
{
	Scb::Body& b = getScbBodyFast();

	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));
	PX_CHECK_AND_RETURN(PxIsFinite(wakeCounterValue), "NpRigidDynamic::setWakeCounter: invalid float.");
	PX_CHECK_AND_RETURN(wakeCounterValue>=0.0f, "RigidDynamic::setWakeCounter: wakeCounterValue must be non-negative!");
	PX_CHECK_AND_RETURN(!(b.getFlags() & PxRigidBodyFlag::eKINEMATIC), "RigidDynamic::setWakeCounter: Body must be non-kinematic!");
	PX_CHECK_AND_RETURN(!(b.getActorFlags() & PxActorFlag::eDISABLE_SIMULATION), "RigidDynamic::setWakeCounter: Not allowed if PxActorFlag::eDISABLE_SIMULATION is set!");

	b.setWakeCounter(wakeCounterValue);
}


PxReal NpRigidDynamic::getWakeCounter() const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));

	return getScbBodyFast().getWakeCounter();
}


void NpRigidDynamic::wakeUp()
{
	Scb::Body& b = getScbBodyFast();

	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));
	PX_CHECK_AND_RETURN(NpActor::getAPIScene(*this), "NpRigidDynamic::wakeUp: Body must be in a scene.");
	PX_CHECK_AND_RETURN(!(b.getFlags() & PxRigidBodyFlag::eKINEMATIC), "RigidDynamic::wakeUp: Body must be non-kinematic!");
	PX_CHECK_AND_RETURN(!(b.getActorFlags() & PxActorFlag::eDISABLE_SIMULATION), "RigidDynamic::wakeUp: Not allowed if PxActorFlag::eDISABLE_SIMULATION is set!");
	
	b.wakeUp();
}


void NpRigidDynamic::putToSleep()
{
	Scb::Body& b = getScbBodyFast();

	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));
	PX_CHECK_AND_RETURN(NpActor::getAPIScene(*this), "NpRigidDynamic::putToSleep: Body must be in a scene.");
	PX_CHECK_AND_RETURN(!(b.getFlags() & PxRigidBodyFlag::eKINEMATIC), "RigidDynamic::putToSleep: Body must be non-kinematic!");
	PX_CHECK_AND_RETURN(!(b.getActorFlags() & PxActorFlag::eDISABLE_SIMULATION), "RigidDynamic::putToSleep: Not allowed if PxActorFlag::eDISABLE_SIMULATION is set!");
	
	b.putToSleep();
}


void NpRigidDynamic::setSolverIterationCounts(PxU32 positionIters, PxU32 velocityIters)
{
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));
	PX_CHECK_AND_RETURN(positionIters > 0, "RigidDynamic::setSolverIterationCount: positionIters must be more than zero!");
	PX_CHECK_AND_RETURN(positionIters <= 255, "RigidDynamic::setSolverIterationCount: positionIters must be no greater than 255!");
	PX_CHECK_AND_RETURN(velocityIters > 0, "RigidDynamic::setSolverIterationCount: velocityIters must be more than zero!");
	PX_CHECK_AND_RETURN(velocityIters <= 255, "RigidDynamic::setSolverIterationCount: velocityIters must be no greater than 255!");

	getScbBodyFast().setSolverIterationCounts((velocityIters & 0xff) << 8 | (positionIters & 0xff));
}


void NpRigidDynamic::getSolverIterationCounts(PxU32 & positionIters, PxU32 & velocityIters) const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));

	PxU16 x = getScbBodyFast().getSolverIterationCounts();
	velocityIters = PxU32(x >> 8);
	positionIters = PxU32(x & 0xff);
}


void NpRigidDynamic::setContactReportThreshold(PxReal threshold)
{
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));
	PX_CHECK_AND_RETURN(PxIsFinite(threshold), "NpRigidDynamic::setContactReportThreshold: invalid float.");
	PX_CHECK_AND_RETURN(threshold >= 0.0f, "RigidDynamic::setContactReportThreshold: Force threshold must be greater than zero!");

	getScbBodyFast().setContactReportThreshold(threshold<0 ? 0 : threshold);

	GRB_EVENT(getScene(), GrbInteropEvent3, GrbInteropEvent3::PxActorSetContactReportThreshold, static_cast<PxActor *>(this), threshold<0 ? 0 : threshold);
}


PxReal NpRigidDynamic::getContactReportThreshold() const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));

	return getScbBodyFast().getContactReportThreshold();
}


PxU32 physx::NpRigidDynamicGetShapes(Scb::Body& body, void* const *&shapes)
{
	NpRigidDynamic* a = static_cast<NpRigidDynamic*>(body.getScBody().getPxActor());
	NpShapeManager& sm = a->getShapeManager();
	shapes = reinterpret_cast<void *const *>(sm.getShapes());
	return sm.getNbShapes();
}


void NpRigidDynamic::switchToNoSim()
{
	getScbBodyFast().switchBodyToNoSim();
}


void NpRigidDynamic::switchFromNoSim()
{
	getScbBodyFast().switchFromNoSim(true);
}


void NpRigidDynamic::wakeUpInternalNoKinematicTest(Scb::Body& body, bool forceWakeUp, bool autowake)
{
	NpScene* scene = NpActor::getOwnerScene(*this);
	PX_ASSERT(scene);
	PxReal wakeCounterResetValue = scene->getWakeCounterResetValueInteral();

	PxReal wakeCounter = body.getWakeCounter();

	bool needsWakingUp = body.isSleeping() && (autowake || forceWakeUp);
	if (autowake && (wakeCounter < wakeCounterResetValue))
	{
		wakeCounter = wakeCounterResetValue;
		needsWakingUp = true;
	}

	if (needsWakingUp)
		body.wakeUpInternal(wakeCounter);
}


#if PX_ENABLE_DEBUG_VISUALIZATION
#include "GuDebug.h"
void NpRigidDynamic::visualize(Cm::RenderOutput& out, NpScene* npScene)
{
	NpRigidDynamicT::visualize(out, npScene);

	if (getScbBodyFast().getActorFlags() & PxActorFlag::eVISUALIZATION)
	{
		PX_ASSERT(npScene);
		PxReal scale = npScene->getVisualizationParameter(PxVisualizationParameter::eSCALE);

		PxReal massAxes = scale * npScene->getVisualizationParameter(PxVisualizationParameter::eBODY_MASS_AXES);
		if (massAxes != 0)
		{
			PxReal sleepTime = getScbBodyFast().getWakeCounter() / npScene->getWakeCounterResetValueInteral();
			PxU32 color = PxU32(0xff * (sleepTime>1.0f ? 1.0f : sleepTime));
			color = getScbBodyFast().isSleeping() ? 0xff0000 : (color<<16 | color<<8 | color);
			PxVec3 dims = invertDiagInertia(getScbBodyFast().getInverseInertia());
			dims = getDimsFromBodyInertia(dims, 1.0f / getScbBodyFast().getInverseMass());

			out << color << Gu::Debug::convertToPxMat44(getScbBodyFast().getBody2World()) << Cm::DebugBox(dims * 0.5f);
		}
	}
}
#endif
