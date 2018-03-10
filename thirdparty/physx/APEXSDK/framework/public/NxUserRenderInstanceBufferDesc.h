/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_USER_RENDER_INSTANCE_BUFFER_DESC_H
#define NX_USER_RENDER_INSTANCE_BUFFER_DESC_H

/*!
\file
\brief class NxUserRenderInstanceBufferDesc, structs NxRenderDataFormat and NxRenderInstanceSemantic
*/

#include "NxUserRenderResourceManager.h"
#include "NxApexRenderDataFormat.h"

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
\brief Enumerates the potential instance buffer semantics
*/
struct NxRenderInstanceSemantic
{
	/**
	\brief Enume of the potential instance buffer semantics
	*/
	enum Enum
	{
		POSITION = 0,	//!< position of instance
		ROTATION_SCALE,	//!< rotation matrix and scale baked together
		VELOCITY_LIFE,	//!< velocity and life remain (1.0=new .. 0.0=dead) baked together 
		DENSITY,		//!< particle density at instance location
		COLOR,			//!< color of instance
		UV_OFFSET,		//!< an offset to apply to all U,V coordinates
		LOCAL_OFFSET,	//!< the static initial position offset of the instance

		USER_DATA,		//!< User data - 32 bits

		POSE,			//!< pose matrix

		NUM_SEMANTICS	//!< Count of semantics, not a valid semantic.
	};
};

/**
\brief potential semantics of a sprite buffer
*/
struct NxRenderInstanceLayoutElement
{
	/**
	\brief Enum of sprite buffer semantics types
	*/
	enum Enum
	{
		POSITION_FLOAT3,
		ROTATION_SCALE_FLOAT3x3,
		VELOCITY_LIFE_FLOAT4,
		DENSITY_FLOAT1,
		COLOR_RGBA8,
		COLOR_BGRA8,
		COLOR_FLOAT4,
		UV_OFFSET_FLOAT2,
		LOCAL_OFFSET_FLOAT3,
		USER_DATA_UINT1,
		POSE_FLOAT3x4,

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
		case ROTATION_SCALE_FLOAT3x3:
			return NxRenderDataFormat::FLOAT3x3;
		case VELOCITY_LIFE_FLOAT4:
			return NxRenderDataFormat::FLOAT4;
		case DENSITY_FLOAT1:
			return NxRenderDataFormat::FLOAT1;
		case COLOR_RGBA8:
			return NxRenderDataFormat::R8G8B8A8;
		case COLOR_BGRA8:
			return NxRenderDataFormat::B8G8R8A8;
		case COLOR_FLOAT4:
			return NxRenderDataFormat::FLOAT4;
		case UV_OFFSET_FLOAT2:
			return NxRenderDataFormat::FLOAT2;
		case LOCAL_OFFSET_FLOAT3:
			return NxRenderDataFormat::FLOAT3;
		case USER_DATA_UINT1:
			return NxRenderDataFormat::UINT1;
		case POSE_FLOAT3x4:
			return NxRenderDataFormat::FLOAT3x4;
		default:
			PX_ALWAYS_ASSERT();
			return NxRenderDataFormat::NUM_FORMATS;
		}
	}
/**
	\brief Get semantic from layout element format
	*/
	static PX_INLINE NxRenderInstanceSemantic::Enum getSemantic(Enum semantic)
	{
		switch (semantic)
		{
		case POSITION_FLOAT3:
			return NxRenderInstanceSemantic::POSITION;		
		case ROTATION_SCALE_FLOAT3x3:
			return NxRenderInstanceSemantic::ROTATION_SCALE;
		case VELOCITY_LIFE_FLOAT4:
			return NxRenderInstanceSemantic::VELOCITY_LIFE;
		case DENSITY_FLOAT1:
			return NxRenderInstanceSemantic::DENSITY;
		case COLOR_RGBA8:
		case COLOR_BGRA8:
		case COLOR_FLOAT4:
			return NxRenderInstanceSemantic::COLOR;
		case UV_OFFSET_FLOAT2:
			return NxRenderInstanceSemantic::UV_OFFSET;
		case LOCAL_OFFSET_FLOAT3:
			return NxRenderInstanceSemantic::LOCAL_OFFSET;
		case USER_DATA_UINT1:
			return NxRenderInstanceSemantic::USER_DATA;
		case POSE_FLOAT3x4:
			return NxRenderInstanceSemantic::POSE;
		default:
			PX_ALWAYS_ASSERT();
			return NxRenderInstanceSemantic::NUM_SEMANTICS;
		}
	}
};

/**
\brief Describes the data and layout of an instance buffer
*/
class NxUserRenderInstanceBufferDesc
{
public:
	NxUserRenderInstanceBufferDesc(void)
	{
		setDefaults();
	}

	/**
	\brief Default values
	*/
	void setDefaults()
	{
		registerInCUDA = false;
		interopContext = 0;
		maxInstances = 0;
		hint         = NxRenderBufferHint::STATIC;
		for (physx::PxU32 i = 0; i < NxRenderInstanceLayoutElement::NUM_SEMANTICS; i++)
		{
			semanticOffsets[i] = physx::PxU32(-1);
		}
		stride = 0;
	}

	/**
	\brief Check if parameter's values are correct
	*/
	bool isValid(void) const
	{
		physx::PxU32 numFailed = 0;

		numFailed += (maxInstances == 0);
		numFailed += (stride == 0);
		numFailed += (semanticOffsets[NxRenderInstanceLayoutElement::POSITION_FLOAT3] == physx::PxU32(-1))
			&& (semanticOffsets[NxRenderInstanceLayoutElement::POSE_FLOAT3x4] == physx::PxU32(-1));
		numFailed += registerInCUDA && (interopContext == 0);

		numFailed += ((stride & 0x03) != 0);
		for (physx::PxU32 i = 0; i < NxRenderInstanceLayoutElement::NUM_SEMANTICS; i++)
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
	bool isTheSameAs(const NxUserRenderInstanceBufferDesc& other) const
	{
		if (registerInCUDA != other.registerInCUDA) return false;
		if (maxInstances != other.maxInstances) return false;
		if (hint != other.hint) return false;

		if (stride != other.stride) return false;
		for (physx::PxU32 i = 0; i < NxRenderInstanceLayoutElement::NUM_SEMANTICS; i++)
		{
			if (semanticOffsets[i] != other.semanticOffsets[i]) return false;
		}
		return true;
	}

public:
	physx::PxU32					maxInstances; //!< The maximum amount of instances this buffer will ever hold.
	NxRenderBufferHint::Enum		hint; //!< Hint on how often this buffer is updated.
	
	/**
	\brief Array of the corresponding offsets (in bytes) for each semantic.
	*/
	physx::PxU32					semanticOffsets[NxRenderInstanceLayoutElement::NUM_SEMANTICS];

	physx::PxU32					stride; //!< The stride between instances of this buffer. Required when CUDA interop is used!

	bool							registerInCUDA; //!< Declare if the resource must be registered in CUDA upon creation

	/**
	This context must be used to register and unregister the resource every time the
	device is lost and recreated.
	*/
	physx::PxCudaContextManager*   interopContext;
};

#if !defined(PX_PS4)
	#pragma warning(pop)
#endif	//!PX_PS4

PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_USER_RENDER_INSTANCE_BUFFER_DESC_H
