/*******************************************************************
 *
 *    DESCRIPTION: euler.h
 *
 *    AUTHOR: Converted from Ken Shoemake's Graphics Gems IV code by Dan Silva
 *
 *    HISTORY:  converted 11/21/96
 *
 *              RB: This file provides only a subset of those
 *                  found in the original Graphics Gems paper.
 *                  All orderings are 'static axis'.
 *
 *******************************************************************/

#pragma once

#include "GeomExport.h"
#include "maxheap.h"
#include "matrix3.h"
#include "quat.h"

#define EULERTYPE_XYZ	0
#define EULERTYPE_XZY	1
#define EULERTYPE_YZX	2
#define EULERTYPE_YXZ	3
#define EULERTYPE_ZXY	4
#define EULERTYPE_ZYX	5
#define EULERTYPE_XYX	6
#define EULERTYPE_YZY	7
#define EULERTYPE_ZXZ	8

#define EULERTYPE_RF    16  // rotating frame (axes)  --prs.

GEOMEXPORT void QuatToEuler(const Quat &q, float *ang, int type, bool flag = false);	// flag added 001101  --prs.
GEOMEXPORT void EulerToQuat(float *ang, Quat &q,int type);
GEOMEXPORT void MatrixToEuler(const Matrix3 &mat, float *ang, int type, bool flag = FALSE);
GEOMEXPORT void EulerToMatrix(float *ang, Matrix3 &mat, int type);
GEOMEXPORT float GetEulerQuatAngleRatio(Quat &quat1,Quat &quat2, float *euler1, float *euler2, int type = EULERTYPE_XYZ);
GEOMEXPORT float GetEulerMatAngleRatio(Matrix3 &mat1,Matrix3 &mat2, float *euler1, float *euler2, int type = EULERTYPE_XYZ);
GEOMEXPORT void ContinuousQuatToEuler(const Quat& quat, float ang[3], int order);

/*! <b>This class is only available in release 5 or later.</b>\n\n
This class is to hold different representations of the rotation. In particular,
it holds rotation value as represented by Euler angles or quaternion. Different
types of rotation controllers may use different representations. To avoid
losing information due to converting from one representation to another, we can
use RotationValue to hold the result.\n\n
For example, the Skin pose feature reads rotation of a node from the rotation
controller and stores the result in RotationValue (c.f.
maxsdk/include/iSkinPose.h).\n\n
<b>// Assuming node is a valid pointer to INode.</b>\n\n
<b>ISkinPose* skpose = ISkinPose::GetISkinPose(*node);</b>\n\n
<b>// skpose should not be null for Max version 5.0 or later.</b>\n\n
<b>RotationValue rv = skpose-\>SkinRot();</b>\n\n
It is guaranteed that rv keeps the original representation of the
controller.\n\n
Being asked of Euler angles, RotationValue will return 3 float numbers in the
format of Point3. There must be an association between numbers and axes.\n\n
There are two classes of Euler angle types. In one class, the rotation axes are
not repeated (non-repetitive). They are enum's from kXYZ to kZYX. In the other
class, one of the rotation axes is repeated (repetitive). They are enum's from
kXYX to kZXZ. For convenience, enum kReptd is used to denote the starting one:
kRept == kXYX.\n\n
For non-repetitive Euler angles, there are two well-defined methods to
associate three ordered angles, to three axes.\n\n
First, we can associate angles with x-, y-, and z-, axes, respectively. The
first angle, for example, is always associated with the x-axis, no matter where
it appears in the Euler order. Suppose\n\n
<b>Point3 a(0.1, 0.2, 0.3)</b>\n\n
then a.x (==0.1), a.y(==0.2), a.z (==0.3), are the angles of the x-axis,
y-axis, and z-axis, respectively, no matter whether the order (type) of the
Euler angles is kXYZ or kZXY.\n\n
Let's call this way of association by axis (name).\n\n
Second, we can associate them by position: the first angle, from left, is
always associated with the first axis in the Euler angle order. For examples,
the first angle is applied to the x-axis for kXYZ and kXZY, but to the y-axis
for kYXZ and kYZX, etc. Suppose a is a Point3, a[0] (==a.x), a[1] (==a.y), a[2]
(==a.z), are the angles of the z-axis, x-axis, and y-axis, respectively, for
Euler type kZXY.\n\n
Let's call this way of association by order.\n\n
For repetitive Euler type, the association by axis is ambiguous because one
axis may appear twice in the Euler axes. In this case, "by order" is well
defined.\n\n
This class uses the association of by axis for non-repetitive types and by
order for repetitive type. Suppose,\n\n
<b>Point3 a = rv.Euler(RotationValue::kZXZ) // repetitive Euler type</b>\n\n
Then, a[0] and a[2] are both applied to the Z axis, but a[0] corresponds to the
first z-axis from left, a[2] corresponds to the second z-axis (third axis) from
left, and a[1] corresponds to the x-axis.\n\n
   */
