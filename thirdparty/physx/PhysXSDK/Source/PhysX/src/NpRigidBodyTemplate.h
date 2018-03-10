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


#ifndef PX_PHYSICS_NP_RIGIDBODY_TEMPLATE
#define PX_PHYSICS_NP_RIGIDBODY_TEMPLATE

#include "NpRigidActorTemplate.h"
#include "ScbBody.h"
#include "GuDebug.h"
#include "NpPhysics.h"
#include "NpShape.h"
#include "NpScene.h"

namespace physx
{

PX_INLINE PxVec3 invertDiagInertia(const PxVec3& m)
{
	return PxVec3(m.x == 0.f ? 0.f : 1.f/m.x, m.y == 0.f ? 0.f : 1.f/m.y, m.z == 0.f ? 0.f : 1.f/m.z);
}


#if PX_ENABLE_DEBUG_VISUALIZATION
/*
given the diagonal of the body space inertia tensor, and the total mass
this returns the body space AABB width, height and depth of an equivalent box
*/
PX_INLINE PxVec3 getDimsFromBodyInertia(const PxVec3& inertiaMoments, PxReal mass)
{
	PxVec3 inertia = inertiaMoments * (6.0f/mass);
	return PxVec3(	PxSqrt(PxAbs(- inertia.x + inertia.y + inertia.z)),
					PxSqrt(PxAbs(+ inertia.x - inertia.y + inertia.z)),
					PxSqrt(PxAbs(+ inertia.x + inertia.y - inertia.z)));
}
#endif


template<class APIClass>
class NpRigidBodyTemplate : public NpRigidActorTemplate<APIClass>
{
private:
	typedef		NpRigidActorTemplate<APIClass> RigidActorTemplateClass;
public:
// PX_SERIALIZATION
										NpRigidBodyTemplate(PxBaseFlags baseFlags) : RigidActorTemplateClass(baseFlags), mBody(PxEmpty)	{}
//~PX_SERIALIZATION
	virtual								~NpRigidBodyTemplate();

	//---------------------------------------------------------------------------------
	// PxRigidActor implementation
	//---------------------------------------------------------------------------------
	// The rule is: If an API method is used somewhere in here, it has to be redeclared, else GCC whines
	virtual			PxTransform			getGlobalPose() const = 0;

	//---------------------------------------------------------------------------------
	// PxRigidBody implementation
	//---------------------------------------------------------------------------------

	// Center of mass pose
	virtual			PxTransform 		getCMassLocalPose() const;

	// Mass
	virtual			void				setMass(PxReal mass);
	virtual			PxReal				getMass() const;
	virtual			PxReal				getInvMass() const;

	virtual			void				setMassSpaceInertiaTensor(const PxVec3& m);
	virtual			PxVec3				getMassSpaceInertiaTensor() const;
	virtual			PxVec3				getMassSpaceInvInertiaTensor() const;

	// Velocity
	virtual			PxVec3				getLinearVelocity()	const;
	virtual			PxVec3				getAngularVelocity() const;

	virtual			PxShape*			createShape(const PxGeometry& geometry, PxMaterial*const* material, PxU16 materialCount, PxShapeFlags shapeFlags);
	virtual			void				attachShape(PxShape& shape);

	//---------------------------------------------------------------------------------
	// Miscellaneous
	//---------------------------------------------------------------------------------
										NpRigidBodyTemplate(PxType concreteType, PxBaseFlags baseFlags, const PxActorType::Enum type, const PxTransform& bodyPose);

	PX_FORCE_INLINE	const Scb::Body&	getScbBodyFast()		const	{ return mBody;			}	// PT: important: keep returning an address here (else update prefetch in SceneQueryManager::addShapes)
	PX_FORCE_INLINE	Scb::Body&			getScbBodyFast()				{ return mBody;			}	// PT: important: keep returning an address here (else update prefetch in SceneQueryManager::addShapes)

