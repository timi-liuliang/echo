//
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.  
//
//

#pragma once

// Parameter types
//    several of these types fold to the same underlying value type but provide
//    either a more complete description of the type or imply non-standard dimensions that are
//    automatically applied.

#define TYPE_TAB	0x0800		//!< Flags a parameter type as a Tab<>

/// \name Function Publishing Type Modification Flags
//@{
#define TYPE_BY_REF	0x1000		//!< Flags a parameter as being delivered by reference (&).
#define TYPE_BY_VAL	0x2000		//!< Flags a parameter as being delivered by value (via a local copy owned by the FPValue).
#define TYPE_BY_PTR	0x4000		//!< Flags a parameter as being delivered by pointer (*)
//@}

/** Identifies a parameter type.
 * \see ParamBlockDesc2, ParamType2*/
enum ParamType {
	TYPE_FLOAT,     //!< Identifies a floating point parameter type.
	TYPE_INT,       //!< Identifies an integer parameter type.
	TYPE_RGBA,      //!< Identifies a Color parameter type. \note Does not contain an alpha channel despite the name.
	TYPE_POINT3,    //!< Identifies a Point3 parameter type.
	TYPE_BOOL,      //!< Identifies a BOOL parameter type.
	TYPE_USER,      //!< \deprecated 
	};

/** Identifies a parameter type. ParamType values are compatible with ParamType2.
 * \li The enumerated types from "TYPE_ENUM" to "TYPE_DOUBLE" are for published function parameter types only, not pblock2 parameter types.
 * \li The enumerated types from "TYPE_FLOAT_TAB" to "TYPE_FRGBA_TAB" are the tables (Tab<>s) of their base types, and must be in same order as the base types.
 * \li The enumerated types from "TYPE_ENUM_TAB" to "TYPE_DOUBLE_TAB" are for published function parameter types only, not pblock2 parameter types.
 * \li The enumerated types from "TYPE_FLOAT_BR" to "TYPE_DOUBLE_BR" are pass by-ref types, implies & parameters, int& & float& are passed via .ptr fields, only for FnPub use.
 * \li The enumerated types from "TYPE_FLOAT_TAB_BR" to "TYPE_DOUBLE_TAB_BR" are pass by-ref Tab<> types - tab is passed by ref, data type in tab is TYPE_XXXX.
 * For example: TYPE_FLOAT_TAB_BR = Tab<float>& , TYPE_INODE_TAB_BR_TYPE = Tab<INode*>&
 * \li The enumerated types from "TYPE_RGBA_BV" to "TYPE_CLASS_BV" are pass by-value types, implies dereferencing the (meaningful) pointer-based values, only for FnPub use.
 * \li The enumerated types from "TYPE_FLOAT_TAB_BV" to "TYPE_DOUBLE_TAB_BV" are pass by-val Tab<> types - tab is passed by val, data type in tab is TYPE_XXXX.
 * For example: TYPE_FLOAT_TAB_BV = Tab<float> , TYPE_INODE_TAB_BV = Tab<INode*>
 * \li The enumerated types from "TYPE_FLOAT_BP" to "TYPE_DOUBLE_BP" are pass by-pointer types for int & float types, implies * parameters, int* & float* are passed via .ptr fields, only for FnPub use.
 * \li The enumerated types from "TYPE_KEYARG_MARKER" to "TYPE_UNSPECIFIED" are MAXScript internal types
 * \see ParamBlockDesc2, ParamType
 */
enum ParamType2 
{
 // TYPE_FLOAT,  
 // TYPE_INT,
 // TYPE_RGBA,
 // TYPE_POINT3,
 // TYPE_BOOL,
	TYPE_ANGLE = TYPE_BOOL + 1, //!< A floating point value with an implied stdAngleDim dimension.
	TYPE_PCNT_FRAC,				//!< A floating point with an implied stdPercentDim dimension.
	TYPE_WORLD,					//!< A floating point specifying a parameter represents world distance units. This implies a parameter dimension of stdWorldDim.
	TYPE_STRING,				//!< A character string. The string has a local copy made and managed by the parameter block.
	TYPE_FILENAME,				//!< Used to identify file names (MCHAR*). 
	TYPE_HSV,					//!< This option is obsolete.
	TYPE_COLOR_CHANNEL,			//!< A single floating point value with an implied stdColor255Dim dimension.
	TYPE_TIMEVALUE,				//!< A single integer value used as a TimeValue -- implies a stdTimeDim dimension.
	TYPE_RADIOBTN_INDEX,		//!< This is used as integer for parameters represented as radio buttons in the UI.
	TYPE_MTL,					//!< A pointer to a material object. This can be one of three types: a reference owned by the parameter block, a reference owned by the block owner, or no reference management (just a copy of the pointer).
	TYPE_TEXMAP,				//!< A pointer to a texture map object. This can be one of three types: a reference owned by the parameter block, a reference owned by the block owner, or no reference management (just a copy of the pointer). 
	TYPE_BITMAP,				//!< A pointer to a bitmap object. This can be one of three types: a reference owned by the parameter block, a reference owned by the block owner, or no reference management (just a copy of the pointer).
	TYPE_INODE,					//!< A pointer to a node. This can be one of three types: a reference owned by the parameter block, a reference owned by the block owner, or no reference management (just a copy of the pointer). 
	TYPE_REFTARG,				//!< A pointer to a reference target. All reference targets in this group can be one of three types: reference owned by parameter block, reference owned by block owner, or no reference management (just a copy of the pointer).
	// new for R4...
	TYPE_INDEX,					//!< Used for parameters that are 0-based, but exposed to MAXScript as 1-based. For example a vertex index.
	TYPE_MATRIX3,				//!< A standard 3ds Max matrix.
	TYPE_PBLOCK2,				//!< A pointer to an IParamBlock2 object. Note "TYPE_PBLOCK2_TYPE" is not defined.
	// new for R6...
	TYPE_POINT4,
	TYPE_FRGBA,

