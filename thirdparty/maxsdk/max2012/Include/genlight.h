/**********************************************************************
 *<
	FILE: genlight.h

	DESCRIPTION:  Defines General-Purpose lights

	CREATED BY: Tom Hudson

	HISTORY: created 5 December 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/
#pragma once
#include "object.h"


#define OMNI_LIGHT		0	// Omnidirectional
#define TSPOT_LIGHT		1	// Targeted
#define DIR_LIGHT		2	// Directional
#define FSPOT_LIGHT		3	// Free
#define TDIR_LIGHT		4   // Targeted directional

#define NUM_LIGHT_TYPES	5

#define DECAY_NONE  0
#define DECAY_INV   1
#define DECAY_INVSQ 2

// SetAtten types
#define ATTEN1_START   	0  // near
#define ATTEN1_END		1  // near
#define ATTEN_START		2  // far
#define ATTEN_END		3  // far

// Shapes
#define RECT_LIGHT		0
#define CIRCLE_LIGHT	1

class ShadowType;

#pragma warning(push)
#pragma warning(disable:4100 4239)

/*! \sa  Class LightObject, Class NameTab, Class Control, Class Interval, Class Point3, Class ObjLightDesc.\n\n
\par Description:
This class describes a generic light object. It is used as a base class for
creating plug-in lights. Methods of this class are used to get and set
properties of the light. All methods of this class are virtual.  */
class GenLight: public LightObject
{
public:
	/*! \remarks Creates a new light object of the specified type.
	\par Parameters:
	<b>int type</b>\n\n
	One of the following values:\n\n
	<b>OMNI_LIGHT</b> -- Omnidirectional light.\n\n
	<b>TSPOT_LIGHT</b> -- Targeted spot light.\n\n
	<b>DIR_LIGHT</b> -- Directional light.\n\n
	<b>FSPOT_LIGHT</b> -- Free spot light.\n\n
	<b>TDIR_LIGHT</b> -- Targeted directional light.
	\return  A pointer to a new instance of the specified light type. */
	virtual GenLight *NewLight(int type)=0;
	virtual RefResult EvalLightState(TimeValue t, Interval& valid, LightState* cs)=0;
	/*! \remarks	Returns the type of light this is.
	\return  One of the following values:\n\n
	<b>OMNI_LIGHT</b> -- Omnidirectional light.\n\n
	<b>TSPOT_LIGHT</b> -- Targeted spot light.\n\n
	<b>DIR_LIGHT</b> -- Directional light.\n\n
	<b>FSPOT_LIGHT</b> -- Free spot light.\n\n
	<b>TDIR_LIGHT</b> -- Targeted directional light. */
	virtual int Type()=0;  // OMNI_LIGHT, TSPOT_LIGHT, DIR_LIGHT, FSPOT_LIGHT, TDIR_LIGHT
	virtual void SetType(int tp) {} // OMNI_LIGHT, TSPOT_LIGHT, DIR_LIGHT, FSPOT_LIGHT, TDIR_LIGHT      
	/*! \remarks Returns TRUE if the light is a spotlight; otherwise FALSE. */
	virtual BOOL IsSpot()=0;
	/*! \remarks Returns TRUE if the light is directional; otherwise FALSE. */
	virtual BOOL IsDir()=0;
	/*! \remarks Sets the light on or off.
	\par Parameters:
	<b>int onOff</b>\n\n
	Nonzero sets the light on; zero sets it off. */
	virtual void SetUseLight(int onOff)=0;
	/*! \remarks Returns TRUE if the light is on; otherwise FALSE. */
	virtual BOOL GetUseLight(void)=0;
	/*! \remarks Sets the shape used for a spotlight, either rectangular or
	circular.
	\par Parameters:
	<b>int s</b>\n\n
	One of the following values:\n\n
	<b>RECT_LIGHT</b>\n\n
	<b>CIRCLE_LIGHT</b> */
	virtual void SetSpotShape(int s)=0;
	/*! \remarks Retrieves the shape used for a spotlight.
	\return  One of the following values:\n\n
	<b>RECT_LIGHT</b>\n\n
	<b>CIRCLE_LIGHT</b>\n\n
	  */
	virtual int GetSpotShape(void)=0;
	/*! \remarks Sets the hotspot to the specified angle at the specified
	time.
	\par Parameters:
	<b>TimeValue time</b>\n\n
	The time to set the value.\n\n
	<b>float f</b>\n\n
	The angle in degrees. */
	virtual void SetHotspot(TimeValue time, float f)=0;
	/*! \remarks Returns the hotspot angle in degrees at the specified time
	and updates the interval to reflect the validity of the hotspot controller.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the angle.\n\n
	<b>Interval\& valid = Interval(0,0)</b>\n\n
	The interval which is updated. */
	virtual float GetHotspot(TimeValue t, Interval& valid = Interval(0,0))=0;
	/*! \remarks Sets the falloff angle at the specified time.
	\par Parameters:
	<b>TimeValue time</b>\n\n
	The time to set the angle.\n\n
	<b>float f</b>\n\n
	The angle to set in degrees. */
	virtual void SetFallsize(TimeValue time, float f)=0;
	/*! \remarks Returns the falloff angle in degrees at the specified time
	and updates the interval passed to reflect the validity of the falloff
	controller.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to return the value.\n\n
	<b>Interval\& valid = Interval(0,0)</b>\n\n
	The interval which is updated. */
	virtual float GetFallsize(TimeValue t, Interval& valid = Interval(0,0))=0;
	/*! \remarks Sets the specified attenuation range distance at the time
	passed.
	\par Parameters:
	<b>TimeValue time</b>\n\n
	The time to set the attenuation distance.\n\n
	<b>int which</b>\n\n
	Indicates which distance to set. One of the following values:\n\n
	<b>ATTEN1_START</b>\n\n
	The near start range.\n\n
	<b>ATTEN1_END</b>\n\n
	The near end range.\n\n
	<b>ATTEN_START</b>\n\n
	The far start range.\n\n
	<b>ATTEN_END</b>\n\n
	The far end range.\n\n
	<b>float f</b>\n\n
	The distance to set. */
	virtual void SetAtten(TimeValue time, int which, float f)=0;
	/*! \remarks Returns the specified attenuation distance at the time passed
	and updates the interval to reflect the validity of the attenuation
	controller.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the attenuation distance.\n\n
	<b>int which</b>\n\n
	Indicates which distance to get. One of the following values:\n\n
	<b>ATTEN1_START</b>\n\n
	The near start range.\n\n
	<b>ATTEN1_END</b>\n\n
	The near end range.\n\n
	<b>ATTEN_START</b>\n\n
	The far start range.\n\n
	<b>ATTEN_END</b>\n\n
	The far end range.\n\n
	<b>Interval\& valid = Interval(0,0)</b>\n\n
	The interval which is updated. */
	virtual float GetAtten(TimeValue t, int which, Interval& valid = Interval(0,0))=0;
	/*! \remarks Sets the light's target distance.
	\par Parameters:
	<b>TimeValue time</b>\n\n
	The time to set the distance.\n\n
	<b>float f</b>\n\n
	The distance to set. */
	virtual void SetTDist(TimeValue time, float f)=0;
	/*! \remarks Returns the light's target distance at the specified time and
	updates the interval passed to reflect the validity of the target distance.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to retrieve the distance.\n\n
	<b>Interval\& valid = Interval(0,0)</b>\n\n
	The interval to update. */
	virtual float GetTDist(TimeValue t, Interval& valid = Interval(0,0))=0;
	/*! \remarks When the renderer goes to render the scene it asks all of the
	lights to create an ObjLighDesc object. This is the method that is called
	to return a pointer to this object.
	\par Parameters:
	<b>INode *n</b>\n\n
	The node pointer of the light.\n\n
	<b>BOOL forceShadowBuffer</b>\n\n
	Forces the creation of a shadow buffer.\n\n
	  */
	virtual ObjLightDesc *CreateLightDesc(INode *inode, BOOL forceShadowBuf=FALSE )=0;
	//JH 06/03/03 overload with RenderGlobalContext as additional arg
	//allows extended light sources to simplify based on global render params
	virtual ObjLightDesc *CreateLightDesc(RenderGlobalContext *rgc, INode *inode, BOOL forceShadowBuf=FALSE )
	{return CreateLightDesc(inode, forceShadowBuf);}
	/*! \remarks Sets the color of the light at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to set the color.\n\n
	<b>Point3\& rgb</b>\n\n
	The color to set. */
	virtual void SetRGBColor(TimeValue t, Point3& rgb)=0;
	/*! \remarks Returns the color of the light at the specified time and
	updates the validity interval to reflect this parameters validity interval.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the color.\n\n
	<b>Interval \&valid = Interval(0,0)</b>\n\n
	The interval which is updated. */
	virtual Point3 GetRGBColor(TimeValue t, Interval &valid = Interval(0,0))=0;
	/*! \remarks Sets the HSV color of the light at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to set the color.\n\n
	<b>Point3\& hsv</b>\n\n
	The color.
	\par Sample Code:
	The following sample shows how the RGB value can be converted to HSV.\n\n
	<b>{</b>\n\n
	<b>int h, s, v;</b>\n\n
	<b>Point3 rgbf = GetRGBColor(t, valid);</b>\n\n
	<b>DWORD rgb = RGB((int)(rgbf[0]*255.0f),</b>\n\n
	<b>(int)(rgbf[1]*255.0f), (int)(rgbf[2]*255.0f));</b>\n\n
	<b>RGBtoHSV (rgb, \&h, \&s, \&v);</b>\n\n
	<b>return Point3(h/255.0f, s/255.0f, v/255.0f);</b>\n\n
	<b>}</b> */
	virtual void SetHSVColor(TimeValue t, Point3& hsv)=0;
	/*! \remarks Retrieves the HSV color of the light at the specified time
	and updates the validity interval to reflect the color parameter.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to retrieve the color.\n\n
	<b>Interval \&valid = Interval(0,0)</b>\n\n
	The interval to update.
	\return  The color of the light (as a Point3). */
	virtual Point3 GetHSVColor(TimeValue t, Interval &valid = Interval(0,0))=0;
	/*! \remarks Sets the intensity (multiplier value) of the light at the
	specified time.
	\par Parameters:
	<b>TimeValue time</b>\n\n
	The time to set the intensity.\n\n
	<b>float f</b>\n\n
	The value to set. */
	virtual void SetIntensity(TimeValue time, float f)=0;
	/*! \remarks Returns the intensity (multiplier value) of the light at the
	specified time and updates the interval passed to reflect the validity of
	the controller.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the value.\n\n
	<b>Interval\& valid = Interval(0,0)</b>\n\n
	The interval is updated. */
	virtual float GetIntensity(TimeValue t, Interval& valid = Interval(0,0))=0;
	/*! \remarks	Sets the light's contrast setting.
	\par Parameters:
	<b>TimeValue time</b>\n\n
	The time to set the contrast value.\n\n
	<b>float f</b>\n\n
	The new contrast value in the range of 0.0 to 100.0. */
	virtual void SetContrast(TimeValue time, float f)=0;
	/*! \remarks	Returns the light's contrast setting in the range 0.0 to 100.0.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the light's contrast setting.\n\n
	<b>Interval\& valid = Interval(0,0)</b>\n\n
	This interval is updated to reflect the interval of the light's contrast
	setting. */
	virtual float GetContrast(TimeValue t, Interval& valid = Interval(0,0))=0;
	/*! \remarks Set the aspect property to the specified value.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the aspect ratio.\n\n
	<b>float f</b>\n\n
	Specifies the aspect ratio setting. */
	virtual void SetAspect(TimeValue t, float f)=0;
	/*! \remarks Returns the aspect property (for rectangular lights) at the
	specified time and updates the interval passed to reflect the validity of
	the aspect controller.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the value.\n\n
	<b>Interval\& valid = Interval(0,0)</b>\n\n
	The interval to update. */
	virtual float GetAspect(TimeValue t, Interval& valid = Interval(0,0))=0;
	/*! \remarks Sets the spotlight cone display to on or off. This controls
	if the cone is depicted graphically in the viewports.
	\par Parameters:
	<b>int s</b>\n\n
	Indicates if the cone display should be on or off. Nonzero indicates the
	cone should be displayed; otherwise it will be turned off.\n\n
	<b>int notify=TRUE</b>\n\n
	If notify is TRUE the plug-in should call <b>NotifyDependents()</b> to
	notify its dependents. */
	virtual void SetConeDisplay(int s, int notify=TRUE)=0;
	/*! \remarks Returns the cone display property. TRUE if the spotlight cone
	is on; FALSE if off. */
	virtual BOOL GetConeDisplay(void)=0;
	/*! \remarks Sets the far attenuation state to on or off.
	\par Parameters:
	<b>int s</b>\n\n
	Nonzero for on; zero for off. */
	virtual void SetUseAtten(int s)=0;
	/*! \remarks Returns nonzero if far attenuation is on; zero if off. */
	virtual BOOL GetUseAtten(void)=0;
	/*! \remarks Establishes if the light far attenuation range is displayed
	in the viewports.
	\par Parameters:
	<b>int s</b>\n\n
	Nonzero for on; zero for off. */
	virtual void SetAttenDisplay(int s)=0;
	/*! \remarks Returns TRUE if the far attenuation range is displayed;
	otherwise FALSE. */
	virtual BOOL GetAttenDisplay(void)=0;
	/*! \remarks	Sets if the light uses near attenuation.
	\par Parameters:
	<b>int s</b>\n\n
	Nonzero to use near attenuation; otherwise zero. */
	virtual void SetUseAttenNear(int s)=0;
	/*! \remarks	Returns TRUE if the light has near attenuation on; otherwise FALSE. */
	virtual BOOL GetUseAttenNear(void)=0;
	/*! \remarks	Establishes if the light near attenuation range is displayed in the
	viewports.
	\par Parameters:
	<b>int s</b>\n\n
	TRUE to turn on the display; otherwise FALSE. */
	virtual void SetAttenNearDisplay(int s)=0;
	/*! \remarks	Returns TRUE if the light near attenuation range is displayed in the
	viewports; otherwise FALSE. */
	virtual BOOL GetAttenNearDisplay(void)=0;
	/*! \remarks Sets the light to enabled or disables (on or off).
	\par Parameters:
	<b>int enab</b>\n\n
	Nonzero for on; zero for off. */
	virtual void Enable(int enab)=0;
	/*! \remarks Sets the map bias value at the time passed.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to set the value.\n\n
	<b>float f</b>\n\n
	The value to set. */
	virtual void SetMapBias(TimeValue t, float f)=0;
	/*! \remarks Returns the map bias setting at the specified time and
	updates the interval passed to reflect the validity of the map bias.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the value.\n\n
	<b>Interval\& valid = Interval(0,0)</b>\n\n
	The interval to update. */
	virtual float GetMapBias(TimeValue t, Interval& valid = Interval(0,0))=0;
	/*! \remarks Sets the map range value at the time passed.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to set the value.\n\n
	<b>float f</b>\n\n
	The value to set. */
	virtual void SetMapRange(TimeValue t, float f)=0;
	/*! \remarks Returns the map range setting at the specified time and
	updates the interval passed to reflect the validity of the map range.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the value.\n\n
	<b>Interval\& valid = Interval(0,0)</b>\n\n
	The interval to update. */
	virtual float GetMapRange(TimeValue t, Interval& valid = Interval(0,0))=0;
	/*! \remarks Sets the map size value at the time passed.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to set the value.\n\n
	<b>int f</b>\n\n
	The value to set. */
	virtual void SetMapSize(TimeValue t, int f)=0;
	/*! \remarks Returns the map size setting at the specified time and
	updates the interval passed to reflect the validity of the map size.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the value.\n\n
	<b>Interval\& valid = Interval(0,0)</b>\n\n
	The interval to update. */
	virtual int GetMapSize(TimeValue t, Interval& valid = Interval(0,0))=0;
	/*! \remarks Sets the map raytrace bias value at the time passed.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to set the value.\n\n
	<b>float f</b>\n\n
	The value to set. */
	virtual void SetRayBias(TimeValue t, float f)=0;
	/*! \remarks Returns the raytrace bias setting at the specified time and
	updates the interval passed to reflect the validity of the bias.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the value.\n\n
	<b>Interval\& valid = Interval(0,0)</b>\n\n
	The interval to update. */
	virtual float GetRayBias(TimeValue t, Interval& valid = Interval(0,0))=0;

