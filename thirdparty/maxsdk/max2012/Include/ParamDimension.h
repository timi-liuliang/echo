//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
#pragma once
#include "MaxHeap.h"
#include "CoreExport.h"
#include "strbasic.h"

// This must be updated if a new entry is added to DimType!
#define NUM_BUILTIN_DIMS	10

/*! \defgroup dimType Dimension Types
	\todo Document this enum
 */
//@{
enum DimType {
	DIM_WORLD,
	DIM_ANGLE,
	DIM_COLOR,	 	//0-1
	DIM_COLOR255,	//0-255
	DIM_PERCENT,  	//0-100
	DIM_NORMALIZED,	//0-1
	DIM_SEGMENTS,
	DIM_TIME,
	DIM_CUSTOM,
	DIM_NONE
};
//@}

// These two classes describe the dimension of a parameter (sub-anim).
// The dimension type and possibly the dimension scale (if the type is
// custom) are used to determine a scale factor for the parameter.
// When a controller is drawing a function curve, it only needs to
// use the Convert() function - the scale factor is rolled into the single
// 'vzoom' parameter passed to PaintFCurves.
// So, for a controller to plot a value 'val' at time t it would do the
// following:
// int x = TimeToScreen(t,tzoom,tscroll);
// int y = ValueToScreen(dim->Convert(val),rect.h()-1,vzoom,vscroll);
//
/*! \sa  Class ParamDimension.
\par Description:
This class (along with ParamDimension) describes the dimension of a parameter.
This dimension can be considered a unit of measure. It describes the
parameter's type and order of magnitude.\n\n
The dimension type and possibly the dimension scale (if the type is custom) are
used to determine a scale factor for the parameter. When a controller is
drawing a function curve, it only needs to use the Convert() function - the
scale factor is rolled into the single 'vzoom' parameter passed to
<b>Animatable::PaintFCurves()</b>. So, for a controller to plot a value 'val'
at time t it would do the following:\n\n
<b>int x = TimeToScreen(t,tzoom,tscroll);</b>\n\n
<b>int y = ValueToScreen(dim-\>Convert(val),rect.h()-1,vzoom,vscroll);</b> 
*/
class ParamDimensionBase: public MaxHeapOperators {
	public:
		/*! \remarks Destructor. */
		virtual ~ParamDimensionBase() {}
		/*! \remarks Returns the dimension type of the parameter. See \ref dimType */
		virtual DimType DimensionType()=0;
		/*! \remarks When a controller needs to display the parameter values (for example in
		the function curve editor) it converts the value using this method.
		\par Parameters:
		<b>float value</b>\n\n
		The value to convert.
		\return  The converted value. */
		virtual float Convert(float value)=0;
		/*! \remarks This method is used to un-converted a converted value.
		\par Parameters:
		<b>float value</b>\n\n
		The value to un-convert.
		\return  The un-converted value. */
		virtual float UnConvert(float value)=0;
}; 

/*! \sa  Class ParamDimensionBase,  \ref dimType.
\par Description:
Any parameter that can be controlled by a controller has a dimension. This
dimension can be considered a unit of measure. It describes its type and its
order of magnitude. When a controller needs to display the parameter values
(for example in the function curve editor) it converts the value using its
parameter dimension Convert() function. It can also convert back using the
Unconvert() function.\n\n
Some parameters are stored over one range of values and displayed in another.
For example parameter that use <b>stdAngleDim</b> store their parameters in
radians but display them in degrees. By using <b>stdAngleDim</b> the value is
converted to the proper format for display. Some parameter dimensions do not
perform any conversion, for example <b>stdWorldDim</b>.\n\n
There are several default parameter dims implemented. Listed with each one is
the type, convert functions and range of values usually stored (these are not
enforced in any way).\n\n
If the type of parameter for your plug-in does not fit within any of the
dimensions listed here you may simply use <b>defaultDim</b>. This performs no
conversions and has no range.\n\n
<b>ParamDimension *defaultDim;</b>\n\n
Convert() is original value.\n\n
UnConvert() is original value.\n\n
Range: None.\n\n
<b>ParamDimension *stdWorldDim;</b>\n\n
The DimType is <b>DIM_WORLD</b>\n\n
Convert() return original value.\n\n
UnConvert() returns original value.\n\n
<b>ParamDimension *stdAngleDim;</b>\n\n
The DimType is <b>DIM_ANGLE.</b>\n\n
Convert() is RadToDeg()\n\n
UnConvert() is DegToRad()\n\n
<b>ParamDimension *stdColorDim;</b>\n\n
The DimType is <b>DIM_COLOR</b>\n\n
Convert() return original value.\n\n
UnConvert() returns original value.\n\n
Range: 0-1\n\n
<b>ParamDimension *stdColor255Dim;</b>\n\n
The DimType is <b>DIM_COLOR255</b>\n\n
Convert() is value * 255.0f\n\n
UnConvert() is value / 255.0f\n\n
Range: 0-255\n\n
<b>ParamDimension *stdPercentDim;</b>\n\n
The DimType is <b>DIM_PERCENT</b>\n\n
Convert() is value * 100.0f\n\n
UnConvert() is value / 100.0f\n\n
Range: 0-100\n\n
<b>ParamDimension *stdNormalizedDim;</b>\n\n
The DimType is <b>DIM_NORMALIZED</b>\n\n
Convert() is original value.\n\n
UnConvert() is original value.\n\n
Range: 0-1\n\n
<b>ParamDimension *stdSegmentsDim;</b>\n\n
The DimType is <b>DIM_SEGMENTS</b>\n\n
Convert() is original value.\n\n
UnConvert() is original value.\n\n
<b>ParamDimension *stdTimeDim;</b>\n\n
The DimType is <b>DIM_TIME</b>\n\n
Convert() is value/GetTicksPerFrame()\n\n
UnConvert()is value*GetTicksPerFrame() (see the section
<a href="ms-its:3dsmaxsdk.chm::/anim_time_functions.html">Time</a> for
details). \n  If the DimType is custom then the methods below must be
implemented. Note: This class is derived from ParamDimensionBase which provides
methods <b>DimensionType()</b>, <b>Convert()</b> and <b>Unconvert()</b>. */
class ParamDimension : public ParamDimensionBase {
	public:
		// If the DimType is custom than these must be implemented.
		/*! \remarks Returns the dimension scale. */
		virtual float GetDimScale() {return 1.0f;}
		/*! \remarks Sets the dimension scale. */
		virtual void SetDimScale() {}
		/*! \remarks Returns the name of the dimension. */
		virtual MCHAR* DimensionName() {return _M("");}		
};

// These point to default implementations for the standard DIM types.
CoreExport extern ParamDimension *defaultDim;
CoreExport extern ParamDimension *stdWorldDim;
CoreExport extern ParamDimension *stdAngleDim;
CoreExport extern ParamDimension *stdColorDim;
CoreExport extern ParamDimension *stdColor255Dim;
CoreExport extern ParamDimension *stdPercentDim;
CoreExport extern ParamDimension *stdNormalizedDim;
CoreExport extern ParamDimension *stdSegmentsDim;
CoreExport extern ParamDimension *stdTimeDim;