/*********************************************************************
 *<
	FILE: evuser.h

	DESCRIPTION: Event user functionality

	CREATED BY:	Tom Hudson

	HISTORY: Created 16 June 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

/**********************************************************************
How to use:

This is a set of classes which form a generic notification system.  To
use:

* Create an EventUser object.

* Register the EventUser object with the appropriate router.

* The EventRouter will call your EventUser's Notify() method when
the event occurs.

* When you're done with the EventUser object, call the EventRouter's
UnRegister() method.  This will delete the EventUser from the router's
notification system.

* If your code is part of a window proc, call the router's Register
and UnRegister methods when the window receives WM_ACTIVATE messages.
This will properly uncouple the notification system when the window is
deactivated.

**********************************************************************/

#pragma once
#include "maxheap.h"

/*! \sa  Class EventRouter, Class Interface.\n\n
\par Description:
This is a generic event notification system. The only two places this is
currently used are for when the Delete key is pressed or the Delete menu item
is selected, and when the Backspace key is pressed. The usage can be seen in
<b>/MAXSDK/SAMPLES/MODIFIERS/EDITPAT.CPP</b>, where the Edit Patch modifier
sets up for notification of Delete operations. It is also used in
<b>EDITSPL.CPP</b> where it deals with deletion of selected items.\n\n
To use it:\n\n
<b>1.</b> Create an EventUser object.\n\n
<b>2.</b> Register the EventUser object with the appropriate router.\n\n
<b>3.</b> The EventRouter will call the EventUser's <b>Notify()</b> method when
the event occurs.\n\n
<b>4.</b> When you're done with the EventUser object, call the EventRouter's
<b>UnRegister()</b> method. This will delete the EventUser from the router's
notification system.\n\n
<b>5.</b> If your code is part of a window proc, call the router's Register and
UnRegister methods when the window receives WM_ACTIVATE messages. This will
properly uncouple the notification system when the window is deactivated. <br>
*/
class EventUser: public MaxHeapOperators {
	public:
		/*! \remarks This is the proc called by the <b>EventRouter</b> when the event
		occurs. */
		virtual void Notify()=0;
	};

