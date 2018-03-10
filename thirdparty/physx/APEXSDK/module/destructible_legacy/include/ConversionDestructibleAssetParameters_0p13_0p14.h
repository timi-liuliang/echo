/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CONVERSIONDESTRUCTIBLEASSETPARAMETERS_0P13_0P14H_H
#define CONVERSIONDESTRUCTIBLEASSETPARAMETERS_0P13_0P14H_H

#include "ParamConversionTemplate.h"
#include "DestructibleAssetParameters_0p13.h"
#include "DestructibleAssetParameters_0p14.h"

#include <algorithm> // for find_if

namespace physx
{
namespace apex
{
namespace legacy
{

typedef ParamConversionTemplate<DestructibleAssetParameters_0p13, DestructibleAssetParameters_0p14, 13, 14> ConversionDestructibleAssetParameters_0p13_0p14Parent;

class ConversionDestructibleAssetParameters_0p13_0p14: ConversionDestructibleAssetParameters_0p13_0p14Parent
{
public:
	static NxParameterized::Conversion* Create(NxParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionDestructibleAssetParameters_0p13_0p14));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionDestructibleAssetParameters_0p13_0p14)(t) : 0;
	}

protected:
	ConversionDestructibleAssetParameters_0p13_0p14(NxParameterized::Traits* t) : ConversionDestructibleAssetParameters_0p13_0p14Parent(t) {}

	const NxParameterized::PrefVer* getPreferredVersions() const
	{
		static NxParameterized::PrefVer prefVers[] =
		{
			//TODO:
			//	Add your preferred versions for included references here.
			//	Entry format is
			//		{ (const char*)longName, (PxU32)preferredVersion }

			{ 0, 0 } // Terminator (do not remove!)
		};

		return prefVers;
	}

	bool convert()
	{
		typedef DestructibleAssetParameters_0p14NS::BehaviorGroup_Type BehaviorGroup;

		const PxI8 DEFAULT_GROUP = -1;
		mNewData->RTFractureBehaviorGroup = DEFAULT_GROUP;
		mNewData->defaultBehaviorGroup.damageThreshold = mLegacyData->destructibleParameters.damageThreshold;
		mNewData->defaultBehaviorGroup.damageToRadius = mLegacyData->destructibleParameters.damageToRadius;
		mNewData->defaultBehaviorGroup.materialStrength = mLegacyData->destructibleParameters.materialStrength;
		
		PX_ASSERT(mLegacyData->chunks.arraySizes[0] == mNewData->chunks.arraySizes[0]);
		for (PxI32 chunkIndex = 0; chunkIndex < mLegacyData->chunks.arraySizes[0]; ++chunkIndex)
		{
			DestructibleAssetParameters_0p14NS::Chunk_Type& newChunk = mNewData->chunks.buf[chunkIndex];
			newChunk.behaviorGroupIndex = DEFAULT_GROUP;
		}

		return true;
	}
};

}
}
} // namespace physx::apex

#endif
