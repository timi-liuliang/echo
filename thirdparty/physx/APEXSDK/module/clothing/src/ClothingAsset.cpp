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

#include "NxClothingAsset.h"

#include "ApexAuthorableObject.h"
#include "ClothingActorProxy.h"
#include "ClothingAsset.h"
#include "ClothingAssetData.h"
#include "ClothingGlobals.h"
#include "ClothingPhysicalMesh.h"
#include "ClothingPreviewProxy.h"
#include "ClothingScene.h"
#include "CookingPhysX.h"
#include "ModulePerfScope.h"
#include "NxParamUtils.h"

#include "ApexMath.h"
#include "ClothingActorParam.h"
#include "ClothingAssetParameters.h"
#include "ClothingGraphicalLodParameters.h"
#include "ModuleClothingHelpers.h"

#include "foundation/PxStrideIterator.h"
#include "PsFastMemory.h"
#include "PsSort.h"
#include "PsThread.h"
#include "PsShare.h"

#include "PsVecMath.h"
using namespace physx::shdfnd::aos;

#include "SimulationAbstract.h"
#include "ApexPermute.h"

#include "PvdDataStream.h"
#include "PVDBinding.h"
using namespace physx::debugger;
using namespace physx::debugger::comm;

#define PX_SIMD_SKINNING 1

#pragma warning( disable: 4101 ) // PX_COMPILE_TIME_ASSERT causes these warnings since they are
// used within our apex namespace

namespace physx
{
namespace apex
{
namespace clothing
{

NxAuthObjTypeID ClothingAsset::mAssetTypeID = 0xffffffff;

ClothingAsset::ClothingAsset(ModuleClothing* module, NxResourceList& list, const char* name) :
	mModule(module),
	mParams(DYNAMIC_CAST(ClothingAssetParameters*)(NiGetApexSDK()->getParameterizedTraits()->createNxParameterized(ClothingAssetParameters::staticClassName()))),
	mPhysicalMeshes(mParams, "physicalMeshes", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->physicalMeshes)),
	mGraphicalLods(mParams, "graphicalLods", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->graphicalLods)),
	mBones(mParams, "bones", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->bones)),
	mBoneSpheres(mParams, "boneSpheres", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->boneSpheres)),
	mSpherePairs(mParams, "boneSphereConnections", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->boneSphereConnections)),
	mBoneActors(mParams, "boneActors", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->boneActors)),
	mBoneVertices(mParams, "boneVertices", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->boneVertices)),
	mBonePlanes(mParams, "bonePlanes", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->bonePlanes)),
	mCollisionConvexes(mParams, "collisionConvexes", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->collisionConvexes)),
	mName(name),
	mExt2IntMorphMappingMaxValue(0),
	mDirty(false),
	mMorphMappingWarning(false)
{
	// this constructor is only executed when initializing the authoring asset
	list.add(*this);

#ifndef WITHOUT_PVD
	mActors.setupForPvd(static_cast<NxApexResource*>(this), "NxClothingActors", "NxClothingActor");
#endif

	// make sure these two methods are compiled!
	NxAbstractMeshDescription pcm;
	pcm.avgEdgeLength = 0.1f;
}


ClothingAsset::ClothingAsset(ModuleClothing* module, NxResourceList& list, NxParameterized::Interface* params, const char* name) :
	mModule(module),
	mParams(NULL),
	mName(name),
	mExt2IntMorphMappingMaxValue(0),
	mDirty(false),
	mMorphMappingWarning(false)
{

#ifndef WITHOUT_PVD
	mActors.setupForPvd(static_cast<NxApexResource*>(this), "NxClothingActors", "NxClothingActor");
#endif

	// wrong name?
	if (params != NULL && strcmp(params->className(), ClothingAssetParameters::staticClassName()) != 0)
	{
		APEX_INTERNAL_ERROR(
		    "The parameterized interface is of type <%s> instead of <%s>.  "
		    "This object will be initialized by an empty one instead!",
		    params->className(),
		    ClothingAssetParameters::staticClassName());

		params->destroy();
		params = NULL;
	}
	else if (params != NULL)
	{
		ClothingAssetParameters* checkParams = DYNAMIC_CAST(ClothingAssetParameters*)(params);

		PxU32 boneRefsMesh = 0, boneRefsRB = 0;
		for (int i = 0; i < checkParams->bones.arraySizes[0]; i++)
		{
			boneRefsMesh += checkParams->bones.buf[i].numMeshReferenced;
			boneRefsRB += checkParams->bones.buf[i].numRigidBodiesReferenced;
		}

		if (checkParams->bones.arraySizes[0] > 0 && (boneRefsRB + boneRefsMesh == 0))
		{
			APEX_INTERNAL_ERROR(
			    "This parameterized object has not been prepared before serialization.  "
			    "It will not be able to work and has been replaced by an empty one instead.  "
			    "See NxParameterized::Interface::callPreSerializeCallback()");

			params->destroy();
			params = NULL;
		}
	}

	if (params == NULL)
	{
		params = DYNAMIC_CAST(ClothingAssetParameters*)(NiGetApexSDK()->getParameterizedTraits()->createNxParameterized(ClothingAssetParameters::staticClassName()));
	}


	PX_ASSERT(strcmp(params->className(), ClothingAssetParameters::staticClassName()) == 0);
	if (strcmp(params->className(), ClothingAssetParameters::staticClassName()) == 0)
	{
		mParams = static_cast<ClothingAssetParameters*>(params);
		mParams->setSerializationCallback(this, NULL);

		bool ok = false;
		PX_UNUSED(ok);

		ok = mPhysicalMeshes.init(mParams, "physicalMeshes", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->physicalMeshes));
		PX_ASSERT(ok);
		ok = mGraphicalLods.init(mParams, "graphicalLods", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->graphicalLods));
		PX_ASSERT(ok);
		ok = mBones.init(mParams, "bones", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->bones));
		PX_ASSERT(ok);
		ok = mBoneSpheres.init(mParams, "boneSpheres", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->boneSpheres));
		PX_ASSERT(ok);
		ok = mSpherePairs.init(mParams, "boneSphereConnections", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->boneSphereConnections));
		PX_ASSERT(ok);
		ok = mBoneActors.init(mParams, "boneActors", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->boneActors));
		PX_ASSERT(ok);
		ok = mBoneVertices.init(mParams, "boneVertices", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->boneVertices));
		PX_ASSERT(ok);
		ok = mBonePlanes.init(mParams, "bonePlanes", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->bonePlanes));
		PX_ASSERT(ok);
		ok = mCollisionConvexes.init(mParams, "collisionConvexes", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->collisionConvexes));
		PX_ASSERT(ok);

		for (PxU32 i = 0; i < mGraphicalLods.size(); i++)
		{
			if (mGraphicalLods[i]->renderMeshAsset == NULL)
				continue;

			char buf[128];
			size_t len = PxMin(mName.len(), 120u);
			buf[0] = 0;
			string::strcat_s(buf, len + 1, mName.c_str());
			buf[len] = '_';
			string::sprintf_s(buf + len + 1, 128 - len - 1, "%d", i);
			NxApexAsset* asset = NxGetApexSDK()->createAsset(mGraphicalLods[i]->renderMeshAsset, buf);
			PX_ASSERT(strcmp(asset->getObjTypeName(), NX_RENDER_MESH_AUTHORING_TYPE_NAME) == 0);

			NiApexRenderMeshAsset* rma = static_cast<NiApexRenderMeshAsset*>(asset);
			if (rma->mergeBinormalsIntoTangents())
			{
				mDirty = true;
				APEX_DEBUG_INFO("Performance warning. This asset <%s> has to be re-saved to speed up loading", name);
			}

			mGraphicalLods[i]->renderMeshAssetPointer = rma;
			mDirty |= reorderGraphicsVertices(i, i == 0); // only warn the first time
		}

		bool cookingInvalid = false;

		for (PxU32 i = 0; i < mPhysicalMeshes.size(); i++)
		{
			bool reorderVerts = false;
			for (PxI32 j = 0; j < mPhysicalMeshes[i]->submeshes.arraySizes[0]; j++)
			{
				if (mPhysicalMeshes[i]->submeshes.buf[j].numMaxDistance0Vertices == 0)
				{
					reorderVerts = true;
				}
			}

			if (reorderVerts)
			{
				ClothingPhysicalMesh* mesh = mModule->createPhysicalMeshInternal(mPhysicalMeshes[i]);

				if (mesh != NULL)
				{
					const bool changed = reorderDeformableVertices(*mesh);

					mesh->release();

					cookingInvalid |= changed;
					mDirty |= changed;
				}
			}
		}

		if (mParams->materialLibrary != NULL && cookingInvalid)
		{
			// So, if we turn on zerostretch also for the new solver, we should make sure the values are set soft enough not to introduce too much ghost forces
			ClothingMaterialLibraryParameters* matLib = static_cast<ClothingMaterialLibraryParameters*>(mParams->materialLibrary);

			for (PxI32 i = 0; i < matLib->materials.arraySizes[0]; i++)
			{
				PxF32& limit = matLib->materials.buf[i].hardStretchLimitation;

				if (limit >= 1.0f)
				{
					limit = PxMax(limit, 1.1f); // must be either 0 (disabled) or > 1.1 for stability
				}
			}
		}

		if (mParams->boundingBox.minimum.isZero() && mParams->boundingBox.maximum.isZero())
		{
			updateBoundingBox();
		}

		PxU32 cookNow = 0;
		const char* cookedDataClass = "Embedded";

		NxParamArray<ClothingAssetParametersNS::CookedEntry_Type> cookedEntries(mParams, "cookedData", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->cookedData));
		for (PxU32 i = 0; i < cookedEntries.size(); i++)
		{
			if (cookedEntries[i].cookedData != NULL)
			{
				BackendFactory* cookedDataBackend = mModule->getBackendFactory(cookedEntries[i].cookedData->className());
				PX_ASSERT(cookedDataBackend);
				if (cookedDataBackend != NULL)
				{
					// compare data version with current version of the data backend
					PxU32 cookedDataVersion = cookedDataBackend->getCookedDataVersion(cookedEntries[i].cookedData);
					PxU32 cookingVersion = cookedDataBackend->getCookingVersion();

					if (cookingVersion != cookedDataVersion || cookingInvalid
#if NX_SDK_VERSION_MAJOR == 3
					        // don't use native CookingPhysX, only use embedded solver
					        || cookedDataVersion == CookingPhysX::getCookingVersion()
#endif
					   )
					{
#if NX_SDK_VERSION_MAJOR == 2
						cookedDataClass = cookedDataVersion < 300 ? "Native" : "Embedded";
#endif
						cookNow = cookedDataVersion;
						cookedEntries[i].cookedData->destroy();
						cookedEntries[i].cookedData = NULL;
					}
				}
			}
		}
		if (cookNow != 0)
		{
			APEX_DEBUG_WARNING("Asset (%s) cooked data version (%d/0x%08x) does not match the current sdk version. Recooking.", name, cookNow, cookNow);
		}

		if (cookedEntries.isEmpty())
		{
			ClothingAssetParametersNS::CookedEntry_Type entry;
			entry.scale = 1.0f;
			entry.cookedData = NULL;
			cookedEntries.pushBack(entry);
			APEX_DEBUG_INFO("Asset (%s) has no cooked data and will be re-cooked every time it's loaded, asset needs to be resaved!", name);
			cookNow = 1;
		}

		if (cookNow != 0)
		{
			BackendFactory* cookingBackend = mModule->getBackendFactory(cookedDataClass);

			PX_ASSERT(cookingBackend != NULL);

			for (PxU32 i = 0; i < cookedEntries.size(); i++)
			{
				if (cookedEntries[i].cookedData == NULL && cookingBackend != NULL)
				{
					ClothingAssetParametersNS::CookedEntry_Type& entry = cookedEntries[i];

					CookingAbstract* cookingJob = cookingBackend->createCookingJob();
					prepareCookingJob(*cookingJob, entry.scale, NULL, NULL);

					if (cookingJob->isValid())
					{
						entry.cookedData = cookingJob->execute();
					}
					PX_DELETE_AND_RESET(cookingJob);
				}
			}

			mDirty = true;
		}
	}

	list.add(*this);
}



PxU32 ClothingAsset::forceLoadAssets()
{
	PxU32 assetLoadedCount = 0;

	for (PxU32 i = 0; i < mGraphicalLods.size(); i++)
	{
		if (mGraphicalLods[i]->renderMeshAssetPointer == NULL)
			continue;

		NiApexRenderMeshAsset* rma = reinterpret_cast<NiApexRenderMeshAsset*>(mGraphicalLods[i]->renderMeshAssetPointer);
		assetLoadedCount += rma->forceLoadAssets();
	}

	return assetLoadedCount;

}



NxParameterized::Interface* ClothingAsset::getDefaultActorDesc()
{
	NxParameterized::Interface* ret = NULL;

	if (mModule != NULL)
	{
		ret = mModule->getApexClothingActorParams();

		if (ret != NULL)
		{
			ret->initDefaults();

			// need to customize it per asset
			PX_ASSERT(strcmp(ret->className(), ClothingActorParam::staticClassName()) == 0);
			ClothingActorParam* actorDesc = static_cast<ClothingActorParam*>(ret);
			actorDesc->clothingMaterialIndex = mParams->materialIndex;

#ifdef PX_PS3
			actorDesc->flags.ParallelCpuSkinning = false; // true on other platforms, but on PS3 it's better done in fetch (SPU!)
#endif
		}
	}

	return ret;
}



NxParameterized::Interface* ClothingAsset::getDefaultAssetPreviewDesc()
{
	NxParameterized::Interface* ret = NULL;

	if (mModule != NULL)
	{
		ret = mModule->getApexClothingPreviewParams();

		if (ret != NULL)
		{
			ret->initDefaults();
		}
	}

	return ret;
}



NxApexActor* ClothingAsset::createApexActor(const NxParameterized::Interface& params, NxApexScene& apexScene)
{
	if (!isValidForActorCreation(params, apexScene))
	{
		return NULL;
	}
	ClothingActorProxy* proxy = NULL;
	ClothingScene* clothingScene =	mModule->getClothingScene(apexScene);
	proxy = PX_NEW(ClothingActorProxy)(params, this, *clothingScene, &mActors);
	PX_ASSERT(proxy != NULL);
	return proxy;
}


NxParameterized::Interface* ClothingAsset::releaseAndReturnNxParameterizedInterface()
{
	NxParameterized::Interface* ret = mParams;
	mParams->setSerializationCallback(NULL, NULL);
	mParams = NULL;
	release();
	return ret;
}



bool ClothingAsset::isValidForActorCreation(const NxParameterized::Interface& params, NxApexScene& apexScene) const
{
	bool ret = false;

	if (ClothingActor::isValidDesc(params))
	{
		ClothingScene* clothingScene =	mModule->getClothingScene(apexScene);
		if (clothingScene)
		{
			if (!clothingScene->isSimulating())
			{
				ret = true;
			}
			else
			{
				APEX_INTERNAL_ERROR("Cannot create NxClothingActor while simulation is running");
			}
		}
	}
	else
	{
		APEX_INVALID_PARAMETER("NxClothingActorDesc is invalid");
	}
	return ret;
}


bool ClothingAsset::isDirty() const
{
	return mDirty;
}



void ClothingAsset::release()
{
	mModule->mSdk->releaseAsset(*this);
}



NxClothingActor* ClothingAsset::getActor(PxU32 index)
{
	NX_READ_ZONE();
	if (index < mActors.getSize())
	{
		return DYNAMIC_CAST(ClothingActorProxy*)(mActors.getResource(index));
	}
	return NULL;
}



PxF32 ClothingAsset::getMaximumSimulationBudget(PxU32 solverIterations) const
{
	PxU32 maxCost = 0;
	for (PxU32 i = 0; i < mPhysicalMeshes.size(); i++)
	{
		const PxU32 numLods = (physx::PxU32)mPhysicalMeshes[i]->physicalLods.arraySizes[0];

		for (PxU32 j = 0; j < numLods; j++)
		{
			ClothingPhysicalMeshParametersNS::PhysicalLod_Type& lod = mPhysicalMeshes[i]->physicalLods.buf[j];

			const PxU32 iterations = (PxU32)(PxCeil(lod.solverIterationScale * solverIterations));
			maxCost = PxMax(maxCost, lod.costWithoutIterations * iterations);
		}
	}
	return maxCost * mModule->getLODUnitCost();
}



PxU32 ClothingAsset::getNumGraphicalLodLevels() const
{
	NX_READ_ZONE();
	return mGraphicalLods.size();
}



PxU32 ClothingAsset::getGraphicalLodValue(PxU32 lodLevel) const
{
	NX_READ_ZONE();
	if (lodLevel < mGraphicalLods.size())
	{
		return mGraphicalLods[lodLevel]->lod;
	}

	return PxU32(-1);
}



PxF32 ClothingAsset::getBiggestMaxDistance() const
{
	NX_READ_ZONE();
	PxF32 maxValue = 0.0f;
	for (PxU32 i = 0; i < mPhysicalMeshes.size(); i++)
	{
		maxValue = PxMax(maxValue, mPhysicalMeshes[i]->physicalMesh.maximumMaxDistance);
	}

	return maxValue;
}



bool ClothingAsset::remapBoneIndex(const char* name, PxU32 newIndex)
{
	NX_WRITE_ZONE();
	PxU32 found = 0;
	const PxU32 numBones = mBones.size();
	for (PxU32 i = 0; i < numBones; i++)
	{
		if (mBones[i].name != NULL && (strcmp(mBones[i].name, name) == 0))
		{
			mBones[i].externalIndex = (physx::PxI32)newIndex;
			found++;
		}
	}

	if (found > 1)
	{
		APEX_DEBUG_WARNING("The asset contains %i bones with name %s. All occurences were mapped.", found, name);
	}
	else if (found == 0)
	{
		APEX_DEBUG_INFO("The asset does not contain a bone with name %s", name);
	}

	return (found == 1); // sanity
}



const char*	ClothingAsset::getBoneName(PxU32 internalIndex) const
{
	NX_READ_ZONE();
	if (internalIndex >= mBones.size())
	{
		return "";
	}

	return mBones[internalIndex].name;
}



bool ClothingAsset::getBoneBasePose(PxU32 internalIndex, PxMat44& result) const
{
	NX_READ_ZONE();
	if (internalIndex < mBones.size())
	{
		result = mBones[internalIndex].bindPose;
		return true;
	}
	return false;
}



void ClothingAsset::getBoneMapping(PxU32* internal2externalMap) const
{
	NX_READ_ZONE();
	for (PxU32 i = 0; i < mBones.size(); i++)
	{
		internal2externalMap[(physx::PxU32)mBones[i].internalIndex] = (physx::PxU32)mBones[i].externalIndex;
	}
}



