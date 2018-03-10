/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CLOTHING_ASSET_H
#define CLOTHING_ASSET_H

#include "NxClothingAsset.h"

#include "ApexInterface.h"
#include "PxMemoryBuffer.h"
#include "ApexRWLockable.h"
#include "ClothingCooking.h"
#include "ModuleClothing.h"
#include "ModuleClothingHelpers.h"

#include "NiApexRenderDebug.h"
#include "ApexAssetTracker.h"
#include "NxParamArray.h"

#include "ReadCheck.h"
#include "WriteCheck.h"

class NxClothDesc;
class NxSoftBodyDesc;

namespace physx
{
namespace apex
{
struct NxAbstractMeshDescription;
class ApexActorSource;
class ApexRenderMeshAsset;
class NiApexPhysXObjectDesc;
class NiApexRenderMeshAsset;

template <class T_Module, class T_Asset, class T_AssetAuthoring>
class ApexAuthorableObject;

namespace clothing
{
typedef ClothingGraphicalLodParametersNS::SkinClothMapB_Type SkinClothMapB;
typedef ClothingGraphicalLodParametersNS::SkinClothMapD_Type SkinClothMap;
typedef ClothingGraphicalLodParametersNS::TetraLink_Type TetraLink;
typedef ClothingPhysicalMeshParametersNS::PhysicalLod_Type PhysicalLod;
typedef ClothingPhysicalMeshParametersNS::PhysicalSubmesh_Type PhysicalSubmesh;

class ClothingActor;
class ClothingActorProxy;
class ClothingAssetData;
class ClothingMaterial;
class ClothingPhysicalMesh;
class CookingAbstract;
class ClothingPreviewProxy;

class SimulationAbstract;
class ClothingPlane;

#define NUM_VERTICES_PER_CACHE_BLOCK 8 // 128 / sizeof boneWeights per vertex (4 PxF32), this is the biggest per vertex data


#define DEFAULT_PM_OFFSET_ALONG_NORMAL_FACTOR 0.1f // Magic value that phil introduced for the mesh-mesh-skinning

struct TetraEncoding
{
	PxF32 sign[4];
	PxU32 lastVtxIdx;
};



#define TETRA_LUT_SIZE 6
static const TetraEncoding tetraTable[TETRA_LUT_SIZE] =
{
	{ { 0,   0,  0, 1 },  0},
	{ { 1,   0,  0, 1 },  2},
	{ { 1,   0,  1, 1 },  1},

	{ { -1, -1, -1, 0 },  0},
	{ { 0,  -1, -1, 0 },  2},
	{ { 0,  -1,  0, 0 },  1}
};



struct ClothingGraphicalMeshAssetWrapper
{
	ClothingGraphicalMeshAssetWrapper(const NxRenderMeshAsset* renderMeshAsset) : meshAsset(renderMeshAsset)
	{
	}
	const NxRenderMeshAsset* meshAsset;

	PxU32 getSubmeshCount() const
	{
		if (meshAsset == NULL)
			return 0;

		return meshAsset->getSubmeshCount();
	}

	PxU32 getNumTotalVertices() const
	{
		if (meshAsset == NULL)
			return 0;

		PxU32 count = 0;
		for (PxU32 i = 0; i < meshAsset->getSubmeshCount(); i++)
		{
			count += meshAsset->getSubmesh(i).getVertexCount(0); // only 1 part is supported
		}
		return count;
	}

	PxU32 getNumVertices(PxU32 submeshIndex) const
	{
		if (meshAsset == NULL)
			return 0;

		return meshAsset->getSubmesh(submeshIndex).getVertexBuffer().getVertexCount();
	}

	PxU32 getNumBonesPerVertex(PxU32 submeshIndex) const
	{
		if (meshAsset == NULL)
			return 0;

		if (submeshIndex < meshAsset->getSubmeshCount())
		{
			const NxVertexFormat& format = meshAsset->getSubmesh(submeshIndex).getVertexBuffer().getFormat();
			return vertexSemanticFormatElementCount(NxRenderVertexSemantic::BONE_INDEX, 
				format.getBufferFormat((physx::PxU32)format.getBufferIndexFromID(format.getSemanticID(NxRenderVertexSemantic::BONE_INDEX))));
		}
		return 0;
	}

