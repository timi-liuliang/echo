//**************************************************************************/
// Copyright (c) 1998-2007 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Plugin DLL API
// AUTHOR: Dan Silva - Nov.30.1994
//***************************************************************************/

#pragma once

#ifndef	_CPPRTTI
#error 3ds Max plugins and dlls must compile with run-time type information (RTTI) On.\
RTTI can be turned on via the Language tab in the C++ folder of the \
Visual Studio project settings of your project, or the /GR compiler flag.
#endif

#include "maxheap.h"
#include "maxtypes.h"
#include "buildver.h"	// russom - 03/08/01
#include "maxversion.h"
#include "iparamb2Typedefs.h"
#include "coreexp.h"
#include "tab.h"
#include "strbasic.h"
#include "box2.h" // for Rect. Cannot forward declare this.
// forward declarations
class INode;
class ReferenceMaker;

/** 
    \file plugapi.h

    This file contains API related to plug-in identification to 3ds Max.

    Definitions from this file can be found in the following documentation modules:

    \li \ref VersionMacros Version Macros 
    \li \ref SuperClassIDs Super Class IDs 
    \li \ref ClassIDs Class IDs 
*/

/** \defgroup VersionMacros Version Macros */
//@{

// 3DSMAX Version number to be compared against that returned by DLL's
// LibVersion function to detect obsolete DLLs.
//#define VERSION_3DSMAX 100      // changed  4/ 9/96 -- DB
//#define VERSION_3DSMAX 110      // changed  7/17/96 -- DS
//#define VERSION_3DSMAX 111      // changed  9/24/96 -- DS
//#define VERSION_3DSMAX 120      // changed  9/30/96 -- DB
//#define VERSION_3DSMAX 200      // changed  10/30/96 -- DS

/**
\name 3ds Max Release Identifiers
*/
//@{
//! Represents an unsupported 3ds Max version. 
#define MAX_RELEASE_UNSUPPORTED (-1)
// MAX release number X 1000
//#define MAX_RELEASE				2000    // DDS 9/30/97  
//#define MAX_RELEASE				2500    // JMK 2/25/98  
//#define MAX_RELEASE				3000    // DDS 9/3/98  
//#define MAX_RELEASE				3100    // CCJ 10/21/99  
//#define MAX_RELEASE				3900    // CCJ 6/26/00
//#define MAX_RELEASE				4000    // CCJ 11/29/00
//#define MAX_RELEASE				4200    // 010514  --prs.
//#define MAX_RELEASE				4900    // for R5 alpha, 011115  --prs.
//#define MAX_RELEASE				5000    // for R5, 020604  --prs.
//#define MAX_RELEASE				5100    // for R5.1, 020801  --prs.
//#define MAX_RELEASE				5900    // for R6 alpha, 030403  --prs.
//#define MAX_RELEASE				6000    // for R6, 030806  --prs.
//#define MAX_RELEASE_R7_ALPHA		6900    // for R7 alpha
//! 3ds Max 7 release ID
#define MAX_RELEASE_R7				7000	
//! 3ds Max 8 alpha release ID
#define MAX_RELEASE_R8_ALPHA		7800	
//! 3ds Max 8 release id
#define MAX_RELEASE_R8				8000	
//! 3ds Max 9 alpha release id
#define MAX_RELEASE_R9_ALPHA		8900	
//! 3ds Max 9 alpha 2 release id (file format change)
#define MAX_RELEASE_R9_ALPHA2		8901	
//! 3ds Max 9 release id
#define MAX_RELEASE_R9				9000	
//! 3ds Max 10 alpha release id
#define MAX_RELEASE_R10_ALPHA		9900	
//! 3ds Max 10 release id
#define MAX_RELEASE_R10				10000	
//! 3ds Max 2009 alpha release id
#define MAX_RELEASE_R11_ALPHA		10900	
//! 3ds Max 2009 release id
#define MAX_RELEASE_R11				11000	
//! 3ds Max 2010 alpha release id
#define MAX_RELEASE_R12_ALPHA		11900	
//! 3ds Max 2010 release id
#define MAX_RELEASE_R12				12000	
//! 3ds Max 2011 alpha release id
#define MAX_RELEASE_R13_ALPHA		12900	
//! 3ds Max 2011 release id
#define MAX_RELEASE_R13				13000	
//! 3ds Max R14 Alpha, Beta release id
#define MAX_RELEASE_R14_ALPHA		13900	
//! 3ds Max R14 release id
#define MAX_RELEASE_R14				14000	
//! Identifier of the current 3ds Max release
#define MAX_RELEASE					MAX_RELEASE_R14
//@}

/** MAX_RELEASE_EXTERNAL is an alternative for MAX_RELEASE
 that can be used for UI when the internal version of the product
 is different from the branded version number.
 MAX_RELEASE_EXTERNAL is defined in maxversion.h */
#ifndef MAX_RELEASE_EXTERNAL
 #define MAX_RELEASE_EXTERNAL MAX_RELEASE
#endif

/** \name Max SDK Version Numbers. 
The SDK version number is updated every time a build of Max containing SDK 
changes is distributed to 3rd party plugin developers. This can be an alpha, beta 
or release candidate build.
Although we make every effort to keep Max backward compatible with plugins built 
against previous versions of the Max SDK, occasionally plugins are required to be 
re-built against the latest version of the Max SDK in order for Max to load them. 
Plugins built against a certain version of the Max SDK will not load in previous 
versions of Max. 
Note that the Max SDK version number is encoded in VERSION_3DSMAX. 

SDK backward compatibility history:
- 3ds Max R5 is binary compatible with 3ds Max R4
- 3ds Max R8 is binary compatible with 3ds Max R6
- 3ds Max R9.0 not binary compatible with previous releases
- 3ds Max R10 is binary compatible with 3ds Max R9 
- 3ds Max R11 not binary compatible with previous releases
- 3ds Max R12 not binary compatible with previous releases
- 3ds Max R13 is binary compatible with 3ds Max R12 

\see VERSION_3DSMAX
\see LibVersion
*/
// Blocks of values are reserved for specific products that are meant
// to have non-compatible APIs. Currently assigned:
//    000-???
//    255:     temporary use to prevent NOVA plugins from loading in MAX

