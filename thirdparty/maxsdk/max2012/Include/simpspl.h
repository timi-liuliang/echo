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
// FILE:        simpspl.h
// DESCRIPTION: Defines a simple spline object class to make spline
//              primitives easier to create
// AUTHOR:      Tom Hudson
// HISTORY:     created 3 October 1995
//**************************************************************************/
#pragma once

#include "iparamm.h"
#include "object.h"
#include "GraphicsConstants.h"

// Special dialog handling
class SimpleSpline;

class SimpleSplineDlgProc : public ParamMapUserDlgProc {
	private:
		SimpleSpline *spl;
	public:
		SimpleSplineDlgProc(SimpleSpline *s) { spl = s; }
		CoreExport INT_PTR DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		CoreExport void DeleteThis();
	};

#pragma warning(push)
#pragma warning(disable:4239)

/*! \sa  Class ShapeObject.\n\n
\par Description:
Defines a simple spline object class to make spline primitives easier to
create. This class provides default implementations for most of the
<b>ShapeObject</b> methods. The plug-in derived from <b>SimpleSpline</b> must
only implement a handful of methods to create a shape plug-in.\n\n
SimpleSpline plug-ins use a Super Class ID of <b>SHAPE_CLASS_ID</b>.
\par Data Members:
<b>IParamBlock *ipblock;</b>\n\n
Interpolation parameter block (handled by <b>SimpleSpline</b>).\n\n
<b>IParamBlock *pblock;</b>\n\n
User's parameter block. See Class IParamBlock.\n\n
<b>static IParamMap *ipmapParam;</b>\n\n
The parameter map. See Class IParamMap.\n\n
<b>static int dlgSteps;</b>\n\n
The dialog steps settings.\n\n
<b>static BOOL dlgOptimize;</b>\n\n
The dialog Optimize toggle.\n\n
<b>static BOOL dlgAdaptive;</b>\n\n
The dialog Adaptive toggle.\n\n
<b>BezierShape shape;</b>\n\n
The Spline cache.\n\n
<b>Interval ivalid;</b>\n\n
The validity interval for the spline. See Class Interval.\n\n
<b>BOOL suspendSnap;</b>\n\n
Flag to suspend snapping used during creation.\n\n
<b>static SimpleSpline *editOb;</b>\n\n
This is the spline being edited in the command panel.  */
class SimpleSpline: public ShapeObject {			   
	private:
	public:
		IParamBlock *ipblock;	// Interpolation parameter block (handled by SimpleSpline)
		IParamBlock *pblock;	// User's parameter block

		static IParamMap *ipmapParam;
		static int dlgSteps;
		static BOOL dlgOptimize;
		static BOOL dlgAdaptive;

		// Spline cache
		BezierShape shape;
		Interval ivalid;

		// Flag to suspend snapping -- Used during creation
		BOOL suspendSnap;

		CoreExport void UpdateShape(TimeValue t);

		static SimpleSpline *editOb;

		/*! \remarks Constructor. The validity interval is set to empty, and
		the pblocks are set to NULL. */
		CoreExport SimpleSpline();
		/*! \remarks Destructor.\n\n
		Clients of SimpleSpline need to implement these methods: */
		CoreExport ~SimpleSpline();

		void ShapeInvalid() { ivalid.SetEmpty(); }

		//  inherited virtual methods:

		// From BaseObject
		CoreExport int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
		CoreExport void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
		CoreExport int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		CoreExport virtual void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		/*! \remarks This method is called when the user is finished editing object's
		parameters. The system passes a flag into the <b>EndEditParams()</b>
		method to indicate if the rollup page should be removed. If this flag
		is TRUE, the plug-in must un-register the rollup page, and delete it
		from the panel.
		\par Parameters:
		<b>IObjParam *ip</b>\n\n
		This is an interface pointer passed in. The developer may use the
		interface pointer to call methods such as
		<b>DeleteRollupPage()</b>.\n\n
		<b>ULONG flags</b>\n\n
		The following flag may be set:\n\n
		<b>END_EDIT_REMOVEUI</b>\n\n
		If TRUE, the item's user interface should be removed.\n\n
		<b>Animatable *next</b>\n\n
		This parameter may be used in the motion and hierarchy branches of the
		command panel. This pointer allows a plug-in to look at the ClassID of
		the next item that was being edited, and if it is the same as this
		item, to not replace the entire UI in the command panel. Note that for
		items that are edited in the modifier branch this field can be ignored.
		*/
		CoreExport virtual void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		IParamArray* GetParamBlock() {return (IParamArray*)pblock;}
		CoreExport int GetParamBlockIndex(int id);

		// From Object
		CoreExport ObjectState Eval(TimeValue time);
		CoreExport Interval ObjectValidity(TimeValue t);
		CoreExport int CanConvertToType(Class_ID obtype);
		CoreExport Object* ConvertToType(TimeValue t, Class_ID obtype);
		CoreExport void GetCollapseTypes(Tab<Class_ID> &clist,Tab<MSTR*> &nlist);

		CoreExport void BuildMesh(TimeValue t, Mesh &mesh);
		
