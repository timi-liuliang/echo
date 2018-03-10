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
#ifndef PXS_FLUID_PACKETSECTIONS_H
#define PXS_FLUID_PACKETSECTIONS_H

namespace physx
{

// Structure describing boundary section (plus inner section) of a fluid packet.
// This will be used for halo optimization, i.e., to reduce the number of particles
// that have to be tested in neighboring packets.
#define PXS_FLUID_PACKET_SECTIONS 27
struct PxsFluidPacketSections
{
	PxU32 numParticles[PXS_FLUID_PACKET_SECTIONS];		//! Number of particles in each packet section
	PxU32 firstParticle[PXS_FLUID_PACKET_SECTIONS];		//! Start index of the associated particle interval for each packet section
};

}

#endif
