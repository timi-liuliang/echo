/**********************************************************************
 *<
	FILE: objmode.h

	DESCRIPTION: Provides some standard modes for subobject manipulation

	CREATED BY: Rolf Berteig

	HISTORY: Created 3-14-95

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "maxheap.h"
#include "matrix3.h"
#include "ipoint2.h"
#include "quat.h"
#include "sceneapi.h"
#include "ref.h"
#include "mouseman.h"
#include "maxapi.h"
#pragma warning(push)
#pragma warning(disable:4100)
class Transformer: public MaxHeapOperators
{
	protected:
		Matrix3 tmAxis;
		IObjParam* ip;
		IPoint2 mBase;
		IPoint2 mCur;
		
	public:
		CoreExport Transformer(IObjParam* i);
		CoreExport virtual ~Transformer();
		CoreExport virtual void BeginDrag( IPoint2& m, Matrix3& tmAxis );
		CoreExport virtual void EndDrag( IPoint2& m );
		CoreExport void SetMouse( IPoint2& m );
#ifdef _OSNAP
		CoreExport virtual void SnapPreview(ViewExp* vpt, IPoint2 in, IPoint2 out, Matrix3* m , DWORD flags){};
#endif
		virtual void Accumulate() {mBase=mCur;}
		Matrix3& Axis() { return tmAxis; }
		IPoint2 Base() { return mBase; }
	};

class MoveTransformer : public Transformer
{
 		Point3 lastDelta;
		Point3 absSnapOrigin;
		BOOL selfSnap;
 	public:
 		void SetSelfSnap(BOOL ss) {selfSnap=ss;}
		BOOL GetSelfSnap() {return selfSnap;}
		CoreExport void BeginDrag( IPoint2& m, Matrix3& tmAxis );
		CoreExport void EndDrag( IPoint2& m );
 		CoreExport Point3 Eval(ViewExp* vpt,BOOL status=TRUE);
		CoreExport MoveTransformer(IObjParam* i,BOOL so=FALSE);
		CoreExport void Accumulate();
#ifdef _OSNAP
		CoreExport void SnapPreview(ViewExp* vpt, IPoint2 in, IPoint2 out, Matrix3 *m , DWORD flags);
#endif
 		};	

class RotateTransformer : public Transformer {
 		Quat qPrev;
		bool   mSelfSnap;
		Point3 mSnapOrigin;

#ifndef NO_THREE_POINT_ALIGN
		class TPARotateTransformerImpl;
		friend class TPARotateTransformerImpl;
		MaxSDK::Util::AutoPtr<TPARotateTransformerImpl> mTPARotateTransformerImpl;
#endif

 	public:
 		CoreExport AngAxis Eval(ViewExp* vpt,BOOL status=TRUE);
		CoreExport void BeginDrag( IPoint2& m, Matrix3& tmAxis );
		CoreExport void EndDrag( IPoint2& m );
		//gets the current rotation defined by the mouse point projected in the current plane.
		CoreExport AngAxis GetMouseOrientation(ViewExp* vpt);
		
		CoreExport void SetSelfSnap(bool ss);
		CoreExport bool GetSelfSnap();
		CoreExport RotateTransformer(IObjParam* i,bool so=false);
		CoreExport virtual ~RotateTransformer();

#ifdef _OSNAP
		CoreExport void SnapPreview(ViewExp* vpt, IPoint2 in, IPoint2 out, Matrix3 *m , DWORD flags);
#endif
 		};	

class ScaleTransformer : public Transformer {
 	public:
 		CoreExport Point3 Eval(ViewExp* vpt,BOOL status=TRUE);
		ScaleTransformer(IObjParam* i) : Transformer(i) {}
#ifdef _OSNAP
		CoreExport void BeginDrag( IPoint2& m, Matrix3& tmAxis );
		CoreExport void SnapPreview(ViewExp* vpt, IPoint2 in, IPoint2 out, Matrix3 *m , DWORD flags);
#endif
 		};	


/*! \sa  Class CommandMode, Class IScene.\n\n
\par Description:
The purpose of this callback is to call <b>INode::FlagForeground()</b> for any
nodes in the scene that are supposed to be in the foreground.  */
class ChangeFGObject : public ChangeForegroundCallback
{
		ReferenceTarget* obj;
		BOOL valid;
	public:		
		ChangeFGObject() { obj = NULL; valid = TRUE; }
		ChangeFGObject(ReferenceTarget* o) { obj = o; valid = TRUE; }
		void SetObj(ReferenceTarget* o ) { obj = o; }
		
