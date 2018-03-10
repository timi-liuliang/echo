/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include <SampleApexRenderer.h>

#include <SampleApexRenderResources.h>

#if !USE_RENDERER_MATERIAL
#include <SampleMaterialAsset.h>
#endif

#include <NxApexRenderContext.h>
#include <NxUserRenderIndexBufferDesc.h>
#include <NxUserRenderInstanceBuffer.h>
#include <NxUserRenderResourceDesc.h>
#include <NxUserRenderBoneBufferDesc.h>
#include <NxUserRenderSpriteBufferDesc.h>
#include <NxUserRenderSurfaceBufferDesc.h>
#include <NxUserRenderVertexBufferDesc.h>
#include <NxUserRenderSpriteBufferDesc.h>
#include <algorithm>	// for std::min

/**********************************
* SampleApexRenderResourceManager *
**********************************/

SampleApexRenderResourceManager::SampleApexRenderResourceManager(SampleRenderer::Renderer& renderer) :
	m_renderer(renderer), m_particleRenderingMechanism(VERTEX_BUFFER_OBJECT)
{
	m_numVertexBuffers   = 0;
	m_numIndexBuffers    = 0;
	m_numSurfaceBuffers  = 0;
	m_numBoneBuffers     = 0;
	m_numInstanceBuffers = 0;
	m_numResources       = 0;
}

SampleApexRenderResourceManager::~SampleApexRenderResourceManager(void)
{
	RENDERER_ASSERT(m_numVertexBuffers   == 0, "Not all Vertex Buffers were released prior to Render Resource Manager destruction!");
	RENDERER_ASSERT(m_numIndexBuffers    == 0, "Not all Index Buffers were released prior to Render Resource Manager destruction!");
	RENDERER_ASSERT(m_numSurfaceBuffers  == 0, "Not all Surface Buffers were released prior to Render Resource Manager destruction!");
	RENDERER_ASSERT(m_numBoneBuffers     == 0, "Not all Bone Buffers were released prior to Render Resource Manager destruction!");
	RENDERER_ASSERT(m_numInstanceBuffers == 0, "Not all Instance Buffers were released prior to Render Resource Manager destruction!");
	RENDERER_ASSERT(m_numResources       == 0, "Not all Resources were released prior to Render Resource Manager destruction!");
}

physx::apex::NxUserRenderVertexBuffer* SampleApexRenderResourceManager::createVertexBuffer(const physx::apex::NxUserRenderVertexBufferDesc& desc)
{
	SampleApexRendererVertexBuffer* vb = 0;

	unsigned int numSemantics = 0;
	for (unsigned int i = 0; i < physx::apex::NxRenderVertexSemantic::NUM_SEMANTICS; i++)
	{
		numSemantics += desc.buffersRequest[i] != physx::apex::NxRenderDataFormat::UNSPECIFIED ? 1 : 0;
	}
	PX_ASSERT(desc.isValid());
	if (desc.isValid() && numSemantics > 0)
	{
		vb = new SampleApexRendererVertexBuffer(m_renderer, desc);
		m_numVertexBuffers++;
	}
	return vb;
}

void SampleApexRenderResourceManager::releaseVertexBuffer(physx::apex::NxUserRenderVertexBuffer& buffer)
{
	PX_ASSERT(m_numVertexBuffers > 0);
	m_numVertexBuffers--;
	delete &buffer;
}

physx::apex::NxUserRenderIndexBuffer* SampleApexRenderResourceManager::createIndexBuffer(const physx::apex::NxUserRenderIndexBufferDesc& desc)
{
	SampleApexRendererIndexBuffer* ib = 0;
	PX_ASSERT(desc.isValid());
	if (desc.isValid())
	{
		ib = new SampleApexRendererIndexBuffer(m_renderer, desc);
		m_numIndexBuffers++;
	}
	return ib;
}

void SampleApexRenderResourceManager::releaseIndexBuffer(physx::apex::NxUserRenderIndexBuffer& buffer)
{
	PX_ASSERT(m_numIndexBuffers > 0);
	m_numIndexBuffers--;
	delete &buffer;
}

physx::apex::NxUserRenderSurfaceBuffer* SampleApexRenderResourceManager::createSurfaceBuffer(const physx::apex::NxUserRenderSurfaceBufferDesc& desc)
{
	SampleApexRendererSurfaceBuffer* sb = 0;
	PX_ASSERT(desc.isValid());
	if (desc.isValid())
	{
		sb = new SampleApexRendererSurfaceBuffer(m_renderer, desc);
		m_numSurfaceBuffers++;
	}
	return sb;
}

void SampleApexRenderResourceManager::releaseSurfaceBuffer(physx::apex::NxUserRenderSurfaceBuffer& buffer)
{
	PX_ASSERT(m_numSurfaceBuffers > 0);
	m_numSurfaceBuffers--;
	delete &buffer;
}

