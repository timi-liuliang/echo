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
#ifndef PXS_FLUID_SPATIAL_HASH_HELPER_H
#define PXS_FLUID_SPATIAL_HASH_HELPER_H


#include "PxcGridCellVector.h"
#include "PxsParticleCell.h"

namespace physx
{

PX_FORCE_INLINE PxU32 hashFunction(const PxcGridCellVector& coord, PxU32 numHashBuckets)
{
	PX_ASSERT((((numHashBuckets-1)^numHashBuckets)+1) == (2*numHashBuckets));
	
	return ( (static_cast<PxU32>(coord.x) + 101*static_cast<PxU32>(coord.y) + 7919*static_cast<PxU32>(coord.z)) & (numHashBuckets - 1) );
	//sschirm: weird! The version that spreads all the coordinates is slower! Is the reason the additional multiplication?
	//return ( (101*static_cast<PxU32>(coord.x) + 7919*static_cast<PxU32>(coord.y) + 73856093*static_cast<PxU32>(coord.z)) & (numHashBuckets - 1) );
}

PX_FORCE_INLINE PxU32 getCellIndex(const PxcGridCellVector& coord, const PxsParticleCell* cells, PxU32 numHashBuckets)
{
#ifdef PX_DEBUG
	PxU32 tries = 0;
#endif

	PxU32 key = hashFunction(coord, numHashBuckets);
	const PxsParticleCell *cell = &cells[key];

	while ( (cell->numParticles != PX_INVALID_U32) && (coord != cell->coords) )
	{
		key = (key+1) & (numHashBuckets-1);
		cell = &cells[key];

#ifdef PX_DEBUG
		tries++;
#endif
		PX_ASSERT(tries < numHashBuckets);
	}

	return key;
}


/*
Compute packet section index for given cell coordinate. The packet sections are indexed as follows.

Left packet boundary      Front packet boundary      Top packet boundary
__________________        __________________         __________________
| 3 |   5    | 4 |        | 4 |   22   |13 |         | 3 |   21   |12 |
|___|________|___|        |___|________|___|         |___|________|___|
|   |        |   |        |   |        |   |         |   |        |   |
| 6 |   8    | 7 |        | 7 |   25   |16 |         | 5 |   23   |14 |
|   |        |   |        |   |        |   |         |   |        |   |
|___|________|___|        |___|________|___|         |___|________|___|
| 0 |   2    | 1 |        | 1 |   19   |10 |         | 4 |   22   |13 |
|___|________|___|        |___|________|___|         |___|________|___|

Right packet boundary     Rear packet boundary       Bottom packet boundary
__________________        __________________         __________________
|13 |   14   |12 |        |12 |   21   | 3 |         | 1 |   19   |10 |
|___|________|___|        |___|________|___|         |___|________|___|
|   |        |   |        |   |        |   |         |   |        |   |
|16 |   17   |15 |        |15 |   24   | 6 |         | 2 |   20   |11 |
|   |        |   |        |   |        |   |         |   |        |   |
|___|________|___|        |___|________|___|         |___|________|___|
|10 |   11   | 9 |        |9  |   18   | 0 |         | 0 |   18   | 9 |
|___|________|___|        |___|________|___|         |___|________|___|

Note: One section is missing in this illustration. Section 26 is in the middle of the packet and
      enclosed by the other sections. For particles in section 26 we know for sure that no interaction
	  with particles of neighboring packets occur.
*/
PX_FORCE_INLINE PxU32 getPacketSectionIndex(const PxcGridCellVector& cellCoords, const PxcGridCellVector& packetMinCellCoords,
							PxU32 packetMult)
{
	PxU32 sectionIndex = 0;

	// Translate cell coordinates such that the minimal cell coordinate of the packet is at the origin (0,0,0)
	PxcGridCellVector coord(cellCoords);
	coord -= packetMinCellCoords;

	// Find section the particle cell belongs to.
	
	if ((PxU32)(coord.x + 1) == packetMult)
	{
		// Right side boundary of packet
		sectionIndex = 9;
	}
	else if (coord.x != 0)
	{
		sectionIndex = 18;
	}
	// else: Left side boundary of packet

	//-----------

	if ((PxU32)(coord.y + 1) == packetMult)
	{
		// Top boundary of packet
		sectionIndex += 3;
	}
	else if (coord.y != 0)
	{
		sectionIndex += 6;
	}
	// else: Bottom boundary of packet

	//-----------

	if ((PxU32)(coord.z + 1) == packetMult)
	{
		// Front boundary of packet
		sectionIndex += 1;
	}
	else if (coord.z != 0)
	{
		sectionIndex += 2;
	}
	// else: Rear boundary of packet

	return sectionIndex;
}

}

#endif
