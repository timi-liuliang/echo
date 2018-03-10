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
#include "ModuleDestructible.h"
#include "DestructibleActor.h"
#include "DestructibleScene.h"
#include "NiApexScene.h"
#include "DestructibleStructure.h"
#include "DestructibleActorProxy.h"
#include "ModulePerfScope.h"

#if APEX_USE_PARTICLES
#include "NxApexEmitterAsset.h"
#include "NxApexEmitterActor.h"
#endif // APEX_USE_PARTICLES

#if NX_SDK_VERSION_MAJOR == 2
#include <NxUserRaycastReport.h>
#include <NxScene.h>
#include <NxBoxShapeDesc.h>
#include <NxConvexMesh.h>
#elif NX_SDK_VERSION_MAJOR == 3
#include "foundation/PxMath.h"
#include <PxMaterial.h>
#endif

#include "foundation/PxString.h"

#include "NiApexRenderMeshAsset.h"

#include "DestructibleActorUtils.h"

namespace physx
{
namespace apex
{
namespace destructible
{

///////////////////////////////////////////////////////////////////////////
#if NX_SDK_VERSION_MAJOR == 3
namespace
{
bool isNxActorSleeping(const physx::NxActor & nxActor)
{
	PX_ASSERT(NULL != &nxActor);
	SCOPED_PHYSX3_LOCK_READ(nxActor.getScene());
	return nxActor.isRigidDynamic()->isSleeping();
}
} // namespace nameless
#endif // NX_SDK_VERSION_MAJOR == 3

template<class type>
PX_INLINE void combsort(type* a, physx::PxU32 num)
{
	physx::PxU32 gap = num;
	bool swapped = false;
	do
	{
		swapped = false;
		gap = (gap * 10) / 13;
		if (gap == 9 || gap == 10)
		{
			gap = 11;
		}
		else if (gap < 1)
		{
			gap = 1;
		}
		for (type* ai = a, *aend = a + (num - gap); ai < aend; ai++)
		{
			type* aj = ai + gap;
			if (*ai > *aj)
			{
				swapped = true;
				physx::swap(*ai, *aj);
			}
		}
	}
	while (gap > 1 || swapped);
}

static NxParameterized::Interface* createDefaultState(NxParameterized::Interface* params = NULL)
{
	NxParameterized::Interface* state = NiGetApexSDK()->getParameterizedTraits()->createNxParameterized(DestructibleActorState::staticClassName());
	PX_ASSERT(NULL != state);
	if(state)
	{
		NxParameterized::Handle handle(*state);
		VERIFY_PARAM(state->getParameterHandle("actorParameters", handle));
		if (params)
			handle.setParamRef(params);
		else
			handle.initParamRef(DestructibleActorParam::staticClassName());

		VERIFY_PARAM(state->getParameterHandle("actorChunks", handle));
		handle.initParamRef(DestructibleActorChunks::staticClassName());
	}
	return state;
}

DestructibleActor::DestructibleActor(NxDestructibleActor* _api, DestructibleAsset& _asset, DestructibleScene& scene ) :
	mState(NULL),
	mParams(NULL),
	mChunks(NULL),
	mTM(physx::PxMat44::createIdentity()),
	mRelTM(physx::PxMat44::createIdentity()),
	mVisibleDynamicChunkShapeCount(0),
	mEssentialVisibleDynamicChunkShapeCount(0),
	mDestructibleScene(&scene),
	mAPI(_api),
	mFlags(0),
	mInternalFlags(0),
	mStructure(NULL),
	mAsset(&_asset),
	mID((physx::PxU32)InvalidID),
	mLinearSize(0.0f),
	mCrumbleEmitter(NULL),
	mDustEmitter(NULL),
	mCrumbleRenderVolume(NULL),
	mDustRenderVolume(NULL),
	mStartTime(scene.mElapsedTime),
	mBenefit(0.0f),
	mInitializedFromState(false),
	mRenderable(NULL),
	mDescOverrideSkinnedMaterialCount(0),
	mDescOverrideSkinnedMaterials(NULL),
	mDescOverrideStaticMaterialCount(0),
	mPhysXActorBufferAcquired(false),
	mInDeleteChunkMode(false),
	mAwakeActorCount(0),
	mActiveFrames(0),
	mDamageEventReportIndex(0xFFFFFFFF)
	#if APEX_RUNTIME_FRACTURE
	//,mRTActor((PX_NEW(DestructibleRTActor)(*this)))
	,mRTActor(NULL)
	#endif
#if USE_DESTRUCTIBLE_RWLOCK
	, mLock(NULL)
#endif
	, mWakeForEvent(false)
#if NX_SDK_VERSION_MAJOR == 2	// In 3.x this is put into the asset
	, mCollisionMeshes(NULL)
#endif
{
	// The client (DestructibleActorProxy) is responsible for calling initialize()
}

void DestructibleActor::initialize(NxParameterized::Interface* p)
{
	// Given actor state
	if (NULL != p     && isType(p, NxDestructibleParameterizedType::State))
	{
		initializeFromState(p);
		initializeCommon();
	}
	// Given actor params
	else if(NULL != p && isType(p, NxDestructibleParameterizedType::Params))
	{
		initializeFromParams(p);
		initializeCommon();
	}
	else
	{
		PX_ALWAYS_ASSERT();
		APEX_INTERNAL_ERROR("Invalid destructible actor creation arguments.");
	}
}

void DestructibleActor::initializeFromState(NxParameterized::Interface* state)
{
	setState(state);
	mInitializedFromState = true;
}

void DestructibleActor::initializeFromParams(NxParameterized::Interface* params)
{
	setState(createDefaultState(params));
	mInitializedFromState                = false;
	mState->enableCrumbleEmitter = APEX_USE_PARTICLES ? true : false;
	mState->enableDustEmitter    = APEX_USE_PARTICLES ? true : false;
	mState->lod                  = physx::PxMax(mAsset->getDepthCount(), (physx::PxU32)1) - 1;
	// To do: use a parameterized descriptor to get these
	mState->internalLODWeights.maxDistance    = 2000.0f;
	mState->internalLODWeights.distanceWeight = 0.5f;
	mState->internalLODWeights.maxAge         = 10.0f;
	mState->internalLODWeights.ageWeight      = 0.5f;
	mState->internalLODWeights.bias           = 0.0f;
}

void DestructibleActor::initializeCommon()
{
	setInitialGlobalPose(getParams()->globalPose);

	mFlags = (physx::PxU16)(getParams()->dynamic ? Dynamic : 0);

	mDescOverrideSkinnedMaterialCount = (physx::PxU32)getParams()->overrideSkinnedMaterialNames.arraySizes[0];
	mDescOverrideStaticMaterialCount  = (physx::PxU32)getParams()->overrideStaticMaterialNames.arraySizes[0];
	mDescOverrideSkinnedMaterials	= (const char**)PX_ALLOC(sizeof(const char*)*(mDescOverrideSkinnedMaterialCount > 0 ? mDescOverrideSkinnedMaterialCount : 1), PX_DEBUG_EXP("DestructibleActor::initializeCommon_mDescOverrideSkinnedMaterials"));
	for (physx::PxU32 i = 0; i < mDescOverrideSkinnedMaterialCount; ++i)
	{
		mDescOverrideSkinnedMaterials[i] = getParams()->overrideSkinnedMaterialNames.buf[i].buf;
	}
	PX_ALLOCA(staticMaterialNames, const char*, mDescOverrideStaticMaterialCount > 0 ? mDescOverrideStaticMaterialCount : 1);
	for (physx::PxU32 i = 0; i < mDescOverrideStaticMaterialCount; ++i)
	{
		staticMaterialNames[i] = getParams()->overrideStaticMaterialNames.buf[i].buf;
	}

	DestructibleActorParam* p = getParams();
	DestructibleActorParamNS::ParametersStruct* ps = static_cast<DestructibleActorParamNS::ParametersStruct*>(p);

#if NX_SDK_VERSION_MAJOR == 2
	NxActorDesc adesc;
	deserialize(adesc, ps->actorDescTemplate);

	NxBodyDesc bdesc;
	deserialize(bdesc, ps->bodyDescTemplate);

	NxBoxShapeDesc sdesc;
	deserialize(sdesc, ps->shapeDescTemplate);

	adesc.body = &bdesc;
	adesc.shapes.pushBack(&sdesc);

	setActorTemplate(&adesc);
	setShapeTemplate(&sdesc);
	setBodyTemplate(&bdesc);

#elif NX_SDK_VERSION_MAJOR == 3
	PhysX3DescTemplate p3Desc;
	deserialize(p3Desc, ps->p3ActorDescTemplate, ps->p3BodyDescTemplate, ps->p3ShapeDescTemplate);
	setPhysX3Template(&p3Desc);
#endif

#if USE_DESTRUCTIBLE_RWLOCK
	if (NULL == mLock)
	{
		mLock = (physx::ReadWriteLock*)PX_ALLOC(sizeof(physx::ReadWriteLock), PX_DEBUG_EXP("DestructibleActor::RWLock"));
		PX_PLACEMENT_NEW(mLock, physx::ReadWriteLock);
	}
#endif

	physx::PxVec3 extents = mAsset->getBounds().getExtents();
	mLinearSize = physx::PxMax(physx::PxMax(extents.x*getScale().x, extents.y*getScale().y), extents.z*getScale().z);

	setDestructibleParameters(getParams()->destructibleParameters, getParams()->depthParameters);

	initializeActor();

	if ((getDestructibleParameters().flags & physx::apex::NxDestructibleParametersFlag::CRUMBLE_VIA_RUNTIME_FRACTURE) != 0)
	{
		initializeRTActor();
	}
}

physx::PxF32 DestructibleActor::getCrumbleParticleSpacing() const
{
	if (mParams->crumbleParticleSpacing > 0.0f)
	{
		return mParams->crumbleParticleSpacing;
	}
#if APEX_USE_PARTICLES
	if (getCrumbleEmitter())
	{
		return 2 * getCrumbleEmitter()->getObjectRadius();
	}
#endif // APEX_USE_PARTICLES
	return 0.0f;
}

physx::PxF32 DestructibleActor::getDustParticleSpacing() const
{
	if (mParams->dustParticleSpacing > 0.0f)
	{
		return mParams->dustParticleSpacing;
	}
#if APEX_USE
	if (getDustEmitter())
	{
		return 2 * getDustEmitter()->getObjectRadius();
	}
#endif // APEX_USE_PARTICLES
	return 0.0f;
}

void DestructibleActor::setInitialGlobalPose(const physx::PxMat44& pose)
{
	mTM = pose;
	mOriginalBounds = mAsset->getBounds();
	physx::PxMat34Legacy scaledTM = mTM;
	scaledTM.M.multiplyDiagonal(getScale());
	PxBounds3Transform(mOriginalBounds, scaledTM.M, scaledTM.t);
}

void DestructibleActor::setState(NxParameterized::Interface* state)
{
	if (mState != state)
	{
		// wrong name?
		if (NULL != state && !isType(state, NxDestructibleParameterizedType::State))
		{
			APEX_INTERNAL_ERROR(
				"The parameterized interface is of type <%s> instead of <%s>.  "
				"This object will be initialized by an empty one instead!",
				state->className(),
				DestructibleActorState::staticClassName());

			state->destroy();
			state = NULL;
		}
		else if (NULL != state)
		{
			//TODO: Verify parameters
		}

		// If no state was given, create the default state
		if (NULL == state)
		{
			state = NiGetApexSDK()->getParameterizedTraits()->createNxParameterized(DestructibleActorState::staticClassName());
}
		PX_ASSERT(state);

		// Reset the state if it already exists
		if (mState != NULL)
		{
			mState->setSerializationCallback(NULL);
			PX_ASSERT(mDestructibleScene && "Expected destructible scene with existing actor state.");
			reset();
		}

		mState  = static_cast<DestructibleActorState*>(state);
		mParams = NULL;
		mChunks = NULL;

		if (NULL != mState)
		{
			mState->setSerializationCallback(this, (void*)((intptr_t)NxDestructibleParameterizedType::State));

			// Cache a handle to the actorParameters
			NxParameterized::Handle handle(*mState);
			NxParameterized::Interface* p = NULL;
			mState->getParameterHandle("actorParameters", handle);
			mState->getParamRef(handle, p);
			if (NULL != p)
			{
				mParams = static_cast<DestructibleActorParam*>(p);
				mParams->setSerializationCallback(this, (void*)((intptr_t)NxDestructibleParameterizedType::Params));
			}
			else
				APEX_INTERNAL_ERROR("Invalid destructible actor state parameterization of actor params.");

			// Cache a handle to the actorChunks
			p = NULL;
			mState->getParameterHandle("actorChunks", handle);
			mState->getParamRef(handle, p);
			if (NULL != p)
			{
				mChunks = static_cast<DestructibleActorChunks*>(p);
			}
			else
				APEX_INTERNAL_ERROR("Invalid destructible actor state parameterization of actor params.");
		}
	}
}

void DestructibleActor::createRenderable()
{
	NxRenderMeshActor* renderMeshActors[NxDestructibleActorMeshType::Count];
	for (int meshN = 0; meshN < NxDestructibleActorMeshType::Count; ++meshN)
	{
		renderMeshActors[meshN] = NULL;
	}

	// There were three choices here as to the context in which to create this renderMeshActor.
	//   a) Use the global ApexSDK context
	//	 b) Make the destructible actor itself an NxApexContext
	//	 c) Use the destructible actor's context (mContext)
	//
	// The decision was made to use option A, the global context, and have the destrucible actor's destructor
	// call the render mesh actor's release() method.  Option B was too much overhead for a single sub-actor
	// and option C would have introduced race conditions at context deletion time.
	PX_ASSERT(mAsset->getRenderMeshAsset());
	NxRenderMeshActorDesc renderableMeshDesc;
	renderableMeshDesc.visible = false;
	renderableMeshDesc.bufferVisibility = true;
	renderableMeshDesc.indexBufferHint = NxRenderBufferHint::DYNAMIC;
	renderableMeshDesc.keepVisibleBonesPacked = keepVisibleBonesPacked();
	renderableMeshDesc.forceBoneIndexChannel = !renderableMeshDesc.keepVisibleBonesPacked;
	renderableMeshDesc.overrideMaterials      = mDescOverrideSkinnedMaterials;
	renderableMeshDesc.overrideMaterialCount  = mDescOverrideSkinnedMaterialCount;
	renderableMeshDesc.keepPreviousFrameBoneBuffer = keepPreviousFrameBoneBuffer();

	renderMeshActors[NxDestructibleActorMeshType::Skinned] = mAsset->getRenderMeshAsset()->createActor(renderableMeshDesc);

	if (drawStaticChunksInSeparateMesh() && !(getFlags() & Dynamic))
	{
		// Create static render mesh
		renderableMeshDesc.renderWithoutSkinning = true;
		renderableMeshDesc.keepPreviousFrameBoneBuffer = false;
#if NX_SDK_VERSION_MAJOR == 2
		PX_ALLOCA(staticMaterialNames, const char*, NxMath::max(mAsset->mParams->staticMaterialNames.arraySizes[0], 1));
#elif NX_SDK_VERSION_MAJOR == 3
		PX_ALLOCA(staticMaterialNames, const char*, PxMax(mAsset->mParams->staticMaterialNames.arraySizes[0], 1));
#endif
		if (mDescOverrideStaticMaterialCount > 0)
		{
			// If static override materials are defined, use them
			renderableMeshDesc.overrideMaterialCount = mDescOverrideStaticMaterialCount;
			renderableMeshDesc.overrideMaterials     = mDescOverrideStaticMaterials;
		}
		else
		{
			// Otherwise, use the static materials in the asset, if they're defined
			renderableMeshDesc.overrideMaterialCount = (physx::PxU32)mAsset->mParams->staticMaterialNames.arraySizes[0];
			renderableMeshDesc.overrideMaterials = (const char**)staticMaterialNames;
			for (int i = 0; i < mAsset->mParams->staticMaterialNames.arraySizes[0]; ++i)
			{
				staticMaterialNames[i] = mAsset->mParams->staticMaterialNames.buf[i].buf;
			}
		}
		renderMeshActors[NxDestructibleActorMeshType::Static] = mAsset->getRenderMeshAsset()->createActor(renderableMeshDesc);
	}

	//#if APEX_RUNTIME_FRACTURE
	//mRenderable = PX_NEW(DestructibleRTRenderable)(renderMeshActors,mRTActor);
	//#else
	mRenderable = PX_NEW(DestructibleRenderable)(renderMeshActors,getAsset(),(physx::PxI32)m_listIndex);
	//#endif
}

void DestructibleActor::initializeActor(void)
{
	if (!mParams->doNotCreateRenderable)
	{
		createRenderable();
	}

#if NX_SDK_VERSION_MAJOR == 2	// In 3.x this is put into the asset
	mCollisionMeshes = mDestructibleScene->mModule->mCachedData->getConvexMeshesForActor(*this);
	PX_ASSERT(mCollisionMeshes != NULL);
#endif

	mFirstChunkIndex = DestructibleAsset::InvalidChunkIndex;

	// Allow color channel replacement if requested
	mUseDamageColoring = getParams()->defaultBehaviorGroup.damageColorChange.magnitudeSquared() != 0.0f;
	for (physx::PxI32 behaviorGroupN = 0; !mUseDamageColoring && behaviorGroupN < getParams()->behaviorGroups.arraySizes[0]; ++behaviorGroupN)
	{
		mUseDamageColoring = getParams()->behaviorGroups.buf[behaviorGroupN].damageColorChange.magnitudeSquared() != 0.0f;
	}
	if (mUseDamageColoring)
	{
		NxRenderMeshAsset* rma = mAsset->getRenderMeshAsset();
		mDamageColorArrays.resize(rma->getSubmeshCount());
		for (physx::PxU32 submeshIndex = 0; submeshIndex < rma->getSubmeshCount(); ++submeshIndex)
		{
			physx::Array<PxColorRGBA>& damageColorArray = mDamageColorArrays[submeshIndex];
			const NxRenderSubmesh& submesh = rma->getSubmesh(submeshIndex);
			const NxVertexBuffer& vb = submesh.getVertexBuffer();
			const NxVertexFormat& vf = vb.getFormat();
			const physx::PxI32 colorBufferIndex = vf.getBufferIndexFromID(vf.getSemanticID(NxRenderVertexSemantic::COLOR));
			// Fix asset on &damageColorArray[0]
			if (vb.getVertexCount() != 0)
			{
				damageColorArray.resize(vb.getVertexCount());
				physx::PxMemZero(&damageColorArray[0], sizeof(PxColorRGBA)*damageColorArray.size());	// Default to zero in case this array doesn't exist in the asset
				if (colorBufferIndex >= 0)
				{
					vb.getBufferData(&damageColorArray[0], NxRenderDataFormat::R8G8B8A8, 0, (physx::PxU32)colorBufferIndex, 0, vb.getVertexCount());
				}
				for (physx::PxU32 typeN = 0; typeN < NxDestructibleActorMeshType::Count; ++typeN)
				{
					NiApexRenderMeshActor* renderMeshActor = (NiApexRenderMeshActor*)getRenderMeshActor((NxDestructibleActorMeshType::Enum)typeN);
					if (renderMeshActor != NULL)
					{
						renderMeshActor->setStaticColorReplacement(submeshIndex, &damageColorArray[0]);
					}
				}
			}
		}
	}

	initializeEmitters();

	mVisibleChunks.reserve(mAsset->getChunkCount());
	mVisibleChunks.lockCapacity(true);

	// This is done to handle old formats, and ensure we have a sufficient # of DepthParameters
	while (mDestructibleParameters.depthParametersCount < mAsset->getDepthCount() && mDestructibleParameters.depthParametersCount < NxDestructibleParameters::kDepthParametersCountMax)
	{
		NxDestructibleDepthParameters& params = mDestructibleParameters.depthParameters[mDestructibleParameters.depthParametersCount];
		if (mDestructibleParameters.depthParametersCount == 0)
		{
			// Set level 0 parameters to the more expensive settings
			memset(&params, 0xFF, sizeof(params));
		}
		++mDestructibleParameters.depthParametersCount;
	}

	// When we add ourselves to the ApexScene, it will call us back with setPhysXScene
	if (!findSelfInContext(*mDestructibleScene->mApexScene->getApexContext()))
	addSelfToContext(*mDestructibleScene->mApexScene->getApexContext());

	// Add ourself to our DestructibleScene
	if (!findSelfInContext(*DYNAMIC_CAST(ApexContext*)(mDestructibleScene)))
	{
		addSelfToContext(*DYNAMIC_CAST(ApexContext*)(mDestructibleScene));

		if (mAsset->mParams->chunkInstanceInfo.arraySizes[0] || mAsset->mParams->scatterMeshAssets.arraySizes[0])
		{
			mDestructibleScene->addInstancedActor(this);
		}
	}

	// Set a reasonable initial bounds until the first physics update
	mRenderBounds = getOriginalBounds();
	if (mRenderable != NULL)
	{
		mRenderable->setBounds(mRenderBounds);
	}
	mNonInstancedBounds = mRenderBounds;
	mInstancedBounds.setEmpty();
}

void DestructibleActor::initializeRTActor(void)
{
#if APEX_RUNTIME_FRACTURE
	//mRTActor->initialize();
	PX_DELETE(mRTActor);
	mRTActor = (physx::fracture::Actor*)mDestructibleScene->getDestructibleRTScene()->createActor(this);
#endif
}

void DestructibleActor::initializeEmitters(void)
{
#if APEX_USE_PARTICLES
	if (getCrumbleEmitterName() || mAsset->getCrumbleEmitterName())
	{
		const char* name;
		if (getCrumbleEmitterName())
		{
			name = getCrumbleEmitterName();
			mAsset->mCrumbleAssetTracker.addAssetName(name, false);
		}
		else
		{
			name = mAsset->getCrumbleEmitterName();
		}
		initCrumbleSystem(name);
	}
	{
		const char* name;
		if (getDustEmitterName())
		{
			name = getDustEmitterName();
			mAsset->mDustAssetTracker.addAssetName(name, false);
		}
		else
		{
			name = mAsset->getDustEmitterName();
		}
		initDustSystem(name);
	}
#endif // APEX_USE_PARTICLES
}

void DestructibleActor::setDestructibleParameters(const DestructibleActorParamNS::DestructibleParameters_Type& destructibleParameters,
												  const DestructibleActorParamNS::DestructibleDepthParameters_DynamicArray1D_Type& destructibleDepthParameters)
{
	NxDestructibleParameters parameters;
	deserialize(destructibleParameters, destructibleDepthParameters, parameters);
	setDestructibleParameters(parameters);
}

void DestructibleActor::setDestructibleParameters(const NxDestructibleParameters& _destructibleParameters)
{
	// If essentialDepth changes, must re-count essential visible chunks
	if(_destructibleParameters.essentialDepth != mDestructibleParameters.essentialDepth)
	{
		mEssentialVisibleDynamicChunkShapeCount = 0;
		const physx::PxU16* chunkIndexPtr = mVisibleChunks.usedIndices();
		const physx::PxU16* chunkIndexPtrStop = chunkIndexPtr + mVisibleChunks.usedCount();
		while (chunkIndexPtr < chunkIndexPtrStop)
		{
			physx::PxU16 chunkIndex = *chunkIndexPtr++;
			if(getDynamic(chunkIndex))
			{
				if((physx::PxU32)getAsset()->mParams->chunks.buf[chunkIndex].depth <= _destructibleParameters.essentialDepth)
				{
					mEssentialVisibleDynamicChunkShapeCount += getAsset()->getChunkHullCount(chunkIndex);
				}
			}
		}
	}

	// For now we keep a cached copy of the parameters
	//    At some point we may want to move completely into the state
	mDestructibleParameters = _destructibleParameters;


	getParams()->supportDepth = physx::PxClamp(getSupportDepth(), (physx::PxU32)0, mAsset->getDepthCount() - 1);

	// Make sure the depth params are filled completely
	physx::PxU32 oldSize    = mDestructibleParameters.depthParametersCount;
	physx::PxU32 neededSize = mAsset->getDepthCount();
	if (neededSize > NxDestructibleParameters::kDepthParametersCountMax)
	{
		neededSize = NxDestructibleParameters::kDepthParametersCountMax;
	}
	mDestructibleParameters.depthParametersCount = neededSize;
	// Fill in remaining with asset defaults
	for (physx::PxU32 i = oldSize; i < neededSize; ++i)
	{
		mDestructibleParameters.depthParameters[i].setToDefault();
	}
}

/* An (emitter) actor in our context has been released */
void DestructibleActor::removeActorAtIndex(physx::PxU32 index)
{
#if APEX_USE_PARTICLES
	if (mDustEmitter == mActorArray[index]->getNxApexActor())
	{
		mDustEmitter = 0;
	}
	if (mCrumbleEmitter == mActorArray[index]->getNxApexActor())
	{
		mCrumbleEmitter = 0;
	}
#endif // APEX_USE_PARTICLES

	ApexContext::removeActorAtIndex(index);
}


void DestructibleActor::wakeUp(void)
{
	PX_ASSERT(mDestructibleScene);
	if (mDestructibleScene)
	{
		mDestructibleScene->addToAwakeList(*this);
	}
}

void DestructibleActor::putToSleep(void)
{
	PX_ASSERT(mDestructibleScene);
	if (mDestructibleScene)
	{
		mDestructibleScene->removeFromAwakeList(*this);
	}
}

// TODO:		clean up mAwakeActorCount management with mUsingActiveTransforms
// [APEX-671]	mind that un/referencedByActor() also call these functions

void DestructibleActor::incrementWakeCount(void)
{
	if (!mAwakeActorCount)
	{
		wakeUp();
	}
	mAwakeActorCount++;
}

void DestructibleActor::referencedByActor(NxActor* actor)
{
	if (mReferencingActors.find(actor) == mReferencingActors.end())
	{
		// We need to check IS_SLEEPING instead of actor->isSleeping,
		// because the state might have changed between the last callback and now.
		// Here, the state of the last callback is needed.
		NiApexPhysXObjectDesc* desc = (NiApexPhysXObjectDesc*)(NiGetApexSDK()->getPhysXObjectInfo(actor));
		if (mDestructibleScene != NULL && !mDestructibleScene->mUsingActiveTransforms && desc != NULL && !desc->getUserDefinedFlag(PhysXActorFlags::IS_SLEEPING))
		{
			incrementWakeCount();
		}
		mReferencingActors.pushBack(actor);
	}
}

void DestructibleActor::unreferencedByActor(NxActor* actor)
{
	if (mReferencingActors.findAndReplaceWithLast(actor))
	{
		NiApexPhysXObjectDesc* desc = (NiApexPhysXObjectDesc*)(NiGetApexSDK()->getPhysXObjectInfo(actor));
		if (mDestructibleScene != NULL && !mDestructibleScene->mUsingActiveTransforms && desc != NULL && !desc->getUserDefinedFlag(PhysXActorFlags::IS_SLEEPING))
		{
			decrementWakeCount();
		}
	}
}

void DestructibleActor::wakeForEvent()
{
	if (!mWakeForEvent)
	{
		mWakeForEvent = true;
		incrementWakeCount();
	}
}

void DestructibleActor::resetWakeForEvent()
{
	if (mWakeForEvent)
	{
		mWakeForEvent = false;
		decrementWakeCount();
	}
}

void DestructibleActor::decrementWakeCount(void)
{
	// this assert shouldn't happen, so if it does it means bad things, tell james.
	// basically we keep a counter of the number of awake bodies in a destructible actor
	// so that when its zero we know no updates are needed (normally updates are really expensive
	// per-destructible). So, if wake count is 0 and its trying to decrement it means
	// this value is completely out of sync and that can result in bad things (performance loss
	// or behavior loss). So, don't remove this assert!

#if NX_SDK_VERSION_MAJOR == 3 // the counter does still not work correctly with 2.8.4
	PX_ASSERT(mAwakeActorCount > 0);
#endif
	if (mAwakeActorCount > 0)
	{
		mAwakeActorCount--;
		if (!mAwakeActorCount)
		{
			putToSleep();
		}
	}
}

void DestructibleActor::setChunkVisibility(physx::PxU16 index, bool visibility)
{
	PX_ASSERT((physx::PxI32)index < mAsset->mParams->chunks.arraySizes[0]);
	if (visibility)
	{
		if(mVisibleChunks.use(index))
		{
			if (createChunkEvents())
			{
				mChunkEventBufferLock.lock();
					
				if (mChunkEventBuffer.size() == 0 && mDestructibleScene->getModule()->m_chunkStateEventCallbackSchedule != NxDestructibleCallbackSchedule::Disabled)
				{
					mStructure->dscene->mActorsWithChunkStateEvents.pushBack(this);
				}

				NxDestructibleChunkEvent& chunkEvent = mChunkEventBuffer.insert();
				chunkEvent.chunkIndex = index;
				chunkEvent.event = NxDestructibleChunkEvent::VisibilityChanged | NxDestructibleChunkEvent::ChunkVisible;

				mChunkEventBufferLock.unlock();
			}
			if(getDynamic(index))
			{
				const physx::PxU32 chunkShapeCount = getAsset()->getChunkHullCount(index);
				mVisibleDynamicChunkShapeCount += chunkShapeCount;
				if((physx::PxU32)getAsset()->mParams->chunks.buf[index].depth <= mDestructibleParameters.essentialDepth)
				{
					mEssentialVisibleDynamicChunkShapeCount += chunkShapeCount;
				}
			}
		}
	}
	else
	{
		if(mVisibleChunks.free(index))
		{
			if(getDynamic(index))
			{
				const physx::PxU32 chunkShapeCount = getAsset()->getChunkHullCount(index);
				mVisibleDynamicChunkShapeCount = mVisibleDynamicChunkShapeCount >= chunkShapeCount ? mVisibleDynamicChunkShapeCount - chunkShapeCount : 0;
				if((physx::PxU32)getAsset()->mParams->chunks.buf[index].depth <= mDestructibleParameters.essentialDepth)
				{
					mEssentialVisibleDynamicChunkShapeCount = mEssentialVisibleDynamicChunkShapeCount >= chunkShapeCount ? mEssentialVisibleDynamicChunkShapeCount - chunkShapeCount : 0;
				}
			}
			if (createChunkEvents())
			{
				mChunkEventBufferLock.lock();

				if (mChunkEventBuffer.size() == 0 && mDestructibleScene->getModule()->m_chunkStateEventCallbackSchedule != NxDestructibleCallbackSchedule::Disabled)
				{
					mStructure->dscene->mActorsWithChunkStateEvents.pushBack(this);
				}

				NxDestructibleChunkEvent& chunkEvent = mChunkEventBuffer.insert();
				chunkEvent.chunkIndex = index;
				chunkEvent.event = NxDestructibleChunkEvent::VisibilityChanged;

				mChunkEventBufferLock.unlock();
			}
		}
	}
	DestructibleAssetParametersNS::Chunk_Type& sourceChunk = mAsset->mParams->chunks.buf[index];
	if ((sourceChunk.flags & DestructibleAsset::Instanced) == 0)
	{
		// Not instanced - need to choose the static or dynamic mesh, and set visibility for the render mesh actor
		const NxDestructibleActorMeshType::Enum typeN = (getDynamic(index) || !drawStaticChunksInSeparateMesh()) ?
				NxDestructibleActorMeshType::Skinned : NxDestructibleActorMeshType::Static;
		NxRenderMeshActor* rma = getRenderMeshActor(typeN);
		if (rma != NULL)
		{
			NiApexRenderMeshActor* rmi = static_cast<NiApexRenderMeshActor*>(rma);
			const bool visibilityChanged = rmi->setVisibility(visibility, sourceChunk.meshPartIndex);
			if (keepPreviousFrameBoneBuffer() && visibilityChanged && visibility)
			{
				// Visibility changed from false to true.  If we're keeping the previous frame bone buffer, be sure to seed the previous frame buffer
				if (!mStructure->chunks[index + mFirstChunkIndex].isDestroyed())
				{
					rmi->setLastFrameTM(getChunkPose(index), getScale(), sourceChunk.meshPartIndex);
				}
			}
		}
	}
}

void DestructibleActor::cacheModuleData() const
{
	PX_PROFILER_PERF_SCOPE("DestructibleCacheChunkCookedCollisionMeshes");

	if (mAsset == NULL)
	{
		PX_ASSERT(!"cacheModuleData: asset is NULL.\n");
		return;
	}

#if NX_SDK_VERSION_MAJOR == 2
	//physx::Array<NxConvexMesh*>* meshes = mStructure->dscene->mModule->mCachedData->getConvexMeshesForActor(*this);
	// No longer need to cache here, as the actor pre-caches these when created
#else
	// The NxDestructibleActor::cacheModuleData() method needs to avoid incrementing the ref count
	bool incCacheRefCount = false;
	physx::Array<NxConvexMesh*>* meshes = mStructure->dscene->mModule->mCachedData->getConvexMeshesForScale(*this->mAsset, 
																											getDestructibleScene()->getModule()->getChunkCollisionHullCookingScale(),
																											incCacheRefCount);
	if (meshes == NULL)
	{
		PX_ASSERT(!"cacheModuleData: failed to create convex mesh cache for actor.\n");
		return;
	}
#endif
}

/* Called by NxApexScene when the actor is added to the scene context or when the NxApexScene's
 * NxScene reference changes.  nxScene can be NULL if the NxApexScene is losing its NxScene reference.
 */
void DestructibleActor::setPhysXScene(NxScene* nxScene)
{
	if (nxScene)
	{
		PX_ASSERT(mStructure == NULL);
		mDestructibleScene->insertDestructibleActor(mAPI);
	}
	else
	{
		PX_ASSERT(mStructure != NULL);
		removeSelfFromStructure();
	}
}

NxScene* DestructibleActor::getPhysXScene() const
{
	return mDestructibleScene->mPhysXScene;
}

void DestructibleActor::removeSelfFromStructure()
{
	if (!mStructure)
	{
		return;
	}

	DestructibleStructure* oldStructure = mStructure;

	mStructure->removeActor(this);

	if (oldStructure->destructibles.size() == 0)
	{
		oldStructure->dscene->removeStructure(oldStructure);
	}

	for (physx::PxU32 typeN = 0; typeN < NxDestructibleActorMeshType::Count; ++typeN)
	{
		NxRenderMeshActor* renderMeshActor = getRenderMeshActor((NxDestructibleActorMeshType::Enum)typeN);
		if (renderMeshActor != NULL)
		{
			while (renderMeshActor->visiblePartCount() > 0)
			{
				renderMeshActor->setVisibility(false, (physx::PxU16)renderMeshActor->getVisibleParts()[renderMeshActor->visiblePartCount() - 1]);
			}
		}
	}

	mDestructibleScene->mTotalChunkCount -= mVisibleChunks.usedCount();

	mVisibleChunks.clear(mAsset->getChunkCount());
}

void DestructibleActor::removeSelfFromScene()
{
	if (mDestructibleScene == NULL)
	{
		return;
	}

	mDestructibleScene->mDestructibles.direct(mID) = NULL;
	mDestructibleScene->mDestructibles.free(mID);

	mID = (physx::PxU32)InvalidID;
}

DestructibleActor::~DestructibleActor()
{
#if APEX_RUNTIME_FRACTURE
	PX_DELETE(mRTActor);
	mRTActor = NULL;
#endif
}

void DestructibleActor::release()
{
	if (mInRelease)
	{
		return;
	}
	mInRelease = true;
	
	mAsset->module->mCachedData->releaseReferencesToConvexMeshesForActor(*this);

	// after this call, mAsset and all other members are freed and unusable
	mAsset->releaseDestructibleActor(*mAPI);

	// HC SVNT DRACONES
}

void DestructibleActor::destroy()
{
	mDestructibleScene->removeReferencesToActor(*this);

	if (NULL != mDescOverrideSkinnedMaterials)
	{
		PX_FREE_AND_RESET(mDescOverrideSkinnedMaterials);
	}

	//ApexActor::destroy();
	mInRelease = true;
	renderDataLock();
	for (physx::PxU32 i = 0 ; i < mContexts.size() ; i++)
	{
		ContextTrack& t = mContexts[i];
		t.ctx->removeActorAtIndex(t.index);
	}
	mContexts.clear();
	renderDataUnLock();

	ApexResource::removeSelf();

	// BRG - moving down here from the top of this function - mState seems to be needed
	if (NULL != mState)
	{
		mState->destroy();
		mState = NULL;
	}

#if APEX_RUNTIME_FRACTURE
	//if (mRTActor != NULL)
	//{
	//	mRTActor->notifyParentActorDestroyed();
	//}
#endif

	if (mRenderable != NULL)
	{
		mRenderable->release();
		mRenderable = NULL;
	}

#if APEX_USE_PARTICLES
	if (mDustEmitter)
	{
		mDustEmitter->release();
		mDustEmitter = NULL;
	}

	if (mCrumbleEmitter)
	{
		mCrumbleEmitter->release();
		mCrumbleEmitter = NULL;
	}
#endif // APEX_USE_PARTICLES

#if USE_DESTRUCTIBLE_RWLOCK
	if (mLock)
	{
		mLock->~ReadWriteLock();
		PX_FREE(mLock);
	}
	mLock = NULL;
#endif

	// acquire the buffer so we can properly release it
	const NxDestructibleChunkEvent* tmpChunkEventBuffer=NULL;
	physx::PxU32 tmpChunkEventBufferSize = 0;
	acquireChunkEventBuffer(tmpChunkEventBuffer, tmpChunkEventBufferSize);

	releaseChunkEventBuffer();

	releasePhysXActorBuffer();

	removeSelfFromScene();

	if(0 != mSyncParams.getUserActorID())
	{
		setSyncParams(0, 0, NULL, NULL);
	}
}

void DestructibleActor::reset()
{
	destroy();

	mInRelease = false;

	///////////////////////////////////////////////////////////////////////////

	mState                            = NULL;
	mParams                           = NULL;
	mTM                               = physx::PxMat44::createIdentity();
	mFlags                            = 0;
	mInternalFlags                    = 0;
	mStructure                        = NULL;
	mID                               = (physx::PxU32)InvalidID;
	mLinearSize                       = 0.0f;
	mCrumbleEmitter                   = NULL;
	mDustEmitter                      = NULL;
	mCrumbleRenderVolume              = NULL;
	mDustRenderVolume                 = NULL;
	mStartTime						  = mDestructibleScene->mElapsedTime;
	mInitializedFromState             = false;
	mAwakeActorCount                  = 0;
	mDescOverrideSkinnedMaterialCount = 0;
	mDescOverrideStaticMaterialCount  = 0;

	mStaticRoots.clear();
	mVisibleChunks.clear();

	#if APEX_RUNTIME_FRACTURE
	PX_DELETE(mRTActor);
	mRTActor = NULL;
	#endif
}

void DestructibleActor::initializeChunk(physx::PxU32 index, DestructibleStructure::Chunk& target) const
{
	const DestructibleAssetParametersNS::Chunk_Type& source = getAsset()->mParams->chunks.buf[index];

	// Derived parameters
	target.destructibleID		= getID();
	target.reportID				= (physx::PxU32)DestructibleScene::InvalidReportID;
	target.indexInAsset			= (physx::PxU16)index;
	target.islandID				= (physx::PxU32)DestructibleStructure::InvalidID;

	// Serialized parameters
	if (mInitializedFromState && (physx::PxI32)index < mChunks->data.arraySizes[0])
	{
		deserializeChunkData(mChunks->data.buf[index], target);
		if (target.visibleAncestorIndex != (physx::PxI32)DestructibleStructure::InvalidChunkIndex)
			target.visibleAncestorIndex += getFirstChunkIndex();
	}
	// Default parameters
	else
	{
		target.state               = PxU8(isInitiallyDynamic() ? ChunkDynamic : 0);
		target.flags               = 0;
		target.damage              = 0;
		target.localOffset         = getScale().multiply(getAsset()->getChunkPositionOffset(index));
		const physx::PxBounds3& bounds = getAsset()->getChunkShapeLocalBounds(index);
		physx::PxVec3 center       = bounds.getCenter();
		physx::PxVec3 extents      = bounds.getExtents();
		center                     = center.multiply(getScale());
		extents                    = extents.multiply(getScale());
		target.localSphere         = NxSphere(NXFROMPXVEC3(center), extents.magnitude());
		target.clearShapes();
		target.controlledChunk     = NULL;

		if (source.numChildren == 0)
		{
			target.flags |= ChunkMissingChild;
		}
	}

#if USE_CHUNK_RWLOCK
	target.lock					= (physx::ReadWriteLock*)PX_ALLOC(sizeof(physx::ReadWriteLock), PX_DEBUG_EXP("DestructibleActor::RWLock"));
	PX_PLACEMENT_NEW(target.lock, physx::ReadWriteLock);
#endif

	// I have not yet determined if this flag should be set for all actors,
	//    or only for those initialized purely from params
	/*if (source.numChildren == 0)
	{
		target.flags |= ChunkMissingChild;
	}*/
}


const DestructibleStructure::Chunk& DestructibleActor::getChunk(physx::PxU32 index) const
{
	PX_ASSERT(mStructure != NULL);
	PX_ASSERT(index + mFirstChunkIndex < mStructure->chunks.size());
	return mStructure->chunks[index + mFirstChunkIndex];
}

physx::PxMat44 DestructibleActor::getChunkPose(physx::PxU32 index) const
{
#if NX_SDK_VERSION_MAJOR == 3
	SCOPED_PHYSX_LOCK_READ(*mDestructibleScene->mApexScene);
#endif

	PX_ASSERT(mStructure != NULL);
	PX_ASSERT(index + mFirstChunkIndex < mStructure->chunks.size());
	PX_ASSERT(!mStructure->chunks[index + mFirstChunkIndex].isDestroyed());
	return mStructure->getChunkGlobalPose(mStructure->chunks[index + mFirstChunkIndex]);
}

physx::PxTransform DestructibleActor::getChunkTransform(physx::PxU32 index) const
{
	PX_ASSERT(mStructure != NULL);
	PX_ASSERT(index + mFirstChunkIndex < mStructure->chunks.size());
	PX_ASSERT(!mStructure->chunks[index + mFirstChunkIndex].isDestroyed());
	return mStructure->getChunkGlobalTransform(mStructure->chunks[index + mFirstChunkIndex]);
}

bool DestructibleActor::getInitialChunkDestroyed(physx::PxU32 index) const
{
	PX_ASSERT((physx::PxI32)index < mChunks->data.arraySizes[0]);
	return (physx::PxI32)index < mChunks->data.arraySizes[0]
		? (mChunks->data.buf[index].shapesCount == 0 && mChunks->data.buf[index].visibleAncestorIndex == (physx::PxI32)DestructibleStructure::InvalidChunkIndex)
		: false;
}

bool DestructibleActor::getInitialChunkDynamic(physx::PxU32 index) const
{
	PX_ASSERT((physx::PxI32)index < mChunks->data.arraySizes[0]);
	return ((physx::PxI32)index < mChunks->data.arraySizes[0])
		? ((mChunks->data.buf[index].state & ChunkDynamic) != 0) : true;
}

bool DestructibleActor::getInitialChunkVisible(physx::PxU32 index) const
{
	PX_ASSERT((index == 0 && mChunks->data.arraySizes[0] == 0) ||
		      (physx::PxI32)index < mChunks->data.arraySizes[0]);
	return ((physx::PxI32)index < mChunks->data.arraySizes[0])
		? ((mChunks->data.buf[index].state & ChunkVisible) != 0) : true;
}

physx::PxTransform DestructibleActor::getInitialChunkGlobalPose(physx::PxU32 index) const
{
	PX_ASSERT((physx::PxI32)index < mChunks->data.arraySizes[0]);
	return (physx::PxI32)index < mChunks->data.arraySizes[0]
		? mChunks->data.buf[index].globalPose : PxTransform::createIdentity();
}

physx::PxTransform DestructibleActor::getInitialChunkLocalPose(physx::PxU32 index) const
{
	return PxTransform(getAsset()->getChunkPositionOffset(index));
}

physx::PxVec3 DestructibleActor::getInitialChunkLinearVelocity(physx::PxU32 index) const
{
	PX_ASSERT((physx::PxI32)index < mChunks->data.arraySizes[0]);
	return (physx::PxI32)index < mChunks->data.arraySizes[0]
		? mChunks->data.buf[index].linearVelocity : PxVec3(0);
}

physx::PxVec3 DestructibleActor::getInitialChunkAngularVelocity(physx::PxU32 index) const
{
	PX_ASSERT((physx::PxI32)index < mChunks->data.arraySizes[0]);
	return (physx::PxI32)index < mChunks->data.arraySizes[0]
		? mChunks->data.buf[index].angularVelocity : PxVec3(0);
}

physx::PxVec3 DestructibleActor::getChunkLinearVelocity(physx::PxU32 index) const
{
	PX_ASSERT(mStructure != NULL);
	PX_ASSERT(index + mFirstChunkIndex < mStructure->chunks.size());
	PX_ASSERT(!mStructure->chunks[index + mFirstChunkIndex].isDestroyed());
	return PXFROMNXVEC3(getChunkActor(index)->getLinearVelocity());
}

physx::PxVec3 DestructibleActor::getChunkAngularVelocity(physx::PxU32 index) const
{
	PX_ASSERT(mStructure != NULL);
	PX_ASSERT(index + mFirstChunkIndex < mStructure->chunks.size());
	PX_ASSERT(!mStructure->chunks[index + mFirstChunkIndex].isDestroyed());
	return PXFROMNXVEC3(getChunkActor(index)->getAngularVelocity());
}

NxActor* DestructibleActor::getChunkActor(physx::PxU32 index)
{
	PX_ASSERT(mStructure != NULL);
	PX_ASSERT(index + mFirstChunkIndex < mStructure->chunks.size());
	DestructibleStructure::Chunk& chunk = mStructure->chunks[index + mFirstChunkIndex];
	return (NxActor*)mStructure->getChunkActor(chunk);
}

const NxActor* DestructibleActor::getChunkActor(physx::PxU32 index) const
{
	PX_ASSERT(mStructure != NULL);
	PX_ASSERT(index + mFirstChunkIndex < mStructure->chunks.size());
	DestructibleStructure::Chunk& chunk = mStructure->chunks[index + mFirstChunkIndex];
	return (NxActor*)mStructure->getChunkActor(chunk);
}

#if NX_SDK_VERSION_MAJOR == 2
physx::PxU32 DestructibleActor::getChunkPhysXShapes(NxShape**& shapes, physx::PxU32 chunkIndex) const
#elif NX_SDK_VERSION_MAJOR == 3
physx::PxU32 DestructibleActor::getChunkPhysXShapes(physx::PxShape**& shapes, physx::PxU32 chunkIndex) const
#endif
{
	PX_ASSERT(mStructure != NULL);
	PX_ASSERT(chunkIndex + mFirstChunkIndex < mStructure->chunks.size());
	DestructibleStructure::Chunk& chunk = mStructure->chunks[chunkIndex + mFirstChunkIndex];
	physx::Array<NxShape*>& shapeArray = mStructure->getChunkShapes(chunk);
	shapes = shapeArray.size() ? &shapeArray[0] : NULL;
	return shapeArray.size();
}

physx::PxU32 DestructibleActor::getChunkActorFlags(physx::PxU32 index) const
{
	PX_ASSERT(mStructure != NULL);
	PX_ASSERT(index + mFirstChunkIndex < mStructure->chunks.size());
	DestructibleStructure::Chunk& chunk = mStructure->chunks[index + mFirstChunkIndex];
	physx::PxU32 flags = 0;
	if (chunk.flags & ChunkWorldSupported)
	{
		flags |= NxDestructibleActorChunkFlags::ChunkIsWorldSupported;
	}
	return flags;
}

void DestructibleActor::applyDamage(physx::PxF32 damage, physx::PxF32 momentum, const physx::PxVec3& position, const physx::PxVec3& direction, physx::PxI32 chunkIndex, void* userData)
{
	DamageEvent& damageEvent = mStructure->dscene->getDamageWriteBuffer().pushBack();
	damageEvent.destructibleID = mID;
	damageEvent.damage = damage;
	damageEvent.momentum = momentum;
	damageEvent.position = position;
	damageEvent.direction = direction;
	damageEvent.radius = 0.0f;
	damageEvent.chunkIndexInAsset = chunkIndex >= 0 ? chunkIndex : NxModuleDestructibleConst::INVALID_CHUNK_INDEX;
	damageEvent.flags = 0;
	damageEvent.impactDamageActor = NULL;
	damageEvent.appliedDamageUserData = userData;
}

void DestructibleActor::applyRadiusDamage(physx::PxF32 damage, physx::PxF32 momentum, const physx::PxVec3& position, physx::PxF32 radius, bool falloff, void* userData)
{
	DamageEvent& damageEvent = mStructure->dscene->getDamageWriteBuffer().pushBack();
	damageEvent.destructibleID = mID;
	damageEvent.damage = damage;
	damageEvent.momentum = momentum;
	damageEvent.position = position;
	damageEvent.direction = physx::PxVec3(0.0f);	// not used
	damageEvent.radius = radius;
	damageEvent.chunkIndexInAsset = NxModuleDestructibleConst::INVALID_CHUNK_INDEX;
	damageEvent.flags = DamageEvent::UseRadius;
	damageEvent.impactDamageActor = NULL;
	damageEvent.appliedDamageUserData = userData;
	if (falloff)
	{
		damageEvent.flags |= DamageEvent::HasFalloff;
	}
}

#if NX_SDK_VERSION_MAJOR == 2
void DestructibleActor::takeImpact(const physx::PxVec3& force, const physx::PxVec3& position, physx::PxU16 chunkIndex, NxActor const* damageActor)
#elif NX_SDK_VERSION_MAJOR == 3
void DestructibleActor::takeImpact(const physx::PxVec3& force, const physx::PxVec3& position, physx::PxU16 chunkIndex, PxActor const* damageActor)
#endif
{
	if (chunkIndex >= (physx::PxU16)mAsset->mParams->chunks.arraySizes[0])
	{
		return;
	}
	DestructibleAssetParametersNS::Chunk_Type& source = mAsset->mParams->chunks.buf[chunkIndex];
	if (!takesImpactDamageAtDepth(source.depth))
	{
		return;
	}
	DamageEvent& damageEvent = mStructure->dscene->getDamageWriteBuffer().pushBack();
	damageEvent.direction = force;
	damageEvent.destructibleID = mID;
	const physx::PxF32 magnitude = damageEvent.direction.normalize();
	damageEvent.damage = magnitude * mDestructibleParameters.forceToDamage;
	damageEvent.momentum = 0.0f;
	damageEvent.position = position;
	damageEvent.radius = 0.0f;
	damageEvent.chunkIndexInAsset = chunkIndex;
	damageEvent.flags = DamageEvent::IsFromImpact;
	damageEvent.impactDamageActor = damageActor;
	damageEvent.appliedDamageUserData = NULL;

	if (mStructure->dscene->mModule->m_impactDamageReport != NULL)
	{
		NxImpactDamageEventData& data = mStructure->dscene->mImpactDamageEventData.insert();
		(NxDamageEventCoreData&)data = (NxDamageEventCoreData&)damageEvent;	// Copy core data
		data.destructible = mAPI;
		data.direction = damageEvent.direction;
		data.impactDamageActor = damageActor;
	}
}

physx::PxI32 DestructibleActor::pointOrOBBSweep(physx::PxF32& time, physx::PxVec3& normal, const NxBox& worldBox, const physx::PxVec3& pxWorldDisp,
        NxDestructibleActorRaycastFlags::Enum flags, physx::PxI32 parentChunkIndex) const
{
	PX_PROFILER_PERF_SCOPE("DestructibleActorPointOrOBBSweep");

	// use the scene lock to protect chunk data for
	// multi-threaded obbSweep calls on destructible actors
	// (different lock would be good, but mixing locks can cause deadlocks)
	SCOPED_PHYSX_LOCK_READ(*mDestructibleScene->mApexScene);

	// TODO: the normal is not always output, for instance, when accurateRaycasts is false, can we generate a decent normal in this case?
	const bool pointSweep = (worldBox.extents.magnitudeSquared() == 0.0f);

	const physx::PxU32 dynamicStateFlags = ((physx::PxU32)flags)&NxDestructibleActorRaycastFlags::AllChunks;
	if (dynamicStateFlags == 0)
	{
		return NxModuleDestructibleConst::INVALID_CHUNK_INDEX;
	}

#if NX_SDK_VERSION_MAJOR == 2
	const NxVec3 worldBoxAxes[3] = { worldBox.rot.getColumn(0),  worldBox.rot.getColumn(1),  worldBox.rot.getColumn(2) };
#elif NX_SDK_VERSION_MAJOR == 3
	const NxVec3 worldBoxAxes[3] = { worldBox.rot.column0,  worldBox.rot.column1,  worldBox.rot.column2 };
#endif
	const NxVec3 worldDisp = NXFROMPXVEC3(pxWorldDisp);

	// Project box along displacement direction, for the entire sweep
	const physx::PxF32 worldDisp2 = worldDisp.magnitudeSquared();
	const bool sweep = (worldDisp2 != 0.0f);
	const physx::PxF32 recipWorldDisp2 = sweep ? 1.0f / worldDisp2 : 0.0f;
	const physx::PxF32 boxProjectedRadius = worldBox.extents.x * physx::PxAbs(worldDisp | worldBoxAxes[0]) +
	                                        worldBox.extents.y * physx::PxAbs(worldDisp | worldBoxAxes[1]) +
	                                        worldBox.extents.z * physx::PxAbs(worldDisp | worldBoxAxes[2]);
	const physx::PxF32 boxProjectedCenter = worldDisp | worldBox.center;
	const physx::PxF32 boxBSphereRadius = worldBox.extents.magnitude();	// May reduce this by projecting the box along worldDisp

	physx::PxF32 boxSweptMax;
	physx::PxF32 minRayT;
	physx::PxF32 maxRayT;

	const physx::PxF32 boxProjectedMin = boxProjectedCenter - boxProjectedRadius;
	const physx::PxF32 boxProjectedMax = boxProjectedCenter + boxProjectedRadius;
	if ((flags & NxDestructibleActorRaycastFlags::SegmentIntersect) != 0)
	{
		boxSweptMax = boxProjectedMax + worldDisp2;
		minRayT = 0.0f;
		maxRayT = 1.0f;
	}
	else
	{
		boxSweptMax = PX_MAX_F32;
		minRayT = -PX_MAX_F32;
		maxRayT = PX_MAX_F32;
	}

	physx::PxVec3 rayorig;
	physx::PxVec3 raydir;

//	OverlapLineSegmentAABBCache segmentCache;
	if (pointSweep)
	{
//		computeOverlapLineSegmentAABBCache(segmentCache, worldDisplacement);
		rayorig = PXFROMNXVEC3(worldBox.center);
		raydir = PXFROMNXVEC3(worldDisp);
	}

	physx::PxF32 minTime = PX_MAX_F32;
	physx::PxU32 totalVisibleChunkCount;
	const physx::PxU16* visibleChunkIndices;
	physx::PxU16 dummyIndexArray = (physx::PxU16)parentChunkIndex;
	if (parentChunkIndex == NxModuleDestructibleConst::INVALID_CHUNK_INDEX)
	{
		totalVisibleChunkCount = mVisibleChunks.usedCount();
		visibleChunkIndices = mVisibleChunks.usedIndices();
	}
	else
	{
		totalVisibleChunkCount = 1;
		visibleChunkIndices = &dummyIndexArray;
	}

	IndexedReal* chunkProjectedMinima = (IndexedReal*)PxAlloca(totalVisibleChunkCount * sizeof(IndexedReal));
	physx::PxU32* chunkIndices = (physx::PxU32*)PxAlloca(totalVisibleChunkCount * sizeof(physx::PxU32));
	physx::PxMat34Legacy* chunkTMs = (physx::PxMat34Legacy*)PxAlloca(totalVisibleChunkCount * sizeof(physx::PxMat34Legacy));

#if USE_DESTRUCTIBLE_RWLOCK
	mLock->lockReader();
#endif
	{
		// Must find an intersecting visible chunk.
		PX_PROFILER_PERF_SCOPE("DestructibleRayCastFindVisibleChunk");

		// Find candidate chunks
		physx::PxU32 candidateChunkCount = 0;

		for (physx::PxU32 chunkNum = 0; chunkNum < totalVisibleChunkCount; ++chunkNum)
		{
			if (candidateChunkCount >= totalVisibleChunkCount)
			{
				PX_ALWAYS_ASSERT();
				break;
			}
			const physx::PxU32 chunkIndexInAsset = visibleChunkIndices[chunkNum];
			DestructibleStructure::Chunk& chunk = mStructure->chunks[chunkIndexInAsset + mFirstChunkIndex];
			DestructibleAssetParametersNS::Chunk_Type& chunkSource = mAsset->mParams->chunks.buf[chunkIndexInAsset];
			if (mDestructibleParameters.depthParameters[chunkSource.depth].ignoresRaycastCallbacks())
			{
				continue;
			}
			if ((chunk.state & ChunkVisible) == 0)
			{
				continue;	// In case a valid parentChunkIndex was passed in
			}
#if USE_CHUNK_RWLOCK
			DestructibleStructure::ChunkScopedReadLock chunkReadLock(chunk);
#endif
			if (chunk.isDestroyed())
			{
				continue;
			}

			if (dynamicStateFlags != NxDestructibleActorRaycastFlags::AllChunks)
			{
				if ((chunk.state & ChunkDynamic)==0)
				{
					if ((flags & NxDestructibleActorRaycastFlags::StaticChunks) == 0)
					{
						continue;
					}
				}
				else
				{
					if ((flags & NxDestructibleActorRaycastFlags::DynamicChunks) == 0)
					{
						continue;
					}
				}
			}

			physx::PxMat34Legacy& tm = chunkTMs[candidateChunkCount];
			tm = mStructure->getChunkGlobalPose(chunk); // Cache this off
			IndexedReal& chunkProjectedMin = chunkProjectedMinima[candidateChunkCount];

			if (sweep)
			{
				// Project chunk bounds along displacement direction
				physx::PxVec3 rM;
				tm.M.multiplyByTranspose(pxWorldDisp, rM);
				const physx::PxF32 chunkProjectedCenter = (rM.dot(PXFROMNXVEC3(chunk.localSphere.center))) + (pxWorldDisp.dot(tm.t));
				physx::PxVec3 chunkLocalExtents = mAsset->getChunkShapeLocalBounds(chunkIndexInAsset).getExtents();
				chunkLocalExtents = chunkLocalExtents.multiply(getScale());
				const physx::PxF32 chunkProjectedRadius = chunkLocalExtents.x * physx::PxAbs(rM.x) +
				        chunkLocalExtents.y * physx::PxAbs(rM.y) +
				        chunkLocalExtents.z * physx::PxAbs(rM.z);
				chunkProjectedMin.value = chunkProjectedCenter - chunkProjectedRadius;
				if (boxProjectedMin >= chunkProjectedCenter + chunkProjectedRadius || boxSweptMax <= chunkProjectedMin.value)
				{
					// Beyond or before projected sweep
					continue;
				}

				// Perform "corridor test"
				const physx::PxVec3 X = tm * PXFROMNXVEC3(chunk.localSphere.center) - PXFROMNXVEC3(worldBox.center);
				const physx::PxF32 sumRadius = boxBSphereRadius + chunk.localSphere.radius;
				const physx::PxF32 Xv = X.dot(pxWorldDisp);
				if (worldDisp2 * (X.magnitudeSquared() - sumRadius * sumRadius) >= Xv * Xv)
				{
					// Outside of corridor
					continue;
				}
			}
			else
			{
				// Overlap test
				const physx::PxVec3 X = tm * PXFROMNXVEC3(chunk.localSphere.center) - PXFROMNXVEC3(worldBox.center);
				const physx::PxF32 sumRadius = boxBSphereRadius + chunk.localSphere.radius;
				if (X.magnitudeSquared() >= sumRadius * sumRadius)
				{
					continue;
				}
			}

			// We'll keep this one
			chunkProjectedMin.index = candidateChunkCount;
			chunkIndices[candidateChunkCount++] = chunkIndexInAsset;
#if USE_CHUNK_RWLOCK
			chunk.lock->lockReader();	// Add another read lock, be sure to unlock after use in next loop
#endif
		}

		// Sort chunk bounds projected minima
		if (sweep && candidateChunkCount > 1)
		{
			combsort(chunkProjectedMinima, candidateChunkCount);
		}

		physx::PxU32 candidateChunkNum = 0;
		for (; candidateChunkNum < candidateChunkCount; ++candidateChunkNum)
		{
			IndexedReal& indexedChunkMin = chunkProjectedMinima[candidateChunkNum];
			if (sweep && minTime <= (indexedChunkMin.value - boxProjectedMax)*recipWorldDisp2)
			{
				// Early-out
				break;
			}
			physx::PxU16 chunkIndexInAsset = (physx::PxU16)chunkIndices[indexedChunkMin.index];
#if USE_CHUNK_RWLOCK
			DestructibleStructure::Chunk& chunk = mStructure->chunks[chunkIndexInAsset + mFirstChunkIndex];
#endif
			for (physx::PxU32 hullIndex = mAsset->getChunkHullIndexStart(chunkIndexInAsset); hullIndex < mAsset->getChunkHullIndexStop(chunkIndexInAsset); ++hullIndex)
			{
				ConvexHull& chunkSourceConvexHull = mAsset->chunkConvexHulls[hullIndex];
				physx::PxF32 in = minRayT;
				physx::PxF32 out = maxRayT;
				physx::PxVec3 n;
				physx::PxVec3 pxWorldBoxAxes[3];
				PxFromNxVec3(pxWorldBoxAxes[0], worldBoxAxes[0]);
				PxFromNxVec3(pxWorldBoxAxes[1], worldBoxAxes[1]);
				PxFromNxVec3(pxWorldBoxAxes[2], worldBoxAxes[2]);
				const bool hit = pointSweep ? (/* overlapLineSegmentAABBCached(rayorig, segmentCache, chunkSource.bounds) && */
				                     chunkSourceConvexHull.rayCast(in, out, rayorig, raydir, chunkTMs[indexedChunkMin.index], getScale(), &n)) :
				                 chunkSourceConvexHull.obbSweep(in, out, PXFROMNXVEC3(worldBox.center), PXFROMNXVEC3(worldBox.extents), pxWorldBoxAxes, pxWorldDisp, chunkTMs[indexedChunkMin.index], getScale(), &n);
				if (hit)
				{
					if (out > minRayT && in < minTime)
					{
						minTime = in;
						normal = n;
						parentChunkIndex = chunkIndexInAsset != (physx::PxU16)DestructibleAsset::InvalidChunkIndex ? chunkIndexInAsset : (physx::PxU16)NxModuleDestructibleConst::INVALID_CHUNK_INDEX;
					}
				}
			}
#if USE_CHUNK_RWLOCK
			chunk.lock->unlockReader();	// Releasing lock from end of previous loop
#endif
		}

#if USE_CHUNK_RWLOCK
		// Release remaining locks
		for (; candidateChunkNum < candidateChunkCount; ++candidateChunkNum)
		{
			IndexedReal& indexedChunkMin = chunkProjectedMinima[candidateChunkNum];
			physx::PxU16 chunkIndexInAsset = (physx::PxU16)chunkIndices[indexedChunkMin.index];
			DestructibleStructure::Chunk& chunk = mStructure->chunks[chunkIndexInAsset + mFirstChunkIndex];
			chunk.lock->unlockReader();
		}
#endif
	}

#if USE_DESTRUCTIBLE_RWLOCK
	mLock->unlockReader();
#endif

	if (minTime != PX_MAX_F32)
	{
		time = minTime;
	}

	bool accurateRaycasts = (mDestructibleParameters.flags & NxDestructibleParametersFlag::ACCURATE_RAYCASTS) != 0;

	if (((physx::PxU32)flags)&NxDestructibleActorRaycastFlags::ForceAccurateRaycastsOn)
	{
		accurateRaycasts = true;
	}

	if (((physx::PxU32)flags)&NxDestructibleActorRaycastFlags::ForceAccurateRaycastsOff)
	{
		accurateRaycasts = false;
	}

	if (!accurateRaycasts)
	{
		return parentChunkIndex;
	}

	physx::PxI32 chunkIndex = parentChunkIndex;

	{
		PX_PROFILER_PERF_SCOPE("DestructibleRayCastFindDeepestChunk");

#if NX_SDK_VERSION_MAJOR == 3
		SCOPED_PHYSX_LOCK_READ(*mDestructibleScene->mApexScene);
#endif

#if USE_DESTRUCTIBLE_RWLOCK
		mLock->lockReader();
#endif

		while (parentChunkIndex != NxModuleDestructibleConst::INVALID_CHUNK_INDEX)
		{
			DestructibleAssetParametersNS::Chunk_Type& source = mAsset->mParams->chunks.buf[parentChunkIndex];
			physx::PxF32 firstInTime = PX_MAX_F32;
			parentChunkIndex = NxModuleDestructibleConst::INVALID_CHUNK_INDEX;
			const physx::PxU16 childStop = source.numChildren;
			if (childStop > 0 && mDestructibleParameters.depthParameters[source.depth + 1].ignoresRaycastCallbacks())
			{
				// Children ignore raycasts.  We may stop here.
				break;
			}
			for (physx::PxU16 childNum = 0; childNum < childStop; ++childNum)
			{
				const physx::PxU16 childIndexInAsset = PxU16(source.firstChildIndex + childNum);
				DestructibleStructure::Chunk& child = mStructure->chunks[childIndexInAsset + mFirstChunkIndex];
#if USE_CHUNK_RWLOCK
				DestructibleStructure::ChunkScopedReadLock chunkReadLock(child);
#endif
				if (!child.isDestroyed())
				{
					for (physx::PxU32 hullIndex = mAsset->getChunkHullIndexStart(childIndexInAsset); hullIndex < mAsset->getChunkHullIndexStop(childIndexInAsset); ++hullIndex)
					{
						ConvexHull& childSourceConvexHull = mAsset->chunkConvexHulls[hullIndex];
						physx::PxF32 in = minRayT;
						physx::PxF32 out = maxRayT;
						physx::PxVec3 n;
						physx::PxVec3 pxWorldBoxAxes[3];
						PxFromNxVec3(pxWorldBoxAxes[0], worldBoxAxes[0]);
						PxFromNxVec3(pxWorldBoxAxes[1], worldBoxAxes[1]);
						PxFromNxVec3(pxWorldBoxAxes[2], worldBoxAxes[2]);
						const bool hit = pointSweep ? (/* overlapLineSegmentAABBCached(rayorig, segmentCache, childSource.bounds) && */
						                     childSourceConvexHull.rayCast(in, out, rayorig, raydir, mStructure->getChunkGlobalPose(child), getScale(), &n)) :
						                 childSourceConvexHull.obbSweep(in, out, PXFROMNXVEC3(worldBox.center), PXFROMNXVEC3(worldBox.extents), pxWorldBoxAxes, pxWorldDisp, mStructure->getChunkGlobalPose(child), getScale(), &n);
						if (hit)
						{
							if (out > minRayT && in < firstInTime)
							{
								firstInTime = in;
								normal = n;
								parentChunkIndex = childIndexInAsset != (physx::PxI32)DestructibleAsset::InvalidChunkIndex ? childIndexInAsset : (physx::PxI32)NxModuleDestructibleConst::INVALID_CHUNK_INDEX;
								if (child.state & ChunkVisible)
								{
									chunkIndex = parentChunkIndex;
								}
							}
						}
					}
				}
			}
			if (firstInTime != PX_MAX_F32)
			{
				time = firstInTime;
			}
		}

	}

#if USE_DESTRUCTIBLE_RWLOCK
	mLock->unlockReader();
#endif

	return chunkIndex;
}

physx::PxI32 DestructibleActor::pointOrOBBSweepStatic(physx::PxF32& time, physx::PxVec3& normal, const NxBox& worldBox, const physx::PxVec3& pxWorldDisp,
        NxDestructibleActorRaycastFlags::Enum flags, physx::PxI32 parentChunkIndex) const
{
	PX_PROFILER_PERF_SCOPE("DestructibleActorPointOrOBBSweepStatic");

	PX_ASSERT((flags & NxDestructibleActorRaycastFlags::DynamicChunks) == 0);

	// use the scene lock to protect chunk data for
	// multi-threaded obbSweep calls on destructible actors
	// (different lock would be good, but mixing locks can cause deadlocks)
	SCOPED_PHYSX_LOCK_READ(*mDestructibleScene->mApexScene);

	// parentChunkIndex out of range
	if (parentChunkIndex >= mAsset->mParams->chunks.arraySizes[0])
	{
		return NxModuleDestructibleConst::INVALID_CHUNK_INDEX;
	}

	DestructibleAssetParametersNS::Chunk_Type* sourceChunks = mAsset->mParams->chunks.buf;

	// parentChunkIndex is valid, but the chunk is invisible, beyond the LOD, or ignores raycasts
	if (parentChunkIndex >= 0)
	{
		if ((getStructure()->chunks[parentChunkIndex + getFirstChunkIndex()].state & ChunkVisible) == 0)
		{
			return NxModuleDestructibleConst::INVALID_CHUNK_INDEX;
		}
	}
	else
	{
		// From now on, we treat parentChunkIndex < 0 as parentChunkIndex = 0 and iterate
		parentChunkIndex = 0;
	}

	if (sourceChunks[parentChunkIndex].depth > (physx::PxU16)getLOD() || mDestructibleParameters.depthParameters[sourceChunks[parentChunkIndex].depth].ignoresRaycastCallbacks())
	{
		return NxModuleDestructibleConst::INVALID_CHUNK_INDEX;
	}

	// TODO: the normal is not always output, for instance, when accurateRaycasts is false, can we generate a decent normal in this case?
	const bool pointSweep = (worldBox.extents.magnitudeSquared() == 0.0f);

#if NX_SDK_VERSION_MAJOR == 2
	const NxVec3 worldBoxAxes[3] = { worldBox.rot.getColumn(0),  worldBox.rot.getColumn(1),  worldBox.rot.getColumn(2) };
#elif NX_SDK_VERSION_MAJOR == 3
	const NxVec3 worldBoxAxes[3] = { worldBox.rot.column0,  worldBox.rot.column1,  worldBox.rot.column2 };
#endif
	const NxVec3 worldDisp = NXFROMPXVEC3(pxWorldDisp);

	physx::PxF32 minRayT;
	physx::PxF32 maxRayT;
	if ((flags & NxDestructibleActorRaycastFlags::SegmentIntersect) != 0)
	{
		minRayT = 0.0f;
		maxRayT = 1.0f;
	}
	else
	{
		minRayT = -PX_MAX_F32;
		maxRayT = PX_MAX_F32;
	}

	physx::PxVec3 rayorig;
	physx::PxVec3 raydir;
	if (pointSweep)
	{
		rayorig = PXFROMNXVEC3(worldBox.center);
		raydir = PXFROMNXVEC3(worldDisp);
	}

	// Must find an intersecting visible chunk.

	physx::PxMat44 staticTM;
	bool success = getGlobalPoseForStaticChunks(staticTM);
	if (!success)
	{
		return NxModuleDestructibleConst::INVALID_CHUNK_INDEX;
	}
	physx::PxMat34Legacy chunkTM = staticTM;	// We'll keep updating the position for this one

	bool accurateRaycasts = (mDestructibleParameters.flags & NxDestructibleParametersFlag::ACCURATE_RAYCASTS) != 0;
	if (((physx::PxU32)flags)&NxDestructibleActorRaycastFlags::ForceAccurateRaycastsOn)
	{
		accurateRaycasts = true;
	}
	if (((physx::PxU32)flags)&NxDestructibleActorRaycastFlags::ForceAccurateRaycastsOff)
	{
		accurateRaycasts = false;
	}

	physx::PxI32 chunkFoundIndex = NxModuleDestructibleConst::INVALID_CHUNK_INDEX;
	physx::PxF32 minTime = PX_MAX_F32;
	physx::PxVec3 normalAtFirstHit(0.0f, 0.0f, 1.0f);

	// Start with the parentChunkIndex.  This stack is a stack of chunk ranges, stored LSW = low, MSW = high
	physx::Array<physx::PxU32> stack;
	stack.pushBack(((physx::PxU32)parentChunkIndex+1)<<16 | (physx::PxU32)parentChunkIndex);

	while (stack.size() > 0)
	{
		const physx::PxU32 range = stack.popBack();
		physx::PxU32 chunkIndex = (range&0xFFFF) + getFirstChunkIndex();
		physx::PxU32 chunkIndexStop = (range>>16) + getFirstChunkIndex();
		const physx::PxU16 depth = sourceChunks[range&0xFFFF].depth;
		const bool atLimit = depth >= getLOD() || depth >= mAsset->mParams->depthCount - 1 || mDestructibleParameters.depthParameters[depth+1].ignoresRaycastCallbacks();

		for (; chunkIndex < chunkIndexStop; ++chunkIndex)
		{
			const DestructibleStructure::Chunk& chunk = getStructure()->chunks[chunkIndex];
			if ((chunk.state & ChunkDynamic) != 0)
			{
				continue;
			}
			chunkTM.t = staticTM.transform(chunk.localOffset);
			const physx::PxU16 chunkIndexInAsset = (physx::PxU16)(chunkIndex - getFirstChunkIndex());
			for (physx::PxU32 hullIndex = mAsset->getChunkHullIndexStart(chunkIndexInAsset); hullIndex < mAsset->getChunkHullIndexStop(chunkIndexInAsset); ++hullIndex)
			{
				ConvexHull& chunkSourceConvexHull = mAsset->chunkConvexHulls[hullIndex];
				physx::PxF32 in = minRayT;
				physx::PxF32 out = maxRayT;
				physx::PxVec3 n;
				bool hit;
				if (pointSweep)
				{
					hit = chunkSourceConvexHull.rayCast(in, out, rayorig, raydir, chunkTM, getScale(), &n);
				}
				else
				{
					physx::PxVec3 pxWorldBoxAxes[3];
					PxFromNxVec3(pxWorldBoxAxes[0], worldBoxAxes[0]);
					PxFromNxVec3(pxWorldBoxAxes[1], worldBoxAxes[1]);
					PxFromNxVec3(pxWorldBoxAxes[2], worldBoxAxes[2]);
					hit = chunkSourceConvexHull.obbSweep(in, out, PXFROMNXVEC3(worldBox.center), PXFROMNXVEC3(worldBox.extents), pxWorldBoxAxes, pxWorldDisp, chunkTM, getScale(), &n);
				}

				if (hit)
				{
					if (out > 0.0f && in < minTime)
					{
						const physx::PxU32 firstChildIndexInAsset = (physx::PxU32)sourceChunks[chunkIndexInAsset].firstChildIndex;
						const physx::PxU32 childCount = (physx::PxU32)sourceChunks[chunkIndexInAsset].numChildren;
						const bool hasChildren = !atLimit && childCount > 0;

						if (!accurateRaycasts)
						{
							if (chunk.state & ChunkVisible)
							{
								minTime = in;
								normalAtFirstHit = n;
								chunkFoundIndex = (physx::PxI32)chunkIndex;
							}
							else
							if (chunk.isDestroyed() && hasChildren)
							{
								stack.pushBack(((physx::PxU32)firstChildIndexInAsset+childCount)<<16 | (physx::PxU32)firstChildIndexInAsset);
							}
						}
						else
						{
							if (!hasChildren)
							{
								minTime = in;
								normalAtFirstHit = n;
								chunkFoundIndex = (physx::PxI32)chunkIndex;
							}
							else
							{
								stack.pushBack(((physx::PxU32)firstChildIndexInAsset+childCount)<<16 | (physx::PxU32)firstChildIndexInAsset);
							}
						}
					}
				}
			}
		}
	}

	if (chunkFoundIndex == NxModuleDestructibleConst::INVALID_CHUNK_INDEX || minTime == PX_MAX_F32)
	{
		return NxModuleDestructibleConst::INVALID_CHUNK_INDEX;
	}

	time = minTime;
	normal = normalAtFirstHit;

	if (accurateRaycasts)
	{
		const DestructibleStructure::Chunk& chunkFound = getStructure()->chunks[(physx::PxU32)chunkFoundIndex];
		if ((chunkFound.state & ChunkVisible) == 0)
		{
			chunkFoundIndex = chunkFound.visibleAncestorIndex;
		}
	}

	return (physx::PxI32)(chunkFoundIndex - getFirstChunkIndex());
}

void DestructibleActor::applyDamage_immediate(DamageEvent& damageEvent)
{
	PX_PROFILER_PERF_SCOPE("DestructibleApplyDamage_immediate");

#if NX_SDK_VERSION_MAJOR == 3
	SCOPED_PHYSX_LOCK_READ(*mDestructibleScene->mApexScene);
#endif

	const physx::PxF32 paddingFactor = 0.01f;	// To do - expose ?  This is necessary now that we're using exact bounds testing.
	const physx::PxF32 padding = paddingFactor * (getOriginalBounds().maximum - getOriginalBounds().minimum).magnitude();

	const physx::PxU32 rayCastFlags = mDestructibleScene->m_damageApplicationRaycastFlags & NxDestructibleActorRaycastFlags::AllChunks;	// Mask against chunk flags

	physx::PxF32 time = 0;
	physx::PxVec3 fractureNormal(0.0f, 0.0f, 1.0f);

	physx::PxI32 chunkIndexInAsset = damageEvent.chunkIndexInAsset >= 0 ? damageEvent.chunkIndexInAsset : NxModuleDestructibleConst::INVALID_CHUNK_INDEX;

	if (rayCastFlags != 0)
	{
		const NxRay worldRay(NXFROMPXVEC3(damageEvent.position).isFinite() ? NXFROMPXVEC3(damageEvent.position) : NxVec3(0.0f), 
			NXFROMPXVEC3(damageEvent.direction).isFinite() && !NXFROMPXVEC3(damageEvent.direction).isZero() ? NXFROMPXVEC3(damageEvent.direction) : NxVec3(0.0f, 0.0f, 1.0f));
		// TODO, even the direction isn't always normalized - physx::PxVec3 fractureNormal(PXFROMNXVEC3(-worldRay.dir));
		const physx::PxI32 actualHitChunk = rayCast(time, fractureNormal, worldRay, (NxDestructibleActorRaycastFlags::Enum)rayCastFlags, chunkIndexInAsset);
		if (actualHitChunk != NxModuleDestructibleConst::INVALID_CHUNK_INDEX)
		{
			chunkIndexInAsset = actualHitChunk;
		}
	}
	else
	if (mDestructibleParameters.fractureImpulseScale != 0.0f)
	{
		// Unfortunately, we need to do *some* kind of bounds check to get a good outward-pointing normal, if mDestructibleParameters.fractureImpulseScale != 0
		// We'll make this as inexpensive as possible, and raycast against the depth 0 chunk
		physx::PxMat34Legacy tm = getInitialGlobalPose();
		const physx::PxVec3 pos = damageEvent.position.isFinite() ? damageEvent.position : physx::PxVec3(0.0f);
		const physx::PxVec3 dir = damageEvent.direction.isFinite() ? damageEvent.direction : physx::PxVec3(0.0f, 0.0f, 1.0f);
		for (physx::PxU32 hullIndex = mAsset->getChunkHullIndexStart(0); hullIndex < mAsset->getChunkHullIndexStop(0); ++hullIndex)
		{
			ConvexHull& chunkSourceConvexHull = mAsset->chunkConvexHulls[hullIndex];
			physx::PxF32 in = -PX_MAX_F32;
			physx::PxF32 out = PX_MAX_F32;
			physx::PxF32 minTime = PX_MAX_F32;
			physx::PxVec3 n;
			if (chunkSourceConvexHull.rayCast(in, out, pos, dir, tm, getScale(), &n))
			{
				if (in < minTime)
				{
					minTime = in;
					fractureNormal = n;
				}
			}
		}
	}

	PX_ASSERT(fractureNormal.isNormalized());

	if (chunkIndexInAsset == NxModuleDestructibleConst::INVALID_CHUNK_INDEX)
	{
		PX_ASSERT(0 == (DamageEvent::UseRadius & damageEvent.flags));
		damageEvent.flags |= DamageEvent::Invalid;
		return;
	}

	damageEvent.chunkIndexInAsset = chunkIndexInAsset;
	damageEvent.position += damageEvent.direction * time;

	physx::PxF32 damage = damageEvent.damage;
	if (mDestructibleParameters.damageCap > 0)
	{
		damage = physx::PxMin(damage, mDestructibleParameters.damageCap);
	}

	DestructibleStructure::Chunk& chunk = mStructure->chunks[damageEvent.chunkIndexInAsset + mFirstChunkIndex];
	if (!(chunk.state & ChunkVisible))
	{
		return;
	}

	// For probabilistic chunk deletion
	physx::PxU32 possibleDeleteChunks = 0;
	physx::PxF32 totalDeleteChunkRelativeDamage = 0.0f;

	physx::PxU32 totalFractureCount = 0;
	NxActor& actor = *(NxActor*)mStructure->getChunkActor(chunk);
	for (physx::PxU32 i = 0; i < actor.getNbShapes(); ++i)
	{
		NxShape* shape = getShape(actor, i);
		DestructibleStructure::Chunk* chunk = mStructure->dscene->getChunk(shape);
		if (chunk != NULL && chunk->isFirstShape(shape))	// BRG OPTIMIZE
		{
			PX_ASSERT(mStructure->dscene->mDestructibles.direct(chunk->destructibleID)->mStructure == mStructure);
			if ((chunk->state & ChunkVisible) != 0)
			{
				// Damage coloring:
				DestructibleActor* destructible = mStructure->dscene->mDestructibles.direct(chunk->destructibleID);
				if (destructible->useDamageColoring())
				{
					if (destructible->applyDamageColoring(chunk->indexInAsset, damageEvent.position, damage, 0.0f))
					{
						destructible->collectDamageColoring(chunk->indexInAsset, damageEvent.position, damage, 0.0f);
					}
				}

				// specify the destructible asset because we're in a structure, it may not be this actor's asset
				DestructibleAssetParametersNS::Chunk_Type& source = destructible->getAsset()->mParams->chunks.buf[chunk->indexInAsset];

				const physx::PxU16 stopDepth = destructible->getParams()->destructibleParameters.damageDepthLimit < PX_MAX_U16 - source.depth ?
					destructible->getParams()->destructibleParameters.damageDepthLimit + source.depth : PX_MAX_U16;

				totalFractureCount += mStructure->damageChunk(*chunk, damageEvent.position, damageEvent.direction, damageEvent.isFromImpact(), damage, 0.0f,
									  damageEvent.fractures, possibleDeleteChunks, totalDeleteChunkRelativeDamage, damageEvent.maxDepth, (physx::PxU32)source.depth, stopDepth, padding);
			}
		}
	}

	// For probabilistic chunk deletion
	physx::PxF32 deletionFactor = getDestructibleParameters().debrisDestructionProbability;
	if (totalDeleteChunkRelativeDamage > 0.0f)
	{
		deletionFactor *= (physx::PxF32)possibleDeleteChunks/totalDeleteChunkRelativeDamage;
	}

	for (physx::PxU32 depth = 0; depth <= damageEvent.maxDepth; ++depth)
	{
		physx::Array<FractureEvent>& fractureEventBuffer = damageEvent.fractures[depth];
		for (physx::PxU32 i = 0; i < fractureEventBuffer.size(); ++i)
		{
			FractureEvent& fractureEvent = fractureEventBuffer[i];
			DestructibleActor* destructible = mStructure->dscene->mDestructibles.direct(fractureEvent.destructibleID);
			physx::PxU32 affectedIndex = fractureEvent.chunkIndexInAsset + destructible->mFirstChunkIndex;
			DestructibleStructure::Chunk& affectedChunk = mStructure->chunks[affectedIndex];
			if (!affectedChunk.isDestroyed())
			{
				const physx::PxF32 deletionProbability = deletionFactor*fractureEvent.deletionWeight;
				if (deletionProbability > 0.0f && mStructure->dscene->mModule->mRandom.getUnit() < deletionProbability)
				{
					fractureEvent.flags |= FractureEvent::CrumbleChunk;
				}

				// Compute impulse
				fractureEvent.impulse = mStructure->getChunkWorldCentroid(affectedChunk) - damageEvent.position;
				fractureEvent.impulse.normalize();
				fractureEvent.impulse *= fractureEvent.damageFraction * damageEvent.momentum;
				fractureEvent.impulse += fractureNormal * mDestructibleParameters.fractureImpulseScale;
				fractureEvent.position = damageEvent.position;
				if (damageEvent.isFromImpact())
				{
					fractureEvent.flags |= FractureEvent::DamageFromImpact;
				}
				if (0 != (DamageEvent::SyncDirect & damageEvent.flags))
				{
					PX_ASSERT(0 == (FractureEvent::SyncDerived & fractureEvent.flags));
					fractureEvent.flags |= FractureEvent::SyncDerived;
				}
			}
		}
	}
}

void DestructibleActor::applyRadiusDamage_immediate(DamageEvent& damageEvent)
{
	PX_PROFILER_PERF_SCOPE("DestructibleApplyRadiusDamage_immediate");

#if NX_SDK_VERSION_MAJOR == 3
	SCOPED_PHYSX_LOCK_READ(*mDestructibleScene->mApexScene);
#endif

	const physx::PxF32 damageCap = mDestructibleParameters.damageCap > 0 ? mDestructibleParameters.damageCap : PX_MAX_F32;

	physx::PxU32 totalFractureCount = 0;

	const physx::PxF32 paddingFactor = 0.01f;	// To do - expose ?  This is necessary now that we're using exact bounds testing.
	const physx::PxF32 padding = paddingFactor * (getOriginalBounds().maximum - getOriginalBounds().minimum).magnitude();

	// For probabilistic chunk deletion
	physx::PxU32 possibleDeleteChunks = 0;
	physx::PxF32 totalDeleteChunkRelativeDamage = 0.0f;

	const bool useLegacyDamageRadiusSpread = getUseLegacyDamageRadiusSpread();

	// Should use scene query here
	const physx::PxU16* chunkIndexPtr = mVisibleChunks.usedIndices();
	const physx::PxU16* chunkIndexPtrStop = chunkIndexPtr + mVisibleChunks.usedCount();
	bool needSaveDamageColor = false;
	while (chunkIndexPtr < chunkIndexPtrStop)
	{
		physx::PxU16 chunkIndex = *chunkIndexPtr++;
		DestructibleStructure::Chunk& chunk = mStructure->chunks[chunkIndex + mFirstChunkIndex];

		// Legacy behavior
		physx::PxF32 minRadius = 0.0f;
		physx::PxF32 maxRadius = damageEvent.radius;
		physx::PxF32 falloff = 1.0f;
		const DestructibleActorParamNS::BehaviorGroup_Type& behaviorGroup = getBehaviorGroup(chunkIndex);
		if (!useLegacyDamageRadiusSpread)
		{
			// New behavior
			minRadius = behaviorGroup.damageSpread.minimumRadius;
			maxRadius = minRadius + damageEvent.radius*behaviorGroup.damageSpread.radiusMultiplier;
			falloff = behaviorGroup.damageSpread.falloffExponent;
		}

		// Damage coloring:
		if (useDamageColoring())
		{
			if (applyDamageColoring(chunk.indexInAsset, damageEvent.position, damageEvent.damage, damageEvent.radius))
			{
				needSaveDamageColor = true;
			}
		}

		const physx::PxVec3 chunkCentroid = mStructure->getChunkWorldCentroid(chunk);
		physx::PxVec3 dir = chunkCentroid - damageEvent.position;
		physx::PxF32 dist = dir.normalize() - chunk.localSphere.radius;
		if (dist < maxRadius)
		{
			physx::PxF32 damageFraction = 1;
			if (useLegacyDamageRadiusSpread)
			{
				dist = physx::PxMax(dist, 0.0f);
				if (falloff && damageEvent.radius > 0.0f)
				{
					damageFraction -= dist / damageEvent.radius;
				}
			}
			const physx::PxF32 effectiveDamage = physx::PxMin(damageEvent.damage * damageFraction, damageCap);
			DestructibleAssetParametersNS::Chunk_Type& source = mAsset->mParams->chunks.buf[chunk.indexInAsset];

			const physx::PxU16 stopDepth = getParams()->destructibleParameters.damageDepthLimit < PX_MAX_U16 - source.depth ?
				getParams()->destructibleParameters.damageDepthLimit + source.depth : PX_MAX_U16;

			totalFractureCount += mStructure->damageChunk(chunk, damageEvent.position, damageEvent.direction, damageEvent.isFromImpact(), effectiveDamage, damageEvent.radius,
			                      damageEvent.fractures, possibleDeleteChunks, totalDeleteChunkRelativeDamage, damageEvent.maxDepth, (physx::PxU32)source.depth, stopDepth, padding);
		}
	}

	if (needSaveDamageColor)
	{
		collectDamageColoring(NxModuleDestructibleConst::INVALID_CHUNK_INDEX, damageEvent.position, damageEvent.damage, damageEvent.radius);
	}

	// For probabilistic chunk deletion
	physx::PxF32 deletionFactor = getDestructibleParameters().debrisDestructionProbability;
	if (totalDeleteChunkRelativeDamage > 0.0f)
	{
		deletionFactor *= (physx::PxF32)possibleDeleteChunks/totalDeleteChunkRelativeDamage;
	}

	for (physx::PxU32 depth = 0; depth <= damageEvent.maxDepth; ++depth)
	{
		physx::Array<FractureEvent>& fractureEventBuffer = damageEvent.fractures[depth];
		for (physx::PxU32 i = 0; i < fractureEventBuffer.size(); ++i)
		{
			FractureEvent& fractureEvent = fractureEventBuffer[i];
			DestructibleActor* destructible = mStructure->dscene->mDestructibles.direct(fractureEvent.destructibleID);
			physx::PxU32 affectedIndex = fractureEvent.chunkIndexInAsset + destructible->mFirstChunkIndex;
			DestructibleStructure::Chunk& affectedChunk = mStructure->chunks[affectedIndex];
			if (!affectedChunk.isDestroyed())
			{
				const physx::PxF32 deletionProbability = deletionFactor*fractureEvent.deletionWeight;
				if (deletionProbability > 0.0f && mStructure->dscene->mModule->mRandom.getUnit() < deletionProbability)
				{
					fractureEvent.flags |= FractureEvent::CrumbleChunk;
				}

				fractureEvent.impulse = mStructure->getChunkWorldCentroid(affectedChunk) - damageEvent.position;
				fractureEvent.impulse.normalize();
				fractureEvent.impulse *= fractureEvent.damageFraction * damageEvent.momentum;

				// Get outward normal for fractureImpulseScale, if this chunk is part of a larger island
				physx::PxVec3 fractureNormal(0.0f);
				if (!mStructure->chunkIsSolitary(affectedChunk))
				{
					NxActor& chunkActor = *(NxActor*)mStructure->getChunkActor(affectedChunk);
					// Search neighbors
					DestructibleActor* destructible = mStructure->dscene->mDestructibles.direct(affectedChunk.destructibleID);
					const physx::PxVec3 chunkPos = destructible->getChunkPose(affectedChunk.indexInAsset).getPosition().multiply(destructible->getScale());
					const physx::PxU32 indexInStructure = affectedChunk.indexInAsset + destructible->getFirstChunkIndex();
					for (physx::PxU32 overlapN = mStructure->firstOverlapIndices[indexInStructure]; overlapN < mStructure->firstOverlapIndices[indexInStructure + 1]; ++overlapN)
					{
						DestructibleStructure::Chunk& overlapChunk = mStructure->chunks[mStructure->overlaps[overlapN]];
						if (!overlapChunk.isDestroyed() && (NxActor*)mStructure->getChunkActor(overlapChunk) == &chunkActor)
						{
							DestructibleActor* overlapDestructible = mStructure->dscene->mDestructibles.direct(overlapChunk.destructibleID);
							fractureNormal += chunkPos - overlapDestructible->getChunkPose(overlapChunk.indexInAsset).getPosition().multiply(overlapDestructible->getScale());
						}
					}
					if (fractureNormal.magnitudeSquared() != 0.0f)
					{
						fractureNormal.normalize();
						fractureEvent.impulse += fractureNormal * mDestructibleParameters.fractureImpulseScale;
					}
				}

				fractureEvent.position = damageEvent.position;
				if (damageEvent.isFromImpact())
				{
					fractureEvent.flags |= FractureEvent::DamageFromImpact;
				}
				if (0 != (DamageEvent::SyncDirect & damageEvent.flags))
				{
					PX_ASSERT(0 == (FractureEvent::SyncDerived & fractureEvent.flags));
					fractureEvent.flags |= FractureEvent::SyncDerived;
				}
			}
		}
	}
}

void DestructibleActor::setSkinnedOverrideMaterial(PxU32 index, const char* overrideMaterialName)
{
	NxRenderMeshActor* rma = getRenderMeshActor(NxDestructibleActorMeshType::Skinned);
	if (rma != NULL)
	{
		rma->setOverrideMaterial(index, overrideMaterialName);
	}
}

void DestructibleActor::setStaticOverrideMaterial(PxU32 index, const char* overrideMaterialName)
{
	NxRenderMeshActor* rma = getRenderMeshActor(NxDestructibleActorMeshType::Static);
	if (rma != NULL)
	{
		rma->setOverrideMaterial(index, overrideMaterialName);
	}
}

void DestructibleActor::setRuntimeFracturePattern(const char* /*fracturePatternName*/)
{
	// TODO: Implement
	/*
	NiResourceProvider* nrp = NiGetApexSDK()->getInternalResourceProvider();
	if (nrp != NULL)
	{
		// do create before release, so we don't release the resource if the newID is the same as the old
		NxResID patternNS = NiGetApexSDK()->getPatternNamespace();

		NxResID newID = nrp->createResource(patternNS, fracturePatternName);
		nrp->releaseResource(mFracturePatternID);

		mFracturePatternID = newID;
		// TODO: Aquire resource for fracture pattern
	}
	*/
}

void DestructibleActor::updateRenderResources(bool rewriteBuffers, void* userRenderData)
{

	PX_PROFILER_PERF_SCOPE("DestructibleUpdateRenderResources");

	//Should not be necessary anymore
	//#if APEX_RUNTIME_FRACTURE
	//	{
	//		mRTActor->updateRenderResources(rewriteBuffers, userRenderData);
	//	}
	//#endif

	// Render non-instanced meshes if we own them
	if (mRenderable != NULL && mRenderable->getReferenceCount() == 1)
	{
		mRenderable->updateRenderResources(rewriteBuffers, userRenderData);
	}
}

void DestructibleActor::dispatchRenderResources(NxUserRenderer& renderer)
{
	PX_PROFILER_PERF_SCOPE("DestructibleDispatchRenderResources");

	// Dispatch non-instanced render resources if we own them
	if (mRenderable != NULL && mRenderable->getReferenceCount() == 1)
	{
		mRenderable->dispatchRenderResources(renderer);
	}

	// Should not be necessary anymore
	//#if APEX_RUNTIME_FRACTURE
	//	mRTActor->dispatchRenderResources(renderer);
	//#endif
}

void DestructibleActor::fillInstanceBuffers()
{
	if (mAsset->mParams->chunkInstanceInfo.arraySizes[0] == 0 && mAsset->mParams->scatterMeshIndices.arraySizes[0] == 0)
	{
		return;	// No instancing for this asset
	}

	PX_PROFILER_PERF_SCOPE("DestructibleActor::fillInstanceBuffers");

	mInstancedBounds.setEmpty();

	DestructibleAssetParametersNS::Chunk_Type* sourceChunks = mAsset->mParams->chunks.buf;

	Mutex::ScopedLock scopeLock(mAsset->m_chunkInstanceBufferDataLock);

	const physx::PxF32 scatterAlpha = physx::PxClamp(2.0f-getLOD(), 0.0f, 1.0f);

	// Iterate over all visible chunks
	const physx::PxU16* indexPtr = mVisibleChunks.usedIndices();
	const physx::PxU16* indexPtrStop = indexPtr + mVisibleChunks.usedCount();
	DestructibleAssetParametersNS::InstanceInfo_Type* instanceDataArray = mAsset->mParams->chunkInstanceInfo.buf;
	while (indexPtr < indexPtrStop)
	{
		const physx::PxU16 index = *indexPtr++;
		if (index < mAsset->getChunkCount())
		{
			DestructibleAssetParametersNS::Chunk_Type& sourceChunk = sourceChunks[index];
			physx::PxMat44 pose = getChunkPose(index);

			const physx::PxMat33 poseScaledRotation = physx::PxMat33(getScale().x*pose.getBasis(0), getScale().y*pose.getBasis(1), getScale().z*pose.getBasis(2));

			// Instanced chunks
			if ((sourceChunk.flags & DestructibleAsset::Instanced) != 0)
			{
				PX_ASSERT(sourceChunk.meshPartIndex < mAsset->mParams->chunkInstanceInfo.arraySizes[0]);
				DestructibleAsset::ChunkInstanceBufferDataElement instanceDataElement;
				const DestructibleAssetParametersNS::InstanceInfo_Type& instanceData = instanceDataArray[sourceChunk.meshPartIndex];
				const physx::PxU16 instancedActorIndex = mAsset->m_instancedChunkActorMap[sourceChunk.meshPartIndex];
				Array<DestructibleAsset::ChunkInstanceBufferDataElement>& instanceBufferData = mAsset->m_chunkInstanceBufferData[instancedActorIndex];
				instanceDataElement.translation = pose.getPosition();// + poseScaledRotation*instanceData.chunkPositionOffset;
				instanceDataElement.scaledRotation = poseScaledRotation;
				instanceDataElement.uvOffset = instanceData.chunkUVOffset;
				instanceDataElement.localOffset = physx::PxVec3(getScale().x*instanceData.chunkPositionOffset.x, getScale().y*instanceData.chunkPositionOffset.y, getScale().z*instanceData.chunkPositionOffset.z);
				
				 // there shouldn't be any allocation here because of the reserve in DestructibleAsset::resetInstanceData
				PX_ASSERT(instanceBufferData.size() < instanceBufferData.capacity());
				instanceBufferData.pushBack(instanceDataElement);

				const PxBounds3& partBounds = mAsset->renderMeshAsset->getBounds(instanceData.partIndex);
				// Transform bounds
				physx::PxVec3 center, extents;
				center = partBounds.getCenter();
				extents = partBounds.getExtents();
				center = poseScaledRotation.transform(center) + instanceDataElement.translation;
				extents = physx::PxVec3(physx::PxAbs(poseScaledRotation(0, 0) * extents.x) + physx::PxAbs(poseScaledRotation(0, 1) * extents.y) + physx::PxAbs(poseScaledRotation(0, 2) * extents.z),
										physx::PxAbs(poseScaledRotation(1, 0) * extents.x) + physx::PxAbs(poseScaledRotation(1, 1) * extents.y) + physx::PxAbs(poseScaledRotation(1, 2) * extents.z),
										physx::PxAbs(poseScaledRotation(2, 0) * extents.x) + physx::PxAbs(poseScaledRotation(2, 1) * extents.y) + physx::PxAbs(poseScaledRotation(2, 2) * extents.z));
				mInstancedBounds.include(physx::PxBounds3::centerExtents(center, extents));
			}

			// Scatter meshes
			if (scatterAlpha > 0.0f)
			{
				const physx::PxU16 scatterMeshStop = PxU16(sourceChunk.firstScatterMesh + sourceChunk.scatterMeshCount);
				for (physx::PxU16 scatterMeshNum = sourceChunk.firstScatterMesh; scatterMeshNum < scatterMeshStop; ++scatterMeshNum)
				{
					const physx::PxU8 scatterMeshIndex = mAsset->mParams->scatterMeshIndices.buf[scatterMeshNum];
					const DestructibleAssetParametersNS::M34_Type& scatterMeshTransform = mAsset->mParams->scatterMeshTransforms.buf[scatterMeshNum];
					DestructibleAsset::ScatterMeshInstanceInfo& scatterMeshInstanceInfo = mAsset->m_scatterMeshInstanceInfo[scatterMeshIndex];
					Array<DestructibleAsset::ScatterInstanceBufferDataElement>& instanceBufferData = scatterMeshInstanceInfo.m_instanceBufferData;
					DestructibleAsset::ScatterInstanceBufferDataElement instanceDataElement;
					instanceDataElement.translation = poseScaledRotation*scatterMeshTransform.vector + pose.getPosition();// + poseScaledRotation*instanceData.chunkPositionOffset;
					instanceDataElement.scaledRotation = poseScaledRotation*scatterMeshTransform.matrix;
					instanceDataElement.alpha = scatterAlpha;
					instanceBufferData.pushBack(instanceDataElement);
					// Not updating bounds for scatter meshes
				}
			}
		}
	}

	mRenderBounds = mNonInstancedBounds;
	mRenderBounds.include(mInstancedBounds);
	if (mRenderable != NULL)
	{
		mRenderable->setBounds(mRenderBounds);
	}
}

void DestructibleActor::setRelativeTMs()
{
	mRelTM = NULL != mStructure ? mTM*mStructure->getActorForStaticChunksPose().inverseRT() : physx::PxMat44::createIdentity();
}

/* Called during ApexScene::fetchResults(), after PhysXScene::fetchResults().  Sends new bone poses
 * to ApexRenderMeshActor, then retrieves new world AABB.
 */

void DestructibleActor::setRenderTMs(bool processChunkPoseForSyncing /*= false*/)
{
	mNonInstancedBounds.setEmpty();

	if (mRenderable)
	{
		mRenderable->lockRenderResources();
	}

	NxRenderMeshActor* skinnedRMA = getRenderMeshActor(NxDestructibleActorMeshType::Skinned);

	const bool syncWriteTM = processChunkPoseForSyncing && mSyncParams.isSyncFlagSet(NxDestructibleActorSyncFlags::CopyChunkTransform) && (NULL != mSyncParams.getChunkSyncState());
	const bool canSyncReadTM = processChunkPoseForSyncing && mSyncParams.isSyncFlagSet(NxDestructibleActorSyncFlags::ReadChunkTransform);

	if (skinnedRMA != NULL || syncWriteTM || canSyncReadTM)
	{
		if (skinnedRMA != NULL)
		{
			skinnedRMA->syncVisibility(false);	// Using mRenderable->mRenderable->lockRenderResources() instead
		}
		//	PX_ASSERT(asset->getRenderMeshAsset()->getPartCount() == asset->getChunkCount());

		DestructibleAssetParametersNS::Chunk_Type* sourceChunks = mAsset->mParams->chunks.buf;

		// Iterate over all visible chunks
		const physx::PxU16* indexPtr = mVisibleChunks.usedIndices();
		const physx::PxU16* indexPtrStop = indexPtr + mVisibleChunks.usedCount();

		// TODO: Here we update all chunks although we practically know the chunks (active transforms) that have moved. Improve. [APEX-670]

		while (indexPtr < indexPtrStop)
		{
			const physx::PxU16 index = *indexPtr++;
			if (index < mAsset->getChunkCount())
			{
				DestructibleAssetParametersNS::Chunk_Type& sourceChunk = sourceChunks[index];
				DestructibleStructure::Chunk & chunk = mStructure->chunks[mFirstChunkIndex + index];
				if (!chunk.isDestroyed() && (sourceChunk.flags & DestructibleAsset::Instanced) == 0)
				{
					const bool syncReadTM = canSyncReadTM && (NULL != chunk.controlledChunk);
					if (syncReadTM && !getDynamic(index))
					{
						setDynamic(index, true);
					}

					if (getDynamic(index) || !drawStaticChunksInSeparateMesh() || initializedFromState())
					{
						physx::PxMat44 chunkPose = physx::PxMat44::createZero();
						if (processChunkPoseForSyncing)
						{
							if (syncWriteTM)
							{
								const NxDestructibleChunkSyncState & chunkSyncState = *(mSyncParams.getChunkSyncState());
								PX_ASSERT(NULL != &chunkSyncState);
								if(!chunkSyncState.disableTransformBuffering &&
#if NX_SDK_VERSION_MAJOR == 2
									(chunkSyncState.excludeSleepingChunks ? !mStructure->getChunkActor(chunk)->isSleeping() : true) &&
#elif NX_SDK_VERSION_MAJOR == 3
									(chunkSyncState.excludeSleepingChunks ? !isNxActorSleeping(*(mStructure->getChunkActor(chunk))) : true) &&
#endif // NX_SDK_VERSION_MAJOR
									(chunkSyncState.chunkTransformCopyDepth >= sourceChunk.depth))
								{
#if NX_SDK_VERSION_MAJOR == 3
									SCOPED_PHYSX_LOCK_READ(*mDestructibleScene->mApexScene);
#endif
									const physx::PxMat44 calculatedChunkPose = getChunkPose(index);
									mSyncParams.pushCachedChunkTransform(CachedChunk(index, calculatedChunkPose));
									chunkPose = calculatedChunkPose;
								}
							}
							if (syncReadTM)
							{
								const physx::PxMat44 controlledChunkPose = physx::PxMat44(physx::PxMat33(chunk.controlledChunk->chunkOrientation), chunk.controlledChunk->chunkPosition);
								{
#if NX_SDK_VERSION_MAJOR == 3
									SCOPED_PHYSX_LOCK_WRITE(*getDestructibleScene()->getApexScene());
#endif
									setChunkPose(index, controlledChunkPose);
								}
								chunk.controlledChunk = NULL;
								chunkPose = controlledChunkPose;
							}
							if (chunkPose.column3.w < FLT_EPSILON)
							{
#if NX_SDK_VERSION_MAJOR == 3
								SCOPED_PHYSX_LOCK_READ(*mDestructibleScene->mApexScene);
#endif
								chunkPose = getChunkPose(index);
							}
						}
						else
						{
#if NX_SDK_VERSION_MAJOR == 3
							SCOPED_PHYSX_LOCK_READ(*mDestructibleScene->mApexScene);
#endif
							chunkPose = getChunkPose(index);
						}
						PX_ASSERT(chunkPose.column3.w > FLT_EPSILON);
						if (skinnedRMA != NULL)
						{
							skinnedRMA->setTM(chunkPose, getScale(), sourceChunk.meshPartIndex);
						}
					}
				}
			}
		}


		if (skinnedRMA != NULL)
		{
			skinnedRMA->updateBounds();
			mNonInstancedBounds.include(skinnedRMA->getBounds());
		}
	}

	NxRenderMeshActor* staticRMA = getRenderMeshActor(NxDestructibleActorMeshType::Static);

	// If a static mesh exists, set its (single) tm from the destructible's tm
	if (staticRMA != NULL)
	{
		staticRMA->syncVisibility(false);	// Using mRenderable->mRenderable->lockRenderResources() instead
		physx::PxMat44 pose;
		if (!getGlobalPoseForStaticChunks(pose))
		{
			pose = physx::PxMat44::createIdentity();	// Should not be rendered, but just in case we'll set pose to something sane.
		}
		staticRMA->setTM(pose, getScale());
		staticRMA->updateBounds();
		mNonInstancedBounds.include(staticRMA->getBounds());
	}

#if APEX_RUNTIME_FRACTURE
	if(mRenderable)
	{
		mRenderable->getRTrenderable().updateRenderCache(mRTActor);
	}
#endif

	if (mNonInstancedBounds.isEmpty())	// This can occur if we have no renderable
	{
		const physx::PxU16* indexPtr = mVisibleChunks.usedIndices();
		const physx::PxU16* indexPtrStop = indexPtr + mVisibleChunks.usedCount();
		while (indexPtr < indexPtrStop)
		{
			const physx::PxU16 index = *indexPtr++;
			if (index < mAsset->getChunkCount())
			{
				const physx::PxMat44 pose = getChunkPose(index);
				for (physx::PxU32 hullIndex = mAsset->getChunkHullIndexStart(index); hullIndex < mAsset->getChunkHullIndexStop(index); ++hullIndex)
				{
					const ConvexHull& chunkSourceConvexHull = mAsset->chunkConvexHulls[hullIndex];
					physx::PxBounds3 chunkConvexHullBounds = chunkSourceConvexHull.getBounds();
					// Apply scale
					chunkConvexHullBounds.minimum = chunkConvexHullBounds.minimum.multiply(getScale());
					chunkConvexHullBounds.maximum = chunkConvexHullBounds.maximum.multiply(getScale());
					// Apply rotation and translation
					const physx::PxVec3 extent = chunkConvexHullBounds.getExtents();
					const PxVec3 newExtent(
						PxAbs(pose.column0.x*extent.x) + PxAbs(pose.column1.x*extent.y) + PxAbs(pose.column2.x*extent.z),
						PxAbs(pose.column0.y*extent.x) + PxAbs(pose.column1.y*extent.y) + PxAbs(pose.column2.y*extent.z),
						PxAbs(pose.column0.z*extent.x) + PxAbs(pose.column1.z*extent.y) + PxAbs(pose.column2.z*extent.z));
					const physx::PxVec3 center = pose.transform(chunkConvexHullBounds.getCenter());
					mNonInstancedBounds.include(PxBounds3(center - newExtent, center + newExtent));
				}
			}
		}
	}

	mRenderBounds = mNonInstancedBounds;
	mRenderBounds.include(mInstancedBounds);
	if (mRenderable != NULL)
	{
		mRenderable->unlockRenderResources();
		mRenderable->setBounds(mRenderBounds);
	}
}

void DestructibleActor::setActorObjDescFlags(NiApexPhysXObjectDesc* actorObjDesc, physx::PxU32 depth) const
{
	const NxDestructibleDepthParameters& depthParameters = mDestructibleParameters.depthParameters[depth];
	actorObjDesc->setIgnoreTransform(depthParameters.ignoresPoseUpdates());
	actorObjDesc->setIgnoreRaycasts(depthParameters.ignoresRaycastCallbacks());
	actorObjDesc->setIgnoreContacts(depthParameters.ignoresContactCallbacks());
	for (physx::PxU32 i = PhysXActorFlags::DEPTH_PARAM_USER_FLAG_0; i <= PhysXActorFlags::DEPTH_PARAM_USER_FLAG_3; ++i)
	{
		actorObjDesc->setUserDefinedFlag(i, depthParameters.hasUserFlagSet(i));
	}
	actorObjDesc->setUserDefinedFlag(PhysXActorFlags::CREATED_THIS_FRAME, true);
	actorObjDesc->setUserDefinedFlag(PhysXActorFlags::IS_SLEEPING, true);
}

void DestructibleActor::setGlobalPose(const physx::PxMat44& pose)
{
	if (!isChunkDestroyed(0) && getDynamic(0))
	{
		setChunkPose(0, pose);
	}
	else
	{
		setGlobalPoseForStaticChunks(pose);
	}
}

void DestructibleActor::setGlobalPoseForStaticChunks(const physx::PxMat44& pose)
{
	if (mStructure != NULL && mStructure->actorForStaticChunks != NULL)
	{
		const physx::PxMat44 actorForStaticChunkPose = mRelTM.inverseRT() * pose;

#if NX_SDK_VERSION_MAJOR == 2
		PxMat34Legacy pose34Legacy(actorForStaticChunkPose);
		NxMat34 pose34;
		NxFromPxMat34(pose34, pose34Legacy);
		mStructure->actorForStaticChunks->moveGlobalPose(pose34);
#elif NX_SDK_VERSION_MAJOR == 3
		mStructure->actorForStaticChunks->moveKinematic(physx::PxTransform(actorForStaticChunkPose));
#endif

		for (physx::PxU32 meshType = 0; meshType < NxDestructibleActorMeshType::Count; ++meshType)
		{
			NxRenderMeshActor* renderMeshActor = getRenderMeshActor((NxDestructibleActorMeshType::Enum)meshType);
			if (renderMeshActor != NULL)
			{
				renderMeshActor->updateBounds();
			}
		}

	}

	wakeForEvent();
}

bool DestructibleActor::getGlobalPoseForStaticChunks(physx::PxMat44& pose) const
{
	if (mStructure != NULL && mStructure->actorForStaticChunks != NULL)
	{
		if (NULL != mStructure->actorForStaticChunks)
			pose = mRelTM * mStructure->getActorForStaticChunksPose();
		else
			pose = mTM;
		return true;
	}

	return false;
}

void DestructibleActor::setChunkPose(physx::PxU32 index, physx::PxMat44 worldPose)
{
	PX_ASSERT(mStructure != NULL);
	PX_ASSERT(index + mFirstChunkIndex < mStructure->chunks.size());
	PX_ASSERT(!mStructure->chunks[index + mFirstChunkIndex].isDestroyed());
	DestructibleStructure::Chunk & chunk = mStructure->chunks[index + mFirstChunkIndex];
	PX_ASSERT(chunk.state & ChunkDynamic);
	mStructure->setChunkGlobalPose(chunk, worldPose);
}

void DestructibleActor::setLinearVelocity(const physx::PxVec3& linearVelocity)
{
	// Only dynamic actors need their velocity set, and they'll all be in the skinned mesh
	const physx::PxU16* indexPtr = getVisibleChunks();
	const physx::PxU16* indexPtrStop = indexPtr + getNumVisibleChunks();
	while (indexPtr < indexPtrStop)
	{
		DestructibleStructure::Chunk& chunk = mStructure->chunks[mFirstChunkIndex + *indexPtr++];
		if (chunk.state & ChunkDynamic)
		{
			NxActor* actor = (NxActor*)mStructure->getChunkActor(chunk);
			if (actor != NULL)
			{
				actor->setLinearVelocity(NXFROMPXVEC3(linearVelocity));
			}
		}
	}
}

void DestructibleActor::setAngularVelocity(const physx::PxVec3& angularVelocity)
{
	// Only dynamic actors need their velocity set, and they'll all be in the skinned mesh
	const physx::PxU16* indexPtr = getVisibleChunks();
	const physx::PxU16* indexPtrStop = indexPtr + getNumVisibleChunks();
	while (indexPtr < indexPtrStop)
	{
		DestructibleStructure::Chunk& chunk = mStructure->chunks[mFirstChunkIndex + *indexPtr++];
		if (chunk.state & ChunkDynamic)
		{
			NxActor* actor = (NxActor*)mStructure->getChunkActor(chunk);
			if (actor != NULL)
			{
#if NX_SDK_VERSION_MAJOR == 2
				actor->setAngularVelocity(NXFROMPXVEC3(angularVelocity));
#elif NX_SDK_VERSION_MAJOR == 3
			PxRigidBody*	rigidBody	= actor->isRigidBody();
			if (rigidBody)
			{
				rigidBody->setAngularVelocity(angularVelocity);
			}
#endif
			}
		}
	}
}

void DestructibleActor::enableHardSleeping()
{
	mParams->useHardSleeping = true;
}

void DestructibleActor::disableHardSleeping(bool wake)
{
	if (!useHardSleeping())
	{
		return;	// Nothing to do
	}

	if (mStructure == NULL || mStructure->dscene == NULL || mStructure->dscene->mModule == NULL)
	{
		return;	// Can't do anything
	}

	mParams->useHardSleeping = false;

	NxBank<DormantActorEntry, physx::PxU32>& dormantActors = mStructure->dscene->mDormantActors;
	for (physx::PxU32 dormantActorRank = dormantActors.usedCount(); dormantActorRank--;)
	{
		const physx::PxU32 dormantActorIndex = dormantActors.usedIndices()[dormantActorRank];
		DormantActorEntry& dormantActorEntry = dormantActors.direct(dormantActorIndex);
		// Look at every destructible actor which contributes to this physx actor, and see if any use hard sleeping
		bool keepDormant = false;
		NiApexPhysXObjectDesc* actorObjDesc = (NiApexPhysXObjectDesc*)mStructure->dscene->mModule->mSdk->getPhysXObjectInfo(dormantActorEntry.actor);
		if (actorObjDesc != NULL)
		{
			for (physx::PxU32 i = 0; i < actorObjDesc->mApexActors.size(); ++i)
			{
				const NxDestructibleActor* dActor = static_cast<const NxDestructibleActor*>(actorObjDesc->mApexActors[i]);
				if (dActor != NULL)
				{
					if (actorObjDesc->mApexActors[i]->getOwner()->getObjTypeID() == DestructibleAsset::getAssetTypeID())
					{
						const DestructibleActor& destructibleActor = static_cast<const DestructibleActorProxy*>(dActor)->impl;
						if (destructibleActor.useHardSleeping())
						{
							keepDormant = true;
							break;
						}
					}
				}
			}
		}
		// If none use hard sleeping, we will remove the physx actor from the dormant list
		if (!keepDormant)
		{
			NxActor* actor = dormantActorEntry.actor;
			dormantActorEntry.actor = NULL;
			dormantActors.free(dormantActorIndex);
			actorObjDesc->userData = NULL;
			actor->clearBodyFlag(NX_BF_KINEMATIC);
			mStructure->dscene->addActor(*actorObjDesc, *actor, dormantActorEntry.unscaledMass,
				((dormantActorEntry.flags & ActorFIFOEntry::IsDebris) != 0));
			// Wake if requested
			if (wake)
			{
				actor->wakeUp();
			}
		}
	}
}

bool DestructibleActor::setChunkPhysXActorAwakeState(physx::PxU32 chunkIndex, bool awake)
{
	NxActor* actor = getChunkActor(chunkIndex);
	if (actor == NULL)
	{
		return false;
	}

#if NX_SDK_VERSION_MAJOR == 3
	if (actor->getScene() == NULL)
	{
		// defer
		if (mDestructibleScene != NULL)
		{
			mDestructibleScene->addForceToAddActorsMap(actor, ActorForceAtPosition(physx::PxVec3(0.0f), physx::PxVec3(0.0f), physx::PxForceMode::eFORCE, awake, false));
			return true;
		}
		return false;
	}
#endif

	// Actor has a scene, set sleep state now
	if (awake)
	{
		actor->wakeUp();
	}
	else
	{
#if NX_SDK_VERSION_MAJOR == 2
		actor->putToSleep();
#elif NX_SDK_VERSION_MAJOR == 3
		((PxRigidDynamic*)actor)->putToSleep();
#endif
	}

	return true;
}

bool DestructibleActor::addForce(PxU32 chunkIndex, const PxVec3& force, physx::PxForceMode::Enum mode, const PxVec3* position, bool wakeup)
{
	NxActor* actor = getChunkActor(chunkIndex);
	if (actor == NULL)
	{
		return false;
	}

#if NX_SDK_VERSION_MAJOR == 3
	if (actor->getScene() == NULL)
	{
		// defer
		if (mDestructibleScene != NULL)
		{
			if (position)
			{
				mDestructibleScene->addForceToAddActorsMap(actor, ActorForceAtPosition(force, *position, mode, wakeup, true));
			}
			else
			{
				mDestructibleScene->addForceToAddActorsMap(actor, ActorForceAtPosition(force, PxVec3(0.0f), mode, wakeup, false));
			}
			return true;
		}
		return false;
	}
#endif

	// Actor has a scene, add force now
#if NX_SDK_VERSION_MAJOR == 2
	NxVec3 nxforce(force.x, force.y, force.z);
	NxVec3 nxposition(0.0f, 0.0f, 0.0f);
	if (position)
	{
		nxposition = NxVec3(position->x, position->y, position->z);
	}
	NxForceMode nxmode = NX_FORCE;
	switch (mode)
	{
	case physx::PxForceMode::eFORCE : nxmode = NX_FORCE;				break;
	case physx::PxForceMode::eIMPULSE : nxmode = NX_IMPULSE;			break;
	case physx::PxForceMode::eVELOCITY_CHANGE : nxmode = NX_VELOCITY_CHANGE;	break;
	case physx::PxForceMode::eACCELERATION : nxmode = NX_ACCELERATION;		break;
	}
	if (position)
	{
		actor->addForceAtPos(nxforce, nxposition, nxmode, wakeup);
	}
	else
	{
		actor->addForce(nxforce, nxmode, wakeup);
	}
#elif NX_SDK_VERSION_MAJOR == 3
	PxRigidBody* rigidBody = actor->isRigidBody();
	if (rigidBody)
	{
		if (position)
		{
			PxRigidBodyExt::addForceAtPos(*rigidBody, force, *position, mode, wakeup);
		}
		else
		{
			rigidBody->addForce(force, mode, wakeup);
		}
	}
#endif

	return true;
}

void DestructibleActor::setLODWeights(physx::PxF32 maxDistance, physx::PxF32 distanceWeight, physx::PxF32 maxAge, physx::PxF32 ageWeight, physx::PxF32 bias)
{
	mState->internalLODWeights.maxDistance    = maxDistance;
	mState->internalLODWeights.distanceWeight = distanceWeight;
	mState->internalLODWeights.maxAge         = maxAge;
	mState->internalLODWeights.ageWeight      = ageWeight;
	mState->internalLODWeights.bias           = bias;
}

void DestructibleActor::getPhysicalLodRange(physx::PxF32& min, physx::PxF32& max, bool& intOnly) const
{
	min = 0.0f;
	max = (physx::PxF32)(physx::PxMax(mAsset->getDepthCount(), (physx::PxU32)1) - 1);
	intOnly = true;
}

physx::PxF32 DestructibleActor::getActivePhysicalLod() const
{
	return (physx::PxF32)getLOD();
}

void DestructibleActor::forcePhysicalLod(physx::PxF32 lod)
{
	if (lod < 0.0f)
	{
		mState->lod = physx::PxMax(mAsset->getDepthCount(), (physx::PxU32)1) - 1;
		mState->forceLod = false;
	}
	else
	{
		physx::PxF32 min, max;
		bool intOnly;
		getPhysicalLodRange(min, max, intOnly);
		mState->lod = (physx::PxU32)physx::PxClamp(lod, min, max);
		mState->forceLod = true;
	}
}

void DestructibleActor::setDynamic(physx::PxI32 chunkIndex, bool immediate)
{
	const physx::PxU16* indexPtr = NULL;
	const physx::PxU16* indexPtrStop = NULL;
	physx::PxU16 index;
	if (chunkIndex == NxModuleDestructibleConst::INVALID_CHUNK_INDEX)
	{
		indexPtr = getVisibleChunks();
		indexPtrStop = indexPtr + getNumVisibleChunks();
	}
	else
	if (chunkIndex >= 0 && chunkIndex < mAsset->mParams->chunks.arraySizes[0])
	{
		index = (physx::PxU16)chunkIndex;
		indexPtr = &index;
		indexPtrStop = indexPtr+1;
	}

	while (indexPtr < indexPtrStop)
	{
		const physx::PxU16 index = *indexPtr++;
		DestructibleStructure::Chunk& chunk = mStructure->chunks[mFirstChunkIndex + index];
		if ((chunk.state & ChunkDynamic)==0)
		{
			FractureEvent stackEvent;
			FractureEvent& fractureEvent = immediate ? stackEvent : mStructure->dscene->mFractureBuffer.pushBack();
			fractureEvent.chunkIndexInAsset = chunk.indexInAsset;
			fractureEvent.destructibleID = mID;
			fractureEvent.impulse = physx::PxVec3(0.0f);
			fractureEvent.position = physx::PxVec3(0.0f);
			fractureEvent.flags = FractureEvent::Forced | FractureEvent::Silent;
			if (immediate)
			{
				getStructure()->fractureChunk(fractureEvent);
			}
		}
	}
}

void DestructibleActor::getChunkVisibilities(physx::PxU8* visibilityArray, physx::PxU32 visibilityArraySize) const
{
	if (visibilityArray == NULL || visibilityArraySize == 0)
	{
		return;
	}

	memset(visibilityArray, 0, visibilityArraySize);

	const physx::PxU32 visiblePartCount = mVisibleChunks.usedCount();
	const physx::PxU16* visiblePartIndices = mVisibleChunks.usedIndices();
	for (physx::PxU32 i = 0; i < visiblePartCount; ++i)
	{
		const physx::PxU32 index = visiblePartIndices[i];
		if (index < visibilityArraySize)
		{
			visibilityArray[index] = 1;
		}
	}
}

bool DestructibleActor::acquireChunkEventBuffer(const NxDestructibleChunkEvent*& buffer, physx::PxU32& bufferSize)
{
	mChunkEventBufferLock.lock();
		
	buffer = mChunkEventBuffer.begin();
	bufferSize = mChunkEventBuffer.size();
	return true;
}

bool DestructibleActor::releaseChunkEventBuffer(bool clearBuffer /* = true */)
{
	if (clearBuffer)
	{
		mChunkEventBuffer.reset();
		// potentially O(n), but is O(1) (empty list) if chunk event callbacks aren't enabled.  The chunk event callbacks
		// and aquireChunkEventBuffer/releaseChunkEventBuffer mechanisms probably won't be used together.
		mDestructibleScene->mActorsWithChunkStateEvents.findAndReplaceWithLast(this);
	}

	mChunkEventBufferLock.unlock();
	return true;
}

// PhysX actor buffer API
#if NX_SDK_VERSION_MAJOR == 2
bool DestructibleActor::acquirePhysXActorBuffer(NxActor**& buffer, physx::PxU32& bufferSize, physx::PxU32 flags)
#elif NX_SDK_VERSION_MAJOR == 3
bool DestructibleActor::acquirePhysXActorBuffer(physx::PxRigidDynamic**& buffer, physx::PxU32& bufferSize, physx::PxU32 flags)
#endif
{
	mPhysXActorBufferLock.lock();

	mPhysXActorBufferAcquired = true;
	// Clear buffer, just in case
	mPhysXActorBuffer.reset();

	const bool eliminateRedundantActors = (flags & NxDestructiblePhysXActorQueryFlags::AllowRedundancy) == 0;

#if NX_SDK_VERSION_MAJOR == 3
	const bool onlyAllowActorsInScene = (flags & NxDestructiblePhysXActorQueryFlags::AllowActorsNotInScenes) == 0;
#endif

	// Fill the actor buffer
	for (physx::PxU32 actorNum = 0; actorNum < mReferencingActors.size(); ++actorNum)
	{
		NxActor* actor = mReferencingActors[actorNum];
		if (actor == NULL || actor->getNbShapes() == 0
#if NX_SDK_VERSION_MAJOR == 3
			// don't return actors that have not yet been added to the scene
			// to prevent errors in actor functions that require a scene
			|| ((actor->getScene() == NULL) && onlyAllowActorsInScene)
#endif
		)
		{
			continue;
		}
		NxShape* firstShape = getShape(*actor, 0);
		NiApexPhysXObjectDesc* objDesc = mStructure->dscene->mModule->mSdk->getGenericPhysXObjectInfo(firstShape);
		if (objDesc == NULL)
		{
			continue;
		}
		const DestructibleStructure::Chunk& chunk = *(DestructibleStructure::Chunk*)objDesc->userData;
		const physx::PxU32 actorState = !actor->readBodyFlag(NX_BF_KINEMATIC) ? NxDestructiblePhysXActorQueryFlags::Dynamic :
			((chunk.state & ChunkDynamic) == 0 ? NxDestructiblePhysXActorQueryFlags::Static : NxDestructiblePhysXActorQueryFlags::Dormant);
		if ((actorState & flags) == 0)
		{
			continue;	// Not a type of actor requested
		}
		if (eliminateRedundantActors && chunk.destructibleID != mID)
		{
			continue;	// If eliminateRedundantActors is set, only take actors whose first shape belongs to a chunk from this destructible
		}
#if NX_SDK_VERSION_MAJOR == 2
		mPhysXActorBuffer.pushBack(actor);
#elif NX_SDK_VERSION_MAJOR == 3
		mPhysXActorBuffer.pushBack((physx::PxRigidDynamic*)actor);
#endif
	}

	// Return buffer
	buffer = mPhysXActorBuffer.begin();
	bufferSize = mPhysXActorBuffer.size();
	return true;
}

bool DestructibleActor::releasePhysXActorBuffer()
{
	if(mPhysXActorBufferAcquired)
	{
		mPhysXActorBuffer.reset();
		mPhysXActorBufferAcquired = false;
		mPhysXActorBufferLock.unlock();
		return true;
	}
	else
	{	
		return false;
	}
}

bool DestructibleActor::recreateApexEmitter(NxDestructibleEmitterType::Enum type)
{
	bool ret = false;

	switch (type)
	{
	case NxDestructibleEmitterType::Crumble:
		if (getCrumbleEmitterName())
		{
			ret = initCrumbleSystem(getCrumbleEmitterName());
		}
		break;
	case NxDestructibleEmitterType::Dust:
		if (getDustEmitterName())
		{
			ret = initDustSystem(getDustEmitterName());
		}
		break;
	default:
		break;
	}
	return ret;
}

bool DestructibleActor::initDustSystem(const char* name)
{
#if APEX_USE_PARTICLES

	if (mDustEmitter)
	{
		return true;
	}

	// Set up dust system
	if (name)
	{
		setDustEmitterName(name);
		/* This destructible actor will hold a reference to its MeshParticleFactoryAsset */
		NxApexAsset* tmpAsset = mAsset->mDustAssetTracker.getAssetFromName(name);
		NxApexEmitterAsset* fasset = static_cast<NxApexEmitterAsset*>(tmpAsset);
		if (fasset)
		{
			NxParameterized::Interface* descParams = fasset->getDefaultActorDesc();
			PX_ASSERT(descParams);
			if (descParams)
			{
				mDustEmitter = static_cast<NxApexEmitterActor*>(fasset->createApexActor(*descParams, *mDestructibleScene->mApexScene));
				if (mDustEmitter)
				{
					ApexActor* aa = mAsset->module->mSdk->getApexActor(mDustEmitter);
					if (aa)
					{
						aa->addSelfToContext(*this);
					}
					if (!mDustEmitter->isExplicitGeom())
					{
						APEX_INTERNAL_ERROR("Destructible actors need NxEmitterExplicitGeom emitters.");
					}
					if (mDustRenderVolume)
					{
						mDustEmitter->setPreferredRenderVolume(mDustRenderVolume);
					}
					mDustEmitter->startEmit(true);
				}
			}
		}
	}
	else
	{
		mState->enableDustEmitter = false;
	}
#else
	PX_UNUSED(name);
#endif // APEX_USE_PARTICLES

	return mDustEmitter ? true : false;
}

bool DestructibleActor::initCrumbleSystem(const char* name)
{
#if APEX_USE_PARTICLES
	// Set up crumble system
	if (mCrumbleEmitter)
	{
		return true;
	}
	if (name)
	{
		setCrumbleEmitterName(name);
		NxApexAsset* tmpAsset = mAsset->mCrumbleAssetTracker.getAssetFromName(name);
		NxApexEmitterAsset* fasset = static_cast<NxApexEmitterAsset*>(tmpAsset);
		if (fasset)
		{
			NxParameterized::Interface* descParams = fasset->getDefaultActorDesc();
			PX_ASSERT(descParams);
			if (descParams)
			{
				mCrumbleEmitter = static_cast<NxApexEmitterActor*>(fasset->createApexActor(*descParams, *mDestructibleScene->mApexScene));
				if (mCrumbleEmitter)
				{
					ApexActor* aa = mAsset->module->mSdk->getApexActor(mCrumbleEmitter);
					if (aa)
					{
						aa->addSelfToContext(*this);
					}
					if (!mCrumbleEmitter->isExplicitGeom())
					{
						APEX_INTERNAL_ERROR("Destructible actors need NxEmitterExplicitGeom emitters.");
					}
					if (mCrumbleRenderVolume)
					{
						mCrumbleEmitter->setPreferredRenderVolume(mCrumbleRenderVolume);
					}
					mCrumbleEmitter->startEmit(true);
				}
			}
		}
	}
	else
	{
		mState->enableCrumbleEmitter = false;
	}
#else
	PX_UNUSED(name);
#endif // APEX_USE_PARTICLES

	return mCrumbleEmitter ? true : false;
}

void DestructibleActor::setCrumbleEmitterName(const char* name)
{
	// Avoid self-assignment
	if (name == getCrumbleEmitterName())
		return;
	NxParameterized::Handle handle(*mParams);
	mParams->getParameterHandle("crumbleEmitterName", handle);
	mParams->setParamString(handle, name ? name : "");
}

const char* DestructibleActor::getCrumbleEmitterName() const
{
	const char* name = (const char*)mParams->crumbleEmitterName;
	return (name && *name) ? name : NULL;
}

void DestructibleActor::setDustEmitterName(const char* name)
{
	// Avoid self-assignment
	if (name == getDustEmitterName())
		return;
	NxParameterized::Handle handle(*mParams);
	mParams->getParameterHandle("dustEmitterName", handle);
	mParams->setParamString(handle, name ? name : "");
}

const char* DestructibleActor::getDustEmitterName() const
{
	const char* name = (const char*)mParams->dustEmitterName;
	return (name && *name) ? name : NULL;
}


void DestructibleActor::setPreferredRenderVolume(NxApexRenderVolume* volume, NxDestructibleEmitterType::Enum type)
{
#if APEX_USE_PARTICLES
	switch (type)
	{
	case NxDestructibleEmitterType::Crumble:
		mCrumbleRenderVolume = volume;
		if (mCrumbleEmitter)
		{
			mCrumbleEmitter->setPreferredRenderVolume(volume);
		}
		break;
	case NxDestructibleEmitterType::Dust:
		mDustRenderVolume = volume;
		if (mDustEmitter)
		{
			mDustEmitter->setPreferredRenderVolume(volume);
		}
		break;
	default:
		break;
	}
#else
	PX_UNUSED(volume);
	PX_UNUSED(type);
#endif // APEX_USE_PARTICLES
}

NxApexEmitterActor* DestructibleActor::getApexEmitter(NxDestructibleEmitterType::Enum type)
{
	NxApexEmitterActor* ret = NULL;

#if APEX_USE_PARTICLES
	switch (type)
	{
	case NxDestructibleEmitterType::Crumble:
		ret = mCrumbleEmitter;
		break;
	case NxDestructibleEmitterType::Dust:
		ret = mDustEmitter;
		break;
	default:
		break;
	}
#else
	PX_UNUSED(type);
#endif // APEX_USE_PARTICLES

	return ret;
}

void DestructibleActor::preSerialize(void* nxParameterizedType)
{
	NxDestructibleParameterizedType::Enum nxType = (NxDestructibleParameterizedType::Enum)((intptr_t)nxParameterizedType);
	switch (nxType)
	{
	case NxDestructibleParameterizedType::State:
		{
			PX_ASSERT(mState->actorChunks);
			serialize(*this, &getStructure()->chunks[0] + mFirstChunkIndex, getAsset()->getChunkCount(), *mChunks);
			break;
		}
	case NxDestructibleParameterizedType::Params:
		{
			PX_ASSERT(mParams);
			physx::PxMat44 globalPoseForStaticChunks;
			if (!getGlobalPoseForStaticChunks(globalPoseForStaticChunks))
			{
				globalPoseForStaticChunks = getInitialGlobalPose();	// This will occur if there are no static chunks
			}
			physx::PxTransform globalPose(globalPoseForStaticChunks);
			globalPose.q.normalize();
			mParams->globalPose = physx::PxMat34Legacy(globalPose);
			PX_ASSERT(mState->actorParameters);
			serialize(mDestructibleParameters, *mState->actorParameters);
			break;
		}
	default:
		PX_ASSERT(0 && "Invalid destructible parameterized type");
		break;
	}
}

bool DestructibleActor::applyDamageColoring(physx::PxU16 indexInAsset, const physx::PxVec3& position, physx::PxF32 damage, physx::PxF32 damageRadius)
{
	bool bRet = applyDamageColoringRecursive(indexInAsset, position, damage, damageRadius);

	NxRenderMeshAsset* rma = mAsset->getRenderMeshAsset();
	for (physx::PxU32 submeshIndex = 0; submeshIndex < rma->getSubmeshCount(); ++submeshIndex)
	{
		physx::Array<PxColorRGBA>& damageColorArray = mDamageColorArrays[submeshIndex];
		// fix asset on &damageColorArray[0]
		if (damageColorArray.size() != 0)
		{
			for (physx::PxU32 typeN = 0; typeN < NxDestructibleActorMeshType::Count; ++typeN)
			{
				NiApexRenderMeshActor* renderMeshActor = (NiApexRenderMeshActor*)getRenderMeshActor((NxDestructibleActorMeshType::Enum)typeN);
				if (renderMeshActor != NULL)
				{
					renderMeshActor->setStaticColorReplacement(submeshIndex, &damageColorArray[0]);
				}
			}
		}
	}

	return bRet;
}

bool DestructibleActor::applyDamageColoringRecursive(physx::PxU16 indexInAsset, const physx::PxVec3& position, physx::PxF32 damage, physx::PxF32 damageRadius)
{
	NxRenderMeshAsset* rma = mAsset->getRenderMeshAsset();
	if (mDamageColorArrays.size() != rma->getSubmeshCount())
	{
		return false;
	}

	// Get behavior group
	const DestructibleAssetParametersNS::Chunk_Type& source = getAsset()->mParams->chunks.buf[indexInAsset];
	const DestructibleActorParamNS::BehaviorGroup_Type& behaviorGroup = getBehaviorGroupImp(source.behaviorGroupIndex);

	const physx::PxF32 maxRadius = behaviorGroup.damageColorSpread.minimumRadius + damageRadius*behaviorGroup.damageColorSpread.radiusMultiplier;

	const DestructibleStructure::Chunk& chunk = mStructure->chunks[indexInAsset + mFirstChunkIndex];
	const physx::PxVec3 disp = mStructure->getChunkWorldCentroid(chunk) - position;
	const physx::PxF32 dist = disp.magnitude();
	if (dist > maxRadius + chunk.localSphere.radius)
	{
		return false;	// Outside of max radius
	}

	bool bRet = false;

//	const physx::PxF32 recipRadiusRange = maxRadius > behaviorGroup.damageColorSpread.minimumRadius ? 1.0f/(maxRadius - behaviorGroup.damageColorSpread.minimumRadius) : 0.0f;
	const physx::PxF32 recipRadiusRange = maxRadius > damageRadius ? 1.0f/(maxRadius - damageRadius) : 0.0f;

	// Color scale multiplier based upon damage
	const physx::PxF32 colorScale = behaviorGroup.damageThreshold > 0.0f ? physx::PxMin(1.0f, damage/behaviorGroup.damageThreshold) : 1.0f;

	const physx::PxU32 partIndex = mAsset->getPartIndex(indexInAsset);

//	const physx::PxF32 minRadiusSquared = behaviorGroup.damageColorSpread.minimumRadius*behaviorGroup.damageColorSpread.minimumRadius;
	const physx::PxF32 minRadiusSquared = damageRadius*damageRadius;
	const physx::PxF32 maxRadiusSquared = maxRadius*maxRadius;

	physx::PxMat44 chunkGlobalPose = getChunkPose(indexInAsset);
	chunkGlobalPose.scale(physx::PxVec4(getScale(), 1.0f));

	// Find all vertices and modify color
	for (physx::PxU32 submeshIndex = 0; submeshIndex < rma->getSubmeshCount(); ++submeshIndex)
	{
		const NxRenderSubmesh& submesh = rma->getSubmesh(submeshIndex);
		physx::Array<PxColorRGBA>& damageColorArray = mDamageColorArrays[submeshIndex];
		const NxVertexBuffer& vb = submesh.getVertexBuffer();
		PX_ASSERT(damageColorArray.size() == vb.getVertexCount());
		if (damageColorArray.size() != vb.getVertexCount())	// Make sure we have the correct size color array
		{
			continue;
		}
		const NxVertexFormat& vf = vb.getFormat();
		const physx::PxI32 positionBufferIndex = vf.getBufferIndexFromID(vf.getSemanticID(NxRenderVertexSemantic::POSITION));
		if (positionBufferIndex >= 0)	// Make sure we have a position buffer
		{
			NxRenderDataFormat::Enum positionBufferFormat = vf.getBufferFormat((physx::PxU32)positionBufferIndex);
			PX_ASSERT(positionBufferFormat == NxRenderDataFormat::FLOAT3);
			if (positionBufferFormat != NxRenderDataFormat::FLOAT3)
			{
				continue;
			}
			const physx::PxVec3* positions = (const physx::PxVec3*)vb.getBuffer((physx::PxU32)positionBufferIndex);
			// Get the vertex range for the part associated with this chunk (note: this will _not_ work with instancing)
			const physx::PxU32 firstVertexIndex = submesh.getFirstVertexIndex(partIndex);
			const physx::PxU32 stopVertexIndex = firstVertexIndex + submesh.getVertexCount(partIndex);
			for (physx::PxU32 vertexIndex = firstVertexIndex; vertexIndex < stopVertexIndex; ++vertexIndex)
			{
				// Adjust the color scale based upon the envelope function
				physx::PxF32 colorChangeMultiplier = colorScale;
				// Get the world vertex position
				const physx::PxVec3 worldVertexPosition = chunkGlobalPose.transform(positions[vertexIndex]);
				const physx::PxF32 radiusSquared = (position - worldVertexPosition).magnitudeSquared();
				if (radiusSquared > maxRadiusSquared)
				{
					continue;
				}
				if (radiusSquared > minRadiusSquared)
				{
					colorChangeMultiplier *= physx::PxPow((maxRadius - physx::PxSqrt(radiusSquared))*recipRadiusRange, behaviorGroup.damageColorSpread.falloffExponent);
				}
				// Get the color, add the scaled color values, clamp, and set the new color
				PxColorRGBA& color = damageColorArray[vertexIndex];
				physx::PxVec4 newColor = physx::PxVec4((physx::PxF32)color.r, (physx::PxF32)color.g, (physx::PxF32)color.b, (physx::PxF32)color.a) + colorChangeMultiplier*behaviorGroup.damageColorChange;
				newColor = newColor.maximum(physx::PxVec4(0.0f));
				newColor = newColor.minimum(physx::PxVec4(255.0f));
				// save previous color
				PxColorRGBA preColor = color;
				color.r = (physx::PxU8)(newColor[0] + 0.5f);
				color.g = (physx::PxU8)(newColor[1] + 0.5f);
				color.b = (physx::PxU8)(newColor[2] + 0.5f);
				color.a = (physx::PxU8)(newColor[3] + 0.5f);

				// Only save the static chunk now.
				if ((chunk.state & ChunkDynamic) == 0)
				{
					// compare the previous color with the new color
					bRet = preColor.r != color.r || preColor.g != color.g || preColor.b != color.b || preColor.a != color.a;
				}
			}
		}
	}

	// Recurse to children
	const physx::PxU32 stopIndex = PxU32(source.firstChildIndex + source.numChildren);
	for (physx::PxU32 childIndex = source.firstChildIndex; childIndex < stopIndex; ++childIndex)
	{
		bRet |= applyDamageColoringRecursive((physx::PxU16)childIndex, position, damage, damageRadius);
	}

	return bRet;
}

void DestructibleActor::fillBehaviorGroupDesc(NxDestructibleBehaviorGroupDesc& behaviorGroupDesc, const DestructibleActorParamNS::BehaviorGroup_Type behaviorGroup) const
{
	behaviorGroupDesc.name = behaviorGroup.name;
	behaviorGroupDesc.damageThreshold = behaviorGroup.damageThreshold;
	behaviorGroupDesc.damageToRadius = behaviorGroup.damageToRadius;
	behaviorGroupDesc.damageSpread.minimumRadius = behaviorGroup.damageSpread.minimumRadius;
	behaviorGroupDesc.damageSpread.radiusMultiplier = behaviorGroup.damageSpread.radiusMultiplier;
	behaviorGroupDesc.damageSpread.falloffExponent = behaviorGroup.damageSpread.falloffExponent;
	behaviorGroupDesc.damageColorSpread.minimumRadius = behaviorGroup.damageColorSpread.minimumRadius;
	behaviorGroupDesc.damageColorSpread.radiusMultiplier = behaviorGroup.damageColorSpread.radiusMultiplier;
	behaviorGroupDesc.damageColorSpread.falloffExponent = behaviorGroup.damageColorSpread.falloffExponent;
	behaviorGroupDesc.damageColorChange = behaviorGroup.damageColorChange;
	behaviorGroupDesc.materialStrength = behaviorGroup.materialStrength;
	behaviorGroupDesc.density = behaviorGroup.density;
	behaviorGroupDesc.fadeOut = behaviorGroup.fadeOut;
	behaviorGroupDesc.maxDepenetrationVelocity = behaviorGroup.maxDepenetrationVelocity;
	behaviorGroupDesc.userData = behaviorGroup.userData;
}

void DestructibleActor::spawnParticles(NxApexEmitterActor* emitter, NxUserChunkParticleReport* report, DestructibleStructure::Chunk& chunk, physx::Array<physx::PxVec3>& positions, bool deriveVelocitiesFromChunk, const physx::PxVec3* overrideVelocity)
{
#if APEX_USE_PARTICLES
	physx::PxU32 numParticles = positions.size();
	if (numParticles == 0)
	{
		return;
	}

	NxEmitterExplicitGeom* geom = emitter != NULL ? emitter->isExplicitGeom() : NULL;	// emitter may be NULL, since we may have a particle callback

	if (geom == NULL && report == NULL)
	{
		APEX_INTERNAL_ERROR("DestructibleActor::spawnParticles requires an NxEmitterExplicitGeom emitter or a NxUserChunkParticleReport, or both.");
		return;
	}

	if (overrideVelocity == NULL && deriveVelocitiesFromChunk)
	{
		physx::Array<physx::PxVec3> volumeFillVel;
		volumeFillVel.resize(numParticles);

		// Use dynamic actor's current velocity
		physx::PxVec3 angVel;
		physx::PxVec3 linVel;
		physx::PxVec3 COM;
		NxActor* actor = mStructure->getChunkActor(chunk);
		PxFromNxVec3(angVel, actor->getAngularVelocity());
		PxFromNxVec3(linVel, actor->getLinearVelocity());
#if NX_SDK_VERSION_MAJOR == 2
		PxFromNxVec3(COM, actor->getGlobalPose().t);
#elif NX_SDK_VERSION_MAJOR == 3
		PxFromNxVec3(COM, actor->getGlobalPose().p);
#endif

		for (physx::PxU32 i = 0; i < numParticles; ++i)
		{
			volumeFillVel[i] = linVel + angVel.cross(positions[i] - COM);
			PX_ASSERT(physx::PxIsFinite(volumeFillVel[i].magnitude()));
		}

		if (geom != NULL)
		{
			geom->addParticleList(numParticles, &positions.front(), &volumeFillVel.front());
			//emitter->emit(false);
		}

		if (report != NULL)
		{
			NxApexChunkParticleReportData reportData;
			reportData.positions = &positions.front();
			reportData.positionCount = numParticles;
			reportData.velocities = &volumeFillVel.front();
			reportData.velocityCount = numParticles;
			report->onParticleEmission(reportData);
		}
	}
	else
	{
		PX_ASSERT(overrideVelocity == NULL || physx::PxIsFinite(overrideVelocity->magnitude()));

		if (geom != NULL)
		{
			const physx::PxVec3 velocity = overrideVelocity != NULL ? *overrideVelocity : physx::PxVec3(0.0f);
			emitter->setVelocityRange(NxRange<physx::PxVec3>(velocity, velocity));
			geom->addParticleList(numParticles, &positions.front());
		}

		if (report != NULL)
		{
			NxApexChunkParticleReportData reportData;
			reportData.positions = &positions.front();
			reportData.positionCount = numParticles;
			reportData.velocities = overrideVelocity;
			reportData.velocityCount = overrideVelocity != NULL ? 1u : 0u;
			report->onParticleEmission(reportData);
		}
	}
#else
	PX_UNUSED(emitter);
	PX_UNUSED(report);
	PX_UNUSED(chunk);
	PX_UNUSED(positions);
	PX_UNUSED(deriveVelocitiesFromChunk);
	PX_UNUSED(overrideVelocity);
#endif // APEX_USE_PARTICLES
}

void DestructibleActor::setDeleteFracturedChunks(bool inDeleteChunkMode)
{
	mInDeleteChunkMode = inDeleteChunkMode;
}

bool DestructibleActor::setHitChunkTrackingParams(bool flushHistory, bool startTracking, physx::PxU32 trackingDepth, bool trackAllChunks)
{
	bool validOperation = false;
	if(getAsset()->getDepthCount() >= trackingDepth)
	{
		hitChunkParams.cacheChunkHits = startTracking;
		hitChunkParams.cacheAllChunks = trackAllChunks;
		if(flushHistory && !hitChunkParams.hitChunkContainer.empty())
		{
			hitChunkParams.hitChunkContainer.clear();
			damageColoringParams.damageEventCoreDataContainer.clear();
		}
		hitChunkParams.trackingDepth = trackingDepth;
		validOperation = true;
	}
	return validOperation;
}

bool DestructibleActor::getHitChunkHistory(const NxDestructibleHitChunk *& hitChunkContainer, physx::PxU32 & hitChunkCount) const
{
	bool validOperation = false;
	{
		hitChunkContainer = !hitChunkParams.hitChunkContainer.empty() ? static_cast<const NxDestructibleHitChunk*>(&hitChunkParams.hitChunkContainer[0]) : NULL;
		hitChunkCount = hitChunkParams.hitChunkContainer.size();
		validOperation = true;
	}
	return validOperation;
}

bool DestructibleActor::forceChunkHits(const NxDestructibleHitChunk * hitChunkContainer, physx::PxU32 hitChunkCount, bool removeChunks /*= true*/, bool deferredEvent /*= false*/, physx::PxVec3 damagePosition /*= physx::PxVec3(0.0f)*/, physx::PxVec3 damageDirection /*= physx::PxVec3(0.0f)*/)
{
	bool validOperation = false;
	PX_ASSERT(!((NULL == hitChunkContainer) ^ (0 == hitChunkCount)));
	if(NULL != hitChunkContainer && 0 != hitChunkCount)
	{
		physx::PxU32 manualFractureCount = 0;
		for(physx::PxU32 index = 0; index < hitChunkCount; ++index)
		{
			if(mStructure->chunks.size() > (hitChunkContainer[index].chunkIndex + mFirstChunkIndex))
			{
				hitChunkParams.manualFractureEventInstance.chunkIndexInAsset	= hitChunkContainer[index].chunkIndex;
				hitChunkParams.manualFractureEventInstance.destructibleID		= mID;
				hitChunkParams.manualFractureEventInstance.position				= damagePosition;
				hitChunkParams.manualFractureEventInstance.hitDirection			= damageDirection;
				hitChunkParams.manualFractureEventInstance.flags				= (hitChunkContainer[index].hitChunkFlags | static_cast<physx::PxU32>(FractureEvent::Manual) | (removeChunks ? static_cast<physx::PxU32>(FractureEvent::DeleteChunk) : 0));
				if(deferredEvent)
				{
					mDestructibleScene->mDeferredFractureBuffer.pushBack()		= hitChunkParams.manualFractureEventInstance;
				}
				else
				{
					mDestructibleScene->mDeprioritisedFractureBuffer.pushBack()		= hitChunkParams.manualFractureEventInstance;					
				}
				++manualFractureCount;
			}
		}
		validOperation = (manualFractureCount == hitChunkCount);
	}
	return validOperation;
}

void DestructibleActor::evaluateForHitChunkList(const FractureEvent & fractureEvent)
{
	PX_ASSERT(NULL != &fractureEvent);

	// cache hit chunks for non-manual fractureEvents, if user set so
	if(hitChunkParams.cacheChunkHits && (0 == (fractureEvent.flags & FractureEvent::Manual)))
	{
		// walk up the depth until we reach the user-specified tracking depth
		physx::PxI32 chunkToUseIndexInAsset = static_cast<physx::PxI32>(fractureEvent.chunkIndexInAsset);
		PX_ASSERT(chunkToUseIndexInAsset >= 0);
		while(NxModuleDestructibleConst::INVALID_CHUNK_INDEX != chunkToUseIndexInAsset)
		{
			PX_ASSERT(chunkToUseIndexInAsset < static_cast<physx::PxI32>(getAsset()->getChunkCount()));
			const DestructibleAssetParametersNS::Chunk_Type & source = getAsset()->mParams->chunks.buf[chunkToUseIndexInAsset];
			if(source.depth <= hitChunkParams.trackingDepth)
			{
				break;
			}
			chunkToUseIndexInAsset = static_cast<physx::PxI32>(source.parentIndex);
		}

		// cache the chunk index at the user-specified tracking depth
		if(NxModuleDestructibleConst::INVALID_CHUNK_INDEX != chunkToUseIndexInAsset)
		{
			PX_ASSERT(chunkToUseIndexInAsset + getFirstChunkIndex() < getStructure()->chunks.size());
			const DestructibleStructure::Chunk & chunkToUse = getStructure()->chunks[chunkToUseIndexInAsset + getFirstChunkIndex()];
			if((!chunkToUse.isDestroyed()) && (0 == (FractureEvent::DamageFromImpact & fractureEvent.flags)) && (!hitChunkParams.cacheAllChunks ? ((chunkToUse.state & ChunkDynamic) == 0) : true))
			{
				// these flags are only used internally for fracture events coming in from the sync buffer, so we should exclude them
				physx::PxU32 hitChunkFlags = fractureEvent.flags;
				hitChunkFlags &= ~FractureEvent::SyncDirect;
				hitChunkFlags &= ~FractureEvent::SyncDerived;
				hitChunkFlags &= ~FractureEvent::Manual;

				// disallow crumbling and snapping as well
				hitChunkFlags &= ~FractureEvent::CrumbleChunk;
				hitChunkFlags &= ~FractureEvent::Snap;

				// cache the chunk index
				hitChunkParams.hitChunkContainer.pushBack(DestructibleActor::CachedHitChunk(static_cast<physx::PxU32>(chunkToUseIndexInAsset), hitChunkFlags));
			}
		}
	}
}

bool DestructibleActor::getDamageColoringHistory(const NxDamageEventCoreData *& damageEventCoreDataContainer, physx::PxU32 & damageEventCoreDataCount) const
{
	bool validOperation = false;
	{
		damageEventCoreDataContainer = !damageColoringParams.damageEventCoreDataContainer.empty() ? static_cast<const NxDamageEventCoreData*>(&damageColoringParams.damageEventCoreDataContainer[0]) : NULL;
		damageEventCoreDataCount = damageColoringParams.damageEventCoreDataContainer.size();
		validOperation = true;
	}
	return validOperation;

}

bool DestructibleActor::forceDamageColoring(const NxDamageEventCoreData * damageEventCoreDataContainer, physx::PxU32 damageEventCoreDataCount)
{
	bool validOperation = false;
	PX_ASSERT(!((NULL == damageEventCoreDataContainer) ^ (0 == damageEventCoreDataCount)));
	if (NULL != damageEventCoreDataContainer && 0 != damageEventCoreDataCount)
	{
		physx::PxU32 manualDamageColorCount = 0;
		for (physx::PxU32 index = 0; index < damageEventCoreDataCount; ++index)
		{
			if (mStructure->chunks.size() > (damageEventCoreDataContainer[index].chunkIndexInAsset + mFirstChunkIndex))
			{
				damageColoringParams.damageEventCoreDataInstance.destructibleID = mID;
				damageColoringParams.damageEventCoreDataInstance.chunkIndexInAsset = damageEventCoreDataContainer[index].chunkIndexInAsset;
				damageColoringParams.damageEventCoreDataInstance.position = damageEventCoreDataContainer[index].position;
				damageColoringParams.damageEventCoreDataInstance.damage = damageEventCoreDataContainer[index].damage;
				damageColoringParams.damageEventCoreDataInstance.radius = damageEventCoreDataContainer[index].radius;

				mDestructibleScene->mSyncDamageEventCoreDataBuffer.pushBack() = damageColoringParams.damageEventCoreDataInstance;

				++manualDamageColorCount;
			}
		}
		validOperation = (manualDamageColorCount == damageEventCoreDataCount);
	}
	return validOperation;

}

void DestructibleActor::collectDamageColoring(const physx::PxI32 indexInAsset, const physx::PxVec3& position, const physx::PxF32 damage, const physx::PxF32 damageRadius)
{
	// only start cached the damage coloring when the flag set. see setHitChunkTrackingParams()
	if (hitChunkParams.cacheChunkHits)
	{
		damageColoringParams.damageEventCoreDataContainer.pushBack(DestructibleActor::CachedDamageEventCoreData(indexInAsset, position, damage, damageRadius));
	}
}

void DestructibleActor::applyDamageColoring_immediate(const physx::PxI32 indexInAsset, const physx::PxVec3& position, const physx::PxF32 damage, const physx::PxF32 damageRadius)
{
	// Damage coloring:
	if (useDamageColoring())
	{
		if (indexInAsset != NxModuleDestructibleConst::INVALID_CHUNK_INDEX)
		{
			// Normal Damage - apply damage coloring directly
			applyDamageColoring(static_cast<physx::PxU16>(indexInAsset), position, damage, damageRadius);
		}
		else
		{
			// Radius Damage - need to traverse all the visible chunks
			const physx::PxU16* chunkIndexPtr = mVisibleChunks.usedIndices();
			const physx::PxU16* chunkIndexPtrStop = chunkIndexPtr + mVisibleChunks.usedCount();

			while (chunkIndexPtr < chunkIndexPtrStop)
			{
				physx::PxU16 chunkIndex = *chunkIndexPtr++;
				DestructibleStructure::Chunk& chunk = mStructure->chunks[chunkIndex + mFirstChunkIndex];

				applyDamageColoring(chunk.indexInAsset, position, damage, damageRadius);
			}
		}
	}
}

bool DestructibleActor::getUseLegacyChunkBoundsTesting() const
{
	const physx::PxI8 setting = mParams->destructibleParameters.legacyChunkBoundsTestSetting;
	if (setting < 0)
	{
		return mDestructibleScene->getModule()->getUseLegacyChunkBoundsTesting();
	}
	return setting > 0;
}

bool DestructibleActor::getUseLegacyDamageRadiusSpread() const
{
	const physx::PxI8 setting = mParams->destructibleParameters.legacyDamageRadiusSpreadSetting;
	if (setting < 0)
	{
		return mDestructibleScene->getModule()->getUseLegacyDamageRadiusSpread();
	}
	return setting > 0;
}

/*** DestructibleActor::SyncParams ***/
bool DestructibleActor::setSyncParams(physx::PxU32 userActorID, physx::PxU32 actorSyncFlags, const NxDestructibleActorSyncState * actorSyncState, const NxDestructibleChunkSyncState * chunkSyncState)
{
	bool validEntry = false;
	PX_ASSERT(!mDestructibleScene->getSyncParams().lockSyncParams && "if this happens, theres more work to do!");
	if(!mDestructibleScene->getSyncParams().lockSyncParams)
	{
		const bool validActorSyncFlags = static_cast<physx::PxU32>(NxDestructibleActorSyncFlags::Last) > actorSyncFlags;
		const bool validActorSyncState = (NULL != actorSyncState) ? (mAsset->getDepthCount() >= actorSyncState->damageEventFilterDepth) && (mAsset->getDepthCount() >= actorSyncState->fractureEventFilterDepth) : true;
		const bool validChunkSyncState = (NULL != chunkSyncState) ? (mAsset->getDepthCount() >= chunkSyncState->chunkTransformCopyDepth) : true;
#if 0
		//const bool validDepth = (mAsset->getDepthCount() >= chunkPositionCopyDepth) ? (chunkPositionCopyDepth >= chunkTransformCopyDepth) : false;
#endif
		if(validActorSyncFlags && validActorSyncState && validChunkSyncState)
		{
			//determine type of operation
			const physx::PxU32 presentUserActorID = mSyncParams.getUserActorID();
			const bool addEntry			= (0 != userActorID) && (0 == presentUserActorID);
			const bool removeEntry		= (0 == userActorID) && (0 != presentUserActorID);
			const bool relocateEntry	= (0 != userActorID) && (0 != presentUserActorID) && (userActorID != presentUserActorID);
			const bool editEntry		= (0 != userActorID) && (0 != presentUserActorID) && (userActorID == presentUserActorID);
			const bool invalidEntry		= (0 == userActorID) && (0 == presentUserActorID);
			PX_ASSERT(addEntry ? (!removeEntry && !relocateEntry && !editEntry && !invalidEntry) : removeEntry ? (!relocateEntry && !editEntry && !invalidEntry) : relocateEntry ? (!editEntry && !invalidEntry) : editEntry? (!invalidEntry) : invalidEntry);

			//attempt update scene and actor params
			if(addEntry || removeEntry || relocateEntry || editEntry)
			{
				bool validUserActorID = false;
				bool useUserArguments = false;
				DestructibleScene::SyncParams & sceneParams = mDestructibleScene->getSyncParamsMutable();
				DestructibleActor * erasedEntry = NULL;
				if(addEntry)
				{
					validUserActorID = sceneParams.setSyncActor(userActorID, this, erasedEntry);
					PX_ASSERT(this != erasedEntry);
					if(validUserActorID)
					{
						useUserArguments = true;
						if(NULL != erasedEntry)
						{
							erasedEntry->getSyncParamsMutable().onReset();
							erasedEntry = NULL;
						}
					}
				}
				else if(removeEntry)
				{
					validUserActorID = sceneParams.setSyncActor(presentUserActorID, NULL, erasedEntry);
					PX_ASSERT((NULL != erasedEntry) && (this == erasedEntry));
					if(validUserActorID)
					{
						mSyncParams.onReset();
					}
				}
				else if(relocateEntry)
				{
					validUserActorID = sceneParams.setSyncActor(presentUserActorID, NULL, erasedEntry);
					PX_ASSERT((NULL != erasedEntry) && (this == erasedEntry));
					if(validUserActorID)
					{
						validUserActorID = sceneParams.setSyncActor(userActorID, this, erasedEntry);
						PX_ASSERT(this != erasedEntry);
						if(validUserActorID)
						{
							useUserArguments = true;
							if(NULL != erasedEntry)
							{
								erasedEntry->getSyncParamsMutable().onReset();
								erasedEntry = NULL;
							}
						}
					}
				}
				else if(editEntry)
				{
					validUserActorID = true;
					useUserArguments = true;
				}
				else
				{
					PX_ASSERT(!"!");
				}
				validEntry = validUserActorID && validActorSyncFlags && validActorSyncState && validChunkSyncState;
				if(useUserArguments)
				{
					PX_ASSERT(validEntry);
					mSyncParams.userActorID			= userActorID;
					mSyncParams.actorSyncFlags		= actorSyncFlags;
					if (actorSyncState)
					{
						mSyncParams.useActorSyncState = true;
						mSyncParams.actorSyncState	= *actorSyncState;
					}
					if (chunkSyncState)
					{
						mSyncParams.useChunkSyncState = true;
						mSyncParams.chunkSyncState	= *chunkSyncState;
					}
				}
			}
			else
			{
				PX_ASSERT(invalidEntry);
				PX_UNUSED(invalidEntry);
				PX_ASSERT(!"invalid use of function!");
			}
		}
	}
	return validEntry;
}

typedef DestructibleActor::SyncParams SyncParams;

SyncParams::SyncParams()
	:userActorID(0)
	,actorSyncFlags(0)
	,useActorSyncState(false)
	,useChunkSyncState(false)
{
	PX_ASSERT(damageBufferIndices.empty());
	PX_ASSERT(fractureBufferIndices.empty());
    PX_ASSERT(cachedChunkTransforms.empty());
}

SyncParams::~SyncParams()
{
	PX_ASSERT(cachedChunkTransforms.empty());
	PX_ASSERT(fractureBufferIndices.empty());
	PX_ASSERT(damageBufferIndices.empty());
	PX_ASSERT(!useChunkSyncState);
	PX_ASSERT(!useActorSyncState);
	PX_ASSERT(0 == actorSyncFlags);
	PX_ASSERT(0 == userActorID);
}

physx::PxU32 SyncParams::getUserActorID() const
{
	return userActorID;
}

bool SyncParams::isSyncFlagSet(NxDestructibleActorSyncFlags::Enum flag) const
{
	PX_ASSERT(0 != userActorID);
	return (0 != (actorSyncFlags & static_cast<physx::PxU32>(flag)));
}

const NxDestructibleActorSyncState * SyncParams::getActorSyncState() const
{
	PX_ASSERT(useActorSyncState ? (isSyncFlagSet(NxDestructibleActorSyncFlags::CopyDamageEvents) || isSyncFlagSet(NxDestructibleActorSyncFlags::CopyFractureEvents)) : true);
	return useActorSyncState ? &actorSyncState : NULL;
}

const NxDestructibleChunkSyncState * SyncParams::getChunkSyncState() const
{
	PX_ASSERT(useChunkSyncState ? isSyncFlagSet(NxDestructibleActorSyncFlags::CopyChunkTransform) : true);
	return useChunkSyncState ? &chunkSyncState : NULL;
}

void SyncParams::pushDamageBufferIndex(physx::PxU32 index)
{
	PX_ASSERT(isSyncFlagSet(NxDestructibleActorSyncFlags::CopyDamageEvents));
	damageBufferIndices.pushBack(index);
}

void SyncParams::pushFractureBufferIndex(physx::PxU32 index)
{
	PX_ASSERT(isSyncFlagSet(NxDestructibleActorSyncFlags::CopyFractureEvents));
	fractureBufferIndices.pushBack(index);
}

void SyncParams::pushCachedChunkTransform(const CachedChunk & cachedChunk)
{
	PX_ASSERT(isSyncFlagSet(NxDestructibleActorSyncFlags::CopyChunkTransform));
	cachedChunkTransforms.pushBack(cachedChunk);
}

const physx::Array<physx::PxU32> & SyncParams::getDamageBufferIndices() const
{
	return damageBufferIndices;
}

const physx::Array<physx::PxU32> & SyncParams::getFractureBufferIndices() const
{
	return fractureBufferIndices;
}

const physx::Array<CachedChunk> & SyncParams::getCachedChunkTransforms() const
{
	return cachedChunkTransforms;
}

template<> void SyncParams::clear<NxApexDamageEventUnit>()
{
	PX_ASSERT(!damageBufferIndices.empty() ? isSyncFlagSet(NxDestructibleActorSyncFlags::CopyDamageEvents) : true);
	if(!damageBufferIndices.empty()) damageBufferIndices.clear();
}

template<> void SyncParams::clear<NxApexFractureEventUnit>()
{
	PX_ASSERT(!fractureBufferIndices.empty() ? isSyncFlagSet(NxDestructibleActorSyncFlags::CopyFractureEvents) : true);
	if(!fractureBufferIndices.empty()) fractureBufferIndices.clear();
}

template<> void SyncParams::clear<NxApexChunkTransformUnit>()
{
	PX_ASSERT(!cachedChunkTransforms.empty() ? isSyncFlagSet(NxDestructibleActorSyncFlags::CopyChunkTransform) : true);
	if(!cachedChunkTransforms.empty()) cachedChunkTransforms.clear();
}

template<> physx::PxU32 SyncParams::getCount<NxApexDamageEventUnit>() const
{
	PX_ASSERT(!damageBufferIndices.empty() ? isSyncFlagSet(NxDestructibleActorSyncFlags::CopyDamageEvents) : true);
	return damageBufferIndices.size();
}

template<> physx::PxU32 SyncParams::getCount<NxApexFractureEventUnit>() const
{
	PX_ASSERT(!fractureBufferIndices.empty() ? isSyncFlagSet(NxDestructibleActorSyncFlags::CopyFractureEvents) : true);
	return fractureBufferIndices.size();
}

template<> physx::PxU32 SyncParams::getCount<NxApexChunkTransformUnit>() const
{
	PX_ASSERT(!cachedChunkTransforms.empty() ? isSyncFlagSet(NxDestructibleActorSyncFlags::CopyChunkTransform) : true);
	return cachedChunkTransforms.size();
}

void SyncParams::onReset()
{
	userActorID = 0;
	actorSyncFlags = 0;
	useActorSyncState = false;
	useChunkSyncState = false;
	damageBufferIndices.clear();
	fractureBufferIndices.clear();
	cachedChunkTransforms.clear();
}

const SyncParams & DestructibleActor::getSyncParams() const
{
    return mSyncParams;
}

SyncParams & DestructibleActor::getSyncParamsMutable()
{
    return const_cast<SyncParams&>(getSyncParams());
}

// Renderable support:

NxDestructibleRenderable* DestructibleActor::acquireRenderableReference()
{
	return mRenderable ? mRenderable->incrementReferenceCount() : NULL;
}

NxRenderMeshActor* DestructibleActor::getRenderMeshActor(NxDestructibleActorMeshType::Enum type) const
{
	return mRenderable ? mRenderable->getRenderMeshActor(type) : NULL;
}

}
}
} // end namespace physx::apex

#endif
