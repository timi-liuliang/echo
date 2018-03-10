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
// FILE:        patchobj.h
// DESCRIPTION: Defines Patch Mesh Object
// AUTHOR:      Tom Hudson
// HISTORY:     created 21 June 1995
//**************************************************************************/

#pragma once
#include "coreexp.h"
#include "maxheap.h"
#include "maxtypes.h"
#include "mouseman.h"
#include "bitarray.h"
#include "maxapi.h"
#include "objmode.h"
#include "object.h"
#include "istdplug.h"
#include "sbmtlapi.h"
#include "patch.h"
// forward declarations
class IObjParam;
class HitRecord;


extern CoreExport Class_ID patchObjectClassID;

extern HINSTANCE hInstance;

// Named selection set list types for PatchObject
#define NS_PO_VERT 0
#define NS_PO_EDGE 1
#define NS_PO_PATCH 2

// set the default pick box size
#define DEF_PICKBOX_SIZE	4

// These are values for selLevel.
#define PO_OBJECT	0
#define PO_VERTEX	1
#define PO_EDGE		2
#define PO_PATCH	3
#define PO_ELEMENT	4
#define PO_HANDLE	5
#define PO_LEVELS	6

#define CID_EP_BIND	CID_USER + 203
#define CID_EP_EXTRUDE	CID_USER + 204
#define CID_EP_BEVEL	CID_USER + 205
#define CID_EP_NORMAL_FLIP	CID_USER + 206
#define CID_CREATE_VERT  CID_USER + 207
#define CID_CREATE_PATCH CID_USER + 208
#define CID_VERT_WELD CID_USER + 209
// CAL-06/02/03: copy/paste tangent. (FID #827)
#define CID_COPY_TANGENT CID_USER + 210
#define CID_PASTE_TANGENT CID_USER + 211

// Flags:
// Disp Result keeps track of "Show End Result" button for this Editable Patch
#define EP_DISP_RESULT 0x0100

// References
#define EP_MASTER_CONTROL_REF 0
#define EP_VERT_BASE_REF 1

class PatchObject;


class EP_BindMouseProc : public MouseCallBack {
	private:
		PatchObject *pobj;
		IObjParam *ip;
		IPoint2 om;
		BitArray knotList;
	
	protected:
		HCURSOR GetTransformCursor();
		BOOL HitAKnot(ViewExp *vpt, IPoint2 *p, int *vert);
		BOOL HitASegment(ViewExp *vpt, IPoint2 *p, int *Seg);

		BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags, int subType );
		BOOL AnyHits( ViewExp *vpt ) { return vpt->NumSubObjHits(); }		

	public:
		EP_BindMouseProc(PatchObject* spl, IObjParam *i) { pobj=spl; ip=i; }
		int proc( 
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
	};



class EP_BindCMode : public CommandMode {
	private:
		ChangeFGObject fgProc;
		EP_BindMouseProc eproc;
		PatchObject* pobj;
//		int type; // See above

	public:
		EP_BindCMode(PatchObject* spl, IObjParam *i) :
			fgProc((ReferenceTarget*)spl), eproc(spl,i) {pobj=spl;}

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_EP_BIND; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &eproc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
//		void SetType(int type) { this->type = type; eproc.SetType(type); }
	};


class EP_ExtrudeMouseProc : public MouseCallBack {
private:
	MoveTransformer moveTrans;
	PatchObject *po;
	Interface *ip;
	IPoint2 om;
	Point3 ndir;
public:
	EP_ExtrudeMouseProc(PatchObject* o, IObjParam *i) : moveTrans(i) {po=o;ip=i;}
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
};


class EP_ExtrudeSelectionProcessor : public GenModSelectionProcessor {
protected:
	HCURSOR GetTransformCursor();
public:
	EP_ExtrudeSelectionProcessor(EP_ExtrudeMouseProc *mc, PatchObject *o, IObjParam *i) 
		: GenModSelectionProcessor(mc,(BaseObject*) o,i) {}
};


class EP_ExtrudeCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	EP_ExtrudeSelectionProcessor mouseProc;
	EP_ExtrudeMouseProc eproc;
	PatchObject* po;

public:
	EP_ExtrudeCMode(PatchObject* o, IObjParam *i) :
		fgProc((ReferenceTarget *)o), mouseProc(&eproc,o,i), eproc(o,i) {po=o;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_EP_EXTRUDE; }
	MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
	ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
	BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void EnterMode();
	void ExitMode();
};

class EP_NormalFlipMouseProc : public MouseCallBack {
private:
	PatchObject *po;
	Interface *ip;
	IPoint2 om;
	Point3 ndir;
public:
	EP_NormalFlipMouseProc(PatchObject* o, IObjParam *i) {po=o;ip=i;}
	BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags, int subType );
	BOOL HitAPatch(ViewExp *vpt, IPoint2 *p, int *pix);
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class EP_NormalFlipCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	EP_NormalFlipMouseProc eproc;
	PatchObject* po;

public:
	EP_NormalFlipCMode(PatchObject* o, IObjParam *i) :
		fgProc((ReferenceTarget *)o), eproc(o,i) {po=o;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_EP_NORMAL_FLIP; }
	MouseCallBack *MouseProc(int *numPoints) { *numPoints=1; return &eproc; }
	ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
	BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void EnterMode();
	void ExitMode();
};


class EP_BevelMouseProc : public MouseCallBack {
private:
	MoveTransformer moveTrans;
	PatchObject *po;
	Interface *ip;
	IPoint2 om;
	
public:
	EP_BevelMouseProc(PatchObject* o, IObjParam *i) : moveTrans(i) {po=o;ip=i;}
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
};


class EP_BevelSelectionProcessor : public GenModSelectionProcessor {
protected:
	HCURSOR GetTransformCursor();
public:
	EP_BevelSelectionProcessor(EP_BevelMouseProc *mc, PatchObject *o, IObjParam *i) 
		: GenModSelectionProcessor(mc,(BaseObject*) o,i) {}
};


class EP_BevelCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	EP_BevelSelectionProcessor mouseProc;
	EP_BevelMouseProc eproc;
	PatchObject* po;

public:
	EP_BevelCMode(PatchObject* o, IObjParam *i) :
		fgProc((ReferenceTarget *)o), mouseProc(&eproc,o,i), eproc(o,i) {po=o;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_EP_BEVEL; }
	MouseCallBack *MouseProc(int *numPoints) { *numPoints=3; return &mouseProc; }
	ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
	BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void EnterMode();
	void ExitMode();
};


