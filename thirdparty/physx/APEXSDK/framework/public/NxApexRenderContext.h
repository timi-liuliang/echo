/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_APEX_RENDER_CONTEXT_H
#define NX_APEX_RENDER_CONTEXT_H

/*!
\file
\brief class NxApexRenderContext
*/

#include "NxApexUsingNamespace.h"
#include "foundation/PxMat44.h"

namespace physx
{
namespace apex
{

class NxUserRenderResource;
class NxUserOpaqueMesh;

PX_PUSH_PACK_DEFAULT

/**
\brief Describes the context of a renderable object
*/
class NxApexRenderContext
{
public:
	NxApexRenderContext(void)
	{
		renderResource = 0;
		isScreenSpace = false;
        renderMeshName = NULL;
	}

public:
	NxUserRenderResource*	renderResource;	//!< The renderable resource to be rendered
	bool					isScreenSpace;		//!< The data is in screenspace and should use a screenspace projection that transforms X -1 to +1 and Y -1 to +1 with zbuffer disabled.
	physx::PxMat44			local2world;		//!< Reverse world pose transform for this renderable
	physx::PxMat44			world2local;		//!< World pose transform for this renderable
    const char*             renderMeshName;     //!< The name of the render mesh this context is associated with.
};

PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_APEX_RENDER_CONTEXT_H
