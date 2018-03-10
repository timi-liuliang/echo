/**********************************************************************
 *<
	FILE:  impexpintf.h

	DESCRIPTION:  Import/Export Interface (namespace)

	CREATED BY: Pete Samson

	HISTORY: created 7 January 2002

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/
#pragma once

#include <WTypes.h>
#include "utilexp.h"

namespace ImportExportInterface
{
	/*! Plug-in calls to say whether plug-in can _both_ convert to/from current
	system units _and_ ignore them (no conversion) */
	UtilExport void SetCanConvertUnits(bool ifSo);
	//! Max calls to ask how plug-in set canConvertUnits
	UtilExport bool GetCanConvertUnits();
	//! Max calls (if plug-in can convert units) to ask user  whether to do so
	UtilExport int AskUserConvertUnits(HWND hWnd, bool isExport);
	/*! Plug-in calls to ask whether user has indicated to convert to/from system
	units (only should be called if last call to
	SetCanConvertUnits(ifSo) had arg value true) */
	UtilExport bool GetShouldConvertUnits();		
};


