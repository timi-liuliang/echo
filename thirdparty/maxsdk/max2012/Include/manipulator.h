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
// FILE:        Manipulator.h
// DESCRIPTION: Defines Manipulator classes
// AUTHOR:      Scott Morrison
// HISTORY:     created 18 October 1999
//**************************************************************************/

#pragma once

#ifdef MANIPSYS_IMP
#define ManipExport __declspec(dllexport)
#else
#define ManipExport __declspec(dllimport)
#endif

#include "iparamb2.h"
#include "iFnPub.h"
#include "polyshp.h"
#include "object.h"
#include "gfx.h"
#include "icolorman.h"

enum DisplayState { kNoRedrawNeeded, kFullRedrawNeeded, kPostRedrawNeeded };

#define MANIP_PLANE_INTERFACE  Interface_ID(0x44460ea4, 0xbf73be6)

/*! \sa  Class FPMixinInterface,  Class Point3, Class Ray , Class IManipulatorMgr\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
The manipulator system exports a helper plane class. This is very useful when
writing <b>OnMouseMove()</b> methods. It is normally used to intersect a view
ray with some plane that is used by the manipulator.
\par Data Members:
<b>static Plane msXYPlane;</b>\n\n
This constant defines the XY plane.\n\n
<b>static Plane msXZPlane;</b>\n\n
This constant defines the XZ plane.\n\n
<b>static Plane msYZPlane;</b>\n\n
This constant defines the YZ plane.  */
class Plane : public FPMixinInterface {
public:
    /*! \remarks Constructor.\n\n
    Create a plane that passes through the given point with the given normal.
    \par Parameters:
    <b>Point3\& normal</b>\n\n
    The normal of the plane.\n\n
    <b>Point3\& point</b>\n\n
    The point the plane should pass through. */
    ManipExport Plane(Point3& normal, Point3& point);
    /*! \remarks Constructor.\n\n
    Create a plane that passes through all three given points.
    \par Parameters:
    <b>Points3\& p1, Points3\& p2, Points3\& p3</b>\n\n
    The three points the plane should pass through.\n\n
      */
    ManipExport Plane(Point3& p1, Point3& p2, Point3& p3);
    /*! \remarks Constructor. */
    ManipExport Plane(): mNormal(0,0,1), mPoint(0,0,0), mD(0.0f) {}

    /*! \remarks This method will intersect the plane with a ray.
    \par Parameters:
    <b>Ray\& ray</b>\n\n
    The ray you wish to test for intersection.\n\n
    <b>Points3\& intersectionPoint</b>\n\n
    The resulting intersection point, if the ray intersected the plane.
    \return  TRUE if the ray intersects, otherwise FALSE. */
    ManipExport bool Intersect(Ray& ray, Point3& intersectionPoint);
    /*! \remarks This method returns the normal of the plane. */
    ManipExport Point3& GetNormal() { return mNormal; }
    /*! \remarks This method returns the point that the plane passes through.
    */
    ManipExport Point3& GetPoint()  { return mPoint; }
    /*! \remarks This method returns the plane equation constant. */
    ManipExport float   GetPlaneConstant() { return mD; }
    /*! \remarks This method can be used to test which plane (either
    <b>this</b> or <b>plane</b>) is most orthogonal with the given view ray,
    returning that plane. This can be useful in determining which plane to
    project a view ray onto, if more than one is applicable.
    \par Parameters:
    <b>Ray\& viewDir</b>\n\n
    The given view ray.\n\n
    <b>Plane\& plane</b>\n\n
    The plane to test. */
    ManipExport Plane&  MostOrthogonal(Ray& viewDir, Plane& plane);

    ManipExport static Plane msXYPlane;
    ManipExport static Plane msXZPlane;
    ManipExport static Plane msYZPlane;
    
	// Function IDs
	enum { intersect, mostOrthogonal, getNormal, getPoint, getPlaneConstant, };
	// Function Map
	BEGIN_FUNCTION_MAP
		FN_2(intersect,		 TYPE_BOOL,		 Intersect,		   TYPE_RAY_BV, TYPE_POINT3_BR);
		FN_2(mostOrthogonal, TYPE_INTERFACE, FPMostOrthogonal, TYPE_RAY_BV, TYPE_INTERFACE);
		RO_PROP_FN(getNormal,		 GetNormal,			TYPE_POINT3_BV);
		RO_PROP_FN(getPoint,		 GetPoint,			TYPE_POINT3_BV);
		RO_PROP_FN(getPlaneConstant, GetPlaneConstant,	TYPE_FLOAT);
	END_FUNCTION_MAP

	// FP interface type-converter wrappers
	ManipExport Plane*	FPMostOrthogonal(Ray& viewRay, FPInterface* plane);

	ManipExport FPInterfaceDesc* GetDesc();

	// russom - 09/01/04 - 582532
	ManipExport LifetimeType LifetimeControl() { return wantsRelease; }
	ManipExport BaseInterface* AcquireInterface() { return this; }
	ManipExport void ReleaseInterface();

private:
    Point3  mNormal;  // Plane normal vector
    Point3  mPoint;   // Point that the plane passes through
    float   mD;       // Plane equation constant
};

#define MANIP_GIZMO_INTERFACE  Interface_ID(0x124e3169, 0xf067ad4)

#pragma warning(push)
#pragma warning(disable:4239)

/*! \sa  Class SimpleManipulator,  Class ISimpleManipulator,  Class FPMixinInterface,  Class PolyLine, Class PolyShape\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the main gizmo shape.\n\n
The Function Publishing interface to SimpleManipulators is defined as:\n\n
<b>#define MANIP_GIZMO_INTERFACE Interface_ID(0x124e3169, 0xf067ad4)</b>\n\n

\par Data Members:
private:\n\n
<b>PolyShape mPolyShape;</b>\n\n
The gizmo polyshape.\n\n
<b>PolyLine mLine;</b>\n\n
A poly line.  */
class GizmoShape: public FPMixinInterface {
public:
    /*! \remarks Constructor.
    \par Default Implementation:
    <b>{ mLine.Init(); }</b> */
    ManipExport GizmoShape() { mLine.Init(); }

    /*! \remarks This method instructs the gizmo shape to start (append) a new
    line segment. */
    ManipExport void StartNewLine() {
        if (mLine.numPts > 0)
            mPolyShape.Append(mLine);
        mLine.Init();
    }
    /*! \remarks This method instructs the gizmo shape to append a new point
    to the line segment.
    \par Parameters:
    <b>Point3\& p</b>\n\n
    The vertex point to add. */
    ManipExport void AppendPoint(Point3& p) {
        mLine.Append(PolyPt(p));
    }

    /*! \remarks This method returns a pointer to the gizmo's poly shape. */
    ManipExport PolyShape* GetPolyShape() {
        if (mLine.numPts > 0)
            mPolyShape.Append(mLine);
        mLine.Init();
        return &mPolyShape;
    }

    /*! \remarks This method allows you to transform the gizmo shape.
    \par Parameters:
    <b>Matrix3\& tm</b>\n\n
    The transformation matrix. */
    ManipExport void Transform(Matrix3& tm);

	// Function IDs
	enum { startNewLine, appendPoint, transform};
	// Function Map
	BEGIN_FUNCTION_MAP
		VFN_0(startNewLine, StartNewLine);
		VFN_1(appendPoint,  AppendPoint, TYPE_POINT3_BV);
		VFN_1(transform,    Transform,   TYPE_MATRIX3_BV);
	END_FUNCTION_MAP

	ManipExport FPInterfaceDesc* GetDesc();

	// russom - 09/01/04 - 582532
	ManipExport LifetimeType LifetimeControl() { return wantsRelease; }
	ManipExport BaseInterface* AcquireInterface() { return this; }
	ManipExport void ReleaseInterface();

private:
    PolyShape mPolyShape;
    PolyLine  mLine;
};


//  Manipulator system static FnPub interace
#define MANIP_MGR_INTERFACE  Interface_ID(0x2c450aa2, 0x7b9d0365)
/*! \sa  Class FPStaticInterface\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This is the abstract interface class for Manipulator Manager Objects\n\n
The Interface ID of this class is <b>MANIP_MGR_INTERFACE</b>.  */
class IManipulatorMgr : public FPStaticInterface
{
public:
	// stock gizmos
	/*! \remarks This method allows you to create a spherical mesh gizmo.
	\par Parameters:
	<b>Point3\& pos</b>\n\n
	The position of the sphere.\n\n
	<b>float radius</b>\n\n
	The radius of the sphere\n\n
	<b>int segments</b>\n\n
	The number of segments in the sphere.
	\return  A pointer to the resulting mesh. */
	virtual Mesh* MakeSphere(Point3& pos, float radius, int segments)=0;
	/*! \remarks This method allows you to create a torus mesh gizmo.
	\par Parameters:
	<b>Point3\& pos</b>\n\n
	The position of the torus.\n\n
	<b>float radius</b>\n\n
	The radius of the torus.\n\n
	<b>float radius2</b>\n\n
	The second radius of the torus.\n\n
	<b>int segs</b>\n\n
	The number of segments in the torus.\n\n
	<b>int sides</b>\n\n
	The number of sides of the torus.
	\return  A pointer to the resulting mesh. */
	virtual Mesh* MakeTorus(Point3& pos, float radius, float radius2, int segs, int sides)=0;
	/*! \remarks This method allows you to create a box mesh gizmo.
	\par Parameters:
	<b>Point3\& pos</b>\n\n
	The position of the box.\n\n
	<b>float l</b>\n\n
	The length of the box.\n\n
	<b>float w</b>\n\n
	The width of the box.\n\n
	<b>float h</b>\n\n
	The height of the box.\n\n
	<b>int lsegs</b>\n\n
	The length segments of the box.\n\n
	<b>int wsegs</b>\n\n
	The width segments of the box.\n\n
	<b>int hsegs</b>\n\n
	The height segments of the box.
	\return  A pointer to the resulting mesh. */
	virtual Mesh* MakeBox(Point3& pos, float l, float w, float h, int lsegs, int wsegs, int hsegs)=0;

