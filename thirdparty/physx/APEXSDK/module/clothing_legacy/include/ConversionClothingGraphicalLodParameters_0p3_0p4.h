/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CONVERSIONCLOTHINGGRAPHICALLODPARAMETERS_0P3_0P4H_H
#define CONVERSIONCLOTHINGGRAPHICALLODPARAMETERS_0P3_0P4H_H

#include "ParamConversionTemplate.h"
#include "ClothingGraphicalLodParameters_0p3.h"
#include "ClothingGraphicalLodParameters_0p4.h"

namespace physx
{
namespace apex
{
namespace legacy
{

typedef ParamConversionTemplate<ClothingGraphicalLodParameters_0p3, ClothingGraphicalLodParameters_0p4, 3, 4> ConversionClothingGraphicalLodParameters_0p3_0p4Parent;

class ConversionClothingGraphicalLodParameters_0p3_0p4: ConversionClothingGraphicalLodParameters_0p3_0p4Parent
{
public:
	static NxParameterized::Conversion* Create(NxParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingGraphicalLodParameters_0p3_0p4));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingGraphicalLodParameters_0p3_0p4)(t) : 0;
	}

protected:
	ConversionClothingGraphicalLodParameters_0p3_0p4(NxParameterized::Traits* t) : ConversionClothingGraphicalLodParameters_0p3_0p4Parent(t) {}

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

		PxF32 meshThickness = mLegacyData->skinClothMapThickness;
		NxParameterized::Handle skinClothMap(*mNewData, "skinClothMap");
		skinClothMap.resizeArray(mLegacyData->skinClothMapC.arraySizes[0]);
		for (PxI32 i = 0; i < mLegacyData->skinClothMapC.arraySizes[0]; ++i)
		{
			const ClothingGraphicalLodParameters_0p3NS::SkinClothMapC_Type& mapC = mLegacyData->skinClothMapC.buf[i];
			ClothingGraphicalLodParameters_0p4NS::SkinClothMapD_Type& mapD = mNewData->skinClothMap.buf[i];

			mapD.vertexBary = mapC.vertexBary;
			mapD.vertexBary.z *= meshThickness;
			mapD.normalBary = mapC.normalBary;
			mapD.normalBary.z *= meshThickness;
			mapD.tangentBary = PxVec3(PX_MAX_F32); // mark tangents as invalid
			mapD.vertexIndexPlusOffset = mapC.vertexIndexPlusOffset;
			//PX_ASSERT((PxU32)i == mapC.vertexIndexPlusOffset);

			// Temporarily store the face index. The ClothingAsset update will look up the actual vertex indices.
			mapD.vertexIndex0 = mapC.faceIndex0;
		}

		return true;
	}
};

}
}
} // namespace physx::apex

#endif
