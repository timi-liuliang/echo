/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "RTdef.h"
#if RT_COMPILE
#include <PsUserAllocated.h>

#include "DestructibleActor.h"
#include "PxMaterial.h"

#include "Actor.h"
#include "Compound.h"
#include "Convex.h"
#include "CompoundCreator.h"
#include "Delaunay2d.h"
#include "Delaunay3d.h"
#include "PolygonTriangulator.h"
#include "IslandDetector.h"
#include "MeshClipper.h"
#include "FracturePattern.h"

#include "SimScene.h"

namespace physx
{
namespace fracture
{

void SimScene::onWake(PxActor** actors, PxU32 count){ 
	if(mAppNotify != NULL)
		mAppNotify->onWake(actors,count);
	
	for(physx::PxU32 i = 0; i < count; i++)
	{
		PxActor* actor = actors[i];
		if(actor != NULL && actor->isRigidDynamic())
		{
			physx::PxU32 shapeCount = actor->isRigidDynamic()->getNbShapes();
			PxShape** shapes = (PxShape**)PX_ALLOC(sizeof(PxShape*)*shapeCount,"onWake Shapes Temp Buffer");
			actor->isRigidDynamic()->getShapes(shapes,sizeof(PxShape*)*shapeCount,0);
			::physx::apex::destructible::DestructibleActor* prevActor = NULL;
			for(physx::PxU32 j = 0; j < shapeCount; j++)
			{
				physx::fracture::base::Convex* convex = findConvexForShape(*shapes[j]);
				if(convex == NULL || convex->getParent() == NULL)
					continue;
				physx::fracture::Compound* parent = (physx::fracture::Compound*)convex->getParent();
				::physx::apex::destructible::DestructibleActor* curActor = parent->getDestructibleActor();
				if(convex && convex->getParent() && curActor != prevActor && curActor)
				{
					curActor->incrementWakeCount();
					prevActor = curActor;
				}
				
			}
			PX_FREE(shapes);
		}
	}
}

void SimScene::onSleep(PxActor** actors, PxU32 count){ 
	if(mAppNotify != NULL)
		mAppNotify->onSleep(actors,count);
	
	for(physx::PxU32 i = 0; i < count; i++)
	{
		PxActor* actor = actors[i];
		if(actor != NULL && actor->isRigidDynamic())
		{
			physx::PxU32 shapeCount = actor->isRigidDynamic()->getNbShapes();
			PxShape** shapes = (PxShape**)PX_ALLOC(sizeof(PxShape*)*shapeCount,"onSleep Shapes Temp Buffer");
			actor->isRigidDynamic()->getShapes(shapes,sizeof(PxShape*)*shapeCount,0);
			::physx::apex::destructible::DestructibleActor* prevActor = NULL;
			for(physx::PxU32 j = 0; j < shapeCount; j++)
			{
				physx::fracture::base::Convex* convex = findConvexForShape(*shapes[j]);
				if(convex == NULL || convex->getParent() == NULL)
					continue;
				physx::fracture::Compound* parent = (physx::fracture::Compound*)convex->getParent();
				::physx::apex::destructible::DestructibleActor* curActor = parent->getDestructibleActor();
				if(convex && convex->getParent() && curActor != prevActor && curActor)
				{
					curActor->decrementWakeCount();;
					prevActor = curActor;
				}
				
			}
			PX_FREE(shapes);
		}
	}
}
SimScene* SimScene::createSimScene(PxPhysics *pxPhysics, PxCooking *pxCooking, PxScene *scene, float minConvexSize, PxMaterial* defaultMat, const char *resourcePath)
{
	SimScene* s = PX_NEW(SimScene)(pxPhysics,pxCooking,scene,minConvexSize,defaultMat,resourcePath);
	s->createSingletons();
	return s;
}

void SimScene::createSingletons()
{
	mCompoundCreator = PX_NEW(CompoundCreator)(this);
	mDelaunay2d = PX_NEW(Delaunay2d)(this);
	mDelaunay3d = PX_NEW(Delaunay3d)(this);
	mPolygonTriangulator = PX_NEW(PolygonTriangulator)(this);
	mIslandDetector = PX_NEW(IslandDetector)(this);
	mMeshClipper = PX_NEW(MeshClipper)(this);
	mDefaultGlass = PX_NEW(FracturePattern)(this);
	mDefaultGlass->createGlass(10.0f,0.25f,30,0.3f,0.03f,1.4f,0.3f);
	//mDefaultGlass->create3dVoronoi(PxVec3(10.0f, 10.0f, 10.0f), 50, 10.0f);
	addActor(createActor(NULL));
}

base::Actor* SimScene::createActor(::physx::apex::destructible::DestructibleActor* actor)
{
	return (base::Actor*)PX_NEW(Actor)(this,actor);
}

base::Convex* SimScene::createConvex()
{
	return (base::Convex*)PX_NEW(Convex)(this);
}

base::Compound* SimScene::createCompound(const base::FracturePattern *pattern, const base::FracturePattern *secondaryPattern, PxReal contactOffset, PxReal restOffset)
{
	return (base::Compound*)PX_NEW(Compound)(this,pattern,secondaryPattern,contactOffset,restOffset);
}

base::FracturePattern* SimScene::createFracturePattern()
{
	return (base::FracturePattern*)PX_NEW(FracturePattern)(this);
}

SimScene::SimScene(PxPhysics *pxPhysics, PxCooking *pxCooking, PxScene *scene, float minConvexSize, PxMaterial* defaultMat, const char *resourcePath):
		base::SimScene(pxPhysics,pxCooking,scene,minConvexSize,defaultMat,resourcePath) 
{
	if( mPxDefaultMaterial == NULL )
	{
		mPxDefaultMaterial = pxPhysics->createMaterial(0.5f,0.5f,0.1f);
	}
}

// --------------------------------------------------------------------------------------------
SimScene::~SimScene()
{
	PX_DELETE(mDefaultGlass);

	mDefaultGlass = NULL;
}

}
}
#endif