		// From ShapeObject
		CoreExport ObjectHandle CreateTriObjRep(TimeValue t);  // for rendering, also for deformation		
		CoreExport void GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box );
		CoreExport void GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vxt, Box3& box );
		CoreExport void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel );
		CoreExport int NumberOfVertices(TimeValue t, int curve);
		CoreExport int NumberOfCurves();
		CoreExport BOOL CurveClosed(TimeValue t, int curve);
		CoreExport Point3 InterpCurve3D(TimeValue t, int curve, float param, int ptype=PARAM_SIMPLE);
		CoreExport Point3 TangentCurve3D(TimeValue t, int curve, float param, int ptype=PARAM_SIMPLE);
		CoreExport float LengthOfCurve(TimeValue t, int curve);
		CoreExport int NumberOfPieces(TimeValue t, int curve);
		CoreExport Point3 InterpPiece3D(TimeValue t, int curve, int piece, float param, int ptype=PARAM_SIMPLE);
		CoreExport Point3 TangentPiece3D(TimeValue t, int curve, int piece, float param, int ptype=PARAM_SIMPLE);
		CoreExport MtlID GetMatID(TimeValue t, int curve, int piece);
		BOOL CanMakeBezier() { return TRUE; }			// Return TRUE if can turn into a bezier representation
		CoreExport void MakeBezier(TimeValue t, BezierShape &shape);	// Create the bezier representation
		CoreExport ShapeHierarchy &OrganizeCurves(TimeValue t, ShapeHierarchy *hier=NULL);	// Ready for lofting, extrusion, etc.
		CoreExport void MakePolyShape(TimeValue t, PolyShape &shape, int steps = PSHAPE_BUILTIN_STEPS, BOOL optimize = FALSE);
		CoreExport int MakeCap(TimeValue t, MeshCapInfo &capInfo, int capType);	// Makes a cap out of the shape
		CoreExport int MakeCap(TimeValue t, PatchCapInfo &capInfo);

		int NumRefs() { return 2 + ShapeObject::NumRefs();}
		CoreExport RefTargetHandle GetReference(int i);
protected:
		CoreExport virtual void SetReference(int i, RefTargetHandle rtarg);		
public:
		CoreExport RefResult NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);

		CoreExport void ReadyInterpParameterBlock();
		void UnReadyInterpParameterBlock() { ipblock = NULL; }

		// When clients are cloning themselves, they should call this 
		// method on the clone to copy SimpleSpline's data.
		// NOTE: DEPRECATED! Use SimpleSplineClone(SimpleSpline *ssplSource, RemapDir& remap )
		CoreExport void SimpleSplineClone( SimpleSpline *ssplSource );

		// When clients are cloning themselves, they should call this 
		// method on the clone to copy SimpleSpline's data.
		CoreExport void SimpleSplineClone( SimpleSpline *ssplSource, RemapDir& remap );

		int NumSubs() { return 2 + ShapeObject::NumSubs(); }  
		CoreExport Animatable* SubAnim(int i);
		CoreExport MSTR SubAnimName(int i);		

		// Animatable methods
		CoreExport void DeleteThis();
		CoreExport void FreeCaches(); 

		// IO
		CoreExport IOResult Save(ISave *isave);
		CoreExport IOResult Load(ILoad *iload);
	#pragma warning(push)
	#pragma warning(disable:4100)
		LRESULT CALLBACK TrackViewWinProc( HWND hwnd,  UINT message, 
	            WPARAM wParam,   LPARAM lParam ){return(0);}

		/*! \remarks Retrieves the name of the plug-in class. This is used internally for
		debugging purposes.
		\par Parameters:
		<b>MSTR\& s</b>\n\n
		The name is stored here. */
		void GetClassName(MSTR& s) {s = GetObjectName();}
		/*! \remarks This method retrieves the default name of the node when it is created.
		\par Parameters:
		<b>MSTR\& s</b>\n\n
		The name is stored here. */
		void InitNodeName(MSTR& s) {s = GetObjectName();}

		// Clients of SimpleSpline need to implement these methods:
	
		/*! \remarks Returns the unique Class_ID of the plug-in. See
		Class Class_ID for more details. */
		virtual Class_ID ClassID() = 0;
		/*! \remarks This method is called to build the shape at the specified time and
		store the results in <b>ashape</b>.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to build the shape.\n\n
		<b>BezierShape\& ashape</b>\n\n
		The created shape is store here. */
		virtual void BuildShape(TimeValue t,BezierShape& ashape) = 0;
		/*! \remarks This method is called to have the plug-in clone itself. 
		The plug-in should clone all its references as well.
		\par Parameters:
		<b>RemapDir \&remap</b>\n\n
		This class is used for remapping references during a Clone. 
		\see class RemapDir
		\return  A pointer to the cloned item. */
		
		virtual RefTargetHandle Clone(RemapDir& remap) = 0;
		
		/*! \remarks This method allows the system to retrieve a callback object used in
		creating the shape in the 3D viewports. This method returns a pointer
		to an instance of a class derived from <b>CreateMouseCallBack</b>. This
		class has a method <b>proc()</b> which is where the developer defines
		the user/mouse interaction used during the shape creation phase.
		\return  A pointer to an instance of a class derived from
		CreateMouseCallBack. */
		virtual CreateMouseCallBack* GetCreateMouseCallBack() = 0;
		/*! \remarks Returns TRUE if it is okay to display the shape at the time passed;
		otherwise FALSE. Certain shapes may not want to be displayed at a
		certain time, for example if their size goes to zero at some point.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to check. */
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
		<b>angleSpin-\>SetValue( newval, FALSE );</b> */
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
		virtual ParamDimension *GetParameterDim(int pbIndex) {return defaultDim;}
		/*! \remarks Returns the name of the parameter whose index is passed.
		\par Parameters:
		<b>int pbIndex</b>\n\n
		The index into the parameter block of the parameter to return the name
		of.
		\par Default Implementation:
		<b>{return MSTR(_M("Parameter"));}</b> */
		virtual MSTR GetParameterName(int pbIndex) {return MSTR(_M("Parameter"));}
		#pragma warning(pop)
		/*! \remarks Returns TRUE if the Simple Spline should display vertex
		ticks during its creation; otherwise FALSE.
		\par Default Implementation:
		<b>{ return TRUE; }</b> */
		virtual BOOL DisplayVertTicksDuringCreation() { return TRUE; }
	};				
#pragma warning(pop)


