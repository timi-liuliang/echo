/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "NxApexDefs.h"
#include "MinPhysxSdkVersion.h"
#if NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED

#include "ClothingPhysicalMesh.h"
#include "ApexMeshHash.h"
#include "ApexQuadricSimplifier.h"
#include "ClothingAssetAuthoring.h"
#include "ModuleClothing.h"

#include "ApexPermute.h"
#include "ApexSharedUtils.h"

#include "foundation/PxStrideIterator.h"
#include "PsSort.h"

namespace physx
{
namespace apex
{
namespace clothing
{

struct SortedEdge
{
	SortedEdge(PxU32 _i0, PxU32 _i1) : i0(PxMin(_i0, _i1)), i1(PxMax(_i0, _i1)) {}

	bool operator==(const SortedEdge& other) const
	{
		return i0 == other.i0 && i1 == other.i1;
	}
	bool operator()(const SortedEdge& s1, const SortedEdge& s2) const
	{
		if (s1.i0 != s2.i0)
		{
			return s1.i0 < s2.i0;
		}

		return s1.i1 < s2.i1;
	}

	PxU32 i0, i1;
};

ClothingPhysicalMesh::ClothingPhysicalMesh(ModuleClothing* module, ClothingPhysicalMeshParameters* params, NxResourceList* list) :
	mModule(module),
	mParams(NULL),
	ownsParams(false),
	mSimplifier(NULL),
	isDirty(false)
{
	if (params != NULL && strcmp(params->className(), ClothingPhysicalMeshParameters::staticClassName()) != 0)
	{
		APEX_INTERNAL_ERROR(
		    "The parameterized interface is of type <%s> instead of <%s>.  "
		    "An empty ClothingPhhsicalMesh has been created instead.",
		    params->className(),
		    ClothingPhysicalMeshParameters::staticClassName());

		params = NULL;
	}

	if (params == NULL)
	{
		params = DYNAMIC_CAST(ClothingPhysicalMeshParameters*)(NiGetApexSDK()->getParameterizedTraits()->createNxParameterized(ClothingPhysicalMeshParameters::staticClassName()));
		ownsParams = true;
	}
	PX_ASSERT(params != NULL);

	mParams = params;
	mVertices.init(mParams, "physicalMesh.vertices", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->physicalMesh.vertices));
	mNormals.init(mParams, "physicalMesh.normals", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->physicalMesh.normals));
	mSkinningNormals.init(mParams, "physicalMesh.skinningNormals", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->physicalMesh.skinningNormals));
	mConstrainCoefficients.init(mParams, "physicalMesh.constrainCoefficients", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->physicalMesh.constrainCoefficients));
	mBoneIndices.init(mParams, "physicalMesh.boneIndices", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->physicalMesh.boneIndices));
	mBoneWeights.init(mParams, "physicalMesh.boneWeights", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->physicalMesh.boneWeights));
	mIndices.init(mParams, "physicalMesh.indices", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->physicalMesh.indices));

	mParams->referenceCount++;

	mParams->setSerializationCallback(this);
#if 0
	// debugging only
	char buf[32];
	sprintf_s(buf, 32, "++ %p -> %d\n", mParams, mParams->referenceCount);
	OutputDebugString(buf);
#endif

	if (mParams->physicalMesh.shortestEdgeLength == 0.0f)
	{
		computeEdgeLengths();
	}

	list->add(*this);
}



void ClothingPhysicalMesh::release()
{
	PX_ASSERT(mParams != NULL);
	if (mParams != NULL)
	{
		// make sure everything is set up correctly before we let the param object live on its own
		preSerialize(NULL);

		mParams->setSerializationCallback(NULL);
	}
	mModule->releasePhysicalMesh(this);
}



void ClothingPhysicalMesh::destroy()
{
	if (mSimplifier != NULL)
	{
		delete mSimplifier;
		mSimplifier = NULL;
	}

	if (mParams != NULL)
	{
		mParams->referenceCount--;
#if 0
		// debugging only
		char buf[32];
		sprintf_s(buf, 32, "-- %p -> %d\n", mParams, mParams->referenceCount);
		OutputDebugString(buf);
#endif
	}

	if (ownsParams && mParams)
	{
		PX_ASSERT(mParams->referenceCount == 0);
		mParams->destroy();
	}

	delete this;
}



void ClothingPhysicalMesh::makeCopy(ClothingPhysicalMeshParameters* params)
{
	PX_ASSERT(mParams != NULL);
	params->copy(*mParams);
}



void ClothingPhysicalMesh::allocateNormalBuffer()
{
	mNormals.resize(mParams->physicalMesh.numVertices);
}



void ClothingPhysicalMesh::allocateSkinningNormalsBuffer()
{
	mSkinningNormals.resize(mParams->physicalMesh.numVertices);
}


void ClothingPhysicalMesh::allocateMasterFlagsBuffer()
{
	PxU32 numVertices = mVertices.size();
	mMasterFlags.resize(numVertices);

	for (PxU32 i = 0; i < numVertices; i++)
	{
		mMasterFlags[i] = 0xffffffff;
	}
}


void ClothingPhysicalMesh::allocateConstrainCoefficientBuffer()
{
	NX_WRITE_ZONE();
	const PxU32 numVertices = mParams->physicalMesh.numVertices;
	mConstrainCoefficients.resize(numVertices);

	for (PxU32 i = 0; i < numVertices; i++)
	{
		mConstrainCoefficients[i].maxDistance = PX_MAX_F32;
		mConstrainCoefficients[i].collisionSphereRadius = PX_MAX_F32;
		mConstrainCoefficients[i].collisionSphereDistance = PX_MAX_F32;
	}
}



void ClothingPhysicalMesh::allocateBoneIndexAndWeightBuffers()
{
	const PxU32 numBonesPerVertex = mParams->physicalMesh.numBonesPerVertex;
	if (numBonesPerVertex == 0)
	{
		APEX_DEBUG_WARNING("Number of bones per vertex is set to 0. Not allocating memory.");
		return;
	}
	const PxU32 numVertices = mParams->physicalMesh.numVertices;

	mBoneIndices.resize(numBonesPerVertex * numVertices);

	// PH: At one point we can start trying to safe this buffer
	//if (numBonesPerVertex > 1)
	{
		mBoneWeights.resize(numBonesPerVertex * numVertices);
	}
}



