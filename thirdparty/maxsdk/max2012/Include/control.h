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
// FILE:        control.h
// DESCRIPTION: Control definitions
// AUTHOR:      Dan Silva and Rolf Berteig
// HISTORY:     created 9 September 1994
//**************************************************************************/

#pragma once

#include "maxheap.h"
#include "plugapi.h"
#include "assert1.h"
#include "matrix3.h"
#include "quat.h"
#include "interval.h"
#include "ref.h"
#include "box3.h"
#include "bitarray.h"
#include "AnimPropertyID.h"

// forward declarations
class ScaleValue;
class ViewExp;
class INode;
class XFormModes;
class INodeTab;
class View;
class Control;
class Object;
class SubObjAxisCallback;

extern CoreExport void ApplyScaling(Matrix3& m, const ScaleValue& v);
extern CoreExport void InitControlLists();




/*! \remarks Returns the sum of two ScaleValues. This still multiplies since
scale values are multiplicative not additive. */
CoreExport ScaleValue operator+(const ScaleValue& s0, const ScaleValue& s1);
/*! \remarks Returns the difference of two ScaleValues. */
CoreExport ScaleValue operator-(const ScaleValue& s0, const ScaleValue& s1);
/*! \remarks Multiplication of a ScaleValue and a float. */
CoreExport ScaleValue operator*(const ScaleValue& s, float f);
/*! \remarks Multiplication of a ScaleValue and a float. */
CoreExport ScaleValue operator*(float f, const ScaleValue& s);
/*! \remarks Returns the sum of a ScaleValue and a float. This adds f to s.x,
s.y, and s.z. */
CoreExport ScaleValue operator+(const ScaleValue& s, float f);
/*! \remarks Returns the sum of a ScaleValue and a float. */
CoreExport ScaleValue operator+(float f, const ScaleValue& s);

/*! \sa  Class Point3, Class Quat.\n\n
\par Description:
A ScaleValue describes an arbitrary non-uniform scaling in an arbitrary axis
system. The Point3 <b>s</b> gives the scaling along the x, y, and z axes, and
the quaternion <b>q</b> defines the axis system in which scaling is to be
applied. All methods are implemented by the system.
\par Data Members:
<b>Point3 s;</b>\n\n
Scale components.\n\n
<b>Quat q;</b>\n\n
The axis system of application.  */
class ScaleValue: public MaxHeapOperators {
   public:
   Point3 s;
   Quat q;
   /*! \remarks Constructor. No initialization is performed. */
   ScaleValue() { /* NO INIT! */ }
   /*! \remarks Constructor. The scale data member is initialized to
   <b>as</b>. The quaternion data member is set to the identity. */
   ScaleValue(const Point3& as) { s = as; q = IdentQuat(); }
   /*! \remarks Constructor. The scale data member is initialized to
   <b>as</b>. The quaternion data member is set to <b>aq</b>.
   \par Operators:
   */
   ScaleValue(const Point3& as, const Quat& aq) {s = as; q = aq; }
   /*! \remarks Adds a ScaleValue to this ScaleValue. */
   ScaleValue& operator+=(const ScaleValue& s) 
	  { (*this)=(*this)+s; return (*this);}
   /*! \remarks Multiplies this ScaleValue by a float. This updates the scale
   components. */
   ScaleValue& operator*=(const float s) 
	  { (*this)=(*this)*s; return (*this);}
   ScaleValue& operator=(const ScaleValue &v) {s=v.s;q=v.q; return (*this);}
   /*! \remarks Array access operator. This allows the scale components to be
   accessed using the array operator.
   \par Parameters:
   <b>int el</b>\n\n
   Specifies the element to access: 0=x, 1=y, 2=z.\n\n
	 */
   float& operator[](int el) {return s[el];}
   };

// Types of ORTs
#define ORT_BEFORE   1
#define ORT_AFTER 2

// Out-of-Range Types
/*! \defgroup outOfRangeTypes Out of Range Types
Out of Range Types provide several methods of extrapolating the pattern of key dots
in a track. These patterns are applied to the animation outside the range of all 
keys in the track. \n\n
\image html ort.gif "Out of Range Types"
*/
//@{
#define ORT_CONSTANT       1	//!< Tracks values before or after the range of keys remains constant.
#define ORT_CYCLE          2	//!< Causes the key pattern to repeat cyclically.
#define ORT_LOOP           3	//!< The same as ORT_CYCLE with continuity.
/*! This reverses the range of keys values to cause the pattern to oscillate. */
#define ORT_OSCILLATE      4	//!< Referred to as "Ping-Pong" in the 3ds Max user interface.
#define ORT_LINEAR         5	//!< Takes the slope at the end key in the range and extrapolate with that slope.
/*! This only is used when mapping time to time. The slope will be set to one 
(a 45 degree diagonal starting at the end of the key range). */
#define ORT_IDENTITY       6	//!< 3ds Max will only set this ORT for Ease Curves.
#define ORT_RELATIVE_REPEAT 7	//!< Causes the key pattern to repeat with the first key taking off where the last key left off.
//@}

//keh set key define 
#define KEY_MODE_NO_BUFFER    1

/*! \sa Class Control, Class Matrix3, Class Point3, Class Interval, Class Quat, Class ScaleValue.
\remarks This structure is available in release 4.0 and later only. \n\n
This structure is for collecting the return results of Control::GetLocalTMComponents.
Position, Rotation, or Scale, controllers will put results at the respective
component when the corresponding pointer is not NULL.
*/
struct TMComponentsArg: public MaxHeapOperators {
  /*! \remarks Constructor  */
  TMComponentsArg():position(0),rotation(0),scale(0),rotRep(kUnknown) {}
  /*! \remarks Constructor  */
  TMComponentsArg(Point3* pos, Interval* posInv, float* rot, Interval* rotInv,
			ScaleValue* scl, Interval* sclInv)
   : position(pos),posValidity(posInv),rotation(rot),rotValidity(rotInv)
   , scale(scl),sclValidity(sclInv) {}
  enum RotationRep {
   // kXYZ should equals EULERTYPE_XYZ, which is 0.(c.f. euler.h)
   kXYZ,
   kXZY,
   kYZX,
   kYXZ,
   kZXY,
   kZYX,
   kXYX,
   kYZY,
   kZXZ,
   kQuat,
   kUnknown
  };
  /*! If not NULL this is the position. */
  Point3*      position;
  /*! If not NULL this points to the validity interval for the position. */
  Interval*    posValidity;
  /*! If not NULL this is the rotation and should be a float[4]. */
  float*    rotation;
  /*! If not NULL this points to the validity interval for the rotation. */
  Interval*    rotValidity;
  /*! The rotation representation. This defines what the 4
  numbers in the rotation array mean. One of the following enum values: \n
  <b>kXYZ</b> - Same as EULERTYPE_XYZ \n
  <b>kXZY</b> - Same as EULERTYPE_XZY \n
  <b>kYZX</b> - Same as EULERTYPE_YZX \n
  <b>kYXZ</b> - Same as EULERTYPE_YXZ \n
  <b>kZXY</b> - Same as EULERTYPE_ZXY \n
  <b>kZYX</b> - Same as EULERTYPE_ZYX \n
  <b>kXYX</b> - Same as EULERTYPE_XYX \n
  <b>kYZY</b> - Same as EULERTYPE_YZY \n
  <b>kZXZ</b> - Same as EULERTYPE_ZXZ \n
  <b>kQuat</b> - A quaternion representation. \n
  <b>kUnknown</b> - An unknown representation.
  */
  RotationRep  rotRep;
  /*! If non-NULL this is the ScaleValue. */
  ScaleValue*  scale;
  /*! The validity interval for the ScaleValue. */
  Interval*    sclValidity;
};

// An object of this class represents a Matrix3. However, its value can be
// obtained only by invoking the operator(). Derived classes may override
// this operator to delay its computation until operator() is called.
//
/*! \sa  Class Matrix3, Structure TMComponentsArg.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
An object of this class represents a Matrix3. However, its value can be
obtained only by invoking the operator(). Derived classes may override this
operator to delay its computation until operator() is called.
\par Data Members:
protected:\n\n
<b>Matrix3 mat;</b>\n\n
The matrix itself.  */
class Matrix3Indirect: public MaxHeapOperators {
public:
  /*! \remarks Constructor.
  \par Default Implementation:
  <b>{}</b> */
  Matrix3Indirect(){}
  /*! \remarks Constructor. This matrix is initialized from the matrix passed.
  \par Default Implementation:
  <b>{}</b> */
  Matrix3Indirect(const Matrix3& m):mat(m){}
  /*! \remarks Destructor.
  \par Default Implementation:
  <b>{}</b> */
  virtual ~Matrix3Indirect(){}
  /*! \remarks Assignment operator.
  \par Default Implementation:
  <b>{ return mat; }</b> */
  virtual const Matrix3& operator()() const { return mat; }
  /*! \remarks Set the matrix to the specified matrix.
  \par Parameters:
  <b>const Matrix3\& m</b>\n\n
  The matrix to set.
  \par Default Implementation:
  <b>{ mat = m; }</b> */
  virtual void Set(const Matrix3& m) { mat = m; }
  /*! \remarks Clone the matrix.
  \return  A pointer to a new clone of the matrix.
  \par Default Implementation:
  <b>{return new Matrix3Indirect(mat);}</b> */
  CoreExport virtual Matrix3Indirect* Clone() const;
  /*! \remarks Pre-translate the matrix by the specified coordinate.
  \par Parameters:
  <b>const Point3\& p</b>\n\n
  The coordinate to pre-translate by.
  \par Default Implementation:
  <b>{ mat.PreTranslate(p);}</b> */
  virtual void PreTranslate(const Point3& p){ mat.PreTranslate(p);}
  /*! \remarks Pre-rotate the X axis by the specified amount.
  \par Parameters:
  <b>float x</b>\n\n
  The amount of rotation.
  \par Default Implementation:
  <b>{ mat.PreRotateX(x); }</b> */
  virtual void PreRotateX(float x){ mat.PreRotateX(x); }
  /*! \remarks Pre-rotate the Y axis by the specified amount.
  \par Parameters:
  <b>float y</b>\n\n
  The amount of rotation.
  \par Default Implementation:
  <b>{ mat.PreRotateY(y); }</b> */
  virtual void PreRotateY(float y){ mat.PreRotateY(y); }
  /*! \remarks Pre-rotate the Z axis by the specified amount.
  \par Parameters:
  <b>float z</b>\n\n
  The amount of rotation.
  \par Default Implementation:
  <b>{ mat.PreRotateZ(z); }</b> */
  virtual void PreRotateZ(float z){ mat.PreRotateZ(z); }
  /*! \remarks Pre-rotate the matrix by the specified quaternion.
  \par Parameters:
  <b>const Quat\& q</b>\n\n
  The quaternion to pre-rotate by.
  \par Default Implementation:
  <b>{PreRotateMatrix(mat,q);}</b> */
  virtual void PreRotate(const Quat& q){PreRotateMatrix(mat,q);}
protected:
  Matrix3   mat;
};

class DelayedMatrix3 : public Matrix3Indirect {
public:
   typedef Matrix3Indirect BaseClass;
   struct DelayedOp : public MaxHeapOperators {
	  enum OpCode {
		 kPreTrans,
		 kPreRotateX,
		 kPreRotateY,
		 kPreRotateZ,
		 kPreRotate
	  };
	  OpCode code;
	  Quat   arg;
	  DelayedOp() {}
	  DelayedOp(const Point3& p) : code(kPreTrans), arg(p.x, p.y, p.z, 0.0f) {}
	  DelayedOp(const Quat& q) : code(kPreRotate), arg(q) {}
	  DelayedOp(float x) : code(kPreRotateX), arg(x, 0.0f, 0.0f, 0.0f) {}
	  DelayedOp(int, float y)
		 : code(kPreRotateY), arg(0.0f, y, 0.0f, 0.0f) {}
	  DelayedOp(int, int, float z)
		 : code(kPreRotateZ), arg(0.0f, 0.0f, z, 0.0f) {}
   };
   struct OpQueue : public Tab<DelayedOp> {
	  typedef Tab<DelayedOp> BaseClass;
	  int head;
	  OpQueue() : BaseClass(), head(0) {}
	  void Clear() { ZeroCount(); head = 0; }
	  int QCount() const { return BaseClass::Count() - head; }
	  DelayedOp& Shift() { return BaseClass::operator[](head++); }
	  void Push(DelayedOp& op) { Append(1, &op, 4); }
   };

   DelayedMatrix3::DelayedMatrix3()
	  : Matrix3Indirect()
	  , mMatInitialized(false)
	  , mOpQueue()
	  {}
   DelayedMatrix3::DelayedMatrix3(const DelayedMatrix3& src)
	  : Matrix3Indirect(src.mat)
	  , mMatInitialized(src.mMatInitialized) {
	  mOpQueue = src.mOpQueue; }

   void EvalMat() {
	  if (!mMatInitialized) {
		 InitializeMat();
		 mMatInitialized = true;
	  }
	  while (mOpQueue.QCount() > 0) {
		 DelayedOp& op = mOpQueue.Shift();
		 switch (op.code) {
		 case DelayedOp::kPreTrans:
			mat.PreTranslate(op.arg.Vector());
			break;
		 case DelayedOp::kPreRotateX:
			mat.PreRotateX(op.arg.x);
			break;
		 case DelayedOp::kPreRotateY:
			mat.PreRotateY(op.arg.y);
			break;
		 case DelayedOp::kPreRotateZ:
			mat.PreRotateZ(op.arg.z);
			break;
		 case DelayedOp::kPreRotate:
			PreRotateMatrix(mat, op.arg);
			break;
		 }
	  }
	  return; }
   void EvalMat() const { const_cast<DelayedMatrix3*>(this)->EvalMat(); }
   size_t PendingOps() const { return mOpQueue.QCount(); }
   virtual void InitializeMat() {
	  mat.IdentityMatrix();
	  mMatInitialized = true; };

   // Methods of Matrix3Indirect:
   void Set(const Matrix3& m) {
	  mat = m;
	  mMatInitialized = true;
	  mOpQueue.Clear(); }
   CoreExport Matrix3Indirect* Clone() const;
   const Matrix3& operator()() const { EvalMat(); return mat; }
   void PreTranslate(const Point3& p) { DelayedOp op(p); mOpQueue.Push(op); }
   void PreRotateX(float x){ DelayedOp op(x); mOpQueue.Push(op); }
   void PreRotateY(float y){ DelayedOp op(0, y); mOpQueue.Push(op); }
   void PreRotateZ(float z){ DelayedOp op(0, 0, z); mOpQueue.Push(op); }
   void PreRotate(const Quat& q){ DelayedOp op(q); mOpQueue.Push(op); }

private:
   mutable bool mMatInitialized;
   mutable OpQueue mOpQueue;
};

class DelayedNodeMat : public DelayedMatrix3 {
public:
   DelayedNodeMat(INode& n, TimeValue t0)
	  : DelayedMatrix3()
	  , node(n)
	  , t(t0)
	  {}
   DelayedNodeMat(const DelayedNodeMat& src)
	  : DelayedMatrix3(src)
	  , node(src.node)
	  , t(src.t)
	  {}
   // of Matrix3Indirect:
   void Set(const Matrix3&) {}
   CoreExport Matrix3Indirect* Clone() const;

   // of DelayedMatrix3:
   CoreExport void InitializeMat();
private:
	DelayedNodeMat& operator=(const DelayedNodeMat& );
   TimeValue t;
   INode&   node;
};

/*---------------------------------------------------------------------*/

// A list of ease curves.
/*! \sa  Class ReferenceTarget, Class Control.\n\n
\par Description:
This class represents a list of ease curves.\n\n
The macro used to access this class is defined as follows:\n\n
<b>#define
GetEaseListInterface(anim) ((EaseCurveList*)anim-\>GetInterface(I_EASELIST))</b>\n\n
This may be used to access the methods of this class as follows:\n\n
		\code
		EaseCurveList *el = GetEaseListInterface(client);
		if (el) {
			int num = el->NumEaseCurves();
			// ...
		}
		\endcode
All methods of this class are implemented by the system.  */

#pragma warning(push)
#pragma warning(disable:4100 4239)

class EaseCurveList : public ReferenceTarget {
	  friend class AddEaseRestore;
	  friend class DeleteEaseRestore;

   private:
	  Tab<Control*> eases;
	  
   public:
	  /*! \remarks Constructor. */
	  EaseCurveList() {OpenTreeEntry(TRACKVIEW_ANIM, ALL_TRACK_VIEWS);}
	  /*! \remarks Destructor. All the references are deleted from this class.
	  */
	  CoreExport ~EaseCurveList();

	  /*! \remarks Returns a <b>TimeValue</b> that reflects the
	  <b>TimeValue</b> passed modified by each of the enabled ease curves in
	  the list.
	  \par Parameters:
	  <b>TimeValue t</b>\n\n
	  The base time which is eased by the curves.\n\n
	  <b>Interval \&valid</b>\n\n
	  The validity interval which is updated by each of the ease curves in the
	  list. */
	  CoreExport TimeValue ApplyEase(TimeValue t,Interval& valid);
	  /*! \remarks Adds the specified ease curve to the end of the ease curve
	  list.
	  \par Parameters:
	  <b>Control *cont</b>\n\n
	  Points to the ease curve to append. */
	  CoreExport void AppendEaseCurve(Control *cont);
	  /*! \remarks Deletes the 'i-th' ease curve in the list.
	  \par Parameters:
	  <b>int i</b>\n\n
	  The index of the ease curve to delete. */
	  CoreExport void DeleteEaseCurve(int i);
	  /*! \remarks Disables the 'i-th' ease curve in the list.
	  \par Parameters:
	  <b>int i</b>\n\n
	  The index of the ease curve to disable. */
	  CoreExport void DisableEaseCurve(int i);
	  /*! \remarks Enables the 'i-th' ease curve in the list.
	  \par Parameters:
	  <b>int i</b>\n\n
	  The index of the ease curve to enable. */
	  CoreExport void EnableEaseCurve(int i);
	  /*! \remarks Returns TRUE if the 'i-th' ease curve is enabled; otherwise
	  FALSE.
	  \par Parameters:
	  <b>int i</b>\n\n
	  The index of the ease curve to check. */
	  CoreExport BOOL IsEaseEnabled(int i);
	  /*! \remarks Returns the number of ease curves in the list. */
	  int NumEaseCurves() {return eases.Count();}

	  // Animatable
	  void GetClassName(MSTR& s) { s= MSTR(_M("EaseCurve")); }  
	  Class_ID ClassID() { return Class_ID(EASE_LIST_CLASS_ID,0); }
	  SClass_ID SuperClassID() { return EASE_LIST_CLASS_ID; }     
	  CoreExport int NumSubs();
	  CoreExport Animatable* SubAnim(int i);
	  CoreExport MSTR SubAnimName(int i);
	  int SubNumToRefNum(int subNum) {return subNum;}
	  BOOL BypassTreeView() { return TRUE; }
	  CoreExport void DeleteThis();
	  ParamDimension* GetParamDimension(int i) {return stdTimeDim;}
	  CoreExport BOOL AssignController(Animatable *control,int subAnim);

		using ReferenceTarget::GetInterface;
	  CoreExport void* GetInterface(ULONG id);

	  CoreExport IOResult Save(ISave *isave);
	  CoreExport IOResult Load(ILoad *iload);
	  
