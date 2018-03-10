/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: iMenus.h

	 DESCRIPTION: abstract classes for menus

	 CREATED BY: michael malone (mjm)

	 HISTORY: created February 17, 2000

   	 Copyright (c) 2000, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */
#pragma once

#include "maxheap.h"
#include "strclass.h"
#include "color.h"
#include "GetCOREInterface.h"

// forward declarations
class IPoint2;
class Box2;

#ifdef MENUS_EXPORTS
#define MENUS_API __declspec(dllexport)
#else
#define MENUS_API __declspec(dllimport)
#endif

enum Event
{
	EVENT_BEGIN_TRACK = 0,
	EVENT_CURSOR_DOWN,
    EVENT_RIGHT_CURSOR_DOWN,
	EVENT_CURSOR_MOVED,
	EVENT_CURSOR_UP,
	EVENT_END_TRACK,
	EVENT_KEY,
    EVENT_RIGHT_CURSOR_UP,
	EVENT_MIDDLE_CURSOR_DOWN, //RK:01/31/02, to support action options
	EVENT_MIDDLE_CURSOR_UP, 
};

enum EventParam { EP_NULL = 0, EP_SHOW_SUBMENU, EP_HIDE_SUBMENU };

/*! \sa Class IMenu, Class Interface.
\remarks This structure is available in release 4.0 and later only. \n
This structure is used internally.
*/
struct MenuEvent: public MaxHeapOperators
{
	/*! Values:
	- EVENT_BEGIN_TRACK
	- EVENT_CURSOR_DOWN
	- EVENT_RIGHT_CURSOR_DOWN
	- EVENT_CURSOR_MOVED
	- EVENT_CURSOR_UP
	- EVENT_END_TRACK
	- EVENT_KEY
	- EVENT_RIGHT_CURSOR_UP  */
	Event mEvent;
	/*! Values:
	- EP_NULL
	- EP_SHOW_SUBMENU
	- EP_HIDE_SUBMENU  */
	unsigned int mEventParam;
};

enum QuadIndex { QUAD_ONE = 0, QUAD_TWO, QUAD_THREE, QUAD_FOUR };

enum DisplayMethod { DM_NORMAL = 0, DM_STRETCH, DM_FADE, DM_NUM_METHODS };



// predeclarations
class IMenu;
class IMenuItem;
class ActionItem;


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/*! \sa  Class IMenu, Class IMenuItem, Class IMenuGlobalContext\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class is used internally. Methods that are marked as internal should not
be used.\n\n

\par Data Members:
<b>IMenu* mpMenu;</b>\n\n
<b>IMenuItem* mpItem;</b>  */
class ItemID: public MaxHeapOperators
{
public:
	IMenu* mpMenu;
	IMenuItem* mpItem;

	/*! \remarks This method is used internally.\n\n
	Constructor. Initialized <b>mpMenu(NULL)</b> and <b>mpItem(NULL).</b>
	\par Default Implementation:
	<b>{ }</b> */
	ItemID() : mpMenu(NULL), mpItem(NULL) { }
	/*! \remarks This method is used internally.\n\n
	Set <b>mpMenu = NULL</b> and <b>mpItem = NULL.</b> */
	void Null() { mpMenu = NULL; mpItem = NULL; }

	/*! \remarks This method is used internally.\n\n
	This operator tests for equality of two ItemID's.
	\par Parameters:
	<b>ItemID\& a, ItemID\& b</b>\n\n
	The two ItemID's you wish to test for equality. */
	friend bool operator==(ItemID& a, ItemID& b);
	/*! \remarks This method is used internally.\n\n
	This operator tests for inequality of two ItemID's.
	\par Parameters:
	<b>ItemID\& a, ItemID\& b</b>\n\n
	The two ItemID's you wish to test for inequality.
	\par Default Implementation:
	<b>{ return !(a == b); }</b> */
	friend bool operator!=(ItemID& a, ItemID& b) { return !(a == b); }
};

inline bool operator==(ItemID& a, ItemID& b)
{
	if ( a.mpMenu  != b.mpMenu  ||
		 a.mpItem  != b.mpItem )
		return false;
	else
		return true;
}



// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/*! \sa  Class IMenu , Class IMenuGlobalContext\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This abstract class represents an interface for a timer and is used internally.
Methods that are marked as internal should not be used.  */
class IMenuTimer: public MaxHeapOperators

//  PURPOSE:
//    Abstract class (Interface) for for a timer
//
//  NOTES:
//    created:  04.04.00 - mjm
//
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

{
public:
	/*! \remarks Destructor. */
	virtual ~IMenuTimer() {;}
	// checks if timer is running
	/*! \remarks This method is used internally.\n\n
	This method indicates whether the timer is running or not by returning TRUE
	or FALSE. */
	virtual bool IsRunning() = 0;

	// (re)starts the timer
	/*! \remarks This method is used internally.\n\n
	This method starts or restarts a timer for a specified IMenu. */
	virtual void Start(IMenu* pIMenu, EventParam timingType) = 0;

	// stops the timer
	/*! \remarks This method is used internally.\n\n
	This method will stop the timer. */
	virtual void Stop() = 0;

	// tells timer to check time. if elapsed, will notify its IMenu client
	/*! \remarks This method is used internally.\n\n
	This method instructs the timer to check the time. If the time has elapsed
	it will notify its IMenu client. */
	virtual void CheckTime() = 0;

	// checks if timer has elapsed
	/*! \remarks This method is used internally.\n\n
	This method indicates whether the timer has elapsed by returning TRUE or
	FALSE. */
	virtual bool HasElapsed() = 0;

	// sets/gets the elapse time
	/*! \remarks This method is used internally.\n\n
	This method allows you to set the elapse time for the timer.
	\par Parameters:
	<b>unsigned int elapseTime</b>\n\n
	The time at which the timer should elapse */
	virtual void SetElapseTime(unsigned int elapseTime) = 0;
	/*! \remarks This method is used internally.\n\n
	This method returns the elapse time that's been set for the timer. */
	virtual unsigned int GetElapseTime() const = 0;
	/*! \remarks This method is used internally.\n\n
	This method returns a pointer to the IMenu client associated with the
	timer. */
	virtual IMenu* GetIMenu() const = 0;
	/*! \remarks This method is used internally.\n\n
	This method returns the timing type for the timer. */
	virtual EventParam GetTimingType() const = 0;
};


typedef unsigned int ValidityToken;


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class MenuColors: public MaxHeapOperators

//  PURPOSE:
//    class declaration for a menu's color settings
//
//  NOTES:
//    created:  08.28.00 - mjm
//
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

{
public:
	MenuColors() { ResetDefaults(); }

	// sets values to defaults
	void ResetDefaults()
	{
		mTitleBarBackgroundColor = Color(.0f, .0f, .0f);
		mTitleBarTextColor = Color(.75f, .75f, .75f);
		mItemBackgroundColor = Color(.75f, .75f, .75f);
		mItemTextColor = Color(.0f, .0f, .0f);
		mLastExecutedItemTextColor = Color(.95f, .85f, .0f);
		mHighlightedItemBackgroundColor = Color(.95f, .85f, .0f);
		mHighlightedItemTextColor = Color(.0f, .0f, .0f);
		mBorderColor = Color(.0f, .0f, .0f);
		mDisabledShadowColor = Color(.5f, .5f, .5f);
		mDisabledHighlightColor = Color(1.0f, 1.0f, 1.0f);
	}

	Color mTitleBarBackgroundColor,
		  mTitleBarTextColor,
		  mItemBackgroundColor,
		  mItemTextColor,
		  mLastExecutedItemTextColor,
		  mHighlightedItemBackgroundColor,
		  mHighlightedItemTextColor,
		  mBorderColor,
		  mDisabledShadowColor,
		  mDisabledHighlightColor;
};

inline COLORREF MakeCOLORREF(const Color& c) { return RGB( FLto255(c.r), FLto255(c.g), FLto255(c.b) ); }

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/*! \sa  Class IMenuGlobalContext\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This abstract class represents an interface for all general menu settings.
Methods that are marked as internal should not be used.  */
class IMenuSettings: public MaxHeapOperators

//  PURPOSE:
//    Abstract class (Interface) for general menu settings
//
//  NOTES:
//    created:  02.17.00 - mjm
//
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

