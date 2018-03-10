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
// FILE:        interpik.h
// DESCRIPTION: Implements IK related methods for interp controllers
// AUTHOR:      Rolf Berteig
// HISTORY:     created 6/19/95
//**************************************************************************/

#pragma once
#include "maxheap.h"
#include "hold.h"
#include "control.h"

// forward declarations
class Control;
class ISpinnerControl;
class IKEnumCallback;
class IKDeriv;
class IKClipObject;
class InitJointData;
class InitJointData2;

#define PROPID_INTERPUI		(PROPID_USER+1)
#define PROPID_JOINTPARAMS	(PROPID_USER+2)
#define PROPID_KEYINFO		(PROPID_USER+3)

// Flags for JointParams
#define JNT_XACTIVE		(1<<0)
#define JNT_YACTIVE		(1<<1)
#define JNT_ZACTIVE		(1<<2)
#define JNT_XLIMITED	(1<<3)
#define JNT_YLIMITED	(1<<4)
#define JNT_ZLIMITED	(1<<5)
#define JNT_XEASE		(1<<6)
#define JNT_YEASE		(1<<7)
#define JNT_ZEASE		(1<<8)
#define JNT_XSPRING		(1<<9)
#define JNT_YSPRING		(1<<10)
#define JNT_ZSPRING		(1<<11)

#define JNT_PARAMS2		(1<<12) // If this bit is set, the structure is a JointParams2
#define JNT_PARAMS_EULER (1<<13) // If this bit is set, the structure is a JointParamsEuler

#define JP_HELD			(1<<27)
#define JNT_LIMITEXACT	(1<<28)
#define JNT_ROLLOPEN	(1<<29)
#define JNT_ROT			(1<<30) 
#define JNT_POS			(1<<31)

class JointParams2;

/*! \sa  Class AnimProperty,
Class JointParams.\n\n
\par Description:
This is simply a container class to hold some data while the controllers
parameters are being edited. All methods of this class are implemented by the
system.
\par Data Members:
<b>HWND hParams;</b>\n\n
The window handle of the rollup page.\n\n
<b>IObjParam *ip;</b>\n\n
The interface pointer.\n\n
<b>Control *cont;</b>\n\n
The controller that is being edited.  */
class InterpCtrlUI : public AnimProperty {
	public:
		HWND hParams;
		IObjParam *ip;
		Control *cont;
		
		/*! \remarks Constructor. The data members are initialized to the
		values passed. */
		InterpCtrlUI(HWND h,IObjParam *i,Control *c) 
			{hParams=h;ip=i;cont=c;}
		
		/*! \remarks Destructor. */
		~InterpCtrlUI() {}
		/*! \remarks Returns the property list of id: <b>PROPID_INTERPUI</b>
		*/
		DWORD ID() {return PROPID_INTERPUI;}		
	};

class InterpKeyInfo : public AnimProperty {
	public:
		DWORD ID() {return PROPID_KEYINFO;}
		virtual ~InterpKeyInfo() {}
	};


// IK Joint parameters
/*! \sa  Class AnimProperty, Class Control, Class InterpCtrlUI.\n\n
\par Description:
This class handles the data storage and user interface for inverse kinematic
joint parameters. The default 3ds Max controllers use this data structure to
store their IK information. Plug-in controllers don't have to unless they want
to.\n\n
Note the following is a dialog proc for handling joint parameters that is
exported for use by plug-ins.\n\n
<b>BOOL CALLBACK JointParamDlgProc(HWND hWnd,UINT message,</b>\n\n
<b> WPARAM wParam,LPARAM lParam);</b>
\par Data Members:
<b>float *min, *max;</b>\n\n
Pointers to an array of floats corresponding to the number of degrees of
freedom These are the From and To parameters.\n\n
<b>float *damping;</b>\n\n
Pointer to an array of floating point Damping parameters for each degree of
freedom.\n\n
<b>float scale;</b>\n\n
This is a scale factor applied to the values in the spinner edit fields. This
is to make them more sensible to the user. For example a percentage that is
stored internally as 0.0 to 1.0 could be presented to the user as 0.0 to 100.0
by using a scale of 100.\n\n
<b>DWORD flags;</b>\n\n
One or more of the following values:\n\n
<b>JNT_XACTIVE</b>\n\n
<b>JNT_YACTIVE</b>\n\n
<b>JNT_ZACTIVE</b>\n\n
<b>JNT_XLIMITED</b>\n\n
<b>JNT_YLIMITED</b>\n\n
<b>JNT_ZLIMITED</b>\n\n
<b>JNT_XEASE</b>\n\n
<b>JNT_YEASE</b>\n\n
<b>JNT_ZEASE</b>\n\n
<b>JNT_LIMITEXACT</b>\n\n
<b>JNT_ROLLOPEN</b>\n\n
<b>JNT_ROT</b>\n\n
<b>JNT_POS</b>\n\n
<b>int dofs;</b>\n\n
The number of degrees of freedom the plug-in has.
\par Operators:
*/
class JointParams : public AnimProperty {
	public:
		float *min, *max;
		float *damping;
		float *spring;
		float *stens;
		float scale;
		DWORD flags;
		int dofs;