	  // Reference
	  CoreExport int NumRefs();
	  CoreExport RefTargetHandle GetReference(int i);
protected:
	  CoreExport virtual void SetReference(int i, RefTargetHandle rtarg);
public:
	  CoreExport RefTargetHandle Clone(RemapDir &remap);
	  CoreExport RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
			PartID& partID,  RefMessage message);     
   };

class EaseCurveAnimProp : public AnimProperty {
   public:
	  EaseCurveList *el;
	  EaseCurveAnimProp() { el=NULL; }
	  DWORD ID() {return PROPID_EASELIST;}
   };

#define GetEaseListInterface(anim)  ((EaseCurveList*)anim->GetInterface(I_EASELIST))

/*---------------------------------------------------------------------*/
// A list of multiplier curves.
/*! \sa  Class ReferenceTarget, Class Control.\n\n
\par Description:
This class is a list of multiplier curves.\n\n
The macro used to access this class is defined as follows:\n\n
<b>#define
GetMultListInterface(anim) ((MultCurveList*)anim-\>GetInterface(I_MULTLIST))</b>\n\n
This may be used to access the methods of this class as follows:\n\n
		\code
		MultCurveList *ml = GetMultListInterface(client);
		if (ml) {
		   int num = ml->NumMultCurves();
		   // ...
		}
		\endcode
All methods of this class are implemented by the system.  */
class MultCurveList : public ReferenceTarget {
	  friend class AddMultRestore;
	  friend class DeleteMultRestore;
   private:
	  Tab<Control*> mults;
	  
   public:
	  /*! \remarks Constructor. */
	  MultCurveList() {OpenTreeEntry(TRACKVIEW_ANIM, ALL_TRACK_VIEWS);}
	  /*! \remarks Destructor. All referrences are removed from this class. */
	  CoreExport ~MultCurveList();

	  /*! \remarks This method starts with a value of <b>1.0f</b>, mutiplies
	  it by each enabled mutiplier curve value in the list, and returns the
	  resulting value.
	  \par Parameters:
	  <b>TimeValue t</b>\n\n
	  The time at which to get the multipler values.\n\n
	  <b>Interval \&valid</b>\n\n
	  The interval that is adjusted to reflect the validity of all the
	  multipler curve controllers validity. */
	  CoreExport float GetMultVal(TimeValue t,Interval& valid);
	  /*! \remarks Adds the specified multiplier curve to the end of the
	  multiplier curve list.
	  \par Parameters:
	  <b>Control *cont</b>\n\n
	  Points to the multiplier curve to append. */
	  CoreExport void AppendMultCurve(Control *cont);
	  /*! \remarks Deletes the 'i-th' multiplier curve.
	  \par Parameters:
	  <b>int i</b>\n\n
	  The index of the multiplier curve to delete. */
	  CoreExport void DeleteMultCurve(int i);
	  /*! \remarks Disables the 'i-th' multiplier curve.
	  \par Parameters:
	  <b>int i</b>\n\n
	  The index of the multiplier curve to disable. */
	  CoreExport void DisableMultCurve(int i);
	  /*! \remarks Enables the 'i-th' multiplier curve.
	  \par Parameters:
	  <b>int i</b>\n\n
	  The index of the multiplier curve to enable. */
	  CoreExport void EnableMultCurve(int i);
	  /*! \remarks Returns TRUE if the 'i-th' multiplier curve is enabled;
	  otherwise FALSE.
	  \par Parameters:
	  <b>int i</b>\n\n
	  The index of the multiplier curve to check. */
	  CoreExport BOOL IsMultEnabled(int i);
	  /*! \remarks Returns the number of multiplier curves in the list. */
	  int NumMultCurves() {return mults.Count();}

	  // Animatable
	  void GetClassName(MSTR& s) { s= MSTR(_M("MultCurve")); }  
	  Class_ID ClassID() { return Class_ID(MULT_LIST_CLASS_ID,0); }
	  SClass_ID SuperClassID() { return MULT_LIST_CLASS_ID; }     
	  CoreExport int NumSubs();
	  CoreExport Animatable* SubAnim(int i);
	  CoreExport MSTR SubAnimName(int i);
	  int SubNumToRefNum(int subNum) {return subNum;}
	  BOOL BypassTreeView() { return TRUE; }
	  CoreExport void DeleteThis();
	  ParamDimension* GetParamDimension(int i) {return stdNormalizedDim;}
	  CoreExport BOOL AssignController(Animatable *control,int subAnim);

		using ReferenceTarget::GetInterface;
	  CoreExport void* GetInterface(ULONG id);

	  CoreExport IOResult Save(ISave *isave);
	  CoreExport IOResult Load(ILoad *iload);
	  
	  // Reference
	  CoreExport int NumRefs();
	  CoreExport RefTargetHandle GetReference(int i);
protected:
	  CoreExport virtual void SetReference(int i, RefTargetHandle rtarg);
public:
	  CoreExport RefTargetHandle Clone(RemapDir &remap);
	  CoreExport RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
			PartID& partID,  RefMessage message);     
   };

class MultCurveAnimProp : public AnimProperty {
   public:
	  MultCurveList *ml;
	  MultCurveAnimProp() { ml=NULL; }
	  DWORD ID() {return PROPID_MULTLIST;}
   };

#define GetMultListInterface(anim)  ((MultCurveList*)anim->GetInterface(I_MULTLIST))

/*---------------------------------------------------------------------*/


//
// For hit testing controller apparatus 
//

/*! class CtrlHitRecord
\par Description:
This class provides a data structure used during controller gizmo hit-testing.
All methods are implemented by the system.
\par Data Members:
<b>INode *nodeRef;</b>\n\n
This identifies the node the user has clicked on.\n\n
<b>DWORD distance;</b>\n\n
The 'distance' of the hit. What the distance actually represents depends on the
rendering level of the viewport. For wireframe modes, it refers to the distance
in the screen XY plane from the mouse to the sub-object component. In a shaded
mode, it refers to the Z depth of the sub-object component. In both cases,
smaller values indicate that the sub-object component is 'closer' to the mouse
cursor.\n\n
<b>ulong hitInfo;</b>\n\n
A general unsigned long value. Most controllers will just need this to identity
the sub-object element. The meaning of this value (how it is used to identify
the element) is up to the plug-in.\n\n
<b>DWORD infoExtra;</b>\n\n
If the above <b>hitInfo</b> data member is not sufficient to describe the
sub-object element this data member may be used as well.  */
class CtrlHitRecord: public MaxHeapOperators {
   friend class CtrlHitLog;
   CtrlHitRecord *next;
   public:
	  INode *nodeRef;
	  DWORD distance;
	  ulong hitInfo;
	  DWORD infoExtra;     
	  /*! \remarks Constructor. The data members are initialized as follows:
	  <b>next=NULL; distance=0;</b>\n\n
	  <b> hitInfo=0; nodeRef=NULL;</b> */
	  CtrlHitRecord() {next=NULL; distance=0; hitInfo=0; nodeRef=NULL;}
	  /*! \remarks Constructor. The data members are initialized to the data
	  passed. */
	  CtrlHitRecord(CtrlHitRecord *nxt,INode *nr, DWORD d, ulong inf, DWORD extra) {
		 next=nxt;nodeRef=nr;distance=d;hitInfo=inf;infoExtra=extra;}
	  /*! \remarks Each <b>CtrlHitRecord</b> maintains a pointer to another
	  <b>CtrlHitRecord</b>. This method returns the next hit record. */
	  CtrlHitRecord *Next() {return next;}      
   };                

/*! class CtrlHitLog
\par Description:
This class provides a data structure for keeping a log of hits during
controller gizmo hit-testing. It provides a list of <b>CtrlHitRecords</b> that
may be added to and cleared. A developer may also request the 'closest' hit
record in the list. All methods are implemented by the system.  */
class CtrlHitLog: public MaxHeapOperators {
   CtrlHitRecord *first;
   int hitIndex;
   bool hitIndexReady;        // CAL-07/10/03: hitIndex is ready to be increased.
   public:
	  /*! \remarks Constructor. */
	  CtrlHitLog()  { first = NULL; hitIndex = 0; hitIndexReady = false; }
	  ~CtrlHitLog() { Clear(); }
	  /*! \remarks Clears the list of hits. */
	  CoreExport void Clear();
	  CoreExport void ClearHitIndex(bool ready = false)     { hitIndex = 0; hitIndexReady = ready; }
	  CoreExport void IncrHitIndex()      { if (hitIndexReady) hitIndex++; else hitIndexReady = true; }
	  /*! \remarks Returns the first hit record in the list. */
	  CtrlHitRecord* First() { return first; }
	  /*! \remarks Returns the <b>CtrlHitRecord</b> that was 'closest' to the
	  mouse position when hit testing was performed. */
	  CoreExport CtrlHitRecord* ClosestHit();
	  /*! \remarks This method is called to log a hit. It creates a new
	  <b>CtrlHitRecord</b> object using the data passed and adds it to the hit
	  log.
	  \par Parameters:
	  <b>INode *nr</b>\n\n
	  The node whose gizmo was hit.\n\n
	  <b>DWORD dist</b>\n\n
	  The 'distance' of the hit. What the distance actually represents depends
	  on the rendering level of the viewport. For wireframe modes, it refers to
	  the distance in the screen XY plane from the mouse to the sub-object
	  component. In a shaded mode, it refers to the Z depth of the sub-object
	  component. In both cases, smaller values indicate that the sub-object
	  component is 'closer' to the mouse cursor.\n\n
	  <b>ulong hitInfo;</b>\n\n
	  A general unsigned long value. Most controllers will just need this to
	  identity the sub-object element. The meaning of this value (how it is
	  used to identify the element) is up to the plug-in.\n\n
	  <b>DWORD infoExtra;</b>\n\n
	  If the above <b>hitInfo</b> data member is not sufficient to describe the
	  sub-object element this data member may be used as well. */
	  CoreExport void LogHit(INode *nr,DWORD dist,ulong info,DWORD infoExtra); 
   };


// For enumerating IK paramaters
/*! \sa  Class Control.\n\n
\par Description:
This class is for enumerating IK parameters. This callback is called once for
each parameter a controller has. This callback is implemented by the system and
passed into the method <b>EnumIKParams()</b> of the controller.  */
class IKEnumCallback: public MaxHeapOperators {
   public:
	  /*! \remarks Implemented by the System.\n\n
	  The plug-in calls this method once for each parameter (degree of freedom
	  it has). It passes a pointer to itself and the index of the parameter.
	  \par Parameters:
	  <b>Control *c</b>\n\n
	  The controller itself is passed here.\n\n
	  <b>int index</b>\n\n
	  The index of the parameter. For example a position controller with three
	  degrees of freedom (X, Y, Z) would call this method three times passing
	  it and index of 0, then 1, then 2. */
	  virtual void proc(Control *c, int index)=0;
   };

/*! \sa  Class Control.\n\n
\par Description:
This class provides method that a plug-in calls in its implementation of the
Control method <b>CompDerivs()</b>. All methods of this class are implemented
by the system.  */
class IKDeriv: public MaxHeapOperators {
   public:
	  /*! \remarks This method returns the number of end effectors. There may
	  be multiple end effectors if there is branching in the IK chain. For
	  example if the plug-in is a controller controlling a torso there might be
	  two end effectors - the two feet. The plug-ins implementation of
	  <b>CompDerivs()</b> should loop through each end effector and call
	  <b>DP()</b> and <b>DR()</b> for each end effector. Thus this method tells
	  the plug-in how many times it needs to loop.
	  \return  The number of end effectors */
	  virtual int NumEndEffectors()=0;
	  /*! \remarks If a plug-in needs to know the position of an end effector
	  to calculate its derivative it may call this method to retrieve it. This
	  method is used to return the position of the end effector whose index is
	  passed.
	  \par Parameters:
	  <b>int index</b>\n\n
	  The index of the end effector whose position will be returned. */
	  virtual Point3 EndEffectorPos(int index)=0;
	  /*! \remarks The plug-in calls this method to specify the derivative of
	  the position of the end effector with respect to the parameter whose
	  index is passed.
	  \par Parameters:
	  <b>Point3 dp</b>\n\n
	  The derivative of the position of the end effector with respect to the
	  parameter.\n\n
	  <b>int index</b>\n\n
	  The index of the end effector. */
	  virtual void DP(Point3 dp,int index)=0;
	  /*! \remarks Allows the plug-in to specify the derivative of the orientation in terms
	  of Euler angle of the end effector with respect to the parameter.
	  \par Parameters:
	  <b>Point3 dr</b>\n\n
	  The derivative of the orientation in terms of Euler angles of the end
	  effector with respect to the parameter.\n\n
	  <b>int index</b>\n\n
	  The index of the end effector. */
	  virtual void DR(Point3 dr,int index)=0;
	  /*! \remarks This method is called after a plug-in has called the above
	  methods <b>DP()</b> and <b>DR()</b> for one of its parameters and it
	  needs to call them again for the next parameter. */
	  virtual void NextDOF()=0;
   };

// Flags passed to CompDerivs
#define POSITION_DERIV  (1<<0)
#define ROTATION_DERIV  (1<<1)


// This class is used to store IK parameters that have been
// copied to a clipboard.
/*! \sa  Class Control.\n\n
\par Description:
This class is used to store IK parameters that have been copied to a clipboard.
The plug-in derives a class from this class to store their data and implements
the methods that describe the creator object. The plug-in should also implement
the <b>DeleteThis()</b> method to delete the instance of the class.  */
class IKClipObject: public MaxHeapOperators {
   public:
	  /*! \remarks Destructor. */
	  virtual ~IKClipObject() { }
	  // Identifies the creator of the clip object
	  /*! \remarks Returns the super class ID of the creator of the clip object. */
	  virtual SClass_ID    SuperClassID()=0;
	  /*! \remarks Returns the class ID of the creator of the clip object. */
	  virtual Class_ID  ClassID()=0;
	  
	  /*! \remarks The system calls this method to delete the clip object after it has been
	  used. */
	  virtual void DeleteThis()=0;
   };

// Values for 'which' pasted to Copy/PasteIKParams
#define COPYPASTE_IKPOS    1
#define COPYPASTE_IKROT    2

// Passed to InitIKJoints() which is called when importing
// R4 3DS files that have IK joint data.
/*! \sa  Class Control.\n\n
\par Description:
This class is passed to <b>Control::InitIKJoints()</b> which is called when
importing R4 3DS files that have IK joint data.
\par Data Members:
<b>BOOL active[3];</b>\n\n
The joint active settings. Index 0=X, 1=Y, 2=Z.\n\n
<b>BOOL limit[3];</b>\n\n
The joint limit settings. Index 0=X, 1=Y, 2=Z.\n\n
<b>BOOL ease[3];</b>\n\n
The joint ease settings. Index 0=X, 1=Y, 2=Z.\n\n
<b>Point3 min, max, damping;</b>\n\n
The joint min, max and damping settings. */
class InitJointData: public MaxHeapOperators {
   public:
	  BOOL active[3];
	  BOOL limit[3];
	  BOOL ease[3];
	  Point3 min, max, damping;
   };

// New for R4: include preferred angle
/*! \sa  Class InitJointData\n\n
\par Description:
This class is passed to <b>Control::InitIKJoints2()</b> which is called when
importing R4 3DS files that have IK joint data. This class contains the added
preferredAngle parameter.
\par Data Members:
<b>Point3 preferredAngle;</b>\n\n
The preferred angle.\n\n
<b>DWORD flags;</b>\n\n
Not used (must be 0), reserved for future expansion. */
class InitJointData2 : public InitJointData {
   public:     
	  Point3 preferredAngle;
	  DWORD flags; // not used (must be 0) - for future expansion
	  /*! \remarks Constructor. */
	  InitJointData2() {flags=0;}
   };

// The following member been added
// in 3ds max 4.2.  If your plugin utilizes this new
// mechanism, be sure that your clients are aware that they
// must run your plugin with 3ds max version 4.2 or higher.
//
// Extend to include Spring parameters that are missing in InitJointData
// and InitJointData2. It is designed for arguments of type InitJointData2.
// A pointer of InitJointData2 can be tested and downcast to InitJointData3
// via the following two inlines: IsInitJointData3(InitJointData2*) and
// DowncastToJointData3(InitJointData2*).
// 
const DWORD bJointData3 = (1 << 0);
/*!  \n\n
class InitJointData3 : public InitJointData2\n\n

\par Description:
An extention to <b>Class InitJointData2</b> to support various Spring
parameters that are missing in InitJointData and InitJointData2. It is designed
for arguments of type InitJointData2.\n\n

\par Data Members:
<b></b>\n\n
<b>bool springOn[3];</b>\n\n
The Spring On settings. Index 0=X, 1=Y, 2=Z.\n\n
<b>Point3 spring;</b>\n\n
The Spring value\n\n
<b>Point3 springTension;</b>\n\n
The Spring Tension value\n\n
  \remarks A pointer of InitJointData2 can be tested and downcast to
InitJointData3 via the following two inlines: IsInitJointData3(InitJointData2*)
and DowncastToJointData3(InitJointData2*).\n\n
<b>inline bool IsInitJointData3(InitJointData2* jd)</b>\n\n
<b>{</b>\n\n
<b>return (jd-\>flags \& bJointData3);</b>\n\n
<b>}</b>\n\n
<b>inline InitJointData3* DowncastToJointData3(InitJointData2* jd)</b>\n\n
<b>{</b>\n\n
<b>return IsInitJointData3(jd) ? (InitJointData3*)jd : NULL;</b>\n\n
<b>}</b> */
class InitJointData3 : public InitJointData2 {
   public:
   InitJointData3() : InitJointData2() {
   active[0] = active[1] = active[2] = FALSE;
   limit[0] = limit[1] = limit[2] = FALSE;
   ease[0] = ease[1] = ease[2] = FALSE;
   min.Set(0.0f, 0.0f, 0.0f);
   max.Set(0.0f, 0.0f, 0.0f);
   damping.Set(0.0f, 0.0f, 0.0f);
   flags |= bJointData3;
   preferredAngle.Set(0.0f, 0.0f, 0.0f);
   springOn[0] = springOn[1] = springOn[2] = false;
   spring.Set(0.0f, 0.0f, 0.0f);
   // from interpik.h:
#define DEF_SPRINGTENS  (0.02f)
   springTension.Set(DEF_SPRINGTENS, DEF_SPRINGTENS, DEF_SPRINGTENS);
#undef DEF_SPRINGTENS
	}
  bool  springOn[3];
  Point3 spring;
  Point3 springTension;
};

inline bool IsInitJointData3(InitJointData2* jd)
{
  return (jd->flags & bJointData3);
}

inline InitJointData3* DowncastToJointData3(InitJointData2* jd)
{
  return IsInitJointData3(jd) ? (InitJointData3*)jd : NULL;
}
// End of 3ds max 4.2 Extension

