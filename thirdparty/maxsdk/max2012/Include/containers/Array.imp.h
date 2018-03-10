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

  file:     Array.imp.h
  author:   Daniel Levesque
  created:  27 March 2006
  description:
    Array container.

==============================================================================*/

template <class T> Array<T>::Array()
: mpArray(NULL),
  mReservedLen(0),
  mUsedLen(0),
  mGrowLen(kDefaultGrowthLength)
{
	if(mGrowLen < 1) {
		// Growth length needs to be at least 1.
		mGrowLen = 1;
	}
}

template <class T> Array<T>::Array(size_t usedLength, const T& defaultVal, size_t growLength)
: mpArray(NULL),
  mReservedLen(0),
  mUsedLen(0),
  mGrowLen(growLength)
{
	if(mGrowLen < 1) {
		// Growth length needs to be at least 1.
		mGrowLen = 1;
	}

	// Re-size the array if a non-zero length was specified.
	if(usedLength > 0) {
		mpArray = ArrayAllocate(usedLength);
		if (mpArray == NULL) {
			handleOutOfMemory();
		}
		else {

			// Initialize the new elements
			ArrayConstruct(mpArray, usedLength, defaultVal);

			mReservedLen = usedLength;
			mUsedLen = usedLength;
		}
	}
}

// This is the usual copy constructor with the caveat that,
// if the system can not allocate enough memory to satisfy the
// request then it is assumed that the entire system is in a
// dangerously low memory situation, and there is no alternative
// but to have the system gracefully abort (i.e., prompting the
// users to save files, and/or free up more memory, or what-have-you).
//
template <class T> Array<T>::Array(const Array<T>& src)
: mpArray(NULL),
  mReservedLen(src.mReservedLen),
  mUsedLen(src.mUsedLen),
  mGrowLen(src.mGrowLen)
{
	if (mReservedLen > 0) {
		mpArray = ArrayAllocate(mReservedLen);
		if (mpArray == NULL) {
			handleOutOfMemory();
			mReservedLen = 0;
			mUsedLen = 0;
		}
		else {
			ArrayCopyConstruct(mpArray, src.mpArray, mUsedLen);
		}
	}
}

template <class T> Array<T>::~Array()
{
	if (mpArray != NULL) {
		ArrayDestruct(mpArray, mUsedLen);
		ArrayDeAllocate(mpArray);
	}
}

// The assignment operator.  The assignment operator copies
// the data from the source array to this array.  If the
// source array contains more elements that this array has
// space to store, then this array is grown to meet the demand.
// Otherwise, the reserved length of this array does not change.
// After this operation is completed the used lengths of the
// two arrays will be equal.  The grow length of this array is
// not affected by this operation.
//
template <class T> Array<T>& Array<T>::operator = (const Array<T>& src)
{
	if (this != &src) {
		// Re-allocate the buffer if necessary
		if (mReservedLen < src.mUsedLen) {
			// Destroy the existing list
			if (mpArray != NULL) {
				ArrayDestruct(mpArray, mUsedLen);
				ArrayDeAllocate(mpArray);
			}
			// Allocate a new buffer
			mReservedLen = src.mUsedLen;
			mpArray = ArrayAllocate(mReservedLen);
			if (mpArray == NULL) { // ...so this only happens if failure.
				handleOutOfMemory();
				mReservedLen = 0;
				mUsedLen = 0;
				return *this;
			}
			// Copy the list
			mUsedLen = src.mUsedLen;
			ArrayCopyConstruct(mpArray, src.mpArray, mUsedLen);
		}
		else if(mUsedLen < src.mUsedLen) {
			// The entire destination list is to be overwritten
			ArrayCopy(mpArray, src.mpArray, mUsedLen);
			// Remaining elements need to be added to the list
			ArrayCopyConstruct(mpArray + mUsedLen, src.mpArray + mUsedLen, src.mUsedLen - mUsedLen);
			mUsedLen = src.mUsedLen;
		}
		else if(mUsedLen > src.mUsedLen) {
			// Copy the entire source list.
			ArrayCopy(mpArray, src.mpArray, src.mUsedLen);
			// Truncate unused elements in the destination list.
			ArrayDestruct(mpArray + src.mUsedLen, mUsedLen - src.mUsedLen);
			mUsedLen = src.mUsedLen;
		}
		else {
			// Lists are of identical size; simply copy the entire contents
			ArrayCopy(mpArray, src.mpArray, mUsedLen);
		}
	}
	return *this;
}


