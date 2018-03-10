/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef __SAMPLE_PLANE_ACTOR_H__
#define __SAMPLE_PLANE_ACTOR_H__

#include "SampleShapeActor.h"

#if NX_SDK_VERSION_MAJOR == 2
#include "NxActor.h"
#include "NxActorDesc.h"
#include "NxBodyDesc.h"
#include "NxPlaneShapeDesc.h"
#include "NxScene.h"
#include "NxFromPx.h"
#elif NX_SDK_VERSION_MAJOR == 3
#include "PxActor.h"
#include "PxScene.h"
#include "PxRigidStatic.h"
#include "geometry/PxPlaneGeometry.h"
namespace physx
{
class PxMaterial;
}
#endif

#include <PsMathUtils.h>

#include <Renderer.h>
#include <RendererMeshContext.h>
#include <RendererGridShape.h>

#if NX_SDK_VERSION_MAJOR == 2

class SamplePlaneActor : public SampleShapeActor
{
public:
	SamplePlaneActor(SampleRenderer::Renderer* renderer,
	                 SampleFramework::SampleMaterialAsset& material,
	                 NxScene& physxScene,
	                 const physx::PxVec3& pos,
	                 const physx::PxVec3& normal,
	                 void* nxmaterial,
	                 bool useGroupsMask,
	                 physx::apex::NxApexRenderDebug* rdebug = NULL)
		: SampleShapeActor(rdebug)
		, mRendererGridShape(NULL)
	{
		PX_UNUSED(nxmaterial);
		mRenderer = renderer;

		createActor(physxScene, pos, normal, useGroupsMask);

		// default is Y_UP
		mRendererGridShape = new SampleRenderer::RendererGridShape(*mRenderer, 10, 1.0f, false, SampleRenderer::RendererGridShape::UP_Y);

		mRendererMeshContext.material         = material.getMaterial();
		mRendererMeshContext.materialInstance = material.getMaterialInstance();
		mRendererMeshContext.mesh             = mRendererGridShape->getMesh();
		mRendererMeshContext.transform        = &mTransform;

		if (rdebug)
		{
			mBlockId = rdebug->beginDrawGroup(mTransform);
			rdebug->addToCurrentState(physx::DebugRenderState::SolidShaded);
			static physx::PxU32 pcount /* = 0 */;
			rdebug->setCurrentColor(0xFFFFFF);
			rdebug->setCurrentTextScale(0.5f);
			rdebug->addToCurrentState(physx::DebugRenderState::CenterText);
			rdebug->addToCurrentState(physx::DebugRenderState::CameraFacing);
			rdebug->debugText(physx::PxVec3(0, 1.0f + 0.01f, 0), "Sample Plane:%d", pcount++);
			rdebug->endDrawGroup();
		}
	}

	virtual ~SamplePlaneActor()
	{
		if (mRendererGridShape)
		{
			delete mRendererGridShape;
			mRendererGridShape = NULL;
		}
	}

private:
	void createActor(NxScene& physxScene,
	                 const physx::PxVec3& pos,
	                 const physx::PxVec3& normal,
	                 bool useGroupsMask)
	{
		mTransform = physx::PxMat44(physx::rotFrom2Vectors(physx::PxVec3(0, 1, 0), normal), pos);

		NxPlaneShapeDesc	shapeDesc;
		NxActorDesc			actorDesc;

		physx::apex::NxFromPxVec3(shapeDesc.normal, normal);
		shapeDesc.d				= pos.dot(normal);
		shapeDesc.shapeFlags	= NX_SF_VISUALIZATION | NX_SF_CLOTH_TWOWAY;
		shapeDesc.materialIndex	= 0;

		if (useGroupsMask)
		{
			shapeDesc.groupsMask.bits0 = 1;
			shapeDesc.groupsMask.bits2 = ~0;
		}

		physx::apex::NxFromPxMat34(actorDesc.globalPose, mTransform);
		actorDesc.flags = 0;
		actorDesc.shapes.push_back(&shapeDesc);

		mPhysxActor = physxScene.createActor(actorDesc);
		PX_ASSERT(mPhysxActor);
	}

private:
	SampleRenderer::RendererGridShape* mRendererGridShape;
};

