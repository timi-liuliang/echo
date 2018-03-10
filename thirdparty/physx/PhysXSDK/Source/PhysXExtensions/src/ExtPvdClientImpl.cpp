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
  
#include "PxVisualDebuggerExt.h"

#if PX_SUPPORT_VISUAL_DEBUGGER
#include "PvdNetworkStreams.h"
#include "PvdConnectionManager.h"
#include "PsFoundation.h"
using namespace physx::shdfnd;
using namespace physx;
using namespace physx::debugger;
using namespace physx::debugger::comm;


PvdConnection* physx::PxVisualDebuggerExt::createConnection( PvdConnectionManager* inMgr
													, const char* inHost
													, int inPort //defaults to 5425
													, unsigned int inTimeoutInMilliseconds
													, PxVisualDebuggerConnectionFlags inConnectionType )
{
	TConnectionFlagsType theFlags( (PxU32)inConnectionType );
	inMgr->connect( Foundation::getInstance().getAllocator(), inHost, inPort, inTimeoutInMilliseconds, theFlags );

	return inMgr->getAndAddRefCurrentConnection();
}

PvdConnection* physx::PxVisualDebuggerExt::createConnection( PvdConnectionManager* inMgr
													, const char* filename
													, PxVisualDebuggerConnectionFlags inConnectionType )
{
	TConnectionFlagsType theFlags( (PxU32)inConnectionType );
	inMgr->connect( Foundation::getInstance().getAllocator(), filename, theFlags );

	return inMgr->getAndAddRefCurrentConnection();
}

#else
namespace physx { namespace debugger { namespace comm {
class PvdConnectionManager;
class PvdConnection;
}}}

physx::debugger::comm::PvdConnection* physx::PxVisualDebuggerExt::createConnection( physx::debugger::comm::PvdConnectionManager* 
													, const char* 
													, int  
													, unsigned int 
													, PxVisualDebuggerConnectionFlags  )
{	
	return NULL;
}

physx::debugger::comm::PvdConnection* physx::PxVisualDebuggerExt::createConnection( physx::debugger::comm::PvdConnectionManager* 
													, const char* 
													, PxVisualDebuggerConnectionFlags  )
{
	return NULL;
}

#endif //PX_SUPPORT_VISUAL_DEBUGGER
