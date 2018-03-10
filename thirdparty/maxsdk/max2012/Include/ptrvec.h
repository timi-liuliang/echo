/**********************************************************************
 *<
	FILE: ptrvec.h

	DESCRIPTION:  An variable length array of pointers

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "maxheap.h"

/*! class PtrVector
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents a variable length array of pointers which can be stored
and manipulated.
\par Data Members:
protected:\n\n
<b>int size;</b>\n\n
The size of the array.\n\n
<b>int nused;</b>\n\n
The number of elements used.\n\n
<b>void** data;</b>\n\n
The variable length data. <br>  protected: */
class PtrVector: public MaxHeapOperators {
	protected:
		int size;
		int nused;
		void** data;
		/*! \remarks Constructor.
		\par Default Implementation:
		<b>{ size = nused = 0; data = NULL; }</b> */
		PtrVector() { size = nused = 0; data = NULL; }
		/*! \remarks Destructor. */
		UtilExport ~PtrVector();
		/*! \remarks Constructor.
		\par Parameters:
		<b>const PtrVector\& v</b>\n\n
		Reference to the PtrVector class to initialize with. */
		UtilExport PtrVector(const PtrVector& v);
		/*! \remarks Assignment operator.
		\par Parameters:
		<b>const PtrVector\& v</b>\n\n
		The reference to the PtrVector to assign. */
		UtilExport PtrVector&	operator=(const PtrVector& v);
	    /*! \remarks This method allows you to append data to the array.
	    \par Parameters:
	    <b>void *ptr</b>\n\n
	    A pointer to the data to add.\n\n
	    <b>int extra</b>\n\n
	    The extra space you wish to reserve. This will reshape the size of the
	    array to <b>nused + extra</b>. */
	    UtilExport void append(void * ptr , int extra);
		/*! \remarks This method allows you to insert data at a specified
		location.
		\par Parameters:
		<b>void *ptr</b>\n\n
		A pointer to the data to add.\n\n
		<b>int at</b>\n\n
		The index in the array you wish to insert at.\n\n
		<b>int extra</b>\n\n
		The extra space you wish to reserve. This will reshape the size of the
		array to <b>nused + extra</b>. */
		UtilExport void insertAt(void * ptr , int at, int extra);
	    /*! \remarks This method allows you to remove an element from the
	    array.
	    \par Parameters:
	    <b>int i</b>\n\n
	    The index of the element in the array you wish to remove.
	    \return  A pointer to the next element in the array. */
	    UtilExport void* remove(int i);
	    /*! \remarks This method allows you to remove the last element in the
	    array.
	    \return  A pointer to the new last element in the array. */
	    UtilExport void* removeLast();
		/*! \remarks Indexing operator.
		\return  The pointer to the data being referenced by the index.
		\par Default Implementation:
		<b>{ return data[i]; }</b> */
		void* operator[](int i) const { return data[i]; }		
		/*! \remarks Indexing operator.
		\par Default Implementation:
		<b>{ return data[i]; }</b>\n\n
		*/
		void*& operator[](int i) { return data[i]; }		
	public:
		/*! \remarks This method allows you set the capacity of the array.
		\par Parameters:
		<b>int i</b>\n\n
		The capacity to set. */
		UtilExport void reshape(int i);  // sets capacity
		/*! \remarks This method allows you to set the length and capacity if
		necessary.
		\par Parameters:
		<b>int i</b>\n\n
		The length to set. */
		UtilExport void setLength(int i);  // sets length, capacity if necessary
		/*! \remarks This method allows you to delete the entire ptr array,
		but not the objects. */
		UtilExport void clear();	// deletes the ptr array, but not the objects
		/*! \remarks This method will shrink the array to its used capacity.
		\par Default Implementation:
		<b>{ reshape(nused); }</b> */
		void shrink() { reshape(nused); }
		/*! \remarks This method returns the length of the array.
		\par Default Implementation:
		<b>{ return nused; }</b> */
		int length() const { return nused; }
		/*! \remarks This method returns the capacity of the array.
		\par Default Implementation:
		<b>{ return size; }</b> */
		int capacity() const { return size; }
	};

/*! \sa  Class PtrVector\n\n
template <class T> class PtrVec: public PtrVector
\par Description:
This class is available in release 4.0 and later only.\n\n
This template class represents a variable length array of pointers which can be
stored and manipulated.  */
template <class T> class PtrVec: public PtrVector
{
public:	
	/*! \remarks Constructor.
	\par Default Implementation:
	<b>{}</b> */
	PtrVec():PtrVector() {}
	/*! \remarks Indexing operator.
	\par Default Implementation:
	<b>{ return (T*)PtrVector::operator[](i); }</b> */
	T* operator[](int i) const { return (T*)PtrVector::operator[](i); }		
	/*! \remarks Indexing operator.
	\par Default Implementation:
	<b>{ return (T*\&)PtrVector::operator[](i); }</b> */
	T*& operator[](int i) { return (T*&)PtrVector::operator[](i); }				
	/*! \remarks Assignment operator.
	\par Default Implementation:
	<b>{ return (PtrVec\<T\>\&)PtrVector::operator=(v); }</b> */
	PtrVec<T>& operator=(const PtrVec<T>& v) { return (PtrVec<T>&)PtrVector::operator=(v); }
	/*! \remarks This method allows you to append data to the array.
	\par Parameters:
	<b>T *ptr</b>\n\n
	A pointer to the data.\n\n
	<b>int extra = 10</b>\n\n
	The amount of extra space you wish to allocate.
	\par Default Implementation:
	<b>{ PtrVector::append(ptr,extra); }</b> */
	void append(T *ptr, int extra = 10) { PtrVector::append(ptr,extra); }	
	/*! \remarks This method allows you to insert data into the array at the
	specified location.
	\par Parameters:
	<b>T* ptr</b>\n\n
	A pointer to the data to add.\n\n
	<b>int at</b>\n\n
	The index in the array you wish to insert at.\n\n
	<b>int extra = 10</b>\n\n
	The extra space you wish to reserve.
	\par Default Implementation:
	<b>{ PtrVector::insertAt(ptr,at,extra); }</b> */
	void insertAt(T* ptr, int at, int extra=10) { PtrVector::insertAt(ptr,at,extra); }	 
	/*! \remarks This method allows you to remove an element from the array.
	\par Parameters:
	<b>int i</b>\n\n
	The index into the array.
	\par Default Implementation:
	<b>{ return (T *)PtrVector::remove(i); }</b> */
	T* remove(int i) { return (T *)PtrVector::remove(i); }		
	/*! \remarks This method will remove the last item in the array.
	\return  A pointer to the new last item.
	\par Default Implementation:
	<b>{ return (T *)PtrVector::removeLast(); }</b> */
	T* removeLast() { return (T *)PtrVector::removeLast(); }		
	/*! \remarks This method allows you to delete the entire array. */
	void deleteAll();  //  deletes all the objects		
	};

template <class T>  void PtrVec<T>::deleteAll()
{
	while (length()) {
		T* p = removeLast();
		delete p;
		p = NULL;
	}
}