	// only for published function parameter types, not pblock2 parameter types...
	TYPE_ENUM,
	TYPE_VOID,
	TYPE_INTERVAL,
	TYPE_ANGAXIS,
	TYPE_QUAT,
	TYPE_RAY,
	TYPE_POINT2,
	TYPE_BITARRAY,
	TYPE_CLASS,
	TYPE_MESH,
	TYPE_OBJECT,
	TYPE_CONTROL,
	TYPE_POINT,
	TYPE_TSTR,
	TYPE_IOBJECT,
	TYPE_INTERFACE,
	TYPE_HWND,
	TYPE_NAME,
	TYPE_COLOR,
	TYPE_FPVALUE,
	TYPE_VALUE,
	TYPE_DWORD,
	TYPE_bool,
	TYPE_INTPTR,
	TYPE_INT64,
	TYPE_DOUBLE,

	// Tab<>s of above  (MUST be in same order as base types)
	TYPE_FLOAT_TAB = TYPE_FLOAT + TYPE_TAB, //!< A table of floating point values.
	TYPE_INT_TAB,			//!< A table of integer values
	TYPE_RGBA_TAB,			//!< A table of Point3 values with an implied stdColor255Dim dimension.
	TYPE_POINT3_TAB,		//!< A table of Point3 data type values.
	TYPE_BOOL_TAB,			//!< A table of integers used as a set of boolean values.
	TYPE_ANGLE_TAB,			//!< A table of floating point value with an implied stdAngleDim dimension.
	TYPE_PCNT_FRAC_TAB,		//!< A table of same as TYPE_STRING, but is used with TYPE_FILEOPENBUTTON and TYPE_FILESAVEBUTTON parameter map controls.
	TYPE_WORLD_TAB,			//!< A table of parameters that represents world distance units. This implies a parameter dimension of stdWorldDim.
	TYPE_STRING_TAB,		//!< A table of character strings (TCHAR*).
	TYPE_FILENAME_TAB,		//!< A table of filenames (see TYPE_FILENAME).
	TYPE_HSV_TAB,			//!< This option is obsolete.
	TYPE_COLOR_CHANNEL_TAB,	//!< A table of floating point values with an implied stdColor255Dim dimension.
	TYPE_TIMEVALUE_TAB,		//!< A table of integer value used as a TimeValue -- implies stdTimeDim dimension.
	TYPE_RADIOBTN_INDEX_TAB,//!< This is currently unused.
	TYPE_MTL_TAB,			//!< A table of material object pointers (see TYPE_MTL).
	TYPE_TEXMAP_TAB,		//!< A table of texmap object pointers (see TYPE_TEXMAP).
	TYPE_BITMAP_TAB,		//!< A table of TYPE_BITMAP values.
	TYPE_INODE_TAB,			//!< A table of TYPE_INODE values.
	TYPE_REFTARG_TAB,		//!< A table of TYPE_REFTARG values.
	// new for R4...
	TYPE_INDEX_TAB,			//!< A table of TYPE_INDEX values. 
	TYPE_MATRIX3_TAB,		//!< A table of TYPE_MATRIX3 values.
	TYPE_PBLOCK2_TAB,		//!< A table of TYPE_BLOCK2 values.
	// new for R6...
	TYPE_POINT4_TAB,
	TYPE_FRGBA_TAB,
    // Only for published function parameter types, not pblock2 parameter types.
	TYPE_ENUM_TAB,		
	TYPE_VOID_TAB,		
	TYPE_INTERVAL_TAB,	
	TYPE_ANGAXIS_TAB,	
	TYPE_QUAT_TAB,		
	TYPE_RAY_TAB,		
	TYPE_POINT2_TAB,	
	TYPE_BITARRAY_TAB,	
	TYPE_CLASS_TAB,		
	TYPE_MESH_TAB,		
	TYPE_OBJECT_TAB,	
	TYPE_CONTROL_TAB,	
	TYPE_POINT_TAB,		
	TYPE_TSTR_TAB,		
	TYPE_IOBJECT_TAB,	
	TYPE_INTERFACE_TAB,	
	TYPE_HWND_TAB,		
	TYPE_NAME_TAB,		
	TYPE_COLOR_TAB,		
	TYPE_FPVALUE_TAB,	
	TYPE_VALUE_TAB,		
	TYPE_DWORD_TAB,		
	TYPE_bool_TAB,		
	TYPE_INTPTR_TAB,	
	TYPE_INT64_TAB,		
	TYPE_DOUBLE_TAB,	

	// pass by-ref types, implies & parameters, int& & float& are passed via .ptr fields, only for FnPub use

