//**************************************************************************/
// Copyright (c) 1998-2010 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
#pragma once

#include "ioapi.h"
#include "ExcludeListConstants.h"

/*! \sa  Class Tab.
	\brief This class is used to store a table of names. 
	\par For example, this class is used by lights for their "Inclusion" and "Exclusion" lists.
	This class maintains an 'include' flag that specifies whether the list of names is things to be
	included, or things to be excluded. There is no reason the NameTab class can't be used for 
	other things where inclusion/exclusion is not relevant: in that case one can just ignore the
	'include' flag. All methods are implemented by the system.  */
class NameTab: public Tab<MCHAR *> {
	ULONG flags;
	public:
		//! \brief Constructor. 
		/*! The 'include' flag is set to FALSE. */
		NameTab() { flags = NT_AFFECT_ILLUM|NT_AFFECT_SHADOWCAST; }

		//! \brief Destructor
		UtilExport ~NameTab();

		// From Tab
		//! \brief removes the items in the Tab by un-allocating memory.
		/*!	Overrides the base class method. The Tab implementation of ZeroCount only sents the count value to
			zero and does not free any memory. This behavior basically guarantees a
			memory leak, especially with strings. To many developers have assumed that NameTab free's it's
			strings, by using this method, thus it's implementation is getting overridden in this class and
			altered. This method simply calls SetSize(0) */
		UtilExport void ZeroCount();

		//! Assignment operator. 
		UtilExport NameTab& operator=(const NameTab&	 n);

		//! \brief Sets the specified flag to the specified value.
		/*! \param ULONG f - The flag(s) to set. One or more of the following values:
			\list
			\li NT_INCLUDE - This bit is used to indicate "Include" mode.
			\li NT_AFFECT_ILLUM - This bit is used to indicate the "Illumination" check box in the exclusion list dialog.
			\li NT_AFFECT_SHADOWCAST - This bit is used to indicate the "Shadow Casting" check box in the exclusion list dialog.
			\endlist
			\param BOOL b=1 - The value to set. */
		void SetFlag(ULONG f, BOOL b=1){ if (b) flags|=f; else flags &= ~f; }

		//! \brief Checks if a flag is set or not
		/*! \param ULONG f - The flag(s) to set. One or more of the following values:
			\list
			\li NT_INCLUDE - This bit is used to indicate "Include" mode.
			\li NT_AFFECT_ILLUM - This bit is used to indicate the "Illumination" check box in the exclusion list dialog.
			\li NT_AFFECT_SHADOWCAST - This bit is used to indicate the "Shadow Casting" check box in the exclusion list dialog. 
			\endlist
			\return - Returns TRUE if the specified flag(s) are set; otherwise FALSE. */
		BOOL TestFlag(ULONG f){ return (flags&f)?1:0; }

		//! \brief Appends a copy of the specified name to the end of the list.
		/*! \param MCHAR* n - The name to add.
			\return - Returns the number of items in the list prior to appending. */
		UtilExport int AddName(const MCHAR *n);

		//! \brief Sets a name in the list
		/*! Stores the specified name at the specified position in the list.
			\param int i - The position in the list for the name.
			\param MCHAR *n - The name to store. If the name is NULL, the 'i-th' entry is set to NULL. */
		UtilExport void SetName(int i, const MCHAR *n);

		//! \brief Sets the size of the list. 
		/*! If the new size is smaller than the current size, the entries are deleted.
			\param int num - Specifies the size of the list. */
		UtilExport void SetSize(int num);

		//! \brief Removes the 'i-th' name from the list.
		/*! \param int i - Specifies the index of the name to remove. */
		UtilExport void RemoveName(int i);

		//! \brief Finds the index of the name passed in
		/*! \param MCHAR* n - The name to find. 
			\return - Returns the index of the name passed; otherwise returns -1.*/
		UtilExport int FindName(const MCHAR* n);

		//! \brief Loads this NameTab from disk.
		/*! \param ILoad *iload - Provides methods to load data from disk.
			\sa  \ref ioResults. */
		UtilExport IOResult Load(ILoad *iload);

		//! \brief Saves this NameTab to disk.
		/*! \param ISave *isave - Provides methods to save data to disk.
			\sa  \ref ioResults. */
		UtilExport IOResult Save(ISave *isave);
	};


