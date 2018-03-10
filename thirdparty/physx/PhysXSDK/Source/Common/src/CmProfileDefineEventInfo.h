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
#define CM_PROFILE_DEFINE_EVENT_INFO_H


/**
 *	This header expects the macro:
 *	PX_PROFILE_EVENT_DEFINITION_HEADER to be defined to a string which is included 
 *	multiple times.
 *
 *	This header needs to be of the form:
 *	
PX_PROFILE_BEGIN_SUBSYSTEM( Subsystem1 )
PX_PROFILE_EVENT( Subsystem1,Event1 )
PX_PROFILE_EVENT( Subsystem1,Event2 )
PX_PROFILE_EVENT( Subsystem1,Event3 )
PX_PROFILE_END_SUBSYSTEM( Subsystem1 )
 *
 *	Produces gPxProfileNames, a copyable object that names all the events and 
 *	subsystems.
 *
 *	
 */
#include "physxprofilesdk/PxProfileCompileTimeEventFilter.h"
#include "PsUtilities.h"

#define PX_PROFILE_BEGIN_SUBSYSTEM( subsys )
#define PX_PROFILE_EVENT( subsystem, name, priority ) PxProfileEventName( #subsystem "." #name, PxProfileEventId( physx::profile::EventIds::subsystem##name, PX_PROFILE_EVENT_FILTER_VALUE( subsystem, name ) ) ),
#define PX_PROFILE_EVENT_DETAIL( subsystem, name, priority ) PxProfileEventName( "." #subsystem "." #name, PxProfileEventId( physx::profile::EventIds::subsystem##name, PX_PROFILE_EVENT_FILTER_VALUE( subsystem, name ) ) ),	
#define PX_PROFILE_END_SUBSYSTEM( subsys ) 
static PxProfileEventName gEventNames[] = {
#include PX_PROFILE_EVENT_DEFINITION_HEADER
};
#undef PX_PROFILE_BEGIN_SUBSYSTEM
#undef PX_PROFILE_EVENT
#undef PX_PROFILE_EVENT_DETAIL
#undef PX_PROFILE_END_SUBSYSTEM

static PxU32 gEventNamesCount = PX_ARRAY_SIZE( gEventNames );
static PxProfileNames gPxProfileNames( gEventNamesCount, gEventNames );

#undef CM_PROFILE_DEFINE_EVENT_INFO_H