		/*! \remarks Constructor. The data members are initialized to the
		values passed. */
		CoreExport JointParams(DWORD type=JNT_POS,int dofs=3,float s=1.0f);
		/*! \remarks Constructor. The data members are initialized to those of
		the JointParam passed. */
		CoreExport JointParams(const JointParams &j);
		/*! \remarks Destructor. */
		CoreExport ~JointParams();
		/*! \remarks Implemented by the System.\n\n
		Returns the ID of the AnimProperty - <b>PROPID_JOINTPARAMS</b> */
		DWORD ID() {return PROPID_JOINTPARAMS;}

		/*! \remarks Assignment operator.  */
		CoreExport JointParams&  operator=(JointParams& j);

		// Returns TRUE if the curent state is the default.
		/*! \remarks Implemented by the System.\n\n
		Returns TRUE if the current state of the parameters are the defaults.
		*/
		CoreExport BOOL IsDefault();
		
		/*! \remarks Implemented by the System.\n\n
		This method may be called to save the joint properties to the 3ds Max
		file.
		\par Parameters:
		<b>ISave *isave</b>\n\n
		This pointer may be used to call methods to write data to disk. See
		Class ISave.
		\return  One of the following values:\n\n
		<b>IO_OK</b> - The result was acceptable - no errors.\n\n
		<b>IO_ERROR</b> - This is returned if an error occured. */
		CoreExport IOResult Save(ISave *isave);
		/*! \remarks Implemented by the System.\n\n
		This method is called to load the joint properties from the 3ds Max
		file.
		\par Parameters:
		<b>ILoad *iload</b>\n\n
		This pointer may be used to call methods to load data from disk. See
		Class ILoad.
		\return  One of the following values:\n\n
		<b>IO_OK</b> - The result was acceptable - no errors.\n\n
		<b>IO_ERROR</b> - This is returned if an error occured. */
		CoreExport IOResult Load(ILoad *iload);

		// Applies contraints to the given delta based on parameters and the current value v.
		/*! \remarks Implemented by the System.\n\n
		This method applies constraints to the given delta based on parameters
		and the current value v. It uses the current min/max limits to
		constrain the result: v+delta so that v+delta \< max and v+delta \>
		min. It returns a new delta such that the previous will both be TRUE.
		If ease is turned on, then the values will be slowed down as they
		approach the limits. It also applies the damping if turned on.
		\par Parameters:
		<b>int index</b>\n\n
		This is the index of the parameter. For example on a position
		controller the index could be 0 (x), 1 (y), or 2 (z).\n\n
		<b>float v</b>\n\n
		The current value of the parameter.\n\n
		<b>float delta</b>\n\n
		The increment to apply.
		\return  A new delta value. Usually it will return <b>delta</b>, but if
		the value was constrained, then the value may be smaller or larger. */
		CoreExport float ConstrainInc(int index,float v,float delta);		
		
