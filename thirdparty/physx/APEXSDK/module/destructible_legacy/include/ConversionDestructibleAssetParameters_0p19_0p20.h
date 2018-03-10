/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CONVERSIONDESTRUCTIBLEASSETPARAMETERS_0P19_0P20H_H
#define CONVERSIONDESTRUCTIBLEASSETPARAMETERS_0P19_0P20H_H

#include "ParamConversionTemplate.h"
#include "DestructibleAssetParameters_0p19.h"
#include "DestructibleAssetParameters_0p20.h"

namespace physx
{
namespace apex
{
namespace legacy
{

typedef ParamConversionTemplate<DestructibleAssetParameters_0p19, DestructibleAssetParameters_0p20, 19, 20> ConversionDestructibleAssetParameters_0p19_0p20Parent;

class ConversionDestructibleAssetParameters_0p19_0p20: ConversionDestructibleAssetParameters_0p19_0p20Parent
{
public:
	static NxParameterized::Conversion* Create(NxParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionDestructibleAssetParameters_0p19_0p20));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionDestructibleAssetParameters_0p19_0p20)(t) : 0;
	}

protected:
	ConversionDestructibleAssetParameters_0p19_0p20(NxParameterized::Traits* t) : ConversionDestructibleAssetParameters_0p19_0p20Parent(t) {}

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
		// Convert to new behaviorGroup index convention

		const physx::PxI32 chunkCount = mNewData->chunks.arraySizes[0];
		for (physx::PxI32 chunkIndex = 0; chunkIndex < chunkCount; ++chunkIndex)
		{
			DestructibleAssetParameters_0p20NS::Chunk_Type& chunk = mNewData->chunks.buf[chunkIndex];
			if (chunk.parentIndex < chunkCount && chunk.behaviorGroupIndex < 0)
			{
				chunk.behaviorGroupIndex = mNewData->chunks.buf[chunk.parentIndex].behaviorGroupIndex;
			}
		}

		return true;
	}
};

}
}
} // namespace physx::apex

#endif
