/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef DESTRUCTIBLE_ASSET_H
#define DESTRUCTIBLE_ASSET_H

#include "NxApex.h"
#include "PsArray.h"
#include "ApexInterface.h"
#include "ApexSDKHelpers.h"
#include "NxDestructibleAsset.h"
#include "ApexSharedUtils.h"
#include "ApexAssetTracker.h"
#include "DestructibleActorParam.h"
#include "PsShare.h"
#include "DestructibleAssetParameters.h"
#include "MeshCookedCollisionStreamsAtScale.h"
#include "MeshCookedCollisionStream.h"
#include "DestructibleAssetCollisionDataSet.h"
#include "CachedOverlaps.h"

#include "authoring/Fracturing.h"

#if NX_SDK_VERSION_MAJOR == 2
#include "NxActor.h"
#elif NX_SDK_VERSION_MAJOR == 3
#include "PhysX3ClassWrapper.h"
#include "ApexActor.h"
#endif
namespace physx
{
namespace apex
{

class NxDestructiblePreview;
class NxDestructiblePreviewDesc;

namespace destructible
{
class ModuleDestructible;


/**
	Descriptor used to create a Destructible actor.
*/
class NxDestructibleActorDesc : public NxApexDesc
{
public:

	/**
	\brief Constructor sets to default.
	*/
	PX_INLINE NxDestructibleActorDesc() : NxApexDesc()
	{
		setToDefault();
	}

	/**
	\brief Resets descriptor to default settings.
	*/
	PX_INLINE void setToDefault()
	{
		NxApexDesc::setToDefault();
#if NX_SDK_VERSION_MAJOR == 2
		actorDescTemplate = NULL;
		shapeDescTemplate = NULL;
		bodyDescTemplate = NULL;
#elif NX_SDK_VERSION_MAJOR == 3
		physX3Template	= NULL;
#endif
		crumbleEmitterName = NULL;
		dustEmitterName = NULL;
		globalPose = PxMat44::createIdentity();
		scale = physx::PxVec3(1.0f);
		dynamic = false;
		supportDepth = 0;
		formExtendedStructures = false;
		useAssetDefinedSupport = false;
		useWorldSupport = false;
		overrideSkinnedMaterials = NULL;
		overrideSkinnedMaterialCount = 0;
		overrideStaticMaterials = NULL;
		overrideStaticMaterialCount = 0;
		renderStaticChunksSeparately = true;
		destructibleParameters.setToDefault();
	}

	/**
		Returns true iff an object can be created using this descriptor.
	*/
	PX_INLINE bool isValid() const
	{
		const PxF32 det = (globalPose.column0.getXYZ().cross(globalPose.column1.getXYZ())).dot(globalPose.column2.getXYZ());
		if (!physx::PxEquals(det, 1.0f, 0.001f))
		{
			APEX_DEBUG_WARNING("Mirror transformations are not allowed in the NxDestructibleActor descriptor.");
			return false;
		}

		if (scale.x <= 0 || scale.y <= 0 || scale.z <= 0)
		{
			APEX_DEBUG_WARNING("Negative scales are not allowed in the NxDestructibleActor descriptor.");
			return false;
		}
#if NX_SDK_VERSION_MAJOR == 2
		if (actorDescTemplate && !actorDescTemplate->isValid())
		{
			APEX_DEBUG_WARNING("Invalid NxActor descriptor template in the NxDestructibleActor descriptor.");
			return false;
		}

		if (bodyDescTemplate && !bodyDescTemplate->isValid())
		{
			APEX_DEBUG_WARNING("Invalid NxActor body descriptor template in the NxDestructibleActor descriptor.");
			return false;
		}
#elif NX_SDK_VERSION_MAJOR == 3
		if (physX3Template && !physX3Template->isValid() )
		{
			APEX_DEBUG_WARNING("Invalid physx3 descriptor template in the NxDestructibleActor descriptor.");
			return false;
		}
#endif



		if (overrideSkinnedMaterialCount > 0 && overrideSkinnedMaterials == NULL)
		{
			APEX_DEBUG_WARNING("overrideSkinnedMaterials is NULL, but overrideSkinnedMaterialCount > 0.");
			return false;
		}

		if (overrideStaticMaterialCount > 0 && overrideStaticMaterials == NULL)
		{
			APEX_DEBUG_WARNING("overrideStaticMaterials is NULL, but overrideStaticMaterialCount > 0.");
			return false;
		}

		// Note - Not checking shapeDescTemplate, since meshData is not supplied

		return NxApexDesc::isValid();
	}

#if NX_SDK_VERSION_MAJOR == 2
	/**
		NxActorDesc fields which can be set by the user, used to create NxActors to represent destructibles.
	*/
	NxActorDescBase*			actorDescTemplate;

