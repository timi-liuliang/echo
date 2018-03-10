/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef __SAMPLE_SPHERE_ACTOR_H__
#define __SAMPLE_SPHERE_ACTOR_H__

#include "SampleShapeActor.h"
#include "RendererCapsuleShape.h"

#if NX_SDK_VERSION_MAJOR == 2
#include "NxActor.h"
#include "NxActorDesc.h"
#include "NxBodyDesc.h"
#include "NxSphereShapeDesc.h"
#include "NxScene.h"
#include "NxFromPx.h"
#elif NX_SDK_VERSION_MAJOR == 3
#include "PxActor.h"
#include "PxScene.h"
#include "PxRigidDynamic.h"
#include "geometry/PxSphereGeometry.h"
#include "extensions/PxExtensionsAPI.h"
namespace physx
{
class PxMaterial;
}
#endif

#include <Renderer.h>
#include <RendererMeshContext.h>

#if NX_SDK_VERSION_MAJOR == 2

class SampleSphereActor : public SampleShapeActor
{
public:
	SampleSphereActor(SampleRenderer::Renderer* renderer,
	                  SampleFramework::SampleMaterialAsset& material,
	                  NxScene& physxScene,
	                  const physx::PxVec3& pos,
	                  const physx::PxVec3& vel,
	                  const physx::PxVec3& radius,
	                  physx::PxF32 density,
	                  void* nxmaterial,
	                  bool useGroupsMask,
	                  physx::apex::NxApexRenderDebug* rdebug = NULL)
		: SampleShapeActor(rdebug)
		, mRendererCapsuleShape(NULL)
		, mRadius(radius)
	{
		PX_UNUSED(nxmaterial);
		mRenderer = renderer;

		createActor(physxScene, pos, vel, mRadius, density, useGroupsMask);

		mRendererCapsuleShape = new SampleRenderer::RendererCapsuleShape(*mRenderer, 0, radius.x);

		mRendererMeshContext.material         = material.getMaterial();
		mRendererMeshContext.materialInstance = material.getMaterialInstance();
		mRendererMeshContext.mesh             = mRendererCapsuleShape->getMesh();
		mRendererMeshContext.transform        = &mTransform;

		if (rdebug)
		{
			mBlockId = rdebug->beginDrawGroup(mTransform);
			rdebug->addToCurrentState(physx::DebugRenderState::SolidShaded);
			static physx::PxU32 scount /* = 0 */;
			rdebug->setCurrentColor(0xFFFFFF);
			rdebug->setCurrentTextScale(0.5f);
			rdebug->addToCurrentState(physx::DebugRenderState::CenterText);
			rdebug->addToCurrentState(physx::DebugRenderState::CameraFacing);
			rdebug->debugText(physx::PxVec3(0, 1.0f + 0.01f, 0), "Sample Sphere:%d", scount++);
			rdebug->endDrawGroup();
		}
	}

	virtual ~SampleSphereActor(void)
	{
		if (mRendererCapsuleShape != NULL)
		{
			delete mRendererCapsuleShape;
			mRendererCapsuleShape = NULL;
		}
	}

private:
	void createActor(NxScene& physxScene,
	                 const physx::PxVec3& pos,
	                 const physx::PxVec3& vel,
	                 const physx::PxVec3& radius,
	                 physx::PxF32 density,
	                 bool useGroupsMask)
	{
		mTransform = physx::PxMat44::createIdentity();
		mTransform.setPosition(pos);

		NxBodyDesc			bodyDesc;
		NxSphereShapeDesc	shapeDesc;
		NxActorDesc			actorDesc;

		physx::apex::NxFromPxVec3(bodyDesc.linearVelocity, vel);
		bodyDesc.flags          = NX_BF_VISUALIZATION;

		shapeDesc.radius		= radius.x;
		shapeDesc.shapeFlags	= NX_SF_VISUALIZATION | NX_SF_CLOTH_TWOWAY;
		shapeDesc.materialIndex	= 0;

		if (useGroupsMask)
		{
			shapeDesc.groupsMask.bits0 = 1;
			shapeDesc.groupsMask.bits2 = ~0;
		}

		physx::apex::NxFromPxMat34(actorDesc.globalPose, mTransform);
		if (density > 0)
		{
			actorDesc.body          = &bodyDesc;
			actorDesc.density       = density;
		}
		actorDesc.flags = 0;
		actorDesc.shapes.push_back(&shapeDesc);

		mPhysxActor = physxScene.createActor(actorDesc);
		PX_ASSERT(mPhysxActor);
	}

private:
	SampleRenderer::RendererCapsuleShape* mRendererCapsuleShape;
	physx::PxVec3 mRadius;
};

