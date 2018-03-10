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
#include "..\maxheap.h"
#include "..\maxnet_types.h"
#include "ServerInfo.h"


#define SERVER_STATE_ABSENT    0
#define SERVER_STATE_IDLE      1
#define SERVER_STATE_BUSY      2
#define SERVER_STATE_ERROR     3
#define SERVER_STATE_SUSPENDED 4

/*! \sa Class MaxNetManager, Structure HSERVER, Structure ServerInfo
\remarks This structure is available in release 4.0 and later only.\n\n
This structure is used by the Network Rendering API to store global 
server state information.\n\n
If <b>hJob</b> is a valid handle (i.e. non 0) and <b>frame</b> holds a <b>NO_FRAME</b>,
this means this server has just be assigned a job or it is in between
frames (no frames assigned). Most likely it is loading 3ds Max. The
transition in between frames is in the nanosecond level, but seeing
as it is possible, it cannot be discarded.
*/
struct ServerList : public MaxHeapOperators
{
	/*! The handle to the server. */
	HSERVER		hServer;
	/*! The current state of the server which is one of the following values:
	- SERVER_STATE_ABSENT   \n
	The server is absent.
	- SERVER_STATE_IDLE \n
	The server is idle.
	- SERVER_STATE_BUSY \n
	The server is busy.
	- SERVER_STATE_ERROR \n
	The server is experiencing an error.
	- SERVER_STATE_SUSPENDED \n
	The server has been suspended. */
	WORD		state;
	/*! The server information structure containing the server details. */
	ServerInfo	info;
	//-- Current Task
	/*! The handle to the job that the server is currently working on, if there is one. */
	HJOB  		hJob;			//-- It will be 0 if no current task is defined
	/*! The frame the server is currently rendering, if there is one. */
	int		frame;			//-- It will be NO_FRAME if loading job (no frames yet assigned)
	/*! The time the server had the frame assigned. */
	SYSTEMTIME	frame_started;
};
