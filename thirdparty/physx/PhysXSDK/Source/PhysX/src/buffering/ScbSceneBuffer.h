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


#ifndef PX_PHYSICS_SCB_SCENE_BUFFER
#define PX_PHYSICS_SCB_SCENE_BUFFER

#define USE_DEFAULT_SPU_PARAM 0xffffffff

#include "CmPhysXCommon.h"

#include "ScScene.h"

#ifdef PX_PS3
#include "ps3/PxPS3Config.h"
#endif

#define PxClientBehaviorFlag_eNOT_BUFFERED	PxClientBehaviorFlags(0xff)

namespace physx
{
namespace Scb
{

struct SceneBuffer
{
public:
	static const PxU32 sMaxNbDominanceGroups = 32;

	PX_INLINE SceneBuffer();

	PX_INLINE void clearDominanceBuffer();
	PX_INLINE void setDominancePair(PxU32 group1, PxU32 group2, const PxDominanceGroupPair& dominance);
	PX_INLINE bool getDominancePair(PxU32 group1, PxU32 group2, PxDominanceGroupPair& dominance) const;
	PX_INLINE void syncDominancePairs(Sc::Scene& scene);

	PX_INLINE void clearVisualizationParams();

#ifdef PX_PS3
	PX_INLINE void clearSceneParamBuffer();
	PX_INLINE void setSceneParamInt(PxPS3ConfigParam::Enum param, PxU32 value);
	PX_INLINE void syncSceneParamInts(Sc::Scene& scene);
#endif

	PxReal								visualizationParam[PxVisualizationParameter::eNUM_VALUES];
	PxU8								visualizationParamChanged[PxVisualizationParameter::eNUM_VALUES];
	PxBounds3							visualizationCullingBox;
	PxU8								visualizationCullingBoxChanged;
	PxU32								dominancePairFlag[sMaxNbDominanceGroups - 1];
	PxU32								dominancePairValues[sMaxNbDominanceGroups];
	PxVec3								gravity;
	PxReal								bounceThresholdVelocity;
	PxSceneFlags						flags;
	PxU32								solverBatchSize;
	PxU32								numClientsCreated;
	Ps::Array<PxClientBehaviorFlags>	clientBehaviorFlags;	//a value is buffered if it is not -1.
#ifdef PX_PS3
	PxU32								spuParams[PxPS3ConfigParam::eCOUNT];
#endif
};


PX_INLINE SceneBuffer::SceneBuffer() : clientBehaviorFlags(PX_DEBUG_EXP("clientBehaviorFlags"))
{
	clearDominanceBuffer();
	clearVisualizationParams();
#ifdef PX_PS3
	clearSceneParamBuffer();
#endif
	numClientsCreated = 0;
	clientBehaviorFlags.pushBack(PxClientBehaviorFlag_eNOT_BUFFERED);	//need member for default client, PxClientBehaviorFlag_eNOT_BUFFERED means its not storing anything.
}


PX_INLINE void SceneBuffer::clearDominanceBuffer()
{
	PxMemSet(&dominancePairFlag, 0, (sMaxNbDominanceGroups - 1) * sizeof(PxU32));
}


PX_INLINE void SceneBuffer::clearVisualizationParams()
{
	PxMemZero(visualizationParamChanged, PxVisualizationParameter::eNUM_VALUES * sizeof(PxU8));
}


PX_INLINE void SceneBuffer::setDominancePair(PxU32 group1, PxU32 group2, const PxDominanceGroupPair& dominance)
{
	PX_ASSERT(group1 != group2);
	PX_ASSERT(group1 < sMaxNbDominanceGroups);
	PX_ASSERT(group2 < sMaxNbDominanceGroups);

	if (group1 < group2)
		dominancePairFlag[group1] = dominancePairFlag[group1] | (1 << group2);
	else
		dominancePairFlag[group2] = dominancePairFlag[group2] | (1 << group1);

	if (dominance.dominance0 != 0.0f)
		dominancePairValues[group1] = dominancePairValues[group1] | (1 << group2);
	else
		dominancePairValues[group1] = dominancePairValues[group1] & (~(1 << group2));

	if (dominance.dominance1 != 0.0f)
		dominancePairValues[group2] = dominancePairValues[group2] | (1 << group1);
	else
		dominancePairValues[group2] = dominancePairValues[group2] & (~(1 << group1));
}


PX_INLINE bool SceneBuffer::getDominancePair(PxU32 group1, PxU32 group2, PxDominanceGroupPair& dominance) const
{
	PX_ASSERT(group1 != group2);
	PX_ASSERT(group1 < sMaxNbDominanceGroups);
	PX_ASSERT(group2 < sMaxNbDominanceGroups);

	PxU32 isBuffered = 0;
	if (group1 < group2)
		isBuffered = dominancePairFlag[group1] & (1 << group2);
	else
		isBuffered = dominancePairFlag[group2] & (1 << group1);

	if (isBuffered)
	{
		dominance.dominance0 = (PxReal)( (dominancePairValues[group1] & (1 << group2)) >> group2 );
		dominance.dominance1 = (PxReal)( (dominancePairValues[group2] & (1 << group1)) >> group1 );
		return true;
	}
	
	return false;
}


PX_INLINE void SceneBuffer::syncDominancePairs(Sc::Scene& scene)
{
	for(PxU32 i=0; i < (sMaxNbDominanceGroups - 1); i++)
	{
		if (dominancePairFlag[i])
		{
			for(PxU32 j=(i+1); j < sMaxNbDominanceGroups; j++)
			{
				PxDominanceGroupPair dominance(0.0f, 0.0f);
				if (getDominancePair(i, j, dominance))
				{
					scene.setDominanceGroupPair(PxDominanceGroup(i), PxDominanceGroup(j), dominance);
				}
			}
		}
	}
}

#ifdef PX_PS3
PX_INLINE void SceneBuffer::clearSceneParamBuffer()
{
	for(PxU32 i=0; i<PxPS3ConfigParam::eCOUNT;i++)
	{
		spuParams[i]=USE_DEFAULT_SPU_PARAM;
	}
}

PX_INLINE void SceneBuffer::setSceneParamInt(PxPS3ConfigParam::Enum param, PxU32 value)
{
	spuParams[param]=value;
}

PX_INLINE void SceneBuffer::syncSceneParamInts(Sc::Scene& scene)
{
	for(PxU32 i=0; i<PxPS3ConfigParam::eCOUNT;i++)
	{
		if(spuParams[i]!=USE_DEFAULT_SPU_PARAM)
		{
			scene.setSceneParamInt((PxPS3ConfigParam::Enum)i,spuParams[i]);
			spuParams[i]=USE_DEFAULT_SPU_PARAM;
		}
	}
}
#endif


}  // namespace Scb

}

#endif