	/**
		NxShapeDesc fields which can be set by the user, used to create NxShapes to represent destructibles.
	*/
	NxShapeDesc*				shapeDescTemplate;

	/**
		NxBodyDesc fields which can be set by the user, used (in the NxActorDesc) to create NxActors to represent destructibles.
	*/
	NxBodyDesc*					bodyDescTemplate;
#elif NX_SDK_VERSION_MAJOR == 3
	PhysX3DescTemplate*			physX3Template;
#endif
	/**
		The name of the NxMeshParticleSystem to use for crumbling.  This overrides the crumble system defined
		in the NxDestructibleAsset if specified.
	*/
	const char*					crumbleEmitterName;

	/**
		The name of the NxMeshParticleSystem to use for fracture-line dust.  This overrides the dust system defined
		in the NxDestructibleAsset if specified.
	*/
	const char*					dustEmitterName;

	/**
		Initial global pose of undamaged destructible
	*/
	physx::PxMat44				globalPose;

	/**
		3D scale
	*/
	physx::PxVec3				scale;

	/**
		Whether or not the destructible starts life as a dynamic actor
	*/
	bool						dynamic;

	/**
		The chunk hierarchy depth at which to create a support graph.  Higher depth levels give more detailed support,
		but will give a higher computational load.  Chunks below the support depth will never be supported.
	*/
	physx::PxU32				supportDepth;

	/**
		If set, and the destructible is initially static, it will become part of an extended support
		structure if it is in contact with another static destructible that also has this flag set.
	*/
	bool						formExtendedStructures;

	/**
		If set, then chunks which are tagged as "support" chunks (via NxDestructibleChunkDesc::isSupportChunk)
		will have environmental support in static destructibles.

		Note: if both ASSET_DEFINED_SUPPORT and WORLD_SUPPORT are set, then chunks must be tagged as
		"support" chunks AND overlap the NxScene's static geometry in order to be environmentally supported.
	*/
	bool						useAssetDefinedSupport;

	/**
		If set, then chunks which overlap the NxScene's static geometry will have environmental support in
		static destructibles.

		Note: if both ASSET_DEFINED_SUPPORT and WORLD_SUPPORT are set, then chunks must be tagged as
		"support" chunks AND overlap the NxScene's static geometry in order to be environmentally supported.
	*/
	bool						useWorldSupport;

	/*
		If true, static chunks will be renderered separately from dynamic chunks, as a single mesh (not using skinning).
		This parameter is ignored if the 'dynamic' parameter is true.
		Default value = false.
	*/
	bool						renderStaticChunksSeparately;

	/**
		Per-actor material names for the skinned mesh
	*/
	const char**				overrideSkinnedMaterials;

	/**
		Size of overrideSkinnedMaterials array
	*/
	physx::PxU32				overrideSkinnedMaterialCount;

	/**
		Per-actor material names for the static mesh
	*/
	const char**				overrideStaticMaterials;

	/**
		Size of overrideStaticMaterials array
	*/
	physx::PxU32				overrideStaticMaterialCount;

	/**
		Initial destructible parameters.  These may be changed at runtime.
	*/
	NxDestructibleParameters	destructibleParameters;
};


/**
	Set of sets of convex meshes [scale#][part#]
*/
class DestructibleConvexMeshContainer
{
public:
	struct ConvexMeshSet : public UserAllocated
	{
		ConvexMeshSet() : mReferenceCount(0) {}

		physx::Array<NxConvexMesh*> mSet;
		physx::PxU32				mReferenceCount;
	};

					~DestructibleConvexMeshContainer() { reset(); }

	physx::PxU32					size() const
									{ 
										return mConvexMeshSets.size();
									}

	physx::Array<NxConvexMesh*>&	operator []	(physx::PxU32 i) const
									{ 
										return mConvexMeshSets[i]->mSet;
									}

	physx::PxU32					getReferenceCount(physx::PxU32 i) const 
									{ 
										return mConvexMeshSets[i]->mReferenceCount;
									}
	void							incReferenceCount(physx::PxU32 i) 
									{
										++mConvexMeshSets[i]->mReferenceCount;
									}
	bool							decReferenceCount(physx::PxU32 i) 
									{
										if (mConvexMeshSets[i]->mReferenceCount == 0) 
											return false; 
										--mConvexMeshSets[i]->mReferenceCount; 
										return true;
									}

