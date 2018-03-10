/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __CONVERSIONCLOTHINGPHYSICALMESHPARAMETERS_0P0_0P1H__
#define __CONVERSIONCLOTHINGPHYSICALMESHPARAMETERS_0P0_0P1H__

#include "ParamConversionTemplate.h"
#include "ClothingPhysicalMeshParameters_0p0.h"
#include "ClothingPhysicalMeshParameters_0p1.h"

namespace physx
{
namespace apex
{
namespace legacy
{

typedef ParamConversionTemplate<ClothingPhysicalMeshParameters_0p0, ClothingPhysicalMeshParameters_0p1, 0, 1> ConversionClothingPhysicalMeshParameters_0p0_0p1Parent;

class ConversionClothingPhysicalMeshParameters_0p0_0p1: ConversionClothingPhysicalMeshParameters_0p0_0p1Parent
{
public:
	static NxParameterized::Conversion* Create(NxParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingPhysicalMeshParameters_0p0_0p1));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingPhysicalMeshParameters_0p0_0p1)(t) : 0;
	}

protected:
	ConversionClothingPhysicalMeshParameters_0p0_0p1(NxParameterized::Traits* t) : ConversionClothingPhysicalMeshParameters_0p0_0p1Parent(t) {}

	bool convert()
	{
		//TODO:
		//	Write custom conversion code here using mNewData and mLegacyData members.
		//
		//	Note that
		//		- mNewData was initialized with default values
		//		- same-named/same-typed members were copied from mLegacyData to mNewData
		//		- included references were moved to mNewData
		//			(and updated to preferred versions according to getPreferredVersions)
		//
		//	For more info see the versioning wiki.

		for (PxI32 i = 0; i < mNewData->submeshes.arraySizes[0]; i++)
		{
			mNewData->submeshes.buf[i].nxDeformableMesh = NULL;
		}

		return true;
	}
};

}
}
} //end of physx::apex:: namespace

#endif