class EP_CreateVertMouseProc : public MouseCallBack {
private:		
	PatchObject *po;
	IObjParam *mpIP;		
public:
	EP_CreateVertMouseProc(PatchObject* mod, IObjParam *i) {po=mod;mpIP=i;}
	int proc (HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class EP_CreateVertCMode : public CommandMode {
private:
	ChangeFGObject fgProc;		
	EP_CreateVertMouseProc proc;
	PatchObject *po;

public:
	EP_CreateVertCMode(PatchObject* mod, IObjParam *i) : 
	  fgProc((ReferenceTarget *)mod), proc(mod,i) {po=mod;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_CREATE_VERT; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=1; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
};

class EP_CreatePatchMouseProc : public MouseCallBack {
public:
	PatchObject *po;
	IObjParam *mpIP;		
	int verts[4];
	IPoint2 anchor, lastPoint, startPoint;

	EP_CreatePatchMouseProc(PatchObject* mod, IObjParam *i);
	BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags, int subType );
	BOOL HitAVert(ViewExp *vpt, IPoint2 *p, int& vert);

	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m );
};

class EP_CreatePatchCMode : public CommandMode {
public:
	ChangeFGObject fgProc;
	PatchObject *po;
	EP_CreatePatchMouseProc proc;

	EP_CreatePatchCMode(PatchObject* mod, IObjParam *i) : 
	  fgProc((ReferenceTarget *)mod), proc(mod,i) {po=mod;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_CREATE_PATCH; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=5; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
};


class EP_VertWeldMouseProc : public MouseCallBack {
public:
	PatchObject *po;
	IObjParam *mpIP;		
	int fromVert, toVert;
	IPoint2 anchor, lastPoint;

	EP_VertWeldMouseProc(PatchObject* mod, IObjParam *i){po=mod;mpIP=i;}
	BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags, int subType );
	BOOL HitAVert(ViewExp *vpt, IPoint2 *p, int& vert);

	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m );
};

class EP_VertWeldCMode : public CommandMode {
public:
	ChangeFGObject fgProc;
	PatchObject *po;
	EP_VertWeldMouseProc proc;

	EP_VertWeldCMode(PatchObject* mod, IObjParam *i) : 
	  fgProc((ReferenceTarget *)mod), proc(mod,i) {po=mod;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_VERT_WELD; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=2; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
};


/*-------------------------------------------------------------------*/
// CAL-06/02/03: copy/paste tangent modes. (FID #827)

class EP_CopyTangentMouseProc : public MouseCallBack {
	private:
		PatchObject *po;
		IObjParam *ip;

	protected:
		HCURSOR GetTransformCursor();
		HitRecord* HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags );

	public:
		EP_CopyTangentMouseProc(PatchObject* obj, IObjParam *i) { po=obj; ip=i; }
		int proc(
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
	};

class EP_CopyTangentCMode : public CommandMode {
	private:
		ChangeFGObject fgProc;
		EP_CopyTangentMouseProc eproc;
		PatchObject* po;

	public:
		EP_CopyTangentCMode(PatchObject* obj, IObjParam *i) :
			fgProc((ReferenceTarget*)obj), eproc(obj,i) {po=obj;}

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_COPY_TANGENT; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=1; return &eproc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
	};

class EP_PasteTangentMouseProc : public MouseCallBack {
	private:
		PatchObject *po;
		IObjParam *ip;

	protected:
		HCURSOR GetTransformCursor();
		HitRecord* HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags );

	public:
		EP_PasteTangentMouseProc(PatchObject* obj, IObjParam *i) { po=obj; ip=i; }
		int proc(
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
	};

class EP_PasteTangentCMode : public CommandMode {
	private:
		ChangeFGObject fgProc;
		EP_PasteTangentMouseProc eproc;
		PatchObject* po;

	public:
		EP_PasteTangentCMode(PatchObject* obj, IObjParam *i) :
			fgProc((ReferenceTarget*)obj), eproc(obj,i) {po=obj;}

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_PASTE_TANGENT; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=1; return &eproc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
	};


/*-------------------------------------------------------------------*/

class POPickPatchAttach : 
		public PickModeCallback,
		public PickNodeCallback {
	public:		
		PatchObject *po;
		
		POPickPatchAttach() {po=NULL;}

		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);

		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);

		HCURSOR GetHitCursor(IObjParam *ip);

		BOOL Filter(INode *node);
		
		PickNodeCallback *GetFilter() {return this;}

		BOOL RightClick(IObjParam* ip,ViewExp* vpt)	{ UNUSED_PARAM(ip); UNUSED_PARAM(vpt); return TRUE;}
	};


class SingleRefMakerPatchNode;
class SingleRefMakerPatchMtl;

#pragma warning(push)
#pragma warning(disable:4239)

// The Base Patch class
/*! \sa  Class GeomObject, Class IPatchOps, Class IPatchSelect, Class IPatchSelectData, Class ISubMtlAPI, Class AttachMatDlgUser, Class PatchMesh, Class Patch,  Class Mesh,  Class TessApprox, <a href="ms-its:3dsmaxsdk.chm::/patches_root.html">Working with Patches</a>.\n\n
class PatchObject : public GeomObject, IPatchOps, IPatchSelect,
IPatchSelectData, ISubMtlAPI, AttachMatDlgUser
\par Description:
This class is the base class for the creation of Patch objects. This class
stores an instance of a <b>PatchMesh</b> that holds all the Patches that make
up this patch object. This class also maintains a <b>Mesh</b> cache. All
methods of this class are implemented by the system.
\par Data Members:
<b>PatchMesh patch;</b>\n\n
The patch mesh for this patch object.\n\n
<b>Mesh mesh;</b>\n\n
The Mesh cache.\n\n
<b>BOOL meshValid;</b>\n\n
Indicates if the mesh cache is valid.\n\n
<b>BOOL showMesh;</b>\n\n
Indicates if the mesh is shown in the viewports\n\n
<b>GenericNamedSelSetList vselSet;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
Vertex level named selection sets.\n\n
<b>GenericNamedSelSetList eselSet;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
Edge level named selection sets.\n\n
<b>GenericNamedSelSetList pselSet;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
Patch level named selection sets.\n\n
<b>int patchSelSubType;</b>\n\n
This data member is available in release 4.0 and later only.\n\n
The sub-object selection level, defined by;\n\n
<b>PO_PATCH</b>\n\n
Patch sub-object level. When SetSubobjectLevel(PO_PATCH) is called, both the
PatchMesh selection\n\n
level and patchSelSubType are set to PO_PATCH.\n\n
<b>PO_ELEMENT</b>\n\n
Element sub-object level. When SetSubobjectLevel(PO_ELEMENT) is called, the
PatchMesh selection level is set to PO_PATCH and patchSelSubType is set to
PO_ELEMENT.  */
class PatchObject: public GeomObject, IPatchOps, IPatchSelect, IPatchSelectData, ISubMtlAPI, AttachMatDlgUser {
	friend class PatchObjectRestore;
	friend class POXFormProc;
	friend class POPickPatchAttach;
	public:
		static HWND hSelectPanel, hOpsPanel, hSurfPanel, hSoftSelPanel;
		static BOOL rsSel, rsOps, rsSurf;	// rollup states (FALSE = rolled up)
		static MoveModBoxCMode *moveMode;
		static RotateModBoxCMode *rotMode;
		static UScaleModBoxCMode *uscaleMode;
		static NUScaleModBoxCMode *nuscaleMode;
		static SquashModBoxCMode *squashMode;
		static SelectModBoxCMode *selectMode;
//watje command mode for the bind 		
		static EP_BindCMode *bindMode;
//watje command mode for the extrude 		
		static EP_ExtrudeCMode *extrudeMode;
		static EP_NormalFlipCMode *normalFlipMode;
		static EP_BevelCMode *bevelMode;
		static EP_CreateVertCMode *createVertMode;
		static EP_CreatePatchCMode *createPatchMode;
		static EP_VertWeldCMode *vertWeldMode;
		// CAL-06/02/03: copy/paste tangent. (FID #827)
		static EP_CopyTangentCMode *copyTangentMode;
		static EP_PasteTangentCMode *pasteTangentMode;