	void							resize(physx::PxU32 newSize)
									{
										const physx::PxU32 oldSize = size();
										mConvexMeshSets.resize(newSize);
										for (physx::PxU32 i = oldSize; i < newSize; ++i)
										{
											mConvexMeshSets[i] = PX_NEW(ConvexMeshSet);
										}
									}

	void							reset(bool force = true)
									{
										for (physx::PxU32 i = mConvexMeshSets.size(); i--;)
										{
											ConvexMeshSet* meshSet = mConvexMeshSets[i];
											if (meshSet != NULL)
											{
												if (force)
												{
													PX_DELETE(meshSet);
													mConvexMeshSets[i] = NULL;
												}
												else
												{
													if (meshSet->mReferenceCount == 0)
													{
														// Release the PhysX convex mesh pointers
														for (physx::PxU32 j=0; j<meshSet->mSet.size(); j++)
														{
															NxConvexMesh* convexMesh = meshSet->mSet[j];
															if (convexMesh == NULL)
															{
																continue;
															}
															#if NX_SDK_VERSION_MAJOR == 2
																	NxGetApexSDK()->getPhysXSDK()->releaseConvexMesh(*convexMesh);
															#elif NX_SDK_VERSION_MAJOR == 3
																	convexMesh->release();
															#endif
														}
														
														meshSet->mSet.resize(0);
														PX_DELETE(meshSet);
														mConvexMeshSets.replaceWithLast(i);
													}
												}
											}
										}
										if (force)
										{
											mConvexMeshSets.reset();
										}
									}

private:
	physx::Array<ConvexMeshSet*>	mConvexMeshSets;
};


/**
	Destructible asset collision data.  Caches collision data for a DestructibleAsset at
	various scales.
*/

#define kDefaultDestructibleAssetCollisionScaleTolerance	(0.0001f)

class DestructibleAssetCollision : public ApexResource
{
public:

	struct Version
	{
		enum Enum
		{
			First = 0,
			// New versions must be put here.  There is no need to explicitly number them.  The
			// numbers above were put there to conform to the old DestructionToolStreamVersion enum.

			Count,
			Current = Count - 1
		};
	};

	DestructibleAssetCollision();
	DestructibleAssetCollision(NxParameterized::Interface* params);
	~DestructibleAssetCollision();

	void								setDestructibleAssetToCook(class DestructibleAsset* asset);

	bool								addScale(const physx::PxVec3& scale);

	bool								cookAll();

	bool								cookScale(const physx::PxVec3& scale);

	void								resize(physx::PxU32 hullCount);

	const char*							getAssetName() const
	{
		return mParams->assetName;
	}
	DestructibleAsset*					getAsset() { return mAsset; }
	NxConvexMesh*						getConvexMesh(physx::PxU32 hullIndex, const physx::PxVec3& scale);

	physx::PxFileBuf&					deserialize(physx::PxFileBuf& stream, const char* assetName);
	physx::PxFileBuf&					serialize(physx::PxFileBuf& stream) const;

	bool								platformAndVersionMatch() const;
	void								setPlatformAndVersion();

	physx::PxU32								memorySize() const;

	MeshCookedCollisionStreamsAtScale*	getCollisionAtScale(const physx::PxVec3& scale);

	physx::Array<NxConvexMesh*>*			getConvexMeshesAtScale(const physx::PxVec3& scale);

	void								clearUnreferencedSets();
	// Spit out warnings to the error stream for any referenced sets
	void								reportReferencedSets();
	bool								incReferenceCount(int scaleIndex);
	bool								decReferenceCount(int scaleIndex);

	void								merge(DestructibleAssetCollision& collisionSet);

	physx::PxI32						getScaleIndex(const physx::PxVec3& scale, physx::PxF32 tolerance) const;

private:
	DestructibleAssetCollisionDataSet*				mParams;
	bool											mOwnsParams;

	class DestructibleAsset*						mAsset;
	DestructibleConvexMeshContainer					mConvexMeshContainer;
};

#define OFFSET_FN(_classname, _membername) static physx::PxU32 _membername##Offset() { return PX_OFFSET_OF(_classname, _membername); }

class DestructibleAsset : public ApexResource
{
public:

	enum
	{
		InvalidChunkIndex =	0xFFFF
	};