PxU32 ClothingAsset::prepareMorphTargetMapping(const PxVec3* originalPositions, PxU32 numPositions, PxF32 epsilon)
{
	NX_WRITE_ZONE();
	PxU32 numInternalVertices = 0;
	for (PxU32 i = 0; i < mGraphicalLods.size(); i++)
	{
		NiApexRenderMeshAsset* rma = getGraphicalMesh(i);
		if (rma == NULL)
			continue;

		for (PxU32 s = 0; s < rma->getSubmeshCount(); s++)
		{
			numInternalVertices += rma->getSubmesh(s).getVertexCount(0);
		}
	}
	numInternalVertices += mBoneVertices.size();

	mExt2IntMorphMapping.resize(numInternalVertices);
	PxU32 indexWritten = 0;

	PxU32 numLarger = 0;
	PxF32 epsilon2 = epsilon * epsilon;

	for (PxU32 i = 0; i < mGraphicalLods.size(); i++)
	{
		NiApexRenderMeshAsset* rma = getGraphicalMesh(i);
		if (rma == NULL)
			continue;

		for (PxU32 s = 0; s < rma->getSubmeshCount(); s++)
		{
			const PxU32 numVertices = rma->getSubmesh(s).getVertexCount(0);
			const NxVertexFormat& format = rma->getSubmesh(s).getVertexBuffer().getFormat();
			const PxU32 positionIndex = (physx::PxU32)format.getBufferIndexFromID(format.getSemanticID(NxRenderVertexSemantic::POSITION));
			if (format.getBufferFormat(positionIndex) == NxRenderDataFormat::FLOAT3)
			{
				PxVec3* positions = (PxVec3*)rma->getSubmesh(s).getVertexBuffer().getBuffer(positionIndex);
				for (PxU32 v = 0; v < numVertices; v++)
				{
					PxF32 closestDist2 = PX_MAX_F32;
					PxU32 closestIndex = (PxU32) - 1;
					for (PxU32 iv = 0; iv < numPositions; iv++)
					{
						PxF32 dist2 = (originalPositions[iv] - positions[v]).magnitudeSquared();
						if (dist2 < closestDist2)
						{
							closestDist2 = dist2;
							closestIndex = iv;
						}
					}
					PX_ASSERT(closestIndex != (PxU32) - 1);
					mExt2IntMorphMapping[indexWritten++] = closestIndex;
					numLarger += closestDist2 < epsilon2 ? 0 : 1;
				}
			}
			else
			{
				PX_ALWAYS_ASSERT();
			}
		}
	}

	for (PxU32 i = 0; i < mBoneVertices.size(); i++)
	{
		PxF32 closestDist2 = PX_MAX_F32;
		PxU32 closestIndex = (PxU32) - 1;
		for (PxU32 iv = 0; iv < numPositions; iv++)
		{
			PxF32 dist2 = (originalPositions[iv] - mBoneVertices[i]).magnitudeSquared();
			if (dist2 < closestDist2)
			{
				closestDist2 = dist2;
				closestIndex = iv;
			}
		}
		PX_ASSERT(closestIndex != (PxU32) - 1);
		mExt2IntMorphMapping[indexWritten++] = closestIndex;
		numLarger += closestDist2 < epsilon2 ? 0 : 1;
	}

	PX_ASSERT(indexWritten == numInternalVertices);
	mExt2IntMorphMappingMaxValue = numPositions;

	return numLarger;
}



void ClothingAsset::preSerialize(void* /*userData*/)
{
	mDirty = false;
}



NiApexRenderMeshAsset* ClothingAsset::getGraphicalMesh(PxU32 index)
{
	if (index < mGraphicalLods.size())
	{
		return reinterpret_cast<NiApexRenderMeshAsset*>(mGraphicalLods[index]->renderMeshAssetPointer);
	}

	return NULL;
}



const ClothingGraphicalLodParameters* ClothingAsset::getGraphicalLod(PxU32 index) const
{
	if (index < mGraphicalLods.size())
	{
		return mGraphicalLods[index];
	}

	return NULL;
}


void ClothingAsset::releaseClothingActor(NxClothingActor& actor)
{
	ClothingActorProxy* proxy = DYNAMIC_CAST(ClothingActorProxy*)(&actor);
	proxy->destroy();
}



void ClothingAsset::releaseClothingPreview(NxClothingPreview& preview)
{
	ClothingPreviewProxy* proxy = DYNAMIC_CAST(ClothingPreviewProxy*)(&preview);
	proxy->destroy();
}



bool ClothingAsset::writeBoneMatrices(PxMat44 localPose, const PxMat44* newBoneMatrices, const PxU32 byteStride,
                                      const PxU32 numBones, PxMat44* dest, bool isInternalOrder, bool multInvBindPose)
{
	PX_PROFILER_PERF_SCOPE("ClothingAsset::writeBoneMatrices");

	PX_ASSERT(byteStride >= sizeof(PxMat44));

	bool changed = false;

	if (mBones.isEmpty())
	{
		APEX_INTERNAL_ERROR("bone map is empty, this is a error condition");
	}
	else
	{
		// PH: if bones are present, but not set, we just have to write them with global pose
		const PxU8* src = (const PxU8*)newBoneMatrices;
		const PxU32 numBonesReferenced = mParams->bonesReferenced;
		for (PxU32 i = 0; i < mBones.size(); i++)
		{
			PX_ASSERT(mBones[i].internalIndex < (PxI32)mBones.size());
			PX_ASSERT(isInternalOrder || mBones[i].externalIndex >= 0);
			const PxU32 internalIndex = i;
			const PxU32 externalIndex = isInternalOrder ? i : mBones[i].externalIndex;
			if (internalIndex < numBonesReferenced && mBones[i].internalIndex >= 0)
			{
				PxMat44& oldMat = dest[internalIndex];
				PX_ALIGN(16, PxMat44) newMat;

				if (src != NULL && externalIndex < numBones)
				{
					PxMat44 skinningTransform = *(const PxMat44*)(src + byteStride * externalIndex);
					if (multInvBindPose)
					{
						skinningTransform = skinningTransform * mInvBindPoses[internalIndex];
					}
					newMat = localPose * skinningTransform;
				}
				else
				{
					newMat = localPose;
				}

				if (newMat != oldMat) // PH: let's hope this comparison is not too slow
				{
					changed |= (i < numBonesReferenced);
					oldMat = newMat;
				}
			}
		}
	}
	return changed;
}



ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* ClothingAsset::getPhysicalMeshFromLod(PxU32 graphicalLodId) const
{
	const PxU32 physicalMeshId = mGraphicalLods[graphicalLodId]->physicalMeshId;

	// -1 is also bigger than mPhysicalMeshes.size() for an unsigned
	if (physicalMeshId >= mPhysicalMeshes.size())
	{
		return NULL;
	}

	return &mPhysicalMeshes[physicalMeshId]->physicalMesh;
}



#if NX_SDK_VERSION_MAJOR == 2
void ClothingAsset::hintSceneDeletion(NxScene* deletedScene)
#elif NX_SDK_VERSION_MAJOR == 3
void ClothingAsset::hintSceneDeletion(PxScene* deletedScene)
#endif
{
	mUnusedSimulationMutex.lock();

	for (PxI32 i = (physx::PxI32)mUnusedSimulation.size() - 1; i >= 0; i--)
	{
		if (mUnusedSimulation[(physx::PxU32)i]->dependsOnScene(deletedScene))
		{
			destroySimulation(mUnusedSimulation[(physx::PxU32)i]);
			mUnusedSimulation.replaceWithLast((physx::PxU32)i);
		}
	}

	mUnusedSimulationMutex.unlock();
}



ClothingPhysicalMeshParametersNS::SkinClothMapB_Type* ClothingAsset::getTransitionMapB(PxU32 dstPhysicalMeshId, PxU32 srcPhysicalMeshId, PxF32& thickness, PxF32& offset)
{
	if (srcPhysicalMeshId + 1 == dstPhysicalMeshId)
	{
		thickness = mPhysicalMeshes[dstPhysicalMeshId]->transitionDownThickness;
		offset = mPhysicalMeshes[dstPhysicalMeshId]->transitionDownOffset;
		return mPhysicalMeshes[dstPhysicalMeshId]->transitionDownB.buf;
	}
	else if (srcPhysicalMeshId == dstPhysicalMeshId + 1)
	{
		thickness = mPhysicalMeshes[dstPhysicalMeshId]->transitionUpThickness;
		offset = mPhysicalMeshes[dstPhysicalMeshId]->transitionUpOffset;
		return mPhysicalMeshes[dstPhysicalMeshId]->transitionUpB.buf;
	}

	thickness = 0.0f;
	offset = 0.0f;
	return NULL;
}



ClothingPhysicalMeshParametersNS::SkinClothMapD_Type* ClothingAsset::getTransitionMap(PxU32 dstPhysicalMeshId, PxU32 srcPhysicalMeshId, PxF32& thickness, PxF32& offset)
{
	if (srcPhysicalMeshId + 1 == dstPhysicalMeshId)
	{
		thickness = mPhysicalMeshes[dstPhysicalMeshId]->transitionDownThickness;
		offset = mPhysicalMeshes[dstPhysicalMeshId]->transitionDownOffset;
		return mPhysicalMeshes[dstPhysicalMeshId]->transitionDown.buf;
	}
	else if (srcPhysicalMeshId == dstPhysicalMeshId + 1)
	{
		thickness = mPhysicalMeshes[dstPhysicalMeshId]->transitionUpThickness;
		offset = mPhysicalMeshes[dstPhysicalMeshId]->transitionUpOffset;
		return mPhysicalMeshes[dstPhysicalMeshId]->transitionUp.buf;
	}

	thickness = 0.0f;
	offset = 0.0f;
	return NULL;
}



NxParameterized::Interface* ClothingAsset::getCookedData(PxF32 actorScale)
{
	NxParameterized::Interface* closest = NULL;
	PxF32 closestDiff = PX_MAX_F32;

	for (PxI32 i = 0; i < mParams->cookedData.arraySizes[0]; i++)
	{
		NxParameterized::Interface* cookedData = mParams->cookedData.buf[i].cookedData;
		const PxF32 cookedDataScale = mParams->cookedData.buf[i].scale;

		if (cookedData == NULL)
		{
			continue;
		}

#ifdef _DEBUG
		if (strcmp(cookedData->className(), ClothingCookedParam::staticClassName()) == 0)
		{
			// silly debug verification
			PX_ASSERT(cookedDataScale == ((ClothingCookedParam*)cookedData)->actorScale);
		}
#endif

		const PxF32 scaleDiff = PxAbs(actorScale - cookedDataScale);
		if (scaleDiff < closestDiff)
		{
			closest = cookedData;
			closestDiff = scaleDiff;
		}
	}

	if (closest != NULL && closestDiff < 0.01f)
	{
		return closest;
	}

	return NULL;
}



PxU32 ClothingAsset::getCookedPhysXVersion() const
{
	PxU32 version = 0;

	for (PxI32 i = 0; i < mParams->cookedData.arraySizes[0]; i++)
	{
		const NxParameterized::Interface* cookedData = mParams->cookedData.buf[i].cookedData;
		if (cookedData != NULL)
		{
			const char* className = cookedData->className();
			BackendFactory* factory = mModule->getBackendFactory(className);

			PxU32 v = factory->getCookedDataVersion(cookedData);
			PX_ASSERT(version == 0 || version == v);
			version = v;
		}
	}

	// version == 0 usually means that there is no maxdistance > 0 at all!
	//PX_ASSERT(version != 0);
	return version;
}



ClothSolverMode::Enum ClothingAsset::getClothSolverMode() const
{
	NX_READ_ZONE();
	ClothSolverMode::Enum mode = ClothSolverMode::v3;
	PxU32 v = getCookedPhysXVersion();
	if (v < 300)
	{
		mode = ClothSolverMode::v2;
	}

	return mode;
}



SimulationAbstract* ClothingAsset::getSimulation(PxU32 physicalMeshId, PxU32 submeshId, NxParameterized::Interface* cookedParam, ClothingScene* clothingScene)
{
	SimulationAbstract* result = NULL;

	mUnusedSimulationMutex.lock();

	for (PxU32 i = 0; i < mUnusedSimulation.size(); i++)
	{
		PX_ASSERT(mUnusedSimulation[i] != NULL);
		if (mUnusedSimulation[i]->physicalMeshId != physicalMeshId)
		{
			continue;
		}

		if (mUnusedSimulation[i]->submeshId != submeshId)
		{
			continue;
		}

		if (mUnusedSimulation[i]->getCookedData() != cookedParam)
		{
			continue;
		}

		if (mUnusedSimulation[i]->getClothingScene() != clothingScene)
		{
			continue;
		}

		// we found one
		result = mUnusedSimulation[i];
		for (PxU32 j = i + 1; j < mUnusedSimulation.size(); j++)
		{
			mUnusedSimulation[j - 1] = mUnusedSimulation[j];
		}

		mUnusedSimulation.popBack();
		break;
	}

	mUnusedSimulationMutex.unlock();

	return result;
}



void ClothingAsset::returnSimulation(SimulationAbstract* simulation)
{
	PX_ASSERT(simulation != NULL);

	bool isAssetParam = false;
	for (PxI32 i = 0; i < mParams->cookedData.arraySizes[0]; i++)
	{
		isAssetParam |= mParams->cookedData.buf[i].cookedData == simulation->getCookedData();
	}

	physx::Mutex::ScopedLock scopeLock(mUnusedSimulationMutex);

	if (mModule->getMaxUnusedPhysXResources() == 0 || !isAssetParam || !simulation->needsExpensiveCreation())
	{
		destroySimulation(simulation);
		return;
	}

	if (mUnusedSimulation.size() > mModule->getMaxUnusedPhysXResources())
	{
		destroySimulation(mUnusedSimulation[0]);

		for (PxU32 i = 1; i < mUnusedSimulation.size(); i++)
		{
			mUnusedSimulation[i - 1] = mUnusedSimulation[i];
		}

		mUnusedSimulation[mUnusedSimulation.size() - 1] = simulation;
	}
	else
	{
		mUnusedSimulation.pushBack(simulation);
	}

	simulation->disablePhysX(mModule->getDummyActor());
}



void ClothingAsset::destroySimulation(SimulationAbstract* simulation)
{
	PX_ASSERT(simulation != NULL);

	simulation->unregisterPhysX();
	PX_DELETE_AND_RESET(simulation);
}



void ClothingAsset::initCollision(SimulationAbstract* simulation, const PxMat44* boneTansformations,
								  NxResourceList& actorPlanes,
								  NxResourceList& actorConvexes,
								  NxResourceList& actorSpheres,
								  NxResourceList& actorCapsules,
								  NxResourceList& actorTriangleMeshes,
								  const ClothingActorParam* actorParam,
								  const PxMat44& globalPose, bool localSpaceSim)
{
	simulation->initCollision(	mBoneActors.begin(), mBoneActors.size(), mBoneSpheres.begin(), mBoneSpheres.size(), mSpherePairs.begin(), mSpherePairs.size(), mBonePlanes.begin(), mBonePlanes.size(), mCollisionConvexes.begin(), mCollisionConvexes.size(), mBones.begin(), boneTansformations,
								actorPlanes, actorConvexes, actorSpheres, actorCapsules, actorTriangleMeshes, 
								actorParam->actorDescTemplate, actorParam->shapeDescTemplate, actorParam->actorScale,
								globalPose, localSpaceSim);
}



void ClothingAsset::updateCollision(SimulationAbstract* simulation, const PxMat44* boneTansformations,
									NxResourceList& actorPlanes,
									NxResourceList& actorConvexes,
									NxResourceList& actorSpheres,
									NxResourceList& actorCapsules,
									NxResourceList& actorTriangleMeshes,
									bool teleport)
{
	simulation->updateCollision(mBoneActors.begin(), mBoneActors.size(), mBoneSpheres.begin(), mBoneSpheres.size(), mBonePlanes.begin(), mBonePlanes.size(), mBones.begin(), boneTansformations,
								actorPlanes, actorConvexes, actorSpheres, actorCapsules, actorTriangleMeshes, teleport);
}



PxU32 ClothingAsset::getNumPhysicalLods(PxU32 graphicalLodId) const
{
	if (graphicalLodId >= mGraphicalLods.size())
	{
		return 0;
	}

	const PxU32 physicalMeshId = mGraphicalLods[graphicalLodId]->physicalMeshId;
	if (physicalMeshId >= mPhysicalMeshes.size())
	{
		return 0;
	}

	return (physx::PxU32)mPhysicalMeshes[physicalMeshId]->physicalLods.arraySizes[0];
}



const PhysicalLod* ClothingAsset::getPhysicalLodData(PxU32 graphicalLodId, PxU32 physicsLod) const
{
	if (graphicalLodId >= mGraphicalLods.size())
	{
		return NULL;
	}

	PxU32 physicalMeshId = mGraphicalLods[graphicalLodId]->physicalMeshId;
	if (physicalMeshId >= mPhysicalMeshes.size())
	{
		return NULL;
	}

	if ((int)physicsLod >= mPhysicalMeshes[physicalMeshId]->physicalLods.arraySizes[0])
	{
		return NULL;
	}

	return &mPhysicalMeshes[physicalMeshId]->physicalLods.buf[physicsLod];
}



const PhysicalSubmesh* ClothingAsset::getPhysicalSubmesh(PxU32 graphicalLodId, PxU32 submeshId) const
{
	if (graphicalLodId >= mGraphicalLods.size())
	{
		return NULL;
	}

	const PxU32 physicalMeshId = mGraphicalLods[graphicalLodId]->physicalMeshId;
	if (physicalMeshId == (PxU32) - 1 || physicalMeshId >= mPhysicalMeshes.size())
	{
		return NULL;
	}

	if ((int)submeshId >= mPhysicalMeshes[physicalMeshId]->submeshes.arraySizes[0])
	{
		return NULL;
	}

	return &mPhysicalMeshes[physicalMeshId]->submeshes.buf[submeshId];
}



PxU32 ClothingAsset::getPhysicalMeshID(PxU32 graphicalLodId) const
{
	if (graphicalLodId >= mGraphicalLods.size())
	{
		return (PxU32) - 1;
	}

	return mGraphicalLods[graphicalLodId]->physicalMeshId;
}



void ClothingAsset::visualizeSkinCloth(NiApexRenderDebug& renderDebug, NxAbstractMeshDescription& srcPM, bool showTets, PxF32 actorScale) const
{
#ifdef WITHOUT_DEBUG_VISUALIZE
	PX_UNUSED(renderDebug);
	PX_UNUSED(srcPM);
	PX_UNUSED(showTets);
	PX_UNUSED(actorScale);
#else
	if ((srcPM.pPosition != NULL) && (srcPM.pIndices != NULL) && (srcPM.pNormal != NULL))
	{
		renderDebug.pushRenderState();

		renderDebug.removeFromCurrentState(DebugRenderState::SolidShaded);

		const PxU32 colorWhite = renderDebug.getDebugColor(DebugColors::White);
		const PxU32 colorRed = renderDebug.getDebugColor(DebugColors::Red);
		const PxU32 colorDarkRed = renderDebug.getDebugColor(DebugColors::DarkRed);
		const PxU32 colorGreen = renderDebug.getDebugColor(DebugColors::Green);
		const PxU32 colorPurple = renderDebug.getDebugColor(DebugColors::Purple);

		const PxF32 meshThickness = srcPM.avgEdgeLength * actorScale;
		// for each triangle in the Physical Mesh draw lines delimiting all the tetras
		for (PxU32 i = 0; i < srcPM.numIndices; i += 3)
		{
			if (showTets)
			{
				PxVec3 vtx[3], nrm[3];
				getNormalsAndVerticesForFace(vtx, nrm, i, srcPM);

				// draw lines for all edges of each tetrahedron (sure, there are redundant lines, but this is for debugging purposes)
				for (PxU32 tIdx = 0; tIdx < TETRA_LUT_SIZE; tIdx++)
				{
					// compute the tetra vertices based on the index
					const TetraEncoding& tetEnc = tetraTable[tIdx];
					PxVec3 tv0 = vtx[0] + (tetEnc.sign[0] * nrm[0] * meshThickness);
					PxVec3 tv1 = vtx[1] + (tetEnc.sign[1] * nrm[1] * meshThickness);
					PxVec3 tv2 = vtx[2] + (tetEnc.sign[2] * nrm[2] * meshThickness);
					PxVec3 tv3 = vtx[tetEnc.lastVtxIdx] + (tetEnc.sign[3] * nrm[tetEnc.lastVtxIdx] * meshThickness);

					PxU32 color  = tIdx < 3 ? colorGreen : colorPurple;

					renderDebug.setCurrentColor(color);
					renderDebug.debugLine(tv1, tv2);
					renderDebug.debugLine(tv2, tv3);
					renderDebug.debugLine(tv3, tv1);
					renderDebug.debugLine(tv0, tv1);
					renderDebug.debugLine(tv0, tv2);
					renderDebug.debugLine(tv0, tv3);
				}
			}
			else
			{
				PxU32 idx[3] =
				{
					srcPM.pIndices[i + 0],
					srcPM.pIndices[i + 1],
					srcPM.pIndices[i + 2],
				};

				PxVec3 vtx[3], nrm[3];
				for (PxU32 u = 0; u < 3; u++)
				{
					vtx[u] = srcPM.pPosition[idx[u]];
					nrm[u] = srcPM.pNormal[idx[u]] * meshThickness;
				}

				renderDebug.setCurrentColor(colorWhite);
				renderDebug.debugTri(vtx[0], vtx[1], vtx[2]);

				renderDebug.setCurrentColor(colorGreen);
				renderDebug.debugTri(vtx[0] + nrm[0], vtx[1] + nrm[1], vtx[2] + nrm[2]);
				for (PxU32 u = 0; u < 3; u++)
				{
					renderDebug.debugLine(vtx[u], vtx[u] + nrm[u]);
				}

				renderDebug.setCurrentColor(colorPurple);
				renderDebug.debugTri(vtx[0] - nrm[0], vtx[1] - nrm[1], vtx[2] - nrm[2]);
				for (PxU32 u = 0; u < 3; u++)
				{
					renderDebug.debugLine(vtx[u], vtx[u] - nrm[u]);
				}
			}
		}

#if 1
		// display some features of the physical mesh as it's updated at runtime

		srcPM.UpdateDerivedInformation(&renderDebug);

		// draw the mesh's bounding box
		PxBounds3 bounds(srcPM.pMin, srcPM.pMax);
		renderDebug.setCurrentColor(colorRed);
		renderDebug.debugBound(bounds);

		// draw line from the centroid to the top-most corner of the AABB
		renderDebug.setCurrentColor(colorDarkRed);
		renderDebug.debugLine(srcPM.centroid, srcPM.pMax);

		// draw white line along the same direction (but negated) to display the avgEdgeLength
		PxVec3 dirToPMax = (srcPM.pMax - srcPM.centroid);
		dirToPMax.normalize();
		PxVec3 pEdgeLengthAway = srcPM.centroid - (dirToPMax * srcPM.avgEdgeLength);
		renderDebug.setCurrentColor(colorWhite);
		renderDebug.debugLine(srcPM.centroid, pEdgeLengthAway);

		// draw green line along the same direction to display the physical mesh thickness
		PxVec3 pPmThicknessAway = srcPM.centroid + (dirToPMax * meshThickness);
		renderDebug.setCurrentColor(colorGreen);
		renderDebug.debugLine(srcPM.centroid, pPmThicknessAway);
#endif

		renderDebug.popRenderState();
	}
#endif
}



