/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_USER_RENDER_RESOURCE_DESC_H
#define NX_USER_RENDER_RESOURCE_DESC_H

/*!
\file
\brief class NxUserRenderResourceDesc
*/

#include "NxUserRenderResourceManager.h"
#include "foundation/PxAssert.h"

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

class NxApexRenderable;
class NxUserOpaqueMesh;
class NxUserRenderVertexBuffer;
class NxUserRenderIndexBuffer;
class NxUserRenderBoneBuffer;
class NxUserRenderInstanceBuffer;
class NxUserRenderSpriteBuffer;

/**
\brief Describes all the data that makes up a renderable resource
*/
class NxUserRenderResourceDesc
{
public:
	NxUserRenderResourceDesc(void)
	{
		firstVertex      = 0;
		numVerts         = 0;

		indexBuffer      = 0;
		firstIndex       = 0;
		numIndices       = 0;

		boneBuffer       = 0;
		firstBone        = 0;
		numBones         = 0;

		instanceBuffer   = 0;
		firstInstance    = 0;
		numInstances     = 0;

		spriteBuffer     = 0;
		firstSprite      = 0;
		numSprites       = 0;
		visibleSpriteCount = 0;

		material         = 0;
		submeshIndex	 = 0;

		userRenderData   = 0;

		numVertexBuffers = 0;
		vertexBuffers     = NULL;

		cullMode         = NxRenderCullMode::CLOCKWISE;
		primitives       = NxRenderPrimitiveType::UNKNOWN;

		opaqueMesh		= NULL;
	}

	/**
	\brief Checks if the resource is valid
	*/
	bool isValid(void) const
	{
		physx::PxU32 numFailed = 0;
		if (numVertexBuffers >= 255)
		{
			numFailed++;
		}
		if (numIndices   && !indexBuffer)
		{
			numFailed++;
		}
		if (numBones     && !boneBuffer)
		{
			numFailed++;
		}
		if (numInstances && !instanceBuffer)
		{
			numFailed++;
		}
		if (numSprites   && !spriteBuffer)
		{
			numFailed++;
		}
		PX_ASSERT(numFailed == 0);
		return numFailed == 0;
	}

public:
	NxUserOpaqueMesh* 				opaqueMesh;			//!< A user specified opaque mesh interface.
	NxUserRenderVertexBuffer**		vertexBuffers;		//!< vertex buffers used when rendering this resource.
	//!< there should be no overlap in semantics between any two VBs.
	physx::PxU32					numVertexBuffers;	//!< number of vertex buffers used when rendering this resource.

	physx::PxU32					firstVertex;		//!< First vertex to render
	physx::PxU32					numVerts;			//!< Number of vertices to render

	NxUserRenderIndexBuffer*		indexBuffer;		//!< optional index buffer used when rendering this resource.
	physx::PxU32					firstIndex;			//!< First index to render
	physx::PxU32					numIndices;			//!< Number of indices to render

	NxUserRenderBoneBuffer*			boneBuffer;			//!< optional bone buffer used for skinned meshes.
	physx::PxU32					firstBone;			//!< First bone to render
	physx::PxU32					numBones;			//!< Number of bones to render

	NxUserRenderInstanceBuffer*		instanceBuffer;	//!< optional instance buffer if rendering multiple instances of the same resource.
	physx::PxU32					firstInstance;		//!< First instance to render
	physx::PxU32					numInstances;		//!< Number of instances to render

	NxUserRenderSpriteBuffer*		spriteBuffer;		//!< optional sprite buffer if rendering sprites
	physx::PxU32					firstSprite;		//!< First sprite to render
	physx::PxU32					numSprites;			//!< Number of sprites to render
	physx::PxU32					visibleSpriteCount; //!< If the sprite buffer is using the view direction modifier; this will represent the number of sprites visible in front of the camera (Not necessarily in the frustum but in front of the camera)

	NxUserRenderSurfaceBuffer**		surfaceBuffers;		//!< optional surface buffer for transferring variable to texture	
	physx::PxU32					numSurfaceBuffers;	//!< Number of surface buffers to render
	physx::PxU32					widthSurfaceBuffers;//!< The surface buffer width
	physx::PxU32					heightSurfaceBuffers;//!< The surface buffer height

	void*							material;			//!< user defined material used when rendering this resource.
	physx::PxU32					submeshIndex;		//!< the index of the submesh that render resource belongs to

	//! user defined pointer originally passed in to NxApexRenderable::NxApexupdateRenderResources(..)
	void*							userRenderData;

	NxRenderCullMode::Enum			cullMode;			//!< Triangle culling mode
	NxRenderPrimitiveType::Enum		primitives;			//!< Rendering primitive type (triangle, line strip, etc)
};

PX_POP_PACK

}
} // end namespace physx::apex

#endif