	PX_FORCE_INLINE	Scb::Actor&			getScbActorFast()				{ return mBody;			}
	PX_FORCE_INLINE	const Scb::Actor&	getScbActorFast()		const	{ return mBody;			}

	// Flags
	PX_DEPRECATED virtual	void		setRigidDynamicFlag(PxRigidDynamicFlag::Enum, bool value);
	PX_DEPRECATED virtual	void		setRigidDynamicFlags(PxRigidDynamicFlags inFlags);
	PX_DEPRECATED PX_FORCE_INLINE	PxRigidDynamicFlags	getRigidDynamicFlagsFast() const
	{
		return static_cast<PxRigidDynamicFlags>(getScbBodyFast().getFlags());
	}
	PX_DEPRECATED virtual	PxRigidDynamicFlags	getRigidDynamicFlags() const
	{
		NP_READ_CHECK(NpActor::getOwnerScene(*this));
		return getRigidDynamicFlagsFast();
	}

	virtual		void				setRigidBodyFlag(PxRigidBodyFlag::Enum, bool value);
	virtual		void				setRigidBodyFlags(PxRigidBodyFlags inFlags);
	PX_FORCE_INLINE	PxRigidBodyFlags	getRigidBodyFlagsFast() const
	{
		return getScbBodyFast().getFlags();
	}
	virtual		PxRigidBodyFlags	getRigidBodyFlags() const
	{
		NP_READ_CHECK(NpActor::getOwnerScene(*this));
		return getRigidDynamicFlagsFast();
	}

	virtual void setMinCCDAdvanceCoefficient(PxReal advanceCoefficient);

	virtual PxReal getMinCCDAdvanceCoefficient() const;

	virtual void setMaxDepenetrationVelocity(const PxReal maxDepenVel);

	virtual PxReal getMaxDepenetrationVelocity() const;


protected:
					void				setCMassLocalPoseInternal(const PxTransform&);

					void				addSpatialForce(const PxVec3* force, const PxVec3* torque, PxForceMode::Enum mode);
					void				clearSpatialForce(PxForceMode::Enum mode, bool force, bool torque);

	PX_INLINE		void				updateBody2Actor(const PxTransform& newBody2Actor);

	PX_FORCE_INLINE void				setRigidBodyFlagsInternal(const PxRigidBodyFlags& currentFlags, const PxRigidBodyFlags& newFlags);

#if PX_ENABLE_DEBUG_VISUALIZATION
public:
					void				visualize(Cm::RenderOutput& out, NpScene* scene);
#endif

	PX_FORCE_INLINE bool				isKinematic()
	{
		return (APIClass::getConcreteType() == PxConcreteType::eRIGID_DYNAMIC) && (getScbBodyFast().getFlags() & PxRigidBodyFlag::eKINEMATIC);
	}

protected:
					Scb::Body 			mBody;
};


template<class APIClass>
NpRigidBodyTemplate<APIClass>::NpRigidBodyTemplate(PxType concreteType, PxBaseFlags baseFlags, PxActorType::Enum type, const PxTransform& bodyPose)
:	RigidActorTemplateClass(concreteType, baseFlags)
,	mBody(type, bodyPose)
{
}


template<class APIClass>
NpRigidBodyTemplate<APIClass>::~NpRigidBodyTemplate()
{
}

namespace
{
	PX_FORCE_INLINE static bool isSimGeom(PxGeometryType::Enum t)
	{
		return t != PxGeometryType::eTRIANGLEMESH && t != PxGeometryType::ePLANE && t != PxGeometryType::eHEIGHTFIELD;
	}
}

template<class APIClass>
PxShape* NpRigidBodyTemplate<APIClass>::createShape(const PxGeometry& geometry, PxMaterial*const* materials, PxU16 materialCount, PxShapeFlags shapeFlags)
{
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));
	PX_CHECK_AND_RETURN_NULL(!(shapeFlags & PxShapeFlag::eSIMULATION_SHAPE)
							 || isSimGeom(geometry.getType()) 
							 || isKinematic(),
							 "createShape: Triangle mesh, heightfield or plane geometry shapes configured as eSIMULATION_SHAPE are not supported for non-kinematic PxRigidDynamic instances.");

	NpShape* shape = static_cast<NpShape*>(NpPhysics::getInstance().createShape(geometry, materials, materialCount, true, shapeFlags));

	if ( shape != NULL )
	{
		RigidActorTemplateClass::mShapeManager.attachShape(*shape, *this);

		GRB_EVENT(this->getScene(), GrbInteropEvent3, GrbInteropEvent3::PxActorCreateShape, static_cast<PxActor *>(this), static_cast<PxShape *>(shape), 1);

		shape->releaseInternal();
	}
	return shape;
}


