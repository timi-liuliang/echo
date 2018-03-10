//
// Copyright [2009] Autodesk, Inc.  All rights reserved. 
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law.  They may
// not be disclosed to, copied or used by any third party without the prior
// written consent of Autodesk, Inc.
//
//

#pragma once

#include "ifnpub.h"
#include "strbasic.h"
#include "plugapi.h"

class Bitmap;

//! \brief Interface ID for the IMaterialBrowserEntryInfo Interface 
/*! \remarks If a plug-in implements the IMaterialBrowserEntryInfo interface, it should return
	an instance of the class in response to ClassDesc::GetInterface(IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE),
	called by the system on the plug-in class descriptor.
\see IMaterialBrowserEntryInfo
*/
#define IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE Interface_ID(0x5e19378b, 0x55535e13)

//! \brief This interface allows materials and textures to customize their appearance in the Material Browser
/*! \remarks This class is optional, and may be provided by plug-ins of superclass MATERIAL_CLASS_ID or TEXMAP_CLASS_ID.
	There is no obligation to implement it, and no guarantee it will be provided by internal materials or textures.
	If implemented, the plug-in class should return its instance of this interface in response to 
	ClassDesc::GetInterface(IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE).  The system fetches the interface from the ClassDesc.
	The interface allows a plug-in to customize the default appearance of its entries, as shown in the Material Browser.
	This includes the display name, the thumbnail, and the location (or category) of entries.
	The user may further customize the appearance of the Material Browser;
	those customizations are saved in a configuration file, and may override values provided by this class.
	The class is implemented by the plug-in.
\see IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE
*/
class IMaterialBrowserEntryInfo : public FPInterface
{
public:

	inline Interface_ID GetID() { return IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE; }

	inline BaseInterface* GetInterface(Interface_ID id) {
		if( id==GetID() ) return this;
		else return FPInterface::GetInterface(id);
	}

	inline FPInterfaceDesc* GetDesc() { return NULL; }

	/*! Get the UI name of the entry.
		\details This name will be displayed in the Material Browser, instead of the class name from the class descriptor.
		\return The UI name of the entry.
	*/	
	virtual const MCHAR* GetEntryName() const = 0;

	/*! The category the entry belongs to.
		\details  This is the group, plus possible subgroups, containing the entry in the Material Browser.
		If the entry is nested into subgroups, the hierarchy should be formatted like a filepath.  For example "A\\B\\C" indicates
		an outermost category of "A" and an innermost category of "C".  No backslash should be present before the outer category, "A".
		\return The category string of the entry.
	*/	
	virtual const MCHAR* GetEntryCategory() const = 0;

	/*! \details This method is for internal use only. Plug-ins should return false. */
	inline virtual bool IsStockObject() const { return false; }

	/*! Get the UI thumbnail of the entry, if any.
		\details This thumbnail will be displayed in the Material Browser, instead of a rendered sample.
		This improves performance by avoiding a render, and allows a customized display in the browser.
		The system automatically displays a scaled image according to the current thumbnail size, but
		the preferred size is 100x100 pixels, to accommodate large thumbnails without upscaling.
		\return The thumbnail bitmap of the entry, or NULL if none.
	*/	
	virtual Bitmap* GetEntryThumbnail() const = 0;

	/*! Returns whether the entry supports a custom thumbnail via GetThumbnail().
		\details If this returns false, the system will render a thumbnail for display in the Material Browser.
		\return true if a thumbnail can be provided, false otherwise.
		\see IMaterialBrowserEntryInfo::GetThumbnail
	*/	
	inline virtual bool HasEntryThumbnail() const { return false; }
};
