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

#include "..\maxheap.h"
#include "..\maxnet_types.h"

//   Server Information for a given Job

#define JOB_SRV_IDLE		0		//-- Idle
#define JOB_SRV_BUSY		1		//-- Busy
#define JOB_SRV_FAILED		2		//-- Render Error
#define JOB_SRV_ABSENT		3		//-- Absent
#define JOB_SRV_SUSPENDED	4		//-- Out of work schedule
#define JOB_SRV_BUSYOTHER	5		//-- Busy with another job
#define JOB_SRV_ERROR		6		//-- Connection Error
#define JOB_SRV_COOL_OFF	7		//-- In Error Recovery

/*! \sa Class MaxNetManager, Structure HSERVER
\remarks This structure is available in release 4.0 and later only. \n\n
This structure is used by the Network Rendering API to store information about the servers in the job queue.
*/
struct JobServer : public MaxHeapOperators
{
	/*! The handle to the server. */
	HSERVER		hServer;
	/*! The current state of the server, which is one of the following values:
	- JOB_SRV_IDLE \n
	The server is idle.
	- JOB_SRV_BUSY \n
	The server is busy.
	- JOB_SRV_FAILED  \n
	The server is has failed (rendering error).
	- JOB_SRV_ABSENT \n
	The server is absent.
	- JOB_SRV_SUSPENDED \n
	The server is suspended, out of work schedule.
	- JOB_SRV_BUSYOTHER    \n
	The server is busy with another job.
	- JOB_SRV_ERROR \n
	The server experienced a connection error.
	- JOB_SRV_COOL_OFF \n
	The server is "cooling off" (i.e. in error recovery).
	*/
	char	  	status;
	/*! This variable is used internally. */
	bool		failed;
	/*! Indicates that the server is active in the job. */
	bool		active;
	/*! The frame which is currently being rendered. */
	int		cur_frame;			
	/*! The total hours the server has spent rendering. */
	float		thours;
	/*! The total number of frames the server has rendered. */
	int		frames;
};
