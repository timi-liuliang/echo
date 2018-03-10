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


#include "ScBodySim.h"
#include "ScStaticSim.h"
#include "ScScene.h"
#include "ScRbElementInteraction.h"
#include "ScParticleBodyInteraction.h"
#include "ScShapeInstancePairLL.h"
#include "ScTriggerInteraction.h"
#include "ScObjectIDTracker.h"
#include "GuHeightFieldUtil.h"
#include "GuDebug.h"
#include "GuTriangleMesh.h"
#include "GuConvexMeshData.h"
#include "GuTriangleMeshData.h"
#include "GuHeightField.h"
#include "PxsContext.h"
#include "PxsAABBManager.h"
#include "PxsTransformCache.h"
#include "CmTransformUtils.h"

using namespace physx;

// PT: keep local functions in cpp, no need to pollute the header. Don't force conversions to bool if not necessary.
static PX_FORCE_INLINE Ps::IntBool hasTriggerFlags(PxShapeFlags flags)	{ return flags & PxShapeFlag::eTRIGGER_SHAPE ? 1 : 0;									}
static PX_FORCE_INLINE Ps::IntBool isBroadPhase(PxShapeFlags flags)		{ return flags & (PxShapeFlag::eTRIGGER_SHAPE|PxShapeFlag::eSIMULATION_SHAPE) ? 1 : 0;	}


Sc::ShapeSim::ShapeSim(RigidSim& owner, const ShapeCore& core, PxsRigidBody* atom, PxBounds3* outBounds)	:
	ElementSim			(owner, PX_ELEMENT_TYPE_SHAPE),
	mTransformCacheId	(PX_INVALID_U32),
	mCore				(core)
{
	// sizeof(ShapeSim) = 32 bytes
	Sc::Scene& scScene = getScene();

	PX_ASSERT(&owner);
	{
		CM_PROFILE_ZONE_WITH_SUBSYSTEM(scScene, API, simAddShapeToBroadPhase);
		if(isBroadPhase(core.getFlags()))
		{
			PX_ASSERT(PX_INVALID_BP_HANDLE==getAABBMgrId().mShapeHandle);
			PxBounds3 bounds = computeWorldBounds(core, owner, outBounds);
			scScene.addBroadPhaseVolume(bounds, owner.getBroadphaseGroupId(), atom ? atom->getAABBMgrId() : AABBMgrId(), *this);

			if(atom)
				atom->setAABBMgrId(getAABBMgrId());
		}
	}

	mId = scScene.getShapeIDTracker().createID();
}

Sc::ShapeSim::~ShapeSim()
{
	PX_ASSERT((!hasAABBMgrHandle()) || (PX_INVALID_BP_HANDLE==getAABBMgrId().mShapeHandle));

	Sc::Scene& scScene = getScene();
	scScene.getShapeIDTracker().releaseID(mId);
}

void Sc::ShapeSim::createTransformCache(PxsTransformCache& cache)
{
	if(mTransformCacheId == PX_INVALID_U32)
	{
		//Create transform cache entry
		PxU32 index = cache.createID();
		cache.setTransformCache(getAbsPose(), index);
		mTransformCacheId = index;
	}
	cache.incReferenceCount(mTransformCacheId);
}

void Sc::ShapeSim::destroyTransformCache(PxsTransformCache& cache)
{
	PX_ASSERT(mTransformCacheId != PX_INVALID_U32);
	if(cache.decReferenceCount(mTransformCacheId))
	{
		cache.releaseID(mTransformCacheId);
		mTransformCacheId = PX_INVALID_U32;
	}
}