void ClothingAsset::visualizeSkinClothMap(NiApexRenderDebug& renderDebug, NxAbstractMeshDescription& srcPM,
		SkinClothMapB* skinClothMapB, PxU32 skinClothMapBSize,
		SkinClothMap* skinClothMap, PxU32 skinClothMapSize,
		PxF32 actorScale, bool onlyBad, bool invalidBary) const
{
#ifdef WITHOUT_DEBUG_VISUALIZE
	PX_UNUSED(renderDebug);
	PX_UNUSED(srcPM);
	PX_UNUSED(skinClothMapB);
	PX_UNUSED(skinClothMapBSize);
	PX_UNUSED(skinClothMap);
	PX_UNUSED(skinClothMapSize);
	PX_UNUSED(actorScale);
	PX_UNUSED(onlyBad);
	PX_UNUSED(invalidBary);
#else
	renderDebug.pushRenderState();

	renderDebug.removeFromCurrentState(DebugRenderState::SolidShaded);
	const PxF32 meshThickness = srcPM.avgEdgeLength * actorScale;

	if ((skinClothMapB != NULL) && (srcPM.pPosition != NULL) && (srcPM.pIndices != NULL) && (srcPM.pNormal != NULL))
	{
		const PxU32 colorRed = renderDebug.getDebugColor(DebugColors::Red);
		const PxU32 colorDarkRed = renderDebug.getDebugColor(DebugColors::DarkRed);
		const PxU32 colorBlue = renderDebug.getDebugColor(DebugColors::Blue);
		const PxU32 colorYellow = renderDebug.getDebugColor(DebugColors::Yellow);
		const PxU32 colorGreen = renderDebug.getDebugColor(DebugColors::Green);
		const PxU32 colorPurple = renderDebug.getDebugColor(DebugColors::Purple);

		for (PxU32 i = 0; i < skinClothMapBSize; i++)
		{
			const SkinClothMapB& mapping = skinClothMapB[i];

			if (mapping.faceIndex0 >= srcPM.numIndices)
			{
				break;
			}

			const PxVec3 vtb = mapping.vtxTetraBary;
			const PxF32 vtb_w = 1.0f - vtb.x - vtb.y - vtb.z;
			const bool badVtx =
			    vtb.x < 0.0f || vtb.x > 1.0f ||
			    vtb.y < 0.0f || vtb.y > 1.0f ||
			    vtb.z < 0.0f || vtb.z > 1.0f ||
			    vtb_w < 0.0f || vtb_w > 1.0f;

			const PxVec3 ntb = mapping.nrmTetraBary;
			const PxF32 ntb_w = 1.0f - ntb.x - ntb.y - ntb.z;

			const bool badNrm =
			    ntb.x < 0.0f || ntb.x > 1.0f ||
			    ntb.y < 0.0f || ntb.y > 1.0f ||
			    ntb.z < 0.0f || ntb.z > 1.0f ||
			    ntb_w < 0.0f || ntb_w > 1.0f;

			if (!onlyBad || badVtx || badNrm)
			{
				PxVec3 vtx[3], nrm[3];
				getNormalsAndVerticesForFace(vtx, nrm, mapping.faceIndex0, srcPM);

				const TetraEncoding& tetEnc = tetraTable[mapping.tetraIndex];

				const PxVec3 tv0 = vtx[0] + (tetEnc.sign[0] * nrm[0] * meshThickness);
				const PxVec3 tv1 = vtx[1] + (tetEnc.sign[1] * nrm[1] * meshThickness);
				const PxVec3 tv2 = vtx[2] + (tetEnc.sign[2] * nrm[2] * meshThickness);
				const PxVec3 tv3 = vtx[tetEnc.lastVtxIdx] + (tetEnc.sign[3] * nrm[tetEnc.lastVtxIdx] * meshThickness);

				const PxVec3 centroid = (tv0 + tv1 + tv2 + tv3) * 0.25f;
				const PxVec3 graphicsPos = (vtb.x * tv0) + (vtb.y * tv1) + (vtb.z * tv2) + (vtb_w * tv3);
				const PxVec3 graphicsNrm = (ntb.x * tv0) + (ntb.y * tv1) + (ntb.z * tv2) + (ntb_w * tv3);

				renderDebug.setCurrentColor(colorYellow);
				renderDebug.debugPoint(graphicsPos, meshThickness * 0.1f);
				renderDebug.setCurrentColor(colorBlue);
				renderDebug.debugLine(graphicsPos, graphicsNrm);

				if (badVtx && onlyBad)
				{
					renderDebug.setCurrentColor(colorRed);
					renderDebug.debugLine(centroid, graphicsPos);
				}

				if (badNrm && onlyBad)
				{
					renderDebug.setCurrentColor(colorDarkRed);
					renderDebug.debugLine(centroid, graphicsPos);
				}

				renderDebug.setCurrentColor(mapping.tetraIndex < 3 ? colorGreen : colorPurple);
				renderDebug.debugLine(tv1, tv2);
				renderDebug.debugLine(tv2, tv3);
				renderDebug.debugLine(tv3, tv1);
				renderDebug.debugLine(tv0, tv1);
				renderDebug.debugLine(tv0, tv2);
				renderDebug.debugLine(tv0, tv3);
			}
		}
	}
	else if ((skinClothMap != NULL) && (srcPM.pPosition != NULL) && (srcPM.pIndices != NULL) && (srcPM.pNormal != NULL))
	{
		const PxU32 colorWhite = renderDebug.getDebugColor(DebugColors::White);
		const PxU32 colorRed = renderDebug.getDebugColor(DebugColors::Red);
		const PxU32 colorDarkRed = renderDebug.getDebugColor(DebugColors::DarkRed);
		const PxU32 colorBlue = renderDebug.getDebugColor(DebugColors::Blue);
		const PxU32 colorYellow = renderDebug.getDebugColor(DebugColors::Yellow);
		const PxU32 colorGreen = renderDebug.getDebugColor(DebugColors::Green);
		const PxU32 colorPurple = renderDebug.getDebugColor(DebugColors::Purple);

		for (PxU32 i = 0; i < skinClothMapSize; i++)
		{
			const SkinClothMap& mapping = skinClothMap[i];

			if (mapping.vertexIndex0 >= srcPM.numVertices || mapping.vertexIndex1 >= srcPM.numVertices || mapping.vertexIndex2 >= srcPM.numVertices)
			{
				continue;
			}

			PxVec3 baryVtx = mapping.vertexBary;
			const PxF32 heightVtx = baryVtx.z;
			baryVtx.z = 1.0f - baryVtx.x - baryVtx.y;

			PxVec3 baryNrm = mapping.normalBary;
			const PxF32 heightNrm = baryNrm.z;
			baryNrm.z = 1.0f - baryNrm.x - baryNrm.y;

			bool badVtx =
				baryVtx.x <  0.0f || baryVtx.x > 1.0f ||
				baryVtx.y <  0.0f || baryVtx.y > 1.0f ||
				baryVtx.z <  0.0f || baryVtx.z > 1.0f ||
				heightVtx < -1.0f || heightVtx > 1.0f;

			bool badNrm =
				baryNrm.x < 0.0f  || baryNrm.x > 1.0f ||
				baryNrm.y < 0.0f  || baryNrm.y > 1.0f ||
				baryNrm.z < 0.0f  || baryNrm.z > 1.0f ||
				heightNrm < -1.0f || heightNrm > 1.0f;

			if (!onlyBad || badVtx || badNrm)
			{
				PxU32 idx[3] =
				{
					mapping.vertexIndex0,
					mapping.vertexIndex1,
					mapping.vertexIndex2,
				};

				PxVec3 vtx[3], nrm[3];
				PxVec3 centroid(0.0f);
				for (PxU32 j = 0; j < 3; j++)
				{
					vtx[j] = srcPM.pPosition[idx[j]];
					nrm[j] = srcPM.pNormal[idx[j]] * meshThickness;
					centroid += vtx[j];
				}
				centroid /= 3.0f;


				PxU32 b = (PxU32)(255 * i / skinClothMapSize);
				PxU32 color = (b << 16) + (b << 8) + b;
				renderDebug.setCurrentColor(color);

				PxVec3 vertexBary = mapping.vertexBary;
				PxF32 vertexHeight = vertexBary.z;
				vertexBary.z = 1.0f - vertexBary.x - vertexBary.y;
				const PxVec3 vertexPos = vertexBary.x * vtx[0] + vertexBary.y * vtx[1] + vertexBary.z * vtx[2];
				const PxVec3 vertexNrm = vertexBary.x * nrm[0] + vertexBary.y * nrm[1] + vertexBary.z * nrm[2]; // meshThickness is already in
				const PxVec3 graphicsPos = vertexPos + vertexNrm * vertexHeight;

				if (invalidBary)
				{
					PxU32 invalidColor = 0;
					invalidColor = vertexBary == PxVec3(PX_MAX_F32) ? colorRed : invalidColor;
					invalidColor = mapping.normalBary == PxVec3(PX_MAX_F32) ? colorPurple : invalidColor;
					invalidColor = mapping.tangentBary == PxVec3(PX_MAX_F32) ? colorBlue : invalidColor;

					if (invalidColor != 0)
					{
						renderDebug.setCurrentColor(invalidColor);
						renderDebug.debugTri(vtx[0], vtx[1], vtx[2]);
						renderDebug.debugPoint(graphicsPos, meshThickness * 0.1f);
						renderDebug.debugLine(centroid, graphicsPos);
					}
					continue;
				}

				renderDebug.debugLine(centroid, graphicsPos);

				renderDebug.setCurrentColor(colorYellow);
				renderDebug.debugPoint(graphicsPos, meshThickness * 0.1f);

				if (badVtx && onlyBad)
				{
					// draw the projected position as well
					renderDebug.setCurrentColor(colorRed);
					renderDebug.debugLine(vertexPos, graphicsPos);
					renderDebug.debugLine(vertexPos, centroid);
				}

				PxVec3 normalBary = mapping.normalBary;
				PxF32 normalHeight = normalBary.z;
				normalBary.z = 1.0f - normalBary.x - normalBary.y;
				const PxVec3 normalPos = normalBary.x * vtx[0] + normalBary.y * vtx[1] + normalBary.z * vtx[2];
				const PxVec3 normalNrm = normalBary.x * nrm[0] + normalBary.y * nrm[1] + normalBary.z * nrm[2]; // meshThickness is already in
				const PxVec3 graphicsNrm = normalPos + normalNrm * normalHeight;
				renderDebug.setCurrentColor(colorBlue);
				renderDebug.debugLine(graphicsNrm, graphicsPos);
#if 0
				renderDebug.setCurrentColor(renderDebug.getDebugColor(DebugColors::Black));
				renderDebug.debugLine(graphicsNrm, centroid);
#endif

				if (badNrm && onlyBad)
				{
					// draw the projected normal as well
					renderDebug.setCurrentColor(colorDarkRed);
					renderDebug.debugLine(normalPos, graphicsNrm);
					renderDebug.debugLine(normalPos, centroid);
				}

				// turn the rendering on for the rest
				badVtx = badNrm = true;

				renderDebug.setCurrentColor(colorWhite);
				renderDebug.debugTri(vtx[0], vtx[1], vtx[2]);
				if ((badVtx && heightVtx > 0.0f) || (badNrm && heightNrm > 0.0f))
				{
					renderDebug.setCurrentColor(colorGreen);
					renderDebug.debugTri(vtx[0] + nrm[0], vtx[1] + nrm[1], vtx[2] + nrm[2]);
					for (PxU32 u = 0; u < 3; u++)
					{
						renderDebug.debugLine(vtx[u], vtx[u] + nrm[u]);
					}
				}
				else if ((badVtx && heightVtx < 0.0f) || (badNrm && heightNrm < 0.0f))
				{
					renderDebug.setCurrentColor(colorPurple);
					renderDebug.debugTri(vtx[0] - nrm[0], vtx[1] - nrm[1], vtx[2] - nrm[2]);
					for (PxU32 u = 0; u < 3; u++)
					{
						renderDebug.debugLine(vtx[u], vtx[u] - nrm[u]);
					}
				}
			}
		}
	}

	renderDebug.popRenderState();
#endif
}



void ClothingAsset::visualizeBones(NiApexRenderDebug& renderDebug, const PxMat44* matrices, bool skeleton, PxF32 boneFramesScale, PxF32 boneNamesScale)
{
#ifdef WITHOUT_DEBUG_VISUALIZE
	PX_UNUSED(renderDebug);
	PX_UNUSED(matrices);
	PX_UNUSED(skeleton);
	PX_UNUSED(boneFramesScale);
	PX_UNUSED(boneNamesScale);
#else

	renderDebug.pushRenderState();

	const PxU32 activeBoneColor = renderDebug.getDebugColor(DebugColors::Purple);
	const PxU32 passiveBoneColor = renderDebug.getDebugColor(DebugColors::Blue);

	const PxU32 colorWhite = renderDebug.getDebugColor(DebugColors::White);
	const PxU32 colorRed = renderDebug.getDebugColor(DebugColors::Red);
	const PxU32 colorGreen = renderDebug.getDebugColor(DebugColors::Green);
	const PxU32 colorBlue = renderDebug.getDebugColor(DebugColors::Blue);

	renderDebug.addToCurrentState(physx::DebugRenderState::CenterText);
	renderDebug.addToCurrentState(physx::DebugRenderState::CameraFacing);
	renderDebug.setCurrentTextScale(boneNamesScale);

	if ((skeleton || boneFramesScale > 0.0f || boneNamesScale > 0.0f) && mPhysicalMeshes.size() > 0)
	{
		ClothingPhysicalMeshParametersNS::PhysicalMesh_Type& physicalMesh = mPhysicalMeshes[0]->physicalMesh;
		PxF32 sphereSize = 0.3f * physicalMesh.averageEdgeLength;
		PxU32 rootIdx = mParams->rootBoneIndex;
		PxMat34Legacy absPose = matrices[rootIdx] * mBones[rootIdx].bindPose;
		renderDebug.setCurrentColor(renderDebug.getDebugColor(DebugColors::DarkRed));
		renderDebug.debugSphere(absPose.t, sphereSize);
	}

	for (PxU32 i = 0; i < getNumUsedBones(); i++)
	{
		const PxI32 parent = mBones[i].parentIndex;

		PxMat34Legacy absPose = matrices[i] * mBones[i].bindPose;

		if (skeleton && parent >= 0 && parent < (PxI32)getNumUsedBones())
		{
			PX_ASSERT((PxU32)parent != i);
			PxMat34Legacy absPoseParent = matrices[(physx::PxU32)parent] * mBones[(physx::PxU32)parent].bindPose;
			if ((mBones[(physx::PxU32)parent].numMeshReferenced + mBones[(physx::PxU32)parent].numRigidBodiesReferenced) == 0)
			{
				renderDebug.setCurrentColor(passiveBoneColor);
			}
			else
			{
				renderDebug.setCurrentColor(activeBoneColor);
			}
			renderDebug.debugLine(absPose.t, absPoseParent.t);
		}

		if (boneFramesScale > 0.0f)
		{
			renderDebug.setCurrentColor(colorRed);
			renderDebug.debugLine(absPose.t, absPose.t + absPose.M.getColumn(0) * boneFramesScale);
			renderDebug.setCurrentColor(colorGreen);
			renderDebug.debugLine(absPose.t, absPose.t + absPose.M.getColumn(1) * boneFramesScale);
			renderDebug.setCurrentColor(colorBlue);
			renderDebug.debugLine(absPose.t, absPose.t + absPose.M.getColumn(2) * boneFramesScale);
		}

		if (boneNamesScale > 0.0f)
		{
			renderDebug.setCurrentColor(colorWhite);
			renderDebug.debugText(absPose.t, mBones[i].name.buf);
		}
	}

	renderDebug.popRenderState();
#endif
}