		// for the tessellation controls
		static BOOL settingViewportTess;  // are we doing viewport or renderer
		static BOOL settingDisp;  // are we doing viewport or renderer
		static ISpinnerControl *uSpin;
		static ISpinnerControl *vSpin;
		static ISpinnerControl *edgeSpin;
		static ISpinnerControl *angSpin;
		static ISpinnerControl *distSpin;
		static ISpinnerControl *mergeSpin;
		static ISpinnerControl *matSpin;
		static ISpinnerControl *matSpinSel;
		// General rollup controls
		static ISpinnerControl *weldSpin;
		static ISpinnerControl *targetWeldSpin;
		static ISpinnerControl *stepsSpin;
#ifndef NO_OUTPUTRENDERER
		//3-18-99 watje to support render steps
		static ISpinnerControl *stepsRenderSpin;
#endif // NO_OUTPUTRENDERER
// 7/20/00 TH -- Relax controls
		static ISpinnerControl *relaxSpin;
		static ISpinnerControl *relaxIterSpin;

		static POPickPatchAttach pickCB;
		static BOOL patchUIValid;
		static BOOL opsUIValid;
		static int attachMat;
		static BOOL condenseMat;
		// selection box size 
		static int pickBoxSize;
		// selection box size for target weld 
		static int weldBoxSize;

		// Load reference version
		int loadRefVersion;

		Interval geomValid;
		Interval topoValid;
		Interval texmapValid;
		Interval selectValid;
		ChannelMask validBits; // for the remaining constant channels
		void CopyValidity(PatchObject *fromOb, ChannelMask channels);

