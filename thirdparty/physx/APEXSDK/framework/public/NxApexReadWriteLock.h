/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef NX_APEX_READ_WRITE_LOCK_H

#define NX_APEX_READ_WRITE_LOCK_H

#include "foundation/PxPreprocessor.h"
#include "NxApexDefs.h"

#if NX_SDK_VERSION_MAJOR == 3

#include "PxScene.h"

namespace physx
{
	namespace apex
	{

/**
\brief A scoped object for acquiring/releasing read/write lock of a PhysX scene
*/
class ScopedPhysX3LockRead
{
public:
	/**
	\brief Ctor
	*/
	ScopedPhysX3LockRead(PxScene *scene,const char *fileName,int lineno) : mScene(scene)
	{
		if ( mScene )
		{
			mScene->lockRead(fileName, (physx::PxU32)lineno);
		}
	}
	~ScopedPhysX3LockRead()
	{
		if ( mScene )
		{
			mScene->unlockRead();
		}
	}
private:
	PxScene* mScene;
};

/**
\brief A scoped object for acquiring/releasing read/write lock of a PhysX scene
*/
class ScopedPhysX3LockWrite
{
public:
	/**
	\brief Ctor
	*/
	ScopedPhysX3LockWrite(PxScene *scene,const char *fileName,int lineno) : mScene(scene)
	{
		if ( mScene ) 
		{
			mScene->lockWrite(fileName, (physx::PxU32)lineno);
		}
	}
	~ScopedPhysX3LockWrite()
	{
		if ( mScene )
		{
			mScene->unlockWrite();
		}
	}
private:
	PxScene* mScene;
};

}; // end apx namespace
}; // end physx namespace


#if defined(_DEBUG) || defined(PX_CHECKED)
#define SCOPED_PHYSX3_LOCK_WRITE(x) physx::apex::ScopedPhysX3LockWrite _wlock(x,__FILE__,__LINE__);
#else
#define SCOPED_PHYSX3_LOCK_WRITE(x) physx::apex::ScopedPhysX3LockWrite _wlock(x,"",__LINE__);
#endif

#if defined(_DEBUG) || defined(PX_CHECKED)
#define SCOPED_PHYSX3_LOCK_READ(x) physx::apex::ScopedPhysX3LockRead _rlock(x,__FILE__,__LINE__);
#else
#define SCOPED_PHYSX3_LOCK_READ(x) physx::apex::ScopedPhysX3LockRead _rlock(x,"",__LINE__);
#endif

#elif NX_SDK_VERSION_MAJOR == 2
#define SCOPED_PHYSX3_LOCK_WRITE(x) ;
#define SCOPED_PHYSX3_LOCK_READ(x) ;
#endif

#endif
