 /**********************************************************************
 
	FILE:           INodeMentalRayProperties.h

	DESCRIPTION:    Public interface for setting and getting a node's
                    mental ray properties.

	CREATED BY:     Daniel Levesque, Discreet

	HISTORY:        created 13 December 2004

 *>	Copyright (c) 2004, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "ifnpub.h"
#include "inode.h"

// forward declarations
class INode;

//! The interface ID for class INodeMentalRayProperties
#define NODEMENTALRAYPROPERTIES_INTERFACE Interface_ID(0x1c763921, 0x4d542752)

//==============================================================================
// Class INodeMentalRayProperties
//
//! Extension to INode for accessing mental ray properties.
//!
//! This interface class defines a set of methods for accessing mental ray properties
//! on an INode. To get a pointer to this interface from an INode, call:
//!
//!			INodeMentalRayProperties* mr_properties = Get_INodeMentalRayProperties(node);
//!
//! The properties exposed by this class to the SDK are also exposed to MaxScript
//! through the function publishing system.
//!
class INodeMentalRayProperties : public FPMixinInterface {

public:

	enum DisplacementMethod {
		kDisplacementMethod_Fine = 0,
		kDisplacementMethod_Fine_NoSmoothing = 1
	};

	enum CastModeFG {
		kCastModeFG_ObjColor = 0,
		kCastModeFG_Black = 1,
		kCastModeFG_Invisible =2
	};

	//! Use this function to retrieve a pointer to the INodeMentalRayProperties interface.
	//! \param[in] node - The node on which you want to query the interface.
	//! \return A pointer to the interface. May be NULL if the interface is not supported by this release.
	static INodeMentalRayProperties* Get_INodeMentalRayProperties(INode& node);

	//! Copy properties from another interface
	//! \param[in] source - The properties to be copied.
	virtual void CopyMentalRayPropertiesFrom(const INodeMentalRayProperties& source) = 0;

	//! Gets the value of the "Use global displacement settings" flag.
	//! \return The value of the flag.
	virtual bool MRGetDisplacementUseGlobalSettings() const = 0;

	//! Sets the value fo the "Use global displacement settings" flag.
	//! \param[in] useGlobal - The value to be set.
	virtual void MRSetDisplacementUseGlobalSettings(bool useGlobal) = 0;

	//! Gets the displacement method to be used.
	//! \return The displacement method to be used.
	virtual DisplacementMethod MRGetDisplacementMethod() const = 0;  

	//! Sets the displacement method to be used.
	//! \param[in] method - The displacement method.
	virtual void MRSetDisplacementMethod(DisplacementMethod method) = 0;

	//! Gets the value of the "View-Dependent Displacement" flag.
	//! \return The value of the flag.
	virtual bool MRGetIsViewDependentDisplacement() const = 0;

	//! Sets the value of the "View-Dependent Displacement" flag.
	//! \param[in] viewDependent - The value to be set.
	virtual void MRSetIsViewDependentDisplacement(bool viewDependent) = 0;

	//! Gets the value of the "Edge Length" property.
	//! Note: this value should be interpreted as pixels if the "view-dependent" flag is ON,
	//! and as object-space units if the "view-dependent" flag is OFF.
	//! \return The value of the "Edge Length" parameter.
	virtual float MRGetDisplacementEdgeLength() const = 0;

	//! Sets the value of the "Edge Length" property.
	//! Note: this value should be interpreted as pixels if the "view-dependent" flag is ON,
	//! and as object-space units if the "view-dependent" flag is OFF.
	//! \param[in] length - The value to be set.
	virtual void MRSetDisplacementEdgeLength(float length) = 0;

	//! Gets the value of the "Max. Displace" property.
	//! \return The value of the "Max. Displace" property.
	virtual float MRGetMaxDisplace() const = 0;

	//! Sets the value of the "Max. Displace" property.
	//! \param[in] maxDisplace - The value to be set.
	virtual void MRSetMaxDisplace(float maxDisplace) = 0;

	//! Gets the value of the "Displacement Max. Level" property.
	//! \return The value of the "Displacement Max. Level" property.
	virtual unsigned short MRGetDisplacementMaxLevel() const = 0;

	//! Sets the value of the "Displacement Max. Level" property.
	//! \param[in] maxLevel - The value to bet set.
	virtual void MRSetDisplacementMaxLevel(unsigned short maxLevel) = 0;

	//! Gets the value of the "Receive Illumination from Final Gather" flag.
	//! \return The value of the flag.
	virtual bool MRGetReceiveFGIllum() const = 0;

	//! Sets the value of the "Receive Illumination from Final Gather" flag.
	//! \param[in] onOff - The value to be set.
	virtual void MRSetReceiveFGIllum(bool onOff) = 0;

	//! Gets the mode for the "Cast Illumination from Final Gather" to be used.
	//! \return The mode to be used for FG Rays that hit the object.
	virtual CastModeFG MRGetCastModeFGIllum() const = 0;  

	//! Sets the mode for the "Cast Illumination from Final Gather" to be used.
	//! \param[in] method - The mode to be used for FG Rays that hit the object.
	virtual void MRSetCastModeFGIllum(CastModeFG method) = 0;

	//! Gets the value of the "Exclude from Global Illumination Calculations" flag.
	//! \return The value of the flag.
	virtual bool MRGetExcludeGlobalIllum() const = 0;

	//! Sets the value of the "Exclude from Global Illumination Calculations" flag.
	//! \param[in] onOff - The value to be set.
	virtual void MRSetExcludeGlobalIllum(bool onOff) = 0;

	//! Gets the value of the "Exclude from Caustics Calculations" flag.
	//! \return The value of the flag.
	virtual bool MRGetExcludeCaustics() const = 0;

	//! Sets the value of the "Exclude from Caustics Calculations" flag.
	//! \param[in] onOff - The value to be set.
	virtual void MRSetExcludeCaustics(bool onOff) = 0;

private:

	// This version of the method is needed by the function publishing macros.
	void MRSetDisplacementMethod(int method);
	void MRSetCastModeFGIllum(int mode);

public:

	// Function publishing interface	
	enum PROPERTY_ACCESSOR_ID {
		GET_DISPLACEMENTUSEGLOBALSETTINGS,
		SET_DISPLACEMENTUSEGLOBALSETTINGS,
		GET_DISPLACEMENTMETHOD,
		SET_DISPLACEMENTMETHOD,
		GET_VIEWDEPENDENTDISPLACEMENT,
		SET_VIEWDEPENDENTDISPLACEMENT,
		GET_DISPLACEMENTEDGELENGTH,
		SET_DISPLACEMENTEDGELENGTH,
		GET_MAXDISPLACE,
		SET_MAXDISPLACE,
		GET_DISPLACEMENTMAXLEVEL,
		SET_DISPLACEMENTMAXLEVEL,
		GET_RECEIVEFGILLUM,
		SET_RECEIVEFGILLUM,
		GET_CASTMODEFGILLUM,
		SET_CASTMODEFGILLUM,
		GET_EXCLUDEGLOBALILLUM,
		SET_EXCLUDEGLOBALILLUM,
		GET_EXCLUDECAUSTICS,
		SET_EXCLUDECAUSTICS
	};

	#pragma warning(push)
	#pragma warning(disable:4244)
	BEGIN_FUNCTION_MAP
		// Property accessors
		PROP_FNS(GET_DISPLACEMENTUSEGLOBALSETTINGS, MRGetDisplacementUseGlobalSettings, SET_DISPLACEMENTUSEGLOBALSETTINGS, MRSetDisplacementUseGlobalSettings, TYPE_bool);
		PROP_FNS(GET_DISPLACEMENTMETHOD, MRGetDisplacementMethod, SET_DISPLACEMENTMETHOD, MRSetDisplacementMethod, TYPE_ENUM);
		PROP_FNS(GET_VIEWDEPENDENTDISPLACEMENT, MRGetIsViewDependentDisplacement, SET_VIEWDEPENDENTDISPLACEMENT, MRSetIsViewDependentDisplacement, TYPE_bool);
		PROP_FNS(GET_DISPLACEMENTEDGELENGTH, MRGetDisplacementEdgeLength, SET_DISPLACEMENTEDGELENGTH, MRSetDisplacementEdgeLength, TYPE_FLOAT);
		PROP_FNS(GET_MAXDISPLACE, MRGetMaxDisplace, SET_MAXDISPLACE, MRSetMaxDisplace, TYPE_WORLD);
		// The warning here is harmless. In the future MRSetDisplacementMaxLevel could take an int instead of an unsigned short.
		PROP_FNS(GET_DISPLACEMENTMAXLEVEL, MRGetDisplacementMaxLevel, SET_DISPLACEMENTMAXLEVEL, MRSetDisplacementMaxLevel, TYPE_INT);
		PROP_FNS(GET_RECEIVEFGILLUM, MRGetReceiveFGIllum, SET_RECEIVEFGILLUM, MRSetReceiveFGIllum, TYPE_bool);
		PROP_FNS(GET_CASTMODEFGILLUM, MRGetCastModeFGIllum, SET_CASTMODEFGILLUM, MRSetCastModeFGIllum, TYPE_ENUM);
		PROP_FNS(GET_EXCLUDEGLOBALILLUM, MRGetExcludeGlobalIllum, SET_EXCLUDEGLOBALILLUM, MRSetExcludeGlobalIllum, TYPE_bool);
		PROP_FNS(GET_EXCLUDECAUSTICS, MRGetExcludeCaustics, SET_EXCLUDECAUSTICS, MRSetExcludeCaustics, TYPE_bool);
	END_FUNCTION_MAP
	#pragma warning(pop)
};

inline void INodeMentalRayProperties::MRSetDisplacementMethod(int method) {

	return MRSetDisplacementMethod(static_cast<DisplacementMethod>(method));
}

inline void INodeMentalRayProperties::MRSetCastModeFGIllum(int mode) {
	MRSetCastModeFGIllum(static_cast<CastModeFG>(mode));
}

inline INodeMentalRayProperties* INodeMentalRayProperties::Get_INodeMentalRayProperties(INode& node) {

	return static_cast<INodeMentalRayProperties*>(node.GetInterface(NODEMENTALRAYPROPERTIES_INTERFACE));
}

