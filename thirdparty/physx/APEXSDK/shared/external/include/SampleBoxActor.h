/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef __SAMPLE_BOX_ACTOR_H__
#define __SAMPLE_BOX_ACTOR_H__

#include "SampleShapeActor.h"
#include "RendererBoxShape.h"

#if NX_SDK_VERSION_MAJOR == 2
#include "NxActor.h"
#include "NxActorDesc.h"
#include "NxBodyDesc.h"
#include "NxBoxShapeDesc.h"
#include "NxScene.h"
#include "NxFromPx.h"
#elif NX_SDK_VERSION_MAJOR == 3
#include "PxPhysics.h"
#include "PxActor.h"
#include "PxScene.h"
#include "PxRigidDynamic.h"
#include "PxRigidStatic.h"
#include "geometry/PxBoxGeometry.h"
#include "extensions/PxExtensionsAPI.h"
namespace physx
{
class PxMaterial;
}
#endif

#include "NxApexRenderDebug.h"
#include <Renderer.h>
#include <RendererMeshContext.h>

#if NX_SDK_VERSION_MAJOR == 2

class SampleBoxActor : public SampleShapeActor
{
public:
	SampleBoxActor(SampleRenderer::Renderer* renderer,
	               SampleFramework::SampleMaterialAsset& material,
	               NxScene& physxScene,
	               const physx::PxVec3& pos,
	               const physx::PxVec3& vel,
	               const physx::PxVec3& extents,
	               physx::PxF32 density,
	               void* nxmaterial,
	               bool useGroupsMask,
	               physx::apex::NxApexRenderDebug* rdebug = NULL)
		: SampleShapeActor(rdebug)
		, mRendererBoxShape(NULL)
	{
		PX_UNUSED(nxmaterial);
		mRenderer = renderer;

		createActor(physxScene, pos, vel, extents, density, useGroupsMask);

		mRendererBoxShape = new SampleRenderer::RendererBoxShape(*mRenderer, extents);

		mRendererMeshContext.material         = material.getMaterial();
		mRendererMeshContext.materialInstance = material.getMaterialInstance();
		mRendererMeshContext.mesh             = mRendererBoxShape->getMesh();
		mRendererMeshContext.transform        = &mTransform;

		if (rdebug)
		{
			mBlockId = rdebug->beginDrawGroup(mTransform);
			rdebug->addToCurrentState(physx::DebugRenderState::SolidShaded);
			static physx::PxU32 bcount /* = 0 */;
			rdebug->setCurrentColor(0xFFFFFF);
			rdebug->setCurrentTextScale(0.5f);
			rdebug->addToCurrentState(physx::DebugRenderState::CenterText);
			rdebug->addToCurrentState(physx::DebugRenderState::CameraFacing);
			rdebug->debugText(physx::PxVec3(0, extents.y + 0.01f, 0), "Sample Box:%d", bcount++);
			rdebug->endDrawGroup();
		}
	}

	virtual ~SampleBoxActor()
	{
		if (mRendererBoxShape)
		{
			delete mRendererBoxShape;
			mRendererBoxShape = NULL;
		}
	}

private:
	void createActor(NxScene& physxScene,
	                 const physx::PxVec3& pos,
	                 const physx::PxVec3& vel,
	                 const physx::PxVec3& extents,
	                 physx::PxF32 density,
	                 bool useGroupsMask)
	{
		mTransform = physx::PxMat44::createIdentity();
		mTransform.setPosition(pos);

		NxBodyDesc     bodyDesc;
		NxBoxShapeDesc shapeDesc;
		NxActorDesc    actorDesc;

		physx::apex::NxFromPxVec3(bodyDesc.linearVelocity, vel);
		bodyDesc.flags = NX_BF_VISUALIZATION;

		physx::apex::NxFromPxVec3(shapeDesc.dimensions, extents);
		shapeDesc.shapeFlags = NX_SF_VISUALIZATION | NX_SF_CLOTH_TWOWAY;
		shapeDesc.materialIndex = 0;

		if (useGroupsMask)
		{
			shapeDesc.groupsMask.bits0 = 1;
			shapeDesc.groupsMask.bits2 = ~0;
		}

		physx::apex::NxFromPxMat34(actorDesc.globalPose, mTransform);
		if (density > 0)
		{
			actorDesc.body = &bodyDesc;
			actorDesc.density = density;
		}
		actorDesc.flags = 0;
		actorDesc.shapes.push_back(&shapeDesc);

		mPhysxActor = physxScene.createActor(actorDesc);
		PX_ASSERT(mPhysxActor);
	}

private:
	SampleRenderer::RendererBoxShape* mRendererBoxShape;
};