	// plane construction
	/*! \remarks This method creates a default plane gizmo. */
	virtual Plane* MakePlane()=0;
	/*! \remarks This method creates a plane gizmo.
	\par Parameters:
	<b>Point3\& p1</b>\n\n
	The first point of the plane.\n\n
	<b>Point3\& p2</b>\n\n
	The second point of the plane.\n\n
	<b>Point3\& p3</b>\n\n
	The third point of the plane. */
	virtual Plane* MakePlane(Point3& p1, Point3& p2, Point3& p3)=0;
	/*! \remarks This method creates a plane gizmo.
	\par Parameters:
	<b>Point3\& normal</b>\n\n
	The normal of the plane\n\n
	<b>Point3\& point</b>\n\n
	The center point in space of the plane. */
	virtual Plane* MakePlane(Point3& normal, Point3& point)=0;

	// constant planes
	/*! \remarks This method returns the XY plane. */
	virtual Plane* GetmsXYPlane()=0;
	/*! \remarks This method returns the XZ plane. */
	virtual Plane* GetmsXZPlane()=0;
	/*! \remarks This method returns the YZ plane. */
	virtual Plane* GetmsYZPlane()=0;

    // PolyShape gizmos
    /*! \remarks This method will create a default gizmo shape. */
    virtual GizmoShape* MakeGizmoShape()=0;
    /*! \remarks This method will make a circular gizmo shape.
    \par Parameters:
    <b>Point3\& center</b>\n\n
    The center of the circle.\n\n
    <b>float radius</b>\n\n
    The radius of the circle.\n\n
    <b>int segments</b>\n\n
    The number of segments of the circle.
    \return  A pointer to the resulting gizmo shape. */
    virtual GizmoShape* MakeCircle(Point3& center, float radius, int segments)=0;

	// Function IDs
	enum { makeSphere, makeTorus, makeBox, makePlane, makePlaneFromPts,
           makePlaneFromNormal, getmsXYPlane, getmsXZPlane, getmsYZPlane,
           makeGizmoShape, makeCircle, };
};

class ManipHitData;

/*! \sa  Class HelperObject, Class SimpleManipulator, Class IManipulatorMgr, Class INode, Class ViewExp, Class IPoint2, Class Box3\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This is the base class for the development of Manipulator plug-ins. Shown are
the methods that need to be implemented by the plug-in. Most of these methods
are implemented by the <b>SimpleManipulator</b> class so most developers will
not need to use these unless <b>SimpleManipulator</b> is not sufficient.
\par Data Members:
protected:\n\n
<b>INode* mpINode;</b>\n\n
The node being manipulated  */
class Manipulator : public HelperObject
{
public:
    
    /*! \remarks Constructor. The node data member is initialized to the node
    passed. */
    ManipExport Manipulator(INode* pINode) { mpINode = pINode; }

    BOOL IsManipulator() { return TRUE; }

    /*! \remarks This method is called to determine if the specified screen
    point intersects the manipulator. The method returns nonzero if the item
    was hit; otherwise 0. This is like the normal <b>HitTest()</b> method in
    the <b>BaseObject</b> class. The difference is that it needs to log it hits
    in the viewport SubObjectHitList. It does this using the
    <b>ManipHitData</b> class defined in <b>Manipulator.h</b>.
    \par Parameters:
    <b>TimeValue t</b>\n\n
    The time to perform the hit test.\n\n
    <b>INode* pNode</b>\n\n
    A pointer to the node to test.\n\n
    <b>int type</b>\n\n
    The type of hit testing to perform. See \ref SceneAndNodeHitTestTypes.\n\n
    <b>int crossing</b>\n\n
    The state of the crossing setting. If TRUE crossing selection is on.\n\n
    <b>int flags</b>\n\n
    The hit test flags. See \ref SceneAndNodeHitTestFlags.\n\n
    <b>IPoint2 *pScreenPoint</b>\n\n
    Points to the screen point to test.\n\n
    <b>ViewExp *pVpt</b>\n\n
    An interface that may be used to call methods associated with the
    viewports.
    \return  Nonzero if the item was hit; otherwise 0. */
    virtual int HitTest(TimeValue t, INode* pNode, int type, int crossing,
                        int flags, IPoint2 *pScreenPoint, ViewExp *pVpt) = 0;
    /*! \remarks This method lines the <b>BaseObject::Display()</b> method and
    displays the manipulator object.
    \par Parameters:
    <b>TimeValue t</b>\n\n
    The time to display the object.\n\n
    <b>INode* pNode</b>\n\n
    Points to the node that is being manipulated by the manipulator.\n\n
    <b>ViewExp *pVpt</b>\n\n
    An interface that may be used to call methods associated with the
    viewports.\n\n
    <b>int flags</b>\n\n
    See <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_display_flags.html">List of Display Flags</a>.
    \return  The return value is not currently used. */
    virtual int Display(TimeValue t, INode* pNode, ViewExp *pVpt, int flags) = 0;

    /*! \remarks Used Internally.\n\n
    Returns the object space bounding box of the manipulator in the object's
    local coordinates.
    \par Parameters:
    <b>TimeValue t</b>\n\n
    The time to retrieve the bounding box.\n\n
    <b>INode* inode</b>\n\n
    The node that is being manipulated by the manipulator.\n\n
    <b>ViewExp* vp</b>\n\n
    An interface that may be used to call methods associated with the
    viewports.\n\n
    <b>Box3\& box</b>\n\n
    The bounding box is returned here. */
    virtual void GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vp,  Box3& box ) = 0;

    // Used for manipulator set manager, which is always active.
    /*! \remarks This method can be used to tell the manipulator management
    system that this manipulator is always active.
    \par Default Implementation:
    <b>{ return false; }</b> */
    ManipExport virtual bool AlwaysActive() { return false; }

    /*! \remarks This method returns the manipulator name string. The
    <b>SimpleManipulator</b> class uses this method for the tooltip in the
    viewport. */
    virtual MSTR& GetManipName() = 0;

    // FIXME these methods should use an FP interface.

    /*! \remarks This method gets called when the mouse first passes over a
    manipulator object. The return value is used to determine whether a redraw
    is needed or not. Normally manipulators display in a different color when
    the mouse is over them, so this should return <b>kFullRedrawNeeded</b>.
    \par Parameters:
    <b>TimeValue t</b>\n\n
    The time to display the object.\n\n
    <b>ViewExp* pVpt</b>\n\n
    An interface that may be used to call methods associated with the
    viewports.\n\n
    <b>IPoint2\& m</b>\n\n
    The location of the tooltip.\n\n
    <b>ManipHitData* pHitData</b>\n\n
    A pointer to the hitdata containing information on which manipulator was
    hit.
    \return  The display state indicating whether no redraw, a full redraw, or
    a post redraw is needed.
    \par Default Implementation:
    <b>{return kNoRedrawNeeded; }</b> */
	#pragma warning(push)
	#pragma warning(disable:4100)
    virtual DisplayState  MouseEntersObject(TimeValue t, ViewExp* pVpt, IPoint2& m, ManipHitData* pHitData)
        {return kNoRedrawNeeded; }
    /*! \remarks This method gets called when the mouse leaves the manipulator
    object. The return value is used to determine whether a redraw is needed or
    not. Normally manipulators display in a different color when the mouse is
    over them, so this should return <b>kFullRedrawNeeded</b>.
    \par Parameters:
    <b>TimeValue t</b>\n\n
    The time to display the object.\n\n
    <b>ViewExp* pVpt</b>\n\n
    An interface that may be used to call methods associated with the
    viewports.\n\n
    <b>IPoint2\& m</b>\n\n
    The location of the tooltip.\n\n
    <b>ManipHitData* pHitData</b>\n\n
    A pointer to the hitdata containing information on which manipulator was
    hit.
    \return  The display state indicating whether no redraw, a full redraw, or
    a post redraw is needed.
    \par Default Implementation:
    <b>{return kNoRedrawNeeded; }</b> */
    virtual DisplayState  MouseLeavesObject(TimeValue t, ViewExp* pVpt, IPoint2& m, ManipHitData* pHitData)
        {return kNoRedrawNeeded; }
    
    /*! \remarks This method gets called when the mouse is pressed down and
    moves within the manipulator context. It is the method that does the actual
    manipulator. It is up to the manipulator code to turn the mouse position
    into a new value for the parameter(s) being manipulated.
    \par Parameters:
    <b>TimeValue t</b>\n\n
    The time to display the object.\n\n
    <b>ViewExp* pVpt</b>\n\n
    An interface that may be used to call methods associated with the
    viewports.\n\n
    <b>IPoint2\& m</b>\n\n
    The location of the tooltip.\n\n
    <b>DWORD flags</b>\n\n
    Not used, should be set to 0.\n\n
    <b>ManipHitData* pHitData</b>\n\n
    A pointer to the hitdata containing information on which manipulator was
    hit.
    \par Default Implementation:
    <b>{ }</b> */
    virtual void OnMouseMove(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData) {}
    /*! \remarks This method gets called when the mouse buttons is first
    pressed down within the manipulator context.
    \par Parameters:
    <b>TimeValue t</b>\n\n
    The time to display the object.\n\n
    <b>ViewExp* pVpt</b>\n\n
    An interface that may be used to call methods associated with the
    viewports.\n\n
    <b>IPoint2\& m</b>\n\n
    The location of the tooltip.\n\n
    <b>DWORD flags</b>\n\n
    Not used, should be set to 0.\n\n
    <b>ManipHitData* pHitData</b>\n\n
    A pointer to the hitdata containing information on which manipulator was
    hit.
    \par Default Implementation:
    <b>{ }</b> */
    virtual void OnButtonDown(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData) {}
    /*! \remarks This method gets called when the mouse buttons is released
    within the manipulator context, and thus signals the end of the
    manipulation.
    \par Parameters:
    <b>TimeValue t</b>\n\n
    The time to display the object.\n\n
    <b>ViewExp* pVpt</b>\n\n
    An interface that may be used to call methods associated with the
    viewports.\n\n
    <b>IPoint2\& m</b>\n\n
    The location of the tooltip.\n\n
    <b>DWORD flags</b>\n\n
    Not used, should be set to 0.\n\n
    <b>ManipHitData* pHitData</b>\n\n
    A pointer to the hitdata containing information on which manipulator was
    hit.
    \par Default Implementation:
    <b>{ }</b> */
    virtual void OnButtonUp(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData) {}
	#pragma warning(pop) // 4100
    /*! \remarks This method returns a pointer to the INode that is currently
    being manipulated.
    \par Default Implementation:
    <b>{ return mpINode; }</b> */
    virtual INode* GetINode() { return mpINode; }

    /*! \remarks This method deletes the manipulator instance.
    \par Default Implementation:
    <b>{ delete this; }</b> */
    ManipExport virtual void DeleteThis();

