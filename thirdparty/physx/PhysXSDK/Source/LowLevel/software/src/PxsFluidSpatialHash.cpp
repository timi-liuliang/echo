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


#include "PxsFluidSpatialHash.h"
#include "PxsFluidSpatialHashHelper.h"
#include "PxsFluidParticle.h"
#include "PxsFluidCollisionData.h"
#include "PxvParticleSystemSim.h"
#include "PsAlloca.h"

#include "CmTask.h"

#ifdef PX_PS3
#include "CellFluidHashTask.h"
#include "CellTimerMarker.h"
#endif

using namespace physx;

PxsFluidSpatialHash::PxsFluidSpatialHash(PxU32 numHashBuckets, PxF32 cellSizeInv, PxU32 packetMultLog, bool supportSections) : 
	mNumCells(0), 
	mNumHashBuckets(numHashBuckets),
	mCellSizeInv(cellSizeInv),
	mPacketMultLog(packetMultLog),
	mPacketSections(NULL)
{
	//(numHashBuckets + 1): including overflow cell
	mCells = reinterpret_cast<PxsParticleCell*>(PX_ALLOC((numHashBuckets + 1)* sizeof(PxsParticleCell), PX_DEBUG_EXP("PxsParticleCell")));

	if (supportSections)
		mPacketSections = reinterpret_cast<PxsFluidPacketSections*>(PX_ALLOC(numHashBuckets * sizeof(PxsFluidPacketSections), PX_DEBUG_EXP("PxsFluidPacketSections")));
}

PxsFluidSpatialHash::~PxsFluidSpatialHash()
{
	PX_FREE(mCells);

	if (mPacketSections)
		PX_FREE(mPacketSections);
	}

/*-------------------------------------------------------------------------*/

