/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_ACTOR_H
#define APEX_ACTOR_H

#include "ApexContext.h"
#include "ApexRenderable.h"
#include "ApexInterface.h"
#include "NiResourceProvider.h"
#include "NxApexSDK.h"

#if NX_SDK_VERSION_MAJOR == 2
#include "NxActor.h"
#elif NX_SDK_VERSION_MAJOR == 3
#include "PxActor.h"
#include "PxShape.h"
#include "PxFiltering.h"
#include "PxRigidDynamic.h"
#include "foundation/PxTransform.h"
#include "PxRigidBodyExt.h"
#include "NxApexActor.h"
#endif

#ifdef APEX_SHIPPING
#define UNIQUE_ACTOR_ID 0
#else
#define UNIQUE_ACTOR_ID 1
#endif

#if NX_SDK_VERSION_MAJOR == 2
class NxActorDescBase;
#endif

namespace physx
{
namespace apex
{

class ApexContext;
class NiApexRenderDebug;
class NiApexScene;
class NxApexAsset;
class NxApexActor;

/**
Class that implements actor interface with its context(s)
*/
class ApexActor : public ApexRenderable, public ApexContext
{
public:
	ApexActor();
	~ApexActor();

	void				addSelfToContext(ApexContext& ctx, ApexActor* actorPtr = NULL);
	void				updateIndex(ApexContext& ctx, physx::PxU32 index);
	bool				findSelfInContext(ApexContext& ctx);

	// Each class that derives from ApexActor should implement the following functions
	// if it wants ActorCreationNotification and Deletion callbacks
	virtual NxApexAsset*	getNxApexAsset(void)
	{
		return NULL;
	}
	virtual void			ContextActorCreationNotification(NxAuthObjTypeID	authorableObjectType,
	        ApexActor*		actorPtr)
	{
		PX_UNUSED(authorableObjectType);
		PX_UNUSED(actorPtr);
		return;
	}
	virtual void			ContextActorDeletionNotification(NxAuthObjTypeID	authorableObjectType,
	        ApexActor*		actorPtr)
	{
		PX_UNUSED(authorableObjectType);
		PX_UNUSED(actorPtr);
		return;
	}

	// Each class that derives from ApexActor may optionally implement these functions
	virtual NxApexRenderable* getRenderable()
	{
		return NULL;
	}
	virtual NxApexActor*      getNxApexActor()
	{
		return NULL;
	}

	virtual void        release() = 0;
	void				destroy();

#if NX_SDK_VERSION_MAJOR == 2
	// NxScene pointer may be NULL
	virtual void		setPhysXScene(NxScene* s) = 0;
	virtual NxScene*	getPhysXScene() const = 0;
#elif NX_SDK_VERSION_MAJOR == 3
	virtual void		setPhysXScene(PxScene* s) = 0;
	virtual PxScene*	getPhysXScene() const = 0;
#endif

	enum ActorState
	{
		StateEnabled,
		StateDisabled,
		StateEnabling,
		StateDisabling,
	};

	void visualizeLodBenefit(NiApexRenderDebug& renderDebug, NiApexScene& apexScene, const PxVec3& centroid, PxF32 radius, PxF32 absoluteBenefit, ActorState state);

	/**
	\brief Selectively enables/disables debug visualization of a specific APEX actor.  Default value it true.
	*/
	virtual void setEnableDebugVisualization(bool state)
	{
		mEnableDebugVisualization = state;
	}

protected:
	bool				mInRelease;

	struct ContextTrack
	{
		physx::PxU32	index;
		ApexContext*	ctx;
	};
	physx::Array<ContextTrack> mContexts;

#if UNIQUE_ACTOR_ID
	static PxI32 mUniqueActorIdCounter;
	PxI32 mUniqueActorId;
#endif

	bool				mEnableDebugVisualization;
	friend class ApexContext;
};


#if NX_SDK_VERSION_MAJOR == 2

/**
Class that manages creation templates.
*/
class ApexActorSource
{
public:

