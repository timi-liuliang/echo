// MNBigMat.h
// Created by Steve Anderson, Nov. 22 1996.

// BigMatrix is for when I need good old-fashioned mxn matrices.

// Classes:
// BigMatrix
#pragma once

#include <WTypes.h>
#include <stdio.h>
#include "export.h"
#include "maxheap.h"

#define BIGMAT_MAX_SIZE 50000  // LAM - defect 292187 - bounced up from 10000

/*! \sa  Class Matrix3.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class implements an m x n matrix for situations \& calculations where the
usual 4x3 Matrix3 class is not adequate. BigMatrix implements several useful
matrix operations, including matrix multiplication and inversion, but is not
guaranteed to be especially efficient. All methods are implemented by the
system.
\par Data Members:
<b>int m, n</b>\n\n
The dimensions of the matrix. There are <b>m</b> rows and <b>n</b> columns.\n\n
<b>float *val</b>\n\n
The elements of the matrix. val[i*n+j] is the value in the i'th row and the
j'th column.  */
class BigMatrix: public MaxHeapOperators {
public:
	int m, n;
	float *val;

	/*! \remarks Initializer. Sets m and n to zero, and val to NULL. */
	BigMatrix () { val=NULL; m=0; n=0; }
	/*! \remarks Initializer. Sets the dimensions of the matrix to mm by nn,
	and allocates space for the contents. The total size of the matrix, mm*nn,
	cannot exceed 10,000. */
	DllExport BigMatrix (int mm, int nn);
	/*! \remarks Initializer. Sets this BigMatrix equal to <b>from</b>. */
	DllExport BigMatrix (const BigMatrix & from);
	/*! \remarks Destructor. Frees the internal arrays. */
	~BigMatrix () { Clear (); }

	/*! \remarks Frees the internal arrays and sets the matrix's size to 0x0.
	*/
	DllExport void Clear ();
	/*! \remarks Sets the matrix's size to mm by nn, and allocates space for
	the contents.
	\return  Returns the total size of the matrix (mm * nn) or -1 if there's an
	error. */
	DllExport int SetSize (int mm, int nn);

	/*! \remarks Returns a pointer to the i'th row in the matrix. Thus for a
	BigMatrix A, A[i][j] is the value in the i'th row and the j'th column. */
	DllExport float *operator[](int i) const;
	/*! \remarks Sets this BigMatrix equal to from. */
	DllExport BigMatrix & operator= (const BigMatrix & from);

	/*! \remarks Sets trans to be the transpose of this BigMatrix. */
	DllExport void SetTranspose (BigMatrix & trans) const;
	/*! \remarks Inverts this matrix. Note that this only works if this matrix
	is "square", i.e. if m = n. This algorithm is CUBIC in the number of rows,
	so be careful!
	\return  The determinant of the matrix (before inversion) is returned. If
	the return value is 0, the matrix could not be inverted. */
	DllExport float Invert();
	/*! \remarks If m and n are equal, this method sets this matrix to the
	identity. If m and n are not equal, it does nothing. */
	DllExport void Identity ();

	// Debugging functions:
	/*! \remarks	Creates a random matrix for testing purposes. Reseeds the random number
	generator with the current system time, for a non-reproducible result.
	Values of the matrix are set to anything in the range (-scale, scale). */
	DllExport void Randomize (float scale);
	/*! \remarks	This method prints the contents of the BigMatrix to the IDE debugging
	window using DebugPrints. */
	DllExport void MNDebugPrint ();

	// Do not use -- does nothing.  (Replaced by MNDebugPrint.)
	DllExport void dump (FILE *fp);
};

DllExport extern BOOL BigMatMult (BigMatrix & a, BigMatrix & b, BigMatrix &c);

