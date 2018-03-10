/**********************************************************************
 *<
   FILE: quat.h

   DESCRIPTION: Class definitions for Quat

   CREATED BY: Dan Silva

   HISTORY:

 *>   Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "GeomExport.h"
#include "maxheap.h"
#include "matrix3.h"
#include "assert1.h"

#include <iosfwd>

class Quat;
class AngAxis;

/*! \sa  Class Quat, Class Point3.\n\n
\par Description:
This class provides a representation for orientation in three space using an
angle and axis. This class is similar to a quaternion, except that a normalized
quaternion only represents -PI to +PI rotation. This class will have the number
of revolutions stored. All methods of this class are implemented by the
system.\n\n
The rotation convention in the 3ds Max API is the left-hand-rule. Note that
this is different from the right-hand-rule used in the 3ds Max user interface.
\par Data Members:
<b>Point3 axis;</b>\n\n
The axis of rotation.\n\n
<b>float angle;</b>\n\n
The angle of rotation about the axis in radians. This angle is left handed.
 */
class AngAxis: public MaxHeapOperators {
public:
   Point3 axis;
   float angle;        // This angle is left-handed!

   /*! \remarks Constructor. No initialization is performed. */
   AngAxis() { /* NO INIT */ }
   /*! \remarks Constructor. The AngAxis is initialized from the specified values.
   \par Parameters:
   <b>float x</b>\n\n
   The x component of the axis.\n\n
   <b>float y</b>\n\n
   The y component of the axis.\n\n
   <b>float z</b>\n\n
   The z component of the axis.\n\n
   <b>float ang</b>\n\n
   The angle component in radians. */
   AngAxis(float x, float y, float z, float ang)
      { axis.x = x; axis.y = y; axis.z = z; angle = ang; }
   /*! \remarks Constructor. Data members are initialized to the specified
   values. */
   AngAxis(const Point3& axis,float angle) { this->axis=axis; this->angle=angle; }  
   /*! \remarks Constructor. Data members are initialized equal to the
   specified Quat. */
   GEOMEXPORT AngAxis(const Quat &q);
   /*! \remarks Constructor. The AngAxis is initialized with the rotation from the specified
   matrix.
   \par Parameters:
   <b>const Matrix3\& m</b>\n\n
   The rotation used to initialize the AngAxis. */
   GEOMEXPORT AngAxis(const Matrix3& m); // GEOMEXPORT added in R5.1

   /*! \remarks Sets the angle and axis to the specified values.
   \par Parameters:
   <b>float x</b>\n\n
   Specifies the x component of the axis.\n\n
   <b>float y</b>\n\n
   Specifies the xycomponent of the axis.\n\n
   <b>float z</b>\n\n
   Specifies the z component of the axis.\n\n
   <b>float ang</b>\n\n
   Specifies the angle to set in radians.
   \return  A reference to this AngAxis. */
   AngAxis& Set(float x, float y, float z, float ang)
      {axis.x = x; axis.y = y; axis.z = z; angle = ang; return *this; }
   /*! \remarks Sets the angle and axis to the specified values.
   \par Parameters:
   <b>const Point3\& ax</b>\n\n
   Specifies the axis to set.\n\n
   <b>float ang</b>\n\n
   Specifies the angle to set in radians.
   \return  A reference to this AngAxis. */
   AngAxis& Set(const Point3& ax, float ang)
      {axis = ax; angle = ang; return *this; }
   /*! \remarks Sets the angle and axis based on the rotations from the specified
   quaternion.
   \par Parameters:
   <b>const Quat\& q</b>\n\n
   Specifies the angle and axis to use.
   \return  A reference to this AngAxis. */
   GEOMEXPORT AngAxis& Set(const Quat& q);
   /*! \remarks Sets the angle and axis based on the rotations from the specified matrix.
   \par Parameters:
   <b>const Matrix3\& m</b>\n\n
   Specifies the angle and axis to use.
   \return  A reference to this AngAxis. */
   GEOMEXPORT AngAxis& Set(const Matrix3& m);
    
