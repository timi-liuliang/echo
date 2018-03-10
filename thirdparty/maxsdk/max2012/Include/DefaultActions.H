//-----------------------------------------------------------------------------
// ----------------
// File ....: DefaultActions.h
// ----------------
// Author...: Larry Minton
// Date ....: June 2003
//
// History .: June, 19 2003 - Started
//
//-----------------------------------------------------------------------------
		
#pragma once
#include "maxheap.h"
#include <WTypes.h>
#include "strclass.h"

//! \brief Specifies the default maximum number of messages logged to the Default Action System 
//! that are retained. New messages that are logged once this limit is hit will not be retained.
#define DEFAULTACTIONS_DEFAULT_MSG_LOG_MAXSIZE 500

// Actions - set as bits in a DWORD
/*! \defgroup DefSysDefaultActions Default Action System: Actions
The following represent actions to be taken by the system when events described in \ref DefSysEvents occur. 
Note that any combination of these actions can be associated with any of the events.
Bits 3 to 7 are reserved for future internal use, while the remaining bits are event specific. 
*/
//@{
#define	DEFAULTACTIONS_LOGTOFILE		0x00000001 //!< Messages get logged to a file "<3dsMaxInstallPath>\network\Max.log"
#define	DEFAULTACTIONS_LOGMSG			0x00000002 //!< Messages get logged to the Default Action System and can be retrived via DefaultActionSys::GetMsgLogMsg
#define	DEFAULTACTIONS_ABORT			0x00000004 //!< The process that generated the event associated with this action is cancelled
#define	DEFAULTACTIONS_RESERVED_4		0x00000008
#define	DEFAULTACTIONS_RESERVED_5		0x00000010
#define	DEFAULTACTIONS_RESERVED_6		0x00000020
#define	DEFAULTACTIONS_RESERVED_7		0x00000040
#define	DEFAULTACTIONS_RESERVED_8		0x00000080
//@}


/*! \defgroup DefSysEvents Default Action System: Events
The following represent events that can occur within the system (3ds Max) during its operation.
Each of these events can be assoctiated with one or more actions listed in \ref DefSysDefaultActions.
Events with ids from 0 through 65535 are reserved for internal use.
3rd party plugins may define their own events with ids greater than 65535.
*/
//@{
#define TYPE_MISSING_EXTERNAL_FILES				1 //!< External assets cannot be found
#define TYPE_MISSING_DLL_FILES					2 //!< Plugin dlls cannot be found
#define TYPE_MISSING_XREF_FILES					3 //!< Xref assets (.max files) cannot be found
#define TYPE_MISSING_UVWS						4 //!< UVW coordinates cannot be found
#define TYPE_UNSUPPORTED_RENDEREFFECT			5 //!< Unsupported render effects have been found
#define TYPE_INVALID_XREF_FILES					6 //!< Files not suitable for xrefing were detected
#define DEFAULTACTIONS_MAX_DEFINED_ACTION		(TYPE_INVALID_XREF_FILES) //!< DEFAULTACTIONS_MAX_DEFINED_ACTION must contain the largest internally used value
//@}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//

//! \brief The Default Action System interface
/*! This interface allows for associating default actions to be taken by the
system (3ds Max) when certain predefined events occur, and managing these
associations. Note that the default actions are carried out only when the system
is runing in quiet mode, i.e. no dialogs are displayed.
\see LogSys
*/
class DefaultActionSys: public MaxHeapOperators {

 public:
	//! \brief Retrieves the action associated with an event
	/*! \param [in] eventID Identifier of a \ref DefSysEvents "predefined event" or a user defined one.
	\param [out] action If an action was specified for the given event, it is returned through this argument.
	The default action value for events which have not had an action set is DEFAULTACTIONS_LOGTOFILE
	The returned value can be a combination of the following \ref DefSysDefaultActions "predefined actions"
	\return - TRUE if an action was specified for the given event, otherwise FALSE.
	*/
	virtual	BOOL	GetAction (DWORD eventID, DWORD &action) = 0;

