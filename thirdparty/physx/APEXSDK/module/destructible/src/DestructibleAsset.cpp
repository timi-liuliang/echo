/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "NxApexDefs.h"
#include "MinPhysxSdkVersion.h"
#if NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED

#include "NxApex.h"
#include "DestructibleAsset.h"
#include "DestructibleActorProxy.h"
#include "DestructiblePreviewProxy.h"
#include "ModuleDestructible.h"
#if NX_SDK_VERSION_MAJOR == 2
#include "NxCooking.h"
#include <NxScene.h>
#include <NxBoxShapeDesc.h>
#include <NxConvexMeshDesc.h>
#elif NX_SDK_VERSION_MAJOR == 3
#include "PxCooking.h"
#include "PxPhysics.h"
#include <PxScene.h>
#endif
#include "ModulePerfScope.h"
#include "PsShare.h"
#include "NiApexRenderMeshAsset.h"
#include "NiCof44.h"
#include "NxParamUtils.h"
#include "PxMemoryBuffer.h"
#if APEX_USE_PARTICLES
#include "NxApexEmitterAsset.h"
#else
static const char* NX_APEX_EMITTER_AUTHORING_TYPE_NAME = "ApexEmitterAsset";
#endif

#include "../../../framework/include/RenderMeshAssetParameters.h"
#include "../../../framework/include/ApexRenderMeshAsset.h"


#include "ApexSharedSerialization.h"
#include "ApexRand.h"

#include "ApexMerge.h"
#include "ApexFind.h"

namespace physx
{
namespace apex
{
namespace destructible
{

struct ChunkSortElement
{
	physx::PxI32 index;
	physx::PxI32 parentIndex;
	physx::PxI32 depth;
};

struct IndexSortedEdge
{
	IndexSortedEdge() {}
	IndexSortedEdge(physx::PxU32 _i0, physx::PxU32 _i1, physx::PxU32 _submeshIndex, const physx::PxVec3& _triangleNormal)
	{
		if (_i0 <= _i1)
		{
			i0 = _i0;
			i1 = _i1;
		}
		else
		{
			i0 = _i1;
			i1 = _i0;
		}
		submeshIndex = _submeshIndex;
		triangleNormal = _triangleNormal;
	}

	physx::PxU32 i0;
	physx::PxU32 i1;
	physx::PxU32 submeshIndex;
	physx::PxVec3 triangleNormal;
};

// We'll use this struct to store trim planes for each hull in each part
struct TrimPlane
{
	physx::PxU32 partIndex;
	physx::PxU32 hullIndex;
	physx::PxPlane plane;