	enum ChunkFlags
	{
		SupportChunk =	(1 << 0),
		UnfracturableChunk =	(1 << 1),
		DescendantUnfractureable =	(1 << 2),
		UndamageableChunk =	(1 << 3),
		UncrumbleableChunk =	(1 << 4),
#if APEX_RUNTIME_FRACTURE
		RuntimeFracturableChunk =	(1 << 5),
#endif

		Instanced = (1 << 8),
	};

	struct ChunkInstanceBufferDataElement
	{
		physx::PxVec3	translation;
		physx::PxMat33	scaledRotation;
		physx::PxVec2	uvOffset;
		physx::PxVec3	localOffset;

		OFFSET_FN(ChunkInstanceBufferDataElement, translation)
		OFFSET_FN(ChunkInstanceBufferDataElement, scaledRotation)
		OFFSET_FN(ChunkInstanceBufferDataElement, uvOffset)
		OFFSET_FN(ChunkInstanceBufferDataElement, localOffset)
	};

	struct ScatterInstanceBufferDataElement
	{
		physx::PxVec3	translation;
		physx::PxMat33	scaledRotation;
		physx::PxF32	alpha;

		OFFSET_FN(ScatterInstanceBufferDataElement, translation)
		OFFSET_FN(ScatterInstanceBufferDataElement, scaledRotation)
		OFFSET_FN(ScatterInstanceBufferDataElement, alpha)
	};

	struct ScatterMeshInstanceInfo
	{
		ScatterMeshInstanceInfo() 
		: m_actor(NULL)
		, m_instanceBuffer(NULL)
		, m_IBSize(0) 
		{}

		~ScatterMeshInstanceInfo();

		NxRenderMeshActor*								m_actor;
		NxUserRenderInstanceBuffer*						m_instanceBuffer;
		physx::PxU32									m_IBSize;
		physx::Array<ScatterInstanceBufferDataElement>	m_instanceBufferData;
	};

	DestructibleAsset(ModuleDestructible* module, NxDestructibleAsset* api, const char* name);
	DestructibleAsset(ModuleDestructible* module, NxDestructibleAsset* api, NxParameterized::Interface* params, const char* name);
	~DestructibleAsset();

	const NxParameterized::Interface* 	getAssetNxParameterized() const
	{
		return mParams;
	}

	NxDestructibleActor*				createDestructibleActorFromDeserializedState(NxParameterized::Interface* params, NxApexScene&);
	NxDestructibleActor*				createDestructibleActor(const NxParameterized::Interface& params, NxApexScene&);
	void								releaseDestructibleActor(NxDestructibleActor& actor);

	NxRenderMeshAsset* 					getRenderMeshAsset() const
	{
		return renderMeshAsset;
	}
	bool								setRenderMeshAsset(NxRenderMeshAsset* newRenderMeshAsset);

	bool								setScatterMeshAssets(NxRenderMeshAsset** scatterMeshAssetArray, physx::PxU32 scatterMeshAssetArraySize);

	void								createScatterMeshInstanceInfo();

	NxUserRenderInstanceBufferDesc		getScatterMeshInstanceBufferDesc();

	physx::PxU32						getScatterMeshAssetCount() const
	{
		return scatterMeshAssets.size();
	}

	virtual NxRenderMeshAsset* const *			getScatterMeshAssets() const
	{
		return scatterMeshAssets.size() > 0 ? &scatterMeshAssets[0] : NULL;
	}

	NxRenderMeshAsset*					getRuntimeRenderMeshAsset() const
	{
		return runtimeRenderMeshAsset;
	}

	physx::PxU32						getInstancedChunkMeshCount() const
	{
		return m_instancedChunkMeshCount;
	}

