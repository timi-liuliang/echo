/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_USER_RENDER_SURFACE_BUFFER_DESC_H
#define NX_USER_RENDER_SURFACE_BUFFER_DESC_H

/*!
\file
\brief class NxUserRenderSurfaceBufferDesc, structs NxRenderDataFormat and NxRenderSurfaceSemantic
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
\brief Describes the semantics and layout of a Surface buffer
*/
class NxUserRenderSurfaceBufferDesc
{
public:
	NxUserRenderSurfaceBufferDesc(void)
	{
		setDefaults();
	}

	/**
	\brief Default values
	*/
	void setDefaults()
	{
//		hint   = NxRenderBufferHint::STATIC;		
		format = NxRenderDataFormat::UNSPECIFIED;
		
		width = 0;
		height = 0;
		depth = 1;

		//moduleIdentifier = 0;
		
		registerInCUDA = false;
		interopContext = 0;
//		stride = 0;
	}

	/**
	\brief Checks if the surface buffer descriptor is valid
	*/
	bool isValid(void) const
	{
		physx::PxU32 numFailed = 0;
		numFailed += (format == NxRenderDataFormat::UNSPECIFIED);
		numFailed += (width == 0) && (height == 0) && (depth == 0);
		numFailed += registerInCUDA && (interopContext == 0);
//		numFailed += registerInCUDA && (stride == 0);
		
		return (numFailed == 0);
	}

public:
	/**
	\brief The size of U-dimension.
	*/
	physx::PxU32				width;
	
	/**
	\brief The size of V-dimension.
	*/
	physx::PxU32				height;

	/**
	\brief The size of W-dimension.
	*/
	physx::PxU32				depth;

	/**
	\brief A hint about the update frequency of this buffer
	*/
//	NxRenderBufferHint::Enum	hint;

	/**
	\brief Data format of suface buffer.
	*/
	NxRenderDataFormat::Enum	format;

	/**
	\brief Identifier of module generating this request
	*/
	//NxAuthObjTypeID				moduleIdentifier;

	/**
	\brief Buffer can be shared by multiple render resources
	*/
	//bool							canBeShared;

//	physx::PxU32					stride; //!< The stride between sprites of this buffer. Required when CUDA interop is used!

	bool							registerInCUDA;  //!< Declare if the resource must be registered in CUDA upon creation

	/**
	This context must be used to register and unregister the resource every time the
	device is lost and recreated.
	*/
	physx::PxCudaContextManager*	interopContext;
};

PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_USER_RENDER_SURFACE_BUFFER_DESC_H
