//*****************************************************************************/
// Copyright (c) 1997,1998 Discreet Logic, Inc.
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Discreet Logic and
// are protected by Federal copyright law. They may not be disclosed
// to third parties or copied or duplicated in any form, in whole or
// in part, without the prior written consent of Discreet Logic.
//*****************************************************************************/

#pragma once

/*==============================================================================
 * EXTERNAL DECLARATIONS
 *============================================================================*/
#include "maxtypes.h"
#include "genlight.h"
#include "custattrib.h"
/*==============================================================================
 * FORWARD DECLARATIONS
 *============================================================================*/
class ToneOperator;
class ShadowType;
class Interval;

//! \brief Photometric Lights class ID.
/*! All Photometric lights will return true to
	IsSubClassOf(LIGHTSCAPE_LIGHT_CLASS). This is how a programmer can
	determine whether casting a LightObject to LightscapeLight is valid. */
#define LIGHTSCAPE_LIGHT_CLASS Class_ID(0x30331006, 0x2454bd3)

//! \brief The Class ID for Photometric Free Point lights
#define LS_POINT_LIGHT_ID Class_ID(0x32375fcc, 0xb025cf0)
//! \brief The Class ID for Photometric Free Linear lights
#define LS_LINEAR_LIGHT_ID Class_ID(0x78207401, 0x357f1d58)
//! \brief The Class ID for Photometric Free Area lights
#define LS_AREA_LIGHT_ID Class_ID(0x36507d92, 0x105a1a47)
//! \brief The Class ID for Photometric Free Disc lights
#define LS_DISC_LIGHT_ID Class_ID(0x5bcc6d42, 0xc4f430e)
//! \brief The Class ID for Photometric Free Sphere lights
#define LS_SPHERE_LIGHT_ID Class_ID(0x7ca93582, 0x1abb6b32)
//! \brief The Class ID for Photometric Free Cylinder lights
#define LS_CYLINDER_LIGHT_ID Class_ID(0x46f634e3, 0xa327aaf)

//! \brief The Class ID for Photometric Target Point lights
#define LS_POINT_LIGHT_TARGET_ID Class_ID(0x658d4f97, 0x72cd4259)
//! \brief The Class ID for Photometric Target Linear lights
#define LS_LINEAR_LIGHT_TARGET_ID Class_ID(0x45076885, 0x40791449)
//! \brief The Class ID for Photometric Target Area lights
#define LS_AREA_LIGHT_TARGET_ID Class_ID(0x71794f9d, 0x70ae52f2)
//! \brief The Class ID for Photometric Target Disc lights
#define LS_DISC_LIGHT_TARGET_ID Class_ID(0x38732348, 0x201758b3)
//! \brief The Class ID for Photometric Target Sphere lights
#define LS_SPHERE_LIGHT_TARGET_ID Class_ID(0x33fc7ae9, 0x54433dc5)
//! \brief The Class ID for Photometric Target Cylinder lights
#define LS_CYLINDER_LIGHT_TARGET_ID Class_ID(0x7c8b5b10, 0x4bd33e86)

//! The Class ID for the Area Sampling custom attribute
#define LS_AREALIGHTCUSTATTRIB_CLASS_ID Class_ID(0x554c5e06, 0x325037bc)
//! \brief The Interface ID for the Area Sampling custom attribute interface
/*! \see LightscapeLight::AreaLightCustAttrib */
#define LS_AREALIGHTCUSTATTRIB_INTERFACE_ID Interface_ID(0x46aa0b9f, 0x64866ff3)

#define LIGHTSCAPE_LIGHT_INTENSITY_MIN 0.0f
#define LIGHTSCAPE_LIGHT_INTENSITY_MAX 1.0e7f

/*==============================================================================
 * CLASS LightscapeLight
 *============================================================================*/
#pragma warning(push)
#pragma warning(disable:4239)

//! \brief Common Photoemtric light

//! LightscapeLight is the base class for all Photometric lights. You can safely
//! cast an Animatble* to LightscapeLight* if IsSubClass(LIGHTSCAPE_LIGHT_CLASS)
//! returns true.

class LightscapeLight : public GenLight {

public: 

	/*----- constants -----*/

	/*----- types and enumerations ----*/

	//! Types for the lights. These values are used in SetType
	//! and returned by Type.
	enum LightTypes {
	  TYPE_BASE                  = 532,
	  TARGET_POINT_TYPE          = TYPE_BASE + 0,  //!< Target point light type
	  POINT_TYPE                 = TYPE_BASE + 1,  //!< Free point light type
	  TARGET_LINEAR_TYPE         = TYPE_BASE + 2,  //!< Target linear light type
	  LINEAR_TYPE                = TYPE_BASE + 3,  //!< Free linear light type
	  TARGET_AREA_TYPE           = TYPE_BASE + 4,  //!< Target area light type
	  AREA_TYPE                  = TYPE_BASE + 5,  //!< Free area light type
	  TARGET_DISC_TYPE           = TYPE_BASE + 6,  //!< Target disc light type
	  DISC_TYPE                  = TYPE_BASE + 7,  //!< Free disc light type
	  TARGET_SPHERE_TYPE         = TYPE_BASE + 8,  //!< Target sphere light type
	  SPHERE_TYPE                = TYPE_BASE + 9,  //!< Free sphere light type
	  TARGET_CYLINDER_TYPE       = TYPE_BASE + 10, //!< Target cylinder light type
	  CYLINDER_TYPE              = TYPE_BASE + 11, //!< Free cylinder light type
	  LAST_TYPE                  = CYLINDER_TYPE   //!< Last valid type
	};