	const void* getVertexBuffer(PxU32 submeshIndex, NxRenderVertexSemantic::Enum semantic, NxRenderDataFormat::Enum& outFormat) const
	{
		if (meshAsset == NULL)
			return NULL;

		const NxVertexBuffer& vb = meshAsset->getSubmesh(submeshIndex).getVertexBuffer();
		const NxVertexFormat& vf = vb.getFormat();
		physx::PxU32 bufferIndex = (physx::PxU32)vf.getBufferIndexFromID(vf.getSemanticID((NxRenderVertexSemantic::Enum)semantic));
		return vb.getBufferAndFormat(outFormat, bufferIndex);
	}

	PxU32 getNumIndices(PxU32 submeshIndex)
	{
		if (meshAsset == NULL)
			return 0;

		return meshAsset->getSubmesh(submeshIndex).getIndexCount(0);
	}

	const void* getIndexBuffer(PxU32 submeshIndex)
	{
		if (meshAsset == NULL)
			return NULL;

		return meshAsset->getSubmesh(submeshIndex).getIndexBuffer(0);
	}

	bool hasChannel(const char* bufferName = NULL, NxRenderVertexSemantic::Enum semantic = NxRenderVertexSemantic::NUM_SEMANTICS) const
	{
		if (meshAsset == NULL)
			return false;

		PX_ASSERT((bufferName != NULL) != (semantic != NxRenderVertexSemantic::NUM_SEMANTICS));
		PX_ASSERT((bufferName == NULL) != (semantic == NxRenderVertexSemantic::NUM_SEMANTICS));

		for (PxU32 i = 0; i < meshAsset->getSubmeshCount(); i++)
		{
			NxRenderDataFormat::Enum outFormat = NxRenderDataFormat::UNSPECIFIED;
			const NxVertexFormat& format = meshAsset->getSubmesh(i).getVertexBuffer().getFormat();

			NxVertexFormat::BufferID id = bufferName ? format.getID(bufferName) : format.getSemanticID(semantic);
			outFormat = format.getBufferFormat((physx::PxU32)format.getBufferIndexFromID(id));

			if (outFormat != NxRenderDataFormat::UNSPECIFIED)
			{
				return true;
			}
		}
		return false;
	}
private:
	void operator=(ClothingGraphicalMeshAssetWrapper&);
};



class ClothingAsset : public NxClothingAsset, public NxApexResource, public ApexResource, public ClothingCookingLock, public NxParameterized::SerializationCallback, public ApexRWLockable
{
protected:
	// used for authoring asset creation only!
	ClothingAsset(ModuleClothing* module, NxResourceList& list, const char* name);
	ClothingAsset(ModuleClothing*, NxResourceList&, NxParameterized::Interface*, const char*);

public:
	APEX_RW_LOCKABLE_BOILERPLATE

	PxU32 initializeAssetData(ClothingAssetData& assetData, const PxU32 uvChannel);

	// from NxApexAsset
	PX_INLINE const char*							getName() const
	{
		return mName.c_str();
	}
	PX_INLINE NxAuthObjTypeID						getObjTypeID() const
	{
		return mAssetTypeID;
	}
	PX_INLINE const char*							getObjTypeName() const
	{
		return NX_CLOTHING_AUTHORING_TYPE_NAME;
	}

	virtual PxU32									forceLoadAssets();
	virtual NxParameterized::Interface*				getDefaultActorDesc();
	virtual NxParameterized::Interface*				getDefaultAssetPreviewDesc();
	virtual const NxParameterized::Interface*		getAssetNxParameterized() const
	{
		return mParams;
	}
	virtual NxApexActor*							createApexActor(const NxParameterized::Interface& params, NxApexScene& apexScene);

	virtual NxApexAssetPreview*						createApexAssetPreview(const ::NxParameterized::Interface& /*params*/, NxApexAssetPreviewScene* /*previewScene*/)
	{
		PX_ALWAYS_ASSERT();
		return NULL;
	}

