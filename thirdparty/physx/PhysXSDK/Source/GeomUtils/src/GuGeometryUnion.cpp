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



#include "GuGeometryUnion.h"

#include "GuConvexMesh.h"
#include "GuTriangleMesh.h"
#include "GuHeightField.h"
#include "GuHeightFieldUtil.h"

namespace physx
{
namespace Gu
{
	PX_FORCE_INLINE Gu::ConvexMesh& getConvexMesh(PxConvexMesh* nxcm)
	{ 
		Gu::ConvexMesh* npcm = static_cast<Gu::ConvexMesh*>(nxcm);
		return *npcm;
	}

	PX_FORCE_INLINE Gu::TriangleMesh& getTriangleMesh(PxTriangleMesh* nxtm)
	{
		Gu::TriangleMesh* nptm = static_cast<Gu::TriangleMesh*>(nxtm);
		return *nptm;
	}

	PX_FORCE_INLINE Gu::HeightField& getHeightField(PxHeightField* nxtm)
	{
		Gu::HeightField* nptm = static_cast<Gu::HeightField*>(nxtm);
		return *nptm;
	}
}

void Gu::GeometryUnion::set(const PxGeometry& g)
{
	switch(g.getType())
	{
	case PxGeometryType::eBOX :
		reinterpret_cast<PxBoxGeometry&>(geometry) = static_cast<const PxBoxGeometry&>(g);
		break;
	case PxGeometryType::eCAPSULE :
		reinterpret_cast<PxCapsuleGeometry&>(geometry) = static_cast<const PxCapsuleGeometry&>(g);
		break;
	case PxGeometryType::eSPHERE :
		reinterpret_cast<PxSphereGeometry&>(geometry) = static_cast<const PxSphereGeometry&>(g);
		reinterpret_cast<PxCapsuleGeometry&>(geometry).halfHeight = 0.0f;		//AM: make sphere geometry also castable as a zero height capsule.
		break;
	case PxGeometryType::ePLANE :
		reinterpret_cast<PxPlaneGeometry&>(geometry) = static_cast<const PxPlaneGeometry&>(g);
		break;
	case PxGeometryType::eCONVEXMESH :
		{
			reinterpret_cast<PxConvexMeshGeometry&>(geometry) = static_cast<const PxConvexMeshGeometry&>(g);
			reinterpret_cast<PxConvexMeshGeometryLL&>(geometry).hullData = &(Gu::getConvexMesh(get<PxConvexMeshGeometryLL>().convexMesh).getHull());
		}
		break;
	case PxGeometryType::eTRIANGLEMESH :
		{
			reinterpret_cast<PxTriangleMeshGeometry&>(geometry) = static_cast<const PxTriangleMeshGeometry&>(g);
			reinterpret_cast<PxTriangleMeshGeometryLL&>(geometry).meshData = &(Gu::getTriangleMesh(get<PxTriangleMeshGeometryLL>().triangleMesh).mMesh.mData);
			reinterpret_cast<PxTriangleMeshGeometryLL&>(geometry).materialIndices = (Gu::getTriangleMesh(get<PxTriangleMeshGeometryLL>().triangleMesh).mMesh.getMaterials());
			reinterpret_cast<PxTriangleMeshGeometryLL&>(geometry).materials = MaterialIndicesStruct();
		}
		break;
	case PxGeometryType::eHEIGHTFIELD :
		{
			reinterpret_cast<PxHeightFieldGeometry&>(geometry) = static_cast<const PxHeightFieldGeometry&>(g);
			reinterpret_cast<PxHeightFieldGeometryLL&>(geometry).heightFieldData = &Gu::getHeightField(get<PxHeightFieldGeometryLL>().heightField).getData();
			reinterpret_cast<PxHeightFieldGeometryLL&>(geometry).materials = MaterialIndicesStruct();
		}
		break;
	case PxGeometryType::eGEOMETRY_COUNT:
	case PxGeometryType::eINVALID:
	default :
		PX_ALWAYS_ASSERT_MESSAGE("geometry type not handled");
		break;
	}
}

}
