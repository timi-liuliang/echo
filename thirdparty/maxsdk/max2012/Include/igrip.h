/*==============================================================================
Copyright 2010 Autodesk, Inc.  All rights reserved.

Use of this software is subject to the terms of the Autodesk license agreement provided
at the time of installation or download, or which otherwise accompanies this software
in either electronic or hard copy form.   

//**************************************************************************/
// DESCRIPTION: Classes To Create and Manipulate Grips
// AUTHOR: Michael Zyracki created April 2009
//***************************************************************************/

#pragma once

#include "ifnpub.h"

//! \brief A pure virtual interface for implementing grips.
/*! class IBaseGrip
A Grip or Caddy is a collection of modeless, transparent, in canvas UI elements that lets a user perform actions or modify values.  A user can create their
own grip by inheriting from IBaseGrip, implementing IBaseGrip's pure virtual functions, and then enable it on via the IGripManager interface. 
Based upon the number of grip items the grip contains and the type each item is, the system will create a set of in canvas UI representations for each
item, which then interact with the IBaseGrip item via various functions.  Usually the implementation of the IBaseGrip acts as a bridge between some existing data
and the in canvas UI. For example if a grip was created to modify different soft selection values in a modifier, the grip would get the values from that 
modifier and then set the values back onto it.  The grip itself will get displayed via the IGripManager::EnableGrip function which basically calls various IBaseGrip
functions to set up the in canvas UI and then ferry messages to and from the UI to the grip.
\see IGripManager.  There are a collection of implemented grips present in the sdk at maxsdk\samples\mesh\editablepoly\polygrips.h, polygrips.cpp
*/
class IBaseGrip :  public BaseInterface
{

public:

	//! Deconstructor
	virtual ~IBaseGrip(){};

	/*! \defgroup Type Grip Value Types 
		Possible supported grip item types. The type determines it's UI representation and what types of data it represents.
	*/
	//@{
	enum Type
	{
		//! The type is invalid.  Usually used when trying to get a grip item which is out of range.
		eInvalid = -1,
		//! This type of the grip element represents an integer, and expects a IBaseGrip::GripValue::mInt value when getting and setting values.
		//! The grip UI will be a spinner.
		eInt = 0,
		//! This type of the grip element represents a float, and expects a IBaseGrip::GripValue::mFloat value when getting and setting values.
		//! The grip UI will be a spinner.
		eFloat,
		//! This type of the grip element represents a TimeValue, and expects a IBaseGrip::GripValue::mInt value, in the form of a tick value,
		//! when getting and setting values. The grip UI will be a spinner.
		eTime,
		//! This type of the grip element represents a value in world space that respects the current system settings,
		//! and expects a IBaseGrip::GripValue::mFloat value when getting and setting values.  The grip UI will be a spinner.
		eUniverse,
		//! This type of the grip element represents a combination of limited distinct choices, and expects a IBaseGrip::GripValue::mCombo value when getting and setting
		//! values.  The UI will present the choices in a drop down menu.
		eCombo,
		//! This type of the grip element is a Boolean toggle and expects a IBaseGrip::GripValue::mbool value when getting and setting values.
		eToggle,
		//! This type of grip element performs an action and doesn't support the getting and setting values.
		eAction,
		//! This type of grip element performs a command action, which means an action that has an off state and a on state. For example a pick action, is
		//! a command action since there is a pick state that's active during the process of picking.  It uses the IBaseGrip::GripValue::mbool value to determine if a mode is
		//! active or not.
		eCommand ,
		//! This type of grip element just show's text and doesn't perform any actions or the getting or setting of any values.
		eStatus
	}; 
	//@}


	/*! \defgroup Customization  Customization of Grips 
		This allows each grip item to get customized in some way that differentiate its behavior from the default behavior for that Type. Note in
		general these customization flags are just checked when the grip is first activated so if they change while the grip is already
		active they may not get enforced.
	*/
	//@{
	enum Customization
	{
		//! This grip item should be drawn on the same row as the previous grip item. When not set the new item
		//! always creates a new row, which is the default.
		eSameRow = 0x1,
		//! This grip won't show any text label in when active, only a spinner.  Only valid for grip items that show spinners.
		eTurnOffLabel = 0x2,
		//! This grip will disable the default ALT key behavior for this grip item. Only valid for grip items that show spinners.
		eDisableAlt = 0x4
		
	}; 
	//@}


