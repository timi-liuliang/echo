/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEX_EMITTER_ASSET_PREVIEW_H__
#define __APEX_EMITTER_ASSET_PREVIEW_H__

#include "ApexPreview.h"

#include "NiApexSDK.h"
#include "NxApexEmitterPreview.h"
#include "NxApexRenderDebug.h"
#include "ApexEmitterAsset.h"
#include "ApexRWLockable.h"

namespace physx
{
namespace apex
{
namespace emitter
{

class ApexEmitterAssetPreview : public NxApexEmitterPreview, public ApexResource, public ApexPreview, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ApexEmitterAssetPreview(const NxApexEmitterPreviewDesc& pdesc, const ApexEmitterAsset& asset, NxApexAssetPreviewScene* previewScene, NxApexSDK* myApexSDK);

	bool					isValid() const;

	void					drawEmitterPreview(void);
	void					drawPreviewAssetInfo(void);
	void					drawInfoLine(physx::PxU32 lineNum, const char* str);
	void					destroy();

	void					setPose(const physx::PxMat44& pose);	// Sets the preview instance's pose.  This may include scaling.
	const physx::PxMat44	getPose() const;

	// from NxApexRenderDataProvider
	void					lockRenderResources(void);
	void					unlockRenderResources(void);
	void					updateRenderResources(bool rewriteBuffers = false, void* userRenderData = 0);

	// from NxApexRenderable.h
	void					dispatchRenderResources(NxUserRenderer& renderer);
	physx::PxBounds3		getBounds(void) const;

	// from NxApexInterface.h
	void					release(void);

private:
	~ApexEmitterAssetPreview();

	void					toggleDrawPreview();
	void					setDrawGroupsPose();


	NxAuthObjTypeID					mModuleID;					// the module ID of Emitter.
	NxApexSDK*						mApexSDK;					// pointer to the APEX SDK
	NxApexRenderDebug*				mApexRenderDebug;			// Pointer to the RenderLines class to draw the
	physx::PxMat44					mPose;						// the pose for the preview rendering
	physx::PxF32					mScale;
	const ApexEmitterAsset*         mAsset;
	physx::PxI32                    mGroupID;
	NxApexAssetPreviewScene*		mPreviewScene;

	void							setScale(physx::PxF32 scale);
};

}
}
} // end namespace physx::apex

#endif // __APEX_EMITTER_ASSET_PREVIEW_H__