	// NxApexActorSource methods

	void setActorTemplate(const NxActorDescBase* desc)
	{
		actorTemplate.set(static_cast<const ActorTemplate*>(desc));
	}
	bool getActorTemplate(NxActorDescBase& dest) const
	{
		return actorTemplate.get(static_cast<ActorTemplate&>(dest));
	}
	void modifyActor(NxActor* actor) const
	{
		if (actorTemplate.isSet)
		{
			actorTemplate.data.apply(actor);
		}
	}

	void setShapeTemplate(const NxShapeDesc* desc)
	{
		shapeTemplate.set(static_cast<const ShapeTemplate*>(desc));
	}
	bool getShapeTemplate(NxShapeDesc& dest) const
	{
		return shapeTemplate.get(static_cast<ShapeTemplate&>(dest));
	}
	void modifyShape(NxShape* shape) const
	{
		if (shapeTemplate.isSet)
		{
			shapeTemplate.data.apply(shape);
		}
	}

	void setBodyTemplate(const NxBodyDesc* desc)
	{
		bodyTemplate.set(static_cast<const BodyTemplate*>(desc));
	}
	bool getBodyTemplate(NxBodyDesc& dest)	const
	{
		return bodyTemplate.get(static_cast<BodyTemplate&>(dest));
	}
	void modifyBody(NxActor* actor) const
	{
		if (bodyTemplate.isSet)
		{
			bodyTemplate.data.apply(actor);
		}
	}


protected:

	class ShapeTemplate : public NxShapeDesc
	{
		//gotta make a special derived class cause of non-void ctor
	public:
		ShapeTemplate() : NxShapeDesc(NX_SHAPE_PLANE) {}	//type is trash, we have to supply something.
		ShapeTemplate& operator=(const ShapeTemplate& src)
		{
			//copy over the variables that Shape templates transfer: (memcopy is a bad idea because it will clobber types, etc.)

			shapeFlags = src.shapeFlags;
			group = src.group;
			materialIndex = src.materialIndex;
			density = src.density;	//do we want this!?
			skinWidth = src.skinWidth;
			userData = src.userData;
			name = src.name;
			groupsMask = src.groupsMask;
#if (NX_SDK_VERSION_NUMBER >= 280) && (NX_SDK_VERSION_NUMBER < 290)
			nonInteractingCompartmentTypes = src.nonInteractingCompartmentTypes;
#endif
			return *this;
		}

		void apply(NxShape* shape) const
		{
#define SETFLAG(_FLAG) shape->setFlag(_FLAG, (shapeFlags & _FLAG) != 0);
			SETFLAG(NX_TRIGGER_ON_ENTER);
			SETFLAG(NX_TRIGGER_ON_LEAVE);
			SETFLAG(NX_TRIGGER_ON_STAY);
			SETFLAG(NX_SF_VISUALIZATION);
			SETFLAG(NX_SF_DISABLE_COLLISION);
			SETFLAG(NX_SF_FEATURE_INDICES);
			SETFLAG(NX_SF_DISABLE_RAYCASTING);
			SETFLAG(NX_SF_POINT_CONTACT_FORCE);
			SETFLAG(NX_SF_FLUID_DRAIN);
			SETFLAG(NX_SF_FLUID_DISABLE_COLLISION);
			SETFLAG(NX_SF_FLUID_TWOWAY);
			SETFLAG(NX_SF_DISABLE_RESPONSE);
			SETFLAG(NX_SF_DYNAMIC_DYNAMIC_CCD);
			SETFLAG(NX_SF_DISABLE_SCENE_QUERIES);
			SETFLAG(NX_SF_CLOTH_DRAIN);
			SETFLAG(NX_SF_CLOTH_DISABLE_COLLISION);
			SETFLAG(NX_SF_CLOTH_TWOWAY);
			SETFLAG(NX_SF_SOFTBODY_DRAIN);
			SETFLAG(NX_SF_SOFTBODY_DISABLE_COLLISION);
			SETFLAG(NX_SF_SOFTBODY_TWOWAY);
#undef SETFLAG

			shape->setGroup(group);
			shape->setMaterial(materialIndex);
			shape->setSkinWidth(skinWidth);
			shape->userData = userData;
			shape->setName(name);
			shape->setGroupsMask(groupsMask);
#if (NX_SDK_VERSION_NUMBER >= 280) && (NX_SDK_VERSION_NUMBER < 290)
			//shape->setNonInteractingCompartmentTypes(nonInteractingCompartmentTypes); // this crashes the mirror manager in the clothing tool, nvbug 552007
#endif
		}

	};