#define MAX_API_NUM_R20				4	 // DDS 9/30/97	
#define MAX_API_NUM_R25				5	 // DDS 10/06/97
#define MAX_API_NUM_R30				6	 // DDS 9/3/98
#define MAX_API_NUM_R31				7	 // CCJ 5/14/99
#define MAX_API_NUM_R40				8	 // CCJ 11/29/00 - R4
#define MAX_API_NUM_R42				9	 // R5 011031  --prs.
//! 3ds Max 5 SDK.
#define MAX_API_NUM_R50				11	 // R5 FCS 020624  --prs.
//! 3ds Max 6 SDK pre-release.
#define MAX_API_NUM_R60_PRE_REL		12	 // R6 prerelease 030409  --prs.
//! 3ds Max 6 SDK.  
#define MAX_API_NUM_R60				13	 // R6 FCS 030806  --prs.
//! 3ds Max 7 SDK pre-release.
#define MAX_API_NUM_R70_PRE_REL		14	 // R7 prerelease 05/18/04 - russom
//! 3ds Max 7 SDK. 
#define MAX_API_NUM_R70				15	 // R7 FCS -- 2004-09-02 -- raffenn
//! 3ds Max 8 SDK pre-release.
#define MAX_API_NUM_R80_PRE_REL		16	 // R8 pre-release Vesper -- 2005-01-18 -- raffenn
//! 3ds Max 8 SDK.
#define MAX_API_NUM_R80				17	 // R8 FCS max 8 -- 2005-09-01 -- raffenn
//! 3ds Max 7 SDK pre-release.
#define MAX_API_NUM_R90_ALPHA		18	 // R9 alpha -- 2005-09-21 -- aszabo
//! 3ds Max 9 Beta 1 SDK.
#define MAX_API_NUM_R90_BETA1		19	 
//! 3ds Max 9 Beta 3 SDK.
#define MAX_API_NUM_R90_BETA3		20	 
//! 3ds Max 9 SDK. SDK incompatible with 3ds Max 8.
#define MAX_API_NUM_R90			    21	 
//! 3ds Max 10 Beta 5 SDK.
#define MAX_API_NUM_R100_BETA5		22	 
//! to 3ds Max 10 SDK. SDK compatible with 3ds Max 9.
#define MAX_API_NUM_R100			23	 
//! 3ds Max 2009 Alpha SDK. SDK is incompatible with 3ds Max 10.
#define MAX_API_NUM_R110_ALPHA		24	 
//! 3ds Max 2009 SDK. SDK is compatible with 3ds Max 2009 Alpha
#define MAX_API_NUM_R110			25	 
//! 3ds Max 2010 Alpha 2 SDK. SDK is incompatible with 3ds Max 2009.
#define MAX_API_NUM_R120_ALPHA2		26	 
//! 3ds Max 2010 Alpha 3 SDK. SDK is incompatible with 3ds Max 2010 Alpha 2.
#define MAX_API_NUM_R120_ALPHA3		27	 
//! 3ds Max 2010 Alpha 4 SDK. SDK is incompatible with 3ds Max 2010 Alpha 3.
#define MAX_API_NUM_R120_ALPHA4		28	 
//! 3ds Max 2010 Beta 1 SDK. SDK is incompatible with 3ds Max 2010 Alpha 4.
#define MAX_API_NUM_R120_BETA1		29	 
//! 3ds Max 2010 Beta 3 SDK. SDK is compatible with 3ds Max 2010 Beta 1 (Beta 2 was identical to Beta 1).
#define MAX_API_NUM_R120_BETA3		30	 
//! 3ds Max 2010 Beta 4 SDK. SDK is compatible with 3ds Max 2010 Beta 3.
#define MAX_API_NUM_R120_BETA4		31	 
//! 3ds Max 2010 Beta 5 SDK. SDK is compatible with 3ds Max 2010 Beta 4. 
#define MAX_API_NUM_R120_BETA5		32	 
//! 3ds Max 2010 SDK. SDK is compatible with 3ds Max 2010 Beta 5.
#define MAX_API_NUM_R120		    33	 
//! 3ds Max 2011 Beta 1 SDK. SDK is compatible with 3ds Max 2010 and 3ds Max 2011 Alpha builds except for new APIs.
#define MAX_API_NUM_R130_BETA1		34	 
//! 3ds Max 2011 SDK. SDK is compatible with 3ds Max 2010 and 3ds Max 2011 Alpha and Beta builds except for new APIs.
#define MAX_API_NUM_R130			35	 
//! 3ds Max R14 Alpha 1 SDK. SDK is incompatible with 3ds Max 2011.
#define MAX_API_NUM_R140_ALPHA1		36	 
//! 3ds Max R14 Beta  2 SDK - incompatible with 3ds Max 2012 Beta 1 and earlier.
#define MAX_API_NUM_R140_BETA2		37 
//! 3ds Max R14 SDK - incompatible with 3ds Max 2011, compatible with 3ds Max 2012 Beta 2 and later.
#define MAX_API_NUM_R140			38 
/** The 3ds Max SDK current version number. 
	It is used to verify a plug-in's binary compatibility with a specific version of 3ds Max. 
	It is encoded into VERSION_3DSMAX which is the version number plug-in must return 
	from their implementation of LibVersion(). The GET_MAX_API_NUM macro can be used 
	to extract the SDK version number from a the library version (VERSION_3DSMAX).*/
#define MAX_API_NUM MAX_API_NUM_R140

/** Denotes the revision of the SDK for a given API. This is incremented.
    when the SDK functionality changes in some significant way (for instance
    a new GetProperty() query  response is added), but the headers have 
    not been changed.*/
#define MAX_SDK_REV     0	  // DDS 9/20/97

/** This is the value is required to be returned by a plug-in DLL's implementation of LibVersion(). */
#define VERSION_3DSMAX ((MAX_RELEASE<<16)+(MAX_API_NUM<<8)+MAX_SDK_REV)

// update jagapi.cpp product_app_id if new enums are added
enum APPLICATION_ID {kAPP_NONE, kAPP_MAX, kAPP_VIZ, kAPP_GMAX, kAPP_PLASMA, kAPP_VIZR};

/** Returns the ApplicationID, either VIZ or MAX. If a plugin
    is designed to work only in one product, then you could use this method
    in your IsPublic() call to switch between exposing the plug-in or not. */
CoreExport APPLICATION_ID GetAppID();

/** \name Macros for extracting parts of VERSION_3DSMAX */
//@{
//! 
#define GET_MAX_RELEASE(x)  (((x)>>16)&0xffff)
#define GET_MAX_API_NUM(x)  (((x)>>8)&0xff)
#define GET_MAX_SDK_REV(x)  ((x)&0xff)
#define GET_MAX_SDK_NUMREV(x)  ((x)&0xffff) 
//@}

//@}
/** \defgroup SuperClassIDs Super Class IDs */
//@{

/** Reserved Super Class IDs 
    \internal 
    Plug-ins developed using the SDK should never use these super class IDs.
*/
//@{
#define GEN_MODAPP_CLASS_ID 		0x00000b
#define MODAPP_CLASS_ID 			0x00000c
#define OBREF_MODAPP_CLASS_ID 		0x00000d
#define BASENODE_CLASS_ID			0x000001
#define GEN_DERIVOB_CLASS_ID 		0x000002
#define DERIVOB_CLASS_ID 			0x000003
#define WSM_DERIVOB_CLASS_ID 		0x000004
#define PARAMETER_BLOCK_CLASS_ID 	0x000008	
#define PARAMETER_BLOCK2_CLASS_ID 	0x000082	
#define EASE_LIST_CLASS_ID			0x000009
#define AXIS_DISPLAY_CLASS_ID		0x00000e
#define MULT_LIST_CLASS_ID			0x00000f
#define NOTETRACK_CLASS_ID			0x0000ff
#define TREE_VIEW_CLASS_ID			0xffffff00
#define SCENE_CLASS_ID				0xfffffd00
#define THE_GRIDREF_CLASS_ID		0xfffffe00
#define VIEWREF_CLASS_ID			0xffffff01
#define BITMAPDAD_CLASS_ID			0xffffff02 // For drag and drop of bitmaps
#define PARTICLE_SYS_CLASS_ID		0xffffff03 // NOTE: this is for internal use only. Particle systems return GEOMOBJECT_CLASS_ID -- use IsParticleObject() to determine if an object is a particle system.
#define AGGMAN_CLASS_ID				0xffffff05 // Object aggregation, VIZ
#define MAXSCRIPT_WRAPPER_CLASS_ID	0xffffff06 // MAX object wrappers within MAXScript
#define TRACKBAR_CLASS_ID			0xffffff07	// Trackbar 
#define MAKEREF_REST_CLASS_ID		0x9876544	// MakeRefRestore
#define DELREF_REST_CLASS_ID		0x9876543	// DeleteRefRestore
#define ASSIGNREF_REST_CLASS_ID		0x121242	// AssignMtlRestore
#define LAYER_CLASS_ID				0x0010f0	
//@}

