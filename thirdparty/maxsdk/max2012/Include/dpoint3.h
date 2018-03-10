/**********************************************************************
 *<
	FILE: dpoint3.h

	DESCRIPTION:

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "GeomExport.h"
#include "maxheap.h"
#include "point3.h"
#include <iosfwd>

/*! class DPoint3
\par Description:
This class describes a 3D point using double precision x, y and z coordinates.
Methods are provided to add and subtract points, multiply and divide by
scalars, and element by element multiply and divide two points. All methods are
implemented by the system.
\par Data Members:
<b>double x,y,z;</b>  */
class DPoint3: public MaxHeapOperators {
public:
	double x,y,z;

	// Constructors
	/*! \remarks Constructor. No initialization is performed. */
	DPoint3(){}
	/*! \remarks Constructor. x, y, and z are initialized to the values
	specified. */
	DPoint3(double X, double Y, double Z)  { x = X; y = Y; z = Z;  }
	/*! \remarks Constructor. x, y, and z are initialized to the DPoint3
	specified. */
	DPoint3(const DPoint3& a) { x = a.x; y = a.y; z = a.z; } 
	DPoint3(const Point3& a) { x = a.x; y = a.y; z = a.z; } 
	/*! \remarks Constructor. x, y, and z are initialized to. af[0], af[1],
	and af[2] respectively.
	\par Operators:
	*/
	DPoint3(double af[3]) { x = af[0]; y = af[1]; z = af[2]; }

	// Access operators
	/*! \remarks Allows access to x, y and z using the subscript operator.
	\return  An index of 0 will return x, 1 will return y, 2 will return z. */
	double& operator[](int i) { return (&x)[i]; }
	/*! \remarks Allows access to x, y and z using the subscript operator.
	\return  An index of 0 will return x, 1 will return y, 2 will return z. */
	const double& operator[](int i) const { return (&x)[i]; }  

 	// Conversion function
	/*! \remarks Conversion function. Returns the address of the DPoint3.x */
	operator double*() { return(&x); }

 	// Unary operators
	/*! \remarks Unary - operator. Negates both x, y and z. */
	DPoint3 operator-() const { return(DPoint3(-x,-y,-z)); } 
	/*! \remarks Unary +. Returns the point unaltered. */
	DPoint3 operator+() const { return *this; } 

	// Assignment operators
	GEOMEXPORT DPoint3& operator=(const Point3& a) {	x = a.x; y = a.y; z = a.z;	return *this; }
	/*! \remarks Subtracts a DPoint3 from this DPoint3. */
	GEOMEXPORT DPoint3& operator-=(const DPoint3&);
	/*! \remarks Adds a DPoint3 to this DPoint3. */
	GEOMEXPORT DPoint3& operator+=(const DPoint3&);
	/*! \remarks Each element of this DPoint3 is multiplied by the specified
	double. */
	GEOMEXPORT DPoint3& operator*=(double);
	/*! \remarks Each element of this DPoint3 is divided by the specified
	double. */
	GEOMEXPORT DPoint3& operator/=(double);

	// Binary operators
	/*! \remarks Subtracts a DPoint3 from a DPoint3. */
	GEOMEXPORT DPoint3 operator-(const DPoint3&) const;
	/*! \remarks Adds a DPoint3 to a DPoint3. */
	GEOMEXPORT DPoint3 operator+(const DPoint3&) const;
	/*! \remarks Computes the dot product of this DPoint3 and the specified
	DPoint3. */
	GEOMEXPORT double operator*(const DPoint3&) const;		// DOT PRODUCT
	/*! \remarks Computes the cross product of this DPoint3 and the specified
	DPoint3. */
	GEOMEXPORT DPoint3 operator^(const DPoint3&) const;	// CROSS PRODUCT

	};

/*! \remarks Returns the 'Length' of the point. This is
<b>sqrt(v.x*v.x+v.y*v.y+v.z*v.z)</b> */
GEOMEXPORT double Length(const DPoint3&); 
/*! \remarks Returns the component with the maximum absolute value. 0=x, 1=y,
2=z. */
GEOMEXPORT int MaxComponent(const DPoint3&);  // the component with the maximum abs value
/*! \remarks Returns the component with the minimum absolute value. 0=x, 1=y,
2=z. */
GEOMEXPORT int MinComponent(const DPoint3&);  // the component with the minimum abs value
/*! \remarks Returns a unit vector. This is a DPoint3 with each component
divided by the point <b>Length()</b>. */
GEOMEXPORT DPoint3 Normalize(const DPoint3&); // Return a unit vector.

GEOMEXPORT DPoint3 operator*(double, const DPoint3&);	// multiply by scalar
GEOMEXPORT DPoint3 operator*(const DPoint3&, double);	// multiply by scalar
GEOMEXPORT DPoint3 operator/(const DPoint3&, double);	// divide by scalar


GEOMEXPORT std::ostream &operator<<(std::ostream&, const DPoint3&); 
	 
// Inlines:

inline double Length(const DPoint3& v) {	
	return (double)sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
	}

inline DPoint3& DPoint3::operator-=(const DPoint3& a) {	
	x -= a.x;	y -= a.y;	z -= a.z;
	return *this;
	}

inline DPoint3& DPoint3::operator+=(const DPoint3& a) {
	x += a.x;	y += a.y;	z += a.z;
	return *this;
	}

inline DPoint3& DPoint3::operator*=(double f) {
	x *= f;   y *= f;	z *= f;
	return *this;
	}

inline DPoint3& DPoint3::operator/=(double f) { 
	x /= f;	y /= f;	z /= f;	
	return *this; 
	}

inline DPoint3 DPoint3::operator-(const DPoint3& b) const {
	return(DPoint3(x-b.x,y-b.y,z-b.z));
	}

inline DPoint3 DPoint3::operator+(const DPoint3& b) const {
	return(DPoint3(x+b.x,y+b.y,z+b.z));
	}

inline double DPoint3::operator*(const DPoint3& b) const {  
	return(x*b.x+y*b.y+z*b.z);	
	}

inline DPoint3 operator*(double f, const DPoint3& a) {
	return(DPoint3(a.x*f, a.y*f, a.z*f));
	}

inline DPoint3 operator*(const DPoint3& a, double f) {
	return(DPoint3(a.x*f, a.y*f, a.z*f));
	}

inline DPoint3 operator/(const DPoint3& a, double f) {
	return(DPoint3(a.x/f, a.y/f, a.z/f));
	}

/*! \remarks Returns the cross product of two DPoint3s. */
GEOMEXPORT DPoint3 CrossProd(const DPoint3& a, const DPoint3& b);	// CROSS PRODUCT
	
/*! \remarks Returns the dot product of two DPoint3s. */
GEOMEXPORT double DotProd(const DPoint3& a, const DPoint3& b) ;		// DOT PRODUCT


