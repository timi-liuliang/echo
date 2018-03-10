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
// DESCRIPTION: Interface for creating and modifying daylight systems programatically.
// AUTHOR: Chloe Mignot - created April 27 2007
//*****************************************************************************/


#pragma once
#include "GetCOREInterface.h"
#include "ifnpub.h"
#include "assetmanagement\AssetUser.h"

class INode;
class Point3;
class LightObject;
class IDaylightSystem;
class IDaylightSystem2;

// --- Interface IDs
#define DAYLIGHTSYSTEM_FACTORY_INTERFACE Interface_ID(0x5c665bc7, 0x61b44a30)

/*! \brief Factory for daylight system objects. The type of Sun and Sky components are specified by the current Market Default.
Use this interface to create instances of the daylight system. 
Client code can get to this interface as follows:
\code
IDaylightSystemFactory* factory = static_cast<IDaylightSystemFactory*>(GetCOREInterface(DAYLIGHTSYSTEM_FACTORY_INTERFACE));
\endcode
This interface is implemented by 3ds Max. 
*/
class IDaylightSystemFactory : public FPStaticInterface
{
public:
	/*! \brief Creates a daylight system and adds it to the scene, at a default position of (0,0,0)
	* \param [in,out] pDaylight returns the component of the daylight system which can be used to access
	* the sun and sky. See IDaylightSystem.
	* \note The type of the sun and the sky components that are created depends on 
	the current Market Defaults settings. See the 3ds Max User Guide for more details.
	* \return A pointer to the Daylight assembly node of the daylight system.
	*/
	virtual INode* Create(IDaylightSystem*& pDaylight) = 0;

	/*! \brief Retrieves the single instance of the Daylight system factory
	*/
	static IDaylightSystemFactory* GetInstance()	{
		return static_cast<IDaylightSystemFactory*>(GetCOREInterface(DAYLIGHTSYSTEM_FACTORY_INTERFACE));
	}
};

#define IID_DAYLIGHT_SYSTEM_FACTORY2 Interface_ID(0x688f66a6, 0x423a37aa)

/*! \brief Factory for daylight system objects. Allows for specifying the type of Sun and Sky components.
 * Use this interface to create instances of the daylight system. 
 * Client code can get to this interface as follows:
 * \code
 * IDaylightSystemFactory2* factory = dynamic_cast<IDaylightSystemFactory2*>(GetCOREInterface(IID_DAYLIGHT_SYSTEM_FACTORY2));
 * \endcode
 * This interface is implemented by 3ds Max. 
*/
class IDaylightSystemFactory2 : public FPStaticInterface
{
public:
	/*! \brief Creates a daylight system with sun and sky components of specified type and adds it to the scene at a default position of (0,0,0)
	* \param [in,out] pDaylight returns the component of the daylight system which 
	* can be used to access the sun and sky. See IDaylightSystem.
	* \param[in] sunClassID Class ID of Sun plugin component to be used when creating the daylight system.
	* The supper class id of the Sun is assumed to be LIGHT_CLASS_ID. If left NULL, the Sun type specified
	* by the Market Defaults will be used.
	* \param[in] skyClassID Class ID of Sky plugin component to be used when creating the daylight system.
	* The supper class id of the Sky is assumed to be LIGHT_CLASS_ID. If left NULL, the Sun type specified
	* by the Market Defaults will be used.
	* \return A pointer to the Daylight assembly node of the daylight system. 
	* NULL if the daylight system could not be created, for example because either 
	* the Sun or the Sky type specified was invalid.
	* \see IMRPhysicalSkyInterface, IMRPhysicalSunLight
	*/
	virtual INode* Create(
		IDaylightSystem2*& pDaylight, 
		const Class_ID* sunClassID = NULL, 
		const Class_ID* skyClassID = NULL) = 0;
};

// --- Interface IDs
#define DAYLIGHT_SYSTEM_INTERFACE Interface_ID(0x20033f9f, 0x6a65470)

/*! \brief Use this interface to access the sun and sky components of a daylight 
* system created by IDaylightSystemFactory.
* This interface can be obtained when the daylight system is created. See IDaylightSystemFactory::Create
* In the case of an already existent daylight system, this interface can be obtained 
* from the base object of the daylight assembly:
* \code
* Object* daylightAssemblyObj = ...
* BaseInterface* bi = daylightAssemblyObj->GetInterface(DAYLIGHT_SYSTEM_INTERFACE);
* IDaylightSystem* ds = dynamic_cast<IDaylightSystem*>(bi);
* \endcode
* \see IDaylightSystemFactory
*/
class IDaylightSystem : public FPMixinInterface 
{
public:
	/*! \brief Returns the sun component of the daylight system.
	* It can be directional light, IES sun or MR sun depending on the market defaults.
	* \return pointer on the sun.
	*/
	virtual LightObject* GetSun() const = 0;