	virtual NxParameterized::Interface*				releaseAndReturnNxParameterizedInterface();
	virtual bool									isValidForActorCreation(const ::NxParameterized::Interface& parms, NxApexScene& apexScene) const;
	virtual bool									isDirty() const;

	// from NxApexInterface
	virtual void									release();

	// from NxClothingAsset
	PX_INLINE PxU32									getNumActors() const
	{
		NX_READ_ZONE();
		return mActors.getSize();
	}
	virtual NxClothingActor*						getActor(PxU32 index);
	PX_INLINE PxBounds3								getBoundingBox() const
	{
		NX_READ_ZONE();
		return mParams->boundingBox;
	}
	virtual PxF32									getMaximumSimulationBudget(PxU32 solverIterations) const;
	virtual PxU32									getNumGraphicalLodLevels() const;
	virtual PxU32									getGraphicalLodValue(PxU32 lodLevel) const;
	virtual PxF32									getBiggestMaxDistance() const;
	virtual bool									remapBoneIndex(const char* name, PxU32 newIndex);
	PX_INLINE PxU32									getNumBones() const
	{
		NX_READ_ZONE();
		return mBones.size();
	}
	PX_INLINE PxU32									getNumUsedBones() const
	{
		NX_READ_ZONE();
		return mParams->bonesReferenced;
	}
	PX_INLINE PxU32									getNumUsedBonesForMesh() const
	{
		return mParams->bonesReferencedByMesh;
	}
	virtual const char*								getBoneName(PxU32 internalIndex) const;
	virtual bool									getBoneBasePose(PxU32 internalIndex, PxMat44& result) const;
	virtual void									getBoneMapping(PxU32* internal2externalMap) const;
	virtual PxU32									prepareMorphTargetMapping(const PxVec3* originalPositions, PxU32 numPositions, PxF32 epsilon);

	// from ApexResource
	PxU32											getListIndex() const
	{
		NX_READ_ZONE();
		return m_listIndex;
	}
	void											setListIndex(class NxResourceList& list, PxU32 index)
	{
		m_list = &list;
		m_listIndex = index;
	}

	// from NxParameterized::SerializationCallback
	virtual void									preSerialize(void* userData);

	// graphical meshes
	PX_INLINE PxU32									getNumGraphicalMeshes() const
	{
		return mGraphicalLods.size();
	}

	NiApexRenderMeshAsset*							getGraphicalMesh(PxU32 index);
	const ClothingGraphicalLodParameters*			getGraphicalLod(PxU32 index) const;

	// actor handling
	void											releaseClothingActor(NxClothingActor& actor);
	void											releaseClothingPreview(NxClothingPreview& preview);

	// module stuff
	PX_INLINE ModuleClothing*						getModule() const
	{
		PX_ASSERT(mModule != NULL);
		return mModule;
	}

	// actor access to the asset
	bool											writeBoneMatrices(PxMat44 localPose, const PxMat44* newBoneMatrices,
	        const PxU32 byteStride, const PxU32 numBones, PxMat44* dest, bool isInternalOrder, bool multInvBindPose);
	ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* getPhysicalMeshFromLod(PxU32 graphicalLodId) const;

#if NX_SDK_VERSION_MAJOR == 2
	void											hintSceneDeletion(NxScene* deletedScene);
#elif NX_SDK_VERSION_MAJOR == 3
	void											hintSceneDeletion(PxScene* deletedScene);
#endif
	void											releaseCookedInstances();

	PX_INLINE bool									getSimulationDisableCCD() const
	{
		return mParams->simulation.disableCCD;
	}
	ClothingPhysicalMeshParametersNS::SkinClothMapB_Type* getTransitionMapB(PxU32 dstPhysicalMeshId, PxU32 srcPhysicalMeshId, PxF32& thickness, PxF32& offset);
	ClothingPhysicalMeshParametersNS::SkinClothMapD_Type* getTransitionMap(PxU32 dstPhysicalMeshId, PxU32 srcPhysicalMeshId, PxF32& thickness, PxF32& offset);

