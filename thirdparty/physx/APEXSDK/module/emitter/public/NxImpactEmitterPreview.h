/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_IMPACT_EMITTER_PREVIEW_H
#define NX_IMPACT_EMITTER_PREVIEW_H

#include "NxApex.h"
#include "NxApexAssetPreview.h"

namespace physx
{
namespace apex
{


PX_PUSH_PACK_DEFAULT

///Impact emitter preview. Used for preview rendering of the emitter actors.
class NxImpactEmitterPreview : public NxApexAssetPreview
{
public:
	/* No need to add any functionality here */

protected:
	NxImpactEmitterPreview() {}
};


PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_IMPACT_EMITTER_PREVIEW_H
