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
#include "..\maxnet_types.h"
#include "MaxNetExport.h"

// forward declarations
class MaxNetManager;
struct ConfigurationBlock;
struct Job;
class CJobText;

/*! \remarks This method will create and return a new instance of the
MaxNetManager class. */
MAXNETEXPORT MaxNetManager*	CreateManager			( );
/*! \remarks This method will destroy an instance of the MaxNetManager class.
\par Parameters:
<b>MaxNetManager* mgr</b>\n\n
Points to the MaxNetManager object to destroy.*/
MAXNETEXPORT void			DestroyManager			(MaxNetManager* mgr);
//-- Initializes a "Job" structure
/*! \remarks This function will initialize a job structure using the data read
from a specified 3ds Max scene file. If you would want to submit a job based on
a 3ds Max file alone, you would call this function passing it to the file
specified. This function takes care of filling all the fields so you can turn
around and just send the job to the queue. An example of this can be found in
the SDK under <b>/MAXSDK/SAMPLES/NETRENDER/JOBASSIGN</b>.
\par Parameters:
<b>char* filename</b>\n\n
The filename of the 3ds Max scene file (*.max).\n\n
<b>Job* job</b>\n\n
A pointer to an empty job structure which will be initialized by the method.
Because the function will initialize the structure any values present prior to
calling this method will be reset.\n\n
<b>CJobText\& jobText</b>\n\n
A reference to an empty CJobText class which will be initialized by the
function. Just like the job structure, any values present prior to calling this
function will be reset.
\return  TRUE if reading the properties was successful, otherwise FALSE. */
MAXNETEXPORT bool			jobReadMAXProperties	(char* max_filename, Job* job, CJobText& jobText);
//-- Reads Render Data from a *.max file and fills in a Job structure
/*! \remarks This function will set the default values for the given Job
structure. You can use this function to prevent frequent resetting of the
structure and its default fields such as size, version, etc.
\par Parameters:
<b>Job *job</b>\n\n
The job structure containing the information about the job. See the Job
structure description for an explanation. */
MAXNETEXPORT void			jobSetJobDefaults		(Job* job);
//-- Utilities
MAXNETEXPORT void			NumberedFilename		(MCHAR* infile, MCHAR* outfile, int number);
MAXNETEXPORT bool			IsMacNull				(BYTE *addr);
MAXNETEXPORT bool			GetMacAddress			(BYTE* addr);
MAXNETEXPORT bool			MatchMacAddress			(BYTE* addr1, BYTE* addr2);
MAXNETEXPORT void			Mac2String				(BYTE* addr, MCHAR* string );
MAXNETEXPORT void			Mac2StringCondensed		(BYTE* addr, MCHAR* string );
MAXNETEXPORT void			StringCondensed2Mac		(MCHAR* string, BYTE* addr);
MAXNETEXPORT void			InitConfigurationInfo	(ConfigurationBlock& cb, MCHAR workdisk = 0);
MAXNETEXPORT bool			MatchServers			(HSERVER srv1, HSERVER srv2);
/*! \remarks This function creates a Network Rendering archive. This is the
archive sent to the manager when submitting a new job. Note that even though
you can use whatever name you feel like, the Manager and Servers will look for
a "jobname.maz" file.
\par Parameters:
<b>MCHAR* archivename</b>\n\n
The full path and filename of the archive you want to create.\n\n
<b>MCHAR* file_list</b>\n\n
A list of NULL terminated filenames to include in the archive. You should
provide a full path and filename. However, only the file names will be saved in
the archive and all files are going to be un-archived in the same
directory.\n\n
An example: file_list[] =
{"c:\\path\\file.maz/0c:\\anotherpath\\maps.tga/0/0"};\n\n
<b>DWORD* filesize</b>\n\n
Optional pointer to a variable to receive the accumulated size of all files
included in the archive. This is the "uncompressed" size. You can use this to
compute the disk space necessary to uncompress the archive.
\return  TRUE if the archive was successfully created, otherwise FALSE. */
MAXNETEXPORT bool			Maz						(MCHAR* archivename, MCHAR* file_list, DWORD* filesize = 0);
/*! \remarks This function is the opposite of <b>Maz()</b> and will un-archive
the given archive into the specified directory.
\par Parameters:
<b>MCHAR* archivename</b>\n\n
The full path and filename of the archive you want to un-archive.\n\n
<b>MCHAR* output_path</b>\n\n
The path you want the files extracted to.
\return  TRUE if the archive was successfully extracted, otherwise FALSE. */
MAXNETEXPORT bool			UnMaz					(MCHAR* archivename, MCHAR* output_path);
//-- Localization Resources
MAXNETEXPORT MCHAR*			ResString				(int id, MCHAR* buffer = 0);
//-- Backburner helper
MAXNETEXPORT bool			ConvertOldJobFile		(MCHAR* oldFile, MCHAR* newFile);

enum MAXNET_PLATFORM { 
	MAXNET_PLATFORM_NATIVE = 0,
	MAXNET_PLATFORM_32BIT = 32,
	MAXNET_PLATFORM_64BIT =64
};
#ifndef NO_MAXNET_PLATFORM_OVERRIDE
	MAXNETEXPORT MaxNetManager* CreateManager		(int platform);
	MAXNETEXPORT bool			ConvertOldJobFile	(MCHAR* oldFile, MCHAR* newFile, int platform);
#endif