	//! The grip value that's used by the grip item when it's value is retrieved via GetValue or set via SetValue.
	struct GripValue : public MaxHeapOperators
	{
		union
		{
			//! The value is an integer
			int mInt;
			//! The value is a float
			float mFloat;
			//! The value is a distinct combination set
			int mCombo;
			//! The value is a Boolean
			bool mbool;
		};
		//! Reserved. Currently not in use.
		DWORD mFlag; 
	};

	//! The label and icon that make up each individual option for Type::eCombo grip item.
	struct ComboLabel : public MaxHeapOperators
	{
		//! The string label for the radio option
		MSTR mLabel;
		//! The icon file
		MSTR mIcon;
	};

	//! \name Operations
	//! \brief The Okay, Cancel and Apply operations that a grip may implement.
	//@{

	/*! Whether or not, the Okay, Apply and Cancel grip buttons will get shown or not.
	    \return If true the grip will have Okay, Apply and grip buttons that will then trigger the correct callback functions listed below.
		If false, then the grip won't show these buttons.
	*/
	virtual bool SupportsOkayApplyCancel() =0;

	/*! Accept any necessary changes and disable the grip to close the UI.  Basically do whatever operations you would do similar if the Okay button
		was pressed in a modeless dialog.  Note that it's the implementations responsibility to disable the grip.
	    \param[in] t The time at which to perform the okay.
	*/
	virtual void Okay(TimeValue t)=0;

	/*! Disregard and cancel any necessary changes and disable the grip to close the UI.  Basically do whatever operations you would do similar if the
		Cancel button was pressed in a modeless dialog.  Note that it's the implementations responsibility to disable the grip.
	*/
	virtual void Cancel()=0;

	/*! Accept any necessary changes and perform any pending action if needed.  Unlike Okay, after an Apply the grip stays active and displayed.
	    \param[in] t The time at which to perform the apply.
	*/
	virtual void Apply(TimeValue t)=0;
	//@}

	/*! Get the name of the grip.
	    \param[out] string The name of the grip.
	*/	
	virtual void GetGripName(MSTR &string) =0;

	/*! Get the number of grip items this grip contains.
	    \return The number of grip items this grip contains.
	*/	
	virtual int GetNumGripItems() =0;

	/*! Get the type that the specified grip item will control.
	    \param[in] which The zero based index of the item. 
	    \return The type of the specified grip item. \see IBaseGrip::Type to see the different types that a grip item may be.  The type
		that the grip item is determines what IBaseGrip::Value values the grip item supports.  If the index is out of range then return
		Type::eInvalid.
	*/	
	virtual IBaseGrip::Type GetType(int which) =0;

	/*! Get the name of the specified grip item.
	   \param[in] which The zero based index of the item. 
	   \param[out] string The name of the specified grip item. 
	   \return true if the operation succeeded, false otherwise.
	*/	
	virtual bool GetText(int which,MSTR &string) =0;

	/*! Get the file path of the icon that represents the specified grip item. Every grip item no matter it's type should try to provide an icon
	   that represents the string that's returned by GetText.
	   \param[in] which The zero based index of the grip item.
	   \param[out] string The name of the resolved path of the icon. It's this functions responsibility to 
	   resolve the path completely.
	   \return true if the operation succeeded, false otherwise.
	*/	
	virtual bool GetResolvedIconName(int which,MSTR &string) =0;

	/*! Get any special customization flags that this grip item needs to follow.  \see IBaseGrip::Customization.
	    \param[in] which The zero based index of the item. 
	    \return Returns a bitwise union of IBaseGrip::Customization types, casted to a DWORD, that specify what if any customizations this grip item will have that differ from the default
		behavior of that grip item. Note that this customization is only checked when the grip is activated and its UI is constructed. 
		Return zero if index is invalid or no customization occurs.
	*/	
	virtual DWORD GetCustomization(int which) =0;

