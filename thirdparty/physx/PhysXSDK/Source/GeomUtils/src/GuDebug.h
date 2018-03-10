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

#ifndef GU_DEBUG_H
#define GU_DEBUG_H

#include "CmRenderOutput.h"
#include "CmPhysXCommon.h"

namespace physx
{

class PxBoxGeometry;
class PxSphereGeometry;
class PxPlaneGeometry;
class PxCapsuleGeometry;
class PxConvexMeshGeometry;
class PxTriangleMeshGeometry;
class PxHeightFieldGeometry;

#if PX_ENABLE_DEBUG_VISUALIZATION

namespace Cm
{
	class Matrix34;
}

namespace Sc
{
	class ShapeCore;
}

namespace Gu
{
	class Debug
	{

	public :

		PX_PHYSX_COMMON_API static PxMat44 convertToPxMat44(const Cm::Matrix34& absPose);
		PX_PHYSX_COMMON_API static PxMat44 convertToPxMat44(const PxTransform& tr);

		PX_PHYSX_COMMON_API static void visualize(const PxBoxGeometry& geometry,
			Cm::RenderOutput& out, const Cm::Matrix34& absPose, const PxBounds3& cullbox,
			const PxU64 mask, const PxReal fscale, const PxU32 numMaterials);

		PX_PHYSX_COMMON_API static void visualize(const PxSphereGeometry& geometry,
			Cm::RenderOutput& out, const Cm::Matrix34& absPose, const PxBounds3& cullbox,
			const PxU64 mask, const PxReal fscale, const PxU32 numMaterials);

		PX_PHYSX_COMMON_API static void visualize(const PxPlaneGeometry& geometry,
			Cm::RenderOutput& out, const Cm::Matrix34& absPose, const PxBounds3& cullbox,
			const PxU64 mask, const PxReal fscale, const PxU32 numMaterials);

		PX_PHYSX_COMMON_API static void visualize(const PxCapsuleGeometry& geometry,
			Cm::RenderOutput& out, const Cm::Matrix34& absPose, const PxBounds3& cullbox,
			const PxU64 mask, const PxReal fscale, const PxU32 numMaterials);

		PX_PHYSX_COMMON_API static void visualize(const PxConvexMeshGeometry& geometry,
			Cm::RenderOutput& out, const Cm::Matrix34& absPose, const PxBounds3& cullbox,
			const PxU64 mask, const PxReal fscale, const PxU32 numMaterials);

		PX_PHYSX_COMMON_API static void visualize(const PxTriangleMeshGeometry& geometry,
			Cm::RenderOutput& out, const Cm::Matrix34& absPose, const PxBounds3& cullbox,
			const PxU64 mask, const PxReal fscale, const PxU32 numMaterials);

		PX_PHYSX_COMMON_API static void visualize(const PxHeightFieldGeometry& geometry,
			Cm::RenderOutput& out, const Cm::Matrix34& absPose, const PxBounds3& cullbox,
			const PxU64 mask, const PxReal fscale,  const PxU32 numMaterials);
	};
}

#endif

}

#endif
