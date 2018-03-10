/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __CONVERSIONAPEXEMITTERASSETPARAMETERS_0P1_0P2H__
#define __CONVERSIONAPEXEMITTERASSETPARAMETERS_0P1_0P2H__

#include "ParamConversionTemplate.h"
#include "ApexEmitterAssetParameters_0p1.h"
#include "ApexEmitterAssetParameters_0p2.h"

namespace physx
{
namespace apex
{
namespace legacy
{

typedef ParamConversionTemplate<ApexEmitterAssetParameters_0p1, ApexEmitterAssetParameters_0p2, 1, 2> ConversionApexEmitterAssetParameters_0p1_0p2Parent;

class ConversionApexEmitterAssetParameters_0p1_0p2: ConversionApexEmitterAssetParameters_0p1_0p2Parent
{
public:
	static NxParameterized::Conversion* Create(NxParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionApexEmitterAssetParameters_0p1_0p2));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionApexEmitterAssetParameters_0p1_0p2)(t) : 0;
	}

protected:
	ConversionApexEmitterAssetParameters_0p1_0p2(NxParameterized::Traits* t) : ConversionApexEmitterAssetParameters_0p1_0p2Parent(t) {}

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

		return true;
	}
};

}
}
} //end of physx::apex:: namespace

#endif