   /*! \remarks Returns the number of revolutions represented by the angle.
   This returns <b>int(angle/TWOPI);</b> */
   GEOMEXPORT int GetNumRevs();
   /*! \remarks Sets the number of revolution to <b>num</b>. This modifies
   angle: <b>angle += float(num)*TWOPI;</b> */
   GEOMEXPORT void SetNumRevs(int num);
   };

/*! \sa  Class Point3, Class Matrix3, Class AngAxis.\n\n
\par Description:
This class provides a compact representation for orientation in three space and
provides methods to perform Quaternion algebra.\n\n
Quaternions provide an alternative representation for orientation in
three-space. To reduce computing, you can substitute quaternion multiplication
for rotation-matrix composition.\n\n
A quaternion is made up of four terms: a real scalar part which specifies the
amount of rotation and an imaginary vector part which defines the axis of
rotation. If the quaternion is normalized, the scalar term equals the cosine of
half the angle of rotation, the vector term is the axis of rotation, and the
magnitude of the vector term equals the sine of half the angle of rotation.\n\n
Interpolation between two key frame orientations is much easier using
quaternions and produces smooth and natural motion. Unlike Euler angles, no
numerical integration is necessary; quaternions provide an analytic result (no
approximations).\n\n
The rotation convention in the 3ds Max API is the left-hand-rule. Note that
this is different from the right-hand-rule used in the 3ds Max user
interface.\n\n
For additional information see: Quaternion operations:\n\n
From "Quaternion Calculus and Fast Animation",\n\n
by Ken Shoemake, in notes for SIGGRAPH 1987 Course # 10,\n\n
"Computer Animation: 3-D Motion Specification and Control".\n\n
All methods of this class are implemented by the system.
\par Data Members:
<b>float x,y,z,w;</b>\n\n
The <b>x</b>, <b>y</b>, <b>z</b> values make up the vector portion. <b>w</b> is
the angle of rotation about the vector (see remarks above for details).  */
class Quat: public MaxHeapOperators {
public:
   float x,y,z,w;

   // Constructors
   /*! \remarks Constructor. No initialization is performed. */
   Quat(): x(0.0f),y(0.0f),z(0.0f),w(1.0f) {}
   /*! \remarks Constructor. The data members are initialized to the values
   passed. */
   Quat(float X, float Y, float Z, float W)  { x = X; y = Y; z = Z; w = W; }
   /*! \remarks Constructor. The data members are initialized to the values
   passed (cast as floats). */
   Quat(double X, double Y, double Z, double W)  { 
      x = (float)X; y = (float)Y; z = (float)Z; w = (float)W;  
      }
   /*! \remarks Constructor. The data members are initialized to the Quat
   passed. */
   Quat(const Quat& a) { x = a.x; y = a.y; z = a.z; w = a.w; } 
   /*! \remarks Constructor. The data members are initialized to the values
   passed.\n\n
   <b>x = af[0]; y = af[1]; z = af[2]; w = af[3];</b> */
   Quat(float af[4]) { x = af[0]; y = af[1]; z = af[2]; w = af[3]; }
   /*! \remarks Constructor. Convert the specified 3x3 rotation matrix to a
   unit quaternion. */
   GEOMEXPORT Quat(const Matrix3& mat);
   /*! \remarks Constructor. The Quat is initialized to the AngAxis passed. */
   GEOMEXPORT Quat(const AngAxis& aa);
   /*! \remarks Constructor. The quaternion is initialized from the vector <b>V</b> and
   angle <b>W</b> passed. The quaternion is then normalized.\n\n
     */
   GEOMEXPORT Quat(const Point3& V, float W);

   // Access operators
   /*! \remarks Array access operator. Valid <b>i</b> values: 0=x, 1=y, 2=z,
   3=w. */
   float& operator[](int i) { return (&x)[i]; }     
   /*! \remarks Array access operator. Valid <b>i</b> values: 0=x, 1=y, 2=z,
   3=w. */
   const float& operator[](int i) const { return (&x)[i]; }
    
