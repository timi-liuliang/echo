//
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.  
//
//

#pragma once
#include "maxheap.h"
#include <WTypes.h>

// This file can be included in plug-in modules so
// it shouldn't reference/include private classes or functions.
class MouseCallBack;
class ChangeForegroundCallback;
class HitByNameDlgCallback;
class PickModeCallback;
class PickNodeCallback;

/** This base class allows the developer to create a command mode that handles
 * processing user interaction using the mouse in the viewports. See the page on 
 * "Command Modes and Mouse Procs" and "Foreground / Background Planes".
 * There are methods in 3ds Max's Interface class to set and get the current
 * command mode.  
 * \see  Class MouseCallBack, Class ChangeForegroundCallback, page "
 */
class CommandMode: public MaxHeapOperators {
	public:
		/** Destructor. */
		virtual ~CommandMode() {} 
		/** Returns the Class of the command mode. The class describes the type of
		command mode this is. If the developer is defining a command mode to be
		used as part of the sub-object manipulation (Move, Rotate, and Scale)
		then one of the following pre-defined values in \ref Command_Classes should be used.
		\return  The Class of the command mode. */
		virtual int Class()=0;

		/** This method can be ignored. The default implementation should be used.*/		
		virtual int SuperClass() { return 0; }

		/** Returns the ID of the command mode. This value should be the constant
		CID_USER plus some random value chosen by the developer.
		As an example, this method could be implemented as:
		{ CID_USER+0x1423; }
		In the special case of the developer implementing a custom command mode
		to be used as part of sub-object manipulation (Move, Rotate or Scale)
		the value for ID() should be one of the following values:
		\li CID_SUBOBJMOVE 
		\li CID_SUBOBJROTATE 
		\li CID_SUBOBJSCALE 
		\li CID_SUBOBJUSCALE 
		\li CID_SUBOBJSQUASH 
		Note: It is not	a problem if two command modes conflict in this ID value,
		so the uniqueness is not strictly required. However, this
		ID() method is often used to check which mode is active. Therefore,  
		unless the value for your command mode is identifiable via this ID, you
		may not be able to recognize if your mode is indeed the active one. */
		virtual int ID()=0;

		/** This method establishes the number of points required by the command
		mode and returns a pointer to the mouse callback object that is used to
		process the user input.
		\param numPoints This is where to store the number of points used by the CommandMode. If
		the plug-in needs to use an undetermined number of points it can
		specify a large number for this value. When the mouse proc has finished
		processing points it returns FALSE to stop the point processing before
		the number of points specified here have been entered.
		\return  A pointer to the mouse callback object that is used to process the user input.
		\see  Class MouseCallBack. */
		virtual MouseCallBack *MouseProc(int *numPoints)=0;

		/** Returns a pointer to a callback procedure that flags nodes that belong
		in the foreground plane. Plug-ins typically use a standard callback
		object provided by the system that flags all nodes dependent on the
		plug-in object. This ensures that when the plug-in object changes, any
		nodes that change as a result will be in the foreground plane, making
		redraw time faster.
		These constants may be specified to use one of the standard callback
		objects instead of an actual FG proc. For example {return
		CHANGE_FG_SELECTED;}
		CHANGE_FG_SELECTED
		Selected nodes are flagged.
		CHANGE_FG_ANIMATED
		Nodes that are animated are flagged.
		\return  A pointer to a callback procedure that flags nodes that belong
		in the foreground plane.
		\see  For additional information see the Advanced Topics section
		"Foreground	/ Background Planes". */
		virtual ChangeForegroundCallback *ChangeFGProc()=0;

