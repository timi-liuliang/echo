/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_DESTRUCTIBLE_RENDERABLE_H
#define NX_DESTRUCTIBLE_RENDERABLE_H

#include "foundation/Px.h"
#include "NxApexInterface.h"
#include "NxApexRenderable.h"
#include "NxModuleDestructible.h"

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT


/**
	Destructible renderable API.  The destructible renderable contains rendering information for an NxDestructibleActor.
*/
class NxDestructibleRenderable : public NxApexInterface, public NxApexRenderable
{
public:
	/**
		Get the render mesh actor for the specified mesh type.
	*/
	virtual NxRenderMeshActor*	getRenderMeshActor(NxDestructibleActorMeshType::Enum type = NxDestructibleActorMeshType::Skinned) const = 0;

protected:
	virtual	~NxDestructibleRenderable() {}
};

PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_DESTRUCTIBLE_RENDERABLE_H
