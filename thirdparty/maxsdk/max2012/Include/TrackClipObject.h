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
#pragma once

#include <WTypes.h>
#include "interval.h"
#include "maxtypes.h"

/*! \sa  Class Animatable.
\par Description:
If a plug-in supports track view copy/paste operations this object is used. A
plug-in should derive a class from this base class to store the data associated
with the objects tracks, and implement the methods that identify the creator of
the clip object. */
#pragma warning(push)
#pragma warning(disable:4100)
class TrackClipObject: public MaxHeapOperators {
	public:
		//! Specifies the interval of time clipped.
		Interval clip;		

		// Identifies the creator of the clip object
		/*! \remarks This method is used to identify the creator of the clip object by
		returning the SuperClassID of the creator. */
		virtual SClass_ID 	SuperClassID()=0;
		/*! \remarks Returns the ClassID of the creator of the clip object. */
		virtual Class_ID	ClassID()=0;

		/*! \remarks Constructor.
		\par Parameters:
		<b>Interval iv</b>\n\n
		The interval of the time clip. */
		TrackClipObject(Interval iv) {clip = iv;}
		/*! \remarks Destructor */
		virtual ~TrackClipObject() {}

		/*! \remarks This method is called to delete this instance of the clip object. */
		virtual void DeleteThis()=0;

		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
				Returns the number of keys in the clip object.
		\par Default Implementation:
		<b>{return 0;}</b> */
		virtual int NumKeys() {return 0;}
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
				Retrieves the value of the 'i-th' key.
		\par Parameters:
		<b>int i</b>\n\n
		Specifies the key to return.\n\n
		<b>void *val</b>\n\n
		The value of the key is stored here.
		\return  TRUE if the value was retrieved; otherwise FALSE.
		\par Default Implementation:
		<b>{return FALSE;}</b> */
		virtual BOOL GetKeyVal(int i, void *val) {return FALSE;}
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
				Sets the value of the 'i-th' key.
		\par Parameters:
		<b>int i</b>\n\n
		Specifies the key to store.\n\n
		<b>void *val</b>\n\n
		The value of the key is passed here.
		\return  TRUE if the value was stored; otherwise FALSE.
		\par Default Implementation:
		<b>{return FALSE;}</b> */
		virtual BOOL SetKeyVal(int i, void *val) {return FALSE;}
};
#pragma warning(pop)


