/*==============================================================================
// Copyright (c) 1998-2008 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Interface to the Simple Viewport Button System
// AUTHOR: Michael Zyracki created May/June 2008
//***************************************************************************/
#pragma once

#include "MaxHeap.h"
#include "ifnpub.h"
#include "strclass.h"
#include "IPoint2.h"
#include "color.h"
#include "box2.h"
// forward declarations
class GraphicsWindow;
class ViewExp;

//! \brief Viewport button abstraction.
/*! The new viewport menu system is implemented using this interface.  It provides
control over the text to display and location to be drawn in.  ViewportButtonText implements
this interface to draw text based button in the viewport.
*/
class IViewportButton :public MaxHeapOperators
{

public:
	//constructor/deconstructor
	IViewportButton(){};
	virtual ~IViewportButton(){};

	//properties
	/*! Access to the enabled flag
	\returns TRUE if enabled
	*/
	virtual bool GetEnabled()const =0;

	/*! Set the Enabled flag. If set to false, the button will not be drawn
	\param v TRUE to enable, FALSE to disable the button.
	*/
	virtual void SetEnabled(bool v) =0;

	/*! Access to the button text to be displayed in the viewport
	\returns A string containing the current label to be displayed.
	*/
	virtual MSTR GetLabel()const =0;

	/*! Set the text to be displayed in the viewport
	\param &label  The string to set.
	*/
	virtual void SetLabel(MSTR &label)=0;

	/*! Get the current location for the button
	\returns The location of the button
	*/
	virtual IPoint2 GetLocation() const =0;

	/*! Set the location of the button
	\param &loc The new location for the button
	*/
	virtual void SetLocation(IPoint2 &loc)=0;

	/*! Set the color for the button to be used
	\param &c The color for button
	*/
	virtual void SetColor(Color &c) =0;

	/*! Get the current color for the button
	\returns The current color
	*/
	virtual Color GetColor()const =0;

	/*! Defines whether the button is only shown in the current active viewport
	\returns TRUE if only drawn in the active viewport
	*/
	virtual bool ShowInActiveViewportOnly()=0;
	//Callbacks
	enum Action{
		eLeftClick = 0x0,
		eRightClick,
		eMouseMove,
	};

	/*! A callback to allow the button to perform an action, such as pop up a menu, or run a function
	\param hwnd The Window handle where the action we performed
	\param gw A pointer to the GraphicsWindow
	\param hitLoc The point where the action occured - this allows the button to perform a hittest.
	\param action The type of action
	\returns TRUE if an action was performed.
	*/
	virtual bool OnAction(HWND hwnd,GraphicsWindow *gw,IPoint2 hitLoc,IViewportButton::Action action)=0;

	/*! An update callback that can be used to update internal data structures.  The ViewportTextButton uses this
	to determine the Label to be used and location based on the current viewport being drawn.
	\param hwnd The Window handle issueing the update request
	\param *vpt A pointer to current View
	\returns TRUE if an update was implemented.
	*/
	virtual bool OnUpdate(HWND hwnd, ViewExp * vpt)=0;
	
	/*! Callback to display the button.  The ViewportTextButton uses this to draw text into the viewport using 
	GraphicsWindow API.  All drawing should go through this API - no GDI.
	\param t The current time
	\param *vpt THe viewport to draw into
	\param flags
	*/
	virtual void Display(TimeValue t, ViewExp *vpt, int flags) = 0;		

	/*! Get the viewport rect for the button
	\param t The current time
	\param *vpt The active viewport
	\param *rect The result of the viewport rect is stored here.
	*/
	virtual void GetViewportRect( TimeValue t, ViewExp *vpt, Rect *rect )=0;


};



//! \brief A simple text based button implementation of the IViewportButton. 
/*! A new viewport text button that is used to cerate the new viewport buttons in 3ds max 2010
It create a [XXXXX] button format, and support mouse over high lights.  Developers can derive from this 
class to provide their own actions and update functions.  TO use this class you must import CORE.LIB 
into your projects.

Please read the IViewportButton help section for an explanation of the methods.
*/
class ViewportTextButton: public IViewportButton
{
public:
	CoreExport ViewportTextButton();
	CoreExport ~ViewportTextButton();
	
	//properties
	CoreExport bool GetEnabled()const;
	CoreExport void SetEnabled(bool v);
	CoreExport MSTR GetLabel()const;

	/*! The developer simply sends in an unformated string, this class will then format it correct
	to [ XXXXX ]
	*/
	CoreExport void SetLabel(MSTR &label);
	CoreExport IPoint2 GetLocation() const;
	CoreExport void SetLocation(IPoint2 &loc);

	/*! The default color will be defined by Viewport Text CUI entry.  There is a new entry called
	Viewport Text Highlight that defines the mouse over color
	*/
	CoreExport void SetColor(Color &c);
	CoreExport Color GetColor()const;
	CoreExport bool ShowInActiveViewportOnly();
	
