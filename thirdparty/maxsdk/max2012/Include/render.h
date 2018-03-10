//
// Copyright 2010 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.  
//
//


#pragma once

// includes
#include "maxheap.h"
#include "sfx.h"
#include "buildver.h"
#include "iTargetedIO.h"
#include "RendType.h"
#include "object.h"

#define FIELD_EVEN 0
#define FIELD_ODD 1

#pragma warning(push)
#pragma warning(disable:4100)

/*! 
Describes a default light. An array of these default lights is
passed into the method Renderer::Open(). \n\n
Note: In 3ds Max 3.0 the production renderer has been modified so that if a
DefaultLight is passed into Renderer::Open() with a transformation
matrix that is all zeros, the renderer will interpret this to mean that on each
frame it should create a light located at the view point, pointing in the view
direction. This allows the implementation of the new viewport 1-light option so
that it tracks the camera during an animated camera move. 
\see  Class Renderer, Class Matrix3, Structure LightState.
*/
class DefaultLight: public MaxHeapOperators
{
public:
	LightState ls;	//!< Describes the properties of the light.
	Matrix3 tm;		//!< The transformation of the light that controls its position in the scene.
};

/*! Describes the properties of a view that is being rendered. These are
properties such as the type of view (parallel or perspective), its clipping
distances, width, height, zoom factor, field-of-view, etc.
\par Data Members:
<b>Matrix3 prevAffineTM;</b>\n\n
This is the world space to camera space transformation matrix computed 2 ticks
before the <b>affineTM</b> matrix below.\n\n
<b>Matrix3 affineTM;</b>\n\n
This matrix will take a point in world space and convert it to camera space (or
world to view space if it's a viewport). The camera coordinates are set up
looking down the -Z axis, X is to the right, and Y is up.\n\n
<b>int projType;</b>\n\n
One of the following values:\n\n
<b>PROJ_PERSPECTIVE</b>\n\n
The view is a perspective projection.\n\n
<b>PROJ_PARALLEL</b>\n\n
The view is a parallel projection.\n\n
<b>float hither, yon;</b>\n\n
The hither and yon clipping distances.\n\n
<b>float distance;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
The distance from the view point to the image (view) plane.\n\n
<b>float zoom;</b>\n\n
The zoom factor of the viewport for parallel projection. The zoom factor gives
the amount of magnification relative to a standard view width of 400 pixels.
This is best explained via the following code fragment:
ComputeViewParams() computes the projection factors for a given view,
and MapToScreen() applies these factors to map a point from 3D camera
coordinates to 2D screen coordinates.\code

#define VIEW_DEFAULT_WIDTH ((float)400.0)
void SRendParams::ComputeViewParams(const ViewParams&vp)
{
	if (vp.projType == PROJ_PERSPECTIVE) {
		float fac = -(float)(1.0 / tan(0.5*(double)vp.fov));
		xscale = fac*dw2;		 // dw2 = float(devWidth)/2.0
		yscale = -devAspect*xscale;
	}
	else {
		xscale = (float)devWidth/(VIEW_DEFAULT_WIDTH*vp.zoom);
		yscale = -devAspect*xscale;
	}
}

Point2 SRendParams::MapToScreen(Point3 p)
{
	Point2 s;
	if (proj_type==PROJ_PERSPECTIVE) {
		s.x = dw2 + xscale*p.x/p.z;
		s.y = dh2 + yscale*p.y/p.z;
	}
	else {
		s.x = dw2 + xscale*p.x;
		s.y = dh2 + yscale*p.y;
	}
	return s;
}
\endcode 
<b>float fov;</b>\n\n
Field of view in radians for perspective projections.\n\n
<b>float nearRange;</b>\n\n
This data member is available in release 2.0 and later only.\n\n
The near environment range setting (used for fog effects).\n\n
<b>float farRange;</b>\n\n
This data member is available in release 2.0 and later only.\n\n
The far environment setting (used for fog effects).  
 \see  Class Matrix3, Class Renderer, Class RendParams.*/
class ViewParams : public BaseInterfaceServer {
	public:
		Matrix3 prevAffineTM; // world space to camera space transform 2 ticks previous 
		Matrix3 affineTM;  // world space to camera space transform
		int projType;      // PROJ_PERSPECTIVE or PROJ_PARALLEL
		float hither,yon;
		float distance; // to view plane
		// Parallel projection params
		float zoom;  // Zoom factor 
		// Perspective params
		float fov; 	// field of view
		float nearRange; // for fog effects
		float farRange;  // for fog effects
		// Generic expansion function
		/*! This is a general purpose function that allows the API to be extended in the
		future. The 3ds Max development team can assign new <b>cmd</b> numbers and
		continue to add functionality to this class without having to 'break' the API.
		\par Parameters:
		<b>int cmd</b>\n\n
		The index of the command to execute.\n\n
		<b>ULONG_PTR  arg1=0</b>\n\n
		Optional argument 1. See the documentation where the <b>cmd</b> option is
		discussed for more details on these parameters.\n\n
		<b>ULONG_PTR  arg2=0</b>\n\n
		Optional argument 2.\n\n
		<b>ULONG_PTR  arg3=0</b>\n\n
		Optional argument 3.
		\return  An integer return value. See the documentation where the <b>cmd</b>
		option is discussed for more details on the meaning of this value. */
		virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) { return 0; } 
	};


// Common renderer parameters

/*
class Atmospheric;
class IRenderElement;
*/
class RadiosityEffect;
class ToneOperator;

/// \defgroup extraFlags_field_of_RendParams Extra Flags Field of RendParams
//@{
/** Removes a fix made for R3 that  cleaned up the edges of shadows where objects intersect-- DS 8/28/00
 * This goes in the extraFlags field of RendParams.*/
#define RENDER_R25SHADOWS    		  (1L << 2)

/** Tell the renderer to hide frozen objects
 * This goes in the extraFlags field of RendParams. */
#define RENDER_HIDE_FROZEN    		  (1L << 3)
//@}

// These parameters are passed to the renderer when the renderer is opend.
/*!
This class has a set of data members, and these parameters are passed to the
renderer when the renderer is opened. All methods of this class are implemented
by the system.
\par Data Members:
<b>RendType rendType;</b>\n\n
The type of rendering to perform. \n\n
<b>BOOL isNetRender;</b>\n\n
Determines if this is a render on a network slave.\n\n
<b>BOOL fieldRender;</b>\n\n
If TRUE the image will be field rendered; otherwise frame rendered.\n\n
<b>int fieldOrder;</b>\n\n
The field order used. One of the following values:\n\n
<b>0</b> specifies even.\n\n
<b>1</b> specifies odd.\n\n
<b>TimeValue frameDur;</b>\n\n
This is used, for example, by video post. In video post you can stretch time. A
video post frame might be 1/2 frame long for example. This data member defines
the duration of one frame in TimeValue units.\n\n
<b>BOOL colorCheck;</b>\n\n
Determines if the color is ranged checked.\n\n
<b>int vidCorrectMethod;</b>\n\n
Video correction method. One of the following values:\n\n
<b>0</b> specifies FLAG (with black).\n\n
<b>1</b> specifies SCALE_LUMA (scale luminance).\n\n
<b>2</b> specifies SCALE_SAT (scale saturation).\n\n
<b>int ntscPAL;</b>\n\n
Determines if the color is range checked using NTSC or PAL standards. One of
the following values:\n\n
<b>0</b> specifies NTSC.\n\n
<b>1</b> specifies PAL.\n\n
<b>BOOL superBlack;</b>\n\n
If TRUE Super Black is used.\n\n
<b>int sbThresh;</b>\n\n
Specifies the Super Black threshold.\n\n
<b>BOOL rendHidden;</b>\n\n
If TRUE hidden objects are rendered.\n\n
<b>BOOL force2Side;</b>\n\n
If TRUE two sided materials are used for all items in the scene.\n\n
<b>BOOL inMtlEdit;</b>\n\n
If TRUE the rendering is taking place in the material editor.\n\n
<b>float mtlEditTile;</b>\n\n
If rendering is taking place in the material editor, scale tiling.\n\n
<b>BOOL mtlEditAA;</b>\n\n
If TRUE antialiasing should be done in the material editor.\n\n
<b>BOOL multiThread;</b>\n\n
This is used internally.\n\n
<b>BOOL useEnvironAlpha;</b>\n\n
If TRUE one should use alpha from the environment map.\n\n
<b>BOOL dontAntialiasBG;</b>\n\n
If the low-order bit is set don't antialias against the background (this is
often used for 'sprites' in video games). For 3ds Max 1.1 and 1.2 (in 2.0 and
later see <b>scanBandHeight</b> below), this parameter may also be used to
access the height of the abuffer in scan lines. This may be obtained using the
following syntax:
\code
abufBandHeight = rendpar.dontAntialiasBG>>8;
\endcode 
<b>BOOL useDisplacement;</b>\n\n
The apply displacement mapping setting.\n\n
<b>bool useRadiosity;</b>\n\n
This data member is available in release 4.0 and later only.\n\n
Indicates if radiosity should be included in rendering.\n\n
<b>bool computeRadiosity;</b>\n\n
This data member is available in release 4.0 and later only.\n\n
Indicates if radiosity should be computed before rendering.\n\n
<b>Texmap *envMap;</b>\n\n
The environment map. This may be NULL.\n\n
<b>Atmospheric *atmos;</b>\n\n
The atmosphere effects. This may be NULL. To the renderer it looks like there
is only one atmosphere. You can use this atmosphere and it will go ahead and
call all the individual atmospheric effects for you.\n\n
<b>Effect *effect;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
The post-processing effects. This may be NULL if there aren't any.\n\n
<b>RadiosityEffect* pRadiosity;</b>\n\n
This data member is available in release 4.0 and later only.\n\n
Points to the radiosity effect.\n\n
<b>ToneOperator* pToneOp;</b>\n\n
This data member is available in release 4.0 and later only.\n\n
Points to the tone operator if present. This may be NULL\n\n
<b>MCHAR biFileName[MAX_PATH];</b>\n\n
This data member is available in release 4.0 and later only.\n\n
The bitmap output file name.\n\n
<b>TimeValue firstFrame;</b>\n\n
This data member is available in release 2.0 and later only.\n\n
This is the first frame that will be rendered. This lets Open() know the
first frame that will be rendered, so it will not have to evaluate at frame 0
when building.\n\n
<b>int scanBandHeight;</b>\n\n
This data member is available in release 2.0 and later only.\n\n
This is the height of a scan band (for the default 3ds Max scanline
renderer).\n\n
<b>ULONG extraFlags;</b>\n\n
This data member is available in release 2.0 and later only.\n\n
<b>RENDER_HIDE_FROZEN</b>\n\n
This option is available in release 4.0 and later only.\n\n
Instruct the renderer to hide frozen objects\n\n
<b>int width;</b>\n\n
The image height.\n\n
<b>int height;</b>\n\n
The image width.\n\n
<b>BOOL filterBG;</b>\n\n
The filter background.  
\see  Class Atmospheric, Class Effect, Class IRenderElementMgr, Class ToneOperator.
*/
class RendParams: public MaxHeapOperators
{
private:
	IRenderElementMgr *mpIRenderElementMgr; // The render element manager, may be NULL -- declared in maxsdk/include/RenderElements.h

public:
	/** Render modes */
	enum RenderMode
	{
		RM_Default,			//!< normal rendering mode
		RM_IReshade,		//!< render is being used for interactive reshading
	};

