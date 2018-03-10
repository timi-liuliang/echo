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
  

#include "NpPhysicsGpu.h"

#if PX_SUPPORT_GPU_PHYSX

#include "NpPhysics.h"
#include "PxvGlobals.h"
#include "PxPhysXGpu.h"
#include "PxParticleGpu.h"
#include "NpScene.h"

using namespace physx;

//-------------------------------------------------------------------------------------------------------------------//

Ps::Array<NpPhysicsGpu::MeshMirror>::Iterator NpPhysicsGpu::findMeshMirror(const void* meshAddress)
{
	Ps::Array<MeshMirror>::Iterator i = mMeshMirrors.begin();
	while (i != mMeshMirrors.end() && i->meshAddress != meshAddress)
		i++;

	return i;
}

Ps::Array<NpPhysicsGpu::MeshMirror>::Iterator NpPhysicsGpu::findMeshMirror(const void* meshAddress, PxCudaContextManager& ctx)
{
	Ps::Array<MeshMirror>::Iterator i = mMeshMirrors.begin();
	while (i != mMeshMirrors.end() && (i->meshAddress != meshAddress || i->ctx != &ctx))
		i++;

	return i;
}

PxPhysXGpu* NpPhysicsGpu::getPhysXGpu(bool createIfNeeded, const char* functionName, bool doWarn)
{
	PxPhysXGpu* physXGpu = PxvGetPhysXGpu(createIfNeeded);
	
	if (!physXGpu && doWarn)
		getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "%s, GPU implementation not available.", functionName);
	
	return physXGpu;
}

bool NpPhysicsGpu::checkNewMirror(const void* meshPtr, PxCudaContextManager& ctx, const char* functionName)
{	
	Ps::Array<MeshMirror>::Iterator it = findMeshMirror(meshPtr, ctx);
	if (it == mMeshMirrors.end())
		return true;
		
	getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "%s has no effect. Mirror already exists.", functionName);
	return false;
}

bool NpPhysicsGpu::checkMirrorExists(Ps::Array<MeshMirror>::Iterator it, const char* functionName)
{
	if (it != mMeshMirrors.end())
		return true;

	getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "%s has no effect. Mirror doesn't exist.", functionName);
	return false;
}

bool NpPhysicsGpu::checkMirrorHandle(PxU32 mirrorHandle, const char* functionName)
{	
	if (mirrorHandle != PX_INVALID_U32)
		return true;

	getFoundation().error(PxErrorCode::eOUT_OF_MEMORY, __FILE__, __LINE__, "%s failed.", functionName);
	return false;
}

void NpPhysicsGpu::addMeshMirror(const void* meshPtr, PxU32 mirrorHandle, PxCudaContextManager& ctx)
{
	MeshMirror& meshMirror = mMeshMirrors.insert();
	meshMirror.meshAddress = meshPtr;
	meshMirror.ctx = &ctx;
	meshMirror.mirrorHandle = mirrorHandle;	
}

void NpPhysicsGpu::releaseMeshMirror(const void* meshPtr, const char* functionName, PxCudaContextManager* ctx)
{
	PxPhysXGpu* physXGpu = getPhysXGpu(false, functionName, ctx != NULL);
	if (!physXGpu)
		return;

	if (ctx)
	{
		Ps::Array<MeshMirror>::Iterator mirrorIt = findMeshMirror(meshPtr, *ctx);
		if (!checkMirrorExists(mirrorIt, functionName))
			return;

		physXGpu->releaseMirror(mirrorIt->mirrorHandle);
		mMeshMirrors.replaceWithLast(mirrorIt);
	}
	else
	{
		//remove all mesh mirrors for all contexts.
		Ps::Array<MeshMirror>::Iterator mirrorIt;
		while ((mirrorIt = findMeshMirror(meshPtr)) != mMeshMirrors.end())
		{
			physXGpu->releaseMirror(mirrorIt->mirrorHandle);
			mMeshMirrors.replaceWithLast(mirrorIt);
		}
	}
}

//-------------------------------------------------------------------------------------------------------------------//


bool NpPhysicsGpu::createTriangleMeshMirror(const PxTriangleMesh& triangleMesh, PxCudaContextManager& contextManager)
{
	const char* functionName = "PxParticleGpu::createTriangleMeshMirror()";
	PxPhysXGpu* physXGpu = getPhysXGpu(true, functionName);
	if (!physXGpu)
		return false;

	const void* meshPtr = (const void*)&triangleMesh;
	if (!checkNewMirror(meshPtr, contextManager, functionName))
		return true;
	
	PxU32 mirrorHandle = physXGpu->createTriangleMeshMirror(triangleMesh, contextManager);
	if (!checkMirrorHandle(mirrorHandle, functionName))
		return false;
	
	addMeshMirror(meshPtr, mirrorHandle, contextManager);
	return true;
}

//-------------------------------------------------------------------------------------------------------------------//

void NpPhysicsGpu::releaseTriangleMeshMirror(const PxTriangleMesh& triangleMesh, PxCudaContextManager* contextManager)
{
	releaseMeshMirror((const void*)&triangleMesh, "PxParticleGpu::releaseTriangleMeshMirror()", contextManager);
}