	//! Types for the distributions. These values are used in SetDistribution
	//! and returned by GetDistribution.
	enum DistTypes {
	  ISOTROPIC_DIST             = 0,           //!< Isotropic distribution type
	  SPOTLIGHT_DIST             = 1,           //!< Spolight distribution type
	  DIFFUSE_DIST               = 2,           //!< Diffuse distribution type
	  WEB_DIST                   = 3,           //!< Web distribution type
	  LAST_DIST                  = WEB_DIST     //!< Last valid distribution type
	};

	//! IDs for the references kept by this object
	enum ReferenceIDs {
	  //! Reference to the common parameters
	  REF_PARAMS                 = 0,
	  //! Reference to the shadow parameters
	  REF_SHADOW_PARAMS          = 1,
	  //! Reference to the spotlight distribution parameters
	  REF_SPOT_PARAMS            = 2,
	  //! Reference to the web distribution parameters
	  REF_WEB_PARAMS             = 3,
	  //! Reference to the shadow generator
	  REF_SHADOWGEN              = 4,
	  //! Reference to the extended parameters
	  REF_EXT_PARAMS             = 5,
	  //! The last reference ID
	  LAST_REF                   = REF_EXT_PARAMS
	};

	//! IDs for the parameters blocks for Lightscape lights
	enum ParameterBlockID {
	  //! Parameter block ID for the common parameters
	  PB_GENERAL                 = 0,
	  //! Parameter block ID for the shadow parameters
	  PB_SHADOW                  = 1,
	  //! Parameter block ID for the spotlight distribution parameters
	  PB_SPOT                    = 2,
	  //! Parameter block ID for the web distribution parameters
	  PB_WEB                     = 3,
	  //! Parameter block ID for the extended parameters
	  PB_EXT                     = 4,
	  //! Last parameter block ID
	  LAST_PB                    = PB_EXT
	};

	//! This enum is used to hold the General parameter IDs
	enum ParameterIDs {
	  //! The light distribution type
	  PB_DIST_TYPE               = 1,
	  //! Use the light
	  PB_USE_LIGHT               = 2,
	  //! Cast shadows
	  PB_CAST_SHADOWS            = 3,
	  //! Light Color
	  PB_LIGHT_COLOR             = 4,
	  //! Filter Color
	  PB_FILTER_COLOR            = 5,
	  //! Intensity
	  PB_INTENSITY               = 6,
	  //! Light color in degrees kelvin
	  PB_KELVIN                  = 8,
	  //! Flag to use Kelvin or RGB
	  PB_USE_KELVIN              = 10,
	  //! Intensity At
	  PB_INTENSITY_AT            = 13,
	  //! Intensity Type
	  PB_INTENSITY_TYPE          = 14,
	  //! Flux
	  PB_FLUX                    = 15,
	  //! Original Intensity
	  PB_ORIGINAL_INTENSITY      = 16,
	  //! Original Flux
	  PB_ORIGINAL_FLUX           = 17,
	  //! use the dimmer slider/spinner
	  PB_USE_DIMMER				  = 18,
	  //! Dimmer/Multiplier
	  PB_DIMMER						  = 19,
	  //! Lamp color shift on dimming
	  PB_COLORSHIFT_DIMMING		  = 20,
	  //! use the far attenuation
	  PB_USE_FARATTENUATION		  = 21,
	  //! display the far attenuation when unselected
	  PB_SHOW_FARATTENUATION_GIZMO = 22,
	  //! start far attenuation value
	  PB_START_FARATTENUATION	  = 23,
	  //! end far attenuation value
	  PB_END_FARATTENUATION      = 24,
	  //! Last id in list
	  LAST_GEN_PB                = PB_END_FARATTENUATION
	};

