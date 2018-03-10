/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "RecordingRenderResourceManager.h"

#include <assert.h>
#include <time.h>

#include <vector>

#include "NxUserRenderVertexBuffer.h"
#include "NxUserRenderResource.h"
#include "NxUserRenderResourceDesc.h"
#include "NxUserRenderIndexBuffer.h"
#include "NxUserRenderIndexBufferDesc.h"
#include "NxUserRenderBoneBuffer.h"
#include "NxUserRenderBoneBufferDesc.h"

#include "NxApexRenderContext.h"

#define BREAK_ON_UNIMPLEMENTED 1

#if BREAK_ON_UNIMPLEMENTED
#define UNIMPLEMENTED assert(0)
#else
#define UNIMPLEMENTED /*void&*/
#endif

#ifdef PX_WINDOWS
#define NOMINMAX
#include "windows.h"
#endif

RecordingRenderResourceManager::RecordingRenderResourceManager(physx::apex::NxUserRenderResourceManager* child, bool /*ownsChild*/, RecorderInterface* recorder) : mChild(child), mRecorder(recorder)
{
}



RecordingRenderResourceManager::~RecordingRenderResourceManager()
{
	if (mOwnsChild)
	{
		delete mChild;
	}
	mChild = NULL;

	if (mRecorder != NULL)
	{
		delete mRecorder;
		mRecorder = NULL;
	}
}


class RecordingVertexBuffer : public physx::apex::NxUserRenderVertexBuffer
{
public:
	RecordingVertexBuffer(const physx::apex::NxUserRenderVertexBufferDesc& desc, physx::apex::NxUserRenderVertexBuffer* child, RecordingRenderResourceManager::RecorderInterface* recorder) :
		mDescriptor(desc), mChild(child), mBufferId(0), mRecorder(recorder)
	{
		mBufferId = vertexBufferId++;

		if (mRecorder != NULL)
		{
			mRecorder->createVertexBuffer(mBufferId, desc);
		}
	}

	~RecordingVertexBuffer()
	{
		if (mRecorder != NULL)
		{
			mRecorder->releaseVertexBuffer(mBufferId);
		}
	}

	virtual void writeBuffer(const physx::apex::NxApexRenderVertexBufferData& data, unsigned int firstVertex, unsigned int numVertices)
	{
		if (mChild != NULL)
		{
			mChild->writeBuffer(data, firstVertex, numVertices);
		}

		if (mRecorder != NULL)
		{
			mRecorder->writeVertexBuffer(mBufferId, data, firstVertex, numVertices);
		}
	}

	physx::apex::NxUserRenderVertexBuffer* getChild()
	{
		return mChild;
	}

protected:
	physx::apex::NxUserRenderVertexBufferDesc mDescriptor;
	physx::apex::NxUserRenderVertexBuffer* mChild;

	static unsigned int vertexBufferId;
	unsigned int mBufferId;

	RecordingRenderResourceManager::RecorderInterface* mRecorder;
};

unsigned int RecordingVertexBuffer::vertexBufferId = 0;


physx::apex::NxUserRenderVertexBuffer* RecordingRenderResourceManager::createVertexBuffer(const physx::apex::NxUserRenderVertexBufferDesc& desc)
{
	physx::apex::NxUserRenderVertexBuffer* child = NULL;
	if (mChild != NULL)
	{
		child = mChild->createVertexBuffer(desc);
	}

	return new RecordingVertexBuffer(desc, child, mRecorder);
}



void RecordingRenderResourceManager::releaseVertexBuffer(physx::apex::NxUserRenderVertexBuffer& buffer)
{
	if (mChild != NULL)
	{
		mChild->releaseVertexBuffer(*reinterpret_cast<RecordingVertexBuffer&>(buffer).getChild());
	}

	delete &buffer;
}



