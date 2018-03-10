/**********************************************************************
 *<
	FILE:  CustAttrib.h

	DESCRIPTION:  Defines CustAttrib class

	CREATED BY: Nikolai Sander

	HISTORY: created 5/25/00

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "maxtypes.h"
#include "plugapi.h"
#include "ref.h"

class ICustAttribContainer;
class IGraphObjectManager;
class IMtlParams;
class ParamDlg;

/*! \sa  Class ICustAttribContainer,  Class ReferenceTarget,  Class ParamDlg, Class Animatable\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the Custom Attributes.\n\n
A sample on how to use this class is located in
<b>/MAXSDK/SAMPLES/HOWTO/CUSTATTRIBUTIL</b>  */

class CustAttrib: public ReferenceTarget
{
public:
	virtual SClass_ID SuperClassID() { return CUST_ATTRIB_CLASS_ID; }
	/*! \remarks A CustAttrib plugin can implement this method in order to provide the name
	that gets displayed in the TrackView.
	\par Default Implementation:
	<b>{ return "Custom Attribute";}</b> */
	virtual const MCHAR* GetName(){ return _M("Custom Attribute");}
	/*! \remarks This method gets called when the material or texture is to be displayed in
	the material editor parameters area. The plug-in should allocate a new
	instance of a class derived from ParamDlg to manage the user interface.
	\par Parameters:
	<b>HWND hwMtlEdit</b>\n\n
	The window handle of the materials editor.\n\n
	<b>IMtlParams *imp</b>\n\n
	The interface pointer for calling methods in 3ds Max.
	\return  A pointer to the created instance of a class derived from
	<b>ParamDlg</b>.
	\par Default Implementation:
	<b>{return NULL;}</b> */
	#pragma warning(push)
	#pragma warning(disable:4100)
	virtual ParamDlg *CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp){return NULL;}
	/*! \remarks This method will check if it possible to copy the current custom attributes
	to the specified custom attributes container.
	\par Parameters:
	<b>ICustAttribContainer *to</b>\n\n
	A pointer to the custom attributes container you wish to check for possible
	reception of the custom attributes..
	\return  TRUE if it is possible to copy, otherwise FALSE.
	\par Default Implementation:
	<b>{ return true; }</b> */
	
	virtual bool CheckCopyAttribTo(ICustAttribContainer *to) { return true; }
	virtual SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags)
	{
		return SvStdTraverseAnimGraph(gom, owner, id, flags); 
	}
	#pragma warning(pop)
};


/*! \brief Structure used with NOTIFY_CUSTOM_ATTRIBUTES_ADDED and NOTIFY_CUSTOM_ATTRIBUTES_REMOVED notifications
*/
struct NotifyCustomAttribute : public MaxHeapOperators 
{
	NotifyCustomAttribute(Animatable* owner = NULL, Animatable* custAttr = NULL): 
		m_owner(owner), m_customAttribute(custAttr) {};
		/*! \brief The owner of the custom attribute that was added or is about to be removed */
		Animatable*	m_owner; 
		/*! \brief The custom attribuet that was added or is about to be removed */
		Animatable*	m_customAttribute;
};