	/** List of commands that can be passed to Execute()*/
	enum Execute_Commands {
		/** Queries whether this is a tone operator preview rendering. 
		 * Execute() will return non-zero if it is indeed a preview render. None of the "arg" parameters are used.*/
		kCommand_IsToneOpPreview = 0x4e80014c
	};

	RendType rendType;	 	// normal, region, blowup, selection

	// JOHNSON RELEASE SDK
	//BOOL isIterativeRender;		// is this an iterative mode render?
	BOOL isNetRender;  		// is this a render on a network slave?	
	BOOL fieldRender;
	int fieldOrder;    		// 0->even, 1-> odd
	TimeValue frameDur; 	// duration of one frame
	
	BOOL colorCheck;
	int vidCorrectMethod; 	// 0->FLAG, 1->SCALE_LUMA 2->SCALE_SAT
	int ntscPAL;  			// 0 ->NTSC,  1 ->PAL
	BOOL superBlack;		// impose superBlack minimum intensity?
	int sbThresh;			// value used for superBlack
	BOOL rendHidden;		// render hidden objects?
	BOOL force2Side;		// force two-sided rendering?
	BOOL inMtlEdit;	  		// rendering in the mtl editor?
	float mtlEditTile; 		// if in mtl editor, scale tiling by this value
	BOOL mtlEditAA;   		// if in mtl editor, antialias? 
	BOOL multiThread; 		// for testing only
	BOOL useEnvironAlpha;  	// use alpha from the environment map.
	BOOL dontAntialiasBG; 	// Don't antialias against background (for video games)		
	BOOL useDisplacement; 	// Apply displacment mapping		
	bool useRadiosity;		// Include radiosity in rendering
	bool computeRadiosity;	// Compute radiosity before rendering
	Texmap *envMap;			// The environment map, may be NULL
	Atmospheric *atmos; 	// The atmosphere effects, may be NULL.
	Effect *effect; 	    // The postprocessing effects, may be NULL.
	RadiosityEffect* pRadiosity;	// The radiosity effect
	ToneOperator* pToneOp;	// The tone operator, may be NULL
	TimeValue firstFrame; 	// The first frame that will be rendered
	int scanBandHeight;		// height of a scan band (default scanline renderer)
	ULONG extraFlags;		// for expansion
	int width,height;		// image width,height.
	BOOL filterBG;			// filter background
	BOOL alphaOutOnAdditive;// produce alpha on additive transparency
#ifdef SIMPLIFY_AREA_LIGHTS
	bool simplifyAreaLights;
#endif

	/*! Constructor. The initialization can be seen in the header
	file RENDER.H. */
	RendParams()
	{
		rendType = RENDTYPE_NORMAL;
		isNetRender = FALSE;
		fieldRender = FALSE;
		fieldOrder = 0;
		frameDur = 0;
		colorCheck = 0;
		vidCorrectMethod = 0;
		ntscPAL = 0;
		superBlack = 0;
		sbThresh = 0;
		rendHidden = 0;
		force2Side = 0;
		inMtlEdit = 0;
		mtlEditTile = 0;
		mtlEditAA = 0;
		multiThread = 0;
		useEnvironAlpha = 0;
		dontAntialiasBG = 0;
		useDisplacement = 0;
		useRadiosity = true;
		computeRadiosity = true;
		envMap = NULL;
		atmos = NULL;
		mpIRenderElementMgr = NULL;
		effect = NULL;
		pRadiosity = NULL;
		pToneOp = NULL;
		firstFrame = 0;
		scanBandHeight = 0;
		extraFlags = 0;
		width=height = 0;
		filterBG = 0;
#ifdef SIMPLIFY_AREA_LIGHTS
		simplifyAreaLights = false;
#endif
	}

	/*! Destructor. */
	virtual ~RendParams() {}

	/*! Returns the rendering mode. One of the following values:\n\n
	<b>RM_Default</b>\n\n
	This is being used for a normal rendering.\n\n
	<b>RM_IReshade</b>\n\n
	The render is being used for interactive reshading. */
	RenderMode GetRenderMode() { return RM_Default; } // mjm - 06.08.00
	/*! Sets the render element manager used.
	\par Parameters:
	<b>IRenderElementMgr *pIRenderElementMgr</b>\n\n
	Points to the render element manager to set. */
	void SetRenderElementMgr(IRenderElementMgr *pIRenderElementMgr) { mpIRenderElementMgr = pIRenderElementMgr; } // mjm - 06.30.00

#define RP_ANTIALIAS_OFF 200 //!< An index to an external command. Can be accepted as the argument to Execute() 

	/*! Returns a pointer to the render element manager interface. */
	IRenderElementMgr *GetRenderElementMgr() { 
		return( Execute(RP_ANTIALIAS_OFF) ? NULL : mpIRenderElementMgr); 
	}

	/*! This is a general purpose function that allows the API to be extended in the
	future. The 3ds Max development team can assign new <b>cmd</b> numbers and
	continue to add functionality to this class without having to 'break' the API.
	\par Parameters:
	<b>int cmd</b>\n\n
	The index of the command to execute.\n\n
	<b>ULONG_PTR arg1=0</b>\n\n
	Optional argument 1. See the documentation where the <b>cmd</b> option is
	discussed for more details on these parameters.\n\n
	<b>ULONG_PTR arg2=0</b>\n\n
	Optional argument 2.\n\n
	<b>ULONG_PTR arg3=0</b>\n\n
	Optional argument 3.
	\return  An integer return value. See the documentation where the <b>cmd</b>
	option is discussed for more details on the meaning of this value. */
	virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) { return 0; }

	// Call this function to determine whether we are doing a tone operator preview rendering
	bool IsToneOperatorPreviewRender();
};

inline bool RendParams::IsToneOperatorPreviewRender() {

	// Execute the special command on the render params
	INT_PTR result = Execute(kCommand_IsToneOpPreview);
	return (result != 0);
}

// These are passed to the renderer on every frame
/*! This is passed to the renderer on every frame. This provides information about
the ambient light color, the background color, the duration of one frame, etc.
\par Data Members:
<b>Color ambient;</b>\n\n
The ambient light color.\n\n
<b>Color background;</b>\n\n
The background color.\n\n
<b>Color globalLightLevel;</b>\n\n
This data member is available in release 2.0 and later only.\n\n
This is a multiplier that scales the brightness of all scene lights: it doesn't
affect the ambient light level.\n\n
<b>float frameDuration;</b>\n\n
The duration of one frame in units of current frames. This describes how much
scene time is used by one (video) frame time. For instance, in Video Post, you
can scale time so it's stretched out or compressed. In this case, you may be
rendering one frame to video, but because the scene is being run at a faster
speed than normal, you in fact see say 2.5 frames of the scene. Things such as
field rendering or motion blur must know about this so they know how long a
frame is in terms of the time of the scene.\n\n
<b>float relSubFrameDuration;</b>\n\n
This data member is available in release 2.0 and later only.\n\n
This is the relative fraction of <b>frameDuration</b> used by a subframe.
Within the <b>frameDuration</b>, if you're sub-dividing the rendering up into
multiple frames for scene motion blur, this may be used.\n\n
For instance, say the duration (frames) is set to 0.5 and the duration
subdivisions is 5, then this data member would be 0.1. This means that each
subframe is effectively covering only 1/10th of the frame duration.\n\n
This value is always less than 1.0.\n\n
<b>int regxmin, regxmax;</b>\n\n
These values were members of Class RendParams prior to release 3.\n\n
The x min and max boundary values for render region or render blowup in device
coordinates.\n\n
<b>int regymin, regymax;</b>\n\n
These values were members of Class RendParams prior to release 3.\n\n
The y min and max boundary values for render region or render blowup in device
coordinates.\n\n
<b>Point2 blowupCenter;</b>\n\n
This parameter is available in release 3.0 and later only.\n\n
The 2D point at the center of the render blowup region.\n\n
<b>Point2 blowupFactor;</b>\n\n
This parameter is available in release 3.0 and later only.\n\n
The X and Y scale factors for render blowup.  
\see  Class RendParams, Class Color,  Class Point2.*/
class FrameRendParams : public BaseInterfaceServer {
	public:
	Color ambient;
	Color background;
	Color globalLightLevel;
	float frameDuration; // duration of one frame, in current frames
	float relSubFrameDuration;  // relative fraction of frameDuration used by subframe.

	// boundaries of the region for render region or crop (device coords).
	int regxmin,regxmax;
	int regymin,regymax;

	// parameters for render blowup.
	Point2 blowupCenter;
	Point2 blowupFactor;

	/*! Constructor. The <b>frameDuration</b> is set to 1.0 and
	<b>relSubFrameDuration = 1.0f</b>. */
	FrameRendParams() { frameDuration = 1.0f; relSubFrameDuration = 1.0f; }
	/*! This is a general purpose function that allows the API to be extended in the
	future. The 3ds Max development team can assign new <b>cmd</b> numbers and
	continue to add functionality to this class without having to 'break' the API.
	\par Parameters:
	<b>int cmd</b>\n\n
	The index of the command to execute.\n\n
	<b>ULONG_PTR arg1=0</b>\n\n
	Optional argument 1. See the documentation where the <b>cmd</b> option is
	discussed for more details on these parameters.\n\n
	<b>ULONG_PTR arg2=0</b>\n\n
	Optional argument 2.\n\n
	<b>ULONG_PTR arg3=0</b>\n\n
	Optional argument 3.
	\return  An integer return value. See the documentation where the <b>cmd</b>
	option is discussed for more details on the meaning of this value. */
	virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) { return 0; } 
	};

// Since this dialog is modless and non-interactive, as the user changes
// parameters in the dialog, the renderer does not need to update it's
// state. When the user is through, they may choose 'OK' or 'Cancel'.
//
// If the user OKs the dialog, AcceptParams() will be called, at which time the
// renderer can read the parameter out of the UI and modify its state.
// 
// If RejectParams() is called, typically the renderer will not have to do anything
// since it has not yet modify its state, but if for some reason it has, it
// should restore its state.
/*! 
An instance of this class is created by Renderer::CreateParamDlg().
Since this dialog is modeless and non-interactive, as the user changes
parameters in the dialog, the renderer does not need to update its state. When
the user is through, they may choose 'OK' or 'Cancel' from the dialog. If the
user selects OK then the AcceptParams() method will be called. If the
user selects Cancel, then the RejectParams() method is called.  
\see  Class Renderer.
*/
class RendParamDlg: public MaxHeapOperators {
	public:
		/*! Destructor. */
		virtual ~RendParamDlg() { }
		/*! If the user selects OK from the dialog, this method will be called, at
		which time the renderer can read the parameters out of the UI and
		modify its state. */
		virtual void AcceptParams()=0;
		/*! If this method is called, typically the renderer will not have to do
		anything since it has not yet modified its state, but if for some
		reason it has, it should restore its state.
		\par Default Implementation:
		<b>{}</b> */
		virtual void RejectParams() {}
		/*! Deletes this instance of the class. The instance is allocated by
		Renderer::CreateParamDialog(). */
		virtual void DeleteThis()=0;		
	};


/** \defgroup Material_Browser_Flags Material Browser Flags 
 *Flag bits for DoMaterialBrowseDlg() */
//@{
#define BROWSE_MATSONLY		(1<<0)	//!< Materials only
#define BROWSE_MAPSONLY		(1<<1)	//!< Maps only
#define BROWSE_INCNONE		(1<<2) 	//!< Include 'None' as an option
#define BROWSE_INSTANCEONLY	(1<<3) 	//!< Only allow instances, no copy
#define BROWSE_TO_MEDIT_SLOT (1<<4) //!< browsing to medit slot
#define BROWSE_EXISTING_ONLY (1<<5)	//!< Browse for existing materials only; no new materials.
//@}
 
