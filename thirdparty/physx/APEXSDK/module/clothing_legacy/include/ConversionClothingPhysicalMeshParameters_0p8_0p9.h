/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CONVERSIONCLOTHINGPHYSICALMESHPARAMETERS_0P8_0P9H_H
#define CONVERSIONCLOTHINGPHYSICALMESHPARAMETERS_0P8_0P9H_H

#include "ParamConversionTemplate.h"
#include "ClothingPhysicalMeshParameters_0p8.h"
#include "ClothingPhysicalMeshParameters_0p9.h"

namespace physx
{
namespace apex
{
namespace legacy
{

typedef ParamConversionTemplate<ClothingPhysicalMeshParameters_0p8, ClothingPhysicalMeshParameters_0p9, 8, 9> ConversionClothingPhysicalMeshParameters_0p8_0p9Parent;

class ConversionClothingPhysicalMeshParameters_0p8_0p9: ConversionClothingPhysicalMeshParameters_0p8_0p9Parent
{
public:
	static NxParameterized::Conversion* Create(NxParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingPhysicalMeshParameters_0p8_0p9));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingPhysicalMeshParameters_0p8_0p9)(t) : 0;
	}

protected:
	ConversionClothingPhysicalMeshParameters_0p8_0p9(NxParameterized::Traits* t) : ConversionClothingPhysicalMeshParameters_0p8_0p9Parent(t) {}

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

		bool hasMaxDist0verts = false;
		for (PxI32 i = 0; i < mNewData->submeshes.arraySizes[0]; i++)
		{
			hasMaxDist0verts |= mNewData->submeshes.buf[i].numMaxDistance0Vertices > 0;
		}

		// make sure that only assets that need to be sorted will be sorted
		mNewData->physicalMesh.physicalMeshSorting = hasMaxDist0verts ? 1u : 0u;

		return true;
	}
};

}
}
} // namespace physx::apex

#endif