{
public:
	/*! \remarks Destructor. */
	virtual ~IMenuSettings() {;}
	// to determine validity of settings
	/*! \remarks This method is used internally.\n\n
	This method checks if a token is valid.
	\par Parameters:
	<b>ValidityToken\& token</b>\n\n
	A reference to a token for which to check its validity.
	\return  TRUE if the token is valid, otherwise FALSE. */
	virtual bool IsTokenValid(const ValidityToken& token) = 0;
	/*! \remarks This method is used internally.\n\n
	This method updates the validity token.
	\par Parameters:
	<b>ValidityToken\& token</b>\n\n
	A reference to a token to update. */
	virtual void UpdateValidityToken(ValidityToken& token) const = 0;

	// sets values to defaults
	/*! \remarks This method will reset the menu settings to their defaults.
	*/
	virtual void ResetDefaults() = 0;

	// sets and gets the border size
	/*! \remarks This method allows you to set the menu border size.
	\par Parameters:
	<b>int borderSz</b>\n\n
	The border size in pixels. */
	virtual void SetBorderSz(int borderSz) = 0;
	/*! \remarks This method returns the menu border size. */
	virtual int GetBorderSz() const = 0;

	// sets and gets the horizontal margin size (in points)
	/*! \remarks This method allows you to set the menu's horizontal margin
	size.
	\par Parameters:
	<b>int horizontalMarginInPoints</b>\n\n
	The horizontal margin size in points. */
	virtual void SetHorizontalMarginInPoints(int horizontalMarginInPoints) = 0;
	/*! \remarks This method returns the menu's horizontal margin size (in
	points). */
	virtual int GetHorizontalMarginInPoints() const = 0;

	// sets and gets the vertical margin size (in points)
	/*! \remarks This method allows you to set the menu's vertical margin
	size.
	\par Parameters:
	<b>int verticalMarginInPoints</b>\n\n
	The vertical margin size in points. */
	virtual void SetVerticalMarginInPoints(int verticalMarginInPoints) = 0;
	/*! \remarks This method returns the menu's vertical margin size (in
	points). */
	virtual int GetVerticalMarginInPoints() const = 0;

	// gets the margins in pixels
	/*! \remarks This method returns the menu's horizontal margin, in pixels.
	\par Parameters:
	<b>HDC hDC</b>\n\n
	A handle to a device context. */
	virtual int GetHorizontalMargin(HDC hDC) const = 0;
	/*! \remarks This method returns the menu's vertical margin, in
	pixels.\n\n

	\par Parameters:
	<b>HDC hDC</b>\n\n
	A handle to a device context. */
	virtual int GetVerticalMargin(HDC hDC) const = 0;

	// sets and gets the item font face
	/*! \remarks This method allows you to set the menu item's font typeface.
	\par Parameters:
	<b>MCHAR* szItemFontFace</b>\n\n
	A string containing the typeface name. */
	virtual void SetItemFontFace(MCHAR* szItemFontFace) = 0;
	/*! \remarks This method returns the name of the menu item's font
	typeface. */
	virtual const MCHAR* GetItemFontFace() const = 0;

	// sets and gets the title font face
	/*! \remarks This method allows you to set the menu title's font typeface.
	\par Parameters:
	<b>MCHAR* szTitleFontFace</b>\n\n
	A string containing the typeface name. */
	virtual void SetTitleFontFace(MCHAR* szTitleFontFace) = 0;
	/*! \remarks This method returns the name of the menu title's font
	typeface. */
	virtual const MCHAR* GetTitleFontFace() const = 0;

	// sets and gets the item font size
	/*! \remarks This method allows you to set the menu item's font size.
	\par Parameters:
	<b>int itemFontSize</b>\n\n
	The size of the font, in points. */
	virtual void SetItemFontSize(int itemFontSize) = 0;
	/*! \remarks This method returns the menu item's font size, in points. */
	virtual int GetItemFontSize() const = 0;

	// sets and gets the title font size
	/*! \remarks This method allows you to set the menu title's font size.
	\par Parameters:
	<b>int titleFontSize</b>\n\n
	The size of the font, in points. */
	virtual void SetTitleFontSize(int titleFontSize) = 0;
	/*! \remarks This method returns the menu title's font size, in points. */
	virtual int GetTitleFontSize() const = 0;

	// sets and gets whether menu item's have uniform height
	/*! \remarks This method allows you to set the status of a menu item's
	uniform height flag.
	\par Parameters:
	<b>bool useUniformItemHeight</b>\n\n
	TRUE to set the uniform height flag ON, FALSE to set it to OFF. */
	virtual void SetUseUniformItemHeight(bool useUniformItemHeight) = 0;
	/*! \remarks This method returns TRUE or FALSE if the menu item's uniform
	height flag is set or not set, respectively. */
	virtual bool GetUseUniformItemHeight() const = 0;
	// these overrides are provided for the function publishing system
	/*! \remarks This method allows you to set the status of a menu item's
	uniform height flag. This version of <b>SetUniformItemHeight()</b> is
	provided for the function publishing system.
	\par Parameters:
	<b>BOOL useUniformItemHeight</b>\n\n
	TRUE to set the uniform height flag ON, FALSE to set it to OFF. */
	virtual void SetUseUniformItemHeightBOOL(BOOL useUniformItemHeight) = 0;
	/*! \remarks This method returns TRUE or FALSE if the menu item's uniform
	height flag is set or not set, respectively. This version of
	<b>GetUniformItemHeight()</b> is provided for the function publishing
	system. */
	virtual BOOL GetUseUniformItemHeightBOOL() const = 0;

	// sets and gets the opacity - 0 to 1
	/*! \remarks This method allows you to set the menu's opacity value.
	\par Parameters:
	<b>float opacity</b>\n\n
	The opacity value, ranging from 0.0 - 1.0. */
	virtual void SetOpacity(float opacity) = 0;
	/*! \remarks This method returns the menu's opacity value. */
	virtual float GetOpacity() const = 0;

	// sets and gets the display method
	/*! \remarks This method allows you to set a menu's display method.
	\par Parameters:
	<b>DisplayMethod displayMethod</b>\n\n
	The display method (enum), which is either of the following; <b>DM_NORMAL,
	DM_STRETCH, DM_FADE, DM_NUM_METHODS</b> */
	virtual void SetDisplayMethod(DisplayMethod displayMethod) = 0;
	/*! \remarks This method returns the menu's display method, which is
	either of the following; <b>DM_NORMAL, DM_STRETCH, DM_FADE,
	DM_NUM_METHODS</b> */
	virtual DisplayMethod GetDisplayMethod() const = 0;

	// sets and gets the number of animation steps
	/*! \remarks This method allows you to set the menu's number of animated
	steps for the 'growing' effect.
	\par Parameters:
	<b>unsigned int steps</b>\n\n
	The number of steps. */
	virtual void SetAnimatedSteps(unsigned int steps) = 0;
	/*! \remarks This method returns the menu's number of animated steps used
	for the 'growing' effect. */
	virtual unsigned int GetAnimatedSteps() const = 0;

	// sets and gets the duration of an animation step (milliseconds)
	/*! \remarks This method allows you to set the menu's animated step time.
	\par Parameters:
	<b>unsigned int ms</b>\n\n
	The animated step time, in milliseconds. */
	virtual void SetAnimatedStepTime(unsigned int ms) = 0;
	/*! \remarks This method returns the menu's animated step time, in
	milliseconds. */
	virtual unsigned int GetAnimatedStepTime() const = 0;

	// sets and gets the delay before a submenu is displayed (milliseconds)
	/*! \remarks This method allows you to set the delay before a submenu is
	displayed.
	\par Parameters:
	<b>unsigned int ms</b>\n\n
	The delay, in milliseconds. */
	virtual void SetSubMenuPauseTime(unsigned int ms) = 0;
	/*! \remarks This method returns the delay before a submenu is displayed,
	in milliseconds. */
	virtual unsigned int GetSubMenuPauseTime() const = 0;

	// sets and gets whether to use the menu's last executed item (when user clicks in title bar)
	/*! \remarks This method allows you to set the "last executed item" flag
	which determines whether to use the menu's last executed item when the user
	clicks on the menu's titlebar.
	\par Parameters:
	<b>bool useLastExecutedItem</b>\n\n
	TRUE to turn ON the flag, FALSE to turn the flag off. */
	virtual void SetUseLastExecutedItem(bool useLastExecutedItem) = 0;
	/*! \remarks This method returns whether the "last executed item" flag is
	set (TRUE) or not set (FALSE). The flag determines whether to use the
	menu's last executed item when the user clicks on the menu's titlebar. */
	virtual bool GetUseLastExecutedItem() const = 0;
	// these overrides are provided for the function publishing system
	/*! \remarks This method allows you to set the "last executed item" flag
	which determines whether to use the menu's last executed item when the user
	clicks on the menu's titlebar. This version of
	<b>SetUseLastExecutedItem()</b> is provided for the function publishing
	system.\n\n

	\par Parameters:
	<b>BOOL useLastExecutedItem</b>\n\n
	TRUE to turn ON the flag, FALSE to turn the flag off. */
	virtual void SetUseLastExecutedItemBOOL(BOOL useLastExecutedItem) = 0;
	/*! \remarks This method returns whether the "last executed item" flag is
	set (TRUE) or not set (FALSE). The flag determines whether to use the
	menu's last executed item when the user clicks on the menu's titlebar. This
	version of <b>GetUseLastExecutedItem()</b> is provided for the function
	publishing system. */
	virtual BOOL GetUseLastExecutedItemBOOL() const = 0;

	// sets and gets whether the menu is repositioned when near the edge of the screen
	/*! \remarks This method allows you to set the flag which controls and
	determines whether the menu is repositioned when near the edge of the
	screen.
	\par Parameters:
	<b>bool repositionWhenClipped</b>\n\n
	TRUE to turn repositioning ON, FALSE to turn it OFF. */
	virtual void SetRepositionWhenClipped(bool repositionWhenClipped) = 0;
	/*! \remarks This method returns the status of the flag which controls and
	determines whether the menu is repositioned when near the edge of the
	screen.
	\return  TRUE if the flag is ON, otherwise FALSE. */
	virtual bool GetRepositionWhenClipped() const = 0;
	// these overrides are provided for the function publishing system
	/*! \remarks This method allows you to set the flag which controls and
	determines whether the menu is repositioned when near the edge of the
	screen. This version of <b>SetRepositionWhenClipped()</b> is provided for
	the function publishing system.
	\par Parameters:
	<b>BOOL repositionWhenClipped</b>\n\n
	TRUE to turn repositioning ON, FALSE to turn it OFF. */
	virtual void SetRepositionWhenClippedBOOL(BOOL repositionWhenClipped) = 0;
	/*! \remarks This method returns the status of the flag which controls and
	determines whether the menu is repositioned when near the edge of the
	screen. This version of <b>GetRepositionWhenClipped()</b> is provided for
	the function publishing system.
	\return  TRUE if the flag is ON, otherwise FALSE. */
	virtual BOOL GetRepositionWhenClippedBOOL() const = 0;

	// sets and gets whether the menu should remove redundant separators
	/*! \remarks This method allows you to set the flag which controls and
	determines whether the menu should remove redundant separators.
	\par Parameters:
	<b>bool removeRedundantSeparators</b>\n\n
	TRUE to turn the flag ON, FALSE to turn it OFF. */
	virtual void SetRemoveRedundantSeparators(bool removeRedundantSeparators) = 0;
	/*! \remarks This method returns the status of the flag which controls and
	determines whether the menu should remove redundant separators.
	\return  TRUE if the flag is ON, otherwise FALSE. */
	virtual bool GetRemoveRedundantSeparators() const = 0;
	// these overrides are provided for the function publishing system
	/*! \remarks This method allows you to set the flag which controls and
	determines whether the menu should remove redundant separators. This
	version of <b>SetRemoveRedundantSeparators()</b> is provided for the
	function publishing system.
	\par Parameters:
	<b>BOOL removeRedundantSeparators</b>\n\n
	TRUE to turn the flag ON, FALSE to turn it OFF. */
	virtual void SetRemoveRedundantSeparatorsBOOL(BOOL removeRedundantSeparators) = 0;
	/*! \remarks This method returns the status of the flag which controls and
	determines whether the menu should remove redundant separators. This
	version of <b>GetRemoveRedundantSeparators()</b> is provided for the
	function publishing system.
	\return  TRUE if the flag is ON, otherwise FALSE. */
	virtual BOOL GetRemoveRedundantSeparatorsBOOL() const = 0;
};


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/*! \sa  Class ImenuItem, Class IMenuSettings,  Class IMenuTimer, Class ItemID, Class IPoint2\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This abstract class represents an global context interface for all menus that
might be displayed during a user's menuing action and is used internally.
Methods that are marked as internal should not be used.  */
class IMenuGlobalContext: public MaxHeapOperators

