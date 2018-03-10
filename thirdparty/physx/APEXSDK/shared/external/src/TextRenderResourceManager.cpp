/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "TextRenderResourceManager.h"

#include "NxUserRenderResource.h"
#include "NxUserRenderResourceDesc.h"

#include "NxApexRenderContext.h"


TextRenderResourceManager::TextRenderResourceManager() :
mVerbosity(0),
	mOutputFile(NULL),
	mIO(NULL),
	mVertexBufferCount(0),
	mIndexBufferCount(0),
	mBoneBufferCount(0),
	mInstanceBufferCount(0),
	mSpriteBufferCount(0),
	mRenderResourceCount(0),
	mSurfaceBufferCount(0)
{
}


TextRenderResourceManager::TextRenderResourceManager(int verbosity, const char* outputFilename) :
	mVerbosity(verbosity),
	mOutputFile(NULL),
	mVertexBufferCount(0),
	mIndexBufferCount(0),
	mBoneBufferCount(0),
	mInstanceBufferCount(0),
	mSpriteBufferCount(0),
	mRenderResourceCount(0),
	mSurfaceBufferCount(0)
{
	if (outputFilename != NULL)
	{
		mOutputFile = fopen(outputFilename, "w");
	}
	else
	{
		mOutputFile = stdout;
	}
	PX_ASSERT(mOutputFile != NULL);
	
	mIO = new Writer(mOutputFile);
}


TextRenderResourceManager::~TextRenderResourceManager()
{
	if (mOutputFile != NULL && mOutputFile != stdout)
	{
		fclose(mOutputFile);
	}

	if (mIO != NULL)
	{
		delete mIO;
		mIO = NULL;
	}
}

unsigned int TextRenderResourceManager::material2Id(void* material)
{
	std::map<void*, unsigned int>::iterator it = mMaterial2Id.find(material);
	if (it != mMaterial2Id.end())
	{
		return it->second;
	}

	unsigned int result = (unsigned int)mMaterial2Id.size();
	mMaterial2Id[material] = result;

	return result;
}

bool TextRenderResourceManager::getSpriteLayoutData(physx::PxU32 spriteCount, physx::PxU32 spriteSemanticsBitmap, physx::apex::NxUserRenderSpriteBufferDesc* bufferDesc)
{
	PX_UNUSED(spriteCount);
	PX_UNUSED(spriteSemanticsBitmap);
	PX_UNUSED(bufferDesc);
	return false;
}

bool TextRenderResourceManager::getInstanceLayoutData(physx::PxU32 particleCount, physx::PxU32 particleSemanticsBitmap, physx::apex::NxUserRenderInstanceBufferDesc* bufferDesc)
{
	PX_UNUSED(particleCount);
	PX_UNUSED(particleSemanticsBitmap);
	PX_UNUSED(bufferDesc);
	return false;
}



Writer::Writer(FILE* outputFile) : mOutputFile(outputFile)
{
	mIsStdout = mOutputFile == stdout;
}

Writer::~Writer() {}

void Writer::printAndScan(const char* format)
{
	fprintf(mOutputFile,"%s", format);
}

void Writer::printAndScan(const char* format, const char* arg)
{
	fprintf(mOutputFile, format, arg);
}

void Writer::printAndScan(const char* format, int arg)
{
	fprintf(mOutputFile, format, arg);
}

void Writer::printAndScan(float /*tol*/, physx::apex::NxRenderVertexSemantic::Enum /*s*/, const char* format, float arg)
{
	fprintf(mOutputFile, format, arg);
}

const char* Writer::semanticToString(physx::apex::NxRenderVertexSemantic::Enum semantic)
{
	const char* result = NULL;
	switch (semantic)
	{
#define CASE(_SEMANTIC) case physx::apex::NxRenderVertexSemantic::_SEMANTIC: result = #_SEMANTIC; break
		CASE(POSITION);
		CASE(NORMAL);
		CASE(TANGENT);
		CASE(BINORMAL);
		CASE(COLOR);
		CASE(TEXCOORD0);
		CASE(TEXCOORD1);
		CASE(TEXCOORD2);
		CASE(TEXCOORD3);
		CASE(BONE_INDEX);
		CASE(BONE_WEIGHT);
#undef CASE

	default:
		PX_ALWAYS_ASSERT();
	}

	return result;
}



