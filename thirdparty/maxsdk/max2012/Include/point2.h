/**********************************************************************
 *<
	FILE: point2.h

	DESCRIPTION: Class definition for Point2

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "maxheap.h"
#include "GeomExport.h"
#include <iosfwd>
#include <math.h>

/*! \sa  Class IPoint2.\n\n
\par Description:
This class describes a 2D point using float x and y coordinates. Methods are
provided to add and subtract points, multiply and divide by scalars, normalize
and compute the dot product of two Point2s. All methods are implemented by the
system.
\par Data Members:
<b>float x,y;</b>\n\n
The x and y components of the point.\n\n
<b>static const Point2 Origin;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
This is equivalent to <b>Point2(0.0f, 0.0f);</b>\n\n
<b>static const Point2 XAxis;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
This is equivalent to <b>Point2(1.0f, 0.0f);</b>\n\n
<b>static const Point2 YAxis;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
This is equivalent to <b>Point2(0.0f, 1.0f);</b> <br>  Constructors */
class GEOMEXPORT Point2: public MaxHeapOperators {
public:
	float x,y;

	// Constructors
	/*! \remarks Constructor. No initialization is performed by this constructor. */
	Point2(){}
	/*! \remarks Constructor. Data members are initialized to X and Y. */
	Point2(float X, float Y)  { 
		x = X; y = Y;  
	}
	/*! \remarks Constructor. Data members are initialized to X and Y cast as floats. */
	Point2(double X, double Y)  { 
		x = (float)X; y = (float)Y;  
	}
	/*! \remarks Constructor. Data members are initialized to X and Y cast as floats. */
	Point2(int X, int Y)  { 
		x = (float)X; y = (float)Y;  
	}
	/*! \remarks Constructor. Data members are initialized to a.x and a.y. */
	Point2(const Point2& a) { 
		x = a.x; y = a.y; 
	} 
	/*! \remarks Constructor. Data members are initialized as x = af[0] and y = af[1]. */
	Point2(float af[2]) { 
		x = af[0]; y = af[1]; 
	}

    // Data members
    static const Point2 Origin;
    static const Point2 XAxis;
    static const Point2 YAxis;

	/*! \remarks Allows access to x, y using the subscript operator.
	\return  A value for <b>i</b> of 0 will return x, 1 will return y. */
	float& operator[](int i) { 
		return (&x)[i]; 
	}     
	/*! \remarks Allows access to x, y using the subscript operator.
	\return  A value for <b>i</b> of 0 will return x, 1 will return y. */
	const float& operator[](int i) const { 
		return (&x)[i]; 
	}  
	
	// Conversion function
	/*! \remarks Returns the address of the Point2.x */
	operator float*() { 
		return(&x); 
	}

	// Unary operators
	/*! \remarks Unary -. Negates both x and y.
	\return  A Point2 with -x, -y. */
	Point2 operator-() const { 
		return(Point2(-x,-y)); 
	} 
	/*! \remarks Unary +. Returns the point unaltered.
	\return  Returns the Point2 unaltered. */
	Point2 operator+() const { 
		return *this; 
	}
    
    // Property functions
    /*! \remarks Returns the length of the point. This is <b>sqrt(v.x*v.x+v.y*v.y);</b> */
    float Length() const;
    /*! \remarks This method returns the component with the maximum absolute value.
    \return  0 for X, 1 for Y, 2 for Z. */
    int MaxComponent() const;
    /*! \remarks This method returns the component with the minimum absolute value.
    \return  0 for X, 1 for Y, 2 for Z. */
    int MinComponent() const;
    /*! \remarks This method returns a normalized version of this Point2. This method is
    more accurate than <b>*this/Length()</b> (internal computations are done in
    double precision). */
    Point2 Normalize() const; // more accurate than *this/Length();

	// Assignment operators
	/*! \remarks Subtracts a Point2 from this Point2.
	\return  A Point2 that is the difference between two Point2s. */
	Point2& operator-=(const Point2&);
	/*! \remarks Adds a Point2 to this Point2.
	\return  A Point2 that is the sum of two Point2's. */
	Point2& operator+=(const Point2&);
	/*! \remarks Multiplies this Point2 by a floating point value.
	\return  A Point2 multiplied by a float. */
	Point2& operator*=(float);
	/*! \remarks Divides this Point2 by a floating point value.
	\return  A Point2 divided by a float. */
	Point2& operator/=(float);

    /*! \remarks Sets the x and y coordinate to the values passed and returns a reference to
    this Point2.
    \par Parameters:
    <b>float X</b>\n\n
    The new x value.\n\n
    <b>float Y</b>\n\n
    The new y value.
    \return  A reference to this <b>Point2</b>. */
    Point2& Set(float X, float Y);

