/**********************************************************************
*<
FILE:			IViewportShadingMgr.h

DESCRIPTION:	Core interface to control siewport shading, including shadows

CREATED BY:		Neil Hazzard

HISTORY:		created March 8th, 2007

*>	Copyright (c) 2007, All Rights Reserved.
**********************************************************************/

#pragma once

#include "iFnPub.h"
#include "GetCOREInterface.h"
// forward declarations
class ViewExp;

#define IVIEWPORTSHADINGMGR_INTERFACE Interface_ID(0x7eba45d1, 0x2b3d454b)
#define IVIEWPORTSHADINGMGR2_INTERFACE Interface_ID(0x245b423f, 0x9df3b47)

//! Viewport manager to control general illumination, including shadow support in the viewport
/*! New to 3ds Max 2008, the IViewportShadingMgr gives access to the new viewport Shading feature.
The viewports have been redesigned to support more realistic rendering including Shadows.  The interface also 
provides access to various flags that control how lights are used in the viewport.  This is used closely with 
the new INodeShadingProperites API, that manages the actual flags set by IViewportShadingMgr
The developer can get access to the CORE interface by using
\code
IViewportShadingMgr * pShaderMgr = GetIViewportShadingMgr();
\endcode

*/
class IViewportShadingMgr : public FPStaticInterface
{

public:
	enum EViewportDisplayQuality
	{
		kNone,	//! Hardware shading disabled
		kGood,	//! Hardware shading with DirectX shader model 2.0
		kBest,	//! Hardware shading with DirectX shader model 3.0
	};

	//! Please refer to INodeShadingProperties for more details.
	enum EViewportLightFlags{
		kShadowCasting,	//!Light is shadow casting
		kIlluminating,	//!Light is illuminating
		kLocked,		//!Light is locked
	};
	
	//! Define the viewports shading quality
	/*! Set the viewport rendering quality to a value defined in EViewportDisplayQuality. Please make sure 
	you call IsShadingLimitValid before this method to determine if the graphics hardware supports this shading level.
	\param limit The shading limits as defined by EViewportDisplayQuality
	\param bQuiet True to enable quiet mode.  No dialogs will be produced to provide user feedback.  This 
	defaults to true.
	*/
	virtual void SetViewportShadingLimits(int limit, bool bQuiet = true)=0;

	//! Retrieve the viewport quality
	/*! Get the viewport rendering quality, as defined by EViewportDisplayQuality
	\returns The rendering quality
	*/
	virtual int GetViewportShadingLimits()=0;

	//! Get the current state of the Auto Display Selected light flag
	/*! This will return the current state of the Auto Display of selected lights flag
	Any lights that are selected will automatically have their illumination flag set and
	take part in viewport rendering.  If they have their shadow flag set as well, they will 
	become shadow casters.
	\return The state of the flag
	*/
	virtual bool GetAutoDisplayOfSelLights()=0;

	//! Set the state of the Auto Display of Selected lights flag
	/*! This will set the current state of the Auto Display of selected lights flag
	If set to true, any lights that are selected will automatically have their illumination flag set and
	take part in viewport rendering.  If they have their shadow flag set as well, they will 
	become shadow casters.
	\param bSet The flag value
	*/
	virtual void SetAutoDisplayOfSelLights(bool bSet)=0;

	//! Flag controlling shadow generation for default lights
	/*! Defines whether default lights cast shadows in the viewport
	\param bSet The flag value
	*/
	virtual void SetCastShadowDefaultLights(bool bSet)=0;

	//! Access the flag defining whether shadows are generated for default lights
	/*!
	\return The state of the flag
	*/
	virtual bool GetCastShadowDefaultLights()=0;

	//! A method that defines that the current selected lights will be part of viewport rendering
	/*! When used, all lights that are currently selected will have their Illumination flag set, and will
	be part of the viewport rendering.  All unselected lights will be deactivated.
	*/
	virtual void DisplayOnlySelectedLights()=0;

	//! Lock the current selected lights
	/*! A locked light will be active in the viewport rendering, regardless of other flags set
	This could be something like a sun light, that always illuminates and casts shadows.  Any selected
	light will have this flag set depending on the parameter value
	\param bSet The value of the flag to set.
	*/
	virtual void LockSelectedLights(bool bSet)=0;

	//! A method to retrieve lights based on the usage in the scene.
	/*! This is a general method to retrieve a list of nodes based on EViewportLightFlags.  This is 
	a quick way off accessing all viewport shadow casting lights in the scene.
	\param &lightList  A node tab to retrieve the node list based on type
	\param type The light type to store based on EViewportLightFlags
	*/
	virtual void GetViewportShadingLights(Tab<INode*> &lightList, int type) = 0;

	//! Sets the global intensity override for the shadows
	/*! The intensity value is a global override for the general appearance of the shadow.  A value
	of 1.0, the default will provide the unaltered shadow.  A value less than one will fade the shadow
	to produce different effects.
	\param intensity The value to set the intensity.  This should be between 0 and 1, but the function will clamp
	*/
	virtual void SetGlobalShadowIntensity(float intensity) = 0;

