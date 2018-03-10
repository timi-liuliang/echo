/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_USER_RENDER_VERTEX_BUFFER_H
#define NX_USER_RENDER_VERTEX_BUFFER_H

/*!
\file
\brief classes NxUserRenderVertexBuffer and NxApexRenderVertexBufferData
*/

#include "NxApexRenderBufferData.h"
#include "NxUserRenderVertexBufferDesc.h"

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
\brief vertex buffer data
*/
class NxApexRenderVertexBufferData : public NxApexRenderBufferData<NxRenderVertexSemantic, NxRenderVertexSemantic::Enum>, public NxApexModuleSpecificRenderBufferData
{
};

/**
\brief Used for storing per-vertex data for rendering.
*/
class NxUserRenderVertexBuffer
{
public:
	virtual		~NxUserRenderVertexBuffer() {}

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

	/**
	\brief Called when APEX wants to update the contents of the vertex buffer.

	The source data type is assumed to be the same as what was defined in the descriptor.
	APEX should call this function and supply data for ALL semantics that were originally
	requested during creation every time its called.

	\param [in] data				Contains the source data for the vertex buffer.
	\param [in] firstVertex			first vertex to start writing to.
	\param [in] numVertices			number of vertices to write.
	*/
	virtual void writeBuffer(const physx::NxApexRenderVertexBufferData& data, physx::PxU32 firstVertex, physx::PxU32 numVertices) = 0;
};

PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_USER_RENDER_VERTEX_BUFFER_H
