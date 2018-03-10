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


#ifndef PX_PHYSICS_EXTENSIONS_NP_TASK_QUEUE_HELPER_H
#define PX_PHYSICS_EXTENSIONS_NP_TASK_QUEUE_HELPER_H

#include "CmPhysXCommon.h"
#include "PxTask.h"
#include "ExtSharedQueueEntryPool.h"


namespace physx
{	
	class PxBaseTask;
}

namespace physx
{

#define EXT_TASK_QUEUE_ENTRY_POOL_SIZE 128

namespace Ext
{
	class TaskQueueHelper
	{
	public:
		static PxBaseTask* fetchTask(Ps::SList& taskQueue, Ext::SharedQueueEntryPool<>& entryPool)
		{
			SharedQueueEntry* entry = static_cast<SharedQueueEntry*>(taskQueue.pop());
			if (entry)
			{
				PxBaseTask* task = reinterpret_cast<PxBaseTask*>(entry->mObjectRef);
				entryPool.putEntry(*entry);
				return task;
			}
			else
				return NULL;
		}
	};

} // namespace Ext

}

#endif
