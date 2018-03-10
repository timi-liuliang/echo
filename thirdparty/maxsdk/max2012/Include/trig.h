//
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.  
//
//

#pragma once

/** The constant Pi defined as float. Pi is the ratio of a circle's circumference to its diameter. */
#define PI  ((float)3.1415926535)
/** Two times the constant Pi defined as a float. */
#define TWOPI ((float)6.283185307)
/** Half of the constant Pi defined as a float. */
#define HALFPI ((float)1.570796326794895)
/** The coefficient to convert the value of an angle in degrees into radians. */
#define DEG_TO_RAD (PI/(float)180.0)
/** The coefficient to convert the value of an angle in radians into degrees. */
#define RAD_TO_DEG ((float)180.0/PI)
/** A function macro to convert degrees to radians with float precision. */ 
#define DegToRad(deg) (((float)deg)*DEG_TO_RAD)
/** A function macro to convert radians to degrees with float precision. */
#define RadToDeg(rad) (((float)rad)*RAD_TO_DEG)


