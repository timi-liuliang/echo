/**********************************************************************
 *<
	FILE: istdplug.h

	DESCRIPTION:  Interfaces into some of the standard plug-ins 
	              that ship with MAX

	CREATED BY: Rolf Berteig	

	HISTORY: created 20 January 1996

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "maxheap.h"
#include "iFnPub.h"
#include "units.h"
#include "BuildWarnings.h"
#include "assetmanagement\assetUser.h"
#include "object.h"
#include "quat.h"
#include "control.h"
#include "ipoint3.h"

//----------------------------------------------------------------
// The following are parameter block IDs for procedural objects

/*! \defgroup paramBlockIDs Parameter Block IDs
\sa BaseObject::GetParamBlockIndex
*/
//@{
/*! \defgroup paramBlockIDs_arc Arc Parameter Block IDs
*/
//@{
#define ARC_RADIUS		0
#define ARC_FROM		1
#define ARC_TO			2
#define ARC_PIE			3
#define ARC_REVERSE		4
//@}

/*! \defgroup paramBlockIDs_boxObject Box Object Parameter Block IDs */
//@{
#define BOXOBJ_LENGTH	0
#define BOXOBJ_WIDTH	1
#define BOXOBJ_HEIGHT	2
#define BOXOBJ_WSEGS	3
#define BOXOBJ_LSEGS	4
#define BOXOBJ_HSEGS	5
#define BOXOBJ_GENUVS	6
//@}

/*! \defgroup paramBlockIDs_circle Circle Parameter Block IDs */
//@{
#define CIRCLE_RADIUS		0
//@}

/*! \defgroup paramBlockIDs_cone Cone Parameter Block IDs */
//@{
#define CONE_RADIUS1		0
#define CONE_RADIUS2		1
#define CONE_HEIGHT			2
#define CONE_SEGMENTS		3
#define CONE_CAPSEGMENTS	4
#define CONE_SIDES			5
#define CONE_SMOOTH			6
#define CONE_SLICEON		7
#define CONE_PIESLICE1		8
#define CONE_PIESLICE2		9
#define CONE_GENUVS			10
//@}

/*! \defgroup paramBlockIDs_cylinder Cylinder Parameter Block IDs */
//@{
#define CYLINDER_RADIUS			0
#define CYLINDER_HEIGHT			1
#define CYLINDER_SEGMENTS		2
#define CYLINDER_CAPSEGMENTS	3
#define CYLINDER_SIDES			4
#define CYLINDER_SMOOTH			5
#define CYLINDER_SLICEON		6
#define CYLINDER_PIESLICE1		7
#define CYLINDER_PIESLICE2		8
#define CYLINDER_GENUVS			9
//@}

/*! \defgroup paramBlockIDs_donut Donut Parameter Block IDs */
//@{
#define DONUT_RADIUS1		0
#define DONUT_RADIUS2		1
//@}

/*! \defgroup paramBlockIDs_ellipse Ellipse Parameter Block IDs */
//@{
#define ELLIPSE_LENGTH		0
#define ELLIPSE_WIDTH		1
//@}

/*! \defgroup paramBlockIDs_hedra Hedra Parameter Block IDs */
//@{
#define HEDRA_RADIUS	0
#define HEDRA_FAMILY	1
#define HEDRA_P			2
#define HEDRA_Q			3
#define HEDRA_SCALEP	4
#define HEDRA_SCALEQ	5
#define HEDRA_SCALER	6
#define HEDRA_VERTS		7
#define HEDRA_GENUVS	8
//@}

/*! \defgroup paramBlockIDs_helix Helix  Parameter Block IDs*/
//@{
#define HELIX_RADIUS1		0
#define HELIX_RADIUS2		1
#define HELIX_HEIGHT		2
#define HELIX_TURNS			3
#define HELIX_BIAS			4
#define HELIX_DIRECTION		5
//@}

/*! \defgroup paramBlockIDs_ngon NGon Parameter Block IDs */
//@{
#define NGON_RADIUS			0
#define NGON_SIDES			1
#define NGON_CIRCULAR		2
//@}

/*! \defgroup paramBlockIDs_patchGrid PatchGrid Parameter Block IDs */
//@{
#define PATCHGRID_LENGTH	0
#define PATCHGRID_WIDTH		1
#define PATCHGRID_WSEGS		2
#define PATCHGRID_LSEGS		3
#define PATCHGRID_TEXTURE	4
//@}

/*! \defgroup paramBlockIDs_rainSnow Rain/Snow Parameter Block IDs */
//@{
#define RSPART_VPTPARTICLES		0
#define RSPART_RNDPARTICLES		1
#define RSPART_DROPSIZE			2
#define RSPART_SPEED			3
#define RSPART_VARIATION		4
#define RSPART_DISPTYPE			5
#define RSPART_STARTTIME		6
#define RSPART_LIFETIME			7
#define RSPART_EMITTERWIDTH		8
#define RSPART_EMITTERHEIGHT	9
#define RSPART_HIDEEMITTER		10
#define RSPART_BIRTHRATE		11
#define RSPART_CONSTANT			12
#define RSPART_RENDER			13
#define RSPART_TUMBLE			14
#define RSPART_SCALE			15
//@}

/*! \defgroup paramBlockIDs_rectangle Rectangle Parameter Block IDs */
//@{
#define RECTANGLE_LENGTH	0
#define RECTANGLE_WIDTH		1
#define RECTANGLE_FILLET	2
//@}

/*! \defgroup paramBlockIDs_sphere Sphere Parameter Block IDs */
//@{
#define SPHERE_RADIUS	0
#define SPHERE_SEGS		1
#define SPHERE_SMOOTH	2
#define SPHERE_HEMI		3
#define SPHERE_SQUASH	4
#define SPHERE_RECENTER	5
#define SPHERE_GENUVS	6
//@}

/*! \defgroup paramBlockIDs_star Star Parameter Block IDs */
//@{
#define START_RADIUS1		0
#define START_RADIUS2		1
#define START_POINTS		2
#define START_DISTORT		3
#define START_FILLET1		4
#define START_FILLET2		5
//@}

/*! \defgroup paramBlockIDs_teaPot Tea Pot Parameter Block IDs */
//@{
#define TEAPOT_RADIUS	0
#define TEAPOT_SEGS		1
#define TEAPOT_SMOOTH	2
#define TEAPOT_TEAPART	3
#define TEAPOT_BODY		4
#define TEAPOT_HANDLE	5
#define TEAPOT_SPOUT	6
#define TEAPOT_LID		7
#define TEAPOT_GENUVS	8
//@}

/*! \defgroup paramBlockIDs_text Text Parameter Block IDs */
//@{
#define TEXT_SIZE 0
#define TEXT_KERNING 1
#define TEXT_LEADING 2
//@}

/*! \defgroup paramBlockIDs_torus Torus Parameter Block IDs */
//@{
#define TORUS_RADIUS		0
#define TORUS_RADIUS2		1
#define TORUS_ROTATION		2
#define TORUS_TWIST			3
#define TORUS_SEGMENTS		4
#define TORUS_SIDES			5
#define TORUS_SMOOTH		6
#define TORUS_SLICEON		7
#define TORUS_PIESLICE1		8
#define TORUS_PIESLICE2		9
#define TORUS_GENUVS		10
//@}

/*! \defgroup paramBlockIDs_tube Tube Parameter Block IDs */
//@{
#define TUBE_RADIUS			0
#define TUBE_RADIUS2		1
#define TUBE_HEIGHT			2
#define TUBE_SEGMENTS		3
#define TUBE_CAPSEGMENTS	4
#define TUBE_SIDES			5
#define TUBE_SMOOTH			6
#define TUBE_SLICEON		7
#define TUBE_PIESLICE1		8
#define TUBE_PIESLICE2		9
#define TUBE_GENUVS			10
//@}

/*! \defgroup paramBlockIDs_grid Grid Parameter Block IDs */
//@{
#define GRIDHELP_LENGTH			0
#define GRIDHELP_WIDTH			1
#define GRIDHELP_GRID			2
//@}

//----------------------------------------------------------------
// The following are parameter block IDs for modifiers

/*! \defgroup paramBlockIDs_bend Bend Parameter Block IDs*/
//@{
#define BEND_ANGLE		0
#define BEND_DIR		1
#define BEND_AXIS		2
#define BEND_DOREGION	3
#define BEND_FROM		4
#define BEND_TO			5
//@}

/*! \defgroup paramBlockIDs_bomb Bomb Parameter Block IDs */
//@{
#define BOMB_STRENGTH		0
#define BOMB_GRAVITY		1
#define BOMB_CHAOS			2
#define BOMB_DETONATION		3
//@}

/*! \defgroup paramBlockIDs_deflector Deflector Parameter Block IDs */
//@{
#define DEFLECTOR_BOUNCE	0
#define DEFLECTOR_WIDTH		1
#define DEFLECTOR_HEIGHT	2
//@}

/*! \defgroup paramBlockIDs_displace Displace  Parameter Block IDs
For modifier and space warp object */
//@{
#define DISPLACE_MAPTYPE		0
#define DISPLACE_UTILE			1
#define DISPLACE_VTILE			2
#define DISPLACE_WTILE			3
#define DISPLACE_BLUR			4
#define DISPLACE_USEMAP			5
#define DISPLACE_APPLYMAP		6
#define DISPLACE_STRENGTH		7
#define DISPLACE_DECAY			8
#define DISPLACE_CENTERLUM		9
#define DISPLACE_UFLIP			10
#define DISPLACE_VFLIP			11
#define DISPLACE_WFLIP			12
#define DISPLACE_CENTERL		13
#define DISPLACE_CAP			14
#define DISPLACE_LENGTH			15
#define DISPLACE_WIDTH			16
#define DISPLACE_HEIGHT			17
#define DISPLACE_AXIS			18
//@}

/*! \defgroup paramBlockIDs_extrude Extrude Parameter Block IDs */
//@{
#define EXTRUDE_AMOUNT			0
#define EXTRUDE_SEGS			1
#define EXTRUDE_CAPSTART		2
#define EXTRUDE_CAPEND			3
#define EXTRUDE_CAPTYPE			4
#define EXTRUDE_OUTPUT			5
#define EXTRUDE_MAPPING			6
#define EXTRUDE_GEN_MATIDS		7
#define EXTRUDE_USE_SHAPEIDS	8
#define EXTRUDE_SMOOTH			9
//@}

/*! \defgroup paramBlockIDs_gravity Gravity Parameter Block IDs */
//@{
#define GRAVITY_STRENGTH		0
#define GRAVITY_DECAY			1
#define GRAVITY_TYPE			2
#define GRAVITY_DISPLENGTH		3
//@}

/*! \defgroup paramBlockIDs_wind Wind Parameter Block IDs */
//@{
#define WIND_STRENGTH		0
#define WIND_DECAY			1
#define WIND_TYPE			2
#define WIND_DISPLENGTH		3
#define WIND_TURBULENCE		4
#define WIND_FREQUENCY		5
#define WIND_SCALE			6
//@}

/*! \defgroup paramBlockIDs_uvwMap UVW Map Parameter Block IDs */
//@{
#define UVWMAP_MAPTYPE		0
#define UVWMAP_UTILE		1
#define UVWMAP_VTILE		2
#define UVWMAP_WTILE		3
#define UVWMAP_UFLIP		4
#define UVWMAP_VFLIP		5
#define UVWMAP_WFLIP		6
#define UVWMAP_CAP			7
#define UVWMAP_CHANNEL		8
#define UVWMAP_LENGTH		9
#define UVWMAP_WIDTH		10
#define UVWMAP_HEIGHT		11
#define UVWMAP_AXIS			12
//@}

/*! \defgroup paramBlockIDs_noiseMod Noise Mod Parameter Block IDs */
//@{
#define NOISEMOD_SEED			0
#define NOISEMOD_SCALE			1
#define NOISEMOD_FRACTAL		2
#define NOISEMOD_ROUGH			3
#define NOISEMOD_ITERATIONS		4
#define NOISEMOD_ANIMATE		5
#define NOISEMOD_FREQ			6
#define NOISEMOD_PHASE			7
#define NOISEMOD_STRENGTH		8
//@}

/*! \defgroup paramBlockIDs_optimize Optimize Parameter Block IDs */
//@{
#define OPTMOD_RENDER			0
#define OPTMOD_VIEWS			1
#define OPTMOD_FACETHRESH1		2
#define OPTMOD_EDGETHRESH1		3
#define OPTMOD_BIAS1			4
#define OPTMOD_PRESERVEMAT1		5
#define OPTMOD_PRESERVESMOOTH1	6
#define OPTMOD_MAXEDGE1			7
#define OPTMOD_FACETHRESH2		8
#define OPTMOD_EDGETHRESH2		9
#define OPTMOD_BIAS2			10
#define OPTMOD_PRESERVEMAT2		11
#define OPTMOD_PRESERVESMOOTH2	12
#define OPTMOD_MAXEDGE2			13
#define OPTMOD_AUTOEDGE			14
#define OPTMOD_MANUPDATE		15
//@}

/*! \defgroup paramBlockIDs_volumeSelectionModifier Volume Selection Modifier Parameter Block IDs */
//@{
#define VOLSEL_LEVEL	0
#define VOLSEL_METHOD	1
#define VOLSEL_TYPE		2
#define VOLSEL_VOLUME	3
#define VOLSEL_INVERT	4
//@}

/*! \defgroup paramBlockIDs_rippleWaveModifier Ripple/Wave Modifier Parameter Block IDs
For Space Warp Object and Object Space */
//@{
#define RWAVE_AMPLITUDE		0
#define RWAVE_AMPLITUDE2	1
#define RWAVE_WAVELEN		2
#define RWAVE_PHASE			3
#define RWAVE_DECAY			4

#define RWAVE_CIRCLES		5 //!< Valid for space warp objects only
#define RWAVE_SEGMENTS		6 //!< Valid for space warp objects only
#define RWAVE_DIVISIONS		7 //!< Valid for space warp objects only
//@}

/*! \defgroup paramBlockIDs_rippleWaveBinding  Ripple/Wave Binding (modifier) Parameter Block IDs */
//@{
#define RWAVE_FLEX			0
//@}

/*! \defgroup paramBlockIDs_skew Skew Parameter Block IDs */
//@{
#define SKEW_AMOUNT		0
#define SKEW_DIR		1
#define SKEW_AXIS		2
#define SKEW_DOREGION	3
#define SKEW_FROM		4
#define SKEW_TO			5
//@}

/*! \defgroup paramBlockIDs_materialModifier Material Modifier Parameter Block IDs */
//@{
#define MATMOD_MATID 0
//@}

/*! \defgroup paramBlockIDs_smoothingGroupModifier Smoothing Group Modifier Parameter Block IDs */
//@{
#define SMOOTHMOD_AUTOSMOOTH 	0
#define SMOOTHMOD_THRESHOLD		1
#define SMOOTHMOD_SMOOTHBITS	2
//@}

/*! \defgroup paramBlockIDs_normalModifier Normal Modifier Parameter Block IDs */
//@{
#define NORMMOD_UNIFY	0
#define NORMMOD_FLIP 	1
//@}

/*! \defgroup paramBlockIDs_surfRevModifier SurfRev (Lathe) Modifier Parameter Block IDs */
//@{
#define SURFREV_DEGREES			0
#define SURFREV_SEGS			1
#define SURFREV_CAPSTART		2
#define SURFREV_CAPEND			3
#define SURFREV_CAPTYPE			4
#define SURFREV_WELDCORE		5
#define SURFREV_OUTPUT			6
#define SURFREV_MAPPING			7
//@}

/*! \defgroup paramBlockIDs_taper Taper Parameter Block IDs */
//@{
#define TAPER_AMT			0
#define TAPER_CRV			1
#define TAPER_AXIS			2
#define TAPER_EFFECTAXIS	3
#define TAPER_SYMMETRY		4
#define TAPER_DOREGION		5
#define TAPER_FROM			6
#define TAPER_TO			7
//@}

/*! \defgroup paramBlockIDs_twist Twist Parameter Block IDs */
//@{
#define TWIST_ANGLE		0
#define TWIST_BIAS		1
#define TWIST_AXIS		2
#define TWIST_DOREGION	3
#define TWIST_FROM		4
#define TWIST_TO		5
//@}

/*! \defgroup paramBlockIDs_materialMod Material Mod Parameter Block IDs */
//@{
#define MATMOD_MATID	0
//@}

/*! \defgroup paramBlockIDs_smoothMod Smooth Mod Parameter Block IDs */
//@{
#define SMOOTH_AUTOSMOOTH 	0
#define SMOOTH_THRESHOLD	1
#define SMOOTH_SMOOTHBITS	2
//@}

/*! \defgroup paramBlockIDs_normalMod Normal Mod Parameter Block IDs */
//@{
#define NORMALMOD_UNIFY		0
#define NORMALMOD_FLIP 		1
//@}

/*! \defgroup paramBlockIDs_tessellationMod Tessellation Mod Parameter Block IDs */
//@{
#define TESSMOD_TYPE		0
#define TESSMOD_TENSION		1
#define TESSMOD_ITERATIONS	2
#define TESSMOD_FACE_TYPE	3
//@}

/*! \defgroup paramBlockIDs_uvmXform UVW Xform Parameter Block IDs */
//@{
#define UVWXFORM_UTILE		0
#define UVWXFORM_VTILE		1
#define UVWXFORM_WTILE		2
#define UVWXFORM_UOFFSET	3
#define UVWXFORM_VOFFSET	4
#define UVWXFORM_WOFFSET	5
#define UVWXFORM_UFLIP		6
#define UVWXFORM_VFLIP		7
#define UVWXFORM_WFLIP		8
#define UVWXFORM_CHANNEL	9
//@}

//@} 	END OF PARAMETER BLOCK IDS

//-- Text shape object interface -------------------------

// Use GetTextObjectInterface() to get a pointer to an 
// ITextObject given a pointer to an Object. 


// Flags passed to ChangeFont()
#define TEXTOBJ_ITALIC		(1<<1)
#define TEXTOBJ_UNDERLINE	(1<<2)

// Alignment types
#define TEXTOBJ_LEFT 0
#define TEXTOBJ_CENTER 1
#define TEXTOBJ_RIGHT 2
#define TEXTOBJ_JUSTIFIED 3

/*! \sa  Class Animatable.\n\n
\par Description:
This is the text shape object interface. This class gives access to the
standard 3ds Max text object. It allows the text objects font, string, and
style bits to be retrieved and set. All methods of this class are implemented
by the system.\n\n
To get a pointer to an <b>ITextObject</b> interface given a pointer to a
object, use the following macro (defined in AnimatableInterfaceIDs.h ). Using this
macro, given any Animatable, it is easy to ask for the text object
interface.\n\n
<b>#define GetTextObjectInterface(anim)</b>\n\n
<b>((ITextObject*)anim-\>GetInterface(I_TEXTOBJECT))</b>\n\n
A plug-in developer may use this macro as follows:\n\n
<b>ITextObject *ito = GetTextObjectInterface(anim);</b>\n\n
This return value will either be NULL or a pointer to a valid text object
interface. You may then use this pointer to call methods of this class to
retrieve and modify the object data. For example:\n\n
<b>ito-\>SetUnderline(TRUE);</b>\n\n
Note: Some aspects of the text are controlled by its parameter block.
Developers can access the parameter block by calling
<b>ito-\>GetParamBlock()</b>. The following are the indices into the parameter
block used to access the size, kerning and leading parameters:\n\n
<b>TEXT_SIZE</b>\n\n
<b>TEXT_KERNING</b>\n\n
<b>TEXT_LEADING</b>  */
class ITextObject: public MaxHeapOperators {
	public:
		// Returns TRUE if string is changed. Can't change string if current font is not installed
		/*! \remarks This method may be called to change the text string. Note
		that you can't change the string if the current font is not installed.
		\par Parameters:
		<b>MSTR string</b>\n\n
		The new text string.
		\return  TRUE if the string is changed; otherwise FALSE. */
		virtual BOOL ChangeText(MSTR string)=0;
		
		// Returns TRUE if font is successfully changed.
		/*! \remarks This method may be called to change the text font.
		\par Parameters:
		<b>MSTR name</b>\n\n
		The name of the font.\n\n
		<b>DWORD flags</b>\n\n
		One or more of the following values:\n\n
		<b>TEXTOBJ_ITALIC</b>\n\n
		<b>TEXTOBJ_UNDERLINE</b>
		\return  TRUE if the font was successfully changed; otherwise FALSE. */
		virtual BOOL ChangeFont(MSTR name, DWORD flags)=0;

		// Get fount and string
		/*! \remarks Returns the name of the text font. */
		virtual MSTR GetFont()=0;
		/*! \remarks Returns the text string. */
		virtual MSTR GetString()=0;
		
		// Get/Set styles
		/*! \remarks Returns TRUE if the text is italicized; otherwise FALSE.
		*/
		virtual BOOL GetItalic()=0;
		/*! \remarks Returns TRUE if the text is underlined; otherwise FALSE.
		*/
		virtual BOOL GetUnderline()=0;
		/*! \remarks Sets if the text is italicized or not.
		\par Parameters:
		<b>BOOL sw</b>\n\n
		TRUE if the text should be italicized; FALSE if not. */
		virtual void SetItalic(BOOL sw)=0;
		/*! \remarks Sets if the text is underlined or not.
		\par Parameters:
		<b>BOOL sw</b>\n\n
		TRUE if the text should be underlined; FALSE if not. */
		virtual void SetUnderline(BOOL sw)=0;

		// Get/Set alignment
		virtual BOOL SetAlignment(int type)=0;
		virtual int GetAlignment()=0;
	};



//-- Controller interfaces -------------------------------

// Base key class
/*! \sa  Class IKeyControl, Class Animatable.\n\n
\par Description:
This is the base class for keys that are part of the controller interface. This
class stores the time of the key, and some flags that describe the properties
of the key.
\par Data Members:
<b>TimeValue time;</b>\n\n
The time of the key.\n\n
<b>DWORD flags;</b>\n\n
The flag bits for keys. One or more of the following values:\n\n
General flags\n\n
<b>IKEY_SELECTED</b>\n\n
The key is selected.\n\n
<b>IKEY_XSEL</b>\n\n
In the function curve editor X is selected.\n\n
<b>IKEY_YSEL</b>\n\n
In the function curve editor Y is selected.\n\n
<b>IKEY_ZSEL</b>\n\n
In the function curve editor Z is selected.\n\n
<b>IKEY_FLAGGED</b>\n\n
The key is flagged. See <b>Animatable:: FlagKey()</b>.\n\n
<b>IKEY_TIME_LOCK</b>\n\n
The key is locked in time so it won't move.\n\n
TCB specific key flags:\n\n
<b>TCBKEY_QUATVALID</b>\n\n
The quaternion TCB key has inside it both a quaternion and an angle axis. When
this bit is set the angle/axis is derived from the quaternion instead of
vice/versa.\n\n
Bezier specific key flags:\n\n
<b>BEZKEY_CONSTVELOCITY</b>\n\n
This key is interpolated using arclength as the interpolation parameter.\n\n
<b>BEZKEY_XBROKEN</b>\n\n
<b>BEZKEY_YBROKEN</b>\n\n
<b>BEZKEY_ZBROKEN</b>\n\n
Indicates if the tangent handles are locked together. Broken means not
locked.\n\n
The following macros may be used to test and set the tangent locks:\n\n
<b>TangentsLocked(f,j);</b>\n\n
<b>SetTangentLick(f,j,l);</b>\n\n
The following macros may be used to access the hybrid tangent types:\n\n
<b>GetInTanType(f);</b>\n\n
<b>GetOutTanType(f);</b>\n\n
<b>SetINTanType(f,t);</b>\n\n
<b>SetOutTanType(f,t);</b>  */
class IKey: public MaxHeapOperators {
	public:
		TimeValue time;
		DWORD flags;
		/*! \remarks Constructor. The time and flags are set to zero. */
		IKey() {time=0;flags=0;}
	};

//--- TCB keys -------------

