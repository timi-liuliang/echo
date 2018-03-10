/*********************************************************************
 *<
	FILE: evrouter.h

	DESCRIPTION: Event router functionality

	CREATED BY:	Tom Hudson

	HISTORY: Created 16 June 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "coreexp.h"
#include "maxheap.h"
#include "evuser.h"
#include "tab.h"

typedef EventUser* PEventUser;
typedef Tab<PEventUser> PEventUserTab;

/*! \sa  Class EventUser.\n\n
\par Description:
Event router functionality. All methods of this class are implemented by the
system.  */
class EventRouter: public MaxHeapOperators {
	private:
		PEventUserTab	userTab;
		BOOL			notifyMultiple;
	public:
		EventRouter(BOOL multiple = FALSE)	{ notifyMultiple = multiple; }
		/*! \remarks Register and activate an event user.
		\par Parameters:
		<b>EventUser *user</b>\n\n
		The EventUser to activate. */
		CoreExport void Register(EventUser *user);
		/*! \remarks Remove an event user from the list (automatically
		re-activates the previous user).
		\par Parameters:
		<b>EventUser *user</b>\n\n
		The EventUser to remove. */
		CoreExport void UnRegister(EventUser *user);
		// Process the event.  Returns TRUE if the event was handed off to a user.
		/*! \remarks Process the event.
		\return  TRUE if the event was handed off to a user. */
		CoreExport BOOL Process();
	};

extern CoreExport EventRouter deleteRouter;
extern CoreExport EventRouter backspaceRouter;
#ifdef _OSNAP
	extern CoreExport EventRouter tabkeyRouter;
#endif

