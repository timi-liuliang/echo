/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __GROUND_EMITTER_ASSET_PREVIEW_H__
#define __GROUND_EMITTER_ASSET_PREVIEW_H__

#include "ApexPreview.h"

#include "NiApexSDK.h"
#include "NxGroundEmitterPreview.h"
#include "NxApexRenderDebug.h"
#include "GroundEmitterAsset.h"
#include "ApexRWLockable.h"

namespace physx
{
namespace apex
{
namespace emitter
{

class GroundEmitterAssetPreview : public NxGroundEmitterPreview, public ApexResource, public ApexPreview, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	GroundEmitterAssetPreview(const NxGroundEmitterPreviewDesc& pdesc, const GroundEmitterAsset& asset, NxApexSDK* myApexSDK, NxApexAssetPreviewScene* previewScene) :
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
		drawEmitterPreview();
#endif
	};

	bool                isValid() const
	{
		return mApexRenderDebug != NULL;
	}
	void				drawEmitterPreview(void);
	void				destroy();

	void				setPose(const physx::PxMat44& pose);	// Sets the preview instance's pose.  This may include scaling.
	const physx::PxMat44	getPose() const;

	// from NxApexRenderDataProvider
	void lockRenderResources(void);
	void unlockRenderResources(void);
	void updateRenderResources(bool rewriteBuffers = false, void* userRenderData = 0);

	// from NxApexRenderable.h
	void dispatchRenderResources(NxUserRenderer& renderer);
	physx::PxBounds3 getBounds(void) const;

	// from NxApexInterface.h
	void	release(void);

private:
	~GroundEmitterAssetPreview();

	NxAuthObjTypeID					mModuleID;					// the module ID of Emitter.
	NxUserRenderResourceManager*	mRrm;						// pointer to the users render resource manager
	NxApexSDK*						mApexSDK;					// pointer to the APEX SDK
	NxApexRenderDebug*				mApexRenderDebug;			// Pointer to the RenderLines class to draw the
	physx::PxMat34Legacy			mPose;						// the pose for the preview rendering
	physx::PxF32					mScale;
	const GroundEmitterAsset*       mAsset;
	physx::PxI32                    mGroupID;
	NxApexAssetPreviewScene*		mPreviewScene;

	void							setScale(physx::PxF32 scale);
};

}
}
} // end namespace physx::apex

#endif // __APEX_EMITTER_ASSET_PREVIEW_H__