PxU32 ClothingAsset::initializeAssetData(ClothingAssetData& assetData, const PxU32 uvChannel)
{
	//OK. Stage 1 - need to calculate the sizes required...

	const PxU32 numLods = mGraphicalLods.size();
	PxU32 numSubMeshes = 0;

	for (PxU32 a = 0; a < numLods; ++a)
	{
		ClothingGraphicalMeshAssetWrapper meshAsset(reinterpret_cast<NiApexRenderMeshAsset*>(mGraphicalLods[a]->renderMeshAssetPointer));
		numSubMeshes += meshAsset.getSubmeshCount();
	}

	const PxU32 numPhysicalMeshes = mPhysicalMeshes.size();
	PxU32 numPhysicalSubmeshes = 0;
	for (PxU32 a = 0; a < numPhysicalMeshes; ++a)
	{
		ClothingPhysicalMeshParameters* pPhysicalMesh = mPhysicalMeshes[a];
		numPhysicalSubmeshes += pPhysicalMesh->submeshes.arraySizes[0];
	}

	//OK. we now know how many lods and submeshes there are

	const PxU32 requiredSize = ((sizeof(ClothingMeshAssetData) * numLods + sizeof(ClothingAssetSubMesh) * numSubMeshes
	                             + sizeof(ClothingPhysicalMeshData) * numPhysicalMeshes + sizeof(ClothingPhysicalMeshSubmeshData) * numPhysicalSubmeshes) + 15) & 0xfffffff0;

	void* data = PX_ALLOC(requiredSize, PX_DEBUG_EXP("ClothingAssetData"));
	memset(data, 0, requiredSize);

	assetData.mData = (PxU8*)data;
	assetData.mAssetSize = requiredSize;

	//assetData.m_pLods = (ClothingMeshAssetData*)data;

	assetData.mGraphicalLodsCount = numLods;

	assetData.mRootBoneIndex = mParams->rootBoneIndex;

	ClothingAssetSubMesh* pMeshes = (ClothingAssetSubMesh*)(((PxU8*)data) + sizeof(ClothingMeshAssetData) * numLods);

	PxU32 maxVertices = 0;

	for (PxU32 a = 0; a < numLods; ++a)
	{
		ClothingGraphicalMeshAssetWrapper meshAsset(reinterpret_cast<NiApexRenderMeshAsset*>(mGraphicalLods[a]->renderMeshAssetPointer));

		const ClothingGraphicalLodParameters* graphicalLod = mGraphicalLods[a];

		const PxU32 subMeshCount = meshAsset.getSubmeshCount();
		ClothingMeshAssetData* pAsset = assetData.GetLod(a);
		PX_PLACEMENT_NEW(pAsset, ClothingMeshAssetData());
		//pAsset->m_pMeshes = pMeshes;
		pAsset->mSubMeshCount = subMeshCount;

		//Params to set
		pAsset->mImmediateClothMap = graphicalLod->immediateClothMap.buf;
		pAsset->mImmediateClothMapCount = (physx::PxU32)graphicalLod->immediateClothMap.arraySizes[0];


		pAsset->mSkinClothMap = graphicalLod->skinClothMap.buf;
		pAsset->mSkinClothMapCount = (physx::PxU32)graphicalLod->skinClothMap.arraySizes[0];

		pAsset->mSkinClothMapB = graphicalLod->skinClothMapB.buf;
		pAsset->mSkinClothMapBCount = (physx::PxU32)graphicalLod->skinClothMapB.arraySizes[0];

		pAsset->mTetraMap = graphicalLod->tetraMap.buf;
		pAsset->mTetraMapCount = (physx::PxU32)graphicalLod->tetraMap.arraySizes[0];

		pAsset->mPhysicalMeshId = graphicalLod->physicalMeshId;

		pAsset->mSkinClothMapThickness = graphicalLod->skinClothMapThickness;
		pAsset->mSkinClothMapOffset = graphicalLod->skinClothMapOffset;

		pAsset->mBounds = mParams->boundingBox;

		//For now - we'll set this outside again
		pAsset->bActive = true;

		pAsset->mSubmeshOffset = (PxU32)((PxU8*)pMeshes - assetData.mData);

		for (PxU32 b = 0; b < subMeshCount; ++b)
		{
			PX_PLACEMENT_NEW(&pMeshes[b], ClothingAssetSubMesh());

			NxRenderDataFormat::Enum outFormat;
			pMeshes[b].mPositions = (const PxVec3*)meshAsset.getVertexBuffer(b, NxRenderVertexSemantic::POSITION, outFormat);
			pMeshes[b].mPositionOutFormat = outFormat;
			pMeshes[b].mNormals = (const PxVec3*)meshAsset.getVertexBuffer(b, NxRenderVertexSemantic::NORMAL, outFormat);
			pMeshes[b].mNormalOutFormat = outFormat;

			pMeshes[b].mTangents = (const PxVec4*)meshAsset.getVertexBuffer(b, NxRenderVertexSemantic::TANGENT, outFormat);
			pMeshes[b].mTangentOutFormat = outFormat;
			PX_ASSERT(((size_t)pMeshes[b].mTangents & 0xf) == 0);

			pMeshes[b].mBoneWeights = (const PxF32*)meshAsset.getVertexBuffer(b, NxRenderVertexSemantic::BONE_WEIGHT, outFormat);
			pMeshes[b].mBoneWeightOutFormat = outFormat;
			pMeshes[b].mBoneIndices = (const PxU16*)meshAsset.getVertexBuffer(b, NxRenderVertexSemantic::BONE_INDEX, outFormat);
			pMeshes[b].mVertexCount = meshAsset.getNumVertices(b);
			pMeshes[b].mNumBonesPerVertex = meshAsset.getNumBonesPerVertex(b);

			maxVertices = PxMax(maxVertices, meshAsset.getNumVertices(b));

			pMeshes[b].mIndices = (const PxU32*)meshAsset.getIndexBuffer(b);
			pMeshes[b].mIndicesCount = meshAsset.getNumIndices(b);

			const NxVertexUV* PX_RESTRICT uvs = NULL;
			NxRenderDataFormat::Enum uvFormat = NxRenderDataFormat::UNSPECIFIED;
			switch (uvChannel)
			{
			case 0:
				uvs = (const NxVertexUV*)meshAsset.getVertexBuffer(b, NxRenderVertexSemantic::TEXCOORD0, uvFormat);
				break;
			case 1:
				uvs = (const NxVertexUV*)meshAsset.getVertexBuffer(b, NxRenderVertexSemantic::TEXCOORD1, uvFormat);
				break;
			case 2:
				uvs = (const NxVertexUV*)meshAsset.getVertexBuffer(b, NxRenderVertexSemantic::TEXCOORD2, uvFormat);
				break;
			case 3:
				uvs = (const NxVertexUV*)meshAsset.getVertexBuffer(b, NxRenderVertexSemantic::TEXCOORD3, uvFormat);
				break;
			}

			pMeshes[b].mUvs = (NxVertexUVLocal*)uvs;
			pMeshes[b].mUvFormat = uvFormat;
		}
		pMeshes += subMeshCount;
	}


	ClothingPhysicalMeshData* pPhysicalMeshes = (ClothingPhysicalMeshData*)pMeshes;

	assetData.mPhysicalMeshOffset = (PxU32)((PxU8*)pMeshes - assetData.mData);
	ClothingPhysicalMeshSubmeshData* pPhysicalSubMesh = (ClothingPhysicalMeshSubmeshData*)(pPhysicalMeshes + numPhysicalMeshes);
	for (PxU32 a = 0; a < numPhysicalMeshes; ++a)
	{
		ClothingPhysicalMeshData* pPhysicalMesh = &pPhysicalMeshes[a];
		PX_PLACEMENT_NEW(pPhysicalMesh, ClothingPhysicalMeshData);

		ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* physicalMesh = getPhysicalMeshFromLod(a);
		ClothingPhysicalMeshParameters* pPhysicalMeshParams = mPhysicalMeshes[a];

		pPhysicalMesh->mVertices = physicalMesh->vertices.buf;
		pPhysicalMesh->mNormals = physicalMesh->normals.buf;
		pPhysicalMesh->mSkinningNormals = physicalMesh->skinningNormals.buf;
		pPhysicalMesh->mSkinningNormalsCount = (physx::PxU32)physicalMesh->skinningNormals.arraySizes[0];
		pPhysicalMesh->mSubmeshesCount = (physx::PxU32)pPhysicalMeshParams->submeshes.arraySizes[0];
		pPhysicalMesh->mNumBonesPerVertex = physicalMesh->numBonesPerVertex;

		pPhysicalMesh->mBoneIndices = physicalMesh->boneIndices.buf;
		pPhysicalMesh->mBoneWeights = physicalMesh->boneWeights.buf;
		PX_ASSERT(physicalMesh->boneIndices.arraySizes[0] == physicalMesh->boneWeights.arraySizes[0]);
		pPhysicalMesh->mBoneWeightsCount = (physx::PxU32)physicalMesh->boneWeights.arraySizes[0];

		pPhysicalMesh->mOptimizationData = physicalMesh->optimizationData.buf;
		pPhysicalMesh->mOptimizationDataCount = (physx::PxU32)physicalMesh->optimizationData.arraySizes[0];

		pPhysicalMesh->mIndices = physicalMesh->indices.buf;
		pPhysicalMesh->mIndicesCount = (physx::PxU32)physicalMesh->indices.arraySizes[0];

		pPhysicalMesh->mSubmeshOffset = (PxU32)((PxU8*)pPhysicalSubMesh - assetData.mData);

		for (PxI32 b = 0; b < pPhysicalMeshParams->submeshes.arraySizes[0]; ++b)
		{
			//Construct all the submeshes...
			PX_PLACEMENT_NEW(&pPhysicalSubMesh[b], ClothingPhysicalMeshSubmeshData);
			const PhysicalSubmesh* physicalSubMesh = &pPhysicalMeshParams->submeshes.buf[b];
			pPhysicalSubMesh[b].mVertexCount = physicalSubMesh->numVertices;
			pPhysicalSubMesh[b].mIndicesCount = physicalSubMesh->numIndices;
			pPhysicalSubMesh[b].mMaxDistance0VerticesCount = physicalSubMesh->numMaxDistance0Vertices;
		}
		pPhysicalSubMesh += pPhysicalMeshParams->submeshes.arraySizes[0];
	}

	//Initialized compressed num bones per vertex...
	mCompressedNumBonesPerVertexMutex.lock();
	if (mCompressedNumBonesPerVertex.empty())
	{
		initializeCompressedNumBonesPerVertex();
	}
	mCompressedNumBonesPerVertexMutex.unlock();

	assetData.mCompressedNumBonesPerVertexCount = mCompressedNumBonesPerVertex.size();
	assetData.mCompressedNumBonesPerVertex = assetData.mCompressedNumBonesPerVertexCount > 0 ? &(mCompressedNumBonesPerVertex.front()) : NULL;

	assetData.mCompressedTangentWCount = mCompressedTangentW.size();
	assetData.mCompressedTangentW = assetData.mCompressedTangentWCount > 0 ? &(mCompressedTangentW.front()) : NULL;

	assetData.mPhysicalMeshesCount = mPhysicalMeshes.size();
	assetData.mExt2IntMorphMappingCount = mExt2IntMorphMapping.size();
	if (mExt2IntMorphMapping.size())
	{
		assetData.mExt2IntMorphMapping = &(mExt2IntMorphMapping.front());
	}
	else
	{
		assetData.mExt2IntMorphMapping = NULL;
	}

	assetData.mBoneCount = mBones.size();

	return maxVertices;

}



const NxRenderMeshAsset* ClothingAsset::getRenderMeshAsset(PxU32 lodLevel) const
{
	NX_READ_ZONE();

	if (lodLevel < mGraphicalLods.size())
	{
		return reinterpret_cast<NiApexRenderMeshAsset*>(mGraphicalLods[lodLevel]->renderMeshAssetPointer);
	}

	return NULL;
}



PxU32 ClothingAsset::getMeshSkinningMapSize(PxU32 lod)
{
	NX_WRITE_ZONE();
	if (lod >= mGraphicalLods.size())
	{
		APEX_INVALID_PARAMETER("lod %i not a valid lod level. There are only %i graphical lods.", lod, mGraphicalLods.size());
		return 0;
	}
	ClothingGraphicalLodParameters* graphicalLod = mGraphicalLods[lod];

	// make sure everything can be skinned using the skinClothMap,
	// so the user doesn't have to care about immediate skinning as well
	mergeMapping(graphicalLod);

	return (physx::PxU32)mGraphicalLods[lod]->skinClothMap.arraySizes[0];
}



void ClothingAsset::getMeshSkinningMap(PxU32 lod, ClothingMeshSkinningMap* map)
{
	NX_WRITE_ZONE();

	if (lod >= mGraphicalLods.size())
	{
		APEX_INVALID_PARAMETER("lod %i not a valid lod level. There are only %i graphical lods.", lod, mGraphicalLods.size());
		return;
	}
	ClothingGraphicalLodParameters* graphicalLod = mGraphicalLods[lod];

	// make sure everything can be skinned using the skinClothMap,
	// so the user doesn't have to care about immediate skinning as well
	mergeMapping(graphicalLod);

	PX_ASSERT(graphicalLod->skinClothMapOffset > 0.0f); // skinClothMapOffset would only be needed if it's negative, but it's not expected to be

	// copy the values
	PxI32 size = mGraphicalLods[lod]->skinClothMap.arraySizes[0];
	for (PxI32 i = 0; i < size; ++i)
	{
		const SkinClothMap& skinMap	= mGraphicalLods[lod]->skinClothMap.buf[i];
		map[i].positionBary			= skinMap.vertexBary;
		map[i].vertexIndex0			= skinMap.vertexIndex0;
		map[i].normalBary			= skinMap.normalBary;
		map[i].vertexIndex1			= skinMap.vertexIndex1;
		map[i].tangentBary			= skinMap.tangentBary;
		map[i].vertexIndex2			= skinMap.vertexIndex2;
	}
}



bool ClothingAsset::releaseGraphicalData()
{
	NX_WRITE_ZONE();
	bool ok = true;
	for (PxU32 i = 0; i < mGraphicalLods.size(); ++i)
	{
		if (mGraphicalLods[i]->skinClothMapB.arraySizes[0] > 0 || mGraphicalLods[i]->tetraMap.arraySizes[0] > 0)
		{
			APEX_DEBUG_WARNING("Asset contains data that is not supported for external skinning, graphical data cannot be released. Reexport the asset with a newer APEX version.");
			ok = false;
		}

		if (mGraphicalLods[i]->immediateClothMap.arraySizes[0] > 0)
		{
			APEX_DEBUG_WARNING("Asset contains immediate map data, graphical data cannot be released. Call getMeshSkinningMap first.");
			ok = false;
		}
	}

	if (mActors.getSize() > 0)
	{
		APEX_DEBUG_WARNING("Graphical data in asset cannot be released while there are actors of this asset.");
		ok = false;
	}

	if (ok)
	{
		mModule->notifyReleaseGraphicalData(this);

		for (PxU32 i = 0; i < mGraphicalLods.size(); ++i)
		{
			if (mGraphicalLods[i]->renderMeshAssetPointer != NULL)
			{
				NiApexRenderMeshAsset* rma = reinterpret_cast<NiApexRenderMeshAsset*>(mGraphicalLods[i]->renderMeshAssetPointer);
				rma->release();
				mGraphicalLods[i]->renderMeshAssetPointer = NULL;

				mGraphicalLods[i]->renderMeshAsset->destroy();
				mGraphicalLods[i]->renderMeshAsset = NULL;
			}

			NxParamArray<SkinClothMap> skinClothMap(mGraphicalLods[i], "skinClothMap",
				reinterpret_cast<NxParamDynamicArrayStruct*>(&mGraphicalLods[i]->skinClothMap));
			skinClothMap.clear();
		}
	}

	return ok;
}



void ClothingAsset::setupInvBindMatrices()
{
	if (mInvBindPoses.size() == mBones.size())
	{
		return;
	}

	mInvBindPoses.resize(mBones.size());
	for (PxU32 i = 0; i < mBones.size(); i++)
	{
		mBones[i].bindPose.getInverse(mInvBindPoses[i]);
	}
}



void ClothingAsset::prepareCookingJob(CookingAbstract& job, PxF32 scale, PxVec3* gravityDirection, PxVec3* morphedPhysicalMesh)
{
	PxVec3* tempVerticesForScale = NULL;
	PxU32 tempVerticesOffset = 0;
	if (scale != 1.0f || morphedPhysicalMesh != NULL)
	{
		PxU32 numMaxVertices = 0;
		for (PxU32 physicalMeshId = 0; physicalMeshId < mPhysicalMeshes.size(); physicalMeshId++)
		{
			numMaxVertices += mPhysicalMeshes[physicalMeshId]->physicalMesh.numVertices;
		}
		numMaxVertices += mBoneVertices.size();

		tempVerticesForScale = (PxVec3*)PX_ALLOC(sizeof(PxVec3) * numMaxVertices, PX_DEBUG_EXP("tempVerticesForScale"));
		PX_ASSERT(tempVerticesForScale != NULL);

		for (PxU32 physicalMeshId = 0; physicalMeshId < mPhysicalMeshes.size(); physicalMeshId++)
		{
			const PxU32 numVertices = mPhysicalMeshes[physicalMeshId]->physicalMesh.numVertices;
			PxVec3* origVertices = morphedPhysicalMesh != NULL ? morphedPhysicalMesh + tempVerticesOffset : mPhysicalMeshes[physicalMeshId]->physicalMesh.vertices.buf;
			PxVec3* tempVertices = tempVerticesForScale + tempVerticesOffset;
			for (PxU32 i = 0; i < numVertices; i++)
			{
				tempVertices[i] = origVertices[i] * scale;
			}
			tempVerticesOffset += numVertices;
		}
	}

	tempVerticesOffset = 0;

	for (PxU32 physicalMeshId = 0; physicalMeshId < mPhysicalMeshes.size(); physicalMeshId++)
	{
		CookingAbstract::PhysicalMesh physicalMesh;
		physicalMesh.meshID = physicalMeshId;
		if (tempVerticesForScale != NULL)
		{
			physicalMesh.vertices = tempVerticesForScale + tempVerticesOffset;
		}
		else
		{
			physicalMesh.vertices = mPhysicalMeshes[physicalMeshId]->physicalMesh.vertices.buf;
		}
		physicalMesh.numVertices = mPhysicalMeshes[physicalMeshId]->physicalMesh.numVertices;
		physicalMesh.indices = mPhysicalMeshes[physicalMeshId]->physicalMesh.indices.buf;
		physicalMesh.numIndices = mPhysicalMeshes[physicalMeshId]->physicalMesh.numIndices;
		physicalMesh.isTetrahedral = mPhysicalMeshes[physicalMeshId]->physicalMesh.isTetrahedralMesh;

		job.addPhysicalMesh(physicalMesh);

		PxU32 numSubMeshes = (physx::PxU32)mPhysicalMeshes[physicalMeshId]->submeshes.arraySizes[0];
		for (PxU32 submeshID = 0; submeshID < numSubMeshes; submeshID++)
		{
			CookingAbstract::PhysicalSubMesh submesh;
			submesh.meshID = physicalMeshId;
			submesh.submeshID = submeshID;
			submesh.numIndices = mPhysicalMeshes[physicalMeshId]->submeshes.buf[submeshID].numIndices;
			submesh.numVertices = mPhysicalMeshes[physicalMeshId]->submeshes.buf[submeshID].numVertices;
			submesh.numMaxDistance0Vertices = mPhysicalMeshes[physicalMeshId]->submeshes.buf[submeshID].numMaxDistance0Vertices;
			job.addSubMesh(submesh);
		}

		tempVerticesOffset += physicalMesh.numVertices;
	}

	if (tempVerticesForScale == NULL)
	{
		// verify that there are no invalid matrices
		for (PxU32 i = 0; i < mBoneActors.size(); i++)
		{
			if (mBoneActors[i].capsuleRadius > 0.0f || mBoneActors[i].convexVerticesCount == 0)
			{
				continue;
			}

			PxU32 boneIndex = (physx::PxU32)mBoneActors[i].boneIndex;
			const PxF32 det = mBones[boneIndex].bindPose.M.determinant() * mBoneActors[i].localPose.M.determinant();
			if (det < 0.0f)
			{
				// invalid matrices found, need to use temporary buffer only for bone vertices now
				tempVerticesForScale = (PxVec3*)NiGetApexSDK()->getTempMemory(sizeof(PxVec3) * mBoneVertices.size());
				PX_ASSERT(tempVerticesOffset == 0);
				break;
			}
		}
	}

	if (tempVerticesForScale != NULL)
	{
		memset(tempVerticesForScale + tempVerticesOffset, 0xff, sizeof(PxVec3) * mBoneVertices.size());

		PxVec3* boneVertices = morphedPhysicalMesh != NULL ? morphedPhysicalMesh + tempVerticesOffset : mBoneVertices.begin();

		for (PxU32 i = 0; i < mBoneActors.size(); i++)
		{
			PxU32 boneIndex = (physx::PxU32)mBoneActors[i].boneIndex;
			const PxF32 det = mBones[boneIndex].bindPose.M.determinant() * mBoneActors[i].localPose.M.determinant();
			for (PxU32 j = 0; j < mBoneActors[i].convexVerticesCount; j++)
			{
				PxU32 boneVertexIndex = mBoneActors[i].convexVerticesStart + j;
				PxVec3 val = boneVertices[boneVertexIndex] * scale;
				if (det < 0.0f)
				{
					val.z = -val.z;
				}

				tempVerticesForScale[boneVertexIndex + tempVerticesOffset] = val;
			}
		}

		for (PxU32 i = 0; i < mBoneVertices.size(); i++)
		{
			PX_ASSERT(tempVerticesForScale[i + tempVerticesOffset].isFinite());
		}
	}

	PxVec3* boneVertices = tempVerticesForScale != NULL ? tempVerticesForScale + tempVerticesOffset : mBoneVertices.begin();
	job.setConvexBones(mBoneActors.begin(), mBoneActors.size(), mBones.begin(), mBones.size(), boneVertices, 256);

	job.setScale(scale);
	job.setVirtualParticleDensity(mParams->simulation.virtualParticleDensity);

	if (mParams->materialLibrary != NULL)
	{
		ClothingMaterialLibraryParameters* matLib = static_cast<ClothingMaterialLibraryParameters*>(mParams->materialLibrary);
		physx::PxF32 selfcollisionThickness = matLib->materials.buf[mParams->materialIndex].selfcollisionThickness;
		job.setSelfcollisionRadius(selfcollisionThickness);
	}

	PxVec3 gravityDir = mParams->simulation.gravityDirection;
	if (gravityDir.isZero() && gravityDirection != NULL)
	{
		gravityDir = *gravityDirection;
	}
	if (gravityDir.isZero())
	{
		APEX_DEBUG_WARNING("(%s) Gravity direction is zero. Impossible to extract vertical- and zero-stretch fibers.", mName.c_str());
	}
	job.setGravityDirection(gravityDir);

	job.freeTempMemoryWhenDone(tempVerticesForScale);
}



