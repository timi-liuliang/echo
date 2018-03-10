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
#include "GuTriangleMesh.h"

using namespace physx;
using namespace Gu;

namespace physx
{
	bool PxcGetMaterialShapeMesh(const PxsShapeCore* shape0, const PxsShapeCore* shape1, PxcNpThreadContext& context, PxsMaterialInfo* materialInfo);
	bool PxcGetMaterialMesh(const PxsShapeCore* shape, const PxU32 index,  PxcNpThreadContext& context, PxsMaterialInfo* materialInfo);
}

bool physx::PxcGetMaterialMesh(const PxsShapeCore* shape, const PxU32 index, PxcNpThreadContext& context, PxsMaterialInfo* materialInfo)
{
	ContactBuffer& contactBuffer = context.mContactBuffer;
	const PxTriangleMeshGeometryLL& shapeMesh = shape->geometry.get<const PxTriangleMeshGeometryLL>();
	if(shapeMesh.materials.numIndices <= 1)
	{
		for(PxU32 i=0; i< contactBuffer.count; ++i) 
		{
			(&materialInfo[i].mMaterialIndex0)[index] = shape->materialIndex;
		}
	}
	else
	{
#ifdef __SPU__
		
		if(shapeMesh.materials.numIndices > 128)
			PX_ASSERT(0);
		if(((PxU32)shapeMesh.materials.indices & 0xf) != 0)
			PX_ASSERT(0);

		CELL_ALIGN(16, PxU16 mMaterialBuffer[8]);
		CELL_ALIGN(16, PxU16 indices[128]);

		PxU32 alignedSize = ((sizeof(PxU16) * shapeMesh.materials.numIndices) + 15) & 0xfffffff0;

		Cm::memFetchAlignedAsync((Cm::MemFetchPtr)(&indices), (Cm::MemFetchPtr)shapeMesh.materials.indices, alignedSize, 1);
		Cm::memFetchWait(1);
#endif
		for(PxU32 i=0; i< contactBuffer.count; ++i)
		{

			Gu::ContactPoint& contact = contactBuffer.contacts[i];
			const PxU16* eaMaterialIndices = shapeMesh.materialIndices;
#ifdef	__SPU__
			if(eaMaterialIndices)
			{
				const PxU16* materialIndices = eaMaterialIndices + contact.internalFaceIndex1;
				uintptr_t alignedMaterialAddress = uint64_t(materialIndices) & 0xfffffff0;
				uintptr_t offset = (uintptr_t)materialIndices - alignedMaterialAddress;

				Cm::memFetchAlignedAsync((Cm::MemFetchPtr)(&mMaterialBuffer), (Cm::MemFetchPtr)alignedMaterialAddress, 16, 1);
				Cm::memFetchWait(1);
				PxU32 localMaterialIndex = mMaterialBuffer[offset >> 1];
				materialInfo[i].mMaterialIndex1 = indices[localMaterialIndex];

			}
#else
			const PxU32 localMaterialIndex = eaMaterialIndices[(&contact.internalFaceIndex0)[index]];//shapeMesh.triangleMesh->getTriangleMaterialIndex(contact.featureIndex1);
			(&materialInfo[i].mMaterialIndex0)[index] = shapeMesh.materials.indices[localMaterialIndex];
#endif
		}

	}
	return true;
}

bool physx::PxcGetMaterialShapeMesh(const PxsShapeCore* shape0, const PxsShapeCore* shape1, PxcNpThreadContext& context, PxsMaterialInfo* materialInfo)
{
	
	ContactBuffer& contactBuffer = context.mContactBuffer;
	const PxTriangleMeshGeometryLL& shapeMesh = shape1->geometry.get<const PxTriangleMeshGeometryLL>();
//	const Gu::InternalTriangleMeshData* meshData = shapeMesh.meshData;
	if(shapeMesh.materials.numIndices <= 1)
	{
		for(PxU32 i=0; i< contactBuffer.count; ++i) 
		{
			materialInfo[i].mMaterialIndex0 = shape0->materialIndex;
			materialInfo[i].mMaterialIndex1 = shape1->materialIndex;
		}
	}
	else
	{
#ifdef __SPU__
		
		if(shapeMesh.materials.numIndices > 128)
			PX_ASSERT(0);
		if(((PxU32)shapeMesh.materials.indices & 0xf) != 0)
			PX_ASSERT(0);

		CELL_ALIGN(16, PxU16 mMaterialBuffer[8]);
		CELL_ALIGN(16, PxU16 indices[128]);

		PxU32 alignedSize = ((sizeof(PxU16) * shapeMesh.materials.numIndices) + 15) & 0xfffffff0;
		Cm::memFetchAlignedAsync((Cm::MemFetchPtr)(&indices), (Cm::MemFetchPtr)shapeMesh.materials.indices, alignedSize, 1);
		Cm::memFetchWait(1);
#endif
		for(PxU32 i=0; i< contactBuffer.count; ++i)
		{

			Gu::ContactPoint& contact = contactBuffer.contacts[i];
			//contact.featureIndex0 = shape0->materialIndex;
			materialInfo[i].mMaterialIndex0 = shape0->materialIndex;
			const PxU16* eaMaterialIndices = shapeMesh.materialIndices;
#ifdef	__SPU__
			if(eaMaterialIndices)
			{
				const PxU16* materialIndices = eaMaterialIndices + contact.internalFaceIndex1;
				uintptr_t alignedMaterialAddress = uint64_t(materialIndices) & 0xfffffff0;
				uintptr_t offset = (uintptr_t)materialIndices - alignedMaterialAddress;

				Cm::memFetchAlignedAsync((Cm::MemFetchPtr)(&mMaterialBuffer), (Cm::MemFetchPtr)alignedMaterialAddress, 16, 1);
				Cm::memFetchWait(1);
				PxU32 localMaterialIndex = mMaterialBuffer[offset >> 1];
				materialInfo[i].mMaterialIndex1 = indices[localMaterialIndex];

			}
#else
			const PxU32 localMaterialIndex = eaMaterialIndices[contact.internalFaceIndex1];//shapeMesh.triangleMesh->getTriangleMaterialIndex(contact.featureIndex1);
			//contact.featureIndex1 = shapeMesh.materials.indices[localMaterialIndex];
			materialInfo[i].mMaterialIndex1 = shapeMesh.materials.indices[localMaterialIndex];
#endif
		}
	}

	return true;
}