class RecordingIndexBuffer : public physx::apex::NxUserRenderIndexBuffer
{
public:
	RecordingIndexBuffer(const physx::apex::NxUserRenderIndexBufferDesc& desc, physx::apex::NxUserRenderIndexBuffer* child, RecordingRenderResourceManager::RecorderInterface* recorder) :
		mDescriptor(desc), mChild(child), mBufferId(0), mRecorder(recorder)
	{
		mBufferId = indexBufferId++;

		if (mRecorder != NULL)
		{
			mRecorder->createIndexBuffer(mBufferId, desc);
		}
	}

	~RecordingIndexBuffer()
	{
		if (mRecorder != NULL)
		{
			mRecorder->releaseIndexBuffer(mBufferId);
		}
	}

	virtual void writeBuffer(const void* srcData, physx::PxU32 srcStride, unsigned int firstDestElement, unsigned int numElements)
	{
		if (mChild != NULL)
		{
			mChild->writeBuffer(srcData, srcStride, firstDestElement, numElements);
		}

		if (mRecorder != NULL)
		{
			mRecorder->writeIndexBuffer(mBufferId, srcData, srcStride, firstDestElement, numElements, mDescriptor.format);
		}
	}

	physx::apex::NxUserRenderIndexBuffer* getChild()
	{
		return mChild;
	}

protected:
	physx::apex::NxUserRenderIndexBufferDesc mDescriptor;
	physx::apex::NxUserRenderIndexBuffer* mChild;

	static unsigned int indexBufferId;
	unsigned int mBufferId;

	RecordingRenderResourceManager::RecorderInterface* mRecorder;
};

unsigned int RecordingIndexBuffer::indexBufferId = 0;


physx::apex::NxUserRenderIndexBuffer* RecordingRenderResourceManager::createIndexBuffer(const physx::apex::NxUserRenderIndexBufferDesc& desc)
{
	physx::apex::NxUserRenderIndexBuffer* child = NULL;
	if (mChild != NULL)
	{
		child = mChild->createIndexBuffer(desc);
	}

	return new RecordingIndexBuffer(desc, child, mRecorder);
}



void RecordingRenderResourceManager::releaseIndexBuffer(physx::apex::NxUserRenderIndexBuffer& buffer)
{
	if (mChild != NULL)
	{
		mChild->releaseIndexBuffer(*reinterpret_cast<RecordingIndexBuffer&>(buffer).getChild());
	}

	delete &buffer;
}



class RecordingBoneBuffer : public physx::apex::NxUserRenderBoneBuffer
{
public:
	RecordingBoneBuffer(const physx::apex::NxUserRenderBoneBufferDesc& desc, physx::apex::NxUserRenderBoneBuffer* child, RecordingRenderResourceManager::RecorderInterface* recorder) :
		mDescriptor(desc), mChild(child), mBufferId(0), mRecorder(recorder)
	{
		mBufferId = boneBufferId++;

		if (mRecorder != NULL)
		{
			mRecorder->createBoneBuffer(mBufferId, desc);
		}
	}

	~RecordingBoneBuffer()
	{
		if (mRecorder != NULL)
		{
			mRecorder->releaseBoneBuffer(mBufferId);
		}
	}

	virtual void writeBuffer(const physx::apex::NxApexRenderBoneBufferData& data, unsigned int firstBone, unsigned int numBones)
	{
		if (mChild != NULL)
		{
			mChild->writeBuffer(data, firstBone, numBones);
		}

		if (mRecorder != NULL)
		{
			mRecorder->writeBoneBuffer(mBufferId, data, firstBone, numBones);
		}
	}

	physx::apex::NxUserRenderBoneBuffer* getChild()
	{
		return mChild;
	}

protected:
	physx::apex::NxUserRenderBoneBufferDesc mDescriptor;
	physx::apex::NxUserRenderBoneBuffer* mChild;

	static unsigned int boneBufferId;
	unsigned int mBufferId;

	RecordingRenderResourceManager::RecorderInterface* mRecorder;
};

unsigned int RecordingBoneBuffer::boneBufferId = 0;


