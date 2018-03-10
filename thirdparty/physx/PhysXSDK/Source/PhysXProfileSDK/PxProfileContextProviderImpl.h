/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef PX_PHYSX_PROFILE_CONTEXT_PROVIDER_IMPL_H
#define PX_PHYSX_PROFILE_CONTEXT_PROVIDER_IMPL_H
#include "PsThread.h"
#include "PsCpu.h"

namespace physx {
	
	struct PxDefaultContextProvider
	{
		PxProfileEventExecutionContext getExecutionContext() 
		{ 
			shdfnd::Thread::Id theId( shdfnd::Thread::getId() );
			return PxProfileEventExecutionContext( static_cast<PxU32>( theId ), static_cast<PxU8>( shdfnd::ThreadPriority::eNORMAL ), 0 );
		}

		PxU32 getThreadId() 
		{ 
			return static_cast<PxU32>( shdfnd::Thread::getId() ); 
		}
	};
}

#endif
