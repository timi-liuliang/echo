//
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.  
//
//
#pragma once
#include <WTypes.h>

/** \defgroup Display_Flags Display Flags
* The arguments for the "dispFlags" parameter in GraphicsWindow::processWireFaces(), and also for 
* Mesh::dispFlags and BezierShape::dispFlags. \see GraphicsWindow::processWireFaces() \see Mesh::dispFlags */
//@{
#define DISP_VERTTICKS		(1<<0)	//!< Display vertices as tick marks.
#define DISP_BEZHANDLES		(1<<1)	//!< Display Bezier handles.
#define DISP_OBJSELECTED	(1<<8)	//!< Mimics COMP_OBJSELECTED in mesh.h 
#define DISP_SELVERTS		(1<<10)	//!< Display selected vertices. 
#define DISP_SELFACES		(1<<11)	//!< Display selected faces. 
#define DISP_SELSEGMENTS	(1<<11) //!< Display selected segments.
#define DISP_SELPATCHES		(1<<11) //!< Display selected patches.
#define DISP_SELEDGES		(1<<12)	//!< Display selected edges.
#define DISP_SELPOLYS		(1<<13)	//!< Display selected polygons.
#define DISP_SELHANDLES		(1<<14) //!< Display selected handles.
#define DISP_UNSELECTED		(1<<14)	//!< Used by loft -- Shape unselected
#define DISP_SELECTED		(1<<15)	//!< Used by loft -- Shape selected
#define DISP_LATTICE		(1<<16)
#define DISP_ATSHAPELEVEL	(1<<16)	//!< Used by loft -- Shape at current level 
#define DISP_VERT_NUMBERS	(1<<17) //!< When this bit is set, and vertex ticks are being displayed, the shape is drawn with vertex numbers in addition to the ticks.
#define DISP_VERTS			(1<<17)
#define DISP_VERT_NUMBERS_SELONLY	(1<<18) //!< When this bit is set and the DISP_VERT_NUMBERS bit is set, only the numbers of selected vertices are displayed.
#define DISP_SPLINES_ORTHOG (1<<25)  //!< Not displayed, but splines to be created orthogonally
//@}

/** \defgroup Steps_Arguments Steps Arguments  
 * Arguments for the "steps" parameter in MakePolyShape(). The third parameter in ShapeObject::MakePolyShape()  
 * is the number of steps, and can accept one of the following arguments for non-fixed number of steps. Any positive 
 * argument will be interpreted as fixed number of steps. The child classes of ShapeObject who implements this function 
 * (LinearShape, SimpleShape, SimpleSpline, SplineShape, MSPluginShape, and MSShapeXtnd) also interpret these in the same 
 * way. \see ShapeObject::MakePolyShape()
 */
//@{
#define PSHAPE_BUILTIN_STEPS -2		//!< Uses the shape's built-in steps/adaptive settings (default) 
#define PSHAPE_ADAPTIVE_STEPS -1	//!< Forces adaptive steps 
//@}


/** \defgroup Selection_Level_Flags Selection Level Flags
 * Arguments for BezierShape::selLevel */
//@{
#define SHAPE_OBJECT		(1<<0) //!< Object level selection. 
#define SHAPE_SPLINE		(1<<1) //!< Spline level selection (a single polygon within the shape).
#define SHAPE_SEGMENT		(1<<2) //!< Segment level selection.
#define SHAPE_VERTEX		(1<<3) //!< Vertex level selection.
//@}

// Arguments for the Flag parameter of the internal function BezierShape::SubObjectHitTest()
// these are the same bits used for object level.*/
#define SUBHIT_SHAPE_SELONLY	(1<<0)
#define SUBHIT_SHAPE_UNSELONLY	(1<<2)
#define SUBHIT_SHAPE_ABORTONHIT	(1<<3)
#define SUBHIT_SHAPE_SELSOLID	(1<<4)

#define SUBHIT_SHAPE_VERTS		(1<<24)
#define SUBHIT_SHAPE_SEGMENTS	(1<<25)
#define SUBHIT_SHAPE_POLYS		(1<<26)
#define SUBHIT_SHAPE_TYPEMASK	(SUBHIT_SHAPE_VERTS|SUBHIT_SHAPE_SEGMENTS|SUBHIT_SHAPE_POLYS)



