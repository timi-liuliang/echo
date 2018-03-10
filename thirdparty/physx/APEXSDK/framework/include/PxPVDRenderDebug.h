/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PX_PVD_RENDER_DEBUG_H

#define PX_PVD_RENDER_DEBUG_H

#include "PxProcessRenderDebug.h"

namespace physx
{

class PxPVDRenderDebug : public PxProcessRenderDebug
{
public:
	virtual PxProcessRenderDebug* getEchoLocal(void) const = 0;
protected:
	virtual ~PxPVDRenderDebug(void) { };
};


PxPVDRenderDebug* createPVDRenderDebug(bool echoLocally);

};

#endif
