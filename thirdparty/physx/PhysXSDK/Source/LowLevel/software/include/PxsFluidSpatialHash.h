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


#ifndef PXS_FLUID_SPATIAL_HASH_H
#define PXS_FLUID_SPATIAL_HASH_H

#include "PxsFluidConfig.h"
#include "PxsParticleCell.h"
#include "PxsFluidPacketSections.h"
#include "PxsFluidParticle.h"
#include "CmBitMap.h"
#include "PxcGridCellVector.h"
#include "PxsFluidSpatialHashHelper.h"

#if defined(PX_PS3) && !defined(__SPU__)
#include "CellFluidHashTask.h"
#include "CellFluidReorderPacketSectionsTask.h"
#endif

namespace physx
{

class PxBaseTask;

struct PxsFluidParticle;

// Structure describing the regions around a packet which are relevant for particle interactions.
// A packet has 26 neighbor packets:
// - 6 of these neighbors share a face with the packet. Each of these neighbors provide 9 halo regions.
// - 12 of these neighbors share an edge with the packet. Each of these neighbors provides 3 halo region.
// - 8 of these neighbors share a corner with the packet. Each of these neighbors provide 1 halo region.
//
// -> Number of halo regions for a packet:  6*9 + 12*3 + 8*1 = 98
#define PXS_FLUID_PACKET_HALO_REGIONS 98
struct PxsFluidPacketHaloRegions
{
	PxU32 numParticles[PXS_FLUID_PACKET_HALO_REGIONS];	//! Number of particles in each halo region
	PxU32 firstParticle[PXS_FLUID_PACKET_HALO_REGIONS];	//! Start index of the associated particle interval for each halo region
	PxU32 maxNumParticles;								//! Maximum number of particles among all halo regions
};


// Structure to cache a local cell hash that was computed for a set of particles
struct PxsFluidLocalCellHash
{
	PxU32						numParticles;		// Number of particles the cell hash is based on
	PxU32*						particleIndices;	// Particle indices (0..numParticles) with respect to the particle array that was used
													// to build the cell hash. Indices are ordered according to cells.
	PxU32						numHashEntries;		// Size of cell hash table
	PxsParticleCell*			hashEntries;		// Hash entry for cells

	bool						isHashValid;		// Marks whether the hash contains valid data or needs to be computed

	PxsFluidLocalCellHash()
	{
		numParticles = 0;
		particleIndices = NULL;
		numHashEntries = 0;
		hashEntries = NULL;
		isHashValid = false;
	}
};

class PxsFluidSpatialHash
{
public:

	PxsFluidSpatialHash(PxU32 numHashBuckets, PxF32 cellSizeInv, PxU32 packetMultLog, bool supportSections);
	~PxsFluidSpatialHash();

	static PX_FORCE_INLINE PxsParticleCell* findCell(PxU32 &cellIndex, const PxcGridCellVector& coord, PxsParticleCell* cells, PxU32 numHashBuckets);
	static PX_FORCE_INLINE const PxsParticleCell* findConstCell(PxU32 &cellIndex, const PxcGridCellVector& coord, const PxsParticleCell* cells, PxU32 numHashBuckets);

	PX_FORCE_INLINE PxF32 getCellSizeInv() { return mCellSizeInv; }
	PX_FORCE_INLINE PxU32 getPacketMultLog() { return mPacketMultLog; }

	PX_FORCE_INLINE PxU32 getNumPackets() const { return mNumCells; }
	PX_FORCE_INLINE const PxsParticleCell* getPackets() { return mCells; }
	PX_FORCE_INLINE const PxsFluidPacketSections* getPacketSections() { return mPacketSections; }

	PX_FORCE_INLINE const PxsParticleCell* findCell(PxU32 &cellIndex, const PxcGridCellVector& coord);
	PX_FORCE_INLINE PxsParticleCell* getCell(PxU32& cellIndex, const PxcGridCellVector& coord);

	/*!
	Given the coordinates of a specific packet, the packet table, the packet sections and the packet table
	size, this function builds the halo region structure for the packet. The halo region specifies the relevant
	particles of neighboring packets.
	*/
	static void getHaloRegions(	PxsFluidPacketHaloRegions& packetHalo, const PxcGridCellVector& packetCoords,
								const PxsParticleCell* packets, const PxsFluidPacketSections* packetSections,
								PxU32 numHashBuckets);