// passed to SetPickMode. This is a callback that gets called as
// the user tries to pick objects in the scene.
/*! An instance of this class is passed to IRendParams::SetPickMode(). This
is a callback that gets called as the user tries to pick objects in the scene.
 \see  Class IRendParams.
 */
class RendPickProc: public MaxHeapOperators
{
public:
	/*! Destructor. */
	virtual ~RendPickProc() { }

	// Called when the user picks something.
	// return TRUE to end the pick mode.
	/*! Called when the user picks something.
	\par Parameters:
	<b>INode *node</b>\n\n
	The node that was selected.
	\return  TRUE to end the pick mode; FALSE to continue. */
	virtual BOOL Pick(INode *node)=0;

	// Return TRUE if this is an acceptable hit, FALSE otherwise.
	/*! Return TRUE if this is an acceptable hit; otherwise FALSE.
	\par Parameters:
	<b>INode *node</b>\n\n
	The node that was selected. */
	virtual BOOL Filter(INode *node)=0;

	// These are called as the mode is entered and exited
	/*! This method is called as the mode is entered.
	\par Default Implementation:
	<b>{}</b> */
	virtual void EnterMode() {}
	/*! This method is called when the mode is exited.
	\par Default Implementation:
	<b>{}</b> */
	virtual void ExitMode() {}

	// Provides two cursor, 1 when over a pickable object and 1 when not.
	/*! Returns the handle of the default cursor. This is the cursor to use when
	the user is not over a pickable object.
	\par Default Implementation:
	<b>{return NULL;}</b> */
	virtual HCURSOR GetDefCursor() {return NULL;}
	/*! Returns the handle of the hit cursor. This is the cursor to use when the
	user IS over a pickable object.
	\par Default Implementation:
	<b>{return NULL;}</b> */
	virtual HCURSOR GetHitCursor() {return NULL;}

	// Return TRUE to allow the user to pick more than one thing.
	// In this case the Pick method may be called more than once.
	/*! \remarks	Implement this method to return TRUE to allow the user to pick more than
	one thing. In that case the Pick() method may be called more than
	once.
	\return  TRUE to allow multiple picks; otherwise FALSE.
	\par Default Implementation:
	<b>{return FALSE;}</b> */
	virtual BOOL AllowMultiSelect() {return FALSE;}
};

class ITabbedDialog;
class ITabPage;

// This is the interface given to a renderer when it needs to display its parameters
// It is also given to atmospheric effects to display thier parameters.
/*! This is the interface given to a renderer, or atmospheric effect when it needs
to display its parameters. For a renderer, the Render Scene dialog may be
extended using this class. All methods of this class are implemented by the
system.  
 \see  Class TimeChangeCallback,  Class RendPickProc.*/
class IRendParams : public InterfaceServer
{
public:
	// The current position of the frame slider
	/*! Returns the current position of the frame slider. */
	virtual TimeValue GetTime()=0;

	/*! Register a callback object that will get called every time
	the current animation time is changed.
	\par Parameters:
	<b>TimeChangeCallback *tc</b>\n\n
	The callback object to register. */
	virtual void RegisterTimeChangeCallback(TimeChangeCallback *tc)=0;
	/*! Un-registers a callback object registered using
	RegisterTimeChangeCallback().
	\par Parameters:
	<b>TimeChangeCallback *tc</b>\n\n
	The callback object to un-register. */
	virtual void UnRegisterTimeChangeCallback(TimeChangeCallback *tc)=0;

	// Brings up the material browse dialog allowing the user to select a material.
	// newMat will be set to TRUE if the material is new OR cloned.
	// Cancel will be set to TRUE if the user cancels the dialog.
	// The material returned will be NULL if the user selects 'None'
	/*! Brings up the material browse dialog allowing the user to
	select a material.
	\par Parameters:
	<b>HWND hParent</b>\n\n
	The parent window handle.\n\n
	<b>DWORD flags</b>\n\n
	See \ref Material_Browser_Flags .\n\n
	<b>BOOL \&newMat</b>\n\n
	Set to TRUE if the material is new OR cloned; otherwise FALSE.\n\n
	<b>BOOL \&cancel</b>\n\n
	Set to TRUE if the user cancels the dialog; otherwise FALSE.
	\return  The material returned will be NULL if the user selects 'None' */
	virtual MtlBase *DoMaterialBrowseDlg(HWND hParent,DWORD flags,BOOL &newMat,BOOL &cancel)=0;

	// Adds rollup pages to the render params dialog. Returns the window
	// handle of the dialog that makes up the page.
	/*! This method adds rollup pages to the dialog and returns the
	window handle of the page.
	\par Parameters:
	<b>HINSTANCE hInst</b>\n\n
	The DLL instance handle of the plug-in.\n\n
	<b>MCHAR *dlgTemplate</b>\n\n
	The dialog template for the rollup page.\n\n
	<b>DLGPROC dlgProc</b>\n\n
	The dialog proc to handle the message sent to the rollup page.\n\n
	<b>MCHAR *title</b>\n\n
	The title displayed in the title bar.\n\n
	<b>LPARAM param=0</b>\n\n
	Any specific data to pass along may be stored here. This may be later
	retrieved using the GetWindowLong() call from the Windows API.\n\n
	<b>DWORD flags=0</b>\n\n
	<b>APPENDROLL_CLOSED</b>\n\n
	Starts the page in the rolled up state.\n\n
	<b>int category = ROLLUP_CAT_STANDARD</b>\n\n
	The category parameter provides flexibility with regard to where a
	particular rollup should be displayed in the UI. RollupPanels with lower
	category fields will be displayed before RollupPanels with higher category
	fields. For RollupPanels with equal category value the one that was added
	first will be displayed first. Allthough it is possible to pass any int
	value as category there exist currently 5 different category defines:
	<b>ROLLUP_CAT_SYSTEM</b>, <b>ROLLUP_CAT_STANDARD</b>, and
	<b>ROLLUP_CAT_CUSTATTRIB</b>.\n\n
	When using <b>ROLLUP_SAVECAT</b>, the rollup page will make the provided
	category sticky, meaning it will not read the category from the
	<b>RollupOrder.cfg</b> file, but rather save the category field that was
	passed as argument in the <b>CatRegistry</b> and in the
	<b>RollupOrder.cfg</b> file.\n\n
	The method will take the category of the replaced rollup in case the flags
	argument contains <b>ROLLUP_USEREPLACEDCAT</b>. This is mainly done, so
	that this system works with param maps as well.
	\return  The window handle of the rollup page. */
	virtual HWND AddRollupPage(
		HINSTANCE hInst, 
		MCHAR *dlgTemplate, 
		DLGPROC dlgProc, 
		MCHAR *title, 
		LPARAM param=0, 
		DWORD flags=0, 
		int category = ROLLUP_CAT_STANDARD)=0;

	/*! \remarks	This method adds rollup pages to the dialog and returns the window handle
	of the page. This method is currently not being used.
	\par Parameters:
	<b>HINSTANCE hInst</b>\n\n
	The DLL instance handle of the plug-in.\n\n
	<b>DLGTEMPLATE *dlgTemplate</b>\n\n
	The dialog template for the rollup page.\n\n
	<b>DLGPROC dlgProc</b>\n\n
	The dialog proc to handle the message sent to the rollup page.\n\n
	<b>MCHAR *title</b>\n\n
	The title displayed in the title bar.\n\n
	<b>LPARAM param=0</b>\n\n
	Any specific data to pass along may be stored here. This may be later
	retrieved using the GetWindowLong() call from the Windows API.\n\n
	<b>DWORD flags=0</b>\n\n
	<b>APPENDROLL_CLOSED</b>\n\n
	Starts the page in the rolled up state.\n\n
	<b>int category = ROLLUP_CAT_STANDARD</b>\n\n
	The category parameter provides flexibility with regard to where a
	particular rollup should be displayed in the UI. RollupPanels with lower
	category fields will be displayed before RollupPanels with higher category
	fields. For RollupPanels with equal category value the one that was added
	first will be displayed first. Allthough it is possible to pass any int
	value as category there exist currently 5 different category defines:
	<b>ROLLUP_CAT_SYSTEM</b>, <b>ROLLUP_CAT_STANDARD</b>, and
	<b>ROLLUP_CAT_CUSTATTRIB</b>.\n\n
	When using <b>ROLLUP_SAVECAT</b>, the rollup page will make the provided
	category sticky, meaning it will not read the category from the
	<b>RollupOrder.cfg</b> file, but rather save the category field that was
	passed as argument in the <b>CatRegistry</b> and in the
	<b>RollupOrder.cfg</b> file.\n\n
	The method will take the category of the replaced rollup in case the flags
	argument contains <b>ROLLUP_USEREPLACEDCAT</b>. This is mainly done, so
	that this system works with param maps as well.
	\return  The window handle of the rollup page. */
	virtual HWND AddRollupPage(
		HINSTANCE hInst, 
		DLGTEMPLATE *dlgTemplate, 
		DLGPROC dlgProc, 
		MCHAR *title, 
		LPARAM param=0, 
		DWORD flags=0, 
		int category = ROLLUP_CAT_STANDARD)=0;

	// Removes a rollup page and destroys it.
	/*! Removes a rollup page and destroys it.
	\par Parameters:
	<b>HWND hRollup</b>\n\n
	The handle of the rollup window. This is the handle returned from
	AddRollupPage(). */
	virtual void DeleteRollupPage(HWND hRollup)=0;

	// When the user mouses down in dead area, the plug-in should pass
	// mouse messages to this function which will pass them on to the rollup.
	/*! This allows hand cursor scrolling when the user clicks the
	mouse in an unused area of the dialog. When the user mouses down in dead
	area of the dialog, the plug-in should pass mouse messages to this function
	which will pass them on to the rollup.\n\n
	Note: In 3ds Max 2.0 and later only use of this method is no longer
	required -- the functionality happens automatically.
	\par Parameters:
	<b>HWND hDlg</b>\n\n
	The window handle of the dialog.\n\n
	<b>UINT message</b>\n\n
	The message sent to the dialog proc.\n\n
	<b>WPARAM wParam</b>\n\n
	Passed in to the dialog proc. Pass along to this method.\n\n
	<b>LPARAM lParam</b>\n\n
	Passed in to the dialog proc. Pass along to this method. */
	virtual void RollupMouseMessage(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)=0;

	// This will set the command mode to a standard pick mode.
	// The callback implements hit testing and a method that is
	// called when the user actually picks an item.
	/*! This will set the command mode to a standard pick mode. The
	callback implements hit testing and a method that is called when the user
	actually picks an item.
	\par Parameters:
	<b>RendPickProc *proc</b>\n\n
	The callback object. See Class RendPickProc. */
	virtual void SetPickMode(RendPickProc *proc)=0;
	
	// If a plug-in is finished editing its parameters it should not
	// leave the user in a pick mode. This will flush out any pick modes
	// in the command stack.
	/*! If a plug-in is finished editing its parameters it should not
	leave the user in a pick mode. This method will flush out any pick modes in
	the command stack. */
	virtual void EndPickMode()=0;
		
	// When a plugin has a Texmap, clicking on the button
	// associated with that map should cause this routine
	// to be called.
	/*! When a plugin has a Texmap, clicking on the button associated
	with that map should cause this routine to be called.
	\par Parameters:
	<b>MtlBase *mb</b>\n\n
	The <b>MtlBase</b> (<b>Texmap</b> or <b>Mtl</b>) to put to the materials
	editor. */
	virtual void PutMtlToMtlEditor(MtlBase *mb)=0;

