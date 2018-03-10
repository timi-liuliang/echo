/**********************************************************************
 *<
	FILE:  IRefTargContainer.h

	DESCRIPTION:  Defines IRefTargContainer class
		A Reference Maker class that stores a variable list of ReferenceTarget* items.
		All actions performed on this class are undoable.

	CREATED BY: Larry Minton

	HISTORY: created 5/19/04

 *>	Copyright (c) 2004, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "ref.h"

// SClass_ID is REF_TARGET_CLASS_ID
#define REFTARG_CONTAINER_CLASS_ID Class_ID(0xa1692edf, 0xd681173f)

/*! \brief Container that handles references to ReferenceTargets.
 *
 * This is the interface for a container that references a variable number of 
 * ReferenceTarget objects. It assumes that none of the references are weak or are nodes. 
 * 
 * 3ds Max's implementation of this interface persists itself and its operations 
 * support being undone/redone. 
 * Plugins that need to store a variable number of references to other plugin objects 
 * should create an instance of this container type and make a reference to it.
 *
 * The code below shows how to create an instance of 3ds Max's implementation of this interface:
 * \code 
 * IRefTargConatiner* myRefTargets = static_cast<IRefTargContainer*>(Interface::CreateInstance(REF_TARGET_CLASS_ID, REFTARG_CONTAINER_CLASS_ID));
 * \endcode
 *
 * \see IIndirectRefTargContainer, IRefTargMonitorClass, INodeMonitor
*/
class IRefTargContainer : public ReferenceTarget
{
public:
	/*! \brief Returns the number of items in the container
	 *
	 * \return The number of items in the container. */
	virtual int GetNumItems()=0; 

	/*! \brief Returns the i-th item, NULL if i is out of range
	 *
	 * \param i The index of the item to return. Must be in range [0, GetNumItems()-1]
	 * \return The i-th item, or NULL if index is out of range. */
	virtual ReferenceTarget *GetItem(int i)=0;  
	
	/*! \brief Adds an item at the end of the container
	 *
	 * \param ref The item to be added
	 * \return The index of the newly added item. */
	virtual int AppendItem(ReferenceTarget *ref)=0; 

	/*! \brief Sets the value of the i-th container item.
	 *
	 * \param i - The index of the item to set. Must be >= 0. If the index is greater 
	 * than GetNumItems(), a new item is added to the container.
	 * \param ref - The item to set as the i-th item. */
	virtual void SetItem(int i, ReferenceTarget *ref)=0; 

	/*! \brief Inserts specified item at i-th place into the container.
	 * 
	 * \param i - The index where the item should be inserted. Must be in range 
	 * [0, GetNumItems()], otherwise i is truncated to fit in this range. When i is 
	 * GetNumItems(), the item is appended to the end of the container.
	 * \param ref - The item to be inserted.
	 * \return Index where the item has been inserted. */
	virtual int InsertItem(int i, ReferenceTarget *ref)=0; 

	/*! \brief Removes the i-th item from the container.
	 *
	 * De-references the i-th item and removes it from the container, thus effectively
	 * changing the number of items the container reports to have via GetNumItems().
	 * \param i - The index of the item to be removed. Must be in range [0, GetNumItems()-1].
	*/
	virtual void RemoveItem(int i)=0; 
};