/** \defgroup Interpolation_Parameter_Block_ Index_Arguments Interpolation Parameter Block Index Arguments
 * SimpleSpline::ipblock index arguments, and also index arguments for ParamUIDesc::ParamUIDesc()
 * \see  ParamUIDesc::ParamUIDesc() \see SimpleSpline::ipblock */
//@{
#define IPB_STEPS		0
#define IPB_OPTIMIZE	1
#define IPB_ADAPTIVE	2
//@}

/** \defgroup Default_Interpolation_Settings Default Interpolation Settings */
//@{
#define DEF_STEPS 6
#define DEF_OPTIMIZE TRUE
#define DEF_ADAPTIVE FALSE
#define DEF_RENDERABLE FALSE
#define DEF_DISPRENDERMESH FALSE
//#define DEF_USEVIEWPORT FALSE
#define DEF_RENDERABLE_THICKNESS 1.0f
#define DEF_RENDERABLE_SIDES 12
#define DEF_RENDERABLE_ANGLE 0.0f
#define DEF_GENUVS FALSE
//@}

/** \defgroup Parameter_types_for_shape_interpolation Parameter types for shape interpolation
 * (Must match types in spline3d.h & polyshp.h) */
//@{
#define PARAM_SIMPLE 0		// Parameter space based on segments
#define PARAM_NORMALIZED 1	// Parameter space normalized to curve length
//@}

/** \defgroup Driver_Types Driver types */
//@{
#define GW_DRV_RENDERER		0 
#define GW_DRV_DEVICE		1 
//@}

/// \internal \defgroup Internally_used_graphics_constants Internally Used Graphics Constants
//@{
#define GW_HEIDI			0
#define GW_OPENGL			1
#define GW_DIRECT3D			2
#define GW_HEIDI3D			3
#define GW_NULL				4
#define GW_CUSTOM			5
#define GW_NITROUS			6
//@}

/** \defgroup Region_Types_Used_for_Hit_Testing Region Types
 * (for built-in hit-testing) */
//@{
#define POINT_RGN	0x0001
#define	RECT_RGN	0x0002
#define CIRCLE_RGN	0x0004
#define FENCE_RGN	0x0008
//@}

/// \defgroup Region_Direction Region Directions
//@{
#define RGN_DIR_UNDEF	-1
#define RGN_DIR_RIGHT	0	
#define RGN_DIR_LEFT	1
//@}

/// \defgroup PI_Constants Pi Constants 
//@{
const double pi        = 3.141592653589793;			//!< The constant PI defined as a double.
const double piOver180 = 3.141592653589793 / 180.0; //!< The constant value of PI divided by 180.0 defined as a double. Useful for converting from degrees to radians.
//@}

/// \defgroup Auto_Crossing_Selection_Mode Auto-Crossing Selection Mode 
/// Used with Interface::SetSelectionType(), setAutoCross(), and getAutoCross().
//@{
#define AC_DIR_RL_CROSS		0 //!< Right to left implies crossing (AutoCAD compatible)
#define AC_DIR_LR_CROSS		1 //!< Left to right implies crossing (AutoCAD incompatible)
//@}

/// \defgroup Custom_Windows_Messages Custom Windows Messages
//@{
#define WM_SHUTDOWN			(WM_USER+2001) 
#define WM_INIT_COMPLETE	(WM_USER+2002)
//@}

/// \defgroup General_Purpose_Graphics_Constants General Purpose Graphics Constants
//@{
#define GW_MAX_FILE_LEN		128 
#define GW_MAX_CAPTION_LEN	128 
#define GW_MAX_VERTS		32
#define GFX_MAX_STRIP		100
#define GFX_MAX_TEXTURES	8
//@}