	// cooked stuff
	NxParameterized::Interface*						getCookedData(PxF32 actorScale);
	PxU32											getCookedPhysXVersion() const;
	ClothSolverMode::Enum							getClothSolverMode() const;

	// create deformables
	SimulationAbstract*								getSimulation(PxU32 physicalMeshId, PxU32 submeshId, NxParameterized::Interface* cookedParam, ClothingScene* clothingScene);
	void											returnSimulation(SimulationAbstract* simulation);
	void											destroySimulation(SimulationAbstract* simulation);

	void											initCollision(	SimulationAbstract* simulation, const PxMat44* boneTansformations,
																	NxResourceList& actorPlanes,
																	NxResourceList& actorConvexes,
																	NxResourceList& actorSpheres,
																	NxResourceList& actorCapsules,
																	NxResourceList& actorTriangleMeshes,
																	const ClothingActorParam* actorParam,
																	const PxMat44& globalPose, bool localSpaceSim);

	void											updateCollision(SimulationAbstract* simulation, const PxMat44* boneTansformationse,
																	NxResourceList& actorPlanes,
																	NxResourceList& actorConvexes,
																	NxResourceList& actorSpheres,
																	NxResourceList& actorCapsules,
																	NxResourceList& actorTriangleMeshes,
																	bool teleport);


	// physical meshes
	PxU32											getNumPhysicalLods(PxU32 graphicalLodId) const;
	const PhysicalLod*								getPhysicalLodData(PxU32 graphicalLodId, PxU32 physicsLod) const;
	const PhysicalSubmesh*							getPhysicalSubmesh(PxU32 graphicalLodId, PxU32 submeshId) const;
	PxU32											getPhysicalMeshID(PxU32 graphicalLodId) const;

	// bone stuff
	PX_INLINE const PxMat34Legacy&					getBoneBindPose(PxU32 i)
	{
		PX_ASSERT(i < mBones.size());
		return mBones[i].bindPose;
	}

	PX_INLINE PxU32									getBoneExternalIndex(PxU32 i)
	{
		PX_ASSERT(i < mBones.size());
		return (physx::PxU32)mBones[i].externalIndex;
	}

	// debug rendering
	void											visualizeSkinCloth(NiApexRenderDebug& renderDebug,
																		NxAbstractMeshDescription& srcPM, bool showTets, PxF32 actorScale) const;
	void											visualizeSkinClothMap(NiApexRenderDebug& renderDebug, NxAbstractMeshDescription& srcPM,
																		SkinClothMapB* skinClothMapB, PxU32 skinClothMapBSize,
																		SkinClothMap* skinClothMap, PxU32 skinClothMapSize,
																		PxF32 actorScale, bool onlyBad, bool invalidBary) const;
	void											visualizeBones(NiApexRenderDebug& renderDebug, const PxMat44* matrices, bool skeleton, PxF32 boneFramesScale, PxF32 boneNamesScale);


	// expose render data
	virtual const NxRenderMeshAsset*				getRenderMeshAsset(PxU32 lodLevel) const;
	virtual PxU32									getMeshSkinningMapSize(PxU32 lod);
	virtual void									getMeshSkinningMap(PxU32 lod, ClothingMeshSkinningMap* map);
	virtual bool									releaseGraphicalData();

	void											setupInvBindMatrices();

	// unified cooking
	void											prepareCookingJob(CookingAbstract& job, PxF32 scale, PxVec3* gravityDirection, PxVec3* morphedPhysicalMesh);

	// morph targets
	PxU32*											getMorphMapping(PxU32 graphicalLod, PxU32 submeshIndex);
	PxU32											getPhysicalMeshOffset(PxU32 physicalMeshId);
	void											getDisplacedPhysicalMeshPositions(PxVec3* morphDisplacements, NxParamArray<PxVec3> displacedMeshPositions);

	// faster cpu skinning
	void											initializeCompressedNumBonesPerVertex();
	PxU32											getRootBoneIndex();

	PxU32											getInterCollisionChannels();

protected:
	void											destroy();