#elif NX_SDK_VERSION_MAJOR == 3

class SampleBoxActor : public SampleShapeActor
{
public:
	SampleBoxActor(SampleRenderer::Renderer* renderer,
	               SampleFramework::SampleMaterialAsset& material,
	               physx::PxScene& physxScene,
	               const physx::PxVec3& pos,
	               const physx::PxVec3& vel,
	               const physx::PxVec3& extents,
	               physx::PxF32 density,
	               physx::PxMaterial* pxmaterial,
	               bool useGroupsMask,
	               physx::apex::NxApexRenderDebug* rdebug = NULL)
		: SampleShapeActor(rdebug)
		, mRendererBoxShape(NULL)
	{
		mRenderer = renderer;
		if (!pxmaterial)
			physxScene.getPhysics().getMaterials(&pxmaterial, 1);
		createActor(physxScene, pos, vel, extents, density, pxmaterial, useGroupsMask);

		mRendererBoxShape = new SampleRenderer::RendererBoxShape(*mRenderer, extents);

		mRendererMeshContext.material         = material.getMaterial();
		mRendererMeshContext.materialInstance = material.getMaterialInstance();
		mRendererMeshContext.mesh             = mRendererBoxShape->getMesh();
		mRendererMeshContext.transform        = &mTransform;

		if (rdebug)
		{
			mBlockId = rdebug->beginDrawGroup(mTransform);
			rdebug->addToCurrentState(physx::DebugRenderState::SolidShaded);
			static physx::PxU32 bcount /* = 0 */;
			rdebug->setCurrentColor(0xFFFFFF);
			rdebug->setCurrentTextScale(0.5f);
			rdebug->addToCurrentState(physx::DebugRenderState::CenterText);
			rdebug->addToCurrentState(physx::DebugRenderState::CameraFacing);
			rdebug->debugText(physx::PxVec3(0, extents.y + 0.01f, 0), "Sample Box:%d", bcount++);
			rdebug->endDrawGroup();
		}
	}

	virtual ~SampleBoxActor()
	{
		if (mRendererBoxShape)
		{
			delete mRendererBoxShape;
			mRendererBoxShape = NULL;
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

		physx::PxRigidActor* actor = NULL;
		if (density > 0)
		{
			actor = physxScene.getPhysics().createRigidDynamic(physx::PxTransform(mTransform));
			((physx::PxRigidDynamic*)actor)->setAngularDamping(0.5f);
			((physx::PxRigidDynamic*)actor)->setLinearVelocity(vel);
		}
		else
		{
			actor = physxScene.getPhysics().createRigidStatic(physx::PxTransform(mTransform));
		}
		
		PX_ASSERT(actor);

		physx::PxBoxGeometry boxGeom(extents);
		physx::PxShape* shape = actor->createShape(boxGeom, *pxmaterial);
		PX_ASSERT(shape);
		if (shape && useGroupsMask)
		{
			shape->setSimulationFilterData(physx::PxFilterData(1u, 0u, ~0u, 0u));
			shape->setQueryFilterData(physx::PxFilterData(1u, 0u, ~0u, 0u));
		}

		if (density > 0)
		{
			physx::PxRigidBodyExt::updateMassAndInertia(*((physx::PxRigidDynamic*)actor), density);
		}
		SCOPED_PHYSX3_LOCK_WRITE(&physxScene);
		physxScene.addActor(*actor);
		mPhysxActor = actor;
	}

private:
	SampleRenderer::RendererBoxShape* mRendererBoxShape;
};

#endif // NX_SDK_VERSION_MAJOR == 3

#endif