// This structure is passed to GetDOFParams().
// Controllers that support IK can provide info about their DOFs
// so that bones can display this information.
// The first 3 DOFs are assumed to be position
// and the next 3 are assumed to be rotation
/*! \sa  Class Control.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This structure is passed to the method <b>Control::GetDOFParams()</b>.
Controllers that support IK can provide information about their Degree Of
Freedoms (DOFs) so that bones can display this information. The first 3 DOFs
are assumed to be position and the next 3 are assumed to be rotation
\par Data Members:
<b>BOOL display[6];</b>\n\n
Indicates if this DOF should be displayed.\n\n
<b>Point3 axis[6];</b>\n\n
Specifies the DOF axis.\n\n
<b>Point3 pos[6];</b>\n\n
Specifies the base of the axis.\n\n
<b>BOOL limit[6];</b>\n\n
Indicates if the joint is limited at all.\n\n
<b>float min[6];</b>\n\n
Specifies the minimum limit.\n\n
<b>float max[6];</b>\n\n
Specifies the maximum limit.\n\n
<b>float curval[6];</b>\n\n
Specifies the current value of the parameter.\n\n
<b>BOOL sel[6];</b>\n\n
Indicates if the DOF should be highlighted.\n\n
<b>BOOL endEffector;</b>\n\n
Indicates if there is an end effector for this controller.\n\n
<b>Matrix3 eeTM;</b>\n\n
Specifies the world transformation matrix of the end effector (if present). */
class DOFParams: public MaxHeapOperators {
   public:
	  BOOL display[6];     // Should this DOF be displayed?
	  Point3 axis[6];         // DOF axis
	  Point3 pos[6];       // Base of axis
	  BOOL limit[6];          // is joint limited?
	  float min[6];        // min limit
	  float max[6];           // max limit
	  float curval[6];     // Current value of the parameter
	  BOOL sel[6];         // should DOF be highlighted
	  BOOL endEffector;    // is there an end effector for this controller
	  Matrix3 eeTM;        // world TM of the end effector if present
   };


// These two ways values can be retreived or set.
// For get:
//    RELATIVE = Apply
//    ABSOLUTE = Just get the value
// For set:
//    RELATIVE = Add the value to the existing value (i.e Move/Rotate/Scale)
//    ABSOLUTE = Just set the value
enum GetSetMethod {CTRL_RELATIVE,CTRL_ABSOLUTE};


// Control class provides default implementations for load and save which save the ORT type in these chunks:
#define CONTROLBASE_CHUNK     0x8499
#define INORT_CHUNK           0x3000
#define OUTORT_CHUNK       0x3001
#define CONT_DISABLED_CHUNK      0x3002
#define CONT_FLAGS_CHUNK      0x3003

// Inheritance flags.
#define INHERIT_POS_X   (1<<0)
#define INHERIT_POS_Y   (1<<1)
#define INHERIT_POS_Z   (1<<2)
#define INHERIT_ROT_X   (1<<3)
#define INHERIT_ROT_Y   (1<<4)
#define INHERIT_ROT_Z   (1<<5)
#define INHERIT_SCL_X   (1<<6)
#define INHERIT_SCL_Y   (1<<7)
#define INHERIT_SCL_Z   (1<<8)
#define INHERIT_ALL     511

/*! \sa  Class ReferenceTarget, Class IKeyControl, Class IKDeriv, Class IKEnumCallback, Class StdControl, <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_controller_related_methods.html">List of Additional Controller Related Functions</a>, 
Class AngAxis,  Class Quat, Class Interval, Class Matrix3, Class Point3, Class ScaleValue, Class JointParams, Class SetXFormPacket.\n\n
\par Description:
Control is the class from which you may derived controller objects. Controllers
are the objects in 3ds Max that control animation. Controllers come in
different types based on the type of data they control. For example, Transform
controllers control the 4x3 matrices used to define the position of nodes in
the scene while float controllers control simple floating point values.\n\n
Note: Many controller plug-ins may be able to subclass from StdControl rather
than Control. This simplifies the developers job. StdControl handles the
processing of Out of Range Types, Ease Curves, and Multiplier Curves. See
Class StdControl for more information.\n\n
Plug-In Information:\n\n
Class Defined In CONTROL.H\n\n
Super Class ID CTRL_FLOAT_CLASS_ID - Used by float controllers.\n\n
 CTRL_POINT3_CLASS_ID - Used by Point3 controllers.\n\n
 CTRL_MATRIX3_CLASS_ID - Used by Matrix3 controllers.\n\n
 CTRL_POSITION_CLASS_ID - Used by position controllers.\n\n
 CTRL_ROTATION_CLASS_ID - Used by rotation controllers.\n\n
 CTRL_SCALE_CLASS_ID - Used by scale controllers.\n\n
 CTRL_MORPH_CLASS_ID - Used by morph controllers.\n\n
Standard File Name Extension DLC\n\n
Extra Include File Needed None
\par Defines:
#define CONT_FLAGS_CHUNK 0x3003<br>
\par Method Groups:
See <a href="class_control_groups.html">Method Groups for Class Control</a>.
*/
class Control : public ReferenceTarget {
   public:
	  // aszabo|MAr.25.02|Prevents GetInterface(ULONG id) from hiding GetInterface(Interface_ID)
	  using ReferenceTarget::GetInterface;

	  CoreExport Control();
	  virtual ~Control() {};

	  /*! \remarks When a controller is assigned to a track in the track view, the new
	  controller is plugged into the parameter and this method is called on the
	  new controller. A pointer to the old controller is passed in to this
	  method. The new controller can attempt to copy any data that it can from
	  the old controller. At the very least it should initialize itself to the
	  value of the old controller at frame 0.
	  \param from A pointer to the previous controller. */
	  virtual void Copy(Control *from)=0;
	  /*! \remarks This method, along with <b>RestoreValue()</b>, comprise an "inner" hold
	  and restore mechanism (see above). When the controller's
	  <b>SetValue()</b> method is called, if the <b>commit</b> parameter is
	  nonzero, then the controller should save the value of the controller at
	  the current time into its cache and also 'commit' the value. For example,
	  this stores a key in the case of a keyframe controller. If the set value
	  was not committed then <b>RestoreValue()</b> may be called to restore the
	  previous value.
	  \param t Specifies the time to save the value.
	  \par Default Implementation:
	  <b>{}</b> */
	  virtual void CommitValue(TimeValue t) {}
	  /*! \remarks This method is the other half of the "inner" hold and restore mechanism.
	  This method is called to restore a previously saved value. This method
	  restores the current cache value to the value that was set before
	  <b>SetValue()</b> was last called. They way the standard 3ds Max
	  controllers handle this is as follows: When <b>SetValue()</b> is called a
	  temporary hold mechanism (<b>TempStore</b> define in <b>CONTROL.H</b>) is
	  used to hold the current value. Then the new value is set. If
	  <b>RestoreValue()</b> is later called then it restores the current value
	  from the temporary storage. Note that in addition to restoring from the
	  <b>TempStore</b>, another way a controller may restore the current value
	  is to re-interpolate the keys.
	  \param t Specifies the time to restore the value.
	  \par Default Implementation:
	  <b>{}</b> */
	  virtual void RestoreValue(TimeValue t) {}
	  /*! \remarks This method retrieves a lookat controller's target.
	  \return  The lookat controllers target node.
	  \par Default Implementation:
	  <b>{ return NULL; }</b> */
	  virtual INode* GetTarget() { return NULL; } 
	  /*! \remarks This method stores a lookat controller's target.
	  \param targ The target node to store.
	  \return  One of the following values:\n\n
	  <b>REF_SUCCEED</b>\n\n
	  Indicates the target was set.\n\n
	  <b>REF_FAIL</b>\n\n
	  Indicates the target was not set.
	  \par Default Implementation:
	  <b>{return REF_SUCCEED;}</b> */
	  virtual RefResult SetTarget(INode *targ) {return REF_SUCCEED;}

	  // Implemented by transform controllers that have position controller
	  // that can be edited in the trajectory branch
	  /*! \remarks Implemented by transform controllers that have a position controller that
	  can be edited in the motion branch. This method returns a pointer to the
	  position controller of the transform controller.
	  \par Default Implementation:
	  <b>{return NULL;}</b> */
	  virtual Control *GetPositionController() {return NULL;}
	  /*! \remarks Implemented by transform controllers that have a rotation controller that
	  can be edited in the motion branch. This method returns a pointer to the
	  rotation controller of the transform controller.
	  \return
	  \par Default Implementation:
	  <b>{return NULL;}</b> */
	  virtual Control *GetRotationController() {return NULL;}
	  /*! \remarks Implemented by transform controllers that have a scale controller that
	  can be edited in the motion branch. This method returns the a pointer to
	  the scale controller of the transform controller.
	  \return
	  \par Default Implementation:
	  <b>{return NULL;}</b> */
	  virtual Control *GetScaleController() {return NULL;}
	  /*! \remarks This method assigns a new position controller. Plug-Ins don't need to be
	  concerned with freeing the previous controller if this method is called.
	  Any previous controller assigned will be deleted by 3ds Max if it is not
	  used elsewhere in the scene.
	  \par Default Implementation:
	  <b>{return FALSE;}</b> */
	  virtual BOOL SetPositionController(Control *c) {return FALSE;}
	  /*! \remarks This method assigns a new rotation controller. Plug-Ins don't need to be
	  concerned with freeing the previous controller if this method is called.
	  Any previous controller assigned will be deleted by 3ds Max if it is not
	  used elsewhere in the scene.
	  \par Default Implementation:
	  <b>{return FALSE;}</b> */
	  virtual BOOL SetRotationController(Control *c) {return FALSE;}
	  /*! \remarks This method assigns a new scale controller. Plug-Ins don't need to be
	  concerned with freeing the previous controller if this method is called.
	  Any previous controller assigned will be deleted by 3ds Max if it is not
	  used elsewhere in the scene.
	  \par Default Implementation:
	  <b>{return FALSE;}</b> */
	  virtual BOOL SetScaleController(Control *c) {return FALSE;}

	  // If a controller has an 'X', 'Y', 'Z', or 'W' controller, it can implement
	  // these methods so that its sub controllers can respect track view filters
	  /*! \remarks	  Returns a pointer to the 'X' sub-controller of this controller. If a
	  controller has an 'X', 'Y', or 'Z' controller, it can implement this set
	  of methods so that its sub-controllers can respect track view filters.
	  Examples of controllers that have XYZ sub-controllers are the Euler angle
	  controller or the Position XYZ controller.
	  \par Default Implementation:
	  <b>{return NULL;}</b> */
	  virtual Control *GetXController() {return NULL;}
	  /*! \remarks	  Returns a pointer to the 'Y' sub-controller of this controller. If a
	  controller has an 'X', 'Y', or 'Z' controller, it can implement this set
	  of methods so that its sub-controllers can respect track view filters.
	  Examples of controllers that have XYZ sub-controllers are the Euler angle
	  controller or the Position XYZ controller.
	  \par Default Implementation:
	  <b>{return NULL;}</b> */
	  virtual Control *GetYController() {return NULL;}
	  /*! \remarks	  Returns a pointer to the 'Z' sub-controller of this controller. If a
	  controller has an 'X', 'Y', or 'Z' controller, it can implement this set
	  of methods so that its sub-controllers can respect track view filters.
	  Examples of controllers that have XYZ sub-controllers are the Euler angle
	  controller or the Position XYZ controller.
	  \par Default Implementation:
	  <b>{return NULL;}</b> */
	  virtual Control *GetZController() {return NULL;}
	  virtual Control *GetWController() {return NULL;}

	  // Implemented by look at controllers that have a float valued roll
	  // controller so that the roll can be edited via the transform type-in
	  /*! \remarks Implemented by lookat controllers that have a float valued roll
	  controller so that the roll can be edited via the transform type-in. This
	  method returns a pointer to the roll controller of the lookat controller.
	  \par Default Implementation:
	  <b>{return NULL;}</b> */
	  virtual Control *GetRollController() {return NULL;}
	  /*! \remarks This method assigns a new roll controller. Plug-Ins don't need to be
	  concerned with freeing the previous controller if this method is called.
	  Any previous controller assigned will be deleted by 3ds Max if it is not
	  used elsewhere in the scene.
	  \par Default Implementation:
	  <b>{return FALSE;}</b> */
	  virtual BOOL SetRollController(Control *c) {return FALSE;}

	  // Implemented by any Point3/Point4 controller that wishes to indicate that it is intended
	  // to control floating point RGB color values
	  /*! \remarks Implemented by any Point3 controller that wishes to indicate that it is
	  intended to control floating point RGB color values. Returns TRUE to
	  indicate that it controls float color values; otherwise FALSE.
	  \par Default Implementation:
	  <b>{return FALSE;}</b> */
	  virtual BOOL IsColorController() {return FALSE;}

	  // Implemented by TM controllers that support 
	  // filtering out inheritance
	  /*! \remarks This method should be implemented by TM controllers that support
	  filtering out inheritance. It returns the state of the transform
	  inheritance flags. These are the values that show up in the Hierarchy
	  branch, under the Link Info section, in the Inheritance rollup.
	  \return  One or more of the following values:\n\n
	  Note: Each bit is used to represent a single inheritance. If the bit is
	  <b>CLEAR (OFF)</b> it means inherit (checked in the 3ds Max UI). If the
	  bit is <b>SET</b> it means <b>DON'T</b> inherit (unchecked in the 3ds Max
	  UI).\n\n
	  <b>INHERIT_POS_X</b>\n\n
	  <b>INHERIT_POS_Y</b>\n\n
	  <b>INHERIT_POS_Z</b>\n\n
	  <b>INHERIT_ROT_X</b>\n\n
	  <b>INHERIT_ROT_Y</b>\n\n
	  <b>INHERIT_ROT_Z</b>\n\n
	  <b>INHERIT_SCL_X</b>\n\n
	  <b>INHERIT_SCL_Y</b>\n\n
	  <b>INHERIT_SCL_Z</b>\n\n
	  <b>INHERIT_ALL</b>
	  \par Default Implementation:
	  <b>{return INHERIT_ALL;}</b> */
	  virtual DWORD GetInheritanceFlags() {return INHERIT_ALL;}
	  /*! \remarks This method should be implemented by TM controllers that support
	  filtering out inheritance.\n\n
	  Note: Each bit is used to represent a single inheritance. This method
	  expects the bits of the flags passed to be <b>CLEAR (OFF)</b> to mean
	  DON'T inherit (unchecked in the 3ds Max UI). If they are <b>SET</b> it
	  means inherit (checked in the 3ds Max UI).
	  \param f The inheritance flags. One or more of the following values:\n\n
	  <b>INHERIT_POS_X</b>\n
	  <b>INHERIT_POS_Y</b>\n
	  <b>INHERIT_POS_Z</b>\n
	  <b>INHERIT_ROT_X</b>\n
	  <b>INHERIT_ROT_Y</b>\n
	  <b>INHERIT_ROT_Z</b>\n
	  <b>INHERIT_SCL_X</b>\n
	  <b>INHERIT_SCL_Y</b>\n
	  <b>INHERIT_SCL_Z</b>\n
	  <b>INHERIT_ALL</b>
	  \param keepPos If TRUE the position of the node should remain the same; otherwise the
	  node may move.
	  \return  Return TRUE if TM controller supports inheritance; otherwise FALSE.
	  \par Default Implementation:
	  <b>{return FALSE;}</b> */
	  virtual BOOL SetInheritanceFlags(DWORD f,BOOL keepPos) {return FALSE;} // return TRUE if TM controller supports inheritance

	  /*! \remarks Indicates whether the controller is a leaf controller. If a controller is
	  a leaf controller, then it MUST NOT BY DEFINITION have any
	  sub-controllers or references. The controller should return TRUE if it
	  has no sub-controllers. For example, a PRS controller is not a leaf
	  controller (because it has sub-controllers for Position, Rotation and
	  Scale), but a simple keyframed float controller is a leaf controller.
	  \return  TRUE if the controller is a leaf controller; FALSE otherwise.
	  \par Default Implementation:
	  <b>{return TRUE;}</b> */
	  virtual BOOL IsLeaf() {return TRUE;}
	  /*! \remarks Indicates if the controller is a keyframe controller. This means the
	  controller stores keys at certain frames and interpolates between keys at
	  other times.
	  \return  Nonzero if the controller is a keyframe controller; zero
	  otherwise.
	  \par Default Implementation:
	  <b>{return 1;}</b> */
	  virtual int IsKeyable() {return 1;}

	  // If a controller does not want to allow another controller
	  // to be assigned on top of it, it can return FALSE to this method.
	  /*! \remarks This method determines if another controller can replace this one. A
	  controller can return FALSE from this method to not allow the user to
	  assign a new controller in its place. This will also prevent the
	  controller from being replaced by a paste controller operation.
	  \return  TRUE to allow the controller to be replaced; otherwise FALSE.
	  \par Default Implementation:
	  <b>{return TRUE;}</b> */
	  virtual BOOL IsReplaceable() {return TRUE;}     

	  // This is called on TM, pos, rot, and scale controllers when their
	  // input matrix is about to change. If they return FALSE, the node will
	  // call SetValue() to make the necessary adjustments.
	  /*! \remarks This method is called on transform, position, rotation, and scale
	  controllers when their input matrix is about to change. This happens when
	  the user links an object (either from one object to another or when the
	  user links an object for the first time). Because a controllers
	  transformation is relative to its parent, when the user changes parents,
	  the transform controller will need to change itself. If a plug-in returns
	  FALSE the node will calculate a change and call <b>SetValue()</b> to make
	  the necessary adjustments at the specific time passed.\n\n
	  Consider the following example of a position controller:\n\n
	  If a node in the scene that is NOT animated, is linked to another node,
	  this method would be called. If the method returned FALSE then the node
	  would calculate a change and call <b>SetValue()</b> to make the
	  adjustment and this would be okay. If however the node was animated there
	  would be a problem. Say for example that an unlinked node was bouncing up
	  and down along the world Z axis. If this node is then linked to a node
	  that was rotated such that its Z axis was pointed in the direction of the
	  world X axis (so the object is flipped over on its side) the linked node
	  (whose animation keys are stored relative to its previous parent (the
	  world)) would then begin to bounce up and down along the world X axis
	  instead. This is because it is still moving along its parent's Z axis,
	  but its parents Z axis is really the world X axis. Thus the object needs
	  to be counter-rotated to compensate. Additionally, all the animation keys
	  for the object also need to be counter-rotated. A position keyframe
	  controller would need to implement this method to handle the correction
	  of the object and its keyframes. See the sample code below.
	  \param t The time of the change.
	  \param oldP The old parent matrix.
	  \param newP The new parent matrix.
	  \param tm The nodes current world transformation.
	  \return  If FALSE the node will call <b>SetValue()</b> to make the
	  necessary adjustments.
	  \par Default Implementation:
	  <b>{return FALSE;}</b>
	  \par Sample Code:
	  This is the code used inside 3ds Max' position controller. It takes the
	  difference between the two parents and transforms the position track by
	  that amount. It computes the relative transformation which is the old
	  parent times the inverse of the new parent.\n\n
	  A plug-in could provide an implementation for this method using a similar
	  concept.\n\n
		\code
		INTERP_CONT_TEMPLATE
		BOOL InterpControl<INTERP_CONT_PARAMS>::ChangeParents(TimeValue t,const Matrix3& oldP,const Matrix3& newP,const  Matrix3& tm)
		{
			if (SuperClassID()==CTRL_POSITION_CLASS_ID) {
				HoldTrack();
		// Position controllers need their path counter rotated to
		// account for the new parent.
					Matrix3 rel = oldP * Inverse(newP);
		// Modify the controllers current value (the controllers cache)
					*((Point3*)(&curval)) = *((Point3*)(&curval)) * rel;
		// Then modify the keys...
				for (int i=0; i<keys.Count(); i++) {
		// All this casting keeps the compiler happy
		// for non-Point3 versions of this template.
					*((Point3*)(&keys[i].val)) = *((Point3*)(&keys[i].val)) * rel;
				}
				keys.KeysChanged(FALSE);
				ivalid.SetEmpty();
				return TRUE;
			}
			else {
				return FALSE;
			}
		}
		\endcode */
	  virtual BOOL ChangeParents(TimeValue t,const Matrix3& oldP,const Matrix3& newP,const Matrix3& tm) {return FALSE;}