		/*! \remarks Returns TRUE if this foreground callback is valid ; otherwise FALSE.
		When the system needs to redraw the viewports the system checks to see
		if the current foreground callback is the same one that was in place
		the last time. If it is the same the system will call this method to
		see if the state is valid. If the state is not valid (this method
		returns FALSE) then the foreground and background are re-built and then
		the <b>Validate()</b> method is called so this foreground callback may
		note that it is invalid. */
		BOOL IsValid() { return valid; }
		/*! \remarks The system calls this method of the plug-in when the foreground state
		is no longer valid. The plug-in should set a flag internally to
		indicate it is invalid. For example if the current node selection set
		changes the system calls this method to mark it as invalid so that the
		next time the system goes to redraw the viewports it can determine that
		it would need to rebuild the foreground and the background. */
		void Invalidate() { valid = FALSE; }
		/*! \remarks The system calls this method when the foreground state is valid. For
		example after the foreground and background buffers have been re-built
		this method is called. The plug-in should set a flag internally to
		indicate it is valid. */
		void Validate() { valid = TRUE; }
		/*! \remarks This method is used to enumerate the scene and flag nodes in the scene
		that need to go in the foreground.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to flag the nodes.\n\n
		<b>IScene *scene</b>\n\n
		This interface pointer provides methods to enumerate all nodes in the
		scene, or to flag nodes that are selected, animated or dependent on a
		given node. */
		void callback(TimeValue t,IScene *scene)
		{
			obj->FlagDependents(t);
		}
	};



class SelectionProcessor : public MouseCallBack {
	friend class PaintSelectionProcessor; //used internally
	private:
		MouseCallBack *mcallback;
		BOOL brokenThresh, hitSel, drag, toggle, cloning, clear, invert;
		IPoint2 offset;
		IPoint2 om,lm;

	protected:
		IObjParam* ip;

		virtual BOOL HitTest( ViewExp* vpt, IPoint2 *p, int type, int flags )=0;
		virtual BOOL AnyHits( ViewExp* vpt )=0;
		virtual HCURSOR GetTransformCursor()=0;
		virtual void Select(ViewExp* vpt,BOOL all,BOOL clear,BOOL invert)=0;
		virtual void DeSelect(ViewExp* vpt,BOOL all)=0;
		virtual void ClearSelection()=0;
		virtual void CloneSelected(int initialCall=TRUE)=0;
		virtual void AbortClone()=0;
		virtual void AcceptClone()=0;
		virtual void SelectChildren(ViewExp* vpt) {}

	public:
		SelectionProcessor(MouseCallBack *m,IObjParam* i) 
			{ mcallback = m; ip = i; offset = IPoint2(0,0); }
		
		CoreExport virtual int proc( 
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );

		CoreExport void pan(IPoint2 d);
	};


class TransformModBox : public MouseCallBack {
	protected:
		BaseObject* obj;
		IObjParam* ip;		
		Matrix3 ptm;

	public:
		CoreExport TransformModBox(BaseObject* o, IObjParam* i);
		CoreExport ~TransformModBox();

		// These are called before and after the DoTransform operations
		CoreExport virtual void PreTransform();
		CoreExport virtual void PreTransformHolding();
		CoreExport virtual void PostTransformHolding();
		CoreExport virtual void PostTransform();
		CoreExport virtual void CancelTransform();

		virtual Transformer& GetTransformer()=0;
		virtual void DoTransform(ViewExp* vpt)=0;
		virtual HCURSOR GetTransformCursor()=0;
		virtual int UndoStringID()=0;