physx::apex::NxUserRenderBoneBuffer* RecordingRenderResourceManager::createBoneBuffer(const physx::apex::NxUserRenderBoneBufferDesc& desc)
{
	physx::apex::NxUserRenderBoneBuffer* child = NULL;
	if (mChild != NULL)
	{
		child = mChild->createBoneBuffer(desc);
	}

	return new RecordingBoneBuffer(desc, child, mRecorder);
}



void RecordingRenderResourceManager::releaseBoneBuffer(physx::apex::NxUserRenderBoneBuffer& buffer)
{
	if (mChild != NULL)
	{
		mChild->releaseBoneBuffer(*reinterpret_cast<RecordingBoneBuffer&>(buffer).getChild());
	}
}



physx::apex::NxUserRenderInstanceBuffer* RecordingRenderResourceManager::createInstanceBuffer(const physx::apex::NxUserRenderInstanceBufferDesc& desc)
{
	UNIMPLEMENTED;
	if (mChild != NULL)
	{
		return mChild->createInstanceBuffer(desc);
	}

	return NULL;
}



void RecordingRenderResourceManager::releaseInstanceBuffer(physx::apex::NxUserRenderInstanceBuffer& buffer)
{
	if (mChild != NULL)
	{
		mChild->releaseInstanceBuffer(buffer);
	}
}



physx::apex::NxUserRenderSpriteBuffer* RecordingRenderResourceManager::createSpriteBuffer(const physx::apex::NxUserRenderSpriteBufferDesc& desc)
{
	UNIMPLEMENTED;
	if (mChild != NULL)
	{
		return mChild->createSpriteBuffer(desc);
	}

	return NULL;
}



void RecordingRenderResourceManager::releaseSpriteBuffer(physx::apex::NxUserRenderSpriteBuffer& buffer)
{
	if (mChild != NULL)
	{
		mChild->releaseSpriteBuffer(buffer);
	}
}


physx::apex::NxUserRenderSurfaceBuffer* RecordingRenderResourceManager::createSurfaceBuffer(const physx::apex::NxUserRenderSurfaceBufferDesc& desc)
{
	UNIMPLEMENTED;
	if (mChild != NULL)
	{
		return mChild->createSurfaceBuffer(desc);
	}

	return NULL;
}



void RecordingRenderResourceManager::releaseSurfaceBuffer(physx::apex::NxUserRenderSurfaceBuffer& buffer)
{
	if (mChild != NULL)
	{
		mChild->releaseSurfaceBuffer(buffer);
	}
}


class RecordingRenderResource : public physx::apex::NxUserRenderResource
{
public:
	RecordingRenderResource(const physx::apex::NxUserRenderResourceDesc& desc, physx::apex::NxUserRenderResourceManager* childRrm, RecordingRenderResourceManager::RecorderInterface* recorder) : mChild(NULL), mDescriptor(desc), mRecorder(recorder)
	{
		assert(desc.numVertexBuffers > 0);

		mResourceId = resourceIds++;

		vertexBufferOriginal.resize(desc.numVertexBuffers);
		vertexBufferChild.resize(desc.numVertexBuffers);
		for (size_t i = 0; i < vertexBufferOriginal.size(); i++)
		{
			vertexBufferOriginal[i] = desc.vertexBuffers[i];
			vertexBufferChild[i] = reinterpret_cast<RecordingVertexBuffer*>(desc.vertexBuffers[i])->getChild();
		}

		mDescriptor.vertexBuffers = &vertexBufferOriginal[0];

		physx::apex::NxUserRenderResourceDesc newDesc(desc);
		newDesc.vertexBuffers = &vertexBufferChild[0];

		if (desc.indexBuffer != NULL)
		{
			newDesc.indexBuffer = reinterpret_cast<RecordingIndexBuffer*>(desc.indexBuffer)->getChild();
		}

		if (desc.boneBuffer != NULL)
		{
			newDesc.boneBuffer = reinterpret_cast<RecordingBoneBuffer*>(desc.boneBuffer)->getChild();
		}

		if (childRrm != NULL)
		{
			mChild = childRrm->createResource(newDesc);
		}

		if (mRecorder != NULL)
		{
			mRecorder->createResource(mResourceId, desc);
		}
	}

