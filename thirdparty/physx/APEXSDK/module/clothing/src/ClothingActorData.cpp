/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "foundation/PxPreprocessor.h"
#include "NxApexRenderDataFormat.h"
#include "ClothingActorData.h"
#include "PsUtilities.h"
#include "AbstractMeshDescription.h"
#include "foundation/PxMemory.h"
#include "PsVecMath.h"
#include "foundation/PxMat44.h"

#include "PlatformMemory.h"
#include "PsVecMath.h"

#include "ClothingGlobals.h"

#if defined(PX_PS3) && defined(__SPU__)
// no perf scopes on PS3
#define PX_PROFILER_PERF_SCOPE(name) /* void */
#else
#include "ProfilerCallback.h"
#endif

using namespace physx::shdfnd;
using namespace physx::shdfnd::aos;

#pragma warning(disable : 4101 4127) // unreferenced local variable and conditional is constant

#define NX_PARAMETERIZED_ONLY_LAYOUTS
#include "ClothingGraphicalLodParameters.h"


namespace physx
{
namespace apex
{
namespace clothing
{




ClothingActorData::ClothingActorData() :
	mNewBounds(PxBounds3::empty()),

	mGlobalPose(PxVec4(1.0f)),
	mInternalGlobalPose(PxVec4(1.0f)),

	mInternalBoneMatricesCur(NULL),
	mInternalBoneMatricesPrev(NULL),
	mRenderingDataPosition(NULL),
	mRenderingDataNormal(NULL),
	mRenderingDataTangent(NULL),
	mMorphDisplacementBuffer(NULL),
	mSdkWritebackNormal(NULL),
	mSdkWritebackPositions(NULL),
	mSkinnedPhysicsPositions(NULL),
	mSkinnedPhysicsNormals(NULL),

	mInternalMatricesCount(0),
	mMorphDisplacementBufferCount(0),
	mSdkDeformableVerticesCount(0),
	mSdkDeformableIndicesCount(0),
	mCurrentGraphicalLodId(0),
	mCurrentPhysicsSubmesh(0),

	mActorScale(0.0f),

