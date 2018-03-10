/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "PsArray.h"
#include "ApexRenderMeshAssetAuthoring.h"
#include "ApexRenderMeshActor.h"
#include "ApexSharedUtils.h"
#include "ApexCustomBufferIterator.h"
#include "PsShare.h"
#include "NiApexSDK.h"
#include "NiResourceProvider.h"

#include "NxFromPx.h" // PxVec3equals

#include "PsSort.h"

#ifndef WITHOUT_APEX_AUTHORING

namespace physx
{
namespace apex
{


ApexRenderMeshAssetAuthoring::ApexRenderMeshAssetAuthoring(NxResourceList& list, RenderMeshAssetParameters* params, const char* name)
{
	list.add(*this);

	createFromParameters(params);

	mName = name;
}

ApexRenderMeshAssetAuthoring::ApexRenderMeshAssetAuthoring(NxResourceList& list)
{
	list.add(*this);
}

ApexRenderMeshAssetAuthoring::~ApexRenderMeshAssetAuthoring()
{
}

// We will create our vertex map here.  Remapping will be from sorting by part index


void ApexRenderMeshAssetAuthoring::createRenderMesh(const MeshDesc& meshDesc, bool createMappingInformation)
{
	if (!meshDesc.isValid())
	{
		APEX_INVALID_OPERATION("MeshDesc is not valid!");
		return;
	}

	if (mParams != NULL)
	{
		mParams->destroy();
	}

	NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();
	mParams = (RenderMeshAssetParameters*)traits->createNxParameterized(RenderMeshAssetParameters::staticClassName());
	NxParameterized::Handle rootHandle(*mParams);

	// Submeshes
	mParams->getParameterHandle("materialNames", rootHandle);
	rootHandle.resizeArray((physx::PxI32)meshDesc.m_numSubmeshes);
	mParams->getParameterHandle("submeshes", rootHandle);
	rootHandle.resizeArray((physx::PxI32)meshDesc.m_numSubmeshes);
	setSubmeshCount(0);
	setSubmeshCount(meshDesc.m_numSubmeshes);
	for (PxU32 submeshNum = 0; submeshNum < meshDesc.m_numSubmeshes; ++submeshNum)
	{
		ApexRenderSubmesh& submesh = *mSubmeshes[submeshNum];
		SubmeshParameters* submeshParams = (SubmeshParameters*)traits->createNxParameterized(SubmeshParameters::staticClassName());
		submesh.createFromParameters(submeshParams);
		mParams->submeshes.buf[submeshNum] = submeshParams;
		//NxParameterized::Handle submeshHandle( submeshParams );
		const SubmeshDesc& submeshDesc = meshDesc.m_submeshes[submeshNum];

		// Material name
		NxParameterized::Handle handle(*mParams);
		mParams->getParameterHandle("materialNames", handle);
		NxParameterized::Handle elementHandle(*mParams);
		handle.getChildHandle((physx::PxI32)submeshNum, elementHandle);
		mParams->setParamString(elementHandle, submeshDesc.m_materialName);

		// Index buffer

		physx::Array<VertexPart> submeshMap;
		submeshMap.resize(submeshDesc.m_numVertices);
		const PxU32 invalidPart = PxMax(1u, submeshDesc.m_numParts);
		for (PxU32 i = 0; i < submeshDesc.m_numVertices; ++i)
		{
			submeshMap[i].part = invalidPart;
			submeshMap[i].vertexIndex = i;
		}
		bool success = false;
		switch (submeshDesc.m_indexType)
		{
		case IndexType::UINT:
			success = fillSubmeshMap<PxU32>(submeshMap, submeshDesc.m_partIndices, submeshDesc.m_numParts, submeshDesc.m_vertexIndices, submeshDesc.m_numIndices, submeshDesc.m_numVertices);
			break;
		case IndexType::USHORT:
			success = fillSubmeshMap<PxU16>(submeshMap, submeshDesc.m_partIndices, submeshDesc.m_numParts, submeshDesc.m_vertexIndices, submeshDesc.m_numIndices, submeshDesc.m_numVertices);
			break;
		default:
			PX_ALWAYS_ASSERT();
		}

		// error message?
		if (!success)
		{
			return;
		}

		if (submeshMap.size() > 1)
		{
			shdfnd::sort(submeshMap.begin(), submeshMap.size(), VertexPart());
		}

		PxU32 vertexCount = 0;
		for (; vertexCount < submeshDesc.m_numVertices; ++vertexCount)
		{
			if (submeshMap[vertexCount].part == invalidPart)
			{
				break;
			}
		}

		// Create inverse map for our internal remapping
		Array<PxI32> invMap;	// maps old indices to new indices
		invMap.resize(submeshDesc.m_numVertices);
		for (PxU32 i = 0; i < submeshDesc.m_numVertices; ++i)
		{
			const PxU32 vIndex = submeshMap[i].vertexIndex;
			if (i >= vertexCount)
			{
				invMap[vIndex] = -1;
			}
			else
			{
				invMap[vIndex] = (physx::PxI32)i;
			}
		}

		// Copy index buffer (remapping)
		NxParameterized::Handle ibHandle(submeshParams);
		submeshParams->getParameterHandle("indexBuffer", ibHandle);
		ibHandle.resizeArray((physx::PxI32)submeshDesc.m_numIndices);
		switch (submeshDesc.m_indexType)
		{
		case IndexType::UINT:
			for (PxU32 i = 0; i < submeshDesc.m_numIndices; ++i)
			{
				const PxU32 index = submeshDesc.m_vertexIndices != NULL ? ((PxU32*)submeshDesc.m_vertexIndices)[i] : i;
				submeshParams->indexBuffer.buf[i] = (physx::PxU32)invMap[index];
				PX_ASSERT(submeshParams->indexBuffer.buf[i] != (physx::PxU32)-1);
			}
			break;
		case IndexType::USHORT:
			for (PxU32 i = 0; i < submeshDesc.m_numIndices; ++i)
			{
				const PxU16 index = submeshDesc.m_vertexIndices != NULL ? ((PxU16*)submeshDesc.m_vertexIndices)[i] : (PxU16)i;
				submeshParams->indexBuffer.buf[i] = (PxU32)invMap[index];
				PX_ASSERT(submeshParams->indexBuffer.buf[i] != (physx::PxU32)-1);
			}
			break;
		default:
			PX_ALWAYS_ASSERT();
		}

		// Smoothing groups
		physx::PxI32 smoothingGroupArraySize = 0;
		if (submeshDesc.m_smoothingGroups != NULL)
		{
			switch (submeshDesc.m_primitive)
			{
			case Primitive::TRIANGLE_LIST:
				smoothingGroupArraySize = (physx::PxI32)submeshDesc.m_numIndices/3;
				break;
			default:
				PX_ALWAYS_ASSERT();	// We only have one kind of primitive
			}
		}
		if (smoothingGroupArraySize != 0)
		{
			NxParameterized::Handle sgHandle(submeshParams);
			submeshParams->getParameterHandle("smoothingGroups", sgHandle);
			sgHandle.resizeArray(smoothingGroupArraySize);
			sgHandle.setParamU32Array(submeshDesc.m_smoothingGroups, smoothingGroupArraySize, 0);
		}

		// Index partition
		NxParameterized::Handle ipHandle(submeshParams);
		submeshParams->getParameterHandle("indexPartition", ipHandle);
		ipHandle.resizeArray(PxMax((physx::PxI32)submeshDesc.m_numParts + 1, 2));

		if (submeshDesc.m_numParts == 0)
		{
			submeshParams->indexPartition.buf[0] = 0;
			submeshParams->indexPartition.buf[1] = submeshDesc.m_numIndices;
		}
		else
		{
			switch (submeshDesc.m_indexType)
			{
			case IndexType::UINT:
				for (PxU32 i = 0; i < submeshDesc.m_numParts; ++i)
				{
					submeshParams->indexPartition.buf[i] = ((PxU32*)submeshDesc.m_partIndices)[i];
				}
				submeshParams->indexPartition.buf[submeshDesc.m_numParts] = submeshDesc.m_numIndices;
				break;
			case IndexType::USHORT:
				for (PxU32 i = 0; i < submeshDesc.m_numParts; ++i)
				{
					submeshParams->indexPartition.buf[i] = (PxU32)((PxU16*)submeshDesc.m_partIndices)[i];
				}
				submeshParams->indexPartition.buf[submeshDesc.m_numParts] = submeshDesc.m_numIndices;
				break;
			default:
				PX_ALWAYS_ASSERT();
			}
		}

		// Vertex partition
		Array<PxU32> lookup;
		createIndexStartLookup(lookup, 0, submeshDesc.m_numParts, (PxI32*)submeshMap.begin(), vertexCount, sizeof(VertexPart));
		NxParameterized::Handle vpHandle(submeshParams);
		submeshParams->getParameterHandle("vertexPartition", vpHandle);
		vpHandle.resizeArray((physx::PxI32)lookup.size());
		vpHandle.setParamU32Array(lookup.begin(), (physx::PxI32)lookup.size());

		// Vertex buffer

		// Create format description
		ApexVertexFormat format;

		for (PxU32 i = 0; i < submeshDesc.m_numVertexBuffers; ++i)
		{
			const VertexBuffer& vb = submeshDesc.m_vertexBuffers[i];
			for (PxU32 semantic = 0; semantic < NxRenderVertexSemantic::NUM_SEMANTICS; ++semantic)
			{
				NxRenderVertexSemantic::Enum vertexSemantic = (NxRenderVertexSemantic::Enum)semantic;
				NxRenderDataFormat::Enum vertexFormat = vb.getSemanticData(vertexSemantic).format;

				if (vertexSemanticFormatValid(vertexSemantic, vertexFormat))
				{
					PxI32 bufferIndex = format.addBuffer(format.getSemanticName(vertexSemantic));
					format.setBufferFormat((physx::PxU32)bufferIndex, vb.getSemanticData(vertexSemantic).format);
				}
				else if (vertexFormat != NxRenderDataFormat::UNSPECIFIED)
				{
					APEX_INVALID_PARAMETER("Format (%d) is not valid for Semantic (%s)", vertexFormat, format.getSemanticName(vertexSemantic));
				}
			}
		}

		format.setWinding(submeshDesc.m_cullMode);

		// Include custom buffers
		for (PxU32 i = 0; i < submeshDesc.m_numVertexBuffers; ++i)
		{
			const VertexBuffer& vb = submeshDesc.m_vertexBuffers[i];
			for (PxU32 index = 0; index < vb.getNumCustomSemantics(); ++index)
			{
				const NxApexRenderSemanticData& data = vb.getCustomSemanticData(index);
				// BRG - reusing data.ident as the custom channel name.  What to do with the serialize parameter?
				PxI32 bufferIndex = format.addBuffer((char*)data.ident);
				format.setBufferFormat((physx::PxU32)bufferIndex, data.format);

				// PH: custom buffers are never serialized this way, we might need to change this!
				format.setBufferSerialize((physx::PxU32)bufferIndex, data.serialize);
			}
		}

		if (createMappingInformation)
		{
			PxI32 bufferIndex = format.addBuffer("VERTEX_ORIGINAL_INDEX");
			format.setBufferFormat((physx::PxU32)bufferIndex, NxRenderDataFormat::UINT1);
		}

		// Create apex vertex buffer
		submesh.buildVertexBuffer(format, vertexCount);

		// Now fill in...
		for (PxU32 i = 0; i < submeshDesc.m_numVertexBuffers; ++i)
		{
			const VertexBuffer& vb = submeshDesc.m_vertexBuffers[i];
			const NxVertexFormat& vf = submesh.getVertexBuffer().getFormat();

			NxApexRenderSemanticData boneWeightData;
			NxApexRenderSemanticData boneIndexData;
			NxRenderDataFormat::Enum checkFormatBoneWeight = NxRenderDataFormat::UNSPECIFIED;
			NxRenderDataFormat::Enum checkFormatBoneIndex = NxRenderDataFormat::UNSPECIFIED;
			NxRenderDataFormat::Enum dstFormatBoneWeight = NxRenderDataFormat::UNSPECIFIED;
			NxRenderDataFormat::Enum dstFormatBoneIndex = NxRenderDataFormat::UNSPECIFIED;
			void* dstDataWeight = NULL;
			void* dstDataIndex = NULL;
			PxU32 numBoneWeights = 0;
			PxU32 numBoneIndices = 0;

			for (PxU32 semantic = 0; semantic < NxRenderVertexSemantic::NUM_SEMANTICS; ++semantic)
			{
				if (vertexSemanticFormatValid((NxRenderVertexSemantic::Enum)semantic, vb.getSemanticData((NxRenderVertexSemantic::Enum)semantic).format))
				{
					NxRenderDataFormat::Enum dstFormat;
					void* dst = submesh.getVertexBufferWritable().getBufferAndFormatWritable(dstFormat, (physx::PxU32)vf.getBufferIndexFromID(vf.getSemanticID((NxRenderVertexSemantic::Enum)semantic)));
					const NxApexRenderSemanticData& data = vb.getSemanticData((NxRenderVertexSemantic::Enum)semantic);

					copyRenderVertexBuffer(dst, dstFormat, 0, 0, data.data, data.srcFormat, data.stride, 0, submeshDesc.m_numVertices, invMap.begin());

					if (semantic == NxRenderVertexSemantic::BONE_WEIGHT)
					{
						boneWeightData = data;
						// Verification code for bone weights.
						switch (data.srcFormat)
						{
						case NxRenderDataFormat::FLOAT1:
							checkFormatBoneWeight = NxRenderDataFormat::FLOAT1;
							numBoneWeights = 1;
							break;
						case NxRenderDataFormat::FLOAT2:
							checkFormatBoneWeight = NxRenderDataFormat::FLOAT2;
							numBoneWeights = 2;
							break;
						case NxRenderDataFormat::FLOAT3:
							checkFormatBoneWeight = NxRenderDataFormat::FLOAT3;
							numBoneWeights = 3;
							break;
						case NxRenderDataFormat::FLOAT4:
							checkFormatBoneWeight = NxRenderDataFormat::FLOAT4;
							numBoneWeights = 4;
							break;
						default:
							break;
						}

						dstDataWeight = dst;
						dstFormatBoneWeight = dstFormat;
					}
					else if (semantic == NxRenderVertexSemantic::BONE_INDEX)
					{
						boneIndexData = data;
						switch (data.srcFormat)
						{
						case NxRenderDataFormat::USHORT1:
							checkFormatBoneIndex = NxRenderDataFormat::USHORT1;
							numBoneIndices = 1;
							break;
						case NxRenderDataFormat::USHORT2:
							checkFormatBoneIndex = NxRenderDataFormat::USHORT2;
							numBoneIndices = 2;
							break;
						case NxRenderDataFormat::USHORT3:
							checkFormatBoneIndex = NxRenderDataFormat::USHORT3;
							numBoneIndices = 3;
							break;
						case NxRenderDataFormat::USHORT4:
							checkFormatBoneIndex = NxRenderDataFormat::USHORT4;
							numBoneIndices = 4;
							break;
						default:
							break;
						}
						dstDataIndex = dst;
						dstFormatBoneIndex = dstFormat;
					}
				}
			}

			// some verification code
			if (numBoneIndices > 1 && numBoneWeights == numBoneIndices)
			{
				PxF32 verifyWeights[4] = { 0.0f };
				PxU16 verifyIndices[4] = { 0 };
				for (PxU32 vi = 0; vi < submeshDesc.m_numVertices; vi++)
				{
					const PxI32 dest = invMap[vi];
					if (dest >= 0)
					{

						copyRenderVertexBuffer(verifyWeights, checkFormatBoneWeight, 0, 0, boneWeightData.data, boneWeightData.srcFormat, boneWeightData.stride, vi, 1);
						copyRenderVertexBuffer(verifyIndices, checkFormatBoneIndex, 0, 0, boneIndexData.data, boneIndexData.srcFormat, boneIndexData.stride, vi, 1);

						PxF32 sum = 0.0f;
						for (PxU32 j = 0; j < numBoneWeights; j++)
						{
							sum += verifyWeights[j];
						}

						if (PxAbs(1 - sum) > 0.001)
						{
							if (sum > 0.0f)
							{
								for (PxU32 j = 0; j < numBoneWeights; j++)
								{
									verifyWeights[j] /= sum;
								}
							}

							APEX_INVALID_PARAMETER("Submesh %d Vertex %d has been normalized, bone weight was (%f)", i, vi, sum);
						}
						// PH: bubble sort, don't kill me for this
						for (PxU32 j = 1; j < numBoneWeights; j++)
						{
							for (PxU32 k = 1; k < numBoneWeights; k++)
							{
								if (verifyWeights[k - 1] < verifyWeights[k])
								{
									physx::swap(verifyWeights[k - 1], verifyWeights[k]);
									physx::swap(verifyIndices[k - 1], verifyIndices[k]);
								}
							}
						}

						for (PxU32 j = 0; j < numBoneWeights; j++)
						{
							if (verifyWeights[j] == 0.0f)
							{
								verifyIndices[j] = 0;
							}
						}

						copyRenderVertexBuffer(dstDataWeight, dstFormatBoneWeight, 0, (physx::PxU32)dest, verifyWeights, checkFormatBoneWeight, 0, 0, 1);
						copyRenderVertexBuffer(dstDataIndex, dstFormatBoneIndex, 0, (physx::PxU32)dest, verifyIndices, checkFormatBoneIndex, 0, 0, 1);
					}
				}
			}

			// Custom buffers
			for (PxU32 index = 0; index < vb.getNumCustomSemantics(); ++index)
			{
				const NxApexRenderSemanticData& data = vb.getCustomSemanticData(index);
				const PxI32 bufferIndex = format.getBufferIndexFromID(format.getID((char*)data.ident));
				PX_ASSERT(bufferIndex >= 0);
				void* dst = const_cast<void*>(submesh.getVertexBuffer().getBuffer((physx::PxU32)bufferIndex));
				NxRenderDataFormat::Enum srcFormat = data.srcFormat != NxRenderDataFormat::UNSPECIFIED ? data.srcFormat : data.format;
				copyRenderVertexBuffer(dst, data.format, 0, 0, data.data, srcFormat, data.stride, 0, submeshDesc.m_numVertices, invMap.begin());
			}
		}

		if (createMappingInformation)
		{
			const NxVertexFormat::BufferID bufferID = format.getID("VERTEX_ORIGINAL_INDEX");
			const PxI32 bufferIndex = format.getBufferIndexFromID(bufferID);
			NxRenderDataFormat::Enum bufferFormat = format.getBufferFormat((physx::PxU32)bufferIndex);
			PX_ASSERT(bufferIndex >= 0);
			const void* dst = submesh.getVertexBuffer().getBuffer((physx::PxU32)bufferIndex);
			copyRenderVertexBuffer(const_cast<void*>(dst), bufferFormat, 0, 0, &submeshMap[0].vertexIndex , NxRenderDataFormat::UINT1 , sizeof(VertexPart), 0, vertexCount, NULL);
		}
	}

	// Part bounds
	PxU32 partCount = 1;
	for (PxU32 submeshNum = 0; submeshNum < meshDesc.m_numSubmeshes; ++submeshNum)
	{
		partCount = physx::PxMax(partCount, meshDesc.m_submeshes[submeshNum].m_numParts);
	}
	mParams->getParameterHandle("partBounds", rootHandle);
	mParams->resizeArray(rootHandle, (physx::PxI32)partCount);
	for (PxU32 partNum = 0; partNum < partCount; ++partNum)
	{
		mParams->partBounds.buf[partNum].setEmpty();
		// Add part vertices
		for (PxU32 submeshNum = 0; submeshNum < meshDesc.m_numSubmeshes; ++submeshNum)
		{
			SubmeshParameters* submeshParams = DYNAMIC_CAST(SubmeshParameters*)(mParams->submeshes.buf[submeshNum]);
			ApexRenderSubmesh& submesh = *mSubmeshes[submeshNum];
			NxRenderDataFormat::Enum positionFormat;
			const NxVertexFormat& vf = submesh.getVertexBuffer().getFormat();
			const PxVec3* positions = (const PxVec3*)submesh.getVertexBuffer().getBufferAndFormat(positionFormat, 
				(physx::PxU32)vf.getBufferIndexFromID(vf.getSemanticID(NxRenderVertexSemantic::POSITION)));
			if (positions && positionFormat == NxRenderDataFormat::FLOAT3)
			{
				for (PxU32 vertexIndex = submeshParams->vertexPartition.buf[partNum]; vertexIndex < submeshParams->vertexPartition.buf[partNum + 1]; ++vertexIndex)
				{
					mParams->partBounds.buf[partNum].include(positions[vertexIndex]);
				}
			}
		}
	}

	mParams->textureUVOrigin = meshDesc.m_uvOrigin;

	createLocalData();
}

physx::PxU32 ApexRenderMeshAssetAuthoring::createReductionMap(PxU32* map, const NxVertex* vertices, const PxU32* smoothingGroups, PxU32 vertexCount,
        const physx::PxVec3& positionTolerance, physx::PxF32 normalTolerance, physx::PxF32 UVTolerance)
{
	physx::Array<BoundsRep> vertexNeighborhoods;
	vertexNeighborhoods.resize(vertexCount);
	const physx::PxVec3 neighborhoodExtent = 0.5f * positionTolerance;
	for (physx::PxU32 vertexNum = 0; vertexNum < vertexCount; ++vertexNum)
	{
		vertexNeighborhoods[vertexNum].aabb = PxBounds3(vertices[vertexNum].position - neighborhoodExtent, vertices[vertexNum].position + neighborhoodExtent);
	}

	physx::Array<IntPair> vertexNeighbors;
	if (vertexNeighborhoods.size() > 0)
	{
		boundsCalculateOverlaps(vertexNeighbors, Bounds3XYZ, &vertexNeighborhoods[0], vertexNeighborhoods.size(), sizeof(vertexNeighborhoods[0]));
	}

	for (physx::PxU32 i = 0; i < vertexCount; ++i)
	{
		map[i] = i;
	}

	for (physx::PxU32 pairNum = 0; pairNum < vertexNeighbors.size(); ++pairNum)
	{
		const IntPair& pair = vertexNeighbors[pairNum];
		const physx::PxU32 map0 = map[pair.i0];
		const physx::PxU32 map1 = map[pair.i1];
		if (smoothingGroups != NULL && smoothingGroups[map0] != smoothingGroups[map1])
		{
			continue;
		}
		const NxVertex& vertex0 = vertices[map0];
		const NxVertex& vertex1 = vertices[map1];
		if (physx::PxAbs(vertex0.position.x - vertex1.position.x) > positionTolerance.x ||
		        physx::PxAbs(vertex0.position.y - vertex1.position.y) > positionTolerance.y ||
		        physx::PxAbs(vertex0.position.z - vertex1.position.z) > positionTolerance.z)
		{
			continue;
		}
		if (!PxVec3equals(vertex0.normal, vertex1.normal, normalTolerance) ||
		        !PxVec3equals(vertex0.tangent, vertex1.tangent, normalTolerance) ||
		        !PxVec3equals(vertex0.binormal, vertex1.binormal, normalTolerance))
		{
			continue;
		}
		physx::PxU32 uvNum = 0;
		for (; uvNum < NxVertexFormat::MAX_UV_COUNT; ++uvNum)
		{
			const NxVertexUV& uv0 = vertex0.uv[uvNum];
			const NxVertexUV& uv1 = vertex1.uv[uvNum];
			if (physx::PxAbs(uv0[0] - uv1[0]) > UVTolerance || physx::PxAbs(uv0[1] - uv1[1]) > UVTolerance)
			{
				break;
			}
		}
		if (uvNum < NxVertexFormat::MAX_UV_COUNT)
		{
			continue;
		}
		map[pair.i1] = map0;
	}

	physx::Array<physx::PxI32> offsets(vertexCount, -1);
	for (physx::PxU32 i = 0; i < vertexCount; ++i)
	{
		offsets[map[i]] = 0;
	}
	physx::PxI32 delta = 0;
	for (physx::PxU32 i = 0; i < vertexCount; ++i)
	{
		delta += offsets[i];
		offsets[i] = delta;
	}
	for (physx::PxU32 i = 0; i < vertexCount; ++i)
	{
		map[i] += offsets[map[i]];
	}
	return vertexCount + delta;
}



void ApexRenderMeshAssetAuthoring::setMaterialName(physx::PxU32 submeshIndex, const char* name)
{
	size_t maxMaterials = (physx::PxU32)mParams->materialNames.arraySizes[0];
	PX_ASSERT(submeshIndex < maxMaterials);
	if (submeshIndex < maxMaterials)
	{
		NxParameterized::Handle handle(*mParams);
		mParams->getParameterHandle("materialNames", handle);
		NxParameterized::Handle elementHandle(*mParams);
		handle.getChildHandle((physx::PxI32)submeshIndex, elementHandle);
		mParams->setParamString(elementHandle, name ? name : "");
	}
}

void						ApexRenderMeshAssetAuthoring::setWindingOrder(physx::PxU32 submeshIndex, NxRenderCullMode::Enum winding)
{
	ApexRenderSubmesh& subMesh = *ApexRenderMeshAsset::mSubmeshes[submeshIndex];
	NiApexVertexBuffer& vb = subMesh.getVertexBufferWritable();
	vb.getFormatWritable().setWinding(winding);
}

NxRenderCullMode::Enum		ApexRenderMeshAssetAuthoring::getWindingOrder(physx::PxU32 submeshIndex) const
{
	const NxRenderSubmesh& subMesh = getSubmesh(submeshIndex);
	const NxVertexBuffer& vb = subMesh.getVertexBuffer();
	const NxVertexFormat& format = vb.getFormat();
	return format.getWinding();
}



template <typename PxU>
bool ApexRenderMeshAssetAuthoring::fillSubmeshMap(physx::Array<VertexPart>& submeshMap, const void* const partIndicesVoid,
        PxU32 numParts, const void* const vertexIndicesVoid,
        PxU32 numSubmeshIndices, PxU32 numSubmeshVertices)
{
	PxU partIndexStart = 0;
	if (numParts == 0)
	{
		numParts = 1;
	}

	const PxU* const partIndices = partIndicesVoid != NULL ? reinterpret_cast<const PxU * const>(partIndicesVoid) : &partIndexStart;
	const PxU* const vertexIndices = reinterpret_cast<const PxU * const>(vertexIndicesVoid);

	for (PxU32 i = 0; i < numParts; ++i)
	{
		const PxU32 stop = i + 1 < numParts ? partIndices[i + 1] : numSubmeshIndices;
		for (PxU32 j = partIndices[i]; j < stop; ++j)
		{
			const PxU32 vertexIndex = vertexIndices != NULL ? vertexIndices[j] : j;
			if (vertexIndex >= numSubmeshVertices)
			{
				return false;	// to do: issue error - index out of range
			}
			if (submeshMap[vertexIndex].part != numParts && submeshMap[vertexIndex].part != i)
			{
				return false;	// to do: issue error - vertex in more than one part
			}
			submeshMap[vertexIndex].part = i;
		}
	}
	return true;
}

}
} // end namespace physx::apex
#endif