template<class APIClass>
void NpRigidBodyTemplate<APIClass>::attachShape(PxShape& shape)
{
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));
	PX_CHECK_AND_RETURN(!(shape.getFlags() & PxShapeFlag::eSIMULATION_SHAPE) 
						|| isSimGeom(shape.getGeometryType()) 
						|| isKinematic(),
						"attachShape: Triangle mesh, heightfield or plane geometry shapes configured as eSIMULATION_SHAPE are not supported for non-kinematic PxRigidDynamic instances.");
	RigidActorTemplateClass::attachShape(shape);
}


template<class APIClass>
void NpRigidBodyTemplate<APIClass>::setCMassLocalPoseInternal(const PxTransform& body2Actor)
{
	//the point here is to change the mass distribution w/o changing the actors' pose in the world

	PxTransform newBody2World = getGlobalPose() * body2Actor;

	mBody.setBody2World(newBody2World, true);
	mBody.setBody2Actor(body2Actor);

	RigidActorTemplateClass::updateShaderComs();
}


template<class APIClass>
PxTransform NpRigidBodyTemplate<APIClass>::getCMassLocalPose() const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));

	return getScbBodyFast().getBody2Actor();
}


template<class APIClass>
void NpRigidBodyTemplate<APIClass>::setMass(PxReal mass)
{
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));
	PX_CHECK_AND_RETURN(PxIsFinite(mass), "NpRigidBody::setMass: invalid float");
	PX_CHECK_AND_RETURN(mass>=0, "RigidDynamic::setMass: mass must be non-negative!");
	PX_CHECK_AND_RETURN(this->getType() != PxActorType::eARTICULATION_LINK || mass > 0, "NpRigidBody::setMassSpaceInertiaTensor: components must be > 0 for articualtions");

	getScbBodyFast().setInverseMass(mass > 0.f ? 1.0f/mass : 0.f);
}


template<class APIClass>
PxReal NpRigidBodyTemplate<APIClass>::getMass() const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));
	const PxReal invMass = mBody.getInverseMass();

	return invMass > 0.f ? 1.f/invMass : 0.f;
}

template<class APIClass>
PxReal NpRigidBodyTemplate<APIClass>::getInvMass() const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));

	return mBody.getInverseMass();
}


template<class APIClass>
void NpRigidBodyTemplate<APIClass>::setMassSpaceInertiaTensor(const PxVec3& m)
{
 	PX_CHECK_AND_RETURN(m.isFinite(), "NpRigidBody::setMassSpaceInertiaTensor: invalid inertia");
	PX_CHECK_AND_RETURN(m.x>=0 && m.y>=0 && m.z>=0, "NpRigidBody::setMassSpaceInertiaTensor: components must be non-negative");
	PX_CHECK_AND_RETURN(this->getType() != PxActorType::eARTICULATION_LINK || (m.x > 0 && m.y > 0 && m.z > 0), "NpRigidBody::setMassSpaceInertiaTensor: components must be > 0 for articualtions");

	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));
	
	mBody.setInverseInertia(invertDiagInertia(m));
}