	struct LessThan
	{
		PX_INLINE bool operator()(const TrimPlane& x, const TrimPlane& y) const
		{
			return x.partIndex != y.partIndex ? (x.partIndex < y.partIndex) : (x.hullIndex < y.hullIndex);
		}
	};
};

#ifndef WITHOUT_APEX_AUTHORING
static int compareChunkParents(
    const void* A,
    const void* B)
{
	ChunkSortElement& eA = *(ChunkSortElement*)A;
	ChunkSortElement& eB = *(ChunkSortElement*)B;

	const physx::PxI32 depthDiff = eA.depth - eB.depth;
	if (depthDiff)
	{
		return depthDiff;
	}

	const physx::PxI32 parentDiff = eA.parentIndex - eB.parentIndex;
	if (parentDiff)
	{
		return parentDiff;
	}

	return eA.index - eB.index;	// Keeps sort stable
}
#endif

void DestructibleAsset::setParameters(const NxDestructibleParameters& parameters)
{
	setParameters(parameters, mParams->destructibleParameters);

	NxParameterized::Handle handle(*mParams);
	mParams->getParameterHandle("depthParameters", handle);
	mParams->resizeArray(handle, (physx::PxI32)parameters.depthParametersCount);
	for (physx::PxU32 i = 0; i < parameters.depthParametersCount; ++i)
	{
		DestructibleAssetParametersNS::DestructibleDepthParameters_Type& d = mParams->depthParameters.buf[i];
		const NxDestructibleDepthParameters& dparm = parameters.depthParameters[i];
		d.OVERRIDE_IMPACT_DAMAGE		= (dparm.flags & NxDestructibleDepthParametersFlag::OVERRIDE_IMPACT_DAMAGE) ? true : false;
		d.OVERRIDE_IMPACT_DAMAGE_VALUE	= (dparm.flags & NxDestructibleDepthParametersFlag::OVERRIDE_IMPACT_DAMAGE_VALUE) ? true : false;
		d.IGNORE_POSE_UPDATES			= (dparm.flags & NxDestructibleDepthParametersFlag::IGNORE_POSE_UPDATES) ? true : false;
		d.IGNORE_RAYCAST_CALLBACKS		= (dparm.flags & NxDestructibleDepthParametersFlag::IGNORE_RAYCAST_CALLBACKS) ? true : false;
		d.IGNORE_CONTACT_CALLBACKS		= (dparm.flags & NxDestructibleDepthParametersFlag::IGNORE_CONTACT_CALLBACKS) ? true : false;
		d.USER_FLAG_0					= (dparm.flags & NxDestructibleDepthParametersFlag::USER_FLAG_0) ? true : false;
		d.USER_FLAG_1					= (dparm.flags & NxDestructibleDepthParametersFlag::USER_FLAG_1) ? true : false;
		d.USER_FLAG_2					= (dparm.flags & NxDestructibleDepthParametersFlag::USER_FLAG_2) ? true : false;
		d.USER_FLAG_3					= (dparm.flags & NxDestructibleDepthParametersFlag::USER_FLAG_3) ? true : false;
	}
}

void DestructibleAsset::setParameters(const NxDestructibleParameters& parameters, DestructibleAssetParametersNS::DestructibleParameters_Type& destructibleParameters)
{
	destructibleParameters.damageCap                             = parameters.damageCap;
	destructibleParameters.debrisDepth                           = parameters.debrisDepth;
	destructibleParameters.debrisLifetimeMax                     = parameters.debrisLifetimeMax;
	destructibleParameters.debrisLifetimeMin                     = parameters.debrisLifetimeMin;
	destructibleParameters.debrisMaxSeparationMax                = parameters.debrisMaxSeparationMax;
	destructibleParameters.debrisMaxSeparationMin                = parameters.debrisMaxSeparationMin;
	destructibleParameters.debrisDestructionProbability          = parameters.debrisDestructionProbability;
	destructibleParameters.dynamicChunkDominanceGroup            = parameters.dynamicChunksDominanceGroup;
	destructibleParameters.dynamicChunksGroupsMask.useGroupsMask = parameters.useDynamicChunksGroupsMask;
#if NX_SDK_VERSION_MAJOR == 2
	destructibleParameters.dynamicChunksGroupsMask.bits0         = parameters.dynamicChunksGroupsMask.bits0;
	destructibleParameters.dynamicChunksGroupsMask.bits1         = parameters.dynamicChunksGroupsMask.bits1;
	destructibleParameters.dynamicChunksGroupsMask.bits2         = parameters.dynamicChunksGroupsMask.bits2;
	destructibleParameters.dynamicChunksGroupsMask.bits3         = parameters.dynamicChunksGroupsMask.bits3;
#elif NX_SDK_VERSION_MAJOR == 3
	destructibleParameters.dynamicChunksGroupsMask.bits0         = parameters.dynamicChunksFilterData.word0;
	destructibleParameters.dynamicChunksGroupsMask.bits1         = parameters.dynamicChunksFilterData.word1;
	destructibleParameters.dynamicChunksGroupsMask.bits2         = parameters.dynamicChunksFilterData.word2;
	destructibleParameters.dynamicChunksGroupsMask.bits3         = parameters.dynamicChunksFilterData.word3;
	destructibleParameters.supportStrength						 = parameters.supportStrength;
#endif
	destructibleParameters.legacyChunkBoundsTestSetting			 = parameters.legacyChunkBoundsTestSetting;
	destructibleParameters.legacyDamageRadiusSpreadSetting		 = parameters.legacyDamageRadiusSpreadSetting;

	destructibleParameters.essentialDepth = parameters.essentialDepth;
	destructibleParameters.flags.ACCUMULATE_DAMAGE       = (parameters.flags & NxDestructibleParametersFlag::ACCUMULATE_DAMAGE) ? true : false;
	destructibleParameters.flags.DEBRIS_TIMEOUT          = (parameters.flags & NxDestructibleParametersFlag::DEBRIS_TIMEOUT) ? true : false;
	destructibleParameters.flags.DEBRIS_MAX_SEPARATION   = (parameters.flags & NxDestructibleParametersFlag::DEBRIS_MAX_SEPARATION) ? true : false;
	destructibleParameters.flags.CRUMBLE_SMALLEST_CHUNKS = (parameters.flags & NxDestructibleParametersFlag::CRUMBLE_SMALLEST_CHUNKS) ? true : false;
	destructibleParameters.flags.ACCURATE_RAYCASTS       = (parameters.flags & NxDestructibleParametersFlag::ACCURATE_RAYCASTS) ? true : false;
	destructibleParameters.flags.USE_VALID_BOUNDS        = (parameters.flags & NxDestructibleParametersFlag::USE_VALID_BOUNDS) ? true : false;
	destructibleParameters.flags.CRUMBLE_VIA_RUNTIME_FRACTURE = (parameters.flags & NxDestructibleParametersFlag::CRUMBLE_VIA_RUNTIME_FRACTURE) ? true : false;
	destructibleParameters.forceToDamage                 = parameters.forceToDamage;
	destructibleParameters.fractureImpulseScale          = parameters.fractureImpulseScale;
	destructibleParameters.damageDepthLimit              = parameters.damageDepthLimit;
	destructibleParameters.impactVelocityThreshold       = parameters.impactVelocityThreshold;
	destructibleParameters.maxChunkSpeed                 = parameters.maxChunkSpeed;
	destructibleParameters.minimumFractureDepth          = parameters.minimumFractureDepth;
	destructibleParameters.impactDamageDefaultDepth      = parameters.impactDamageDefaultDepth;
	destructibleParameters.debrisDestructionProbability  = parameters.debrisDestructionProbability;
	destructibleParameters.validBounds                   = parameters.validBounds;

	// RT Fracture Parameters
	destructibleParameters.runtimeFracture.sheetFracture			= parameters.rtFractureParameters.sheetFracture;
	destructibleParameters.runtimeFracture.depthLimit				= parameters.rtFractureParameters.depthLimit;
	destructibleParameters.runtimeFracture.destroyIfAtDepthLimit	= parameters.rtFractureParameters.destroyIfAtDepthLimit;
	destructibleParameters.runtimeFracture.minConvexSize			= parameters.rtFractureParameters.minConvexSize;
	destructibleParameters.runtimeFracture.impulseScale				= parameters.rtFractureParameters.impulseScale;
	destructibleParameters.runtimeFracture.glass.numSectors			= parameters.rtFractureParameters.glass.numSectors;
	destructibleParameters.runtimeFracture.glass.sectorRand			= parameters.rtFractureParameters.glass.sectorRand;
	destructibleParameters.runtimeFracture.glass.firstSegmentSize	= parameters.rtFractureParameters.glass.firstSegmentSize;
	destructibleParameters.runtimeFracture.glass.segmentScale		= parameters.rtFractureParameters.glass.segmentScale;
	destructibleParameters.runtimeFracture.glass.segmentRand		= parameters.rtFractureParameters.glass.segmentRand;
	destructibleParameters.runtimeFracture.attachment.posX			= parameters.rtFractureParameters.attachment.posX;
	destructibleParameters.runtimeFracture.attachment.negX			= parameters.rtFractureParameters.attachment.negX;
	destructibleParameters.runtimeFracture.attachment.posY			= parameters.rtFractureParameters.attachment.posY;
	destructibleParameters.runtimeFracture.attachment.negY			= parameters.rtFractureParameters.attachment.negY;
	destructibleParameters.runtimeFracture.attachment.posZ			= parameters.rtFractureParameters.attachment.posZ;
	destructibleParameters.runtimeFracture.attachment.negZ			= parameters.rtFractureParameters.attachment.negZ;
}

void DestructibleAsset::setInitParameters(const NxDestructibleInitParameters& parameters)
{
	mParams->supportDepth = parameters.supportDepth;
	mParams->formExtendedStructures      = (parameters.flags & NxDestructibleInitParametersFlag::FORM_EXTENDED_STRUCTURES) ? true : false;
	mParams->useAssetDefinedSupport		 = (parameters.flags & NxDestructibleInitParametersFlag::ASSET_DEFINED_SUPPORT) ? true : false;
	mParams->useWorldSupport			 = (parameters.flags & NxDestructibleInitParametersFlag::WORLD_SUPPORT) ? true : false;
}

NxDestructibleParameters DestructibleAsset::getParameters() const
{
	return getParameters(mParams->destructibleParameters, &mParams->depthParameters);
}

NxDestructibleParameters DestructibleAsset::getParameters(const DestructibleAssetParametersNS::DestructibleParameters_Type& destructibleParameters,
														  const DestructibleAssetParametersNS::DestructibleDepthParameters_DynamicArray1D_Type* destructibleDepthParameters)
{
	NxDestructibleParameters parameters;

	parameters.damageCap                  = destructibleParameters.damageCap;
	parameters.debrisDepth                = destructibleParameters.debrisDepth;
	parameters.debrisLifetimeMax          = destructibleParameters.debrisLifetimeMax;
	parameters.debrisLifetimeMin          = destructibleParameters.debrisLifetimeMin;
	parameters.debrisMaxSeparationMax     = destructibleParameters.debrisMaxSeparationMax;
	parameters.debrisMaxSeparationMin     = destructibleParameters.debrisMaxSeparationMin;
	parameters.dynamicChunksDominanceGroup   = (physx::PxU8)destructibleParameters.dynamicChunkDominanceGroup;
	parameters.useDynamicChunksGroupsMask = destructibleParameters.dynamicChunksGroupsMask.useGroupsMask;
#if NX_SDK_VERSION_MAJOR == 2
	parameters.dynamicChunksGroupsMask.bits0 = destructibleParameters.dynamicChunksGroupsMask.bits0;
	parameters.dynamicChunksGroupsMask.bits1 = destructibleParameters.dynamicChunksGroupsMask.bits1;
	parameters.dynamicChunksGroupsMask.bits2 = destructibleParameters.dynamicChunksGroupsMask.bits2;
	parameters.dynamicChunksGroupsMask.bits3 = destructibleParameters.dynamicChunksGroupsMask.bits3;
#elif NX_SDK_VERSION_MAJOR == 3
	parameters.dynamicChunksFilterData.word0 = destructibleParameters.dynamicChunksGroupsMask.bits0;
	parameters.dynamicChunksFilterData.word1 = destructibleParameters.dynamicChunksGroupsMask.bits1;
	parameters.dynamicChunksFilterData.word2 = destructibleParameters.dynamicChunksGroupsMask.bits2;
	parameters.dynamicChunksFilterData.word3 = destructibleParameters.dynamicChunksGroupsMask.bits3;
#endif
	parameters.essentialDepth = destructibleParameters.essentialDepth;
	parameters.flags = 0;
	if (destructibleParameters.flags.ACCUMULATE_DAMAGE)
	{
		parameters.flags |= NxDestructibleParametersFlag::ACCUMULATE_DAMAGE;
	}
	if (destructibleParameters.flags.DEBRIS_TIMEOUT)
	{
		parameters.flags |= NxDestructibleParametersFlag::DEBRIS_TIMEOUT;
	}
	if (destructibleParameters.flags.DEBRIS_MAX_SEPARATION)
	{
		parameters.flags |= NxDestructibleParametersFlag::DEBRIS_MAX_SEPARATION;
	}
	if (destructibleParameters.flags.CRUMBLE_SMALLEST_CHUNKS)
	{
		parameters.flags |= NxDestructibleParametersFlag::CRUMBLE_SMALLEST_CHUNKS;
	}
	if (destructibleParameters.flags.ACCURATE_RAYCASTS)
	{
		parameters.flags |= NxDestructibleParametersFlag::ACCURATE_RAYCASTS;
	}
	if (destructibleParameters.flags.USE_VALID_BOUNDS)
	{
		parameters.flags |= NxDestructibleParametersFlag::USE_VALID_BOUNDS;
	}
	if (destructibleParameters.flags.CRUMBLE_VIA_RUNTIME_FRACTURE)
	{
		parameters.flags |= NxDestructibleParametersFlag::CRUMBLE_VIA_RUNTIME_FRACTURE;
	}
	parameters.forceToDamage            = destructibleParameters.forceToDamage;
	parameters.fractureImpulseScale     = destructibleParameters.fractureImpulseScale;
	parameters.damageDepthLimit         = destructibleParameters.damageDepthLimit;
	parameters.impactVelocityThreshold  = destructibleParameters.impactVelocityThreshold;
	parameters.maxChunkSpeed            = destructibleParameters.maxChunkSpeed;
	parameters.minimumFractureDepth     = destructibleParameters.minimumFractureDepth;
	parameters.impactDamageDefaultDepth = destructibleParameters.impactDamageDefaultDepth;
	parameters.debrisDestructionProbability = destructibleParameters.debrisDestructionProbability;
	parameters.validBounds              = destructibleParameters.validBounds;

	if (destructibleDepthParameters)
	{
		//NxParameterized::Handle handle(*mParams);
		//mParams->getParameterHandle("depthParameters", handle);
		parameters.depthParametersCount = physx::PxMin((physx::PxU32)NxDestructibleParameters::kDepthParametersCountMax,
														(physx::PxU32)destructibleDepthParameters->arraySizes[0]);
	for (int i = 0; i < (int)parameters.depthParametersCount; ++i)
	{
			DestructibleAssetParametersNS::DestructibleDepthParameters_Type& d = destructibleDepthParameters->buf[i];
		NxDestructibleDepthParameters& dparm = parameters.depthParameters[i];
		dparm.flags = 0;
		if (d.OVERRIDE_IMPACT_DAMAGE)
		{
			dparm.flags |= NxDestructibleDepthParametersFlag::OVERRIDE_IMPACT_DAMAGE;
		}
		if (d.OVERRIDE_IMPACT_DAMAGE_VALUE)
		{
			dparm.flags |= NxDestructibleDepthParametersFlag::OVERRIDE_IMPACT_DAMAGE_VALUE;
		}
		if (d.IGNORE_POSE_UPDATES)
		{
			dparm.flags |= NxDestructibleDepthParametersFlag::IGNORE_POSE_UPDATES;
		}
		if (d.IGNORE_RAYCAST_CALLBACKS)
		{
			dparm.flags |= NxDestructibleDepthParametersFlag::IGNORE_RAYCAST_CALLBACKS;
		}
		if (d.IGNORE_CONTACT_CALLBACKS)
		{
			dparm.flags |= NxDestructibleDepthParametersFlag::IGNORE_CONTACT_CALLBACKS;
		}
		if (d.USER_FLAG_0)
		{
			dparm.flags |= NxDestructibleDepthParametersFlag::USER_FLAG_0;
		}
		if (d.USER_FLAG_1)
		{
			dparm.flags |= NxDestructibleDepthParametersFlag::USER_FLAG_1;
		}
		if (d.USER_FLAG_2)
		{
			dparm.flags |= NxDestructibleDepthParametersFlag::USER_FLAG_2;
		}
		if (d.USER_FLAG_3)
		{
			dparm.flags |= NxDestructibleDepthParametersFlag::USER_FLAG_3;
		}
	}
	}

	// RT Fracture Parameters
	parameters.rtFractureParameters.sheetFracture			= destructibleParameters.runtimeFracture.sheetFracture;
	parameters.rtFractureParameters.depthLimit				= destructibleParameters.runtimeFracture.depthLimit;
	parameters.rtFractureParameters.destroyIfAtDepthLimit	= destructibleParameters.runtimeFracture.destroyIfAtDepthLimit;
	parameters.rtFractureParameters.minConvexSize			= destructibleParameters.runtimeFracture.minConvexSize;
	parameters.rtFractureParameters.impulseScale			= destructibleParameters.runtimeFracture.impulseScale;
	parameters.rtFractureParameters.glass.numSectors		= destructibleParameters.runtimeFracture.glass.numSectors;
	parameters.rtFractureParameters.glass.sectorRand		= destructibleParameters.runtimeFracture.glass.sectorRand;
	parameters.rtFractureParameters.glass.firstSegmentSize	= destructibleParameters.runtimeFracture.glass.firstSegmentSize;
	parameters.rtFractureParameters.glass.segmentScale		= destructibleParameters.runtimeFracture.glass.segmentScale;
	parameters.rtFractureParameters.glass.segmentRand		= destructibleParameters.runtimeFracture.glass.segmentRand;
	parameters.rtFractureParameters.attachment.posX			= destructibleParameters.runtimeFracture.attachment.posX;
	parameters.rtFractureParameters.attachment.negX			= destructibleParameters.runtimeFracture.attachment.negX;
	parameters.rtFractureParameters.attachment.posY			= destructibleParameters.runtimeFracture.attachment.posY;
	parameters.rtFractureParameters.attachment.negY			= destructibleParameters.runtimeFracture.attachment.negY;
	parameters.rtFractureParameters.attachment.posZ			= destructibleParameters.runtimeFracture.attachment.posZ;
	parameters.rtFractureParameters.attachment.negZ			= destructibleParameters.runtimeFracture.attachment.negZ;
#if NX_SDK_VERSION_MAJOR == 3
	parameters.supportStrength								= destructibleParameters.supportStrength;
#endif 

	parameters.legacyChunkBoundsTestSetting					 = destructibleParameters.legacyChunkBoundsTestSetting;
	parameters.legacyDamageRadiusSpreadSetting				 = destructibleParameters.legacyDamageRadiusSpreadSetting;

	return parameters;
}

NxDestructibleInitParameters DestructibleAsset::getInitParameters() const
{
	NxDestructibleInitParameters parameters;

	parameters.supportDepth = mParams->supportDepth;
	parameters.flags = 0;
	if (mParams->formExtendedStructures)
	{
		parameters.flags |= NxDestructibleInitParametersFlag::FORM_EXTENDED_STRUCTURES;
	}
	if (mParams->useAssetDefinedSupport)
	{
		parameters.flags |= NxDestructibleInitParametersFlag::ASSET_DEFINED_SUPPORT;
	}
	if (mParams->useWorldSupport)
	{
		parameters.flags |= NxDestructibleInitParametersFlag::WORLD_SUPPORT;
	}

	return parameters;
}

void DestructibleAsset::setCrumbleEmitterName(const char* name)
{
	NxParameterized::Handle handle(*mParams);
	mParams->getParameterHandle("crumbleEmitterName", handle);
	mParams->setParamString(handle, name ? name : "");
}

const char* DestructibleAsset::getCrumbleEmitterName() const
{
	const char* name = mParams->crumbleEmitterName;
	return (name && *name) ? name : NULL;
}

void DestructibleAsset::setDustEmitterName(const char* name)
{
	NxParameterized::Handle handle(*mParams);
	mParams->getParameterHandle("dustEmitterName", handle);
	mParams->setParamString(handle, name ? name : "");
}

const char* DestructibleAsset::getDustEmitterName() const
{
	const char* name = mParams->dustEmitterName;
	return (name && *name) ? name : NULL;
}

void DestructibleAsset::setFracturePatternName(const char* name)
{
	NxParameterized::Handle handle(*mParams);
	mParams->getParameterHandle("fracturePatternName", handle);
	mParams->setParamString(handle, name ? name : "");
}

const char* DestructibleAsset::getFracturePatternName() const
{
	// TODO: Add to asset params
	const char* name = "";//mParams->fracturePatternName;
	return (name && *name) ? name : NULL;
}

void DestructibleAsset::setChunkOverlapsCacheDepth(physx::PxI32 depth)
{
	chunkOverlapCacheDepth = depth;
}

void DestructibleAsset::calculateChunkDepthStarts()
{
	const physx::PxU32 chunkCount = (physx::PxU32)mParams->chunks.arraySizes[0];

	NxParameterized::Handle handle(*mParams);
	mParams->getParameterHandle("firstChunkAtDepth", handle);
	mParams->resizeArray(handle, (physx::PxI32)mParams->depthCount + 1);
	mParams->firstChunkAtDepth.buf[mParams->depthCount] = chunkCount;

	physx::PxU32 stopIndex = 0;
	for (physx::PxU32 depth = 0; depth < mParams->depthCount; ++depth)
	{
		mParams->firstChunkAtDepth.buf[depth] = stopIndex;
		while (stopIndex < chunkCount)
		{
			if (mParams->chunks.buf[stopIndex].depth != depth)
			{
				break;
			}
			++stopIndex;
		}
	}
}

CachedOverlapsNS::IntPair_DynamicArray1D_Type* DestructibleAsset::getOverlapsAtDepth(physx::PxU32 depth, bool create) const
{
	if (depth >= mParams->depthCount)
	{
		return NULL;
	}

	int size = mParams->overlapsAtDepth.arraySizes[0];
	if (size <= (int)depth && !create)
	{
		return NULL;
	}

	NxParameterized::Handle handle(*mParams);
	mParams->getParameterHandle("overlapsAtDepth", handle);

	if (create)
	{
		mParams->resizeArray(handle, (physx::PxI32)mParams->depthCount);
		NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();
		while (size < (int)mParams->depthCount)
		{
			CachedOverlaps* cachedOverlaps = DYNAMIC_CAST(CachedOverlaps*)(traits->createNxParameterized(CachedOverlaps::staticClassName()));
			mParams->overlapsAtDepth.buf[size++] = cachedOverlaps;
			cachedOverlaps->isCached = false;
		}
	}

	CachedOverlaps* cachedOverlapsAtDepth = DYNAMIC_CAST(CachedOverlaps*)(mParams->overlapsAtDepth.buf[depth]);
	if (!cachedOverlapsAtDepth->isCached)
	{
		if (!create)
		{
			return NULL;
		}
		physx::Array<IntPair> overlaps;
		calculateChunkOverlaps(overlaps, depth);
		NxParameterized::Handle overlapsHandle(*cachedOverlapsAtDepth);
		cachedOverlapsAtDepth->getParameterHandle("overlaps", overlapsHandle);
		overlapsHandle.resizeArray(2*(physx::PxI32)overlaps.size());
		for (physx::PxU32 i = 0; i < overlaps.size(); ++i)
		{
			IntPair& pair = overlaps[i];
			
			CachedOverlapsNS::IntPair_Type& ppair = cachedOverlapsAtDepth->overlaps.buf[2*i];
			ppair.i0 = pair.i0;
			ppair.i1 = pair.i1;

			CachedOverlapsNS::IntPair_Type& ppairSymmetric = cachedOverlapsAtDepth->overlaps.buf[2*i+1];
			ppairSymmetric.i0 = pair.i1;
			ppairSymmetric.i1 = pair.i0;
		}
		qsort(cachedOverlapsAtDepth->overlaps.buf, (physx::PxU32)cachedOverlapsAtDepth->overlaps.arraySizes[0], sizeof(IntPair), IntPair::compare);

		cachedOverlapsAtDepth->isCached = 1;
	}

	return &cachedOverlapsAtDepth->overlaps;
}

void DestructibleAsset::calculateChunkOverlaps(physx::Array<IntPair>& overlaps, physx::PxU32 depth) const
{
	const physx::PxF32 padding = mParams->neighborPadding * (mParams->bounds.maximum - mParams->bounds.minimum).magnitude();

	const physx::PxMat44 identityTM(physx::PxVec4(1.0f));
	const physx::PxVec3 identityScale(1.0f);

	const physx::PxU32 startIndex = mParams->firstChunkAtDepth.buf[depth];
	const physx::PxU32 stopIndex = mParams->firstChunkAtDepth.buf[depth + 1];
	const physx::PxU32 chunksAtDepth = stopIndex - startIndex;

	// Find AABB overlaps
	physx::Array<BoundsRep> chunkBoundsReps;
	chunkBoundsReps.reserve(chunksAtDepth);
	for (physx::PxU32 chunkIndex = startIndex; chunkIndex < stopIndex; ++chunkIndex)
	{
		BoundsRep& chunkBoundsRep = chunkBoundsReps.insert();
		chunkBoundsRep.aabb = getChunkActorLocalBounds(chunkIndex);
		PX_ASSERT(!chunkBoundsRep.aabb.isEmpty());
		chunkBoundsRep.aabb.fattenFast(padding);
	}
	if (chunkBoundsReps.size() > 0)
	{
		boundsCalculateOverlaps(overlaps, Bounds3XYZ, &chunkBoundsReps[0], chunkBoundsReps.size(), sizeof(chunkBoundsReps[0]));
	}

	// Now do detailed overlap test
	physx::PxU32 overlapCount = 0;
	for (physx::PxU32 overlapIndex = 0; overlapIndex < overlaps.size(); ++overlapIndex)
	{
		IntPair& AABBOverlap = overlaps[overlapIndex];
		AABBOverlap.i0 += startIndex;
		AABBOverlap.i1 += startIndex;
		if (chunksInProximity(*this, (physx::PxU16)AABBOverlap.i0, identityTM, identityScale, *this, (physx::PxU16)AABBOverlap.i1, identityTM, identityScale, 2 * padding))
		{
			overlaps[overlapCount++] = AABBOverlap;
		}
	}

	overlaps.resize(overlapCount);
}

void DestructibleAsset::cacheChunkOverlapsUpToDepth(physx::PxI32 depth)
{
	if (mParams->depthCount < 1)
	{
		return;
	}

	if (depth < 0)
	{
		depth = (physx::PxI32)mParams->supportDepth;
	}

	depth = physx::PxMin(depth, (physx::PxI32)mParams->depthCount - 1);

	for (physx::PxU32 d = 0; d <= (physx::PxU32)depth; ++d)
	{
		getOverlapsAtDepth(d);
	}

	for (physx::PxU32 d = (physx::PxU32)depth + 1; d < (physx::PxU32)mParams->overlapsAtDepth.arraySizes[0]; ++d)
	{
		CachedOverlaps* cachedOverlaps = DYNAMIC_CAST(CachedOverlaps*)(mParams->overlapsAtDepth.buf[d]);
		NxParameterized::Handle handle(*cachedOverlaps);
		cachedOverlaps->getParameterHandle("overlaps", handle);
		cachedOverlaps->resizeArray(handle, 0);
	}
}


void DestructibleAsset::clearChunkOverlaps(physx::PxI32 depth, bool keepCachedFlag)
{
	PxI32 depthStart = (depth < 0) ? 0 : depth;
	PxI32 depthEnd = (depth < 0) ? mParams->overlapsAtDepth.arraySizes[0] : PxMin(depth+1, mParams->overlapsAtDepth.arraySizes[0]);
	for (physx::PxI32 d = depthStart; d < depthEnd; ++d)
	{
		CachedOverlaps* cachedOverlaps = DYNAMIC_CAST(CachedOverlaps*)(mParams->overlapsAtDepth.buf[d]);
		NxParameterized::Handle handle(*cachedOverlaps);
		cachedOverlaps->getParameterHandle("overlaps", handle);
		cachedOverlaps->resizeArray(handle, 0);
		if (!keepCachedFlag)
		{
			cachedOverlaps->isCached = false;
		}
	}
}


void DestructibleAsset::addChunkOverlaps(NxIntPair* supportGraphEdges, PxU32 supportGraphEdgeCount)
{
	if (supportGraphEdgeCount == 0)
		return;

	PxU32 numChunks = (physx::PxU32)mParams->chunks.arraySizes[0];

	Array< Array<IntPair> > overlapsAtDepth(mParams->depthCount);

	// store symmetric pairs at corresponding depth
	for (PxU32 i = 0; i < supportGraphEdgeCount; ++i)
	{
		PxU32 chunkIndex0 = (physx::PxU32)supportGraphEdges[i].mI0;
		PxU32 chunkIndex1 = (physx::PxU32)supportGraphEdges[i].mI1;

		if (chunkIndex0 >= numChunks || chunkIndex1 >= numChunks)
		{
			APEX_DEBUG_WARNING("Edge %i supportGraphEdges has indices (%i,%i), but only a total of %i chunks are provided. supportEdges will be ignored.", i, chunkIndex0, chunkIndex1, numChunks);
			overlapsAtDepth.clear();
			break;
		}

		const DestructibleAssetParametersNS::Chunk_Type& chunk0 = mParams->chunks.buf[chunkIndex0];
		const DestructibleAssetParametersNS::Chunk_Type& chunk1 = mParams->chunks.buf[chunkIndex1];

		if (chunk0.depth != chunk1.depth)
		{
			APEX_DEBUG_WARNING("Support graph can only have edges between sibling chunks. supportEdges will be ignored.");
			overlapsAtDepth.clear();
			break;
		}

		PX_ASSERT(chunk0.depth < mParams->depthCount);

		IntPair pair;
		pair.i0 = (physx::PxI32)chunkIndex0;
		pair.i1 = (physx::PxI32)chunkIndex1;
		overlapsAtDepth[chunk0.depth].pushBack(pair);

		pair.i0 = (physx::PxI32)chunkIndex1;
		pair.i1 = (physx::PxI32)chunkIndex0;
		overlapsAtDepth[chunk0.depth].pushBack(pair);
	}

	if (overlapsAtDepth.size() == 0)
		return;


	// for each depth
	for (PxU32 depth = 0; depth < overlapsAtDepth.size(); ++depth)
	{
		if (overlapsAtDepth[depth].size() > 0)
		{
			// sort overlaps pairs
			qsort(&overlapsAtDepth[depth][0], overlapsAtDepth[depth].size(), sizeof(IntPair), IntPair::compare);
		}

		if (overlapsAtDepth[depth].size() == 0)
			continue;

		// resize parameterized array
		PxU32 numCachedDepths = (PxU32)mParams->overlapsAtDepth.arraySizes[0];
		if (depth >= numCachedDepths)
		{
			NxParameterized::Handle handle(*mParams);
			mParams->getParameterHandle("overlapsAtDepth", handle);
			mParams->resizeArray(handle, (physx::PxI32)depth+1);

			NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();
			for (PxU32 d = numCachedDepths; d < depth+1; ++d)
			{
				CachedOverlaps* cachedOverlaps = DYNAMIC_CAST(CachedOverlaps*)(traits->createNxParameterized(CachedOverlaps::staticClassName()));
				mParams->overlapsAtDepth.buf[d] = cachedOverlaps;
				cachedOverlaps->isCached = false;
			}
		}

		CachedOverlaps* cachedOverlapsAtDepth = DYNAMIC_CAST(CachedOverlaps*)(mParams->overlapsAtDepth.buf[depth]);
		NxParameterized::Handle overlapsHandle(*cachedOverlapsAtDepth);
		cachedOverlapsAtDepth->getParameterHandle("overlaps", overlapsHandle);
		PxI32 oldSize = cachedOverlapsAtDepth->overlaps.arraySizes[0];
		overlapsHandle.resizeArray(cachedOverlapsAtDepth->overlaps.arraySizes[0] + (physx::PxI32)overlapsAtDepth[depth].size());

		// merge new pairs into existing graph
		bool ok = ApexMerge<IntPair>(	(IntPair*)cachedOverlapsAtDepth->overlaps.buf, (physx::PxU32)oldSize,
										&overlapsAtDepth[depth][0], overlapsAtDepth[depth].size(),
										(IntPair*)cachedOverlapsAtDepth->overlaps.buf, (physx::PxU32)cachedOverlapsAtDepth->overlaps.arraySizes[0],
										IntPair::compare);

		PX_UNUSED(ok);
		PX_ASSERT(ok);

		// check for duplicates
		if (cachedOverlapsAtDepth->overlaps.arraySizes[0] > 1)
		{
			Array<PxU32> toRemove;
			for (PxU32 j = 1; j < (physx::PxU32)cachedOverlapsAtDepth->overlaps.arraySizes[0]; ++j)
			{
				if (cachedOverlapsAtDepth->overlaps.buf[j].i1 == cachedOverlapsAtDepth->overlaps.buf[j-1].i1 &&
					cachedOverlapsAtDepth->overlaps.buf[j].i0 == cachedOverlapsAtDepth->overlaps.buf[j-1].i0)
				{
					toRemove.pushBack(j);
				}
			}

			// remove duplicates
			toRemove.pushBack((physx::PxU32)cachedOverlapsAtDepth->overlaps.arraySizes[0]); // add guard
			PxU32 shift = 0;
			for (PxU32 j = 0; j < toRemove.size()-1; ++j)
			{
				++shift;
				for (PxU32 index = toRemove[j]+1; index < toRemove[j+1]; ++index)
				{
					cachedOverlapsAtDepth->overlaps.buf[index - shift] = cachedOverlapsAtDepth->overlaps.buf[index];
				}
			}
			NxParameterized::Handle overlapsHandle(*cachedOverlapsAtDepth);
			cachedOverlapsAtDepth->getParameterHandle("overlaps", overlapsHandle);
			overlapsHandle.resizeArray(cachedOverlapsAtDepth->overlaps.arraySizes[0] - (physx::PxI32)shift);
		}

		cachedOverlapsAtDepth->isCached = cachedOverlapsAtDepth->overlaps.arraySizes[0] > 0;
	}
}


void DestructibleAsset::removeChunkOverlaps(NxIntPair* supportGraphEdges, PxU32 numSupportGraphEdges, bool keepCachedFlagIfEmpty)
{
	Array< Array<PxU32> > toRemoveAtDepth(mParams->depthCount);
	for (PxU32 i = 0; i < numSupportGraphEdges; ++i)
	{
		CachedOverlapsNS::IntPair_Type& pair = (CachedOverlapsNS::IntPair_Type&)supportGraphEdges[i];
		
		if (pair.i0 >= mParams->chunks.arraySizes[0] || pair.i1 >= mParams->chunks.arraySizes[0])
			continue;

		const DestructibleAssetParametersNS::Chunk_Type& chunk0 = mParams->chunks.buf[pair.i0];
		const DestructibleAssetParametersNS::Chunk_Type& chunk1 = mParams->chunks.buf[pair.i1];

		if (chunk0.depth != chunk1.depth)
			continue;

		if (chunk0.depth >= mParams->overlapsAtDepth.arraySizes[0])
			continue;

		if (chunk0.depth >= mParams->depthCount)
			continue;

		CachedOverlaps* cachedOverlapsAtDepth = DYNAMIC_CAST(CachedOverlaps*)(mParams->overlapsAtDepth.buf[chunk0.depth]);

		// binary search for pair and add add to index to removal list
		PxI32 index = ApexFind(cachedOverlapsAtDepth->overlaps.buf, (physx::PxU32)cachedOverlapsAtDepth->overlaps.arraySizes[0], pair, IntPair::compare);
		if (index != -1)
		{
			toRemoveAtDepth[chunk0.depth].pushBack((physx::PxU32)index);
		}

		CachedOverlapsNS::IntPair_Type symmetricPair;
		symmetricPair.i0 = pair.i1;
		symmetricPair.i1 = pair.i0;
		index = ApexFind(cachedOverlapsAtDepth->overlaps.buf, (physx::PxU32)cachedOverlapsAtDepth->overlaps.arraySizes[0], symmetricPair, IntPair::compare);
		if (index != -1)
		{
			toRemoveAtDepth[chunk0.depth].pushBack((physx::PxU32)index);
		}
	}

	// go through removal list of each depth and shift remaining entries to overwrite the removed ones
	for (PxU32 depth = 0; depth < toRemoveAtDepth.size(); ++depth)
	{
		CachedOverlaps* cachedOverlapsAtDepth = DYNAMIC_CAST(CachedOverlaps*)(mParams->overlapsAtDepth.buf[depth]);
		toRemoveAtDepth[depth].pushBack((physx::PxU32)cachedOverlapsAtDepth->overlaps.arraySizes[0]); // add guard
		PxU32 shift = 0;
		for (PxU32 j = 1; j < toRemoveAtDepth[depth].size(); ++j)
		{
			++shift;
			for (PxU32 index = toRemoveAtDepth[depth][j-1]+1; index < toRemoveAtDepth[depth][j]; ++index)
			{
				cachedOverlapsAtDepth->overlaps.buf[index - shift] = cachedOverlapsAtDepth->overlaps.buf[index];
			}
		}

		NxParameterized::Handle overlapsHandle(*cachedOverlapsAtDepth);
		cachedOverlapsAtDepth->getParameterHandle("overlaps", overlapsHandle);
		overlapsHandle.resizeArray(cachedOverlapsAtDepth->overlaps.arraySizes[0] - (physx::PxI32)shift);

		if (!keepCachedFlagIfEmpty)
		{
			cachedOverlapsAtDepth->isCached = cachedOverlapsAtDepth->overlaps.arraySizes[0] > 0;
		}
	}
}


DestructibleAsset::DestructibleAsset(ModuleDestructible* inModule, NxDestructibleAsset* api, const char* name) :
	mCrumbleAssetTracker(inModule->mSdk, NX_APEX_EMITTER_AUTHORING_TYPE_NAME),
	mDustAssetTracker(inModule->mSdk, NX_APEX_EMITTER_AUTHORING_TYPE_NAME),
	m_instancedChunkMeshCount(0)
{
	mApexDestructibleActorParams = 0;
	init();

	module = inModule;
	mNxAssetApi  = api;
	mName = name;

	NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();
	mParams = DYNAMIC_CAST(DestructibleAssetParameters*)(traits->createNxParameterized(DestructibleAssetParameters::staticClassName()));
	mOwnsParams = mParams != NULL;
	PX_ASSERT(mOwnsParams);
}

DestructibleAsset::DestructibleAsset(ModuleDestructible* inModule, NxDestructibleAsset* api, NxParameterized::Interface* params, const char* name) 
	: mCrumbleAssetTracker(inModule->mSdk, NX_APEX_EMITTER_AUTHORING_TYPE_NAME)
	, mDustAssetTracker(inModule->mSdk, NX_APEX_EMITTER_AUTHORING_TYPE_NAME)
	, mRuntimeCookedConvexCount(0)
	, m_instancedChunkMeshCount(0)
{
	mApexDestructibleActorParams = 0;
	init();

	module = inModule;
	mNxAssetApi  = api;
	mName = name;

	mParams = DYNAMIC_CAST(DestructibleAssetParameters*)(params);

	// The pattern for NxParameterized assets is that the params pointer now belongs to the asset
	mOwnsParams = true;

	// there's no deserialize, so init the ARMs
	if (mParams->renderMeshAsset)
	{
		ApexSimpleString meshName = mName + ApexSimpleString("RenderMesh");
		module->mSdk->getInternalResourceProvider()->generateUniqueName(module->mSdk->getApexMeshNameSpace(), meshName);

		setRenderMeshAsset(static_cast<NxRenderMeshAsset*>(module->mSdk->createAsset(mParams->renderMeshAsset,  meshName.c_str())));
	}

	// scatter meshes
	bool scatterMeshAssetsValid = true;
	physx::Array<NxRenderMeshAsset*> scatterMeshAssetArray((physx::PxU32)mParams->scatterMeshAssets.arraySizes[0]);
	for (physx::PxU32 i = 0; i < (physx::PxU32)mParams->scatterMeshAssets.arraySizes[0]; ++i)
	{
		if (i > 65535 || mParams->scatterMeshAssets.buf[i] == NULL)
		{
			scatterMeshAssetsValid = false;
			break;
		}
		char suffix[20];
		sprintf(suffix, "ScatterMesh%d", i);
		ApexSimpleString meshName = mName + ApexSimpleString(suffix);
		module->mSdk->getInternalResourceProvider()->generateUniqueName(module->mSdk->getApexMeshNameSpace(), meshName);
		scatterMeshAssetArray[i] = static_cast<NxRenderMeshAsset*>(module->mSdk->createAsset(mParams->scatterMeshAssets.buf[i], meshName.c_str()));
	}

	bool success = false;
	if (scatterMeshAssetsValid && mParams->scatterMeshAssets.arraySizes[0] > 0)
	{
		success = setScatterMeshAssets(&scatterMeshAssetArray[0], (physx::PxU32)mParams->scatterMeshAssets.arraySizes[0]);
	}
	if (!success)
	{
		for (physx::PxU32 i = 0; i < scatterMeshAssetArray.size(); ++i)
		{
			if (scatterMeshAssetArray[i] != NULL)
			{
				scatterMeshAssetArray[i]->release();
			}
		}
	}

	bool hullWarningGiven = false;

	// Connect contained classes to referenced parameters
	chunkConvexHulls.resize((physx::PxU32)mParams->chunkConvexHulls.arraySizes[0]);
	for (physx::PxU32 i = 0; i < chunkConvexHulls.size(); ++i)
	{
		chunkConvexHulls[i].init(mParams->chunkConvexHulls.buf[i]);

		// Fix convex hulls to account for adjacentFaces bug
		if (chunkConvexHulls[i].mParams->adjacentFaces.arraySizes[0] != chunkConvexHulls[i].mParams->edges.arraySizes[0])
		{
			chunkConvexHulls[i].buildFromPoints(chunkConvexHulls[i].mParams->vertices.buf, (physx::PxU32)chunkConvexHulls[i].mParams->vertices.arraySizes[0], 
				(physx::PxU32)chunkConvexHulls[i].mParams->vertices.elementSize);
			if (!hullWarningGiven)
			{
				physx::NiGetApexSDK()->reportError(physx::PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, __FUNCTION__,
					"Chunk convex hull data bad in asset %s, rebuilding.  Asset should be re-exported.", name);
				hullWarningGiven = true;
			}
		}
	}

	m_currentInstanceBufferActorAllowance = mParams->initialDestructibleActorAllowanceForInstancing;

	physx::Array<physx::PxU16> tempPartToActorMap;
	tempPartToActorMap.resize(renderMeshAsset->getPartCount(), 0xFFFF);

	m_instancedChunkMeshCount = 0;

	m_instancedChunkActorMap.resize((physx::PxU32)mParams->chunkInstanceInfo.arraySizes[0]);
	for (physx::PxU32 i = 0; i < (physx::PxU32)mParams->chunkInstanceInfo.arraySizes[0]; ++i)
	{
		physx::PxU16 partIndex = mParams->chunkInstanceInfo.buf[i].partIndex;
		if (tempPartToActorMap[partIndex] == 0xFFFF)
		{
			tempPartToActorMap[partIndex] = m_instancedChunkMeshCount++;
		}
		m_instancedChunkActorMap[i] = tempPartToActorMap[partIndex];
	}

	m_instancedChunkActorVisiblePart.resize(m_instancedChunkMeshCount);
	for (physx::PxU32 i = 0; i < (physx::PxU32)mParams->chunks.arraySizes[0]; ++i)
	{
		DestructibleAssetParametersNS::Chunk_Type& chunk = mParams->chunks.buf[i];
		if ((chunk.flags & DestructibleAsset::Instanced) != 0)
		{
			physx::PxU16 partIndex = mParams->chunkInstanceInfo.buf[chunk.meshPartIndex].partIndex;
			m_instancedChunkActorVisiblePart[m_instancedChunkActorMap[chunk.meshPartIndex]] = partIndex;
		}
	}

	m_instancingRepresentativeActorIndex = -1;	// not set

	reduceAccordingToLOD();

	initializeAssetNameTable();

	mStaticMaterialIDs.resize((physx::PxU32)mParams->staticMaterialNames.arraySizes[0]);
	NiResourceProvider* resourceProvider = NiGetApexSDK()->getInternalResourceProvider();
	NxResID materialNS = NiGetApexSDK()->getMaterialNameSpace();
	// Resolve material names using the NRP...
	for (physx::PxU32 i = 0; i < (physx::PxU32)mParams->staticMaterialNames.arraySizes[0]; ++i)
	{
		if (resourceProvider)
		{
			mStaticMaterialIDs[i] = resourceProvider->createResource(materialNS, mParams->staticMaterialNames.buf[i]);
		}
		else
		{
			mStaticMaterialIDs[i] = INVALID_RESOURCE_ID;
		}
	}
}

DestructibleAsset::~DestructibleAsset()
{
	// Release named resources
	NiResourceProvider* resourceProvider = NiGetApexSDK()->getInternalResourceProvider();
	for (physx::PxU32 i = 0 ; i < mStaticMaterialIDs.size() ; i++)
	{
		resourceProvider->releaseResource(mStaticMaterialIDs[i]);
	}

	if (mParams != NULL && mOwnsParams)
	{
		mParams->destroy();
	}
	mParams = NULL;
	mOwnsParams = false;

	if (mApexDestructibleActorParams)
	{
		mApexDestructibleActorParams->destroy();
		mApexDestructibleActorParams = 0;
	}
	/* Assets that were forceloaded or loaded by actors will be automatically
	 * released by the ApexAssetTracker member destructors.
	 */
}

void DestructibleAsset::init()
{
	module = NULL;
	chunkOverlapCacheDepth = -1;
	renderMeshAsset = NULL;
	runtimeRenderMeshAsset = NULL;
#if NX_SDK_VERSION_MAJOR != 2	// In 2.x this is put into the actor
	mCollisionMeshes = NULL;
#endif
	m_currentInstanceBufferActorAllowance = 0;
	m_needsInstanceBufferDataResize = false;
	m_needsInstanceBufferResize = false;
	m_needsScatterMeshInstanceInfoCreation = false;
}

physx::PxU32 DestructibleAsset::forceLoadAssets()
{
	physx::PxU32 assetLoadedCount = 0;

	assetLoadedCount += mCrumbleAssetTracker.forceLoadAssets();
	assetLoadedCount += mDustAssetTracker.forceLoadAssets();

	if (renderMeshAsset != NULL)
	{
		assetLoadedCount += renderMeshAsset->forceLoadAssets();
	}

	NiResourceProvider* nrp = NiGetApexSDK()->getInternalResourceProvider();
	NxResID materialNS = NiGetApexSDK()->getMaterialNameSpace();
	for (physx::PxU32 i = 0; i < mStaticMaterialIDs.size(); i++)
	{
		if (!nrp->checkResource(materialNS, mParams->staticMaterialNames.buf[i]))
		{
			/* we know for SURE that createResource() has already been called, so just getResource() */
			nrp->getResource(mStaticMaterialIDs[i]);
			assetLoadedCount++;
		}
	}

	return assetLoadedCount;
}

void DestructibleAsset::initializeAssetNameTable()
{
	if (mParams->dustEmitterName && *mParams->dustEmitterName)
	{
		mDustAssetTracker.addAssetName(mParams->dustEmitterName, false);
	}

	if (mParams->crumbleEmitterName && *mParams->crumbleEmitterName)
	{
		mCrumbleAssetTracker.addAssetName(mParams->crumbleEmitterName, false);
	}
}

void DestructibleAsset::cleanup()
{
	// Release internal NxRenderMesh, preview instances, and authoring instance

	while (m_previewList.getSize())
	{
		DestructiblePreviewProxy* proxy = DYNAMIC_CAST(DestructiblePreviewProxy*)(m_previewList.getResource(m_previewList.getSize() - 1));
		PX_ASSERT(proxy != NULL);
		if (proxy == NULL)
		{
			m_previewList.remove(m_previewList.getSize() - 1);	// To avoid an infinite loop
		}
		else
		{
			proxy->release();
		}
	}

	m_previewList.clear();
	m_destructibleList.clear();

	setRenderMeshAsset(NULL);

	// release chunk instance render resources
	m_chunkInstanceBufferDataLock.lock();
	m_needsInstanceBufferResize = false;
	m_chunkInstanceBufferData.clear();
	updateChunkInstanceRenderResources(false, NULL);
	m_chunkInstanceBufferDataLock.unlock();

	setScatterMeshAssets(NULL, 0);

	m_instancedChunkActorMap.resize(0);
	m_instancedChunkActorVisiblePart.resize(0);

	if (module->mCachedData != NULL)
	{
		module->mCachedData->clearAssetCollisionSet(*this);
	}
}

void DestructibleAsset::prepareForNewInstance()
{
	if (m_currentInstanceBufferActorAllowance < m_destructibleList.getSize() + 1)	// Add 1 to predict new actor
	{
		// This loop should only be hit once
		do
		{
			m_currentInstanceBufferActorAllowance = m_currentInstanceBufferActorAllowance > 0 ? 2*m_currentInstanceBufferActorAllowance : 1;
			m_needsInstanceBufferDataResize = true;
		}
		while (m_currentInstanceBufferActorAllowance < m_destructibleList.getSize());	// Add 1 to predict new actor
	}
}



void DestructibleAsset::resetInstanceData()
{
	PX_PROFILER_PERF_SCOPE("DestructibleAsset::resetInstanceData");

	m_chunkInstanceBufferDataLock.lock();
	m_chunkInstanceBufferData.resize(m_instancedChunkMeshCount);
	if (m_needsInstanceBufferDataResize)
	{
		//
		// reserve the right amount of memory in the per chunk mesh arrays
		// 
		for (physx::PxU32 index = 0; index < m_instancedChunkMeshCount; ++index)
		{
			if (m_currentInstanceBufferActorAllowance > 0)
			{
				// Find out how many potential instances there are
				physx::PxU32 maxInstanceCount = 0;
				for (physx::PxI32 i = 0; i < mParams->chunkInstanceInfo.arraySizes[0]; ++i)
				{
					if (mParams->chunkInstanceInfo.buf[i].partIndex == m_instancedChunkActorVisiblePart[index])
					{
						maxInstanceCount += m_currentInstanceBufferActorAllowance;
					}
				}

				// Instance buffer data
				m_chunkInstanceBufferData[index].reserve(maxInstanceCount);
			}
			
			m_chunkInstanceBufferData[index].resize(0);
		}

		m_needsInstanceBufferDataResize = false;
		m_needsInstanceBufferResize = true;
	}
	else
	{
		for (physx::PxU32 j = 0; j < m_chunkInstanceBufferData.size(); ++j)
		{
			m_chunkInstanceBufferData[j].resize(0);
		}
	}
	m_chunkInstanceBufferDataLock.unlock();


	for (physx::PxU32 j = 0; j < m_scatterMeshInstanceInfo.size(); ++j)
	{
		m_scatterMeshInstanceInfo[j].m_instanceBufferData.resize(0);
	}
	m_instancingRepresentativeActorIndex = -1;	// not set
}


template <class ParamType>
NxDestructibleActor* createDestructibleActorImpl(ParamType& params, 
												 DestructibleAsset& destructibleAsset,
												 NxResourceList& destructibleList,
												 DestructibleScene* destructibleScene)
{
	if (NULL == destructibleScene)
		return NULL;

	destructibleAsset.prepareForNewInstance();

	return PX_NEW(DestructibleActorProxy)(params, destructibleAsset, destructibleList, *destructibleScene);
}

NxDestructibleActor* DestructibleAsset::createDestructibleActorFromDeserializedState(NxParameterized::Interface* params, NxApexScene& scene)
{
	PX_PROFILER_PERF_SCOPE("DestructibleCreateActor");

	if (NULL == params || !isValidForActorCreation(*params, scene))
		return NULL;

	return createDestructibleActorImpl(params, *this, m_destructibleList, module->getDestructibleScene(scene));
}

NxDestructibleActor* DestructibleAsset::createDestructibleActor(const NxParameterized::Interface& params, NxApexScene& scene)
{
	PX_PROFILER_PERF_SCOPE("DestructibleCreateActor");

	return createDestructibleActorImpl(params, *this, m_destructibleList, module->getDestructibleScene(scene));
}

void DestructibleAsset::releaseDestructibleActor(NxDestructibleActor& nxactor)
{
	DestructibleActorProxy* proxy = DYNAMIC_CAST(DestructibleActorProxy*)(&nxactor);
	proxy->destroy();
}

bool DestructibleAsset::setRenderMeshAsset(NxRenderMeshAsset* newRenderMeshAsset)
{
	if (newRenderMeshAsset == renderMeshAsset)
	{
		return false;
	}

	for (physx::PxU32 i = 0; i < m_instancedChunkRenderMeshActors.size(); ++i)
	{
		if (m_instancedChunkRenderMeshActors[i] != NULL)
		{
			m_instancedChunkRenderMeshActors[i]->release();
			m_instancedChunkRenderMeshActors[i] = NULL;
		}
	}

	if (renderMeshAsset != NULL)
	{
		if(mOwnsParams && mParams != NULL)
		{
			// set isReferenced to false, so that the parameterized object
			// for the render mesh asset is destroyed in renderMeshAsset->release
			NxParameterized::ErrorType e;
			if (mParams->renderMeshAsset != NULL)
			{
				NxParameterized::Handle h(*mParams->renderMeshAsset);
				e = mParams->renderMeshAsset->getParameterHandle("isReferenced", h);
				PX_ASSERT(e == NxParameterized::ERROR_NONE);
				if (e == NxParameterized::ERROR_NONE)
				{
					h.setParamBool(false);
				}
				mParams->renderMeshAsset = NULL;
			}
		}
		renderMeshAsset->release();
	}

	renderMeshAsset = newRenderMeshAsset;
	if (renderMeshAsset != NULL)
	{
		mParams->renderMeshAsset = (NxParameterized::Interface*)renderMeshAsset->getAssetNxParameterized();
		NxParameterized::ErrorType e;
		NxParameterized::Handle h(*mParams->renderMeshAsset);
		e = mParams->renderMeshAsset->getParameterHandle("isReferenced", h);
		PX_ASSERT(e == NxParameterized::ERROR_NONE);
		if (e == NxParameterized::ERROR_NONE)
		{
			h.setParamBool(true);
		}

		for (physx::PxU32 i = 0; i < m_instancedChunkRenderMeshActors.size(); ++i)
		{
			// Create actor
			NxRenderMeshActorDesc renderableMeshDesc;
			renderableMeshDesc.maxInstanceCount = m_chunkInstanceBufferData[i].capacity();
			renderableMeshDesc.renderWithoutSkinning = true;
			renderableMeshDesc.visible = false;
			m_instancedChunkRenderMeshActors[i] = newRenderMeshAsset->createActor(renderableMeshDesc);
			m_instancedChunkRenderMeshActors[i]->setInstanceBuffer(m_chunkInstanceBuffers[i]);
			m_instancedChunkRenderMeshActors[i]->setVisibility(true, m_instancedChunkActorVisiblePart[i]);
			m_instancedChunkRenderMeshActors[i]->setReleaseResourcesIfNothingToRender(false);
		}
	}

	return true;
}

bool DestructibleAsset::setScatterMeshAssets(NxRenderMeshAsset** scatterMeshAssetArray, physx::PxU32 scatterMeshAssetArraySize)
{
	if (scatterMeshAssetArray == NULL && scatterMeshAssetArraySize > 0)
	{
		return false;
	}

	for (physx::PxU32 i = 0; i < scatterMeshAssetArraySize; ++i)
	{
		if (scatterMeshAssetArray[i] == NULL)
		{
			return false;
		}
	}

	// First clear instance information
	m_scatterMeshInstanceInfo.resize(0);	// Ensure we delete all instanced actors
	m_scatterMeshInstanceInfo.resize(scatterMeshAssetArraySize);

	// Clear out scatter mesh assets, including parameterized data
	for (physx::PxI32 i = 0; mParams && (i < mParams->scatterMeshAssets.arraySizes[0]); ++i)
	{
		if (mParams->scatterMeshAssets.buf[i] != NULL)
		{
			NxParameterized::ErrorType e;
			NxParameterized::Handle h(*mParams->scatterMeshAssets.buf[i]);
			e = mParams->scatterMeshAssets.buf[i]->getParameterHandle("isReferenced", h);
			PX_ASSERT(e == NxParameterized::ERROR_NONE);
			if (e == NxParameterized::ERROR_NONE)
			{
				h.setParamBool(false);
			}
			mParams->scatterMeshAssets.buf[i] = NULL;
		}
	}

	for (physx::PxU32 i = 0; i < scatterMeshAssets.size(); ++i)
	{
		if (scatterMeshAssets[i] != NULL)
		{
			scatterMeshAssets[i]->release();
			scatterMeshAssets[i] = NULL;
		}
	}

	if (mParams != NULL)
	{
	scatterMeshAssets.resize(scatterMeshAssetArraySize, NULL);
	NxParameterized::Handle h(*mParams, "scatterMeshAssets");
	h.resizeArray((physx::PxI32)scatterMeshAssetArraySize);

	for (physx::PxU32 i = 0; i < scatterMeshAssetArraySize; ++i)
	{
		// Create new asset
		scatterMeshAssets[i] = scatterMeshAssetArray[i];
		mParams->scatterMeshAssets.buf[i] = (NxParameterized::Interface*)scatterMeshAssets[i]->getAssetNxParameterized();
		NxParameterized::ErrorType e;
		NxParameterized::Handle h(*mParams->scatterMeshAssets.buf[i]);
		e = mParams->scatterMeshAssets.buf[i]->getParameterHandle("isReferenced", h);
		PX_ASSERT(e == NxParameterized::ERROR_NONE);
		if (e == NxParameterized::ERROR_NONE)
		{
			h.setParamBool(true);
		}
	}

	m_needsScatterMeshInstanceInfoCreation = true;
	}


	return true;
}

void DestructibleAsset::createScatterMeshInstanceInfo()
{
	if (!m_needsScatterMeshInstanceInfoCreation)
		return;

	m_needsScatterMeshInstanceInfoCreation = false;

	NxUserRenderResourceManager* rrm = NiGetApexSDK()->getUserRenderResourceManager();

	physx::Array<physx::PxU32> totalInstanceCounts(scatterMeshAssets.size(), 0);
	for (physx::PxI32 i = 0; i < mParams->scatterMeshIndices.arraySizes[0]; ++i)
	{
		const physx::PxU8 scatterMeshIndex = mParams->scatterMeshIndices.buf[i];
		if (scatterMeshIndex < scatterMeshAssets.size())
		{
			++totalInstanceCounts[scatterMeshIndex];
		}
	}

	for (physx::PxU32 i = 0; i < scatterMeshAssets.size(); ++i)
	{
		// Create instanced info
		ScatterMeshInstanceInfo& info = m_scatterMeshInstanceInfo[i];
		NxRenderMeshActorDesc renderableMeshDesc;
		renderableMeshDesc.maxInstanceCount = totalInstanceCounts[i];
		renderableMeshDesc.renderWithoutSkinning = true;
		renderableMeshDesc.visible = true;
		info.m_actor = scatterMeshAssets[i]->createActor(renderableMeshDesc);

		// Create instance buffer
		info.m_instanceBuffer = NULL;
		info.m_IBSize = totalInstanceCounts[i];
		if (totalInstanceCounts[i] > 0)
		{
			NxUserRenderInstanceBufferDesc instanceBufferDesc = getScatterMeshInstanceBufferDesc();
			instanceBufferDesc.maxInstances = totalInstanceCounts[i];
			info.m_instanceBuffer = rrm->createInstanceBuffer(instanceBufferDesc);
		}

		// Instance buffer data
		info.m_instanceBufferData.reset();
		info.m_instanceBufferData.reserve(totalInstanceCounts[i]);

		info.m_actor->setInstanceBuffer(info.m_instanceBuffer);
		info.m_actor->setReleaseResourcesIfNothingToRender(false);
	}
}

NxUserRenderInstanceBufferDesc DestructibleAsset::getScatterMeshInstanceBufferDesc()
{
	NxUserRenderInstanceBufferDesc instanceBufferDesc;
	instanceBufferDesc.hint = NxRenderBufferHint::DYNAMIC;
	instanceBufferDesc.semanticOffsets[NxRenderInstanceLayoutElement::POSITION_FLOAT3] = ScatterInstanceBufferDataElement::translationOffset();
	instanceBufferDesc.semanticOffsets[NxRenderInstanceLayoutElement::ROTATION_SCALE_FLOAT3x3] = ScatterInstanceBufferDataElement::scaledRotationOffset();
	instanceBufferDesc.semanticOffsets[NxRenderInstanceLayoutElement::DENSITY_FLOAT1] = ScatterInstanceBufferDataElement::alphaOffset();
	instanceBufferDesc.stride = sizeof(ScatterInstanceBufferDataElement);

	return instanceBufferDesc;
}

void DestructibleAsset::updateChunkInstanceRenderResources(bool rewriteBuffers, void* userRenderData)
{
	PX_PROFILER_PERF_SCOPE("DestructibleAsset::updateChunkInstanceRenderResources");

	NxUserRenderResourceManager* rrm = NiGetApexSDK()->getUserRenderResourceManager();

	Mutex::ScopedLock scopeLock(m_chunkInstanceBufferDataLock);

	PxU32 oldCount = m_chunkInstanceBuffers.size();
	PxU32 count = m_chunkInstanceBufferData.size();

	//
	// release resources
	//
	// release all on resize for recreation lateron
	PxU32 startIndexForRelease = m_needsInstanceBufferResize ? 0 : count;
	for (physx::PxU32 i = startIndexForRelease; i < oldCount; ++i)
	{
		if (m_instancedChunkRenderMeshActors[i] != NULL)
		{
			m_instancedChunkRenderMeshActors[i]->release();
			m_instancedChunkRenderMeshActors[i] = NULL;
		}
		if (m_chunkInstanceBuffers[i] != NULL)
		{
			rrm->releaseInstanceBuffer(*m_chunkInstanceBuffers[i]);
			m_chunkInstanceBuffers[i] = NULL;
		}
	}

	// resize and init arrays
	m_chunkInstanceBuffers.resize(count);
	m_instancedChunkRenderMeshActors.resize(count);
	for (physx::PxU32 index = oldCount; index < count; ++index)
	{
		m_instancedChunkRenderMeshActors[index] = NULL;
		m_chunkInstanceBuffers[index] = NULL;
	}


	//
	// create resources when needed
	//

	for (physx::PxU32 index = 0; index < count; ++index)
	{
		// if m_chunkInstanceBufferData[index] contains any data there's an instance to render
		if (m_chunkInstanceBuffers[index] == NULL && m_chunkInstanceBufferData[index].size() > 0)
		{
			// Find out how many potential instances there are
			physx::PxU32 maxInstanceCount = 0;
			for (physx::PxI32 i = 0; i < mParams->chunkInstanceInfo.arraySizes[0]; ++i)
			{
				if (mParams->chunkInstanceInfo.buf[i].partIndex == m_instancedChunkActorVisiblePart[index])
				{
					maxInstanceCount += m_currentInstanceBufferActorAllowance;
				}
			}

			// Create instance buffer
			NxUserRenderInstanceBufferDesc instanceBufferDesc;
			instanceBufferDesc.maxInstances = maxInstanceCount;
			instanceBufferDesc.hint = NxRenderBufferHint::DYNAMIC;
			instanceBufferDesc.semanticOffsets[NxRenderInstanceLayoutElement::POSITION_FLOAT3] = ChunkInstanceBufferDataElement::translationOffset();
			instanceBufferDesc.semanticOffsets[NxRenderInstanceLayoutElement::ROTATION_SCALE_FLOAT3x3] = ChunkInstanceBufferDataElement::scaledRotationOffset();
			instanceBufferDesc.semanticOffsets[NxRenderInstanceLayoutElement::UV_OFFSET_FLOAT2] = ChunkInstanceBufferDataElement::uvOffsetOffset();
			instanceBufferDesc.semanticOffsets[NxRenderInstanceLayoutElement::LOCAL_OFFSET_FLOAT3] = ChunkInstanceBufferDataElement::localOffsetOffset();
			instanceBufferDesc.stride = sizeof(ChunkInstanceBufferDataElement);
			m_chunkInstanceBuffers[index] = rrm->createInstanceBuffer(instanceBufferDesc);

			// Create actor
			if (renderMeshAsset != NULL)
			{
				PX_ASSERT(m_instancedChunkRenderMeshActors[index] == NULL);

				NxRenderMeshActorDesc renderableMeshDesc;
				renderableMeshDesc.maxInstanceCount = maxInstanceCount;
				renderableMeshDesc.renderWithoutSkinning = true;
				renderableMeshDesc.visible = false;
				m_instancedChunkRenderMeshActors[index] = renderMeshAsset->createActor(renderableMeshDesc);
				m_instancedChunkRenderMeshActors[index]->setInstanceBuffer(m_chunkInstanceBuffers[index]);
				m_instancedChunkRenderMeshActors[index]->setVisibility(true, m_instancedChunkActorVisiblePart[index]);
				m_instancedChunkRenderMeshActors[index]->setReleaseResourcesIfNothingToRender(false);
			}
		}

		//
		// update with new data
		//
		PX_ASSERT(index < m_chunkInstanceBufferData.size());
		NiApexRenderMeshActor* renderMeshActor = (NiApexRenderMeshActor*)m_instancedChunkRenderMeshActors[index];
		if (renderMeshActor != NULL)
		{
			NxApexRenderInstanceBufferData data;
			const physx::PxU32 instanceBufferSize = m_chunkInstanceBufferData[index].size();

			if (instanceBufferSize > 0)
			{
				m_chunkInstanceBuffers[index]->writeBuffer(&m_chunkInstanceBufferData[index][0], 0, instanceBufferSize);
			}

			renderMeshActor->setInstanceBufferRange(0, instanceBufferSize);
			renderMeshActor->updateRenderResources(false, rewriteBuffers, userRenderData);
		}
	}

	m_needsInstanceBufferResize = false;
}

bool DestructibleAsset::setPlatformMaxDepth(NxPlatformTag platform, physx::PxU32 maxDepth)
{
	bool isExistingPlatform = false;
	for (Array<PlatformKeyValuePair>::Iterator iter = m_platformFractureDepthMap.begin(); iter != m_platformFractureDepthMap.end(); ++iter)
	{
		if (strcmp(iter->key, platform) == 0)
		{
			isExistingPlatform = true;
			iter->val = maxDepth; //overwrite if existing
			break;
		}
	}
	if (!isExistingPlatform)
	{
		m_platformFractureDepthMap.pushBack(PlatformKeyValuePair(platform, maxDepth));
	}
	return maxDepth < mParams->depthCount - 1; //depthCount == 1 => unfractured mesh
}

bool DestructibleAsset::removePlatformMaxDepth(NxPlatformTag platform)
{
	bool isExistingPlatform = false;
	for (physx::PxU32 index = 0; index < m_platformFractureDepthMap.size(); ++index)
	{
		if (strcmp(m_platformFractureDepthMap[index].key, platform) == 0)
		{
			isExistingPlatform = true;
			m_platformFractureDepthMap.remove(index); //yikes! for lack of a map...
			break;
		}
	}
	return isExistingPlatform;
}

bool DestructibleAsset::setDepthCount(physx::PxU32 targetDepthCount) const
{
	if (mParams->depthCount <= targetDepthCount)
	{
		return false;
	}

	int newChunkCount = mParams->chunks.arraySizes[0];
	for (int i = newChunkCount; i--;)
	{
		if (mParams->chunks.buf[i].depth >= targetDepthCount)
		{
			newChunkCount = i;
		}
		else if (mParams->chunks.buf[i].depth == targetDepthCount - 1)
		{
			// These chunks must have no children
			DestructibleAssetParametersNS::Chunk_Type& chunk = mParams->chunks.buf[i];
			chunk.numChildren = 0;
			chunk.firstChildIndex = DestructibleAsset::InvalidChunkIndex;
			chunk.flags &= ~(physx::PxU16)DescendantUnfractureable;
		}
		else
		{
			break;
		}
	}

	NxParameterized::Handle handle(*mParams);
	mParams->getParameterHandle("chunks", handle);
	mParams->resizeArray(handle, newChunkCount);

	mParams->getParameterHandle("overlapsAtDepth", handle);
	int size;
	mParams->getArraySize(handle, size);
	if ((int)targetDepthCount < size)
	{
		mParams->resizeArray(handle, (physx::PxI32)targetDepthCount);
	}
	mParams->getParameterHandle("firstChunkAtDepth", handle);
	mParams->resizeArray(handle, (physx::PxI32)targetDepthCount + 1);

	mParams->depthCount = targetDepthCount;

	return true;
}

bool DestructibleAsset::prepareForPlatform(physx::apex::NxPlatformTag platform) const
{
	bool isExistingPlatform = false;
	bool isDepthLimitChanged = false;
	for (Array<PlatformKeyValuePair>::ConstIterator kIter = m_platformFractureDepthMap.begin(); kIter != m_platformFractureDepthMap.end(); ++kIter)
	{
		if (strcmp(kIter->key, platform) == 0)
		{
			isExistingPlatform = true;
			isDepthLimitChanged = setDepthCount(kIter->val + 1); //targetDepthCount == 1 => unfractured mesh
			break;
		}
	}
	//if (!isExistingPlatform) {/*keep all depths, behaviour by default*/}
	return (isExistingPlatform & isDepthLimitChanged);
}

void DestructibleAsset::reduceAccordingToLOD()
{
	if (module == NULL)
	{
		return;
	}

	const physx::PxU32 targetDepthCount = mParams->originalDepthCount > module->m_maxChunkDepthOffset ? mParams->originalDepthCount - module->m_maxChunkDepthOffset : 1;

	setDepthCount(targetDepthCount);
}

void DestructibleAsset::getStats(NxDestructibleAssetStats& stats) const
{
	memset(&stats, 0, sizeof(NxDestructibleAssetStats));

	if (renderMeshAsset)
	{
		renderMeshAsset->getStats(stats.renderMeshAssetStats);
	}

	// BRG - to do - need a way of getting the serialized size
	stats.totalBytes = 0;

	stats.chunkCount = (physx::PxU32)mParams->chunks.arraySizes[0];
	stats.chunkBytes = mParams->chunks.arraySizes[0] * sizeof(DestructibleAssetParametersNS::Chunk_Type);

	physx::PxU32 maxEdgeCount = 0;

	for (physx::PxU16 chunkIndex = 0; chunkIndex < getChunkCount(); ++chunkIndex)
	{
		for (physx::PxU32 hullIndex = getChunkHullIndexStart(chunkIndex); hullIndex < getChunkHullIndexStop(chunkIndex); ++hullIndex)
		{
			const ConvexHull& hullData = chunkConvexHulls[hullIndex];
			const physx::PxU32 hullDataBytes = hullData.getVertexCount() * sizeof(physx::PxVec3) +
											   hullData.getUniquePlaneNormalCount() * 4 * sizeof(physx::PxF32) +
											   hullData.getUniquePlaneNormalCount() * sizeof(physx::PxF32) +
											   hullData.getEdgeCount() * sizeof(physx::PxU32);
			stats.chunkHullDataBytes += hullDataBytes;
			stats.chunkBytes += hullDataBytes;

			// Get cooked convex mesh stats
			physx::PxU32 numVerts = 0;
			physx::PxU32 numPolys = 0;
			const physx::PxU32 cookedHullDataBytes = hullData.calculateCookedSizes(numVerts, numPolys, true);
			stats.maxHullVertexCount = physx::PxMax(stats.maxHullVertexCount, numVerts);
			stats.maxHullFaceCount = physx::PxMax(stats.maxHullFaceCount, numPolys);
			const physx::PxU32 numEdges = numVerts + numPolys - 2;
			if (numEdges > maxEdgeCount)
			{
				maxEdgeCount = numEdges;
				stats.chunkWithMaxEdgeCount = chunkIndex;
			}

			stats.chunkBytes += cookedHullDataBytes;
		}
	}

	stats.runtimeCookedConvexCount = mRuntimeCookedConvexCount;
	/* To do - count collision data in ApexScene! */
}

void DestructibleAsset::applyTransformation(const physx::PxMat44& transformation, physx::PxF32 scale)
{
	/* For now, we'll just clear the current cached streams at scale. */
	/* transform and scale the NxConvexMesh(es) */
	if (mParams->collisionData)
	{
		APEX_DEBUG_WARNING("Cached collision data is already present, removing");
		mParams->collisionData->destroy();
		mParams->collisionData = NULL;
#if 0
		DestructibleAssetCollisionDataSet* cds =
		    DYNAMIC_CAST(DestructibleAssetCollisionDataSet*)(mParams->collisionData);

		for (int i = 0; i < cds->meshCookedCollisionStreamsAtScale.arraySizes[0]; ++i)
		{
			PX_ASSERT(cds->meshCookedCollisionStreamsAtScale.buf[i]);

			MeshCookedCollisionStreamsAtScale* meshStreamsAtScale =
			    DYNAMIC_CAST(MeshCookedCollisionStreamsAtScale*)(cds->meshCookedCollisionStreamsAtScale.buf[i]);
			for (int j = 0; j < meshStreamsAtScale->meshCookedCollisionStreams.arraySizes[0]; ++j)
			{
				PX_ASSERT(meshStreamsAtScale->meshCookedCollisionStreams.buf[i]);

				MeshCookedCollisionStream* meshStreamParams =
				    DYNAMIC_CAST(MeshCookedCollisionStream*)(meshStreamsAtScale->meshCookedCollisionStreams.buf[i]);

				/* stream it into the physx sdk */
				PxMemoryBuffer memStream(meshStreamParams->bytes.buf, meshStreamParams->bytes.arraySizes[0]);
				memStream.setEndianMode(physx::PxFileBuf::ENDIAN_NONE);
				NxFromPxStream nxs(memStream);
				NxConvexMesh* convexMesh = NxGetApexSDK()->getPhysXSDK()->createConvexMesh(nxs);

				NxConvexMeshDesc meshDesc;
				convexMesh->saveToDesc(meshDesc);
				meshDesc.

				physx::PxU32 submeshCount = convexMesh->getSubmeshCount();
				(void)submeshCount;
			}
		}
#endif
	}

	/* chunk surface normal */
	for (int i = 0; i < mParams->chunks.arraySizes[0]; ++i)
	{
		PX_ASSERT(mParams->chunks.buf);

		DestructibleAssetParametersNS::Chunk_Type* chunk = &(mParams->chunks.buf[i]);

		chunk->surfaceNormal = transformation.rotate(chunk->surfaceNormal);
	}

	/* bounds */
	PX_ASSERT(!mParams->bounds.isEmpty());
	mParams->bounds.minimum *= scale;
	mParams->bounds.maximum *= scale;
	if (scale < 0.0f)
	{
		physx::swap(mParams->bounds.minimum, mParams->bounds.maximum);
	}
	mParams->bounds = physx::transform(transformation, mParams->bounds);

	/* chunk convex hulls */
	for (int i = 0; i < mParams->chunkConvexHulls.arraySizes[0]; ++i)
	{
		PX_ASSERT(mParams->chunkConvexHulls.buf[i]);
		ConvexHull chunkHull;
		chunkHull.mParams = DYNAMIC_CAST(ConvexHullParameters*)(mParams->chunkConvexHulls.buf[i]);
		chunkHull.mOwnsParams = false;
		chunkHull.applyTransformation(transformation, scale);
	}

	/* render mesh (just apply to both the asset and author if they both exist) */
	if (renderMeshAsset)
	{
		reinterpret_cast<NiApexRenderMeshAsset*>(renderMeshAsset)->applyTransformation(transformation, scale);
	}

	/* scatter meshes */
	const physx::PxMat33 m33(transformation.column0.getXYZ(), transformation.column1.getXYZ(), transformation.column2.getXYZ());
	for (int i = 0; i < mParams->scatterMeshTransforms.arraySizes[0]; ++i)
	{
		DestructibleAssetParametersNS::M34_Type& tm = mParams->scatterMeshTransforms.buf[i];
		tm.matrix = m33*tm.matrix;
		tm.matrix *= scale;
		tm.vector = transformation.getPosition() + scale*(m33*tm.vector);
	}
	if (m33.getDeterminant() * scale < 0.0f)
	{
		for (physx::PxU32 i = 0; i < scatterMeshAssets.size(); ++i)
		{
			NxRenderMeshAsset* scatterMesh = scatterMeshAssets[i];
			if (scatterMesh != NULL)
			{
				reinterpret_cast<NiApexRenderMeshAsset*>(scatterMesh)->reverseWinding();
			}
		}
	}
}

void DestructibleAsset::applyTransformation(const physx::PxMat44& transformation)
{
	/* For now, we'll just clear the current cached streams at scale. */
	/* transform and scale the NxConvexMesh(es) */
	if (mParams->collisionData)
	{
		APEX_DEBUG_WARNING("Cached collision data is already present, removing");
		mParams->collisionData->destroy();
		mParams->collisionData = NULL;
	}

	physx::NiCof44 cofTM(transformation);

	/* chunk surface normal */
	for (int i = 0; i < mParams->chunks.arraySizes[0]; ++i)
	{
		PX_ASSERT(mParams->chunks.buf);

		DestructibleAssetParametersNS::Chunk_Type* chunk = &(mParams->chunks.buf[i]);

		chunk->surfaceNormal = cofTM.getBlock33().transform(chunk->surfaceNormal);
		chunk->surfaceNormal.normalize();
	}

	/* bounds */
	PX_ASSERT(!mParams->bounds.isEmpty());
	mParams->bounds = physx::transform(transformation, mParams->bounds);

	/* chunk convex hulls */
	for (int i = 0; i < mParams->chunkConvexHulls.arraySizes[0]; ++i)
	{
		PX_ASSERT(mParams->chunkConvexHulls.buf[i]);
		ConvexHull chunkHull;
		chunkHull.mParams = DYNAMIC_CAST(ConvexHullParameters*)(mParams->chunkConvexHulls.buf[i]);
		chunkHull.mOwnsParams = false;
		chunkHull.applyTransformation(transformation);
	}

	/* render mesh (just apply to both the asset and author if they both exist) */
	if (renderMeshAsset)
	{
		reinterpret_cast<NiApexRenderMeshAsset*>(renderMeshAsset)->applyTransformation(transformation, 1.0f);	// This transformation function properly handles non-uniform scales
	}
}

void DestructibleAsset::traceSurfaceBoundary(physx::Array<physx::PxVec3>& outPoints, physx::PxU16 chunkIndex, const physx::PxMat34Legacy& localToWorldRT, const physx::PxVec3& scale,
        physx::PxF32 spacing, physx::PxF32 jitter, physx::PxF32 surfaceDistance, physx::PxU32 maxPoints)
{
	outPoints.resize(0);

	// Removing this function's implementation for now

	PX_UNUSED(chunkIndex);
	PX_UNUSED(localToWorldRT);
	PX_UNUSED(scale);
	PX_UNUSED(spacing);
	PX_UNUSED(jitter);
	PX_UNUSED(surfaceDistance);
	PX_UNUSED(maxPoints);
}


bool DestructibleAsset::rebuildCollisionGeometry(physx::PxU32 partIndex, const NxDestructibleGeometryDesc& geometryDesc)
{
#ifdef WITHOUT_APEX_AUTHORING
	PX_UNUSED(partIndex);
	PX_UNUSED(geometryDesc);
	APEX_DEBUG_WARNING("NxDestructibleAsset::rebuildCollisionGeometry is not available in release builds.");
	return false;
#else
	NxParameterized::Handle handle(*mParams);
	mParams->getParameterHandle("chunkConvexHulls", handle);
	NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();

	const physx::PxU32 startHullIndex = mParams->chunkConvexHullStartIndices.buf[partIndex];
	physx::PxU32 hullCount = geometryDesc.convexHullCount;
	const physx::NxExplicitHierarchicalMesh::NxConvexHull** convexHulls = geometryDesc.convexHulls;
	physx::Array<physx::PartConvexHullProxy*> collision;
	if (hullCount == 0 && geometryDesc.collisionVolumeDesc != NULL)
	{
		physx::Array<physx::PxVec3> vertices;
		physx::Array<physx::PxU32> indices;
		gatherPartMesh(vertices, indices, renderMeshAsset, partIndex);
		buildCollisionGeometry(collision, *geometryDesc.collisionVolumeDesc, vertices.begin(), vertices.size(), sizeof(physx::PxVec3), indices.begin(), indices.size());
		convexHulls = (const physx::NxExplicitHierarchicalMesh::NxConvexHull**)collision.begin();
		hullCount = collision.size();
	}
	const physx::PxU32 oldStopHullIndex = mParams->chunkConvexHullStartIndices.buf[partIndex + 1];
	const physx::PxU32 oldHullCount = oldStopHullIndex - startHullIndex;
	const physx::PxU32 stopHullIndex = startHullIndex + hullCount;
	const physx::PxI32 hullCountDelta = (physx::PxI32)(hullCount - oldHullCount);

	// Adjust start indices
	for (physx::PxU32 i = partIndex+1; i < (physx::PxU32)mParams->chunkConvexHullStartIndices.arraySizes[0]; ++i)
	{
		mParams->chunkConvexHullStartIndices.buf[i] += hullCountDelta;
	}
	const physx::PxU32 totalHullCount = (physx::PxU32)mParams->chunkConvexHullStartIndices.buf[mParams->chunkConvexHullStartIndices.arraySizes[0]-1];

	// Eliminate hulls if the count has decreased
	if (hullCountDelta < 0)
	{
		for (physx::PxU32 index = stopHullIndex; index < totalHullCount; ++index)
		{
			chunkConvexHulls[index].mParams->copy( *chunkConvexHulls[index-hullCountDelta].mParams );
		}
	}

	// Resize the hull arrays
	if (hullCountDelta != 0)
	{
		mParams->resizeArray(handle, (physx::PxI32)totalHullCount);
		chunkConvexHulls.resize(totalHullCount);
	}

	// Insert hulls if the count has increased
	if (hullCountDelta > 0)
	{
		for (physx::PxU32 index = totalHullCount; index-- > stopHullIndex;)
		{
			mParams->chunkConvexHulls.buf[index] = mParams->chunkConvexHulls.buf[index-hullCountDelta];
			chunkConvexHulls[index].init(mParams->chunkConvexHulls.buf[index]);
		}
		for (physx::PxU32 index = oldStopHullIndex; index < stopHullIndex; ++index)
		{
			mParams->chunkConvexHulls.buf[index] = traits->createNxParameterized(ConvexHullParameters::staticClassName());
			chunkConvexHulls[index].init(mParams->chunkConvexHulls.buf[index]);
		}
	}

	if (hullCount > 0)
	{
		for (physx::PxU32 hullNum = 0; hullNum < hullCount; ++hullNum)
		{
			ConvexHull& chunkHullData = chunkConvexHulls[startHullIndex + hullNum];
			physx::PartConvexHullProxy* convexHullProxy = (physx::PartConvexHullProxy*)convexHulls[hullNum];
			chunkHullData.mParams->copy(*convexHullProxy->impl.mParams);
			if (chunkHullData.isEmpty())
			{
				chunkHullData.buildFromAABB(renderMeshAsset->getBounds(partIndex));	// \todo: need better way of simplifying
			}
		}
	}

	return hullCount > 0;
#endif
}


// DestructibleAssetAuthoring
#ifndef WITHOUT_APEX_AUTHORING



void gatherPartMesh(physx::Array<physx::PxVec3>& vertices,
					physx::Array<physx::PxU32>& indices,
					const NxRenderMeshAsset* renderMeshAsset,
					physx::PxU32 partIndex)
{
	if (renderMeshAsset == NULL || partIndex >= renderMeshAsset->getPartCount())
	{
		vertices.resize(0);
		indices.resize(0);
		return;
	}


	// Pre-count vertices and indices so we can allocate once
	physx::PxU32 vertexCount = 0;
	physx::PxU32 indexCount = 0;
	for (physx::PxU32 submeshIndex = 0; submeshIndex < renderMeshAsset->getSubmeshCount(); ++submeshIndex)
	{
		const NxRenderSubmesh& submesh = renderMeshAsset->getSubmesh(submeshIndex);
		vertexCount += submesh.getVertexCount(partIndex);
		indexCount += submesh.getIndexCount(partIndex);
	}

	vertices.resize(vertexCount);
	indices.resize(indexCount);

	vertexCount = 0;
	indexCount = 0;
	for (physx::PxU32 submeshIndex = 0; submeshIndex < renderMeshAsset->getSubmeshCount(); ++submeshIndex)
	{
		const NxRenderSubmesh& submesh = renderMeshAsset->getSubmesh(submeshIndex);
		if (submesh.getVertexCount(partIndex) > 0)
		{
			const NxVertexBuffer& vertexBuffer = submesh.getVertexBuffer();
			const NxVertexFormat& vertexFormat = vertexBuffer.getFormat();
			const physx::PxI32 bufferIndex = vertexFormat.getBufferIndexFromID(vertexFormat.getSemanticID(physx::NxRenderVertexSemantic::POSITION));
			if (bufferIndex >= 0)
			{
				vertexBuffer.getBufferData(&vertices[vertexCount], physx::NxRenderDataFormat::FLOAT3, sizeof(physx::PxVec3), (physx::PxU32)bufferIndex, 
					submesh.getFirstVertexIndex(partIndex), submesh.getVertexCount(partIndex));
			}
			else
			{
				memset(&vertices[vertexCount], 0, submesh.getVertexCount(partIndex)*sizeof(physx::PxVec3));
			}
			const physx::PxU32* partIndexBuffer = submesh.getIndexBuffer(partIndex);
			const physx::PxU32 partIndexCount = submesh.getIndexCount(partIndex);
			for (physx::PxU32 indexNum = 0; indexNum < partIndexCount; ++indexNum)
			{
				indices[indexCount++] = partIndexBuffer[indexNum] + vertexCount - submesh.getFirstVertexIndex(partIndex);
			}
			vertexCount += submesh.getVertexCount(partIndex);
		}
	}
}


/**
	Private function - it's way too finicky and really only meant to serve the (public) trimCollisionVolumes function.
	The chunkIndices array is expected to contain every member of an instanced set.  That is, if a chunk indexed by some element of chunkIndices
	references partIndex, then *every* chunk that references partIndex should be in the chunkIndices array.
	Also, all chunks in chunkIndices are expected to be at the same depth.
*/
void DestructibleAssetAuthoring::trimCollisionGeometryInternal(const physx::PxU32* chunkIndices, physx::PxU32 chunkIndexCount, const physx::Array<IntPair>& parentDepthOverlaps, physx::PxU32 depth, physx::PxF32 maxTrimFraction)
{
	/*
		1) Find overlaps between each chunk (hull to hull).
		2) For each overlap:
			a) Create a trim plane for the overlapping hulls (one for each).  Adjust the trim planes to respect maxTrimFraction.
		3) For each chunk:
			a) For each hull in the chunk:
				i) Add additional trim planes from all parent chunks' neighbors' hulls, and sibling's hulls.  Adjust trim planes to respect maxTrimFraction.
		4) Intersect the trimmed hull(s) from (2) and (3) with the trim planes.  This may cause some hulls to disappear.
		5) Recurse to children
	*/

	// Create an epslion
	physx::PxBounds3 bounds = renderMeshAsset->getBounds();
	const physx::PxF32 sizeScale = (bounds.minimum - bounds.maximum).magnitude();
	const physx::PxF32 eps = 0.0001f * sizeScale;	// \todo - expose?

	//	1) Find overlaps between each chunk (hull to hull).
	const physx::PxMat44 identityTM(physx::PxVec4(1.0f));
	const physx::PxVec3 identityScale(1.0f);

	// Find AABB overlaps
	physx::Array<BoundsRep> chunkBoundsReps;
	chunkBoundsReps.reserve(chunkIndexCount);
	for (physx::PxU32 chunkNum = 0; chunkNum < chunkIndexCount; ++chunkNum)
	{
		const physx::PxU32 chunkIndex = chunkIndices[chunkNum];
		BoundsRep& chunkBoundsRep = chunkBoundsReps.insert();
		chunkBoundsRep.aabb = getChunkActorLocalBounds(chunkIndex);
	}
	physx::Array<IntPair> overlaps;
	if (chunkBoundsReps.size() > 0)
	{
		boundsCalculateOverlaps(overlaps, Bounds3XYZ, &chunkBoundsReps[0], chunkBoundsReps.size(), sizeof(chunkBoundsReps[0]));
	}

	// We'll store the trim planes here
	physx::Array<TrimPlane> trimPlanes;

	// Now do detailed overlap test
	for (physx::PxU32 overlapIndex = 0; overlapIndex < overlaps.size(); ++overlapIndex)
	{
		IntPair& AABBOverlap = overlaps[overlapIndex];
		const physx::PxU32 chunkIndex0 = chunkIndices[AABBOverlap.i0];
		const physx::PxU32 chunkIndex1 = chunkIndices[AABBOverlap.i1];

		// Offset chunks (in case they are instanced)
		const physx::PxMat44 tm0(physx::PxMat33::createIdentity(), getChunkPositionOffset(chunkIndex0));
		const physx::PxMat44 tm1(physx::PxMat33::createIdentity(), getChunkPositionOffset(chunkIndex1));
		for (physx::PxU32 hullIndex0 = getChunkHullIndexStart(chunkIndex0); hullIndex0 < getChunkHullIndexStop(chunkIndex0); ++hullIndex0)
		{
			TrimPlane trimPlane0;
			trimPlane0.hullIndex = hullIndex0;
			trimPlane0.partIndex = getPartIndex(chunkIndex0);
			for (physx::PxU32 hullIndex1 = getChunkHullIndexStart(chunkIndex1); hullIndex1 < getChunkHullIndexStop(chunkIndex1); ++hullIndex1)
			{
				TrimPlane trimPlane1;
				trimPlane1.hullIndex = hullIndex1;
				trimPlane1.partIndex = getPartIndex(chunkIndex1);
				ConvexHull::Separation separation;
				if (ConvexHull::hullsInProximity(chunkConvexHulls[hullIndex0], tm0, identityScale, chunkConvexHulls[hullIndex1], tm1, identityScale, 0.0f, &separation))
				{
					//	2) For each overlap:
					//		a) Create a trim plane for the overlapping hulls (one for each).  Adjust the trim planes to respect maxTrimFraction.
					trimPlane0.plane = separation.plane;
					trimPlane0.plane.d = physx::PxMin(trimPlane0.plane.d, maxTrimFraction*(separation.max0-separation.min0) - separation.max0);	// Bound clip distance
					trimPlane0.plane.d += trimPlane0.plane.n.dot(tm0.getPosition());	// Transform back into part local space
					trimPlanes.pushBack(trimPlane0);
					trimPlane1.plane = physx::PxPlane(-separation.plane.n, -separation.plane.d);
					trimPlane1.plane.d = physx::PxMin(trimPlane1.plane.d, maxTrimFraction*(separation.max1-separation.min1) + separation.min1);	// Bound clip distance
					trimPlane1.plane.d += trimPlane1.plane.n.dot(tm1.getPosition());	// Transform back into part local space
					trimPlanes.pushBack(trimPlane1);
				}
			}
		}
	}

	// Get overlaps for this depth
	physx::Array<IntPair> overlapsAtDepth;
	calculateChunkOverlaps(overlapsAtDepth, depth);

	//	3) For each chunk:
	for (physx::PxU32 chunkNum = 0; chunkNum < chunkIndexCount; ++chunkNum)
	{
		const physx::PxU32 chunkIndex = chunkIndices[chunkNum];
		const physx::PxMat44 tm0(physx::PxMat33::createIdentity(), getChunkPositionOffset(chunkIndex));
		const physx::PxI32 chunkParentIndex = mParams->chunks.buf[chunkIndex].parentIndex;

		for (int ancestorLevel = 0; ancestorLevel < 2; ++ancestorLevel)	// First time iterate through uncles, second time through siblings
		{
			// Optimization opportunity: symmetrize, sort and index the overlap list
			physx::PxU32 overlapCount;
			const IntPair* overlapArray;
			if (ancestorLevel == 0)
			{
				overlapCount = parentDepthOverlaps.size();
				overlapArray = parentDepthOverlaps.begin();
			}
			else
			{
				overlapCount = overlapsAtDepth.size();
				overlapArray = overlapsAtDepth.begin();
			}
			for (physx::PxU32 overlapNum = 0; overlapNum < overlapCount; ++overlapNum)
			{
				const IntPair& overlap = overlapArray[overlapNum];
				physx::PxU32 otherChunkIndex;
				const physx::PxI32 ignoreChunkIndex = ancestorLevel == 0 ? chunkParentIndex : (physx::PxI32)chunkIndex;
				if (overlap.i0 == ignoreChunkIndex)
				{
					otherChunkIndex = (physx::PxU32)overlap.i1;
				}
				else
				if (overlap.i1 == ignoreChunkIndex)
				{
					otherChunkIndex = (physx::PxU32)overlap.i0;
				}
				else
				{
					continue;
				}
				// Make sure we're not trimming from a chunk already in our chunk list (we've handled that already)
				bool alreadyConsidered = false;
				if (ancestorLevel == 1)
				{
					for (physx::PxU32 checkNum = 0; checkNum < chunkIndexCount; ++checkNum)
					{
						if (chunkIndices[checkNum] == otherChunkIndex)
						{
							alreadyConsidered = true;
							break;
						}
					}
				}
				if (alreadyConsidered)
				{
					continue;
				}
				// Check other Chunk
				const physx::PxMat44 tm1(physx::PxMat33::createIdentity(), getChunkPositionOffset(otherChunkIndex));
				//	a) For each hull in the chunk:
				for (physx::PxU32 hullIndex0 = getChunkHullIndexStart(chunkIndex); hullIndex0 < getChunkHullIndexStop(chunkIndex); ++hullIndex0)
				{
					TrimPlane trimPlane;
					trimPlane.hullIndex = hullIndex0;
					trimPlane.partIndex = getPartIndex(chunkIndex);
					for (physx::PxU32 hullIndex1 = getChunkHullIndexStart(otherChunkIndex); hullIndex1 < getChunkHullIndexStop(otherChunkIndex); ++hullIndex1)
					{
						ConvexHull::Separation separation;
						if (ConvexHull::hullsInProximity(chunkConvexHulls[hullIndex0], tm0, identityScale, chunkConvexHulls[hullIndex1], tm1, identityScale, 0.0f, &separation))
						{
							if (PxMax(separation.min0 - separation.max1, separation.min1 - separation.max0) < -eps)
							{
								trimPlane.plane = separation.plane;
								trimPlane.plane.d = -separation.min1;	// Allow for other hull to intrude completely
								trimPlane.plane.d = physx::PxMin(trimPlane.plane.d, maxTrimFraction*(separation.max0-separation.min0) - separation.max0);	// Bound clip distance
								trimPlane.plane.d += trimPlane.plane.n.dot(tm0.getPosition());	// Transform back into part local space
								trimPlanes.pushBack(trimPlane);
							}
						}
					}
				}
			}
		}
	}

	//	4) Intersect the trimmed hull(s) from (2) and (3) with the trim planes.  This may cause some hulls to disappear.

	// Sort by part, then by hull.  We're going to get a little rough with these parts.
	physx::sort<TrimPlane, TrimPlane::LessThan>(trimPlanes.begin(), trimPlanes.size(), TrimPlane::LessThan());

	// Create a lookup into the array
	physx::Array<physx::PxU32> trimPlaneLookup;
	createIndexStartLookup(trimPlaneLookup, 0, renderMeshAsset->getPartCount(), trimPlanes.size() > 0 ? (physx::PxI32*)&trimPlanes[0].partIndex : NULL, trimPlanes.size(), sizeof(TrimPlane));

	// Trim
	for (physx::PxU32 partIndex = 0; partIndex < renderMeshAsset->getPartCount(); ++partIndex)
	{
		bool hullCountChanged = false;
		for (physx::PxU32 partTrimPlaneIndex = trimPlaneLookup[partIndex]; partTrimPlaneIndex < trimPlaneLookup[partIndex+1]; ++partTrimPlaneIndex)
		{
			TrimPlane& trimPlane = trimPlanes[partTrimPlaneIndex];
			ConvexHull& hull = chunkConvexHulls[trimPlane.hullIndex];
			hull.intersectPlaneSide(trimPlane.plane);
			if (hull.isEmpty())
			{
				hullCountChanged = true;
			}
		}
		if (hullCountChanged)
		{
			// Re-apply hulls that haven't disappeared
			physx::Array<physx::PartConvexHullProxy*> newHulls;
			for (physx::PxU32 hullIndex = mParams->chunkConvexHullStartIndices.buf[partIndex]; hullIndex < mParams->chunkConvexHullStartIndices.buf[partIndex+1]; ++hullIndex)
			{
				ConvexHull& hull = chunkConvexHulls[hullIndex];
				if (!hull.isEmpty())
				{
					physx::PartConvexHullProxy* newHull = PX_NEW(physx::PartConvexHullProxy);
					newHulls.pushBack(newHull);
					newHull->impl.init(hull.mParams);
				}
			}
			NxDestructibleGeometryDesc geometryDesc;
			physx::NxCollisionVolumeDesc collisionVolumeDesc;
			if (newHulls.size() > 0)
			{
				geometryDesc.convexHulls = (const physx::NxExplicitHierarchicalMesh::NxConvexHull**)newHulls.begin();
				geometryDesc.convexHullCount = newHulls.size();
			}
			else
			{
				// We've lost all of the collision volume!  Quite a shame... create a hull to replace it.
				geometryDesc.collisionVolumeDesc = &collisionVolumeDesc;
				collisionVolumeDesc.mHullMethod = NxConvexHullMethod::WRAP_GRAPHICS_MESH;
			}
			rebuildCollisionGeometry(partIndex, geometryDesc);
			for (physx::PxU32 hullN = 0; hullN < newHulls.size(); ++hullN)
			{
				PX_DELETE(newHulls[hullN]);
			}
		}
	}

	//	5) Recurse to children

	// Iterate through chunks and collect children
	physx::Array<PxU32> childChunkIndices;
	for (physx::PxU32 chunkNum = 0; chunkNum < chunkIndexCount; ++chunkNum)
	{
		const physx::PxU32 chunkIndex = chunkIndices[chunkNum];
		const physx::PxU16 firstChildIndex = mParams->chunks.buf[chunkIndex].firstChildIndex;
		for (physx::PxU16 childNum = 0; childNum < mParams->chunks.buf[chunkIndex].numChildren; ++childNum)
		{
			childChunkIndices.pushBack((physx::PxU32)(firstChildIndex + childNum));
		}
	}

	// Recurse
	if (childChunkIndices.size())
	{
		trimCollisionGeometryInternal(childChunkIndices.begin(), childChunkIndices.size(), overlapsAtDepth, depth+1, maxTrimFraction);
	}
}

struct PartAndChunk
{
	physx::PxU32 chunkIndex;
	physx::PxI32 partIndex;

