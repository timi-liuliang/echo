/**********************************************************************
 *<
FILE: shadgen.h : pluggable shadow generators.

	DESCRIPTION:

	CREATED BY: Dan Silva

	HISTORY: Created 10/27/98

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "maxheap.h"
#include "ref.h"
#include "plugapi.h"
#include "render.h"

// forward declarations
class LightObject;
class ObjLightDesc;
class RendContext;
class RenderGlobalContext;
class ShadeContext;
class Color;

/////////////////////////////////////////////////////////////////////////
// Shadow Generator flags
//
// These define the the 'flags' parameter to the CreateShadowGeerator call
// of the ShadowType class
// 
#define SHAD_PARALLEL       2	// light is directional, parallel projection
#define SHAD_OMNI           4	// generator can do omni, so do it on create
#define SHAD_2SIDED         8	// both sides of geometry shd cast shadows
#define MIN_SHADOW_MAP_SIZE 1.0f	// Minimum allowed value for the shadow map size property
#define MAX_SHADOW_MAP_SIZE 10000.0f	// Maximum allowed value for the shadow map size property
#define MIN_SHADOW_BIAS_VALUE 0.0f // Minimum allowed value for the shadow map bias property
#define MAX_SHADOW_BIAS_VALUE 10000.0f // Maximum allowed value for the shadow map bias property
#define MIN_SHADOW_SAMPLE_RANGE 0.01f // Minimum allowed value for the shadow map sample range property
#define MAX_SHADOW_SAMPLE_RANGE 50.0f // Maximum allowed value for the shadow map sample range property

class ShadowGenerator;
class AreaShadowGenerator;
class IAreaShadowType;
class ParamBlockDescID;
class IParamBlock;

#pragma warning(push)
#pragma warning(disable:4100)


/*! \sa  Class ShadowType.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
A developer derives a class from this class to provide the user interface for
the Shadow Generator plug-in. The <b>DeleteThis()</b> method deletes this
object when done. An instance of this class is returned from
<b>ShadowType::CreateShadowParamDlg()</b>.  */
class ShadowParamDlg: public MaxHeapOperators {
public:
	/*! \remarks Destructor. */
	virtual ~ShadowParamDlg() {}
	/*! \remarks This method deletes this instance of the class. */
	virtual void DeleteThis()=0;
	};

//
// NB: This Class needs to be made extensible w/ derivation from baseInterfaceServer
// This will be done the next time the API is changed
//
// This class carries the parameters for the shadow type, and puts up the parameter rollup.
#define AREA_SHADOW_TYPE_INTERFACE_ID Interface_ID(0x68436888, 0x5b5b2ab0)

/*! <b>This class is only available in release 5 or later.</b> The user of
GetAreaShadowType() is a linear or area light. The usage is:\n\n
\code

void AreaShadowLightObjDesc::createShadowGenerator(AreaLight* light,bool forceShadowBuf)
{
	ShadowType* shad = light->ActiveShadowType();
	IAreaShadowType* area = shad->GetAreaShadowType();

// If we aren't forcing Shadow Map and the shadow generator
// supports area shadows, then create the area shadow generator.
// The flags are the same as for CreateShdowGenerator
	if (!forceShadowBuf && area != NULL) {
		_areaShadGen = area->CreateAreaShadowGenerator(light, this,	SHAD_2SIDED)
	}
	else {
		_shadGen = shad->CreateShadowGenerator(light, this, SHAD_2SIDED);
	}
}
\endcode 
Sampling the area shadows is a little tricky to allow for some optimizaton.
This is an example of the code needed in AreaShadowLightObjDesc::Illuminate.
The variable, sampler, should be local to allow multithreading.
\code
AreaShadowSampler* sampler =_areaShadGen->InitializeSampler(alloca(_areaShadGen->GetSamplerSize()));
\endcode 
Once the sampler has been initialized, you can calculate the visibility between
any point on the light and the point being shaded by using:
\code
float atten = sampler->(sc, pointOnLight, shadedNormal, lightColor);
\endcode
The value of pointOnLight depends on the type of light we are sampling. If the
light is parallel, then pointOnLight needs to be in the local light
coordinates. If the light is not parallel, then pointOnLight needs to be in
camera coordinates. */
#pragma warning(push)
#pragma warning(disable:4239)
class ShadowType: public ReferenceTarget {
	public:
		SClass_ID SuperClassID() { return SHADOW_TYPE_CLASS_ID;}
		virtual ShadowParamDlg *CreateShadowParamDlg(Interface *ip) { return NULL; }
		virtual ShadowGenerator* CreateShadowGenerator(LightObject *l,  ObjLightDesc *ld, ULONG flags)=0;
		virtual BOOL SupportStdMapInterface() { return FALSE; }

