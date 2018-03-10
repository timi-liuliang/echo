//**************************************************************************/
// Copyright (c) 1998-2005 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Interfaces for ATS Notifications 
// AUTHOR: Michael Russo - created April 27, 2005
//***************************************************************************/

#pragma once

#include "IATSProvider.h"

#ifndef ATSExport
	#define ATSExport __declspec( dllexport )
#endif

///////////////////////////////////////////////////////////////////////////////
//
// Consts and Typedefs
//
///////////////////////////////////////////////////////////////////////////////
typedef DWORD			ATSNotifyID;
typedef DWORD			ATSNotifyResult;
typedef LONG_PTR		ATSNotifyClientPtr;


namespace ATS {

	// ATS Notification IDs
	const ATSNotifyID		kATSNotifyPreLogin				= 1;
	const ATSNotifyID		kATSNotifyPostLogin				= 2;
	const ATSNotifyID		kATSNotifyPreLogout				= 3;
	const ATSNotifyID		kATSNotifyPostLogout			= 4;
	const ATSNotifyID		kATSNotifyPreOpenProject		= 5;
	const ATSNotifyID		kATSNotifyPostOpenProject		= 6;
	const ATSNotifyID		kATSNotifyPreCloseProject		= 7;
	const ATSNotifyID		kATSNotifyPostCloseProject		= 8;
	const ATSNotifyID		kATSNotifyPreLaunchOptions		= 9;
	const ATSNotifyID		kATSNotifyPostLaunchOptions		= 10;
	const ATSNotifyID		kATSNotifyPreLaunchProvider		= 11;
	const ATSNotifyID		kATSNotifyPostLaunchProvider	= 12;
	const ATSNotifyID		kATSNotifyPreCheckin			= 13;
	const ATSNotifyID		kATSNotifyPostCheckin			= 14;
	const ATSNotifyID		kATSNotifyPreCheckout			= 15;
	const ATSNotifyID		kATSNotifyPostCheckout			= 16;
	const ATSNotifyID		kATSNotifyPreUndoCheckout		= 17;
	const ATSNotifyID		kATSNotifyPostUndoCheckout		= 18;
	const ATSNotifyID		kATSNotifyPreAddFiles			= 19;
	const ATSNotifyID		kATSNotifyPostAddFiles			= 20;
	const ATSNotifyID		kATSNotifyPreGetLatest			= 21;
	const ATSNotifyID		kATSNotifyPostGetLatest			= 22;
	const ATSNotifyID		kATSNotifyPreProperties			= 23;
	const ATSNotifyID		kATSNotifyPostProperties		= 24;
	const ATSNotifyID		kATSNotifyPreShowHistory		= 25;
	const ATSNotifyID		kATSNotifyPostShowHistory		= 26;
	/*! Broadcasts that the ATSFileList associated with the current 
		notification has been modified. Broadcast this message if you
		make any modifications to the file list during a callback.
	*/
	const ATSNotifyID		kATSNotifyFileListUpdate		= 27;
	const ATSNotifyID		kATSNotifyPreExploreProvider	= 28;
	const ATSNotifyID		kATSNotifyPostExploreProvider	= 29;
	const ATSNotifyID		kATSNotifyPreGetVersion			= 30;
	const ATSNotifyID		kATSNotifyPostGetVersion		= 31;


	//
	// ATS Notification Callback Results
	//

	/*! Continue notification process */
	const ATSNotifyResult	kATSNotifyResultOk				= 0;
	/*! Cancel specified action.  This will cancel the action specified in the IATSNotification class.  
		It will stop the notification process and will notify any previous that the action has 
		been cancelled. 
	*/
	const ATSNotifyResult	kATSNotifyResultCancel			= 1;	
	/*! Stop notification process */
	const ATSNotifyResult	kATSNotifyResultStop			= 2;	
};

///////////////////////////////////////////////////////////////////////////////
//
// class ATSNotifyParams
//
///////////////////////////////////////////////////////////////////////////////

//! \brief Asset Tracking System Notification Parameter class
/*! This class is created and configured by an object send an ATS
notification message.
*/
class ATSNotifyParams : public MaxHeapOperators
{
public:
	//! \brief Constructor
	ATSExport ATSNotifyParams();
	//! \brief Destructor
	ATSExport virtual ~ATSNotifyParams();

	//! \brief Returns the Notification ID
	/*! \return returns the ID of the notification message
	*/
	ATSExport virtual ATSNotifyID		GetID();

	//! \brief Set the Notification ID
	/*! \param[in] id ID of notification message
	*/
	ATSExport virtual void				SetID( ATSNotifyID id );

