/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef TEXT_RENDER_RESOURCE_MANAGER_H
#define TEXT_RENDER_RESOURCE_MANAGER_H

#include "NxUserRenderVertexBuffer.h"
#include "NxUserRenderIndexBuffer.h"
#include "NxUserRenderIndexBufferDesc.h"
#include "NxUserRenderBoneBuffer.h"
#include "NxUserRenderInstanceBuffer.h"
#include "NxUserRenderSpriteBuffer.h"
#include "NxUserRenderSurfaceBuffer.h"

#include "NxUserRenderResourceManager.h"
#include "NxUserRenderer.h"

#include <stdio.h>
#include <map>

class Writer
{
public:
	Writer(FILE* outputFile);
	virtual ~Writer();

	virtual void printAndScan(const char* format);
	virtual void printAndScan(const char* format, const char* arg);
	virtual void printAndScan(const char* format, int arg);
	virtual void printAndScan(float tolerance, physx::apex::NxRenderVertexSemantic::Enum s, const char* format, float arg);

	const char* semanticToString(physx::apex::NxRenderVertexSemantic::Enum semantic);
	const char* semanticToString(physx::apex::NxRenderBoneSemantic::Enum semantic);

	void writeElem(const char* name, unsigned int val);
	void writeArray(physx::apex::NxRenderDataFormat::Enum format, unsigned int stride, unsigned int numElements, const void* data, float tolerance, physx::apex::NxRenderVertexSemantic::Enum s);

protected:
	FILE* mReferenceFile;
	FILE* mOutputFile;
	bool mIsStdout;
};



class TextRenderResourceManager : public physx::apex::NxUserRenderResourceManager
{
protected:
	TextRenderResourceManager();

public:

	TextRenderResourceManager(int verbosity, const char* outputFilename);
	~TextRenderResourceManager();

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

	unsigned int									 material2Id(void* material);

	void											 setVerbosity(int v) { mVerbosity = v; }
	int												 getVerbosity() { return mVerbosity; }

	virtual float									 getVBTolerance(physx::apex::NxRenderVertexSemantic::Enum /*s*/) 
	{ 
		return 0.0f; 
	}
	virtual float									 getRenderPoseTolerance() { return 0.0f; }
	virtual float									 getBonePoseTolerance() { return 0.0f; }

	virtual bool									getSpriteLayoutData(physx::PxU32 spriteCount, 
																		physx::PxU32 spriteSemanticsBitmap, 
																		physx::apex::NxUserRenderSpriteBufferDesc* bufferDesc);
	virtual bool									getInstanceLayoutData(physx::PxU32 particleCount, 
																		physx::PxU32 particleSemanticsBitmap, 
																		physx::apex::NxUserRenderInstanceBufferDesc* bufferDesc);
protected:

	int mVerbosity;
	FILE* mOutputFile;
	Writer* mIO;

	std::map<void*, unsigned int> mMaterial2Id;

	int mVertexBufferCount;
	int mIndexBufferCount;
	int mBoneBufferCount;
	int mInstanceBufferCount;
	int mSpriteBufferCount;
	int mRenderResourceCount;
	int mSurfaceBufferCount;
};



class TextUserRenderer : public physx::apex::NxUserRenderer
{
public:
	TextUserRenderer() {}
	virtual ~TextUserRenderer() {}

	virtual void renderResource(const physx::apex::NxApexRenderContext& context);
};

#endif // TEXT_RENDER_RESOURCE_MANAGER_H
