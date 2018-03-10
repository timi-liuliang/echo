/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CONVERSIONCLOTHINGPHYSICALMESHPARAMETERS_0P9_0P10H_H
#define CONVERSIONCLOTHINGPHYSICALMESHPARAMETERS_0P9_0P10H_H

#include "ParamConversionTemplate.h"
#include "ClothingPhysicalMeshParameters_0p9.h"
#include "ClothingPhysicalMeshParameters_0p10.h"

namespace physx
{
namespace apex
{
namespace legacy
{

typedef ParamConversionTemplate<ClothingPhysicalMeshParameters_0p9, ClothingPhysicalMeshParameters_0p10, 9, 10> ConversionClothingPhysicalMeshParameters_0p9_0p10Parent;

class ConversionClothingPhysicalMeshParameters_0p9_0p10: ConversionClothingPhysicalMeshParameters_0p9_0p10Parent
{
public:
	static NxParameterized::Conversion* Create(NxParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingPhysicalMeshParameters_0p9_0p10));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingPhysicalMeshParameters_0p9_0p10)(t) : 0;
	}

protected:
	ConversionClothingPhysicalMeshParameters_0p9_0p10(NxParameterized::Traits* t) : ConversionClothingPhysicalMeshParameters_0p9_0p10Parent(t) {}

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

		PxF32 meshThickness = mLegacyData->transitionDownThickness;
		NxParameterized::Handle transitionDownHandle(*mNewData, "transitionDown");
		transitionDownHandle.resizeArray(mLegacyData->transitionDownC.arraySizes[0]);
		for (PxI32 i = 0; i < mLegacyData->transitionDownC.arraySizes[0]; ++i)
		{
			const ClothingPhysicalMeshParameters_0p9NS::SkinClothMapC_Type& mapC = mLegacyData->transitionDownC.buf[i];
			ClothingPhysicalMeshParameters_0p10NS::SkinClothMapD_Type& mapD = mNewData->transitionDown.buf[i];

			mapD.vertexBary = mapC.vertexBary;
			mapD.vertexBary.z *= meshThickness;
			mapD.normalBary = mapC.normalBary;
			mapD.normalBary.z *= meshThickness;
			mapD.tangentBary = PxVec3(PX_MAX_F32); // mark tangents as invalid
			mapD.vertexIndexPlusOffset = mapC.vertexIndexPlusOffset;

			// temporarily store face index to update in the ClothingAsset update
			mapD.vertexIndex0 = mapC.faceIndex0;
		}



		meshThickness = mLegacyData->transitionUpThickness;
		NxParameterized::Handle transitionUpHandle(*mNewData, "transitionUp");
		transitionUpHandle.resizeArray(mLegacyData->transitionUpC.arraySizes[0]);
		for (PxI32 i = 0; i < mLegacyData->transitionUpC.arraySizes[0]; ++i)
		{
			const ClothingPhysicalMeshParameters_0p9NS::SkinClothMapC_Type& mapC = mLegacyData->transitionUpC.buf[i];
			ClothingPhysicalMeshParameters_0p10NS::SkinClothMapD_Type& mapD = mNewData->transitionUp.buf[i];

			mapD.vertexBary = mapC.vertexBary;
			mapD.vertexBary.z *= meshThickness;
			mapD.normalBary = mapC.normalBary;
			mapD.normalBary.z *= meshThickness;
			mapD.tangentBary = PxVec3(PX_MAX_F32); // mark tangents as invalid
			mapD.vertexIndexPlusOffset = mapC.vertexIndexPlusOffset;

			// temporarily store face index to update in the ClothingAsset update
			mapD.vertexIndex0 = mapC.faceIndex0;
		}

		return true;
	}
};

}
}
} // namespace physx::apex

#endif