	struct LessThan
	{
		PX_INLINE bool operator()(const PartAndChunk& x, const PartAndChunk& y) const
		{
			return x.partIndex != y.partIndex ? (x.partIndex < y.partIndex) : (x.chunkIndex < y.chunkIndex);
		}
	};
};

// Here's our chunk list element, with depth (for sorting)
struct ChunkAndDepth
{
	physx::PxU32 chunkIndex;
	physx::PxI32 depth;

	struct LessThan
	{
		PX_INLINE bool operator()(const ChunkAndDepth& x, const ChunkAndDepth& y) const
		{
			return x.depth != y.depth ? (x.depth < y.depth) : (x.chunkIndex < y.chunkIndex);
		}
	};
};

void DestructibleAssetAuthoring::trimCollisionGeometry(const physx::PxU32* partIndices, physx::PxU32 partIndexCount, physx::PxF32 maxTrimFraction)
{
	/*
		1) Create a list of chunks which reference each partIndex. (If there is instancing, there may be more than one chunk per part.)
		2) Sort by depth (stable sort)
		3) For each depth:
			a) Collect a list of chunks at that depth, and call trimCollisionVolumesInternal.
	*/

	//	1) Create a list of chunks which reference each partIndex. (If there is instancing, there may be more than one chunk per part.)

	// Fill array and sort
	physx::Array<PartAndChunk> partAndChunkList;
	partAndChunkList.resize(getChunkCount());
	for (physx::PxU32 chunkIndex = 0; chunkIndex < getChunkCount(); ++chunkIndex)
	{
		partAndChunkList[chunkIndex].chunkIndex = chunkIndex;
		partAndChunkList[chunkIndex].partIndex = (physx::PxI32)getPartIndex(chunkIndex);
	}
	physx::sort<PartAndChunk, PartAndChunk::LessThan>(partAndChunkList.begin(), partAndChunkList.size(), PartAndChunk::LessThan());

	// Create a lookup into the array
	physx::Array<physx::PxU32> partAndChunkLookup;
	createIndexStartLookup(partAndChunkLookup, 0, renderMeshAsset->getPartCount(), &partAndChunkList[0].partIndex, getChunkCount(), sizeof(PartAndChunk));

	//	2) Sort by depth (stable sort)

	// Count how many chunks there will be
	physx::PxU32 chunkListSize = 0;
	for (physx::PxU32 partNum = 0; partNum < partIndexCount; ++partNum)
	{
		const physx::PxU32 partIndex = partIndices[partNum];
		chunkListSize += partAndChunkLookup[partIndex+1] - partAndChunkLookup[partIndex];
	}

	// Fill and sort
	physx::Array<ChunkAndDepth> chunkAndDepthList;
	chunkAndDepthList.resize(chunkListSize);
	physx::PxU32 chunkNum = 0;
	for (physx::PxU32 partNum = 0; partNum < partIndexCount; ++partNum)
	{
		const physx::PxU32 partIndex = partIndices[partNum];
		for (physx::PxU32 partChunkNum = partAndChunkLookup[partIndex]; partChunkNum < partAndChunkLookup[partIndex+1]; ++partChunkNum, ++chunkNum)
		{
			const physx::PxU32 chunkIndex = partAndChunkList[partChunkNum].chunkIndex;
			chunkAndDepthList[chunkNum].chunkIndex = chunkIndex;
			chunkAndDepthList[chunkNum].depth = mParams->chunks.buf[chunkIndex].depth;
		}
	}
	physx::sort<ChunkAndDepth, ChunkAndDepth::LessThan>(chunkAndDepthList.begin(), chunkAndDepthList.size(), ChunkAndDepth::LessThan());

	// And create a lookup into the array
	physx::Array<physx::PxU32> chunkAndDepthLookup;
	createIndexStartLookup(chunkAndDepthLookup, 0, mParams->depthCount, &chunkAndDepthList[0].depth, chunkListSize, sizeof(ChunkAndDepth));

	// 3) For each depth:
	for (physx::PxU32 depth = 0; depth < mParams->depthCount; ++depth)
	{
		physx::Array<physx::PxU32> chunkIndexList;
		chunkIndexList.resize(chunkAndDepthLookup[depth+1] - chunkAndDepthLookup[depth]);
		if (chunkIndexList.size() == 0)
		{
			continue;
		}
		physx::PxU32 chunkIndexListSize = 0;
		for (physx::PxU32 depthChunkNum = chunkAndDepthLookup[depth]; depthChunkNum < chunkAndDepthLookup[depth+1]; ++depthChunkNum)
		{
			chunkIndexList[chunkIndexListSize++] = chunkAndDepthList[depthChunkNum].chunkIndex;
		}
		PX_ASSERT(chunkIndexListSize == chunkIndexList.size());
		physx::Array<IntPair> overlaps;
		if (depth > 0)
		{
			calculateChunkOverlaps(overlaps, depth-1);
		}
		trimCollisionGeometryInternal(chunkIndexList.begin(), chunkIndexList.size(), overlaps, depth, maxTrimFraction);
	}
}

void DestructibleAssetAuthoring::setToolString(const char* toolString)
{
	if (mParams != NULL)
	{
		NxParameterized::Handle handle(*mParams, "comments");
		PX_ASSERT(handle.isValid());
		if (handle.isValid())
		{
			PX_ASSERT(handle.parameterDefinition()->type() == NxParameterized::TYPE_STRING);
			handle.setParamString(toolString);
		}
	}
}

void DestructibleAssetAuthoring::cookChunks(const NxDestructibleAssetCookingDesc& cookingDesc, bool cacheOverlaps, PxU32* chunkIndexMapUser2Apex, PxU32* chunkIndexMapApex2User, PxU32 chunkIndexMapCount)
{
	if (!cookingDesc.isValid())
	{
		APEX_INVALID_PARAMETER("DestructibleAssetAuthoring::cookChunks: cookingDesc invalid.");
		return;
	}

	const physx::PxU32 numChunks = cookingDesc.chunkDescCount;
	const physx::PxU32 numBehaviorGroups = cookingDesc.behaviorGroupDescCount;
	const physx::PxU32 numParts = renderMeshAsset->getPartCount();

	if ((chunkIndexMapUser2Apex != NULL || chunkIndexMapApex2User != NULL) && chunkIndexMapCount < numChunks)
	{
		APEX_INVALID_PARAMETER("DestructibleAssetAuthoring::cookChunks: chunkIndexMap is not big enough.");
		return;
	}

	NxParameterized::Handle handle(*mParams);

	mParams->getParameterHandle("chunks", handle);
	mParams->resizeArray(handle, (physx::PxI32)numChunks);
	mParams->depthCount = 0;

	// Create convex hulls
	mParams->getParameterHandle("chunkConvexHulls", handle);
	mParams->resizeArray(handle, 0);
	chunkConvexHulls.reset();

	mParams->getParameterHandle("chunkConvexHullStartIndices", handle);
	mParams->resizeArray(handle, (physx::PxI32)numParts + 1);
	for (physx::PxU32 partIndex = 0; partIndex <= numParts; ++partIndex)
	{
		mParams->chunkConvexHullStartIndices.buf[partIndex] = 0;	// Initialize all to zero, so that the random-access rebuildCollisionGeometry does the right thing (below)
	}
	for (physx::PxU32 partIndex = 0; partIndex < numParts; ++partIndex)
	{
		if (!rebuildCollisionGeometry(partIndex, cookingDesc.geometryDescs[partIndex]))
		{
			APEX_INVALID_PARAMETER("DestructibleAssetAuthoring::cookChunks: Could not find or generate collision hull for part.");
		}
	}

	// Sort - chunks must be in parent-sorted order
	Array<ChunkSortElement> sortElements;
	sortElements.resize(numChunks);
	for (physx::PxU32 i = 0; i < numChunks; ++i)
	{
		const NxDestructibleChunkDesc& chunkDesc = cookingDesc.chunkDescs[i];
		sortElements[i].index = (physx::PxI32)i;
		sortElements[i].parentIndex = chunkDesc.parentIndex;
		sortElements[i].depth = 0;
		physx::PxI32 parent = (physx::PxI32)i;
		physx::PxU32 counter = 0;
		while ((parent = cookingDesc.chunkDescs[parent].parentIndex) >= 0)
		{
			++sortElements[i].depth;
			if (++counter > numChunks)
			{
				APEX_INVALID_PARAMETER("DestructibleAssetAuthoring::cookChunks: loop found in cookingDesc parent indices.  Cannot build an NxDestructibleAsset.");
				return;
			}
		}
	}
	qsort(sortElements.begin(), numChunks, sizeof(ChunkSortElement), compareChunkParents);

	Array<physx::PxU32> ranks;
	if (chunkIndexMapUser2Apex == NULL && numChunks > 0)
	{
		ranks.resize(numChunks);
		chunkIndexMapUser2Apex = &ranks[0];
	}

	for (physx::PxU32 i = 0; i < numChunks; ++i)
	{
		chunkIndexMapUser2Apex[sortElements[i].index] = i;
		if (chunkIndexMapApex2User != NULL)
		{
			chunkIndexMapApex2User[i] = (physx::PxU32)sortElements[i].index;
		}
	}

	// Count instanced chunks and allocate instanced info array
	physx::PxU32 instancedChunkCount = 0;
	for (physx::PxU32 i = 0; i < numChunks; ++i)
	{
		const NxDestructibleChunkDesc& chunkDesc = cookingDesc.chunkDescs[sortElements[i].index];
		if (chunkDesc.useInstancedRendering)
		{
			++instancedChunkCount;
		}
	}

	mParams->getParameterHandle("chunkInstanceInfo", handle);
	mParams->resizeArray(handle, (physx::PxI32)instancedChunkCount);

	mParams->getParameterHandle("scatterMeshIndices", handle);
	mParams->resizeArray(handle, 0);
	mParams->getParameterHandle("scatterMeshTransforms", handle);
	mParams->resizeArray(handle, 0);

	const NxDestructibleChunkDesc defaultChunkDesc;

	instancedChunkCount = 0;	// reset and use as cursor

	for (physx::PxU32 i = 0; i < numChunks; ++i)
	{
		DestructibleAssetParametersNS::Chunk_Type& chunk = mParams->chunks.buf[i];
		const NxDestructibleChunkDesc& chunkDesc = cookingDesc.chunkDescs[sortElements[i].index];
		chunk.flags = 0;
		if (chunkDesc.isSupportChunk)
		{
			chunk.flags |= SupportChunk;
		}
		if (chunkDesc.doNotFracture)
		{
			chunk.flags |= UnfracturableChunk;
		}
		if (chunkDesc.doNotDamage)
		{
			chunk.flags |= UndamageableChunk;
		}
		if (chunkDesc.doNotCrumble)
		{
			chunk.flags |= UncrumbleableChunk;
		}
#if APEX_RUNTIME_FRACTURE
		if (chunkDesc.runtimeFracture)
		{
			chunk.flags |= RuntimeFracturableChunk;
		}
#endif
		if (!chunkDesc.useInstancedRendering)
		{
			// Not instanced, meshPartIndex will be used to directly access the mesh part in the "normal" mesh
			chunk.meshPartIndex = chunkDesc.meshIndex;
		}
		else
		{
			// Instanced, meshPartIndex will be used to access instance info
			chunk.flags |= Instanced;
			chunk.meshPartIndex = (physx::PxU16)instancedChunkCount++;
			DestructibleAssetParametersNS::InstanceInfo_Type& instanceInfo = mParams->chunkInstanceInfo.buf[chunk.meshPartIndex];
			instanceInfo.partIndex = chunkDesc.meshIndex;
			instanceInfo.chunkPositionOffset = chunkDesc.instancePositionOffset;
			instanceInfo.chunkUVOffset = chunkDesc.instanceUVOffset;
		}
		if (sortElements[i].index == 0)
		{
			chunk.depth = 0;
			chunk.parentIndex = DestructibleAsset::InvalidChunkIndex;
		}
		else
		{
			chunk.parentIndex = (physx::PxU16)chunkIndexMapUser2Apex[(physx::PxI16)chunkDesc.parentIndex];
			DestructibleAssetParametersNS::Chunk_Type& parent = mParams->chunks.buf[chunk.parentIndex];
			PX_ASSERT(chunk.parentIndex >= mParams->chunks.buf[i - 1].parentIndex ||
			          mParams->chunks.buf[i - 1].parentIndex == DestructibleAsset::InvalidChunkIndex);	// Parent-sorted order
			if (chunk.parentIndex != mParams->chunks.buf[i - 1].parentIndex)
			{
				parent.firstChildIndex = (physx::PxU16)i;
			}
			++parent.numChildren;
			chunk.depth = PxU16(parent.depth + 1);
			if ((parent.flags & SupportChunk) != 0)
			{
				chunk.flags |= SupportChunk;	// All children of support chunks can be support chunks
			}
			if ((parent.flags & UnfracturableChunk) != 0)
			{
				chunk.flags |= UnfracturableChunk;	// All children of unfracturable chunks are unfracturable
			}
#if APEX_RUNTIME_FRACTURE
			if ((parent.flags & RuntimeFracturableChunk) != 0) // Runtime fracturable chunks cannot have any children
			{
				PX_ALWAYS_ASSERT();
			}
#endif
		}
		if ((chunk.flags & UnfracturableChunk) != 0)
		{
			// All ancestors of unfracturable chunks must be unfracturable or note that they have an unfracturable descendant
			physx::PxU16 parentIndex = chunk.parentIndex;
			while (parentIndex != DestructibleAsset::InvalidChunkIndex)
			{
				DestructibleAssetParametersNS::Chunk_Type& parent = mParams->chunks.buf[parentIndex];
				if ((parent.flags & UnfracturableChunk) == 0)
				{
					parent.flags |= DescendantUnfractureable;
				}
				parentIndex = parent.parentIndex;
			}
		}
		chunk.numChildren = 0;
		chunk.firstChildIndex = DestructibleAsset::InvalidChunkIndex;
		mParams->depthCount = physx::PxMax((physx::PxU16)mParams->depthCount, (physx::PxU16)(chunk.depth + 1));
		chunk.surfaceNormal = chunkDesc.surfaceNormal;
		chunk.behaviorGroupIndex = chunkDesc.behaviorGroupIndex;

		// Default behavior is to take on the parent's behavior group
		if (chunk.parentIndex != DestructibleAsset::InvalidChunkIndex)
		{
			if (chunk.behaviorGroupIndex < 0)
			{
				chunk.behaviorGroupIndex = mParams->chunks.buf[chunk.parentIndex].behaviorGroupIndex;
			}
		}

		// Scatter mesh
		const physx::PxI32 oldScatterMeshCount = mParams->scatterMeshIndices.arraySizes[0];
		chunk.firstScatterMesh = (physx::PxU16)oldScatterMeshCount;
		chunk.scatterMeshCount = (physx::PxU16)chunkDesc.scatterMeshCount;
		if (chunk.scatterMeshCount > 0)
		{
			mParams->getParameterHandle("scatterMeshIndices", handle);
			mParams->resizeArray(handle, oldScatterMeshCount + (physx::PxI32)chunk.scatterMeshCount);
			handle.setParamU8Array(chunkDesc.scatterMeshIndices, (physx::PxI32)chunk.scatterMeshCount, oldScatterMeshCount);
			mParams->getParameterHandle("scatterMeshTransforms", handle);
			mParams->resizeArray(handle, oldScatterMeshCount + (physx::PxI32)chunk.scatterMeshCount);
			for (physx::PxU16 tmNum = 0; tmNum < chunk.scatterMeshCount; ++tmNum)
			{
				DestructibleAssetParametersNS::M34_Type& tm = mParams->scatterMeshTransforms.buf[oldScatterMeshCount + tmNum];
				const physx::PxMat44& mat = chunkDesc.scatterMeshTransforms[tmNum];
				tm.matrix = physx::PxMat33(mat.getBasis(0), mat.getBasis(1), mat.getBasis(2));
				tm.vector = mat.getPosition();
			}
		}
	}
	
	mParams->getParameterHandle("behaviorGroups", handle);
	mParams->resizeArray(handle, (physx::PxI32)numBehaviorGroups);

	struct {
		void operator() (const NxDestructibleBehaviorGroupDesc& behaviorGroupDesc, DestructibleAssetParameters* mParams, NxParameterized::Handle& elementHandle)
		{
			NxParameterized::Handle subElementHandle(*mParams);

			elementHandle.getChildHandle(mParams, "name", subElementHandle);
			mParams->setParamString(subElementHandle, behaviorGroupDesc.name);

			elementHandle.getChildHandle(mParams, "damageThreshold", subElementHandle);
			mParams->setParamF32(subElementHandle, behaviorGroupDesc.damageThreshold);
			
			elementHandle.getChildHandle(mParams, "damageToRadius", subElementHandle);
			mParams->setParamF32(subElementHandle, behaviorGroupDesc.damageToRadius);

			elementHandle.getChildHandle(mParams, "damageSpread.minimumRadius", subElementHandle);
			mParams->setParamF32(subElementHandle, behaviorGroupDesc.damageSpread.minimumRadius);

			elementHandle.getChildHandle(mParams, "damageSpread.radiusMultiplier", subElementHandle);
			mParams->setParamF32(subElementHandle, behaviorGroupDesc.damageSpread.radiusMultiplier);

			elementHandle.getChildHandle(mParams, "damageSpread.falloffExponent", subElementHandle);
			mParams->setParamF32(subElementHandle, behaviorGroupDesc.damageSpread.falloffExponent);

			elementHandle.getChildHandle(mParams, "damageColorSpread.minimumRadius", subElementHandle);
			mParams->setParamF32(subElementHandle, behaviorGroupDesc.damageColorSpread.minimumRadius);

			elementHandle.getChildHandle(mParams, "damageColorSpread.radiusMultiplier", subElementHandle);
			mParams->setParamF32(subElementHandle, behaviorGroupDesc.damageColorSpread.radiusMultiplier);

			elementHandle.getChildHandle(mParams, "damageColorSpread.falloffExponent", subElementHandle);
			mParams->setParamF32(subElementHandle, behaviorGroupDesc.damageColorSpread.falloffExponent);

			elementHandle.getChildHandle(mParams, "damageColorChange", subElementHandle);
			mParams->setParamVec4(subElementHandle, behaviorGroupDesc.damageColorChange);

			elementHandle.getChildHandle(mParams, "materialStrength", subElementHandle);
			mParams->setParamF32(subElementHandle, behaviorGroupDesc.materialStrength);

			elementHandle.getChildHandle(mParams, "density", subElementHandle);
			mParams->setParamF32(subElementHandle, behaviorGroupDesc.density);

			elementHandle.getChildHandle(mParams, "fadeOut", subElementHandle);
			mParams->setParamF32(subElementHandle, behaviorGroupDesc.fadeOut);

			elementHandle.getChildHandle(mParams, "maxDepenetrationVelocity", subElementHandle);
			mParams->setParamF32(subElementHandle, behaviorGroupDesc.maxDepenetrationVelocity);

			elementHandle.getChildHandle(mParams, "userData", subElementHandle);
			mParams->setParamU64(subElementHandle, behaviorGroupDesc.userData);
		}
	} ConvertBehaviorGroupDesc; // local function definitions illegal, eh?

	const PxU32 bufferCount = 128;
	char buffer[bufferCount] = {0};

	for (physx::PxU32 i = 0; i < numBehaviorGroups; ++i)
	{
		const NxDestructibleBehaviorGroupDesc& chunkDesc = cookingDesc.behaviorGroupDescs[i];
		NxParameterized::Handle elementHandle(*mParams);

		physx::string::sprintf_s(buffer, bufferCount, "behaviorGroups[%d]", i);
		mParams->getParameterHandle(buffer, elementHandle);

		ConvertBehaviorGroupDesc(chunkDesc, mParams, elementHandle);
	}

	mParams->getParameterHandle("defaultBehaviorGroup", handle);
	ConvertBehaviorGroupDesc(cookingDesc.defaultBehaviorGroupDesc, mParams, handle);

	mParams->RTFractureBehaviorGroup = cookingDesc.RTFractureBehaviorGroup;


	// Fill in default destructible parameters up to depth
	int oldDepthParametersSize = mParams->depthParameters.arraySizes[0];
	if (oldDepthParametersSize < (int)mParams->depthCount)
	{
		NxParameterized::Handle depthParametersHandle(*mParams);
		mParams->getParameterHandle("depthParameters", depthParametersHandle);
		mParams->resizeArray(depthParametersHandle, (physx::PxI32)mParams->depthCount);
		for (int i = oldDepthParametersSize; i < (int)mParams->depthCount; ++i)
		{
			DestructibleAssetParametersNS::DestructibleDepthParameters_Type& depthParameters = mParams->depthParameters.buf[i];
			depthParameters.OVERRIDE_IMPACT_DAMAGE = false;
			depthParameters.OVERRIDE_IMPACT_DAMAGE_VALUE = false;
			depthParameters.IGNORE_POSE_UPDATES = false;
			depthParameters.IGNORE_RAYCAST_CALLBACKS = false;
			depthParameters.IGNORE_CONTACT_CALLBACKS = false;
			depthParameters.USER_FLAG_0 = false;
			depthParameters.USER_FLAG_1 = false;
			depthParameters.USER_FLAG_2 = false;
			depthParameters.USER_FLAG_3 = false;
		}
	}

	// Build collision data and bounds
	physx::PxF32 skinWidth = 0.0025f;	// Default value
	if (NxGetApexSDK()->getCookingInterface())
	{
#if NX_SDK_VERSION_MAJOR == 2
		const NxCookingParams& cookingParams = NxGetApexSDK()->getCookingInterface()->NxGetCookingParams();
#elif NX_SDK_VERSION_MAJOR == 3
		const PxCookingParams& cookingParams = NxGetApexSDK()->getCookingInterface()->getParams();
#endif
		skinWidth = cookingParams.skinWidth;
	}

	mParams->bounds.setEmpty();
	for (physx::PxU32 partIndex = 0; partIndex < numParts; ++partIndex)
	{
		const NxDestructibleGeometryDesc& geometryDesc = cookingDesc.geometryDescs[partIndex];
		const physx::PxU32 startHullIndex = mParams->chunkConvexHullStartIndices.buf[partIndex];
		for (physx::PxU32 hullIndex = 0; hullIndex < geometryDesc.convexHullCount; ++hullIndex)
		{
			ConvexHull& chunkHullData = chunkConvexHulls[startHullIndex + hullIndex];
			physx::PartConvexHullProxy* convexHullProxy = (physx::PartConvexHullProxy*)geometryDesc.convexHulls[hullIndex];
			chunkHullData.mParams->copy(*convexHullProxy->impl.mParams);
			if (chunkHullData.isEmpty())
			{
				chunkHullData.buildFromAABB(renderMeshAsset->getBounds(partIndex));	// \todo: need better way of simplifying
			}
		}
	}

	for (physx::PxU32 chunkIndex = 0; chunkIndex < numChunks; ++chunkIndex)
	{
		DestructibleAssetParametersNS::Chunk_Type& chunk = mParams->chunks.buf[chunkIndex];
		physx::PxBounds3 bounds = getChunkActorLocalBounds(chunkIndex);
		mParams->bounds.include(bounds);
		chunk.surfaceNormal = physx::PxVec3(0.0f);
	}
	PX_ASSERT(!mParams->bounds.isEmpty());
	mParams->bounds.fattenFast(skinWidth);

	mParams->originalDepthCount = mParams->depthCount;

	calculateChunkDepthStarts();

	if (cacheOverlaps)
	{
		cacheChunkOverlapsUpToDepth(chunkOverlapCacheDepth);
	}

	Array<NxIntPair> supportGraphEdgesInternal(cookingDesc.supportGraphEdgeCount);
	if (cookingDesc.supportGraphEdgeCount > 0)
	{
		for (PxU32 i = 0; i < cookingDesc.supportGraphEdgeCount; ++i)
		{
			supportGraphEdgesInternal[i].mI0 = (physx::PxI32)chunkIndexMapUser2Apex[(physx::PxU32)cookingDesc.supportGraphEdges[i].mI0];
			supportGraphEdgesInternal[i].mI1 = (physx::PxI32)chunkIndexMapUser2Apex[(physx::PxU32)cookingDesc.supportGraphEdges[i].mI1];
		}

		addChunkOverlaps(&supportGraphEdgesInternal[0], supportGraphEdgesInternal.size());
	}

	m_needsScatterMeshInstanceInfoCreation = true;
}

void DestructibleAssetAuthoring::serializeFractureToolState(physx::PxFileBuf& stream, physx::NxExplicitHierarchicalMesh::NxEmbedding& embedding) const
{
	stream.storeDword((physx::PxU32)ApexStreamVersion::Current);
	hMesh.serialize(stream, embedding);
	hMeshCore.serialize(stream, embedding);
	cutoutSet.serialize(stream);
}

void DestructibleAssetAuthoring::deserializeFractureToolState(physx::PxFileBuf& stream, physx::NxExplicitHierarchicalMesh::NxEmbedding& embedding)
{
	const physx::PxU32 version = stream.readDword();
	PX_UNUSED(version);	// Initial version

	hMesh.deserialize(stream, embedding);
	hMeshCore.deserialize(stream, embedding);
	cutoutSet.deserialize(stream);
}
#endif

NxParameterized::Interface* DestructibleAsset::getDefaultActorDesc()
{
	NxParameterized::Interface* ret = 0;

	NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();

	// non-optimal.  Should use a copy-constructor so this only gets built once.
	if (!mApexDestructibleActorParams && traits)
	{
		mApexDestructibleActorParams = traits->createNxParameterized(DestructibleActorParam::staticClassName());
	}

	if (traits)
	{
		if (mApexDestructibleActorParams)
		{
			ret = mApexDestructibleActorParams;
			DestructibleActorParam* p = static_cast<DestructibleActorParam*>(ret);
			DestructibleActorParamNS::ParametersStruct* ps = static_cast<DestructibleActorParamNS::ParametersStruct*>(p);

			{
				NxParameterized::Handle handle(*p);
				if (p->getParameterHandle("crumbleEmitterName", handle) == NxParameterized::ERROR_NONE)
				{
					p->setParamString(handle, getCrumbleEmitterName());
				}
			}

			{
				NxParameterized::Handle handle(*p);
				if (p->getParameterHandle("dustEmitterName", handle) == NxParameterized::ERROR_NONE)
				{
					p->setParamString(handle, getDustEmitterName());
				}
			}

			NxDestructibleParameters destructibleParameters = getParameters();

			ps->destructibleParameters.damageCap								= destructibleParameters.damageCap;
			ps->destructibleParameters.forceToDamage							= destructibleParameters.forceToDamage;
			ps->destructibleParameters.impactVelocityThreshold					= destructibleParameters.impactVelocityThreshold;
			ps->destructibleParameters.minimumFractureDepth						= destructibleParameters.minimumFractureDepth;
			ps->destructibleParameters.damageDepthLimit							= destructibleParameters.damageDepthLimit;
			ps->destructibleParameters.impactDamageDefaultDepth					= destructibleParameters.impactDamageDefaultDepth;
			ps->destructibleParameters.debrisDepth								= destructibleParameters.debrisDepth;
			ps->destructibleParameters.essentialDepth							= destructibleParameters.essentialDepth;
			ps->destructibleParameters.debrisLifetimeMin						= destructibleParameters.debrisLifetimeMin;
			ps->destructibleParameters.debrisLifetimeMax						= destructibleParameters.debrisLifetimeMax;
			ps->destructibleParameters.debrisMaxSeparationMin					= destructibleParameters.debrisMaxSeparationMin;
			ps->destructibleParameters.debrisMaxSeparationMax					= destructibleParameters.debrisMaxSeparationMax;
			ps->destructibleParameters.dynamicChunksGroupsMask.useGroupsMask	= destructibleParameters.useDynamicChunksGroupsMask;
			ps->destructibleParameters.debrisDestructionProbability             = destructibleParameters.debrisDestructionProbability;
			ps->destructibleParameters.dynamicChunkDominanceGroup				= destructibleParameters.dynamicChunksDominanceGroup;
#if NX_SDK_VERSION_MAJOR == 2
			ps->destructibleParameters.dynamicChunksGroupsMask.bits0			= destructibleParameters.dynamicChunksGroupsMask.bits0;
			ps->destructibleParameters.dynamicChunksGroupsMask.bits1			= destructibleParameters.dynamicChunksGroupsMask.bits1;
			ps->destructibleParameters.dynamicChunksGroupsMask.bits2			= destructibleParameters.dynamicChunksGroupsMask.bits2;
			ps->destructibleParameters.dynamicChunksGroupsMask.bits3			= destructibleParameters.dynamicChunksGroupsMask.bits3;
#elif NX_SDK_VERSION_MAJOR == 3
			ps->destructibleParameters.dynamicChunksGroupsMask.bits0			= destructibleParameters.dynamicChunksFilterData.word0;
			ps->destructibleParameters.dynamicChunksGroupsMask.bits1			= destructibleParameters.dynamicChunksFilterData.word1;
			ps->destructibleParameters.dynamicChunksGroupsMask.bits2			= destructibleParameters.dynamicChunksFilterData.word2;
			ps->destructibleParameters.dynamicChunksGroupsMask.bits3			= destructibleParameters.dynamicChunksFilterData.word3;
			ps->destructibleParameters.supportStrength							= destructibleParameters.supportStrength;
#endif
			ps->destructibleParameters.legacyChunkBoundsTestSetting				= destructibleParameters.legacyChunkBoundsTestSetting;
			ps->destructibleParameters.legacyDamageRadiusSpreadSetting			= destructibleParameters.legacyDamageRadiusSpreadSetting;
			ps->destructibleParameters.validBounds								= destructibleParameters.validBounds;
			ps->destructibleParameters.maxChunkSpeed							= destructibleParameters.maxChunkSpeed;
			ps->destructibleParameters.flags.ACCUMULATE_DAMAGE					= (destructibleParameters.flags & NxDestructibleParametersFlag::ACCUMULATE_DAMAGE) ? true : false;
			ps->destructibleParameters.flags.DEBRIS_TIMEOUT						= (destructibleParameters.flags & NxDestructibleParametersFlag::DEBRIS_TIMEOUT) ? true : false;
			ps->destructibleParameters.flags.DEBRIS_MAX_SEPARATION				= (destructibleParameters.flags & NxDestructibleParametersFlag::DEBRIS_MAX_SEPARATION) ? true : false;
			ps->destructibleParameters.flags.CRUMBLE_SMALLEST_CHUNKS			= (destructibleParameters.flags & NxDestructibleParametersFlag::CRUMBLE_SMALLEST_CHUNKS) ? true : false;
			ps->destructibleParameters.flags.ACCURATE_RAYCASTS					= (destructibleParameters.flags & NxDestructibleParametersFlag::ACCURATE_RAYCASTS) ? true : false;
			ps->destructibleParameters.flags.USE_VALID_BOUNDS					= (destructibleParameters.flags & NxDestructibleParametersFlag::USE_VALID_BOUNDS) ? true : false;
			ps->destructibleParameters.flags.CRUMBLE_VIA_RUNTIME_FRACTURE			= (destructibleParameters.flags & NxDestructibleParametersFlag::CRUMBLE_VIA_RUNTIME_FRACTURE) ? true : false;

			ps->supportDepth			= mParams->supportDepth;
			ps->formExtendedStructures	= mParams->formExtendedStructures;
			ps->useAssetDefinedSupport	= mParams->useAssetDefinedSupport;
			ps->useWorldSupport			= mParams->useWorldSupport;

			// RT Fracture Parameters
			ps->destructibleParameters.runtimeFracture.sheetFracture			= destructibleParameters.rtFractureParameters.sheetFracture;
			ps->destructibleParameters.runtimeFracture.depthLimit				= destructibleParameters.rtFractureParameters.depthLimit;
			ps->destructibleParameters.runtimeFracture.destroyIfAtDepthLimit	= destructibleParameters.rtFractureParameters.destroyIfAtDepthLimit;
			ps->destructibleParameters.runtimeFracture.minConvexSize			= destructibleParameters.rtFractureParameters.minConvexSize;
			ps->destructibleParameters.runtimeFracture.impulseScale				= destructibleParameters.rtFractureParameters.impulseScale;
			ps->destructibleParameters.runtimeFracture.glass.numSectors			= destructibleParameters.rtFractureParameters.glass.numSectors;
			ps->destructibleParameters.runtimeFracture.glass.sectorRand			= destructibleParameters.rtFractureParameters.glass.sectorRand;
			ps->destructibleParameters.runtimeFracture.glass.firstSegmentSize	= destructibleParameters.rtFractureParameters.glass.firstSegmentSize;
			ps->destructibleParameters.runtimeFracture.glass.segmentScale		= destructibleParameters.rtFractureParameters.glass.segmentScale;
			ps->destructibleParameters.runtimeFracture.glass.segmentRand		= destructibleParameters.rtFractureParameters.glass.segmentRand;
			ps->destructibleParameters.runtimeFracture.attachment.posX			= destructibleParameters.rtFractureParameters.attachment.posX;
			ps->destructibleParameters.runtimeFracture.attachment.negX			= destructibleParameters.rtFractureParameters.attachment.negX;
			ps->destructibleParameters.runtimeFracture.attachment.posY			= destructibleParameters.rtFractureParameters.attachment.posY;
			ps->destructibleParameters.runtimeFracture.attachment.negY			= destructibleParameters.rtFractureParameters.attachment.negY;
			ps->destructibleParameters.runtimeFracture.attachment.posZ			= destructibleParameters.rtFractureParameters.attachment.posZ;
			ps->destructibleParameters.runtimeFracture.attachment.negZ			= destructibleParameters.rtFractureParameters.attachment.negZ;

			physx::PxU32 depth = destructibleParameters.depthParametersCount;
			if (depth > 0)
			{
				NxParameterized::Handle handle(*p);
				if (p->getParameterHandle("depthParameters", handle) == NxParameterized::ERROR_NONE)
				{
					p->resizeArray(handle, (physx::PxI32)depth);
					for (physx::PxU32 i = 0; i < depth; i++)
					{
						const NxDestructibleDepthParameters& dparm = destructibleParameters.depthParameters[i];
						ps->depthParameters.buf[i].OVERRIDE_IMPACT_DAMAGE		= (dparm.flags & NxDestructibleDepthParametersFlag::OVERRIDE_IMPACT_DAMAGE) ? true : false;
						ps->depthParameters.buf[i].OVERRIDE_IMPACT_DAMAGE_VALUE	= (dparm.flags & NxDestructibleDepthParametersFlag::OVERRIDE_IMPACT_DAMAGE_VALUE) ? true : false;
						ps->depthParameters.buf[i].IGNORE_POSE_UPDATES			= (dparm.flags & NxDestructibleDepthParametersFlag::IGNORE_POSE_UPDATES) ? true : false;
						ps->depthParameters.buf[i].IGNORE_RAYCAST_CALLBACKS		= (dparm.flags & NxDestructibleDepthParametersFlag::IGNORE_RAYCAST_CALLBACKS) ? true : false;
						ps->depthParameters.buf[i].IGNORE_CONTACT_CALLBACKS		= (dparm.flags & NxDestructibleDepthParametersFlag::IGNORE_CONTACT_CALLBACKS) ? true : false;
						ps->depthParameters.buf[i].USER_FLAG_0					= (dparm.flags & NxDestructibleDepthParametersFlag::USER_FLAG_0) ? true : false;
						ps->depthParameters.buf[i].USER_FLAG_1					= (dparm.flags & NxDestructibleDepthParametersFlag::USER_FLAG_1) ? true : false;
						ps->depthParameters.buf[i].USER_FLAG_2					= (dparm.flags & NxDestructibleDepthParametersFlag::USER_FLAG_2) ? true : false;
						ps->depthParameters.buf[i].USER_FLAG_3					= (dparm.flags & NxDestructibleDepthParametersFlag::USER_FLAG_3) ? true : false;
					}
				}
			}

			{
				NxParameterized::Handle handle(*p);
				if (p->getParameterHandle("behaviorGroups", handle) == NxParameterized::ERROR_NONE)
				{
					PxU32 behaviorGroupArraySize = (physx::PxU32)mParams->behaviorGroups.arraySizes[0];
					p->resizeArray(handle, (physx::PxI32)behaviorGroupArraySize);

					struct {
						void operator() (const DestructibleAssetParametersNS::BehaviorGroup_Type& src,
										 DestructibleActorParamNS::BehaviorGroup_Type& dest)
						{
							dest.damageThreshold = src.damageThreshold;
							dest.damageToRadius = src.damageToRadius;
							dest.damageSpread.minimumRadius = src.damageSpread.minimumRadius;
							dest.damageSpread.radiusMultiplier = src.damageSpread.radiusMultiplier;
							dest.damageSpread.falloffExponent = src.damageSpread.falloffExponent;
							dest.materialStrength = src.materialStrength;
							dest.density = src.density;
							dest.fadeOut = src.fadeOut;
							dest.maxDepenetrationVelocity = src.maxDepenetrationVelocity;
						}						
					} ConvertBehaviorGroup;

					ConvertBehaviorGroup(mParams->defaultBehaviorGroup, ps->defaultBehaviorGroup);
					for (physx::PxU32 i = 0; i < behaviorGroupArraySize; i++)
					{
						ConvertBehaviorGroup(mParams->behaviorGroups.buf[i],
											 ps->behaviorGroups.buf[i]);
					}
				}
			}
		}
	}
	return ret;
}

NxParameterized::Interface* DestructibleAsset::getDefaultAssetPreviewDesc()
{
	NxParameterized::Interface* ret = NULL;

	if (module != NULL)
	{
		ret = module->getApexDestructiblePreviewParams();

		if (ret != NULL)
		{
			ret->initDefaults();
		}
	}

	return ret;
}

bool DestructibleAsset::isValidForActorCreation(const ::NxParameterized::Interface& params, NxApexScene& /*apexScene*/) const
{
	return strcmp(params.className(), DestructibleActorParam::staticClassName()) == 0 ||
	       strcmp(params.className(), DestructibleActorState::staticClassName()) == 0;
}

NxApexActor* DestructibleAsset::createApexActor(const NxParameterized::Interface& params, NxApexScene& apexScene)
{
	if (!isValidForActorCreation(params, apexScene))
	{
		return NULL;
	}

	return createDestructibleActor(params, apexScene);
}

NxApexAssetPreview* DestructibleAsset::createApexAssetPreview(const NxParameterized::Interface& params, NxApexAssetPreviewScene* /*previewScene*/)
{
	DestructiblePreviewProxy* proxy = NULL;

	proxy = PX_NEW(DestructiblePreviewProxy)(*this, m_previewList, &params);
	PX_ASSERT(proxy != NULL);

	return proxy;
}

void DestructibleAsset::appendActorTransforms(const physx::PxMat44* transforms, physx::PxU32 transformCount)
		{
	NxParameterized::Handle handle(*mParams);
	mParams->getParameterHandle("actorTransforms", handle);
	const physx::PxI32 oldSize = mParams->actorTransforms.arraySizes[0];
	mParams->resizeArray(handle, (physx::PxI32)(oldSize + transformCount));
	mParams->setParamMat44Array(handle, transforms, (physx::PxI32)transformCount, oldSize);
		}

void DestructibleAsset::clearActorTransforms()
{
	NxParameterized::Handle handle(*mParams);
	mParams->getParameterHandle("actorTransforms", handle);
	mParams->resizeArray(handle, 0);
}

bool DestructibleAsset::chunksInProximity(const DestructibleAsset& asset0, physx::PxU16 chunkIndex0, const physx::PxMat44& tm0, const physx::PxVec3& scale0,
        const DestructibleAsset& asset1, physx::PxU16 chunkIndex1, const physx::PxMat44& tm1, const physx::PxVec3& scale1,
        physx::PxF32 padding)
{
	PX_ASSERT(asset0.mParams != NULL);
	PX_ASSERT(asset1.mParams != NULL);

	// Offset chunks (in case they are instanced)
	physx::PxMat44 effectiveTM0 = tm0;
	effectiveTM0.setPosition(tm0.getPosition() + tm0.rotate(scale0.multiply(asset0.getChunkPositionOffset(chunkIndex0))));

	physx::PxMat44 effectiveTM1 = tm1;
	effectiveTM1.setPosition(tm1.getPosition() + tm1.rotate(scale1.multiply(asset1.getChunkPositionOffset(chunkIndex1))));

	for (physx::PxU32 hullIndex0 = asset0.getChunkHullIndexStart(chunkIndex0); hullIndex0 < asset0.getChunkHullIndexStop(chunkIndex0); ++hullIndex0)
	{
		for (physx::PxU32 hullIndex1 = asset1.getChunkHullIndexStart(chunkIndex1); hullIndex1 < asset1.getChunkHullIndexStop(chunkIndex1); ++hullIndex1)
		{
			if (ConvexHull::hullsInProximity(asset0.chunkConvexHulls[hullIndex0], effectiveTM0, scale0, asset1.chunkConvexHulls[hullIndex1], effectiveTM1, scale1, padding))
			{
				return true;
			}
		}
	}
	return false;
}

bool DestructibleAsset::chunkAndSphereInProximity(physx::PxU16 chunkIndex, const physx::PxMat44& chunkTM, const physx::PxVec3& chunkScale, 
												  const physx::PxVec3& sphereWorldCenter, physx::PxF32 sphereRadius, physx::PxF32 padding, physx::PxF32* distance)
{
	// Offset chunk (in case it is instanced)
	physx::PxMat44 effectiveTM = chunkTM;
	effectiveTM.setPosition(chunkTM.getPosition() + chunkTM.rotate(chunkScale.multiply(getChunkPositionOffset(chunkIndex))));

	ConvexHull::Separation testSeparation;
	ConvexHull::Separation* testSeparationPtr = distance != NULL? &testSeparation : NULL;
	bool result = false;
	for (physx::PxU32 hullIndex = getChunkHullIndexStart(chunkIndex); hullIndex < getChunkHullIndexStop(chunkIndex); ++hullIndex)
	{
		if (chunkConvexHulls[hullIndex].sphereInProximity(effectiveTM, chunkScale, sphereWorldCenter, sphereRadius, padding, testSeparationPtr))
		{
			result = true;
			if (distance != NULL)
			{
				const physx::PxF32 testDistance = testSeparation.getDistance();
				if (testDistance < *distance)
				{
					*distance = testDistance;
				}
			}
		}
	}
	return result;
}


/*
	DestructibleAssetCollision
*/


DestructibleAssetCollision::DestructibleAssetCollision() :
	mAsset(NULL)
{
	NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();
	mParams = DYNAMIC_CAST(DestructibleAssetCollisionDataSet*)(traits->createNxParameterized(DestructibleAssetCollisionDataSet::staticClassName()));
	mOwnsParams = mParams != NULL;
	PX_ASSERT(mOwnsParams);
}

DestructibleAssetCollision::DestructibleAssetCollision(NxParameterized::Interface* params) :
	mAsset(NULL)
{
	mParams = DYNAMIC_CAST(DestructibleAssetCollisionDataSet*)(params);
	mOwnsParams = false;
}

DestructibleAssetCollision::~DestructibleAssetCollision()
{
	resize(0);

	if (mParams != NULL && mOwnsParams)
	{
		mParams->destroy();
	}
	mParams = NULL;
	mOwnsParams = false;
}

void DestructibleAssetCollision::setDestructibleAssetToCook(DestructibleAsset* asset)
{
	if (asset == NULL || getAssetName() == NULL || strcmp(asset->getName(), getAssetName()))
	{
		resize(0);
	}

	mAsset = asset;

	NxParameterized::Handle handle(*mParams);
	mParams->getParameterHandle("assetName", handle);
	mParams->setParamString(handle, mAsset != NULL ? mAsset->getName() : "");
}

void DestructibleAssetCollision::resize(physx::PxU32 hullCount)
{
	for (physx::PxU32 i = 0; i < mConvexMeshContainer.size(); ++i)
	{
		physx::Array< NxConvexMesh* >& convexMeshSet = mConvexMeshContainer[i];
		for (physx::PxU32 j = hullCount; j < convexMeshSet.size(); ++j)
		{
			if (convexMeshSet[j] != NULL)
			{
#if NX_SDK_VERSION_MAJOR == 2
				NxGetApexSDK()->getPhysXSDK()->releaseConvexMesh(*convexMeshSet[j]);
#elif NX_SDK_VERSION_MAJOR == 3
				convexMeshSet[j]->release();
#endif
			}
		}
		mConvexMeshContainer[i].resize(hullCount);
	}

	NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();
	for (int i = 0; i < mParams->meshCookedCollisionStreamsAtScale.arraySizes[0]; ++i)
	{
		MeshCookedCollisionStreamsAtScale* streamsAtScale = DYNAMIC_CAST(MeshCookedCollisionStreamsAtScale*)(mParams->meshCookedCollisionStreamsAtScale.buf[i]);
		if (streamsAtScale == NULL)
		{
			streamsAtScale = DYNAMIC_CAST(MeshCookedCollisionStreamsAtScale*)(traits->createNxParameterized(MeshCookedCollisionStreamsAtScale::staticClassName()));
			mParams->meshCookedCollisionStreamsAtScale.buf[i] = streamsAtScale;
		}
		NxParameterized::Handle handle(*streamsAtScale);
		streamsAtScale->getParameterHandle("meshCookedCollisionStreams", handle);

		// resizing NxParam ref arrays doesn't call destroy(), do this here
		physx::PxI32 currentArraySize = 0;
		streamsAtScale->getArraySize(handle, currentArraySize);
		for (int j = currentArraySize - 1; j >= (physx::PxI32)hullCount; j--)
		{
			NxParameterized::Interface*& hullStream = streamsAtScale->meshCookedCollisionStreams.buf[j];
			if (hullStream != NULL)
			{
				hullStream->destroy();
			}
			hullStream = NULL;
		}
		streamsAtScale->resizeArray(handle, (physx::PxI32)hullCount);
	}
}

bool DestructibleAssetCollision::addScale(const physx::PxVec3& scale)
{
	if (getScaleIndex(scale, kDefaultDestructibleAssetCollisionScaleTolerance) >= 0)
	{
		return false;	// Scale already exists
	}

	int scaleIndex = mParams->scales.arraySizes[0];

	NxParameterized::Handle handle(*mParams);
	mParams->getParameterHandle("scales", handle);
	mParams->resizeArray(handle, scaleIndex + 1);
	mParams->getParameterHandle("meshCookedCollisionStreamsAtScale", handle);
	mParams->resizeArray(handle, scaleIndex + 1);

	mConvexMeshContainer.resize((physx::PxU32)scaleIndex + 1);

	mParams->scales.buf[(physx::PxU32)scaleIndex] = scale;
	mParams->meshCookedCollisionStreamsAtScale.buf[(physx::PxU32)scaleIndex] = NULL;
	mConvexMeshContainer[(physx::PxU32)scaleIndex].reset();

	return true;
}

bool DestructibleAssetCollision::cookAll()
{
	bool result = true;
	for (int i = 0; i < mParams->scales.arraySizes[0]; ++i)
	{
		if (!cookScale(mParams->scales.buf[i]))
		{
			result = false;
		}
	}

	return result;
}

bool DestructibleAssetCollision::cookScale(const physx::PxVec3& scale)
{
	if (mAsset == NULL)
	{
		return false;
	}

	const physx::PxI32 partCount = mAsset->mParams->chunkConvexHullStartIndices.arraySizes[0];
	if (partCount <= 0)
	{
		return false;
	}

	const physx::PxU32 hullCount = mAsset->mParams->chunkConvexHullStartIndices.buf[partCount-1];

	bool result = true;
	for (physx::PxU16 i = 0; i < hullCount; ++i)
	{
		NxConvexMesh* convexMesh = getConvexMesh(i, scale);
		if (convexMesh == NULL)
		{
			result = false;
		}
	}

	return result;
}

NxConvexMesh* DestructibleAssetCollision::getConvexMesh(physx::PxU32 hullIndex, const physx::PxVec3& scale)
{
	physx::PxI32 scaleIndex = getScaleIndex(scale, kDefaultDestructibleAssetCollisionScaleTolerance);

	if (scaleIndex >= 0)
	{
		if (scaleIndex < (physx::PxI32)mConvexMeshContainer.size())
		{
			physx::Array<NxConvexMesh*>& convexMeshSet = mConvexMeshContainer[(physx::PxU32)scaleIndex];
			if (hullIndex < convexMeshSet.size())
			{
				NxConvexMesh* convexMesh = convexMeshSet[hullIndex];
				if (convexMesh != NULL)
				{
					return convexMesh;
				}
			}
		}
	}

	NxParameterized::Handle handle(*mParams);
	NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();

	MeshCookedCollisionStreamsAtScale* streamsAtScale;
	if (scaleIndex < 0)
	{
		scaleIndex = mParams->scales.arraySizes[0];

		mParams->getParameterHandle("scales", handle);
		mParams->resizeArray(handle, scaleIndex + 1);
		mParams->getParameterHandle("meshCookedCollisionStreamsAtScale", handle);
		mParams->resizeArray(handle, scaleIndex + 1);

		mConvexMeshContainer.resize((physx::PxU32)scaleIndex + 1);

		mParams->scales.buf[(physx::PxU32)scaleIndex] = scale;
	}

	if (mParams->meshCookedCollisionStreamsAtScale.buf[scaleIndex] != NULL)
	{
		streamsAtScale = DYNAMIC_CAST(MeshCookedCollisionStreamsAtScale*)(mParams->meshCookedCollisionStreamsAtScale.buf[scaleIndex]);
	}
	else
	{
		streamsAtScale = DYNAMIC_CAST(MeshCookedCollisionStreamsAtScale*)(traits->createNxParameterized(MeshCookedCollisionStreamsAtScale::staticClassName()));
		mParams->meshCookedCollisionStreamsAtScale.buf[scaleIndex] = streamsAtScale;
	}

	if (mAsset != NULL && (int)mAsset->chunkConvexHulls.size() != streamsAtScale->meshCookedCollisionStreams.arraySizes[0])
	{
		NxParameterized::Handle streamsHandle(*streamsAtScale);
		streamsAtScale->getParameterHandle("meshCookedCollisionStreams", streamsHandle);
		streamsHandle.resizeArray((physx::PxI32)mAsset->chunkConvexHulls.size());
		mConvexMeshContainer[(physx::PxU32)scaleIndex].resize(mAsset->chunkConvexHulls.size());
	}

	if ((int)hullIndex >= streamsAtScale->meshCookedCollisionStreams.arraySizes[0])
	{
		return NULL;
	}

	NxConvexMesh* convexMesh = NULL;

	MeshCookedCollisionStream* stream = DYNAMIC_CAST(MeshCookedCollisionStream*)(streamsAtScale->meshCookedCollisionStreams.buf[hullIndex]);
	if (stream == NULL)
	{
		if (mAsset == NULL)
		{
			return NULL;
		}

		stream = DYNAMIC_CAST(MeshCookedCollisionStream*)(traits->createNxParameterized(MeshCookedCollisionStream::staticClassName()));
		streamsAtScale->meshCookedCollisionStreams.buf[hullIndex] = stream;

		PX_PROFILER_PERF_SCOPE("DestructibleCookChunkCollisionMeshes");

		// Update the asset's stats with the number of cooked collision convex meshes
		if (mAsset != NULL)
		{
			++mAsset->mRuntimeCookedConvexCount;
		}

		const ConvexHull& hullData = mAsset->chunkConvexHulls[hullIndex];

		if (hullData.getVertexCount() == 0)
		{
			return NULL;
		}

		Array<PxVec3> scaledPoints;
		scaledPoints.resize(hullData.getVertexCount());
		PxVec3 centroid(0.0f);
		for (PxU32 i = 0; i < scaledPoints.size(); ++i)
		{
			scaledPoints[i] = hullData.getVertex(i);	// Cook at unit scale first
			centroid += scaledPoints[i];
		}
		centroid *= 1.0f/(physx::PxF32)scaledPoints.size();
		for (PxU32 i = 0; i < scaledPoints.size(); ++i)
		{
			scaledPoints[i] -= centroid;
		}

		PxMemoryBuffer memStream;
		memStream.setEndianMode(physx::PxFileBuf::ENDIAN_NONE);
		NxFromPxStream nxs(memStream);
#if NX_SDK_VERSION_MAJOR == 2
		NxConvexMeshDesc meshDesc;
		meshDesc.numVertices = scaledPoints.size();
		meshDesc.points = scaledPoints.begin();
		meshDesc.pointStrideBytes = sizeof(physx::PxVec3);
		meshDesc.flags = NX_CF_COMPUTE_CONVEX | NX_CF_USE_UNCOMPRESSED_NORMALS;
		const physx::PxF32 skinWidth = NxGetApexSDK()->getCookingInterface() != NULL ? NxGetApexSDK()->getCookingInterface()->NxGetCookingParams().skinWidth : 0.0f;
		if (skinWidth > 0.0f)
		{
			meshDesc.flags |= NX_CF_INFLATE_CONVEX;
		}
		bool success = NxGetApexSDK()->getCookingInterface()->NxCookConvexMesh(meshDesc, nxs);
#elif NX_SDK_VERSION_MAJOR == 3
		PxConvexMeshDesc meshDesc;
		meshDesc.points.count = scaledPoints.size();
		meshDesc.points.data = scaledPoints.begin();
		meshDesc.points.stride = sizeof(physx::PxVec3);
		meshDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;
		const physx::PxF32 skinWidth = NxGetApexSDK()->getCookingInterface() != NULL ? NxGetApexSDK()->getCookingInterface()->getParams().skinWidth : 0.0f;
		if (skinWidth > 0.0f)
		{
			meshDesc.flags |= PxConvexFlag::eINFLATE_CONVEX;
		}
		bool success = NxGetApexSDK()->getCookingInterface()->cookConvexMesh(meshDesc, nxs);
#endif

		// Now scale all the points, in case this array is used as-is (failure cases)
		for (PxU32 i = 0; i < scaledPoints.size(); ++i)
		{
			scaledPoints[i] += centroid;
			scaledPoints[i] = scaledPoints[i].multiply(scale);
		}

		if (success)
		{
			NxFromPxStream nxs(memStream);
			convexMesh = NxGetApexSDK()->getPhysXSDK()->createConvexMesh(nxs);

			// Examine the mass properties to make sure they're reasonable.
			if (convexMesh != NULL)
			{
				PxReal mass;
				PxMat33 localInertia;
#if NX_SDK_VERSION_MAJOR == 2
				NxMat33 nlocalInertia;
				NxVec3 localCenterOfMass;
				convexMesh->getMassInformation(mass, nlocalInertia, localCenterOfMass);
				physx::PxF32 buf[9];
				nlocalInertia.getRowMajor(buf);
				localInertia = PxMat33(buf);
#else
				PxVec3 localCenterOfMass;
				convexMesh->getMassInformation(mass, localInertia, localCenterOfMass);
#endif
				PxMat33 massFrame;
				const PxVec3 massLocalInertia = diagonalizeSymmetric(massFrame, localInertia);
				success = (mass > 0 && massLocalInertia.x > 0 && massLocalInertia.y > 0 && massLocalInertia.z > 0);
				if (success && massLocalInertia.maxElement() > 4000*massLocalInertia.minElement())
				{
#if NX_SDK_VERSION_MAJOR == 2
					NxGetApexSDK()->getPhysXSDK()->releaseConvexMesh(*convexMesh);
#else
					convexMesh->release();
#endif
					convexMesh = NULL;
					success = false;
				}
			}
			else
			{
				success = false;
			}

			if (success)
			{
				// Now scale the convex hull
				memStream.reset();
				memStream.setEndianMode(physx::PxFileBuf::ENDIAN_NONE);
				NxFromPxStream nxs(memStream);
#if NX_SDK_VERSION_MAJOR == 2
				PX_ASSERT(convexMesh->getStride(0, NX_ARRAY_VERTICES) == sizeof(physx::PxVec3));

				const physx::PxU32 numVerts = convexMesh->getCount(0, NX_ARRAY_VERTICES);
				const physx::PxVec3* verts = (const physx::PxVec3*)convexMesh->getBase(0, NX_ARRAY_VERTICES);

				scaledPoints.resize(numVerts);
				for (PxU32 i = 0; i < numVerts; ++i)
				{
					scaledPoints[i] = (verts[i] + centroid).multiply(scale);
				}

				NxConvexMeshDesc meshDesc;
				meshDesc.numVertices = numVerts;
				meshDesc.points = scaledPoints.begin();
				meshDesc.pointStrideBytes = sizeof(physx::PxVec3);
				meshDesc.numTriangles = convexMesh->getCount(0, NX_ARRAY_TRIANGLES);
				meshDesc.triangles = convexMesh->getBase(0, NX_ARRAY_TRIANGLES);
				meshDesc.triangleStrideBytes = convexMesh->getStride(0, NX_ARRAY_TRIANGLES);
				meshDesc.flags = NX_CF_USE_UNCOMPRESSED_NORMALS;
				success = NxGetApexSDK()->getCookingInterface()->NxCookConvexMesh(meshDesc, nxs);

				NxGetApexSDK()->getPhysXSDK()->releaseConvexMesh(*convexMesh);
#elif NX_SDK_VERSION_MAJOR == 3
				const physx::PxU32 numVerts = convexMesh->getNbVertices();
				const physx::PxVec3* verts = convexMesh->getVertices();

				scaledPoints.resize(numVerts);
				for (PxU32 i = 0; i < numVerts; ++i)
				{
					scaledPoints[i] = (verts[i] + centroid).multiply(scale);
				}

				// Unfortunately, we must build our own triangle buffer from the polygon buffer
				PxU32 triangleCount = 0;
				for (PxU32 i = 0; i < convexMesh->getNbPolygons(); ++i)
				{
					PxHullPolygon polygon;
					convexMesh->getPolygonData(i, polygon);
					triangleCount += polygon.mNbVerts - 2;
				}
				const physx::PxU8* indexBuffer = (const physx::PxU8*)convexMesh->getIndexBuffer();
				Array<PxU32> indices;
				indices.reserve(triangleCount*3);
				for (PxU32 i = 0; i < convexMesh->getNbPolygons(); ++i)
				{
					PxHullPolygon polygon;
					convexMesh->getPolygonData(i, polygon);
					for (PxU16 j = 1; j < polygon.mNbVerts-1; ++j)
					{
						indices.pushBack((physx::PxU32)indexBuffer[polygon.mIndexBase]);
						indices.pushBack((physx::PxU32)indexBuffer[polygon.mIndexBase+j]);
						indices.pushBack((physx::PxU32)indexBuffer[polygon.mIndexBase+j+1]);
					}
				}

				PxConvexMeshDesc meshDesc;
				meshDesc.points.count = scaledPoints.size();
				meshDesc.points.data = scaledPoints.begin();
				meshDesc.points.stride = sizeof(physx::PxVec3);
				meshDesc.triangles.count = triangleCount;
				meshDesc.triangles.data = indices.begin();
				meshDesc.triangles.stride = 3*sizeof(PxU32);
				success = NxGetApexSDK()->getCookingInterface()->cookConvexMesh(meshDesc, nxs);

				convexMesh->release();
#endif
				convexMesh = NULL;

				if (success)
				{
					convexMesh = NxGetApexSDK()->getPhysXSDK()->createConvexMesh(nxs);
				}

				if (convexMesh == NULL)
				{
					success = false;
				}
			}
		}

		if (!success)
		{
			convexMesh = NULL;
			memStream.reset();
			memStream.setEndianMode(physx::PxFileBuf::ENDIAN_NONE);
			NxFromPxStream nxs(memStream);
			// Just form bbox
			physx::PxBounds3 bounds;
			bounds.setEmpty();
			for (physx::PxU32 i = 0; i < scaledPoints.size(); ++i)
			{
				bounds.include(scaledPoints[i]);
			}
			PX_ASSERT(!bounds.isEmpty());
			bounds.fattenFast(PxMax(0.00001f, bounds.getExtents().magnitude()*0.001f));
			scaledPoints.resize(8);
			for (physx::PxU32 i = 0; i < 8; ++i)
			{
				scaledPoints[i] = physx::PxVec3((i & 1) ? bounds.maximum.x : bounds.minimum.x,
				                                (i & 2) ? bounds.maximum.y : bounds.minimum.y,
				                                (i & 4) ? bounds.maximum.z : bounds.minimum.z);
			}
#if NX_SDK_VERSION_MAJOR == 2
			meshDesc.points = scaledPoints.begin();
			meshDesc.numVertices = 8;
			if (!NxGetApexSDK()->getCookingInterface()->NxCookConvexMesh(meshDesc, nxs))
#elif NX_SDK_VERSION_MAJOR == 3
			meshDesc.points.data = scaledPoints.begin();
			meshDesc.points.count = 8;
			if (!NxGetApexSDK()->getCookingInterface()->cookConvexMesh(meshDesc, nxs))
#endif
			{
				memStream.reset();
			}
		}

		{
			NxParameterized::Handle bytesHandle(stream);
			stream->getParameterHandle("bytes", bytesHandle);
			stream->resizeArray(bytesHandle, (physx::PxI32)memStream.getWriteBufferSize());
			stream->setParamU8Array(bytesHandle, memStream.getWriteBuffer(), (physx::PxI32)memStream.getWriteBufferSize());
		}
	}

	if (convexMesh == NULL)
	{
		PxMemoryBuffer memStream(stream->bytes.buf, (physx::PxU32)stream->bytes.arraySizes[0]);
		memStream.setEndianMode(physx::PxFileBuf::ENDIAN_NONE);
		NxFromPxStream nxs(memStream);
		convexMesh = NxGetApexSDK()->getPhysXSDK()->createConvexMesh(nxs);
	}

	// These resizes should not be required, fix it and remove them
	if (mConvexMeshContainer.size() <= (physx::PxU32)scaleIndex)
	{
		APEX_DEBUG_WARNING("The asset's (%s) convex mesh container needed resizing, debug this", mAsset->getName());
		mConvexMeshContainer.resize((physx::PxU32)scaleIndex+1);
	}
	if (mConvexMeshContainer[(physx::PxU32)scaleIndex].size() <= (physx::PxU32)hullIndex)
	{
		APEX_DEBUG_WARNING("The asset's (%s) convex mesh container at scale index %d needed resizing, debug this", mAsset->getName(), scaleIndex);
		mConvexMeshContainer[(physx::PxU32)scaleIndex].resize(hullIndex+1);
	}
	
	mConvexMeshContainer[(physx::PxU32)scaleIndex][hullIndex] = convexMesh;

	return convexMesh;
}

MeshCookedCollisionStreamsAtScale* DestructibleAssetCollision::getCollisionAtScale(const physx::PxVec3& scale)
{
	cookScale(scale);

	const physx::PxI32 scaleIndex = getScaleIndex(scale, kDefaultDestructibleAssetCollisionScaleTolerance);
	if (scaleIndex < 0)
	{
		return NULL;
	}

	return DYNAMIC_CAST(MeshCookedCollisionStreamsAtScale*)(mParams->meshCookedCollisionStreamsAtScale.buf[scaleIndex]);
}

physx::Array<NxConvexMesh*>* DestructibleAssetCollision::getConvexMeshesAtScale(const physx::PxVec3& scale)
{
	cookScale(scale);

	const physx::PxI32 scaleIndex = getScaleIndex(scale, kDefaultDestructibleAssetCollisionScaleTolerance);
	if (scaleIndex < 0)
	{
		return NULL;
	}

	return &mConvexMeshContainer[(physx::PxU32)scaleIndex];
}

physx::PxFileBuf& DestructibleAssetCollision::deserialize(physx::PxFileBuf& stream, const char* assetName)
{
	// If there are any referenced meshes in ANY scales we're going to revoke this operation as not supported
	for (physx::PxU32 i=0; i<mConvexMeshContainer.size(); i++)
	{
		if (mConvexMeshContainer.getReferenceCount(i) > 0)
		{
			APEX_DEBUG_INFO("Cannot deserialize the cooked collision data cache for asset <%s> scaleIdx <%d> because it is in use by actors", getAssetName(), i);
			return stream;
		}
	}

	mAsset = NULL;
	mConvexMeshContainer.reset();

	/*physx::PxU32 version =*/
	stream.readDword();	// Eat version #, not used since this is the initial version

	ApexSimpleString name;
	name.deserialize(stream);
	NxParameterized::Handle handle(*mParams);
	mParams->getParameterHandle("assetName", handle);
	mParams->setParamString(handle, name.c_str());

	if (assetName != NULL)
		mParams->setParamString(handle, assetName);

	stream >> mParams->cookingPlatform;
	stream >> mParams->cookingVersionNum;

	NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();

	int scaleCount = (int)stream.readDword();
	mParams->getParameterHandle("scales", handle);
	mParams->resizeArray(handle, scaleCount);
	for (int i = 0; i < scaleCount; ++i)
	{
		stream >> mParams->scales.buf[i];
	}

	int meshScaleCount = (int)stream.readDword();
	mParams->getParameterHandle("meshCookedCollisionStreamsAtScale", handle);
	mParams->resizeArray(handle, meshScaleCount);
	mConvexMeshContainer.resize((physx::PxU32)meshScaleCount);
	for (physx::PxU32 i = 0; i < (physx::PxU32)meshScaleCount; ++i)
	{
		MeshCookedCollisionStreamsAtScale* streamsAtScale = DYNAMIC_CAST(MeshCookedCollisionStreamsAtScale*)(mParams->meshCookedCollisionStreamsAtScale.buf[i]);
		if (streamsAtScale == NULL)
		{
			streamsAtScale = DYNAMIC_CAST(MeshCookedCollisionStreamsAtScale*)(traits->createNxParameterized(MeshCookedCollisionStreamsAtScale::staticClassName()));
			mParams->meshCookedCollisionStreamsAtScale.buf[i] = streamsAtScale;
		}

		handle.setInterface(streamsAtScale);
		physx::Array<NxConvexMesh*>& meshSet = mConvexMeshContainer[i];

		physx::PxU32 meshCount = stream.readDword();
		streamsAtScale->getParameterHandle("meshCookedCollisionStreams", handle);
		streamsAtScale->resizeArray(handle, (physx::PxI32)meshCount);
		meshSet.resize(meshCount);
		for (physx::PxU32 j = 0; j < meshCount; ++j)
		{
			MeshCookedCollisionStream* collisionStream = DYNAMIC_CAST(MeshCookedCollisionStream*)(streamsAtScale->meshCookedCollisionStreams.buf[j]);
			if (collisionStream == NULL)
			{
				collisionStream = DYNAMIC_CAST(MeshCookedCollisionStream*)(traits->createNxParameterized(MeshCookedCollisionStream::staticClassName()));
				streamsAtScale->meshCookedCollisionStreams.buf[j] = collisionStream;
			}

			handle.setInterface(collisionStream);

			int bufferSize = (int)stream.readDword();
			collisionStream->getParameterHandle("bytes", handle);
			collisionStream->resizeArray(handle, bufferSize);
			stream.read(collisionStream->bytes.buf, (physx::PxU32)bufferSize);

			PxMemoryBuffer memStream(collisionStream->bytes.buf, (physx::PxU32)collisionStream->bytes.arraySizes[0]);
			memStream.setEndianMode(physx::PxFileBuf::ENDIAN_NONE);
			NxFromPxStream nxs(memStream);
			meshSet[j] = NxGetApexSDK()->getPhysXSDK()->createConvexMesh(nxs);
		}
	}
	return stream;
}

physx::PxFileBuf& DestructibleAssetCollision::serialize(physx::PxFileBuf& stream) const
{
#ifndef WITHOUT_APEX_AUTHORING
	stream << (physx::PxU32)Version::Current;

	ApexSimpleString name(mParams->assetName);
	name.serialize(stream);

	stream << mParams->cookingPlatform;
	stream << mParams->cookingVersionNum;

	stream.storeDword((physx::PxU32)mParams->scales.arraySizes[0]);
	for (physx::PxU32 i = 0; i < (physx::PxU32)mParams->scales.arraySizes[0]; ++i)
	{
		stream << mParams->scales.buf[i];
	}

	stream.storeDword((physx::PxU32)mParams->meshCookedCollisionStreamsAtScale.arraySizes[0]);
	for (physx::PxU32 i = 0; i < (physx::PxU32)mParams->meshCookedCollisionStreamsAtScale.arraySizes[0]; ++i)
	{
		MeshCookedCollisionStreamsAtScale* streamsAtScale = DYNAMIC_CAST(MeshCookedCollisionStreamsAtScale*)(mParams->meshCookedCollisionStreamsAtScale.buf[i]);
		if (streamsAtScale == NULL)
		{
			stream.storeDword(0);
		}
		else
		{
			stream.storeDword((physx::PxU32)streamsAtScale->meshCookedCollisionStreams.arraySizes[0]);
			for (physx::PxU32 j = 0; j < (physx::PxU32)streamsAtScale->meshCookedCollisionStreams.arraySizes[0]; ++j)
			{
				MeshCookedCollisionStream* collisionStream = DYNAMIC_CAST(MeshCookedCollisionStream*)(streamsAtScale->meshCookedCollisionStreams.buf[j]);
				if (collisionStream == NULL)
				{
					stream.storeDword(0);
				}
				else
				{
					stream.storeDword((physx::PxU32)collisionStream->bytes.arraySizes[0]);
					stream.write(collisionStream->bytes.buf, (physx::PxU32)collisionStream->bytes.arraySizes[0]);
				}
			}
		}
	}
#endif // #ifndef WITHOUT_APEX_AUTHORING
	return stream;
}

bool DestructibleAssetCollision::platformAndVersionMatch() const
{
#if NX_SDK_VERSION_MAJOR == 2
	const NxCookingParams& cookingParams = NiGetApexSDK()->getCookingInterface()->NxGetCookingParams();
#elif NX_SDK_VERSION_MAJOR == 3
	const PxCookingParams& cookingParams = NiGetApexSDK()->getCookingInterface()->getParams();
#endif
	const physx::PxU32 presentCookingVersionNum = NiGetApexSDK()->getCookingVersion();

	return ((physx::PxU32) cookingParams.targetPlatform == mParams->cookingPlatform) &&
	       ((presentCookingVersionNum & 0xFFFF0000) == (mParams->cookingVersionNum & 0xFFFF0000));
}

void DestructibleAssetCollision::setPlatformAndVersion()
{
#if NX_SDK_VERSION_MAJOR == 2
	mParams->cookingPlatform = NiGetApexSDK()->getCookingInterface()->NxGetCookingParams().targetPlatform;
#elif NX_SDK_VERSION_MAJOR == 3
	mParams->cookingPlatform = NiGetApexSDK()->getCookingInterface()->getParams().targetPlatform;
#endif
	mParams->cookingVersionNum = NiGetApexSDK()->getCookingVersion();
}

physx::PxU32 DestructibleAssetCollision::memorySize() const
{
	physx::PxU32 size = 0;

	for (int i = 0; i < mParams->meshCookedCollisionStreamsAtScale.arraySizes[0]; ++i)
	{
		MeshCookedCollisionStreamsAtScale* streamsAtScale = DYNAMIC_CAST(MeshCookedCollisionStreamsAtScale*)(mParams->meshCookedCollisionStreamsAtScale.buf[i]);
		if (streamsAtScale == NULL)
		{
			continue;
		}
		for (int j = 0; j < streamsAtScale->meshCookedCollisionStreams.arraySizes[0]; ++j)
		{
			MeshCookedCollisionStream* stream = DYNAMIC_CAST(MeshCookedCollisionStream*)(streamsAtScale->meshCookedCollisionStreams.buf[j]);
			if (stream == NULL)
			{
				continue;
			}
			size += (physx::PxU32)stream->bytes.arraySizes[0];
		}
	}

	return size;
}

void DestructibleAssetCollision::clearUnreferencedSets()
{
	for (physx::PxU32 i = 0; i < mConvexMeshContainer.size(); ++i)
	{
		if (mConvexMeshContainer.getReferenceCount(i) == 0)
		{
			MeshCookedCollisionStreamsAtScale* streamsAtScale = DYNAMIC_CAST(MeshCookedCollisionStreamsAtScale*)(mParams->meshCookedCollisionStreamsAtScale.buf[i]);
			if (streamsAtScale)
			{
				NxParameterized::Handle handle(*streamsAtScale);
				streamsAtScale->getParameterHandle("meshCookedCollisionStreams", handle);
				streamsAtScale->resizeArray(handle, 0);
			}

#if NX_SDK_VERSION_MAJOR != 2
				// We need to NULL this pointer, otherwise we'll be accessing old data as a result of the reset below
				DestructibleAsset* asset = mAsset;
				if (asset)
				{
					asset->mCollisionMeshes = NULL;
				}
#endif
		}
	}
	mConvexMeshContainer.reset(false);
}

// Spit out warnings to the error stream for any referenced sets
void DestructibleAssetCollision::reportReferencedSets()
{
	for (physx::PxU32 i = 0; i < mConvexMeshContainer.size(); ++i)
	{
		if (mConvexMeshContainer.getReferenceCount(i))
		{
			APEX_DEBUG_WARNING("Clearing a referenced convex mesh container for asset: %s", mAsset);
		}
	}
}

bool DestructibleAssetCollision::incReferenceCount(int scaleIndex)
{
	if (scaleIndex < 0 || scaleIndex >= (int)mConvexMeshContainer.size())
	{
		return false;
	}

	mConvexMeshContainer.incReferenceCount((physx::PxU32)scaleIndex);

	return true;
}

bool DestructibleAssetCollision::decReferenceCount(int scaleIndex)
{
	if (scaleIndex < 0 || scaleIndex >= (int)mConvexMeshContainer.size())
	{
		return false;
	}

	return mConvexMeshContainer.decReferenceCount((physx::PxU32)scaleIndex);
}

// The source 'collisionSet' is not const because it's list of NxConvexMesh pointers
// in 'mConvexMeshContainer' needs to be cleared so they aren't released in 
// DestructibleAssetCollision::resize(0)
void DestructibleAssetCollision::merge(DestructibleAssetCollision& collisionSet)
{
	NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();

	// Prepare the convexMesh container for the collisionSet's meshes
	if (mConvexMeshContainer.size() < collisionSet.mConvexMeshContainer.size())
	{
		mConvexMeshContainer.resize(collisionSet.mConvexMeshContainer.size());
	}

	// Loop through scales contained in collisionSet
	for (physx::PxU32 i = 0; i < (physx::PxU32)collisionSet.mParams->scales.arraySizes[0]; ++i)
	{
		const physx::PxVec3& scale = collisionSet.mParams->scales.buf[i];
		int scaleIndex = getScaleIndex(scale, kDefaultDestructibleAssetCollisionScaleTolerance);
		if (scaleIndex < 0)
		{
			// Scale not found, add it to this set
			addScale(scale);
			scaleIndex = getScaleIndex(scale, kDefaultDestructibleAssetCollisionScaleTolerance);
			if (scaleIndex < 0)
			{
				continue;	// Failed to add scale
			}
			if (mParams->meshCookedCollisionStreamsAtScale.buf[scaleIndex] == NULL)	// Create streams if we need them
			{
				mParams->meshCookedCollisionStreamsAtScale.buf[scaleIndex] = DYNAMIC_CAST(MeshCookedCollisionStreamsAtScale*)(traits->createNxParameterized(MeshCookedCollisionStreamsAtScale::staticClassName()));
			}
			mParams->meshCookedCollisionStreamsAtScale.buf[scaleIndex]->copy(*collisionSet.mParams->meshCookedCollisionStreamsAtScale.buf[i]);
		}
		else
		{
			MeshCookedCollisionStreamsAtScale* streamsAtScale = DYNAMIC_CAST(MeshCookedCollisionStreamsAtScale*)(mParams->meshCookedCollisionStreamsAtScale.buf[scaleIndex]);
			PX_ASSERT(streamsAtScale != NULL);
			if (streamsAtScale->meshCookedCollisionStreams.arraySizes[0] == 0)	// Only merge if this scale is empty; we won't stomp any existing data
			{
				mParams->meshCookedCollisionStreamsAtScale.buf[scaleIndex]->copy(*collisionSet.mParams->meshCookedCollisionStreamsAtScale.buf[i]);
			}

			// also copy the NxConvexMesh pointers (because the source collisionSet has them already at this point)
			physx::Array<NxConvexMesh*>& srcConvexMeshSet = collisionSet.mConvexMeshContainer[i];

			// make sure the destination list is present
			bool convexMeshSetResized = false;
			if (mConvexMeshContainer[i].size() < srcConvexMeshSet.size())
			{
				convexMeshSetResized = true;
				mConvexMeshContainer[i].resize(srcConvexMeshSet.size());
			}

			for (physx::PxU32 j=0; j<srcConvexMeshSet.size(); j++)
			{
				// Only merge if we need NxConvexMesh pointers, otherwise just release these 
				// newly created NxConvexMesh after exiting this method
				if(mConvexMeshContainer[i][j] == NULL || convexMeshSetResized)
				{
					mConvexMeshContainer[i][j] = srcConvexMeshSet[j];
					// This prevents DestructibleAssetCollision::resize() from releasing the convex mesh
					srcConvexMeshSet[j] = NULL;
				}
			}
		}
	}
}

physx::PxI32 DestructibleAssetCollision::getScaleIndex(const physx::PxVec3& scale, physx::PxF32 tolerance) const
{
	for (int i = 0; i < mParams->scales.arraySizes[0]; ++i)
	{
		const physx::PxVec3& error = scale - mParams->scales.buf[i];
		if (physx::PxAbs(error.x) <= tolerance && physx::PxAbs(error.y) <= tolerance && physx::PxAbs(error.z) <= tolerance)
		{
			return i;
		}
	}

	return -1;
}

/** DestructibleAsset::ScatterMeshInstanceInfo **/

DestructibleAsset::ScatterMeshInstanceInfo::~ScatterMeshInstanceInfo()
{
	if (m_actor != NULL)
	{
		m_actor->release();
		m_actor = NULL;
	}

	if (m_instanceBuffer != NULL)
	{
		NxUserRenderResourceManager* rrm = NiGetApexSDK()->getUserRenderResourceManager();
		rrm->releaseInstanceBuffer(*m_instanceBuffer);
		m_instanceBuffer = NULL;
	}
}

}
}
} // end namespace physx::apex

#endif
