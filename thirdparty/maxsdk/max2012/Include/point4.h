/**********************************************************************
 *<
   FILE: point4.h

   DESCRIPTION: Class definitions for Point4

   CREATED BY: Dan Silva

   HISTORY:

 *>   Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "GeomExport.h"
#include "maxheap.h"
#include "point3.h"
#include "assert1.h"

class GEOMEXPORT Point4;

/*! \sa  Class Point3.\n\n
\par Description:
This class describes a point using float x, y, z and w coordinates. Methods are
provided to add and subtract points, multiply and divide by scalars, and
element by element multiply and divide two points. All methods are implemented
by the system.\n\n
This class is available in release 2.0 and later only.
\par Data Members:
<b>float x, y, z, w;</b>\n\n
The x, y, z and w components of the point.\n\n
<b>static const Point4 Origin;</b>\n\n
This is equivalent to <b>Point4(0.0f, 0.0f, 0.0f, 0.0f);</b>\n\n
<b>static const Point4 XAxis;</b>\n\n
This is equivalent to <b>Point4(1.0f, 0.0f, 0.0f, 0.0f);</b>\n\n
<b>static const Point4 YAxis;</b>\n\n
This is equivalent to <b>Point4(0.0f,1.0f, 0.0f, 0.0f);</b>\n\n
<b>static const Point4 ZAxis;</b>\n\n
This is equivalent to <b>Point4(0.0f, 0.0f,1.0f, 0.0f);</b>\n\n
<b>static const Point4 WAxis;</b>\n\n
This is equivalent to <b>Point4(0.0f, 0.0f, 0.0f,1.0f);</b>  */
class GEOMEXPORT Point4: public MaxHeapOperators {
public:
   float x,y,z,w;

   // Constructors
   /*! \remarks Constructor. No initialization is performed. */
   Point4(){  /* NO INIT */}
   /*! \remarks Constructor. x, y, z and w are initialized to the values specified. */
   Point4(float X, float Y, float Z, float W)  { 
		 x = X; y = Y; z = Z; w = W; 
	 }
   /*! \remarks Constructor. x, y, z and w are initialized to the specified values (cast as floats). */
   Point4(double X, double Y, double Z, double W) { 
		 x = (float)X; y = (float)Y; z = (float)Z; w = (float)W; 
	 }
   /*! \remarks Constructor. x, y, z and w are initialized to the specified values (cast as floats). */
   Point4(int X, int Y, int Z, int W) { 
		 x = (float)X; y = (float)Y; z = (float)Z; w = (float)W; 
	 }
   /*! \remarks Constructor. x, y, z and w are initialized to the specified Point3 and W. */
   Point4(const Point3& a, float W=0.0f) { 
		 x = a.x; y = a.y; z = a.z; w = W; 
	 } 
   
	 Point4(const Point4& a) { 
		 x = a.x; y = a.y; z = a.z; w = a.w; 
	 } 
   /*! \remarks Constructor. x, y, z and w are initialized to af[0], af[1], af[2] and af[3] respectively. */
   Point4(float af[4]) { 
		 x = af[0]; y = af[1]; z = af[2]; w = af[3]; 
	 }

    // Data members
    static const Point4 Origin;
    static const Point4 XAxis;
    static const Point4 YAxis;
    static const Point4 ZAxis;
    static const Point4 WAxis;

	/*! \remarks Allows access to x, y, z and w using the subscript operator.
	\return  An value for <b>i</b> of 0 will return x, 1 will return y, 2 will
	return z and 3 will return w. */
  float& operator[](int i) { 
		 return (&x)[i];
	 }     
	/*! \remarks Allows access to x, y, z and w using the subscript operator.
	\return  An value for <b>i</b> of 0 will return x, 1 will return y, 2 will
	return z and 3 will return w. */
   const float& operator[](int i) const { 
		 return (&x)[i]; 
	 }  

   // Conversion function
   /*! \remarks Conversion function. Returns the address of the Point4.x */
   operator float*() { 
		 return(&x); 
	 }

