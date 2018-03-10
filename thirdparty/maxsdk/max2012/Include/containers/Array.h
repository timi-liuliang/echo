//*****************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//*****************************************************************************/
/*==============================================================================

	file:     Array.h
	author:   Daniel Levesque
	created:  27 March 2006
	description:
		Array container.

==============================================================================*/
#pragma once

#include "..\maxheap.h"
#include <memory.h>     
#include <stdlib.h>
#include <new>
#include "../assert1.h"
#include "../utilexp.h"
#include "../coreexp.h"

UtilExport void UtilOutOfMemoryException();
UtilExport void* UtilAllocateMemory(size_t);
UtilExport void UtilDeallocateMemory(void*);

namespace MaxSDK {

//==============================================================================
// class MaxSDK::Array
//
//! \brief A generic array container, with proper support for non-POD types.
/*!
	This template class is a generaic, dynamic array container, similar to the
	STL class "vector". Whereas the classical 3ds Max SDK class Tab supports
	only POD types, this class supports non-POD types as well.
	\note POD stands for "plain old data", and basically denotes any data type which 
	does not need to be constructed, destructed, and which can be copied with 
	memcpy() rather than requiring a copy operator to be called.
*/
template <class T> class Array
: public MaxHeapOperators {
public:

	//! Type of function to pass to sort().
	/*! \par Must return:
		- < 0 if elem1 is smaller than elem2, 
		- > 0 if elem 1 is greater, 
		- or 0 if they're equal.
	*/
	typedef int( __cdecl *CompareFnc) ( const void *elem1, const void *elem2 );

	//! Initializes an empty array.
	Array();
	/*! \brief Initializes an array with an initial size.
		\param[in] initUsedLength - Number of elements initially allocated in the array.
		\param[in] defaultVal - The default value for the elements initially allocated.
		\param[in] initGrowLength - The initial growth length of the array.
		For more information on the growth length, see setGrowLength().
	*/
	Array(size_t initUsedLength, const T& defaultVal = T(), size_t initGrowLength = kDefaultGrowthLength);
	/*! \brief Copy constructor. Copies the contents of another array.
		\param[in] src - Array from which the elements are copied.
	*/
	Array(const Array<T>& src);
	//!	\brief Destructor. Destroys every element stored in the array, and frees the allocated storage.
	~Array();

	/*! \brief Copy operator. Copies the contents of another array.
		\param[in] src - Array from which the elements are copied.
		\return A reference to 'this'.
	*/
	Array<T>& operator=(const Array<T>& src);
	/*! \brief Equality operator. 
		\param[in] op - Array to be compared to 'this'.
		\return true if and only if both arrays contain the same number of 
		elements and each of these elements if equal to the corresponding element 
		from the other Array.
	*/
	bool operator==(const Array<T>& op) const;

	//@{
	/*! \brief Subscript operator.
		\param[in] i - Index of array element to access. This index 
		must be within the array bounds.
		\return A reference to the array element at the specified index.
		\remarks Does not implement bounds checking.
	*/
	T& operator[](size_t i);
	const T& operator[](size_t i) const;
	//@}

	//@{
	/*! \brief Same as subscript operator.
		\param[in] index - Index of array element to access. This index
		must be within the array bounds.
		\return A reference to the array element at the specified index.
		\remarks Does not implement bounds checking.
	*/
	const T& at(size_t index) const;
	T& at(size_t index);
	//@}

	/*!	\brief Sets a copy of value at the given index.
		\param[in] index - The position in the array where a copy of value is placed. This index 
		must be within the array bounds.
		\param[in] value - a reference to the original object.
		\return A reference to 'this'.
		\remarks Does not implement bounds checking.
	*/
	Array<T>& setAt(size_t index, const T& value);
	/*! \brief Sets all the elements of the array to the given value.
		\param[in] value - The value to which the elements of the array are set.
		\return A reference to 'this'.
	*/
	Array<T>& setAll(const T& value);

	//@{
	/*!	\brief Accesses the first element in the array.
		\return A reference to the first element of the array.
		\remarks It is invalid to call this on an empty array.
	*/
	T& first();
	const T& first() const;
	//@}

	//@{
	/*!	\brief Accesses the last element in the array.
		\return A reference to the last element of the array.
		\remarks It is invalid to call this on an empty array.
	*/
	T& last();
	const T& last() const;
	//@}

	/*!	\brief Appends a copy of value to the array.
		\param[in] value - A reference to the original value.
		\return The number of elements in the array prior to the append operation.
	*/
	size_t append(const T& value);
	/*! \brief Appends one or more element(s) to the array.
		\param[in] values - A pointer to a C-style array of elements, from which
		the appended elements will be copied.
		\param[in] count - The number of elements to be appended.
		\return A reference to 'this'.
	*/
	Array<T>& append(const T* values, size_t count);
	/*!	\brief Appends the contents of another array to this array.
		\param[in] array - The array from which elements are to be appended.
		\return A reference to 'this'.
	*/
	Array<T>& append(const Array<T>& array);

	/*!	\brief Inserts a single value, at a given location, into this array.
		\param[in] index - The index at which the element is to be inserted.
		This index must be smaller or equal to the used length of the array.
		\param[in] value - The value to be inserted.
		\return A reference to 'this'.
	*/
	Array<T>& insertAt(size_t index, const T& value);
	/*!	\brief Inserts a one or more value(s), at a given location, into this array.
		\param[in] index - The index at which the element is to be inserted.
		This index must be smaller or equal to the used length of the array.
		\param[in] values - A pointer to a C-style array of elements, from which
		the inserted elements will be copied.
		\param[in] count - The number of elements to be inserted.
		\return A reference to 'this'.
	*/
	Array<T>& insertAt(size_t index, const T* values, size_t count);

	/*! \brief Removes a single element from the array.
		\param [in] index - The index of the element to be removed. This index
		must be valid (within bounds).
		\return A reference to 'this'.
	*/
	Array<T>& removeAt(size_t index);
	/*! \brief Searches for a value in the array and, if it is found, removes it from the array.
		\param[in] value - The value to search for.
		\param[in] start - The index at which to start searching. Preceding elements
		are not searched.
		\return true if a value was found & removed; false otherwise.
		\remarks If multiple copies of the same value are stored in the array,
		only the first instance will be removed.
	*/
	bool remove(const T& value, size_t start = 0);
	/*! \brief Removes the first element of the array.
		\returns A reference to 'this'.
		\remarks Must not be called on an empty array.
	*/
	Array<T>& removeFirst();
	/*! \brief Removes the last element of the array.
		\returns A reference to 'this'.
		\remarks Must not be called on an empty array.
	*/
	Array<T>& removeLast();
	/*! \brief Removes all the elements from the array.
		\returns A reference to 'this'.
	*/
	Array<T>& removeAll();
	/*! \brief Removes a subset of the array.
		\param[in] startIndex - The index of the first element to be removed.
		\param[in] endIndex - The index of the last element to be removed.
		\returns A reference to 'this'.
		\remarks 
		- Both the start and end indices must be within bounds.
		- The end index must be greater or equal to the start index.
	*/
	Array<T>& removeSubArray(size_t startIndex, size_t endIndex);

	/*!	\brief Determines if a value is stored in the array.
		\param[in] value - The value for which to search for.
		\param[in] start - The index at which to start searching. Preceding elements
		are not searched.
		\return true if the value was found in the array; false otherwise.
	*/
	bool contains(const T& value, size_t start = 0) const;
	/*!	\brief Searches for a value in the array.
		\param[in] value - The value to search for.
		\param[out] foundAt - The index at which the value was found. Indeterminate if
		the value was not found.
		\param[in] start - The index at which to start searching. Preceding elements
		are not searched.
		\return true if the value was found in the array; false otherwise.
	*/
	bool find(const T& value, size_t& foundAt, size_t start = 0) const;
	/*! \brief Searches for a value in the array.
		\param[in] value - The value to search for.
		\return The index at which the value was found, or -1 if the value 
		was not found. (Since this returns an unsigned value, -1 is converted to the
		the largest positive value).
	*/
	size_t find(const T& value) const;
	/*! \brief Searches for a value in the array, starting at a given index.
		\param[in] value - The value to search for.
		\param[in] start - The index at which to start searching.
		\return The index at which the value was found, or -1 if the value 
		was not found. (Since this returns an unsigned value, -1 is converted to the
		the largest positive value).
	*/
	size_t findFrom(const T& value, size_t start) const;

	//!	Returns the number of elements used (as opposed to simply allocated/reserved) in the array.
	size_t length() const; // Used length.
	//! Returns true if the number of used elements in the array is 0; returns false otherwise.
	bool isEmpty() const;
	//! Returns the number of elements used (as opposed to simply allocated/reserved) in the array.
	size_t lengthUsed() const;
	/*! \brief Sets the number of elements used (as opposed to simply allocated/reserved) in the array.
		\param[in] length - The new "used length" of the array.
		\param[in] defaultVal - The default value for new elements, used only if the length
		of the array is increased.
		\return A reference to 'this'.
	*/
	Array<T>& setLengthUsed(size_t length, const T& defaultVal = T());
	//! Returns the number of elements allocated/reserved (as opposed to actually used) in the array.
	size_t lengthReserved() const;
	/*! \brief Sets the number of elements allocated/reserved (as opposed to actually used) in the array.
		\param[in] length - The new "reserved length" of the array.
		\return A reference to 'this'.
	*/
	Array<T>& setLengthReserved(size_t length);

	//! \brief Returns the growth length of the array.
	//!	For more information on the growth length, see setGrowLength().
	size_t growLength() const;
	//!	\brief Sets the growth length of the array.
	/*!	The growth length is the minimum number elements by which the reserved space
		is grown whenever the array runs out of reserved space.
	*/
	Array<T>& setGrowLength(size_t);

	/*! \brief Reverses the sequence of elements in the array.
		Reverses the sequence of elements in the array such that the last element becomes the first.
		\returns A reference to 'this'.
	*/
	Array<T>& reverse();
	/*! \brief Swaps two elements in this array.
		\param[in] i1 - The index of the first element to swap. This index must be within bounds.
		\param[in] i2 - The index of the second element to swap. This index must be within bounds.
	*/
	Array<T>& swap(size_t i1, size_t i2);
	/*! \brief Sorts the elements of the array using a custom comparison function. 
		The sort if performed with the QuickSort algorithm.
		\param[in] cmp - The comparison function used to order the elements.
		\see CompareFnc
	*/
	void sort(CompareFnc cmp);

	//@{
	/*! \brief Returns the array storage as a C-style array pointer.
		\remarks Any modification to the contents of the array, through this pointer,
		may be dangerous.
	*/
	const T* asArrayPtr() const;
	T* asArrayPtr();
	//@}

	/*! \brief Returns whether the given array index is valid for this array.
		\returns true if the given index is within the bounds of this array; false otherwise.
	*/
	bool isValidIndex(size_t) const;

protected:

	enum {
		/*! The maximum extra space (in bytes) that may be reserved when the 
			buffer is enlarged. The array will usually enlarge the buffer by half
			its size whenever it runs out of space, unless that enlargement
			would exceed this value.
		*/
		kArrayGrowthThreshold = 0x10000,
		//! The default growth length. See setGrowLength().
		kDefaultGrowthLength = 8
	};

	//! Pointer to the storage buffer.
	T* mpArray;
	//! The reserved length (in number of elements, not bytes).
	size_t mReservedLen;
	//! The used length (in number of elements, not bytes).
	size_t mUsedLen;
	//! The growth length. See setGrowLength().
	size_t mGrowLen;

	//! The partition portion of the QuickSort algorithm
	static size_t quickSortPartition(T* data, size_t first, size_t last, CompareFnc cmp);
	//! Recursive QuickSort function used to sort the elements of the array.
	static void	quickSortRecursive(T* data, size_t first, size_t last, CompareFnc cmp);
	//! Utility function, called when the array fails to allocate memory.
	static void	handleOutOfMemory();

	//! Allocates an array of elements without constructing them.
	static T* ArrayAllocate(size_t len);
	//! Constructs an array of elements.
	static void ArrayConstruct(T* arrayBegin, size_t len, const T& defaultVal);
	//! De-allocates an array of elements without destructing them.
	static void ArrayDeAllocate(T* arrayBegin);
	//! Destructs an array of elements.
	static void ArrayDestruct(T* arrayBegin, size_t len);
	//! \brief Copies an array of elements to an already-constructed buffer. 
	//! Will use the copy operator if needed.
	static void ArrayCopy(T* pCopy, const T * pSource, size_t nCount);
	//! Copies an array of elements when the target and destination memory buffers may overlap.
	static void ArrayCopyOverlap(T* pCopy, const T * pSource, size_t nCount);
	//! \brief Copies and array of elements to a non-constructed. 
	//! Will use the copy constructor if needed.
	static void ArrayCopyConstruct(T* pCopy, const T * pSource, size_t nCount);
};

#include "Array.inline.h"
#include "Array.imp.h"

} // namespace MaxSDK