	//! Retreive the Global shadow intensity
	/*! This will return the current intensity value for the shadows.
	\returns The intensity value
	*/
	virtual float GetGlobalShadowIntensity()=0;

	//! Defines whether the viewport shading code supports transparent object/materials in the shadow code
	/*! This is a performance flag.  If it is turned off, then the viewport will become faster as less GPU 
	resources are active.  This flag is typically used by the Adaptive Degradation system
	\param bSet True to turn it on - False to turn it off
	*/
	virtual void SetUseTransparencyInShadows(bool bSet) = 0;

	//! Get the current Transparency in shadows flag
	/*! See SetUseTransparencyInShadows
	\returns The current state of the flag
	*/
	virtual bool GetUseTransparencyInShadows() = 0;

	//! Defines whether the viewport shading code updates the shadows for all windows or just the active viewport
	/*! The us a performance flag.  TO increase performance in the viewports, turning this flag on will only update
	shadows in the active viewport - if it is a supported mode.
	\param bSet True to turn it on - False to turn it off
	*/
	virtual void SetUpdateShadowInActiveViewOnly(bool bSet) =0;

	//!Get the current update shadow in active view only flag
	/*! See SetUpdateShadowInActiveViewOnly
	\returns The current state of the flag
	*/
	virtual bool GetUpdateShadowInActiveViewOnly()=0;

	//! Determine whether the shading limit is compatible with the current graphics hardware 
	/*! This method is used to determine if the graphics hardware can support the shading level.  This 
	will query the GPU to find out what pixel shader support exists.  For "Best", shader model 3 is required,
	"Good" needs a minimum of shader model 2.  The developer should call this method before calling 
	SetViewportShadingLimits as that method will not perform any additional tests.
	\param limit The shading limit to test
	\return True is the shading limit is supported
	*/
	virtual bool IsShadingLimitValid(int limit)=0;

	//! Get the light icon color based on the current viewport shading limits
	/*! The light icon that gets drawn should reflect the current state of the viewport shading.  When the 
	light is shadow casting it will be displayed in a duller color.  If the light is currently not illuminating
	again it will appear duller than the usual wireframe color.  This method should be used for any lights that 
	display a viewport icon and want to adhere to the new coloring scheme.  The color returned can be used in 
	the GraphicsWindow::setColor function.
	\param &lightNode The light node to check for the current state of viewport shading flags
	\return The new altered color.  If viewport shading is set to "None" - this will be the default wireframe color
	*/
	virtual Color GetLightIconColor(INode &lightNode)=0;

	//! Set whether the GI viewport system works with Transparent objects
	virtual void SetUseTransparencyInGI(bool bSet) =0;

	//! Get whether the GI viewport system uses Transparency
	virtual bool GetUseTransparencyInGI() = 0;

	//! Set the shadow filter size of soft-edge shadow
	/*! This method is used to control the softness of the shadow edge when the viewport shadow mode is set to
	soft-edge shadow.  It has no effect when in hard-edge shadow mode or if an area light with viewport soft shadow 
	turned on.
	\param filterSize The value to set the filter size.  This should be between 0 and 1, but the function will clamp.
	*/
	virtual void SetShadowFilterSize(float filterSize)=0;

	//! Get the shadow filter size of soft-edge shadow
	/*! This will return the current filter size of soft-edge shadow.
	\returns The filter size value
	*/
	virtual float GetShadowFilterSize()=0;

};

/*! 
\brief Extends class IViewportShadingMgr
*/
class IViewportShadingMgr2  : public IViewportShadingMgr
{
public:
	//* Get the active viewport's hardware enabled flag.  
	/*! The active viewport can be interpreted by the function is two ways.  Firstly if bCurrentlyTraversed is false, the default, 
	the actual active viewport is used.  This would be the same as Interface::GetActiveViewport(). Secondly
	if bCurrentlyTraversed is true, then the viewport that is currently being traversed as part of 
	the viewport rendering will be considered the active viewport.  This could be used by a material that
	needs additional setup if hardware rendering is being performed.
	\param bCurrentlyTraversed Defines what the method will interpret as the active view.  See notes for the function.
	\returns TRUE if enabled
	*/
	virtual bool IsHWShadingEnabled(bool bCurrentlyTraversed = false)const=0;

	//! Get the hardware enabled flag for the view supplied.
	/*!
	\param & view The view to query
	\returns TRUE if enabled
	*/
	virtual bool IsHWShadingEnabled(const ViewExp &view)const=0;
};


inline IViewportShadingMgr* GetIViewportShadingMgr() {

	return dynamic_cast<IViewportShadingMgr*>(GetCOREInterface(IVIEWPORTSHADINGMGR_INTERFACE));
};

inline IViewportShadingMgr2* GetIViewportShadingMgr2() 
{
	IViewportShadingMgr * lpVpMgr = GetIViewportShadingMgr();
	return dynamic_cast<IViewportShadingMgr2*>(lpVpMgr->GetInterface(IVIEWPORTSHADINGMGR2_INTERFACE));
};