	//! This enum is used to hold the Extended parameter IDs
	enum ExtParamIDs {
	  //! Contrast between diffuse and ambient
	  PB_CONTRAST                = 0,
	  //! Softening between diffuse and ambient
	  PB_DIFFSOFT                = 1,
	  //! Use projector map
	  PB_PROJECTOR               = 2,
	  //! The projector map
	  PB_PROJECTOR_MAP           = 3,
	  //! Affect diffuse channel
	  PB_AFFECT_DIFFUSE          = 4,
	  //! Affect specular channel
	  PB_AFFECT_SPECULAR         = 5,
	  //! Ambient only
	  PB_AMBIENT_ONLY            = 6,
	  //! Target distance
	  PB_TDIST                   = 7,
//		PB_LENGTH						= 8,
//		PB_WIDTH							= 9,  
//		PB_GARBAGE						= 10,
//		WARNING:  DO NOT REDEFINE ABOVE IDs.  THIS MAY CAUSE A CRASH WHEN LOADING OLD
//					 FILES
//    Change made August 27, 2001 David Cunningham
//    for B24 of VIZ R4
		//! Line Light length
		PB_LINELIGHT_LENGTH				= 11,
		//! Area Light length (Rectangle)
		PB_AREALIGHT_LENGTH				= 11,
		//! Area light height (Rectangle)
		PB_AREALIGHT_WIDTH				= 12, 
		//! Disc light radius
		PB_DISCLIGHT_RADIUS				= 13,
		//! Sphere light radius
		PB_SPHERELIGHT_RADIUS_OLD		= 14, // obsolete, use PB_SPHERELIGHT_RADIUS instead
		PB_SPHERELIGHT_RADIUS			= PB_DISCLIGHT_RADIUS,
		//! Cylinder light radius
		PB_CYLINDERLIGHT_RADIUS_OLD		= 15, // obsolete, use PB_CYLINDERLIGHT_RADIUS instead
		PB_CYLINDERLIGHT_RADIUS			= PB_DISCLIGHT_RADIUS,
		//! Cylinder light length
		PB_CYLINDERLIGHT_LENGTH_OLD		= 16, // obsolete, use PB_CYLINDERLIGHT_LENGTH instead
		PB_CYLINDERLIGHT_LENGTH			= PB_LINELIGHT_LENGTH,
	  //! Last id in list
	  LAST_EXT_PB                = PB_CYLINDERLIGHT_LENGTH_OLD
	};

	//! This enum is used to hold the Shadow parameter IDs
	enum ShadowIDs {
	  //! Atmospheric shadows
	  PB_ATMOS_SHAD              = 0,
	  //! Atmosphere opacity
	  PB_ATMOS_OPACITY           = 1,
	  //! Atmosphere color influence
	  PB_ATMOS_COLAMT            = 2,
	  //! Shadow Density
	  PB_SHADMULT                = 3,
	  //! Shadow Color Map Enabled
	  PB_SHAD_COLMAP             = 4,
	  //! Shadow color
	  PB_SHADCOLOR               = 5,
	  //! Light affects shadow color
	  PB_LIGHT_AFFECTS_SHADOW    = 6,
	  //! Use global shadow generator
	  PB_USE_GLOBAL_PARAMS       = 7,
	  //! Shadow Projection map
	  PB_SHAD_PROJ_MAP           = 8,
	  //! Last shadow id
	  LAST_SHAD_PB               = PB_SHAD_PROJ_MAP
	};

	//! This enum is used to hold the Spotlight parameter IDs
	enum SpotlightIDs {
	  //! Beam angle
	  PB_BEAM_ANGLE              = 0,
	  //! Field angle
	  PB_FIELD_ANGLE             = 1,
	  //! Display the spot light cone
	  PB_CONE_DISPLAY            = 2,
	  //! Last spotlight distribution id
	  LAST_SPOT_PB               = PB_CONE_DISPLAY
	};

	//! This enum is used to hold the Photometric Web parameter IDs
	enum DistributionIDs {
	  //! The name of the webfile
	  PB_WEB_FILE_NAME           = 0,
	  //! Rotate X
	  PB_WEB_ROTATE_X            = 1,
	  //! Rotate Y
	  PB_WEB_ROTATE_Y            = 2,
	  //! Rotate Z
	  PB_WEB_ROTATE_Z            = 3,
	  //! Last web distribution id
	  LAST_WEB_PB               = PB_WEB_ROTATE_Z
	};

	//! This enum is used to hold the intensity types.
	//! These are valid values for the Intensity Type parameter.
	enum IntensityType {
	  //! Intensity is in lumens
	  LUMENS                     = 0,
	  //! Intensity is in candelas
	  CANDELAS                   = 1,
	  //! Intensity is in Lux at distance
	  LUX_AT                     =2,
	};

	//! Shadow types for MAX
	enum MaxShadowType {
	  //! No Shadow Generator
	  NO_SHADOW_GENERATOR        = -1,
	  //! Bitmap Shadows
	  BITMAP_SHADOWS             = 0,
	  //! RayTraceShadows
	  RAYTRACE_SHADOWS           = 1,
	  //! Other shadow generator
	  OTHER_SHADOWS              = 0xffff
	};

	/*----- classes -----*/

	class AreaLightCustAttrib;

	/*----- static member functions -----*/

	//! \brief Returns the AreaLightCustAttrib interface for a custom attribute
	/*! \pre attrib is non-null
		\param[in] attrib - The custom attribute whose AreaLightCustAttrib interface is desired.
		\return The AreaLightCustAttrib interface to attrib, or NULL if
		attrib doesn't support the interface. */
	static AreaLightCustAttrib* GetAreaLightCustAttrib(
	  CustAttrib* attrib
	);

