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
	FILE: ISimpleFaceDataChannel.h

	DESCRIPTION: Declares a public header for a function publish interface
				 for controlling the SimpleFaceData custom attributes.
					- interface ID
					- FP command ID's
					- Mixin FP interface declaration and function map

	AUTHOR: ktong - created 02.16.2006
/***************************************************************************/

#pragma once

#include "iFnPub.h"

//! \brief The unique interface ID for a SimpleFaceData Channel.
#define SimpleFaceDataChannel_InterfaceID Interface_ID(0x1b2f01d0, 0x1c42591f)

//! \brief The function-published interface for a simple per-face data channel.
/*! Provides read/write access to a simple per-face data channel's description and
data values. An instance of this mixin function-published interface can be obtained
through the SimpleFaceDataManager or from the implementing object using GetInterface()
and this interface's defined ID, SimpleFaceDataChannel_InterfaceID. Functions 
provided by this interface can also be called through function-publish dispatching 
and through Maxscript.
\see ISimpleFaceDataManager
*/
class ISimpleFaceDataChannel : public FPMixinInterface
{
public:
	//! \brief Function-publish IDs for the SimpleFaceData Channel functions.
	enum {
		eFpGetChannelName,	// get the channel's name
		eFpSetChannelName,	// set the channel's name
		eFpGetChannelID,	// get the identifying channel id
		eFpGetChannelType,	// get the channel's data type
		eFpGetNumFaces,		// get the number of faces in the channel
		eFpGetValue,		// get a specific face value
		eFpSetValue,		// set a specific face value
		eFpGetValues,		// get all face values
		eFpSetValues,		// set all face values
		eFpGetValueBySelection,		// get the value of a selection of faces
		eFpSetValueBySelection,		// set the value of a selection of faces

		eFpChannelTypeEnum,	// the function-published channel-type enum
	};

#pragma warning(push)
#pragma warning(disable:4238)
	BEGIN_FUNCTION_MAP
		// functions exposed to Maxscript as properties
		PROP_FNS(eFpGetChannelName, GetChannelName, eFpSetChannelName, SetChannelName, TYPE_STRING);
		RO_PROP_FN(eFpGetChannelID, ChannelID, TYPE_DWORD_TAB_BV)
		RO_PROP_FN(eFpGetChannelType, ChannelType, TYPE_ENUM)
		RO_PROP_FN(eFpGetNumFaces, NumFaces, TYPE_DWORD)

		// functions exposed to Maxscript as methods
		FN_1(eFpGetValue, TYPE_FPVALUE_BV, GetValue, TYPE_INDEX)
		FN_2(eFpSetValue, TYPE_BOOL, SetValue, TYPE_INDEX, TYPE_VALUE)
		FN_0(eFpGetValues, TYPE_FPVALUE_BV, GetValues)
		FN_1(eFpSetValues, TYPE_BOOL, SetValues, TYPE_VALUE)
		FN_1(eFpGetValueBySelection, TYPE_FPVALUE_BV, GetValueBySelection, TYPE_BITARRAY)
		FN_2(eFpSetValueBySelection, TYPE_BOOL, SetValueBySelection, TYPE_BITARRAY, TYPE_VALUE)
	END_FUNCTION_MAP
#pragma warning(pop)
	//! \brief Set the name of the channel. (C++, Maxscript)
	/*! Change the name of the channel to the specified string.
	\param[in] pName - The new name of the channel. Must be non-null.
	*/
	virtual void		SetChannelName(const MCHAR* pName)=0;

	//! \brief Get the name of the channel. (C++, Maxscript)
	/*! Gets the name of the channel.
	\return A pointer to the string buffer containing the channel's name.
	*/
	virtual MCHAR*		GetChannelName() =0;

	//! \brief Get the channel ID of this channel. (C++)
	/*! Gets the identifying channel ID of this channel. The channel's ID cannot be changed
	after it has been created.
	\return The Class_ID that identifies this channel.
	*/
	virtual Class_ID	GetChannelID()	=0;

	//! \brief Get the channel ID of this channel. (Maxscript)
	/*! The maxscript exposed wrapper for ChannelID(). The only difference is that the 
	channel ID is returned as a 2 element array of DWORDS.
	\see GetChannelID (C++)
	*/
	virtual Tab<DWORD>	ChannelID() =0;

	//! \brief Get the number of faces in this channel.
	/*! Gets the number of per-face data items in this channel. The number of items in the 
	channel will always match the number of faces on the object.
	\return The number of faces in this channel. 0 if an error occurred.
	*/
	virtual ULONG		NumFaces() =0;

	//! \brief Get the data type of the channel.
	/*! Gets the data type of per-face data items in this channel. The channel's type cannot be
	changed after it has been created.
	\return The ParamType or ParamType2 enum value that corresponds to the channel's data type.
	*/
	virtual int			ChannelType() =0;

	//! \brief Get a face's per-face data value.
	/*! Gets the value of the specified face's per-face data.
	\param[in] face - The target face's 0-based index.
	\return An FPValue that wraps the face's per-face data. An FPValue wrapping NULL 
	is returned if the per-face data item could not be found.
	*/
	virtual FPValue		GetValue(ULONG face) =0;

	//! \brief Set a face's per-face data value.
	/*! Sets the value of the specified face's per-face data.
	\param[in] face - The target face's 0-based index.
	\param[in] pVal - The new Value of the face's per-face data. Must be non-null, loaded,
	and the type must match the channel type.
	\return TRUE if the specified face value is set, FALSE otherwise.
	*/
	virtual BOOL		SetValue(ULONG face, Value* pVal)=0;

	//! \brief Get the entire list of per-face data values for the channel.
	/*! Get a copy of the entire array of per-face data values for this channel.
	\return An FPValue that wraps an array of per-face data items. An FPValue wrapping NULL 
	is returned if the per-face data item could not be found.
	*/
	virtual FPValue		GetValues() =0;

	//! \brief Set the value of each face in the channel.
	/*! Sets the value of each face in the channel to a new value specified in an array.
	\param[in] pVal - A Value that wraps an array of per-face data items that specify the 
	channel's new per-face data values. Must be non-null, loaded, and the type must match
	the channel type. The array and the channel must have an equal number of data elements.
	\return TRUE if the specified face values were set, FALSE otherwise.
	*/
	virtual BOOL		SetValues(Value* pVal)=0;

	//! \brief Get the per-face value of a selection of faces.
	/*! Gets the common per-face value of a selection of faces specified by a BitArray.
	\param[in] pFaces - A BitArray that indicates the face selection. Must be non-null. This
	selection array must have fewer or an equal number of elements as the data channel.
	\return An FPValue that wraps the selection's per-face value. An FPValue wrapping NULL 
	is returned if the selected faces don't have a matching value or no selection was
	specified.
	*/
	virtual FPValue		GetValueBySelection(const BitArray* pFaces) =0;

	//! \brief Set the per-face value of a selection of faces.
	/*! Sets the per-face value of the specified selection of faces to a new specified value.
	\param[in] pFaces - A BitArray that indicates the face selection. Must be non-null. This
	selection array must have fewer or an equal number of elements as the data channel.
	\param[in] pVal- The new Value of the faces' per-face data. Must be non-null, loaded,
	and the type must match the channel type.
	\return TRUE if the per-face data values were set, FALSE otherwise.
	*/
	virtual BOOL		SetValueBySelection(const BitArray* pFaces, Value* pVal)=0;
};

// EOF
