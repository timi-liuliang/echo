/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_USER_RENDER_SPRITE_BUFFER_DESC_H
#define NX_USER_RENDER_SPRITE_BUFFER_DESC_H

/*!
\file
\brief class NxUserRenderSpriteBufferDesc, structs NxRenderDataFormat and NxRenderSpriteSemantic
*/

#include "NxApexUsingNamespace.h"
#include "NxApexRenderDataFormat.h"
#include "NxUserRenderResourceManager.h"

namespace physx
{
class PxCudaContextManager;
};

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

#if !defined(PX_PS4)
	#pragma warning(push)
	#pragma warning(disable:4121)
#endif	//!PX_PS4

/**
\brief potential semantics of a sprite buffer
*/
struct NxRenderSpriteSemantic
{
	/**
	\brief Enum of sprite buffer semantics types
	*/
	enum Enum
	{
		POSITION = 0,	//!< Position of sprite
		COLOR,			//!< Color of sprite
		VELOCITY,		//!< Linear velocity of sprite
		SCALE,			//!< Scale of sprite
		LIFE_REMAIN,	//!< 1.0 (new) .. 0.0 (dead)
		DENSITY,		//!< Particle density at sprite location
		SUBTEXTURE,		//!< Sub-texture index of sprite
		ORIENTATION,	//!< 2D sprite orientation (angle in radians, CCW in screen plane)

		USER_DATA,		//!< User data - 32 bits (passed from Emitter)

		NUM_SEMANTICS	//!< Count of semantics, not a valid semantic.
	};
};

/**
\brief potential semantics of a sprite buffer
*/
struct NxRenderSpriteLayoutElement
{
	/**
	\brief Enum of sprite buffer semantics types
	*/
	enum Enum
	{
		POSITION_FLOAT3,
		COLOR_RGBA8,
		COLOR_BGRA8,
		COLOR_FLOAT4,
		VELOCITY_FLOAT3,
		SCALE_FLOAT2,
		LIFE_REMAIN_FLOAT1,
		DENSITY_FLOAT1,
		SUBTEXTURE_FLOAT1,
		ORIENTATION_FLOAT1,
		USER_DATA_UINT1,

		NUM_SEMANTICS
	};

	/**
	\brief Get semantic format
	*/
	static PX_INLINE NxRenderDataFormat::Enum getSemanticFormat(Enum semantic)
	{
		switch (semantic)
		{
		case POSITION_FLOAT3:
			return NxRenderDataFormat::FLOAT3;
		case COLOR_RGBA8:
			return NxRenderDataFormat::R8G8B8A8;
		case COLOR_BGRA8:
			return NxRenderDataFormat::B8G8R8A8;
		case COLOR_FLOAT4:
			return NxRenderDataFormat::FLOAT4;
		case VELOCITY_FLOAT3:
			return NxRenderDataFormat::FLOAT3;
		case SCALE_FLOAT2:
			return NxRenderDataFormat::FLOAT2;
		case LIFE_REMAIN_FLOAT1:
			return NxRenderDataFormat::FLOAT1;
		case DENSITY_FLOAT1:
			return NxRenderDataFormat::FLOAT1;
		case SUBTEXTURE_FLOAT1:
			return NxRenderDataFormat::FLOAT1;
		case ORIENTATION_FLOAT1:
			return NxRenderDataFormat::FLOAT1;
		case USER_DATA_UINT1:
			return NxRenderDataFormat::UINT1;
		default:
			PX_ALWAYS_ASSERT();
			return NxRenderDataFormat::NUM_FORMATS;
		}
	}
/**
	\brief Get semantic from layout element format
	*/
	static PX_INLINE NxRenderSpriteSemantic::Enum getSemantic(Enum semantic)
	{
		switch (semantic)
		{
		case POSITION_FLOAT3:
			return NxRenderSpriteSemantic::POSITION;
		case COLOR_RGBA8:
		case COLOR_BGRA8:
		case COLOR_FLOAT4:
			return NxRenderSpriteSemantic::COLOR;
		case VELOCITY_FLOAT3:
			return NxRenderSpriteSemantic::VELOCITY;
		case SCALE_FLOAT2:
			return NxRenderSpriteSemantic::SCALE;
		case LIFE_REMAIN_FLOAT1:
			return NxRenderSpriteSemantic::LIFE_REMAIN;
		case DENSITY_FLOAT1:
			return NxRenderSpriteSemantic::DENSITY;
		case SUBTEXTURE_FLOAT1:
			return NxRenderSpriteSemantic::SUBTEXTURE;
		case ORIENTATION_FLOAT1:
			return NxRenderSpriteSemantic::ORIENTATION;
		case USER_DATA_UINT1:
			return NxRenderSpriteSemantic::USER_DATA;
		default:
			PX_ALWAYS_ASSERT();
			return NxRenderSpriteSemantic::NUM_SEMANTICS;
		}
	}
};

/**
\brief Struct for sprite texture layout info
*/
struct NxRenderSpriteTextureLayout
{
	/**
	\brief Enum of sprite texture layout info
	*/
	enum Enum
	{
		NONE = 0,
		POSITION_FLOAT4, //float4(POSITION.x, POSITION.y, POSITION.z, 1)
		SCALE_ORIENT_SUBTEX_FLOAT4, //float4(SCALE.x, SCALE.y, ORIENTATION, SUBTEXTURE)
		COLOR_RGBA8,
		COLOR_BGRA8,
		COLOR_FLOAT4,
		NUM_LAYOUTS
	};

