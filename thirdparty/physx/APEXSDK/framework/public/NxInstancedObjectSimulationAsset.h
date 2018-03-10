/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_INSTANCED_OBJECT_SIMULATION_ASSET_H
#define NX_INSTANCED_OBJECT_SIMULATION_ASSET_H

/*!
\file
\brief class NxIosAsset
*/

#include "NxApexAsset.h"

namespace physx
{
namespace apex
{

class NxApexScene;
class NxApexActor;

class NxIofxAsset;

PX_PUSH_PACK_DEFAULT

/**
\brief The base class of all Instanced Object Simulation classes
*/
class NxIosAsset : public NxApexAsset
{
public:
	//! \brief create a generic IOS NxApexActor in a specific NxApexSene
	virtual NxApexActor*		        createIosActor(NxApexScene& scene, NxIofxAsset* iofxAsset) = 0;
	//! \brief release a generic IOS NxApexActor
	virtual void						releaseIosActor(NxApexActor& actor) = 0;
	//! \brief get supports density
	virtual bool						getSupportsDensity() const = 0;
};

PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_INSTANCED_OBJECT_SIMULATION_ASSET_H