	/*! \remarks Returns TRUE if the use global setting is on; otherwise
	FALSE. */
	virtual int GetUseGlobal()=0;
	/*! \remarks Set the use global setting to on or off.
	\par Parameters:
	<b>int a</b>\n\n
	TRUE for on; FALSE for off. */
	virtual void SetUseGlobal(int a)=0;
	/*! \remarks	Returns the lights Cast Shadows flag. Nonzero indicates the light casts
	shadows; otherwise 0.
	\par Default Implementation:
	<b>{return 0;}</b> */
	virtual int GetShadow()=0;
	/*! \remarks	Sets the lights Cast Shadows flag.
	\par Parameters:
	<b>int a</b>\n\n
	Nonzero indicates the light casts shadows; zero indicates the light does
	not cast shadows.
	\par Default Implementation:
	<b>{}</b> */
	virtual void SetShadow(int a)=0;

	/*! \remarks	Retrieves the type of shadows used by the light - mapped or raytraced.
	\return  One of the following values:\n\n
	<b>-1</b>: if the Shadow Generator is NULL. (R3 only).\n\n
	<b>0</b>: if the light uses Shadow Maps.\n\n
	<b>1</b>: if the light uses Raytraced Shadows.\n\n
	<b>0xffff</b>: for any other Shadow Generators. (R3 only).
	\par Default Implementation:
	<b>{return 0;}</b> */
	virtual int GetShadowType()=0;
	/*! \remarks	Sets the type of shadows used by the light - mapped or raytraced.
	\par Parameters:
	<b>int a</b>\n\n
	The shadow type. One of the following values:\n\n
	<b>0</b>: This value plugs in a Shadow Map Generator.\n\n
	<b>1</b>: This value plugs in a Raytraced Shadow Generator.\n\n
	Any other value is a NO-OP.
	\par Default Implementation:
	<b>{}</b> */
	virtual void SetShadowType(int a)=0;

