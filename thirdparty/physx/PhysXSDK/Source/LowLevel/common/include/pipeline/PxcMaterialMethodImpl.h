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



#ifndef PXC_MATERIALMETHOD_H
#define PXC_MATERIALMETHOD_H
#include "CmPhysXCommon.h"

namespace physx
{

struct PxsShapeCore;
struct PxsMaterialInfo;
class PxcNpThreadContext;

#define MATERIAL_METHOD_ARGS			\
	const PxsShapeCore* shape0,			\
	const PxsShapeCore* shape1,			\
	PxcNpThreadContext& pairContext,	\
	PxsMaterialInfo* materialInfo


#define SINGLE_MATERIAL_METHOD_ARGS			\
	const PxsShapeCore* shape,				\
	const PxU32	index,						\
	PxcNpThreadContext& pairContext,		\
	PxsMaterialInfo* materialInfo			

/*!
Method prototype for fetch material routines
*/
typedef bool (*PxcGetMaterialMethod) (MATERIAL_METHOD_ARGS);

typedef bool (*PxcGetSingleMaterialMethod) (SINGLE_MATERIAL_METHOD_ARGS);

extern PxcGetMaterialMethod g_GetMaterialMethodTable[][7];

extern PxcGetSingleMaterialMethod g_GetSingleMaterialMethodTable[7];

}

#endif
