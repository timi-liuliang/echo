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


#include "PsPAEventSrc.h"

namespace physx
{
namespace shdfnd
{

/* PAUtils */

PAUtils::PAUtils()
{
}


PAUtils::~PAUtils()
{
}


EventID PAUtils::registerEvent(const char *name)
{
	return INVALID_EVENT_ID;
}


bool PAUtils::isEnabled()
{
	return false;
}


void PAUtils::rawEvent(EventID id, PxU32 data0, PxU32 data1, PxU8 data2)
{
}


void PAUtils::startEvent(EventID id, PxU16 data)
{
}


void PAUtils::stopEvent(EventID id, PxU16 data)
{
}


void PAUtils::statEvent(EventID id, PxU32 stat)
{
}


void PAUtils::statEvent(EventID id, PxU32 stat, PxU32 ident)
{
}


void PAUtils::debugEvent(EventID id, PxU32 data0, PxU32 data1)
{
}


bool PAUtils::isEventEnabled(EventID id)
{
	return false;
}


} // end shdfnd namespace
} // end physx namespace