	TYPE_FLOAT_BR					= TYPE_FLOAT + TYPE_BY_REF,			
	TYPE_INT_BR						= TYPE_INT + TYPE_BY_REF,			
	TYPE_BOOL_BR 					= TYPE_BOOL + TYPE_BY_REF,			
	TYPE_ANGLE_BR 					= TYPE_ANGLE + TYPE_BY_REF,			
	TYPE_PCNT_FRAC_BR 				= TYPE_PCNT_FRAC + TYPE_BY_REF,		
	TYPE_WORLD_BR 					= TYPE_WORLD + TYPE_BY_REF,			
	TYPE_COLOR_CHANNEL_BR 			= TYPE_COLOR_CHANNEL + TYPE_BY_REF,	
	TYPE_TIMEVALUE_BR 				= TYPE_TIMEVALUE + TYPE_BY_REF,		
	TYPE_RADIOBTN_INDEX_BR 			= TYPE_RADIOBTN_INDEX + TYPE_BY_REF,
	TYPE_INDEX_BR 					= TYPE_INDEX + TYPE_BY_REF,			
	TYPE_RGBA_BR 					= TYPE_RGBA + TYPE_BY_REF,			
	TYPE_BITMAP_BR 					= TYPE_BITMAP + TYPE_BY_REF,		
	TYPE_POINT3_BR 					= TYPE_POINT3 + TYPE_BY_REF,		
	TYPE_HSV_BR 					= TYPE_HSV + TYPE_BY_REF,			
	TYPE_REFTARG_BR 				= TYPE_REFTARG + TYPE_BY_REF,		
	TYPE_MATRIX3_BR 				= TYPE_MATRIX3 + TYPE_BY_REF,		
	TYPE_POINT4_BR 					= TYPE_POINT4 + TYPE_BY_REF,		
	TYPE_FRGBA_BR 					= TYPE_FRGBA + TYPE_BY_REF,			
	TYPE_ENUM_BR 					= TYPE_ENUM + TYPE_BY_REF,			
	TYPE_INTERVAL_BR 				= TYPE_INTERVAL + TYPE_BY_REF,		
	TYPE_ANGAXIS_BR 				= TYPE_ANGAXIS + TYPE_BY_REF,		
	TYPE_QUAT_BR 					= TYPE_QUAT + TYPE_BY_REF,			
	TYPE_RAY_BR 					= TYPE_RAY + TYPE_BY_REF,			
	TYPE_POINT2_BR 					= TYPE_POINT2 + TYPE_BY_REF,		
	TYPE_BITARRAY_BR 				= TYPE_BITARRAY + TYPE_BY_REF,		
	TYPE_MESH_BR 					= TYPE_MESH + TYPE_BY_REF,			
	TYPE_POINT_BR 					= TYPE_POINT + TYPE_BY_REF,			
	TYPE_TSTR_BR 					= TYPE_TSTR + TYPE_BY_REF,			
	TYPE_COLOR_BR 					= TYPE_COLOR + TYPE_BY_REF,			
	TYPE_FPVALUE_BR 				= TYPE_FPVALUE + TYPE_BY_REF,		
	TYPE_DWORD_BR 					= TYPE_DWORD + TYPE_BY_REF,			
	TYPE_bool_BR 					= TYPE_bool + TYPE_BY_REF,			
	TYPE_INTPTR_BR					= TYPE_INTPTR + TYPE_BY_REF,		
	TYPE_INT64_BR					= TYPE_INT64 + TYPE_BY_REF,			
	TYPE_DOUBLE_BR					= TYPE_DOUBLE + TYPE_BY_REF,		

	

	// pass by-ref Tab<> types - tab is passed by ref, data type in tab is TYPE_XXXX.
	// For example: TYPE_FLOAT_TAB_BR = Tab<float>& , TYPE_INODE_TAB_BR_TYPE = Tab<INode*>&
	TYPE_FLOAT_TAB_BR				= TYPE_FLOAT + TYPE_TAB + TYPE_BY_REF, 
	TYPE_INT_TAB_BR					= TYPE_INT + TYPE_TAB + TYPE_BY_REF,
	TYPE_RGBA_TAB_BR				= TYPE_RGBA + TYPE_TAB + TYPE_BY_REF,
	TYPE_POINT3_TAB_BR				= TYPE_POINT3 + TYPE_TAB + TYPE_BY_REF,
	TYPE_BOOL_TAB_BR				= TYPE_BOOL + TYPE_TAB + TYPE_BY_REF,
	TYPE_ANGLE_TAB_BR				= TYPE_ANGLE + TYPE_TAB + TYPE_BY_REF,
	TYPE_PCNT_FRAC_TAB_BR			= TYPE_PCNT_FRAC + TYPE_TAB + TYPE_BY_REF,
	TYPE_WORLD_TAB_BR				= TYPE_WORLD + TYPE_TAB + TYPE_BY_REF,
	TYPE_STRING_TAB_BR				= TYPE_STRING + TYPE_TAB + TYPE_BY_REF,
	TYPE_FILENAME_TAB_BR			= TYPE_FILENAME + TYPE_TAB + TYPE_BY_REF,
	TYPE_HSV_TAB_BR					= TYPE_HSV + TYPE_TAB + TYPE_BY_REF,
	TYPE_COLOR_CHANNEL_TAB_BR		= TYPE_COLOR_CHANNEL + TYPE_TAB + TYPE_BY_REF,
	TYPE_TIMEVALUE_TAB_BR			= TYPE_TIMEVALUE + TYPE_TAB + TYPE_BY_REF,
	TYPE_RADIOBTN_INDEX_TAB_BR		= TYPE_RADIOBTN_INDEX + TYPE_TAB + TYPE_BY_REF,
	TYPE_MTL_TAB_BR					= TYPE_MTL + TYPE_TAB + TYPE_BY_REF,
	TYPE_TEXMAP_TAB_BR				= TYPE_TEXMAP + TYPE_TAB + TYPE_BY_REF,
	TYPE_BITMAP_TAB_BR				= TYPE_BITMAP + TYPE_TAB + TYPE_BY_REF,
	TYPE_INODE_TAB_BR				= TYPE_INODE + TYPE_TAB + TYPE_BY_REF,
	TYPE_REFTARG_TAB_BR				= TYPE_REFTARG + TYPE_TAB + TYPE_BY_REF,
	TYPE_INDEX_TAB_BR				= TYPE_INDEX + TYPE_TAB + TYPE_BY_REF,
	TYPE_MATRIX3_TAB_BR				= TYPE_MATRIX3 + TYPE_TAB + TYPE_BY_REF,
	TYPE_POINT4_TAB_BR 				= TYPE_POINT4 + TYPE_TAB + TYPE_BY_REF,
	TYPE_FRGBA_TAB_BR 				= TYPE_FRGBA + TYPE_TAB + TYPE_BY_REF,
	TYPE_TSTR_TAB_BR				= TYPE_TSTR + TYPE_TAB + TYPE_BY_REF,
	TYPE_ENUM_TAB_BR				= TYPE_ENUM + TYPE_TAB + TYPE_BY_REF,
	TYPE_INTERVAL_TAB_BR			= TYPE_INTERVAL + TYPE_TAB + TYPE_BY_REF,
	TYPE_ANGAXIS_TAB_BR				= TYPE_ANGAXIS + TYPE_TAB + TYPE_BY_REF,
	TYPE_QUAT_TAB_BR				= TYPE_QUAT + TYPE_TAB + TYPE_BY_REF,
	TYPE_RAY_TAB_BR					= TYPE_RAY + TYPE_TAB + TYPE_BY_REF,
	TYPE_POINT2_TAB_BR				= TYPE_POINT2 + TYPE_TAB + TYPE_BY_REF,
	TYPE_BITARRAY_TAB_BR			= TYPE_BITARRAY + TYPE_TAB + TYPE_BY_REF,
	TYPE_CLASS_TAB_BR				= TYPE_CLASS + TYPE_TAB + TYPE_BY_REF,
	TYPE_MESH_TAB_BR				= TYPE_MESH + TYPE_TAB + TYPE_BY_REF,
	TYPE_OBJECT_TAB_BR				= TYPE_OBJECT + TYPE_TAB + TYPE_BY_REF,
	TYPE_CONTROL_TAB_BR				= TYPE_CONTROL + TYPE_TAB + TYPE_BY_REF,
	TYPE_POINT_TAB_BR				= TYPE_POINT + TYPE_TAB + TYPE_BY_REF,
	TYPE_IOBJECT_TAB_BR				= TYPE_IOBJECT + TYPE_TAB + TYPE_BY_REF,
	TYPE_INTERFACE_TAB_BR			= TYPE_INTERFACE + TYPE_TAB + TYPE_BY_REF,
	TYPE_HWND_TAB_BR				= TYPE_HWND + TYPE_TAB + TYPE_BY_REF,
	TYPE_NAME_TAB_BR				= TYPE_NAME + TYPE_TAB + TYPE_BY_REF,
	TYPE_COLOR_TAB_BR				= TYPE_COLOR + TYPE_TAB + TYPE_BY_REF,
	TYPE_FPVALUE_TAB_BR				= TYPE_FPVALUE + TYPE_TAB + TYPE_BY_REF,
	TYPE_VALUE_TAB_BR				= TYPE_VALUE + TYPE_TAB + TYPE_BY_REF,
	TYPE_DWORD_TAB_BR				= TYPE_DWORD + TYPE_TAB + TYPE_BY_REF,
	TYPE_bool_TAB_BR				= TYPE_bool + TYPE_TAB + TYPE_BY_REF,
	TYPE_INTPTR_TAB_BR				= TYPE_INTPTR + TYPE_TAB + TYPE_BY_REF,
	TYPE_INT64_TAB_BR				= TYPE_INT64 + TYPE_TAB + TYPE_BY_REF,
	TYPE_DOUBLE_TAB_BR				= TYPE_DOUBLE + TYPE_TAB + TYPE_BY_REF,