/*! \sa  Class IKey.\n\n
\par Description:
This is the base class for Tension Continuity and Bias keys.
\par Data Members:
<b>float tens;</b>\n\n
The tension setting. Values are in the range -1.0 to 1.0, where 0.0 is the
default.\n\n
<b>float cont;</b>\n\n
The continuity setting. Values are in the range -1.0 to 1.0, where 0.0 is the
default.\n\n
<b>float bias;</b>\n\n
The bias setting. Values are in the range -1.0 to 1.0, where 0.0 is the
default.\n\n
<b>float easeIn;</b>\n\n
The ease in value. Values are in the range 0.0 to 50.0, where25.0 is the
default.\n\n
<b>float easeOut;</b>\n\n
The ease out value. Values are in the range 0.0 to 50.0, where 25.0 is the
default. */
class ITCBKey : public IKey {
	public:		
		float tens, cont, bias, easeIn, easeOut;
	};

/*! \sa  Class ITCBKey.\n\n
\par Description:
This class stores a Tension Continuity and Bias (TCB) floating point key.
\par Data Members:
<b>float val;</b>\n\n
The value of the key. */
class ITCBFloatKey : public ITCBKey {
	public:
		float val;		
	};

class ITCBPoint4Key : public ITCBKey {
public:
	Point4 val;		
};

/*! \sa  Class ITCBKey, Class Point3.\n\n
\par Description:
This class stores a Tension Continuity and Bias (TCB) Point3 key.
\par Data Members:
<b>Point3 val;</b>\n\n
The value of the key. */
class ITCBPoint3Key : public ITCBKey {
	public:
		Point3 val;		
	};

/*! \sa  Class ITCBKey, Class AngAxis.\n\n
\par Description:
This class stores a Tension Continuity and Bias (TCB) rotation key.
\par Data Members:
<b>AngAxis val;</b>\n\n
The value of the key. */
class ITCBRotKey : public ITCBKey {
	public:
		AngAxis val;		
	};

/*! \sa  Class ITCBKey, Class ScaleValue.\n\n
\par Description:
This class stores a Tension Continuity and Bias (TCB) Scale key.
\par Data Members:
<b>ScaleValue val;</b>\n\n
The value of the key. */
class ITCBScaleKey : public ITCBKey {
	public:
		ScaleValue val;		
	};


//--- Bezier keys -------------

/*! \sa  Class IKey.\n\n
\par Description:
This class stores a Bezier floating point key.
\par Data Members:
<b>float intan;</b>\n\n
The in tangent value.\n\n
<b>float outtan;</b>\n\n
The out tangent value.\n\n
<b>float val;</b>\n\n
The value of the key.\n\n
<b>float inLength;</b>\n\n
The in length of the horizontal handle.\n\n
<b>float outLength;</b>\n\n
The out length of the horizontal handle. */
class IBezFloatKey : public IKey {
	public:
		float intan, outtan;
		float val;
//watje horizontal handles
//these are the length of the handles
		float inLength, outLength;
	};

/*! \sa  Class IKey, Class Point3.\n\n
\par Description:
This class stores a Bezier Point3 key.\n\n
The value of the intan and outtan values of IBezPoint3Key returned from GetKey
is actually the tangent of the angle that is spanned between the horizontal
axis and the tangent. In order to get the actual handle of the tangent it is
important to know that the horizontal (time) distance from the handle to the
key value is dependent on the previous (for intan) or next (for outtan) point.
The horizontal distance is basically a third of the total distance between the
current key and the previous (or next) key :\n\n
<b>Control *c = node-\>GetTMController()-\>GetPositionController();</b>\n\n
<b>dt = (c-\>GetKeyTime(PreviousIdx)-c-\>GetKeyTime(CurrentIdx)) /
3.0f;</b>\n\n
since the value in IBezPoint3Key pos_key.intan.x is the tan(alpha), you can
easily get the vertical location of the handle by calculating :\n\n
<b>tan(alpha) = pos_key.intan.x = dy/dt -\> dy = pos_key.intan.x * dt</b>\n\n
dt is the horizontal coordinate of the tangent handle relative to the key
value.\n\n
dy is the vertical coordinate of the tangent handle relative to the key value.
\par Data Members:
<b>Point3 intan;</b>\n\n
The incoming tangent vector. This can be thought of as the tangent handle of a
3D spline. The value is relative to <b>val</b>.\n\n
<b>Point3 outtan;</b>\n\n
The outgoing tangent vector. This can be thought of as the tangent handle of a
3D spline. The value is relative to <b>val</b>.\n\n
<b>Point3 val;</b>\n\n
The value of the key.\n\n
<b>Point3 inLength;</b>\n\n
The in length of the horizontal handle.\n\n
<b>Point3 outLength;</b>\n\n
The out length of the horizontal handle. */
class IBezPoint3Key : public IKey {
	public:
		Point3 intan, outtan;
		Point3 val;
//watje horizontal handles
//these are the length of the handles
		Point3 inLength, outLength;
	};

/*! \sa  Class IKey, Class Quat.\n\n
\par Description:
This class stores a Bezier quaternion key.
\par Data Members:
<b>Quat val;</b>\n\n
The value of the key. */
class IBezQuatKey : public IKey  {
	public:		
		Quat val;
	};

/*! \sa  Class IKey, Class Point3, Class ScaleValue.\n\n
\par Description:
This class stores a Bezier scale key.
\par Data Members:
<b>Point3 intan;</b>\n\n
The int tangent vector.\n\n
<b>Point3 outtan;</b>\n\n
The out tangent vector.\n\n
<b>ScaleValue val;</b>\n\n
The value of the key.\n\n
<b>Point3 inLength;</b>\n\n
The in length of the horizontal handle.\n\n
<b>Point3 outLength;</b>\n\n
The out length of the horizontal handle. */
class IBezScaleKey : public IKey  {
	public:
		Point3 intan, outtan;
		ScaleValue val;
//watje horizontal handles
//these are the length of the handles
		Point3 inLength, outLength;
	};

/*! \sa  Class IKey, Class Point4.\n\n
\par Description:
This class stores a Bezier Point4 key.
\par Data Members:
<b>Point4 intan;</b>\n\n
The int tangent vector.\n\n
<b>Point4 outtan;</b>\n\n
The out tangent vector.\n\n
<b>Point4 val;</b>\n\n
The value of the key.\n\n
<b>Point4 inLength;</b>\n\n
The in length of the horizontal handle.\n\n
<b>Point4 outLength;</b>\n\n
The out length of the horizontal handle. */
class IBezPoint4Key : public IKey {
public:
	Point4 intan, outtan;
	Point4 val;
	//watje horizontal handles
	//these are the length of the handles
	Point4 inLength, outLength;
};

/*! \sa  Class IKey, Class Point2.\n\n
\par Description:
This class stores a Bezier Point2 key.
\par Data Members:
<b>Point2 intan;</b>\n\n
The int tangent vector.\n\n
<b>Point2 outtan;</b>\n\n
The out tangent vector.\n\n
<b>Point2 val;</b>\n\n
The value of the key.\n\n
<b>Point2 inLength;</b>\n\n
The in length of the horizontal handle.\n\n
<b>Point2 outLength;</b>\n\n
The out length of the horizontal handle. */
class IBezPoint2Key : public IKey {
public:
	Point2 intan, outtan;
	Point2 val;
	//watje horizontal handles
	//these are the length of the handles
	Point2 inLength, outLength;
};


//--- Linear Keys --------------

/*! \sa  Class IKey.\n\n
\par Description:
This class stores a Linear floating point key.
\par Data Members:
<b>float val;</b>\n\n
The value of the key. */
class ILinFloatKey : public IKey {
	public:
		float val;
	};

/*! \sa  Class IKey, Class Point3.\n\n
\par Description:
This class stores a Linear Point3 key.
\par Data Members:
<b>Point3 val;</b>\n\n
The value of the key. */
class ILinPoint3Key : public IKey {
	public:
		Point3 val;
	};

/*! \sa  Class IKey, Class Quat.\n\n
\par Description:
This class stores a Linear rotation key.
\par Data Members:
<b>Quat val;</b>\n\n
The value of the key. */
class ILinRotKey : public IKey {
	public:
		Quat val;
	};

/*! \sa  Class IKey,
Class ScaleValue.\n\n
\par Description:
This class stores a Linear scale key.
\par Data Members:
<b>ScaleValue val;</b>\n\n
The value of the key. */
class ILinScaleKey : public IKey {
	public:
		ScaleValue val;
	};

//--- Boolean Controller Keys -------------- AG: 11/08/01

/*! \sa  : Class Animatable, Class IKey\n\n
class IBoolFloatKey : public IKey\n\n

\par Description:
<b>This class is only available in release 5 or later.</b>\n\n
  \remarks Derived from standard IKey with an additional data called
val.\n\n

\par Parameters:
<b>float val;</b>\n\n
Value of the last key which can only be either 0 or 1 */
class IBoolFloatKey : public IKey {
	public:
		float val;
	};



/*! \defgroup keyFlagBits Flag Bits for Keys */
//@{

/*! \defgroup keyFlagBits_generalFlags General Key Flags
*/
//@{
#define IKEY_SELECTED	(1<<0)
#define IKEY_XSEL		(1<<1)
#define IKEY_YSEL		(1<<2)
#define IKEY_ZSEL		(1<<3)
#define IKEY_WSEL		(1<<30)
#define IKEY_FLAGGED	(1<<31)  //13
#define IKEY_TIME_LOCK	(1<<14)

#define IKEY_ALLSEL		(IKEY_SELECTED|IKEY_XSEL|IKEY_YSEL|IKEY_ZSEL|IKEY_WSEL)

#define IKEY_VALLOCK_SHIFT	16
#define IKEY_VALX_LOCK		(1<<IKEY_VALLOCK_SHIFT)
#define IKEY_VALY_LOCK		(1<<(IKEY_VALLOCK_SHIFT+1))
#define IKEY_VALZ_LOCK		(1<<(IKEY_VALLOCK_SHIFT+2))
#define IKEY_VALA_LOCK		(1<<(IKEY_VALLOCK_SHIFT+3))
//@}

/*! \defgroup keyFlagBits_tcbSpecific TCB-specific Key Flags */
//@{
#define TCBKEY_QUATVALID	(1<<4) //!< Angle/axis is derived from the quat instead of vice/versa
//@}

/*! \defgroup keyFlagBits_bezierSpecific Bezier-specific Key Flags */
//@{
#define BEZKEY_XBROKEN		(1<<4) //!< Broken means not locked
#define BEZKEY_YBROKEN		(1<<5)
#define BEZKEY_ZBROKEN		(1<<6)
#define BEZKEY_WBROKEN		(1<<21)
//@}

/*! \defgroup keyFlagBits_inOutTypes In and Out types
Stored in bits 7-13 */
//@{
#define BEZKEY_NUMTYPEBITS	3
#define BEZKEY_INTYPESHIFT	7
#define	BEZKEY_OUTTYPESHIFT	(BEZKEY_INTYPESHIFT+BEZKEY_NUMTYPEBITS)
#define BEZKEY_TYPEMASK		7
//@}

/*! \defgroup keyFlagBits_bezierTangentTypes Bezier Tangeant Types */
//@{
#define BEZKEY_SMOOTH	0
#define BEZKEY_LINEAR	1
#define BEZKEY_STEP		2
#define BEZKEY_FAST		3
#define BEZKEY_SLOW		4
#define BEZKEY_USER		5
#define BEZKEY_FLAT		6

#define NUM_TANGENTTYPES	7
//@}

/*! \defgroup keyFlagBits_macros Key Flagbit Macros */
//@{
#define BEZKEY_CONSTVELOCITY	(1<<15) //!< Interpolated using arclength as the interpolation parameter
#define BEZKEY_UNCONSTRAINHANDLE	(1<<20) //!< Determines whether a user handle is limited

#define TangentsLocked(f,j) (!(f&(j <= 2 ? (BEZKEY_XBROKEN<<j) : BEZKEY_WBROKEN)))
#define SetTangentLock(f,j,l) {if (l) (f)=(f)&(~(j <= 2 ? (BEZKEY_XBROKEN<<j) : BEZKEY_WBROKEN)); else (f)|=(j <= 2 ? (BEZKEY_XBROKEN<<j) : BEZKEY_WBROKEN);}
//@}

/*! \defgroup keyFlagBits_hybridTangeantAccessMacros Macros to Access Hybrid Tangent Types */
//@{
#define GetInTanType(f)  int(((f)>>BEZKEY_INTYPESHIFT)&BEZKEY_TYPEMASK)
#define GetOutTanType(f) int(((f)>>BEZKEY_OUTTYPESHIFT)&BEZKEY_TYPEMASK)
#define SetInTanType(f,t)  {(f) = ((f)&(~(BEZKEY_TYPEMASK<<BEZKEY_INTYPESHIFT)))|(t<<BEZKEY_INTYPESHIFT);}
#define SetOutTanType(f,t) {(f) = ((f)&(~(BEZKEY_TYPEMASK<<BEZKEY_OUTTYPESHIFT)))|(t<<BEZKEY_OUTTYPESHIFT);}
//@}
//@}	END OF KEYFLAGBITS GROUP

// HitTrackRecord flags
// KEY_XSEL, KEY_YSEL, KEY_ZSEL, and KEY_WSEL are also used to identify the component
#define HITKEY_INTAN	(1<<10)
#define HITKEY_OUTTAN	(1<<11)

// Track flags
#define TFLAG_CURVESEL			(1<<0)
#define TFLAG_RANGE_UNLOCKED	(1<<1)
#define TFLAG_LOOPEDIN			(1<<3)
#define TFLAG_LOOPEDOUT			(1<<4)
#define TFLAG_COLOR				(1<<5)	// Set for Bezier Point3/Point4 controlers that are color controllers
#define TFLAG_HSV				(1<<6)	// Set for color controls that interpolate in HSV
#define TRACK_XLOCKED			(1<<7)	// Used by controller to lock Y and Z to X.
#define KT_FLAG_DELAY_KEYSCHANGED (1<<8)
#define TFLAG_NOTKEYABLE		(1<<9)
#define TFLAG_TCBQUAT_NOWINDUP	(1<<10)

//! \brief Interface to the Key Property

//! This interface allows defining properties for keys.
//! Properties are additional attributes that are not interpolated
//! between keys.
class IKeyProperty: public MaxHeapOperators {
public:
	//! Virtual destructor
	virtual ~IKeyProperty() {}

	//! Property ID enumeration.
	enum PropID {
		//! Property ID of IIKKeyProp
		kIKProp
	};

	//! Get the Property ID.
	//! \return the property ID enum.
	virtual PropID GetPropID() const = 0;
};

//! \brief Interface to the IK Key Property.

//! This class defines the interface to the IK key property. 
//! This property has a property ID of IKeyProperty::kIKProp.
//! A pointer to IKeyProperty whose propertyID is kIKProp
//! can be safely cast to a pointer to an IIKKeyProp: 
//! \code
//! if (ptr->GetPropID() == IKeyProperty::kIKProp)
//!		IIKKeyProp* ikProp = (IIKKeyProp*)ptr;
//! \endcode
class IIKKeyProp : public IKeyProperty {
public:
	//! The enum type for the space with regard to which the IK goal is interpreted.
	enum EnumIKSpace {
		//! The body space is the parent space of the start bone shifted by the position of the start bone.
		kBodySpace,

		//! The object space if the parent space of the IK goal object.
		kObjectSpace,
	};

	//! The Property ID of this interface.
	//! \see IKeyProperty::GetPropID
	PropID GetPropID() const { return IKeyProperty::kIKProp; }

	//! Get the IK Blend attribute of the property.
	//! It should be in the range of [0,1].
	virtual float		GetIKBlend() const =0;

	//! Get the IK Space attribute of the property.
	virtual EnumIKSpace	GetIKSpace() const =0;

	//! Get the Pivot On attribute of the property.
	virtual bool		GetPivotOn() const =0;

	//! Get the World Pivot attribute of the property.
	virtual Point3		GetWorldPivot() const =0;

	//! Get the Pivot Node attribute of the property.
	virtual INode*		GetPivotNode() const =0;

	//! When passed in IKeyControl2::SetKeyProp(), it tells whether to set value to the
	//! IK Blend attribute. If it is true, GetIKBlend() will be used as the
	//! target of the set value.
	virtual bool		SetIKBlend() const { return false; }

	//! When passed in IKeyControl2::SetKeyProp(), it tells whether to set value to the
	//! IK Space attribute. If it is true, GetIKSpace() will be used as the
	//! target of the set value.
	virtual bool		SetIKSpace() const { return false; }

	//! When passed in IKeyControl2::SetKeyProp(), it tells whether to
	//! set value to the Pivot On attribute. If it is true, GetPivotOn()
	//! will be used as the target of the set value.
	virtual bool		SetPivotOn() const { return false; }

	//! When passed in IKeyControl2::SetKeyProp(), it tells whether to
	//! set value to the World Pivot attribute. If it is true, GetWorldPivot()
	//! will be used as the target of the set value.
	virtual bool		SetWorldPivot() const { return false; }

	//! When passed in IKeyControl2::SetKeyProp(), it tells whether to
	//! set value to the Pivot Node attribute. If it is true, GetPivotNode()
	//! will be used as the target of the set value.
	virtual bool		SetPivotNode() const { return false; }
};

//! This is a helper class. If we want to set just one attribute of the
//! IK Key Property, we can derive a new class from it as follows:
//! \code
//! class SetIKBlendProp : public ISetIKKeyProp {
//! public:
//!     SetIKBlendProp(float blend) : mBlend(blend) {}
//!     float GetIKBlend() const { return mBlend; }
//!     bool  SetIKBlend() const { return true; }
//!     float mBlend;
//! };
//! IKeyControl2* keyControl = (IKeyControl2*)control->GetInterface(I_KEYCONTROL2);
//! SetIKBlendProp prop(new_blend_value);
//! keyControl->SetKeyProp(IKeyProperty::kIKProp, key_index, prop); 
//! \endcode
class ISetIKKeyProp : public IIKKeyProp {
public:
	//! Override IIKKeyProp::GetIKBlend().
	float GetIKBlend() const { return 0.0f; }
	//! Override IIKKeyProp::GetIKSpace().
	EnumIKSpace GetIKSpace() const { return kObjectSpace; }
	//! Override IIKKeyProp::GetPivotOn().
	bool GetPivotOn() const { return false; }
	//! Override IIKKeyProp::GetWorldPivot().
	Point3 GetWorldPivot() const { return Point3::Origin; }
	//! Override IIKKeyProp::GetPivotNode().
	INode* GetPivotNode() const { return NULL; }
};

//-------------------------------------------------------
// This is an interface into key frame controllers. 
// To get a pointer to the IKeyControl interface given a pointer to a controller,
// use the macro defined in AnimatableInterfaceIDs.h: GetKeyControlInterface()
// Use class AnyKey as wrapper for IKey for automatic handling of memory. See example below

/*! \sa  Class Animatable, Class IKey,  Class Control,  <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_super_class_ids.html">List of SuperClass IDs</a>,
<a href="ms-its:3dsmaxsdk.chm::/anim_keyframe_and_procedural_controller_data_access.html">Keyframe and Procedural Controller Data Access</a>.\n\n
\par Description:
This is an interface into the TCB, Linear, and Bezier keyframe controllers. It
allows a developer to add, delete, retrieve and store the keys of the
controller. This is for controllers that have made their keys accessible using
this interface. 3ds Max has done this for its keyframe controllers. It is up to
other third party developers to decide if they wish to make their keys
available through this interface as well. See below for more details.\n\n
It is up to the developer to make sure that the <b>IKey*</b> points to a key of
the appropriate derived class based on the <b>ClassID()</b> of the controller.
For the details of using these APIs see the Advanced Topics section
<a href="ms-its:3dsmaxsdk.chm::/anim_keyframe_and_procedural_controller_data_access.html">Keyframe
and Procedural Cotroller Data Access</a>.\n\n
All methods of this class are implemented by the system.\n\n
To get a pointer to this interface given a pointer to a controller, use the
following macro (defined in AnimatableInterfaceIDs.h). Using this macro, given any
Animatable, it is easy to ask for the control interface.\n\n
<b>#define GetKeyControlInterface(anim)</b>\n\n
<b>((IKeyControl*)anim-\>GetInterface(I_KEYCONTROL))</b>\n\n
A plug-in developer may use this macro as follows:\n\n
<b>IKeyControl *ikc = GetKeyControlInterface(anim);</b>\n\n
This return value will either be NULL or a pointer to a valid controller
interface. Here is an example of getting the controller interface from a node
in the scene. First get the position controller from the node (see
Class INode) and then get the controller interface.\n\n
<b> Control *c;</b>\n\n
<b> c = node-\>GetTMController()-\>GetPositionController();</b>\n\n
<b> IKeyControl *ikeys = GetKeyControlInterface(c);</b>\n\n
With this controller interface you can use its methods to get information about
the keys.\n\n
<b> int num = ikeys-\>GetNumKeys();</b>\n\n
Developers should note that the values that are retrieved from this class may
differ from the values that appear in Key Info in the 3ds Max user interface.
For instance, the Intan and Outtan values are multiplied by the global function
<b>GetFrameRate()</b> when displayed. Additionally, the sign of angles (+ or -)
may be reversed from the what is found in the UI. For example, the following
shows the values shown in Key Info versus the values retrieved from
<b>GetKey()</b>:\n\n
Motion branch Key Info:\n\n
<b>Key#1</b>\n\n
<b>X: -1.0</b>\n\n
<b>Y: 0.0</b>\n\n
<b>Z: 0.0</b>\n\n
<b>Ang: 0.0</b>\n\n
<b>Key#2</b>\n\n
<b>X: 0.0</b>\n\n
<b>Y: 1.0</b>\n\n
<b>Z: 0.0</b>\n\n
<b>Ang: 90.0</b>\n\n
<b>Key#3</b>\n\n
<b>X: 0.0</b>\n\n
<b>Y: 0.0</b>\n\n
<b>Z: 1.0</b>\n\n
<b>Ang: 90.0</b>\n\n
<b>ITCBRotKey key;</b>\n\n
<b>ikc-\>GetKey(i, \&key);</b>\n\n
<b>Key#1</b>\n\n
<b>X: 1.0</b>\n\n
<b>Y: 0.0</b>\n\n
<b>Z: 0.0</b>\n\n
<b>Ang: 0.0</b>\n\n
<b>Key#2</b>\n\n
<b>X: 0.0</b>\n\n
<b>Y: -1.0</b>\n\n
<b>Z: 0.0</b>\n\n
<b>Ang: 1.57</b>\n\n
<b>Key#3</b>\n\n
<b>X: 0.0</b>\n\n
<b>Y: 0.0</b>\n\n
<b>Z: -1.0</b>\n\n
<b>Ang: 1.57</b>\n\n
Sample code in the SDK that makes use of this interface is the 3D Studio Export
plug-in. See <b>/MAXSDK/SAMPLES/IMPEXP/3DSEXP.CPP</b>.
\par Tension/Continuity/Bias:
Class ITCBKey\n\n
Class ITCBFloatKey\n\n
Class ITCBPoint3Key,\n\n
Class ITCBRotKey\n\n
Class ITCBScaleKey
\par Bezier:
Class IBezFloatKey\n\n
Class IBezPoint3Key\n\n
Class IBezQuatKey\n\n
Class IBezScaleKey
\par Linear:
Class ILinFloatKey\n\n
Class ILinPoint3Key\n\n
Class ILinRotKey\n\n
Class ILinScaleKey\n\n
Note: Developers creating controller plug-ins may wish to make their keys
accessible to others through this interface. The way 3ds Max does this is by
deriving the controllers from this class (<b>IKeyControl</b>) in addition to
class <b>Control</b>. So, multiple inheritance is used, and 3ds Max then
implements the methods of this class to provide the interface.\n\n
Below is the code from 3ds Max implementation of
<b>Animatable::GetInterface()</b> (as part of a template). Note the cast of the
<b>this</b> pointer to <b>IKeyControl</b>.\n\n
\code
INTERP_CONT_TEMPLATE
void* InterpControl<INTERP_CONT_PARAMS>::GetInterface(ULONG id)
{
	if (id==I_KEYCONTROL) {
		return (IKeyControl*)this;
	}
	else {
		return Control::GetInterface(id);
	}
}
\endcode  */
class IKeyControl: public MaxHeapOperators {
	public:
		// Total number of keys.
		/*! \remarks Returns the total number of keys. */
		virtual int GetNumKeys()=0;
		
