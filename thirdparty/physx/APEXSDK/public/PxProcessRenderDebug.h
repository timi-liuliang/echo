/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PX_PROCESS_RENDER_DEBUG_H
#define PX_PROCESS_RENDER_DEBUG_H

#include "PxRenderDebug.h"

namespace physx
{
namespace general_renderdebug4
{

#define MAX_LINE_VERTEX 2048
#define MAX_SOLID_VERTEX 2048

PX_PUSH_PACK_DEFAULT


class PxProcessRenderDebug
{
public:
	enum DisplayType
	{
		SCREEN_SPACE,
		WORLD_SPACE,
		WORLD_SPACE_NOZ,
		DT_LAST
	};

	virtual void processRenderDebug(const DebugPrimitive **dplist,
									PxU32 pcount,
									RenderDebugInterface *iface,
									DisplayType type) = 0;

	virtual void flush(RenderDebugInterface *iface,DisplayType type) = 0;

	virtual void flushFrame(RenderDebugInterface *iface) = 0;

	virtual void release(void) = 0;

	virtual void setViewMatrix(const physx::PxMat44 &view) 
	{
		PX_UNUSED(view);
	}

protected:
	virtual ~PxProcessRenderDebug(void) { };

};


PxProcessRenderDebug * createProcessRenderDebug(void);


PX_POP_PACK

}; // end of namespace
using namespace general_renderdebug4;
}; // end of namespace

#endif