/// \defgroup Rendering_Modes Rendering Modes
//@{
#define GW_NO_ATTS				0x0000000
#define GW_WIREFRAME			0x0000001
#define GW_ILLUM				0x0000002
#define GW_FLAT					0x0000004
#define GW_SPECULAR				0x0000008
#define GW_TEXTURE				0x0000010
#define GW_Z_BUFFER				0x0000020
#define GW_PERSP_CORRECT		0x0000040
#define GW_POLY_EDGES			0x0000080
#define GW_BACKCULL				0x0000100
#define GW_TWO_SIDED			0x0000200
#define GW_COLOR_VERTS			0x0000400
#define GW_SHADE_CVERTS			0x0000800
#define GW_PICK					0x0001000
#define GW_BOX_MODE				0x0002000
#define GW_ALL_EDGES			0x0004000
#define GW_VERT_TICKS			0x0008000
#define GW_SHADE_SEL_FACES		0x0010000
#define GW_TRANSPARENCY			0x0020000
#define GW_TRANSPARENT_PASS		0x0040000
#define GW_EMISSIVE_VERTS		0x0080000
#define GW_ALL_OPAQUE			0x0100000
#define GW_EDGES_ONLY			0x0200000
#define GW_CONSTANT				0x0400000
#define GW_HIDDENLINE			0x0800000 //!< Similar to GW_CONSTANT but the shade color will be the color of the background.
#define GW_BLENDING				0x1000000
#define GW_DEPTHWRITE_DISABLE	0x2000000 //!< Disables writing into the depth buffer
//@}

/// \defgroup Lighting_Constants Lighting Constants 
//@{
#define GW_LIGHTING			(GW_ILLUM | GW_SPECULAR)
//@}

/// \defgroup Spotlight_Shapes Spotlight Shapes
//@{
#define GW_SHAPE_RECT		0
#define GW_SHAPE_CIRCULAR	1
//@}

/// \defgroup Texture_Tiling_Modes Texture Tiling Modes
//@{
#define GW_TEX_NO_TILING	0
#define GW_TEX_REPEAT		1
#define GW_TEX_MIRROR		2
//@}


/// \defgroup Texture_Operations Texture Operations
//@{
#define GW_TEX_LEAVE				0	//!< Use the source pixel value
#define GW_TEX_REPLACE				1	//!< Use the texture pixel value
#define GW_TEX_MODULATE				2	//!< Multiply the source with the texture
#define GW_TEX_ADD					3	//!< Add the source and texture
#define GW_TEX_ADD_SIGNED			4	//!< Add the source and texture with an 0.5 subtraction
#define GW_TEX_SUBTRACT				5	//!< Subtract the source from the texture
#define GW_TEX_ADD_SMOOTH			6	//!< Add the source and the texture then subtract their product
#define GW_TEX_ALPHA_BLEND			7	//!< Alpha blend the texture with the source
#define GW_TEX_PREMULT_ALPHA_BLEND	8	//!< Alpha blend the the source with a premultiplied alpha texture
#define GW_TEX_ALPHA_BLEND2			9	//!< Alpha blend the the source with a premultiplied alpha texture with GL_MODULATE as the tex env instead of GL_DECAL.
//@}

/// \defgroup Texture_Scale_Factors Texture Scale Factors
//@{
#define GW_TEX_SCALE_1X		0	//!< Multiply the tex op result by 1
#define GW_TEX_SCALE_2X		1	//!< Multiply the tex op result by 2
#define GW_TEX_SCALE_4X		2	//!< Multiply the tex op result by 4
//@}

/// \defgroup Texture_Alpha_Sources Texture Alpha Sources
//@{
#define GW_TEX_ZERO			0	//!< Use no alpha value
#define GW_TEX_SOURCE		1	//!< Use the source alpha
#define GW_TEX_TEXTURE		2	//!< Use the texture alpha
#define GW_TEX_CONSTANT		3	//!< Use a constant BGRA color as an alpha
#define GW_TEX_PREVIOUS		4	//!< Use the previous texture stage alpha
//@}

/// \defgroup View_Volume_Clip_Flags View Volume Clip Flags
//@{
#define GW_LEFT_PLANE		0x0100
#define GW_RIGHT_PLANE		0x0200
#define GW_BOTTOM_PLANE		0x0400
#define GW_TOP_PLANE		0x0800
#define GW_FRONT_PLANE		0x1000
#define GW_BACK_PLANE		0x2000
#define GW_PLANE_MASK		0x3f00
//@}

