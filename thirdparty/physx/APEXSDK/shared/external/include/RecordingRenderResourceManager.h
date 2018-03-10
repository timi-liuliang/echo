/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef RECORDING_RENDER_RESOURCE_MANAGER_H
#define RECORDING_RENDER_RESOURCE_MANAGER_H

#include "NxUserRenderResourceManager.h"
#include "NxUserRenderer.h"

#include <string>

namespace physx
{
namespace apex
{
class NxApexRenderVertexBufferData;
//class NxApexRenderBoneBufferData; // not possible, d'oh
}
}

#include "NxUserRenderBoneBuffer.h"


class RecordingRenderResourceManager : public physx::apex::NxUserRenderResourceManager
{
public:

	class RecorderInterface
	{
	public:
		virtual ~RecorderInterface() {}

		virtual void createVertexBuffer(unsigned int id, const physx::apex::NxUserRenderVertexBufferDesc& desc) = 0;
		virtual void writeVertexBuffer(unsigned int id, const physx::apex::NxApexRenderVertexBufferData& data, unsigned int firstVertex, unsigned int numVertices) = 0;
		virtual void releaseVertexBuffer(unsigned int id) = 0;

		virtual void createIndexBuffer(unsigned int id, const physx::apex::NxUserRenderIndexBufferDesc& desc) = 0;
		virtual void writeIndexBuffer(unsigned int id, const void* srcData, physx::PxU32 srcStride, unsigned int firstDestElement, unsigned int numElements, physx::apex::NxRenderDataFormat::Enum format) = 0;
		virtual void releaseIndexBuffer(unsigned int id) = 0;

		virtual void createBoneBuffer(unsigned int id, const physx::apex::NxUserRenderBoneBufferDesc& desc) = 0;
		virtual void writeBoneBuffer(unsigned int id, const physx::apex::NxApexRenderBoneBufferData& data, unsigned int firstBone, unsigned int numBones) = 0;
		virtual void releaseBoneBuffer(unsigned int id) = 0;

		virtual void createResource(unsigned int id, const physx::apex::NxUserRenderResourceDesc& desc) = 0;
		virtual void renderResource(unsigned int id, const physx::apex::NxUserRenderResourceDesc& desc) = 0;
		virtual void releaseResource(unsigned int id) = 0;

		virtual void setMaxBonesForMaterial(void* material, unsigned int maxBones) = 0;

	};

	RecordingRenderResourceManager(physx::apex::NxUserRenderResourceManager* child, bool ownsChild, RecorderInterface* recorder);
	~RecordingRenderResourceManager();


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

	virtual physx::apex::NxUserRenderSurfaceBuffer*  createSurfaceBuffer(const physx::apex::NxUserRenderSurfaceBufferDesc& desc);
	virtual void									 releaseSurfaceBuffer(physx::apex::NxUserRenderSurfaceBuffer& buffer);

	virtual physx::apex::NxUserRenderResource*       createResource(const physx::apex::NxUserRenderResourceDesc& desc);

	virtual void                                     releaseResource(physx::apex::NxUserRenderResource& resource);

	virtual physx::PxU32                             getMaxBonesForMaterial(void* material);

	/** \brief Get the sprite layout data */
	virtual bool getSpriteLayoutData(physx::PxU32 spriteCount, physx::PxU32 spriteSemanticsBitmap, physx::apex::NxUserRenderSpriteBufferDesc* textureDescArray)
	{
		PX_ALWAYS_ASSERT(); // TODO TODO TODO : This needs to be implemented.
		PX_UNUSED(spriteCount);
		PX_UNUSED(spriteSemanticsBitmap);
		PX_UNUSED(textureDescArray);
		return false;
	}

	/** \brief Get the instance layout data */
	virtual bool getInstanceLayoutData(physx::PxU32 spriteCount, physx::PxU32 spriteSemanticsBitmap, physx::apex::NxUserRenderInstanceBufferDesc* instanceDescArray)
	{
		PX_ALWAYS_ASSERT(); // TODO TODO TODO : This needs to be implemented.
		PX_UNUSED(spriteCount);
		PX_UNUSED(spriteSemanticsBitmap);
		PX_UNUSED(instanceDescArray);
		return false;
	}
protected:

	physx::apex::NxUserRenderResourceManager* mChild;
	bool mOwnsChild;

	RecorderInterface* mRecorder;
};


class RecordingRenderer : public physx::apex::NxUserRenderer
{
public:
	RecordingRenderer(physx::apex::NxUserRenderer* child, RecordingRenderResourceManager::RecorderInterface* recorder);
	virtual ~RecordingRenderer();

	virtual void renderResource(const physx::apex::NxApexRenderContext& context);

protected:
	physx::apex::NxUserRenderer* mChild;
	RecordingRenderResourceManager::RecorderInterface* mRecorder;
};


class FileRecorder : public RecordingRenderResourceManager::RecorderInterface
{
public:
	FileRecorder(const char* filename);
	~FileRecorder();

	virtual void createVertexBuffer(unsigned int id, const physx::apex::NxUserRenderVertexBufferDesc& desc);
	virtual void writeVertexBuffer(unsigned int id, const physx::apex::NxApexRenderVertexBufferData& data, unsigned int firstVertex, unsigned int numVertices);
	virtual void releaseVertexBuffer(unsigned int id);

	virtual void createIndexBuffer(unsigned int id, const physx::apex::NxUserRenderIndexBufferDesc& desc);
	virtual void writeIndexBuffer(unsigned int id, const void* srcData, physx::PxU32 srcStride, unsigned int firstDestElement, unsigned int numElements, physx::apex::NxRenderDataFormat::Enum format);
	virtual void releaseIndexBuffer(unsigned int id);

	virtual void createBoneBuffer(unsigned int id, const physx::apex::NxUserRenderBoneBufferDesc& desc);
	virtual void writeBoneBuffer(unsigned int id, const physx::apex::NxApexRenderBoneBufferData& data, unsigned int firstBone, unsigned int numBones);
	virtual void releaseBoneBuffer(unsigned int id);

	virtual void createResource(unsigned int id, const physx::apex::NxUserRenderResourceDesc& desc);
	virtual void renderResource(unsigned int id, const physx::apex::NxUserRenderResourceDesc& desc);
	virtual void releaseResource(unsigned int id);

	virtual void setMaxBonesForMaterial(void* material, unsigned int maxBones);

protected:
	void writeElem(const char* name, unsigned int value);

	void writeBufferData(const void* data, unsigned int stride, unsigned int numElements, physx::apex::NxRenderDataFormat::Enum format);
	void writeBufferDataFloat(const void* data, unsigned int stride, unsigned int numElements, unsigned int numFloatsPerDataSet);
	void writeBufferDataShort(const void* data, unsigned int stride, unsigned int numElements, unsigned int numFloatsPerDataSet);
	void writeBufferDataLong(const void* data, unsigned int stride, unsigned int numElements, unsigned int numFloatsPerDataSet);

	FILE* mOutputFile;
};


#endif // RECORDING_RENDER_RESOURCE_MANAGER_H