	~RecordingRenderResource()
	{

	}


	physx::apex::NxUserRenderResource* getChild()
	{
		return mChild;
	}



	void setVertexBufferRange(unsigned int firstVertex, unsigned int numVerts)
	{
		if (mChild != NULL)
		{
			mChild->setVertexBufferRange(firstVertex, numVerts);
		}

		mDescriptor.firstVertex = firstVertex;
		mDescriptor.numVerts = numVerts;
	}



	void setIndexBufferRange(unsigned int firstIndex, unsigned int numIndices)
	{
		if (mChild != NULL)
		{
			mChild->setIndexBufferRange(firstIndex, numIndices);
		}

		mDescriptor.firstIndex = firstIndex;
		mDescriptor.numIndices = numIndices;
	}



	void setBoneBufferRange(unsigned int firstBone, unsigned int numBones)
	{
		if (mChild != NULL)
		{
			mChild->setBoneBufferRange(firstBone, numBones);
		}

		mDescriptor.firstBone = firstBone;
		mDescriptor.numBones = numBones;
	}



	void setInstanceBufferRange(unsigned int firstInstance, unsigned int numInstances)
	{
		if (mChild != NULL)
		{
			mChild->setInstanceBufferRange(firstInstance, numInstances);
		}

		mDescriptor.firstInstance = firstInstance;
		mDescriptor.numInstances = numInstances;
	}



	void setSpriteBufferRange(unsigned int firstSprite, unsigned int numSprites)
	{
		if (mChild != NULL)
		{
			mChild->setSpriteBufferRange(firstSprite, numSprites);
		}

		mDescriptor.firstSprite = firstSprite;
		mDescriptor.numSprites = numSprites;
	}