	/*! Get the option possibilities for this combo grip item
	    \param[in] which The zero based index of the item. 
		\param[out] cobmoOptions  A returned tab of labels that describes each possible option for the IBaseGrip::eCombo type grip button. These values 
		will be presented in the UI	as the possible options which can be selected.  Note that it's the responsiblity of the client of this 
		function to delete each allocated ComboLabel pointer in the Tab.
	    \return Returns true if the options were correctly set up, false if not, for example if this grip item isn't a eCombo type.
	*/	
	virtual bool GetComboOptions(int which, Tab<ComboLabel*> &comboOptions)= 0;

	/*! Get an icon for the IBaseGrip::eCommand grip item.  Since a IBaseGrip::eCommand can have two different states,  
		the grip item can show additional icons, in addition to the one returned by GetResolvedIconName, to represent these states.
		\param[in] which The zero based index of the grip item.
	    \param[out] string The name of the resolved path of the icon. It's this functions responsibility to resolve the path
		completely.
	    \return Returns true if an icon was resolved up, false if not, for example if this grip item isn't a IBaseGrip::eCommmand type.
	*/	
	virtual bool GetCommandIcon(int which, MSTR &string)= 0;
	
	/*! Get the value of the specified grip item.
	   \param[in] which The zero based index of the grip item.
	   \param[in] t The time at which to get the value.
	   \param[out] value The value for that grip. See \IBaseGrip::GripValue to see the values that are supported.
	   \return true if the operation succeeded, false otherwise.
	*/	
	virtual bool GetValue(int which,TimeValue t,IBaseGrip::GripValue &value) =0;
	
	//!\name Set Functions
	//! \brief Set of functions which are called when setting a value.  Note that SetValue may be called multiple times within each StartSetValue\EndSetValue pair.
	//@{
	/*! Set the value of the specified. 
	   \param[in] which The zero based index of the grip item.
	   \param[in] t The time at which to set the value.
	   \param[in] value The value for that grip. See \IBaseGrip::GripValue to see the values that are supported.
	   \return true if the operation succeeded, false otherwise.
	*/	
	virtual bool SetValue(int which,TimeValue t,IBaseGrip::GripValue &value) =0; 
	
	/*! Get ready to set this value. Sometimes an item needs to do some setup before a value get's set, for example cache some values, or get ready for an
		undo.  This is called before SetValue is called and SetValue may be called multiple times after StartSetValue is called.
	   \param[in] which The zero based index of the grip item.
	   \param[in] t The time at which to set the value.
	   \return true if the operation succeeded, false otherwise.
	*/
	virtual bool StartSetValue(int which,TimeValue t) =0;
	
	/*! The value is done setting values. Sometimes an item needs to do some cleanup after a value get's set, for example delete some cached values or
	    accept an undo.  This will get called immediately after the last SetValue function get's called.
	   \param[in] which The zero based index of the grip item.
	   \param[in] t The time at which the value was set.
	   \param[in] accepted If true the setvalue operation has succeeded and any undo's may be accepted if needed. If false then the setvalue operation
	   was canceled and it's the implementors responsibility to rever the value back to it's original value it had when StartSetValue was called, 
	   for example it could do this by canceling the hold.
	   \return true if the operation succeeded, false otherwise.
	*/
	virtual bool EndSetValue(int which,TimeValue t,bool accepted) = 0;
	//@}

	/*! Whether or not key brackets, which will show in a spinner that a key is present,  should be shown at the specified time.  
		Usually this function will check to see if a key exists at this time.
	   \param[in] which The zero based index of the grip item, which we are checking key brackets for. Will only check for those grip items which have spinners.
	   \param[in] t The time at which we are checking to see if we should show a key bracket.
	   \return true if a key bracket should be shown at that time, false otherwise.
	*/	
	virtual bool ShowKeyBrackets(int which,TimeValue t) =0;

	//!\name Scaling Functions
	//! \brief Set of functions which are called when the grip is scaled by either incrementing or decrementing it.  This value is the value that will thus
	//! added or subtracted from it. Only supports those grip items which have spinners.
	//@{
	/*! Whether or nor we will use auto scaling when incrementing or decrementing this item.  
	   \param[in] which The zero based index of the grip item.
	   \return Returns true if the grip should auto scale this value, false if not.
	*/	
	virtual bool GetAutoScale(int which)=0;