		BOOL BypassPropertyLevel() { return TRUE; }  // want to promote shadowtype props to light level

		// If the shadow generator can handle omni's directly, this should return true. If it does,
		// then when doing an Omni light, the SHAD_OMNI flag will be passed in to 
		// the CreateShadowGenerator call, and only one ShadowGenerator will be created
		// instead of the normal 6 (up,down,right,left,front,back).
		virtual BOOL CanDoOmni() { return FALSE; }

		// This method used for converting old files: only needs to be supported by default 
		// shadow map and ray trace shadows.
		virtual void ConvertParamBlk( ParamBlockDescID *descOld, int oldCount, IParamBlock *oldPB ) { }

		// This method valid iff SupportStdMapInterface returns TRUE
		virtual int MapSize(TimeValue t) { return 512; } 

		// This interface is solely for the default shadow map type ( Class_ID(STD_SHADOW_MAP_CLASS_ID,0) )
		virtual void SetMapRange(TimeValue t, float f) {}
		virtual float GetMapRange(TimeValue t, Interval& valid = Interval(0,0)) { return 0.0f; }
		virtual void SetMapSize(TimeValue t, int f) {}
		virtual int GetMapSize(TimeValue t, Interval& valid = Interval(0,0)) { return 0; }
		virtual void SetMapBias(TimeValue t, float f) {} 
		virtual float GetMapBias(TimeValue t, Interval& valid = Interval(0,0)) { return 0.0f; }
		virtual void SetAbsMapBias(TimeValue t, int a) {}
		virtual int GetAbsMapBias(TimeValue t, Interval& valid = Interval(0,0)) { return 0; }

		// This interface is solely for the default raytrace shadow type ( Class_ID(STD_RAYTRACE_SHADOW_CLASS_ID,0) )
		virtual float GetRayBias(TimeValue t, Interval &valid = Interval(0,0)) { return 0.0f; }
		virtual	void SetRayBias(TimeValue t, float f) {}
		virtual int GetMaxDepth(TimeValue t, Interval &valid = Interval(0,0)) { return 1; } 
		virtual void SetMaxDepth(TimeValue t, int f) {}

		// Because this method is inlined and only uses existing methods
		// it doesn't break the SDK. Return the IAreaShadowType interface
		IAreaShadowType* GetAreaShadowType();
			
	};

#pragma warning(pop) // C4239

// This class carries the parameters for area shadows. It also creates
// an AreaShadowGenerator to process the shadows during rendering.
class IAreaShadowType : public BaseInterface {
	public:
		// Create the AreaShadowGenerator to process shadows during rendering
		virtual AreaShadowGenerator* CreateAreaShadowGenerator(LightObject *l,  ObjLightDesc *ld, ULONG flags)=0;

		// This method can be used to disable the area related controls
		// in the UI. It is used by area and linear lights to disable
		// these controls because the lights will control the area shadows.
		virtual void EnableAreaUI(bool onoff) {}

		// These are the area shadow parameters. You don't need to
		// implement these if you only want to use the AreaShadowGenerator interface.
		virtual float GetLength(TimeValue t) { return 0.0f; }
		virtual void SetLength(TimeValue t, float w) {}
		virtual float GetWidth(TimeValue t) { return 0.0f; }
		virtual void SetWidth(TimeValue t, float w) {}
	};

inline IAreaShadowType* ShadowType::GetAreaShadowType()
{
	return static_cast<IAreaShadowType*>(GetInterface(AREA_SHADOW_TYPE_INTERFACE_ID));
}

///////////////////////////////////////////////////////////////
//
//	This controls the default value for the 2Sided shadow attribute
//	of viz4 shadow generators. must be compile time const as it's
//  compiled into pb2's
//
#define TWO_SIDED_SHADOW_DEFAULT	FALSE


