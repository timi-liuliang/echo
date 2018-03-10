/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_USER_RENDER_SPRITE_BUFFER_H
#define NX_USER_RENDER_SPRITE_BUFFER_H

/*!
\file
\brief class NxUserRenderSpriteBuffer
*/

#include "NxApexRenderBufferData.h"
#include "NxUserRenderSpriteBufferDesc.h"

/**
\brief Cuda graphics resource
*/
typedef struct CUgraphicsResource_st* CUgraphicsResource;

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief Sprite buffer data (deprecated)
*/
class PX_DEPRECATED NxApexRenderSpriteBufferData : public NxApexRenderBufferData<NxRenderSpriteSemantic, NxRenderSpriteSemantic::Enum>, public NxApexModuleSpecificRenderBufferData
{
};

/**
\brief Used for storing per-sprite instance data for rendering.
*/
class NxUserRenderSpriteBuffer
{
public:
	virtual		~NxUserRenderSpriteBuffer() {}

	/**
	\brief Called when APEX wants to update the contents of the sprite buffer.

	The source data type is assumed to be the same as what was defined in the descriptor.
	APEX should call this function and supply data for ALL semantics that were originally
	requested during creation every time its called.

	\param [in] data				Contains the source data for the sprite buffer.
	\param [in] firstSprite			first sprite to start writing to.
	\param [in] numSprites			number of vertices to write.
	*/
	virtual void writeBuffer(const void* data, physx::PxU32 firstSprite, physx::PxU32 numSprites)
	{
		PX_UNUSED(data);
		PX_UNUSED(firstSprite);
		PX_UNUSED(numSprites);
	}

	///Get the low-level handle of the buffer resource
	///\return true if succeeded, false otherwise
	virtual bool getInteropResourceHandle(CUgraphicsResource& handle)
#if NX_APEX_DEFAULT_NO_INTEROP_IMPLEMENTATION
	{
		PX_UNUSED(&handle);
		return false;
	}
#else
	= 0;
#endif

	/**
	\brief Get interop texture handle list
	*/
	virtual bool getInteropTextureHandleList(CUgraphicsResource* handleList)
	{
		PX_UNUSED(handleList);
		return false;
	}

	/**
	\brief Write data to the texture
	*/
	virtual void writeTexture(physx::PxU32 textureId, physx::PxU32 numSprites, const void* srcData, size_t srcSize)
	{
		PX_UNUSED(textureId);
		PX_UNUSED(numSprites);
		PX_UNUSED(srcData);
		PX_UNUSED(srcSize);

		PX_ALWAYS_ASSERT();
	}
};

PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_USER_RENDER_SPRITE_BUFFER_H
