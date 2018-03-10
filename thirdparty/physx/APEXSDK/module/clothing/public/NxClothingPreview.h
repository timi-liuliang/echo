/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_CLOTHING_PREVIEW_H
#define NX_CLOTHING_PREVIEW_H

#include "NxApexAssetPreview.h"

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT



/**
\brief A lightweight preview of a NxClothingActor that will not perform any simulation
*/
class NxClothingPreview : public NxApexAssetPreview
{

public:

	/**
	\brief Updates all internal bone matrices.
	\see NxClothingActor::updateState
	*/
	virtual void updateState(const physx::PxMat44& globalPose, const physx::PxMat44* newBoneMatrices, physx::PxU32 boneMatricesByteStride, physx::PxU32 numBoneMatrices) = 0;

};


PX_POP_PACK

}
} // namespace physx::apex

#endif // NX_CLOTHING_PREVIEW_H