/// \name Super-class IDs for Plug-ins
/// These are the super-class IDs that can be used by the different plug-in types 
/// that can be built with the 3ds Max SDK.
//@{
#define GEOMOBJECT_CLASS_ID			0x000010    //!< Geometric object super-class ID.
#define CAMERA_CLASS_ID				0x000020    //!< Camera object super-class ID.
#define LIGHT_CLASS_ID				0x000030    //!< Light object super-class ID.  
#define SHAPE_CLASS_ID				0x000040    //!< Shape object super-class ID.  
#define HELPER_CLASS_ID				0x000050    //!< Helper object super-class ID.  
#define SYSTEM_CLASS_ID	 			0x000060    //!< System plug-in super-class ID.   
#define REF_MAKER_CLASS_ID			0x000100    //!< ReferenceMaker super-class ID.   	
#define REF_TARGET_CLASS_ID	 		0x000200    //!< ReferenceTarget super-class ID.  
#define OSM_CLASS_ID				0x000810    //!< Object-space modifier (Modifier) super-class ID.  
#define WSM_CLASS_ID				0x000820    //!< World-space modifier (WSModifier) super-class ID.  
#define WSM_OBJECT_CLASS_ID			0x000830    //!< World-space modifier object (WSMObject) super-class ID.  
#define SCENE_IMPORT_CLASS_ID		0x000A10    //!< Scene importer (SceneImport) super-class ID.  
#define SCENE_EXPORT_CLASS_ID		0x000A20    //!< Scene exporter (SceneExport) super-class ID.  
#define BMM_STORAGE_CLASS_ID		0x000B10    //!< Bitmap storage super-class ID.  
#define BMM_FILTER_CLASS_ID			0x000B20    //!< Image filter super-class ID.  
#define BMM_IO_CLASS_ID				0x000B30    //!< Image loading/saving super-class ID.  
#define BMM_DITHER_CLASS_ID			0x000B40    //!< Bitmap dithering super-class ID.  
#define BMM_COLORCUT_CLASS_ID		0x000B50    //!< Shape object super-class ID.  
#define USERDATATYPE_CLASS_ID		0x000B60    //!< \deprecated
#define MATERIAL_CLASS_ID			0x000C00    //!< Materials super-class ID.
#define TEXMAP_CLASS_ID				0x000C10    //!< Texture maps super-class ID.
#define UVGEN_CLASS_ID				0x0000C20   //!< UV Generator super-class ID.
#define XYZGEN_CLASS_ID				0x0000C30   //!< XYZ Generator super-class ID.
#define TEXOUTPUT_CLASS_ID			0x0000C40   //!< Texture output filter super-class ID.
#define SOUNDOBJ_CLASS_ID			0x000D00    //!< Sound object super-class ID.
#define FLT_CLASS_ID				0x000E00    //!< Image processing filter super-class ID. 
#define RENDERER_CLASS_ID			0x000F00    //!< Renderer super-class ID.
#define BEZFONT_LOADER_CLASS_ID		0x001000    //!< Bezier font loader super-class ID.
#define ATMOSPHERIC_CLASS_ID		0x001010    //!< Atmospheric effect super-class ID.
#define UTILITY_CLASS_ID			0x001020	//!< Utility object super-class ID.
#define TRACKVIEW_UTILITY_CLASS_ID	0x001030    //!< Trackview utility super-class ID.
#define FRONTEND_CONTROL_CLASS_ID	0x001040    //!< Front-end control super-class ID.
#define MOT_CAP_DEV_CLASS_ID		0x001060    //!< Motion capture device super-class ID.
#define MOT_CAP_DEVBINDING_CLASS_ID	0x001050    //!< Motion capture device binding super-class ID.
#define OSNAP_CLASS_ID				0x001070    //!< Object snap super-class ID.
#define TEXMAP_CONTAINER_CLASS_ID	0x001080    //!< Texture map container super-class ID.
#define RENDER_EFFECT_CLASS_ID      0x001090    //!< Render post-effects super-class ID.
#define FILTER_KERNEL_CLASS_ID      0x0010a0    //!< Anti-aliasing filter kernel super-class ID.
#define SHADER_CLASS_ID				0x0010b0    //!< Standard material shader super-class ID.
#define COLPICK_CLASS_ID		  	0x0010c0    //!< Color picker super-class ID.
#define SHADOW_TYPE_CLASS_ID		0x0010d0    //!< Shadow generator super-class ID.
#define GUP_CLASS_ID		  		0x0010e0    //!< Global utility plug-in super-class ID.
#define SCHEMATICVIEW_UTILITY_CLASS_ID	0x001100 //!< Schematic view utility super-class ID.
#define SAMPLER_CLASS_ID			0x001110    //!< Sampler super-class ID.
#define IK_SOLVER_CLASS_ID			0x001140    //!< IK solver super-class ID.
#define RENDER_ELEMENT_CLASS_ID		0x001150	//!< Render output element super-class ID.
#define BAKE_ELEMENT_CLASS_ID		0x001151	//!< Texture bake output element super-class ID.
#define CUST_ATTRIB_CLASS_ID		0x001160    //!< Custom attributes super-class ID.
#define RADIOSITY_CLASS_ID			0x001170	//!< Global illumination plugin super-class ID.
#define TONE_OPERATOR_CLASS_ID		0x001180	//!< Tone operator super-class ID.
#define MPASS_CAM_EFFECT_CLASS_ID	0x001190	//!< Multi-pass camera effect super-class ID.
#define MR_SHADER_CLASS_ID_DEFUNCT	0x0011a0	//!< Mental ray shader super-class ID. \note No longer used, kept for file compatibility.
//@}

//! \name Controller super-class IDs
//! These super-class IDs identify the different kinds of controller (Control) plug-ins.
//@{
#define	CTRL_SHORT_CLASS_ID 	   	0x9001  //!< \deprecated
#define	CTRL_INTEGER_CLASS_ID		0x9002  //!< \deprecated
#define	CTRL_FLOAT_CLASS_ID			0x9003  //!< Float controller super-class ID.
#define	CTRL_POINT2_CLASS_ID	   	0x9004  //!< \deprecated
#define	CTRL_POINT3_CLASS_ID	   	0x9005  //!< Point3 controller super-class ID.
#define	CTRL_POS_CLASS_ID		   	0x9006  //!< \deprecated Use CTRL_POSITION_CLASS_ID instead.
#define	CTRL_QUAT_CLASS_ID			0x9007  //!< \deprecated
#define	CTRL_MATRIX3_CLASS_ID		0x9008  //!< Matrix3 controller super-class ID.
#define	CTRL_COLOR_CLASS_ID     	0x9009	//!< \deprecated
#define	CTRL_COLOR24_CLASS_ID   	0x900A  //!< \deprecated
#define	CTRL_POSITION_CLASS_ID		0x900B  //!< Position controller super-class ID.
#define	CTRL_ROTATION_CLASS_ID		0x900C  //!< Rotation controller super-class ID.
#define	CTRL_SCALE_CLASS_ID			0x900D  //!< Scale controller super-class ID.
#define CTRL_MORPH_CLASS_ID			0x900E  //!< Morph controller super-class ID.
#define CTRL_USERTYPE_CLASS_ID		0x900F  //!< \deprecated
#define CTRL_MASTERPOINT_CLASS_ID	0x9010	
#define MASTERBLOCK_SUPER_CLASS_ID	0x9011
#define	CTRL_POINT4_CLASS_ID	   	0x9012  
#define	CTRL_FRGBA_CLASS_ID     	0x9013	
//@}

/* \name Pseudo super-class IDs 
    Psuedo super-class IDs are used to identify categories of objects. */
//@{
#define DEFORM_OBJ_CLASS_ID 		0x000005 //!< Deformable object pseudo super-class ID
#define MAPPABLE_OBJ_CLASS_ID 		0x000006 //!< Mappable object pseudo super-class ID
#define GENERIC_SHAPE_CLASS_ID		0x0000ab //!< Shape pseudo super-class ID
//@}

//@}
/** \defgroup ClassIDs Predefined Class IDs */
//@{

/** \name Class IDs of built-in classes 
    Class ID's of built-in classes. The value is the first ULONG of the 
     8 byte Class ID: the second ULONG is 0 for most built-in classes.
    \note Plug-ins should never have a class ID where the second ULONG is 0. */
//@{
#define PATH_CONTROL_CLASS_ID				0x2011 
#define EULER_CONTROL_CLASS_ID				0x2012
#define EXPR_POS_CONTROL_CLASS_ID			0x2013
#define EXPR_P3_CONTROL_CLASS_ID			0x2014
#define EXPR_FLOAT_CONTROL_CLASS_ID			0x2015
#define EXPR_SCALE_CONTROL_CLASS_ID			0x2016
#define EXPR_ROT_CONTROL_CLASS_ID			0x2017
#define LOCAL_EULER_CONTROL_CLASS_ID		0x2018
#define POSITION_CONSTRAINT_CLASS_ID		0x2019  // AG added for Position Constraint 4/21/2000
#define ORIENTATION_CONSTRAINT_CLASS_ID		0x2020  // AG added for Position Constraint 5/04/2000
#define LOOKAT_CONSTRAINT_CLASS_ID			0x2021  // AG added for new LookAt Constraint 5/24/2000
#define ADDITIVE_EULER_CONTROL_CLASS_ID		0x2022	// 000830  --prs.
#define BOOLCNTRL_CLASS_ID					Class_ID(0x5ed56fc5, 0x6af0e521) // AG added for the new Boolean Controller 11/08/2001
#define FLOATNOISE_CONTROL_CLASS_ID		0x87a6df24
#define POSITIONNOISE_CONTROL_CLASS_ID	0x87a6df25
#define POINT3NOISE_CONTROL_CLASS_ID	0x87a6df26
#define ROTATIONNOISE_CONTROL_CLASS_ID	0x87a6df27
#define SCALENOISE_CONTROL_CLASS_ID		0x87a6df28
#define SURF_CONTROL_CLASSID			Class_ID(0xe8334011,0xaeb330c8)
#define LINKCTRL_CLASSID				Class_ID(0x873fe764,0xaabe8601)
#define ATTACH_CONTROL_CLASS_ID			Class_ID(0xbb27e611,0xa72f43e7)
#define FLOATLIMITCTRL_CLASS_ID			0x17127e3e
#define DUMMYCHANNEL_CLASS_ID				0xefffffff // used by PB and PB2 - superclass is CTRL_USERTYPE_CLASS_ID
#define RINGARRAY_CLASS_ID			0x9120 
#define SUNLIGHT_CLASS_ID			Class_ID(0x5897670e, 0x61b5008d)
#define DAYLIGHT_CLASS_ID			Class_ID(0x18147db5, 0x20f17194)
#define CUSTATTRIB_CONTAINER_CLASS_ID Class_ID(0x5ddb3626, 0x23b708db) // CustAttribContainer class id
//@}


/** \name Class IDs of classes derived from GEOMOBJECT_CLASS_ID 
    These are built-in class IDs. */
