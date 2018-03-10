/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#ifndef __PHYSXINDICATOR_H__
#define __PHYSXINDICATOR_H__

#include <foundation/PxPreprocessor.h>

namespace physx
{
	struct NvPhysXToDrv_Data_V1_;

	class PhysXIndicator
	{
	public:
		PhysXIndicator(bool isGpu = false);
		~PhysXIndicator();

		void setIsGpu(bool isGpu);

	private:
		void updateCounter(int delta);

		NvPhysXToDrv_Data_V1_* mPhysXDataPtr;
		void* mFileHandle;
		bool mIsGpu;
	};
}

#endif	// __PHYSXINDICATOR_H__
