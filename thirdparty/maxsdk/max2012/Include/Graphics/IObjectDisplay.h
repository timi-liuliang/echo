//
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form.   
//
//

#pragma once

#include "./Types.h"
#include "./MaterialRequiredStreams.h"
#include "./RenderItemHandle.h"

#include "../BaseInterface.h"

/** Interface id of IObjectDisplay.
*/
#define IOBJECT_DISPLAY_INTERFACE_ID Interface_ID(0x39820fa0, 0x6311340d)

namespace MaxSDK { namespace Graphics {

/** This is the base interface for all objects that want to be displayed in viewport. 
A plug-in which desires to be displayed in the viewport must expose this interface 
to 3ds Max via the GetInterface function. Max will then obtain graphics data via 
this interfaces. The possible plug-in types that need this interface are: Modifiers, 
Objects and Controllers. 

Internally plugin should maintain a set of render items and manage the life time 
of all those items. A render item should be created by calling XXXRenderItem::CreateInstance() 
and release via its ReleaseInterface function. plugin needs to decide the type of the 
most fit render items and fill in correct graphics data into them.
*/
class IObjectDisplay : public BaseInterface
{
public:
	IObjectDisplay() {}
	virtual ~IObjectDisplay() {}

	/** Check if this plugin contains legacy display code and want legacy code 
	to be called as well.
	\return true if contains legacy display code, false otherwise.
	\remarks default return value is false.
	*/
	virtual bool RequiresSupportForLegacyDisplayMode() const = 0;

	/** Manually update all render items of this object. Plugin should build 
	render items in this function to satisfy the input category, flags and required streams.
	\param renderItemCategories A combination of RenderItemCategory
	\param materialRequiredStreams required streams of render items
	\param t current time.
	\return true if update successfully, false otherwise.
	*/
	virtual bool UpdateDisplay(
		unsigned long renderItemCategories, 
		const MaterialRequiredStreams& materialRequiredStreams, 
		TimeValue t) = 0;

	/** Retrieves the render items to be displayed.
	\return the render items to be displayed.
	*/
	virtual const RenderItemHandleArray& GetRenderItems() const = 0;

	// from BaseInterface
	virtual Interface_ID GetID() {
		return IOBJECT_DISPLAY_INTERFACE_ID;
	}

	virtual BaseInterface* GetInterface(Interface_ID id) {
		if (id == IOBJECT_DISPLAY_INTERFACE_ID) {
			return this;
		}

		return BaseInterface::GetInterface(id);
	}

	// internally used only
	void SetDirtyFlag(unsigned long flags) {
		mFlags = flags;
	}

	unsigned long GetDirtyFlag() const {
		return mFlags;
	}

private:
	unsigned long mFlags;
};

} } // end namespace