	// pass by-value types, implies dereferencing the (meaningful) pointer-based values, only for FnPub use
	TYPE_RGBA_BV					= TYPE_RGBA + TYPE_BY_VAL, //[TBA]
	TYPE_POINT3_BV					= TYPE_POINT3 + TYPE_BY_VAL,
	TYPE_HSV_BV						= TYPE_HSV + TYPE_BY_VAL,
	TYPE_INTERVAL_BV				= TYPE_INTERVAL + TYPE_BY_VAL,
	TYPE_BITMAP_BV					= TYPE_BITMAP + TYPE_BY_VAL,
	TYPE_MATRIX3_BV					= TYPE_MATRIX3 + TYPE_BY_VAL,
	TYPE_POINT4_BV 					= TYPE_POINT4 + TYPE_BY_VAL,
	TYPE_FRGBA_BV 					= TYPE_FRGBA + TYPE_BY_VAL,
	TYPE_ANGAXIS_BV					= TYPE_ANGAXIS + TYPE_BY_VAL,
	TYPE_QUAT_BV					= TYPE_QUAT + TYPE_BY_VAL,
	TYPE_RAY_BV						= TYPE_RAY + TYPE_BY_VAL,
	TYPE_POINT2_BV					= TYPE_POINT2 + TYPE_BY_VAL,
	TYPE_BITARRAY_BV				= TYPE_BITARRAY + TYPE_BY_VAL,
	TYPE_MESH_BV					= TYPE_MESH + TYPE_BY_VAL,
	TYPE_POINT_BV					= TYPE_POINT + TYPE_BY_VAL,
	TYPE_TSTR_BV					= TYPE_TSTR + TYPE_BY_VAL,
	TYPE_COLOR_BV					= TYPE_COLOR + TYPE_BY_VAL,
	TYPE_FPVALUE_BV					= TYPE_FPVALUE + TYPE_BY_VAL,
	TYPE_CLASS_BV					= TYPE_CLASS + TYPE_BY_VAL,

