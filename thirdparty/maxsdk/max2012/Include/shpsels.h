/**********************************************************************
 *<
	FILE: shpsels.h

	DESCRIPTION:  Defines Shape Selection utility objects

	CREATED BY: Tom Hudson

	HISTORY: created 31 October 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#pragma once
#include <WTypes.h>
#include "coreexp.h"
#include "maxheap.h"
#include "maxtypes.h"
#include "bitarray.h"

// forward declarations
class BezierShape;
class PolyShape;
class ILoad;
class ISave;

/*! \sa  Class BitArray, Class PolyShape.\n\n
\par Description:
This class stores and provides access to shape vertex selection data. All
methods of this class are implemented by the system.
\par Data Members:
<b>int polys;</b>\n\n
The number of splines in the shape.\n\n
<b>BitArray *sel;</b>\n\n
An array of BitArrays, one for each spline.  */
class ShapeVSel: public MaxHeapOperators {
	public:
	int polys;
	BitArray *sel;
	/*! \remarks Constructor. The number of splines is set to 0. The BitArray
	pointer is set to NULL. */
	CoreExport ShapeVSel();
	CoreExport ShapeVSel(ShapeVSel& from);
	CoreExport ~ShapeVSel();
	/*! \remarks Assignment operator. */
	CoreExport ShapeVSel& operator=(ShapeVSel& from);
	CoreExport BOOL operator==(ShapeVSel& s);
	/*! \remarks Creates and inserts a new BitArray into <b>sel</b>.
	\par Parameters:
	<b>int where</b>\n\n
	The index into <b>sel</b> indicating where to insert the new BitArray.\n\n
	<b>int count=0</b>\n\n
	The number of bits in the new BitArray. */
	CoreExport void Insert(int where,int count=0);
	/*! \remarks Deletes the specified BitArray from the <b>sel</b> list.
	\par Parameters:
	<b>int where</b>\n\n
	The index into <b>sel</b> indicating which BitArray to delete. */
	CoreExport void Delete(int where);
	CoreExport void SetSize(ShapeVSel& selset, BOOL save=FALSE);
	/*! \remarks Sets the number of splines and allocates the corresponding
	number of BitArrays based on the shape passed. The size of each BitArray is
	set to the number of vertices in each polyline.
	\par Parameters:
	<b>BezierShape\& shape</b>\n\n
	The shape whose splines determine the sizes set.\n\n
	<b>BOOL save=FALSE</b>\n\n
	TRUE to keep the previous BitArray contents. FALSE to discard it.\n\n
	  */
	CoreExport void SetSize(BezierShape& shape, BOOL save=FALSE);
	/*! \remarks Sets the number of splines and allocates the corresponding
	number of BitArrays based on the shape passed. The size of each BitArray is
	set to the number of vertices in each spline.
	\par Parameters:
	<b>PolyShape\& shape</b>\n\n
	The shape whose lines determine the sizes set.\n\n
	<b>BOOL save=FALSE</b>\n\n
	TRUE to keep the previous BitArray contents. FALSE to discard it. */
	CoreExport void SetSize(PolyShape& shape, BOOL save=FALSE);
	CoreExport BitArray& operator[](int index);
	/*! \remarks Clears every bit for every poly. */
	CoreExport void ClearAll();
	CoreExport void SetAll();
	CoreExport void Toggle();
	/*! \remarks Sets the size of every poly BitArray to 0. */
	CoreExport void Empty();
	CoreExport BOOL IsCompatible(ShapeVSel& selset);
	CoreExport BOOL IsCompatible(BezierShape& shape);
	CoreExport BOOL IsCompatible(PolyShape& shape);
	/*! \remarks Saves the ShapeVSel to disk. */
	CoreExport IOResult Save(ISave* isave);
	/*! \remarks Loads the ShapeVSel from disk.
	\par Operators:
	*/
	CoreExport IOResult Load(ILoad* iload);

	//! \brief Delete multiple entries at once
	/*!	Deleting multiple entries at once is faster than deleting one by one by calling Delete(), because the internal bookkeeping only needs to be done once.

			\param[in] indices Point to an array of indices to delete.  Indices should be unique, and sorted in ascending order.
			\param[in] n Number of entries in the indices array.
			\return false if any of the vertex indices are invalid.
			\see ShapeVSel::Delete
	*/
	CoreExport bool Delete(const unsigned int* pIndices, unsigned int n);
	};

