/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CONVERSIONDESTRUCTIBLEACTORPARAM_0P29_0P30H_H
#define CONVERSIONDESTRUCTIBLEACTORPARAM_0P29_0P30H_H

#include "ParamConversionTemplate.h"
#include "DestructibleActorParam_0p29.h"
#include "DestructibleActorParam_0p30.h"

namespace physx
{
namespace apex
{

typedef ParamConversionTemplate<DestructibleActorParam_0p29, DestructibleActorParam_0p30, 29, 30> ConversionDestructibleActorParam_0p29_0p30Parent;

class ConversionDestructibleActorParam_0p29_0p30: ConversionDestructibleActorParam_0p29_0p30Parent
{
public:
	static NxParameterized::Conversion* Create(NxParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionDestructibleActorParam_0p29_0p30));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionDestructibleActorParam_0p29_0p30)(t) : 0;
	}

protected:
	ConversionDestructibleActorParam_0p29_0p30(NxParameterized::Traits* t) : ConversionDestructibleActorParam_0p29_0p30Parent(t) {}

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
		if (mLegacyData->p3ActorDescTemplate.contactReportFlags.eRESOLVE_CONTACTS)
		{
			mNewData->p3ActorDescTemplate.contactReportFlags.eCONTACT_DEFAULT = true;
		}

		if (mLegacyData->p3ActorDescTemplate.contactReportFlags.eCCD_LINEAR)
		{
			mNewData->p3ActorDescTemplate.contactReportFlags.eSOLVE_CONTACT = true;
			mNewData->p3ActorDescTemplate.contactReportFlags.eDETECT_CCD_CONTACT = true;
		}

		return true;
	}
};

}
} // namespace physx::apex

#endif
