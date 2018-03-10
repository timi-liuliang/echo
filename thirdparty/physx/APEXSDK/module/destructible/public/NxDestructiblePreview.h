/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_DESTRUCTIBLE_PREVIEW_H
#define NX_DESTRUCTIBLE_PREVIEW_H

#include "NxApexAssetPreview.h"
#include "NxRenderMeshActor.h"

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT


/**
\brief Destructible module's derivation NxApexAssetPreview.
*/

class NxDestructiblePreview : public NxApexAssetPreview
{
public:
	virtual const NxRenderMeshActor* getRenderMeshActor() const = 0;

	/**
	\brief Sets the preview chunk depth and "explode" amount (pushes the chunks out by that relative distance).
	\param [in] depth Must be nonnegative.  Internally the value will be clamped to the depth range in the asset.
	\param [in] explode must be nonnegative.
	*/
	virtual void	setExplodeView(physx::PxU32 depth, physx::PxF32 explode) = 0;

protected:
	NxDestructiblePreview() {}
};

PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_DESTRUCTIBLE_PREVIEW_H
