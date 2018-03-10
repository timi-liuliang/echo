/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "CookingPhysX3.h"

#include <PsArray.h>
#include <PsMathUtils.h>
#include <PsSort.h>
#include <Ps.h>

#include <NiApexSDK.h>

#include "ClothingCookedPhysX3Param.h"

#include "ExtClothFabricCooker.h"
#include "ExtClothMeshQuadifier.h"

#include "ModuleClothingHelpers.h"

#include <ctime>

namespace
{
	using namespace physx;
	struct VirtualParticle
	{
		VirtualParticle(PxU32 i0, PxU32 i1, PxU32 i2)
		{
			indices[0] = i0;
			indices[1] = i1;
			indices[2] = i2;
			tableIndex = 0;
		}

		void rotate(PxU32 count)
		{
			while (count--)
			{
				const PxU32 temp = indices[2];
				indices[2] = indices[1];
				indices[1] = indices[0];
				indices[0] = temp;
			}
		}

		PxU32 indices[3];
		PxU32 tableIndex;
	};

	struct EdgeAndLength
	{
		EdgeAndLength(PxU32 edgeNumber, PxF32 length) : mEdgeNumber(edgeNumber), mLength(length) {}
		PxU32 mEdgeNumber;
		PxF32 mLength;

		bool operator<(const EdgeAndLength& other) const
		{
			return mLength < other.mLength;
		}
	};
}