protected:

    INode*  mpINode;   // The node being manipulated

};

class ManipHitList;

/*! \sa  Class BaseInterfaceServer,  Class PolyShape, Class Mesh,  Class Point3,  Class Point2, Class Matrix3, Class HitRegion, Class GraphicsWindow, Class ViewExp, Class INode, <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_marker_types.html">List of Marker Types</a>,
<a href="ms-its:3dsmaxsdk.chm::/vports_hit_testing.html">Hit Testing</a>\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This is the base class for the creation of manipulator gizmo objects and can
hold a single Mesh, PolyLine, Marker, or Text gizmo.
\par Data Members:
private:\n\n
<b>PolyShape* mpShape</b>\n\n
The polyshape gizmo.\n\n
<b>Mesh* mpMesh</b>\n\n
The mesh gizmo.\n\n
<b>Point3 mPosition</b>\n\n
The position for markers and text.\n\n
<b>MarkerType* mpMarkerType</b>\n\n
The marker type for marker gizmos. See the
<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_marker_types.html">List of Marker Types</a>\n\n
<b>MSTR* mpText</b>\n\n
The string used for text gizmos\n\n
<b>Point3 mUnselColor</b>\n\n
The color of the gizmo when unselected.\n\n
<b>Point3 mSelColor</b>\n\n
The color of the gizmo when selected.\n\n
<b>DWORD mFlags</b>\n\n
Display and hit testing flags.\n\n
<b>int mGizmoSize</b>\n\n
The size of the gizmo in pixels for <b>kGizmoScaleToViewport</b> gizmos
\par Data Members:
Display and hit-test flags.\n\n
<b>static const DWORD kGizmoDontDisplay</b>\n\n
Don't display this gizmo, it is still hit-tested.\n\n
<b>static const DWORD kGizmoDontHitTest</b>\n\n
Don't hit test this gizmo, it is still displayed.\n\n
<b>static const DWORD kGizmoScaleToViewport</b>\n\n
Tells the gizmo to scale itself to have a constant size in the viewport. In
this case, the system uses the <b>mGizmoSize</b> to determine how big the
manipulator should be. It interprets <b>mGizmoSize</b> as pixels it this case.
This flag only applies to mesh gizmo currently.\n\n
<b>static const DWORD kGizmoUseRelativeScreenSpace</b>\n\n
The coordinates are in normalized screen space. the X and Y values are in the
range 0.0 to 1.0, and interpreted as percentages of screen space. Note: This is
only supported for PolyShape, Marker and Text gizmos.\n\n
<b>static const DWORD kGizmoUseScreenSpace</b>\n\n
The coordinates are in screen space. Note: This is only supported for
PolyShape, Marker and Text gizmos.\n\n
<b>static const DWORD kGizmoActiveViewportOnly</b>\n\n
Only display the gizmo in the active viewport.  */
class ManipulatorGizmo : public BaseInterfaceServer
{
public:
    /*! \remarks Constructor. */
    ManipExport ManipulatorGizmo();
	 /*! \remarks Constructor. This constructor method is used to initialize a
	 polyshape manipulator.
	 \par Parameters:
	 <b>PolyShape* mpShape</b>\n\n
	 The polyshape gizmo.\n\n
	 <b>DWORD flags</b>\n\n
	 Display and hit testing flags.\n\n
	 <b>Point3\& unselColor</b>\n\n
	 The color of the gizmo when unselected.\n\n
	 <b>Point3\& selColor</b>\n\n
	 The color of the gizmo when selected. */
    ManipExport ManipulatorGizmo(PolyShape* pShape, DWORD flags,
                     Point3& unselColor,
                     Point3& selColor =  GetSubSelColor());
	 /*! \remarks Constructor. This constructor method is used to initialize a mesh
	 manipulator.
	 \par Parameters:
	 <b>Mesh* pMesh</b>\n\n
	 The polyshape gizmo.\n\n
	 <b>DWORD flags</b>\n\n
	 Display and hit testing flags.\n\n
	 <b>Point3\& unselColor</b>\n\n
	 The color of the gizmo when unselected.\n\n
	 <b>Point3\& selColor</b>\n\n
	 The color of the gizmo when selected. */
    ManipExport ManipulatorGizmo(Mesh* pMesh, DWORD flags,
                     Point3& unselColor,
                     Point3& selColor = GetSubSelColor());
	 /*! \remarks Constructor. This constructor method is used to initialize a
	 marker manipulator.
	 \par Parameters:
	 <b>MarkerType markerType</b>\n\n
	 The marker type. See the <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_marker_types.html">List of
	 Marker Types</a>\n\n
	 <b>Point3\& position</b>\n\n
	 The marker position.\n\n
	 <b>DWORD flags</b>\n\n
	 Display and hit testing flags.\n\n
	 <b>Point3\& unselColor</b>\n\n
	 The color of the gizmo when unselected.\n\n
	 <b>Point3\& selColor</b>\n\n
	 The color of the gizmo when selected. */
    ManipExport ManipulatorGizmo(MarkerType markerType, Point3& position,
                     DWORD flags,
                     Point3& unselColor,
                     Point3& selColor = GetSubSelColor());
	 /*! \remarks Constructor. This constructor method is used to initialize a text
	 manipulator.
	 \par Parameters:
	 <b>MCHAR* pText</b>\n\n
	 The text string for the manipulator\n\n
	 <b>Point3\& position</b>\n\n
	 The manipulator position.\n\n
	 <b>DWORD flags</b>\n\n
	 Display and hit testing flags.\n\n
	 <b>Point3\& unselColor</b>\n\n
	 The color of the gizmo when unselected.\n\n
	 <b>Point3\& selColor</b>\n\n
	 The color of the gizmo when selected. */
    ManipExport ManipulatorGizmo(MCHAR* pText, Point3& position,
                     DWORD flags,
                     Point3& unselColor,
                     Point3& selColor = GetSubSelColor());
    /*! \remarks Destructor. */
    ManipExport ~ManipulatorGizmo();
                     

    /*! \remarks Used Internally.\n\n
    This method is called when a hit test on a manipulator is needed.
    \par Parameters:
    <b>GraphicsWindow* pGW</b>\n\n
    The graphics window where the hit test is done.\n\n
    <b>HitRegion* pHR</b>\n\n
    The hit region. See Class HitRegion.\n\n
    <b>ManipHitList* pHitList</b>\n\n
    A pointer to the hit list.\n\n
    <b>Matrix3* tm</b>\n\n
    The transformation matrix.\n\n
    <b>IPoint2 pScreenPoint</b>\n\n
    The screen point to test.\n\n
    <b>int gizmoIndex</b>\n\n
    The index of the gizmo.
    \return  TRUE if the manipulator was hit, otherwise FALSE. */
    ManipExport BOOL HitTest(GraphicsWindow* pGW, HitRegion* pHR, ManipHitList* pHitList,
                 Matrix3* tm, IPoint2 pScreenPoint, int gizmoIndex);

    /*! \remarks This method will render the manipulator gizmo.
    \par Parameters:
    <b>ViewExp* pVpt</b>\n\n
    An interface that may be used to call methods associated with the
    viewports.\n\n
    <b>TimeValue t</b>\n\n
    The time value to render at.\n\n
    <b>INode* pNode</b>\n\n
    A pointer to the node to render.\n\n
    <b>int flags</b>\n\n
    The manipulator flags.\n\n
    <b>bool selected</b>\n\n
    This flag sets the selected or unselected state. This will instruct the
    render process to use the proper color for the manipulator.\n\n
    <b>bool isStandAlone</b>\n\n
    This flag sets whether the manipulator is a stand alone manipulator (i.e.
    manipulators which manipulate themselves such as the slider manipulator).
    */
    ManipExport void Render(ViewExp* pVpt, TimeValue t, INode* pNode, int flags, bool selected, BOOL isStandAlone);

    /*! \remarks Used Internally.\n\n
    This method returns the bounding box of the manipulator.
    \par Parameters:
    <b>INode* pNode</b>\n\n
    A pointer to the node to calculate the bounding box for.\n\n
    <b>ViewExp* pVpt</b>\n\n
    An interface that may be used to call methods associated with the
    viewports. <br>  private: */
    ManipExport Box3 GetBoundingBox(INode* pNode, ViewExp* pVpt);

    // Gizmo flags

    // Don't display this gizmo.  It is still hit-tested.
    ManipExport static const DWORD kGizmoDontDisplay;

    // Don't hit test this gizmo.  It is still displayed.
    ManipExport static const DWORD kGizmoDontHitTest;

    // Scale this gizmo to viewport size, using mGizmoSize as the size in pixels
    // Only for mesh and shape gizmos.
    ManipExport static const DWORD kGizmoScaleToViewport;

    // The coordinates are in normalized screen space.  the X and Y values are
    // in the range 0.0 to 1.0, and interpreted as percentages of screen space.
    // This is only supported for PolyShape, Marker and Text gizmos.
    ManipExport static const DWORD kGizmoUseRelativeScreenSpace;  

    // The coordinates are in screen space.  
    // This is only supported for PolyShape, Marker and Text gizmos.
    ManipExport static const DWORD kGizmoUseScreenSpace;  

