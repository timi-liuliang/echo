/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_USER_RENDER_VERTEX_BUFFER_DESC_H
#define NX_USER_RENDER_VERTEX_BUFFER_DESC_H

/*!
\file
\brief class NxUserRenderVertexBufferDesc, structs NxRenderDataFormat and NxRenderVertexSemantic
*/

#include "NxApexUsingNamespace.h"
#include "NxUserRenderResourceManager.h"
#include "NxApexRenderDataFormat.h"
#include "NxApexSDK.h"

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief Potential semantics of a vertex buffer
*/
struct NxRenderVertexSemantic
{
	/**
	\brief Enum of vertex buffer semantics types
	*/
	enum Enum
	{
		CUSTOM = -1,			//!< User-defined

		POSITION = 0,			//!< Position of vertex
		NORMAL,					//!< Normal at vertex
		TANGENT,				//!< Tangent at vertex
		BINORMAL,				//!< Binormal at vertex
		COLOR,					//!< Color at vertex
		TEXCOORD0,				//!< Texture coord 0 of vertex
		TEXCOORD1,				//!< Texture coord 1 of vertex
		TEXCOORD2,				//!< Texture coord 2 of vertex
		TEXCOORD3,				//!< Texture coord 3 of vertex
		BONE_INDEX,				//!< Bone index of vertex
		BONE_WEIGHT,			//!< Bone weight of vertex

		DISPLACEMENT_TEXCOORD,	//!< X Displacement map texture coord of vertex
		DISPLACEMENT_FLAGS,		//!< Displacement map flags of vertex

		NUM_SEMANTICS			//!< Count of standard semantics, not a valid semantic
	};
};

/**
\brief Texture UV direction convention
*/
struct NxTextureUVOrigin
{
	/**
	\brief Enum of texture UV directions types
	*/
	enum Enum
	{
		ORIGIN_TOP_LEFT = 0,		//!< Origin is at top left
		ORIGIN_TOP_RIGHT = 1,		//!< Origin is at top right
		ORIGIN_BOTTOM_LEFT = 2,		//!< Origin is at bottom left
		ORIGIN_BOTTOM_RIGHT = 3,	//!< Origin is at bottom right

		ORIGIN_UNKNOWN = 0xFFFFFFFF	//!< Unknown origin
	};
};


/**
\brief Describes the semantics and layout of a vertex buffer
*/
class NxUserRenderVertexBufferDesc
{
public:
	NxUserRenderVertexBufferDesc(void)
	{
		interopContext = 0;
		registerInCUDA = false;
		maxVerts = 0;
		hint     = NxRenderBufferHint::STATIC;
		for (physx::PxU32 i = 0; i < NxRenderVertexSemantic::NUM_SEMANTICS; i++)
		{
			buffersRequest[i] = NxRenderDataFormat::UNSPECIFIED;
		}
		moduleIdentifier = 0;
		numCustomBuffers = 0;
		customBuffersIdents = NULL;
		customBuffersRequest = NULL;
		uvOrigin         = NxTextureUVOrigin::ORIGIN_TOP_LEFT;
		canBeShared      = true;
	}

	/**
	\brief Checks if dsta is correct
	*/
	bool isValid(void) const
	{
		physx::PxU32 numFailed = 0;
		if (maxVerts == 0)
		{
			numFailed++;
		}
		if (!customBuffersRequest)
		{
			physx::PxU32 vertexSize = 0;
			for (physx::PxU32 i = 0; i < NxRenderVertexSemantic::NUM_SEMANTICS; i++)
			{
				vertexSize += NxRenderDataFormat::getFormatDataSize(buffersRequest[i]);
			}
			if (vertexSize == 0)
			{
				numFailed++;
			}
		}
		return (numFailed == 0);
	}

public:
	/**
	\brief The maximum number of vertices that APEX will store in this buffer.
	*/
	physx::PxU32				maxVerts;

	/**
	\brief A hint about the update frequency of this buffer
	*/
	NxRenderBufferHint::Enum	hint;

	/**
	\brief Array of semantics with the corresponding format.

	NxRenderDataFormat::UNSPECIFIED is used for semantics that are disabled
	*/
	NxRenderDataFormat::Enum	buffersRequest[NxRenderVertexSemantic::NUM_SEMANTICS];

	/**
	\brief List of custom semantics that will be provided for this VB.
	*/

	/**
	\brief The number of custom VB semantics.
	*/
	physx::PxU32				numCustomBuffers;

	/**
	\brief Array of custom buffer identifiers
	*/
	void** 						customBuffersIdents;

	/**
	\brief Array of custom VB format types
	*/
	NxRenderDataFormat::Enum*	customBuffersRequest;

	/**
	\brief Identifier of module generating this request
	*/
	NxAuthObjTypeID				moduleIdentifier;

	/**
	\brief Texture UV direction convention.
	*/
	NxTextureUVOrigin::Enum		uvOrigin;

	/**
	\brief Buffer can be shared by multiple render resources
	*/
	bool						canBeShared;
	
	/**
	\brief Declare if the resource must be registered in CUDA upon creation
	*/
	bool						registerInCUDA;

	/**
	\brief The CUDA context

	This context must be used to register and unregister the resource every time the
	device is lost and recreated.
	*/
	physx::PxCudaContextManager*	interopContext;
};

PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_USER_RENDER_VERTEX_BUFFER_DESC_H
