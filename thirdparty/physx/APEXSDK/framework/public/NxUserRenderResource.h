/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_USER_RENDER_RESOURCE_H
#define NX_USER_RENDER_RESOURCE_H

/*!
\file
\brief class NxUserRenderResource
*/

#include "NxApexUsingNamespace.h"

namespace physx
{
namespace apex
{

class NxUserRenderVertexBuffer;
class NxUserRenderIndexBuffer;
class NxUserRenderBoneBuffer;
class NxUserRenderInstanceBuffer;
class NxUserRenderSpriteBuffer;


PX_PUSH_PACK_DEFAULT

/**
\brief An abstract interface to a renderable resource
*/
class NxUserRenderResource
{
public:
	virtual ~NxUserRenderResource() {}

	/** \brief Set vertex buffer range */
	virtual void setVertexBufferRange(physx::PxU32 firstVertex, physx::PxU32 numVerts) = 0;
	/** \brief Set index buffer range */
	virtual void setIndexBufferRange(physx::PxU32 firstIndex, physx::PxU32 numIndices) = 0;
	/** \brief Set bone buffer range */
	virtual void setBoneBufferRange(physx::PxU32 firstBone, physx::PxU32 numBones) = 0;
	/** \brief Set instance buffer range */
	virtual void setInstanceBufferRange(physx::PxU32 firstInstance, physx::PxU32 numInstances) = 0;
	/** \brief Set sprite buffer range */
	virtual void setSpriteBufferRange(physx::PxU32 firstSprite, physx::PxU32 numSprites) = 0;
	/** \brief Set sprite visible count */
	virtual void setSpriteVisibleCount(physx::PxU32 visibleCount) { PX_UNUSED(visibleCount); }
	/** \brief Set material */
	virtual void setMaterial(void* material) = 0;

	/** \brief Get number of vertex buffers */
	virtual physx::PxU32				getNbVertexBuffers() const = 0;
	/** \brief Get vertex buffer */
	virtual NxUserRenderVertexBuffer*	getVertexBuffer(physx::PxU32 index) const = 0;
	/** \brief Get index buffer */
	virtual NxUserRenderIndexBuffer*	getIndexBuffer() const = 0;
	/** \brief Get bone buffer */
	virtual NxUserRenderBoneBuffer*		getBoneBuffer() const = 0;
	/** \brief Get instance buffer */
	virtual NxUserRenderInstanceBuffer*	getInstanceBuffer() const = 0;
	/** \brief Get sprite buffer */
	virtual NxUserRenderSpriteBuffer*	getSpriteBuffer() const = 0;
};

PX_POP_PACK

}
} // end namespace physx::apex

#endif
