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
// DESCRIPTION: Interface to xref objects
// AUTHOR: Attila Szabo - created Feb.09.2005
//***************************************************************************/

#pragma once

#include "..\object.h"
#include "..\assetmanagement\AssetUser.h"

// Forward declarations
class IObjXRefRecord;
class Animatable;

//! \brief Legacy interface for xref objects
/* Although this interface is still supported, it is recommended that IXRefObject8 
and IXRefItem is used instead.
*/
/*! <b>Note:</b>In 3ds  Max 8 this class was replaced with
class IXrefObject8. It is still documented for backwards compatibility.
\sa Class Object,  Class INode.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class is an interface to the parameters of an XRef object. Object XRefs
are derived from this class.\n\n
If you have an object pointer you can check its super class ID and class ID to
see if it is an XRef object. Then you can cast it to an instance of this class
and call these methods. For example:
\code
INode *node = ip->GetSelNode(0);
Object *obj = node->GetObjectRef();

if (obj->SuperClassID()==SYSTEM_CLASS_ID && obj->ClassID()==XREFOBJ_CLASS_ID)
{
   IXRefObject *ix = (IXRefObject *)obj;
   ix->BrowseFile(FALSE);
}
\endcode
However you may have an object that depends on an XRef object (for example a Boolean
with an XRef object as one of it's operands) or you may have a pointer to the
object that the XRef object references. In such cases the only way to know for
sure is to look up and down the pipeline for XRef objects.  */
class IXRefObject : public Object 
{
	public:
		Class_ID ClassID() {return XREFOBJ_CLASS_ID;}
		SClass_ID SuperClassID() {return SYSTEM_CLASS_ID;}

		// Initialize a new XRef object
		/*! \remarks  This method initializes a newly created XRef object. The
		caller provides the name of the file (fname), the name of the object in
		the file (<b>oname</b>), and a pointer to the object being XRefed
		(<b>ob</b>).
		\par Parameters:
		<b>MSTR \&fname</b>\n\n
		The file name is set to this string.\n\n
		<b>MSTR \&oname</b>\n\n
		The object name is set to this string.\n\n
		<b>Object *ob</b>\n\n
		Points to the object being XRefed.\n\n
		<b>BOOL asProxy=FALSE</b>\n\n
		If TRUE then the above information is considered to specify the proxy
		portion of the XRef. */
		virtual void Init(const MaxSDK::AssetManagement::AssetUser &asset, const MSTR &oname, Object *ob, BOOL asProxy=FALSE)=0;

		/*! \remarks  Sets the Source or Proxy file depending on the value passed.
		\par Parameters:
		<b>AssetUser &asset</b>\n\n
		The asset to set.\n\n
		<b>BOOL proxy=FALSE</b>\n\n
		TRUE to set the Proxy file name; FALSE for the XRef file name.\n\n
		<b>BOOL update=TRUE</b>\n\n
		TRUE to update the scene; FALSE to not update immediately. */
		virtual void SetFile(const MaxSDK::AssetManagement::AssetUser &asset, BOOL proxy=FALSE, BOOL update=TRUE)=0;
		/*! \remarks  Sets the Object name or the Proxy name depending on the
		value passed.
		\par Parameters:
		<b>MCHAR *name</b>\n\n
		The name to set.\n\n
		<b>BOOL proxy=FALSE</b>\n\n
		TRUE to set the Proxy name; FALSE to set the Object name. */
		virtual void SetObjName(MCHAR *name, BOOL proxy=FALSE)=0;
		/*! \remarks  Sets the state of the Use Proxy choice and optionally
		redraws the viewports.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off.\n\n
		<b>BOOL redraw=TRUE</b>\n\n
		TRUE to redraw the viewports; otherwise FALSE. */
		virtual void SetUseProxy(BOOL onOff,BOOL redraw=TRUE)=0;
		/*! \remarks  Sets the state of the Render Proxy option.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetRenderProxy(BOOL onOff)=0;
		/*! \remarks  Sets the state of the Update Materials option.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetUpdateMats(BOOL onOff)=0;
		/*! \remarks  Sets the state of the Ignore Animation choice and
		optionally redraws the viewports.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off.\n\n
		<b>BOOL redraw=TRUE</b>\n\n
		TRUE to redraw the viewports; otherwise FALSE. */
		virtual void SetIgnoreAnim(BOOL onOff,BOOL redraw=TRUE)=0;
		
