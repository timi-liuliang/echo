/**********************************************************************
 *<
	FILE: stack.h

	DESCRIPTION: Simple stack using Tab.

	CREATED BY:	Rolf Berteig

	HISTORY: Created 22 November 1994

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "maxheap.h"
#include "tab.h"

/*! \brief A simple stack implementation. */
template<class T> class Stack : public MaxHeapOperators
{
	private:
		Tab<T> s;		
	
	public:		
		// access the stack indexing from the top down.
		const T& operator[](const int i) const { 
			DbgAssert(s.Count()-i>0);
			return s[s.Count()-i-1];
			}
		T& operator[](const int i) { 
			DbgAssert(s.Count()-i>0);
			return s[s.Count()-i-1];
		}

		/*! \remarks Pushes the specified element on the stack.
		\par Parameters:
		<b>T *el</b>\n\n
		The item to push. */
		void Push( T *el ) { 
			s.Append( 1, el ); 			
			}

		/*! \remarks Pops an item off the stack.
		\par Parameters:
		<b>T *el</b>\n\n
		The item popped off the stack is returned here. */
		void Pop( T *el ) { 
			DbgAssert( s.Count() );	
			*el = s[s.Count()-1];
			s.Delete( s.Count()-1, 1 );			
			}

		/*! \remarks Pops an item off the stack and discards it.  The item is
		not returned. */
		void Pop() { 
			DbgAssert( s.Count() );				
			s.Delete( s.Count()-1, 1 );			
			}

		/*! \remarks Retrieves the item on top of the stack without altering
		the stack.
		\par Parameters:
		<b>T *el</b>\n\n
		The top item is returned here. */
		void GetTop( T *el ) {
			DbgAssert( s.Count() );	
			*el = s[s.Count()-1];
			}

		/*! \remarks Clears the stack.  All items are deleted. */
		void Clear() {
			s.Delete(0,s.Count());			
			}

		/*! \remarks Returns the number of item currently on the stack. */
		int Count() const {
			return s.Count(); 
			}

		/*! \remarks Removes the 'i-th' item from the stack.
		\par Parameters:
		<b>int i</b>\n\n
		The item to remove from the stack.
		\return  Returns the number of items left in the stack. */
		int Remove( int i ) {
			assert(i<s.Count());
			return s.Delete(s.Count()-1-i,1);
			}
	};