		CoreExport int proc( 
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
	};


class GenModSelectionProcessor : public SelectionProcessor {
	protected:
		BaseObject* obj;
		BOOL transformGizmoActive;

		CoreExport BOOL HitTest( ViewExp* vpt, IPoint2 *p, int type, int flags );
		BOOL AnyHits( ViewExp* vpt ) { return vpt->NumSubObjHits(); }		
		CoreExport void Select(ViewExp* vpt,BOOL all,BOOL clear,BOOL invert);
		CoreExport void DeSelect(ViewExp* vpt,BOOL all);
		CoreExport void ClearSelection();
		CoreExport void CloneSelected(int initialCall=TRUE);
		CoreExport void AbortClone();
		CoreExport void AcceptClone();

	public:
		GenModSelectionProcessor(MouseCallBack *mc, BaseObject* o, IObjParam* i) 
			: SelectionProcessor(mc,i) {obj = o; transformGizmoActive=FALSE; }
	};

class SubModSelectionProcessor : public GenModSelectionProcessor {
	private:		
		TransformModBox *tproc;
		BOOL supportTransformGizmo;

	protected:
		HCURSOR GetTransformCursor() { return tproc ? tproc->GetTransformCursor() : LoadCursor(NULL, IDC_ARROW); }
		
	public:
		SubModSelectionProcessor(TransformModBox *mc, BaseObject* o, IObjParam* i) 
			: GenModSelectionProcessor(mc,o,i) { 
				tproc = mc; supportTransformGizmo=FALSE; }

		// Transform Gizmo Interface
		BOOL SupportTransformGizmo() { return supportTransformGizmo; }
		void DeactivateTransformGizmo() {
			if (transformGizmoActive) {
				ip->DeactivateTransformGizmo();
				transformGizmoActive = FALSE;
				}
			}
		// End of Transform Gizmo Interface
		// Private gizmo stuff
		void SetSupportTransformGizmo(BOOL b) { supportTransformGizmo = b; }
	};


class MoveModBox : public TransformModBox {
	private:
		MoveTransformer moveTrans;
	public:
		MoveModBox(BaseObject* o, IObjParam* i) : moveTrans(i,TRUE), TransformModBox(o,i) {}
		Transformer& GetTransformer() { return moveTrans; }
		CoreExport void DoTransform(ViewExp* vpt);
		HCURSOR GetTransformCursor() { return ip->GetSysCursor(SYSCUR_MOVE); }
		CoreExport int UndoStringID();
		};
class RotateModBox : public TransformModBox {
	private:
		RotateTransformer rotTrans;
	public:
		RotateModBox(BaseObject* o, IObjParam* i) : rotTrans(i), TransformModBox(o,i) {}
		Transformer& GetTransformer() { return rotTrans; }
		CoreExport void DoTransform(ViewExp* vpt);
		HCURSOR GetTransformCursor() { return ip->GetSysCursor(SYSCUR_ROTATE); }
		CoreExport int UndoStringID();
		};
class ScaleModBox : public TransformModBox {
	private:
		ScaleTransformer scaleTrans;
	public:
		ScaleModBox(BaseObject* o, IObjParam* i) : scaleTrans(i), TransformModBox(o,i) {}
		Transformer& GetTransformer() { return scaleTrans; }
		CoreExport void DoTransform(ViewExp* vpt);
		CoreExport HCURSOR GetTransformCursor();
		CoreExport int UndoStringID();
		};
class SelectModBox : public TransformModBox {
	private:
		MoveTransformer moveTrans;
	public:
		SelectModBox(BaseObject* o, IObjParam* i) : moveTrans(i), TransformModBox(o,i) {}
		Transformer& GetTransformer() { return moveTrans; }
		void DoTransform(ViewExp* vpt) {}
		HCURSOR GetTransformCursor() { return ip->GetSysCursor(SYSCUR_SELECT); }
		CoreExport int UndoStringID();
		};


class MoveModBoxCMode : public CommandMode
{
	private:
		ChangeFGObject fgProc;
		SubModSelectionProcessor mouseProc;
		MoveModBox transProc;
		IObjParam* ip;

