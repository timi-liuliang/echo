/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "NxApex.h"
#include "NxApexDefs.h"
#include "ApexAssetPreviewScene.h"
#include "ApexSceneTasks.h"
#include "ApexSDK.h"
#include "ApexActor.h"
#include "FrameworkPerfScope.h"
#include "ApexRenderDebug.h"
#include "NiModule.h"
#include "PVDBinding.h"
#include "PsTime.h"
#include "NxApexReadWriteLock.h"
#include "PsShare.h"
#include "PsSync.h"
#include "PxTask.h"
#include "PxTaskManager.h"
#include "PxGpuDispatcher.h"
#include "PxCudaContextManager.h"


namespace physx
{
namespace apex
{

	ApexAssetPreviewScene::ApexAssetPreviewScene(ApexSDK* sdk) : mApexSDK(sdk)
	, mShowFullInfo(false)
	{
		mCameraMatrix = mCameraMatrix.createIdentity();
	}

	void ApexAssetPreviewScene::setCameraMatrix(const physx::PxMat44& cameraMatrix)
	{
		mCameraMatrix = cameraMatrix;
	}

	physx::PxMat44 ApexAssetPreviewScene::getCameraMatrix() const
	{
		return mCameraMatrix;
	}

	void ApexAssetPreviewScene::setShowFullInfo(bool showFullInfo)
	{
		mShowFullInfo = showFullInfo;
	}

	bool ApexAssetPreviewScene::getShowFullInfo() const
	{
		return mShowFullInfo;
	}

	void ApexAssetPreviewScene::release()
	{
		mApexSDK->releaseAssetPreviewScene(this);
	}

	void ApexAssetPreviewScene::destroy()
	{
		PX_DELETE(this);
	}
}
} // end namespace physx::apex
