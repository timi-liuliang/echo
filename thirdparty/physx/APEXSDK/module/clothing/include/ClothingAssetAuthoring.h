/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CLOTHING_ASSET_AUTHORING_H
#define CLOTHING_ASSET_AUTHORING_H


#include "NxClothingAssetAuthoring.h"
#include "ClothingAsset.h"
#include "ClothingGraphicalLodParameters.h"
#include "ApexAssetAuthoring.h"

#include "ReadCheck.h"
#include "WriteCheck.h"

#ifndef WITHOUT_APEX_AUTHORING

namespace physx
{
namespace apex
{
namespace clothing
{

class ClothingPhysicalMesh;


class ClothingAssetAuthoring : public NxClothingAssetAuthoring, public ApexAssetAuthoring, public ClothingAsset
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ClothingAssetAuthoring(ModuleClothing* module, NxResourceList& list, const char* name);
	ClothingAssetAuthoring(ModuleClothing* module, NxResourceList& list);
	ClothingAssetAuthoring(ModuleClothing* module, NxResourceList& list, NxParameterized::Interface* params, const char* name);

	// from NxApexAssetAuthoring
	virtual const char* getName(void) const
	{
		return ClothingAsset::getName();
	}
	virtual const char* getObjTypeName() const
	{
		return NX_CLOTHING_AUTHORING_TYPE_NAME;
	}
	virtual bool							prepareForPlatform(physx::apex::NxPlatformTag);

	virtual void setToolString(const char* toolName, const char* toolVersion, PxU32 toolChangelist)
	{
		ApexAssetAuthoring::setToolString(toolName, toolVersion, toolChangelist);
	}

	// from NxApexInterface
	virtual void							release();

	// from NxClothingAssetAuthoring
	virtual void							setDefaultConstrainCoefficients(const NxClothingConstrainCoefficients& coeff)
	{
		NX_WRITE_ZONE();
		mDefaultConstrainCoefficients = coeff;
	}
	virtual void							setInvalidConstrainCoefficients(const NxClothingConstrainCoefficients& coeff)
	{
		NX_WRITE_ZONE();
		mInvalidConstrainCoefficients = coeff;
	}

	virtual void							setMeshes(PxU32 lod, NxRenderMeshAssetAuthoring* asset, NxClothingPhysicalMesh* mesh,
													PxU32 numMaxDistReductions = 0, PxF32* maxDistReductions = NULL, 
													PxF32 normalResemblance = 90, bool ignoreUnusedVertices = true, 
													IProgressListener* progress = NULL);
	virtual bool							addPlatformToGraphicalLod(PxU32 lod, NxPlatformTag platform);
	virtual bool							removePlatform(PxU32 lod,  NxPlatformTag platform);
	virtual PxU32							getNumPlatforms(PxU32 lod) const;
	virtual NxPlatformTag					getPlatform(PxU32 lod, PxU32 i) const;
	virtual PxU32							getNumLods() const;
	virtual PxI32							getLodValue(PxU32 lod) const;
	virtual void							clearMeshes();
	virtual NxClothingPhysicalMesh*			getClothingPhysicalMesh(PxU32 graphicalLod) const;

	virtual void							setBoneInfo(PxU32 boneIndex, const char* boneName, const PxMat44& bindPose, PxI32 parentIndex);
	virtual void							setRootBone(const char* boneName);
	virtual PxU32							addBoneConvex(const char* boneName, const PxVec3* positions, PxU32 numPositions);
	virtual PxU32							addBoneConvex(PxU32 boneIndex, const PxVec3* positions, PxU32 numPositions);
	virtual void							addBoneCapsule(const char* boneName, PxF32 capsuleRadius, PxF32 capsuleHeight, const PxMat44& localPose);
	virtual void							addBoneCapsule(PxU32 boneIndex, PxF32 capsuleRadius, PxF32 capsuleHeight, const PxMat44& localPose);
	virtual void							clearBoneActors(const char* boneName);
	virtual void							clearBoneActors(PxU32 boneIndex);
	virtual void							clearAllBoneActors();

