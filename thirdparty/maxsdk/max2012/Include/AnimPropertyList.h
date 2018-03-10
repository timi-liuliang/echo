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

#include "AnimProperty.h"
#include "CoreExport.h"
#include "tab.h"

/*! \sa  Class AnimProperty, Class Animatable, Class Tab\n\n
\par Description:
This class is simply a table of anim properties. It has a single method to
locate a given id in the list. See Class Tab
for details on how to manipulate the table.  */
class AnimPropertyList : public Tab<AnimProperty*> {
	public:
		/*! \remarks Implemented by the System.\n\n
		Returns the table index of the property whose id is passed.
		\par Parameters:
		<b>DWORD id</b>\n\n
		The id to find.\n\n
		<b>int start=0</b>\n\n
		A table index that may be used to define an alternate starting point
		for the search. This is a index into the table of properties.
		\return  The table index of the property, or if not found, -1. */
		CoreExport int FindProperty(DWORD id,int start=0);
	};