	// Pluggable Shadow generator (11/2/98): 
	/*! \remarks	Sets the shadow generator used by the light.
	\par Parameters:
	<b>ShadowType *s</b>\n\n
	The shadow plug-in to use. See Class ShadowType.
	\par Default Implementation:
	<b>{}</b> */
	virtual	void SetShadowGenerator(ShadowType *s) {};
	/*! \remarks	Returns a pointer to the shadow generator plug-in in use by the light. See
	Class ShadowType.
	\par Default Implementation:
	<b>{ return NULL; }</b> */
	virtual ShadowType *GetShadowGenerator() { return NULL; } 

	virtual int GetAbsMapBias()=0;
	virtual void SetAbsMapBias(int a)=0;

	/*! \remarks	Sets the atmospheric shadow flag to on or off at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the value.\n\n
	<b>int onOff</b>\n\n
	TRUE for on; FALSE for off.
	\par Default Implementation:
	<b>{}</b> */
	virtual void SetAtmosShadows(TimeValue t, int onOff) {}
	/*! \remarks	Returns the atmospheric shadow setting at the specified time.
	\par Parameters:
	<b>TimeValue t</b>
	\par Default Implementation:
	<b>{ return 0; }</b> */
	virtual int GetAtmosShadows(TimeValue t) { return 0; }
	/*! \remarks	Sets the atmospheric opacity value at the time passed.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to set the value.\n\n
	<b>float f</b>\n\n
	The value to set.
	\par Default Implementation:
	<b>{}</b> */
	virtual void SetAtmosOpacity(TimeValue t, float f) {}
	/*! \remarks	Returns the atmospheric opacity value at the specified time and updates the
	validity interval to reflect the validity of the opacity controller.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the value.\n\n
	<b>Interval\& valid=FOREVER</b>\n\n
	The interval to update.
	\par Default Implementation:
	<b>{ return 0.0f; }</b> */
	virtual float GetAtmosOpacity(TimeValue t, Interval& valid=FOREVER) { return 0.0f; }
	/*! \remarks	Sets the atmospheric shadow color amount at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to set the value.\n\n
	<b>float f</b>\n\n
	The value to set.
	\par Default Implementation:
	<b>{}</b> */
	virtual void SetAtmosColAmt(TimeValue t, float f) {}
	/*! \remarks	Returns the atmospheric shadow color amount at the specified time and
	updates the interval passed to reflect the validity of the amount.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get.\n\n
	<b>Interval\& valid=FOREVER</b>\n\n
	The interval to update.
	\par Default Implementation:
	<b>{ return 0.0f; }</b> */
	virtual float GetAtmosColAmt(TimeValue t, Interval& valid=FOREVER) { return 0.0f; }
	virtual void SetUseShadowColorMap(TimeValue t, int onOff) { }
	virtual int GetUseShadowColorMap(TimeValue t) { return FALSE; }
	
