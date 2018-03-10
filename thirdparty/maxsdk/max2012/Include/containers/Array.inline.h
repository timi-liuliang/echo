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

  file:     Array.inline.h
  author:   Daniel Levesque
  created:  27 March 2006
  description:
    Array container.

==============================================================================*/

template<class T> inline T* Array<T>::ArrayAllocate(size_t len)
{
	DbgAssert(len < 0x40000000);  // 1G sanity check
	T* p = (T*) UtilAllocateMemory(len * sizeof(T));
	return p;
}
#pragma warning(push)
// Disable the warnings for conditional expression is constant as a result of the compiler support for Type Traits
#pragma warning(disable:4127)
template <class T> inline void Array<T>::ArrayConstruct(T* arrayBegin, size_t len, const T& defaultVal)
{
	if(!__has_trivial_constructor(T))
	{
		for(size_t i = 0; i < len; ++i)
		{
			new(&(arrayBegin[i])) T(defaultVal);
		}
	}
}

template <class T> inline void Array<T>::ArrayDeAllocate(T* arrayBegin)
{
	UtilDeallocateMemory(arrayBegin);
}

#pragma warning(push)
#pragma warning(disable:4100)
template <class T> inline void Array<T>::ArrayDestruct(T* arrayBegin, size_t len)
{
	if(!__has_trivial_destructor(T)) 
	{
		for(size_t i = 0; i < len; ++i)
		{
			arrayBegin[i].~T();
		}
	}
}
#pragma warning(pop) // 4100

template <class T> void Array<T>::ArrayCopy(T* pCopy, const T * pSource, size_t nCount)
{
	// Auto-detect whether it's safe to use memcpy() or whether we need
	// to call the copy operator. We're counting on the fact that this condition,
	// being resolvable at compile-time, will be removed by the optimizer.
	if(__has_assign(T)) {
		// Type has an assignment operator; use it.
		for(size_t i = 0; i < nCount; ++i)
		{
			pCopy[i] = (pSource[i]);
		}
	}
	else {
		// Type does not have an assignment operator; use memcpy() as it's usually faster.
		if (nCount > 0) 
		{
			memcpy(pCopy, pSource, nCount * sizeof(T));
		}
	}
}

template <class T> void Array<T>::ArrayCopyOverlap(T* pCopy, const T * pSource, size_t nCount)
{
	// Auto-detect whether it's safe to use memcpy() or whether we need
	// to call the copy operator. We're counting on the fact that this condition,
	// being resolvable at compile-time, will be removed by the optimizer.
	if(__has_assign(T)) {
		// Type has an assignment operator; use it.
		for(size_t i = nCount; --i >= 0;)
		{
			pCopy[i] = (pSource[i]);
		}
	}
	else {
		// Type does not have an assignment operator; use memcpy() as it's usually faster.
		if (nCount > 0) 
		{
			memmove(pCopy, pSource, nCount * sizeof(T));
		}
	}
}

template <class T> void Array<T>::ArrayCopyConstruct(T* pCopy, const T * pSource, size_t nCount)
{
	// Auto-detect whether it's safe to use memcpy() or whether we need
	// to call the copy operator. We're counting on the fact that this condition,
	// being resolvable at compile-time, will be removed by the optimizer.
	if(__has_copy(T)) {
		// Type has an assignment operator; use it.
		for(size_t i = 0; i < nCount; ++i)
		{
			new(&(pCopy[i])) T(pSource[i]);
		}
	}
	else {
		// Type does not have an assignment operator; use memcpy() as it's usually faster.
		if (nCount > 0) 
		{
			memcpy(pCopy, pSource, nCount * sizeof(T));
		}
	}
}

#pragma warning(pop) // 4127

// Inline methods.
template <class T> inline bool Array<T>::contains(const T& value, size_t start) const
{ 
	return this->findFrom(value, start) != -1;
}

template <class T> inline size_t Array<T>::length() const
{ 
	return mUsedLen;
}

template <class T> inline bool Array<T>::isEmpty() const
{ 
	return mUsedLen == 0; 
}

template <class T> inline size_t Array<T>::lengthUsed() const
{ 
	return mUsedLen; 
}

template <class T> inline size_t Array<T>::lengthReserved() const
{ 
	return mReservedLen; 
}

template <class T> inline size_t Array<T>::growLength() const
{ 
	return mGrowLen;
}

template <class T> inline const T* Array<T>::asArrayPtr() const
{ 
	return mpArray;
}

template <class T> inline T* Array<T>::asArrayPtr()
{ 
	return mpArray; 
}

template <class T> inline bool Array<T>::isValidIndex(size_t i) const
{ 
	return i >= 0 && i < mUsedLen; 
}

template <class T> inline T& Array<T>::operator [] (size_t i)
{
	DbgAssert(this->isValidIndex(i));
	return mpArray[i];
}

template <class T> inline const T& Array<T>::operator [] (size_t i) const
{ 
	DbgAssert(this->isValidIndex(i)); 
	return mpArray[i];
}

template <class T> inline T& Array<T>::at(size_t i)
{ 
	DbgAssert(this->isValidIndex(i));
	return mpArray[i];
}

template <class T> inline const T& Array<T>::at(size_t i) const
{ 
	DbgAssert(this->isValidIndex(i)); 
	return mpArray[i];
}

template <class T> inline Array<T>& Array<T>::setAt(size_t i, const T& value)
{ 
	DbgAssert(this->isValidIndex(i)); 
	mpArray[i] = value;
	return *this;
}

template <class T> inline T& Array<T>::first()
{ 
	DbgAssert(!this->isEmpty()); 
	return mpArray[0]; 
}

template <class T> inline const T& Array<T>::first() const
{ 
	DbgAssert(!this->isEmpty()); 
	return mpArray[0];
}

template <class T> inline T& Array<T>::last()
{
	DbgAssert(!this->isEmpty()); 
	return mpArray[mUsedLen-1];
}

template <class T> inline const T& Array<T>::last() const
{ 
	DbgAssert(!this->isEmpty()); 
	return mpArray[mUsedLen-1];
}

template <class T> inline size_t Array<T>::append(const T& value)
{ 
	insertAt(mUsedLen, value); 
	return mUsedLen-1; 
}

template <class T> Array<T>& Array<T>::append(const T* values, size_t count)
{
	return insertAt(mUsedLen, values, count);
}

template <class T> inline Array<T>& Array<T>::removeFirst()
{ 
	DbgAssert(!isEmpty()); 
	return removeAt(0); 
}

template <class T> inline Array<T>& Array<T>::removeLast()
{ 
	DbgAssert(!isEmpty());
	return removeAt(mUsedLen - 1); 
}

template <class T> inline Array<T>& Array<T>::removeAll()
{ 
	if(mUsedLen > 0) {
		ArrayDestruct(mpArray, mUsedLen);
		mUsedLen = 0;
	}
	return *this; 
}

template <class T> inline Array<T>& Array<T>::setGrowLength(size_t glen)
{ 
	DbgAssert(glen > 0);
	if(glen > 0) {
		mGrowLen = glen;
	}
	else {
		DbgAssert(false);
		// Growth length needs to be at least 1.
		mGrowLen = 1; 
	}
	return *this; 
}


template <class T> inline void Array<T>::handleOutOfMemory() {

	DbgAssert(false);
	UtilOutOfMemoryException();
}
