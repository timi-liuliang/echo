/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CONVERSIONDESTRUCTIBLEACTORPARAM_0P18_0P19H_H
#define CONVERSIONDESTRUCTIBLEACTORPARAM_0P18_0P19H_H

#include "ParamConversionTemplate.h"
#include "DestructibleActorParam_0p18.h"
#include "DestructibleActorParam_0p19.h"

namespace physx
{
namespace apex
{
namespace legacy
{

typedef ParamConversionTemplate<DestructibleActorParam_0p18, DestructibleActorParam_0p19, 18, 19> ConversionDestructibleActorParam_0p18_0p19Parent;

class ConversionDestructibleActorParam_0p18_0p19: ConversionDestructibleActorParam_0p18_0p19Parent
{
public:
	static NxParameterized::Conversion* Create(NxParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionDestructibleActorParam_0p18_0p19));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionDestructibleActorParam_0p18_0p19)(t) : 0;
	}

protected:
	ConversionDestructibleActorParam_0p18_0p19(NxParameterized::Traits* t) : ConversionDestructibleActorParam_0p18_0p19Parent(t) {}

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
		mNewData->p3BodyDescTemplate.flags.eENABLE_CCD = mLegacyData->p3ShapeDescTemplate.flags.eUSE_SWEPT_BOUNDS;
		mNewData->p3ActorDescTemplate.contactReportFlags.eCCD_LINEAR = mLegacyData->p3ActorDescTemplate.contactReportFlags.eSWEPT_INTEGRATION_LINEAR;
		return true;
	}
};

}
}
} // namespace physx::apex

#endif