	//! \brief Sets the default action and descriptive title for an event.
	/*! \param [in] eventID Identifier of a \ref DefSysEvents "predefined event" or a user defined one.
	\param [in] action The \ref DefSysDefaultActions "actions" to be taken when the specified event occurs.
	\param [in] title The event's title. Appears in the log file or dialog displaying 
	the messages associated with the event.
	\param [in, out] oldAction The old \ref DefSysDefaultActions "action" value can be returned via this argument
	\return Returns TRUE if a default action was previously associated with the event
	*/
	virtual	BOOL	SetAction ( DWORD eventID, DWORD action, MCHAR* title = NULL, DWORD *oldAction = NULL) = 0;

	//! \brief Deletes the specified default action event
	/*! \param [in] eventID Identifier of a \ref DefSysEvents "predefined event" or a user defined one.
	\return Returns TRUE if the event was found and deleted
	*/
	virtual	BOOL	DeleteAction (DWORD eventID) = 0;

	//! \brief Retrieves the number of events registered with the Default Action System.
	/*! \return The number of events registered with the Default Action System.
	*/
	virtual	int		GetActionCount() = 0;

	//! \brief Retrieves the title of the specified event
	/*! \param [in] eventID Identifier of a \ref DefSysEvents "predefined event" or a user defined one.
	\return The title of the specified event, or an empty string if the event is not found.
	*/
	virtual	MSTR	GetActionTitle(DWORD eventID) = 0;

	//! \brief Retrieves the title of an event specified by its index in the Default Action System
	/*! \param [in] index The index of an event registered with the Default Action System
	\return The title of the event at the specified index, or an empty string if there's no event with the specified index.
	*/
	virtual	MSTR	GetActionTitleByIndex(int index) = 0;

	// Returns the indexed default action event's ID.
	//! \brief Retrieves the id of an event specified by its index in the Default Action System
	/*! \param [in] index The index of an event registered with the Default Action System
	\return The id of the event at the specified index.
	*/
	virtual	DWORD	GetActionIDByIndex(int index) = 0;

	//! \brief Logs a message for the specified event
	/*! \param [in] eventID Identifier of a \ref DefSysEvents "predefined event" or a user defined one
	\param [in] message The message to log for the specified event. The message is kept
	within the Defautl Action System. Use GetMsgLogMsg to retrieve messages logged for an event.
	\return Returns TRUE if message was successfully logged.
	*/
	virtual BOOL	LogEntry(DWORD eventID, MCHAR *message) = 0;

	//! \brief Clears logged messages
	/*! \param [in] eventID Identifier of a \ref DefSysEvents "predefined" or a user defined event.
	If eventID is 0, all messages are deleted, otherwise only messages for the specific event are deleted.
	*/
	virtual	void	MsgLogClear(DWORD eventID) = 0;

	//! \brief Retrieves the number of logged messages. 
	/*! \param [in] eventID Identifier of a \ref DefSysEvents "predefined" or a user defined event.
	If eventID is 0, all messages are counted, otherwise only messages for the specific event are counted.
	\return The number of messages
	*/
	virtual	int		GetMsgLogCount(DWORD eventID) = 0;

	//! \brief Retrives a specified logged message.
	/*! \param [in] eventID Identifier of a \ref DefSysEvents "predefined" or a user defined event.
	\param [in] index The index of a logged message to be retrieved.
	\return The message logged for the specified event, found at the specified index.
	*/
	virtual	MSTR	GetMsgLogMsg(DWORD eventID, int index) = 0;

	//! \brief Retrieves the event for a logged message 
	/*! \param [in] index The index of a message logged with the Default Action System.
	\return The id of the event for which the message at the specified index was logged.
	*/
	virtual	DWORD	GetMsgLogID(int index) = 0;

	//! \brief Sets the total maximum number of messages that can be logged with the Default Action Sustem.
	/*! \param [in] count The maximum number of messages that can be logged.
	\return The previous maximum number of messages.
	*/
	virtual	int		SetMsgLogMaxCount(int count) = 0;
};


//-- EOF: DefaultActions.h ---------------------------------------------------------------
