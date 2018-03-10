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

#pragma once

#include "..\baseinterface.h"
#include "..\maxtypes.h"
#include "..\acolor.h"
// forward declarations
class Interval;
class Texmap;

#define PORTAL_LIGHT_OBJECT_CLASSID	Class_ID(0x335c655c, 0x382151b1)
#define IID_MR_SKY_PORTAL_LIGHT Interface_ID(0x102804f9, 0x41d71b3f)


/*! \brief Access interface for the Mental Ray Sky Portal.
 *
 * This class represents the Mental Ray Sky Portal object, implemented by 3ds Max.
 * For more information on the Sky Portal object see the 3ds Max User Guide.
 * SDK client code can access this interface by querying a 3d Max light object for 
 * this interface:
 * \code 
 * Object* obj = // obtain a pointer to an object
 * BaseInterface* bi = obj->GetInterface(IID_MR_SKY_PORTAL_LIGHT);
 * IMrSkyPortalLight* mrSkyPortal = dynamic_cast<IMrSkyPortalLight*>(bi);
 * \endcode
*/
class IMrSkyPortalLight : public BaseInterface {
public:
	//! \brief Returns the Interface ID of this interface
	virtual Interface_ID GetID() { return IID_MR_SKY_PORTAL_LIGHT; };
	
	//! \brief A virtual destructor for the interface
	virtual ~IMrSkyPortalLight() {};

	//! \name Multiplier
	//! \brief The multiplier for the value of the sky portal's output.
	//@{
	/*! \brief Gets the value of the multiplier for the sky portal's output.
	 * \param[in] t - The time at which the multiplier's value is retrieved
	 * \param[in] valid - The time interval for which the multiplier value is valid
	 *  * \return	The value of the multiplier
	*/
	virtual float GetMultiplier(const TimeValue t, Interval& valid) const = 0;

	/*! \brief Sets the value of the multiplier for the sky portal's output.
	 * \param[in] t - The time at which the multiplier value is set
	 * \param[in] multiplier - The new value of multiplier
	 * \return true if the new value has been set successfully
	*/
	virtual bool SetMultiplier(const TimeValue t, float multiplier) = 0;
	//@}

	//! \name Filter Color
	//! \brief Tints the coloring of the light going through a sky portal 
	//@{
	/*! \brief Gets the filter color of the sky portal
	 * \param[in] t - The time at which the color is retrieved
	 * \param[in, out] valid - The time interval for which the color value is valid
	 * \return	The value of the filter color
	*/
	virtual AColor GetFilterColor(const TimeValue t, Interval &valid) const = 0;

	/*! \brief Sets the filter color of the sky portal
	 * \param[in] t - The time at which the color is set
	 * \param[in] color - The new color value 
	 * \return true if setting the new color was successful, false otherwise
	*/
	virtual bool SetFilterColor(const TimeValue t, const AColor& color) = 0;
	//@}

	//! \name Shadow Casting
	/*! \brief Methods for accessing shadow casting related parameters
	For basic shadow casting related functionality see class LightObject */
	//@{
	/*! \brief Gets whether shadows are casts from objects that are "outdoors" or 
	 * "outside" of the sky portal object. 
	 *
	 * The "outside" represents the side that 
	 * is away from the sky portal's arrow icon.
	 * \return	True if the sky portal casts shadows from "outdoor, false otherwise
	*/
	virtual bool GetCastShadowsFromOutdoors() const = 0;

	/*! \brief Sets whether shadows are casts from objects that are "outdoors" or 
	 * "outside" of the sky portal object. 
	 * 
	 * The "outside" represents the side that is away from the sky portal's arrow icon.
	 * Note that turning this option on can significantly increase render times.
	 * \param[in] bEnable - The new parameter value 
	 * \return true if setting the new parameter value was successful, false otherwise
	*/
	virtual bool SetCastShadowsFromOutdoors(bool bEnable) = 0;
	//@}

	//! \name Shadow Samples
	/*! \brief Affects the quality of the shadows cast by the light portal.
	 * Increase the value of this parameter if the shadows are grainy */
	//@{
	/*! \brief Gets the number of shadow samples.
	 * \param[in] t - The time at which the parameter is retrieved
	 * \param[in, out] valid - The time interval for which the parameter value is valid
	 * \return	The number of samples being used for shadows, as an exponent of 2.
	 * For example, if this method returns 10, the actual number of shadow samples used is
	 * 2 at the power of 10 = 1024.
	*/
	virtual int GetShadowSamples(const TimeValue t, Interval &valid) const = 0;

	/*! \brief Sets the number of shadow samples.
	 * \param[in] t - The time at which the parameter is set
	 * \param[in] shadowSamples - The new parameter value. 2 raised to the power of
	 * this value represents the actual number of shadow samples. Must be a value 
	 * within [1, 10]. For example, if 10 is set via this method, the actual 
	 * number of shadow samples will be 1024.
	 * \return true if setting the new parameter value was successful, false otherwise
	*/
	virtual bool SetShadowSamples (const TimeValue t, int shadowSamples) = 0;
	//@}

	//! \name Dimensions
	//! \brief The length and the width of the sky portal.
	//@{
	/*! \brief Gets the length of the sky portal.
	 * \param[in] t - The time at which the parameter is retrieved
	 * \param[in, out] valid - The time interval for which the parameter value is valid
	 * \return	The length of the sky portal
	*/
	virtual float GetLength(const TimeValue t, Interval &valid) const = 0;

