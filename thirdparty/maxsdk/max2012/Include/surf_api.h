/**********************************************************************
 *<
    FILE: surf_api.h

    DESCRIPTION:  Provides the SDK api for NURBS surfaces

    CREATED BY: Charlie Thaeler

    HISTORY: created 15 April, 1997

 *> Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "export.h"
#include "maxheap.h"
#include "point3.h"
#include "maxtess.h"
#include "tab.h"
#include "strbasic.h"
#include "point2.h"
#include "matrix3.h"
// forward declarations
class Object;
class SetXFormPacket;
class IObjParam;
class ShapeObject;
class BitArray;
class Modifier;

#define EDITABLE_SURF_CLASS_ID Class_ID(0x76a11646, 0x12a822fb)
#define FITPOINT_PLANE_CLASS_ID Class_ID(0x76a11646, 0xbadbeff)
#define EDITABLE_CVCURVE_CLASS_ID Class_ID(0x76a11646, 0x12a83145)
#define EDITABLE_FPCURVE_CLASS_ID Class_ID(0x76a11646, 0x12a92143)

#define NURBS_NAME_SIZE 80

// NOTE: THESE ARE NOT PERSISTANT ACROSS SESSIONS and should NOT be saved to a file.
typedef ULONG_PTR NURBSId;  
typedef Tab<NURBSId> NURBSIdTab;
typedef Tab<BOOL> BoolTab;

/*! \defgroup nurbResults NURBS Results
These results are returned when one is modifiying existing objects.
\sa <a href="ms-its:3dsmaxsdk.chm::/NURBS_root.html">Working With NURBS</a>. */
//@{
enum NURBSResult {
	/*! The function succeeded. */
	kNOk,
	/*! The specified object was invalid. For example, if you use the
	SetSurfaceApprox() function and specify a non-NURBS	object as input, you get this error. */
	kNInvalidObject,
	/*! The specified ID was invalid. For example, if you use the Transform() function and pass an invalid NURBSId,
	you get this error. */
	kNInvalidId,
	/*! A specified parameter was invalid. For example, if you try to create a cone using GenNURBSConeSurface() with a
	radius <=0, you get this error. */
	kNInvalidParameter,	//!< 
	/*! The function failed. This is a catch-all for any kind of unwanted result not covered above. */
	kNBad
};
//@}

/*! \defgroup nurbMirrorAxisTypes NURBSMirrorAxis Types
One of the following enum values describe the axis of reflection for a mirror curve or surface.
\sa Class NURBSMirrorCurve, Class NURBSMirrorSurface. */
//@{
enum NURBSMirrorAxis {
	kMirrorX,
	kMirrorY,
	kMirrorZ,
	kMirrorXY,
	kMirrorXZ,
	kMirrorYZ
};
//@}

/*! \defgroup nurbConstTypes NURBSConst Types
One of the following enum values describes the type of NURBSConst this is. For example, these correspond to the types of dependent points available in the NURBS Surface user interface in the 'Surface Point' rollup.\n\n
Constrained objects are those that exist in the parameter space of other objects.
\sa Class NURBSPointConstPoint, Class NURBSCurveConstPoint, Class NURBSSurfConstPoint.
*/
//@{
enum NURBSConstType {
	/*! Indicates the point is actually on the surface of the object. */
	kNConstOnObject,
	/*! Indicates the points is offset some distance (specified in object space) from
	the surface of the object. */
	kNConstOffset,
	/*! Indicates the point is offset some distance along the normal to the curve or
	surface. */
	kNConstNormal,
	/*! Indicates the point is offset some U and/or V distance along the tangent from
	the curve or surface. If the value is positive, it's the tangent that heads in the
	direction of increasing parameter value; if negative, it's the tangent that heads in
	the direction of decreasing parameter value. */
	kNConstTangent
};
//@}
/*! \defgroup nurbObjectKinds NURBSObject Kinds
One of the following enum values describes the kind of NURBSObject this is (sub-classed from
which base class). The sub-classed NURBSObject returns one of these values in
NURBSObject::GetKind().
\sa Class NURBSObject.
*/
//@{
enum NURBSKind {
	/*! Specifies a NURBSPoint object. */
	kNURBSPoint,
	/*! Specifies a NURBSTexturePoint object. */
	kNURBSTexturePoint,
	/*! Specifies a NURBSControlVertex object. */
	kNURBSCV,
	/*! Specifies a NURBSCurve object. */
	kNURBSCurve,
	/*! Specifies a NURBSSurface object. */
	kNURBSSurface
};
//@}

/*! \defgroup nurbParameterizationTypes NURBSParameterization Types
One of the following enum values describes the types or reparameterization.
\sa Class NURBSCVCurve, Class NURBSCVSurface. */
//@{
enum NURBSParamaterization {
	/*! Chooses the chord-length algorithm for reparameterization. Chord-length
	reparameterization spaces knots (in parameter space) based on the square root of the
	length of each curve segment. */
    kCentripetalKnots,
    /*! Spaces the knots uniformly. A uniform knot vector has the advantage that the curve or
    surface changes only locally when you edit it. With chord-length parameterization, moving
    any CV can potentially change the entire sub-object. */
	kUniformKnots,
};
//@}

/*! \defgroup nurbAutoParamTypes NURBSAutoParam Types
These values describe how automatic reparameterization is handled.
\sa Class NURBSCVCurve, Class NURBSCVSurface */
//@{
enum NURBSAutoParam {
	/*! Specifies that the reparameterization is not automatic. When it is automatic,
	curves are reparameterized as they are editied. */
	kNotAutomatic,
	/*! Chooses the chord-length algorithm for reparameterization. Chord-length
	reparameterization spaces knots (in parameter space) based on the square root of
	the length of each curve segment. */
    kAutoCentripetal,
    /*! Spaces the knots uniformly. A uniform knot vector has the advantage that the
    curve or surface changes only locally when you edit it. With chord-length
    parameterization, moving any CV can potentially change the entire sub-object. */
	kAutoUniform,
};
//@}

/*! \defgroup nurbObjectTypes NURBSObject Types
One of the following enum values describes the type of NURBSObject this is. The sub-classed
NURBSObject returns one of these values in NURBSObject::GetType():
/sa Class NURBSObject, Class NURBSSurface, Class NURBSControlVertex, Class NURBSPoint,
Class NURBSCurve. */
//@{
enum NURBSType {
	/*! Specifies a NURBSIndependentPoint object. */
	kNPoint,
	/*! Specifies a NURBSPointConstPoint object (constrained points). */
	kNPointCPoint,
	/*! Specifies a NURBSCurveConstPoint object. */
	kNCurveCPoint,
	/*! Specifies a NURBSCurveCurveIntersectionPoint object. */
	kNCurveCurveIntersectionPoint,
	/*! Specifies a NURBSSurfConstPoint object. */
	kNSurfaceCPoint,
	/*! Specifies a NURBSCurveSurfaceIntersectionPoint object. */
	kNCurveSurfaceIntersectionPoint,
	/*! Specifies a NURBSTexturePoint object. */
	kNTexturePoint,
	/*! Specifies a NURBSControlVertex object. */
	kNCV,
	/*! Specifies a NURBSCVCurve object. */
	kNCVCurve,
	/*! Specifies a NURBSPointCurve object. */
	kNPointCurve,
	/*! Specifies a NURBSBlendCurve object. */
	kNBlendCurve,
	/*! Specifies a NURBSOffsetCurve object. */
	kNOffsetCurve,
	/*! Specifies a NURBSXFormCurve object. */
	kNXFormCurve,
	/*! Specifies a NURBSMirrorCurve object. */
	kNMirrorCurve,
	/*! Specifies a NURBSFilletCurve object. */
	kNFilletCurve,
	/*! Specifies a NURBSChamferCurve object. */
	kNChamferCurve,
	/*! Specifies a NURBSIsoCurve object. */
	kNIsoCurve,
	/*! Specifies a NURBSProjectVectorCurve object. */
	kNProjectVectorCurve,
	/*! Specifies a NURBSProjectNormalCurve object. */
	kNProjectNormalCurve,
	/*! Specifies a NURBSSurfSurfIntersectionCurve object. */
	kNSurfSurfIntersectionCurve,
	/*! Specifies a NURBSCurveOnSurface object. */
	kNCurveOnSurface,
	/*! Specifies a NURBSPointCurveOnSurface object. */
	kNPointCurveOnSurface,
	/*! Specifies a NURBSSurfaceNormalCurve object. */
	kNSurfaceNormalCurve,
	/*! Specifies a NURBSSurfaceEdgeCurve object. */
	kNSurfaceEdgeCurve,
	/*! Specifies a NURBSCVSurface object. */
	kNCVSurface,
	/*! Specifies a NURBSPointSurface object. */
	kNPointSurface,
	/*! Specifies a NURBSBlendSurface object. */
	kNBlendSurface,
	/*! Specifies a NURBSOffsetSurface object. */
	kNOffsetSurface,
	/*! Specifies a NURBSXFormSurface object. */
	kNXFormSurface,
	/*! Specifies a NURBSMirrorSurface object. */
	kNMirrorSurface,
	/*! Specifies a NURBSRuledSurface object. */
	kNRuledSurface,
	/*! Specifies a NURBSULoftSurface object. */
	kNULoftSurface,
	/*! Specifies a NURBSExtrudeSurface object. */
	kNExtrudeSurface,
	/*! Specifies a NURBSLatheSurface object. */
	kNLatheSurface,
	/*! Specifies a NURBSUVLoftSurface object. */
	kNUVLoftSurface,
	/*! Specifies a NURBSNBlendSurface object. */
	kNNBlendSurface,
	/*! Specifies a NURBS1RailSweepSurface object. */
	kN1RailSweepSurface,
	/*! Specifies a NURBS2RailSweepSurface object. */
	kN2RailSweepSurface,
	/*! Specifies a NURBSCapSurface object. */
	kNCapSurface,
	/*! Specifies a NURBSMultiCurveTrimSurface object. */
	kNMultiCurveTrimSurface,
	/*! Specifies a NURBSFilletSurface object. */
	kNFilletSurface
};
//@}

/*! \defgroup nurbTessTypes NURBSTess Types
One of the following enum values determines the tesselation type.
\sa Class NURBSSurface, Class TessApprox, Class NURBSSet. */
//@{
enum NURBSTessType {
	/*! Surface tesselation. */
	kNTessSurface,
	/*! Displacement tesselation. */
	kNTessDisplacement,
	/*! Curve tesselation. */
	kNTessCurve
};
//@}

/*! \defgroup nurbSubObjectLevelOptions NURBSSubObjectLevel Options
One of the following enum values describes the sub-object level of the object.
\sa Class NURBSSurface, Class NURBSSet. */
//@{
enum NURBSSubObjectLevel {
	/*! The object level. */
    kNTopLevel = 0,
    /*! The surface CV sub-object level. */
    kNSurfaceCVLevel,
    /*! The surface level. */
    kNSurfaceLevel,
    /*! The curve CV sub-object level. */
    kNCurveCVLevel,
    /*! The point sub-object level. */
    kNPointLevel,
    /*! The curve sub-object level. */
    kNCurveLevel,
    /*! The imports level. */
    kNImportsLevel,
};
//@}


class NURBSSet;

typedef Tab<NURBSId> NURBSIdTab;

extern int FindIndex(NURBSIdTab ids, NURBSId id);

/*! \sa  Class NURBSSurface, Class NURBSControlVertex, Class NURBSPoint, Class NURBSCurve, Class NURBSSet, \ref nurbObjectTypes.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This is the base class for many of the other classes in the NURBS API. It
provides a common set of methods that each of them use. It has methods to get
and set the name of the item, and methods to deal with error processing. To
determine the type of object the derived class is use the method
<b>GetType()</b>.\n\n
All methods of this class are implemented by the system.
\par Data Members:
protected:\n\n
<b>MCHAR mName[NURBS_NAME_SIZE];</b>\n\n
The name of the NURBS object. The maximum length of this name including the
terminating NULL is specified using the following #define: <b>#define
NURBS_NAME_SIZE 80</b>\n\n
<b>NURBSType mType;</b>\n\n
The type of NURBS object this is. See \ref nurbObjectTypes.\n\n
<b>NURBSKind mKind;</b>\n\n
The kind of NURBS object this is. See \ref nurbObjectKinds.\n\n
<b>NURBSId mId;</b>\n\n
This is the ID of the NURBS object used to specify the parent object in many of
the dependent point, curves and surface classes. This ID is not persistant
across sessions and should not be saved to a file. A NURBSId is defined as
follows: <b>typedef unsigned long NURBSId;</b>\n\n
<b>Object *mpObject;</b>\n\n
When an object is instantiated in the 3ds Max scene this pointer is filled in.
For example, if you use the function <b>CreateNURBSObject()</b> and pass a
<b>NURBSSet</b>, this data member is filled in to point to the actual editable
NURBS object in 3ds Max that this <b>NURBSObject</b> is a part of.\n\n
<b>NURBSSet* mpNSet;</b>\n\n
When an object is instantiated in the 3ds Max scene this pointer is filled in.
This points to the <b>NURBSSet</b> this object is a contained within.\n\n
<b>BOOL mSelected;</b>\n\n
TRUE if the object is selected; otherwise FALSE. <br>  protected: */
class NURBSObject: public MaxHeapOperators {
	friend class NURBSSet;
protected:
	MCHAR mName[NURBS_NAME_SIZE];
	NURBSType mType;
	NURBSKind mKind;
	NURBSId mId;
	Object* mpObject;
	NURBSSet* mpNSet;
	BOOL mSelected;
	/*! \remarks	Sets the <b>mId</b> to 0 and <b>mpObject</b> pointer to NULL. This
	effectively breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	This parameter is not used.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mName[0] = '/0';</b>\n\n
	<b> mId = 0;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mSelected = FALSE;</b> */
	DllExport NURBSObject(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSObject(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSObject\& pt</b>\n\n
	The NURBSObject to assign. */
	DllExport NURBSObject & operator=(const NURBSObject& pt);
	/*! \remarks Sets the name of the item to the specified string.
	\par Parameters:
	<b>MCHAR *name</b>\n\n
	The name to set. */
	DllExport void SetName(MCHAR *name);
	/*! \remarks Returns a pointer to the name of the item. */
	DllExport MCHAR *GetName(void);
	/*! \remarks Returns the specific type of object this is. See \ref nurbObjectTypes */
	DllExport NURBSType GetType();
	/*! \remarks Returns the specific kind of object this is. See \ref nurbObjectKinds. */
	DllExport NURBSKind GetKind();
	/*! \remarks Returns the <b>NURBSId</b> of this <b>NURBSObject</b>. This
	ID is not persistant across sessions and should not be saved to a file. */
	DllExport NURBSId GetId();
	/*! \remarks Sets the <b>NURBSId</b> of this <b>NURBSObject</b>.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	The ID to set. */
	DllExport void SetId(NURBSId id);
	/*! \remarks Sets the pointer to the <b>NURBSSet</b> maintained by the
	object.
	\par Parameters:
	<b>NURBSSet *nset</b>\n\n
	The pointer to set. */
	DllExport void SetNSet(NURBSSet *nset);
	/*! \remarks Sets the pointer to the 3ds Max editable NURBS object
	maintained by this object.
	\par Parameters:
	<b>Object *object</b>\n\n
	The pointer to set. */
	DllExport void SetObject(Object *object);
	/*! \remarks Returns a pointer to the <b>Object</b> maintained by this
	class. */
	DllExport Object* GetMAXObject();
	/*! \remarks Returns a pointer to the <b>NURBSSet</b> maintained by this
	class. */
	DllExport NURBSSet* GetNSet();
	/*! \remarks Returns the index in the <b>NURBSSet</b> of this object or -1
	if there isn't an associated <b>NURBSSet</b>. */
	DllExport int GetIndex();
	/*! \remarks Returns TRUE if the object is selected; otherwise FALSE. */
	DllExport BOOL IsSelected();
	/*! \remarks Sets the object to selected or not.
	\par Parameters:
	<b>BOOL set</b>\n\n
	TRUE to select the object; FALSE to de-select it.
	\par Operators:
	*/
	DllExport void SetSelected(BOOL set);
};


/*! \sa  Class NURBSObject, Class NURBSSurface, Class NURBSSet, Class NURBSControlVertex, Class NURBSCurve, Class NURBSFuseSurfaceCV, Class NURBSFuseCurveCV.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class describes a point in 3 space using double precision X, Y and Z
coordinates. Methods are available for getting the point coordinates a floats,
doubles or a <b>Point3</b>.\n\n
All methods of this class are implemented by the system.
\par Data Members:
protected:\n\n
<b>double mX, mY, mZ;</b>\n\n
The X, Y and Z coordinates for the point.  */
class NURBSPoint : public NURBSObject {
protected:
	double mX, mY, mZ;
public:
	DllExport NURBSPoint();
	/*! \remarks Returns the point position as a <b>Point3</b>. */
	DllExport virtual Point3 GetPosition(TimeValue t);
	/*! \remarks Retrieves the point position using single precision.
	\par Parameters:
	<b>float\& x</b>\n\n
	The X coordinate is returned here.\n\n
	<b>float\& y</b>\n\n
	The Ycoordinate is returned here.\n\n
	<b>float\& z</b>\n\n
	The Z coordinate is returned here. */
	DllExport virtual void GetPosition(TimeValue t, float& x, float& y, float& z);
	/*! \remarks Retrieves the point position using double precision.
	\par Parameters:
	<b>double\& x</b>\n\n
	The X coordinate is returned here.\n\n
	<b>double\& y</b>\n\n
	The Ycoordinate is returned here.\n\n
	<b>double\& z</b>\n\n
	The Z coordinate is returned here. */
	DllExport virtual void GetPosition(TimeValue t, double& x, double& y, double& z);
};

/*! \sa  Class NURBSTextureSurface,  Class NURBSObject,  Class Point2.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class holds a single texture vertex in a NURBS texture surface. Methods
are available to get and set the point in various formats.
\par Data Members:
protected:\n\n
<b>double mX, mY;</b>\n\n
The position of the point.  */
class NURBSTexturePoint : public NURBSObject {
protected:
	double mX, mY;
    int    mUIndex, mVIndex;
public:
	/*! \remarks Constructor. The data members (from <b>NURBSObject</b>) are
	initialized as follows:\n\n
	<b>mKind = kNURBSTexturePoint;</b>\n\n
	<b>mType = kNTexturePoint;</b> */
	DllExport NURBSTexturePoint();
	/*! \remarks Returns the position of the texture point at the specified
	time (as a Point2).
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the position. */
	DllExport virtual Point2 GetPosition(TimeValue t);
	/*! \remarks Retrieves the position of the texture point at the specified
	time (as floats).
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the position.\n\n
	<b>float\& x</b>\n\n
	The x position is returned here.\n\n
	<b>float\& y</b>\n\n
	The y position is returned here. */
	DllExport virtual void GetPosition(TimeValue t, float& x, float& y);
	/*! \remarks Retrieves the position of the texture point at the specified
	time (as doubles).
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the position.\n\n
	<b>double\& x</b>\n\n
	The x position is returned here.\n\n
	<b>double\& y</b>\n\n
	The y position is returned here. */
	DllExport virtual void GetPosition(TimeValue t, double& x, double& y);
	/*! \remarks Sets the position of the point at the specified time (using a
	Point2).
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the position.\n\n
	<b>Point2 pt</b>\n\n
	The position to set. */
	DllExport void SetPosition(TimeValue t, Point2 pt);
	/*! \remarks Sets the position of the point at the specified time (using
	floats).
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the position.\n\n
	<b>float x</b>\n\n
	The x coordinate to set.\n\n
	<b>float y</b>\n\n
	The y coordinate to set. */
	DllExport void SetPosition(TimeValue t, float x, float y);
	/*! \remarks Sets the position of the point at the specified time (using
	doubles).
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the position.\n\n
	<b>double x</b>\n\n
	The x coordinate to set.\n\n
	<b>double y</b>\n\n
	The y coordinate to set. */
	DllExport void SetPosition(TimeValue t, double x, double y);

    DllExport void SetIndices(int uIndex, int vIndex);
};


/*! \sa  Class NURBSPoint.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class is used to create an independent, free-standing point. There are
methods to set the position of the point in various floating point formats and
operators to compare points.\n\n
All methods of this class are implemented by the system.  */
class NURBSIndependentPoint : public NURBSPoint {
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNPoint;</b>\n\n
	<b> mX = mY = mZ = 0.0;</b> */
	DllExport NURBSIndependentPoint(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSIndependentPoint(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSIndependentPoint\& pt</b>\n\n
	The point to assign. */
	DllExport NURBSIndependentPoint & operator=(const NURBSIndependentPoint& pt);
	/*! \remarks Equality operator. Compares if the X, Y and Z coordinates are
	equal.
	\par Parameters:
	<b>const NURBSIndependentPoint\& pt</b>\n\n
	The point to compare.
	\return  TRUE if equal; otherwise FALSE. */
	DllExport BOOL operator==(const NURBSIndependentPoint& pt);
	/*! \remarks Inequality operator. Compares if the X, Y and Z coordinates
	are <b>not</b> equal.
	\par Parameters:
	<b>const NURBSIndependentPoint\& pt</b>\n\n
	The point to compare.
	\return  TRUE if any of the coordinates are <b>not</b> equal; otherwise
	FALSE. */
	DllExport BOOL operator!=(const NURBSIndependentPoint& pt);
	/*! \remarks Sets the position of the point at the specified time (as a
	Point3).
	\par Parameters:
	<b>TimeValue t</b>\n\n
	Specifies the time to set the position.\n\n
	<b>Point3 pt</b>\n\n
	The position to set. */
	DllExport void SetPosition(TimeValue t, Point3 pt);
	/*! \remarks Sets the position of the point at the specified time (as
	floats).
	\par Parameters:
	<b>TimeValue t</b>\n\n
	Specifies the time to set the position.\n\n
	<b>float x</b>\n\n
	The X position to set.\n\n
	<b>float y</b>\n\n
	The Y position to set.\n\n
	<b>float z</b>\n\n
	The Z position to set. */
	DllExport void SetPosition(TimeValue t, float x, float y, float z);
	/*! \remarks Sets the position of the point at the specified time (as
	doubles).
	\par Parameters:
	<b>TimeValue t</b>\n\n
	Specifies the time to set the position.\n\n
	<b>double x</b>\n\n
	The X position to set.\n\n
	<b>double y</b>\n\n
	The Y position to set.\n\n
	<b>double z</b>\n\n
	The Z position to set.
	\par Operators:
	*/
	DllExport void SetPosition(TimeValue t, double x, double y, double z);

};


/*! \sa  Class NURBSObject, Class NURBSSurface, Class NURBSSet, Class NURBSPoint, Class NURBSCurve, Class NURBSFuseSurfaceCV, Class NURBSFuseCurveCV.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class represents a control vertex in a NURBS curve. Methods are available
to get and set the point position, and get/set the weight.\n\n
All methods of this class are implemented by the system.  */
class NURBSControlVertex : public NURBSObject {
	double mX, mY, mZ;
	double mW;  // weight
public:
	/*! \remarks Constructor. The name is set to NULL, the points are set to
	0.0f and the weight is set to 1.0. */
	DllExport NURBSControlVertex(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSControlVertex(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSControlVertex\& cv</b>\n\n
	The control vertex to assign. */
	DllExport NURBSControlVertex & operator=(const NURBSControlVertex& cv);
	/*! \remarks Equality operator. Compares if the coordinates and weight are
	the same.
	\par Parameters:
	<b>const NURBSControlVertex\& cv</b>\n\n
	The control vertex to compare.
	\return  TRUE if the CVs are equal; otherwise FALSE. */
	DllExport BOOL operator==(const NURBSControlVertex& cv);
	/*! \remarks Inequality operator. Compares if the coordinates or weight
	are <b>not</b> the same.
	\par Parameters:
	<b>const NURBSControlVertex\& cv</b>\n\n
	The control vertex to compare.
	\return  TRUE if the CVs are not equal; otherwise FALSE. */
	DllExport BOOL operator!=(const NURBSControlVertex& cv);
	/*! \remarks Sets the position of the control vertex.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to set the position.\n\n
	<b>Point3 pt</b>\n\n
	Specifies the position to set. */
	DllExport void SetPosition(TimeValue t, Point3 pt);
	/*! \remarks Sets the position of the control vertex.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to set the position.\n\n
	<b>float x, float y, float z</b>\n\n
	Specifies the position to set. */
	DllExport void SetPosition(TimeValue t, float x, float y, float z);
	/*! \remarks Sets the position of the control vertex.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to set the position.\n\n
	<b>double x, double y, double z</b>\n\n
	Specifies the position to set. */
	DllExport void SetPosition(TimeValue t, double x, double y, double z);
	/*! \remarks Returns the position of the control vertex.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the position. */
	DllExport Point3 GetPosition(TimeValue t);
	/*! \remarks Retrieves the position of the control vertex.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the position.\n\n
	<b>float\& x</b>\n\n
	The X coordinates is returned here.\n\n
	<b>float\& y</b>\n\n
	The Y coordinates is returned here.\n\n
	<b>float\& z</b>\n\n
	The Z coordinates is returned here. */
	DllExport void GetPosition(TimeValue t, float& x, float& y, float& z);
	/*! \remarks Retrieves the position of the control vertex.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the position.\n\n
	<b>double\& x</b>\n\n
	The X coordinates is returned here.\n\n
	<b>double\& y</b>\n\n
	The Y coordinates is returned here.\n\n
	<b>double\& z</b>\n\n
	The Z coordinates is returned here. */
	DllExport void GetPosition(TimeValue t, double& x, double& y, double& z);
	/*! \remarks Sets the weight of the control vertex.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to set the weight.\n\n
	<b>float wt</b>\n\n
	Specifies the weight to set. This is a value greater than zero. Larger
	values cause the CV to have a greater effect, thus the curve or surface
	will try to pass closer to the CV. */
	DllExport void SetWeight(TimeValue t, float wt);
	/*! \remarks Sets the weight of the control vertex.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to set the weight.\n\n
	<b>double wt</b>\n\n
	Specifies the weight to set. . This is a value greater than zero. Larger
	values cause the CV to have a greater effect, thus the curve or surface
	will try to pass closer to the CV. */
	DllExport void SetWeight(TimeValue t, double wt);
	/*! \remarks Retrieves the weight of the control vertex.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the weight.\n\n
	<b>float\& wt</b>\n\n
	The weight is returned here. */
	DllExport void GetWeight(TimeValue t, float& wt);
	/*! \remarks Returns the weight of the control vertex.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the weight. */
	DllExport double GetWeight(TimeValue t);
	/*! \remarks Retrieves the weight of the control vertex.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the weight.\n\n
	<b>double\& wt</b>\n\n
	The weight is returned here.
	\par Operators:
	*/
	DllExport void GetWeight(TimeValue t, double& wt);
};


/*! \sa  Class NURBSObject, \ref nurbConstTypes, Class Point3.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class is used to create a dependent point that lies at a point or relative
to it.\n\n
All methods of this class are implemented by the system.
\par Data Members:
protected:\n\n
<b>NURBSId mParentId;</b>\n\n
The id of the parent object.\n\n
<b>int mParentIndex;</b>\n\n
The index in the NURBSSet of the parent object.\n\n
<b>NURBSConstType mCType;</b>\n\n
The type of constraint in use.\n\n
<b>Point3 mOffset;</b>\n\n
The offset from the object that the point is. */
class NURBSPointConstPoint : public NURBSPoint {
	friend class NURBSSet;
protected:
	NURBSId mParentId;
	int mParentIndex;
	NURBSConstType mCType;
	Point3 mOffset;
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);

public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNPointCPoint;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mParentId = 0;</b>\n\n
	<b> mParentIndex = -1;</b>\n\n
	<b> mCType = kNConstOnObject;</b>\n\n
	<b> mOffset = Point3(0,0,0);</b> */
	DllExport NURBSPointConstPoint(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSPointConstPoint(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSPointConstPoint\& pt</b>\n\n
	The construction point to assign. */
	DllExport NURBSPointConstPoint & operator=(const NURBSPointConstPoint& pt);
	/*! \remarks Sets the parent index to the specified value.
	\par Parameters:
	<b>int index</b>\n\n
	The parent index to set. */
	DllExport void SetParent(int index);
	/*! \remarks Sets the parent ID to the specified value.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	The parent ID to set. */
	DllExport void SetParentId(NURBSId id);
	/*! \remarks Returnst the parent index. */
	DllExport int GetParent(void);
	/*! \remarks Returns the <b>NURBSId</b> of the parent. Note that a
	<b>NURBSId</b> won't be valid until the object has been instantiated in the
	scene. */
	DllExport NURBSId GetParentId(void);
	/*! \remarks Sets the type of point.
	\par Parameters:
	<b>NURBSConstType type</b>\n\n
	Specifies the type of point to set. See \ref nurbConstTypes. */
	DllExport void SetPointType(NURBSConstType type);
	/*! \remarks Returns the type of construction point this is. See \ref nurbConstTypes. */
	DllExport NURBSConstType GetPointType(void);
	/*! \remarks Sets the offset value at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	Specifies the time at which the offset is set.\n\n
	<b>Point3 pt</b>\n\n
	The offset value to set. This value is specified in object coordinates. */
	DllExport void SetOffset(TimeValue t, Point3 pt);
	/*! \remarks Returns the offset value at the specified time in object
	coordinates.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to retrieve the offset.
	\par Operators:
	*/
	DllExport Point3 GetOffset(TimeValue t);
};

/*! \sa  Class NURBSObject, \ref nurbConstTypes, Class Point3.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class is used to create a dependent point that lies on a curve or relative
to it.\n\n
The point can either be on the curve or off the curve. If it is on the curve,
the <b>U Position</b> is the only control of its location. The <b>U
Position</b> specifies a location along the curve (based on the curve's local U
axis). There are three ways to displace the point's location relative to the U
position:\n\n
<b>Offset</b> moves the point according to a relative (object space) X,Y,Z
location.\n\n
<b>Normal</b> moves the point along the direction of the curve's normal.
(Negative values move it opposite to the normal.)\n\n
U Position\n\n
<b>Tangent</b> moves the point along the tangent of the <b>U Position</b>.\n\n
All methods of this class are implemented by the system.
\par Data Members:
protected:\n\n
<b>NURBSId mParentId;</b>\n\n
The NURBSId of the parent curve.\n\n
<b>int mParentIndex;</b>\n\n
The index in the <b>NURBSSet</b> of the parent curve.\n\n
<b>NURBSConstType mCType;</b>\n\n
The type of constraint in use.\n\n
<b>Point3 mOffset;</b>\n\n
The offset amount.\n\n
<b>float mNormal;</b>\n\n
The distance along the normal.\n\n
<b>float mUTangent;</b>\n\n
The distance along the tangent.\n\n
<b>double mUParam;</b>\n\n
Specifies the point along the parent curve.\n\n
<b>BOOL mTrimCurve;</b>\n\n
The point that is constrained to the curve may be used to trim the curve.\n\n
<b>BOOL mFlipTrim;</b>\n\n
If TRUE the curve is trimmed from the point towards low parameter space. If
FALSE the curve is trimmed from the point towards high parameter space. */
class NURBSCurveConstPoint : public NURBSPoint {
	friend class NURBSSet;
protected:
	NURBSId mParentId;
	int mParentIndex;
	NURBSConstType mCType;
	Point3 mOffset;
	float mNormal;
	float mUTangent;
	double mUParam;
    BOOL mTrimCurve;
    BOOL mFlipTrim;

	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNCurveCPoint;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mParentId = 0;</b>\n\n
	<b> mParentIndex = -1;</b>\n\n
	<b> mCType = kNConstOnObject;</b>\n\n
	<b> mOffset = Point3(0,0,0);</b>\n\n
	<b> mNormal = 0.0f;</b>\n\n
	<b> mUTangent = 0.0f;</b>\n\n
	<b> mUParam = 0.0;</b>\n\n
	<b> mTrimCurve = FALSE;</b>\n\n
	<b> mFlipTrim = FALSE;</b> */
	DllExport NURBSCurveConstPoint(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSCurveConstPoint(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSCurveConstPoint\& pt</b>\n\n
	The point to assign. */
	DllExport NURBSCurveConstPoint & operator=(const NURBSCurveConstPoint& pt);
	/*! \remarks Sets the parent index to the specified value.
	\par Parameters:
	<b>int index</b>\n\n
	The parent index to set. */
	DllExport void SetParent(int index);
	/*! \remarks Sets the parent ID to the specified value.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	The parent ID to set. */
	DllExport void SetParentId(NURBSId id);
	/*! \remarks Returns the parent index. */
	DllExport int GetParent(void);
	/*! \remarks Returns the <b>NURBSId</b> of the parent. Note that a
	<b>NURBSId</b> won't be valid until the object has been instantiated in the
	scene. */
	DllExport NURBSId GetParentId(void);
	/*! \remarks Sets the point type.
	\par Parameters:
	<b>NURBSConstType type</b>\n\n
	Specifies the construction point type. See \ref nurbConstTypes. */
	DllExport void SetPointType(NURBSConstType type);
	/*! \remarks Returns the point type. See \ref nurbConstTypes. */
	DllExport NURBSConstType GetPointType(void);
	/*! \remarks Sets the offset value at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	Specifies the time at which the offset value is set.\n\n
	<b>Point3 pt</b>\n\n
	The offset to set in object space. */
	DllExport void SetOffset(TimeValue t, Point3 pt);
	/*! \remarks Returns the offset value in object space at the specified
	time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	Specifies the time at which the offset value is returned. */
	DllExport Point3 GetOffset(TimeValue t);
	/*! \remarks Sets the U Parameter for the point.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	Specifies the time at which the value is set.\n\n
	<b>double param</b>\n\n
	The U parameter to set. */
	DllExport void SetUParam(TimeValue t, double param);
	/*! \remarks Returns the U parameter at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	Specifies the time at which the value is returned. */
	DllExport double GetUParam(TimeValue t);
	/*! \remarks Sets the normal distance at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	Specifies the time at which the value is set.\n\n
	<b>float dist</b>\n\n
	The distance to set. */
	DllExport void SetNormal(TimeValue t, float dist);
	/*! \remarks Returns the distance along the normal at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	Specifies the time at which the value is returned. */
	DllExport float GetNormal(TimeValue t);
	/*! \remarks Set the U tangent value at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	Specifies the time at which the value is set.\n\n
	<b>float dist</b>\n\n
	The distance to set. */
	DllExport void SetUTangent(TimeValue t, float dist);
	/*! \remarks Returns the U tangent value at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	Specifies the time at which the value is returned. */
	DllExport float GetUTangent(TimeValue t);
    /*! \remarks Returns TRUE if this point is used to trim the curve and
    FALSE if it is not. */
    DllExport BOOL GetTrimCurve();
    /*! \remarks Sets the trim curve boolean.
    \par Parameters:
    <b>BOOL trim</b>\n\n
    TRUE to indicate this point is used to trim the curve; FALSE to indicate
    the curve is not trimmed beyond the point. */
    DllExport void SetTrimCurve(BOOL trim);
    /*! \remarks Returns the state of the flip trim boolean. */
    DllExport BOOL GetFlipTrim();
    /*! \remarks Sets the state of the flip trim boolean.
    \par Parameters:
    <b>BOOL flip</b>\n\n
    TRUE to indicate the curve is trimmed from the point towards low parameter
    space. Use FALSE to indicate the curve is trimmed from the point towards
    high parameter space.
    \par Operators:
    */
    DllExport void SetFlipTrim(BOOL flip);
};

/*! \sa  Class NURBSPoint.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class is used to create a dependent point at the intersection of two
curves.\n\n
All methods of this class are implemented by the system.
\par Data Members:
protected:\n\n
<b>NURBSId mParentId[2];</b>\n\n
The ids of the two parent curves.\n\n
<b>int mParentIndex[2];</b>\n\n
The indicies of the two parent curves in the <b>NURBSSet</b>.\n\n
<b>BOOL mTrimCurve[2];</b>\n\n
Indicates the point that is constrained to the curve may be used to trim the
curve.\n\n
<b>BOOL mFlipTrim[2];</b>\n\n
If TRUE the curve is trimmed from the point towards low parameter space. If
FALSE the curve is trimmed from the point towards high parameter space. */
class NURBSCurveCurveIntersectionPoint : public NURBSPoint {
	friend class NURBSSet;
protected:
	NURBSId mParentId[2];
	int mParentIndex[2];
    double mCurveParam[2];
    BOOL mTrimCurve[2];
    BOOL mFlipTrim[2];

	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized by value:\n\n
	<b>mType = kNCurveCurveIntersectionPoint;</b>\n\n
	<b>mpObject = NULL;</b>\n\n
	<b>mpNSet = NULL;</b>\n\n
	<b>mParentId[0] = mParentId[1] = 0;</b>\n\n
	<b>mParentIndex[0] = mParentIndex[1] = -1;</b>\n\n
	<b>mCurveParam[0] = mCurveParam[1] = 0.0;</b>\n\n
	<b>mTrimCurve[0] = mTrimCurve[1] = FALSE;</b>\n\n
	<b>mFlipTrim[0] = mFlipTrim[1] = FALSE;</b> */
	DllExport NURBSCurveCurveIntersectionPoint(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSCurveCurveIntersectionPoint(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSCurveCurveIntersectionPoint \&pt</b>\n\n
	The intersection point to assign. */
	DllExport NURBSCurveCurveIntersectionPoint & operator=(const NURBSCurveCurveIntersectionPoint &pt);
    DllExport void SetCurveParam(int curveNum, double param);
    /*! \remarks Returns the point in parameter space of the specified curve
    of the point of intersection.
    \par Parameters:
    <b>int curveNum</b>\n\n
    The parent curve number: 0 or 1. */
    DllExport double GetCurveParam(int curveNum);
	/*! \remarks Sets the index in the <b>NURBSSet</b> of the specified parent
	object.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1.\n\n
	<b>int index</b>\n\n
	The index into the <b>NURBSSet</b> of the parent surface. */
	DllExport void SetParent(int pnum, int index);
	/*! \remarks Sets the <b>NURBSId</b> of the specified parent.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1.\n\n
	<b>NURBSId id</b>\n\n
	The id to set. */
	DllExport void SetParentId(int pnum, NURBSId id);
	/*! \remarks Returns the index in the NURBSSet of the specified parent
	object.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1. */
	DllExport int GetParent(int pnum);
	/*! \remarks Returns the <b>NURBSId</b> of the specified parent. Note that
	a <b>NURBSId</b> won't be valid until the object has been instantiated in
	the scene
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1. */
	DllExport NURBSId GetParentId(int pnum);

    /*! \remarks Returns TRUE if this point is used to trim the specified
    curve and FALSE if it is not.
    \par Parameters:
    <b>int pnum</b>\n\n
    The parent curve number: 0 or 1. */
    DllExport BOOL GetTrimCurve(int pnum);
    /*! \remarks Sets the trim curve boolean for the specified curve.
    \par Parameters:
    <b>int pnum</b>\n\n
    The parent curve number: 0 or 1.\n\n
    <b>BOOL trim</b>\n\n
    TRUE to indicate this point is used to trim the curve; FALSE to indicate
    the curve is not trimmed beyond the point. */
    DllExport void SetTrimCurve(int pnum, BOOL trim);
    /*! \remarks Returns the state of the flip trim boolean.
    \par Parameters:
    <b>int pnum</b>\n\n
    The parent curve number: 0 or 1.
    \return  TRUE indicates the specified curve is trimmed from the point
    towards low parameter space. FALSE indicates the curve is trimmed from the
    point towards high parameter space. */
    DllExport BOOL GetFlipTrim(int pnum);
    /*! \remarks Sets the state of the flip trim boolean for the specified
    parent curve.
    \par Parameters:
    <b>int pnum</b>\n\n
    The parent curve number: 0 or 1.\n\n
    <b>BOOL flip</b>\n\n
    TRUE to indicate the curve is trimmed from the point towards low parameter
    space. Use FALSE to indicate the curve is trimmed from the point towards
    high parameter space.
    \par Operators:
    */
    DllExport void SetFlipTrim(int pnum, BOOL flip);
};

/*! \sa  Class NURBSObject, \ref nurbConstTypes.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class is used to create a dependent point on a surface or related to
it.\n\n
All methods of this class are implemented by the system.
\par Data Members:
protected:\n\n
<b>NURBSId mParentId;</b>\n\n
The <b>NURBSId</b> of the parent surface.\n\n
<b>int mParentIndex;</b>\n\n
The index in the <b>NURBSSet</b> of the parent surface.\n\n
<b>NURBSConstType mCType;</b>\n\n
The type of constraint used by the point.\n\n
<b>Point3 mOffset;</b>\n\n
The offset amount.\n\n
<b>float mNormal;</b>\n\n
The distance along the normal.\n\n
<b>float mUTangent;</b>\n\n
The positive tangent offset in U. At the location in parameter space of the
constrained point is a tangent to the surface. This is the distance along the
positive U tangent in parameter space.\n\n
<b>double mUParam;</b>\n\n
The point is constrained to exist on the parent surface. The surface itself is
defined over a parameter range. The point is defined at a point in the parent
surface parameter space. This is the location of the point in the parent
surface parameter space in U.\n\n
<b>float mVTangent;</b>\n\n
This is the distance along the positive V tangent in parameter space.\n\n
<b>double mVParam;</b>\n\n
This is the location of the point in the parent surface parameter space in V. */
class NURBSSurfConstPoint : public NURBSPoint {
	friend class NURBSSet;
protected:
	NURBSId mParentId;
	int mParentIndex;
	NURBSConstType mCType;
	Point3 mOffset;
	float mNormal;
	float mUTangent;
	double mUParam;
	float mVTangent;
	double mVParam;
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNSurfaceCPoint;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mParentId = 0;</b>\n\n
	<b> mParentIndex = -1;</b>\n\n
	<b> mCType = kNConstOnObject;</b>\n\n
	<b> mOffset = Point3(0,0,0);</b>\n\n
	<b> mNormal = 0.0f;</b>\n\n
	<b> mUTangent = 0.0f;</b>\n\n
	<b> mUParam = 0.0;</b>\n\n
	<b> mVTangent = 0.0f;</b>\n\n
	<b> mVParam = 0.0;</b> */
	DllExport NURBSSurfConstPoint(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSSurfConstPoint(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSSurfConstPoint\& pt</b>\n\n
	The point to assign. */
	DllExport NURBSSurfConstPoint & operator=(const NURBSSurfConstPoint& pt);
	/*! \remarks Sets the index in the <b>NURBSSet</b> of the parent object.
	\par Parameters:
	<b>int index</b>\n\n
	The index into the <b>NURBSSet</b> of the parent surface. */
	DllExport void SetParent(int index);
	/*! \remarks Sets the <b>NURBSId</b> of the specified parent.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	The id to set. */
	DllExport void SetParentId(NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the parent
	object. */
	DllExport int GetParent(void);
	/*! \remarks Returns the <b>NURBSId</b> of the parent. Note that a
	<b>NURBSId</b> won't be valid until the object has been instantiated in the
	scene */
	DllExport NURBSId GetParentId(void);
	/*! \remarks Sets the type of constrained point this is. See \ref nurbConstTypes.
	\par Parameters:
	<b>NURBSConstType type</b>\n\n
	The type of the constrained point. */
	DllExport void SetPointType(NURBSConstType type);
	/*! \remarks Returns the type of constrained point this is. See \ref nurbConstTypes */
	DllExport NURBSConstType GetPointType(void);
	/*! \remarks Sets the position of the point in the parent U parameter space.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to set the U parameter.\n\n
	<b>double param</b>\n\n
	The value to set. */
	DllExport void SetUParam(TimeValue t, double param);
	/*! \remarks Returns the position of the point in the parent U parameter
	space.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the U parameter. */
	DllExport double GetUParam(TimeValue t);
	/*! \remarks Sets the position of the point in the parent V parameter
	space.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to set the U parameter.\n\n
	<b>double param</b>\n\n
	The value to set. */
	DllExport void SetVParam(TimeValue t, double param);
	/*! \remarks Returns the position of the point in the parent V parameter
	space.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the Vparameter. */
	DllExport double GetVParam(TimeValue t);
	/*! \remarks Sets the offset of the point from the parent surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the offset.\n\n
	<b>Point3 pt</b>\n\n
	The offset to set. */
	DllExport void SetOffset(TimeValue t, Point3 pt);
	/*! \remarks Returns the offset of the point from the parent surface at
	the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the offset. */
	DllExport Point3 GetOffset(TimeValue t);
	/*! \remarks Sets the distance along the normal to the surface of the
	point at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the distance.\n\n
	<b>float dist</b>\n\n
	The distance to set. */
	DllExport void SetNormal(TimeValue t, float dist);
	/*! \remarks Returns the distance along the normal to the surface of the
	point at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the distance. */
	DllExport float GetNormal(TimeValue t);
	/*! \remarks Sets the distance along the U tangent of the point.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the distance.\n\n
	<b>float dist</b>\n\n
	The distance to set. */
	DllExport void SetUTangent(TimeValue t, float dist);
	/*! \remarks Returns the distance along the U tangent of the point.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the distance. */
	DllExport float GetUTangent(TimeValue t);
	/*! \remarks Sets the distance along the V tangent of the point.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the distance.\n\n
	<b>float dist</b>\n\n
	The distance to set. */
	DllExport void SetVTangent(TimeValue t, float dist);
	/*! \remarks Returns the distance along the V tangent of the point.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the distance.
	\par Operators:
	*/
	DllExport float GetVTangent(TimeValue t);
};

/*! \sa  Class NURBSPoint.\n\n
\par Description:
This class is available in release 2.5 and later only.\n\n
This class is used to create a dependent point at the intersection of a curve
and a surface.
\par Data Members:
protected:\n\n
<b>NURBSId mParentId[2];</b>\n\n
The NURBSIds of the parent surface and curve. Parent 0 should be the surface
parent 1 should be the curve.\n\n
<b>int mParentIndex[2];</b>\n\n
The NURBSSet indexes of the parent surface and curve. Parent 0 should be the
surface parent 1 should be the curve.\n\n
<b>double mSeed;</b>\n\n
The seed location is a U position along the length of the parent curve.\n\n
<b>BOOL mTrimCurve;</b>\n\n
The trim curve flag.\n\n
<b>BOOL mFlipTrim;</b>\n\n
The trim flip flag. */
class NURBSCurveSurfaceIntersectionPoint : public NURBSPoint {
	friend class NURBSSet;
protected:
	// parent 0 should be the surface parent 1 should be the curve
	NURBSId mParentId[2];
	int mParentIndex[2];
	double mSeed;
    BOOL mTrimCurve;
    BOOL mFlipTrim;

	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNCurveSurfaceIntersectionPoint;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mParentId[0] = mParentId[1] = 0;</b>\n\n
	<b> mParentIndex[0] = mParentIndex[1] = -1;</b>\n\n
	<b> mSeed = 0.5;</b>\n\n
	<b> mTrimCurve = FALSE;</b>\n\n
	<b> mFlipTrim = FALSE;</b> */
	DllExport NURBSCurveSurfaceIntersectionPoint(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSCurveSurfaceIntersectionPoint(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSCurveSurfaceIntersectionPoint \&pt</b>\n\n
	The point to assign. */
	DllExport NURBSCurveSurfaceIntersectionPoint & operator=(const NURBSCurveSurfaceIntersectionPoint &pt);
    /*! \remarks Sets the seed value.
    \par Parameters:
    <b>double seed</b>\n\n
    The U position along the length of the parent curve. */
    DllExport void SetSeed(double seed);
    /*! \remarks Returns the seed value. */
    DllExport double GetSeed();
	/*! \remarks Establishes the curve or surface used by specifying its index
	in the <b>NURBSSet</b>.
	\par Parameters:
	<b>int pnum</b>\n\n
	Pass 0 for the surface; 1 for the curve.\n\n
	<b>int index</b>\n\n
	The index in the NURBSSet of the curve or surface. */
	DllExport void SetParent(int pnum, int index);
	/*! \remarks Establishes the curve or surface used by specifying its
	<b>NURBSId</b>.
	\par Parameters:
	<b>int pnum</b>\n\n
	Pass 0 for the surface; 1 for the curve.\n\n
	<b>NURBSId id</b>\n\n
	The id to set. */
	DllExport void SetParentId(int pnum, NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the parent curve
	or surface.
	\par Parameters:
	<b>int pnum</b>\n\n
	Pass 0 for the surface; 1 for the curve. */
	DllExport int GetParent(int pnum);
	/*! \remarks Returns the <b>NURBSId</b> of the parent curve or surface.
	\par Parameters:
	<b>int pnum</b>\n\n
	Pass 0 for the surface; 1 for the curve. */
	DllExport NURBSId GetParentId(int pnum);

    /*! \remarks Returns the state of the trim curve flag. TRUE causes
    trimming; FALSE does not. */
    DllExport BOOL GetTrimCurve();
    /*! \remarks Sets the state of the trim curve flag.
    \par Parameters:
    <b>BOOL trim</b>\n\n
    TRUE to trim; FALSE to not trim. */
    DllExport void SetTrimCurve(BOOL trim);
    /*! \remarks Returns the state of the trim flip flag. TRUE for flipped;
    FALSE for not flipped. */
    DllExport BOOL GetFlipTrim();
    /*! \remarks Sets the state of the trim flip flag.
    \par Parameters:
    <b>BOOL flip</b>\n\n
    TRUE to flip; FALSE to not flip. */
    DllExport void SetFlipTrim(BOOL flip);
};





typedef Tab<NURBSControlVertex> NURBSCVTab;
typedef Tab<double> NURBSKnotTab;


/*! \defgroup nurbTrimDirections NURBS Trim Directions
Defines the side to Keep
\sa Class NURBSTrimPoint */
//@{
enum NURBSTrimDirection // 
{
	/*! Specifies the curve on both sides of the point should be kept -- no trimming
	is done. */
    kNone = 0,
    /*! Specifies the curve on the positive side of the curve should be kept. The
    positive side is the side between the point towards greater values in the curve
    parameter space. */
    kPositive = 1,
    /*! Specifies the curve on the negative side of the curve should be kept. The
    negative side is the side between the point towards lesser values in the curve
    parameter space. */
    kNegative = 2
};
//@}

/*! \sa \ref nurbTrimDirections.
\par Description:
This class is available in release 2.0 and later only.\n\n
This class defines a point on a curve used to trim a portion of the curve from
the point towards one of the ends of the curve. The trim point is defined by a
parameter and a direction. For instance, consider a CV curve that exists in the
parameter space from 0.0 to 1.0. If there is a trim point in the middle of the
curve the parameter for the point would be 0.5. If the portion that was being
trimmed off was from 0.5 to 1.0 then the trim direction would be positive.\n\n
All methods of this class are implemented by the system. */
class NURBSTrimPoint: public MaxHeapOperators {
public:
	/*! \remarks Constructor.
	\par Parameters:
	<b>double parameter</b>\n\n
	This is point in parameter space at which the trim point exists.\n\n
	<b>NURBSTrimDirection direction</b>\n\n
	The positive or negative direction in parameter space of the portion that is
	being trimmed off. The direction can also be set to 'none' so no trimming is
	done. */
    DllExport NURBSTrimPoint(double parameter, NURBSTrimDirection direction) :
        mParameter(parameter), mDirection(direction) {}
    /*! \remarks Returns the point in parameter space at which the trim point
    exists. */
    DllExport double GetParameter() {return mParameter; }
    /*! \remarks Returns the trim direction (positive, negative, or none)
    describing which portion of the curve is trimmed off. */
    DllExport NURBSTrimDirection GetDirection() {return mDirection; }

private:
    double mParameter;
    NURBSTrimDirection mDirection;
};



/*! \sa  Class NURBSObject, Class NURBSTrimPoint.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class describes the properties of a NURBS curve. This includes its number
of trim points and its open/closed state. The <b>Evaluate()</b> method is used
to compute points on the curve.\n\n
All methods of this class are implemented by the system.
\par Data Members:
protected:\n\n
<b>int mMatID;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
The material ID for the curve.  */
class NURBSCurve : public NURBSObject {
protected:
	friend class NURBSCVCurve;
	friend class NURBSPointCurve;
	friend class NURBSBlendCurve;
	friend class NURBSOffsetCurve;
	friend class NURBSXFormCurve;
	friend class NURBSMirrorCurve;
	friend class NURBSFilletCurve;
	friend class NURBSChamferCurve;
	friend class NURBSIsoCurve;
	friend class NURBSSurfaceEdgeCurve;
	friend class NURBSProjectVectorCurve;
	friend class NURBSProjectNormalCurve;
	friend class NURBSSurfaceNormalCurve;
	friend class NURBSNBlendSurface;
	friend class NURBSRuledSurface;
	friend class NURBSULoftSurface;
	friend class NURBSUVLoftSurface;
	friend class NURBSExtrudeSurface;
	friend class NURBSLatheSurface;
	friend class NURBSCapSurface;
	friend class NURBS1RailSweepSurface;
	friend class NURBS2RailSweepSurface;
	friend class NURBSMultiCurveTrimSurface;

	int mMatID;

public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mKind = kNURBSCurve;</b>\n\n
	<b> mMatID = 1;</b> */
	DllExport NURBSCurve(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSCurve(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSCurve\& curve</b>\n\n
	The curve to assign. */
	DllExport NURBSCurve & operator=(const NURBSCurve& curve);
	/*! \remarks Returns TRUE if the curve is closed; otherwise FALSE. */
	DllExport BOOL IsClosed(void);

    /*! \remarks Returns the number of trim points in the curve. */
    DllExport int NumTrimPoints();
    /*! \remarks Returns the 'i-th' trim point.
    \par Parameters:
    <b>TimeValue t</b>\n\n
    The time to retrieve the trim point.\n\n
    <b>int i</b>\n\n
    The zero based index of the trim point to return. */
    DllExport NURBSTrimPoint GetTrimPoint(TimeValue t, int i);
    
	/*! \remarks Retrieves the point along the curve at the specified point
	and the tangent at that point.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to evaluate the curve.\n\n
	<b>double u</b>\n\n
	Specifies the point along the curve to evaluate. This value must be between
	the <b>uMin</b> and <b>uMax</b> as returned from
	<b>GetParameterRange()</b>.\n\n
	<b>Point3\& pt</b>\n\n
	The point along the curve is returned here.\n\n
	<b>Point3\& tangent</b>\n\n
	The tangent at the specified point is returned here.
	\return  TRUE if the method was able to evaluate the curve; otherwise
	FALSE. */
	DllExport BOOL Evaluate(TimeValue t, double u, Point3& pt, Point3& tangent);
	/*! \remarks Retrieves the minimum and maximum valid values for <b>u</b>
	as passed to <b>Evaluate()</b>.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the parameter range of the curve.\n\n
	<b>double\& uMin</b>\n\n
	The minimum value is returned here.\n\n
	<b>double\& uMax</b>\n\n
	The maximum value is returned here. */
	DllExport void GetParameterRange(TimeValue t, double& uMin, double& uMax);
	/*! \remarks	Retrieves data about the NURBSCurve at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the NURBS information.\n\n
	<b>int\& degree</b>\n\n
	The degree of the curve.\n\n
	<b>int\& numCVs</b>\n\n
	The number of CVs.\n\n
	<b>NURBSCVTab\& cvs</b>\n\n
	The table of CVs. Note: <b>typedef Tab\<NURBSControlVertex\>
	NURBSCVTab;</b>\n\n
	<b>int\& numKnots</b>\n\n
	The number of knots.\n\n
	<b>NURBSKnotTab knots</b>\n\n
	A table of knots in U. Note: <b>typedef Tab\<double\> NURBSKnotTab;</b>
	\return  TRUE if the data was retrieved; otherwise FALSE. */
	DllExport BOOL GetNURBSData(TimeValue t,
								int& degree,
								int& numCVs,
								NURBSCVTab& cvs,
								int& numKnots,
								NURBSKnotTab& knots);
	/*! \remarks	Returns the material ID for the curve. */
	DllExport int MatID();
	/*! \remarks	Sets the material ID for the curve.
	\par Parameters:
	<b>int id</b>\n\n
	The ID to set.
	\par Operators:
	*/
	DllExport void MatID(int id);
};


/*! \sa  Class NURBSCurve, Class NURBSControlVertex.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class defines a NURBS CV Curve. CV Curves have control vertices. The
position of the control vertices (CVs) controls the shape of the curve. Unlike
spline vertices, CVs don't necessarily lie on the curve they define. The CVs
define a control lattice which surrounds the NURBS curve.\n\n
All methods of this class are implemented by the system.
\par Data Members:
protected:\n\n
<b>NURBSControlVertex *mpCVs;</b>\n\n
Array of control vertices.\n\n
<b>double *mpKnots;</b>\n\n
Array of knots.\n\n
<b>BOOL mClosed;</b>\n\n
TRUE if the curve is closed; otherwise FALSE.\n\n
<b>int mOrder;</b>\n\n
The order of the curve.\n\n
<b>int mNumKnots;</b>\n\n
The number of knots.\n\n
<b>int mNumCVs;</b>\n\n
The number of control verticies.\n\n
<b>NURBSAutoParam mAutoParam;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
Controls automatic reparameterization. See \ref nurbAutoParamTypes. */
class NURBSCVCurve : public NURBSCurve {
	friend class NURBSSet;
protected:
	NURBSControlVertex *mpCVs;
	double *mpKnots;
	BOOL mClosed;
	int mOrder;
	int mNumKnots;
	int mNumCVs;
	NURBSAutoParam mAutoParam;
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNCVCurve;</b>\n\n
	<b> mClosed = FALSE;</b>\n\n
	<b> mpCVs = NULL;</b>\n\n
	<b> mpKnots = NULL;</b> */
	DllExport NURBSCVCurve(void);
	/*! \remarks Destructor. If the knots and CV arrays are allocated they are
	freed and any caches are cleared. */
	DllExport virtual ~NURBSCVCurve(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSCVCurve\& curve</b>\n\n
	The NURBSCVCurve to assign. */
	DllExport NURBSCVCurve & operator=(const NURBSCVCurve& curve);
	/*! \remarks Closes the curve. */
	DllExport void Close(void);
	DllExport BOOL IsClosed(void);

	/*! \remarks Sets the order of the curve. This is one more than the degree
	of polynomial of any segment of the curve. All curves have a degree. The
	degree of a curve is the highest exponent in the equation used to represent
	it. A linear equation is degree 1, a quadratic equation degree 2. NURBS
	curves typically are represented by cubic equations and have a degree of 3.
	\par Parameters:
	<b>int order</b>\n\n
	Specifies the order of the curve. */
	DllExport void SetOrder(int order);
	/*! \remarks Returns the order of the curve. */
	DllExport int GetOrder(void);
	/*! \remarks Sets the number of knots in the curve. Note that the previous
	knot data is NOT maintained\n\n
	Because they are generated mathematically, NURBS curves have a parameter
	space in addition to the 3D geometric space in which they are displayed.
	Specifically, an array of values called knots specifies the extent of
	influence of each control vertex (CV) on the curve or surface.
	\par Parameters:
	<b>int num</b>\n\n
	Specifies the number of knots for the curve. */
	DllExport void SetNumKnots(int num);         // data is NOT maintained
	/*! \remarks Returns the number of knots in the curve. */
	DllExport int GetNumKnots(void);
	/*! \remarks Set the number of control vertices in the curve. Note that
	the previous control vertex data is NOT maintained.
	\par Parameters:
	<b>int num</b>\n\n
	Specifies the number of control vertices. */
	DllExport void SetNumCVs(int num);           // data is NOT maintained
	/*! \remarks Retrieves the number of control vertices.
	\par Parameters:
	<b>int\& num</b>\n\n
	The number is stored here. */
	DllExport void GetNumCVs(int& num);
	/*! \remarks Returns the number of control vertices. */
	DllExport int GetNumCVs(void);
	/*! \remarks Returns the knot value whose index is passed.
	\par Parameters:
	<b>int index</b>\n\n
	Specifies which knot value to return. */
	DllExport double GetKnot(int index);
	/*! \remarks Sets the specified knot to the specified value.
	\par Parameters:
	<b>int index</b>\n\n
	The 0 based index of the knot to set.\n\n
	<b>double value</b>\n\n
	Specifies the value to set. */
	DllExport void SetKnot(int index, double value);
	/*! \remarks Returns a pointer to the specified control vertex of the
	curve.
	\par Parameters:
	<b>int index</b>\n\n
	The 0 based index of the control vertex to return. */
	DllExport NURBSControlVertex* GetCV(int index);
	/*! \remarks Sets the specified control vertex to the CV passed.
	\par Parameters:
	<b>int index</b>\n\n
	The 0 based index of the control vertex to set.\n\n
	<b>NURBSControlVertex \&cv</b>\n\n
	The CV to set. */
	DllExport void SetCV(int index, NURBSControlVertex &cv);
	/*! \remarks Sets the transformation matrix for the <b>NURBSCVCurve</b>.
	This controls the relative position of the item within a <b>NURBSSet</b>.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the matrix.\n\n
	<b>SetXFormPacket\& xPack</b>\n\n
	An instance of the <b>XFormPacket</b> class that describes the properties
	of the transformation. See Class SetXFormPacket. */
	DllExport void SetTransformMatrix(TimeValue t, SetXFormPacket& xPack);
	/*! \remarks Returns the transformation matrix of the <b>NURBSCVCurve</b>
	at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to retrieve the matrix. */
	DllExport Matrix3 GetTransformMatrix(TimeValue t);
	/*! \remarks	Returns the current settings for automatic reparameterization. See
	\ref nurbAutoParamTypes. */
	DllExport NURBSAutoParam AutoParam();
	/*! \remarks	Sets the automatic reparameterization settings at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to set them.\n\n
	<b>NURBSAutoParam param</b>\n\n
	The settings to establish. See \ref nurbAutoParamTypes. */
	DllExport void AutoParam(TimeValue t, NURBSAutoParam param);
	/*! \remarks	This method reparameterizes this CV sub-object. This can be used to provide
	a better relation between control point locations and the shape of the
	sub-object.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to reparameterize.\n\n
	<b>NURBSParamaterization param</b>\n\n
	The type of reparameterizing to perform. See \ref nurbMirrorAxisTypes. */
	DllExport void Reparameterize(TimeValue t, NURBSParamaterization param);

	/*! \remarks This method determines if the ends of the curve overlap even
	though the curve may not be closed (that is, the tangents match at the
	ends).
	\par Parameters:
	<b>BOOL\& overlap</b>\n\n
	The result is returned here: TRUE if the ends overlap; otherwise FALSE. */
	DllExport void EndsOverlap(BOOL& overlap);
	/*! \remarks By specifying a parameter point on the curve, this method
	adds a new control vertex to the curve. It moves all the other CVs but
	maintains the shape of the curve. Note that this causes the loss of any
	animation on the curve.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to refine the curve. If the curve is animated the
	<b>u</b> parameter below is time dependent.\n\n
	<b>double u</b>\n\n
	Specifies the distance along the curve to add the CV. See the base class
	method <b>GetParameterRange()</b> for the valid range of values for this
	parameter. */
	DllExport void Refine(TimeValue t, double u); // looses animation
	/*! \remarks	This method places a new CV along the current CV hull at the specified
	parameter point. This method leaves all the other CVs in place and changes
	the shape of the curve. This method preserves animation.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to refine the curve. If the curve is animated the
	<b>u</b> parameter below is time dependent.\n\n
	<b>double u</b>\n\n
	Specifies the distance along the curve to add the CV. See the base class
	method <b>GetParameterRange()</b> for the valid range of values for this
	parameter.
	\par Operators:
	*/
	DllExport void Insert(TimeValue t, double u);
};


/*! \sa  Class NURBSCurve, Class NURBSIndependentPoint.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class defines a curve that uses points to describe its shape. All the
points lie on the curve itself. There are methods to get/set the number of
points in the curve, get/set the points themselves, refine the curve (add
points without changing its shape), and to get/set the transformation matrix
used by the curve. This matrix is used to set the postion of the curve in the
<b>NURBSSet</b>.\n\n
All methods of this class are implemented by the system.
\par Data Members:
protected:\n\n
<b>NURBSIndependentPoint *mpPts;</b>\n\n
Array of independent points.\n\n
<b>BOOL mClosed;</b>\n\n
TRUE if the curve is closed; otherwise FALSE.\n\n
<b>int mNumPts;</b>\n\n
The number of independent points.  */
class NURBSPointCurve : public NURBSCurve {
	friend class NURBSSet;
protected:
	NURBSIndependentPoint *mpPts;
	BOOL mClosed;
	int mNumPts;
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNPointCurve;</b>\n\n
	<b> mClosed = FALSE;</b>\n\n
	<b> mpPts = NULL;</b> */
	DllExport NURBSPointCurve(void);
	/*! \remarks Destructor. If any points were allocated they are freed and
	the cache is cleared. */
	DllExport virtual ~NURBSPointCurve(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSPointCurve\& curve</b>\n\n
	The curve to assign. */
	DllExport NURBSPointCurve & operator=(const NURBSPointCurve& curve);
	/*! \remarks Closes the point curve. */
	DllExport void Close(void);
	/*! \remarks	Returns TRUE if the curve is closed; otherwise FALSE. */
	DllExport BOOL IsClosed(void);

	/*! \remarks Sets the number of points in the point curve. Note that any
	previous point data is not maintained when the new number is set.
	\par Parameters:
	<b>int num</b>\n\n
	The number of points in the curve. */
	DllExport void SetNumPts(int num);       // data is NOT maintained
	/*! \remarks Returns the number of points in the curve. */
	DllExport int GetNumPts(void);
	/*! \remarks Retrieves the number of points in the curve.
	\par Parameters:
	<b>int \&num</b>\n\n
	The result is stored here. */
	DllExport void GetNumPts(int &num);
	/*! \remarks Returns a pointer to the specified point.
	\par Parameters:
	<b>int index</b>\n\n
	The zero based index of the point to get. */
	DllExport NURBSIndependentPoint* GetPoint(int index);
	/*! \remarks Sets the specified point in the curve.
	\par Parameters:
	<b>int index</b>\n\n
	The zero based index of the point to set.\n\n
	<b>NURBSIndependentPoint \&pt</b>\n\n
	The point to set. */
	DllExport void SetPoint(int index, NURBSIndependentPoint &pt);
	/*! \remarks Sets the transformation matrix for the
	<b>NURBSPointCurve</b>. This controls the relative position of the item
	within a <b>NURBSSet</b>.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the matrix.\n\n
	<b>SetXFormPacket\& xPack</b>\n\n
	An instance of the <b>XFormPacket</b> class that describes the properties
	of the transformation. See Class SetXFormPacket. */
	DllExport void SetTransformMatrix(TimeValue t, SetXFormPacket& xPack);
	/*! \remarks Returns the transformation matrix that controls the relative
	position of the point curve in the <b>NURBSSet</b>.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to return the matrix. */
	DllExport Matrix3 GetTransformMatrix(TimeValue t);
	/*! \remarks This method adds a new point at the specified location on the
	curve without changing its shape. The point is specified as a distance in U
	parameter space. Note that calling this method causes all animation of the
	curve to be removed.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to refine the curve. The curve may be animated, and thus
	the underlying parameter space may be changing. So when the <b>u</b> value
	is specified it must relate to the curve at a specific time.\n\n
	<b>double u</b>\n\n
	The point at which to refine the curve. Developers should use the method
	<b>NURBSCurve::GetParameterRange()</b> to get the valid range of values
	that may be passed here.
	\par Operators:
	*/
	DllExport void Refine(TimeValue t, double u); // looses animation
};


/*! \sa  Class NURBSCurve.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class defines a dependent blend curve. A blend curve connects the
specified end of one curve to the specified end of another, blending the
curvature of the parents to create a smooth curve between them. Methods are
available to get/set the parent indices and ids, to get/set the ends of the
curves used for the blend, and to get/set the tension values used.\n\n
All methods of this class are implemented by the system. */
class NURBSBlendCurve : public NURBSCurve {
	friend class NURBSSet;
	NURBSId mParentId[2];
	int mParentIndex[2];
	BOOL mParentEnd[2];
	double mTension[2];
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	\code
	mType = kNBlendCurve;
	mpObject = NULL;
	mpNSet = NULL;
	for (int i = 0; i < 2; i++)
	{
		mParentId[i] = 0;
		mParentIndex[i] = -1;
		mParentEnd[i] = TRUE;
		mTension[i] = 1.0;
	}
	\endcode  */
	DllExport NURBSBlendCurve(void);
	/*! \remarks Destructor. The cache is cleared. */
	DllExport virtual ~NURBSBlendCurve(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSBlendCurve\& curve</b>\n\n
	The curve to assign. */
	DllExport NURBSBlendCurve & operator=(const NURBSBlendCurve& curve);
	/*! \remarks Sets the index in the <b>NURBSSet</b> of the specified parent
	object.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1.\n\n
	<b>int index</b>\n\n
	The index into the <b>NURBSSet</b> of the parent object. */
	DllExport void SetParent(int pnum, int index);
	/*! \remarks Sets the <b>NURBSId</b> of the specified parent.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1.\n\n
	<b>NURBSId id</b>\n\n
	The id to set. */
	DllExport void SetParentId(int pnum, NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the specified
	parent object.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1. */
	DllExport int GetParent(int pnum);
	/*! \remarks Returns the <b>NURBSId</b> of the parent. Note that a
	<b>NURBSId</b> won't be valid until the object has been instantiated in the
	scene
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1. */
	DllExport NURBSId GetParentId(int pnum);
	/*! \remarks Sets if the beginning or end of the specified curve is used
	for the blend.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1.\n\n
	<b>BOOL end</b>\n\n
	TRUE to use the end of the curve; FALSE to use the beginning. The beginning
	of the curve has lesser parameter values than the end. */
	DllExport void SetEnd(int pnum, BOOL end);
	/*! \remarks Indicates if the beginning or end of the specified curve is
	used for the blend.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1.
	\return  TRUE if the end of the curve is used; FALSE if the beginning is
	used. */
	DllExport BOOL GetEnd(int pnum);
	/*! \remarks Sets the tension value for the specified parent curve.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the tension\n\n
	<b>int pnum</b>\n\n
	The parent number: 0 or 1.\n\n
	<b>double ten</b>\n\n
	The tension value to set. */
	DllExport void SetTension(TimeValue t, int pnum, double ten);
	/*! \remarks Returns the tension value for the specified parent curve.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the tension\n\n
	<b>int pnum</b>\n\n
	The parent number: 0 or 1.
	\par Operators:
	*/
	DllExport double GetTension(TimeValue t, int pnum);
};


/*! \sa  Class NURBSCurve.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class defines a dependent offset curve. An offset curve is offset from the
original, parent curve. It lies in the same plane as its parent, and is normal
to the original. Methods are available to get/set the parent index and id and
to get/set the distance from the parent curve.\n\n
All methods of this class are implemented by the system. */
class NURBSOffsetCurve : public NURBSCurve {
	friend class NURBSSet;
	NURBSId mParentId;
	int mParentIndex;
	double mDistance;
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNOffsetCurve;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mParentId = 0;</b>\n\n
	<b> mParentIndex = -1;</b>\n\n
	<b> mDistance = 0.0;</b> */
	DllExport NURBSOffsetCurve(void);
	/*! \remarks Destructor. The cache is cleared. */
	DllExport virtual ~NURBSOffsetCurve(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSOffsetCurve\& curve</b>\n\n
	The curve to assign. */
	DllExport NURBSOffsetCurve & operator=(const NURBSOffsetCurve& curve);
	/*! \remarks Sets the index in the <b>NURBSSet</b> of the parent object.
	\par Parameters:
	<b>int index</b>\n\n
	The index into the <b>NURBSSet</b> of the parent curve. */
	DllExport void SetParent(int index);
	/*! \remarks Sets the <b>NURBSId</b> of the parent.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	The id to set. */
	DllExport void SetParentId(NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the specified
	parent object. */
	DllExport int GetParent(void);
	/*! \remarks Returns the <b>NURBSId</b> of the parent. Note that a
	<b>NURBSId</b> won't be valid until the object has been instantiated in the
	scene */
	DllExport NURBSId GetParentId(void);
	/*! \remarks Sets the distance of the offset curve from the original in
	3ds Max units at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to set the offset distance.\n\n
	<b>double d</b>\n\n
	The distance to set. */
	DllExport void SetDistance(TimeValue t, double d);
	/*! \remarks Returns the distance of the offset curve from the parent at
	the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the offset distance.
	\par Operators:
	*/
	DllExport double GetDistance(TimeValue t);
};

/*! \sa  Class NURBSCurve.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class defines a dependent transform (xform) curve. A transform curve is a
copy of the original curve with a different position, rotation, or scale.\n\n
All methods of this class are implemented by the system. */
class NURBSXFormCurve : public NURBSCurve {
	friend class NURBSSet;
	NURBSId mParentId;
	int mParentIndex;
	Matrix3 mXForm;
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNXFormCurve;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mParentId = 0;</b>\n\n
	<b> mParentIndex = -1;</b>\n\n
	<b> mXForm.IdentityMatrix();</b> */
	DllExport NURBSXFormCurve(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSXFormCurve(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSXFormCurve\& curve</b>\n\n
	The curve to assign. */
	DllExport NURBSXFormCurve & operator=(const NURBSXFormCurve& curve);
	/*! \remarks Sets the index in the <b>NURBSSet</b> of the specified parent
	object.
	\par Parameters:
	<b>int index</b>\n\n
	The index into the <b>NURBSSet</b> of the parent curve. */
	DllExport void SetParent(int index);
	/*! \remarks Sets the <b>NURBSId</b> of the parent.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	The id to set. */
	DllExport void SetParentId(NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the parent
	object. */
	DllExport int GetParent(void);
	/*! \remarks Returns the <b>NURBSId</b> of the parent. Note that a
	<b>NURBSId</b> won't be valid until the object has been instantiated in the
	scene */
	DllExport NURBSId GetParentId(void);
	/*! \remarks Sets the transformation used to move/rotate/scale the curve
	from the original.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to set the transformation.\n\n
	<b>Matrix3\& mat</b>\n\n
	The transformation matrix to set. */
	DllExport void SetXForm(TimeValue t, Matrix3& mat);
	/*! \remarks Returns the transformation used to move/rotate/scale the
	curve from the original.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the transformation.
	\par Operators:
	*/
	DllExport Matrix3& GetXForm(TimeValue t);
};

/*! \sa  Class NURBSCurve, \ref nurbMirrorAxisTypes.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class defines a dependent mirror curve. A mirror curve is similar to a
mirror object that you create using the Mirror tool (on the 3ds Max toolbar) or
the Mirror modifier. It is the original curve relfected about one or two axes.
Methods are available to get/set the parent index and id, to get/set the
reflection axis, and to get/set the offset distance.\n\n
All methods of this class are implemented by the system. */
class NURBSMirrorCurve : public NURBSCurve {
	friend class NURBSSet;
	NURBSId mParentId;
	int mParentIndex;
	NURBSMirrorAxis mAxis;
	Matrix3 mXForm;
	double mDistance;
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNMirrorCurve;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mParentId = 0;</b>\n\n
	<b> mParentIndex = -1;</b>\n\n
	<b> mXForm.IdentityMatrix();</b>\n\n
	<b> mAxis = kMirrorX;</b>\n\n
	<b> mDistance = 0.0;</b> */
	DllExport NURBSMirrorCurve(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSMirrorCurve(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSMirrorCurve\& curve</b>\n\n
	The curve to assign. */
	DllExport NURBSMirrorCurve & operator=(const NURBSMirrorCurve& curve);
	/*! \remarks Sets the index in the <b>NURBSSet</b> of the parent object.
	\par Parameters:
	<b>int index</b>\n\n
	The index into the NURBSSet of the parent curve. */
	DllExport void SetParent(int index);
	/*! \remarks Sets the <b>NURBSId</b> of the specified parent.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	The id to set. */
	DllExport void SetParentId(NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the parent
	object. */
	DllExport int GetParent(void);
	/*! \remarks Returns the <b>NURBSId</b> of the parent. Note that a
	<b>NURBSId</b> won't be valid until the object has been instantiated in the
	scene */
	DllExport NURBSId GetParentId(void);
	/*! \remarks Sets the axis or axes of reflection for the curve.
	\par Parameters:
	<b>NURBSMirrorAxis axis</b>\n\n
	Specifies the axis or axes of reflection. */
	DllExport void SetAxis(NURBSMirrorAxis axis);
	/*! \remarks Returns the axis or axes of reflection for the curve. */
	DllExport NURBSMirrorAxis GetAxis(void);
	/*! \remarks This is an additional transformation applied to the axis
	specification. This corresponds to the gizmo they user may use
	interactively to alter the location of the mirror axis. This is exactly
	equivalent to setting the transform on the gizmo of a mirror modifier.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the transformation.\n\n
	<b>Matrix3\& mat</b>\n\n
	The transformation to set. */
	DllExport void SetXForm(TimeValue t, Matrix3& mat);
	/*! \remarks Returns the additional transformation applied to the mirror
	axis at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the transformation matrix. */
	DllExport Matrix3& GetXForm(TimeValue t);
	/*! \remarks Sets the offset distance of the curve.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the distance.\n\n
	<b>double d</b>\n\n
	The distance to set. */
	DllExport void SetDistance(TimeValue t, double d);
	/*! \remarks Returns the offset distance of the curve.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the distance.
	\par Operators:
	*/
	DllExport double GetDistance(TimeValue t);
};


/*! \sa  Class NURBSCurve.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class defines a dependent fillet curve. A fillet is a curve that creates a
circular arc corner between two parent curves. Methods are available to get/set
the parent indices and ids, get/set the trim state of the curves, get/set which
ends of the curves are used, and get/set the radius of the fillet.\n\n
All methods of this class are implemented by the system. */
class NURBSFilletCurve : public NURBSCurve {
	friend class NURBSSet;
	NURBSId mParentId[2];
	int mParentIndex[2];
	BOOL mParentEnd[2];
	double mRadius;
    BOOL mTrimCurve[2];
    BOOL mFlipTrim[2];

	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	\code
	mType = kNFilletCurve;
	mpObject = NULL;
	mpNSet = NULL;
	mRadius = 10.0;
	for (int i = 0; i < 2; i++)
	{
		mParentId[i] = 0;
		mParentIndex[i] = -1;
		mParentEnd[i] = TRUE;
		mTrimCurve[i] = TRUE;
		mFlipTrim[i] = FALSE;
	}
	\endcode  */
	DllExport NURBSFilletCurve(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSFilletCurve(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSFilletCurve\& curve</b>\n\n
	The curve to assign. */
	DllExport NURBSFilletCurve & operator=(const NURBSFilletCurve& curve);
	/*! \remarks Sets the index in the <b>NURBSSet</b> of the specified parent
	object.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1.\n\n
	<b>int index</b>\n\n
	The index into the <b>NURBSSet</b> of the parent curve. */
	DllExport void SetParent(int pnum, int index);
	/*! \remarks Sets the NURBSId of the specified parent.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1.\n\n
	<b>NURBSId id</b>\n\n
	The id to set. */
	DllExport void SetParentId(int pnum, NURBSId id);
	/*! \remarks Returns the index in the NURBSSet of the specified parent
	object.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1. */
	DllExport int GetParent(int pnum);
	/*! \remarks Returns the <b>NURBSId</b> of the specified parent. Note that
	a <b>NURBSId</b> won't be valid until the object has been instantiated in
	the scene.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1. */
	DllExport NURBSId GetParentId(int pnum);
	/*! \remarks
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1.\n\n
	<b>BOOL end</b>\n\n
	TRUE to use the end of the curve; FALSE to use the beginning. */
	DllExport void SetEnd(int pnum, BOOL end);
	/*! \remarks Returns which end of the specified curve is used for the
	fillet. TRUE if the end of the curve is used; FALSE if the beginning is
	used.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent curve number: 0 or 1. */
	DllExport BOOL GetEnd(int pnum);
	/*! \remarks Sets the radius for the fillet at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the radius value.\n\n
	<b>double radius</b>\n\n
	The radius to set. */
	DllExport void SetRadius(TimeValue t, double radius);
	/*! \remarks Returns the radius of the fillet at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to return the fillet value. */
	DllExport double GetRadius(TimeValue t);

    /*! \remarks Determines if the specified curve is trimmed beyond the
    fillet. TRUE if the curve is trimmed; otherwise FALSE.
    \par Parameters:
    <b>int pnum</b>\n\n
    The parent curve number: 0 or 1. */
    DllExport BOOL GetTrimCurve(int pnum);
    /*! \remarks Sets if the specified curve is trimmed beyond the fillet.
    \par Parameters:
    <b>int pnum</b>\n\n
    The parent curve number: 0 or 1.\n\n
    <b>BOOL trim</b>\n\n
    TRUE to trim the curve beyond the fillet; otherwise FALSE. */
    DllExport void SetTrimCurve(int pnum, BOOL trim);
    /*! \remarks Returns the flip state for the specified curve.
    \par Parameters:
    <b>int pnum</b>\n\n
    The parent curve number: 0 or 1.
    \return  TRUE if flip is set; FALSE it not. */
    DllExport BOOL GetFlipTrim(int pnum);
    /*! \remarks Sets the flip state for the specified curve.
    \par Parameters:
    <b>int pnum</b>\n\n
    The parent curve number: 0 or 1.\n\n
    <b>BOOL flip</b>\n\n
    TRUE to flip; FALSE to not flip.
    \par Operators:
    */
    DllExport void SetFlipTrim(int pnum, BOOL flip);
};


/*! \sa  Class NURBSCurve.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class defines a dependent chamfer curve. A chamfer is a curve that creates
a straight line corner between two parent curves. Methods are availalble to
get/set the parent indices and ids, to get/set which ends of the curves are
used for the chamfer, get/set the trim settings for each curve, and to get/set
the length of the curve back from the selected end that represents the start of
the chamfer.\n\n
All methods of this class are implemented by the system. */
class NURBSChamferCurve : public NURBSCurve {
	friend class NURBSSet;
	NURBSId mParentId[2];
	int mParentIndex[2];
	BOOL mParentEnd[2];
	double mLength[2];
    BOOL mTrimCurve[2];
    BOOL mFlipTrim[2];

	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	\code
	mType = kNChamferCurve;
	mpObject = NULL;
	mpNSet = NULL;
	for (int i = 0; i < 2; i++)
	{
		mParentId[i] = 0;
		mParentIndex[i] = -1;
		mParentEnd[i] = TRUE;
		mTrimCurve[i] = TRUE;
		mFlipTrim[i] = FALSE;
		mLength[i] = 0.0;
	}
	\endcode  */
	DllExport NURBSChamferCurve(void);
	/*! \remarks Destructor. The cache is cleared. */
	DllExport virtual ~NURBSChamferCurve(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSChamferCurve\& curve</b>\n\n
	The curve to assign. */
	DllExport NURBSChamferCurve & operator=(const NURBSChamferCurve& curve);
	/*! \remarks Sets the index in the <b>NURBSSet</b> of the specified parent
	object.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1.\n\n
	<b>int index</b>\n\n
	The index into the <b>NURBSSet</b> of the parent object. */
	DllExport void SetParent(int pnum, int index);
	/*! \remarks Sets the <b>NURBSId</b> of the specified parent.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1.\n\n
	<b>NURBSId id</b>\n\n
	The id to set. */
	DllExport void SetParentId(int pnum, NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the specified
	parent object.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1. */
	DllExport int GetParent(int pnum);
	/*! \remarks Returns the <b>NURBSId</b> of the specified parent. Note that
	a <b>NURBSId</b> won't be valid until the object has been instantiated in
	the scene.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1. */
	DllExport NURBSId GetParentId(int pnum);
	/*! \remarks Sets which end of the specified curve is used for the
	chamfer.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1.\n\n
	<b>BOOL end</b>\n\n
	TRUE to use the end of the curve; FALSE to use the beginning. */
	DllExport void SetEnd(int pnum, BOOL end);
	/*! \remarks Returns which end of the specified curve is used for the
	chamfer. TRUE if the end of the curve is used; FALSE if the beginning is
	used.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent curve number: 0 or 1. */
	DllExport BOOL GetEnd(int pnum);
	/*! \remarks Sets the length for the specified parent curve back from the
	end that defines the beginning of the chamfer, at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the chamfer length.\n\n
	<b>int pnum</b>\n\n
	The parent curve number: 0 or 1.\n\n
	<b>double length</b>\n\n
	The chamfer length to set. */
	DllExport void SetLength(TimeValue t, int pnum, double length);
	/*! \remarks Returns the length of the chamfer at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to return the chamfer length.\n\n
	<b>int pnum</b>\n\n
	The parent curve number: 0 or 1. */
	DllExport double GetLength(TimeValue t, int pnum);

    /*! \remarks Determines if the specified curve is trimmed beyond the
    fillet. TRUE if the curve is trimmed; otherwise FALSE.
    \par Parameters:
    <b>int pnum</b>\n\n
    The parent curve number: 0 or 1. */
    DllExport BOOL GetTrimCurve(int pnum);
    /*! \remarks Sets if the specified curve is trimmed beyond the chamfer.
    \par Parameters:
    <b>int pnum</b>\n\n
    The parent curve number: 0 or 1.\n\n
    <b>BOOL trim</b>\n\n
    TRUE to trim the curve beyond the chamfer; otherwise FALSE. */
    DllExport void SetTrimCurve(int pnum, BOOL trim);
    /*! \remarks Returns the flip state for the specified curve.
    \par Parameters:
    <b>int pnum</b>\n\n
    The parent curve number: 0 or 1.
    \return  TRUE if flip is set; FALSE it not. */
    DllExport BOOL GetFlipTrim(int pnum);
    /*! \remarks Sets the flip state for the specified curve.
    \par Parameters:
    <b>int pnum</b>\n\n
    The parent curve number: 0 or 1.\n\n
    <b>BOOL flip</b>\n\n
    TRUE to flip; FALSE to not flip.
    \par Operators:
    */
    DllExport void SetFlipTrim(int pnum, BOOL flip);
};


/*! \sa  Class NURBSCurve.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class defines a dependent iso curve. U and V iso curves are dependent
curves created along lines of constant parameter value of a NURBS surface. Note
the difference between "Iso Lines", which are a display artifact, and "Iso
Curves" which are the dependent objects. There are methods available to get/set
the parent surface index and id, get/set the direction of the iso curve, and
get/set the parameter which determines the location on the surface the curve
matches.\n\n
All methods of this class are implemented by the system. */
class NURBSIsoCurve : public NURBSCurve {
	friend class NURBSSet;
	NURBSId mParentId;
	int mParentIndex;
	BOOL mIsU;  // false for V...
	double mParam;
	BOOL mTrim;
	BOOL mFlipTrim;
	Point2 mSeed;
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNIsoCurve;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mParentId = 0;</b>\n\n
	<b> mParentIndex = -1;</b>\n\n
	<b> mIsU = TRUE;</b>\n\n
	<b> mParam = 0.0;</b>\n\n
	<b> mTrim = FALSE;</b>\n\n
	<b> mFlipTrim = FALSE;</b>\n\n
	<b> mSeed = Point2(0.0, 0.0);</b> */
	DllExport NURBSIsoCurve(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSIsoCurve(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSIsoCurve\& curve</b>\n\n
	The curve to assign. */
	DllExport NURBSIsoCurve & operator=(const NURBSIsoCurve& curve);
	/*! \remarks Sets the index in the <b>NURBSSet</b> of the specified parent
	object.
	\par Parameters:
	<b>int index</b>\n\n
	The index into the <b>NURBSSet</b> of the parent surface. */
	DllExport void SetParent(int index);
	/*! \remarks Sets the <b>NURBSId</b> of the specified parent.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	The id to set. */
	DllExport void SetParentId(NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the parent
	object. */
	DllExport int GetParent(void);
	/*! \remarks Returns the <b>NURBSId</b> of the parent. Note that a
	<b>NURBSId</b> won't be valid until the object has been instantiated in the
	scene. */
	DllExport NURBSId GetParentId(void);
	/*! \remarks Sets the direction of the iso curve, either U or V.
	\par Parameters:
	<b>BOOL isU</b>\n\n
	TRUE for U; FALSE for V. */
	DllExport void SetDirection(BOOL isU);
	/*! \remarks Returns TRUE if the iso curve is in the U direction; FALSE
	for the V direction. */
	DllExport BOOL GetDirection(void);
	/*! \remarks Sets the parameter which determines where on the surface the
	iso curve is defined.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to evaluate the surface.\n\n
	<b>double p</b>\n\n
	The parameter defining the location on the surface. */
	DllExport void SetParam(TimeValue t, double p);
	/*! \remarks Returns the parameter which determines where on the surface
	the iso curve is defined.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to evaluate the surface. */
	DllExport double GetParam(TimeValue t);
	/*! \remarks Returns the state of the trim flag. When set the surface is
	trimmed against the curve. When not set, the surface isn't trimmed */
	DllExport BOOL GetTrim();
	/*! \remarks Sets the state of the trim flag. When set the surface is
	trimmed against the curve. When not set, the surface isn't trimmed
	\par Parameters:
	<b>BOOL trim</b>\n\n
	TRUE to trim; FALSE to not trim. */
	DllExport void SetTrim(BOOL trim);
	/*! \remarks Returns the state of the trim flip flag. When set this trims
	the surface in the opposite direction */
	DllExport BOOL GetFlipTrim();
	/*! \remarks Sets the state of the trim flip flag. When set this trims the
	surface in the opposite direction
	\par Parameters:
	<b>BOOL flip</b>\n\n
	TRUE to flip; FALSE to not flip. */
	DllExport void SetFlipTrim(BOOL flip);
	/*! \remarks Returns the UV location of the seed value on the curve. */
	DllExport Point2 GetSeed();
	/*! \remarks Sets the UV location of the seed value on the curve.
	\par Parameters:
	<b>Point2\& seed</b>\n\n
	The seed value to set.
	\par Operators:
	*/
	DllExport void SetSeed(Point2& seed);
};


/*! \sa  Class NURBSCurve.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class defines a dependent edge curve. There are methods available to
get/set the parent surface index and id, and get/set the parameter which
determines the location on the surface the curve matches.\n\n
All methods of this class are implemented by the system. */
class NURBSSurfaceEdgeCurve : public NURBSCurve {
	friend class NURBSSet;
	NURBSId mParentId;
	int mParentIndex;
	Point2 mSeed;
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructr. The data members are initialized as follows:\n\n
	<b> mType = kNSurfaceEdgeCurve;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mParentId = 0;</b>\n\n
	<b> mParentIndex = -1;</b>\n\n
	<b> mSeed = Point2(0.0, 0.0);</b> */
	DllExport NURBSSurfaceEdgeCurve(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSSurfaceEdgeCurve(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSSurfaceEdgeCurve\& curve</b>\n\n
	The curve to assign from. */
	DllExport NURBSSurfaceEdgeCurve & operator=(const NURBSSurfaceEdgeCurve& curve);
	/*! \remarks Sets the NURBSId of the specified parent.
	\par Parameters:
	<b>int index</b>\n\n
	The index into the <b>NURBSSet</b> of the parent surface. */
	DllExport void SetParent(int index);
	/*! \remarks Sets the <b>NURBSId</b> of the specified parent.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	The id to set. */
	DllExport void SetParentId(NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the parent
	object. */
	DllExport int GetParent(void);
	/*! \remarks Returns the <b>NURBSId</b> of the parent. Note that a
	<b>NURBSId</b> won't be valid until the object has been instantiated in the
	scene. */
	DllExport NURBSId GetParentId(void);
	/*! \remarks Returns the UV location of the seed value on the curve. */
	DllExport Point2 GetSeed();
	/*! \remarks Sets the UV location of the seed value on the curve.
	\par Parameters:
	<b>Point2\& seed</b>\n\n
	The seed value to set.
	\par Operators:
	*/
	DllExport void SetSeed(Point2& seed);
};


/*! \sa  Class NURBSCurve, Class Point2,  Class Point3.\n\n
\par Description:
This class is available in release 2.5 and later only.\n\n
This class provides access to the Vector Projected Curve. A Vector Projected
curve lies on a surface. This is almost the same as a Normal Projected curve,
except that the projection from the existing curve to the surface is in the
direction of a vector that you can control. Vector projected curves may be used
for trimming. */
class NURBSProjectVectorCurve : public NURBSCurve {
	friend class NURBSSet;
	// parent 0 should be the surface parent 1 should be the curve
	NURBSId mParentId[2];
	int mParentIndex[2];
	BOOL mTrim;
	BOOL mFlipTrim;
	Point2 mSeed;
	Point3 mPVec;
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNProjectVectorCurve;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mParentId[0] = mParentId[1] = 0;</b>\n\n
	<b> mParentIndex[0] = mParentIndex[1] = -1;</b>\n\n
	<b> mTrim = FALSE;</b>\n\n
	<b> mFlipTrim = FALSE;</b>\n\n
	<b> mSeed = Point2(0.0, 0.0);</b>\n\n
	<b> mPVec = Point3(0.0, 0.0, 1.0);</b> */
	DllExport NURBSProjectVectorCurve(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSProjectVectorCurve(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSProjectVectorCurve\& curve</b>\n\n
	The curve to assign. */
	DllExport NURBSProjectVectorCurve & operator=(const NURBSProjectVectorCurve& curve);
	/*! \remarks
	\par Parameters:
	<b>int pnum</b>\n\n
	Pass 0 for the surface and 1 for the curve.\n\n
	<b>int index</b>\n\n
	The index in the <b>NURBSSet</b> of the surface or curve. */
	DllExport void SetParent(int pnum, int index);
	/*! \remarks Sets the surface or curve used by specifying its
	<b>NURBSId</b>.
	\par Parameters:
	<b>int pnum</b>\n\n
	Pass 0 for the surface and 1 for the curve.\n\n
	<b>NURBSId id</b>\n\n
	The id of the surface or curve. */
	DllExport void SetParentId(int pnum, NURBSId id);
	/*! \remarks Returns the index into the <b>NURBSSet</b> of surface or
	curve used.
	\par Parameters:
	<b>int pnum</b>\n\n
	Pass 0 for the surface and 1 for the curve. */
	DllExport int GetParent(int pnum);
	/*! \remarks Returns the <b>NURBSId</b> of surface or curve used.
	\par Parameters:
	<b>int pnum</b>\n\n
	Pass 0 for the surface and 1 for the curve. */
	DllExport NURBSId GetParentId(int pnum);
	/*! \remarks Returns the state of the trim flag. When set the surface is
	trimmed against the curve. When not set, the surface isn't trimmed */
	DllExport BOOL GetTrim();
	/*! \remarks Sets the state of the trim flag. When set the surface is
	trimmed against the curve. When not set, the surface isn't trimmed
	\par Parameters:
	<b>BOOL trim</b>\n\n
	TRUE to trim; FALSE to not trim. */
	DllExport void SetTrim(BOOL trim);
	/*! \remarks Returns the state of the trim flip flag. When set this trims
	the surface in the opposite direction. */
	DllExport BOOL GetFlipTrim();
	/*! \remarks Sets the state of the trim flip flag. When set this trims the
	surface in the opposite direction
	\par Parameters:
	<b>BOOL flip</b>\n\n
	TRUE to flip; FALSE to not flip. */
	DllExport void SetFlipTrim(BOOL flip);
	/*! \remarks Returns the UV location of the seed value on the surface. */
	DllExport Point2 GetSeed();
	/*! \remarks Sets the UV location of the seed value on the surface. If
	there is a choice of projections, the projection closest to the seed point
	is the one used to create the curve.
	\par Parameters:
	<b>Point2\& seed</b>\n\n
	The seed value to set. */
	DllExport void SetSeed(Point2& seed);
	/*! \remarks Sets the projection vector used at the time passed.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the projection vector.\n\n
	<b>Point3\& pvec</b>\n\n
	The vector to set. */
	DllExport void SetPVec(TimeValue t, Point3& pvec); // projection vector
	/*! \remarks Returns the projection vector used at the time passed.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the projection vector. */
	DllExport Point3& GetPVec(TimeValue t);
};



/*! \sa  Class NURBSCurve, Class Point2.\n\n
\par Description:
This class is available in release 2.5 and later only.\n\n
This class provides access to the Normal Proejcted Curve. A Normal Projected
curve lies on a surface. It is based on an existing curve, which is projected
onto the surface in the direction of the surface's normals. */
class NURBSProjectNormalCurve : public NURBSCurve {
	friend class NURBSSet;
	// parent 0 should be the surface parent 1 should be the curve
	NURBSId mParentId[2];
	int mParentIndex[2];
	BOOL mTrim;
	BOOL mFlipTrim;
	Point2 mSeed;
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. */
	DllExport NURBSProjectNormalCurve(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSProjectNormalCurve(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSProjectNormalCurve\& curve</b>\n\n
	The curve to assign. */
	DllExport NURBSProjectNormalCurve & operator=(const NURBSProjectNormalCurve& curve);
	/*! \remarks Sets the surface or curve used by specifying its index into
	the NURBSSet.
	\par Parameters:
	<b>int pnum</b>\n\n
	Pass 0 for the surface and 1 for the curve.\n\n
	<b>int index</b>\n\n
	The index in the NURBSSet of the surface or curve. */
	DllExport void SetParent(int pnum, int index);
	/*! \remarks Sets the surface or curve used by specifying its NURBSId.
	\par Parameters:
	<b>int pnum</b>\n\n
	Pass 0 for the surface and 1 for the curve.\n\n
	<b>NURBSId id</b>\n\n
	The id of the surface or curve. */
	DllExport void SetParentId(int pnum, NURBSId id);
	/*! \remarks Returns the index in the NURBSSet of the surface or curve in
	use.
	\par Parameters:
	<b>int pnum</b>\n\n
	Pass 0 for the surface and 1 for the curve. */
	DllExport int GetParent(int pnum);
	/*! \remarks Returns the NURBSId of the surface or curve in use.
	\par Parameters:
	<b>int pnum</b>\n\n
	Pass 0 for the surface and 1 for the curve. */
	DllExport NURBSId GetParentId(int pnum);
	/*! \remarks Returns the state of the trim flag. When set the surface is
	trimmed against the curve. When not set, the surface isn't trimmed */
	DllExport BOOL GetTrim();
	/*! \remarks Sets the state of the trim flag. When set the surface is
	trimmed against the curve. When not set, the surface isn't trimmed
	\par Parameters:
	<b>BOOL trim</b>\n\n
	TRUE to trim; FALSE to not trim. */
	DllExport void SetTrim(BOOL trim);
	/*! \remarks Returns the state of the trim flip flag. When set this trims
	the surface in the opposite direction */
	DllExport BOOL GetFlipTrim();
	/*! \remarks Sets the state of the trim flip flag. When set this trims the
	surface in the opposite direction
	\par Parameters:
	<b>BOOL flip</b>\n\n
	TRUE to flip; FALSE to not flip. */
	DllExport void SetFlipTrim(BOOL flip);
	/*! \remarks Returns the UV location of the seed value on the surface. */
	DllExport Point2 GetSeed();
	/*! \remarks Sets the UV location of the seed value on the surface. If
	there is a choice of projections, the projection closest to the seed point
	is the one used to create the curve.
	\par Parameters:
	<b>Point2\& seed</b>\n\n
	The seed value to set. */
	DllExport void SetSeed(Point2& seed);
};


/*! \sa  Class NURBSCurve, Class Point2.\n\n
\par Description:
This class is available in release 2.5 and later only.\n\n
This class provides access to the Surface-Surface Intersection Curve. This is a
curve that is defined by the intersection of two surfaces. You can use
surface-surface intersection curves for trimming. */
class NURBSSurfSurfIntersectionCurve : public NURBSCurve {
	friend class NURBSSet;
	// parent 0 should be the surface parent 1 should be the curve
	NURBSId mParentId[2];
	int mParentIndex[2];
	BOOL mTrim[2];
	BOOL mFlipTrim[2];
	Point2 mSeed;
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNSurfSurfIntersectionCurve;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mParentId[0] = mParentId[1] = 0;</b>\n\n
	<b> mParentIndex[0] = mParentIndex[1] = -1;</b>\n\n
	<b> mTrim[0] = mTrim[1] = FALSE;</b>\n\n
	<b> mFlipTrim[0] = mFlipTrim[1] = FALSE;</b>\n\n
	<b> mSeed = Point2(0.0, 0.0);</b> */
	DllExport NURBSSurfSurfIntersectionCurve(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSSurfSurfIntersectionCurve(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSSurfSurfIntersectionCurve\& curve</b>\n\n
	The curve to assign. */
	DllExport NURBSSurfSurfIntersectionCurve & operator=(const NURBSSurfSurfIntersectionCurve& curve);
	/*! \remarks Sets the surface or curve by specifying its index into the
	NURBSSet.
	\par Parameters:
	<b>int pnum</b>\n\n
	Pass 0 for the surface and 1 for the curve.\n\n
	<b>int index</b>\n\n
	The index into the NURBSSet of the object to set. */
	DllExport void SetParent(int pnum, int index);
	/*! \remarks Sets the surface or curve by specifying its index into the
	NURBSSet.
	\par Parameters:
	<b>int pnum</b>\n\n
	Pass 0 for the surface and 1 for the curve.\n\n
	<b>NURBSId id</b>\n\n
	The id of the object to set. */
	DllExport void SetParentId(int pnum, NURBSId id);
	/*! \remarks Returns the index into the NURBSSet of the curve or surface.
	\par Parameters:
	<b>int pnum</b>\n\n
	Pass 0 for the surface and 1 for the curve. */
	DllExport int GetParent(int pnum);
	/*! \remarks Returns the NURBSId of the curve or surface.
	\par Parameters:
	<b>int pnum</b>\n\n
	Pass 0 for the surface and 1 for the curve. */
	DllExport NURBSId GetParentId(int pnum);
	/*! \remarks Returns the state of the specified trim flag. When on, the
	surface is trimmed against the intersection curve. When off, the surface
	isn't trimmed.
	\par Parameters:
	<b>int tnum</b>\n\n
	Passing 0 trims the first parent surface, and passing 1 trims the second
	parent surface */
	DllExport BOOL GetTrim(int tnum);
	/*! \remarks Sets the state of the specified trim flag. When on, the
	surface is trimmed against the intersection curve. When off, the surface
	isn't trimmed.
	\par Parameters:
	<b>int tnum</b>\n\n
	Passing 0 trims the first parent surface, and passing 1 trims the second
	parent surface\n\n
	<b>BOOL trim</b>\n\n
	TRUE to trim; FALSE to not trim. */
	DllExport void SetTrim(int tnum, BOOL trim);
	/*! \remarks Returns the state of the trim flip flag. When on, the
	specified surface is trimmed in the opposite direction.
	\par Parameters:
	<b>int tnum</b>\n\n
	0 for the first surface; 1 for the second surface. */
	DllExport BOOL GetFlipTrim(int tnum);
	/*! \remarks Sets the state of the trim flip flag. When on, the specified
	surface is trimmed in the opposite direction.
	\par Parameters:
	<b>int tnum</b>\n\n
	0 for the first surface; 1 for the second surface.\n\n
	<b>BOOL flip</b>\n\n
	TRUE to flip; FALSE not to flip. */
	DllExport void SetFlipTrim(int tnum, BOOL flip);
	/*! \remarks Returns the UV location of the seed value on the first
	surface. If there is a choice of intersections, the intersection closest to
	the seed point is the one used to create the curve. */
	DllExport Point2 GetSeed();
	/*! \remarks Change the UV location of the seed value on the first
	surface. If there is a choice of intersections, the intersection closest to
	the seed point is the one used to create the curve.
	\par Parameters:
	<b>Point2\& seed</b>\n\n
	The UV location. */
	DllExport void SetSeed(Point2& seed);
};



/*! \sa  Class NURBSCVCurve.\n\n
\par Description:
This class is available in release 2.5 and later only.\n\n
This class provides access to the CV curve on surface parameters. These curves
can be used for trimming the surface they lie on. */
class NURBSCurveOnSurface : public NURBSCVCurve {
	friend class NURBSSet;
	NURBSId mParentId;
	int mParentIndex;
	BOOL mTrim;
	BOOL mFlipTrim;
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNCurveOnSurface;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mParentId = 0;</b>\n\n
	<b> mParentIndex = -1;</b>\n\n
	<b> mTrim = FALSE;</b>\n\n
	<b> mFlipTrim = FALSE;</b> */
	DllExport NURBSCurveOnSurface(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSCurveOnSurface(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSCurveOnSurface\& curve</b>\n\n
	The curve to assign. */
	DllExport NURBSCurveOnSurface & operator=(const NURBSCurveOnSurface& curve);
	/*! \remarks Sets the index in the <b>NURBSSet</b> of the parent surface.
	\par Parameters:
	<b>int index</b>\n\n
	The index into the <b>NURBSSet</b> of the parent surface. */
	DllExport void SetParent(int index);
	/*! \remarks Sets the <b>NURBSId</b> of the parent.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	The id to set. */
	DllExport void SetParentId(NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the specified
	parent object. */
	DllExport int GetParent();
	/*! \remarks Returns the <b>NURBSId</b> of the parent. Note that a
	<b>NURBSId</b> won't be valid until the object has been instantiated in the
	scene. */
	DllExport NURBSId GetParentId();
	/*! \remarks Returns the state of the trim surface toggle. */
	DllExport BOOL GetTrim();
	/*! \remarks Sets the state of the trim surface toggle.
	\par Parameters:
	<b>BOOL trim</b>\n\n
	TRUE to trim the surface at the curve; FALSE to not trim. */
	DllExport void SetTrim(BOOL trim);
	/*! \remarks Returns the state of the trim flip toggle. */
	DllExport BOOL GetFlipTrim();
	/*! \remarks Sets the state of the trim flip toggle. This controls which
	portion of the surface is trimmed.
	\par Parameters:
	<b>BOOL flip</b>\n\n
	TRUE for on; FALSE for off. */
	DllExport void SetFlipTrim(BOOL flip);
};

/*! \sa  Class NURBSPointCurve.\n\n
\par Description:
This class is available in release 2.5 and later only.\n\n
This class provides access to the point curve on surface parameters. These
curves can be used for trimming the surface they lie on. */
class NURBSPointCurveOnSurface : public NURBSPointCurve {
	friend class NURBSSet;
	NURBSId mParentId;
	int mParentIndex;
	BOOL mTrim;
	BOOL mFlipTrim;
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNPointCurveOnSurface;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mParentId = 0;</b>\n\n
	<b> mParentIndex = -1;</b>\n\n
	<b> mTrim = FALSE;</b>\n\n
	<b> mFlipTrim = FALSE;</b> */
	DllExport NURBSPointCurveOnSurface(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSPointCurveOnSurface(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSPointCurveOnSurface\& curve</b>\n\n
	The curve to assign. */
	DllExport NURBSPointCurveOnSurface & operator=(const NURBSPointCurveOnSurface& curve);
	/*! \remarks Sets the index in the <b>NURBSSet</b> of the parent surface.
	\par Parameters:
	<b>int index</b>\n\n
	The index in the <b>NURBSSet</b> of the parent surface. */
	DllExport void SetParent(int index);
	/*! \remarks Sets the <b>NURBSId</b> of the parent.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	The id to set. */
	DllExport void SetParentId(NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the parent
	surface. */
	DllExport int GetParent();
	/*! \remarks Returns the <b>NURBSId</b> of the parent surface. Note that a
	NURBSId won't be valid until the object has been instantiated in the scene.
	*/
	DllExport NURBSId GetParentId();
	/*! \remarks Returns the state of the trim toggle. */
	DllExport BOOL GetTrim();
	DllExport void SetTrim(BOOL trim);
	/*! \remarks Returns the state of the trim flip toggle. */
	DllExport BOOL GetFlipTrim();
	/*! \remarks Sets the state of the trim flip toggle. This controls which
	portion of the parent surface is trimmed.
	\par Parameters:
	<b>BOOL flip</b>\n\n
	TRUE for flipped; FALSE for not. */
	DllExport void SetFlipTrim(BOOL flip);
};


/*! \sa  Class NURBSCurve.\n\n
\par Description:
This class is available in release 2.5 and later only.\n\n
This provides access to the Surface Normal Curve. This is a curve created at a
specified distance from a surface and normal to it.\n\n
Note: The parent curve specified below must have one of the following types:
surface-surface intersection, U Iso, V Iso, normal projected, vector projected,
CV curve on surface, or point curve on surface. */
class NURBSSurfaceNormalCurve : public NURBSCurve {
	friend class NURBSSet;
	NURBSId mParentId;
	int mParentIndex;
	double mDistance;
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNSurfaceNormalCurve;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mParentId = 0;</b>\n\n
	<b> mParentIndex = -1;</b> */
	DllExport NURBSSurfaceNormalCurve(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSSurfaceNormalCurve(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSSurfaceNormalCurve\& curve</b>\n\n
	The curve to assign. */
	DllExport NURBSSurfaceNormalCurve & operator=(const NURBSSurfaceNormalCurve& curve);
	/*! \remarks Establishes the parent curve to use by passing its index in
	the <b>NURBSSet</b>.
	\par Parameters:
	<b>int index</b>\n\n
	The index in the <b>NURBSSet</b> of the parent curve to use. */
	DllExport void SetParent(int index);
	/*! \remarks Establishes the parent curve to use by passing its
	<b>NURBSId</b>.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	The id of the parent curve to use. */
	DllExport void SetParentId(NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the parent curve.
	*/
	DllExport int GetParent();
	/*! \remarks Returns the <b>NURBSId</b> of the parent curve. */
	DllExport NURBSId GetParentId();
	/*! \remarks Set the distance along the normal of the curve from the
	surface at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the distance.\n\n
	<b>double dist</b>\n\n
	The distance along the normal from the surface to the curve. */
	DllExport void SetDistance(TimeValue t, double dist);
	/*! \remarks Returns the distance along the normal from the surface to the
	curve.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the distance. */
	DllExport double GetDistance(TimeValue t);
};

/*! \defgroup nurbTextureSurfaceTypes NURBSTextureSurface Types
Determines the type of texture surface generated. 
\sa Class NURBSTextureSurface, Class NURBSControlVertex, Class NURBSSurface. */
//@{
enum NURBSTexSurfType {
	/*! Automatically generates a texture surface. This method evenly distributes the texture, and attempts to compensate for
	stretching of the surface. */
	kNMapDefault,
	/*! Generates a texture surface that the user can edit. A user can edit the user-defined texture surface by
	editing texture points directly in the viewports. */
	kNMapUserDefined, 
	/*! Generates the texture surface by projecting the texture of another NURBS surface sub-object in the NURBS model. The projection travels along the direction of the normals of the source
	surface. Projected texture surfaces are relational. */
	kNMapSufaceMapper  
};
//@}

/*! \sa  Class NURBSControlVertex,  Class NURBSSurface,  Class NURBSTexturePoint.\n\n
\par Description:
This class is available in release 2.5 and later only.\n\n
A NURBS texture surface is a surface associated with the surface sub-object.
3ds Max uses the texture surface to control how materials are mapped. In
effect, changing the texture surface stretches or otherwise changes the UV
coordinates for the surface, altering the mapping.\n\n
This class provides access to the NURBS Texture Surface. This is a 2D (not 3D)
surface that lives in the parameter space of the corresponding
<b>NURBSSurface</b> which controls the texture mapping used by the
<b>NURBSSurface</b>.  */
class NURBSTextureSurface: public MaxHeapOperators {
	NURBSTexSurfType mMapperType;
	NURBSTexturePoint *mpPoints;
	int mNumUPoints;
	int mNumVPoints;

	NURBSId mParentId;
	int mParentIndex;
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mMapperType = kNMapDefault;</b>\n\n
	<b> mpPoints = NULL;</b> */
	DllExport NURBSTextureSurface(void);
	/*! \remarks Destructor. Any <b>mpPoints</b> are deleted. */
	DllExport ~NURBSTextureSurface(void);
	DllExport NURBSTextureSurface(NURBSTextureSurface& tsurf);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSTextureSurface\& surf</b>\n\n
	The surface to assign. */
	DllExport NURBSTextureSurface & operator=(const NURBSTextureSurface& surf);
	/*! \remarks	Returns the NURBS Texture Surface Type. See \ref nurbTextureSurfaceTypes. */
	DllExport NURBSTexSurfType MapperType();
	/*! \remarks	Sets the NURBS Texture Surface Type.
	\par Parameters:
	<b>NURBSTexSurfType type</b>\n\n
	The type to set. See \ref nurbTextureSurfaceTypes. */
	DllExport void SetMapperType(NURBSTexSurfType type);

	/*! \remarks	This sets the Id of the "source" surface for a <b>kNMapSurfaceMapper</b>
	texture surface, it should be NULL in other cases. This is only used if the
	<b>NURBSTexSurfType</b> is set the <b>kNMapSufaceMapper</b>. In that case
	the texture surface is generated by projecting the texture of another NURBS
	surface sub-object in the NURBS model. The projection travels along the
	direction of the normals of the source surface. Projected texture surfaces
	are relational. This method set the parent surface which is projected.
	\par Parameters:
	<b>int index</b>\n\n
	The index in the <b>NURBSSet</b> of the source surface. */
	DllExport void SetParent(int index);
	/*! \remarks	This sets the index in the NURBSSet of the "source" surface for a
	<b>kNMapSurfaceMapper</b> texture surface. This is only used if the
	<b>NURBSTexSurfType</b> is set the <b>kNMapSufaceMapper</b>. In that case
	the texture surface is generated by projecting the texture of another NURBS
	surface sub-object in the NURBS model. The projection travels along the
	direction of the normals of the source surface. Projected texture surfaces
	are relational. This method set the parent surface which is projected.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	The ID of the source surface. */
	DllExport void SetParentId(NURBSId id);
	/*! \remarks	Returns the index in the NURBSSet of the surface that's mapped. */
	DllExport int GetParent();
	/*! \remarks	Returns the ID of the surface that's mapped. */
	DllExport NURBSId GetParentId();

	/*! \remarks	Sets the number of points in U and V for the texture surface.
	\par Parameters:
	<b>int u</b>\n\n
	The number of points in U.\n\n
	<b>int v</b>\n\n
	The number of points in V. */
	DllExport void SetNumPoints(int u, int v);
	/*! \remarks	Returns the number of points in U. */
	DllExport int GetNumUPoints(void);
	/*! \remarks	Returns the number of points in V. */
	DllExport int GetNumVPoints(void);
	/*! \remarks	Retrieves the number of points in U and V.
	\par Parameters:
	<b>int \&u</b>\n\n
	The number of points in U is returned here.\n\n
	<b>int \&v</b>\n\n
	The number of points in V is returned here. */
	DllExport void GetNumPoints(int &u, int &v);
	/*! \remarks	Returns a pointer to the specified NURBS Texture Point.
	\par Parameters:
	<b>int u</b>\n\n
	Specifies the U point.\n\n
	<b>int v</b>\n\n
	Specifies the V point. */
	DllExport NURBSTexturePoint* GetPoint(int u, int v);
	/*! \remarks	Sets the specified NURBS Texture Point.
	\par Parameters:
	<b>int u</b>\n\n
	Specifies the U point.\n\n
	<b>int v</b>\n\n
	Specifies the V point.\n\n
	<b>NURBSTexturePoint\& pnt</b>\n\n
	The point to set. */
	DllExport void SetPoint(int u, int v, NURBSTexturePoint& pnt);
};



class NURBSTextureChannel;

/*! \sa  Class NURBSTextureChannel, NURBSSurface, Template Class Tab.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class holds a table of pointers to all the <b>NURBSTextureChannel</b> data
for a surface. There are methods to returns the table data by channel or by
index and a method to add a new texture channel.
\par Data Members:
private:\n\n
<b>Tab\<NURBSTextureChannel*\> mTextureChannels;</b>\n\n
A table of pointers to the texture channel data. <br>  private: */
class NURBSTextureChannelSet: public MaxHeapOperators {
	friend class NURBSSurface;
    ~NURBSTextureChannelSet();

private:
    /*! \remarks Returns a pointer to the texture channel object whose index
    is specified.
    \par Parameters:
    <b>int index</b>\n\n
    The zero based index into the table of texture channels in the set. */
    NURBSTextureChannel* GetChannelByIndex(int index) { return mTextureChannels[index]; }
    /*! \remarks Returns a pointer to the specific texture channel object. If
    not found a new channel is added with the specified index.
    \par Parameters:
    <b>int channel</b>\n\n
    The channel to get. This is a number in the range 0 and 98 which correspond
    to 1 to 99 in the user interface. */
    NURBSTextureChannel* GetChannel(int channel);
    /*! \remarks Adds the specified channel and returns a pointer to the
    allocated texture object.
    \par Parameters:
    <b>int channel</b>\n\n
    The texture channel. This is a number in the range 0 and 98 which
    correspond to 1 to 99 in the user interface. */
    NURBSTextureChannel* AddChannel(int channel);
    /*! \remarks Returns the number of channels in the set. */
    int NumChannels() { return mTextureChannels.Count(); }

    Tab<NURBSTextureChannel*> mTextureChannels;
};

/*! \sa  Class NURBSTextureSurface, Class NURBSTextureChannelSet,  Class NURBSSurface,  Class Point2.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class holds the data associated with a single texture channel. This
includes the <b>NURBSTextureSurface</b>, texture vertices, channel number,
tiling, offset, and rotation settings. The tiling, offset and angle values are
applied after the texture surface is applied. This gives the user more control
of the mapping.
\par Data Members:
private:\n\n
<b>int mChannel;</b>\n\n
The UV coordinates channel. This value can range from 0 to 98. A single surface
can use up to 99 texture channels.\n\n
<b>BOOL mGenUVs;</b>\n\n
TRUE if the generate UV setting is on for this channel; otherwise FALSE.\n\n
<b>Point2 mTexUVs[4];</b>\n\n
The texture vertices (UV coordinates).\n\n
<b>float mUTile;</b>\n\n
The U tiling.\n\n
<b>float mVTile;</b>\n\n
The V tiling.\n\n
<b>float mUOffset;</b>\n\n
The U offset.\n\n
<b>float mVOffset;</b>\n\n
The V offset.\n\n
<b>float mAngle;</b>\n\n
The rotation angle in radians.\n\n
<b>NURBSTextureSurface mTexSurface;</b>\n\n
The texture surface associated this channel. <br>  private: */
class NURBSTextureChannel: public MaxHeapOperators {
	friend class NURBSSurface;
	friend class NURBSTextureChannelSet;
private:
    /*! \remarks Constructor. The data members are initialized as follows:\n\n
    <b>mChannel = channel;</b>\n\n
    <b>mGenUVs = FALSE;</b>\n\n
    <b>mTexUVs[0] = Point2(0.0f, 0.0f);</b>\n\n
    <b>mTexUVs[1] = Point2(1.0f, 0.0f);</b>\n\n
    <b>mTexUVs[2] = Point2(0.0f, 1.0f);</b>\n\n
    <b>mTexUVs[3] = Point2(1.0f, 1.0f);</b>\n\n
    <b>mUTile = 1.0f;</b>\n\n
    <b>mVTile = 1.0f;</b>\n\n
    <b>mUOffset = 0.0f;</b>\n\n
    <b>mVOffset = 0.0f;</b>\n\n
    <b>mAngle = 0.0f;</b> */
    DllExport NURBSTextureChannel(int channel);

    /*! \remarks Returns the UV coordinate channel. */
    DllExport int GetChannel() { return mChannel; }

    /*! \remarks Returns TRUE if the generate UVs state is on; otherwise
    FALSE. */
    DllExport BOOL GenerateUVs();
	/*! \remarks Sets the generate UV state.
	\par Parameters:
	<b>BOOL state</b>\n\n
	TRUE for on; FALSE for off. */
	DllExport void SetGenerateUVs(BOOL state);
	/*! \remarks Returns the 'i-th' texture corner at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to return the texture vertex.\n\n
	<b>int i</b>\n\n
	The index of the texture vertex. This is a value in the range 0 to 3. */
	DllExport Point2 GetTextureUVs(TimeValue t, int i);
	/*! \remarks Sets the specified texture coordinate.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the texture UV.\n\n
	<b>int i</b>\n\n
	The index of the UV to set. This is a value in the range 0 to 3.\n\n
	<b>Point2 pt</b>\n\n
	The UV point to set. */
	DllExport void SetTextureUVs(TimeValue t, int i, Point2 pt);
	/*! \remarks Retrieves the tiling, offset and angle values at the
	specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the values.\n\n
	<b>float \&ut</b>\n\n
	The U tiling.\n\n
	<b>float \&vt</b>\n\n
	The V tiling.\n\n
	<b>float \&uo</b>\n\n
	The U offset.\n\n
	<b>float \&vo</b>\n\n
	The V offset.\n\n
	<b>float \&a</b>\n\n
	The angle. */
	DllExport void GetTileOffset(TimeValue t, float &ut, float &vt, float &uo, float &vo, float &a);
	/*! \remarks Sets the tiling, offset and angle values at the specified
	time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the values.\n\n
	<b>float ut</b>\n\n
	The U tiling.\n\n
	<b>float vt</b>\n\n
	The V tiling.\n\n
	<b>float uo</b>\n\n
	The U offset.\n\n
	<b>float vo</b>\n\n
	The V offset.\n\n
	<b>float a</b>\n\n
	The angle. */
	DllExport void SetTileOffset(TimeValue t, float ut, float vt, float uo, float vo, float a);

    /*! \remarks Returns a reference to the <b>NURBSTextureSurface</b>
    maintained by the class. */
    DllExport NURBSTextureSurface& GetTextureSurface() {return mTexSurface; }
    /*! \remarks Set the <b>NURBSTextureSurface</b> maintained by the class.
    \par Parameters:
    <b>NURBSTextureSurface\& texSurf</b>\n\n
    The texture surface to set. */
    DllExport void SetTextureSurface(NURBSTextureSurface& texSurf);

    int mChannel;

	BOOL mGenUVs;
	Point2 mTexUVs[4];

	float mUTile;
    float mVTile;
    float mUOffset;
    float mVOffset;
    float mAngle;

	NURBSTextureSurface mTexSurface;
};

/*! \sa  Class NURBSObject, Class NURBSTextureSurface, Template Class Tab, Class NURBSTextureChannelSet.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class describes the properties of a NURBS surface. This includes its
material ID, texture/tiling options, renderable state, open/closed state, and
normal inverted state. The <b>Evaluate()</b> method is used to compute points
and tangents on the surface.\n\n
All methods of this class are implemented by the system.
\par Data Members:
protected:\n\n
<b>BOOL mGenUVs;</b>\n\n
TRUE if the 'Generate Mapping Coordinates' checkbox is on; otherwise FALSE.\n\n
<b>BOOL mFlipNormals;</b>\n\n
TRUE if the surface normals are inverted; otherwise FALSE.\n\n
<b>BOOL mRenderable;</b>\n\n
TRUE if the surface may be rendered; otherwise FALSE.\n\n
<b>int mMatID;</b>\n\n
The material ID for the surface.\n\n
<b>BOOL mClosedInU, mClosedInV;</b>\n\n
The surface closed flags.\n\n
<b>NURBSTextureChannelSet mTextureChannelSet;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
This is the set of texture mapping channels used by this surface.\n\n
<b>BOOL mFlipNormals;</b>\n\n
TRUE if normals are flipped for the surface; otherwise FALSE.\n\n
<b>BOOL mRenderable;</b>\n\n
TRUE if the surface is renderable; otherwise FALSE.\n\n
<b>int mMatID;</b>\n\n
The zero based material ID for the surface.\n\n
<b>BOOL mClosedInU</b>\n\n
TRUE if the surface is closed in U; otherwise FALSE.\n\n
<b>BOOL mClosedInV;</b>\n\n
TRUE if the surface is closed in V; otherwise FALSE.\n\n
<b>TessApprox *mpVTess;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
Points to the TessApprox object for the viewport (optional).\n\n
<b>TessApprox *mpRTess;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
Points to the TessApprox object for the production renderer (optional).\n\n
<b>TessApprox *mpRTessDisp;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
Points to the TessApprox object for displacement mapping (optional).\n\n
<b>TessApprox *mpVTessCurve;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
Points to the TessApprox object for tesselating curves in the
viewports(optional).\n\n
<b>TessApprox *mpRTessCurve;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
Points to the TessApprox object for tesselating curves in the production
renderer (optional).  */
class NURBSSurface : public NURBSObject {
	friend class NURBSCVSurface;
	friend class NURBSPointSurface;
	friend class NURBSBlendSurface;
	friend class NURBSNBlendSurface;
	friend class NURBSOffsetSurface;
	friend class NURBSXFormSurface;
	friend class NURBSMirrorSurface;
	friend class NURBSCapSurface;
	friend class NURBSIsoCurve;
	friend class NURBSProjectVectorCurve;
	friend class NURBSProjectNormalCurve;
	friend class NURBSSurfSurfIntersectionCurve;
	friend class NURBSCurveOnSurface;
	friend class NURBSPointCurveOnSurface;
	friend class NURBSMultiCurveTrimSurface;
	friend class NURBSTextureChannel;
	friend class NURBSTextureChannelSet;

private:
	DllExport NURBSTextureChannel* GetChannel(int index);

protected:
    NURBSTextureChannelSet mTextureChannelSet;

	BOOL mFlipNormals;
	BOOL mRenderable;
	int mMatID;
	BOOL mClosedInU, mClosedInV;

	// new for R3 -- optional
	TessApprox *mpVTess;
	TessApprox *mpRTess;
	TessApprox *mpRTessDisp;
	TessApprox *mpVTessCurve;
	TessApprox *mpRTessCurve;

    // Internal surface cache
    // 
    void* mpSurfCache;

public:

    // Internal use only cache
    void* GetSurfCache() { return mpSurfCache; }
    void SetSurfCache(void* pCache) { mpSurfCache = pCache; }

	/*! \remarks Constructor. The surface is initialized as follows:\n\n
	<b> mKind = kNURBSSurface;</b>\n\n
	<b> mFlipNormals = FALSE;</b>\n\n
	<b> mRenderable = TRUE;</b>\n\n
	<b> mMatID = 1;</b>\n\n
	<b> mpVTess = NULL;</b>\n\n
	<b> mpRTess = NULL;</b>\n\n
	<b> mpRTessDisp = NULL;</b>\n\n
	<b> mpVTessCurve = NULL;</b>\n\n
	<b> mpRTessCurve = NULL;</b> */
	DllExport NURBSSurface(void);
	/*! \remarks Destructor. Any <b>TessApprox</b> objects are freed. */
	DllExport virtual ~NURBSSurface(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSSurface\& curve</b>\n\n
	The surface to assign. */
	DllExport NURBSSurface & operator=(const NURBSSurface& surf);
	/*! \remarks Returns TRUE if the surface is renderable; otherwise FALSE.
	*/
	DllExport BOOL Renderable();
	/*! \remarks Sets the renderable flag to the specified state.
	\par Parameters:
	<b>BOOL state</b>\n\n
	TRUE for renderable; FALSE for non-renderable. */
	DllExport void Renderable(BOOL state);
	/*! \remarks Returns the state of the flip normals flag. */
	DllExport BOOL FlipNormals();
	/*! \remarks Set the state of the flip normals flag.
	\par Parameters:
	<b>BOOL state</b>\n\n
	TRUE if the normals should be flipped; FALSE for not flipped. */
	DllExport void FlipNormals(BOOL state);
	/*! \remarks Returns TRUE if the generate UV mapping coordinates flag is
	set for the specified channel; otherwise FALSE.
	\par Parameters:
	<b>int channel = 0</b>\n\n
	The channel. This is a number in the range 0 and 98 which correspond to 1
	to 99 in the user interface. */
	DllExport BOOL GenerateUVs(int channel = 0);
	/*! \remarks Sets the state of the generate UV mapping coordinates flag.
	\par Parameters:
	<b>BOOL state</b>\n\n
	TRUE for on; FALSE for off.\n\n
	<b>int channel = 0</b>\n\n
	The channel. This is a number in the range 0 and 98 which correspond to 1
	to 99 in the user interface. */
	DllExport void SetGenerateUVs(BOOL state, int channel = 0);
	/*! \remarks Returns the material ID. */
	DllExport int MatID();
	/*! \remarks Sets the mateials ID to the specified value.
	\par Parameters:
	<b>int id</b>\n\n
	Specifies the material ID to set. */
	DllExport void MatID(int id);
	/*! \remarks Returns the specified texture coordinate.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the texture UVs.\n\n
	<b>int i</b>\n\n
	The zero based index of the coordinate to return. This value must be \>=0
	and \< 4.\n\n
	<b>int channel = 0</b>\n\n
	The channel. This is a number in the range 0 and 98 which correspond to 1
	to 99 in the user interface. */
	DllExport Point2 GetTextureUVs(TimeValue t, int i, int channel = 0);
	/*! \remarks Set the specified texture coordinate to the specified value.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to set the texture UVs.\n\n
	<b>int i</b>\n\n
	The zero based index of the texture coordinate to set. This value must be
	\>= 0 and \< 4.\n\n
	<b>Point2 pt</b>\n\n
	The texture coordinate value to set.\n\n
	<b>int channel = 0</b>\n\n
	The channel. This is a number in the range 0 and 98 which correspond to 1
	to 99 in the user interface. */
	DllExport void SetTextureUVs(TimeValue t, int i, Point2 pt, int channel = 0);
	/*! \remarks Retrieves the texture tiling and offset values for the surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the tile offset.\n\n
	<b>float \&ut</b>\n\n
	The tiling value in the U direction.\n\n
	<b>float \&vt</b>\n\n
	The tiling value in the V direction.\n\n
	<b>float \&uo</b>\n\n
	The offset value in the U direction.\n\n
	<b>float \&vo</b>\n\n
	The offset value in the V direction.\n\n
	<b>int channel = 0</b>\n\n
	The channel. This is a number in the range 0 and 98 which correspond to 1 to 99
	in the user interface. */
	DllExport void GetTileOffset(TimeValue t, float &ut, float &vt, float &uo, float &vo, float &a, int channel = 0);
	/*! \remarks Sets the texture tiling and offset values for the surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to set the tile offset.\n\n
	<b>float ut</b>\n\n
	The tiling value in the U direction.\n\n
	<b>float vt</b>\n\n
	The tiling value in the V direction.\n\n
	<b>float uo</b>\n\n
	The offset value in the U direction.\n\n
	<b>float vo</b>\n\n
	The offset value in the V direction.\n\n
	<b>int channel = 0</b>\n\n
	The channel. This is a number in the range 0 and 98 which correspond to 1 to 99
	in the user interface. */
	DllExport void SetTileOffset(TimeValue t, float ut, float vt, float uo, float vo, float a, int channel = 0);

    /*! \remarks Returns a reference to the texture surface used by this surface for the
    specified channel.
    \par Parameters:
    <b>int channel</b>\n\n
    The channel. This is a number in the range 0 and 98 which correspond to 1
    to 99 in the user interface. */
    DllExport NURBSTextureSurface& GetTextureSurface(int channel);
    /*! \remarks Sets the texture surface used by this surface for the specified channel.
    \par Parameters:
    <b>int channel</b>\n\n
    The channel. This is a number in the range 0 and 98 which correspond to 1
    to 99 in the user interface.\n\n
    <b>NURBSTextureSurface\& texSurf</b>\n\n
    The texture surface to set. */
    DllExport void SetTextureSurface(int channel, NURBSTextureSurface& texSurf);

    /*! \remarks Returns the number of channels used by the surface. */
    DllExport int NumChannels();
	/*! \remarks	Returns a channel number corresponding to the specified index into the
	<b>NURBSTextureChannelSet</b>.
	\par Parameters:
	<b>int index</b>\n\n
	The zero based index into the <b>NURBSTextureChannelSet</b>. */
	DllExport int GetChannelFromIndex(int index);



	/*! \remarks Returns TRUE if the surface is closed in the U direction;
	otherwise FALSE. */
	DllExport BOOL IsClosedInU(void);
	/*! \remarks Returns TRUE if the surface is closed in the V direction;
	otherwise FALSE. */
	DllExport BOOL IsClosedInV(void);

	/*! \remarks Retrieves the point on the surface, and the u and v
	derivatives based on the parameters u and v.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to evaluate the surface.\n\n
	<b>double u</b>\n\n
	The value at which to evaluate the surface in u. This value must be between
	the <b>uMin</b> and <b>uMax</b> as returned from
	<b>GetParameterRange()</b>.\n\n
	<b>double v</b>\n\n
	The value at which to evaluate the surface in v. This value must be between
	the <b>vMin</b> and <b>vMax</b> as returned from
	<b>GetParameterRange()</b>.\n\n
	<b>Point3\& pt</b>\n\n
	The point on the surface.\n\n
	<b>Point3\& dPdU</b>\n\n
	The derivative along u.\n\n
	<b>Point3\& dPdV</b>\n\n
	The derivative along v.
	\return  TRUE if the method was able to evaluate the surface; otherwise
	FALSE. */
	DllExport BOOL Evaluate(TimeValue t, double u, double v, Point3& pt,
			Point3& dPdU, Point3& dPdV);
	/*! \remarks Retrieves the point on the surface, and the u and v
	derivatives and second derivatives based on the parameters u and v.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to evaluate the surface.\n\n
	<b>double u</b>\n\n
	The value at which to evaluate the surface in u. This value must be between
	the <b>uMin</b> and <b>uMax</b> as returned from
	<b>GetParameterRange()</b>.\n\n
	<b>double v</b>\n\n
	The value at which to evaluate the surface in v. This value must be between
	the <b>vMin</b> and <b>vMax</b> as returned from
	<b>GetParameterRange()</b>.\n\n
	<b>Point3\& pt</b>\n\n
	The point on the surface.\n\n
	<b>Point3\& dPdU</b>\n\n
	The derivative along u.\n\n
	<b>Point3\& dPdV</b>\n\n
	The derivative along v.\n\n
	<b>Point3\& d2PdU2</b>\n\n
	The second derivative along u.\n\n
	<b>Point3\& d2PdV2</b>\n\n
	The second derivative along v.\n\n
	<b>Point3\& d2PdUdV</b>
	\return  TRUE if the method was able to evaluate the surface; otherwise
	FALSE. */
	DllExport BOOL Evaluate(TimeValue t, double u, double v, Point3& pt,
			Point3& dPdU, Point3& dPdV, Point3& d2PdU2, Point3& d2PdV2, Point3& d2PdUdV);
	/*! \remarks Retrieves the minimum and maximum valid values for <b>u</b>
	and <b>v</b> as passed to <b>Evaluate()</b>.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the parameter range.\n\n
	<b>double\& uMin</b>\n\n
	The minimum value in u is returned here.\n\n
	<b>double\& uMax</b>\n\n
	The maximum value in v is returned here.\n\n
	<b>double\& vMin</b>\n\n
	The minimum value in u is returned here.\n\n
	<b>double\& vMax</b>\n\n
	The maximum value in v is returned here. */
	DllExport void GetParameterRange(TimeValue t, double& uMin, double& uMax, double& vMin, double& vMax);
	/*! \remarks	Retrieves data about the NURBSSurface at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the NURBS information.\n\n
	<b>int\& degreeInU</b>\n\n
	The degree of the surface in U.\n\n
	<b>int\& degreeInV</b>\n\n
	The degree of the surface in V.\n\n
	<b>int\& numInU</b>\n\n
	The number of CVs in U.\n\n
	<b>int\& numInV</b>\n\n
	The number of CVs in V.\n\n
	<b>NURBSCVTab\& cvs</b>\n\n
	The table of CVs. Note: <b>typedef Tab\<NURBSControlVertex\>
	NURBSCVTab;</b>\n\n
	<b>int\& numKnotsInU</b>\n\n
	The number of knots in U.\n\n
	<b>int\& numKnotsInV</b>\n\n
	The number of knots in V.\n\n
	<b>NURBSKnotTab uKnots</b>\n\n
	A table of knots in U. Note: <b>typedef Tab\<double\> NURBSKnotTab;</b>\n\n
	<b>NURBSKnotTab vKnots</b>\n\n
	A table of knots in V.
	\return  TRUE if the data was retrieved; otherwise FALSE. */
	DllExport BOOL GetNURBSData(TimeValue t,
								int& degreeInU,
								int& degreeInV,
								int& numInU,
								int& numInV,
								NURBSCVTab& cvs,
								int& numKnotsInU,
								int& numKnotsInV,
								NURBSKnotTab& uKnots, 
								NURBSKnotTab& vKnots);
	/*! \remarks	This method retrieves the Chord Length Parameterization Texture Surface data.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to retrieve the data.\n\n
	<b>int channel</b>\n\n
	The texture channel. This is a number in the range 0 and 98 which correspond to
	1 to 99 in the user interface.\n\n
	<b>int\& degreeInU</b>\n\n
	The degree of the surface in U.\n\n
	<b>int\& degreeInV</b>\n\n
	The degree in V.\n\n
	<b>int\& numInU</b>\n\n
	The number of CVs in U.\n\n
	<b>int\& numInV</b>\n\n
	The number of CVs in V.\n\n
	<b>NURBSCVTab\& cvs</b>\n\n
	The table of CVs. Note: <b>typedef Tab\<NURBSControlVertex\>
	NURBSCVTab;</b>\n\n
	<b>int\& numKnotsInU</b>\n\n
	The number of knots in U.\n\n
	<b>int\& numKnotsInV</b>\n\n
	The number of knots in V.\n\n
	<b>NURBSKnotTab uKnots</b>\n\n
	A table of knots in U. Note: <b>typedef Tab\<double\> NURBSKnotTab;</b>\n\n
	<b>NURBSKnotTab vKnots</b>\n\n
	A table of knots in V.
	\return  TRUE if the data was retrieved; otherwise FALSE. */
	DllExport BOOL GetCLPTextureSurfaceData(TimeValue t, int channel,
								int& degreeInU,
								int& degreeInV,
								int& numInU,
								int& numInV,
								NURBSCVTab& cvs,
								int& numKnotsInU,
								int& numKnotsInV,
								NURBSKnotTab& uKnots, 
								NURBSKnotTab& vKnots);
	/*! \remarks	Returns the number of trim loops at the specified time. Each loop may be
	made up of several curves.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to return the number. */
	DllExport int NumTrimLoops(TimeValue t);
	/*! \remarks	Returns the number of curves in the specified trim loop.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to return the number.\n\n
	<b>int loop</b>\n\n
	The zero based index of the trim loop. */
	DllExport int NumCurvesInLoop(TimeValue t, int loop);
	/*! \remarks	Retrieves data about the specified 2D trim curve in use by the surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to retrieve the data.\n\n
	<b>int loop</b>\n\n
	The zero based index of the trim loop.\n\n
	<b>int curve</b>\n\n
	The zero based index of the trim curve within the loop.\n\n
	<b>int\& degree</b>\n\n
	The degree of the curve is returned here.\n\n
	<b>int\& numCVs</b>\n\n
	The number of CVs.\n\n
	<b>NURBSCVTab\& cvs</b>\n\n
	The table of CVs. Note: <b>typedef Tab\<NURBSControlVertex\>
	NURBSCVTab;</b>\n\n
	<b>int\& numKnots</b>\n\n
	The number of knots.\n\n
	<b>NURBSKnotTab knots</b>\n\n
	A table of knots. Note: <b>typedef Tab\<double\> NURBSKnotTab;</b>
	\return  TRUE if the data was retrieved; otherwise FALSE. */
	DllExport BOOL Get2dTrimCurveData(TimeValue t, int loop, int curve,
                                        int& degree,
                                        int& numCVs,
                                        NURBSCVTab& cvs,
                                        int& numKnots,
                                        NURBSKnotTab& knots);
	/*! \remarks	Retrieves data about the specified 2D trim curve in use by the surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to retrieve the data.\n\n
	<b>int loop</b>\n\n
	The zero based index of the trim loop.\n\n
	<b>int curve</b>\n\n
	The zero based index of the trim curve within the loop.\n\n
	<b>int\& degree</b>\n\n
	The degree of the curve is returned here.\n\n
	<b>int\& numCVs</b>\n\n
	The number of CVs.\n\n
	<b>NURBSCVTab\& cvs</b>\n\n
	The table of CVs. Note: <b>typedef Tab\<NURBSControlVertex\>
	NURBSCVTab;</b>\n\n
	<b>int\& numKnots</b>\n\n
	The number of knots.\n\n
	<b>NURBSKnotTab knots</b>\n\n
	A table of knots. Note: <b>typedef Tab\<double\> NURBSKnotTab;</b>
	\return  TRUE if the data was retrieved; otherwise FALSE. */
	DllExport BOOL Get3dTrimCurveData(TimeValue t, int loop, int curve,
										int& degree,
										int& numCVs,
										NURBSCVTab& cvs,
										int& numKnots,
										NURBSKnotTab& knots);
	/*! \remarks	Returns a pointer to the TessApprox object used for production rendering of
	the specified type.
	\par Parameters:
	<b>NURBSTessType type=kNTessSurface</b>\n\n
	The tesselation type. See \ref nurbTessTypes. */
	DllExport TessApprox* GetProdTess(NURBSTessType type=kNTessSurface);
	/*! \remarks	Returns a pointer to the TessApprox object used for viewport rendering of
	the specified type.
	\par Parameters:
	<b>NURBSTessType type=kNTessSurface</b>\n\n
	The tesselation type. See \ref nurbTessTypes. */
	DllExport TessApprox* GetViewTess(NURBSTessType type=kNTessSurface);
	/*! \remarks	Sets the TessApprox object used for production rendering of the specified
	type.
	\par Parameters:
	<b>TessApprox\& tess</b>\n\n
	The object to set.\n\n
	<b>NURBSTessType type=kNTessSurface</b>\n\n
	The tesselation type. See \ref nurbTessTypes. */
	DllExport void SetProdTess(TessApprox& tess, NURBSTessType type=kNTessSurface);
	/*! \remarks	Sets the TessApprox object used for viewport rendering of the specified
	type.
	\par Parameters:
	<b>TessApprox\& tess</b>\n\n
	The object to set.\n\n
	<b>NURBSTessType type=kNTessSurface</b>\n\n
	The tesselation type. See \ref nurbTessTypes. */
	DllExport void SetViewTess(TessApprox& tess, NURBSTessType type=kNTessSurface);
	/*! \remarks	Clears (deletes) the TessApprox object used for viewport rendering of the
	specified type.
	\par Parameters:
	<b>NURBSTessType type=kNTessSurface</b>\n\n
	The tesselation type. See \ref nurbTessTypes. */
	DllExport void ClearViewTess(NURBSTessType type=kNTessSurface);
	/*! \remarks	Clears (deletes) the TessApprox object used for production rendering of the
	specified type.
	\par Parameters:
	<b>NURBSTessType type=kNTessSurface</b>\n\n
	The tesselation type. See \ref nurbTessTypes.
	\par Operators:
	*/
	DllExport void ClearProdTess(NURBSTessType type=kNTessSurface);
};


/*! \sa  Class NURBSSurface.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class defines a surface that uses control vertices (CVs) to describe its
shape. The CVs define a control lattice which surrounds the surface. This class
has methods to close the surface in U and V, set its order in U and V, set the
number of knots and CVs in U and V, and get/set the knots and CVs in U and V.
There is also a method to add additional CVs to the surface. The CV surface has
a transformation matrix used to position the surface within a NURBSSet.\n\n
All methods of this class are implemented by the system.  */
class NURBSCVSurface : public NURBSSurface {
	friend class NURBSSet;
	NURBSControlVertex *mpCVs;
	double *mpUKnots;
	double *mpVKnots;
	int mUOrder;
	int mVOrder;
	int mNumUCVs;
	int mNumVCVs;
	int mNumUKnots;
	int mNumVKnots;
	BOOL mRigid;
	NURBSAutoParam mAutoParam;
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNCVSurface;</b>\n\n
	<b> mRigid = FALSE;</b>\n\n
	<b> mClosedInU = FALSE;</b>\n\n
	<b> mClosedInV = FALSE;</b>\n\n
	<b> mpCVs = NULL;</b>\n\n
	<b> mpUKnots = NULL;</b>\n\n
	<b> mpVKnots = NULL;</b>\n\n
	<b> mNumUCVs = 0;</b>\n\n
	<b> mNumVCVs = 0;</b>\n\n
	<b> mNumUKnots = 0;</b>\n\n
	<b> mNumVKnots = 0;</b>\n\n
	<b> mUOrder = 0;</b>\n\n
	<b> mVOrder = 0;</b>\n\n
	<b> mAutoParam = kNotAutomatic;</b> */
	DllExport NURBSCVSurface(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSCVSurface(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSCVSurface\& surf</b>\n\n
	The CV surface to assign. */
	DllExport NURBSCVSurface & operator=(const NURBSCVSurface& surf);
	/*! \remarks	Returns TRUE if the surface is 'rigid'; otherwise FALSE. */
	DllExport BOOL IsRigid();
	/*! \remarks	Sets the 'rigid' state of the surface.
	\par Parameters:
	<b>BOOL isRigid</b>\n\n
	TRUE for on; FALSE for off. */
	DllExport void SetRigid(BOOL isRigid);
	/*! \remarks	Returns the automatic parameterization setting. See \ref nurbAutoParamTypes. */
	DllExport NURBSAutoParam AutoParam();
	/*! \remarks	Sets the automatic parameterization setting at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the parameterization.\n\n
	<b>NURBSAutoParam param</b>\n\n
	See \ref nurbAutoParamTypes. */
	DllExport void AutoParam(TimeValue t, NURBSAutoParam param);
	/*! \remarks	Sets the reparameterization type at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to sets the reparameterization type.\n\n
	<b>NURBSParamaterization param</b>\n\n
	See nurbParameterizationTypes. */
	DllExport void Reparameterize(TimeValue t, NURBSParamaterization param);
	/*! \remarks This method closes the surface in the U direction. The aligns
	the surface edge to edge in U and sets the tangents to match. */
	DllExport void CloseInU(void);
	/*! \remarks This method closes the surface in the V direction. The aligns
	the surface edge to edge in V and sets the tangents to match. */
	DllExport void CloseInV(void);
	/*! \remarks Sets the order of the surface in the U direction.
	\par Parameters:
	<b>int order</b>\n\n
	Specifies the order of the surface in the U direction. */
	DllExport void SetUOrder(int order);
	/*! \remarks Returns the order of the surface in the U direction. */
	DllExport int GetUOrder(void);
	/*! \remarks Returns the order of the surface in the V direction. */
	DllExport int GetVOrder(void);
	/*! \remarks Sets the order of the surface in the V direction.
	\par Parameters:
	<b>int order</b>\n\n
	Specifies the order of the surface in the V direction. */
	DllExport void SetVOrder(int order);
	/*! \remarks Sets the number of knots in the U direction. Note that the
	knot data is not maintained.
	\par Parameters:
	<b>int num</b>\n\n
	Specifies the number of knots in the U direction. */
	DllExport void SetNumUKnots(int num);         // data is NOT maintained
	/*! \remarks Sets the number of knots in the V direction. Note that the
	knot data is not maintained.
	\par Parameters:
	<b>int num</b>\n\n
	Specifies the number of knots in the V direction. */
	DllExport void SetNumVKnots(int num);         // data is NOT maintained
	/*! \remarks Returns the number of knots in the U direction. */
	DllExport int GetNumUKnots(void);
	/*! \remarks Returns the number of knots in the V direction. */
	DllExport int GetNumVKnots(void);
	/*! \remarks Sets the number of control vertices in both the U and V
	directions. Note that the CV data is not maintained.
	\par Parameters:
	<b>int u</b>\n\n
	Specifies the number of control vertices in the U direction.\n\n
	<b>int v</b>\n\n
	Specifies the number of control vertices in the V direction. */
	DllExport void SetNumCVs(int u, int v);       // data is NOT maintained
	/*! \remarks Returns the number of control vertices in the U direction. */
	DllExport int GetNumUCVs(void);
	/*! \remarks Returns the number of control vertices in the V direction. */
	DllExport int GetNumVCVs(void);
	/*! \remarks Returns the number of control vertices in both the U and V
	directions.
	\par Parameters:
	<b>int \&u</b>\n\n
	The number of CVs in the U direction is returned here.\n\n
	<b>int \&v</b>\n\n
	The number of CVs in the V direction is returned here. */
	DllExport void GetNumCVs(int &u, int &v);
	/*! \remarks Returns the specified knot value in the U direction.
	\par Parameters:
	<b>int index</b>\n\n
	The 0 based index of the knot value to return. */
	DllExport double GetUKnot(int index);
	/*! \remarks Returns the specified knot value in the V direction.
	\par Parameters:
	<b>int index</b>\n\n
	The 0 based index of the knot value to return. */
	DllExport double GetVKnot(int index);
	/*! \remarks Sets the specified knot in the U direction to the specified
	value.
	\par Parameters:
	<b>int index</b>\n\n
	The 0 based index of the knot value to set.\n\n
	<b>double value</b>\n\n
	The value to set. */
	DllExport void SetUKnot(int index, double value);
	/*! \remarks Sets the specified knot in the U direction to the specified
	value.
	\par Parameters:
	<b>int index</b>\n\n
	The 0 based index of the knot value to set.\n\n
	<b>double value</b>\n\n
	The value to set. */
	DllExport void SetVKnot(int index, double value);
	/*! \remarks Returns the specified control vertex of this surface.
	\par Parameters:
	<b>int u</b>\n\n
	The 0 based index in the U direction.\n\n
	<b>int v</b>\n\n
	The 0 based index in the V direction. */
	DllExport NURBSControlVertex* GetCV(int u, int v);
	/*! \remarks Sets the specified control vertex.
	\par Parameters:
	<b>int u</b>\n\n
	The 0 based index in the U direction.\n\n
	<b>int v</b>\n\n
	The 0 based index in the V direction.\n\n
	<b>NURBSControlVertex \&cv</b>\n\n
	The control vertex to set. */
	DllExport void SetCV(int u, int v, NURBSControlVertex& cv);

	/*! \remarks Sets the transformation matrix for the <b>NURBSCVSurface</b>.
	This matrix controls the relative position of the surface within a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the matrix.\n\n
	<b>SetXFormPacket\& xPack</b>\n\n
	An instance of the <b>XFormPacket</b> class that describes the properties
	of the transformation (specifically if it's being moved, rotated, or
	scaled). See Class SetXFormPacket. */
	DllExport void SetTransformMatrix(TimeValue t, SetXFormPacket& mat);
	/*! \remarks Returns the transformation matrix of the
	<b>NURBSCVSurface</b> at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to retrieve the matrix. */
	DllExport Matrix3 GetTransformMatrix(TimeValue t);

	/*! \remarks This method determines if the edges of the surface overlap in
	U and/or V even though the surface may not be closed (that is, the tangents
	match at the edges).
	\par Parameters:
	<b>BOOL\& uOverlap</b>\n\n
	The U result is returned here: TRUE if the edges overlap in U; otherwise
	FALSE.\n\n
	<b>BOOL\& vOverlap</b>\n\n
	The V result is returned here: TRUE if the edges overlap in V; otherwise
	FALSE. */
	DllExport void EdgesOverlap(BOOL& uOverlap, BOOL& vOverlap);
	// If you refine in U (U_V_Both = 0) you must specify v
	// If you refine in V (U_V_Both = 1) you must specify u
	// If you refine in U and (U_V_Both = -1) you must specify u and v
	/*! \remarks This method adds a control vertex at the specified point on
	the surface without changing the shape of the surface. The other CV points
	will move to maintain the current shape. The point may be specified as a U
	value or a V value, or both.\n\n
	If you refine in U (U_V_Both = 0) you must specify v\n\n
	If you refine in V (U_V_Both = 1) you must specify u\n\n
	If you refine in U and V (U_V_Both = -1) you must specify u and v
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to refine the surface.\n\n
	<b>double u</b>\n\n
	The position for the point in U space.\n\n
	<b>double v</b>\n\n
	The position for the point in V space.\n\n
	<b>int U_V_Both</b>\n\n
	This value must be <b>0</b>, <b>1</b> or <b>-1</b>.\n\n
	If 0 the refinement is done in u (and v is specified).\n\n
	If 1 the refinement is done in v (and u is specified).\n\n
	If -1 the refinement is done in both u and v (and both u and v must be
	specified). */
	DllExport void Refine(TimeValue t, double u, double v, int U_V_Both);
	/*! \remarks	This method adds a new CV to the surface and changes its shape. The other
	CVs will not move as they do in <b>Refine()</b>. This method preserves any
	existing animation of the surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to refine the surface.\n\n
	<b>double u</b>\n\n
	The position for the point in U space.\n\n
	<b>double v</b>\n\n
	The position for the point in V space.\n\n
	<b>int U_V_Both</b>\n\n
	This value must be <b>0</b>, <b>1</b> or <b>-1</b>.\n\n
	If 0 the refinement is done in u (and v is specified).\n\n
	If 1 the refinement is done in v (and u is specified).\n\n
	If -1 the refinement is done in both u and v (and both u and v must be
	specified).
	\par Operators:
	*/
	DllExport void Insert(TimeValue t, double u, double v, int U_V_Both);
};


/*! \sa  Class NURBSSurface, Class NURBSIndependentPoint.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class defines a surface that uses points to describe its shape. This class
has methods to close the surface in U and V, set the number of points in U and
V, and get/set the points in U and V. There is also a method to add additional
points to the surface. The point surface has a transformation matrix used to
set the relative position of the surface in a <b>NURBSSet</b>.\n\n
All methods of this class are implemented by the system. */
class NURBSPointSurface : public NURBSSurface {
	friend class NURBSSet;
	NURBSIndependentPoint *mpPts;
	int mNumUPts;
	int mNumVPts;
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNPointSurface;</b>\n\n
	<b> mClosedInU = FALSE;</b>\n\n
	<b> mClosedInV = FALSE;</b>\n\n
	<b> mpPts = NULL;</b> */
	DllExport NURBSPointSurface(void);
	/*! \remarks Destructor. Any allocated points are deleted. */
	DllExport virtual ~NURBSPointSurface(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSPointSurface\& surf</b>\n\n
	The surface to assign. */
	DllExport NURBSPointSurface & operator=(const NURBSPointSurface& surf);
	/*! \remarks This method closes the surface in the U direction. */
	DllExport void CloseInU(void);
	/*! \remarks This method closes the surface in the V direction. */
	DllExport void CloseInV(void);
	/*! \remarks Sets the number of points in the surface in the U and V
	directions. Any previously allocated points are not maintained when the new
	number is set. Note, if any of the u and v parameters are 1 or less, the 
	method will return and do nothing.
	\param u The number of points in U. Valid values are 2 and higher.
	\param v The number of points in V. Valid values are 2 and higher. */
	DllExport void SetNumPts(int u, int v);       // data is NOT maintained
	/*! \remarks Returns the number of points in the U direction. */
	DllExport int GetNumUPts(void);
	/*! \remarks Returns the number of points in the V direction. */
	DllExport int GetNumVPts(void);
	/*! \remarks Retrieves the number of points in both the U and V
	directions.
	\par Parameters:
	<b>int \&u</b>\n\n
	The number in U is stored here.\n\n
	<b>int \&v</b>\n\n
	The number in V is stored here. */
	DllExport void GetNumPts(int &u, int &v);
	/*! \remarks Returns a pointer to the specified point in the surface.
	\par Parameters:
	<b>int u</b>\n\n
	The zero based index of the point in the U direction.\n\n
	<b>int v</b>\n\n
	The zero based index of the point in the V direction. */
	DllExport NURBSIndependentPoint* GetPoint(int u, int v);
	/*! \remarks Sets the specified point in the surface to the point passed.
	\par Parameters:
	<b>int u</b>\n\n
	The zero based index of the point in the U direction.\n\n
	<b>int v</b>\n\n
	The zero based index of the point in the V direction.\n\n
	<b>NURBSIndependentPoint\& pt</b>\n\n
	The point to set. */
	DllExport void SetPoint(int u, int v, NURBSIndependentPoint& pt);

	/*! \remarks Sets the transformation matrix for the
	<b>NURBSPointSurface</b>. This controls the relative position of the
	surface within a <b>NURBSSet</b>.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the matrix.\n\n
	<b>SetXFormPacket\& xPack</b>\n\n
	An instance of the <b>XFormPacket</b> class that describes the properties
	of the transformation. See Class SetXFormPacket. */
	DllExport void SetTransformMatrix(TimeValue t, SetXFormPacket& mat);
	/*! \remarks Returns the transformation matrix of the
	<b>NURBSPointSurface</b> at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to retrieve the matrix. */
	DllExport Matrix3 GetTransformMatrix(TimeValue t);

	// If you refine in U (U_V_Both = 0) you must specify v
	// If you refine in V (U_V_Both = 1) you must specify u
	// If you refine in U and (U_V_Both = -1) you must specify u and v
	/*! \remarks This method adds a new point at the specified location on the
	surface without changing the shape of the surface. The location may be
	specified as a U value or a V value, or both.\n\n
	If you refine in U (U_V_Both = 0) you must specify v\n\n
	If you refine in V (U_V_Both = 1) you must specify u\n\n
	If you refine in U and V (U_V_Both = -1) you must specify u and v
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to refine the surface.\n\n
	<b>double u</b>\n\n
	The location for the point in U space (range 0.0 to 1.0).\n\n
	<b>double v</b>\n\n
	The location for the point in V space (range 0.0 to 1.0).\n\n
	<b>int U_V_Both</b>\n\n
	This value must be <b>0</b>, <b>1</b> or <b>-1</b>.\n\n
	If 0 the refinement is done in u (and v is specified).\n\n
	If 1 the refinement is done in v (and u is specified).\n\n
	If -1 the refinement is done in both u and v (and both u and v must be
	specified).
	\par Operators:
	*/
	DllExport void Refine(TimeValue t, double u, double v, int U_V_Both);
};

/*! \sa  Class NURBSSurface.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class defines a dependent blend surface. A blend surface connects the edge
of one surface to the edge of another, blending the curvature of the parents to
create a smooth surface between them. Methods are available to get/set the
parents, parent Ids, tension parameters and surface normal matching state.\n\n
All methods of this class are implemented by the system. */
class NURBSBlendSurface : public NURBSSurface {
	friend class NURBSSet;
	NURBSId mParentId[2];
	int mParentIndex[2];
	int mParentEdge[2];
	BOOL mFlip[2];
	double mTension[2];
	double mCurveStartParam[2];
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	\code
	mType = kNBlendSurface;
	mpObject = NULL;
	mpNSet = NULL;
	for (int i = 0; i < 2; i++)
	{
		mParentId[i] = 0;
		mParentIndex[i] = -1;
		mParentEdge[i] = 0;
		mFlip[i] = FALSE;
		mTension[i] = 1.0;
		mCurveStartParam[i] = 0.0;
	}
	\endcode  */
	DllExport NURBSBlendSurface(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSBlendSurface(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSBlendSurface\& surf</b>\n\n
	The surface to assign. */
	DllExport NURBSBlendSurface & operator=(const NURBSBlendSurface& surf);
	/*! \remarks Sets the index in the <b>NURBSSet</b> of the specified parent
	object.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1.\n\n
	<b>int index</b>\n\n
	The index into the <b>NURBSSet</b> of the parent surface. */
	DllExport void SetParent(int pnum, int index);
	/*! \remarks Sets the NURBSId of the specified parent.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1.\n\n
	<b>NURBSId id</b>\n\n
	The id to set. */
	DllExport void SetParentId(int pnum, NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the specified
	parent object.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1. */
	DllExport int GetParent(int pnum);
	/*! \remarks Returns the <b>NURBSId</b> of the specified parent. Note that
	a <b>NURBSId</b> won't be valid until the object has been instantiated in
	the scene.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1. */
	DllExport NURBSId GetParentId(int pnum);
	/*! \remarks Sets which edge of the specified surface is used for the
	blend.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1.\n\n
	<b>int edge</b>\n\n
	One of the following values:\n\n
	<b>0</b>: The low U edge.\n\n
	<b>1</b>: The high U edge.\n\n
	<b>2</b>: The low V edge.\n\n
	<b>3</b>: The high V edge. */
	DllExport void SetEdge(int pnum, int edge);
	/*! \remarks Returns an integer that determines which edge of the
	specified surface is used for the blend.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1.
	\return  One of the following values:\n\n
	<b>0</b>: The low U edge.\n\n
	<b>1</b>: The high U edge.\n\n
	<b>2</b>: The low V edge.\n\n
	<b>3</b>: The high V edge. */
	DllExport int GetEdge(int pnum);
	/*! \remarks Sets the tension value for the specified parent surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the tension value.\n\n
	<b>int pnum</b>\n\n
	The parent number: 0 or 1.\n\n
	<b>double ten</b>\n\n
	The tension value to set. */
	DllExport void SetTension(TimeValue t, int pnum, double ten);
	/*! \remarks Returns the tension value for the specified parent surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to return the tension value.\n\n
	<b>int pnum</b>\n\n
	The parent number: 0 or 1. */
	DllExport double GetTension(TimeValue t, int pnum);
	/*! \remarks This allows one to control the matching of parent surface
	normals when creating the blend surface. For example, normally when you
	create a blend surface between two parent surfaces you don't want a 'bow
	tie' surface (one with the ends rotated 180 degrees so it crosses on itself
	in the middle). If you simply match the parent normals you'll occasionally
	get a 'bow tie' surface. To prevent this you use this method to set a state
	indicating that one or the other should be flipped before it's used. In
	this way, when the blend is created, a 'bow tie' won't occur.
	\par Parameters:
	<b>int pnum</b>\n\n
	The number of the parent surface: 0 or 1.\n\n
	<b>BOOL flip</b>\n\n
	TRUE to match the parent surface normal; FALSE to not match it. */
	DllExport void SetFlip(int pnum, BOOL flip);
	/*! \remarks Returns the flip state of the specified parent surface.
	\par Parameters:
	<b>int pnum</b>\n\n
	The number of the parent surface: 0 or 1. */
	DllExport BOOL GetFlip(int pnum);

	// only if the parent is a closed curve
	/*! \remarks	Sets the start point for the specified parent curve. Note: This is only
	applicable if the parent is a closed curve.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the start point.\n\n
	<b>int pnum</b>\n\n
	The number of the parent surface: 0 or 1.\n\n
	<b>double startpoint</b>\n\n
	The start point in the range 0.0 to 1.0. */
	DllExport void SetCurveStartPoint(TimeValue t, int pnum, double startpoint);
	/*! \remarks	Returns the start point of the specified parent curve.\n\n
	Note: This is only applicable if the parent is a closed curve.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the start point.\n\n
	<b>int pnum</b>\n\n
	The number of the parent surface: 0 or 1.
	\par Operators:
	*/
	DllExport double GetCurveStartPoint(TimeValue t, int pnum);
};


/*! \sa  Class NURBSSurface.\n\n
\par Description:
This class is available in release 2.5 and later only.\n\n
This class provides access to the Multisided Blend surface. A Multisided Blend
surface is a surface that "fills in" the edges defined by three or four other
curve or surfaces. Unliked a regular, two-sided Blend surface, the curves or
surfaces edges must form a closed loop--that is, they must completely surround
the opening the Multisided Blend will cover.\n\n
Note: For the blend to work, the curves that define the blend must form a loop
(that is, sequence head to tail, and the ends must match). <br>  protected: */
class NURBSNBlendSurface : public NURBSSurface {
	friend class NURBSSet;
	// The parents can be either curves or surfaces (with edge IDs)
	NURBSId mParentId[4];
	int mParentIndex[4];
	int mParentEdge[4];  // used only if the parent is a surface
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	\code
	mType = kNNBlendSurface;
	mpObject = NULL;
	mpNSet = NULL;
	for (int i = 0; i < 4; i++)
	{
		mParentId[i] = 0;
		mParentIndex[i] = -1;
		mParentEdge[i] = 0;
	}
	\endcode  */
	DllExport NURBSNBlendSurface(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSNBlendSurface(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSNBlendSurface\& surf</b>\n\n
	The surface to assign. */
	DllExport NURBSNBlendSurface & operator=(const NURBSNBlendSurface& surf);
	/*! \remarks Sets the specified parent curve or surface (by NURBSSet
	index) as the speciifed edge for the surface.
	\par Parameters:
	<b>int pnum</b>\n\n
	The index of the parent curve or surface to set.\n\n
	<b>int index</b>\n\n
	The index in the <b>NURBSSet</b> of the curve or surface. */
	DllExport void SetParent(int pnum, int index);
	/*! \remarks Sets the specified parent curve or surface (by NURBSId) as
	the speciifed edge for the surface.
	\par Parameters:
	<b>int pnum</b>\n\n
	The index of the parent curve or surface to set.\n\n
	<b>NURBSId id</b>\n\n
	The id of the curve or surface to set. */
	DllExport void SetParentId(int pnum, NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the specified
	curve or surface.
	\par Parameters:
	<b>int pnum</b>\n\n
	The index of the parent curve or surface to get. */
	DllExport int GetParent(int pnum);
	/*! \remarks Returns the <b>NURBSId</b> of the specified curve or surface.
	\par Parameters:
	<b>int pnum</b>\n\n
	The index of the parent curve or surface to get. */
	DllExport NURBSId GetParentId(int pnum);
	/*! \remarks If using a surface for the blend surface edge, this method
	indicates which edge on the surface to use.
	\par Parameters:
	<b>int pnum</b>\n\n
	The index of the parent surface.\n\n
	<b>int edge</b>\n\n
	The edge to use for the blend. One of the following values:\n\n
	<b>0</b>: The low U edge.\n\n
	<b>1</b>: The high U edge.\n\n
	<b>2</b>: The low V edge.\n\n
	<b>3</b>: The high V edge. */
	DllExport void SetEdge(int pnum, int edge);
	/*! \remarks Returns the edge used by the specified surface to create the
	blend.
	\par Parameters:
	<b>int pnum</b>\n\n
	The index of the parent surface.
	\return  The edge used for the blend. One of the following values:\n\n
	<b>0</b>: The low U edge.\n\n
	<b>1</b>: The high U edge.\n\n
	<b>2</b>: The low V edge.\n\n
	<b>3</b>: The high V edge. */
	DllExport int GetEdge(int pnum);
};

/*! \sa  Class NURBSSurface.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class defines a dependent offset surface. An Offset surface is offset a
specified distance from the original along the parent surface's normals.
Methods are available to get/set the parent index and parent Ids, and to
get/set the offset distance of the surface.\n\n
All methods of this class are implemented by the system. */
class NURBSOffsetSurface : public NURBSSurface {
	friend class NURBSSet;
	NURBSId mParentId;
	int mParentIndex;
	double mDistance;
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNOffsetSurface;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mParentId = 0;</b>\n\n
	<b> mParentIndex = -1;</b>\n\n
	<b> mDistance = 0.0;</b> */
	DllExport NURBSOffsetSurface(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSOffsetSurface(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSOffsetSurface\& surf</b>\n\n
	The surface to assign. */
	DllExport NURBSOffsetSurface & operator=(const NURBSOffsetSurface& surf);
	/*! \remarks Sets the index in the <b>NURBSSet</b> of the specified parent
	object.
	\par Parameters:
	<b>int index</b>\n\n
	The index into the <b>NURBSSet</b> of the parent surface. */
	DllExport void SetParent(int index);
	/*! \remarks Sets the Id of the parent surface.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	The Id to set. */
	DllExport void SetParentId(NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the parent
	surface. */
	DllExport int GetParent(void);
	/*! \remarks Returns the <b>NURBSId</b> of the parent surface. Note that a
	<b>NURBSId</b> won't be valid until the object has been instantiated in the
	scene. */
	DllExport NURBSId GetParentId(void);
	/*! \remarks Sets the offset distance of the surface at the specified
	time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the offset a value.\n\n
	<b>double d</b>\n\n
	A distance of the offset in 3ds Max units. */
	DllExport void SetDistance(TimeValue t, double d);
	/*! \remarks Returns the distance of the offset at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to return the offset a value.
	\par Operators:
	*/
	DllExport double GetDistance(TimeValue t);
};

/*! \sa  Class NURBSSurface.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class defines a dependent transform (xform) surface. A transform surface
is a copy of the original surface with a different position, rotation, or
scale. Methods are available to get/set the indices of the parent surface in
the <b>NURBSSet</b> and the parent Ids and to specify/retrieve the
transformation used on the surface.\n\n
All methods of this class are implemented by the system. */
class NURBSXFormSurface : public NURBSSurface {
	friend class NURBSSet;
	NURBSId mParentId;
	int mParentIndex;
	Matrix3 mXForm;
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNXFormSurface;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mParentId = 0;</b>\n\n
	<b> mParentIndex = -1;</b>\n\n
	<b> mXForm.IdentityMatrix();</b> */
	DllExport NURBSXFormSurface(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSXFormSurface(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSXFormSurface\& surf</b>\n\n
	The surface to assign. */
	DllExport NURBSXFormSurface & operator=(const NURBSXFormSurface& surf);
	/*! \remarks Sets the index in the NURBSSet of the parent object.
	\par Parameters:
	<b>int index</b>\n\n
	The index in the NURBSSet of the parent object. */
	DllExport void SetParent(int index);
	/*! \remarks Set the NURBSId of the parent object.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	The Id to set. */
	DllExport void SetParentId(NURBSId id);
	/*! \remarks Returns the index in the NURBSSet of the parent object. */
	DllExport int GetParent(void);
	/*! \remarks Returns the NURBSId of the parent object. */
	DllExport NURBSId GetParentId(void);
	/*! \remarks Sets the transformation from the parent surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the transformation.\n\n
	<b>Matrix3\& mat</b>\n\n
	The transformation to set. */
	DllExport void SetXForm(TimeValue t, Matrix3& mat);
	/*! \remarks Returns the transformation from the parent surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which the transformation is returned.
	\par Operators:
	*/
	DllExport Matrix3& GetXForm(TimeValue t);
};

/*! \sa  Class NURBSSurface, \ref nurbMirrorAxisTypes.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class defines a dependent mirror surface. A mirror surface is similar to a
mirror object that you create using the Mirror tool (on the 3ds Max toolbar) or
the Mirror modifier. It is the original surface relfected about one or two
axes. Methods are available to get/set the indices of the parent surface in the
<b>NURBSSet</b> and the parent Ids, to get/set the mirror axes, to get/set the
mirror distance, and to set the transformation used to position the surface in
the <b>NURBSSet</b>.\n\n
All methods of this class are implemented by the system. */
class NURBSMirrorSurface : public NURBSSurface {
	friend class NURBSSet;
	NURBSId mParentId;
	int mParentIndex;
	NURBSMirrorAxis mAxis;
	Matrix3 mXForm;
	double mDistance;
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNMirrorSurface;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mParentId = 0;</b>\n\n
	<b> mParentIndex = -1;</b>\n\n
	<b> mXForm.IdentityMatrix();</b>\n\n
	<b> mAxis = kMirrorX;</b>\n\n
	<b> mDistance = 0.0;</b> */
	DllExport NURBSMirrorSurface(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSMirrorSurface(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSMirrorSurface\& surf</b>\n\n
	The surface to assign. */
	DllExport NURBSMirrorSurface & operator=(const NURBSMirrorSurface& surf);
	/*! \remarks Sets the index in the <b>NURBSSet</b> of the parent object.
	\par Parameters:
	<b>int index</b>\n\n
	The index in the <b>NURBSSet</b> of the parent object */
	DllExport void SetParent(int index);
	/*! \remarks Sets the NURBSId of the parent object.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	The NURBSId of the parent object. */
	DllExport void SetParentId(NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the parent
	object. */
	DllExport int GetParent(void);
	/*! \remarks Returns the <b>NURBSId</b> of the parent. Note that a
	<b>NURBSId</b> won't be valid until the object has been instantiated in the
	scene. */
	DllExport NURBSId GetParentId(void);
	/*! \remarks Sets the mirror axis to the specified constant.
	\par Parameters:
	<b>NURBSMirrorAxis axis</b>\n\n
	The mirror axis to set. */
	DllExport void SetAxis(NURBSMirrorAxis axis);
	/*! \remarks Returns the axis or axes of reflection for the surface. */
	DllExport NURBSMirrorAxis GetAxis(void);
	/*! \remarks This is an additional transformation applied to the axis
	specification. This corresponds to the gizmo the user may use interactively
	to alter the location of the mirror axis. This is exactly equivalent to
	setting the transform on the gizmo of a mirror modifier.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the transformation.\n\n
	<b>Matrix3\& mat</b>\n\n
	The transformation to set. */
	DllExport void SetXForm(TimeValue t, Matrix3& mat);
	/*! \remarks Returns the additional transformation applied to the mirror
	axis at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the transformation matrix. */
	DllExport Matrix3& GetXForm(TimeValue t);
	/*! \remarks This is just like the offset parameter in the mirror
	modifier. It is an offset from the center of the local coordinate system
	for the mirror object that moves the mirror, in the direction specified by
	the mirror axis.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the offset.\n\n
	<b>double d</b>\n\n
	The offset distance. */
	DllExport void SetDistance(TimeValue t, double d);
	/*! \remarks Returns the offset distance of the mirror at the specified
	time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to return the offset.
	\par Operators:
	*/
	DllExport double GetDistance(TimeValue t);
};

/*! \sa  Class NURBSSurface.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class defines a dependent ruled surface. A ruled surface is generated from
two curve sub-objects. It lets you use curves to design the two opposite
borders of a surface. Methods are available to get/set the indices of the
parent surface in the <b>NURBSSet</b> and the parent Ids and to set the surface
normal matching state of the surfaces.\n\n
All methods of this class are implemented by the system. */
class NURBSRuledSurface : public NURBSSurface {
	friend class NURBSSet;
	NURBSId mParentId[2];
	int mParentIndex[2];
	BOOL mFlip[2];
	double mCurveStartParam[2];
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	\code
	mType = kNRuledSurface;
	mpObject = NULL;
	mpNSet = NULL;
	for (int i = 0; i < 2; i++)
	{
		mParentId[i] = 0;
		mParentIndex[i] = -1;
		mFlip[i] = FALSE;
		mCurveStartParam[i] = 0.0;
	}
	\endcode  */
	DllExport NURBSRuledSurface(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSRuledSurface(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSRuledSurface\& surf</b>\n\n
	The surface to assign. */
	DllExport NURBSRuledSurface & operator=(const NURBSRuledSurface& surf);
	/*! \remarks Sets the index in the <b>NURBSSet</b> of the specified parent
	object.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1.\n\n
	<b>int index</b>\n\n
	The index in the NURBSSet of the specified parent object. */
	DllExport void SetParent(int pnum, int index);
	/*! \remarks Sets the <b>NURBSId</b> of the specified parent.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1.\n\n
	<b>NURBSId id</b>\n\n
	The id to set. */
	DllExport void SetParentId(int pnum, NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the specified
	parent object.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1. */
	DllExport int GetParent(int pnum);
	/*! \remarks Returns the <b>NURBSId</b> of the specified parent. Note that
	a <b>NURBSId</b> won't be valid until the object has been instantiated in
	the scene
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number: 0 or 1. */
	DllExport NURBSId GetParentId(int pnum);
	/*! \remarks This allows one to control the matching of parent surface
	normals when creating the ruled surface. For example, normally when you
	create a ruled surface between two parent curves you don't want a 'bow tie'
	surface (one with the ends rotated 180 degrees so it crosses on itself in
	the middle). If you simply match the parent normals you'll get a 'bow tie'
	surface. To prevent this you use this method to set a state indicating that
	one or the other should be flipped before it's used. In this way, when the
	ruled surface is created, a 'bow tie' won't occur.
	\par Parameters:
	<b>int pnum</b>\n\n
	The number of the parent curve: 0 or 1.\n\n
	<b>BOOL flip</b>\n\n
	TRUE to match the parent surface normal; FALSE to not match it. */
	DllExport void SetFlip(int pnum, BOOL flip);
	/*! \remarks Returns the flip state of the specified parent curve.
	\par Parameters:
	<b>int pnum</b>\n\n
	The number of the parent curve: 0 or 1. */
	DllExport BOOL GetFlip(int pnum);
	/*! \remarks	Sets the start point for the specified parent curve. Note: This is only
	applicable if the parent is a closed curve.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the start point.\n\n
	<b>int pnum</b>\n\n
	The number of the parent curve: 0 or 1.\n\n
	<b>double startpoint</b>\n\n
	The start point in the range 0.0 to 1.0. */
	DllExport void SetCurveStartPoint(TimeValue t, int pnum, double startpoint);
	/*! \remarks	Returns the start point for the specified parent curve. Note: This is only
	applicable if the parent is a closed curve.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the start point.\n\n
	<b>int pnum</b>\n\n
	The number of the parent curve: 0 or 1.
	\par Operators:
	*/
	DllExport double GetCurveStartPoint(TimeValue t, int pnum);
};


/*! \sa  Class NURBSSurface.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class defines a dependent U Loft surface. A U Loft surface interpolates a
surface across multiple curve sub-objects. The curves become U-axis contours of
the surface. Methods are available to get/set the number of curves used to make
the loft, append curves to the list, get/set the parent ids, and get/set the
flipped state for each of the curves.\n\n
All methods of this class are implemented by the system. */
class NURBSULoftSurface : public NURBSSurface {
	friend class NURBSSet;
	Tab<NURBSId> mParentId;
	Tab<int> mParentIndex;
	Tab<BOOL> mFlip;
	Tab<double>mCurveStartParam;
	Tab<double>mTension;
	Tab<BOOL> mUseTangents;
	Tab<BOOL> mFlipTangents;
	BOOL mAutoAlign;
	BOOL mCloseLoft;
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNULoftSurface;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mParentId.SetCount(0);</b>\n\n
	<b> mParentIndex.SetCount(0);</b>\n\n
	<b> mFlip.SetCount(0);</b>\n\n
	<b> mCurveStartParam.SetCount(0);</b>\n\n
	<b> mTension.SetCount(0);</b>\n\n
	<b> mUseTangents.SetCount(0);</b>\n\n
	<b> mFlipTangents.SetCount(0);</b>\n\n
	<b> mAutoAlign = FALSE;</b>\n\n
	<b> mCloseLoft = FALSE;</b> */
	DllExport NURBSULoftSurface(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSULoftSurface(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSULoftSurface\& surf</b>\n\n
	The surface to assign. */
	DllExport NURBSULoftSurface & operator=(const NURBSULoftSurface& surf);
	/*! \remarks Sets the number of curves used by the U loft.
	\par Parameters:
	<b>int num</b>\n\n
	The number of curves to set. */
	DllExport void SetNumCurves(int num);
	/*! \remarks Returns the number of curves used by the surface. */
	DllExport int GetNumCurves(void);
	/*! \remarks Adds a curve to the end of the list of curves used to make the U
	loft surface.
	\par Parameters:
	<b>int index</b>\n\n
	The index of the curve to add in the <b>NURBSSet</b>.\n\n
	<b>BOOL flip</b>\n\n
	TRUE to flip the orientation of this curve's normal used to build the loft;
	otherwise FALSE.
	\return  The number of curves in the list prior to this one being added. */
	DllExport int AppendCurve(int index, BOOL flip, double startpoint=0.0,
								double tension=0.0, BOOL useTangent=FALSE, BOOL flipTangent=FALSE);
	/*! \remarks Adds a curve to the end of the list of curves used to make the U
	loft surface.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	The NURBS id of the curve to add.\n\n
	<b>BOOL flip</b>\n\n
	TRUE to flip the orientation of this curve's normal used to build the loft;
	otherwise FALSE. */
	DllExport int AppendCurve(NURBSId id, BOOL flip, double startpoint=0.0,
								double tension=0.0, BOOL useTangent=FALSE, BOOL flipTangent=FALSE);
	/*! \remarks Sets the index in the <b>NURBSSet</b> of the specified parent
	object.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number, 0, 1, 2, etc.\n\n
	<b>int index</b>\n\n
	The index into the <b>NURBSSet</b> of the parent surface. */
	DllExport void SetParent(int pnum, int index);
	/*! \remarks Sets the <b>NURBSId</b> of the specified parent.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number, 0, 1, 2, etc.\n\n
	<b>NURBSId id</b>\n\n
	The id to set. */
	DllExport void SetParentId(int pnum, NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the specified
	parent object.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number, 0, 1, 2, etc. */
	DllExport int GetParent(int pnum);
	/*! \remarks Returns the <b>NURBSId</b> of the specified parent. Note that
	a <b>NURBSId</b> won't be valid until the object has been instantiated in
	the scene
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number, 0, 1, 2, etc. */
	DllExport NURBSId GetParentId(int pnum);
	/*! \remarks This method allows one to control the matching of parent
	normals when creating the U loft surface.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number, 0, 1, 2, etc.\n\n
	<b>BOOL flip</b>\n\n
	TRUE to flip the orientation of the normal when building the surface at
	this curve; otherwise FALSE. */
	DllExport void SetFlip(int pnum, BOOL flip);
	/*! \remarks Returns the flip state of the specified parent normal.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number, 0, 1, 2, etc. */
	DllExport BOOL GetFlip(int pnum);
	/*! \remarks	Sets the start point for the specified parent curve. Note: This is only
	applicable if the parent is a closed curve.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the start point.\n\n
	<b>int pnum</b>\n\n
	The parent number, 0, 1, 2, etc.\n\n
	<b>double startpoint</b>\n\n
	The start point in the range 0.0 to 1.0. */
	DllExport void SetCurveStartPoint(TimeValue t, int pnum, double startpoint);
	/*! \remarks	Returns the start point for the specified parent curve. Note: This is only
	applicable if the parent is a closed curve.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the start point.\n\n
	<b>int pnum</b>\n\n
	The parent number, 0, 1, 2, etc. */
	DllExport double GetCurveStartPoint(TimeValue t, int pnum);

	/*! \remarks	Sets the curve tension for the specified parent curve.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the tension.\n\n
	<b>int pnum</b>\n\n
	The parent number, 0, 1, 2, etc.\n\n
	<b>double tension</b>\n\n
	The tension value to set. */
	DllExport void SetCurveTension(TimeValue t, int pnum, double tension);
	/*! \remarks	Returns the tension setting of the specified parent curve at the specified
	time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the tension.\n\n
	<b>int pnum</b>\n\n
	The parent number, 0, 1, 2, etc. */
	DllExport double GetCurveTension(TimeValue t, int pnum);
	/*! \remarks	Sets the curve use surface tangent setting. If the curve is a curve on
	surface, turning this on causes the U loft to use the tangency of the
	surface. This can help blend a loft smoothly onto a surface.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number, 0, 1, 2, etc.\n\n
	<b>BOOL useTangent</b>\n\n
	TRUE to use the tangent; otherwise FALSE. */
	DllExport void SetCurveUseSurfaceTangent(int pnum, BOOL useTangent);
	/*! \remarks	Returns TRUE if the curve use surface tangent setting is on; FALSE if off.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number, 0, 1, 2, etc. */
	DllExport BOOL GetCurveUseSurfaceTangent(int pnum);
	/*! \remarks	Sets the flip tangent setting for the specified parent curve.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number, 0, 1, 2, etc.\n\n
	<b>BOOL flipTangent</b>\n\n
	TRUE to flip the tangent; otherwise FALSE. */
	DllExport void SetFlipTangent(int pnum, BOOL flipTangent);
	/*! \remarks	Returns TRUE if the tangent is flipped for the specified parent curve;
	otherwise FALSE.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent number, 0, 1, 2, etc. */
	DllExport BOOL GetFlipTangent(int pnum);
	/*! \remarks	Sets if auto align is on or off.
	\par Parameters:
	<b>BOOL autoalign</b>\n\n
	TRUE for on; FALSE for off. */
	DllExport void SetAutoAlign(BOOL autoalign);
	/*! \remarks	Returns TRUE if auto align is on; otherwise FALSE. */
	DllExport BOOL GetAutoAlign();
	/*! \remarks	Sets if the loft is closed or not.
	\par Parameters:
	<b>BOOL closeLoft</b>\n\n
	TRUE for closed; FALSE for open. */
	DllExport void SetCloseLoft(BOOL closeLoft);
	/*! \remarks	Returns TRUE if loft is closed; otherwise FALSE.
	\par Operators:
	*/
	DllExport BOOL GetCloseLoft();
};


/*! \sa  Class NURBSSurface.\n\n
\par Description:
This class is available in release 2.5 and later only.\n\n
This class provides access to the UV Loft Surface. This surface is similar to
the U Loft surface, but has a set of curves in the V dimension as well as the U
dimension. */
class NURBSUVLoftSurface : public NURBSSurface {
	friend class NURBSSet;
	Tab<NURBSId> mUParentId;
	Tab<int> mUParentIndex;
	Tab<NURBSId> mVParentId;
	Tab<int> mVParentIndex;
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNUVLoftSurface;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mUParentId.SetCount(0);</b>\n\n
	<b> mUParentIndex.SetCount(0);</b>\n\n
	<b> mVParentId.SetCount(0);</b>\n\n
	<b> mVParentIndex.SetCount(0);</b> */
	DllExport NURBSUVLoftSurface(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSUVLoftSurface(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSUVLoftSurface\& surf</b>\n\n
	The surface to assign. */
	DllExport NURBSUVLoftSurface & operator=(const NURBSUVLoftSurface& surf);

	/*! \remarks Sets the number of curves in the U dimension.
	\par Parameters:
	<b>int num</b>\n\n
	The number of curves to use in the U dimension. */
	DllExport void SetNumUCurves(int num);
	/*! \remarks Returns the number of curves in the U dimension. */
	DllExport int GetNumUCurves(void);
	/*! \remarks Appends the specified curve (by NURBSSet index) to the list
	of U curves.
	\par Parameters:
	<b>int index</b>\n\n
	The index in the <b>NURBSSet</b> of the curve to use. */
	DllExport int AppendUCurve(int index);
	/*! \remarks Appends the specified curve (by NURBSId) to the list of U
	curves.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	The <b>NURBSId</b> of the curve to use.
	\return  Returns the number of curves in the set prior to appending. */
	DllExport int AppendUCurve(NURBSId id);
	/*! \remarks Sets the specified parent curve (by NURBSSet index) as the
	speciifed curve in the surface.
	\par Parameters:
	<b>int pnum</b>\n\n
	The index into the list of U curves of the parent curve to set.\n\n
	<b>int index</b>\n\n
	The index into the <b>NURBSSet</b> of the parent curve to set. */
	DllExport void SetUParent(int pnum, int index);
	/*! \remarks Sets the specified parent curve (by NURBSId) as the speciifed
	curve in the surface.
	\par Parameters:
	<b>int pnum</b>\n\n
	The index into the list of U curves of the parent curve to set.\n\n
	<b>NURBSId id</b>\n\n
	The <b>NURBSId</b> of the parent curve to set. */
	DllExport void SetUParentId(int pnum, NURBSId id);
	/*! \remarks Returns the index into the <b>NURBSSet</b> of the specified
	parent curve.
	\par Parameters:
	<b>int pnum</b>\n\n
	The zero based index of the parent curve. */
	DllExport int GetUParent(int pnum);
	/*! \remarks Returns the <b>NURBSId</b> of the specified parent curve.
	Note that a <b>NURBSId</b> won't be valid until the surface has been
	instantiated in the scene.
	\par Parameters:
	<b>int pnum</b>\n\n
	The zero based index of the parent curve. */
	DllExport NURBSId GetUParentId(int pnum);

	/*! \remarks Sets the number of curves in the V dimension.
	\par Parameters:
	<b>int num</b>\n\n
	The number of curves to use in the V dimension. */
	DllExport void SetNumVCurves(int num);
	/*! \remarks Returns the number of curves in the V dimension. */
	DllExport int GetNumVCurves(void);
	/*! \remarks Appends the specified curve (by NURBSSet index) to the list
	of V curves.
	\par Parameters:
	<b>int index</b>\n\n
	The index in the <b>NURBSSet</b> of the curve to use.
	\return  Returns the number of curves in the set prior to appending. */
	DllExport int AppendVCurve(int index);
	/*! \remarks Appends the specified curve (by NURBSId) to the list of U
	curves.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	The <b>NURBSId</b> of the curve to use.
	\return  Returns the number of curves in the set prior to appending. */
	DllExport int AppendVCurve(NURBSId id);
	/*! \remarks Sets the specified parent curve (by <b>NURBSSet</b> index) as
	the speciifed curve in the surface.
	\par Parameters:
	<b>int pnum</b>\n\n
	The index into the list of V curves of the parent curve to set.\n\n
	<b>int index</b>\n\n
	The index into the <b>NURBSSet</b> of the parent curve to set. */
	DllExport void SetVParent(int pnum, int index);
	/*! \remarks Sets the specified parent curve (by NURBSId) as the speciifed
	curve in the surface.
	\par Parameters:
	<b>int pnum</b>\n\n
	The index into the list of V curves of the parent curve to set.\n\n
	<b>NURBSId id</b>\n\n
	The <b>NURBSId</b> of the parent curve to set. */
	DllExport void SetVParentId(int pnum, NURBSId id);
	/*! \remarks Returns the index into the <b>NURBSSet</b> of the specified
	parent curve.
	\par Parameters:
	<b>int pnum</b>\n\n
	The zero based index of the parent curve. */
	DllExport int GetVParent(int pnum);
	/*! \remarks Returns the <b>NURBSId</b> of the specified parent curve.
	Note that a <b>NURBSId</b> won't be valid until the surface has been
	instantiated in the scene.
	\par Parameters:
	<b>int pnum</b>\n\n
	The zero based index of the parent curve. */
	DllExport NURBSId GetVParentId(int pnum);
};



/*! \sa  Class NURBSSurface.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class defines a dependent extrude surface. An extrude surface is extruded
from a curve sub-object. It is similar to a surface created with the Extrude
modifier. The advantage is that an extrude sub-object is part of the NURBS
model, so you can use it to construct other curve and surface sub-objects.
Methods are available to get/set the parent index and id, get/set the extrusion
vector and get/set the extrusion distance.\n\n
All methods of this class are implemented by the system. */
class NURBSExtrudeSurface : public NURBSSurface {
	friend class NURBSSet;
	NURBSId mParentId;
	int mParentIndex;
	Matrix3 mXForm;
	double mDistance;
	double mCurveStartParam;
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNExtrudeSurface;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mParentId = 0;</b>\n\n
	<b> mParentIndex = -1;</b>\n\n
	<b> mXForm.IdentityMatrix();</b>\n\n
	<b> mDistance = 0.0;</b>\n\n
	<b> mCurveStartParam = 0.0;</b> */
	DllExport NURBSExtrudeSurface(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSExtrudeSurface(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSExtrudeSurface\& surf</b>\n\n
	The surface to assign. */
	DllExport NURBSExtrudeSurface & operator=(const NURBSExtrudeSurface& surf);
	/*! \remarks Sets the index in the <b>NURBSSet</b> of the parent object.
	\par Parameters:
	<b>int index</b>\n\n
	The index into the <b>NURBSSet</b> of the parent surface. */
	DllExport void SetParent(int index);
	/*! \remarks Sets the <b>NURBSId</b> of the parent.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	The id to set. */
	DllExport void SetParentId(NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the parent
	object. */
	DllExport int GetParent(void);
	/*! \remarks Returns the <b>NURBSId</b> of the parent. Note that a
	<b>NURBSId</b> won't be valid until the object has been instantiated in the
	scene. */
	DllExport NURBSId GetParentId(void);
	/*! \remarks	Sets the extrusion axis at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the axis system.\n\n
	<b>Matrix3\& ray</b>\n\n
	The extrusion axis. */
	DllExport void SetAxis(TimeValue t, Matrix3& ray);
	/*! \remarks	Returns the extrusion axis at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the axis. */
	DllExport Matrix3& GetAxis(TimeValue t);
	/*! \remarks Sets the length of the extrudion at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the distance.\n\n
	<b>double d</b>\n\n
	The distance to set. */
	DllExport void SetDistance(TimeValue t, double d);
	/*! \remarks Returns the length of the extrudion at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to return the distance. */
	DllExport double GetDistance(TimeValue t);
	/*! \remarks	Sets the start point at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the start point.\n\n
	<b>double startpoint</b>\n\n
	The start point in the range 0.0 to 1.0. */
	DllExport void SetCurveStartPoint(TimeValue t, double startpoint);
	/*! \remarks	Returns the start point at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the start point.
	\par Operators:
	*/
	DllExport double GetCurveStartPoint(TimeValue t);
};


/*! \sa  Class NURBSSurface.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class defines a dependent lathe surface. A lathe surface is generated from
a curve sub-object. It is similar to a surface created with the Lathe modifier.
The advantage is that a lathe sub-object is part of the NURBS model, so you can
use it to construct other curve and surface sub-objects. Methods are available
to get/set the parent index and id, get/set the axis and amount rotation of the
lathe.\n\n
All methods of this class are implemented by the system. */
class NURBSLatheSurface : public NURBSSurface {
	friend class NURBSSet;
	NURBSId mParentId;
	int mParentIndex;
	Matrix3 mXForm;
	double mRotation;
	double mCurveStartParam;
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNLatheSurface;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mParentId = 0;</b>\n\n
	<b> mParentIndex = -1;</b>\n\n
	<b> mXForm.IdentityMatrix();</b>\n\n
	<b> mRotation = 360.0;</b>\n\n
	<b> mCurveStartParam = 360.0;</b> */
	DllExport NURBSLatheSurface(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSLatheSurface(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSLatheSurface\& surf</b>\n\n
	The surface to assign. */
	DllExport NURBSLatheSurface & operator=(const NURBSLatheSurface& surf);
	/*! \remarks Sets the index in the <b>NURBSSet</b> of the parent object.
	\par Parameters:
	<b>int index</b>\n\n
	The index into the <b>NURBSSet</b> of the parent surface. */
	DllExport void SetParent(int index);
	/*! \remarks Sets the <b>NURBSId</b> of the specified parent.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	The id to set. */
	DllExport void SetParentId(NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the specified
	parent object. */
	DllExport int GetParent(void);
	/*! \remarks Returns the <b>NURBSId</b> of the parent. Note that a
	<b>NURBSId</b> won't be valid until the object has been instantiated in the
	scene */
	DllExport NURBSId GetParentId(void);
	/*! \remarks Sets the axis to use for the surface of revolution by
	specifying a time and a axis system.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which the axis is set.\n\n
	<b>Matrix3\& ray</b>\n\n
	Specifies the axis for revolution. See Class Matrix3. */
	DllExport void SetAxis(TimeValue t, Matrix3& ray);
	/*! \remarks Returns the axis system for the surface of revolution.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to return the axis. */
	DllExport Matrix3& GetAxis(TimeValue t);
	/*! \remarks Sets the amount of rotation for the surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the amount.\n\n
	<b>double degrees</b>\n\n
	The angle of the revolution in degrees. */
	DllExport void SetRotation(TimeValue t, double degrees);
	/*! \remarks Returns the angle of the revolution in degrees.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to return the angle. */
	DllExport double GetRotation(TimeValue t);
	/*! \remarks	Sets the start point at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the start point.\n\n
	<b>double startpoint</b>\n\n
	The start point to set in the range 0.0 to 1.0. */
	DllExport void SetCurveStartPoint(TimeValue t, double startpoint);
	/*! \remarks	Returns the start point at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the start point.
	\par Operators:
	*/
	DllExport double GetCurveStartPoint(TimeValue t);
};


/*! \sa  Class NURBSSurface.\n\n
\par Description:
This class is available in release 2.5 and later only.\n\n
This class provides access to the Cap Surface. A Cap Surface is a surface that
caps a closed curve or the edge of a closed surface. Caps are especially useful
with extruded surfaces. <br>  protected: */
class NURBSCapSurface : public NURBSSurface {
	friend class NURBSSet;
	NURBSId mParentId;
	int mParentIndex;
	int mParentEdge;
	double mCurveStartParam;
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNCapSurface;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mParentId = 0;</b>\n\n
	<b> mParentIndex = -1;</b>\n\n
	<b> mParentEdge = -1;</b>\n\n
	<b> mCurveStartParam = 0.0;</b> */
	DllExport NURBSCapSurface(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSCapSurface(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSCapSurface\& surf</b>\n\n
	The surface to assign. */
	DllExport NURBSCapSurface & operator=(const NURBSCapSurface& surf);
	/*! \remarks Establishes the curve or surface that's capped by specifying
	its index in the <b>NURBSSet</b>.
	\par Parameters:
	<b>int index</b>\n\n
	The index in the NURBSSet of the curve or surface to cap. */
	DllExport void SetParent(int index);
	/*! \remarks Establishes the curve or surface that's capped by specifying
	its <b>NURBSId</b>.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	The id of the curve or surface to cap. */
	DllExport void SetParentId(NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the curve or
	surface that's capped. */
	DllExport int GetParent(void);
	/*! \remarks Returns the <b>NURBSId</b> of the curve or surface that's
	capped. */
	DllExport NURBSId GetParentId(void);
	/*! \remarks Establishes which edge of the closed parent surface is
	capped.
	\par Parameters:
	<b>int edge</b>\n\n
	The edge to cap. One of the following values:\n\n
	<b>0</b>: The low U edge.\n\n
	<b>1</b>: The high U edge.\n\n
	<b>2</b>: The low V edge.\n\n
	<b>3</b>: The high V edge. */
	DllExport void SetEdge(int edge);
	/*! \remarks Returns the edge of the closed parent surface is capped.
	\return  One of the following values:\n\n
	<b>0</b>: The low U edge.\n\n
	<b>1</b>: The high U edge.\n\n
	<b>2</b>: The low V edge.\n\n
	<b>3</b>: The high V edge. */
	DllExport int GetEdge();

	// only if the parent is a closed curve
	/*! \remarks	Sets the start point for the curve. Note: This is only applicable if the
	parent is a closed curve.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to set the start point.\n\n
	<b>double startpoint</b>\n\n
	The start point to set. */
	DllExport void SetCurveStartPoint(TimeValue t, double startpoint);
	/*! \remarks	Returns the start point for the curve. Note: This is only applicable if the
	parent is a closed curve.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to get the start point. */
	DllExport double GetCurveStartPoint(TimeValue t);
};


/*! \sa  Class NURBSSurface.\n\n
\par Description:
This class is available in release 2.5 and later only.\n\n
This class provides access to the 1-Rail Sweep Surface. A 1-Rail Sweep Surface
uses at least two curves. One curve, the "rail," defines one edge of the
surface. The other curves define the surface's cross sections. The
cross-section curves should intersect the rail curve. If the cross sections
don't intersect the rail, the resulting surface is unpredicable. */
class NURBS1RailSweepSurface : public NURBSSurface {
	friend class NURBSSet;
	NURBSId mRailId;
	int mRailIndex;
	Tab<NURBSId> mParentId;
	Tab<int> mParentIndex;
	Tab<BOOL> mFlip;
	Tab<double> mCurveStartParam;
	BOOL mParallel;
	BOOL mSnapCrossSections;
	BOOL mRoadlike;
	Matrix3 mXForm;
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kN1RailSweepSurface;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mRailId = NULL;</b>\n\n
	<b> mRailIndex = -1;</b>\n\n
	<b> mParentId.SetCount(0);</b>\n\n
	<b> mParentIndex.SetCount(0);</b>\n\n
	<b> mFlip.SetCount(0);</b>\n\n
	<b> mCurveStartParam.SetCount(0);</b>\n\n
	<b> mSnapCrossSections = FALSE;</b>\n\n
	<b> mRoadlike = FALSE;</b>\n\n
	<b> mXForm.IdentityMatrix();</b> */
	DllExport NURBS1RailSweepSurface(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBS1RailSweepSurface(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBS1RailSweepSurface\& surf</b>\n\n
	The surface to assign. */
	DllExport NURBS1RailSweepSurface & operator=(const NURBS1RailSweepSurface& surf);
	/*! \remarks Establishes the curve to use as the rail by specifying its
	index in the <b>NURBSSet</b>.
	\par Parameters:
	<b>int index</b>\n\n
	The index in the <b>NURBSSet</b>. */
	DllExport void SetParentRail(int index);
	/*! \remarks Establishes the curve to use as the rail by specifying its
	<b>NURBSId</b>.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	The id of the rail curve to use. */
	DllExport void SetParentRailId(NURBSId id);
	/*! \remarks Returns the index in the NURBSSet of the rail curve. */
	DllExport int GetParentRail();
	/*! \remarks Returns the NURBSId of the rail curve. */
	DllExport NURBSId GetParentRailId();
	/*! \remarks Sets the number of cross-section curves.
	\par Parameters:
	<b>int num</b>\n\n
	The number of cross-section curves to use. */
	DllExport void SetNumCurves(int num);
	/*! \remarks Returns the number of cross-section curves used. */
	DllExport int GetNumCurves(void);
	/*! \remarks Adds a curve to the end of the list of cross-section curves by
	specifying the index in the <b>NURBSSet</b>.
	\par Parameters:
	<b>int index</b>\n\n
	The index in the <b>NURBSSet</b> of the cross-section curve to append.\n\n
	<b>BOOL flip</b>\n\n
	TRUE to reverse (or flip) the direction of the curve; FALSE to use the
	non-reversed orientation.
	\return  The number of cross-section curves prior to appending. */
	DllExport int AppendCurve(int index, BOOL flip, double startpoint=0.0);
	/*! \remarks Adds a curve to the end of the list of cross-section curves by
	specifying a <b>NURBSId</b>.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	Specifies the cross-section curve to append.\n\n
	<b>BOOL flip</b>\n\n
	TRUE to reverse (or flip) the direction of the curve; FALSE to use the
	non-reversed orientation.
	\return  The number of cross-section curves prior to appending. */
	DllExport int AppendCurve(NURBSId id, BOOL flip, double startpoint=0.0);
	/*! \remarks Specifies the curve to use as a cross-section via its index
	in the <b>NURBSSet</b>.
	\par Parameters:
	<b>int pnum</b>\n\n
	The zero based index of the curve to set.\n\n
	<b>int index</b>\n\n
	The index in the NURBSSet of the curve. */
	DllExport void SetParent(int pnum, int index);
	/*! \remarks Specifies the curve to use as a cross-section via its
	<b>NURBSId</b>.
	\par Parameters:
	<b>int pnum</b>\n\n
	The zero based index of the curve to set.\n\n
	<b>NURBSId id</b>\n\n
	The id of the curve. */
	DllExport void SetParentId(int pnum, NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the specified
	cross-section curve.
	\par Parameters:
	<b>int pnum</b>\n\n
	The zero based index of the curve to get. */
	DllExport int GetParent(int pnum);
	/*! \remarks Returns the <b>NURBSId</b> of the specified cross-section
	curve.
	\par Parameters:
	<b>int pnum</b>\n\n
	The zero based index of the curve to get. */
	DllExport NURBSId GetParentId(int pnum);
	/*! \remarks Sets the reversed (or flipped) state of the specified
	cross-section curve.
	\par Parameters:
	<b>int pnum</b>\n\n
	The zero based index of the curve.\n\n
	<b>BOOL flip</b>\n\n
	TRUE to reverse the direction; FALSE for the normal direction. */
	DllExport void SetFlip(int pnum, BOOL flip);
	/*! \remarks Returns the reversed (or flipped) state of the specified
	cross-section curve. TRUE is reversed; FALSE is not.
	\par Parameters:
	<b>int pnum</b>\n\n
	The zero based index of the curve. */
	DllExport BOOL GetFlip(int pnum);
	/*! \remarks Sets the state of the parallel flag. When on, it ensures that
	the sweep surface's normal is parallel to the rail.
	\par Parameters:
	<b>BOOL para</b>\n\n
	TRUE for on; FALSE for off. */
	DllExport void SetParallel(BOOL para);
	/*! \remarks Returns the state of the parallel flag. When TRUE, 3ds Max
	ensures that the sweep surface's normal is parallel to the rail. */
	DllExport BOOL GetParallel();
	/*! \remarks	Sets the start point for the specified parent curve. Note: This is only
	applicable if the parent is a closed curve.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the start point.\n\n
	<b>int pnum</b>\n\n
	The zero based index of the curve in the set of cross sections.\n\n
	<b>double startpoint</b>\n\n
	The start point in the range 0.0 to 1.0. */
	DllExport void SetCurveStartPoint(TimeValue t, int pnum, double startpoint);
	/*! \remarks	Returns the start point of the specified parent curve.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the start point.\n\n
	<b>int pnum</b>\n\n
	The zero based index of the curve in the set of cross sections.\n\n
	  */
	DllExport double GetCurveStartPoint(TimeValue t, int pnum);
	/*! \remarks	Sets the snap cross sections setting. When on, cross-section curves are
	translated so they intersect the rail.
	\par Parameters:
	<b>BOOL snapCS</b>\n\n
	TRUE for on; FALSE for off. */
	DllExport void SetSnapCS(BOOL snapCS);
	/*! \remarks	Returns TRUE if snap to Cross Section is on; otherwise FALSE. */
	DllExport BOOL GetSnapCS();
	/*! \remarks	Sets the roadlike setting to on or off. When on, the sweep uses a constant
	up-vector so the cross sections twist uniformly as they travel along the
	rail. In other words, the cross sections bank like a car following a road,
	or a camera following a path controller in 3ds Max.
	\par Parameters:
	<b>BOOL roadlike</b>\n\n
	TRUE for on; FALSE for off. */
	DllExport void SetRoadlike(BOOL roadlike);
	/*! \remarks	Returns TRUE if the roadlike setting is on; otherwise FALSE. */
	DllExport BOOL GetRoadlike();
	/*! \remarks	Sets the axis of the sweep.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the axis.\n\n
	<b>Matrix3\& ray</b>\n\n
	The axis system to set. */
	DllExport void SetAxis(TimeValue t, Matrix3& ray);
	/*! \remarks	Returns the axis of the sweep.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the axis.
	\par Operators:
	*/
	DllExport Matrix3& GetAxis(TimeValue t);
};


/*! \sa  Class NURBSSurface.\n\n
\par Description:
This class is available in release 2.5 and later only.\n\n
This class provides access to the 2-Rail Sweep Surface. A 2-Rail Sweep surface
uses at least three curves. Two curves, the "rails," define the two edges of
the surface. The other curves define the surface's cross sections. A 2-Rail
Sweep surface is similar to a 1-Rail sweep. The additional rail gives you more
control over the shape of the surface. */
class NURBS2RailSweepSurface : public NURBSSurface {
	friend class NURBSSet;
	Tab<NURBSId> mParentId;
	Tab<int> mParentIndex;
	Tab<BOOL> mFlip;
	NURBSId mRailParentId[2];
	int mRailParentIndex[2];
	BOOL mParallel;
	BOOL mScale;
	BOOL mSnapCrossSections;
	Tab<double> mCurveStartParam;
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kN2RailSweepSurface;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mParentId.SetCount(0);</b>\n\n
	<b> mParentIndex.SetCount(0);</b>\n\n
	<b> mRailParentId[0] = mRailParentId[1] = NULL;</b>\n\n
	<b> mRailParentIndex[0] = mRailParentIndex[1] = -1;</b>\n\n
	<b> mCurveStartParam.SetCount(0);</b>\n\n
	<b> mSnapCrossSections = FALSE;</b> */
	DllExport NURBS2RailSweepSurface(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBS2RailSweepSurface(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBS2RailSweepSurface\& surf</b>\n\n
	The surface to assign. */
	DllExport NURBS2RailSweepSurface & operator=(const NURBS2RailSweepSurface& surf);

	/*! \remarks Sets the number of cross-section curves.
	\par Parameters:
	<b>int num</b>\n\n
	The number of cross-section curves to use. */
	DllExport void SetNumCurves(int num);
	/*! \remarks Returns the number of cross-section curves used. */
	DllExport int GetNumCurves(void);
	/*! \remarks Adds a curve to the end of the list of cross-section curves by
	specifying the index in the <b>NURBSSet</b>.
	\par Parameters:
	<b>int index</b>\n\n
	The index in the <b>NURBSSet</b> of the cross-section curve to append.\n\n
	<b>BOOL flip</b>\n\n
	TRUE to reverse (or flip) the direction of the curve; FALSE to use the
	non-reversed orientation.
	\return  The number of cross-section curves prior to appending. */
	DllExport int AppendCurve(int index, BOOL flip, double startpoint=0.0);
	/*! \remarks Adds a curve to the end of the list of cross-section curves by
	specifying a <b>NURBSId</b>.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	Specifies the cross-section curve to append.\n\n
	<b>BOOL flip</b>\n\n
	TRUE to reverse (or flip) the direction of the curve; FALSE to use the
	non-reversed orientation.
	\return  The number of cross-section curves prior to appending. */
	DllExport int AppendCurve(NURBSId id, BOOL flip, double startpoint=0.0);
	/*! \remarks Specifies the curve to use as a cross-section via its index
	in the <b>NURBSSet</b>.
	\par Parameters:
	<b>int pnum</b>\n\n
	The zero based index of the curve to set.\n\n
	<b>int index</b>\n\n
	The index in the NURBSSet of the curve. */
	DllExport void SetParent(int pnum, int index);
	/*! \remarks Specifies the curve to use as a cross-section via its
	<b>NURBSId</b>.
	\par Parameters:
	<b>int pnum</b>\n\n
	The zero based index of the curve to set.\n\n
	<b>NURBSId id</b>\n\n
	The id of the curve. */
	DllExport void SetParentId(int pnum, NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the specified
	cross-section curve.
	\par Parameters:
	<b>int pnum</b>\n\n
	The zero based index of the curve to get. */
	DllExport int GetParent(int pnum);
	/*! \remarks Returns the <b>NURBSId</b> of the specified cross-section
	curve.
	\par Parameters:
	<b>int pnum</b>\n\n
	The zero based index of the curve to get. */
	DllExport NURBSId GetParentId(int pnum);
	/*! \remarks Sets the reversed (or flipped) state of the specified
	cross-section curve.
	\par Parameters:
	<b>int pnum</b>\n\n
	The zero based index of the curve.\n\n
	<b>BOOL flip</b>\n\n
	TRUE to reverse the direction; FALSE for the normal direction. */
	DllExport void SetFlip(int pnum, BOOL flip);
	/*! \remarks Returns the reversed (or flipped) state of the specified
	cross-section curve. TRUE is reversed; FALSE is not.
	\par Parameters:
	<b>int pnum</b>\n\n
	The zero based index of the curve. */
	DllExport BOOL GetFlip(int pnum);
	/*! \remarks Sets the state of the parallel flag. When off, it allows the
	cross section curve to bank to follow the rails. If on, it maintains the
	initial orientation of the cross section during the course of the sweep
	\par Parameters:
	<b>BOOL para</b>\n\n
	TRUE for on; FALSE for off. */
	DllExport void SetParallel(BOOL para);
	/*! \remarks Returns the state of the parallel flag. */
	DllExport BOOL GetParallel();
	/*! \remarks To allow the cross sections to run along the rails, the cross
	section curves may need to be scaled. This method controls if the scaling
	is done uniformly or non-uniformly.
	\par Parameters:
	<b>BOOL scale</b>\n\n
	TRUE to scale uniformly; FALSE to scale non-uniformly. */
	DllExport void SetScale(BOOL scale);
	/*! \remarks Returns TRUE if the cross section curves are scaled
	uniformly; FALSE for non-uniformly. */
	DllExport BOOL GetScale();
	/*! \remarks	Sets the snap cross sections setting. When on, cross-section curves are
	translated so they intersect the rail.
	\par Parameters:
	<b>BOOL snapCS</b>\n\n
	TRUE for on; FALSE for off. */
	DllExport void SetSnapCS(BOOL snapCS);
	/*! \remarks	Returns TRUE if snap cross sections is on; otherwise FALSE. */
	DllExport BOOL GetSnapCS();

	/*! \remarks Specifies the index in the <b>NURBSSet</b> of the first or
	second rail curve to use.
	\par Parameters:
	<b>int pnum</b>\n\n
	Pass 0 for the first rail curve; 1 for the second rail curve.\n\n
	<b>int index</b>\n\n
	The index in the <b>NURBSSet</b> of the curve to use as a rail. */
	DllExport void SetRailParent(int pnum, int index);
	/*! \remarks Specifies the <b>NURBSId</b> of the first or second rail
	curve to use.
	\par Parameters:
	<b>int pnum</b>\n\n
	Pass 0 for the first rail curve; 1 for the second rail curve.\n\n
	<b>NURBSId id</b>\n\n
	The id of the curve to use as a rail. */
	DllExport void SetRailParentId(int pnum, NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the specified
	parent rail curve.
	\par Parameters:
	<b>int pnum</b>\n\n
	Pass 0 for the first rail curve; 1 for the second rail curve. */
	DllExport int GetRailParent(int pnum);
	/*! \remarks Returns the <b>NURBSId</b> of the specified parent rail
	curve.
	\par Parameters:
	<b>int pnum</b>\n\n
	Pass 0 for the first rail curve; 1 for the second rail curve. */
	DllExport NURBSId GetRailParentId(int pnum);

	/*! \remarks	Sets the start point for the specified parent curve at the time passed.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the start point.\n\n
	<b>int pnum</b>\n\n
	Pass 0 for the first rail curve; 1 for the second rail curve.\n\n
	<b>double startpoint</b>\n\n
	The start point in the range of 0.0 to 1.0. */
	DllExport void SetCurveStartPoint(TimeValue t, int pnum, double startpoint);
	/*! \remarks	Returns the start point for the specified parent curve at the time passed.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the start point.\n\n
	<b>int pnum</b>\n\n
	Pass 0 for the first rail curve; 1 for the second rail curve.
	\par Operators:
	*/
	DllExport double GetCurveStartPoint(TimeValue t, int pnum);
};



/*! \sa  Class NURBSSurface.\n\n
\par Description:
This class is available in release 2.5 and later only.\n\n
This class provides access to the Multicurve Trim Surface which is a surface
that is trimmed by multiple curves forming a loop. */
class NURBSMultiCurveTrimSurface : public NURBSSurface {
	friend class NURBSSet;
	Tab<NURBSId> mParentId;
	Tab<int> mParentIndex;
	NURBSId mSurfaceId;
	int mSurfaceIndex;
	BOOL mFlipTrim;
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNMultiCurveTrimSurface;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mParentId.SetCount(0);</b>\n\n
	<b> mParentIndex.SetCount(0);</b>\n\n
	<b> mSurfaceId = NULL;</b>\n\n
	<b> mSurfaceIndex = -1;</b>\n\n
	<b> mFlipTrim = FALSE;</b>\n\n
	  */
	DllExport NURBSMultiCurveTrimSurface(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSMultiCurveTrimSurface(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSMultiCurveTrimSurface\& surf</b>\n\n
	The surface to assign. */
	DllExport NURBSMultiCurveTrimSurface & operator=(const NURBSMultiCurveTrimSurface& surf);

	/*! \remarks Sets the number of curves used for the trim loop.
	\par Parameters:
	<b>int num</b>\n\n
	The number of curves to use.\n\n
	  */
	DllExport void SetNumCurves(int num);
	/*! \remarks Returns the number of curves used for the trim loop. */
	DllExport int GetNumCurves(void);
	/*! \remarks Adds the specified curve (using a <b>NURBSSet</b> index) to
	the end of the list of curves comprising the trim loop.
	\par Parameters:
	<b>int index</b>\n\n
	The index into the <b>NURBSSet</b> of the curve to add.
	\return  The number of curves in the loop prior to appending. */
	DllExport int AppendCurve(int index);
	/*! \remarks Adds the specified curve (using a <b>NURBSSet</b> index) to
	the end of the list of curves comprising the trim loop.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	The id to use.
	\return  The number of curves in the loop prior to appending. */
	DllExport int AppendCurve(NURBSId id);
	/*! \remarks Sets the specified curve in the trim loop to the curve
	specified by an index into the <b>NURBSSet</b>.
	\par Parameters:
	<b>int pnum</b>\n\n
	Zero based index of which curve to set.\n\n
	<b>int index</b>\n\n
	The index into the <b>NURBSSet</b> of the curve to use. */
	DllExport void SetParent(int pnum, int index);
	/*! \remarks Sets the specified curve in the trim loop to the curve
	specified by a <b>NURBSId</b>.
	\par Parameters:
	<b>int pnum</b>\n\n
	Zero based index of which curve to set.\n\n
	<b>NURBSId id</b>\n\n
	The id of the curve to use. */
	DllExport void SetParentId(int pnum, NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the specified
	parent object.
	\par Parameters:
	<b>int pnum</b>\n\n
	Zero based index of which curve to get. */
	DllExport int GetParent(int pnum);
	/*! \remarks Returns the <b>NURBSId</b> of the specified curve in the trim
	loop.
	\par Parameters:
	<b>int pnum</b>\n\n
	Zero based index of which curve to get. */
	DllExport NURBSId GetParentId(int pnum);

	/*! \remarks Sets the surface that's trimmed by specifying it's index into
	the <b>NURBSSet</b>.
	\par Parameters:
	<b>int index</b>\n\n
	The index in the <b>NURBSSet</b> of the surface. */
	DllExport void SetSurfaceParent(int index);
	/*! \remarks Sets the surface that's trimmed by specifying it's
	<b>NURBSId</b>.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	The id of the surface. */
	DllExport void SetSurfaceParentId(NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the surface that
	is trimmed. */
	DllExport int GetSurfaceParent();
	/*! \remarks Returns the <b>NURBSId</b> of the surface that's trimmed. */
	DllExport NURBSId GetSurfaceParentId();

	/*! \remarks Returns the state of the flip trim flag. This controls which
	side of the curve is trimmed from the surface. */
	DllExport BOOL GetFlipTrim();
	/*! \remarks Sets the state of the flip trim flag.
	\par Parameters:
	<b>BOOL flip</b>\n\n
	TRUE to flip; FALSE to not flip.
	\par Operators:
	*/
	DllExport void SetFlipTrim(BOOL flip);
};



/*! \sa  Class NURBSSurface.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class provides access to the NURBS Fillet Surface. Methods are provided to
get / set the parent surfaces, get / set the radius, and get / set the trim and
trim flip settings. <br>  protected: */
class NURBSFilletSurface : public NURBSSurface {
	friend class NURBSSet;
	// parent 0 should be the surface parent 1 should be the curve
	NURBSId mParentId[2];
	int mParentIndex[2];
	BOOL mCubic;
	float mRadius[2];
	Point2 mSeed[2];
    BOOL mTrimSurface[2];
    BOOL mFlipTrim[2];
	/*! \remarks	This methods breaks the relation between this <b>NURBSObject</b> and a
	<b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSIdTab ids</b>\n\n
	A table with the IDs of each object in the <b>NURBSSet</b>.\n\n
	*/
	DllExport void Clean(NURBSIdTab ids);
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mType = kNFilletSurface;</b>\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpNSet = NULL;</b>\n\n
	<b> mParentId[0] = mParentId[1] = 0;</b>\n\n
	<b> mParentIndex[0] = mParentIndex[1] = -1;</b>\n\n
	<b> mRadius[0] = mRadius[1] = 10.0f;</b>\n\n
	<b> mSeed[0] = mSeed[1] = Point2(0.0, 0.0);</b>\n\n
	<b> mCubic = TRUE;</b> */
	DllExport NURBSFilletSurface(void);
	/*! \remarks Destructor. */
	DllExport virtual ~NURBSFilletSurface(void);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSFilletSurface\& curve</b>\n\n
	The surface to assign. */
	DllExport NURBSFilletSurface & operator=(const NURBSFilletSurface& curve);
	/*! \remarks Sets the specified parent surface (by <b>NURBSSet</b> index)
	for the fillet.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent surface: 0 or 1.\n\n
	<b>int index</b>\n\n
	The index in the <b>NURBSSet</b>. */
	DllExport void SetParent(int pnum, int index);
	/*! \remarks Sets the specified parent surface (by <b>NURBSId</b>) for the
	fillet.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent surface: 0 or 1.\n\n
	<b>NURBSId id</b>\n\n
	The ID of the surface. */
	DllExport void SetParentId(int pnum, NURBSId id);
	/*! \remarks Returns the index in the <b>NURBSSet</b> of the specified
	parent surface.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent surface: 0 or 1. */
	DllExport int GetParent(int pnum);
	/*! \remarks Returns the NURBSId of the specified parent surface.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent surface: 0 or 1. */
	DllExport NURBSId GetParentId(int pnum);
	/*! \remarks Returns the UV location of the seed value on the specified
	surface.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent surface: 0 or 1. */
	DllExport Point2 GetSeed(int pnum);
	/*! \remarks Sets the UV location of the seed value on the specified
	surface.
	\par Parameters:
	<b>int pnum</b>\n\n
	The parent surface: 0 or 1.\n\n
	<b>Point2\& seed</b>\n\n
	The seed value to set. */
	DllExport void SetSeed(int pnum, Point2& seed);
	/*! \remarks Returns TRUE if the cubic setting is on; FALSE if off. When
	off, the radius is always linear. When on, the radius is treated as a cubic
	function, allowing it to change based on the parent surface's geometry. */
	DllExport BOOL IsCubic();
	/*! \remarks Sets the cubic setting to on or off.
	\par Parameters:
	<b>BOOL cubic</b>\n\n
	TRUE for on; FALSE for off. */
	DllExport void SetCubic(BOOL cubic);
	/*! \remarks Returns the specified radius at the time passed.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to get the radius.\n\n
	<b>int rnum</b>\n\n
	Specifies which radius to get: 0 for start, 1 for end. */
	DllExport float GetRadius(TimeValue t, int rnum);
	/*! \remarks Sets the specified radius at the time passed.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the radius.\n\n
	<b>int rnum</b>\n\n
	Specifies which radius to set: 0 for start, 1 for end.\n\n
	<b>float radius</b>\n\n
	The radius to set. */
	DllExport void SetRadius(TimeValue t, int rnum, float radius);

    /*! \remarks Returns the trim surface setting for the specified parent
    surface. TRUE if on (trims the parent surface at the edge of the fillet);
    FALSE if off.
    \par Parameters:
    <b>int pnum</b>\n\n
    The parent surface: 0 or 1. */
    DllExport BOOL GetTrimSurface(int pnum);
    /*! \remarks Sets the trim surface setting for the specified parent
    surface.
    \par Parameters:
    <b>int pnum</b>\n\n
    The parent surface: 0 or 1.\n\n
    <b>BOOL trim</b>\n\n
    TRUE for on (trims the parent surface at the edge of the fillet); FALSE for
    off. */
    DllExport void SetTrimSurface(int pnum, BOOL trim);
    /*! \remarks Returns the state of the trim flip setting. When set this
    reverses the direction of the trim.
    \par Parameters:
    <b>int pnum</b>\n\n
    The parent surface: 0 or 1. */
    DllExport BOOL GetFlipTrim(int pnum);
    /*! \remarks Sets the state of the trim flip setting. When set this
    reverses the direction of the trim.
    \par Parameters:
    <b>int pnum</b>\n\n
    The parent surface: 0 or 1.\n\n
    <b>BOOL flip</b>\n\n
    TRUE for on; FALSE for off. */
    DllExport void SetFlipTrim(int pnum, BOOL flip);
};



/*! \sa  Class NURBSSet.\n\n
\par Description:
This class is available in release 2.5 or later.\n\n
This class provides information about the display of the <b>NURBSSet</b> in the
3D viewports. An instance of this class is maintained by each <b>NURBSSet</b>.
\par Data Members:
<b>BOOL mDisplayCurves;</b>\n\n
TRUE if curves are displayed; otherwise FALSE.\n\n
<b>BOOL mDisplaySurfaces;</b>\n\n
TRUE if surfaces are displayed; otherwise FALSE.\n\n
<b>BOOL mDisplayLattices;</b>\n\n
TRUE if lattices are displayed; otherwise FALSE.\n\n
<b>BOOL mDisplaySurfCVLattices;</b>\n\n
TRUE if surface CV lattices are displayed; otherwise FALSE.\n\n
<b>BOOL mDisplayCurveCVLattices;</b>\n\n
TRUE if curve CV lattices are displayed; otherwise FALSE.\n\n
<b>BOOL mDisplayDependents;</b>\n\n
TRUE if dependent sub-objects are displayed; otherwise FALSE.\n\n
<b>BOOL mDisplayTrimming;</b>\n\n
TRUE if surface trimming is displayed; otherwise FALSE.\n\n
<b>BOOL mDegradeOnMove;</b>\n\n
TRUE if the surface may degrade while transforming it; otherwise FALSE.  */
class NURBSDisplay: public MaxHeapOperators {
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mDisplayCurves = TRUE;</b>\n\n
	<b> mDisplaySurfaces = TRUE;</b>\n\n
	<b> mDisplayLattices = FALSE;</b>\n\n
	<b> mDisplaySurfCVLattices = TRUE;</b>\n\n
	<b> mDisplayCurveCVLattices = TRUE;</b>\n\n
	<b> mDisplayDependents = TRUE;</b>\n\n
	<b> mDisplayTrimming = TRUE;</b>\n\n
	<b> mDegradeOnMove = TRUE;</b> */
	DllExport NURBSDisplay();
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const NURBSDisplay\& disp</b>\n\n
	The object to assign. */
	DllExport NURBSDisplay & operator=(const NURBSDisplay& disp);

	BOOL mDisplayCurves;
	BOOL mDisplaySurfaces;
	BOOL mDisplayLattices;
	BOOL mDisplaySurfCVLattices;
	BOOL mDisplayCurveCVLattices;
	BOOL mDisplayDependents;
	BOOL mDisplayTrimming;
	BOOL mDegradeOnMove;
    BOOL mDisplayShadedLattice;
};



/*! \sa  Class NURBSSurface, Class NURBSSet, Class NURBSControlVertex, Class NURBSPoint, Class NURBSCurve, Class NURBSFuseCurveCV.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class may be used with a NURBSSet to fuse two CVs in a surface. This
causes the CVs to reference one another so if you move one the other moves with
it.\n\n
All methods of this class are implemented by the system.
\par Data Members:
<b>int mSurf1, mSurf2;</b>\n\n
The zero based indices of the surfaces to fuse. These may be the same value.
Note that this is <b>not</b> the index in the <b>NURBSSet</b> of these items.
Rather it is the index of CV surface in list of CV surfaces in the NURBSSet.
For instance, if there were first two CV curves and then two CV surfaces in the
<b>NURBSSet</b>, <b>mSurf1</b> would be 0 and <b>mSurf2</b> would be 1 since
the CV curves don't count.\n\n
<b>int mRow1, mCol1, mRow2, mCol2;</b>\n\n
The indices of the row and column CVs of surface 1 and surface 2 to fuse. <br>
*/
class NURBSFuseSurfaceCV: public MaxHeapOperators {
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b>mSurf1 = mSurf2 = 0;</b>\n\n
	<b>mRow1 = mCol1 = mRow2 = mCol2 = 0;</b> */
	DllExport NURBSFuseSurfaceCV();
	int mSurf1, mSurf2;
	int mRow1, mCol1, mRow2, mCol2;
};


/*! \sa  Class NURBSSurface, Class NURBSSet, Class NURBSControlVertex, Class NURBSPoint, Class NURBSCurve, Class NURBSFuseSurfaceCV.\n\n
\par Description:
This class is available in release 2.0 and later only.<br> This class may be
used with a NURBSSet to fuse two CVs in a curve. This causes the CVs to
reference one another so if you move one the other moves with it.\n\n
All methods of this class are implemented by the system.
\par Data Members:
<b>int mCurve1, mCurve2;</b>\n\n
The zero based indices of the curves to fuse. These may be the same value. Note
that this is <b>not</b> the index in the <b>NURBSSet</b> of these items. Rather
it is the index of CV curve in list of CV curves in the <b>NURBSSet</b>. For
instance, if there were first two CV surfaces and then two CV curves in the
<b>NURBSSet</b>, <b>mCurve1</b> would be 0 and <b>mCurve2</b> would be 1 since
the CV surfaces don't count.\n\n
<b>int mCV1, mCV2;</b>\n\n
The indices of the CVs of curve1 and curve2 to fuse.  */
class NURBSFuseCurveCV: public MaxHeapOperators {
public:
	/*! \remarks Constructor.  The data members are initialized as
	follows:\n\n
	<b>mCurve1 = mCurve2 = 0;</b>\n\n
	<b>mCV1 = mCV2 = 0;</b> */
	DllExport NURBSFuseCurveCV();
	int mCurve1, mCurve2;
	int mCV1, mCV2;
};

/*! \sa  Class NURBSObject, Class NURBSDisplay, Class TessApprox, Class NURBSFuseSurfaceCV, Class NURBSFuseCurveCV, Template Class Tab.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class contains a table of <b>NURBSObject</b> entities used to make up the
set. Additionally it has two fuse tables: one for fuse curves and one for fuse
surfaces. These are used to allow the CVs in the curves or surfaces to be
'stitched' together so if one curve or surface moves the other moves with it.
This class also has information required to tessellate the objects to triangle
meshes for use in the viewports and the production renderer.\n\n
All methods of this class are implemented by the system.
\par Data Members:
protected:\n\n
<b>TessApprox *mpVTess;</b>\n\n
This object describes the properties of a tesselation approximation to the
mathematical surface for representation in the viewports.\n\n
<b>TessApprox *mpRTess;</b>\n\n
This object describes the properties of a tesselation approximation to the
mathematical surface for the production renderer.\n\n
<b>TessApprox *mpRTessDisp;</b>\n\n
This object describes the tesselation properties for displacement mapping in
the production renderer.\n\n
<b>TessApprox *mpVTessCurve;</b>\n\n
This object describes the properties of a tesselation approximation to the
mathematical curve for representation in the viewports.\n\n
<b>TessApprox *mpRTessCurve;</b>\n\n
This object describes the properties of a tesselation approximation to the
mathematical curve for the production renderer.\n\n
<b>float mTessMerge;</b>\n\n
Controls the tessellation of surface sub-objects whose edges are joined or very
nearly joined. When input to a modifier -- such as Mesh Select -- requires a
mesh, and when NURBS surfaces are tessellated for production rendering, by
default 3ds Max adjusts the tessellation of adjoining surfaces to match each
other, in terms of the number of faces along the edges. The Merge parameter
controls how this is done. If Merge is zero, adjoining faces are unchanged.
Increasing the value of Merge increases the distance 3ds Max uses to calculate
how edges should match, guaranteeing no gaps between the surfaces when they are
rendered.\n\n
<b>Tab\<NURBSObject*\> mObjects;</b>\n\n
A table of pointers to the <b>NURBSObject</b>s in the set.\n\n
<b>Object *mpObject;</b>\n\n
The instantiated object in the scene associated with this <b>NURBSSet</b>. This
is NULL if there isn't one.\n\n
<b>NURBSDisplay mDisplay;</b>\n\n
Controls the display of the object in the viewport..\n\n
<b>Tab\<NURBSFuseSurfaceCV\> mSurfFuse;</b>\n\n
A table of objects used to allow surfaces in the set to relate to on
another.\n\n
<b>Tab\<NURBSFuseCurveCV\> mCurveFuse;</b>\n\n
A table of objects used to allow curves in the set to relate to on another.
 */
class NURBSSet: public MaxHeapOperators {
protected:
	friend DllExport Object* CreateNURBSObject(IObjParam* ip, NURBSSet *nset, Matrix3& mat);
	friend DllExport int AddNURBSObjects(Object* MAXobj, IObjParam* ip, NURBSSet *nset);
    friend DllExport BOOL GetNURBSSet(Object *object, TimeValue t, NURBSSet &nset, BOOL Relational);
	TessApprox *mpVTess;
	TessApprox *mpRTess;
	// new for R3 -- optional
	TessApprox *mpRTessDisp;
	TessApprox *mpVTessCurve;
	TessApprox *mpRTessCurve;

	float mTessMerge;
	Tab<NURBSObject*> mObjects;
	Object *mpObject;
	NURBSDisplay mDisplay;

public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> mpObject = NULL;</b>\n\n
	<b> mpVTess = NULL;</b>\n\n
	<b> mpRTess = NULL;</b>\n\n
	<b> mpRTessDisp = NULL;</b>\n\n
	<b> mpVTessCurve = NULL;</b>\n\n
	<b> mpRTessCurve = NULL;</b>\n\n
	<b> mTessMerge = 0.01f;</b>\n\n
	<b> mDisplay.mDisplayCurves = TRUE;</b>\n\n
	<b> mDisplay.mDisplaySurfaces = TRUE;</b>\n\n
	<b> mDisplay.mDisplayLattices = FALSE;</b>\n\n
	<b> mDisplay.mDisplayShadedLattice = FALSE;</b>\n\n
	<b> mDisplay.mDisplaySurfCVLattices = TRUE;</b>\n\n
	<b> mDisplay.mDisplayCurveCVLattices = TRUE;</b>\n\n
	<b> mDisplay.mDisplayDependents = TRUE;</b>\n\n
	<b> mDisplay.mDisplayTrimming = TRUE;</b>\n\n
	<b> mDisplay.mDegradeOnMove = TRUE;</b> */
	DllExport NURBSSet(void);
	/*! \remarks Destructor. Any tesselation objects are deleted. */
	DllExport virtual ~NURBSSet(void);
	/*! \remarks	This removes the NURBS Set connection to a live NURBS object. One use of
	this is so you can call <b>CreateNURBSObject()</b> twice with the same
	<b>NURBSSet</b>. You call <b>Clean()</b> in between the calls. Another use
	is if you want to use the API to copy a NURBS object. You could call
	<b>GetNURBSSet()</b> followed by a <b>Clea()</b> followed by a
	<b>CreateNURBSObject()</b>. */
	DllExport void Clean();	// this method removes any relation to a live NURBS object
	/*! \remarks Returns the number of objects in the set. */
	DllExport int GetNumObjects();
	/*! \remarks This method sets an object in the table of objects maintained
	by the set. If the <b>index</b> is to an existing object in the set this
	will replace that object. If it is a new index, all the objects which
	follow this one in the set are set to NULL and the one passed is set.
	\par Parameters:
	<b>int index</b>\n\n
	If the index is an existing object in the set this will replace the object.
	If it is a new index, all the objects which follow this one are set to NULL
	and the one passed is set.\n\n
	<b>NURBSObject* obj</b>\n\n
	Points to the object to add to the table. */
	/*! \remarks Sets the specified NURBSObject pointer in the table to the
	specified pointer. If the value of index is greater than the number of
	items in the table, the table is resized and any non-initialized pointers
	are set to NULL.
	\par Parameters:
	<b>int index</b>\n\n
	The zero based index of the object to set.\n\n
	<b>NURBSObject* obj</b>\n\n
	Points to the object to set. */
	DllExport void SetObject(int index, NURBSObject* obj);
	/*! \remarks Adds the specified object pointer to the end of the table of
	object pointers.\n\n
	Note: This method is ONLY used for adding an object to a <b>NURBSSet</b>
	that is not yet in the scene. To add an object to an existing scene use the
	global function <b>AddNURBSObjects()</b>.
	\par Parameters:
	<b>NURBSObject* obj</b>\n\n
	The pointer to the object to append.
	\return  Returns the number of objects in the table prior to appending. */
	DllExport int AppendObject(NURBSObject* obj);
	/*! \remarks Removes the specified object pointer from the table.
	\par Parameters:
	<b>int index</b>\n\n
	The zero based index of the object to remove. */
	DllExport void RemoveObject(int index);
	/*! \remarks Deletes all the objects that are in the table.\n\n
	This method frees all the <b>NURBSObjects</b> in a <b>NURBSSet</b>. A
	developer using a <b>NURBSSet</b> must call this method to free all the
	memory when done. */
	DllExport void DeleteObjects();
	/*! \remarks Returns a pointer to the specified object in the table.
	\par Parameters:
	<b>int index</b>\n\n
	The zero based index of the object to return. */
	DllExport NURBSObject* GetNURBSObject(int index);
	/*! \remarks Returns a pointer to the specified object in the table.
	\par Parameters:
	<b>NURBSId id</b>\n\n
	The Id of the object to return. */
	DllExport NURBSObject* GetNURBSObject(NURBSId id);
	/*! \remarks	Returns a pointer to the TessApprox object for the production renderer for
	the specified tesselation operation.
	\par Parameters:
	<b>NURBSTessType type=kNTessSurface</b>\n\n
	The type of tesselation. See \ref nurbTessTypes. */
	DllExport TessApprox* GetProdTess(NURBSTessType type=kNTessSurface);
	/*! \remarks	Returns a pointer to the TessApprox object for the viewport renderer for
	the specified tesselation operation.
	\par Parameters:
	<b>NURBSTessType type=kNTessSurface</b>\n\n
	The type of tesselation. See \ref nurbTessTypes. */
	DllExport TessApprox* GetViewTess(NURBSTessType type=kNTessSurface);
	/*! \remarks Sets the TessApprox object for the production renderer for
	the specified tesselation operation.
	\par Parameters:
	<b>TessApprox\& tess</b>\n\n
	The tesselation object.\n\n
	<b>NURBSTessType type=kNTessSurface</b>\n\n
	This parameter is available in release 3.0 and later only.\n\n
	The type of tesselation. See \ref nurbTessTypes. */
	DllExport void SetProdTess(TessApprox& tess, NURBSTessType type=kNTessSurface);
	/*! \remarks Sets the TessApprox object for the viewport renderer for the
	specified tesselation operation.
	\par Parameters:
	<b>TessApprox\& tess</b>\n\n
	The tesselation object.\n\n
	<b>NURBSTessType type=kNTessSurface</b>\n\n
	This parameter is available in release 3.0 and later only.\n\n
	The type of tesselation. See \ref nurbTessTypes. */
	DllExport void SetViewTess(TessApprox& tess, NURBSTessType type=kNTessSurface);
	/*! \remarks	Clears (deletes) the specified tesselation object used for viewport
	rendering.
	\par Parameters:
	<b>NURBSTessType type=kNTessSurface</b>\n\n
	The type of tesselation. See \ref nurbTessTypes. */
	DllExport void ClearViewTess(NURBSTessType type=kNTessSurface);
	/*! \remarks	Clears (deletes) the specified tesselation object used for production
	rendering.
	\par Parameters:
	<b>NURBSTessType type=kNTessSurface</b>\n\n
	The type of tesselation. See \ref nurbTessTypes. */
	DllExport void ClearProdTess(NURBSTessType type=kNTessSurface);
	/*! \remarks Returns the tesselation merge value. */
	DllExport float GetTessMerge();
	/*! \remarks Sets the tesselation merge value.
	\par Parameters:
	<b>float merge</b>\n\n
	The tessellation merge value. Controls the tessellation of surface sub-objects
	whose edges are joined or very nearly joined. When input to a modifier -- such
	as Mesh Select -- requires a mesh, and when NURBS surfaces are tessellated for
	production rendering, by default 3ds Max adjusts the tessellation of adjoining
	surfaces to match each other, in terms of the number of faces along the edges.
	The Merge parameter controls how this is done. If Merge is zero, adjoining
	faces are unchanged. Increasing the value of Merge increases the distance 3ds
	Max uses to calculate how edges should match, guaranteeing no gaps between the
	surfaces when they are rendered. */
	DllExport void SetTessMerge(float merge);
	/*! \remarks	Returns a pointer to the instantiated object in the scene associated with
	this <b>NURBSSet</b>. This is NULL if there isn't one. */
	DllExport Object* GetMAXObject();
	/*! \remarks	Returns the display settings for this <b>NURBSSet</b>. */
	DllExport NURBSDisplay GetDisplaySettings();
	/*! \remarks	Sets the display settings used by this <b>NURBSSet</b>.
	\par Parameters:
	<b>NURBSDisplay\& disp</b>\n\n
	The settings to use. */
	DllExport void SetDisplaySettings(NURBSDisplay& disp);


	Tab<NURBSFuseSurfaceCV> mSurfFuse;
	Tab<NURBSFuseCurveCV> mCurveFuse;
};



/*! \remarks The surface parameter range procedure. This retrieves the minimum
and maximum valid values for u and v as passed to the <b>SurfEvalProc()</b>.
\par Parameters:
<b>double\& uMin</b>\n\n
The min U value.\n\n
<b>double\& uMax</b>\n\n
The max U value.\n\n
<b>double\& vMin</b>\n\n
The min V value.\n\n
<b>double\& vMax</b>\n\n
The max V value.
\return  See \ref nurbResults. */
typedef NURBSResult (*SurfParamRangeProc)(double& uMin, double& uMax, double& vMin, double& vMax);
/*! \remarks This evaluates the surface for the point or position at a given U
and V parameter.
\par Parameters:
<b>double u</b>\n\n
Specifies the U point along the surface to evaluate. This value must be between
the <b>uMin</b> and <b>uMax</b> as returned from
<b>SurfParamRangeProc()</b>.\n\n
<b>double v</b>\n\n
Specifies the V point along the surface to evaluate. This value must be between
the <b>vMin</b> and <b>vMax</b> as returned from
<b>SurfParamRangeProc()</b>.\n\n
<b>Point3\& pt</b>\n\n
The output point on the surface at (u,v).
\return  See \ref nurbResults. */
typedef NURBSResult (*SurfEvalProc)(double u, double v, Point3& pt);
/*! \remarks This evaluates the surface for the partial derivative with
respect to U and the partial derivative with respect to V at a given U and V
parameter. That partial derivative with respect to U is a tangent vector in the
U direction and the partial derivative with respect to V is a tangent vector in
the V direction.
\par Parameters:
<b>double u</b>\n\n
Specifies the U point along the curve to evaluate. This value must be between
the <b>uMin</b> and <b>uMax</b> as returned from
<b>SurfParamRangeProc()</b>.\n\n
<b>double v</b>\n\n
Specifies the V point along the surface to evaluate. This value must be between
the <b>vMin</b> and <b>vMax</b> as returned from
<b>SurfParamRangeProc()</b>.\n\n
<b>Point3\& uTan</b>\n\n
The tangent along u.\n\n
<b>Point3\& vTan</b>\n\n
The tangent along v.
\return  See \ref nurbResults. */
typedef NURBSResult (*SurfEvalTan)(double u, double v, Point3& uTan, Point3& vTan);
/*! \remarks This evaluates the surface for the mixed partial derivative with
respect to U and V at a given U and V parameter.
\par Parameters:
<b>double u</b>\n\n
Specifies the U point along the curve to evaluate. This value must be between
the <b>uMin</b> and <b>uMax</b> as returned from
<b>SurfParamRangeProc()</b>.\n\n
<b>double v</b>\n\n
Specifies the V point along the surface to evaluate. This value must be between
the <b>vMin</b> and <b>vMax</b> as returned from
<b>SurfParamRangeProc()</b>.\n\n
<b>Point3\& mixed</b>\n\n
This is the resulting mixed partial derivative vector that has been evaluated.
\return  See \ref nurbResults.
\par Data Members:
<b>SurfParamRangeProc mParamProc;</b>\n\n
The surface parameter range procedure. Note: This procedure must be
implemented.\n\n
<b>SurfEvalProc mEvalProc;</b>\n\n
The surface evaluation procedure. Note: This procedure must be implemented.\n\n
<b>SurfEvalTan mEvalTanProc;</b>\n\n
The surface tangent evaluation procedure. Note: This procedure is optional.\n\n
<b>SurfEvalMixedProc mEvalMixedProc;</b>\n\n
The mixed partial derivative procedure. Note: This procedure is optional. <br>
*/
typedef NURBSResult (*SurfEvalMixedProc)(double u, double v, Point3& mixed);

// base class for a proceedurally defined surfaces
// NOTE THIS IS NOT SUBCLASSED FROM NURBSObject
// You must use the GenNURBSCVSurfaceProcedurally
/*! \sa  Class NURBSCVSurface,  Class NURBSSurface.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This is the base class for a proceedurally defined surface. Note: This class is
not subclassed from <b>NURBSObject</b>. You must use the
<b>GenNURBSCVSurfaceProcedurally()</b> function.\n\n
The following typedef-ed functions are used by the
<b>GenNURBSCVSurfaceProcedurally()</b> function. */
class NURBSProceeduralSurface: public MaxHeapOperators {
public:
	SurfParamRangeProc	mParamProc;		// this one MUST be implemented
	SurfEvalProc		mEvalProc;		// this one MUST be implemented
	SurfEvalTan			mEvalTanProc;	// this one is optional
	SurfEvalMixedProc	mEvalMixedProc; // this one is optional
	/*! \remarks Constructor. The data members are initialized to the values
	passed.\n\n
	Both <b>(*SurfEvalTan)</b> and <b>(*SurfEvalMixedProc)</b> are optional. If
	NULLs are provided for either of these functions then 3ds Max will resort
	to a finite differences based solution.
	\par Parameters:
	*/
	DllExport NURBSProceeduralSurface(SurfParamRangeProc param, SurfEvalProc eval,
							SurfEvalTan tan, SurfEvalMixedProc mixed);
};
/*! \remarks This global function is available in release 3.0 and later
only.\n\n
To use this function you need to create an instance of
<b>NURBSProceeduralSurface</b> setting at least the ParameterRange and Eval
procedures. The others can be NULL unless you have fast versions of them. This
call then fills in a NURBSCVSurface with the surface that is defined by the
procedures and the tolerence.
\par Parameters:
<b>NURBSProceeduralSurface *pSurf</b>\n\n
Points to the <b>NURBSProceeduralSurface</b> to generate CV surfaces from.\n\n
<b>double tolerence</b>\n\n
The tolerance is the allowable deviation of the approximating NURBS surface to
the surface defined by the procs.\n\n
<b>NURBSCVSurface\& surf</b>\n\n
The generated NURBSCVSurface is returned here.
\return  See \ref nurbResults. */
DllExport NURBSResult GenNURBSCVSurfaceProceedurally(NURBSProceeduralSurface *pSurf, double tolerence, NURBSCVSurface& surf);


/*! \remarks The curve parameter range procedure. This retrieves the minimum
and maximum valid values for u as passed to the <b>CurveEvalProc()</b>.
\par Parameters:
<b>double\& tMin</b>\n\n
The minimum value.\n\n
<b>double\& tMax</b>\n\n
The maximum value.
\return  See \ref nurbResults. */
typedef NURBSResult (*CurveParamRangeProc)(double& tMin, double& tMax);
/*! \remarks The curve evaluation procedure. This retrieves the point on the
curve based on the u parameter.
\par Parameters:
<b>double u</b>\n\n
Specifies the U point along the curve to evaluate.\n\n
<b>Point3\& pt</b>\n\n
The output point on the curve at U.
\return  See \ref nurbResults. */
typedef NURBSResult (*CurveEvalProc)(double u, Point3& pt);
/*! \remarks The curve point and tangent evaluation procedure. This retrieves
the point and tangent at the point on the curve based on the U parameter.
\par Parameters:
<b>double u</b>\n\n
Specifies the point along the curve to evaluate.\n\n
<b>Point3\& pt</b>\n\n
The output point at U.\n\n
<b>Point3\& tan</b>\n\n
The output tangent at U.
\return  See \ref nurbResults. */
typedef NURBSResult (*CurveEvalTan)(double u, Point3& pt, Point3& tan);
/*! \remarks Retrieves the length of the curve.
\par Parameters:
<b>double\& arcLength</b>\n\n
The output arc length.
\return  See \ref nurbResults.
\par Data Members:
<b>CurveParamRangeProc mParamProc;</b>\n\n
The curve parameter range procedure. Note: This procedure must be
implemented.\n\n
<b>CurveEvalProc mEvalProc;</b>\n\n
The curve evaluate procedure. Note: This procedure must be implemented.\n\n
<b>CurveEvalTan mEvalTanProc;</b>\n\n
The curve evaluate tangent procedure. Note: This procedure is optional.\n\n
<b>CurveArcLengthProc mArcLengthProc;</b>\n\n
The curve arc length procedure. Note: This procedure is optional.  */
typedef NURBSResult (*CurveArcLengthProc)(double& arcLength);
// base class for a proceedurally defined curves
// NOTE THIS IS NOT SUBCLASSED FROM NURBSObject
// You must use the GenNURBSCVCurveProcedurally
/*! \sa  Class NURBSCVCurve, Class NURBSCurve.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This is the base class for procedurally defined curves. Note that this is not
subclassed from <b>NURBSObject</b>. You must use the
<b>GenNURBSCVCurveProcedurally()</b> function.\n\n
The following typedef-ed functions are used by the
<b>GenNURBSCVCurveProcedurally()</b> function. */
class NURBSProceeduralCurve: public MaxHeapOperators {
public:
	CurveParamRangeProc	mParamProc;		// this one MUST be implemented
	CurveEvalProc		mEvalProc;		// this one MUST be implemented
	CurveEvalTan		mEvalTanProc;	// this one is optional
	CurveArcLengthProc	mArcLengthProc; // this one is optional
	/*! \remarks Constructor. The data members are initialized to the values
	passed. */
	DllExport NURBSProceeduralCurve(CurveParamRangeProc param, CurveEvalProc eval,
							CurveEvalTan tan, CurveArcLengthProc arclen);
};

/*! \remarks This global function is available in release 3.0 and later
only.\n\n
To use this API you need to create an instance of <b>NURBSProceeduralCurve</b>
setting at least the ParameterRange and Eval procedures. The others can be NULL
unless you have fast versions of them. The call tothis function then fills in a
<b>NURBSCVCurve</b> with the curve that is defined by the proceedures and the
tolerence.
\par Parameters:
<b>*pCrv</b>\n\n
Points to the <b>NURBSProceeduralCurve</b> object used to generate the curve
procedurally.\n\n
<b>double tolerence</b>\n\n
The tolerance is the allowable deviation of the approximating NURBS surface to
the surface defined by the procs.\n\n
<b>NURBSCVCurve\& crv</b>\n\n
The generated curve is returned here.
\return  See \ref nurbResults. */
DllExport NURBSResult GenNURBSCVCurveProceedurally(NURBSProceeduralCurve *pCrv, double tolerence, NURBSCVCurve& crv);






DllExport NURBSResult GenNURBSLatheSurface(NURBSCVCurve& curve, Point3& origin, Point3& north,
									float start, float end, NURBSCVSurface& surf);
DllExport NURBSResult GenNURBSSphereSurface(float radius, Point3& center, Point3& northAxis, Point3& refAxis,
					float startAngleU, float endAngleU, float startAngleV, float endAngleV, BOOL open, NURBSCVSurface& surf);
DllExport NURBSResult GenNURBSCylinderSurface(float radius, float height, Point3& origin, Point3& symAxis, Point3& refAxis,
					float startAngle, float endAngle, BOOL open, NURBSCVSurface& surf);
DllExport NURBSResult GenNURBSConeSurface(float radius1, float radius2, float height, Point3& origin, Point3& symAxis, Point3& refAxis,
					float startAngle, float endAngle, BOOL open, NURBSCVSurface& surf);
DllExport NURBSResult GenNURBSTorusSurface(float majorRadius, float minorRadius, Point3& origin,
					Point3& symAxis, Point3& refAxis, float startAngleU, float endAngleU,
					float startAngleV, float endAngleV, BOOL open, NURBSCVSurface& surf);

DllExport Object *CreateNURBSObject(IObjParam* ip, NURBSSet *nset, Matrix3& mat);
DllExport int AddNURBSObjects(Object* obj, IObjParam* ip, NURBSSet *nset);

DllExport Object *CreateNURBSLatheShape(IObjParam* ip, MSTR name, TimeValue t, ShapeObject *shape,
                     Matrix3& axis, float degrees, int capStart, int capEnd,
                     int capType, BOOL weldCore, BOOL flipNormals, BOOL texturing,
					 int segs, BOOL matIds, BOOL shapeIDs);
DllExport Object *CreateNURBSExtrudeShape(IObjParam* ip, MSTR name, TimeValue t, ShapeObject *shape, float amount,
					   int capStart, int capEnd, int capType, BOOL texturing,
					   BOOL matIds, BOOL shapeIDs);

DllExport BOOL GetNURBSSet(Object *object, TimeValue t, NURBSSet &nset, BOOL Relational);



// modify extant objects
DllExport NURBSResult SetSurfaceApprox(Object* obj, BOOL viewport, TessApprox *tess, BOOL clearSurfs=FALSE);
DllExport NURBSResult SetCurveApprox(Object* obj, BOOL viewport, TessApprox *tess, BOOL clearSurfs);
DllExport NURBSResult SetDispApprox(Object* obj, TessApprox *tess, BOOL clearSurfs);
DllExport NURBSResult SetSurfaceDisplaySettings(Object* obj, NURBSDisplay& disp);
DllExport NURBSResult GetSurfaceDisplaySettings(Object* obj, NURBSDisplay& disp);

DllExport NURBSResult Transform(Object* obj, NURBSIdTab& ids, SetXFormPacket& xPack, Matrix3& mat, TimeValue t);

DllExport NURBSResult BreakCurve(Object* obj, NURBSId id, double u, TimeValue t);
DllExport NURBSResult BreakSurface(Object* obj, NURBSId id, BOOL breakU, double param, TimeValue t);

DllExport NURBSResult JoinCurves(Object* obj, NURBSId id1, NURBSId id2, BOOL begin1, BOOL begin2,
								 double tolerance, double ten1, double ten2, TimeValue t);
DllExport NURBSResult JoinSurfaces(Object* obj, NURBSId id1, NURBSId id2, int edge1, int edge2,
								   double tolerance, double ten1, double ten2, TimeValue t);

DllExport NURBSResult ZipCurves(Object* obj, NURBSId id1, NURBSId id2, BOOL begin1, BOOL begin2,
								 double tolerance, TimeValue t);
DllExport NURBSResult ZipSurfaces(Object* obj, NURBSId id1, NURBSId id2, int edge1, int edge2,
								   double tolerance, TimeValue t);

DllExport NURBSId MakeIndependent(Object* obj, NURBSId id, TimeValue t);
DllExport NURBSId MakeRigid(Object* obj, NURBSId id, TimeValue t);
DllExport void SetApproxPreset(Object* pObj, int i);
DllExport void ToggleShadedLattice(Object* pObj);
DllExport TessApprox* GetTessPreset(int which, int preset);
DllExport void SetTessPreset(int which, int preset, TessApprox& tess);

DllExport Object *BuildEMObjectFromLofterObject(Object *loftObject, double tolerance);
DllExport Object *BuildEMObjectFromPatchObject(Object *patchObject);

typedef Tab<NURBSId> NURBSIdList;

DllExport Object *DetachObjects(TimeValue t, INode *pNode, Object* pobj, NURBSIdList list, char *newObjName, BOOL copy, BOOL relational);


DllExport NURBSSubObjectLevel GetSelectionLevel(Object* pObj);
DllExport NURBSResult SetSelectionLLevel(Object* pObj, NURBSSubObjectLevel level);

DllExport NURBSResult GetSelection(Object* pObj, NURBSSubObjectLevel level,
                                   BitArray& selset);

DllExport NURBSResult SetSelection(Object* pObj, NURBSSubObjectLevel level,
                                   BitArray& selset);

DllExport NURBSResult
MoveCurrentSelection(Object* pObj, NURBSSubObjectLevel level,
                     TimeValue t, Matrix3& partm, Matrix3& tmAxis,
                     Point3& val, BOOL localOrigin);

DllExport NURBSResult
RotateCurrentSelection(Object* pObj, NURBSSubObjectLevel level,
                       TimeValue t, Matrix3& partm, Matrix3& tmAxis,
                       Quat& val, BOOL localOrigin);

DllExport NURBSResult 
ScaleCurrentSelection(Object* pObj, NURBSSubObjectLevel level,
                      TimeValue t, Matrix3& partm, Matrix3& tmAxis,
                      Point3& val, BOOL localOrigin);


// Get the number of sub-objects at a particular level
DllExport int
SubObjectCount(Object* pObj, NURBSSubObjectLevel level);

// number of named sets at a particular level
DllExport int 
NamedSelSetCount(Object* pObj, NURBSSubObjectLevel level);

// Get ith named sel set name
DllExport MCHAR* 
GetNamedSelSetName(Object* pObj, NURBSSubObjectLevel level, int i);  

// Set the bit array to the named selection set
DllExport NURBSResult 
GetNamedSelSet(Object* pObj, NURBSSubObjectLevel level, MCHAR* name, BitArray& selSet);

// Set the named selection set the selection in BitArray
DllExport NURBSResult 
SetNamedSelSet(Object* pObj, NURBSSubObjectLevel level, MCHAR* name, BitArray& sel); 

// Add a new named selection set
DllExport NURBSResult
AppendNamedSelSet(Object* pObj, NURBSSubObjectLevel level, MCHAR* name, BitArray& sel); 

DllExport NURBSResult
DeleteCurrentSelection(Object* pObj, NURBSSubObjectLevel level);

DllExport NURBSResult
MapNURBSIdToSelSetIndex(Object* pObj, NURBSId id, int& index, NURBSSubObjectLevel& level);

DllExport NURBSResult
MapSelSetIndexToNURBSId(Object* pObj, int index, NURBSSubObjectLevel level, NURBSId& id);

DllExport void 
ApplyUVWMapAsTextureSurface(Object* pObj, int type, float utile, float vtile,
                            float wtile, int uflip, int vflip, int wflip, int cap,
                            const Matrix3 &tm,int channel);

// Has the same affect as the "Update" button on the "Surface Mapper" WSM.
DllExport void UpdateSurfaceMapper(Modifier* pMod);