	// Binary operators
	/*! \remarks Subtracts a Point2 from a Point2.
	\return  A Point2 that is the difference between two Point2's. */
	Point2 operator-(const Point2&) const;
	/*! \remarks Adds a Point2 to a Point2.
	\return  The sum of two Point2's. */
	Point2 operator+(const Point2&) const;
	/*! \remarks Returns the dot product of two Point2's. This is the sum of
	both x values multiplied together and both y values multiplied together. */
	float DotProd(const Point2&) const;		// DOT PRODUCT
	/*! \remarks Returns the dot product of two Point2's. This is the sum of
	both x values multiplied together and both y values multiplied together. */
	float operator*(const Point2&) const;	// DOT PRODUCT

	// Relational operators
	/*! \remarks Equality operator. Compares two Point2's.
	\return  Nonzero if the Point2's are equal; otherwise 0. */
	int operator==(const Point2& p) const { return (x == p.x && y == p.y); }
	int operator!=(const Point2& p) const { return ( (x != p.x) || (y != p.y) ); }
	/*! \remarks	Compares this Point2 and the specified one to see if the x and y values are
	within plus or minus the specified tolerance.
	\par Parameters:
	<b>const Point2\& p</b>\n\n
	The point to compare.\n\n
	<b>float epsilon = 1E-6f</b>\n\n
	The tolerance to use in the comparison.
	\return  Nonzero if the points are 'equal'; otherwise zero. */
  int Equals(const Point2& p, float epsilon = 1E-6f);
    
    // In-place normalize
    /*! \remarks This method is used to unify (or normalize) this Point2 (in place) and
    return the result. Internal computations are done in double precision. */
    Point2& Unify();
    /*! \remarks This method is used to unify (or normalize) this Point2 (in place) and
    return the previous length. Internal computations are done in double precision.
    */
    float LengthUnify();          // returns old Length
    };

/*! \remarks Returns a copy of the Point2 argument offset by (f, f). */
inline Point2 operator+(const Point2& a, float f) {
   return(Point2(a.x+f, a.y+f));
   }


/*! \remarks Returns the length of the Point2, ie:\n\n
<b>sqrt(v.x*v.x+v.y*v.y);</b> */
GEOMEXPORT float Length(const Point2&); 
/*! \remarks Returns the component with the maximum absolute value. 0=x, 1=y.
*/
GEOMEXPORT int MaxComponent(const Point2&);  // the component with the maximum abs value
/*! \remarks Returns the component with the minimum absolute value. 0=x, 1=y.
*/
GEOMEXPORT int MinComponent(const Point2&);  // the component with the minimum abs value
/*! \remarks Returns a unit vector. This is a Point2 with each component
divided by the point <b>Length()</b>. */
GEOMEXPORT Point2 Normalize(const Point2&);  // more accurate than v/Length(v)

/*! \remarks Returns a Point2 multiplied by a scalar. */
GEOMEXPORT Point2 operator*(float, const Point2&);	// multiply by scalar
/*! \remarks Returns a Point2 multiplied by a scalar. */
GEOMEXPORT Point2 operator*(const Point2&, float);	// multiply by scalar
/*! \remarks Returns a Point2 whose x and y members are divided by a scalar.*/
GEOMEXPORT Point2 operator/(const Point2&, float);	// divide by scalar

GEOMEXPORT std::ostream &operator<<(std::ostream&, const Point2&);
	 
// Inlines:

inline float Length(const Point2& v) {	
	return (float)sqrt(v.x*v.x+v.y*v.y);
	}

inline float Point2::Length() const {	
	return (float)sqrt(x*x+y*y);
	}

/*! \remarks The 'Length' squared of the point. This is
<b>v.x*v.x+v.y*v.y.</b> */
inline float LengthSquared(const Point2& v) {   
	return (float)(v.x*v.x+v.y*v.y);
	}

inline Point2& Point2::operator-=(const Point2& a) {	
	x -= a.x;	y -= a.y;  
	return *this;
	}

inline Point2& Point2::operator+=(const Point2& a) {
	x += a.x;	y += a.y;  
	return *this;
	}

inline Point2& Point2::operator*=(float f) {
	x *= f;   y *= f;	
	return *this;
	}

inline Point2& Point2::operator/=(float f) { 
	x /= f;	y /= f;		
	return *this; 
	}

inline Point2& Point2::Set(float X, float Y) {
    x = X; y = Y;
    return *this;
    }

inline Point2 Point2::operator-(const Point2& b) const{
	return(Point2(x-b.x,y-b.y));
	}

inline Point2 Point2::operator+(const Point2& b) const {
	return(Point2(x+b.x,y+b.y));
	}

inline float Point2::DotProd(const Point2& b) const{
	return(x*b.x+y*b.y);
	}

inline float Point2::operator*(const Point2& b)const {
	return(x*b.x+y*b.y);
	}

inline Point2 operator*(float f, const Point2& a) {
	return(Point2(a.x*f, a.y*f));
	}

inline Point2 operator*(const Point2& a, float f) {
	return(Point2(a.x*f, a.y*f));
	}

inline Point2 operator/(const Point2& a, float f) {
	return(Point2(a.x/f, a.y/f));
	}