//-------------------------------------------------------------------------------------------------------------------//

bool NpPhysicsGpu::createHeightFieldMirror(const PxHeightField& heightField, PxCudaContextManager& contextManager)
{
	const char* functionName = "PxParticleGpu::createHeightFieldMirror()";
	PxPhysXGpu* physXGpu = getPhysXGpu(true, functionName);
	if (!physXGpu)
		return false;

	const void* meshPtr = (const void*)&heightField;
	if (!checkNewMirror(meshPtr, contextManager, functionName))
		return true;

	PxU32 mirrorHandle = physXGpu->createHeightFieldMirror(heightField, contextManager);
	if (!checkMirrorHandle(mirrorHandle, functionName))
		return false;

	addMeshMirror(meshPtr, mirrorHandle, contextManager);
	return true;
}

//-------------------------------------------------------------------------------------------------------------------//

void NpPhysicsGpu::releaseHeightFieldMirror(const PxHeightField& heightField, PxCudaContextManager* contextManager)
{
	releaseMeshMirror((const void*)&heightField, "PxParticleGpu::releaseHeightFieldMirror()", contextManager);
}

//-------------------------------------------------------------------------------------------------------------------//

bool NpPhysicsGpu::createConvexMeshMirror(const PxConvexMesh& convexMesh, PxCudaContextManager& contextManager)
{
	const char* functionName = "PxParticleGpu::createConvexMeshMirror()";
	PxPhysXGpu* physXGpu = getPhysXGpu(true, functionName);
	if (!physXGpu)
		return false;

	const void* meshPtr = (const void*)&convexMesh;
	if (!checkNewMirror(meshPtr, contextManager, functionName))
		return true;

	PxU32 mirrorHandle = physXGpu->createConvexMeshMirror(convexMesh, contextManager);
	if (!checkMirrorHandle(mirrorHandle, functionName))
		return false;

	addMeshMirror(meshPtr, mirrorHandle, contextManager);
	return true;
}

//-------------------------------------------------------------------------------------------------------------------//

void NpPhysicsGpu::releaseConvexMeshMirror(const PxConvexMesh& convexMesh, PxCudaContextManager* contextManager)
{
	releaseMeshMirror((const void*)&convexMesh, "PxParticleGpu::releaseConvexMeshMirror()", contextManager);
}

//-------------------------------------------------------------------------------------------------------------------//

namespace
{
	PxSceneGpu* getSceneGpu(const PxScene& scene)
	{
#if PX_SUPPORT_GPU_PHYSX
		return ((NpScene*)&scene)->mScene.getScScene().getSceneGpu(true);
#else
		return NULL;
#endif
	}
}

void NpPhysicsGpu::setExplicitCudaFlushCountHint(const class PxScene& scene, PxU32 cudaFlushCount)
{
	const char* functionName = "PxParticleGpu::setExplicitCudaFlushCountHint()";
	PxPhysXGpu* physXGpu = getPhysXGpu(true, functionName);
	if (physXGpu)
	{
		PxSceneGpu* gpuScene = getSceneGpu(scene);
		PX_ASSERT(gpuScene);
		physXGpu->setExplicitCudaFlushCountHint((PxgSceneGpu&)(*gpuScene), cudaFlushCount);
	}
}

//-------------------------------------------------------------------------------------------------------------------//

bool NpPhysicsGpu::setTriangleMeshCacheSizeHint(const class PxScene& scene, PxU32 size)
{
	const char* functionName = "PxParticleGpu::setTriangleMeshCacheSizeHint()";
	PxPhysXGpu* physXGpu = getPhysXGpu(true, functionName);
	if (physXGpu)
	{
		PxSceneGpu* gpuScene = getSceneGpu(scene);
		PX_ASSERT(gpuScene);
		return physXGpu->setTriangleMeshCacheSizeHint((PxgSceneGpu&)(*gpuScene), size);
	}
	return false;
}

//-------------------------------------------------------------------------------------------------------------------//

PxTriangleMeshCacheStatistics NpPhysicsGpu::getTriangleMeshCacheStatistics(const class PxScene& scene)
{
	PxTriangleMeshCacheStatistics stats;

	const char* functionName = "PxParticleGpu::getTriangleMeshCacheStatistics()";
	PxPhysXGpu* physXGpu = getPhysXGpu(true, functionName);
	if (physXGpu)
	{
		PxSceneGpu* gpuScene = getSceneGpu(scene);
		PX_ASSERT(gpuScene);
		stats = physXGpu->getTriangleMeshCacheStatistics((PxgSceneGpu&)(*gpuScene));
	}
	return stats;
}

//-------------------------------------------------------------------------------------------------------------------//

NpPhysicsGpu::NpPhysicsGpu(NpPhysics& npPhysics) : mMeshMirrors(PX_DEBUG_EXP("NpPhysicsGpu:mMeshMirrors")), mNpPhysics(npPhysics) 
{}

//-------------------------------------------------------------------------------------------------------------------//

NpPhysicsGpu::~NpPhysicsGpu()
{}

//-------------------------------------------------------------------------------------------------------------------//

#endif // PX_SUPPORT_GPU_PHYSX