	public:
		MoveModBoxCMode( BaseObject *o, IObjParam *i ) : 
			fgProc((ReferenceTarget*)o), transProc(o,i), mouseProc(&transProc,o,i) 
				{ ip = i; mouseProc.SetSupportTransformGizmo(TRUE); }
		
		int Class() { return MOVE_COMMAND; }
		int ID() { return CID_SUBOBJMOVE; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode* oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode() { ip->SetToolButtonState(MOVE_BUTTON,TRUE); }
		void ExitMode() { ip->SetToolButtonState(MOVE_BUTTON,FALSE); }
	};

class RotateModBoxCMode : public CommandMode
{
	private:
		ChangeFGObject fgProc;
		SubModSelectionProcessor mouseProc;
		RotateModBox transProc;
		IObjParam* ip;

	public:
		RotateModBoxCMode( BaseObject* o, IObjParam* i ) : 
			fgProc((ReferenceTarget*)o), transProc(o,i), mouseProc(&transProc,o,i) 
				{ ip = i; mouseProc.SetSupportTransformGizmo(TRUE); }
		
		int Class() { return ROTATE_COMMAND; }
		int ID() { return CID_SUBOBJROTATE; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode* oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode() { ip->SetToolButtonState(ROTATE_BUTTON,TRUE); }
		void ExitMode() { ip->SetToolButtonState(ROTATE_BUTTON,FALSE); }
	};

class UScaleModBoxCMode : public CommandMode
{
	private:
		ChangeFGObject fgProc;
		SubModSelectionProcessor mouseProc;
		ScaleModBox transProc;
		IObjParam* ip;

	public:
		UScaleModBoxCMode( BaseObject* o, IObjParam* i ) : 
			fgProc((ReferenceTarget*)o), transProc(o,i), mouseProc(&transProc,o,i) 
				{ ip = i; mouseProc.SetSupportTransformGizmo(TRUE); }
		
		int Class() { return USCALE_COMMAND; }
		int ID() { return CID_SUBOBJUSCALE; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode* oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode() { ip->SetToolButtonState(USCALE_BUTTON,TRUE); }
		void ExitMode() { ip->SetToolButtonState(USCALE_BUTTON,FALSE); }
	};

class NUScaleModBoxCMode : public CommandMode {
	
	private:
		ChangeFGObject fgProc;
		SubModSelectionProcessor mouseProc;
		ScaleModBox transProc;
		IObjParam* ip;

	public:
		NUScaleModBoxCMode( BaseObject* o, IObjParam* i ) : 
			fgProc((ReferenceTarget*)o), transProc(o,i), mouseProc(&transProc,o,i) 
				{ ip = i; mouseProc.SetSupportTransformGizmo(TRUE); }
		
		int Class() { return SCALE_COMMAND; }
		int ID() { return CID_SUBOBJSCALE; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode* oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode() { ip->SetToolButtonState(NUSCALE_BUTTON,TRUE); }
		void ExitMode() { ip->SetToolButtonState(NUSCALE_BUTTON,FALSE); }
	};

class SquashModBoxCMode : public CommandMode {
	
	private:
		ChangeFGObject fgProc;
		SubModSelectionProcessor mouseProc;
		ScaleModBox transProc;
		IObjParam* ip;

	public:
		SquashModBoxCMode( BaseObject* o, IObjParam* i ) : 
			fgProc((ReferenceTarget*)o), transProc(o,i), mouseProc(&transProc,o,i) 
				{ ip = i; mouseProc.SetSupportTransformGizmo(TRUE); }
		
		int Class() { return SQUASH_COMMAND; }
		int ID() { return CID_SUBOBJSQUASH; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode* oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode() { ip->SetToolButtonState(SQUASH_BUTTON,TRUE); }
		void ExitMode() { ip->SetToolButtonState(SQUASH_BUTTON,FALSE); }
	};

class SelectModBoxCMode : public CommandMode {
	
