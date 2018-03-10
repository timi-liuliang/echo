/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "MultiClientRenderResourceManager.h"

#include "NxUserRenderVertexBuffer.h"
#include "NxUserRenderIndexBuffer.h"
#include "NxUserRenderBoneBuffer.h"
#include "NxUserRenderInstanceBuffer.h"
#include "NxUserRenderSpriteBuffer.h"
#include "NxUserRenderSurfaceBuffer.h"

#include "NxUserRenderResource.h"
#include "NxUserRenderResourceDesc.h"

#include "NxApexRenderContext.h"


#include <assert.h>
#include <algorithm>	// for std::min


MultiClientRenderResourceManager::MultiClientRenderResourceManager()
{

}



MultiClientRenderResourceManager::~MultiClientRenderResourceManager()
{
	for (size_t i = 0; i < mChildren.size(); i++)
	{
		if (mChildren[i].destroyRrm)
		{
			delete mChildren[i].rrm;
		}
		mChildren[i].rrm = NULL;
	}
}



void MultiClientRenderResourceManager::addChild(physx::apex::NxUserRenderResourceManager* rrm, bool destroyAutomatic)
{
	for (size_t i = 0; i < mChildren.size(); i++)
	{
		if (mChildren[i].rrm == rrm)
		{
			return;
		}
	}

	mChildren.push_back(Child(rrm, destroyAutomatic));
}

bool MultiClientRenderResourceManager::getSpriteLayoutData(physx::PxU32 spriteCount, physx::PxU32 spriteSemanticsBitmap, physx::apex::NxUserRenderSpriteBufferDesc* bufferDesc)
{
	PX_UNUSED(spriteCount);
	PX_UNUSED(spriteSemanticsBitmap);
	PX_UNUSED(bufferDesc);
	return false;
}

bool MultiClientRenderResourceManager::getInstanceLayoutData(physx::PxU32 particleCount, physx::PxU32 particleSemanticsBitmap, physx::apex::NxUserRenderInstanceBufferDesc* bufferDesc)
{
	PX_UNUSED(particleCount);
	PX_UNUSED(particleSemanticsBitmap);
	PX_UNUSED(bufferDesc);
	return false;
}

template<typename T>
class MultiClientBuffer
{
public:
	MultiClientBuffer() {}
	~MultiClientBuffer() {}


	void addChild(T* vb)
	{
		mChildren.push_back(vb);
	}

	T* getChild(size_t index)
	{
		assert(index < mChildren.size());
		return mChildren[index];
	}

protected:
	std::vector<T*> mChildren;
};




class MultiClientVertexBuffer : public physx::apex::NxUserRenderVertexBuffer, public MultiClientBuffer<physx::apex::NxUserRenderVertexBuffer>
{
public:
	MultiClientVertexBuffer() {}
	~MultiClientVertexBuffer() {}

	virtual void writeBuffer(const physx::apex::NxApexRenderVertexBufferData& data, unsigned int firstVertex, unsigned int numVertices)
	{
		for (size_t i = 0; i < mChildren.size(); i++)
		{
			mChildren[i]->writeBuffer(data, firstVertex, numVertices);
		}
	}

};


physx::apex::NxUserRenderVertexBuffer* MultiClientRenderResourceManager::createVertexBuffer(const physx::apex::NxUserRenderVertexBufferDesc& desc)
{
	MultiClientVertexBuffer* vb = new MultiClientVertexBuffer();

	for (size_t i = 0; i < mChildren.size(); i++)
	{
		vb->addChild(mChildren[i].rrm->createVertexBuffer(desc));
	}

	return vb;
}

void MultiClientRenderResourceManager::releaseVertexBuffer(physx::apex::NxUserRenderVertexBuffer& buffer)
{
	MultiClientVertexBuffer* vb = static_cast<MultiClientVertexBuffer*>(&buffer);

	for (size_t i = 0; i < mChildren.size(); i++)
	{
		physx::apex::NxUserRenderVertexBuffer* childVb = vb->getChild(i);
		mChildren[i].rrm->releaseVertexBuffer(*childVb);
	}

	delete vb;
}