		// Access bits
		/*! \remarks Implemented by the System.\n\n
		Returns TRUE if the specified joint is active; otherwise FALSE.
		\par Parameters:
		<b>int i</b>\n\n
		One of the following values:\n\n
		<b>0</b> = X\n\n
		<b>1</b> = Y\n\n
		<b>2</b> = Z
		\return  TRUE if the joint is active; otherwise FALSE. */
		BOOL Active(int i) {return (flags&(JNT_XACTIVE<<i))?TRUE:FALSE;}
		/*! \remarks Implemented by the System.\n\n
		Returns TRUE if the joint is limited; otherwise FALSE.
		\par Parameters:
		<b>int i</b>\n\n
		One of the following values:\n\n
		<b>0</b> = X\n\n
		<b>1</b> = Y\n\n
		<b>2</b> = Z
		\return  TRUE if the joint is limited; otherwise FALSE. */
		BOOL Limited(int i) {return (flags&(JNT_XLIMITED<<i))?TRUE:FALSE;}
		/*! \remarks Implemented by the System.\n\n
		Returns TRUE if the joint has the Ease property set; otherwise FALSE.
		\par Parameters:
		<b>int i</b>\n\n
		One of the following values:\n\n
		<b>0</b> = X\n\n
		<b>1</b> = Y\n\n
		<b>2</b> = Z
		\return  TRUE if the joint has the Ease property set; otherwise FALSE.
		*/
		BOOL Ease(int i) {return (flags&(JNT_XEASE<<i))?TRUE:FALSE;}
		BOOL Spring(int i) {return (flags&(JNT_XSPRING<<i))?TRUE:FALSE;}
		/*! \remarks Implemented by the System.\n\n
		Returns the type of joint, sliding or rotation. This will either be
		<b>JNT_POS</b> for sliding joints of <b>JNT_ROT</b> for rotational
		joints. */
		DWORD Type() {return flags & (JNT_POS|JNT_ROT);}
		/*! \remarks Implemented by the System.\n\n
		Returns TRUE if the rollup page if open; otherwise FALSE. */
		BOOL RollupOpen() {return (flags&JNT_ROLLOPEN)?TRUE:FALSE;}
		/*! \remarks Implemented by the System.\n\n
		Sets the specified joint to the specified active or inactive state.
		\par Parameters:
		<b>int i</b>\n\n
		One of the following values:\n\n
		<b>0</b> = X\n\n
		<b>1</b> = Y\n\n
		<b>2</b> = Z\n\n
		<b>BOOL s</b>\n\n
		TRUE to set the joint active; otherwise FALSE. */
		void SetActive(int i,BOOL s) {if (s) flags|=(JNT_XACTIVE<<i); else flags&=~(JNT_XACTIVE<<i);}
		/*! \remarks Implemented by the System.\n\n
		Sets the specified joint to the specified limited or not limited state.
		\par Parameters:
		<b>int i</b>\n\n
		One of the following values:\n\n
		<b>0</b> = X\n\n
		<b>1</b> = Y\n\n
		<b>2</b> = Z\n\n
		<b>BOOL s</b>\n\n
		TRUE to set the joint as limited; otherwise FALSE. */
		void SetLimited(int i,BOOL s) {if (s) flags|=(JNT_XLIMITED<<i); else flags&=~(JNT_XLIMITED<<i);}
		/*! \remarks Implemented by the System.\n\n
		Sets the specified joint to the specified eased or not eased state.
		\par Parameters:
		<b>int i</b>\n\n
		One of the following values:\n\n
		<b>0</b> = X\n\n
		<b>1</b> = Y\n\n
		<b>2</b> = Z\n\n
		<b>BOOL s</b>\n\n
		TRUE to set the joint as eased; otherwise FALSE. */
		void SetEase(int i,BOOL s) {if (s) flags|=(JNT_XEASE<<i); else flags&=~(JNT_XEASE<<i);}
		void SetSpring(int i,BOOL s) {if (s) flags|=(JNT_XSPRING<<i); else flags&=~(JNT_XSPRING<<i);}
		/*! \remarks Implemented by the System.\n\n
		Sets the type of joint.
		\par Parameters:
		<b>DWORD type</b>\n\n
		Specifies the type of joint. One of the following values:\n\n
		<b>JNT_POS</b> - Sliding joint.\n\n
		<b>JNT_ROT</b> - Rotating joint. */
		void SetType(DWORD type) {flags&=~(JNT_POS|JNT_ROT);flags|=type;}
		/*! \remarks Implemented by the System.\n\n
		Set the rollup page as open or closed.
		\par Parameters:
		<b>BOOL open</b>\n\n
		TRUE to open the page; FALSE to close it. */
		void SetRollOpen(BOOL open) {if (open) flags|=JNT_ROLLOPEN; else flags&= ~JNT_ROLLOPEN;}