	/*! Get the scale value for this item when we increment or decrement it. Will normally only get used if GetAutoScale and GetSCaleInViewSpace returns false.
	   \param[in] which The zero based index of the grip item.
	   \param[out] scaleValue The value of the scale. 
	   \return true if the operation succeeded, false otherwise.
	*/	
	virtual bool GetScale(int which, IBaseGrip::GripValue &scaleValue) =0;

	/*! Get the scale value for this grip in view space, if one exists.
	   \param[in] which The zero based index of the grip item.
	   \param[out] depth The screen space depth that the scale should use. /see ViewExp::MapScreenToView for a more in depth description on screen depth.
	   -200 is usually a good default value.
	   \return Returns true if we should support scaling in view space, false if not. If false then we either using auto scaling or the normal incremental scale
	   value specified by GetScale, based upon the result of GetAutoScale.  Screen space scaling is used when dragging on a grip item spinner.
	*/	
	virtual bool GetScaleInViewSpace(int which,float &depth) = 0 ;
	//@}

	/*! Get the range of this grip values.
	    \param[in] which The grip item
		\param[in] minRange The minimum range value for this grip. If the grip type is Type::eAction then no valid value is expected.
		\param[in] maxRange The maximum range value for this grip. If the grip type is Type::eAction then no valid value is expected.
		\return true if the operation succeeded, false otherwise.
	*/	
	virtual bool GetRange(int which,IBaseGrip::GripValue &minRange, IBaseGrip::GripValue &maxRange)=0;

	/*! Get the reset value of the grip.
	    \param[in] which The grip item whose reset value we will get.
		\param[out] resetValue The resetValue for that value.  If the grip type is eAction then no valid value is expected.
		\return true if the operation succeeded, false otherwise.
	*/	
	virtual bool GetResetValue(int which,IBaseGrip::GripValue &resetValue)=0;

	/*! Reset the specified grip.
	    \param[in] t The time at which to reset.
		\param[in] which Which grip item to reset.
		\return true if the operation succeeded, false otherwise.
	*/	
	virtual bool ResetValue(TimeValue t,int which)=0;

};

//! \brief A grip changed callback
//! A callback object passed to IGripManager::RegisterGripChangedCallback() The
//! method GripChanged is called every time a different grip or no grip becomes active.  */
class GripChangedCallback: public MaxHeapOperators {
	public:
		//! Deconstructor
		virtual ~GripChangedCallback() {}
		//! \brief Called whenever a new grip becomes active.
		//! \param[in] grip - The new active grip. If NULL, no grip is active.
		virtual void GripChanged(IBaseGrip *grip)=0;
	};



//! \brief Interface ID for the IGripManager Interface 
//! \see IGripManager
#define IGRIPMANAGER_INTERFACE	 Interface_ID(0x59cb513c, 0x7a0a5232)

//! Helper method for getting the IGripManager interface
#define GetIGripManager()	static_cast<IGripManager*>(GetCOREInterface(IGRIPMANAGER_INTERFACE))


//! \brief The interface to the functionality for using the Grip Manager
/*! This class contains key functionality for using grips. It contains functionality for setting which grip is active in addition to various
options for how grips are displayed and manipulated. You get this interface by calling GetIGripManager(). This class is created by the system.
\see IGRIPMANAGER_INTERFACE
\see GetIGripManager()
*/
class IGripManager : public FPStaticInterface {

public:
	//!\name Enable/Disable
	//@{
	/*! Make the grip active and show it.
	    \param[in] grip The grip that will turn active and have its UI displayed.  If another grip was active, that grip becomes inactive. Only one
		grip may be active at a time. If the value is NULL this functions acts like SetGripsInactive.
	*/	
	virtual void SetGripActive(IBaseGrip *grip) =0;
	
	/*! Inactivate the current active grip. If no grip is active, nothing happens.
	*/	
	virtual void SetGripsInactive() = 0;

	/*! Get the current active grip. 
	\return Returns the current active grip. If no grip is active, returns NULL.
	*/	
	virtual IBaseGrip *GetActiveGrip() =0;
	//@}

