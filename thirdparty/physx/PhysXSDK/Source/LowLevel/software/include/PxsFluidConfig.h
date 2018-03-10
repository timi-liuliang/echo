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


#ifndef PXS_FLUID_CONFIG_H
#define PXS_FLUID_CONFIG_H

#include "PxvParticleSystemFlags.h"

// Marker for fluid particles with no collision constraints
#define PXS_FLUID_NO_CONSTRAINT		PXD_PARTICLE_SYSTEM_PARTICLE_LIMIT

// Needs to be addressable with PxU16 PxsFluidParticle::hashKey
// - Ps::nextPowerOf2((PXD_PARTICLE_SYSTEM_HASH_KEY_LIMIT + 1)) must be addressable 
//   through PxU16 PxsFluidParticle::hashKey, see and PXS_FLUID_SUBPACKET_PARTICLE_LIMIT_FORCE_DENSITY
//   PxsFluidDynamics::updatePacket()
#define PXS_PARTICLE_SYSTEM_HASH_KEY_LIMIT			0x7ffe	

// Size of particle packet hash table.
// - Must be a power of 2
// - Must be at least as large as PXD_PARTICLE_SYSTEM_PACKET_LIMIT (see further below), but should be larger for the hash to be efficient.
// - Must to be addressable through PxU16 PxsFluidParticle::hashKey.
#define PXS_PARTICLE_SYSTEM_PACKET_HASH_SIZE		1024

// One larger than PXS_PARTICLE_SYSTEM_PACKET_HASH_SIZE to fit a special cell for overflow particles.
#define PXS_PARTICLE_SYSTEM_PACKET_HASH_BUFFER_SIZE	1025

// Index of special overflow packet
#define PXS_PARTICLE_SYSTEM_OVERFLOW_INDEX			1024

// Maximum number of particle packets (should be smaller than hash size since a full hash table is not efficient)
#define PXS_PARTICLE_SYSTEM_PACKET_LIMIT			924

// Slack for building the triangle packet hash. Has to be bigger than any epsilons used in collision detection.
// TODO: Move this to PxsFluid if the high level does not need this parameter anymore
#define PXS_PARTICLE_SYSTEM_COLLISION_SLACK			1.0e-3f

// Maximum number of fluid particles in a packet that can be handled at a time
#ifdef PX_X360
#define PXS_FLUID_SUBPACKET_PARTICLE_LIMIT	512
#define PXS_FLUID_BRUTE_FORCE_PARTICLE_THRESHOLD			100
#define PXS_FLUID_BRUTE_FORCE_PARTICLE_THRESHOLD_HALO_VS_SECTION   200
#elif defined PX_PS3
#ifdef PX_DEBUG
#define PXS_FLUID_SUBPACKET_PARTICLE_LIMIT	100
#define PXS_FLUID_BRUTE_FORCE_PARTICLE_THRESHOLD			64
#define PXS_FLUID_BRUTE_FORCE_PARTICLE_THRESHOLD_HALO_VS_SECTION 64
#else
#define PXS_FLUID_SUBPACKET_PARTICLE_LIMIT	192
#define PXS_FLUID_BRUTE_FORCE_PARTICLE_THRESHOLD			100
#define PXS_FLUID_BRUTE_FORCE_PARTICLE_THRESHOLD_HALO_VS_SECTION   192
#endif
#else
#define PXS_FLUID_SUBPACKET_PARTICLE_LIMIT	512
// If the number of particles in a packet and the number of particles for each neighboring halo region
// are below this threshold, then no local hash will be constructed and each particle of one packet will be
// tested against each particle of the other packet (for particle-particle interaction only).
//
// Note: Has to be smaller or equal to PXS_FLUID_SUBPACKET_PARTICLE_LIMIT_FORCE_DENSITY
#define PXS_FLUID_BRUTE_FORCE_PARTICLE_THRESHOLD			100
// If the number of particles in a packet section and the number of particles in a neighboring halo
// region are below this threshold, then no local hash will be constructed and each particle of the
// packet section will be tested against each particle of the halo region (for particle-particle interaction only).
//
// Note: Has to be smaller or equal to PXS_FLUID_SUBPACKET_PARTICLE_LIMIT_FORCE_DENSITY
#define PXS_FLUID_BRUTE_FORCE_PARTICLE_THRESHOLD_HALO_VS_SECTION   200
#endif

