/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_EMITTER_LOD_PARAM_DESC_H
#define NX_EMITTER_LOD_PARAM_DESC_H

#include "NxApex.h"

namespace physx
{
namespace apex
{


PX_PUSH_PACK_DEFAULT

///LOD parameters fro emitters
class NxEmitterLodParamDesc : public NxApexDesc
{
public:
	physx::PxF32  maxDistance;        ///< Objects greater than this distance from the player will be culled more aggressively
	physx::PxF32  distanceWeight;     ///< Weight given to distance parameter in LOD function
	physx::PxF32  speedWeight;        ///< Weight given to velocity parameter in LOD function
	physx::PxF32  lifeWeight;         ///< Weight given to life remain parameter in LOD function
	physx::PxF32  separationWeight;   ///< Weight given to separation parameter in LOD function

	physx::PxF32  bias;               ///< Bias given to objects spawned by this emitter, relative to other emitters in the same IOS

	/**
	\brief constructor sets to default.
	*/
	PX_INLINE NxEmitterLodParamDesc() : NxApexDesc()
	{
		init();
	}

	/**
	\brief sets members to default values.
	*/
	PX_INLINE void setToDefault()
	{
		NxApexDesc::setToDefault();
		init();
	}

	/**
	\brief checks if this is a valid descriptor.
	*/
	PX_INLINE bool isValid() const
	{
		if (distanceWeight < 0.0f || speedWeight < 0.0f || lifeWeight < 0.0f)
		{
			return false;
		}
		if (separationWeight < 0.0f || maxDistance < 0.0f || bias < 0.0f)
		{
			return false;
		}
		return NxApexDesc::isValid();
	}

	enum ManifestVersions
	{
		initial,

		count,
		current = count - 1
	};

private:

	PX_INLINE void init()
	{
		// These defaults give you pure distance based LOD weighting
		maxDistance = 0.0f;
		distanceWeight = 1.0f;
		speedWeight = 0.0f;
		lifeWeight = 0.0f;
		separationWeight = 0.0f;
		bias = 1.0f;
	}
};


PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_EMITTER_LOD_PARAM_DESC_H