	//!\name Grip Item
	//@{
	/*! Set whether or not a particular grip item of the active grip will be activated or not. By default, all grip items are activated and interactive.
	    \param[in] whichItem The zero based index for the item for which we will be activate or not. If out of range of the number of grip items,
		on the active grip this function will do nothing.
		\param[in] active If true the item is active and can get manipulated, if false, the item is disabled and greyed out in the viewport.
		\return true if the operation succeeded, false otherwise.
	*/	
	virtual bool ActivateGripItem(int whichItem, bool active) = 0;

	/*! Get whether or not a particular grip item of the active grip is active or not.
	    \param[in] whichItem The zero based index for the item for which we are checking to see if it's active or not.
		\return Returns true if the item is active, false if the item is not and it's been disabled. If the whichItem parameter is out of range or
		no grip is active then this function will return false.
	*/	
	virtual bool IsGripItemActive(int whichItem) = 0;

	/*! Set whether or not a particular grip item of the active grip will be visible or not.  By default, all grip items are visible.
	    \param[in] whichItem The zero based index for the item for which we will display or hide. If out of range of the number of grip items,
		on the active grip this function will do nothing.
		\param[in] hide If true the item is hidden and is not displayed, if false, the item is displayed. Note that
		by default all grip items are visible.  Note in order to see the change in the UI, you must call ResetAllGripUI().
		\return true if the operation succeeded, false otherwise.
	*/	
	virtual bool HideGripItem(int whichItem, bool hide) = 0;

	/*! Get whether or not a particular grip item of the active grip is visible or not.
	    \param[in] whichItem The zero based index for the item for which we are checking to see if it's visible.
		\return Returns true if the item is visible, false if the item is not. If the whichItem parameter is out of range or
		no grip is active then this function will return false.
	*/	
	virtual bool IsGripItemVisible(int whichItem) = 0;
	//@}


	//!\name ResetUI
	//@{
	/*! Tell that grip item it should reset its UI, usually since some value has changed and needs to get refleced in the UI. When called 
		that item will call the necessary IBaseGrip functions, such as GetValue, to reset and recreate the UI,
	    \param[in] whichItem The zero based index for the item for which needs to reset its UI.
		\return Returns true if the item is active, false if the item is not. If the whichItem parameter is out of range or
		no grip is active then this function will do nothing and return false.
	*/	
	virtual bool ResetGripUI(int whichItem) =0;

	/*! Convienence function to reset all of a grip's UI. Resetting the UI will cause the system to get the various values from each grip item.
	*/	
	virtual void ResetAllUI() = 0;
	//@}

	//!\name Show
	//@{
	/*! Show grips. By default grips are shown when one is active.
		\param[in] val If true grips will be shown, if false no grips will be shown even if active.
	*/
	virtual void SetShow(bool val) =0;
	/*! Get if grips are shown. 
		\return If true grips will be shown,if false no grips will be shown even if active.
	*/
	virtual bool GetShow()const =0;
	//@}
	

	//!\name Grip Placement
	//@{
	/*! Get the location of the grip in screen space.
	   \param[out] x The x location of the grip in screen space.
	   \param[out] y The y location of the grip in screen space.
	*/
	virtual void GetXYLocation(float &x, float &y)=0;

	/*! Set the location of the grip in screen space.
	   \param[in] x The x location of the grip in screen space that you want the grip's upper right corner to be placed.
	   \param[in] y The y location of the grip in screen space that you want the grip's upper right corner to be placed.
	*/
	virtual void SetLocation(int x, int y) =0;	
	