//  PURPOSE:
//    Abstract class (Interface) for context global to all menus that might be
//    displayed during a user's menuing action
//
//  NOTES:
//    created:  02.17.00 - mjm
//
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

{
public:
	/*! \remarks Destructor. */
	virtual ~IMenuGlobalContext() {}
	// sets and gets the menu settings
	/*! \remarks This method is used internally.\n\n
	This method allows you to set the menu settings associated with this global
	context.
	\par Parameters:
	<b>IMenuSettings* pIMenuSettings</b>\n\n
	A pointer to a menu settings object. */
	virtual void SetIMenuSettings(IMenuSettings* pIMenuSettings) = 0;
	/*! \remarks This method is used internally.\n\n
	This method returns a pointer to the menu settings associated with this
	global context. */
	virtual IMenuSettings* GetIMenuSettings() const = 0;

	// updates cursor position from win32
	/*! \remarks This method is used internally.\n\n
	This method updates the cursor position from Win32. */
	virtual void UpdateCursorPosition() = 0;

	// gets the cached cursor position
	/*! \remarks This method is used internally.\n\n
	This method returns the cached cursor position.
	\return  The x and y coordinates of the cursor. */
	virtual const IPoint2& GetCursorPosition() const = 0;

	// sets and gets the cached initial cursor position. This is where the user clicked
	/*! \remarks This method is used internally.\n\n
	This method returns the cached initial cursor position (i.e. the coordinate
	where the user clicked).
	\return  The x and y coordinates of the cached initial cursor position. */
	virtual const IPoint2& GetInitialCursorPosition() const = 0;
	/*! \remarks This method is used internally.\n\n
	This method allows you to set the cached initial cursor position (i.e. the
	coordinate where the user clicked).
	\par Parameters:
	<b>IPoint2\& initPos</b>\n\n
	The x and y coordinate of the cursor position. */
	virtual void SetInitialCursorPosition(IPoint2& initPos) = 0;

	// sets and gets the global timer
	/*! \remarks This method is used internally.\n\n
	This method allows you to set a global timer for the menu's global context.
	\par Parameters:
	<b>IMenuTimer* pIMenuTimer</b>\n\n
	A pointer to the menu timer object, */
	virtual void SetIMenuTimer(IMenuTimer* pIMenuTimer) = 0;
	/*! \remarks This method is used internally.\n\n
	This method returns a pointer to the global timer for the menu's global
	context. */
	virtual IMenuTimer* GetIMenuTimer() const = 0;

	// sets and gets the handle to the display window
	/*! \remarks This method is used internally.\n\n
	This method allows you to set the handle to the display window.
	\par Parameters:
	<b>HWND hDisplayWnd</b>\n\n
	The handle to the display window. */
	virtual void SetHDisplayWnd(HWND hDisplayWnd) = 0;
	/*! \remarks This method is used internally.\n\n
	This method returns the handle to the display window. */
	virtual HWND GetHDisplayWnd() const = 0;

	// sets and gets the handle to the messsage window
	/*! \remarks This method is used internally.\n\n
	This method allows you to set the handle to the message window
	\par Parameters:
	<b>HWND hDisplayWnd</b>\n\n
	The handle to the message window. */
	virtual void SetHMessageWnd(HWND hDisplayWnd) = 0;
	/*! \remarks This method is used internally.\n\n
	This method returns the handle to the message window. */
	virtual HWND GetHMessageWnd() const = 0;

	// sets and gets the handle to the display device context
	/*! \remarks This method is used internally.\n\n
	This method allows you to set a handle to the display device context.
	\par Parameters:
	<b>HDC hDisplayDC</b>\n\n
	The handle to the display device context. */
	virtual void SetHDisplayDC(HDC hDisplayDC) = 0;
	/*! \remarks This method is used internally.\n\n
	This method returns the handle to the display device context. */
	virtual HDC GetHDisplayDC() const = 0;

	// sets and gets the handle to the title font
	/*! \remarks This method is used internally.\n\n
	This method allows you to set the handle to the title font.
	\par Parameters:
	<b>HFONT hTitleFont</b>\n\n
	The handle to the title font. */
	virtual void SetTitleHFont(HFONT hTitleFont) = 0;
	/*! \remarks This method is used internally.\n\n
	This method returns the handle to the title font. */
	virtual HFONT GetTitleHFont() const = 0;

	// sets and gets the handle to the item font
	/*! \remarks This method is used internally.\n\n
	This method allows you to set the handle to the item font.
	\par Parameters:
	<b>HFONT hItemFont</b>\n\n
	The handle to the item font. */
	virtual void SetItemHFont(HFONT hItemFont) = 0;
	/*! \remarks This method is used internally.\n\n
	This method returns the handle to the item font. */
	virtual HFONT GetItemHFont() const = 0;

	// sets and gets the handle to the accelerator font
	/*! \remarks This method is used internally.\n\n
	This method allows you to set the handle to the accelerator font.
	\par Parameters:
	<b>HFONT hItemFont</b>\n\n
	The handle to the accelerator font. */
	virtual void SetAcceleratorHFont(HFONT hItemFont) = 0;
	/*! \remarks This method is used internally.\n\n
	This method returns the handle to the accelerator font. */
	virtual HFONT GetAcceleratorHFont() const = 0;

	// sets and gets the menu's maximum item size
	/*! \remarks This method is used internally.\n\n
	This method allows you to set the menu's maximum uniform item size.
	\par Parameters:
	<b>IPoint2\& itemSize</b>\n\n
	The size rectangle. */
	virtual void SetUniformItemSize(const IPoint2& itemSize) = 0;
	/*! \remarks This method is used internally.\n\n
	This method returns the menu's maximum uniform item size as a rectangle. */
	virtual const IPoint2& GetUniformItemSize() const = 0;

	// gets the height of a title bar, not counting the border
	/*! \remarks This method is used internally.\n\n
	This method returns the height of the title bar, not counting the border.
	*/
	virtual int GetTitleBarHeight() = 0;

	// gets the ItemID of the menu/item triplet currently being traversed
	/*! \remarks This method is used internally.\n\n
	This method returns the ItemID of the menu/item triplet that's currently
	being traversed. */
	virtual ItemID& GetCurrentItemID() = 0;

	// gets the ItemID of the menu/item triplet currently selected
	/*! \remarks This method is used internally.\n\n
	This method returns the ItemID of the menu/item triplet that's currently
	selected. */
	virtual ItemID& GetSelectionItemID() = 0;

	// convenience functions to determine selection is available
	/*! \remarks This method is used internally.\n\n
	This method will determine selection is available and will return TRUE if
	selection is available or FALSE if it is not. */
	virtual bool HasSelection() = 0;

	// convenience functions to determine selection status
	/*! \remarks This method is used internally.\n\n
	This method will determine the selection status of the current menu and
	will return TRUE if the current menu is selected or FALSE if it is not. */
	virtual bool IsCurrentMenuSelected() = 0;  // current menu is selected
	/*! \remarks This method is used internally.\n\n
	This method will determine the selection status of the current menu and
	item and will return TRUE if the current menu and item are selected or
	FALSE if they are not. */
	virtual bool IsCurrentItemSelected() = 0;  // current menu and item are selected

	// convenience function to select current menu item
	/*! \remarks This method is used internally.\n\n
	This method selects the current item. */
	virtual void SelectCurrentItem() = 0;
};


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/*! \sa  Class IMenuItem, Class IMenuColors, Class IPoint2\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This abstract class represents a local context interface for a specific menu
and is used internally. Methods that are marked as internal should not be used.
 */
class IMenuLocalContext: public MaxHeapOperators

//  PURPOSE:
//    Abstract class (Interface) for context local to a specific menu
//
//  NOTES:
//    created:  02.17.00 - mjm
//
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