	/*----- member functions -----*/

	//@{
	//! \brief Sets the light type.
	/*! \post The light type is changed. When setting the light type to an invaild value,
		nothing is changed, and no error is returned.
		\param[in] type - The type id the light is to be changed to. Must be a value in
		the LightTypes enum
		\see LightscapeLight::LightTypes */
	virtual void SetType( int type ) = 0;
	//! \brief Sets the light type.
	/*! \param[in] name - The type name the light is to be changed to. May not be NULL.
		The valid strings are:
		\li "Target_Point"
		\li "Free_Point"
		\li "Target_Linear"
		\li "Free_Linear"
		\li "Target_Area"
		\li "Free_Area"
		\li "Target_Disc"
		\li "Free_Disc"
		\li "Target_Sphere"
		\li "Free_Sphere"
		\li "Target_Cylinder"
		\li "Free_Cylinder"
		\post The light type is changed or returned. When setting the light
		type to an invaild value, nothing is changed, and no error is returned. */
	virtual void SetType( const MCHAR* name ) = 0;
	//! \brief Returns the current light type
	/*! \return The current light type, as a value from the LightTypes enum
		\see LightscapeLight::LightTypes */
	virtual int Type( ) = 0;
	//! \brief Returns the light type.
	/*! \return The current light type, as a string */
	virtual const MCHAR* TypeName( ) = 0;
	//@}

	//@{
	//! \brief Sets the distribution of the light.
	/*! \post The distribution is changed
		\param[in] dist - The ID distribution that the light is to use. Must be a value
		from DistTypes. */
	virtual void SetDistribution( DistTypes dist ) = 0;
	//! \brief Returns the distribution of the light.
	/*! \return The distribution ID of the current distribution. */
	virtual DistTypes GetDistribution( ) const = 0;
	//@}

	//@{
	//! \brief Sets the distance at which the light intensity is measured.
	/*! This distance is used then the light intensity type is LUX_AT.
		\post The distance is set
		\param[in] f - The distance at which the light intensity is measured in system units.
		Must be greater than 0. */
	virtual void SetIntensityAt( float f ) = 0;
	//! \brief Returns the distance at which the light intensity is measured.
	/*! This distance is used then the light intensity type is LUX_AT.
		\return The distance is returned. */
	virtual float GetIntensityAt( ) = 0;
	//@}

	//@{
	//! \brief Sets the intensity type of the light.
	/*! \post The intensity type is set
		\param[in] t - The new intensity type of the light. Must be a value from IntensityType.
		\see LightscapeLight::IntensityType. */
	virtual void SetIntensityType( IntensityType t ) = 0;
	//! \brief Returns the intensity type of the light.
	/*! \see LightscapeLight::IntensityType. */
	virtual IntensityType GetIntensityType( ) = 0;
	//@}

	//@{
	//! \brief Sets the flux of the light.
	/*! The flux of a light is a measure of how much light the light produces. It is
		related to the intensity by a constant factor that depends on the distribution
		of the light. For example the flux of an isotropic light is always 4PI times
		the intensity (4PI is the area of a unit sphere).
		\post The flux of the light is set, which changes the intensity.
		\param[in] t - The time at which the flux is set.
		\param[in] flux - The flux to be set in lumens. Must be greater than or equal to 0. */
	virtual void SetFlux(
	  TimeValue t,
	  float     flux
	) = 0;
	//! \brief Returns the flux of the light.
	/*! \param[in] t - The time at which the flux is retrieved.
		\param[in,out] valid - The validity of the flux is intersected with the validity in this argument.
		\return The flux is returned in lumens.
		\see For an explanation for flux, see LightscapeLight::SetFlux() */
	virtual float GetFlux(
	  TimeValue t,
	  Interval &valid = Interval( 0,0 )
	) const = 0;
	//@}

	//@{
	//! \brief Sets the color of the light filter.
	/*! Lightscape lights use separate values for the light color and filter color.
		You should imagine that the light color is the color of the light bulb, and
		the filter color is the color of the surrounding piece of glass or plastic.
		The effective color of the light is the product of the filter and light colors.
		The color can be set in either RGB or HSV format.
		\post The filter color is changed or returned.
		\param[in] t - The time at which the filter color is set.
		\param[in] rgb - The RGB color to be set */
	virtual void SetRGBFilter(
	  TimeValue t,
	  Point3& rgb
	) = 0;
	//! \brief Returns the color of the light filter.
	/*! \param[in] t - The time at which the filter color is retreived.
		\param[in,out] valid - The validity of the filter color is intersected with the validity in this argument.
		\return The filter color is returned in either RGB format.
		\see For an explanation of the light filter, see LightscapeLight::SetRBGFilter() */
	virtual Point3 GetRGBFilter(
	  TimeValue t,
	  Interval &valid = Interval( 0,0 )
	) = 0;
	//! \brief Sets the color of the light filter.
	/*! \post The filter color is changed or returned.
		\param[in] t - The time at which the filter color is set.
		\param[in] hsv - The HSV color to be set
		\see For an explanation of the light filter, see LightscapeLight::SetRBGFilter() */
	virtual void SetHSVFilter(
	  TimeValue t,
	  Point3& hsv
	) = 0;
	//! \brief Returns the color of the light filter.
	/*! \param[in] t - The time at which the filter color is retrieved.
		\param[in,out] valid - The validity of the filter color is intersected with the validity in this argument.
		\return The filter color is returned in either HSV format.
		\see For an explanation of the light filter, see LightscapeLight::SetRBGFilter() */
	virtual Point3 GetHSVFilter(
	  TimeValue t,
	  Interval &valid = Interval( 0,0 )
	) = 0;
	//@}


