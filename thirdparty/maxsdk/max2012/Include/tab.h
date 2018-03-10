//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Declaration of a generic "table" class
// AUTHOR: Dan Silva, September 13, 1994
//***************************************************************************/


/*-------------------------------------------------------------------------------

 A Generic "Table" class.      
	
	(DSilva 9-13-94)

	This is a type-safe variable length array which also supports list-like
	operations of insertion, appending and deleting.  Two instance variables
	are maintained: "nalloc" is the number items allocated in the
	array; "count" is the number actual used. (count<=nalloc).
	Allocation is performed automatically when Insert or Append operations
	are performed.  It can also be done manually by calling Resize or Shrink.
	Note: Delete does not resize the storage: to do this call Shrink().  
	If you are going to do a sequence of Appends, it's more efficient to 
	first call Resize to make room for them.  Beware of using the Addr 
	function: it returns a pointer which may be invalid after subsequent 
	Insert, Append, Delete, Resize, or Shrink operations.  
	
	
	The implementation minimizes the storage of empty Tables: they are
	represented by a single NULL pointer.  Also, the major part of the
	code is generic, shared by different Tabs for different types of items.

------------------------------------------------------------------------------*/

#pragma once

#include "maxheap.h"
#include <malloc.h>
#include <stdlib.h>
#include <wtypes.h>
#include "utilexp.h"
#include "assert1.h"

typedef int CNT;

struct TabHdr: public MaxHeapOperators 
{
	CNT count;
	CNT nalloc;
};

////////////////////////////////////////////////////////////////////////////////
// Functions for internal use only: Clients should never call these.
//
UtilExport int TBMakeSize(TabHdr** pth, int num, int elsize); 
UtilExport int TBInsertAt(TabHdr** pth, int at, int num, void *el, int elsize, int extra); 
UtilExport int TBCopy(TabHdr** pth, int at, int num, void *el, int elsize); 
UtilExport int TBDelete(TabHdr** pth, int starting, int num, int elsize);
UtilExport void TBSetCount(TabHdr** pth, int n, int elsize, BOOL resize);
UtilExport void zfree(void**p);
////////////////////////////////////////////////////////////////////////////////

#define NoExport
		
template <class T> class NoExport TabHd: public MaxHeapOperators 
{
	public:
		CNT count;
		CNT nalloc;
		T data[100];
		TabHd() { 
			count = 0; nalloc = 0; 
		}
		// Unimplemented assignment operator
		TabHd& operator=(const TabHd&);
};


// Type of function to pass to Sort.
// Note: Sort just uses the C lib qsort function. If we restricted
// all Tab items to have well defined <,>,== then we wouldn't need
// this callback function.
typedef int( __cdecl *CompareFnc) ( const void *elem1, const void *elem2 );

