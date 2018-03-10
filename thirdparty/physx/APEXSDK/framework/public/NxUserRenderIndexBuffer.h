/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_USER_RENDER_INDEX_BUFFER_H
#define NX_USER_RENDER_INDEX_BUFFER_H

/*!
\file
\brief class NxUserRenderIndexBuffer
*/

#include "NxApexUsingNamespace.h"

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
\brief Used for storing index data for rendering.
*/
class NxUserRenderIndexBuffer
{
public:
	virtual		~NxUserRenderIndexBuffer() {}

	///Get the low-level handle of the buffer resource (D3D resource pointer or GL buffer object ID)
	///\return true id succeeded, false otherwise
	virtual bool getInteropResourceHandle(CUgraphicsResource& handle)
#if NX_APEX_DEFAULT_NO_INTEROP_IMPLEMENTATION
	{
		PX_UNUSED(&handle);
		return false;
	}
#else
	= 0;
#endif

	//! write some data into the buffer.
	//  the source data type is assumed to be the same as what was defined in the descriptor.
	virtual void writeBuffer(const void* srcData, physx::PxU32 srcStride, physx::PxU32 firstDestElement, physx::PxU32 numElements) = 0;
};

PX_POP_PACK

}
} // namespace physx::apex

#endif // NX_USER_RENDER_INDEX_BUFFER_H