/*! \sa  Class BitArray, Class PolyShape.\n\n
\par Description:
This class stores and provides access to shape segment selection data. All
methods of this class are implemented by the system.
\par Data Members:
<b>int polys;</b>\n\n
The number of splines in the shape.\n\n
<b>BitArray *sel;</b>\n\n
An array of BitArrays, one for each spline.  */
class ShapeSSel: public MaxHeapOperators {
	public:
	int polys;
	BitArray *sel;
	/*! \remarks Constructor. Initialize the class members.\n\n
	Constructor. The number of splines is set to 0. The BitArray pointer is set
	to NULL. */
	CoreExport ShapeSSel();
	CoreExport ShapeSSel(ShapeSSel& from);
	/*! \remarks Destructor. Any BitArrays are freed. */
	CoreExport ~ShapeSSel();
	/*! \remarks Assignment operator. */
	CoreExport ShapeSSel& operator=(ShapeSSel& from);
	CoreExport BOOL operator==(ShapeSSel& s);
	/*! \remarks Creates and inserts a new BitArray into <b>sel</b>.
	\par Parameters:
	<b>int where</b>\n\n
	The index into <b>sel</b> indicating where to insert the new BitArray.\n\n
	<b>int count=0</b>\n\n
	The number of bits in the new BitArray. */
	CoreExport void Insert(int where,int count=0);
	/*! \remarks Deletes the specified BitArray from the <b>sel</b> list.
	\par Parameters:
	<b>int where</b>\n\n
	The index into <b>sel</b> indicating which BitArray to delete. */
	CoreExport void Delete(int where);
	CoreExport void SetSize(ShapeSSel& selset, BOOL save=FALSE);
	/*! \remarks Sets the number of splines and allocates the corresponding
	number of BitArrays based on the shape passed. The size of each BitArray is
	set to the number of segments in each polyline.
	\par Parameters:
	<b>BezierShape\& shape</b>\n\n
	The shape whose splines determine the sizes set.\n\n
	<b>BOOL save=FALSE</b>\n\n
	TRUE to keep the previous BitArray contents. FALSE to discard it. */
	CoreExport void SetSize(BezierShape& shape, BOOL save=FALSE);
	/*! \remarks Sets the number of splines and allocates the corresponding
	number of BitArrays based on the shape passed. The size of each BitArray is
	set to the number of segments in each spline.
	\par Parameters:
	<b>PolyShape\& shape</b>\n\n
	The shape whose lines determine the sizes set.\n\n
	<b>BOOL save=FALSE</b>\n\n
	TRUE to keep the previous BitArray contents. FALSE to discard it. */
	CoreExport void SetSize(PolyShape& shape, BOOL save=FALSE);
	CoreExport BitArray& operator[](int index);
	/*! \remarks Clears every bit for every poly. */
	CoreExport void ClearAll();
	CoreExport void SetAll();
	CoreExport void Toggle();
	/*! \remarks Sets the size of every poly BitArray to 0. */
	CoreExport void Empty();
	CoreExport BOOL IsCompatible(ShapeSSel& selset);
	CoreExport BOOL IsCompatible(BezierShape& shape);
	CoreExport BOOL IsCompatible(PolyShape& shape);
	/*! \remarks Saves the ShapeSSel to disk. */
	CoreExport IOResult Save(ISave* isave);
	/*! \remarks Loads the ShapeSSel from disk.
	\par Operators:
	*/
	CoreExport IOResult Load(ILoad* iload);

	//! \brief Delete multiple entries at once
	/*!	Deleting multiple entries at once is faster than deleting one by one by calling Delete(), because the internal bookkeeping only needs to be done once.

			\param[in] indices Point to an array of indices to delete.  Indices should be unique, and sorted in ascending order.
			\param[in] n Number of entries in the indices array.
			\return false if any of the vertex indices are invalid.
			\see ShapeSSel::Delete
	*/
	CoreExport bool Delete(const unsigned int* pIndices, unsigned int n);
	};