namespace physx
{
namespace apex
{
namespace clothing
{

bool CookingPhysX3::mTetraWarning = false;

NxParameterized::Interface* CookingPhysX3::execute()
{

	ClothingCookedPhysX3Param* rootCookedData = NULL;

	for (PxU32 subMeshIndex = 0; subMeshIndex < mPhysicalSubmeshes.size(); subMeshIndex++)
	{
		const PxU32 physicalMeshIndex = mPhysicalSubmeshes[subMeshIndex].meshID;

		if (mPhysicalMeshes[physicalMeshIndex].isTetrahedral)
		{
			if (!mTetraWarning)
			{
				mTetraWarning = true;
				APEX_INVALID_OPERATION("Tetrahedral meshes are not (yet) supported with the 3.x solver");
			}
			continue;
		}

		ClothingCookedPhysX3Param* cookedData = NULL;

		cookedData = fiberCooker(subMeshIndex);

		computeVertexWeights(cookedData, subMeshIndex);
		fillOutSetsDesc(cookedData);

		createVirtualParticles(cookedData, subMeshIndex);
		createSelfcollisionIndices(cookedData, subMeshIndex);

		if (rootCookedData == NULL)
		{
			rootCookedData = cookedData;
		}
		else
		{
			ClothingCookedPhysX3Param* addCookedData = rootCookedData;
			while (addCookedData != NULL && addCookedData->nextCookedData != NULL)
			{
				addCookedData = static_cast<ClothingCookedPhysX3Param*>(addCookedData->nextCookedData);
			}
			addCookedData->nextCookedData = cookedData;
		}
	}

	return rootCookedData;
}



ClothingCookedPhysX3Param* CookingPhysX3::fiberCooker(PxU32 subMeshIndex) const
{

	const PxU32 physicalMeshIndex = mPhysicalSubmeshes[subMeshIndex].meshID;
	const PxU32 numVertices = mPhysicalSubmeshes[subMeshIndex].numVertices;
	const PxU32 numAttached = mPhysicalSubmeshes[subMeshIndex].numMaxDistance0Vertices;

	shdfnd::Array<PxVec4> vertices(numVertices);
	for (PxU32 i = 0; i < numVertices; i++)
		vertices[i] = PxVec4(mPhysicalMeshes[physicalMeshIndex].vertices[i], 1.0f);

	if (numAttached > 0)
	{
		const PxU32 start = mPhysicalSubmeshes[subMeshIndex].numVertices - numAttached;
		for (PxU32 i = start; i < numVertices; i++)
			vertices[i].w = 0.0f;
	}

	PxClothMeshDesc desc;

	desc.points.data = vertices.begin();
	desc.points.count = numVertices;
	desc.points.stride = sizeof(PxVec4);

	desc.invMasses.data = &vertices.begin()->w;
	desc.invMasses.count = numVertices;
	desc.invMasses.stride = sizeof(PxVec4);

	desc.triangles.data = mPhysicalMeshes[physicalMeshIndex].indices;
	desc.triangles.count = mPhysicalSubmeshes[subMeshIndex].numIndices / 3;
	desc.triangles.stride = sizeof(PxU32) * 3;

	PxClothMeshQuadifier quadifier(desc);

	PxClothFabricCooker cooker(quadifier.getDescriptor(), mGravityDirection);
	
	PxClothFabricDesc fabric = cooker.getDescriptor(); 

	PxI32 nbConstraints = (physx::PxI32)fabric.sets[fabric.nbSets - 1];

	ClothingCookedPhysX3Param* cookedData = NULL;

	bool success = true;
	if (success)
	{
		cookedData = static_cast<ClothingCookedPhysX3Param*>(NiGetApexSDK()->getParameterizedTraits()->createNxParameterized(ClothingCookedPhysX3Param::staticClassName()));

		NxParameterized::Handle arrayHandle(cookedData);
		arrayHandle.getParameter("deformableIndices");
		arrayHandle.resizeArray(nbConstraints * 2);
		arrayHandle.setParamU32Array(fabric.indices, nbConstraints * 2);

		arrayHandle.getParameter("deformableRestLengths");
		arrayHandle.resizeArray(nbConstraints);
		arrayHandle.setParamF32Array(fabric.restvalues, nbConstraints);

		arrayHandle.getParameter("deformableSets");
		const PxI32 numSets = (physx::PxI32)fabric.nbSets;
		arrayHandle.resizeArray(numSets);
		for (PxI32 i = 0; i < numSets; i++)
		{
			arrayHandle.set(i);
			arrayHandle.set(0);
			arrayHandle.setParamU32(fabric.sets[(physx::PxU32)i]);
			arrayHandle.popIndex();
			arrayHandle.popIndex();
		}

		arrayHandle.getParameter("deformablePhaseDescs");
		arrayHandle.resizeArray((physx::PxI32)fabric.nbPhases);

		for (PxU32 i = 0; i < fabric.nbPhases; i++)
		{
			PxClothFabricPhase phase = fabric.phases[i];
			cookedData->deformablePhaseDescs.buf[i].phaseType = phase.phaseType;
			cookedData->deformablePhaseDescs.buf[i].setIndex = phase.setIndex;
		}

		arrayHandle.getParameter("tetherAnchors");
		arrayHandle.resizeArray((physx::PxI32)fabric.nbTethers);
		arrayHandle.setParamU32Array(fabric.tetherAnchors, (physx::PxI32)fabric.nbTethers);

		arrayHandle.getParameter("tetherLengths");
		arrayHandle.resizeArray((physx::PxI32)fabric.nbTethers);
		arrayHandle.setParamF32Array(fabric.tetherLengths, (physx::PxI32)fabric.nbTethers);

		cookedData->physicalMeshId = physicalMeshIndex;
		cookedData->physicalSubMeshId = mPhysicalSubmeshes[subMeshIndex].submeshID;
		cookedData->numVertices = mPhysicalSubmeshes[subMeshIndex].numVertices;

		//dumpObj("c:\\lastCooked.obj", subMeshIndex);
		//dumpApx("c:\\lastCooked.apx", cookedData);

		cookedData->cookedDataVersion = getCookingVersion();
	}
	else
	{
#ifdef PX_WINDOWS
		static int failureCount = 0;
		char buf[64];
		sprintf_s(buf, 64, "c:\\cookingFailure_%d.obj", failureCount++);
		dumpObj(buf, subMeshIndex);

		APEX_INTERNAL_ERROR("Fiber cooking failure (mesh %d, submesh %d), the failing mesh has been dumped to \'%s\'", physicalMeshIndex, mPhysicalSubmeshes[subMeshIndex].submeshID, buf);
#else
		APEX_INTERNAL_ERROR("Fiber cooking failure (mesh %d, submesh %d)", physicalMeshIndex, mPhysicalSubmeshes[subMeshIndex].submeshID);
#endif

	}


	return cookedData;
}

void CookingPhysX3::computeVertexWeights(ClothingCookedPhysX3Param* cookedData, PxU32 subMeshIndex) const
{
	const PxU32*  indices =   mPhysicalMeshes[cookedData->physicalMeshId].indices;
	const PxVec3* positions = mPhysicalMeshes[cookedData->physicalMeshId].vertices;
	const PxU32 numIndices =  mPhysicalSubmeshes[subMeshIndex].numIndices;
	const PxU32 numVertices = mPhysicalSubmeshes[subMeshIndex].numVertices;

	shdfnd::Array<PxF32> weights(numVertices, 0.0f);

	PX_ASSERT(numIndices % 3 == 0);
	for (PxU32 i = 0; i < numIndices; i += 3)
	{
		const PxVec3 v1 = positions[indices[i + 1]] - positions[indices[i]];
		const PxVec3 v2 = positions[indices[i + 2]] - positions[indices[i]];
		const PxF32 area = v1.cross(v2).magnitude();

		for (PxU32 j = 0; j < 3; j++)
		{
			weights[indices[i + j]] += area;
		}
	}

	PxF32 weightSum = 0.0f;
	for (PxU32 i = 0; i < numVertices; i++)
	{
		weightSum += weights[i];
	}

	const PxF32 weightScale = (PxF32)numVertices / weightSum;

	for (PxU32 i = 0; i < numVertices; i++)
	{
		weights[i] *= weightScale;
	}

	NxParameterized::Handle handle(*cookedData, "deformableInvVertexWeights");
	if (handle.resizeArray((physx::PxI32)numVertices) == NxParameterized::ERROR_NONE)
	{
		for (PxU32 i = 0; i < numVertices; i++)
		{
			cookedData->deformableInvVertexWeights.buf[i] = 1.0f / weights[i];
		}
	}
}



void CookingPhysX3::createVirtualParticles(ClothingCookedPhysX3Param* cookedData, PxU32 subMeshIndex)
{
	const PxVec3* positions = mPhysicalMeshes[cookedData->physicalMeshId].vertices;
	const PxU32*  indices =   mPhysicalMeshes[cookedData->physicalMeshId].indices;
	const PxU32 numIndices =  mPhysicalSubmeshes[subMeshIndex].numIndices;

	shdfnd::Array<VirtualParticle> particles;

	const PxF32 minTriangleArea = mVirtualParticleDensity * mPhysicalMeshes[cookedData->physicalMeshId].smallestTriangleArea / 2.0f +
	                              (1.0f - mVirtualParticleDensity) * mPhysicalMeshes[cookedData->physicalMeshId].largestTriangleArea;
	const PxF32 coveredTriangleArea = minTriangleArea;

	for (PxU32 i = 0; i < numIndices; i += 3)
	{
		VirtualParticle particle(indices[i], indices[i + 1], indices[i + 2]);

		const PxVec3 edge1 = positions[particle.indices[1]] - positions[particle.indices[0]];
		const PxVec3 edge2 = positions[particle.indices[2]] - positions[particle.indices[0]];
		const PxF32 triangleArea = edge1.cross(edge2).magnitude();

		const PxF32 numSpheres = triangleArea / coveredTriangleArea;

		if (numSpheres <= 1.0f)
		{
			// do nothing
		}
		else if (numSpheres < 2.0f)
		{
			// add one virtual particle
			particles.pushBack(particle);
		}
		else
		{
			// add two or three, depending on whether it's a slim triangle.
			EdgeAndLength eal0(0, edge1.magnitude());
			EdgeAndLength eal1(1, (positions[particle.indices[2]] - positions[particle.indices[1]]).magnitude());
			EdgeAndLength eal2(2, edge2.magnitude());
			EdgeAndLength middle = eal0 < eal1 ? eal0 : eal1; // technically this does not have to be the middle of the three, but for the test below it suffices.
			EdgeAndLength smallest = middle < eal2 ? middle : eal2;
			if (smallest.mLength * 2.0f < middle.mLength)
			{
				// two
				particle.rotate(smallest.mEdgeNumber);
				particle.tableIndex = 2;
				particles.pushBack(particle);
				particle.tableIndex = 3;
				particles.pushBack(particle);
			}
			else
			{
				// three
				particle.tableIndex = 1;
				particles.pushBack(particle);
				particle.rotate(1);
				particles.pushBack(particle);
				particle.rotate(1);
				particles.pushBack(particle);
			}
		}
	}

	if (!particles.empty())
	{
		NxParameterized::Handle handle(cookedData);
		handle.getParameter("virtualParticleIndices");
		handle.resizeArray((physx::PxI32)particles.size() * 4);
		handle.getParameter("virtualParticleWeights");
		handle.resizeArray(3 * 4);

		// table index 0, the center particle
		cookedData->virtualParticleWeights.buf[0] = 1.0f / 3.0f;
		cookedData->virtualParticleWeights.buf[1] = 1.0f / 3.0f;
		cookedData->virtualParticleWeights.buf[2] = 1.0f / 3.0f;

		// table index 1, three particles
		cookedData->virtualParticleWeights.buf[3] = 0.1f;
		cookedData->virtualParticleWeights.buf[4] = 0.3f;
		cookedData->virtualParticleWeights.buf[5] = 0.6f;

		// table index 2, the pointy particle
		cookedData->virtualParticleWeights.buf[6] = 0.7f;
		cookedData->virtualParticleWeights.buf[7] = 0.15f;
		cookedData->virtualParticleWeights.buf[8] = 0.15f;

		// table index 3, the flat particle
		cookedData->virtualParticleWeights.buf[9] = 0.3f;
		cookedData->virtualParticleWeights.buf[10] = 0.35f;
		cookedData->virtualParticleWeights.buf[11] = 0.35f;

		for (PxU32 i = 0; i < particles.size(); i++)
		{
			for (PxU32 j = 0; j < 3; j++)
			{
				cookedData->virtualParticleIndices.buf[4 * i + j] = particles[i].indices[j];
			}
			cookedData->virtualParticleIndices.buf[4 * i + 3] = particles[i].tableIndex; // the table index
		}
	}
}


void CookingPhysX3::createSelfcollisionIndices(ClothingCookedPhysX3Param* cookedData, PxU32 subMeshIndex) const
{
	const PxVec3* positions = mPhysicalMeshes[cookedData->physicalMeshId].vertices;
	const PxU32 numVertices = mPhysicalSubmeshes[subMeshIndex].numVertices;


	// we'll start with a full set of indices, and eliminate the ones we don't want. selfCollisionIndices
	//  is an array of indices, i.e. a second layer of indirection
	Array<PxU32> selfCollisionIndices;
	for (PxU32 i = 0; i < numVertices; ++i)
	{
		selfCollisionIndices.pushBack(i);
	}

	physx::PxF32 selfcollisionThicknessSq = mSelfcollisionRadius * mSelfcollisionRadius;
	for (PxU32 v0ii = 0; v0ii < selfCollisionIndices.size(); ++v0ii)
	{
		// ii suffix means "index into indices array", i suffix just means "index into vertex array"

		// load the first vertex
		PxU32 v0i = selfCollisionIndices[v0ii];
		const physx::PxVec3& v0 = positions[v0i];

		// no need to start at the beginning of the array, those comparisons have already been made.
		// don't autoincrement the sequence index. if we eliminate an index, we'll replace it with one from
		//  the end, and reevaluate that element
		for (PxU32 v1ii = v0ii + 1; v1ii < selfCollisionIndices.size(); ) // don't autoincrement iteratorsee if/else
		{
			PxU32 v1i = selfCollisionIndices[v1ii];
			const physx::PxVec3& v1 = positions[v1i];

			// how close is this particle?
			PxReal v0v1DistanceSq = (v0 - v1).magnitudeSquared();
			if (v0v1DistanceSq < selfcollisionThicknessSq )
			{
				// too close for missiles
				selfCollisionIndices.replaceWithLast(v1ii);

				// don't move on to the next - replaceWithLast put a fresh index at v1ii, so reevaluate it
			}
			else
			{
				// it's comfortably distant, so we'll keep it around (for now). 

				// we need to be mindful of which element we visit next in the outer loop. we want to minimize the distance between 
				//  self colliding particles and not unnecessarily introduce large gaps between them. the easiest way is to pick
				//  the closest non-eliminated particle to the one currently being evaluated, and evaluate it next. if we find one
				//  that's closer than what's currently next in the list, swap it. both of these elements are prior to the next 
				//  sinner-loop element, so this doesn't impact the inner loop traversal

				// if we assume the index of the closest known particle is always v0ii + 1, we can just reevaluate it's distance to 
				//  v0ii every iteration. slightly expensive, but it eliminates the need to maintain redundant 
				//  ClosestDistance/ClosestIndex variables
				PxU32 vNexti = selfCollisionIndices[v0ii + 1];
				const physx::PxVec3& nextVertexToEvaluate = positions[vNexti];

				PxReal v0vNextDistanceSq = (v0 - nextVertexToEvaluate).magnitudeSquared();
				if (v0v1DistanceSq < v0vNextDistanceSq)
				{
					physx::swap(selfCollisionIndices[v0ii + 1], selfCollisionIndices[v1ii]);
				}

				// move on to the next
				++v1ii;
			}
		}
	}

	NxParameterized::Handle arrayHandle(cookedData);
	arrayHandle.getParameter("selfCollisionIndices");
	arrayHandle.resizeArray((physx::PxI32)selfCollisionIndices.size());
	arrayHandle.setParamU32Array(selfCollisionIndices.begin(), (physx::PxI32)selfCollisionIndices.size());
}


bool CookingPhysX3::verifyValidity(const ClothingCookedPhysX3Param* cookedData, PxU32 subMeshIndex)
{
	if (cookedData == NULL)
	{
		return false;
	}

	const char* errorMessage = NULL;

	const PxU32 numSetsDescs			= (physx::PxU32)cookedData->deformableSets.arraySizes[0];
	const PxU32 numDeformableVertices	= mPhysicalSubmeshes[subMeshIndex].numVertices;

	for (PxU32 validSetsDescs = 0; validSetsDescs < numSetsDescs && errorMessage == NULL; ++validSetsDescs)
	{
		const PxU32 fromIndex = validSetsDescs ? cookedData->deformableSets.buf[validSetsDescs - 1].fiberEnd : 0;
		const PxU32 toIndex = cookedData->deformableSets.buf[validSetsDescs].fiberEnd;
		if (toIndex <= fromIndex)
		{
			errorMessage = "Set without fibers";
		}

		for (PxU32 f = fromIndex; f < toIndex && errorMessage == NULL; ++f)
		{
			PxU32	posIndex1	= cookedData->deformableIndices.buf[2 * f];
			PxU32	posIndex2	= cookedData->deformableIndices.buf[2 * f + 1];

			if (posIndex2 > (PxU32)cookedData->deformableIndices.arraySizes[0])
			{
				errorMessage = "Fiber index out of bounds";
			}
	
			if (posIndex1 >= numDeformableVertices)
			{
				errorMessage = "Deformable index out of bounds";
			}
		}
	}

	if (errorMessage != NULL)
	{
		APEX_INTERNAL_ERROR("Invalid cooked data: %s", errorMessage);
	}

	return (errorMessage == NULL);
}




void CookingPhysX3::fillOutSetsDesc(ClothingCookedPhysX3Param* cookedData)
{
	const PxVec3* vertices = mPhysicalMeshes[cookedData->physicalMeshId].vertices;
	for (PxI32 sd = 0; sd < cookedData->deformableSets.arraySizes[0]; sd++)
	{
		const PxU32 firstFiber = sd ? cookedData->deformableSets.buf[sd - 1].fiberEnd : 0;
		const PxU32 lastFiber = cookedData->deformableSets.buf[sd].fiberEnd;

		PxU32 numEdges = 0;
		PxF32 avgEdgeLength = 0.0f;

		for (PxU32 f = firstFiber; f < lastFiber; f++)
		{
			PxU32 from = cookedData->deformableIndices.buf[f * 2];
			PxU32 to = cookedData->deformableIndices.buf[f*2+1];
			numEdges ++;
			avgEdgeLength += (vertices[to] - vertices[from]).magnitude();
		}

		if (numEdges > 0)
		{
			cookedData->deformableSets.buf[sd].longestFiber = 0;
			cookedData->deformableSets.buf[sd].shortestFiber = 0;
			cookedData->deformableSets.buf[sd].numEdges = numEdges;
			cookedData->deformableSets.buf[sd].avgFiberLength = 0;
			cookedData->deformableSets.buf[sd].avgEdgeLength = avgEdgeLength / (PxF32)numEdges;
		}
	}
}



void CookingPhysX3::groupPhases(ClothingCookedPhysX3Param* cookedData, PxU32 subMeshIndex, PxU32 startIndex, PxU32 endIndex, Array<PxU32>& phaseEnds) const
{
	shdfnd::Array<bool> usedInPhase(mPhysicalSubmeshes[subMeshIndex].numVertices, false);
	for (PxU32 f = startIndex; f < endIndex; f++)
	{
		PxU32 index1 = cookedData->deformableIndices.buf[2 * f + 0];
		PxU32 index2 = cookedData->deformableIndices.buf[2 * f + 1];

		if (usedInPhase[index1] || usedInPhase[index2])
		{
			bool swapped = false;

			// need to replace this with one further ahead
			for (PxU32 scanAhead = f + 1; scanAhead < endIndex; scanAhead++)
			{
				const PxU32 i1 = cookedData->deformableIndices.buf[2 * scanAhead + 0];
				const PxU32 i2 = cookedData->deformableIndices.buf[2 * scanAhead + 1];
				if (!usedInPhase[i1] && !usedInPhase[i2])
				{
					// swap
					cookedData->deformableIndices.buf[2 * f + 0] = i1;
					cookedData->deformableIndices.buf[2 * f + 1] = i2;

					cookedData->deformableIndices.buf[2 * scanAhead + 0] = index1;
					cookedData->deformableIndices.buf[2 * scanAhead + 1] = index2;

					shdfnd::swap(cookedData->deformableRestLengths.buf[2 * f], cookedData->deformableRestLengths.buf[2 * scanAhead]);

					index1 = i1;
					index2 = i2;

					swapped = true;

					break;
				}
			}

			if (!swapped)
			{
				phaseEnds.pushBack(f);
				f--;

				for (PxU32 i = 0; i < usedInPhase.size(); i++)
				{
					usedInPhase[i] = false;
				}

				continue;
			}
		}

		usedInPhase[index1] = true;
		usedInPhase[index2] = true;
	}
	phaseEnds.pushBack(endIndex);
}



void CookingPhysX3::dumpObj(const char* filename, PxU32 subMeshIndex) const
{
	PX_UNUSED(filename);
	PX_UNUSED(subMeshIndex);
#ifdef PX_WINDOWS
	FILE* outputFile = NULL;
	fopen_s(&outputFile, filename, "w");

	if (outputFile == NULL)
	{
		return;
	}

	fprintf(outputFile, "# PhysX3 Cooking input mesh\n");

	const PxU32 physicalMeshIndex = mPhysicalSubmeshes[subMeshIndex].meshID;
	fprintf(outputFile, "# Mesh %d, Submesh %d\n", physicalMeshIndex, mPhysicalSubmeshes[subMeshIndex].submeshID);

	{
		time_t rawtime;
		struct tm* timeinfo;

		time(&rawtime);
		timeinfo = localtime(&rawtime);
		fprintf(outputFile, "# File Created: %s", asctime(timeinfo));
	}

	fprintf(outputFile, "\n\n\n");

	const PxU32 numVertices = mPhysicalSubmeshes[subMeshIndex].numVertices;
	const PxU32 numIndices = mPhysicalSubmeshes[subMeshIndex].numIndices;

	const PxVec3* vert = mPhysicalMeshes[physicalMeshIndex].vertices;
	for (PxU32 i = 0; i < numVertices; i++)
	{
		fprintf(outputFile, "v %f %f %f\n", vert[i].x, vert[i].y, vert[i].z);
	}

	fprintf(outputFile, "\n\n\n");

	const PxU32* indices = mPhysicalMeshes[physicalMeshIndex].indices;
	for (PxU32 i = 0; i < numIndices; i += 3)
	{
		fprintf(outputFile, "f %d %d %d\n", indices[i] + 1, indices[i + 1] + 1, indices[i + 2] + 1);
	}

	fclose(outputFile);
#endif
}



void CookingPhysX3::dumpApx(const char* filename, const NxParameterized::Interface* data) const
{
	NxParameterized::Serializer::SerializeType serType = NxParameterized::Serializer::NST_XML;

	if (data == NULL)
	{
		return;
	}

	physx::PxFileBuf* filebuffer = NiGetApexSDK()->createStream(filename, physx::PxFileBuf::OPEN_WRITE_ONLY);

	if (filebuffer != NULL)
	{
		if (filebuffer->isOpen())
		{
			NxParameterized::Serializer* serializer = NiGetApexSDK()->createSerializer(serType);
			serializer->serialize(*filebuffer, &data, 1);

			serializer->release();
		}

		filebuffer->release();
		filebuffer = NULL;
	}
}

}
}
}