//@{
#define TRIOBJ_CLASS_ID 	 	0x0009	    //!< TriObject class ID
#define EDITTRIOBJ_CLASS_ID	0xe44f10b3	    //!< Base triangle mesh (Edit class ID
#define POLYOBJ_CLASS_ID		0x5d21369a	//!< Polygon mesh (PolyObject) class ID
#define PATCHOBJ_CLASS_ID  		0x1030      //!< Patch object
#define NURBSOBJ_CLASS_ID		0x4135      //!< Nurbs object 
#define EPOLYOBJ_CLASS_ID Class_ID(0x1bf8338d,0x192f6098) //!< Editable poly.
//@}

// Primitives
// xavier robitaille | 03.02.15 
// Prevents the user from creating box/cylinder/sphere objects without
// removing them completely (we need them for the material editor).
/// \name Primitive Object Class IDs
//@{
#ifndef NO_OBJECT_STANDARD_PRIMITIVES
#define BOXOBJ_CLASS_ID 		0x0010
#define SPHERE_CLASS_ID 		0x0011 
#define CYLINDER_CLASS_ID 		0x0012
#define PLANE_CLASS_ID          Class_ID(0x81f1dfc, 0x77566f65)
#define PYRAMID_CLASS_ID        Class_ID(0x76bf318a, 0x4bf37b10)
#define GSPHERE_CLASS_ID        Class_ID(0, 32670)

#else
#define BOXOBJ_CLASS_ID 		0x108f1589
#define SPHERE_CLASS_ID 		0x72ce5ad6 
#define CYLINDER_CLASS_ID 		0x759e40e1
#endif
#define CONE_CLASS_ID			0xa86c23dd
#define TORUS_CLASS_ID			0x0020
#define TUBE_CLASS_ID			0x7B21
#define HEDRA_CLASS_ID			0xf21c5e23
#define BOOLOBJ_CLASS_ID		0x387BB2BB //!< \deprecated use NEWBOOL_CLASS_ID
#define NEWBOOL_CLASS_ID Class_ID(0x51db4f2f,0x1c596b1a) //!< Boolean object Class ID (replaces BOOLOBJ_CLASS_ID)
//@}

/// \name XRef Class IDs
//@{
#define XREFOBJ_CLASS_ID		Class_ID(0x92aab38c, 0) //!< XRef object class ID
#define XREFATMOS_CLASS_ID	Class_ID(0x4869d60f, 0x21af2ae4) //!< XRef atmospheric effect class ID
#define XREFMATERIAL_CLASS_ID Class_ID(0x272c0d4b, 0x432a414b) //!< XRef material class ID
#define XREFCTRL_CLASS_ID Class_ID(0x32fb4637, 0x65fd482b) //!< XRef control class ID 
//@}

/// \name Subclasses of OSNAP_CLASS_ID
//@{
#define GRID_OSNAP_CLASS_ID Class_ID(0x62f565d6, 0x110a1f97) 
//@}

#define TEAPOT_CLASS_ID1		0xACAD13D3 //!< Teapot upper class ID
#define TEAPOT_CLASS_ID2		0xACAD26D9 //!< Teapot lower class ID

#define PATCHGRID_CLASS_ID  	0x1070

#define BONE_OBJ_CLASSID		Class_ID(0x28bf6e8d, 0x2ecca840) //!< Procedural bone object, subclass of GeomObject
// block IDs
enum BoneObj_Block { boneobj_params, };

// boneobj_params IDs
enum BoneObj_Params { 
	boneobj_width, boneobj_height, boneobj_taper, boneobj_length,
	boneobj_sidefins, boneobj_sidefins_size, boneobj_sidefins_starttaper, boneobj_sidefins_endtaper,
	boneobj_frontfin, boneobj_frontfin_size, boneobj_frontfin_starttaper, boneobj_frontfin_endtaper,
	boneobj_backfin,  boneobj_backfin_size,  boneobj_backfin_starttaper,  boneobj_backfin_endtaper,
	boneobj_genmap };

/// \name Particle Class IDs
//@{
#define RAIN_CLASS_ID			0x9bd61aa0
#define SNOW_CLASS_ID			0x9bd61aa1
//@}

/// \name Space Warp Object Class IDs
//@{
#define WAVEOBJ_CLASS_ID 		0x0013
//@}

/// \name Lofter Class IDs
//@{
#define LOFTOBJ_CLASS_ID		0x1035
#define LOFT_DEFCURVE_CLASS_ID	0x1036
#define LOFT_GENERIC_CLASS_ID	0x10B0
//@}

/// \name Miscellaneous Class IDs
//@{
#define TARGET_CLASS_ID  		0x1020  //!< Camera target class ID
#define MORPHOBJ_CLASS_ID		0x1021  //!< Morph object class ID
//@}

/// \name Subclass class IDs of SHAPE_CLASS_ID
//@{
#define SPLINESHAPE_CLASS_ID 	0x00000a
#define LINEARSHAPE_CLASS_ID 	0x0000aa
#define SPLINE3D_CLASS_ID  		0x1040
#define NGON_CLASS_ID  			0x1050
#define DONUT_CLASS_ID  		0x1060
#define STAR_CLASS_ID			0x1995
#define RECTANGLE_CLASS_ID		0x1065
#define HELIX_CLASS_ID			0x1994
#define ELLIPSE_CLASS_ID		0x1097
#define CIRCLE_CLASS_ID			0x1999
#define TEXT_CLASS_ID			0x1993
#define ARC_CLASS_ID			0x1996
#define HALF_ROUND_CLASS_ID		0x1997
#define QUARTER_ROUND_CLASS_ID	0x1998
#define PIPE_CLASS_ID			0x199A
#define WALLED_RECTANGLE_CLASS_ID 0xd8bff66c
#define CHANNEL_CLASS_ID		0x7dbc0e96
#define ANGLE_CLASS_ID			0x93257030
#define TEE_CLASS_ID			0xfaead70c
#define WIDE_FLANGE_CLASS_ID	0xa2b72ef6
//@}

/// \name Subclass class IDs of CAMERA_CLASS_ID
//@{
#define SIMPLE_CAM_CLASS_ID  	0x1001
#define LOOKAT_CAM_CLASS_ID  	0x1002
//@}

/// \name Subclass class IDs of LIGHT_CLASS_ID
//@{
#define OMNI_LIGHT_CLASS_ID  		0x1011
#define SPOT_LIGHT_CLASS_ID  		0x1012
#define DIR_LIGHT_CLASS_ID  		0x1013
#define FSPOT_LIGHT_CLASS_ID  		0x1014
#define TDIR_LIGHT_CLASS_ID  		0x1015
//@}

/// \name Subclass class IDs of HELPER_CLASS_ID
//@{
#define DUMMY_CLASS_ID 			0x876234
#define BONE_CLASS_ID 			0x8a63c0
#define TAPEHELP_CLASS_ID 		0x02011
#define GRIDHELP_CLASS_ID		0x02010
#define POINTHELP_CLASS_ID		0x02013
#define PROTHELP_CLASS_ID		0x02014
//@}

/// \name Subclass class IDs of UVGEN_CLASS_ID
//@{
#define STDUV_CLASS_ID 			0x0000100
//@}

/// \name Subclass class IDs of XYZGEN_CLASS_ID
//@{
#define STDXYZ_CLASS_ID 		0x0000100
//@}

/// \name Subclass class IDs of TEXOUT_CLASS_ID
//@{
#define STDTEXOUT_CLASS_ID 		0x0000100
//@}

/// \name Subclass class IDs of MATERIAL_CLASS_ID
//@{
#define DMTL_CLASS_ID  			0x00000002	//!< StdMtl2: standard material.
#define DMTL2_CLASS_ID  		0x00000003	//!< \deprecated Was used when migrating from StdMtl to StdMtl2

#ifndef NO_MTL_TOPBOTTOM // orb 01-07-2001
#define CMTL_CLASS_ID 			0x0000100  //!< Top-bottom material 
#endif // NO_MTL_TOPBOTTOM

#define MULTI_CLASS_ID 			0x0000200  //!< Super class ID multi material
#define MULTI_MATERIAL_CLASS_ID		Class_ID(MULTI_CLASS_ID, 0) //! Multi-sub-object material class ID

#define DOUBLESIDED_CLASS_ID 	0x0000210  //!< Double-sided material
#define MIXMAT_CLASS_ID 		0x0000250  //!< Blend material
#define BAKE_SHELL_CLASS_ID 	0x0000255  //!< Two material shell for baking

#ifndef NO_MTL_MATTESHADOW // orb 01-07-2001
#define MATTE_CLASS_ID 			0x0000260  //!< Matte material class ID.
#endif // NO_MTL_MATTESHADOW

#define LOCKMAT_CLASS_ID	Class_ID(0x5c92647b, 0x35a5272a) //!< Locked material class ID
//@}