	void setMaterial(void* material)
	{
		if (mChild != NULL)
		{
			mChild->setMaterial(material);
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

	void render()
	{
		if (mRecorder != NULL)
		{
			mRecorder->renderResource(mResourceId, mDescriptor);
		}
	}


protected:
	std::vector<physx::apex::NxUserRenderVertexBuffer*> vertexBufferOriginal;
	std::vector<physx::apex::NxUserRenderVertexBuffer*> vertexBufferChild;
	physx::apex::NxUserRenderResource* mChild;

	physx::apex::NxUserRenderResourceDesc mDescriptor;

	static unsigned int resourceIds;
	unsigned int mResourceId;

	RecordingRenderResourceManager::RecorderInterface* mRecorder;
};

unsigned int RecordingRenderResource::resourceIds = 0;



physx::apex::NxUserRenderResource* RecordingRenderResourceManager::createResource(const physx::apex::NxUserRenderResourceDesc& desc)
{
	return new RecordingRenderResource(desc, mChild, mRecorder);
}



void RecordingRenderResourceManager::releaseResource(physx::apex::NxUserRenderResource& resource)
{
	if (mChild != NULL)
	{
		mChild->releaseResource(*reinterpret_cast<RecordingRenderResource&>(resource).getChild());
	}

	delete &resource;
}



unsigned int RecordingRenderResourceManager::getMaxBonesForMaterial(void* material)
{
	unsigned int maxBones = 60; // whatever
	if (mChild != NULL)
	{
		maxBones = mChild->getMaxBonesForMaterial(material);
	}

	if (mRecorder != NULL)
	{
		mRecorder->setMaxBonesForMaterial(material, maxBones);
	}

	return maxBones;
}




RecordingRenderer::RecordingRenderer(physx::apex::NxUserRenderer* child, RecordingRenderResourceManager::RecorderInterface* recorder) : mChild(child), mRecorder(recorder)
{

}



RecordingRenderer::~RecordingRenderer()
{

}



void RecordingRenderer::renderResource(const physx::apex::NxApexRenderContext& context)
{
	RecordingRenderResource* resource = reinterpret_cast<RecordingRenderResource*>(context.renderResource);

	resource->render();

	physx::apex::NxUserRenderResource* child = resource->getChild();
	if (mChild != NULL && child != NULL)
	{
		physx::apex::NxApexRenderContext newContext(context);
		newContext.renderResource = child;

		mChild->renderResource(newContext);
	}
}




FileRecorder::FileRecorder(const char* filename)
{

	if (filename != NULL)
	{
		mOutputFile = fopen(filename, "w");
		assert(mOutputFile);

		if (mOutputFile != NULL)
		{
			time_t curtime;
			::time(&curtime);

			fprintf(mOutputFile, "# Logfile created on %s\n\n", ctime(&curtime));
		}
	}
	else
	{
		// console
		mOutputFile = stdout;

#ifdef PX_WINDOWS
		//open a console for printf:
		if (AllocConsole())
		{
			FILE* stream;
			freopen_s(&stream, "CONOUT$", "wb", stdout);
			freopen_s(&stream, "CONOUT$", "wb", stderr);

			SetConsoleTitle("Recording Resource Manager Output");
			//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);

			CONSOLE_SCREEN_BUFFER_INFO coninfo;
			GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
			coninfo.dwSize.Y = 1000;
			SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);
		}
#endif
	}
}


FileRecorder::~FileRecorder()
{
	if (mOutputFile != stdout && mOutputFile != NULL)
	{
		fclose(mOutputFile);
	}
}


#define WRITE_ITEM(_A) writeElem(#_A, _A)
#define WRITE_DESC_ELEM(_A) writeElem(#_A, (physx::PxU32)desc._A)
#define WRITE_REQUEST(_A) if (desc.buffersRequest[physx::apex::NxRenderVertexSemantic::_A] != physx::apex::NxRenderDataFormat::UNSPECIFIED) writeElem(#_A, desc.buffersRequest[physx::apex::NxRenderVertexSemantic::_A])



void FileRecorder::createVertexBuffer(unsigned int id, const physx::apex::NxUserRenderVertexBufferDesc& desc)
{
	fprintf(mOutputFile, "VertexBuffer[%d]::create: ", id);

	WRITE_DESC_ELEM(maxVerts);
	WRITE_DESC_ELEM(hint);

	WRITE_REQUEST(POSITION);
	WRITE_REQUEST(NORMAL);
	WRITE_REQUEST(TANGENT);
	WRITE_REQUEST(BINORMAL);
	WRITE_REQUEST(COLOR);
	WRITE_REQUEST(TEXCOORD0);
	WRITE_REQUEST(TEXCOORD1);
	WRITE_REQUEST(TEXCOORD2);
	WRITE_REQUEST(TEXCOORD3);
	WRITE_REQUEST(BONE_INDEX);
	WRITE_REQUEST(BONE_WEIGHT);
	WRITE_REQUEST(DISPLACEMENT_TEXCOORD);
	WRITE_REQUEST(DISPLACEMENT_FLAGS);

	WRITE_DESC_ELEM(numCustomBuffers);
	// PH: not done on purpose (yet)
	//void** 						customBuffersIdents;
	//NxRenderDataFormat::Enum*	customBuffersRequest;
	WRITE_DESC_ELEM(moduleIdentifier);
	WRITE_DESC_ELEM(uvOrigin);
	WRITE_DESC_ELEM(canBeShared);

	fprintf(mOutputFile, "\n");

#ifdef PX_X86
	// PH: Make sure that if the size of the descriptor changes, we get a compile error here and adapt the WRITE_REQUESTs from above accordingly
	PX_COMPILE_TIME_ASSERT(sizeof(desc) == 4 + 4 + (13 * 4) + 4 + sizeof(void*) + sizeof(void*) + 4 + 4 + 1 + 1 + 2/*padding*/ + sizeof(void*) );
#endif
}



#define WRITE_DATA_ITEM(_A) writeElem(#_A, semanticData._A)
void FileRecorder::writeVertexBuffer(unsigned int id, const physx::apex::NxApexRenderVertexBufferData& data, unsigned int firstVertex, unsigned int numVertices)
{
	fprintf(mOutputFile, "VertexBuffer[%d]::write: ", id);
	WRITE_ITEM(firstVertex);
	WRITE_ITEM(numVertices);

	WRITE_ITEM(data.moduleId);
	WRITE_ITEM(data.numModuleSpecificSemantics);

	fprintf(mOutputFile, "\n");

#ifdef PX_X86
	PX_COMPILE_TIME_ASSERT(sizeof(physx::apex::NxApexRenderSemanticData) == (sizeof(void*) + 4 + sizeof(void*) + 4 + 4 + 1 + 3/*padding*/));
#endif

	for (unsigned int i = 0; i < physx::apex::NxRenderVertexSemantic::NUM_SEMANTICS; i++)
	{
		const physx::apex::NxApexRenderSemanticData& semanticData = data.getSemanticData(physx::apex::NxRenderVertexSemantic::Enum(i));
		if (semanticData.format != physx::apex::NxRenderDataFormat::UNSPECIFIED)
		{
			fprintf(mOutputFile, " [%d]: ", i);
			WRITE_DATA_ITEM(stride);
			WRITE_DATA_ITEM(format);
			WRITE_DATA_ITEM(srcFormat);
			fprintf(mOutputFile, "\n");

			//writeBufferData(semanticData.data, semanticData.stride, numVertices, semanticData.format);
		}
	}

	PX_COMPILE_TIME_ASSERT(physx::apex::NxRenderVertexSemantic::NUM_SEMANTICS == 13);

#ifdef PX_X86
	PX_COMPILE_TIME_ASSERT(sizeof(data) == sizeof(physx::apex::NxApexRenderSemanticData) * physx::apex::NxRenderVertexSemantic::NUM_SEMANTICS + 4 + sizeof(void*) + 4 + sizeof(void*) + 4);
#endif
}
#undef WRITE_DATA_ITEM



void FileRecorder::releaseVertexBuffer(unsigned int id)
{
	fprintf(mOutputFile, "VertexBuffer[%d]::release\n", id);
}




void FileRecorder::createIndexBuffer(unsigned int id, const physx::apex::NxUserRenderIndexBufferDesc& desc)
{
	fprintf(mOutputFile, "IndexBuffer[%d]::create: ", id);

	WRITE_DESC_ELEM(maxIndices);
	WRITE_DESC_ELEM(hint);
	WRITE_DESC_ELEM(format);
	WRITE_DESC_ELEM(primitives);
	WRITE_DESC_ELEM(registerInCUDA);
	//WRITE_DESC_ELEM(interopContext);

	fprintf(mOutputFile, "\n");

#ifdef PX_X86
	PX_COMPILE_TIME_ASSERT(sizeof(desc) == 4 + 4 + 4 + 4 + 1 + 3/*padding*/ + sizeof(void*));
#endif
}



void FileRecorder::writeIndexBuffer(unsigned int id, const void* /*srcData*/, physx::PxU32 /*srcStride*/, unsigned int firstDestElement, unsigned int numElements, physx::apex::NxRenderDataFormat::Enum /*format*/)
{
	fprintf(mOutputFile, "IndexBuffer[%d]::write ", id);
	WRITE_ITEM(firstDestElement);
	WRITE_ITEM(numElements);

	//writeBufferData(srcData, srcStride, numElements, format);

	fprintf(mOutputFile, "\n");
}



void FileRecorder::releaseIndexBuffer(unsigned int id)
{
	fprintf(mOutputFile, "IndexBuffer[%d]::release\n", id);
}



void FileRecorder::createBoneBuffer(unsigned int id, const physx::apex::NxUserRenderBoneBufferDesc& /*desc*/)
{
	fprintf(mOutputFile, "BoneBuffer[%d]::create\n", id);
}



void FileRecorder::writeBoneBuffer(unsigned int id, const physx::apex::NxApexRenderBoneBufferData& /*data*/, unsigned int /*firstBone*/, unsigned int /*numBones*/)
{
	fprintf(mOutputFile, "BoneBuffer[%d]::write\n", id);
}



void FileRecorder::releaseBoneBuffer(unsigned int id)
{
	fprintf(mOutputFile, "BoneBuffer[%d]::release\n", id);
}



void FileRecorder::createResource(unsigned int id, const physx::apex::NxUserRenderResourceDesc& /*desc*/)
{
	fprintf(mOutputFile, "Resource[%d]::create\n", id);
}



void FileRecorder::renderResource(unsigned int id, const physx::apex::NxUserRenderResourceDesc& /*desc*/)
{
	fprintf(mOutputFile, "Resource[%d]::render\n", id);
}



void FileRecorder::releaseResource(unsigned int id)
{
	fprintf(mOutputFile, "Resource[%d]::release\n", id);
}



void FileRecorder::setMaxBonesForMaterial(void* material, unsigned int maxBones)
{
	fprintf(mOutputFile, "MaterialMaxBones[%p]=%d\n", material, maxBones);
}



void FileRecorder::writeElem(const char* name, unsigned int value)
{
	fprintf(mOutputFile, "%s=%d ", name, value);
}


void FileRecorder::writeBufferData(const void* data, unsigned int stride, unsigned int numElements, physx::apex::NxRenderDataFormat::Enum format)
{
	switch (format)
	{
	case physx::apex::NxRenderDataFormat::FLOAT2:
		writeBufferDataFloat(data, stride, numElements, 2);
		break;
	case physx::apex::NxRenderDataFormat::FLOAT3:
		writeBufferDataFloat(data, stride, numElements, 3);
		break;
	case physx::apex::NxRenderDataFormat::FLOAT4:
		writeBufferDataFloat(data, stride, numElements, 4);
		break;
	case physx::apex::NxRenderDataFormat::USHORT4:
		writeBufferDataShort(data, stride, numElements, 4);
		break;
	case physx::apex::NxRenderDataFormat::UINT1:
		writeBufferDataLong(data, stride, numElements, 1);
		break;
	default:
		UNIMPLEMENTED;
		break;
	}
}




void FileRecorder::writeBufferDataFloat(const void* data, unsigned int stride, unsigned int numElements, unsigned int numFloatsPerDataSet)
{
	const char* startData = (const char*)data;
	for (unsigned int i = 0; i < numElements; i++)
	{
		const float* elemData = (const float*)(startData + stride * i);

		fprintf(mOutputFile, "(");
		for (unsigned int j = 0; j < numFloatsPerDataSet; j++)
		{
			fprintf(mOutputFile, "%f ", elemData[j]);
		}
		fprintf(mOutputFile, "),");
	}
}


void FileRecorder::writeBufferDataShort(const void* data, unsigned int stride, unsigned int numElements, unsigned int numFloatsPerDataSet)
{
	const char* startData = (const char*)data;
	for (unsigned int i = 0; i < numElements; i++)
	{
		const short* elemData = (const short*)(startData + stride * i);

		fprintf(mOutputFile, "(");
		for (unsigned int j = 0; j < numFloatsPerDataSet; j++)
		{
			fprintf(mOutputFile, "%d ", elemData[j]);
		}
		fprintf(mOutputFile, "),");
	}
}


void FileRecorder::writeBufferDataLong(const void* data, unsigned int stride, unsigned int numElements, unsigned int numFloatsPerDataSet)
{
	const char* startData = (const char*)data;
	for (unsigned int i = 0; i < numElements; i++)
	{
		const long* elemData = (const long*)(startData + stride * i);

		fprintf(mOutputFile, "(");
		for (unsigned int j = 0; j < numFloatsPerDataSet; j++)
		{
			fprintf(mOutputFile, "%d ", (int)elemData[j]);
		}
		fprintf(mOutputFile, "),");
	}
}
