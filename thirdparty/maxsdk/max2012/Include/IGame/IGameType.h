/**********************************************************************
 *<
	FILE: IGameType.h

	DESCRIPTION:	General definition for IGame types

	CREATED BY:		Alex Zadorozhny

	HISTORY:		05/12/2005

	IGame Version: 1.122

 *>	Copyright (c) 2005, All Rights Reserved.
 **********************************************************************/

/*!\file IGameType.h
\brief IGame data types definition
*/

#pragma once

#include "..\maxheap.h"
#include "IGameExport.h"
#include "..\point3.h"
#include "..\matrix3.h"
// forward declarations
class Point4;


//! GMatrix row
typedef float GRow[4]; 


//! This class implements a 4x4 matrix object
class GMatrix : public MaxHeapOperators
{
	private:
		float m[4][4];
		friend IGAMEEXPORT Point3  operator*(const Point3& V, const GMatrix& A);
		
	public:
        //! \name Subscript Operator
		//@{
		/*! Return a reference to the 'i-th' Point4 of the matrix */
		Point4& operator[](int i) { return((Point4&)(*m[i]));}  
		const Point4& operator[](int i) const { return((Point4&)(*m[i])); }
        //@}
		
		//!Return the Address of the GMatrix.  This allows direct access via the [] operator
		/*!
		\returns The address of the GMatrix
		*/
		GRow * GetAddr(){return (GRow *)(m);}

		//!Return the Address of the GMatrix.  This allows direct access via the [] operator.  
		/*! This method is const aware.
		\returns The address of the GMatrix
		*/
		const GRow* GetAddr() const { return (GRow *)(m); }
		
		//! Default constructor, set Identity matrix.
		IGAMEEXPORT GMatrix();

		//!Constructor from a Matrix3
		IGAMEEXPORT GMatrix(Matrix3);

		//! Set all values to 0.0f
		void ResetMatrix();
		
		//! Determine whether or not the matrix is an Identity matrix
		/*! within the specified tolerance
		\param epsilon Tolerance defaulted to epsilon = 1E-5f
		\returns True if the matrix is the Identity matrix 
		*/
		IGAMEEXPORT BOOL IsIdentity(float epsilon = 1E-5f);

		//! Set the Standard Identity Matrix
		IGAMEEXPORT void SetIdentity();

		//! Return Determinant of matrix
		IGAMEEXPORT float Determinant() const;

		//! Return Inverse of matrix
		IGAMEEXPORT GMatrix Inverse();
      
		//! Return a translation transformation
		IGAMEEXPORT Point3 Translation () const;

		//! Return a essential rotation transformation
		IGAMEEXPORT Quat Rotation () const;

		//! Return a scaling transformation
		IGAMEEXPORT Point3 Scaling () const;

		//! Return a stretch transformation. 
		/*! This is the axis system of the scaling application 
		*/
		Quat ScaleRotation () const;

		//! Return 'parity' sign of the matrix, -1 if negative, 1 otherwise
		IGAMEEXPORT int Parity() const;
		
		//! Access to the matrix column.
		/*!\param i The number of the column to retrieve
		\returns A Point4 containing the column
		*/
		IGAMEEXPORT Point4 GetColumn(int i) const; 
		
		//! Set the the matrix column.
		/*\param i The number of the column to set
		\param col Point4 containing the column to set
		*/
		IGAMEEXPORT void SetColumn(int i,  Point4 col); 

		//! Access to the matrix row.
		/*\param i The number of the row to retrieve
		\returns A Point4 containing the row
		*/
		Point4 GetRow(int i) const { return (*this)[i]; }	

		//! Set to the matrix row to the desired data.
		/*\param i The number of the row to set
		\param row A Point4 containing the row
		*/
		IGAMEEXPORT void SetRow(int i, Point4 row);

		//! \name Binary operators
		//@{
		IGAMEEXPORT GMatrix operator-(const GMatrix&)const;
		IGAMEEXPORT GMatrix operator*(const GMatrix&)const;
		IGAMEEXPORT GMatrix operator+(const GMatrix&)const;
		//@}
		
		//! \name Assignment operators 
		//@{
		IGAMEEXPORT GMatrix& operator-=( const GMatrix& M);
		IGAMEEXPORT GMatrix& operator+=( const GMatrix& M); 
		IGAMEEXPORT GMatrix& operator*=( const GMatrix& M);  	// Matrix multiplication
		IGAMEEXPORT GMatrix& operator*=( const float a);
		IGAMEEXPORT GMatrix& operator=(const Matrix3&);  /*!< Assignment operator from a Matrix3 */
		//@}

		//! \name Comparison operators
		//@{
		IGAMEEXPORT BOOL operator==(const GMatrix& M) const;
		IGAMEEXPORT BOOL Equals(const GMatrix& M, float epsilon = 1E-5f) const;  //!< Test for equality within the specified tolerance
        //@}

		//!Extract a Matrix3 from the GMatrix
		/*!This is for backward compatibility.  This is only of use if you use 3ds Max as a coordinate system, other wise
		standard 3ds Max algebra might not be correct for your format.
		\returns A 3ds Max Matrix3 form of the GMatrix
		*/
		IGAMEEXPORT Matrix3 ExtractMatrix3()const ;
		
};

//! Multiply a GMatrix with a Point3
IGAMEEXPORT Point3 operator*(const Point3& V, const GMatrix& A);


