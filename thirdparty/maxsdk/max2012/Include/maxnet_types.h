//
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.  
//
//
#pragma once

#include <WTypes.h>
#include "MaxHeap.h"

/** MaxNet Errors*/
enum maxnet_error_t
{
	MAXNET_ERR_NONE = 0,			//!< No Error
	MAXNET_ERR_CANCEL,				//!< A cancellation Error has occured.
	MAXNET_ERR_NOMEMORY,			//!< An out of memory error has occured.
	MAXNET_ERR_FILEIO,				//!< A file IO error has occured.
	MAXNET_ERR_BADARGUMENT,			//!< Bad arguments were passed along.
	MAXNET_ERR_NOTCONNECTED,		//!< A connection was not established.
	MAXNET_ERR_NOTREADY,			//!< Windows network is not installed or is not initialized.
	MAXNET_ERR_IOERROR,				//!< An IO error has occurred.
	MAXNET_ERR_CMDERROR,			//!< A command error has occurred.
	MAXNET_ERR_HOSTNOTFOUND,		//!< The specified host could not be found.
	MAXNET_ERR_BADSOCKETVERSION,	//!< Winsock.dll is obsolete.
	MAXNET_ERR_WOULDBLOCK,			//!< An internal blocking error has occurred.
	MAXNET_ERR_SOCKETLIMIT,			//!< No more available TCP/IP Sockets.
	MAXNET_ERR_CONNECTIONREFUSED,	//!< A connection has been refused. Service not installed on host computer.
	MAXNET_ERR_ACCESSDENIED,		//!< Access to a host was denied.
	MAXNET_ERR_TIMEOUT,				//!< A network time-out has occurred.
	MAXNET_ERR_BADADDRESS,			//!< A bad network address was supplied.
	MAXNET_ERR_HOSTUNREACH,			//!< The specified host is unreachable.
	MAXNET_ERR_DUPLICATE_JOB_NAME,	//!< Job name already exists.
	MAXNET_ERR_UNKNOWN				//!< An unknown error has occurred.
};

//---------------------------------------------------------
//-- Special Types

/** Used by the Network Rendering API as a handle to a server.
 * \see MaxNetManager */
struct HSERVER : public MaxHeapOperators {
	
	BYTE addr[8]; //!<The handle to a server by its mac (Ethernet) address.
};

#define HBSERVER	(BYTE *)(void *) 
typedef DWORD		HJOB; //!< Job Handle


/** Used by the Network Rendering API to store hourly scheduling information.
 * \see MaxNetManager, WeekSchedule*/
struct Schedule: public MaxHeapOperators {
	/*! This bit map represents the hourly schedule where 24 bits represent the hours.
	A bit set to 0 indicates it is allowed to work, a bit set to 1 indicates
	its not allowed to work.
	*/
	DWORD hour;
};

 /** Used by the Network Rendering API to store weekly scheduling information.
 * \see MaxNetManager, Schedule */
struct WeekSchedule: public MaxHeapOperators {
	/** The hourly schedule configuration for each day of the week. */
	Schedule	day[7];
	/** The attended priority value which is one of the following values: HIGHPRIORITY, LOWPRIORITY, or IDLEPRIORITY. */
	int		AttendedPriority;
	/** The unattended priority value which is one of the following values: HIGHPRIORITY, LOWPRIORITY, or IDLEPRIORITY. */
	int		UnattendedPriority;
};