// This class generates the shadows. It only exists during render, one per instance of the light.
/*! \sa  Class ShadowType, Class ShadBufRenderer, Class RendContext, Class RenderGlobalContext, Class ShadeContext, Class Matrix3, Class Point3,  Class Color.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class is used by a Shadow Type plug-in to generate the shadows. It only
exists during a render, with one per instance of the light. Methods of this
class perform the shadow buffer creation and sampling.\n\n
The ShadowGenerator API allows for two methods of sampling: A generator can use
either a "generic" sampling method:\n\n
<b>float Sample(ShadeContext \&sc, Point3 \&norm, Color\& color);</b>\n\n
Or, if it the generator is to work with Volumetric lights, it must use the
following sampling shadow-map style interface:
\code
float Sample(ShadeContext &sc, float x, float y, float z, float xslope, float yslope);
BOOL QuickSample(int x, int y, float z);
FiltSample(int x, int y, float z, int level);
LineSample(int x1, int y1, float z1, int x2, int y2, float z2); 
\endcode
To indicate that the latter interface is used, the method
ShadowType::SupportStdMapInterface() must return TRUE;
 */
class ShadowGenerator: public MaxHeapOperators {
public:
	/*! \remarks Destructor. */
	virtual ~ShadowGenerator() {}

	/*! \remarks This method is called on every frame to create a new shadow buffer for
	that frame. For example, the objects in the scene will have moved to different
	position, etc., so a new shadow buffer will need to be set up. See
	Class ShadBufRenderer for a helper
		class used for generating shadow map buffers.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time for the update.\n\n
	<b>const RendContext\& rendCntxt</b>\n\n
	The render context -- this is used for the progress bar.\n\n
	<b>RenderGlobalContext *rgc</b>\n\n
	This is used to get an instance list.\n\n
	<b>Matrix3\& lightToWorld</b>\n\n
	The light to world space transformation matrix. This is not necessarily the
	same as that of the light.\n\n
	<b>float aspect</b>\n\n
	This is the aspect ratio for non-square buffers. The aspect gives the
	height/width ratio of the shadow rectangle. The shadow buffer bitmap is always
	the same number of pixels wide as it is high, but it can be mapped into a
	non-square rectangle.\n\n
	<b>float param</b>\n\n
	This is the field-of-view of the light in radians for perspective projections
	or the width in world coordinates for parallel projections.\n\n
	<b>float clipDist = DONTCLIP</b>\n\n
	This parameter specifies the far clipping distance for the light. This is used
	when the far distance attenuation is turned on, and can result in much more
	efficient shadow buffer creation. If you have a small scene in the middle of a
	large complex scene, and the small scene is lit by, for instance, a
	shadow-casting omni, if you don't use far attenuation the omni has to take into
	account the entire large scene in its shadow map. Using far attenuation will
	clip all this outside stuff. Also omnis free up any of their 6 shadow buffer
	that end up being empty, so this can save memory usage.
	\return  Nonzero on success; otherwise zero. */
	virtual int Update(
		TimeValue t,
		const RendContext& rendCntxt,   // Mostly for progress bar.
		RenderGlobalContext *rgc,       // Need to get at instance list.
		Matrix3& lightToWorld, // light to world space: not necessarly same as that of light
		float aspect,      // aspect
		float param,   	   // persp:field-of-view (radians) -- parallel : width in world coords
		float clipDist = DONT_CLIP  
		)=0;

	/*! \remarks If things such as automatic cubic maps or mirror are used,
	the rendering is done from several different points of view. This method is
	called to allow the view matrix to be computed and cached so it won't have
	to be computed over and over again. The shadow buffer caches the matrix
	that does the transformation from the current view coordinates into its
	coordinates.
	\par Parameters:
	<b>const Matrix3\& worldToCam</b>\n\n
	This is the direction the view is looking from. Object coordinates are
	relative to this 'camera'. This is not always a 'camera', it is just world
	to whatever view is needed, for example from a mirror.
	\return  Nonzero on success; otherwise zero. */
	virtual int UpdateViewDepParams(const Matrix3& worldToCam)=0;

	/*! \remarks This method is used to delete the memory associated with the
	buffer. */
	virtual void FreeBuffer()=0;
	/*! \remarks Call this to destroy the ShadowGenerator. */
	virtual void DeleteThis()=0; // call this to destroy the ShadowGenerator

	// Generic shadow sampling function
	// Implement this when ShadowType::SupportStdMapInterface() returns FALSE. 
	/*! \remarks Generic shadow sampling function. Implement this when
	<b>ShadowType::SupportStdMapInterface()</b> returns FALSE.\n\n
	This is the Sample method used for ray traced shadows, for example. It
	takes the color that would illuminate the surface if there were no shadows,
	and returns a modified value. The shade context provides the point on the
	surface (<b>sc.P()</b>) and <b>norm</b> is the normal to the surface.
	\par Parameters:
	<b>ShadeContext \&sc</b>\n\n
	The shade context provides the point on the surface (<b>sc.P()</b>).\n\n
	<b>Point3 \&norm</b>\n\n
	This is the normal to the surface.\n\n
	<b>Color\& color</b>\n\n
	The input color.
	\return  It returns an attenuation, where 1.0 indicates it is not in
	shadow, and 0.0 indicates it is in shadow.
	\par Default Implementation:
	<b>{ return 1.0f; }</b> */
	virtual float Sample(ShadeContext &sc, Point3 &norm, Color& color) { return 1.0f; }