void ClothingPhysicalMesh::freeAdditionalBuffers()
{

	mNormals.resize(0);
	mSkinningNormals.resize(0);
	mConstrainCoefficients.resize(0);
	mBoneIndices.resize(0);
	mBoneWeights.resize(0);
	mParams->physicalMesh.numBonesPerVertex = 0;
}



PxU32 ClothingPhysicalMesh::getNumVertices() const
{
	NX_READ_ZONE();
	if (mSimplifier != NULL)
	{
		return mSimplifier->getNumVertices() - mSimplifier->getNumDeletedVertices();
	}

	return mParams->physicalMesh.numVertices;
}



PxU32 ClothingPhysicalMesh::getNumIndices() const
{
	NX_READ_ZONE();
	if (mSimplifier != NULL)
	{
		return mSimplifier->getNumTriangles() * 3;
	}

	return mParams->physicalMesh.numIndices;
}



void ClothingPhysicalMesh::getIndices(void* indexDestination, PxU32 byteStride, PxU32 numIndices) const
{
	NX_READ_ZONE();
	numIndices = PxMin(numIndices, mParams->physicalMesh.numIndices);

	if (byteStride == 0)
	{
		byteStride = sizeof(PxU32);
	}

	if (byteStride < sizeof(PxU32))
	{
		APEX_INTERNAL_ERROR("byte stride is too small (%d)", byteStride);
		return;
	}

	const_cast<ClothingPhysicalMesh*>(this)->writeBackData();

	PxU8* destPtr = (PxU8*)indexDestination;
	for (PxU32 i = 0; i < numIndices; i++)
	{
		(PxU32&)(*(destPtr + byteStride * i)) = mIndices[i];
	}
}



void ClothingPhysicalMesh::simplify(PxU32 subdivisions, PxI32 maxSteps, PxF32 maxError, IProgressListener* progressListener)
{
	NX_WRITE_ZONE();
	if (mParams->physicalMesh.isTetrahedralMesh)
	{
		APEX_INVALID_OPERATION("Cannot simplify a tetrahedral mesh");
		return;
	}

	if (mParams->physicalMesh.boneIndices.buf != NULL || mParams->physicalMesh.boneWeights.buf != NULL)
	{
		APEX_INVALID_OPERATION("Cannot simplif a triangle mesh with additional bone data");
		return;
	}


	const PxU32 numVertices = mParams->physicalMesh.numVertices;
	const PxU32 numIndices = mParams->physicalMesh.numIndices;

	if (numVertices == 0 || numIndices == 0)
	{
		return;
	}

	HierarchicalProgressListener progress(100, progressListener);

	if (mSimplifier == NULL)
	{
		progress.setSubtaskWork(80, "Init simplificator");
		mSimplifier = PX_NEW(ApexQuadricSimplifier);

		for (PxU32 i = 0; i < numVertices; i++)
		{
			mSimplifier->registerVertex(mVertices[i]);
		}

		for (PxU32 i = 0; i < numIndices; i += 3)
		{
			mSimplifier->registerTriangle(mIndices[i + 0], mIndices[i + 1], mIndices[i + 2]);
		}

		mSimplifier->endRegistration(false, &progress);
		progress.completeSubtask();
	}

	progress.setSubtaskWork(-1, "Simplification steps");

	PxU32 steps = mSimplifier->simplify(subdivisions, maxSteps, maxError, &progress);

	if (!isDirty)
	{
		isDirty = steps > 0;
	}

	progress.completeSubtask();
}



void ClothingPhysicalMesh::setGeometry(bool tetraMesh, PxU32 numVertices, PxU32 vertexByteStride, const void* vertices, const PxU32* masterFlags, PxU32 numIndices, PxU32 indexByteStride, const void* indices)
{
	NX_WRITE_ZONE();
	if (vertexByteStride < sizeof(PxVec3))
	{
		APEX_INTERNAL_ERROR("vertexByteStride is too small (%d)", vertexByteStride);
		return;
	}

	if (indexByteStride < sizeof(PxU32))
	{
		APEX_INTERNAL_ERROR("indexByteStride is too small (%d)", indexByteStride);
		return;
	}

	if (numVertices > 0 && vertices == NULL)
	{
		APEX_INTERNAL_ERROR("vertex pointer is NULL");
		return;
	}

	if (numIndices > 0 && indices == NULL)
	{
		APEX_INTERNAL_ERROR("index pointer is NULL");
		return;
	}

	if (tetraMesh && (numIndices % 4 != 0))
	{
		APEX_INTERNAL_ERROR("Indices must be a multiple of 4 for physical tetrahedral meshes");
		return;
	}

	if (!tetraMesh && (numIndices % 3 != 0))
	{
		APEX_INTERNAL_ERROR("Indices must be a multiple of 3 for physical meshes");
		return;
	}

	mParams->physicalMesh.isTetrahedralMesh = tetraMesh;

	mParams->physicalMesh.numVertices = numVertices;

	mVertices.resize(numVertices);
	mMasterFlags.resize(numVertices);

	mNormals.resize(0);
	mSkinningNormals.resize(0);

	const PxU8* srcVertices = (const PxU8*)vertices;
	for (PxU32 i = 0; i < numVertices; i++)
	{
		const PxVec3& currVec = *(const PxVec3*)(srcVertices + vertexByteStride * i);
		mVertices[i] = currVec;

		mMasterFlags[i] = masterFlags != NULL ? masterFlags[i] : 0xffffffff;
	}


	if (tetraMesh || !removeDuplicatedTriangles(numIndices, indexByteStride, indices))
	{
		mParams->physicalMesh.numIndices = numIndices;
		mIndices.resize(numIndices);

		const PxU8* srcIndices = (const PxU8*)indices;
		for (PxU32 i = 0; i < numIndices; i++)
		{
			const PxU32 currIndex = *(const PxU32*)(srcIndices + indexByteStride * i);
			mIndices[i] = currIndex;
		}
	}

	if (mSimplifier != NULL)
	{
		delete mSimplifier;
		mSimplifier = NULL;
	}

	clearMiscBuffers();
	computeEdgeLengths();

	if (!mParams->physicalMesh.isTetrahedralMesh)
	{
		mSkinningNormals.resize(numVertices);
		updateSkinningNormals();
	}
}