class RotationValue: public MaxHeapOperators {
//
// There are two classes of Euler angle types, that of which rotation
// axes are not repeated, from kXYZ to kZYX, and one of which an axis is
// repeated, from kXYX to kZXZ. Repeated types starts from kReptd.
// For non-repetitive Euler angles, there are two well-defined methods
// to associate three ordered angles, to axes. First, we can associate
// them with x-, y-, and z-, axes in order. The first angle, for example,
// is always associated with the x-axis, no matter where it appears
// in the Euler type (x-axis appears at second place in kZXY, for example).
// Second, we can associate them by position: the first angle is always
// associated with the first axis in the Euler type. For examples,
// the first angle is applied to the x-axis for kXYZ and kXZY, to the
// y-axis for kYXZ and kYZX, etc.
// Let's call the first method by (axis) name, and the second method
// by order. We associate angles by name for non-repetitive Euler types.
// For repetitive types, by-name is not well-defined, because there is
// a missing axis. For repetitive types, we associate angles by-order.
// Example:
//   Point3 a = Euler(RotationValue::kZYX);
// then, a.x, a.y, a.z, are the Euler angles for the x-axis, y-axis,
// and z-axis.
//   Point3 a = Euler(RotationValue::kZXZ);
// then, a.x is angle applied to the first z-axis (from left), a.y is
// applied to the x-axis, and a.z is applied to the second z-axis.
//
public:
	enum EulerType {
		kXYZ = EULERTYPE_XYZ,
		kXZY,
		kYZX,
		kYXZ,
		kZXY,
		kZYX,
		kXYX,
		kYZY,
		kZXZ
	};
	enum {
		kReptd = kXYX,
		kQuat = 100
	};
	/*! \remarks Conveniency (static) method to test whether an integer
	corresponds to an enum of rotation representations used in RotationValue of
	type Euler angles\n\n
	  */
	static bool IsEuler(int rep) { return (kXYZ <= rep && rep <= kZXZ); }
	/*! \remarks Conveniency (static) method to test whether in cases where a
	call to <b>IsEuler(int rep)</b> is made, and the return value is of type
	Euler angles, this method call returns whether it has repetitive axes (such
	as XYX).\n\n
	  */
	static bool IsRepetitive(int rep) {
		// Pre-cond: IsEuler(rep)
		return rep >= kReptd; }
	/*! \remarks Conveniency (static) method to test whether an integer
	corresponds to an enum of rotation representations used in RotationValue of
	type Quaternion.\n\n
	  */
	static bool IsQuat(int rep) { return rep == kQuat; }

