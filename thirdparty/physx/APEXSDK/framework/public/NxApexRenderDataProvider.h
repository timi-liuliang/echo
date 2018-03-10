/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_APEX_RENDER_DATA_PROVIDER_H
#define NX_APEX_RENDER_DATA_PROVIDER_H

/*!
\file
\brief class NxApexRenderDataProvider
*/

#include "NxApexUsingNamespace.h"

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief An actor instance that provides renderable data
*/
class NxApexRenderDataProvider
{
public:
	/**
	\brief Lock the renderable resources of this NxApexRenderable actor

	Locks the renderable data of this NxApexRenderable actor.  If the user uses an NxApexRenderableIterator
	to retrieve the list of NxApexRenderables, then locking is handled for them automatically by APEX.  If the
	user is storing NxApexRenderable pointers and using them ad-hoc, then they must use this API to lock the
	actor while updateRenderResources() and/or dispatchRenderResources() is called.  If an iterator is not being
	used, the user is also responsible for insuring the NxApexRenderable has not been deleted by another game
	thread.
	*/
	virtual void lockRenderResources() = 0;

	/**
	\brief Unlocks the renderable data of this NxApexRenderable actor.

	See locking semantics for NxApexRenderDataProvider::lockRenderResources().
	*/
	virtual void unlockRenderResources() = 0;

	/**
	\brief Update the renderable data of this NxApexRenderable actor.

	When called, this method will use the NxUserRenderResourceManager interface to inform the user
	about its render resource needs.  It will also call the writeBuffer() methods of various graphics
	buffers.  It must be called by the user each frame before any calls to dispatchRenderResources().
	If the actor is not being rendered, this function may also be skipped.

	\param [in] rewriteBuffers If true then static buffers will be rewritten (in the case of a graphics 
	device context being lost if managed buffers aren't being used)

	\param [in] userRenderData A pointer used by the application for context information which will be sent in
	the NxUserRenderResourceManager::createResource() method as a member of the NxUserRenderResourceDesc class.
	*/
	virtual void updateRenderResources(bool rewriteBuffers = false, void* userRenderData = 0) = 0;
};

PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_APEX_RENDER_DATA_PROVIDER_H
