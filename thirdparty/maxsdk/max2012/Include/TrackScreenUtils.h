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


/*! \defgroup timeToScreen Screen-Time-Value Macros
These are macros used when working with Track View to scale into and out of screen space.
 */
//@{  

// Macros for converting track screen coords to time and back.
/*! Given a TimeValue t, if you have the time scale and time scroll factors, 
this macro returns the screen space X for that time.
 */
#define TimeToScreen(t,scale,scroll) (int(floor((t)*(scale)+0.5)) - (scroll))

/*! Given a screen coordinate s, if you have the scale and scroll factors, 
this macro will return a TimeValue for that screen position.
 */
#define ScreenToTime(s,scale,scroll) ((int)floor((s)/(scale) + (scroll)/(scale)+0.5))

/*! Given a Value v, if you have the value scale and value scroll factors, 
this macro returns the screen space Y for that value.
 */
#define ValueToScreen(v,h,scale,scroll) (h-int(floor((v)*(scale)+0.5)) - (scroll))

/*! Given a screen coordinate s, if you have the scale and scroll factors, 
this macro will return a value for that screen position.
*/
#define ScreenToValue(s,h,scale,scroll) ((float(h)-(float(s)+float(scroll)))/(scale))

// Scales a value about an origin
#define ScaleAboutOrigin(val,origin,scale) ((((val)-(origin))*(scale))+(origin))
//@}