	//@{
	//! \brief Returns the plug-in shadow generator
	/*! There are two shadow generators that can be used by a light. The local
		shadow generator that is referenced by the light and the global shadow generator
		that is in the scene. The LightObject methods GetUseGlobal() and SetUseGlobal() access
		the parameter that determines which shadow generator is actually used.
		GetShadowGenerator() always returns the local shadow generator, while other methods
		return and set the shadow generator based on the value of the use global parameter.
		\return The shadow generator is returned. */
	virtual ShadowType* ActiveShadowType( ) = 0;
	//! \brief Returns the plug-in shadow generator
	/*! \return The local shadow generator. Always returns the local shadow generator, not the global generator.
		\see For an explanation of local and global shadow generators, see LightscapeLight::ActiveShadowType() */
	virtual ShadowType* GetShadowGenerator( ) = 0;
	//! \brief Returns the plug-in shadow generator name
	/*! \return The local or global shadow generator name, depending on the value of the use global parameter.
		\see For an explanation of local and global shadow generators, see LightscapeLight::ActiveShadowType() */
	virtual const MCHAR* GetShadowGeneratorName( ) = 0;
	//! \brief Sets the plug-in shadow generator
	/*! \post The shadow generator is set.
		\param[in] s - An instance of the shadow type to be set. The light establishes a reference
		to this object.
		\see For an explanation of local and global shadow generators, see LightscapeLight::ActiveShadowType() */
	virtual void SetShadowGenerator( ShadowType* s ) = 0;
	//! \brief Sets the plug-in shadow generator
	/*! \post The shadow generator is set.
		\param[in] name - The class name of the shadow type to be set.
		\see For an explanation of local and global shadow generators, see LightscapeLight::ActiveShadowType() */
	virtual void SetShadowGenerator( const MCHAR* name ) = 0;
	//@}

	//@{
	//! \brief Sets whether a shadow color map is used by the light
	/*! \post The flag value is set.
		\param[in] t - The time at which the flag value is set.
		\param[in] onOff - The value to be set. TRUE indicates the the light should use a shadow color map. */
	virtual void SetUseShadowColorMap(
	  TimeValue t,
	  int onOff
	) = 0;
	//! \brief Returns whether a shadow color map is used by the light
	/*! \param[in] t - The time at which the flag value is set.
		\return The flag value is returned. */
	virtual int GetUseShadowColorMap( TimeValue t ) = 0;
	//@}

	//! \brief Sets whether the Include/Exclude list should include or exclude objects.
	/*! \post The flag value is set.
		\param[in] onOff - The flag value to be set. TRUE indicates that the light should include objects
		in the Include/Exclude list.*/
	virtual void SetInclude( BOOL onOff ) = 0;

	//! \brief Update the target distance parameter of the light to the distance between
	//! the light target and the light.
	/*! \pre The inode should have a target. Nothing is changed if no target is present,
		however, the method does not check whether this light is attached to the inode argument.
		\post The distance is updated.
		\param[in] t - The time at which the distance is calculated and set.
		\param[in] inode - The node to be used to calculate the target. */
	virtual void UpdateTargDistance(
	  TimeValue t,
	  INode* inode
	) = 0;

	//@{
	//! \brief Sets the controller for the Kelvin temperature.
	/*! \post The controller is set.
		\param[in] kelvin - The new controller to use for kelvin temperature. NULL will remove any animation. */
	virtual BOOL SetKelvinControl( Control *kelvin ) = 0;
	//! \brief Sets the controller for the filter color.
	/*! \post The controller is set.
		\param[in] filter - The new controller to use for filter color. NULL will remove any animation. */
	virtual BOOL SetFilterControl( Control *filter ) = 0;
	//! \brief Returns the controller for the Kelvin temperature.
	/*! \return The controller is returned. */
	virtual Control* GetKelvinControl( ) = 0;
	//! \brief Returns the controller for the filter color.
	/*! \return The controller is returned. */
	virtual Control* GetFilterControl( ) = 0;
	//@}

