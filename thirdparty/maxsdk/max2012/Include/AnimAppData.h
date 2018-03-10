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
#include "tab.h"
#include "maxtypes.h"
#include "AnimPropertyID.h"

// forward declarations
class AppDataChunk;
class ILoad;
class ISave;

// This list is maintained by the systems. Plug-ins need not concern themselves with it.
class AnimAppData : public AnimProperty {
	public:				
		Tab<AppDataChunk*> chunks;
		CRITICAL_SECTION csect;
		AppDataChunk *lastSearch;

		DWORD ID() {return PROPID_APPDATA;}		
		CoreExport ~AnimAppData();
		CoreExport AnimAppData();

		CoreExport AppDataChunk *FindChunk(Class_ID cid, SClass_ID sid, DWORD sbid);
		void AddChunk(AppDataChunk *newChunk) {chunks.Append(1,&newChunk);}
		CoreExport BOOL RemoveChunk(Class_ID cid, SClass_ID sid, DWORD sbid);

		CoreExport IOResult Load(ILoad *iload);
		CoreExport IOResult Save(ISave *isave);
	};