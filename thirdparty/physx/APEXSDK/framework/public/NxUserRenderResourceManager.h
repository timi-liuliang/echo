/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_USER_RENDER_RESOURCE_MANAGER_H
#define NX_USER_RENDER_RESOURCE_MANAGER_H

/*!
\file
\brief class NxUserRenderResourceManager, structs NxRenderPrimitiveType, NxRenderBufferHint, and NxRenderCullMode
*/

#include "NxApexUsingNamespace.h"

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

class NxUserRenderVertexBuffer;
class NxUserRenderVertexBufferDesc;
class NxUserRenderIndexBuffer;
class NxUserRenderIndexBufferDesc;
class NxUserRenderSurfaceBuffer;
class NxUserRenderSurfaceBufferDesc;
class NxUserRenderBoneBuffer;
class NxUserRenderBoneBufferDesc;
class NxUserRenderInstanceBuffer;
class NxUserRenderInstanceBufferDesc;
class NxUserRenderSpriteBuffer;
class NxUserRenderSpriteBufferDesc;
class NxUserRenderResource;
class NxUserRenderResourceDesc;
class NxUserOpaqueMesh;
class NxUserOpaqueMeshDesc;

class NxUserRenderSpriteTextureDesc;

/**
\brief Describe the implied vertex ordering
*/
struct NxRenderPrimitiveType
{
	/**
	\brief Enum of the implied vertex ordering types
	*/
	enum Enum
	{
		UNKNOWN = 0,

		TRIANGLES,
		TRIANGLE_STRIP,

		LINES,
		LINE_STRIP,

		POINTS,
		POINT_SPRITES,
	};
};

/**
\brief Possible triangle culling modes
*/
struct NxRenderCullMode
{
	/**
	\brief Enum of possible triangle culling mode types
	*/
	enum Enum
	{
		CLOCKWISE = 0,
		COUNTER_CLOCKWISE,
		NONE
	};
};

/**
\brief Hint of the buffer data lifespan
*/
struct NxRenderBufferHint
{
	/**
	\brief Enum of hints of the buffer data lifespan
	*/
	enum Enum
	{
		STATIC = 0,
		DYNAMIC,
		STREAMING,
	};
};

/**
\brief User defined renderable resource manager

A render resource manager is an object that creates and manages renderable resources...
This is given to the APEX SDK at creation time via the descriptor and must be persistent through the lifetime
of the SDK.
*/
class NxUserRenderResourceManager
{
public:
	virtual								~NxUserRenderResourceManager() {}

	/**
		The create methods in this class will only be called from the context of an NxApexRenderable::updateRenderResources()
		call, but the release methods can be triggered by any APEX API call that deletes an NxApexActor.  It is up to
		the end-user to make the release methods thread safe.
	*/

	virtual NxUserRenderVertexBuffer*   createVertexBuffer(const NxUserRenderVertexBufferDesc& desc)     = 0;
	/** \brief Release vertex buffer */
	virtual void                        releaseVertexBuffer(NxUserRenderVertexBuffer& buffer)            = 0;

	/** \brief Create index buffer */
	virtual NxUserRenderIndexBuffer*    createIndexBuffer(const NxUserRenderIndexBufferDesc& desc)       = 0;
	/** \brief Release index buffer */
	virtual void                        releaseIndexBuffer(NxUserRenderIndexBuffer& buffer)              = 0;

	/** \brief Create bone buffer */
	virtual NxUserRenderBoneBuffer*     createBoneBuffer(const NxUserRenderBoneBufferDesc& desc)         = 0;
	/** \brief Release bone buffer */
	virtual void                        releaseBoneBuffer(NxUserRenderBoneBuffer& buffer)                = 0;

	/** \brief Create instance buffer */
	virtual NxUserRenderInstanceBuffer* createInstanceBuffer(const NxUserRenderInstanceBufferDesc& desc) = 0;
	/** \brief Release instance buffer */
	virtual void                        releaseInstanceBuffer(NxUserRenderInstanceBuffer& buffer)        = 0;

	/** \brief Create sprite buffer */
	virtual NxUserRenderSpriteBuffer*   createSpriteBuffer(const NxUserRenderSpriteBufferDesc& desc)     = 0;
	/** \brief Release sprite buffer */
	virtual void                        releaseSpriteBuffer(NxUserRenderSpriteBuffer& buffer)            = 0;

	/** \brief Create surface buffer */
	virtual NxUserRenderSurfaceBuffer*  createSurfaceBuffer(const NxUserRenderSurfaceBufferDesc& desc)   = 0;
	/** \brief Release surface buffer */
	virtual void                        releaseSurfaceBuffer(NxUserRenderSurfaceBuffer& buffer)          = 0;

	/** \brief Create resource */
	virtual NxUserRenderResource*       createResource(const NxUserRenderResourceDesc& desc)             = 0;

	/**
	releaseResource() should not release any of the included buffer pointers.  Those free methods will be
	called separately by the APEX SDK before (or sometimes after) releasing the NxUserRenderResource.
	*/
	virtual void                        releaseResource(NxUserRenderResource& resource)                  = 0;

	/**
	Get the maximum number of bones supported by a given material. Return 0 for infinite.
	For optimal rendering, do not limit the bone count (return 0 from this function).
	*/
	virtual physx::PxU32                       getMaxBonesForMaterial(void* material)                             = 0;


	/** \brief Get the sprite layout data 
		Returns true in case textureDescArray is set.
		In case user is not interested in setting specific layout for sprite PS,
		this function should return false. 
	*/
	virtual bool getSpriteLayoutData(physx::PxU32 spriteCount, physx::PxU32 spriteSemanticsBitmap, NxUserRenderSpriteBufferDesc* textureDescArray) = 0;

	/** \brief Get the instance layout data 
		Returns true in case textureDescArray is set.
		In case user is not interested in setting specific layout for sprite PS,
		this function should return false. 
	*/
	virtual bool getInstanceLayoutData(physx::PxU32 spriteCount, physx::PxU32 spriteSemanticsBitmap, NxUserRenderInstanceBufferDesc* instanceDescArray) = 0;

};

PX_POP_PACK

}
} // end namespace physx::apex

#endif
