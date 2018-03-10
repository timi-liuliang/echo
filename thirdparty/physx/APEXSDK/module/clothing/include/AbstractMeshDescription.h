/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_ABSTRACT_MESH_DESCRIPTION_H
#define NX_ABSTRACT_MESH_DESCRIPTION_H

#include <NxApexUsingNamespace.h>
#include "foundation/PxVec3.h"

class NxClothConstrainCoefficients;


namespace physx
{
namespace apex
{

class NxApexRenderDebug;
/**
\brief a simplified, temporal container for a mesh with non-interleaved vertex buffers
*/

struct NxAbstractMeshDescription
{
	NxAbstractMeshDescription() : numVertices(0), numIndices(0), numBonesPerVertex(0),
		pPosition(NULL), pNormal(NULL), pTangent(NULL), pTangent4(NULL), pBitangent(NULL),
		pBoneIndices(NULL), pBoneWeights(NULL), pConstraints(NULL), pVertexFlags(NULL), pIndices(NULL),
		avgEdgeLength(0.0f), avgTriangleArea(0.0f), pMin(0.0f), pMax(0.0f), centroid(0.0f), radius(0.0f) {}

	/// the number of vertices in the mesh
	PxU32	numVertices;
	/// the number of indices in the mesh
	PxU32	numIndices;
	/// the number of bones per vertex in the boneIndex and boneWeights buffer. Can be 0
	PxU32	numBonesPerVertex;

	/// pointer to the positions array
	PxVec3*	PX_RESTRICT pPosition;
	/// pointer to the normals array
	PxVec3*	PX_RESTRICT pNormal;
	/// pointer to the tangents array
	PxVec3*	PX_RESTRICT pTangent;
	/// alternative pointer to the tangents array, with float4
	PxVec4*	PX_RESTRICT pTangent4;
	/// pointer to the bitangents/binormal array
	PxVec3*	PX_RESTRICT pBitangent;
	/// pointer to the bone indices array
	PxU16*	PX_RESTRICT pBoneIndices;
	/// pointer to the bone weights array
	PxF32*	PX_RESTRICT pBoneWeights;
	/// pointer to the cloth constraints array
	NxClothConstrainCoefficients* PX_RESTRICT pConstraints;
	/// pointer to per-vertex flags
	PxU32*	PX_RESTRICT pVertexFlags;
	/// pointer to the indices array
	PxU32*	PX_RESTRICT pIndices;

	/// updates the derived data
	void UpdateDerivedInformation(NxApexRenderDebug* renderDebug);

	/// Derived Data, average Edge Length
	PxF32	avgEdgeLength;
	/// Derived Data, average Triangle Area
	PxF32	avgTriangleArea;
	/// Derived Data, Bounding Box min value
	PxVec3	pMin;
	/// Derived Data, Bounding Box max value
	PxVec3	pMax;
	/// Derived Data, Average of pMin and pMax
	PxVec3	centroid;
	/// Derived Data, Half the distance between pMin and pMax
	PxF32	radius;
};

}
}


#endif //_NX_ABSTRACT_MESH_DESCRIPTION_H
