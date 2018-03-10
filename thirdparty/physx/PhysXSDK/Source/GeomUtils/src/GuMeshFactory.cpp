/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#include "PsIntrinsics.h"
#include "GuMeshFactory.h"

#include "PxHeightFieldDesc.h"

#include "GuTriangleMesh.h"
#include "GuConvexMesh.h"
#include "GuHeightField.h"
#include "CmUtils.h"

#if PX_SUPPORT_GPU_PHYSX
	#define	GU_MESH_FACTORY_GPU_NOTIFICATION(notificationMethod, argument) notificationMethod(argument);
#else
	#define GU_MESH_FACTORY_GPU_NOTIFICATION(notificationMethod, argument)
#endif

using namespace physx;

// PT: TODO: refactor all this with a dedicated container

GuMeshFactory::~GuMeshFactory()
{
}

///////////////////////////////////////////////////////////////////////////////

void GuMeshFactory::release()
{
	// Release all objects in case the user didn't do it

	while(mTriangleMeshes.size())
	{
		Gu::TriangleMesh* mesh = mTriangleMeshes.getEntries()[0];
		PX_ASSERT(mesh->getRefCount()==1);
		GU_MESH_FACTORY_GPU_NOTIFICATION(notifyReleaseTriangleMesh, *mesh);
		mesh->release();
	}

	while(mConvexMeshes.size())
	{
		Gu::ConvexMesh* mesh = mConvexMeshes.getEntries()[0];
		PX_ASSERT(mesh->getRefCount()==1);
		GU_MESH_FACTORY_GPU_NOTIFICATION(notifyReleaseConvexMesh, *mesh);
		mesh->release();
	}

	while(mHeightFields.size())
	{
		Gu::HeightField* mesh = mHeightFields.getEntries()[0];
		PX_ASSERT(mesh->getRefCount()==1);
		GU_MESH_FACTORY_GPU_NOTIFICATION(notifyReleaseHeightField, *mesh);
		mesh->release();
	}

	PX_DELETE(this);
}

namespace
{
	template<typename TDataType>
	inline void notifyReleaseFactoryItem( Ps::Array<GuMeshFactoryListener*>& listeners, const TDataType* type, PxType typeID, bool memRelease )
	{
		PxU32 numListeners = listeners.size();
		for ( PxU32 idx = 0; idx < numListeners; ++idx )
			listeners[idx]->onGuMeshFactoryBufferRelease( type, typeID, memRelease );
	}

	template <typename T> void addToHash(Ps::CoalescedHashSet<T*>& hash, T* element, Ps::Mutex* mutex)
	{
		if(!element)
			return;

		if(mutex)
			mutex->lock();

		hash.insert(element);

		if(mutex)
			mutex->unlock();
	}
}

///////////////////////////////////////////////////////////////////////////////

void GuMeshFactory::addTriangleMesh(Gu::TriangleMesh* np, bool lock)
{
	addToHash(mTriangleMeshes, np, lock ? &mTrackingMutex : NULL);
}

PxTriangleMesh* GuMeshFactory::createTriangleMesh(PxInputStream& desc)
{	
	Gu::TriangleMesh* np;
	PX_NEW_SERIALIZED(np,Gu::TriangleMesh);
	if(!np)
		return NULL;

	np->setMeshFactory(this);

	if(!np->load(desc))
	{
		np->decRefCount();
		return NULL;
	}

	addTriangleMesh(np);
	return np;
}

bool GuMeshFactory::removeTriangleMesh(PxTriangleMesh& m)
{
	Ps::Mutex::ScopedLock lock(mTrackingMutex);
	Gu::TriangleMesh* gu = static_cast<Gu::TriangleMesh*>(&m);

	bool found = mTriangleMeshes.erase(gu);

	if(found)
	{
		GU_MESH_FACTORY_GPU_NOTIFICATION(notifyReleaseTriangleMesh, m)
	}

	return found;
}

PxU32 GuMeshFactory::getNbTriangleMeshes() const
{
	Ps::Mutex::ScopedLock lock(mTrackingMutex);
	return mTriangleMeshes.size();
}

PxU32 GuMeshFactory::getTriangleMeshes(PxTriangleMesh** userBuffer, PxU32 bufferSize, PxU32 startIndex) const
{
	Ps::Mutex::ScopedLock lock(mTrackingMutex);

	const PxU32 size = mTriangleMeshes.size();
	const PxU32 remainder = (PxU32)PxMax<PxI32>(PxI32(size - startIndex), 0);
	const PxU32 writeCount = PxMin(remainder, bufferSize);

	Gu::TriangleMesh*const* meshes = mTriangleMeshes.getEntries();
	for(PxU32 i=0; i<writeCount; i++)
		userBuffer[i] = meshes[i+startIndex];

	return writeCount;
}

///////////////////////////////////////////////////////////////////////////////

void GuMeshFactory::addConvexMesh(Gu::ConvexMesh* np, bool lock)
{
	addToHash(mConvexMeshes, np, lock ? &mTrackingMutex : NULL);
}

