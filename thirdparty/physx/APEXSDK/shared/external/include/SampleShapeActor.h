/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef __SAMPLE_SHAPE_ACTOR_H__
#define __SAMPLE_SHAPE_ACTOR_H__

#include "SampleActor.h"
#include "NxApexReadWriteLock.h"
#include "NxApexDefs.h"
#if NX_SDK_VERSION_MAJOR == 2
#include "NxActor.h"
#include "NxScene.h"
#include "NxShape.h"
#include "NxFromPx.h"
#elif NX_SDK_VERSION_MAJOR == 3
#include "PxActor.h"
#include "PxRigidDynamic.h"
#endif

#include "NxApexRenderDebug.h"

#include <Renderer.h>
#include <RendererMeshContext.h>

class SampleShapeActor : public SampleFramework::SampleActor
{
public:
	SampleShapeActor(physx::apex::NxApexRenderDebug* rdebug)
		: mBlockId(-1)
		, mApexRenderDebug(rdebug)
		, mRenderer(NULL)
		, mPhysxActor(NULL)
	{
	}

	virtual ~SampleShapeActor(void)
	{
		if (mApexRenderDebug != NULL)
		{
			mApexRenderDebug->reset(mBlockId);
		}

		if (mPhysxActor)
		{
#if NX_SDK_VERSION_MAJOR == 2
			mPhysxActor->getScene().releaseActor(*mPhysxActor);
#elif NX_SDK_VERSION_MAJOR == 3
			SCOPED_PHYSX3_LOCK_WRITE(mPhysxActor->getScene());
			mPhysxActor->release();
#endif
		}
	}

	physx::PxTransform getPose() const
	{
		return physx::PxTransform(mTransform);
	}

	void setPose(const physx::PxTransform& pose)
	{
		mTransform = physx::PxMat44(pose);
		if (mPhysxActor)
		{
#if NX_SDK_VERSION_MAJOR == 2
			NxMat34 mat;
			physx::apex::NxFromPxMat44(mat, mTransform);
			mPhysxActor->setGlobalPose(mat);
#elif NX_SDK_VERSION_MAJOR == 3
			SCOPED_PHYSX3_LOCK_WRITE(mPhysxActor->getScene());
			if (physx::PxRigidDynamic* rd = mPhysxActor->isRigidDynamic())
			{
				rd->setGlobalPose(this->convertToPhysicalCoordinates(mTransform));
			}
#endif
		}
		if (mApexRenderDebug != NULL)
		{
			mApexRenderDebug->setDrawGroupPose(mBlockId, mTransform);
		}
	}

#if NX_SDK_VERSION_MAJOR == 2
	void setGroupsMask(const NxGroupsMask& mask)
	{
		const physx::PxU32 numShapes = mPhysxActor->getNbShapes();
		NxShape* const* shapes = mPhysxActor->getShapes();
		for (physx::PxU32 i = 0; i < numShapes; ++i)
		{
			shapes[i]->setGroupsMask(mask);
		}
	}

	virtual void tick(float /*dtime*/, bool /*rewriteBuffers*/ = false)
	{
		if (mPhysxActor && !mPhysxActor->isSleeping())
		{
			physx::apex::PxFromNxMat34(mTransform, mPhysxActor->getGlobalPose());
			if (mApexRenderDebug != NULL)
			{
				mApexRenderDebug->setDrawGroupPose(mBlockId, mTransform);
			}
		}
	}

	NxActor* getPhysXActor()
	{
		return mPhysxActor;
	}

#elif NX_SDK_VERSION_MAJOR == 3

	virtual void tick(float dtime, bool rewriteBuffers = false)
	{
		if (mPhysxActor)
		{
			physx::PxRigidDynamic* rd = mPhysxActor->isRigidDynamic();
			SCOPED_PHYSX3_LOCK_READ(mPhysxActor->getScene());
			if (rd && !rd->isSleeping())
			{
				mTransform = this->convertToGraphicalCoordinates(rd->getGlobalPose());
				if (mApexRenderDebug != NULL)
				{
					mApexRenderDebug->setDrawGroupPose(mBlockId, mTransform);
				}
			}
		}
	}

	physx::PxActor* getPhysXActor()
	{
		return mPhysxActor;
	}

#endif

	virtual void render(bool /*rewriteBuffers*/ = false)
	{
		if (mRenderer)
		{
			mRenderer->queueMeshForRender(mRendererMeshContext);
		}
	}

protected:
	physx::PxI32						mBlockId;
	physx::apex::NxApexRenderDebug*		mApexRenderDebug;
	SampleRenderer::Renderer*			mRenderer;
	SampleRenderer::RendererMeshContext	mRendererMeshContext;
	physx::PxMat44						mTransform; 
#if NX_SDK_VERSION_MAJOR == 2
	NxActor*							mPhysxActor;
#elif NX_SDK_VERSION_MAJOR == 3
	physx::PxActor*						mPhysxActor;
#endif

private:
	virtual physx::PxMat44		convertToGraphicalCoordinates(const physx::PxTransform & physicsPose) const
	{
		return physx::PxMat44(physicsPose);
	}

	virtual physx::PxTransform	convertToPhysicalCoordinates(const physx::PxMat44 & graphicsPose) const
	{
		return physx::PxTransform(graphicsPose);
	}
};

#endif