// Maximum number of fluid particles in a packet that can be handled at a time for dividing
// a packet into sections and reordering the particles accordingly
#define PXS_FLUID_SUBPACKET_PARTICLE_LIMIT_PACKET_SECTIONS	PXS_FLUID_SUBPACKET_PARTICLE_LIMIT

// Maximum number of fluid particles in a packet that can be handled at a time for SPH dynamics
// calculations, i.e., computation of density & force
// - Ps::nextPowerOf2((PXS_FLUID_SUBPACKET_PARTICLE_LIMIT_FORCE_DENSITY + 1)) must be addressable 
//   through PxU16 PxsFluidParticle::hashKey, see PxsFluidDynamics::updatePacket().
#define PXS_FLUID_SUBPACKET_PARTICLE_LIMIT_FORCE_DENSITY	PXS_FLUID_SUBPACKET_PARTICLE_LIMIT

//  loacl hash bucket size,  should equal nextPowerOfTwo(PXS_FLUID_SUBPACKET_PARTICLE_LIMIT_FORCE_DENSITY + 1)
#define PXS_FLUID_SUBPACKET_PARTICLE_HASH_BUCKET_SIZE       512

#ifdef PX_X360
#define PXS_FLUID_MAX_PARALLEL_TASKS_SPH 3
#elif defined PX_PS3
#define PXS_FLUID_MAX_PARALLEL_TASKS_SPH 1
#else
#define PXS_FLUID_MAX_PARALLEL_TASKS_SPH 8
#endif

// Maximum number of fluid particles in a packet that can be handled at a time for velocity
// integration
#define PXS_FLUID_SUBPACKET_PARTICLE_LIMIT_VEL_INTEGRATION	PXS_FLUID_SUBPACKET_PARTICLE_LIMIT

// Maximum number of fluid particles in a packet that can be handled at a time for
// detecting and resolving collisions. 
// - Must be smaller than PXS_FLUID_LOCAL_HASH_SIZE_MESH_COLLISION.
#ifdef PX_PS3
#define PXS_FLUID_SUBPACKET_PARTICLE_LIMIT_COLLISION	96
#else
#define PXS_FLUID_SUBPACKET_PARTICLE_LIMIT_COLLISION	128
#endif


// Maximum number of contact managers supported for collision on spus.
#define PXS_FLUID_MAX_CM_PER_FLUID_SHAPE_SPU				32

#define PXS_FLUID_PACKET_HASH_PARTICLE_BATCH_SPU			1024
#define PXS_FLUID_PACKET_HASH_PARTICLE_MIN_SPU				64
#define PXS_FLUID_PACKET_HASH_PARTICLE_MAX_SPU				8192

// Hash size for the local particle cell hash.
// - Must to be larger than PXS_FLUID_SUBPACKET_PARTICLE_LIMIT_COLLISION
// - Must be a power of 2
// - Must be addressable with PxU16 PxsFluidParticle::hashKey
#ifdef PX_PS3
#define PXS_FLUID_LOCAL_HASH_SIZE_MESH_COLLISION			128
#else
#define PXS_FLUID_LOCAL_HASH_SIZE_MESH_COLLISION			256
#endif

// Number of fluid packet shapes to run in parallel during collision update.
#ifdef PX_X360
#define PXS_FLUID_NUM_PACKETS_PARALLEL_COLLISION			3
#elif defined PX_PS3
#define PXS_FLUID_NUM_PACKETS_PARALLEL_COLLISION			7
#else
#define PXS_FLUID_NUM_PACKETS_PARALLEL_COLLISION			8
#endif

// Initial size of triangle mesh collision buffer (for storing indices of colliding triangles)
#define PXS_FLUID_INITIAL_MESH_COLLISION_BUFFER_SIZE		1024


#define PXS_FLUID_USE_SIMD_CONVEX_COLLISION 1


#endif // PXS_FLUID_CONFIG_H