class MultiClientIndexBuffer : public physx::apex::NxUserRenderIndexBuffer, public MultiClientBuffer<physx::apex::NxUserRenderIndexBuffer>
{
public:
	MultiClientIndexBuffer() {}
	~MultiClientIndexBuffer() {}

	virtual void writeBuffer(const void* srcData, unsigned int srcStride, unsigned int firstDestElement, unsigned int numElements)
	{
		for (size_t i = 0; i < mChildren.size(); i++)
		{
			mChildren[i]->writeBuffer(srcData, srcStride, firstDestElement, numElements);
		}
	}
};


physx::apex::NxUserRenderIndexBuffer* MultiClientRenderResourceManager::createIndexBuffer(const physx::apex::NxUserRenderIndexBufferDesc& desc)
{
	MultiClientIndexBuffer* ib = new MultiClientIndexBuffer();

	for (size_t i = 0; i < mChildren.size(); i++)
	{
		ib->addChild(mChildren[i].rrm->createIndexBuffer(desc));
	}

	return ib;
}

void MultiClientRenderResourceManager::releaseIndexBuffer(physx::apex::NxUserRenderIndexBuffer& buffer)
{
	MultiClientIndexBuffer* ib = static_cast<MultiClientIndexBuffer*>(&buffer);

	for (size_t i = 0; i < mChildren.size(); i++)
	{
		physx::apex::NxUserRenderIndexBuffer* childIb = ib->getChild(i);
		mChildren[i].rrm->releaseIndexBuffer(*childIb);
	}

	delete ib;
}








class MultiClientBoneBuffer : public physx::apex::NxUserRenderBoneBuffer, public MultiClientBuffer<physx::apex::NxUserRenderBoneBuffer>
{
public:
	MultiClientBoneBuffer() {}
	~MultiClientBoneBuffer() {}

	virtual void writeBuffer(const physx::apex::NxApexRenderBoneBufferData& data, unsigned int firstBone, unsigned int numBones)
	{
		for (size_t i = 0; i < mChildren.size(); i++)
		{
			mChildren[i]->writeBuffer(data, firstBone, numBones);
		}
	}
};



physx::apex::NxUserRenderBoneBuffer* MultiClientRenderResourceManager::createBoneBuffer(const physx::apex::NxUserRenderBoneBufferDesc& desc)
{
	MultiClientBoneBuffer* bb = new MultiClientBoneBuffer();

	for (size_t i = 0; i < mChildren.size(); i++)
	{
		bb->addChild(mChildren[i].rrm->createBoneBuffer(desc));
	}

	return bb;
}



void MultiClientRenderResourceManager::releaseBoneBuffer(physx::apex::NxUserRenderBoneBuffer& buffer)
{
	MultiClientBoneBuffer* bb = static_cast<MultiClientBoneBuffer*>(&buffer);

	for (size_t i = 0; i < mChildren.size(); i++)
	{
		physx::apex::NxUserRenderBoneBuffer* childBb = bb->getChild(i);
		mChildren[i].rrm->releaseBoneBuffer(*childBb);
	}

	delete bb;
}





class MultiClientInstanceBuffer : public physx::apex::NxUserRenderInstanceBuffer, public MultiClientBuffer<physx::apex::NxUserRenderInstanceBuffer>
{
public:
	MultiClientInstanceBuffer() {}
	~MultiClientInstanceBuffer() {}

	virtual void writeBuffer(const void* data, unsigned int firstInstance, unsigned int numInstances)
	{
		for (size_t i = 0; i < mChildren.size(); i++)
		{
			mChildren[i]->writeBuffer(data, firstInstance, numInstances);
		}
	}
};




