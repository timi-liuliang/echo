/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_USER_RENDER_BONE_BUFFER_DESC_H
#define NX_USER_RENDER_BONE_BUFFER_DESC_H

/*!
\file
\brief class NxUserRenderBoneBufferDesc, structs NxRenderDataFormat and NxRenderBoneSemantic
*/

#include "NxApexRenderDataFormat.h"
#include "NxUserRenderResourceManager.h"

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief The semantics available for bone buffers
*/
struct NxRenderBoneSemantic
{
	/**
	\brief Enum of the semantics available for bone buffers
	*/
	enum Enum
	{
		POSE = 0,		//!< A matrix that transforms from object space into animated object space or even world space
		PREVIOUS_POSE,	//!< The corresponding poses from the last frame
		NUM_SEMANTICS	//!< Count of semantics, not a valid semantic.
	};
};



/**
\brief Descriptor to generate a bone buffer

This descriptor is filled out by APEX and helps as a guide how the
bone buffer should be generated.
*/
class NxUserRenderBoneBufferDesc
{
public:
	NxUserRenderBoneBufferDesc(void)
	{
		maxBones = 0;
		hint     = NxRenderBufferHint::STATIC;
		for (physx::PxU32 i = 0; i < NxRenderBoneSemantic::NUM_SEMANTICS; i++)
		{
			buffersRequest[i] = NxRenderDataFormat::UNSPECIFIED;
		}
	}

	/**
	\brief Check if parameter's values are correct
	*/
	bool isValid(void) const
	{
		physx::PxU32 numFailed = 0;
		return (numFailed == 0);
	}

public:
	/**
	\brief The maximum amount of bones this buffer will ever hold.
	*/
	physx::PxU32				maxBones;

	/**
	\brief Hint on how often this buffer is updated.
	*/
	NxRenderBufferHint::Enum	hint;

	/**
	\brief Array of semantics with the corresponding format.

	NxRenderDataFormat::UNSPECIFIED is used for semantics that are disabled
	*/
	NxRenderDataFormat::Enum	buffersRequest[NxRenderBoneSemantic::NUM_SEMANTICS];
};

PX_POP_PACK

}
} // end namespace physx::apex

#endif