	/*! Set whether or not the grip stays centered around the current selection gizmo.
	   \param[in] val If true the grip stays centered around the current selection gizmo, if false the grip stays fixed at it's current viewport location.
	*/	virtual void SetCenterOnSelected(bool val) =0;
	/*! Get whether or not the grip stays centered around the current selection gizmo.
	  return If true the grip stays centered around the current selection gizmo, if false the grip stays fixed at it's current viewport location.
	*/
	virtual bool GetCenterOnSelected()const =0;
	/*! Set the normalized x and y location of the grip in the active viewport. These values are used if GetCenterOnSelection returns false.
	   \param[in] x The normalized [0-1] x location of the grip in the active viewport. So a value of 0.5 will place the grip in the center of viewport.
	   \param[in] y The normalized [0-1] y location of the grip in the active viewport. So a value of 0.5 will place the grip in the center of viewport.
	*/
	virtual void SetCenterXYPos(float x,float y ) = 0;
	/*! Get the normalized y location of the grip in the active viewport. These values are used if GetCenterOnSelection returns false.
	   \return The normalized [0-1] y location of the grip in the active viewport. So a value of 0.5 will place the grip in the center of viewport.
	*/
	virtual float GetCenterYPos()const = 0;
	/*! Get the normalized x location of the grip in the active viewport. These values are used if GetCenterOnSelection returns false.
	   \return The normalized [0-1] x location of the grip in the active viewport. So a value of 0.5 will place the grip in the center of viewport.
	*/
	virtual float GetCenterXPos()const = 0;

	/*! Set the x and y pixel offset from the current selection gizmo where the grip will get placed. These values are used if GetCenterOnSelection returns true.
	   \param[in] offsetPixelX The x pixel offset off of the current selection where the grip will get placed.
	   \param[in] offsetPIxelY The y pixel offset off of the current selection where the grip will get placed.
	*/
	virtual void SetSelectedOffsetXY(int offsetPixelX, int offsetPixelY)=0;

	/*! Get the x pixel offset from the current selection gizmo where the grip will get placed. These values are used if GetCenterOnSelection returns true.
	   \return The x pixel offset off of the current selection where the grip will get placed.
	*/
	virtual int GetSelectedOffsetX()const=0;
	/*! Get the y pixel offset from the current selection gizmo where the grip will get placed. These values are used if GetCenterOnSelection returns true.
	   \return The y pixel offset off of the current selection where the grip will get placed.
	*/
	virtual int GetSelectedOffsetY()const=0;
	//@}

	//!\name Transparency
	//@{
	/*! Set the transparency of the grip items in the viewport.
	   \param [in] The transparency of the grip items in the viewport. Range is from 0, opaque to 1, fully transparent.
	*/
	virtual void SetTransparency(float val) =0;
	/*! Get the transparency of the grip items in the viewport.
	   \return Returns the transparency of the grip items in the viewport. Range is from 0, opaque to 1, fully transparent.
	*/
	virtual float GetTransparency()const=0;
	/*! Set the hover transparency of the grip items in the viewport.
	   \param [in] The transparency of the grip items in the viewport when the grip item has the mouse cursor over it. 
	   Range is from 0, opaque to 1, fully transparent.
	*/
	virtual void SetHoverTransparency(float val) =0;
	/*! Get the hover transparency of the grip items in the viewport.
	   \return Returns the transparency of the grip items in the viewport. Range is from 0, opaque to 1, fully transparent.
	*/
	virtual float GetHoverTransparency()const=0;
	//@}

	//!\name Widget Spacing
	//@{
	/*! Set the spacing in pixels between the main grip UI elements.
	   \param[in] spacing Set the pacing in pixels between the main grip UI elements.
	*/
	virtual void SetWidgetSpacing(float spacing)=0;

	/*! Get the spacing in pixels between the main grip UI elements.
	   \return Returns the pacing in pixels between the main grip UI elements.
	*/
	virtual float GetWidgetSpacing()const=0;
	//@}

	//!\name GripChangedCallback
	//@{
	/*!\brief Register a callback that will get called when the active grip changes.
	   \param[in] cB - The callback that you are registering. If NULL this function does nothing.
	*/
	virtual void RegisterGripChangedCallback(GripChangedCallback *cB) =0;

	/*!\brief Unregister the grip changed ballback
	  \param[in] cB - The callback that you are unregistering. If NULL or the callback is not registered
	  this function does nothing.\
	*/
	virtual void UnRegisterGripChangedCallback(GripChangedCallback *cB) =0;
	//@}

	/*! Function to recalculate the layout and position of all of the grip items. This usually just needs to get called by the system due to the
	fact that a window was moved or it's size changed or the selection changed.
	*/	
	virtual void RecalcLayout() =0;
};