/*!
Builds the packet hash and reorders particles.
*/
void PxsFluidSpatialHash::updatePacketHash(PxU32& numSorted, PxU32* sortedIndices, PxsFluidParticle* particles, const Cm::BitMap& particleMap, const PxU32 validParticleRange, PxBaseTask* continuation)
{
	PX_ASSERT(validParticleRange > 0);
	PX_UNUSED(validParticleRange);
	
	// Mark packet hash entries as empty.
	for (PxU32 p=0; p<PXS_PARTICLE_SYSTEM_PACKET_HASH_SIZE; p++)
	{
		PxsParticleCell& packet = mCells[p];
		packet.numParticles = PX_INVALID_U32;
	}

	// Initialize overflop packet
	mCells[PXS_PARTICLE_SYSTEM_OVERFLOW_INDEX].numParticles = 0;

	PxU32 packetMult = PxU32(1 << mPacketMultLog);
	const PxF32 packetSizeInv = mCellSizeInv / packetMult;
	
	const PxU32 validWordCount = particleMap.size() >> 5; //((validParticleRange + 0x1F) & ~0x1F) >> 5;
#ifdef PX_PS3
	startTimerMarker(ePARTICLEUPDATEPACKETHASH);

	if( (mNumHashSPU > 0) && (validWordCount*32 > mNumHashSPU * PXS_FLUID_PACKET_HASH_PARTICLE_MIN_SPU) && (validWordCount*32 < mNumHashSPU * PXS_FLUID_PACKET_HASH_PARTICLE_MAX_SPU))
	{
		mHashSPU.setSpuCount(mNumHashSPU);
		mHashSPU.init(numSorted, sortedIndices, particles, particleMap, validWordCount, packetSizeInv, mNumHashBuckets, mCells);		
		mHashSPU.setContinuation(continuation);
		mHashSPU.removeReference();
	}
	else
#endif
	{		
		PxU32 numPackets = 0;
		numSorted = 0;

		// Add particles to packet hash
		PxU16* hashKeyArray = reinterpret_cast<PxU16*>(PX_ALLOC(validWordCount*32 * sizeof(PxU16), PX_DEBUG_EXP("hashKeys")));  //save the hashkey for reorder
		Cm::BitMap::Iterator particleIt(particleMap);
		PX_ASSERT(hashKeyArray);

		for (PxU32 particleIndex = particleIt.getNext(); particleIndex != Cm::BitMap::Iterator::DONE; particleIndex = particleIt.getNext())
		{
			PxsFluidParticle& particle = particles[particleIndex];

			if (particle.flags.api & PxParticleFlag::eSPATIAL_DATA_STRUCTURE_OVERFLOW) // particles which caused overflow in the past are rejected.
			{
				mCells[PXS_PARTICLE_SYSTEM_OVERFLOW_INDEX].numParticles++;
				hashKeyArray[particleIndex] = PXS_PARTICLE_SYSTEM_OVERFLOW_INDEX;
				continue;
			}

			// Compute cell coordinate for particle
			// Transform cell to packet coordinate		
			PxcGridCellVector packetCoords(particle.position, packetSizeInv);

			PxU32 hashKey;
			PxsParticleCell* packet = getCell(hashKey, packetCoords);
			PX_ASSERT(packet);
			PX_ASSERT(hashKey < PXS_PARTICLE_SYSTEM_PACKET_HASH_SIZE);
			hashKeyArray[particleIndex] = Ps::to16(hashKey);

			if(packet->numParticles == PX_INVALID_U32)
			{
				// Entry is empty -> Initialize new entry

				if(numPackets >= PXS_PARTICLE_SYSTEM_PACKET_LIMIT)
				{
					// Reached maximum number of packets -> Mark particle for deletion
					PX_WARN_ONCE(true, "Particles: Spatial data structure overflow! Particles might miss collisions with the scene. See particle section of the guide for more information.");
					particle.flags.api |= PxParticleFlag::eSPATIAL_DATA_STRUCTURE_OVERFLOW;
					particle.flags.low &= (PxU16)~PxvInternalParticleFlag::eANY_CONSTRAINT_VALID;
					mCells[PXS_PARTICLE_SYSTEM_OVERFLOW_INDEX].numParticles++;
					hashKeyArray[particleIndex] = PXS_PARTICLE_SYSTEM_OVERFLOW_INDEX;
					continue;
				}

				packet->coords = packetCoords;
				packet->numParticles = 0;
				numPackets++;
			}

			PX_ASSERT(packet->numParticles != PX_INVALID_U32);
			packet->numParticles++;
			numSorted++;
		}

		mNumCells = numPackets;

		// Set for each packet the starting index of the associated particle interval and clear the
		// particle counter (preparation for reorder step).
		// include overflow packet.
		PxU32 numParticles = 0;
		for (PxU32 p=0; p<PXS_PARTICLE_SYSTEM_PACKET_HASH_BUFFER_SIZE; p++)
		{
			PxsParticleCell& packet = mCells[p];

			if(packet.numParticles == PX_INVALID_U32)
				continue;

			packet.firstParticle = numParticles;			
			numParticles += packet.numParticles;		
			packet.numParticles = 0;			
		}

		reorderParticleIndicesToPackets(sortedIndices, numParticles, particleMap,  hashKeyArray);

		PX_FREE(hashKeyArray);
	}

	continuation->removeReference();

#ifdef PX_PS3
	stopTimerMarker(ePARTICLEUPDATEPACKETHASH);
#endif
}

/*!
Reorders particle indices to packets.
*/
void PxsFluidSpatialHash::reorderParticleIndicesToPackets(PxU32* sortedIndices, PxU32 numParticles, const Cm::BitMap& particleMap, PxU16* hashKeyArray)
{
	Cm::BitMap::Iterator particleIt(particleMap);
	for (PxU32 particleIndex = particleIt.getNext(); particleIndex != Cm::BitMap::Iterator::DONE; particleIndex = particleIt.getNext())
	{
		// Get packet for fluid
		PxsParticleCell* packet = &mCells[hashKeyArray[particleIndex]];
		PX_ASSERT(packet);
		PX_ASSERT(packet->numParticles != PX_INVALID_U32);

		PxU32 index = packet->firstParticle + packet->numParticles;
		PX_ASSERT(index < numParticles);
		PX_UNUSED(numParticles);
		sortedIndices[index] = particleIndex;
		packet->numParticles++;
	}
}