	// This is for use only by the scanline renderer.
	/*! This method is used internally. */
	virtual float GetMaxPixelSize() = 0;

	/*! \remarks	This is a general purpose function that allows the API to be extended in the
	future. The 3ds Max development team can assign new <b>cmd</b> numbers and
	continue to add functionality to this class without having to 'break' the API.
	\par Parameters:
	<b>int cmd</b>\n\n
	The index of the command to execute.\n\n
	<b>ULONG_PTR arg1=0</b>\n\n
	Optional argument 1. See the documentation where the <b>cmd</b> option is
	discussed for more details on these parameters.\n\n
	<b>ULONG_PTR arg2=0</b>\n\n
	Optional argument 2.\n\n
	<b>ULONG_PTR arg3=0</b>\n\n
	Optional argument 3.
	\return  An integer return value. See the documentation where the <b>cmd</b>
	option is discussed for more details on the meaning of this value. */
	virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) { return 0; } 

	// JBW 12/1/98: get interface to rollup window interface
	virtual IRollupWindow* GetIRollup()=0;

	// Adds rollup pages to the render params dialog. Returns the window
	// handle of the dialog that makes up the page.
	virtual HWND AddTabRollupPage(const Class_ID& id, HINSTANCE hInst, MCHAR *dlgTemplate, 
		DLGPROC dlgProc, MCHAR *title, LPARAM param=0, DWORD flags=0, int category = ROLLUP_CAT_STANDARD)
	{
		return AddRollupPage(hInst, dlgTemplate, dlgProc, title, param, flags, category);
	}

	virtual HWND AddTabRollupPage(const Class_ID& id, HINSTANCE hInst, DLGTEMPLATE *dlgTemplate, 
		DLGPROC dlgProc, MCHAR *title, LPARAM param=0, DWORD flags=0, int category = ROLLUP_CAT_STANDARD)
	{
		return AddRollupPage(hInst, dlgTemplate, dlgProc, title, param, flags, category);
	}

	// Removes a rollup page and destroys it.
	virtual void DeleteTabRollupPage(const Class_ID& id, HWND hRollup)
	{
		DeleteRollupPage(hRollup);
	}

	// When the user mouses down in dead area, the plug-in should pass
	// mouse messages to this function which will pass them on to the rollup.
	virtual void RollupTabMouseMessage(const Class_ID& id, HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		RollupMouseMessage(hDlg, message, wParam, lParam);
	}

	// JBW 12/1/98: get interface to rollup window interface
	virtual IRollupWindow* GetTabIRollup(const Class_ID& id)
	{
		return GetIRollup();
	}

	// Return the tabbed dialog for these render params
	virtual ITabbedDialog* GetTabDialog()
	{
		return NULL;
	}

	// Return the page with the given class id.
	virtual ITabPage* GetTabPage(const Class_ID& id)
	{
		return NULL;
	}
};


/// \defgroup Render_Instance_Flags Render Instance Flags
//@{
#define INST_HIDE	  		(1<<0) //!< instance is hidden
#define INST_CLIP			(1<<1) //!< clip instance: ray tracers should skip it 
#define INST_BLUR			(1<<2) //!< secondary motion blur instance 
#define INST_RCV_SHADOWS	(1<<3) //!< instance receives shadows
#define INST_TM_NEGPARITY	(1<<4) //!< mesh is inside-out: need to reverse normals on-the-fly
#define INST_MTL_BYFACE     (1<<5) //!<instance's object supports mtl-by-face interface ( chkmtlapi.h)
//@}


/*! 
This class is available in release 2.0 and later only.\n\n
This class provides information about a single node being rendered. This
includes information such as the mesh of the object, its material, unique node
ID, object space bounding extents, number of lights affecting it, material
requirements, and normals and vertex coordinates in various spaces (object and
camera).
\par Data Members:
<b>ULONG flags;</b>\n\n
The flags that describe the properties of this instance. See \ref Render_Instance_Flags .\n\n
<b>Mtl *mtl;</b>\n\n
This is the material from the node.\n\n
<b>float wireSize;</b>\n\n
The wireframe size.\n\n
<b>Mesh *mesh;</b>\n\n
The mesh to be rendered. This is the result of
GeomObject::GetRenderMesh().\n\n
<b>float vis;</b>\n\n
Object visibility (between 0.0 and 1.0). This is the value the visibility track
evaluates to at a particular time.\n\n
<b>int nodeID;</b>\n\n
A unique ID associated with the node. It's unique within the scene during a
render.\n\n
<b>int objMotBlurFrame;</b>\n\n
This will be equal to <b>NO_MOTBLUR</b> for all non-blurred objects. For
blurred objects, it takes on the values (<b>0..nBlurFrames-1</b>) for the
successive blur-instances.\n\n
<b>int objBlurID;</b>\n\n
The purpose of this is to differentiate blur-instances generated from different
nodes. All the blur-instances for an object-motion-blurred object will have the
same <b>objBlurID</b>. This is as distinct from <b>nodeID</b>, which is
different for every instance. This makes it possible to easily avoid
intersecting a ray with all blur-instances for an object. If
RenderGlobalContext::IntersectWorld() is being used, then passing in the
<b>objBlurID</b> for the parameter <b>skipID</b> will have this effect.\n\n
The basic technique is this: When reflecting or refracting rays, and object
motion blur is enabled, choose sub-frame times randomly for the different rays
(effectively giving a coarse stochastic sampling of time).\n\n
<b>Matrix3 objToWorld;</b>\n\n
This matrix can be used to transform object coordinates to world
coordinates.\n\n
<b>Matrix3 objToCam;</b>\n\n
This matrix can be used to transform object coordinates to camera
coordinates.\n\n
<b>Matrix3 normalObjToCam;</b>\n\n
This matrix can be used for transforming surface normals from object space to
camera space.\n\n
<b>Matrix3 camToObj;</b>\n\n
This matrix can be used to transform camera coordinates to object
coordinates.\n\n
<b>Box3 obBox;</b>\n\n
The object space extents of the object being rendered.\n\n
<b>Point3 center;</b>\n\n
The object bounding sphere center (in camera coordinates)\n\n
<b>float radsq;</b>\n\n
The square of the bounding sphere's radius.  
\see  Class Mtl, Class LightDesc, Class INode,  Class Object,  Class Mesh, Class Matrix3,  Class Box3,  Class Point3, Class Interval.
*/
class RenderInstance: public MaxHeapOperators
{
public:
	ULONG flags;
	Mtl *mtl;       		// from inode, for convenience
	float wireSize;         // Mtl wireframe size
	Mesh *mesh;				// result of GetRenderMesh call
	float vis;				// Object visibility
	int nodeID;				// unique within scene during render- corresponds to ShadeContext::NodeID()
	int objMotBlurFrame;  	// Object motion blur sub frame (= NO_MOTBLUR for non-blurred objects)
	int objBlurID;		    // Blur instances for an object share a objBlurID value.
	Matrix3 objToWorld;		// transforms object coords to world coords
	Matrix3 objToCam;		// transforms object coords to cam coords
	Matrix3 normalObjToCam; // for transforming surface normals from obj to camera space
	Matrix3 camToObj;    	// transforms camera coords to object coords
	Box3 obBox;				// Object space extents
	Point3 center;			// Bounding sphere center (camera coords)
	float radsq;			// square of bounding sphere's radius

	/*! Destructor. */
	virtual ~RenderInstance() {}

	/*! Sets the specified flag(s) to the state passed.
	\par Parameters:
	<b>ULONG f</b>\n\n
	The flags to set. See \ref Render_Instance_Flags .\n\n
	<b>BOOL b</b>\n\n
	The state to set; TRUE for on; FALSE for off. */
	void SetFlag(ULONG f, BOOL b) { if (b) flags |= f; else flags &= ~f; }
	/*! Sets the specified flag(s) to on.
	\par Parameters:
	<b>ULONG f</b>\n\n
	The flags to set. See \ref Render_Instance_Flags . */
	void SetFlag(ULONG f) {  flags |= f; }
	/*! Clears the specified flag(s).
	\par Parameters:
	<b>ULONG f</b>\n\n
	The flags to set to zero. See \ref Render_Instance_Flags . */
	void ClearFlag(ULONG f) {  flags &= ~f; }
	/*! Returns TRUE if the specified flag(s) are set; otherwise
	FALSE.
	\par Parameters:
	<b>ULONG f</b>\n\n
	The flags to set to zero. See \ref Render_Instance_Flags */
	BOOL TestFlag(ULONG f) { return flags&f?1:0; }
	BOOL Hidden() { return TestFlag(INST_HIDE); }
	BOOL IsClip() { return TestFlag(INST_CLIP); }

	/*! Returns a pointer to the next in <b>RenderInstance</b> in the
	list. A pointer to the first element in the list may to retrieved from
	RenderGlobalContext::InstanceList(). */
	virtual RenderInstance *Next()=0;	// next in list

	/*! Returns the validity interval of the mesh of this render
	instance. */
	virtual Interval MeshValidity()=0;
	/*! Returns the number of lights affecting the node. */
	virtual int NumLights()=0;
	/*! Returns a pointer to the LightDesc for the 'i-th' light
	affecting the node.
	\par Parameters:
	<b>int n</b>\n\n
	Specifies which light. */
	virtual LightDesc *Light(int n)=0; 

	/*! \remarks	Returns TRUE if this particular instance will cast shadows from the
	particular light based on the light's Exclusion/Inclusion list; FALSE if it
	won't cast shadows.
	\par Parameters:
	<b>const ObjLightDesc\& lt</b>\n\n
	Describes the light. See Class ObjLightDesc. */
	virtual BOOL CastsShadowsFrom(const ObjLightDesc& lt)=0; // is lt shadowed by this instance?

