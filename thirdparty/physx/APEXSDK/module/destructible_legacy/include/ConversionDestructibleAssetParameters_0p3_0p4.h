/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CONVERSIONDESTRUCTIBLEASSETPARAMETERS_0P3_0P4H_H
#define CONVERSIONDESTRUCTIBLEASSETPARAMETERS_0P3_0P4H_H

#include "ParamConversionTemplate.h"
#include "DestructibleAssetParameters_0p3.h"
#include "DestructibleAssetParameters_0p4.h"

namespace physx
{
namespace apex
{
namespace legacy
{

typedef ParamConversionTemplate<DestructibleAssetParameters_0p3, DestructibleAssetParameters_0p4, 3, 4> ConversionDestructibleAssetParameters_0p3_0p4Parent;

class ConversionDestructibleAssetParameters_0p3_0p4: ConversionDestructibleAssetParameters_0p3_0p4Parent
{
public:
	static NxParameterized::Conversion* Create(NxParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionDestructibleAssetParameters_0p3_0p4));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionDestructibleAssetParameters_0p3_0p4)(t) : 0;
	}

protected:
	ConversionDestructibleAssetParameters_0p3_0p4(NxParameterized::Traits* t) : ConversionDestructibleAssetParameters_0p3_0p4Parent(t) {}

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
		NxParameterized::Handle handle(*mNewData);
		mNewData->getParameterHandle("chunkConvexHullStartIndices", handle);
		mNewData->resizeArray(handle, mNewData->chunks.arraySizes[0] + 1);
		for (physx::PxI32 i = 0; i <= mNewData->chunks.arraySizes[0]; ++i)
		{
			mNewData->chunkConvexHullStartIndices.buf[i] = (physx::PxU32)i;
		}

		return true;
	}
};

} // namespace legacy
} // namespace apex
} // namespace physx

#endif