	// pass by-val Tab<> types - tab is passed by val, data type in tab is TYPE_XXXX.
	// For example: TYPE_FLOAT_TAB_BV = Tab<float> , TYPE_INODE_TAB_BV = Tab<INode*>
	TYPE_FLOAT_TAB_BV				= TYPE_FLOAT + TYPE_TAB + TYPE_BY_VAL, //[TBA]
	TYPE_INT_TAB_BV					= TYPE_INT + TYPE_TAB + TYPE_BY_VAL,
	TYPE_RGBA_TAB_BV				= TYPE_RGBA + TYPE_TAB + TYPE_BY_VAL,
	TYPE_POINT3_TAB_BV				= TYPE_POINT3 + TYPE_TAB + TYPE_BY_VAL,
	TYPE_BOOL_TAB_BV				= TYPE_BOOL + TYPE_TAB + TYPE_BY_VAL,
	TYPE_ANGLE_TAB_BV				= TYPE_ANGLE + TYPE_TAB + TYPE_BY_VAL,
	TYPE_PCNT_FRAC_TAB_BV			= TYPE_PCNT_FRAC + TYPE_TAB + TYPE_BY_VAL,
	TYPE_WORLD_TAB_BV				= TYPE_WORLD + TYPE_TAB + TYPE_BY_VAL,
	TYPE_STRING_TAB_BV				= TYPE_STRING + TYPE_TAB + TYPE_BY_VAL,
	TYPE_FILENAME_TAB_BV			= TYPE_FILENAME + TYPE_TAB + TYPE_BY_VAL,
	TYPE_HSV_TAB_BV					= TYPE_HSV + TYPE_TAB + TYPE_BY_VAL,
	TYPE_COLOR_CHANNEL_TAB_BV		= TYPE_COLOR_CHANNEL + TYPE_TAB + TYPE_BY_VAL,
	TYPE_TIMEVALUE_TAB_BV			= TYPE_TIMEVALUE + TYPE_TAB + TYPE_BY_VAL,
	TYPE_RADIOBTN_INDEX_TAB_BV		= TYPE_RADIOBTN_INDEX + TYPE_TAB + TYPE_BY_VAL,
	TYPE_MTL_TAB_BV					= TYPE_MTL + TYPE_TAB + TYPE_BY_VAL,
	TYPE_TEXMAP_TAB_BV				= TYPE_TEXMAP + TYPE_TAB + TYPE_BY_VAL,
	TYPE_BITMAP_TAB_BV				= TYPE_BITMAP + TYPE_TAB + TYPE_BY_VAL,
	TYPE_INODE_TAB_BV				= TYPE_INODE + TYPE_TAB + TYPE_BY_VAL,
	TYPE_REFTARG_TAB_BV				= TYPE_REFTARG + TYPE_TAB + TYPE_BY_VAL,
	TYPE_INDEX_TAB_BV				= TYPE_INDEX + TYPE_TAB + TYPE_BY_VAL,
	TYPE_MATRIX3_TAB_BV				= TYPE_MATRIX3 + TYPE_TAB + TYPE_BY_VAL,
	TYPE_POINT4_TAB_BV 				= TYPE_POINT4 + TYPE_TAB + TYPE_BY_VAL,
	TYPE_FRGBA_TAB_BV 				= TYPE_FRGBA + TYPE_TAB + TYPE_BY_VAL,
	TYPE_PBLOCK2_TAB_BV				= TYPE_PBLOCK2 + TYPE_TAB + TYPE_BY_VAL,
	TYPE_VOID_TAB_BV				= TYPE_VOID + TYPE_TAB + TYPE_BY_VAL,
	TYPE_TSTR_TAB_BV				= TYPE_TSTR + TYPE_TAB + TYPE_BY_VAL,
	TYPE_ENUM_TAB_BV				= TYPE_ENUM + TYPE_TAB + TYPE_BY_VAL,
	TYPE_INTERVAL_TAB_BV			= TYPE_INTERVAL + TYPE_TAB + TYPE_BY_VAL,
	TYPE_ANGAXIS_TAB_BV				= TYPE_ANGAXIS + TYPE_TAB + TYPE_BY_VAL,
	TYPE_QUAT_TAB_BV				= TYPE_QUAT + TYPE_TAB + TYPE_BY_VAL,
	TYPE_RAY_TAB_BV					= TYPE_RAY + TYPE_TAB + TYPE_BY_VAL,
	TYPE_POINT2_TAB_BV				= TYPE_POINT2 + TYPE_TAB + TYPE_BY_VAL,
	TYPE_BITARRAY_TAB_BV			= TYPE_BITARRAY + TYPE_TAB + TYPE_BY_VAL,
	TYPE_CLASS_TAB_BV				= TYPE_CLASS + TYPE_TAB + TYPE_BY_VAL,
	TYPE_MESH_TAB_BV				= TYPE_MESH + TYPE_TAB + TYPE_BY_VAL,
	TYPE_OBJECT_TAB_BV				= TYPE_OBJECT + TYPE_TAB + TYPE_BY_VAL,
	TYPE_CONTROL_TAB_BV				= TYPE_CONTROL + TYPE_TAB + TYPE_BY_VAL,
	TYPE_POINT_TAB_BV				= TYPE_POINT + TYPE_TAB + TYPE_BY_VAL,
	TYPE_IOBJECT_TAB_BV				= TYPE_IOBJECT + TYPE_TAB + TYPE_BY_VAL,
	TYPE_INTERFACE_TAB_BV			= TYPE_INTERFACE + TYPE_TAB + TYPE_BY_VAL,
	TYPE_HWND_TAB_BV				= TYPE_HWND + TYPE_TAB + TYPE_BY_VAL,
	TYPE_NAME_TAB_BV				= TYPE_NAME + TYPE_TAB + TYPE_BY_VAL,
	TYPE_COLOR_TAB_BV				= TYPE_COLOR + TYPE_TAB + TYPE_BY_VAL,
	TYPE_FPVALUE_TAB_BV				= TYPE_FPVALUE + TYPE_TAB + TYPE_BY_VAL,
	TYPE_VALUE_TAB_BV				= TYPE_VALUE + TYPE_TAB + TYPE_BY_VAL,
	TYPE_DWORD_TAB_BV				= TYPE_DWORD + TYPE_TAB + TYPE_BY_VAL,
	TYPE_bool_TAB_BV				= TYPE_bool + TYPE_TAB + TYPE_BY_VAL,
	TYPE_INTPTR_TAB_BV				= TYPE_INTPTR + TYPE_TAB + TYPE_BY_VAL,
	TYPE_INT64_TAB_BV				= TYPE_INT64 + TYPE_TAB + TYPE_BY_VAL,
	TYPE_DOUBLE_TAB_BV				= TYPE_DOUBLE + TYPE_TAB + TYPE_BY_VAL,

