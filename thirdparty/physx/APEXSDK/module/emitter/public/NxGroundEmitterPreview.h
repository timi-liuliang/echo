/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_GROUND_EMITTER_PREVIEW_H
#define NX_GROUND_EMITTER_PREVIEW_H

#include "NxApex.h"
#include "NxApexAssetPreview.h"

namespace physx
{
namespace apex
{


PX_PUSH_PACK_DEFAULT

///Ground emitter preview class. Use for preview rendering
class NxGroundEmitterPreview : public NxApexAssetPreview
{
public:
	///Does the drawing
	virtual void	drawEmitterPreview() = 0;
	///Sets the scale factor of the preview object
	virtual void	setScale(physx::PxF32 scale) = 0;

protected:
	NxGroundEmitterPreview() {}
};


PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_GROUND_EMITTER_PREVIEW_H