    // Only display the gizmo in the active viewport.
    ManipExport static const DWORD kGizmoActiveViewportOnly;

private:
    /*! \remarks This method will render the mesh based manipulator gizmo.
    \par Parameters:
    <b>ViewExp* pVpt</b>\n\n
    An interface that may be used to call methods associated with the
    viewports.\n\n
    <b>TimeValue t</b>\n\n
    The time value to render at.\n\n
    <b>INode* pNode</b>\n\n
    A pointer to the node to render.\n\n
    <b>int flags</b>\n\n
    The manipulator flags.\n\n
    <b>bool selected</b>\n\n
    This flag sets the selected or unselected state. This will instruct the
    render process to use the proper color for the manipulator.\n\n
    <b>bool isStandAlone</b>\n\n
    This flag sets whether the manipulator is a stand alone manipulator (i.e.
    manipulators which manipulate themselves such as the slider manipulator).
    */
    void RenderMesh(ViewExp* pVpt, TimeValue t, INode* pNode, int flags, bool selected, BOOL isStandAlone);
    /*! \remarks This method will render the shape based manipulator gizmo.
    \par Parameters:
    <b>ViewExp* pVpt</b>\n\n
    An interface that may be used to call methods associated with the
    viewports.\n\n
    <b>TimeValue t</b>\n\n
    The time value to render at.\n\n
    <b>INode* pNode</b>\n\n
    A pointer to the node to render.\n\n
    <b>int flags</b>\n\n
    The manipulator flags.\n\n
    <b>bool selected</b>\n\n
    This flag sets the selected or unselected state. This will instruct the
    render process to use the proper color for the manipulator.\n\n
    <b>bool isStandAlone</b>\n\n
    This flag sets whether the manipulator is a stand alone manipulator (i.e.
    manipulators which manipulate themselves such as the slider manipulator).
    */
    void RenderShape(ViewExp* pVpt, TimeValue t, INode* pNode, int flags, bool selected, BOOL isStandAlone);
    /*! \remarks This method will render the marker based manipulator gizmo.
    \par Parameters:
    <b>ViewExp* pVpt</b>\n\n
    An interface that may be used to call methods associated with the
    viewports.\n\n
    <b>TimeValue t</b>\n\n
    The time value to render at.\n\n
    <b>INode* pNode</b>\n\n
    A pointer to the node to render.\n\n
    <b>int flags</b>\n\n
    The manipulator flags.\n\n
    <b>bool selected</b>\n\n
    This flag sets the selected or unselected state. This will instruct the
    render process to use the proper color for the manipulator.\n\n
    <b>bool isStandAlone</b>\n\n
    This flag sets whether the manipulator is a stand alone manipulator (i.e.
    manipulators which manipulate themselves such as the slider manipulator).
    */
    void RenderMarker(ViewExp* pVpt, TimeValue t, INode* pNode, int flags, bool selected, BOOL isStandAlone);
    /*! \remarks This method will render the text based manipulator gizmo.
    \par Parameters:
    <b>ViewExp* pVpt</b>\n\n
    An interface that may be used to call methods associated with the
    viewports.\n\n
    <b>TimeValue t</b>\n\n
    The time value to render at.\n\n
    <b>INode* pNode</b>\n\n
    A pointer to the node to render.\n\n
    <b>int flags</b>\n\n
    The manipulator flags.\n\n
    <b>bool selected</b>\n\n
    This flag sets the selected or unselected state. This will instruct the
    render process to use the proper color for the manipulator.\n\n
    <b>bool isStandAlone</b>\n\n
    This flag sets whether the manipulator is a stand alone manipulator (i.e.
    manipulators which manipulate themselves such as the slider manipulator).
    */
    void RenderText(ViewExp* pVpt, TimeValue t, INode* pNode, int flags, bool selected, BOOL isStandAlone);

    /*! \remarks Used Internally.\n\n
    This method is called when a hit test on a shape based manipulator is
    needed.
    \par Parameters:
    <b>GraphicsWindow* pGW</b>\n\n
    The graphics window where the hit test is done.\n\n
    <b>HitRegion* pHR</b>\n\n
    The hit region. See Class HitRegion.\n\n
    <b>ManipHitList* pHitList</b>\n\n
    A pointer to the hit list.\n\n
    <b>Matrix3* tm</b>\n\n
    The transformation matrix.\n\n
    <b>IPoint2 pScreenPoint</b>\n\n
    The screen point to test.\n\n
    <b>int gizmoIndex</b>\n\n
    The index of the gizmo.
    \return  TRUE if the manipulator was hit, otherwise FALSE. */
    BOOL HitTestShape(GraphicsWindow* pGW, HitRegion* pHR, ManipHitList* pHitList,
                      Matrix3* tm, IPoint2 pScreenPoint, int gizmoIndex);
    /*! \remarks Used Internally.\n\n
    This method is called when a hit test on a mesh based manipulator is
    needed.
    \par Parameters:
    <b>GraphicsWindow* pGW</b>\n\n
    The graphics window where the hit test is done.\n\n
    <b>HitRegion* pHR</b>\n\n
    The hit region. See Class HitRegion.\n\n
    <b>ManipHitList* pHitList</b>\n\n
    A pointer to the hit list.\n\n
    <b>Matrix3* tm</b>\n\n
    The transformation matrix.\n\n
    <b>IPoint2 pScreenPoint</b>\n\n
    The screen point to test.\n\n
    <b>int gizmoIndex</b>\n\n
    The index of the gizmo.
    \return  TRUE if the manipulator was hit, otherwise FALSE. */
    BOOL HitTestMesh(GraphicsWindow* pGW, HitRegion* pHR, ManipHitList* pHitList,
                     Matrix3* tm, IPoint2 pScreenPoint, int gizmoIndex);
    /*! \remarks Used Internally.\n\n
    This method is called when a hit test on a marker based manipulator is
    needed.
    \par Parameters:
    <b>GraphicsWindow* pGW</b>\n\n
    The graphics window where the hit test is done.\n\n
    <b>HitRegion* pHR</b>\n\n
    The hit region. See Class HitRegion.\n\n
    <b>ManipHitList* pHitList</b>\n\n
    A pointer to the hit list.\n\n
    <b>Matrix3* tm</b>\n\n
    The transformation matrix.\n\n
    <b>IPoint2 pScreenPoint</b>\n\n
    The screen point to test.\n\n
    <b>int gizmoIndex</b>\n\n
    The index of the gizmo.
    \return  TRUE if the manipulator was hit, otherwise FALSE. */
    BOOL HitTestMarker(GraphicsWindow* pGW, HitRegion* pHR, ManipHitList* pHitList,
                      Matrix3* tm, IPoint2 pScreenPoint, int gizmoIndex);
    /*! \remarks Used Internally.\n\n
    This method is called when a hit test on a text based manipulator is
    needed.
    \par Parameters:
    <b>GraphicsWindow* pGW</b>\n\n
    The graphics window where the hit test is done.\n\n
    <b>HitRegion* pHR</b>\n\n
    The hit region. See Class HitRegion.\n\n
    <b>ManipHitList* pHitList</b>\n\n
    A pointer to the hit list.\n\n
    <b>Matrix3* tm</b>\n\n
    The transformation matrix.\n\n
    <b>IPoint2 pScreenPoint</b>\n\n
    The screen point to test.\n\n
    <b>int gizmoIndex</b>\n\n
    The index of the gizmo.
    \return  TRUE if the manipulator was hit, otherwise FALSE. */
    BOOL HitTestText(GraphicsWindow* pGW, HitRegion* pHR, ManipHitList* pHitList,
                     Matrix3* tm, IPoint2 pScreenPoint, int gizmoIndex);

    /*! \remarks Used Internally.\n\n
    This method returns the bounding box of the shape based manipulator.
    \par Parameters:
    <b>INode* pNode</b>\n\n
    A pointer to the node to calculate the bounding box for.\n\n
    <b>ViewExp* pVpt</b>\n\n
    An interface that may be used to call methods associated with the
    viewports. */
    Box3 GetShapeBoundingBox(INode* pNode, ViewExp* pVpt);
    /*! \remarks Used Internally.\n\n
    This method returns the bounding box of the mesh based manipulator.
    \par Parameters:
    <b>INode* pNode</b>\n\n
    A pointer to the node to calculate the bounding box for.\n\n
    <b>ViewExp* pVpt</b>\n\n
    An interface that may be used to call methods associated with the
    viewports. */
    Box3 GetMeshBoundingBox(INode* pNode, ViewExp* pVpt);
    /*! \remarks Used Internally.\n\n
    This method returns the bounding box of the marker based manipulator.
    \par Parameters:
    <b>INode* pNode</b>\n\n
    A pointer to the node to calculate the bounding box for.\n\n
    <b>ViewExp* pVpt</b>\n\n
    An interface that may be used to call methods associated with the
    viewports. */
    Point3 GetMarkerBoundingBox(INode* pNode, ViewExp* pVpt);
    /*! \remarks Used Internally.\n\n
    This method returns the bounding box of the text based manipulator.
    \par Parameters:
    <b>INode* pNode</b>\n\n
    A pointer to the node to calculate the bounding box for.\n\n
    <b>ViewExp* pVpt</b>\n\n
    An interface that may be used to call methods associated with the
    viewports. */
    Box3 GetTextBoundingBox(INode* pNode, ViewExp* pVpt);

    /*! \remarks This method calculates the scaling factor and center of the
    manipulator gizmo.
    \par Parameters:
    <b>GraphicsWindow* pGW</b>\n\n
    A pointer to the graphics window.\n\n
    <b>Point3\& scale</b>\n\n
    The scale factor will be placed here.\n\n
    <b>Point3\& center</b>\n\n
    The center of the manipulator will be placed here. */
    void GetScaleFactor(GraphicsWindow* pGW, Point3& scale, Point3& center);

    /*! \remarks This method will return the screen coordinates of the
    manipulator gizmo in x and y based on the input coordinates you supply.
    This method will return absolute coordinates if the gizmo is in relative
    screen space.
    \par Parameters:
    <b>GraphicsWindow* pGW</b>\n\n
    A pointer to the graphics window.\n\n
    <b>Point3\& input</b>\n\n
    The coordinates of the manipulator gizmo.\n\n
    <b>int\& x, int\& y</b>\n\n
    The screen coordinates will be placed here. */
    void GetScreenCoords(GraphicsWindow* pGW, Point3& input, int& x, int& y);

    /*! \remarks This method returns TRUE if the manipulator is being used in
    screen space.
    \par Default Implementation:
    <b>{ return mFlags \& kGizmoUseRelativeScreenSpace ||</b>\n\n
    <b>mFlags \& kGizmoUseScreenSpace; }</b> */
    BOOL UseScreenSpace() { return mFlags & kGizmoUseRelativeScreenSpace ||
                                   mFlags & kGizmoUseScreenSpace; }

    PolyShape*  mpShape;      // Polyshape gizmo
    Mesh*       mpMesh;       // Mesh gizmo

    Point3      mPosition;    // Used for markers and text
    MarkerType* mpMarkerType; // Used for marker gizmos
    MSTR*       mpText;       // Used for text gizmos 

    Point3      mUnselColor;  // Color of gizmo
    Point3      mSelColor;    // Color of gizmo when selected
    DWORD       mFlags;       // Display and hit testing flags

    // The size of the gizmo in pixels for kGizmoScaleToViewport gizmos.
    int         mGizmoSize;
};

enum MouseState {
    kMouseIdle,
    kMouseDragging,
    kMouseOverManip,
};