		//  inherited virtual methods for Reference-management
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message );

		static IObjParam *ip;		
		static PatchObject *editObj;

		PatchMesh patch;
		int patchSelSubType;		// PO_PATCH or PO_ELEMENT

		MasterPointControl	*masterCont;		// Master track controller
		Tab<Control*> vertCont;
		Tab<Control*> vecCont;

		// Remembered info
		PatchMesh *rememberedPatch;	// NULL if using all selected patches
		int rememberedIndex;
		int rememberedData;

		// Editing stuff:
		BOOL doingHandles;

		BOOL showMesh;
		BOOL propagate;

		BOOL inExtrude,inBevel, inPatchCreate;

		DWORD epFlags;

		// CAL-06/02/03: copy/paste tangent info. (FID #827)
		BOOL copyTanLength;
		BOOL tangentCopied;
		Point3 copiedTangent;

		// Named selection sets:
		GenericNamedSelSetList hselSet;  // Handle // CAL-06/10/03: (FID #1914)
		GenericNamedSelSetList vselSet;  // Vertex
		GenericNamedSelSetList eselSet;  // Edge
		GenericNamedSelSetList pselSet;  // Patch

		// additonal references
		SingleRefMakerPatchNode* noderef;                  
		SingleRefMakerPatchMtl* mtlref; 

		/*! \remarks Constructor. */
		CoreExport PatchObject();
		CoreExport PatchObject(PatchObject &from);

		CoreExport void PatchObjectInit();	// Constructor helper

		/*! \remarks Destructor. */
		CoreExport ~PatchObject();

		// display functions from BaseObject
		CoreExport virtual bool RequiresSupportForLegacyDisplayMode() const;
		CoreExport virtual bool UpdateDisplay(
			unsigned long renderItemCategories, 
			const MaxSDK::Graphics::MaterialRequiredStreams& materialRequiredStreams, 
			TimeValue t);

		CoreExport PatchObject &operator=(PatchObject &from);

		// Flag methods.
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method sets or clears the status of the Show End Result flag.
		\par Parameters:
		<b>DWORD fl</b>\n\n
		The flag you wish to set or clear. Currently the only flag defined is
		the Show End Result flag <b>EP_DISP_RESULT</b><b>.</b>\n\n
		<b>BOOL val</b>\n\n
		Specifies if the given flag should be set or cleared. */
		void SetFlag (DWORD fl, BOOL val=TRUE) { if (val) epFlags |= fl; else epFlags &= ~fl; }
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method clears the status of the Show End Result flag.
		\par Parameters:
		<b>DWORD fl</b>\n\n
		The flag you wish to set or clear. Currently the only flag defined is
		the Show End Result flag <b>EP_DISP_RESULT</b><b>.</b> */
		void ClearFlag (DWORD fl) { epFlags &= (~fl); }
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This methods allows you to obtain the status of the Show End Result
		flag.
		\par Parameters:
		<b>DWORD fl</b>\n\n
		The flag you wish to set or clear. Currently the only flag defined is
		the Show End Result flag <b>EP_DISP_RESULT</b><b>.</b> */
		bool GetFlag (DWORD fl) { return (epFlags&fl) ? TRUE : FALSE; }

		//  inherited virtual methods:

		// From BaseObject
		CoreExport int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
		CoreExport int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
		CoreExport int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		CoreExport void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
		CoreExport CreateMouseCallBack* GetCreateMouseCallBack();
		CoreExport RefTargetHandle Clone(RemapDir& remap);
		
		// Gizmo versions:
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method is used to display the gizmo version of the patch mesh.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to display the object.\n\n
		<b>INode* inode</b>\n\n
		The node to display.\n\n
		<b>ViewExp* vpt</b>\n\n
		An interface pointer that may be used to call methods associated with
		the viewports.\n\n
		<b>int flags</b>\n\n
		The display flags. See the
		<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_display_flags.html">List of Display Flags</a>
		for more information.\n\n
		<b>ModContext* mc</b>\n\n
		A pointer to the modifiers ModContext. */
		CoreExport int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc);
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method returns the world space bounding box for the gizmo version
		of the patch mesh.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to compute the bounding box.\n\n
		<b>INode* inode</b>\n\n
		The node to calculate the bounding box for.\n\n
		<b>ViewExp* vpt</b>\n\n
		An interface pointer that may be used to call methods associated with
		the viewports.\n\n
		<b>Box3\& box</b>\n\n
		The bounding box which was computed.\n\n
		<b>ModContext* mc</b>\n\n
		A pointer to the modifiers ModContext. */
		CoreExport void GetWorldBoundBox (TimeValue t, INode * inode, ViewExp* vp, Box3& box, ModContext *mc);

		// NS: New SubObjType API
		CoreExport int NumSubObjTypes();
		CoreExport ISubObjType *GetSubObjType(int i);

		// From Object			 
		CoreExport Interval ObjectValidity(TimeValue t);
		CoreExport BOOL GetExtendedProperties(TimeValue t, MSTR &prop1Label, MSTR &prop1Data, MSTR &prop2Label, MSTR &prop2Data);
        CoreExport BOOL PolygonCount(TimeValue t, int& numFaces, int& numVerts);

		// get and set the validity interval for the nth channel
	   	CoreExport Interval ChannelValidity(TimeValue t, int nchan);
		CoreExport void SetChannelValidity(int i, Interval v);
		CoreExport void InvalidateChannels(ChannelMask channels);
		CoreExport void TopologyChanged(); // mjm - 5.6.99

		// Convert-to-type validity
		CoreExport Interval ConvertValidity(TimeValue t);

		// Deformable object procs	
		virtual int IsDeformable() { return 1; }  
		CoreExport int NumPoints();
		CoreExport Point3 GetPoint(int i);
		CoreExport void SetPoint(int i, const Point3& p);
		CoreExport BOOL IsPointSelected (int i);
		
		CoreExport void PointsWereChanged();
		CoreExport void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm=NULL,BOOL useSel=FALSE );
		CoreExport void Deform(Deformer *defProc, int useSel);

		virtual BOOL IsParamSurface() {return TRUE;}
		CoreExport  Point3 GetSurfacePoint(TimeValue t, float u, float v,Interval &iv);

		// Mappable object procs
		virtual int IsMappable() { return 1; }
		virtual int NumMapChannels () { return patch.NumMapChannels (); }
		virtual int NumMapsUsed () { return patch.getNumMaps(); }
		virtual void ApplyUVWMap(int type, float utile, float vtile, float wtile,
			int uflip, int vflip, int wflip, int cap,const Matrix3 &tm,int channel=1) {
				patch.ApplyUVWMap(type,utile,vtile,wtile,uflip,vflip,wflip,cap,tm,channel); }

		CoreExport int CanConvertToType(Class_ID obtype);
		CoreExport Object* ConvertToType(TimeValue t, Class_ID obtype);
		CoreExport void GetCollapseTypes(Tab<Class_ID> &clist,Tab<MSTR*> &nlist);
		CoreExport void FreeChannels(ChannelMask chan);
		CoreExport Object *MakeShallowCopy(ChannelMask channels);
		CoreExport void ShallowCopy(Object* fromOb, ChannelMask channels);
		CoreExport void NewAndCopyChannels(ChannelMask channels);

		CoreExport DWORD GetSubselState();

		// From GeomObject
		CoreExport int IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm);
		CoreExport void GetWorldBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& box );
		CoreExport void GetLocalBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& box );
		CoreExport int IsInstanceDependent();	// Not view-dependent (yet)
		CoreExport Mesh* GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete);

		CoreExport PatchMesh& GetPatchMesh(TimeValue t);
		CoreExport Mesh& GetMesh(TimeValue t);

		// Animatable methods

		CoreExport virtual void DeleteThis();
		virtual void FreeCaches() {patch.InvalidateGeomCache(); }
		virtual Class_ID ClassID() { return Class_ID(PATCHOBJ_CLASS_ID,0); }
		CoreExport BOOL IsSubClassOf(Class_ID classID);
		CoreExport virtual void GetClassName(MSTR& s);
        
        using GeomObject::GetInterface;
		CoreExport void* GetInterface(ULONG id);
		
		// This is the name that will appear in the history browser.
		CoreExport virtual MCHAR *GetObjectName();

		// Controller stuff for animatable points
		int NumSubs()  { return 1; }	// Just tell it about the master point controller
		CoreExport Animatable* SubAnim(int i);
		CoreExport MSTR SubAnimName(int i);
		CoreExport BOOL AssignController(Animatable *control,int subAnim);
		int SubNumToRefNum(int subNum) {return subNum;}
		CoreExport BOOL SelectSubAnim(int subNum);
		CoreExport BOOL HasControllers(BOOL assertCheck=TRUE);

		// Reference methods
		CoreExport void RescaleWorldUnits(float f);
		int NumRefs() {return vertCont.Count() + vecCont.Count() + 1;}	// vert conts + vec conts + master
		CoreExport RefTargetHandle GetReference(int i);
protected:
		CoreExport virtual void SetReference(int i, RefTargetHandle rtarg);
public:
		CoreExport int RemapRefOnLoad(int iref);
		CoreExport void PlugControllersSel(TimeValue t);
		CoreExport void AllocVertContArray(int count);
		CoreExport void AllocVecContArray(int count);
		CoreExport void AllocContArrays(int vertCount, int vecCount);
		CoreExport void ReplaceVertContArray(Tab<Control *> &nc);
		CoreExport void ReplaceVecContArray(Tab<Control *> &nc);
		CoreExport void ReplaceContArrays(Tab<Control *> &vertnc, Tab<Control *> &vecnc);
		CoreExport BOOL PlugVertControl(TimeValue t,int i);
		CoreExport BOOL PlugVecControl(TimeValue t,int i);
		CoreExport void SetVertAnim(TimeValue t, int point, Point3 pt);
		CoreExport void SetVecAnim(TimeValue t, int point, Point3 pt);
		CoreExport void SetVertCont(int i, Control *c);
		CoreExport void SetVecCont(int i, Control *c);

		// IO
		CoreExport IOResult Save(ISave *isave);
		CoreExport IOResult Load(ILoad *iload);

		// PatchObject-specific methods
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
		This should be implemented by classes derived from PatchObject whose
		patches change over time.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to update the patch mesh.
		\par Default Implementation:
		<b>{}</b> */
		CoreExport virtual void UpdatePatchMesh(TimeValue t);
		/*! \remarks This method checks to see if the mesh cache is up to
		date, and if not, it generates it.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		This parameter is available in release 2.0 and later only.\n\n
		The mesh cache should be generated to reflect this time. */
		CoreExport void PrepareMesh(TimeValue t);
		CoreExport Mesh& GetUnrelaxedMesh(TimeValue t);	// Ignore relax!
		/*! \remarks Returns TRUE if the patch lattice is displayed; otherwise
		FALSE. */
		CoreExport BOOL ShowLattice() { return patch.GetDispFlag(DISP_LATTICE) ? TRUE : FALSE; }
		/*! \remarks Returns TRUE if the patch vertices are shown; otherwise
		FALSE. */
		CoreExport BOOL ShowVerts() { return patch.GetDispFlag(DISP_VERTS) ? TRUE : FALSE; }
		/*! \remarks Sets the state of the lattice display switch.
		\par Parameters:
		<b>BOOL sw</b>\n\n
		TRUE to turn on the lattice display; FALSE to turn it off. */
		CoreExport void SetShowLattice(BOOL sw) { if(sw) patch.SetDispFlag(DISP_LATTICE); else patch.ClearDispFlag(DISP_LATTICE); }
		/*! \remarks Sets the state of the vertex display switch
		\par Parameters:
		<b>BOOL sw</b>\n\n
		TRUE to turn on the vertex display; FALSE to turn it off. */
		CoreExport void SetShowVerts(BOOL sw) { if(sw) patch.SetDispFlag(DISP_VERTS); else patch.ClearDispFlag(DISP_VERTS); }
		/*! \remarks Sets the number of mesh steps (viewport).
		\par Parameters:
		<b>int steps</b>\n\n
		The number of steps to set. */
		CoreExport void SetMeshSteps(int steps);
		/*! \remarks Returns the number of mesh steps (viewport). */
		CoreExport int GetMeshSteps();