void PxsFluidSpatialHash::updatePacketSections(PxU32* particleIndices, PxsFluidParticle* particles, PxBaseTask* continuation)
{
	PX_ASSERT(mPacketSections);
	PX_UNUSED(continuation);

	// MS: For this task we could use multithreading, gather a couple of packets and run them in parallel.
	//     Multiprocessor systems might take advantage of this but for the PC we will postpone this for now.
    PxU32 skipSize = 0;
#ifdef PX_PS3	
	startTimerMarker(eFLUIDUPDATEPACKETSECTIONS);
	if(mNumReorderSPU > 0)  
	{
		mReorderPacketSectionsSPUs.setSpuCount(mNumReorderSPU);
		mReorderPacketSectionsSPUs.init(mCells, mPacketSections, mPacketMultLog, mCellSizeInv, particles, particleIndices);
		mReorderPacketSectionsSPUs.setContinuation(continuation);
		mReorderPacketSectionsSPUs.removeReference();

		skipSize = SPU_PARTICLE_LIMIT_REORDER_SECTIONS;
	}
#endif

	for (PxU32 p=0; p < PXS_PARTICLE_SYSTEM_PACKET_HASH_SIZE; p++)
	{
		PxsParticleCell& packet = mCells[p];

		if((packet.numParticles == PX_INVALID_U32) || (packet.numParticles <= skipSize))
			continue;

		buildPacketSections(packet,	mPacketSections[p],	mPacketMultLog, particles, particleIndices);
	}
 
#ifdef PX_PS3	
	stopTimerMarker(eFLUIDUPDATEPACKETSECTIONS);
#endif

}

void PxsFluidSpatialHash::buildPacketSections(const PxsParticleCell& packet, PxsFluidPacketSections& sections, PxU32 packetMultLog,
											  PxsFluidParticle* particles, PxU32* particleIndices)
{
	PX_ASSERT(packetMultLog > 0);

	PxU32 packetMult = PxU32(1 << packetMultLog);

	// Compute the smallest cell coordinate within the packet
	PxcGridCellVector packetMinCellCoords = packet.coords << packetMultLog;

	// Clear packet section entries
	PxMemSet(&sections, 0, sizeof(PxsFluidPacketSections));

	// Divide the packet into subpackets that fit into local memory of processing unit.
	PxU32 particlesRemainder = packet.numParticles % PXS_FLUID_SUBPACKET_PARTICLE_LIMIT_PACKET_SECTIONS;
	if (particlesRemainder == 0)
		particlesRemainder = PXS_FLUID_SUBPACKET_PARTICLE_LIMIT_PACKET_SECTIONS;


	PxU32* packetParticleIndices = particleIndices + packet.firstParticle;

	PX_ALLOCA(sectionIndexBuf, PxU16, packet.numParticles * sizeof(PxU16));
	PX_ASSERT(sectionIndexBuf);

	PxU32 startIdx = 0;
	PxU32 endIdx = particlesRemainder;	// We start with the smallest subpacket, i.e., the subpacket which does not reach its particle limit.
	PxcGridCellVector cellCoord;
	PxU16* pSectionIndexBuf = sectionIndexBuf;
	while (endIdx <= packet.numParticles)
	{
		// Loop over particles of the subpacket.
		for(PxU32 p=startIdx; p < endIdx; p++)
		{
			PxU32 particleIndex = packetParticleIndices[p];
			PxsFluidParticle& particle = particles[particleIndex];
			// Find packet section the particle belongs to.
			cellCoord.set(particle.position, mCellSizeInv);
			PxU32 sectionIndex = getPacketSectionIndex(cellCoord, packetMinCellCoords, packetMult);
			PX_ASSERT(sectionIndex < PXS_FLUID_PACKET_SECTIONS);

			*pSectionIndexBuf++ = Ps::to16(sectionIndex);

			// Increment particle count of the section the particle belongs to.
			sections.numParticles[sectionIndex]++;
		}

		startIdx = endIdx;
		endIdx += PXS_FLUID_SUBPACKET_PARTICLE_LIMIT_PACKET_SECTIONS;
	}

	// Set for each packet section the starting index of the associated particle interval.
	PxU32 particleIndex = packet.firstParticle;
	for (PxU32 s=0; s < PXS_FLUID_PACKET_SECTIONS; s++)
	{
		sections.firstParticle[s] = particleIndex;
		particleIndex += sections.numParticles[s];
	}

	// Simon: This is not yet chunked. Need to when porting.	
	PX_ALLOCA(tmpIndexBuffer, PxU32, packet.numParticles * sizeof(PxU32));
	PX_ASSERT(tmpIndexBuffer);
	PxMemCopy(tmpIndexBuffer, packetParticleIndices, packet.numParticles*sizeof(PxU32));

	reorderParticlesToPacketSections(packet, sections, particles, tmpIndexBuffer, packetParticleIndices, sectionIndexBuf);
}

