/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_CLOTHING_ISO_MESH_H
#define NX_CLOTHING_ISO_MESH_H

#include "NxApexInterface.h"
#include "NxApexUserProgress.h"

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

class NxClothingPhysicalMesh;

/**
\brief This class represents an ISO surface. This is the intermediate step for multi-layered clothing.
*/
class NxClothingIsoMesh : public NxApexInterface
{
public:
	/**
	\brief The numer of vertices in the ISO surface
	*/
	virtual physx::PxU32 getNumVertices() const = 0;

	/**
	\brief The number of indices in the ISO surface
	*/
	virtual physx::PxU32 getNumIndices() const = 0;

	/**
	\brief Write all vertices into this buffer

	\param [out] vertexDestination	For the vertices to be written, must be at least of size byteStride * NxClothingIsoMesh::getNumVertices()
	\param [in] byteStride			The stride of the buffer provided, must be at least sizeof(physx::PxVec3)
	*/
	virtual void getVertices(void* vertexDestination, physx::PxU32 byteStride) = 0;

	/**
	\brief Write all indices into this buffer

	\param [out] indexDestination	For the indices to be written, must be at least the size byteStride * NxClothingIsoMesh::getNumIndices()
	\param [in] byteStride			The stride of the buffer provided, must be at least sizeof(int)
	*/
	virtual void getIndices(void* indexDestination, physx::PxU32 byteStride) = 0;

	/**
	\brief simplifies the iso mesh

	\param [in] subdivisions	used to derive the maximal length a new edge can get.<br>
								Divide the bounding box diagonal by this value to get the maximal edge length for newly created edges<br>
								Use 0 to not restrict the maximal edge length
	\param [in] maxSteps		The maximum number of edges to be considered for simplification.<br>
								Use 0 to turn off
	\param [in] maxError		The maximal quadric error an edge can cause to be considered simplifyable.<br>
								Use any value < 0 to turn off
	\param [in] progress		Callback class that will be fired every now and then to update a progress bar in the gui
	\return The number of edges collapsed
	*/
	virtual physx::PxU32 simplify(physx::PxU32 subdivisions, physx::PxI32 maxSteps, physx::PxF32 maxError, IProgressListener* progress) = 0;

	/**
	\brief contracts the iso mesh according to a signed distance field

	\param [in] steps			The maximum number of contraction iterations performed, 0 to turn off.
	\param [in] abortionRatio	Compares the contracted volume to the original volume of the iso mesh, early outs if ratio is below
								this value. Good values are around 0.01.
	\param [in] expand			If set to true, the border vertices are expanded such that they approximate the iso mesh better and
								the contraction is only executed in the normal direction of the surface.
	\param [in] progress		Callback class that will be fired every now and then to update a progress bar in the gui
	\return The number of contraction steps executed
	*/
	virtual physx::PxF32 contract(physx::PxI32 steps, physx::PxF32 abortionRatio, bool expand, IProgressListener* progress) = 0;

	/**
	\brief generate the physical mesh (cloth) (this requires the iso mesh to be 'well' contracted

	\param [in] bubbleSizeToRemove	Bubbles are parts of a mesh where it is not manifold. if one side of a bubble has less faces than
									this value, that one side gets removed.
	\param [in] progress			Callback class that will be fired every now and then to update a progress bar in the gui
	*/
	virtual NxClothingPhysicalMesh* generateClothMesh(physx::PxU32 bubbleSizeToRemove, IProgressListener* progress) = 0;

	/**
	\brief generate the physical mesh (softbody) (works on less or not contracted iso meshes)

	\param [in] tetraSubdivision	scale to the bound diagonal to derive values for vertex welding etc.
	\param [in] progress			Callback class that will be fired every now and then to update a progress bar in the gui
	*/
	virtual NxClothingPhysicalMesh* generateSoftbodyMesh(physx::PxU32 tetraSubdivision, IProgressListener* progress) = 0;
};


PX_POP_PACK

}
} // namespace physx::apex

#endif // NX_CLOTHING_ISO_MESH_H