	virtual void							setCollision(const char** boneNames, PxF32* radii, physx::PxVec3* localPositions, 
														PxU32 numSpheres, PxU16* pairs, PxU32 numPairs);
	virtual void							setCollision(PxU32* boneIndices, PxF32* radii, PxVec3* localPositions, PxU32 numSpheres, 
														PxU16* pairs, PxU32 numPairs);
	virtual void							clearCollision();

	virtual void							setSimulationHierarchicalLevels(PxU32 levels)
	{
		NX_WRITE_ZONE();
		mParams->simulation.hierarchicalLevels = levels;
		clearCooked();
	}
	virtual void							setSimulationThickness(physx::PxF32 thickness)
	{
		NX_WRITE_ZONE();
		mParams->simulation.thickness = thickness;
	}
	virtual void							setSimulationVirtualParticleDensity(physx::PxF32 density)
	{
		NX_WRITE_ZONE();
		PX_ASSERT(density >= 0.0f);
		PX_ASSERT(density <= 1.0f);
		mParams->simulation.virtualParticleDensity = PxClamp(density, 0.0f, 1.0f);
	}
	virtual void							setSimulationSleepLinearVelocity(physx::PxF32 sleep)
	{
		NX_WRITE_ZONE();
		mParams->simulation.sleepLinearVelocity = sleep;
	}
	virtual void							setSimulationGravityDirection(const physx::PxVec3& gravity)
	{
		NX_WRITE_ZONE();
		mParams->simulation.gravityDirection = gravity.getNormalized();
	}

	virtual void							setSimulationDisableCCD(bool disable)
	{
		NX_WRITE_ZONE();
		mParams->simulation.disableCCD = disable;
	}
	virtual void							setSimulationTwowayInteraction(bool enable)
	{
		NX_WRITE_ZONE();
		mParams->simulation.twowayInteraction = enable;
	}
	virtual void							setSimulationUntangling(bool enable)
	{
		NX_WRITE_ZONE();
		mParams->simulation.untangling = enable;
	}
	virtual void							setSimulationRestLengthScale(float scale)
	{
		NX_WRITE_ZONE();
		mParams->simulation.restLengthScale = scale;
	}

	virtual void							setExportScale(physx::PxF32 scale)
	{
		NX_WRITE_ZONE();
		mExportScale = scale;
	}
	virtual void							applyTransformation(const physx::PxMat44& transformation, physx::PxF32 scale, bool applyToGraphics, bool applyToPhysics);
	virtual void							updateBindPoses(const physx::PxMat44* newBindPoses, physx::PxU32 newBindPosesCount, bool isInternalOrder, bool collisionMaintainWorldPose);
	virtual void							setDeriveNormalsFromBones(bool enable)
	{
		NX_WRITE_ZONE();
		mDeriveNormalsFromBones = enable;
	}
	virtual NxParameterized::Interface*		getMaterialLibrary();
	virtual bool							setMaterialLibrary(NxParameterized::Interface* materialLibrary, physx::PxU32 materialIndex, bool transferOwnership);
	virtual NxParameterized::Interface*		getRenderMeshAssetAuthoring(physx::PxU32 lodLevel) const;

	// parameterization
	NxParameterized::Interface*				getNxParameterized() const
	{
		return mParams;
	}
	virtual NxParameterized::Interface*		releaseAndReturnNxParameterizedInterface();

	// from NxParameterized::SerializationCallback
	virtual void							preSerialize(void* userData);

	// from ApexAssetAuthoring
	virtual void							setToolString(const char* toolString);

	// internal
	void									destroy();

	virtual bool							setBoneBindPose(PxU32 boneIndex, const PxMat44& bindPose);
	virtual bool							getBoneBindPose(PxU32 boneIndex, PxMat44& bindPose) const;

private:
	// bones
	PxU32									addBoneConvexInternal(PxU32 boneIndex, const PxVec3* positions, PxU32 numPositions);
	void									addBoneCapsuleInternal(PxU32 boneIndex, PxF32 capsuleRadius, PxF32 capsuleHeight, const PxMat44& localPose);
	void									clearBoneActorsInternal(PxI32 internalBoneIndex);
	void									compressBones() const;
	void									compressBoneCollision();
	void									collectBoneIndices(PxU32 numVertices, const PxU16* boneIndices, const PxF32* boneWeights, PxU32 numBonesPerVertex) const;