	// Implement these methods when ShadowType::SupportStdMapInterface() returns TRUE. 
	// This interface allows illuminated atmospherics
	// Note: Sample should return a small NEGATIVE number ehen the sample falls outside of the shadow buffer, so
	//    the caller can know to take appropriate action.
	/*! \remarks Implement this method when
	<b>ShadowType::SupportStdMapInterface()</b> returns TRUE. This interface
	allows illuminated atmospherics.\n\n
	This method is called to determine how much the point (x, y, z) is in
	shadow. It returns an attenuation, where 1.0 indicates it is not in shadow,
	and 0.0 indicates it is in shadow, and potentially a small negative number.
	A small negative number should be returned when the sample falls outside of
	the buffer (this is needed in order to fix a problem occuring with Omni
	Lights when using shadow maps). All shadow generators that implement this
	function need to do this. The value itself isn't important, as long as it
	is negative and very small (for instance (-float(1.0e-30)).
	\par Parameters:
	<b>ShadeContext \&sc</b>\n\n
	The shade context.\n\n
	<b>float x</b>\n\n
	The x coordinate of the point to check. This point is normalized into
	shadow buffer space. For example if the shadow buffer was 256x256 a point
	at the center would be 128, 128.\n\n
	<b>float y</b>\n\n
	The y coordinate of the point to check. This point is normalized into
	shadow buffer space.\n\n
	<b>float z</b>\n\n
	The z coordinate of the point to check. This is the distance perpendicular
	to the light where 0.0 is right at the light.\n\n
	<b>float xslope</b>\n\n
	This indicates the slope of the surface relative to the shadow buffer in
	x.\n\n
	<b>float yslope</b>\n\n
	This indicates the slope of the surface relative to the shadow buffer in y.
	\par Default Implementation:
	<b>{ return 1.0f; }</b> */
	virtual	float Sample(ShadeContext &sc, float x, float y, float z, float xslope, float yslope) { return 1.0f; }
	/*! \remarks Implement this method when
	<b>ShadowType::SupportStdMapInterface()</b> returns TRUE. This interface
	allows illuminated atmospherics.\n\n
	This method determines if the given point is in a shadow. It samples a
	single pixel in the shadow map.
	\par Parameters:
	<b>int x</b>\n\n
	The x coordinate of the point to check. This point is normalized into
	shadow buffer space. For example if the shadow buffer was 256x256 a point
	at the center would be 128, 128.\n\n
	<b>int y</b>\n\n
	The y coordinate of the point to check. This point is normalized into
	shadow buffer space.\n\n
	<b>float z</b>\n\n
	The z coordinate of the point to check. This is the distance perpendicular
	to the light where 0.0 is right at the light.
	\return  TRUE if the point is in shadow; otherwise FALSE.
	\par Default Implementation:
	<b>{ return 1; }</b> */
	virtual BOOL QuickSample(int x, int y, float z) { return 1; }
	/*! \remarks Implement this method when
	<b>ShadowType::SupportStdMapInterface()</b> returns TRUE. This interface
	allows illuminated atmospherics.\n\n
	This method is called to determine how much the point (x, y, z) is in
	shadow. It returns an attenuation, where 1.0 indicates it is not in shadow,
	and 0.0 indicates it is in shadow. The method <b>QuickSample()</b> above
	looks at a single pixel in the shadow buffer. This method looks at either 4
	or 8 pixels (based on the level parameter) to compute the result. The
	center pixel is given the highest weighting, while the other pixels are
	given lesser weightings. However this method is still fairly quick, since
	it doesn't base the weighting on the location within the pixel. This is in
	contrast to the Sample() method above, where the blending of the adjacent
	pixels is weighted by the position within the sub-pixel.
	\par Parameters:
	<b>int x</b>\n\n
	The x coordinate of the point to check. This point is normalized into
	shadow buffer space. For example if the shadow buffer was 256x256 a point
	at the center would be 128, 128.\n\n
	<b>int y</b>\n\n
	The y coordinate of the point to check. This point is normalized into
	shadow buffer space.\n\n
	<b>float z</b>\n\n
	The z coordinate of the point to check. This is the distance perpendicular
	to the light where 0.0 is right at the light.\n\n
	<b>int level</b>\n\n
	This may be 0 or 1. If 0, four neighboring pixels are blended in. If 1,
	eight neighboring pixels are blended in.
	\return  A value in the range 0.0 to 1.0.
	\par Default Implementation:
	<b>{ return 1.0f; }</b> */
	virtual float FiltSample(int x, int y, float z, int level) { return 1.0f; }
	/*! \remarks Implement this method when
	<b>ShadowType::SupportStdMapInterface()</b> returns TRUE. This interface
	allows illuminated atmospherics.\n\n
	This method is called to sample the shadow map along a line segment. It
	uses a line between x1, y1 and x2, y2. The z values are interpolated
	between z1 and z2 and compared to the z value in the shadow map for that
	pixel.
	\par Parameters:
	<b>int x1</b>\n\n
	The start x coordinate of the line. This point is normalized into shadow
	buffer space. For example if the shadow buffer was 256x256 a point at the
	center would be 128, 128.\n\n
	<b>int y1</b>\n\n
	The start y coordinate of the line. This point is normalized into shadow
	buffer space.\n\n
	<b>float z1</b>\n\n
	The start z coordinate of the line. This is the distance perpendicular to
	the light where 0.0 is right at the light.\n\n
	<b>int x2</b>\n\n
	The end x coordinate of the line. This point is normalized into shadow
	buffer space.\n\n
	<b>int y2</b>\n\n
	The end y coordinate of the line. This point is normalized into shadow
	buffer space.\n\n
	<b>float z2</b>\n\n
	The end z coordinate of the line. This is the distance perpendicular to the
	light where 0.0 is right at the light.
	\return  A value in the range 0.0 to 1.0 which represents how much of the
	ray was inside the light and how much was outside the light.
	\par Default Implementation:
	<b>{ return 1.0f; }</b> */
	virtual float LineSample(int x1, int y1, float z1, int x2, int y2, float z2) { return 1.0f; }

	};