physx::apex::NxUserRenderInstanceBuffer* MultiClientRenderResourceManager::createInstanceBuffer(const physx::apex::NxUserRenderInstanceBufferDesc& desc)
{
	MultiClientInstanceBuffer* ib = new MultiClientInstanceBuffer();

	for (size_t i = 0; i < mChildren.size(); i++)
	{
		ib->addChild(mChildren[i].rrm->createInstanceBuffer(desc));
	}

	return ib;
}



void MultiClientRenderResourceManager::releaseInstanceBuffer(physx::apex::NxUserRenderInstanceBuffer& buffer)
{
	MultiClientInstanceBuffer* ib = static_cast<MultiClientInstanceBuffer*>(&buffer);

	for (size_t i = 0; i < mChildren.size(); i++)
	{
		physx::apex::NxUserRenderInstanceBuffer* childIb = ib->getChild(i);
		mChildren[i].rrm->releaseInstanceBuffer(*childIb);
	}

	delete ib;
}





class MultiClientSpriteBuffer : public physx::apex::NxUserRenderSpriteBuffer, public MultiClientBuffer<physx::apex::NxUserRenderSpriteBuffer>
{
public:
	MultiClientSpriteBuffer() {}
	~MultiClientSpriteBuffer() {}

	virtual void writeBuffer(const void* data, unsigned int firstSprite, unsigned int numSprites)
	{
		for (size_t i = 0; i < mChildren.size(); i++)
		{
			mChildren[i]->writeBuffer(data, firstSprite, numSprites);
		}
	}
};



physx::apex::NxUserRenderSpriteBuffer* MultiClientRenderResourceManager::createSpriteBuffer(const physx::apex::NxUserRenderSpriteBufferDesc& desc)
{
	MultiClientSpriteBuffer* sb = new MultiClientSpriteBuffer();

	for (size_t i = 0; i < mChildren.size(); i++)
	{
		sb->addChild(mChildren[i].rrm->createSpriteBuffer(desc));
	}

	return sb;
}



void MultiClientRenderResourceManager::releaseSpriteBuffer(physx::apex::NxUserRenderSpriteBuffer& buffer)
{
	MultiClientSpriteBuffer* sb = static_cast<MultiClientSpriteBuffer*>(&buffer);

	for (size_t i = 0; i < mChildren.size(); i++)
	{
		physx::apex::NxUserRenderSpriteBuffer* childSb = sb->getChild(i);
		mChildren[i].rrm->releaseSpriteBuffer(*childSb);
	}

	delete sb;
}


class MultiClientSurfaceBuffer : public physx::apex::NxUserRenderSurfaceBuffer, public MultiClientBuffer<physx::apex::NxUserRenderSurfaceBuffer>
{
public:
	MultiClientSurfaceBuffer() {}
	~MultiClientSurfaceBuffer() {}

	virtual void writeBuffer(const void* /*srcData*/, 
							physx::PxU32 /*srcPitch*/, 
							physx::PxU32 /*srcHeight*/, 
							physx::PxU32 /*dstX*/, 
							physx::PxU32 /*dstY*/, 
							physx::PxU32 /*dstZ*/, 
							physx::PxU32 /*width*/, 
							physx::PxU32 /*height*/, 
							physx::PxU32 /*depth*/)
	{
		//for (size_t i = 0; i < mChildren.size(); i++)
		//{
		//	mChildren[i]->writeBuffer(data, firstSprite, numSprites);
		//}
	}
};

physx::apex::NxUserRenderSurfaceBuffer*	 MultiClientRenderResourceManager::createSurfaceBuffer( const physx::apex::NxUserRenderSurfaceBufferDesc &desc )
{
	MultiClientSurfaceBuffer* sb = new MultiClientSurfaceBuffer();

	for (size_t i = 0; i < mChildren.size(); i++)
	{
		sb->addChild(mChildren[i].rrm->createSurfaceBuffer(desc));
	}

	return sb;
}



