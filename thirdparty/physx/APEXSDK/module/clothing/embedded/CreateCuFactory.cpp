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

#include "CuFactory.h"
#include "CreateCuFactory.h"

#if defined(PX_WINDOWS)

physx::cloth::CuFactory* PxCreateCuFactory(physx::PxCudaContextManager* contextManager)
{
	return new physx::cloth::CuFactory(contextManager);
}

#endif //PX_WINDOWS