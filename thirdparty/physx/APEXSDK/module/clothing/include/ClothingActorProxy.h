/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CLOTHING_ACTOR_PROXY_H
#define CLOTHING_ACTOR_PROXY_H

#include "NxClothingActor.h"
#include "PsUserAllocated.h"
#include "ClothingAsset.h"
#include "ApexRWLockable.h"
#include "ClothingActor.h"

#include "ReadCheck.h"
#include "WriteCheck.h"

namespace physx
{
namespace apex
{
namespace clothing
{

class ClothingActorProxy : public NxClothingActor, public physx::UserAllocated, public NxApexResource, ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ClothingActor impl;

#pragma warning(push)
#pragma warning( disable : 4355 ) // disable warning about this pointer in argument list

	ClothingActorProxy(const NxParameterized::Interface& desc, ClothingAsset* asset, ClothingScene& clothingScene, NxResourceList* list) :
		impl(desc, this, NULL, asset, &clothingScene)
	{
		list->add(*this);
	}

#pragma warning(pop)

	virtual void release()
	{
		impl.release();			// calls release method on asset
	}

	virtual physx::PxU32 getListIndex() const
	{
		return impl.m_listIndex;
	}

	virtual void setListIndex(class NxResourceList& list, physx::PxU32 index)
	{
		impl.m_list = &list;
		impl.m_listIndex = index;
	}

#ifndef WITHOUT_PVD
	virtual void initPvdInstances(physx::debugger::comm::PvdDataStream& pvdStream)
	{
		impl.initPvdInstances(pvdStream);
	}
#endif

	virtual void destroy()
	{
		impl.destroy();
		delete this;
	}

	// NxApexActor base class
	virtual NxApexAsset* getOwner() const
	{
		return impl.getOwner();
	}
	virtual physx::PxBounds3 getBounds() const
	{
		return impl.getBounds();
	}

	virtual void lockRenderResources()
	{
		impl.lockRenderResources();
	}

	virtual void unlockRenderResources()
	{
		impl.unlockRenderResources();
	}

	virtual void updateRenderResources(bool rewriteBuffers, void* userRenderData)
	{
		URR_SCOPE;
		impl.updateRenderResources(rewriteBuffers, userRenderData);
	}

	virtual void dispatchRenderResources(NxUserRenderer& renderer)
	{
		impl.dispatchRenderResources(renderer);
	}

	virtual NxParameterized::Interface* getActorDesc()
	{
		NX_READ_ZONE();
		return impl.getActorDesc();
	}

	virtual void updateState(const physx::PxMat44& globalPose, const physx::PxMat44* newBoneMatrices, physx::PxU32 boneMatricesByteStride, physx::PxU32 numBoneMatrices, ClothingTeleportMode::Enum teleportMode)
	{
		NX_WRITE_ZONE();
		impl.updateState(globalPose, newBoneMatrices, boneMatricesByteStride, numBoneMatrices, teleportMode);
	}

	virtual void updateMaxDistanceScale(PxF32 scale, bool multipliable)
	{
		NX_WRITE_ZONE();
		impl.updateMaxDistanceScale(scale, multipliable);
	}

	virtual const physx::PxMat44& getGlobalPose() const
	{
		NX_READ_ZONE();
		return impl.getGlobalPose();
	}

	virtual void setWind(PxF32 windAdaption, const physx::PxVec3& windVelocity)
	{
		NX_WRITE_ZONE();
		impl.setWind(windAdaption, windVelocity);
	}

	virtual void setMaxDistanceBlendTime(PxF32 blendTime)
	{
		NX_WRITE_ZONE();
		impl.setMaxDistanceBlendTime(blendTime);
	}

	virtual PxF32 getMaxDistanceBlendTime() const
	{
		NX_READ_ZONE();
		return impl.getMaxDistanceBlendTime();
	}

	virtual void getPhysicalMeshPositions(void* buffer, physx::PxU32 byteStride)
	{
		NX_READ_ZONE();
		impl.getPhysicalMeshPositions(buffer, byteStride);
	}

	virtual void getPhysicalMeshNormals(void* buffer, physx::PxU32 byteStride)
	{
		NX_READ_ZONE();
		impl.getPhysicalMeshNormals(buffer, byteStride);
	}

	virtual physx::PxF32 getMaximumSimulationBudget() const
	{
		NX_READ_ZONE();
		return impl.getMaximumSimulationBudget();
	}

	virtual physx::PxU32 getNumSimulationVertices() const
	{
		NX_READ_ZONE();
		return impl.getNumSimulationVertices();
	}

	virtual const physx::PxVec3* getSimulationPositions()
	{
		NX_READ_ZONE();
		return impl.getSimulationPositions();
	}

	virtual const physx::PxVec3* getSimulationNormals()
	{
		NX_READ_ZONE();
		return impl.getSimulationNormals();
	}

	virtual bool getSimulationVelocities(PxVec3* velocities)
	{
		NX_READ_ZONE();
		return impl.getSimulationVelocities(velocities);
	}

	virtual physx::PxU32 getNumGraphicalVerticesActive(PxU32 submeshIndex) const
	{
		NX_READ_ZONE();
		return impl.getNumGraphicalVerticesActive(submeshIndex);
	}

	virtual PxMat44 getRenderGlobalPose() const
	{
		NX_READ_ZONE();
		return impl.getRenderGlobalPose();
	}