const char* Writer::semanticToString(physx::apex::NxRenderBoneSemantic::Enum semantic)
{
	const char* result = NULL;
	switch (semantic)
	{
#define CASE(_SEMANTIC) case physx::apex::NxRenderBoneSemantic::_SEMANTIC: result = #_SEMANTIC; break
		CASE(POSE);
		CASE(PREVIOUS_POSE);
#undef CASE

	// if this assert is hit add/remove semantics above to match NxRenderBoneSemantic
	PX_COMPILE_TIME_ASSERT(physx::apex::NxRenderBoneSemantic::NUM_SEMANTICS == 2);

	default:
		PX_ALWAYS_ASSERT();
	}

	return result;
}



void Writer::writeElem(const char* name, unsigned int val)
{
	const bool isStdout = mIsStdout;

	if (isStdout)
	{
		printAndScan("\n  ");
	}
	printAndScan("%s=", name);
	printAndScan("%d", (int)val);
	if (!isStdout)
	{
		printAndScan(" ");
	}
}



void Writer::writeArray(physx::apex::NxRenderDataFormat::Enum format, unsigned int stride, unsigned int numElements, const void* data, float tolerance, physx::apex::NxRenderVertexSemantic::Enum s)
{
	if (mIsStdout)
	{
		unsigned int maxNumElements = 1000000;

		switch (format)
		{
		case physx::apex::NxRenderDataFormat::USHORT1:
		case physx::apex::NxRenderDataFormat::UINT1:
			maxNumElements = 10;
			break;
		case physx::apex::NxRenderDataFormat::FLOAT2:
			maxNumElements = 3;
			break;
		case physx::apex::NxRenderDataFormat::FLOAT3:
			maxNumElements = 2;
			break;
		case physx::apex::NxRenderDataFormat::FLOAT3x4:
			maxNumElements = 1;
			break;
		default:
			PX_ALWAYS_ASSERT();
		}

		if (maxNumElements < numElements)
		{
			numElements = maxNumElements;
		}
	}

	for (unsigned int i = 0; i < numElements; i++)
	{
		const char* dataSample = ((const char*)data) + stride * i;

		switch (format)
		{
		case physx::apex::NxRenderDataFormat::USHORT1:
		{
			const short* sh = (const short*)dataSample;
			printAndScan("%d ", sh[0]);
		}
		break;
		case physx::apex::NxRenderDataFormat::USHORT2:
		{
			const short* sh = (const short*)dataSample;
			printAndScan("(%d ", sh[0]);
			printAndScan("%d )", sh[1]);
		}
		break;
		case physx::apex::NxRenderDataFormat::USHORT3:
			{
				const short* sh = (const short*)dataSample;
				printAndScan("(%d ", sh[0]);
				printAndScan("%d ", sh[2]);
				printAndScan("%d )", sh[1]);
			}
			break;
		case physx::apex::NxRenderDataFormat::USHORT4:
		{
			const short* sh = (const short*)dataSample;
			printAndScan("(%d ", sh[0]);
			printAndScan("%d ", sh[1]);
			printAndScan("%d ", sh[2]);
			printAndScan("%d )", sh[3]);
		}
		break;
		case physx::apex::NxRenderDataFormat::UINT1:
		{
			const unsigned int* ui = (const unsigned int*)dataSample;
			printAndScan("%d ", (int)ui[0]);
		}
		break;
		case physx::apex::NxRenderDataFormat::FLOAT2:
		{
			const float* fl = (const float*)dataSample;
			printAndScan(tolerance, s, "(%f ", fl[0]);
			printAndScan(tolerance, s, "%f ) ", fl[1]);
		}
		break;
		case physx::apex::NxRenderDataFormat::FLOAT3:
		{
			const physx::PxVec3* vec3 = (const physx::PxVec3*)dataSample;
			printAndScan(tolerance, s, "(%f ", vec3->x);
			printAndScan(tolerance, s, "%f ", vec3->y);
			printAndScan(tolerance, s, "%f ) ", vec3->z);
		}
		break;
		case physx::apex::NxRenderDataFormat::FLOAT4:
		{
			const physx::PxVec4* vec4 = (const physx::PxVec4*)dataSample;
			printAndScan(tolerance, s, "(%f ", vec4->x);
			printAndScan(tolerance, s, "%f ", vec4->y);
			printAndScan(tolerance, s, "%f ", vec4->z);
			printAndScan(tolerance, s, "%f ) ", vec4->w);
		}
		break;
		case physx::apex::NxRenderDataFormat::FLOAT3x4:
		{
			const physx::PxVec3* vec3 = (const physx::PxVec3*)dataSample;
			printAndScan("(");
			for (unsigned int j = 0; j < 4; j++)
			{
				printAndScan(tolerance, s, "%f ", vec3[j].x);
				printAndScan(tolerance, s, "%f ", vec3[j].y);
				printAndScan(tolerance, s, "%f ", vec3[j].z);
			}
			printAndScan(") ");
		}
		break;
		case physx::apex::NxRenderDataFormat::R8G8B8A8:
		case physx::apex::NxRenderDataFormat::B8G8R8A8:
			{
				const unsigned int* ui = (const unsigned int*)dataSample;
				printAndScan("0x%x ", (int)ui[0]);
			}
		break;
		default:
			PX_ALWAYS_ASSERT();
		}
	}
}


