/**********************************************************************
 
	FILE: naturalLight.h

	DESCRIPTION:  Natural Light definitions

	CREATED BY: Cleve Ard, Discreet

	HISTORY: - created July 24, 2001

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "ifnpub.h"
#include "GetCOREInterface.h"
#include "Animatable.h"
#include "plugapi.h"

// Natural light is supplied by the interaction of several objects:

// SUN LIGHTS:

// Direct light
// IES sunlight
// CIE sunlight

// SKY LIGHTS:

// Standard Skylight
// IES skylight
// CIE skylight
// Isotropic skylight (why)
// Environmemnt map skylight
// HDR environment map skylight

// SUN POSITION CONTROLLERS

// MAX Sun position
// IES Sun position
// CIE Sun position

// NATURAL LIGHT SYSTEM

// The natural light system combines these three objects to form natural
// light. It uses a 

/*===========================================================================*\
 | Natural Light Class Interface:
\*===========================================================================*/

// This interface is used to tell the natural light assembly that the
// objects created by the class are natural light objects. This interface
// is used to decide when the classes are to be displayed in the
// drop down lists for the natural light assembly.

#define NATURAL_LIGHT_CLASS_INTERFACE_ID Interface_ID(0x75985ea5, 0x115c2791)

// A sun light is just a direct light. The intensities and
/*!  \n\n
class INaturalLightClass: public BaseInterface\n\n

\par Description:
<b>This class is only available in release 5 or later.</b>\n\n
INaturalLightClass is a static interface that is attached to the class
descriptors for light classes that wish to advertise that they represent either
skylight or sunlight on the surface of the earth.\n\n
It is used by the Daylight System, to populate it's UI with the classes that
describe themselves as naturalLight. This is an example of the way it is
used.\n\n
<b>void addClassToCombo(ClassDesc* pCd, HWND sunCombo, HWND skyCombo)</b>\n\n
<b>{</b>\n\n
<b>INaturalLightClass* pNC;</b>\n\n
<b>pNC = GetNaturalLightClassInterface(pCd);</b>\n\n
<b>if (pNC != NULL) {</b>\n\n
<b>if (pNC-\>IsSun()) {</b>\n\n
<b>addLightToCombo(sunCombo, pCd-\>ClassName(), pCd);</b>\n\n
<b>}</b>\n\n
<b>if (pNC-\>IsSky()) {</b>\n\n
<b>addLightToCombo(skyCombo, pCd-\>ClassName(), pCd);</b>\n\n
<b>}</b>\n\n
<b>}</b>\n\n
<b>}</b>\n\n
   */
class INaturalLightClass : public FPStaticInterface {
public:
	// Function publishing enum
	enum {
		natLightIsSun = 0,
		natLightIsSky = 1
	};

	DECLARE_DESCRIPTOR(INaturalLightClass)
	#pragma warning(push)
	#pragma warning(disable:4100)
	BEGIN_FUNCTION_MAP
		FN_0(natLightIsSun, TYPE_BOOL, IsSun)
		FN_0(natLightIsSky, TYPE_BOOL, IsSky)
	END_FUNCTION_MAP
	#pragma warning(pop)

	// Is this class a sun?
	/*! \remarks Returns whether the light is a "Sun" light.\n\n
	  */
	virtual BOOL IsSun() const = 0;

	// Is this class a sky?
	/*! \remarks Returns whether the light is a "Sky" light */
	virtual BOOL IsSky() const = 0;
};

inline INaturalLightClass* GetNaturalLightClass(SClass_ID s, const Class_ID& id)
{
	return static_cast<INaturalLightClass*>( GetInterface(s, id, NATURAL_LIGHT_CLASS_INTERFACE_ID) );
}

inline INaturalLightClass* GetNaturalLightClass(Animatable* a)
{
	return a == NULL ? NULL
		: GetNaturalLightClass(a->SuperClassID(), a->ClassID());
}

// This class is used to set up the function publishing
class NaturalLightClassBaseImp : public INaturalLightClass {
public:
	NaturalLightClassBaseImp(ClassDesc* cd, int classStrID, int isSunStrID, int isSkyStrID)
		: INaturalLightClass(
			NATURAL_LIGHT_CLASS_INTERFACE_ID, _M("NaturalLightClass"),
				classStrID, cd, FP_STATIC_METHODS,
			natLightIsSun, _M("isSun"), isSunStrID, TYPE_BOOL, 0, 0,
			natLightIsSky, _M("isSky"), isSkyStrID, TYPE_BOOL, 0, 0,
			end) {}
};

inline INaturalLightClass* GetNaturalLightClassInterface(ClassDesc* c)
{
	return static_cast<INaturalLightClass*>(c->GetInterface(NATURAL_LIGHT_CLASS_INTERFACE_ID));
}

#define SUNLIGHT_INTERFACE_ID Interface_ID(0x43b76ff2, 0x60ae0d61)

// A sun light is just a direct light with a constant intensity.
/*! class ISunLight : public BaseInterface\n\n

\par Description:
<b>This class is only available in release 5 or later.</b>\n\n
ISunLight is used by the radiosity system to determine whether the intensity of
the light object should be measured as a 3ds Max multiplier (for example
directional light) or as a physical quantity (for example the IES Sun).\n\n
If a light object's class claims to be a sun, it should derive from ISunLight
and implement IsIntensityInMAXUnits to return true if its intensity is in 3ds
Max units, or false if its intensity is in lux.\n\n
   */
