/**********************************************************************
 *<
	FILE: utilexp.h

	DESCRIPTION:

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once

#ifdef BLD_UTIL
#define UtilExport __declspec( dllexport )
#else
#define UtilExport __declspec( dllimport )
#endif