	/*! \brief Sets the length of the sky portal.
	 * \param[in] t - The time at which the parameter is set
	 * \param[in] length - The new parameter value. Must be a positive floating point value
	 * \return true if setting the new parameter value was successful, false otherwise
	*/
	virtual bool SetLength(const TimeValue t, float length) = 0;

	/*! \brief Gets the width of the sky portal.
	 * \param[in] t - The time at which the parameter is retrieved
	 * \param[in, out] valid - The time interval for which the parameter value is valid
	 * \return	The width of the sky portal
	*/
	virtual float GetWidth(const TimeValue t, Interval &valid) const = 0;

	/*! \brief Sets the width of the sky portal.
	 * \param[in] t - The time at which the parameter is set
	 * \param[in] width - The new parameter value. Must be a positive floating point value
	 * \return true if setting the new parameter value was successful, false otherwise
	*/
	virtual bool SetWidth(const TimeValue t, float width) = 0;
	//@}

		
	//! \name Light Flux Direction
	/*! \brief Light Flux Direction Determines the direction in which light flows through the portal. 
	 * The sky portal has two sides that can either correspond to the "indoor" or 
	 * "outdoor" sides of the scene. 
	 */
	//@{
	//! \brief The possible directions of the light flux through the sky portal
	enum LightFluxDirection {
		/*! \brief Light flows from the "outdoor" towards "indoor" side of the sky portal */
		kOutdoorToIndoor = 0, 
		/*! \brief Light flows from the "indoor" towards "outdoor" side of the sky portal */
		kIndoorToOutdoor = 1, 
		kLastDir,
	};

	/*! \brief Gets the light flux direction through the sky portal
	 * \return	The light flux direction through the sky portal
	*/
	virtual LightFluxDirection GetLightFluxDirection() const = 0;

	/*! \brief Sets the light flux direction through the sky portal
	 * \param[in] dir - The new direction value 
	 * \return true if setting the new parameter value was successful, false otherwise
	*/
	virtual bool SetLightFluxDirection(LightFluxDirection dir) = 0;
	//@}

	//! \name Visibility in renderings
	/*! \brief Allows for specifying whether the sky portal is visible in renderings or not.
	 * When the sky portal appears in renderings, objects that are on the "indoors" 
	 * side of the sky portal do not appear in the rendering. 
	 */
	//@{
	/*! \brief Gets whether the sky portal appears in rendering
	 * \param[in] t - The time at which the parameter is retrieved
	 * \param[in, out] valid - The time interval for which the parameter value is valid
	 * \return	true if the sky portal appears in rendering, false otherwise
	*/
	virtual bool GetVisibleInRendering(const TimeValue t, Interval &valid) const = 0;

	/*! \brief Sets whether the sky portal appears in rendering
	 * \param[in] t - The time at which the parameter is set
	 * \param[in] visible - The new parameter value. 
	 * \return true if setting the new parameter value was successful, false otherwise
	*/
	virtual bool SetVisibleInRenderin(const TimeValue t, bool visible) = 0;
	//@}

	//! \name Transparency
	/*! \brief Transparency Filters the view outside the window
	 * Changing this color does not change the light coming in, but has the effect of 
	 * darkening outside objects, which can help if they are overexposed. To avoid  
	 * recoloring the outside view, use a shade of gray, such as R=G=B=0.5.
	*/
	//@{
	/*! \brief Gets the transparency color of the sky portal
	 * \param[in] t - The time at which the parameter is retrieved
	 * \param[in, out] valid - The time interval for which the parameter is valid
	 * \return	The value of the transparency color
	*/
	virtual AColor GetTransparencyColor(const TimeValue t, Interval &valid) const = 0;

	/*! \brief Sets the transparency color of the sky portal
	 * \param[in] t - The time at which the parameter is set
	 * \param[in] color - The new parameter value
	 * \return true if setting the new color was successful, false otherwise
	*/
	virtual bool SetTransparencyColor(const TimeValue t, const AColor& color) = 0;
	//@}
	
	//! \name Illumination Color
	//! \brief The sky portal can derive its illumination color from different sources.
	//@{
	//! \brief The possible sources of illumination color used by a sky portal
	enum IlluminationColorSource {
		/*! \brief Uses the current scene environment map. This allows the outdoor to 
		be illuminated with a different color than the indoor. */
		kSceneEnvironmentColor, 
		/*! \brief Use a texture map for the illumination coloring. 
		See IMrSkyPortalLight::setIlluminationMap */
		kCustomMapColor, 
		/*! \brief Uses the existent skylight in the scene */
		kSkylightColor, 
		kLastColorSource,
	};

	/*! \brief Gets the illumination color source used by the sky portal
	 * \return	The illumination color source used by the sky portal
	*/
	virtual IlluminationColorSource GetIllumColorSource() const = 0;

	/*! \brief Sets the illumination color source used by the sky portal
	 * \param[in] dir - The new parameter value 
	 * \return true if setting the new parameter value was successful, false otherwise
	*/
	virtual bool SetIllumColorSource(IlluminationColorSource source) = 0;
	//@}

	//! \name Custom Illumination Color
	//! \brief A map can be used to specify a custom illumination color for the sky portal
	//@{
	/*! \brief Gets the current map used to supply the illumination color.
	 * \return	A pointer to a texture map that supplies the illumination color
	*/
	virtual Texmap* GetIlluminationMap() const = 0;

	/*! \brief Sets the map used for supplying the illumination color
	 * \param[in] illumColorMap - A pointer to a texture map that supplies the 
	 * illumination color. To set no map, set this parameter to NULL.
	 * \return true if the setting was successful	
	*/
	virtual bool SetIlluminationMap(Texmap* illumColorMap) = 0;
	//@}

};

