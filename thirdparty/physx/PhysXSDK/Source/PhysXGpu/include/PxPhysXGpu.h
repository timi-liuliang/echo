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


#ifndef PX_PHYSX_GPU_H
#define PX_PHYSX_GPU_H

#include "Pxg.h"
#include "Ps.h"
#include "PxSceneGpu.h"

namespace physx
{

class PxFoundation;
class PxCudaContextManager;

namespace Cm
{
	class EventProfiler;
	class RenderBuffer;
}

struct PxTriangleMeshCacheStatistics;

/**
\brief Interface to create and run CUDA enabled PhysX features.

The methods of this interface are expected not to be called concurrently. 
Also they are expected to not be called concurrently with any methods of PxSceneGpu and any tasks spawned before the end pipeline ... TODO make clear.
*/
class PxPhysXGpu
{
public:
	/**
	\brief Closes this instance of the interface.
	*/
	virtual		void					release() = 0;

	/**
	\brief Create a gpu scene instance.
	
	\param contextManager The PxCudaContextManager the scene is supposed to use.
	\param rigidBodyAccess The PxRigidBodyAccessGpu implementation the scene is supposed to use.
	\return pointer to gpu scene class (might be NULL on failure).
	*/
	virtual		class PxSceneGpu*		createScene(physx::PxCudaContextManager& contextManager, 
													class PxRigidBodyAccessGpu& rigidBodyAccess, 
													Cm::EventProfiler& eventProfiler,
													Cm::RenderBuffer& debugRenderBuffer) = 0;
	
	/**
	Mirror a triangle mesh onto the gpu memory corresponding to contextManager. Returns a handle for the mirrored mesh, PX_INVALID_U32 if failed.
	*/
	virtual		physx::PxU32			createTriangleMeshMirror(const class PxTriangleMesh& triangleMesh, physx::PxCudaContextManager& contextManager) = 0;
	
	/**
	Mirror a height field mesh onto the gpu memory corresponding to contextManager. Returns a handle for the mirrored mesh, PX_INVALID_U32 if failed.
	*/
	virtual		physx::PxU32			createHeightFieldMirror(const class PxHeightField& heightField, physx::PxCudaContextManager& contextManager) = 0;

	/**
	Mirror a convex mesh onto the gpu memory corresponding to contextManager. Returns a handle for the mirrored mesh, PX_INVALID_U32 if failed.
	*/
	virtual		physx::PxU32			createConvexMeshMirror(const class PxConvexMesh& convexMesh, physx::PxCudaContextManager& contextManager) = 0;

	/**
	Release a mesh mirror, providing the mirror handle. The mesh might still be mirrored implicitly if its in contact with particle systems.
	*/
	virtual		void					releaseMirror(physx::PxU32 mirrorHandle) = 0;

	/**
	Set the explicit count down counter to explicitly flush the cuda push buffer.
	*/
	virtual		void					setExplicitCudaFlushCountHint(const class PxgSceneGpu& scene, physx::PxU32 cudaFlushCount) = 0;

	/**
	Set the amount of memory for triangle mesh cache. Returns true if cache memory is sucessfully allocated, false otherwise.
	*/
	virtual		bool					setTriangleMeshCacheSizeHint(const class PxgSceneGpu& scene, physx::PxU32 size) = 0;

	/**
	Gets the usage statistics for triangle mesh cache. Returns PxTriangleMeshCacheStatistics.
	*/
	virtual		const physx::PxTriangleMeshCacheStatistics&	getTriangleMeshCacheStatistics(const class PxgSceneGpu& scene) const = 0;

	/**
	Create GPU cloth factory.
	*/
	virtual cloth::Factory* createClothFactory(int, void*) = 0;
};

}

/**
Create PxPhysXGpu interface class.
*/
PX_C_EXPORT PX_PHYSX_GPU_API physx::PxPhysXGpu* PX_CALL_CONV PxCreatePhysXGpu();

#endif // PX_PHYSX_GPU_H
