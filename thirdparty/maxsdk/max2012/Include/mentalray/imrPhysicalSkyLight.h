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
 |  AUTH:   Claude RObillard
 |          Taken from SkyLight.cpp 8/20/01
 |			Chloe Mignot: 05/04/2007 moved to src\include
\*===========================================================================*/

#pragma once

#include "..\baseinterface.h"
#include "..\maxtypes.h"
#include "..\acolor.h"
// forward declarations
class Interval;

//! \brief Class id for the Mental Ray Physical Sky object
#define MRPHYSSKY_LIGHT_OBJECT_CLASSID	Class_ID(0x26ee3350, 0x799b5b56)
//! \brief Interface id for the Mental Ray Physical Sky object
#define MRPHYSSKY_LIGHT_INTERFACECLASS_ID Interface_ID(0x23a857ab, 0x5fad4778)

//! \brief Access interface for the Mental Ray Physical Sky object.
/*! This class represents the Mental Ray Physical Sky object implemented by 3ds Max.
Given a pointer to a skylight object, one can get this interfaces as follows:
\code 
Object* skylight = // obtain a pointer to an object
BaseInterface* skyInteface = skylight->GetInterface(MRPHYSSKY_LIGHT_INTERFACECLASS_ID);
IMRPhysicalSkyInterface* physSkyInterface = dynamic_cast<IMRPhysicalSkyInterface*>(skyInterface);
\endcode
*/
class IMRPhysicalSkyInterface : public BaseInterface {
public:
	enum SkyModel { eHaze = 0, ePerez, eCIE};
	enum CIEModel { eOvercastSky = 0, eClearSky};

	//! \brief Returns the Interface ID of this interface
	virtual Interface_ID GetID() { return MRPHYSSKY_LIGHT_INTERFACECLASS_ID; };
	
	//! \brief A virtual destructor for the interface
	virtual ~IMRPhysicalSkyInterface() {};

	//! \name Multiplier
	//! \brief The multiplier for the value of the sky's brightness
	//@{
	/*! \brief Gets the value of the multiplier for the sky's brightness.
	\param[in] t - The time at which we're getting the multiplier
	\param[in] valid - The interval at which we're getting the multiplier
	\return	The value of the multiplier
	*/
	virtual float  getMultiplier   (TimeValue &t, Interval &valid) const = 0;
	/*! \brief Sets the value of the multiplier for the sky's brightness.
	\param[in] t - The time at which to set the multiplier
	\param[in] multiplier - The value of what to set the multiplier to
	\return true if the setting was successful	
	*/
	virtual bool   setMultiplier   (TimeValue t, float multiplier) = 0;
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

	//! \name Ground Color
	//! \brief The color of the virtual ground plane. Note that this is a diffuse 
	//! reflectance value (that is, albedo). The ground appears as a Lambertian 
	//! reflector with this diffuse color, lit by the sun and sky only, and 
	//! does not receive any shadows.

	//@{
	/*! \brief Gets the ground color.
	\param[in] t - The time at which we're getting the ground color
	\param[in] valid - The interval at which we're getting the ground color
	\return	The value of the ground color
	*/
	virtual AColor getGroundColor  (TimeValue &t, Interval &valid) const = 0;
	/*! \brief Sets the ground color.
	\param[in] t - The time at which to set the ground color
	\param[in] groundColor - The value of what to set the ground color to
	\return true if the setting was successful	
	*/
	virtual bool   setGroundColor  (TimeValue t, AColor groundColor) = 0;
	//@}

	//! \name Night Color
	//! \brief The minimum color of the sky: The sky will never become darker than 
	//! this value. It can be useful for adding things like moon, stars, 
	//! high-altitude cirrus clouds that remain lit long after sunset, etc. 
	//! As the sun sets and the sky darkens, the contribution from Night 
	//! Color is unaffected and remains as the base light level.
	//@{
	/*! \brief Gets the night color of the sky.
	\param[in] t - The time at which we're getting the night color
	\param[in] valid - The interval at which we're getting the night color
	\return	The value of the night color
	*/
	virtual AColor getNightColor   (TimeValue &t, Interval &valid) const = 0;
	/*! \brief Sets the night color of the sky.
	\param[in] t - The time at which to set the night color
	\param[in] nightColor - The value of what to set the night color to
	\return true if the setting was successful	
	*/
	virtual bool   setNightColor   (TimeValue t, AColor nightColor) = 0;
	//@}

	//! \name Horizon Blur
	//! \brief The 'blurriness' with which the horizon is rendered. Default=0.1.
	//@{
	/*! \brief Gets the horizon blur value.
	\param[in] t - The time at which we're getting the horizon blur value
	\param[in] valid - The interval at which we're getting the horizon blur
	\return	The value of the horizon blur
	*/
	virtual float  getHorizonBlur  (TimeValue &t, Interval &valid) const = 0;
	/*! \brief Sets the horizon blur value.
	\param[in] t - The time at which to set the horizon blur value
	\param[in] horizonBlur - The value of what to set the horizon blur to
	\return true if the setting was successful	
	*/
	virtual bool   setHorizonBlur  (TimeValue t, float horizonBlur) = 0;
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