#define WRITE_ITEM(_A) mIO->writeElem(#_A, _A)
#define WRITE_DESC_ELEM(_A) mIO->writeElem(#_A, (physx::PxU32)desc._A)
#define WRITE_VREQUEST(_A) if (desc.buffersRequest[physx::apex::NxRenderVertexSemantic::_A] != physx::apex::NxRenderDataFormat::UNSPECIFIED) mIO->writeElem(#_A, desc.buffersRequest[physx::apex::NxRenderVertexSemantic::_A])
#define WRITE_BREQUEST(_A) if (desc.buffersRequest[physx::apex::NxRenderBoneSemantic::_A] != physx::apex::NxRenderDataFormat::UNSPECIFIED) mIO->writeElem(#_A, desc.buffersRequest[physx::apex::NxRenderBoneSemantic::_A])











class TextUserRenderVertexBuffer : public physx::apex::NxUserRenderVertexBuffer
{
public:
	TextUserRenderVertexBuffer(const physx::apex::NxUserRenderVertexBufferDesc& desc, TextRenderResourceManager* manager, Writer* readerWriter, int bufferId) : mManager(manager), mIO(readerWriter), mWriteCalls(0), mDescriptor(desc)
	{
		mBufferId = bufferId;

		if (mManager->getVerbosity() >= 1)
		{
			mIO->printAndScan("VertexBuffer[%d]::create(", mBufferId);

			WRITE_DESC_ELEM(maxVerts);
			WRITE_DESC_ELEM(hint);
			WRITE_VREQUEST(POSITION);
			WRITE_VREQUEST(NORMAL);
			WRITE_VREQUEST(TANGENT);
			WRITE_VREQUEST(BINORMAL);
			WRITE_VREQUEST(COLOR);
			WRITE_VREQUEST(TEXCOORD0);
			WRITE_VREQUEST(TEXCOORD1);
			WRITE_VREQUEST(TEXCOORD2);
			WRITE_VREQUEST(TEXCOORD3);
			WRITE_VREQUEST(BONE_INDEX);
			WRITE_VREQUEST(BONE_WEIGHT);
			// PH: not done on purpose (yet)
			//WRITE_DESC_ELEM(numCustomBuffers);
			//void** 						customBuffersIdents;
			//NxRenderDataFormat::Enum*	customBuffersRequest;
			WRITE_DESC_ELEM(moduleIdentifier);
			WRITE_DESC_ELEM(uvOrigin);
			WRITE_DESC_ELEM(canBeShared);

			mIO->printAndScan(")\n");
		}
	}

	~TextUserRenderVertexBuffer()
	{
		if (mManager->getVerbosity() >= 1)
		{
			mIO->printAndScan("VertexBuffer[%d]::destroy\n", mBufferId);
		}
	}

