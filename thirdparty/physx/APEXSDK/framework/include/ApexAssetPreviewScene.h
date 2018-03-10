/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_ASSET_PREVIEW_SCENE_H
#define APEX_ASSET_PREVIEW_SCENE_H

#include "NxApex.h"
#include "ApexInterface.h"
#include "PsUserAllocated.h"
#include "ApexSDK.h"
#include "NxApexAssetPreviewScene.h"
#include "NiModule.h"
#include "ApexContext.h"
#include "PsMutex.h"

#if NX_SDK_VERSION_MAJOR == 2
#include "NxScene.h"
#include "NxDebugRenderable.h"
#else
#include "PxScene.h"
#include "PxRenderBuffer.h"
#endif

#include "ApexSceneUserNotify.h"

#include "PsSync.h"
#include "PxTask.h"
#include "PxTaskManager.h"

#include "ApexGroupsFiltering.h"
#include "ApexRWLockable.h"

namespace physx
{
namespace apex
{

class ApexAssetPreviewScene : public NxApexAssetPreviewScene, public ApexRWLockable, public physx::UserAllocated
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ApexAssetPreviewScene(ApexSDK* sdk);
	virtual ~ApexAssetPreviewScene() {}

	//Sets the view matrix. Should be called whenever the view matrix needs to be updated.
	virtual void					setCameraMatrix(const physx::PxMat44& viewTransform);
		
	//Returns the view matrix set by the user for the given viewID.
	virtual physx::PxMat44			getCameraMatrix() const;

	virtual void					setShowFullInfo(bool showFullInfo);

	virtual bool					getShowFullInfo() const;

	virtual void					release();

	void							destroy();

private:
	ApexSDK*						mApexSDK;

	physx::PxMat44					mCameraMatrix;				// the pose for the preview rendering
	bool							mShowFullInfo;
};

}
} // end namespace physx::apex

#endif // APEX_ASSET_PREVIEW_SCENE_H