	// pass by-pointer types for int & float types, implies * parameters, int* & float* are passed via .ptr fields, only for FnPub use
	TYPE_FLOAT_BP					= TYPE_FLOAT + TYPE_BY_PTR, //[TBA]
	TYPE_INT_BP						= TYPE_INT + TYPE_BY_PTR,
	TYPE_BOOL_BP					= TYPE_BOOL + TYPE_BY_PTR,
	TYPE_ANGLE_BP					= TYPE_ANGLE + TYPE_BY_PTR,
	TYPE_PCNT_FRAC_BP				= TYPE_PCNT_FRAC + TYPE_BY_PTR,
	TYPE_WORLD_BP					= TYPE_WORLD + TYPE_BY_PTR,
	TYPE_COLOR_CHANNEL_BP			= TYPE_COLOR_CHANNEL + TYPE_BY_PTR,
	TYPE_TIMEVALUE_BP				= TYPE_TIMEVALUE + TYPE_BY_PTR,
	TYPE_RADIOBTN_INDEX_BP			= TYPE_RADIOBTN_INDEX + TYPE_BY_PTR,
	TYPE_INDEX_BP					= TYPE_INDEX + TYPE_BY_PTR,
	TYPE_ENUM_BP					= TYPE_ENUM + TYPE_BY_PTR,
	TYPE_DWORD_BP					= TYPE_DWORD + TYPE_BY_PTR,
	TYPE_bool_BP					= TYPE_bool + TYPE_BY_PTR,
	TYPE_INTPTR_BP					= TYPE_INTPTR + TYPE_BY_PTR,
	TYPE_INT64_BP					= TYPE_INT64 + TYPE_BY_PTR,
	TYPE_DOUBLE_BP					= TYPE_DOUBLE + TYPE_BY_PTR,

	// there are no specific by-pointer Tab<> types, all Tab<> types are by-pointer by default

	TYPE_MAX_TYPE,	
	
	// MAXScript internal types
	TYPE_KEYARG_MARKER = 253,
	TYPE_MSFLOAT,
	TYPE_UNSPECIFIED, 
	
};

/** Used to associated automatically generated UI controls with parameters in a ParamBlockDesc2.
    \see ControlType2, ParamBlockDesc2 
    \guide{Parameter UI Control Types}
*/ 
enum ControlType {
	/** Identifies a spinner control. The spinner control is used to provide input of values limited to a fixed numeric type. For example, the control may be 
        limited to the input of only positive integers. The input options are integer, float, universe (world space units), 
        positive integer, positive float, positive universe, and time. This control allows the user to increment or decrement a 
        value by clicking on the up or down arrows. The user may also click and drag on the arrows to interactively adjust the value. 
        The Ctrl key may be held to accelerate the value changing speed, while the Alt key may be held to decrease the value changing speed. 
        \guide{Spinner and Slider Control Types} */
    TYPE_SPINNER, 

    /** Identifies a radio button control. Radio buttons are used to provide the user with a single boolean choice, or when used in groups, to select 
        among several options. These are the standard Win32 radio button controls. Following the TYPE_RADIO, you must supply 
        an integer count of the number of radio buttons in this group and then a list of dialog item IDs for each button. A radio 
        button can be used with TYPE_INT or TYPE_RADIOBTN_INDEX parameters. The value of the parameter defaults to the ordinal number 
        of the radio button, starting at 0. You can optionally supply a p_vals tag immediately following the p_ui section, which 
        would be followed by a list of numbers, one for each radio button. These numbers will become the (non-ordinal) parameter 
        value corresponding to which button is set. */
	TYPE_RADIO,         

    /** Identifies a single checkbox control with a parameter. Check boxes are used to provide the user with a single boolean choice. This is 
        the standard Win32 check box control. The TYPE_SINGLECHEKBOX must be followed dialog item ID. A 
        TYPE_SINGLECHEKBOX can only be used with TYPE_INT or TYPE_BOOL parameters. */
	TYPE_SINGLECHEKBOX, 

    /** \deprecated Identifies a multiple checkbox control. This control type is not currently supported. */
	TYPE_MULTICHEKBOX,  
	
    /** Identifies a color swatch color control. The color swatch control presents the user with the 
        standard 3ds Max  modeless color selector when the user clicks on the control. The color swatch control displays 
        the currently selected color. The TYPE_COLORSWATCH with the dialog item ID of the swatch custom control. This 
        can only be used with TYPE_POINT3 or TYPE_RGBA parameter. */
    TYPE_COLORSWATCH,   
	};

/** Used to associated automatically generated UI controls with parameters in a ParamBlockDesc2. 
    Wherever ControlType2 is used, a ControlType can be used as well. 
    \see ControlType, ParamBlockDesc2 
    \guide{Parameter UI Control Types}
*/ 
enum ControlType2
{
  
    // TYPE_SPINNER,	 
    // TYPE_RADIO,
    // TYPE_SINGLECHEKBOX,
    // TYPE_MULTICHEKBOX,
    // TYPE_COLORSWATCH,

    /** Identifies a 3ds Max custom edit box control. The TYPE_EDITBOX must be followed by the dialog item ID of the edit box 
        custom control. This can only be used with TYPE_STRING and TYPE_FILENAME parameters. */
	TYPE_EDITBOX = TYPE_COLORSWATCH + 1,    //!< An edit box control.
	
    /** Idenfies a two-state (pressed or not-pressed) check button control. Check boxes are used to provide the user 
        with a single boolean choice. This is the standard Win32 check box control. The TYPE_SINGLECHEKBOX must 
        be followed by a dialog item ID. A TYPE_SINGLECHEKBOX can only be used with TYPE_INT or TYPE_BOOL parameters. */
    TYPE_CHECKBUTTON,       

    /** Identifies a scene node (INode) picker button. This is a 3ds Max ICustButton control used in a CBT_CHECK mode 
        with GREEN_WASH highlight color, as per node picking button conventions. Follow the control type with the 
        dialog item ID of the button control. When the user presses this button a PickModeCallback command mode is entered 
        and the user can pick a scene node under the filtering of any validation supplied (see tags p_validator, p_classID 
        and p_sclassID). This can only be used with TYPE_INODE parameters. Use the p_prompt tag to supply a status line prompt. */
	TYPE_PICKNODEBUTTON,    

    /** A texture map selector button that supports drag and drop of texture maps. This is an ICustButton control used in 
        CBT_PUSH mode. The control type must be followed by the dialog item ID of the button control. This button opens up 
        a map selector dialog when pressed. This can only be used with TYPE_TEXMAP parameters. Use the p_prompt tag to 
        supply a status line prompt. */
	TYPE_TEXMAPBUTTON,      