		// Sets the number of keys allocated. 
		// May add blank keys or delete existing keys
		/*! \remarks Sets the number of keys allocated. This may add blank
		keys or delete existing keys. It is more efficient to set a large
		number of keys using this method and then calling <b>SetKey()</b> to
		store the values rather than calling <b>AppendKey()</b> over and over
		for each key.
		\par Parameters:
		<b>int n</b>\n\n
		The new number of keys. */
		virtual void SetNumKeys(int n)=0;
		
		// Fill in 'key' with the ith key
		/*! \remarks Retrieves the 'i-th' key and stores the result in key.
		\par Parameters:
		<b>int i</b>\n\n
		The index of the key to retrieve.\n\n
		<b>IKey *key</b>\n\n
		Storage for the key data. */
		virtual void GetKey(int i,IKey *key)=0;
		
		// Set the ith key
		/*! \remarks Sets the 'i-th' key. The 'i-th' key must exist.
		\par Parameters:
		<b>int i</b>\n\n
		The index of the key to set.\n\n
		<b>IKey *key</b>\n\n
		Pointer to the key data. */
		virtual void SetKey(int i,IKey *key)=0;

		// Append a new key onto the end. Note that the
		// key list will ultimately be sorted by time. Returns
		// the key's index.
		/*! \remarks This method will append a new key onto the end of the key
		list. Note that the key list will ultimately be sorted by time.
		\par Parameters:
		<b>IKey *key</b>\n\n
		Pointer to the key data to append.
		\return  The key's index. */
		virtual int AppendKey(IKey *key)=0;

		// If any changes are made that would require the keys to be sorted
		// this method should be called.
		/*! \remarks This method should be called if any changes are made that
		would require the keys to be sorted. The keys are stored in order by
		TimeValue. */
		virtual void SortKeys()=0;		

		// Access track flags
		/*! \remarks Retrieves the track flags.
		\return  One or more of the following values:\n\n
		<b>TFLAG_CURVESEL</b>\n\n
		Determines if the curve is selected in the track view in the function
		curve editor.\n\n
		<b>TFLAG_RANGE_UNLOCKED</b>\n\n
		Determines if the range is locked to the first key and the last key. If
		a user goes into Position Ranges mode and moves the range bar, the
		range becomes unlocked.\n\n
		<b>TFLAG_LOOPEDIN</b>\n\n
		This is set if the in out of range type is set to loop.\n\n
		<b>TFLAG_LOOPEDOUT</b>\n\n
		This is set if the out of range type is set to loop.\n\n
		<b>TFLAG_COLOR</b>\n\n
		Set for Bezier Point3 controllers that are color controllers.\n\n
		<b>TFLAG_HSV</b>\n\n
		Set for color controls that interpolate in HSV rather than RGB. */
		virtual DWORD &GetTrackFlags()=0;

		// Specify the max size of a key in bytes. Just need to implement if 
		// size of IKey is greater than this default value.
		/*! \remarks Retrieves the maximim size of a key in bytes. Implement
		only if  size of IKey is greater than the default value.<br>
		\return  128 (default), or the value you specify in your
		implementation. \n\n
		  */
		virtual int GetKeySize() {return 128;}
	};

//! Extend IKeyControl to include interfaces to IKeyProperty.
//! Keys of a key controller may have extra properties that does not affect
//! the interpolated value. These properties are accessed through interface
//! IKeyProperty. IKeyControl2 provides interface to the Key Property of
//! individual keys.\n
//! The interface ID is I_KEYCONTROL2 defined in AnimatableInterfaceIDs.h.
#pragma warning(push)
#pragma warning(disable:4100)
class IKeyControl2 : public IKeyControl {
public:
	//! Whether the key of this key controller has property of ID id.
	//! \param[in] id - the property ID being queried.
	//! \return true if it has the property of ID id.
	virtual bool DoesKeyPropExist(IKeyProperty::PropID id) { return false;}

	//! Get the property of the i-th key.
	//! Use this method to get the property of the i-th key.
	//! \param[in] id - the property ID.
	//! \param[in] i - the index of the key
	//! \return a pointer to the key property object. It may return null\n
	//! if the property of the respected ID does not exist at the key\n
	//! of this key frame controller. To get the actual attributes of the\n
	//! properties, cast the returned pointer to the appropriate property
	//! interface.\n
	//! The returned pointer must be deleted after use.
	virtual const IKeyProperty* GetKeyProp(IKeyProperty::PropID id, int i) { return NULL; }

	//! Set the property to the i-th key.
	//! \param[in] id - the property ID.
	//! \param[in] i the index for the key at which to set.
	//! \param[in] prop - the property object whose values are used for set-value.\n
	//! The implementation will cast it to appropriate property interface\n
	//! and call the get methods to extract values. The values will be\n
	//! checked against the expected range. An attribute of the key property\n
	//! won't be affected if the value of that particular attribute is out of range.
	//! \return true if set the key property has actually affected. It can\n
	//! be false if (a) this key controller does not have the property, or\n
	//! (b) the values in prop are out of expected ranges.
	virtual bool SetKeyProp(IKeyProperty::PropID id, int i, const IKeyProperty& prop) { return false; }
};

class AnyKey: public MaxHeapOperators
{
public:
	Tab<char> data;
	AnyKey(int size = 128) { data.SetCount(size); } // 128 is default from IKeyControl::GetKeySize()
	void SetSize(int size) { data.SetCount(size); }
	operator IKey*() { return (IKey*)data.Addr(0); }
};


// ------- example:
//	IKeyControl* ki = GetKeyControlInterface(controller);
//	if (ki != NULL)
//	{
//		if (key_index >= ki->GetNumKeys())
//			throw RuntimeError (MaxSDK::GetResourceStringAsMSTR(IDS_KEY_NO_LONGER_EXISTS_IN_CONTROLLER), controller);
//		AnyKey ak(ki->GetKeySize()); IKey* k = ak;
//		ki->GetKey(key_index, k);


//--------------------------------------------------------------
// The following interface is an FP interface to flag TFLAG_TCBQUAT_NOWINDUP
// of IKeyControl::GetTrackFlags(). Specifically,
//   IRotWindup::GetRotWindup() ==
//		!(IKeyControl::GetTrackFlags()&TFLAG_TCBQUAT_NOWINDUP)
// However, IRotWindup::SetRotWindup() will notify dependents and handle
// undo/redo, etc.
// This interface is only available to TCB rotation controller (of class id,
// TCBINTERP_ROTATION_CLASS_ID).
// R4.5 and later only.
//
#define ROTWINDUP_INTERFACE Interface_ID(0x13a3032c, 0x381345ca)
class IRotWindup : public FPMixinInterface {
public:
	static IRotWindup* GetIRotWindup(Animatable& a) {
		return static_cast<IRotWindup*>(a.GetInterface(ROTWINDUP_INTERFACE));}

	FPInterfaceDesc* GetDesc() { return GetDescByID(ROTWINDUP_INTERFACE); }

	virtual bool GetRotWindup() const =0;
	virtual void SetRotWindup(bool) =0;

	enum FuncID {
		kRotWindupGet, kRotWindupSet
	};

BEGIN_FUNCTION_MAP
	PROP_FNS(kRotWindupGet, GetRotWindup, kRotWindupSet, SetRotWindup, TYPE_bool)
END_FUNCTION_MAP
};

// Access to default key parameters
CoreExport void SetBezierDefaultTangentType(int in, int out);
CoreExport void GetBezierDefaultTangentType(int &in, int &out);

CoreExport void SetTCBDefaultParams(float t, float c, float b, float easeIn, float easeOut);
CoreExport void GetTCBDefaultParams(float &t, float &c, float &b, float &easeIn, float &easeOut);

// An interface for supporting mutiple tangent adjustment.  
// If the user adjusts a tangent from a different track this tells the controller what angle to apply to its selected keys
// The first track hit is called to adjust its own handles and return the difference in angle and the 
// percentage difference in length.  All other tracks use this info to adjust their own handles.
// R4.5 and later only
class IAdjustMultipleTangents: public MaxHeapOperators {
	public:
		virtual void AdjustInitialTangents(TrackHitRecord hit,ParamDimensionBase *dim,Rect& rcGraph,
							float tzoom,int tscroll,float vzoom,int vscroll,int dx,int dy,DWORD flags, 
							float &angle, float &length)=0;  // the angle and percentage of length offsets are retrieved here
		virtual void AdjustSecondaryTangents(DWORD hitFlags,ParamDimensionBase *dim,Rect& rcGraph,
							float tzoom,int tscroll,float vzoom,int vscroll,float angle, float length, DWORD flags)=0;

	};

// An interface for computing and gathering weighted key info
// If a keyframe controller wants to support soft selections, they should return an instance of this class when 
// GetInterface(I_SOFTSELECT) is called.  In order to limit the amount of data held by keyframe controllers 
// the controller should build a weight table when ComputeWeights is called, and delete the weight table when ReleaseWeights()
// is called.  The weight table is only valid while in Soft Selection mode.  The controller should keep track of how many times
// GetInterface is called and only ReleaseWeights when the last client calls it.
// R4.5 and later only
class ISoftSelect: public MaxHeapOperators {
	public:
		virtual void ComputeWeights(TimeValue range, float falloff)=0;
		virtual float GetWeight(int i)=0;
		virtual void ReleaseWeights()=0;
	};

//-----------------------------------------------------------
// A plug-in can register itself to read a particular APP_DATA 
// chunk when a 3DS file is loaded. If a chunk is encountered
// that matches a registered plug-in, that plug-in will be
// asked to create an instance of itself based on the contents
// of the APP_DATA chunk.

class TriObject;

/*! \sa  Class TriObject.\n\n
\par Description:
3D Studio DOS allowed developers to store APP_DATA with objects and nodes in
the scene. When the 3DS DOS file is imported into 3ds Max, and no plug-in has
registered to convert it, then it is just hung off the object (or INode in the
case of KXP app data).\n\n
A 3ds Max plug-in can register itself to read a particular APP_DATA chunk when
a 3DS DOS file is loaded. If a chunk is encountered that matches a registered
plug-in, that plug-in will be asked to create an instance of itself based on
the contents of the APP_DATA chunk. The plug-in callback is given an
opportunity to read the chunk and create an object other than a
<b>TriObject</b> based on the contents of the chunk and the original
object.\n\n
A plug-in that wants to process app data registers a new class derived from
this class by calling <b>void
RegisterObjectAppDataReader(ObjectDataReaderCallback *cb);</b>\n\n
The system then maintains a list of these <b>ObjectDataReaderCallbacks</b>.
 */
class ObjectDataReaderCallback: public MaxHeapOperators {
	public:
		// Chunk name
		/*! \remarks Returns the name that identifies the app data chunk. When the 3DS
		Import plug-in is loading objects, it will look for app data. For each
		app data chunk that the object has, it will go through the list of
		registered callbacks and call this method looking for a name match.
		When it does find a match it will call <b>ReadData()</b> on the
		callback that matched. */
		virtual char *DataName()=0;

		// Create an instance of an object based on the data and the original mesh object
		/*! \remarks This method is called to read the app data and create an instance of an
		object based on the data and the original mesh object. For example, the
		3D Surfer plug-in from 3DS DOS creates a patch object based on the app
		data parameters and returns a pointer to it.
		\par Parameters:
		<b>TriObject *obj</b>\n\n
		The original mesh object the app data was assigned to. If no callback
		was registered, this would be the object that would get created.\n\n
		<b>void *data</b>\n\n
		Points to the particular app data chunk handled by the registered
		callback.\n\n
		<b>DWORD len</b>\n\n
		The length of the app data chunk.
		\return  The Object created by the plug-in to hold the appdata. This
		would be the object created to take the place of the TriObject. For
		example, consider a 3DS object that had appdata embedded in it which
		represented the patch object from which the mesh was created (like 3D
		Surfer for example). This method would take that data and created a
		patch object so that the user could work with the object as a patch
		object in 3ds Max instead of a tri object. */
		virtual Object *ReadData(TriObject *obj, void *data, DWORD len)=0;

		/*! \remarks This method is called to delete this callback object. When the user
		exits 3ds Max, this method is called on each of the registered
		callbacks. So if the callback was allocated dynamically, it could free
		itself in this implementation. */
		virtual void DeleteThis()=0;
	};
 
/*! \remarks Implemented by the System.\n\n
This method allows a plug-in to register a callback that will be called when
reading a 3DS file that has a particular appdata chunk. The plug-in callback is
given an opportunity to read the chunk and create an object other than a
TriObject based on the contents of the chunk and the original object.
\par Parameters:
<b>ObjectDataReaderCallback *cb</b>\n\n
The callback to read the chunk and create an object. */
CoreExport void RegisterObjectAppDataReader(ObjectDataReaderCallback *cb);
 
/*! \remarks This function is used internally by the 3DS Import plug-in.\n\n
Note the following about 3DS App Data. If app data is encountered and no
plug-in has registered to convert it, then it is just hung off the object (or
INode in the case of KXP app data). For object app data, TriObject's super
class and class ID are used to identify the chunk and the sub ID is set to 0.
For node app data, INode's super class and class ID are used to identify the
chunk and the sub ID is set to 0.\n\n
This single 3ds Max app data chunk will contain the entire 3DS app data chunk,
which may have sub chunks (see the IPAS SDK). The following routines will aid
in parsing 3DS app data. */
CoreExport Object *ObjectFromAppData(TriObject *obj, char *name, void *data, DWORD len);


// Note about 3DS App Data:
// If app data is encountered and no plug-in has registered to
// convert it, then it is just hung off the object (or INode in
// the case of KXP app data).
// For object app data, TriObject's super class and class ID are used
// to identify the chunk and the sub ID is set to 0.
// For node app data, INode's super class and class ID are used
// to identify the chunk and the sub ID is set to 0.
//
// This single MAX app data chunk will contain the entire
// 3DS app data chunk, which may have sub chunks (see IPAS SDK).
// The following routines will aid in parsing 3DS app data.

// Get the ID string out of an XDATA_ENTRY chunk and null terminates it
/*! \remarks This function gets the ID string out of an <b>XDATA_ENTRY</b>
chunk and null terminates it.
\par Parameters:
<b>char *chunk</b>\n\n
A pointer to the chunk.\n\n
<b>char *idstring</b>\n\n
The ID string. */
CoreExport void GetIDStr(char *chunk, char *idstring);

// Returns the offset into 'appd' of the specified chunk 
// or -1 if it is not found
/*! \remarks Returns the offset into <b>appd</b> of the specified chunk or -1
if it is not found.
\par Parameters:
<b>void *appd</b>\n\n
The is the entire app data chunk containing all the sub-chunks.\n\n
<b>DWORD len</b>\n\n
This is the length of the entire app data chunk.\n\n
<b>char *idstring</b>\n\n
The ID of the chunk to find.
\return  The offset into <b>appd</b> of the specified chunk or -1 if it is not
found. */
CoreExport int FindAppDataChunk(void *appd, DWORD len, char *idstring);

// Similar to Find, but actually returns a pointer to the chunk
// or NULL if it is not found
/*! \remarks This function is similar to <b>FindAppDataChunk()</b> above, but
returns a pointer to the chunk or NULL if it is not found.
\par Parameters:
<b>void *appd</b>\n\n
The is the entire app data chunk containing all the sub-chunks.\n\n
<b>DWORD len</b>\n\n
This is the length of the entire app data chunk.\n\n
<b>char *idstring</b>\n\n
The ID of the chunk to find.
\return  A pointer to the chunk or NULL if it is not found. */
CoreExport void *GetAppDataChunk(void *appd, DWORD len, char *idstring);

// Adds the chunk to the appdata chunk, preserving existing chunks.
// 'chunk' should point to the new chunk header followed by its data.
/*! \remarks This function adds the chunk to the appdata chunk, preserving
existing chunks. <b>chunk</b> should point to the new chunk header followed by
its data.
\par Parameters:
<b>void **pappd</b>\n\n
The is the entire app data chunk containing all the sub-chunks.\n\n
<b>DWORD \&len</b>\n\n
This is the length of the entire app data chunk.\n\n
<b>void *chunk</b>\n\n
A pointer to the new chunk header.
\return  Nonzero if the chunk was added; otherwise zero. */
CoreExport int SetAppDataChunk(void **pappd, DWORD &len, void *chunk);

// Deletes a chunk from the appdata while preserving other chunks.
/*! \remarks Deletes a chunk from the appdata while preserving other chunks.
\par Parameters:
<b>void **pappd</b>\n\n
The is the entire app data chunk containing all the sub-chunks.\n\n
<b>DWORD \&len</b>\n\n
This is the length of the entire app data chunk.\n\n
<b>char *idstring</b>\n\n
The ID of the chunk to delete.
\return  Nonzero if the chunk was deleted; otherwise zero. */
CoreExport int DeleteAppDataChunk(void **pappd, DWORD &len, char *idstring);

// Known sub chunks inside an appdata chunk
#define XDATA_ENTRY		0x8001
#define XDATA_APPNAME	0x8002



//---------------------------------------------------------
// Interface into MAX's default WAV sound object
// use the Interface method GetSoundObject() to get a pointer
// to the current sound object and then use the
// GetWaveSoundInterface() on the result to see if it supports
// this interface.

/*! SeeAlso: Class Interface.\n\n
\par Description:
This class provides an interface into 3ds Max's default WAV sound object. Use
the <b>Interface</b> method <b>GetSoundObject()</b> to get a pointer to the
current sound object and then use the macro <b>GetWaveSoundInterface()</b> on
the result to see if it supports this interface. See the sample code below:\n\n
<b>// Retrieve the current sound object</b>\n\n
<b>SoundObj *sound = ip-\>GetSoundObject();</b>\n\n
<b>// Attempt to get a wave interface</b>\n\n
<b>IWaveSound *iWave = GetWaveSoundInterface(sound);</b>\n\n
<b>if (iWave) {</b>\n\n
<b>iWave-\>SetSoundFileName(_M("LedZep.wav"))</b>\n\n
<b>}</b>\n\n
All methods of this class are implemented by the system.  */
class IWaveSound: public MaxHeapOperators {
	public:
		// Retreives the name of the current sound file asset
		/*! \remarks Returns the name of the current sound file asset. */
		virtual const MaxSDK::AssetManagement::AssetUser& GetSoundFile()=0;

		// Sets the sound file asset. This will cause the WAV to
		// be loaded into the tack view. Returns FALSE if
		// the file can't be opened or no wave track exist.
		/*! \remarks Sets the sound file asset. This will cause the .WAV to be
		loaded into the track view.
		\par Parameters:
		<b>MSTR name</b>\n\n
		The name of the sound file asset to set.
		\return  FALSE if the file can't be opened or no wave track exists;
		otherwise TRUE. */
		virtual BOOL SetSoundFile(const MaxSDK::AssetManagement::AssetUser& name)=0;

		// Set the time offset for the wave
		/*! \remarks Sets the time offset for the wave file.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The start time. */
		virtual void SetStartTime(TimeValue t)=0;

		// Get the time offset for the wave
		/*! \remarks Returns the start time offset for the wave. */
		virtual TimeValue GetStartTime()=0;		
		/*! \remarks Returns the end time for the wave. */
		virtual TimeValue GetEndTime()=0;
	};


//-----------------------------------------------------------
//
// Access to the boolean object's parameters. Given a pointer to
// an object whose class ID is Class_ID(BOOLOBJ_CLASS_ID,0) or
// NEWBOOL_CLASS_ID, you can cast that pointer to the following
// class.  Note that some options do not work in the old Boolean
// (BOOLOBJ_CLASS_ID), and there is no Optimize parameter in
// the new Boolean.
//

#define BOOLOP_UNION			0
#define BOOLOP_INTERSECTION		1
#define BOOLOP_SUB_AB			2
#define BOOLOP_SUB_BA			3
#define BOOLOP_CUT				4

#define BOOLOP_CUT_REFINE  0
#define BOOLOP_CUT_SEPARATE  1
#define BOOLOP_CUT_REMOVE_IN  2
#define BOOLOP_CUT_REMOVE_OUT  3

#define BOOLUPDATE_ALWAYS		0
#define BOOLUPDATE_SELECTED		1
#define BOOLUPDATE_RENDER		2
#define BOOLUPDATE_MANUAL		3

#define BOOL_ADDOP_REFERENCE 0
#define BOOL_ADDOP_INSTANCE 1
#define BOOL_ADDOP_COPY 2
#define BOOL_ADDOP_MOVE 3

#define BOOL_MAT_NO_MODIFY 0
#define BOOL_MAT_IDTOMAT 1
#define BOOL_MAT_MATTOID 2
#define BOOL_MAT_DISCARD_ORIG 3
#define BOOL_MAT_DISCARD_NEW 4