	/*! \brief Returns the sky component of the daylight system.
	* It can be standard sky, IES sky or MR sky depending on the market defaults.
	* \return pointer on the sky.
	*/
	virtual LightObject* GetSky() const = 0;

	/*! \brief Sets the position of the compass of the daylight system.
	* \param [in] position Origin position of the system
	*/
	virtual void SetPosition(const Point3& position) = 0;

	/*! \brief Returns the position of the compass of the daylight system.
	* \return position of the compass of the daylight system.
	*/
	virtual Point3 GetPosition() const = 0;

	/*! \brief Sets the orbital scale of the system.
	* \param [in] orbScale  distance of the sun (the directional light) from the compass rose (orbital scale in the UI)
	* Note that the orbital scale does not have an effect on the sun's precision or how it's rendered.
	*/
	virtual void  SetOrbitalScale(float orbScale) = 0;

	/*! \brief Returns the orbital scale of the system.
	* \return orbital scale value.
	* Note that the orbital scale does not have an effect on the sun's precision or how it's rendered.
	*/
	virtual float GetOrbitalScale() const = 0;

	/*! \brief Sets the value of the north direction.
	* \param [in] angle rotational direction of the compass rose in the scene (north direction in the UI)
	* The angle is in degree. The value is expected to be >=0.
	*/
	virtual void SetNorthDirection(float angle) = 0;

	/*! \brief Returns the value of the north direction.
	* \return the value of the north direction.
	* The angle is in degree. The value is expected to be >=0.
	*/
	virtual float GetNorthDirection() const = 0;

	/*! \brief Sets the compass diameter of this daylight.
	* \param [in] compassDiameter this parameter controls the diameter of the compass rose.
	* Note that the compass diameter does not have an effect on the sun's precision or how it's rendered.
	* The value is expected to be >=0.
	*/
	virtual void SetCompassDiameter(float compassDiameter) = 0;

	/*! \brief Returns the compass diameter of this daylight system.
	* Note that the compass diameter does not have an effect on the sun's precision or how it's rendered.
	* \return compass diameter.
	*/
	virtual float GetCompassDiameter() const = 0;

	/*! \brief Sets the current time associated with this daylight system.
	* \param [in] time time of day. The expected format is : (hour,minute,second)
	*/
	virtual void SetTimeOfDay(const Point3& time) = 0;

	/*! \brief Returns the current time associated with this daylight system.
	* \return current time of the daylight system. 
	* The format is : (hour,minute,second).
	*/
	virtual Point3 GetTimeOfDay() const = 0;

	/*! \brief Sets the current date associated to this daylight system.
	* \param [in] date date of the year. The expected format is : (month,day,year)
	*/
	virtual void SetDate(const Point3& date) = 0;

	/*! \brief Returns the current date associated to this daylight system.
	* \return the current date associated to this daylight system. 
	* The format is (month,day,year).
	*/
	virtual Point3 GetDate() const = 0;

	/*! \brief Set the latitude and longitude of this daylight system, note that
	* the time zone, city, azimuth and altitude are automatically calculated.
	* \param [in] latitude latitude of the location (used to determine city)
	* \param [in] longitude longitude of the location (used to determine city)
	*/
	virtual void SetLatLong(float latitude, float longitude) = 0;

	/*! \brief Returns latitude value of this daylight system.
	* \return Latitude value of this daylight system.
	*/
	virtual float GetLatitude() const = 0;


	/*! \brief Longitude value of this daylight system.
	* \return Longitude value of this daylight system.
	*/
	virtual float GetLongitude() const = 0;

	/*! \brief Set the daylight saving time parameter.
	* \param [in] isDaylightSavingTime When on, calculates daylight savings by adjusting azimuth and altitude 
	* during the summer months. 
	*/
	virtual void SetDaylightSavingTime(BOOL isDaylightSavingTime) = 0;

	/*! \brief Returns the daylight saving time parameter.
	* \return isDaylightSavingTime When on, calculates daylight savings by adjusting azimuth and altitude 
	* during the summer months. 
	*/
	virtual BOOL GetDaylightSavingTime() const = 0;
};