	  // val points to an instance of a data type that corresponds with the controller
	  // type. float for float controllers, etc.
	  // Note that for SetValue on Rotation controllers, if the SetValue is
	  // relative, val points to an AngAxis while if it is absolute it points
	  // to a Quat.
	  /*! \remarks Retrieves the value of the controller at the specified time, and updates
	  the validity interval passed in to reflect the interval of the
	  controller. This method is responsible for handling Out of Range Types,
	  Ease Curves and Multiplier Curves. See the sample code below.
	  \param t Specifies the time to retrieve the value.
	  \param val This points to a variable to hold the computed value of the controller at
	  the specified time. What the plug-in needs to do to store the value of
	  the controller depends on the controller type. There are six controller
	  types: <b>float, Point3, Position, Rotation, Scale,</b> and
	  <b>Transform</b>. The way the value is stored also depends on the
	  <b>GetSetMethod</b> parameter <b>method</b>. Below is list of the
	  possible cases and how the value should be stored in each case.\n\n
	  <b>float</b>\n\n
	  If <b>method == CTRL_ABSOLUTE</b>, <b>*val</b> points to a float.
	  The controller should simply store the value.\n\n
	  If <b>method == CTRL_RELATIVE</b>, <b>*val</b> points to a float. 
	  The controller should add its value to the existing floating point value.\n\n
	  <b>Point3</b>\n\n
	  If <b>method == CTRL_ABSOLUTE</b>, <b>*val</b> points to a <b>Point3</b>. 
	  The controller should simply store the value.\n\n
	  If <b>method == CTRL_RELATIVE</b>, <b>*val</b> points to a <b>Point3</b>. 
	  The controller should add its value to the existing <b>Point3</b> value.\n\n
	  <b>Position</b>\n\n
	  If <b>method == CTRL_ABSOLUTE</b>, <b>*val</b> points to a <b>Point3</b>.
	  The controller should simply store the value.\n\n
	  If <b>method == CTRL_RELATIVE</b>, <b>*val</b> points to a <b>Matrix3</b>. 
	  The controller should apply its value to the matrix by pre-multiplying
	  its position.
	  <b>Matrix3 *mat = (Matrix3*)val;</b>\n\n
	  <b>Point3 v =</b> the computed value of the controller...\n\n
	  <b>mat-\>PreTranslate(v);</b>\n\n
	  <b>Rotation</b>\n\n
	  If <b>method == CTRL_ABSOLUTE</b>, <b>*val</b> points to a <b>Quat</b>.
	  The controller should simply store the value.\n\n
	  If <b>method == CTRL_RELATIVE</b>, <b>*val</b> points to a <b>Matrix3</b>.
	  The controller should apply its value to the matrix by pre-multiplying its rotation.\n\n
	  <b>Matrix3 *mat = (Matrix3*)val;</b>\n\n
	  <b>Quat q =</b> the computed value of the controller...\n\n
	  <b>PreRotateMatrix(*mat,q);</b>\n\n
	  <b>Scale</b>\n\n
	  If <b>method == CTRL_ABSOLUTE</b>, <b>*val</b> points to a ScaleValue. 
	  The controller should simply store the value.\n\n
	  If <b>method == CTRL_RELATIVE</b>, <b>*val</b> points to a <b>Matrix3</b>. 
	  The controller should apply its value to the matrix by pre-multiplying its scale.\n\n
	  <b>Matrix3 *mat = (Matrix3*)val;</b>\n\n
	  <b>ScaleValue s =</b> the computed value of the controller...\n\n
	  <b>ApplyScaling(*mat,s);</b>\n\n
	  <b>Transform (Matrix3)</b>\n\n
	  If <b>method == CTRL_ABSOLUTE</b>, <b>*val</b> points to a <b>Matrix3</b>. 
	  The controller should simply store the value.\n\n
	  <b>Important:</b> Developers should only pass <b>CTRL_RELATIVE</b>
	  when getting the value of a <b>Matrix3</b> controller. This is
	  because the controller may use the matrix as input to compute the value.
	  Therefore it is not acceptable to use<b>CTRL_ABSOLUTE</b>to get
	  the value.\n\n
	  If <b>method == CTRL_RELATIVE</b>, <b>*val</b> points to a <b>Matrix3</b>. 
	  The controller should apply its value to the matrix by pre-multiplying.
	  When <b>GetValue()</b> is called on a transform controller the method is
	  <b>CTRL_RELATIVE</b> and the matrix passed is usually the <b>parent</b>
	  of the node.\n\n
	  <b>Important for Matrix3 Controllers:</b> when <b>SetValue()</b> is
	  called <b>*val</b> points to an instance of Class SetXFormPacket. See that
	  class for more details on how it is used.
	  \param valid The validity interval to update. The controllers validity interval should
	  be intersected with this interval. This updates the interval to reflect
	  the interval of the controller.
	  \param method One of the following values:\n\n
	  <b>CTRL_RELATIVE</b>\n
	  Indicates the plug-in should apply the value of the controller to <b>*val</b>. See Above.\n\n
	  <b>CTRL_ABSOLUTE</b>\n
	  Indicates the controller should simply store its value in <b>*val</b>. See Above.
	  \par Sample Code:
	  The following code is from the StdControl implementation of this method.
	  It demonstrates how the out of range and multiplier curves are
	  handled.\n\n
		\code
		void StdControl::GetValue(TimeValue t, void *val, Interval &valid,
		GetSetMethod method)
		{
			Interval range = GetTimeRange(TIMERANGE_ALL);
			Interval wvalid = FOREVER, cvalid = FOREVER;
			void *ptr = val;
			int ort;
			float m;
			TimeValue oldTime = t;
		
			if (method==CTRL_RELATIVE) {
				ptr = CreateTempValue();
			}
		
		// Grab the multiplier before the time warp.
			m = GetMultVal(t,valid);
		
		// Apply the time warp.
			t = ApplyEase(t,wvalid);
		
			if (t<=range.Start()) {
				ort = GetORT(ORT_BEFORE);
			}
			else {
				ort = GetORT(ORT_AFTER);
			}
		
			if (/startcomment*ort==ORT_CONSTANT*endcomment/TestAFlag(A_ORT_DISABLED) ||
			  range.Empty() || 
			  range.InInterval(t)) {
		
				GetValueLocalTime(t,ptr,cvalid);
			}
			else {
				switch (ort) {
					case ORT_CONSTANT:
						if (t<range.Start()) {
							GetValueLocalTime(range.Start(),ptr,cvalid,CTRL_ABSOLUTE);
							cvalid.Set(TIME_NegInfinity,range.Start());
						}
						else {
							GetValueLocalTime(range.End(),ptr,cvalid,CTRL_ABSOLUTE);
							cvalid.Set(range.End(),TIME_PosInfinity);
						}
						break;
		
					case ORT_LOOP:
					case ORT_CYCLE:
						GetValueLocalTime(CycleTime(range,t),ptr,
							cvalid,CTRL_ABSOLUTE);
						break;
		
					case ORT_OSCILLATE:
					{
						int cycles = NumCycles(range,t);
						TimeValue tp = CycleTime(range,t);
						if (cycles&1) {
							tp =
								range.End()-(tp-range.Start());
						}
		
						GetValueLocalTime(tp,ptr,cvalid,CTRL_ABSOLUTE);
						break;
					}
		
					case ORT_RELATIVE_REPEAT:
					case ORT_IDENTITY:
					case ORT_LINEAR:
						Extrapolate(range,t,ptr,cvalid,ort);
						break;
				}
			}
		
			if (m!=1.0f) {
				MultiplyValue(ptr,m);
			}
			if (method==CTRL_RELATIVE) {
				ApplyValue(val,ptr);
				DeleteTempValue(ptr);
			}
		
			if (ort!=ORT_CONSTANT) {
				cvalid.Set(oldTime,oldTime);
			}
			valid &= cvalid;
			valid &= wvalid;
		// Time warps can cause this to happen.
			if (valid.Empty()) valid.Set(oldTime,oldTime);
		}
		\endcode  */
		
