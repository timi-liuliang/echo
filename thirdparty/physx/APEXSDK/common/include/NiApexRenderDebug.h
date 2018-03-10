/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NI_APEX_RENDER_DEBUG_H
#define NI_APEX_RENDER_DEBUG_H

#include "NxApexRenderDebug.h"
#include "PxProcessRenderDebug.h"

namespace physx
{
namespace apex
{

#ifndef WITHOUT_DEBUG_VISUALIZE

class NiApexRenderDebug : public NxApexRenderDebug
{
public:
	virtual physx::PxProcessRenderDebug* getProcessRenderDebug(void) = 0;
protected:
	virtual ~NiApexRenderDebug(void) { };
};

#else

class NiApexRenderDebug
{
	// no methods, no inheritance, debug rendering is disabled and the code should not be compiled in this configuration
};

#endif

} // namespace apex
} // namespace physx

#endif // NI_APEX_RENDER_DEBUG_H
