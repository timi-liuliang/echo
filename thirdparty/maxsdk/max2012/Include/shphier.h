/**********************************************************************
 *<
	FILE: shphier.h

	DESCRIPTION:  Defines Shape Hierarchy Class

	CREATED BY: Tom Hudson

	HISTORY: created 30 December 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "maxheap.h"
#include "genhier.h"
#include "bitarray.h"

// This class stores the hierarchy tree of a shape object, along with
// a bitarray with an entry for each polygon in the shape which indicates
// whether that polygon should be reversed in order to provide the proper
// clockwise/counterclockwise ordering for the nested shapes.

/*! \sa  Class GenericHierarchy, Class BitArray.\n\n
\par Description:
This class stores the hierarchy tree of a shape object. In addition it stores a
<b>BitArray</b> with an entry for each polygon in the shape which indicates
whether that polygon should be reversed in order to provide the proper
clockwise/counterclockwise ordering for the nested shapes. All methods of this
class are implemented by the system.
\par Data Members:
<b>GenericHierarchy hier;</b>\n\n
Describes the hierarchy.\n\n
<b>BitArray reverse;</b>\n\n
Indicates whether that polygon should be reversed in order to provide the
proper clockwise / counterclockwise ordering for the nested shapes. There is
one bit in the bit array for every polygon in the shape. For example, if you
pass in two nested circles and they are both clockwise, the outermost circle
will have its reverse bit set to indicate it should be reversed in order to be
properly extruded or lofted. This is because for nested shapes the outermost
circle should be counterclockwise.  */
class ShapeHierarchy: public MaxHeapOperators {
	public:
		GenericHierarchy hier;
		BitArray reverse;
		/*! \remarks Constructor. No initialization is performed. */
		ShapeHierarchy() {}
		/*! \remarks Constructor. This constructor clears out the hierarchy,
		sets the number of polygons to <b>poly</b> and clears all the bits in
		the BitArray.
		\par Parameters:
		<b>int polys</b>\n\n
		The number of polygons in the hierarchy. */
		ShapeHierarchy(int polys) { New(polys); }
		/*! \remarks This methods clears out the hierarchy, sets the number of
		polygons to <b>poly</b> and clears all the bits in the BitArray.
		\par Parameters:
		<b>int polys = 0</b>\n\n
		The number of polygons in the hierarchy.
		\par Operators:
		*/
		void New(int polys = 0) { hier.New(); reverse.SetSize(polys); reverse.ClearAll(); }
		/*! \remarks Assignment operator.
		\par Parameters:
		<b>ShapeHierarchy \&from</b>\n\n
		The source shape hierarchy. */
		ShapeHierarchy &operator=(ShapeHierarchy &from) { hier=from.hier; reverse=from.reverse; return *this; }
	};