/*! \sa  Class BitArray, Class PolyShape.\n\n
\par Description:
This class stores and provides access to shape polygon (spline) selection data.
All methods of this class are implemented by the system.
\par Data Members:
<b>int polys;</b>\n\n
The number of splines in the shape.\n\n
<b>BitArray sel;</b>\n\n
One bit for each spline in the shape.  */
class ShapePSel: public MaxHeapOperators {
	public:
	int polys;
	BitArray sel;
	/*! \remarks Constructor. Initialize the class members. */
	CoreExport ShapePSel();
	CoreExport ShapePSel(ShapePSel& from);
	/*! \remarks Destructor. */
	CoreExport ~ShapePSel();
	/*! \remarks Assignment operator. */
	CoreExport ShapePSel& operator=(ShapePSel& from);
	CoreExport BOOL operator==(ShapePSel& s);
	/*! \remarks Resizes the BitArray <b>sel</b> to include a new bit at the
	specified location.
	\par Parameters:
	<b>int where</b>\n\n
	The location for the new bit in the BitArray. */
	CoreExport void Insert(int where);
	/*! \remarks Deletes the specified bit from the BitArray.
	\par Parameters:
	<b>int where</b>\n\n
	Indicates which bit to delete. */
	CoreExport void Delete(int where);
	CoreExport void SetSize(ShapePSel& selset, BOOL save=FALSE);
	/*! \remarks Sets the number of splines and resizes the BitArray based on
	the shape passed.
	\par Parameters:
	<b>BezierShape\& shape</b>\n\n
	The shape whose splines determine the sizes set.\n\n
	<b>BOOL save=FALSE</b>\n\n
	TRUE to keep the previous BitArray contents. FALSE to discard it. */
	CoreExport void SetSize(BezierShape& shape, BOOL save=FALSE);
	/*! \remarks Sets the number of splines and resizes the BitArray based on
	the shape passed.
	\par Parameters:
	<b>PolyShape\& shape</b>\n\n
	The shape whose lines determine the sizes set.\n\n
	<b>BOOL save=FALSE</b>\n\n
	TRUE to keep the previous BitArray contents. FALSE to discard it. */
	CoreExport void SetSize(PolyShape& shape, BOOL save=FALSE);
	/*! \remarks Sets the bit specified by the index to 1.
	\par Parameters:
	<b>int index</b>\n\n
	The bit to set. */
	CoreExport void Set(int index);
	/*! \remarks Sets the bit specified by the index to the value passed.
	\par Parameters:
	<b>int index</b>\n\n
	The bit to set or clear.\n\n
	<b>int value</b>\n\n
	The value to set, either 0 or 1. */
	CoreExport void Set(int index, int value);
	/*! \remarks Clears the bit specified by the index to 1.
	\par Parameters:
	<b>int index</b>\n\n
	The bit to clear. */
	CoreExport void Clear(int index);
	/*! \remarks Array access operator. */
	CoreExport int operator[](int index) const;
	/*! \remarks Clears all the bits in the array (sets them to 0). */
	CoreExport void ClearAll();
	CoreExport void SetAll();
	CoreExport void Toggle();
	/*! \remarks Sets the size of <b>sel</b> to 0. */
	CoreExport void Empty();
	CoreExport BOOL IsCompatible(ShapePSel& selset);
	CoreExport BOOL IsCompatible(BezierShape& shape);
	CoreExport BOOL IsCompatible(PolyShape& shape);
	/*! \remarks Saves the BitArray to disk. */
	CoreExport IOResult Save(ISave* isave);
	/*! \remarks Loads the BitArray from disk.
	\par Operators:
	*/
	CoreExport IOResult Load(ILoad* iload);
	//! \brief Delete multiple entries at once
	/*!	Deleting multiple entries at once is faster than deleting one by one by calling Delete(), because the internal bookkeeping only needs to be done once.

			\param[in] indices Point to an array of indices to delete.  Indices should be unique, and sorted in ascending order.
			\param[in] n Number of entries in the indices array.
			\return false if any of the vertex indices are invalid.
			\see ShapePSel::Delete
	*/
	CoreExport bool Delete(const unsigned int* pIndices, unsigned int n);
	};


