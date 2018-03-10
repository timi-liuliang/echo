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


#include "NpShapeManager.h"
#include "NpFactory.h"
#include "ScbRigidObject.h"
#include "SqUtilities.h"
#include "NpActor.h"
#include "SqSceneQueryManager.h"
#include "NpScene.h"
#include "NpPtrTableStorageManager.h"

using namespace physx;

NpShapeManager::NpShapeManager()
{
}

// PX_SERIALIZATION
NpShapeManager::NpShapeManager(const PxEMPTY&) 
: mShapes(PxEmpty)
, mSceneQueryData(PxEmpty) 
{	
}

NpShapeManager::~NpShapeManager() 
{ 
	Cm::PtrTableStorageManager &sm = NpFactory::getInstance().getPtrTableStorageManager();
	mShapes.clear(sm);
	mSceneQueryData.clear(sm);
}


namespace
{
	bool isSceneQuery(NpShape& shape) { return shape.NpShape::getFlags() & PxShapeFlag::eSCENE_QUERY_SHAPE; }
}

void NpShapeManager::exportExtraData(PxSerializationContext& stream)
{ 
	mShapes.exportExtraData(stream);							
	mSceneQueryData.exportExtraData(stream);
}

void NpShapeManager::importExtraData(PxDeserializationContext& context)
{ 
	mShapes.importExtraData(context);	
	mSceneQueryData.importExtraData(context);
}
//~PX_SERIALIZATION

void NpShapeManager::attachShape(NpShape& shape,
								 PxRigidActor& actor)
{
	Cm::PtrTableStorageManager &sm = NpFactory::getInstance().getPtrTableStorageManager();

	PxU32 index = getNbShapes();
	mShapes.add(&shape, sm);	
	mSceneQueryData.add(NULL, sm);

	NpScene* scene = NpActor::getAPIScene(actor);		
	if(scene && isSceneQuery(shape))
		setupSceneQuery(scene->getSceneQueryManagerFast(), actor, index);

	Scb::RigidObject& ro = static_cast<Scb::RigidObject&>(NpActor::getScbFromPxActor(actor));
	ro.onShapeAttach(shape.getScbShape());

	PX_ASSERT(!shape.isExclusive() || shape.getActor()==NULL);
	shape.incRefCount();
	if(shape.isExclusive())
		shape.setActor(&actor);
}
				 
void NpShapeManager::detachShape(NpShape& s, 
								 PxRigidActor& actor,
								 bool wakeOnLostTouch)
{
	Cm::PtrTableStorageManager &sm = NpFactory::getInstance().getPtrTableStorageManager();

	PxU32 index = mShapes.find(&s);
	PX_ASSERT(index!=0xffffffff);

	Scb::RigidObject& ro = static_cast<Scb::RigidObject&>(NpActor::getScbFromPxActor(actor));

	NpScene* scene = NpActor::getAPIScene(actor);
	if(scene && isSceneQuery(s))
		scene->getSceneQueryManagerFast().removeShape(static_cast<Sq::ActorShape*>(mSceneQueryData.getPtrs()[index]));

	Scb::Shape& scbShape = s.getScbShape();
	ro.onShapeDetach(scbShape, wakeOnLostTouch, (s.getRefCount() == 1));
	mShapes.replaceWithLast(index, sm);
	mSceneQueryData.replaceWithLast(index, sm);

	if(s.NpShape::isExclusive())
		s.setActor(NULL);

	s.decRefCount();
}

bool NpShapeManager::shapeIsAttached(NpShape& s) const
{
	return mShapes.find(&s)!=0xffffffff;
}

void NpShapeManager::detachAll(NpScene* scene)
{
	// assumes all SQ data has been released, which is currently the responsbility of the owning actor
	const PxU32 nbShapes = getNbShapes();
	NpShape*const *shapes = getShapes();

	if(scene)
		teardownAllSceneQuery(scene->getSceneQueryManagerFast()); 

	// actor cleanup in Scb/Sc will remove any outstanding references corresponding to sim objects, so we don't need to do that here.
	for(PxU32 i=0;i<nbShapes;i++)
	{
		NpShape& shape = *shapes[i];
		if(shape.NpShape::isExclusive())
			shape.setActor(NULL);
		shape.decRefCount();
	}

	Cm::PtrTableStorageManager &sm = NpFactory::getInstance().getPtrTableStorageManager();

	mShapes.clear(sm);
	mSceneQueryData.clear(sm);
}


PxU32 NpShapeManager::getShapes(PxShape** buffer, PxU32 bufferSize, PxU32 startIndex) const
{
	const PxU32 size = getNbShapes();
	NpShape*const* PX_RESTRICT shapes = getShapes() + startIndex;

	const PxU32 remainder = (PxU32)PxMax<PxI32>(PxI32(size - startIndex), 0);
	const PxU32 writeCount = (PxU32)PxMin(remainder, bufferSize);
	PxMemCopy(buffer, shapes, writeCount*sizeof(PxShape*));

	return writeCount;
}

PxBounds3 NpShapeManager::getWorldBounds(const PxRigidActor& actor) const
{
	PxBounds3 bounds(PxBounds3::empty());

	const PxU32 nbShapes = getNbShapes();
	PxTransform actorPose = actor.getGlobalPose();
	NpShape*const* PX_RESTRICT shapes = getShapes();

	for(PxU32 i=0; i<nbShapes; i++)
		bounds.include(shapes[i]->getGeometryFast().computeBounds(actorPose*shapes[i]->getLocalPoseFast()));
		
	return bounds;
}


