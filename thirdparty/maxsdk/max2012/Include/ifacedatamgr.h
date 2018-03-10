 /**********************************************************************
 
	FILE: IFaceDataMgr.h

	DESCRIPTION:  Face-Data management API

	CREATED BY: Attila Szabo, Discreet

	HISTORY: [attilas|30.8.2000]


 *>	Copyright (c) 1998-2000, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "idatachannel.h"
#include "baseinterface.h"
// forward declarations
class ISave;
class ILoad;

// GUID that identifies this ifc (interface)
#define FACEDATAMGR_INTERFACE Interface_ID(0x1b454148, 0x6a066927)

// Interface for managing face-data channels. 
// Objects that want to have face-data channels should implement this ifc
//
// If this interface needs to b changed, a new one should be derived from
// it and changed (IFaceDataMgr2) and made sure objects that support 
// face-data implement both old and new interfaces
/*! \sa  Class BaseInterface, Class IDataChannel, Class IFaceDataChannel\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents an interface for managing per-face-data.    Geometric
objects that wish to support multiple per-face-data channels should
implement this interface. Both <b>Mesh </b>and <b>MNMesh</b> support    the
<b>IFaceDataMgr</b> interface. They have full control over the lifetime    of
the face-data manager they expose. Consequently, client code should not cache
 an IFaceDataMgr interface aquired from Mesh or MNMesh for later use. */
class IFaceDataMgr : public BaseInterface
{
	public:
		//
		// Modifiers and procedural objects should call these methods 
		// to add\remove\retrieve a face-data channel on an object (mesh,patch,poly)
		//

		// Returns the number of face-data-channels
		/*! \remarks This method returns the number of face-data channels. */
		virtual ULONG		NumFaceDataChans( ) const = 0;

		// Retrieves a face-data-channel 
		/*! \remarks This method returns a pointer to the face-data channel.
		\par Parameters:
		<b>const Class_ID\& ID</b>\n\n
		The class ID of the channel you wish to retrieve. */
		virtual IFaceDataChannel* GetFaceDataChan( const Class_ID& ID ) const = 0;

		// Adds a face-data-channel to the object. Returns TRUE on success
		/*! \remarks This method adds a face-data channel to the object.
		\par Parameters:
		<b>IFaceDataChannel* pChan</b>\n\n
		A pointer to the face-data channel.
		\return  TRUE if successful, otherwise FALSE. */
		virtual BOOL		AddFaceDataChan( IFaceDataChannel* pChan ) = 0;

		// Removes a face-data-channel from the object. Returns TRUE on success
		/*! \remarks This method removes a face-data channel from the object.
		\par Parameters:
		<b>const Class_ID\& ID</b>\n\n
		The class ID of the channel you wish to remove.
		\return  TRUE if successful, otherwise FALSE. */
		virtual BOOL		RemoveFaceDataChan( const Class_ID& ID ) = 0;

		//
		// The "system" (Max) should call these methods to manage the
		// face-data channels when the object flows up the stack
		//

		// Appends a face-data-channel to the object. Returns TRUE on success
		/*! \remarks This method appends a face-data channel to the object.
		\par Parameters:
		<b>const IFaceDataChannel* pChan</b>\n\n
		The face-data channel to append.
		\return  TRUE if successful, otherwise FALSE. */
		virtual BOOL		AppendFaceDataChan( const IFaceDataChannel* pChan ) = 0;

		// Adds or appends face-data channels from the from object, to this object
		// If the channel already exists on this object, it's appended otherwise 
		// gets added
		/*! \remarks This method adds or appends face-data channels from the
		<b>from</b> object, to this object If the channel already exists on
		this  object, it's appended otherwise it gets added.
		\par Parameters:
		<b>const IFaceDataMgr* pFrom</b>\n\n
		The face-data channel to copy from.
		\return  TRUE if successful, otherwise FALSE. */
		virtual BOOL		CopyFaceDataChans( const IFaceDataMgr* pFrom ) = 0;

		// Deletes all face-data-channels from this object
		/*! \remarks This method removes all face-data channels from this
		object. */
		virtual void		RemoveAllFaceDataChans() = 0;

		// Mechanism for executing an operation for all face-data-channels on this object:
		// For all face-data-channels calls IFaceDataEnumCallBack::proc() with 
		// a pointer to that face-data- channel and a context data
		// Returns FALSE if the call back returns FALSE for any of the face-data-channels
		/*! \remarks This method provides a mechanism for executing an
		operation for  all face-data-channels on this object: For all
		face-data-channels calls  <b>IFaceDataEnumCallBack::proc()</b> with a
		pointer to that face-data-  channel and a context data
		\par Parameters:
		<b>IFaceDataChannelsEnumCallBack\& cb</b>\n\n
		A pointer to the face-data channel enumerator callback.\n\n
		<b>void* pContext</b>\n\n
		A pointer to the context data.
		\return  FALSE if the callback returns FALSE for any of the face-data
		channels. */
		virtual BOOL EnumFaceDataChans( IFaceDataChannelsEnumCallBack& cb, void* pContext ) const = 0; 

		// Allow persistance of info kept in object implementing this interface
		/*! \remarks Saves the face-data to the max file.
		\sa  \ref ioResults. */
		virtual IOResult Save(ISave* isave) = 0;
		/*! \remarks Loads the face-data from the max file.
		\sa \ref ioResults */
		virtual IOResult Load(ILoad* iload) = 0;

		// --- from GenericInterface
		/*! \remarks This method returns the interface ID of the object.
		\par Default Implementation:
		<b>{ return FACEDATAMGR_INTERFACE;  }</b> */
		virtual Interface_ID	GetID() { return FACEDATAMGR_INTERFACE; }

};