	/*! Returns the INode pointer for the instance. */
	virtual INode *GetINode()=0;  						 // get INode for instance
	/*! Returns a pointer to the evaluated object for the instance.
	You can use this to get more information about the type of object being
	rendered. For instance you could look at the <b>Class_ID</b> and recognize
	it as a sphere, a box, a torus, etc. */
	virtual Object *GetEvalObject()=0; 					 // evaluated object for instance
	/*! Returns the material requirements of the material assigned to
	the node. See \ref materialRequirementsFlags.
	\par Parameters:
	<b>int mtlNum</b>\n\n
	Specifies the number of the sub-material whose requirements should be
	returned. A value of <b>-1</b> may be passed to return a value generated by
	looping over all the sub-materials and ORing together the requirements.\n\n
	<b>int faceNum</b>\n\n
	This parameter is available in release 4.0 and later only.\n\n
	This is the integer face number for objects which support material per face
	(if flag <b>INST_MTL_BYFACE</b> is set). See
	Class IChkMtlAPI. */
	virtual ULONG MtlRequirements(int mtlNum, int faceNum)=0;  	 // node's mtl requirements. DS 3/31/00: added faceNum to support mtl-per-face objects
	/*! Returns the geometric normal of the specified face in object
	space.
	\par Parameters:
	<b>int faceNum</b>\n\n
	Zero based index of the face whose normal is returned. */
	virtual Point3 GetFaceNormal(int faceNum)=0;         // geometric normal in camera coords
	/*! Returns the vertex normal of the specified face in camera
	coordinates.
	\par Parameters:
	<b>int faceNum</b>\n\n
	Zero based index of the face in the mesh.\n\n
	<b>int vertNum</b>\n\n
	Zero based index of the vertex in the face. */
	virtual Point3 GetFaceVertNormal(int faceNum, int vertNum)=0;  // camera coords
	/*! Returns the three vertex normals of the specified face in
	camera coordinates.
	\par Parameters:
	<b>int faceNum</b>\n\n
	Zero based index of the face in the mesh.\n\n
	<b>Point3 n[3]</b>\n\n
	The normals are returned here. */
	virtual void GetFaceVertNormals(int faceNum, Point3 n[3])=0;   // camera coords
	/*! Returns the coordinate for the specified vertex in camera
	coordinates
	\par Parameters:
	<b>int vertnum</b>\n\n
	The zero based index of the vertex in the mesh. */
	virtual Point3 GetCamVert(int vertnum)=0; 			 // coord for vertex in camera coords		
	/*! Returns the vertices of the specified face in object
	coordinates.
	\par Parameters:
	<b>int fnum</b>\n\n
	Zero based index of the face in the mesh.\n\n
	<b>Point3 obp[3]</b>\n\n
	The three vertices of the face in object coordinates. */
	virtual void GetObjVerts(int fnum, Point3 obp[3])=0; // vertices of face in object coords
	/*! Returns the vertices of the specified face in camera (view)
	coordinates.
	\par Parameters:
	<b>int fnum</b>\n\n
	Zero based index of the face in the mesh.\n\n
	<b>Point3 cp[3]</b>\n\n
	The three vertices of the face in camera coordinates. */
	virtual void GetCamVerts(int fnum, Point3 cp[3])=0; // vertices of face in camera(view) coords
	/*! \remarks	This is a general purpose function that allows the API to be extended in the
	future. The 3ds Max development team can assign new <b>cmd</b> numbers and
	continue to add functionality to this class without having to 'break' the API.
	\par Parameters:
	<b>int cmd</b>\n\n
	The index of the command to execute.\n\n
	<b>ULONG_PTR arg1=0</b>\n\n
	Optional argument 1. See the documentation where the <b>cmd</b> option is
	discussed for more details on these parameters.\n\n
	<b>ULONG_PTR arg2=0</b>\n\n
	Optional argument 2.\n\n
	<b>ULONG_PTR arg3=0</b>\n\n
	Optional argument 3.
	\return  An integer return value. See the documentation where the <b>cmd</b>
	option is discussed for more details on the meaning of this value.
	\par Default Implementation:
	<b>{ return 0; }</b> */
	virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) { return 0; } 

	// Material-by-face access
	// Objects can provide a material as a function of face number via the IChkMtlAPI interface (chkmtlapi.h).
	// This method will return RenderInstance::mtl if flag INST_MTL_BYFACE is not set. If INST_MTL_BYFACE is
	// set it will return the proper by-face mtl. // DS 4/3/00
	/*! \remarks	Objects can provide a material as a function of face number via the
	interface provided by Class IChkMtlAPI. This method will return <b>RenderInstance::mtl</b> if flag
	<b>INST_MTL_BYFACE</b> is not set. If <b>INST_MTL_BYFACE</b> is set it will
	return the proper by-face material. See \ref Render_Instance_Flags .
	\par Parameters:
	<b>int faceNum</b>\n\n
	The zero based index of the face in the mesh.\n\n
	Objects can provide a material as a function of face number via the
	IChkMtlAPI interface (chkmtlapi.h). */
	virtual Mtl *GetMtl(int faceNum)=0;  
};

//----------------------------------------------------------------

/// \defgroup Values_returned_from_Progress__ Values Returned from Progress()
//@{
#define RENDPROG_CONTINUE	1	//!< Continue to Process
#define RENDPROG_ABORT		0	//!< Stop Processing
//@}

/// \defgroup Values_passed_to_SetCurField__ Values Passed to SetCurField()
//@{
#define FIELD_FIRST		0
#define FIELD_SECOND	1
#define FIELD_NONE		-1
//@}


/*! This class is a callback passed in to the renderer. The system passes this
callback to the renderer, and the renderer will use these methods whenever it
is doing something that is going to take some time. For instance when
transforming objects it can update the progress bar. This is also passed in to
the shadow buffer code so the shadow buffer can show its progress. All methods
of this class are implemented by the system. They are called by a plug-in
renderer.  
\see  Class Renderer.*/
// A callback passed in to the renderer
class RendProgressCallback: public MaxHeapOperators
{
public:
	/*! Destructor. */
	virtual ~RendProgressCallback() { }
	/*! Allows the plug-in to set the string displayed in renderer
	dialog.
	\par Parameters:
	<b>const MCHAR *title</b>\n\n
	The string to display. */
	virtual void SetTitle(const MCHAR *title)=0;
	/*! Allows the plug-in to update the renderer progress display.
	\par Parameters:
	<b>int done</b>\n\n
	The number of items completed so far.\n\n
	<b>int total</b>\n\n
	The total number of items to process.
	\return  <b>RENDPROG_CONTINUE</b>\n\n
	Continue to process.\n\n
	<b>RENDPROG_ABORT</b>\n\n
	Stop processing. */
	virtual int Progress(int done, int total)=0;
	/*! Sets the field number display.
	\par Parameters:
	<b>int which</b>\n\n
	<b>FIELD_FIRST</b>\n\n
	<b>FIELD_SECOND</b>\n\n
	<b>FIELD_NONE</b> */
	virtual void SetCurField(int which) {}
	/*! The plug-in renderer should call this on every frame, passing
	in values for the various parameters. These are displayed in the rendering
	in progress dialog.
	\par Parameters:
	<b>int nlights</b>\n\n
	The total number of lights.\n\n
	<b>int nrayTraced</b>\n\n
	The number of lights using raytraced shadows.\n\n
	<b>int nshadowed</b>\n\n
	The number of lights using shadows.\n\n
	<b>int nobj</b>\n\n
	The total number of objects.\n\n
	<b>int nfaces</b>\n\n
	The total number of faces. */
	virtual void SetSceneStats(int nlights, int nrayTraced, int nshadowed, int nobj, int nfaces) {}
};


//==============================================================================
// class Renderer
//
//! \brief This is the base class for any renderer plugin.
/*! 
The main entry points for this class are methods Open(), Render(), and Close().
Any render operation works as follows:
1. Open() is called exactly once.
2. Render() is called zero or more times.
3. Close() is called exactly once.
*/
/*! \sa  Class ReferenceTarget, Class FrameRendParams, Class RendProgressCallback, Class IRendParams, Class INode, Class ViewParams, Class RendParamDlg, Class RendParams, Class DefaultLight.\n\n
\par Description:
This is the base class for the creation of plug-in renderers. There are five
methods that need to be implemented: Open(), Render(),
Close(), CreateParamDialog() and ResetParams().\n\n
In 3ds Max 2.0 and later developers must also implement
ReferenceTarget::Clone() to support the new Production/Draft renderer
capability.  */
class Renderer : public ReferenceTarget
{
public:

	//! \brief Returns the super class ID RENDERER_CLASS_ID.
	SClass_ID SuperClassID() {return RENDERER_CLASS_ID;}
	
	//! \brief Called once and only once per render operation; used to initialize the renderer
	//! before rendering a sequence of frames.
	/*! This gives a chance to the renderer to build any data structures which it will need
		in the rendering phase. If this call returns 0, then the caller is not required to call
		Close(), so Open() should do any necessary cleanup before returning 0.
		\param[in] scene - The root node of the scene to render. Note: If you are rendering 
		in the Materials Editor, you'll instead get a pointer to the INode that is in the 
		sample slot - not the root node of the scene.
		\param[in] vnode - The view node. This may be a camera, a light, or NULL.
		\param[in] viewPar - View parameters for rendering orthographic or user viewports. 
		This is used if vnode is NULL.
		\param[in] rp - This class contains a set of common renderer parameters.
		\param[in] hwnd - The owner window for messages.
		\param[in] defaultLights - An array of default lights if there are no user created lights in the scene.
		\param[in] numDefLights - Number of lights in defaultLights array.
		\param[in] prog - A callback used to allow the renderer to update the progress dialog.
		\return Nonzero for success, zero for failure.
	*/
	virtual int Open(
		INode *scene,
		INode *vnode,
		ViewParams *viewPar,
		RendParams &rp,
		HWND hwnd,
		DefaultLight* defaultLights=NULL,
		int numDefLights=0,
		RendProgressCallback* prog = NULL
	) = 0;
	
	//! \brief Called to render a single frame.
	/*! This may be called zero or more times, in between calls to Open() and Close(), to
		render a sequence of frames, at a series of time values which are not necessarily
		sequential.
		\param[in] t - The time at which this frame is to be rendered.
		\param[in] tobm - The bitmap to which the image is to be rendered.
		\param[in] frp - A set of frame dependent parameters.
		\param[in] hwnd - The owner window handle.
		\param[in] prog - A callback used to allow the renderer to update the progress dialog.
		\param[in] viewPar - This parameter allows one to specify a different view transformation 
		on each render call. For example, one may render a given scene at a given time from many 
		different viewpoints, without calling Render::Open() for each one.
		\return Nonzero for success, zero for failure.
	*/
	virtual int Render(
		TimeValue t,
   		Bitmap *tobm,
		FrameRendParams &frp,
		HWND hwnd,
		RendProgressCallback *prog=NULL,
		ViewParams *viewPar=NULL
	)=0;

	//! \brief Called once and only once per render operation; used to free any resources
	//! allocated by Open() or Render().
	/*! This method needs to be called whenever Open() was called and returned a non-zero value.
		The renderer should free any allocated resources, returning in a state identical to the one
		before Open() was called.
		\param[in] hwnd - The owner window handle.
		\param[in] prog - A callback used to allow the renderer to update the progress dialog.
		\return Nonzero for success, zero for failure.
	*/
	virtual void Close(	HWND hwnd, RendProgressCallback* prog = NULL )=0;	

	// apply render effects at the requested time value - should be called between Renderer::Open() and Renderer::Close()
	// this can be used during a multi-pass rendering, in order to apply the render effects to the final, blended bitmap.
	// 'updateDisplay' indicates that Bitmap's display should be refreshed by the renderer
	// return value indicats if effects were successfully applied.
	//! \brief This method is called to apply the render effects at the specified time value. 
	/*! It should be called between the Open() and Close()
	methods.\n\n
	This can be used during a multi-pass rendering, in order to apply the
	render effects to the final, blended bitmap.
	\par Parameters:
	\param t - The time to apply the render effects.
	\param pBitmap - Points to the bitmap.
	\param updateDisplay - Passing true indicates that Bitmap's display should be refreshed by the
		renderer; false indicates it should not be.
	\return  Returns true if the effects were successfully applied; otherwise false. */
	virtual bool ApplyRenderEffects(TimeValue t, Bitmap *pBitmap, bool updateDisplay=true) { return false; }

	// Adds rollup page(s) to renderer configure dialog
	// If prog==TRUE then the rollup page should just display the parameters
	// so the user has them for reference while rendering, they should not be editable.
	//! \brief This method is called to create and return a pointer to an instance of the
	//! RendParamDlg class.
	/*! The renderer can add rollup page(s) to the renderer configuration dialog 
	using the IRendParams interface passed into this method.
	\param ir - An interface that provides methods for use in displaying parameters, for
	example this class has methods for adding rollup pages.
	\param prog - If TRUE then the rollup page should just display the parameters so the user
	has them for reference while rendering, they should not be editable.
	\return  A pointer to an instance of the <b>RendParamDlg</b> class. This
	class will be deleted using RendParamDlg::DeleteThis(). */
	virtual RendParamDlg *CreateParamDialog(IRendParams *ir,BOOL prog=FALSE)=0;

