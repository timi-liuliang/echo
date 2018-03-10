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

using namespace physx;
using namespace Gu;

namespace physx
{
bool PxcGetMaterialShape(const PxsShapeCore* shape, const PxU32 index, PxcNpThreadContext& context, PxsMaterialInfo* materialInfo)
{
	ContactBuffer& contactBuffer = context.mContactBuffer;
	for(PxU32 i=0; i< contactBuffer.count; ++i)
	{
		(&materialInfo[i].mMaterialIndex0)[index] = shape->materialIndex;
	}
	return true;
}

bool PxcGetMaterialShapeShape(const PxsShapeCore* shape0, const PxsShapeCore* shape1, PxcNpThreadContext& context,  PxsMaterialInfo* materialInfo)
{
	ContactBuffer& contactBuffer = context.mContactBuffer;
	for(PxU32 i=0; i< contactBuffer.count; ++i)
	{
		materialInfo[i].mMaterialIndex0 = shape0->materialIndex;
		materialInfo[i].mMaterialIndex1 = shape1->materialIndex;
	}
	return true;
}
}