#pragma warning(pop) // C4100

// For performance reasons, the area shadow generator is broken into two
// parts, the AreaShadowGenerator and the AreaShadowSampler. This is to allow
// some coherency in the multiple samples that are required to sample
// and area light. The AreaShadowGenerator returns the size of the AreaShadowSampler,
// which is allocated by the caller. The AreaShadowGenerator then initializes
// the allocate memory and return the address of the sampler. The destructor
// for the AreaShadowSampler is not called when the memory is freed. The
// AreaShadowSampler may not be used by multiple threads.

class AreaShadowSampler;

// The class generates area shadows. It only exists during render, one per instance of the light.
class AreaShadowGenerator : public ShadowGenerator {
	public:
		// Get the size of the sampler.
		virtual int GetSamplerSize() = 0;

		// Initialize the sampler and return a pointer to it. Memory must
		// be allocated by the caller and must be at least GetSamplerSize.
		virtual AreaShadowSampler* InitializeSampler(void* memory, ShadeContext& sc, bool antialias) = 0;

		// Return the number of samples we should use for
		// determining visibility of an area.
		virtual int GetNumSamples() { return 1; }
	};

// The class samples area shadows. It only exists during render, and is dynamically
// allocated on the stack using _alloca.
class AreaShadowSampler: public MaxHeapOperators {
	public:
		virtual ~AreaShadowSampler() {}
		// Sample the area shadow generator. This call samples the occlusion
		// from sourcePnt to the point being shaded. SourcePnt needs to be
		// in local light space coordinates if the light is parallel,
		// and in camera space coordinate if the light is not parallel.
		// If antialias is true, the result is antialiased using the settings
		// for the generator. If antialias is false a single sample is returned.
		// The default implementation ignores the sourcePnt and anitalias
		// arguments and simply returns the result of the standard sampler.
		virtual float Sample(
			ShadeContext&	sc,
			const Point3&	sourcePnt,
			Point3&			norm,
			Color&			color
			) = 0;
	};

// This returns a new default shadow-map shadow generator
/*! \remarks This global function returns a new default shadow-map shadow
generator. */
CoreExport ShadowType *NewDefaultShadowMapType();

// This returns a new default ray-trace shadow generator
/*! \remarks This global function returns a new default ray-trace shadow
generator.  */
CoreExport ShadowType *NewDefaultRayShadowType();


