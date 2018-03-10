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

#include "PhysXIndicator.h"
#include "nvPhysXtoDrv.h"

physx::PhysXIndicator::PhysXIndicator(bool isGpu) 
: mPhysXDataPtr(0), mFileHandle(0), mIsGpu(isGpu)
{
    
}

physx::PhysXIndicator::~PhysXIndicator()
{
}

void physx::PhysXIndicator::setIsGpu(bool isGpu)
{
}

PX_INLINE void physx::PhysXIndicator::updateCounter(int delta)
{
}