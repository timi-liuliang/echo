//**************************************************************************/
// Copyright (c) 1998-2008 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
#pragma once

#include <WTypes.h>
#include "..\strbasic.h"
#include "..\maxheap.h"

/*! \sa Class MaxNetManager
\remarks This structure is available in release 4.0 and later only. \n\n
This structure is used by the Network Rendering API to store network status information. */
struct NetworkStatus : public MaxHeapOperators
{
	/*! The number of packets dropped due to buffer overflows. */
	DWORD		dropped_packets;
	/*! The number of bad formed packets. */
	DWORD		bad_packets;
	/*! The total number of TCP requests since boot time. */
	DWORD		tcprequests;
	/*! The total number of UDP requests since boot time. */
	DWORD		udprequests;
	/*! The system boot time. */
	SYSTEMTIME	boot_time;
	/*! Reserved for future use. */
	char		reserved[32];
};