/*! \sa  Class GeomObject.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class provides access to the boolean object's parameters. Given a pointer
to an object whose ClassID is <b>BOOLOBJ_CLASS_ID</b>, you can cast that
pointer to this class and use the methods to access the parameters. Note: In
3ds Max 3.0 and later the new boolean object is used and it has its own
ClassID. Therefore where you previously used
<b>Class_ID(BOOLOBJ_CLASS_ID,0)</b> you should now use <b>NEWBOOL_CLASS_ID</b>
(<b>#define NEWBOOL_CLASS_ID Class_ID(0x51db4f2f,0x1c596b1a)</b>).\n\n
The boolean object has four references. 2 references to the operand objects and
2 references to transform controllers providing a transformation matrix for the
2 operands. The following values may be used to access the boolean object's
references:\n\n
<b>BOOLREF_OBJECT1</b>\n\n
<b>BOOLREF_OBJECT2</b>\n\n
<b>BOOLREF_CONT1</b>\n\n
<b>BOOLREF_CONT2</b>\n\n
All methods of this class are implemented by the system.  */
class IBoolObject : public GeomObject {
public:
	/*! \remarks Returns TRUE if the specified operand is selected; otherwise
	FALSE.
	\par Parameters:
	<b>int which</b>\n\n
	Specifies which operand to check.\n\n
	<b>0</b> for operand A, <b>1</b> for operand B. */
	virtual BOOL GetOperandSel(int which)=0;
	/*! \remarks Sets the selected state of the specified operand.
	\par Parameters:
	<b>int which</b>\n\n
	Specifies which operand to select.\n\n
	<b>0</b> for operand A, <b>1</b> for operand B.\n\n
	<b>BOOL sel</b>\n\n
	TRUE to select it; FALSE to deselect it. */
	virtual void SetOperandSel(int which,BOOL sel)=0;
	/*! \remarks Returns the boolean operation setting.
	\return  One of the following values:\n\n
	<b>BOOLOP_UNION</b>\n\n
	<b>BOOLOP_INTERSECTION</b>\n\n
	<b>BOOLOP_SUB_AB</b>\n\n
	<b>BOOLOP_SUB_BA</b>\n\n
	<b>BOOLOP_CUT</b> -- This option is available in release 3.0 and later
	only. */
	virtual int GetBoolOp()=0;
	/*! \remarks Sets the boolean operation.
	\par Parameters:
	<b>int op</b>\n\n
	One of the following values:\n\n
	<b>BOOLOP_UNION</b>\n\n
	<b>BOOLOP_INTERSECTION</b>\n\n
	<b>BOOLOP_SUB_AB</b>\n\n
	<b>BOOLOP_SUB_BA</b>\n\n
	<b>BOOLOP_CUT</b> -- This option is available in release 3.0 and later
	only. */
	virtual void SetBoolOp(int op)=0;
	/*! \remarks	Returns the type if the 'Cut' operation is choosen. One of the following
	values:\n\n
	<b>BOOLOP_CUT_REFINE</b>\n\n
	<b>BOOLOP_CUT_SEPARATE</b>\n\n
	<b>BOOLOP_CUT_REMOVE_IN</b>\n\n
	<b>BOOLOP_CUT_REMOVE_OUT</b> */
	virtual int GetBoolCutType()=0;
	/*! \remarks	Sets the type for the 'Cut' operation. One of the following values:
	\par Parameters:
	<b>int ct</b>\n\n
	One of the following values:\n\n
	<b>BOOLOP_CUT_REFINE</b>\n\n
	<b>BOOLOP_CUT_SEPARATE</b>\n\n
	<b>BOOLOP_CUT_REMOVE_IN</b>\n\n
	<b>BOOLOP_CUT_REMOVE_OUT</b> */
	virtual void SetBoolCutType(int ct)=0;
	/*! \remarks Returns the display result setting. If TRUE the result is
	display; otherwise the operands are displayed. */
	virtual BOOL GetDisplayResult()=0;
	/*! \remarks Sets the display result state.
	\par Parameters:
	<b>BOOL onOff</b>\n\n
	TRUE to show the result; FALSE to show the operands. */
	virtual void SetDisplayResult(BOOL onOff)=0;
	/*! \remarks Returns the show hidden operands state. TRUE if on; FALSE if
	off. */
	virtual BOOL GetShowHiddenOps()=0;
	/*! \remarks Sets the state of the show hidden operands setting.
	\par Parameters:
	<b>BOOL onOff</b>\n\n
	TRUE for on; FALSE for off. */
	virtual void SetShowHiddenOps(BOOL onOff)=0;
	/*! \remarks Returns the state of the update mode.
	\return  One of the following values:\n\n
	<b>BOOLUPDATE_ALWAYS</b>\n\n
	<b>BOOLUPDATE_SELECTED</b>\n\n
	<b>BOOLUPDATE_RENDER</b>\n\n
	<b>BOOLUPDATE_MANUAL</b> */
	virtual int GetUpdateMode()=0;
	/*! \remarks Sets the state of the update mode.
	\par Parameters:
	<b>int mode</b>\n\n
	One of the following values:\n\n
	<b>BOOLUPDATE_ALWAYS</b>\n\n
	<b>BOOLUPDATE_SELECTED</b>\n\n
	<b>BOOLUPDATE_RENDER</b>\n\n
	<b>BOOLUPDATE_MANUAL</b> */
	virtual void SetUpdateMode(int mode)=0;
	/*! \remarks Returns the state of the optimize result setting. TRUE if on;
	FALSE if off. Note: This method does not work on 'new' boolean objects as
	there is no optimize setting for these. */
	virtual BOOL GetOptimize()=0;
	/*! \remarks Sets the state of the optimize result setting. Note: This
	method does not work on 'new' boolean objects as there is no optimize
	setting for these.
	\par Parameters:
	<b>BOOL onOff</b>\n\n
	TRUE for on; FALSE for off. */
	virtual void SetOptimize(BOOL onOff)=0;
	/*! \remarks	Sets the node used for operand A. This method adds a restore object if
	<b>theHold.Holding()</b>, but the <b>Begin()</b> and <b>Accept()</b> should
	be done in the calling routine.\n\n
	Note the following:\n\n
	- In a Boolean Object, A should be specified before B. (This allows
	material merges, etc, to work.)\n\n
	- This method adds a restore object if <b>theHold.Holding()</b>, but the
	<b>Begin()</b> and <b>Accept()</b> should be done in the calling routine.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the operand.\n\n
	<b>INode *node</b>\n\n
	Points to the operand A node. */
	virtual void SetOperandA (TimeValue t, INode *node)=0;
	/*! \remarks	Sets the node used for operand B. This method adds a restore object if
	<b>theHold.Holding()</b>, but the <b>Begin()</b> and <b>Accept()</b> should be
	done in the calling routine.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the operand.\n\n
	<b>INode *node</b>\n\n+
	Points to the operand B node.\n\n
	<b>INode *boolNode</b>\n\n
	Points to the node of the boolean object.\n\n
	<b>int addOpMethod=0</b>\n\n
	One of the following values:\n\n
	<b>BOOL_ADDOP_REFERENCE</b>\n\n
	Operand is a reference to original node.\n\n
	<b>BOOL_ADDOP_INSTANCE</b>\n\n
	Operand is an instance of the original node.\n\n
	<b>BOOL_ADDOP_COPY</b>\n\n
	Operand is a copy of the original node.\n\n
	<b>BOOL_ADDOP_MOVE</b>\n\n
	Original node should be deleted.\n\n
	<b>int matMergeMethod=0</b>\n\n
	One of the following values:\n\n
	<b>BOOL_MAT_NO_MODIFY</b>\n\n
	Combines materials without changing them or the ID's.\n\n
	<b>BOOL_MAT_IDTOMAT</b>\n\n
	Matches ID's to materials, then combines materials.\n\n
	<b>BOOL_MAT_MATTOID</b>\n\n
	Matches materials to ID's, then combines them.\n\n
	<b>BOOL_MAT_DISCARD_ORIG</b>\n\n
	Discards original material, uses new node's instead.\n\n
	<b>BOOL_MAT_DISCARD_NEW</b>\n\n
	Discards new node's material, uses original. */
	virtual void SetOperandB (TimeValue t, INode *node, INode *boolNode,
		int addOpMethod=0, int matMergeMethod=0, bool *canUndo=NULL)=0;
};

// The boolean object has five references. 2 references to the
// operand objects, 2 references to transform controllers 
// providing a transformation matrix for the 2 operands,
// and one to the parameter block.
#define BOOLREF_OBJECT1		0
#define BOOLREF_OBJECT2		1
#define BOOLREF_CONT1		2
#define BOOLREF_CONT2		3
#define BOOLREF_PBLOCK     4

//-------------------------------------------------------------
// Access to path controller's parameters.
//
//Function Publishing interface Added by Ambarish Goswami (8-26-2000)
//***********************************************************

class IPathPosition;

#define PATH_CONSTRAINT_INTERFACE Interface_ID(0x79d15f78, 0x1f901f8e)
#define GetIPathConstInterface(cd) \
		(IPathPosition*)(cd)->GetInterface(PATH_CONSTRAINT_INTERFACE)


/*! \sa  Class Control, Class INode.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class represents the interface to the Path Position Controller. You can
obtain a pointer to the path position controller interface using;
<b>GetIPathConstInterface(cd)</b>. This macro will return\n\n
<b>(IPathPosition*)(CD)-\>GetFPInterface(PATH_CONSTRAINT_INTERFACE).</b>\n\n
<b>PATHPOS_PATH_REF</b> may be used to access the position constraint
controller's references and <b>PATHPOS_PBLOCK_REF</b> to reference the
parameter block.\n\n
All methods of this class are Implemented by the System.  */
class IPathPosition : public Control, public FPMixinInterface  
{
	public:

		enum {	get_num_targets,		get_node,			get_target_weight,		
				set_target_weight,		append_target,		delete_target,};

		
		// Function Map for Function Publish System 
		//***********************************
		BEGIN_FUNCTION_MAP
		FN_0(get_num_targets,		TYPE_INT,  GetNumTargets);
		FN_1(get_node,				TYPE_INODE, GetNode, TYPE_INDEX);
		FN_1(get_target_weight,		TYPE_FLOAT, GetTargetWeight, TYPE_INDEX);
		FN_2(set_target_weight,		TYPE_BOOL, SetTargetWeight, TYPE_INDEX, TYPE_FLOAT);
		FN_2(append_target,			TYPE_BOOL, AppendTarget, TYPE_INODE, TYPE_FLOAT);
		FN_1(delete_target,			TYPE_BOOL, DeleteTarget, TYPE_INDEX);
		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    // <-- must implement 

		//End of Function Publishing system code 
		//***********************************

		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		Returns the number of nodes in the path list. */
		virtual int GetNumTargets()=0;
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		Gets one of the path nodes that the path controller follows, specified
		by <b>targetNumber</b>.
		\par Parameters:
		<b>int targetNumber</b>\n\n
		The node number in the path list to be obtained. */
		virtual INode* GetNode(int targetNumber)=0;
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		Gets the weight of one of the path nodes that the path controller
		follows, specified by <b>targetNumber</b>, and time t. If the
		<b>targetNumber</b> is not relevant then 0.0f is returned.
		\par Parameters:
		<b>int targetNumber</b>\n\n
		The node number in the path list whose weight is to be obtained. */
		virtual	float GetTargetWeight(int targetNumber)=0;
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		Sets the weight of one of the path nodes that the path controller
		follows, specified by <b>targetNumber</b>.
		\par Parameters:
		<b>int targetNumber</b>\n\n
		The node number in the path list whose weight is to be set.\n\n
		<b>float weight</b>\n\n
		The weight to assign.
		\return  TRUE if there is more than one path in the list and you are
		trying to set weight, FALSE otherwise. */
		virtual BOOL SetTargetWeight(int targetNumber, float weight)=0;
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		Appends the current path list by one and appends the current weight
		list by one.
		\par Parameters:
		<b>INode *target</b>\n\n
		The node that is to be appended to the current path list.\n\n
		<b>float weight=50.0</b>\n\n
		The weight to be assigned to the newly appended path. */
		virtual BOOL AppendTarget(INode *target, float weight=50.0)=0;
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method allows you to delete a specified target.
		\par Parameters:
		<b>int selection</b>\n\n
		The node number in the orientation target list to delete.
		\return  TRUE if successful, otherwise FALSE. */
		virtual BOOL DeleteTarget(int selection)=0;	

		/*! \remarks This method allows you to set the follow flag.
		\par Parameters:
		<b>BOOL f</b>\n\n
		TRUE for on, FALSE for off.\n\n
		  */
		virtual void SetFollow(BOOL f)=0;
		/*! \remarks This method returns the state of the follow flag. TRUE if
		on; FALSE if off. */
		virtual BOOL GetFollow()=0;
		/*! \remarks Sets the bank amount parameter.\n\n
		Bank and tracking are scaled in the UI.\n\n
		The bank values are scaled in the user interface. The following macros
		may be used to convert to and from the UI values.\n\n
		<b>#define BANKSCALE 100.0f</b>\n\n
		<b>#define FromBankUI(a) ((a)*BANKSCALE)</b>\n\n
		<b>#define ToBankUI(a)  ((a)/BANKSCALE)</b>
		\par Parameters:
		<b>float a</b>\n\n
		The bank amount. */
		virtual void SetBankAmount(float a)=0;
		/*! \remarks Returns the bank amount setting. See the remarks in
		<b>SetBankAmount()</b> above. */
		virtual float GetBankAmount()=0;
		/*! \remarks Sets the bank parameter to on or off.
		\par Parameters:
		<b>BOOL b</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetBank(BOOL b)=0;
		/*! \remarks Returns the on/off state of the bank parameter. TRUE if
		on; FALSE if off. */
		virtual BOOL GetBank()=0;
		/*! \remarks Sets the smoothness parameter.\n\n
		The smoothing (tracking) values are scaled in the user interface. The
		following macros may be used to convert to and from the UI values.\n\n
		<b>#define TRACKSCALE 0.04f</b>\n\n
		<b>#define FromTrackUI(a) ((a)*TRACKSCALE)</b>\n\n
		<b>#define ToTrackUI(a) ((a)/TRACKSCALE)</b>
		\par Parameters:
		<b>float t</b>\n\n
		The smoothness setting. */
		virtual void SetTracking(float t)=0;		// smoothness
		/*! \remarks Returns the smoothness setting. See remarks in
		<b>SetTracking()</b> above. */
		virtual float GetTracking()=0;
		/*! \remarks Sets the state of the 'Allow Upside Down' parameter.
		\par Parameters:
		<b>BOOL f</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetAllowFlip(BOOL f)=0;
		/*! \remarks Returns the state of the 'Allow Upside Down' parameter.
		\return  TRUE for on; FALSE for off. */
		virtual BOOL GetAllowFlip()=0;
		/*! \remarks Sets the state of the 'Constant Velocity' parameter.
		\par Parameters:
		<b>BOOL cv</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetConstVel(BOOL cv)=0;
		/*! \remarks Returns the state of the 'Constant Velocity' parameter.
		\return  TRUE for on; FALSE for off. */
		virtual BOOL GetConstVel()=0;
		/*! \remarks Sets the state of the 'Flip' parameter.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetFlip(BOOL onOff)=0;
		/*! \remarks Returns the state of the 'Flip' parameter.
		\return  TRUE for on; FALSE for off. */
		virtual BOOL GetFlip()=0;
		/*! \remarks Set the state of the axis parameter.
		\par Parameters:
		<b>int axis</b>\n\n
		The axis setting. One of the following values:\n\n
		<b>0</b>: X axis.\n\n
		<b>1</b>: Y axis.\n\n
		<b>2</b>: Z axis. */
		virtual void SetAxis(int axis)=0;
		/*! \remarks Returns the axis setting.
		\return  One of the following values:\n\n
		<b>0</b>: X axis.\n\n
		<b>1</b>: Y axis.\n\n
		<b>2</b>: Z axis. */
		virtual int GetAxis()=0;
		/*! \remarks This method allows you to set the state of the loop flag.
		\par Parameters:
		<b>BOOL l</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetLoop(BOOL l)=0;			// AG added
		/*! \remarks Returns the state of the loop flag.
		\return  TRUE for on; FALSE for off. */
		virtual BOOL GetLoop()=0;				// AG added
		/*! \remarks This method allows you to set the state of the
		relative/absolute flag.
		\par Parameters:
		<b>BOOL rel</b>\n\n
		TRUE to set to relative; FALSE to set to absolute. */
		virtual void SetRelative(BOOL rel)=0;	// AG added
		/*! \remarks Returns the state of the relative/absolute flag.
		\return  TRUE if relative is on; FALSE is off (i.e. absolute). */
		virtual BOOL GetRelative()=0;	

	};

// block IDs
enum { path_params, path_joint_params };

// path_params param IDs
enum {	path_percent,			path_path,			path_follow,  
		path_bank,				path_bank_amount,	path_smoothness, 
		path_allow_upsidedown,	path_constant_vel,	path_axis, 
		path_axis_flip,			path_path_weight,	path_path_list, 
		path_loop,				path_relative,};

// Bank and tracking are scaled in the UI.
#define BANKSCALE 100.0f
#define FromBankUI(a) ((a)*BANKSCALE)
#define ToBankUI(a)	  ((a)/BANKSCALE)

#define TRACKSCALE 0.04f
#define FromTrackUI(a) ((a)*TRACKSCALE)
#define ToTrackUI(a)   ((a)/TRACKSCALE)

// percent controller, path node and paramblock2 refs
// #define PATHPOS_PERCENT_REF	0    // obsolete in Ed. 2, percent is an animatable in the ParamBlock
#define PATHPOS_PATH_REF	1
#define PATHPOS_PBLOCK_REF	2


//-------------------------------------------------------------
// // Access to Position Constraint controller's parameters.
//	Ambarish Goswami implemented April, 2000

//Function Publishing interface Added by Adam Felt (5-16-00)
//***********************************************************

class IPosConstPosition;

#define POS_CONSTRAINT_INTERFACE Interface_ID(0x32040779, 0x794a1278)
#define GetIPosConstInterface(cd) \
		(IPosConstPosition*)(cd)->GetInterface(POS_CONSTRAINT_INTERFACE)

//***********************************************************

/*! \sa  Class Control, Class FPMixinInterface.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the interface to the Position Constraint. You can obtain
a pointer to the position constraint interface using;
<b>GetIPosConstInterface(cd)</b>. This macro will return\n\n
<b>(IPosConstPosition*)(CD)-\>GetFPInterface(POS_CONSTRAINT_INTERFACE).</b>\n\n
<b>POSPOS_PBLOCK_REF</b> may be used to access the position constraint
controller's references:\n\n
All methods of this class are Implemented by the System.  */
class IPosConstPosition : public Control, public FPMixinInterface {
	public:

		enum {	get_num_targets,		get_node,			get_target_weight,		
				set_target_weight,		append_target,		delete_target,};
		
		// Function Map for Function Publish System 
		//***********************************
		BEGIN_FUNCTION_MAP
		FN_0(get_num_targets,				TYPE_INT, GetNumTargets);
		FN_1(get_node,						TYPE_INODE, GetNode, TYPE_INDEX);
		FN_1(get_target_weight,				TYPE_FLOAT, GetTargetWeight, TYPE_INDEX);
		FN_2(set_target_weight,				TYPE_BOOL, SetTargetWeight, TYPE_INDEX, TYPE_FLOAT);
		FN_2(append_target,					TYPE_BOOL, AppendTarget, TYPE_INODE, TYPE_FLOAT);
		FN_1(delete_target,					TYPE_BOOL, DeleteTarget, TYPE_INDEX);
		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    // <-- must implement 
		//End of Function Publishing system code 
		//***********************************

		/*! \remarks Returns the number of target nodes in the position target
		list. */
		virtual int GetNumTargets()=0;
		/*! \remarks Gets one of the position nodes that the position
		constraint controller targets, specified by <b>targetNumber</b>.
		\par Parameters:
		<b>int targetNumber</b>\n\n
		The node number in the position target list to be obtained. */
		virtual INode* GetNode(int targetNumber)=0;
		/*! \remarks Gets the weight of one of the position nodes that the
		position constraint controller targets, specified by
		<b>targetNumber</b>.
		\par Parameters:
		<b>int targetNumber</b>\n\n
		The node number in the position target list to set.
		\return  Returns the position target weight if the <b>targetNumber</b>
		is relevant, 0.0f otherwise. */
		virtual	float GetTargetWeight(int targetNumber)=0;
		/*! \remarks Sets the weight of one of the position nodes that the
		position constraint controller follows, specified by
		<b>targetNumber</b>.
		\par Parameters:
		<b>int targetNumber</b>\n\n
		The node number in the position target list whose weight is to be
		set.\n\n
		<b>float weight</b>\n\n
		The weight to set.
		\return  TRUE if there is more than one position target in the list and
		you are trying to set weight, FALSE otherwise. */
		virtual BOOL SetTargetWeight(int targetNumber, float weight)=0;
		/*! \remarks Appends the current position target list by one and
		appends.
		\par Parameters:
		<b>INode *target</b>\n\n
		The node that is to be appended to the current position target
		list.\n\n
		<b>float weight=50.0</b>\n\n
		This is the weight that is to be assigned to the newly appended
		position target. The default weight is 50.0.
		\return  TRUE if the target was appended, otherwise FALSE.\n\n
		  */
		virtual BOOL AppendTarget(INode *target, float weight=50.0)=0;
		/*! \remarks This method allows you to delete a specified target.
		\par Parameters:
		<b>int selection</b>\n\n
		The node number in the position target list to delete.
		\return  TRUE if successful, otherwise FALSE. */
		virtual BOOL DeleteTarget(int selection)=0;	
	};

#define POSPOS_PBLOCK_REF	0


//-------------------------------------------------------------
// // Access to Orientation Constraint controller's parameters.
//	Ambarish Goswami implemented May, 2000

//Function Publishing interface Added by Ambarish Goswami 6/18/2000 adapted from Adam Felt (5-16-00)
//**************************************************************************************************

class IOrientConstRotation;

#define ORIENT_CONSTRAINT_INTERFACE Interface_ID(0x71e2231b, 0x72522ab2)
#define GetIOrientConstInterface(cd) \
		(IOrientConstRotation*)(cd)->GetInterface(ORIENT_CONSTRAINT_INTERFACE)

//***********************************************************

/*! \sa  Class Control, Class FPMixinInterface.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
The Orientation Constraint matches the orientation of an object to its target
without affecting its position. Multiple weighted targets are supported. This
class is an interface to the parameters of this controllers. You can obtain a
pointer to the list control interface using;
<b>GetIOrientConstInterface(cd)</b>. This macro will return
<b>(IOrientConstRotation*)(CD)-\>GetFPInterface(ORIENT_CONSTRAINT_INTERFACE).</b>\n\n
<b>ORIENT_ROT_PBLOCK_REF</b> may be used to access the orientation constraint
controller's references:\n\n
All methods of this class are Implemented by the System.  */
class IOrientConstRotation : public Control, public FPMixinInterface {
	public:

		enum {	get_num_targets,		get_node,			get_target_weight,		
				set_target_weight,		append_target,		delete_target,};

		
		// Function Map for Function Publish System 
		//***********************************
		BEGIN_FUNCTION_MAP
		FN_0(get_num_targets,		TYPE_INT,  GetNumTargets);
		FN_1(get_node,				TYPE_INODE, GetNode, TYPE_INDEX);
		FN_1(get_target_weight,		TYPE_FLOAT, GetTargetWeight, TYPE_INDEX);
		FN_2(set_target_weight,		TYPE_BOOL, SetTargetWeight, TYPE_INDEX, TYPE_FLOAT);
		FN_2(append_target,			TYPE_BOOL, AppendTarget, TYPE_INODE, TYPE_FLOAT);
		FN_1(delete_target,			TYPE_BOOL, DeleteTarget, TYPE_INDEX);
		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    // <-- must implement 
		//End of Function Publishing system code 
		//***********************************

		/*! \remarks Returns the number of target nodes in the orientation
		target list. */
		virtual int GetNumTargets()=0;
		/*! \remarks Gets one of the orientation nodes that the orientation
		constraint controller targets, specified by <b>targetNumber</b>.
		\par Parameters:
		<b>int targetNumber</b>\n\n
		The node number in the orientation target list to be obtained. */
		virtual INode* GetNode(int targetNumber)=0;
		/*! \remarks Gets the weight of one of the orientation nodes that the
		orientation constraint controller targets, specified by
		<b>targetNumber</b>.
		\par Parameters:
		<b>int targetNumber</b>\n\n
		The node number in the orientation target list to set.
		\return  Returns the orientation target weight if the
		<b>targetNumber</b> is relevant, 0.0f otherwise. */
		virtual	float GetTargetWeight(int targetNumber)=0;
		/*! \remarks Sets the weight of one of the orientation nodes that the
		orientation constraint controller follows, specified by
		<b>targetNumber</b>.
		\par Parameters:
		<b>int targetNumber</b>\n\n
		The node number in the orientation target list whose weight is to be
		set.\n\n
		<b>float weight</b>\n\n
		The weight to set.
		\return  TRUE if there is more than one orientation target in the list
		and you are trying to set weight, FALSE otherwise. */
		virtual BOOL SetTargetWeight(int targetNumber, float weight)=0;
		/*! \remarks Appends the current orientation target list by one and
		appends the current orient_targ_wtlist (orientation target weightlist)
		by one.
		\par Parameters:
		<b>INode *target</b>\n\n
		The node that is to be appended to the current orientation target
		list.\n\n
		<b>float weight=50.0</b>\n\n
		This is the weight that is to be assigned to the newly appended
		orientation target. The default weight is 50.0.
		\return  TRUE if the target was appended, otherwise FALSE. */
		virtual BOOL AppendTarget(INode *target, float weight=50.0)=0;
		/*! \remarks This method allows you to delete a specified target.
		\par Parameters:
		<b>int selection</b>\n\n
		The node number in the orientation target list to delete.
		\return  TRUE if successful, otherwise FALSE. */
		virtual BOOL DeleteTarget(int selection)=0;
	};

#define ORIENT_ROT_PBLOCK_REF	0



//-------------------------------------------------------------
// // Access to LookAt Constraint controller's parameters.
//	Ambarish Goswami implemented May, 2000

//Function Publishing interface Added by Ambarish Goswami 5/24/2000 adapted from Adam Felt (5-16-00)
//**************************************************************************************************


class ILookAtConstRotation;

#define LOOKAT_CONSTRAINT_INTERFACE Interface_ID(0x5dbe7ad8, 0x1d1b488b)
#define GetILookAtConstInterface(cd) \
		(ILookAtConstRotation*)(cd)->GetInterface(LOOKAT_CONSTRAINT_INTERFACE)

//***********************************************************

/*! \sa  Class Control, Class FPMixinInterface.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class is an interface to the LookAt Constraint rotation controller. You
can obtain a pointer to the list control interface using;
<b>GetILookAtConstInterface(cd)</b>. This macro will return
<b>(ILookAtConstRotation*)(CD)-\>GetFPInterface(LOOKAT_CONSTRAINT_INTERFACE).</b>\n\n
<b>LOOKAT_ROT_PBLOCK_REF</b> may be used to access the look-at controller's
references:\n\n
All methods of this class are Implemented by the System.  */
class ILookAtConstRotation : public Control , public FPMixinInterface {
	public:

		enum {	get_num_targets,		get_node,			get_target_weight,		
				set_target_weight,		append_target,		delete_target,};

		// FUNCTION_PUBLISHING
		
		// Function Map for Function Publish System 
		//***********************************
		BEGIN_FUNCTION_MAP
		FN_0(get_num_targets,		TYPE_INT,  GetNumTargets);
		FN_1(get_node,				TYPE_INODE, GetNode, TYPE_INDEX);
		FN_1(get_target_weight,		TYPE_FLOAT, GetTargetWeight, TYPE_INDEX);
		FN_2(set_target_weight,		TYPE_BOOL, SetTargetWeight, TYPE_INDEX, TYPE_FLOAT);
		FN_2(append_target,			TYPE_BOOL, AppendTarget, TYPE_INODE, TYPE_FLOAT);
		FN_1(delete_target,			TYPE_BOOL, DeleteTarget, TYPE_INDEX);

		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    // <-- must implement 
		//End of Function Publishing system code 
		//***********************************

		/*! \remarks Returns the number of nodes in the list of nodes to look
		at. */
		virtual int GetNumTargets()=0;
		virtual INode* GetNode(int targetNumber)=0;
		/*! \remarks Gets the weight of one of the Look-At nodes that the
		Look-At constraint controller targets, specified by
		<b>targetNumber</b>.
		\par Parameters:
		<b>int targetNumber</b>\n\n
		The node number in the Look-At target list whose weight is to be
		obtained. */
		virtual	float GetTargetWeight(int targetNumber)=0;
		/*! \remarks Sets the weight of one of the Look-At nodes that the Look-At
		constraint controller follows, specified by <b>targetNumber</b>.
		\par Parameters:
		<b>targetNumber</b>\n\n
		The node number in the Look-At target list whose weight is to be set.
		<b>float weight</b>\n\n
		The weight to set.\n\n
		\return  TRUE if there is more than one Look-At targets in the list and you are
		trying to set weight, FALSE otherwise. */
		virtual BOOL SetTargetWeight(int targetNumber, float weight)=0;
		/*! \remarks Appends the current Look-At target list by one and
		appends the current Look-At target weightlist by one.
		\par Parameters:
		<b>INode *target</b>\n\n
		The node that is to be appended to the current Look-At target list.\n\n
		<b>float weight=50.0</b>\n\n
		This is the weight that is to be assigned to the newly appended Look-At
		target. The default weight is 50.0.
		\return  TRUE if the target was appended, otherwise FALSE. */
		virtual BOOL AppendTarget(INode *target, float weight=50.0)=0;
		/*! \remarks This method allows you to delete a specified target.
		\par Parameters:
		<b>int selection</b>\n\n
		The zero based node number in the list of nodes the controller looks at. */
		virtual BOOL DeleteTarget(int selection)=0;


		/*! \remarks Gets the relative/absolute mode corresponding to the
		"Keep Initial Offset" checkbox in the UI. */
		virtual BOOL GetRelative()=0;
		/*! \remarks Gets the ViewLine relative/absolute mode corresponding to
		the "Keep ViewLine Length Absolute" checkbox in the UI. When Viewline
		Length is absolute, the "ViewLine Length" spinner sets the length of
		the ViewLine. A negative length implies that starting from the source
		object the line travels opposite to the direction of the target object.
		The source/target distance has no effect on the ViewLine length in this
		mode. If the "Keep ViewLine Length Absolute" checkbox is unchecked, the
		ViewLine length is determined from the spinner value, which is
		interpreted as a percentage of the source/target distance.
		\return  TRUE if the ViewLine length is absolute, FALSE otherwise. */
		virtual BOOL GetVLisAbs()=0;
		/*! \remarks Returns TRUE if the "World" checkbox is on; FALSE if off.
		*/
		virtual BOOL GetUpnodeWorld()=0;
		/*! \remarks Returns TRUE if the "selected" axis flip checkbox is on;
		FALSE if off. */
		virtual BOOL GetStoUPAxisFlip()=0;
		/*! \remarks Returns TRUE if the "source" axis flip checkbox is on;
		FALSE if off. */
		virtual BOOL GetTargetAxisFlip()=0;
		/*! \remarks Returns TRUE if the orientation flag is set, FALSE if
		off. */
		virtual BOOL Get_SetOrientation()=0;
		/*! \remarks Gets the selection corresponding to the "Select LookAt
		Axis" button in the UI. Obtains which of the source axes is required to
		coincide with the target axis.
		\return  (0) if the target axis coincides with the x axis of the source
		object. (1) if the target axis coincides with the y axis of the source
		object. (2) if the target axis coincides with the z axis of the source
		object. */
		virtual int GetTargetAxis()=0;
		/*! \remarks Gets the selection corresponding to the "Source/Upnode
		Alignment: Aligned to UpNode Axis:" radiobutton in the UI. Obtains
		which of the upnode axes is required to align with a specified source
		axis.
		\return  (0) if the upnode x axis coincides with a specified source
		object. (1) if the upnode y axis coincides with a specified source
		object. (2) if the upnode z axis coincides with a specified source
		object. */
		virtual int GetUpNodeAxis()=0;
		/*! \remarks Gets the selection corresponding to the "Source/Upnode
		Alignment: Aligned to UpNode Axis:" radiobutton in the UI. Obtains
		which of the source axes is required to align with a specified upnode
		axis.
		\return  (0) if the source x axis coincides with a specified upnode
		axis. (1) if the source y axis coincides with a specified upnode axis.
		(2) if the source z axis coincides with a specified upnode axis. */
		virtual int Get_StoUPAxis()=0;
		virtual int Get_upnode_control()=0;
		/*! \remarks This method allows you to set the "relative" flag.
		\par Parameters:
		<b>BOOL rel</b>\n\n
		TRUE to set the relative flag, otherwise FALSE. */
		virtual void SetRelative(BOOL rel)=0;
		/*! \remarks Sets the relative/absolute mode corresponding to the
		"Keep ViewLine Length Absolute" checkbox in the UI.
		\par Parameters:
		<b>BOOL rel</b>\n\n
		TRUE if "Keep ViewLine Length Absolute" is active (checked), FALSE
		otherwise. */
		virtual void SetVLisAbs(BOOL rel)=0;
		/*! \remarks This method allows you to set the "World" flag.
		\par Parameters:
		<b>BOOL uw</b>\n\n
		TRUE to set the world flag, otherwise false. */
		virtual void SetUpnodeWorld(BOOL uw)=0;
		/*! \remarks This method allows you to set the "source" flip axis
		flag.
		\par Parameters:
		<b>BOOL rel</b>\n\n
		TRUE to set the source flip axis flag, otherwise FALSE. */
		virtual void SetTargetAxisFlip(BOOL rel)=0;
		/*! \remarks This method allows you to set the "selected" axis flip
		flag.
		\par Parameters:
		<b>BOOL rel</b>\n\n
		TRUE to set the selected axis flip flag, otherwise FALSE. */
		virtual void SetStoUPAxisFlip(BOOL rel)=0;
		/*! \remarks This method allows you to set the orientation flag.
		\par Parameters:
		<b>BOOL rel</b>\n\n
		TRUE to set the orientation flag, otherwise FALSE. */
		virtual void Set_SetOrientation(BOOL rel)=0;
		/*! \remarks Resets to zero the amount of orientation offset, effected
		through the "Set Orientation" feature. */
		virtual void Set_Reset_Orientation()=0;
		/*! \remarks Sets the selection corresponding to the "Set Orientation"
		button in the UI. Specifies which of the source axes is required to
		coincide with the target axis.
		\par Parameters:
		<b>int axis</b>\n\n
		(0) if TargetAxis coincides with the X axis of the source object. (1)
		if TargetAxis coincides with the Y axis of the source object. (2) if
		TargetAxis coincides with the Z axis of the source object */
		virtual void SetTargetAxis(int axis)=0;
		/*! \remarks Sets the selection corresponding to the "Source/Upnode
		Alignment: Aligned to UpNode Axis:" radiobutton in the UI. Specifies
		which of the upnode axes is required to align with a specified source
		axis.
		\par Parameters:
		<b>int axis</b>\n\n
		(0) if the upnode X axis coincides with a specified source axis. (1) if
		the upnode Y axis coincides with a specified source axis. (2) if the
		upnode Z axis coincides with a specified source axis. */
		virtual void SetUpNodeAxis(int axis)=0;
		/*! \remarks Sets the selection corresponding to the "Source/Upnode
		Alignment: Aligned to UpNode Axis:" radiobutton in the UI. Specifies
		which of the source axes is required to align with a specified upnode
		axis.
		\par Parameters:
		<b>int axis</b>\n\n
		(0) if the source X axis coincides with a specified upnode axis. (1) if
		the source Y axis coincides with a specified upnode axis. (2) if the
		source Z axis coincides with a specified upnode axis. */
		virtual void Set_StoUPAxis(int axis)=0;


	};

#define LOOKAT_ROT_PBLOCK_REF	0


//-------------------------------------------------------------
// Access to noise controller's parameters.
// All noise controllers are derived from this class
//

/*! \sa  Class Control.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class provides access to noise controller's parameters. All noise
controllers are derived from this class.\n\n
All methods of this class are implemented by the system.  */
class INoiseControl : public StdControl {
	public:
		/*! \remarks Sets the seed value for the noise controller.
		\par Parameters:
		<b>int seed</b>\n\n
		The seed value, greater than or equal to zero. */
		virtual void SetSeed(int seed)=0;
		/*! \remarks Returns the seed value. */
		virtual int GetSeed()=0;
		/*! \remarks Sets the frequency parameter.
		\par Parameters:
		<b>float f</b>\n\n
		The value to set, greater than zero. */
		virtual void SetFrequency(float f)=0;
		/*! \remarks Returns the frequency value. */
		virtual float GetFrequency()=0;
		/*! \remarks Sets the fractal setting on or off.
		\par Parameters:
		<b>BOOL f</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetFractal(BOOL f)=0;
		/*! \remarks Returns the state of the fractal setting. TRUE if on;
		FALSE if off. */
		virtual BOOL GetFractal()=0;
		/*! \remarks Sets the roughness setting.
		\par Parameters:
		<b>float f</b>\n\n
		The value to set, between 0.0 and 1.0. */
		virtual void SetRoughness(float f)=0;
		/*! \remarks Returns the roughness setting. */
		virtual float GetRoughness()=0;
		/*! \remarks Sets the ramp in setting.
		\par Parameters:
		<b>TimeValue in</b>\n\n
		The value to set, greater than or equal to zero. */
		virtual void SetRampIn(TimeValue in)=0;
		/*! \remarks Returns the ramp in setting. */
		virtual TimeValue GetRampIn()=0;
		/*! \remarks Sets the ramp out setting.
		\par Parameters:
		<b>TimeValue out</b>\n\n
		The value to set, greater than or equal to zero. */
		virtual void SetRampOut(TimeValue out)=0;
		/*! \remarks Returns the ramp out setting. */
		virtual TimeValue GetRampOut()=0;
		/*! \remarks Sets the positive only setting (\>0) for the specified
		axis to the specified value.
		\par Parameters:
		<b>int which</b>\n\n
		Specifes the axis. One of the following values:\n\n
		<b>0</b>: X, <b>1</b>: y, <b>2</b>: Z.\n\n
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetPositiveOnly(int which,BOOL onOff)=0;
		/*! \remarks Returns the positive only setting (\>0) for the specified
		axis to the specified value.
		\par Parameters:
		<b>int which</b>\n\n
		Specifes the axis. One of the following values:\n\n
		<b>0</b>: X, <b>1</b>: y, <b>2</b>: Z. */
		virtual BOOL GetPositiveOnly(int which)=0;
		/*! \remarks Returns a pointer to the controller for the strength
		parameter. */
		virtual Control *GetStrengthController()=0;
		/*! \remarks Sets the controller used for the strength parameter.
		\par Parameters:
		<b>Control *c</b>\n\n
		Points to the controller to set. */
		virtual void SetStrengthController(Control *c)=0;
	};

//-------------------------------------------------------------
// Access to SurfPosition controller
//

/*! \sa  Class Control, Class INode.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class provides access to the surface position controller's parameters.\n\n
The following values may be used to access the surface controller's
references.\n\n
<b>SURFCONT_U_REF</b>\n\n
<b>SURFCONT_V_REF</b>\n\n
<b>SURFCONT_SURFOBJ_REF</b>\n\n
All methods of this class are implemented by the system.  */
class ISurfPosition : public Control {
	public:
		/*! \remarks Sets the node that this controller uses as the surface
		object.
		\par Parameters:
		<b>INode *node</b>\n\n
		Points to the node to set. */
		virtual void SetSurface(INode *node)=0;
		/*! \remarks Returns the alignment setting.
		\return  One of the following values:\n\n
		<b>0</b>: No Alignment.\n\n
		<b>1</b>: Align to U.\n\n
		<b>2</b>: Align to V. */
		virtual int GetAlign()=0;
		/*! \remarks Sets the alignment setting.
		\par Parameters:
		<b>int a</b>\n\n
		One of the following values:\n\n
		<b>0</b>: No Alignment.\n\n
		<b>1</b>: Align to U.\n\n
		<b>2</b>: Align to V. */
		virtual void SetAlign(int a)=0;
		/*! \remarks Returns the flip setting. TRUE if on; FALSE if off. */
		virtual BOOL GetFlip()=0;
		/*! \remarks Sets the flip setting.
		\par Parameters:
		<b>BOOL f</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetFlip(BOOL f)=0;
	};

// Surface controller references
#define SURFCONT_U_REF			0
#define SURFCONT_V_REF			1
#define SURFCONT_SURFOBJ_REF	2


//-------------------------------------------------------------
// Access to the LinkCtrl
//

class ILinkCtrl;
#define LINK_CONSTRAINT_INTERFACE Interface_ID(0x32f03b37, 0x6700693a)
#define GetLinkConstInterface(cd) \
		(LinkConstTransform*)(cd)->GetInterface(LINK_CONSTRAINT_INTERFACE)
#define ADD_WORLD_LINK  PROPID_USER + 10
#define LINK_TIME_CONTROL_CLASS_ID Class_ID(0x5d084b4b, 0x1b1d318a)

/*! \sa  Class Control.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class represents the interface to the Link Controller. You can obtain a
pointer to the link controller interface using;
<b>GetLinkConstInterface(cd)</b>. This macro will return\n\n
<b>(LinkConstTransform*)(CD)-\>GetFPInterface(LINK_CONSTRAINT_INTERFACE).</b>\n\n
Developers may use the following values to access the references of the Link
controller.\n\n
<b>LINKCTRL_CONTROL_REF</b>\n\n
The TM controller\n\n
<b>LINKCTRL_FIRSTPARENT_REF</b>\n\n
The index of the first parent node.\n\n
<b>LINKCTRL_PBLOCK_REF</b>\n\n
The parameter block.\n\n
All methods of this class are Implemented by the System.  */
class ILinkCtrl : public Control, public FPMixinInterface  
{
	public:

		enum {	get_num_targets,		get_node,			set_node,
				get_frame_no,			set_frame_no,		add_target,		
				delete_target,			add_world, };
		

		// FUNCTION_PUBLISHING		
		// Function Map for Function Publish System 
		//***********************************
		BEGIN_FUNCTION_MAP

			FN_0(get_num_targets,	TYPE_INT,  GetNumTargets);
			FN_1(get_node,			TYPE_INODE, GetNode, TYPE_INDEX);
//			FN_2(set_node,			TYPE_BOOL, SetNode, TYPE_INODE, TYPE_INT);
			FN_1(get_frame_no,		TYPE_INT, GetFrameNumber, TYPE_INDEX);
			FN_2(set_frame_no,		TYPE_BOOL, SetFrameNumber, TYPE_INDEX, TYPE_INT);
			FN_2(add_target,		TYPE_BOOL, AddTarget, TYPE_INODE, TYPE_INT);
			FN_1(delete_target,		TYPE_BOOL, DeleteTarget, TYPE_INDEX);
			FN_1(add_world,			TYPE_INT, AddWorld, TYPE_INT);

		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    // <-- must implement 
		//End of Function Publishing system code 
		//***********************************{
//	public:
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method returns the number of parents (links). */
		virtual int GetNumTargets()=0;
		/*! \remarks Returns the start time associated with the 'i-th' link.
		\par Parameters:
		<b>int i</b>\n\n
		Specifies which parent (link). */
		virtual TimeValue GetLinkTime(int i)=0;
		/*! \remarks Sets the start time associated with the 'i-th' link. See
		<b>LinkTimeChanged()</b> below. Note: This method also sorts the
		indices according to increasing time values.
		\par Parameters:
		<b>int i</b>\n\n
		Specifies which parent (link).\n\n
		<b>TimeValue t</b>\n\n
		The time to set. */
		virtual void SetLinkTime(int i,TimeValue t)=0;
		/*! \remarks This method should be called after changing link times.
		Note: This method also sorts the indices according to increasing time
		values. */
		virtual void LinkTimeChanged()=0; // call after changing  Link times
		/*! \remarks Adds a new link at the specified time.
		\par Parameters:
		<b>INode *node</b>\n\n
		Points to the node of the link to add.\n\n
		<b>TimeValue t</b>\n\n
		The time to change to this link. */
		virtual void AddNewLink(INode *node,TimeValue t)=0;
		/*! \remarks This method will delete the specified link.
		\par Parameters:
		<b>int frameNo</b>\n\n
		Specifies which link to delete. */
		virtual BOOL DeleteTarget(int frameNo)=0;

		/*! \remarks This method returns the start frame of the specified
		target.
		\par Parameters:
		<b>int targetNumber</b>\n\n
		The target number for which to get the start frame. */
		virtual int GetFrameNumber(int targetNumber)=0;
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method allows you to set the start frame of the specified target.
		\par Parameters:
		<b>int targetNumber</b>\n\n
		The target number for which to set the start frame.\n\n
		<b>int frameNumber</b>\n\n
		The time value.
		\return  TRUE if the start frame is set, otherwise FALSE. */
		virtual BOOL SetFrameNumber(int targetNumber, int frameNumber)=0;
//		virtual BOOL SetNode(INode *target, int targetNumber)=0;
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method allows you to append a target.
		\par Parameters:
		<b>INode *target</b>\n\n
		The target node to append.\n\n
		<b>int frameNo</b>\n\n
		The frame time.
		\return  TRUE if the target was appended, otherwise FALSE. */
		virtual BOOL AddTarget(INode *target, int frameNo)=0;
//		BOOL AddWorld(int frameNo)=0;
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method returns one of the link nodes that the link constraint
		controller targets, specified by <b>targetNumber</b>.
		\par Parameters:
		<b>int targetNumber</b>\n\n
		The node number in the link target list to be obtained. */
		virtual INode* GetNode(int targetNumber)=0;

	private:

		virtual int AddWorld(int frameNo)
		{
			if (frameNo==-99999) frameNo = GetCOREInterface()->GetTime()/GetTicksPerFrame();
			return SetProperty(ADD_WORLD_LINK, &frameNo);
		}

	};

// LinkCtrl references
#define LINKCTRL_CONTROL_REF		0	// the TM controller
#define LINKCTRL_FIRSTPARENT_REF	1	// parent nodes... refs 1-n
#define LINKCTRL_PBLOCK_REF			2   // added for Paramblock implementation
#define LINKCTRL_LTCTL_REF			3
#define LINKCTRL_CORE_REFs			4  //Total "core" refs for LINKCTRL, nodes references start at this index



//-------------------------------------------------------------
// Access to the OLD LookatControl
//

/*! \sa  Class Control.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class provides access to the LookAt Controller parameters.\n\n
Developers may use the following values to access the references of the Look At
controller.\n\n
<b>LOOKAT_TARGET_REF</b>\n\n
<b>LOOKAT_POS_REF</b>\n\n
<b>LOOKAT_ROLL_REF</b>\n\n
<b>LOOKAT_SCL_REF</b>\n\n
All methods of this class are implemented by the system.  */
class ILookatControl : public Control {
	public:
		/*! \remarks Sets the flip setting.
		\par Parameters:
		<b>BOOL f</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetFlip(BOOL f)=0;
		/*! \remarks Returns the state of the flip setting. TRUE if on; FALSE
		if off. */
		virtual BOOL GetFlip()=0;
		/*! \remarks Sets the axis setting.
		\par Parameters:
		<b>int a</b>\n\n
		One of the following values:\n\n
		<b>0</b>: X axis.\n\n
		<b>1</b>: Y axis.\n\n
		<b>2</b>: Z axis. */
		virtual void SetAxis(int a)=0;
		/*! \remarks Returns the axis setting.
		\return  One of the following values:\n\n
		<b>0</b>: X axis.\n\n
		<b>1</b>: Y axis.\n\n
		<b>2</b>: Z axis. */
		virtual int GetAxis()=0;
	};

// References for the lookat controller
#define LOOKAT_TARGET_REF	0
#define LOOKAT_POS_REF		1
#define LOOKAT_ROLL_REF		2
#define LOOKAT_SCL_REF		3

//-------------------------------------------------------------
//Access to the New Boolean Controller AG 11/08/01
//


class IBoolCntrl;
#define BOOL_CONTROL_INTERFACE Interface_ID(0x5d511b6, 0x52a302db)
#define GetIBoolCntrlInterface(cd) \
		(IBoolCntrl*)(cd)->GetInterface(BOOL_CONTROL_INTERFACE)
/*! \sa Class Animatable, Class StdControl, Class IKeyControl\n\n
\par Description:
<b>This class is only available in release 5 or later.</b>\n\n
Certain problems plague the current ON/OFF controller that produces
counter-intuitive results and makes it difficult to use it in an animation
system. The objective of this feature is to overcome these problems, either by
fixing the existing controller or by creating a new controller. In the
beginning we'll focus on the first option.\n\n
We have decided to keep the current ON/OFF controller as it is because of its
utility to certain users and develop a new Boolean controller.\n\n
This is a controller similar to On/Off. However, its animation behavior is
quite different in that, unlike On/Off, it is NOT a flip flop control. Each key
in On/Off merely creates a state change. Its output is -1 or 1.\n\n
The Boolean controller is a True/False control. Its output is the value of the
last key which can only be either 0 or 1, thus any number of similar keys may
be in sequence; the controller is merely evaluated at the current time and its
value is output. This is not possible with the On/Off controller where each key
must necessarily be a change of state from either -1 to 1 <b>OR</b> 1 to
-1.\n\n
The Boolean Controller will be useful for any situation where something must be
turned on and off and the user wishes to work in a non-linear fashion i.e.
jumping backwards and forwards at will in the time line. For example, such
binary functions as object smoothing, object visibility scale multipliers,
light on/off state, etc.\n\n
The most pressing need for the Boolean is in the IK enabled state. Currently,
the enable state is controlled by the On/Off controller. The On/Off behavior is
in direct conflict with the idea of non-linear IK/FK blend animation. For
multiple reasons that are too difficult and involved to go into here the flip
flop behavior of On/Off becomes cumbersome at best. Furthermore, in R4 the
behavior of On/Off was modified to mitigate this problem: if the Animate button
is active and the IK Enable button is pressed, the enable track is examined, if
any enable keys exist in the track later in time they are "rippled" back to the
current time in order to preserve the overall animated state of IK Enable. With
the Animate button off On/Off behaves as it did in R3: the new key is
introduced and all existing animation in the Enable track is inverted. It is
only when an even number of keys is added to the track that existing animation
is preserved.\n\n
The code for this controller is in: <b>maxsdk/samples/controllers/boolcntrl</b>
*/
class IBoolCntrl: public StdControl, public IKeyControl {

};

// References for the boolean controller controller
#define BOOL_PBLOCK_REF		0



//-------------------------------------------------------------
// [dl | 22mar2002]
// Interface for exposing the new parameter "Use Target as Up Node" of the
// LookAtControl
#define ILOOKATCONTROL_EXTENSION Interface_ID(0x40ce4981, 0x3ea31c3b)

class ILookatControl_Extension : public FPMixinInterface {
public:

    virtual void SetTargetIsUpNode(bool val) = 0;
    virtual bool GetTargetIsUpNode() const = 0;

    // -- From BaseInterface
    virtual Interface_ID GetID() { return ILOOKATCONTROL_EXTENSION; }
};


//-------------------------------------------------------------
//Access to the List Controller
//
#define FLOATLIST_CONTROL_CLASS_ID		0x4b4b1000
#define POINT3LIST_CONTROL_CLASS_ID		0x4b4b1001
#define POSLIST_CONTROL_CLASS_ID		0x4b4b1002
#define ROTLIST_CONTROL_CLASS_ID		0x4b4b1003
#define SCALELIST_CONTROL_CLASS_ID		0x4b4b1004
#define DUMMY_CONTROL_CLASS_ID			0xeeefffff
#define MASTERLIST_CONTROL_CLASS_ID		0x4b4b1015
#define POINT4LIST_CONTROL_CLASS_ID		0x4b4b1005

class IListControl;

#define LIST_CONTROLLER_INTERFACE Interface_ID(0x444e7687, 0x722e6e36)

#define GetIListControlInterface(cd) \
		(IListControl*)(cd)->GetInterface(LIST_CONTROLLER_INTERFACE)

//! \brief IListControl provides API access to list control functionality.
/*! The interface is returned by calling GetIListControlInterface.
*/
/*! \sa  Class Control, Class FPMixinInterface\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the interface to the list controller. You can obtain a
pointer to the list control interface using;
<b>GetIListControlInterface(cd)</b>. This macro will return
<b>(IListControl*)(CD)-\>GetFPInterface(LIST_CONTROLLER_INTERFACE).</b>\n\n
The Class ID's of individual controllers are; <b>FLOATLIST_CONTROL_CLASS_ID,
POINT3LIST_CONTROL_CLASS_ID, POSLIST_CONTROL_CLASS_ID,
ROTLIST_CONTROL_CLASS_ID, SCALELIST_CONTROL_CLASS_ID, DUMMY_CONTROL_CLASS_ID,
MASTERLIST_CONTROL_CLASS_ID.</b>\n\n
All methods of this class are Implemented by the System.  */
class IListControl : public Control, public FPMixinInterface {
	public:
		
		enum{ list_getNumItems, list_setActive, list_getActive, list_cutItem, list_pasteItem, 
			  list_deleteItem, list_count, list_setActive_prop, list_getActive_prop, 
			  list_getName, list_setName, 
			  list_getSubCtrl, list_getSubCtrlWeight};

		// FUNCTION_PUBLISHING		
		// Function Map for Function Publish System 
		//***********************************
		#pragma warning(push)
		#pragma warning(disable:4238)
		BEGIN_FUNCTION_MAP
			FN_0		(list_getNumItems,	TYPE_INT,	GetListCount				);
			VFN_1		(list_setActive,				SetActive,		TYPE_INDEX	);
			FN_0		(list_getActive,	TYPE_INDEX,	GetActive					);
			VFN_1		(list_deleteItem,				DeleteItem,		TYPE_INDEX	);
			VFN_1		(list_cutItem,					CutItem,		TYPE_INDEX	);
			VFN_1		(list_pasteItem,				PasteItem,		TYPE_INDEX	);
			FN_1		(list_getName,	  TYPE_TSTR_BV,	GetName,		TYPE_INDEX	);
			VFN_2		(list_setName,					SetName,		TYPE_INDEX, TYPE_STRING );
			RO_PROP_FN	(list_count,					GetListCount,	TYPE_INT	); 
			PROP_FNS	(list_getActive_prop, GetActive, list_setActive_prop, SetActive, TYPE_INDEX	); 
			FN_1		(list_getSubCtrl,	TYPE_CONTROL,	GetSubCtrl,	TYPE_INDEX	);
			FNT_1		(list_getSubCtrlWeight,	TYPE_FLOAT,	GetSubCtrlWeight,	TYPE_INDEX	);
		END_FUNCTION_MAP
		#pragma warning(pop)

		FPInterfaceDesc* GetDesc();    // <-- must implement 
		//End of Function Publishing system code 
		//***********************************

		/*! \remarks This method returns the number of items that appear in
		the List controllers list box. */
		virtual int	 GetListCount()=0;
		/*! \remarks This method allows you to sets the indexed item in the
		list active so its parameters appear in the motion panel, and any input
		is directed to the indexed sub controller.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the item to set as the active item. */
		virtual void SetActive(int index)=0;
		/*! \remarks This method returns the index of the currently active
		item. */
		virtual int	 GetActive()=0;
		/*! \remarks This method allows you to delete the indexed sub
		controller from the list.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the item to delete from the list. */
		virtual void DeleteItem(int index)=0;
		/*! \remarks This method allows you to cutsthe index sub controller
		from the list and stores it in the buffer to paste later.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the item you wish to cut. */
		virtual void CutItem(int index)=0;
		/*! \remarks This method allows you to paste the sub-controller from
		the buffer into the indexed slot in the\n\n
		list.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the slot to paste into. */
		virtual void PasteItem(int index)=0;
		/*! \remarks This method allows you to set the name of an indexed
		item.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the item.\n\n
		<b>MSTR name</b>\n\n
		The name to set it to. */
		virtual void SetName(int index, MSTR name)=0;
		/*! \remarks This method returns the class name of the indexed
		sub-controller if a user defined name doesn't exist.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the item for which to get the name. */
		virtual MSTR GetName(int index)=0;

		//! Get a subcontroller from its index. 
		//! \return - a pointer to a Control, or NULL if the index is invalid.
		//! \param[in] in_index - index of the list controller subcontroller
		virtual Control*	GetSubCtrl(int in_index) const =0;

		//! Get the weight of a subcontroller at a given time. 
		//! \return - the weight of the subcontroller (non-percent, typically 0.0f to 1.0f), or -1.0f if the index is invalid.
		//! \param[in] in_time - time of the weight evaluation (in case of an animated weight)
		//! \param[in] in_index - index of the list controller subcontroller
		virtual float		GetSubCtrlWeight(int in_index, TimeValue in_time) const=0;
};


//-------------------------------------------------------------
// Access to Spline IK Control modifier
//
class ISplineIKControl;
#define SPLINEIK_CONTROL_INTERFACE Interface_ID(0x7c93607a, 0x47d54f80)
#define GetISplineIKControlInterface(cd) \
		(ISplineIKControl*)(cd)->GetInterface(SPLINEIK_CONTROL_INTERFACE)
/*! <b>This class is only available in release 5 or later.</b>\n\n
This class refers to the new modifier which, when assigned to a spline (or a
NURBS) curve, generates a certain number of helper objects attached to the
knots of the curve.\n\n
Spline IK Control Modifier (works for NURBS Point curve and NURBS CV Curve
too): This is a modifier and can be used independently of the SplineIK. When
applied to a spline, this modifier creates one point helper for each knot of
the spline. The user can then animate the spline by simply animating (position
and rotation) the point helpers. Thus to animate the spline, the user wouldn't
need to get into the sub-object level.\n\n
There are three options Link Types, presented as a set of 3 radio buttons:\n\n
<b>Link All in Hierarchy (default):</b>\n\n
Makes a helper a child to its immediately previous helper. So, Helper#2 is
child to Helper#1, Helper#3 is child to Helper#2, and so on. Helper#1 is still
child to the world. Translation and rotation of a helper then "solidly"
moves/rotates part of the spline _subsequent_ to the selected helper. The part
of the spline previous to the helper is unaffected.\n\n
<b>Link All to Root</b>\n\n
Makes all helpers children to Helper#1, i.e., knot#1. Helper#1 can be position
constrained or linked to another object, like it is possible above.
Additionally individual helpers can be moved and rotated without any other
helper being affect.\n\n
<b>No Linking</b>\n\n
All helpers are independent -- not linked to any other helper -- so that they
can be moved and rotated without any other helper being affect. \n\n
<b>"Create Helpers" button:</b>\n\n
Helpers are not automatically added to the spline on the assignment of the
modifier. To do that the user need to press the "Create Helpers" button.\n\n
Helper Display:\n\n
These are the all the options: <ul> <li> center marker (default OFF) </li> <li>
axis tripod (default OFF) </li> <li> cross (default OFF) </li> <li> box
(default ON) </li> <li> size (default 20.0) </li> <li> constant screen size
(default OFF) </li> <li> draw on top (default ON) for a standard point object.
</li> </ul> If the user adds ("insert") a knot to the spline, a new helper
object is automatically created at that knotpoint.\n\n
Furthermore, please refer to the implementation project which is in
<b>/maxsdk/samples/modifiers/splineikcontrol</b>.\n\n
The following helper methods have been added to <b>istdplug.h</b> for general
access: */
class ISplineIKControl: public Modifier, public FPMixinInterface {

	public:

		enum { SplineIKControl_params };

		enum {  
			sm_point_node_list,		sm_helpersize,	sm_helper_centermarker,		sm_helper_axistripod, 
			sm_helper_cross,		sm_helper_box,	sm_helper_screensize,		sm_helper_drawontop,
			sm_link_types,

		};

		enum{ getHelperCount, getKnotCount, link_allToRoot, link_allinHierarchy, link_none, create_hlpr};

		// FUNCTION_PUBLISHING		
		// Function Map for Function Publish System 
		//***********************************
		BEGIN_FUNCTION_MAP
			FN_0		(getHelperCount,		TYPE_INT,	GetHelperCount		);
			FN_0		(getKnotCount,			TYPE_INT,	GetKnotCount		);
			FN_0		(link_allToRoot,		TYPE_BOOL,	LinkToRoot			);
			FN_0		(link_allinHierarchy,	TYPE_BOOL,	LinkInHierarchy		);
			FN_0		(link_none,				TYPE_BOOL,	UnLink				);
			FN_1		(create_hlpr,			TYPE_BOOL,	CreateHelpers,		TYPE_INT);
		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    // <-- must implement 
		//End of Function Publishing system code 
		//***********************************

		/*! \remarks Obtain the number of helpers created\n\n
		  */
		virtual int	 GetHelperCount()=0;
		/*! \remarks Gets the total number of knots for spline.\n\n
		  */
		virtual int	 GetKnotCount()=0;
		/*! \remarks Makes all helpers children to Helper#1, i.e., knot#1.
		Helper#1 can be position constrained or linked to another object, like
		it is possible above. Additionally individual helpers can be moved and
		rotated without any other helper being affected.\n\n
		  */
		virtual BOOL LinkToRoot() = 0;
		/*! \remarks Makes a helper a child to its immediately previous
		helper. So, Helper#2 is child to Helper#1, Helper#3 is child to
		Helper#2, and so on. Helper#1 is still child to the world. Translation
		and rotation of a helper then "solidly" moves/rotates part of the
		spline _subsequent_ to the selected helper. The part of the spline
		previous to the helper is unaffected.\n\n
		  */
		virtual BOOL LinkInHierarchy() = 0;
		/*! \remarks All helpers are independent - not linked to any other
		helper so that they can be moved and rotated without any other helper
		being affected.\n\n
		  */
		virtual BOOL UnLink() = 0;
		/*! \remarks Helpers are not automatically added to the spline on the
		assignment of the modifier. To do that the user need to press the
		"<b>Create Helpers</b>" button. */
		virtual BOOL CreateHelpers(int knotCt) = 0;

};

// References for the splineIK controller
#define SPLINEIKCONTROL_PBLOCK_REF		0



// The following two enums are transfered from helpers\pthelp.cpp by AG: 01/20/2002 
// in order to access the parameters for use in Spline IK Control modifier
// and the Spline IK Solver

// block IDs
enum { pointobj_params, };

// pointobj_params IDs

 enum { 
	pointobj_size, pointobj_centermarker, pointobj_axistripod, 
	pointobj_cross, pointobj_box, pointobj_screensize, pointobj_drawontop };



// The following two enums are transfered from modifiers\nspline.cpp  by AG: 01/20/2002 
// in order to access the parameters for use in Spline IK Solver

// block IDs
enum { nspline_params};

// nspline_params ID
enum { nspline_length};

//! Access to the new ISplineIKControl2 interface. This interface exposes the helper nodes
//! that drive the spline ik modifier.
class ISplineIKControl2;
#define SPLINEIK_CONTROL_INTERFACE2 Interface_ID(0x72428cc, 0x2a4a1b5f)
#define GetISplineIKControlInterface2(cd) \
		(ISplineIKControl2*)(cd)->GetInterface(SPLINEIK_CONTROL_INTERFACE2)
class ISplineIKControl2 : public BaseInterface
{
public:
	//!Get's the helper nodes that define the spline.
	//! param [out] helperNodes. The node of the spline helpers.
	virtual void GetHelperNodes(INodeTab & helperNodes)=0;
};



//-------------------------------------------------------------
// Access to FFD modifiers
//

// Can either be casted to IFFDMod<Modifier> or IFFDMod<WSMObject> based on the ClassID
/*! \sa  Class Control, Class Point3, Class IPoint3, Class Modifier.\n\n\
\par Description:
This class is available in release 3.0 and later only.\n\n
This class is an interface into both FFD OS modifiers and also into helper
objects for FFD spacewarps.\n\n
Given a pointer to ReferenceTarget <b>ref</b>, you can use it as follows:
\code
Class_ID id = ref->ClassID();
if (id==FFDNMOSSQUARE_CLASS_ID || id==FFDNMOSCYL_CLASS_ID ||
	id==FFD44_CLASS_ID || id==FFD33_CLASS_ID || id==FFD22_CLASS_ID)
	IFFDMod<Modifier>* ffd = (IFFDMod<Modifier>*)ref;
  // call various methods
}
else if(id==FFDNMWSSQUARE_CLASS_ID || id = FFDNMWSCYL_CLASS_ID)
	IFFDMod<WSMObject>* ffd = (IFFDMod<WSMObject>*)ref;
// call various methods
}
\endcode  */
template <class T> class IFFDMod : public T {
	public:
		/*! \remarks Returns the number of lattice control points. */
		virtual int			NumPts()=0;								// number of lattice control points 
		/*! \remarks Returns the number of Control Ponits having controllers.
		*/
		virtual int			NumPtConts()=0;							// number of CP's having controllers
		/*! \remarks Returns a pointer to the 'i-th' Control Point controller.
		\par Parameters:
		<b>int i</b>\n\n
		The zero based index of the Control Point. */
		virtual Control*	GetPtCont(int i)=0;						// get i'th CP controller
		/*! \remarks Sets the controller used by the specified Control Point.
		\par Parameters:
		<b>int i</b>\n\n
		The zero based index of the Control Point.\n\n
		<b>Control *c</b>\n\n
		Points to the controller to set. */
		virtual void		SetPtCont(int i,Control *c)=0;			// set i'th CP controller
		/*! \remarks Returns the 'i-th' Control Point.
		\par Parameters:
		<b>int i</b>\n\n
		The zero based index of the Control Point. */
		virtual Point3		GetPt(int i)=0;							// get i'th CP
		/*! \remarks Sets the 'i-th' control point.
		\par Parameters:
		<b>int i</b>\n\n
		The zero based index of the Control Point.\n\n
		<b>Point3 p</b>\n\n
		The point to set. */
		virtual	void		SetPt(int i, Point3 p)=0;				// set i'th CP
		/*! \remarks Sets the lattice dimension.
		\par Parameters:
		<b>IPoint3 d</b>\n\n
		The dimensions to set.
		\par Default Implementation:
		<b>{ }</b> */
		virtual	void		SetGridDim(IPoint3 d) { }				// set the lattice dimensions
		/*! \remarks Returns the lattice dimensions.
		\par Default Implementation:
		<b>{ return IPoint3(0,0,0); }</b> */
		virtual	IPoint3		GetGridDim() { return IPoint3(0,0,0); }	// get the lattice dimensions
		/*! \remarks This method assigns controllers to all the Control
		Points.
		\par Default Implementation:
		<b>{ }</b> */
		virtual void		AnimateAll() { }						// assign controllers to all CP's				
		/*! \remarks Calling this method is the equivalent of pressing the
		Conform button in the FFD UI. Note: This method is not valid for
		WSMObject's.
		\par Default Implementation:
		<b>{ }</b> */
		virtual	void		Conform() { }							// not valid for WSMObject's
		/*! \remarks Selects or de-selects the specified Control Point.
		\par Parameters:
		<b>int i</b>\n\n
		The zero based index of the Control Point.\n\n
		<b>BOOL sel</b>\n\n
		TRUE to select; FALSE to de-selelct.\n\n
		<b>BOOL clearAll=FALSE</b>\n\n
		TRUE to clear all the currently selected Control Points before setting
		the specified one; FALSE to leave the selected points alone.
		\par Default Implementation:
		<b>{ }</b> */
		virtual void		SelectPt(int i, BOOL sel, BOOL clearAll=FALSE) { }
		virtual void		PlugControllers(TimeValue t,BOOL all)=0;
};

//-------------------------------------------------------------
// Access to mesh selections in editable mesh and edit mesh mod
//

#include "namesel.h"

// Selection levels:
#define IMESHSEL_OBJECT 0
#define IMESHSEL_VERTEX 1
#define IMESHSEL_FACE 2
#define IMESHSEL_EDGE 3

/*! \sa  Class Mesh, Class LocalModData, Class IMeshSelectData.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class provides access to selection data for the Mesh Select Modifier,
Editable Mesh and Edit Mesh modifier.\n\n
To get a pointer to this interface given a pointer to a modifier or editable
mesh object, use the following macro (defined in AnimatableInterfaceIDs.h ). Using this
macro, given any Animatable, it is easy to ask for the interface.\n\n
<b>#define GetMeshSelectInterface(anim)
((IMeshSelect*)anim-\>GetInterface(I_MESHSELECT))</b>\n\n
A plug-in developer may use this macro as follows:\n\n
<b>IMeshSelect *ims = GetMeshSelectInterface(anim);</b>\n\n
This return value will either be NULL or a pointer to a valid Mesh Select
interface.  */
class IMeshSelect: public MaxHeapOperators {
public:
	/*! \remarks Returns the current level of selection for the modifier.
	\return  One of the following values:\n\n
	<b>IMESHSEL_OBJECT</b>: Object level.\n\n
	<b>IMESHSEL_VERTEX</b>: Vertex level.\n\n
	<b>IMESHSEL_FACE</b>: Face level.\n\n
	<b>IMESHSEL_EDGE</b>: Edge level. */
	virtual DWORD GetSelLevel()=0;
	/*! \remarks Sets the selection level of the modifier.
	\par Parameters:
	<b>DWORD level</b>\n\n
	One of the following values:\n\n
	<b>IMESHSEL_OBJECT</b>: Object level.\n\n
	<b>IMESHSEL_VERTEX</b>: Vertex level.\n\n
	<b>IMESHSEL_FACE</b>: Face level.\n\n
	<b>IMESHSEL_EDGE</b>: Edge level. */
	virtual void SetSelLevel(DWORD level)=0;
	/*! \remarks This method must be called when the <b>LocalModData</b> of
	the modifier is changed. Developers can use the methods of
	<b>IMeshSelectData</b> to get and set the actual selection for vertex, face
	and edge. When a developers does set any of these selection sets this
	method must be called when done. */
	virtual void LocalDataChanged()=0;
	/*! \remarks	Returns TRUE if this modifier or object has weighted vertex selection data
	(Soft Selection data), FALSE if not.
	\par Default Implementation:
	<b>{ return FALSE; }</b> */
	virtual BOOL HasWeightedVertSel () { return FALSE; }
	/*! \remarks	Returns TRUE if this modifier or object can assign weighted vertex
	selection data; FALSE if it cannot.
	\par Default Implementation:
	<b>{ return FALSE; }</b> */
	virtual BOOL CanAssignWeightedVertSel () { return FALSE; }
};

/*! \sa  Class ModContext, Class BitArray, Class IMeshSelect, Class Interface, Class GenericNamedSelSetList.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
When a developer gets the <b>LocalModData</b> from the <b>ModContext</b> of the
Mesh Select Modifier or Edit Mesh Modifier, they may cast it to this class and
use these methods. They may be used to get/set the vert/face/edge selection
state of the modifier. This class also provides access to the named sub-object
selection sets.\n\n
To get a pointer to this interface given a pointer to a modifier use the
following macro (defined in AnimatableInterfaceIDs.h ). Using this macro, given any
Animatable, it is easy to ask for the interface.\n\n
<b>#define GetMeshSelectDataInterface(anim)
((IMeshSelectData*)anim-\>GetInterface(I_MESHSELECTDATA))</b>\n\n
A plug-in developer may use this macro as follows:\n\n
<b>IMeshSelectData *imsd = GetMeshSelectDataInterface(anim);</b>\n\n
This return value will either be NULL or a pointer to a valid Mesh Select Data
interface.  */
class IMeshSelectData: public MaxHeapOperators {
public:
	/*! \remarks Returns a <b>BitArray</b> that reflects the current vertex
	selection. There is one bit for each vertex. Bits that are 1 indicate the
	vertex is selected. */
	virtual BitArray GetVertSel()=0;
	/*! \remarks Returns a <b>BitArray</b> that reflects the current face
	selection. There is one bit for each face. Bits that are 1 indicate the
	face is selected. */
	virtual BitArray GetFaceSel()=0;
	/*! \remarks Returns a <b>BitArray</b> that reflects the current edge
	selection. There is one bit for each edge. Bits that are 1 indicate the
	edge is selected. */
	virtual BitArray GetEdgeSel()=0;
	
	/*! \remarks Sets the vertex selection of the modifier.
	\par Parameters:
	<b>BitArray \&set</b>\n\n
	There is one bit for each vertex. Bits that are 1 indicate the vertex is
	selected.\n\n
	<b>IMeshSelect *imod</b>\n\n
	This parameter is available in release 3.0 and later only.\n\n
	Points to the IMeshSelect instance (generally this is a modifier).\n\n
	<b>TimeValue t</b>\n\n
	This parameter is available in release 3.0 and later only.\n\n
	The current time at which the call is made. */
	virtual void SetVertSel(BitArray &set, IMeshSelect *imod, TimeValue t)=0;
	/*! \remarks Sets the face selection of the modifier.
	\par Parameters:
	<b>BitArray \&set</b>\n\n
	There is one bit for each face. Bits that are 1 indicate the face is
	selected.\n\n
	<b>IMeshSelect *imod</b>\n\n
	This parameter is available in release 3.0 and later only.\n\n
	Points to the IMeshSelect instance (generally this is a modifier).\n\n
	<b>TimeValue t</b>\n\n
	This parameter is available in release 3.0 and later only.\n\n
	The current time at which the call is made. */
	virtual void SetFaceSel(BitArray &set, IMeshSelect *imod, TimeValue t)=0;
	/*! \remarks Sets the edge selection of the modifier.
	\par Parameters:
	<b>BitArray \&set</b>\n\n
	There is one bit for each edge. Bits that are 1 indicate the edge is
	selected.\n\n
	<b>IMeshSelect *imod</b>\n\n
	This parameter is available in release 3.0 and later only.\n\n
	Points to the IMeshSelect instance (generally this is a modifier).\n\n
	<b>TimeValue t</b>\n\n
	This parameter is available in release 3.0 and later only.\n\n
	The current time at which the call is made. */
	virtual void SetEdgeSel(BitArray &set, IMeshSelect *imod, TimeValue t)=0;