bool ClothingPhysicalMesh::getIndices(PxU32* indices, PxU32 byteStride) const
{
	NX_READ_ZONE();
	if (mIndices.size() == 0)
	{
		return false;
	}

	if (byteStride == 0)
	{
		byteStride = sizeof(PxU32);
	}

	if (byteStride < sizeof(PxU32))
	{
		APEX_INTERNAL_ERROR("bytestride too small (%d)", byteStride);
		return false;
	}

	const_cast<ClothingPhysicalMesh*>(this)->writeBackData();

	const PxU32 numIndices = mParams->physicalMesh.numIndices;

	PxStrideIterator<PxU32> iterator(indices, byteStride);
	for (PxU32 i = 0; i < numIndices; i++, ++iterator)
	{
		*iterator = mIndices[i];
	}

	return true;
}



bool ClothingPhysicalMesh::getVertices(PxVec3* vertices, PxU32 byteStride) const
{
	NX_READ_ZONE();
	if (mVertices.size() == 0)
	{
		return false;
	}

	if (byteStride == 0)
	{
		byteStride = sizeof(PxVec3);
	}

	if (byteStride < sizeof(PxVec3))
	{
		APEX_INTERNAL_ERROR("bytestride too small (%d)", byteStride);
		return false;
	}

	const_cast<ClothingPhysicalMesh*>(this)->writeBackData();

	const PxU32 numVertices = mParams->physicalMesh.numVertices;

	PxStrideIterator<PxVec3> iterator(vertices, byteStride);
	for (PxU32 i = 0; i < numVertices; i++, ++iterator)
	{
		*iterator = mVertices[i];
	}

	return true;
}



bool ClothingPhysicalMesh::getNormals(PxVec3* normals, PxU32 byteStride) const
{
	NX_READ_ZONE();
	if (mNormals.size() == 0)
	{
		return false;
	}

	if (byteStride == 0)
	{
		byteStride = sizeof(PxVec3);
	}

	if (byteStride < sizeof(PxVec3))
	{
		APEX_INTERNAL_ERROR("bytestride too small (%d)", byteStride);
		return false;
	}

	const_cast<ClothingPhysicalMesh*>(this)->writeBackData();

	const PxU32 numVertices = mParams->physicalMesh.numVertices;

	PxStrideIterator<PxVec3> iterator(normals, byteStride);
	for (PxU32 i = 0; i < numVertices; i++, ++iterator)
	{
		*iterator = mNormals[i];
	}

	return true;
}



bool ClothingPhysicalMesh::getBoneIndices(PxU16* boneIndices, PxU32 byteStride) const
{
	NX_READ_ZONE();
	if (mBoneIndices.size() == 0)
	{
		return false;
	}

	const PxU32 numBonesPerVertex = mParams->physicalMesh.numBonesPerVertex;
	if (byteStride == 0)
	{
		byteStride = numBonesPerVertex * sizeof(PxU16);
	}

	if (byteStride < numBonesPerVertex * sizeof(PxU16))
	{
		APEX_INTERNAL_ERROR("bytestride too small (%d)", byteStride);
		return false;
	}

	const PxU32 numElements = mParams->physicalMesh.numVertices * numBonesPerVertex;

	PxStrideIterator<PxU16> iterator(boneIndices, byteStride);
	for (PxU32 i = 0; i < numElements; i += numBonesPerVertex, ++iterator)
	{
		for (PxU32 j = 0; j < numBonesPerVertex; j++)
		{
			PxU16* current = iterator.ptr();
			current[j] = mBoneIndices[i + j];
		}
	}

	return true;
}



bool ClothingPhysicalMesh::getBoneWeights(PxF32* boneWeights, PxU32 byteStride) const
{
	NX_READ_ZONE();
	if (mBoneWeights.size() == 0)
	{
		return false;
	}

	const PxU32 numBonesPerVertex = mParams->physicalMesh.numBonesPerVertex;
	if (byteStride == 0)
	{
		byteStride = numBonesPerVertex * sizeof(PxF32);
	}

	if (byteStride < numBonesPerVertex * sizeof(PxF32))
	{
		APEX_INTERNAL_ERROR("bytestride too small (%d)", byteStride);
		return false;
	}

	const_cast<ClothingPhysicalMesh*>(this)->writeBackData();

	const PxU32 numElements = mParams->physicalMesh.numVertices * numBonesPerVertex;

	PxStrideIterator<PxF32> iterator(boneWeights, byteStride);
	for (PxU32 i = 0; i < numElements; i += numBonesPerVertex, ++iterator)
	{
		for (PxU32 j = 0; j < numBonesPerVertex; j++)
		{
			PxF32* current = iterator.ptr();
			current[j] = mBoneWeights[i + j];
		}
	}


	return true;
}



bool ClothingPhysicalMesh::getConstrainCoefficients(NxClothingConstrainCoefficients* coeffs, PxU32 byteStride) const
{
	NX_READ_ZONE();
	if (mConstrainCoefficients.size() == 0)
	{
		return false;
	}

	if (byteStride == 0)
	{
		byteStride = sizeof(NxClothingConstrainCoefficients);
	}

	if (byteStride < sizeof(NxClothingConstrainCoefficients))
	{
		APEX_INTERNAL_ERROR("bytestride too small (%d)", byteStride);
		return false;
	}

	const_cast<ClothingPhysicalMesh*>(this)->writeBackData();

	const PxU32 numVertices = mParams->physicalMesh.numVertices;

	PxStrideIterator<NxClothingConstrainCoefficients> iterator(coeffs, byteStride);
	for (PxU32 i = 0; i < numVertices; i++, ++iterator)
	{
		iterator->maxDistance = mConstrainCoefficients[i].maxDistance;
		iterator->collisionSphereDistance = mConstrainCoefficients[i].collisionSphereDistance;
		iterator->collisionSphereRadius = mConstrainCoefficients[i].collisionSphereRadius;
	}

	return true;
}



