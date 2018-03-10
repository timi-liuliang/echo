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
// DESCRIPTION: Interface for Dialog Monitor System
// AUTHOR: Ian Barberi - created April 3, 2006
//***************************************************************************/

#pragma once

#include "maxheap.h"
#include "iFnPub.h"

//! \brief Dialog Monitor Notification Callback Interface
/*! This interface is implemented by any object that wants to receive
Dialog Monitor notification messages.
*/
class IDialogMonitorNotification: public MaxHeapOperators {

public:
   //! \brief Dialog Monitor Notification Callback method
	/*! Returned value 'true' will prevent the notification message from continuing
	\param[in] hWnd the window handle sent by Dialog Monitor System 
	\return true if window processed (closed) by client, false otherwise
	*/
	virtual bool WindowNotification( HWND hWnd ) = 0;

};


//! ID for Dialog Monitor interface
#define IDIALOGMONITOR_INTERFACE	Interface_ID(0x5024223b, 0x4c6026e6)

//! \brief Interface to Dialog Monitor System
/*! This interface gives access to the Dialog Monitor System.
*/
class IDialogMonitor : public FPStaticInterface {

public:
	//! \brief Get Dialog Monitor state
	/*! This checks if the Dialog Monitor is enabled.
	\return true if the Dialog Monitor is enabled, false if not.
	*/
	virtual bool			IsDialogMonitorEnabled() = 0;
	
	//! \brief Set Dialog Monitor state.
	/*! \param[in] bDialogMonitor true to enable, false to disable.
	*/
	virtual void			SetDialogMonitor( bool bDialogMonitor ) = 0;
	
	//! \brief Get the Interactive Mode state.
	/*! If true, then a dialog pops up asking if you want to close a dialog or not.
	/*! This helps when debugging a script or attempting to do anything interactivly.
	\return true if Interactive Mode is enabled, false if not.
	*/
	virtual bool			IsInteractiveModeEnabled() = 0;
	
	//! \brief Set Interactive Mode state.
	/*! \param[in] bInteractive true to enable, false to disable
	*/
	virtual void			SetInteractiveMode( bool bInteractive ) = 0;

	//! \brief Register for Dialog Monitor Notification Messages
	/*! 
	\param[in] pNotification pointer to IDialogMonitorNotification interface
	\return true if success, false if failure
	*/
	virtual bool RegisterNotification(IDialogMonitorNotification *pNotification) = 0;
    
	//! \brief Unregister for Dialog Monitor Notification Messages
	/*! 
	\param[in] pNotification pointer to IDialogMonitorNotification interface
	\return true if success, false if failure
	*/
	virtual bool UnRegisterNotification(IDialogMonitorNotification *pNotification) = 0;
  

};

//! Access to IDialogMonitor interface
#define GetIDialogMonitor() ((IDialogMonitor*)GetCOREInterface(IDIALOGMONITOR_INTERFACE))

