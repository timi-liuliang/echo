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

#include "PxBase.h"
#include "PsArray.h"
#include "PxShape.h"
#include "PxConvexMesh.h"
#include "PxTriangleMesh.h"
#include "PxHeightField.h"
#include "PxMaterial.h"
#include "PxJoint.h"
#include "PxConstraintExt.h"
#include "PxArticulation.h"
#include "PxAggregate.h"
#include "PxPhysics.h"
#include "PxScene.h"
#include "PxCollectionExt.h"


using namespace physx;

void PxCollectionExt::releaseObjects(PxCollection& collection)
{
	shdfnd::Array<PxBase*> releasableObjects;

	for (PxU32 i = 0; i < collection.getNbObjects(); ++i)
	{	
		PxBase* s = &collection.getObject(i);
		if(s->isReleasable())
			releasableObjects.pushBack(s);
	}

	for (PxU32 i = 0; i < releasableObjects.size(); ++i)
		releasableObjects[i]->release();		

	while (collection.getNbObjects() > 0)
		collection.remove(collection.getObject(0));
}


void PxCollectionExt::remove(PxCollection& collection, PxType concreteType, PxCollection* to)
{	
	shdfnd::Array<PxBase*> removeObjects;
	
	for (PxU32 i = 0; i < collection.getNbObjects(); i++)
	{
		PxBase& object = collection.getObject(i);
		if(concreteType == object.getConcreteType())
		{
			if(to)
			   to->add(object);	

			removeObjects.pushBack(&object);
		}
	}

	for (PxU32 i = 0; i < removeObjects.size(); ++i)
		collection.remove(*removeObjects[i]);
}

PxCollection* PxCollectionExt::createCollection(PxPhysics& physics)
{
	PxCollection* collection = PxCreateCollection();
	if (!collection)
		return NULL;

	// Collect convexes
	{
		shdfnd::Array<PxConvexMesh*> objects(physics.getNbConvexMeshes());
		const PxU32 nb = physics.getConvexMeshes(objects.begin(), objects.size());
		PX_ASSERT(nb == objects.size());
		PX_UNUSED(nb);

		for(PxU32 i=0;i<objects.size();i++)
			collection->add(*objects[i]);
	}

	// Collect triangle meshes
	{
		shdfnd::Array<PxTriangleMesh*> objects(physics.getNbTriangleMeshes());
		const PxU32 nb = physics.getTriangleMeshes(objects.begin(), objects.size());

		PX_ASSERT(nb == objects.size());
		PX_UNUSED(nb);

		for(PxU32 i=0;i<objects.size();i++)
			collection->add(*objects[i]);
	}

	// Collect heightfields
	{
		shdfnd::Array<PxHeightField*> objects(physics.getNbHeightFields());
		const PxU32 nb = physics.getHeightFields(objects.begin(), objects.size());

		PX_ASSERT(nb == objects.size());
		PX_UNUSED(nb);

		for(PxU32 i=0;i<objects.size();i++)
			collection->add(*objects[i]);
	}

	// Collect materials
	{
		shdfnd::Array<PxMaterial*> objects(physics.getNbMaterials());
		const PxU32 nb = physics.getMaterials(objects.begin(), objects.size());

		PX_ASSERT(nb == objects.size());
		PX_UNUSED(nb);

		for(PxU32 i=0;i<objects.size();i++)
			collection->add(*objects[i]);
	}

#if PX_USE_CLOTH_API
	// Collect cloth fabrics
	{
		shdfnd::Array<PxClothFabric*> objects(physics.getNbClothFabrics());
		const PxU32 nb = physics.getClothFabrics(objects.begin(), objects.size());

		PX_ASSERT(nb == objects.size());
		PX_UNUSED(nb);

		for(PxU32 i=0;i<objects.size();i++)
			collection->add(*objects[i]);
	}
#endif

	// Collect shapes
	{
		shdfnd::Array<PxShape*> objects(physics.getNbShapes());
		const PxU32 nb = physics.getShapes(objects.begin(), objects.size());

		PX_ASSERT(nb == objects.size());
		PX_UNUSED(nb);

		for(PxU32 i=0;i<objects.size();i++)
			collection->add(*objects[i]);
	}
	return collection;
}

PxCollection* PxCollectionExt::createCollection(PxScene& scene)
{
	PxCollection* collection = PxCreateCollection();
	if (!collection)
		return NULL;

	// Collect actors
	{
		PxActorTypeFlags selectionFlags = PxActorTypeFlag::eRIGID_STATIC | PxActorTypeFlag::eRIGID_DYNAMIC;

#if PX_USE_PARTICLE_SYSTEM_API
		selectionFlags |= PxActorTypeFlag::ePARTICLE_SYSTEM | PxActorTypeFlag::ePARTICLE_FLUID;
#endif
#if PX_USE_CLOTH_API
		selectionFlags |= PxActorTypeFlag::eCLOTH;
#endif

		shdfnd::Array<PxActor*> objects(scene.getNbActors(selectionFlags));
		const PxU32 nb = scene.getActors(selectionFlags, objects.begin(), objects.size());

		PX_ASSERT(nb==objects.size());
		PX_UNUSED(nb);

		for(PxU32 i=0;i<objects.size();i++)
			collection->add(*objects[i]);
	}


	// Collect constraints
	{
		shdfnd::Array<PxConstraint*> objects(scene.getNbConstraints());
		const PxU32 nb = scene.getConstraints(objects.begin(), objects.size());

		PX_ASSERT(nb==objects.size());
		PX_UNUSED(nb);

		for(PxU32 i=0;i<objects.size();i++)
		{
			PxU32 typeId;
			PxJoint* joint = reinterpret_cast<PxJoint*>(objects[i]->getExternalReference(typeId));
			if(typeId == PxConstraintExtIDs::eJOINT)
				collection->add(*joint);
		}
	}

	// Collect articulations
	{
		shdfnd::Array<PxArticulation*> objects(scene.getNbArticulations());
		const PxU32 nb = scene.getArticulations(objects.begin(), objects.size());

		PX_ASSERT(nb==objects.size());
		PX_UNUSED(nb);

		for(PxU32 i=0;i<objects.size();i++)
			collection->add(*objects[i]);
	}

	// Collect aggregates
	{
		shdfnd::Array<PxAggregate*> objects(scene.getNbAggregates());
		const PxU32 nb = scene.getAggregates(objects.begin(), objects.size());

		PX_ASSERT(nb==objects.size());
		PX_UNUSED(nb);

		for(PxU32 i=0;i<objects.size();i++)
			collection->add(*objects[i]);
	}

	return collection;
}
