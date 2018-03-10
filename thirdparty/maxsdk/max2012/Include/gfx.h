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
// FILE:        gfx.h
// DESCRIPTION: main graphics system include file.
// AUTHOR:      Don Brittain
// HISTORY:     
//**************************************************************************/
#pragma once

#include "maxheap.h"
#include "geomlib.h"
#include "export.h"
#include "tab.h"
#include "mtl.h"
#include "BaseInterface.h"
#include "HWMesh.h"
#include "MeshFaceFlagConstants.h"
#include "GraphicsConstants.h"
#include "GraphicsTypes.h"
#include "GraphicsViewPortColorConstants.h"
#include "maxtypes.h"
#include "TabTypes.h"
#include "LightTypeEnums.h"
#include "Strip.h"
#include "GraphicsWindow.h"

/** A vertex buffer, containing a Point3 and color. */
class VertexBuffer: public MaxHeapOperators {
public:
	Point3	position;
	DWORD		color;
};

/** A line buffer, containing a single Point3 and color. Used by GraphicsWindow. */
class LineBuffer: public MaxHeapOperators {
public:
	Point3	position;
	DWORD		color;
};

/** A triangular face, which is represented as indexes into a vertex array.  */
class GWFace: public MaxHeapOperators {
public:
	DWORD	v[3];	//< indexed references to the vertex array
	DWORD	flags;	
};

/** Describes lights used in the interactive renderer. All methods
of this class are implemented by the system.  
*/
class Light : public BaseInterfaceServer {
public:
    /** Constructor */
    DllExport Light();
    
    /** The type of the light. Has one of the following values:
        \li OMNI_LGT - Omni-directional.
        \li SPOT_LGT - Spot light.
        \li DIRECT_LGT - Directional light.
        \li AMBIENT_LGT - Ambient light (global illumination) */
    LightType		type; 

    /** The color of the light. */
    Point3			color;

    /** Attenuation type. GW_ATTEN_NONE by default. */
    int				attenType;

    /** Starting attenuation value. */
    float			attenStart;

    /** Ending attenuation value. */
    float			attenEnd;

    /** Light multiplier factor. */
    float			intensity;

    /** Angle of cone for spot and cone lights in degrees. */
    float			hotSpotAngle;

    /** Fall-off angle. */
	float			fallOffAngle;

    /** Indicates the shape of the light. It is either:  
     \li GW_SHAPE_RECT - Rectangular spotlights.
     \li GW_SHAPE_CIRCULAR - Circular spotlights. */
	int				shape;

    /** The aspect ratio of the light. */
	float			aspect;
	
    /** If this value is non-zero it indicates the light supports overshoot. */
    int				overshoot;

    /** When set to FALSE by a user the
    light does not illuminate the diffuse component of an object's
    material. This value is TRUE by default. */
	BOOL 			affectDiffuse;
	
    /** When set to FALSE by a user the
    light does not illuminate the specular component of an object's
    material. This value is TRUE by default. */
    BOOL 			affectSpecular;
};

/** Represents the projection matrix and common properties of a perspective or orthographic camera. */
class Camera : public BaseInterfaceServer {
public:
	DllExport Camera();


    /** Initializes the camera with a perspective view given a field-of-view value and aspect ratio. */
	void			setPersp(float f, float asp)
						{ type = PERSP_CAM; persp.fov = f; 
						  persp.aspect = asp; makeMatrix(); }

    /** Initializes the camera with a orthographic view. */
	void			setOrtho(float l, float t, float r, float b)
						{ type = ORTHO_CAM; ortho.left = l; ortho.top = t; 
						  ortho.right = r; ortho.bottom = b; makeMatrix(); }
	
    /** Sets the camera clipping hither and yon values. */
    void			setClip(float h, float y) 
						{ hither = h; yon = y; makeMatrix(); }
	
    /** Returns the type of camera whether it is a perspective camera (PERSP_CAM) or orthographic camera (ORTHO_CAM) */
    CameraType		getType(void)	{ return type; }

    /** Returns the clipping hither value */ 
	float			getHither(void) { return hither; }

    /** Returns the clipping yon value */ 
	float			getYon(void)	{ return yon; }

    /** Resets the projection matrix. */
	DllExport void	reset();

    /** Returns the projection matrix. */
	DllExport void	getProj(float mat[4][4]);
private:
	DllExport void	makeMatrix();
	float			proj[4][4];
	CameraType		type;
	union {
	    struct : public MaxHeapOperators {
            float	fov;
            float	aspect;
		} persp;
		struct : public MaxHeapOperators {
		    float	left;
		    float	right;
		    float	bottom;
		    float	top;
		} ortho;
	};
	float			hither;
	float			yon;
};