void PxsFluidSpatialHash::reorderParticlesToPacketSections(const PxsParticleCell& packet, PxsFluidPacketSections& sections,
														   const PxsFluidParticle* particles, const PxU32* inParticleIndices, PxU32* outParticleIndices, PxU16* sectionIndexBuf)
{
	// Divide the packet into subpackets that fit into local memory of processing unit.
	PxU32 particlesRemainder = packet.numParticles % PXS_FLUID_SUBPACKET_PARTICLE_LIMIT_PACKET_SECTIONS;
	if (particlesRemainder == 0)
		particlesRemainder = PXS_FLUID_SUBPACKET_PARTICLE_LIMIT_PACKET_SECTIONS;

	// Prepare section structure for reorder
	PxMemSet(sections.numParticles, 0, (PXS_FLUID_PACKET_SECTIONS * sizeof(PxU32)));

	PxU32 startIdx = 0;
	PxU32 endIdx = particlesRemainder;	// We start with the smallest subpacket, i.e., the subpacket which does not reach its particle limit.
	while (endIdx <= packet.numParticles)
	{
		// Loop over particles of the subpacket.
		for(PxU32 p=startIdx; p < endIdx; p++)
		{
			PxU32 particleIndex = inParticleIndices[p];
			const PxsFluidParticle& particle = particles[particleIndex];
			PX_UNUSED(particle);

			// Reorder particle according to packet section.
			//
			// It is important that particles inside the core section (the section that will not interact with neighbor packets)
			// are moved to the end of the buffer. This way we can easily ignore these particles when testing against
			// particles of neighboring packets.

			PxU32 sectionIndex = *sectionIndexBuf++;
			PxU32 outIndex = sections.firstParticle[sectionIndex] + sections.numParticles[sectionIndex];

			//the output index array start at the packet start, unlike the section indices, which are absolute.
			PxU32 relativeOutIndex = outIndex - packet.firstParticle;
			PX_ASSERT(relativeOutIndex < packet.numParticles);
			outParticleIndices[relativeOutIndex] = particleIndex;

			sections.numParticles[sectionIndex]++;
		}

		startIdx = endIdx;
		endIdx += PXS_FLUID_SUBPACKET_PARTICLE_LIMIT_PACKET_SECTIONS;
	}
}

