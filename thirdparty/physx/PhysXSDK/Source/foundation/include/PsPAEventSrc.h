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


#ifndef PSPA_EVENT_SRC_API
#define PSPA_EVENT_SRC_API

#include "Ps.h"

namespace physx
{
namespace shdfnd
{

typedef PxU16 EventID;

static const size_t  EVENT_NAME_MAX_LEN = 128;
static const EventID INVALID_EVENT_ID = (EventID) 0xffff;

struct PsEventTypes
{
	enum Enum
	{
		eSTART,
		eSTOP,
		eSTAT,
		eDEBUG
	};
};

// Manages the connection to an event collector, will be a singleton in the foundation
class PX_FOUNDATION_API PAUtils
{
public:
	PAUtils();
	~PAUtils();

	bool		isEnabled();
	bool		isEventEnabled( EventID id );

	EventID		registerEvent( const char* );
	void		startEvent( EventID id, PxU16 data );
	void		stopEvent( EventID id, PxU16 data );

	void		statEvent( EventID id, PxU32 stat );
	void		statEvent( EventID id, PxU32 stat, PxU32 ident );
	void		debugEvent( EventID id, PxU32 data0, PxU32 data1 );

    /* do not use these unless you know what you're doing */
    void		rawEvent( EventID id, PxU32 data0, PxU32 data1, PxU8 data2 );
    bool		rawEventWithTimestamp( EventID id, PxU64 timestamp, PxU32 data0, PxU32 data1, PxU8 data2 );
    bool		lock();
    bool		unlock();

private:
	class ConnImpl* mImpl;
};

} // end namespace shdfnd
} // end namespace physx

#endif