		/** This method returns TRUE if the command mode needs to change the
		foreground proc (using ChangeFGProc()) and FALSE if it does not.
		A command mode that does not involve any redrawing of the viewports can
		just return FALSE.
		\param oldMode This is the command mode that is currently in place. This may be used
		for comparison with a potential new mode. See the sample code below.
		\par Sample Code:
		The sample code below checks to see if the command mode is already
		CHANGE_FG_SELECTED. If it is there is no reason to change to
		foreground proc to this mode so the method returns FALSE. If a
		different mode is in place TRUE is returned.
		\code
		BOL ChangeFG( CommandMode *oldMode ) { return (oldMode->ChangeFGProc() != CHANGE_FG_SELECTED); }
		ChangeForegroundCallback *ChangeFGProc() { return CHANGE_FG_SELECTED; }
		\endcode  */
		virtual BOOL ChangeFG( CommandMode* oldMode )=0;
		/** This method is called when a command mode becomes active. Usually a
		developer responds by changing the state of a control to indicate to
		the user that they have entered the mode. Typically this means pushing
		in a tool button. When the mode is finished the button should be
		returned to normal (see ExitMode() below).
		Note: A developer should use the standard color GREEN_WASH for
		check buttons that instigate a command mode. While the command mode is
		active the button should be displayed in GREEN_WASH. See
		Class ICustButton (specifically
		the method SetHighlightColor()) for more details.
		\par Sample Code:
		iPick-\>SetHighlightColor(GREEN_WASH);
		iPick-\>SetCheck(TRUE); */
		virtual void EnterMode()=0;

		/** This method is called when the active command mode is replaced by a
		different mode. Typically a developer implements this method to set the
		state of the control that invoked the command mode to the 'out' state.
		See Class ICustButton
		(specifically the method SetCheck().
		\par Sample Code:
		iPick-\>SetCheck(FALSE); */
		virtual void ExitMode()=0;		
	};

/** This class contains a set of six command mode pointers that make up the XForm
 * modes. Plug-in developers can specify these for their sub-object types. See the
 * method BaseObject::ActivateSubobjSel() or class Control for more details. 
 */
class XFormModes: public MaxHeapOperators {
	public:
		CommandMode *move;		//!< Standard command mode to process Move.
		CommandMode *rotate;	//!< Standard command mode to process Rotate.
		CommandMode *scale;		//!< Standard command mode to process Non-Uniform Scale.
		CommandMode *uscale;	//!< Standard command mode to process Uniform Scale.
		CommandMode *squash;	//!< Standard command mode to process Squash.
		CommandMode *select;	//!< Standard command mode to process Selection.
		/** Constructor. The data members are set to the command
		modes passed. */
		XFormModes( 
			CommandMode *move_,
			CommandMode *rotate_,
			CommandMode *scale_,
			CommandMode *uscale_,
			CommandMode *squash_,
			CommandMode *select_ )
			{
			this->move   = move_;
			this->rotate = rotate_;
			this->scale  = scale_;
			this->uscale = uscale_;
			this->squash = squash_;
			this->select = select_;
			}
		/** Constructor. All the data members are set to NULL. */
		XFormModes() { move = rotate = scale = uscale = squash = select = NULL; } 
	};


// These can be returned from ChangeFGProc() instead of an actual FG proc
// to use predefined FG sets.
#define CHANGE_FG_SELECTED		((ChangeForegroundCallback *)1)	//!< Selected nodes are flagged.
#define CHANGE_FG_ANIMATED		((ChangeForegroundCallback *)2)	//!< Nodes that are animated are flagged. 

/// \defgroup Command_Super_Classes Command Super Classes
//@{
#define TRANSFORM_CMD_SUPER 	1
//@}

/// \defgroup Command_Classes Command Classes
//@{
#define VIEWPORT_COMMAND		1
#define MOVE_COMMAND 				2
#define ROTATE_COMMAND			3
#define SCALE_COMMAND				4
#define USCALE_COMMAND			5
#define SQUASH_COMMAND			6
#define SELECT_COMMAND			7
#define HIERARCHY_COMMAND		8
#define CREATE_COMMAND			9
#define MODIFY_COMMAND			10
#define MOTION_COMMAND			11
#define ANIMATION_COMMAND		12
#define CAMERA_COMMAND			13
#define NULL_COMMAND				14
#define DISPLAY_COMMAND			15
#define SPOTLIGHT_COMMAND		16
#define PICK_COMMAND				17
#define MANIPULATE_COMMAND	18
#define PICK_EX_COMMAND			19 //!< Extended Pick Command Mode
//@}