	  virtual void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE)=0;
	  
	  /*! \remarks This method sets the value of the controller at the specified time. This
	  method is responsible for handling Out of Range Types, Ease Curves and
	  Multiplier Curves. See the sample code below.\n\n
	  Note: Developers who want to create keys for a keyframe controller by
	  calling <b>SetValue()</b> directly can do so, but the animate button
	  should be turned on using the following code:\n\n
	  <b>SuspendAnimate();</b>\n\n
	  <b>AnimateOn();</b>\n\n
	  <b>// Call SetValue() -- make sure commit=1</b>\n\n
	  <b>ResumeAnimate();</b>
	  \param t Specifies the time to save the value.
	  \param val Points to an instance of a data type that corresponds with the controller
	  type. These are the same as <b>GetValue()</b> above with the following
	  exceptions:\n\n
	  For rotation controllers, if the <b>GetSetMethod</b> is
	  <b>CTRL_RELATIVE</b>, <b>*val</b> points to an <b>AngAxis</b>, while if
	  it is <b>CTRL_ABSOLUTE</b> it points to a <b>Quat</b>.\n\n
	  For <b>Matrix3</b> controllers <b>*val</b> points to an instance of class
	  SetXFormPacket. See Class SetXFormPacket.
	  \param commit When <b>SetValue()</b> is called the controller should store it value
	  (usually into a cache it maintains), and if this parameter is set, also
	  'commit' it's value (usually by calling <b>CommitValue()</b>).\n\n
	  For example, consider a 3ds Max keyframe controller: If <b>commit==1</b>
	  and if the Animate button is on, then the cache should be updated and a
	  key should be created. If the Animate button is off then the cache should
	  be updated and the keys should all be offset. If <b>commit==0</b> then
	  the cache value is set and its validity interval is set to the current
	  time. If later <b>commit==1</b> then a key would be created from that
	  cached value. If <b>SetValue()</b> is never called with <b>commit=1</b>
	  then the key is never set. For instance with Inverse Kinetmatics,
	  <b>SetValue()</b> is called many times over and over at the same
	  <b>TimeValue</b> with <b>commit=0</b>. The controller doesn't create a
	  key, it just changes its cached value. When an IK solution is finally
	  reached, <b>SetValue()</b> is called with <b>commit=1</b> and a key is
	  created.\n\n
	  Note that calling <b>SetValue()</b> with <b>commit=0</b> and then calling
	  <b>CommitValue()</b> should have the same effect as calling
	  <b>SetValue()</b> with <b>commit=1</b>.\n\n
	  See the methods <b>CommitValue()</b> and <b>RestoreValue()</b> below.
	  \param method One of the following values:\n\n
	  <b>CTRL_RELATIVE</b>\n
	  Indicates the plug-in should add the value to the existing value
	  <b>*val</b> (i.e. Move/Rotate/Scale)\n\n
	  <b>CTRL_ABSOLUTE</b>\n
	  Indicates the plug-in should just set the value.
	  <b>Important Note for Matrix3 Controllers:</b>When <b>SetValue()</b> is
	  called the <b>method</b> parameter is ignored. The <b>*val</b> pointer
	  passed to <b>SetValue()</b> points to an instance of Class SetXFormPacket. See that
	  class for more details on how it is used.
	  \par Sample Code:
	  The following code is from the StdControl implementation of this method.
	  It demonstrates how the out of range and multiplier curves are
	  handled.\n\n
		\code
		void StdControl::SetValue(TimeValue t, void *val, int commit,
		GetSetMethod method)
		{
			Interval range = GetTimeRange(TIMERANGE_ALL);
			Interval wvalid, mvalid;
			int ort;
			float m;
		
		// Grab the multiplier before the time warp.
			m = GetMultVal(t,mvalid);
			if (m!=1.0f && m!=0.0f) {
				MultiplyValue(val,1.0f/m);
			}
		
		// Apply the time warp.
			t = ApplyEase(t,wvalid);
			if (range.Empty()) {
				SetValueLocalTime(t,val,commit,method);
				return;
			}
		
			if (t<=range.Start()) {
				ort = GetORT(ORT_BEFORE);
			}
			else {
				ort = GetORT(ORT_AFTER);
			}
			if (TestAFlag(A_ORT_DISABLED)) ort = ORT_CONSTANT;
		
			switch (ort) {
				case ORT_LOOP:
				case ORT_CYCLE:
					SetValueLocalTime(CycleTime(range,t),val,commit,method);
					break;
		
				case ORT_OSCILLATE:
				{
					int cycles = NumCycles(range,t);
					if (cycles&1) {
						t = range.Duration() -
							CycleTime(range,t);
					}
					else {
						t = CycleTime(range,t);
					}
					SetValueLocalTime(t,val,commit,method);
					break;
				}
		
		// These ORTs aren't cyclic so we just set the value
				out of range.
					case ORT_RELATIVE_REPEAT:
				case ORT_CONSTANT:
				case ORT_IDENTITY:
				case ORT_LINEAR:
					SetValueLocalTime(t,val,commit,method);
					break;
			}
		}
		\endcode */
	  virtual  void SetValue(TimeValue t, void *val, int commit=1, GetSetMethod method=CTRL_ABSOLUTE)=0;

	  /*! \remarks This method returns the PRS components of the local matrix. In general,
	  controller cannot decide on the local matrix without knowing the parent
	  matrix. However, many controllers, such as default controllers, are well
	  defined without the parent matrix. In these cases, it is more efficient
	  to compute the local components directly without going through the world
	  matrix.\n\n
	  Therefore, the argument parentMatrix is a reference to Matrix3Indirect.
	  This would allow clients to supply a "delayed parent matrix," which will
	  be computed only if it is necessary. It returns true for Matrix3,
	  Position, Rotation, or Scale controllers, and return false otherwise.\n\n
	  The PRS components will be put in argument cmpts in the respective fields
	  with corresponding validity intervals. NULL pointer, of
	  TMComponentsArg::position for example, indicates that the client is not
	  concerned about the component. When it is not NULL, the corresponding
	  pointer to the validity interval MUST NOT be NULL. When it is not NULL,
	  TMComponentsArg::rotation is a float[4]. rotRep tells what the numbers
	  mean.\n\n
	  Position, Rotation, or Scale, controllers will put results at the
	  respective component when the corresponding pointer is not NULL.\n\n
	  Upon entry, parentMatrix should represent the parent matrix up to the
	  first requested components. For Matrix3 controllers, for example, if
	  cmpts.position==NULL \&\& cmpts.rotation!=NULL, then parentMatrix should
	  be matrix that includes the parent node matrix plus the position of this
	  node. Upon return, this matrix may be modified.
	  \param t The time at which to get the local TM components.
	  \param cmpts See Structure TMComponentsArgs.
	  \param parentMatrix The parent matrix.\n\n
	  Note the definition: LocalMatrix = WorldMatrix * ParentWorldMatrix^(-1) */
	  CoreExport virtual bool GetLocalTMComponents(TimeValue t, TMComponentsArg& cmpts, Matrix3Indirect& parentMatrix);

	  // Transform controllers that do not inherit their parent's  transform 
	  // should override this method. Returning FALSE will cause SetValue 
	  // to be called even in the case when the parent is also being transformed.
	  /*! \remarks This method is only implemented by transform controllers. Transform
	  controllers that do not inherit their parent's transform should override
	  this method.\n\n
	  When a transform controller is evaluated, the parent transform is passed
	  in to the controller and the controller typically applies its value to
	  the parent transform. However, some controllers (for example Biped) may
	  choose to control the TM in an absolute manner and therefore ignore the
	  incoming parent's TM. The system needs to know about this because
	  normally if an object and its parent are selected and the user attempts
	  to move them only the parent transform is modified because it is assumed
	  that the child will inherit its parents TM.
	  \return  TRUE if the controller inherits its parents TM; otherwise FALSE.
	  Returning FALSE will cause <b>SetValue()</b> to be called even in the
	  case when the parent is also being transformed.
	  \note This method may still return TRUE even if all the bits returned
	  from <b>GetInheritanceFlags()</b> are SET to indicate that nothing is
	  inherited from the parent. This is simply because these methods don't
	  have the same level of 'granularity'. This method deals with the overall
	  inheritance of the parent's transform whereas the inheritance flags
	  relate to individual parts.
	  \par Default Implementation:
	  <b>{ return TRUE; }</b> */
	  virtual BOOL InheritsParentTransform() { return TRUE; }

	  /*! \remarks Implemented by the System.\n\n
	  Returns the specified Out of Range Type used by the controller. The
	  system handles this method but the controller needs to process the ORT in
	  its implementation of <b>GetValue()</b> and <b>SetValue()</b>.
	  \param type One of the following values:\n\n
	  <b>ORT_BEFORE</b> - leading up to the pattern\n\n
	  <b>ORT_AFTER</b> - beyond the key pattern
	  \return  One of the following values:\n\n
	  See \ref outOfRangeTypes. */
	  virtual int GetORT(int type) {return (aflag>>(type==ORT_BEFORE?A_ORT_BEFORESHIFT:A_ORT_AFTERSHIFT))&A_ORT_MASK;}
	  
	  /*! \remarks Implemented by the System.\n\n
	  Sets the specified Out of Range Type to be used by the controller. The
	  system handles this method but the controller needs to process the ORT in
	  its implementation of <b>GetValue()</b> and <b>SetValue()</b>.
	  \param ort See outOfRangeTypes.
	  \param type One of the following values:\n\n
	  <b>ORT_BEFORE</b> - leading up to the pattern\n\n
	  <b>ORT_AFTER</b> - beyond the key pattern */
	  CoreExport virtual void SetORT(int ort,int type);
	  
	  // Sets the enabled/disabled state for ORTs
	  /*! \remarks Implemented by the System.\n\n
	  Sets the enabled/disabled state for Out of Range Types. If disabled, this
	  temporarily causes the Out of Range Types to behave as if set to
	  constant. This can be used if you want to modify a controller but don't
	  want ORT mapping for ORT_LOOP, ORT_CYCLE, or ORT_OSCILLATE.
	  \param enable TRUE to enable ORTs; FALSE to disable. */
	  CoreExport virtual void EnableORTs(BOOL enable);

	  // Default implementations of load and save handle loading and saving of out of range type.
	  // Call these from derived class load and save.
	  // NOTE: Must call these before any of the derived class chunks are loaded or saved.
	  /*! \remarks Implemented by the System.\n\n
	  The default implementation of <b>Save()</b> handles the saving of the out
	  of range types. The plug-in should call this method from its
	  implementation of <b>Save()</b>. The plug-in should call this method
	  before it saves any of its chunks.\n\n
	  The out of range types are saved in these chunks:\n\n
	  <b>CONTROLBASE_CHUNK</b>\n\n
	  <b>INORT_CHUNK</b>\n\n
	  <b>OUTORT_CHUNK</b>
	  \param isave This pointer may be used to call methods to write data to disk. See Class ISave.
	  \return  One of the following values:\n\n
	  <b>IO_OK</b> - The result was acceptable - no errors.\n\n
	  <b>IO_ERROR</b> - This is returned if an error occurred. */
	  CoreExport IOResult Save(ISave *isave);
	  /*! \remarks Implemented by the System.\n\n
	  The default implementation of <b>Load()</b> handles the loading of the
	  out of range types. The plug-in should call this method from its
	  implementation of <b>Load()</b>. The plug-in should call this method
	  before it loads any of its chunks.\n\n
	  The out of range types are saved in these chunks:\n\n
	  <b>CONTROLBASE_CHUNK</b>\n\n
	  <b>INORT_CHUNK</b>\n\n
	  <b>OUTORT_CHUNK</b>
	  \param iload This pointer may be used to call methods to load data from disk. See Class ILoad.
	  \return  One of the following values:\n\n
	  <b>IO_OK</b> - The result was acceptable - no errors.\n\n
	  <b>IO_ERROR</b> - This is returned if an error occurred. */
	  CoreExport IOResult Load(ILoad *iload);

	  // For IK
	  // Note: IK params must be given in the order they are applied to
	  // the parent matrix. When derivatives are computed for a parameter
	  // that parameter will apply itself to the parent matrix so the next
	  // parameter has the appropriate reference frame. If a controller isn't
	  // participating in IK then it should return FALSE and the client (usually PRS)
	  // will apply the controller's value to the parent TM.
	  /*! \remarks This tells the system how many parameters the controller has. A
	  controller can have as many IK parameters as it wants. An IK parameter
	  corresponds to a degree of freedom in IK. The parameter is a floating
	  point scalar value. For example a position controller has three degrees
	  of freedom (X, Y, Z) and thus three parameters that IK can vary in its
	  solution. The path controller has only a single parameter (degree of
	  freedom) - the position along the path. The 3ds Max user may set the
	  number of degrees of freedom. For example, a user can specify that a
	  rotation controller cannot rotate about one or more axes. These are then
	  no longer degrees of freedom or IK parameters.\n\n
	  This method is called by the system so the plug-in can specify how many
	  IK parameters it has. It does this by calling the provided callback
	  object <b>proc()</b> method once for each parameter it has. It passes a
	  pointer to itself and the index of the IK parameter. For example a
	  position controller with three degrees of freedom (and thus three IK
	  parameters) would call the <b>callback.proc()</b> three time passing an
	  index of 0, then 1, then 2. See the sample code below.
	  \param callback This callback is provided by the system and should be called by the
	  plug-in once for each IK parameter the plug-in has. See Class IKEnumCallback.
	  \par Default Implementation:
	  <b>{}</b>
	  \par Sample Code:
		\code
		void QuatEnumIKParams(Control *cont,IKEnumCallback &callback)
		{
			JointParams *jp = (JointParams*)cont->GetProperty(PROPID_JOINTPARAMS);
			for (int i=0; i<3; i++) {
				if (!jp || jp->Active(i)) {
					callback.proc(cont,i);
				}
			}
		}
		\endcode */
	  virtual void EnumIKParams(IKEnumCallback &callback) {}
	  /*! \remarks This method is used to determine what effect a change in the parameter
	  has on the end effector. This is the derivative of the end effector with
	  respect to the parameter. What the derivative means in this case is what
	  happens to the end effector if the parameter is changed by some small
	  delta.\n\n
	  The plug-in provides the derivatives to the system calling
	  <b>derivs.DP()</b> and <b>derivs.DR()</b>. It should call
	  <b>derivs.DP()</b> and <b>derivs.DR()</b> in the same order as the
	  <b>callback.proc()</b> was called in the <b>NumIKParams()</b> method
	  implementation.\n\n
	  When the controller computes the derivative it should apply itself to the
	  parent matrix. For example a position controller would compute its
	  derivative based on the parent and the position of the end effector and
	  then apply itself to the parent matrix. If it does apply itself to the
	  parent it should return TRUE. If it does not apply itself it should
	  return FALSE.
	  \param t Specifies the time to compute the derivative.
	  \param ptm The parents transformation.
	  \param derivs This class provides methods the plug-in calls to set the derivatives. See Class IKDeriv.
	  \param flags One of the following values:\n\n
	  <b>POSITION_DERIV</b>\n
	  Indicates that <b>derivs.DP()</b> should be called.\n\n
	  <b>ROTATION_DERIV</b>\n
	  Indicates that <b>derivs.DR()</b> should be called.
	  \return  If a controller isn't participating in IK then it should return
	  FALSE and the client (usually PRS) will apply the controller's value to
	  the parent TM.
	  \par Default Implementation:
	  <b>{return FALSE;}</b>
	  \par Sample Code:
	  The following sample code shows how the quaternion controller has
	  implemented this method. Note that the method loops based on the number
	  of end effectors, and calls <b>derivs.NextDOF()</b> after each
	  iteration.\n\n
		\code
		BOOL QuatCompDeriv(Control *cont,TimeValue t,Matrix3& ptm, IKDeriv& derivs,DWORD flags)
		{
			JointParams *jp = (JointParams*)cont->GetProperty(PROPID_JOINTPARAMS);
			Quat q;
			Interval valid;
			for (int i=0; i<3; i++) {
				if (!jp || jp->Active(i)) {
					for (int j=0; j<derivs.NumEndEffectors(); j++) {
						Point3 r = derivs.EndEffectorPos(j) - ptm.GetRow(3);
						if (flags&POSITION_DERIV) {
							derivs.DP(CrossProd(ptm.GetRow(i),r),j);
						}
						if (flags&ROTATION_DERIV) {
							derivs.DR(ptm.GetRow(i),j);
						}
					}
					derivs.NextDOF();
				}
			}
			return FALSE;
		}
		\endcode */
	  virtual BOOL CompDeriv(TimeValue t,Matrix3& ptm,IKDeriv& derivs,DWORD flags) {return FALSE;}
	  /*! \remarks When the system has computed a change in the parameter it will call this
	  method. The controller should increment the specified parameter by the
	  specified delta. The controller can increment the parameter less than
	  this delta if it needs to. This could be for several reasons:\n\n
	  <b>1.</b> Its parameter may be constrained to lie within a specific
	  interval. It would not want to add a delta that took the parameter
	  outside of this interval.\n\n
	  <b>2.</b> It was asked to calculate a constant partial derivative for a
	  linkage that could be nonlinear. Therefore the derivative may have only
	  been an instantaneous approximation. Due to the locality of the IK
	  solution, the controller might not want to allow a delta that was too
	  large.\n\n
	  After the controller has applied the delta, it needs to indicate to the
	  system how much of the delta was used.
	  \param t The time of the increment.
	  \param index Specifies the IK parameter to increment.
	  \param delta The delta to apply to the parameter. The controller can increment the
	  parameter less than this delta if it needs to in order to accommodate a
	  limit it has. This methods returns the amount that was actually incremented.
	  \return  The amount the parameter was actually incremented. This allows
	  the IK solver to know the value was not incremented the full amount.
	  \par Default Implementation:
	  <b>{return 0.0f;}</b>
	  \par Sample Code:
		\code
		float QuatIncIKParam(Control *cont,TimeValue t,int index,float delta)
		{
			JointParams *jp =
				(JointParams*)cont->GetProperty(PROPID_JOINTPARAMS);
			if ((float)fabs(delta)>MAX_IKROT) delta = MAX_IKROT * SGN(delta);
			if (jp) {
				float v=0.0f;
				if (jp->Limited(index)) {
					Quat q;
					Interval valid;
					cont->GetValue(t,&q,valid,CTRL_ABSOLUTE);
					v = GetRotation(q,index);
				}
				delta = jp->ConstrainInc(index,v,delta);
			}
			Point3 a(0,0,0);
			a[index] = 1.0f;
			AngAxis aa(a,-delta);
			cont->SetValue(t,&aa,FALSE,CTRL_RELATIVE);
			return delta;
		}
		
		
		static float GetRotation(Quat& q,int axis)
		{
			Matrix3 tm;
			q.MakeMatrix(tm);
			MRow* t = tm.GetAddr();
			int n = (axis+1)%3, nn = (axis+2)%3;
			if (fabs(t[n][axis]) < fabs(t[nn][axis])) {
				return (float)atan2(t[n][nn],t[n][n]);
			}
			else {
				return -(float)atan2(t[nn][n],t[nn][nn]);
			}
		}
		\endcode  */
	  virtual float IncIKParam(TimeValue t,int index,float delta) {return 0.0f;}
	  /*! \remarks This method is called to have the controller delete its keys. If the user
	  has the 'Clear Keys' check box checked when they press the 'Apply IK'
	  button, this method is called to have the controller deletes keys in the
	  given interval for the specified degree of freedom.
	  \param iv The interval over which the keys should be deleted.
	  \param index Specified the degree of freedom (parameter) that the keys should be deleted for.
	  \par Default Implementation:
	  <b>{return;}</b> */
	  virtual void ClearIKParam(Interval iv,int index) {return;}
	  /*! \remarks This method returns TRUE if the controller has IK parameters it can copy
	  and FALSE otherwise.
	  \param which One of the following values:\n\n
	  <b>COPYPASTE_IKPOS</b>\n
	  <b>COPYPASTE_IKROT</b>
	  \return  TRUE if the controller can copy the specified IK parameters;
	  otherwise FALSE.
	  \par Default Implementation:
	  <b>{return FALSE;}</b> */
	  virtual BOOL CanCopyIKParams(int which) {return FALSE;}
	  /*! \remarks This method is called to have the controller copy the specified IK
	  parameters to an IKClipObject and return a pointer to it. The plug-in
	  should derive a class from the IKClipObject, put its data in the class,
	  and return a new instance of it. See
	  Class IKClipObject.
	  \param which One of the following values:\n\n
	  <b>COPYPASTE_IKPOS</b>\n
	  <b>COPYPASTE_IKROT</b>
	  \par Default Implementation:
	  <b>{return NULL;}</b> */
	  virtual IKClipObject *CopyIKParams(int which) {return NULL;}
	  /*! \remarks Returns TRUE if the controller can paste the specified IK parameters;
	  otherwise FALSE.
	  \param co A pointer to the current <b>IKClipObject</b> in the clipboard. This class
	  identifies the creator of the clip object. See Class IKClipObject. The plug-in
	  should look at the IDs in the <b>IKClipObject</b> to make sure it matches
	  this controller. If it does not, the plug-in should return FALSE.
	  \param which One of the following values:\n\n
	  <b>COPYPASTE_IKPOS</b>\n
	  <b>COPYPASTE_IKROT</b>
	  \return  TRUE if the controller can paste the specified IK parameters;
	  otherwise FALSE.
	  \par Default Implementation:
	  <b>{return FALSE;}</b> */
	  virtual BOOL CanPasteIKParams(IKClipObject *co,int which) {return FALSE;}
	  /*! \remarks This method is called to have the controller paste the specified IK
	  parameters from the specified IKClipObject to itself.
	  \param co A pointer to an IKClipObject. See Class IKClipObject.
	  \param which One of the following values:\n\n
	  <b>COPYPASTE_IKPOS</b>\n
	  <b>COPYPASTE_IKROT</b>
	  \par Default Implementation:
	  <b>{}</b> */
	  virtual void PasteIKParams(IKClipObject *co,int which) {}
	  /*! \remarks This is an optional method that can be implemented by controllers that
	  support IK to initialize their joint parameters based on data loaded from
	  3D Studio R4/ DOS files.
	  \param posData The position data from the 3DS file. See Class InitJointData.
	  \param rotData The rotation data from the 3DS file.
	  \par Default Implementation:
	  <b>{}</b> */
	  virtual void InitIKJoints(InitJointData *posData,InitJointData *rotData) {}
	  /*! \remarks	  This method retrieves the IK joint parameter data from the UI.
	  \param posData Points to the object to hold the position data. See Class InitJointData.
	  \param rotData Points to the object to hold the rotation data.
	  \return  TRUE if the data was retrieved; otherwise FALSE.
	  \par Default Implementation:
	  <b>{return FALSE;}</b> */
	  virtual BOOL GetIKJoints(InitJointData *posData,InitJointData *rotData) {return FALSE;}
	  /*! \remarks	  The new IK system has some axes gizmos which show the degrees of freedom,
	  etc. This method is called by the system.
	  \param t The current time.
	  \param ptm The parent matrix.
	  \param dofs This is the structure to be filled in. See Class DOFParams.
	  \param nodeSel TRUE if the node is currently selected; otherwise FALSE.
	  \return  TRUE if the method is implemented; FALSE otherwise.
	  \par Default Implementation:
	  <b>{return FALSE;}</b> */
	  virtual BOOL GetDOFParams(TimeValue t,Matrix3 &ptm,DOFParams &dofs,BOOL nodeSel) {return FALSE;}
	  /*! \remarks	  This method is called to create a locking key. This is a key that looks
	  back to the previous key and creates a new key at the specified time
	  which matches the previous key in value. It also adjusts the parameters
	  for the key such that the value stays constant from the previous key to
	  this key. For instance, the TCB controller will set the previous and new
	  continuity to 0. The Bezier controller sets the out tangent type of the
	  previous key to linear and the in tangent type of the new key to linear.
	  \param t The time to create the key.
	  \param which Specifies which type of key to create: <b>0</b> for position, <b>1</b> for rotation.
	  \return  TRUE if the method is implemented; FALSE otherwise.
	  \par Default Implementation:
	  <b>{return FALSE;}</b> */
	  virtual BOOL CreateLockKey(TimeValue t, int which) {return FALSE;}
	  /*! \remarks	  This method is called to mirror the specified IK constraints about the
	  specified axis. When IK constraints are mirrored they need to be updated
	  to reflect the new orientation. For instance, if you set the constraints
	  for a left arm to bend only +90 degrees along one axis and then copied
	  these to a right arm the joint would bend backwards. What you need to do
	  is provides the appropriate compensation so the orientation is kept
	  proper.
	  \param axis Specifies the axis of reflection: <b>0</b> for X, <b>1</b> for Y, <b>2</b> for Z.
	  \param which Specifies which type of constraints are being mirrored: <b>0</b> for
	  position, <b>1</b> for rotation.
	  \param pasteMirror TRUE if the mirror is being done as part of a paste operation; otherwise
	  FALSE (for example if the mirror was being done with the mirror tool).
	  \return  TRUE if the method is implemented; FALSE otherwise.
	  \par Default Implementation:
	  <b>{}</b> */
	  virtual void MirrorIKConstraints(int axis,int which,BOOL pasteMirror=FALSE) {}
	  /*! \remarks	  User can specifiy a node as a terminator. This method gives the
	  associated controller the chance to specify that it's terminated.
	  \return  TRUE if the method is implemented; FALSE otherwise.
	  \par Default Implementation:
	  <b>{return FALSE;}</b> */
	  virtual BOOL TerminateIK() {return FALSE;} // controllers can act as terminators.

	  // New for R4
	  /*! \remarks	  This is an optional method that can be implemented by controllers that
	  support IK to initialize their joint parameters based on data loaded from
	  3D Studio R4/ DOS files.
	  \param posData The position data from the 3DS file. See Class InitJointData2.
	  \param rotData The rotation data from the 3DS file.
	  \par Default Implementation:
	  <b>{}</b> */
	  virtual void InitIKJoints2(InitJointData2 *posData,InitJointData2 *rotData) {}
	  /*! \remarks	  This method retrieves the IK joint parameter data from the UI.
	  \param posData Points to the object to hold the position data. See Class InitJointData2.
	  \param rotData Points to the object to hold the rotation data.
	  \return  TRUE if the data was retrieved; otherwise FALSE.
	  \par Default Implementation:
	  <b>{return FALSE;}</b> */
	  virtual BOOL GetIKJoints2(InitJointData2 *posData,InitJointData2 *rotData) {return FALSE;}

	  // Called on a transform controller when the a message is received from a pin node
	  /*! \remarks	  If a node is pinned to another node, and the node gets a
	  <b>NotifyRefChanged()</b> message that its pinned node has changed, then
	  this method is called on the transform controller of the node. Otherwise
	  the controller wouldn't get notified since the controller doesn't have a
	  reference to the pin node (but the node does). Most controllers don't
	  really care, but the IK controller does.
	  \param message The message that was sent.
	  \param changeInt This is the interval of time over which the message is active. Currently,
	  all controllers will receive <b>FOREVER</b> for this interval.
	  \param partID This contains information specific to the message passed in. Some
	  messages don't use the <b>partID</b> at all. See \ref Reference_Messages and \ref partids for more 
	  information.
	  \return  The return value from this method is of type <b>RefResult</b>.
	  This is usually <b>REF_SUCCEED</b> indicating the message was processed.
	  Sometimes, the return value may be <b>REF_STOP</b>. This return value is
	  used to stop the message from being propagated to the dependents of the
	  item.
	  \par Default Implementation:
	  <b>{return REF_SUCCEED;}</b> */
	  virtual RefResult PinNodeChanged(RefMessage message,Interval changeInt, PartID &partID) {return REF_SUCCEED;}

	  // Called on a transform controller when one of the node level IK parameters has been changed
	  /*! \remarks	  This method is called on a transform controller when one of the node
	  level IK parameters has been changed.
	  \par Default Implementation:
	  <b>{}</b> */
	  /*! \remarks	  This method is called on a transform controller when one of the node
	  level IK parameters has been changed
	  \par Default Implementation:
	  <b>{}</b> */
	  virtual void NodeIKParamsChanged() {}

	  // Called in a transform controller when a node invalidates its TM cache
	  /*! \remarks	  This method is called in a transform controller when a node invalidates
	  its TM cache
	  \par Default Implementation:
	  <b>{}</b> */
	  virtual void TMInvalidated() {}

	  // Let's the TM controller determine if it's OK to bind (IK bind) to a particular node.
	  /*! \remarks	  This method lets a TM controller determine if it's OK to IK bind to a
	  particular node.
	  \param node Points to the node to check.
	  \return  TRUE if it's okay to bind; FALSE if it's not.
	  \par Default Implementation:
	  <b>{return TRUE;}</b> */
	  virtual BOOL OKToBindToNode(INode *node) {return TRUE;}

	  // Ease curves
	  /*! \remarks This method determines if a controller may have ease or multiplier curves
	  applied to it. This method defaults to returning TRUE, but can be
	  implemented to return FALSE by a controller that does not wish to let
	  ease or multiplier curves be applied to it.
	  \return  TRUE to allow the application of ease and multiplier curves;
	  otherwise FALSE.
	  \par Default Implementation:
	  <b>{return TRUE;}</b> */
	  virtual BOOL CanApplyEaseMultCurves() {return TRUE;}
	  /*! \remarks Implemented by the System.\n\n
	  The controller calls this method to pipe the TimeValue passed through the
	  ease curve to get the modified TimeValue.
	  \param t The time to have modified by the ease curve.
	  \param valid The validity interval of the TimeValue returned.
	  \return  The modified TimeValue. */
	  CoreExport TimeValue ApplyEase(TimeValue t,Interval &valid);
	  /*! \remarks Implemented by the System.\n\n
	  Adds an ease curve to the specified controller.
	  \param cont The controller that the ease curve will be applied to. */
	  CoreExport void AppendEaseCurve(Control *cont);
	  /*! \remarks Implemented by the System.\n\n
	  Deletes the 'i-th' ease curve from the controller.
	  \param i The index of the ease curve to delete. */
	  CoreExport void DeleteEaseCurve(int i);
	  /*! \remarks Implemented by the System.\n\n
	  Returns the number of ease curves applied to the controller. */
	  CoreExport int NumEaseCurves();

	  // Multiplier curves    
	  /*! \remarks Implemented by the System.\n\n
	  Retrieves a floating point value that is the product of all the
	  multiplier curves at the specified time.
	  \param t The time to retrieve the value. 
	  \param valid The validity interval of the value.
	  \return  The product of all the multiplier curves applied to the
	  controller. */
	  CoreExport float GetMultVal(TimeValue t,Interval &valid);
	  /*! \remarks Implemented by the System.\n\n
	  Adds a multiplier curve to the specified controller.
	  \param cont The controller to have the multiplier curve added. */
	  CoreExport void AppendMultCurve(Control *cont);
	  /*! \remarks Implemented by the System.\n\n
	  Deletes the 'i-th' multiplier curve from this controller.
	  \param i The index of the curve to delete. */
	  CoreExport void DeleteMultCurve(int i);
	  /*! \remarks Implemented by the System.\n\n
	  Returns the number of multiplier curves assigned to the controller. */
	  CoreExport int NumMultCurves();

	  // These are implemented to handle ease curves. If a controller
	  // is a leaf controller, then it MUST NOT BY DEFINITION have any
	  // sub controllers or references. If it is a leaf controller, then
	  // these are implemented to handle the ease curve list.
	  // If it is NOT a leaf controller, then these can be overridden.
	  CoreExport int NumRefs();
	  CoreExport RefTargetHandle GetReference(int i);
