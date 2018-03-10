/**********************************************************************
 *<
	FILE: decomp.h

	DESCRIPTION:

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "maxheap.h"
#include "point3.h"
#include "quat.h"

/*! \sa  Class Matrix3, Class Quat, Class Point, Class ScaleValue.
\remarks This structure and the associated functions provide a way
to decompose an arbitrary Matrix3 into its translation, rotation, and scale components. \n\n
To use these APIs put the following statement in your source file:\n
<b>#include "decomp.h"</b> \n\n
For a full discussion of this decomposition see Graphics Gems IV -
Polar Matrix Decomposition by Ken Shoemake. ISBN 0-12-336155-9. \n\n
<b>T F R U K U'</b> \n
T - translation matrix \n
F - either an identity matrix or negative identity matrix \n
R - rotation defined by Quat q. \n
U - rotates you into the coordinates system where the scaling or stretching is done \n
K - scaling matrix \n
U' - inverse of u.
\par See Functions:
SpectralDecomp(Matrix3 m, Point3 &s, Quat& q); \n
decomp_affine(Matrix3 A, AffineParts *parts); \n
invert_affine(AffineParts *parts, AffineParts *inverse);
*/
struct AffineParts: public MaxHeapOperators {
	/*! The translation components. */
	Point3 t;	/* Translation components */
	/*! The essential rotation. */
	Quat q;	/* Essential rotation	  */
	/*! The stretch rotation. This is the axis system of the scaling application. */
	Quat u;	/* Stretch rotation	  */
	/*! The stretch factors. These are the scale factors for x, y and z. */
	Point3 k;	/* Stretch factors	  */
	/*! Sign of the determinant. */
	float f;	/* Sign of determinant	  */
	};

/*! \remarks This is another way to decompose a matrix into the scale
and rotation parts (the position part can be retrieved from the bottom
row of the matrix). This does not return correct results for off axis scale.
\par Parameters:
<b>Matrix3 m</b> \n
The input matrix to decompose.\n\n
<b>Point3 &s</b> \n
The scale from the matrix.\n\n
<b>Quat& q</b> \n
The rotation of the matrix.
*/
CoreExport void SpectralDecomp(Matrix3 m, Point3 &s, Quat& q);

/*! \remarks This will decompose a matrix into the translation,
rotation and scale components and store the results in the AffineParts
structure passed. This will return correct results for off axis
scale. This is a fairly computationally intensive iterative solution operation.
\par Parameters:
<b>Matrix3 A</b> \n
The input matrix to decompose.\n\n
<b>AffineParts *parts</b> \n
The result. See above.\n\n
\par Sample Code:
Note: If you want to rebuild a Matrix3 from the decomposed parts you get
back from decomp_affine() the important thing is the order the parts are combined.\n\n
Consider the following matrices constructed from the various affine parts:\n
<b>ptm</b> = position component (t) \n
<b>rtm</b> = "essential" rotation (q) \n
<b>srtm</b> = "stretch" rotation (u) \n
<b>stm</b> = scale component (k) \n
<b>ftm</b> = the flip tm -> ScaleMatrix(Point3(ap.f,ap.f,ap.f)); \n\n
Here's the correct way of reassembling the decomposed matrix: \n
\code
Matrix3 srtm, rtm, ptm, stm, ftm;
ptm.IdentityMatrix();
ptm.SetTrans(ap.t);
ap.q.MakeMatrix(rtm);
ap.u.MakeMatrix(srtm);
stm = ScaleMatrix(ap.k);
mat = Inverse(srtm) * stm * srtm * rtm * ftm * ptm;
\endcode
*/
CoreExport void decomp_affine(Matrix3 A, AffineParts *parts);

/*! \remarks This is used to take the AffineParts and inverts them.
This gives you the equivalent parts for the inverse matrix.
\par Parameters:
<b>AffineParts *parts</b> \n
The input AffineParts pointer. \n\n
<b>AffineParts *inverse</b> \n
The inverse of parts.
*/
CoreExport void invert_affine(AffineParts *parts, AffineParts *inverse);

