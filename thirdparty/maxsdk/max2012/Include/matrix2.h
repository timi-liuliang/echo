/**********************************************************************
 *<
	FILE: matrix2.h

	DESCRIPTION: Class definitions for Matrix2

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "GeomExport.h"
#include "maxheap.h"
#include "ioapi.h"
#include "point2.h"
#include "point3.h"

#include <iosfwd>

/*! \sa  Class Point2, Class Point3, Class Matrix3.\n\n
\par Description:
This class defines a 3x2 2D transformation matrix. Methods are provided to zero
the matrix, set it to the identity matrix, translate, rotate and scale it, and
compute its inverse. Operators are available for matrix addition, subtraction
and multiplication. All methods of this class are implemented by the system.
\par Data Members:
<b>float m[3][2];</b>  */
class Matrix2: public MaxHeapOperators {
	Point2& operator[](int i) { return((Point2&)(*m[i]));  }
	Point2& operator[](int i) const { return((Point2&)(*m[i])); }
public:
	float m[3][2];

	// Constructors
	/*! \remarks Constructor. No initialization is done in this constructor.
	Use <b>Zero()</b> or <b>IdentityMatrix()</b> to initialize the matrix. */
	Matrix2(){}	 // NO INITIALIZATION done in this constructor!! (can use Zero or IdentityMatrix)
	Matrix2(BOOL init) { UNUSED_PARAM(init); IdentityMatrix(); } // An option to initialize

	/*! \remarks Constructor. The matrix is initialized using <b>fp</b>. */
	GEOMEXPORT Matrix2(float (*fp)[2]); 
	
    // Data member
    static const Matrix2 Identity;
    
    // Assignment operators
	/*! \remarks Subtracts a Matrix2 from this Matrix2. */
	GEOMEXPORT Matrix2& operator-=( const Matrix2& M);
	/*! \remarks Adds a Matrix2 to this Matrix2. */
	GEOMEXPORT Matrix2& operator+=( const Matrix2& M); 
	/*! \remarks Matrix multiplication between this Matrix2 and <b>M</b>. */
	GEOMEXPORT Matrix2& operator*=( const Matrix2& M);  	// Matrix multiplication

 	// Conversion function
	/*! \remarks Returns the address of the Matrix2. */
	operator float*() { return(&m[0][0]); }

	// Initialize matrix
	/*! \remarks Sets this Matrix2 to the Identity Matrix. */
	GEOMEXPORT void IdentityMatrix(); 	// Set to the Identity Matrix
	/*! \remarks Set all elements of this Matrix2 to 0.0f */
	GEOMEXPORT void Zero();		// Set all elements to 0

	/*! \remarks Returns the specified row of this matrix.
	\par Parameters:
	<b>int i</b>\n\n
	Specifies the row to retrieve (0-2). */
	Point2 GetRow(int i) const { return (*this)[i]; }	
	/*! \remarks Sets the specified row of this matrix.
	\par Parameters:
	<b>int i</b>\n\n
	Specifies the row to set (0-2).\n\n
	<b>Point2 p</b>\n\n
	The values to set. */
	GEOMEXPORT void SetRow(int i, Point2 p) { (*this)[i] = p; }

	/*! \remarks Returns the specified column of this matrix.
	\par Parameters:
	<b>int i</b>\n\n
	Specifies the column to retrieve (0 or 1). */
	GEOMEXPORT Point3 GetColumn(int i);
	/*! \remarks Sets the specified column of this matrix.
	\par Parameters:
	<b>int i</b>\n\n
	Specifies the column to set (0 or 1).\n\n
	<b>Point3 col</b>\n\n
	The values to set. */
	GEOMEXPORT void SetColumn(int i,  Point3 col);
	/*! \remarks This method returns a Point2 containing the upper two rows of
	the specified column.
	\par Parameters:
	<b>int i</b>\n\n
	Specifies the column to get (0 or 1). */
	GEOMEXPORT Point2 GetColumn2(int i);

	// Access the translation row
	/*! \remarks Sets the translation row of the matrix to the specified
	values.
	\par Parameters:
	<b>const Point2 p</b>\n\n
	The values to set. */
	void SetTrans(const Point2 p) { (*this)[2] = p;  }
	/*! \remarks Sets the specified element of the translation row of this
	matrix to the specified value.
	\par Parameters:
	<b>int i</b>\n\n
	Specifies which column to set (0 or 1)\n\n
	<b>float v</b>\n\n
	The value to store. */
	void SetTrans(int i, float v) { (*this)[2][i] = v;  }
	/*! \remarks Returns the translation row of this matrix. */
	Point2 GetTrans() { return (*this)[2]; }
   