	//! \name Aerial Perspective
	//! \brief How distant objects are perceived as hazier and tinted towards the
	//! blue end of the spectrum. This is a boolean for enabling/disabling aerial
	//! perspective.
	//@{
	/*! \brief Gets whether or not aerial perspective is enabled.
	\param[in] t - The time at which we're getting the aerial perspective
	\param[in] valid - The interval at which we're getting the aerial perspective
	\return	A boolean value enabling or disabling the aerial perspective
	*/
	virtual bool   getAerialPerspective(TimeValue &t, Interval &valid) const = 0;
	/*! \brief Sets whether or not aerial perspective is enabled.
	\param[in] t - The time at which to set the aerial perspective
	\param[in] aerialPerspective - Enable/disable the aerial perspective
	\return true if the setting was successful	
	*/
	virtual bool   setAerialPerspective(TimeValue t, bool aerialPerspective) = 0;  	
	//@}

	//! \name Visibility Distance
	//! \brief The visibility distance of the aerial perspective. When nonzero,
	//! it defines the the distance at which
	//! approximately 10% of haze is visible at a Haze level of 0.0.
	//@{
	/*! \brief Gets the visibility distance value.
	\param[in] t - The time at which we're getting the visibility distance
	\param[in] valid - The interval at which we're getting the visibility distance
	\return	The value of the visibility distance
	*/
	virtual float  getVisibilityDistance(TimeValue &t, Interval &valid) const = 0;
	/*! \brief Sets the visibility distance value.
	\param[in] t - The time at which to set the visibility distance
	\param[in] visibilityDistance - The value of what to set the visibility distance to
	\return true if the setting was successful	
	*/
	virtual bool   setVisibilityDistance(TimeValue t, float visibilityDistance) = 0;
	//@}

	//! \name Sky Model
	//! \brief The type of internal sky model that will be used to determine the sky luminance
	//! distrubtion pattern.
	//@{
	/*! \brief Gets the type of sky model being used
	\param[in] t - The time at which we're getting the sky model
	\param[in] valid - The interval at which we're getting the sky model
	\return	The sky model type either SkyModel::eHaze, the default haze model, SkyModel::ePerez, the Perez sky model, or
	SkyModel::eCIE, the CIE sky model type
	*/
	virtual SkyModel getSkyModel(TimeValue &t, Interval &valid) const = 0;
	/*! \brief Sets the type of sky model being used.
	\param[in] t - The time at which we are setting the sky model
	\param[in] skyModel - The value that the sky model will be set to.  It can be either SkyModel::eHaze, SkyModel::ePerez,
	or SkyModel::eCIE
	\return true if the setting was successful, false otherwise
	*/
	virtual bool	setSkyModel(TimeValue t, SkyModel skyModel) = 0;
	//@}

	//! \name Perez Diffuse Horizontal Illuminance
	//! \brief The diffuse horizontal illuminance value used by the Perez sky model
	//@{
	/*! \brief Gets the Perez diffuse horizontal illuminance value
	\param[in] t - The time at which we're getting the diffuse horizontal illuminance for the Perez model
	\param[in] valid - The interval at which we're getting the diffuse horizontal illuminance for the Perez model
	\return	The diffuse horizontal illuminance value used by the Perez sky model
	*/
	virtual float  getPerezDiffuseHorizIlluminance(TimeValue &t, Interval &valid) const = 0;
	/*! \brief Sets the Perez diffuse horizontal illuminance value
	\param[in] t - The time at which we are setting the Perez diffuse horizontal illuminance
	\param[in] perezDiffuseHorizIlluminance - The value that the Perez diffuse horizontal illuminance will be set to
	\return true if the setting was successful, false otherwise
	*/
	virtual bool   setPerezDiffuseHorizIlluminance(TimeValue t, float perezDiffuseHorizIlluminance) = 0;
	//@}

	//! \name Perez Direct Normal Illuminance
	//! \brief The direct normal illuminance value used by the Perez Sky Model.  This value will drive a MR Sun object
	//! if one exists in the same daylight system as the sky object
	//@{
	/*! \brief Gets the Perez direct normal illuminance value
	\param[in] t - The time at which we're getting the direct normal illuminance for the Perez model
	\param[in] valid - The interval at which we're getting the direct normal illuminance for the Perez model
	\return	The direct normal illuminance value used by the Perez model
	*/
	virtual float  getPerezDirectNormalIlluminance(TimeValue &t, Interval &valid) const = 0;
	/*! \brief Sets the Perez direct normal illuminance value
	\param[in] t - The time at which we are setting the Perez direct normal illuminance
	\param[in] perezDirectNormalIlluminance - The value that the Perez direct normal illuminance will be set to
	\return true if the setting was successful, false otherwise
	*/
	virtual bool   setPerezDirectNormalIlluminance(TimeValue t, float perezDirectNormalIlluminance) = 0;
	//@}