physx::apex::NxUserRenderBoneBuffer* SampleApexRenderResourceManager::createBoneBuffer(const physx::apex::NxUserRenderBoneBufferDesc& desc)
{
	SampleApexRendererBoneBuffer* bb = 0;
	PX_ASSERT(desc.isValid());
	if (desc.isValid())
	{
		bb = new SampleApexRendererBoneBuffer(m_renderer, desc);
		m_numBoneBuffers++;
	}
	return bb;
}

void SampleApexRenderResourceManager::releaseBoneBuffer(physx::apex::NxUserRenderBoneBuffer& buffer)
{
	PX_ASSERT(m_numBoneBuffers > 0);
	m_numBoneBuffers--;
	delete &buffer;
}

physx::apex::NxUserRenderInstanceBuffer* SampleApexRenderResourceManager::createInstanceBuffer(const physx::apex::NxUserRenderInstanceBufferDesc& desc)
{
	SampleApexRendererInstanceBuffer* ib = 0;
	PX_ASSERT(desc.isValid());
	if (desc.isValid())
	{
		ib = new SampleApexRendererInstanceBuffer(m_renderer, desc);
		m_numInstanceBuffers++;
	}
	return ib;
}

void SampleApexRenderResourceManager::releaseInstanceBuffer(physx::apex::NxUserRenderInstanceBuffer& buffer)
{
	PX_ASSERT(m_numInstanceBuffers > 0);
	m_numInstanceBuffers--;
	delete &buffer;
}

physx::apex::NxUserRenderSpriteBuffer* SampleApexRenderResourceManager::createSpriteBuffer(const physx::apex::NxUserRenderSpriteBufferDesc& desc)
{
#if USE_RENDER_SPRITE_BUFFER
	SampleApexRendererSpriteBuffer* sb = 0;
	PX_ASSERT(desc.isValid());
	if (desc.isValid())
	{
		// convert SB to VB
		sb = new SampleApexRendererSpriteBuffer(m_renderer, desc);
		m_numVertexBuffers++;
	}
	return sb;
#else
	return NULL;
#endif
}

void  SampleApexRenderResourceManager::releaseSpriteBuffer(physx::apex::NxUserRenderSpriteBuffer& buffer)
{
#if USE_RENDER_SPRITE_BUFFER
	// LRR: for now, just use a VB
	PX_ASSERT(m_numVertexBuffers > 0);
	m_numVertexBuffers--;
	delete &buffer;
#endif
}

physx::apex::NxUserRenderResource* SampleApexRenderResourceManager::createResource(const physx::apex::NxUserRenderResourceDesc& desc)
{
	SampleApexRendererMesh* mesh = 0;
	PX_ASSERT(desc.isValid());
	if (desc.isValid())
	{
		mesh = new SampleApexRendererMesh(m_renderer, desc);
		m_numResources++;
	}
	return mesh;
}

void SampleApexRenderResourceManager::releaseResource(physx::apex::NxUserRenderResource& resource)
{
	PX_ASSERT(m_numResources > 0);
	m_numResources--;
	delete &resource;
}

physx::PxU32 SampleApexRenderResourceManager::getMaxBonesForMaterial(void* material)
{
	if (material != NULL)
	{
		unsigned int maxBones = 0xffffffff;
#if USE_RENDERER_MATERIAL
		// don't yet know if this material even supports bones, but this would be the max...
		maxBones = RENDERER_MAX_BONES;
#else
		SampleFramework::SampleMaterialAsset* materialAsset = static_cast<SampleFramework::SampleMaterialAsset*>(material);
		for (size_t i = 0; i < materialAsset->getNumVertexShaders(); i++)
		{
			unsigned int maxBonesMat = materialAsset->getMaxBones(i);
			if (maxBonesMat > 0)
			{
				maxBones = std::min(maxBones, maxBonesMat);
			}
		}
#endif

		return maxBones != 0xffffffff ? maxBones : 0;
	}
	else
	{
		return 0;
	}
}

bool SampleApexRenderResourceManager::getInstanceLayoutData(physx::PxU32 particleCount, 
																	physx::PxU32 particleSemanticsBitmap, 
																	physx::apex::NxUserRenderInstanceBufferDesc* bufferDesc)
{
	using namespace physx::apex;
	NxRenderDataFormat::Enum positionFormat = NxRenderInstanceLayoutElement::getSemanticFormat(NxRenderInstanceLayoutElement::POSITION_FLOAT3);
	NxRenderDataFormat::Enum rotationFormat = NxRenderInstanceLayoutElement::getSemanticFormat(NxRenderInstanceLayoutElement::ROTATION_SCALE_FLOAT3x3);
	NxRenderDataFormat::Enum velocityFormat = NxRenderInstanceLayoutElement::getSemanticFormat(NxRenderInstanceLayoutElement::VELOCITY_LIFE_FLOAT4);
	const physx::PxU32 positionElementSize = NxRenderDataFormat::getFormatDataSize(positionFormat);
	const physx::PxU32 rotationElementSize = NxRenderDataFormat::getFormatDataSize(rotationFormat);
	const physx::PxU32 velocityElementSize = NxRenderDataFormat::getFormatDataSize(velocityFormat);
	bufferDesc->semanticOffsets[NxRenderInstanceLayoutElement::POSITION_FLOAT3]			= 0;
	bufferDesc->semanticOffsets[NxRenderInstanceLayoutElement::ROTATION_SCALE_FLOAT3x3] = positionElementSize;
	bufferDesc->semanticOffsets[NxRenderInstanceLayoutElement::VELOCITY_LIFE_FLOAT4] = positionElementSize + rotationElementSize;
	physx::PxU32 strideInBytes = positionElementSize + rotationElementSize + velocityElementSize;
	bufferDesc->stride = strideInBytes;
	bufferDesc->maxInstances = particleCount;
	return true;
}