	class ActorTemplate : public NxActorDescBase
	{
	public:
		ActorTemplate()
		{
			setToDefault();	//this is not done by default for NxActorDescBase!
		}

		ActorTemplate& operator=(const ActorTemplate& src)
		{
			//copy over the variables that Actor templates transfer: (memcopy is a bad idea because it will clobber types, etc.)

			density = src.density;
			flags = src.flags;
			group = src.group;
			dominanceGroup = src.dominanceGroup;
#if (NX_SDK_VERSION_NUMBER >= 280)
			contactReportFlags = src.contactReportFlags;
			forceFieldMaterial = src.forceFieldMaterial;
#endif
			userData = src.userData;
			name = src.name;
			compartment = src.compartment;

			return *this;
		}

		void apply(NxActor* actor) const
		{
			PX_ASSERT(actor != NULL);

#define SETFLAG(_FLAG) if (flags & _FLAG) actor->raiseActorFlag(_FLAG); else actor->clearActorFlag(_FLAG);
			SETFLAG(NX_AF_DISABLE_RESPONSE);
			SETFLAG(NX_AF_CONTACT_MODIFICATION);
			SETFLAG(NX_AF_FORCE_CONE_FRICTION);
			SETFLAG(NX_AF_USER_ACTOR_PAIR_FILTERING);
			SETFLAG(NX_AF_DISABLE_COLLISION);
#undef SETFLAG

			actor->setGroup(group);
			actor->setDominanceGroup(dominanceGroup);
#if (NX_SDK_VERSION_NUMBER >= 280)
			actor->setContactReportFlags(contactReportFlags);
			actor->setForceFieldMaterial(forceFieldMaterial);
#endif
			actor->userData = userData;
			actor->setName(name);
		}
	};

	class BodyTemplate : public NxBodyDesc
	{
	public:
		BodyTemplate& operator=(const BodyTemplate& src)
		{
			//copy over the variables that Body templates transfer: (memcopy is a bad idea because it will clobber types, etc.)

			wakeUpCounter = src.wakeUpCounter;
			linearDamping = src.linearDamping;
			angularDamping = src.angularDamping;
			maxAngularVelocity = src.maxAngularVelocity;
			CCDMotionThreshold = src.CCDMotionThreshold;
			flags = src.flags;
			sleepLinearVelocity = src.sleepLinearVelocity;
			sleepAngularVelocity = src.sleepAngularVelocity;
			solverIterationCount = src.solverIterationCount;
			sleepEnergyThreshold = src.sleepEnergyThreshold;
			sleepDamping = src.sleepDamping;
#if (NX_SDK_VERSION_NUMBER >= 280)
			contactReportThreshold = src.contactReportThreshold;
#endif

			return *this;
		}