void ClothingPhysicalMesh::getStats(NxClothingPhysicalMeshStats& stats) const
{
	NX_READ_ZONE();
	memset(&stats, 0, sizeof(NxClothingPhysicalMeshStats));

	stats.totalBytes = sizeof(ClothingPhysicalMesh);

	/*

	stats.numVertices = mNumVertices;
	stats.numIndices = mNumIndices;

	stats.totalBytes += (mVertices != NULL ? mNumVertices : 0) * sizeof(PxVec3);
	stats.totalBytes += (mNormals != NULL ? mNumVertices : 0) * sizeof(PxVec3);
	stats.totalBytes += (mSkinningNormals != NULL ? mNumVertices : 0) * sizeof(PxVec3);
	stats.totalBytes += (mVertexFlags != NULL ? mNumVertices : 0) * sizeof(PxU32);
	stats.totalBytes += (mConstrainCoefficients != NULL ? mNumVertices : 0) * sizeof(NxClothConstrainCoefficients);

	stats.totalBytes += (mBoneIndices != NULL ? mNumVertices * mNumBonesPerVertex : 0) * sizeof(PxU16);
	stats.totalBytes += (mBoneWeights != NULL ? mNumVertices * mNumBonesPerVertex : 0) * sizeof(PxF32);

	stats.totalBytes += (mIndices != NULL ? mNumIndices : 0) * sizeof(PxU32);
	*/
}



void ClothingPhysicalMesh::writeBackData()
{
	if (!isDirty || mSimplifier == NULL)
	{
		return;
	}

	isDirty = false;

	Array<PxI32> old2new(mSimplifier->getNumVertices());

	PX_ASSERT(mSimplifier->getNumVertices() - mSimplifier->getNumDeletedVertices() < mParams->physicalMesh.numVertices);

	PxU32 verticesWritten = 0;
	for (PxU32 i = 0; i < mSimplifier->getNumVertices(); i++)
	{
		PxVec3 pos;
		if (mSimplifier->getVertexPosition(i, pos))
		{
			old2new[i] = (physx::PxI32)verticesWritten;
			mVertices[verticesWritten++] = pos;
		}
		else
		{
			old2new[i] = -1;
		}
	}
	PX_ASSERT(verticesWritten == (mSimplifier->getNumVertices() - mSimplifier->getNumDeletedVertices()));
	mParams->physicalMesh.numVertices = verticesWritten;

	PX_ASSERT(mSimplifier->getNumTriangles() * 3 < mParams->physicalMesh.numIndices);

	PxU32 indicesWritten = 0;
	PxU32 trianglesRead = 0;
	while (indicesWritten < mSimplifier->getNumTriangles() * 3)
	{
		PxU32 v0, v1, v2;
		if (mSimplifier->getTriangle(trianglesRead++, v0, v1, v2))
		{
			PX_ASSERT(old2new[v0] != -1);
			PX_ASSERT(old2new[v1] != -1);
			PX_ASSERT(old2new[v2] != -1);
			mIndices[indicesWritten++] = (physx::PxU32)old2new[v0];
			mIndices[indicesWritten++] = (physx::PxU32)old2new[v1];
			mIndices[indicesWritten++] = (physx::PxU32)old2new[v2];
		}
	}
	mParams->physicalMesh.numIndices = indicesWritten;

	updateSkinningNormals();
}


void ClothingPhysicalMesh::clearMiscBuffers()
{
	mConstrainCoefficients.resize(0);
	mBoneIndices.resize(0);
	mBoneWeights.resize(0);
}



void ClothingPhysicalMesh::computeEdgeLengths() const
{
	const PxU32 numIndices = mParams->physicalMesh.numIndices;

	PxF32 average = 0;
	PxF32 shortest = PX_MAX_F32;

	if (mParams->physicalMesh.isTetrahedralMesh)
	{
		for (PxU32 i = 0; i < numIndices; i += 4)
		{
			const PxF32 edge0 = (mVertices[mIndices[i + 0]] - mVertices[mIndices[i + 1]]).magnitudeSquared();
			const PxF32 edge1 = (mVertices[mIndices[i + 0]] - mVertices[mIndices[i + 2]]).magnitudeSquared();
			const PxF32 edge2 = (mVertices[mIndices[i + 0]] - mVertices[mIndices[i + 3]]).magnitudeSquared();
			const PxF32 edge3 = (mVertices[mIndices[i + 1]] - mVertices[mIndices[i + 2]]).magnitudeSquared();
			const PxF32 edge4 = (mVertices[mIndices[i + 1]] - mVertices[mIndices[i + 3]]).magnitudeSquared();
			const PxF32 edge5 = (mVertices[mIndices[i + 2]] - mVertices[mIndices[i + 3]]).magnitudeSquared();
			shortest = PxMin(shortest, edge0);
			shortest = PxMin(shortest, edge1);
			shortest = PxMin(shortest, edge2);
			shortest = PxMin(shortest, edge3);
			shortest = PxMin(shortest, edge4);
			shortest = PxMin(shortest, edge5);

			average += PxSqrt(edge0) + PxSqrt(edge1) + PxSqrt(edge2) + PxSqrt(edge3) + PxSqrt(edge4) + PxSqrt(edge5);
		}
		mParams->physicalMesh.isClosed = false;
	}
	else
	{
		// also check if the mesh is closed
		shdfnd::Array<SortedEdge> edges;

		for (PxU32 i = 0; i < numIndices; i += 3)
		{
			const PxF32 edge0 = (mVertices[mIndices[i + 0]] - mVertices[mIndices[i + 1]]).magnitudeSquared();
			const PxF32 edge1 = (mVertices[mIndices[i + 0]] - mVertices[mIndices[i + 2]]).magnitudeSquared();
			const PxF32 edge2 = (mVertices[mIndices[i + 1]] - mVertices[mIndices[i + 2]]).magnitudeSquared();
			shortest = PxMin(shortest, edge0);
			shortest = PxMin(shortest, edge1);
			shortest = PxMin(shortest, edge2);

			average += PxSqrt(edge0) + PxSqrt(edge1) + PxSqrt(edge2);

			edges.pushBack(SortedEdge(mIndices[i + 0], mIndices[i + 1]));
			edges.pushBack(SortedEdge(mIndices[i + 1], mIndices[i + 2]));
			edges.pushBack(SortedEdge(mIndices[i + 2], mIndices[i + 0]));
		}

		shdfnd::sort(edges.begin(), edges.size(), SortedEdge(0, 0));

		bool meshClosed = false;
		if ((edges.size() & 0x1) == 0) // only works for even number of indices
		{
			meshClosed = true;
			for (PxU32 i = 0; i < edges.size(); i += 2)
			{
				meshClosed &= edges[i] == edges[i + 1];

				if (i > 0)
				{
					meshClosed &= !(edges[i - 1] == edges[i]);
				}
			}
		}
		mParams->physicalMesh.isClosed = meshClosed;
	}

	mParams->physicalMesh.shortestEdgeLength = PxSqrt(shortest);
	if (numIndices > 0)
	{
		mParams->physicalMesh.averageEdgeLength = average / (PxF32)numIndices;
	}
	else
	{
		mParams->physicalMesh.averageEdgeLength = 0.0f;
	}
}