	/*! Implementation of OnAction.  Developers must perform hittesting in there derived function
	and update the mMouseOver flag, followed by calling the base implementation.  This is to allow
	the mouse over highlighting to work.
	*/
	CoreExport bool OnAction(HWND hwnd,GraphicsWindow *gw,IPoint2 hitLoc,IViewportButton::Action action);
	CoreExport bool OnUpdate(HWND hwnd, ViewExp * vpt);	
	//Display
	CoreExport void Display(TimeValue t, ViewExp *vpt, int flags);		
	CoreExport void GetViewportRect( TimeValue t, ViewExp *vpt, Rect *rect );

protected:
	bool mEnabled;
	MSTR mButtonText;
	MSTR mLabel;
	IPoint2 mLocation; 
	Color mColor;
	bool mMouseOver;
	bool mOldMouseOver;
};


//! \brief The unique interface ID for the Viewport Button Manager
#define IVIEWPORTBUTTONMANAGER_INTERFACE Interface_ID(0x47ab2195, 0x22e8126e)

//! \brief A manager to control the usage of the viewport buttons
/*! A Manager that the developer uses to register viewport buttons.  It also contains helper function
to provide global hittesting and per button hittesting.  The developer can use these from within the OnAction
callback.  The manager is not responsible for deleting the manager.  The developer should Unregister and then
delete the button\n
To use IViewportButtonManager simply use
static_cast<IViewportButtonManager*>(GetCOREInterface(IVIEWPORTBUTTONMANAGER_INTERFACE ))
*/
class IViewportButtonManager : public FPStaticInterface
{
public:

	//! Register a new button
	/*! Register a new button with the manager.  The system will check against buttons already registered
	\param *button The new button to register
	*/
	virtual void RegisterButton(IViewportButton *button)=0;

	//! Remove a previously registered button
	/*! Remove a button already registered with the system.  The manager will not delete the button, 
	simply removes it from the table
	\param * button The button to remove
	*/
	virtual void UnRegisterButton(IViewportButton *button)=0;

	//! A query to check if a button is registered
	/*! Check to see if the button is registered.  Developers can use this to check to see if a button 
	is registered before trying to register it
	\param *button The button to test against
	*/
	virtual bool IsButtonRegistered(IViewportButton *button)const=0;

	//! The total number of buttons 
	/*! The number of button registered with the manager
	\returns The total button count
	*/
	virtual int GetNumRegisteredButtons()const=0;

	//! Access an indivdual button
	/*! Access to a specfiic button by index.
	\param index The index of the button to acess
	\return The actual button, or NULL if index is invalid
	*/
	virtual IViewportButton * GetViewportButton(int index)=0;
	
	//! globally enable/disable the viewport buttons
	/*! A global method for enabling the viewport button.  If the buttons are disabled, nothing will be displayed
	or processed.
	\param val TRUE if the buttons are to be enabled
	*/
	virtual void EnableButtons(bool val)=0;

	//! Globally acces whether the viewport buttons are enabled
	/*! A global method for querying viewport buttons enabled status.  If the buttons are disabled, nothing will be displayed
	or processed.
	\returns TRUE if enabled
	*/
	virtual bool GetButtonsEnabled()const =0;

	//! Sends the OnAction command
	/*! Send the OnAction command to all registered viewports
	\param hwnd The Window handle where the action we performed
	\param gw A pointer to the GraphicsWindow
	\param hitLoc The point where the action occured - this allow the button to perform a hittest.
	\param action The type of action
	\returns TRUE if an action was performed.
	*/
	virtual bool OnAction(HWND hwnd,GraphicsWindow *gw,IPoint2 hitLoc,IViewportButton::Action action) =0;

	//! Send the OnUpdate command
	/*! Sends the OnUpate command to all registered viewports
	\param hwnd The Window handle issueing the update request
	\param *vpt A pointer to current View
	\returns TRUE if an update was implemented.
	*/
	virtual bool OnUpdate(HWND hwnd, ViewExp * vpt)=0;

	//! Provides a single hittest of the text
	/*! Utility function to perform hittesting on the supplied string and the location provided
	The derived IViewportButton can use this for localised hit testing, for example in the OnAction method
	\param hwnd The window handle
	\param *gw GraphicsWindow pointer from the active viewport
	\param stringStartLoc The start location of the supplied string
	\param hitLoc The mouse position
	\param string The label to hittest against
	\return TRUE if the mouse point intersects the string rectangle
	*/
	virtual bool HitTest(HWND hwnd,GraphicsWindow *gw,IPoint2 stringStartLoc, IPoint2 hitLoc,MSTR &string) = 0;

	//! Global hittesting on all registered buttons
	/*! Utility function to allow hittesting on all buttons registered.  It will simply return TRUE on the first
	successful hit test on the button label
	\param hwnd The window handle
	\param *gw The GraphicsWindow for the active viewport
	\param hitLoc The mouse position
	\return TRUE if the mouse point intersects any button's label
	*/
	virtual bool HitTest(HWND hwnd,GraphicsWindow *gw, IPoint2 hitLoc) = 0;

	//! Render the buttons
	/*! Global function to draw each button registered in the system.
	\param *vpt The current active viewport
	\param *rect If none NULL, the complete viewport Rect is stored here.
	*/
	virtual void DrawButtons(ViewExp * vpt, Rect * rect = NULL) =0; 

	//! Utility function for forcing a refresh of the button display.
	virtual void RefreshButtonDisplay() = 0;
};