    /** Identifies a material selector button that supports drag and drop of materials. This is an ICustButton control used in CBT_PUSH 
        mode. The control type must be followed by the dialog item ID of the button control. This button throws up a material 
        selector dialog when pressed. This can only be used with TYPE_MTL parameters. Use the p_prompt tag to supply a status 
        line prompt. */
	TYPE_MTLBUTTON,    

    /** Identifies a bitmap browser button. This is a 3ds Max CustButton control used in a CBT_PUSH mode. Follow the control type with 
        the dialog item ID of the CustButton custom control. This button throws up a standard 3ds Max Bitmap browser when 
        pressed and is Bitmap drag-and-drop sensitive. Can only be used with TYPE_BITMAP parameters. Use the p_prompt tag 
        to supply a status line prompt. */
	TYPE_BITMAPBUTTON,         

    /** Identifies a file open dialog button. This is an ICustButton control used in CBT_PUSH mode. The control type with the 
        dialog item ID of the button control. This button open a standard Windows Open File dialog for selecting a file name. This 
        type can only be used with TYPE_STRING and TYPE_FILENAME parameters. Use any of the p_caption, p_init_file, and p_file_types 
        tags to further control the dialog. */
	TYPE_FILEOPENBUTTON,    

    /** Identifies a file save dialog button. This is an ICustButton control used in CBT_PUSH mode. The control type with the 
        dialog item ID of the button control. This button open a standard Windows Save File dialog for selecting a file name. This 
        type can only be used with TYPE_STRING and TYPE_FILENAME parameters. Use any of the p_caption, p_init_file, and p_file_types 
        tags to further control the dialog. */
	TYPE_FILESAVEBUTTON,    

    /** Identifies a list box control for managing a Tab<int>. \guide{Int and Float List Boxes}. */
	TYPE_INTLISTBOX,        

    /** Identifies a list box control for managing a Tab<float>. \guide{Int and Float List Boxes}. */
    TYPE_FLOATLISTBOX,      

    /** Identifies a series a controls for displaying and managing a list box control containing a Tab<MSTR> parameter. */
	TYPE_STRINGLISTBOX,     

    /** Identifies a series a controls for displaying and managing a list box control containing a Tab<INode> parameter. */
	TYPE_NODELISTBOX,       
	
    /** \deprecated Not currently used */
    TYPE_MAPLISTBOX,        
    
    /** Identifies a standard 3ds Max slider control.  */
    TYPE_SLIDER,            
	
    /** \internal */
    TYPE_BUTTON,

    /** Identifies A series a controls for displaying and managing a list box control containing a Tab<Point3> parameter. */
	TYPE_POINT3LISTBOX, 

    /** \deprecated */
	TYPE_SHADERBUTTON_DEFUNCT,	

    /** \deprecated */
    TYPE_CONNECTIONSHADERBUTTON_DEFUNCT,   

    /** Parameters with a UI type of TYPE_SHADERSUBPARAMETERBLOCK indicate PB2 parameters
        for which param maps must be created, while all parameters should be edited
        in the same rollup.
        \see mrShaderParamMap2 */
	TYPE_SHADERSUBPARAMETERBLOCK,   

    /** Identifies a series a controls for displaying and managing a list box control containing a Tab<Point4> parameter. */
	TYPE_POINT4LISTBOX,     

    /** Identifies a color swatch color control for colors with an alpha channel. 
        The color swatch control presents the user with the 
        standard 3ds Max  modeless color selector when the user clicks on the control. The color swatch control displays 
        the currently selected color. The TYPE_COLORSWATCH with the dialog item ID of the swatch custom control. This 
        can only be used with TYPE_POINT4 or TYPE_FRGBA parameter. */
    TYPE_COLORSWATCH_FRGBA, 

    /** A combo box control that can be used with parameters of type TYPE_INT and TYPE_INT_TAB. 
        \guide{Combo Boxes}. */
	TYPE_INT_COMBOBOX,      
};

/** Optional parameter definition tags. These tags are used in the ParamBlockDesc2 constructor as 
    part of the "optional_tagged_param_specs". The typical format in the var args list is:
    "tag", "optional_param_spec", */
enum ParamTags
{	
    /** Specifies the default value used when a parameter block is first created (or reset to defaults). 
        Valid for the following types: TYPE_ANGLE, TYPE_PCNT_FRAC, TYPE_COLOR_CHANNEL, TYPE_FLOAT, 
        TYPE_TIMEVALUE, TYPE_INT, TYPE_BOOL, TYPE_RADIOBTN_INDEX, TYPE_POINT3, TYPE_RGBA, TYPE_STRING, 
        TYPE_FILENAME, TYPE_MATRIX3. */
    p_default = -(1<<30),		

    /** Specifies default value when constructed by scripts. For example the MAXScript command sphere.radius defaults to 
        25.0 when created by the scripter, but p_default is set to 0.0 so interactive creation starts out with a point-sized 
        sphere when the first mouse click is made.*/
	p_ms_default,				

    /** Specifies legal range, used in MS validation and spinner/slider setup.
        Ranges are used for the following control tyeps: TYPE_ANGLE, TYPE_PCNT_FRAC, 
        TYPE_COLOR_CHANNEL, TYPE_FLOAT, TYPE_TIMEVALUE, TYPE_INT, TYPE_RADIOBTN_INDEX, TYPE_POINT3, TYPE_RGBA, TYPE_FRGBA. */
	p_range,					

    /** Specifies associated UI rollout control IDs, etc.
        An optional tag that precedes a variable list of arguments depending on the type of UI control specified. 
        This sequence of arguments is similar in form to the ParamDescBlock2 class constructors.
        Following the p_ui tag, one of the following control types should be specified. Following that are 
        further tags as defined in each type.
        
        \note If a p_ui is supplied for a Tab<> parameter type and the control is *not* of on the ListBox types, 
            the table size should be fixed and supplied in the "required_param_specs" table_size field, and you 
            should supply a set of dialog item ID's for each element in the table. 
        */
	p_ui,						
	