// The equal to operator.  The equal to operator compares
// the data in two arrays.  If the used length of the
// two arrays are the same and the corresponding entries of
// the two arrays are equal, true is returned. Otherwise,
// false is returned.
//
template <class T> bool Array<T>::operator == (const Array<T>& cpr) const
{
	if (mUsedLen == cpr.mUsedLen)
	{
		for (size_t i = 0; i < mUsedLen; i++)
			if (mpArray[i] != cpr.mpArray[i])
				return false;
		return true;
	}
	return false;
}

// Sets all the elements within the used length of the array,
// (that is, elements 0..length()-1), to `value'.
//
template <class T> Array<T>& Array<T>::setAll(const T& value)
{
	for (size_t i = 0; i < mUsedLen; i++) {
		mpArray[i] = value;
	}
	return *this;
}

// Appends the `otherArray' to the end of this array.  The used length of
// this array will increase by the length of the `otherArray'.  If the
// reserved length is not long enough it will increase by the amount
// necessary to fit the newly added elements (with the usual caveat about
// insufficient memory).
//
template <class T> Array<T>& Array<T>::append(const Array<T>& otherArray)
{
	size_t otherLen = otherArray.length();
	if (otherLen == 0) {
		return *this;
	}
	size_t newLen = mUsedLen + otherLen;
	if (newLen > mReservedLen) {
		setLengthReserved(newLen);
	}

	ArrayCopyConstruct(mpArray + mUsedLen, otherArray.mpArray, otherLen);

	mUsedLen = newLen;
	return *this;
}

// Inserts `value' at `index'.  The value formerly at `index'
// gets moved to `index+1',  `index+1 gets moved to `index+2' and so on.
// Note that insertAt(length(), value) is equivalent to append(value).
// The used length of the array will increase by one.  If the reserved
// length is not long enough it will increase by the grow length (with the
// usual caveat about insufficient memory).
//
template <class T> Array<T>& Array<T>::insertAt(size_t index, const T& value)
{
	DbgAssert(index >= 0 && index <= mUsedLen);

	if (mUsedLen >= mReservedLen) {
		size_t growth = (mUsedLen * sizeof(T)) < kArrayGrowthThreshold ?
			(mUsedLen / 2) : kArrayGrowthThreshold / sizeof(T);
		setLengthReserved(mUsedLen + __max(growth, mGrowLen));
	}

	if (index != mUsedLen) {

		// Initialize the new member of the array
		ArrayCopyConstruct(mpArray + mUsedLen, mpArray + mUsedLen - 1, 1);

		// Copy the remainder of the list that needs to be shifted
		for(size_t i = mUsedLen - 1; i > index; --i) {
			mpArray[i] = mpArray[i-1];
		}

		// Now copy the new element into the array
		mpArray[index] = value;
	}
	else {
		// Add the new value to the end of the list
		ArrayCopyConstruct(mpArray + mUsedLen, &value, 1);
	}

	mUsedLen++;
	return *this;
}

template <class T> Array<T>& Array<T>::insertAt(size_t index, const T* values, size_t count)
{
	DbgAssert(index >= 0 && index <= mUsedLen);

	if(index <= mUsedLen) {

		size_t lastInsertIndex = index + count - 1;

		// Increase the allocated memory if necessary
		size_t newUsedLen = mUsedLen + count;
		if(newUsedLen > mReservedLen) {

			// Allocate a new buffer
			T* newArray = ArrayAllocate(newUsedLen);
			if(newArray == NULL) {
				// Can't insert the new element since the allocation failed.
				handleOutOfMemory();
				return *this;
			}

			// Copy existing elements located to the left of the insertion range
			ArrayCopyConstruct(newArray, mpArray, index);

			// Copy the inserted elements
			ArrayCopyConstruct(newArray + index, values, count);

			// Copy existing elements located to the right of the insertion range
			if(index < mUsedLen) {
				ArrayCopyConstruct(newArray + index + count, mpArray + index, mUsedLen - index);
			}

			// Destroy the old array
			ArrayDestruct(mpArray, mUsedLen);
			ArrayDeAllocate(mpArray);

			mpArray = newArray;
			mUsedLen = newUsedLen;
			mReservedLen = newUsedLen;
		}
		else {
			if(index < mUsedLen) {
				// Shift elements that get moved beyond the current limit of the array
				ArrayCopyConstruct(mpArray + mUsedLen, mpArray + mUsedLen - count, count);

				// Shift elements that stay inside the current limits of the array
				if((index + count) < mUsedLen) {
					ArrayCopyOverlap(mpArray + index + count, mpArray + index, mUsedLen - index - count);
				}

				// Copy new elements that get inserted within the current size of the array
				if(lastInsertIndex < mUsedLen) {
					ArrayCopy(mpArray + index, values, count);
				}
				else {
					ArrayCopy(mpArray + index, values, mUsedLen - index);
				}
			}

			// Copy new elements that get inserted beyond the current size of the array
			if(lastInsertIndex >= mUsedLen) {
				size_t numElementsInserted = (mUsedLen - index);
				DbgAssert(numElementsInserted < count);
				ArrayCopyConstruct(mpArray + mUsedLen, values + numElementsInserted, count - numElementsInserted);
			}

			mUsedLen += count;
		}
	}

	return *this;
}

