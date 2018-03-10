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


#include "PxTriangleMeshExt.h"
#include "PxMeshQuery.h"
#include "PxGeometryQuery.h"
#include "PxTriangleMeshGeometry.h"
#include "PxHeightFieldGeometry.h"
#include "PxTriangleMesh.h"
#include "PsAllocator.h"

using namespace physx;

PxMeshOverlapUtil::PxMeshOverlapUtil() : mResultsMemory(mResults), mNbResults(0), mMaxNbResults(64)
{
}

PxMeshOverlapUtil::~PxMeshOverlapUtil()
{
	if(mResultsMemory != mResults)
		PX_FREE(mResultsMemory);
}

PxU32 PxMeshOverlapUtil::findOverlap(const PxGeometry& geom, const PxTransform& geomPose, const PxTriangleMeshGeometry& meshGeom, const PxTransform& meshPose)
{
	bool overflow;
	PxU32 nbTouchedTris = PxMeshQuery::findOverlapTriangleMesh(geom, geomPose, meshGeom, meshPose, mResultsMemory, mMaxNbResults, 0, overflow);

	if(overflow)
	{
		const PxU32 maxNbTris = meshGeom.triangleMesh->getNbTriangles();
		if(!maxNbTris)
		{
			mNbResults = 0;
			return 0;
		}

		if(mMaxNbResults<maxNbTris)
		{
			if(mResultsMemory != mResults)
				PX_FREE(mResultsMemory);

			mResultsMemory = (PxU32*)PX_ALLOC(sizeof(PxU32)*maxNbTris, PX_DEBUG_EXP("PxMeshOverlapUtil::findOverlap"));
			mMaxNbResults = maxNbTris;
		}
		nbTouchedTris = PxMeshQuery::findOverlapTriangleMesh(geom, geomPose, meshGeom, meshPose, mResultsMemory, mMaxNbResults, 0, overflow);
		PX_ASSERT(nbTouchedTris);
		PX_ASSERT(!overflow);
	}
	mNbResults = nbTouchedTris;
	return nbTouchedTris;
}

PxU32 PxMeshOverlapUtil::findOverlap(const PxGeometry& geom, const PxTransform& geomPose, const PxHeightFieldGeometry& hfGeom, const PxTransform& hfPose)
{
	bool overflow = true;
	PxU32 nbTouchedTris = 0;
	do
	{
		nbTouchedTris = PxMeshQuery::findOverlapHeightField(geom, geomPose, hfGeom, hfPose, mResultsMemory, mMaxNbResults, 0, overflow);
		if(overflow)
		{
			const PxU32 maxNbTris = mMaxNbResults * 2;

			if(mResultsMemory != mResults)
				PX_FREE(mResultsMemory);

			mResultsMemory = (PxU32*)PX_ALLOC(sizeof(PxU32)*maxNbTris, PX_DEBUG_EXP("PxMeshOverlapUtil::findOverlap"));
			mMaxNbResults = maxNbTris;
		}
	}while(overflow);

	mNbResults = nbTouchedTris;
	return nbTouchedTris;
}

PxVec3 physx::PxComputeMeshPenetration(PxU32 maxIter, const PxGeometry& geom, const PxTransform& geomPose, const PxTriangleMeshGeometry& meshGeom, const PxTransform& meshPose, PxU32& nb)
{
	PxU32 nbIter = 0;
	bool isValid = true;
	PxTransform pose = geomPose;
	while(isValid && nbIter<maxIter)
	{
		PxVec3 mtd;
		PxF32 depth;
		isValid = PxGeometryQuery::computePenetration(mtd, depth, geom, pose, meshGeom, meshPose);
		if(isValid)
		{
			nbIter++;
			PX_ASSERT(depth>=0.0f);
			pose.p += mtd * depth;
		}
	}
	nb = nbIter;
	return pose.p - geomPose.p;
}

PxVec3 physx::PxComputeHeightFieldPenetration(PxU32 maxIter, const PxGeometry& geom, const PxTransform& geomPose, const PxHeightFieldGeometry& meshGeom, const PxTransform& meshPose, PxU32& nb)
{
	PxU32 nbIter = 0;
	bool isValid = true;
	PxTransform pose = geomPose;
	while(isValid && nbIter<maxIter)
	{
		PxVec3 mtd;
		PxF32 depth;
		isValid = PxGeometryQuery::computePenetration(mtd, depth, geom, pose, meshGeom, meshPose);
		if(isValid)
		{
			nbIter++;
			PX_ASSERT(depth>=0.0f);
			pose.p += mtd * depth;
		}
	}
	nb = nbIter;
	return pose.p - geomPose.p;
}