	//! This method simply sets all the parameters to their default values.
	virtual void ResetParams()=0;
	virtual int	GetAAFilterSupport(){ return 0; } // point sample, no reconstruction filter

	//! \brief Renderers which support texture baking should override this method to return true. 
	/*! It is checked when the Render to Texture dialog is opened;
		if the current renderer does not support texture baking, then a warning message 
		is displayed and the user will not be able to press the Render button in the dialog.
		\return By default this will return false */
	virtual bool SupportsTexureBaking() { return false; }

	//! \brief A renderer should override this method to return true if it supports any custom preset categories beyond the standard categories.
	/*! \return By default this will returns false */
	virtual bool SupportsCustomRenderPresets() { return false; }

	//! \brief Return a number indicating the current version of the renderer's custom preset.
	/*! The number can be arbitrary 
		\return By default this will return -1 */
	virtual int  RenderPresetsFileVersion() { return -1; }

	//! \brief Return true if the renderer can load presets of the indicated version.
	/*! \param version The version to test compatibility against
		\return By default this will return false. */
	virtual BOOL RenderPresetsIsCompatible( int version ) { return false; }

	//! \brief Returns the UI name of a supported custom category
	/*! \param catIndex - The custom preset index to return the name of.  catIndex will be a number between 
		RENDER_PRESETS_CUSTOM_CATEGORY_INDEX_BEGIN and RENDER_PRESETS_CATEGORY_COUNT.
		\return If the ID is a supported custom category, return the name of the category to be displayed in the UI. Otherwise return NULL */
	virtual MCHAR * RenderPresetsMapIndexToCategory( int catIndex ) { return NULL; }

	//! \brief Returns the index of a supported custom category
	/*! \param category - The UI name of a custom category
		\return If the input is the name of a custom category supported by the renderer, return the ID number of the category. Otherwise returns 0 */
	virtual int RenderPresetsMapCategoryToIndex( const MCHAR* category )  { return 0; }

	// \deprecated Implement RenderPresetsMapCategoryToIndex( const MCHAR* category ) 
	virtual int RenderPresetsMapCategoryToIndex( MCHAR* category ) { return RenderPresetsMapCategoryToIndex( const_cast<const MCHAR*>(category));}

	//! \brief called before a render preset is saved. 
	/*! For each custom category supported use root->AddSaveTarget() passing the object with the 
		parameters for that category if the corresponding bit is set in the saveCategories, 
		The object will be saved along with the preset. 
		\par Example Implementation:
		\code
		{
			// Iterate through all our possible custom preset indices
			for (int i = RENDER_PRESETS_CUSTOM_CATEGORY_INDEX_BEGIN; i < RENDER_PRESETS_CUSTOM_CATEGORY_INDEX_BEGIN + MY_CUSTOM_PRESET_COUNT; i++)
			{
				// Should the preset be saved?
				if (saveCategories[i])
				{
					// p_myOptions is a class derived from ReferenceTarget that contains our options.
					root->AddSaveTarget(i, p_myOptions);
				}
			}
		}
		\endcode
		\param root An instance of an ITargetedIO class to be used to save any custom presets specified
		\param saveCategories Species the custom preset categories to be saved	*/
	virtual int RenderPresetsPreSave( ITargetedIO * root, BitArray saveCategories ) { return -1; }

	//! \brief called after a preset is saved. 
	/*! No specific action is required from the renderer at this time.
		\sa RenderPresetsPreSave */
	virtual int RenderPresetsPostSave( ITargetedIO * root, BitArray loadCategories ) { return -1; }

	//! \brief called before a preset is loaded. 
	/*! For each custom category supported, if the corresponding bit is not set
		in the loadCategories, use root->Store() passing the object with the parameters for that category. The object 
		will be preserved, so the renderer can refer to it after the preset is loaded.
		\sa RenderPresetsPreSave, RenderPresetsPostLoad
		\param root An instance of an ITargetedIO class to be used to store any custom presets specified
		\param saveCategories Lists the custom preset categories to be loaded.  Any categories not being loaded should
			be stored on root. */
	virtual int RenderPresetsPreLoad( ITargetedIO * root, BitArray saveCategories ) { return -1; }

	//! \brief called after a preset is loaded. 
	/*! For each custom category supported...\n\n
		If the bit is set in the loadCategories: use root->GetSaveTarget() to retrieve the loaded object, 
		and update the renderer's active parameters to match this object\n\n
		If the bit is not set in the loadCategories: use root->Retrieve() to retrieve the object that was
		stored during pre-load. Update the renderer's active parameters to match this object. 
		This is important in case a certain category was held in the file and loaded, but the user did 
		not choose to load that category. In this case the renderer must restore its parameters to their former value. 
		\sa RenderPresetsPreLoad
		\param root An instance of an ITargetedIO class to be used to retrieve any custom presets stored pre-load.
		\param loadCategories Lists the custom preset categories that have been loaded. */
	virtual int RenderPresetsPostLoad( ITargetedIO * root, BitArray loadCategories ) { return -1; }
};


class ShadowBuffer;
class ShadowQuadTree;

/*! This class is passed into the method ObjLightDesc::Update(). The methods
of this class are implemented by the 3ds Max scanline renderer. Developer
creating other renderer plug-ins may choose to implement the methods of this
class if they wish to use the same architecture. Developers who wish to take
advantage of the 3ds Max volumetric light effects should implement the methods
of this class. The volumetric lights are set up to work with this mechanism.
\see  Class ObjLightDesc.
 */
class RendContext: public MaxHeapOperators
{
public:
	/*! Destructor. */
	virtual ~RendContext() {}
	/*! This method is used to update the progress bar and check the keyboard and
	mouse for user cancellation. A plug-in renderer should override this
	method by calling the RendProgressCallback::Progress() method on the
	RendProgressCallback passed in to the Renderer::Render()
	method.
	\par Parameters:
	\param done	This is the number completed so far.
	\param total This is the total number of things to complete.
	\return  Nonzero to continue; zero if the user has canceled. */
	virtual int Progress(int done, int total) const { return 1; }
	//! \brief	This is a multiplier that scales the brightness of all scene lights.
	/*! it doesn't affect the ambient light level. It is included in
	RendContext so the lights can use it to multiply times the light's color. */
	virtual Color GlobalLightLevel() const = 0;
};

/*! This structure contains information on rendering for Mirror and 
Automatic Cubic materials. This is used by the methods of the RenderMapsContext class.
\see Class RenderMapsContext.
*/
struct SubRendParams : public BaseInterfaceServer
{
	/*! The rendering type being done.*/
	RendType rendType;
	/*! TRUE if field rendering is being used; otherwise FALSE. */
	BOOL fieldRender;
	/*! This is used when field rendering. TRUE if doing even numbered scanlines; FALSE for odd numbered. */
	BOOL evenLines; // when field rendering
	/*! This is used as part of implementing the Mirror material. It should be FALSE in all other cases. */
	BOOL doingMirror;
	BOOL doEnvMap;  // do environment maps?
	/*! The dimensions in pixels of Bitmap tobm. */
	int devWidth, devHeight;
	/*! The aspect ratio of Bitmap tobm. */
	float devAspect;
	/*! The location on the screen of the upper left corner of the output bitmap. */
	int xorg, yorg;
	/*! The area of the screen being rendered. */
	int xmin,xmax,ymin,ymax;

	// parameters for render blowup.
	/*! This parameter is available in release 4.0 and later only. \n
	The 2D point at the center of the render blowup region. */
	Point2 blowupCenter;
	/*! This parameter is available in release 4.0 and later only. \n
	The X and Y scale factors for render blowup. */
	Point2 blowupFactor;

	/*! \remarks	This is a general purpose function that allows the API to be extended in the
	future. The 3ds max development team can assign new cmd numbers and continue
	to add functionality to this class without having to 'break' the API.
	\par Parameters:
	<b>int cmd</b>\n
	The index of the command to execute.\n\n
	<b>ULONG arg1=0</b>\n
	Optional argument 1. See the documentation where the cmd option is discussed for more details on these parameters.\n\n
	<b>ULONG arg2=0</b>\n
	Optional argument 2.\n\n
	<b>ULONG arg3=0</b>\n
	Optional argument 3.
	\par Return Value:
	An integer return value. See the documentation where the cmd option is
	discussed for more details on the meaning of this value.
	*/
	virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) { return 0; }
};

#define RENDMAP_SHOW_NODE  1  // DO NOT exclude this node from the render. An argument for RenderMapsContext::Render()


// A pointer to this data structure is passed to MtlBase::BuildMaps();
// when rendering reflection and refraction maps.
/*! 
An instance of this class is passed into the MtlBase::BuildMaps()
method. This is used for the Mirror and Automatic Cubic maps. These maps
callback to methods of this class to perform a rendering from a particular
view. Sample code using these methods is available in
<b>/MAXSDK/SAMPLES/MATERIALS/MIRROR.CPP</b> and <b>ACUBIC.CPP</b>. All methods
of this class are implemented by the system.  
\see  Class INode, Class ViewParams, Structure SubRendParams, Class Matrix3,  Class Box3,  Class Point4, Class Bitmap, Class RenderGlobalContext.*/
class RenderMapsContext: public MaxHeapOperators
{ 
public:
	/*! Destructor. */
	virtual ~RenderMapsContext() { }
	/*! Returns the INode pointer of the node being rendered. This
	pointer allows a developer to access the properties of the node. See
	Class INode. */
	virtual INode *GetNode()=0;
	/*! Returns the node ID for the item being rendered or -1 if not
	set. This ID is assigned when the scene is being rendered - each node is
	simply given an ID: 0, 1, 2, 3, etc. The NodeRenderID() is simply a
	number automatically assigned to every node being rendered so that they can
	be differentiated in texture maps such as the Auto-cubic, which needs to
	store a cubic map for each node it is applied to. */
	virtual int NodeRenderID()=0;
	/*! Retrieves the current view dependent parameters.
	\par Parameters:
	<b>ViewParams \&vp</b>\n\n
	The <b>ViewParams</b> instance to update. */
	virtual void GetCurrentViewParams(ViewParams &vp)=0;
	/*! Retrieves the sub-render parameters.
	\par Parameters:
	<b>SubRendParams \&srp</b>\n\n
	The <b>SubRendParams</b> instance to update. */
	virtual void GetSubRendParams(SubRendParams &srp)=0;
	/*! Returns the current sub material index or -1 if at node
	material level. */
	virtual int SubMtlIndex()=0;
	virtual void SetSubMtlIndex(int mindex)=0;
	/*! Computes the plane containing the current material or sub
	material. This is used by the Flat Mirror material.
	\par Parameters:
	<b>float pl[4]</b>\n\n
	The plane containing the current material or sub material. The four float
	values in <b>pl[4]</b> represent the plane equation. If you call the four
	values A,B,C and D, then the plane equation of the plane is Ax + By + cZ +
	D = 0. */
	virtual void FindMtlPlane(float pl[4])=0;
	/*! This method computes the rectangle in screen space of the
	specified material. This uses the viewTM, but assumes the remaining view
	params (devWidth, devHeight, devAspect, fov) are the same as the main
	render. This method is specific to the Flat Mirror material.
	\par Parameters:
	<b>Rect \&sbox</b>\n\n
	The resulting 2D box.\n\n
	<b>Matrix3* viewTM=NULL</b>\n\n
	The view matrix.\n\n
	<b>int mtlIndex=-1</b>\n\n
	The material index, or -1 if a node level material. */
	virtual void FindMtlScreenBox(Rect &sbox, Matrix3* viewTM=NULL, int mtlIndex=-1)=0;
	/*! \remarks	This method computes the bounding box in camera space of the object
	associated with the reflection or refraction map being built in a call to a
	map's BuildMaps() method. */
	virtual Box3 CameraSpaceBoundingBox()=0;
	/*! \remarks	This method computes the bounding box in object space of the object
	associated with the reflection or refraction map being built in a call to a
	map's BuildMaps() method. */
	virtual Box3 ObjectSpaceBoundingBox()=0;
	/*! \remarks	This method returns the object to world transformation. */
	virtual Matrix3 ObjectToWorldTM()=0;
	/*! \remarks	Returns a pointer to a class that describes properties of the rendering
	environment. */
	virtual RenderGlobalContext *GetGlobalContext() { return NULL; }
	// ClipPlanes is a pointer to an array of Point4's,  each of which
	// represents a clip plane.  nClip Planes is the number of planes (up to 6);
	// The planes are in View space.
	/*! Renders the scene and stores in the result into <b>bm</b>.
	\par Parameters:
	<b>Bitmap *bm</b>\n\n
	The Bitmap to render the result to. The properties of this bitmap define
	the properties of the render (such as the width and height).\n\n
	<b>ViewParams \&vp</b>\n\n
	The ViewParams.\n\n
	<b>SubRendParams \&srp</b>\n\n
	The SubRendParams.\n\n
	<b>Point4 *clipPlanes=NULL</b>\n\n
	This revised parameter is available in release 2.0 and later only.\n\n
	This is a pointer to an array of Point4s, each of which represents a clip
	plane. If it is non-null, the renderer will clip all objects against these
	planes in addition to the normal left/right/top/bottom clipping. This is
	used by the Mirror material to clip away stuff that is behind the mirror.
	If not needed this may default to NULL.\n\n
	<b>int nClipPlanes=0</b>\n\n
	This parameter is available in release 2.0 and later only.\n\n
	The number of clipping planes above. A maximum of 6 is possible.
	\return  Nonzero on success; otherwise zero. */
	virtual	int Render(Bitmap *bm, ViewParams &vp, SubRendParams &srp, Point4 *clipPlanes=NULL, int nClipPlanes=0)=0; 
	/*! \remarks	This is a general purpose function that allows the API to be extended in the
	future. The 3ds Max development team can assign new <b>cmd</b> numbers and
	continue to add functionality to this class without having to 'break' the API.
	\par Parameters:
	<b>int cmd</b>\n\n
	The index of the command to execute.\n\n
	<b>ULONG_PTR arg1=0</b>\n\n
	Optional argument 1. See the documentation where the <b>cmd</b> option is
	discussed for more details on these parameters.\n\n
	<b>ULONG_PTR arg2=0</b>\n\n
	Optional argument 2.\n\n
	<b>ULONG_PTR arg3=0</b>\n\n
	Optional argument 3.
	\return  An integer return value. See the documentation where the <b>cmd</b>
	option is discussed for more details on the meaning of this value. */
	virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) { return 0; } 
};	


