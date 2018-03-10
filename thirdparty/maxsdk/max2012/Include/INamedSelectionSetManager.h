//
// Copyright [2009] Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.  
//
//

#pragma once

#include "maxapi.h"
#include "iFnPub.h"

//! \brief Named Selection Set Manager interface ID
#define IID_NAMED_SELECTION_SET_MANAGER Interface_ID(0x42a42b3, 0x1cf1cc5)

//! \brief The Named Selection Sets management class
/*! The user may use functions exposed by this class to manage all the named selection sets 
	of objects.
	\note This class DOES NOT deal with named selection sets of sub-objects but only named 
		  selection sets of objects.
*/
class INamedSelectionSetManager : public FPStaticInterface
{
public:
	//! \brief Get the instance of INamedSelectionSetManager.
	/*! \return the instance of INamedSelectionSetManager.
	*/
	static inline INamedSelectionSetManager* GetInstance()
	{
		return static_cast<INamedSelectionSetManager*>(
			GetCOREInterface(IID_NAMED_SELECTION_SET_MANAGER));
	}

	//! \brief Get the count of all the named selection sets.
	/*! \return the count of all the named selection sets.
	*/
	virtual int GetNumNamedSelSets() const = 0;

	//! \name Named Selection Set functions.
	//! \brief The following functions deal with a specific named selection set. 
	//@{

	//! \brief Get the name of the named selection set whose index is passed.
	/*! \param[in] setNum - The index of the named selection set.
		\return the name of the named selection set if the given index is valid,
				NULL otherwise.
	 */
	virtual MCHAR* GetNamedSelSetName(int setNum) const = 0;

	//! \brief Adds a new named selection set to those already available in the drop
	//!		   down list in the MAX toolbar.
	/*! \param[in] nodes - The table of nodes making up the selection set.
		\param[in] name - The name for the set. 
		\return TRUE if the named selection set is successfully added,FALSE otherwise.
	*/
	virtual BOOL AddNewNamedSelSet(Tab<INode*>& nodes,MSTR& name) = 0;
	
	//! \brief Removes the specified named selection set those already available in
	//! the drop down list in the MAX toolbar.
	/*! \param[in] name - The name for the set to remove.
		\return TRUE if the named selection set is successfully removed,FALSE otherwise.
	 */
	virtual BOOL RemoveNamedSelSet(MSTR &name) = 0;

	//! \brief Removes the specified named selection set those already available in
	//! the drop down list in the MAX toolbar.
	/*! \param[in] setNum - The index of the specified named selection set.
		\return TRUE if the named selection set is successfully removed,FALSE otherwise.
	*/
	virtual BOOL RemoveNamedSelSet(int setNum) = 0;
	
	//! \brief Get a specified named selection set by its index.
	/*! \param[out] nodes - The node set of the specified named selection 
			set that is to be fatched.This node set will be filled only if the specified
			index is valid.
		\param[in] setNum - The index of the specified named selection set.
							   If the index is invalid, an empty nodes set is 
							   to be returned.
	    \return TRUE if the specified named set index is valid,FALSE otherwise.
	*/
	virtual BOOL GetNamedSelSetList(Tab<INode*>& nodes,int setNum) const = 0;

	//! \brief Sets node named selection set's name. Returns TRUE if successful.
	/*! \param[in] setNum - The selection set index.
		\param[in] name - The new name for the selection set.
		\return TRUE if the specified named selection set is found with its name set,
				FALSE otherwise.
	 */
	virtual BOOL SetNamedSelSetName(int setNum, MSTR& name) = 0;
	//@}

	//! \name Named Selection Set items functions.
	//! \brief The following functions deal with items of a specific named selection set. 
	//@{

	//! \brief Get the number of items in the named selection set whose index is passed.
	/*! \param[in] setNum The index of the named selection set.
	\return the number of items in the named selection set if the given index is valid,
			0 otherwise.
	*/
	virtual int GetNamedSelSetItemCount(int setNum) const = 0;

	//! \brief Get the specified node of a specified named selection set.
	/*! This method may be used to retrieve the INode pointer of the 'i-th'
	item in the named selection set whose index is passed.
	\param[in] setNum - The index of the selection set whose 'i-th' 
	INode pointer is returned.
	\param[in] i - The index into the selection set.
	\return the specified node if the given set index and node index is valid,
			NULL otherwise.
	*/
	virtual INode* GetNamedSelSetItem(int setNum, int i) const = 0;

	//! \brief Replace the specified named selection set with given nodes.
	/*! \param[in] nodes - The given nodes to be filled into 
										the named selection set.
		\param[in] setNum - The index of the specified named selection set.
		\return TRUE if the specified selection set is found and replaced,
				FALSE otherwise.
	*/
	virtual BOOL ReplaceNamedSelSet(Tab<INode*>& nodes,int setNum) = 0;

	//! \brief Replace the specified named selection set with given nodes.
	/*! \param[in] nodes - The given nodes to be filled into 
									   the named selection set.
		\param[in] name - The name of the specified named selection set.
		\return TRUE if the specified selection set is found and replaced,
				FALSE otherwise.
	*/
	virtual BOOL ReplaceNamedSelSet(Tab<INode*>& nodes,MSTR& name) = 0;
	//@}
};
