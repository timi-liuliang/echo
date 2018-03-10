/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: iMenuMan.h

	 DESCRIPTION: abstract classes for the menu manager

	 CREATED BY: Scott Morrison

	 HISTORY: created March 21, 2000

   	 Copyright (c) 2000, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */
#pragma once

#include "iMenus.h"

typedef int MenuContextId;

// Define some menu contexts used by the code of MAX

const MenuContextId kMainMenuBar			= 1;
const MenuContextId kViewportQuad			= 2;
const MenuContextId kSchematicViewQuad		= 3;
const MenuContextId kIReshadeQuad			= 4;
const MenuContextId kIUVWUnwrapQuad			= 5;
const MenuContextId kTrackViewQuad			= 6;
const MenuContextId kTrackViewMenuBar		= 7;
const MenuContextId kSchematicViewMenuBar	= 8;
const MenuContextId kMaterialEditorMenuBar	= 9;


enum MenuContextType {
    kMenuContextMenuBar = 0,
    kMenuContextQuadMenu,
    kMenuContextPopupMenu,
};
    

/*! \sa  Class InterfaceServer, Class IMenuBarContext, Class Interface.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the interface to the various menu types such as MenuBar,
QuadMenus, and right-click Popup Menus. The class provides the fundamental
basis for the ID, Type, and Name.\n\n
   */
class IMenuContext : public InterfaceServer {

public:
    /*! \remarks This method returns the Menu Context ID. */
    virtual MenuContextId GetId() = 0;
    /*! \remarks This method returns the Menu Contex Type. One of the
    following values:\n\n
    <b>kMenuContextMenuBar</b>\n\n
    The context for the main menu bar.\n\n
    <b>kMenuContextQuadMenu</b>\n\n
    The context for the quad menu.\n\n
    <b>kMenuContextPopupMenu</b>\n\n
    The context for the Ctrl-right click popup menu. */
    virtual MenuContextType GetType() = 0;
    /*! \remarks This method returns the name of the Menu Context. */
    virtual MSTR& GetName() = 0;
};

/*! \sa  Class IMenuContext, Class Interface.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n

\par Description:
This class is available in release 4.0 and later only.\n\n
This abstract class represents an interface for a menu bar context. Methods
that are marked as internal should not be used.  */
class IMenuBarContext: public IMenuContext {

public:

    /*! \remarks This method allows you to set the menu associated with this
    context.
    \par Parameters:
    <b>IMenu* pMenu</b>\n\n
    A pointer to the menu. */
    virtual void SetMenu(IMenu* pMenu) = 0;

    /*! \remarks This method returns a pointer to the menu associated with
    this context. */
    virtual IMenu* GetMenu() = 0;
    /*! \remarks Used Internally.\n\n
    This method will create a new windows menu and return it's handle. */
    virtual HMENU CreateWindowsMenu() = 0;
    /*! \remarks Used Internally.\n\n
    This method will update the current windows menu. */
    virtual void UpdateWindowsMenu() = 0;

    /*! \remarks Used Internally.\n\n
    This method returns the handle to the current windows menu. */
    virtual HMENU GetCurWindowsMenu() = 0;

    /*! \remarks This method executes an action based on the provided command
    ID.
    \par Parameters:
    <b>int cmdId</b>\n\n
    The command ID of the action to execute. */
    virtual void ExecuteAction(int cmdId) = 0;
    /*! \remarks Used internally.
    \par Parameters:
    <b>int cmdId</b>\n\n
    The command ID. */
    virtual bool CommandIDInRange(int cmdId) = 0;
};

/*! \sa  Class IMenuContext, Class Interface, \ref rightClickMenuContexts.
\par Description:
This class is available in release 4.0 and later only.\n\n
This abstract class represents the interface for a quad menu bar context and
provides the functionality to manage the quad menu context by adding and
removing menu sections.\n\n
Note that methods marked for internal use only should not be used.\n\n
   */
class IQuadMenuContext: public IMenuContext {

public:

    // Add a new quad menu to the context.
    /*! \remarks This method allows you to add a new quad menu to the context.
    \par Parameters:
    <b>IQuadMenu* pMenu</b>\n\n
    A pointer to the quad menu you wish to add.\n\n
    <b>MCHAR* pName</b>\n\n
    The name of the quad menu.
    \return  TRUE if the quad menu is successfully added, otherwise FALSE. */
    virtual bool AddQuadMenu(IQuadMenu* pMenu, const MCHAR* pName) = 0;
    // Set the quad menu for a slot in the context.
    /*! \remarks This method allows you to set a quad menu for a slot in the
    context.
    \par Parameters:
    <b>int index</b>\n\n
    The index of the slot in the context.\n\n
    <b>IQuadMenu* pMenu</b>\n\n
    A pointer to the quad menu you wish to set.\n\n
    <b>MCHAR* pName</b>\n\n
    The name of the quad menu. */
    virtual void SetMenu(int index, IQuadMenu* pMenu, const MCHAR* pName) = 0;
    // Remove a Quad menu from the context.
    /*! \remarks This method allows you to remove a quad menu from the
    context.
    \par Parameters:
    <b>int index</b>\n\n
    The index of the quad menu to remove. */
    virtual void RemoveMenu(int index) = 0; 

