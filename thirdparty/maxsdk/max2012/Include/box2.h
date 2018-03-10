/**********************************************************************
 *<
	FILE: box2.h

	DESCRIPTION:

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "GeomExport.h"
#include "maxheap.h"
#include "ipoint2.h"
#include "point2.h"
#include <wtypes.h>

/*! \sa  Class IPoint2, <a href="ms-its:3dsmaxsdk.chm::/start_data_types.html">Data Types</a>.\n\n
\par Description:
This class describes a 2D rectangular region using integer coordinates. This
class is sub-classed from RECT (from the Windows API). Box2 provides methods
that return individual coordinates of the box, scale and translate it, retrieve
its center, modify its size, expand it to include points or other boxes, and
determine if points are inside the box. All methods are implemented by the
system.  */
class Box2: public RECT, public MaxHeapOperators {
	public:
	/*! \remarks Constructs a Box2 object. The box is initialized such that it
	is 'empty'. See <b>IsEmpty()</b> below. */
	GEOMEXPORT Box2();
	/*! \remarks Constructs a Box2 object from the specified corners.
	\par Parameters:
	<b>const IPoint2 a</b>\n\n
	The upper left corner of the box.\n\n
	<b>const IPoint2 b</b>\n\n
	The lower right corner of the box. */
	GEOMEXPORT Box2(const IPoint2 a, const IPoint2 b);
	/*! \remarks Determines whether the box has been 'Set Empty' (see below).
	When a box is created using the default constructor it is set to 'empty'.
	\return  TRUE if the box is empty; FALSE otherwise. */
	GEOMEXPORT int IsEmpty();
	/*! \remarks Sets the box to 'empty'. This indicates the box has not had
	specific values set by the developer. */
	GEOMEXPORT void SetEmpty();
	/*! \remarks Adjusts the coordinates of the box such that
	<b>top\<bottom</b> and <b>left\<right</b>. */
	GEOMEXPORT void Rectify();   // makes top<bottom, left<right
	/*! \remarks Scales the coordinates of the box about the center of the
	box.
	\par Parameters:
	<b>float f</b>\n\n
	Specifies the scale factor. */
	GEOMEXPORT void Scale(float f);
	/*! \remarks Translate the box by the distance specified.
	\par Parameters:
	<b>IPoint2 t</b>\n\n
	The distance to translate the box. */
	GEOMEXPORT void Translate(IPoint2 t);

	/*! \remarks Returns the center of the box (the midpoint between the box
	corners). */
	IPoint2 GetCenter() { return IPoint2((left+right)/2, (top+bottom)/2); }
	/*! \remarks Returns the minimum x coordinate of the box. */
	int x() { return min(left,right); }
	/*! \remarks Returns the minimum y coordinate. */
	int y() { return min(top,bottom); }
	/*! \remarks Returns the width of the box. */
	int w() { return abs(right-left)+1; }
	/*! \remarks Returns the height of the box. */
	int h() { return abs(bottom-top)+1; }
	
	/*! \remarks Sets the box width to the width specified. The 'right'
	coordinate is adjusted such that:\n\n
	<b>right = left + w -1</b>
	\par Parameters:
	<b>int w</b>\n\n
	The new width for the box. */
	void SetW(int w) { right = left + w -1; } 
	/*! \remarks Sets the height of the box to the height specified. The
	'bottom' coordinate is adjusted such that:\n\n
	<b>bottom = top + h -1;</b>
	\par Parameters:
	<b>int h</b>\n\n
	The new height for the box. */
	void SetH(int h) { bottom = top + h -1; } 
	/*! \remarks Sets the left coordinate of the box to x.
	\par Parameters:
	<b>int x</b>\n\n
	The new value for the left coordinate. */
	void SetX(int x) { left = x; }
	/*! \remarks Set the top coordinate to y.
	\par Parameters:
	<b>int y</b>\n\n
	The new value for the top coordinate. */
	void SetY(int y) { top = y; }
	/*! \remarks Sets both the width and height of the box.
	\par Parameters:
	<b>int w</b>\n\n
	The new width for the box.\n\n
	<b>int h</b>\n\n
	The new height of the box. */
	void SetWH(int w, int h) { SetW(w); SetH(h); }
	/*! \remarks Sets both the left and top coordinates of the box.
	\par Parameters:
	<b>int x</b>\n\n
	The new left coordinate.\n\n
	<b>int y</b>\n\n
	The new top coordinate. */
	void SetXY(int x, int y) { SetX(x); SetY(y); }

	/*! \remarks Assignment operators. Copies the specified source RECT into this
	Box2 object. */
	GEOMEXPORT Box2& operator=(const RECT& r);
	/*! \remarks Assignment operators. Copies the specified source RECT into this
	Box2 object. */
	GEOMEXPORT Box2& operator=(RECT& r);
	/*! \remarks Expands this <b>Box2</b> to completely include box <b>b</b>.
	*/
	GEOMEXPORT Box2& operator+=(const Box2& b);
	/*! \remarks Expands this <b>Box2</b> to include point <b>p</b>. */
	GEOMEXPORT Box2& operator+=(const IPoint2& p);
	/*! \remarks Equality operator. Determines whether b is equal to Box2.
	Returns nonzero if the boxes are equal; 0 otherwise. */
	int operator==( const Box2& b ) const { 	return (left==b.left && right==b.right && top==b.top && bottom==b.bottom); }
	/*! \remarks Determines if the point passed is contained within the box.
	Returns nonzero if the point is inside the box; otherwise 0.
	\par Operators:
	*/
	GEOMEXPORT int Contains(const IPoint2& p) const;  // is point in this box?
	};

typedef Box2 Rect;


struct FBox2: public MaxHeapOperators {
	Point2 pmin;
	Point2 pmax;
	int IsEmpty() { return pmin.x>pmax.x?1:0; }
	void SetEmpty() { pmin = Point2(1E30,1E30); pmax = -pmin; }
	FBox2& operator=(const FBox2& r) { pmin = r.pmin; pmax = r.pmax; return *this; }
	GEOMEXPORT FBox2& operator+=(const Point2& p);
	GEOMEXPORT FBox2& operator+=(const FBox2& b);
	GEOMEXPORT int Contains(const Point2& p) const;  // is point in this box?
	};