#elif NX_SDK_VERSION_MAJOR == 3

class SamplePlaneActor : public SampleShapeActor
{
public:
	SamplePlaneActor(SampleRenderer::Renderer* renderer,
	                 SampleFramework::SampleMaterialAsset& material,
	                 physx::PxScene& physxScene,
	                 const physx::PxVec3& pos,
	                 const physx::PxVec3& normal,
	                 physx::PxMaterial* pxmaterial,
	                 bool useGroupsMask,
	                 physx::apex::NxApexRenderDebug* rdebug = NULL)
		: SampleShapeActor(rdebug)
		, mRendererGridShape(NULL)
	{
		mRenderer = renderer;
		if (!pxmaterial)
			physxScene.getPhysics().getMaterials(&pxmaterial, 1);
		createActor(physxScene, pos, normal, pxmaterial, useGroupsMask);

		// default is X_UP
		mRendererGridShape = new SampleRenderer::RendererGridShape(*mRenderer, 10, 1.0f, false, SampleRenderer::RendererGridShape::UP_X);

		mRendererMeshContext.material         = material.getMaterial();
		mRendererMeshContext.materialInstance = material.getMaterialInstance();
		mRendererMeshContext.mesh             = mRendererGridShape->getMesh();
		mRendererMeshContext.transform        = &mTransform;

		if (rdebug)
		{
			mBlockId = rdebug->beginDrawGroup(mTransform);
			rdebug->addToCurrentState(physx::DebugRenderState::SolidShaded);
			static physx::PxU32 pcount /* = 0 */;
			rdebug->setCurrentColor(0xFFFFFF);
			rdebug->setCurrentTextScale(0.5f);
			rdebug->addToCurrentState(physx::DebugRenderState::CenterText);
			rdebug->addToCurrentState(physx::DebugRenderState::CameraFacing);
			rdebug->debugText(physx::PxVec3(0, 1.0f + 0.01f, 0), "Sample Plane:%d", pcount++);
			rdebug->endDrawGroup();
		}
	}

	virtual ~SamplePlaneActor()
	{
		if (mRendererGridShape)
		{
			delete mRendererGridShape;
			mRendererGridShape = NULL;
		}
	}

private:
	void createActor(physx::PxScene& physxScene,
	                 const physx::PxVec3& pos,
	                 const physx::PxVec3& normal,
	                 physx::PxMaterial* pxmaterial,
	                 bool useGroupsMask)
	{
		//PxMat33 m33(physx::rotFrom2Vectors(physx::PxVec3(0, 1, 0), normal));
		physx::PxMat33 m33(physx::rotFrom2Vectors(physx::PxVec3(1, 0, 0), normal));
		mTransform = physx::PxMat44(m33, pos);

		//PxTransform pose(pos, PxQuat(physx::rotFrom2Vectors(physx::PxVec3(1, 0, 0), normal)));
		physx::PxTransform pose	= physx::PxTransform(mTransform);
		pose.q.normalize();
		physx::PxRigidStatic* actor = physxScene.getPhysics().createRigidStatic(pose);
		PX_ASSERT(actor);

		physx::PxPlaneGeometry planeGeom;
		physx::PxShape* shape = actor->createShape(planeGeom, *pxmaterial);
		PX_ASSERT(shape);
		if (shape && useGroupsMask)
		{
			shape->setSimulationFilterData(physx::PxFilterData(1, 0, ~0u, 0));
			shape->setQueryFilterData(physx::PxFilterData(1, 0, ~0u, 0));
		}
		SCOPED_PHYSX3_LOCK_WRITE(&physxScene);
		physxScene.addActor(*actor);
		mPhysxActor = actor;
	}

private:
	SampleRenderer::RendererGridShape* mRendererGridShape;
};
#endif

#endif