		/*! \remarks  Returns the File asset or the Proxy file asset depending on
		the value passed.
		\par Parameters:
		<b>BOOL proxy=FALSE</b>\n\n
		TRUE to return the Proxy file asset; FALSE for the XRef file asset. */
		virtual const MaxSDK::AssetManagement::AssetUser& GetFile(BOOL proxy=FALSE) const =0;
		/*! \remarks  Returns the Object name or the Proxy name depending on
		the value passed.
		\par Parameters:
		<b>BOOL proxy=FALSE</b>\n\n
		TRUE to return the Proxy name; FALSE to return the Object name. */
		virtual MSTR GetObjName(BOOL proxy=FALSE) const =0;
		/*! \remarks  Returns the XRef File. */
		virtual const MaxSDK::AssetManagement::AssetUser& GetCurFile() const =0;
		/*! \remarks  Sets the XRef File. */
		virtual void SetCurFile(const MaxSDK::AssetManagement::AssetUser&)=0;
		/*! \remarks  Returns the XRef Object Name. */
		virtual const MSTR &GetCurObjName() const =0;
		/*! \remarks  Sets the XRef Object Name. */
		virtual void SetCurObjName(const MCHAR* name)=0;
		/*! \remarks  Returns TRUE if the Use Proxy option is on; otherwise
		FALSE. */
		virtual BOOL GetUseProxy() =0;
		/*! \remarks  Returns TRUE if the Render Proxy option is on; otherwise
		FALSE. */
		virtual BOOL GetRenderProxy() =0;
		/*! \remarks  Returns TRUE if the Update Material option is on;
		otherwise FALSE. */
		virtual BOOL GetUpdateMats() =0;
		/*! \remarks  Returns TRUE if the Ignore Animation option is on;
		otherwise FALSE. */
		virtual BOOL GetIgnoreAnim() =0;		
		
		// Causes browse dialogs to appear
		/*! \remarks  This method brings up the browse object (Merge) dialog.
		\par Parameters:
		<b>BOOL proxy</b>\n\n
		TRUE to have Use Proxy mode active; FALSE to have it inactive. */
		virtual void BrowseObject(BOOL proxy)=0;
		/*! \remarks  This method brings up the browse file (Open File) dialog.
		\par Parameters:
		<b>BOOL proxy</b>\n\n
		TRUE to have Use Proxy mode active; FALSE to have it inactive. */
		virtual void BrowseFile(BOOL proxy)=0;

		// Try to reload ref
		/*! \remarks  Reloads this Object XRef. */
		virtual void ReloadXRef()=0;
};


//! Id for IXRefObject8 interface
#define IID_XREF_OBJECT8 Interface_ID(0x470145ad, 0x3daa64e1)

//! \brief Class representing Xref objects
/*! This interface groups functionality specific to xref objects. In order to
access functionality common to all xref scene entities, use the IXRefItem interface.
An xref object acts as a wrapper around the xrefed object (also referred to as
the "source object"). For example, requests for evaluating the xref object (Eval) and 
finding its base object (FindBaseObject) are passed to the source object, as are
requests for interfaces. Client or plugin code working with objects should not be 
required to write xref object specific code, unless the desired functionality is xref specific.
*/
// Needs to derive from the IXrefObject in order to preserve sdk backward compatibility
class IXRefObject8 : public IXRefObject, public FPMixinInterface
{
public:
	//! \brief Retrieves the active source file  
	/*! The file currently used by the xref object could be the one where 
	the source object comes from or the one where the proxy object comes from.
	Which file is currently used depends on the proxy settings of the xref object
	\return - The file asset currently used by the xref object.
	\see IXRefProxy, IXRefItem
	*/
	virtual const MaxSDK::AssetManagement::AssetUser& GetActiveFile() const = 0;
	
	//! \brief Retrieves the active source item name
	/*! The object name currently used by the xref object could be the source or
	the proxy object name. Which object name currently used depends on the proxy 
	settings of the xref object.
	\return - Pointer to the name of the object currently xrefed or NULL if	the name is not specified.
	\see IXRefProxy, IXRefItem
	*/
	virtual const MCHAR* GetActiveObjName() const = 0;