	bMeshMeshSkinningOnPPU(false),
	bInternalFrozen(false),
	bShouldComputeRenderData(false),
	bIsInitialized(false),
	bIsSimulationMeshDirty(false),
	bRecomputeNormals(false),
	bRecomputeTangents(false),
	bCorrectSimulationNormals(false),
	bParallelCpuSkinning(false),
	bIsClothingSimulationNull(false),
	bAllGraphicalSubmeshesFitOnSpu(false),
	bSkinPhysicsMeshSpu(false),
	bSkinPhysicsMeshSpuValid(false)
{
}



ClothingActorData::~ClothingActorData()
{
	PX_ASSERT(mInternalBoneMatricesCur == NULL); // properly deallocated
}



void ClothingActorData::renderDataLock()
{
	//TODO - grab a mutex here (needs to be written for SPU/PPU synchronisations)
	mRenderLock.lock();
}



void ClothingActorData::renderDataUnLock()
{
	//TODO - release a mutex here
	mRenderLock.unlock();
}



void ClothingActorData::skinPhysicsMaxDist0Normals_NoPhysx()
{
	if (mSdkWritebackNormal == NULL /*|| bInternalFrozen == 1*/)
	{
		return;
	}

	//ClothingPhysicalMeshParametersNS::PhysicalMesh_Type* physicalMesh = mAsset->getPhysicalMeshFromLod(mCurrentGraphicalLodId);

	ClothingPhysicalMeshData* physicalMesh = mAsset.GetPhysicalMeshFromLod(mCurrentGraphicalLodId);
	const PxVec3* PX_RESTRICT _normals = physicalMesh->mSkinningNormals;

	if (_normals == NULL)
	{
		return;
	}



	const ClothingPhysicalMeshSubmeshData& physicalSubMesh = *mAsset.GetPhysicalSubmesh(physicalMesh, mCurrentPhysicsSubmesh);

	if (physicalSubMesh.mMaxDistance0VerticesCount == 0)
	{
		return;
	}

	memory::MemBaseMarker outer("skinPhysicsMaxDist0Normals");

	const PxU32 startVertex = physicalSubMesh.mVertexCount - physicalSubMesh.mMaxDistance0VerticesCount;
	const PxU32 numVertices = physicalSubMesh.mVertexCount;
	const PxU32 numBoneIndicesPerVertex = physicalMesh->mNumBonesPerVertex;

	// offset the normals array as well
	_normals += startVertex;

	memory::MemBaseMarker outerMarker("skinPhysicsMaxDist0Normals");

	const PxU32 UnrollSize = 160;
	const PxU32 vertCount = numVertices - startVertex;
	const PxU32 numIterations = (vertCount + UnrollSize - 1) / UnrollSize;

#ifndef __SPU__
	PxVec3* PX_RESTRICT targetNormals = mSdkWritebackNormal + startVertex;
#else
	PxVec3* PX_RESTRICT eaTargetNormals = mSdkWritebackNormal + startVertex;

	PxVec3* PX_RESTRICT targetNormals = (PxVec3 * const PX_RESTRICT)memory::SpuAllocMatchAlignment(sizeof(PxVec3) * UnrollSize, eaTargetNormals);
#endif


	//uint32_t tags[2] = {10, 11};
	//const PxU32 prefetchRange = (startVertex & 0xfffffff0); //A multiple of 16 before this prefetch, with the assumption that normals is 16-byte aligned!
	//C_Prefetcher<2, sizeof(PxVec3) * UnrollSize> normPrefetcher(tags, (void*)(normals + prefetchRange), (void*)(normals + numVertices));

	if (mInternalBoneMatricesCur == NULL || numBoneIndicesPerVertex == 0)
	{
		if (mActorScale == 1.0f)
		{
			for (PxU32 a = 0; a < numIterations; ++a)
			{
				memory::MemBaseMarker innerMarker("skinPhysicsMaxDist0Normals::noActorScale");
				const PxU32 numToProcess = PxMin(UnrollSize, (vertCount - (UnrollSize * a)));
				const PxVec3* PX_RESTRICT localNormals = (const PxVec3 * PX_RESTRICT)memory::FetchAllocUnaligned((void*)_normals, sizeof(PxVec3) * numToProcess);
				for (PxU32 i = 0; i < numToProcess; i++)
				{
					targetNormals[i] = mInternalGlobalPose.rotate(localNormals[i]);
				}
#ifndef __SPU__
				targetNormals += UnrollSize;
#else
				memory::SpuStoreUnaligned(eaTargetNormals, targetNormals, sizeof(PxVec3) * numToProcess);
				eaTargetNormals += UnrollSize;
#endif
				_normals += UnrollSize;
			}
		}
		else
		{
			const PxF32 recipActorScale = 1.f / mActorScale;
			for (PxU32 a = 0; a < numIterations; ++a)
			{
				memory::MemBaseMarker innerMarker("skinPhysicsMaxDist0Normals::actorScale");
				const PxU32 numToProcess = PxMin(UnrollSize, (vertCount - (UnrollSize * a)));
				const PxVec3* PX_RESTRICT localNormals = (const PxVec3 * PX_RESTRICT)memory::FetchAllocUnaligned((void*)_normals, sizeof(PxVec3) * numToProcess);
				for (PxU32 i = 0; i < numToProcess; i++)
				{
					targetNormals[i] = mInternalGlobalPose.rotate(localNormals[i]) * recipActorScale;
				}
#ifndef __SPU__
				targetNormals += UnrollSize;
#else
				memory::SpuStoreUnaligned(eaTargetNormals, targetNormals, sizeof(PxVec3) * numToProcess);
				eaTargetNormals += UnrollSize;
#endif
				_normals += UnrollSize;
			}
		}
	}
	else
	{
		memory::MemBaseMarker middleMarker("skinPhysicsMaxDist0Normals::bones");
		//OK a slight refactor is required here - we don't want to fetch in everything only to
		const PxU32 startBoneIndex = startVertex * numBoneIndicesPerVertex;
		//Another problem - this is an arbitrarily large amount of data that has to be fetched here!!!! Consider revising
		//const PxU16* const PX_RESTRICT simBoneIndices = (const PxU16* const PX_RESTRICT)SpuFetchAllocUnaligned((void*)&physicalMesh->m_pBoneIndices[startIndex], sizeof(PxU16) * numBoneIndicesPerVertex * numToProcess);
		//const PxF32* const PX_RESTRICT simBoneWeights = (const PxF32* const PX_RESTRICT)SpuFetchAllocUnaligned((void*)&physicalMesh->m_pBoneWeights[startIndex], sizeof(PxF32) * numBoneIndicesPerVertex * numToProcess);

		const PxU16* PX_RESTRICT eaSimBoneIndices = &physicalMesh->mBoneIndices[startBoneIndex];
		const PxF32* PX_RESTRICT eaSimBoneWeights = &physicalMesh->mBoneWeights[startBoneIndex];

		const PxMat44* const PX_RESTRICT matrices = (const PxMat44*)memory::FetchAllocUnaligned(mInternalBoneMatricesCur, sizeof(PxMat44) * mInternalMatricesCount);

		for (PxU32 a = 0; a < numIterations; ++a)
		{
			memory::MemBaseMarker innerMarker("skinPhysicsMaxDist0Normals::bonesInner");

			const PxU32 numToProcess = PxMin(UnrollSize, (vertCount - (UnrollSize * a)));
			const PxVec3* PX_RESTRICT localNormals = (const PxVec3 * PX_RESTRICT)memory::FetchAllocUnaligned((void*)_normals, sizeof(PxVec3) * numToProcess);

			const PxU16* const PX_RESTRICT simBoneIndices = (const PxU16 * const PX_RESTRICT)memory::FetchAllocUnaligned((void*)eaSimBoneIndices, sizeof(PxU16) * numBoneIndicesPerVertex * numToProcess);
			const PxF32* const PX_RESTRICT simBoneWeights = (const PxF32 * const PX_RESTRICT)memory::FetchAllocUnaligned((void*)eaSimBoneWeights, sizeof(PxF32) * numBoneIndicesPerVertex * numToProcess);

			eaSimBoneIndices += numBoneIndicesPerVertex * numToProcess;
			eaSimBoneWeights += numBoneIndicesPerVertex * numToProcess;

			for (PxU32 i = 0; i < numToProcess; i++)
			{
				PxVec3 normal(0.0f, 0.0f, 0.0f);
				for (PxU32 j = 0; j < numBoneIndicesPerVertex; j++)
				{
					const PxF32 weight = simBoneWeights[i * numBoneIndicesPerVertex + j];

					if (weight > 0.f)
					{
						PX_ASSERT(weight <= 1.0f);
						const PxU32 index = simBoneIndices[i * numBoneIndicesPerVertex + j];

						const PxMat44& bone = matrices[index];

						normal += bone.rotate(localNormals[i]) * weight; // 12% here
					}
					else
					{
						// PH: Assuming sorted weights is faster
						break;
					}
				}

				//normal *= NxClothingUserRecompute::invSqrt(normal.magnitudeSquared());
				normal.normalize();
				targetNormals[i] = normal;
			}
#ifndef __SPU__
			targetNormals += UnrollSize;
#else
			memory::SpuStoreUnaligned(eaTargetNormals, targetNormals, sizeof(PxVec3) * numToProcess);
			eaTargetNormals += UnrollSize;
#endif
			_normals += UnrollSize;
		}

	}

}


void ClothingActorData::skinToAnimation_NoPhysX(bool fromFetchResults)
{
	// This optimization only works if the render data from last frame is still there.
	// So this can only be used if we're using the same ClothingRenderProxy again.
	//if (!bIsSimulationMeshDirty)
	//{
	//	return;
	//}

	PX_PROFILER_PERF_SCOPE("ClothingActor::skinToAnimation");

	using namespace physx::shdfnd::aos;

	//const bool recomputeNormals = bRecomputeNormals;

	// PH: If fromFetchResults is true, renderLock does not need to be aquired as it is already aquired by ApexScene::fetchResults()
	if (!fromFetchResults)
	{
		renderDataLock();
	}

	//PxVec3* pMorphDisplacementBuffer = m_pMorphDisplacementBuffer ? (PxVec3*)SpuLargeFetchAllocUnaligned(m_pMorphDisplacementBuffer, sizeof(PxVec3) * m_MorphDisplacementBufferCount) : NULL;
	memory::MorphTargetCache cache(mMorphDisplacementBuffer, mMorphDisplacementBuffer + mMorphDisplacementBufferCount, "SkinToAnimation::cache");


	for (PxU32 graphicalLod = 0; graphicalLod < mAsset.mGraphicalLodsCount; graphicalLod++)
	{
		ClothingMeshAssetData& meshAsset = *mAsset.GetLod(graphicalLod);
		if (!meshAsset.bActive)
		{
			continue;
		}

		PxU32 submeshVertexOffset = 0;

		for (PxU32 submeshIndex = 0; submeshIndex < meshAsset.mSubMeshCount; submeshIndex++)
		{
			memory::MemBaseMarker innerMarker("skinToAnimation::submesh");

			NxAbstractMeshDescription renderData;
			ClothingAssetSubMesh* pSubMesh = mAsset.GetSubmesh(&meshAsset, submeshIndex);
			renderData.numVertices = pSubMesh->mVertexCount;

			renderData.pPosition = mRenderingDataPosition + submeshVertexOffset;

			renderData.pNormal = mRenderingDataNormal + submeshVertexOffset;

			if (mRenderingDataTangent != NULL)
			{
				renderData.pTangent4 = mRenderingDataTangent + submeshVertexOffset;
			}

			PxMat44* matrices = NULL;
			PX_ALIGN(16, PxMat44 alignedGlobalPose); // matrices must be 16 byte aligned!
			if (mInternalBoneMatricesCur == NULL)
			{
				matrices = &alignedGlobalPose;
				alignedGlobalPose = mInternalGlobalPose;
			}
			else
			{
				matrices = (PxMat44*)memory::FetchAllocUnaligned(mInternalBoneMatricesCur, mInternalMatricesCount * sizeof(PxMat44));
				PX_ASSERT(matrices != NULL);
			}

			mAsset.skinToBones(renderData, submeshIndex, graphicalLod, pSubMesh->mCurrentMaxVertexSimulation, matrices, mMorphDisplacementBuffer, cache);

			submeshVertexOffset += pSubMesh->mVertexCount;
		}
	}

	if (!fromFetchResults)
	{
		renderDataUnLock();
	}
}

template<bool computeNormals>
PxU32 ClothingAssetData::skinClothMap(PxVec3* dstPositions, PxVec3* dstNormals, PxVec4* dstTangents, PxU32 numVertices,
									const NxAbstractMeshDescription& srcPM, ClothingGraphicalLodParametersNS::SkinClothMapD_Type* map,
									PxU32 numVerticesInMap, PxF32 offsetAlongNormal, PxF32 actorScale) const
{
	PX_ASSERT(srcPM.numIndices % 3 == 0);

	const ClothingGraphicalLodParametersNS::SkinClothMapD_Type* PX_RESTRICT pTCM = map;
	physx::shdfnd::prefetchLine(pTCM);

	const PxF32 invOffsetAlongNormal = 1.0f / offsetAlongNormal;

	PxU32 numVerticesWritten = 0;
	PxU32 numTangentsWritten = 0;
	const PxU32 numVerticesTotal = numVertices;

	PxU32 firstMiss = numVerticesInMap;

	const PxU32 unrollCount = 256;

	PxVec3* endDstPositions = dstPositions + numVertices;
	PxVec3* endDstNormals = dstNormals + numVertices;
	PxVec4* endDstTangents = dstTangents + numVertices;

	memory::VectorCache vertCache(srcPM.pPosition, srcPM.pPosition + srcPM.numVertices, "skinClothMap::vertCache");
	memory::VectorCache normalCache(srcPM.pNormal, srcPM.pNormal + srcPM.numVertices, "skinClothMap::normalCache");
	memory::VectorWriteCache vertexWriteCache(dstPositions, endDstPositions, 1, 2, "skinClothMap::vertexWriteCache");
	memory::VectorWriteCache normalWriteCache(dstNormals, endDstNormals, 3, 4, "skinClothMap::normalWriteCache");
	memory::VectorWriteCache tangentWriteCache(dstTangents, endDstTangents, 5, 6, "skinClothMap::tangentWriteCache");

	const PxU32 numIterations = (numVerticesInMap + unrollCount - 1) / unrollCount;


	PxU32 tags[2] = {10, 11};
	memory::Prefetcher<2, sizeof(ClothingGraphicalLodParametersNS::SkinClothMapD_Type) * unrollCount> mapPrefetcher(tags,
			(void*)pTCM,
			(void*)(pTCM + numVerticesInMap),
			"skinClothMap::mapPrefetcher");

	//PxU32 vertexIndex = 0;
	for (PxU32 a = 0; a < numIterations; ++a)
	{
		const PxU32 numToProcess = PxMin(numVerticesInMap - (a * unrollCount), unrollCount);
		//const SkinClothMapC_TypeLocal* PX_RESTRICT pTCMLocal = (const SkinClothMapC_TypeLocal* PX_RESTRICT)SpuFetchAllocUnaligned((void*)pTCM, sizeof(SkinClothMapC_TypeLocal) * numToProcess);
		const ClothingGraphicalLodParametersNS::SkinClothMapD_Type* PX_RESTRICT pTCMLocal =
			(const ClothingGraphicalLodParametersNS::SkinClothMapD_Type * PX_RESTRICT)
			mapPrefetcher.GetLocalAddress((void*)pTCM, sizeof(ClothingGraphicalLodParametersNS::SkinClothMapD_Type) * numToProcess);

		for (PxU32 j = 0; j < numToProcess; ++j)
		{
			PX_ASSERT(memory::VerifyStack());
			physx::shdfnd::prefetchLine(pTCMLocal + 1);

			//PX_ASSERT(vertexIndex == pTCMLocal->vertexIndexPlusOffset);
			PxU32 vertexIndex = pTCMLocal->vertexIndexPlusOffset;
			const PxU32 physVertIndex0 = pTCMLocal->vertexIndex0;
			const PxU32 physVertIndex1 = pTCMLocal->vertexIndex1;
			const PxU32 physVertIndex2 = pTCMLocal->vertexIndex2;

			if (vertexIndex >= numVerticesTotal)
			{
				pTCM++;
				pTCMLocal++;
				//vertexIndex++;
				continue;
			}

			// TODO do only 1 test, make sure physVertIndex0 is the smallest index
			if (physVertIndex0 >= srcPM.numVertices || physVertIndex1 >= srcPM.numVertices || physVertIndex2 >= srcPM.numVertices)
			{
				firstMiss = PxMin(firstMiss, vertexIndex);
				pTCM++;
				pTCMLocal++;
				//vertexIndex++;
				continue;
			}

			numVerticesWritten++;

			//PX_ASSERT(!vertexWriteCache.IsStomped());

			PX_ASSERT(memory::VerifyStack());
			//PX_ASSERT(!vertexWriteCache.IsStomped());

			const PxVec3 vtx[3] =
			{
				*(PxVec3*)vertCache.GetMemoryAddress(&srcPM.pPosition[physVertIndex0]),
				*(PxVec3*)vertCache.GetMemoryAddress(&srcPM.pPosition[physVertIndex1]),
				*(PxVec3*)vertCache.GetMemoryAddress(&srcPM.pPosition[physVertIndex2]),
			};

			/*const PxVec3 vtx[3] =
			{
				*(PxVec3*)SpuFetchAllocUnaligned(&srcPM.pPosition[idx[0]], sizeof(PxVec3)),
				*(PxVec3*)SpuFetchAllocUnaligned(&srcPM.pPosition[idx[1]], sizeof(PxVec3)),
				*(PxVec3*)SpuFetchAllocUnaligned(&srcPM.pPosition[idx[2]], sizeof(PxVec3)),
			};*/

			PX_ASSERT(memory::VerifyStack());
			//PX_ASSERT(!vertexWriteCache.IsStomped());

			const PxVec3 nrm[3] =
			{
				*(PxVec3*)normalCache.GetMemoryAddress(&srcPM.pNormal[physVertIndex0]),
				*(PxVec3*)normalCache.GetMemoryAddress(&srcPM.pNormal[physVertIndex1]),
				*(PxVec3*)normalCache.GetMemoryAddress(&srcPM.pNormal[physVertIndex2]),
			};

			PX_ASSERT(memory::VerifyStack());
			//PX_ASSERT(!vertexWriteCache.IsStomped());

			PxVec3 bary = pTCMLocal->vertexBary;
			const PxF32 vHeight = bary.z * actorScale;
			bary.z = 1.0f - bary.x - bary.y;

			const PxVec3 positionVertex = bary.x * vtx[0] + bary.y * vtx[1] + bary.z * vtx[2];
			const PxVec3 positionNormal = (bary.x * nrm[0] + bary.y * nrm[1] + bary.z * nrm[2]) * vHeight;

			const PxVec3 resultPosition = positionVertex + positionNormal;
			//Write back - to use a DMA list

			PxVec3* dstPosition = (PxVec3*)vertexWriteCache.GetLocalAddress(&dstPositions[vertexIndex]);

			*dstPosition = resultPosition;

			PX_ASSERT(resultPosition.isFinite());

			if (computeNormals)
			{
				bary = pTCMLocal->normalBary;
				const PxF32 nHeight = bary.z * actorScale;
				bary.z = 1.0f - bary.x - bary.y;

				const PxVec3 normalVertex = bary.x * vtx[0] + bary.y * vtx[1] + bary.z * vtx[2];
				const PxVec3 normalNormal = (bary.x * nrm[0] + bary.y * nrm[1] + bary.z * nrm[2]) * nHeight;

				PxVec3* dstNormal = (PxVec3*)normalWriteCache.GetLocalAddress(&dstNormals[vertexIndex]);
				//PxVec3* dstNormal = (PxVec3*)SpuFetchAllocUnaligned(&dstNormals[vertexIndex], sizeof(PxVec3));

				// we multiply in invOffsetAlongNormal in order to get a newNormal that is closer to size 1,
				// so the normalize approximation will be better
				PxVec3 newNormal = ((normalVertex + normalNormal) - (resultPosition)) * invOffsetAlongNormal;
#if 1
				// PH: Normally this is accurate enough. For testing we can also use the second
				const PxVec3 resultNormal = newNormal * physx::shdfnd::recipSqrtFast(newNormal.magnitudeSquared());
				*dstNormal = resultNormal;
#else
				newNormal.normalize();
				*dstNormal = newNormal;
#endif
			}
			if (dstTangents != NULL)
			{
				bary = pTCMLocal->tangentBary;
				const PxF32 nHeight = bary.z * actorScale;
				bary.z = 1.0f - bary.x - bary.y;

				const PxVec3 tangentVertex = bary.x * vtx[0] + bary.y * vtx[1] + bary.z * vtx[2];
				const PxVec3 tangentTangent = (bary.x * nrm[0] + bary.y * nrm[1] + bary.z * nrm[2]) * nHeight;

				PxVec4* dstTangent = (PxVec4*)tangentWriteCache.GetLocalAddress(&dstTangents[vertexIndex]);

				// we multiply in invOffsetAlongNormal in order to get a newNormal that is closer to size 1,
				// so the normalize approximation will be better
				PxVec3 newTangent = ((tangentVertex + tangentTangent) - (resultPosition)) * invOffsetAlongNormal;
#if 1
				// PH: Normally this is accurate enough. For testing we can also use the second
				const PxVec3 resultTangent = newTangent * physx::shdfnd::recipSqrtFast(newTangent.magnitudeSquared());

				PxU32 arrayIndex	= numTangentsWritten / 4;
				PxU32 offset		= numTangentsWritten % 4;
				PxF32 w = ((mCompressedTangentW[arrayIndex] >> offset) & 1) ? 1.f : -1.f;

				*dstTangent = PxVec4(resultTangent, w);
#else
				newTangent.normalize();
				*dstTangent = newTangent;
#endif
			}
			PX_ASSERT(memory::VerifyStack());

			pTCM++;
			pTCMLocal++;
			//vertexIndex++;
		}
	}

	return firstMiss;
}


#if defined(PX_ANDROID)
template PxU32 ClothingAssetData::skinClothMap<true>(PxVec3* dstPositions, PxVec3* dstNormals, PxVec4* dstTangents, PxU32 numVertices,
                                       const NxAbstractMeshDescription& srcPM, ClothingGraphicalLodParametersNS::SkinClothMapD_Type* map,
                                       PxU32 numVerticesInMap, PxF32 offsetAlongNormal, PxF32 actorScale) const;

template PxU32 ClothingAssetData::skinClothMap<false>(PxVec3* dstPositions, PxVec3* dstNormals, PxVec4* dstTangents, PxU32 numVertices,
                                       const NxAbstractMeshDescription& srcPM, ClothingGraphicalLodParametersNS::SkinClothMapD_Type* map,
                                       PxU32 numVerticesInMap, PxF32 offsetAlongNormal, PxF32 actorScale) const;
#endif

void ClothingActorData::skinToImmediateMap(const PxU32* immediateClothMap_, PxU32 numGraphicalVertices_, PxU32 numSrcVertices_,
        const PxVec3* srcPositions_)
{
	const PxU32* PX_RESTRICT immediateClothMap = immediateClothMap_;

	const PxVec3* PX_RESTRICT srcPositions = srcPositions_;
	PxVec3* PX_RESTRICT destPositions = mRenderingDataPosition;

	const PxU32 numGraphicalVertices = numGraphicalVertices_;
	const PxU32 numSrcVertices = numSrcVertices_;

	const PxU32 WorkSize = 512;

	const PxU32 numIterations = (numGraphicalVertices + WorkSize - 1) / WorkSize;

	memory::MorphTargetCache srcPositionCache(srcPositions, srcPositions + numSrcVertices, "skinToImmediateMap::srcPositionCache");


	PxU32 tags[2] = {10, 11};
	PxU32 tags2[] = {12, 13, 14};
	memory::Prefetcher<2, sizeof(PxU32) * WorkSize> mapPrefetcher(tags, (void*)immediateClothMap, (void*)(immediateClothMap + numGraphicalVertices), "skinToImmediateMap::mapPrefetcher");
	memory::StreamReaderWriter<sizeof(PxVec3) * WorkSize> posPrefetcher(tags2, (void*)destPositions, (void*)(destPositions + numGraphicalVertices), "skinToImmediateMap::posPrefetcher");

	for (PxU32 a = 0; a < numIterations; ++a)
	{
		const PxU32 numToProcess = PxMin(numGraphicalVertices - (a * WorkSize), WorkSize);

		const PxU32* PX_RESTRICT immediateClothMapLocal = (const PxU32 * PX_RESTRICT)mapPrefetcher.GetLocalAddress((void*)&immediateClothMap[a * WorkSize], sizeof(PxU32) * numToProcess);
		PxVec3* PX_RESTRICT destPositionsLocal = (PxVec3 * PX_RESTRICT)posPrefetcher.GetLocalAddress((void*)&destPositions[a * WorkSize], sizeof(PxVec3) * numToProcess);

		for (PxU32 j = 0; j < numToProcess; ++j)
		{
			const PxU32 mapEntry = immediateClothMapLocal[j];
			const PxU32 index = mapEntry & ClothingConstants::ImmediateClothingReadMask;
			const PxU32 flags = mapEntry & ~ClothingConstants::ImmediateClothingReadMask;

			if (index < numSrcVertices && ((flags & ClothingConstants::ImmediateClothingInSkinFlag)) == 0)
			{
				destPositionsLocal[j] = *((PxVec3*)srcPositionCache.GetMemoryAddress((void*)&srcPositions[index]));
				PX_ASSERT(destPositionsLocal[j].isFinite());
			}
		}
	}
}



void ClothingActorData::skinToImmediateMap(const PxU32* immediateClothMap_, PxU32 numGraphicalVertices_, PxU32 numSrcVertices_,
        const PxVec3* srcPositions_, const PxVec3* srcNormals_)
{
	const PxU32* PX_RESTRICT immediateClothMap = immediateClothMap_;

	const PxVec3* PX_RESTRICT srcPositions = srcPositions_;
	const PxVec3* PX_RESTRICT srcNormals = srcNormals_;

	PxVec3* PX_RESTRICT destPositions = mRenderingDataPosition;
	PxVec3* PX_RESTRICT destNormals = mRenderingDataNormal;

	const PxU32 numGraphicalVertices = numGraphicalVertices_;
	const PxU32 numSrcVertices = numSrcVertices_;

	const PxU32 WorkSize = 160;

	//__builtin_snpause();

	const PxU32 numIterations = (numGraphicalVertices + WorkSize - 1) / WorkSize;
	memory::MorphTargetCache srcPositionCache(srcPositions, srcPositions + numSrcVertices, "skinToImmediateMap::srcPositionCache");
	memory::MorphTargetCache srcNormalCache(srcNormals, srcNormals + numSrcVertices, "skinToImmediateMap::srcNormalCache");

	PxU32 tags[2] = {7, 8};
	PxU32 tags2[] = {9, 10, 11};
	PxU32 tags3[] = {12, 13, 14};
	memory::Prefetcher<2, sizeof(PxU32) * WorkSize> mapPrefetcher(tags, (void*)immediateClothMap, (void*)(immediateClothMap + numGraphicalVertices), "skinToImmediateMap::mapPrefetcher");
	memory::StreamReaderWriter<sizeof(PxVec3) * WorkSize> posPrefetcher(tags2, (void*)destPositions, (void*)(destPositions + numGraphicalVertices), "skinToImmediateMap::posPrefetcher");
	memory::StreamReaderWriter<sizeof(PxVec3) * WorkSize> normPrefetcher(tags3, (void*)destNormals, (void*)(destNormals + numGraphicalVertices), "skinToImmediateMap::normPrefetcher");


	for (PxU32 a = 0; a < numIterations; ++a)
	{
		const PxU32 numToProcess = PxMin(numGraphicalVertices - (a * WorkSize), WorkSize);

		const PxU32* PX_RESTRICT immediateClothMapLocal = (const PxU32 * PX_RESTRICT)mapPrefetcher.GetLocalAddress((void*)&immediateClothMap[a * WorkSize], sizeof(PxU32) * numToProcess);
		PxVec3* PX_RESTRICT destPositionsLocal = (PxVec3 * PX_RESTRICT)posPrefetcher.GetLocalAddress((void*)&destPositions[a * WorkSize], sizeof(PxVec3) * numToProcess);
		PxVec3* PX_RESTRICT destNormalsLocal = (PxVec3 * PX_RESTRICT)normPrefetcher.GetLocalAddress((void*)&destNormals[a * WorkSize], sizeof(PxVec3) * numToProcess);

		for (PxU32 j = 0; j < numToProcess; ++j)
		{
			const PxU32 mapEntry = immediateClothMapLocal[j];
			const PxU32 index = mapEntry & ClothingConstants::ImmediateClothingReadMask;
			const PxU32 flags = mapEntry & ~ClothingConstants::ImmediateClothingReadMask;

			if (index < numSrcVertices && ((flags & ClothingConstants::ImmediateClothingInSkinFlag)) == 0)
			{
				destPositionsLocal[j] = *((PxVec3*)srcPositionCache.GetMemoryAddress((void*)&srcPositions[index]));
				PX_ASSERT(destPositionsLocal[j].isFinite());

				const PxVec3 destNormal = *((PxVec3*)srcNormalCache.GetMemoryAddress((void*)&srcNormals[index]));
				destNormalsLocal[j] = (flags & ClothingConstants::ImmediateClothingInvertNormal) ? -destNormal : destNormal;
				PX_ASSERT(destNormalsLocal[j].isFinite());
			}
		}
	}
}



void ClothingActorData::skinToPhysicalMesh_NoPhysX(bool fromFetchResults)
{
	// This optimization only works if the render data from last frame is still there.
	// So this can only be used if we're using the same ClothingRenderProxy again.
	//if (!bIsSimulationMeshDirty)
	//{
	//	return;
	//}

	PX_PROFILER_PERF_SCOPE("ClothingActor::meshMesh-Skinning");

	const ClothingMeshAssetData& graphicalLod = *mAsset.GetLod(mCurrentGraphicalLodId);

	const ClothingPhysicalMeshData* physicalMesh = mAsset.GetPhysicalMeshFromLod(mCurrentGraphicalLodId);

	NxAbstractMeshDescription pcm;
	pcm.numVertices = mSdkDeformableVerticesCount;
	pcm.numIndices = mSdkDeformableIndicesCount;
	pcm.pPosition = mSdkWritebackPositions;
	pcm.pNormal = mSdkWritebackNormal;
	pcm.pIndices = physicalMesh->mIndices;
	pcm.avgEdgeLength = graphicalLod.mSkinClothMapThickness;

	const bool skinNormals = !bRecomputeNormals;

	if (!fromFetchResults)
	{
		renderDataLock();
	}

	PxU32 activeCount = 0;

	for (PxU32 i = 0; i < mAsset.mGraphicalLodsCount; i++)
	{
		const ClothingMeshAssetData& lod = *mAsset.GetLod(i);
		if (!lod.bActive)
		{
			continue;
		}
		activeCount++;

		bool skinTangents = !bRecomputeTangents;

		PxU32 graphicalVerticesCount = 0;
		for (PxU32 j = 0; j < lod.mSubMeshCount; j++)
		{
			ClothingAssetSubMesh* subMesh = mAsset.GetSubmesh(&lod, j);
			graphicalVerticesCount += subMesh->mVertexCount; // only 1 part is supported

			if (subMesh->mTangents == NULL)
			{
				skinTangents = false;
			}
		}

		//__builtin_snpause();
		//NiApexRenderMeshAsset* renderMeshAsset = mAsset->getGraphicalMesh(i);
		//PX_ASSERT(renderMeshAsset != NULL);

		// Do mesh-to-mesh skinning here
		if (graphicalLod.mSkinClothMapB != NULL)
		{
			mAsset.skinClothMapB(mRenderingDataPosition, mRenderingDataNormal, graphicalVerticesCount, pcm,
			                     graphicalLod.mSkinClothMapB, graphicalLod.mSkinClothMapBCount, skinNormals);
		}
		else if (graphicalLod.mSkinClothMap != NULL)
		{
			PxVec4* tangents = skinTangents ? mRenderingDataTangent : NULL;
			if (skinNormals)
				mAsset.skinClothMap<true>(mRenderingDataPosition, mRenderingDataNormal, tangents, graphicalVerticesCount, pcm,
											graphicalLod.mSkinClothMap, graphicalLod.mSkinClothMapCount, graphicalLod.mSkinClothMapOffset, mActorScale);
			else
				mAsset.skinClothMap<false>(mRenderingDataPosition, mRenderingDataNormal, tangents, graphicalVerticesCount, pcm,
											graphicalLod.mSkinClothMap, graphicalLod.mSkinClothMapCount, graphicalLod.mSkinClothMapOffset, mActorScale);

		}
		else if (graphicalLod.mTetraMap != NULL)
		{
			NxAbstractMeshDescription destMesh;
			destMesh.pPosition = mRenderingDataPosition;
			if (skinNormals)
			{
				destMesh.pNormal = mRenderingDataNormal;
			}
			destMesh.numVertices = graphicalVerticesCount;
			mAsset.skinToTetraMesh(destMesh, pcm, graphicalLod);
		}

		if (graphicalLod.mImmediateClothMap != NULL)
		{
			if (skinNormals)
			{
				skinToImmediateMap(graphicalLod.mImmediateClothMap, graphicalVerticesCount, pcm.numVertices, pcm.pPosition, pcm.pNormal);
			}
			else
			{
				skinToImmediateMap(graphicalLod.mImmediateClothMap, graphicalVerticesCount, pcm.numVertices, pcm.pPosition);
			}
		}
	}

	PX_ASSERT(activeCount < 2);

	if (!fromFetchResults)
	{
		renderDataUnLock();
	}
}



void ClothingActorData::finalizeSkinning_NoPhysX(bool fromFetchResults)
{
	// PH: If fromFetchResults is true, renderLock does not need to be aquired as it is already aquired by ApexScene::fetchResults()
	if (!fromFetchResults)
	{
		renderDataLock();
	}

	mNewBounds.setEmpty();

	for (PxU32 graphicalLod = 0; graphicalLod < mAsset.mGraphicalLodsCount; graphicalLod++)
	{
		ClothingMeshAssetData& renderMeshAsset = *mAsset.GetLod(graphicalLod);
		if (!renderMeshAsset.bActive)
		{
			continue;
		}

		const PxU32 submeshCount = renderMeshAsset.mSubMeshCount;

		PxU32 submeshVertexOffset = 0;
		for (PxU32 submeshIndex = 0; submeshIndex < submeshCount; submeshIndex++)
		{
			NxAbstractMeshDescription renderData;

			ClothingAssetSubMesh* pSubmesh = mAsset.GetSubmesh(&renderMeshAsset, submeshIndex);

			renderData.numVertices = pSubmesh->mVertexCount;

			renderData.pPosition = mRenderingDataPosition + submeshVertexOffset;

			bool recomputeTangents = bRecomputeTangents && renderMeshAsset.bNeedsTangents;
			if (bRecomputeNormals || recomputeTangents)
			{
				renderData.pNormal = mRenderingDataNormal + submeshVertexOffset;

				const PxU32* compressedTangentW = NULL;

				if (recomputeTangents)
				{
					renderData.pTangent4 = mRenderingDataTangent + submeshVertexOffset;
					PxU32 mapSize = 0;
					compressedTangentW = mAsset.getCompressedTangentW(graphicalLod, submeshIndex, mapSize);
				}

				if (bRecomputeNormals && recomputeTangents)
				{
					PX_PROFILER_PERF_SCOPE("ClothingActor::recomupteNormalAndTangent");
					computeTangentSpaceUpdate<true, true>(renderData, renderMeshAsset, submeshIndex, compressedTangentW);
				}
				else if (bRecomputeNormals)
				{
					PX_PROFILER_PERF_SCOPE("ClothingActor::recomputeNormal");
					computeTangentSpaceUpdate<true, false>(renderData, renderMeshAsset, submeshIndex, compressedTangentW);
				}
				else
				{
					PX_PROFILER_PERF_SCOPE("ClothingActor::recomputeTangent");
					computeTangentSpaceUpdate<false, true>(renderData, renderMeshAsset, submeshIndex, compressedTangentW);
				}
			}

			const PxU32 unrollCount = 1024;
			const PxU32 numIterations = (renderData.numVertices + unrollCount - 1) / unrollCount;

			PxU32 tags[2] = {9, 10};
			memory::Prefetcher<2, sizeof(PxVec3) * unrollCount> posPrefetcher(tags, (void*)renderData.pPosition, (void*)(renderData.pPosition + renderData.numVertices), "finalizeSkinning::posPrefetcher");
			for (PxU32 a = 0; a < numIterations; ++a)
			{
				const PxU32 numToProcess = PxMin(unrollCount, renderData.numVertices - (a * unrollCount));
				const PxVec3* PX_RESTRICT positions = (const PxVec3 * PX_RESTRICT)posPrefetcher.GetLocalAddress(renderData.pPosition + (a * unrollCount), sizeof(PxVec3) * numToProcess);
				for (PxU32 b = 0; b < numToProcess; ++b)
				{
					mNewBounds.include(positions[b]);
				}
			}

			submeshVertexOffset += renderData.numVertices;
		}
	}

	if (!fromFetchResults)
	{
		renderDataUnLock();
	}
}



void ClothingActorData::finalizeSkinningSimple_NoPhysX(bool fromFetchResults)
{
	// PH: If fromFetchResults is true, renderLock does not need to be aquired as it is already aquired by ApexScene::fetchResults()
	if (!fromFetchResults)
	{
		renderDataLock();
	}

	mNewBounds.setEmpty();

	for (PxU32 graphicalLod = 0; graphicalLod < mAsset.mGraphicalLodsCount; graphicalLod++)
	{
		ClothingMeshAssetData& renderMeshAsset = *mAsset.GetLod(graphicalLod);
		if (!renderMeshAsset.bActive)
		{
			continue;
		}

		memory::MemBaseMarker outerMarker("finalizeSkinningSimple::submesh");

		const PxU32 submeshCount = renderMeshAsset.mSubMeshCount;

		PxU32 submeshVertexOffset = 0;
		for (PxU32 submeshIndex = 0; submeshIndex < submeshCount; submeshIndex++)
		{
			NxAbstractMeshDescription renderData;

			ClothingAssetSubMesh* pSubmesh = mAsset.GetSubmesh(&renderMeshAsset, submeshIndex);

			renderData.numVertices = pSubmesh->mVertexCount;

			renderData.pPosition = mRenderingDataPosition + submeshVertexOffset;

			bool recomputeTangents = bRecomputeTangents && renderMeshAsset.bNeedsTangents;
			if (bRecomputeNormals || recomputeTangents)
			{
				renderData.pNormal = mRenderingDataNormal + submeshVertexOffset;

				const PxU32* compressedTangentW = NULL;

				if (recomputeTangents)
				{
					renderData.pTangent4 = mRenderingDataTangent + submeshVertexOffset;
					PxU32 mapSize = 0;
					compressedTangentW = mAsset.getCompressedTangentW(graphicalLod, submeshIndex, mapSize);
				}
				if (bRecomputeNormals && recomputeTangents)
				{
					PX_PROFILER_PERF_SCOPE("ClothingActor::recomupteNormalAndTangent");
					computeTangentSpaceUpdateSimple<true, true>(renderData, renderMeshAsset, submeshIndex, compressedTangentW);
				}
				else if (bRecomputeNormals)
				{
					PX_PROFILER_PERF_SCOPE("ClothingActor::recomupteNormal");
					computeTangentSpaceUpdateSimple<true, false>(renderData, renderMeshAsset, submeshIndex, compressedTangentW);
				}
				else
				{
					PX_PROFILER_PERF_SCOPE("ClothingActor::recomupteTangent");
					computeTangentSpaceUpdateSimple<false, true>(renderData, renderMeshAsset, submeshIndex, compressedTangentW);
				}
			}

			const PxU32 unrollCount = 1024;
			const PxU32 numIterations = (renderData.numVertices + unrollCount - 1) / unrollCount;

			PxU32 tags[2] = {9, 10};
			memory::Prefetcher<2, sizeof(PxVec3) * unrollCount> posPrefetcher(tags,
			        (void*)renderData.pPosition,
			        (void*)(renderData.pPosition + renderData.numVertices),
			        "finalizeSkinningSimple::posPrefetcher");

			for (PxU32 a = 0; a < numIterations; ++a)
			{
				const PxU32 numToProcess = PxMin(unrollCount, renderData.numVertices - (a * unrollCount));
				const PxVec3* PX_RESTRICT positions = (const PxVec3 * PX_RESTRICT)posPrefetcher.GetLocalAddress(renderData.pPosition + (a * unrollCount), sizeof(PxVec3) * numToProcess);
				for (PxU32 b = 0; b < numToProcess; ++b)
				{
					mNewBounds.include(positions[b]);
				}
			}

			submeshVertexOffset += renderData.numVertices;
		}
	}

	if (!fromFetchResults)
	{
		renderDataUnLock();
	}
}

#define FLOAT_TANGENT_UPDATE 0


template <bool withNormals, bool withTangents>
void ClothingActorData::computeTangentSpaceUpdate(NxAbstractMeshDescription& destMesh,
        const ClothingMeshAssetData& rendermesh, PxU32 submeshIndex, const PxU32* compressedTangentW)
{
	//__builtin_snpause();
	using namespace physx::shdfnd::aos;
	ClothingAssetSubMesh* pSubMesh = mAsset.GetSubmesh(&rendermesh, submeshIndex);

	PX_ASSERT(withTangents == (compressedTangentW != NULL));

	if (withNormals && withTangents)
	{
		computeTangentSpaceUpdate<true, false>(destMesh, rendermesh, submeshIndex, NULL);
		computeTangentSpaceUpdate<false, true>(destMesh, rendermesh, submeshIndex, compressedTangentW);
	}
	else
	{
		const NxRenderDataFormat::Enum uvFormat = pSubMesh->mUvFormat;

		if (uvFormat != NxRenderDataFormat::FLOAT2)
		{
			if (withNormals)
			{
				computeTangentSpaceUpdate<true, false>(destMesh, rendermesh, submeshIndex, compressedTangentW);
			}

			return;
		}

		const PxU32 numGraphicalVertexIndices =	pSubMesh->mCurrentMaxIndexSimulation;
		const PxU32* indices =					pSubMesh->mIndices;

		const NxVertexUVLocal* PX_RESTRICT uvs = pSubMesh->mUvs;
		PX_ASSERT(uvs != NULL);

		const PxU32 numVertices = pSubMesh->mCurrentMaxVertexAdditionalSimulation;
		const PxU32 numZeroVertices = pSubMesh->mCurrentMaxVertexSimulation;
		PX_ASSERT(numVertices <= destMesh.numVertices);

		PX_ASSERT(pSubMesh->mVertexCount == destMesh.numVertices);
		PX_ASSERT(destMesh.pPosition != NULL);
		PX_ASSERT(destMesh.pNormal != NULL);
		PX_ASSERT(destMesh.pTangent4 != NULL || !withTangents);
		PX_ASSERT(destMesh.pTangent == NULL);
		PX_ASSERT(destMesh.pBitangent == NULL);
#ifndef __SPU__
		if (withNormals)
		{
			physx::PxMemSet(destMesh.pNormal, 0, sizeof(physx::PxVec3) * numZeroVertices);
		}

		if (withTangents)
		{
			physx::PxMemSet(destMesh.pTangent4, 0, sizeof(physx::PxVec4) * numZeroVertices);
		}
#else
		//Zero all the data!!!
		if (withNormals || withTangents)
		{
			memory::MemBaseMarker outerMarker("computeTangentSpaceUpdate::zero");
			PxU32* data = (PxU32*)memory::SpuAlloc(sizeof(PxU32) * 3 * 1024);
			//CELL_ALIGN(16, PxU32 data[3*1024]);
			//memSet falls over on SPU so using something else...
			//memset(data, 0, sizeof(PxVec3) * 1024);
			for (PxU32 a = 0; a < 3 * 1024; ++a)
			{
				data[a] = 0;
			}

			const PxI32 numIterations = (numZeroVertices + 1023) / 1024;
			const PxU32 lastPageSize = numZeroVertices - (1024 * (numIterations - 1));
			if (withNormals)
			{
				for (PxI32 a = 0; a < (numIterations - 1); ++a)
				{
					memory::SpuStoreAsync(destMesh.pNormal + (a * 1024), data, sizeof(PxVec3) * 1024, 0);
				}

				memory::SpuStoreUnalignedAsync(destMesh.pNormal + ((numIterations - 1) * 1024), data, sizeof(PxVec3) * lastPageSize, 0);
			}

			if (withTangents)
			{
				//DMA a bunch of 0s over the top of this data...
				for (PxI32 a = 0; a < (numIterations - 1); ++a)
				{
					memory::SpuStoreAsync(destMesh.pTangent4 + (a * 1024), data, sizeof(PxVec4) * 1024, 0);
				}

				memory::SpuStoreUnalignedAsync(destMesh.pTangent4 + ((numIterations - 1) * 1024), data, sizeof(PxVec4) * lastPageSize, 0);
			}
			memory::SpuWaitDma(0);
		}
#endif

		//All indices read in in blocks of 3...hence need to fetch in an exact multiple of 3...

		const PxU32 UnrollSize = 192; //exactly divisible by 16 AND 3 :-)

		const PxU32 numIterations = (numGraphicalVertexIndices + UnrollSize - 1) / UnrollSize;

		memory::VectorCache2 destPositionCache(destMesh.pPosition, destMesh.pPosition + numVertices, "computeTangentSpaceUpdate::destPositionCache");
		destPositionCache.WarmStartCache();
		//These read/write caches will write back data. Need to pre-declare the range they work on
		//to avoid them splatting over memory not inside the range

		if (withNormals)
		{
			{
				memory::VectorReadWriteCache3 destNormalCache(destMesh.pNormal, destMesh.pNormal + numVertices, 6, "computeTangentSpaceUpdate::destNormalCache");
				destNormalCache.WarmStartCache();

				PxU32 tags[2] = {9, 10};
				memory::Prefetcher<2, sizeof(PxU32) * UnrollSize> indicesPrefetcher(tags,
				        (void*)indices,
				        (void*)(indices + numGraphicalVertexIndices),
				        "computeTangentSpaceUpdate::indicesPrefetcher");

				for (PxU32 a = 0; a < numIterations; ++a)
				{
					//__builtin_snpause();
					const PxU32 numToProcess = PxMin(numGraphicalVertexIndices - (a * UnrollSize), UnrollSize);
					const PxU32* localIndices = (const PxU32*)indicesPrefetcher.GetLocalAddress((void*)(indices + (a * UnrollSize)), sizeof(PxU32) * numToProcess);

					for (PxU32 i = 0; i < numToProcess; i += 3)
					{
						const PxU32 i0 = localIndices[i + 0];
						const PxU32 i1 = localIndices[i + 1];
						const PxU32 i2 = localIndices[i + 2];
#if FLOAT_TANGENT_UPDATE
						const PxVec3 p0 = *((const PxVec3*)destPositionCache.GetMemoryAddress(&destMesh.pPosition[i0], destMesh.pPosition));
						const PxVec3 p1 = *((const PxVec3*)destPositionCache.GetMemoryAddress(&destMesh.pPosition[i1], destMesh.pPosition));
						const PxVec3 p2 = *((const PxVec3*)destPositionCache.GetMemoryAddress(&destMesh.pPosition[i2], destMesh.pPosition));

						PxVec3 faceNormal = (p1 - p0).cross(p2 - p0);
						faceNormal *= 1.0f / faceNormal.magnitude();

						PxVec3* normal0 = (PxVec3*)destNormalCache.GetMemoryAddress(&destMesh.pNormal[i0]);
						*normal0 += faceNormal;

						PxVec3* normal1 = (PxVec3*)destNormalCache.GetMemoryAddress(&destMesh.pNormal[i1]);
						*normal1 += faceNormal;

						PxVec3* normal2 = (PxVec3*)destNormalCache.GetMemoryAddress(&destMesh.pNormal[i2]);
						*normal2 += faceNormal;
#else
						Vec3V P0 = V3LoadU(*((const PxVec3*)destPositionCache.GetMemoryAddress(&destMesh.pPosition[i0])));
						Vec3V P1 = V3LoadU(*((const PxVec3*)destPositionCache.GetMemoryAddress(&destMesh.pPosition[i1])));
						Vec3V P2 = V3LoadU(*((const PxVec3*)destPositionCache.GetMemoryAddress(&destMesh.pPosition[i2])));

						Vec3V X1 = V3Sub(P1, P0);
						Vec3V X2 = V3Sub(P2, P0);

						Vec3V FACENORMAL = V3Cross(X1, X2);

						PxVec3* nor1 = (PxVec3*)destNormalCache.GetMemoryAddress(&destMesh.pNormal[i0]);
						Vec3V n1 = V3LoadU(*nor1);
						n1 = V3Add(n1, FACENORMAL);
						V3StoreU(n1, *nor1);


						PxVec3* nor2 = (PxVec3*)destNormalCache.GetMemoryAddress(&destMesh.pNormal[i1]);
						Vec3V n2 = V3LoadU(*nor2);
						n2 = V3Add(n2, FACENORMAL);
						V3StoreU(n2, *nor2);

						PxVec3* nor3 = (PxVec3*)destNormalCache.GetMemoryAddress(&destMesh.pNormal[i2]);
						Vec3V n3 = V3LoadU(*nor3);
						n3 = V3Add(n3, FACENORMAL);
						V3StoreU(n3, *nor3);

#endif
					}
				}
			}
		}

		if (withTangents)
		{
			{
				memory::VectorReadWriteCache3 tangentCache(destMesh.pTangent4, destMesh.pTangent4 + numVertices, 7, "computeTangentSpaceUpdate::tangentCache");
				tangentCache.WarmStartCache();
				memory::NxVertexUVLocalCache uvLocalCache(uvs, uvs + numVertices, "computeTangentSpaceUpdate::uvLocalCache");
				PxU32 tags[2] = {9, 10};
				memory::Prefetcher<2, sizeof(PxU32) * UnrollSize> indicesPrefetcher(tags,
				        (void*)indices,
				        (void*)(indices + numGraphicalVertexIndices),
				        "computeTangentSpaceUpdate::indicesPrefetcher");


#if !FLOAT_TANGENT_UPDATE
				const FloatV vZero = FZero();
#endif
				for (PxU32 a = 0; a < numIterations; ++a)
				{
					//__builtin_snpause();
					const PxU32 numToProcess = PxMin(numGraphicalVertexIndices - (a * UnrollSize), UnrollSize);
					const PxU32* localIndices = (const PxU32*)indicesPrefetcher.GetLocalAddress((void*)(indices + (a * UnrollSize)), sizeof(PxU32) * numToProcess);

					for (PxU32 i = 0; i < numToProcess; i += 3)
					{
						const PxU32 i0 = localIndices[i + 0];
						const PxU32 i1 = localIndices[i + 1];
						const PxU32 i2 = localIndices[i + 2];
#if FLOAT_TANGENT_UPDATE
						const PxVec3 p0 = *((const PxVec3*)destPositionCache.GetMemoryAddress(&destMesh.pPosition[i0], destMesh.pPosition));
						const PxVec3 p1 = *((const PxVec3*)destPositionCache.GetMemoryAddress(&destMesh.pPosition[i1], destMesh.pPosition));
						const PxVec3 p2 = *((const PxVec3*)destPositionCache.GetMemoryAddress(&destMesh.pPosition[i2], destMesh.pPosition));

						const NxVertexUVLocal w0 = *((NxVertexUVLocal*)uvLocalCache.GetMemoryAddress((void*)&uvs[i0], uvs));
						const NxVertexUVLocal w1 = *((NxVertexUVLocal*)uvLocalCache.GetMemoryAddress((void*)&uvs[i1], uvs));
						const NxVertexUVLocal w2 = *((NxVertexUVLocal*)uvLocalCache.GetMemoryAddress((void*)&uvs[i2], uvs));

						const float s1 = w1.u - w0.u;
						const float s2 = w2.u - w0.u;
						const float t1 = w1.v - w0.v;
						const float t2 = w2.v - w0.v;

						const float invH = (s1 * t2 - s2 * t1);


						if (invH != 0.0f)
						{
							const float x1 = p1.x - p0.x;
							const float x2 = p2.x - p0.x;
							const float y1 = p1.y - p0.y;
							const float y2 = p2.y - p0.y;
							const float z1 = p1.z - p0.z;
							const float z2 = p2.z - p0.z;

							const float r = 1.0f / invH;

							// sdir = ((t2,t2,t2)*(x1,y1,z1) - (t1,t1,t1)*(x2,y2,z2)) * r
							physx::PxVec3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
							physx::PxVec3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

							PxVec3* tangent0 = (PxVec3*)tangentCache.GetMemoryAddress(&destMesh.pTangent4[i0]);
							*tangent0 += sdir;

							PxVec3* tangent1 = (PxVec3*)tangentCache.GetMemoryAddress(&destMesh.pTangent4[i1]);
							*tangent1 += sdir;

							PxVec3* tangent2 = (PxVec3*)tangentCache.GetMemoryAddress(&destMesh.pTangent4[i2]);
							*tangent2 += sdir;
						}
#else
						Vec3V P0 = V3LoadU(*((const PxVec3*)destPositionCache.GetMemoryAddress(&destMesh.pPosition[i0])));
						Vec3V P1 = V3LoadU(*((const PxVec3*)destPositionCache.GetMemoryAddress(&destMesh.pPosition[i1])));
						Vec3V P2 = V3LoadU(*((const PxVec3*)destPositionCache.GetMemoryAddress(&destMesh.pPosition[i2])));

						Vec3V X1 = V3Sub(P1, P0);
						Vec3V X2 = V3Sub(P2, P0);

						const NxVertexUVLocal w0 = *((NxVertexUVLocal*)uvLocalCache.GetMemoryAddress((void*)&uvs[i0]));
						const NxVertexUVLocal w1 = *((NxVertexUVLocal*)uvLocalCache.GetMemoryAddress((void*)&uvs[i1]));
						const NxVertexUVLocal w2 = *((NxVertexUVLocal*)uvLocalCache.GetMemoryAddress((void*)&uvs[i2]));

						const FloatV W0U = FLoad(w0.u);
						const FloatV W1U = FLoad(w1.u);
						const FloatV W2U = FLoad(w2.u);
						const FloatV W0V = FLoad(w0.v);
						const FloatV W1V = FLoad(w1.v);
						const FloatV W2V = FLoad(w2.v);

						//This could be just 1 sub...
						const FloatV S1 = FSub(W1U, W0U);
						const FloatV S2 = FSub(W2U, W0U);
						const FloatV T1 = FSub(W1V, W0V);
						const FloatV T2 = FSub(W2V, W0V);

						// invH = (s1 * t2 - s2 * t1);
						const FloatV S1T2 = FMul(S1, T2);
						const FloatV invHR = FNegMulSub(S2, T1, S1T2);
						const FloatV HR = FRecipFast(invHR);
						const Vec3V T2X1 = V3Scale(X1, T2);
						//const Vec3V S1X2 = V3Scale(X2, S1);
						const BoolV invHREqZero = FIsEq(invHR, vZero);

						const Vec3V T1X2MT2X1 = V3NegScaleSub(X2, T1, T2X1);
						//const Vec3V S2X1MS1X2 = V3NegScaleSub(X1, S2, S1X2);

						const FloatV scale = FSel(invHREqZero, vZero, HR);

						Vec3V SDIR = V3Scale(T1X2MT2X1, scale);
						//Vec3V TDIR = V3Scale(S2X1MS1X2, scale);

						PxVec3* tangent0 = (PxVec3*)tangentCache.GetMemoryAddress(&destMesh.pTangent4[i0]);
						Vec3V t0 = V3LoadU(*tangent0);
						t0 = V3Add(t0, SDIR);
						V3StoreU(t0, *tangent0);

						PxVec3* tangent1 = (PxVec3*)tangentCache.GetMemoryAddress(&destMesh.pTangent4[i1]);
						Vec3V t1 = V3LoadU(*tangent1);
						t1 = V3Add(t1, SDIR);
						V3StoreU(t1, *tangent1);

						PxVec3* tangent2 = (PxVec3*)tangentCache.GetMemoryAddress(&destMesh.pTangent4[i2]);
						Vec3V t2 = V3LoadU(*tangent2);
						t2 = V3Add(t2, SDIR);
						V3StoreU(t2, *tangent2);
#endif
					}
				}
			}

			PxU32 tags[] = {3, 4, 5};
			memory::StreamReaderWriter<sizeof(PxVec4) * 1024> tangentStreamer(tags, destMesh.pTangent4, destMesh.pTangent4 + numVertices, "computeTangentSpaceUpdate::tangentStreamer");

			const PxU32 numIterations = (numVertices + 1023) / 1024;
			for (PxU32 a = 0; a < numIterations; ++a)
			{
				const PxU32 numToProcess = PxMin(numVertices - a * 1024u, 1024u);
				PxVec4* tangents = (PxVec4*)tangentStreamer.GetLocalAddress((void*)(destMesh.pTangent4 + (a * 1024)), sizeof(PxVec4) * numToProcess);

				PxU32 tangentW = 0;

				for (PxU32 j = 0; j < numToProcess; ++j)
				{
					if ((j & 0x1f) == 0)
					{
						tangentW = compressedTangentW[(a * 1024 + j) >> 5];
					}

					tangents[j].w = (tangentW & 0x1) ? 1.0f : -1.0f;
					tangentW >>= 1;
				}
			}
		}
	}
}



template <bool withNormals, bool withTangents>
void ClothingActorData::computeTangentSpaceUpdateSimple(NxAbstractMeshDescription& destMesh,
        const ClothingMeshAssetData& rendermesh, PxU32 submeshIndex, const PxU32* compressedTangentW)
{
	//__builtin_snpause();
	using namespace physx::shdfnd::aos;
	ClothingAssetSubMesh* pSubMesh = mAsset.GetSubmesh(&rendermesh, submeshIndex);

	if (withNormals && withTangents)
	{
		computeTangentSpaceUpdateSimple<true, false>(destMesh, rendermesh, submeshIndex, compressedTangentW);
		computeTangentSpaceUpdateSimple<false, true>(destMesh, rendermesh, submeshIndex, compressedTangentW);
	}
	else
	{
		const NxRenderDataFormat::Enum uvFormat = pSubMesh->mUvFormat;

		if (uvFormat != NxRenderDataFormat::FLOAT2)
		{
			if (withNormals)
			{
				computeTangentSpaceUpdateSimple<true, false>(destMesh, rendermesh, submeshIndex, compressedTangentW);
			}

			return;
		}

		PX_ASSERT(pSubMesh->mCurrentMaxIndexSimulation <= pSubMesh->mIndicesCount);
		const PxU32 numGraphicalVertexIndices =	pSubMesh->mCurrentMaxIndexSimulation;
		const PxU32* indices =					pSubMesh->mIndices;

		const NxVertexUVLocal* PX_RESTRICT uvs = pSubMesh->mUvs;
		PX_ASSERT(uvs != NULL);

		const PxU32 numVertices = pSubMesh->mCurrentMaxVertexAdditionalSimulation;
		const PxU32 numZeroVertices = pSubMesh->mCurrentMaxVertexSimulation;
		PX_ASSERT(numVertices <= destMesh.numVertices);

		PX_ASSERT(pSubMesh->mVertexCount == destMesh.numVertices);
		PX_ASSERT(destMesh.pPosition != NULL);
		PX_ASSERT(destMesh.pNormal != NULL);
		PX_ASSERT(destMesh.pTangent4 != NULL || !withTangents);
		PX_ASSERT(destMesh.pTangent == NULL);
		PX_ASSERT(destMesh.pBitangent == NULL);

		const FloatV vZero = FZero();

		//All indices read in in blocks of 3...hence need to fetch in an exact multiple of 3...

		const PxU32 UnrollSize = 192; //exactly divisible by 16 AND 3 :-)

		const PxU32 numIterations = (numGraphicalVertexIndices + UnrollSize - 1) / UnrollSize;

		memory::MemBaseMarker outerMarker("computeTangentSpaceUpdateSimple");
		const PxVec3* PX_RESTRICT destPositions = (const PxVec3 * PX_RESTRICT)memory::LargeFetchAllocUnaligned(destMesh.pPosition, numVertices * sizeof(PxVec3));

		if (withNormals)
		{
			memory::MemBaseMarker outerMarker("computeTangentSpaceUpdateSimple::normals");
			//__builtin_snpause();
#ifdef __SPU__
			PxVec3* PX_RESTRICT destNormals = (PxVec3 * PX_RESTRICT)memory::SpuAllocMatchAlignment(sizeof(PxVec3) * numVertices, destMesh.pNormal);
			memory::cellDmaGetUnaligned(destNormals + numZeroVertices, (uint32_t)(destMesh.pNormal + numZeroVertices), (numVertices - numZeroVertices) * sizeof(PxVec4), 11);
			memory::SpuWaitDma(1 << 11);

#else
			PxVec3* PX_RESTRICT destNormals = destMesh.pNormal;
#endif
			for (PxU32 a = 0; a < numZeroVertices; ++a)
			{
				destNormals[a] = PxVec3(0.0f);
			}

			PxU32 tags[2] = {9, 10};
			memory::Prefetcher<2, sizeof(PxU32) * UnrollSize> indicesPrefetcher(tags,
			        (void*)indices,
			        (void*)(indices + numGraphicalVertexIndices),
			        "computeTangentSpaceUpdateSimple::indicesPrefetcher");
			for (PxU32 a = 0; a < numIterations; ++a)
			{
				//__builtin_snpause();
				const PxU32 numToProcess = PxMin(numGraphicalVertexIndices - (a * UnrollSize), UnrollSize);
				const PxU32* localIndices = (const PxU32*)indicesPrefetcher.GetLocalAddress((void*)(indices + (a * UnrollSize)), sizeof(PxU32) * numToProcess);

				for (PxU32 i = 0; i < numToProcess; i += 3)
				{
					const PxU32 i0 = localIndices[i + 0];
					const PxU32 i1 = localIndices[i + 1];
					const PxU32 i2 = localIndices[i + 2];

					const Vec3V P0 = V3LoadU(destPositions[i0]);
					const Vec3V P1 = V3LoadU(destPositions[i1]);
					const Vec3V P2 = V3LoadU(destPositions[i2]);

					const Vec3V X1 = V3Sub(P1, P0);
					const Vec3V X2 = V3Sub(P2, P0);

					const Vec3V FACENORMAL = V3Cross(X1, X2);

					PxVec3* PX_RESTRICT nor1 = &destNormals[i0];
					Vec3V n1 = V3LoadU(*nor1);
					n1 = V3Add(n1, FACENORMAL);
					V3StoreU(n1, *nor1);

					PxVec3* PX_RESTRICT nor2 = &destNormals[i1];
					Vec3V n2 = V3LoadU(*nor2);
					n2 = V3Add(n2, FACENORMAL);
					V3StoreU(n2, *nor2);

					PxVec3* PX_RESTRICT nor3 = &destNormals[i2];
					Vec3V n3 = V3LoadU(*nor3);
					n3 = V3Add(n3, FACENORMAL);
					V3StoreU(n3, *nor3);

				}
			}

#ifdef __SPU__
			memory::SpuLargeStoreUnaligned(destMesh.pNormal, destNormals, sizeof(PxVec3) * numVertices);
#endif
		}
		if (withTangents)
		{
			memory::MemBaseMarker outerMarker("computeTangentSpaceUpdateSimple::tangents");

			const NxVertexUVLocal* PX_RESTRICT uvLocal = (const NxVertexUVLocal * PX_RESTRICT)memory::LargeFetchAllocUnaligned((void*)uvs, sizeof(NxVertexUVLocal) * numVertices);
			PxU32 tags[2] = {9, 10};
			memory::Prefetcher<2, sizeof(PxU32) * UnrollSize> indicesPrefetcher(tags,
			        (void*)indices,
			        (void*)(indices + numGraphicalVertexIndices),
			        "computeTangentSpaceUpdateSimple::indicesPrefetcher");

#ifdef __SPU__
			PxVec4* PX_RESTRICT tangents = (PxVec4 * PX_RESTRICT)memory::SpuAllocMatchAlignment(sizeof(PxVec4) * numVertices, destMesh.pTangent4);
			memory::cellDmaGetUnaligned(tangents + numZeroVertices, (uint32_t)(destMesh.pTangent4 + numZeroVertices), (numVertices - numZeroVertices) * sizeof(PxVec4), 11);
			memory::SpuWaitDma(1 << 11);
#else
			PxVec4* PX_RESTRICT tangents = destMesh.pTangent4;
#endif

			for (PxU32 a = 0; a < numZeroVertices; ++a)
			{
				tangents[a] = PxVec4(0.f);
			}


			for (PxU32 a = 0; a < numIterations; ++a)
			{
				//__builtin_snpause();
				const PxU32 numToProcess = PxMin(numGraphicalVertexIndices - (a * UnrollSize), UnrollSize);
				const PxU32* localIndices = (const PxU32*)indicesPrefetcher.GetLocalAddress((void*)(indices + (a * UnrollSize)), sizeof(PxU32) * numToProcess);

				for (PxU32 i = 0; i < numToProcess; i += 3)
				{
					const PxU32 i0 = localIndices[i + 0];
					const PxU32 i1 = localIndices[i + 1];
					const PxU32 i2 = localIndices[i + 2];

					const Vec3V P0 = V3LoadU(destPositions[i0]);
					const Vec3V P1 = V3LoadU(destPositions[i1]);
					const Vec3V P2 = V3LoadU(destPositions[i2]);

					const Vec3V X1 = V3Sub(P1, P0);
					const Vec3V X2 = V3Sub(P2, P0);

					const NxVertexUVLocal& w0 = uvLocal[i0];
					const NxVertexUVLocal& w1 = uvLocal[i1];
					const NxVertexUVLocal& w2 = uvLocal[i2];

					const FloatV W0U = FLoad(w0.u);
					const FloatV W1U = FLoad(w1.u);
					const FloatV W2U = FLoad(w2.u);
					const FloatV W0V = FLoad(w0.v);
					const FloatV W1V = FLoad(w1.v);
					const FloatV W2V = FLoad(w2.v);

					//This could be just 1 sub...
					const FloatV S1 = FSub(W1U, W0U);
					const FloatV S2 = FSub(W2U, W0U);
					const FloatV T1 = FSub(W1V, W0V);
					const FloatV T2 = FSub(W2V, W0V);

					// invH = (s1 * t2 - s2 * t1);
					const FloatV S1T2 = FMul(S1, T2);
					const FloatV invHR = FNegMulSub(S2, T1, S1T2);
					const FloatV HR = FRecipFast(invHR);
					const Vec3V T2X1 = V3Scale(X1, T2);
					//const Vec3V S1X2 = V3Scale(X2, S1);
					const BoolV invHREqZero = FIsEq(invHR, vZero);

					const Vec3V T1X2MT2X1 = V3NegScaleSub(X2, T1, T2X1);
					//const Vec3V S2X1MS1X2 = V3NegScaleSub(X1, S2, S1X2);

					const FloatV scale = FSel(invHREqZero, vZero, HR);

					const Vec4V SDIR = Vec4V_From_Vec3V(V3Scale(T1X2MT2X1, scale)); // .w gets overwritten later on
					//const Vec3V TDIR = V3Scale(S2X1MS1X2, scale);

					PxVec4* PX_RESTRICT tangent0 = tangents + i0;
					PxVec4* PX_RESTRICT tangent1 = tangents + i1;
					PxVec4* PX_RESTRICT tangent2 = tangents + i2;
					Vec4V t0 = V4LoadA((PxF32*)tangent0);
					Vec4V t1 = V4LoadA((PxF32*)tangent1);
					Vec4V t2 = V4LoadA((PxF32*)tangent2);

					t0 = V4Add(t0, SDIR);
					t1 = V4Add(t1, SDIR);
					t2 = V4Add(t2, SDIR);

					V4StoreA(t0, (PxF32*)tangent0);
					V4StoreA(t1, (PxF32*)tangent1);
					V4StoreA(t2, (PxF32*)tangent2);
				}
			}

			PxU32 tangentW = 0;

			PxI32 j = 0;
#if 1
			// This makes it quite a bit faster, but it also works without it.
			for (; j < (PxI32)numVertices - 4; j += 4)
			{
				if ((j & 0x1f) == 0)
				{
					tangentW = compressedTangentW[j >> 5];
				}

				tangents[j].w = (tangentW & 0x1) ? 1.0f : -1.0f;
				tangents[j + 1].w = (tangentW & 0x2) ? 1.0f : -1.0f;
				tangents[j + 2].w = (tangentW & 0x4) ? 1.0f : -1.0f;
				tangents[j + 3].w = (tangentW & 0x8) ? 1.0f : -1.0f;
				tangentW >>= 4;
			}
#endif

			// We need this loop to handle last vertices in tangents[], it shares the same j as previous loop
			for (; j < (PxI32)numVertices; j++)
			{
				if ((j & 0x1f) == 0)
				{
					tangentW = compressedTangentW[j >> 5];
				}

				tangents[j].w = (tangentW & 0x1) ? 1.0f : -1.0f;
				tangentW >>= 1;
			}

#ifdef __SPU__
			memory::SpuLargeStoreUnaligned(destMesh.pTangent4, tangents, sizeof(PxVec4) * numVertices);
#endif
		}
	}
}


PxBounds3 ClothingActorData::getRenderMeshAssetBoundsTransformed()
{
	PxBounds3 newBounds = mAsset.GetLod(mCurrentGraphicalLodId)->mBounds;

	PxMat44 transformation;
	if (mInternalBoneMatricesCur != NULL)
	{
		transformation = mInternalBoneMatricesCur[mAsset.mRootBoneIndex];
	}
	else
	{
		//transformation = mActorDesc->globalPose;
		transformation = mGlobalPose;
	}

	if (!newBounds.isEmpty())
	{
		PxVec3 center = transformation.transform(newBounds.getCenter());
		PxVec3 extent = newBounds.getExtents();

		// extended basis vectors
		PxVec3 c0 = transformation.column0.getXYZ() * extent.x;
		PxVec3 c1 = transformation.column1.getXYZ() * extent.y;
		PxVec3 c2 = transformation.column2.getXYZ() * extent.z;

		// find combination of base vectors that produces max. distance for each component = sum of physx::PxAbs()
		extent.x = PxAbs(c0.x) + PxAbs(c1.x) + PxAbs(c2.x);
		extent.y = PxAbs(c0.y) + PxAbs(c1.y) + PxAbs(c2.y);
		extent.z = PxAbs(c0.z) + PxAbs(c1.z) + PxAbs(c2.z);

		return PxBounds3::centerExtents(center, extent);
	}
	else
	{
		return newBounds;
	}
}


void ClothingActorData::tickSynchAfterFetchResults_LocksPhysX()
{
	if (bIsInitialized && !bIsClothingSimulationNull && bShouldComputeRenderData /*&& !bInternalFrozen*/)
	{
		// overwrite a few writeback normals!

		if (bCorrectSimulationNormals)
		{
			skinPhysicsMaxDist0Normals_NoPhysx();
		}

		//// perform mesh-to-mesh skinning if using skin cloth

		if (!bParallelCpuSkinning)
		{
			skinToAnimation_NoPhysX(true);
		}

		skinToPhysicalMesh_NoPhysX(true);

		finalizeSkinning_NoPhysX(true);

		PX_ASSERT(!mNewBounds.isEmpty());
		PX_ASSERT(mNewBounds.isFinite());
	}
}

void ClothingActorData::tickSynchAfterFetchResults_LocksPhysXSimple()
{
	if (bIsInitialized && !bIsClothingSimulationNull && bShouldComputeRenderData /*&& !bInternalFrozen*/)
	{
		// overwrite a few writeback normals!

		if (bCorrectSimulationNormals)
		{
			skinPhysicsMaxDist0Normals_NoPhysx();
		}

		//// perform mesh-to-mesh skinning if using skin cloth

		if (!bParallelCpuSkinning)
		{
			skinToAnimation_NoPhysX(true);
		}

		skinToPhysicalMesh_NoPhysX(true);

		finalizeSkinningSimple_NoPhysX(true);

		PX_ASSERT(!mNewBounds.isEmpty());
		PX_ASSERT(mNewBounds.isFinite());
	}
}


bool ClothingActorData::calcIfSimplePhysicsMesh() const
{
	// this number is the blocksize in SPU_ClothSkinPhysicsSimple.spu.cpp
	return skinPhysicsSimpleMem() < BLOCK_SIZE_SKIN_PHYSICS;

	// with
	// BLOCK_SIZE_SKIN_PHYSICS (32768*6)
	// 100 bones
	// 4 bone indices per vertex
	// => simple mesh is vertexCount < 3336
}



bool ClothingActorData::getSkinOnSpu() const
{
	PX_ASSERT(bSkinPhysicsMeshSpuValid);
	return bSkinPhysicsMeshSpu && bSkinPhysicsMeshSpuValid;
}


PxU32 ClothingActorData::skinPhysicsSimpleMem() const
{
	PX_ASSERT(bIsInitialized);

	const ClothingPhysicalMeshData* physicalMesh = mAsset.GetPhysicalMeshFromLod(mCurrentGraphicalLodId);
	ClothingPhysicalMeshSubmeshData* physicalSubmesh = mAsset.GetPhysicalSubmesh(physicalMesh, mCurrentPhysicsSubmesh);

	PX_ASSERT(physicalMesh != NULL);
	PX_ASSERT(physicalSubmesh != NULL);

	const PxU32 numVertices = physicalSubmesh->mVertexCount;
	const PxU32 numBoneIndicesPerVertex = physicalMesh->mNumBonesPerVertex;

	PxU32 srcPositionMem = numVertices * sizeof(PxVec3);
	PxU32 srcNormalMem = numVertices * sizeof(PxVec3);

	PxU32 simBoneIndicesMem = numBoneIndicesPerVertex * numVertices * sizeof(PxU16);
	PxU32 simBoneWeightsMem = numBoneIndicesPerVertex * numVertices * sizeof(PxF32);

	PxU32 matricesMem = mInternalMatricesCount * sizeof(PxMat44);

	PxU32 optimizationDataMem = physicalMesh->mOptimizationDataCount * sizeof(PxU8); // mOptimizationDataCount ~ numVertices

	PxU32 mem = srcPositionMem + srcNormalMem + simBoneIndicesMem + simBoneWeightsMem + matricesMem + optimizationDataMem;
	// numVertices * (33 + (6*numBonesPerVert)) + 64*numBones

	return mem;
}


void ClothingActorData::skinPhysicsMeshSimple()
{
	if (!bIsInitialized)
	{
		return;
	}

	PX_ASSERT(getSkinOnSpu());

	// with bones, no interpolated matrices, no backstop?

	// data
	const ClothingPhysicalMeshData* physicalMesh = mAsset.GetPhysicalMeshFromLod(mCurrentGraphicalLodId);
	ClothingPhysicalMeshSubmeshData* physicalSubmesh = mAsset.GetPhysicalSubmesh(physicalMesh, mCurrentPhysicsSubmesh);
	PX_ASSERT(physicalMesh != NULL);
	PX_ASSERT(physicalSubmesh != NULL);

	const PxU32 numVertices = physicalSubmesh->mVertexCount;
	const PxU32 numBoneIndicesPerVertex = physicalMesh->mNumBonesPerVertex;

	PxVec3* const PX_RESTRICT eaPositions = physicalMesh->mVertices;
	PxVec3* const PX_RESTRICT positions = (PxVec3*)physx::apex::memory::LargeFetchAllocUnaligned(eaPositions, numVertices * sizeof(PxVec3));

	PxVec3* const PX_RESTRICT eaNormals = physicalMesh->mNormals;
	PxVec3* const PX_RESTRICT normals = (PxVec3*)physx::apex::memory::LargeFetchAllocUnaligned(eaNormals, numVertices * sizeof(PxVec3));

#ifdef __SPU__
	//PxVec3* const PX_RESTRICT targetPositions = (PxVec3*)physx::apex::memory::SpuAllocMatchAlignment(numVertices*sizeof(PxVec3), skinnedPhysicsPositions); //skinnedPhysicsPositions;
	//PxVec3* const PX_RESTRICT targetNormals = (PxVec3*)physx::apex::memory::SpuAllocMatchAlignment(numVertices*sizeof(PxVec3), skinnedPhysicsNormals); // skinnedPhysicsNormals;

	// both eaPositions and skinnedPhysicsPositions have a 16 byte alignment (same holds for eaNormals and skinnedPhysicsNormals)
	// by reusing the positions and normals arrays we can save spu mem
	PxVec3* const PX_RESTRICT targetPositions = positions;
	PxVec3* const PX_RESTRICT targetNormals = normals;
#else
	PxVec3* const PX_RESTRICT targetPositions = mSkinnedPhysicsPositions;
	PxVec3* const PX_RESTRICT targetNormals = mSkinnedPhysicsNormals;
#endif

	PxU16* const PX_RESTRICT eaSimBoneIndices = physicalMesh->mBoneIndices;
	const PxU16* const PX_RESTRICT simBoneIndices = (PxU16*)physx::apex::memory::LargeFetchAllocUnaligned(eaSimBoneIndices, numBoneIndicesPerVertex * numVertices * sizeof(PxU16));

	PxF32* const PX_RESTRICT eaSimBoneWeights = physicalMesh->mBoneWeights;
	const PxF32* const PX_RESTRICT simBoneWeights = (PxF32*)physx::apex::memory::LargeFetchAllocUnaligned(eaSimBoneWeights, numBoneIndicesPerVertex * numVertices * sizeof(PxF32));

	PxMat44* eaMatrices = mInternalBoneMatricesCur; // TODO interpolated matrices?
	const PxMat44* matrices = (PxMat44*)physx::apex::memory::FetchAlloc(eaMatrices, mInternalMatricesCount * sizeof(PxMat44));

	PxU8* const PX_RESTRICT eaOptimizationData = physicalMesh->mOptimizationData;
	const PxU8* const PX_RESTRICT optimizationData = (PxU8*)physx::apex::memory::LargeFetchAllocUnaligned(eaOptimizationData, physicalMesh->mOptimizationDataCount * sizeof(PxU8));

	PX_ASSERT(optimizationData != NULL);

	for (PxU32 vertexIndex = 0; vertexIndex < numVertices; ++vertexIndex)
	{
		Vec3V positionV = V3Zero();
		Vec3V normalV = V3Zero();

		const PxU8 shift = 4 * (vertexIndex % 2);
		const PxU8 numBones = PxU8((optimizationData[vertexIndex / 2] >> shift) & 0x7);
		for (PxU32 k = 0; k < numBones; k++)
		{
			const PxF32 weight = simBoneWeights[vertexIndex * numBoneIndicesPerVertex + k];

			PX_ASSERT(weight <= 1.0f);

			//sumWeights += weight;
			FloatV weightV = FLoad(weight);

			const PxU32 index = simBoneIndices[vertexIndex * numBoneIndicesPerVertex + k];
			PX_ASSERT(index < mInternalMatricesCount);

			/// PH: This might be faster without the reference, but on PC I can't tell
			/// HL: Now with SIMD it's significantly faster as reference
			const Mat34V& bone = (Mat34V&)matrices[index];

			Vec3V pV = M34MulV3(bone, V3LoadU(positions[vertexIndex]));
			pV = V3Scale(pV, weightV);
			positionV = V3Add(positionV, pV);

			///todo There are probably cases where we don't need the normal on the physics mesh
			Vec3V nV = M34Mul33V3(bone, V3LoadU(normals[vertexIndex]));
			nV = V3Scale(nV, weightV);
			normalV = V3Add(normalV, nV);
		}

		normalV = V3NormalizeFast(normalV);
		V3StoreU(normalV, targetNormals[vertexIndex]);
		V3StoreU(positionV, targetPositions[vertexIndex]);
	}

#ifdef __SPU__
	physx::apex::memory::SpuLargeStoreUnaligned(mSkinnedPhysicsPositions, targetPositions, numVertices * sizeof(PxVec3));
	physx::apex::memory::SpuLargeStoreUnaligned(mSkinnedPhysicsNormals, targetNormals, numVertices * sizeof(PxVec3));
#endif
}


}
} // namespace apex
} // namespace physx