#elif NX_SDK_VERSION_MAJOR == 3

class SampleSphereActor : public SampleShapeActor
{
public:
	SampleSphereActor(SampleRenderer::Renderer* renderer,
	                  SampleFramework::SampleMaterialAsset& material,
	                  physx::PxScene& physxScene,
	                  const physx::PxVec3& pos,
	                  const physx::PxVec3& vel,
	                  const physx::PxVec3& radius,
	                  physx::PxF32 density,
	                  physx::PxMaterial* pxmaterial,
	                  bool useGroupsMask,
	                  physx::apex::NxApexRenderDebug* rdebug = NULL)
		: SampleShapeActor(rdebug)
		, mRendererCapsuleShape(NULL)
		, mRadius(radius)
	{
		mRenderer = renderer;
		if (!pxmaterial)
			physxScene.getPhysics().getMaterials(&pxmaterial, 1);
		createActor(physxScene, pos, vel, mRadius, density, pxmaterial, useGroupsMask);

		mRendererCapsuleShape = new SampleRenderer::RendererCapsuleShape(*mRenderer, 0, radius.x);

		mRendererMeshContext.material         = material.getMaterial();
		mRendererMeshContext.materialInstance = material.getMaterialInstance();
		mRendererMeshContext.mesh             = mRendererCapsuleShape->getMesh();
		mRendererMeshContext.transform        = &mTransform;

		if (rdebug)
		{
			mBlockId = rdebug->beginDrawGroup(mTransform);
			rdebug->addToCurrentState(physx::DebugRenderState::SolidShaded);
			static physx::PxU32 scount /* = 0 */;
			rdebug->setCurrentColor(0xFFFFFF);
			rdebug->setCurrentTextScale(0.5f);
			rdebug->addToCurrentState(physx::DebugRenderState::CenterText);
			rdebug->addToCurrentState(physx::DebugRenderState::CameraFacing);
			rdebug->debugText(physx::PxVec3(0, 1.0f + 0.01f, 0), "Sample Sphere:%d", scount++);
			rdebug->endDrawGroup();
		}
	}

	virtual ~SampleSphereActor()
	{
		if (mRendererCapsuleShape)
		{
			delete mRendererCapsuleShape;
			mRendererCapsuleShape = NULL;
		}
	}

private:
	void createActor(physx::PxScene& physxScene,
	                 const physx::PxVec3& pos,
	                 const physx::PxVec3& vel,
	                 const physx::PxVec3& extents,
	                 physx::PxF32 density,
	                 physx::PxMaterial* pxmaterial,
	                 bool useGroupsMask)
	{
		mTransform = physx::PxMat44::createIdentity();
		mTransform.setPosition(pos);

		physx::PxRigidDynamic* actor = physxScene.getPhysics().createRigidDynamic(physx::PxTransform(mTransform));
		PX_ASSERT(actor);
		actor->setAngularDamping(0.5f);
		actor->setLinearVelocity(vel);

		physx::PxSphereGeometry sphereGeom(extents.x);
		physx::PxShape* shape = actor->createShape(sphereGeom, *pxmaterial);
		PX_ASSERT(shape);
		if (shape && useGroupsMask)
		{
			shape->setSimulationFilterData(physx::PxFilterData(1, 0, ~0u, 0));
			shape->setQueryFilterData(physx::PxFilterData(1, 0, ~0u, 0));
		}

		if (density > 0)
		{
			physx::PxRigidBodyExt::updateMassAndInertia(*actor, density);
		}
		else
		{
			actor->setMass(1.0f);
		}
		SCOPED_PHYSX3_LOCK_WRITE(&physxScene);
		physxScene.addActor(*actor);
		mPhysxActor = actor;
	}

private:
	SampleRenderer::RendererCapsuleShape* mRendererCapsuleShape;
	physx::PxVec3 mRadius;
};

#endif

#endif