    // Accessors for all the Quad menus in the context
    /*! \remarks This method returns the number of quad menu's in this
    context. */
    virtual int MenuCount() = 0;
    /*! \remarks This method returns a pointer to a quad menu based on its
    index in the context.
    \par Parameters:
    <b>int index</b>\n\n
    The index of the quad menu you wish to retrieve. */
    virtual IQuadMenu* GetMenu(int index) = 0;

    // Get/Set the index of the current default right-click menu
    /*! \remarks This method returns the index of the currently set default
    right-click menu. */
    virtual int GetCurrentMenuIndex() = 0;
    /*! \remarks This method allows you to set the current default right-click
    menu.
    \par Parameters:
    <b>int index</b>\n\n
    The index of the menu you wish to set as the default menu. */
    virtual void SetCurrentMenuIndex(int index) = 0;

    // Get set the falg that indicates we should display all quads in the menu
    /*! \remarks This method returns the state of the "Show All Menus" flag in
    each quad registered in the menu manager. TRUE if the flag is set or FALSE
    if the flag is not set.
    \par Parameters:
    <b>int index</b>\n\n
    The index of the quad menu. */
    virtual bool GetShowAllQuads(int index) = 0;
    /*! \remarks This method allows you to set the "Show All Menus" flag in
    each quad registered in the menu manager.
    \par Parameters:
    <b>int index</b>\n\n
    The index of the quad menu.\n\n
    <b>bool showAll</b>\n\n
    Set this parameter to TRUE if you wish to enable the "Show All Menus" flag.
    Otherwise FALSE. */
    virtual void SetShowAllQuads(int index, bool showAll) = 0;

    /*! \defgroup rightClickMenuContexts Right-Click Contexts
    This is a list of modifiers key combinations that determine which context menu to display */
    //@{
    enum RightClickContext {
        kNonePressed,					//!< No key is pressed
        kShiftPressed,					//!< Shift key only.
        kAltPressed,					//!< Alt key only.
        kControlPressed,				//!< Control key only.
        kShiftAndAltPressed,			//!< Alt-Shift
        kShiftAndControlPressed,		//!< Ctrl-Shift
        kControlAndAltPressed,			//!< Ctrl-Alt
        kShiftAndAltAndControlPressed,	//!< Ctrl-Shift-Alt
    };
    //@}

    // This method queries the state of the modifier keys and returns the
    // appropriate context.
    /*! \remarks This method queries the state of the modifier keys and
    returns the appropriate context.
    \return See \ref rightClickMenuContexts. */
    virtual RightClickContext GetRightClickContext() = 0;

    // Get/Set the right-click menu associated with a context
    /*! \remarks This method returns a pointer to the quad menu which has been
    assigned to the specified right-click context, or NULL if no menu is
    assigned.
    \par Parameters:
    <b>RightClickContext context</b>\n\n
    See \ref rightClickMenuContexts. */
    virtual IQuadMenu* GetRightClickMenu(RightClickContext context) = 0;
    /*! \remarks This method allows you to set the quad menu associated with a
    specific right-click context.
    \par Parameters:
    <b>RightClickContext context</b>\n\n
    See \ref rightClickMenuContexts.\n\n
    <b>IQuadMenu *pMenu</b>\n\n
    A pointer to the quad menu you wish to set. */
    virtual void SetRightClickMenu(RightClickContext context, IQuadMenu *pMenu) = 0;

    // return the index for the given quad menu.  Return -1 if the menu is not in the context.
    /*! \remarks This method returns the index for a specified quad menu, or
    -1 if the menu is not in the context.
    \par Parameters:
    <b>IQuadMenu* pMenu</b>\n\n
    A pointer to the quad menu you wish to obtain the index of. */
    virtual int FindMenu(IQuadMenu* pMenu) = 0;
    // Find a Quad menu based on its name
    /*! \remarks This method returns a pointer to a quad menu by specifying
    the title of the menu you wish to find. NULL will be returned if the menu
    was not found.
    \par Parameters:
    <b>MCHAR* pTitle</b>\n\n
    The title string of the menu. */
    virtual IQuadMenu* FindMenuByTitle(const MCHAR* pTitle) = 0;

};

