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
// DESCRIPTION: File Resolution Manager => resolves the physical location of
// an asset file
// AUTHOR: Peter Ochodo
// DATE: 2008-06-04 (YYYY-MM-DD) 
//***************************************************************************/

#pragma once

#include <Rpc.h>
#include <cguid.h>
#include "..\maxheap.h"

namespace MaxSDK
{
	namespace AssetManagement
	{
		struct AssetId : GUID, public MaxHeapOperators
		{
			AssetId() { *static_cast<GUID*>(this) = CLSID_NULL; }
		};

		static AssetId kInvalidId;

	}
}