template<class APIClass>
PxVec3 NpRigidBodyTemplate<APIClass>::getMassSpaceInertiaTensor() const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));

	return invertDiagInertia(mBody.getInverseInertia());
}

template<class APIClass>
PxVec3 NpRigidBodyTemplate<APIClass>::getMassSpaceInvInertiaTensor() const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));

	return mBody.getInverseInertia();
}


template<class APIClass>
PxVec3 NpRigidBodyTemplate<APIClass>::getLinearVelocity() const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));

	return mBody.getLinearVelocity();
}


template<class APIClass>
PxVec3 NpRigidBodyTemplate<APIClass>::getAngularVelocity() const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));

	return mBody.getAngularVelocity();
}


template<class APIClass>
void NpRigidBodyTemplate<APIClass>::addSpatialForce(const PxVec3* force, const PxVec3* torque, PxForceMode::Enum mode)
{
	PX_ASSERT(!(mBody.getFlags() & PxRigidBodyFlag::eKINEMATIC));

	switch (mode)
	{
		case PxForceMode::eFORCE:
		{
			PxVec3 linAcc, angAcc;
			if (force)
			{
				linAcc = (*force) * mBody.getInverseMass();
				force = &linAcc;
			}
			if (torque)
			{
				angAcc = mBody.getGlobalInertiaTensorInverse() * (*torque);
				torque = &angAcc;
			}
			mBody.addSpatialAcceleration(force, torque);
		}
		break;

		case PxForceMode::eACCELERATION:
			mBody.addSpatialAcceleration(force, torque);
		break;

		case PxForceMode::eIMPULSE:
		{
			PxVec3 linVelDelta, angVelDelta;
			if (force)
			{
				linVelDelta = ((*force) * mBody.getInverseMass());
				force = &linVelDelta;
			}
			if (torque)
			{
				angVelDelta = (mBody.getGlobalInertiaTensorInverse() * (*torque));
				torque = &angVelDelta;
			}
			mBody.addSpatialVelocity(force, torque);
		}
		break;

		case PxForceMode::eVELOCITY_CHANGE:
			mBody.addSpatialVelocity(force, torque);
		break;
	}
}

template<class APIClass>
void NpRigidBodyTemplate<APIClass>::clearSpatialForce(PxForceMode::Enum mode, bool force, bool torque)
{
	PX_ASSERT(!(mBody.getFlags() & PxRigidBodyFlag::eKINEMATIC));

	switch (mode)
	{
	case PxForceMode::eFORCE:
	case PxForceMode::eACCELERATION:
		mBody.clearSpatialAcceleration(force, torque);
		break;
	case PxForceMode::eIMPULSE:
	case PxForceMode::eVELOCITY_CHANGE:
		mBody.clearSpatialVelocity(force, torque);
		break;
	}
}


#if PX_ENABLE_DEBUG_VISUALIZATION
template<class APIClass>
void NpRigidBodyTemplate<APIClass>::visualize(Cm::RenderOutput& out, NpScene* scene)
{
	RigidActorTemplateClass::visualize(out, scene);

	if (mBody.getActorFlags() & PxActorFlag::eVISUALIZATION)
	{
		Scb::Scene& scbScene = scene->getScene();
		PxReal scale = scbScene.getVisualizationParameter(PxVisualizationParameter::eSCALE);

		//visualize actor frames
		PxReal actorAxes = scale * scbScene.getVisualizationParameter(PxVisualizationParameter::eACTOR_AXES);
		if (actorAxes != 0)
			out << Gu::Debug::convertToPxMat44(getGlobalPose()) << Cm::DebugBasis(PxVec3(actorAxes));

		PxReal bodyAxes = scale * scbScene.getVisualizationParameter(PxVisualizationParameter::eBODY_AXES);
		if (bodyAxes != 0)
			out << Gu::Debug::convertToPxMat44(mBody.getBody2World()) << Cm::DebugBasis(PxVec3(bodyAxes));

		PxReal linVelocity = scale * scbScene.getVisualizationParameter(PxVisualizationParameter::eBODY_LIN_VELOCITY);
		if (linVelocity != 0)
		{
			out << 0xffffff << PxMat44(PxIdentity) << Cm::DebugArrow(mBody.getBody2World().p, 
				mBody.getLinearVelocity() * linVelocity, 0.2f * linVelocity);
		}

		PxReal angVelocity = scale * scbScene.getVisualizationParameter(PxVisualizationParameter::eBODY_ANG_VELOCITY);
		if (angVelocity != 0)
		{
			out << 0x000000 << PxMat44(PxIdentity) << Cm::DebugArrow(mBody.getBody2World().p, 
				mBody.getAngularVelocity() * angVelocity, 0.2f * angVelocity);
		}
	}
}
#endif