	/*! \remarks	Returns the overshoot setting. Nonzero is on; zero is off. */
	virtual int GetOvershoot()=0;
	/*! \remarks	Sets the overshoot setting.
	\par Parameters:
	<b>int a</b>\n\n
	Nonzero for on; zero for off. */
	virtual void SetOvershoot(int a)=0;

	/*! \remarks Returns the exclusion list for the light. */
	virtual ExclList& GetExclusionList()=0;
	/*! \remarks Sets the exclusion list for the light.
	\par Parameters:
	<b>ExclList \&list</b>\n\n
	The exclusion list. */
	virtual void SetExclusionList(ExclList &list)=0;

	/*! \remarks Sets the controller for the hot spot parameter.
	\par Parameters:
	<b>Control *c</b>\n\n
	The controller to set.
	\return  TRUE if the controller was set; otherwise FALSE. */
	virtual BOOL SetHotSpotControl(Control *c)=0;
	/*! \remarks Sets the controller for the falloff parameter.
	\par Parameters:
	<b>Control *c</b>\n\n
	The controller to set.
	\return  TRUE if the controller was set; otherwise FALSE. */
	virtual BOOL SetFalloffControl(Control *c)=0;
	/*! \remarks Sets the controller for the color parameter.
	\par Parameters:
	<b>Control *c</b>\n\n
	The controller to set.
	\return  TRUE if the controller was set; otherwise FALSE. */
	virtual BOOL SetColorControl(Control *c)=0;
	/*! \remarks Returns the controller for the hot spot parameter. */
	virtual Control* GetHotSpotControl()=0;
	/*! \remarks Returns the controller for the falloff parameter. */
	virtual Control* GetFalloffControl()=0;
	/*! \remarks Returns the controller for the color parameter. */
	virtual Control* GetColorControl()=0;
	