	/*!
	Build local hash table for cells within a packet. Reorders a particle index array according to particle cells.

	The cell entry array must have more entries than the number of particles passed. The particle index
	table must have the size of the number of particles passed. The particle array is not declared const 
	because hash keys might get stored temporarily in the particles.
	*/
	static void buildLocalHash(	const PxsFluidParticle* particles, PxU32 numParticles, PxsParticleCell* cells,
								PxU32* particleIndices, PxU16* hashKeyArray, PxU32 numHashBuckets, PxF32 cellSizeInv, const PxVec3& packetCorner);

	/*!
	Builds the packet hash and reorders particle indices to packets. Particles are not declared const since
	each particle hash key  and cell gets precomputed.
	*/
	void updatePacketHash(PxU32& numSorted, PxU32* sortedIndices, PxsFluidParticle* particles, const Cm::BitMap& particleMap, const PxU32 validParticleRange, PxBaseTask* continuation);

	/*!
	Divides each fluid packet into sections and reorders particle indices according to sections.
	Input particles are not declared const since for each particle the section index gets precomputed.
	*/
	void updatePacketSections(PxU32* particleIndices, PxsFluidParticle* particles, PxBaseTask* continuation);

private:
	static void reorderParticleIndicesToCells(const PxsFluidParticle* particles, PxU32 numParticles, PxsParticleCell* cells,
		PxU32* particleIndices, PxU32 numHashBuckets, PxU16* hashKeyArray);

	void reorderParticleIndicesToPackets(PxU32* sortedIndices, PxU32 numHashedParticles, const Cm::BitMap& particleMap, PxU16* hashKeyArray);

	/*!
	Splits the specified packet into 26 boundary sections (plus one inner section) and reorders the particles
	according to sections.
	*/
	void buildPacketSections(const PxsParticleCell& packet, PxsFluidPacketSections& sections, PxU32 packetMultLog, PxsFluidParticle* particles, 
							 PxU32* particleIndices);

	void reorderParticlesToPacketSections(const PxsParticleCell& packet, PxsFluidPacketSections& sections, const PxsFluidParticle* particles, 
										  const PxU32* inParticleIndices, PxU32* outParticleIndices, PxU16* sectionIndexBuf);


private:
	PxsParticleCell*		mCells;
	PxU32					mNumCells;
	PxU32                   mNumHashBuckets;
	PxF32					mCellSizeInv;

	//Packet Hash data
	PxU32					mPacketMultLog;
	PxsFluidPacketSections*	mPacketSections;

#if defined(PX_PS3) && !defined(__SPU__)
public:
	CellFluidHashTask		mHashSPU;
	CellFluidReorderPacketSectionsTask mReorderPacketSectionsSPUs;
	PxU32					mNumHashSPU;
	PxU32					mNumReorderSPU;
#endif
};

PX_FORCE_INLINE const PxsParticleCell* PxsFluidSpatialHash::findConstCell(PxU32 &cellIndex, const PxcGridCellVector& coord, const PxsParticleCell* cells,
																	 PxU32 numHashBuckets)
{
	cellIndex = getCellIndex(coord, cells, numHashBuckets);
	const PxsParticleCell* cell = &cells[cellIndex];

	if(cell->numParticles == PX_INVALID_U32)
		return NULL;
	else
		return cell;
}

PX_FORCE_INLINE PxsParticleCell* PxsFluidSpatialHash::findCell(PxU32 &cellIndex, const PxcGridCellVector& coord, PxsParticleCell* cells,
														  PxU32 numHashBuckets)
{
	const PxsParticleCell* constCell = findConstCell(cellIndex, coord, cells, numHashBuckets);
	return const_cast<PxsParticleCell*>(constCell);
}

PX_FORCE_INLINE const PxsParticleCell* PxsFluidSpatialHash::findCell(PxU32 &cellIndex, const PxcGridCellVector& coord)
{
	return findCell(cellIndex, coord, mCells, mNumHashBuckets);
}

PX_FORCE_INLINE PxsParticleCell* PxsFluidSpatialHash::getCell(PxU32& cellIndex, const PxcGridCellVector& coord)
{
	cellIndex = getCellIndex(coord, mCells, mNumHashBuckets);
	return &mCells[cellIndex];
}

}

#endif // PXS_FLUID_SPATIAL_HASH_H