//! \brief Generic container class.
/*! This is a type-safe variable length array class which also supports list-like 
operations of insertion, appending and deleting. Two instance variables are maintained: 
nalloc is the number items allocated in the array; count is the number actual 
used (count<=nalloc). Allocation is performed automatically when Insert or Append 
operations are performed. It can also be done manually by calling Resize() or Shrink().

\note Delete does not resize the storage: to do this call Shrink(). If you are 
going to do a sequence of Appends, its more efficient to first call Resize() to 
make room for them. Beware of using the Addr() function: it returns a pointer which 
may be invalid after subsequent Insert(), Append(), Delete(), Resize(), or Shrink() 
operations.

\note In 3ds max 1.x, the method SetCount(n) will set the count to n, but will not 
assure that only n items are allocated. To do that you should call Resize(n). 
This sets the number allocated. It will also make sure that count<=numAlloc. 
To make sure that exactly n are allocated and that count = n, call both Resize(n) 
and SetCount(n). In 3ds max 2.x and later using SetCount() will also effectively 
call Resize().

\note This structure is not meant to support more than 2G items; if you need to
have more items, consider using an STL container which does not have the	2G 
barrier and is most likely more optimized than this version.

The implementation minimizes the storage of empty Tables: they are represented by 
a single NULL pointer. Also, the major part of the code is generic, shared by 
different Tabs for different types of items.

Tabs may be used on the stack, i.e. they may be declared as a local variable of 
a function or method. You can set the number of items in the table, work with 
them, and then when the function returns, the destructor of the Tab is called, 
and the memory will be deallocated.

Tabs are only appropriate for use with classes that don't allocate memory. 
For example, Tab<float> is fine while Tab<MSTR> is problematic (MSTR is the class 
used for strings in 3ds max). In this case, the MSTR class itself allocates memory 
for the string. It relies on its constructor or destructor to allocate and free 
the memory. The problem is the Tab class will not call the constructors and destructors 
for all the items in the table, nor will it call the copy operator. 
As an example of this, when you assign a string to another string, the MSTR class 
does not just copy the pointer to the string buffer (which would result in two items 
pointing to the same block of memory). Rather it will allocate new memory and copy 
the contents of the source buffer. In this way you have two individual pointers 
pointing at two individual buffers. When each of the MSTR destructors is called it 
will free each piece of memory. So, the problem with using a Tab<MSTR> is that when 
you assign a Tab to another Tab, the Tab copy constructor will copy all the items 
in the table, but it will not call the copy operator on the individual items. 
Thus, if you had a Tab<MSTR> and you assigned it to another Tab<MSTR>, you'd have 
two TSTRs pointing to the same memory. Then when the second one gets deleted it 
will be trying to double free that memory.

So again, you should only put things in a Tab that don't allocate and deallocate 
memory in their destructors. Thus, this class should not be used with classes that 
implement an assignment operator and or destructor because neither are guaranteed 
to be called. The way around this is to use a table of pointers to the items. 
For example, instead of Tab<MSTR> use Tab <MSTR *>. As another example, Tab<int> 
is OK, while Tab<BitArray> would be no good. In the BitArray case one should use 
class pointers, i.e. Tab<BitArray *>.

All methods of this class are implemented by the system except the compare function 
used in sorting (see Sort()).
\see class BitArray, class MaxSDK::Array
*/
template <class T> class NoExport Tab: public MaxHeapOperators 
{
	private:
		TabHd<T> *th;

	public:
		//! \brief Default constructor
		Tab() : th(0) { }
		
		//! \brief Copy constructor
		/*! \param [in] tb The Tab that will be copied
		*/
		Tab(const Tab& tb) : th(0) {  
			TBCopy((TabHdr**)&th, 0, tb.Count(), (tb.th ? &tb.th->data : NULL), sizeof(T)); 
		}

		//! \brief Destructor
		/*!	The memory occupied by the Tab's items is freed, but the objects pointed 
		by the items are not.
		*/
		~Tab() {
			zfree((void**)&th); 
		}

		//! \brief Initializes a Tab instance
		/*! Provides a way of initializing a Tab instance outside of its constructor, 
		such as when they are are in-place constructed (constructed in pre-allocated memory).
		*/
		void Init() {
			th = 0;
		}

		//! \brief Retrieves the number of items in the Tab
		/*! \return The number of items in use in the Tab
		*/
		int Count() const 
		{ 
			if (th) {
				return (th->count); 
			}
			return 0; 
		}  

		//! \brief Resets the number of used items to zero.
		/*! WARNING: Using this method does not free any of the allocated memory
			stored.
		*/
		void ZeroCount() 
		{ 
			if (th) {
				th->count = 0; 
			}
		}
		
		//!	\brief Sets the number of used items 
		/*!	\param n The number of used items to set
		\param resize If TRUE, the Tab is resized to n items
		*/
		void SetCount(int n, BOOL resize=TRUE) { 
			TBSetCount((TabHdr **)&th, n, sizeof(T), resize); 
		}

		//!	\brief Returns the address of the i-th item
		/*!	\param i The index of the item whose address is to be returned
			\return Pointer to the i-th item
			\note This method returns a pointer which may be invalid after subsequent Insert, 
			Append, Delete, Resize, or Shrink operations.
		*/
		T* Addr(const INT_PTR i) const {             
			DbgAssert(th);
			DbgAssert(i < th->count); 
			return (&th->data[i]); 
		}

		//! \brief Inserts items in the Tab at a specified position 
		/*! \param at Index where to insert the items.
		\param num Number of items to insert
		\param el Pointer to the start of an array of items to insert
		\return If the insertion was successful, returns the value of at.
		*/
		int Insert(int at, int num, T *el) {
			return (TBInsertAt((TabHdr**)&th, at, num, (void *)el, sizeof(T), 0));
		}

		//! \brief Appends items at the end of the Tab
		/*! \param num Number of items to append
		\param el Pointer to the start of an array of items to insert
		\param allocExtra Number of extra Tab elements to be allocated in order to 
		enlarge the Tab.
		\return Returns the number of items in use (count of items) prior to appending
		*/
		int Append(int num, T *el, int allocExtra=0) {
			return (TBInsertAt((TabHdr**)&th, (th ? th->count : 0), num, (void *)el, sizeof(T), allocExtra)); 
		}
		
		//! \brief Deletes items from the Tab
		/*! \param start The index of the item the deletion starts at
		\param num The number of items to be deleted
		\return The number of items left in the table
		*/
		int Delete(int start, int num) { 
			return (TBDelete((TabHdr**)&th, start, num, sizeof(T)));
		} 
		
		//! \brief Changes the number of items allocated in memory.
		/*! Resize sets the amount of allocated memory, but doesn't 
		 *  change the actual number of items said to be in the tab
		 *  So if you know you will want a Tab with 1000 items, you 
		 *  could use Resize to pre-allocate the memory for the 1000
		 *  items, and then use Append to add each item without taking
		 *  a hit on reallocs as the count increases
		 *	\param num The new size (in number of items) of the array
		 *	\return Nonzero if the array was resized; otherwise 0.
		 */
		int Resize(int num) { 
			return (TBMakeSize((TabHdr**)&th, num, sizeof(T)));
		}	
		
		//! \brief Frees unused Tab items to reduce memory footprint
		void Shrink() {
			TBMakeSize((TabHdr**)&th, (th ? th->count : 0), sizeof(T)); 
		}

		//! \brief Sorts the array using the compare function
		/*! \param cmp Pointer to the comparison function to the used by Sort to 
		compare Tab items.
		\note: Sort() uses the C library qsort function. 3rd party developers must 
		implement the CompareFnc function.
		\code
		typedef int( __cdecl *CompareFnc) (const void *item1, const void *item2);
		\endcode
		The return value of CompareFnc is show below:
		< 0 - if item1 less than item2
		0 - if item 1 is identical to item2
		> 0 - if item1 is greater than item2
		\code
		static int CompTable(const void* item1, const void* item2) {
			MCHAR* a = (MCHAR*)item1;
			MCHAR* b = (MCHAR*)item2;
			return(_tcscmp(a, b));
		}
		\endcode
		*/
		void Sort(CompareFnc cmp) 
		{
			if (th) {
				qsort(th->data, th->count, sizeof(T), cmp);
			}
		}

		//! \brief Assignment operator
		/*! \param tb The Tab to copy the items from
		\note The objects pointed to by the Tab items are not copied, only the Tab items
		are copied.
		\return Reference to this Tab
		*/
		Tab& operator=(const Tab& tb) {
			TBCopy((TabHdr**)&th, 0, tb.Count(), (tb.th ? &tb.th->data : NULL), sizeof(T)); 
			return *this;
		}

		//! \brief Accesses the i-th Tab item
		/*! \param i The index of the Tab item to access
		\return Reference to the object in the i-th item.
		*/
		T& operator[](const INT_PTR i) const {       
			DbgAssert(th);
			DbgAssert(i < th->count); 
			return (th->data[i]); 
		}
};

#ifndef __tab_name2
#define __tab_name2(a, b) a##b
#endif
//#pragma deprecated("__tab_name2")

#define MakeTab(TYPE) typedef Tab<TYPE> __tab_name2(TYPE, Tab); 															
//#pragma deprecated("MakeTab")

UtilExport void TabStartRecording();
UtilExport void TabStopRecording();
UtilExport void TabPrintAllocs();
UtilExport void TabAssertAllocNum(int i);