protected:
	  CoreExport virtual void SetReference(int i, RefTargetHandle rtarg);
public:
	  CoreExport int NumSubs();
	  CoreExport Animatable* SubAnim(int i);
	  CoreExport MSTR SubAnimName(int i);

	  // Default implementations of some Animatable methods
	  CoreExport void* GetInterface(ULONG id);
	  CoreExport int PaintFCurves(        
		 ParamDimensionBase *dim,
		 HDC hdc,
		 Rect& rcGraph,
		 Rect& rcPaint,
		 float tzoom,
		 int tscroll,
		 float vzoom,
		 int vscroll,
		 DWORD flags );
	  CoreExport int GetFCurveExtents(
		 ParamDimensionBase *dim,
		 float &min, float &max, DWORD flags);
		/*! \remarks Introduced an implementation in this the base class. */
	  CoreExport virtual void BaseClone(ReferenceTarget *from, ReferenceTarget *to, RemapDir &remap);


	  // This is called on transform controller after a node is
	  // cloned and the clone process has finished
	  /*! \remarks	  This method is called on a transform controller after a node is cloned
	  and the clone process has finished. This allows the controller to do any
	  work it needs to after the clone is complete.
	  \par Default Implementation:
	  <b>{}</b> */
	  virtual void PostCloneNode() {}

	  // Slave TM controllers can implement this to prevent plug-ins
	  // deleting their node via the DeleteNode API.
	  /*! \remarks	  This method is called on TM controllers so that system slave controllers
	  can prevent the <b>Interface::DeleteNode()</b> API from deleting them.
	  Note that <b>DeleteNode()</b>has an optional parameter to override this
	  so master controllers can easily ddelete slave nodes if they want to.
	  \return  TRUE to prevent deletion; FALSE to allow it.
	  \par Default Implementation:
	  <b>{return FALSE;}</b> */
	  virtual BOOL PreventNodeDeletion() {return FALSE;}

	  // New interface for visibility float controllers to allow view dependent visibility
	  // The default implementation will call GetValue()
	  /*! \remarks	  The Level of Detail utility lets you construct an object that alters its
	  geometric complexity (or level of detail) based on its size in the
	  rendered image. You do this by creating several versions of the same
	  object -- each with different levels of detail, grouping them as one, and
	  then assigning the Level of Detail utility, which automatically creates a
	  special LOD controller as a Visibility track. The LOD controller then
	  hides and unhides the various objects in the group, depending on their
	  size in the rendered scene.\n\n
	  This method is called on visibility float controllers with view related
	  parameters. This is used by the Level of Detail controller to allow view
	  dependent visibility.
	  \param t The time at which to evaluate.
	  \param view This class contains information about the view being rendered. This
	  includes information such as the image width and height, the projection
	  type, and matrices to convert between world to view and world to camera.
	  See Class View.
	  \param pbox The bounding box of the node that's being evaluated.
	  \param valid This interval should be updated to reflect the validity of the visibility
	  controller.
	  \return  The visibility of the object at the specified time.
	  \par Default Implementation:
	  The default implementation will simply call <b>GetValue()</b>. */
	  CoreExport virtual float EvalVisibility(TimeValue t, View& view, Box3 pbox, Interval& valid);
	  
	  // Called on visibility controllers. Gives them the option to completely hide an object in the viewports
	  /*! \remarks	  This method is called on visibility controllers. This gives them the
	  option to completely hide an object in the viewports.
	  \return  TRUE if the object is visible in the viewport; FALSE if
	  invisible.
	  \par Default Implementation:
	  <b>{return TRUE;}</b> */
	  virtual BOOL VisibleInViewports() {return TRUE;}

	  // Called on transform controllers or visibility controllers when a node is cloned and the user has chosen to instance
	  /*! \remarks	  Called on transform controllers or visibility controllers when a node is
	  cloned and the user has chosen to instance
	  \return   
	  \par Default Implementation:
	  <b>{return TRUE;}</b> */
	  virtual BOOL CanInstanceController() {return TRUE;}

	  // Should be called by any leaf controller's clone method so
	  // that ease and multipier curves are cloned.
	  /*! \remarks	  This method is implemented by the System. It should be called by any leaf
	  controller's <b>Clone()</b> method so that ease and multipier curves are
	  cloned.
	  \param ctrl Points to the cloned controller (the new one).
	  \param remap The <b>RemapDir</b> passed to this controller's <b>Clone()</b> method.
	  \par Sample Code:
		\code
		RefTargetHandle Clone(RemapDir& remap)
		{
			ExprControl *ctrl = new ExprControl(this->type, *this);
			CloneControl(ctrl,remap);
			return ctrl;
		}
		\endcode      */
	  CoreExport void CloneControl(Control *ctrl,RemapDir &remap);

	  //-------------------------------------------------------
	  // Controllers that wish to have an apparatus available in
	  // the scene will implement these methods:
	  // NOTE: Most of these methods are duplicated in BaseObject or Object
	  // (see object.h for descriptions).
	  /*! \remarks This is called by the system to have the controller display its gizmo.
	  When a controller is being edited in the Motion branch, this method is
	  called to allow it to display any apparatus it may have in the scene.
	  Note that <b>Display()</b> is only called on Transform Controllers. It is
	  not called only any sub-controllers, for example it wouldn't be called on
	  the position controller of a PRS transform controller.\n\n
	  In R4 and higher however the display method <b>WILL</b> be called on
	  Position, Rotation and scale controllers as well.
	  \param t The time to display the object.
	  \param inode The node to display.
	  \param vpt An interface pointer that exposes methods the plug-in may call related to the viewports.
	  \param flags See <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_display_flags.html">List of Display Flags</a>.
	  \return  Nonzero if the item was displayed; otherwise 0.
	  \par Default Implementation:
	  <b>{ return 0; }</b> */
	  virtual int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) { return 0; };
	  /*! \remarks This method is called to determine if the specified screen point
	  intersects the controller gizmo. The method returns nonzero if the gizmo
	  was hit; otherwise 0.
	  \param t The time to perform the hit test.
	  \param inode A pointer to the node whose gizmo should be tested.
	  \param type The type of hit testing to perform. See \ref SceneAndNodeHitTestTypes for details.
	  \param crossing The state of the crossing setting. If TRUE crossing selection is on.
	  \param flags The hit test flags. See \ref SceneAndNodeHitTestFlags for details.
	  \param p The screen point to test.
	  \param vpt An interface pointer that may be used to call methods associated with the viewports.
	  \return  Nonzero if the controller gizmo was hit; otherwise 0.
	  \par Default Implementation:
	  <b>{ return 0; }</b> */
	  virtual int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt) { return 0; }
	  
	  /*! \remarks This is the world space bounding box of the controllers gizmo.
	  \param t The time to retrieve the bounding box.
	  \param inode The node to calculate the bounding box for.
	  \param vpt An interface pointer that exposes portions of View3D that are exported
	  for use by plug-ins.
	  \param box The returned bounding box.
	  \par Default Implementation:
	  <b>{}</b> */
	  virtual  void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box) {}

	  /*! \remarks When the user changes the selection of the sub-object drop down, this
	  method is called to notify the plug-in. This method should provide
	  instances of a class derived from <b>CommandMode</b> to support move,
	  rotate, non-uniform scale, uniform scale, and squash modes. These modes
	  replace their object mode counterparts however the user still uses the
	  move/rotate/scale tool buttons in the toolbar to activate them. If a
	  certain level of sub-object selection does not support one or more of the
	  modes NULL may be passed. If NULL is specified the corresponding toolbar
	  button will be grayed out.
	  \param level The sub-object selection level the command modes should be set to
	  support. A level of 0 indicates object level selection. If level is
	  greater than or equal to 1 the index refers to the types registered by
	  the object in the order they appeared in the list when registered by
	  <b>Interface::RegisterSubObjectTypes()</b>. See Class Interface.
	  \param modes The command modes to support. See Class XFormModes.
	  \par Default Implementation:
	  <b>{}</b> */
	  virtual void ActivateSubobjSel(int level, XFormModes& modes ) {}

	  /*! \remarks This method is called to change the selection state of the component
	  identified by <b>hitRec</b>.
	  \param hitRec Identifies the component whose selected state should be modified. See
	  Class CtrlHitRecord.
	  \param selected TRUE if the item should be selected; FALSE if the item should be de-selected.
	  \param all TRUE if the entire object should be selected; FALSE if only the portion
	  of the identified by <b>hitRec</b>.
	  \param invert This is set to TRUE when <b>all</b> is also set to TRUE and the user is
	  holding down the Shift key while region selecting in select mode. This
	  indicates the items hit in the region should have their selection state
	  inverted.
	  \par Default Implementation:
	  <b>{}</b> */
	  virtual void SelectSubComponent(CtrlHitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE) {}
	  /*! \remarks This method is called to clear the selection for the given sub-object
	  level. All sub-object elements of this type should be deselected.
	  \param selLevel Specifies the selection level to clear.
	  \par Default Implementation:
	  <b>{}</b> */
	  virtual void ClearSelection(int selLevel) {}
	  /*! \remarks Implemented by the System.\n\n
	  Returns the index of the sub-object element identified by the
	  CtrlHitRecord <b>hitRec</b>. The sub-object index identifies a sub-object
	  component. The relationship between the index and the component is
	  established by the controller. For example a controller may allow the
	  user to select a group of footprints and these groups may be identified
	  as group 0, group 1, group 2, etc. Given a hit record that identifies a
	  footstep, the controller's implementation of this method would return the
	  group index that the footprint belonged to.
	  \param hitRec Identifies the component whose index should be returned. See Class CtrlHitRecord.
	  \return  The index of the sub-object element.
	  \par Default Implementation:
	  <b>{return 0;}</b> */
	  virtual int SubObjectIndex(CtrlHitRecord *hitRec) {return 0;}     
	  /*! \remarks	  This method is called to select every element of the given sub-object
	  level. This will be called when the user chooses Select All from the 3ds
	  Max Edit menu.
	  \param selLevel Specifies the selection level to select.
	  \par Default Implementation:
	  <b>{}</b> */
	  virtual void SelectAll(int selLevel) {}
	  /*! \remarks	  This method is called to invert the specified sub-object level. If the
	  element is selected it should be deselected. If it's deselected it should
	  be selected. This will be called when the user chooses Select Invert from
	  the 3ds Max Edit menu.
	  \param selLevel Specifies the selection level to invert.
	  \par Default Implementation:
	  <b>{}</b> */
	  virtual void InvertSelection(int selLevel) {}

	  /*! \remarks When the user is in a sub-object selection level, the system needs to get
	  the reference coordinate system definition from the current controller
	  being edited so that it can display the axes. This method specifies the
	  position of the center. The plug-in enumerates its centers and calls the
	  callback <b>cb</b> once for each. See
	  <a href="ms-its:3dsmaxsdk.chm::/selns_sub_object_coordinate_systems.html">Sub-Object
	  Coordinate Systems</a>.
	  \param cb The callback object whose methods may be called. See Class SubObjAxisCallback.
	  \param t The time to enumerate the centers.
	  \param node A pointer to the node.
	  \par Default Implementation:
	  <b>{}</b> */
	  virtual void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node) {}
	  /*! \remarks When the user is in a sub-object selection level, the system needs to get
	  the reference coordinate system definition from the current controller
	  being edited so that it can display the axes. This method returns the
	  axis system of the reference coordinate system. The plug-in enumerates
	  its TMs and calls the callback <b>cb</b> once for each. See
	  <a href="ms-its:3dsmaxsdk.chm::/selns_sub_object_coordinate_systems.html">Sub-Object
	  Coordinate Systems</a>.
	  \param cb The callback object whose methods may be called. See Class SubObjAxisCallback.
	  \param t The time to enumerate the TMs.
	  \param node A pointer to the node.
	  \par Default Implementation:
	  <b>{}</b>\n\n
		*/
	  virtual void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node) {}

	  // Modify sub object apparatuses
		/*!\remarks When this method is called the plug-in should respond by moving its selected
		sub-object components.
		\param t The time of the transformation.
		\param partm The 'parent' transformation matrix. This matrix represents a transformation
		that would take points in the controller's space and convert them into world space points.
		\param tmAxis The matrix that represents the axis system. This is the space in which the
		transformation is taking place.
		\param val This value is a vector with X, Y, and Z representing the movement along each axis.
		\param localOrigin When TRUE the transformation is occurring about the sub-object's local origin.
		\par Default Implementation:
		<b>{}</b> */
	  virtual void SubMove( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE ){}
	  /*! \remarks When this method is called the plug-in should respond by rotating its
	  selected sub-object components.
	  \par Parameters:
	  <b>TimeValue t</b>\n\n
	  The time of the transformation.\n\n
	  <b>Matrix3\& partm</b>\n\n
	  The 'parent' transformation matrix. This matrix represents a
	  transformation that would take points in the controller's space and
	  convert them into world space points.\n\n
	  <b>Matrix3\& tmAxis</b>\n\n
	  The matrix that represents the axis system. This is the space in which
	  the transformation is taking place.\n\n
	  <b>Quat\& val</b>\n\n
	  The amount to rotate the selected components.\n\n
	  <b>BOOL localOrigin=FALSE</b>\n\n
	  When TRUE the transformation is occurring about the sub-object's local
	  origin. Note: This information may be passed onto a transform controller
	  (if there is one) so they may avoid generating 0 valued position keys for
	  rotation and scales. For example if the user is rotating an item about
	  anything other than its local origin then it will have to translate in
	  addition to rotating to achieve the result. If a user creates an object,
	  turns on the animate button, and rotates the object about the world
	  origin, and then plays back the animation, the object does not do what
	  the was done interactively. The object ends up in the same position, but
	  it does so by both moving and rotating. Therefore both a position and a
	  rotation key are created. If the user performs a rotation about the local
	  origin however there is no need to create a position key since the object
	  didn't move (it only rotated). So a transform controller can use this
	  information to avoid generating 0 valued position keys for rotation and
	  scales. */
	  virtual void SubRotate( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin=FALSE ){}
	  /*! \remarks When this method is called the plug-in should respond by scaling its
	  selected sub-object components.
	  \param t The time of the transformation.
	  \param partm The 'parent' transformation matrix. This matrix represents a
	  transformation that would take points in the modifier's space and convert
	  them into world space points.
	  \param tmAxis The matrix that represents the axis system. This is the space in which
	  the transformation is taking place.
	  \param val This value is a vector with X, Y, and Z representing the scale along X,
	  Y, and Z respectively.
	  \param localOrigin When TRUE the transformation is occurring about the sub-object's local
	  origin. See the note above in the Rotate method. */
	  virtual void SubScale( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE ){}      
	  
	  // Schematic View Animatable Overides...
	  CoreExport virtual SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *object, int id, DWORD flags);
	  CoreExport virtual MSTR SvGetName(IGraphObjectManager *gom, IGraphNode *gNode, bool isBeingEdited);
	  CoreExport virtual bool SvHandleDoubleClick(IGraphObjectManager *gom, IGraphNode *gNode);
	  CoreExport virtual bool SvCanInitiateLink(IGraphObjectManager *gom, IGraphNode *gNode);
	  CoreExport virtual bool SvCanConcludeLink(IGraphObjectManager *gom, IGraphNode *gNode, IGraphNode *gNodeChild);
	  CoreExport virtual bool SvLinkChild(IGraphObjectManager *gom, IGraphNode *gNodeThis, IGraphNode *gNodeChild);
	  CoreExport virtual bool SvEditProperties(IGraphObjectManager *gom, IGraphNode *gNode);

	  // Called when the user rescales time in the time configuration dialog. If FALSE
	  // is returned from this method then MapKeys() will be used to perform the scaling. 
	  // Controllers can override this method to handle things like rescaling tagents that 
	  // MapKeys() won't affect and return TRUE if they don't want map keys to be called.
	  /*! \remarks	  This method is called when the user rescales time in the time
	  configuration dialog. If FALSE is returned from this method then
	  <b>Animatable::MapKeys()</b> will be used to perform the scaling.
	  Controllers can override this method to handle things like rescaling
	  tagents that <b>MapKeys()</b> won't affect and return TRUE if they don't
	  want <b>MapKeys()</b> to be called.
	  \param oseg The old time segment.
	  \param nseg The new time segment.
	  \par Default Implementation:
	  <b>{return FALSE;}</b> */
	  virtual BOOL RescaleTime(Interval oseg, Interval nseg) {return FALSE;}

//watje these are to allow a control to get sampled at a different rate than
//what trackview does by default so the controller can speed up redraws
//this is the pixel sample rate for when the curve is drawn
	  /*! \remarks	  Prior to R4 TrackView was using static defines to determines the number
	  samples/pixel it used to draw and compute curve extents. Now a controller
	  can override these defaults by implementing GetDrawPixelStep() and
	  GetExtentTimeStep().\n\n
	  This method allows a control to get sampled at a different rate than what
	  trackview does by default so the controller can speed up redraws. It
	  returns the pixel sample rate for when the curve is drawn.
	  \par Default Implementation:
	  <b>{return 5;}</b> */
	  virtual int GetDrawPixelStep() {return 5;}
//this is the ticks sample rate for when the curve is checked for its y extents
	  /*! \remarks	  Prior to R4 TrackView was using static defines to determines the number
	  samples/pixel it used to draw and compute curve extents. Now a controller
	  can override these defaults by implementing GetDrawPixelStep() and
	  GetExtentTimeStep().\n\n
	  This method returns the ticks sample rate used when the curve is checked
	  for its Y extents.
	  \par Default Implementation:
	  <b>{return 40;}</b> */
	  virtual int GetExtentTimeStep() {return 40;}
   };