	virtual void writeBuffer(const physx::NxApexRenderVertexBufferData& data, physx::PxU32 firstVertex, physx::PxU32 numVertices)
	{
		if (mManager->getVerbosity() >= 2)
		{
			mIO->printAndScan("VertexBuffer[%d]", mBufferId);
			mIO->printAndScan("::writeBuffer#%d", (int)mWriteCalls);
			mIO->printAndScan("(firstVertex=%d ", (int)firstVertex);
			mIO->printAndScan("numVertices=%d)\n", (int)numVertices);

			if (mManager->getVerbosity() >= 3)
			{
				for (unsigned int i = 0; i < physx::apex::NxRenderVertexSemantic::NUM_SEMANTICS; i++)
				{
					physx::apex::NxRenderVertexSemantic::Enum s = physx::apex::NxRenderVertexSemantic::Enum(i);
					const physx::apex::NxApexRenderSemanticData& semanticData = data.getSemanticData(s);

					if (semanticData.data != NULL)
					{
						mIO->printAndScan(" %s: ", mIO->semanticToString(s));
						mIO->writeArray(semanticData.format, semanticData.stride, numVertices, semanticData.data, mManager->getVBTolerance(s), s);
						mIO->printAndScan("\n");
					}
				}
			}
		}

		mWriteCalls++;
	}


	unsigned int getId()
	{
		return (physx::PxU32)mBufferId;
	}


protected:
	TextRenderResourceManager* mManager;
	Writer* mIO;
	int mBufferId;
	unsigned int mWriteCalls;
	physx::apex::NxUserRenderVertexBufferDesc mDescriptor;
};




physx::apex::NxUserRenderVertexBuffer* TextRenderResourceManager::createVertexBuffer(const physx::apex::NxUserRenderVertexBufferDesc& desc)
{
	physx::apex::NxUserRenderVertexBuffer* vb = new TextUserRenderVertexBuffer(desc, this, mIO, mVertexBufferCount++);
	return vb;
};



void TextRenderResourceManager::releaseVertexBuffer(physx::apex::NxUserRenderVertexBuffer& buffer)
{
	delete &buffer;
}


















class TextUserRenderIndexBuffer : public physx::apex::NxUserRenderIndexBuffer
{
public:
	TextUserRenderIndexBuffer(const physx::apex::NxUserRenderIndexBufferDesc& desc, TextRenderResourceManager* manager, Writer* readerWriter, int bufferId) : mManager(manager), mIO(readerWriter), mWriteCalls(0), mDescriptor(desc)
	{
		mBufferId = bufferId;

		if (mManager->getVerbosity() >= 1)
		{
			mIO->printAndScan("IndexBuffer[%d]::create(", mBufferId);

			WRITE_DESC_ELEM(maxIndices);
			WRITE_DESC_ELEM(hint);
			WRITE_DESC_ELEM(format);
			WRITE_DESC_ELEM(primitives);
			WRITE_DESC_ELEM(registerInCUDA);

			mIO->printAndScan(")\n");
		}
	}

	~TextUserRenderIndexBuffer()
	{
		if (mManager->getVerbosity() >= 1)
		{
			mIO->printAndScan("IndexBuffer[%d]::destroy\n", mBufferId);
		}
	}

	virtual void writeBuffer(const void* srcData, physx::PxU32 srcStride, physx::PxU32 firstDestElement, physx::PxU32 numElements)
	{
		if (mManager->getVerbosity() >= 2)
		{
			mIO->printAndScan("IndexBuffer[%d]", mBufferId);
			mIO->printAndScan("::writeBuffer#%d", (int)mWriteCalls);
			mIO->printAndScan("(srcStride=%d ", (int)srcStride);
			mIO->printAndScan("firstDestElement=%d ", (int)firstDestElement);
			mIO->printAndScan("numElements=%d)\n ", (int)numElements);

			mIO->writeArray(mDescriptor.format, srcStride, numElements, srcData, 0.0f, physx::apex::NxRenderVertexSemantic::CUSTOM);

			mIO->printAndScan("\n");
		}

		mWriteCalls++;
	}


	unsigned int getId()
	{
		return (physx::PxU32)mBufferId;
	}

protected:
	TextRenderResourceManager* mManager;
	Writer* mIO;
	int mBufferId;
	unsigned int mWriteCalls;
	physx::apex::NxUserRenderIndexBufferDesc mDescriptor;
};



physx::apex::NxUserRenderIndexBuffer* TextRenderResourceManager::createIndexBuffer(const physx::apex::NxUserRenderIndexBufferDesc& desc)
{
	physx::apex::NxUserRenderIndexBuffer* ib = new TextUserRenderIndexBuffer(desc, this, mIO, mIndexBufferCount++);
	return ib;
}



