//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
#pragma once

#include "CoreExport.h"
#include "AnimProperty.h"
#include "AnimPropertyID.h"
#include "maxtypes.h"

// forward declarations
class ILoad;
class ISave;

static const DWORD SV_NO_REF_INDEX = 0xFFFFFFFF;
class SchematicViewProperty : public AnimProperty
{
private:
	DWORD nodeRefIndex;

public:				
	DWORD ID() { return PROPID_SVDATA; }
	CoreExport ~SchematicViewProperty() {}
	CoreExport SchematicViewProperty();

	DWORD GetRefIndex();
	void SetRefIndex(DWORD refIndex);
	bool GetRefSaveMark();

	CoreExport IOResult Load(ILoad *iload);
	CoreExport IOResult Save(ISave *isave);
};