/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CONVERSIONCLOTHINGCOOKEDPARAM_0P1_0P2H_H
#define CONVERSIONCLOTHINGCOOKEDPARAM_0P1_0P2H_H

#include "ParamConversionTemplate.h"
#include "ClothingCookedParam_0p1.h"
#include "ClothingCookedParam_0p2.h"

namespace physx
{
namespace apex
{
namespace legacy
{

typedef ParamConversionTemplate<ClothingCookedParam_0p1, ClothingCookedParam_0p2, 1, 2> ConversionClothingCookedParam_0p1_0p2Parent;

class ConversionClothingCookedParam_0p1_0p2: ConversionClothingCookedParam_0p1_0p2Parent
{
public:
	static NxParameterized::Conversion* Create(NxParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingCookedParam_0p1_0p2));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingCookedParam_0p1_0p2)(t) : 0;
	}

protected:
	ConversionClothingCookedParam_0p1_0p2(NxParameterized::Traits* t) : ConversionClothingCookedParam_0p1_0p2Parent(t) {}

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
