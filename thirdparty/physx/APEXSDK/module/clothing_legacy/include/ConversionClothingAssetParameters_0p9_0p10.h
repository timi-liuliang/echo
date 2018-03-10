/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CONVERSIONCLOTHINGASSETPARAMETERS_0P9_0P10H_H
#define CONVERSIONCLOTHINGASSETPARAMETERS_0P9_0P10H_H

#include "ParamConversionTemplate.h"
#include "ClothingAssetParameters_0p9.h"
#include "ClothingAssetParameters_0p10.h"
#include "ClothingMaterialLibraryParameters_0p11.h"

namespace physx
{
namespace apex
{
namespace legacy
{

typedef ParamConversionTemplate<ClothingAssetParameters_0p9, ClothingAssetParameters_0p10, 9, 10> ConversionClothingAssetParameters_0p9_0p10Parent;

class ConversionClothingAssetParameters_0p9_0p10: ConversionClothingAssetParameters_0p9_0p10Parent
{
public:
	static NxParameterized::Conversion* Create(NxParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingAssetParameters_0p9_0p10));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingAssetParameters_0p9_0p10)(t) : 0;
	}

protected:
	ConversionClothingAssetParameters_0p9_0p10(NxParameterized::Traits* t) : ConversionClothingAssetParameters_0p9_0p10Parent(t) {}

	const NxParameterized::PrefVer* getPreferredVersions() const
	{
		static NxParameterized::PrefVer prefVers[] =
		{
			//TODO:
			//	Add your preferred versions for included references here.
			//	Entry format is
			//		{ (const char*)longName, (PxU32)preferredVersion }
			{ "materialLibrary", 11 },
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
		ClothingMaterialLibraryParameters_0p11* matLib = (ClothingMaterialLibraryParameters_0p11*)mNewData->materialLibrary;
		for (PxI32 i = 0; i < matLib->materials.arraySizes[0]; ++i)
		{
			matLib->materials.buf[i].selfcollisionThickness =
				mLegacyData->simulation.selfcollision
				? mLegacyData->simulation.selfcollisionThickness
				: 0.0f;
		}
		
		return true;
	}
};

}
}
} // namespace physx::apex

#endif
