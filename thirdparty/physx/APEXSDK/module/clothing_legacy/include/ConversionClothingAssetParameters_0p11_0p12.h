/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CONVERSIONCLOTHINGASSETPARAMETERS_0P11_0P12H_H
#define CONVERSIONCLOTHINGASSETPARAMETERS_0P11_0P12H_H

#include "ParamConversionTemplate.h"
#include "ClothingAssetParameters_0p11.h"
#include "ClothingAssetParameters_0p12.h"
#include "ClothingGraphicalLodParameters_0p4.h"
#include "ClothingPhysicalMeshParameters_0p10.h"
#include "RenderMeshAssetParameters.h"
#include "SubmeshParameters.h"
#include "VertexBufferParameters.h"
#include "VertexFormatParameters.h"
#include "BufferF32x3.h"
#include "BufferF32x4.h"
#include "ModuleClothingHelpers.h"

namespace physx
{
namespace apex
{
namespace legacy
{

typedef ParamConversionTemplate<ClothingAssetParameters_0p11, ClothingAssetParameters_0p12, 11, 12> ConversionClothingAssetParameters_0p11_0p12Parent;

class ConversionClothingAssetParameters_0p11_0p12: ConversionClothingAssetParameters_0p11_0p12Parent
{
public:
	static NxParameterized::Conversion* Create(NxParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingAssetParameters_0p11_0p12));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingAssetParameters_0p11_0p12)(t) : 0;
	}

protected:
	ConversionClothingAssetParameters_0p11_0p12(NxParameterized::Traits* t) : ConversionClothingAssetParameters_0p11_0p12Parent(t) {}

	const NxParameterized::PrefVer* getPreferredVersions() const
	{
		static NxParameterized::PrefVer prefVers[] =
		{
			//TODO:
			//	Add your preferred versions for included references here.
			//	Entry format is
			//		{ (const char*)longName, (PxU32)preferredVersion }
			{ "graphicalLods[]", 4 },
			{ "physicalMeshes[]", 10 },
			{ 0, 0 } // Terminator (do not remove!)
		};

		return prefVers;
	}

	bool convert()
	{
		//TODO:
		//	Write custom conversion code here using mNewData and mLegacyData members.
		//
		//	Note that
		//		- mNewData has already been initialized with default values
		//		- same-named/same-typed members have already been copied
		//			from mLegacyData to mNewData
		//		- included references were moved to mNewData
		//			(and updated to preferred versions according to getPreferredVersions)
		//
		//	For more info see the versioning wiki.

		// look up the vertex indices for the skinClothMap update
		for (PxI32 i = 0; i < mNewData->graphicalLods.arraySizes[0]; ++i)
		{
			ClothingGraphicalLodParameters_0p4* graphicalLod = (ClothingGraphicalLodParameters_0p4*)mNewData->graphicalLods.buf[i];

			PxU32 physicalMeshIndex = graphicalLod->physicalMeshId;
			PxF32 meshThickness = graphicalLod->skinClothMapOffset;

			// use already updated physical mesh for lookup
			PX_ASSERT(physicalMeshIndex < (PxU32)mNewData->physicalMeshes.arraySizes[0]);
			ClothingPhysicalMeshParameters_0p10NS::PhysicalMesh_Type& physicalMesh = ((ClothingPhysicalMeshParameters_0p10*)mNewData->physicalMeshes.buf[physicalMeshIndex])->physicalMesh;

			RenderMeshAssetParameters* rma = DYNAMIC_CAST(RenderMeshAssetParameters*)(graphicalLod->renderMeshAsset);

			PxI32 numSubmeshes = rma->submeshes.arraySizes[0];
			Array<PxVec3*> submeshPositionBuffer((physx::PxU32)numSubmeshes, NULL);
			Array<PxVec4*> submeshTangentBuffer((physx::PxU32)numSubmeshes, NULL);
			Array<PxU32> submeshVertexOffsets((physx::PxU32)numSubmeshes);
			PxU32 submeshVertexOffset = 0;
			for (PxI32 submeshIdx = 0; submeshIdx < numSubmeshes; ++submeshIdx)
			{
				SubmeshParameters* submesh = DYNAMIC_CAST(SubmeshParameters*)(rma->submeshes.buf[submeshIdx]);

				VertexBufferParameters* vb = DYNAMIC_CAST(VertexBufferParameters*)(submesh->vertexBuffer);
				VertexFormatParameters* vertexFormat = DYNAMIC_CAST(VertexFormatParameters*)(vb->vertexFormat);

				PxU32 numBuffers = (physx::PxU32)vb->buffers.arraySizes[0];
				for (PxU32 bufferIdx = 0; bufferIdx < numBuffers; ++bufferIdx)
				{
					VertexFormatParametersNS::BufferFormat_Type bufferFormat = vertexFormat->bufferFormats.buf[bufferIdx];

					if (bufferFormat.semantic == NxRenderVertexSemantic::POSITION)
					{
						if (bufferFormat.format == NxRenderDataFormat::FLOAT3)
						{
							BufferF32x3* posBuffer = DYNAMIC_CAST(BufferF32x3*)(vb->buffers.buf[bufferIdx]);
							submeshPositionBuffer[(physx::PxU32)submeshIdx] = posBuffer->data.buf;
						}
					}

					if (bufferFormat.semantic == NxRenderVertexSemantic::TANGENT)
					{
						if (bufferFormat.format == NxRenderDataFormat::FLOAT4)
						{
							BufferF32x4* tangentBuffer = DYNAMIC_CAST(BufferF32x4*)(vb->buffers.buf[bufferIdx]);
							submeshTangentBuffer[(physx::PxU32)submeshIdx] = (PxVec4*)tangentBuffer->data.buf;
						}
					}
				}

				submeshVertexOffsets[(physx::PxU32)submeshIdx] = submeshVertexOffset;
				submeshVertexOffset += vb->vertexCount;
			}

			PxVec3* normals = physicalMesh.skinningNormals.buf;
			if (normals == NULL)
			{
				normals = physicalMesh.normals.buf;
			}
			for (PxI32 j = 0; j < graphicalLod->skinClothMap.arraySizes[0]; ++j)
			{
				ClothingGraphicalLodParameters_0p4NS::SkinClothMapD_Type& mapD = graphicalLod->skinClothMap.buf[j];

				const PxU32 faceIndex = mapD.vertexIndex0; // this was temporarily stored in the GraphicalLod update
				mapD.vertexIndex0 = physicalMesh.indices.buf[faceIndex + 0];
				mapD.vertexIndex1 = physicalMesh.indices.buf[faceIndex + 1];
				mapD.vertexIndex2 = physicalMesh.indices.buf[faceIndex + 2];

				clothing::TriangleWithNormals triangle;
				triangle.faceIndex0 = faceIndex;
				// store vertex information in triangle
				for (physx::PxU32 k = 0; k < 3; k++)
				{
					PxU32 triVertIndex = physicalMesh.indices.buf[triangle.faceIndex0 + k];
					triangle.vertices[k] = physicalMesh.vertices.buf[triVertIndex];
					triangle.normals[k] = normals[triVertIndex];
				}
				triangle.init();

				// find index of the gaphics vertex in the submesh
				PxU32 graphVertIdx = 0;
				PxI32 submeshIdx = 0;
				for (PxI32 k = (physx::PxI32)submeshVertexOffsets.size()-1; k >= 0; --k)
				{
					if (submeshVertexOffsets[(physx::PxU32)k] <= mapD.vertexIndexPlusOffset)
					{
						graphVertIdx = mapD.vertexIndexPlusOffset - submeshVertexOffsets[(physx::PxU32)k];
						submeshIdx = k;
						break;
					}
				}

				if (submeshPositionBuffer[(physx::PxU32)submeshIdx] != NULL && submeshTangentBuffer[(physx::PxU32)submeshIdx] != NULL)
				{
					PxVec3 graphicalPos = submeshPositionBuffer[(physx::PxU32)submeshIdx][graphVertIdx];
					PxVec3 graphicalTangent = submeshTangentBuffer[(physx::PxU32)submeshIdx][graphVertIdx].getXYZ();
					PxVec3 tangentRelative = graphicalTangent.isZero() ? PxVec3(0) : graphicalPos + graphicalTangent;

					PxVec3 dummy(0.0f);
					clothing::ModuleClothingHelpers::computeTriangleBarys(triangle, dummy, dummy, tangentRelative, meshThickness, 0, true);

					mapD.tangentBary = triangle.tempBaryTangent;
					if (mapD.vertexBary == PxVec3(PX_MAX_F32) || !mapD.vertexBary.isFinite())
					{
						APEX_DEBUG_WARNING("Barycentric coordinates for position is not valid. SubmeshIndex: %i, VertexIndex: %i. MapIndex: %i", submeshIdx, graphVertIdx, j);
					}
					if (mapD.normalBary == PxVec3(PX_MAX_F32) || !mapD.normalBary.isFinite())
					{
						APEX_DEBUG_WARNING("Barycentric coordinates for normal is not valid. SubmeshIndex: %i, VertexIndex: %i. MapIndex: %i", submeshIdx, graphVertIdx, j);
					}
					if (mapD.tangentBary == PxVec3(PX_MAX_F32) || !mapD.tangentBary.isFinite())
					{
						APEX_DEBUG_WARNING("Barycentric coordinates for tangent is not valid. SubmeshIndex: %i, VertexIndex: %i. MapIndex: %i", submeshIdx, graphVertIdx, j);
					}
				}
			}
		}


		// look up the vertex indices for the lod transition maps
		for (PxI32 i = 0; i < mNewData->physicalMeshes.arraySizes[0]; ++i)
		{
			ClothingPhysicalMeshParameters_0p10** physicalMeshes = (ClothingPhysicalMeshParameters_0p10**)mNewData->physicalMeshes.buf;
			ClothingPhysicalMeshParameters_0p10* currentMesh = (ClothingPhysicalMeshParameters_0p10*)physicalMeshes[i];

			const ClothingPhysicalMeshParameters_0p10* nextMesh = (i+1) < mNewData->physicalMeshes.arraySizes[0] ? (ClothingPhysicalMeshParameters_0p10*)physicalMeshes[i+1] : NULL;
			for (PxI32 j = 0; j < currentMesh->transitionUp.arraySizes[0]; ++j)
			{
				const PxU32 faceIndex0 = currentMesh->transitionUp.buf[j].vertexIndex0; // this was temporarily stored in the PhysicalMesh update
				PX_ASSERT(nextMesh != NULL);
				PX_ASSERT(faceIndex0 + 2 < (PxU32)nextMesh->physicalMesh.indices.arraySizes[0]);
				currentMesh->transitionUp.buf[j].vertexIndex0 = nextMesh->physicalMesh.indices.buf[faceIndex0 + 0];
				currentMesh->transitionUp.buf[j].vertexIndex1 = nextMesh->physicalMesh.indices.buf[faceIndex0 + 1];
				currentMesh->transitionUp.buf[j].vertexIndex2 = nextMesh->physicalMesh.indices.buf[faceIndex0 + 2];
			}

			const ClothingPhysicalMeshParameters_0p10* previousMesh = (i-1) >= 0 ? (ClothingPhysicalMeshParameters_0p10*)physicalMeshes[i-1] : NULL;
			for (PxI32 j = 0; j < currentMesh->transitionDown.arraySizes[0]; ++j)
			{
				const PxU32 faceIndex0 = currentMesh->transitionDown.buf[j].vertexIndex0; // this was temporarily stored in the PhysicalMesh update
				PX_ASSERT(previousMesh != NULL);
				PX_ASSERT(faceIndex0 + 2 < (PxU32)previousMesh->physicalMesh.indices.arraySizes[0]);
				currentMesh->transitionDown.buf[j].vertexIndex0 = previousMesh->physicalMesh.indices.buf[faceIndex0 + 0];
				currentMesh->transitionDown.buf[j].vertexIndex1 = previousMesh->physicalMesh.indices.buf[faceIndex0 + 1];
				currentMesh->transitionDown.buf[j].vertexIndex2 = previousMesh->physicalMesh.indices.buf[faceIndex0 + 2];
			}
		}

		return true;
	}
};

}
}
} // namespace physx::apex

#endif