void TextRenderResourceManager::releaseIndexBuffer(physx::apex::NxUserRenderIndexBuffer& buffer)
{
	delete &buffer;
}


















class TextUserRenderBoneBuffer : public physx::apex::NxUserRenderBoneBuffer
{
public:
	TextUserRenderBoneBuffer(const physx::apex::NxUserRenderBoneBufferDesc& desc, TextRenderResourceManager* manager, Writer* readerWriter, int bufferId) : mManager(manager), mIO(readerWriter), mWriteCalls(0), mDescriptor(desc)
	{
		mBufferId = bufferId;

		if (mManager->getVerbosity() >= 1)
		{
			mIO->printAndScan("BoneBuffer[%d]::create(", mBufferId);
			WRITE_DESC_ELEM(maxBones);
			WRITE_DESC_ELEM(hint);
			WRITE_BREQUEST(POSE);
			mIO->printAndScan(")\n");
		}
	}

	~TextUserRenderBoneBuffer()
	{
		if (mManager->getVerbosity() >= 1)
		{
			mIO->printAndScan("BoneBuffer[%d]::destroy\n", mBufferId);
		}
	}

	virtual void writeBuffer(const physx::NxApexRenderBoneBufferData& data, physx::PxU32 firstBone, physx::PxU32 numBones)
	{
		if (mManager->getVerbosity() >= 2)
		{
			mIO->printAndScan("BoneBuffer[%d]", mBufferId);
			mIO->printAndScan("::writeBuffer#%d", (int)mWriteCalls);
			mIO->printAndScan("(firstBone=%d ", (int)firstBone);
			mIO->printAndScan("numBones=%d)\n", (int)numBones);

			for (unsigned int i = 0; i < physx::apex::NxRenderBoneSemantic::NUM_SEMANTICS; i++)
			{
				physx::apex::NxRenderBoneSemantic::Enum s = physx::apex::NxRenderBoneSemantic::Enum(i);
				const physx::apex::NxApexRenderSemanticData& semanticData = data.getSemanticData(s);

				if (semanticData.data != NULL)
				{
					mIO->printAndScan(" %s: ", mIO->semanticToString(s));
					mIO->writeArray(semanticData.format, semanticData.stride, numBones, semanticData.data, mManager->getBonePoseTolerance(), physx::apex::NxRenderVertexSemantic::CUSTOM);
					mIO->printAndScan("\n");
				}
			}
		}

		mWriteCalls++;
	}


	unsigned int getId()
	{
		return (physx::PxU32)mBufferId;
	}

protected:
	TextRenderResourceManager* mManager;
	Writer* mIO;
	int mBufferId;
	unsigned int mWriteCalls;
	physx::apex::NxUserRenderBoneBufferDesc mDescriptor;
};



physx::apex::NxUserRenderBoneBuffer* TextRenderResourceManager::createBoneBuffer(const physx::apex::NxUserRenderBoneBufferDesc& desc)
{
	physx::apex::NxUserRenderBoneBuffer* bb = new TextUserRenderBoneBuffer(desc, this, mIO, mBoneBufferCount++);
	return bb;
}



void TextRenderResourceManager::releaseBoneBuffer(physx::apex::NxUserRenderBoneBuffer& buffer)
{
	delete &buffer;
}















class TextUserRenderInstanceBuffer : public physx::apex::NxUserRenderInstanceBuffer
{
public:
	TextUserRenderInstanceBuffer(const physx::apex::NxUserRenderInstanceBufferDesc& desc, TextRenderResourceManager* manager, Writer* readerWriter, int bufferId) : mManager(manager), mIO(readerWriter), mDescriptor(desc)
	{
		mBufferId = bufferId;

		if (mManager->getVerbosity() >= 1)
		{
			mIO->printAndScan("InstanceBuffer[%d]::create\n", mBufferId);
		}
	}

	~TextUserRenderInstanceBuffer()
	{
		if (mManager->getVerbosity() >= 1)
		{
			mIO->printAndScan("InstanceBuffer[%d]::destroy\n", mBufferId);
		}
	}

