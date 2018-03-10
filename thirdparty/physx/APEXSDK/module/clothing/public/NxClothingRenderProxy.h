/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_CLOTHING_RENDER_PROXY_H
#define NX_CLOTHING_RENDER_PROXY_H

#include "NxApexRenderable.h"
#include "NxApexInterface.h"

namespace NxParameterized
{
class Interface;
}

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief Instance of NxClothingRenderProxy. This is the renderable of a clothing actor.
The data in this object is consistent until it is returned to APEX with the release()
call.
*/
class NxClothingRenderProxy : public NxApexInterface, public NxApexRenderable
{
protected:
	virtual ~NxClothingRenderProxy() {}

public:
	/**
	\brief True if the render proxy contains simulated data, false if it is purely animated.
	*/
	virtual bool hasSimulatedData() const = 0;
};

PX_POP_PACK

}
} // namespace physx::apex

#endif // NX_CLOTHING_RENDER_PROXY_H
