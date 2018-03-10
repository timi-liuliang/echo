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
#ifndef PXS_FLUID_DYNAMIC_HELPER_H
#define PXS_FLUID_DYNAMIC_HELPER_H

#include "PxsFluidDynamicsKernels.h"
#include "PxsFluidSpatialHash.h"
#include "PxsFluidDynamicsTempBuffers.h"

namespace physx
{

//-------------------------------------------------------------------------------------------------------------------//

PX_FORCE_INLINE void updateParticlesPrePass(const PxsSphUpdateType updateType, PxVec3* forceBuf, PxsFluidParticle* particles, PxU32 numParticles, const PxsFluidDynamicsParameters& params)
{
	if (updateType == PXS_SPH_DENSITY)
	{
		for (PxU32 i = 0; i < numParticles; ++i)
		{
			PxsFluidParticle& particle = particles[i];

			// Initialize particle densities with self density value
			particle.density = params.selfDensity;
			forceBuf[i] = PxVec3(0);
		}
	}
}

//-------------------------------------------------------------------------------------------------------------------//

PX_FORCE_INLINE void updateParticlesPostPass(const PxsSphUpdateType updateType, PxVec3* forceBuf, PxsFluidParticle* particles, PxU32 numParticles, const PxsFluidDynamicsParameters& params)
{
	if (updateType == PXS_SPH_FORCE)
	{
		for (PxU32 i = 0; i < numParticles; ++i)
		{
			PxsFluidParticle& particle = particles[i];

			forceBuf[i] *= params.scaleToWorld*(1.0f / particle.density);	
		}
	}
}

//-------------------------------------------------------------------------------------------------------------------//

/*!
Given a cell hash table, find neighboring cells and compute particle interactions.
*/
void updateCellsSubpacket(PxsSphUpdateType updateType, PxVec3* __restrict forceBuf, PxsFluidParticle* __restrict particles, const PxsParticleCell* __restrict cells,
						  const PxU32* __restrict particleIndices, const PxU32 numCellHashBuckets, const PxsFluidDynamicsParameters& params,
						  PxsFluidDynamicsTempBuffers& tempBuffers)
{
	PX_ASSERT(particles);
	PX_ASSERT(cells);
	PX_ASSERT(particleIndices);

	const PxsParticleCell* neighborCells[13];

	for(PxU32 c=0; c < numCellHashBuckets; c++)
	{
		const PxsParticleCell& cell = cells[c];

		if (cell.numParticles == PX_INVALID_U32)
			continue;

		PxcGridCellVector coords(cell.coords);

		//
		// To process each pair of neighboring cells only once, a special neighborhood layout can be
		// used. Thus, we do not need to consider all 26 neighbors of a cell but only half of them.
		// Going through the list of cells, a cell X might not be aware of a neighboring cell Y with
		// this layout, however, since cell Y in turn is aware of cell X the pair will still be processed
		// at the end.
		//

		// Complete back plane
		PxU32 cellIdx;

		PxI16 neighbor[13][3] = { {-1, -1, -1}, {0, -1, -1}, {1, -1, -1},
								{-1,  0, -1}, { 0,  0, -1},{1,  0, -1},
								{-1,  1, -1}, { 0,  1, -1},{1,  1, -1},
								{1,  0, 0},	{-1,  1, 0}, {0,  1, 0}, {1,  1, 0}	};

		for(PxU32 n=0; n < 13; n++)
		{
			neighborCells[n] = PxsFluidSpatialHash::findConstCell(cellIdx, PxcGridCellVector(coords.x + neighbor[n][0], coords.y + neighbor[n][1], coords.z + neighbor[n][2]), cells, numCellHashBuckets);
		}
		
		// Compute interaction between particles inside the current cell
		// These calls still produce a lot of LHS. Going from two way to one way updates didn't help. TODO, more investigation.
		for(PxU32 p=1; p < cell.numParticles; p++)
		{
			updateParticleGroupPair(forceBuf, forceBuf, particles, particles, 
				particleIndices + cell.firstParticle + p - 1, 1, 
				particleIndices + cell.firstParticle + p, cell.numParticles - p, 
				true, updateType == PXS_SPH_DENSITY, params, 
				tempBuffers.simdPositionsSubpacket, tempBuffers.indexStream);
		}

		// Compute interaction between particles of current cell and neighboring cells
		PxU32 srcIndexCount = 0;

		for(PxU32 n=0; n < 13; n++)
		{
			if (!neighborCells[n])
				continue;

			const PxsParticleCell* nCell = neighborCells[n];

			for (PxU32 i = nCell->firstParticle, end = nCell->firstParticle + nCell->numParticles; i < end; i++)
				tempBuffers.mergedIndices[srcIndexCount++] = particleIndices[i];
		}

		if (srcIndexCount > 0)
		{
			updateParticleGroupPair(forceBuf, forceBuf, particles, particles,
				particleIndices + cell.firstParticle, cell.numParticles,
				tempBuffers.mergedIndices, srcIndexCount, 
				true, updateType == PXS_SPH_DENSITY, params, 
				tempBuffers.simdPositionsSubpacket, tempBuffers.indexStream);
		}
	}
}

//-------------------------------------------------------------------------------------------------------------------//

/*!
Given two subpackets, i.e., their cell hash tables and particle arrays, find for each cell of the first subpacket
the neighboring cells within the second subpacket and compute particle interactions for these neighboring cells.
*/
void updateCellsSubpacketPair(PxsSphUpdateType updateType, PxVec3* __restrict forceBufA, PxVec3* __restrict forceBufB, PxsFluidParticle* __restrict particlesSpA, PxsFluidParticle* __restrict particlesSpB,
							  const PxsParticleCell* __restrict cellsSpA, const PxsParticleCell* __restrict cellsSpB,
							  const PxU32* __restrict particleIndicesSpA, const PxU32* __restrict particleIndicesSpB,
							  const PxU32 numCellHashBucketsA, const PxU32 numCellHashBucketsB, bool twoWayUpdate,
							  const PxsFluidDynamicsParameters& params, PxsFluidDynamicsTempBuffers& tempBuffers, bool swapAB)
{
	PX_ASSERT(particlesSpA);
	PX_ASSERT(particlesSpB);
	PX_ASSERT(cellsSpA);
	PX_ASSERT(cellsSpB);
	PX_ASSERT(particleIndicesSpA);
	PX_ASSERT(particleIndicesSpB);

	const PxsParticleCell* __restrict srcCell;
	const PxsParticleCell* __restrict dstCell;
	const PxU32* __restrict dstIndices;
	PxU32 srcBuckets,dstBuckets;

	if(swapAB)
	{
		srcCell = cellsSpB;
		srcBuckets = numCellHashBucketsB;

		dstCell = cellsSpA;
		dstIndices = particleIndicesSpA;
		dstBuckets = numCellHashBucketsA;
	}
	else
	{
		srcCell = cellsSpA;
		srcBuckets = numCellHashBucketsA;

		dstCell = cellsSpB;
		dstIndices = particleIndicesSpB;
		dstBuckets = numCellHashBucketsB;
	}


	const PxsParticleCell* neighborCells[27];

	// For the cells of the subpacket A find neighboring cells in the subpacket B.
	const PxsParticleCell* pcell_end = srcCell + srcBuckets;
	for(const PxsParticleCell* pcell = srcCell; pcell < pcell_end; pcell++)
	{
		if(pcell->numParticles != PX_INVALID_U32)
		{
			PxcGridCellVector coords(pcell->coords);

			//
			// Check the 26 neighboring cells plus the cell with the same coordinates but inside the other subpacket
			//

			// Back plane
			PxU32 cellIdx;
			PxI16 neighbor[27][3] = { {-1, -1, -1}, {0, -1, -1}, {1, -1, -1},
								{-1,  0, -1}, { 0,  0, -1},{1,  0, -1},
								{-1,  1, -1}, { 0,  1, -1},{1,  1, -1},
								{-1, -1, 0}, {0, -1, 0}, {1, -1, 0},
								{-1,  0, 0}, {0,  0, 0}, {1,  0, 0},
								{-1,  1, 0}, {0,  1, 0}, {1,  1, 0},
								{-1, -1, 1}, {0, -1, 1}, {1, -1, 1},
								{-1,  0, 1}, {0,  0, 1}, {1,  0, 1},
								{-1,  1, 1}, {0,  1, 1}, {1,  1, 1}};

			for(PxU32 n=0; n < 27; n++)
			{
				neighborCells[n] = PxsFluidSpatialHash::findConstCell(cellIdx, PxcGridCellVector(coords.x + neighbor[n][0], coords.y + neighbor[n][1], coords.z + neighbor[n][2]), dstCell, dstBuckets);
			}

			// Compute interaction between particles of current cell and neighboring cells
			PxU32 indexCount = 0;

			for(PxU32 n=0; n < 27; n++)
			{
				if (!neighborCells[n])
					continue;

				const PxsParticleCell* nCell = neighborCells[n];

				for (PxU32 i = nCell->firstParticle, end = nCell->firstParticle + nCell->numParticles; i < end; i++)
					tempBuffers.mergedIndices[indexCount++] = dstIndices[i];
			}

			if (indexCount > 0)
			{

				if(swapAB)
				{
					updateParticleGroupPair(forceBufA, forceBufB, particlesSpA, particlesSpB,				
						tempBuffers.mergedIndices, indexCount,
						particleIndicesSpB + pcell->firstParticle, pcell->numParticles,
						twoWayUpdate, updateType == PXS_SPH_DENSITY, params, 
						tempBuffers.simdPositionsSubpacket,tempBuffers.indexStream);
				}
				else
				{
					updateParticleGroupPair(forceBufA, forceBufB, particlesSpA, particlesSpB,
						particleIndicesSpA + pcell->firstParticle, pcell->numParticles,
						tempBuffers.mergedIndices, indexCount,
						twoWayUpdate, updateType == PXS_SPH_DENSITY, params, 
						tempBuffers.simdPositionsSubpacket, tempBuffers.indexStream);
				}
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------------------------//

PX_FORCE_INLINE void normalizeParticleDensity(PxsFluidParticle& particle, const PxF32 selfDensity, const PxF32 densityNormalizationFactor)
{
	//normalize density
	particle.density = (particle.density - selfDensity) * densityNormalizationFactor;
}

//-------------------------------------------------------------------------------------------------------------------//

}

#endif // PXS_FLUID_DYNAMIC_HELPER_H