PX_FORCE_INLINE void updateDynamicSceneQueryShapes(NpShapeManager& shapeManager, Sq::SceneQueryManager& sqManager)
{
	shapeManager.markAllSceneQueryForUpdate(sqManager);
	sqManager.invalidateDynamicTimestamp();
}


template<class APIClass>
PX_FORCE_INLINE void NpRigidBodyTemplate<APIClass>::setRigidBodyFlagsInternal(const PxRigidBodyFlags& currentFlags, const PxRigidBodyFlags& newFlags)
{
	PxRigidBodyFlags filteredNewFlags = newFlags;
	//Test to ensure we are not enabling both CCD and kinematic state on a body. This is unsupported
	if((filteredNewFlags & PxRigidBodyFlag::eENABLE_CCD) && (filteredNewFlags & PxRigidBodyFlag::eKINEMATIC))
	{
		physx::shdfnd::getFoundation().error(physx::PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, 
			"RigidBody::setRigidBodyFlag: kinematic bodies with CCD enabled are not supported! CCD will be ignored.");
		filteredNewFlags &= PxRigidBodyFlags(~PxRigidBodyFlag::eENABLE_CCD);
	}

	Scb::Body& body = getScbBodyFast();
	NpScene* scene = NpActor::getAPIScene(*this);

	const bool isKinematic = currentFlags & PxRigidBodyFlag::eKINEMATIC;
	const bool willBeKinematic = filteredNewFlags & PxRigidBodyFlag::eKINEMATIC;
	const bool kinematicSwitchingToDynamic = isKinematic && (!willBeKinematic);
	const bool dynamicSwitchingToKinematic = (!isKinematic) && willBeKinematic;

	if(kinematicSwitchingToDynamic)
	{
		NpShapeManager& shapeManager = this->getShapeManager();
		PxU32 nbShapes = shapeManager.getNbShapes();
		NpShape*const* shapes = shapeManager.getShapes();
		bool hasTriangleMesh = false;
		for(PxU32 i=0;i<nbShapes;i++)
		{
			if((shapes[i]->getFlags() & PxShapeFlag::eSIMULATION_SHAPE) && (shapes[i]->getGeometryTypeFast()==PxGeometryType::eTRIANGLEMESH || shapes[i]->getGeometryTypeFast()==PxGeometryType::ePLANE || shapes[i]->getGeometryTypeFast()==PxGeometryType::eHEIGHTFIELD))
			{
				hasTriangleMesh = true;
				break;
			}
		}
		if(hasTriangleMesh)
		{
			physx::shdfnd::getFoundation().error(physx::PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "RigidBody::setRigidBodyFlag: dynamic meshes/planes/heightfields are not supported!");
			return;
		}

		PxTransform bodyTarget;
		if ((currentFlags & PxRigidBodyFlag::eUSE_KINEMATIC_TARGET_FOR_SCENE_QUERIES) && body.getKinematicTarget(bodyTarget) && scene)
		{
			updateDynamicSceneQueryShapes(shapeManager, scene->getSceneQueryManagerFast());
		}

		body.clearSimStateDataForPendingInsert();
	}
	else if (dynamicSwitchingToKinematic)
	{
		if (this->getType() != PxActorType::eARTICULATION_LINK)
		{
			body.transitionSimStateDataForPendingInsert();
		}
		else
		{
			//We're an articulation, raise an issue
			physx::shdfnd::getFoundation().error(physx::PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "RigidBody::setRigidBodyFlag: kinematic articulation links are not supported!");
			return;
		}
	}

	const bool kinematicSwitchingUseTargetForSceneQuery = isKinematic && willBeKinematic && 
														((currentFlags & PxRigidBodyFlag::eUSE_KINEMATIC_TARGET_FOR_SCENE_QUERIES) != (filteredNewFlags & PxRigidBodyFlag::eUSE_KINEMATIC_TARGET_FOR_SCENE_QUERIES));
	if (kinematicSwitchingUseTargetForSceneQuery)
	{
		PxTransform bodyTarget;
		if (body.getKinematicTarget(bodyTarget) && scene)
		{
			updateDynamicSceneQueryShapes(this->getShapeManager(), scene->getSceneQueryManagerFast());
		}
	}

	body.setFlags(filteredNewFlags);
}


