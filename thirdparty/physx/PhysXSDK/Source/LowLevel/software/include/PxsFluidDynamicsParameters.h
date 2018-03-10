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
#ifndef PXS_FLUID_DYNAMICS_PARAMETER_H
#define PXS_FLUID_DYNAMICS_PARAMETER_H

namespace physx
{

enum PxsSphUpdateType
{
	PXS_SPH_DENSITY,
	PXS_SPH_FORCE,
	PXS_SPH_EXIT
};

struct PxsFluidDynamicsParameters
{
	PxReal		selfDensity;
	PxReal		particleMassStd;
	PxReal		cellSize;
	PxReal		cellSizeInv;
	
	PxReal		cellSizeSq;
	PxReal		packetSize;
	PxReal		radiusStd;
	PxReal		radiusSqStd;
	
	PxReal		densityMultiplierStd;
	PxReal		stiffMulPressureMultiplierStd;
	PxReal		viscosityMultiplierStd;
	PxReal		initialDensity;	
	
	PxReal		scaleToStd;
	PxReal		scaleSqToStd;
	PxReal		scaleToWorld;
	PxReal		densityNormalizationFactor;
	
	PxU32		packetMultLog;	
	PxU32		pad[3];
};

PX_COMPILE_TIME_ASSERT(sizeof(PxsFluidDynamicsParameters) % 16 == 0);

}

#endif