/// \name Subclass class IDs of TEXMAP_CLASS_ID
//@{
#define CHECKER_CLASS_ID 		0x0000200
#define MARBLE_CLASS_ID 		0x0000210
#define MASK_CLASS_ID 			0x0000220  //!< Mask texture
#define MIX_CLASS_ID 			0x0000230
#define NOISE_CLASS_ID 			0x0000234

#ifndef NO_MAPTYPE_GRADIENT // orb 01-07-2001
#define GRADIENT_CLASS_ID 		0x0000270
#endif // NO_MAPTYPE_GRADIENT

#ifndef NO_MAPTYPE_RGBTINT // orb 01-07-2001
#define TINT_CLASS_ID 			0x0000224  //!< Tint texture
#endif // NO_MAPTYPE_RGBTINT

#define BMTEX_CLASS_ID 			0x0000240  //!< Bitmap texture

#ifndef NO_MAPTYPE_REFLECTREFRACT // orb 01-07-2001
#define ACUBIC_CLASS_ID 		0x0000250  //!< Reflect/refract
#endif // NO_MAPTYPE_REFLECTREFRACT

#ifndef NO_MAPTYPE_FLATMIRROR // orb 01-07-2001
#define MIRROR_CLASS_ID 		0x0000260  //!< Flat mirror
#endif // NO_MAPTYPE_FLATMIRROR

#define COMPOSITE_CLASS_ID 		0x0000280   //!< Composite texture
#define COMPOSITE_MATERIAL_CLASS_ID Class_ID(0x61dc0cd7, 0x13640af6)  //!< Composite Material

#ifndef NO_MAPTYPE_RGBMULT // orb 01-07-2001
#define RGBMULT_CLASS_ID 		0x0000290   //!< RGB Multiply texture
#endif //NO_MAPTYPE_RGBMULT

#define FALLOFF_CLASS_ID 		0x00002A0   //!< Falloff texture

#ifndef NO_MAPTYPE_OUTPUT // orb 01-07-2001
#define OUTPUT_CLASS_ID 		0x00002B0   //!< Output texture
#endif // NO_MAPTYPE_OUTPUT

#define PLATET_CLASS_ID 		0x00002C0   //!< Plate glass texture

#define COLORCORRECTION_CLASS_ID    0x00002D0   //!< Color Correction texture

#ifndef NO_MAPTYPE_VERTCOLOR // orb 01-07-2001
#define VCOL_CLASS_ID 			0x0934851	//!< Vertex color map
// JBW: IDs for ParamBlock2 blocks and parameters
// Parameter and ParamBlock IDs
enum TextureMap_Vertex_Color_BlockID
{
	vertexcolor_params
};
enum TextureMap_Vertex_Color_ParamIDs
{ 
	vertexcolor_map,
	vertexcolor_subid
};
#endif // NO_MAPTYPE_VERTCOLOR

#define MULTIOUTPUTTOTEXMAP_CLASS_ID	Class_ID(0x896EF2FC, 0x44BD743F) //!< MultiOutputToTexmap texture
//@}

/// \name Subclass class IDs of SHADER_CLASS_ID
//@{
#define STDSHADERS_CLASS_ID		0x00000035	// to 39 
//@}

/// \name Subclass class IDs of SHADOW_TYPE_CLASS_ID
//@{
#define STD_SHADOW_MAP_CLASS_ID       0x0000100 
#define STD_RAYTRACE_SHADOW_CLASS_ID  0x0000200
//@}
		
/// \name Subclass class IDs of RENDERER_CLASS_ID
//@{
#define SREND_CLASS_ID 			0x000001 //!< Default scan-line renderer.

//@}
/// \name Subclass class IDs of BAKE_ELEMENT_CLASS_ID
//@{
#define COMPLETE_BAKE_ELEMENT_CLASS_ID			0x00010001
#define SPECULAR_BAKE_ELEMENT_CLASS_ID			0x00010002
#define DIFFUSE_BAKE_ELEMENT_CLASS_ID			0x00010003
#define REFLECT_REFRACT_BAKE_ELEMENT_CLASS_ID	0x00010004
#define SHADOW_BAKE_ELEMENT_CLASS_ID			0x00010005
#define LIGHT_BAKE_ELEMENT_CLASS_ID				0x00010006
#define NORMALS_ELEMENT_CLASS_ID				0x00010007
#define BLEND_BAKE_ELEMENT_CLASS_ID				0x00010008
#define ALPHA_BAKE_ELEMENT_CLASS_ID				0x00010009
#define HEIGHT_BAKE_ELEMENT_CLASS_ID			0x0001000A
#define AMBIENTOCCLUSION_BAKE_ELEMENT_CLASS_ID	0x0001000B
//@}

/// \name Subclass class IDs of REF_MAKER_CLASS_ID
//@{
#define MTL_LIB_CLASS_ID 		0x001111
#define MTLBASE_LIB_CLASS_ID 	0x003333
#define THE_SCENE_CLASS_ID   	0x002222
#define MEDIT_CLASS_ID 	 		0x000C80
#define MTL_CATEGORY_CLASS_ID   Class_ID(0xb8e3b2d, 0x19854ad2)
//@}

//! Subclass class IDs for all classes
#define STANDIN_CLASS_ID   		0xffffffff  

//! Default sound object
#define DEF_SOUNDOBJ_CLASS_ID	0x0000001

//! Default atmosphere effect
#define FOG_CLASS_ID 0x10000001

/// \name Subclass class IDs of OSM_CLASS_ID
//@{
#define SKEWOSM_CLASS_ID			0x6f3cc2aa
#define BENDOSM_CLASS_ID 			0x00010
#define TAPEROSM_CLASS_ID 			0x00020
#define TWISTOSM_CLASS_ID 			0x00090
#define SPLINEIKCONTROL_CLASS_ID	Class_ID(0x5f43ba4, 0x55fe9305) //AG added : 01/08/02

#define UVWMAPOSM_CLASS_ID			0xf72b1
#define SELECTOSM_CLASS_ID			0xf8611
#define MATERIALOSM_CLASS_ID		0xf8612
#define SMOOTHOSM_CLASS_ID			0xf8613
#define NORMALOSM_CLASS_ID			0xf8614
#define OPTIMIZEOSM_CLASS_ID		0xc4d31
#define AFFECTREGION_CLASS_ID		0xc4e32
#define SUB_EXTRUDE_CLASS_ID		0xc3a32
#define TESSELLATE_CLASS_ID			0xa3b26ff2
#define DELETE_CLASS_ID				0xf826ee01
#define MESHSELECT_CLASS_ID			0x73d8ff93
#define UVW_XFORM_CLASS_ID			0x5f32de12
#define UVW_XFORM2_CLASS_ID			0x5f32de13 //SS 11/20/2002: added
#define EDIT_POLY_MODIFIER_CLASS_ID Class_ID(0x79aa6e1d, 0x71a075b7)

#define EXTRUDEOSM_CLASS_ID 		0x000A0
#define SURFREVOSM_CLASS_ID 		0x000B0

#define DISPLACEOSM_CLASS_ID		0xc4d32
#define DISPLACE_OBJECT_CLASS_ID	0xe5240
#define DISPLACE_WSM_CLASS_ID		0xe5241

#define SINEWAVE_OBJECT_CLASS_ID 	0x00030
#define SINEWAVE_CLASS_ID 			0x00040
#define SINEWAVE_OMOD_CLASS_ID 		0x00045
#define LINWAVE_OBJECT_CLASS_ID 	0x00035
#define LINWAVE_CLASS_ID 			0x00042
#define LINWAVE_OMOD_CLASS_ID 		0x00047

#define GRAVITYOBJECT_CLASS_ID		0xe523c
#define GRAVITYMOD_CLASS_ID			0xe523d
#define WINDOBJECT_CLASS_ID			0xe523e
#define WINDMOD_CLASS_ID			0xe523f

#define DEFLECTOBJECT_CLASS_ID		0xe5242
#define DEFLECTMOD_CLASS_ID			0xe5243

#define BOMB_OBJECT_CLASS_ID 		0xf2e32
#define BOMB_CLASS_ID 				0xf2e42
//@}

/// \name FFD Modifier Class IDs
//@{
#define FFDNMOSSQUARE_CLASS_ID		Class_ID(0x8ab36cc5,0x82d7fe74)
#define FFDNMWSSQUARE_CLASS_ID		Class_ID(0x67ea40b3,0xfe7a30c4)
#define FFDNMWSSQUARE_MOD_CLASS_ID	Class_ID(0xd6636ea2,0x9aa42bf3)

#define FFDNMOSCYL_CLASS_ID			Class_ID(0x98f37a63,0x3ffe9bca)
#define FFDNMWSCYL_CLASS_ID			Class_ID(0xfa4700be,0xbbe85051)
#define FFDNMWSCYL_MOD_CLASS_ID		Class_ID(0xf1c630a3,0xaa8ff601)