class ISunLight : public BaseInterface {
public:
	// Return whether the intensity is in MAX Units. If it isn't
	// in MAX units it must be in international units.
	/*! \remarks return true if its intensity is in 3ds Max units, or false if
	its intensity is in lux. */
	virtual bool IsIntensityInMAXUnits() const = 0;
};

inline ISunLight* GetSunLightInterface(Animatable* o)
{
	return static_cast<ISunLight*>(o->GetInterface(SUNLIGHT_INTERFACE_ID));
}

// This class is used to evaluate the sky for Flow radiosity.
/*! class SkyLightEval : public BaseInterface\n\n

\par Description:
<b>This class is only available in release 5 or later.</b>\n\n
SkyLightEval is used by the radiosity system and LightTracer to sample the
radiance of the sky in a specific direction.\n\n
  */
class SkyLightEval : public BaseInterface {
public:
	// Delete yourself
	/*! \remarks Standard method to delete the object. */
	virtual void DeleteThis() = 0;

	// Returns the spectral radiance along a ray of given origin
	// and direction in world space. Return true if the intensity is in
	// MAX units, or false if intensity is in international units
	/*! \remarks Returns the spectral radiance along a ray of given origin and
	direction in world space. Return true if the intensity is in MAX units, or
	false if intensity is in international units. The radiance is stored in the
	parameter radiance. */
	virtual bool Gather(
		const Point3&		origin,
		const Point3&		direction,
		Point3&				radiance
	) = 0;

	// Return the class of the sky that created you.
	/*! \remarks Returns the class id of the class that created this
	SkyLightEval. This is used by IsSameSky to determine whether the sky has
	changed. */
	virtual Class_ID ClassID() const = 0;

	// Compare two skys. Return true if the Gather method
	// will return the same result.
	/*! \remarks Compare two skys. Return true if the Gather method will
	return the same result. The implementation should determine whether the
	objects are the same type using ClassID(). If they are different types,
	return false. If they are the same type, the parameters should be compared
	to see if the Gather method will return the same values for the two
	objects. */
	virtual bool IsSameSky( const SkyLightEval* sky ) const = 0;
};

#define SKYLIGHT_INTERFACE_ID Interface_ID(0x47056297, 0x7f8b06e3)

// This interface is used to create the
/*! class ISkyLight : public BaseInterface\n\n

\par Description:
<b>This class is only available in release 5 or later.</b>\n\n
<b>ISkyLight</b> is used by the radiosity system and LightTracer to evaluate
skylight. It also is used to determine whether the intensity of the light
object should be measured as a 3ds Max multiplier (for example Texture Sky) or
as a physical quantity (for example the IES Sky).\n\n
If a light object's class claims to be a sky, it should derive from
<b>ISkyLight</b> and implement <b>IsIntensityInMAXUnits</b> to return true if
its intensity is in 3ds Max units, or false if its intensity is in lux. It also
needs to implement <b>CreateSkyEval</b> which is used to create an object that
can be used to evaluate the skylight at different directions.\n\n
Deriving from an interface (such as ISkyLight) also require the child class to 
override GetInterface() to return the ISkyLight interface pointer. Here is a 
code example:\n\n
\verbatim
BaseInterface* YOUR_CLASS::GetInterface(Interface_ID id)
{ 
   if(id == SKYLIGHT_INTERFACE_ID) 
      return (ISkyLight*) this; 
   else return BaseInterface::GetInterface(id);
}
\endverbatim
\n\n
   */
class ISkyLight : public BaseInterface {
public:
	// Create a SkyLightEval that can be used to evaluate
	// this skylight for the radiosity solution. Tm transforms
	// (0,0,1) to the zenith direction for the scene.
	/*! \remarks This method is used to create the object that samples the sky
	for the radiosity and global lighting plugins.\n\n

	\par Parameters:
	<b>TimeValue t</b>\n\n
	T is the current time\n\n
	<b>INode *node</b>\n\n
	node is the node for the light object we are asking to create the
	SkyLightEval\n\n
	<b>const Matrix3\& tm</b>\n\n
	tm is a matrix that transforms (0,0,1) to the Zenith direction (that is
	straight up) for the sky in the scene\n\n
	  */
	virtual SkyLightEval* CreateSkyEval(
		TimeValue		t,
		INode*			node,
		const Matrix3&	tm
	) = 0;

	// Return whether the intensity is in MAX Units. If it isn't
	// in MAX units it must be in international units.
	/*! \remarks none */
	virtual bool IsIntensityInMAXUnits() const = 0;
};

inline ISkyLight* GetSkyLightInterface(Animatable* o)
{
	return static_cast<ISkyLight*>(o->GetInterface(SKYLIGHT_INTERFACE_ID));
}


#define SUNLIGHT_POSITION_INTERFACE_ID Interface_ID(0x6fa56707, 0x4ebe3d73)

class ISunLightPosition : public BaseInterface {
public:
};

inline ISunLightPosition* GetSunLightPositionInterface(Animatable* o)
{
	return static_cast<ISunLightPosition*>(o->GetInterface(SUNLIGHT_POSITION_INTERFACE_ID));
}