#ifndef NO_OUTPUTRENDERER
		//3-18-99 watje to support render steps
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets the Surface Render Steps setting.
		\par Parameters:
		<b>int steps</b>\n\n
		The value to set. */
		CoreExport void SetMeshStepsRender(int steps);
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns the Surface Render Steps setting. */
		CoreExport int GetMeshStepsRender();
#endif // NO_OUTPUTRENDERER
		CoreExport BOOL Relaxing();		// returns TRUE if Relax && RelaxValue != 0 && RelaxIter != 0
		CoreExport void SetRelax(BOOL v);
		CoreExport void SetRelaxViewports(BOOL v);
		CoreExport void SetRelaxValue(float v);
		CoreExport void SetRelaxIter(int v);
		CoreExport void SetRelaxBoundary(BOOL v);
		CoreExport void SetRelaxSaddle(BOOL v);
		BOOL GetRelax() { return patch.GetRelax(); }
		BOOL GetRelaxViewports() { return patch.GetRelaxViewports(); }
		float GetRelaxValue() { return patch.GetRelaxValue(); }
		int GetRelaxIter() { return patch.GetRelaxIter(); }
		BOOL GetRelaxBoundary() { return patch.GetRelaxBoundary(); }
		BOOL GetRelaxSaddle() { return patch.GetRelaxSaddle(); }

		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets the 'Show Interior Edges' value.
		\par Parameters:
		<b>BOOL si</b>\n\n
		TRUE for on; FALSE for off. */
		CoreExport void SetShowInterior(BOOL si);
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns the 'Show Interior Edge' setting; TRUE if on; FALSE if off. */
		CoreExport BOOL GetShowInterior();

		CoreExport void SetUsePatchNormals(BOOL usePatchNorm);
		CoreExport BOOL GetUsePatchNormals();

		/*! \remarks Sets the state of the adaptive switch.
		\par Parameters:
		<b>BOOL sw</b>\n\n
		TRUE to turn on; FALSE to turn off. */
		CoreExport void SetAdaptive(BOOL sw);
		/*! \remarks Returns the state of the adaptive switch. TRUE is on;
		FALSE is off. */
		CoreExport BOOL GetAdaptive();
		/*! \remarks Sets the tesselation approximation object used for
		viewport rendering.
		\par Parameters:
		<b>TessApprox tess</b>\n\n
		The tesselation approximation object to be used for viewport rendering.
		*/
		CoreExport void SetViewTess(TessApprox tess);
		/*! \remarks Returns the tesselation approximation object used for
		rendering in the viewports. */
		CoreExport TessApprox GetViewTess();
		/*! \remarks Sets the tesselation approximation object used for
		production rendering.
		\par Parameters:
		<b>TessApprox tess</b>\n\n
		The tesselation approximation object to be used for production
		rendering. */
		CoreExport void SetProdTess(TessApprox tess);
		/*! \remarks Returns the tesselation approximation object used for
		production rendering. */
		CoreExport TessApprox GetProdTess();
		/*! \remarks Sets the tesselation approximation object used for
		display in the viewports.
		\par Parameters:
		<b>TessApprox tess</b>\n\n
		The tesselation approximation object to be used for the viewports. */
		CoreExport void SetDispTess(TessApprox tess);
		/*! \remarks Returns the tesselation approximation object used for
		display in the viewports. */
		CoreExport TessApprox GetDispTess();
		/*! \remarks Returns TRUE if normals are used from the viewport
		tesselator; otherwise FALSE. */
		CoreExport BOOL GetViewTessNormals();
		/*! \remarks Sets if normals are used from the viewport tesselator.
		\par Parameters:
		<b>BOOL use</b>\n\n
		TRUE to use normals; FALSE to not use them. */
		CoreExport void SetViewTessNormals(BOOL use);
		/*! \remarks Returns TRUE if normals are used from the production
		renderer tesselator; otherwise FALSE. */
		CoreExport BOOL GetProdTessNormals();
		/*! \remarks Sets if normals are used from the production renderer
		tesselator.
		\par Parameters:
		<b>BOOL use</b>\n\n
		TRUE to use normals; FALSE to not use them. */
		CoreExport void SetProdTessNormals(BOOL use);
		/*! \remarks Returns TRUE if the viewport mesh is welded after
		tesselation; otherwise FALSE. */
		CoreExport BOOL GetViewTessWeld();
		/*! \remarks Sets if the viewport mesh is welded after tesselation;
		otherwise FALSE.
		\par Parameters:
		<b>BOOL weld</b>\n\n
		TRUE to weld; FALSE to not weld. */
		CoreExport void SetViewTessWeld(BOOL weld);
		/*! \remarks Returns TRUE if the production renderer mesh is welded
		after tesselation; otherwise FALSE. */
		CoreExport BOOL GetProdTessWeld();
		/*! \remarks Sets if the production renderer mesh is welded after
		tesselation; otherwise FALSE.
		\par Parameters:
		<b>BOOL weld</b>\n\n
		TRUE to weld; FALSE to not weld. */
		CoreExport void SetProdTessWeld(BOOL weld);
		/*! \remarks Invalidates the mesh cache. */
		CoreExport void InvalidateMesh();
		CoreExport void InvalidateRelaxedMesh();

		// Editable patch stuff follows...
		CoreExport virtual void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		CoreExport virtual void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		CoreExport int GetSubobjectLevel();
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method goes hand-in-hand with <b>GetSubobjectLevel()</b>, except
		that this method returns the type of geometry that is actually being
		acted upon.
		\return  The sub-object type, either <b>PO_PATCH</b> or
		<b>PO_ELEMENT</b>. */
		CoreExport int GetSubobjectType();	// returns PO_PATCH for PO_ELEMENT level
		CoreExport void SetSubobjectLevel(int level);
		CoreExport void ActivateSubobjSel(int level, XFormModes& modes );
		CoreExport int SubObjectIndex(HitRecord *hitRec);
		CoreExport void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method is called by the system then the status of the Show End
		Result function changes (ie. the Show End Results button has been
		toggled on or off). Note that setting the state of the Show End Result
		is done through the <b>Interface::SetShowEndResult()</b> method.
		\par Parameters:
		<b>BOOL showEndResult</b>\n\n
		This flag specifies the Show End Result status, which is TRUE if on;
		FALSE if off. */
		CoreExport void ShowEndResultChanged (BOOL showEndResult);
		CoreExport void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		int NeedUseSubselButton() { return 0; }
		CoreExport void SelectSubComponent( HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert );
		CoreExport void ClearSelection(int selLevel);
		CoreExport void SelectAll(int selLevel);
		CoreExport void InvertSelection(int selLevel);
		
		CoreExport void PatchSelChanged();
		CoreExport void InvalidateSurfaceUI();
		CoreExport void InvalidateOpsUI();

		CoreExport void ChangeRememberedPatch(int type);
		CoreExport void ChangeSelPatches(int type);
		CoreExport int RememberPatchThere(HWND hWnd, IPoint2 m);
		CoreExport void SetRememberedPatchType(int type);
		CoreExport void ChangeRememberedVert(int type);
		CoreExport void ChangeSelVerts(int type);
		CoreExport int RememberVertThere(HWND hWnd, IPoint2 m);
		CoreExport void SetRememberedVertType(int type);

		// Generic xform procedure.
		CoreExport void XFormVerts( POXFormProc *xproc, TimeValue t, Matrix3& partm, Matrix3& tmAxis );

		// Specialized xform for bezier handles
		CoreExport void XFormHandles( POXFormProc *xproc, TimeValue t, Matrix3& partm, Matrix3& tmAxis, int handleIndex );

		// Affine transform methods		
		CoreExport void Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE );
		CoreExport void Rotate( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin=FALSE );
		CoreExport void Scale( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE );

		// The following is called before the first Move(), Rotate() or Scale() call
		CoreExport void TransformStart(TimeValue t);

		// The following is called after the user has completed the Move, Rotate or Scale operation and
		// the undo object has been accepted.
		CoreExport void TransformFinish(TimeValue t);		

		// The following is called when the transform operation is cancelled by a right-click and the undo
		// has been cancelled.
		CoreExport void TransformCancel(TimeValue t);		

		CoreExport ObjectState Eval(TimeValue t);

		BOOL SupportsNamedSubSels() {return TRUE;}
		CoreExport void ActivateSubSelSet(MSTR &setName);
		CoreExport void NewSetFromCurSel(MSTR &setName);
		CoreExport void RemoveSubSelSet(MSTR &setName);
		CoreExport void SetupNamedSelDropDown();
		CoreExport int NumNamedSelSets();
		CoreExport MSTR GetNamedSelSetName(int i);
		CoreExport void SetNamedSelSetName(int i,MSTR &newName);
		CoreExport void NewSetByOperator(MSTR &newName,Tab<int> &sets,int op);
		CoreExport BOOL GetUniqueSetName(MSTR &name);
		CoreExport int SelectNamedSet();
		CoreExport void NSCopy();
		CoreExport void NSPaste();
		CoreExport GenericNamedSelSetList &GetSelSet();

		CoreExport void RefreshSelType();
		CoreExport void SetNumSelLabel();
		CoreExport void SetSelDlgEnables();
		CoreExport void SetOpsDlgEnables();
		CoreExport void SetSurfDlgEnables();
		CoreExport void SetSoftSelDlgEnables( HWND hSoftSel = NULL );

		// from AttachMatDlgUser
		CoreExport int GetAttachMat();
		CoreExport void SetAttachMat(int value);
		CoreExport BOOL GetCondenseMat();
		CoreExport void SetCondenseMat(BOOL sw);

		CoreExport void DoAttach(INode *node, PatchMesh *attPatch, bool & canUndo);
		CoreExport void SetTessUI(HWND hDlg, TessApprox *tess);
		// 'interactive' in the following records Undo information, displays prompts, notifies
		// dependents and redraws when TRUE
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		This method will delete the selected patches, exactly like the
		DeleteMesh modifier does.
		\par Parameters:
		<b>BOOL interactive</b>\n\n
		This parameter is available in release 4.0 and later only.\n\n
		If this parameter is set to TRUE it will cause the method to display
		the appropriate prompts, create an undo object, and notifies the
		dependents. */
		CoreExport void DoDeleteSelected(BOOL interactive=TRUE);
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		In vertex mode, this method examines selected vertices, and if any of
		the vertices that are part of the selection set are used by more than
		one patch, those vertices (and any attached vectors) are duplicated
		into separate geometry for each patch using it.\n\n
		In edge mode, this method examines the selected edges and any vectors
		on the selected edges that are used by more than one patch are
		duplicated into separate geometry for each patch using them. Any
		vertices used by more than one selected edge are duplicated as well for
		patches on opposite sides of the edge.\n\n
		Note: If the vertices and vectors involved have controllers attached,
		they are removed by this operation.
		\par Parameters:
		<b>BOOL interactive</b>\n\n
		If this parameter is set to TRUE it will cause the method to display
		the appropriate prompts, create an undo object, and notifies the
		dependents. */
		CoreExport void DoBreak(BOOL interactive=TRUE);
		CoreExport void ResolveTopoChanges();
		CoreExport void DeletePatchParts(BitArray &delVerts, BitArray &delPatches);
		// Animated point stuff:
		CoreExport void CreateContArrays();
		CoreExport void SynchContArrays();
		// Materials
		CoreExport int GetSelMatIndex();
		CoreExport void SetSelMatIndex(int index);
		CoreExport void SelectByMat(int index,BOOL clear);
		// Smoothing
		CoreExport DWORD GetSelSmoothBits(DWORD &invalid);
		CoreExport DWORD GetUsedSmoothBits();
		CoreExport void SelectBySmoothGroup(DWORD bits,BOOL clear);
		CoreExport void SetSelSmoothBits(DWORD bits,DWORD which);
		// Subdivision / addition
		CoreExport void SetPropagate(BOOL sw);
		CoreExport BOOL GetPropagate();
		CoreExport void DoPatchAdd(int type);
		CoreExport void DoSubdivide(int level);
		CoreExport void DoPatchDetach(int copy, int reorient);
		// Welding
		CoreExport void DoVertWeld();
		CoreExport void DoVertWeld(int fromVert, int toVert);
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method will perform the edge weld function on the patch object.
		Note that this does not take into account any threshold but welds edges
		only if they use the same two vertices as endpoints. When two or more
		edges are welded, the locations of the edge vectors are averaged to
		create the new edge. */
		CoreExport void DoEdgeWeld();
		// CAL-06/02/03: copy/paste tangent. (FID #827)
		CoreExport void StartCopyTangentMode();
		CoreExport void StartPasteTangentMode();
		CoreExport void StartPasteTangent();
		CoreExport void EndPasteTangent();

		CoreExport BOOL CopyTangent(int vec);
		CoreExport BOOL PasteTangent(int vec);