    /** Specifies a validator object. You supply a pointer to an instance of a class derived from PBValidator. 
        This class has a PBValidator::Validate() method which can return TRUE if the PB2Value passed to it is valid and 
        FALSE otherwise. This can be used for instance by a node pick button to check if a choosen node is acceptable. 
        */
    p_validator,				

    /** Specifies an accessor object. You supply a pointer to an instance of a class derived 
        from PBAccessor. This class is used to provide a parameter Get/SetValue callback. The callback 
        can be used to monitor parameter value changes, or to implement dynamically-computed virtual parameters. The 
        class has two virtual methods, PBAccessor::Get() and PBAccessor::Set(), each given a PB2Value&, parameter ID, 
        etc. In the case of a Get() you can modify the value in the PB2Value& to implement a virtual get. 
        */
	p_accessor,					

    /** This defines radio button values in button order if button settings need to correspond to non-ordinal numbers. 
        The value of the parameter defaults to the ordinal number of the radio button, starting at 0. You can optionally 
        supply this tag immediately following the TYPE_RADIO p_ui. This tag should be followed by a list of numbers, one 
        for each radio button. These numbers will become the (non-ordinal) parameter value corresponding to which button is set. 
        */
	p_vals,						

    //	p_subanim_order,			//!< Overrides sequential subanim order. Unused.
	
    /** Used if the flag value includes P_OWNERS_REF. For these reftarg parameters this specifies the reference number in 
        the block's owner for this reference. If the parameter is a Tab<>, then the reference number supplied is the base 
        reference number of the 0'th element, with sequential reference numbers for the following elements. 
        */
    p_refno,					

    /** For Texmap items in Mtls this defines the integer SubTexmap index for this Texmap in the owner Mtl. This is used 
        by TYPE_TEXMAPBUTTON ParamMap2 control types to give to the Material Edtor for automatic button and drag And 
        drop handling. */
	p_subtexno,					

    /** Defines the sub-material integer index for this Mtl in an owner Mtl. This is used by TYPE_MTLBUTTON ParamMap2 control 
        types to give to the Material Edtor for automatic button and Drag And Drop handling. */
	p_submtlno,					

    /** This allows you to supply a dimension for this parameter. You specify a ParamDimension* as the argument. Certain 
        parameter types have an implied dimension. Defaults to defaultDim. */
	p_dim,						

    /** This specifies the class ID used as a validator for the various reftarg parameters. This is used by the scripter, 
        picknode filter, etc. For example, if you supply this in a TYPE_INODE parameter and use a TYPE_PICKNODE parammap 
        control, the picker uses this class ID in the picking filter. If you set P_CAN_CONVERT in the parameter's flag, it 
        applies an Object::CanConvertTo() test to the node's world state using the Class_ID supplied to perform the filter test. */
	p_classID,					

    /** Specifies the super class ID used as a validator for the various reftarg parameters. For example, if you supply this 
        in a TYPE_INODE parameter and use a TYPE_PICKNODE parammap control, the picker uses this super class ID in the 
        picking filter. */
	p_sclassID,			
	
    /** The associated UI controls are enabled by default. Supply TRUE or FALSE to indicate whether associated UI controls 
        are enabled or disabled when the rollout dialog is first opened. If not supplied, defaults to enabled. Can be 
        overridden by p_enable_ctrls. 
        
        For example:

        \code 
        p_enable_ctrls, 3, sel_falloff, sel_pinch, sel_bubble
        \endcode
        */
    p_enabled,					

    /** For TYPE_BOOL parametes, lists which other params should be automatically UI enabled/disabled by changes in state 
        of this parameter. This allows you to easily set up conditional enabling of other UI controls based on the state of 
        the controlling parameter. */
	p_enable_ctrls,				
	
    /** This sets the status line prompt string resource ID for various picker buttons. */
    p_prompt,					

    /** This is a caption string resource ID for TYPE_FILEOPENBUTTON or TYPE_FILESAVEBUTTON open/save file dialogs */
	p_caption,					

    /** This establishes the initial filename for open/save file dlgs. Use a direct string for the argument, not 
        a resource ID. The filename can be changed at runtime; do this by setting the init_file member of the 
        ParamDef for the parameter. For example:
        
        \code
        pbdesc->GetParamDef(file_param).init_file = new_file_name; 
        \endcode 
    */
	p_init_file,				

    /** Defines file patterns used by open/save file dialogs. The argument is a string resource ID. This 
        string sets up the file type drop-down in the open/save dialog. It is in the following form:

        \code
        "<description1>|<pattern1>|<description2>|<pattern2>|...|"
        \endcode

        In other words, it is a sequence of file type descriptions and file type patterns each separated by a '|' 
        vertical bar and terminated by a '|' vertical bar. For example:

        \code
        "Data(*.dat)|*.dat|Excel(*.csv)|*.csv|All|*.*|"
        \endcode 

        This example specifies 3 types in the file type dropdown, the first reading "Data(*.dat)" and 
        matching *.dat and the second reading "Excel(*.csv)" and matching *.csv and the third reading "All" 
        and matching any file. */
	p_file_types,				

    /** \internal Sets the ctrl_id array for the parameter. */
	p_ctrl_ids,					
	
    /** Specifies which additional rollup/map the parameter is supposed to appear in. A parameter may have 
        multiple instance of p_uix. */
    p_uix,						

    /** Default can be taken from .ini file */
	p_configurable_default,		

    /** Specifies a tooltip string by resource ID to be displayed for the user interface control associated with a 
        parameter described in a parameter block. The p_tooltip flag needs to be followed by the resource id of the 
        string representing the tooltip. */
    p_tooltip,				    

    /** Specfies asset type ID. */ 
	p_assetTypeID,              

    /** Specifies asset type name. */
	p_assetTypeName,           

    /** Signals the end of the "required_param_specs" entry in a parameter block descriptor. */
	end = p_default + 1024,

    /** \internal */
	properties,

    /** \internal */
	enums,
};