	physx::PxU32						getChunkCount() const
	{
		return (physx::PxU32)mParams->chunks.arraySizes[0];
	}
	physx::PxU32						getDepthCount() const
	{
		return mParams->depthCount;
	}
	physx::PxU32 getChunkChildCount(physx::PxU32 chunkIndex) const
	{
		if (chunkIndex >= (physx::PxU32)mParams->chunks.arraySizes[0])
		{
			return 0;
		}
		return (physx::PxU32)mParams->chunks.buf[chunkIndex].numChildren;
	}
	physx::PxU16 getChunkDepth(physx::PxU32 chunkIndex) const
	{
		if (chunkIndex >= (physx::PxU32)mParams->chunks.arraySizes[0])
		{
			return 0;
		}
		return mParams->chunks.buf[chunkIndex].depth;
	}
	physx::PxI32 getChunkChild(physx::PxU32 chunkIndex, physx::PxU32 childIndex) const
	{
		if (childIndex >= getChunkChildCount(chunkIndex))
		{
			return -1;
		}
		return physx::PxI32(mParams->chunks.buf[chunkIndex].firstChildIndex + childIndex);
	}
	physx::PxBounds3					getBounds() const
	{
		return mParams->bounds;
	}
	NxDestructibleParameters			getParameters() const;
	NxDestructibleInitParameters		getInitParameters() const;
	const char*							getCrumbleEmitterName() const;
	const char*							getDustEmitterName() const;
	const char*							getFracturePatternName() const;
	physx::PxF32						getFractureImpulseScale() const
	{
		return mParams->destructibleParameters.fractureImpulseScale;
	}
	physx::PxF32						getImpactVelocityThreshold() const
	{
		return mParams->destructibleParameters.impactVelocityThreshold;
	}
	void								getStats(NxDestructibleAssetStats& stats) const;
	void								cacheChunkOverlapsUpToDepth(physx::PxI32 depth = -1);
	physx::PxF32						getNeighborPadding() const
	{
		return mParams->neighborPadding;
	}

	physx::PxU16						getChunkParentIndex(physx::PxU32 chunkIndex) const 
	{
		PX_ASSERT(chunkIndex < (physx::PxU16)mParams->chunks.arraySizes[0]);
		return mParams->chunks.buf[chunkIndex].parentIndex;
	}

	physx::PxVec3						getChunkPositionOffset(physx::PxU32 chunkIndex) const
	{
		PX_ASSERT(chunkIndex < (physx::PxU16)mParams->chunks.arraySizes[0]);
		DestructibleAssetParametersNS::Chunk_Type& sourceChunk = mParams->chunks.buf[chunkIndex];
		return (sourceChunk.flags & DestructibleAsset::Instanced) == 0 ? physx::PxVec3(0.0f) : mParams->chunkInstanceInfo.buf[sourceChunk.meshPartIndex].chunkPositionOffset;
	}

	physx::PxVec2						getChunkUVOffset(physx::PxU32 chunkIndex) const
	{
		PX_ASSERT(chunkIndex < (physx::PxU16)mParams->chunks.arraySizes[0]);
		DestructibleAssetParametersNS::Chunk_Type& sourceChunk = mParams->chunks.buf[chunkIndex];
		return (sourceChunk.flags & DestructibleAsset::Instanced) == 0 ? physx::PxVec2(0.0f) : mParams->chunkInstanceInfo.buf[sourceChunk.meshPartIndex].chunkUVOffset;
	}

	physx::PxU32						getChunkFlags(physx::PxU32 chunkIndex) const
	{
		PX_ASSERT(chunkIndex < (physx::PxU16)mParams->chunks.arraySizes[0]);
		DestructibleAssetParametersNS::Chunk_Type& sourceChunk = mParams->chunks.buf[chunkIndex];
		physx::PxU32 flags = 0;
		if (sourceChunk.flags & DestructibleAsset::SupportChunk)
		{
			flags |= NxDestructibleAsset::ChunkEnvironmentallySupported;
		}
		if (sourceChunk.flags & DestructibleAsset::UnfracturableChunk)
		{
			flags |= NxDestructibleAsset::ChunkAndDescendentsDoNotFracture;
		}
		if (sourceChunk.flags & DestructibleAsset::UndamageableChunk)
		{
			flags |= NxDestructibleAsset::ChunkDoesNotFracture;
		}
		if (sourceChunk.flags & DestructibleAsset::UncrumbleableChunk)
		{
			flags |= NxDestructibleAsset::ChunkDoesNotCrumble;
		}
		if (sourceChunk.flags & DestructibleAsset::Instanced)
		{
			flags |= NxDestructibleAsset::ChunkIsInstanced;
		}
#if APEX_RUNTIME_FRACTURE
		if (sourceChunk.flags & DestructibleAsset::RuntimeFracturableChunk)
		{
			flags |= NxDestructibleAsset::ChunkRuntimeFracture;
		}
#endif
		return flags;
	}