	virtual void writeBuffer(const void* /*data*/, physx::PxU32 /*firstInstance*/, physx::PxU32 /*numInstances*/)
	{
		if (mManager->getVerbosity() >= 2)
		{
			mIO->printAndScan("InstanceBuffer[%d]::writeBuffer\n", mBufferId);

		}
	}

	unsigned int getId()
	{
		return (physx::PxU32)mBufferId;
	}

protected:
	TextRenderResourceManager* mManager;
	Writer* mIO;
	FILE* mOutputFile;
	int mBufferId;
	physx::apex::NxUserRenderInstanceBufferDesc mDescriptor;
};



physx::apex::NxUserRenderInstanceBuffer* TextRenderResourceManager::createInstanceBuffer(const physx::apex::NxUserRenderInstanceBufferDesc& desc)
{
	physx::apex::NxUserRenderInstanceBuffer* ib = new TextUserRenderInstanceBuffer(desc, this, mIO, mInstanceBufferCount++);
	return ib;
}



void TextRenderResourceManager::releaseInstanceBuffer(physx::apex::NxUserRenderInstanceBuffer& buffer)
{
	delete &buffer;
}




















class TextUserRenderSpriteBuffer : public physx::apex::NxUserRenderSpriteBuffer
{
public:
	TextUserRenderSpriteBuffer(const physx::apex::NxUserRenderSpriteBufferDesc& desc, TextRenderResourceManager* manager, Writer* readerWriter, int bufferId) : mManager(manager), mIO(readerWriter), mDescriptor(desc)
	{
		mBufferId = bufferId;

		if (mManager->getVerbosity() >= 1)
		{
			mIO->printAndScan("SpriteBuffer[%d]::create\n", mBufferId);
		}
	}

	~TextUserRenderSpriteBuffer()
	{
		if (mManager->getVerbosity() >= 1)
		{
			mIO->printAndScan("SpriteBuffer[%d]::destroy\n", mBufferId);
		}
	}

	virtual void writeBuffer(const void* /*data*/, physx::PxU32 /*firstSprite*/, physx::PxU32 /*numSprites*/)
	{
		if (mManager->getVerbosity() >= 2)
		{
			mIO->printAndScan("SpriteBuffer[%d]::writeBuffer\n", mBufferId);
		}
	}


	unsigned int getId()
	{
		return (physx::PxU32)mBufferId;
	}

protected:
	TextRenderResourceManager* mManager;
	Writer* mIO;
	FILE* mOutputFile;
	int mBufferId;
	physx::apex::NxUserRenderSpriteBufferDesc mDescriptor;
};



physx::apex::NxUserRenderSpriteBuffer* TextRenderResourceManager::createSpriteBuffer(const physx::apex::NxUserRenderSpriteBufferDesc& desc)
{
	physx::apex::NxUserRenderSpriteBuffer* sb = new TextUserRenderSpriteBuffer(desc, this, mIO, mSpriteBufferCount++);
	return sb;
}



void TextRenderResourceManager::releaseSpriteBuffer(physx::apex::NxUserRenderSpriteBuffer& buffer)
{
	delete &buffer;
}





class TextUserRenderSurfaceBuffer : public physx::apex::NxUserRenderSurfaceBuffer
{
public:
	TextUserRenderSurfaceBuffer(const physx::apex::NxUserRenderSurfaceBufferDesc& desc, TextRenderResourceManager* manager, Writer* readerWriter, int bufferId) : mManager(manager), mIO(readerWriter), mDescriptor(desc)
	{
		mBufferId = bufferId;

		if (mManager->getVerbosity() >= 1)
		{
			mIO->printAndScan("SurfaceBuffer[%d]::create\n", mBufferId);
		}
	}

	~TextUserRenderSurfaceBuffer()
	{
		if (mManager->getVerbosity() >= 1)
		{
			mIO->printAndScan("SurfaceBuffer[%d]::destroy\n", mBufferId);
		}
	}

	virtual void writeBuffer(const void* /*srcData*/, physx::PxU32 /*srcPitch*/, physx::PxU32 /*srcHeight*/, physx::PxU32 /*dstX*/, physx::PxU32 /*dstY*/, physx::PxU32 /*dstZ*/, physx::PxU32 /*width*/, physx::PxU32 /*height*/, physx::PxU32 /*depth*/)
	{
		if (mManager->getVerbosity() >= 2)
		{
			mIO->printAndScan("SurfaceBuffer[%d]::writeBuffer\n", mBufferId);
		}
	}