	/*! \remarks	Establishes if the light affects the diffuse color of objects.
	\par Parameters:
	<b>BOOL onOff</b>\n\n
	TRUE to have the light affect the diffuse color; otherwise FALSE. */
	virtual void SetAffectDiffuse(BOOL onOff) {}
	/*! \remarks	Returns TRUE if the light affects the diffuse color of objects; otherwise
	FALSE. */
	virtual BOOL GetAffectDiffuse() {return 0;}
	/*! \remarks	Establishes if the light affects the specular color of objects.
	\par Parameters:
	<b>BOOL onOff</b>\n\n
	TRUE to have the light affect the specular color; otherwise FALSE. */
	virtual void SetAffectSpecular(BOOL onOff) {}
	/*! \remarks	Returns TRUE if the light affects the specular color of objects; otherwise
	FALSE. */
	virtual BOOL GetAffectSpecular() {return 0;}

	/*! \remarks	Sets the decay state of the light.
	\par Parameters:
	<b>BOOL onOff</b>\n\n
	This boolean works as an integer where <b>0</b> is None, <b>1</b> is
	Inverse and <b>2</b> is Inverse Square.
	\par Default Implementation:
	<b>{}</b> */
	virtual void SetDecayType(BOOL onOff) {}
	/*! \remarks	Returns the decay state of the light.
	\return  This boolean works as an integer where <b>0</b> is None, <b>1</b>
	is Inverse and <b>2</b> is Inverse Square.
	\par Default Implementation:
	<b>{return 0;}</b> */
	virtual BOOL GetDecayType() {return 0;}
	/*! \remarks	Sets the decay radius (i.e. falloff) of the light.
	\par Parameters:
	<b>TimeValue time</b>\n\n
	The time at which to set the radius.\n\n
	<b>float f</b>\n\n
	The radius to set.
	\par Default Implementation:
	<b>{}</b> */
	virtual void SetDecayRadius(TimeValue time, float f) {}
	/*! \remarks	Returns the decay radius of the light and updates the validity interval to
	reflect the validity of the radius controller.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to return the radius.\n\n
	<b>Interval\& valid = Interval(0,0)</b>\n\n
	The validity interval which is updated.
	\par Default Implementation:
	<b>{ return 0.0f;}</b> */
	virtual float GetDecayRadius(TimeValue t, Interval& valid = Interval(0,0)) { return 0.0f;}
	/*! \remarks	Sets the state of the 'Soften Diffuse Edge' parameter.
	\par Parameters:
	<b>TimeValue time</b>\n\n
	The time at which to set the value.\n\n
	<b>float f</b>\n\n
	The value to set in the range of 0.0 to 100.0.
	\par Default Implementation:
	<b>{}</b> */
	virtual void SetDiffuseSoft(TimeValue time, float f) {}
	/*! \remarks	Returns the state of the 'Soften Diffuse Edge' parameter.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to return the value.\n\n
	<b>Interval\& valid = Interval(0,0)</b>\n\n
	The validity interval that is updated to reflect the state of this
	parameter.
	\par Default Implementation:
	<b>{ return 0.0f; }</b> */
	virtual float GetDiffuseSoft(TimeValue t, Interval& valid = Interval(0,0)) { return 0.0f; }