/// \name Window / Crossing Mode Functions 
//@{ 
/** Set to a non-zero value if crossing mode is used for selections, or zero if windowing mode is used instead. */
DllExport void	setAutoCross(int onOff);
/** Returns a non-zero value if crossing mode is used for selections, or zero if windowing mode is used instead. */
DllExport int	getAutoCross();
/** Sets the type of crossing mechanism used in crossing mode of selections. Legal values are AC_DIR_RL_CROSS or AC_DIR_LR_CROSS. */
DllExport void	setAutoCrossDir(int dir);
/** Sets the type of crossing mechanism used in crossing mode of selections. This is either AC_DIR_RL_CROSS or AC_DIR_LR_CROSS. */
DllExport int	getAutoCrossDir();
//@}

/** Represents a circular region when doing hit testing. \see HitRegion */
struct CIRCLE: public MaxHeapOperators
{
    LONG  x; //< X coordinates of circle.
    LONG  y; //< Y coordinates of circle
	LONG  r; //< Radius of circle.
};

/** Describes the properties of a region used for built-in hit testing
of items in the interactive renderer.
*/
class HitRegion: public MaxHeapOperators {
	DWORD size;
public:
    /** The region type. One of the following values
    \li POINT_RGN A single point.
    \li RECT_RGN A rectangular region.
    \li CIRCLE_RGN  A circular region.
    \li FENCE_RGN An arbitrary multi-point polygon region.
    */
	int				type;

    /** Region direction. RGN_DIR_UNDEF by default. */
	int				dir;		// region direction

    /**
      If nonzero, elements that are contained within or that cross the region
      boundary are hit. If zero, only those elements entirely within the
      boundary are hit. This is not used for point hit testing. */
	int				crossing;	

    /** 
        Specifies the distance in pixels outside the pick point within which elements
        may be and still be hit. This is not used for rect or circle testing, is sometimes used with fence hit
        testing, where it doubles the size of the region, especially when selecting subobject edges or vertices
        and is always used for point hit testing. */
	int				epsilon;	

    /** Region data. */
	union {
		POINT		pt;
		RECT		rect;
		CIRCLE		circle;
		POINT *		pts;
	};

	HitRegion()		{ dir = RGN_DIR_UNDEF; size = sizeof(HitRegion);}
};

#ifndef ABS
/** Returns the absolute value of an integer. */
inline int ABS(const int x) { return (x > 0) ? x : -x; }
#endif

/** \internal 
A graphics window setup structure. An instance of this class is
passed to the function <code>createGW()</code> to create a new graphics window.
\see GraphicsWindow
*/
class GWinSetup: public MaxHeapOperators {
public:
    DllExport GWinSetup();
    MCHAR		caption[GW_MAX_CAPTION_LEN];
	MCHAR		renderer[GW_MAX_FILE_LEN];
	MCHAR		device[GW_MAX_FILE_LEN];
	DWORD		winStyle;
	POINT		size;
	POINT		place;
	INT_PTR		id;
	int			type;
	bool		quietMode;
};

/** Given three points in a windows coordinate system (where the upper-left of the window is origin) returns 
    TRUE if the face is facing backwards, FALSE otherwise */
inline int wIsFacingBack(const IPoint3 &v0, const IPoint3 &v1, const IPoint3 &v2, int flip=0 )
{
	int s = ( (v0[0]-v1[0])*(v2[1]-v1[1]) - (v2[0]-v1[0])*(v0[1]-v1[1]) ) < 0;
	return flip ? !s : s;
}

/** Given three points in a coordinate system where the lower-left of the window is origin returns 
    TRUE if the face is facing backwards, FALSE otherwise */
inline int hIsFacingBack(const IPoint3 &v0, const IPoint3 &v1, const IPoint3 &v2, int flip=0 )
{
	int s = ( (v0[0]-v1[0])*(v2[1]-v1[1]) - (v2[0]-v1[0])*(v0[1]-v1[1]) );
	return flip ? s < 0 : s > 0;
}

/** Returns the facing of a given triangle relative to the screen.
 Returns whether a given triangle is front-facing, 
 side-facing, or back-facing relative to the screen. The triangle is passed as three points in screen space. 
 This function is used for "w" format device coordinates.
    \param v0 The 1st triangle vertex
    \param v1 The 2nd triangle vertex
    \param v2 The 3rd triangle vertex 
    \param flip If true, flip the triangle (so backfacing would return frontfacing) */
inline FacingType wFacingType(const IPoint3 &v0, const IPoint3 &v1, const IPoint3 &v2, int flip=0 )
{
	int s = ( (v0[0]-v1[0])*(v2[1]-v1[1]) - (v2[0]-v1[0])*(v0[1]-v1[1]) );
	return (s == 0) ? kSideFacing : ((flip ? s > 0 : s < 0) ? kBackFacing : kFrontFacing);
}

/** Returns the facing of a given triangle relative to the screen.
 The methods wFacingType() and hFacingType() will return whether a given triangle is front-facing, 
side-facing, or back-facing relative to the screen. The triangle is passed as three points in screen space. 
This function is used for "h" format device coordinates. 
    \param v0 The 1st triangle vertex
    \param v1 The 2nd triangle vertex
    \param v2 The 3rd triangle vertex
    \param flip If true, flip the triangle (so backfacing would return frontfacing) */