	//@{
	//! \brief Returns the Kelvin temperature of the light
	/*! Kelvin temperature is a method for retreiving light color based on black
		body radiation from physics.
		\param[in] t - The time at which the color is retrieved.
		\param[in,out] v - The validity of the color is intersected with this argument.
		\return The current kelvin temperature. */
	virtual float GetKelvin(
	  TimeValue t,
	  Interval& v = Interval(0,0)
	) = 0;
	//! \brief Sets the Kelvin temperature of the light
	/*! Kelvin temperature is a method for specifying light color based on black
		body radiation from physics.
		\post The kelvin temperature is changed.
		\param[in] t - The time at which the color is set.
		\param[in] kelvin - The kelvin temperature in degrees Kelvin. Should be greater than or equal to 0. */
	virtual void SetKelvin(
	  TimeValue t,
	  float kelvin
	) = 0;
	//@}

	//@{
	//! \brief Returns whether the kelvin temperature or light color is to be used
	/*! The light color can be set using either an RGB value or a kelvin temperature.
		\return TRUE if the kelvin temperature is used, or FALSE if the RBG value is used. */
	virtual BOOL GetUseKelvin( ) = 0;
	//! \brief Sets whether the kelvin temperature or light color is to be used
	/*! The light color can be set using either an RGB value or a kelvin temperature.
		\post The flag value is changed or returned.
		\param[in] useKelvin - TRUE to use the kelvin temperature, or FALSE to use the RGB value. */
	virtual void SetUseKelvin( BOOL useKelvin ) = 0;
	//@}

	//@{
	//! \brief Returns the web distribution file for the light as an AssetUser.
	/*! \return The AssetUser is returned. */
	virtual MaxSDK::AssetManagement::AssetUser GetWebFile( ) const = 0;
	//! \brief Returns the filename of the web distribution file for the light.
	/*! The full web filename is the full path to the web filename. If the
		file couldn't be found or the current distribution is not the web distribution,
		the filename stored in the light is returned.
		\return The filename is returned. */
	virtual const MCHAR* GetFullWebFileName( ) const = 0;
	//! \brief Sets the web distribution file for the light.
	/*! \post The file is set
		\param[in] file - The new file for the web distribution */
	virtual void SetWebFile( const MaxSDK::AssetManagement::AssetUser& file ) = 0;
	//@}

	//@{
	//! \brief Returns the rotation along the X axis of the web distribution in the light.
	/*! \return The rotation in degrees is returned. */
	virtual float GetWebRotateX( ) const = 0;
	//! \brief Sets the rotation along the X axis for the web distribution in the light.
	/*! \post The rotation is set.
		\param[in] degrees - The amount of rotation in degrees. */
	virtual void SetWebRotateX( float degrees ) = 0;
	//! \brief Returns the rotation along the Y axis of the web distribution in the light.
	/*! \return The rotation in degrees is returned. */
	virtual float GetWebRotateY( ) const = 0;
	//! \brief Sets the rotation along the Y axis for the web distribution in the light.
	/*! \post The rotation is set.
		\param[in] degrees - The amount of rotation in degrees. */
	virtual void SetWebRotateY( float degrees ) = 0;
	//! \brief Returns the rotation along the Z axis of the web distribution in the light.
	/*! \return The rotation in degrees is returned. */
	virtual float GetWebRotateZ( ) const = 0;
	//! \brief Sets the rotation along the Z axis for the web distribution in the light.
	/*! \post The rotation is set.
		\param[in] degrees - The amount of rotation in degrees. */
	virtual void SetWebRotateZ( float degrees ) = 0;
	//@}

	//@{
	//! \brief Returns the dimmer value for the light.
	/*! An additional dimmer can be used to independently modify the light intensity.
		\param[in] t - The time at which the dimmer value is retrieved.
		\param[in,out] valid - The validity of the dimmer is intersected with the validity in this argument.
		\return the dimmer value is returned. */
	virtual float GetDimmerValue(TimeValue t, Interval &valid = Interval( 0,0 )) const = 0;
	//! \brief Sets the dimmer value for the light.
	/*! An additional dimmer can be used to independently modify the light intensity.
		\post The dimmer value is set.
		\param[in] t - The time at which the dimmer value is set.
		\param[in] dimmer - The value of the dimmer. 1.0 is full-on. 0.0 if full-off. */
	virtual void SetDimmerValue(TimeValue t, float dimmer) = 0;
	//@}

	//@{
	//! \brief Returns whether the dimmer is used
	/*! \return The value of the flag is returned. */
	virtual BOOL GetUseMultiplier() const = 0;
	//! \brief Sets whether the dimmer is used
	/*! \post The value of the flag is changed.
		\param[in] on - The value of the flag to be set. TRUE indicates that the dimmer is used. */
	virtual void SetUseMultiplier(BOOL on) = 0;
	//@}

	//@{
	//! \brief Returns whether the light's intensity shifts (affects) its color.
	/*! Note that this parameter is really activated only if GetUseMultiplier() is true.
		\return The value of the flag is returned. */
	virtual BOOL IsColorShiftEnabled() const = 0;

	//! \brief Sets whether the color of the light shifts when its intensity changes
	//! Note that this parameter is really activated only if GetUseMultiplier() is true.
	/*! \post The value of the flag is changed.
	\param[in] on - If true, the changes in the light's intensity will cause shifts in the light's color, otherwise the light's color will be unaffected by the intensity*/
	virtual void EnableColorShift(BOOL on) = 0;
	//@}

