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

#include "MaxHeap.h"
// forward declarations
namespace MaxSDK
{
	namespace AssetManagement
	{
		class AssetUser;
	}
}

// A callback object passed to EnumAuxFiles().
/*! \sa  Class Animatable, Class NameTab\n\n
\par Description:
This class is the callback object passed to <b>Interface::EnumAuxFiles()</b>
and to <b>Animatable::EnumAuxFiles()</b>. A developer derives a class from this
class and implements the <b>RecordAsset()</b> method to store each asset as it's
passed to it. At the end of the <b>EnumAuxFiles()</b> processing, the table of
names may be used. See Class NameTab for help
storing the names.  */
class AssetEnumCallback : public MaxHeapOperators
{
public:
	/*! \remarks Destructor. */
	virtual ~AssetEnumCallback() {}
	/*! \remarks This method is used to record the asset passed.
	\par Parameters:
	<b>const MaxSDK::AssetManagement::AssetUser& asset</b>\n\n
	The asset to store. */
	virtual void RecordAsset(const MaxSDK::AssetManagement::AssetUser& asset)=0;
};