   float Scalar() { return w; }
   Point3 Vector() { return Point3(x, y, z); }

   // Conversion function
   /*! \remarks Returns the address of the Quaternion.\n\n
   Unary operators */
   operator float*() { return(&x); }

   // Unary operators
   /*! \remarks Unary negation. Returns <b>Quat(-x,-y,-z,-w)</b>. */
   Quat operator-() const { return(Quat(-x,-y,-z,-w)); } 
   /*! \remarks Unary +. Returns the Quat unaltered.\n\n
   Assignment operators */
   Quat operator+() const { return *this; }
    
    // Math functions
   /*! \remarks Returns the inverse of this quaternion (1/q). */
   GEOMEXPORT Quat Inverse() const;
   /*! \remarks Returns the conjugate of a quaternion. */
   GEOMEXPORT Quat Conjugate() const;
   /*! \remarks Returns the natural logarithm of a UNIT quaternion. */
   GEOMEXPORT Quat LogN() const;
   /*! \remarks Returns the exponentiate quaternion (where <b>q.w</b>==0).
   \par Operators:
   */
   GEOMEXPORT Quat Exp() const;

   // Assignment operators
   /*! \remarks This operator is the same as the <b>/=</b> operator. */
   GEOMEXPORT Quat& operator-=(const Quat&);
   /*! \remarks This operator is the same as the <b>*=</b> operator.. */
   GEOMEXPORT Quat& operator+=(const Quat&);
   /*! \remarks Multiplies this quaternion by a quaternion. */
   GEOMEXPORT Quat& operator*=(const Quat&);
   /*! \remarks Multiplies this quaternion by a floating point value. */
   GEOMEXPORT Quat& operator*=(float);
   /*! \remarks Divides this quaternion by a floating point value. */
   GEOMEXPORT Quat& operator/=(float);

   Quat& Set(float X, float Y, float Z, float W)
      { x = X; y = Y; z = Z; w = W; return *this; }
   Quat& Set(double X, double Y, double Z, double W)
      { x = (float)X; y = (float)Y; z = (float)Z; w = (float)W;
        return *this; }
   GEOMEXPORT Quat& Set(const Matrix3& mat);
   GEOMEXPORT Quat& Set(const AngAxis& aa);
   Quat& Set(const Point3& V, float W)
      { x = V.x; y = V.y; z = V.z; w = W; return *this; } 
   GEOMEXPORT Quat& SetEuler(float X, float Y, float Z);
   GEOMEXPORT Quat& Invert();                 // in place

   /*! \remarks Modifies <b>q</b> so it is on same side of hypersphere as
   <b>qto</b>. */
   GEOMEXPORT Quat& MakeClosest(const Quat& qto);

   // Comparison
   /*! \remarks Returns nonzero if the quaternions are equal; otherwise 0. */
   GEOMEXPORT int operator==(const Quat& a) const;
   GEOMEXPORT int Equals(const Quat& a, float epsilon = 1E-6f) const;

   /*! \remarks Sets this quaternion to the identity quaternion (<b>x=y=z=0.0;
   w=1.0</b>). */
   void Identity() { x = y = z = 0.0f; w = 1.0f; }
   /*! \remarks Returns nonzero if the quaternion is the identity; otherwise
   0. */
   GEOMEXPORT int IsIdentity() const;
   /*! \remarks Normalizes this quaternion, dividing each term by a scale
   factor such that the resulting sum or the squares of all parts equals unity.
   */
   GEOMEXPORT void Normalize();  // normalize
   /*! \remarks Converts the quaternion to a 3x3 rotation matrix. The
   quaternion need not be unit magnitude.
   \par Parameters:
   <b>Matrix3 \&mat</b>\n\n
   The matrix.\n\n
   <b>BOOL b=FALSE</b>\n\n
   This parameter is available in release 4.0 and later only.\n\n
   When this argument is set to false (or omitted), each function performs as
   it did before version 4.0. When the boolean is TRUE, the matrix is made with
   its terms transposed. When this transposition is specified,
   <b>EulerToQuat()</b> and <b>QuatToEuler()</b> are consistent with one
   another. (In 3ds Max 3, they have opposite handedness). */
   GEOMEXPORT void MakeMatrix(Matrix3 &mat, bool flag=false) const;   // flag added 001031  --prs.
   GEOMEXPORT void GetEuler(float *X, float *Y, float *Z) const;

