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
// FILE:        linklist.h
// DESCRIPTION: Linked-list template classes
// AUTHOR:      Tom Hudson
// HISTORY:     created 10 December 1995
//**************************************************************************/
#pragma once

#include "maxheap.h"

template <class T> class LinkedEntryT: public MaxHeapOperators {
public:
	T data;
	void* next;
	LinkedEntryT(T& d) { data = d; next = NULL; }
};

/*!
\par Description:
Simple linked list class.  Methods and operators are provided to create new
linked lists, return the number of items in the list, access item using the
array operator ([]), and assign one list to another.  All methods of this class
are implemented by the system. */
template <class T, class TE> class LinkedListT: public MaxHeapOperators
{
private:
	TE* head;
	TE* tail;
	int count;
public:
	/*! \remarks Constructor.  The list is initialed to NULL
	and the count is set to 0. */
	LinkedListT()
	{
		head = tail = NULL;
		count = 0;
	}
	/*! \remarks Destructor. */
	~LinkedListT()
	{
		New();
	}
	/*! \remarks The items in the list are deleted. The list is initialed
	to NULL and the count is set to 0. */
	void New()
	{
		while(head)
		{
			TE* next = (TE*)head->next;
			delete head;
			head = next;
		}
		head = tail = NULL;
		count = 0;
	}
	/*!   \remarks Returns the number of items in the list. */
	int			Count() { return count; }
	/*! \remarks Adds a new item to the end of the list.
	\par Parameters:
	<b>T\& item</b>\n\n
	The item to add.
	\par Operators:
	*/
	void Append(T& item)
	{
		TE* entry = new TE(item);
		if(tail)
			tail->next = entry;
		tail = entry;
		if(!head)
			head = entry;
		count++;
	}
	/*! \remarks Allows access to items in the list using the array
	operator.  The first item in the list has an <b>index</b> of 0.
	\par Parameters:
	<b>int index</b>\n\n
	The array index of the item to access. */
	T& operator[](int index)
	{
		TE* e = head;
		while(index && e) {
			e = (TE*)e->next;
			index--;
		}
		// This should never happen, so we'll punt and return...
		// the head's data
		if(!e) {
			DbgAssert(0);
			return head->data;
		}
		return e->data;
	}
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>LinkedListT \&from</b>\n\n
	The list to copy.
	\return  A new linked list that is a copy of the list passed. */
	LinkedListT& operator=(LinkedListT &from)
	{
		New();
		for(int i = 0; i < from.Count(); ++i)
			Append(from[i]);
		return *this;
	}
};
