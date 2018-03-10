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


#include "CmEventProfiler.h"

using namespace physx;
using namespace physx::profile;

#include "CmProfileDefineEventInfo.h"

namespace physx
{
namespace Cm
{
	physx::PxProfileNames CmEventNameProvider::getProfileNames() const { return gPxProfileNames; }

#if PX_NVTX	
	const char* EventProfiler::getStringFromId(PxU16 id)
	{
		return gEventNames[id].mName;
	}
#endif

#define PX_PROFILE_BEGIN_SUBSYSTEM( subsys )
#define PX_PROFILE_EVENT( subsys, name, priority ) PX_UNIX_EXPORT const physx::PxProfileEventId ProfileEventId::subsys::name = PX_PROFILE_EVENT_ID( subsys, name );
#define PX_PROFILE_EVENT_DETAIL( subsys, name, priority ) PX_PROFILE_EVENT( subsys, name, priority )
#define PX_PROFILE_END_SUBSYSTEM( subsys )
#include "CmProfileEventDefs.h"	
#undef PX_PROFILE_BEGIN_SUBSYSTEM
#undef PX_PROFILE_EVENT
#undef PX_PROFILE_EVENT_DETAIL
#undef PX_PROFILE_END_SUBSYSTEM

// defines whether the NVTX support is enabled or not
static bool sNVTXSupportEnabled = true;

void enableNVTXSupport(bool enable)
{
	sNVTXSupportEnabled = enable;
}

bool isNVTXSupportEnabled()
{
	return sNVTXSupportEnabled;
}

}
}