PxConvexMesh* GuMeshFactory::createConvexMesh(PxInputStream& desc)
{
	Gu::ConvexMesh* np;
	PX_NEW_SERIALIZED(np,Gu::ConvexMesh);
	if(!np)
		return NULL;

	np->setMeshFactory(this);

	if(!np->load(desc))
	{
		np->decRefCount();
		return NULL;
	}

	addConvexMesh(np);
	return np;
}

bool GuMeshFactory::removeConvexMesh(PxConvexMesh& m)
{
	Ps::Mutex::ScopedLock lock(mTrackingMutex);
	Gu::ConvexMesh* gu = static_cast<Gu::ConvexMesh*>(&m);

	bool found = mConvexMeshes.erase(gu);
	if(found)
	{
		GU_MESH_FACTORY_GPU_NOTIFICATION(notifyReleaseConvexMesh, m)
	}

	return found;
}

PxU32 GuMeshFactory::getNbConvexMeshes() const
{
	Ps::Mutex::ScopedLock lock(mTrackingMutex);
	return mConvexMeshes.size();
}

PxU32 GuMeshFactory::getConvexMeshes(PxConvexMesh** userBuffer, PxU32 bufferSize, PxU32 startIndex) const
{
	Ps::Mutex::ScopedLock lock(mTrackingMutex);
	
	const PxU32 size = mConvexMeshes.size();
	const PxU32 remainder = (PxU32)PxMax<PxI32>(PxI32(size - startIndex), 0);
	const PxU32 writeCount = PxMin(remainder, bufferSize);

	Gu::ConvexMesh*const* meshes = mConvexMeshes.getEntries();
	for(PxU32 i=0; i<writeCount; i++)
		userBuffer[i] = meshes[i+startIndex];

	return writeCount;
}

///////////////////////////////////////////////////////////////////////////////

void GuMeshFactory::addHeightField(Gu::HeightField* np, bool lock)
{
	addToHash(mHeightFields, np, lock ? &mTrackingMutex : NULL);
}

PxHeightField* GuMeshFactory::createHeightField(const PxHeightFieldDesc& desc)
{
	Gu::HeightField* np;
	PX_NEW_SERIALIZED(np,Gu::HeightField)(this);
	if(!np)
		return NULL;

	if(!np->loadFromDesc(desc))
	{
		np->decRefCount();
		return NULL;
	}

	addHeightField(np);
	return np;
}

PxHeightField* GuMeshFactory::createHeightField(PxInputStream& stream)
{
	Gu::HeightField* np;
	PX_NEW_SERIALIZED(np,Gu::HeightField)(this);
	if(!np)
		return NULL;

	if(!np->load(stream))
	{
		np->decRefCount();
		return NULL;
	}

	addHeightField(np);
	return np;
}

bool GuMeshFactory::removeHeightField(PxHeightField& hf)
{
	Ps::Mutex::ScopedLock lock(mTrackingMutex);
	Gu::HeightField* gu = static_cast<Gu::HeightField*>(&hf);

	bool found = mHeightFields.erase(gu);
	if(found)
	{
		GU_MESH_FACTORY_GPU_NOTIFICATION(notifyReleaseHeightField, hf)
	}

	return found;
}

PxU32 GuMeshFactory::getNbHeightFields() const
{
	Ps::Mutex::ScopedLock lock(mTrackingMutex);
	return mHeightFields.size();
}

PxU32 GuMeshFactory::getHeightFields(PxHeightField** userBuffer, PxU32 bufferSize, PxU32 startIndex) const
{
	Ps::Mutex::ScopedLock lock(mTrackingMutex);
	
	const PxU32 size = mHeightFields.size();
	const PxU32 remainder = (PxU32)PxMax<PxI32>(PxI32(size - startIndex), 0);
	const PxU32 writeCount = PxMin(remainder, bufferSize);

	Gu::HeightField*const* meshes = mHeightFields.getEntries();
	for(PxU32 i=0; i<writeCount; i++)
		userBuffer[i] = meshes[i+startIndex];

	return writeCount;
}


void GuMeshFactory::addFactoryListener( GuMeshFactoryListener& listener )
{
	Ps::Mutex::ScopedLock lock(mTrackingMutex);
	mFactoryListeners.pushBack( &listener );
}
void GuMeshFactory::removeFactoryListener( GuMeshFactoryListener& listener )
{
	Ps::Mutex::ScopedLock lock(mTrackingMutex);
	for ( PxU32 idx = 0; idx < mFactoryListeners.size(); ++idx )
	{
		if ( mFactoryListeners[idx] == &listener )
		{
			mFactoryListeners.replaceWithLast( idx );
			--idx;
		}
	}
}

void GuMeshFactory::notifyFactoryListener(const PxBase* base, PxType typeID, bool memRelease)
{
	notifyReleaseFactoryItem(mFactoryListeners, base, typeID, memRelease);
}

///////////////////////////////////////////////////////////////////////////////