	//! \brief Retrieves the active xref record this xref object belongs to.
	/*! When the proxy object assigned to this xref object is active, this method
	returns the xref record that owns the xref proxy. Otherwise it returns the xref
	record that owns the original xref item.
	\return The active xref record this xref object belongs to. A return value of NULL, 
	indicates that the xref object is not part of an xref record yet.
	*/
	virtual IObjXRefRecord* GetActiveRecord() const = 0;

	//! \brief Retrieves the actual source object behind the XRef.
	/*! \param [in] resolveNested - When set to true, the method will recursively resolve
	any nested xref objects, guaranteeing that the one being returned is NOT an xref object.
	When set to false, the source object is returned without attempting to resolve nesting.
	\param [in, out] modifiers - a pointer to a Tab<Modifier*> or NULL. If not NULL, 
	the modifiers in the source object will be placed in the tab.
	\return The source object, or NULL if the xref is unresolved. 
	*/
	virtual Object* GetSourceObject(bool resolveNested = false, Tab<Modifier*> *modifiers = NULL) const = 0;

	//! \brief Allows to determine whethe a scene entity implements this interface.
	/*! \param [in] animatable - Reference to scene entity
	\return - true if the scene entity implements the IXRefObject8 interface,	false otherwise
	*/
	static bool Is_IXRefObject8(Animatable& animatable);

	using FPMixinInterface::GetInterface;
	//! \brief Retrieves the IXRefObject8 interface from a scene entity.
	/*! \param [in] is - Reference to scene entity
	\return - Pointer to the IXRefObject8 interface of the scene entity or NULL	if it's not an xref object
	*/
	static IXRefObject8* GetInterface(InterfaceServer& is) {
		return static_cast<IXRefObject8*>(is.GetInterface(IID_XREF_OBJECT8));
	};

	// Function publishing - todo
	virtual Interface_ID GetID() { return IID_XREF_OBJECT8; }

	enum {
		kfpGetActiveFileName,
		kfpGetActiveObjName,
		kfpGetActiveRecord,
		kfpGetSourceObject,
	};

	BEGIN_FUNCTION_MAP
		RO_PROP_FN(kfpGetActiveFileName, FPGetActiveFileName, TYPE_FILENAME);
		RO_PROP_FN(kfpGetActiveObjName, GetActiveObjName, TYPE_STRING);
		RO_PROP_FN(kfpGetActiveRecord, GetActiveRecord, TYPE_INTERFACE);
		FN_2(kfpGetSourceObject, TYPE_REFTARG, FPGetSourceObject, TYPE_bool, TYPE_REFTARG_TAB_BR);
	END_FUNCTION_MAP

	// From FPMixinInterface
	virtual FPInterfaceDesc* GetDesc() { return &mFPInterfaceDesc; }
	static FPInterfaceDesc mFPInterfaceDesc;

	private:
		virtual const MCHAR* FPGetActiveFileName() const = 0;
		Object* FPGetSourceObject(bool resolveNestedXRef, Tab<ReferenceTarget*> &modifiers);
	protected:
		// Deprecated methods
		virtual void Init(const MaxSDK::AssetManagement::AssetUser &asset, const MSTR &oname, Object *ob, BOOL asProxy=FALSE)=0;
		virtual void SetFile(const MaxSDK::AssetManagement::AssetUser& asset, BOOL proxy=FALSE, BOOL update=TRUE)=0;
		virtual void SetObjName(MCHAR *name, BOOL proxy=FALSE)=0;
		virtual const MaxSDK::AssetManagement::AssetUser& GetFile(BOOL proxy=FALSE) const =0;
		virtual MSTR GetObjName(BOOL proxy=FALSE) const =0;
		// Exposed by class IXrefProxy
		virtual void SetUseProxy(BOOL onOff,BOOL redraw=TRUE)=0;
		virtual void SetRenderProxy(BOOL onOff)=0;
		virtual BOOL GetUseProxy() =0;
		virtual BOOL GetRenderProxy() =0;
		// Replaced by IXRefObject8::Get\SetCurFileName
		virtual const MaxSDK::AssetManagement::AssetUser& GetCurFile() const =0;
		virtual const MSTR &GetCurObjName() const =0;
};

inline bool IXRefObject8::Is_IXRefObject8(Animatable& animatable) {
	return ((animatable.SuperClassID() == SYSTEM_CLASS_ID) 
					&& (animatable.ClassID() == XREFOBJ_CLASS_ID));
}

