/*****************************************************************************

	FILE: iXrefCtrl.h

	DESCRIPTION: Public interface for the XRef controller.

	CREATED BY: Richard Laperriere

	HISTORY: April 5th, 2005	Creation

	Copyright (c) 2005, All Rights Reserved.

*****************************************************************************/

#pragma once

#include "..\ifnpub.h"
#include "..\control.h"

//! Id for IXRefCtrl interface
#define IID_XREF_CTRL Interface_ID(0x501064f, 0x4026738f)

//==============================================================================
// Class IXRefCtrl
//
//! \brief Interface implemented by the XRef controller.
/*! The XRef controller is a wrapper that allows the user to retrieve a pointer to the "source controller".
*/
class IXRefCtrl : public Control, public FPMixinInterface
{
public:

	//! \brief Determines whether it is safe to cast an Animatable to an IXRefCtrl.
	/*! \param[in] animatable - The animatable to test.
	\return true if the specified animatable can be safely cast to IXRefCtrl, false otherwise.
	*/
	static bool Is_IXRefCtrl(Animatable& animatable);

	using FPMixinInterface::GetInterface;
	//! \brief Retrieves the IXRefCtrl interface from a scene entity 
	/*! \param is - Reference to scene entity
	\return - Pointer to the IXRefCtrl interface of the scene entity or NULL if it's not an xref controller
	*/
	static IXRefCtrl* GetInterface(InterfaceServer& is);

	//! \brief Retrieves the source controller, i.e., the actual controller behind the XRef.
	/*! \param resolveNestedXRef - When set to true, the method will recursively resolve
	any nested XRef controllers, guaranteeing that the one being returned is NOT an XRef controller.
	When set to false, the source is returned directly without attempting to resolve nesting.
	\return The source controller, or NULL if the XRef is unresolved.
	*/
	virtual Control* GetSourceCtrl(bool resolveNestedXRef = false) const = 0;

	//! \brief Returns the function publishing descriptor for the XRef Controller.
	/*! This method is needed in order to avoid multiple inheritance problems.
	\return The function publishing descriptor for the XRef Controller.
	*/
	virtual FPInterfaceDesc* GetIXRefCtrlDesc() = 0;

	//! \brief If the XRefCtrl is not nested, set its offset matrix back to identity, and if it is resolved,
	//! i.e., it has a Source controller, set this one's transformation matrix accordingly.
	virtual void ResetOffsetTM() = 0;

	// -- from FPMixinInterface
	virtual Interface_ID GetID()			{return IID_XREF_CTRL;}
	virtual FPInterfaceDesc* GetDesc()		{return GetIXRefCtrlDesc();}

	// --- Function publishing
	enum {
		kfpGetSourceCtrl,
		kfpResetOffsetTM,
	};

	BEGIN_FUNCTION_MAP
		FN_1(kfpGetSourceCtrl, TYPE_CONTROL, GetSourceCtrl, TYPE_bool);
		VFN_0(kfpResetOffsetTM, ResetOffsetTM);
	END_FUNCTION_MAP
};

//==============================================================================
// Class IXRefCtrl inlined methods
//==============================================================================

inline bool IXRefCtrl::Is_IXRefCtrl(Animatable& animatable)
{
	return ((animatable.SuperClassID() == CTRL_MATRIX3_CLASS_ID) &&
			(animatable.ClassID() == XREFCTRL_CLASS_ID));
}

inline IXRefCtrl* IXRefCtrl::GetInterface(InterfaceServer& is)
{
	return static_cast<IXRefCtrl*>(is.GetInterface(IID_XREF_CTRL));
}

