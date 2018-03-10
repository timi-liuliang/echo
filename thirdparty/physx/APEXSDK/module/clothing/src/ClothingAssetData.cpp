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
#include "ClothingAssetData.h"
#include "PsUtilities.h"
#include "PsVecMath.h"
#include "foundation/PxMat44.h"

#define NX_PARAMETERIZED_ONLY_LAYOUTS
#include "ClothingGraphicalLodParameters.h"

#pragma warning(disable : 4101 4127) // unreferenced local variable and conditional expression is constant


namespace physx
{
using namespace shdfnd;
using namespace shdfnd::aos;
namespace apex
{
namespace clothing
{


ClothingAssetSubMesh::ClothingAssetSubMesh() :
	mPositions(NULL),
	mNormals(NULL),
	mTangents(NULL),
	mBoneWeights(NULL),
	mBoneIndices(NULL),
	mIndices(NULL),
	mUvs(NULL),

	mPositionOutFormat(NxRenderDataFormat::UNSPECIFIED),
	mNormalOutFormat(NxRenderDataFormat::UNSPECIFIED),
	mTangentOutFormat(NxRenderDataFormat::UNSPECIFIED),
	mBoneWeightOutFormat(NxRenderDataFormat::UNSPECIFIED),
	mUvFormat(NxRenderDataFormat::UNSPECIFIED),

	mVertexCount(0),
	mIndicesCount(0),
	mUvCount(0),
	mNumBonesPerVertex(0),

	mCurrentMaxVertexSimulation(0),
	mCurrentMaxVertexAdditionalSimulation(0),
	mCurrentMaxIndexSimulation(0)
{
}



ClothingMeshAssetData::ClothingMeshAssetData() :
	mImmediateClothMap(NULL),
	mSkinClothMap(NULL),
	mSkinClothMapB(NULL),
	mTetraMap(NULL),

	mImmediateClothMapCount(0),
	mSkinClothMapCount(0),
	mSkinClothMapBCount(0),
	mTetraMapCount(0),

	mSubmeshOffset(0),
	mSubMeshCount(0),

	mPhysicalMeshId(0),

	mSkinClothMapThickness(0.0f),
	mSkinClothMapOffset(0.0f),

	mBounds(PxBounds3::empty()),

	bActive(false)
{

}



ClothingPhysicalMeshData::ClothingPhysicalMeshData() :
	mVertices(NULL),
	mNormals(NULL),
	mSkinningNormals(NULL),
	mBoneIndices(NULL),
	mBoneWeights(NULL),
	mOptimizationData(NULL),
	mIndices(NULL),

	mSkinningNormalsCount(0),
	mBoneWeightsCount(0),
	mOptimizationDataCount(0),
	mIndicesCount(0),

	mSubmeshOffset(0),
	mSubmeshesCount(0),

	mNumBonesPerVertex(0)
{

}



ClothingAssetData::ClothingAssetData() :
	mData(NULL),
	mCompressedNumBonesPerVertex(NULL),
	mCompressedTangentW(NULL),
	mExt2IntMorphMapping(NULL),
	mCompressedNumBonesPerVertexCount(0),
	mCompressedTangentWCount(0),
	mExt2IntMorphMappingCount(0),

