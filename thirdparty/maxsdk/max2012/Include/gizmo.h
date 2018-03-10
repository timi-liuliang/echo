/**********************************************************************
 *<
	FILE: gizmo.h

	DESCRIPTION: An apparatus object

	CREATED BY: Rolf Berteig

	HISTORY: 4-15-96

 *>	Copyright (c) 1996 Rolf Berteig, All Rights Reserved.
 **********************************************************************/
#pragma once
#include "object.h"
#include "GraphicsViewportColorConstants.h"
#include "gfx.h"

/*! \sa  Class HelperObject, Class IParamMap, Class IParamBlock, Class Interface, Class ParamDimension.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This is the base class for the creation of atmospheric gizmo objects. It
provides implementations of most of the methods needed. Developers will
probably want to provide their own implementations of <b>BeginEditParams()</b>
and <b>EndEditParams()</b> and call the <b>GizmoObject</b> implementations from
within their implementation. See the source code for <b>GizmoObject</b> in
<b>/MAXSDK/SAMPLES/HOWTO/MISC/GIZMO.CPP</b> to see how the base class methods
are implemented.\n\n
Note that developers must use the data members below as the base class code
relies on these being used.
\par Data Members:
<b>IParamBlock *pblock;</b>\n\n
This is a pointer to the parameter block that should be used to manage the
animated parameters of the gizmo object.\n\n
<b>static IParamMap *pmapParam;</b>\n\n
This is a pointer to the parameter map that should be used to manage the user
interface for the gizmo object.\n\n
<b>static IObjParam *ip;</b>\n\n
This is the interface pointer for the gizmo object.\n\n
<b>static GizmoObject *editOb;</b>\n\n
This is a pointer to the current gizmo object being edited in the command
panel. When <b>BeginEditParams()</b> is called, this pointer is set to the
<b>this</b> pointer of the <b>GizmoObject</b> being edited.  */
#pragma warning(push)
#pragma warning(disable:4100)
class GizmoObject : public HelperObject {
	public:
		IParamBlock *pblock;		
		static IParamMap *pmapParam;
		static IObjParam *ip;

		/*! \remarks Constructor. The parameter block pointer is set to NULL.
		*/
		CoreExport GizmoObject();
		/*! \remarks Destructor. All references are deleted from the gizmo object. 
		*/
		CoreExport ~GizmoObject();

		CoreExport static GizmoObject *editOb;

		// From BaseObject
		CoreExport void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		CoreExport void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		CoreExport int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);		
		CoreExport int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);		

		// From Object
		ObjectState Eval(TimeValue time) {return ObjectState(this);}
		void InitNodeName(MSTR& s) {s = GetObjectName();}		
		CoreExport int CanConvertToType(Class_ID obtype);
		CoreExport Object* ConvertToType(TimeValue t, Class_ID obtype);		
		CoreExport void GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box );
		CoreExport void GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box );
		CoreExport void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel );		

		// Animatable methods		
		int NumSubs() { return 1; }
		// Gets the Sub Animatable parameter block. Upcasts a Parameter block to an Animatable
		Animatable* SubAnim(int i) { return (Animatable*) pblock; }
		MSTR SubAnimName(int i) {return _M("Parameters");}

		// From ref
		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return (ReferenceTarget*)pblock;}
protected:
		virtual void SetReference(int i, RefTargetHandle rtarg) {pblock=(IParamBlock*)rtarg;}		
public:
		CoreExport RefResult NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);
		
		// Must implement...
		/*! \remarks Returns the validity interval of the gizmo object around
		the specified time.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		Specifies the time at which the gizmo validity is returned.
		\par Default Implementation:
		<b>{return FOREVER;}</b> */
		Interval ObjectValidity(TimeValue t) {return FOREVER;}		
		/*! \remarks Invalidates the user interface for the gizmo so it will
		get redrawn on the next screen update.
		\par Default Implementation:
		<b>{}</b>
		\par Sample Code:
		This is the code from the Sphere GizmoObject implementation of this
		method. Note that it simply calls <b>Invalidate()</b> on the parameter map.
		\code
		void SphereGizmoObject::InvalidateUI() {
			if (pmapParam) pmapParam->Invalidate();
		}
		\endcode  
		*/
		virtual void InvalidateUI() {}
		/*! \remarks Returns the dimension of the parameter whose parameter
		block index is passed.
		\par Parameters:
		<b>int pbIndex</b>\n\n
		Specifies which parameter name to return.
		\par Default Implementation:
		<b>{return defaultDim;}</b>
		\par Sample Code:
		\code	
		ParamDimension *SphereGizmoObject::GetParameterDim(int pbIndex) {
			switch (pbIndex) {
				case PB_GIZMO_RADIUS: return stdWorldDim;
				default: return defaultDim;
			}
		}
		\endcode
		*/
		virtual	ParamDimension *GetParameterDim(int pbIndex) {return defaultDim;}
		/*! \remarks Returns the name of the parameter whose parameter block index is passed.
		\par Parameters:
		<b>int pbIndex</b>\n\n
		Specifies which parameter name to return.
		\par Default Implementation:
		<b>{return MSTR(_M("Parameter"));}</b> */
		virtual MSTR GetParameterName(int pbIndex) {return MSTR(_M("Parameter"));}
		/*! \remarks This method is called to draw the gizmo at the specified
		time into the specified viewport.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to draw the gizmo.\n\n
		<b>GraphicsWindow *gw</b>\n\n
		The GraphicsWindow associated with the viewport in which to draw the gizmo.
		\par Default Implementation:
		<b>{}</b> */
		virtual void DrawGizmo(TimeValue t,GraphicsWindow *gw) {}
		/*! \remarks Returns the wire frame color for the gizmo in the viewports.
		\par Default Implementation:
		<b>{return Point3(1,1,0);}</b> */
		virtual Point3 WireColor() { return GetUIColor(COLOR_ATMOS_APPARATUS); } // mjm - 4.20.99
		/*! \remarks Returns the bounding box for the gizmo, as transformed by
		the matrix passed, at the time passed.
		\par Parameters:
		<b>Matrix3 \&mat</b>\n\n
		The points of the gizmo object should be transformed by this matrix
		before the bounding box is computed from them.\n\n
		<b>TimeValue t</b>\n\n
		The time to compute the bounding box.\n\n
		<b>Box3 \&box</b>\n\n
		The result is stored here.
		\par Default Implementation:
		<b>{}</b>
		\par Sample Code:
		\code
		void SphereGizmoObject::GetBoundBox(Matrix3 &mat, TimeValue t, Box3 &box) {
			float radius;
			int hemi;
			pblock->GetValue(PB_GIZMO_RADIUS,t,radius,FOREVER);
			pblock->GetValue(PB_GIZMO_HEMI,t,hemi,FOREVER);
			BoxLineProc proc(&mat);
			DrawSphere(proc,radius,hemi);
			box += proc.Box();
		}
		\endcode
		*/
		virtual void GetBoundBox(Matrix3 &mat,TimeValue t,Box3 &box) {}
	};
#pragma warning(pop)