	/*! \remarks	Returns a reference to an instance of <b>GenericNamedSelSetList</b> used
	for storing vertex level named selection sets. This class provides access
	to and the ability to manipulate a list of named selection sets. */
	virtual GenericNamedSelSetList & GetNamedVertSelList ()=0;
	/*! \remarks	Returns a reference to an instance of <b>GenericNamedSelSetList</b> used
	for storing edge level named selection sets. This class provides access to
	and the ability to manipulate a list of named selection sets. */
	virtual GenericNamedSelSetList & GetNamedEdgeSelList ()=0;
	/*! \remarks	Returns a reference to an instance of <b>GenericNamedSelSetList</b> used
	for storing face level named selection sets. This class provides access to
	and the ability to manipulate a list of named selection sets. */
	virtual GenericNamedSelSetList & GetNamedFaceSelList ()=0;

	/*! \remarks	Retrieves the weighted vertex selections data (Soft Selection data).
	\par Parameters:
	<b>int nv</b>\n\n
	The number of vertices.\n\n
	<b>float *sel</b>\n\n
	An array of floats to store the results.
	\par Default Implementation:
	<b>{}</b> */
	virtual void GetWeightedVertSel (int nv, float *sel) {}
	/*! \remarks	Sets the weighted vertex selection data (Soft Selection data) to the values
	passed.
	\par Parameters:
	<b>int nv</b>\n\n
	The number of vertices.\n\n
	<b>float *sel</b>\n\n
	An array of floats with the data.\n\n
	<b>IMeshSelect *imod</b>\n\n
	Points to the IMeshSelect object.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to set the data.
	\par Default Implementation:
	<b>{}</b> */
	virtual void SetWeightedVertSel (int nv, float *sel, IMeshSelect *imod, TimeValue t) {}
};

//-------------------------------------------------------------
// Access to spline selections and operations in SplineShape and EditSplineMod
//

// selection levels defined in splshape.h   

/*! \sa  Class Animatable, <a href="ms-its:3dsmaxsdk.chm::/shps_shapes_and_splines.html">Working with Shapes and Splines</a>.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class provides an interface to the Spline Select Modifer. To obtain a
pointer to this class use the method <b>Animatable::GetInterface()</b> passing
<b>I_SPLINESELECT</b>.\n\n
For example:\n\n
<b>ISplineSelect *iss =
(ISplineSelect*)anim-\>GetInterface(I_SPLINESELECT));</b>\n\n
<b>DWORD sl = iss-\>GetSelLevel();</b>  */
class ISplineSelect: public MaxHeapOperators					// accessed via GetInterface(I_SPLINESELECT)
{
public:
	/*! \remarks Returns a value indicating the current selection level of the
	modifier. One of the following values:\n\n
	<b>SS_VERTEX</b>\n\n
	<b>SS_SEGMENT</b>\n\n
	<b>SS_SPLINE</b>\n\n
	<b>SS_OBJECT</b> */
	virtual DWORD GetSelLevel()=0;
	/*! \remarks Sets the current level of selection of the modifier.
	\par Parameters:
	<b>DWORD level</b>\n\n
	The level to set. One of the following values:\n\n
	<b>SS_VERTEX</b>\n\n
	<b>SS_SEGMENT</b>\n\n
	<b>SS_SPLINE</b>\n\n
	<b>SS_OBJECT</b> */
	virtual void SetSelLevel(DWORD level)=0;
	/*! \remarks This method must be called when the selection level of the
	modifier is changed. Developers can use the methods of <b>ISplineSelect</b>
	to get and set the actual selection data. When a developers does set any of
	these selection sets this method must be called when done. */
	virtual void LocalDataChanged()=0;
};

/*! \sa  Class ISplineSelect, Class Animatable, Class BitArray, <a href="ms-its:3dsmaxsdk.chm::/shps_shapes_and_splines.html">Working with Shapes and Splines</a>, Class GenericNamedSelSetList.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
When a developer gets the <b>LocalModData</b> from the <b>ModContext</b> of the
Spline Select Modifier, they may cast it to this class and use these methods.
They may be used to get/set the vertex/segment/spline selection state of the
modifier as well as the named selection sets.\n\n
To obtain a pointer to this class use the method
<b>Animatable::GetInterface()</b> passing <b>I_SPLINESELECTDATA</b>.\n\n
For example:\n\n
<b>ISplineSelectData *iss =
(ISplineSelectData*)anim-\>GetInterface(I_SPLINESELECTDATA));</b>\n\n
<b>BitArray vs = iss-\>GetVertSel();</b>  */
class ISplineSelectData: public MaxHeapOperators				// accessed via GetInterface(I_SPLINESELECTDATA)
{
public:
	// access spline sub-object selections, current & named
	/*! \remarks Returns a <b>BitArray</b> that reflects the current vertex
	selection. There is one bit for each vertex. Bits that are 1 indicate the
	vertex is selected. */
	virtual BitArray GetVertSel()=0;
	/*! \remarks Returns a <b>BitArray</b> that reflects the current segment
	selection. There is one bit for each segment. Bits that are 1 indicate the
	segment is selected. */
	virtual BitArray GetSegmentSel()=0;
	/*! \remarks Returns a <b>BitArray</b> that reflects the current spline
	selection. There is one bit for each spline. Bits that are 1 indicate the
	spline is selected. */
	virtual BitArray GetSplineSel()=0;
	
	/*! \remarks Sets the vertex selection of the modifier.
	\par Parameters:
	<b>BitArray \&set</b>\n\n
	There is one bit for each vertex. Bits that are 1 indicate the vertex
	should be selected.\n\n
	<b>ISplineSelect *imod</b>\n\n
	Points to the ISplineSelect instance (generally this is a modifier).\n\n
	<b>TimeValue t</b>\n\n
	The current time at which the call is made. */
	virtual void SetVertSel(BitArray &set, ISplineSelect *imod, TimeValue t)=0;
	/*! \remarks Sets the segment selection of the modifier.
	\par Parameters:
	<b>BitArray \&set</b>\n\n
	There is one bit for each vertex. Bits that are 1 indicate the segment
	should be selected.\n\n
	<b>ISplineSelect *imod</b>\n\n
	Points to the ISplineSelect instance (generally this is a modifier).\n\n
	<b>TimeValue t</b>\n\n
	The current time at which the call is made. */
	virtual void SetSegmentSel(BitArray &set, ISplineSelect *imod, TimeValue t)=0;
	/*! \remarks Sets the spline selection of the modifier.
	\par Parameters:
	<b>BitArray \&set</b>\n\n
	There is one bit for each spline. Bits that are 1 indicate the spline
	should be selected.\n\n
	<b>ISplineSelect *imod</b>\n\n
	Points to the ISplineSelect instance (generally this is a modifier).\n\n
	<b>TimeValue t</b>\n\n
	The current time at which the call is made. */
	virtual void SetSplineSel(BitArray &set, ISplineSelect *imod, TimeValue t)=0;

	/*! \remarks Returns a reference to a class used for manipulating the
	lists of vertex level named selection sets associated with this modifier.
	*/
	virtual GenericNamedSelSetList & GetNamedVertSelList ()=0;
	/*! \remarks Returns a reference to a class used for manipulating the
	lists of segment level named selection sets associated with this modifier.
	*/
	virtual GenericNamedSelSetList & GetNamedSegmentSelList ()=0;
	/*! \remarks Returns a reference to a class used for manipulating the
	lists of spline level named selection sets associated with this modifier.
	*/
	virtual GenericNamedSelSetList & GetNamedSplineSelList ()=0;
};

enum splineCommandMode { ScmCreateLine, ScmAttach, ScmInsert, ScmConnect, ScmRefine, ScmFillet, ScmChamfer, 
					     ScmBind, ScmRefineConnect, ScmOutline, ScmTrim, ScmExtend, ScmCrossInsert,
						 ScmBreak, ScmUnion, ScmSubtract, ScmCrossSection, ScmCopyTangent, ScmPasteTangent, };
enum splineButtonOp    { SopHide, SopUnhideAll, SopDelete, SopDetach, SopDivide, SopCycle,
						 SopUnbind, SopWeld, SopMakeFirst, SopAttachMultiple, SopExplode, SopReverse, 
						 SopClose, SopIntersect, SopMirrorHoriz, SopMirrorVert,
						 SopMirrorBoth, SopSelectByID, SopFuse, };
// LAM: added 9/3/00
enum splineUIParam {  };

/*! \sa  Class ISplineSelect, Class Animatable, <a href="ms-its:3dsmaxsdk.chm::/shps_shapes_and_splines.html">Working with Shapes and Splines</a>.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class provides an interface to the command modes and button press
operations of the Editable Spline object. To obtain a pointer to this class use
the method <b>Animatable::GetInterface()</b> passing <b>I_SPLINEOPS</b>.\n\n
For example:\n\n
<b>ISplineOps *iso = (ISplineOps*)anim-\>GetInterface(I_SPLINEOPS));</b>\n\n
<b>iso-\>StartCommandMode(ScmAttach);</b>  */
class ISplineOps: public MaxHeapOperators				// accessed via GetInterface(I_SPLINEOPS)
{
public:
	// start up interactive command mode, uses mode enum above
	/*! \remarks Begins the specified interactive command mode.
	\par Parameters:
	<b>splineCommandMode mode</b>\n\n
	The mode to begin. One of the following values:\n\n
	<b>ScmCreateLine</b>\n\n
	<b>ScmAttach</b>\n\n
	<b>ScmInsert</b>\n\n
	<b>ScmConnect</b>\n\n
	<b>ScmRefine</b>\n\n
	<b>ScmFillet</b>\n\n
	<b>ScmChamfer</b>\n\n
	<b>ScmBind</b>\n\n
	<b>ScmRefineConnect</b>\n\n
	<b>ScmOutline</b>\n\n
	<b>ScmTrim</b>\n\n
	<b>ScmExtend</b>\n\n
	<b>ScmBreak</b>\n\n
	<b>ScmUnion</b>\n\n
	<b>ScmCrossInsert</b> */
	virtual void StartCommandMode(splineCommandMode mode)=0;
	// perform button op, uses op enum above
	/*! \remarks Performs the same operation as a button press inside the
	Editable Spline UI.
	\par Parameters:
	<b>splineButtonOp opcode</b>\n\n
	The button operation to execute. One of the following values:\n\n
	<b>SopHide</b>\n\n
	<b>SopUnhideAll</b>\n\n
	<b>SopDelete</b>\n\n
	<b>SopDetach</b>\n\n
	<b>SopDivide</b>\n\n
	<b>SopBreak</b>\n\n
	<b>SopCycle</b>\n\n
	<b>SopUnbind</b>\n\n
	<b>SopWeld</b>\n\n
	<b>SopMakeFirst</b>\n\n
	<b>SopAttachMultiple</b>\n\n
	<b>SopExplode</b>\n\n
	<b>SopReverse</b>\n\n
	<b>SopClose</b>\n\n
	<b>SopUnion</b>\n\n
	<b>SopSubstract</b>\n\n
	<b>SopIntersect</b>\n\n
	<b>SopMirrorHoriz</b>\n\n
	<b>SopMirrorVert</b>\n\n
	<b>SopMirrorBoth</b>\n\n
	<b>SopSelectByID</b>\n\n
	<b>SopFuse</b> */
	virtual void ButtonOp(splineButtonOp opcode)=0;
// LAM: added 9/3/00
	// UI controls access
	/*! \remarks	This method allows you to get the edit spline parameters from the command
	panel. Currently not in use.
	\par Parameters:
	<b>splineUIParam uiCode</b>\n\n
	This enum is currently empty.\n\n
	<b>int \&ret</b>\n\n
	The returned value.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void GetUIParam (splineUIParam uiCode, int & ret) { }
	/*! \remarks	This method allows you to set the edit spline parameters from the command
	panel. Currently not in use.
	\par Parameters:
	<b>splineUIParam uiCode</b>\n\n
	This enum is currently empty.\n\n
	<b>int val</b>\n\n
	The value to set.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void SetUIParam (splineUIParam uiCode, int val) { }
	/*! \remarks	This method allows you to get the edit spline parameters from the command
	panel. Currently not in use.
	\par Parameters:
	<b>splineUIParam uiCode</b>\n\n
	This enum is currently empty.\n\n
	<b>float \&ret</b>\n\n
	The returned value.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void GetUIParam (splineUIParam uiCode, float & ret) { }
	/*! \remarks	This method allows you to set the edit spline parameters from the command
	panel. Currently not in use.
	\par Parameters:
	<b>splineUIParam uiCode</b>\n\n
	This enum is currently empty.\n\n
	<b>float val</b>\n\n
	The value to set.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void SetUIParam (splineUIParam uiCode, float val) { }
};

//-------------------------------------------------------------
// Access to spline selections and operations in PatchObject and EditPatchMod
//

// selection levels defined in patchobj.h   

/*! \sa  Class Animatable, <a href="ms-its:3dsmaxsdk.chm::/patches_root.html">Working with Patches</a>.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class provides an interface to the Patch Select Modifer. To obtain a
pointer to this class use the method <b>Animatable::GetInterface()</b> passing
<b>I_PATCHSELECT</b>.\n\n
For example:\n\n
<b>IPatchSelect *ips =
(IPatchSelect*)anim-\>GetInterface(I_PATCHSELECT));</b>\n\n
<b>DWORD sl = ips-\>GetSelLevel();</b>  */
class IPatchSelect: public MaxHeapOperators					// accessed via GetInterface(I_PATCHSELECT)
{
public:
	/*! \remarks Returns a value indicating the current selection level of the
	modifier. One of the following values:\n\n
	<b>PO_VERTEX</b>\n\n
	<b>PO_EDGE</b>\n\n
	<b>PO_PATCH</b>\n\n
	<b>PO_OBJECT</b> */
	virtual DWORD GetSelLevel()=0;
	/*! \remarks Sets the current level of selection of the modifier.
	\par Parameters:
	<b>DWORD level</b>\n\n
	The level to set. One of the following values:\n\n
	<b>PO_VERTEX</b>\n\n
	<b>PO_EDGE</b>\n\n
	<b>PO_PATCH</b>\n\n
	<b>PO_OBJECT</b> */
	virtual void SetSelLevel(DWORD level)=0;
	/*! \remarks This method must be called when the selection level of the
	modifier is changed. Developers can use the methods of this class to get
	and set the actual selection data. When a developers does set any of these
	selection sets this method must be called when done. */
	virtual void LocalDataChanged()=0;
};

/*! \sa  Class IPatchSelect, Class Animatable, Class BitArray, <a href="ms-its:3dsmaxsdk.chm::/patches_root.html">Working with Patches</a>, Class GenericNamedSelSetList.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
When a developer gets the <b>LocalModData</b> from the <b>ModContext</b> of the
Patch Select Modifier, they may cast it to this class and use these methods.
They may be used to get/set the vertex/edge/patch sub-object selection state of
the modifier as well as the named selection sets.\n\n
To obtain a pointer to this class use the method
<b>Animatable::GetInterface()</b> passing <b>I_PATCHSELECTDATA</b>.\n\n
For example:\n\n
<b>IPatchSelectData *ips =
(IPatchSelectData*)anim-\>GetInterface(I_PATCHSELECTDATA));</b>\n\n
<b>BitArray vs = ips-\>GetVertSel();</b>  */
class IPatchSelectData: public MaxHeapOperators				// accessed via GetInterface(I_PATCHSELECTDATA)
{
public:
	// access patch sub-object selections, current & named
	virtual BitArray GetVecSel()=0;
	/*! \remarks Returns a <b>BitArray</b> that reflects the current vertex
	selection. There is one bit for each vertex. Bits that are 1 indicate the
	vertex is selected. */
	virtual BitArray GetVertSel()=0;
	/*! \remarks Returns a <b>BitArray</b> that reflects the current edge
	selection. There is one bit for each edge. Bits that are 1 indicate the
	edge is selected. */
	virtual BitArray GetEdgeSel()=0;
	/*! \remarks Returns a <b>BitArray</b> that reflects the current patch
	selection. There is one bit for each patch. Bits that are 1 indicate the
	patch is selected. */
	virtual BitArray GetPatchSel()=0;
	
	virtual void SetVecSel(BitArray &set, IPatchSelect *imod, TimeValue t)=0;
	/*! \remarks Sets the vertex selection of the modifier.
	\par Parameters:
	<b>BitArray \&set</b>\n\n
	There is one bit for each vertex. Bits that are 1 indicate the vertex
	should be selected.\n\n
	<b>IPatchSelect *imod</b>\n\n
	Points to the IPatchSelect instance (generally this is a modifier).\n\n
	<b>TimeValue t</b>\n\n
	The current time at which the call is made. */
	virtual void SetVertSel(BitArray &set, IPatchSelect *imod, TimeValue t)=0;
	/*! \remarks Sets the edge selection of the modifier.
	\par Parameters:
	<b>BitArray \&set</b>\n\n
	There is one bit for each edge. Bits that are 1 indicate the edge should be
	selected.\n\n
	<b>IPatchSelect *imod</b>\n\n
	Points to the IPatchSelect instance (generally this is a modifier).\n\n
	<b>TimeValue t</b>\n\n
	The current time at which the call is made. */
	virtual void SetEdgeSel(BitArray &set, IPatchSelect *imod, TimeValue t)=0;
	/*! \remarks Sets the patch selection of the modifier.
	\par Parameters:
	<b>BitArray \&set</b>\n\n
	There is one bit for each patch. Bits that are 1 indicate the patch should
	be selected.\n\n
	<b>IPatchSelect *imod</b>\n\n
	Points to the IPatchSelect instance (generally this is a modifier).\n\n
	<b>TimeValue t</b>\n\n
	The current time at which the call is made. */
	virtual void SetPatchSel(BitArray &set, IPatchSelect *imod, TimeValue t)=0;

	virtual GenericNamedSelSetList & GetNamedVecSelList ()=0;
	/*! \remarks Returns a reference to a class used for manipulating the
	lists of vertex level named selection sets associated with this modifier.
	*/
	virtual GenericNamedSelSetList & GetNamedVertSelList ()=0;
	/*! \remarks Returns a reference to a class used for manipulating the
	lists of edge level named selection sets associated with this modifier. */
	virtual GenericNamedSelSetList & GetNamedEdgeSelList ()=0;
	/*! \remarks Returns a reference to a class used for manipulating the
	lists of patch level named selection sets associated with this modifier. */
	virtual GenericNamedSelSetList & GetNamedPatchSelList ()=0;
};

enum patchCommandMode { PcmAttach, PcmExtrude, PcmBevel, PcmBind, PcmCreate, PcmWeldTarget,
						PcmFlipNormal, PcmCopyTangent, PcmPasteTangent };
enum patchButtonOp    { PopUnbind, PopHide, PopUnhideAll, PopWeld, PopDelete, PopSubdivide,
						PopAddTri, PopAddQuad, PopDetach, PopSelectOpenEdges, PopBreak, 
						PopCreateShapeFromEdges, PopFlipNormal, PopUnifyNormal, PopSelectByID, 
						PopSelectBySG, PopClearAllSG, PopPatchSmooth, PopSelectionShrink, PopSelectionGrow,
						PopEdgeRingSel, PopEdgeLoopSel, PopShadedFaceToggle };
// LAM: added 9/3/00
enum patchUIParam {  };

/*! \sa  Class Animatable, <a href="ms-its:3dsmaxsdk.chm::/patches_root.html">Working with Patches</a>.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class provides an interface to the command modes and button press
operations of the Editable Patch object. To obtain a pointer to this class use
the method <b>Animatable::GetInterface()</b> passing <b>I_PATCHOPS</b>.\n\n
For example:\n\n
<b>IPatchOps *ipo = (IPatchOps*)anim-\>GetInterface(I_PATCHOPS));</b>\n\n
<b>ipo-\>StartCommandMode(PcmAttach);</b>  */
class IPatchOps: public MaxHeapOperators				// accessed via GetInterface(I_PATCHOPS)
{
public:
	// start up interactive command mode, uses mode enum above
	/*! \remarks Begins the specified interactive command mode.
	\par Parameters:
	<b>patchCommandMode mode</b>\n\n
	<b>PcmAttach</b>\n\n
	<b>PcmExtrude</b>\n\n
	<b>PcmBevel</b>\n\n
	<b>PcmCreate</b>\n\n
	<b>PcmWeldTarget</b>\n\n
	<b>PcmFlipNormal</b>\n\n
	<b>PcmBind</b> */
	virtual void StartCommandMode(patchCommandMode mode)=0;
	// perform button op, uses op enum above
	/*! \remarks Performs the same operation as a button press inside the
	Editable Patch UI.
	\par Parameters:
	<b>patchButtonOp opcode</b>\n\n
	The button operation to execute. One of the following values:\n\n
	<b>PopBind</b>\n\n
	<b>PopUnbind</b>\n\n
	<b>PopHide</b>\n\n
	<b>PopUnhideAll</b>\n\n
	<b>PopWeld</b>\n\n
	<b>PopDelete</b>\n\n
	<b>PopSubdivide</b>\n\n
	<b>PopAddTri</b>\n\n
	<b>PopAddQuad</b>\n\n
	<b>PopDetach</b> */
	virtual void ButtonOp(patchButtonOp opcode)=0;
// LAM: added 9/3/00
	// UI controls access
	/*! \remarks	This method allows you to get the edit patch parameters from the command
	panel. Currently not in use.
	\par Parameters:
	<b>patchUIParam uiCode</b>\n\n
	This enum is currently empty.\n\n
	<b>int \&ret</b>\n\n
	The returned value.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void GetUIParam (patchUIParam uiCode, int & ret) { }
	/*! \remarks	This method allows you to set the edit patch parameters from the command
	panel. Currently not in use.
	\par Parameters:
	<b>patchUIParam uiCode</b>\n\n
	This enum is currently empty.\n\n
	<b>int val</b>\n\n
	The value to set.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void SetUIParam (patchUIParam uiCode, int val) { }
	/*! \remarks	This method allows you to get the edit patch parameters from the command
	panel. Currently not in use.
	\par Parameters:
	<b>patchUIParam uiCode</b>\n\n
	This enum is currently empty.\n\n
	<b>float \&ret</b>\n\n
	The returned value.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void GetUIParam (patchUIParam uiCode, float & ret) { }
	/*! \remarks	This method allows you to set the edit patch parameters from the command
	panel. Currently not in use.
	\par Parameters:
	<b>patchUIParam uiCode</b>\n\n
	This enum is currently empty.\n\n
	<b>float val</b>\n\n
	The value to set.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void SetUIParam (patchUIParam uiCode, float val) { }
};

#pragma warning(pop)

//----------------------------------------------------------------
// Access to the new Assign Vertex Color utility - MAB - 6/04/03

#define APPLYVC_UTIL_CLASS_ID	Class_ID(0x6e989195, 0x5dfb41b7)
#define IASSIGNVERTEXCOLORS_INTERFACE_ID Interface_ID(0x4f913fd8, 0x422a32af)

namespace LightingModel
{
   enum LightingModel {		  //!< The lighting model to use when assigning vertex colors
      kLightingOnly = 0,      //!< Store lighting only
      kShadedLighting = 1,    //!< Store shaded color with lighting
      kShadedOnly = 2         //!< Store shaded color without lighting
   };
}

/*!
 \brief The IAssignVertexColors is a static interface to the Assign Vertex Color utility.

 It can be obtained as follows:
 \code
 IAssignVertexColors* avc = (IAssignVertexColors*)GetCOREInterface(IASSIGNVERTEXCOLORS_INTERFACE_ID);
 \endcode
*/ 
class IAssignVertexColors : public FPStaticInterface {
public:
	DECLARE_DESCRIPTOR( IAssignVertexColors );

   typedef LightingModel::LightingModel LightingModel;

	//! \brief The options used when calculating the vertex colors
	class Options: public MaxHeapOperators {
	public:
		int mapChannel;
		bool mixVertColors; //!< face colors is false, or mixed vertex colors if true
		bool castShadows;
		bool useMaps;
		bool useRadiosity;
		bool radiosityOnly;
		LightingModel lightingModel;
	};