void ClothingPhysicalMesh::addBoneToVertex(PxU32 vertexNumber, PxU16 boneIndex, PxF32 boneWeight)
{
	if (mBoneIndices.size() == 0 || mBoneWeights.size() == 0)
	{
		return;
	}

	const PxU32 numBonesPerVertex = mParams->physicalMesh.numBonesPerVertex;
	for (PxU32 i = 0; i < numBonesPerVertex; i++)
	{
		if (mBoneIndices[vertexNumber * numBonesPerVertex + i] == boneIndex ||
		        mBoneWeights[vertexNumber * numBonesPerVertex + i] == 0.0f)
		{
			mBoneIndices[vertexNumber * numBonesPerVertex + i] = boneIndex;
			mBoneWeights[vertexNumber * numBonesPerVertex + i] =
			    PxMax(mBoneWeights[vertexNumber * numBonesPerVertex + i], boneWeight);
			sortBonesOfVertex(vertexNumber);
			return;
		}
	}
}



void ClothingPhysicalMesh::sortBonesOfVertex(PxU32 vertexNumber)
{
	const PxU32 numBonesPerVertex = mParams->physicalMesh.numBonesPerVertex;
	if (mBoneIndices.size() == 0 || mBoneWeights.size() == 0 || numBonesPerVertex <= 1)
	{
		return;
	}

	// bubble sort
	bool changed = true;
	while (changed)
	{
		changed = false;
		for (PxU32 i = 0; i < numBonesPerVertex - 1; i++)
		{
			const PxU32 index = vertexNumber * numBonesPerVertex + i;
			if (mBoneWeights[index] < mBoneWeights[index + 1])
			{
				// swap
				PxF32 tempF = mBoneWeights[index];
				mBoneWeights[index] = mBoneWeights[index + 1];
				mBoneWeights[index + 1] = tempF;

				PxU16 tempI = mBoneIndices[index];
				mBoneIndices[index] = mBoneIndices[index + 1];
				mBoneIndices[index + 1] = tempI;

				changed = true;
			}
		}
	}
}



void ClothingPhysicalMesh::normalizeBonesOfVertex(PxU32 vertexNumber)
{
	if (mBoneIndices.size() == 0 || mBoneWeights.size() == 0)
	{
		return;
	}

	const PxU32 numBonesPerVertex = mParams->physicalMesh.numBonesPerVertex;

	PxF32 sum = 0;
	PxF32 last = FLT_MAX;
	for (PxU32 i = 0; i < numBonesPerVertex; i++)
	{
		sum += mBoneWeights[vertexNumber * numBonesPerVertex + i];

		// make sure it is sorted!
		PX_ASSERT(mBoneWeights[vertexNumber * numBonesPerVertex + i] <= last);
		last = mBoneWeights[vertexNumber * numBonesPerVertex + i];
	}

	PX_UNUSED(last);

	if (sum > 0)
	{
		PxF32 invSum = 1.0f / sum;
		for (PxU32 i = 0; i < numBonesPerVertex; i++)
		{
			PxF32& weight = mBoneWeights[vertexNumber * numBonesPerVertex + i];
			if (weight > 0)
			{
				weight *= invSum;
			}
			else
			{
				mBoneIndices[vertexNumber * numBonesPerVertex + i] = 0;
			}
		}
	}
	else
	{
		for (PxU32 i = 0; i < numBonesPerVertex; i++)
		{
			mBoneIndices[vertexNumber * numBonesPerVertex + i] = 0;
			mBoneWeights[vertexNumber * numBonesPerVertex + i] = 0.0f;
		}
	}
}



void ClothingPhysicalMesh::updateSkinningNormals()
{
	// only for non-softbodies
	if (isTetrahedralMesh())
	{
		return;
	}

	const PxU32 numVertices = mParams->physicalMesh.numVertices;
	const PxU32 numIndices = mParams->physicalMesh.numIndices;

	PX_ASSERT(mSkinningNormals.size() == mVertices.size());
	memset(mSkinningNormals.begin(), 0, sizeof(PxVec3) * numVertices);

	for (PxU32 i = 0; i < numIndices; i += 3)
	{
		PxVec3 normal;
		normal = mVertices[mIndices[i + 1]] - mVertices[mIndices[i]];
		normal = normal.cross(mVertices[mIndices[i + 2]] - mVertices[mIndices[i]]);
		mSkinningNormals[mIndices[i]] += normal;
		mSkinningNormals[mIndices[i + 1]] += normal;
		mSkinningNormals[mIndices[i + 2]] += normal;
	}

	for (PxU32 i = 0; i < numVertices; i++)
	{
		mSkinningNormals[i].normalize();
	}
}



void ClothingPhysicalMesh::smoothNormals(PxU32 numIterations)
{
	const PxU32 increment = mParams->physicalMesh.isTetrahedralMesh ? 4u : 3u;
	const PxU32 numIndices = mParams->physicalMesh.numIndices;

	for (PxU32 iters = 0; iters < numIterations; iters++)
	{
		for (PxU32 i = 0; i < numIndices; i += increment)
		{
			PxVec3& n0 = mNormals[mIndices[i + 0]];
			PxVec3& n1 = mNormals[mIndices[i + 1]];
			PxVec3& n2 = mNormals[mIndices[i + 2]];
			PxVec3 n = n0 + n1 + n2;
			n.normalize();
			n0 = n;
			n1 = n;
			n2 = n;
		}
	}
}