		// This is the interactive adjustment of limits
		/*! \remarks This is called when the user is interactively manipulating one of the
		spinner controls or enters a value into a spinner's edit field. This
		method has a default implementation.
		\par Parameters:
		<b>InterpCtrlUI *ui</b>\n\n
		This is simply a container class to hold some data while the
		controllers parameters are being edited.\n\n
		<b>WORD id</b>\n\n
		The spinner control id.\n\n
		<b>ISpinnerControl *spin</b>\n\n
		A pointer to the spinner control.\n\n
		<b>BOOL interactive</b>\n\n
		TRUE if the user is doing an interactive adjustment; otherwise
		FALSE.\n\n
		  */
		CoreExport virtual void SpinnerChange(InterpCtrlUI *ui,WORD id,ISpinnerControl* spin,BOOL interactive);

		// These methods manage the joint parameters dialog.
		/*! \remarks Implemented by the System.\n\n
		This is used internally. */
		CoreExport void InitDialog(InterpCtrlUI *ui);
		/*! \remarks Implemented by the System.\n\n
		This is used internally. */
		CoreExport void EndDialog(InterpCtrlUI *ui,BOOL dontDel=FALSE);		
		/*! \remarks Implemented by the System.\n\n
		This is used internally. */
		CoreExport void SpinnerDown(InterpCtrlUI *ui,WORD id,ISpinnerControl *spin);
		/*! \remarks Implemented by the System.\n\n
		This is used internally. */
		CoreExport void SpinnerUp(InterpCtrlUI *ui,WORD id,ISpinnerControl *spin,BOOL accept);
		/*! \remarks Implemented by the System.\n\n
		This is used internally. */
		CoreExport void Command(InterpCtrlUI *ui,WORD notify, WORD id, HWND hCtrl);
		/*! \remarks Implemented by the System.\n\n
		This is used internally. */
		CoreExport void EnableDisable(InterpCtrlUI *ui);

		CoreExport void MirrorConstraints(int axis);

		// RB 8/25/2000: Added this method to support JointParams2
		JointParams2 *GetJointParams2Interface() {if (flags & JNT_PARAMS2) return (JointParams2*)this; else return NULL;}
	};

/*! \sa  Class JointParams\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class is derived from class <b>JointParams</b>, which is the original data
structure that holds joint parameter settings. This new derived class adds the
<b>preferredAngle</b> parameter. Note that <b>JointParams</b> is not a virtual
class and as such a flag definition of <b>JNT_PARAMS2</b> is added in order to
differentiate a <b>JointParams2</b> instances from a <b>JointParams</b>
instance. When the <b>JNT_PARAMS2</b> flag is set, a <b>JointParams</b> pointer
can be safely cast to a <b>JointParams2</b>.
\par Data Members:
<b>float *preferredAngle;</b>\n\n
The preferred angle parameter.  */
class JointParams2 : public JointParams {
	public:
		float *preferredAngle;

		/*! \remarks Constructor. The data members are initialized to the
		values passed.
		\par Parameters:
		<b>DWORD type=JNT_POS</b>\n\n
		The type, which corresponds to the <b>JointParams::flags</b>
		parameter.\n\n
		<b>int dofs = 3</b>\n\n
		The degrees of freedom for the joint.\n\n
		<b>float s = 1.0f</b>\n\n
		The scale factor. */
		CoreExport JointParams2(DWORD type=JNT_POS,int dofs=3,float s=1.0f);
		/*! \remarks Constructor. The data members are initialized to those of
		the JointParams2 passed. */
		CoreExport JointParams2(const JointParams2 &j);
		/*! \remarks Constructor. The data members are initialized to those of
		the JointParams passed. */
		CoreExport JointParams2(const JointParams  &j);
		/*! \remarks Destructor. */
		CoreExport ~JointParams2();
	};



class JPLimitsRestore : public RestoreObj {
	public:
		JointParams *jp;
		float umin[6], umax[6], uspring[6];
		float rmin[6], rmax[6], rspring[6];
		