   // Unary operators
   /*! \remarks Unary - operator. Negates x, y, z and w. */
   Point4 operator-() const { 
		 return(Point4(-x,-y,-z, -w)); 
	 } 
   /*! \remarks Unary +. Returns the Point4. */
   Point4 operator+() const { 
		 return *this; 
	 } 

   // Property functions
   float Length() const;
   float FLength() const;
   float LengthSquared() const;
   int MaxComponent() const;
   int MinComponent() const;
   Point4 Normalize() const;     // more accurate than FNormalize()
   Point4 FNormalize() const;    // faster than Normalize()

   // Assignment operators
   /*! \remarks Subtracts a Point4 from this Point4.
   \return  A Point4 that is the difference between two Point4s. */
   inline Point4& operator-=(const Point4&);
   /*! \remarks Adds a Point4 to this Point4.
   \return  A Point4 that is the sum of two Point4s. */
   inline Point4& operator+=(const Point4&);
   /*! \remarks Multiplies this Point4 by a floating point value.
   \return  A Point4 multiplied by a float. */
   inline Point4& operator*=(float); 
   /*! \remarks Divides this Point4 by a floating point value.
   \return  A Point4 divided by a float. */
   inline Point4& operator/=(float);
   /*! \remarks Element-by-element multiplication of two Point4s:\n\n
   <b>(x*x, y*y, z*z, w*w)</b>.
   \return  A Point4 element-by-element multiplied by another Point4. */
   inline Point4& operator*=(const Point4&); // element-by-element multiply.
    /*! \remarks Sets the x, y, z and w coordinate to the values passed and returns a
    reference to this Point4.
    \par Parameters:
    <b>float X</b>\n\n
    The new x value.\n\n
    <b>float Y</b>\n\n
    The new y value.\n\n
    <b>float Z</b>\n\n
    The new z value.\n\n
    <b>float W</b>\n\n
    The new w value.
    \return  A reference to this <b>Point4</b>. */
    inline Point4& Set(float X, float Y, float Z, float W);

   // Test for equality
   /*! \remarks Equality operator. Test for equality between two Point4's.
   \return  Nonzero if the Point4's are equal; otherwise 0. */
   int operator==(const Point4& p) const { 
		 return ((p.x==x)&&(p.y==y)&&(p.z==z)&&(p.w==w)); 
	 }
   int operator!=(const Point4& p) const { 
		 return ((p.x!=x)||(p.y!=y)||(p.z!=z)||(p.w!=w)); 
	 }

	 /*! \remarks	Compares this Point4 and the specified one to see if the x, y, z and w values
	are within plus or minus the specified tolerance.
	\par Parameters:
	<b>const Point4\& p</b>\n\n
	The point to compare.\n\n
	<b>float epsilon = 1E-6f</b>\n\n
	The tolerance to use in the comparison.
	\return  Nonzero if the points are 'equal'; otherwise zero.
	\par Operators:
	*/
	 int Equals(const Point4& p, float epsilon = 1E-6f);

   // In-place normalize
   Point4& Unify();
   float LengthUnify();              // returns old Length

   // Binary operators
   /*! \remarks Subtracts a Point4 from a Point4.
   \return  A Point4 that is the difference between two Point4s. */
   inline  Point4 operator-(const Point4&) const;
   /*! \remarks Adds a Point4 to a Point4.
   \return  A Point4 that is the sum of two Point4s. */
   inline  Point4 operator+(const Point4&) const;
   /*! \remarks Divides a Point4 by a Point4 element by element.
   \return  A Point4 resulting from dividing a Point4 by a Point4 element by
   element. */
   inline  Point4 operator/(const Point4&) const;
   /*! \remarks Multiplies a Point4 by a Point4 element by element.\n\n
   <b>(x*x, y*y, z*z, w*w)</b>.
   \return  A Point4 resulting from the multiplication of a Point4 and a
   Point4. */
   inline  Point4 operator*(const Point4&) const;   

   inline float operator%(const Point4&) const;     // DOT PRODUCT
   };