	physx::PxU32						getPartIndex(physx::PxU32 chunkIndex) const
	{
		PX_ASSERT(chunkIndex < (physx::PxU16)mParams->chunks.arraySizes[0]);
		DestructibleAssetParametersNS::Chunk_Type& sourceChunk = mParams->chunks.buf[chunkIndex];
		return (sourceChunk.flags & DestructibleAsset::Instanced) == 0 ? sourceChunk.meshPartIndex : mParams->chunkInstanceInfo.buf[sourceChunk.meshPartIndex].partIndex;
	}

	physx::PxU32						getChunkHullIndexStart(physx::PxU32 chunkIndex) const
	{
		return mParams->chunkConvexHullStartIndices.buf[getPartIndex(chunkIndex)];
	}

	physx::PxU32						getChunkHullIndexStop(physx::PxU32 chunkIndex) const
	{
		return mParams->chunkConvexHullStartIndices.buf[getPartIndex(chunkIndex)+1];
	}

	physx::PxU32						getChunkHullCount(physx::PxU32 chunkIndex) const
	{
		const physx::PxU32 partIndex = getPartIndex(chunkIndex);
		return mParams->chunkConvexHullStartIndices.buf[partIndex+1] - mParams->chunkConvexHullStartIndices.buf[partIndex];
	}

	physx::PxU32						getPartHullIndexStart(physx::PxU32 partIndex) const
	{
		return mParams->chunkConvexHullStartIndices.buf[partIndex];
	}

	physx::PxU32						getPartHullIndexStop(physx::PxU32 partIndex) const
	{
		return mParams->chunkConvexHullStartIndices.buf[partIndex+1];
	}

	NxParameterized::Interface**		getConvexHullParameters(physx::PxU32 hullIndex) const
	{
		return mParams->chunkConvexHulls.buf + hullIndex;
	}

	physx::PxBounds3					getChunkActorLocalBounds(physx::PxU32 chunkIndex) const
	{
		const physx::PxU32 partIndex = getPartIndex(chunkIndex);
		physx::PxBounds3 bounds = renderMeshAsset->getBounds(partIndex);
		const physx::PxVec3 offset = getChunkPositionOffset(chunkIndex);
		bounds.minimum += offset;
		bounds.maximum += offset;
		return bounds;
	}

	const physx::PxBounds3&				getChunkShapeLocalBounds(physx::PxU32 chunkIndex) const
	{
		const physx::PxU32 partIndex = getPartIndex(chunkIndex);
		return renderMeshAsset->getBounds(partIndex);
	}

	void								applyTransformation(const physx::PxMat44& transformation, physx::PxF32 scale);
	void								applyTransformation(const physx::PxMat44& transformation);
	bool                                setPlatformMaxDepth(NxPlatformTag platform, physx::PxU32 maxDepth);
	bool                                removePlatformMaxDepth(NxPlatformTag platform);

	CachedOverlapsNS::IntPair_DynamicArray1D_Type*	getOverlapsAtDepth(physx::PxU32 depth, bool create = true) const;

	void											clearChunkOverlaps(physx::PxI32 depth = -1, bool keepCachedFlag = false);
	void											addChunkOverlaps(NxIntPair* supportGraphEdges, PxU32 numSupportGraphEdges);
	void											removeChunkOverlaps(NxIntPair* supportGraphEdges, PxU32 numSupportGraphEdges, bool keepCachedFlagIfEmpty);

	void								traceSurfaceBoundary(physx::Array<physx::PxVec3>& outPoints, physx::PxU16 chunkIndex, const physx::PxMat34Legacy& localToWorldRT, const physx::PxVec3& scale,
	        physx::PxF32 spacing, physx::PxF32 jitter, physx::PxF32 surfaceDistance, physx::PxU32 maxPoints);

	NxAuthObjTypeID						getObjTypeID() const
	{
		return mAssetTypeID;
	}
	const char* 						getObjTypeName() const
	{
		return getClassName();
	}
	const char*                         getName() const
	{
		return mName.c_str();
	}

	void								prepareForNewInstance();

	void								resetInstanceData();

	// NxDestructibleAssetAuthoring methods
	virtual bool						prepareForPlatform(physx::apex::NxPlatformTag platform) const;

	void								setFractureImpulseScale(physx::PxF32 scale)
	{
		mParams->destructibleParameters.fractureImpulseScale = scale;
	}
	void								setImpactVelocityThreshold(physx::PxF32 threshold)
	{
		mParams->destructibleParameters.impactVelocityThreshold = threshold;
	}
	void								setChunkOverlapsCacheDepth(physx::PxI32 depth = -1);
	void								setParameters(const NxDestructibleParameters&);
	void								setInitParameters(const NxDestructibleInitParameters&);
	void								setCrumbleEmitterName(const char*);
	void								setDustEmitterName(const char*);
	void								setFracturePatternName(const char*);
	void								setNeighborPadding(physx::PxF32 neighborPadding)
	{
		mParams->neighborPadding = neighborPadding;
	}