// Manipulator with a built-in ParamBlock2 and many methods implemented
// by default.
// SimpleManipulator also provides support for a table of meshes ,
// poly shapes, markers and text for use as gizmos.

// FnPub interface to SimpleManipulators (for scripted manipulators)
#define SIMPLE_MANIP_INTERFACE  Interface_ID(0x617c41d4, 0x6af06a5f)

/*! \sa  Class FPMixinInterface,  Class SimpleManipulator, Class Interface, Class PolyShape, Class Mesh,  Class Point3,  Class Point2, Class ViewExp, Class IColorManager, Class Ray, Class ManipHitData, <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_marker_types.html">List of Marker Types</a>\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
The ISimpleManipulator class is an interface to SimpleManipulators with
built-in ParamBlock2 support and a variety of additionally useful methods. <br>
*/
class ISimpleManipulator : public FPMixinInterface
{
public:
	// the published API
    /*! \remarks Implemented by the system.\n\n
    Removes all of the current gizmos in the manipulator. This is normally
    called at the top of <b>UpdateShapes()</b> to clear out any previous gizmos
    before creating new ones. */
    virtual void	ClearPolyShapes()=0;

    /*! \remarks Implemented by the system.\n\n
    This method adds a new PolyShape gizmo to the manipulator. The shape is
    defined in the local coordinates of the node that owns the manipulator.
    \par Parameters:
    <b>PolyShape* pPolyShape</b>\n\n
    A pointer to the poly shape to add.\n\n
    <b>DWORD flags = 0</b>\n\n
    The flags can have one or more of the following values:\n\n
    <b>kGizmoDontDisplay</b>\n\n
    Instruct the gizmo not to display. It will still hit-test.\n\n
    <b>kGizmoDontHitTest</b>\n\n
    Instruct the gizmo not to do any hit testing. It will still display.\n\n
    <b>kGizmoScaleToViewport</b>\n\n
    Instruct the gizmo to scale itself to have a constant size in the viewport.
    In this case the system will use the <b>ManipulatorGizmo::mGizmoSize</b> to
    determine how big the manipulator should be. It interprets
    <b>mGizmoSize</b> as pixels in this case.\n\n
    <b>Point3\& unselColor = GetUIColor(COLOR_SEL_GIZMOS)</b>\n\n
    The color of the gizmo when unselected.\n\n
    <b>Point3\& selColor = GetSubSelColor()</b>\n\n
    The color of the gizmo when selected. */
    virtual void	AppendPolyShape(PolyShape* pPolyShape, DWORD flags,
                                    Point3& unselColor,
                                    Point3& selColor =  ColorMan()->GetColorAsPoint3(kManipulatorsSelected))=0;
    
    /*! \remarks Implemented by the system.\n\n
    This method adds a new GizmoShape to the manipulator. The shape is defined
    in the local coordinates of the node that owns the manipulator.
    \par Parameters:
    <b>GizmoShape* pGizmoShape</b>\n\n
    A pointer to the gizmo shape to add.\n\n
    <b>DWORD flags = 0</b>\n\n
    The flags can have one or more of the following values:\n\n
    <b>kGizmoDontDisplay</b>\n\n
    Instruct the gizmo not to display. It will still hit-test.\n\n
    <b>kGizmoDontHitTest</b>\n\n
    Instruct the gizmo not to do any hit testing. It will still display.\n\n
    <b>kGizmoScaleToViewport</b>\n\n
    Instruct the gizmo to scale itself to have a constant size in the viewport.
    In this case the system will use the <b>ManipulatorGizmo::mGizmoSize</b> to
    determine how big the manipulator should be. It interprets
    <b>mGizmoSize</b> as pixels in this case.\n\n
    <b>Point3\& unselColor = GetUIColor(COLOR_SEL_GIZMOS)</b>\n\n
    The color of the gizmo when unselected.\n\n
    <b>Point3\& selColor = GetSubSelColor()</b>\n\n
    The color of the gizmo when selected. */
    virtual void	AppendGizmo(GizmoShape* pGizmoShape, DWORD flags,
                                Point3& unselColor,
                                Point3& selColor =  ColorMan()->GetColorAsPoint3(kManipulatorsSelected))=0;
    
    /*! \remarks Implemented by the system.\n\n
    This method adds a new mesh to the manipulator. The mesh is defined in the
    local coordinates of the node that owns the manipulator.
    \par Parameters:
    <b>Mesh* pMesh</b>\n\n
    A pointer to the mesh to add.\n\n
    <b>DWORD flags = 0</b>\n\n
    The flags can have one or more of the following values:\n\n
    <b>kGizmoDontDisplay</b>\n\n
    Instruct the gizmo not to display. It will still hit-test.\n\n
    <b>kGizmoDontHitTest</b>\n\n
    Instruct the gizmo not to do any hit testing. It will still display.\n\n
    <b>kGizmoScaleToViewport</b>\n\n
    Instruct the gizmo to scale itself to have a constant size in the viewport.
    In this case the system will use the <b>ManipulatorGizmo::mGizmoSize</b> to
    determine how big the manipulator should be. It interprets
    <b>mGizmoSize</b> as pixels in this case.\n\n
    <b>Point3\& unselColor = GetUIColor(COLOR_SEL_GIZMOS)</b>\n\n
    The color of the gizmo when unselected.\n\n
    <b>Point3\& selColor = GetSubSelColor()</b>\n\n
    The color of the gizmo when selected. */
    virtual void	AppendMesh(Mesh* pMesh, DWORD flags,
                               Point3& unselColor,
                               Point3& selColor =  ColorMan()->GetColorAsPoint3(kManipulatorsSelected))=0;
    
    /*! \remarks Implemented by the system.\n\n
    This method adds a new marker to the manipulator.
    \par Parameters:
    <b>MarkerType markerType</b>\n\n
    The marker type to add.\n\n
    <b>Point3\& position</b>\n\n
    The position of the marker.\n\n
    <b>DWORD flags = 0</b>\n\n
    The flags can have one or more of the following values:\n\n
    <b>kGizmoDontDisplay</b>\n\n
    Instruct the gizmo not to display. It will still hit-test.\n\n
    <b>kGizmoDontHitTest</b>\n\n
    Instruct the gizmo not to do any hit testing. It will still display.\n\n
    <b>kGizmoScaleToViewport</b>\n\n
    Instruct the gizmo to scale itself to have a constant size in the viewport.
    In this case the system will use the <b>ManipulatorGizmo::mGizmoSize</b> to
    determine how big the manipulator should be. It interprets
    <b>mGizmoSize</b> as pixels in this case.\n\n
    <b>Point3\& unselColor = GetUIColor(COLOR_SEL_GIZMOS)</b>\n\n
    The color of the gizmo when unselected.\n\n
    <b>Point3\& selColor = GetSubSelColor()</b>\n\n
    The color of the gizmo when selected. */
    virtual void	AppendMarker(MarkerType markerType, Point3& position,
                                 DWORD flags,
                                 Point3& unselColor,
                                 Point3& selColor =  ColorMan()->GetColorAsPoint3(kManipulatorsSelected))=0;
    
    /*! \remarks Implemented by the system.\n\n
    This method adds a new text entry to the manipulator.
    \par Parameters:
    <b>MCHAR* pText</b>\n\n
    The string containing the text to add.\n\n
    <b>Point3\& position</b>\n\n
    The position of the text.\n\n
    <b>DWORD flags = 0</b>\n\n
    The flags can have one or more of the following values:\n\n
    <b>kGizmoDontDisplay</b>\n\n
    Instruct the gizmo not to display. It will still hit-test.\n\n
    <b>kGizmoDontHitTest</b>\n\n
    Instruct the gizmo not to do any hit testing. It will still display.\n\n
    <b>kGizmoScaleToViewport</b>\n\n
    Instruct the gizmo to scale itself to have a constant size in the viewport.
    In this case the system will use the <b>ManipulatorGizmo::mGizmoSize</b> to
    determine how big the manipulator should be. It interprets
    <b>mGizmoSize</b> as pixels in this case.\n\n
    <b>Point3\& unselColor = GetUIColor(COLOR_SEL_GIZMOS)</b>\n\n
    The color of the gizmo when unselected.\n\n
    <b>Point3\& selColor = GetSubSelColor()</b>\n\n
    The color of the gizmo when selected. */
    virtual void	AppendText(MCHAR* pText, Point3& position,
                               DWORD flags,
                               Point3& unselColor,
                               Point3& selColor =  ColorMan()->GetColorAsPoint3(kManipulatorsSelected))=0;
    
    /*! \remarks This method returns the current status of the mouse.
    \return  One of the following values:\n\n
    <b>kMouseIdle</b>\n\n
    The mouse is idle, manipulator not active and the mouse is not over it.\n\n
    <b>kMouseDragging</b>\n\n
    The mouse is currently dragging the manipulator.\n\n
    <b>kMouseOverManip</b>\n\n
    The mouse is over the manipulator, but it is not being dragged. */
    virtual MouseState GetMouseState()=0;
    /*! \remarks This method gets the view ray going through the given screen
    coordinate. The result is in local coordinates of the owning INode.
    \par Parameters:
    <b>ViewExp* pVpt</b>\n\n
    An interface that may be used to call methods associated with the
    viewports.\n\n
    <b>IPoint2\& m</b>\n\n
    The screen coordinate.\n\n
    <b>Ray\& viewRay</b>\n\n
    The returned local view ray. */
    virtual void	GetLocalViewRay(ViewExp* pVpt, IPoint2& m, Ray& viewRay)=0;;
    /*! \remarks This method gets called whenever the manipulator needs to update its
    gizmos. This is implemented by the manipulator to create the gizmos based
    on the current state of the node being manipulated.
    \par Parameters:
    <b>TimeValue t</b>\n\n
    The time value at which the shapes should be updated.\n\n
    <b>MSTR\& toolTip</b>\n\n
    The tooltip string. */
    virtual void	UpdateShapes(TimeValue t, MSTR& toolTip)=0;
    /*! \remarks This method gets called when the mouse moves within the
    manipulator context.
    \par Parameters:
    <b>TimeValue t</b>\n\n
    The time to display the object.\n\n
    <b>ViewExp* pVpt</b>\n\n
    An interface that may be used to call methods associated with the
    viewports.\n\n
    <b>IPoint2\& m</b>\n\n
    The screen coordinates of the mouse.\n\n
    <b>DWORD flags</b>\n\n
    Not used, should be set to 0.\n\n
    <b>ManipHitData* pHitData</b>\n\n
    A pointer to the hitdata containing information on which manipulator was
    hit. */
    virtual void	OnMouseMove(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData)=0;

