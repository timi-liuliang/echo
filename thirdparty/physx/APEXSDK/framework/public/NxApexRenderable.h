/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_APEX_RENDERABLE_H
#define NX_APEX_RENDERABLE_H

/*!
\file
\brief class NxApexRenderable
*/

#include "NxApexRenderDataProvider.h"

namespace physx
{
namespace apex
{

class NxUserRenderer;

PX_PUSH_PACK_DEFAULT

/**
\brief Base class of any actor that can be rendered
 */
class NxApexRenderable : public NxApexRenderDataProvider
{
public:
	/**
	When called, this method will use the NxUserRenderer interface to render itself (if visible, etc)
	by calling renderer.renderResource( NxApexRenderContext& ) as many times as necessary.   See locking
	semantics for NxApexRenderDataProvider::lockRenderResources().
	*/
	virtual void dispatchRenderResources(NxUserRenderer& renderer) = 0;

	/**
	Returns AABB covering rendered data.  The actor's world bounds is updated each frame
	during NxApexScene::fetchResults().  This function does not require the NxApexRenderable actor to be locked.
	*/
	virtual physx::PxBounds3 getBounds() const = 0;
};

PX_POP_PACK

}
} // end namespace physx::apex

#endif