	private:
		ChangeFGObject fgProc;
		SubModSelectionProcessor mouseProc;
		SelectModBox transProc;
		IObjParam* ip;

	public:
		SelectModBoxCMode( BaseObject* o, IObjParam* i ) : 
			fgProc((ReferenceTarget*)o), transProc(o,i), mouseProc(NULL/*&transProc*/,o,i) 
				{ ip = i; }
		
		int Class() { return SELECT_COMMAND; }
		int ID() { return CID_SUBOBJSELECT; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode* oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode() { ip->SetToolButtonState(SELECT_BUTTON,TRUE); }
		void ExitMode() { ip->SetToolButtonState(SELECT_BUTTON,FALSE); }
	};



///////////////////////////////////////////////////////////////////////////

class TransformCtrlApparatus : public MouseCallBack {
	protected:
		Control *ctrl;
		IObjParam* ip;		
		Matrix3 ptm;

	public:
		CoreExport TransformCtrlApparatus(Control *c, IObjParam* i);
		CoreExport ~TransformCtrlApparatus();

		virtual Transformer& GetTransformer()=0;
		virtual void DoTransform(ViewExp* vpt)=0;
		virtual HCURSOR GetTransformCursor()=0;
		virtual int UndoStringID()=0;

		CoreExport int proc( 
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
	};


class GenControlSelectionProcessor : public SelectionProcessor {
	protected:
		Control *ctrl;
		BOOL transformGizmoActive;

		CoreExport BOOL HitTest( ViewExp* vpt, IPoint2 *p, int type, int flags );
		BOOL AnyHits( ViewExp* vpt ) { return vpt->NumSubObjHits(); }		
		CoreExport void Select(ViewExp* vpt,BOOL all,BOOL clear,BOOL invert);
		CoreExport void DeSelect(ViewExp* vpt,BOOL all);
		CoreExport void ClearSelection();
		void CloneSelected(int initialCall=TRUE) {};
		void AbortClone() {};
		void AcceptClone() {};

	public:
		GenControlSelectionProcessor(MouseCallBack *mc,Control *c,IObjParam* i) 
			: SelectionProcessor(mc,i) {ctrl=c; transformGizmoActive=FALSE; }
	};

class SubControlSelectionProcessor : public GenControlSelectionProcessor {
	private:		
		TransformCtrlApparatus *tproc;
		BOOL supportTransformGizmo;

	protected:
		HCURSOR GetTransformCursor() { return tproc->GetTransformCursor(); }
		
	public:
		SubControlSelectionProcessor(TransformCtrlApparatus *tc,Control *c,IObjParam* i) 
			: GenControlSelectionProcessor(tc,c,i) { 
				tproc = tc; supportTransformGizmo=FALSE; }

		// Transform Gizmo Interface
		BOOL SupportTransformGizmo() { return supportTransformGizmo; }
		void DeactivateTransformGizmo() {
			if (transformGizmoActive) {
				ip->DeactivateTransformGizmo();
				transformGizmoActive = FALSE;
				}
			}
		// End of Transform Gizmo Interface
		// Private gizmo stuff
		void SetSupportTransformGizmo(BOOL b) { supportTransformGizmo = b; }

	};


class MoveCtrlApparatus : public TransformCtrlApparatus {
	private:
		MoveTransformer moveTrans;
	public:
		MoveCtrlApparatus(Control *c, IObjParam* i) : moveTrans(i,TRUE), TransformCtrlApparatus(c,i) {}
		Transformer& GetTransformer() { return moveTrans; }
		CoreExport void DoTransform(ViewExp* vpt);
		HCURSOR GetTransformCursor() { return ip->GetSysCursor(SYSCUR_MOVE); }
		CoreExport int UndoStringID();
		};
class RotateCtrlApparatus : public TransformCtrlApparatus {
	private:
		RotateTransformer rotTrans;
	public:
		RotateCtrlApparatus(Control *c, IObjParam* i) : rotTrans(i), TransformCtrlApparatus(c,i) {}
		Transformer& GetTransformer() { return rotTrans; }
		CoreExport void DoTransform(ViewExp* vpt);
		HCURSOR GetTransformCursor() { return ip->GetSysCursor(SYSCUR_ROTATE); }
		CoreExport int UndoStringID();
		};
class ScaleCtrlApparatus : public TransformCtrlApparatus {
	private:
		ScaleTransformer scaleTrans;
	public:
		ScaleCtrlApparatus(Control *c, IObjParam* i) : scaleTrans(i), TransformCtrlApparatus(c,i) {}
		Transformer& GetTransformer() { return scaleTrans; }
		CoreExport void DoTransform(ViewExp* vpt);
		CoreExport HCURSOR GetTransformCursor();
		CoreExport int UndoStringID();
		};
class SelectCtrlApparatus : public TransformCtrlApparatus {
	private:
		MoveTransformer moveTrans;
	public:
		SelectCtrlApparatus(Control *c, IObjParam* i) : moveTrans(i), TransformCtrlApparatus(c,i) {}
		Transformer& GetTransformer() { return moveTrans; }
		void DoTransform(ViewExp* vpt) {}
		HCURSOR GetTransformCursor() { return ip->GetSysCursor(SYSCUR_SELECT); }
		CoreExport int UndoStringID();
		};
#pragma warning(pop)

class MoveCtrlApparatusCMode : public CommandMode {
	
