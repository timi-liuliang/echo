//*****************************************************************************/
// Copyright (c) 1998-2007 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//*****************************************************************************/
/*===========================================================================*\
 | 
 |  FILE:  imrPhysicalSunLight.h
 |         
 | 
 |  AUTH:   Benjamin Cecchetto
 |          Nov 7th, 2007
 |			
\*===========================================================================*/

#pragma once

#include "..\baseinterface.h"
#include "..\maxtypes.h"

// forward declarations
class Interval;

//! \brief Class id for the Mental Ray Physical Sun Light object
#define MR_PHYSSUN_CLASS_ID Class_ID(0x71503bbb, 0x25e21124)
//! \brief Interface id for the Mental Ray Physical Sun Light object
#define IID_MR_PHYSICAL_SUN_LIGHT Interface_ID(0x4c230590, 0x15fe4287)

//! \brief Access interface for the Mental Ray Physical Sun Light.
/*! This class represents the Mental Ray Physical Sun Light implemented by 3ds Max.
Given a pointer to a sun light, one can get the parameters as follows:
\code 
Object* sunlight = // obtain a pointer to an object
BaseInterface* sunInterface = sunlight->GetInterface(IID_MR_PHYSICAL_SUN_LIGHT);
IMRPhysicalSunLight* physSunInterface =  dynamic_cast<IMRPhysicalSunLight*>(sunInterface);
\endcode
*/
class IMRPhysicalSunLight  : public BaseInterface {
public:
	//! \brief Returns the Interface ID of this interface
	virtual Interface_ID GetID() { return IID_MR_PHYSICAL_SUN_LIGHT; };

	//! \brief A virtual destructor for the interface
	virtual ~IMRPhysicalSunLight() {};

	//! \name Inherit from mrSky
	//! \brief Uses the equivalent settings from the mrSky Parameters rollout
	//! for the remaining mr Physical Sky Parameters rollout settings, except
	//! for Aerial Perspective. Default=on.
	//@{
	/*! \brief Gets whether or not the environment map inherits from mrSky.
	\param[in] t - The time at which we're getting the value
	\param[in] valid - The interval at which we're getting the value
	\return	A boolean if we're inheriting from mrSky or not
	*/
	virtual bool   getInheritFromSky(TimeValue &t, Interval &valid) const = 0;
	/*! \brief Sets whether or not the environment map inherits from mrSky.
	\param[in] t - The time at which to set the value
	\param[in] inheritFromSky - A boolean value of what whether or not we're inheriting from mrSky
	\return true if the setting was successful	
	*/
	virtual bool   setInheritFromSky(TimeValue t, bool inheritFromSky) = 0;
	//@}

	//! \name Photon Target Enabled
	//! \brief Focus global-illumination photons on an area of interest. For
	//! example, if you've modeled a huge city as a backdrop, but are 
	//! rendering only a room interior, you probably don't want mental ray 
	//! to shoot photons over the entire city, with the result that only a
	//! few will find their way into the room. When on, uses the Radius 
	//! (or PhotonTarget) setting with respect to the light target.
	//@{
	/*! \brief Gets whether or not the a photon target is enabled.
	\param[in] t - The time at which we're getting the value
	\param[in] valid - The interval at which we're getting the value
	\return	A boolean if a photon target is enabled
	*/
	virtual bool   getPhotonTargetEnabled(TimeValue &t, Interval &valid) const = 0;
	/*! \brief Sets whether or not the a photon target is enabled.
	\param[in] t - The time at which to set the horizon height
	\param[in] enablePhotonTarget - A boolean value to enable/disable a photon target
	\return true if the setting was successful	
	*/
	virtual bool   setPhotonTargetEnabled(TimeValue t, bool enablePhotonTarget) = 0;
	//@}

	//! \name Photon Target
	//! \brief The radial distance from the light target in which the mr Sun casts
	//! GI photons.
	//@{
	/*! \brief Gets the radial distance from the photon target.
	\param[in] t - The time at which we're getting the radius
	\param[in] valid - The interval at which we're getting the radius
	\return	The current radius from the photon target
	*/
	virtual float  getPhotonTarget  (TimeValue &t, Interval &valid) const = 0;
	/*! \brief Sets the radial distance from the photon target.
	\param[in] t - The time at which to set the radius
	\param[in] photonTarget - The value of what to set photon target radius to
	\return true if the setting was successful	
	*/
	virtual bool   getPhotonTarget  (TimeValue t, float photonTarget) = 0;
	//@}

	//! \name Multiplier
	//! \brief The multiplier for the value of the sun's light output.
	//@{
	/*! \brief Gets the value of the multiplier for the sun's light output.
	\param[in] t - The time at which we're getting the multiplier
	\param[in] valid - The interval at which we're getting the multiplier
	\return	The value of the multiplier
	*/
	virtual float  getMultiplier   (TimeValue &t, Interval &valid) const = 0;
	/*! \brief Sets the value of the multiplier for the sun's light output.
	\param[in] t - The time at which to set the multiplier
	\param[in] multiplier - The value of what to set the multiplier to
	\return true if the setting was successful	
	*/
	virtual bool   setMultiplier   (TimeValue t, float multiplier) = 0;
	//@}