const DWORD kMenuMenuBar  = (1 << 0);
const DWORD kMenuQuadMenu = (1 << 1);

#define MENU_MGR_INTERFACE  Interface_ID(0xadc20bd, 0x7491741d)
/*! \sa  Class IMenu, Class IMenuBarContext, Class IQuadMenuContext, Class Interface.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
To get an interface for calling the methods of this class use
<b>Interface::GetMenuManager()</b>.  */
class IMenuManager : public FPStaticInterface {

public:
    // Add a menu to the manager return false of menu already is registered.
    /*! \remarks This method allows you to add a menu to the manager.
    \par Parameters:
    <b>IMenu* pMenu</b>\n\n
    Points to the menu to register.\n\n
    <b>DWORD flags = 0</b>\n\n
    Not used.
    \return  Returns false if the menu is already registered; true if not. */
    virtual bool RegisterMenu(IMenu* pMenu, DWORD flags = 0) = 0;
    // Remove a menu form the mananger.  return false of the menu is not registered
    /*! \remarks This method allows you to remove a menu form the mananger.
    \par Parameters:
    <b>IMenu* pMenu</b>\n\n
    Points to the menu to unregister.
    \return  FALSE if the menu was not registered, TRUE if successfully
    unregistered. */
    virtual bool UnRegisterMenu(IMenu* pMenu) = 0;

    // Find a menu based on its name
    /*! \remarks This method will return a pointer to a menu based on its
    name.
    \par Parameters:
    <b>MCHAR* pTitle</b>\n\n
    The name of the menu to return.
    \return  A pointer to the menu or NULL if the menu wasn't found. */
    virtual IMenu* FindMenu(const MCHAR* pTitle) = 0;

    // Find a Quad menu based on its name
    /*! \remarks This method will return a pointer to a quad menu based on its
    name.
    \par Parameters:
    <b>MCHAR* pTitle</b>\n\n
    The name of the menu to return.
    \return  A pointer to the quad menu or NULL if the menu wasn't found. */
    virtual IQuadMenu* FindQuadMenu(const MCHAR* pTitle) = 0;

    // Register a new 
    // Register a new context
    /*! \remarks This method allows you to register a new menu bar context
    \par Parameters:
    <b>MenuContextId contextId</b>\n\n
    The menu context ID.\n\n
    <b>MCHAR* pName</b>\n\n
    The name of the menu bar.
    \return  TRUE if the new menu is registered, FALSE if the menu was already
    registered. */
    virtual bool RegisterMenuBarContext(MenuContextId contextId, const MCHAR* pName) = 0;
    /*! \remarks This method allows you to register a new quad menu context.
    \par Parameters:
    <b>MenuContextId contextId</b>\n\n
    The menu context ID.\n\n
    <b>MCHAR* pName</b>\n\n
    The name of the quad menu.
    \return  TRUE if the new quad menu is registered, FALSE if the quad menu
    was already registered. */
    virtual bool RegisterQuadMenuContext(MenuContextId contextId, const MCHAR* pName) = 0;

    /*! \remarks This method returns the number of contexts registered. */
    virtual int NumContexts() = 0;
    /*! \remarks This method returns a pointer to a menu context by the
    specified index.
    \par Parameters:
    <b>int index</b>\n\n
    The index of the menu context to retrieve. */
    virtual IMenuContext* GetContextByIndex(int index) = 0;
    /*! \remarks This method returns a pointer to a menu context by the
    specified menu context ID. This method returns NULL if the context does not
    exist.
    \par Parameters:
    <b>MenuContextId contextId</b>\n\n
    The menu context ID. */
    virtual IMenuContext* GetContext(MenuContextId contextId) = 0;

    // Update MAX's main menu bar after adding sub-menus or menu items
    /*! \remarks This method can be called to update 3ds Max' main menu bar
    after adding sub-menu's or menu items. */
    virtual void UpdateMenuBar() = 0;

    // Load a menu file and update everything.  
    /*! \remarks This method allows you to load a menu file from disk and
    automatically update the UI accordingly.
    \par Parameters:
    <b>MCHAR* pMenuFile</b>\n\n
    The path and filename of the menu file to load.
    \return  TRUE if the menu file was loaded, otherwise FALSE. */
    virtual BOOL LoadMenuFile(const MCHAR* pMenuFile) = 0;