	unsigned int getId()
	{
		return (physx::PxU32)mBufferId;
	}

protected:
	TextRenderResourceManager* mManager;
	Writer* mIO;
	FILE* mOutputFile;
	int mBufferId;
	physx::apex::NxUserRenderSurfaceBufferDesc mDescriptor;
};


physx::apex::NxUserRenderSurfaceBuffer*	 TextRenderResourceManager::createSurfaceBuffer( const physx::apex::NxUserRenderSurfaceBufferDesc &desc )
{
	physx::apex::NxUserRenderSurfaceBuffer* sb = new TextUserRenderSurfaceBuffer(desc, this, mIO, mSurfaceBufferCount++);
	return sb;
}



void TextRenderResourceManager::releaseSurfaceBuffer( physx::apex::NxUserRenderSurfaceBuffer &buffer )
{
	delete &buffer;
}











class TextUserRenderResource : public physx::apex::NxUserRenderResource
{
public:
	TextUserRenderResource(const physx::apex::NxUserRenderResourceDesc& desc, TextRenderResourceManager* manager, Writer* readerWriter, int bufferId) : mManager(manager), mIO(readerWriter), mRenderCount(0), mDescriptor(desc)
	{
		mBufferId = bufferId;

		if (mManager->getVerbosity() >= 1)
		{
			mIO->printAndScan("RenderResource[%d]::create(", mBufferId);

			for (unsigned int i = 0; i < mDescriptor.numVertexBuffers; i++)
			{
				TextUserRenderVertexBuffer* vb = static_cast<TextUserRenderVertexBuffer*>(mDescriptor.vertexBuffers[i]);
				mIO->writeElem("VertexBuffer", vb->getId());
			}

			if (mDescriptor.indexBuffer != NULL)
			{
				TextUserRenderIndexBuffer* ib = static_cast<TextUserRenderIndexBuffer*>(mDescriptor.indexBuffer);
				mIO->writeElem("IndexBuffer", ib->getId());
			}

			if (mDescriptor.boneBuffer != NULL)
			{
				TextUserRenderBoneBuffer* bb = static_cast<TextUserRenderBoneBuffer*>(mDescriptor.boneBuffer);
				mIO->writeElem("BoneBuffer", bb->getId());
			}

			if (mDescriptor.instanceBuffer != NULL)
			{
				TextUserRenderInstanceBuffer* ib = static_cast<TextUserRenderInstanceBuffer*>(mDescriptor.instanceBuffer);
				mIO->writeElem("InstanceBuffer", ib->getId());
			}

			if (mDescriptor.spriteBuffer != NULL)
			{
				TextUserRenderSpriteBuffer* sp = static_cast<TextUserRenderSpriteBuffer*>(mDescriptor.spriteBuffer);
				mIO->writeElem("SpriteBuffer", sp->getId());
			}

			{
				unsigned int materialId = manager->material2Id(mDescriptor.material);
				mIO->writeElem("Material", materialId);
			}

			WRITE_DESC_ELEM(numVertexBuffers);
			WRITE_DESC_ELEM(firstVertex);
			WRITE_DESC_ELEM(numVerts);
			WRITE_DESC_ELEM(firstIndex);
			WRITE_DESC_ELEM(numIndices);
			WRITE_DESC_ELEM(firstBone);
			WRITE_DESC_ELEM(numBones);
			WRITE_DESC_ELEM(firstInstance);
			WRITE_DESC_ELEM(numInstances);
			WRITE_DESC_ELEM(firstSprite);
			WRITE_DESC_ELEM(numSprites);
			WRITE_DESC_ELEM(submeshIndex);
			WRITE_DESC_ELEM(cullMode);
			WRITE_DESC_ELEM(primitives);

			mIO->printAndScan(")\n");
		}
	}

	~TextUserRenderResource()
	{
		if (mManager->getVerbosity() >= 1)
		{
			mIO->printAndScan("RenderResource[%d]::destroy\n", mBufferId);
		}
	}



