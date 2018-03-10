/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_CONTEXT_H
#define APEX_CONTEXT_H

#include "PsShare.h"
#include "NxApexContext.h"
#include "PsMutex.h"
#include "PsArray.h"
#include "PsUserAllocated.h"
#include "ApexRWLockable.h"

namespace physx
{
namespace apex
{

class ApexActor;
class ApexRenderableIterator;

class ApexContext
{
public:
	ApexContext() : mIterator(NULL) {}
	virtual ~ApexContext();

	virtual physx::PxU32	addActor(ApexActor& actor, ApexActor* actorPtr = NULL);
	virtual void	callContextCreationCallbacks(ApexActor* actorPtr);
	virtual void	callContextDeletionCallbacks(ApexActor* actorPtr);
	virtual void	removeActorAtIndex(physx::PxU32 index);

	void			renderLockAllActors();
	void			renderUnLockAllActors();

	void			removeAllActors();
	NxApexRenderableIterator* createRenderableIterator();
	void			releaseRenderableIterator(NxApexRenderableIterator&);

protected:
	physx::Array<ApexActor*> mActorArray;
	physx::Array<ApexActor*> mActorArrayCallBacks;
	physx::ReadWriteLock	mActorListLock;
	ApexRenderableIterator* mIterator;

	friend class ApexRenderableIterator;
	friend class ApexActor;
};

class ApexRenderableIterator : public NxApexRenderableIterator, public ApexRWLockable, public physx::UserAllocated
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	NxApexRenderable* getFirst();
	NxApexRenderable* getNext();
	void			  reset();
	void			  release();

protected:
	void			  destroy();
	ApexRenderableIterator(ApexContext&);
	virtual ~ApexRenderableIterator() {}
	void			  removeCachedActor(ApexActor&);

	ApexContext*	  ctx;
	physx::PxU32             curActor;
	ApexActor*        mLockedActor;
	physx::Array<ApexActor*> mCachedActors;
	physx::Array<ApexActor*> mSkippedActors;

	friend class ApexContext;
};

}
} // end namespace physx::apex

#endif // APEX_CONTEXT_H