	// Apply Incremental transformations to this matrix
	/*! \remarks Apply an incremental translation to this matrix.
	\par Parameters:
	<b>const Point2\& p</b>\n\n
	Specifies the amount to translate the matrix. */
	GEOMEXPORT void Translate(const Point2& p);
	/*! \remarks Apply an incremental rotation to this matrix using the
	specified angle.
	\par Parameters:
	<b>float angle</b>\n\n
	Specifies the angle of rotation. */
	GEOMEXPORT void Rotate(float angle);  
	// if trans = FALSE the translation component is unaffected:
	/*! \remarks Apply an incremental scaling to this matrix using the
	specified scale factors.
	\par Parameters:
	<b>const Point2\& s</b>\n\n
	The scale factors.\n\n
	<b>BOOL trans = FALSE</b>\n\n
	If set to <b>TRUE</b>, the translation component is scaled. If <b>trans</b>
	= <b>FALSE</b> the translation component is unaffected. When 3ds Max was
	originally written there was a bug in the code for this method where the
	translation portion of the matrix was not being scaled. This meant that
	when a matrix was scaled the bottom row was not scaled. Thus it would
	always scale about the local origin of the object, but it would scale the
	world axes. When this bug was discovered, dependencies existed in the code
	upon this bug. Thus it could not simply be fixed because it would break the
	existing code that depended upon it working the incorrect way. To correct
	this the <b>trans</b> parameter was added. If this is set to <b>TRUE</b>,
	the translation component will be scaled correctly. The existing plug-ins
	don't use this parameter, it defaults to <b>FALSE</b>, and the code behaves
	the old way. */
	GEOMEXPORT void Scale(const Point2& s, BOOL trans=FALSE);

	// Apply Incremental transformations to this matrix
	// Equivalent to multiplying on the LEFT by transform 
	GEOMEXPORT void PreTranslate(const Point2& p);
	GEOMEXPORT void PreRotate(float angle);  
	GEOMEXPORT void PreScale(const Point2& s, BOOL trans = FALSE);

    // Set matrix as described
    /*! \remarks Initializes the matrix to the identity then sets the translation row to the
    specified values.
    \par Parameters:
    <b>const Point2\& s</b>\n\n
    The values to store. */
    GEOMEXPORT void SetTranslate(const Point2& s); // makes translation matrix
    /*! \remarks Initializes the matrix to the identity then sets the rotation to the
    specified value.
    \par Parameters:
    <b>float angle</b>\n\n
    The rotation angle in radians. */
    GEOMEXPORT void SetRotate(float angle);        // makes rotation matrix
    /*! \remarks This matrix may be used to invert the matrix in place. */
    GEOMEXPORT void Invert();
		
	// Binary operators		
	/*! \remarks Perform matrix multiplication. */
	GEOMEXPORT Matrix2 operator*(const Matrix2& B) const;	
	/*! \remarks Perform matrix addition. */
	GEOMEXPORT Matrix2 operator+(const Matrix2& B) const;
	/*! \remarks Perform matrix subtraction.\n\n
	  */
	GEOMEXPORT Matrix2 operator-(const Matrix2& B) const;

	GEOMEXPORT IOResult Save(ISave* isave);
	GEOMEXPORT IOResult Load(ILoad* iload);

	};

// Build new matrices for transformations
/*! \remarks Builds an identity matrix and sets the rotation components based
on the specified angle.
\par Parameters:
<b>float angle</b>\n\n
Specifies the angle of rotation.
\return  A new Matrix2 object with the specified rotation angle. */
GEOMEXPORT Matrix2 RotateMatrix(float angle);   
/*! \remarks Builds an identity matrix and sets the specified translation
components.
\par Parameters:
<b>const Point2\& p</b>\n\n
Specifies the translation.
\return  A new Matrix2 object with the specified translation. */
GEOMEXPORT Matrix2 TransMatrix(const Point2& p);
/*! \remarks Builds an identity matrix and sets the specified scaling
components.
\par Parameters:
<b>const Point2\& s</b>\n\n
Specifies the scale factors.
\return  A new Matrix2 object with the specified scale. */
GEOMEXPORT Matrix2 ScaleMatrix(const Point2& s);
 
/*! \remarks Returns the inverse of the specified Matrix2.
\par Parameters:
<b>const Matrix2\& M</b>\n\n
Specifies the matrix to return the inverse of.
\return  The inverse of the specified Matrix2. */
GEOMEXPORT Matrix2 Inverse(const Matrix2& M);

// Transform point with matrix:
/*! \remarks Transforms the specified Point2 with the specified Matrix2.
\par Parameters:
<b>const Matrix2\& A</b>\n\n
The matrix to transform the point with.\n\n
<b>const Point2\& V</b>\n\n
The point to transform.
\return  The transformed Point2. */
GEOMEXPORT Point2 operator*(const Matrix2& A, const Point2& V);
/*! \remarks Transforms the specified Point2 with the specified Matrix2.
\par Parameters:
<b>const Point2\& V</b>\n\n
The point to transform.
<b>const Matrix2\& A</b>\n\n
The matrix to transform the point with.\n\n
\return  The transformed Point2. */
GEOMEXPORT Point2 operator*( const Point2& V, const Matrix2& A);
/*! \remarks This method transforms a 2D point by a 2x3 matrix. This is
analogous to the 3D case.
\par Parameters:
<b>const Matrix2\& M</b>\n\n
The matrix to transform the point with.\n\n
<b>const Point2\& V</b>\n\n
The point to transform.
\return  The transformed Point2. */
GEOMEXPORT Point2 VectorTransform(const Matrix2& M, const Point2& V);

// Printout
GEOMEXPORT std::ostream &operator<<(std::ostream& s, const Matrix2& A);