// Any controller that does not evaluate itself as a function of it's
// input can subclass off this class.
// GetValueLocalTime() will never ask the controller to apply the value,
// it will always ask for it absolute.
/*! \sa  Class Control, Class Interval, Class Matrix3, Class Point3,  Class Quat, Class ScaleValue, <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_controller_related_methods.html">List of Additional
Control Related Functions</a>\n\n
\par Description:
StdControl is a class from which you may derived controller objects.
Controllers are the objects in 3ds Max that control animation. Any controller
that does not evaluate itself as a function of its input can subclass off this
class.\n\n
The purpose of this class is to simplify some aspects of implementing
controllers. The only restriction when using this class is that the controller
can not evaluate itself as a function of its input. For example, position,
rotation, and scale controllers are passed a TM when they are evaluated. They
are supposed to calculate their value and apply it to the TM (pre-multiply). It
is possible that the controller could calculate its value as some function of
this input matrix. For example, a rotation controller could look at the
position of the matrix and the position of some other node in the scene and
calculate the rotation such that the object is looking at the other node. Most
controllers don't do this so they can subclass off this class which handles
processing ORTs (Out of Range Types), ease curves and multiplier curves.\n\n
This class implements <b>GetValue()</b> and <b>SetValue()</b> but requires the
derived class to implement two new methods: <b>GetValueLocalTime()</b> and
<b>SetValueLocalTime()</b>.\n\n
The implementations of <b>GetValue()</b> and <b>SetValue()</b> handle
processing the ORTs and ease and multiplier curves.
\par Plug-In Information:
Class Defined In CONTROL.H\n\n
Super Class ID CTRL_FLOAT_CLASS_ID - Used by float controllers.\n\n
 CTRL_POINT3_CLASS_ID - Used by Point3 controllers.\n\n
 CTRL_MATRIX3_CLASS_ID - Used by Matrix3 controllers.\n\n
 CTRL_POSITION_CLASS_ID - Used by position controllers.\n\n
 CTRL_ROTATION_CLASS_ID - Used by rotation controllers.\n\n
 CTRL_SCALE_CLASS_ID - Used by scale controllers.\n\n
 CTRL_MORPH_CLASS_ID - Used by morph controllers.\n\n
Standard File Name Extension DLC\n\n
Extra Include File Needed None  */
class StdControl : public Control {
   public:     
	  /*! \remarks This method is called to have the controller evaluate itself at the given
	  time. In this class the system implements the method <b>GetValue()</b>.
	  <b>GetValue()</b> calls this method to retrieves the value of the
	  controller at the specified time. The implementation of <b>GetValue()</b>
	  then takes care of handling the ORTs, ease curves and multiplier curves.
	  The plug-in must only return the value of the controller.
	  \param t The time to retrieve the value.
	  \param val This points to a variable to hold the computed value of the controller at
	  the specified time. What the plug-in needs to do to store the value of
	  the controller depends on the controller type. There are six controller
	  types as follows:\n\n
	  <b>float</b>\n
	  <b>*val</b> points to a float\n\n
	  <b>Point3</b>\n
	  <b>*val</b> points to a Point3\n\n
	  <b>Position</b>\n
	  <b>*val</b> points to a Point3\n\n
	  <b>Rotation</b>\n
	  <b>*val</b> points to a Quat\n\n
	  <b>Scale</b>\n
	  <b>*val</b> points to a ScaleValue\n\n
	  <b>Transform</b>\n
	  <b>*val</b> points to a Matrix3\n
	  \param valid The interval to update. The controllers validity interval should be
	  intersected with this interval. This updates the interval to reflect the
	  interval of the controller.
	  \param method This will always be: <b>CTRL_ABSOLUTE</b>\n\n. It indicates the 
	  controller should simply store its value in <b>*val</b>. */
	  virtual void GetValueLocalTime(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE)=0;
	  /*! \remarks In this class the system implements the method <b>SetValue()</b>.
	  <b>SetValue()</b> calls this method to store the value of the controller
	  at the specified time. The system takes care of handling the ORTs and
	  multiplier curves. The plug-in must only store the value of the
	  controller.
	  \param t The time to store the value.
	  \param val Storage for the value to set. See <b>*val</b> in
	  <b>Control::SetValue()</b> for the possible data types passed here.
	  \param commit If this parameter is zero, the controller should save the value at the
	  given time before setting the value. If commit is nonzero, the controller
	  doesn't need to actually update its keys or tangents. See Control methods
	  <b>CommitValue()</b> and <b>RestoreValue()</b>.
	  \param method One of the following values:\n\n
	  <b>CTRL_RELATIVE</b>\n
	  Indicates the plug-in should add the value to the existing value
	  <b>*val</b> (i.e. Move/Rotate/Scale)\n\n
	  <b>CTRL_ABSOLUTE</b>\n
	  Indicates the plug-in should just set the value. */
	  virtual  void SetValueLocalTime(TimeValue t, void *val, int commit=1, GetSetMethod method=CTRL_ABSOLUTE)=0;
	  /*! \remarks This method is implemented by the system. Controller that
	  subclass from StdControl only need to implement
	  <b>GetValueLocalTime()</b>. See above. */
	  CoreExport void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
	  /*! \remarks This method is implemented by the system. Controller that
	  subclass from StdControl only need to implement
	  <b>SetValueLocalTime()</b>. See above. */
	  CoreExport void SetValue(TimeValue t, void *val, int commit=1, GetSetMethod method=CTRL_ABSOLUTE);
	  // Computes the local components without calling parentMatrix
	  // for position, rotation, and Scale controllers.
	  CoreExport bool GetLocalTMComponents(TimeValue t, TMComponentsArg& cmpts, Matrix3Indirect& parentMatrix);

	  /*! \remarks This method is used to calculate some of the Out of Range Types (ORTs).
	  There are several kinds of extrapolations that need to be done based on
	  the ORT type. There are template functions implemented in
	  <b>CONTROL.H</b> to do them. See
	  <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_misc_ctrl_funcs.html">List of Miscellaneous
	  Controller Functions</a> for a description of those available.
	  \param range The range that must be extrapolated.
	  \param t The time outside the range to extrapolate.
	  \param val Storage for the extrapolated value. See <b>*val</b> in
	  <b>GetValueLocalTime()</b> for the possible data types passed here.
	  \param valid The validity interval of the extrapolated value.
	  \param type See \ref outOfRangeTypes.
	  \par Sample Code:
	  The following sample code shows the use of the template functions to
	  implement this method to calculate the ORTs.
		\code
		INTERP_CONT_TEMPLATE
		void InterpControl<INTERP_CONT_PARAMS>::Extrapolate(Interval range,TimeValue t,void *val, Interval &valid,int type)
		{
			T val0, val1, val2, res;
			switch (type) {
				case ORT_LINEAR:
					if (t<range.Start()) {
						GetValueLocalTime(range.Start(),&val0,valid);
						GetValueLocalTime(range.Start()+1,&val1,valid);
						res = LinearExtrapolate(range.Start(),t,val0,val1,val0);
					}
					else {
						GetValueLocalTime(range.End()-1,&val0,valid);
						GetValueLocalTime(range.End(),&val1,valid);
						res = LinearExtrapolate(range.End(),t,val0,val1,val1);
					}
					break;
		
				case ORT_IDENTITY:
					if (t<range.Start()) {
						GetValueLocalTime(range.Start(),&val0,valid);
						res = IdentityExtrapolate(range.Start(),t,val0);
					}
					else {
						GetValueLocalTime(range.End(),&val0,valid);
						res = IdentityExtrapolate(range.End(),t,val0);
					}
					break;
		
				case ORT_RELATIVE_REPEAT:
					GetValueLocalTime(range.Start(),&val0,valid);
					GetValueLocalTime(range.End(),&val1,valid);
					GetValueLocalTime(CycleTime(range,t),&val2,valid);
					res = RepeatExtrapolate(range,t,val0,val1,val2);
					break;
			}
			valid.Set(t,t);
			*((T*)val) = res;
		}
		\endcode  */
	  virtual void Extrapolate(Interval range,TimeValue t,void *val,Interval &valid,int type)=0;
	  
	  /*! \remarks When processing the ORTs the system might need a temporary variable to
	  hold an intermediate value. Since the system doesn't know the type of the
	  data that the controller is controlling it can't allocate the right
	  amount of temporary storage. It calls this method to do so. The plug-in's
	  implementation of this method should allocate storage to hold its type of
	  data and return a pointer to it. */
	  virtual void *CreateTempValue()=0;
	  /*! \remarks This method simply deletes the memory allocated by
	  <b>CreateTempValue()</b>.
	  \param val Points to the memory allocated by <b>CreateTempValue()</b>. */
	  virtual void DeleteTempValue(void *val)=0;
	  /*! \remarks Applies the given value to the given input value. For position, rotation,
	  and scale controllers, the input value will be a matrix and the value
	  being applied will be a Point3, Quaternion, or ScaleValue, respectively.
	  For other controllers the input value is the same type as the value being
	  applied.
	  \param val The value to update.
	  \param delta The value to apply. */
	  virtual void ApplyValue(void *val, void *delta)=0;
	  /*! \remarks If the controller has multiplier curves then the system will calculate
	  the factor from all the multiplier curves and then ask the controller to
	  multiply the scalar value to the particular data type.
	  \param val The value to update.
	  \param m The scalar value to multiply <b>*val</b> by. */
	  virtual void MultiplyValue(void *val, float m)=0;
   };

// Each super class of controller may have a specific packet defined that
// the 'val' pointer will point to instead of a literal value.
// In reality, probably only the Transform controller will do this.
enum SetXFormCommand { XFORM_MOVE, XFORM_ROTATE, XFORM_SCALE, XFORM_SET };
/*! \sa  Class Control, Class Matrix3,  Class Quat,  Class AngAxis, Class Point3.\n\n
\par Description:
This class is used to allow a transform (<b>Matrix3</b>) controller to know
that it is being specifically moved, rotated, or scaled.\n\n
When <b>SetValue()</b> is called on a controller, the <b>val</b> pointer is
passed in for a certain data type. For a transform (<b>Matrix3</b>) controller
<b>SetValue()</b> passes in a pointer to an instance of this. This provides
higher level information to the transform controller than what is provided by
passing a matrix. For example, if rotation is taking place, the
<b>XFORM_ROTATE</b> command would be used. In this way the PRS transform
controller would not make position or scale keys since it knows only rotation
is taking place. Typically one of the different constructors is used depending
on the command needed. All methods of this class are implemented by the system.
\par Data Members:
<b>SetXFormCommand command;</b>\n\n
The command. The transform controller takes the <b>val</b> pointer and casts it
to an instance of this class and looks at this data member to see which
operation is being performed.\n\n
One of the following values:\n\n
<b>XFORM_MOVE</b>\n\n
The move command. An incremental move is being applied to the matrix.\n\n
<b>XFORM_ROTATE</b>\n\n
The rotate command. An incremental rotation is being applied to the matrix.\n\n
<b>XFORM_SCALE</b>\n\n
The scale command. An incremental scaling is being applied to the matrix.\n\n
<b>XFORM_SET</b>\n\n
To just set the matrix without telling the controller any other higher level
information this command may be used. This just sets the value of the matrix
(it is not incremental). Any time a node modifies a <b>Matrix3</b> controller,
it will set the method to get <b>CTRL_RELATIVE</b>, and the packet command is
set to <b>XFORM_SET</b>.\n\n
<b>Matrix3 tmParent;</b>\n\n
The parent matrix.\n\n
<b>Matrix3 tmAxis;</b>\n\n
This usually represents the coordinate system one is moving, rotating, or
scaling in. However, if the <b>command</b> is <b>XFORM_SET</b>, then
<b>tmAxis</b> is the actual matrix being set.\n\n
<b>Point3 p;</b>\n\n
If the <b>command</b> is <b>XFORM_MOVE</b> or <b>XFORM_SCALE</b>, then this
contains the amount of the move or scale.\n\n
<b>Quat q;</b>\n\n
If the command is <b>XFORM_ROTATE</b> then this contains the amount of the
rotation.\n\n
<b>AngAxis aa;</b>\n\n
If the command is <b>XFORM_ROTATE</b> this will also contain the amount of the
rotation. This form can represent multiple revolutions however (as opposed to
<b>Quat q</b>).\n\n
<b>BOOL localOrigin;</b>\n\n
Indicates the local axis is being used. If TRUE it is; otherwise it is not. If
the rotation or scaling is occurring about the pivot point this is TRUE. <br>
*/
class SetXFormPacket: public MaxHeapOperators {
   public:
	  SetXFormCommand command;
	  Matrix3 tmParent;
	  Matrix3 tmAxis;      // if command is XFORM_SET, this will contain the new value for the XFORM.    
	  Point3 p;
	  Quat q;
	  AngAxis aa;
	  BOOL localOrigin;    
	  
	  // XFORM_SET
	  /*! \remarks Constructor. The <b>XFORM_SET</b> command.
	  \par Parameters:
	  <b>const Matrix3\& mat</b>\n\n
	  The <b>tmAxis</b> value.\n\n
	  <b>const Matrix3\& par=Matrix3(1)</b>\n\n
	  The <b>tmParent</b> value. */
	  SetXFormPacket(const Matrix3& mat,const Matrix3& par=Matrix3(1))
		 {command=XFORM_SET,tmParent=par,tmAxis=mat;}

	  // XFORM_MOVE
	  /*! \remarks Constructor. The <b>XFORM_MOVE</b> command.
	  \par Parameters:
	  <b>Point3 pt</b>\n\n
	  The <b>p</b> value.\n\n
	  <b>const Matrix3\& par=Matrix3(1)</b>\n\n
	  The <b>tmParent</b> value.\n\n
	  <b>const Matrix3\& a=Matrix3(1)</b>\n\n
	  The <b>tmAxis</b> value. */
	  SetXFormPacket(Point3 pt, const Matrix3& par=Matrix3(1), 
				  const Matrix3& a=Matrix3(1))
		 {command=XFORM_MOVE;tmParent=par;tmAxis=a;p=pt;localOrigin=FALSE;}

	  // XFORM_ROTATE
	  /*! \remarks Constructor. The <b>XFORM_ROTATE</b> command.
	  \par Parameters:
	  <b>Quat qt</b>\n\n
	  The <b>q</b> value.\n\n
	  <b>BOOL l</b>\n\n
	  The <b>localOrigin</b> value.\n\n
	  <b>const Matrix3\& par=Matrix3(1)</b>\n\n
	  The <b>tmParent</b> value.\n\n
	  <b>const Matrix3\& a=Matrix3(1)</b>\n\n
	  The <b>tmAxis</b> value. */
	  SetXFormPacket(Quat qt, BOOL l, const Matrix3& par=Matrix3(1),
				  const Matrix3& a=Matrix3(1))
		 {command=XFORM_ROTATE;tmParent=par;tmAxis=a;q=qt;aa=AngAxis(q);localOrigin=l;}
	  /*! \remarks Constructor. The <b>XFORM_ROTATE</b> command.
	  \par Parameters:
	  <b>AngAxis aA</b>\n\n
	  The <b>aa</b> value.\n\n
	  <b>BOOL l</b>\n\n
	  The <b>localOrigin</b> value.\n\n
	  <b>const Matrix3\& par=Matrix3(1)</b>\n\n
	  The <b>tmParent</b> value.\n\n
	  <b>const Matrix3\& a=Matrix3(1)</b>\n\n
	  The <b>tmAxis</b> value. */
	  SetXFormPacket(AngAxis aA, BOOL l, const Matrix3& par=Matrix3(1),
				  const Matrix3& a=Matrix3(1))
		 {command=XFORM_ROTATE;tmParent=par;tmAxis=a;q=Quat(aA);aa=aA;localOrigin=l;}

	  // XFORM_SCALE
	  /*! \remarks Constructor. The <b>XFORM_SCALE</b> command.
	  \par Parameters:
	  <b>Point3 pt</b>\n\n
	  The <b>p</b> value.\n\n
	  <b>BOOL l</b>\n\n
	  The <b>localOrigin</b> value.\n\n
	  <b>const Matrix3\& par=Matrix3(1)</b>\n\n
	  The <b>tmParent</b> value.\n\n
	  <b>const Matrix3\& a=Matrix3(1)</b>\n\n
	  The <b>tmAxis</b> value. */
	  SetXFormPacket(Point3 pt, BOOL l, const Matrix3& par=Matrix3(1),
				  const Matrix3& a=Matrix3(1))
		 {command=XFORM_SCALE;tmParent=par;tmAxis=a;p=pt;localOrigin=l;}

	  // Just in case you want to do it by hand...
	  /*! \remarks Constructor. This constructor is provided in case you want
	  to set the data members yourself. */
	  SetXFormPacket() {};
   };



// This is a special control base class for controllers that control
// morphing of geomoetry.
//
// The 'val' pointer used with GetValue will point to an object state.
// This would be the result of evaluating a combination of targets and
// producing a new object that is some combination of the targets.
//
// The 'val' pointer used with SetValue will point to a 
// SetMorphTargetPacket data structure. This has a pointer to
// an object (entire pipeline) and the name of the target.

// A pointer to one of these is passed to SetValue
/*! \sa  Class MorphControl, Class Object, Class Matrix3.\n\n
\par Description:
The Morph Object communicates with the Morph Controller through this class. A
pointer to one of these is passed to <b>MorphControl::SetValue()</b> as the
<b>val</b> parameter. All methods of this class are implemented by the system.
\par Data Members:
<b>Matrix3 tm;</b>\n\n
The relative transformation matrix from the Morph Object to the Target.\n\n
<b>Object *obj;</b>\n\n
The target object.\n\n
<b>MSTR name;</b>\n\n
The name it creates.\n\n
<b>BOOL forceCreate;</b>\n\n
If TRUE then make sure the key is created even if it is at frame 0.  */
class SetMorphTargetPacket: public MaxHeapOperators {
   public:
	  Matrix3 tm;
	  Object* obj;
	  MSTR name;
	  BOOL forceCreate; // Make sure the key is created even if it is at frame 0
	  /*! \remarks Constructor. The data members are initialized to the values
	  passed. */
	  SetMorphTargetPacket(Object *o,MSTR n,Matrix3 &m,BOOL fc=FALSE) {obj = o;name = n;tm = m;forceCreate=fc;}
	  /*! \remarks Constructor. The data members are initialized to the values
	  passed. The matrix is set to the identity. */
	  SetMorphTargetPacket(Object *o,MSTR n,BOOL fc=FALSE) {obj = o;name = n;tm = Matrix3(1);forceCreate=fc;}
   };

/*! \sa  Class Control, Class SetMorphTargetPacket, Class Object, Class ObjectState.\n\n
\par Description:
This is a special control base class for controllers that control morphing of
geometry.\n\n
For the 3ds Max morph system, the morph object itself doesn't do much except
provide the front end UI in the create and modify branches. It just allows the
user to add targets, delete targets, etc., but it doesn't really do any of the
work -- it just passes everything on to the morph controller.\n\n
The morph controller has references to the objects (one for each target) and
stores keys in whatever form it wants, and interpolates between the objects. In
3ds Max 1.x there was only one morph controller. In 3ds Max 2.0 there is the
new Barycentric one. Both of these work with any type of object as long as its
Deformable. It just deforms the points.\n\n
The key to the way this works is that the controller just takes the targets and
creates a new object from some combination of the targets. Basically the morph
object calls <b>GetValue()</b> on the controller and passes it an
<b>ObjectState</b>. The morph controller then computes the new object and drops
it down into the <b>ObjectState</b> passed. This can be any object.\n\n
The <b>val</b> pointer used with <b>GetValue()</b> will point to an
<b>ObjectState</b>. This would be the result of evaluating a combination of
targets and producing a new object that is some combination of the targets.\n\n
The <b>val</b> pointer used with <b>SetValue()</b> will point to a
<b>SetMorphTargetPacket</b> data structure. This has a pointer to an object
(entire pipeline) and the name of the target.\n\n
The Morph Object is not really an object. Rather it has a reference to the
Morph Controller which is the thing that actually generates the object. The
Morph Controller needs to call <b>Object::HasUVW()</b> and
<b>Object::SetGenUVW()</b> on each of the targets.  */
class MorphControl : public Control {
   public:
	  
