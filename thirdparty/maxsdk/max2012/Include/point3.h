/**********************************************************************
 *<
   FILE: point3.h

   DESCRIPTION: Class definitions for Point3

   CREATED BY: Dan Silva

   HISTORY:

 *>   Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "GeomExport.h"
#include "maxheap.h"
#include "gfloat.h"
#include "assert1.h"
#include <math.h>

/*! \sa  Class IPoint3, Class DPoint3, Class Matrix3.\n\n
\par Description:
This class describes a 3D point using float x, y and z coordinates. Methods are
provided to add and subtract points, multiply and divide by scalars, and
element by element multiply and divide two points.\n\n
This class is also frequently used to simply store three floating point values
that may not represent a point. For example, a color value where x=red,
y=green, and z=blue. For color, the range of values is 0.0 to 1.0, where 0 is 0
and 1.0 is 255. All methods are implemented by the system.\n\n
Note: In 3ds Max, all vectors are assumed to be row vectors. Under this
assumption, multiplication of a vector with a matrix can be written either way
(Matrix*Vector or Vector*Matrix), for ease of use, and the result is the same
-- the (row) vector transformed by the matrix.
\par Data Members:
<b>float x, y, z;</b>\n\n
The x, y and z components of the point.\n\n
<b>static const Point3 Origin;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
This is equivalent to <b>Point3(0.0f, 0.0f, 0.0f);</b>\n\n
<b>static const Point3 XAxis;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
This is equivalent to <b>Point3(1.0f, 0.0f, 0.0f);</b>\n\n
<b>static const Point3 YAxis;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
This is equivalent to <b>Point3(0.0f, 1.0f, 0.0f);</b>\n\n
<b>static const Point3 ZAxis;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
This is equivalent to <b>Point3(0.0f, 0.0f, 1.0f);</b>  */
class GEOMEXPORT Point3: public MaxHeapOperators {
public:
   float x,y,z;

   // Constructors
   /*! \remarks Constructor. No initialization is performed. */
   Point3() { /* NO INIT */ }
   /*! \remarks Constructor. x, y, and z are initialized to the values specified. */
   Point3(float X, float Y, float Z)  { 
		 x = X; y = Y; z = Z; 
	 }
   /*! \remarks Constructor. x, y, and z are initialized to the specified values (cast as floats). */
   Point3(double X, double Y, double Z) { 
		 x = (float)X; y = (float)Y; z = (float)Z; 
	 }
   /*! \remarks Constructor. x, y, and z are initialized to the specified values (cast as floats). */
   Point3(int X, int Y, int Z) { 
		 x = (float)X; y = (float)Y; z = (float)Z; 
	 }
   /*! \remarks Constructor. x, y, and z are initialized to the specified Point3. */
   Point3(const Point3& a) { 
		 x = a.x; y = a.y; z = a.z; 
	 } 
   /*! \remarks Constructor. x, y, and z are initialized to af[0], af[1], and af[2] respectively.
   */
   Point3(float af[3]) { 
		 x = af[0]; y = af[1]; z = af[2]; 
	 }

    // Data members
    static const Point3 Origin;
    static const Point3 XAxis;
    static const Point3 YAxis;
    static const Point3 ZAxis;

	/*! \remarks Allows access to x, y and z using the subscript operator.
	\return  An value for <b>i</b> of 0 will return x, 1 will return y, 2 will
	return z. */
   float& operator[](int i) { 
		 return (&x)[i];
	 }     
	/*! \remarks Allows access to x, y and z using the subscript operator.
	\return  An value for <b>i</b> of 0 will return x, 1 will return y, 2 will
	return z. */
   const float& operator[](int i) const { 
		 return (&x)[i]; 
	 }  

   // Conversion function
   /*! \remarks Conversion function. Returns the address of the Point3.x */
   operator float*() { 
		 return(&x); 
	 }

   // Unary operators
   /*! \remarks Unary - operator. Negates x, y and z. */
   Point3 operator-() const { 
		 return(Point3(-x,-y,-z)); 
	 } 
   /*! \remarks Unary +. Returns the Point3. */
   Point3 operator+() const { 
		 return *this; 
	 }
    
