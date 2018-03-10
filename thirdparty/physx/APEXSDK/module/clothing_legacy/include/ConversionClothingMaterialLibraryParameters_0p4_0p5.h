/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CONVERSIONCLOTHINGMATERIALLIBRARYPARAMETERS_0P4_0P5H_H
#define CONVERSIONCLOTHINGMATERIALLIBRARYPARAMETERS_0P4_0P5H_H

#include "ParamConversionTemplate.h"
#include "ClothingMaterialLibraryParameters_0p4.h"
#include "ClothingMaterialLibraryParameters_0p5.h"

namespace physx
{
namespace apex
{
namespace legacy
{

typedef ParamConversionTemplate<ClothingMaterialLibraryParameters_0p4, ClothingMaterialLibraryParameters_0p5, 4, 5> ConversionClothingMaterialLibraryParameters_0p4_0p5Parent;

class ConversionClothingMaterialLibraryParameters_0p4_0p5: ConversionClothingMaterialLibraryParameters_0p4_0p5Parent
{
public:
	static NxParameterized::Conversion* Create(NxParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingMaterialLibraryParameters_0p4_0p5));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingMaterialLibraryParameters_0p4_0p5)(t) : 0;
	}

protected:
	ConversionClothingMaterialLibraryParameters_0p4_0p5(NxParameterized::Traits* t) : ConversionClothingMaterialLibraryParameters_0p4_0p5Parent(t) {}

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

		PxU32 numMaterials = (physx::PxU32)mNewData->materials.arraySizes[0];
		ClothingMaterialLibraryParameters_0p4NS::ClothingMaterial_Type* oldMaterials = mLegacyData->materials.buf;
		ClothingMaterialLibraryParameters_0p5NS::ClothingMaterial_Type* newMaterials = mNewData->materials.buf;
		PX_ASSERT((PxU32)mLegacyData->materials.arraySizes[0] == numMaterials);
		for (PxU32 i = 0; i < numMaterials; ++i)
		{
			ClothingMaterialLibraryParameters_0p4NS::ClothingMaterial_Type& oldMat = oldMaterials[i];
			ClothingMaterialLibraryParameters_0p5NS::ClothingMaterial_Type& newMat = newMaterials[i];
			newMat.verticalStretchingStiffness = oldMat.stretchingStiffness;
			newMat.horizontalStretchingStiffness = oldMat.stretchingStiffness;
			
			newMat.verticalStiffnessScaling.range = 1.0f;
			newMat.verticalStiffnessScaling.scale = 1.0f;

			newMat.horizontalStiffnessScaling.range = oldMat.stretchingLimit.limit;
			newMat.horizontalStiffnessScaling.scale = oldMat.stretchingLimit.stiffness;

			newMat.bendingStiffnessScaling.range = oldMat.bendingLimit.limit;
			newMat.bendingStiffnessScaling.scale = oldMat.bendingLimit.stiffness;

			newMat.shearingStiffnessScaling.range = oldMat.shearingLimit.limit;
			newMat.shearingStiffnessScaling.scale = oldMat.shearingLimit.stiffness;
		}

		return true;
	}
};

}
}
} // namespace physx::apex

#endif