	//! \name CIE Diffuse Horizontal Illuminance
	//! \brief The diffuse horizontal illuminance value used by the CIE sky model
	//@{
	/*! \brief Gets the CIE diffuse horizontal illuminance value
	\param[in] t - The time at which we're getting the diffuse horizontal illuminance for the CIE model
	\param[in] valid - The interval at which we're getting the diffuse horizontal illuminance for the CIE model
	\return	The diffuse horizontal illuminance value used by the CIE sky model
	*/
	virtual float  getCIEDiffuseHorizIlluminance(TimeValue &t, Interval &valid) const = 0;
	/*! \brief Sets the CIE diffuse horizontal illuminance value
	\param[in] t - The time at which we are setting the CIE diffuse horizontal illuminance
	\param[in] cieDiffuseHorizIlluminance - The value that the CIE diffuse horizontal illuminance will be set to
	\return true if the setting was successful, false otherwise
	*/
	virtual bool   setCIEDiffuseHorizIlluminance(TimeValue t, float cieDiffuseHorizIlluminance) = 0;
	//@}

	//! \name CIE Model
	//! \brief The CIE sky type supports two models, an overcast sky model and a clear sky model.
	//@{
	/*! \brief Get the CIE sky type model
	\param[in] t - The time at which we're getting the CIE sky type model
	\param[in] valid - The interval at which we're getting the model, overcast or clear, for the CIE sky type
	\return	Will return either CIEModel::eOvercastSky or CIEModel::eClearSky
	*/
	virtual CIEModel getCIEModel(TimeValue &t,Interval &valid) const = 0;
	/*! \brief Set the CIE sky type model
	\param[in] t - The time at which we are setting the CIE sky model type
	\param[in] model - The CIE sky model that will be used, can be ither CIEModel::eOvercastSky or CIEModel::eClearSky
	\return true if the setting was successful	
	*/
	virtual bool setCIEModel(TimeValue t,CIEModel model) =0;
	//@}

	/*! \brief Get the current calculate Perez Params based upon the current illuminance (or possibly irradiance if coming from a
	weather file). Please see All Weather Model For Sky Luminance Distrubution - Preliminary Configuration and Validation, R Perez et al. 1989.
	\param[out] A -  If negative horizon is brighter than zenith else darker
	\param[out] B -  Luminance Gradient near the horizon
	\param[out] C -  Relative intensity of the circumsolar region
	\param[out] D -  Width of circumsolar region
	\param[out] E -  Relative intensity of backscattered ligth at earth's surface
	\param[in] t - The time at which we're getting the multiplier
	\param[in] valid - The interval at which we're getting the multiplier
	*/
	virtual void getPerezABCDE(float &A, float &B, float &C, float &D,float &E,TimeValue t, Interval &valid) const = 0;

};

//! \brief Interface id for the additional functionality for the Mental Ray Physical Sky object
#define MRPHYSSKY_LIGHT_INTERFACECLASS2_ID Interface_ID(0x3e3e5a80, 0x157908fb)

//! \brief Access to additional fucntionality for the Mental Ray Physical Sky object.
/*! This class represents additional functionality to the Mental Ray Physical Sky object.
Given a pointer to a skylight object, one can get this interfaces as follows:
\code 
Object* skylight = // obtain a pointer to an object
BaseInterface* skyInteface = skylight->GetInterface(MRPHYSSKY_LIGHT_INTERFACECLASS2_ID);
IMRPhysicalSkyInterface2* physSkyInterface = dynamic_cast<IMRPhysicalSkyInterface2*>(skyInterface);
\endcode
*/
class IMRPhysicalSkyInterface2 : public IMRPhysicalSkyInterface {
public:

	/* \brief Retrieves the unique identifier of this interface
	*/
	virtual Interface_ID GetID() { return MRPHYSSKY_LIGHT_INTERFACECLASS2_ID; }

	//! \name CIE Direct Normal Illuminance
	//! \brief The direct normal illuminance value used by the CIE Sky Model.  This value will drive a MR Sun object
	//! if one exists in the same daylight system as the sky object
	//@{
	/*! \brief Gets the CIE direct normal illuminance value
	\param[in] t - The time at which we're getting the direct normal illuminance for the CIE model
	\param[in] valid - The interval at which we're getting the direct normal illuminance for the CIE model
	\return	The direct normal illuminance value used by the CIE model
	*/
	virtual float  getCIEDirectNormalIlluminance(TimeValue &t, Interval &valid) const = 0;
	/*! \brief Sets the CIE direct normal illuminance value
	\param[in] t - The time at which we are setting the CIE direct normal illuminance
	\param[in] CIEDirectNormalIlluminance - The value that the CIE direct normal illuminance will be set to
	\return true if the setting was successful, false otherwise
	*/
	virtual bool   setCIEDirectNormalIlluminance(TimeValue t, float CIEDirectNormalIlluminance) = 0;
	//@}
};
