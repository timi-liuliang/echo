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
#include "ConfigurationBlock.h"
#include "..\maxheap.h"

#define _CLIENT_INFO_VERSION 400

/*! \sa Class MaxNetManager, Structure ConfigurationBlock
\remarks This structure is available in release 4.0 and later only. \n\n
This structure is used by the Network Rendering API to store information about a Client.
*/
struct ClientInfo : public MaxHeapOperators
{
	/*! The size of the structure, being sizeof(ClientInfo). */
	DWORD				size;
	/*! The version information, defined by _CLIENTINFO_VERSION. */
	DWORD				version;
	/*! The network system configuration data. Refer to this structure for more information. */
	ConfigurationBlock	cfg;
	/*! TRUE if the Client is currently controlling the queue. */
	bool				controller;
	/*! The UDP port being used for network communications. */
	short		  		udp_port;
	/*! Reserved for future use. */
	char				reserved[32];
};