/// \defgroup Command_IDs Command IDs
//@{
#define CID_USER				0x0000ffff
//@}


/*! \defgroup stdCommandModes Standard Command Modes
These are the ID's to the standard Max-defined command modes.
\sa Class CommandMode, Interface::SetStdCommandMode */
//@{
/// \name Max Transform Modes
//@{
#define CID_OBJMOVE				1		//!< Max Transform Move Modes
#define CID_OBJROTATE			2		//!< Max Transform Rotate Modes
#define CID_OBJSCALE			3		//!< Max Transform Scale Modes
#define CID_OBJUSCALE			4		//!< Max Transform Uniform Scale Modes
#define CID_OBJSQUASH			5		//!< Max Transform Squash Modes
#define CID_OBJSELECT			6		//!< Max Transform Select Modes
//@}



/** \internal \name subobject Command IDs
The subobject command IDs are used internally and 
are not valid arguments to Interface::SetStdCommandMode */
//@{
#define CID_SUBOBJMOVE			7		//!< \internal This is not to be used externally
#define CID_SUBOBJROTATE		8		//!< \internal This is not to be used externally
#define CID_SUBOBJSCALE			9		//!< \internal This is not to be used externally
#define CID_SUBOBJUSCALE		10		//!< \internal This is not to be used externally
#define CID_SUBOBJSQUASH		11		//!< \internal This is not to be used externally
#define CID_SUBOBJSELECT		12		//!< \internal This is not to be used externally
//@}

/** \internal \name Display Branch Command Modes
These are also used internally and are not valid to SetStdCommandMode. */
//@{
#define CID_UNFREEZE			13		//!< \internal This is not to be used externally
#define CID_UNHIDE				14		//!< \internal This is not to be used externally
//@}

/// \name Hierarchy Commands
//@{
#define CID_LINK				100
#define CID_BINDWSM				110		// I guess this is a heirarchy command... sort of
//@}

/// \name Viewport Commands
//@{
#define CID_ZOOMVIEW			200
#define CID_ZOOMREGION			201
#define CID_PANVIEW				202
#define CID_ROTATEVIEW			203
#define CID_ZOOMALL				204
#define CID_RNDREGION			205
//@}

/// \name Camera Commands
//@{
#define CID_CAMFOV				210
#define CID_CAMDOLLY			211
#define CID_CAMPERSP			212
#define CID_CAMTRUCK			213
#define CID_CAMROTATE			214
#define CID_CAMROLL				215
//@}

#define CID_PLAYANIMATION		300	//!< Animation Command


//@}

#define CID_SIMPLECREATE		400	//!< Create Command

#define CID_MODIFYPARAM			500	//!< Modify Command

/** Command Mode Id for the Edit Soft Selection Command Mode.
\see EditSSMode */
#define CID_EDITSOFTSELECTION	525

#define CID_NULL				600	//!< Motion Command

/// \defgroup Pick_Modes Pick Modes
//@{
#define CID_STDPICK				710
#define CID_PICKAXISOBJECT		700
//@}

/// \defgroup Attach_to_Group_Commands Attach to Group Commands
//@{
#define CID_GRP_ATTACH			800
#define CID_ASSEMBLY_ATTACH	810
//@}

#define CID_MANIPULATE          900	//!< Manipulate Command Mode

/** \defgroup Special_Command_IDs Special Command IDs
 * Special Command IDs used internally by the transform gizmo
 * These are not to be used by third party developers. */
//@{
#define CID_FREE_AXIS_ROTATE	-INT_MAX
#define CID_SCREEN_SPACE_ROTATE -INT_MAX+1
//@}


/** Derive your command mode class from this one if you wish to implement your 
 * own pick command mode and want to hook it into the select by name mechanism.
 */
class PickCommandMode : public CommandMode
{
	public:
		virtual ~PickCommandMode() { };
		virtual int Class() { return PICK_EX_COMMAND; }
		virtual HitByNameDlgCallback* GetHitByNameDlgCallback() = 0;
		virtual PickModeCallback* GetPickModeCallback() = 0;
		virtual PickNodeCallback* GetPickNodeCallback() = 0;
};


