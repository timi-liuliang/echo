/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#ifndef __GRB_ENVENT_STREAM_H__
#define __GRB_ENVENT_STREAM_H__

#include "GrbEvents.h"

namespace physx
{

class GrbInteropEventStream3
{
public:
	GrbInteropEventStream3(NpScene * scene, shdfnd::Array<GrbInteropEvent3> * eventStream) : eventStream(eventStream), scene(scene) {}

	void clear()
	{
		//This is needed to ensure that the PhysX dll's heap is used to destruct the events
		scene->clearEventStream(eventStream);
	}

	PX_INLINE unsigned size() const
	{
		return eventStream->size();
	}

	PX_INLINE const GrbInteropEvent3 & operator[](unsigned n) const
	{
		return (*eventStream)[n];
	}

	PX_INLINE GrbInteropEvent3 & operator[](unsigned n)
	{
		return (*eventStream)[n];
	}

	shdfnd::Array<GrbInteropEvent3> * getEventStream()
	{
		return eventStream;
	}
	
private:

	shdfnd::Array<GrbInteropEvent3> * eventStream;

	//We need to keep a copy of the NpScene pointer
	//in order to delete the contents of the stream
	//at the end of the frame. The deletion has to
	//be done by NpScene because the PhysX dll will
	//probably have a different heap to the calling
	//code
	NpScene * scene;
};

};

#endif