    // Property functions
	/*! \remarks Returns the 'Length' of this point (vector). This is:\n\n
	<b>sqrt(v.x*v.x+v.y*v.y+v.z*v.z)</b> */
    float Length() const;
	 /*! \remarks Returns the 'Length' of this point (vector) using a faster
	 assembly language implementation for square root. This is:\n\n
	 <b>Sqrt(v.x*v.x+v.y*v.y+v.z*v.z)</b> */
    float FLength() const;
	 /*! \remarks The 'Length' squared of this point. This is
	 <b>v.x*v.x+v.y*v.y+v.z*v.z.</b> */
    float LengthSquared() const;
    int MaxComponent() const;
    int MinComponent() const;
    Point3 Normalize() const;     // more accurate than FNormalize()
    Point3 FNormalize() const;    // faster than Normalize()

   // Assignment operators
   /*! \remarks Subtracts a Point3 from this Point3. */
   inline Point3& operator-=(const Point3&);
   /*! \remarks Adds a Point3 to this Point3. */
   inline Point3& operator+=(const Point3&);
   /*! \remarks Multiplies this Point3 by a floating point value. */
   inline Point3& operator*=(float); 
   /*! \remarks Divides this Point3 by a floating point value. */
   inline Point3& operator/=(float);
   /*! \remarks Element-by-element multiplication of two Point3s:\n\n
   <b>(x*x, y*y, z*z)</b>. */
   inline Point3& operator*=(const Point3&); // element-by-element multiply.

    inline Point3& Set(float X, float Y, float Z);

   // Test for equality
   /*! \remarks Equality operator. Test for equality between two Point3's.
   \return  Nonzero if the Point3's are equal; otherwise 0. */
   int operator==(const Point3& p) const { 
		 return ((p.x==x)&&(p.y==y)&&(p.z==z)); 
	 }
   int operator!=(const Point3& p) const { 
		 return ((p.x!=x)||(p.y!=y)||(p.z!=z)); 
	 }
    int Equals(const Point3& p, float epsilon = 1E-6f) const;

    // In-place normalize
    Point3& Unify();
    float LengthUnify();              // returns old Length

   // Binary operators
   /*! \remarks Subtracts a Point3 from a Point3. */
   inline  Point3 operator-(const Point3&) const;
   /*! \remarks Adds a Point3 to a Point3. */
   inline  Point3 operator+(const Point3&) const;
   /*! \remarks Divides a Point3 by a Point3 element by element. */
   inline  Point3 operator/(const Point3&) const;
   /*! \remarks Multiplies a Point3 by a Point3 element by element.\n\n
   <b>(x*x, y*y, z*z)</b>. */
   inline  Point3 operator*(const Point3&) const;   

   /*! \remarks The cross product of two Point3's (vectors).
   \return  The cross product of two Point3's. */
   Point3 operator^(const Point3&) const; // CROSS PRODUCT
   inline float operator%(const Point3&) const;     // DOT PRODUCT
   };

/*! \remarks Returns the 'Length' of the point (vector). This is:\n\n
<b>sqrt(v.x*v.x+v.y*v.y+v.z*v.z)</b> */
GEOMEXPORT float Length(const Point3&); 
/*! \remarks Returns the 'Length' of the point (vector) using a faster
assembly language implementation for square root. This is:\n\n
<b>Sqrt(v.x*v.x+v.y*v.y+v.z*v.z)</b> */
GEOMEXPORT float FLength(const Point3&); 
/*! \remarks The 'Length' squared of the point. This is
<b>v.x*v.x+v.y*v.y+v.z*v.z.</b> */
GEOMEXPORT float LengthSquared(const Point3&); 
/*! \remarks Returns the component with the maximum absolute value. 0=x, 1=y,
2=z. */
GEOMEXPORT int MaxComponent(const Point3&);  // the component with the maximum abs value
/*! \remarks Returns the component with the minimum absolute value. 0=x, 1=y,
2=z. */
GEOMEXPORT int MinComponent(const Point3&);  // the component with the minimum abs value
/*! \remarks Returns a normalized unit vector. This is a Point3 with each
component divided by the point <b>Length()</b>. */
GEOMEXPORT Point3 Normalize(const Point3&);  // Accurate normalize
/*! \remarks Returns a normalized unit vector using faster assembly language
code than that used by <b>Normalize()</b>. This is a Point3 with each component
divided by the point <b>Length()</b>. */
GEOMEXPORT Point3 FNormalize(const Point3&); // Fast normalize 
/*! \remarks This returns the cross product of the specified Point3's
(vectors). The cross product of two vectors is a third vector, perpendicular to
the plane formed by the two vectors. */
GEOMEXPORT Point3 CrossProd(const Point3& a, const Point3& b);  // CROSS PRODUCT