		JPLimitsRestore(JointParams *j) {
			jp = j;
			for (int i=0; i<jp->dofs; i++) {
				umin[i]    = jp->min[i];
				umax[i]    = jp->max[i];
				uspring[i] = jp->spring[i];
				}
			}
		void Restore(int isUndo) {
			if (isUndo) {
				for (int i=0; i<jp->dofs; i++) {
					rmin[i]    = jp->min[i];
					rmax[i]    = jp->max[i];
					rspring[i] = jp->spring[i];
					}
				}
			for (int i=0; i<jp->dofs; i++) {
				jp->min[i]    = umin[i];
				jp->max[i]    = umax[i];
				jp->spring[i] = uspring[i];
				}			
			}
		void Redo() {
			for (int i=0; i<jp->dofs; i++) {
				jp->min[i]    = rmin[i];
				jp->max[i]    = rmax[i];
				jp->spring[i] = rspring[i];
				}			
			}
		void EndHold() {
			jp->flags &= ~JP_HELD;
			}
	};


// Just holds a couple of pointers.
/*! \sa  Class JointParams, Class InterpCtrlUI.\n\n
\par Description:
This class simply holds a few pointers used with joint parameter dialogs.
\par Data Members:
<b>InterpCtrlUI *ui;</b>\n\n
This points to storage where the window handle, the interface pointer, and the
controller being edited can be retrieved.\n\n
<b>JointParams *jp;</b>\n\n
This points to a class used to work with the IK joint parameters dialog. <br>
*/
class JointDlgData: public MaxHeapOperators {
	public:
		InterpCtrlUI *ui;
		JointParams *jp;	
		/*! \remarks Constructor. The data members are initialized to the
		values passed. */
		JointDlgData(InterpCtrlUI *ui,JointParams *jp) {this->ui=ui;this->jp=jp;}
	};

// A window proc for handling joint parameters.
CoreExport INT_PTR CALLBACK JointParamDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);


// Handles the IK functions for all types of point3 and quat key frame controllers.
void QuatEnumIKParams(Control *cont,IKEnumCallback &callback);
BOOL QuatCompDeriv(Control *cont,TimeValue t,Matrix3& ptm,IKDeriv& derivs,DWORD flags);
float QuatIncIKParam(Control *cont,TimeValue t,int index,float delta);
CoreExport void QuatBeginIKParams(Control *cont,IObjParam *ip, ULONG flags,Animatable *prev);
void Point3EnumIKParams(Control *cont,IKEnumCallback &callback);
BOOL Point3CompDeriv(Control *cont,TimeValue t,Matrix3& ptm,IKDeriv& derivs,DWORD flags);
float Point3IncIKParam(Control *cont,TimeValue t,int index,float delta);
CoreExport void Point3BeginIKParams(Control *cont,IObjParam *ip, ULONG flags,Animatable *prev);

CoreExport BOOL CanCopyIKParams(Control *cont,int which);
CoreExport IKClipObject* CopyIKParams(Control *cont,int which);
CoreExport BOOL CanPasteIKParams(Control *cont,IKClipObject *co,int which);
CoreExport void PasteIKParams(Control *cont,IKClipObject *co,int which);

CoreExport void InitIKJointsPos(Control *cont,InitJointData *posData);
CoreExport void InitIKJointsRot(Control *cont,InitJointData *rotData);
CoreExport BOOL GetIKJointsPos(Control *cont,InitJointData *posData);
CoreExport BOOL GetIKJointsRot(Control *cont,InitJointData *rotData);

CoreExport void InitIKJointsPos(Control *cont,InitJointData2 *posData);
CoreExport void InitIKJointsRot(Control *cont,InitJointData2 *rotData);
CoreExport BOOL GetIKJointsPos(Control *cont,InitJointData2 *posData);
CoreExport BOOL GetIKJointsRot(Control *cont,InitJointData2 *rotData);

CoreExport void QuatMirrorIKConstraints(Control *cont,int axis,int which);

class StdIKClipObject : public IKClipObject {
	public:
		JointParams *jp;
		SClass_ID sid;
		Class_ID cid;
		
		StdIKClipObject(SClass_ID s,Class_ID c,JointParams *j) 
			{sid=s;cid=c;jp=j;}
		CoreExport ~StdIKClipObject();
		SClass_ID 	SuperClassID() {return sid;}
		Class_ID	ClassID() {return cid;}		
		CoreExport void DeleteThis();
	};


#define SPRINGTENS_UI	(50.0f)
#define DEF_SPRINGTENS	(0.02f)


