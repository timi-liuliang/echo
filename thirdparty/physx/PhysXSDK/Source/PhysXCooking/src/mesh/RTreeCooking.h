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

#include "GuRTree.h"
#include "PxCooking.h"

namespace physx
{
	struct RTreeCooker
	{
		struct RemapCallback // a callback to convert indices from triangle to LeafTriangles or other uses
		{
            virtual ~RemapCallback() {}
			virtual void remap(PxU32* rtreePtr, PxU32 start, PxU32 leafCount) = 0;
		};

		// triangles will be remapped so that newIndex = resultPermute[oldIndex]
		static void buildFromTriangles(
			Gu::RTree& resultTree, const PxVec3* verts, PxU32 numVerts, const PxU16* tris16, const PxU32* tris32, PxU32 numTris,
			Array<PxU32>& resultPermute, RemapCallback* rc, PxReal sizePerfTradeOff01, PxMeshCookingHint::Enum hint);
	};
}
