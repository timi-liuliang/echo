/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef SIMULATION_NX_SOFTBODY_H
#define SIMULATION_NX_SOFTBODY_H


#include "SimulationNxCommon.h"

#include "NxApexDefs.h"
#if NX_SDK_VERSION_MAJOR != 2
#error "SimulationNxSoftBody.h must only be included for builds with 2.8.x"
#endif

class NxSoftBody;

namespace physx
{
namespace apex
{

class NiApexScene;

namespace clothing
{
class ClothingCookedParam;

class SimulationNxSoftBody : public SimulationNxCommon
{
public:
	SimulationNxSoftBody(ClothingScene* clothingScene, bool useHW);
	virtual ~SimulationNxSoftBody();

	virtual SimulationType::Enum getType() const { return SimulationType::SOFTBODY2x; }
	virtual bool dependsOnScene(void* scenePointer);
	virtual PxU32 getNumSolverIterations() const;
	virtual bool initPhysics(NxScene* scene, PxU32 physicalMeshId, PxU32 submeshId, PxU32* indices, PxVec3* restPositions, tMaterial* material, const PxMat44& globalPose, const PxVec3& scaledGravity, bool localSpaceSim);

	virtual void registerPhysX(NxApexActor* actor);
	virtual void unregisterPhysX();
	virtual void disablePhysX(NxApexActor* dummy);
	virtual void reenablePhysX(NxApexActor* newMaster, const PxMat44& globalPose);

	virtual void setStatic(bool on);
	virtual bool applyPressure(PxF32 pressure);

	virtual bool raycast(const PxVec3& rayOrigin, const PxVec3& rayDirection, PxF32& hitTime, PxVec3& hitNormal, PxU32& vertexIndex);
	virtual void attachVertexToGlobalPosition(PxU32 vertexIndex, const PxVec3& globalPosition);
	virtual void freeVertex(PxU32 vertexIndex);

	virtual void verifyTimeStep(PxF32 substepSize);

	virtual void setPositions(PxVec3* positions);
	virtual void setConstrainCoefficients(const tConstrainCoeffs* assetCoeffs, PxF32 maxDistanceBias, PxF32 maxDistanceScale, PxF32 maxDistanceDeform, PxF32 actorScale);
	virtual void getVelocities(PxVec3* velocities) const;
	virtual void setVelocities(PxVec3* velocities);


	virtual void setSolverIterations(PxU32 iterations);
	virtual void updateConstrainPositions(bool isDirty);
	virtual bool applyClothingMaterial(tMaterial* material, PxVec3 scaledGravity);
	virtual void applyClothingDesc(tClothingDescTemplate& clothingTemplate);

	virtual NxCompartment* getDeformableCompartment() const;
	virtual void enableCCD(bool on);

protected:
	PxU32 applyMaterialFlags(PxU32 flags, tMaterial& material) const;

	NxSoftBody* mNxSoftBody;
	NxCompartment* mCompartment;

};

}
} // namespace apex
} // namespace physx


#endif // SIMULATION_NX_SOFTBODY_H