#define IID_DAYLIGHT_SYSTEM2 Interface_ID(0x226e7d9a, 0x215a193f)
/*! \brief Extends IDaylightSystem with new functionality
* This interface can be obtained when the daylight system is created. See IDaylightSystemFactory2::Create
* In the case of an already existent daylight system, this interface can be obtained 
* from the base object of the daylight assembly:
* \code
* Object* daylightAssemblyObj = ...
* BaseInterface* bi = daylightAssemblyObj->GetInterface(IID_DAYLIGHT_SYSTEM2);
* IDaylightSystem2* ds = dynamic_cast<IDaylightSystem2*>(bi);
* \endcode
*/
class IDaylightSystem2 : public IDaylightSystem
{
public:
	/* \brief Retrieves the unique identifier of this interface
	*/
	virtual Interface_ID GetID() { return IID_DAYLIGHT_SYSTEM2; }

	/*! \brief Computes the approximate time zone from a longitude
	* \param [in] longitude The longitude from which the time zone is computed. 
	* The longitude is expected to be in the [-180.0, 180.0] range. 
	* It it isn't, it's converted to this range.
	* \return The approximate time zone corresponding to a given longitude 
	* The time zone will be in the [-12, 12] range.
	*/
	virtual int GetTimeZone(float longitude) const = 0;

	/*! \brief Sets the sun object of the daylight system
	* \param[in] sunClassID Class ID of new sun. The supper class id of the sun is 
	* assumed to be LIGHT_CLASS_ID. 
	* \return A pointer to the new sun object or NULL if the operation failed
	*/
	virtual LightObject* SetSun(const Class_ID& sunClassID) = 0;

	/*! \brief Sets the sun object of the daylight system
	* \param[in] skyClassID Class ID of new sky. The supper class id of the sky is 
	* assumed to be LIGHT_CLASS_ID. 
	* \return A pointer to the new sky object or NULL if the operation failed
	*/
	virtual LightObject* SetSky(const Class_ID& skyClassID) = 0;

        /*! \remarks The DaylightControlType enumeration is for specifying how the daylight system is controlled,manually, from
	* a specified date and time or via a weather file.
	*/
	enum DaylightControlType
	{
		eDateAndTime = 0, eManual = 1, eWeatherFile = 2
	};

	/*! \brief Returns how the location of the sun in the daylight system is being controlled.
	* \return Returns how the location of the sun in the daylight system is being controlled. 
	* If the return value is eDataAndTime, the daylight system is controlled by a specified date and time. If the return value
	* is eManual, then the postion of the sun is controlled manually, if the value is eWeather File then the postion of the sun
	* is driven by a weather file.
	*/
	virtual DaylightControlType GetDaylightControlType()const =0;

	/*! \brief Specify how the location of the sun in the daylight system will be controlled.
	* \param[in] val  Specifies how the location of the sun in the daylight system is being controlled. 
	* If the value is eDataAndTime, the daylight system will be controlled by a specified date and time. If the value
	* is eManual, then the postion of the sun will be controlled manually, if the value is eWeather File then the position of the sun
	* will be driven by a weather file.
	*/
	virtual void SetDaylightControlType(DaylightControlType val) =0;

	/*! \brief Returns the name of the location of the daylight system. Usually this is the city name.
	* \return Returns the name of the location, usually a city name. If no location is specified in the Daylight System an empty string will be returned.
	*/
	virtual MSTR GetLocation() = 0;

	/*! \brief Get the alititude and the azimuth angles of the sun at that particular time.
	\param[in] t The time at which to get the alitude and azimuth of the sun.
	\param[out] altitude The angle of the sun above the horizon.  In radians, ranges from 0 at the horizon to PI/2 at the zenith.
	\param[out] azimuth The angle of the sun about north.  In radians, ranges from 0 at due north, PI/2 at due east, etc..
	*/
	virtual void GetAltitudeAzimuth(TimeValue t, float &altitude,float &azimuth) =0;

	/*! \brief Get the name of the weather file that is in use in the daylight system. 
	* \return Returns an AssetUser pointing on the weather file.  Note that a valid name may be returned even if the weather file isn't being used
	* to drive the sun position.
	\see GetDaylightControlType.
	*/
	virtual MaxSDK::AssetManagement::AssetUser GetWeatherFile()const =0;

	/*!\brief Set the name of the weather file.
	*  \param[in] val The AssetUser containing the weather file name that may be used to drive the daylight system.
	*/
	virtual void SetWeatherFile(const MaxSDK::AssetManagement::AssetUser &val) =0;

