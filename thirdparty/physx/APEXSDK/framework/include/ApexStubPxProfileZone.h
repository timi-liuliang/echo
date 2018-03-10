/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_STUB_PX_PROFILE_ZONE_H
#define APEX_STUB_PX_PROFILE_ZONE_H

#include "Px.h"
#include "PxProfileZone.h"
#include "PsUserAllocated.h"

namespace physx
{
	class PxUserCustomProfiler;

namespace apex
{

// This class provides a stub implementation of PhysX's PxProfileZone.
// It would be nice to not be forced to do this, but our scoped profile event macros
// cannot have an if(gProfileZone) because it would ruin the scope.  So here we just
// create a stub that will be called so that the user need not create a PxProfileZoneManager
// in debug mode (and suffer an assertion).

class ApexStubPxProfileZone : public physx::PxProfileZone, public physx::UserAllocated
{
public:	

	// physx::PxProfileZone methods
	virtual const char* getName() { return 0; }
	virtual void release() { PX_DELETE(this); }

	virtual void setProfileZoneManager(physx::PxProfileZoneManager* ) {}
	virtual physx::PxProfileZoneManager* getProfileZoneManager() { return 0; }

	virtual PxU16 getEventIdForName( const char*  ) { return 0; }

	virtual PxU16 getEventIdsForNames( const char** , PxU32  ) { return 0; }
	virtual void setUserCustomProfiler(PxUserCustomProfiler* ) {};

	// physx::PxProfileEventBufferClientManager methods
	virtual void addClient( PxProfileZoneClient&  ) {}
	virtual void removeClient( PxProfileZoneClient&  ) {}
	virtual bool hasClients() const { return false; }

	// physx::PxProfileNameProvider methods
	virtual physx::PxProfileNames getProfileNames() const { return PxProfileNames(); }

	// physx::PxProfileEventSender methods
	virtual void startEvent( PxU16 , PxU64 ) {}
	virtual void stopEvent( PxU16 , PxU64 ) {}

	virtual void startEvent( PxU16 , PxU64 , PxU32 ) {}
	virtual void stopEvent( PxU16 , PxU64 , PxU32  ) {}
	virtual void eventValue( PxU16 , PxU64 , PxI64  ) {}

	virtual void CUDAProfileBuffer( PxF32 , const PxU8* , PxU32 , PxU32  ) {}

	// physx::PxProfileEventFlusher methods
	virtual void flushProfileEvents() {}
};

}
} // end namespace physx::apex

#endif // APEX_STUB_PX_PROFILE_ZONE_H
