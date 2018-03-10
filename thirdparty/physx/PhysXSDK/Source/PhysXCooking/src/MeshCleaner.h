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

#ifndef PX_MESH_CLEANER_H
#define PX_MESH_CLEANER_H

#include "Px.h"

#ifndef PX_COOKING
#error Do not include anymore!
#endif

namespace physx
{
	class MeshCleaner
	{
		public:
			MeshCleaner(PxU32 nbVerts, const PxVec3* verts, PxU32 nbTris, const PxU32* indices, PxF32 meshWeldTolerance);
			~MeshCleaner();

			PxU32	mNbVerts;
			PxU32	mNbTris;
			PxVec3*	mVerts;
			PxU32*	mIndices;
			PxU32*	mRemap;
	};
}

#endif // PX_MESH_CLEANER_H
