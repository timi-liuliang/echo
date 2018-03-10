/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULES_H_
#define MODULES_H_

#include <NxApexDefs.h>

#include <NxModuleLegacy.h>
#include <NxModuleClothing.h>
#include <NxModuleDestructible.h>

#if APEX_USE_PARTICLES

#include <NxModuleBasicFS.h>
#include <NxModuleBasicIOS.h>
#include <NxModuleEmitter.h>
#include <NxModuleExplosion.h>
#include <NxModuleFieldBoundary.h>
#include <NxModuleFieldSampler.h>
#include <NxModuleFluidIOS.h>
#include <NxModuleForceField.h>
#include <NxModuleIOFX.h>
#include <NxModuleParticleIOS.h>
#include <NxModuleParticles.h>
#include <NxModuleTurbulenceFS.h>

#endif

#endif