PxU32* ClothingAsset::getMorphMapping(PxU32 graphicalLod, PxU32 submeshIndex)
{
	if (mExt2IntMorphMapping.empty())
	{
		if (!mMorphMappingWarning)
		{
			APEX_INVALID_OPERATION("A clothing actor with morph displacements was specified, but the Asset <%s> was not prepared for morph displacements", mName.c_str());
			mMorphMappingWarning = true;
		}
		return NULL;
	}

	if (graphicalLod == (PxU32) - 1 || graphicalLod > mGraphicalLods.size())
	{
		graphicalLod = mGraphicalLods.size();
	}

	PxU32 offset = 0;
	for (PxU32 i = 0; i < graphicalLod; i++)
	{
		NiApexRenderMeshAsset* rma = getGraphicalMesh(i);
		if (rma == NULL)
			continue;

		for (PxU32 s = 0; s < rma->getSubmeshCount(); s++)
		{
			offset += rma->getSubmesh(s).getVertexCount(0);
		}
	}


	NiApexRenderMeshAsset* rma = getGraphicalMesh(graphicalLod);
	if (rma != NULL)
	{
		PX_ASSERT(submeshIndex < rma->getSubmeshCount());
		submeshIndex = PxMin(submeshIndex, rma->getSubmeshCount());

		for (PxU32 i = 0; i < submeshIndex; ++i)
		{
			offset += rma->getSubmesh(i).getVertexCount(0);
		}
	}

	return mExt2IntMorphMapping.begin() + offset;
}



PxU32 ClothingAsset::getPhysicalMeshOffset(PxU32 physicalMeshId)
{
	physicalMeshId = PxMin(physicalMeshId, mPhysicalMeshes.size());

	PxU32 result = 0;

	for (PxU32 i = 0; i < physicalMeshId; i++)
	{
		result += mPhysicalMeshes[i]->physicalMesh.numVertices;
	}

	return result;
}



class SkinClothMapFacePredicate
{
public:
	bool operator()(const SkinClothMapB& map1, const SkinClothMapB& map2) const
	{
		if (map1.submeshIndex < map2.submeshIndex)
		{
			return true;
		}
		else if (map1.submeshIndex > map2.submeshIndex)
		{
			return false;
		}

		return map1.faceIndex0 < map2.faceIndex0;
	}
};



class SkinClothMapBVertexPredicate
{
public:
	bool operator()(const SkinClothMapB& map1, const SkinClothMapB& map2) const
	{
		return map1.vertexIndexPlusOffset < map2.vertexIndexPlusOffset;
	}
};