//watje
		//hide and unhide stuff
		CoreExport void DoHide(int type) ;
		CoreExport void DoUnHide();
		CoreExport void DoPatchHide();
		CoreExport void DoVertHide();
		CoreExport void DoEdgeHide();
//watje hook stuff
		CoreExport void DoAddHook(int vert1, int seg1) ;
		CoreExport void DoRemoveHook() ;
//watje bevel and extrusion stuff
		/*! \remarks When called with the Animate state active and on a
		nonzero TimeValue, this method will prepare the controllers for the
		geometry that is being created. The program can then fill in the
		animated vertex values later.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		This parameter is available in release 4.0 and later only.\n\n
		The time at which to prepare and execute the extrude operation. */
		CoreExport void DoExtrude(TimeValue t) ;
		CoreExport void BeginExtrude(TimeValue t); 	
		CoreExport void EndExtrude (TimeValue t, BOOL accept=TRUE);		
		CoreExport void Extrude( TimeValue t, float amount, BOOL useLocalNorm );

		/*! \remarks		This method flips the normals of a specified patch or all selected patches.
		This method will save undo information and displays a prompt if <b>patchIndex
		\< 0</b> while there are no patches selected.
		\par Parameters:
		<b>int patchIndex</b>\n\n
		The index of the patch for which to flip the normal. If this parameter is \< 0,
		the normals of all selected patches will be flipped (if there are any
		selected).\n\n
		Any vertices set to <b>PVERT_COPLANAR</b> that lie on the boundary between
		flipped and unflipped patches will have their type set to <b>PVERT_CORNER</b>.
		This is because attempting to compute normals of neighboring patches with
		opposite normals in order to get a proper plane often results in invalid
		normals being generated. Making the vertex a corner type prevents the problem.
		Any vertices not on the boundary between flipped and unflipped patches are left
		as is. */
		CoreExport void DoFlipNormals(int patchIndex = -1);
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method examines the selected patch set and attempts to make them
		all face the same direction. Preferred direction is arbitrary; the
		first selected patch encountered in each contiguous group determines
		the direction all patches in that group will attain.\n\n
		Any vertices set to <b>PVERT_COPLANAR</b> that lie on the boundary
		between flipped and unflipped patches will have their type set to
		<b>PVERT_CORNER</b>. This is because attempting to compute normals of
		neighboring patches with opposite normals in order to get a proper
		plane often results in invalid normals being generated. Making the
		vertex a corner type prevents the problem. Any vertices not on the
		boundary between flipped and unflipped patches are left as is. */
		CoreExport void DoUnifyNormals();
		
		/*! \remarks When called with the Animate state active and on a
		nonzero TimeValue, this method will prepare the controllers for the
		geometry that is being created. The program can then fill in the
		animated vertex values later.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		This parameter is available in release 4.0 and later only.\n\n
		The time at which to prepare and execute the bevel operation. */
		CoreExport void DoBevel(TimeValue t) ;
		CoreExport void BeginBevel(TimeValue t); 	
		CoreExport void EndBevel (TimeValue t, BOOL accept=TRUE);		
		CoreExport void Bevel( TimeValue t, float amount, BOOL smoothStart, BOOL smoothEnd );

		// CAL-04/23/03: patch smooth (FID #1914)
		CoreExport void DoPatchSmooth(int type);

		// methods for creating new vertices and patches
		CoreExport void CreateVertex(Point3 pt, int& newIndex);
		CoreExport void CreatePatch(int vertIndx1,int vertIndx2,int vertIndx3);
		CoreExport void CreatePatch(int vertIndx1,int vertIndx2,int vertIndx3,int vertIndx4);

		// patch select and operations interfaces, JBW 2/2/99
		CoreExport void StartCommandMode(patchCommandMode mode);
		CoreExport void ButtonOp(patchButtonOp opcode);