inline FacingType hFacingType(const IPoint3 &v0, const IPoint3 &v1, const IPoint3 &v2, int flip=0 )
{
	int s = ( (v0[0]-v1[0])*(v2[1]-v1[1]) - (v2[0]-v1[0])*(v0[1]-v1[1]) );
	return (s == 0) ? kSideFacing : ((flip ? s < 0 : s > 0) ? kBackFacing : kFrontFacing);
}

/** \internal */
DllExport HINSTANCE GetGraphicsLibHandle(MCHAR *driverLibName);

/** \internal */
DllExport BOOL GraphicsSystemIsAvailable(HINSTANCE drv);

/** \internal */
DllExport BOOL GraphicsSystemCanConfigure(HINSTANCE drv);

/** \internal */
DllExport BOOL GraphicsSystemConfigure(HWND hWnd, HINSTANCE drv);

/** \internal */
DllExport void FreeGraphicsLibHandle(HINSTANCE drv);

/** \internal 
    This function is used internally to create a new graphics window. */
DllExport GraphicsWindow* createGW(HWND hWnd, GWinSetup &gws);

/** Returns a bounding rectangle that encloses the entire hit region.
For example if the hit regions was a fence region, this method would return the
smallest rectangle that included the entire set of fence region points.
    \param hr The hit region to check
    \param rect Points to the returned bounding rectangle. */
DllExport void getRegionRect(HitRegion *hr, RECT *rect);

/**  Returns TRUE if the specified point is inside the region \c hr or FALSE otherwise. */
DllExport BOOL pointInRegion(int x, int y, HitRegion *hr);

/** Returns the signed distance from \c x, \c y to the line defined by <code>p1-\>p2</code>. */
DllExport int distToLine(int x, int y, int *p1, int *p2);

/** Returns z depth at closest point in the line defined by p1->p2 to x,y
    v = the vector from p1 to p2
    w = the vector from p1 to q=(x,y)
    t = (w dot v) / (v dot v)
    p1[2] + (p2[2] - p1[2]) * t 
*/
DllExport int zDepthToLine(int x, int y, int *p1, int *p2);

/** Returns nonzero if the line defined by <code>p1-\>p2</code> crosses into the RECT and 0 otherwise. */
DllExport int lineCrossesRect(RECT *rc, int *p1, int *p2);

/** Returns nonzero if the line-segment defined by <code>p1-\>p2</code> crosses into the RECT and 0 otherwise. */
DllExport int segCrossesRect(RECT *rc, int *p1, int *p2);

/** Returns nonzero if the line-segment defined by <code>p1-\>p2</code> crosses the
circle center at (<code>cx, cy</code>) with a radius of <code>r</code> 0 otherwise. */
DllExport int segCrossesCircle(int cx, int cy, int r, int *p1, int *p2);

/** Returns TRUE if the point passed is inside the specified
triangle.
\param p0 The first point of the triangle
\param p1 The second point of the triangle
\param p2 The third point of the triangle.
\param q The point to check.
\return Returns TRUE if the point passed is inside the specified triangle;
otherwise FALSE. */
DllExport BOOL insideTriangle(IPoint3 &p0, IPoint3 &p1, IPoint3 &p2, IPoint3 &q);

/** Returns the z value of where the projected screen point <code>q</code> would intersect the triangle 
defined by (<code>p0, p1, p2</code>).
\param p0 The first point of the triangle
\param p1 The second point of the triangle
\param p2 The third point of the triangle.
\param q The screen point to check. 
\return Returns TRUE if the point passed is inside the specified triangle;
otherwise FALSE. */
DllExport int getZfromTriangle(IPoint3 &p0, IPoint3 &p1, IPoint3 &p2, IPoint3 &q);

/** Given an integer returns another integer which is the closest power of 2 */
DllExport int getClosestPowerOf2(int num);

/** \name User-Defined Colors
 Access to customizable colors used to draw viewport, scene and user interface elements (not elements 
    of different color schemes available in 3ds Max). 
    Clients of these methods should consider registering for NOTIFY_COLOR_CHANGE notification in order
    to ensure that they use the current custom colors.
*/
//@{
/** Retrieves the current value of the specified custom color
* \param which The identifier of the custom color to retrieve. See \ref viewportDrawingColors
* \return The current value of the color, as an RGB value
*/
DllExport Point3 GetUIColor(int which);
/** Sets the current value of the specified custom color.
* \param which The identifier of the custom color to retrieve. See \ref viewportDrawingColors
* \param clr Pointer to the new color value object
*/
DllExport void SetUIColor(int which, Point3 *clr);
/** Retrieves the default value of the specified custom color.
* \param which The identifier of the custom color to retrieve. See \ref viewportDrawingColors
* \return The default value of the color, as an RGB value
*/
DllExport Point3 GetDefaultUIColor(int which);
/** Retrieves the selection color */ 
#define GetSelColor()		GetUIColor(COLOR_SELECTION)
/** Retrieves the sub-object selection color */ 
#define GetSubSelColor()	GetUIColor(COLOR_SUBSELECTION)
/** Retrieves the freeze color */ 
#define GetFreezeColor()	GetUIColor(COLOR_FREEZE)
//@}
