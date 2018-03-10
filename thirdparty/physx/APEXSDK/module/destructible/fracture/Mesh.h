/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "RTdef.h"
#if RT_COMPILE
#ifndef MESH
#define MESH

#include <foundation/PxVec3.h>
#include <PsArray.h>
#include <PsAllocator.h>

#include "MeshBase.h"

namespace physx
{
namespace apex
{
	class NxRenderMeshAsset;
}
}

namespace physx
{
namespace fracture
{

using namespace ::physx::shdfnd;

class Mesh : public base::Mesh
{
public:
	void loadFromRenderMesh(const apex::NxRenderMeshAsset& mesh, PxU32 partIndex);
protected:
	static void gatherPartMesh(Array<physx::PxVec3>& vertices, Array<physx::PxU32>&  indices, Array<physx::PxVec3>& normals,
		Array<physx::PxVec2>& texcoords, shdfnd::Array<SubMesh>& subMeshes, const apex::NxRenderMeshAsset& renderMeshAsset, physx::PxU32 partIndex);
};

}
}

#endif
#endif