// LAM: added 9/3/00
	// UI controls access
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method allows you to get the edit patch parameters from the
		command panel. Currently not in use.
		\par Parameters:
		<b>patchUIParam uiCode</b>\n\n
		This enum is currently empty.\n\n
		<b>int \&ret</b>\n\n
		The returned value.
		\par Default Implementation:
		<b>{ }</b> */
		CoreExport void GetUIParam (patchUIParam uiCode, int & ret);
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method allows you to set the edit patch parameters from the
		command panel. Currently not in use.
		\par Parameters:
		<b>patchUIParam uiCode</b>\n\n
		This enum is currently empty.\n\n
		<b>int val</b>\n\n
		The value to set.
		\par Default Implementation:
		<b>{ }</b> */
		CoreExport void SetUIParam (patchUIParam uiCode, int val);
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method allows you to get the edit patch parameters from the
		command panel. Currently not in use.
		\par Parameters:
		<b>patchUIParam uiCode</b>\n\n
		This enum is currently empty.\n\n
		<b>float \&ret</b>\n\n
		The returned value.
		\par Default Implementation:
		<b>{ }</b> */
		CoreExport void GetUIParam (patchUIParam uiCode, float & ret);
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method allows you to set the edit patch parameters from the
		command panel. Currently not in use.
		\par Parameters:
		<b>patchUIParam uiCode</b>\n\n
		This enum is currently empty.\n\n
		<b>float val</b>\n\n
		The value to set.
		\par Default Implementation:
		<b>{ }</b> */
		CoreExport void SetUIParam (patchUIParam uiCode, float val);
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method will return TRUE if the SplineShape object or Edit Spline
		modifier is active in the command panel.
		\par Default Implementation:
		<b>{ return (ip \&\& (editObj==this)) ? TRUE : FALSE; }</b> */
		bool Editing () { return (ip && (editObj==this)) ? TRUE : FALSE; }

		CoreExport DWORD GetSelLevel();
		CoreExport void SetSelLevel(DWORD level);
		CoreExport void LocalDataChanged();

		CoreExport BitArray GetVecSel();
		CoreExport BitArray GetVertSel();
		CoreExport BitArray GetEdgeSel();
		CoreExport BitArray GetPatchSel();
		
		CoreExport void SetVecSel(BitArray &set, IPatchSelect *imod, TimeValue t);
		CoreExport void SetVertSel(BitArray &set, IPatchSelect *imod, TimeValue t);
		CoreExport void SetEdgeSel(BitArray &set, IPatchSelect *imod, TimeValue t);
		CoreExport void SetPatchSel(BitArray &set, IPatchSelect *imod, TimeValue t);

		CoreExport GenericNamedSelSetList& GetNamedVecSelList();
		CoreExport GenericNamedSelSetList& GetNamedVertSelList();
		CoreExport GenericNamedSelSetList& GetNamedEdgeSelList();
		CoreExport GenericNamedSelSetList& GetNamedPatchSelList();

		// Vertex Color Support...
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method returns the common color for all selected vertices. If no
		vertices are selected then white (1,1,1) will be returned, however, if
		multiple vertices with different colors are selected, then black
		(0,0,0) will be returned.
		\par Parameters:
		<b>int mp=0</b>\n\n
		The map channel.\n\n
		<b>bool *differs=NULL</b>\n\n
		This parameter is returned to indicate if there were any
		differences.\n\n
		  */
		CoreExport Color GetVertColor(int mp=0, bool *differs=NULL);
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method will set all selected vertices to the specified color.
		\par Parameters:
		<b>Color clr</b>\n\n
		The color you wish to apply to all the selected vertices.\n\n
		<b>int mp=0</b>\n\n
		The map channel. */
		CoreExport void SetVertColor(Color clr, int mp=0);
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method will select all vertices which fall into a specified color
		range.
		\par Parameters:
		<b>VertColor clr</b>\n\n
		The starting color of the vertices you wish to select by color.\n\n
		<b>int deltaR</b>\n\n
		The difference range for the red color component.\n\n
		<b>int deltaG</b>\n\n
		The difference range for the green color component.\n\n
		<b>int deltaB</b>\n\n
		The difference range for the blue color component.\n\n
		<b>BOOL add</b>\n\n
		This flag adds vertices to the selection that fall into the color
		range.\n\n
		<b>BOOL sub</b>\n\n
		This flag subtracts vertices from the selection that fall into the
		color range.\n\n
		<b>int mp=0</b>\n\n
		The map channel. */
		CoreExport void SelectVertByColor(VertColor clr, int deltaR, int deltaG, int deltaB, BOOL add, BOOL sub, int mp=0);
		// and patch-level vertex color support
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method returns the common color for all selected patches. If no
		patches are selected then white (1,1,1) will be returned, however, if
		different vertex colors are present in the selected patches, then black
		(0,0,0) will be returned.
		\par Parameters:
		<b>int mp=0</b>\n\n
		The map channel.\n\n
		<b>bool *differs=NULL</b>\n\n
		This parameter is returned to indicate if there were any
		differences.\n\n
		  */
		CoreExport Color GetPatchColor(int mp=0, bool *differs=NULL);
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method will set all selected patches to the specified color.
		\par Parameters:
		<b>Color clr</b>\n\n
		The color you wish to apply to all the selected patches.\n\n
		<b>int mp=0</b>\n\n
		The map channel. */
		CoreExport void SetPatchColor(Color clr, int mp=0);

		// ISubMtlAPI methods:
		CoreExport MtlID GetNextAvailMtlID(ModContext* mc);
		CoreExport BOOL HasFaceSelection(ModContext* mc);
		CoreExport void SetSelFaceMtlID(ModContext* mc, MtlID id, BOOL bResetUnsel = FALSE);
		CoreExport int GetSelFaceUniqueMtlID(ModContext* mc);
		CoreExport int GetSelFaceAnyMtlID(ModContext* mc);
		CoreExport int GetMaxMtlID(ModContext* mc);
