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

#ifndef PXS_SAP_TASKS_H
#define PXS_SAP_TASKS_H

#include "CmEventProfiler.h"
#include "CmTask.h"

#ifdef PX_PS3
#define SPU_BP_SAP 1
#define FORCE_SINGLE_SPU_SAP 0
#define SPU_PROFILE 0
#else
#define SPU_BP_SAP 0
#endif //PX_PS3

namespace physx
{
	class PxsBroadPhaseContextSap;

	class SapUpdateWorkTask: public Cm::Task
	{
	public:

		SapUpdateWorkTask()
		{
		}

		void setBroadPhase(PxsBroadPhaseContextSap* sap) 
		{
			mSAP = sap;
		}

		void set(const PxU32 numCpuTasks, const PxU32 numSpus) 
		{
			mNumCpuTasks = numCpuTasks; 
			mNumSpus = numSpus;
		}

		virtual void runInternal();

		virtual const char* getName() const { return "PxsSap.sapUpdateWork"; }

	private:

		PxsBroadPhaseContextSap* mSAP;
		PxU32 mNumCpuTasks;
		PxU32 mNumSpus;
	};

	class SapPostUpdateWorkTask: public Cm::Task
	{
	public:

		SapPostUpdateWorkTask()
		{
		
		}
		void setBroadPhase(PxsBroadPhaseContextSap* sap) 
		{
			mSAP = sap;
		}

		void set(const PxU32 numCpuTasks, const PxU32 numSpus) 
		{
			mNumCpuTasks=numCpuTasks; 
			mNumSpus = numSpus; 
		}

		virtual void runInternal();

		virtual const char* getName() const { return "PxsSap.sapPostUpdateWork"; }

	private:

		PxsBroadPhaseContextSap* mSAP;
		PxU32 mNumCpuTasks;
		PxU32 mNumSpus;
	};

} //namespace physx

#endif // PXS_SAP_TASKS_H
