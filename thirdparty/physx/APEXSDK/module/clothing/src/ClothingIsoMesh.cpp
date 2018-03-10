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

#include "NxApex.h"
#include "ClothingIsoMesh.h"

#include "ModuleClothing.h"
#include "ApexIsoMesh.h"
#include "ApexQuadricSimplifier.h"
#include "ApexMeshContractor.h"
#include "ApexGeneralizedMarchingCubes.h"
#include "ApexTetrahedralizer.h"
#include "ClothingPhysicalMesh.h"

#include "ApexSharedUtils.h"

#include "ReadCheck.h"
#include "WriteCheck.h"

namespace physx
{
namespace apex
{
namespace clothing
{

ClothingIsoMesh::ClothingIsoMesh(ModuleClothing* module, const ApexIsoMesh& isoMesh, physx::PxU32 subdivision, NxResourceList* list) :
	mModule(module),
	mSubdivision(subdivision),
	mNumVertices(0),
	mVertexCapacity(0),
	mVertices(NULL),
	mNumIndices(0),
	mIndexCapacity(0),
	mIndices(NULL),
	mSimplifier(NULL),
	mContractor(NULL),
	bDirty(0)
{
	mNumVertices = mVertexCapacity = isoMesh.getNumVertices();
	mVertices = (physx::PxVec3*)PX_ALLOC(sizeof(physx::PxVec3) * mNumVertices, PX_DEBUG_EXP("ClothingIsoMesh::mVertices"));

	for (physx::PxU32 i = 0 ; i < isoMesh.getNumVertices(); i++)
	{
		mVertices[i] = isoMesh.getVertex(i);
	}

	mNumIndices = mIndexCapacity = isoMesh.getNumTriangles() * 3;
	mIndices = (physx::PxU32*)PX_ALLOC(sizeof(physx::PxU32) * mNumIndices, PX_DEBUG_EXP("ClothingIsoMesh::mIndices"));

	mBound.setEmpty();
	for (physx::PxU32 i = 0; i < isoMesh.getNumTriangles(); i++)
	{
		const physx::PxU32 index = i * 3;
		isoMesh.getTriangle(i, mIndices[index + 0], mIndices[index + 1], mIndices[index + 2]);
		mBound.include(mVertices[mIndices[index + 0]]);
		mBound.include(mVertices[mIndices[index + 1]]);
		mBound.include(mVertices[mIndices[index + 2]]);
	}

	list->add(*this);
}




physx::PxU32 ClothingIsoMesh::getNumVertices() const
{
	NX_READ_ZONE();
	if (mSimplifier != NULL)
	{
		PX_ASSERT(mContractor == NULL);
		return mSimplifier->getNumVertices() - mSimplifier->getNumDeletedVertices();
	}

	if (mContractor != NULL)
	{
		PX_ASSERT(mSimplifier == NULL);
		return mContractor->getNumVertices();
	}

	return mNumVertices;
}



physx::PxU32 ClothingIsoMesh::getNumIndices() const
{
	NX_READ_ZONE();
	if (mSimplifier != NULL)
	{
		PX_ASSERT(mContractor == NULL);
		return mSimplifier->getNumTriangles() * 3;
	}

	if (mContractor != NULL)
	{
		PX_ASSERT(mSimplifier == NULL);
		return mContractor->getNumIndices();
	}

	return mNumIndices;
}



void ClothingIsoMesh::getVertices(void* vertexDestination, physx::PxU32 byteStride)
{
	NX_WRITE_ZONE();
	if (byteStride == 0)
	{
		byteStride = sizeof(physx::PxVec3);
	}

	if (byteStride < sizeof(physx::PxVec3))
	{
		APEX_INTERNAL_ERROR("byte stride is too small (%d)", byteStride);
		return;
	}

	writeBackData();

	physx::PxU8* destPtr = (physx::PxU8*)vertexDestination;
	for (physx::PxU32 i = 0; i < mNumVertices; i++)
	{
		(physx::PxVec3&)(*(destPtr + byteStride * i)) = mVertices[i];
	}
}



void ClothingIsoMesh::getIndices(void* indexDestination, physx::PxU32 byteStride)
{
	NX_WRITE_ZONE();
	if (byteStride == 0)
	{
		byteStride = sizeof(physx::PxU32);
	}

	if (byteStride < sizeof(physx::PxU32))
	{
		APEX_INTERNAL_ERROR("byte stride is too small (%d)", byteStride);
		return;
	}

	writeBackData();

	physx::PxU8* destPtr = (physx::PxU8*)indexDestination;
	for (physx::PxU32 i = 0; i < mNumIndices; i++)
	{
		(physx::PxU32&)(*(destPtr + byteStride * i)) = mIndices[i];
	}
}




void ClothingIsoMesh::release()
{
	mModule->releaseIsoMesh(this);
}



void ClothingIsoMesh::destroy()
{
	// clear all stuff
	if (mContractor != NULL)
	{
		delete mContractor;
		mContractor = NULL;
	}

	if (mSimplifier != NULL)
	{
		delete mSimplifier;
		mSimplifier = NULL;
	}

	if (mVertices != NULL)
	{
		PX_FREE(mVertices);
		mVertices = NULL;
		mNumVertices = mVertexCapacity = 0;
	}

	if (mIndices != NULL)
	{
		PX_FREE(mIndices);
		mIndices = NULL;
		mNumIndices = mIndexCapacity = 0;
	}

	delete this;
}




physx::PxU32 ClothingIsoMesh::simplify(physx::PxU32 subdivisions, physx::PxI32 maxSteps, physx::PxF32 maxError, IProgressListener* progressCallback)
{
	NX_WRITE_ZONE();
	PxI32 timeRemaining = 100;
	HierarchicalProgressListener progress(100, progressCallback);

	if (mContractor != NULL)
	{
		writeBackContractor(true);
	}

	if (mSimplifier == NULL)
	{
		progress.setSubtaskWork(50, "Init Simplifier");
		initSimplifier(&progress);
		progress.completeSubtask();

		timeRemaining = 50;
	}

	PX_ASSERT(mContractor == NULL);
	PX_ASSERT(mSimplifier != NULL);

	progress.setSubtaskWork(timeRemaining, "Simplification step");

	physx::PxU32 steps = mSimplifier->simplify(subdivisions, maxSteps, maxError, &progress);

	progress.completeSubtask();

	bDirty = 1;

	return steps;
}



physx::PxF32 ClothingIsoMesh::contract(physx::PxI32 steps, physx::PxF32 abortionRatio, bool expand, IProgressListener* progressListener)
{
	NX_WRITE_ZONE();
	HierarchicalProgressListener progress(100, progressListener);

	if (mSimplifier != NULL)
	{
		writeBackSimplifier(true);
	}

	if (mContractor == NULL)
	{
		progress.setSubtaskWork(50, "Init Contractor");
		initContractor(&progress);
		progress.completeSubtask();
	}

	PX_ASSERT(mSimplifier == NULL);
	PX_ASSERT(mContractor != NULL);

	progress.setSubtaskWork(-1, "Contraction step");

	physx::PxF32 volumeRatio;
	mContractor->contract(steps, abortionRatio, volumeRatio, &progress);

	if (expand)
	{
		mContractor->expandBorder();
	}

	progress.completeSubtask();

	bDirty = 1;

	return volumeRatio;
}



NxClothingPhysicalMesh* ClothingIsoMesh::generateClothMesh(PxU32 bubbleSizeToRemove, IProgressListener* progressListener)
{
	NX_WRITE_ZONE();
	writeBackData();

	HierarchicalProgressListener progress(100, progressListener);

	progress.setSubtaskWork(10, "Initializing Generalized Marching Cubes");

	PxBounds3 bound;
	bound.setEmpty();
	for (physx::PxU32 i = 0; i < mNumIndices; i++)
	{
		bound.include(mVertices[mIndices[i]]);
	}

	ApexGeneralizedMarchingCubes* agmc = PX_NEW(ApexGeneralizedMarchingCubes)(bound, mSubdivision);
	for (physx::PxU32 i = 0; i < mNumIndices; i += 3)
	{
		agmc->registerTriangle(mVertices[mIndices[i]], mVertices[mIndices[i + 1]], mVertices[mIndices[i + 2]]);
	}

	progress.completeSubtask();

	progress.setSubtaskWork(70, "Running Generalized Marching Cubes");

	agmc->endRegistration(bubbleSizeToRemove, &progress);

	progress.completeSubtask();

	progress.setSubtaskWork(20, "Copying the geometry");

	Array<PxU32> masterValues(agmc->getNumVertices(), 0xffffffff);
	ClothingPhysicalMesh* clothingPhysicalMesh = mModule->createPhysicalMeshInternal(NULL);
	clothingPhysicalMesh->setGeometry(false, agmc->getNumVertices(), sizeof(physx::PxVec3), agmc->getVertices(), masterValues.begin(), agmc->getNumIndices(), sizeof(physx::PxU32), agmc->getIndices());

	progress.completeSubtask();

	PX_DELETE(agmc);

	return clothingPhysicalMesh;
}



NxClothingPhysicalMesh* ClothingIsoMesh::generateSoftbodyMesh(PxU32 tetraSubdivision, IProgressListener* progressListener)
{
	NX_WRITE_ZONE();
	writeBackData();

	if (tetraSubdivision == 0)
	{
		tetraSubdivision = mSubdivision;
	}

	HierarchicalProgressListener progress(100, progressListener);

	progress.setSubtaskWork(10, "Initializing SoftBody Mesh");
	ApexTetrahedralizer tetralizer(tetraSubdivision);

	for (PxU32 i = 0; i < mNumVertices; i++)
	{
		tetralizer.registerVertex(mVertices[i]);
	}

	PX_ASSERT(mNumIndices % 3 == 0);
	for (PxU32 i = 0; i < mNumIndices; i += 3)
	{
		tetralizer.registerTriangle(mIndices[i], mIndices[i + 1], mIndices[i + 2]);
	}

	progress.completeSubtask();
	progress.setSubtaskWork(80, "Generate SoftBody Mesh");

	tetralizer.endRegistration(&progress);

	progress.completeSubtask();
	progress.setSubtaskWork(10, "Copying results");

	ClothingPhysicalMesh* physicalMesh = mModule->createPhysicalMeshInternal(NULL);

	shdfnd::Array<PxVec3> vertices(tetralizer.getNumVertices());
	shdfnd::Array<PxU32> indices(tetralizer.getNumIndices());
	tetralizer.getVertices(vertices.begin());
	tetralizer.getIndices(indices.begin());

	shdfnd::Array<PxU32> masterValues(vertices.size(), 0xffffffff);

	physicalMesh->setGeometry(true, vertices.size(), sizeof(PxVec3), vertices.begin(), masterValues.begin(), indices.size(), sizeof(PxU32), indices.begin());

	progress.completeSubtask();

	return physicalMesh;
}



void ClothingIsoMesh::initContractor(IProgressListener* progress)
{
	PX_ASSERT(mSimplifier == NULL);
	PX_ASSERT(mContractor == NULL);
	mContractor = PX_NEW(ApexMeshContractor);

	for (physx::PxU32 i = 0; i < mNumVertices; i++)
	{
		mContractor->registerVertex(mVertices[i]);
	}

	for (physx::PxU32 i = 0; i < mNumIndices; i += 3)
	{
		mContractor->registerTriangle(mIndices[i], mIndices[i + 1], mIndices[i + 2]);
	}
	mContractor->endRegistration(mSubdivision, progress);
}



void ClothingIsoMesh::writeBackContractor(bool clearItAsWell)
{
	PX_ASSERT(mSimplifier == NULL);
	PX_ASSERT(mContractor != NULL);

	if (mContractor->getNumVertices() > mVertexCapacity)
	{
		PX_FREE(mVertices);
		mVertices = (physx::PxVec3*)PX_ALLOC(sizeof(physx::PxVec3) * mContractor->getNumVertices(), PX_DEBUG_EXP("ClothingIsoMesh::mVertices"));
		mVertexCapacity = mContractor->getNumVertices();
	}

	// if buffer is too big we don't reallocate
	memcpy(mVertices, mContractor->getVertices(), sizeof(physx::PxVec3) * mContractor->getNumVertices());
	mNumVertices = mContractor->getNumVertices();

	if (mContractor->getNumIndices() > mIndexCapacity)
	{
		PX_FREE(mIndices);
		mIndices = (physx::PxU32*)PX_ALLOC(sizeof(physx::PxU32) * mContractor->getNumIndices(), PX_DEBUG_EXP("ClothingIsoMesh::mIndices"));
		mIndexCapacity = mContractor->getNumIndices();
	}

	memcpy(mIndices, mContractor->getIndices(), sizeof(physx::PxU32) * mContractor->getNumIndices());
	mNumIndices = mContractor->getNumIndices();

	mBound.setEmpty();
	for (physx::PxU32 i = 0; i < mNumIndices; i++)
	{
		mBound.include(mVertices[mIndices[i]]);
	}

	if (clearItAsWell)
	{
		delete mContractor;
		mContractor = NULL;
	}
}



void ClothingIsoMesh::initSimplifier(IProgressListener* progressCallback)
{
	PX_ASSERT(mSimplifier == NULL);
	PX_ASSERT(mContractor == NULL);

	mSimplifier = PX_NEW(ApexQuadricSimplifier);

	HierarchicalProgressListener progress(100, progressCallback);

	progress.setSubtaskWork(5, "Add Vertices");

	for (physx::PxU32 i = 0; i < mNumVertices; i++)
	{
		//if ((i & 0xffff) == 0)
		//progress.setProgress(100 * i / mNumVertices, "Add Vertices");
		mSimplifier->registerVertex(mVertices[i]);
	}

	progress.completeSubtask();
	progress.setSubtaskWork(10, "Add Triangles");

	for (physx::PxU32 i = 0; i < mNumIndices; i += 3)
	{
		//if ((i & 0xffff) == 0)
		//progress.setProgress(100 * i / mNumIndices, "Add Triangles");
		mSimplifier->registerTriangle(mIndices[i], mIndices[i + 1], mIndices[i + 2]);
	}
	progress.completeSubtask();

	progress.setSubtaskWork(85, "Init");
	mSimplifier->endRegistration(true, &progress);
	progress.completeSubtask();
}



void ClothingIsoMesh::writeBackSimplifier(bool clearItAsWell)
{
	PX_ASSERT(mContractor == NULL);
	PX_ASSERT(mSimplifier != NULL);

	if (mSimplifier->getNumVertices() > mVertexCapacity)
	{
		PX_FREE(mVertices);
		mVertices = (physx::PxVec3*)PX_ALLOC(sizeof(physx::PxVec3) * mSimplifier->getNumVertices(), PX_DEBUG_EXP("ClothingIsoMesh::mVertices"));
		mVertexCapacity = mSimplifier->getNumVertices();
	}

	physx::Array<physx::PxI32> old2new(mSimplifier->getNumVertices(), -1);
	physx::PxU32 verticesWritten = 0;
	for (physx::PxU32 i = 0; i < mSimplifier->getNumVertices(); i++)
	{
		physx::PxVec3 pos;
		if (mSimplifier->getVertexPosition(i, pos))
		{
			old2new[i] = (physx::PxI32)verticesWritten;
			mVertices[verticesWritten++] = pos;
		}
		//else
		//{
		//	old2new[i] = verticesWritten;
		//	mVertices[verticesWritten++].set(0,0,0);
		//}
	}
	mNumVertices = verticesWritten;
	PX_ASSERT(mNumVertices == (mSimplifier->getNumVertices() - mSimplifier->getNumDeletedVertices()));

	if (mSimplifier->getNumTriangles() * 3 > mIndexCapacity)
	{
		PX_FREE(mIndices);
		mIndices = (physx::PxU32*)PX_ALLOC(sizeof(physx::PxU32) * mSimplifier->getNumTriangles() * 3, PX_DEBUG_EXP("ClothingIsoMesh::mIndices"));
		mIndexCapacity = mSimplifier->getNumTriangles() * 3;
	}

	mBound.setEmpty();
	physx::PxU32 indicesWritten = 0;
	physx::PxU32 indicesRead = 0;
	while (indicesWritten < mSimplifier->getNumTriangles() * 3)
		//for (physx::PxU32 i = 0; i < mSimplifier->getNumTriangles(); i++)
	{
		physx::PxU32 v0, v1, v2;
		if (mSimplifier->getTriangle(indicesRead++, v0, v1, v2))
		{
			PX_ASSERT(old2new[v0] != -1);
			PX_ASSERT(old2new[v1] != -1);
			PX_ASSERT(old2new[v2] != -1);
			mIndices[indicesWritten++] = (physx::PxU32)old2new[v0];
			mIndices[indicesWritten++] = (physx::PxU32)old2new[v1];
			mIndices[indicesWritten++] = (physx::PxU32)old2new[v2];
			mBound.include(mVertices[old2new[v0]]);
			mBound.include(mVertices[old2new[v1]]);
			mBound.include(mVertices[old2new[v2]]);
		}
	}
	mNumIndices = indicesWritten;
	PX_ASSERT(mNumIndices == mSimplifier->getNumTriangles() * 3);

	if (clearItAsWell)
	{
		delete mSimplifier;
		mSimplifier = NULL;
	}
}




void ClothingIsoMesh::writeBackData()
{
	if (bDirty ==  0 || (mSimplifier == NULL && mContractor == NULL))
	{
		return;
	}

	bDirty = 0;

	if (mSimplifier != NULL)
	{
		writeBackSimplifier(false);
	}

	if (mContractor != NULL)
	{
		writeBackContractor(false);
	}
}

}
}
} // namespace physx::apex

#endif // NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED