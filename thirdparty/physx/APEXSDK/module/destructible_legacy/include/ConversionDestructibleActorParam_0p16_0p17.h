/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CONVERSIONDESTRUCTIBLEACTORPARAM_0P16_0P17H_H
#define CONVERSIONDESTRUCTIBLEACTORPARAM_0P16_0P17H_H

#include "ParamConversionTemplate.h"
#include "DestructibleActorParam_0p16.h"
#include "DestructibleActorParam_0p17.h"

namespace physx
{
namespace apex
{
namespace legacy
{

typedef ParamConversionTemplate<DestructibleActorParam_0p16, DestructibleActorParam_0p17, 16, 17> ConversionDestructibleActorParam_0p16_0p17Parent;

class ConversionDestructibleActorParam_0p16_0p17: ConversionDestructibleActorParam_0p16_0p17Parent
{
public:
	static NxParameterized::Conversion* Create(NxParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionDestructibleActorParam_0p16_0p17));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionDestructibleActorParam_0p16_0p17)(t) : 0;
	}

protected:
	ConversionDestructibleActorParam_0p16_0p17(NxParameterized::Traits* t) : ConversionDestructibleActorParam_0p16_0p17Parent(t) {}

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
		// Convert default behavior group's damage spread function parameters
		mNewData->defaultBehaviorGroup.damageSpread.minimumRadius = mLegacyData->defaultBehaviorGroup.minimumDamageRadius;
		mNewData->defaultBehaviorGroup.damageSpread.radiusMultiplier = mLegacyData->defaultBehaviorGroup.damageRadiusMultiplier;
		mNewData->defaultBehaviorGroup.damageSpread.falloffExponent = mLegacyData->defaultBehaviorGroup.damageFalloffExponent;

		// Convert user-defined behavior group's damage spread function parameters
		const physx::PxI32 behaviorGroupCount = mLegacyData->behaviorGroups.arraySizes[0];
		PX_ASSERT(mNewData->behaviorGroups.arraySizes[0] == behaviorGroupCount);
		for (physx::PxI32 i = 0; i < physx::PxMin(behaviorGroupCount, mNewData->behaviorGroups.arraySizes[0]); ++i)
		{
			mNewData->behaviorGroups.buf[i].damageSpread.minimumRadius = mLegacyData->behaviorGroups.buf[i].minimumDamageRadius;
			mNewData->behaviorGroups.buf[i].damageSpread.radiusMultiplier = mLegacyData->behaviorGroups.buf[i].damageRadiusMultiplier;
			mNewData->behaviorGroups.buf[i].damageSpread.falloffExponent = mLegacyData->behaviorGroups.buf[i].damageFalloffExponent;
		}

		return true;
	}
};

}
}
} // namespace physx::apex

#endif