	mAssetSize(0),
	mGraphicalLodsCount(0),
	mPhysicalMeshesCount(0),
	mPhysicalMeshOffset(0),
	mBoneCount(0),
	mRootBoneIndex(0)
{

}




ClothingAssetData::~ClothingAssetData()
{
#ifndef __SPU__
	PX_FREE(mData);
	mData = NULL;
#endif
}



void ClothingAssetData::skinToBones(NxAbstractMeshDescription& destMesh, PxU32 submeshIndex, PxU32 graphicalMeshIndex, PxU32 startVertex,
                                    PxMat44* compositeMatrices, PxVec3* morphDisplacements, memory::MorphTargetCache& cache)
{
	// This is no nice code, but it allows to have 8 different code paths through skinToBones that have all the if's figured out at compile time (hopefully)
	PX_ASSERT(destMesh.pTangent == NULL);
	PX_ASSERT(destMesh.pBitangent == NULL);

	if (destMesh.pNormal != NULL)
	{
		if (mBoneCount != 0)
		{
			if (morphDisplacements != NULL)
			{
				if (destMesh.pTangent4 != NULL)
				{
					skinToBonesInternal<true, true, true, true>(destMesh, submeshIndex, graphicalMeshIndex, startVertex, compositeMatrices, morphDisplacements, cache);
				}
				else
				{
					skinToBonesInternal<true, true, true, false>(destMesh, submeshIndex, graphicalMeshIndex, startVertex, compositeMatrices, morphDisplacements, cache);
				}
			}
			else
			{
				if (destMesh.pTangent4 != NULL)
				{
					skinToBonesInternal<true, true, false, true>(destMesh, submeshIndex, graphicalMeshIndex, startVertex, compositeMatrices, morphDisplacements, cache);
				}
				else
				{
					skinToBonesInternal<true, true, false, false>(destMesh, submeshIndex, graphicalMeshIndex, startVertex, compositeMatrices, morphDisplacements, cache);
				}
			}
		}
		else
		{
#ifndef __SPU__
			if (morphDisplacements != NULL)
			{
				if (destMesh.pTangent4 != NULL)
				{
					skinToBonesInternal<true, false, true, true>(destMesh, submeshIndex, graphicalMeshIndex, startVertex, compositeMatrices, morphDisplacements, cache);
				}
				else
				{
					skinToBonesInternal<true, false, true, false>(destMesh, submeshIndex, graphicalMeshIndex, startVertex, compositeMatrices, morphDisplacements, cache);
				}
			}
			else
			{
				if (destMesh.pTangent4 != NULL)
				{
					skinToBonesInternal<true, false, false, true>(destMesh, submeshIndex, graphicalMeshIndex, startVertex, compositeMatrices, morphDisplacements, cache);
				}
				else
				{
					skinToBonesInternal<true, false, false, false>(destMesh, submeshIndex, graphicalMeshIndex, startVertex, compositeMatrices, morphDisplacements, cache);
				}
			}
#endif
		}
	}
	else
	{
		if (mBoneCount != 0)
		{
			if (morphDisplacements != NULL)
			{
				if (destMesh.pTangent4 != NULL)
				{
					skinToBonesInternal<false, true, true, true>(destMesh, submeshIndex, graphicalMeshIndex, startVertex, compositeMatrices, morphDisplacements, cache);
				}
				else
				{
					skinToBonesInternal<false, true, true, false>(destMesh, submeshIndex, graphicalMeshIndex, startVertex, compositeMatrices, morphDisplacements, cache);
				}
			}
			else
			{
				if (destMesh.pTangent4 != NULL)
				{
					skinToBonesInternal<false, true, false, true>(destMesh, submeshIndex, graphicalMeshIndex, startVertex, compositeMatrices, morphDisplacements, cache);
				}
				else
				{
					skinToBonesInternal<false, true, false, false>(destMesh, submeshIndex, graphicalMeshIndex, startVertex, compositeMatrices, morphDisplacements, cache);
				}
			}
		}
		else
		{
#ifndef __SPU__
			if (morphDisplacements != NULL)
			{
				if (destMesh.pTangent4 != NULL)
				{
					skinToBonesInternal<false, false, true, true>(destMesh, submeshIndex, graphicalMeshIndex, startVertex, compositeMatrices, morphDisplacements, cache);
				}
				else
				{
					skinToBonesInternal<false, false, true, false>(destMesh, submeshIndex, graphicalMeshIndex, startVertex, compositeMatrices, morphDisplacements, cache);
				}
			}
			else
			{
				if (destMesh.pTangent4 != NULL)
				{
					skinToBonesInternal<false, false, false, true>(destMesh, submeshIndex, graphicalMeshIndex, startVertex, compositeMatrices, morphDisplacements, cache);
				}
				else
				{
					skinToBonesInternal<false, false, false, false>(destMesh, submeshIndex, graphicalMeshIndex, startVertex, compositeMatrices, morphDisplacements, cache);
				}
			}
#endif
		}
	}
}


template<bool withNormals, bool withBones, bool withMorph, bool withTangents>
void ClothingAssetData::skinToBonesInternal(NxAbstractMeshDescription& destMesh, PxU32 submeshIndex, PxU32 graphicalMeshIndex,
        PxU32 startVertex, PxMat44* compositeMatrices, PxVec3* morphDisplacements, memory::MorphTargetCache& cache)
{
	memory::MemBaseMarker outerMarker("skinToBonesInternal");

	PX_ASSERT(withNormals == (destMesh.pNormal != NULL));
	PX_ASSERT(withMorph == (morphDisplacements != NULL));

	PX_ASSERT(withTangents == (destMesh.pTangent4 != NULL));
	PX_ASSERT(destMesh.pTangent == NULL);
	PX_ASSERT(destMesh.pBitangent == NULL);

	// so we need to start further behind, but to make vec3* still aligned it must be a multiple of 4, and to make
	// compressedNumBones aligned, it needs to be a multiple of 16 (hence 16)
	// The start offset is reduced by up to 16, and instead of skipping the first [0, 16) vertices and adding additional logic
	// they just get computed as well, assuming much less overhead than an additional if in the innermost loop
	// Note: This only works if the mesh-mesh skinning is executed afterwards!
	const PxU32 startVertex16 = startVertex - (startVertex % 16);

	PX_ASSERT(((size_t)(compositeMatrices) & 0xf) == 0); // make sure the pointer is 16 byte aligned!!

	ClothingMeshAssetData* meshAsset = GetLod(graphicalMeshIndex);
	PX_ASSERT(submeshIndex < meshAsset->mSubMeshCount);
	ClothingAssetSubMesh* pSubmesh = GetSubmesh(meshAsset, submeshIndex);
	const PxU32 numVertices = pSubmesh->mVertexCount - startVertex16;
	if(numVertices == 0)
	{
		return;
	}

	// use the per-asset setting
	const PxU32 startVertexDiv16 = startVertex16 / 16;
	PxU32 mapSize = 0;
	const PxU32* compressedNumBonesPerVertexEa = getCompressedNumBonesPerVertex(graphicalMeshIndex, submeshIndex, mapSize);
	PX_ASSERT((mapSize - startVertexDiv16) * 16 >= numVertices);

	const PxU32* compressedNumBonesPerVertex = (const PxU32*)memory::FetchAllocUnaligned((PxU32*)compressedNumBonesPerVertexEa + startVertexDiv16, sizeof(PxU32) * (mapSize - startVertexDiv16));

	const PxVec3* PX_RESTRICT positionsEa = pSubmesh->mPositions + startVertex16;
	PX_ASSERT(pSubmesh->mPositionOutFormat == NxRenderDataFormat::FLOAT3);
	const PxVec3* PX_RESTRICT normalsEa = pSubmesh->mNormals + startVertex16;
	PX_ASSERT(pSubmesh->mNormalOutFormat == NxRenderDataFormat::FLOAT3);

	const PxVec4* PX_RESTRICT tangentsEa = pSubmesh->mTangents + (pSubmesh->mTangents != NULL ? startVertex16 : 0);
	PX_ASSERT(tangentsEa == NULL || pSubmesh->mTangentOutFormat == NxRenderDataFormat::FLOAT4);

	//These are effective addresses (ea)
	PxVec3* PX_RESTRICT destPositionsEa = destMesh.pPosition + startVertex16;
	PxVec3* PX_RESTRICT destNormalsEa = destMesh.pNormal + startVertex16;
	PxVec4* PX_RESTRICT destTangentsEa = (destMesh.pTangent4 != NULL) ? destMesh.pTangent4 + startVertex16 : NULL;

	PxU32* morphReorderingEa = morphDisplacements != NULL ? getMorphMapping(graphicalMeshIndex) : NULL;
	if (morphReorderingEa != NULL)
	{
		morphReorderingEa += startVertex16;
	}
	else if (withMorph)
	{
		// actually we don't have a morph map, so let's take one step back
		skinToBonesInternal<withNormals, withBones, false, withTangents>(destMesh, submeshIndex, graphicalMeshIndex,
			startVertex, compositeMatrices, NULL, cache);
		return;
	}
	PX_ASSERT((morphDisplacements != NULL) == (morphReorderingEa != NULL));

	const PxU16* PX_RESTRICT boneIndicesEa = NULL;
	const PxF32* PX_RESTRICT boneWeightsEa = NULL;

	const PxU32 maxNumBonesPerVertex = pSubmesh->mNumBonesPerVertex;
	if (withBones)
	{
		boneIndicesEa = (const PxU16*)pSubmesh->mBoneIndices + (startVertex16 * maxNumBonesPerVertex);
		boneWeightsEa = (const PxF32*)pSubmesh->mBoneWeights + (startVertex16 * maxNumBonesPerVertex);
	}


	//bones per vertex is LS address!!!!
	const PxU32* PX_RESTRICT bonesPerVertex = compressedNumBonesPerVertex;

	//OK. Need to split this up into an even larger subset of work!!!! Perhaps sets of 1024 vertices/normals/bones....

	const PxU32 WorkSize = 160;

	const PxU32 countWork = (numVertices + (WorkSize - 1)) / WorkSize;

	// PH: There is only 32 of these, and now they are all used  up
	PxU32 tags1[] = {6, 7};
	PxU32 tags2[] = {8, 9};
	PxU32 tags3[] = {10, 11};
	//PxU32 tags4[] = {12, 13};
	PxU32 tags5[] = {14, 15};
	PxU32 tags6[] = {16, 17};
	PxU32 tags7[] = {18, 19};
	PxU32 tags8[] = {20, 21, 22};
	PxU32 tags9[] = {23, 24, 25};
	PxU32 tags10[] = {26, 27, 28};
	//PxU32 tags11[] = {29, 30, 31};
	//TODO - finish prefetching here!!!

	memory::Prefetcher<2, sizeof(PxVec3) * WorkSize> positionPrefetcher(tags1, (void*)positionsEa, (void*)(positionsEa + numVertices), "skinToBonesInternal::positionPrefetcher");
	memory::Prefetcher<2, sizeof(PxVec3) * WorkSize> normalPrefetcher(tags2, (void*)normalsEa, (void*)(normalsEa + (withNormals ? numVertices : 0)), "skinToBonesInternal::normalPrefetcher");

	memory::Prefetcher<2, sizeof(PxVec4) * WorkSize> tangentPrefetcher(tags3, (void*)tangentsEa, (void*)(tangentsEa + (withTangents ? numVertices : 0)), "skinToBonesInternal::tangentPrefetcher");

	memory::Prefetcher<2, sizeof(PxU16) * WorkSize * 4> boneIndicesPrefetcher(tags5, (void*)boneIndicesEa, (void*)(boneIndicesEa + (withBones ? numVertices * maxNumBonesPerVertex : 0)), "skinToBonesInternal::boneIndicesPrefetcher");
	memory::Prefetcher<2, sizeof(PxF32) * WorkSize * 4> boneWeightsPrefetcher(tags6, (void*)boneWeightsEa, (void*)(boneWeightsEa + (withBones ? numVertices * maxNumBonesPerVertex : 0)), "skinToBonesInternal::boneWeightsPrefetcher");
	memory::Prefetcher<2, sizeof(PxU32) * WorkSize> morphPrefetcher(tags7, (void*)morphReorderingEa, (void*)(morphReorderingEa + (withMorph ? numVertices : 0)), "skinToBonesInternal::morphPrefetcher");

	memory::StreamReaderWriter<sizeof(PxVec3) * WorkSize> destPositionStreamer(tags8, (void*)destPositionsEa, (void*)(destPositionsEa + numVertices), "skinToBonesInternal::destPositionStreamer");
	memory::StreamReaderWriter<sizeof(PxVec3) * WorkSize> destNormalStreamer(tags9, (void*)destNormalsEa, (void*)(destNormalsEa + (withNormals ? numVertices : 0)), "skinToBonesInternal::destNormalStreamer");
	memory::StreamReaderWriter<sizeof(PxVec4) * WorkSize> destTangentStreamer(tags10, (void*)destTangentsEa, (void*)(destTangentsEa + (withTangents ? numVertices : 0)), "skinToBonesInternal::destTangentStreamer");

	for (PxU32 a = 0; a < countWork; ++a)
	{
		const PxU32 workCount = PxMin(numVertices - (a * WorkSize), WorkSize);

		const PxVec3* PX_RESTRICT positions = (const PxVec3 * PX_RESTRICT)positionPrefetcher.GetLocalAddress((void*)positionsEa, sizeof(PxVec3) * workCount);
		const PxVec3* PX_RESTRICT normals = NULL;
		const PxF32* PX_RESTRICT tangents4 = NULL;
		const PxU16* PX_RESTRICT boneIndices = NULL;
		const PxF32* PX_RESTRICT boneWeights = NULL;

		PxVec3* PX_RESTRICT destPositions = (PxVec3 * PX_RESTRICT)destPositionStreamer.GetLocalAddress((void*)destPositionsEa, sizeof(PxVec3) * workCount);
		PxVec3* PX_RESTRICT destNormals = NULL;
		PxF32* PX_RESTRICT destTangents4 = NULL;

		PxU32* morphReordering = NULL;
		if (withMorph)
		{
			morphReordering = (PxU32*)morphPrefetcher.GetLocalAddress(morphReorderingEa, sizeof(PxU32) * workCount);
			morphReorderingEa += workCount;
		}

		if (withBones)
		{
			boneIndices = (const PxU16 * PX_RESTRICT)boneIndicesPrefetcher.GetLocalAddress((void*)boneIndicesEa, sizeof(PxU16) * workCount * maxNumBonesPerVertex);
			boneWeights = (const PxF32 * PX_RESTRICT)boneWeightsPrefetcher.GetLocalAddress((void*)boneWeightsEa, sizeof(PxF32) * workCount * maxNumBonesPerVertex);
		}

		if (withNormals)
		{
			destNormals = (PxVec3 * PX_RESTRICT)destNormalStreamer.GetLocalAddress((void*)destNormalsEa, sizeof(PxVec3) * workCount);
			normals = (const PxVec3 * PX_RESTRICT)normalPrefetcher.GetLocalAddress((void*)normalsEa, sizeof(PxVec3) * workCount);
		}

		if (withTangents)
		{
			destTangents4 = (PxF32*)destTangentStreamer.GetLocalAddress((void*)destTangentsEa, sizeof(PxVec4) * workCount);
			tangents4 = (const PxF32*)tangentPrefetcher.GetLocalAddress((void*)tangentsEa, sizeof(PxVec4) * workCount);
		}


		const PxU32 count16 = (workCount + 15) / 16;
		for (PxU32 i = 0; i < count16; ++i)
		{
			//Fetch in the next block of stuff...
			const PxU32 maxVerts = PxMin(numVertices - (a * WorkSize) - (i * 16u), 16u);
			PxU32 numBoneWeight = *bonesPerVertex++;

#ifndef __SPU__
			::physx::shdfnd::prefetchLine(positions + 16);

			if (withNormals)
			{
				::physx::shdfnd::prefetchLine(normals + 16);
			}

			if (withBones)
			{
				::physx::shdfnd::prefetchLine(boneIndices + (4 * 16));
				::physx::shdfnd::prefetchLine(boneWeights + (2 * 16));
				::physx::shdfnd::prefetchLine(boneWeights + (4 * 16));
			}

			if (withTangents)
			{
				::physx::shdfnd::prefetchLine(tangents4 + (4 * 16));
			}
#endif

			for (PxU32 j = 0; j < maxVerts; j++)
			{
				const PxU32 vertexIndex = i * 16 + j;
				const PxU32 numBones = (numBoneWeight & 0x3) + 1;
				numBoneWeight >>= 2;

				{
					PxVec3 untransformedPosition = *positions;
					if (withMorph)
					{
						const PxVec3* morphDisplacement = (PxVec3*)cache.GetMemoryAddress(&morphDisplacements[morphReordering[vertexIndex]]);
						untransformedPosition += *morphDisplacement;
					}

					Vec3V positionV = V3Zero();
					Vec3V normalV = V3Zero();
					Vec3V tangentV = V3Zero();

					if (withBones)
					{
						for (PxU32 k = 0; k < numBones; k++)
						{
							PxF32 weight = boneWeights[k];

							PX_ASSERT(PxIsFinite(weight));

							const PxU16 boneNr = boneIndices[k];

							Mat34V& skinningMatrixV = (Mat34V&)compositeMatrices[boneNr];

							const FloatV weightV = FLoad(weight);

							Vec3V transformedPositionV = M34MulV3(skinningMatrixV, V3LoadU(untransformedPosition));
							transformedPositionV = V3Scale(transformedPositionV, weightV);
							positionV = V3Add(positionV, transformedPositionV);

							if (withNormals)
							{
								Vec3V transformedNormalV = M34Mul33V3(skinningMatrixV, V3LoadU(*normals));
								transformedNormalV = V3Scale(transformedNormalV, weightV);
								normalV = V3Add(normalV, transformedNormalV);
							}

							if (withTangents)
							{
								const Vec3V inTangent = Vec3V_From_Vec4V(V4LoadA(tangents4));
								const Vec3V transformedTangentV = M34Mul33V3(skinningMatrixV, inTangent);
								tangentV = V3Add(tangentV, V3Scale(transformedTangentV, weightV));
							}
						}
					}
					else
					{
						Mat34V& skinningMatrixV = (Mat34V&)compositeMatrices[0];
						positionV = M34MulV3(skinningMatrixV, V3LoadU(untransformedPosition));
						if (withNormals)
						{
							normalV = M34Mul33V3(skinningMatrixV, V3LoadU(*normals));
						}
						if (withTangents)
						{
							const Vec3V inTangent = Vec3V_From_Vec4V(V4LoadA(tangents4));
							tangentV = M34Mul33V3(skinningMatrixV, inTangent);
						}
					}

					if (withNormals)
					{
						normalV = V3NormalizeFast(normalV);
						V3StoreU(normalV, *destNormals);
					}


					if (withTangents)
					{
						tangentV = V3NormalizeFast(tangentV);
						const Vec4V bitangent = V4Load(tangents4[3]);
						const Vec4V outTangent = V4Sel(BTTTF(), Vec4V_From_Vec3V(tangentV), bitangent);
						V4StoreA(outTangent, destTangents4);
					}

					// if all weights are 0, we use the bind pose
					// we don't really need that and it just slows us down..
					//*destPositions = (numBones == 0) ? *positions : ps;
					//*destPositions = ps;
					V3StoreU(positionV, *destPositions);
				}

				positions++;
				positionsEa++;
				normals++;
				normalsEa++;
				if (withBones)
				{
					boneWeights += maxNumBonesPerVertex;
					boneWeightsEa += maxNumBonesPerVertex;
					boneIndices += maxNumBonesPerVertex;
					boneIndicesEa += maxNumBonesPerVertex;
				}
				if (withTangents)
				{
					tangents4 += 4;
					tangentsEa++;
					destTangents4 += 4;
					destTangentsEa++;
				}
				destPositions++;
				destPositionsEa++;
				destNormals++;
				destNormalsEa++;
			}
		}
//#ifdef __SPU__
//		SpuStoreUnaligned(destPositionsEaTarget, destPositionsLs, sizeof(PxVec3) * workCount);
//		if(withNormals)
//			SpuStoreUnaligned(destNormalsEaTarget, destNormalsLs, sizeof(PxVec3) * workCount);
//#endif
	}

}



PxU32* ClothingAssetData::getMorphMapping(PxU32 graphicalLod)
{
	if (mExt2IntMorphMappingCount == 0)
	{
		return NULL;
	}

	if (graphicalLod == (PxU32) - 1 || graphicalLod > mGraphicalLodsCount)
	{
		graphicalLod = mGraphicalLodsCount;
	}

	PxU32 offset = 0;
	for (PxU32 i = 0; i < graphicalLod; i++)
	{
		const ClothingMeshAssetData* meshAsset = GetLod(i);
		for (PxU32 s = 0; s < meshAsset->mSubMeshCount; s++)
		{
			offset += GetSubmesh(meshAsset, s)->mVertexCount;
		}
	}

	PX_ASSERT(offset < mExt2IntMorphMappingCount);
	return mExt2IntMorphMapping + offset;
}



const PxU32* ClothingAssetData::getCompressedNumBonesPerVertex(PxU32 graphicalLod, PxU32 submeshIndex, PxU32& mapSize)
{
	mapSize = 0;

	PxU32 offset = 0;
	for (PxU32 lodIndex = 0; lodIndex < mGraphicalLodsCount; lodIndex++)
	{
		const ClothingMeshAssetData* meshAsset = GetLod(lodIndex);

		for (PxU32 s = 0; s < meshAsset->mSubMeshCount; s++)
		{
			const PxU32 numVertices = GetSubmesh(meshAsset, s)->mVertexCount;

			PxU32 numEntries = (numVertices + 15) / 16;
			while ((numEntries & 0x3) != 0) // this is a numEntries % 4
			{
				numEntries++;
			}

			if (lodIndex == graphicalLod && submeshIndex == s)
			{
				mapSize = numEntries;
				PX_ASSERT((mapSize % 4) == 0);
				return &mCompressedNumBonesPerVertex[offset];
			}

			offset += numEntries;
			PX_ASSERT(mCompressedNumBonesPerVertexCount >= offset);
		}
	}
	return NULL;
}



const PxU32* ClothingAssetData::getCompressedTangentW(PxU32 graphicalLod, PxU32 submeshIndex, PxU32& mapSize)
{
	mapSize = 0;

	PxU32 offset = 0;
	for (PxU32 lodIndex = 0; lodIndex < mGraphicalLodsCount; lodIndex++)
	{
		const ClothingMeshAssetData* meshAsset = GetLod(lodIndex);

		for (PxU32 s = 0; s < meshAsset->mSubMeshCount; s++)
		{
			const PxU32 numVertices = GetSubmesh(meshAsset, s)->mVertexCount;

			PxU32 numEntries = (numVertices + 31) / 32;
			while ((numEntries & 0x3) != 0) // this is a numEntries % 4
			{
				numEntries++;
			}

			if (lodIndex == graphicalLod && submeshIndex == s)
			{
				mapSize = numEntries;
				PX_ASSERT((mapSize % 4) == 0);
				return (PxU32*)memory::FetchAlloc(&mCompressedTangentW[offset], sizeof(PxU32) * mapSize);
			}

			offset += numEntries;
			PX_ASSERT(mCompressedTangentWCount >= offset);
		}
	}
	return NULL;
}



void ClothingAssetData::getNormalsAndVerticesForFace(PxVec3* vtx, PxVec3* nrm, PxU32 i,
        const NxAbstractMeshDescription& srcPM) const
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


PxU32 ClothingAssetData::skinClothMapBSkinVertex(PxVec3& dstPos, PxVec3* dstNormal, PxU32 vIndex,
        ClothingGraphicalLodParametersNS::SkinClothMapB_Type* pTCMB, const ClothingGraphicalLodParametersNS::SkinClothMapB_Type* pTCMBEnd,
        const NxAbstractMeshDescription& srcPM) const
{
	PxU32 numIterations(0);
	//PxU32 numSkipped(0);

	//PX_ASSERT(dstPos.isZero());
	//PX_ASSERT(dstNormal == NULL || dstNormal->isZero());

	PX_ASSERT(srcPM.avgEdgeLength != 0.0f);
	const PxF32 pmThickness = srcPM.avgEdgeLength;

	while ((pTCMB->vertexIndexPlusOffset == vIndex) && (pTCMB < pTCMBEnd))
	{
		// skip vertices that we couldn't find a binding for
		if (pTCMB->faceIndex0 == 0xFFFFFFFF || pTCMB->faceIndex0 >= srcPM.numIndices)
		{
			pTCMB++;
			//numSkipped++;
			//numIterations++;
			continue;
		}

		PxVec3 vtx[3], nrm[3];
		getNormalsAndVerticesForFace(vtx, nrm, pTCMB->faceIndex0, srcPM);

		const TetraEncoding_Local& tetEnc = tetraTableLocal[pTCMB->tetraIndex];
		const PxVec3 tv0 = vtx[0] + (tetEnc.sign[0] * pmThickness) * nrm[0];
		const PxVec3 tv1 = vtx[1] + (tetEnc.sign[1] * pmThickness) * nrm[1];
		const PxVec3 tv2 = vtx[2] + (tetEnc.sign[2] * pmThickness) * nrm[2];
		const PxVec3 tv3 = vtx[tetEnc.lastVtxIdx] + (tetEnc.sign[3] * pmThickness) * nrm[tetEnc.lastVtxIdx];

		const PxVec3& vtb = pTCMB->vtxTetraBary;
		const PxF32 vtbw = 1.0f - vtb.x - vtb.y - vtb.z;
		dstPos = (vtb.x * tv0) + (vtb.y * tv1) + (vtb.z * tv2) + (vtbw * tv3);
		PX_ASSERT(dstPos.isFinite());

		if (dstNormal != NULL)
		{
			const PxVec3& ntb = pTCMB->nrmTetraBary;
			const PxF32 ntbw = 1.0f - ntb.x - ntb.y - ntb.z;
			*dstNormal = (ntb.x * tv0) + (ntb.y * tv1) + (ntb.z * tv2) + (ntbw * tv3);
			PX_ASSERT(dstNormal->isFinite());
		}

		pTCMB++;
		numIterations++;
	}

	if (dstNormal != NULL && numIterations > 0)
	{
		// PH: this code certainly does not work if numIterations is bigger than 1 (it would need to average by dividing through numIterations)
		PX_ASSERT(numIterations == 1);
		*dstNormal -= dstPos;
		//*dstNormal *= NxClothingUserRecompute::invSqrt(dstNormal->magnitudeSquared());
		*dstNormal *= physx::recipSqrtFast(dstNormal->magnitudeSquared());
	}

	return numIterations;
}


PxU32 ClothingAssetData::skinClothMapB(PxVec3* dstPositions, PxVec3* dstNormals, PxU32 numVertices,
                                       const NxAbstractMeshDescription& srcPM, ClothingGraphicalLodParametersNS::SkinClothMapB_Type* map,
                                       PxU32 numVerticesInMap, bool computeNormals) const
{
#ifdef __SPU__
	PX_UNUSED(dstPositions);
	PX_UNUSED(dstNormals);
	PX_UNUSED(numVertices);
	PX_UNUSED(srcPM);
	PX_UNUSED(map);
	PX_UNUSED(numVerticesInMap);
	PX_UNUSED(computeNormals);
	PX_ASSERT(0);
	return 0;
#else
	PX_ASSERT(srcPM.numIndices % 3 == 0);

	PX_ASSERT(srcPM.avgEdgeLength != 0.0f);

	ClothingGraphicalLodParametersNS::SkinClothMapB_Type* pTCMB = map;
	ClothingGraphicalLodParametersNS::SkinClothMapB_Type* pTCMBEnd = map + numVerticesInMap;

	for (PxU32 j = 0; j < numVerticesInMap; j++)
	{
		PxU32 vertexIndex = pTCMB->vertexIndexPlusOffset;

		if (vertexIndex >= numVertices)
		{
			pTCMB++;
			continue;
		}

		PxVec3& p = dstPositions[vertexIndex];

		PxVec3* n = NULL;
		if (computeNormals)
		{
			n = dstNormals + vertexIndex;
		}

		PxU32 numIters = skinClothMapBSkinVertex(p, n, vertexIndex, pTCMB, pTCMBEnd, srcPM);
		if (numIters == 0)
		{
			// PH: find next submesh
			const PxU32 currentIterations = (PxU32)(size_t)(pTCMB - map);
			for (PxU32 i = currentIterations; i < numVerticesInMap; i++)
			{
				if (map[i].submeshIndex > pTCMB->submeshIndex)
				{
					numIters = i - currentIterations;
					break;
				}
			}

			// only return if it's still 0
			if (numIters == 0)
			{
				return currentIterations;
			}
		}

		pTCMB += numIters;
	}

	return numVertices;
#endif
}

bool ClothingAssetData::skinToTetraMesh(NxAbstractMeshDescription& destMesh,
                                        const NxAbstractMeshDescription& srcPM,
                                        const ClothingMeshAssetData& graphicalLod)
{
	if (graphicalLod.mTetraMap == NULL)
	{
		return false;
	}
#ifdef __SPU__
	PX_UNUSED(destMesh);
	PX_UNUSED(srcPM);
	PX_UNUSED(graphicalLod);
	PX_ASSERT(0);
	return false;
#else


	PX_ASSERT(srcPM.numIndices % 4 == 0);

	PX_ASSERT(destMesh.pIndices == NULL);
	PX_ASSERT(destMesh.pTangent == NULL);
	PX_ASSERT(destMesh.pBitangent == NULL);
	const bool computeNormals = destMesh.pNormal != NULL;

	PxVec3 dummyNormal;

	const PxU32 numGraphicalVertices = destMesh.numVertices;

	const PxU32* mainIndices = GetPhysicalMesh(graphicalLod.mPhysicalMeshId)->mIndices;

	for (PxU32 i = 0; i < numGraphicalVertices; i++)
	{
		const ClothingGraphicalLodParametersNS::TetraLink_Type& currentLink = graphicalLod.mTetraMap[i];
		if (currentLink.tetraIndex0 >= srcPM.numIndices)
		{
			continue;
		}

		PxVec3& position = destMesh.pPosition[i];
		position = PxVec3(0.0f);

		PxF32 vertexBary[4];
		vertexBary[0] = currentLink.vertexBary.x;
		vertexBary[1] = currentLink.vertexBary.y;
		vertexBary[2] = currentLink.vertexBary.z;
		vertexBary[3] = 1 - vertexBary[0] - vertexBary[1] - vertexBary[2];

		const PxU32* indices = mainIndices + currentLink.tetraIndex0;

		if (computeNormals)
		{
			PxVec3& normal = computeNormals ? destMesh.pNormal[i] : dummyNormal;
			normal = PxVec3(0.0f);

			PxF32 normalBary[4];
			normalBary[0] = currentLink.normalBary.x;
			normalBary[1] = currentLink.normalBary.y;
			normalBary[2] = currentLink.normalBary.z;
			normalBary[3] = 1 - normalBary[0] - normalBary[1] - normalBary[2];

			// compute skinned positions and normals
			for (PxU32 j = 0; j < 4; j++)
			{
				const PxVec3& pos = srcPM.pPosition[indices[j]];
				position += pos * vertexBary[j];
				normal += pos * normalBary[j];
			}

			normal = normal - position;
			//normal *= NxClothingUserRecompute::invSqrt(normal.magnitudeSquared());
			normal *= physx::recipSqrtFast(normal.magnitudeSquared());
		}
		else
		{
			// only compute skinned positions, not normals
			for (PxU32 j = 0; j < 4; j++)
			{
				const PxVec3& pos = srcPM.pPosition[indices[j]];
				position += pos * vertexBary[j];
			}
		}
		PX_ASSERT(position.isFinite());
	}
	return true;
#endif
}



}
}
}
