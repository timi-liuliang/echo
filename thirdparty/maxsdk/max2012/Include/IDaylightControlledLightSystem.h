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
 |  AUTH:   Michael Zyracki
 |         
\*===========================================================================*/

#pragma once

#include "BaseInterface.h"
#include "Maxtypes.h"
#include "IDaylightSystem.h"

//! \brief Interface id for the Daylight Controlled Light System
#define DAYLIGHT_CONTROLLED_LIGHT_SYSTEM Interface_ID(0x257c2747, 0x78cf4e71)

//! \brief Interface for a Sun or Sky light that may be driven by the daylight system.
/*! This class represents an interface to a light object, in almost all cases a Sun or Sky light that's contained
 in IDaylightSystem, that have it's illuminance or irradiance values be driven by the daylight system.
 By returning this interface, the light object says that it may get told whether or not it
 should query the IDaylightSystem2 interface when it needs illuminance or irradiance values.
 \see IDaylightSystem2

\code 
Object* skylight = // obtain a pointer to an object
BaseInterface* interface = daylightControlled->GetInterface(DAYLIGHT_CONTROLLED_LIGHT_SYSTEM);
IDaylightControlledLightSystem * daylightControlled = dynamic_cast<IDaylightControlledLightSystem *>(interface);
\endcode
*/
#define IDCLS_ILLEGAL_VALUE		-9999.0f
class IDaylightControlledLightSystem : public BaseInterface {
public:

	//! \brief Set of parameters that may be fed by the daylight system to the controlled light.
	struct DaylightSimulationParams
	{
		float mDryBulbTemperature;
		float mDewPointTemperature;
		float mDirectNormalIrradiance;  
		float mDiffuseHorizontalIrradiance;  
		float mDirectNormalIlluminance;     
		float mDiffuseHorizontalIlluminance; 
		float mZenithLuminance;

	};

	//! \brief struct for passing in relevant values for a daylight
	//! \brief Returns the Interface ID of this interface
	virtual Interface_ID GetID() { return  DAYLIGHT_CONTROLLED_LIGHT_SYSTEM; };
	
	//! \brief A virtual destructor for the interface
	virtual ~IDaylightControlledLightSystem() {};

	//! \brief Whether or not it's controlled by the daylight system.
	//! \param[in] val  If true the light object is controlled by the daylight system and and should query the IDaylightSystem2 interface,
	//! in order to get any necessary illuminance,irradiance, temperature, etc. values.  \see IDaylightSystem2
	virtual void ControlledByDaylightSystem(IDaylightSystem2 *dls,bool val) =0;

	//! \brief Function to set simulation parameters that may be used by the controlled light.
	//! \param[in] params  Set of parameters that may be used by the controlled light to determine how it illuminates.
	//! Note that if any of the parameters have a value of IDCLS_ILLEGAL_VALUE then that parameter
	//! shouldn't be used in any calculations.
	virtual void SetSimulationParams(IDaylightControlledLightSystem::DaylightSimulationParams &params) =0;
};