	//@{
	//! \brief Returns the intensity including the dimmer multiplier if it is used.
	/*! This utility method returns the effective intensity of a light including the dimmer value if there is one.
		\param[in] t - The time at which to retrieve the intensity or flux.
		\param[in,out] valid - The validity of the effective intensity is intersected with this argument.
		This includes the validity of the dimmer.
		\return The effective intensity in candelas. */
	virtual float GetResultingIntensity(TimeValue t, Interval &valid = Interval( 0,0 )) const = 0;
	//! \brief Returns the flux including the dimmer multiplier if it is used.
	/*! This utility method returns the effective flux of a light including the dimmer value if there is one.
		\param[in] t - The time at which to retrieve the intensity or flux.
		\param[in,out] valid - The validity of the effective flux is intersected with this argument.
		This includes the validity of the dimmer.
		\return The effective flux in lumens. */
	virtual float GetResultingFlux(TimeValue t, Interval &valid = Interval( 0,0 )) const = 0;
	//@}

	//! \brief Returns the location of the center of the light. 
	/*! This location is in the light's coordinates.
		\return The light coordinate of the center of the light. */
	virtual Point3 GetCenter( ) const = 0;

	//@{
	//! \brief Returns the radius of a disc, sphere, cylinder light.
	/*! This method allows to access the radius of a disc, sphere or cylinder light type.
		\param[in] t - The time at which the radius is retrieved.
		\param[in,out] valid - The validity interval of the radius is intersected with this argument.
		\return The radius of the light, or zero if the light is not of any of the above mentioned types.  */
	virtual float GetRadius(TimeValue t, Interval &valid = Interval(0,0)) const = 0;

	//! \brief Sets the radius of a disc, sphere, cylinder light.
	/*! This method allows to set the radius of a disc, sphere or cylinder light.
		If the light is not of one of the above mentioned types, the method is a no-op.
		\param[in] t - The time at which the radius is set.
		\param[in] radius - The new radius value*/
	virtual void SetRadius(TimeValue t, float radius) = 0;
	//@}

	//@{
	//! \brief Returns the length of a linear, area, cylinder light. 
	/*! This method allows to access the length of a linear, area, or cylinder light.
		\param[in] t - The time at which the length is retrieved.
		\param[in,out] valid - The validity interval of the length is intersected with this argument.
		\return The length of the light, or zero if the light is not of any of the above mentioned types.  */
	virtual float GetLength(TimeValue t, Interval &valid = Interval(0,0)) const = 0;

	//! \brief Sets the length of a linear, area, cylinder light.
	/*! This method allow to set the length of a linear, area, cylinder light.
		If the light is not of one of the above mentioned types, the method is a no-op.
		\param[in] t - The time at which the length is set.
		\param[in] length - The new length value*/
	virtual void SetLength(TimeValue t, float length) = 0;
	//@}

	//@{
	//! \brief Returns the width of an area light. 
	/*! This method allows to access the width of an area light. 
		\param[in] t - The time at which the width is retrieved.
		\param[in,out] valid - The validity interval of the width is intersected with this argument.
		\return The width of the light, or zero if the light is not of any of the above mentioned types.  */
	virtual float GetWidth(TimeValue t, Interval &valid = Interval(0,0)) const = 0;

	//! \brief Sets the width of an area light. 
	/*! This method allows to set the width of an area light. 
		If the light is not of one of the above mentioned types, the method is a no-op.
		\param[in] t - The time at which the width is set.
		\param[in] width - The new width value*/
	virtual void SetWidth(TimeValue t, float width) = 0;
	//@}


	//@{
	//! \brief Sets the coordinates of the light.
	/*! The use of this functions varies based on the current light type.
		If the light is a point light, the center is
		set to the center of the set of points. If the light is a linear light the light's
		line segment is set to the segment between the first two points. If fewer than
		two points are specified, no change is made. If the light is an area light,
		the light area is set to the triangle or quadrilater defined by the 3 or 4 points,
		respectively. If fewer than 3 points is specified, no change is made. If more than
		4 points is specified a quadrliateral is made from the first 4 points.
		\pre All points are in light coordinates. Quadrilaterals must be planar.
		\post The shape is changed.
		\param[in] count - Number of points of the shape.
		\param[in] pointsIn - The array of points in the shape. */
	virtual void SetShape(
	  int            count,
	  const Point3*  pointsIn
	) = 0;
	//! \brief Returns the coordinates of the light.
	/*! \param[out] - pointsOut The buffer that will receive the vertices in the shape. If bufSize is
		0 this can be NULL.
		\param[in] bufSize - The number of points in the buffer. Only the vertices that fit in the
		buffer will be stored. The return value will include the count of
		vertices that could not be stored.
		\return The total number of vertices in the shape. This is independant of the
		number of point in the receiving buffer.
		\see For further usage notes, see LightscapeLight::SetShape() */
	virtual int GetShape(
	  Point3*  pointsOut,
	  int      bufSize
	) const = 0;
	//@}