	private:
		ChangeFGObject fgProc;
		SubControlSelectionProcessor mouseProc;
		MoveCtrlApparatus transProc;
		IObjParam* ip;

	public:
		MoveCtrlApparatusCMode( Control* c, IObjParam* i ) : 
			fgProc((ReferenceTarget*)c), transProc(c,i), mouseProc(&transProc,c,i) 
				{ ip = i; mouseProc.SetSupportTransformGizmo(TRUE); }
		
		int Class() { return MOVE_COMMAND; }
		int ID() { return CID_SUBOBJMOVE; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode* oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode() { ip->SetToolButtonState(MOVE_BUTTON,TRUE); }
		void ExitMode() { ip->SetToolButtonState(MOVE_BUTTON,FALSE); }
	};

class RotateCtrlApparatusCMode : public CommandMode {
	
	private:
		ChangeFGObject fgProc;
		SubControlSelectionProcessor mouseProc;
		RotateCtrlApparatus transProc;
		IObjParam* ip;

	public:
		RotateCtrlApparatusCMode( Control *c, IObjParam* i ) : 
			fgProc((ReferenceTarget*)c), transProc(c,i), mouseProc(&transProc,c,i) 
				{ ip = i; mouseProc.SetSupportTransformGizmo(TRUE); }
		
		int Class() { return ROTATE_COMMAND; }
		int ID() { return CID_SUBOBJROTATE; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode* oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode() { ip->SetToolButtonState(ROTATE_BUTTON,TRUE); }
		void ExitMode() { ip->SetToolButtonState(ROTATE_BUTTON,FALSE); }
	};

class UScaleCtrlApparatusCMode : public CommandMode {
	
	private:
		ChangeFGObject fgProc;
		SubControlSelectionProcessor mouseProc;
		ScaleCtrlApparatus transProc;
		IObjParam* ip;

	public:
		UScaleCtrlApparatusCMode( Control *c, IObjParam* i ) : 
			fgProc((ReferenceTarget*)c), transProc(c,i), mouseProc(&transProc,c,i) 
				{ ip = i; mouseProc.SetSupportTransformGizmo(TRUE); }
		
		int Class() { return USCALE_COMMAND; }
		int ID() { return CID_SUBOBJUSCALE; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode* oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode() { ip->SetToolButtonState(USCALE_BUTTON,TRUE); }
		void ExitMode() { ip->SetToolButtonState(USCALE_BUTTON,FALSE); }
	};

class NUScaleCtrlApparatusCMode : public CommandMode {
	
	private:
		ChangeFGObject fgProc;
		SubControlSelectionProcessor mouseProc;
		ScaleCtrlApparatus transProc;
		IObjParam* ip;

