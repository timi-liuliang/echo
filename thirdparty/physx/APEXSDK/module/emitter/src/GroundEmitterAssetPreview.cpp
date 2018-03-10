/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "NxApexRenderDebug.h"
#include "NxGroundEmitterPreview.h"
#include "GroundEmitterAssetPreview.h"
#include "ApexPreview.h"
#include "foundation/PxFoundation.h"
#include "PsShare.h"
#include "WriteCheck.h"
#include "ReadCheck.h"

namespace physx
{
namespace apex
{
namespace emitter
{

void GroundEmitterAssetPreview::drawEmitterPreview(void)
{
	NX_WRITE_ZONE();
#ifndef WITHOUT_DEBUG_VISUALIZE
	if (!mApexRenderDebug)
	{
		return;
	}
	physx::PxVec3 tmpUpDirection(0.0f, 1.0f, 0.0f);

	//asset preview init
	if (mGroupID == 0)
	{
		mGroupID = mApexRenderDebug->beginDrawGroup(physx::PxMat44().createIdentity());
		// Cylinder that describes the refresh radius, upDirection, and spawnHeight
		mApexRenderDebug->setCurrentColor(mApexRenderDebug->getDebugColor(physx::DebugColors::Green));
		mApexRenderDebug->debugCylinder(
		    physx::PxVec3(0.0f),
		    tmpUpDirection * (mAsset->getSpawnHeight() + mAsset->getRaycastHeight() + 0.01f),
		    mAsset->getRadius());

		// Ray that describes the raycast spawn height
		mApexRenderDebug->setCurrentColor(mApexRenderDebug->getDebugColor(physx::DebugColors::Yellow), mApexRenderDebug->getDebugColor(physx::DebugColors::Yellow));
		mApexRenderDebug->setCurrentArrowSize(mScale);
		mApexRenderDebug->debugRay(tmpUpDirection * mAsset->getRaycastHeight(), physx::PxVec3(0.0f));
		mApexRenderDebug->endDrawGroup();
	}

	//asset preview set pose
	physx::PxMat44 groupPose = mPose;
	mApexRenderDebug->setDrawGroupPose(mGroupID, groupPose);

	//asset preview set visibility
	mApexRenderDebug->setDrawGroupVisible(mGroupID, true);
#endif
}

void GroundEmitterAssetPreview::destroy(void)
{
	if (mApexRenderDebug)
	{
		mApexRenderDebug->reset();
		mApexRenderDebug->release();
	}
	mApexRenderDebug = NULL;
	ApexPreview::destroy();
	delete this;
}

GroundEmitterAssetPreview::~GroundEmitterAssetPreview(void)
{
}

void GroundEmitterAssetPreview::setPose(const physx::PxMat44& pose)
{
	NX_WRITE_ZONE();
	mPose = pose;
	drawEmitterPreview();
}

void GroundEmitterAssetPreview::setScale(physx::PxF32 scale)
{
	NX_WRITE_ZONE();
	mScale = scale;
	drawEmitterPreview();
}

const physx::PxMat44 GroundEmitterAssetPreview::getPose() const
{
	NX_READ_ZONE();
	return(mPose);
}

// from NxApexRenderDataProvider
void GroundEmitterAssetPreview::lockRenderResources(void)
{
	ApexRenderable::renderDataLock();
}

void GroundEmitterAssetPreview::unlockRenderResources(void)
{
	ApexRenderable::renderDataUnLock();
}

void GroundEmitterAssetPreview::updateRenderResources(bool /*rewriteBuffers*/, void* /*userRenderData*/)
{
	mApexRenderDebug->updateRenderResources();
}

// from NxApexRenderable.h
void GroundEmitterAssetPreview::dispatchRenderResources(NxUserRenderer& renderer)
{
	mApexRenderDebug->dispatchRenderResources(renderer);
}

physx::PxBounds3 GroundEmitterAssetPreview::getBounds(void) const
{
	return mApexRenderDebug->getBounds();
}

void GroundEmitterAssetPreview::release(void)
{
	if (mInRelease)
	{
		return;
	}
	mInRelease = true;
	const_cast<GroundEmitterAsset*>(mAsset)->releaseEmitterPreview(*this);
}

}
}
} // namespace physx::apex
