/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_USER_RENDERER_H
#define NX_USER_RENDERER_H

/*!
\file
\brief class NxUserRenderer
*/

#include "NxApexUsingNamespace.h"

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

class NxApexRenderContext;

/**
\brief User provided renderer interface
*/
class NxUserRenderer
{
public:
	virtual ~NxUserRenderer() {}
	/**
	\brief Render a resource

	NxApexRenderable::dispatchRenderResouces() will call this
	function as many times as possible to render all of the actor's
	sub-meshes.
	*/
	virtual void renderResource(const NxApexRenderContext& context) = 0;
};

PX_POP_PACK

}
} // end namespace physx::apex

#endif