void Sc::ShapeSim::createLowLevelVolume(const PxU32 group, const PxBounds3& bounds, const PxU32 aggregateID, AABBMgrId aabbMgrId)
{
	//Add the volume to the aabb manager.

	if(!Element::createLowLevelVolume(group, bounds, aggregateID, aabbMgrId))
		return;

	const PxsShapeCore& shapeCore = getCore().getCore();
	const Gu::GeometryUnion &geometry = shapeCore.geometry;
	const PxBounds3* localSpaceAABB = NULL;
	switch (geometry.getType())
	{
	case PxGeometryType::eCONVEXMESH:
		localSpaceAABB = &geometry.get<const PxConvexMeshGeometryLL>().hullData->mAABB;
		break;
	case PxGeometryType::eTRIANGLEMESH:
		localSpaceAABB = &geometry.get<const PxTriangleMeshGeometryLL>().meshData->mAABB;
		break;
	case PxGeometryType::eHEIGHTFIELD:
		localSpaceAABB = &geometry.get<const PxHeightFieldGeometryLL>().heightFieldData->mAABB;											
		break;
	case PxGeometryType::eSPHERE:
	case PxGeometryType::ePLANE:
	case PxGeometryType::eCAPSULE:
	case PxGeometryType::eBOX:
	case PxGeometryType::eGEOMETRY_COUNT:
	case PxGeometryType::eINVALID:
	default:
		break;
	}

	Sc::Actor& actor = getScActor();
	PxsContext* llContext = getInteractionScene().getLowLevelContext();
	if(actor.isDynamicRigid())
	{
		PxcAABBDataDynamic aabbData;
		aabbData.mShapeCore = &shapeCore;
		aabbData.mLocalSpaceAABB = localSpaceAABB;
		aabbData.mRigidCore = &static_cast<BodySim&>(actor).getBodyCore().getCore();
		aabbData.mBodyAtom = &static_cast<BodySim*>(&actor)->getLowLevelBody();
		llContext->getAABBManager()->setDynamicAABBData(getAABBMgrId().mShapeHandle, aabbData);
	}
	else
	{
		PxcAABBDataStatic aabbData;
		aabbData.mShapeCore = &shapeCore;
		aabbData.mRigidCore = &static_cast<StaticSim&>(actor).getStaticCore().getCore();
		llContext->getAABBManager()->setStaticAABBData(getAABBMgrId().mShapeHandle, aabbData);
	}

	llContext->markShape(getAABBMgrId().mActorHandle);
}

bool Sc::ShapeSim::destroyLowLevelVolume()
{
	//Need to test that shape has entry in bp.  The shape might not have a bp
	//entry because it has its simulation flag set to false.
	const PxcBpHandle actorHandle = getAABBMgrId().mActorHandle;
	if(PX_INVALID_BP_HANDLE!=actorHandle)
	{
		getInteractionScene().getLowLevelContext()->unMarkShape(actorHandle);
	}
	bool removingLastShape=Element::destroyLowLevelVolume();
	if(removingLastShape)
	{
		BodySim* b = getBodySim();
		if(b)
			b->getLowLevelBody().resetAABBMgrId();
	}
	return removingLastShape;
}

Sc::Scene& Sc::ShapeSim::getScene() const
{
	return getInteractionScene().getOwnerScene();
}

PX_FORCE_INLINE void Sc::ShapeSim::internalAddToBroadPhase()
{
	getScene().addBroadPhaseVolume(computeWorldBounds(getCore(), getRbSim(), NULL), *this);
}

PX_FORCE_INLINE void Sc::ShapeSim::internalRemoveFromBroadPhase()
{
	getScene().removeBroadPhaseVolume((PxU32)PairReleaseFlag::eWAKE_ON_LOST_TOUCH, *this);
}

void Sc::ShapeSim::removeFromBroadPhase(bool wakeOnLostTouch)
{
	if(hasAABBMgrHandle())
	{
		PxU32 flags;
		if (wakeOnLostTouch)
			flags = (PxU32)PairReleaseFlag::eWAKE_ON_LOST_TOUCH;
		else
			flags = 0;

		getScene().removeBroadPhaseVolume(flags, *this);
		PX_ASSERT(PX_INVALID_BP_HANDLE==getAABBMgrId().mShapeHandle);
	}
}

void Sc::ShapeSim::reinsertBroadPhase()
{
	internalRemoveFromBroadPhase();
	internalAddToBroadPhase();
}

void Sc::ShapeSim::onFilterDataChange()
{
	setElementInteractionsDirty(CoreInteraction::CIF_DIRTY_FILTER_STATE, PX_INTERACTION_FLAG_FILTERABLE);
}

void Sc::ShapeSim::onResetFiltering()
{
	if(hasAABBMgrHandle())
		internalRemoveFromBroadPhase();

	if(isBroadPhase(mCore.getFlags()))
	{
		internalAddToBroadPhase();
		PX_ASSERT(getAABBMgrId().mShapeHandle!=PX_INVALID_BP_HANDLE);
		Sc::BodySim* bs = getBodySim();
		if(bs)
			bs->getLowLevelBody().setAABBMgrId(getAABBMgrId());
	}
}