	/*! \remarks	Sets the shadow color to the specified value at the time passed.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the shadow color.\n\n
	<b>Point3\& rgb</b>\n\n
	The color to set.
	\par Default Implementation:
	<b>{}</b> */
	virtual void SetShadColor(TimeValue t, Point3& rgb) {}
	/*! \remarks	Returns the shadow color at the time passed and updates the validity
	interval passed to reflect the validity of the shadow color controller.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to return the shadow color.\n\n
	<b>Interval \&valid = Interval(0,0)</b>\n\n
	The validity interval which is updated.
	\par Default Implementation:
	<b>{ return Point3(0,0,0); }</b> */
	virtual Point3 GetShadColor(TimeValue t, Interval &valid = Interval(0,0)) { return Point3(0,0,0); }
	/*! \remarks	Returns TRUE if the Light Affects Shadow Color flag is set; otherwise
	FALSE.
	\par Default Implementation:
	<b>{ return 0; }</b> */
	virtual BOOL GetLightAffectsShadow() { return 0; }
	/*! \remarks	Sets the state of the Light Affects Shadow Color flag to the value passed.
	\par Parameters:
	<b>BOOL b</b>\n\n
	TRUE for set; FALSE for off.
	\par Default Implementation:
	<b>{}</b> */
	virtual void SetLightAffectsShadow(BOOL b) {  }
	/*! \remarks	Sets the shadow color multiplier (density) to the value passed at the
	specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the value.\n\n
	<b>float m</b>\n\n
	The value to set.
	\par Default Implementation:
	<b>{}</b> */
	virtual void SetShadMult(TimeValue t, float m) {}
	/*! \remarks	Returns the shadow color multiplier (density) at the specified time and
	updates the interval passed to reflect the validity of the multiplier
	controller.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to return the value.\n\n
	<b>Interval \&valid = Interval(0,0)</b>\n\n
	The interval which is updated.
	\par Default Implementation:
	<b>{ return 1.0f; }</b> */
	virtual float GetShadMult(TimeValue t, Interval &valid = Interval(0,0)) { return 1.0f; }

