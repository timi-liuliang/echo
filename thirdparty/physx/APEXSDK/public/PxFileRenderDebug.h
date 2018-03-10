/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PX_FILE_RENDER_DEBUG_H
#define PX_FILE_RENDER_DEBUG_H

#include "PxProcessRenderDebug.h"

namespace physx
{
namespace general_renderdebug4
{

class PxFileRenderDebug : public PxProcessRenderDebug
{
public:
	virtual PxProcessRenderDebug * getEchoLocal(void) const = 0;
	virtual PxU32 getFrameCount(void) const = 0;
	virtual void setFrame(PxU32 frameNo) = 0;
protected:
	virtual ~PxFileRenderDebug(void) { };
};


PxFileRenderDebug * createFileRenderDebug(const char *fileName,
										  bool readAccess,
										  bool echoLocally);

};
using namespace general_renderdebug4;
};

#endif