	void									updateMappingAuthoring(ClothingGraphicalLodParameters& graphLod, NiApexRenderMeshAsset* renderMeshAssetCopy,
																	NiApexRenderMeshAssetAuthoring* renderMeshAssetOrig, PxF32 normalResemblance, 
																	bool ignoreUnusedVertices, IProgressListener* progress);
	void									sortSkinMapB(SkinClothMapB* skinClothMap, PxU32 skinClothMapSize, PxU32* immediateClothMap, PxU32 immediateClothMapSize);

	void									setupPhysicalLods(ClothingPhysicalMeshParameters& physicalMeshParameters, PxU32 numMaxDistReductions, PxF32* borderMaxdistances) const;

	void									distributeSolverIterations();

	bool									checkSetMeshesInput(PxU32 lod, NxClothingPhysicalMesh* nxPhysicalMesh, PxU32& graphicalLodIndexTest);
	void									sortPhysicalMeshes();

	// mesh reordering
	void									sortDeformableIndices(ClothingPhysicalMesh& physicalMesh);


	bool									getGraphicalLodIndex(PxU32 lod, PxU32& graphicalLodIndex) const;
	PxU32									addGraphicalLod(PxU32 lod);

	// cooking
	void									clearCooked();

	// access
	bool									addGraphicalMesh(NxRenderMeshAssetAuthoring* renderMesh, physx::PxU32 graphicalLodIndex);

	Array<ClothingPhysicalMesh*>			mPhysicalMeshesInput;

	PxF32									mExportScale;
	bool									mDeriveNormalsFromBones;
	bool									mOwnsMaterialLibrary;

	NxClothingConstrainCoefficients			mDefaultConstrainCoefficients;
	NxClothingConstrainCoefficients			mInvalidConstrainCoefficients;

	const char*								mPreviousCookedType;

	ApexSimpleString						mRootBoneName;

	void									initParams();

	// immediate cloth: 1-to-1 mapping from physical to rendering mesh (except for LOD)
	bool									generateImmediateClothMap(const NxAbstractMeshDescription* targetMeshes, PxU32 numTargetMeshes,
															ClothingPhysicalMeshParametersNS::PhysicalMesh_Type& physicalMesh, 
															PxU32* masterFlags, PxF32 epsilon, PxU32& numNotFoundVertices, 
															PxF32 normalResemblance, NxParamArray<PxU32>& result, IProgressListener* progress) const;
	bool									generateSkinClothMap(const NxAbstractMeshDescription* targetMeshes, PxU32 numTargetMeshes,
															ClothingPhysicalMeshParametersNS::PhysicalMesh_Type& physicalMesh, PxU32* masterFlags, 
															PxU32* immediateMap, PxU32 numEmptyInImmediateMap, NxParamArray<SkinClothMap>& result,
															PxF32& offsetAlongNormal, bool integrateImmediateMap, IProgressListener* progress) const;

	void									removeMaxDistance0Mapping(ClothingGraphicalLodParameters& graphicalLod, NiApexRenderMeshAsset* renderMeshAsset) const;

	bool									generateTetraMap(const NxAbstractMeshDescription* targetMeshes, PxU32 numTargetMeshes,
															ClothingPhysicalMeshParametersNS::PhysicalMesh_Type& physicalMesh, PxU32* masterFlags,
															NxParamArray<ClothingGraphicalLodParametersNS::TetraLink_Type>& result, IProgressListener* progress) const;
	PxF32									computeBaryError(PxF32 baryX, PxF32 baryY) const;
	PxF32									computeTriangleError(const TriangleWithNormals& triangle, const PxVec3& normal) const;


	bool									hasTangents(const NiApexRenderMeshAsset& rma);
	PxU32									getMaxNumGraphicalVertsActive(const ClothingGraphicalLodParameters& graphicalLod, PxU32 submeshIndex);
	bool									isMostlyImmediateSkinned(const NiApexRenderMeshAsset& rma, const ClothingGraphicalLodParameters& graphicalLod);
	bool									conditionalMergeMapping(const NiApexRenderMeshAsset& rma, ClothingGraphicalLodParameters& graphicalLod);

};

}
} // namespace apex
} // namespace physx

#endif // WITHOUT_APEX_AUTHORING

#endif // CLOTHING_ASSET_AUTHORING_H