// Removes the element at `index'.  The used length will
// decrease by one.  `index' MUST BE within bounds.
//
template <class T> Array<T>& Array<T>::removeAt(size_t index)
{
	DbgAssert(isValidIndex(index));

	if(index < mUsedLen) {
		// Shift array elements to the left if needed.
		//
		if (index < mUsedLen - 1) {
			for(size_t i = index; i < mUsedLen - 1; ++i) {
				mpArray[i] = mpArray[i+1];
			}
		}

		// Destroy the last element of the array
		ArrayDestruct(mpArray + mUsedLen - 1, 1);

		mUsedLen--;
	}

	return *this;
}

// Removes all elements starting with 'startIndex' and ending with 'endIndex'
// The used length will decrease by number of removed elements.
//
template <class T> Array<T>& Array<T>::removeSubArray(size_t startIndex, size_t endIndex)
{
	DbgAssert(isValidIndex(startIndex));
	DbgAssert(startIndex <= endIndex);

	if(startIndex < mUsedLen) {

		if(endIndex >= mUsedLen) {
			endIndex = mUsedLen - 1;
		}

		size_t numToRemove = endIndex - startIndex + 1;

		// Shift all elements that reside on the right of the sub-array to be removed
		for(size_t i = endIndex + 1; i < mUsedLen; ++i) {
			mpArray[i - numToRemove] = mpArray[i];
		}

		// Truncate the array
		ArrayDestruct(mpArray + mUsedLen - numToRemove, numToRemove);

		mUsedLen -= numToRemove;
	}

	return *this;
}

// Returns true if and only if the array contains `value' from
// index `start' onwards.  Returns, in `index', the first location
// that contains `value'.  The search begins at position `start'.
// `start' is supplied with a default value of `0', i.e., the
// beginning of the array.
//
template <class T> bool Array<T>::find(const T& value, size_t& index, size_t start) const
{
	const size_t nFoundAt = this->findFrom(value, start);
	if (nFoundAt == -1)
		return false;
	index = nFoundAt;
	return true;
}

template <class T> size_t Array<T>::find(const T& value) const
{
	return this->findFrom(value, 0);   // search from the beginning
}

template <class T> size_t Array<T>::findFrom(const T& value, size_t start) const
{
	for (size_t i = start; i < this->mUsedLen; i++) {
		if (mpArray[i] == value)
			return i;
	}
	return (size_t)-1;
}

// Allows you to set the used length of the array.
// If you try to set the used length to be greater than
// the reserved length, then the array is grown to a
// reasonable size (thus increasing both the used length
// AND the reserved length).
// Also, the reserved length will grow in growth length
// steps.
template <class T> Array<T>& Array<T>::setLengthUsed(size_t n, const T& defaultVal)
{
	DbgAssert(n >= 0);
	if (n > mReservedLen) {

		size_t growth = (mReservedLen * sizeof(T)) < kArrayGrowthThreshold ?
			(mReservedLen / 2) : kArrayGrowthThreshold / sizeof(T);

		size_t minSize = mReservedLen + __max(growth, mGrowLen);
		if ( n > minSize)
			minSize = n;
		setLengthReserved(minSize);
	}

	if(n > mUsedLen) {
		// Initialize the new elements
		ArrayConstruct(mpArray + mUsedLen, n - mUsedLen, defaultVal);
	}
	else {
		// Destroy the elements to be removed
		ArrayDestruct(mpArray + n, mUsedLen - n);
	}

	mUsedLen = n;
	return *this;
}

