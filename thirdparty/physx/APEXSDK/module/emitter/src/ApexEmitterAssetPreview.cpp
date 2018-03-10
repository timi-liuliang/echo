/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "NxApexEmitterPreview.h"
#include "ApexEmitterAssetPreview.h"

#define ASSET_INFO_XPOS					(-0.9f)	// left position of the asset info
#define ASSET_INFO_YPOS					( 0.9f)	// top position of the asset info
#define DEBUG_TEXT_HEIGHT				(0.4f)	// in screen space

namespace physx
{
namespace apex
{
namespace emitter
{

bool ApexEmitterAssetPreview::isValid() const
{
	return mApexRenderDebug != NULL;
}

void ApexEmitterAssetPreview::drawEmitterPreview(void)
{
	NX_WRITE_ZONE();
#ifndef WITHOUT_DEBUG_VISUALIZE
	if (!mApexRenderDebug)
	{
		return;
	}

	//asset preview init
	if (mGroupID == 0)
	{
		mGroupID = mApexRenderDebug->beginDrawGroup(physx::PxMat44().createIdentity());
		mAsset->mGeom->drawPreview(mScale, mApexRenderDebug);
		mApexRenderDebug->endDrawGroup();
	}

	toggleDrawPreview();
	setDrawGroupsPose();
#endif
}

void	ApexEmitterAssetPreview::drawPreviewAssetInfo(void)
{
	if (!mApexRenderDebug)
	{
		return;
	}
		
	char buf[128];
	buf[sizeof(buf) - 1] = 0;

	ApexSimpleString myString;
	ApexSimpleString floatStr;
	physx::PxU32 lineNum = 0;

	mApexRenderDebug->pushRenderState();
	//	mApexRenderDebug->addToCurrentState(physx::DebugRenderState::ScreenSpace);
	mApexRenderDebug->addToCurrentState(physx::DebugRenderState::NoZbuffer);
	mApexRenderDebug->setCurrentTextScale(2.0f);
	mApexRenderDebug->setCurrentColor(mApexRenderDebug->getDebugColor(physx::DebugColors::Yellow));

	// asset name
	APEX_SPRINTF_S(buf, sizeof(buf) - 1, "%s %s", mAsset->getObjTypeName(), mAsset->getName());
	drawInfoLine(lineNum++, buf);
	lineNum++;
			
	mApexRenderDebug->popRenderState();
}


void		ApexEmitterAssetPreview::drawInfoLine(physx::PxU32 lineNum, const char* str)
{
#ifdef WITHOUT_DEBUG_VISUALIZE
	PX_UNUSED(lineNum);
	PX_UNUSED(str);
#else
	mApexRenderDebug->setCurrentColor(mApexRenderDebug->getDebugColor(physx::DebugColors::Green));
	physx::PxVec3 textLocation = mPose.getPosition();
	PxMat44 cameraMatrix = mPreviewScene->getCameraMatrix();
	textLocation += cameraMatrix.column1.getXYZ() * (ASSET_INFO_YPOS - (lineNum * DEBUG_TEXT_HEIGHT));
	cameraMatrix.setPosition(textLocation);
	mApexRenderDebug->debugOrientedText(cameraMatrix, str);	
#endif
}


void ApexEmitterAssetPreview::destroy(void)
{
	ApexPreview::destroy();
	delete this;
}

ApexEmitterAssetPreview::~ApexEmitterAssetPreview(void)
{
	if (mApexRenderDebug)
	{
		mApexRenderDebug->reset();
		mApexRenderDebug->reset(mGroupID);
		mApexRenderDebug->release();
		mApexRenderDebug = NULL;
	}
}

void ApexEmitterAssetPreview::setScale(physx::PxF32 scale)
{
	NX_WRITE_ZONE();
	mScale = scale;
	drawEmitterPreview();
}

void ApexEmitterAssetPreview::setPose(const physx::PxMat44& pose)
{
	NX_WRITE_ZONE();
	mPose = pose;
	setDrawGroupsPose();
}

const physx::PxMat44 ApexEmitterAssetPreview::getPose() const
{
	NX_READ_ZONE();
	return mPose;
}

void ApexEmitterAssetPreview::setDrawGroupsPose()
{
	mApexRenderDebug->setDrawGroupPose(mGroupID, mPose);
}

void ApexEmitterAssetPreview::toggleDrawPreview()
{
	//asset preview set visibility
	mApexRenderDebug->setDrawGroupVisible(mGroupID, true);
}


// from NxApexRenderDataProvider
void ApexEmitterAssetPreview::lockRenderResources(void)
{
	ApexRenderable::renderDataLock();
}

void ApexEmitterAssetPreview::unlockRenderResources(void)
{
	ApexRenderable::renderDataUnLock();
}

void ApexEmitterAssetPreview::updateRenderResources(bool /*rewriteBuffers*/, void* /*userRenderData*/)
{
	if (mApexRenderDebug)
	{
		mApexRenderDebug->updateRenderResources();
	}
}

// from NxApexRenderable.h
void ApexEmitterAssetPreview::dispatchRenderResources(NxUserRenderer& renderer)
{
	if (mApexRenderDebug)
	{
		drawPreviewAssetInfo();
		mApexRenderDebug->dispatchRenderResources(renderer);
	}
}

physx::PxBounds3 ApexEmitterAssetPreview::getBounds(void) const
{
	NX_READ_ZONE();
	return(mApexRenderDebug->getBounds());
}

void ApexEmitterAssetPreview::release(void)
{
	if (mInRelease)
	{
		return;
	}
	mInRelease = true;
	const_cast<ApexEmitterAsset*>(mAsset)->releaseEmitterPreview(*this);
}

	ApexEmitterAssetPreview::ApexEmitterAssetPreview(const NxApexEmitterPreviewDesc& pdesc, const ApexEmitterAsset& asset, NxApexAssetPreviewScene* previewScene, NxApexSDK* myApexSDK) :
		mApexSDK(myApexSDK),
		mApexRenderDebug(0),
		mScale(pdesc.mScale),
		mAsset(&asset),
		mPreviewScene(previewScene),
		mGroupID(0)
	{
#ifndef WITHOUT_DEBUG_VISUALIZE
		mApexRenderDebug = mApexSDK->createApexRenderDebug();
		setPose(pdesc.mPose);
		drawPreviewAssetInfo();
		drawEmitterPreview();
#endif
	};

}
}
} // namespace physx::apex