	void setVertexBufferRange(unsigned int firstVertex, unsigned int numVerts)
	{
		mDescriptor.firstVertex = firstVertex;
		mDescriptor.numVerts = numVerts;

		if (mManager->getVerbosity() >= 2)
		{
			mIO->printAndScan("RenderResource[%d]", mBufferId);
			mIO->printAndScan("::setVertexBufferRange(firstVertex=%d", (int)firstVertex);
			mIO->printAndScan(" numVerts=%d)\n", (int)numVerts);
		}
	}



	void setIndexBufferRange(unsigned int firstIndex, unsigned int numIndices)
	{
		mDescriptor.firstIndex = firstIndex;
		mDescriptor.numIndices = numIndices;

		if (mManager->getVerbosity() >= 2)
		{
			mIO->printAndScan("RenderResource[%d]", mBufferId);
			mIO->printAndScan("::setIndexBufferRange(firstIndex=%d ", (int)firstIndex);
			mIO->printAndScan("numIndices=%d)\n", (int)numIndices);
		}
	}



	void setBoneBufferRange(unsigned int firstBone, unsigned int numBones)
	{
		mDescriptor.firstBone = firstBone;
		mDescriptor.numBones = numBones;

		if (mManager->getVerbosity() >= 2)
		{
			mIO->printAndScan("RenderResource[%d]", mBufferId);
			mIO->printAndScan("::setBoneBufferRange(firstBone=%d ", (int)firstBone);
			mIO->printAndScan("numBones=%d)\n", (int)numBones);
		}
	}



	void setInstanceBufferRange(unsigned int firstInstance, unsigned int numInstances)
	{
		mDescriptor.firstInstance = firstInstance;
		mDescriptor.numInstances = numInstances;

		if (mManager->getVerbosity() >= 2)
		{
			mIO->printAndScan("RenderResource[%d]", mBufferId);
			mIO->printAndScan("::setInstanceBufferRange(firstInstance=%d ", (int)firstInstance);
			mIO->printAndScan("numInstances=%d)\n", (int)numInstances);
		}
	}



	void setSpriteBufferRange(unsigned int firstSprite, unsigned int numSprites)
	{
		mDescriptor.firstSprite = firstSprite;
		mDescriptor.numSprites = numSprites;

		if (mManager->getVerbosity() >= 2)
		{
			mIO->printAndScan("RenderResource[%d]", mBufferId);
			mIO->printAndScan("(firstSprite=%d ", (int)firstSprite);
			mIO->printAndScan("numSprites=%d)\n", (int)numSprites);
		}
	}



	void setMaterial(void* material)
	{
		mDescriptor.material = material;

		if (mManager->getVerbosity() >= 2 && material != mDescriptor.material)
		{
			mIO->printAndScan("RenderResource[%d]", mBufferId);
			mIO->printAndScan("::setMaterial(material=%d)\n", (int)mManager->material2Id(material));
		}
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



	void render(const physx::apex::NxApexRenderContext& context)
	{
		if (mManager->getVerbosity() >= 2)
		{
			mIO->printAndScan("RenderResource[%d]", mBufferId);
			mIO->printAndScan("::render#%d(", (int)mRenderCount);

			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					mIO->printAndScan(mManager->getRenderPoseTolerance(), physx::apex::NxRenderVertexSemantic::CUSTOM, "%f ", context.local2world[i][j]);
				}
			}

			mIO->printAndScan(")\n");
		}

		mRenderCount++;
	}

protected:
	TextRenderResourceManager* mManager;
	Writer* mIO;
	int mBufferId;
	unsigned int mRenderCount;
	physx::apex::NxUserRenderResourceDesc mDescriptor;
};



physx::apex::NxUserRenderResource* TextRenderResourceManager::createResource(const physx::apex::NxUserRenderResourceDesc& desc)
{
	physx::apex::NxUserRenderResource* rr = new TextUserRenderResource(desc, this, mIO, mRenderResourceCount++);
	return rr;
}



void TextRenderResourceManager::releaseResource(physx::apex::NxUserRenderResource& resource)
{
	delete &resource;
}



unsigned int TextRenderResourceManager::getMaxBonesForMaterial(void* /*material*/)
{
	return 0;
}



void TextUserRenderer::renderResource(const physx::apex::NxApexRenderContext& context)
{
	TextUserRenderResource* rr = static_cast<TextUserRenderResource*>(context.renderResource);
	rr->render(context);
}