	//@{
	//! \brief Returns the original flux for a light.
	/*! This is the original flux in the web distribution for a light.
		\return The original flux in lumens is returned. */
	virtual float GetOriginalFlux( ) const = 0;
	//! \brief Sets the original flux for a light.
	/*! This is the original flux in the web distribution for a light.
		It can be used to reset a light intensity to a default value.
		The method can be used, but the flux will be reset if the web filename
		is changed.
		\post The original flux is set
		\param[in] flux - The flux to be set, in lumens. */
	virtual void SetOriginalFlux( float flux ) = 0;
	//@}

	//@{
	//! \brief Returns the original intensity for a light.
	/*! This is the original intensity in the web distribution for a light.
		\return The original intensity in candelas is returned. */
	virtual float GetOriginalIntensity( ) const = 0;
	//! \brief Sets the original intensity for a light.
	/*! This is the original intensity in the web distribution for a light.
		It can be used to reset a light intensity to a default value.
		The method can be used, but the intensity will be reset if the web filename
		is changed.
		\post The original intensity is set.
		\param[in] candelas - The intensity to be set, in candelas */
	virtual void SetOriginalIntensity( float candelas ) = 0;
	//@}
};

#pragma warning(pop)

//! \brief Area Lights sampling custom attribute
/*! This class holds the sampling information and additional
	 information for area lights.
*/
class LightscapeLight::AreaLightCustAttrib : public BaseInterface {

public:

	//! Default number of samples
	enum {
		kDefaultSamples_Area		 = 32,							//!< Default num samples for area  lights
		kDefaultSamples_Linear   = kDefaultSamples_Area,	//!< Default num samples for linear lights
		kDefaultSamples_Cylinder = kDefaultSamples_Area,	//!< Default num samples for cylinder lights
		kDefaultSamples_Sphere   = kDefaultSamples_Area,	//!< Default num samples for sphere lights
		kDefaultSamples_Disc     = kDefaultSamples_Area,	//!< Default num samples for disc lights
		kDefaultSamples_Point    = 0   							//!< Default num samples for point lights
	};

	//! Default value of the area light computation state
	static const BOOL kDefault_EnableAreaLight = TRUE;

	//! Default value of the light shape rendering state
	static const BOOL kDefault_EnableLightShapeRendering = FALSE;

	//! \brief Returns whether the area light computation is enabled.
	/*! \param[in] t - The time at which the value is retrieved
		\param[in,out] valid - If this argument is not NULL, the validity of this value is intersected
		 with the interval pointed to by this argument.
		\return always the value kDefault_EnableAreaLight */
	virtual bool AreaLightComputationEnabled(TimeValue t, Interval* valid = NULL) const = 0;

	//! \brief Returns whether the light's shape is rendered
	/*! \param[in] t - The time at which the value is retrieved
	\param[in,out] valid - If this argument is not NULL, the validity of this value is intersected
	with the interval pointed to by this argument.
	\return TRUE if the light shape is rendered */
	virtual BOOL IsLightShapeRenderingEnabled(TimeValue t, Interval* valid = NULL) const = 0;

	//! \brief Set whether the light's shape is rendered
	/*! \param[in] t - The time at which the value is retrieved
	\param[in] on - Render or not the light shape in mental ray
	\return TRUE if the light shape is rendered */
	virtual void LightShapeRenderingEnabled(TimeValue t, BOOL on) = 0;

	//@{
	//! \brief Returns the number of samples to be used
	/*! \post The number of samples should be a power of 2
		\param[in] t - The time at which the number of samples is retrieved.
		\param[in,out] valid - If this argument is not NULL, the validity of this number of samples is intersected
		 with the interval pointed to by this argument.
		\return The number of samples */
	virtual int GetNumSamples(TimeValue t, Interval* valid = NULL) const = 0;
	//! \brief Sets the number of samples to be used
	/*! \pre The number of samples should be a power of 2
		\param[in] t - The time at which the number of samples is set.
		\param[in] numSamples - The number of sample to use. */
	virtual void SetNumSamples(TimeValue t, int numSamples) = 0;
	//@}

	// -- from BaseInterface
	//! \brief Returns the Interface ID for this class, LS_AREALIGHTCUSTATTRIB_INTERFACE_ID
	virtual Interface_ID GetID() { return LS_AREALIGHTCUSTATTRIB_INTERFACE_ID; }   
};

//! \brief Returns the AreaLightCustAttrib interface for a custom attribute
/*! Returns NULL if the interface is not present. */
inline LightscapeLight::AreaLightCustAttrib*
LightscapeLight::GetAreaLightCustAttrib(
	CustAttrib* attrib
)
{
	return attrib == NULL ? NULL
		: static_cast<LightscapeLight::AreaLightCustAttrib*>(
			attrib->GetInterface(LS_AREALIGHTCUSTATTRIB_INTERFACE_ID));
}