#define DONT_CLIP 1.0E38f


/// \defgroup Interface_ID Interface ID
//@{
#define SCANLINE_RENDERER_INTERFACE  Interface_ID(0x342323, 0x551665)
#define SCANLINE_RENDERER3_INTERFACE Interface_ID(0x44e40bbc, 0x52bc7cd1)
//@}

#define GetScanRendererInterface(obj) ((IScanRenderer2*)obj->GetInterface(SCANLINE_RENDERER_INTERFACE)) 
#define GetScanRendererInterface3(obj) ((IScanRenderer3*)obj->GetInterface(SCANLINE_RENDERER3_INTERFACE)) 

//--------------------------------------------------------------------------
// Interface into the default scanline renderer, Class_ID(SREND_CLASS_ID,0)
//---------------------------------------------------------------------------
class FilterKernel;
/*! 
This class provides an interface into the standard 3ds Max scanline renderer.
This renderer has a Class_ID of <b>Class_ID(SREND_CLASS_ID,0)</b>. This class
is available in release 2.0 and later only. All methods of this class are
implemented by the system.  
\see  Class Renderer.*/
class IScanRenderer: public Renderer {
	public:
	/*! Sets the 'Anti-Aliasing' setting of the renderer to the value
	passed.
	\par Parameters:
	<b>BOOL b</b>\n\n
	TRUE for on; FALSE for off. */
	virtual void SetAntialias(BOOL b) = 0;
	/*! Returns TRUE if anti-aliasing is on; FALSE if off. */
	virtual BOOL GetAntialias() = 0;

	/*! Sets the 'Filter Maps' setting to the value passed.
	\par Parameters:
	<b>BOOL b</b>\n\n
	TRUE for on; FALSE for off. */
	virtual void SetFilter(BOOL b) = 0;

	/*! Returns TRUE if Filter Maps is on; FALSE if off. */
	virtual BOOL GetFilter() = 0;

	/*! Sets the 'Shadows' setting to the value passed.
	\par Parameters:
	<b>BOOL b</b>\n\n
	TRUE for on; FALSE for off. */
	virtual void SetShadows(BOOL b) = 0;
	/*! Returns TRUE if the rendering of shadow is on; FALSE if off.
	*/
	virtual BOOL GetShadows() = 0;
	/*! Sets the 'Mapping' setting to the value passed.
	\par Parameters:
	<b>BOOL b</b>\n\n
	TRUE for on; FALSE for off. */
	virtual void SetMapping(BOOL b) = 0;
	/*! Returns TRUE if the Mapping setting is on; FALSE if off. */
	virtual BOOL GetMapping() = 0;
	/*! Sets the 'Force Wireframe' setting to the value passed.
	\par Parameters:
	<b>BOOL b</b>\n\n
	TRUE for on; FALSE for off. */
	virtual void SetForceWire(BOOL b) = 0;
	/*! Returns TRUE if Force Wire is on; FALSE if off. */
	virtual BOOL GetForceWire() = 0;
	/*! Sets the 'Auto-Reflect/Refract and Mirrors' setting to the
	value passed.
	\par Parameters:
	<b>BOOL b</b>\n\n
	TRUE for on; FALSE for off. */
	virtual	void SetAutoReflect(BOOL b)=0;
	/*! Returns TRUE if auto reflect is on; FALSE if off. */
	virtual	BOOL GetAutoReflect()=0;


	/*! Sets Object Motion Blur to on or off.
	\par Parameters:
	<b>BOOL b</b>\n\n
	TRUE for on; FALSE for off. */
	virtual void SetObjMotBlur(BOOL b) = 0;

	/*! Returns TRUE if object motion blur is on; FALSE if off. */
	virtual BOOL GetObjMotBlur() = 0;


	/*! Sets Velocity Motion Blur to on or off.
	\par Parameters:
	<b>BOOL b</b>\n\n
	TRUE for on; FALSE for off. */
	virtual void SetVelMotBlur(BOOL b) = 0;

	/*! Returns TRUE if velocity motion blur is on; FALSE if off. */
	virtual BOOL GetVelMotBlur() = 0;

	// Obsolete, use setfiltersz. pixel sz = 1.0 for all filtering
	/*! This method is obsolete. */
	virtual void SetPixelSize(float size) = 0;
	
	/*! Sets the Auto Reflect/Refract Maps Rendering Iterations
	setting to the value passed.
	\par Parameters:
	<b>int n</b>\n\n
	The value to set in the range of 1 to 10. */
	virtual void SetAutoReflLevels(int n) = 0;
	/*! Sets the ''Wire Thickness' setting to the value passed.
	\par Parameters:
	<b>float t</b>\n\n
	The value to set in the range of 0.0 to 10000.0. */
	virtual void SetWireThickness(float t) = 0;

	virtual void SetObjBlurDuration(float dur) = 0;
	virtual void SetVelBlurDuration(float dur) = 0;
	/*! Sets the Motion Blur Duration setting to the value passed.
	\par Parameters:
	<b>int n</b>\n\n
	The value to set in the range of 1 to 16. */
	virtual void SetNBlurFrames(int n) = 0;
	/*! Sets the Object Motion Blur Samples setting to the value
	passed.
	\par Parameters:
	<b>int n</b>\n\n
	The value to set in the range of 1 to 16. */
	virtual void SetNBlurSamples(int n) = 0;


	/*! Sets the '3ds Max Quadtree Depth' setting for Ray-Traced
	shadows to the value passed.
	\par Parameters:
	<b>int n</b>\n\n
	The value to set in the range of 0-10. */
	virtual void SetMaxRayDepth(int n) = 0;
	/*! \remarks	Returns the maximum ray depth. */
	virtual int GetMaxRayDepth() { return 7; }


	/*! \remarks	Sets the anti-aliasing filter to the one passed.
	\par Parameters:
	<b>FilterKernel *pKernel</b>\n\n
	Points to the filter to use. See Class FilterKernel. */
	virtual void SetAntiAliasFilter( FilterKernel * pKernel ) = 0;

	/*! \remarks	Returns a pointer to the anti-aliasing filter in use. See
	Class FilterKernel. */
	virtual FilterKernel * GetAntiAliasFilter() = 0;

	/*! \remarks	Sets the anti-aliasing filter size to the given value.
	\par Parameters:
	<b>float size</b>\n\n
	The size to set. */
	virtual void SetAntiAliasFilterSz(float size) = 0;

	/*! \remarks	Returns the anti-aliasing filter size. */
	virtual float GetAntiAliasFilterSz() = 0;


	/*! \remarks	Sets if sampling is enabled or disabled.
	\par Parameters:
	<b>BOOL on</b>\n\n
	TRUE for enabled (on); FALSE for disabled (off). */
	virtual void SetPixelSamplerEnable( BOOL on ) = 0;

	/*! \remarks	Returns TRUE if anti-aliasing is on; FALSE if off. */
	virtual BOOL GetPixelSamplerEnable() = 0;
};

//--------------------------------------------------------------------------
// Extended Interface into the default scanline renderer, Class_ID(SREND_CLASS_ID,0)
//---------------------------------------------------------------------------
class IScanRenderer2: public IScanRenderer, public FPMixinInterface {
	public:

	enum {  get_mapping, set_mapping,  
			get_shadows, set_shadows,  
			get_autoReflect, set_autoReflect,  
			get_forceWire, set_forceWire,  
			get_antialias, set_antialias,  
			get_filter, set_filter,  
			get_objMotBlur, set_objMotBlur,  
			get_velMotBlur, set_velMotBlur,  
			get_applyVelBlurEnv, set_applyVelBlurEnv,  
			get_velBlurTrans, set_velBlurTrans,  
			get_memFrugal, set_memFrugal,  
			get_pixelSamplerEnable, set_pixelSamplerEnable,  
			get_wireThickness, set_wireThickness,  
			get_objBlurDuration, set_objBlurDuration,  
			get_velBlurDuration, set_velBlurDuration,  
			get_antiAliasFilterSz, set_antiAliasFilterSz,  
			get_NBlurSamples, set_NBlurSamples,  
			get_NBlurFrames, set_NBlurFrames,  
			get_autoReflLevels, set_autoReflLevels,  
			get_colorClampType, set_colorClampType,  
			get_antiAliasFilter, set_antiAliasFilter,  
			get_enableSSE, set_enableSSE,  
			//new in R6
			get_globalSamplerEnabled, set_globalSamplerEnabled, 
			get_globalSamplerClassByName, set_globalSamplerClassByName,
			get_globalSamplerSampleMaps, set_globalSamplerSampleMaps, 
			get_globalSamplerQuality, set_globalSamplerQuality, 
			get_globalSamplerAdaptive, set_globalSamplerAdaptive, 
			get_globalSamplerAdaptiveThresh, set_globalSamplerAdaptiveThresh,
			get_globalSamplerParam1, set_globalSamplerParam1,
			get_globalSamplerParam2, set_globalSamplerParam2,
		};