#define FFD44_CLASS_ID				Class_ID(0x21325596, 0x2cd10bd8)
#define FFD33_CLASS_ID				Class_ID(0x21325596, 0x2cd10bd9)
#define FFD22_CLASS_ID				Class_ID(0x21325596, 0x2cd10bd0)
//@}

/// \name Association Context Modifier Class IDs
//@{
#define ACMOD_GEOM_GEOM_BOOLADD_CID	0x4e0f483a
#define ACMOD_GEOM_GEOM_BOOLSUB_CID	0x61661a5c
#define ACMOD_GEOM_GEOM_BOOLINT_CID	0x2a4f3945
#define ACMOD_GEOM_GEOM_SIMPAGG_CID	0x40cb05ab

#define ACMOD_SHAPE_GEOM_HOLE_CID	0x366307b0
#define ACMOD_SHAPE_GEOM_INT_CID	0x782d8d50
#define ACMOD_SHAPE_GEOM_EMBOSS_CID	0x7a13397c
#define ACMOD_SHAPE_GEOM_REVEAL_CID	0x55ed658c
//@}

#define GENERIC_AMSOLID_CLASS_ID	Class_ID(0x5bb661e8, 0xa2c27f02)

/// \name  Subclass Class IDs of Controllers
//@{
#define LININTERP_FLOAT_CLASS_ID 			0x2001
#define LININTERP_POSITION_CLASS_ID 		0x2002
#define LININTERP_ROTATION_CLASS_ID 		0x2003
#define LININTERP_SCALE_CLASS_ID			0x2004
#define PRS_CONTROL_CLASS_ID				0x2005
#define LOOKAT_CONTROL_CLASS_ID				0x2006				

#define HYBRIDINTERP_FLOAT_CLASS_ID 		0x2007
#define HYBRIDINTERP_POSITION_CLASS_ID 		0x2008
#define HYBRIDINTERP_ROTATION_CLASS_ID 		0x2009
#define HYBRIDINTERP_POINT3_CLASS_ID		0x200A
#define HYBRIDINTERP_SCALE_CLASS_ID			0x2010
#define HYBRIDINTERP_COLOR_CLASS_ID			0x2011
#define HYBRIDINTERP_POINT4_CLASS_ID		0x2012
#define HYBRIDINTERP_FRGBA_CLASS_ID			0x2013
#define HYBRIDINTERP_POINT2_CLASS_ID		Class_ID(0x15205122, 0x1690125b)

#define TCBINTERP_FLOAT_CLASS_ID 			0x442311
#define TCBINTERP_POSITION_CLASS_ID 		0x442312
#define TCBINTERP_ROTATION_CLASS_ID 		0x442313
#define TCBINTERP_POINT3_CLASS_ID			0x442314
#define TCBINTERP_SCALE_CLASS_ID			0x442315
#define TCBINTERP_POINT4_CLASS_ID			0x442316

#define MASTERPOINTCONT_CLASS_ID			0xd9c20ff
//@}

/// \name Particle Flow Class IDs
//@{
#define PF_SUBCLASS_ID_PARTB		0x1eb34000
#define PF_OPERATOR_SUBCLASS_ID 	0x74f93a02
#define PF_OPERATOR3D_SUBCLASS_ID 	0x74f93a03
#define PF_TEST_SUBCLASS_ID 		0x74f93a04
#define PF_TEST3D_SUBCLASS_ID 		0x74f93a05
#define PF_ITEM_SUBCLASS_ID 		0x74f93a06
#define PF_OBJECT_SUBCLASS_ID 		0x74f93a07
//@}

//@} // Closes the \defgroup 

class ISave;
class ILoad;
class Interface;
class ActionTable;
class ParamBlockDesc2;
class IParamBlock2;
class IObjParam;
class Animatable;
class ParamMap2UserDlgProc;
class IParamMap2;
class FPInterface;
class Manipulator;
class ReferenceTarget;

/** 
Class descriptors provide the system with information about the plug-in classes
in the DLL. The developer creates a class descriptor by deriving a class from
ClassDesc2 (which derives from ClassDesc) and implementing several of its methods.
*/
#pragma warning(push)
#pragma warning(disable:4100)
class ClassDesc: public MaxHeapOperators {
	private:
		Tab<FPInterface*>		interfaces;		// the FnPub interfaces published by this plugin
	public:
		virtual					~ClassDesc() {}
		/** Controls if the plug-in shows up in lists from the user to choose from.
		\return  If the plug-in can be picked and assigned by the user, as is
		usually the case, return TRUE. Certain plug-ins may be used privately
		by other plug-ins implemented in the same DLL and should not appear in
		lists for user to choose from. These plug-ins would return FALSE. */
		virtual int				IsPublic()=0;  
		
        /** 3ds Max calls this method when it needs a pointer to a new instance of
		the plug-in class. For example, if 3ds Max is loading a file from disk
		containing a previously used plug-in (procedural object, modifier,
		controller, etc...), it will call the plug-in's Animatable::Create() method.
		The plug-in responds by allocating a new instance of its plug-in class.
		See the Advanced Topic section on
		<a href="ms-its:3dsmaxsdk.chm::/alloc_memory_allocation.html">Memory
		Allocation</a> for more details.
		\param loading This parameter is a flag indicating if the class being created is going
		to be loaded from a disk file. If the flag is TRUE, the plug-in may not
		have to perform any initialization of the object because the loading
		process will take care of it. See the Advanced Topics section on
		<a href="ms-its:3dsmaxsdk.chm::/loading_and_saving.html">Loading
		and Saving</a> for more information.\n\n
		Note: If this parameter is TRUE, developers must initialize their
		references to NULL. Otherwise 3ds Max may crash.\n\n
		3ds Max provides a default plug-in object creation process. Many
		plug-ins fit this form. When the system is about to create an instance
		of the plug-in object it calls a method
		BaseObject::GetCreateMouseCallBack().This method returns a
		callback object whose proc() method handles the mouse input
		during its creation phase. Most of the work is then handled by the
		system. The procedural sphere is an example of this type of plug-in.
		Certain plug-ins may have special creation needs however. The target
		camera is an example of such a plug-in. Because it needs to create two
		nodes in the scene (the camera and the target) it requires a custom
		creation process. To support these plug-ins the following two methods
		are provided. They allow the plug-in to manage the creation process
		themselves. See
		<a href="ms-its:3dsmaxsdk.chm::/nodes_object_creation_methods.html">Object
		Creation Methods</a> for more details. */
		virtual void *			Create(BOOL loading=FALSE)=0;   // return a pointer to an instance of the class.
		
		/** The custom creation process of the plug-in object is handled by this
		method. For example, a plug-in can create a custom command mode and
		push it on the command stack to handle the creation process.\n\n
		Important Note: A plug-in that doesn't want to participate in the
		standard object creation mechanism using CreateMouseCallBack
		must push a CommandMode on the stack in this method and remove
		it in EndCreate(). This is true even if the plug-in doesn't do
		anything inside the mode. A mode has to be pushed on the stack and then
		later popped off otherwise a crash will occur (if the default
		implementation of this method is not used). For more details on object
		creation see the Advanced Topics section
		<a href="ms-its:3dsmaxsdk.chm::/nodes_object_creation_methods.html">Object
		Creation Methods</a>.
		\param i An interface pointer the plug-in may use to call functions in 3ds Max.
		\return  To use the default creation process (the system implementation
		for this method) return 0; Return nonzero if the plug-in implements
		this method. */
		virtual	int 			BeginCreate(Interface *i) {return 0;}
		
		/** The termination of the custom creation process is managed by the
		implementation of this method. For example, the plug-in could remove a
		custom command mode from the command stack. See the Advanced Topics
		section on
		<a href="ms-its:3dsmaxsdk.chm::/nodes_object_creation_methods.html">Object
		Creation Methods</a> for more details.
		\param i An interface pointer the plug-in may use to call functions in 3ds Max.
		\return  To use the system implementation for this method return 0;
		Return nonzero if the plug-in implements this method. */
		virtual int 			EndCreate(Interface *i) {return 0;};
		
		/** This method returns the name of the class. This name appears in the
		button for the plug-in in the 3ds Max user interface.
		\return  The name of the class. */
		virtual const MCHAR* 	ClassName()=0;
		
		/** This method returns a system defined constant describing the class this
		plug-in class was derived from. For example, the Bend modifier returns
		OSM_CLASS_ID. This super class ID is used by all object space
		modifiers. See <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_super_class_ids.html">List of
		SuperClassIDs</a>.
		\return  The SuperClassID of the plug-in. */
		virtual SClass_ID		SuperClassID()=0;
		
