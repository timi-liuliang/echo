/**********************************************************************
*<
	FILE: utilapi.h

	DESCRIPTION: Utility plug-ins interface

	CREATED BY: Rolf Berteig

	HISTORY: 12-23-95 file created

*>	Copyright (c) 1994, All Rights Reserved.
**********************************************************************/

#pragma once
#include "maxheap.h"
#include "strclass.h"
// forward declarations
class Interface;

/*! \sa  Class UtilityObj.\n\n
\par Description:
This class provides a single method to allow the developer to close their
utility plug-in in the command panel.  */
class IUtil: public MaxHeapOperators {
	public:
		/*! \remarks Destructor */
		virtual ~IUtil() { }
		// Closes the current utility in the command panel
		/*! \remarks Implemented by the System.\n\n
		A developer may call this method to close the current utility in the
		command panel. If a plug-in developer chooses to use a Close button in
		one of their rollup pages this method should be called when the user
		has selected the Close button. This method simply tells the system that
		the plug-in should be closed. A developer does not need to use this
		method. It is just available if a developer wants to have a specific
		termination point to their utility plug-in. */
		virtual void CloseUtility()=0;
	};

// A utility plug-in object
/*! \sa  Class Interface, Class IUtil.\n\n
\par Description:
3ds Max utility plug-ins are derived from this class. Methods are provided for
editing the utilities parameters and responding to changes in the current
selection set. An interface pointer is provided for calling the utility methods
provided by MAX.\n\n
Note: Utility plug-ins are not a direct participant in the geometry pipeline
system of 3ds Max in the same way modifiers or space warps are. For this
reason, <b>UtilityObj</b> plug-ins are not suitable for modifying objects
flowing down the pipeline. Use <b>Modifier</b> or <b>WSModifier</b> plug-ins
for this purpose.\n\n
Also note: It is possible to create a utility plug-in that uses a modeless
dialog box. When 3ds Max itself uses modeless dialogs, it disables input to the
other open windows such as the Track %View, the Materials Editor, etc. In this
way, the user cannot perform some action that could disturb the operation of
the modeless dialog. For example using Track %View, a user could assign a
different controller to a node, and a utility plug-in might be accessing keys
from the node's previous controller. Since utility plug-ins cannot currently
prevent the user from operating these other parts of MAX, developers need to be
careful about the use of modeless dialogs.  */
class UtilityObj: public MaxHeapOperators {
	public:
		/*! \remarks Destructor */
		virtual ~UtilityObj() { }
		/*! \remarks This method is called when the utility plug-in may be used in the
		Utility branch of the command panel. The plug-in may add rollup pages
		to the command panel in this method for example.
		\param ip An interface pointer you may use to call methods of the Interface class.
		\param iu An interface pointer you may use to close the current utility in the command panel. */
		virtual void BeginEditParams(Interface *ip,IUtil *iu)=0;

		/*! \remarks This method is called when the utility plug-in is done being used in
		the Utility branch of the command panel (for example if the user
		changes to the Create branch).
		\param ip An interface pointer you may use to call methods of the Interface class.
		\param iu An interface pointer you may use to close the current utility in the command panel. */
		virtual void EndEditParams(Interface *ip,IUtil *iu)=0;
		/*! \remarks This method is called when the selection set changes. A plug-in may
		implement this method to respond to this condition.
		\param ip An interface pointer you may use to call methods of the Interface class.
		\param iu An interface pointer you may use to close the current utility in the command panel. */
		virtual void SelectionSetChanged(Interface *ip,IUtil *iu) { UNUSED_PARAM(ip); UNUSED_PARAM(iu);}

		/*! \remarks This method is called to delete the utility object allocated by
		<b>ClassDesc::Create()</b>.\n\n
		For example if the developer has used the <b>new</b> operator to
		allocate memory for their plug-in class they should implement this
		method as <b>{ delete this; }</b> to delete the plug-in object.\n\n
		In some cases it may be better to use a single static instance of the
		plug-in class and not allocate and deallocate memory. For example some
		of the sample utility plug-ins use a single static instance of their
		plug-in class. This is done so that if the user moves between branches
		in the command panel (goes into the Create branch and then returns to
		the Utility branch) all the utility plug-in parameters remain intact.
		If the memory was allocated and deallocated each time the parameter
		would be 'forgotten'. The samples that use a single static instance
		implement this method as <b>{}</b> (NULL). See the sample code in
		<b>/MAXSDK/SAMPLES/UTILITIES/COLCLIP.CPP</b> for an example. */
		virtual void DeleteThis()=0;
		/*! \remarks This method is called after <b>BeginEditParams()</b> when the user
		starts the utility from the command line with the option <b>-U</b> and
		passes an argument to the utility.
		\param param The command line argument is passed here.
		\par Default Implementation:
		<b>{}</b> */
		virtual void SetStartupParam(MSTR param) {}
	};



