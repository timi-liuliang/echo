/**********************************************************************
 *<
	FILE: ipoint3.h

	DESCRIPTION: Class definitions for IPoint3

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "GeomExport.h"
#include "maxheap.h"
#include <iosfwd>
#include <math.h>

/*! \sa  Class Point3.\n\n
\par Description:
This class describes a 3D point using integer x, y and z coordinates. Methods
are provided to add and subtract points, multiply and divide by scalars, and
element by element multiply and divide two points. All methods are implemented
by the system. <br>
\par Data Members:
<b>int x,y,z;</b> */
class IPoint3: public MaxHeapOperators {
public:
	int x,y,z;

	// Constructors
	/*! \remarks Constructor. No initialization is performed. */
	IPoint3(){}
	/*! \remarks Constructor. x, y, and z are initialized to the values
	specified. */
	IPoint3(int X, int Y, int Z)  { x = X; y = Y; z = Z;  }
	/*! \remarks Constructor. x, y, and z are initialized to the IPoint3
	specified. */
	IPoint3(const IPoint3& a) { x = a.x; y = a.y; z = a.z; } 
	/*! \remarks Constructor. x, y, and z are initialized to. ai[0], ai[1],
	and ai[2] respectively. */
	IPoint3(int ai[3]) { x = ai[0]; y = ai[1]; z = ai[2]; }

	// Access operators
	/*! \remarks Allows access to x, y and z using the [ ] operator.
	\return  An index of 0 will return x, 1 will return y, 2 will return z. */
	int& operator[](int i) { return (&x)[i]; }     
	/*! \remarks Allows access to x, y and z using the [ ] operator.
	\return  An index of 0 will return x, 1 will return y, 2 will return z. */
	const int& operator[](int i) const { return (&x)[i]; }  
 	
 	// Conversion function
	/*! \remarks Conversion function. Returns the address of the IPoint3. */
	operator int*() { return(&x); }

	// Unary operators
	/*! \remarks Unary - operator. Negates x, y and z. */
	IPoint3 operator-() const { return(IPoint3(-x,-y,-z)); } 
	/*! \remarks Unary +. Returns the point unaltered. */
	IPoint3 operator+() const { return *this; } 

	// Assignment operators
	/*! \remarks Subtracts a IPoint3 from this IPoint3. */
	GEOMEXPORT IPoint3& operator-=(const IPoint3&);
	/*! \remarks Adds a IPoint3 to this IPoint3. */
	GEOMEXPORT IPoint3& operator+=(const IPoint3&);

	// Binary operators
	/*! \remarks Subtracts a IPoint3 from a IPoint3. */
	GEOMEXPORT IPoint3 operator-(const IPoint3&) const;
	/*! \remarks Adds a IPoint3 to a IPoint3. */
	GEOMEXPORT IPoint3 operator+(const IPoint3&) const;
	/*! \remarks Returns the dot product of two IPoint3s. */
	GEOMEXPORT int operator*(const IPoint3&) const;    // DOT PRODUCT
	/*! \remarks Returns the dot product of two IPoint3s. */
	GEOMEXPORT int DotProd(const IPoint3&) const;    // DOT PRODUCT
	/*! \remarks The cross product of two IPoint3's (vectors). */
	GEOMEXPORT IPoint3 operator^(const IPoint3&) const;   // CROSS PRODUCT
	/*! \remarks Returns the cross product of two IPoint3's (vectors).
	\par Operators:
	*/
	GEOMEXPORT IPoint3 CrossProd(const IPoint3&) const;   // CROSS PRODUCT

	// Relational operators
	/*! \remarks Test for equality between two IPoint3's.
	\return  Nonzero if the IPoint3's are equal; otherwise 0.\n\n
	  */
	int operator==(const IPoint3& p) const { return (x == p.x && y == p.y && z == p.z); }
	int operator!=(const IPoint3& p) const { return ( (x != p.x) || (y != p.y) || (z != p.z) ); }
	};

    // friends, so you can write Length(A) instead of A.Length(), etc.
/*! \remarks Returns the component with the maximum absolute value. 0=x, 1=y,
2=z. */
GEOMEXPORT int MaxComponent(const IPoint3&);  // the component with the maximum abs value
/*! \remarks Returns the component with the minimum absolute value. 0=x, 1=y,
2=z. */
GEOMEXPORT int MinComponent(const IPoint3&);  // the component with the minimum abs value
	 
GEOMEXPORT std::ostream &operator<<(std::ostream&, const IPoint3&); 


// Inlines:

/*! \remarks Returns the 'Length' of the point. This is
<b>sqrt(v.x*v.x+v.y*v.y+v.z*v.z)</b> */
inline float Length(const IPoint3& v) {	
	return (float)sqrt((double)(v.x*v.x+v.y*v.y+v.z*v.z));
	}

inline IPoint3& IPoint3::operator-=(const IPoint3& a) {	
	x -= a.x;	y -= a.y;	z -= a.z;
	return *this;
	}

inline IPoint3& IPoint3::operator+=(const IPoint3& a) {
	x += a.x;	y += a.y;	z += a.z;
	return *this;
	}

inline IPoint3 IPoint3::operator-(const IPoint3& b) const {
	return(IPoint3(x-b.x,y-b.y,z-b.z));
	}

inline IPoint3 IPoint3::operator+(const IPoint3& b) const {
	return(IPoint3(x+b.x,y+b.y,z+b.z));
	}

inline int IPoint3::operator*(const IPoint3& b) const {  
	return(x*b.x+y*b.y+z*b.z);	
	}

inline int IPoint3::DotProd(const IPoint3& b)  const { 
	return(x*b.x+y*b.y+z*b.z);	
	}


