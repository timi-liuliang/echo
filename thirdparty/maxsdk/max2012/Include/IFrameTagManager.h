/**********************************************************************
*<
FILE: IFrameTagManager.h

DESCRIPTION: Interface to Frame Tag Manager

CREATED BY: Larry Minton

HISTORY: created 8/23/04

*>	Copyright (c) 2004 Autodesk, All Rights Reserved.
**********************************************************************/
#pragma once

#include "ifnpub.h"

#define FRAMETAGMANAGER_INTERFACE Interface_ID(0x8cd73eea, 0x688b1f11)

/*! \par Description:
	IFrameTagManager exposes the FrameTagManager
*/
class IFrameTagManager : public FPStaticInterface {
	public:
		//! Get number of frame tags
		virtual int GetTagCount() = 0;

		//! Get ID for the i'th frame tag in the list of frame tags
		/*! \remarks the list is maintained in sorted order, so a given
			tag can move around in the list. Each tag has a unique
			ID, so all access to the tag is via this ID 
			\return The ID of the tag at index i */
		virtual DWORD GetTagID(int i) = 0;

		//! Get the tag's time value. 
		/*! \param id - The id of the tag to query
			\param relative - if TRUE, time is relative to the tag this 
				tag is locked to, if any. If false, or this tag is 
				not locked to another tag, the time is absolute */
		virtual TimeValue GetTimeByID(DWORD id, BOOL relative) = 0;
		//! Set the tag's time value. 
		/*! \param id - The id of the tag to set
			\param newTime - The new time value to set on the Tag
			\param relative - if TRUE, time is relative to the tag this 
				tag is locked to, if any. If false, or this tag is 
				not locked to another tag, the time is absolute */
		virtual void SetTimeByID(DWORD id, TimeValue newTime, BOOL relative) = 0;

		//! get the tag this tag is locked to. 
		/*!	\param id - The ID of the tag to query on
			\return The ID of the tag id is locked to, or 0 if not locked */
		virtual DWORD GetLockIDByID(DWORD id) = 0;

		//! set the tag this tag is locked to. 
		/*!	\remarks If you attempt to set a lock that would cause a circular dependency, the
			current lock will not be changed and the method will return FALSE. See CheckForLoop
			for testing for loops
			\param id - The ID of the tag to query on
			\param newLockID - value of 0 means not tagged 
			\return TRUE if the lock was set. */
		virtual BOOL SetLockIDByID(DWORD id, DWORD newLockID) = 0;

		//! get the tag's name
		virtual MSTR GetNameByID(DWORD id) = 0;
		//! set the tag's name
		virtual void SetNameByID(DWORD id, MSTR &newName) = 0;

		//! get whether to scale the tag's time when time is scaled
		virtual BOOL GetScaleByID(DWORD id) = 0;
		//! set whether to scale the tag's time when time is scaled
		virtual void SetScaleByID(DWORD id, BOOL newScale) = 0;

		//! \brief get the index of the tag in the sorted tag list. 
		/*! \return The index of the tag with the given ID, or -1 if a tag with the given ID was not found */
		virtual int FindItem(DWORD id) = 0;

		//! finds the first tag in the sorted list, if any, for the given absolute time value
		virtual int FindItemByTime(TimeValue t) = 0;

		//! \brief creates a new tag. 
		/*! \param name - The name of the tag
			\param t - The tags absolute time 
			\param lockID - The ID of the tag this new tag is locked too.  if 0, the tag is not locked to another tag
			\param scale - Whether or not to scale the tags time when time is scaled
			\return The ID of the created flag */
		virtual DWORD CreateNewTag(MCHAR *name, TimeValue t, DWORD lockID, BOOL scale) = 0;

		//! deletes the specified tag
		virtual void DeleteTag(DWORD id) = 0;

		//! deletes all the tags
		virtual void ResetFrameTags() = 0;

		//! returns true if tag testID is dependent on tag id
		virtual BOOL CheckForLoop(DWORD testID, DWORD id) = 0;

};