void ClothingPhysicalMesh::updateOptimizationData()
{
	PX_ASSERT(mParams != NULL);

	const PxI32 numVertices = mParams->physicalMesh.vertices.arraySizes[0];
	const PxU32 numBonesPerVertex = mParams->physicalMesh.numBonesPerVertex;

	const PxF32* boneWeights = mParams->physicalMesh.boneWeights.buf;
	PX_ASSERT(boneWeights == NULL || mParams->physicalMesh.boneWeights.arraySizes[0] == numVertices * (PxI32)numBonesPerVertex);

	const ClothingPhysicalMeshParametersNS::ConstrainCoefficient_Type* constrainCoeffs = mParams->physicalMesh.constrainCoefficients.buf;
	PX_ASSERT(constrainCoeffs == NULL || mParams->physicalMesh.constrainCoefficients.arraySizes[0] == numVertices);

	if (boneWeights == NULL && constrainCoeffs == NULL)
	{
		return;
	}

	PxU32 allocNumVertices = ((PxU32)ceil((PxF32)numVertices / NUM_VERTICES_PER_CACHE_BLOCK)) * NUM_VERTICES_PER_CACHE_BLOCK; // allocate more to have a multiple of numVerticesPerCachBlock

	NxParameterized::Handle optimizationDataHandle(*mParams, "physicalMesh.optimizationData");
	PX_ASSERT(optimizationDataHandle.isValid());
	optimizationDataHandle.resizeArray((physx::PxI32)(allocNumVertices + 1) / 2);
	PxU8* optimizationData = mParams->physicalMesh.optimizationData.buf;
	memset(optimizationData, 0, sizeof(PxU8) * mParams->physicalMesh.optimizationData.arraySizes[0]);

	for (PxI32 i = 0; i < numVertices; ++i)
	{
		PxU8 numBones = 0;
		if (boneWeights != NULL)
		{
			for (; numBones < numBonesPerVertex; numBones++)
			{
				if (boneWeights[i * numBonesPerVertex + numBones] == 0.0f)
				{
					break;
				}
			}
		}

		PxU8& data = optimizationData[i / 2];
		PX_ASSERT(numBones < 8); // we use 3 bits

		if (constrainCoeffs != NULL)
		{
			PxU8 bitShift = 0;
			if (i % 2 == 0)
			{
				data = 0;
			}
			else
			{
				bitShift = 4;
			}
			data |= numBones << bitShift;

			// store for each vertex if collisionSphereDistance is < 0
			if (constrainCoeffs[i].collisionSphereDistance < 0.0f)
			{
				data |= 8 << bitShift;
				mParams->physicalMesh.hasNegativeBackstop = true;
			}
			else
			{
				data &= ~(8 << bitShift);
			}
		}
	}
}



void ClothingPhysicalMesh::updateMaxMaxDistance()
{
	const PxU32 numVertices = mParams->physicalMesh.numVertices;

	PxF32 maxMaxDistance = 0.0f;
	for (PxU32 i = 0; i < numVertices; i++)
	{
		maxMaxDistance = PxMax(maxMaxDistance, mConstrainCoefficients[i].maxDistance);
	}

	mParams->physicalMesh.maximumMaxDistance = maxMaxDistance;
}



void ClothingPhysicalMesh::preSerialize(void* userData)
{
	PX_UNUSED(userData);

	writeBackData();

	// shrink the buffers

	if (!mVertices.isEmpty() && mVertices.size() != mParams->physicalMesh.numVertices)
	{
		mVertices.resize(mParams->physicalMesh.numVertices);
	}

	if (!mNormals.isEmpty() && mNormals.size() != mParams->physicalMesh.numVertices)
	{
		mNormals.resize(mParams->physicalMesh.numVertices);
	}

	if (!mSkinningNormals.isEmpty() && mSkinningNormals.size() != mParams->physicalMesh.numVertices)
	{
		mSkinningNormals.resize(mParams->physicalMesh.numVertices);
	}

	if (!mConstrainCoefficients.isEmpty() && mConstrainCoefficients.size() != mParams->physicalMesh.numVertices)
	{
		mConstrainCoefficients.resize(mParams->physicalMesh.numVertices);
	}

	if (!mBoneIndices.isEmpty() && mBoneIndices.size() != mParams->physicalMesh.numVertices * mParams->physicalMesh.numBonesPerVertex)
	{
		mBoneIndices.resize(mParams->physicalMesh.numVertices * mParams->physicalMesh.numBonesPerVertex);
	}

	if (!mBoneWeights.isEmpty() && mBoneWeights.size() != mParams->physicalMesh.numVertices * mParams->physicalMesh.numBonesPerVertex)
	{
		mBoneWeights.resize(mParams->physicalMesh.numVertices * mParams->physicalMesh.numBonesPerVertex);
	}

	if (!mIndices.isEmpty() && mIndices.size() != mParams->physicalMesh.numIndices)
	{
		mIndices.resize(mParams->physicalMesh.numIndices);
	}

	updateOptimizationData();
}



void ClothingPhysicalMesh::permuteBoneIndices(Array<PxI32>& old2newBoneIndices)
{
	if (mBoneIndices.size() == 0)
	{
		return;
	}

	const PxU32 numVertices = mParams->physicalMesh.numVertices;
	const PxU32 numBonesPerVertex = mParams->physicalMesh.numBonesPerVertex;

	for (PxU32 j = 0; j < numVertices; j++)
	{
		for (PxU32 k = 0; k < numBonesPerVertex; k++)
		{
			PxU16& index = mBoneIndices[j * numBonesPerVertex + k];
			PX_ASSERT(old2newBoneIndices[index] >= 0);
			PX_ASSERT(old2newBoneIndices[index] <= 0xffff);
			index = (PxU16)old2newBoneIndices[index];
		}
	}
}