	/*! \remarks	Returns a pointer to the texmap used as the projector image or NULL if not
	set.
	\par Default Implementation:
	<b>{ return NULL; }</b> */
	virtual Texmap* GetProjMap() { return NULL;  }
	/*! \remarks	Sets the texmap to use as the light's projector image.
	\par Parameters:
	<b>Texmap* pmap</b>\n\n
	Points to the texmap to set or NULL to clear it.
	\par Default Implementation:
	<b>{}</b> */
	virtual void SetProjMap(Texmap* pmap) {}
	/*! \remarks	Returns a pointer to the texmap used as the shadow projector or NULL if not
	set.
	\par Default Implementation:
	<b>{ return NULL; }</b> */
	virtual Texmap* GetShadowProjMap() { return NULL;  }
	/*! \remarks	Sets the texmap to use as the light's shadow projector.
	\par Parameters:
	<b>Texmap* pmap</b>\n\n
	Points to the texmap to set or NULL to clear it.
	\par Default Implementation:
	<b>{}</b> */
	virtual void SetShadowProjMap(Texmap* pmap) {}

	/*! \remarks	Sets the ambient only flag to on or off.
	\par Parameters:
	<b>BOOL onOff</b>\n\n
	TRUE for on; FALSE for off.
	\par Default Implementation:
	<b>{}</b> */
	virtual void SetAmbientOnly(BOOL onOff) {  }
	/*! \remarks	Returns the state of the ambient only flag. TRUE is on; FALSE is off.
	\par Default Implementation:
	<b>{ return FALSE; }</b> */
	virtual BOOL GetAmbientOnly() { return FALSE; }