	/**
	\brief Get layout format
	*/
	static PX_INLINE NxRenderDataFormat::Enum getLayoutFormat(Enum layout)
	{
		switch (layout)
		{
		case NONE:
			return NxRenderDataFormat::UNSPECIFIED;
		case POSITION_FLOAT4:
			return NxRenderDataFormat::FLOAT4;
		case SCALE_ORIENT_SUBTEX_FLOAT4:
			return NxRenderDataFormat::FLOAT4;
		case COLOR_RGBA8:
			return NxRenderDataFormat::R8G8B8A8;
		case COLOR_BGRA8:
			return NxRenderDataFormat::B8G8R8A8;
		case COLOR_FLOAT4:
			return NxRenderDataFormat::R32G32B32A32_FLOAT;
		default:
			PX_ALWAYS_ASSERT();
			return NxRenderDataFormat::NUM_FORMATS;
		}
	}

};

/**
\brief Class for storing sprite texture render data
*/
class NxUserRenderSpriteTextureDesc
{
public:
	NxRenderSpriteTextureLayout::Enum layout;	//!< texture layout
	physx::PxU32 width;							//!< texture width
	physx::PxU32 height;						//!< texture height
	physx::PxU32 pitchBytes;					//!< texture pitch bytes
	physx::PxU32 arrayIndex;					//!< array index for array textures or cubemap face index
	physx::PxU32 mipLevel;						//!< mipmap level

public:
	PX_INLINE NxUserRenderSpriteTextureDesc(void)
	{
		layout = NxRenderSpriteTextureLayout::NONE;
		width = 0;
		height = 0;
		pitchBytes = 0;

		arrayIndex = 0;
		mipLevel = 0;
	}

	/**
	\brief Check if this object is the same as other
	*/
	bool isTheSameAs(const NxUserRenderSpriteTextureDesc& other) const
	{
		if (layout != other.layout) return false;
		if (width != other.width) return false;
		if (height != other.height) return false;
		if (pitchBytes != other.pitchBytes) return false;
		if (arrayIndex != other.arrayIndex) return false;
		if (mipLevel != other.mipLevel) return false;
		return true;
	}
};

/**
\brief describes the semantics and layout of a sprite buffer
*/
class NxUserRenderSpriteBufferDesc
{
public:
	/**
	\brief Max number of sprite textures
	*/
	static const physx::PxU32 MAX_SPRITE_TEXTURES = 4;

	NxUserRenderSpriteBufferDesc(void)
	{
		setDefaults();
	}

	/**
	\brief Default values
	*/
	void setDefaults()
	{
		maxSprites = 0;
		hint         = NxRenderBufferHint::STATIC;
		registerInCUDA = false;
		interopContext = 0;

		for (physx::PxU32 i = 0; i < NxRenderSpriteLayoutElement::NUM_SEMANTICS; i++)
		{
			semanticOffsets[i] = static_cast<physx::PxU32>(-1);
		}
		stride = 0;

		textureCount = 0;
	}

	/**
	\brief Checks if data is correct
	*/
	bool isValid(void) const
	{
		physx::PxU32 numFailed = 0;

		numFailed += (maxSprites == 0);
		numFailed += (textureCount == 0) && (stride == 0);
		numFailed += (textureCount == 0) && (semanticOffsets[NxRenderSpriteLayoutElement::POSITION_FLOAT3] == physx::PxU32(-1));
		numFailed += registerInCUDA && (interopContext == 0);

		numFailed += ((stride & 0x03) != 0);
		for (physx::PxU32 i = 0; i < NxRenderSpriteLayoutElement::NUM_SEMANTICS; i++)
		{
			if (semanticOffsets[i] != static_cast<physx::PxU32>(-1))
			{
				numFailed += (semanticOffsets[i] >= stride);
				numFailed += ((semanticOffsets[i] & 0x03) != 0);
			}
		}

		return (numFailed == 0);
	}

	/**
	\brief Check if this object is the same as other
	*/
	bool isTheSameAs(const NxUserRenderSpriteBufferDesc& other) const
	{
		if (registerInCUDA != other.registerInCUDA) return false;
		if (maxSprites != other.maxSprites) return false;
		if (hint != other.hint) return false;
		if (textureCount != other.textureCount) return false;
		if (textureCount == 0)
		{
			if (stride != other.stride) return false;
			for (physx::PxU32 i = 0; i < NxRenderSpriteLayoutElement::NUM_SEMANTICS; i++)
			{
				if (semanticOffsets[i] != other.semanticOffsets[i]) return false;
			}
		}
		else
		{
			for (physx::PxU32 i = 0; i < textureCount; i++)
			{
				if (textureDescs[i].isTheSameAs(other.textureDescs[i]) == false) return false;
			}
		}
		return true;
	}

public:
	physx::PxU32					maxSprites;		//!< The maximum number of sprites that APEX will store in this buffer
	NxRenderBufferHint::Enum		hint;			//!< A hint about the update frequency of this buffer

	/**
	\brief Array of the corresponding offsets (in bytes) for each semantic.
	*/
	physx::PxU32					semanticOffsets[NxRenderSpriteLayoutElement::NUM_SEMANTICS];

	physx::PxU32					stride;			//!< The stride between sprites of this buffer. Required when CUDA interop is used!

	bool							registerInCUDA;  //!< Declare if the resource must be registered in CUDA upon creation

	/**
	This context must be used to register and unregister the resource every time the
	device is lost and recreated.
	*/
	physx::PxCudaContextManager* interopContext;

	physx::PxU32					textureCount;						//!< the number of textures
	NxUserRenderSpriteTextureDesc	textureDescs[MAX_SPRITE_TEXTURES];	//!< an array of texture descriptors
};

#if !defined(PX_PS4)
	#pragma warning(pop)
#endif	//!PX_PS4

PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_USER_RENDER_SPRITE_BUFFER_DESC_H