	  // Access the object pipelines of the controller's targets. Note
	  // that these are pointers to the pipelines, not the result of
	  // evaluating the pipelines.
	  /*! \remarks Returns the number of morph targets managed by this
	  controller.
	  \par Default Implementation:
	  <b>{return 0;}</b> */
	  virtual int NumMorphTargs() {return 0;}
	  /*! \remarks Returns a pointer to the 'i-th' object managed by this
	  morph controller. Note that these are pointers to the pipelines, not the
	  result of evaluating the pipelines.
	  \par Parameters:
	  <b>int i</b>\n\n
	  The index of the morph target. This value may be <b>0</b> through
	  <b>NumMorphTargs()-1</b>.
	  \par Default Implementation:
	  <b>{return NULL;}</b> */
	  virtual Object *GetMorphTarg(int i) {return NULL;}
	  /*! \remarks Deletes the 'i-th' target.
	  \par Parameters:
	  <b>int i</b>\n\n
	  The index of the morph target. This value may be <b>0</b> through
	  <b>NumMorphTargs()-1</b>.
	  \par Default Implementation:
	  <b>{}</b> */
	  virtual void DeleteMorphTarg(int i) {}
	  /*! \remarks Retrieves the name of the specified morph target.
	  \par Parameters:
	  <b>int i</b>\n\n
	  The index of the morph target whose name is returned. This value may be
	  <b>0</b> through <b>NumMorphTargs()-1</b>.\n\n
	  <b>MSTR \&name</b>\n\n
	  The name is returned here.
	  \par Default Implementation:
	  <b>{name.printf(_M("Target #%d"),i);}</b> */
	  virtual void GetMorphTargName(int i,MSTR &name) {name.printf(_M("Target #%d"),i);}
	  /*! \remarks Sets the name of the specified morph target.
	  \par Parameters:
	  <b>int i</b>\n\n
	  The index of the morph target whose name is set. This value may be
	  <b>0</b> through <b>NumMorphTargs()-1</b>.\n\n
	  <b>MSTR \&name</b>\n\n
	  The to set is passed here.
	  \par Default Implementation:
	  <b>{}</b> */
	  virtual void SetMorphTargName(int i,MSTR name) {}
	  /*! \remarks Retrieves the transformation matrix of the 'i-th' morph
	  target.
	  \par Parameters:
	  <b>int i</b>\n\n
	  The index of the morph target whose TM is returned. This value may be
	  <b>0</b> through <b>NumMorphTargs()-1</b>.
	  \par Default Implementation:
	  <b>{return Matrix3(1);}</b> */
	  virtual Matrix3 GetMorphTargTM(int i) {return Matrix3(1);}

	  // Checks an object to see if it is an acceptable target.
	  /*! \remarks This method checks the object passed to see if it is an
	  acceptable target.
	  \par Parameters:
	  <b>TimeValue t</b>\n\n
	  The time to check the object.\n\n
	  <b>Object *obj</b>\n\n
	  The object to check.
	  \return  TRUE if the object is a valid target; otherwise FALSE.
	  \par Default Implementation:
	  <b>{return FALSE;}</b> */
	  virtual BOOL ValidTarget(TimeValue t,Object *obj) {return FALSE;}

	  // When a \ref REFMSG_SELECT_BRANCH message is received the morph controller should
	  // mark the target indicated and be prepared to return its ID from this method.
	  /*! \remarks When a 3ds Max reference \ref REFMSG_SELECT_BRANCH
	  message is received the morph controller should mark the target indicated
	  and be prepared to return its ID from this method. This way the morph
	  object can set the branch to that target. See this message in \ref Reference_Messages.
	  \return  The ID of the morph target.
	  \par Default Implementation:
	  <b>{return -1;}</b> */
	  virtual int GetFlaggedTarget() {return -1;}

	  // Should call these methods on targets
	  virtual BOOL HasUVW() { return 1; }
	  virtual void SetGenUVW(BOOL sw) {  }
   };

//-------------------------------------------------------------
// Control base class for Master Controllers
//

/*! \sa  Class Control.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This is the base class for Master Controllers. Methods are available to get and
set the sub-controllers, get/set the number of sub-controllers, and add
sub-controllers.  */
class MasterPointControl : public Control {
   public:
	  // Set the number of sub-controllers
	  /*! \remarks Set the number of sub-controllers.
	  \par Parameters:
	  <b>int num</b>\n\n
	  The number of sub-controllers.\n\n
	  <b>BOOL keep=FALSE</b>\n\n
	  If the number if sub-controllers is reallocated this indicates if the
	  previously allocated ones are kept. TRUE to keep; FALSE to discard them.
	  \par Default Implementation:
	  <b>{}</b> */
	  virtual  void SetNumSubControllers(int num, BOOL keep=FALSE) {}
	  // Return the number of sub-controllers
	  /*! \remarks Return the number of sub-controllers.
	  \par Default Implementation:
	  <b>{ return NULL; }</b> */
	  virtual  int    GetNumSubControllers() { return 0; }
	  // Delete all the sub-controllers that are set to TRUE in the BitArray
	  /*! \remarks Delete all the sub-controllers that are set to TRUE in the
	  BitArray.
	  \par Parameters:
	  <b>BitArray set</b>\n\n
	  This BitArray has bits set for the sub-controllers to be deleted. It
	  should be <b>GetNumSubControllers()</b> in size.
	  \par Default Implementation:
	  <b>{}</b> */
	  virtual void DeleteControlSet (BitArray set) {}
	  // Add a new sub-controller
	  /*! \remarks Adds the specified sub-controller.
	  \par Parameters:
	  <b>Control* ctrl</b>\n\n
	  Points to the controller to add.
	  \return  Nonzero if implemented; otherwise zero.
	  \par Default Implementation:
	  <b>{ return 0; }</b> */
	  virtual int  AddSubController(Control* ctrl) { return 0; }
	  // Return i'th of sub-controller
	  /*! \remarks Return a pointer to the i-th sub-controller.
	  \par Parameters:
	  <b>int i</b>\n\n
	  The zero based index of the
	  \par Default Implementation:
	  <b>{ return NULL; }</b> */
	  virtual Control* GetSubController(int i) { return NULL; }
	  // Set the i'th sub-controller
	  /*! \remarks Set the I-th sub-controller.
	  \par Parameters:
	  <b>int i</b>\n\n
	  The zero based index of the sub-controller to set.\n\n
	  <b>Control* ctrl</b>\n\n
	  Points to the control to set.
	  \par Default Implementation:
	  <b>{}</b> */
	  virtual  void SetSubController(int i, Control* ctrl) {}
};

#pragma warning(pop) // C4100 C4239

//----------------------------------------------------------------//
//
// Some stuff to help with ORTs - these could actually be Interval methods

inline TimeValue CycleTime(Interval i,TimeValue t)
   {
   int res, dur = i.Duration()-1;
   if (dur<=0) return t;      
   res   = (t-i.Start())%dur;
   if (t<i.Start()) {
	  return i.End()+res;
   } else {
	  return i.Start()+res;
	  }
   }

inline int NumCycles(Interval i,TimeValue t)
   {
   int dur = i.Duration()-1;
   if (dur<=0) return 1;
   if (t<i.Start()) {
	  return (abs(t-i.Start())/dur)+1;
   } else 
   if (t>i.End()) {
	  return (abs(t-i.End())/dur)+1;
   } else {
	  return 0;
	  }
   }



// Types that use this template must support:
//  T + T, T - T, T * float, T + float 

template <class T> T LinearExtrapolate(TimeValue t0, TimeValue t1, T &val0, T &val1, T &endVal)
{
	return (T)(endVal + (val1-val0) * float(t1-t0));
}

template <class T> T RepeatExtrapolate(Interval range, 
									   TimeValue t, 
									   T& startVal, 
									   T& endVal, 
									   T& cycleVal)
{
	int cycles = NumCycles(range,t);
	T delta;
	if (t<range.Start()) {
		delta = startVal - endVal;
	} else {
		delta = endVal - startVal;
	}
	return (T)(cycleVal + delta * float(cycles));
}

template <class T> T IdentityExtrapolate(TimeValue endPoint, TimeValue t, T &endVal )
{
	return (T)(endVal + float(t-endPoint));
}

CoreExport Quat LinearExtrapolate(TimeValue t0, TimeValue t1, Quat &val0, Quat &val1, Quat &endVal);
CoreExport Quat RepeatExtrapolate(Interval range, TimeValue t, Quat &startVal, Quat &endVal, Quat &cycleVal);
CoreExport Quat IdentityExtrapolate(TimeValue endPoint, TimeValue t, Quat &endVal );

CoreExport ScaleValue LinearExtrapolate(TimeValue t0, TimeValue t1, ScaleValue &val0, ScaleValue &val1, ScaleValue &endVal);
CoreExport ScaleValue RepeatExtrapolate(Interval range, TimeValue t, ScaleValue &startVal, ScaleValue &endVal, ScaleValue &cycleVal);
CoreExport ScaleValue IdentityExtrapolate(TimeValue endPoint, TimeValue t, ScaleValue &endVal);


template <class T> T LinearInterpolate(const T& v0,const T& v1,float u)
{
	return (T)((1.0f-u)*v0 + u*v1);
}

inline Quat 
LinearInterpolate(const Quat& v0,const Quat& v1,float u)
   {
   return Slerp(v0,v1,u);
   }

inline ScaleValue 
LinearInterpolate(const ScaleValue &v0,const ScaleValue &v1,float u)
   {
   ScaleValue res;
   res.s = ((float)1.0-u)*v0.s + u*v1.s;
   res.q = Slerp(v0.q,v1.q,u);
   return res;
   }


inline Interval TestInterval(Interval iv, DWORD flags)
   {
   TimeValue start = iv.Start();
   TimeValue end = iv.End();
   if (!(flags&TIME_INCLEFT)) {
	  start++;
	  }  
   if (!(flags&TIME_INCRIGHT)) {
	  end--;
	  }
   if (end<start) {
	  iv.SetEmpty();
   } else {
	  iv.Set(start,end);
	  }
   return iv;  
   }

inline Quat ScaleQuat(Quat q, float s)
   {
   float angle;
   Point3 axis;
   AngAxisFromQ(q,&angle,axis);
   return QFromAngAxis(angle*s,axis);
   }

//-------------------------------------------------------------------
// A place to store values during Hold/Restore periods
//
//********************************************************
// TempStore:  This is a temporary implementation:
//  It uses a linear search-
//  A hash-coded dictionary would be faster.
//  (if there are ever a lot of entries)
//********************************************************

/*! \sa  Class TempStore.
\remarks This structure is used as part of a list of entries to 
store values into the TempStore.
*/
struct Slot: public MaxHeapOperators {
   void *key;
   void *pdata;
   int nbytes;
   Slot *next;
   public:
	  /*! \remarks Constructor. The data pointer is set to NULL. */
	  Slot() { pdata = NULL; }
	  /*! \remarks Destructor. If data is allocated it is freed. */
	  CoreExport ~Slot();

   };

/*! \sa  Structure Slot.\n\n
\par Description:
This class is used as a place to store values during Hold/Restore periods. All
methods of this class are implemented by the system.\n\n
<b>TempStore tmpStore;</b>\n\n
This is a global instance of <b>TempStore</b> whose methods may be called to
store the values. For instance:\n\n
<b>  tmpStore.PutBytes(sizeof(Quat), \&curval, this);</b>\n\n
Note that the above code uses the plug-in's <b>this</b> pointer as the key to
identify the owner of the data in the store.  */
class TempStore: public MaxHeapOperators {
   Slot *slotList;            
   Slot* Find(int n, void *data, void *ptr);
   public:
	  /*! \remarks Constructor. The list is set to NULL. */
	  TempStore() {  slotList = NULL;  }
	  /*! \remarks Destructor. This empties out the store. */
	  ~TempStore() { ClearAll(); }
	  /*! \remarks This empties out the store. */
	  CoreExport void ClearAll();   // empty out the store 
	  /*! \remarks This method puts bytes to the storage.
	  \par Parameters:
	  <b>int n</b>\n\n
	  The number of bytes to put.\n\n
	  <b>void *data</b>\n\n
	  Points to the data to put.\n\n
	  <b>void *ptr</b>\n\n
	  A key to identify the data in the store. */
	  CoreExport void PutBytes(int n, void *data, void *ptr);
	  /*! \remarks This method gets bytes from the storage.
	  \par Parameters:
	  <b>int n</b>\n\n
	  The number of bytes to get.\n\n
	  <b>void *data</b>\n\n
	  Points to the data to get.\n\n
	  <b>void *ptr</b>\n\n
	  A key to identify the data in the store. */
	  CoreExport void GetBytes(int n, void *data, void *ptr);
	  /*! \remarks Remove a single entry from the storage.
	  \par Parameters:
	  <b>void *ptr</b>\n\n
	  A key to identify the data in the store. */
	  CoreExport void Clear(void *ptr);  // Remove single entry
	  /*! \remarks Puts the specified floating point value to the storage.
	  \par Parameters:
	  <b>float f</b>\n\n
	  The value to store.\n\n
	  <b>void *ptr</b>\n\n
	  A key to identify the data in the store. */
	  void PutFloat(float  f, void *ptr) {
		  PutBytes(sizeof(float),(void *)&f,ptr);
		  }
	  /*! \remarks	  \par Parameters:
	  <b>int f</b>\n\n
	  The value to store.\n\n
	  <b>void *ptr</b>\n\n
	  A key to identify the data in the store. */
	  CoreExport void PutInt(int i, void *ptr) {
		  PutBytes(sizeof(int),(void *)&i,ptr);
		  }
	  /*! \remarks Retrieves a floating point value from the storage.
	  \par Parameters:
	  <b>float *f</b>\n\n
	  The value to retrieve is stored here.\n\n
	  <b>void *ptr</b>\n\n
	  A key to identify the data in the store. */
	  CoreExport void GetFloat(float *f, void *ptr) { 
		 GetBytes(sizeof(float),(void *)f,ptr);
		 }
	  /*! \remarks Retrieves an integer point value from the storage.
	  \par Parameters:
	  <b>int *f</b>\n\n
	  The value to retrieve is stored here.\n\n
	  <b>void *ptr</b>\n\n
	  A key to identify the data in the store. */
	  CoreExport void GetInt(int *i, void *ptr) { 
		 GetBytes(sizeof(int),(void *)i,ptr);
		 }
	  /*! \remarks Puts a <b>Point3</b> value to the storage.
	  \par Parameters:
	  <b>Point3 f</b>\n\n
	  The value to store.\n\n
	  <b>void *ptr</b>\n\n
	  A key to identify the data in the store. */
	  CoreExport void PutPoint3(Point3  f, void *ptr) {
		  PutBytes(sizeof(Point3),(void *)&f,ptr);
		  }
	  /*! \remarks Retrieves a Point3 value from the storage.
	  \par Parameters:
	  <b>Point3 *f</b>\n\n
	  The value to retrieve is stored here.\n\n
	  <b>void *ptr</b>\n\n
	  A key to identify the data in the store. */
	  CoreExport void GetPoint3(Point3 *f, void *ptr) { 
		 GetBytes(sizeof(Point3),(void *)f,ptr);
		 }
	  CoreExport void PutPoint4(Point4  f, void *ptr) {
		 PutBytes(sizeof(Point4),(void *)&f,ptr);
		 }
	  CoreExport void GetPoint4(Point4 *f, void *ptr) { 
		 GetBytes(sizeof(Point4),(void *)f,ptr);
		 }
	  /*! \remarks Puts a quaternion value to the storage.
	  \par Parameters:
	  <b>Quat f</b>\n\n
	  The quaternion to store.\n\n
	  <b>void *ptr</b>\n\n
	  A key to identify the data in the store. */
	  CoreExport void PutQuat( Quat  f, void *ptr) {
		  PutBytes(sizeof(Quat),(void *)&f,ptr);
		  }
	  /*! \remarks Retrieves a quaternion from the storage.
	  \par Parameters:
	  <b>Quat *f</b>\n\n
	  The quaternion retrieved is stored here.\n\n
	  <b>void *ptr</b>\n\n
	  A key to identify the data in the store. */
	  CoreExport void GetQuat( Quat *f, void *ptr) { 
		 GetBytes(sizeof(Quat),(void *)f,ptr);
		 }
	  /*! \remarks Puts a ScaleValue to the storage.
	  \par Parameters:
	  <b>ScaleValue f</b>\n\n
	  The value to store.\n\n
	  <b>void *ptr</b>\n\n
	  A key to identify the data in the store. */
	  CoreExport void PutScaleValue( ScaleValue  f, void *ptr) {
		  PutBytes(sizeof(ScaleValue),(void *)&f,ptr);
		  }
	  /*! \remarks Retrieves a ScaleValue from the storage.
	  \par Parameters:
	  <b>ScaleValue *f</b>\n\n
	  The retrieved value is stored here.\n\n
	  <b>void *ptr</b>\n\n
	  A key to identify the data in the store. */
	  CoreExport void GetScaleValue( ScaleValue *f, void *ptr) { 
		 GetBytes(sizeof(ScaleValue),(void *)f,ptr);
		 }
   };


extern CoreExport TempStore tmpStore;   // this should be in the scene data struct.


CoreExport int Animating();    // is the animate switch on??
CoreExport void AnimateOn();  // turn animate on
CoreExport void AnimateOff();  // turn animate off
CoreExport void SuspendAnimate(); // suspend animation (uses stack)
CoreExport void ResumeAnimate();   // resume animation ( " )

CoreExport BOOL AreWeAnimating(const TimeValue &t);
CoreExport BOOL AreWeKeying(const TimeValue &t);


CoreExport TimeValue GetAnimStart();
CoreExport TimeValue GetAnimEnd();
CoreExport void SetAnimStart(TimeValue s);
CoreExport void SetAnimEnd(TimeValue e);

CoreExport Control *NewDefaultFloatController();
CoreExport Control *NewDefaultPoint3Controller();
CoreExport Control *NewDefaultMatrix3Controller();
CoreExport Control *NewDefaultPositionController();
CoreExport Control *NewDefaultRotationController();
CoreExport Control *NewDefaultScaleController();
CoreExport Control *NewDefaultBoolController();
CoreExport Control *NewDefaultColorController();
CoreExport Control *NewDefaultMasterPointController();
CoreExport Control *NewDefaultPoint4Controller();
CoreExport Control *NewDefaultFRGBAController();
CoreExport Control *NewDefaultPoint2Controller();

CoreExport Control* CreateInterpFloat();
CoreExport Control* CreateInterpPosition();
CoreExport Control* CreateInterpPoint3();
CoreExport Control* CreateInterpRotation();
CoreExport Control* CreateInterpScale();
CoreExport Control* CreatePRSControl();
CoreExport Control* CreateLookatControl();
CoreExport Control* CreateMasterPointControl();
CoreExport Control* CreateInterpPoint4();
CoreExport Control* CreateInterpPoint2();

CoreExport void SetDefaultController(SClass_ID sid, ClassDesc *desc);
CoreExport ClassDesc *GetDefaultController(SClass_ID sid);

CoreExport void SetDefaultColorController(ClassDesc *desc);
CoreExport void SetDefaultFRGBAController(ClassDesc *desc);
CoreExport void SetDefaultBoolController(ClassDesc *desc);

CoreExport BOOL GetSetKeyMode();
CoreExport void SetSetKeyMode(BOOL onOff);

CoreExport void SuspendSetKeyMode();
CoreExport void ResumeSetKeyMode();
CoreExport BOOL GetSetKeySuspended();
CoreExport BOOL GetSetKeyModeStatus();

CoreExport BOOL IsSetKeyModeFeatureEnabled();