	//! \brief Returns the index of the provider
	/*! \return Returns the index of the provider that is associated with the message
	*/
	ATSExport virtual UINT				GetProviderIndex();

	//! \brief Set the provider index
	/*! \param[in] uIndex Index of provider that is associated with the message
	*/
	ATSExport virtual void				SetProviderIndex( UINT uIndex );

	//! \brief Specifies if this notification message was cancelled
	/*! If a message is cancelled by an object that has registered
	for notifications, all objects that received the notification
	before it was cancelled will be notified again, but this
	method will return true to allow the objects to perform
	any cleanup or undo any action taken.
	\return true if cancelled, false if not cancelled
	*/
	ATSExport virtual bool				IsCancelled();

	//! \brief Sets the cancel state of a notification message
	/*! \see ATSNotifyParams::IsCancelled
	\param[in] bCancelled true if cancelled, false if not.
	*/
	ATSExport virtual void				SetCancelled( bool bCancelled );

	//! \brief Returns the file list associated with the notification message
	/*! \see ATSFileList
	\return pointer to ATSFileList object
	*/
	ATSExport virtual ATSFileList*		GetATSFileList();

	//! \brief Set the file list associated with the notification message
	/*! \see ATSFileList
	\param[in,out] pATSFileList pointer to ATSFileList object
	*/
	ATSExport virtual void				SetATSFileList( ATSFileList* pATSFileList );

protected:
	ATSNotifyID		mID;
	UINT			mProviderIndex;
	bool			mCancelled;
	ATSFileList*	mpATSFileList;
};

///////////////////////////////////////////////////////////////////////////////
//
// class IATSNotifyCallback
//
///////////////////////////////////////////////////////////////////////////////

//! \brief Asset Tracking System Notification Callback Interface
/*! This interface is implemented by any object that wants to receive
notification messages.
*/
class IATSNotifyCallback : public MaxHeapOperators
{
public:
	//! \brief Asset Tracking System Notification Callback method
	/*! \see ATSNotifyParams
	\param[in,out] pATSNotifyParams pointer to ATSNotifyParams object that contains
	information about the notification message
	\param[in] pClientPtr pointer supplied by client when registered for notifications
	\return three values: kATSNotifyResultOk, kATSNotifyResultCancel, or kkATSNotifyResultStop.
	kATSNotifyResultOk will continue the notification through the system
	kATSNotifyResultStop will prevent the notification message from continuing
	kATSNotifyResultCancel is similar to kATSNotifyResultStop, but all previous objects that have
	ready received the notification will be sent a cancel notification.
	*/
	virtual	ATSNotifyResult	ATSNotifyCallback( ATSNotifyParams *pATSNotifyParams, ATSNotifyClientPtr pClientPtr ) = 0;
};

///////////////////////////////////////////////////////////////////////////////
//
// class ATSNotification
//
///////////////////////////////////////////////////////////////////////////////
//! \brief Register for and Broadcast ATS Notification Messages
class ATSNotification : public MaxHeapOperators
{
public:
	//! \brief Register for ATS Notification Messages
	/*! \see IATSNotifyCallback
	\param[in] pCallback pointer to IATSNotifyCallback interface
	\param[in] pClientPtr client supplied pointer that will be returned with all notifications
	\return true if success, false if failure
	*/
	ATSExport static bool				RegisterATSNotification( IATSNotifyCallback *pCallback, ATSNotifyClientPtr pClientPtr );
	//! \brief UnRegister for ATS Notification Messages
	/*! \see IATSNotifyCallback
	\param[in] pCallback pointer to IATSNotifyCallback interface
	\return true if success, false if failure
	*/
	ATSExport static bool				UnRegisterATSNotification( IATSNotifyCallback *pCallback );

	//! \brief Broadcast ATS Notification Message
	/*! \see ATSNotifyParams
	\param[in] pATSNotifyParamsn pointer to ATSNotifyParams object
	\return three values: kATSNotifyResultOk, kATSNotifyResultCancel, or kkATSNotifyResultStop.
	*/
	ATSExport static ATSNotifyResult	BroadcastATSNotification( ATSNotifyParams *pATSNotifyParamsn );
	//! \brief Broadcast ATS Notification Message
	/*! \param[in] id ID of notification message. An ATSNotifyParams object will be created with the
	specified ID and Provder Index.
	\param[in] uProviderIndex Index of provider. An ATSNotifyParams object will be created with the
	specified ID and Provder Index.
	\return three values: kATSNotifyResultOk, kATSNotifyResultCancel, or kkATSNotifyResultStop.
	*/
	ATSExport static ATSNotifyResult	BroadcastATSNotification( ATSNotifyID id, UINT uProviderIndex );
};

