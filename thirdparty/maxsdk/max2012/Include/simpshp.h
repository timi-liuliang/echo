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
// FILE:        simpshp.h
// DESCRIPTION: Defines a simple shape object class to make
//              procedural shape primitives easier to create
// AUTHOR:      Tom Hudson
// HISTORY:     created 30 October 1995
//**************************************************************************/
#pragma once

#include "object.h"
#include "GraphicsConstants.h"

// Parameter block reference indices
// IMPORTANT: Reference #0 is ShapeObject's parameter block!  (Starting with MAXr4)
#define SHAPEOBJPBLOCK 0	// ShapeObject's parameter block

#pragma warning(push)
#pragma warning(disable:4239 4100)

/*! \sa  Class ShapeObject, Class IParamBlock, Class PolyShape, Class Interval, <a href="ms-its:3dsmaxsdk.chm::/shps_shapes_and_splines.html">Working with Shapes and Splines</a>.\n\n
\par Description:
This class defines a simple shape object to make procedural shape primitives
easier to create. For example, the 3ds Max Helix plug-in is derived from this
class. There are a set of mandatory and optional methods to implement.\n\n
Revised for 3ds Max 2.0 SimpleShape-based objects have a new 'General' rollup,
which contains renderable shape options: Renderable checkbox, Thickness
spinner, and a Mapping coords checkbox. These are supported automatically. To
support the new features of the renderable splines, the derived class of
SimpleShape needs to work with a few new methods -- see
<b>SimpleShapeClone()</b> and <b>ReadyGeneralParameters()</b> below for
details.
\par Data Members:
<b>IParamBlock *pblock;</b>\n\n
The parameter block for managing the shape's parameters.\n\n
<b>static IObjParam *ip;</b>\n\n
This data member is available in release 2.0 and later only.\n\n
This is the interface pointer stored by the class.\n\n
<b>static HWND hGenParams;</b>\n\n
This data member is available in release 2.0 and later only.\n\n
The window handle to the 'General' rollup.\n\n
<b>static BOOL dlgRenderable;</b>\n\n
This data member is available in release 2.0 and later only.\n\n
The 'Renderable' flag in the 'General' rollup.\n\n
<b>static float dlgThickness;</b>\n\n
This data member is available in release 2.0 and later only.\n\n
The 'Thickness' setting in the 'General' rollup.\n\n
<b>static BOOL dlgGenUVs;</b>\n\n
This data member is available in release 2.0 and later only.\n\n
The 'Generate Mapping Coords' flag in the 'General' rollup.\n\n
<b>PolyShape shape;</b>\n\n
The shape cache.\n\n
<b>Interval ivalid;</b>\n\n
The validity interval for the shape cache.\n\n
<b>BOOL suspendSnap;</b>\n\n
A flag to suspend snapping used during the creation process.\n\n
<b>static SimpleShape *editOb;</b>\n\n
The shape that is currently being edited in the command panel.\n\n
<b>static ISpinnerControl *thickSpin;</b>\n\n
Points to the spinner control used for the thickness parameter.  */
class SimpleShape: public ShapeObject
{			   
	public:
		IParamBlock* pblock;

		static IObjParam* ip;
		static HWND hGenParams;
		static BOOL dlgRenderable;
		static float dlgThickness;
		static int dlgSides;
		static float dlgAngle;
		static BOOL dlgGenUVs;
		static ISpinnerControl *thickSpin;

		// Shape cache
		PolyShape shape;
		Interval ivalid;

		// Flag to suspend snapping -- Used during creation
		BOOL suspendSnap;

		CoreExport void UpdateShape(TimeValue t);

		static SimpleShape* editOb;

		/*! \remarks Constructor. */
		CoreExport SimpleShape();
		/*! \remarks Destructor.\n\n
		Clients of SimpleShape need to implement these methods: */
		CoreExport ~SimpleShape();

		void ShapeInvalid() { ivalid.SetEmpty(); }

		//  inherited virtual methods:

		// From BaseObject
		CoreExport int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
		CoreExport void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
		CoreExport int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		CoreExport virtual void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		CoreExport virtual void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		// Return an upcasted version of the parameter block.
		IParamArray* GetParamBlock() {return (IParamArray*)pblock;}
		CoreExport int GetParamBlockIndex(int id);

		// From Object
		CoreExport ObjectState Eval(TimeValue time);
		CoreExport Interval ObjectValidity(TimeValue t);
		CoreExport int CanConvertToType(Class_ID obtype);
		CoreExport Object* ConvertToType(TimeValue t, Class_ID obtype);
		CoreExport void BuildMesh(TimeValue t, Mesh &mesh);
				
