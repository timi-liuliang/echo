/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CONVERSIONCLOTHINGACTORPARAM_0P12_0P13H_H
#define CONVERSIONCLOTHINGACTORPARAM_0P12_0P13H_H

#include "ParamConversionTemplate.h"
#include "ClothingActorParam_0p12.h"
#include "ClothingActorParam_0p13.h"

namespace physx
{
namespace apex
{
namespace legacy
{

typedef ParamConversionTemplate<ClothingActorParam_0p12, ClothingActorParam_0p13, 12, 13> ConversionClothingActorParam_0p12_0p13Parent;

class ConversionClothingActorParam_0p12_0p13: ConversionClothingActorParam_0p12_0p13Parent
{
public:
	static NxParameterized::Conversion* Create(NxParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingActorParam_0p12_0p13));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingActorParam_0p12_0p13)(t) : 0;
	}

protected:
	ConversionClothingActorParam_0p12_0p13(NxParameterized::Traits* t) : ConversionClothingActorParam_0p12_0p13Parent(t) {}

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

		PX_UNUSED(prefVers[0]); // Make compiler happy

		return 0;
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