void Sc::ShapeSim::onMaterialChange()
{
	setElementInteractionsDirty(CoreInteraction::CIF_DIRTY_MATERIAL, PX_INTERACTION_FLAG_RB_ELEMENT);
}

void Sc::ShapeSim::onRestOffsetChange()
{
	setElementInteractionsDirty(CoreInteraction::CIF_DIRTY_REST_OFFSET, PX_INTERACTION_FLAG_RB_ELEMENT);
}

void Sc::ShapeSim::onFlagChange(PxShapeFlags oldFlags)
{
	PxShapeFlags newFlags = mCore.getFlags();

	if (hasTriggerFlags(newFlags) != hasTriggerFlags(oldFlags))
	{
		setElementInteractionsDirty(CoreInteraction::CIF_DIRTY_FILTER_STATE, PX_INTERACTION_FLAG_FILTERABLE);
	}

	// Change of collision shape flag requires removal/add to broadphase
	const Ps::IntBool oldBp = isBroadPhase(oldFlags);
	const Ps::IntBool newBp = isBroadPhase(newFlags);

	if (!oldBp && newBp)
	{
		PX_ASSERT(!hasAABBMgrHandle());
		internalAddToBroadPhase();
		Sc::BodySim* bs = getBodySim();
		if(bs)
			bs->getLowLevelBody().setAABBMgrId(getAABBMgrId());
	}
	else if (oldBp && !newBp)
	{
		internalRemoveFromBroadPhase();
	}
}

void Sc::ShapeSim::getFilterInfo(PxFilterObjectAttributes& filterAttr, PxFilterData& filterData) const
{
	filterAttr = 0;
	const PxShapeFlags flags = mCore.getFlags();

	if (hasTriggerFlags(flags))
		filterAttr |= PxFilterObjectFlag::eTRIGGER;

	BodySim* b = getBodySim();
	if (b)
	{
		if (!b->isArticulationLink())
		{
			if (b->isKinematic())
				filterAttr |= PxFilterObjectFlag::eKINEMATIC;

			setFilterObjectAttributeType(filterAttr, PxFilterObjectType::eRIGID_DYNAMIC);
		}
		else
			setFilterObjectAttributeType(filterAttr, PxFilterObjectType::eARTICULATION);
	}
	else
	{
		setFilterObjectAttributeType(filterAttr, PxFilterObjectType::eRIGID_STATIC);
	}

	filterData = mCore.getSimulationFilterData();
}

PxTransform Sc::ShapeSim::getAbsPose() const
{
	const PxTransform& shape2Actor = getCore().getCore().transform;
	if(getActorSim().getActorType()==PxActorType::eRIGID_STATIC)
	{
		PxsRigidCore& core = static_cast<StaticSim&>(getScActor()).getStaticCore().getCore();
		return core.body2World.transform(shape2Actor);
	}
	else
	{
		PxsBodyCore& core = static_cast<BodySim&>(getScActor()).getBodyCore().getCore();
		return core.body2World.transform(core.body2Actor.getInverse()).transform(shape2Actor);
	}
		       
}

Sc::RigidSim& Sc::ShapeSim::getRbSim() const	
{ 
	return static_cast<RigidSim&>(getScActor());
}

Sc::BodySim* Sc::ShapeSim::getBodySim() const	
{ 
	Actor& a = getScActor();
	return a.isDynamicRigid() ? static_cast<BodySim*>(&a) : 0;
}

PxsRigidCore& Sc::ShapeSim::getPxsRigidCore() const
{
	Actor& a = getScActor();
	return a.isDynamicRigid() ? static_cast<BodySim&>(a).getBodyCore().getCore()
							  : static_cast<StaticSim&>(a).getStaticCore().getCore();
}

bool Sc::ShapeSim::actorIsDynamic() const
{
	return getScActor().isDynamicRigid();
}