	//! \brief Performs a lighting calculation.
	/*! The result is stored in the given VertexPaint modifier (if provided) or creates
		a new modifier instanced across the given nodes.
	 \param[in, out] nodes A pointer to an array of nodes pointers to apply the iVertexPaint interface to.
		This list of nodes will be filtered to include only nodes with geometry objects on return.
	 \param[in] iVertexPaint An instance of a ReferenceTarget derived class which supports the IVertexPaint interface.
		If this parameter is null, an instance of the standard Max Vertex Paint modifier will be created and assigned to the nodes.
	 \return 0 if the vertex paint interface could not be applied, else 1. */
	virtual int		ApplyNodes( Tab<INode*>* nodes, ReferenceTarget* iVertexPaint=NULL ) = 0;
	
	//! \brief The Get/SetOptions() methods control the parameters used for the lighting calculation, as shown in the UI of the utility. 
	virtual void	GetOptions( Options& options ) = 0;
	//! \brief The Get/SetOptions() methods control the parameters used for the lighting calculation, as shown in the UI of the utility. 
	virtual void	SetOptions( Options& options ) = 0;
};

#define IASSIGNVERTEXCOLORS_R7_INTERFACE_ID Interface_ID(0x77870f30, 0x4ed82b62)

/*! 
\brief This class extends IAssignVertexColors to include more options.
\sa Class IAssignVertexColors
Options2 has been extended to add the reuseillumination boolean, indicating
whether we reuse illumination or we render the direct lights.
*/
class IAssignVertexColors_R7 : public IAssignVertexColors {
public:
	DECLARE_DESCRIPTOR( IAssignVertexColors_R7 );

	//! \brief The options used when calculating the vertex colors
	class Options2 : public IAssignVertexColors::Options
	{
	public:
		bool reuseIllumination;			//!< Reuse Illumination or render direct lights
	} ;

	//! \brief The Get/SetOptions() methods control the parameters used for the lighting calculation, as shown in the UI of the utility. 
	virtual void	GetOptions2( Options2& options ) = 0;
	//! \brief The Get/SetOptions() methods control the parameters used for the lighting calculation, as shown in the UI of the utility. 
	virtual void	SetOptions2( Options2& options ) = 0;
};

//----------------------------------------------------------------
// Access to the new Vertex Paint modifier in 3ds max 6 - MAB - 5/15/03

#define PAINTLAYERMOD_CLASS_ID	Class_ID(0x7ebb4645, 0x7be2044b)
#define IVERTEXPAINT_INTERFACE_ID Interface_ID(0x3e262ef9, 0x220e7190)

/*!
 \brief The IVertexPaint interface provides a way to set the vertex colors held in a VertexPaint modifier. 

 The interface can be obtained as follows:
 \code
 ReferenceTarget* vertexPaintMod = ...;
 IVertexPaint* ivertexPaint = (IVertexPaint*)vertexPaintMod->GetInterface(IVERTEXPAINT_INTERFACE_ID);
 \endcode
 
 \note The class ID for the new VertexPaint in R6 and up is PAINTLAYERMOD_CLASS_ID, different from the 
 legacy modifier. The legacy modifier does not support IVertexPaint
*/
class IVertexPaint : public FPMixinInterface {
public:
	struct FaceColor: public MaxHeapOperators { //!< Stores a color for each vertex of a triangle face.
		Color colors[3];
	};
	typedef Tab<Color*> VertColorTab;
	typedef Tab<FaceColor*> FaceColorTab;
	typedef IAssignVertexColors::Options Options;

	//! \brief Set the colors by providing one color per vert. 
	/*! If the vertex paint modifier is applied on a poly object, you should provide colors 
	 based on the tri-mesh version of the object, as this is the object-type operated on by the modifier.
	 \param [in] node The node to apply the vertex painting to
	 \param [in] vertColors A table of colors, this should supply one for each vertex
	 \return 0 if the colors could not be applied, else 1	*/
	virtual int		SetColors( INode* node, VertColorTab& vertColors ) = 0;
	
	//! \brief Set colors by providing three colors per triangle. 
	/*! If the vertex paint modifier is applied on a poly object, you should provide colors 
	 based on the tri-mesh version of the object, as this is the object-type operated on by the modifier.
	 \param [in] node The node to apply the vertex painting to
	 \param [in] faceColors A table of face colors, each with 3 colors for each vertex.
	 \return 0 if the colors could not be applied, else 1	*/
	virtual int		SetColors( INode* node, FaceColorTab& faceColors ) = 0;
	//! \brief The Get/SetOptions() methods control the parameters used for the lighting calculation, as shown in the UI of the utility. 	
	virtual void	GetOptions( Options& options ) = 0;
	//! \brief The Get/SetOptions() methods control the parameters used for the lighting calculation, as shown in the UI of the utility. 
	virtual void	SetOptions( Options& options ) = 0;
};

#define IVERTEXPAINT_R7_INTERFACE_ID Interface_ID(0x4ea42df5, 0x44a35866)

class IVertexPaint_R7 : public IVertexPaint {
public:
	typedef IAssignVertexColors_R7::Options2 Options2;

	// Assumes colors were calculated on the same object as seen by the modifier (converted to a tri-mesh)
	virtual void	GetOptions2( Options2& options ) = 0;
	virtual void	SetOptions2( Options2& options ) = 0;
};


// Interface for the TimeSlider
#define TIMESLIDER_INTERFACE Interface_ID(0x829e89e5, 0x878ef6e5)

//! \brief This class represents the interface for the time slider.
/*! This class is available in release 4.0 and later only.\n\n
The interface ID is
defined as <b>TIMESLIDER_INTERFACE</b>.\n\n
All methods of this class are Implemented by the System.  */
class ITimeSlider : public FPStaticInterface {
public:
	/*! \remarks This method allows you to set the visibility flags of the
	time slider.
	\par Parameters:
	<b>BOOL bVisible</b>\n\n
	TRUE to set the time slider visible, otherwise FALSE.\n\n
	<b>BOOL bPersistent = TRUE</b>\n\n
	If this parameter is set to TRUE the value will be written to the ini file
	so the visibility switch will be persistent between sessions. Set this
	parameter to FALSE if you wish to reset the visibility switch the next time
	3ds Max starts. */
	virtual void	SetVisible(BOOL bVisible, BOOL bPersistent = TRUE) = 0;
	/*! \remarks This method returns TRUE if the time slider is visible,
	otherwise FALSE. */
	virtual BOOL	IsVisible() = 0;
};

// Interface for the StatusPanel
#define STATUSPANEL_INTERFACE Interface_ID(0x94357f0, 0x623e71c2)

class IStatusPanel : public FPStaticInterface {
public:
	virtual void	SetVisible(BOOL bVisible) = 0;
	virtual BOOL	IsVisible() = 0;
};

// Interface for the BMP I/O plug-in
#define BMPIO_INTERFACE Interface_ID(0x374f288f, 0x19e460d6)
// Valid types are, BMM_PALETTED, BMM_TRUE_24 and BMM_NO_TYPE (=BMM_TRUE_24)

//! \brief This class represents the interface for the Bitmap IO BMP format.
/*! This class is available in release 4.0 and later only.\n\n
All methods of this class are Implemented by the System.\n\n
The interfaceID is defined as <b>BMPIO_INTERFACE</b>. Through C++ you can access the
interface the following way:
\code
ClassEntry* ce = ip->GetDllDirectory()->ClassDir().FindClassEntry(BMM_IO_CLASS_ID, Class_ID(BMPCLASSID,0));
if (ce)
{
	ClassDesc* cd = ce->FullCD();
	if (cd) {
		IBitmapIO_Bmp* ib = (IBitmapIO_Bmp*)cd->GetInterface(BMPIO_INTERFACE);
		if (ib) {
			int i = ib->GetType();
			ib->SetType(i);
		}
	}
}
\endcode  */
class IBitmapIO_Bmp : public FPStaticInterface {
public:
	/*! \remarks This method returns the format type, which is one of the
	following; <b>BMM_PALETTED</b>, <b>BMM_TRUE_24</b> and <b>BMM_NO_TYPE</b>
	(which is equivalent to <b>BMM_TRUE_24</b>). */
	virtual int		GetType() = 0;
	/*! \remarks This method allows you to set the format type.
	\par Parameters:
	<b>int type</b>\n\n
	The type, which is one of the following; <b>BMM_PALETTED</b>,
	<b>BMM_TRUE_24</b> and <b>BMM_NO_TYPE</b> (which is equivalent to
	<b>BMM_TRUE_24</b>). */
	virtual void	SetType(int type) = 0;
	};

// Interface for the JPeg I/O plug-in
#define JPEGIO_INTERFACE Interface_ID(0x466c7964, 0x2db94ff2)
/*! \sa  Class FPStaticInterface\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the interface for the Bitmap IO JPG format. The interface
ID is defined as <b>BMPIO_INTERFACE</b>.\n\n
All methods of this class are Implemented by the System.  */
class IBitmapIO_Jpeg : public FPStaticInterface {
public:
	/*! \remarks This method returns the quality level of the output image. */
	virtual int		GetQuality() = 0;
	/*! \remarks This method allows you to set the quality level of the output
	image.
	\par Parameters:
	<b>int quality</b>\n\n
	The quality level. */
	virtual void	SetQuality(int quality) = 0;
	/*! \remarks This method returns the smoothing level of the output image.
	*/
	virtual int		GetSmoothing() = 0;
	/*! \remarks This method allows you set the smoothing level of the output
	image.
	\par Parameters:
	<b>int smoothing</b>\n\n
	The smoothing level. */
	virtual void	SetSmoothing(int smoothing) = 0;
	};

// Interface for the Png I/O plug-in
#define PNGIO_INTERFACE Interface_ID(0x1d7c41db, 0x328c1142)
/*! \sa  Class FPStaticInterface\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the interface for the Bitmap IO PNG format. The interface
ID is defined as <b>BMPIO_INTERFACE</b>.\n\n
All methods of this class are Implemented by the System.  */
class IBitmapIO_Png : public FPStaticInterface {
public:
	// Valid type are:
	// BMM_PALETTED
	// BMM_TRUE_24
	// BMM_TRUE_48
	// BMM_GRAY_8
	// BMM_GRAY_16
	/*! \remarks This method returns the bitmap type, which is one of the
	following; <b>BMM_PALETTED, BMM_TRUE_24, BMM_TRUE_48, BMM_GRAY_8</b>, or
	<b>BMM_GRAY_16</b>. */
	virtual int		GetType() = 0;
	/*! \remarks This method allows you to set the bitmap type.
	\par Parameters:
	<b>int type</b>\n\n
	One of the following; <b>BMM_PALETTED, BMM_TRUE_24, BMM_TRUE_48,
	BMM_GRAY_8</b>, or <b>BMM_GRAY_16</b>. */
	virtual void	SetType(int type) = 0;
	/*! \remarks This method returns TRUE if the alpha flag is set, otherwise
	FALSE. */
	virtual BOOL	GetAlpha() = 0;
	/*! \remarks This method allows you to set the alpha flag.
	\par Parameters:
	<b>BOOL alpha</b>\n\n
	TRUE to set the alpha flag, otherwise FALSE. */
	virtual void	SetAlpha(BOOL alpha) = 0;
	/*! \remarks This method returns TRUE if the interlaced flag is set,
	otherwise FALSE. */
	virtual BOOL	GetInterlaced() = 0;
	/*! \remarks This method allows you to set the interlaced flag.
	\par Parameters:
	<b>BOOL interlaced</b>\n\n
	TRUE to set the interlaced flag, otherwise FALSE. */
	virtual void	SetInterlaced(BOOL interlaced) = 0;
	};

// Interface for the Targa I/O plug-in
#define TGAIO_INTERFACE Interface_ID(0x21d673b7, 0x1d34198d)
/*! \sa  Class FPStaticInterface,  Class BitmapIO\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the interface for the Bitmap IO TGA format. The interface
ID is defined as <b>BMPIO_INTERFACE</b>.\n\n
All methods of this class are Implemented by the System.  */
class IBitmapIO_Tga : public FPStaticInterface {
public:
	// 16, 24, 32
	/*! \remarks This method returns the color depth, which would be 16, 24,
	or 32. */
	virtual int		GetColorDepth() = 0;
	/*! \remarks This method allows you to set the color depth.
	\par Parameters:
	<b>int bpp</b>\n\n
	The color depth, being either 16, 24, or 32. */
	virtual void	SetColorDepth(int bpp) = 0;
	/*! \remarks This method returns TRUE if the compression flag is set,
	otherwise FALSE. */
	virtual BOOL	GetCompressed() = 0;
	/*! \remarks This method allows you to set the compression flag.
	\par Parameters:
	<b>BOOL compressed</b>\n\n
	TRUE to set the compression flag, otherwise FALSE. */
	virtual void	SetCompressed(BOOL compressed) = 0;
	/*! \remarks This method returns TRUE if the alpha split flag is set,
	otherwise FALSE. */
	virtual BOOL	GetAlphaSplit() = 0;
	/*! \remarks This method allows you to set the alpha split flag.
	\par Parameters:
	<b>BOOL alphaSplit</b>\n\n
	TRUE to set the alpha split flag, otherwise FALSE. */
	virtual void	SetAlphaSplit(BOOL alphaSplit) = 0;
	/*! \remarks This method returns TRUE if the premultiplied alpha flag is
	set, otherwise FALSE. */
	virtual BOOL	GetPreMultAlpha() = 0;
	/*! \remarks This method allows you to set the premultiplied alpha flag.
	\par Parameters:
	<b>BOOL preMult</b>\n\n
	TRUE to set the premultiplied alpha flag, otherwise FALSE. */
	virtual void	SetPreMultAlpha(BOOL preMult) = 0;
	};

// Interface for the RLA I/O plug-in
//Fetching the interface using either RLAIO_INTERFACE or RPFIO_INTERFACE returns an object
//of the same class.  But the object affects only the RLA or RPF settings accordingly.
//The "RPF-specific" methods have undefined behavior unless used with an RPFIO_INTERFACE object
#define RLAIO_INTERFACE Interface_ID(0x282c2f79, 0x68f7373d)
#define RPFIO_INTERFACE Interface_ID(0x25a87871, 0x2e265a49)

//! \brief The IBitmapIO_RLA class expands upon the previous IBitmapIO classes
/*! \sa Class IBitmapIO
\par Description:
The IBitmapIO_RLA class expands upon the previous IBitmapIO classes for .bmp, 
.jpeg, .png and .tga. Like these others, it is a static interface; the code to 
fetch the interface is given below. The interface does not directly change the
configuration of an existing BitmapIO. Instead, these methods set the default 
values, which will be used the next time an RLA or RPF file is saved. 
The defaults will also appear in the bitmap configuration dialog if the user saves
an RLA or RPF file.

\par Usage:
Both RLA and RPF have the same interface class, IBitmapIO_RLA, but some of the methods
of this class are RPF-specific, giving undefined behavior if used on an RLA instance 
of the class.
\code
DllDir* dllDir = GetCOREInterface()->GetDllDirectory();
ClassDirectory& classDir = dllDir->ClassDir();
ClassEntry* classEntry = classDir.FindClassEntry( BMM_IO_CLASS_ID, Class_ID(RPFCLASSID,0));
ClassDesc* classDesc = classEntry->FullCD();
return classDesc->GetInterface( RPFIO_INTERFACE ); 
// You can substitute RLACLASSID and RLAIO_INTERFACE if you want the RLA interface.
\endcode
*/
class IBitmapIO_RLA : public FPStaticInterface {
public:
	// 8, 16, 32
	virtual int		GetColorDepth() = 0;
	virtual void	SetColorDepth(int bpp) = 0;
	virtual BOOL	GetStoreAlpha() = 0;
	virtual void	SetStoreAlpha(BOOL storeAlpha) = 0;
	virtual BOOL	GetPremultAlpha() = 0;
	virtual void	SetPremultAlpha(BOOL preMult) = 0;

	virtual MSTR	GetDescription() = 0;
	virtual void	SetDescription(MSTR description) = 0;
	virtual MSTR	GetAuthor() = 0;
	virtual void	SetAuthor(MSTR author) = 0;

	virtual BOOL	GetZChannel() = 0;
	virtual void	SetZChannel(BOOL b) = 0;
	virtual BOOL	GetMtlIDChannel() = 0;
	virtual void	SetMtlIDChannel(BOOL b) = 0;
	virtual BOOL	GetNodeIDChannel() = 0;
	virtual void	SetNodeIDChannel(BOOL b) = 0;
	virtual BOOL	GetUVChannel() = 0;
	virtual void	SetUVChannel(BOOL b) = 0;
	virtual BOOL	GetNormalChannel() = 0;
	virtual void	SetNormalChannel(BOOL b) = 0;
	virtual BOOL	GetRealpixChannel() = 0;
	virtual void	SetRealpixChannel(BOOL b) = 0;
	virtual BOOL	GetCoverageChannel() = 0;
	virtual void	SetCoverageChannel(BOOL b) = 0;

	// RPF-specific methods
	virtual BOOL	GetNodeRenderIDChannel() = 0;
	virtual void	SetNodeRenderIDChannel(BOOL b) = 0;
	virtual BOOL	GetColorChannel() = 0;
	virtual void	SetColorChannel(BOOL b) = 0;
	virtual BOOL	GetTranspChannel() = 0;
	virtual void	SetTranspChannel(BOOL b) = 0;
	virtual BOOL	GetVelocChannel() = 0;
	virtual void	SetVelocChannel(BOOL b) = 0;
	virtual BOOL	GetWeightChannel() = 0;
	virtual void	SetWeightChannel(BOOL b) = 0;
	virtual BOOL	GetMaskChannel() = 0;
	virtual void	SetMaskChannel(BOOL b) = 0;
};

// Interface for the Rgb I/O plug-in
#define RGBIO_INTERFACE Interface_ID(0x096c2424, 0x21b84dae)
/*! \sa  Class FPStaticInterface\n\n
\par Description:
This class is available in 3ds Max 2011 and later only.\n\n
This class represents the interface for the Bitmap IO RGB format. The interface
ID is defined as <b>BMPIO_INTERFACE</b>.\n\n
All methods of this class are Implemented by the System.  */
class IBitmapIO_Rgb : public FPStaticInterface 
{
public:
	/*! \remarks This method returns the bytes per channel info of the output image. */
	virtual int		GetBytesPerChannel() = 0;
	/*! \remarks This method allows you to set the bytes per channel of the output
	image.
	\par Parameters:
	<b>int bytesPerChannel</b>\n\n
	The bytesPerChannel can only be 1 or 2. */
	virtual void	SetBytesPerChannel(int bytesPerChannel) = 0;
	/*! \remarks This method tells whether the output image should have an alpha channel or not
	*/
	virtual BOOL	GetAlphaChannelState() = 0;
	/*! \remarks This method allows you set whether the output image should have an alpha channel or not
	\par Parameters:
	<b>bool bHasAlpha</b>\n\n
	true if with alpha channel, false otherwise */
	virtual void	SetAlphaChannelState(BOOL bHasAlpha) = 0;
	/*! \remarks This method tells whether the output image should be compressed or not
	*/
	virtual BOOL	GetCompressionState() = 0;
	/*! \remarks This method allows you set whether the output image should be compressed or not
	\par Parameters:
	<b>bool bCompressed</b>\n\n
	true if output image should be compressed, false otherwise */
	virtual void	SetCompressionState(BOOL bCompressed) = 0;
};

//IUnReplaceableControl --added 08/03/04 MZ. Added for exposetransform helper object.
//This interface serves 2 related functions.  First, if this interface is present, pb2's and wires won't try to replace this controller
///Basically it's a backwards-compatabile fix for the fact that the 'IsReplaceable' control flag isn't used by PB2's.  Secondly the class provides
//a function to replace it's default 'clone' method. This is currently also used in wiring when a wire is unconnected.

//! \brief IUnReplaceableControl is a new interface which if present lets the internal system know that the controller can't be replaced. 
/*! \par Description:
	This interface is mainly used by the wire controller UI. \n\n
	For example, a controller that implements this interface can't be replaced by a wire controller, it can only participate 
	in one way wires where this controller is the master. In addition when the controller is replaced and collapsed the
	Control returned by the GetReplacementClone function will be used to replace the wire.\n\n
	The interface is retrieved by calling GetInterface(I_UNREPLACEABLECTL). 
	\par Examples: 
	The expose transform controller (see maxsdk\samples\objects\helpers\exposetransform) implements this interface.
*/
class IUnReplaceableControl: public MaxHeapOperators
{
public:
	virtual ~IUnReplaceableControl(){};
	virtual Control * GetReplacementClone()=0;
};

//! \brief IEulerControl is an interface for euler controllers so we can get/set their xyz ordering.

//! The interface is returned by calling GetInterface(I_EULERCTRL).
class IEulerControl: public MaxHeapOperators {
	public:
		//! \brief Get/Set the XYZ Order of the euler controller.
		
		//! Returns the order of the xyz eulers as defined in euler.h
		virtual int GetOrder()=0;

		//! Sets the order of the xyz eulers as defined in euler.h
		virtual void SetOrder(int newOrder)=0;
	};

class AKey;
#define I_ATTACHCTRL	Interface_ID(0x199a6b11, 0x5ca27621) // (IAttachControl*) - for attach controllers

//! \brief IAttachCtrl is an interface for getting/setting properties on the Attach controller.
/*! The interface is returned by calling GetInterface(I_ATTACHCTRL).
*/
class IAttachCtrl : public BaseInterface  {
public:
	//! \brief Set the Node to attach to
	/*! \param[in] node The Node to attach to
	\return True if the node could be set as the attached to node
	*/
	virtual BOOL SetObject(INode *node)=0;

	//! \brief Get the Node to attached to
	/*! \return The node attached to
	*/
	virtual INode* GetObject()=0;

	//! \brief Get a Attach controller key
	/*! \param[in] index The key index
	\return The indexed Attach controller key, NULL if index invalid
	*/
	virtual AKey* GetKey(int index)=0;

	//! \brief Set a attach controller key, creating the key if necessary
	/*! \param[in] t The time of the key
	\param[in] faceIndex The face index on the node's mesh to attach to
	\param[in] bary The barycentric coordinates on the face
	*/
	virtual void SetKeyPos(TimeValue t, DWORD faceIndex, Point3 bary)=0;

	//! \brief Get Attach controller Align property
	/*! \return The Align state
	*/
	virtual BOOL GetAlign()=0;

	//! \brief Set Attach controller Align property
	/*! \param[in] align New Align state
	*/
	virtual void SetAlign(BOOL align)=0;

	//! \brief Get Attach controller Manual Update property
	/*! \return The Manual Update state
	*/
	virtual BOOL GetManualUpdate()=0;

	//! \brief Set Attach controller Manual Update property
	/*! \param[in] manUpdate New Manual Update state
	*/
	virtual void SetManualUpdate(BOOL manUpdate)=0;

	//! \brief Invalidate the Attach controller and update its ui if displayed
	/*! \param[in] forceUpdate If true, forces an update of the controller even if manual update is off.
	*/
	virtual void Invalidate(BOOL forceUpdate = FALSE)=0;
};


//! Interface for interactions between 'systems', in particular biped, and the max xref core.
//! This interface allows for xrefs to notify systems when key operations occur so they may act accordingly.
//! The interface ID is I_SYSTEM_XREF defined in AnimatableInterfaceIDs.h.
class ISystemXRef: public MaxHeapOperators  {
public:
	//! The system is created as an object xref.
	virtual void SystemIsObjectXRef() = 0;

	//! The system is no longer an object xref. Most likely because it has been merged into the scene.
	virtual void SystemNoLongerObjectXRef() = 0;

	//! The system is updated. Passed in this the head node of the system that's being xref'd in.
	//! param[in] headNode - the first node returned from Animatable::GetSystemNodes
	virtual void  SystemUpdated(INode *headNode) = 0;
};


