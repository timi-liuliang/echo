/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CONVERSIONDESTRUCTIBLEASSETPARAMETERS_0P2_0P3H_H
#define CONVERSIONDESTRUCTIBLEASSETPARAMETERS_0P2_0P3H_H

#include "ParamConversionTemplate.h"
#include "DestructibleAssetParameters_0p2.h"
#include "DestructibleAssetParameters_0p3.h"

namespace physx
{
namespace apex
{
namespace legacy
{

typedef ParamConversionTemplate<DestructibleAssetParameters_0p2, DestructibleAssetParameters_0p3, 2, 3> ConversionDestructibleAssetParameters_0p2_0p3Parent;

class ConversionDestructibleAssetParameters_0p2_0p3: ConversionDestructibleAssetParameters_0p2_0p3Parent
{
public:
	static NxParameterized::Conversion* Create(NxParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionDestructibleAssetParameters_0p2_0p3));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionDestructibleAssetParameters_0p2_0p3)(t) : 0;
	}

protected:
	ConversionDestructibleAssetParameters_0p2_0p3(NxParameterized::Traits* t) : ConversionDestructibleAssetParameters_0p2_0p3Parent(t) {}

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
		mNewData->massScaleExponent = mLegacyData->destructibleParameters.massScaleExponent;
		mNewData->supportDepth = mLegacyData->destructibleParameters.supportDepth;
		mNewData->formExtendedStructures = mLegacyData->destructibleParameters.formExtendedStructures != 0;
		mNewData->useAssetDefinedSupport = mLegacyData->destructibleParameters.flags.ASSET_DEFINED_SUPPORT;
		mNewData->useWorldSupport = mLegacyData->destructibleParameters.flags.WORLD_SUPPORT;

		return true;
	}
};

} // namespace legacy
} // namespace apex
} // namespace physx

#endif