	virtual const PxMat44* getCurrentBoneSkinningMatrices() const
	{
		NX_READ_ZONE();
		return impl.getCurrentBoneSkinningMatrices();
	}

	virtual void setVisible(bool enable)
	{
		NX_WRITE_ZONE();
		impl.setVisible(enable);
	}

	virtual bool isVisible() const
	{
		NX_READ_ZONE();
		return impl.isVisibleBuffered();
	}

	virtual void setFrozen(bool enable)
	{
		NX_WRITE_ZONE();
		impl.setFrozen(enable);
	}

	virtual bool isFrozen() const
	{
		NX_READ_ZONE();
		return impl.isFrozenBuffered();
	}

	virtual ClothSolverMode::Enum getClothSolverMode() const
	{
		NX_READ_ZONE();
		return impl.getClothSolverMode();
	}

	virtual void setLODWeights(PxF32 maxDistance, PxF32 distanceWeight, PxF32 bias, PxF32 benefitBias)
	{
		NX_WRITE_ZONE();
		impl.setLODWeights(maxDistance, distanceWeight, bias, benefitBias);
	}

	virtual void setGraphicalLOD(PxU32 lod)
	{
		NX_WRITE_ZONE();
		impl.setGraphicalLOD(lod);
	}

	virtual PxU32 getGraphicalLod()
	{
		NX_READ_ZONE();
		return impl.getGraphicalLod();
	}

	virtual bool rayCast(const PxVec3& worldOrigin, const PxVec3& worldDirection, PxF32& time, PxVec3& normal, PxU32& vertexIndex)
	{
		NX_READ_ZONE();
		return impl.rayCast(worldOrigin, worldDirection, time, normal, vertexIndex);
	}

	virtual void attachVertexToGlobalPosition(PxU32 vertexIndex, const PxVec3& globalPosition)
	{
		NX_WRITE_ZONE();
		impl.attachVertexToGlobalPosition(vertexIndex, globalPosition);
	}

	virtual void freeVertex(PxU32 vertexIndex)
	{
		NX_WRITE_ZONE();
		impl.freeVertex(vertexIndex);
	}

	virtual PxU32 getClothingMaterial() const
	{
		NX_READ_ZONE();
		return impl.getClothingMaterial();
	}

	virtual void setClothingMaterial(physx::PxU32 index)
	{
		NX_WRITE_ZONE();
		impl.setClothingMaterial(index);
	}

	virtual void setOverrideMaterial(PxU32 submeshIndex, const char* overrideMaterialName)
	{
		NX_WRITE_ZONE();
		impl.setOverrideMaterial(submeshIndex, overrideMaterialName);
	}

	virtual void setUserRecompute(NxClothingUserRecompute* recompute)
	{
		NX_WRITE_ZONE();
		impl.setUserRecompute(recompute);
	}

	virtual void setVelocityCallback(NxClothingVelocityCallback* callback)
	{
		NX_WRITE_ZONE();
		impl.setVelocityCallback(callback);
	}

	virtual void setInterCollisionChannels(physx::PxU32 channels)
	{
		NX_WRITE_ZONE();
		impl.setInterCollisionChannels(channels);
	}

	virtual physx::PxU32 getInterCollisionChannels()
	{
		NX_READ_ZONE();
		return impl.getInterCollisionChannels();
	}

	virtual void getPhysicalLodRange(PxF32& min, PxF32& max, bool& intOnly) const
	{
		NX_READ_ZONE();
		impl.getPhysicalLodRange(min, max, intOnly);
	}

	virtual PxF32 getActivePhysicalLod() const
	{
		NX_READ_ZONE();
		return impl.getActivePhysicalLod();
	}

	virtual void forcePhysicalLod(PxF32 lod)
	{
		NX_WRITE_ZONE();
		impl.forcePhysicalLod(lod);
	}

	/**
	\brief Selectively enables/disables debug visualization of a specific APEX actor.  Default value it true.
	*/
	virtual void setEnableDebugVisualization(bool state)
	{
		impl.setEnableDebugVisualization(state);
	}

	virtual NxClothingPlane* createCollisionPlane(const PxPlane& plane)
	{
		NX_WRITE_ZONE();
		return impl.createCollisionPlane(plane);
	}

	virtual NxClothingConvex* createCollisionConvex(NxClothingPlane** planes, PxU32 numPlanes)
	{
		NX_WRITE_ZONE();
		return impl.createCollisionConvex(planes, numPlanes);
	}

	virtual NxClothingSphere* createCollisionSphere(const PxVec3& position, PxF32 radius)
	{
		NX_WRITE_ZONE();
		return impl.createCollisionSphere(position, radius);
	}

	virtual NxClothingCapsule* createCollisionCapsule(NxClothingSphere& sphere1, NxClothingSphere& sphere2) 
	{
		NX_WRITE_ZONE();
		return impl.createCollisionCapsule(sphere1, sphere2);
	}

	virtual NxClothingTriangleMesh* createCollisionTriangleMesh()
	{
		NX_WRITE_ZONE();
		return impl.createCollisionTriangleMesh();
	}

	virtual NxClothingRenderProxy* acquireRenderProxy()
	{
		NX_READ_ZONE();
		return impl.acquireRenderProxy();
	}
};


}
} // namespace apex
} // namespace physx

#endif // CLOTHING_ACTOR_PROXY_H
