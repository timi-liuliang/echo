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
#ifndef PXS_PARTILCECELL_H
#define PXS_PARTILCECELL_H

#include "PxcGridCellVector.h"

namespace physx
{

// Structure describing a particle cell hash entry.
struct PxsParticleCell
{
	PxcGridCellVector coords;	//! The packet coordinates
	PxU32 numParticles;			//! Number of particles in the packet
	PxU32 firstParticle;		//! Start index of the associated particle interval 
};

}

#endif