   // Binary operators
   /*! \remarks This operator is the same as the <b>/</b> operator. */
   GEOMEXPORT Quat operator-(const Quat&) const;  //RB: Changed these to    // difference of two quaternions
   /*! \remarks This operator is the same as the <b>*</b> operator. */
   GEOMEXPORT Quat operator+(const Quat&) const;  // duplicate * and /         // sum of two quaternions
   /*! \remarks Returns the product of two quaternions. */
   GEOMEXPORT Quat operator*(const Quat&) const;  // product of two quaternions
   /*! \remarks Returns the ratio of two quaternions: This creates a result
   quaternion r = p/q, such that q*r = p. (Order of multiplication is
   important) */
   GEOMEXPORT Quat operator/(const Quat&) const;  // ratio of two quaternions
   GEOMEXPORT float operator%(const Quat&) const;   // dot product
   GEOMEXPORT Quat Plus(const Quat&) const;       // what + should have done
   GEOMEXPORT Quat Minus(const Quat&) const;      // what - should have done
    };

/*! \remarks Multiplies the quaternion by a scalar. */
GEOMEXPORT Quat operator*(float, const Quat&);   // multiply by scalar
/*! \remarks Multiplies the quaternion by a scalar. */
GEOMEXPORT Quat operator*(const Quat&, float);   // multiply by scalar
/*! \remarks Divides the quaternion by a scalar. */
GEOMEXPORT Quat operator/(const Quat&, float);   // divide by scalar
/*! \remarks Returns the inverse of the quaternion (1/<b>q</b>). */
GEOMEXPORT Quat Inverse(const Quat& q);  // Inverse of quaternion (1/q)
/*! \remarks Returns the conjugate of a quaternion. */
GEOMEXPORT Quat Conjugate(const Quat& q); 
/*! \remarks Returns the natural logarithm of UNIT quaternion. */
GEOMEXPORT Quat LogN(const Quat& q);
/*! \remarks Exponentiate quaternion (where <b>q.w</b>==0). */
GEOMEXPORT Quat Exp(const Quat& q);
/*! \remarks Spherical linear interpolation of UNIT quaternions.\n\n
As t goes from 0 to 1, qt goes from p to q.\n\n
slerp(p,q,t) = (p*sin((1-t)*omega) + q*sin(t*omega)) / sin(omega) */
GEOMEXPORT Quat Slerp(const Quat& p, const Quat& q, float t);
/*! \remarks Computes the "log difference" of two quaternions, <b>p</b> and
<b>q</b>, as <b>ln(qinv(p)*q)</b>. */
GEOMEXPORT Quat LnDif(const Quat& p, const Quat& q);
/*! \remarks Compute a, the term used in Boehm-type interpolation.\n\n
<b>a[n] = q[n]* qexp(-(1/4)*( ln(qinv(q[n])*q[n+1]) +ln( qinv(q[n])*q[n-1]
)))</b> */
GEOMEXPORT Quat QCompA(const Quat& qprev,const Quat& q, const Quat& qnext);
/*! \remarks Squad(p,a,b,q; t) = Slerp(Slerp(p,q;t), Slerp(a,b;t); 2(1-t)t).
*/
GEOMEXPORT Quat Squad(const Quat& p, const Quat& a, const Quat &b, const Quat& q, float t); 
/*! \remarks Rotate <b>p</b> by 90 degrees (quaternion space metric) about the
specified <b>axis</b>. */
GEOMEXPORT Quat qorthog(const Quat& p, const Point3& axis);

