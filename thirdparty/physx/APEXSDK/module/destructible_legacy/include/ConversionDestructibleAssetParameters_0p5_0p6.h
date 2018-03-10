/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CONVERSIONDESTRUCTIBLEASSETPARAMETERS_0P5_0P6H_H
#define CONVERSIONDESTRUCTIBLEASSETPARAMETERS_0P5_0P6H_H

#include "ParamConversionTemplate.h"
#include "DestructibleAssetParameters_0p5.h"
#include "DestructibleAssetParameters_0p6.h"

namespace physx
{
namespace apex
{
namespace legacy
{

typedef ParamConversionTemplate<DestructibleAssetParameters_0p5, DestructibleAssetParameters_0p6, 5, 6> ConversionDestructibleAssetParameters_0p5_0p6Parent;

class ConversionDestructibleAssetParameters_0p5_0p6: ConversionDestructibleAssetParameters_0p5_0p6Parent
{
public:
	static NxParameterized::Conversion* Create(NxParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionDestructibleAssetParameters_0p5_0p6));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionDestructibleAssetParameters_0p5_0p6)(t) : 0;
	}

protected:
	ConversionDestructibleAssetParameters_0p5_0p6(NxParameterized::Traits* t) : ConversionDestructibleAssetParameters_0p5_0p6Parent(t) {}

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
		//TODO:
		//	Write custom conversion code here using mNewData and mLegacyData members.
		//
		//	Note that
		//		- mNewData has already been initialized with default values
		//		- same-named/same-typed members have already been copied
		//			from mLegacyData to mNewData
		//		- included references were moved to mNewData
		//			(and updated to preferred versions according to getPreferredVersions)
		//
		//	For more info see the versioning wiki.

		return true;
	}
};

}
}
} // namespace physx::apex

#endif
