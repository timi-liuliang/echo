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

//! \brief Class id for the Mental Ray Physical Sky Environment Map
#define MRSKYENVSHADER_CLASS_ID	Class_ID(0x628e269e, 0x72386abf) 
//! \brief Interface id for the Mental Ray Physical Sky Environment Map object
#define IID_MR_PHYSICAL_SKY_ENV_MAP Interface_ID(0x72941467, 0x1a8f3ae2)

class Texmap;

//! \brief Access interface for the Mental Ray Physical Sky Environment Map.
/*! This class represents the Mental Ray Physical Sky Environment Map, implemented by 3ds Max.
SDK client code can get this interface by querying 3ds Max for the current
environment map, then asking that object for this interface:
\code 
Texmap* envMap = GetCOREInterface()->GetEnvironmentMap()
BaseInterface* envMapInterface = envMap->GetInterface(IID_MR_PHYSICAL_SKY_ENV_MAP);
IMRPhysicalSkyEnvironmentMap* mrEnvMap = dynamic_cast<IMRPhysicalSkyEnvironmentMap*>(envMapInterface);
\endcode
*/
class IMRPhysicalSkyEnvironmentMap : public BaseInterface {
public:
	//! \brief Returns the Interface ID of this interface
	virtual Interface_ID GetID() { return IID_MR_PHYSICAL_SKY_ENV_MAP; };
	
	//! \brief A virtual destructor for the interface
	virtual ~IMRPhysicalSkyEnvironmentMap() {};

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
	\param[in] inherits - A boolean value of what whether or not we're inheriting from mrSky
	\return true if the setting was successful	
	*/
	virtual bool   setInheritFromSky(TimeValue t, bool inherits) = 0;
	//@}

	//! \name Background Enabled
	//! \brief When on but no background map is specified, the background of
	//! the rendering is transparent black, suitable for external compositing.
	//! If you supply a background shader by clicking the button and then
	//! specifying a map or shader, the background of the rendering will come
	//! from that shader (for example, a texture map that uses a background
	//! photograph). In either case the mr Physical Sky will still be visible
	//! in reflections and refraction.
	//@{
	/*! \brief Gets the boolean value of the background being enabled
	\param[in] t - The time at which we're getting the value
	\param[in] valid - The interval at which we're getting the value
	\return	The value of if the background is enabled or not
	*/
	virtual bool   getBackgroundEnabled(TimeValue &t, Interval &valid) const = 0;
	/*! \brief Sets the boolean value of the background being enabled
	\param[in] t - The time at which to set the value
	\param[in] enableBackground - The value of what to set the background being enabled to
	\return true if the setting was successful	
	*/
	virtual bool   setBackgroundEnabled(TimeValue t, bool enableBackground) = 0;
	//@}

	//! \name Sun Disk Intensity
	//! \brief The brightness of the sun.
	//@{
	/*! \brief Gets the intensity of the sun's disk.
	\param[in] t - The time at which we're getting the intensity
	\param[in] valid - The interval at which we're getting intensity
	\return	The value of the intensity
	*/
	virtual float  getSunDiskIntensity(TimeValue &t, Interval &valid) const = 0;
	/*! \brief Sets the intensity of the sun's disk.
	\param[in] t - The time at which to set the intensity
	\param[in] sunDiskIntensity - The value of what to set the intensity to
	\return true if the setting was successful	
	*/
	virtual bool   setSunDiskIntensity(TimeValue t, float sunDiskIntensity) = 0;
	//@}

	//! \name Sun Disk Scale
	//! \brief The size of the sun in the sky.
	//@{
	/*! \brief Gets the scale of the sun.
	\param[in] t - The time at which we're getting the scale
	\param[in] valid - The interval at which we're getting the scale
	\return	The value of the scale
	*/
	virtual float  getSunDiskScale(TimeValue &t, Interval &valid) const = 0;
	/*! \brief Sets the scale of the sun.
	\param[in] t - The time at which to set the scale
	\param[in] sunDiskScale - The value of what to set the scale to
	\return true if the setting was successful	
	*/
	virtual bool   setSunDiskScale(TimeValue t, float sunDiskScale) = 0;
	//@}

	//! \name Sun Glow Intensity
	//! \brief The brightness of the glow surrounding the sun.
	//@{
	/*! \brief Gets the intensity of the sun's glow.
	\param[in] t - The time at which we're getting the intensity
	\param[in] valid - The interval at which we're getting the intensity
	\return	The value of the intensity
	*/
	virtual float  getSunGlowIntensity(TimeValue &t, Interval &valid) const = 0;
	/*! \brief Sets the intensity of the sun's glow.
	\param[in] t - The time at which to set the intensity
	\param[in] sunGlowIntensity - The value of what to set the intensity to
	\return true if the setting was successful	
	*/
	virtual bool   setSunGlowIntensity(TimeValue t, float sunGlowIntensity) = 0;
	//@}

	//! \name Background
	//! \brief The custom user specified background to be used as a background
	//! map in the scene.
	//@{
	/*! \brief Gets the current background being used.
	\param[in] t - The time at which we're getting background
	\param[in] valid - The interval at which we're getting the background
	\return	A pointer to a texture map of the background
	*/
	virtual Texmap*  getBackground(TimeValue &t, Interval &valid) const = 0;
	/*! \brief Sets the background of the scene.
	\param[in] t - The time at which to set the background
	\param[in] background - A pointer to a texture map of what to set the background to
	\return true if the setting was successful	
	*/
	virtual bool     setBackground(TimeValue t, Texmap* background) = 0;
	//@}
};

