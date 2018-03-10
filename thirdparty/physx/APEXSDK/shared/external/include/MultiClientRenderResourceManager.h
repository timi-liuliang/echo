/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MULTI_CLIENT_RENDER_RESOURCE_MANAGER_H
#define MULTI_CLIENT_RENDER_RESOURCE_MANAGER_H


#include "NxUserRenderResourceManager.h"
#include "NxUserRenderer.h"

#include <vector>


class MultiClientRenderResourceManager : public physx::apex::NxUserRenderResourceManager
{
public:

	MultiClientRenderResourceManager();
	~MultiClientRenderResourceManager();

	void addChild(physx::apex::NxUserRenderResourceManager* rrm, bool destroyAutomatic);


	virtual physx::apex::NxUserRenderVertexBuffer*   createVertexBuffer(const physx::apex::NxUserRenderVertexBufferDesc& desc);
	virtual void                                     releaseVertexBuffer(physx::apex::NxUserRenderVertexBuffer& buffer);

	virtual physx::apex::NxUserRenderIndexBuffer*    createIndexBuffer(const physx::apex::NxUserRenderIndexBufferDesc& desc);
	virtual void                                     releaseIndexBuffer(physx::apex::NxUserRenderIndexBuffer& buffer);

	virtual physx::apex::NxUserRenderBoneBuffer*     createBoneBuffer(const physx::apex::NxUserRenderBoneBufferDesc& desc);
	virtual void                                     releaseBoneBuffer(physx::apex::NxUserRenderBoneBuffer& buffer);

	virtual physx::apex::NxUserRenderInstanceBuffer* createInstanceBuffer(const physx::apex::NxUserRenderInstanceBufferDesc& desc);
	virtual void                                     releaseInstanceBuffer(physx::apex::NxUserRenderInstanceBuffer& buffer);

	virtual physx::apex::NxUserRenderSpriteBuffer*   createSpriteBuffer(const physx::apex::NxUserRenderSpriteBufferDesc& desc);
	virtual void                                     releaseSpriteBuffer(physx::apex::NxUserRenderSpriteBuffer& buffer);

	virtual physx::apex::NxUserRenderSurfaceBuffer*	 createSurfaceBuffer( const physx::apex::NxUserRenderSurfaceBufferDesc &desc );
	virtual void									 releaseSurfaceBuffer( physx::apex::NxUserRenderSurfaceBuffer &buffer );

	virtual physx::apex::NxUserRenderResource*       createResource(const physx::apex::NxUserRenderResourceDesc& desc);

	virtual void                                     releaseResource(physx::apex::NxUserRenderResource& resource);

	virtual physx::PxU32                             getMaxBonesForMaterial(void* material);

	virtual bool									getSpriteLayoutData(physx::PxU32 spriteCount, 
																		physx::PxU32 spriteSemanticsBitmap, 
																		physx::apex::NxUserRenderSpriteBufferDesc* bufferDesc);
	virtual bool									getInstanceLayoutData(physx::PxU32 particleCount, 
																		physx::PxU32 particleSemanticsBitmap, 
																		physx::apex::NxUserRenderInstanceBufferDesc* bufferDesc);
protected:

	struct Child
	{
		Child(physx::apex::NxUserRenderResourceManager* _rrm, bool destroy) : rrm(_rrm), destroyRrm(destroy) {}

		physx::apex::NxUserRenderResourceManager* rrm;
		bool destroyRrm;
	};

	std::vector<Child> mChildren;
};


class MultiClientUserRenderer : public physx::apex::NxUserRenderer
{
public:
	MultiClientUserRenderer() {}
	virtual ~MultiClientUserRenderer() {}

	void addChild(physx::apex::NxUserRenderer* child);
	virtual void renderResource(const physx::apex::NxApexRenderContext& context);

protected:
	std::vector<physx::apex::NxUserRenderer*> mChildren;
};


#endif // MULTI_CLIENT_RENDER_RESOURCE_MANAGER_H
