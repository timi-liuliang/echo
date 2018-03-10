
/**********************************************************************
 *<
	FILE: templt.h

	DESCRIPTION:  Defines 2D Template Object

	CREATED BY: Tom Hudson

	HISTORY: created 31 October 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "maxheap.h"
#include "CoreExport.h"
#include <WTypes.h>
#include "point2.h"
#include "point3.h"
#include "box3.h"
// forward declarations
class PolyLine;
class Spline3D;

// Intersection callbacks

class IntersectionCallback2D: public MaxHeapOperators {
	public:
		virtual ~IntersectionCallback2D() {}
		virtual BOOL Intersect(Point2 p, int piece)=0; // Return FALSE to stop intersect tests
	};

class IntersectionCallback3D: public MaxHeapOperators {
	public:
		virtual ~IntersectionCallback3D() {}
		virtual BOOL Intersect(Point3 p, int piece)=0; // Return FALSE to stop intersect tests
	};

// A handy 2D floating-point box class

/*! \sa  Class Point2.\n\n
\par Description:
A 2D floating-point box class. This class has methods and operators to clear
the box, and update its size (bounding rectangle) by specifying additional
points. All methods of this class are implemented by the system.
\par Data Members:
<b>BOOL empty;</b>\n\n
Indicates if the box is empty. When the += operator is used to update the size
of the box, if the box is empty, the box corners are set to the point.\n\n
<b>Point2 min, max;</b>\n\n
The corners of the 2D box.  */
class Box2D: public MaxHeapOperators {
	public:
		BOOL empty;
		Point2 min, max;
		/*! \remarks Constructor. The box is set to empty initially. */
		Box2D() { empty = TRUE; }
		/*! \remarks Sets the box to an empty status.
		\par Operators:
		*/
		void SetEmpty() { empty = TRUE; }
		/*! \remarks Expand this box to include <b>p</b>. If this box is
		empty, the box corners are set to the point <b>p</b>.
		\par Parameters:
		<b>const Point2\& p</b>\n\n
		This box is expanded to include <b>p</b>. */
		CoreExport Box2D& operator+=(const Point2& p);	// expand this box to include p
	};

// This object is used to test shapes for self-intersection, clockwise status, point
// surrounding and intersection with other templates.  The last and first points will be the
// same if it is closed.

class Template3D;

/*! \sa  Class PolyShape, Class Box2D, Class Spline3D, Class PolyLine.\n\n
\par Description:
This class defines a 2D template object. This object is used to test shapes for
self-intersection, clockwise status, point surrounding and intersection with
other templates. The last and first points will be the same if it is closed.
All methods of this class are implemented by the system.\n\n
<b>Note:</b> Developers should use the PolyShape class instead of this class.
It provides methods for the same purposes.
\par Data Members:
<b>int points;</b>\n\n
The number of points in the template.\n\n
<b>BOOL closed;</b>\n\n
Indicates if the template is closed.\n\n
<b>Point2 *pts;</b>\n\n
The template points.  */
class Template: public MaxHeapOperators {
	public:
		int points;
		BOOL closed;
		Point2 *pts;
		/*! \remarks Constructor.
		\par Parameters:
		<b>Spline3D *spline</b>\n\n
		Builds the template from this spline. */
		Template(Spline3D *spline);
		/*! \remarks Constructor.
		\par Parameters:
		<b>PolyLine *line</b>\n\n
		Builds the template from this polyline. */
		Template(PolyLine *line);
		Template(Template3D *t3);
		/*! \remarks Updates the template with the data from the specified
		<b>PolyLine</b>.
		\par Parameters:
		<b>PolyLine *line</b>\n\n
		Builds the template from this polyline. */
		void Create(PolyLine *line);
		/*! \remarks Destructor. */
		~Template();
		/*! \remarks Returns the number of points. */
		int Points() { return points; }
		/*! \remarks Returns TRUE if the specified point is surrounded
		(contained within) this Template.
		\par Parameters:
		<b>Point2\& point</b>\n\n
		The point to check. */
		BOOL SurroundsPoint(Point2& point);
		/*! \remarks Returns TRUE if the Template is clockwise in the XY plane
		(it ignores Z); otherwise FALSE. If the Template self intersects, the
		results from this method are meaningless. */
		BOOL IsClockWise();
		/*! \remarks Returns TRUE if the Template intersects itself in the XY
		plane (it ignores Z); otherwise FALSE.
		\par Parameters:
		<b>BOOL findAll = FALSE</b>\n\n
		TRUE to find all self intersections. FALSE to find only the first self
		intersection.\n\n
		<b>IntersectionCallback2D *cb = NULL</b>\n\n
		A pointer to an IntersectionCallback2D class. */
		BOOL SelfIntersects(BOOL findAll = FALSE, IntersectionCallback2D *cb = NULL);
		/*! \remarks Returns TRUE if the specified Template intersects this
		Template in the XY plane (it ignores Z); otherwise FALSE.
		\par Parameters:
		<b>Template \&t</b>\n\n
		The Template to check.\n\n
		<b>BOOL findAll = FALSE</b>\n\n
		TRUE to find all self intersections. FALSE to find only the first self
		intersection.\n\n
		<b>IntersectionCallback2D *cb = NULL</b>\n\n
		A pointer to an IntersectionCallback2D class. */
		BOOL Intersects(Template &t, BOOL findAll = FALSE, IntersectionCallback2D *cb = NULL);
		/*! \remarks Returns an instance of the <b>Box2D</b> class that
		contains two corner points defining the bounding box size of this
		template. */
		Box2D Bound();
	};

// This is a version for 3D use -- the various tests (SurroundsPoint, SelfIntersects, etc.
// are all performed on the X and Y coordinates only, discarding Z.  The IntersectionCallback
// returns the intersection point on the template in 3D.

class Template3D: public MaxHeapOperators {
	private:
		Template *template2D;
	public:
		int points;
		BOOL closed;
		Point3 *pts;
		Template3D(Spline3D *spline);
		Template3D(PolyLine *line);
		void Create(PolyLine *line);
		~Template3D();
		int Points() { return points; }
		BOOL SurroundsPoint(Point2& point);	// 2D test!
		BOOL IsClockWise(); // 2D test!
		BOOL SelfIntersects(BOOL findAll = FALSE, IntersectionCallback3D *cb = NULL);	// 2D test!
		BOOL Intersects(Template3D &t, BOOL findAll = FALSE, IntersectionCallback3D *cb = NULL); // 2D test!
		Box2D Bound();
		Box3 Bound3D();
		void Ready2DTemplate();
	};