// RB: moved this here from object.h
/*! \sa  Class Point3.\n\n
\par Description:
This class describes a vector in space using an origin point <b>p</b>, and a
unit direction vector <b>dir</b>.
\par Data Members:
<b>Point3 p;</b>\n\n
Point of origin.\n\n
<b>Point3 dir;</b>\n\n
Unit direction vector. */
class Ray: public MaxHeapOperators {
   public:
      Point3 p;   // point of origin
      Point3 dir; // unit vector
   };

 
// Inlines:

inline float Point3::Length() const {  
   return (float)sqrt(x*x+y*y+z*z);
   }

inline float Point3::FLength() const { 
   return Sqrt(x*x+y*y+z*z);
   }

inline float Point3::LengthSquared() const { 
   return (x*x+y*y+z*z);
   }

inline float Length(const Point3& v) { 
   return v.Length();
   }

inline float FLength(const Point3& v) {   
   return v.FLength();
   }

inline float LengthSquared(const Point3& v) {   
   return v.LengthSquared();
   }

inline Point3& Point3::operator-=(const Point3& a) {  
   x -= a.x;   y -= a.y;   z -= a.z;
   return *this;
   }

inline Point3& Point3::operator+=(const Point3& a) {
   x += a.x;   y += a.y;   z += a.z;
   return *this;
   }

inline Point3& Point3::operator*=(float f) {
   x *= f;   y *= f; z *= f;
   return *this;
   }

inline Point3& Point3::operator/=(float f) { 
   x /= f;  y /= f;  z /= f;  
   return *this; 
   }

inline Point3& Point3::operator*=(const Point3& a) { 
   x *= a.x;   y *= a.y;   z *= a.z;   
   return *this; 
   }

inline Point3& Point3::Set(float X, float Y, float Z) {
    x = X;
    y = Y;
    z = Z;
    return *this;
    }

inline Point3 Point3::operator-(const Point3& b) const {
   return(Point3(x-b.x,y-b.y,z-b.z));
   }

inline Point3 Point3::operator+(const Point3& b) const {
   return(Point3(x+b.x,y+b.y,z+b.z));
   }

inline Point3 Point3::operator/(const Point3& b) const {
   assert(b.x != 0.0f && b.y != 0.0f && b.z != 0.0f);
   return Point3(x/b.x,y/b.y,z/b.z);
   }

inline Point3 Point3::operator*(const Point3& b) const {  
   return Point3(x*b.x, y*b.y, z*b.z); 
   }

inline float Point3::operator%(const Point3& b) const {
   return (x*b.x + y*b.y + z*b.z);
   }

/*! \remarks Returns a Point3 that is the specified Point3 multiplied by the
specified float. */
inline Point3 operator*(float f, const Point3& a) {
   return(Point3(a.x*f, a.y*f, a.z*f));
   }

/*! \remarks Returns a Point3 that is the specified Point3 multiplied by the
specified float. */
inline Point3 operator*(const Point3& a, float f) {
   return(Point3(a.x*f, a.y*f, a.z*f));
   }

/*! \remarks Returns a Point3 that is the specified Point3 divided by the
specified float. */
inline Point3 operator/(const Point3& a, float f) {
   DbgAssert(f != 0.0f);
   return(Point3(a.x/f, a.y/f, a.z/f));
   }

/*! \remarks Returns a Point3 that is the specified Point3 with the specified
floating point valued added to each component x, y, and z. */
inline Point3 operator+(const Point3& a, float f) {
   return(Point3(a.x+f, a.y+f, a.z+f));
   }

/*! \remarks Returns the dot product of two Point3s. This is the sum of each
of the components multiplied together, element by element
<b>a.x*b.x+a.y*b.y+a.z*b.z</b>\n\n
The dot product has the property of equaling the product of the magnitude
(length) of the two vector times the cosine of the angle between them. */
inline float DotProd(const Point3& a, const Point3& b) { 
   return(a.x*b.x+a.y*b.y+a.z*b.z); 
   }


// These typedefs must be the same as each other, since
// vertex colors are contained in a MeshMap.
typedef Point3 UVVert;
typedef Point3 VertColor;