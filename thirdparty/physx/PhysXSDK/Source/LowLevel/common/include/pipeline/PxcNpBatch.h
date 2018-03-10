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



#ifndef PXC_NP_BATCH_H
#define PXC_NP_BATCH_H

#include "PxvConfig.h"
#include "PxGeometry.h"
#include "PxcNpWorkUnit.h"

namespace physx
{

struct PxcNpWorkUnit;
class PxsThreadContext;
class PxsContactManager;

namespace Cm
{
	class FlushPool;
}

class PxLightCpuTask;


void PxcDiscreteNarrowPhase(PxcNpThreadContext& context, PxcNpWorkUnit& n);
void PxcDiscreteNarrowPhasePCM(PxcNpThreadContext& context, PxcNpWorkUnit& n);
void PxcSkipNarrowPhase(PxcNpWorkUnit& n);

struct PxcNpBatchEntry
{
	PxcNpWorkUnit*		workUnit;
	PxU32				cmIndex;
	PxsContactManager*	cm;

	PxcNpBatchEntry()
	{}
	PxcNpBatchEntry(PxcNpWorkUnit* workUnit_, PxU32 cmIndex_, PxsContactManager* cm_)
		: workUnit(workUnit_), cmIndex(cmIndex_), cm(cm_)
	{}
};

void PxcRunNpBatch(const PxU32 numSpusPrim, const PxU32 numSpusCnvx, const PxU32 numSpusHF, const PxU32 numSpusMesh, const PxU32 numSpusCnvxMesh,
				   PxsThreadContext* context, PxcNpMemBlockPool& memBlockPool,
				   const PxU32 numContraintBlocks, const PxU32 numFrictionBlocks,
				   PxcNpBatchEntry* entriesPrimVsPrim, PxU32 numEntriesPrimVsPrim,
				   PxcNpBatchEntry* entriesPrimOrCnvxVsCnvx, PxU32 numEntriesPrimOrCnvxVsCnvx,
				   PxcNpBatchEntry* entriesPrimOrCnvxVsHF, PxU32 numEntriesPrimOrCnvxVsHF,
				   PxcNpBatchEntry* entriesPrimVsTrimesh, PxU32 numEntriesPrimVsTrimesh,
				   PxcNpBatchEntry* entriesCvxBoxVsTrimesh, PxU32 numEntriesCvxBoxVsTrimesh,
				   PxcNpBatchEntry* entriesOther, PxU32 numEntriesOther,
				   PxU32* bitmapBase, PxU32 bitmapWordCount,
				   PxU32& touchLost, PxU32& touchFound,
				   physx::PxLightCpuTask* continuation, Cm::FlushPool& taskPool);


//void PxcRunNpPCMBatch(const PxU32 numSpusCnvx, const PxU32 numSpusHF, const PxU32 numSpusMesh, const PxU32 numSpusCnvxMesh,
//				   PxsThreadContext* context, PxcNpMemBlockPool& memBlockPool,
//				   const PxU32 numContraintBlocks, const PxU32 numFrictionBlocks,
//				   PxcNpBatchEntry* entriesPrimVsPrim, PxU32 numEntriesPrimVsPrim,
//				   PxcNpBatchEntry* entriesPrimOrCnvxVsHF, PxU32 numEntriesPrimOrCnvxVsHF,
//				   PxcNpBatchEntry* entriesPrimVsTrimesh, PxU32 numEntriesPrimVsTrimesh,
//				   PxcNpBatchEntry* entriesCvxBoxVsTrimesh, PxU32 numEntriesCvxBoxVsTrimesh,
//				   PxcNpBatchEntry* entriesOther, PxU32 numEntriesOther,
//				   PxU32* bitmapBase, PxU32 bitmapWordCount,
//				   PxU32& touchLost, PxU32& touchFound,
//				   physx::PxLightCpuTask* continuation, Cm::FlushPool& taskPool);

void PxcRunNpPCMBatch(	const PxU32 numSpusCnvx, const PxU32 numSpusHF, const PxU32 numSpusMesh,
								PxsThreadContext* context, PxcNpMemBlockPool& memBlockPool,
								const PxU32 numContactBlocks, const PxU32 numNpCacheBlocks,
								PxcNpBatchEntry* entriesPrimVsPrim, PxU32 numEntriesPrimVsPrim,
								PxcNpBatchEntry* entriesPrimOrCnvxVsHF, PxU32 numEntriesPrimOrCnvxVsHF,
								PxcNpBatchEntry* entriesPrimOrCnvxVsTrimesh, PxU32 numEntriesPrimOrCnvxVsTrimesh,
								PxcNpBatchEntry* entriesOther, PxU32 numEntriesOther,
								PxU32* changeBitmapBase, PxU32 changeBitmapWordCount,
								PxU32& touchLost, PxU32& touchFound,
								physx::PxLightCpuTask* continuation, Cm::FlushPool& taskPool);

}

#endif