{
public:
	/*! \remarks Destructor. */
	virtual ~IMenuLocalContext() {;}
	// sets and gets the handle to the drawing device context
	/*! \remarks This method is used internally.\n\n
	This method allows you to set the handle to the drawing device context.
	\par Parameters:
	<b>HDC hDrawDC</b>\n\n
	The handle to the drawing device context. */
	virtual void SetHDrawDC(HDC hDrawDC) = 0;
	/*! \remarks This method is used internally.\n\n
	This method returns the handle to the drawing device context. */
	virtual HDC GetHDrawDC() const = 0;

	// sets and gets the cursor position in the local coordinates
	/*! \remarks This method is used internally.\n\n
	This method allows you to set the cursor position in the local coordinate
	system of the menu.
	\par Parameters:
	<b>const IPoint2\& localCursorPos</b>\n\n
	The cursor position coordinates. */
	virtual void SetLocalCursorPosition(const IPoint2& localCursorPos) = 0;
	/*! \remarks This method is used internally.\n\n
	This method returns the cursor position in the local coordinate system of
	the menu. */
	virtual const IPoint2& GetLocalCursorPosition() const = 0;

	// sets and gets the menu's current width
	/*! \remarks This method is used internally.\n\n
	This method allows you to set the menu's current width.
	\par Parameters:
	<b>int menuWidth</b>\n\n
	The menu width. */
	virtual void SetMenuItemWidth(int menuWidth) = 0;
	/*! \remarks This method is used internally.\n\n
	This method returns the menu's current width. */
	virtual int GetMenuItemWidth() const = 0;

	// sets and gets the menu's current level (submenus have level > 0)
	/*! \remarks This method is used internally.\n\n
	This method allows you to set the menu's current level. Submenus are
	indexed by a level \> 0.
	\par Parameters:
	<b>int level</b>\n\n
	The current level to set. */
	virtual void SetLevel(int level) = 0;
	/*! \remarks This method is used internally.\n\n
	This method returns the menu's current level. */
	virtual int GetLevel() const = 0;

	// sets and gets the menu's last executed item path (a tab of IMenuItems, listing the selected item at each menu level)
	/*! \remarks This method is used internally.\n\n
	This method allows you to set the menu's last executed item path. The item
	path is a table of IMenuItem entries, listing the selected item at each
	menu level.
	\par Parameters:
	<b>Tab\<IMenuItem *\> *pExecutedItemPath</b>\n\n
	A pointer to the item path. */
	virtual void SetLastExecutedItemPath(Tab<IMenuItem *> *pExecutedItemPath) = 0;
	/*! \remarks This method is used internally.\n\n
	This method returns the menu's last executed item path. The item path is a
	table of IMenuItem entries, listing the selected item at each menu level.
	*/
	virtual Tab<IMenuItem *> *GetLastExecutedItemPath() = 0;

	// sets and gets the menu's current colors
	/*! \remarks This method is used internally.\n\n
	This method allows you to set the menu's current colors.
	\par Parameters:
	<b>MenuColors *pMenuColors</b>\n\n
	A pointer to the menu colors. */
	virtual void SetMenuColors(const MenuColors *pMenuColors) = 0;
	/*! \remarks This method is used internally.\n\n
	This method returns a pointer to the menu's current colors. */
	virtual const MenuColors *GetMenuColors() const = 0;

	// sets and gets the global menu context
	/*! \remarks This method is used internally.\n\n
	This method allows you to set the global menu context.
	\par Parameters:
	<b>IMenuGlobalContext* pIMenuGlobalContext</b>\n\n
	A pointer to the menu's global context object.\n\n
	<b>int level</b>\n\n
	The menu level.\n\n
	<b>Tab\<IMenuItem *\> *pExecutedItemPath</b>\n\n
	A pointer to the item path. The item path is a table of IMenuItem entries,
	listing the selected item at each menu level.\n\n
	<b>MenuColors *pMenuColors</b>\n\n
	A pointer to the menu colors. */
	virtual void SetIMenuGlobalContext(IMenuGlobalContext* pIMenuGlobalContext, int level, Tab<IMenuItem *> *pExecutedItemPath, const MenuColors *pMenuColors) = 0;
	/*! \remarks This method is used internally.\n\n
	This method returns a pointer to the menu's global context object. */
	virtual IMenuGlobalContext* GetIMenuGlobalContext() const = 0;
};


#include "iFnPub.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/*! \sa  Class IMenuItem,  Class ImenuSettings,  Class MenuColors\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This abstract class represents an interface for quad menu settings. The methods
contained in this class allow you to access and control all quad menu related
settings and configuration parameters.  */
class IQuadMenuSettings : public IMenuSettings, public FPStaticInterface

//  PURPOSE:
//    Abstract class (Interface) for quad menu settings
//
//  NOTES:
//    created:  02.17.00 - mjm
//
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

