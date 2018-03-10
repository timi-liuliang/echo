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
#include "..\strbasic.h"


/*! \sa Class MaxNetManager, Structure ManagerInfo, Structure ServerInfo, Structure ClientInfo
\remarks This structure is available in release 4.0 and later only. \n\n
This structure is used by the Network Rendering API to store information about the system.
*/
struct ConfigurationBlock : public MaxHeapOperators
{
	/*! The systems total physical memory, retrieved through GlobalMemoryStatus(). */
	SIZE_T dwTotalPhys;			   //-- GlobalMemoryStatus();
	/*! The number of processors in the system, retrieved through GetSystemInfo(). */
	DWORD	dwNumberOfProcessors;	//-- GetSystemInfo();
	/*! The systems major version, retrieved through GetVersionEx(). */
	DWORD	dwMajorVersion;			//-- GetVersionEx();
	/*! The systems minor version, retrieved through GetVersionEx(). */
	DWORD	dwMinorVersion;			//-- GetVersionEx();
	/*! The systems build number, retrieved through GetVersionEx(). */
	DWORD	dwBuildNumber;			//-- GetVersionEx();
	/*! The systems platform ID, retrieved through GetVersionEx(). */
	DWORD	dwPlatformId;			//-- GetVersionEx();
	/*! The systems CSD version, retrieved through GetVersionEx(). */
	MCHAR	szCSDVersion[128];		//-- GetVersionEx();
	/*! The logged in users name, retrieved through GetUserName(). */
	MCHAR	user[MAX_PATH];			//-- GetUserName();
	/*! The temporary directory, retrieved through ExpandEnvironmentStrings(). */
	MCHAR	tempdir[MAX_PATH];		//-- ExpandEnvironmentStrings()
	/*! The name of the computer, retrieved through GetComputerName(). */
	MCHAR	name[MAX_PATH];			//-- GetComputerName()
	/*! The disk, indexed by drive letter, used for server files such as
	incoming jobs. The index is alphabetical, disk A = 0, disk B = 1, etc. */
	char	workDisk;				//-- Disk used for Server files (incomming jobs, etc. A = 0, B = 1, etc)
	/*! The bit map representing the available disks. 
	Disk A = 0x1, B = 0x2, C = 0x4, etc. */
	DWORD	disks;					//-- Available disks (bitmap A=0x1, B=0x2, C=0x4, etc)
	/*! The space available on disks in MegaBytes, indexed by 
	drive letter. Disk A = diskSpace[0], B = diskSpace[1], etc. */
	DWORD	diskSpace[26];			//-- Space available on disks in MegaBytes (A=diskSpace[0], B=diskSpace[1], etc.)
	/*! The computer NIC hardware address (00:00:00:00:00:00), 
	which is 6 bytes and 2 padding bytes. */
	BYTE	mac[8];					//-- Computer NIC address (00:00:00:00:00:00) 6 bytes + 2 padding
	/*! Reserved for future use. */
	char	reserved[32];
};