void MultiClientRenderResourceManager::releaseSurfaceBuffer( physx::apex::NxUserRenderSurfaceBuffer &buffer )
{
	MultiClientSurfaceBuffer* sb = static_cast<MultiClientSurfaceBuffer*>(&buffer);

	for (size_t i = 0; i < mChildren.size(); i++)
	{
		physx::apex::NxUserRenderSurfaceBuffer* childSb = sb->getChild(i);
		mChildren[i].rrm->releaseSurfaceBuffer(*childSb);
	}

	delete sb;
}







class MultiClientRenderResource : public physx::apex::NxUserRenderResource
{
public:
	MultiClientRenderResource(const physx::apex::NxUserRenderResourceDesc& desc) : mDescriptor(desc)
	{
		assert(desc.numVertexBuffers > 0);

		mVertexBufferOriginal.resize(desc.numVertexBuffers);
		for (size_t i = 0; i < mVertexBufferOriginal.size(); i++)
		{
			mVertexBufferOriginal[i] = desc.vertexBuffers[i];
		}

		mDescriptor.vertexBuffers = &mVertexBufferOriginal[0];

	}

	~MultiClientRenderResource()
	{

	}



	void addChild(physx::apex::NxUserRenderResourceManager* rrm)
	{
		physx::apex::NxUserRenderResourceDesc newDesc(mDescriptor);

		std::vector<physx::apex::NxUserRenderVertexBuffer*> childVertexBuffers(mVertexBufferOriginal.size());

		size_t nextChild = mChildren.size();

		for (size_t i = 0; i < mVertexBufferOriginal.size(); i++)
		{
			MultiClientVertexBuffer* vb = static_cast<MultiClientVertexBuffer*>(mVertexBufferOriginal[i]);
			childVertexBuffers[i] = vb->getChild(nextChild);
		}

		newDesc.vertexBuffers = &childVertexBuffers[0];

		if (mDescriptor.indexBuffer != NULL)
		{
			newDesc.indexBuffer = static_cast<MultiClientIndexBuffer*>(mDescriptor.indexBuffer)->getChild(nextChild);
		}

		if (mDescriptor.boneBuffer != NULL)
		{
			newDesc.boneBuffer = static_cast<MultiClientBoneBuffer*>(mDescriptor.boneBuffer)->getChild(nextChild);
		}

		if (mDescriptor.spriteBuffer != NULL)
		{
			newDesc.spriteBuffer = static_cast<MultiClientSpriteBuffer*>(mDescriptor.spriteBuffer)->getChild(nextChild);
		}

		if (rrm != NULL)
		{
			mChildren.push_back(rrm->createResource(newDesc));
		}
	}


	physx::apex::NxUserRenderResource* getChild(size_t index)
	{
		assert(index < mChildren.size());
		return mChildren[index];
	}



	void setVertexBufferRange(unsigned int firstVertex, unsigned int numVerts)
	{
		for (size_t i = 0; i < mChildren.size(); i++)
		{
			mChildren[i]->setVertexBufferRange(firstVertex, numVerts);
		}

		mDescriptor.firstVertex = firstVertex;
		mDescriptor.numVerts = numVerts;
	}



	void setIndexBufferRange(unsigned int firstIndex, unsigned int numIndices)
	{
		for (size_t i = 0; i < mChildren.size(); i++)
		{
			mChildren[i]->setIndexBufferRange(firstIndex, numIndices);
		}

		mDescriptor.firstIndex = firstIndex;
		mDescriptor.numIndices = numIndices;
	}



	void setBoneBufferRange(unsigned int firstBone, unsigned int numBones)
	{
		for (size_t i = 0; i < mChildren.size(); i++)
		{
			mChildren[i]->setBoneBufferRange(firstBone, numBones);
		}

		mDescriptor.firstBone = firstBone;
		mDescriptor.numBones = numBones;
	}