	/* [dl | 01apr2003] These methods were used by mental ray to retrieve indirect
	   illumination properties from MAX standard lights. The standard lights implemented
	   a rollup to specify these properties. The new mental ray connection no longer
	   uses these properties, but instead uses a custom attribute. These methods have
	   therefore become useless, and we decided to remove them to avoid cluttering the SDK
	   with deprecated functionality.
	virtual void SetEmitterEnable(TimeValue t, BOOL onOff) {}
	virtual BOOL GetEmitterEnable(TimeValue t, Interval& valid = Interval(0,0)) { return 0; }
	virtual void SetEmitterEnergy(TimeValue t, float energy) {}
	virtual float GetEmitterEnergy(TimeValue t, Interval& valid = Interval(0,0)) { return 0.0f; }
	virtual void SetEmitterDecayType(TimeValue t, int decay) {}
	virtual int  GetEmitterDecayType(TimeValue t, Interval& valid = Interval(0,0)) { return 0; }
	virtual void SetEmitterCausticPhotons(TimeValue t, int photons) {}
	virtual int  GetEmitterCausticPhotons(TimeValue t, Interval& valid = Interval(0,0)) { return 0; }
	virtual void SetEmitterGlobalIllumPhotons(TimeValue t, int photons) {}
	virtual int  GetEmitterGlobalIllumPhotons(TimeValue t, Interval& valid = Interval(0,0)) { return 0; }
	*/
};

#pragma warning(pop)