/*! \remarks Quaternion interpolation for angles \> 2PI.
\par Parameters:
<b>float angle</b>\n\n
Angle of rotation\n\n
<b>const Point3\& axis</b>\n\n
The axis of rotation\n\n
<b>const Quat\& p</b>\n\n
Start quaternion\n\n
<b>const Quat\& a</b>\n\n
Start tangent quaternion\n\n
<b>const Quat\& b</b>\n\n
End tangent quaternion\n\n
<b>const Quat\& q</b>\n\n
End quaternion\n\n
<b>float t</b>\n\n
Parameter, in range [0.0,1.0] */
GEOMEXPORT Quat squadrev(
      float angle,   // angle of rotation 
      const Point3& axis,  // the axis of rotation 
      const Quat& p,    // start quaternion 
      const Quat& a,       // start tangent quaternion 
      const Quat& b,       // end tangent quaternion 
      const Quat& q,    // end quaternion 
      float t     // parameter, in range [0.0,1.0] 
      );

/*! \remarks Converts the quaternion to a matrix and multiples it by the
specified matrix. The result is returned in <b>mat</b>. */
GEOMEXPORT void RotateMatrix(Matrix3& mat, const Quat& q);     
/*! \remarks Converts the quaternion to a matrix and multiples it on the left
by the specified matrix. . The result is returned in <b>mat</b>. */
GEOMEXPORT void PreRotateMatrix(Matrix3& mat, const Quat& q);
/*! \remarks Converts the [angle,axis] representation to the equivalent
quaternion. */
GEOMEXPORT Quat QFromAngAxis(float ang, const Point3& axis);
/*! \remarks Converts the quaternion to the equivalent [angle,axis]
representation. */
GEOMEXPORT void AngAxisFromQ(const Quat& q, float *ang, Point3& axis);
/*! \remarks Compute the [angle,axis] corresponding to the rotation from
<b>p</b> to <b>q</b>. Returns angle, sets axis. */
GEOMEXPORT float QangAxis(const Quat& p, const Quat& q, Point3& axis);
/*! \remarks Decomposes a matrix into a rotation, scale, and translation (to
be applied in that order). This only will work correctly for scaling which was
applied in the rotated axis system. For more general decomposition see the
function <b>decomp_affine()</b>. See
Structure AffineParts. */
GEOMEXPORT void DecomposeMatrix(const Matrix3& mat, Point3& p, Quat& q, Point3& s);
/*! \remarks Returns the transformation of the specified quaternion by the
specified matrix. */
GEOMEXPORT Quat TransformQuat(const Matrix3 &m, const Quat&q );
/*! \remarks Returns the identity quaternion (<b>Quat(0.0,0.0,0.0,1.0)</b>).
*/
inline Quat IdentQuat() { return(Quat(0.0,0.0,0.0,1.0)); }

// Assumes Euler angles are of the form:
// RotateX(ang[0])
// RotateY(ang[1])
// RotateZ(ang[2])
//
/*! \remarks Converts the quaternion to Euler angles. When converting a
quaternion to Euler angles using this method, the correct order of application
of the resulting three rotations is X, then Y, then Z. The angles are returned
as <b>ang[0]=x, ang[1]=y, ang[2]=z</b>. */
GEOMEXPORT void QuatToEuler(Quat &q, float *ang);
/*! \remarks Converts Euler angles to a quaternion. The angles are specified
as <b>ang[0]=x, ang[1]=y, ang[2]=z</b>. This method is implemented as:\n\n
\code
void EulerToQuat(float *ang, Quat \&q, int order) {
	Matrix3 mat(1);</b>\n\n
	for (int i=0; i\<3; i++) {
		switch (orderings[order][i]) {
			case 0: mat.RotateX(ang[i]); break;
			case 1: mat.RotateY(ang[i]); break;
			case 2: mat.RotateZ(ang[i]); break;
		}
	}
	q = Quat(mat);
}
\endcode
*/
GEOMEXPORT void EulerToQuat(float *ang, Quat &q);

GEOMEXPORT std::ostream &operator<<(std::ostream&, const Quat&);

