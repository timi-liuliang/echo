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

#ifndef GU_TRIANGLEMESH_DATA_H
#define GU_TRIANGLEMESH_DATA_H

#include "PxVec3.h"
#include "GuHybridModel.h"

namespace physx
{
namespace Gu
{

class RTreeMidphase;
struct EdgeListData;

//Data

PX_ALIGN_PREFIX(16)
struct InternalTriangleMeshData
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================

// PX_SERIALIZATION
						InternalTriangleMeshData()											{}
						InternalTriangleMeshData(const PxEMPTY&) : mCollisionModel(PxEmpty)	{}
//~PX_SERIALIZATION

	// 16 bytes block
						PxU32				mNumVertices;
						PxU32				mNumTriangles;
						PxVec3*				mVertices;
						void*				mTriangles;				//!< 16 (<= 0xffff #vertices) or 32 bit trig indices (mNumTriangles * 3)
	// 16 bytes block
						Gu::RTreeMidphase	mCollisionModel;

	// 16 bytes block
						PxBounds3			mAABB;
						PxU8*				mExtraTrigData;			//one per trig
	/*
	low 3 bits (mask: 7) are the edge flags:
	b001 = 1 = ignore edge 0 = edge v0-->v1
	b010 = 2 = ignore edge 1 = edge v0-->v2
	b100 = 4 = ignore edge 2 = edge v1-->v2
	*/
						PxU8				mFlags;			//!< Flag whether indices are 16 or 32 bits wide.  In cooking we are always using 32 bits, otherwise we could tell from the number of vertices.
															//!< Flag whether triangle adajacencies are build

} PX_ALIGN_SUFFIX(16);
PX_COMPILE_TIME_ASSERT((sizeof(Gu::InternalTriangleMeshData)&15) == 0);
PX_COMPILE_TIME_ASSERT((PX_OFFSET_OF(Gu::InternalTriangleMeshData, mCollisionModel)&15) == 0);

} // namespace Gu

}

#endif
