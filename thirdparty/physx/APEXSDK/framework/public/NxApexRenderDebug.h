/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_APEX_RENDER_DEBUG_H
#define NX_APEX_RENDER_DEBUG_H

/*!
\file
\brief debug rendering classes and structures
*/

#include "NxApexDefs.h"
#include "PxRenderDebug.h" // include the header file containing the base class
#include "NxApexInterface.h"
#include "NxApexRenderable.h"

#if NX_SDK_VERSION_MAJOR == 2
class NxDebugRenderable;
#endif // NX_SDK_VERSION_MAJOR == 2
#if NX_SDK_VERSION_MAJOR == 3
#include "common/PxRenderBuffer.h"
#endif


namespace physx
{

namespace apex
{
class NxUserRenderer;
class NxUserRenderResourceManager;

PX_PUSH_PACK_DEFAULT

#if NX_SDK_VERSION_MAJOR == 3

/**
\brief This is a helper class implementation of PxRenderBuffer that holds the counts
and pointers for renderable data.  Does not own the memory, simply used to transfer
the state.  The append method is not supported.
*/
class PhysXRenderBuffer : public PxRenderBuffer
{
public:
	/**
	\brief Get number of points in the render buffer
	*/
	virtual PxU32 getNbPoints() const 
	{
		return mNbPoints;
	}

	/**
	\brief Get points data
	*/
	virtual const PxDebugPoint* getPoints() const
	{
		return mPoints;
	}

	/**
	\brief Get number of lines in the render buffer
	*/
	virtual PxU32 getNbLines() const
	{
		return mNbLines;
	}

	/**
	\brief Get lines data
	*/
	virtual const PxDebugLine* getLines() const 
	{
		return mLines;
	}

	/**
	\brief Get number of triangles in the render buffer
	*/
	virtual PxU32 getNbTriangles() const 
	{
		return mNbTriangles;
	}

	/**
	\brief Get triangles data
	*/
	virtual const PxDebugTriangle* getTriangles() const 
	{
		return mTriangles;
	}

	/**
	\brief Get number of texts in the render buffer
	*/
	virtual PxU32 getNbTexts() const 
	{
		return mNbTexts;
	}

	/**
	\brief Get texts data
	*/
	virtual const PxDebugText* getTexts() const 
	{
		return mTexts;
	}

	/**
	\brief Append PhysX render buffer
	*/
	virtual void append(const PxRenderBuffer& other)
	{
		PX_UNUSED(other);
		PX_ALWAYS_ASSERT(); // this method not implemented!
	}

	/**
	\brief Clear this buffer
	*/
	virtual void clear() 
	{
		mNbPoints = 0;
		mPoints = NULL;
		mNbLines = 0;
		mLines = NULL;
		mNbTriangles = 0;
		mTriangles = NULL;
		mNbTexts = 0;
		mTexts = NULL;
	}

	/**
	\brief Number of points
	*/
	PxU32			mNbPoints;
	/**
	\brief Points data
	*/
	PxDebugPoint	*mPoints;
	/**
	\brief Number of lines
	*/
	PxU32			mNbLines;
	/**
	\brief Lines data
	*/
	PxDebugLine		*mLines;
	/**
	\brief Number of triangles
	*/
	PxU32			mNbTriangles;
	/**
	\brief Triangles data
	*/
	PxDebugTriangle	*mTriangles;
	/**
	\brief Number of texts
	*/
	PxU32			mNbTexts;
	/**
	\brief Text data
	*/
	PxDebugText		*mTexts;
};
#endif

/**
\brief wrapper for NxDebugRenderable
 */
class NxApexRenderDebug : public NxApexInterface, public NxApexRenderable, public physx::RenderDebug
{
public:
	/**
	\brief Method to support rendering to a legacy PhysX SDK NxDebugRenderable object instead
	of to the APEX Render Resources API (i.e.: NxApexRenderable).

	This method is used to enable or disable the use of a legacy NxDebugRenderable.  When enabled,
	use the getDebugRenderable() method to get a legacy NxDebugRenerable object that will contain
	all the debug output.
	*/
	virtual void	setUseDebugRenderable(bool state) = 0;


#if NX_SDK_VERSION_MAJOR == 2
	/**
	\brief Method to support rendering to a legacy PhysX SDK NxDebugRenderable object instead
	of to the APEX Render Resources API (i.e.: NxApexRenderable).

	When enabled with a call to setUseDebugRenderable(true), this method will return a legacy
	NxDebugRenderable object that contains all of the output of the NxApexRenderDebug class.
	*/
	virtual void	getDebugRenderable(NxDebugRenderable& renderable) = 0;

	/**
	\brief Method to support rendering to a legacy PhysX SDK NxDebugRenderable object instead
	of to the APEX Render Resources API (i.e.: NxApexRenderable) for lines and triangles 
	in screen space

	When enabled with a call to setUseDebugRenderable(true), this method will return a legacy
	NxDebugRenderable object that contains all of the output of the NxApexRenderDebug class.
	*/
	virtual void	getDebugRenderableScreenSpace(NxDebugRenderable& renderable) = 0;

	/**
	\brief Method to support rendering from an existing legacy PhysX SDK NxDebugRenderable object.

	The contents of the legacy NxDebugRenderable is added to the current contents of the
	NxApexRenderDebug object, and is output through the APEX Render Resources API.
	*/
	virtual void	addDebugRenderable(const NxDebugRenderable& renderable) = 0;
#elif NX_SDK_VERSION_MAJOR == 3

	/**
	\brief Method to support rendering to a legacy PhysX SDK PxRenderBuffer object instead
	of to the APEX Render Resources API (i.e.: NxApexRenderable).

	When enabled with a call to setUseDebugRenderable(true), this method will return a legacy
	PxRenderBuffer object that contains all of the output of the NxApexRenderDebug class.
	*/
	virtual void	getRenderBuffer(PhysXRenderBuffer& renderable) = 0;

	/**
	\brief Method to support rendering to a legacy PhysX SDK PxRenderBuffer object instead
	of to the APEX Render Resources API (i.e.: NxApexRenderable). Lines and triangle in
	screen space

	When enabled with a call to setUseDebugRenderable(true), this method will return a legacy
	PxRenderBuffer object that contains all of the output of the NxApexRenderDebug class.
	*/
	virtual void	getRenderBufferScreenSpace(PhysXRenderBuffer& renderable) = 0;

	/**
	\brief Method to support rendering from an existing PhysX SDK PxRenderBuffer object.

	The contents of the PxRenderBuffer is added to the current contents of the
	NxApexRenderDebug object, and is output through the APEX Render Resources API.
	*/
	virtual void	addDebugRenderable(const physx::PxRenderBuffer& renderBuffer) = 0;

#endif //NX_SDK_VERSION_MAJOR == 2

protected:

	virtual ~NxApexRenderDebug(void) { };

};

PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_APEX_RENDER_DEBUG_H
