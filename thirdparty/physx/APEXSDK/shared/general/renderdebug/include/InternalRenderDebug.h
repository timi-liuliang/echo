/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef INTERNAL_RENDER_DEBUG_H
#define INTERNAL_RENDER_DEBUG_H

#include "PsShare.h"

namespace physx
{
	namespace general_renderdebug4
	{

class RenderDebug;
class PxProcessRenderDebug;

RenderDebug * createInternalRenderDebug(PxProcessRenderDebug *process);

}; 
using namespace general_renderdebug4;
};

#endif // INTERNAL_RENDER_DEBUG_H