	physx::PxU32						forceLoadAssets();
	void								initializeAssetNameTable();

	void								cleanup();
	ModuleDestructible* 				getOwner()
	{
		return module;
	}
	NxDestructibleAsset* 				getNxAsset()
	{
		return mNxAssetApi;
	}

	static NxAuthObjTypeID				getAssetTypeID()
	{
		return mAssetTypeID;
	}

	virtual NxParameterized::Interface* getDefaultActorDesc();
	virtual NxParameterized::Interface* getDefaultAssetPreviewDesc();
	virtual NxApexActor*				createApexActor(const NxParameterized::Interface& params, NxApexScene& apexScene);

	virtual NxApexAssetPreview*			createApexAssetPreview(const ::NxParameterized::Interface& /*params*/, NxApexAssetPreviewScene* /*previewScene*/);

	virtual bool						isValidForActorCreation(const ::NxParameterized::Interface& /*parms*/, NxApexScene& /*apexScene*/) const;

	physx::PxU32						getActorTransformCount() const
	{
		return (physx::PxU32)mParams->actorTransforms.arraySizes[0];
	}

	const physx::PxMat44*				getActorTransforms() const
	{
		return mParams->actorTransforms.buf;
	}

	void								appendActorTransforms(const physx::PxMat44* transforms, physx::PxU32 transformCount);

	void								clearActorTransforms();

	bool								rebuildCollisionGeometry(physx::PxU32 partIndex, const NxDestructibleGeometryDesc& geometryDesc);


	NxParameterized::Interface*			acquireNxParameterizedInterface(void)
	{
		NxParameterized::Interface* ret = mParams;
		
		// don't set mParams to NULL here, because it's read during the asset release
		mOwnsParams = false;

		return ret;
	}

protected:

	void init();

	void calculateChunkDepthStarts();
	void calculateChunkOverlaps(physx::Array<IntPair>& overlaps, physx::PxU32 depth) const;

	void reduceAccordingToLOD();

	void updateChunkInstanceRenderResources(bool rewriteBuffers, void* userRenderData);

	static bool chunksInProximity(const DestructibleAsset& asset0, physx::PxU16 chunkIndex0, const physx::PxMat44& tm0, const physx::PxVec3& scale0, 
								  const DestructibleAsset& asset1, physx::PxU16 chunkIndex1, const physx::PxMat44& tm1, const physx::PxVec3& scale1,
								  physx::PxF32 padding);

	bool chunkAndSphereInProximity(physx::PxU16 chunkIndex, const physx::PxMat44& chunkTM, const physx::PxVec3& chunkScale, 
								   const physx::PxVec3& sphereWorldCenter, physx::PxF32 sphereRadius, physx::PxF32 padding, physx::PxF32* distance);

	static NxDestructibleParameters getParameters(const DestructibleAssetParametersNS::DestructibleParameters_Type&,
												  const DestructibleAssetParametersNS::DestructibleDepthParameters_DynamicArray1D_Type*);
	static void setParameters(const NxDestructibleParameters&, DestructibleAssetParametersNS::DestructibleParameters_Type&);

	static NxAuthObjTypeID		mAssetTypeID;
	static const char* 			getClassName()
	{
		return NX_DESTRUCTIBLE_AUTHORING_TYPE_NAME;
	}
	ApexSimpleString			mName;

	NxDestructibleAsset* 		mNxAssetApi;
	ModuleDestructible* 		module;

	DestructibleAssetParameters*	mParams;
	bool							mOwnsParams;

	// Has a parameterized internal representation
	Array<ConvexHull>			chunkConvexHulls;

	// Runtime / derived
	PxI32						chunkOverlapCacheDepth;
	Array<NxResID>				mStaticMaterialIDs;

	ApexAssetTracker			mCrumbleAssetTracker;
	ApexAssetTracker			mDustAssetTracker;

	NxRenderMeshAsset*			renderMeshAsset;
	NxRenderMeshAsset*			runtimeRenderMeshAsset;

#if NX_SDK_VERSION_MAJOR != 2	// In 2.x this is put into the actor
	physx::Array<NxConvexMesh*>*	mCollisionMeshes;
#endif
	physx::PxU32				mRuntimeCookedConvexCount;

