/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef __NULL_RENDERER_H_
#define __NULL_RENDERER_H_

#include "NxApex.h"

/* This class is intended for use by command line tools that require an
 * APEX SDK.  Apps which use this renderer should _NOT_ call
 * updateRenderResources() or dispatchRenderResources().  You _WILL_
 * crash.
 */

namespace physx
{
namespace apex
{

class NullRenderResourceManager : public NxUserRenderResourceManager
{
public:
	NxUserRenderVertexBuffer*	createVertexBuffer(const NxUserRenderVertexBufferDesc&)
	{
		return NULL;
	}
	NxUserRenderIndexBuffer*	createIndexBuffer(const NxUserRenderIndexBufferDesc&)
	{
		return NULL;
	}
	NxUserRenderBoneBuffer*		createBoneBuffer(const NxUserRenderBoneBufferDesc&)
	{
		return NULL;
	}
	NxUserRenderInstanceBuffer*	createInstanceBuffer(const NxUserRenderInstanceBufferDesc&)
	{
		return NULL;
	}
	NxUserRenderSpriteBuffer*   createSpriteBuffer(const NxUserRenderSpriteBufferDesc&)
	{
		return NULL;
	}
	NxUserRenderSurfaceBuffer*   createSurfaceBuffer(const NxUserRenderSurfaceBufferDesc&)
	{
		return NULL;
	}
	NxUserRenderResource*		createResource(const NxUserRenderResourceDesc&)
	{
		return NULL;
	}
	void						releaseVertexBuffer(NxUserRenderVertexBuffer&) {}
	void						releaseIndexBuffer(NxUserRenderIndexBuffer&) {}
	void						releaseBoneBuffer(NxUserRenderBoneBuffer&) {}
	void						releaseInstanceBuffer(NxUserRenderInstanceBuffer&) {}
	void						releaseSpriteBuffer(NxUserRenderSpriteBuffer&) {}
	void						releaseSurfaceBuffer(NxUserRenderSurfaceBuffer&) {}
	void						releaseResource(NxUserRenderResource&) {}
	physx::PxU32						getMaxBonesForMaterial(void*)
	{
		return 0;
	}

	/** \brief Get the sprite layout data */
	virtual bool getSpriteLayoutData(physx::PxU32 spriteCount, physx::PxU32 spriteSemanticsBitmap, physx::apex::NxUserRenderSpriteBufferDesc* textureDescArray) 
	{
		PX_UNUSED(spriteCount);
		PX_UNUSED(spriteSemanticsBitmap);
		PX_UNUSED(textureDescArray);
		return false;
	}

	/** \brief Get the instance layout data */
	virtual bool getInstanceLayoutData(physx::PxU32 particleCount, physx::PxU32 particleSemanticsBitmap, physx::apex::NxUserRenderInstanceBufferDesc* instanceDescArray) 
	{
		PX_UNUSED(particleCount);
		PX_UNUSED(particleSemanticsBitmap);
		PX_UNUSED(instanceDescArray);
		return false;
	}


};

}
} // end namespace physx::apex

#endif