		/** This method must return the unique ID for the object. If two
		ClassIDs conflict, the system will only load the first one it finds.
		The ClassID consists of two unsigned 32-bit quantities. The constructor
		assigns a value to each of these, for example Class_ID(0xA1C8E1D1,
		0xE7AA2BE5). A developer should use the random Class_ID generator
		to avoid conflicts (Generate a random Class_ID). See
		Class Class_ID for more information.
		\return  The unique ClassID of the plug-in. */
		virtual Class_ID		ClassID()=0;
		
        /** This methods returns a string describing the category a plug-in fits
		into. The category is usually selected in the drop down list in the
		create, or utility branch of the command panel. In the create branch,
		if this is set to be an existing category (i.e. "Standard Primitives",
		"Splines", ...) then the plug-in will appear in that category. If the
		category doesn't yet exists then it is created. If the plug-in does not
		need to appear in the list, it may simply return a null string as in
		_M("").
		In the modify branch, the category determines which group it appears in
		the Configure Button Sets / Modifiers list. These are the categories
		such as "MAX STANDARD", "MAX EDIT", and "MAX SURFACE".\n\n
		This method is also used to distinguish between the various types of
		texture maps so they can be separated in the Material/Map Browser. The
		appropriate string should be returned by this method of the Texmap. For
		example:
		
        \code
        const MCHAR* Category() { 
            return TEXMAP_CAT_3D; 
        }
        \endcode 

		The options for texture maps are:
		\li MCHAR TEXMAP_CAT_2D[]; -> 2D maps.
		\li MCHAR TEXMAP_CAT_3D[]; - 3D maps.
		\li MCHAR TEXMAP_CAT_COMP[]; - Composite.
		\li MCHAR TEXMAP_CAT_COLMOD[]; - Color modifier.
		\li MCHAR TEXMAP_CAT_ENV[]; - Environment. */
		virtual const MCHAR* 	Category()=0;   // primitive/spline/loft/ etc
		
        /** This method is used to enable or disable the button that allows the
		plug-ins class to be created. For example, at certain times it is not
		appropriate to for the Boolean object to be created. When there is not
		an object of the appropriate type selected the Boolean object cannot be
		created. At these times the button should be disabled (the button will
		appear as grayed out in the Create branch of the command panel). The
		button should be enabled if there is an object of the appropriate type
		selected. This method allows a plug-in to control the state of the
		button.
		\param i An interface pointer the plug-in may use to call functions in 3ds Max.
		\return  TRUE to enable the class creation button; FALSE to disable it.

        \par Sample Code:
		The following code from <b>/MAXSDK/SAMPLES/OBJECTS/BOOLOBJ.CPP</b>
		demonstrates an implementation of this method. If there is not a node
		selected, it is not OK to use the command so the button should appear
		disabled. To disable the button OkToCreate() returns FALSE. If
		the object that is selected is not of the appropriate type it the
		button is disabled as well

		\code
		BOOL BoolObjClassDesc::OkToCreate(Interface *i)
		{
			if (i->GetSelNodeCount()!=1) return FALSE;
			ObjectState os = i->GetSelNode(0)->GetObjectRef()->Eval(i->GetTime());
			if (os.obj->SuperClassID()!=GEOMOBJECT_CLASS_ID) {
				return FALSE;
			}
			return TRUE;
		}
		\endcode */
		virtual BOOL			OkToCreate(Interface *i) { return TRUE; }	// return FALSE to disable create button		
		
        /** If a plug-in class has default parameters that it needs to allow the
		user to edit, TRUE should be returned and EditClassParams() and
		ResetClassParams() should be implemented. Otherwise return FALSE
		(the default). */
		virtual BOOL			HasClassParams() {return FALSE;}
		
		/** If the user picks the class from the list this method is called. The
		plug-in should put up a modal dialog that allows the user to edit the
		plug-ins default parameters. The plug-in should not return until the
		user has finished editing the parameters. 
		\param hParent The parent window handle. */
		virtual void			EditClassParams(HWND hParent) {}
		
		/** When the user executes File / Reset or presses the 'Reset to Factory
		Settings...' button in the File / Preferences... / Animation tab /
		Controller Defaults section this method is called. The plug-in can
		respond by resetting itself to use its default values. 

		\param fileReset When TRUE, the user has performed a File / Reset operation. When FALSE,
		the user is in the Preferences... dialog doing a reset controller
		defaults operation. */
		virtual void			ResetClassParams(BOOL fileReset=FALSE) {}
		
		/** \name ActionTable Methods
		Plug-ins do not need to register their action tables from within these methods;
		they are registered automatically by 3ds Max on behalf of the plug-in. Therefore
		plug-ins are encouraged to report the correct number of action tables via these methods,
		as opposed to reporting 0 but registering the action tables themselves in the 
		implementation of these methods.
		*/
		//@{
		/** 3ds Max calls this to get the number of action tables a plug-in has. 
		If more than one class uses the table only one of the classes should 
		export the table, but they can all use them.
		\see ActionTable */
		virtual int NumActionTables() { return 0; }
		
		/** Returns a pointer to the specified action table. 
		\param i The zero based index of the table to return. 
		\see ActionTable */
		virtual ActionTable* GetActionTable(int i) { return NULL; }
		//@}

		/// \name Manipulator Related Methods
		//@{
		/** Returns TRUE if the class implements
		a manipulator object; otherwise FALSE. */
		virtual BOOL IsManipulator() { return FALSE; }

		/** The method returns true if the class is a manipulator and it
		manipulates the given base object, modifier or controller. When
		starting "Manipulate" mode, this is called on selected nodes for the
		base object, all modifiers, the TM controller and the position,
		rotation and scale controllers, if the TM controller is a
		PRSController.
		\param hTarget A pointer to a reference target. */
		virtual BOOL CanManipulate(ReferenceTarget* hTarget) { return FALSE; }

		/** Returns TRUE if the manipulator applies to the given node; otherwise FALSE.
		This method can be used to indicate that the manipulator works on a part
		of an object that is not covered by BOOL CanManipulate(ReferenceTarget* hTarget)
		such as the visibility controller of a node.
		\param pNode The INode to check. */
		virtual BOOL CanManipulateNode(INode* pNode) { return FALSE; }

		/** Creates a manipulator object
		When a manipulator returns TRUE to CanManipulate(ReferenceTarget* hTarget), the system 
		calls this version of CreateManipulator() to create an instance of the manipulator.
		\param hTarget - The ReferenceTarget for which a manipulator is requested
		\param pNode - The node that the manipulator needs to manipulate (know about)
		\return - Pointer to the newly created manipulator, or NULL if the creation failed.
		\code
		Manipulator* BendManipClassDesc::CreateManipulator(ReferenceTarget* hTarget, INode* node) {
			if (hTarget->ClassID() != Class_ID(BENDOSM_CLASS_ID, 0))
				return NULL;
			return (new BendManip((SimpleMod2*)hTarget, node));
		}
		\endcode
		*/
		virtual Manipulator* CreateManipulator(
			ReferenceTarget* hTarget,
			INode* pNode) { return NULL; }

		/** Creates a manipulator object.
		When a manipulator returns TRUE to CanManipulateNode(INode* pNode), the system 
		calls this version of CreateManipulator() to create an instance of the manipulator.
		\param pNode - The node that the manipulator needs to manipulate (know about)
		\return - Pointer to the newly created manipulator, or NULL if the creation failed.
		*/
		virtual Manipulator* CreateManipulator(INode* pNode) {return NULL;}
		//@}

		/// \name IO Methods
		//@{
		/** Returns TRUE if there is data associated with the class that needs to
		be saved in the 3ds Max file. If this is so, implement the
		Save() and Load() methods below. If there is no class
		data to save return FALSE. */
		virtual BOOL			NeedsToSave() { return FALSE; }
		
		/** If NeedsToSave() returns TRUE then this method should be
		implemented to save the data associated with the class.
		\param isave A pointer that may be used to call methods to save data to disk.
		\return IO_OK if the save was successful; otherwise IO_ERROR. */
		virtual IOResult 		Save(ISave *isave) { return IO_OK; }
		
		/** If NeedsToSave() returns TRUE then this method should be
		implemented to load the data associated with the class.
		\param iload A pointer that may be used to load data from a file.
		\return  IO_OK if the load was successful; otherwise
		IO_ERROR. */
		virtual IOResult 		Load(ILoad *iload) { return IO_OK; }
		//@}

		/** This method returns a DWORD which is used to initialize the rollup
		state in both the create branch and the modify branch. The semantics
		are different, however for these two cases. Whenever the rollups are
		created in the create branch, their state will be that specified by
		this method. In the modify branch, the first time an object of this
		type is modified the state will be that of this method, but after that
		it will remain what it was last set to.
		\return  The bits of this DWORD set indicate the corrresponding rollup
		page is closed. The zero bit corresponds to the plug-ins first rollup,
		the first bit is the second rollup, etc. The value \c 0x7fffffff is
		returned by the default implementation so the command panel can detect
		this method is not being overridden, and just leave the rollups as is. */
		virtual DWORD			InitialRollupPageState() { return 0x7fffffff; }