/// \defgroup Edge_Styles Edge Styles
//@{
#define GW_EDGE_SKIP		0
#define GW_EDGE_VIS			1
#define GW_EDGE_INVIS		2
//@}

/// \defgroup Dual_Plane_Buffer_Types
//@{
#define BUF_F_BUFFER		0
#define BUF_Z_BUFFER		1
//@}

/// \defgroup GraphicsWindow__querySupport__Return_Values GraphicsWindow::querySupport() Return Values
/// These are the possible return values from the function GraphicsWindow::querySupport().
//@{
#define GW_DOES_SUPPORT			TRUE
#define GW_DOES_NOT_SUPPORT		FALSE
//@}

/// \defgroup GraphicsWindow__querySupport__Arguments GraphicsWindow::querySupport() Arguments
/// These are valid arguments to the function GraphicsWindow::querySupport().
//@{
#define GW_SPT_TXT_CORRECT		0	//!< Allow perspective correction to be toggled?
#define GW_SPT_GEOM_ACCEL		1	//!< Do 3D xforms, clipping, lighting thru driver?
#define GW_SPT_TRI_STRIPS		2	//!< Send down strips instead of individual triangles?
#define GW_SPT_DUAL_PLANES		3	//!< Allow dual planes to be used?
#define GW_SPT_SWAP_MODEL		4	//!< Update viewports with complete redraw on WM_PAINT?
#define GW_SPT_INCR_UPDATE		5	//!< Redraw only damaged areas on object move?
#define GW_SPT_1_PASS_DECAL		6	//!< Do decaling with only one pass?
#define GW_SPT_DRIVER_CONFIG	7	//!< Allow driver config dialog box?
#define GW_SPT_TEXTURED_BKG		8	//!< Is viewport background a texture?
#define GW_SPT_VIRTUAL_VPTS		9	//!< Are viewports bigger than the window allowed?
#define GW_SPT_PAINT_DOES_BLIT	10	//!< Does WM_PAINT cause a backbuffer blit?
#define GW_SPT_WIREFRAME_STRIPS	11	//!< Are wireframe objects are sent as tristrips?
#define GW_SPT_ORG_UPPER_LEFT	12	//!< Is the device origin at upper left?
#define GW_SPT_ARRAY_PROCESSING	13	//!< Can the drive handle vertex array data?
#define GW_SPT_NUM_LIGHTS		14	//!< Number of lights supported?
#define GW_SPT_NUM_TEXTURES		15	//!< Number of multitexture stages supported?
#define GW_SPT_WIRE_FACES		16	//!< Support for wireframe faces with visibility flags?
#define GW_SPT_TOTAL			17	//!< The maximum number of support queries.
//@}

/// \defgroup Display_State_of_the_Graphics_Window Display State of the Graphics Window
//@{
#define GW_DISPLAY_MAXIMIZED	1
#define GW_DISPLAY_WINDOWED		2
#define GW_DISPLAY_INVISIBLE	3
//@}

/// \defgroup Pass_Identification_for_Multi_Pass_Rendering Pass Identification for Multi-Pass Rendering
//@{
#define GW_PASS_ONE				0
#define GW_PASS_TWO				1
//@}

/// \defgroup Light_Attenuation_Types Light Attenuation Types
/// \note Not fully supported.
//@{
#define GW_ATTEN_NONE		0x0000
#define GW_ATTEN_START		0x0001
#define GW_ATTEN_END		0x0002
#define GW_ATTEN_LINEAR		0x0010
#define GW_ATTEN_QUAD		0x0020
//@}

/// \internal \defgroup Internal_Flags Internal Flags 
/// Flags used to tell Max graphics drivers that the user has returned from various system events.
/// Resolves issues with DirectX. The flags are used with GraphicsWindow::setFlags, but are 
/// intended for internal use only.
//@{
#define SYSTEM_LOCK_RETURN 0x001 
#define SYSTEM_REMOTE_RETURN 0x002 
//@}
