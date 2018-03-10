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
// DESCRIPTION: Interface to xref Atmospherics
// AUTHOR: Attila Szabo - created Feb.03.2005
//***************************************************************************/

#pragma once

#include "..\ifnpub.h"
#include "..\sfx.h"


// Forward declarations
class Animatable;

//! Id for IXRefAtmospheric interface
#define IID_XREF_ATMOS Interface_ID(0x2672319a, 0x657b4bc1)

//! \brief Class representing xref atmospherics
/*! This interface groups functionality specific to xref atmospherics. In order to
access functionality common to all xref scene entities, use the IXRefItem interface.
An xref atmospheric acts as a wrapper around the xrefed atmospheric (also referred to as
the "source atmospheric"). For example, requests for getting the atmospheric's gizmos 
(NumGizmos, GetGizmo) are passed to the source atmospheric, as are requests for interfaces. 
Client or plugin code working with atmospherics should not be required to write xref 
atmospheric specific code, unless the desired functionality is xref specific.
\note Note that xref atmospherics can only have gizmos that are xrefs (xref objects) themselves.
Xref atmospherics will be automatically removed from the scene when all their
gizmos have been removed.
\note Xref Atmospherics are never nested, even when their gizmos are, and they are 
always owned by non-nested (top-level) xref records.
*/
class IXRefAtmospheric : public Atmospheric, public FPMixinInterface
{
public:
	//! \brief Returns the class id of the object type this class represents
	Class_ID ClassID() { return XREFATMOS_CLASS_ID; }		
	//! \brief Returns the super class id the object type this class represents
	SClass_ID SuperClassID() { return ATMOSPHERIC_CLASS_ID; }
	
	//! \brief Retrieves the source atmospheric, i.e., the actual atmospheric behind the XRef.
	/*! \param [in] resolveNestedXRef - When set to true, the method will recursively resolve
	any nested XRef atmospherics, guaranteeing that the one being returned is NOT an XRef atmospheric.
	When set to false, the source is returned directly without attempting to resolve nesting.
	\return The source atmospheric, or NULL if the XRef is unresolved.
	*/
	virtual Atmospheric* GetSourceAtmospheric(bool resolveNestedXRef = false) = 0;

	//! \brief Determines whether it is safe to cast an Animatable to an IXRefAtmospheric.
	/*! \param[in] animatable - The animatable to test.
	\return true if the specified animatable can be safely cast to IXRefAtmospheric, false otherwise.
	*/
	static bool Is_IXRefAtmospheric(Animatable& animatable);

	using FPMixinInterface::GetInterface;
	//! \brief Retrieves the IXRefAtmospheric interface from a scene entity 
	/*! \param [in] is - Reference to scene entity
	\return - Pointer to the IXRefAtmospheric interface of the scene entity 
	or NULL if it's not an xref atmospheric
	*/
	static IXRefAtmospheric* GetInterface(InterfaceServer& is) {
		return static_cast<IXRefAtmospheric*>(is.GetInterface(IID_XREF_ATMOS));
	};

	// From FPMixinInterface
	virtual Interface_ID GetID() { return IID_XREF_ATMOS; }
	virtual FPInterfaceDesc* GetDesc() { return &mFPInterfaceDesc; }
	static FPInterfaceDesc mFPInterfaceDesc;

	// --- Function publishing
	enum {
		kfpGetSourceAtm,
	};

	BEGIN_FUNCTION_MAP
		FN_1(kfpGetSourceAtm, TYPE_REFTARG, GetSourceAtmospheric, TYPE_bool);
	END_FUNCTION_MAP
};

inline bool IXRefAtmospheric::Is_IXRefAtmospheric(Animatable& animatable) {

	return ((animatable.SuperClassID() == ATMOSPHERIC_CLASS_ID) &&
					(animatable.ClassID() == XREFATMOS_CLASS_ID));
}