		/** Returns a string which provides a fixed, machine parsable internal name
		for the plug-in. This name is used by MAXScript. */
		virtual const MCHAR*	InternalName() { return NULL; }
		
		/** Returns the DLL instance handle of the plug-in. This is used so that
		string resources can be loaded by the ParamBlock2 system. */
		virtual HINSTANCE		HInstance() { return NULL; }
		
		/// \name Parameter Block Descriptor Functions
		//@{
		/** Returns the number or ParamBlockDesc2s used by the plug-in. */
		virtual int				NumParamBlockDescs() { return 0; }
		
		/** Returns a pointer to the 'i-th' parameter block 2 descriptor.
		\param i The zero based index of the descriptor to return. */
		virtual ParamBlockDesc2* GetParamBlockDesc(int i) { return NULL; }
		
		/** \remarks Implemented by the System.\n\n
		Returns a pointer to the specified parameter block 2 descriptor.
		\param id The ID of the parameter block. */
		virtual ParamBlockDesc2* GetParamBlockDescByID(BlockID id) { return NULL; }
		
		/** \remarks Implemented by the System.\n\n
		Adds the specified parameter block 2 descriptor to the list of those
		maintained by the class.
		\param pbd Points to the parameter block 2 descriptor to add. */
		virtual void			AddParamBlockDesc(ParamBlockDesc2* pbd) { }
		//@}
		
		/// \name Automatic UI Management
		//@{
		/** Implemented by the System.\n\n
		This method is called to handle the beginning of the automatic command
		panel user interface management provided by the param map 2 system.
		This method is called by the plug-in from its
		Animatable::BeginEditParams() method. The parameters passed to
		that method are simply passed along to this method.
		\param ip The interface pointer passed to the plug-in.
		\param obj Points to the plug-in class calling this method.
		\param flags The flags passed along to the plug-in in Animatable::BeginEditParams().
		\param prev The pointer passed to the plug-in in Animatable::BeginEditParams(). */
		virtual void			BeginEditParams(IObjParam *ip, ReferenceMaker* obj, ULONG flags, Animatable *prev) { }
		
        /** This method is called to handle the ending of the automatic command
		panel user interface management provided by the param map 2 system.
		This method is called by the plug-in from its
		Animatable::EndEditParams() method. The parameters passed to
		that method are simply passed along to this method.
		\param ip The interface pointer passed to the plug-in.
		\param obj Points to the plug-in class calling this method.
		\param flags The flags passed along to the plug-in in Animatable::EndEditParams().
		\param prev The pointer passed to the plug-in in Animatable::EndEditParams(). */
		virtual void			EndEditParams(IObjParam *ip, ReferenceMaker* obj, ULONG flags, Animatable *prev) { }
		
        /** Invalidates the user interface for the rollup or dialog managed by the
		specified descriptor. This will cause the controls in that rollup to be
		redrawn.
		\param pbd Points to the parameter block 2 descriptor whose corresponding UI is invalidated.*/
		virtual void			InvalidateUI(ParamBlockDesc2* pbd) { }
		//@}

        /** Returns a pointer to the string from the resource string table. */
		CoreExport virtual MCHAR* GetRsrcString(INT_PTR id);

		/** This method creates the automatic parameter blocks for the specified
		plug-in. These are the ones with the P_AUTO_CONSTRUCT bit set on the ParamBlockDesc2::flags value.
		\param owner Points to the owner of the parameter block. */
		virtual void			MakeAutoParamBlocks(ReferenceMaker* owner) { }

		/// \name Parameter Map Functions
		//@{
		/** Returns the number of parameter map2s used by the plug-in. */
		virtual int				NumParamMaps() { return 0; }

		/** Returns a pointer to the nth parameter map2.
		\param i The zero based index of the parameter map2 to return. */
		virtual IParamMap2*		GetParamMap(int i) { return NULL; }

		/** Returns a pointer to the parameter map2 whose descriptor is passed.
		\param pbd Points to the parameter block2 descriptor. */
		virtual IParamMap2*		GetParamMap(ParamBlockDesc2* pbd) { return NULL; }
		
		/** Sets the parameter map 2 user dialog proc for the specified descriptor.
		\param pbd Points to the parameter block 2 descriptor.
		\param proc This object manages user interface control that require special
		processing. 
        \see ParamMap2UserDlgProc.*/
		virtual void			SetUserDlgProc(ParamBlockDesc2* pbd, ParamMap2UserDlgProc* proc=NULL) { }

		/** Returns a pointer to the parameter map 2 user dialog proc (if any) for
		the specified descriptor.
		\param pbd Points to the parameter block 2 descriptor.
		\return  See Class ParamMap2UserDlgProc.*/
		virtual ParamMap2UserDlgProc* GetUserDlgProc(ParamBlockDesc2* pbd) { return NULL; }
		//@}

		/** Allows a plug-in to provide a custom image for
		display in Schematic View.
		\param bkColor The background color. See <a href="ms-its:listsandfunctions.chm::/idx_R_colorref.html">COLORREF-DWORD format</a>.
		\param hDC The handle for the device context.
		\param rect The rectangle to draw in.
		\return  TRUE if this class can draw an image to represent itself
		graphically; otherwise FALSE. */
		virtual bool DrawRepresentation(COLORREF bkColor, HDC hDC, Rect& rect) { return FALSE; }

        /// \name Function publishing methods
        //@{
        /** Returns the number of function publishing interfaces maintained by the class descriptor. */
		virtual int				NumInterfaces() { return interfaces.Count(); }

        /** Returns the nth function publishing interface. */ 
		virtual FPInterface*	GetInterfaceAt(int i) { return interfaces[i]; }

		/** Returns a pointer to the function publishing interface whose ID is specified.
		\param id The inteface ID. */
		CoreExport virtual FPInterface*	GetInterface(Interface_ID id);

		/** Returns a pointer to the function publishing interface whose name is
		specified.
		\param name The name of the interface. */
		CoreExport virtual FPInterface*	GetInterface(MCHAR* name);
		
        /** Adds the specified interface to the list maintained by this class
		descriptor.
		\param fpi Points to the interface to add. */
		CoreExport virtual void	AddInterface(FPInterface* fpi);
		
        /** Deletes all the interfaces maintained by the class descriptor. */
		virtual void			ClearInterfaces() { interfaces.ZeroCount(); }
        //@}
		
        /** This method can be used for further categorizing plugins. If a plugin
		has sub-plugins (like light \> shadows, particles \> operators), this
		method can be used to differentiate them. sub-plugins can be derived
		from ReferenceTarget but return a particular class ID published by the
		parent plugins SDK headers. Then parent plugin can get a list of all
		reference targets whose SubClassID matches the published SubClassID. */
		virtual Class_ID		SubClassID() { return Class_ID(); }

		/** This function is maintained so the 3ds Max SDK can be extended without breaking backwards compatibility. 
            The behavior of this function depends on the \c cmd parameter:
            \li I_EXEC_CTRL_BYPASS_TREE_VIEW - Returning 1 will hide a controller in track view.
            \li I_EXEC_GET_SOA_STATE - The \c arg1 parameter is expected to be a bool*. The bool gets set to true or false 
                depending whether sub-obj anim is or not enabled for that object type.
                Returns TRUE if the arg1 is set successfully, otherwise FALSE.
            \li I_EXEC_EVAL_SOA_TIME - The \c arg1 parameter is expected to be a TimeValue* The TimeValue gets set appropriately,
                depending on whether sub-obj animation is allowed of not for that object type.
                Returns TRUE if the arg1 is set successfully, otherwise FALSE. 
        */
		virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) { return 0; } 
	};


#pragma warning(pop)

/// \name ClassDesc::Execute() Command Values 
/// These values may be passed as the \c cmd argument of the ClassDesc::Execute() function.
//@{
#define I_EXEC_CTRL_BYPASS_TREE_VIEW 1001 
#define I_EXEC_GET_SOA_STATE 1002 
#define I_EXEC_EVAL_SOA_TIME 1003
//@}

//! Creates an instance of the specified class.
CoreExport void* CreateInstance(SClass_ID superID, Class_ID classID);

/** CallParam data for NOTIFY_CLASSDESC_REPLACED broadcasts.
    A pointer to an instance of this structure is passed to BroadcastNotification after a ClassDesc is 
    replaced by another one in a ClassDirectory. This occurs when the dll containing a deferred loaded plugin 
    is loaded.
    \see NOTIFY_CLASSDESC_REPLACED */
struct ClassDescReplaced: public MaxHeapOperators {  
	//! The old class descriptor 
	const ClassDesc* oldClassDesc;
	//! The new class descriptor
	const ClassDesc* newClassDesc;
}; 