	public:
		NUScaleCtrlApparatusCMode( Control *c, IObjParam* i ) : 
			fgProc((ReferenceTarget*)c), transProc(c,i), mouseProc(&transProc,c,i) 
				{ ip = i; mouseProc.SetSupportTransformGizmo(TRUE); }
		
		int Class() { return SCALE_COMMAND; }
		int ID() { return CID_SUBOBJSCALE; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode* oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode() { ip->SetToolButtonState(NUSCALE_BUTTON,TRUE); }
		void ExitMode() { ip->SetToolButtonState(NUSCALE_BUTTON,FALSE); }
	};

class SquashCtrlApparatusCMode : public CommandMode {
	
	private:
		ChangeFGObject fgProc;
		SubControlSelectionProcessor mouseProc;
		ScaleCtrlApparatus transProc;
		IObjParam* ip;

	public:
		SquashCtrlApparatusCMode( Control *c, IObjParam* i ) : 
			fgProc((ReferenceTarget*)c), transProc(c,i), mouseProc(&transProc,c,i) 
				{ ip = i; mouseProc.SetSupportTransformGizmo(TRUE); }
		
		int Class() { return SQUASH_COMMAND; }
		int ID() { return CID_SUBOBJSQUASH; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode* oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode() { ip->SetToolButtonState(SQUASH_BUTTON,TRUE); }
		void ExitMode() { ip->SetToolButtonState(SQUASH_BUTTON,FALSE); }
	};

class SelectCtrlApparatusCMode : public CommandMode {
	
	private:
		ChangeFGObject fgProc;
		SubControlSelectionProcessor mouseProc;
		SelectCtrlApparatus transProc;
		IObjParam* ip;

	public:
		SelectCtrlApparatusCMode( Control *c, IObjParam* i ) :
			fgProc((ReferenceTarget*)c), transProc(c,i), mouseProc(NULL,c,i) 
				{ ip = i; }
		
		int Class() { return SELECT_COMMAND; }
		int ID() { return CID_SUBOBJSELECT; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode* oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode() { ip->SetToolButtonState(SELECT_BUTTON,TRUE); }
		void ExitMode() { ip->SetToolButtonState(SELECT_BUTTON,FALSE); }
	};



#define MOVE_INTERSECTION	1
#define MOVE_PROJECTION		2
/*! \remarks This global function is not part of class MouseCallBack but is
available for use.\n\n
Returns the mouse speed value used in perspective viewports. This value
corresponds to the value that the user may set using the 3ds Max command in the
File / Preferences . . . Move / Rotate Transform area. */
CoreExport float GetPerspMouseSpeed();
/*! \remarks This global function is not part of class MouseCallBack but is
available for use.\n\n
Sets the mouse speed value used in perspective viewports. This value
corresponds to the value that the user may set using the 3ds Max command in the
File / Preferences . . . Move / Rotate Transform area.
\par Parameters:
<b>float speed</b>\n\n
The value to set. */
CoreExport void SetPerspMouseSpeed(float speed);
/*! \remarks This global function is not part of class MouseCallBack but is
available for use.\n\n
Sets the mouse move mode. This value corresponds to the value that the user may
set using the 3ds Max command in the File / Preferences . . . Move / Rotate
Transform area.
\par Parameters:
<b>int moveType</b>\n\n
One of the following values:\n\n
<b>MOVE_INTERSECTION</b>\n\n
<b>MOVE_PROJECTION</b> */
CoreExport void SetMoveModeType(int moveType);
/*! \remarks This global function is not part of class MouseCallBack but is
available for use.\n\n
Returns the mouse move mode. This value corresponds to the value that the user
may set using the 3ds Max command in the File / Preferences . . . Move / Rotate
Transform area.
\return  One of the following values:\n\n
<b>MOVE_INTERSECTION</b>\n\n
<b>MOVE_PROJECTION</b> */
CoreExport int GetMoveModeType();

CoreExport void SetRotationIncrement(float inc);
CoreExport float GetRotationIncrement();

CoreExport int GetPaintSelBrushSize();
CoreExport void SetPaintSelBrushSize(int size);
