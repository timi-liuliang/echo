/**********************************************************************
 *<
	FILE: box3.h

	DESCRIPTION: 3D Box class

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "GeomExport.h"
#include "maxheap.h"
#include "point3.h"
#include "matrix3.h"

/*! \sa  Class Point3, Class Matrix3.\n\n
\par Description:
This class represents a 3D box volume described by two 3D corner coordinates.
Box3 provides methods that return individual coordinates of the box, scale and
translate it, retrieve its center, modify its size, expand it to include points
or other boxes, and determine if points are inside the box. All methods are
implemented by the system.
\par Data Members:
<b>Point3 pmin,pmax;</b>\n\n
The corners of the 3D box.  */
class Box3: public MaxHeapOperators {
	public:
		Point3 pmin,pmax;
		/*! \remarks Constructor. The corners of the box are initialized such
		that the box is 'empty'. See <b>IsEmpty()</b>. */
		GEOMEXPORT Box3();
		/*! \remarks Constructor. The corners of the box are initialized to
		the points passed. <b>pmin=p; pmax = q</b>. */
		Box3(const Point3& p, const Point3& q) { pmin = p; pmax = q;}
		/*! \remarks Initializes this box such that <b>pmin</b> is a very
		large value while <b>pmax</b> is a small value. Thus the box is
		'empty'. See <b>IsEmpty()</b>. */
		GEOMEXPORT void Init();

		/*! \remarks Modifies this box such that half the side length is
		subtracted from <b>pmin</b> and added to <b>pmax</b>. This creates a
		cube with the specified center <b>p</b> and side length <b>side</b>.
		\par Parameters:
		<b>const Point3\& p</b>\n\n
		Specifies the center point of the cube.\n\n
		<b>float side</b>\n\n
		Specifies the side length. */
		GEOMEXPORT void MakeCube(const Point3& p, float side);

		// Access
		/*! \remarks Returns the value of corner <b>pmin</b>. */
		Point3 Min() const { return pmin; }
		/*! \remarks Returns the value of corner <b>pmax</b>. */
		Point3 Max() const { return pmax; }
		/*! \remarks Returns the center of this Box3 as a Point3. */
		Point3 Center() const { return(pmin+pmax)/(float)2.0; }
		/*! \remarks Returns the width of the box as a Point3. This is
		<b>pmax-pmin</b>. */
		Point3 Width() const { return(pmax-pmin); }

		/* operator[] returns ith corner point: (i == (0..7) )
			Mapping:
			        X   Y   Z
			[0] : (min,min,min)
			[1] : (max,min,min)
			[2] : (min,max,min)
			[3] : (max,max,min)
			[4] : (min,min,max)
			[5] : (max,min,max)
			[6] : (min,max,max)
			[7] : (max,max,max)
			*/
		/*! \remarks Operator[] returns the 'i-th' corner point:\n\n
		<b>Mapping : X Y Z</b>\n\n
		<b>[0] : (min,min,min)</b>\n\n
		<b>[1] : (max,min,min)</b>\n\n
		<b>[2] : (min,max,min)</b>\n\n
		<b>[3] : (max,max,min)</b>\n\n
		<b>[4] : (min,min,max)</b>\n\n
		<b>[5] : (max,min,max)</b>\n\n
		<b>[6] : (min,max,max)</b>\n\n
		<b>[7] : (max,max,max)</b>
		\par Parameters:
		<b>int i</b>\n\n
		Specifies the corner to retrieve (0 \<= i \<= 7)
		\return  The 'i-th' corner point as a Point3. */
		GEOMEXPORT Point3 operator[](int i) const;	  

		// Modifiers
		/*! \remarks Expands this Box3 to include the Point3 <b>p</b>.
		\par Parameters:
		<b>const Point3\& p</b>\n\n
		Specifies the point to expand the box to include. */
		GEOMEXPORT Box3& operator+=(const Point3& p);	// expand this box to include Point3
		/*! \remarks Expands this Box3 to include the Box3 <b>b</b>.
		\par Parameters:
		<b>const Box3\& b</b>\n\n
		Specifies the Box3 to expand this box to include. */
		GEOMEXPORT Box3& operator+=(const Box3& b);   // expand this box to include  Box3

		/*! \remarks Scales this box about its center by the specified scale.
		\par Parameters:
		<b>float s</b>\n\n
		Specifies the scale factor for this Box3. */
		GEOMEXPORT void Scale(float s); // scale box about center
		/*! \remarks Translates this box by the distance specified. The point
		is added to each corner.
		\par Parameters:
		<b>const Point3 \&p</b>\n\n
		Specifies the distance to translate the box. */
		GEOMEXPORT void Translate(const Point3 &p); // translate box
		/*! \remarks Enlarges this box. A Point3 is created from <b>s</b> as
		Point3(s,s,s) and added to <b>pmax</b> and subtracted from <b>pmin</b>.
		If the box is 'empty', the box is centered at (0,0,0) and then
		enlarged. */
		GEOMEXPORT void EnlargeBy(float s); // enlarge by this amount on all sides

		// include an array of points, optionally transformed by tm
		GEOMEXPORT void IncludePoints(Point3 *pts, int numpoints, Matrix3 *tm=NULL); 

		// Returns a box that bounds the 8 transformed corners of the input box.
		/*! \remarks Returns a box that bounds the 8 transformed corners of
		the input box.
		\par Parameters:
		<b>const Matrix3\& tm</b>\n\n
		Specifies the matrix to transform the box corners by. */
		GEOMEXPORT Box3 operator*(const Matrix3& tm) const;

		// Tests
		/*! \remarks Determines if the box is empty. This indicates the box
		has not had specific values set by the developer.
		\return  Nonzero if the box is empty; otherwise 0. */
		GEOMEXPORT int IsEmpty() const;   // is this box empty?
		/*! \remarks Determines if the specified point <b>p</b> is contained
		in this box.
		\par Parameters:
		<b>const Point3\& p</b>\n\n
		Specifies the point to check.
		\return  Nonzero if the specified point is contained in this box;
		otherwise 0. */
		GEOMEXPORT int Contains(const Point3& p) const;  // is point in this box?
		/*! \remarks Determines if the specified Box3 is contained totally
		within this box.
		\par Parameters:
		<b>const Box3\& b</b>\n\n
		Specifies the box to check.
		\return  Nonzero if the specified box is entirely contained within this
		box; otherwise 0.
		\par Operators:
		*/
		GEOMEXPORT int Contains(const Box3& b) const;  // is box b totally in this box?
		GEOMEXPORT int Intersects(const Box3& b) const;  // does  box b intersect this box at all?
		
		

	};


