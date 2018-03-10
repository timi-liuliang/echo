/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEX_CUDA_KERNEL_MANAGER__
#define __APEX_CUDA_KERNEL_MANAGER__

#include "NxApexDefs.h"
#include "NxApexCudaProfileManager.h"

#include "PxMemoryBuffer.h"
#include "ApexString.h"
#include "NiApexScene.h"

namespace physx
{
namespace apex
{

class ApexCudaObj;
class ApexCudaFunc;
class ApexCudaProfileManager;

class ApexCudaProfileSession
{
	struct ProfileData
	{
		PxU32 id;
		void* start;
		void* stop;
	};
public:
	ApexCudaProfileSession();
	~ApexCudaProfileSession();

	PX_INLINE void	init(ApexCudaProfileManager* manager)
	{
		mManager = manager;
	}
	void		nextFrame();
	void		start();
	bool		stopAndSave();
	PxU32		getProfileId(const char* name, const char* moduleName);

	void		onFuncStart(PxU32 id, void* stream);
	void		onFuncFinish(PxU32 id, void* stream);

protected:
	PxF32		flushProfileInfo(ProfileData& pd);

	ApexCudaProfileManager* mManager;
	void*			mTimer;
	PxMemoryBuffer	mMemBuf;
	physx::Mutex	mLock;
	Array <ProfileData> mProfileDataList;
	physx::PxF32	mFrameStart;
	physx::PxF32	mFrameFinish;
};

/** 
 */
class ApexCudaProfileManager : public NxApexCudaProfileManager
{
public:
	struct KernelInfo
	{
		ApexSimpleString functionName;
		ApexSimpleString moduleName;
		PxU32 id;
		
		KernelInfo(const char* functionName, const char* moduleName, PxU32 id = 0) 
			: functionName(functionName), moduleName(moduleName), id(id) {}

		bool operator!= (const KernelInfo& ki)
		{
			return		(this->functionName != "*" && this->functionName != ki.functionName) 
					|| 	(this->moduleName != ki.moduleName);
		}
	};

	ApexCudaProfileManager();

	virtual ~ApexCudaProfileManager();

	PX_INLINE void setNiApexScene(NiApexScene* scene)
	{
		mApexScene = scene;
	}
	void nextFrame();

	// interface for NxApexCudaProfileManager
	PX_INLINE void setPath(const char* path)
	{
		mPath = ApexSimpleString(path);
		enable(false);
	}
	void setKernel(const char* functionName, const char* moduleName);
	PX_INLINE void setTimeFormat(TimeFormat tf)
	{
		mTimeFormat = tf;
	}
	void enable(bool state);
	PX_INLINE bool isEnabled() const
	{
		return mState;
	}
		
private:
	bool			mState;
	PxU32			mSessionCount;
	TimeFormat		mTimeFormat;
	PxU32			mReservedId;
	ApexSimpleString			mPath;
	Array <KernelInfo>			mKernels;
	ApexCudaProfileSession		mSession;
	NiApexScene*	mApexScene;
	friend class ApexCudaProfileSession;
};

}
} // namespace physx::apex

#endif // __APEX_CUDA_KERNEL_MANAGER__