/*
To optimize particle interaction between particles of neighboring packets, each packet is split
into 27 sections. Of these 27 sections, 26 are located at the surface of the packet, i.e., contain
the outermost particle cells, and one section contains all the inner cells. If we want to compute
the particle interactions between neighboring packets, we only want to work with the 26 "surface
sections" of each packet, neglecting the inner sections. Thus, we need to find for a given packet
all the relevant sections of the neighboring packets. These sections will be called halo regions.
The following illustration specifies how these halo regions are indexed (there are 98 halo regions
for a packet). The illustration shows the halo regions of a packet from a viewer perspective that
looks from the outside at the different sides of a packet.

    Left halo regions                 Front halo regions                 Top halo regions
__________________________        __________________________        __________________________
|92 |60 |   62   | 61| 93|        |93 |87 |   89   | 88| 97|        |92 |81 |   83   | 82| 96|
|___|___|________|___|___|        |___|___|________|___|___|        |___|___|________|___|___|
|67 | 3 |    5   |  4| 73|        |73 |46 |   52   | 49| 76|        |60 |27 |   33   | 30| 63|
|___|___|________|___|___|        |___|___|________|___|___|        |___|___|________|___|___|
|   |   |        |   |   |        |   |   |        |   |   |        |   |   |        |   |   |
|   |   |        |   |   |        |   |   |        |   |   |        |   |   |        |   |   |
|68 | 6 |    8   |  7| 74|        |74 |47 |   53   | 50| 77|        |62 |29 |   35   | 32| 65|
|   |   |        |   |   |        |   |   |        |   |   |        |   |   |        |   |   |
|___|___|________|___|___|        |___|___|________|___|___|        |___|___|________|___|___|
|66 | 0 |    2   |  1| 72|        |72 |45 |   51   | 48| 75|        |61 |28 |   34   | 31| 64|
|___|___|________|___|___|        |___|___|________|___|___|        |___|___|________|___|___|
|90 |54 |   56   | 55| 91|        |91 |84 |   86   | 85| 95|        |93 |87 |   89   | 88| 97|
|___|___|________|___|___|        |___|___|________|___|___|        |___|___|________|___|___|


   Right halo regions                  Rear halo regions                Bottom halo regions
__________________________        __________________________        __________________________
|97 |64 |   65   | 63| 96|        |96 |82 |   83   | 81| 92|        |91 |84 |   86   | 85| 95|
|___|___|________|___|___|        |___|___|________|___|___|        |___|___|________|___|___|
|76 |13 |   14   | 12| 70|        |70 |40 |   43   | 37| 67|        |55 |19 |   25   | 22| 58|
|___|___|________|___|___|        |___|___|________|___|___|        |___|___|________|___|___|
|   |   |        |   |   |        |   |   |        |   |   |        |   |   |        |   |   |
|   |   |        |   |   |        |   |   |        |   |   |        |   |   |        |   |   |
|77 |16 |   17   | 15| 71|        |71 |41 |   44   | 38| 68|        |56 |20 |   26   | 23| 59|
|   |   |        |   |   |        |   |   |        |   |   |        |   |   |        |   |   |
|___|___|________|___|___|        |___|___|________|___|___|        |___|___|________|___|___|
|75 |10 |   11   |  9| 69|        |69 |39 |   42   | 36| 66|        |54 |18 |   24   | 21| 57|
|___|___|________|___|___|        |___|___|________|___|___|        |___|___|________|___|___|
|95 |58 |   59   | 57| 94|        |94 |79 |   80   | 78| 90|        |90 |78 |   80   | 79| 94|
|___|___|________|___|___|        |___|___|________|___|___|        |___|___|________|___|___|

*/
void PxsFluidSpatialHash::getHaloRegions(	PxsFluidPacketHaloRegions& packetHalo,
											const PxcGridCellVector& packetCoords,
											const PxsParticleCell* packets,
											const PxsFluidPacketSections* packetSections,
											PxU32 numHashBuckets)
{
#define PXS_COPY_PARTICLE_INTERVAL(destIdx, srcIdx)						\
	packetHalo.firstParticle[destIdx] = sections.firstParticle[srcIdx];	\
	packetHalo.numParticles[destIdx] = sections.numParticles[srcIdx];

#define PXS_GET_HALO_REGIONS_FACE_NEIGHBOR(dx, dy, dz, startIdx, idx1, idx2, idx3, idx4, idx5, idx6, idx7, idx8, idx9)	\
	coords.set(packetCoords.x + dx, packetCoords.y + dy, packetCoords.z + dz);		\
	packet = findConstCell(packetIndex, coords, packets, numHashBuckets);			\
	if (packet)																		\
	{																				\
		const PxsFluidPacketSections& sections = packetSections[packetIndex];		\
																					\
		PXS_COPY_PARTICLE_INTERVAL(startIdx, idx1);									\
		PXS_COPY_PARTICLE_INTERVAL(startIdx + 1, idx2);								\
		PXS_COPY_PARTICLE_INTERVAL(startIdx + 2, idx3);								\
		PXS_COPY_PARTICLE_INTERVAL(startIdx + 3, idx4);								\
		PXS_COPY_PARTICLE_INTERVAL(startIdx + 4, idx5);								\
		PXS_COPY_PARTICLE_INTERVAL(startIdx + 5, idx6);								\
		PXS_COPY_PARTICLE_INTERVAL(startIdx + 6, idx7);								\
		PXS_COPY_PARTICLE_INTERVAL(startIdx + 7, idx8);								\
		PXS_COPY_PARTICLE_INTERVAL(startIdx + 8, idx9);								\
	}

#define PXS_GET_HALO_REGIONS_EDGE_NEIGHBOR(dx, dy, dz, startIdx, idx1, idx2, idx3)	\
	coords.set(packetCoords.x + dx, packetCoords.y + dy, packetCoords.z + dz);		\
	packet = findConstCell(packetIndex, coords, packets, numHashBuckets);			\
	if (packet)																		\
	{																				\
		const PxsFluidPacketSections& sections = packetSections[packetIndex];		\
																					\
		PXS_COPY_PARTICLE_INTERVAL(startIdx, idx1);									\
		PXS_COPY_PARTICLE_INTERVAL(startIdx + 1, idx2);								\
		PXS_COPY_PARTICLE_INTERVAL(startIdx + 2, idx3);								\
	}

#define PXS_GET_HALO_REGIONS_CORNER_NEIGHBOR(dx, dy, dz, startIdx, idx1)			\
	coords.set(packetCoords.x + dx, packetCoords.y + dy, packetCoords.z + dz);		\
	packet = findConstCell(packetIndex, coords, packets, numHashBuckets);			\
	if (packet)																		\
	{																				\
		const PxsFluidPacketSections& sections = packetSections[packetIndex];		\
																					\
		PXS_COPY_PARTICLE_INTERVAL(startIdx, idx1);									\
	}


	PX_ASSERT(packets);
	PX_ASSERT(packetSections);

	// Clear halo information
	PxMemSet(&packetHalo, 0, sizeof(PxsFluidPacketHaloRegions));

	const PxsParticleCell* packet;
	PxU32 packetIndex;
	PxcGridCellVector coords;

	//
	// Fill halo regions for the 6 neighbors which share a face with the packet.
	//

	// Left neighbor
	coords.set(packetCoords.x - 1, packetCoords.y, packetCoords.z);
	packet = findConstCell(packetIndex, coords, packets, numHashBuckets);
	if (packet)
	{
		const PxsFluidPacketSections& sections = packetSections[packetIndex];

		PxMemCopy(&(packetHalo.firstParticle[0]), &(sections.firstParticle[9]), (9 * sizeof(PxU32)));
		PxMemCopy(&(packetHalo.numParticles[0]), &(sections.numParticles[9]), (9 * sizeof(PxU32)));
	}

	// Right neighbor
	coords.set(packetCoords.x + 1, packetCoords.y, packetCoords.z);
	packet = findConstCell(packetIndex, coords, packets, numHashBuckets);
	if (packet)
	{
		const PxsFluidPacketSections& sections = packetSections[packetIndex];

		PxMemCopy(&(packetHalo.firstParticle[9]), &(sections.firstParticle[0]), (9 * sizeof(PxU32)));
		PxMemCopy(&(packetHalo.numParticles[9]), &(sections.numParticles[0]), (9 * sizeof(PxU32)));
	}

	// Bottom neighbor
	PXS_GET_HALO_REGIONS_FACE_NEIGHBOR(0, -1, 0,  18,  3, 4, 5, 12, 13, 14, 21, 22, 23)

	// Top neighbor
	PXS_GET_HALO_REGIONS_FACE_NEIGHBOR(0, 1, 0,  27,  0, 1, 2, 9, 10, 11, 18, 19, 20)

	// Rear neighbor
	PXS_GET_HALO_REGIONS_FACE_NEIGHBOR(0, 0, -1,  36,  1, 4, 7, 10, 13, 16, 19, 22, 25)

	// Front neighbor
	PXS_GET_HALO_REGIONS_FACE_NEIGHBOR(0, 0, 1,  45,  0, 3, 6, 9, 12, 15, 18, 21, 24)

	//
	// Fill halo regions for the 12 neighbors which share an edge with the packet.
	//
	
	PXS_GET_HALO_REGIONS_EDGE_NEIGHBOR(-1, -1, 0,  54,  12, 13, 14)
	PXS_GET_HALO_REGIONS_EDGE_NEIGHBOR( 1, -1, 0,  57,  3, 4, 5)
	PXS_GET_HALO_REGIONS_EDGE_NEIGHBOR(-1,  1, 0,  60,  9, 10, 11)
	PXS_GET_HALO_REGIONS_EDGE_NEIGHBOR( 1,  1, 0,  63,  0, 1, 2)

	PXS_GET_HALO_REGIONS_EDGE_NEIGHBOR(-1, 0, -1,  66,  10, 13, 16)
	PXS_GET_HALO_REGIONS_EDGE_NEIGHBOR( 1, 0, -1,  69,  1, 4, 7)
	PXS_GET_HALO_REGIONS_EDGE_NEIGHBOR(-1, 0,  1,  72,  9, 12, 15)
	PXS_GET_HALO_REGIONS_EDGE_NEIGHBOR( 1, 0,  1,  75,  0, 3, 6)

	PXS_GET_HALO_REGIONS_EDGE_NEIGHBOR(0, -1, -1,  78,  4, 13, 22)
	PXS_GET_HALO_REGIONS_EDGE_NEIGHBOR(0,  1, -1,  81,  1, 10, 19)
	PXS_GET_HALO_REGIONS_EDGE_NEIGHBOR(0, -1,  1,  84,  3, 12, 21)
	PXS_GET_HALO_REGIONS_EDGE_NEIGHBOR(0,  1,  1,  87,  0, 9, 18)

	//
	// Fill halo regions for the 8 neighbors which share a corner with the packet.
	//

	PXS_GET_HALO_REGIONS_CORNER_NEIGHBOR(-1, -1, -1, 90, 13)
	PXS_GET_HALO_REGIONS_CORNER_NEIGHBOR(-1, -1,  1, 91, 12)
	PXS_GET_HALO_REGIONS_CORNER_NEIGHBOR(-1,  1, -1, 92, 10)
	PXS_GET_HALO_REGIONS_CORNER_NEIGHBOR(-1,  1,  1, 93, 9)
	PXS_GET_HALO_REGIONS_CORNER_NEIGHBOR( 1, -1, -1, 94, 4)
	PXS_GET_HALO_REGIONS_CORNER_NEIGHBOR( 1, -1,  1, 95, 3)
	PXS_GET_HALO_REGIONS_CORNER_NEIGHBOR( 1,  1, -1, 96, 1)
	PXS_GET_HALO_REGIONS_CORNER_NEIGHBOR( 1,  1,  1, 97, 0)


	for(PxU32 i=0; i < PXS_FLUID_PACKET_HALO_REGIONS; i++)
		packetHalo.maxNumParticles = PxMax(packetHalo.maxNumParticles, packetHalo.numParticles[i]);
}