		// From ShapeObject
		CoreExport ObjectHandle CreateTriObjRep(TimeValue t);  // for rendering, also for deformation		
		CoreExport void GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box );
		CoreExport void GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vxt, Box3& box );
		CoreExport void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel );
		CoreExport int NumberOfVertices(TimeValue t, int curve);
		CoreExport int NumberOfCurves();
		CoreExport BOOL CurveClosed(TimeValue t, int curve);
		CoreExport ShapeHierarchy &OrganizeCurves(TimeValue t, ShapeHierarchy *hier=NULL);	// Ready for lofting, extrusion, etc.
		CoreExport void MakePolyShape(TimeValue t, PolyShape &shape, int steps = PSHAPE_BUILTIN_STEPS, BOOL optimize = FALSE);
		CoreExport int MakeCap(TimeValue t, MeshCapInfo &capInfo, int capType);	// Makes a cap out of the shape
		CoreExport int MakeCap(TimeValue t, PatchCapInfo &capInfo);

		int NumRefs() { return 1 + ShapeObject::NumRefs(); }
		CoreExport RefTargetHandle GetReference(int i);
protected:
		CoreExport virtual void SetReference(int i, RefTargetHandle rtarg);		
public:
		CoreExport RefResult NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);

		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
		To support the new features of the renderable splines, in the derived
		class's constructor, call ReadyGeneralParameters(). This will set up
		the general parameters in the base class to the proper defaults.
		Failure to make this call will cause SimpleShape-based objects to be
		created with default general parameters rather than those of the
		previously-created object. */
		CoreExport void ReadyGeneralParameters();
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
		To support the new features of the renderable splines, the derived
		class of SimpleShape needs to, in the Clone method, call this method.
		This will insure that the base class parameters are copied to the
		cloned object. Failure to make this call will cause cloned
		SimpleShape-based objects to revert to the default rendering
		parameters.
		\par Parameters:
		<b>SimpleShape *sshpSource</b>\n\n
		The source shape for the clone. */
		CoreExport void SimpleShapeClone( SimpleShape *sshpSource );

		int NumSubs() { return 1 + ShapeObject::NumSubs(); }  
		CoreExport Animatable* SubAnim(int i);
		CoreExport MSTR SubAnimName(int i);		

		// Animatable methods
		CoreExport void DeleteThis();
		CoreExport void FreeCaches(); 

		// IO
		CoreExport IOResult Save(ISave *isave);
		CoreExport IOResult Load(ILoad *iload);

		CoreExport void SetGenUVs(BOOL sw);
		CoreExport void SetRenderable(BOOL sw);
		LRESULT CALLBACK TrackViewWinProc( HWND hwnd,  UINT message, 
	            WPARAM wParam,   LPARAM lParam ){return(0);}

		void GetClassName(MSTR& s) {s = GetObjectName();}
		void InitNodeName(MSTR& s) {s = GetObjectName();}

		// Clients of SimpleShape need to implement these methods:
	
		virtual Class_ID ClassID() = 0;
		/*! \remarks This method is called to build the shape at the specified time and
		store the result into the PolyShape passed.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to build the shape.\n\n
		<b>PolyShape\& ashape</b>\n\n
		The built shape is stored here. */
		virtual void BuildShape(TimeValue t,PolyShape& ashape) = 0;
		
		virtual RefTargetHandle Clone(RemapDir& remap) = 0;
		
		virtual CreateMouseCallBack* GetCreateMouseCallBack() = 0;
		/*! \remarks This method indicates if the shape may be displayed at the time passed.
		At certain times, for certain shapes, the shape may not be in a
		displayable form. For example, the size of the shape may go to zero at
		a certain point and would be inappropriate to display.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to check.
		\return  TRUE if the shape may be displayed at the specified time;
		otherwise FALSE. */
		virtual BOOL ValidForDisplay(TimeValue t) = 0;
		/*! \remarks This is called if the user interface parameters needs to be updated
		because the user moved to a new time. The UI controls must display
		values for the current time.\n\n
		If the plug-in uses a parameter map for handling its UI, it may call a
		method of the parameter map to handle this:
		<b>ipmapParam-\>Invalidate();</b>\n\n
		If the plug-in does not use parameter maps, it should call the
		<b>SetValue()</b> method on each of its controls that display a value,
		for example the spinner controls. This will cause to the control to
		update the value displayed. The code below shows how this may be done
		for a spinner control. Note that <b>ip</b> and <b>pblock</b> are
		assumed to be initialized interface and parameter block pointers\n\n
		<b>(IObjParam *ip, IParamBlock *pblock).</b>\n\n
		<b>float newval;</b>\n\n
		<b>Interval valid=FOREVER;</b>\n\n
		<b>TimeValue t=ip-\>GetTime();</b>\n\n
		<b>// Get the value from the parameter block at the current
		time.</b>\n\n
		<b>pblock-\>GetValue( PB_ANGLE, t, newval, valid );</b>\n\n
		<b>// Set the value. Note that the notify argument is passed as
		FALSE.</b>\n\n
		<b>// This ensures no messages are sent when the value changes.</b>\n\n
		<b>angleSpin-\>SetValue( newval, FALSE );</b>
		\par Default Implementation:
		<b>{}</b> */
		virtual void InvalidateUI() {}
		/*! \remarks This method returns the parameter dimension of the parameter whose
		index is passed.
		\par Parameters:
		<b>int pbIndex</b>\n\n
		The index of the parameter to return the dimension of.
		\return  Pointer to a ParamDimension. See
		Class ParamDimension.
		\par Default Implementation:
		<b>{return defaultDim;}</b> */
		virtual	ParamDimension *GetParameterDim(int pbIndex) {return defaultDim;}
		/*! \remarks Returns the name of the parameter whose index is passed.
		\par Parameters:
		<b>int pbIndex</b>\n\n
		The index into the parameter block of the parameter to return the name
		of.
		\par Default Implementation:
		<b>{return MSTR(_M("Parameter"));}</b> */
		virtual MSTR GetParameterName(int pbIndex) {return MSTR(_M("Parameter"));}

		// Unlike SimpleSplines, you're probably procedural, so implementing these
		// is a must!
		/*! \remarks This method returns a point interpolated on the entire curve.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to evaluate.\n\n
		<b>int curve</b>\n\n
		The index of the curve to evaluate.\n\n
		<b>float param</b>\n\n
		The 'distance' along the curve where 0 is the start and 1 is the
		end.\n\n
		<b>int ptype=PARAM_SIMPLE</b>\n\n
		The parameter type for spline interpolation. See
		<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_ptypes_for_shape.html">List of Parameter Types
		for Shape Interpolation</a>.
		\return  The interpolated point on the curve. */
		virtual Point3 InterpCurve3D(TimeValue t, int curve, float param, int ptype=PARAM_SIMPLE) = 0;
		/*! \remarks This method returns a tangent vector interpolated on the entire curve.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at which to evaluate the curve.\n\n
		<b>int curve</b>\n\n
		The index of the curve to evaluate.\n\n
		<b>float param</b>\n\n
		The 'distance' along the curve where 0.0 is the start and 1.0 is the
		end.\n\n
		<b>int ptype=PARAM_SIMPLE</b>\n\n
		The parameter type for spline interpolation. See
		<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_ptypes_for_shape.html">List of Parameter Types
		for Shape Interpolation</a>.
		\return  The tangent vector. */
		virtual Point3 TangentCurve3D(TimeValue t, int curve, float param, int ptype=PARAM_SIMPLE) = 0;
		/*! \remarks Returns the length of the specified curve.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at which to compute the length.\n\n
		<b>int curve</b>\n\n
		The index of the curve. */
		virtual float LengthOfCurve(TimeValue t, int curve) = 0;
		
		// Here are some optional methods.
		// You should _really_ implement these, because they just do the bare-minimum job
		// (Chopping your curve up into manageable pieces makes things look better)
		/*! \remarks Returns the number of sub-curves in a curve.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at which to check.\n\n
		<b>int curve</b>\n\n
		The index of the curve.
		\par Default Implementation:
		<b>{ return 1; }</b> */
		virtual int NumberOfPieces(TimeValue t, int curve) { return 1; }
		/*! \remarks This method returns the interpolated point along the specified
		sub-curve.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to evaluate the sub-curve.\n\n
		<b>int curve</b>\n\n
		The curve to evaluate.\n\n
		<b>int piece</b>\n\n
		The sub-curve (segment) to evaluate.\n\n
		<b>float param</b>\n\n
		The position along the sub-curve to return where 0.0 is the start and
		1.0 is the end.\n\n
		<b>int ptype=PARAM_SIMPLE</b>\n\n
		The parameter type for spline interpolation. See
		<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_ptypes_for_shape.html">List of Parameter Types
		for Shape Interpolation</a>.
		\return  The point in world space.
		\par Default Implementation:
		<b>{ return InterpCurve3D(t, curve, param); }</b> */
		virtual Point3 InterpPiece3D(TimeValue t, int curve, int piece, float param, int ptype=PARAM_SIMPLE) { return InterpCurve3D(t, curve, param, ptype); }
		/*! \remarks Returns the tangent vector on a sub-curve at the specified 'distance'
		along the curve.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to evaluate the sub-curve.\n\n
		<b>int curve</b>\n\n
		The curve to evaluate.\n\n
		<b>int piece</b>\n\n
		The sub-curve (segment) to evaluate.\n\n
		<b>float param</b>\n\n
		The position along the sub-curve to return where 0.0 is the start and
		1.0 is the end.\n\n
		<b>int ptype=PARAM_SIMPLE</b>\n\n
		The parameter type for spline interpolation. See
		<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_ptypes_for_shape.html">List of Parameter Types
		for Shape Interpolation</a>.
		\return  The tangent vector.
		\par Default Implementation:
		<b>{ return TangentCurve3D(t, curve, param, ptype); }</b> */
		virtual Point3 TangentPiece3D(TimeValue t, int curve, int piece, float param, int ptype=PARAM_SIMPLE) { return TangentCurve3D(t, curve, param, ptype); }
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns the material ID of the specified segment of the specified curve
		or the shape.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at which to return the material ID\n\n
		<b>int curve</b>\n\n
		The zero based index of the curve.\n\n
		<b>int piece</b>\n\n
		The zero based index of the segment of the curve. */
		CoreExport virtual MtlID GetMatID(TimeValue t, int curve, int piece);
	};				

#pragma warning(pop)
