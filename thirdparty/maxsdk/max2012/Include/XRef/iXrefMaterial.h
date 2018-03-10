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
// DESCRIPTION: Interface to xref materials
// AUTHOR: Daniel Levesque - created Feb.24.2005
//***************************************************************************/

#pragma once

#include "..\imtl.h"

//! Id for IXRefMaterial interface
#define IID_XREF_MATERIAL Interface_ID(0x2edc624a, 0x3de347b)

//==============================================================================
// Class IXRefMaterial
//
//! \brief Class representing xref materials
/*! This interface groups functionality specific to xref materials. In order to
access functionality common to all xref scene entities, use the IXRefItem interface.
An xref material acts as a wrapper around the xrefed material (also referred to as
the "source material"). For example, all shading methods on the XRef Material 
are automatically forwarded to the source material, requests for interfaces
are also forwarded to the source material. Client or plugin code working with 
materials should not be required to write xref material specific code, unless the
desired functionality is xref specific.
*/
class IXRefMaterial : public Mtl, public FPMixinInterface {
public:

	//! \brief Retrieves the source material, the actual material behind the XRef.
	/*! \param [in] resolveNestedXRef - When set to true, the method will recursively resolve
	any nested XRef materials, guaranteeing that the material being returned is NOT an XRef material.
	When set to false, the source is returned directly without attempting to resolve nesting.
	\return The source material, or NULL if the XRef is unresoled.
	*/
	virtual Mtl* GetSourceMaterial(bool resolveNestedXRef = false) const = 0;

	//! \brief Determines whether it is safe to cast an Animatable to an IXRefMaterial.
	/*! \param[in] animatable - The animatable to test.
	\return true if the specified animatable can be safely cast to IXRefMaterial, false otherwise.
	*/
	static bool Is_IXRefMaterial(Animatable& animatable);

	using FPMixinInterface::GetInterface;
	//! \brief Retrieves the IXRefMaterial interface from a scene entity 
	/*! \param [in] is - Reference to scene entity
	\return Pointer to the IXRefMaterial interface of the scene entity or NULL if it's not an xref material
	*/
	static IXRefMaterial* GetInterface(InterfaceServer& is);

	//! \brief Returns the function publishing descriptor for the XRef Material.
	/*! This method is need in order to avoid problems inherant to multiple inheritance.
	\return The function publishing descriptor for the XRef Material.
	*/
	virtual FPInterfaceDesc* GetIXRefMaterialDesc() = 0;

	// -- from FPMixinInterface
	virtual Interface_ID GetID();
	virtual FPInterfaceDesc* GetDesc();

	// --- Function publishing
	enum {
		kfpGetSourceMaterial,
	};

	BEGIN_FUNCTION_MAP
		FN_1(kfpGetSourceMaterial, TYPE_MTL, GetSourceMaterial, TYPE_bool);
	END_FUNCTION_MAP
};

//==============================================================================
// Class IXRefMaterial inlined methods
//==============================================================================

inline bool IXRefMaterial::Is_IXRefMaterial(Animatable& animatable) {

	return ((animatable.SuperClassID() == MATERIAL_CLASS_ID) 
		&& (animatable.ClassID() == XREFMATERIAL_CLASS_ID));
}

inline IXRefMaterial* IXRefMaterial::GetInterface(InterfaceServer& is) {

	return static_cast<IXRefMaterial*>(is.GetInterface(IID_XREF_MATERIAL));
};

inline Interface_ID IXRefMaterial::GetID() { 
	
	return IID_XREF_MATERIAL; 
}

inline FPInterfaceDesc* IXRefMaterial::GetDesc() {

	return GetIXRefMaterialDesc();
}