PxBounds3 Sc::ShapeSim::computeWorldBounds(const ShapeCore& core, const RigidSim& actor, PxBounds3* uninflatedBounds) const
{
	const PxTransform& shape2Actor = core.getShape2Actor();
	PX_ALIGN(16, PxTransform) globalPose;
	
	if(actor.isDynamicRigid())
	{
		const PxsBodyCore& bodyCore = static_cast<const BodySim&>(actor).getBodyCore().getCore();
		Cm::getDynamicGlobalPoseAligned(bodyCore.body2World, shape2Actor, bodyCore.body2Actor, globalPose);
	}
	else
		Cm::getStaticGlobalPoseAligned(static_cast<const StaticSim&>(actor).getStaticCore().getCore().body2World, shape2Actor, globalPose);

	PxBounds3 bounds;
	PxReal contactOffset = core.getContactOffset();
	if(uninflatedBounds)
	{		
		core.getGeometryUnion().computeBounds(*uninflatedBounds, globalPose, 0, NULL);
		bounds.minimum = uninflatedBounds->minimum - PxVec3(contactOffset);
		bounds.maximum = uninflatedBounds->maximum + PxVec3(contactOffset);
	}
	else
		core.getGeometryUnion().computeBounds(bounds, globalPose, contactOffset, NULL);
	return bounds;
}

void Sc::ShapeSim::onTransformChange()
{
	InteractionScene& scene = getInteractionScene();
	
	const AABBMgrId aabbMgrId=getAABBMgrId();
	if(PX_INVALID_BP_HANDLE!=aabbMgrId.mActorHandle)
	{
		scene.getLowLevelContext()->markShape(aabbMgrId.mActorHandle);
		scene.getLowLevelContext()->getAABBManager()->setActorDirty(aabbMgrId.mActorHandle);
	}

	const bool isActorAsleep = !getActorSim().isActive();
	Element::ElementInteractionIterator iter = getElemInteractions();
	ElementInteraction* i = iter.getNext();
	while(i)
	{
		if(i->getType()==PX_INTERACTION_TYPE_OVERLAP)
		{
			Sc::ShapeInstancePairLL* sip = static_cast<Sc::ShapeInstancePairLL*>(i);
			sip->resetManagerCachedState();

			if (isActorAsleep)
				sip->onPoseChangedWhileSleeping();
		}
		else if (i->getType()==PX_INTERACTION_TYPE_TRIGGER)
			(static_cast<Sc::TriggerInteraction*>(i))->forceProcessingThisFrame(scene);  // trigger pairs need to be checked next frame
#if PX_USE_PARTICLE_SYSTEM_API
		else if (i->getType()==PX_INTERACTION_TYPE_PARTICLE_BODY)
			(static_cast<Sc::ParticleElementRbElementInteraction *>(i))->onRbShapeChange();
#endif

		i = iter.getNext();
	}
	
	PxsContext* context = getInteractionScene().getLowLevelContext();

	context->onShapeChange(mCore.getCore(), getPxsRigidCore(), actorIsDynamic());

	if(getTransformCacheID() != PX_INVALID_U32)
	{
		//Update transforms
		PxsTransformCache& cache = context->getTransformCache();
		cache.setTransformCache(getAbsPose(), getTransformCacheID());
	}
}

void Sc::ShapeSim::onVolumeChange()
{
	const AABBMgrId aabbMgrId=getAABBMgrId();
	if(PX_INVALID_BP_HANDLE!=aabbMgrId.mActorHandle)
	{
		getInteractionScene().getLowLevelContext()->markShape(aabbMgrId.mActorHandle);
		getInteractionScene().getLowLevelContext()->getAABBManager()->setActorDirty(aabbMgrId.mActorHandle);
	}

	Element::ElementInteractionIterator iter = getElemInteractions();
	ElementInteraction* i = iter.getNext();
	while(i)
	{
#if PX_USE_PARTICLE_SYSTEM_API
		if (i->getType()==PX_INTERACTION_TYPE_PARTICLE_BODY)
			(static_cast<Sc::ParticleElementRbElementInteraction *>(i))->onRbShapeChange();
		else
#endif
		if (i->getType()==PX_INTERACTION_TYPE_OVERLAP)
			(static_cast<Sc::ShapeInstancePairLL *>(i))->resetManagerCachedState();

		i = iter.getNext();
	}

	getInteractionScene().getLowLevelContext()->onShapeChange(mCore.getCore(), getPxsRigidCore(), actorIsDynamic());
}