void NpShapeManager::clearShapesOnRelease(Scb::Scene& s, PxRigidActor& r)
{
	PX_ASSERT(static_cast<Scb::RigidObject&>(NpActor::getScbFromPxActor(r)).isSimDisabledInternally());
	
	const PxU32 nbShapes = getNbShapes();
	NpShape*const* PX_RESTRICT shapes = getShapes();

	for(PxU32 i=0; i < nbShapes; i++)
	{
		Scb::Shape& scbShape = shapes[i]->getScbShape();
		scbShape.checkUpdateOnRemove<false>(&s);
		s.removeShapeFromPvd(scbShape, r);
	}
}

void NpShapeManager::releaseExclusiveUserReferences()
{
	// when the factory is torn down, release any shape owner refs that are still outstanding
	const PxU32 nbShapes = getNbShapes();
	NpShape*const* PX_RESTRICT shapes = getShapes();
	for(PxU32 i=0;i<nbShapes;i++)
	{
		if(shapes[i]->isExclusive() && shapes[i]->getRefCount()>1)
			shapes[i]->release();
	}
}

void NpShapeManager::setupSceneQuery(Sq::SceneQueryManager& sqManager, const PxRigidActor& actor, const NpShape& shape)
{ 
	PX_ASSERT(shape.getFlags() & PxShapeFlag::eSCENE_QUERY_SHAPE);
	PxU32 index = mShapes.find(&shape);
	PX_ASSERT(index!=0xffffffff);

	setupSceneQuery(sqManager, actor, index);
}


void NpShapeManager::teardownSceneQuery(Sq::SceneQueryManager& sqManager, const NpShape& shape)
{
	PxU32 index = mShapes.find(&shape);
	PX_ASSERT(index!=0xffffffff);
	teardownSceneQuery(sqManager, index);
}

void NpShapeManager::setupAllSceneQuery(const PxRigidActor& actor)
{ 
	NpScene* scene = NpActor::getAPIScene(actor); 
	PX_ASSERT(scene);		// shouldn't get here unless we're in a scene
	Sq::SceneQueryManager& sqManager = scene->getSceneQueryManagerFast();

	const PxU32 nbShapes = getNbShapes();
	NpShape*const *shapes = getShapes();

	const PxType actorType = actor.getConcreteType();
	const bool isDynamic = actorType == PxConcreteType::eRIGID_DYNAMIC || actorType == PxConcreteType::eARTICULATION_LINK;

	for(PxU32 i=0;i<nbShapes;i++)
	{
		if(isSceneQuery(*shapes[i]))
			mSceneQueryData.getPtrs()[i] = sqManager.addShape(*shapes[i], actor, isDynamic);
	}
}

void NpShapeManager::teardownAllSceneQuery(Sq::SceneQueryManager& sqManager)
{
	NpShape*const *shapes = getShapes();
	const PxU32 nbShapes = getNbShapes();
	Sq::ActorShape**sqData = getSqDataInternal();

	for(PxU32 i=0;i<nbShapes;i++)
	{
		if(i < nbShapes - 1)
			Ps::prefetch(shapes[i+1],sizeof(NpShape));

		if(isSceneQuery(*shapes[i]))
			sqManager.removeShape(sqData[i]);
		sqData[i] = NULL;
	}
}

void NpShapeManager::markAllSceneQueryForUpdate(Sq::SceneQueryManager& sqManager)
{
	const PxU32 nbShapes = getNbShapes();
	Sq::ActorShape*const *sqData = getSceneQueryData();

	for(PxU32 i=0; i<nbShapes; i++)
	{
		if(sqData[i])
			sqManager.markForUpdate(sqData[i]);
	}
}

Sq::ActorShape* NpShapeManager::findSceneQueryData(const NpShape& shape)
{
	PxU32 index = mShapes.find(&shape);
	PX_ASSERT(index!=0xffffffff);

	return getSceneQueryData()[index];
}

//
// internal methods
// 

void NpShapeManager::setupSceneQuery(Sq::SceneQueryManager& sqManager, const PxRigidActor& actor, PxU32 index)
{ 
	const PxType actorType = actor.getConcreteType();
	const bool isDynamic = actorType == PxConcreteType::eRIGID_DYNAMIC || actorType == PxConcreteType::eARTICULATION_LINK;
	mSceneQueryData.getPtrs()[index] = sqManager.addShape(*(getShapes()[index]), actor, isDynamic);
}

void NpShapeManager::teardownSceneQuery(Sq::SceneQueryManager& sqManager, PxU32 index)
{
	sqManager.removeShape((getSqDataInternal()[index]));
	mSceneQueryData.getPtrs()[index] = NULL;
}

#if PX_ENABLE_DEBUG_VISUALIZATION
void NpShapeManager::visualize(Cm::RenderOutput& out, NpScene* scene, const PxRigidActor& actor)
{
	const PxU32 nbShapes = getNbShapes();
	NpShape*const* PX_RESTRICT shapes = getShapes();
	PxTransform actorPose = actor.getGlobalPose();

	const bool visualizeCompounds = (nbShapes>1) && scene->getVisualizationParameter(PxVisualizationParameter::eCOLLISION_COMPOUNDS)!=0.0f;

	PxBounds3 compoundBounds(PxBounds3::empty());
	for(PxU32 i=0; i<nbShapes; i++)
	{
		Scb::Shape& shape = shapes[i]->getScbShape();
		if(shape.getFlags() & PxShapeFlag::eVISUALIZATION)
		{
			shapes[i]->visualize(out, actor);
			if(visualizeCompounds)
				compoundBounds.include(shapes[i]->getGeometryFast().computeBounds(actorPose*shapes[i]->getLocalPose()));
		}
	}
	if(visualizeCompounds && !compoundBounds.isEmpty())
		out << PxU32(PxDebugColor::eARGB_MAGENTA) << PxMat44(PxIdentity) << Cm::DebugBox(compoundBounds);
}
#endif  // PX_ENABLE_DEBUG_VISUALIZATION