void ClothingPhysicalMesh::applyTransformation(const PxMat34Legacy& transformation, PxF32 scale)
{
	const PxU32 numVertices = mParams->physicalMesh.numVertices;

	PX_ASSERT(scale > 0.0f); // PH: negative scale won't work well here

	for (PxU32 i = 0; i < numVertices; i++)
	{
		if (!mVertices.isEmpty())
		{
			mVertices[i] = (transformation * mVertices[i]) * scale;
		}
		if (!mNormals.isEmpty())
		{
			mNormals[i] = transformation.M * mNormals[i];
		}
		if (!mSkinningNormals.isEmpty())
		{
			mSkinningNormals[i] = transformation.M * mSkinningNormals[i];
		}
		if (!mConstrainCoefficients.isEmpty())
		{
			mConstrainCoefficients[i].maxDistance *= scale;
			mConstrainCoefficients[i].collisionSphereDistance *= scale;
			mConstrainCoefficients[i].collisionSphereRadius *= scale;
		}
	}

	if (transformation.M.determinant() * scale < 0.0f)
	{
		const PxU32 numIndices = mParams->physicalMesh.numIndices;

		if (mParams->physicalMesh.isTetrahedralMesh)
		{
			PX_ASSERT(numIndices % 4 == 0);
			for (PxU32 i = 0; i < numIndices; i += 4)
			{
				physx::swap(mIndices[i + 2], mIndices[i + 3]);
			}
		}
		else
		{
			// Flip the triangle indices to change winding (and thus normal generation in the PhysX SDK
			PX_ASSERT(numIndices % 3 == 0);
			for (PxU32 i = 0; i < numIndices; i += 3)
			{
				physx::swap(mIndices[i + 1], mIndices[i + 2]);
			}
		}

		mParams->physicalMesh.flipNormals ^= true;

		if (mParams->transitionDownB.buf != NULL || mParams->transitionUpB.buf != NULL)
		{
			APEX_DEBUG_WARNING("applyTransformation will not work with old assets, re-export from DCC tools");
		}

		const PxU32 numTransDown = (physx::PxU32)mParams->transitionDown.arraySizes[0];
		for (PxU32 i = 0; i < numTransDown; i++)
		{
			mParams->transitionDown.buf[i].vertexBary.z *= scale;
			mParams->transitionDown.buf[i].normalBary.z *= scale;
		}

		const PxU32 numTransUp = (physx::PxU32)mParams->transitionUp.arraySizes[0];
		for (PxU32 i = 0; i < numTransUp; i++)
		{
			mParams->transitionUp.buf[i].vertexBary.z *= scale;
			mParams->transitionUp.buf[i].normalBary.z *= scale;
		}
	}

	mParams->physicalMesh.maximumMaxDistance *= scale;
	mParams->physicalMesh.shortestEdgeLength *= scale;
	mParams->physicalMesh.averageEdgeLength *= scale;
}



void ClothingPhysicalMesh::applyPermutation(const Array<PxU32>& permutation)
{
	const PxU32 numVertices = mParams->physicalMesh.numVertices;
	const PxU32 numBonesPerVertex = mParams->physicalMesh.numBonesPerVertex;

	if (!mVertices.isEmpty())
	{
		ApexPermute<PxVec3>(mVertices.begin(), &permutation[0], numVertices);
	}

	if (!mNormals.isEmpty())
	{
		ApexPermute<PxVec3>(mNormals.begin(), &permutation[0], numVertices);
	}

	if (!mSkinningNormals.isEmpty())
	{
		ApexPermute<PxVec3>(mSkinningNormals.begin(), &permutation[0], numVertices);
	}

	if (!mConstrainCoefficients.isEmpty())
	{
		ApexPermute<ClothingPhysicalMeshParametersNS::ConstrainCoefficient_Type>(mConstrainCoefficients.begin(), &permutation[0], numVertices);
	}

	if (!mBoneIndices.isEmpty())
	{
		ApexPermute<PxU16>(mBoneIndices.begin(), &permutation[0], numVertices, numBonesPerVertex);
	}

	if (!mBoneWeights.isEmpty())
	{
		ApexPermute<PxF32>(mBoneWeights.begin(), &permutation[0], numVertices, numBonesPerVertex);
	}
}



struct OrderedTriangle
{
	void init(PxU32 _triNr, PxU32 _v0, PxU32 _v1, PxU32 _v2)
	{
		triNr = _triNr;
		v0 = _v0;
		v1 = _v1;
		v2 = _v2;
		// bubble sort
		if (v0 > v1)
		{
			physx::PxU32 v = v0;
			v0 = v1;
			v1 = v;
		}
		if (v1 > v2)
		{
			physx::PxU32 v = v1;
			v1 = v2;
			v2 = v;
		}
		if (v0 > v1)
		{
			physx::PxU32 v = v0;
			v0 = v1;
			v1 = v;
		}
	}
	bool operator()(const OrderedTriangle& a, const OrderedTriangle& b) const
	{
		if (a.v0 < b.v0)
		{
			return true;
		}
		if (a.v0 > b.v0)
		{
			return false;
		}
		if (a.v1 < b.v1)
		{
			return true;
		}
		if (a.v1 > b.v1)
		{
			return false;
		}
		return (a.v2 < b.v2);
	}
	bool operator == (const OrderedTriangle& t) const
	{
		return v0 == t.v0 && v1 == t.v1 && v2 == t.v2;
	}
	PxU32 v0, v1, v2;
	PxU32 triNr;
};

