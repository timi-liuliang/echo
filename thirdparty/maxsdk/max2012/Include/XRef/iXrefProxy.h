//**************************************************************************/
// Copyright (c) 1998-2005 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Interface for working with xref that support proxies
// AUTHOR: Attila Szabo - created Feb.09.2005
//***************************************************************************/

#pragma once

#include "..\iFnPub.h"
#include "..\AssetManagement\AssetUser.h"

// Foreward declarations
class IObjXRefRecord;

//! Id for IXRefProxy interface
#define IID_XREF_PROXY Interface_ID(0x30d59df, 0x2f06322b)

//! \brief Class for working with the proxy aspect of an xref scene entity.
/*! Some types of xref scene entities allow to be replaced by a proxy (an alternate 
scene entity) that may reside in a different scene than the xrefed scene entity itself.
The proxy scene entity can have a different type (superclass and class ids) than the 
xrefed scene entity. Proxies can be used to replace the actual xref scene entity in the 
viewport and/or rendering. Some xref scene entities provided by the system, such as xref objects,
expose the IXRefProxy interface. 
In order to retrieve this interface one must call IXRefProxy::GetInterface with a 
reference to a scene entity.
*/
class IXRefProxy : public FPMixinInterface
{
public:
	//! \brief Retrieves the proxy file asset.
	/*! \return - The proxy file asset.
	*/
	virtual const MaxSDK::AssetManagement::AssetUser& GetProxyFile() const = 0;

	//! \brief Retrieves the name of the proxy scene entity. 
	/*! \return - Pointer to the name of the proxy scene entity or NULL if there's no proxy object set up.
	*/
	virtual const MCHAR* GetProxyItemName() const = 0;

	//! \brief Retrieves the xref record this xref proxy belongs to.
	/*! \return The xref record this xref proxy belongs to. An xref proxy can belong to 
	only one xref record at any one time. A return value of NULL, indicates that the proxy 
	is not part of an xref record yet.
	*/
	virtual IObjXRefRecord* GetProxyRecord() const = 0;

	//! \brief Allows to turn on or off the use of the proxy for viewport display.
	/*! \param [in] bUseProxy - If true, the proxy is to be displayed in the viewport
	instead of the xrefed scene entity, otherwise false
	*/
	virtual void SetUseForDisplay(bool bUseProxy) = 0;
	
	//! \brief Retrieves whether the proxy is being used for viewport display.
	/*! \return bool - True if the proxy is displayed in the viewport, otherwise false
	*/
	virtual bool GetUseForDisplay() const = 0;
	
	//! \brief Allows to turn on or off the use of the proxy for rendering purposes
	/*! \param [in] bUseProxy - If true, the proxy is to be rendered instead of 
	the xrefed scene entity, otherwise false
	*/
	virtual void SetUseForRendering(bool bUseProxy) = 0;
	
	//! \brief Retrieves whether the proxy is being used for rendering purposes.
	/*! \return bool - True if the proxy is being rendered, otherwise false
	*/
	virtual bool GetUseForRendering() const = 0;

	//! \brief Allows to determine whethe a scene entity is implements this interface 
	/*! \param [in] is - Reference to scene entity
	\return - true if the scene entity implements the IXRefProxy interface, false otherwise
	*/
	static bool IsIXRefProxy(InterfaceServer& is) {
		return (is.GetInterface(IID_XREF_PROXY) != NULL);
	};

	using FPMixinInterface::GetInterface;
	//! \brief Retrieves the IXRefProxy interface from a scene entity.
	/*! \param [in] is - Reference to scene entity
	\return - Pointer to the IXRefProxy interface of the scene entity or NULL if it does not support the proxy feature
	*/
	static IXRefProxy* GetInterface(InterfaceServer& is) {
		return static_cast<IXRefProxy*>(is.GetInterface(IID_XREF_PROXY));
	};

	// --- Function publishing
	virtual Interface_ID GetID() { return IID_XREF_PROXY; }

	enum {
		kfpGetProxyFileName, kfpSetProxyFileName,
		kfpGetProxyFileAssetId, kfpSetProxyFileAssetId,
		kfpGetProxyItemName, kfpSetProxyItemName,
		kfpGetProxyRecord,
		kfpGetUseForDisplay,
		kfpSetUseForDisplay,
		kfpGetUseForRendering,
		kfpSetUseForRendering,
	};

	BEGIN_FUNCTION_MAP
		PROP_FNS(kfpGetProxyFileName, FPGetProxyFileName, kfpSetProxyFileName, FPSetProxyFileName, TYPE_FILENAME);
		PROP_FNS(kfpGetProxyItemName, GetProxyItemName, kfpSetProxyItemName, FPSetProxyItemName, TYPE_STRING);
		RO_PROP_FN(kfpGetProxyRecord, GetProxyRecord, TYPE_INTERFACE);
		PROP_FNS(kfpGetUseForDisplay, GetUseForDisplay, kfpSetUseForDisplay, SetUseForDisplay, TYPE_bool);
		PROP_FNS(kfpGetUseForRendering, GetUseForRendering, kfpSetUseForRendering, SetUseForRendering, TYPE_bool);
	END_FUNCTION_MAP

	// From FPMixinInterface
	virtual FPInterfaceDesc* GetDesc() { return &mFPInterfaceDesc; }
	static FPInterfaceDesc mFPInterfaceDesc;

protected:
	//! \name Function Publishing methods 
	//@{
	//! \brief Sets file name of xref proxy. Implemented by the system.
	virtual void FPSetProxyFileName(const MCHAR* fileName) = 0;
	//! \brief Gets file name of xref proxy. Implemented by the system.
	virtual const MCHAR* FPGetProxyFileName() const = 0;
	//! \brief Sets item name of xref proxy. Implemented by the system.
	virtual void FPSetProxyItemName(const MCHAR* itemName) = 0;
	//@}
};

