/**********************************************************************
 *<
	FILE:  ICustAttribContainer.h

	DESCRIPTION:  Defines ICustAttribContainer class

	CREATED BY: Nikolai Sander

	HISTORY: created 5/22/00

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "ref.h"
// forward declarations
class CustAttrib;
class ParamDlg;
class IMtlParams;

/*! \sa  Class CustAttrib , Class RemapDir, Class ReferenceTarget\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the interface class to a custom attributes container.
 */
class ICustAttribContainer : public ReferenceTarget
{
public:
	/*! \remarks This method returns the number of custom attributes. */
	virtual int GetNumCustAttribs()=0;
	/*! \remarks This method allows you to retrieve the custom attribute by
	its specified index.
	\par Parameters:
	<b>int i</b>\n\n
	The index of the custom attribute you with to obtain. */
	virtual CustAttrib *GetCustAttrib(int i)=0;
	/*! \remarks This method allows you to append a custom attribute.
	\par Parameters:
	<b>CustAttrib *attribute</b>\n\n
	A pointer to the custom attribute you wish to add. */
	virtual void AppendCustAttrib(CustAttrib *attribute)=0;
	/*! \remarks This method allows you to set the custom attribute at the
	specified index.
	\par Parameters:
	<b>int i</b>\n\n
	The index for which to set the custom attribute.\n\n
	<b>CustAttrib *attribute</b>\n\n
	A pointer to the custom attribute you wish to set. */
	virtual void SetCustAttrib(int i, CustAttrib *attribute)=0;
	/*! \remarks This method allows you to insert a custom attribute at the
	specified index.
	\par Parameters:
	<b>int i</b>\n\n
	The index at which to insert the custom attribute.\n\n
	<b>CustAttrib *attribute</b>\n\n
	A pointer to the custom attribute you wish to insert.\n\n

	\return    */
	virtual void InsertCustAttrib(int i, CustAttrib *attribute)=0;
	/*! \remarks This method allows you to remove a custom attribute.
	\par Parameters:
	<b>int i</b>\n\n
	The index of the custom attribute to remove. */
	virtual void RemoveCustAttrib(int i)=0;
	/*! \remarks This method gets called when the material or texture is to be
	displayed in the material editor parameters area. The plug-in should
	allocate a new instance of a class derived from ParamDlg to manage the user
	interface.
	\par Parameters:
	<b>HWND hwMtlEdit</b>\n\n
	The window handle of the materials editor.\n\n
	<b>IMtlParams *imp</b>\n\n
	The interface pointer for calling methods in 3ds Max.
	\return  A pointer to the created instance of a class derived from
	<b>ParamDlg</b>. */
	virtual ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams* imp)=0;
	/*! \remarks This method will copy the parameters from a specified
	reference maker.
	\par Parameters:
	<b>ReferenceMaker *from</b>\n\n
	A pointer to the reference maker to copy the parameters from.\n\n
	<b>RemapDir \&remap</b>\n\n
	This class is used for remapping references during a Clone. See
	Class RemapDir. */
		virtual void CopyParametersFrom(ReferenceMaker *from, RemapDir &remap)=0;
	/*! \remarks This method returns a pointer to the owner of the custom
	attributes. */
	virtual Animatable *GetOwner()=0;
	/*! \remarks Self deletion. */
	virtual void DeleteThis()=0;
	/*! \brief Finds the first custom attribute in a custom attribute container, 
	that has a specific interface id.
	\param iid The interface id to be search for
	\return Returns a BaseInterface* to the found custom attribute, or NULL if there is 
	no custom attributes with the given interface id exist in the custom attribute container.
	*/
	virtual BaseInterface* FindCustAttribInterface(const Interface_ID& iid) = 0;
};