template<class APIClass>
void NpRigidBodyTemplate<APIClass>::setRigidBodyFlag(PxRigidBodyFlag::Enum flag, bool value)
{
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));

	Scb::Body& body = getScbBodyFast();
	const PxRigidBodyFlags currentFlags = body.getFlags();
	const PxRigidBodyFlags newFlags = value ? currentFlags | flag : currentFlags & (~PxRigidBodyFlags(flag));

	setRigidBodyFlagsInternal(currentFlags, newFlags);
}


template<class APIClass>
void NpRigidBodyTemplate<APIClass>::setRigidBodyFlags(PxRigidBodyFlags inFlags)
{
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));

	Scb::Body& body = getScbBodyFast();
	const PxRigidBodyFlags currentFlags = body.getFlags();

	setRigidBodyFlagsInternal(currentFlags, inFlags);
}


template<class APIClass>
void NpRigidBodyTemplate<APIClass>::setRigidDynamicFlag(PxRigidDynamicFlag::Enum flag, bool value)
{
	PxRigidBodyFlag::Enum bodyFlag = static_cast<PxRigidBodyFlag::Enum>(flag);
	setRigidBodyFlag(bodyFlag, value);
}

template<class APIClass>
void NpRigidBodyTemplate<APIClass>::setRigidDynamicFlags(PxRigidDynamicFlags inFlags)
{
	PxRigidBodyFlags bodyFlags = static_cast<PxRigidBodyFlags>(inFlags);
	setRigidBodyFlags(bodyFlags);
}

template<class APIClass>
void NpRigidBodyTemplate<APIClass>::setMinCCDAdvanceCoefficient(PxReal minCCDAdvanceCoefficient)
{
	mBody.setMinCCDAdvanceCoefficient(minCCDAdvanceCoefficient);
}

template<class APIClass>
PxReal NpRigidBodyTemplate<APIClass>::getMinCCDAdvanceCoefficient() const
{
	return mBody.getMinCCDAdvanceCoefficient();
}

template<class APIClass>
void NpRigidBodyTemplate<APIClass>::setMaxDepenetrationVelocity(const PxReal maxDepenVel)
{
	PX_CHECK_AND_RETURN(maxDepenVel > 0.f, "NpRigidBody::setMaxDepenetrationVelocity: maxDepenVel must be greater than zero.");
	mBody.setMaxPenetrationBias(-maxDepenVel);
}

template<class APIClass>
PxReal NpRigidBodyTemplate<APIClass>::getMaxDepenetrationVelocity() const
{
	return -mBody.getMaxPenetrationBias();
}


}

#endif
