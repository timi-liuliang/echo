/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef SAMPLE_APEX_RENDERER_H
#define SAMPLE_APEX_RENDERER_H

#include <NxUserRenderer.h>
#include <NxUserRenderResourceManager.h>


#pragma warning(push)
#pragma warning(disable:4512)

class NxUserRenderSpriteTextureDesc;

namespace SampleRenderer
{
class Renderer;
}

namespace SampleFramework
{
class SampleMaterialAsset;
}


class SampleApexRenderResourceManager : public physx::apex::NxUserRenderResourceManager
{
public:
	/* We either store particle position/color/transform in the texture 
	   and fetch it in the vertex shader OR we store them in the VBO */
	enum ParticleRenderingMechanism {
		VERTEX_TEXTURE_FETCH,
		VERTEX_BUFFER_OBJECT
	};

	SampleApexRenderResourceManager(SampleRenderer::Renderer& renderer);
	virtual								~SampleApexRenderResourceManager(void);

public:
	virtual physx::apex::NxUserRenderVertexBuffer*		createVertexBuffer(const physx::apex::NxUserRenderVertexBufferDesc& desc);
	virtual void										releaseVertexBuffer(physx::apex::NxUserRenderVertexBuffer& buffer);

	virtual physx::apex::NxUserRenderIndexBuffer*		createIndexBuffer(const physx::apex::NxUserRenderIndexBufferDesc& desc);
	virtual void										releaseIndexBuffer(physx::apex::NxUserRenderIndexBuffer& buffer);

	virtual physx::apex::NxUserRenderSurfaceBuffer*		createSurfaceBuffer(const physx::apex::NxUserRenderSurfaceBufferDesc& desc);
	virtual void										releaseSurfaceBuffer(physx::apex::NxUserRenderSurfaceBuffer& buffer);

	virtual physx::apex::NxUserRenderBoneBuffer*		createBoneBuffer(const physx::apex::NxUserRenderBoneBufferDesc& desc);
	virtual void										releaseBoneBuffer(physx::apex::NxUserRenderBoneBuffer& buffer);

	virtual physx::apex::NxUserRenderInstanceBuffer*	createInstanceBuffer(const physx::apex::NxUserRenderInstanceBufferDesc& desc);
	virtual void										releaseInstanceBuffer(physx::apex::NxUserRenderInstanceBuffer& buffer);

	virtual physx::apex::NxUserRenderSpriteBuffer*		createSpriteBuffer(const physx::apex::NxUserRenderSpriteBufferDesc& desc);
	virtual void										releaseSpriteBuffer(physx::apex::NxUserRenderSpriteBuffer& buffer);

	virtual physx::apex::NxUserRenderResource*			createResource(const physx::apex::NxUserRenderResourceDesc& desc);
	virtual void										releaseResource(physx::apex::NxUserRenderResource& resource);

	virtual physx::PxU32								getMaxBonesForMaterial(void* material);

	virtual bool										getSpriteLayoutData(physx::PxU32 spriteCount, 
																			physx::PxU32 spriteSemanticsBitmap, 
																			physx::apex::NxUserRenderSpriteBufferDesc* vertexDescArray);

	virtual bool										getInstanceLayoutData(physx::PxU32 spriteCount, 
																				physx::PxU32 particleSemanticsBitmap, 
																				physx::apex::NxUserRenderInstanceBufferDesc* instanceDescArray);

	// change the material of a render resource
	void												setMaterial(physx::apex::NxUserRenderResource& resource, void* material);

	void												setParticleRenderingMechanism(ParticleRenderingMechanism m) { m_particleRenderingMechanism = m;	}
protected:
	SampleRenderer::Renderer&	m_renderer;
	ParticleRenderingMechanism  m_particleRenderingMechanism;
	physx::PxU32				m_numVertexBuffers;
	physx::PxU32				m_numIndexBuffers;
	physx::PxU32				m_numSurfaceBuffers; //?
	physx::PxU32				m_numBoneBuffers;
	physx::PxU32				m_numInstanceBuffers;
	physx::PxU32				m_numResources;
};

class SampleApexRenderer : public physx::apex::NxUserRenderer
{
public:
	SampleApexRenderer() : mForceWireframe(false), mOverrideMaterial(NULL) {}
	virtual void renderResource(const physx::apex::NxApexRenderContext& context);

	bool mForceWireframe;
	SampleFramework::SampleMaterialAsset* mOverrideMaterial;
};

#pragma warning(pop)

#endif
