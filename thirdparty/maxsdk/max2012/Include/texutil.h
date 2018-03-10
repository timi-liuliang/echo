/**********************************************************************
 *<
	FILE: texutil.h

	DESCRIPTION:

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "coreexp.h"
#include "point2.h"
#include "point3.h"
#include "acolor.h"

//
//	Misc. noise functions from Texturing and Modeling A Procedural Approach
//  Perlin, Musgrave...
//

CoreExport float bias(float a, float b);
CoreExport float gain(float a, float b);
CoreExport float clamp(float x, float a, float b);

CoreExport float boxstep(float a, float b, float x); // linear from (a,0) to (b,1)
CoreExport float smoothstep(float a, float b, float x);  // Hermite cubic from (a,0) to (b,1)
CoreExport float mod(float x, float m); // returns x Mod m, handles negatives correctly
CoreExport int mod(int x, int m); // returns x Mod m, handles negatives correctly

// This function makes a sort of straight segment S curve.
// sramp() is a for x < a-d and b for x > b+d.
// for a+d < x < b-d sramp(x) = x
// for a-d < x < a+d sramp makes a smooth transition (parabolic) from
//     sramp' = 0 to sramp' = 1
// for b-d < x < b+d sramp makes a smooth transition (parabolic) from
//     sramp' = 1 to sramp' = 0
CoreExport float sramp(float x,float a, float b, float d);

// returns 0 if x<a, 1 if x>b otherwise x.
CoreExport float threshold(float x,float a, float b);

CoreExport void  setdebug(int i);
CoreExport float noise1(float arg);
CoreExport float noise2(Point2 p);
CoreExport float noise3(Point3 p);
CoreExport float noise4(Point3 p,float time);

// This is 3DStudio's Noise function: its only slightly different from noise3:
//  scaled up by factor of 1.65 and clamped to -1,+1.
CoreExport float noise3DS(Point3 p);

CoreExport float turbulence(Point3& p, float freq);

CoreExport int Perm(int v);

#define MAX_OCTAVES	50
CoreExport float fBm1(float  point, float H, float lacunarity, float octaves);
CoreExport float fBm1(Point2 point, float H, float lacunarity, float octaves);
CoreExport float fBm1(Point3 point, float H, float lacunarity, float octaves);

CoreExport float spline(float x, int nknots, float *knot);

CoreExport Color color_spline(float x, int nknots, Color *knot);


// faster version of floor
inline int FLOOR( float x) { return ((int)(x) - ((int)(x)>(x)? 1:0)); }

inline float frac(float x) { return x - (float)FLOOR(x); }
inline float fmax(float x, float y) { return x>y?x:y;	}
inline float fmin(float x, float y) { return x<y?x:y;	}

// Macro to map it into interval [0,1]
#define NOISE(p) ((1.0f+noise3DS(p))*.5f)

// alpha-composite ctop on top of cbot, assuming pre-multiplied alpha
inline AColor AComp(AColor cbot, AColor ctop) {
	float ia = 1.0f - ctop.a;
	return (ctop + ia*cbot);
	}


//-----------------------------------------------------------
// Based on: A Cellular Basis Function
//                   Steven Worley
//
// SIGGRAPH 1996 Conference Procedings
//
#define MAX_CELL_LEVELS	20

CoreExport void CellFunction(Point3 v,int n,float *dist,int *celIDs=NULL,Point3 *grads=NULL,float gradSmooth=0.0f);
CoreExport void FractalCellFunction(Point3 v,float iterations, float lacunarity,int n,float *dist,int *celIDs=NULL,Point3 *grads=NULL,float gradSmooth=0.0f);
CoreExport float RandFromCellID(int id);

