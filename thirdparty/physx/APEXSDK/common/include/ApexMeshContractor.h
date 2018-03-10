/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_MESH_CONTRACTOR_H
#define APEX_MESH_CONTRACTOR_H

#include "NxApexDefs.h"
#include "PsShare.h"
#include "PsArray.h"
#include "foundation/PxVec3.h"
#include "PsUserAllocated.h"

namespace physx
{
namespace apex
{

class IProgressListener;

class ApexMeshContractor : public UserAllocated
{
public:
	ApexMeshContractor();

	void registerVertex(const physx::PxVec3& pos);
	void registerTriangle(physx::PxU32 v0, physx::PxU32 v1, physx::PxU32 v2);
	bool endRegistration(physx::PxU32 subdivision, IProgressListener* progress);

	physx::PxU32 contract(physx::PxI32 steps, physx::PxF32 abortionRatio, physx::PxF32& volumeRatio, IProgressListener* progress);
	void expandBorder();

	physx::PxU32 getNumVertices()
	{
		return mVertices.size();
	}
	physx::PxU32 getNumIndices()
	{
		return mIndices.size();
	}
	const physx::PxVec3* getVertices()
	{
		return mVertices.begin();
	}
	const physx::PxU32* getIndices()
	{
		return mIndices.begin();
	}
private:

	void computeNeighbours();
	void computeSignedDistanceField();
	void contractionStep();
	void computeAreaAndVolume(physx::PxF32& area, physx::PxF32& volume);

	void addTriangle(const physx::PxVec3& v0, const physx::PxVec3& v1, const physx::PxVec3& v2);
	bool updateDistance(physx::PxU32 xi, physx::PxU32 yi, physx::PxU32 zi);
	void setInsideOutside();
	void interpolateGradientAt(const physx::PxVec3& pos, physx::PxVec3& grad);
	void subdivide(physx::PxF32 spacing);
	void collapse(float spacing);

	void getButterfly(physx::PxU32 triNr, physx::PxU32 v0, physx::PxU32 v1, physx::PxI32& adj, physx::PxI32& t0, physx::PxI32& t1, physx::PxI32& t2, physx::PxI32& t3) const;
	physx::PxI32 getOppositeVertex(physx::PxI32 t, physx::PxU32 v0, physx::PxU32 v1) const;
	void replaceVertex(physx::PxI32 t, physx::PxU32 vOld, physx::PxU32 vNew);
	void replaceNeighbor(physx::PxI32 t, physx::PxI32 nOld, physx::PxU32 nNew);
	bool triangleContains(physx::PxI32 t, physx::PxU32 v) const;
	bool legalCollapse(physx::PxI32 triNr, physx::PxU32 v0, physx::PxU32 v1) const;
	void advanceAdjTriangle(physx::PxU32 v, physx::PxI32& t, physx::PxI32& prev) const;
	bool areNeighbors(physx::PxI32 t0, physx::PxI32 t1) const;
	physx::PxF32 findMin(const physx::PxVec3& p, const physx::PxVec3& maxDisp) const;
	physx::PxF32 interpolateDistanceAt(const physx::PxVec3& pos) const;
	void collectNeighborhood(physx::PxI32 triNr, physx::PxF32 radius, physx::PxU32 newMark, physx::Array<physx::PxI32> &tris, physx::Array<physx::PxF32> &dists, physx::PxU32* triMarks) const;
	void getTriangleCenter(physx::PxI32 triNr, physx::PxVec3& center) const;
	float curvatureAt(int triNr, int v);

	struct ContractorCell
	{
		ContractorCell() : inside(0), distance(PX_MAX_F32), marked(false)
		{
			numCuts[0] = numCuts[1] = numCuts[2] = 0;
		}
		/*
		void init() {
			distance = PX_MAX_F32;
			inside = 0;
			marked = false;
			numCuts[0] = 0;
			numCuts[1] = 0;
			numCuts[2] = 0;
		}
		*/
		physx::PxU32 inside;
		physx::PxF32 distance;
		physx::PxU8 numCuts[3];
		bool marked;
	};
	inline ContractorCell& cellAt(physx::PxI32 xi, physx::PxI32 yi, physx::PxI32 zi)
	{
		return mGrid[(((physx::PxU32)xi * mNumY) + (physx::PxU32)yi) * mNumZ + (physx::PxU32)zi];
	}

	inline const ContractorCell& constCellAt(physx::PxI32 xi, physx::PxI32 yi, physx::PxI32 zi) const
	{
		return mGrid[(((physx::PxU32)xi * mNumY) + (physx::PxU32)yi) * mNumZ + (physx::PxU32)zi];
	}
	physx::PxF32 mCellSize;
	physx::PxVec3 mOrigin;

	physx::PxU32 mNumX, mNumY, mNumZ;

	physx::Array<physx::PxVec3> mVertices;
	physx::Array<physx::PxU32> mIndices;
	physx::Array<physx::PxI32> mNeighbours;

	physx::Array<ContractorCell> mGrid;
	physx::Array<float> mVertexCurvatures;

	physx::PxF32 mInitialVolume;
	physx::PxF32 mCurrentVolume;
};

}
} // end namespace physx::apex

#endif // APEX_MESH_CONTRACTOR_H