	PxI32											getBoneInternalIndex(const char* boneName) const;
	PxI32											getBoneInternalIndex(PxU32 boneIndex) const;

	bool											reorderGraphicsVertices(PxU32 graphicalLodId, bool perfWarning);
	bool											reorderDeformableVertices(ClothingPhysicalMesh& physicalMesh);

	PxF32											getMaxMaxDistance(ClothingPhysicalMeshParametersNS::PhysicalMesh_Type& physicalMesh, 
																		PxU32 index, PxU32 numIndices) const;

	PxU32											getCorrespondingPhysicalVertices(const ClothingGraphicalLodParameters& graphLod, PxU32 submeshIndex,
																					PxU32 graphicalVertexIndex, const NxAbstractMeshDescription& pMesh,
																					PxU32 submeshVertexOffset, PxU32 indices[4], PxF32 trust[4]) const;

	void											getNormalsAndVerticesForFace(PxVec3* vtx, PxVec3* nrm, PxU32 i,
																					const NxAbstractMeshDescription& srcPM) const;
	bool											setBoneName(PxU32 internalIndex, const char* name);
	void											clearMapping(PxU32 graphicalLodId);
	void											updateBoundingBox();

	bool											mergeMapping(ClothingGraphicalLodParameters* graphicalLod);
	bool											findTriangleForImmediateVertex(PxU32& faceIndex, PxU32& indexInTriangle, PxU32 physVertIndex, 
																					ClothingPhysicalMeshParametersNS::PhysicalMesh_Type& physicalMesh) const;

#ifndef WITHOUT_PVD
	void											initPvdInstances(physx::debugger::comm::PvdDataStream& pvdStream);
	void											destroyPvdInstances();
#endif

	// the parent
	ModuleClothing* mModule;

	// the parameterized object
	ClothingAssetParameters* mParams;

	// the meshes
	NxParamArray<ClothingPhysicalMeshParameters*> mPhysicalMeshes;

	NxParamArray<ClothingGraphicalLodParameters*> mGraphicalLods;

	mutable NxParamArray<ClothingAssetParametersNS::BoneEntry_Type> mBones;
	Array<PxMat34Legacy> mInvBindPoses; // not serialized!

	mutable NxParamArray<ClothingAssetParametersNS::BoneSphere_Type> mBoneSpheres;
	mutable NxParamArray<PxU16> mSpherePairs;
	mutable NxParamArray<ClothingAssetParametersNS::ActorEntry_Type> mBoneActors;
	mutable NxParamArray<PxVec3> mBoneVertices;
	mutable NxParamArray<ClothingAssetParametersNS::BonePlane_Type> mBonePlanes;
	mutable NxParamArray<PxU32> mCollisionConvexes; // bitmap for indices into mBonePlanes array

private:
	// internal methods
	PxF32												getMaxDistReduction(ClothingPhysicalMeshParameters& physicalMesh, PxF32 maxDistanceMultiplier) const;

	static const char*									getClassName()
	{
		return NX_CLOTHING_AUTHORING_TYPE_NAME;
	}

	Array<PxU32>										mCompressedNumBonesPerVertex;
	Array<PxU32>										mCompressedTangentW;
	shdfnd::Mutex										mCompressedNumBonesPerVertexMutex;

	ApexSimpleString									mName;

	// Keep track of all ClothingActor objects created from this ClothingAsset
	NxResourceList										mActors;
	NxResourceList										mPreviews;


	Array<SimulationAbstract*>							mUnusedSimulation;
	shdfnd::Mutex										mUnusedSimulationMutex;

	static NxAuthObjTypeID mAssetTypeID;
	friend class ModuleClothing;


	Array<PxU32>										mExt2IntMorphMapping;
	PxU32												mExt2IntMorphMappingMaxValue; // this is actually one larger than max

	bool												mDirty;
	bool												mMorphMappingWarning;

	template <class T_Module, class T_Asset, class T_AssetAuthoring>
	friend class physx::apex::ApexAuthorableObject;
};

}
}
} // namespace physx::apex

#endif // CLOTHING_ASSET_H