    // Save a menu file.
    /*! \remarks This method allows you to save a menu file to disk.
    \par Parameters:
    <b>MCHAR* pMenuFile</b>\n\n
    The path and filename of the menu file to save.
    \return  TRUE if the menu file was saved, otherwise FALSE. */
    virtual BOOL SaveMenuFile(const MCHAR* pMenuFile) = 0;

    // Get the name of the current menu file.
    /*! \remarks This method returns the file name of the currently loaded and
    active menu file. */
    virtual const MCHAR* GetMenuFile() = 0;

     // Set the given menu to be used as the main menu bar
    /*! \remarks This method allows you to set the main menu bar.
    \par Parameters:
    <b>IMenu* pMenu</b>\n\n
    A pointer to the menu you wish to set as the main menu bar.
    \return  TRUE if it was set successfully. */
    virtual BOOL SetMainMenuBar(IMenu* pMenu) = 0;

    // Set the the viewport right-click menu to be the given quad menu
    /*! \remarks This method allows you to set the viewport right-click menu
    to the specified quad menu.
    \par Parameters:
    <b>IQuadMenuContext::RightClickContext context</b>\n\n
    See \ref rightClickMenuContexts.\n\n
    <b>IQuadMenu* pQuadMenu</b>\n\n
    A pointer to the quad menu you wish to set.
    \return  TRUE if it was set successfully. */
    virtual BOOL SetViewportRightClickMenu(IQuadMenuContext::RightClickContext context,
                                           IQuadMenu* pQuadMenu) = 0;

    // Get the the current viewport right-click menu 
    /*! \remarks This method returns a pointer to the current viewport
    right-click quad menu.
    \par Parameters:
    <b>IQuadMenuContext::RightClickContext context</b>\n\n
    See \ref rightClickMenuContexts. */
    virtual IQuadMenu* GetViewportRightClickMenu(IQuadMenuContext::RightClickContext context) = 0;

    // Set the the given viewport right-click menu to be the named quad menu
    /*! \remarks This method returns a pointer to the main menu bar. */
    virtual IMenu* GetMainMenuBar() = 0;

    // Get/Set the show all wuad flags on a quad menu
    /*! \remarks This method checks if the "Show All Quads" flag is set for a specific
    QuadMenu and will return TRUE if the flag is set or FALSE if the flag is
    not set.
    \par Parameters:
    <b>IQuadMenu* pQuadMenu</b>\n\n
    A pointer to the QuadMenu you wish to check the flag for. */
    virtual bool GetShowAllQuads(IQuadMenu* pQuadMenu) = 0;
    /*! \remarks This method sets the "Show All Quads" flag for a specific QuadMenu.
    \par Parameters:
    <b>IQuadMenu* pQuadMenu</b>\n\n
    A pointer to the QuadMenu you wish to set the flag for.\n\n
    <b>bool showAll</b>\n\n
    TRUE to set the flag to on, FALSE to set the flag off. */
    virtual void SetShowAllQuads(IQuadMenu* pQuadMenu, bool showAll) = 0;

    // Get/Set the name of a quad menu
    /*! \remarks This method returns the name given to a specific QuadMenu as a string.
    \par Parameters:
    <b>IQuadMenu* pQuadMenu</b>\n\n
    A pointer to the QuadMenu for which you wish to retrieve the name. */
    virtual const MCHAR* GetQuadMenuName(IQuadMenu* pQuadMenu) = 0;
    /*! \remarks This method allows you to set the name of a specific QuadMenu.
    \par Parameters:
    <b>IQuadMenu* pQuadMenu</b>\n\n
    A pointer to the QuadMenu for which you wish to set the name.\n\n
    <b>MCHAR* pName</b>\n\n
    The string containing the name for the QuadMenu. */
    virtual void SetQuadMenuName(IQuadMenu* pQuadMenu, const MCHAR* pName) = 0;

	// Function Publishing IDs
	enum { 
#ifndef NO_CUI	// russom - 02/12/02
		   loadMenuFile,
           saveMenuFile,
           getMenuFile,
#endif // NO_CUI
           unRegisterMenu,
           unRegisterQuadMenu,
           registerMenuContext,
           findMenu,
           findQuadMenu,
           updateMenuBar,
           createQuadMenu,
           createMenu,
           createSubMenuItem,
           createSeparatorItem,
           createActionItem,
           setViewportRightClickMenu,
           getViewportRightClickMenu,
           getMainMenuBar,
           setMainMenuBar,
           getShowAllQuads,
           setShowAllQuads,
           numMenus,
           getMenu,
           numQuadMenus,
           getQuadMenu,
           getQuadMenuName,
           setQuadMenuName,
		   createMenuItemFromAction,
    };

	// enumeration IDs
	enum { rightClickContext, };
};

