/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_MODULE_EMITTER_H
#define NX_MODULE_EMITTER_H

#include "NxApex.h"

namespace physx
{
namespace apex
{


PX_PUSH_PACK_DEFAULT

class NxApexEmitterAsset;
class NxApexEmitterAssetAuthoring;

class NxGroundEmitterAsset;
class NxGroundEmitterAssetAuthoring;

class NxImpactEmitterAsset;
class NxImpactEmitterAssetAuthoring;


/**
\brief An APEX Module that provides generic Emitter classes
*/
class NxModuleEmitter : public NxModule
{
protected:
	virtual ~NxModuleEmitter() {}

public:
};



PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_MODULE_EMITTER_H