	// Function IDs
	enum { clearPolyShapes, appendPolyShape, appendMesh, getMouseState, getLocalViewRay, 
			updateShapes, onMouseMove, appendGizmo, appendMarker, appendText};
	// enumeration IDs
	enum { mouseState, markerType, };
	// Function Map
	#pragma warning(push)
	#pragma warning(disable:4238)
	BEGIN_FUNCTION_MAP
		VFN_0(clearPolyShapes,				  ClearPolyShapes	 );
		VFN_4(appendMesh,					  FPAppendMesh,		 TYPE_MESH, TYPE_INT, TYPE_POINT3_BV, TYPE_POINT3_BV);
		VFN_4(appendGizmo,					  FPAppendGizmo,	 TYPE_INTERFACE, TYPE_INT, TYPE_POINT3_BV, TYPE_POINT3_BV);
		VFN_5(appendMarker,					  FPAppendMarker,    TYPE_ENUM, TYPE_POINT3_BV, TYPE_INT, TYPE_POINT3_BV, TYPE_POINT3_BV);
		VFN_5(appendText,					  AppendText,	     TYPE_STRING, TYPE_POINT3_BV, TYPE_INT, TYPE_POINT3_BV, TYPE_POINT3_BV);
		VFN_2(updateShapes,					  UpdateShapes,		 TYPE_TIMEVALUE, TYPE_TSTR_BR);
//		VFN_3(onMouseMove,					  FPOnMouseMove,	 TYPE_TIMEVALUE, TYPE_POINT2_BV, TYPE_INT);
		FN_1(getLocalViewRay,    TYPE_RAY_BV, FPGetLocalViewRay, TYPE_POINT2_BV);
		RO_PROP_FN(getMouseState,			  GetMouseState,	 TYPE_ENUM);
	END_FUNCTION_MAP
	#pragma warning(pop) // 4238

	// FP interface type-converter wrappers
	ManipExport Ray		FPGetLocalViewRay(Point2& m);
	ManipExport void	FPAppendMesh(Mesh* pMesh, DWORD flags, Point3& unselColor, Point3& selColor);
	ManipExport void	FPAppendGizmo(FPInterface* pGizmo, DWORD flags, Point3& unselColor, Point3& selColor);
//	ManipExport void	FPOnMouseMove(TimeValue t, Point2& m, DWORD flags);
    ManipExport void	FPAppendMarker(int markerType, Point3& position,
                                 DWORD flags, Point3& unselColor, Point3& selColor);

	ManipExport FPInterfaceDesc* GetDesc();
};

/*! \sa  Class Manipulator, Class ISimpleManipulator, Class PolyShape, Class GizmoShape, Class ReferenceTarget, Class IParamBlock2, Class INode, Class Mesh,  Class Point3, Class Point2, Class ViewExp, Class IColorManager, Class Ray, Class ManipHitData, <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_marker_types.html">List of Marker Types</a>\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
The SimpleManipulator class provides a framework for implementing many common
manipulators. It provides the following services:\n\n
*  It supports an arbitrary number of gizmos made from PolyShape and/or
Mesh objects.\n\n
*  It creates and maintains tool tips in the viewport.\n\n
*  It does hit testing, display and bounding box computations of the
gizmos.\n\n
*  It maintains an IParamBlock2 for the parameters of the Manipulator.\n\n
This class maintains a pointer to a parameter block. If the client of
<b>SimpleManipulator</b> uses a single parameter block then
<b>SimpleManipulator</b> can manage all the methods associated with SubAnims
and References for the client.\n\n
If the client of <b>SimpleManipulator</b> maintains several parameter blocks
then the client must implement the methods <b>NumSubs(), SubAnim(i),
SubAnimName(i), NumRefs(), GetReference(i)</b> and <b>SetReference(i)</b> and
call the <b>SimpleManipulator</b> methods when 'i' refers to the parameters
maintained by <b>SimpleManipulator</b>.\n\n
Samples of Manipulators can be found in the SDK,
<b>/MAXSDK/SAMPLES/MANIPULATORS</b>.\n\n
The Function Publishing interface to SimpleManipulators is defined as:\n\n
<b>#define SIMPLE_MANIP_INTERFACE Interface_ID(0x617c41d4, 0x6af06a5f)</b>\n\n
The following functions are not part of this class but are available for use
with it in the making of gizmo objects: */
class SimpleManipulator: public Manipulator, public ISimpleManipulator
{
public:

    /*! \remarks Constructor. */
    ManipExport SimpleManipulator();
    /*! \remarks Constructor. */
    ManipExport SimpleManipulator(INode* pNode);
    /*! \remarks Destructor. */
    ManipExport ~SimpleManipulator();

    // ReferenceMaker functions
    ManipExport int NumRefs();
    ManipExport RefTargetHandle GetReference(int i);
protected:
    ManipExport virtual void SetReference(int i, RefTargetHandle rtarg);
public:
    ManipExport RefResult NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,
                               PartID& partID, RefMessage message);
    
    // From Object
    ManipExport ObjectState Eval(TimeValue time);
    void InitNodeName(MSTR& s) {s = GetObjectName();}
    ManipExport Interval ObjectValidity(TimeValue t);
    
    // From GeomObject
    ManipExport void GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box );
    ManipExport void GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box );
    ManipExport void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel );
    ManipExport void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
    ManipExport void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
    
    // Animatable methods
    ManipExport void GetClassName(MSTR& s) {s = GetObjectName();}		
    ManipExport int NumSubs() { return 1; }
    ManipExport Animatable* SubAnim(int i) { UNUSED_PARAM(i); return mpPblock; }
    ManipExport MSTR SubAnimName(int i);
    
    ManipExport	BaseInterface* GetInterface(Interface_ID id) ;

    // Implement the basic manipulator operations
    ManipExport int HitTest(TimeValue t, INode* pNode, int type, int crossing,
                int flags, IPoint2 *pScreenPoint, ViewExp *pVpt);
    ManipExport int Display(TimeValue t, INode* pNode, ViewExp *pVpt, int flags);
    ManipExport static const int kNoneSelected;

    /*! \remarks Implemented by the system.\n\n
    Removes all of the current gizmos in the manipulator. This is normally
    called at the top of <b>UpdateShapes()</b> to clear out any previous gizmos
    before creating new ones. */
    ManipExport void ClearPolyShapes();
    /*! \remarks Implemented by the system.\n\n
    This method adds a new PolyShape gizmo to the manipulator. The shape is
    defined in the local coordinates of the node that owns the manipulator.
    \par Parameters:
    <b>PolyShape* pPolyShape</b>\n\n
    A pointer to the poly shape to add.\n\n
    <b>DWORD flags = 0</b>\n\n
    The flags can have one or more of the following values:\n\n
    <b>kGizmoDontDisplay;</b>\n\n
    Tells the gizmo not to display. It will still hit test it.\n\n
    <b>kGizmoDontHitTest;</b>\n\n
    Tells the gizmo not to hit test. It will still display.\n\n
    <b>kGizmoScaleToViewport;</b>\n\n
    Tells the gizmo to scale itself to have a constant size in the viewport. In
    this case, the system uses the <b>ManipulatorGizmo::mGizmoSize</b> to
    determine how big the manipulator should be. It interprets
    <b>mGizmoSize</b> as pixels it this case. This flag only applies to mesh
    gizmo currently.\n\n
    <b>Point3\& unselColor = GetUIColor(COLOR_SEL_GIZMOS)</b>\n\n
    The color of the gizmo when unselected.\n\n
    <b>Point3\& selColor = GetSubSelColor()</b>\n\n
    The color of the gizmo when selected. */
    ManipExport void AppendPolyShape(PolyShape* pPolyShape, DWORD flags,
                                     Point3& unselColor,
                                     Point3& selColor =  ColorMan()->GetColorAsPoint3(kManipulatorsSelected));
    /*! \remarks Implemented by the system.\n\n
    This method adds a new GIzmoShape to the manipulator. The shape is defined
    in the local coordinates of the node that owns the manipulator.
    \par Parameters:
    <b>GizmoShape* pGizmoShape</b>\n\n
    A pointer to the gizmo shape to add.\n\n
    <b>DWORD flags = 0</b>\n\n
    The flags can have one or more of the following values:\n\n
    <b>kGizmoDontDisplay;</b>\n\n
    Tells the gizmo not to display. It will still hit test it.\n\n
    <b>kGizmoDontHitTest;</b>\n\n
    Tells the gizmo not to hit test. It will still display.\n\n
    <b>kGizmoScaleToViewport;</b>\n\n
    Tells the gizmo to scale itself to have a constant size in the viewport. In
    this case, the system uses the <b>ManipulatorGizmo::mGizmoSize</b> to
    determine how big the manipulator should be. It interprets
    <b>mGizmoSize</b> as pixels it this case. This flag only applies to mesh
    gizmo currently.\n\n
    <b>Point3\& unselColor = GetUIColor(COLOR_SEL_GIZMOS)</b>\n\n
    The color of the gizmo when unselected.\n\n
    <b>Point3\& selColor = GetSubSelColor()</b>\n\n
    The color of the gizmo when selected. */
    ManipExport void AppendGizmo(GizmoShape* pGizmoShape, DWORD flags,
                                 Point3& unselColor,
                                 Point3& selColor =  ColorMan()->GetColorAsPoint3(kManipulatorsSelected));
    /*! \remarks Implemented by the system.\n\n
    This method adds a new Mesh to the manipulator. The shape is defined in the
    local coordinates of the node that owns the manipulator.
    \par Parameters:
    <b>Mesh* pMesh</b>\n\n
    A pointer to the mesh to add.\n\n
    <b>DWORD flags = 0</b>\n\n
    The flags can have one or more of the following values:\n\n
    <b>kGizmoDontDisplay;</b>\n\n
    Tells the gizmo not to display. It will still hit test it.\n\n
    <b>kGizmoDontHitTest;</b>\n\n
    Tells the gizmo not to hit test. It will still display.\n\n
    <b>kGizmoScaleToViewport;</b>\n\n
    Tells the gizmo to scale itself to have a constant size in the viewport. In
    this case, the system uses the <b>ManipulatorGizmo::mGizmoSize</b> to
    determine how big the manipulator should be. It interprets
    <b>mGizmoSize</b> as pixels it this case. This flag only applies to mesh
    gizmo currently.\n\n
    <b>Point3\& unselColor = GetUIColor(COLOR_SEL_GIZMOS)</b>\n\n
    The color of the gizmo when unselected.\n\n
    <b>Point3\& selColor = GetSubSelColor()</b>\n\n
    The color of the gizmo when selected. */
    ManipExport void AppendMesh(Mesh* pMesh, DWORD flags,
                                Point3& unselColor,
                                Point3& selColor =  ColorMan()->GetColorAsPoint3(kManipulatorsSelected));
    /*! \remarks Implemented by the system.\n\n
    This method adds a new Marker to the manipulator. The shape is defined in
    the local coordinates of the node that owns the manipulator.
    \par Parameters:
    <b>MarkerType markerType</b>\n\n
    The marker type for marker gizmos. See the
    <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_marker_types.html">List of Marker Types</a>\n\n
    <b>Point3\& position</b>\n\n
    The position of the marker.\n\n
    <b>DWORD flags = 0</b>\n\n
    The flags can have one or more of the following values:\n\n
    <b>kGizmoDontDisplay;</b>\n\n
    Tells the gizmo not to display. It will still hit test it.\n\n
    <b>kGizmoDontHitTest;</b>\n\n
    Tells the gizmo not to hit test. It will still display.\n\n
    <b>kGizmoScaleToViewport;</b>\n\n
    Tells the gizmo to scale itself to have a constant size in the viewport. In
    this case, the system uses the <b>ManipulatorGizmo::mGizmoSize</b> to
    determine how big the manipulator should be. It interprets
    <b>mGizmoSize</b> as pixels it this case. This flag only applies to mesh
    gizmo currently.\n\n
    <b>Point3\& unselColor = GetUIColor(COLOR_SEL_GIZMOS)</b>\n\n
    The color of the gizmo when unselected.\n\n
    <b>Point3\& selColor = GetSubSelColor()</b>\n\n
    The color of the gizmo when selected. */
    ManipExport void AppendMarker(MarkerType markerType, Point3& position,
                                  DWORD flags,
                                  Point3& unselColor,
                                  Point3& selColor =  ColorMan()->GetColorAsPoint3(kManipulatorsSelected));
    /*! \remarks Implemented by the system.\n\n
    This method adds a new Text to the manipulator. The shape is defined in the
    local coordinates of the node that owns the manipulator.
    \par Parameters:
    <b>MCHAR* pText</b>\n\n
    The text string to add.\n\n
    <b>Point3\& position</b>\n\n
    The position of the text.\n\n
    <b>DWORD flags = 0</b>\n\n
    The flags can have one or more of the following values:\n\n
    <b>kGizmoDontDisplay;</b>\n\n
    Tells the gizmo not to display. It will still hit test it.\n\n
    <b>kGizmoDontHitTest;</b>\n\n
    Tells the gizmo not to hit test. It will still display.\n\n
    <b>kGizmoScaleToViewport;</b>\n\n
    Tells the gizmo to scale itself to have a constant size in the viewport. In
    this case, the system uses the <b>ManipulatorGizmo::mGizmoSize</b> to
    determine how big the manipulator should be. It interprets
    <b>mGizmoSize</b> as pixels it this case. This flag only applies to mesh
    gizmo currently.\n\n
    <b>Point3\& unselColor = GetUIColor(COLOR_SEL_GIZMOS)</b>\n\n
    The color of the gizmo when unselected.\n\n
    <b>Point3\& selColor = GetSubSelColor()</b>\n\n
    The color of the gizmo when selected. */
    ManipExport void AppendText(MCHAR* pText, Point3& position,
                                DWORD flags,
                                Point3& unselColor,
                                Point3& selColor =  ColorMan()->GetColorAsPoint3(kManipulatorsSelected));

    /*! \remarks Implemented by the system.\n\n
    This method returns the manipulator name. */
    ManipExport MSTR& GetManipName() {return mToolTip; }

    /*! \remarks Implemented by the system.\n\n
    This method allows you to set the scale of the gizmo.
    \par Parameters:
    <b>float gizmoScale</b>\n\n
    The scale factor. */
    ManipExport void SetGizmoScale(float gizmoScale) { mGizmoScale = gizmoScale; }
    /*! \remarks Implemented by the system.\n\n
    This method returns the tooltip string. Used internally.
    \par Default Implementation:
    <b>{ return mToolTip; }</b> */
    ManipExport MSTR& GetToolTip() { return mToolTip; }
    /*! \remarks Implemented by the system.\n\n
    Used internally. */
    ManipExport void SetToolTipWnd(HWND hWnd) { mToolTipWnd = hWnd; }
    /*! \remarks Implemented by the system.\n\n
    Used internally.\n\n
      */
    ManipExport void SetToolTipTimer(UINT timer) { mToolTipTimer = timer; }
    /*! \remarks Implemented by the system.\n\n
    Used internally. */
    ManipExport UINT GetToolTipTimer() { return mToolTipTimer; }
    /*! \remarks Implemented by the system.\n\n
    Used internally. */
    ManipExport HWND GetToolTipWnd() { return mToolTipWnd; }

    /*! \remarks Implemented by the system.\n\n
    This method returns a pointer to the parameter block.
    \par Default Implementation:
    <b>{ return mpPblock; }</b>\n\n
    These must be implemented in the sub-class of SimpleManipulator */
    ManipExport IParamBlock2* GetPBlock() { return mpPblock; }

    // These must be implemented in the sub-class of SimpleManipulator

    // Called when the sub-class needs to update it's poly shapes
    // The toolTip string is used to signal
    /*! \remarks This method gets called whenever the manipulator needs to update its
    gizmos. This is implemented by the manipulator to create the gizmos based
    on the current state of the node being manipulated.
    \par Parameters:
    <b>TimeValue t</b>\n\n
    The time at which to update the shape.\n\n
    <b>MSTR\& toolTip</b>\n\n
    The tool tip text to update. */
    virtual void UpdateShapes(TimeValue t, MSTR& toolTip) = 0;

    /*! \remarks Implemented by the system.\n\n
    Used internally.
    \par Default Implementation:
    <b>{}</b> */
    ManipExport virtual void ManipulatorSelected() {};

    /*! \remarks Implemented by the system.\n\n
    This tells the SimpleManipulator to make a reference to hTarg. This is
    normally called in the contructor of a manipulator to set a reference to
    whatever is being manipulated.
    \par Parameters:
    <b>RefTargetHandle hTarg</b>\n\n
    The handle to the reference target. */
    ManipExport void SetManipTarget(RefTargetHandle hTarg);
    /*! \remarks This method returns the handle to the manipulator reference
    target.
    \par Default Implementation:
    <b>{ return mhTarget; }</b>\n\n
      */
    ManipExport RefTargetHandle GetManipTarget() { return mhTarget; }

    /*! \remarks This method sets the state of the mouse.
    \par Parameters:
    <b>MouseState state</b>\n\n
    One of the following values:\n\n
    <b>kMouseIdle</b>\n\n
    The mouse is idle, manipulator not active and the mouse is not over it.\n\n
    <b>kMouseDragging</b>\n\n
    The mouse is currently dragging the manipulator.\n\n
    <b>kMouseOverManip</b>\n\n
    The mouse is over the manipulator, but it is not being dragged.
    \par Default Implementation:
    <b>{ mState = state; }</b> */
    ManipExport void SetMouseState(MouseState state) { mState = state; }
    /*! \remarks Implemented by the system.\n\n
    This method returns the state of the mouse, which is one of the following
    values: <b>kMouseIdle, kMouseDragging, or kMouseOverManip</b>.
    \par Default Implementation:
    <b>{ return mState; }</b> */
    ManipExport MouseState GetMouseState() { return mState; }

    /*! \remarks Implemented by the system.\n\n
    This method gets called when the mouse buttons is pressed within the
    manipulator context. Used internally.
    \par Parameters:
    <b>TimeValue t</b>\n\n
    The time to display the object.\n\n
    <b>ViewExp* pVpt</b>\n\n
    An interface that may be used to call methods associated with the
    viewports.\n\n
    <b>IPoint2\& m</b>\n\n
    The location of the tooltip.\n\n
    <b>DWORD flags</b>\n\n
    Not used, should be set to 0.\n\n
    <b>ManipHitData* pHitData</b>\n\n
    A pointer to the hitdata containing information on which manipulator was
    hit. */
    ManipExport void OnButtonDown(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData);
    /*! \remarks This method gets called when the mouse is pressed down and
    moves within the manipulator context. It is the method that does the actual
    manipulator. It is up to the manipulator code to turn the mouse position
    into a new value for the parameter(s) being manipulated. It also updates
    the tooltip with the current value of the parameter.
    \par Parameters:
    <b>TimeValue t</b>\n\n
    The time to display the object.\n\n
    <b>ViewExp* pVpt</b>\n\n
    An interface that may be used to call methods associated with the
    viewports.\n\n
    <b>IPoint2\& m</b>\n\n
    The location of the tooltip.\n\n
    <b>DWORD flags</b>\n\n
    Not used, should be set to 0.\n\n
    <b>ManipHitData* pHitData</b>\n\n
    A pointer to the hitdata containing information on which manipulator was
    hit. */
    ManipExport void OnMouseMove(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData);
    /*! \remarks Implemented by the system.\n\n
    This method gets called when the mouse buttons is released within the
    manipulator context. Used internally.
    \par Parameters:
    <b>TimeValue t</b>\n\n
    The time to display the object.\n\n
    <b>ViewExp* pVpt</b>\n\n
    An interface that may be used to call methods associated with the
    viewports.\n\n
    <b>IPoint2\& m</b>\n\n
    The location of the tooltip.\n\n
    <b>DWORD flags</b>\n\n
    Not used, should be set to 0.\n\n
    <b>ManipHitData* pHitData</b>\n\n
    A pointer to the hitdata containing information on which manipulator was
    hit. */
    ManipExport void OnButtonUp(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData);

    /*! \remarks Implemented by the system.\n\n
    This method gets called when the mouse enters the manipulator object. Used
    interlally.
    \par Parameters:
    <b>TimeValue t</b>\n\n
    The time to display the object.\n\n
    <b>ViewExp* pVpt</b>\n\n
    An interface that may be used to call methods associated with the
    viewports.\n\n
    <b>IPoint2\& m</b>\n\n
    The location of the tooltip.\n\n
    <b>ManipHitData* pHitData</b>\n\n
    A pointer to the hitdata containing information on which manipulator was
    hit.
    \return  The display state indicating whether no redraw, a full redraw, or
    a post redraw is needed. */
    ManipExport DisplayState  MouseEntersObject(TimeValue t, ViewExp* pVpt, IPoint2& m, ManipHitData* pHitData);
    /*! \remarks Implemented by the system.\n\n
    This method gets called when the mouse leaves the manipulator object. Used
    internally.
    \par Parameters:
    <b>TimeValue t</b>\n\n
    The time to display the object.\n\n
    <b>ViewExp* pVpt</b>\n\n
    An interface that may be used to call methods associated with the
    viewports.\n\n
    <b>IPoint2\& m</b>\n\n
    The location of the tooltip.\n\n
    <b>ManipHitData* pHitData</b>\n\n
    A pointer to the hitdata containing information on which manipulator was
    hit.
    \return  The display state indicating whether no redraw, a full redraw, or
    a post redraw is needed. */
    ManipExport DisplayState  MouseLeavesObject(TimeValue t, ViewExp* pVpt, IPoint2& m, ManipHitData* pHitData);

    /*! \remarks Implemented by the system.\n\n
    This method returns the position of the tooltip. Used internally.
    \par Default Implementation:
    <b>{ return mToolTipPos; }</b> */
    ManipExport IPoint2& GetTipPos() { return mToolTipPos; }

    // Get the view ray going through the given screen coordinate.
    // result is in local coordinates of the owning INode.
    /*! \remarks This method is normally called from a manipualtor's
    OnMouseMove method. It computes a ray that passes through the given mouse
    point in the given viewport. The result is in the local coordinates of the
    node owning the manipulator.
    \par Parameters:
    <b>ViewExp* pVpt</b>\n\n
    An interface that may be used to call methods associated with the
    viewports.\n\n
    <b>IPoint2\& m</b>\n\n
    The screen coordinate.\n\n
    <b>Ray\& viewRay</b>\n\n
    The returned local view ray. */
    ManipExport void GetLocalViewRay(ViewExp* pVpt, IPoint2& m, Ray& viewRay);

    /*! \remarks This method invalidates the validity interval.
    \par Default Implementation:
    <b>{ mValid = NEVER; }</b> */
    ManipExport void Invalidate() { mValid = NEVER; }

    // From Object
    BOOL UseSelectionBrackets() { return FALSE; }
	 /*! \remarks This method unregisters the notifications so changes in the view
	 are no longer registered. */
    ManipExport void UnRegisterViewChange(BOOL fromDelete = FALSE);
    void RegisterViewChange();
    /*! \remarks Used internally. */
    void SetResettingFlag(BOOL val) { mResetting = val; }
    /*! \remarks Used internally. */
    BOOL GetResettingFlag() { return mResetting; }
    /*! \remarks This method will destroy the tooltip and its timer and cleans
    up. */
    ManipExport void KillToolTip();

    /*! \remarks This method returns the color of the gizmo when unselected.
    */
    ManipExport Point3 GetUnselectedColor();
    ManipExport BOOL ActiveViewOnly() { return mActiveViewOnly; }

protected:
    // Index of manip that mouse is over, for display
    int     mDispSelectedIndex; 
    MSTR    mToolTip;       // text and location for tooltip
    float   mGizmoScale;
    IParamBlock2 *mpPblock;
    Interval   mValid; // Validity of reference
    RefTargetHandle mhTarget;  // The object/modifier/controller being manipulated

    MouseState mState;

    BOOL mActiveViewOnly;
    BOOL mResetting;

private:

    void StartToolTipTimer(HWND hWnd, IPoint2& m);


    Tab<ManipulatorGizmo*>   mGizmos;

    // Tooltip management
    HWND     mToolTipWnd;
    HWND     mToolTipParent;
    UINT     mToolTipTimer;
    IPoint2  mToolTipPos;

    bool mNotificationsRegistered;
};
#pragma warning(pop) // 4239
// Stock gizmo objects

/*! \remarks This global function is available in release 4.0 and later
only.\n\n
Creates a new mesh sphere at the given center position with the specified
radius and segments count.
\par Parameters:
<b>Point3\& pos</b>\n\n
The center point for the sphere in object space.\n\n
<b>float radius</b>\n\n
The radius for the sphere.\n\n
<b>int segments</b>\n\n
The number of segments for the sphere mesh.
\return  A pointer to the sphere mesh. Developers are responsible for deleting
this mesh when done. */
ManipExport Mesh* MakeSphere(Point3& pos, float radius, int segments);
/*! \remarks This global function is available in release 4.0 and later
only.\n\n
Create a mesh torus with the given center point, outer radius, inner radius,
and segment counts along the two circular dimensions of the torus.
\par Parameters:
<b>Point3\& pos</b>\n\n
The center point in object space.\n\n
<b>float radius</b>\n\n
The first radius.\n\n
<b>float radius2</b>\n\n
The second radius.\n\n
<b>int segs</b>\n\n
The segment count along the vertical circular dimension.\n\n
<b>int sides</b>\n\n
The segment count along the horizontal circular dimension.
\return  A pointer to the torus mesh created. Developers are responsible for
deleting this mesh when done with it. */
ManipExport Mesh* MakeTorus(Point3& pos, float radius, float radius2, int segs, int sides);
/*! \remarks This global function is available in release 4.0 and later
only.\n\n
Creates a mesh box with the given center point, length, width and height as
well as segment parameters.
\par Parameters:
<b>Point3\& pos</b>\n\n
The box is built from this position in size along +X, +Y and +Z. This
coordinate is in object space.\n\n
<b>float l</b>\n\n
The length of the box.\n\n
<b>float w</b>\n\n
The width of the box.\n\n
<b>float h</b>\n\n
The height of the box.\n\n
<b>int lsegs</b>\n\n
The number of segments in the length dimension.\n\n
<b>int wsegs</b>\n\n
The number of segments in the width dimension.\n\n
<b>int hsegs</b>\n\n
The number of segments in the height dimension.
\return  A pointer to the box mesh. Developers are responsible for deleting
this mesh when done. */
ManipExport Mesh* MakeBox(Point3& pos, float l, float w, float h, int lsegs, int wsegs, int hsegs);
/*! \remarks This global function is available in release 4.0 and later
only.\n\n
Creates a new series of lines in the form of a cube and adds it to the
specified PolyShape with the given position and side length size.
\par Parameters:
<b>PolyShape\& shape</b>\n\n
The PolyShape to add the box shape to.\n\n
<b>Point3\& pos</b>\n\n
The position for the center of the cube shape in object space.\n\n
<b>float size</b>\n\n
The size of one side of the cube in object space.
\par Data Members:
protected:\n\n
<b>int mDispSelectedIndex;</b>\n\n
The Index of manipulator that the mouse is over, for display.\n\n
<b>MSTR mToolTip;</b>\n\n
The tooltip text.\n\n
<b>float mGizmoScale;</b>\n\n
The scaling factor of the gizmo.\n\n
<b>IParamBlock2 *mpPblock;</b>\n\n
The Parameter Block 2 for the manipulator.\n\n
<b>RefTargetHandle mhTarget;</b>\n\n
The handle to the manipulator reference target.\n\n
<b>MouseState mState;</b>\n\n
The state of the mouse, which is one of the following:\n\n
<b>kMouseIdle</b>\n\n
The mouse is idle, manipulator not active and the mouse is not over it.\n\n
<b>kMouseDragging</b>\n\n
The mouse is currently dragging the manipulator.\n\n
<b>kMouseOverManip</b>\n\n
The mouse is over the manipulator, but it is not being dragged.\n\n
<b>BOOL mActiveViewOnly;</b>\n\n
This flag defines whether the manipulator is shown in the active viewport
only.\n\n
<b>Interval mValid;</b>\n\n
The validity interval of the reference.  */
ManipExport void AddCubeShape(PolyShape& shape, Point3& pos, float size);


// Special storage class for hit records so we can know which manipulator was hit
/*! \sa  Class HitData, Class Manipulator\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This is a special storage class for hit records that keep track of which
manipulator was hit.\n\n

\par Data Members:
<b>Manipulator* mpManip;</b>\n\n
A pointer to the manipulator associated with the hit data.\n\n
<b>int mShapeIndex;</b>\n\n
The index of the selected manipulator.  */
class ManipHitData : public HitData 
{
public:
    Manipulator* mpManip;
    int mShapeIndex;

    /*! \remarks Constructor.
    \par Parameters:
    <b>Manipulator* pManip</b>\n\n
    A pointer to the manipulator to use. */
    ManipExport ManipHitData(Manipulator* pManip) {
        mpManip = pManip;
        mShapeIndex = -1;
    }

    /*! \remarks Constructor. */
    ManipExport ManipHitData() {
        mpManip = NULL;
    }

    /*! \remarks This method allows you to copy the hit data.
    \par Default Implementation:
    <b>{ return new ManipHitData(mpManip); }</b> */
    ManipExport virtual ManipHitData* Copy();

    /*! \remarks Destructor. */
    ManipExport ~ManipHitData() {}
};

// Special storage class for hit records so we can know which manipulator was hit
/*! \sa  Class ManipHitData, Class Manipulator\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This is a special storage class for hit records that keep track of which
manipulator was hit and is provided as a simple class for developers to
sub-class from.  */
class SimpleManipHitData : public ManipHitData 
{
public:

    /*! \remarks Constructor.
    \par Parameters:
    <b>int shapeIndex</b>\n\n
    This sets the shape index.\n\n
    <b>Manipulator* pManip</b>\n\n
    A pointer to the manipulator. */
    ManipExport SimpleManipHitData(int shapeIndex, Manipulator* pManip) {
        mpManip = pManip;
        mShapeIndex = shapeIndex;
    }

    /*! \remarks Constructor. */
    ManipExport SimpleManipHitData() {
        mShapeIndex = -1;
        mpManip = NULL;
    }
    /*! \remarks Destructor. */
    ManipExport ~SimpleManipHitData() {}

    /*! \remarks This method allows you to copy the hit data.
    \par Default Implementation:
    <b>{ return new ManipHitData(mpManip); }</b> */
    ManipExport virtual ManipHitData* Copy();
};