		void apply(NxActor* actor) const
		{
			if (actor->isDynamic())
			{
#define SETFLAG(_FLAG) if (flags & _FLAG) actor->raiseBodyFlag(_FLAG); else actor->clearBodyFlag(_FLAG);
				SETFLAG(NX_BF_DISABLE_GRAVITY);
				SETFLAG(NX_BF_FILTER_SLEEP_VEL);
				SETFLAG(NX_BF_ENERGY_SLEEP_TEST);
#undef SETFLAG

				actor->wakeUp(wakeUpCounter);
				actor->setLinearDamping(linearDamping);
				actor->setAngularDamping(angularDamping);
				actor->setMaxAngularVelocity(maxAngularVelocity);
				actor->setCCDMotionThreshold(CCDMotionThreshold);
				actor->setSleepLinearVelocity(sleepLinearVelocity);
				actor->setSleepAngularVelocity(sleepAngularVelocity);
				actor->setSolverIterationCount(solverIterationCount);
				actor->setSleepEnergyThreshold(sleepEnergyThreshold);
				// actor->setSleepDamping(sleepDamping); // interface does not exist
#if (NX_SDK_VERSION_NUMBER >= 280)
				//actor->setContactReportThreshold(contactReportThreshold); // this crashes the mirror manager in the clothing tool, nvbug 552022
#endif
			}
		}
	};