{
public:
// function IDs 
	enum
	{
		// from IMenuSettings
		fnIdResetDefaults,
		fnIdSetBorderSize,
		fnIdGetBorderSize,
		fnIdSetHorizontalMarginInPoints,
		fnIdGetHorizontalMarginInPoints,
		fnIdSetVerticalMarginInPoints,
		fnIdGetVerticalMarginInPoints,
		fnIdSetItemFontFace,
		fnIdGetItemFontFace,
		fnIdSetTitleFontFace,
		fnIdGetTitleFontFace,
		fnIdSetItemFontSize,
		fnIdGetItemFontSize,
		fnIdSetTitleFontSize,
		fnIdGetTitleFontSize,
		fnIdSetUseUniformItemHeight,
		fnIdGetUseUniformItemHeight,
		fnIdSetOpacity,
		fnIdGetOpacity,
		fnIdSetDisplayMethod,
		fnIdGetDisplayMethod,
		fnIdSetAnimatedSteps,
		fnIdGetAnimatedSteps,
		fnIdSetAnimatedStepTime,
		fnIdGetAnimatedStepTime,
		fnIdSetSubMenuPauseTime,
		fnIdGetSubMenuPauseTime,
		fnIdSetUseLastExecutedItem,
		fnIdGetUseLastExecutedItem,
		fnIdSetRepositionWhenClipped,
		fnIdGetRepositionWhenClipped,
		fnIdSetRemoveRedundantSeparators,
		fnIdGetRemoveRedundantSeparators,

		// from IQuadMenuSettings
		fnIdSetFirstQuadDisplayed,
		fnIdGetFirstQuadDisplayed,
		fnIdSetUseUniformQuadWidth,
		fnIdGetUseUniformQuadWidth,
		fnIdSetMirrorQuad,
		fnIdGetMirrorQuad,
		fnIdSetMoveCursorOnReposition,
		fnIdGetMoveCursorOnReposition,
		fnIdSetReturnCursorAfterReposition,
		fnIdGetReturnCursorAfterReposition,
		fnIdSetInitialCursorLocInBox_0to1,
		fnIdGetInitialCursorLocXInBox_0to1,
		fnIdGetInitialCursorLocYInBox_0to1,

		fnIdSetTitleBarBackgroundColor,
		fnIdGetTitleBarBackgroundColor,
		fnIdSetTitleBarTextColor,
		fnIdGetTitleBarTextColor,
		fnIdSetItemBackgroundColor,
		fnIdGetItemBackgroundColor,
		fnIdSetItemTextColor,
		fnIdGetItemTextColor,
		fnIdSetLastExecutedItemTextColor,
		fnIdGetLastExecutedItemTextColor,
		fnIdSetHighlightedItemBackgroundColor,
		fnIdGetHighlightedItemBackgroundColor,
		fnIdSetHighlightedItemTextColor,
		fnIdGetHighlightedItemTextColor,
		fnIdSetBorderColor,
		fnIdGetBorderColor,
		fnIdSetDisabledShadowColor,
		fnIdGetDisabledShadowColor,
		fnIdSetDisabledHighlightColor,
		fnIdGetDisabledHighlightColor,
#if defined(USE_NEW_CUI_IO_METHODS) && !defined(NO_CUI)	// russom - 02/15/02
		fnIdSaveSettingsFile,
		fnIdLoadSettingsFile,
#endif
};

	// sets and gets the first quadrant displayed
	/*! \remarks This method allows you to set the first quad which will be
	displayed when a quad menu pops up.
	\par Parameters:
	<b>QuadIndex firstQuadDisplayed</b>\n\n
	The quad index, one of the following; <b>QUAD_ONE</b>, <b>QUAD_TWO</b>,
	<b>QUAD_THREE</b>, or <b>QUAD_FOUR</b>. */
	virtual void SetFirstQuadDisplayed(QuadIndex firstQuadDisplayed) = 0;
	/*! \remarks This method returns the index of the first quad which will be
	displayed.
	\return  The quad index, one of the following; <b>QUAD_ONE</b>,
	<b>QUAD_TWO</b>, <b>QUAD_THREE</b>, or <b>QUAD_FOUR</b>. */
	virtual QuadIndex GetFirstQuadDisplayed() const = 0;

	// sets and gets whether the quadrants have uniform width
	/*! \remarks This method allows you to set whether the quad menu has a
	uniform width.
	\par Parameters:
	<b>bool useUniformQuadWidth</b>\n\n
	TRUE to set the uniform width, FALSE to set it to non-uniform. */
	virtual void SetUseUniformQuadWidth(bool useUniformQuadWidth) = 0;
	/*! \remarks This method returns the status of the uniform width flag for
	the quad menu. TRUE if the quad menu has been set to use uniform width,
	otherwise FALSE. */
	virtual bool GetUseUniformQuadWidth() const = 0;
	// these overrides are provided for the function publishing system
	/*! \remarks This method allows you to set whether the quad menu has a
	uniform width. This version of <b>SetUseUniformQuadWidth()</b> is provided
	for the function publishing system.
	\par Parameters:
	<b>BOOL useUniformQuadWidth</b>\n\n
	TRUE to set the uniform width, FALSE to set it to non-uniform. */
	virtual void SetUseUniformQuadWidthBOOL(BOOL useUniformQuadWidth) = 0;
	/*! \remarks This method returns the status of the uniform width flag for
	the quad menu. TRUE if the quad menu has been set to use uniform width,
	otherwise FALSE. This version of <b>GetUseUniformQuadWidth()</b> is
	provided for the function publishing system. */
	virtual BOOL GetUseUniformQuadWidthBOOL() const = 0;

	// sets and gets whether the quad menus are mirrored (left - right)
	/*! \remarks This method allows you to set whether the quad menus are
	mirrored left to right.
	\par Parameters:
	<b>bool mirrorQuad</b>\n\n
	TRUE to mirror the menus, otherwise FALSE. */
	virtual void SetMirrorQuad(bool mirrorQuad) = 0;
	/*! \remarks This method returns TRUE if the quad menu is mirrored left to
	right, otherwise FALSE. */
	virtual bool GetMirrorQuad() const = 0;
	// these overrides are provided for the function publishing system
	/*! \remarks This method allows you to set whether the quad menus are
	mirrored left to right. This version of <b>SetMirrorQuad()</b> is provided
	for the function publishing system.
	\par Parameters:
	<b>BOOL mirrorQuad</b>\n\n
	TRUE to mirror the menus, otherwise FALSE. */
	virtual void SetMirrorQuadBOOL(BOOL mirrorQuad) = 0;
	/*! \remarks This method returns TRUE if the quad menu is mirrored left to
	right, otherwise FALSE. This version of <b>GetMirrorQuad()</b> is provided
	for the function publishing system. */
	virtual BOOL GetMirrorQuadBOOL() const = 0;

	// sets and gets whether the cursor moves when the quad menu is repositioned because of clipping the edge of the screen
	/*! \remarks This method allows you to set whether the cursor moves when
	the quad menu is repositioned because of clipping the edge of the screen.
	\par Parameters:
	<b>bool moveCursorOnReposition</b>\n\n
	TRUE to move the cursor, otherwise FALSE. */
	virtual void SetMoveCursorOnReposition(bool moveCursorOnReposition) = 0;
	/*! \remarks This method returns TRUE if the cursor moves when the quad
	menu is repositioned because of clipping the edge of the screen, otherwise
	FALSE. */
	virtual bool GetMoveCursorOnReposition() const = 0;
	// these overrides are provided for the function publishing system
	/*! \remarks This method allows you to set whether the cursor moves when
	the quad menu is repositioned because of clipping the edge of the screen.
	This version of <b>SetMoveCursorOnReposition()</b> is provided for the
	function publishing system.
	\par Parameters:
	<b>BOOL moveCursorOnReposition</b>\n\n
	TRUE to move the cursor, otherwise FALSE. */
	virtual void SetMoveCursorOnRepositionBOOL(BOOL moveCursorOnReposition) = 0;
	/*! \remarks This method returns TRUE if the cursor moves when the quad
	menu is repositioned because of clipping the edge of the screen, otherwise
	FALSE. This version of <b>GetMoveCursorOnReposition()</b> is provided for
	the function publishing system. */
	virtual BOOL GetMoveCursorOnRepositionBOOL() const = 0;

	// sets and gets whether the cursor is moved the opposite distance that it was automatically moved when the quad menu is repositioned because of clipping the edge of the screen
	/*! \remarks This method allows you to set whether the cursor is moved the
	opposite distance that it was automatically moved when the quad menu is
	repositioned because of clipping the edge of the screen.
	\par Parameters:
	<b>bool returnCursorAfterReposition</b>\n\n
	TRUE to set the flag, otherwise FALSE. */
	virtual void SetReturnCursorAfterReposition(bool returnCursorAfterReposition) = 0;
	/*! \remarks This method returns TRUE if the cursor is moved the opposite
	distance that it was automatically moved when the quad menu is repositioned
	because of clipping the edge of the screen, otherwise FALSE. */
	virtual bool GetReturnCursorAfterReposition() const = 0;
	// these overrides are provided for the function publishing system
	/*! \remarks This method allows you to set whether the cursor is moved the
	opposite distance that it was automatically moved when the quad menu is
	repositioned because of clipping the edge of the screen. This version of
	<b>GetReturnCursorAfterReposition()</b> is provided for the function
	publishing system.
	\par Parameters:
	<b>BOOL returnCursorAfterReposition</b>\n\n
	TRUE to set the flag, otherwise FALSE. */
	virtual void SetReturnCursorAfterRepositionBOOL(BOOL returnCursorAfterReposition) = 0;
	/*! \remarks This method returns TRUE if the cursor is moved the opposite
	distance that it was automatically moved when the quad menu is repositioned
	because of clipping the edge of the screen, otherwise FALSE. This version
	of <b>GetReturnCursorAfterReposition()</b> is provided for the function
	publishing system. */
	virtual BOOL GetReturnCursorAfterRepositionBOOL() const = 0;

	// sets and gets the initial location of the cursor in the center box - as a ratio (0 to 1) of box size
	/*! \remarks This method allows you to set the initial location of the
	cursor in the center quad box.
	\par Parameters:
	<b>float x, float y</b>\n\n
	The location of the cursor, as a ratio of the box size, between 0.0 and
	1.0. */
	virtual void SetCursorLocInBox_0to1(float x, float y) = 0;
	/*! \remarks This method returns the initial x location of the cursor in
	the center quad box, as a ratio of the box size, between 0.0 and 1.0. */
	virtual float GetCursorLocXInBox_0to1() const = 0;
	/*! \remarks This method returns the initial y location of the cursor in
	the center quad box, as a ratio of the box size, between 0.0 and 1.0. */
	virtual float GetCursorLocYInBox_0to1() const = 0;


	// gets the color array for a specific quad (numbered 1 through 4)
	/*! \remarks This method returns the color array for a specific quad.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad to obtain the color array for, (numbered 1 through 4). */
	virtual const MenuColors *GetMenuColors(int quadNum) const = 0;

	// sets and gets the title bar background color for a specific quad (numbered 1 through 4)
	/*! \remarks This method allows you to set the title bar background color
	for a specific quad.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4).\n\n
	<b>Color\& color</b>\n\n
	The color to set. */
	virtual void SetTitleBarBackgroundColor(int quadNum, const Color& color) = 0;
	/*! \remarks This method returns the title bar background color of a
	specific quad. This method returns the color as a <b>Color</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual const Color& GetTitleBarBackgroundColor(int quadNum) const = 0;
	/*! \remarks This method returns the title bar background color of a
	specific quad. This method returns the color as a <b>COLORREF</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual COLORREF GetTitleBarBackgroundColorRef(int quadNum) const = 0;

	// sets and gets the title bar text color for a specific quad (numbered 1 through 4)
	/*! \remarks This method allows you to set the title bar text color for a
	specific quad.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4).\n\n
	<b>Color\& color</b>\n\n
	The color to set. */
	virtual void SetTitleBarTextColor(int quadNum, const Color& color) = 0;
	/*! \remarks This method returns the title bar text color of a specific
	quad. This method returns the color as a <b>Color</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual const Color& GetTitleBarTextColor(int quadNum) const = 0;
	/*! \remarks This method returns the title bar text color of a specific
	quad. This method returns the color as a <b>COLORREF</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual COLORREF GetTitleBarTextColorRef(int quadNum) const = 0;

	// sets and gets the item background color for a specific quad (numbered 1 through 4)
	/*! \remarks This method allows you to set the item background color for a
	specific quad.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4).\n\n
	<b>Color\& color</b>\n\n
	The color to set. */
	virtual void SetItemBackgroundColor(int quadNum, const Color& color) = 0;
	/*! \remarks This method returns the item background color of a specific
	quad. This method returns the color as a <b>Color</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual const Color& GetItemBackgroundColor(int quadNum) const = 0;
	/*! \remarks This method returns the item background color of a specific
	quad. This method returns the color as a <b>COLORREF</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual COLORREF GetItemBackgroundColorRef(int quadNum) const = 0;

	// sets and gets the item text color for a specific quad (numbered 1 through 4)
	/*! \remarks This method allows you to set the item text color for a
	specific quad.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4).\n\n
	<b>Color\& color</b>\n\n
	The color to set. */
	virtual void SetItemTextColor(int quadNum, const Color& color) = 0;
	/*! \remarks This method returns the item text color of a specific quad.
	This method returns the color as a <b>Color</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual const Color& GetItemTextColor(int quadNum) const = 0;
	/*! \remarks This method returns the item text color of a specific quad.
	This method returns the color as a <b>COLORREF</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual COLORREF GetItemTextColorRef(int quadNum) const = 0;

	// sets and gets the last executed item text color for a specific quad (numbered 1 through 4)
	/*! \remarks This method allows you to set the last executed item text
	color for a specific quad.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4).\n\n
	<b>Color\& color</b>\n\n
	The color to set. */
	virtual void SetLastExecutedItemTextColor(int quadNum, const Color& color) = 0;
	/*! \remarks This method returns the last executed item text color of a
	specific quad. This method returns the color as a <b>Color</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual const Color& GetLastExecutedItemTextColor(int quadNum) const = 0;
	/*! \remarks This method returns the last executed item text color of a
	specific quad. This method returns the color as a <b>COLORREF</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual COLORREF GetLastExecutedItemTextColorRef(int quadNum) const = 0;

	// sets and gets the highlighted item background color for a specific quad (numbered 1 through 4)
	/*! \remarks This method allows you to set the highlighted item background
	color for a specific quad.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4).\n\n
	<b>Color\& color</b>\n\n
	The color to set. */
	virtual void SetHighlightedItemBackgroundColor(int quadNum, const Color& color) = 0;
	/*! \remarks This method returns the highlighted item background color of
	a specific quad. This method returns the color as a <b>Color</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual const Color& GetHighlightedItemBackgroundColor(int quadNum) const = 0;
	/*! \remarks This method returns the highlighted item background color of
	a specific quad. This method returns the color as a <b>COLORREF</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual COLORREF GetHighlightedItemBackgroundColorRef(int quadNum) const = 0;

	// sets and gets the highlighted item text color for a specific quad (numbered 1 through 4)
	/*! \remarks This method allows you to set the highlighted item text color
	for a specific quad.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4).\n\n
	<b>Color\& color</b>\n\n
	The color to set. */
	virtual void SetHighlightedItemTextColor(int quadNum, const Color& color) = 0;
	/*! \remarks This method returns the highlighted item text color of a
	specific quad. This method returns the color as a <b>Color</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual const Color& GetHighlightedItemTextColor(int quadNum) const = 0;
	/*! \remarks This method returns the highlighted item text color of a
	specific quad. This method returns the color as a <b>COLORREF</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual COLORREF GetHighlightedItemTextColorRef(int quadNum) const = 0;

	// sets and gets the border color for a specific quad (numbered 1 through 4)
	/*! \remarks This method allows you to set the border color for a specific
	quad.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4).\n\n
	<b>Color\& color</b>\n\n
	The color to set. */
	virtual void SetBorderColor(int quadNum, const Color& color) = 0;
	/*! \remarks This method returns the border color of a specific quad. This
	method returns the color as a <b>Color</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual const Color& GetBorderColor(int quadNum) const = 0;
	/*! \remarks This method returns the border color of a specific quad. This
	method returns the color as a <b>COLORREF</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual COLORREF GetBorderColorRef(int quadNum) const = 0;

	// sets and gets the disabled shadow color for a specific quad (numbered 1 through 4)
	/*! \remarks This method allows you to set the disabled shadow color for a
	specific quad.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4).\n\n
	<b>Color\& color</b>\n\n
	The color to set. */
	virtual void SetDisabledShadowColor(int quadNum, const Color& color) = 0;
	/*! \remarks This method returns the disabled shadow color of a specific
	quad. This method returns the color as a <b>Color</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual const Color& GetDisabledShadowColor(int quadNum) const = 0;
	/*! \remarks This method returns the disabled shadow color of a specific
	quad. This method returns the color as a <b>COLORREF</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual COLORREF GetDisabledShadowColorRef(int quadNum) const = 0;

	// sets and gets the disabled highlight color for a specific quad (numbered 1 through 4)
	/*! \remarks This method allows you to set the disabled highlight color
	for a specific quad.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4).\n\n
	<b>Color\& color</b>\n\n
	The color to set. */
	virtual void SetDisabledHighlightColor(int quadNum, const Color& color) = 0;
	/*! \remarks This method returns the disabled highlight color of a
	specific quad. This method returns the color as a <b>Color</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual const Color& GetDisabledHighlightColor(int quadNum) const = 0;
	/*! \remarks This method returns the disabled highlight color of a
	specific quad. This method returns the color as a <b>COLORREF</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual COLORREF GetDisabledHighlightColorRef(int quadNum) const = 0;

#ifdef USE_NEW_CUI_IO_METHODS	// russom - 02/15/02
	// save and load .qmo quad menu option files
	virtual BOOL SaveSettingsFile( MCHAR *szFilename ) = 0;
	virtual BOOL LoadSettingsFile( MCHAR *szFilename ) = 0;
#endif

};


#define MENU_SETTINGS Interface_ID(0x31561ddb, 0x1a2f4619)
inline IQuadMenuSettings* GetQuadSettings() { return (IQuadMenuSettings*)GetCOREInterface(MENU_SETTINGS); }


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/*! \sa  Class IMenuItem, Class IPoint2, Class Box2\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This abstract class represents an interface for any menu element. Methods that
are marked as internal should not be used.  */
class IMenuElement: public MaxHeapOperators

//  PURPOSE:
//    Abstract class (Interface) for any menu element
//
//  NOTES:
//    created:  02.17.00 - mjm
//
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

{
public:
	// location indicates origin location relative to element rectangle
	enum OriginLocation { UPPER_LEFT, LOWER_LEFT, LOWER_RIGHT, UPPER_RIGHT };
	/*! \remarks Destructor. */
	virtual ~IMenuElement() {;}
	// sets and gets the element's origin, and origin location
	/*! \remarks This method is used internally.\n\n
	This method allows you to set the element's origin and origin location.
	\par Parameters:
	<b>IPoint2\& origin</b>\n\n
	The x, y coordinates of the origin.\n\n
	<b>OriginLocation location</b>\n\n
	The origin location, either one of; <b>UPPER_LEFT</b>, <b>LOWER_LEFT</b>,
	<b>LOWER_RIGHT</b>, or <b>UPPER_RIGHT</b>. */
	virtual void SetOrigin(const IPoint2& origin, OriginLocation location) = 0;
	/*! \remarks This method is used internally.\n\n
	This method returns the x, y coordinates of the element's origin. */
	virtual const IPoint2& GetOrigin() const = 0;

	// sets and gets the item's visibility
	/*! \remarks This method allows you to set the visibility of the element.
	\par Parameters:
	<b>bool visible</b>\n\n
	TRUE for visible, FALSE for invisible. */
	virtual void SetVisible(bool visible) = 0;
	/*! \remarks This method returns the visibility of the element. TRUE if
	the element is visible, otherwise FALSE. */
	virtual bool GetVisible() = 0;

	// sets and gets the item's title
	/*! \remarks This method allows you to set the item's title.
	\par Parameters:
	<b>MCHAR *customTitle</b>\n\n
	The title string. */
	virtual void SetTitle(const MCHAR *customTitle) = 0;
	/*! \remarks This method returns the item's title string. */
	virtual const MSTR& GetTitle() = 0;

	// sets and gets the enabled state of the element
	/*! \remarks This method allows you to enable and disable the element.
	\par Parameters:
	<b>bool enabled</b>\n\n
	TRUE to enable, FALSE to disable. */
	virtual void SetEnabled(bool enabled) = 0;
	/*! \remarks This method returns the state of the element. TRUE if it's
	enabled, FALSE if it's disabled. */
	virtual bool GetEnabled() = 0;

	// gets the element's size, in the menu's coordinate space
	/*! \remarks This method is used internally.\n\n
	This method returns the element's size in the menu's coordinate space. */
	virtual const IPoint2& GetSize() = 0;

	// gets the element's rectangle, in the menu's coordinate space
	/*! \remarks This method is used internally.\n\n
	This method returns the element's rectangle size in the menu's coordinate
	space. */
	virtual const Box2& GetRect() = 0;

	// determines if point is in element's rectangle
	/*! \remarks This method is used internally.\n\n
	This method determines if a specific point is inside the element's
	rectangle.
	\par Parameters:
	<b>IPoint2\& point</b>\n\n
	The point to test.
	\return  TRUE if the point is inside the rectangle, otherwise FALSE. */
	virtual bool IsInRect(const IPoint2& point) = 0;
};


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#define MENU_ITEM_INTERFACE Interface_ID(0x2e926bd1, 0x296e68f6)

/*! \sa  Class IMenuElement, Class IMenuGlobalContext, Class IMenuLocalContext, Class ActionItem, Class IMenu, Class MaxIcon , Class IQuadMenuSettings\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This abstract class represents an interface for a menu item. Methods that are
marked as internal should not be used.  */
class IMenuItem : public FPMixinInterface, public IMenuElement

//  PURPOSE:
//    Abstract class (Interface) for a menu item
//
//  NOTES:
//    created:  02.17.00 - mjm
//
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

{
public:
	typedef int IMenuItemIcon;

	// function prototype - to be called when item is selected
	typedef void (* ActionFn)(void);
	// function prototype - to be called before item is displayed
	typedef void (* PreDisplayCB)(IMenuItem& menuItem);

	// action mode - item selection instigates action item, calls function, or displays a submenu
	// these values are saved/loaded by the menu customization system. you may append new values,
	// but cannot change any existing values.
	enum ActionMode { AM_INACTIVE = 0,
                      AM_SEPARATOR,
                      AM_ITEM,
                      AM_FN,
                      AM_SUBMENU,
                      AM_ITEM_SUBMENU };

	// sets a new context for the menu, invalidating the menu's cache
	/*! \remarks This method is used internally.\n\n
	This method allows you to set a new (local) context for the menu,
	invalidating the menu's cache.
	\par Parameters:
	<b>IMenuLocalContext* pIMenuLocalContext</b>\n\n
	A pointer to the new local context object you wish to set. */
	virtual void SetIMenuLocalContext(IMenuLocalContext* pIMenuLocalContext) = 0;

	// gets the current action mode
	/*! \remarks This method returns the current action mode. When item
	selection instigates an action item, calls functions, or displays a submenu
	the action mode changes to indicate the exact state the system is in.
	\return  Either of the following; <b>AM_INACTIVE, AM_SEPARATOR, AM_ITEM,
	AM_FN, AM_SUBMENU, AM_ITEM_SUBMENU</b> */
	virtual ActionMode GetActionMode() const = 0;

	// executes the current action
	/*! \remarks This method will execute the current action.
	\return  TRUE if the action was executed successfully, otherwise FALSE. */
	virtual bool ExecuteAction() const = 0;

	// makes the item act as an item separator
	/*! \remarks This method allows you to make the item act as an item
	seperator. */
	virtual void ActAsSeparator() = 0;
	// checks if the item is acting as an item separator
	/*! \remarks This method determines if the item is acting as a seperator
	(TRUE) or not (FALSE). */
	virtual bool IsSeparator() const = 0;

	// sets and gets the current action item. GetActionItem returns NULL if the ActionMode is not AM_ITEM
	/*! \remarks This method allows you to set the current action item. Note
	that <b>GetActionItem()</b> returns NULL if the ActionMode is not
	<b>AM_ITEM.</b>
	\par Parameters:
	<b>ActionItem* pActionItem</b>\n\n
	The action item you wish to set. */
	virtual void SetActionItem(ActionItem* pActionItem) = 0;
	/*! \remarks This method returns a pointer to the current action item, or
	NULL if the ActionMode is not <b>AM_ITEM.</b> */
	virtual ActionItem* GetActionItem() const = 0;

	// sets and gets the current action function. GetActionFn returns NULL if the ActionMode is not AM_FN
	/*! \remarks This method allows you to set the current action function.
	Note that <b>GetActionFn()</b> returns NULL if the ActionMode is not
	<b>AM_FN</b>. A\n\n
	lso note: <b>typedef void (* ActionFn)(void);</b>
	\par Parameters:
	<b>ActionFn actionFn</b>\n\n
	The action function you wish to set. */
	virtual void SetActionFn(ActionFn actionFn) = 0;
	/*! \remarks This method returns the current action function, or NULL if
	the ActionMode is not <b>AM_FN</b>.\n\n
	Note: <b>typedef void (* ActionFn)(void);</b> */
	virtual const ActionFn GetActionFn() const = 0;

	// sets and gets the submenu. GetSubMenu returns NULL if the ActionMode is not AM_SUBMENU
	/*! \remarks This method allows you to set the submenu. Note that
	<b>GetSubMenu()</b> returns NULL if the ActionMode is not
	<b>AM_SUBMENU</b>.
	\par Parameters:
	<b>IMenu* menu</b>\n\n
	The submenu you wish to set. */
	virtual void SetSubMenu(IMenu* menu) = 0;
	/*! \remarks This method returns a pointer to the submenu, or NULL if the
	ActionMode is not <b>AM_SUBMENU</b>. */
	virtual IMenu* GetSubMenu() = 0;

	// sets and gets the current pre-display callback
	/*! \remarks This method allows you to set the pre-display callback.\n\n
	Note: <b>typedef void (* PreDisplayCB)(IMenuItem\& menuItem);</b>
	\par Parameters:
	<b>PreDisplayCB preDisplayCB</b>\n\n
	The callback to set. */
	virtual void SetPreDisplayCB(PreDisplayCB preDisplayCB) = 0;
	/*! \remarks This method returns the pre-display callback. */
	virtual const PreDisplayCB GetPreDisplayCB() const = 0;

	// displays the item
	/*! \remarks This method is used internally. */
	virtual void Display(bool leftToRight) = 0;

	// gets the item's accelerator, returns 0 if none
	/*! \remarks This method is used internally.\n\n
	This method returns the item's accelerator, or 0 if none is assigned. */
	virtual MCHAR GetAccelerator() = 0;

	// sets and gets the item's icon
	/*! \remarks This method allows you to set the item's icon.
	\par Parameters:
	<b>MaxIcon* pMaxIcon</b>\n\n
	A pointer to a MaxIcon to set. */
	virtual void SetIcon(MaxIcon* pMaxIcon) = 0;
	/*! \remarks This method returns a pointer to the item's icon. */
	virtual const MaxIcon* GetIcon() const = 0;

	// sets and gets the item's checked state
	/*! \remarks This method allows you to set the checked state of the item.
	\par Parameters:
	<b>bool checked</b>\n\n
	TRUE to check the item, FALSE to uncheck the item. */
	virtual void SetChecked(bool checked) = 0;
	/*! \remarks This method returns TRUE if the item is checked or FALSE if
	it is unchecked. */
	virtual bool GetChecked() = 0;

	// sets and gets the item's highlighted state
	/*! \remarks This method allows you to set the highlighted state of the
	item.
	\par Parameters:
	<b>bool highlighted</b>\n\n
	TRUE to highlight the item, FALSE if you do not want to highlight the item.
	*/
	virtual void SetHighlighted(bool highlighted) = 0;
	/*! \remarks This method returns TRUE if the item is highlighted,
	otherwise FALSE. */
	virtual bool GetHighlighted() const = 0;

	// sets and gets if the item should use a custom title -- set via SetTitle()
	/*! \remarks This method allows you to tell the item it should use a
	custom title, which is set through <b>SetTitle()</b>.
	\par Parameters:
	<b>bool useCustomTitle</b>\n\n
	TRUE to use a custom title, FALSE to use the default. */
	virtual void SetUseCustomTitle(bool useCustomTitle) = 0;
	/*! \remarks This method returns TRUE if the item is using a custom title
	or FALSE if it's using a default. */
	virtual bool GetUseCustomTitle() const = 0;

	// sets and gets if the submenu-item should be displayed flat
	/*! \remarks This method allows you to set whether the submenu-item should
	be displayed 'flat'.
	\par Parameters:
	<b>bool displayFlat</b>\n\n
	TRUE to set to flat, otherwise FALSE. */
	virtual void SetDisplayFlat(bool displayFlat) = 0;
	/*! \remarks This method returns TRUE if the submenu-item should be
	displayed 'flat', otherwise FALSE. */
	virtual bool GetDisplayFlat() const = 0;

	// called after a user/menu interaction
	/*! \remarks This method is used internally.\n\n
	This method is called after the user/menu interaction is done after which
	it will clear the caches for ActionItem handler values. */
	virtual void PostMenuInteraction() = 0;

    // Function publishing function ids.
    enum
    {
        setTitle,
        getTitle,
        setUseCustomTitle,
        getUseCustomTitle,
        setDisplayFlat,
        getDisplayFlat,
        getIsSeparator,
        getSubMenu,
        getMacroScript,
    };
};


#define MENU_INTERFACE Interface_ID(0x4bd57e2e, 0x6de57aeb)

/*! \sa  Class IMenuElement, Class IMenuItem, Class IMenuGlobalContext, Class IMenuLocalContext, Structure MenuEvent, Class IPoint2, Class Interface.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This abstract class represents an interface for a menu item. Methods that are
marked as internal should not be used.  */
class IMenu : public FPMixinInterface, public IMenuElement
{
public:
	// sets a new context for the menu, invalidating the menu's cache
	/*! \remarks This method is used internally.\n\n
	This method sets a new context for the menu, invalidating the menu's cache.
	\par Parameters:
	<b>IMenuGlobalContext* pIMenuGlobalContext</b>\n\n
	Points to the context to set. */
	virtual void SetIMenuGlobalContext(IMenuGlobalContext* pIMenuGlobalContext, int level, Tab<IMenuItem *> *pExecutedItemPath, const MenuColors *pMenuColors) = 0;

	// gets the menu's local context
	/*! \remarks This method is used internally.\n\n
	Returns a pointer to the menu's local context. */
	virtual IMenuLocalContext* GetIMenuLocalContext() = 0;

	// returns number of contained items
	/*! \remarks Returns the number of items in the menu. */
	virtual int NumItems() const = 0;

	// retrieves an item from the menu
	/*! \remarks Returns a pointer to the specified menu item.
	\par Parameters:
	<b>int position</b>\n\n
	The position/index in the menu. */
	virtual IMenuItem* GetItem(int position) = 0;

	// adds an item into the menu, defaulting to the end position.
	// position 0 indicates beginnng of list. a negative or otherwise invalid position defaults to end of list.
	/*! \remarks This method adds the specified item into the menu at the position
	passed. The default position is at the end.
	\par Parameters:
	<b>IMenuItem* item</b>\n\n
	Points to the menu item to add.\n\n
	<b>int position = -1</b>\n\n
	Position 0 indicates the beginnng of the list. A negative or otherwise invalid
	position defaults to end of list. */
	virtual void AddItem(IMenuItem* item, int position = -1) = 0;

	// if valid position, removes item at position in menu
	/*! \remarks Removes the item from the menu whose position is passed.
	\par Parameters:
	<b>int position</b>\n\n
	The zero based index in the list of the item to remove. Position 0 is the
	first item. */
	virtual void RemoveItem(int position) = 0;

	// if present, removes an item from the menu
	/*! \remarks Removes the specified item from the menu (if it indeed
	appears in the menu).
	\par Parameters:
	<b>IMenuItem* item</b>\n\n
	Points to the menu item to remove. */
	virtual void RemoveItem(IMenuItem* item) = 0;

	// returns the maximum size of all items in menu
	/*! \remarks This method is used internally.\n\n
	Returns the maximum size of all items in the menu in pixels. */
	virtual IPoint2 GetMaxItemSize() = 0;

	// called before menu is first displayed during a user/menu interaction
	/*! \remarks This method is used internally.\n\n
	This method is called before menu is first displayed during a user / menu
	interaction. */
	virtual void Initialize() = 0;

	// called after a user/menu interaction
	/*! \remarks This method is used internally.\n\n
	This method is called after a user / menu interaction. */
	virtual void PostMenuInteraction() = 0;

	// handles an event occuring within the menu
	/*! \remarks This method is used internally.\n\n
	This method is called to handle an event occuring within the menu.
	\par Parameters:
	<b>MenuEvent event</b>\n\n
	A menu event structure containing the event data.
	\return  TRUE if the event was handled successfully, otherwise FALSE.\n\n
	  */
	virtual bool HandleEvent(MenuEvent &event) = 0;

	// shows the menu with the given show type
	/*! \remarks This method is used internally.\n\n
	This method will display the menu using the provided display method. */
	virtual void Show(DisplayMethod displayMethod = DM_NORMAL, Box2 *rect = NULL) = 0;

	// hides the menu with the given show type
	/*! \remarks This method is used internally.\n\n
	This method will hide the menu using the provided display method. */
	virtual void Hide(DisplayMethod displayMethod = DM_NORMAL) = 0;

	/*! \remarks This method is used internally.\n\n
	Finds and returns a pointer to the menu item whose accelerator is passed.
	\par Parameters:
	<b>MCHAR accelerator</b>\n\n
	The single character of the accelerator.
	\return  A pointer to the menu item or NULL if not found. */
	virtual IMenuItem* FindAccelItem(MCHAR accelerator) = 0;
	/*! \remarks This method is used internally.\n\n
	Returns a pointer to the currently selected menu item. */
	virtual IMenuItem* FindNewSelectedItem() = 0;

	// displays the menu
	/*! \remarks This method is used internally.\n\n
	This method displays the menu.
	\par Parameters:
	<b>IMenu* pParentMenu = NULL</b>\n\n
	Points to the parent menu. */
	virtual void Display(IMenu* pParentMenu = NULL, bool show = true) = 0;

	/*! \remarks This method is used internally. */
	virtual void DisplayItems(IPoint2& origin, bool descending, bool leftToRight, bool nextSeparatorOK) = 0;

	// removes the menu from the display
	/*! \remarks This method is used internally.\n\n
	This methods removes the menu from the display. */
	virtual void Undisplay() = 0;

	// determines if menu is displaying a submenu
	/*! \remarks This method is used internally.\n\n
	Returns TRUE if the menu is displaying a sub-menu; otherwise FALSE. */
	virtual bool IsDisplayingSubMenu() = 0;

	// notifies menu that timer has elapsed
	/*! \remarks This method is used internally.\n\n
	This method notifies the menu that the timer has elapsed. */
	virtual void TimerElapsed(EventParam timingType) = 0;

	// sets/gets whether to show the title
	/*! \remarks This method is used internally.\n\n
	Sets whether to show the title or not.
	\par Parameters:
	<b>bool showTitle</b>\n\n
	Pass true to show the title; false to not show it. */
	virtual void SetShowTitle(bool showTitle) = 0;
	/*! \remarks This method is used internally.\n\n
	Returns true if a title is shown; otherwise false. */
	virtual bool GetShowTitle() const = 0;

	// sets and gets the custom title
	/*! \remarks Sets the custom title to the string passed.
	\par Parameters:
	<b>const MCHAR *customTitle</b>\n\n
	Points to the string to use. */
	virtual void SetCustomTitle(const MCHAR *customTitle) = 0;
	/*! \remarks Returns the custom title string. */
	virtual const MSTR& GetCustomTitle() const = 0;

	// sets and gets if the item should use a custom title -- set via SetCustomTitle()
	/*! \remarks Sets if the item should use a custom title.
	\par Parameters:
	<b>bool useCustomTitle</b>\n\n
	Pass true to use a custom title; false to not use one. */
	virtual void SetUseCustomTitle(bool useCustomTitle) = 0;
	/*! \remarks This method returns TRUE if the menu uses a custom title,
	otherwise FALSE. */
	virtual bool GetUseCustomTitle() const = 0;

	// sets/gets whether to show the title
	/*! \remarks This method is used internally.\n\n
	This method allows you to instruct the menu to use global widths in order
	to show the title.
	\par Parameters:
	<b>bool useGlobalWidths</b>\n\n
	TRUE to use global widths, otherwise FALSE. */
	virtual void SetUseGlobalWidths(bool useGlobalWidths) = 0;
	/*! \remarks This method is used internally.\n\n
	This method returns TRUE if the menu uses global widths in order to show
	the title, otherwise FALSE. */
	virtual bool GetUseGlobalWidths() const = 0;

    // return true if the menu has no visible items in it.
    /*! \remarks This method is used internally.\n\n
    Returns true if the menu has no visible items in it; otherwise false. */
    virtual bool NoVisibleItems() = 0;

    // Function publishing function ids.
    enum
    {
        numItems,
        getItem,
        addItem,
        removeItem,
        removeItemByPosition,
        setTitle,
        getTitle,
        getUseCustomTitle,
    };        
};


/*! \sa  Class IMenu\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This abstract class represents an interface for a popup menu and is used
internally. Methods that are marked as internal should not be used.  */
class IPopupMenu : public IMenu
{
public:
	// sets and gets the menu
	/*! \remarks This method is used internally.\n\n
	This method allows you to set the menu associated with this popup menu.
	\par Parameters:
	<b>IMenu* menu</b>\n\n
	A pointer to the menu. */
	virtual void SetMenu(IMenu* menu) const = 0;
	/*! \remarks This method is used internally.\n\n
	This method returns a pointer to the menu associated with this popup menu.
	*/
	virtual IMenu* GetMenu() const = 0;

	/*! \remarks This method is used internally.\n\n
	This method allows you to track a quad menu.
	\par Parameters:
	<b>HWND hMessageWnd</b>\n\n
	A handle to the message window.\n\n
	<b>bool displayAll = false</b>\n\n
	TRUE if all menus should be displayed, otherwise FALSE. */
	virtual void TrackMenu(HWND hMessageWnd, bool displayAll = false) = 0;
};


/*! \sa  Class IMenu\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This abstract class represents an interface for a multi-menu.  */
class IMultiMenu: public MaxHeapOperators
{
public:
	/*! \remarks Destructor. */
	virtual ~IMultiMenu() {;}
	// returns number of contained menus allowed, -1 indicates infinite
	/*! \remarks This method returns the number of contained menus allowed. A
	value of -1 will indicate an infinite number. */
	virtual int NumMenusAllowed() const = 0;

	// returns number of contained menus
	/*! \remarks This method returns the number of menus contained. */
	virtual int NumMenus() const = 0;

	// retrieves a menu from the multi-menu
	/*! \remarks This method returns a pointer to a menu from the multi-menu
	depending on its position index.
	\par Parameters:
	<b>int position</b>\n\n
	The position/index of the menu you wish to retrieve. */
	virtual IMenu* GetMenu(int position) = 0;

	// adds a menu into the container, defaulting to the end position.
	// position of -1 means end of list, 0 means beginning.
	/*! \remarks This method allows you to add a menu into the container. Each new
	menu which is added will default to the end position.
	\par Parameters:
	<b>IMenu* menu</b>\n\n
	A pointer to the menu to add.\n\n
	<b>int pos = -1</b>\n\n
	The position to insert the menu at, -1 for the end of the list, and 0 signifies
	the beginning of the list. */
	virtual void AddMenu(IMenu* menu, int pos = -1) = 0;

	// if valid position, removes menu at position in menu
	/*! \remarks This method allows you to remove the menu at the specified
	position.
	\par Parameters:
	<b>int position</b>\n\n
	The position/index of the menu to remove. */
	virtual void RemoveMenu(int position) = 0;

	// if present, removes a menu from the quad menu
	/*! \remarks This method allows you to remove the specified menu from the
	container.
	\par Parameters:
	<b>IMenu* menu</b>\n\n
	A pointer to the menu to remove */
	virtual void RemoveMenu(IMenu* menu) = 0;

	// sets and gets the title for the menu position indicated. used when SetUseCustomTitle(true) has been called
	/*! \remarks This method allows you to set the title for the specified
	menu position.
	\par Parameters:
	<b>MCHAR *customTitle</b>\n\n
	The custom title string.\n\n
	<b>int pos</b>\n\n
	The menu position. */
	virtual void SetTitle(const MCHAR *customTitle, int pos) = 0; // TODO: use 'customtitle' in item as well
	/*! \remarks This method returns the title for the specified menu
	position. */
	virtual const MSTR& GetTitle(int pos) = 0;

	// sets and gets if the menu at indicated position should use a custom title -- set via SetTitle()
	/*! \remarks This method allows you to set whether the menu at indicated
	position should use a custom title.
	\par Parameters:
	<b>int pos</b>\n\n
	The position of the menu.\n\n
	<b>bool useCustomTitle</b>\n\n
	TRUE to set the menu to use a custom title, otherwise FALSE. */
	virtual void SetUseCustomTitle(int pos, bool useCustomTitle) = 0;
	/*! \remarks This method returns TRUE if the menu as the specified
	position is using a custom title, otherwise FALSE.
	\par Parameters:
	<b>int pos</b>\n\n
	The position of the menu. */
	virtual bool GetUseCustomTitle(int pos) const = 0;
};


/*! \sa  Class IMultiMenu\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This abstract class represents an interface for a menu bar and is used
internally. Methods that are marked as internal should not be used.  */
class IMenuBar : public IMultiMenu
{
public:
	/*! \remarks This method is used internally.\n\n
	This method allows you to track a quad menu.
	\par Parameters:
	<b>HWND hMessageWnd</b>\n\n
	A handle to the message window.\n\n
	<b>bool displayAll = false</b>\n\n
	TRUE if all menus should be displayed, otherwise FALSE. */
	virtual void TrackMenu(HWND hMessageWnd, bool displayAll = false) = 0;
};


#define QUAD_MENU_INTERFACE Interface_ID(0x78b735e9, 0x7c001f68)

/*! \sa  Class IMultiMenu, Class FPMixinInterface\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This abstract class represents an interface for a quad menu.  */
class IQuadMenu : public FPMixinInterface, public IMultiMenu
{
public:
	/*! \remarks This method allows you to track a quad menu.
	\par Parameters:
	<b>HWND hMessageWnd</b>\n\n
	A handle to the message window.\n\n
	<b>bool displayAll = false</b>\n\n
	TRUE if all quads should be displayed, otherwise FALSE. */
	virtual void TrackMenu(HWND hMessageWnd, bool displayAll = false) = 0;

    // Function publishing function ids.
    enum
    {
        getMenu,
        addMenu,
        removeMenu,
        removeMenuByPosition,
        setTitle,
        getTitle,
        trackMenu,
    };
};


/*! \remarks This method will return a pointer to the IMenuItem. */
MENUS_API IMenuItem * GetIMenuItem();
/*! \remarks This method will release the specified IMenuItem.
\par Parameters:
<b>IMenuItem *</b>\n\n
A pointer to the IMenuItem you wish to release. */
MENUS_API void ReleaseIMenuItem(IMenuItem *);

/*! \remarks This method will return a pointer to the IMenu. */
MENUS_API IMenu * GetIMenu();
/*! \remarks This method will release the specified IMenu.
\par Parameters:
<b>IMenuItem *</b>\n\n
A pointer to the IMenu you wish to release. */
MENUS_API void ReleaseIMenu(IMenu *);

/*! \remarks This method will return a pointer to the IQuadMenu. */
MENUS_API IQuadMenu * GetIQuadMenu();
/*! \remarks This method will release the specified IQuadMenu.
\par Parameters:
<b>IMenuItem *</b>\n\n
A pointer to the IQuadMenu you wish to release. */
MENUS_API void ReleaseIQuadMenu(IQuadMenu *);