bool SampleApexRenderResourceManager::getSpriteLayoutData(physx::PxU32 spriteCount, 
																	physx::PxU32 spriteSemanticsBitmap, 
																	physx::apex::NxUserRenderSpriteBufferDesc* bufferDesc)
{
	using namespace physx::apex;
	if(m_particleRenderingMechanism == VERTEX_TEXTURE_FETCH)
	{
		const physx::PxU32 TextureCount = 3;

		physx::PxU32 width = (physx::PxU32)physx::PxCeil(physx::PxSqrt((physx::PxF32)spriteCount));
		//make sizeX >= 32 [32 is WARP_SIZE in CUDA]
		width = physx::PxMax(width, 32U);
		//compute the next highest power of 2
		width--;
		width |= width >> 1;
		width |= width >> 2;
		width |= width >> 4;
		width |= width >> 8;
		width |= width >> 16;
		width++;

		physx::PxU32 height = (spriteCount + width - 1) / width;
		bufferDesc->textureCount = TextureCount;
		bufferDesc->textureDescs[0].layout = NxRenderSpriteTextureLayout::POSITION_FLOAT4;
		bufferDesc->textureDescs[1].layout = NxRenderSpriteTextureLayout::SCALE_ORIENT_SUBTEX_FLOAT4;
		bufferDesc->textureDescs[2].layout = NxRenderSpriteTextureLayout::COLOR_FLOAT4;

		for (physx::PxU32 i = 0; i < TextureCount; ++i)
		{
			bufferDesc->textureDescs[i].width = width;
			bufferDesc->textureDescs[i].height = height;

			const physx::PxU32 ElemSize = NxRenderDataFormat::getFormatDataSize( NxRenderSpriteTextureLayout::getLayoutFormat(bufferDesc->textureDescs[i].layout) );
			bufferDesc->textureDescs[i].pitchBytes = ElemSize * bufferDesc->textureDescs[i].width;

			bufferDesc->textureDescs[i].arrayIndex = 0;
			bufferDesc->textureDescs[i].mipLevel = 0;
		}

		bufferDesc->maxSprites = spriteCount;
		return true;
	}
	else if(m_particleRenderingMechanism == VERTEX_BUFFER_OBJECT)
	{
		NxRenderDataFormat::Enum positionFormat = NxRenderSpriteLayoutElement::getSemanticFormat(NxRenderSpriteLayoutElement::POSITION_FLOAT3);
		NxRenderDataFormat::Enum colorFormat = NxRenderSpriteLayoutElement::getSemanticFormat(NxRenderSpriteLayoutElement::COLOR_BGRA8);
		const physx::PxU32 positionElementSize = NxRenderDataFormat::getFormatDataSize(positionFormat);
		const physx::PxU32 colorElementSize = NxRenderDataFormat::getFormatDataSize(colorFormat);
		bufferDesc->semanticOffsets[NxRenderSpriteLayoutElement::POSITION_FLOAT3] = 0;
#if defined(PX_PS3)
		bufferDesc->semanticOffsets[NxRenderSpriteLayoutElement::COLOR_RGBA8] = positionElementSize;
#else
		bufferDesc->semanticOffsets[NxRenderSpriteLayoutElement::COLOR_BGRA8] = positionElementSize;
#endif
		physx::PxU32 strideInBytes = positionElementSize + colorElementSize;
		bufferDesc->stride = strideInBytes;
		bufferDesc->maxSprites = spriteCount;
		bufferDesc->textureCount = 0;
		return true;
	}
	else
	{
		PX_ASSERT(0 && "Select a method to update particle render buffer.");
	}
	return true;
}

void SampleApexRenderResourceManager::setMaterial(physx::apex::NxUserRenderResource& resource, void* material)
{
	static_cast<SampleApexRendererMesh&>(resource).setMaterial(material);
}


/*********************
* SampleApexRenderer *
*********************/

void SampleApexRenderer::renderResource(const physx::apex::NxApexRenderContext& context)
{
	if (context.renderResource)
	{
		static_cast<SampleApexRendererMesh*>(context.renderResource)->render(context, mForceWireframe, mOverrideMaterial);
	}
}
