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


#ifndef PXS_BROADPHASE_MBP_H
#define PXS_BROADPHASE_MBP_H

#include "CmPhysXCommon.h"
#include "PxvBroadPhase.h"
#include "PxsBroadPhaseCommon.h"
#include "PsUserAllocated.h"
#include "PxsMBPTasks.h"

	class MBP;

namespace physx
{
	class PxSceneDesc;
	class PxsAABBManager;

	class PxsBroadPhaseMBP : public PxvBroadPhase, public Ps::UserAllocated
	{
		PX_NOCOPY(PxsBroadPhaseMBP)
		public:

												PxsBroadPhaseMBP(PxcScratchAllocator& allocator, Cm::EventProfiler& eventProfiler, const PxSceneDesc& desc, PxsAABBManager*);
		virtual									~PxsBroadPhaseMBP();

	// PxBroadPhaseBase
		virtual	bool							getCaps(PxBroadPhaseCaps& caps)														const;
		virtual	PxU32							getNbRegions()																		const;
		virtual	PxU32							getRegions(PxBroadPhaseRegionInfo* userBuffer, PxU32 bufferSize, PxU32 startIndex=0) const;
		virtual	PxU32							addRegion(const PxBroadPhaseRegion& region, bool populateRegion);
		virtual	bool							removeRegion(PxU32 handle);
		virtual	PxU32							getNumOutOfBoundsObjects()	const;
		virtual	const PxU32*					getOutOfBoundsObjects()		const;
	//~PxBroadPhaseBase

	// PxvBroadPhase
		virtual	PxBroadPhaseType::Enum			getType()																			const	{ return PxBroadPhaseType::eMBP;	}

		virtual	void							destroy();

		virtual	void							update(const PxU32 numCpuTasks, const PxU32 numSpus, const PxcBroadPhaseUpdateData& updateData, PxBaseTask* continuation);

		virtual	PxU32							getNumCreatedPairs()		const;
		virtual PxcBroadPhasePair*				getCreatedPairs();
		virtual PxU32							getNumDeletedPairs()		const;
		virtual PxcBroadPhasePair*				getDeletedPairs();

		virtual void							freeBuffers();

		virtual void							shiftOrigin(const PxVec3& shift);

#ifdef PX_CHECKED
		virtual bool							isValid(const PxcBroadPhaseUpdateData& updateData)	const;
#endif
	//~PxvBroadPhase

				MBPUpdateWorkTask				mMBPUpdateWorkTask;
				MBPPostUpdateWorkTask			mMBPPostUpdateWorkTask;

				MBP*							mMBP;		// PT: TODO: aggregate

				PxU32*							mMapping;
				PxU32							mCapacity;
				Ps::Array<PxcBroadPhasePair>	mCreated;
				Ps::Array<PxcBroadPhasePair>	mDeleted;

				const PxcBpHandle*				mGroups;	// ### why are those 'handles'?

				void							setUpdateData(const PxcBroadPhaseUpdateData& updateData);
				void							updatePPU(PxBaseTask* continuation);
				void							postUpdatePPU(PxBaseTask* continuation);
#ifdef PX_PS3
				bool							canRunOnSpu() const														{ return false;	}
				void							updateSPU(const PxU32 , PxBaseTask* )			{}
				void							postUpdateSPU(const PxU32 , PxBaseTask* )		{}
#endif

	};

}

#endif //PXS_BROADPHASE_MBP_H