	//Function Map For Mixin Interface
	//*************************************************
	#pragma warning(push)
	#pragma warning(disable:4238)
	BEGIN_FUNCTION_MAP
		PROP_FNS(get_mapping, GetMapping,						set_mapping, SetMapping, TYPE_BOOL);
		PROP_FNS(get_shadows, GetShadows,						set_shadows, SetShadows, TYPE_BOOL);
		PROP_FNS(get_autoReflect, GetAutoReflect,				set_autoReflect, SetAutoReflect, TYPE_BOOL);
		PROP_FNS(get_forceWire, GetForceWire,					set_forceWire, SetForceWire, TYPE_BOOL);
		PROP_FNS(get_antialias, GetAntialias,					set_antialias, SetAntialias, TYPE_BOOL);
		PROP_FNS(get_filter, GetFilter,							set_filter, SetFilter, TYPE_BOOL);
		PROP_FNS(get_objMotBlur, GetObjMotBlur,					set_objMotBlur, SetObjMotBlur, TYPE_BOOL);
		PROP_FNS(get_velMotBlur, GetVelMotBlur,					set_velMotBlur, SetVelMotBlur, TYPE_BOOL);
		PROP_FNS(get_applyVelBlurEnv, GetApplyVelBlurEnv,		set_applyVelBlurEnv, SetApplyVelBlurEnv, TYPE_BOOL);
		PROP_FNS(get_velBlurTrans, GetVelBlurTrans,				set_velBlurTrans, SetVelBlurTrans, TYPE_BOOL);
		PROP_FNS(get_memFrugal, GetMemFrugal,					set_memFrugal, SetMemFrugal, TYPE_BOOL);
		PROP_FNS(get_pixelSamplerEnable, GetPixelSamplerEnable,	set_pixelSamplerEnable, SetPixelSamplerEnable, TYPE_BOOL);

		PROP_FNS(get_wireThickness, GetWireThickness,			set_wireThickness, SetWireThickness, TYPE_FLOAT);
		PROP_FNS(get_objBlurDuration, GetObjBlurDuration,		set_objBlurDuration, SetObjBlurDuration, TYPE_FLOAT);
		PROP_FNS(get_velBlurDuration, GetVelBlurDuration,		set_velBlurDuration, SetVelBlurDuration, TYPE_FLOAT);
		PROP_FNS(get_antiAliasFilterSz, GetAntiAliasFilterSz,	set_antiAliasFilterSz, SetAntiAliasFilterSz, TYPE_FLOAT);

		PROP_FNS(get_NBlurSamples, GetNBlurSamples,				set_NBlurSamples, SetNBlurSamples, TYPE_INT);
		PROP_FNS(get_NBlurFrames, GetNBlurFrames,				set_NBlurFrames, SetNBlurFrames, TYPE_INT);
		PROP_FNS(get_autoReflLevels, GetAutoReflLevels,			set_autoReflLevels, SetAutoReflLevels, TYPE_INT);
		PROP_FNS(get_colorClampType, GetColorClampType,			set_colorClampType, SetColorClampType, TYPE_INT);

		PROP_FNS(get_antiAliasFilter, GetAntiAliasFilter,		set_antiAliasFilter, SetAntiAliasFilterRT, TYPE_REFTARG);

		PROP_FNS(get_enableSSE, IsSSEEnabled,					set_enableSSE, SetEnableSSE, TYPE_BOOL);

		PROP_FNS(get_globalSamplerEnabled, GetGlobalSamplerEnabled,					set_globalSamplerEnabled, SetGlobalSamplerEnabled, TYPE_BOOL); 
		PROP_FNS(get_globalSamplerClassByName, GetGlobalSamplerClassByName,			set_globalSamplerClassByName, SetGlobalSamplerClassByName, TYPE_TSTR_BV);
		PROP_FNS(get_globalSamplerSampleMaps, GetGlobalSamplerSampleMaps,			set_globalSamplerSampleMaps, SetGlobalSamplerSampleMaps, TYPE_BOOL);
		PROP_FNS(get_globalSamplerQuality, GetGlobalSamplerQuality ,				set_globalSamplerQuality, SetGlobalSamplerQuality, TYPE_FLOAT);
		PROP_FNS(get_globalSamplerAdaptive, GetGlobalSamplerAdaptive,				set_globalSamplerAdaptive, SetGlobalSamplerAdaptive, TYPE_BOOL);
		PROP_FNS(get_globalSamplerAdaptiveThresh, GetGlobalSamplerAdaptiveThresh,	set_globalSamplerAdaptiveThresh, SetGlobalSamplerAdaptiveThresh, TYPE_FLOAT);
		PROP_FNS(get_globalSamplerParam1, GetGlobalSamplerParam1,					set_globalSamplerParam1, SetGlobalSamplerParam1, TYPE_FLOAT);
		PROP_FNS(get_globalSamplerParam2, GetGlobalSamplerParam2,					set_globalSamplerParam2, SetGlobalSamplerParam2, TYPE_FLOAT);

	END_FUNCTION_MAP
	#pragma warning(pop)

	FPInterfaceDesc* GetDesc();    // <-- must implement 
		//**************************************************

	void SetAntiAliasFilterRT(ReferenceTarget* op) {
		if (op && op->SuperClassID( ) == FILTER_KERNEL_CLASS_ID)
			SetAntiAliasFilter(static_cast< FilterKernel* >(op));
	}

	virtual float GetWireThickness() = 0;
	virtual void SetColorClampType (int i) = 0;
	virtual int GetColorClampType () = 0;
	virtual float GetObjBlurDuration() = 0;
	virtual int GetNBlurSamples() = 0;
	virtual int GetNBlurFrames() = 0;
	virtual float GetVelBlurDuration() = 0;
	virtual void SetApplyVelBlurEnv(BOOL b) = 0;
	virtual BOOL GetApplyVelBlurEnv() = 0;
	virtual void SetVelBlurTrans(BOOL b) = 0;
	virtual BOOL GetVelBlurTrans() = 0;
	virtual int GetAutoReflLevels() = 0;
	virtual void SetMemFrugal(BOOL b) = 0;
	virtual BOOL GetMemFrugal() = 0;
	virtual void SetEnableSSE(BOOL b) = 0;
	virtual BOOL IsSSEEnabled() = 0;

	//new for R6
	virtual BOOL GetGlobalSamplerEnabled() = 0;
	virtual void SetGlobalSamplerEnabled(BOOL enable) = 0;
	virtual MSTR GetGlobalSamplerClassByName() = 0;
	virtual void SetGlobalSamplerClassByName(const MSTR) = 0;
	virtual BOOL GetGlobalSamplerSampleMaps() = 0;
	virtual void SetGlobalSamplerSampleMaps(BOOL enable) = 0;
	virtual float GetGlobalSamplerQuality() = 0;
	virtual void SetGlobalSamplerQuality(float f) = 0;
	virtual BOOL GetGlobalSamplerAdaptive() = 0;
	virtual void SetGlobalSamplerAdaptive(BOOL enable) = 0;
	virtual float GetGlobalSamplerAdaptiveThresh() = 0;
	virtual void SetGlobalSamplerAdaptiveThresh(float f) = 0;
	virtual float GetGlobalSamplerParam1() = 0;
	virtual void SetGlobalSamplerParam1(float f) = 0;
	virtual float GetGlobalSamplerParam2() = 0;
	virtual void SetGlobalSamplerParam2(float f) = 0;
};

#if defined(SINGLE_SUPERSAMPLE_IN_RENDER)
//--------------------------------------------------------------------------
// Extended Interface into the default scanline renderer, Class_ID(SREND_CLASS_ID,0)
//---------------------------------------------------------------------------
class IScanRenderer3: public IScanRenderer2 {
	public:

	virtual float GetSamplerQuality() = 0;
	virtual void SetSamplerQuality(float) = 0;

	virtual BOOL GetRenderWatermark() = 0;
	virtual void SetRenderWatermark(BOOL on) = 0;
	virtual Bitmap* GetWatermarkBitmap() = 0;
	virtual void SetWatermarkBitmap(Bitmap* bm) = 0;
	virtual int GetWatermarkTop() = 0;
	virtual void SetWatermarkTop(int top) = 0;
	virtual int GetWatermarkLeft() = 0;
	virtual void SetWatermarkLeft(int left) = 0;
	virtual float GetWatermarkBlend() = 0;
	virtual void SetWatermarkBlend(float blend) = 0;
	virtual BOOL GetWatermarkUI() = 0;
	virtual void SetWatermarkUI(BOOL on) = 0;
};
#endif	// defined(SINGLE_SUPERSAMPLE_IN_RENDER)

//==============================================================================
// class IRendererRequirements
//
// This interface is used to query special requirement flags from renderers.
//==============================================================================
#define IRENDERERREQUIREMENTS_INTERFACE_ID Interface_ID(0x27c85c29, 0xfab6ee0) //!< Interface ID for the class IRendererRequirements
class IRendererRequirements : public BaseInterface {
public:

	enum Requirement {
		/** Indicates that the renderer does not support the pause button found
		 * in the render progress dialog, and that the pause button should be disabled.*/
		kRequirement_NoPauseSupport = 0,
		/** Indicates that the VFB shouldn't be popped-up after rendering, even if 
		 * "Show VFB" is ON in the common render parameters. This is useful for
		 * renderers which generate something other than an image.
		 * Note that this also affects render element VFBs.*/
		kRequirement_NoVFB = 1,
		/** Indicates that the rendered image shouldn't be saved after rendering, even
		 * if a file was specified in the common render parameters. This is useful for
		 * renderers which generate something other than an image.
		 * Note that this also affects render element outputs.*/
		kRequirement_DontSaveRenderOutput = 2,
		/** Indicates the renderer wants a 32bit floating-point RGBA frame buffer to be
		 * created for output. The render executer will query the renderer and will create a
		 * 32bit floating-point frame buffer, instead of a 16bit integer buffer, if the
		 * renderer returns true for this requirement.
		 * Note that there is no guarantee about the frame buffer type: even if the renderer
		 * returns true for this requirement, a 16bit integer buffer could still be created.*/
		kRequirement8_Wants32bitFPOutput = 3,
		/** Indicates the renderer wants an object selection for rendering. 
		 * The render executer will throw an error message when the renderer indicates
		 * this requirement, and no objects are selected. */
		kRequirement11_WantsObjectSelection = 4,
	};

	// Returns true if the renderer has the given requirement, or returns false otherwise.
	virtual bool HasRequirement(Requirement requirement) = 0;

	// -- from BaseInterface
	virtual Interface_ID GetID();
};

#pragma warning(pop)

inline Interface_ID IRendererRequirements::GetID() {

	return IRENDERERREQUIREMENTS_INTERFACE_ID;
}

inline IRendererRequirements* GetRendererRequirements(Renderer* renderer) {
	return (renderer != NULL) ? static_cast<IRendererRequirements*>(renderer->GetInterface(IRENDERERREQUIREMENTS_INTERFACE_ID)) : NULL;
}

inline bool RendererHasRequirement(Renderer* renderer, IRendererRequirements::Requirement requirement) {
	IRendererRequirements* iRequirements = GetRendererRequirements(renderer);
	return (iRequirements != NULL) ? iRequirements->HasRequirement(requirement) : false;
}


