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


#include "ScbBase.h"
#include "ScbNpDeps.h"

using namespace physx;
using namespace Scb;

void Base::destroy()
{
	if(!isBuffering())
		NpDestroy(*this);
	else
	{
		// Schedule for destroy
		PX_ASSERT(!(getControlFlags() & ControlFlag::eIS_RELEASED));
		PX_ASSERT(getControlState() == ControlState::eREMOVE_PENDING);
		setControlFlag(ControlFlag::eIS_RELEASED);
	}
}
