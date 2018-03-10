	 /**********************************************************************
 
	FILE: ILuminaire.h

	DESCRIPTION:  Public interface of a Luminaire (lighting fixture)

	CREATED BY: Attila Szabo, Discreet

	HISTORY: - created Aug 12, 2001

 *>	Copyright (c) 1998-2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "iFnPub.h"
#include "maxtypes.h"
#include "interval.h"


//  This is an interface class that defines the operations and properties
//  of a luminaire. A luminaire is a lighting fixture. A luminaire is made
//  up by at least one light object and several other objects (geometry, etc)
//
//  This interface can be implemented by any plugin object, whether it is a 
//  helper, geometry, etc. 
//  Luminaire plugin objects can be used, for example,
//  as luminaire assembly head objects. In this case, the object's parameters 
//  defined in its param block can be parameter wired to the the parameters of
//  the light objects in the luminaire assembly. In this case, the methods of 
//  this interface should read and write the corresponding paramblock parameters 
//  of the object plugin. 
#pragma warning(push)
#pragma warning(disable:4239)
class ILuminaire : public FPMixinInterface 
{
	public:
    // Dimming a luminaire means changing the intensity of all of its light 
    // sources at once with the same percentage.
    // A dimmer value of zero means effectively turning off all light sources 
    // in the luminiare; a dimmer value of 100 means that the light sources 
    // shine at their ("factory") default intensity, while a dimmer value 
    // greater than 100 means that their intensity is amplified\multiplied above
    // the ("factory") default value.
    // In the case of standard Max light sources, "factory" default intensity 
    // means a certain arbitrary multiplier value that is chosen by the user.
    // In case of physically based light sources, "factory" default intensity 
    // means the intensity given in the light's photometric file.
		virtual void		SetDimmer(float value, TimeValue time) = 0;
		virtual float		GetDimmer(TimeValue time, Interval& valid = FOREVER) const = 0 ;

		// These methods allow for specifing and retrieving at once (simultaneously) 
    // the filter color of all light sources of a luminaire
		virtual void		SetRGBFilterColor(Point3& value, TimeValue& time) = 0;
		virtual Point3	GetRGBFilterColor(TimeValue& time, Interval& valid = FOREVER) const = 0;

		// These methods allow for specifing and retrieving at once (simultaneously) 
    // the on\off state of all light sources of a luminaire
		virtual void		SetUseState(bool onOff, TimeValue& time) = 0;
		virtual bool		GetUseState(TimeValue& time, Interval& valid = FOREVER) const = 0;

		// Function Publishing Methods IDs
		enum 
		{ 
			kLUM_SET_DIMMER, 
			kLUM_GET_DIMMER, 
			kLUM_SET_RGB_FILTER_COLOR, 
			kLUM_GET_RGB_FILTER_COLOR, 
			kLUM_SET_USE_STATE, 
			kLUM_GET_USE_STATE, 
		}; 
	
}; 

#pragma warning(pop)
// Luminaire interface ID
#define LUMINAIRE_INTERFACE Interface_ID(0x7e631fe1, 0x7163389b)

inline ILuminaire* GetLuminaireInterface(BaseInterface* baseIfc)	
{ 
	DbgAssert( baseIfc != NULL);
	return static_cast<ILuminaire*>(baseIfc->GetInterface(LUMINAIRE_INTERFACE));
}