void ClothingAsset::getDisplacedPhysicalMeshPositions(PxVec3* morphDisplacements, NxParamArray<PxVec3> displacedMeshPositions)
{
	PxU32 numPhysicalVertices = getPhysicalMeshOffset((PxU32) - 1);
	numPhysicalVertices += mBoneVertices.size();

	if (numPhysicalVertices == 0)
	{
		displacedMeshPositions.clear();
		return;
	}

	displacedMeshPositions.resize(numPhysicalVertices);
	physx::fastMemset(displacedMeshPositions.begin(), 0, sizeof(PxVec3) * numPhysicalVertices);
	PxF32* resultWeights = (PxF32*)PX_ALLOC(sizeof(PxF32) * numPhysicalVertices, PX_DEBUG_EXP("ClothingAsset::getDisplacedPhysicalMeshPositions"));
	physx::fastMemset(resultWeights, 0, sizeof(PxF32) * numPhysicalVertices);

	PxU32 resultOffset = 0;
	for (PxU32 pm = 0; pm < mPhysicalMeshes.size(); pm++)
	{
		PxU32 gm = 0;
		for (; gm < mGraphicalLods.size(); gm++)
		{
			if (mGraphicalLods[gm]->physicalMeshId == pm)
			{
				break;
			}
		}

		ClothingPhysicalMeshParametersNS::PhysicalMesh_Type& physicalMesh = mPhysicalMeshes[pm]->physicalMesh;

		if (gm < mGraphicalLods.size())
		{
			const ClothingGraphicalLodParameters* graphicalLod = mGraphicalLods[gm];

			const NxRenderMeshAsset* rma = getRenderMeshAsset(gm);
			PxU32* morphMapping = getMorphMapping(gm, 0);
			PxU32 morphOffset = 0;

			NxAbstractMeshDescription pMesh;
			pMesh.pIndices = physicalMesh.indices.buf;
			pMesh.numIndices = physicalMesh.numIndices;
			pMesh.pPosition = physicalMesh.vertices.buf;

			if (graphicalLod->immediateClothMap.buf == NULL && graphicalLod->skinClothMapB.buf != NULL)
			{
				// PH: Need to resort the skinMapB buffer to vertex order, will resort back to face order just below
				sort<SkinClothMapB, SkinClothMapBVertexPredicate>(graphicalLod->skinClothMapB.buf, (physx::PxU32)graphicalLod->skinClothMapB.arraySizes[0], SkinClothMapBVertexPredicate());
			}

			for (PxU32 submeshIndex = 0; submeshIndex < rma->getSubmeshCount(); submeshIndex++)
			{
				const NxRenderSubmesh& submesh = rma->getSubmesh(submeshIndex);

				const PxU32 vertexCount = submesh.getVertexCount(0);
				for (PxU32 vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
				{
					const PxVec3 displacement = morphMapping != NULL ? morphDisplacements[morphMapping[morphOffset + vertexIndex]] : PxVec3(0.0f);

					PxU32 indices[4];
					PxF32 weight[4];

					PxU32 numIndices = getCorrespondingPhysicalVertices(*graphicalLod, submeshIndex, vertexIndex, pMesh, morphOffset, indices, weight);

					for (PxU32 i = 0; i < numIndices; i++)
					{
						weight[i] += 0.001f; // none of the weights is 0!
						PX_ASSERT(weight[i] > 0.0f);
						PX_ASSERT(indices[i] < pMesh.numIndices);
						displacedMeshPositions[resultOffset + indices[i]] += (displacement + pMesh.pPosition[indices[i]]) * weight[i];
						resultWeights[resultOffset + indices[i]] += weight[i];
					}
				}

				morphOffset += vertexCount;
			}

			if (graphicalLod->immediateClothMap.buf == NULL && graphicalLod->skinClothMapB.buf != NULL)
			{
				physx::sort<SkinClothMapB, SkinClothMapFacePredicate>(graphicalLod->skinClothMapB.buf, (physx::PxU32)graphicalLod->skinClothMapB.arraySizes[0], SkinClothMapFacePredicate());
			}

			for (PxU32 i = 0; i < physicalMesh.numVertices; i++)
			{
				const PxU32 resultIndex = i + resultOffset;
				if (resultWeights[resultIndex] > 0.0f)
				{
					displacedMeshPositions[resultIndex] /= resultWeights[resultIndex];
				}
				else
				{
					morphOffset = 0;
					const PxVec3 physicsPosition = pMesh.pPosition[i];
					PxF32 shortestDistance = PX_MAX_F32;
					for (PxU32 submeshIndex = 0; submeshIndex < rma->getSubmeshCount() && shortestDistance > 0.0f; submeshIndex++)
					{
						const NxRenderSubmesh& submesh = rma->getSubmesh(submeshIndex);

						const NxVertexFormat& format = submesh.getVertexBuffer().getFormat();
						PxU32 positionIndex = (physx::PxU32)format.getBufferIndexFromID(format.getSemanticID(NxRenderVertexSemantic::POSITION));
						PX_ASSERT(format.getBufferFormat(positionIndex) == NxRenderDataFormat::FLOAT3);
						PxVec3* positions = (PxVec3*)submesh.getVertexBuffer().getBuffer(positionIndex);


						const PxU32 vertexCount = submesh.getVertexCount(0);
						for (PxU32 vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
						{
							PxF32 dist2 = (physicsPosition - positions[vertexIndex]).magnitudeSquared();
							if (dist2 < shortestDistance)
							{
								shortestDistance = dist2;
								displacedMeshPositions[resultIndex] = physicsPosition + morphDisplacements[morphMapping[morphOffset + vertexIndex]];
								if (dist2 == 0.0f)
								{
									break;
								}
							}
						}
						morphOffset += submesh.getVertexCount(0);
					}
				}
			}
		}

		resultOffset += physicalMesh.numVertices;
	}

	PX_FREE(resultWeights);
	resultWeights = NULL;

	for (PxU32 ba = 0; ba < mBoneActors.size(); ba++)
	{

		for (PxU32 bi = 0; bi < mBoneActors[ba].convexVerticesCount; bi++)
		{
			PxU32 boneIndex = bi + mBoneActors[ba].convexVerticesStart;
			PX_ASSERT(mBoneActors[ba].localPose.isIdentity());
			const PxMat34Legacy bindPose = mBones[(physx::PxU32)mBoneActors[ba].boneIndex].bindPose;
			const PxVec3 physicsPosition =  bindPose * mBoneVertices[boneIndex];
			PxVec3 resultPosition(0.0f);

			PxF32 shortestDistance = PX_MAX_F32;
			for (PxU32 gm = 0; gm < mGraphicalLods.size() && shortestDistance > 0.0f; gm++)
			{
				const NxRenderMeshAsset* rma = getRenderMeshAsset(gm);
				PxU32* morphMapping = getMorphMapping(gm, 0);
				PxU32 morphOffset = 0;

				for (PxU32 submeshIndex = 0; submeshIndex < rma->getSubmeshCount() && shortestDistance > 0.0f; submeshIndex++)
				{
					const NxRenderSubmesh& submesh = rma->getSubmesh(submeshIndex);

					const NxVertexFormat& format = submesh.getVertexBuffer().getFormat();
					PxU32 positionIndex = (physx::PxU32)format.getBufferIndexFromID(format.getSemanticID(NxRenderVertexSemantic::POSITION));
					PX_ASSERT(format.getBufferFormat(positionIndex) == NxRenderDataFormat::FLOAT3);
					PxVec3* positions = (PxVec3*)submesh.getVertexBuffer().getBuffer(positionIndex);


					const PxU32 vertexCount = submesh.getVertexCount(0);
					for (PxU32 vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
					{
						PxF32 dist2 = (physicsPosition - positions[vertexIndex]).magnitudeSquared();
						if (dist2 < shortestDistance)
						{
							shortestDistance = dist2;
							resultPosition = physicsPosition + morphDisplacements[morphMapping[morphOffset + vertexIndex]];
							if (dist2 == 0.0f)
							{
								break;
							}
						}
					}
					morphOffset += submesh.getVertexCount(0);
				}
			}

			PxMat34Legacy invBindPose;
			bindPose.getInverse(invBindPose);
			displacedMeshPositions[resultOffset + boneIndex] = invBindPose * resultPosition;

		}
	}
	resultOffset += mBoneVertices.size();


	PX_ASSERT(resultOffset == numPhysicalVertices);
}



void ClothingAsset::initializeCompressedNumBonesPerVertex()
{
	// PH: Merged the tangent w into this code, can be done at the same time
	PxU32 numBonesElementCount = 0;
	PxU32 numTangentElementCount = 0;
	for (PxU32 lodIndex = 0; lodIndex < mGraphicalLods.size(); lodIndex++)
	{
		ClothingGraphicalMeshAssetWrapper meshAsset(reinterpret_cast<NiApexRenderMeshAsset*>(mGraphicalLods[lodIndex]->renderMeshAssetPointer));

		for (PxU32 submeshIdx = 0; submeshIdx < meshAsset.getSubmeshCount(); submeshIdx++)
		{
			const PxU32 numVertices = meshAsset.getNumVertices(submeshIdx);

			// 3 bits per entry, 10 entries per U32
			PxU32 numBoneEntries = (numVertices + 15) / 16;

			NxRenderDataFormat::Enum outFormat = NxRenderDataFormat::UNSPECIFIED;
			const PxVec4* PX_RESTRICT tangents = (const PxVec4*)meshAsset.getVertexBuffer(submeshIdx, NxRenderVertexSemantic::TANGENT, outFormat);
			PX_ASSERT(tangents == NULL || outFormat == NxRenderDataFormat::FLOAT4);

			PxU32 numTangentEntries = tangents != NULL ? (numVertices + 31) / 32 : 0;

			// Round up such that map for all submeshes is 16 byte aligned
			while ((numBoneEntries & 0x3) != 0) // this is a numEntries % 4
			{
				numBoneEntries++;
			}

			while ((numTangentEntries & 0x3) != 0)
			{
				numTangentEntries++;
			}

			numBonesElementCount += numBoneEntries;
			numTangentElementCount += numTangentEntries;
		}
	}

	if (numBonesElementCount > 0)
	{
		mCompressedNumBonesPerVertex.resize(numBonesElementCount, 0);

		PxU32 numNonNormalizedVertices = 0;
		PxU32 numInefficientVertices = 0;

		PxU32* bonesPerVertex = mCompressedNumBonesPerVertex.begin();
		for (PxU32 lodIndex = 0; lodIndex < mGraphicalLods.size(); lodIndex++)
		{
			ClothingGraphicalMeshAssetWrapper meshAsset(reinterpret_cast<NiApexRenderMeshAsset*>(mGraphicalLods[lodIndex]->renderMeshAssetPointer));

			for (PxU32 submeshIdx = 0; submeshIdx < meshAsset.getSubmeshCount(); submeshIdx++)
			{
				PxU32 numVerticesWritten = 0;

				NxRenderDataFormat::Enum outFormat = NxRenderDataFormat::UNSPECIFIED;
				const PxF32* PX_RESTRICT boneWeights = (const PxF32*)meshAsset.getVertexBuffer(submeshIdx, NxRenderVertexSemantic::BONE_WEIGHT, outFormat);

				const PxU32 numVertices = meshAsset.getNumVertices(submeshIdx);
				const PxU32 numBonesPerVertex = meshAsset.getNumBonesPerVertex(submeshIdx);
				PX_ASSERT((numBonesPerVertex > 0) == (boneWeights != NULL));

				PX_ASSERT(((size_t)bonesPerVertex & 0xf) == 0); // make sure we start 16 byte aligned
				for (PxU32 vertexIndex = 0; vertexIndex < numVertices; vertexIndex++)
				{
					PxU32 firstZeroBoneAfter = 0;
					if (boneWeights != NULL)
					{
						const PxF32* PX_RESTRICT vertexBoneWeights = boneWeights + (vertexIndex * numBonesPerVertex);
						PxU32 firstZeroBone = numBonesPerVertex;
						PxF32 sumWeights = 0.0f;
						for (PxU32 k = 0; k < numBonesPerVertex; k++)
						{
							sumWeights += vertexBoneWeights[k];
							if (vertexBoneWeights[k] == 0.0f)
							{
								firstZeroBone = PxMin(firstZeroBone, k);
							}
							else
							{
								firstZeroBoneAfter = k + 1;
							}
						}
						PX_ASSERT(firstZeroBoneAfter <= numBonesPerVertex);

						numNonNormalizedVertices += PxAbs(sumWeights - 1.0f) > 0.001f ? 1 : 0;
						numInefficientVertices += firstZeroBone < firstZeroBoneAfter ? 1 : 0;
					}
					else
					{
						firstZeroBoneAfter = 1;
					}

					// write the value
					if (numVerticesWritten == 16)
					{
						bonesPerVertex++;
						numVerticesWritten = 0;
					}

					PX_ASSERT(firstZeroBoneAfter > 0);
					PX_ASSERT(firstZeroBoneAfter < 5); // or else it doesn't fit
					(*bonesPerVertex) |= ((firstZeroBoneAfter - 1) & 0x3) << (numVerticesWritten * 2);
					numVerticesWritten++;
				}

				// if *bonesPerVertex contains data, advance
				if (numVerticesWritten > 0)
				{
					bonesPerVertex++;
				}

				// advance until 16 byte aligned
				while (((size_t)bonesPerVertex & 0xf) != 0)
				{
					bonesPerVertex++;
				}
			}
		}

		if (numNonNormalizedVertices > 0)
		{
			APEX_DEBUG_WARNING("The Clothing Asset <%s> has %d vertices with non-normalized bone weights. This may lead to wrongly displayed meshes.", mName.c_str(), numNonNormalizedVertices);
		}

		if (numInefficientVertices > 0)
		{
			APEX_DEBUG_WARNING("The Clothing Asset <%s> has %d vertices with non-sorted bone weights. This can decrease performance of the skinning. Resave the asset!", mName.c_str(), numInefficientVertices);
		}
	}

	if (numTangentElementCount > 0)
	{
		mCompressedTangentW.resize(numTangentElementCount, 0);

		PxU32* tangentW = mCompressedTangentW.begin();
		for (PxU32 lodIndex = 0; lodIndex < mGraphicalLods.size(); lodIndex++)
		{
			ClothingGraphicalMeshAssetWrapper meshAsset(reinterpret_cast<NiApexRenderMeshAsset*>(mGraphicalLods[lodIndex]->renderMeshAssetPointer));

			for (PxU32 submeshIdx = 0; submeshIdx < meshAsset.getSubmeshCount(); submeshIdx++)
			{
				PxU32 numVerticesWritten = 0;

				NxRenderDataFormat::Enum outFormat = NxRenderDataFormat::UNSPECIFIED;
				const PxVec4* PX_RESTRICT tangents = (const PxVec4*)meshAsset.getVertexBuffer(submeshIdx, NxRenderVertexSemantic::TANGENT, outFormat);
				PX_ASSERT(outFormat == NxRenderDataFormat::FLOAT4);

				PX_ASSERT(((size_t)tangentW & 0xf) == 0); // make sure we start 16 byte aligned

				const PxU32 numVertices = meshAsset.getNumVertices(submeshIdx);
				for (PxU32 vertexIndex = 0; vertexIndex < numVertices; vertexIndex++)
				{
					PX_ASSERT(PxAbs(tangents[vertexIndex].w) == 1.0f);
					const PxU32 tangentWpositive = tangents[vertexIndex].w > 0.0f ? 1u : 0u;

					// write the value
					if (numVerticesWritten == 32)
					{
						tangentW++;
						numVerticesWritten = 0;
					}

					(*tangentW) |= tangentWpositive << numVerticesWritten;
					numVerticesWritten++;
				}

				// if *bonesPerVertex contains data, advance
				if (numVerticesWritten > 0)
				{
					tangentW++;
				}

				// advance until 16 byte aligned
				while (((size_t)tangentW & 0xf) != 0)
				{
					tangentW++;
				}
			}
		}
	}
}



PxU32 ClothingAsset::getRootBoneIndex()
{
	PX_ASSERT(mParams->rootBoneIndex < getNumUsedBones());
	return mParams->rootBoneIndex;
}



PxU32 ClothingAsset::getInterCollisionChannels()
{
	return mParams->interCollisionChannels;
}



void ClothingAsset::releaseCookedInstances()
{
	if (mParams != NULL && mActors.getSize() == 0)
	{
		for (PxI32 i = 0; i < mParams->cookedData.arraySizes[0]; i++)
		{
			NxParameterized::Interface* cookedData = mParams->cookedData.buf[i].cookedData;
			if (cookedData != NULL)
			{
				BackendFactory* factory = mModule->getBackendFactory(cookedData->className());
				PX_ASSERT(factory != NULL);
				if (factory != NULL)
				{
					bool isTetra = false;
					if (!mPhysicalMeshes.isEmpty())
					{
						isTetra = mPhysicalMeshes[0]->physicalMesh.isTetrahedralMesh;
					}

					factory->releaseCookedInstances(mParams->cookedData.buf[i].cookedData, isTetra);
				}
			}
		}
	}
}



void ClothingAsset::destroy()
{
	mActors.clear();

	while (numCookingDependencies() > 0)
	{
		physx::shdfnd::Thread::sleep(0);
	}

	mModule->unregisterAssetWithScenes(this);

	mUnusedSimulationMutex.lock();
	for (PxU32 i = 0; i < mUnusedSimulation.size(); i++)
	{
		if (mUnusedSimulation[i] == NULL)
		{
			continue;
		}

		destroySimulation(mUnusedSimulation[i]);
		mUnusedSimulation[i] = NULL;
	}
	mUnusedSimulation.clear();
	mUnusedSimulationMutex.unlock();

	releaseCookedInstances();

	for (PxU32 i = 0; i < mPhysicalMeshes.size(); i++)
	{
		if (mParams != NULL)
		{
			// PH: we should only decrement if we don't use releaseAndReturnNxParameterizedInterface
			mPhysicalMeshes[i]->referenceCount--;
		}
	}

	for (PxU32 i = 0; i < mGraphicalLods.size(); i++)
	{
		if (mGraphicalLods[i]->renderMeshAssetPointer == NULL)
			continue;

		reinterpret_cast<NiApexRenderMeshAsset*>(mGraphicalLods[i]->renderMeshAssetPointer)->release();
		mGraphicalLods[i]->renderMeshAssetPointer = NULL;
	}

#ifndef WITHOUT_PVD
	destroyPvdInstances();
#endif
	if (mParams != NULL)
	{
		// safety!
		for (PxU32 i = 0; i < mPhysicalMeshes.size(); i++)
		{
			PX_ASSERT(mPhysicalMeshes[i]->referenceCount == 0);
		}

		mParams->setSerializationCallback(NULL, NULL);
		mParams->destroy();
		mParams = NULL;
	}

	mCompressedNumBonesPerVertex.reset();

	delete this;
}



PxI32 ClothingAsset::getBoneInternalIndex(const char* boneName) const
{
	if (boneName == NULL)
	{
		return -1;
	}

	PxI32 internalBoneIndex = -1;
	for (PxU32 i = 0; i < mBones.size(); i++)
	{
		if (mBones[i].name != NULL && (strcmp(mBones[i].name, boneName) == 0))
		{
			internalBoneIndex  = (physx::PxI32)i;
			break;
		}
	}

	return internalBoneIndex ;
}



PxI32 ClothingAsset::getBoneInternalIndex(PxU32 boneIndex) const
{
	if (boneIndex >= mBones.size())
	{
		return -1;
	}

	PxI32 internalBoneIndex  = -1;
	for (PxU32 i = 0; i < mBones.size(); i++)
	{
		if (mBones[i].externalIndex == (PxI32)boneIndex)
		{
			internalBoneIndex  = (physx::PxI32)i;
			break;
		}
	}

	return internalBoneIndex;
}



class SortGraphicalVerts
{
public:
	SortGraphicalVerts(PxU32 numVerts, PxU32 submeshOffset, const PxU32* indices, PxU32 numIndices, ClothingGraphicalLodParameters* lodParameters,
						ClothingPhysicalMeshParameters* physicsMesh, NiApexRenderMeshAsset* renderMeshAsset = NULL) : mNumNotFoundVertices(0), mIndices(indices), mNumIndices(numIndices)
	{
		mNew2Old.resize(numVerts, 0);
		mOld2New.resize(numVerts, 0);
		mVertexInfo.resize(numVerts);
		for (PxU32 i = 0; i < numVerts; i++)
		{
			mNew2Old[i] = i;
			mOld2New[i] = i;
			mVertexInfo[i].idealPosition = (PxF32)i;
		}

		// scale the max distance such that it gives a guess for the 'idealPosition'
		// const PxF32 maxDistanceScale = (PxF32)numVerts / physicsMesh->physicalMesh.maximumMaxDistance;
		// ClothingPhysicalMeshParametersNS::ConstrainCoefficient_Type* coeffs = physicsMesh->physicalMesh.constrainCoefficients.buf;

		if (lodParameters->immediateClothMap.arraySizes[0] > 0)
		{
			for (PxU32 i = 0; i < numVerts; i++)
			{
				const PxU32 immediateMap = lodParameters->immediateClothMap.buf[i + submeshOffset];
				if (immediateMap != ClothingConstants::ImmediateClothingInvalidValue)
				{
					if ((immediateMap & ClothingConstants::ImmediateClothingInSkinFlag) == 0)
					{
						const PxU32 targetIndex = immediateMap & ClothingConstants::ImmediateClothingReadMask;
						mVertexInfo[i].physicsSubmeshNumber = getSubmeshIndex(targetIndex, physicsMesh);
						//mVertexInfo[i].idealPosition = coeffs[targetIndex].maxDistance * maxDistanceScale;
					}
				}
			}
		}

		const PxU32 numSkins = (physx::PxU32)lodParameters->skinClothMap.arraySizes[0];
		for (PxU32 i = 0; i < numSkins; i++)
		{
			// const PxU32 faceIndex0 = lodParameters->skinClothMap.buf[i].faceIndex0;
			const PxU32 vertexIndex = lodParameters->skinClothMap.buf[i].vertexIndexPlusOffset - submeshOffset;
			if (vertexIndex < numVerts) // this also handles underflow
			{
				PxU32 physVertexIndex = PxMax(lodParameters->skinClothMap.buf[i].vertexIndex0, lodParameters->skinClothMap.buf[i].vertexIndex1);
				physVertexIndex = PxMax(physVertexIndex, lodParameters->skinClothMap.buf[i].vertexIndex2);
				const PxI32 submeshNumber = getSubmeshIndexMaxFromVert(physVertexIndex, physicsMesh);

				mVertexInfo[vertexIndex].physicsSubmeshNumber = PxMax(submeshNumber, mVertexInfo[vertexIndex].physicsSubmeshNumber);
			}
		}

		const PxU32 numSkinB = (physx::PxU32)lodParameters->skinClothMapB.arraySizes[0];
		for (PxU32 i = 0; i < numSkinB; i++)
		{
			const PxU32 faceIndex0 = lodParameters->skinClothMapB.buf[i].faceIndex0;
			const PxU32 vertexIndex = lodParameters->skinClothMapB.buf[i].vertexIndexPlusOffset - submeshOffset;
			if (vertexIndex < numVerts)
			{
				mVertexInfo[vertexIndex].physicsSubmeshNumber = PxMax(getSubmeshIndexMaxFromFace(faceIndex0, physicsMesh), mVertexInfo[vertexIndex].physicsSubmeshNumber);
			}
		}

		const PxU32 numTetras = (physx::PxU32)lodParameters->tetraMap.arraySizes[0];
		for (PxU32 i = 0; i < numTetras; i++)
		{
			const PxU32 tetraIndex0 = lodParameters->tetraMap.buf[i].tetraIndex0;
			mVertexInfo[i].physicsSubmeshNumber = PxMax(getSubmeshIndexMaxFromFace(tetraIndex0, physicsMesh), mVertexInfo[i].physicsSubmeshNumber);
		}

		if (renderMeshAsset != NULL)
		{
			PxU32 numSkinClothMaps		= (physx::PxU32)lodParameters->skinClothMap.arraySizes[0];
			SkinClothMap* skinClothMaps	= lodParameters->skinClothMap.buf;

			PxU32 count = 0;
			PxU32 targetOffset = 0;
			for (PxU32 s = 0; s < renderMeshAsset->getSubmeshCount(); s++)
			{
				const NxVertexBuffer& vb = renderMeshAsset->getSubmesh(s).getVertexBuffer();
				const NxVertexFormat& vf = vb.getFormat();

				const PxU32 graphicalMaxDistanceIndex	= (physx::PxU32)vf.getBufferIndexFromID(vf.getID("MAX_DISTANCE"));
				NxRenderDataFormat::Enum outFormat		= vf.getBufferFormat(graphicalMaxDistanceIndex);
				const PxF32* graphicalMaxDistance		= outFormat == NxRenderDataFormat::UNSPECIFIED ? NULL :
					reinterpret_cast<const PxF32*>(vb.getBuffer(graphicalMaxDistanceIndex));
				PX_ASSERT(graphicalMaxDistance == NULL || outFormat == NxRenderDataFormat::FLOAT1);

				const PxU32 numVertices = renderMeshAsset->getSubmesh(s).getVertexCount(0);
				for (PxU32 vertexIndex = 0; vertexIndex < numVertices; vertexIndex++)
				{
					const PxU32 index = vertexIndex + targetOffset;
					if (graphicalMaxDistance != NULL && graphicalMaxDistance[vertexIndex] == 0.0f)
					{
						for (PxU32 i = 0; i < numSkinClothMaps; i++)
						{
							PxU32 grIndex = skinClothMaps[i].vertexIndexPlusOffset;
							if (grIndex == index)
							{	
								count++;

								const PxU32 vertexIndex = grIndex - submeshOffset;
								if (vertexIndex < numVerts) // this also handles underflow
								{
					//				PxU32 physVertexIndex = PxMax(lodParameters->skinClothMap.buf[i].vertexIndex0, lodParameters->skinClothMap.buf[i].vertexIndex1);
					//				physVertexIndex = PxMax(physVertexIndex, lodParameters->skinClothMap.buf[i].vertexIndex2);
					//				const PxI32 submeshNumber = getSubmeshIndexMaxFromVert(physVertexIndex, physicsMesh);

									mVertexInfo[vertexIndex].physicsSubmeshNumber = -1;
								}
							}
						}
					}
				}

				targetOffset += numVertices;
			}
		}

		for (PxU32 i = 0; i < numVerts; i++)
		{
			if (mVertexInfo[i].physicsSubmeshNumber == -1)
			{
				// give it the largest submesh number, such that it gets sorted to the very end instead of the very beginning. Then at least it's cpu skinned.
				mVertexInfo[i].physicsSubmeshNumber = PX_MAX_I32;
				mNumNotFoundVertices++;
			}
			PX_ASSERT(mVertexInfo[i].idealPosition != -1);
		}

		// we only know the submesh number for each individual vertex, but we need to make sure that this is consistent for each
		// triangle. So we define the submesh number for a triangle as the min of all the submesh numbers of its vertices.
		// Then we set the vertex submesh number to the min of all its triangle's submesh numbers.

		shdfnd::Array<PxI32> triangleSubmeshIndex(mNumIndices / 3, 0x7fffffff);
		for (PxU32 i = 0; i < triangleSubmeshIndex.size(); i++)
		{
			PxU32 index = i * 3;
			PxI32 submeshNumber = PxMin(mVertexInfo[mIndices[index]].physicsSubmeshNumber, mVertexInfo[mIndices[index + 1]].physicsSubmeshNumber);
			triangleSubmeshIndex[i] = PxMin(submeshNumber, mVertexInfo[mIndices[index + 2]].physicsSubmeshNumber);
		}

		// now let's redistribute it
		for (PxU32 i = 0; i < triangleSubmeshIndex.size(); i++)
		{
			for (PxU32 j = 0; j < 3; j ++)
			{
				PxU32 index = mIndices[i * 3 + j];
				if (triangleSubmeshIndex[i] < mVertexInfo[index].physicsSubmeshNumber)
				{
					mVertexInfo[index].physicsSubmeshNumber = triangleSubmeshIndex[i];
					// we need to distinguish the ones that naturally belong to a submesh, or the border ones that got added this late
					mVertexInfo[index].pulledInThroughTriangle = true;
				}
			}
		}
	}

	PxI32 getSubmeshIndex(PxU32 vertexIndex, const ClothingPhysicalMeshParameters* physicsMesh)
	{
		for (PxI32 i = 0; i < physicsMesh->submeshes.arraySizes[0]; i++)
		{
			if (physicsMesh->submeshes.buf[i].numVertices > vertexIndex)
			{
				return i;
			}
		}

		return physicsMesh->submeshes.arraySizes[0];
	}

	PxI32 getSubmeshIndexMaxFromFace(PxU32 faceIndex0, const ClothingPhysicalMeshParameters* physicsMesh)
	{
		for (PxI32 i = 0; i < physicsMesh->submeshes.arraySizes[0]; i++)
		{
			if (physicsMesh->submeshes.buf[i].numIndices > faceIndex0)
			{
				return i;
			}
		}

		return physicsMesh->submeshes.arraySizes[0];
	}

	PxI32 getSubmeshIndexMaxFromVert(PxU32 vertexIndex, const ClothingPhysicalMeshParameters* physicsMesh)
	{
		for (PxI32 i = 0; i < physicsMesh->submeshes.arraySizes[0]; i++)
		{
			if (physicsMesh->submeshes.buf[i].numVertices > vertexIndex)
			{
				return i;
			}
		}

		return physicsMesh->submeshes.arraySizes[0];
	}

	bool operator()(const PxU32 a, const PxU32 b) const
	{
		if (mVertexInfo[a].physicsSubmeshNumber != mVertexInfo[b].physicsSubmeshNumber)
		{
			return mVertexInfo[a].physicsSubmeshNumber < mVertexInfo[b].physicsSubmeshNumber;
		}
		else
		{
			if (mVertexInfo[a].pulledInThroughTriangle != mVertexInfo[b].pulledInThroughTriangle)
			{
				return mVertexInfo[a].pulledInThroughTriangle < mVertexInfo[b].pulledInThroughTriangle;
			}

			return mVertexInfo[a].idealPosition < mVertexInfo[b].idealPosition;
		}
	}

	void sortVertices()
	{
		shdfnd::sort(mNew2Old.begin(), mNew2Old.size(), *this);

		for (PxU32 i = 0; i < mNew2Old.size(); i++)
		{
			mOld2New[mNew2Old[i]] = i;
		}
	}

#if 0
	// experimental stuff, disabled for now
	PxI32 median(PxI32 a, PxI32 b, PxI32 c)
	{
#if 1
		return PxMax(PxMin(a, b), PxMin(PxMax(a, b), c));
#else
		if (a < b)
		{
			if (a < c)
			{
				return PxMin(b, c);
			}
			return a;
		}
		else
		{
			// a >= b
			if (a > c)
			{
				return PxMax(b, c);
			}
			return a;
		}
#endif
	}

	void refineIdealPositions(PxF32 weight)
	{
		for (PxU32 i = 0; i < mNumIndices; i += 3)
		{
			PxI32 verts[3] =
			{
				mOld2New[mIndices[i + 0]],
				mOld2New[mIndices[i + 1]],
				mOld2New[mIndices[i + 2]],
			};

			//const PxI32 avg = (verts[0] + verts[1] + verts[2]) / 3;
			const PxI32 avg = median(verts[0], verts[1], verts[2]);

			for (PxU32 j = 0; j < 3; j++)
			{
				const PxF32 pos = mVertexInfo[mIndices[i + j]].idealPosition;
				const PxF32 idealPos = (1.0f - weight) * pos + weight * avg;
				mVertexInfo[mIndices[i + j]].idealChange += idealPos - pos;
				mVertexInfo[mIndices[i + j]].idealCount += 1.0f;
			}
		}

		for (PxU32 i = 0; i < mVertexInfo.size(); i++)
		{
			const PxF32 change = mVertexInfo[i].idealChange / mVertexInfo[i].idealCount;
			mVertexInfo[i].idealPosition += change;
			mVertexInfo[i].idealChange = 0.0f;
			mVertexInfo[i].idealCount = 0.0f;
		}
	}
#endif

	PxU32 computeCost()
	{
		PxU32 totalDist = 0;
		//const PxU32 numVertsInCacheLine = 4096 / 12;
		for (PxU32 i = 0; i < mNumIndices; i += 3)
		{
			// create 3 edges
			const PxU32 edges[3] =
			{
				(PxU32)PxAbs((PxI32)mOld2New[mIndices[i + 0]] - (PxI32)mOld2New[mIndices[i + 1]]),
				(PxU32)PxAbs((PxI32)mOld2New[mIndices[i + 1]] - (PxI32)mOld2New[mIndices[i + 2]]),
				(PxU32)PxAbs((PxI32)mOld2New[mIndices[i + 2]] - (PxI32)mOld2New[mIndices[i + 0]]),
			};

			for (PxU32 j = 0; j < 3; j++)
			{
#if 0
				if (edges[j] > numVertsInCacheLine)
				{
					// every pair that is not in the same cache line is counted as a (potential) miss
					totalDist++;
				}
#endif

#if 1
				totalDist += edges[j];
#endif
			}

		}

		return totalDist;
	}

	PxI32 getSubmesh(PxU32 newVertexIndex)
	{
		return mVertexInfo[mNew2Old[newVertexIndex]].physicsSubmeshNumber;
	}

	bool isAdditional(PxU32 newVertexIndex)
	{
		return mVertexInfo[mNew2Old[newVertexIndex]].pulledInThroughTriangle;
	}

	shdfnd::Array<PxU32> mOld2New;
	shdfnd::Array<PxU32> mNew2Old;

	PxU32 mNumNotFoundVertices;

private:
	SortGraphicalVerts& operator=(const SortGraphicalVerts&);

	struct InternalInfo
	{
		InternalInfo() : physicsSubmeshNumber(-1), idealPosition(-1), idealChange(0.0f), idealCount(0.0f), pulledInThroughTriangle(false) {}
		PxI32 physicsSubmeshNumber;
		PxF32 idealPosition;
		PxF32 idealChange;
		PxF32 idealCount;
		bool pulledInThroughTriangle;
	};

	shdfnd::Array<InternalInfo> mVertexInfo;

	const PxU32* mIndices;
	const PxU32 mNumIndices;
};



template <typename T>
class SkinClothMapPredicate
{
public:
	bool operator()(const T& map1, const T& map2) const
	{
		return map1.vertexIndexPlusOffset < map2.vertexIndexPlusOffset;
	}
};



class SortGraphicalIndices
{
public:
	SortGraphicalIndices(PxU32 numIndices, PxU32* indices) : mIndices(indices)
	{
		mTriangleInfo.resize(numIndices / 3);
		for (PxU32 i = 0; i < mTriangleInfo.size(); i++)
		{
			mTriangleInfo[i].submesh = -1;
			mTriangleInfo[i].originalPosition = i;
		}

		mNew2Old.resize(numIndices / 3);
		mOld2New.resize(numIndices / 3);
		for (PxU32 i = 0; i < mNew2Old.size(); i++)
		{
			mNew2Old[i] = i;
			mOld2New[i] = i;
		}
	}

	void setTriangleSubmesh(PxU32 triangle, PxI32 submesh)
	{
		mTriangleInfo[triangle].submesh = submesh;
	}

	PxI32 getTriangleSubmesh(PxU32 newTriangleIndex) const
	{
		return mTriangleInfo[mNew2Old[newTriangleIndex]].submesh;
	}

	bool operator()(const PxU32 t1, const PxU32 t2) const
	{
		if (mTriangleInfo[t1].submesh != mTriangleInfo[t2].submesh)
		{
			return mTriangleInfo[t1].submesh < mTriangleInfo[t2].submesh;
		}

		return mTriangleInfo[t1].originalPosition < mTriangleInfo[t2].originalPosition;
	}

	void sort()
	{
		shdfnd::sort(mNew2Old.begin(), mNew2Old.size(), *this);

		for (PxU32 i = 0; i < mNew2Old.size(); i++)
		{
			mOld2New[mNew2Old[i]] = i;
		}

		ApexPermute(mIndices, mNew2Old.begin(), mNew2Old.size(), 3);
	}

private:
	shdfnd::Array<PxU32> mNew2Old;
	shdfnd::Array<PxU32> mOld2New;
	PxU32* mIndices;

	struct TriangleInfo
	{
		PxI32 submesh;
		PxU32 originalPosition;
	};
	shdfnd::Array<TriangleInfo> mTriangleInfo;
};



bool ClothingAsset::reorderGraphicsVertices(PxU32 graphicalLodId, bool perfWarning)
{
	PX_ASSERT(mGraphicalLods[graphicalLodId] != NULL);

	const PxU32 curSortingVersion = 2; // bump this number when the sorting changes!

	if (mGraphicalLods[graphicalLodId]->renderMeshAssetSorting >= curSortingVersion)
	{
		// nothing needs to be done.
		return false;
	}

	mGraphicalLods[graphicalLodId]->renderMeshAssetSorting = curSortingVersion;

	if (perfWarning)
	{
		APEX_DEBUG_INFO("Performance warning. This asset <%s> has to be re-saved to speed up loading", mName.c_str());
	}

	NiApexRenderMeshAsset* rma = static_cast<NiApexRenderMeshAsset*>(mGraphicalLods[graphicalLodId]->renderMeshAssetPointer);
	PX_ASSERT(rma != NULL);
	if (rma == NULL)
		return false;

	const PxU32 numSubMeshes = rma->getSubmeshCount();
	PxU32 submeshVertexOffset = 0;

	const PxU32 numSkinClothMap = (physx::PxU32)mGraphicalLods[graphicalLodId]->skinClothMap.arraySizes[0];
	ClothingGraphicalLodParametersNS::SkinClothMapD_Type* skinClothMap = mGraphicalLods[graphicalLodId]->skinClothMap.buf;

	const PxU32 numSkinClothMapB = (physx::PxU32)mGraphicalLods[graphicalLodId]->skinClothMapB.arraySizes[0];
	ClothingGraphicalLodParametersNS::SkinClothMapB_Type* skinClothMapB = mGraphicalLods[graphicalLodId]->skinClothMapB.buf;

	// allocate enough space
	{
		ClothingPhysicalMeshParameters* physicsMesh = mPhysicalMeshes[mGraphicalLods[graphicalLodId]->physicalMeshId];
		PxI32 count = physicsMesh->submeshes.arraySizes[0] * (physx::PxI32)numSubMeshes;

		if (mGraphicalLods[graphicalLodId]->physicsSubmeshPartitioning.arraySizes[0] != count)
		{
			NxParameterized::Handle handle(*mGraphicalLods[graphicalLodId], "physicsSubmeshPartitioning");
			PX_ASSERT(handle.isValid());
			PX_ASSERT(handle.parameterDefinition()->type() == NxParameterized::TYPE_ARRAY);
			handle.resizeArray(count);
		}
	}

	PxU32 submeshPartitioningIndex = 0;
	for (PxU32 s = 0; s < numSubMeshes; s++)
	{
		const NxRenderSubmesh& submesh = rma->getSubmesh(s);
		const PxU32 numVertices = submesh.getVertexCount(0);

		const PxU32 numIters = 2;
		PxU32 costs[numIters] = { 0 };

		ClothingPhysicalMeshParameters* physicsMesh = mPhysicalMeshes[mGraphicalLods[graphicalLodId]->physicalMeshId];

		SortGraphicalVerts sortedVertices(numVertices, submeshVertexOffset, submesh.getIndexBuffer(0), submesh.getIndexCount(0), mGraphicalLods[graphicalLodId], physicsMesh, rma);

		costs[0] = sortedVertices.computeCost();
		sortedVertices.sortVertices(); // same ordering as before, but grouped by submeshes now

		costs[1] = costs[0]; // stupid warnings
		costs[1] = sortedVertices.computeCost();


#if 0
		// reorder based on triangle distances (into the vertex buffer)
		// disable for now...
		// PH: On the PS3 perf is 30% better if this is not performed! Needs much more further investigation!
		PxU32 numIncreases = 0;
		for (PxU32 i = 2; i < numIters; i++)
		{
			sortedVertices.refineIdealPositions(1.0f);
			sortedVertices.sortVertices();
			costs[i] = sortedVertices.computeCost();
			if (costs[i] >= costs[i - 1])
			{
				numIncreases++;
				if (numIncreases > 40)
				{
					break;
				}
			}
			else if (numIncreases > 0)
			{
				numIncreases--;
			}
		}
#endif

		rma->getNiSubmesh(s).applyPermutation(sortedVertices.mOld2New, sortedVertices.mNew2Old);

		{
			PxU32 vertexCount = 0;
			PxU32 vertexAdditionalCount = 0;
			for (PxI32 i = 0; i < physicsMesh->submeshes.arraySizes[0]; i++)
			{
				while (vertexAdditionalCount < sortedVertices.mNew2Old.size() && sortedVertices.getSubmesh(vertexAdditionalCount) <= i)
				{
					if (!sortedVertices.isAdditional(vertexAdditionalCount))
					{
						vertexCount = vertexAdditionalCount + 1;
					}
					vertexAdditionalCount++;
				}

				mGraphicalLods[graphicalLodId]->physicsSubmeshPartitioning.buf[submeshPartitioningIndex + i].graphicalSubmesh = s;
				mGraphicalLods[graphicalLodId]->physicsSubmeshPartitioning.buf[submeshPartitioningIndex + i].physicalSubmesh = (physx::PxU32)i;
				mGraphicalLods[graphicalLodId]->physicsSubmeshPartitioning.buf[submeshPartitioningIndex + i].numSimulatedVertices = vertexCount;
				mGraphicalLods[graphicalLodId]->physicsSubmeshPartitioning.buf[submeshPartitioningIndex + i].numSimulatedVerticesAdditional = vertexAdditionalCount;
			}
		}


		// also sort the index buffer accordingly
		{
			PxU32* indices = rma->getNiSubmesh(s).getIndexBufferWritable(0);

			PxU32 indexCount = submesh.getIndexCount(0);
			PxU32 triCount = indexCount / 3;
			SortGraphicalIndices sortedIndices(indexCount, indices);

			for (PxU32 i = 0; i < triCount; i++)
			{
				PxI32 submeshIndex = 0;
				for (PxU32 j = 0; j < 3; j++)
				{
					submeshIndex = PxMax(submeshIndex, sortedVertices.getSubmesh(indices[i * 3 + j]));
				}
				sortedIndices.setTriangleSubmesh(i, submeshIndex);
			}

			sortedIndices.sort();

			PxU32 startTriangle = 0;
			for (PxI32 i = 0; i < physicsMesh->submeshes.arraySizes[0]; i++)
			{
				while (startTriangle < triCount && sortedIndices.getTriangleSubmesh(startTriangle) <= i)
				{
					startTriangle++;
				}
				mGraphicalLods[graphicalLodId]->physicsSubmeshPartitioning.buf[submeshPartitioningIndex + i].numSimulatedIndices = startTriangle * 3;
			}
		}

		// also adapt all mesh-mesh skinning tables

		PxU32* immediateMap = mGraphicalLods[graphicalLodId]->immediateClothMap.buf;
		if (immediateMap != NULL)
		{
			ApexPermute(immediateMap + submeshVertexOffset, sortedVertices.mNew2Old.begin(), numVertices);
		}

		for (PxU32 i = 0; i < numSkinClothMap; i++)
		{
			if (skinClothMap[i].vertexIndexPlusOffset < submeshVertexOffset)
			{
				continue;
			}
			else if (skinClothMap[i].vertexIndexPlusOffset >= submeshVertexOffset + numVertices)
			{
				break;
			}

			PxU32 oldVertexIndex = skinClothMap[i].vertexIndexPlusOffset - submeshVertexOffset;
			skinClothMap[i].vertexIndexPlusOffset = submeshVertexOffset + sortedVertices.mOld2New[oldVertexIndex];
		}

		for (PxU32 i = 0; i < numSkinClothMapB; i++)
		{
			const PxU32 vertexIndex = skinClothMapB[i].vertexIndexPlusOffset;
			if (vertexIndex >= submeshVertexOffset && vertexIndex < submeshVertexOffset + numVertices)
			{
				skinClothMapB[i].vertexIndexPlusOffset = submeshVertexOffset + sortedVertices.mOld2New[vertexIndex - submeshVertexOffset];
			}
		}

		submeshVertexOffset += numVertices;
		submeshPartitioningIndex += physicsMesh->submeshes.arraySizes[0];
	}

	// make sure all maps are sorted again, only mapC type!
	shdfnd::sort(skinClothMap, numSkinClothMap, SkinClothMapPredicate<ClothingGraphicalLodParametersNS::SkinClothMapD_Type>());

	PxU32* immediateMap = mGraphicalLods[graphicalLodId]->immediateClothMap.buf;
	if (immediateMap != NULL)
	{
		for (PxU32 i = 0; i < numSkinClothMap; i++)
		{
			PX_ASSERT((immediateMap[skinClothMap[i].vertexIndexPlusOffset] & ClothingConstants::ImmediateClothingInSkinFlag) == ClothingConstants::ImmediateClothingInSkinFlag);
			immediateMap[skinClothMap[i].vertexIndexPlusOffset] = i | ClothingConstants::ImmediateClothingInSkinFlag;
		}

#ifdef _DEBUG
		// sanity check
		for (PxU32 i = 0; i < submeshVertexOffset; i++)
		{
			PxU32 imm = immediateMap[i];
			if (imm != ClothingConstants::ImmediateClothingInvalidValue)
			{
				if ((imm & ClothingConstants::ImmediateClothingInSkinFlag) == ClothingConstants::ImmediateClothingInSkinFlag)
				{
					imm &= ClothingConstants::ImmediateClothingReadMask;

					if (numSkinClothMap > 0)
					{
						PX_ASSERT(imm < numSkinClothMap);
						PX_ASSERT(skinClothMap[imm].vertexIndexPlusOffset == i);
					}
					else if (numSkinClothMapB > 0)
					{
						PX_ASSERT(imm < numSkinClothMapB);
						PX_ASSERT(skinClothMapB[imm].vertexIndexPlusOffset == i);
					}
				}
			}
		}
#endif
	}

	return true;
}



class DeformableVerticesMaxDistancePredicate
{
public:
	DeformableVerticesMaxDistancePredicate(NxClothingConstrainCoefficients* constrainCoefficients) : mConstrainCoefficients(constrainCoefficients) {}
	bool operator()(PxU32 oldindex1, PxU32 oldIndex2) const
	{
		return mConstrainCoefficients[oldindex1].maxDistance > mConstrainCoefficients[oldIndex2].maxDistance;
	}

private:
	NxClothingConstrainCoefficients* mConstrainCoefficients;
};



bool ClothingAsset::reorderDeformableVertices(ClothingPhysicalMesh& physicalMesh)
{
	ClothingPhysicalMeshParameters* params = static_cast<ClothingPhysicalMeshParameters*>(physicalMesh.getNxParameterized());

	const PxU32 curSortingVersion = 1; // bump this number when the sorting changes!

	if (params->physicalMesh.physicalMeshSorting >= curSortingVersion)
	{
		// nothing needs to be done.
		return false;
	}

	params->physicalMesh.physicalMeshSorting = curSortingVersion;

	PxU32* indices = physicalMesh.getIndicesBuffer();

	// create mapping arrays
	Array<PxU32> newIndices(physicalMesh.getNumVertices(), (PxU32) - 1);
	Array<PxU32> oldIndices(physicalMesh.getNumVertices(), (PxU32) - 1);
	PxU32 nextIndex = 0;
	for (PxU32 i = 0; i < physicalMesh.getNumIndices(); i++)
	{
		const PxU32 vertexIndex = indices[i];
		if (newIndices[vertexIndex] == (PxU32) - 1)
		{
			newIndices[vertexIndex] = nextIndex;
			oldIndices[nextIndex] = vertexIndex;
			nextIndex++;
		}
	}

	PxU32 maxVertexIndex = 0;
	for (PxI32 i = 0; i < params->submeshes.arraySizes[0]; i++)
	{
		//PX_ASSERT(params->submeshes.buf[i].numVertices == 0); // should not have been set before!

		const PxU32 minVertexIndexForSubmesh = maxVertexIndex;
		const PxU32 lastNumIndices = i > 0 ? params->submeshes.buf[i - 1].numIndices : 0;
		for (PxU32 j = lastNumIndices; j < params->submeshes.buf[i].numIndices; j++)
		{
			const PxU32 newVertexIndex = newIndices[indices[j]];
			maxVertexIndex = PxMax(maxVertexIndex, newVertexIndex);
		}

		maxVertexIndex++;

		params->submeshes.buf[i].numVertices = maxVertexIndex;

		DeformableVerticesMaxDistancePredicate predicate(physicalMesh.getConstrainCoefficientBuffer());

		// only sort the part that this submesh has new over the previous one. This makes sure the border between them don't move anymore.
		physx::sort(oldIndices.begin() + minVertexIndexForSubmesh, maxVertexIndex - minVertexIndexForSubmesh, predicate);
	}


	// fix newIndices, the sort has destroyed them
	for (PxU32 i = 0; i < nextIndex; i++)
	{
		PX_ASSERT(newIndices[oldIndices[i]] != (PxU32) - 1);
		newIndices[oldIndices[i]] = i;
	}

	// move unused vertices to the end
	if (nextIndex < physicalMesh.getNumVertices())
	{
		// TODO check if ApexPermute works without this
		for (PxU32 i = 0; i < newIndices.size(); i++)
		{
			if (newIndices[i] == (PxU32) - 1)
			{
				newIndices[i] = nextIndex;
				oldIndices[nextIndex] = i;
				nextIndex++;
			}
		}
	}

	PX_ASSERT(physicalMesh.getNumVertices() == oldIndices.size());
	PX_ASSERT(nextIndex == physicalMesh.getNumVertices()); // at this point we assume that all vertices are referenced

	// do reordering
	physicalMesh.applyPermutation(oldIndices);

	// set max distance 0 vertices
	NxClothingConstrainCoefficients* coeffs = physicalMesh.getConstrainCoefficientBuffer();
	for (PxI32 i = 1; i < params->physicalLods.arraySizes[0]; i++)
	{
		const PxU32 submeshid = params->physicalLods.buf[i].submeshId;
		const PxU32 numVertices = params->submeshes.buf[submeshid].numVertices;
		const PxF32 maxDistanceReduction = params->physicalLods.buf[i].maxDistanceReduction;

		PxU32 vertexIndex = 0;
		for (; vertexIndex < numVertices; vertexIndex++)
		{
			if (coeffs[vertexIndex].maxDistance <= maxDistanceReduction)
			{
				break;
			}
		}

		params->submeshes.buf[submeshid].numMaxDistance0Vertices = numVertices - vertexIndex;

		// safety
		for (; vertexIndex < numVertices; vertexIndex++)
		{
			PX_ASSERT(coeffs[vertexIndex].maxDistance <= maxDistanceReduction);
		}
	}


	// clean up existing references
	for (PxU32 i = 0; i < physicalMesh.getNumIndices(); i++)
	{
		indices[i] = newIndices[indices[i]];
	}

	// update mappings into deformable vertex buffer
	for (PxU32 i = 0; i < mGraphicalLods.size(); i++)
	{
		if (mPhysicalMeshes[mGraphicalLods[i]->physicalMeshId] == physicalMesh.getNxParameterized())
		{
			ClothingGraphicalMeshAssetWrapper meshAsset(reinterpret_cast<NiApexRenderMeshAsset*>(mGraphicalLods[i]->renderMeshAssetPointer));
			ClothingGraphicalLodParameters& graphicalLod = *mGraphicalLods[i];

			const PxU32 numGraphicalVertices = meshAsset.getNumTotalVertices();

			if (graphicalLod.immediateClothMap.arraySizes[0] > 0)
			{
				for (PxU32 j = 0; j < numGraphicalVertices; j++)
				{
					if (graphicalLod.immediateClothMap.buf[j] != ClothingConstants::ImmediateClothingInvalidValue)
					{
						if ((graphicalLod.immediateClothMap.buf[j] & ClothingConstants::ImmediateClothingInSkinFlag) == 0)
						{
							const PxU32 flags = graphicalLod.immediateClothMap.buf[j] & ~ClothingConstants::ImmediateClothingReadMask;
							graphicalLod.immediateClothMap.buf[j] =
							    newIndices[graphicalLod.immediateClothMap.buf[j] & ClothingConstants::ImmediateClothingReadMask] | flags;
						}
					}
				}
			}

			for(PxI32 j = 0; j < graphicalLod.skinClothMap.arraySizes[0]; ++j)
			{
				graphicalLod.skinClothMap.buf[j].vertexIndex0 = newIndices[graphicalLod.skinClothMap.buf[j].vertexIndex0];
				graphicalLod.skinClothMap.buf[j].vertexIndex1 = newIndices[graphicalLod.skinClothMap.buf[j].vertexIndex1];
				graphicalLod.skinClothMap.buf[j].vertexIndex2 = newIndices[graphicalLod.skinClothMap.buf[j].vertexIndex2];
			}

		}
	}

	// update transition maps

	if (params->transitionDown.arraySizes[0] > 0)
	{
		for (PxI32 i = 0; i < params->transitionDown.arraySizes[0]; i++)
		{
			PxU32& vertexIndex = params->transitionDown.buf[i].vertexIndexPlusOffset;
			PX_ASSERT(vertexIndex == (PxU32)i);
			vertexIndex = newIndices[vertexIndex];
		}

		shdfnd::sort(params->transitionDown.buf, (physx::PxU32)params->transitionDown.arraySizes[0], SkinClothMapPredicate<ClothingPhysicalMeshParametersNS::SkinClothMapD_Type>());
	}

	if (params->transitionUp.arraySizes[0] > 0)
	{
		for (PxI32 i = 0; i < params->transitionUp.arraySizes[0]; i++)
		{
			PxU32& vertexIndex = params->transitionUp.buf[i].vertexIndexPlusOffset;
			PX_ASSERT(vertexIndex == (PxU32)i);
			vertexIndex = newIndices[vertexIndex];
		}
		shdfnd::sort(params->transitionUp.buf, (physx::PxU32)params->transitionUp.arraySizes[0], SkinClothMapPredicate<ClothingPhysicalMeshParametersNS::SkinClothMapD_Type>());
	}

	return true;
}



PxF32 ClothingAsset::getMaxMaxDistance(ClothingPhysicalMeshParametersNS::PhysicalMesh_Type& physicalMesh, PxU32 index, PxU32 numIndices) const
{
	PxU32* indices = physicalMesh.indices.buf;
	ClothingPhysicalMeshParametersNS::ConstrainCoefficient_Type* coeffs = physicalMesh.constrainCoefficients.buf;

	PxF32 maxDist = coeffs[indices[index]].maxDistance;
	for (PxU32 i = 1; i < numIndices; i++)
	{
		maxDist = PxMax(maxDist, coeffs[indices[index + i]].maxDistance);
	}

	return maxDist;
}



PxU32 ClothingAsset::getCorrespondingPhysicalVertices(const ClothingGraphicalLodParameters& graphLod, PxU32 submeshIndex,
													PxU32 graphicalVertexIndex, const NxAbstractMeshDescription& pMesh,
													PxU32 submeshVertexOffset, PxU32 indices[4], PxF32 trust[4]) const
{
	PX_UNUSED(submeshIndex); // stupid release mode

	PX_ASSERT(pMesh.numIndices > 0);
	PX_ASSERT(pMesh.pIndices != NULL);


	PxU32 result = 0;

	if (graphLod.immediateClothMap.arraySizes[0] > 0)
	{
		indices[0] = graphLod.immediateClothMap.buf[graphicalVertexIndex + submeshVertexOffset];
		trust[0] = 1.0f;

		if (indices[0] != ClothingConstants::ImmediateClothingInvalidValue)
		{
			if ((indices[0] & ClothingConstants::ImmediateClothingInSkinFlag) == 0)
			{
				indices[0] &= ClothingConstants::ImmediateClothingReadMask;
				result = 1;
			}
			else if (graphLod.skinClothMapB.arraySizes[0] > 0)
			{
				const PxI32 temp = PxI32(indices[0] & ClothingConstants::ImmediateClothingReadMask);
				if (temp < graphLod.skinClothMapB.arraySizes[0])
				{
					for (PxU32 i = 0; i < 3; i++)
					{
						PX_ASSERT(graphLod.skinClothMapB.buf[temp].faceIndex0 + i < pMesh.numIndices);
						indices[i] = pMesh.pIndices[graphLod.skinClothMapB.buf[temp].faceIndex0 + i];
						trust[i] = 1.0f; // PH: This should be lower for some vertices!
					}
					result = 3;
				}
			}
			else if (graphLod.skinClothMap.arraySizes[0] > 0)
			{
				const PxI32 temp = PxI32(indices[0] & ClothingConstants::ImmediateClothingReadMask);
				if (temp < graphLod.skinClothMap.arraySizes[0])
				{
					PxVec3 bary = graphLod.skinClothMap.buf[temp].vertexBary;
					bary.x = PxClamp(bary.x, 0.0f, 1.0f);
					bary.y = PxClamp(bary.y, 0.0f, 1.0f);
					bary.z = PxClamp(1.0f - bary.x - bary.y, 0.0f, 1.0f);
					PxU32 physVertIndex[3] = 
					{
						graphLod.skinClothMap.buf[temp].vertexIndex0,
						graphLod.skinClothMap.buf[temp].vertexIndex1,
						graphLod.skinClothMap.buf[temp].vertexIndex2
					};
					for (PxU32 i = 0; i < 3; i++)
					{
						//PX_ASSERT(graphLod.skinClothMap.buf[temp].faceIndex0 + i < pMesh.numIndices);
						indices[i] = physVertIndex[i];
						trust[i] = bary[i];
					}
					result = 3;
				}
			}
		}
	}
	else if (graphLod.skinClothMapB.arraySizes[0] > 0)
	{
		PX_ASSERT(graphLod.skinClothMapB.buf[graphicalVertexIndex + submeshVertexOffset].submeshIndex == submeshIndex);
		PX_ASSERT(graphLod.skinClothMapB.buf[graphicalVertexIndex + submeshVertexOffset].vertexIndexPlusOffset == graphicalVertexIndex + submeshVertexOffset);

		for (PxU32 i = 0; i < 3; i++)
		{
			PX_ASSERT(graphLod.skinClothMapB.buf[graphicalVertexIndex + submeshVertexOffset].faceIndex0 + i < pMesh.numIndices);
			indices[i] = pMesh.pIndices[graphLod.skinClothMapB.buf[graphicalVertexIndex + submeshVertexOffset].faceIndex0 + i];
			trust[i] = 1.0f;
		}
		result = 3;
	}
	else if (graphLod.skinClothMap.arraySizes[0] > 0)
	{
		// we need to do binary search here
		PxU32 curMin = 0;
		PxU32 curMax = (physx::PxU32)graphLod.skinClothMap.arraySizes[0];
		const PxU32 searchFor = graphicalVertexIndex + submeshVertexOffset;
		while (curMax > curMin)
		{
			PxU32 middle = (curMin + curMax) >> 1;
			PX_ASSERT(middle == graphLod.skinClothMap.buf[middle].vertexIndexPlusOffset);
			const PxU32 probeResult = middle;
			if (probeResult < searchFor)
			{
				curMin = middle + 1;
			}
			else
			{
				curMax = middle;
			}
		}

		PX_ASSERT(curMin == graphLod.skinClothMap.buf[curMin].vertexIndexPlusOffset);
		if (curMin == searchFor)
		{
			PxVec3 bary = graphLod.skinClothMap.buf[curMin].vertexBary;
			bary.x = PxClamp(bary.x, 0.0f, 1.0f);
			bary.y = PxClamp(bary.y, 0.0f, 1.0f);
			bary.z = PxClamp(1.0f - bary.x - bary.y, 0.0f, 1.0f);

			PxU32 physVertIndex[3] = 
			{
				graphLod.skinClothMap.buf[curMin].vertexIndex0,
				graphLod.skinClothMap.buf[curMin].vertexIndex1,
				graphLod.skinClothMap.buf[curMin].vertexIndex2
			};
			for (PxU32 i = 0; i < 3; i++)
			{
			//	PX_ASSERT(graphLod.skinClothMap.buf[curMin].faceIndex0 + i < pMesh.numIndices);
				indices[i] = physVertIndex[i];
				trust[i] = bary[i];
			}
			result = 3;
		}
	}
	else if (graphLod.tetraMap.arraySizes[0] > 0)
	{
		for (PxU32 i = 0; i < 4; i++)
		{
			PX_ASSERT(graphLod.tetraMap.buf[graphicalVertexIndex + submeshVertexOffset].tetraIndex0 + i < pMesh.numIndices);
			indices[i] = pMesh.pIndices[graphLod.tetraMap.buf[graphicalVertexIndex + submeshVertexOffset].tetraIndex0 + i];
			trust[i] = 1.0f;
		}
		result = 4;
	}

	for (PxU32 i = 0; i < result; i++)
	{
		PX_ASSERT(trust[i] >= 0.0f);
		PX_ASSERT(trust[i] <= 1.0f);
	}

	return result;
}



void ClothingAsset::getNormalsAndVerticesForFace(PxVec3* vtx, PxVec3* nrm, PxU32 i, const NxAbstractMeshDescription& srcPM) const
{
	// copy indices for convenience
	PX_ASSERT(i < srcPM.numIndices);
	PxU32 di[3];
	for (PxU32 j = 0; j < 3; j++)
	{
		di[j] = srcPM.pIndices[i + j];
	}

	// To guarantee consistency in our implicit tetrahedral mesh definition we must always order vertices
	// idx[0,1,2] = min, max and mid
	PxU32 idx[3];
	idx[0] = PxMin(di[0], PxMin(di[1], di[2]));
	idx[1] = PxMax(di[0], PxMax(di[1], di[2]));
	idx[2] = idx[0];
	for (PxU32 j = 0; j < 3; j++)
	{
		if ((idx[0] != di[j]) && (idx[1] != di[j]))
		{
			idx[2] = di[j];
		}
	}

	for (PxU32 j = 0; j < 3; j++)
	{
		vtx[j] = srcPM.pPosition[idx[j]];
		nrm[j] = srcPM.pNormal[idx[j]];
#ifdef _DEBUG
		// sanity
		// PH: These normals 'should' always be normalized, maybe we can get rid of the normalize completely!
		const PxF32 length = nrm[j].magnitudeSquared();
		if (!(length >= 0.99f && length <= 1.01f))
		{
			static bool first = true;
			if (first)
			{
				PX_ALWAYS_ASSERT();
				first = false;
			}
		}
#else
		// PH: let's try and disable it in release mode...
		//nrm[j].normalize();
#endif
	};
}



bool ClothingAsset::setBoneName(PxU32 internalIndex, const char* name)
{
	NxParameterized::Handle bonesHandle(*mParams);
	mParams->getParameterHandle("bones", bonesHandle);

	if (bonesHandle.isValid())
	{
		NxParameterized::Handle boneHandle(*mParams);
		bonesHandle.getChildHandle((physx::PxI32)internalIndex, boneHandle);

		if (boneHandle.isValid())
		{
			NxParameterized::Handle nameHandle(*mParams);
			boneHandle.getChildHandle(mParams, "name", nameHandle);

			if (nameHandle.isValid())
			{
				mParams->setParamString(nameHandle, name);
				return true;
			}
		}
	}
	return false;
}



void ClothingAsset::clearMapping(PxU32 graphicalLodIndex)
{
	if (mGraphicalLods[graphicalLodIndex]->immediateClothMap.buf != NULL)
	{
		NxParamArray<PxU32> immediateClothMap(mGraphicalLods[graphicalLodIndex], "immediateClothMap", reinterpret_cast<NxParamDynamicArrayStruct*>(&mGraphicalLods[graphicalLodIndex]->immediateClothMap));
		immediateClothMap.clear();
	}
	if (mGraphicalLods[graphicalLodIndex]->skinClothMapB.buf != NULL)
	{
		NxParamArray<ClothingGraphicalLodParametersNS::SkinClothMapB_Type> skinClothMapB(mGraphicalLods[graphicalLodIndex], "skinClothMapB", reinterpret_cast<NxParamDynamicArrayStruct*>(&mGraphicalLods[graphicalLodIndex]->skinClothMapB));
		skinClothMapB.clear();
	}
	if (mGraphicalLods[graphicalLodIndex]->tetraMap.buf != NULL)
	{
		NxParamArray<ClothingGraphicalLodParametersNS::TetraLink_Type> tetraMap(mGraphicalLods[graphicalLodIndex], "tetraMap", reinterpret_cast<NxParamDynamicArrayStruct*>(&mGraphicalLods[graphicalLodIndex]->tetraMap));
		tetraMap.clear();
	}
}



bool ClothingAsset::findTriangleForImmediateVertex(PxU32& faceIndex, PxU32& indexInTriangle, PxU32 physVertIndex, ClothingPhysicalMeshParametersNS::PhysicalMesh_Type& physicalMesh) const
{
	// find triangle with smallest faceIndex, indices have been sorted during authoring
	// such that simulated triangles are first
	for (physx::PxU32 physIndex = 0; physIndex < (physx::PxU32)physicalMesh.indices.arraySizes[0]; physIndex++)
	{
		if (physicalMesh.indices.buf[physIndex] == physVertIndex)
		{
			// this is a triangle that contains the vertex from the immediate map)
			PxU32 currentFaceIndex = physIndex - (physIndex%3);

			faceIndex = currentFaceIndex;
			indexInTriangle = physIndex - faceIndex;
			return true;
		}
	}

	return false;
}



// we can't just regenerate the skinClothMap, because master/slave info is only available during authoring
bool ClothingAsset::mergeMapping(ClothingGraphicalLodParameters* graphicalLod)
{
	if (graphicalLod->immediateClothMap.buf == NULL)
		return false;

	NxParamArray<physx::PxU32> immediateMap(graphicalLod, "immediateClothMap", reinterpret_cast<NxParamDynamicArrayStruct*>(&graphicalLod->immediateClothMap));

	// size of immediateMap (equals number of graphical vertices)
	PxU32 immediateCount = (physx::PxU32)graphicalLod->immediateClothMap.arraySizes[0];

	NxParamArray<SkinClothMap> skinClothMap(graphicalLod, "skinClothMap",
		reinterpret_cast<NxParamDynamicArrayStruct*>(&graphicalLod->skinClothMap));

	PxU32 oldSkinMapSize = skinClothMap.size();
	skinClothMap.resize(immediateCount);

	SkinClothMap* mapEntry = &skinClothMap[oldSkinMapSize];

	// get RenderMeshAsset
	ClothingGraphicalMeshAssetWrapper renderMesh(reinterpret_cast<NiApexRenderMeshAsset*>(graphicalLod->renderMeshAssetPointer));
	PxU32 numSubmeshes = renderMesh.getSubmeshCount();

	ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* physicalMesh = &mPhysicalMeshes[graphicalLod->physicalMeshId]->physicalMesh;
	PX_ASSERT(physicalMesh != NULL);

	// some updates for the case where we didn't have a skinClothMap yet
	if (graphicalLod->skinClothMapThickness == 0.0f)
	{
		graphicalLod->skinClothMapThickness = 1.0f;
	}
	if (graphicalLod->skinClothMapOffset == 0.0f)
	{
		graphicalLod->skinClothMapOffset = DEFAULT_PM_OFFSET_ALONG_NORMAL_FACTOR * physicalMesh->averageEdgeLength;
	}

	PxU32 invalidCount = 0;

	// iterate through immediate map, append verts to skinClothMap that are not yet in there
	PxU32 mapVertIndex = 0;
	for (PxU32 submeshIndex = 0; submeshIndex < numSubmeshes; ++submeshIndex)
	{
		NxRenderDataFormat::Enum outFormat;
		const PxVec4* tangents = (const PxVec4*)renderMesh.getVertexBuffer(submeshIndex, NxRenderVertexSemantic::TANGENT, outFormat);
		PX_ASSERT(tangents == NULL || outFormat == NxRenderDataFormat::FLOAT4);

		const PxVec3* positions = (const PxVec3*)renderMesh.getVertexBuffer(submeshIndex, NxRenderVertexSemantic::POSITION, outFormat);
		PX_ASSERT(positions == NULL || outFormat == NxRenderDataFormat::FLOAT3);

		const PxVec3* normals = (const PxVec3*)renderMesh.getVertexBuffer(submeshIndex, NxRenderVertexSemantic::NORMAL, outFormat);
		PX_ASSERT(normals == NULL || outFormat == NxRenderDataFormat::FLOAT3);

		for (PxU32 submeshVertIndex = 0; submeshVertIndex < renderMesh.getNumVertices(submeshIndex); ++submeshVertIndex, ++mapVertIndex)
		{
			PX_ASSERT(mapVertIndex < immediateCount);

			PxU32 physVertIndex = 0;
			if (immediateMap[mapVertIndex] == ClothingConstants::ImmediateClothingInvalidValue)
			{
				++invalidCount;
				continue;
			}

			physVertIndex = immediateMap[mapVertIndex] & ClothingConstants::ImmediateClothingReadMask;

			if ((immediateMap[mapVertIndex] & ClothingConstants::ImmediateClothingInSkinFlag) > 0)
			{
				// in that case physVertIndex is the index in the skinClothMap.
				// set it to the current index, so we can sort it afterwards
				skinClothMap[physVertIndex].vertexIndexPlusOffset = mapVertIndex;
				continue;
			}
			
			// find triangle mapping
			PxU32 faceIndex = PX_MAX_U32;
			PxU32 indexInTriangle = PX_MAX_U32;
			findTriangleForImmediateVertex(faceIndex, indexInTriangle, physVertIndex, *physicalMesh);

			mapEntry->vertexIndex0 = physicalMesh->indices.buf[faceIndex + 0];
			mapEntry->vertexIndex1 = physicalMesh->indices.buf[faceIndex + 1];
			mapEntry->vertexIndex2 = physicalMesh->indices.buf[faceIndex + 2];

			// for immediate skinned verts
			// position, normal and tangent all have the same barycentric coord on the triangle
			PxVec3 bary(0.0f);
			if (indexInTriangle < 2)
			{
				bary[indexInTriangle] = 1.0f;
			}

			mapEntry->vertexBary = bary;
			// mapEntry->vertexBary.z = 0 because it's on the triangle

			// offset the normal
			mapEntry->normalBary = bary;
			mapEntry->normalBary.z = graphicalLod->skinClothMapOffset;

			// we need to compute tangent bary's because there are no tangents on physical mesh
			bary.x = bary.y = bary.z = PX_MAX_U32;
			if (positions != NULL && normals != NULL && tangents != NULL)
			{
				PxVec3 dummy(0.0f);
				PxVec3 position = positions[submeshVertIndex];
				PxVec3 tangent = tangents[submeshVertIndex].getXYZ();

				// prepare triangle data
				TriangleWithNormals triangle;
				triangle.valid = 0;
				triangle.faceIndex0 = faceIndex;
				PxVec3* physNormals = physicalMesh->skinningNormals.buf;
				if (physNormals == NULL)
				{
					physNormals = physicalMesh->normals.buf;
				}
				PX_ASSERT(physNormals != NULL);
				for (physx::PxU32 j = 0; j < 3; j++)
				{
					PxU32 triVertIndex = physicalMesh->indices.buf[triangle.faceIndex0 + j];
					triangle.vertices[j] = physicalMesh->vertices.buf[triVertIndex];
					triangle.normals[j] = physNormals[triVertIndex];
				}
				triangle.init();

				ModuleClothingHelpers::computeTriangleBarys(triangle, dummy, dummy, position + tangent, graphicalLod->skinClothMapOffset, 0, true);
				mapEntry->tangentBary = triangle.tempBaryTangent;
			}

			mapEntry->vertexIndexPlusOffset = mapVertIndex;
			mapEntry++;
		}
	}

	skinClothMap.resize(skinClothMap.size() - invalidCount);

	// make sure skinClothMap is sorted by graphical vertex order
	shdfnd::sort(graphicalLod->skinClothMap.buf, (physx::PxU32)graphicalLod->skinClothMap.arraySizes[0], SkinClothMapPredicate<ClothingGraphicalLodParametersNS::SkinClothMapD_Type>());

	immediateMap.clear();

	// notify actors of the asset change
	for (PxU32 i = 0; i < getNumActors(); ++i)
	{
		ClothingActor& actor = DYNAMIC_CAST(ClothingActorProxy*)(mActors.getResource(i))->impl;
		actor.reinitActorData();
	}

	return true;
}


void ClothingAsset::updateBoundingBox()
{
	PxBounds3 tempBounds;
	tempBounds.setEmpty();

	for (PxU32 graphicalMeshId = 0; graphicalMeshId < mGraphicalLods.size(); graphicalMeshId++)
	{
		if (mGraphicalLods[graphicalMeshId]->renderMeshAssetPointer == NULL)
			continue;

		NiApexRenderMeshAsset* renderMeshAsset = reinterpret_cast<NiApexRenderMeshAsset*>(mGraphicalLods[graphicalMeshId]->renderMeshAssetPointer);
		for (PxU32 submeshIndex = 0; submeshIndex < renderMeshAsset->getSubmeshCount(); submeshIndex++)
		{
			const NxVertexBuffer& vb = renderMeshAsset->getNiSubmesh(submeshIndex).getVertexBuffer();
			const NxVertexFormat& vf = vb.getFormat();
			physx::PxU32 bufferIndex = (physx::PxU32)vf.getBufferIndexFromID(vf.getSemanticID(physx::apex::NxRenderVertexSemantic::POSITION));
			NxRenderDataFormat::Enum positionFormat;
			const PxVec3* pos = (const physx::PxVec3*)vb.getBufferAndFormat(positionFormat, bufferIndex);
			PX_ASSERT(positionFormat == physx::NxRenderDataFormat::FLOAT3);
			const PxU32 numVertices = renderMeshAsset->getNiSubmesh(submeshIndex).getVertexCount(0);
			for (PxU32 i = 0; i < numVertices; i++)
			{
				tempBounds.include(pos[i]);
			}
		}
	}
	for (PxU32 i = 0; i < mPhysicalMeshes.size(); i++)
	{
		NxAbstractMeshDescription other;
		other.pPosition = mPhysicalMeshes[i]->physicalMesh.vertices.buf;

		for (PxU32 j = 0; j < other.numVertices; j++)
		{
			tempBounds.include(other.pPosition[j]);
		}
	}

	mParams->boundingBox = tempBounds;
}



PxF32 ClothingAsset::getMaxDistReduction(ClothingPhysicalMeshParameters& physicalMesh, PxF32 maxDistanceMultiplier) const
{
	return physicalMesh.physicalMesh.maximumMaxDistance * (1.0f - maxDistanceMultiplier);
}



#ifndef WITHOUT_PVD
void ClothingAsset::initPvdInstances(physx::debugger::comm::PvdDataStream& pvdStream)
{
	NxApexResource* pvdInstance = static_cast<NxApexResource*>(this);

	// Asset Params
	pvdStream.createInstance(NamespacedName(APEX_PVD_NAMESPACE, "ClothingAssetParameters"), mParams);
	pvdStream.setPropertyValue(pvdInstance, "AssetParams", DataRef<const PxU8>((const PxU8*)&mParams, sizeof(ClothingAssetParameters*)), getPvdNamespacedNameForType<ObjectRef>());

	// update asset param properties (should we do this per frame? if so, how?)
	PVD::PvdBinding* pvdBinding = NxGetApexSDK()->getPvdBinding();
	PX_ASSERT(pvdBinding != NULL);
	pvdBinding->updatePvd(mParams, *mParams);
	
	mActors.initPvdInstances(pvdStream);
}



void ClothingAsset::destroyPvdInstances()
{
	PVD::PvdBinding* pvdBinding = NxGetApexSDK()->getPvdBinding();
	if (pvdBinding != NULL)
	{
		if (pvdBinding->getConnectionType() & PvdConnectionType::eDEBUG)
		{
			pvdBinding->lock();
			physx::debugger::comm::PvdDataStream* pvdStream = pvdBinding->getDataStream();
			{
				if (pvdStream != NULL)
				{
					pvdBinding->updatePvd(mParams, *mParams, PVD::PvdAction::DESTROY);
					pvdStream->destroyInstance(mParams);
				}
			}
			pvdBinding->unlock();
		}
	}
}
#endif

}
}
} // namespace physx::apex

#endif // NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED
