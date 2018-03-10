/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "PxTriangleMesh.h"
#include "PxvGeometry.h"
#include "PxsMaterialManager.h"
#include "PxcNpThreadContext.h"
#include "GuHeightField.h"
#include "CmMemFetch.h"

using namespace physx;
using namespace Gu;

namespace physx
{
	bool PxcGetMaterialShapeHeightField(const PxsShapeCore* shape0, const PxsShapeCore* shape1, PxcNpThreadContext& context, PxsMaterialInfo* materialInfo);
	bool PxcGetMaterialHeightField(const PxsShapeCore* shape, const PxU32 index, PxcNpThreadContext& context, PxsMaterialInfo* materialInfo);
	PxU32 GetMaterialIndex(const Gu::HeightFieldData* hfData, PxU32 triangleIndex);
}


physx::PxU32 physx::GetMaterialIndex(const Gu::HeightFieldData* hfData, PxU32 triangleIndex)
{
	const PxU32 sampleIndex = triangleIndex >> 1;
	const bool isFirstTriangle = (triangleIndex & 0x1) == 0;

	//get sample
#ifdef __SPU__
#if HF_TILED_MEMORY_LAYOUT
	PxHeightFieldSample* hf = NULL;
	if(physx::IsHeightfieldCacheInitialised())
	{
		PxU32 col = sampleIndex%hfData->columns;
		PxU32 row = sampleIndex/hfData->columns;
		hf = g_sampleCache.getSample(col, row);
	}
	else
	{
		hf = Cm::memFetchAsync<PxHeightFieldSample>(g_HFSampleBuffer, Cm::MemFetchPtr(hfData->samples + sampleIndex), sizeof(PxHeightFieldSample), 1);
		Cm::memFetchWait(1);
	}
#else
	PxHeightFieldSample* hf = Cm::memFetchAsync<PxHeightFieldSample>(g_HFSampleBuffer, Cm::MemFetchPtr(hfData->samples + sampleIndex), sizeof(PxHeightFieldSample), 1);
	Cm::memFetchWait(1);
#endif

#else
	PxHeightFieldSample* hf = &hfData->samples[sampleIndex];
#endif
	return isFirstTriangle ? hf->materialIndex0 : hf->materialIndex1;
}

bool physx::PxcGetMaterialHeightField(const PxsShapeCore* shape, const PxU32 index, PxcNpThreadContext& context, PxsMaterialInfo* materialInfo)
{
	ContactBuffer& contactBuffer = context.mContactBuffer;
	const PxHeightFieldGeometryLL& hfGeom = shape->geometry.get<const PxHeightFieldGeometryLL>();
	if(hfGeom.materials.numIndices <= 1)
	{
		for(PxU32 i=0; i< contactBuffer.count; ++i)
		{
			(&materialInfo[i].mMaterialIndex0)[index] = shape->materialIndex;
		}
	}
	else
	{

	
		PxU16* materialIndices = hfGeom.materials.indices;
			
#ifdef __SPU__
			
		const PxU32 alignedSize = (sizeof(Gu::HeightFieldData) + 31) & 0xfffffff0;
		const PxU32 alignedAddress = (PxU32)hfGeom.heightFieldData & 0xfffffff0;
		const PxU32 offset = (PxU32)hfGeom.heightFieldData - alignedAddress;

		CELL_ALIGN(16, PxU8 hfData[alignedSize]);

		Cm::memFetchAlignedAsync((Cm::MemFetchPtr)hfData, (Cm::MemFetchPtr)alignedAddress, alignedSize, 1);

		Gu::HeightFieldData* hf = (Gu::HeightFieldData*)&hfData[offset];

		Cm::memFetchWait(1);

		CELL_ALIGN(16, PxU16 indices[128]);

		const PxU32 alignedIndSize = ((sizeof(PxU16) * hfGeom.materials.numIndices) + 15) & 0xfffffff0;

		Cm::memFetchAlignedAsync((Cm::MemFetchPtr)(&indices), (Cm::MemFetchPtr)hfGeom.materials.indices, alignedIndSize, 1);
		Cm::memFetchWait(1);

		materialIndices = indices;
#else
		const Gu::HeightFieldData* hf = hfGeom.heightFieldData;
	#endif
		
		for(PxU32 i=0; i< contactBuffer.count; ++i)
		{
			Gu::ContactPoint& contact = contactBuffer.contacts[i];
			const PxU32 localMaterialIndex = GetMaterialIndex(hf, (&contact.internalFaceIndex0)[index]);
			(&materialInfo[i].mMaterialIndex0)[index] = materialIndices[localMaterialIndex];
		}
	}
	
	return true;

}

bool physx::PxcGetMaterialShapeHeightField(const PxsShapeCore* shape0, const PxsShapeCore* shape1, PxcNpThreadContext& context,  PxsMaterialInfo* materialInfo)
{
	
	ContactBuffer& contactBuffer = context.mContactBuffer;
	const PxHeightFieldGeometryLL& hfGeom = shape1->geometry.get<const PxHeightFieldGeometryLL>();
	if(hfGeom.materials.numIndices <= 1)
	{
		for(PxU32 i=0; i< contactBuffer.count; ++i)
		{
			materialInfo[i].mMaterialIndex0 = shape0->materialIndex;
			materialInfo[i].mMaterialIndex1 = shape1->materialIndex;
		}
	}
	else
	{

	
		PxU16* materialIndices = hfGeom.materials.indices;
			
#ifdef __SPU__
			
		const PxU32 alignedSize = (sizeof(Gu::HeightFieldData) + 31) & 0xfffffff0;
		const PxU32 alignedAddress = (PxU32)hfGeom.heightFieldData & 0xfffffff0;
		const PxU32 offset = (PxU32)hfGeom.heightFieldData - alignedAddress;

		CELL_ALIGN(16, PxU8 hfData[alignedSize]);

		Cm::memFetchAlignedAsync((Cm::MemFetchPtr)hfData, (Cm::MemFetchPtr)alignedAddress, alignedSize, 1);

		Gu::HeightFieldData* hf = (Gu::HeightFieldData*)&hfData[offset];

		Cm::memFetchWait(1);

		if(hfGeom.materials.numIndices > 128)
			PX_ASSERT(0);

		CELL_ALIGN(16, PxU16 indices[128]);

		if(((PxU32)hfGeom.materials.indices & 0xf) != 0)
			PX_ASSERT(0);

		const PxU32 alignedIndSize = ((sizeof(PxU16) * hfGeom.materials.numIndices) + 15) & 0xfffffff0;

		Cm::memFetchAlignedAsync((Cm::MemFetchPtr)(&indices), (Cm::MemFetchPtr)hfGeom.materials.indices, alignedIndSize, 1);
		Cm::memFetchWait(1);

		materialIndices = indices;
#else
		const Gu::HeightFieldData* hf = hfGeom.heightFieldData;
	#endif
		
		for(PxU32 i=0; i< contactBuffer.count; ++i)
		{
			Gu::ContactPoint& contact = contactBuffer.contacts[i];
			materialInfo[i].mMaterialIndex0 = shape0->materialIndex;
			//contact.featureIndex0 = shape0->materialIndex;
			const PxU32 localMaterialIndex = GetMaterialIndex(hf, contact.internalFaceIndex1);
			//contact.featureIndex1 = materialIndices[localMaterialIndex];
			PX_ASSERT(localMaterialIndex<hfGeom.materials.numIndices);
			materialInfo[i].mMaterialIndex1 = materialIndices[localMaterialIndex];
		}
	}
	
	return true;
}