// Allows you to set the reserved length of the array.
// If you set the reserved length to be less than
// the used length, then the used length is reset
// to match the new reserved length.  A reserved length
// of zero is valid.
//
template <class T> Array<T>& Array<T>::setLengthReserved(size_t n)
{
	DbgAssert(n >= 0);

	if(n != mReservedLen) {

		if(n == 0) {
			if(mReservedLen > 0) {
				ArrayDestruct(mpArray, mUsedLen);
				ArrayDeAllocate(mpArray);
				mpArray = NULL;
				mUsedLen = 0;
				mReservedLen = 0;
			}
		}
		else if(mReservedLen == 0) {
			mpArray = ArrayAllocate(n);
			if(mpArray == NULL) {
				// Failure to allocate memory; can't increase the reserved length.
				handleOutOfMemory();
				return *this;
			}
			mReservedLen = n;
		}
		else {
			T* oldArray = mpArray;
			size_t oldUsedLen = mUsedLen;

			// Allocate the new array
			mpArray = ArrayAllocate(n);
			if(mpArray == NULL) {
				// Failure to allocate memory; can't change the reserved length.
				handleOutOfMemory();
				mpArray = oldArray;
				return *this;
			}

			// Copy the old array to the new one.
			if(n < mUsedLen) {
				// The old members don't all fit in the new array
				ArrayCopyConstruct(mpArray, oldArray, n);
				mUsedLen = n;
			}
			else {
				ArrayCopyConstruct(mpArray, oldArray, mUsedLen);
			}
			mReservedLen = n;

			// Destroy the old array
			ArrayDestruct(oldArray, oldUsedLen);
			ArrayDeAllocate(oldArray);
		}
	}

	return *this;
}

// Reverses the order of the array.  That is if you have two
// arrays, `a' and `b', then if you assign `a = b' then call
// `a.reverse()' then a[0] == b[n], a[1] == b[n-1],... a[n] == b[0].
//
template <class T> Array<T>& Array<T>::reverse()
{
	size_t halfUsedLen = mUsedLen/2;
	for (size_t i = 0; i < halfUsedLen; i++) {
		T tmp = mpArray[i];
		mpArray[i] = mpArray[mUsedLen - 1 - i];
		mpArray[mUsedLen - 1 - i] = tmp;
	}
	return *this;
}

// Swaps the elements in `i1' and `i2'.
//
template <class T> Array<T>& Array<T>::swap(size_t i1, size_t i2)
{
	DbgAssert(isValidIndex(i1));
	DbgAssert(isValidIndex(i2));

	if (i1 == i2) return *this;

	T tmp = mpArray[i1];
	mpArray[i1] = mpArray[i2];
	mpArray[i2] = tmp;
	return *this;
}

// Returns true if and only if `value' was removed from the array from
// position `start' onwards.  Only the first occurrence of `value'
// is removed.  Calling this function is equivalent to doing a "find(),
// then "removeAt()".
//
template <class T> bool Array<T>::remove(const T& value, size_t start)
{
	const size_t i = this->findFrom(value, start);
	if (i == -1)
		return false;
	this->removeAt(i);
	return true;
}

#pragma warning(push)
#pragma warning(disable:4127)
template <class T> void Array<T>::sort(CompareFnc cmp) {

	if(mUsedLen > 1) {
		// Use the standard C function of the type doesn't have a copy operator
		// (meaning that memcpy() is safe)
		/*	From MSDN: The compiler's support for type traits allows library writers to 
			determine various characteristics of a type at compile time. 
			All type traits return false if the specified conditions are not met. 
			Returns true if the CLR or native type has a copy assignment operator. */
		if(!__has_assign(T)) {
			qsort(mpArray, mUsedLen, sizeof(T), cmp);
		}
		else {
			quickSortRecursive(mpArray, 0, mUsedLen - 1, cmp);
		}
	}
}
#pragma warning(pop)

template <class T> size_t Array<T>::quickSortPartition(T* data, size_t first, size_t last, CompareFnc cmp)
{
	const T& pivot = data[last]; // use the last item as the pivot
	size_t left = first; // sort from the first item
	size_t right = last - 1; // sort to the item excluding the pivot

	do {
		while ((left < last) && (cmp(&(data[left]), &pivot) <= 0))
		{
			++left;
		}
		while ((right > first) && (cmp(&(data[right]), &pivot) >= 0))
		{
			--right;
		}
		if (left < right) {
			T swapValue = data[left];
			data[left] = data[right];
			data[right] = swapValue;
		}
	} while (left < right);

	if (cmp(&data[left], &pivot) > 0)
	{
		T swapValue = data[left];
		data[left] = data[last];
		data[last] = swapValue;
	}

	return left;
}

template <class T> void Array<T>::quickSortRecursive(T* data, size_t first, size_t last, CompareFnc cmp)
{
	if (first < last)
	{
		size_t pivot_position = quickSortPartition(data, first, last, cmp);

		// Protect against overflow. Normally the "if (first < last)" test would
		// guard against this, but size_t is unsigned, meaning "right - 1" can result
		// in a test of -1 > 0 when right is 0, which is an invalid unsigned inequality.
		if (pivot_position > 0)
		{
			quickSortRecursive(data, first, pivot_position - 1, cmp);
		}
		quickSortRecursive(data, pivot_position + 1, last, cmp);
	}
}