	InitTemplate<ActorTemplate> actorTemplate;
	InitTemplate<ShapeTemplate> shapeTemplate;
	InitTemplate<BodyTemplate> bodyTemplate;
};
#elif NX_SDK_VERSION_MAJOR == 3

#define APEX_ACTOR_TEMPLATE_PARAM(_type, _name, _variable) \
bool set##_name(_type x) \
{ \
	_variable = x; \
	return is##_name##Valid(x); \
} \
_type get##_name() const { return _variable; }

#define APEX_ACTOR_TEMPLATE_PARAM_VALID_OR_RETURN(_name, _variable) \
if (!is##_name##Valid(_variable)) \
{ \
	return false; \
}

#define APEX_ACTOR_TEMPLATE_PARAM_SET_DEFAULT(_name)	set##_name( getDefault##_name() );

// template for PhysX3.0 actor, body and shape.
class PhysX3DescTemplate : public physx::NxPhysX3DescTemplate, public UserAllocated
{
public:
	PhysX3DescTemplate()
	{
		SetToDefault();
	}
	void apply(PxActor* actor) const
	{
		actor->setActorFlags((PxActorFlags)actorFlags);
		actor->setDominanceGroup(dominanceGroup);
		actor->setOwnerClient(ownerClient);
		PX_ASSERT(clientBehaviorBits < PX_MAX_U8);
		actor->setClientBehaviorFlags(PxActorClientBehaviorFlags((PxU8)clientBehaviorBits));
		//actor->contactReportFlags;	// must be set via call NxApexPhysX3Interface::setContactReportFlags
		actor->userData	= userData;
		if (name)
		{
			actor->setName(name);
		}

		// body
		PxRigidBody*	rb	= actor->isRigidBody();
		if (rb)
		{
			// density, user should call updateMassAndInertia when shapes are created.
		}

		PxRigidDynamic*	rd	= actor->isRigidDynamic();
		if (rd)
		{
			rd->setRigidDynamicFlags(PxRigidDynamicFlags(bodyFlags));
			rd->setWakeCounter(wakeUpCounter);
			rd->setLinearDamping(linearDamping);
			rd->setAngularDamping(angularDamping);
			rd->setMaxAngularVelocity(maxAngularVelocity);
			// sleepLinearVelocity	attribute for deformable/cloth, see below.
			rd->setSolverIterationCounts(solverIterationCount, velocityIterationCount);
			rd->setContactReportThreshold(contactReportThreshold);
			rd->setSleepThreshold(sleepThreshold);
		}
	}
	void apply(PxShape* shape) const
	{
		shape->setFlags((PxShapeFlags)shapeFlags);
		shape->setMaterials(materials.begin(), static_cast<physx::PxU16>(materials.size()));
		shape->userData	= shapeUserData;
		if (shapeName)
		{
			shape->setName(shapeName);
		}
		shape->setSimulationFilterData(simulationFilterData);
		shape->setQueryFilterData(queryFilterData);
		shape->setContactOffset(contactOffset);
		shape->setRestOffset(restOffset);
	}

	APEX_ACTOR_TEMPLATE_PARAM(physx::PxDominanceGroup,	DominanceGroup,				dominanceGroup)
	APEX_ACTOR_TEMPLATE_PARAM(physx::PxU16,				ActorFlags,					actorFlags)
	APEX_ACTOR_TEMPLATE_PARAM(physx::PxClientID,		OwnerClient,				ownerClient)
	APEX_ACTOR_TEMPLATE_PARAM(physx::PxU32,				ClientBehaviorBits,			clientBehaviorBits)
	APEX_ACTOR_TEMPLATE_PARAM(physx::PxU16,				ContactReportFlags,			contactReportFlags)
	APEX_ACTOR_TEMPLATE_PARAM(void*,					UserData,					userData)
	APEX_ACTOR_TEMPLATE_PARAM(const char*,				Name,						name)

	APEX_ACTOR_TEMPLATE_PARAM(physx::PxReal,			Density,					density)
	APEX_ACTOR_TEMPLATE_PARAM(physx::PxU8,				BodyFlags,					bodyFlags)
	APEX_ACTOR_TEMPLATE_PARAM(physx::PxReal,			WakeUpCounter,				wakeUpCounter)
	APEX_ACTOR_TEMPLATE_PARAM(physx::PxReal,			LinearDamping,				linearDamping)
	APEX_ACTOR_TEMPLATE_PARAM(physx::PxReal,			AngularDamping,				angularDamping)
	APEX_ACTOR_TEMPLATE_PARAM(physx::PxReal,			MaxAngularVelocity,			maxAngularVelocity)
	APEX_ACTOR_TEMPLATE_PARAM(physx::PxReal,			SleepLinearVelocity,		sleepLinearVelocity)
	APEX_ACTOR_TEMPLATE_PARAM(physx::PxU32,				SolverIterationCount,		solverIterationCount)
	APEX_ACTOR_TEMPLATE_PARAM(physx::PxU32,				VelocityIterationCount,		velocityIterationCount)
	APEX_ACTOR_TEMPLATE_PARAM(physx::PxReal,			ContactReportThreshold,		contactReportThreshold)
	APEX_ACTOR_TEMPLATE_PARAM(physx::PxReal,			SleepThreshold,				sleepLinearVelocity)

	APEX_ACTOR_TEMPLATE_PARAM(physx::PxU8,				ShapeFlags,					shapeFlags)
	APEX_ACTOR_TEMPLATE_PARAM(void*,					ShapeUserData,				shapeUserData)
	APEX_ACTOR_TEMPLATE_PARAM(const char*,				ShapeName,					shapeName)
	APEX_ACTOR_TEMPLATE_PARAM(physx::PxFilterData,		SimulationFilterData,		simulationFilterData)
	APEX_ACTOR_TEMPLATE_PARAM(physx::PxFilterData,		QueryFilterData,			queryFilterData)
	APEX_ACTOR_TEMPLATE_PARAM(physx::PxReal,			ContactOffset,				contactOffset)
	APEX_ACTOR_TEMPLATE_PARAM(physx::PxReal,			RestOffset,					restOffset)
	physx::PxMaterial**	getMaterials(physx::PxU32& materialCount) const
	{
		materialCount = materials.size();
		return const_cast<physx::PxMaterial**>(materials.begin());
	}
	bool				setMaterials(physx::PxMaterial** materialArray, physx::PxU32 materialCount)
	{
		const bool valid = materialArray != NULL && materialCount > 0;
		if (!valid)
		{
			materials.reset();
		}
		else
		{
			materials = Array<PxMaterial*>(materialArray, materialArray + materialCount);
		}
		return valid;
	}

	bool isValid()
	{
		APEX_ACTOR_TEMPLATE_PARAM_VALID_OR_RETURN(DominanceGroup,			dominanceGroup)
		APEX_ACTOR_TEMPLATE_PARAM_VALID_OR_RETURN(ActorFlags,				actorFlags)
		APEX_ACTOR_TEMPLATE_PARAM_VALID_OR_RETURN(OwnerClient,				ownerClient)
		APEX_ACTOR_TEMPLATE_PARAM_VALID_OR_RETURN(ClientBehaviorBits,		clientBehaviorBits)
		APEX_ACTOR_TEMPLATE_PARAM_VALID_OR_RETURN(ContactReportFlags,		contactReportFlags)
		APEX_ACTOR_TEMPLATE_PARAM_VALID_OR_RETURN(UserData,					userData)
		APEX_ACTOR_TEMPLATE_PARAM_VALID_OR_RETURN(Name,						name)

		APEX_ACTOR_TEMPLATE_PARAM_VALID_OR_RETURN(Density,					density)
		APEX_ACTOR_TEMPLATE_PARAM_VALID_OR_RETURN(BodyFlags,				bodyFlags)
		APEX_ACTOR_TEMPLATE_PARAM_VALID_OR_RETURN(WakeUpCounter,			wakeUpCounter)
		APEX_ACTOR_TEMPLATE_PARAM_VALID_OR_RETURN(LinearDamping,			linearDamping)
		APEX_ACTOR_TEMPLATE_PARAM_VALID_OR_RETURN(AngularDamping,			angularDamping)
		APEX_ACTOR_TEMPLATE_PARAM_VALID_OR_RETURN(MaxAngularVelocity,		maxAngularVelocity)
		APEX_ACTOR_TEMPLATE_PARAM_VALID_OR_RETURN(SleepLinearVelocity,		sleepLinearVelocity)
		APEX_ACTOR_TEMPLATE_PARAM_VALID_OR_RETURN(SolverIterationCount,		solverIterationCount)
		APEX_ACTOR_TEMPLATE_PARAM_VALID_OR_RETURN(VelocityIterationCount,	velocityIterationCount)
		APEX_ACTOR_TEMPLATE_PARAM_VALID_OR_RETURN(ContactReportThreshold,	contactReportThreshold)
		APEX_ACTOR_TEMPLATE_PARAM_VALID_OR_RETURN(SleepThreshold,			sleepLinearVelocity)

		APEX_ACTOR_TEMPLATE_PARAM_VALID_OR_RETURN(ShapeFlags,				shapeFlags)
		APEX_ACTOR_TEMPLATE_PARAM_VALID_OR_RETURN(ShapeUserData,			shapeUserData)
		APEX_ACTOR_TEMPLATE_PARAM_VALID_OR_RETURN(ShapeName,				shapeName)
		APEX_ACTOR_TEMPLATE_PARAM_VALID_OR_RETURN(SimulationFilterData,		simulationFilterData)
		APEX_ACTOR_TEMPLATE_PARAM_VALID_OR_RETURN(QueryFilterData,			queryFilterData)
		APEX_ACTOR_TEMPLATE_PARAM_VALID_OR_RETURN(ContactOffset,			contactOffset)
		APEX_ACTOR_TEMPLATE_PARAM_VALID_OR_RETURN(RestOffset,				restOffset)
		if (materials.size() == 0)
		{
			return false;
		}

		return true;
	}

	void SetToDefault()
	{
		APEX_ACTOR_TEMPLATE_PARAM_SET_DEFAULT(DominanceGroup)
		APEX_ACTOR_TEMPLATE_PARAM_SET_DEFAULT(ActorFlags)
		APEX_ACTOR_TEMPLATE_PARAM_SET_DEFAULT(OwnerClient)
		APEX_ACTOR_TEMPLATE_PARAM_SET_DEFAULT(ClientBehaviorBits)
		APEX_ACTOR_TEMPLATE_PARAM_SET_DEFAULT(ContactReportFlags)
		APEX_ACTOR_TEMPLATE_PARAM_SET_DEFAULT(UserData)
		APEX_ACTOR_TEMPLATE_PARAM_SET_DEFAULT(Name)

		APEX_ACTOR_TEMPLATE_PARAM_SET_DEFAULT(Density)
		APEX_ACTOR_TEMPLATE_PARAM_SET_DEFAULT(BodyFlags)
		APEX_ACTOR_TEMPLATE_PARAM_SET_DEFAULT(WakeUpCounter)
		APEX_ACTOR_TEMPLATE_PARAM_SET_DEFAULT(LinearDamping)
		APEX_ACTOR_TEMPLATE_PARAM_SET_DEFAULT(AngularDamping)
		APEX_ACTOR_TEMPLATE_PARAM_SET_DEFAULT(MaxAngularVelocity)
		APEX_ACTOR_TEMPLATE_PARAM_SET_DEFAULT(SleepLinearVelocity)
		APEX_ACTOR_TEMPLATE_PARAM_SET_DEFAULT(SolverIterationCount)
		APEX_ACTOR_TEMPLATE_PARAM_SET_DEFAULT(VelocityIterationCount)
		APEX_ACTOR_TEMPLATE_PARAM_SET_DEFAULT(ContactReportThreshold)
		APEX_ACTOR_TEMPLATE_PARAM_SET_DEFAULT(SleepThreshold)

		APEX_ACTOR_TEMPLATE_PARAM_SET_DEFAULT(ShapeFlags)
		APEX_ACTOR_TEMPLATE_PARAM_SET_DEFAULT(ShapeUserData)
		APEX_ACTOR_TEMPLATE_PARAM_SET_DEFAULT(ShapeName)
		APEX_ACTOR_TEMPLATE_PARAM_SET_DEFAULT(SimulationFilterData)
		APEX_ACTOR_TEMPLATE_PARAM_SET_DEFAULT(QueryFilterData)
		APEX_ACTOR_TEMPLATE_PARAM_SET_DEFAULT(ContactOffset)
		APEX_ACTOR_TEMPLATE_PARAM_SET_DEFAULT(RestOffset)
		materials.reset();
	}

	void release()
	{
		delete this;
	}

public:
	// actor
	PxDominanceGroup	dominanceGroup;
	PxU16 				actorFlags;
	PxClientID			ownerClient;
	PxU32				clientBehaviorBits;
	PxU16				contactReportFlags;
	void* 				userData;
	const char* 		name;

	// body
	PxReal				density;

	PxU8				bodyFlags;
	PxReal				wakeUpCounter;
	PxReal				linearDamping;
	PxReal				angularDamping;
	PxReal				maxAngularVelocity;
	PxReal				sleepLinearVelocity;
	PxU32				solverIterationCount;
	PxU32				velocityIterationCount;
	PxReal				contactReportThreshold;
	PxReal				sleepThreshold;

	// shape
	PxU8				shapeFlags;
	Array<PxMaterial*>	materials;
	void*				shapeUserData;
	const char*			shapeName;
	PxFilterData		simulationFilterData;
	PxFilterData		queryFilterData;
	PxReal				contactOffset;
	PxReal				restOffset;
};	// PhysX3DescTemplate

class ApexActorSource
{
public:

	// NxApexActorSource methods

	void setPhysX3Template(const NxPhysX3DescTemplate* desc)
	{
		physX3Template.set(static_cast<const PhysX3DescTemplate*>(desc));
	}
	bool getPhysX3Template(NxPhysX3DescTemplate& dest) const
	{
		return physX3Template.get(static_cast<PhysX3DescTemplate&>(dest));
	}
	NxPhysX3DescTemplate* createPhysX3DescTemplate() const
	{
		return PX_NEW(PhysX3DescTemplate);
	}

	void modifyActor(PxRigidActor* actor) const
	{
		if (physX3Template.isSet)
		{
			physX3Template.data.apply(actor);
		}
	}
	void modifyShape(PxShape* shape) const
	{
		if (physX3Template.isSet)
		{
			physX3Template.data.apply(shape);
		}
	}



protected:

	InitTemplate<PhysX3DescTemplate> physX3Template;
};

#endif // NX_SDK_VERSION_MAJOR == 2

}
} // end namespace physx::apex

#endif // __APEX_ACTOR_H__