//watje new patch mapping
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method will change the mapping type of the selected patches to
		linear or curved.
		\par Parameters:
		<b>BOOL linear</b>\n\n
		If TRUE the mapping type will be changed to linear. FALSE will change
		the mapping type to curved. */
		CoreExport void ChangeMappingTypeLinear(BOOL linear);

		// CAL-04/23/03: Shrink/Grow, Edge Ring/Loop selection. (FID #1914)
		CoreExport void ShrinkSelection(int type);
		CoreExport void GrowSelection(int type);
		CoreExport void SelectEdgeRing();
		CoreExport void SelectEdgeLoop();

		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method examines the patch mesh and selects any edges used by only
		one single patch. */
		CoreExport void SelectOpenEdges();
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method will create a bezier spline shape from the selected edges
		of the patch mesh. Each edge will become a separate spline in the
		output shape. The user will be prompted to enter a name for the new
		editable spline object that will be created. */
		CoreExport void DoCreateShape();

// soft selection support
//
		CoreExport int  UseEdgeDists();
		CoreExport void SetUseEdgeDists( int useSoftSelections );
		CoreExport int  UseSoftSelections();
		CoreExport void SetUseSoftSelections( int useSoftSelections );
		CoreExport void InvalidateVertexWeights();
		// CAL-05/06/03: toggle shaded faces display for soft selection. (FID #1914)
		CoreExport void ToggleShadedFaces();

		CoreExport void UpdateVertexDists();
		CoreExport void UpdateEdgeDists( );
		CoreExport void UpdateVertexWeights();

		CoreExport Point3 VertexNormal( int vIndex ); 

};

#pragma warning(pop)

CoreExport ClassDesc* GetPatchObjDescriptor();

// Helper classes used internally for undo operations

class POModRecord: public MaxHeapOperators {
	public:
		virtual ~POModRecord(){}
		virtual BOOL Restore(PatchObject *po, BOOL isUndo)=0;
		virtual BOOL Redo(PatchObject *po)=0;
		virtual DWORD Parts()=0;				// Flags of parts modified by this
	};

class PatchObjectRestore : public RestoreObj {
	public:
		POModRecord *rec;		// Modification record
		PatchObject	 *po;

		PatchObjectRestore(PatchObject *po, POModRecord *rec);
		virtual ~PatchObjectRestore();

		void Restore(int isUndo);
		void Redo();
		int Size() { return 1; }
		void EndHold() {po->ClearAFlag(A_HELD);}
		MSTR Description() { return MSTR(_M("PatchObject restore")); }
	};

class SingleRefMakerPatchNode : public SingleRefMaker{
	public:
		HWND hwnd;
		PatchObject *po;
		SingleRefMakerPatchNode() {hwnd = NULL; po = NULL;}
		~SingleRefMakerPatchNode() { }
		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message );
	};

class SingleRefMakerPatchMtl : public SingleRefMaker{
	public:	
		HWND hwnd;
		PatchObject *po;
		SingleRefMakerPatchMtl() {hwnd = NULL; po = NULL;}
		~SingleRefMakerPatchMtl() { }
		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message );
};

// Command ID for the dynamic spline quad menu entry
#define ID_PATCH_MENU 1333

const DWORD kPatchActions = 0x34fe4c77;
const DWORD kPatchActionsContext = 0x34fe4c77;