	Array<NxRenderMeshAsset*>	scatterMeshAssets;

	// Instanced chunks
	physx::PxU16								m_instancedChunkMeshCount;
	physx::Array<NxRenderMeshActor*>			m_instancedChunkRenderMeshActors;	// One per render mesh actor per instanced chunk
	physx::Array<physx::PxU16>					m_instancedChunkActorVisiblePart;
	physx::Array<physx::PxU16>					m_instancedChunkActorMap;	// from instanced chunk instanceInfo index to actor index
	physx::Array<NxUserRenderInstanceBuffer*>	m_chunkInstanceBuffers;
	physx::Array< physx::Array< ChunkInstanceBufferDataElement > >	m_chunkInstanceBufferData;

	// Scatter meshes
	physx::Array<ScatterMeshInstanceInfo>		m_scatterMeshInstanceInfo;	// One per scatter mesh asset

	physx::PxU32				m_currentInstanceBufferActorAllowance;
	bool						m_needsInstanceBufferDataResize;
	bool						m_needsInstanceBufferResize;
	physx::Mutex				m_chunkInstanceBufferDataLock;

	bool						m_needsScatterMeshInstanceInfoCreation;

	physx::PxI32				m_instancingRepresentativeActorIndex;	// -1 => means it's not set

	NxResourceList				m_destructibleList;
	NxResourceList				m_previewList;

	friend class DestructibleStructure;
	friend class DestructiblePreview;
	friend class DestructibleActor;
	friend class DestructibleActorJoint;
	friend class DestructibleScene;
	friend class ModuleDestructible;
	friend class DestructibleAssetCollision;
	friend class DestructibleModuleCachedData;
	friend class ApexDamageEventReportData;
	friend class DestructibleRenderable;
	
	NxParameterized::Interface* mApexDestructibleActorParams;

private:
	struct PlatformKeyValuePair
	{
		PlatformKeyValuePair(physx::apex::NxPlatformTag k, physx::PxU32 v): key(k), val(v) {}
		~PlatformKeyValuePair() {}
		physx::apex::NxPlatformTag key;
		physx::PxU32 val;
	private:
		PlatformKeyValuePair();
	};
	Array<PlatformKeyValuePair> m_platformFractureDepthMap;
	bool setDepthCount(physx::PxU32 targetDepthCount) const;
};

#ifndef WITHOUT_APEX_AUTHORING

void gatherPartMesh(physx::Array<physx::PxVec3>& vertices, physx::Array<physx::PxU32>& indices, const NxRenderMeshAsset* renderMeshAsset, physx::PxU32 partIndex);

class DestructibleAssetAuthoring : public DestructibleAsset
{
public:

	ExplicitHierarchicalMesh		hMesh;
	ExplicitHierarchicalMesh		hMeshCore;
	CutoutSet						cutoutSet;
	physx::IntersectMesh			intersectMesh;

	DestructibleAssetAuthoring(ModuleDestructible* module, NxDestructibleAsset* api, const char* name) :
		DestructibleAsset(module, api, name) {}

	DestructibleAssetAuthoring(ModuleDestructible* module, NxDestructibleAsset* api, NxParameterized::Interface* params, const char* name) :
		DestructibleAsset(module, api, params, name) {}

	void	setToolString(const char* toolString);

	void	cookChunks(const NxDestructibleAssetCookingDesc&, bool cacheOverlaps, PxU32* chunkIndexMapUser2Apex, PxU32* chunkIndexMapApex2User, PxU32 chunkIndexMapCount);

	void	trimCollisionGeometry(const physx::PxU32* partIndices, physx::PxU32 partIndexCount, physx::PxF32 maxTrimFraction = 0.2f);

	void	serializeFractureToolState(physx::PxFileBuf& stream, physx::NxExplicitHierarchicalMesh::NxEmbedding& embedding) const;
	void	deserializeFractureToolState(physx::PxFileBuf& stream, physx::NxExplicitHierarchicalMesh::NxEmbedding& embedding);

private:
	void	trimCollisionGeometryInternal(const physx::PxU32* chunkIndices, physx::PxU32 chunkIndexCount, const physx::Array<IntPair>& parentDepthOverlaps, physx::PxU32 depth, physx::PxF32 maxTrimFraction);
};
#endif

}
}
} // end namespace physx::apex

#endif // DESTRUCTIBLE_ASSET_H