	/*! \remarks Set an object of RotationValue to an Euler angle
	representation. Angles are assumed in radians. To set to Euler angles of x,
	y, z, of order XYZ, do, for example:\n\n
	<b>RotationValue a;</b>\n\n
	<b>a.Set(Point3(x, y, z), RotationValue::kXYZ);</b>\n\n
	  */
	void Set(const Point3& a, EulerType et) {
		mQ.x = a.x;
		mQ.y = a.y;
		mQ.z = a.z;
		mRep = (short)et; }
	/*! \remarks Set an object of RotationValue to a quaternion
	representation.\n\n
	  */
	void Set(const Quat& q) {
		mQ = q;
		mRep = kQuat; }
	/*! \remarks Constructor */
	RotationValue() : mQ(), mRep(kXYZ) {}
	/*! \remarks Constructor */
	RotationValue(const Point3& a, EulerType et) { Set(a, et); }
	/*! \remarks K Prototype\n\n
	Constructor. */
	RotationValue(const Quat& q) { Set(q); }
	/*! \remarks Copy constructor. */
	RotationValue(const RotationValue& src) : mQ(src.mQ), mRep(src.mRep) {}

	/*! \remarks Used to get the rotation in specific representation. Suppose
	rv is a RotationValue, to get it in terms of Euler angles of order XYZ:\n\n
	<b>rv.Euler();</b>\n\n
	in order ZXY:\n\n
	<b>rv.Euler(RotationValue::kZXY);</b>\n\n
	or, to get it in quaternion:\n\n
	<b>(Quat)rv;</b>\n\n
	to get it in matrix form:\n\n
	<b>(Matrix3)rv</b> */
	Point3 Euler(EulerType et =kXYZ) const {
		if (et == mRep) return Point3(mQ.x, mQ.y, mQ.z);
		else return ToEulerAngles(et); }
	/*! \remarks Please see method Euler() for fuller explanation. */
	operator Quat() const {
		if (mRep == kQuat) return mQ;
		else return ToQuat(); }
	/*! \remarks Please see method Euler() for fuller explanation. */
	GEOMEXPORT operator Matrix3() const;
	/*! \remarks Given a matrix, m, we can apply the rotation, rv, of
	RotationValue from left side (PreApplyTo)\n\n
	 <b>rv.PreApplyTo(m) == ((Matrix3)rv) * m</b> */
	GEOMEXPORT void PreApplyTo(Matrix3& m) const; // m = *this * m
	/*! \remarks Given a matrix, m, we can apply the rotation, rv, of
	RotationValue from right side (PoseApplyTo)\n\n
	 <b>rv.PoseApplyTo(m) == m * (Matrix3)rv)</b> */
	GEOMEXPORT void PostApplyTo(Matrix3& m) const; // m = m * *this
	// *this = *this * aa
	// Post-condition: NativeRep() will be changed after RotateBy()
	/*! \remarks To apply a rotation, aa, as represented as AngAxis to a
	RotationValue, rv, from the right side,\n\n
	<b>rv.PostRotate(aa)</b>\n\n
	The internal representation of rv after applying to it will not be change.
	Mathematically,\n\n
	<b>(Matrix3)rv.PostRotate(aa) == ((Matrix3)rv) * MatrixOf(aa)</b>\n\n
	If rv is in Euler angles, this method will try to keep the Euler angles
	from jumping at the borders of (+/-)180 degrees. */
	GEOMEXPORT void PostRotate(const AngAxis& aa);

	/*! \remarks Used to get the internal representation and returns the
	representation type. If it is a Euler angle type, the first three numbers
	of the Quat returned from GetNative() are to be interpreted as Euler
	angles. */
	int		NativeRep() const { return mRep; }
	/*! \remarks Used to get the internal representation and returns the
	actual float numbers. */
	Quat	GetNative() const { return mQ; }

	// Suppose a and o are Point3's, which holds Euler angles by axis name
	// and by order, respectively. That is, a[0] is applied to the x-axis and
	// o[0] is applied to the first axis (from left in the Euler type).
	// Let et be non-repetitive Euler type. Then,
	//		o[kAxisToOrdinal[et][i]] = a[i]
	// and	a[kOrdinalToAxis[et][i]] = o[i]
	//
	static const int kAxisToOrdinal[kReptd][3];
	static const int kOrdinalToAxis[kZXZ+1][3];

protected:
	GEOMEXPORT Point3	ToEulerAngles(EulerType et) const;
	GEOMEXPORT Quat	ToQuat() const;

private:
	Quat	mQ;
	short	mRep;
};