	/*! \brief Opens the weather file dialog. 
	* Opens the Weather File Dialog. Note that if the daylight system isn't being controlled by the weather file,
	\see GetDaylightControlType, then this operation will do nothing.
	*/
	virtual void OpenWeatherFileDlg() = 0;

	
	/*! \brief Get temperature values.
	* Get temperature values usually from a weather file based upon the current time.
	* \param[out] dryBulbTemperature  The dry bulb temperature usually stored in the weather file at the current time.
	* \param[out] dewPointTemperature  The dew point temperatureusually  stored in the weather file at the current time.
	* \return Will return false if the daylight system isn't being controlled by a weather file of the weather file
	* can't be opened correctly or if the weather file doesn't contain these values accurately otherwise if it gets the values
	* correctly it will return true.   In the future these values may be stored elsehwhere besides weather files.
	*/
	virtual bool GetTemps(float &dryBulbTemperature, float &dewPointTemperature)=0;

	/*! \brief Get extraterrestrial radiation values.
	* Get extraterrestrial radiation values usually from a weather file based upon the current time.
	* \param[out] extraterrestrialHorizontalRadiation  The extraterrestrial horizontal radiation usually stored in the weather file at
	* the current time.
	* \param[out] extraterrestrialDirectNormalRadiation  The extraterrestrial direct normal radiation usually stored in the weather file
	* at the current time.
	* \return Will return false if the daylight system isn't being controlled by a weather file of the weather file
	* can't be opened correctly or if the weather file doesn't contain these values accurately otherwise if it gets the values
	* correctly it will return true.   In the future these values may be stored elsehwhere besides weather files.
	*/
	virtual bool GetExtraTerrestialRadiation(float &extraterrestrialHorizontalRadiation, float &extraterrestrialDirectNormalRadiation)=0;


	/*! \brief Get the radiation values.
	* Get radiation values usually from a weather file based upon the current time.
	* \param[out] globalHorizontalRadiation  The global horizontal radiation usually stored in the weather file at
	* the current time.
	* \param[out] directNormalRadiation  The direct normal radiation usually stored in the weather file
	* at the current time.
	* \param[out] diffuseHorizontalRadiation  The diffuse horizontal radiation usually stored in the weather file
	* at the current time.
	* \return Will return false if the daylight system isn't being controlled by a weather file of the weather file
	* can't be opened correctly or if the weather file doesn't contain these values accurately otherwise if it gets the values
	* correctly it will return true.  In the future these values may be stored elsehwhere besides weather files.
	*/
	virtual bool GetRadiation(float &globalHorizontalRadiation, float &directNormalRadiation,
											 float &diffuseHorizontalRadiation)=0;


	/*! \brief Get the illuminance values.
	* Get illuminance values usually from a weather file based upon the current time.
	* \param[out] globalHorizontalIlluminance  The global horizontal illuminance usually stored in the weather file at
	* the current time.
	* \param[out] directNormalIlluminance  The direct normal illuminance usually stored in the weather file
	* at the current time.
	* \param[out] diffuseHorizontalIlluminance The diffuse horizontal illuminance usually stored in the weather file
	* at the current time.
	* \param[out] zenithIlluminance The zenith illuminance usually stored in the weather file
	* at the current time.
	* \return Will return false if the daylight system isn't being controlled by a weather file of the weather file
	* can't be opened correctly or if the weather file doesn't contain these values accurately otherwise if it gets the values
	* correctly it will return true.  In the future these values may be stored elsehwhere besides weather files.
	*/
	virtual bool GetIlluminance(float &globalHorizontalIlluminance, float &directNormalIlluminance,
											 float &diffuseHorizontalIlluminance, float &zenithLuminance)=0;
	//@}

};

#define IID_DAYLIGHT_SYSTEM3 Interface_ID(0x36df4e9f, 0x6b160e40)
/*! \brief Extends IDaylightSystem2 with new functionality
* This interface can only be obtained from an already existent daylight system,
* from the base object of the daylight assembly:
* \code
* Object* daylightAssemblyObj = ...
* BaseInterface* bi = daylightAssemblyObj->GetInterface(IID_DAYLIGHT_SYSTEM3);
* IDaylightSystem3* ds = dynamic_cast<IDaylightSystem3*>(bi);
* \endcode
*/
class IDaylightSystem3 : public IDaylightSystem2
{
public:
	/*! \brief Get the alititude and the azimuth angles of the sun at that particular time.
	\param[in] t The time at which to get the alitude and azimuth of the sun.
	\param[out] altitude The angle of the sun above the horizon.  In radians, ranges from 0 at the horizon to PI/2 at the zenith.
	\param[out] azimuth The angle of the sun about north.  In radians, ranges from 0 at due north, PI/2 at due east, etc..
	*/
	virtual void GetAltAz(TimeValue t, float &altitude,float &azimuth)=0;
};

