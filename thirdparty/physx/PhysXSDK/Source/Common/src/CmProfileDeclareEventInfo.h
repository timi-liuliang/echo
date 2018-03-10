/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

// no include guard on purpose!
#define CM_PROFILE_DECLARE_EVENT_INFO_H


/**
 *	This header expects the macro:
 *	PX_PROFILE_EVENT_DEFINITION_HEADER to be defined to a string which is included 
 *	in order to produce the enum and event id lists.
 * 
 *	This header needs to be of the form:
 *	
PX_PROFILE_BEGIN_SUBSYSTEM( Subsystem1 )
PX_PROFILE_EVENT( Subsystem1,Event1, Coarse )
PX_PROFILE_EVENT( Subsystem1,Event2, Detail )
PX_PROFILE_EVENT( Subsystem1,Event3, Medium )
PX_PROFILE_END_SUBSYSTEM( Subsystem1 )
 */
#include "physxprofilesdk/PxProfileCompileTimeEventFilter.h"
#include "physxprofilesdk/PxProfileEventNames.h"


namespace physx { namespace profile {

//Event id enumeration
#define PX_PROFILE_BEGIN_SUBSYSTEM( subsys ) 
#define PX_PROFILE_EVENT( subsystem, name, priority ) subsystem##name,
#define PX_PROFILE_EVENT_DETAIL( subsystem, name, priority ) PX_PROFILE_EVENT(subsystem, name, priority )
#define PX_PROFILE_EVENT_NO_COMMA( subsystem, name, priority ) subsystem##name
#define PX_PROFILE_END_SUBSYSTEM( subsys )
struct EventIds
{
	enum Enum
	{
#include PX_PROFILE_EVENT_DEFINITION_HEADER
	};
};
#undef PX_PROFILE_BEGIN_SUBSYSTEM
#undef PX_PROFILE_EVENT_NO_COMMA 
#undef PX_PROFILE_EVENT
#undef PX_PROFILE_EVENT_DETAIL
#undef PX_PROFILE_END_SUBSYSTEM


//Event priority definition
#define PX_PROFILE_BEGIN_SUBSYSTEM( subsys )
#define PX_PROFILE_EVENT( subsys, name, priority ) \
	template<> struct EventPriority<EventIds::subsys##name> { static const PxU32 val = EventPriorities::priority; };
#define PX_PROFILE_EVENT_DETAIL( subsys, name, priority ) PX_PROFILE_EVENT( subsys, name, priority )
#define PX_PROFILE_END_SUBSYSTEM( subsys )
#include PX_PROFILE_EVENT_DEFINITION_HEADER
#undef PX_PROFILE_BEGIN_SUBSYSTEM
#undef PX_PROFILE_EVENT
#undef PX_PROFILE_EVENT_DETAIL
#undef PX_PROFILE_END_SUBSYSTEM

} }


#undef CM_PROFILE_DECLARE_EVENT_INFO_H
