/*********************************************************************
 *<
	FILE: rtclick.h

	DESCRIPTION: Right-click menu functionality

	CREATED BY:	Tom Hudson

	HISTORY: Created 14 June 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "maxheap.h"
#include "ipoint2.h"
#include "tab.h"
#include "stack.h"
#include "coreexp.h"
#include "strbasic.h"

const int kMaxMenuEntries = 1000;

class RightClickMenuManager;

/*! \sa  Class RightClickMenuManager,  Class Interface, Class IPoint2.\n\n
\par Description:
This class provides methods to work with the right click menu that pops up when
the user right clicks over an item in a viewport. Methods of this class are
used to initialize the menu and process the users selections. A developer
derives a class from this class. For sample code see
<b>/MAXSDK/SAMPLES/MODIFIERS/EDITPATCH/EDITPAT.CPP</b>. Also see the method
<b>Interface::GetRightClickMenuManager()</b>.  */
class RightClickMenu: public MaxHeapOperators {
	public:
		/*! \remarks Destructor. */
		virtual ~RightClickMenu() {}
		/*! \remarks This method is called when the user right clicks on an object in the
		scene. At this point you can determine what you need in the menu, and
		add these items using <b>manager-\>AddMenu()</b>.
		\par Parameters:
		<b>RightClickMenuManager* manager</b>\n\n
		The menu manager. You may use this pointer to call methods of this
		class (for example to add items to the menu).\n\n
		<b>HWND hWnd</b>\n\n
		The window handle the user right clicked in.\n\n
		<b>IPoint2 m</b>\n\n
		The screen point the user right clicked on. */
		virtual void Init(RightClickMenuManager* manager, HWND hWnd, IPoint2 m)=0;
		/*! \remarks This method is called when the user has selected an item from the menu.
		\par Parameters:
		<b>UINT id</b>\n\n
		This is the <b>id</b> of the users selection. This <b>id</b> is
		established when the developer calls
		<b>RightClickMenuManager::AddMenu()</b>. */
		virtual void Selected(UINT id)=0;
	};

typedef RightClickMenu* PRightClickMenu;
typedef Tab<PRightClickMenu> PRightClickMenuTab;

class RCMData: public MaxHeapOperators {
	public:
		RightClickMenu *menu;
		UINT menuId;
		UINT managerId;
		RCMData() { menu = NULL; menuId = managerId = 0; };
		RCMData(RightClickMenu *menu, UINT menuId, UINT managerID);
	};

typedef Tab<RCMData> RCMDataTab;

/*! \sa  Class RightClickMenu, Class Interface.\n\n
\par Description:
Methods of this class allow a developer to extend the menu presented when a
user right clicks on an item in the viewports. All methods of this class are
implemented by the system.  */
class RightClickMenuManager: public MaxHeapOperators {
	private:
        Stack<HMENU> menuStack;
        Tab<HMENU>   allSubMenus;
		HMENU theMenu;
		PRightClickMenuTab menuTab;
		RCMDataTab dataTab;
		int index;
		int limit;
		int startId;
	public:
		/*! \remarks This method is used to register an instance of a class
		derived from <b>RightClickMenu</b>. This allows its methods to be
		called when the user right clicks on an object in a viewport.
		\par Parameters:
		<b>RightClickMenu *menu</b>\n\n
		The menu to set as register. */
		CoreExport void Register(RightClickMenu *menu);
		/*! \remarks This method is used to un-register a registered right
		click menu.
		\par Parameters:
		<b>RightClickMenu *menu</b>\n\n
		The menu to set as un-register. */
		CoreExport void Unregister(RightClickMenu *menu);
		/*! \remarks This method is used internally. */
		CoreExport void Init(HMENU menu, int startId, int limit, HWND hWnd, IPoint2 m);
        /*! \remarks This method is available in release 3.0 and later
        only.\n\n
        This removes all sub menus from the right click menu. */
        CoreExport void CleanUp();

		/*! \remarks This method is called to add items to the right click
		menu passed. A string or separator may be added. Strings may be checked
		or unchecked. They may also be disabled and grayed.
		\par Parameters:
		<b>RightClickMenu *menu</b>\n\n
		The menu to add the item to.\n\n
		<b>UINT flags</b>\n\n
		Describes the item being added. One or more of the following
		values:\n\n
		<b>MF_CHECKED</b>\n\n
		<b>MF_UNCHECKED</b>\n\n
		<b>MF_STRING</b>\n\n
		<b>MF_DISABLED</b>\n\n
		<b>MF_GRAYED</b>\n\n
		<b>MF_SEPARATOR</b>\n\n
		<b>UINT id</b>\n\n
		The <b>id</b> of the selection. This is the <b>id</b> passed to the
		<b>RightClickMenu::Selected()</b> method when the user makes a
		selection from the menu.\n\n
		<b>MCHAR* data</b>\n\n
		The string to display in the menu (or NULL if adding a separator).
		\return  Nonzero if the item was added; otherwise 0. */
		CoreExport int AddMenu(RightClickMenu *menu, UINT flags, UINT id, const MCHAR* data);
        /*! \remarks This method is available in release 3.0 and later
        only.\n\n
        This begins a new sub menu. Items added after this call will appear as
        sub choices of this one until <b>EndSubMenu()</b> is called.
        \par Parameters:
        <b>MCHAR* name</b>\n\n
        The name to appear for the sub menu item.
        \return  Always returns TRUE. */
        CoreExport int BeginSubMenu(const MCHAR* name);
        /*! \remarks This method is available in release 3.0 and later
        only.\n\n
        This ends a sub menu. Items added after this call will appear as they
        did prior to calling <b>BeginSubMenu()</b>.
        \return  Always returns TRUE. */
        CoreExport int EndSubMenu();

		/*! \remarks This method is used internally. */
		CoreExport void Process(UINT id);
	};