	//! \name Sky Multiplier
	//! \brief The multiplier for the value of the sky's brightness.
	//@{
	/*! \brief Gets the value of the multiplier for the sky's brightness.
	\param[in] t - The time at which we're getting the multiplier
	\param[in] valid - The interval at which we're getting the multiplier
	\return	The value of the multiplier
	*/
	virtual float  getSkyMultiplier   (TimeValue &t, Interval &valid) const = 0;
	/*! \brief Sets the value of the multiplier for the sky's brightness.
	\param[in] t - The time at which to set the multiplier
	\param[in] skyMultiplier - The value of what to set the multiplier to
	\return true if the setting was successful	
	*/
	virtual bool   setSkyMultiplier   (TimeValue t, float skyMultiplier) = 0;
	//@}

	//! \name Haze
	//! \brief The amount of particulate matter in the air. Possible values range from 0.0 
	//!	(a completely clear day) to 15.0 (extremely overcast, or a sandstorm in 
	//! the Sahara). Default=0.0.
	//! 
	//@{
	/*! \brief Gets the haze value of the sky.
	\param[in] t - The time at which we're getting the haze value 
	\param[in] valid - The interval at which we're getting the haze value
	\return	The value of the haze
	*/
	virtual float  getHaze(TimeValue &t, Interval &valid) const = 0;
	/*! \brief Sets the haze value of the sky.
	\param[in] t - The time at which to set the haze value
	\param[in] haze - The value of what to set the haze value to
	\return true if the setting was successful	
	*/
	virtual bool   setHaze(TimeValue t, float haze) = 0;
	//@}

	//! \name Red/Blue Shift
	//! \brief Provides artistic control over the redness of the sky light. The default value 
	//! of 0.0 is the physically correct value (calculated for a 6500K whitepoint), 
	//! but can be changed with this parameter, which ranges from -1.0 (extremely blue) 
	//! to 1.0 (extremely red).
	//@{
	/*! \brief Gets the red/blue tint value of the sky.
	\param[in] t - The time at which we're getting the red/blue shift
	\param[in] valid - The interval at which we're getting the red/blue shift
	\return	The value of the red/blue shift
	*/
	virtual float  getRedBlueShift (TimeValue &t, Interval &valid) const = 0;
	/*! \brief Sets the red/blue tint value of the sky.
	\param[in] t - The time at which to set the red/blue shift
	\param[in] redBlueShift - The value of what to set the red/blue shift to
	\return true if the setting was successful	
	*/
	virtual bool   setRedBlueShift (TimeValue t, float redBlueShift) = 0;
	//@}

	//! \name Saturation
	//! \brief Provides artistic control over saturation of the sky light. The default 
	//! value of 1.0 is the physically calculated saturation level. Possible 
	//! values range from 0.0 (black and white) to 2.0 (extremely high 
	//! saturation).
	//@{
	/*! \brief Gets the saturation value of the sky.
	\param[in] t - The time at which we're getting the saturation value
	\param[in] valid - The interval at which we're getting the saturation value
	\return	The value of the saturation
	*/
	virtual float  getSaturation   (TimeValue &t, Interval &valid) const = 0;	
	/*! \brief Sets the saturation value of the sky.
	\param[in] t - The time at which to set the saturation
	\param[in] saturation - The value of what to set the saturation to
	\return true if the setting was successful	
	*/
	virtual bool   setSaturation   (TimeValue t, float saturation) = 0;	
	//@}

	//! \name Horizon Height
	//! \brief The vertical position of the horizon. Default=0.0.
	//@{
	/*! \brief Gets the horizon height value.
	\param[in] t - The time at which we're getting the horizon height
	\param[in] valid - The interval at which we're getting the horizon height
	\return	The value of the horizon height
	*/
	virtual float  getHorizonHeight(TimeValue &t, Interval &valid) const = 0;
	/*! \brief Sets the horizon height value.
	\param[in] t - The time at which to set the horizon height
	\param[in] horizonHeight - The value of what to set the horizon height to
	\return true if the setting was successful	
	*/
	virtual bool   setHorizonHeight(TimeValue t, float horizonHeight) = 0;
	//@}

	//! \name Shadow Softness
	//! \brief The softness of shadow edges. The default value of 1.0 
	//! accurately matches the softness of real solar shadows. Lower values
	//! make the shadows sharper and higher values make them softer.
	//@{
	/*! \brief Gets the value of the shadow softness.
	\param[in] t - The time at which we're getting the value
	\param[in] valid - The interval at which we're getting the value
	\return	The value of the shadow softness
	*/
	virtual float  getShadowSoftness (TimeValue &t, Interval &valid) const = 0;
	/*! \brief Sets the value of the shadow softness.
	\param[in] t - The time at which to set the value
	\param[in] shadowSoftness - The value of what to set the shadow softness to
	\return true if the setting was successful	
	*/
	virtual bool   setShadowSoftness (TimeValue t, float shadowSoftness) = 0;
	//@}

	//! \name Shadow Samples
	//! \brief The number of shadow samples for the soft shadows. If it is set
	//! to 0, no soft shadows are generated. Default=8.
	//@{
	/*! \brief Gets the number of shadow samples.
	\param[in] t - The time at which we're getting the value
	\param[in] valid - The interval at which we're getting the value
	\return	The number of samples being used for soft shadows.
	*/
	virtual int    getShadowSamples (TimeValue &t, Interval &valid) const = 0;
	/*! \brief Sets the number of shadow samples.
	\param[in] t - The time at which to set the value
	\param[in] shadowSamples - The value of what to set number of samples to
	\return true if the setting was successful	
	*/
	virtual bool   setShadowSamples (TimeValue t, int shadowSamples) = 0;
	//@}
};

