/**********************************************************************
 *<
	FILE: ipoint2.h

	DESCRIPTION: Class definintion for IPoint2: Integer 2D point.

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "GeomExport.h"
#include "maxheap.h"
#include <iosfwd>
#include <math.h>


/*! \sa  Class Point2.\n\n
\par Description:
This class describes a 2D point using int x and y coordinates. Methods are
provided to add and subtract points, multiply and divide by scalars, normalize
and compute the dot product of two IPoint2s. All methods are implemented by the
system.
\par Data Members:
<b>int x,y;</b>  */
class IPoint2 : public MaxHeapOperators {
public:
	int x,y;

	// Constructors
	/*! \remarks Constructor. */
	IPoint2(){}
	/*! \remarks Constructor. Data members are initialized to X and Y. */
	IPoint2(int X, int Y)  { x = X; y = Y;  }
	/*! \remarks Constructor. Data members are initialized to a.x and a.y. */
	IPoint2(const IPoint2& a) { x = a.x; y = a.y; } 
	/*! \remarks Constructor. Data members are initialized as x = af[0] and y
	= af[1]. */
	IPoint2(int af[2]) { x = af[0]; y = af[1]; }

	// Access operators
	/*! \remarks Allows access to x, y using the subscript operator.
	\return  An index of 0 will return x, 1 will return y. */
	int& operator[](int i) { return (&x)[i]; }     
	/*! \remarks Allows access to x, y using the subscript operator.
	\return  An index of 0 will return x, 1 will return y. */
	const int& operator[](int i) const { return (&x)[i]; }  

 	// Conversion function
	/*! \remarks Returns the address of the IPoint2.x. */
	operator int*() { return(&x); }
	 
	// Unary operators
	/*! \remarks Unary -. Negates both x and y. */
	IPoint2 operator-() const { return(IPoint2(-x,-y)); } 
	/*! \remarks Unary +. Returns the Ipoint2 unaltered. */
	IPoint2 operator+() const { return *this; } 

	// Assignment operators
	/*! \remarks Subtracts a IPoint2 from this IPoint2. */
	IPoint2& operator-=(const IPoint2&);
	/*! \remarks Adds a IPoint2 to this IPoint2. */
	IPoint2& operator+=(const IPoint2&);
	/*! \remarks Multiplies this IPoint2 by an integer value. */
	GEOMEXPORT IPoint2& operator*=(int);
	/*! \remarks Divides this IPoint2 by an integer value. */
	GEOMEXPORT IPoint2& operator/=(int);

	// Binary operators
	/*! \remarks Subtracts a IPoint2 from a IPoint2. */
	GEOMEXPORT IPoint2 operator-(const IPoint2&) const;
	/*! \remarks Adds a IPoint2 to a IPoint2. */
	GEOMEXPORT IPoint2 operator+(const IPoint2&) const;
	/*! \remarks Returns the dot product of two IPoint2's. This is the sum of
	both x values multiplied together and both y values multiplied together.
	\par Operators:
	*/
	GEOMEXPORT int DotProd(const IPoint2&) const;    // DOT PRODUCT
	/*! \remarks Returns the dot product of two IPoint2's. This is the sum of
	both x values multiplied together and both y values multiplied together. */
	GEOMEXPORT int operator*(const IPoint2&) const;    // DOT PRODUCT

	// Relational operators
	/*! \remarks Equality operator. Compare two IPoint2's.
	\return  Nonzero if the IPoint2's are equal; otherwise 0.\n\n
	  */
	int operator==(const IPoint2& p) const { return (x == p.x && y == p.y); }
	int operator!=(const IPoint2& p) const { return (x != p.x || y != p.y); }
	};

/*! \remarks Returns the length of the IPoint2, ie:\n\n
<b>sqrt(v.x*v.x+v.y*v.y);</b> */
GEOMEXPORT int Length(const IPoint2&); 
/*! \remarks Returns a unit vector. This is an IPoint2 with each component
divided by the point <b>Length()</b>. */
GEOMEXPORT IPoint2 Normalize(const IPoint2&); // Return a unit vector.
/*! \remarks Returns an IPoint2 multiplied by a scalar. */
GEOMEXPORT IPoint2 operator*(int, const IPoint2&);	// multiply by scalar
/*! \remarks Returns an IPoint2 multiplied by a scalar. */
GEOMEXPORT IPoint2 operator*(const IPoint2&, int);	// multiply by scalar
/*! \remarks Returns an IPoint2 whose x and y members are divided by a scalar.
*/
GEOMEXPORT IPoint2 operator/(const IPoint2&, int);	// divide by scalar

GEOMEXPORT std::ostream &operator<<(std::ostream&, const IPoint2&); 

// Inlines:

/*! \remarks Returns the component with the minimum abs value. 0=x, 1=y. */
inline int MaxComponent(const IPoint2& p) { return(p.x>p.y?0:1); }
/*! \remarks Returns the component with the minimum abs value. 0=x, 1=y. */
inline int MinComponent(const IPoint2& p) { return(p.x<p.y?0:1); }

inline int Length(const IPoint2& v) {	
	return (int)sqrt((double)(v.x*v.x+v.y*v.y));
	}

inline IPoint2& IPoint2::operator-=(const IPoint2& a) {	
	x -= a.x;	y -= a.y;  
	return *this;
	}

inline IPoint2& IPoint2::operator+=(const IPoint2& a) {
	x += a.x;	y += a.y;  
	return *this;
	}

inline IPoint2& IPoint2::operator*=(int f) {
	x *= f;   y *= f;	
	return *this;
	}

inline IPoint2& IPoint2::operator/=(int f) { 
	x /= f;	y /= f;		
	return *this; 
	}

inline IPoint2 IPoint2::operator-(const IPoint2& b) const{
	return(IPoint2(x-b.x,y-b.y));
	}

inline IPoint2 IPoint2::operator+(const IPoint2& b) const {
	return(IPoint2(x+b.x,y+b.y));
	}

inline int IPoint2::DotProd(const IPoint2& b) const{
	return(x*b.x+y*b.y);
	}

inline int IPoint2::operator*(const IPoint2& b)const {
	return(x*b.x+y*b.y);
	}

inline IPoint2 operator*(int f, const IPoint2& a) {
	return(IPoint2(a.x*f, a.y*f));
	}

inline IPoint2 operator*(const IPoint2& a, int f) {
	return(IPoint2(a.x*f, a.y*f));
	}

inline IPoint2 operator/(const IPoint2& a, int f) {
	return(IPoint2(a.x/f, a.y/f));
	}


