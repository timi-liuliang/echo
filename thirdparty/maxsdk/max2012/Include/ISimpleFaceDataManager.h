//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
/**************************************************************************
	FILE: ISimpleFaceDataManager.h

	DESCRIPTION: Declares a public header for a function published core interface
				 for managing SimpleFaceDataChannel custom attributes.
					- interface ID
					- FP command ID's
					- Static FP interface declaration

	AUTHOR: ktong - created 02.16.2006
/***************************************************************************/

#pragma once

#include "iFnPub.h"
#include "ISimpleFaceDataChannel.h"

//! \brief The unique interface ID for the SimpleFaceData Manager.
#define SimpleFaceDataManager_InterfaceID Interface_ID(0x5e62018, 0xa73467e)

//! \brief The function-published interface for managing SimpleFaceData channels on an object.
/*! This manager allows simple per-face data channels to be added, removed and retrieved from
the specified editable mesh or poly object. This static function-published interface is 
registered as a core interface. Obtain a pointer to this interface with GetInterface() using 
its defined ID, SimpleFaceDataManger_InterfaceID. Functions provided by this interface can 
also be called through function-publish dispatching and through Maxscript.
\see ISimpleFaceFaceDataChannel
*/
class ISimpleFaceDataManager : public FPStaticInterface
{
public:
	//! \brief Function-publish IDs for the SimpleFaceData Manager functions.
	enum {
		eFpAddChannel, // Add a channel to this object
		eFpRemoveChannel, // Remove a channel from this object
		eFpGetChannel, // Get a channel on this object
		eFpGetChannels, // Get an array of all channels on this object

		eFpChannelTypeEnum, // ID for the function-published channel-type enum.
	};

	//! \brief Add a simple per-face data channel to a geometry object (C++).
	/*! A new per-face data channel of the specified type with an identifying ID and
	name is added to specified object's base object. Per-face data channels	are geometry 
	pipeline clients - adding a channel to an object adds the channel throughout the 
	entire modifier stack.
	\param[in] pObj - The target geometry object. Must be non-null. Only editable mesh and
	editable poly objects are currently supported.
	\param[in] type - A ParamType or ParamType2 enum value that specifies the data type for all
	faces within the data channel. The current list of supported types is:
	- TYPE_INT
	- TYPE_INDEX
	- TYPE_BOOL
	- TYPE_FLOAT
	- TYPE_POINT2_BV
	- TYPE_POINT3_BV
	No channel is added if an unsupported type is specified.
	\param[in] pChannelID - The identifying Class_ID of the new channel. Specify NULL to have a
	unique ID generated and assigned to the new channel. If an ID is specified, no channel is 
	added if a channel with the specified ID already exists.
	\param[in] pChannelName - The name of the new channel. Specify NULL to use a default channel name.
	\return A pointer to the new channel instance if one was added. NULL if	no channel was added.
	*/
	virtual ISimpleFaceDataChannel* AddChannel(Object* pObj, int type, const Class_ID* pChannelID, const MCHAR* pChannelName) const =0;
	//! \brief Add a simple per-face data channel to the specified geometry object (Maxscript).
	/*! The maxscript exposed wrapper for AddChannel(). The only difference is that this channel ID
	is specified as a 2 element array of DWORDS.
	\see AddChannel (C++)
	*/
    virtual ISimpleFaceDataChannel* AddChannel(Object* pObj, int type, const Tab<DWORD>* pChannelID, const MCHAR* pChannelName) const =0;

	//! \brief Remove a simple per-face data channel from a geometry object (C++).
	/*! Remove the per-face data channel on the specified object's base object that matches 
	the specified channel ID. The channel is removed throughout the entire modifier stack.
	\param[in] pObj - The target geometry object. Must be non-null.
	\param[in] channelID - The identifying Class_ID  of the channel to remove.
	*/
	virtual void RemoveChannel(Object* pObj, const Class_ID &channelID)=0;
	//! \brief Remove a simple per-face data channel from a geometry object (Maxscript).
	/*! The maxscript exposed wrapper for RemoveChannel(). The only difference is that the channel ID
	is specified as a 2 element array of DWORDS.
	\see RemoveChannel (C++)
	*/
	virtual void RemoveChannel(Object* pObj, const Tab<DWORD>* pChannelID)=0;

	//! \brief Get a simple per-face data channel from a geometry object (C++).
	/*! Obtain a pointer to the per-face data channel on the specified object that matches the specified channel ID.
	\param[in] pObj - The target geometry object. Must be non-null.
	\param[in] pChannelID - The identifying Class_ID of the channel to retrieve.
	\return A pointer to the desired channel instance. NULL if a channel with the specified ID was not found.
	*/
    virtual ISimpleFaceDataChannel* GetChannel(Object* pObj, const Tab<DWORD>* pChannelID) const =0;
	//! \brief Get a simple per-face data channel from a geometry object (Maxscript).
	/*! The maxscript exposed wrapper for GetChannel(). The only difference is that this channel ID
	is specified as a 2 element array of DWORDS.
	\see GetChannel (C++)
	*/
	virtual ISimpleFaceDataChannel* GetChannel(Object* pObj, const Class_ID &channelID) const =0;

	//! \brief Get all simple per-face data channels from a geometry object (C++, Maxscript).
	/*! Obtain an array of pointers to all simple per-face data channels on the specified object.
	\param[in] pObj - The target geometry object. Must be non-null.
	\return An array of pointers to simple per-face data channels. An empty array if no channels were found.
	*/
    virtual Tab<ISimpleFaceDataChannel*> GetChannels(Object* pObj) const =0;
};

// EOF