bool ClothingPhysicalMesh::removeDuplicatedTriangles(PxU32 numIndices, PxU32 indexByteStride, const void* indices)
{
	PxU32 numTriangles = numIndices / 3;
	Array<OrderedTriangle> triangles;
	triangles.resize(numTriangles);

	{
		const PxU8* srcIndices = (const PxU8*)indices;
		for (PxU32 i = 0; i < numTriangles; i++)
		{
			PxU32 i0 = *(const PxU32*)(srcIndices);
			srcIndices += indexByteStride;
			PxU32 i1 = *(const PxU32*)(srcIndices);
			srcIndices += indexByteStride;
			PxU32 i2 = *(const PxU32*)(srcIndices);
			srcIndices += indexByteStride;

			triangles[i].init(i, i0, i1, i2);
		}
	}

	shdfnd::sort(triangles.begin(), triangles.size(), OrderedTriangle());

	PxU32 fromPos = 0;
	PxU32 toPos = 0;
	while (fromPos < numTriangles)
	{
		OrderedTriangle& t = triangles[fromPos];
		triangles[toPos] = t;
		fromPos++;
		toPos++;
		while (fromPos < numTriangles && triangles[fromPos] == t)
		{
			fromPos++;
		}
	}
	if (fromPos == toPos)
	{
		return false;
	}

	mParams->physicalMesh.numIndices = 3 * toPos;

	mIndices.resize(mParams->physicalMesh.numIndices);
	if (mParams->physicalMesh.numIndices > 0)
	{
		for (PxU32 i = 0; i < toPos; i++)
		{
			OrderedTriangle& t = triangles[i];
			const PxU8* srcIndices = (const PxU8*)indices + 3 * t.triNr * indexByteStride;

			mIndices[3 * i]   = *(PxU32*)srcIndices;
			srcIndices += indexByteStride;
			mIndices[3 * i + 1] = *(PxU32*)srcIndices;
			srcIndices += indexByteStride;
			mIndices[3 * i + 2] = *(PxU32*)srcIndices;
			srcIndices += indexByteStride;
		}
	}

	fixTriangleOrientations();
	return true;
}


struct OrderedTriangleEdge
{
	void init(PxU32 _v0, PxU32 _v1, PxU32 _triNr, PxU32 _edgeNr)
	{
		if (_v0 < _v1)
		{
			v0 = _v0;
			v1 = _v1;
		}
		else
		{
			v0 = _v1;
			v1 = _v0;
		}
		triNr = _triNr;
		edgeNr = _edgeNr;
	}
	bool operator()(const OrderedTriangleEdge& a, const OrderedTriangleEdge& b) const
	{
		if (a.v0 < b.v0)
		{
			return true;
		}
		if (a.v0 > b.v0)
		{
			return false;
		}
		return (a.v1 < b.v1);
	}
	bool operator == (const OrderedTriangleEdge& e) const
	{
		return v0 == e.v0 && v1 == e.v1;
	}
	PxU32 v0, v1;
	PxU32 triNr, edgeNr;
};

void ClothingPhysicalMesh::computeNeighborInformation(Array<PxI32> &neighbors)
{
	// compute neighbor information
	const PxU32 numTriangles = mParams->physicalMesh.numIndices / 3;

	Array<OrderedTriangleEdge> edges;
	edges.resize(3 * numTriangles);

	for (PxU32 i = 0; i < numTriangles; i++)
	{
		PxU32 i0 = mIndices[3 * i];
		PxU32 i1 = mIndices[3 * i + 1];
		PxU32 i2 = mIndices[3 * i + 2];
		edges[3 * i  ].init(i0, i1, i, 0);
		edges[3 * i + 1].init(i1, i2, i, 1);
		edges[3 * i + 2].init(i2, i0, i, 2);
	}

	shdfnd::sort(edges.begin(), edges.size(), OrderedTriangleEdge());

	neighbors.resize(3 * numTriangles, -1);

	PxU32 i = 0;
	while (i < edges.size())
	{
		OrderedTriangleEdge& e0 = edges[i];
		i++;
		while (i < edges.size() && edges[i] == e0)
		{
			OrderedTriangleEdge& e1 = edges[i];
			neighbors[3 * e0.triNr + e0.edgeNr] = (physx::PxI32)e1.triNr;
			neighbors[3 * e1.triNr + e1.edgeNr] = (physx::PxI32)e0.triNr;
			i++;
		}
	}
}


void ClothingPhysicalMesh::fixTriangleOrientations()
{
	PX_ASSERT(!mParams->physicalMesh.isTetrahedralMesh);
	Array<PxI32> neighbors;
	computeNeighborInformation(neighbors);

	const PxU32 numTriangles = mParams->physicalMesh.numIndices / 3;

	// 0 = non visited, 1 = visited, 2 = visited, to be flipped
	Array<PxU8> marks;
	marks.resize(numTriangles, 0);

	Array<PxU32> queue;

	for (PxU32 i = 0; i < numTriangles; i++)
	{
		if (marks[i] != 0)
		{
			continue;
		}
		queue.clear();
		marks[i] = 1;
		queue.pushBack(i);
		while (!queue.empty())
		{
			physx::PxU32 triNr = queue[queue.size() - 1];
			queue.popBack();
			for (PxU32 j = 0; j < 3; j++)
			{
				int adjNr = neighbors[3 * triNr + j];
				if (adjNr < 0 || marks[(physx::PxU32)adjNr] != 0)
				{
					continue;
				}
				queue.pushBack((physx::PxU32)adjNr);
				PxU32 i0, i1;
				if (marks[triNr] == 1)
				{
					i0 = mIndices[3 * triNr + j];
					i1 = mIndices[3 * triNr + ((j + 1) % 3)];
				}
				else
				{
					i1 = mIndices[3 * triNr + j];
					i0 = mIndices[3 * triNr + ((j + 1) % 3)];
				}
				// don't swap here because this would corrupt the neighbor information
				marks[(physx::PxU32)adjNr] = 1;
				if (mIndices[3 * (physx::PxU32)adjNr + 0] == i0 && mIndices[3 * (physx::PxU32)adjNr + 1] == i1)
				{
					marks[(physx::PxU32)adjNr] = 2;
				}
				if (mIndices[3 * (physx::PxU32)adjNr + 1] == i0 && mIndices[3 * (physx::PxU32)adjNr + 2] == i1)
				{
					marks[(physx::PxU32)adjNr] = 2;
				}
				if (mIndices[3 * (physx::PxU32)adjNr + 2] == i0 && mIndices[3 * (physx::PxU32)adjNr + 0] == i1)
				{
					marks[(physx::PxU32)adjNr] = 2;
				}
			}
		}
	}
	for (PxU32 i = 0; i < numTriangles; i++)
	{
		if (marks[i] == 2)
		{
			PxU32 i0 = mIndices[3 * i];
			mIndices[3 * i] = mIndices[3 * i + 1];
			mIndices[3 * i + 1] = i0;
		}
	}
}

}
}
} // namespace physx::apex

#endif // NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED
