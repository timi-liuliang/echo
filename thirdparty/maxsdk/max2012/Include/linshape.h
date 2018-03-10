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
// FILE:        linshape.h
// DESCRIPTION: Defines a Linear Shape Object Class
// AUTHOR:      Tom Hudson
// HISTORY:     created 31 October 1995
//**************************************************************************/

#pragma once

#include "object.h"

extern CoreExport Class_ID  linearShapeClassID; 

#pragma warning(push)
#pragma warning(disable:4239)

/*! \sa  Class ShapeObject, Class PolyShape, <a href="ms-its:3dsmaxsdk.chm::/shps_shapes_and_splines.html">Working with Shapes and Splines</a>.\n\n
\par Description:
This class represents a linear shape object. This class is similar to a
<b>SplineShape</b> except this class uses a <b>PolyShape</b> as its data while
a <b>SplineShape</b> uses a <b>BezierShape</b> as its data. Therefore this is a
shape made up of entirely linear segments. All methods of this class are
implemented by the system.
\par Data Members:
<b>PolyShape shape;</b>\n\n
The PolyShape that holds the linear shape.  */
class LinearShape : public ShapeObject {			   
	private:
		Interval geomValid;
		Interval topoValid;
		Interval selectValid;
		ChannelMask validBits; // for the remaining constant channels
		void CopyValidity(LinearShape *fromOb, ChannelMask channels);

	protected:
		//  inherited virtual methods for Reference-management
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message );

	public:
		PolyShape		shape;

		/*! \remarks Constructor. */
		CoreExport LinearShape();
		/*! \remarks Destructor. */
		CoreExport ~LinearShape();

		CoreExport LinearShape &operator=(LinearShape &from);
		
		//  inherited virtual methods:

		// From BaseObject
		CoreExport int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
		CoreExport void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
		CoreExport int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		CoreExport CreateMouseCallBack* GetCreateMouseCallBack();
		CoreExport RefTargetHandle Clone(RemapDir& remap);
		// From Object			 
		CoreExport ObjectState Eval(TimeValue time);
		CoreExport Interval ObjectValidity(TimeValue t);

		// The validity interval of channels necessary to do a convert to type
		CoreExport Interval ConvertValidity(TimeValue t);

		// get and set the validity interval for the nth channel
	   	CoreExport Interval ChannelValidity(TimeValue t, int nchan);
		CoreExport void SetChannelValidity(int i, Interval v);
		CoreExport void InvalidateChannels(ChannelMask channels);

		// Deformable object procs	
		int IsDeformable() { return 1; }  
		CoreExport int NumPoints();
		CoreExport Point3 GetPoint(int i);
		CoreExport void SetPoint(int i, const Point3& p);
		CoreExport BOOL IsPointSelected (int i);
		
		CoreExport void PointsWereChanged();
		CoreExport void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm=NULL,BOOL useSel=FALSE );
		CoreExport void Deform(Deformer *defProc, int useSel);

		CoreExport int CanConvertToType(Class_ID obtype);
		CoreExport Object* ConvertToType(TimeValue t, Class_ID obtype);
		CoreExport void FreeChannels(ChannelMask chan);
		CoreExport Object *MakeShallowCopy(ChannelMask channels);
		CoreExport void ShallowCopy(Object* fromOb, ChannelMask channels);
		CoreExport void NewAndCopyChannels(ChannelMask channels);

		CoreExport DWORD GetSubselState();

		// From ShapeObject
		CoreExport ObjectHandle CreateTriObjRep(TimeValue t);  // for rendering, also for deformation		
		CoreExport void GetWorldBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& box );
		CoreExport void GetLocalBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& box );
		CoreExport int NumberOfVertices(TimeValue t, int curve);
		CoreExport int NumberOfCurves();
		CoreExport BOOL CurveClosed(TimeValue t, int curve);
		CoreExport Point3 InterpCurve3D(TimeValue t, int curve, float param, int ptype=PARAM_SIMPLE);
		CoreExport Point3 TangentCurve3D(TimeValue t, int curve, float param, int ptype=PARAM_SIMPLE);
		CoreExport float LengthOfCurve(TimeValue t, int curve);
		CoreExport int NumberOfPieces(TimeValue t, int curve);
		CoreExport Point3 InterpPiece3D(TimeValue t, int curve, int piece, float param, int ptype=PARAM_SIMPLE);
		CoreExport Point3 TangentPiece3D(TimeValue t, int curve, int piece, float param, int ptype=PARAM_SIMPLE);
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		This method provides access to the material IDs of the shape. It
		returns the material ID of the specified segment of the specified curve
		of this shape at the time passed. There is a default implementation so
		there is no need to implement this method if the shape does not support
		material IDs.\n\n
		Note: <b>typedef unsigned short MtlID;</b>
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to evaluate the sub-curve.\n\n
		<b>int curve</b>\n\n
		The zero based index of the curve to evaluate.\n\n
		<b>int piece</b>\n\n
		The sub-curve (segment) to evaluate. */
		CoreExport MtlID GetMatID(TimeValue t, int curve, int piece);
		BOOL CanMakeBezier() { return TRUE; }
		CoreExport void MakeBezier(TimeValue t, BezierShape &shape);
		CoreExport ShapeHierarchy &OrganizeCurves(TimeValue t, ShapeHierarchy *hier = NULL);
		CoreExport void MakePolyShape(TimeValue t, PolyShape &shape, int steps = PSHAPE_BUILTIN_STEPS, BOOL optimize = FALSE);
		CoreExport int MakeCap(TimeValue t, MeshCapInfo &capInfo, int capType);
		CoreExport int MakeCap(TimeValue t, PatchCapInfo &capInfo);

		/*! \remarks Returns the <b>shape</b> data member. */
		PolyShape& GetShape() { return shape; }

		// This does the job of setting all points in the PolyShape to "POLYPT_KNOT"
		// types, and removing the "POLYPT_INTERPOLATED" flag.  This is because the
		// LinearShape knows nothing about its origin
		/*! \remarks This method does the job of setting all points in the
		PolyShape to <b>POLYPT_KNOT</b> types, and removing the
		<b>POLYPT_INTERPOLATED</b> flag. This is because the LinearShape knows
		nothing about its origin. */
		CoreExport void SetPointFlags();

		// Animatable methods

		CoreExport void DeleteThis();
		void FreeCaches() { shape.InvalidateGeomCache(FALSE); }
		Class_ID ClassID() { return linearShapeClassID; }
		CoreExport void GetClassName(MSTR& s);
		void NotifyMe(Animatable* subAnim, int message) { UNUSED_PARAM(subAnim); UNUSED_PARAM(message);}
		int IsKeyable() { return 0;}
		int Update(TimeValue t) { UNUSED_PARAM(t); return 0; }
		BOOL BypassTreeView() { return TRUE; }
		// This is the name that will appear in the history browser.
		CoreExport MCHAR *GetObjectName();

		// IO
		CoreExport IOResult Save(ISave *isave);
		CoreExport IOResult Load(ILoad *iload);

		// RefMaker methods
		CoreExport void RescaleWorldUnits(float f);

		// Flush all caches
		CoreExport void InvalidateGeomCache();
	};				

#pragma warning(pop)

CoreExport ClassDesc* GetLinearShapeDescriptor();