	void setInstanceBufferRange(unsigned int firstInstance, unsigned int numInstances)
	{
		for (size_t i = 0; i < mChildren.size(); i++)
		{
			mChildren[i]->setInstanceBufferRange(firstInstance, numInstances);
		}

		mDescriptor.firstInstance = firstInstance;
		mDescriptor.numInstances = numInstances;
	}



	void setSpriteBufferRange(unsigned int firstSprite, unsigned int numSprites)
	{
		for (size_t i = 0; i < mChildren.size(); i++)
		{
			mChildren[i]->setSpriteBufferRange(firstSprite, numSprites);
		}

		mDescriptor.firstSprite = firstSprite;
		mDescriptor.numSprites = numSprites;
	}




	void setMaterial(void* material)
	{
		for (size_t i = 0; i < mChildren.size(); i++)
		{
			mChildren[i]->setMaterial(material);
		}

		mDescriptor.material = material;
	}




	unsigned int getNbVertexBuffers() const
	{
		return mDescriptor.numVertexBuffers;
	}



	physx::apex::NxUserRenderVertexBuffer* getVertexBuffer(unsigned int index) const
	{
		return mDescriptor.vertexBuffers[index];
	}



	physx::apex::NxUserRenderIndexBuffer* getIndexBuffer() const
	{
		return mDescriptor.indexBuffer;
	}




	physx::apex::NxUserRenderBoneBuffer* getBoneBuffer()	const
	{
		return mDescriptor.boneBuffer;
	}



	physx::apex::NxUserRenderInstanceBuffer* getInstanceBuffer()	const
	{
		return mDescriptor.instanceBuffer;
	}



	physx::apex::NxUserRenderSpriteBuffer* getSpriteBuffer() const
	{
		return mDescriptor.spriteBuffer;
	}

protected:
	std::vector<physx::apex::NxUserRenderVertexBuffer*> mVertexBufferOriginal;
	std::vector<physx::apex::NxUserRenderResource*> mChildren;

	physx::apex::NxUserRenderResourceDesc mDescriptor;
};




physx::apex::NxUserRenderResource* MultiClientRenderResourceManager::createResource(const physx::apex::NxUserRenderResourceDesc& desc)
{
	MultiClientRenderResource* rr = new MultiClientRenderResource(desc);

	for (size_t i = 0; i < mChildren.size(); i++)
	{
		rr->addChild(mChildren[i].rrm);
	}

	return rr;
}



void MultiClientRenderResourceManager::releaseResource(physx::apex::NxUserRenderResource& resource)
{
	MultiClientRenderResource* rr = static_cast<MultiClientRenderResource*>(&resource);

	for (size_t i = 0; i < mChildren.size(); i++)
	{
		mChildren[i].rrm->releaseResource(*rr->getChild(i));
	}

	delete rr;
}



unsigned int MultiClientRenderResourceManager::getMaxBonesForMaterial(void* material)
{
	unsigned int smallestMax = 10000;

	for (size_t i = 0; i < mChildren.size(); i++)
	{
		unsigned int childMax = mChildren[i].rrm->getMaxBonesForMaterial(material);
		if (childMax > 0)
		{
			smallestMax = std::min(smallestMax, childMax);
		}
	}

	return smallestMax;
}



void MultiClientUserRenderer::addChild(physx::apex::NxUserRenderer* child)
{
	mChildren.push_back(child);
}



void MultiClientUserRenderer::renderResource(const physx::apex::NxApexRenderContext& context)
{
	MultiClientRenderResource* rr = static_cast<MultiClientRenderResource*>(context.renderResource);

	for (size_t i = 0; i < mChildren.size(); i++)
	{
		physx::apex::NxApexRenderContext newContext(context);
		newContext.renderResource = rr->getChild(i);
		mChildren[i]->renderResource(newContext);
	}
}