GEOMEXPORT float Length(const Point4&); 
GEOMEXPORT float FLength(const Point4&); 
GEOMEXPORT float LengthSquared(const Point4&); 
GEOMEXPORT int MaxComponent(const Point4&);  // the component with the maximum abs value
GEOMEXPORT int MinComponent(const Point4&);  // the component with the minimum abs value
GEOMEXPORT Point4 Normalize(const Point4&);  // Accurate normalize
GEOMEXPORT Point4 FNormalize(const Point4&); // Fast normalize 
GEOMEXPORT Point4 CrossProd(const Point4& a, const Point4& b, const Point4& c);   // CROSS PRODUCT

// Inlines:

inline float Point4::Length() const {  
   return (float)sqrt(x*x+y*y+z*z+w*w);
}

inline float Point4::FLength() const { 
   return Sqrt(x*x+y*y+z*z+w*w);
}

inline float Point4::LengthSquared() const { 
   return (x*x+y*y+z*z+w*w);
}

inline float Length(const Point4& v) { 
   return v.Length();
}

inline float FLength(const Point4& v) {   
   return v.FLength();
}

inline float LengthSquared(const Point4& v) {   
   return v.LengthSquared();
}

inline Point4& Point4::operator-=(const Point4& a) {  
   x -= a.x;   y -= a.y;   z -= a.z; w -= a.w;

   return *this;
   }

inline Point4& Point4::operator+=(const Point4& a) {
   x += a.x;   y += a.y;   z += a.z; w += a.w;
   return *this;
   }

inline Point4& Point4::operator*=(float f) {
   x *= f;   y *= f; z *= f; w *= f;

   return *this;
   }

inline Point4& Point4::operator/=(float f) { 
   if (f==0.0f) f = .000001f;
   x /= f;  y /= f;  z /= f;   w /= f;

   return *this; 
   }

inline Point4& Point4::operator*=(const Point4& a) { 
   x *= a.x;   y *= a.y;   z *= a.z;   w *= a.w;

   return *this; 
   }

inline Point4& Point4::Set(float X, float Y, float Z, float W) {
    x = X;
    y = Y;
    z = Z;
    w = W;

    return *this;
    }

inline Point4 Point4::operator-(const Point4& b) const {
   return(Point4(x-b.x,y-b.y,z-b.z, w-b.w));
   }

inline Point4 Point4::operator+(const Point4& b) const {
   return(Point4(x+b.x,y+b.y,z+b.z, w+b.w));
   }

inline Point4 Point4::operator/(const Point4& b) const {
   assert(b.x != 0.0f && b.y != 0.0f && b.z != 0.0f && b.w != 0.0f);
   return Point4(x/b.x,y/b.y,z/b.z,w/b.w);
   }

inline Point4 Point4::operator*(const Point4& b) const {  
   return Point4(x*b.x, y*b.y, z*b.z,w*b.w); 
   }

/*! \remarks Returns a Point4 that is the specified Point4 multiplied by the
specified float. */
inline Point4 operator*(float f, const Point4& a) {
   return(Point4(a.x*f, a.y*f, a.z*f, a.w*f));
   }

/*! \remarks Returns a Point4 that is the specified Point4 multiplied by the
specified float. */
inline Point4 operator*(const Point4& a, float f) {
   return(Point4(a.x*f, a.y*f, a.z*f, a.w*f));
   }

/*! \remarks Returns a Point4 that is the specified Point4 divided by the
specified float. */
inline Point4 operator/(const Point4& a, float f) {
   assert( f != 0.0f );
   return(Point4(a.x/f, a.y/f, a.z/f, a.w/f));
   }

/*! \remarks Returns a Point4 that is the specified Point4 with the specified
floating point valued added to each component x, y, z and w. */
inline Point4 operator+(const Point4& a, float f) {
   return(Point4(a.x+f, a.y+f, a.z+f, a.w+f));
   }

inline float Point4::operator%(const Point4& b) const {
   return (x*b.x + y*b.y + z*b.z + w*b.w);
}

inline int Point4::Equals(const Point4& p, float epsilon) {
    return (fabs(p.x - x) <= epsilon && fabs(p.y - y) <= epsilon
            && fabs(p.z - z) <= epsilon && fabs(p.w - w) <= epsilon);
    }

inline float DotProd(const Point4& a, const Point4& b) { 
   return(a.x*b.x+a.y*b.y+a.z*b.z+a.w*b.w);  
}


