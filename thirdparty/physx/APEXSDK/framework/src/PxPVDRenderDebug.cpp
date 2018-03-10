/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "PxPVDRenderDebug.h"
#include "PxProcessRenderDebug.h"
#include "PsUserAllocated.h"

#pragma warning(disable:4100)

namespace physx
{

class PVDRenderDebug : public PxPVDRenderDebug, public shdfnd::UserAllocated
{
public:
	PVDRenderDebug(bool echoLocally)
	{
		mEchoLocally = NULL;
		if (echoLocally)
		{
			mEchoLocally = createProcessRenderDebug();
		}
	}

	virtual ~PVDRenderDebug(void)
	{
		if (mEchoLocally)
		{
			mEchoLocally->release();
		}
	};

	virtual PxProcessRenderDebug* getEchoLocal(void) const
	{
		return mEchoLocally;
	}

	bool isOk(void) const
	{
		return true;
	}

	virtual void processRenderDebug(const DebugPrimitive** dplist,
	                                PxU32 pcount,
	                                RenderDebugInterface* iface,
	                                PxProcessRenderDebug::DisplayType type)
	{
		// At this location, PVD should stream these primitives over the socket connection.
		// All primitives are raw-data types; no pointers, etc.
		// If you need to munge the data to be endian aware, this is the location you would do it.
		// On the other side, PVD should grab this set of debug primitives and capture them relative to the current frame.
		if (mEchoLocally)
		{
			mEchoLocally->processRenderDebug(dplist, pcount, iface, type);
		}
	}


	virtual void flush(RenderDebugInterface* iface, PxProcessRenderDebug::DisplayType type)
	{
		// Indicates that primitives of this data type are completed.
		if (mEchoLocally)
		{
			mEchoLocally->flush(iface, type);
		}
	}

	// releases the PVD render debug interface.
	virtual void release(void)
	{
		delete this;
	}

	virtual void flushFrame(RenderDebugInterface* /*iface*/)
	{
	}

private:
	PxProcessRenderDebug*	mEchoLocally;
};


PxPVDRenderDebug* createPVDRenderDebug(bool echoLocally)
{
	PVDRenderDebug* f = PX_NEW(PVDRenderDebug)(echoLocally);
	if (!f->isOk())
	{
		delete f;
		f = NULL;
	}
	return static_cast< PxPVDRenderDebug*>(f);
}



};
