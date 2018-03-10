//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// FILE:        ActionTable.h
// DESCRIPTION: Action Table definitions
// AUTHOR:      Scott Morrison
// HISTORY:     Created 8 February, 2000, Based on KbdAction.h.
//**************************************************************************/

#pragma once

#include "maxheap.h"
#include "stack.h"
#include "iFnPub.h"
#include "strbasic.h"
#include "actiontableTypedefs.h"
#include "Noncopyable.h"

// forward declarations
class MaxIcon;
class MacroEntry;
class IPoint2;

/// \defgroup ActionTableIdConstatns ActionTableId Constants 
/// These constants identify action tables (ActionTable) managed by 3ds Max. 
///@{
const ActionTableId kActionMainUI           = 0;
const ActionTableId kActionTrackView        = 1;
const ActionTableId kActionMaterialEditor   = 2;
const ActionTableId kActionVideoPost        = 3;
const ActionTableId kActionSchematicView    = 5;
const ActionTableId kActionCommonIReshade   = 6;
const ActionTableId kActionScanlineIReshade = 7;
///@}

class ActionTable;
class IMenu;

/// \defgroup ActionContextIdConstants Action Context IDs
/// IDs that identify action contexts (ActionContext) managed by 3ds Max. Multiple action tables (ActionTable) may share the same action context ID.
//@{
const ActionContextId kActionMainUIContext         = 0;
const ActionContextId kActionTrackViewContext      = 1;
const ActionContextId kActionMaterialEditorContext = 2;
const ActionContextId kActionVideoPostContext      = 3;
const ActionContextId kActionSchematicViewContext  = 5;
const ActionContextId kActionIReshadeContext       = 6;
//@}

/** 
 * Description of a command for building action tables from static data
 * This is a helper structure used for building ActionTables. A static array 
 * of these descriptors is passed to the ActionTable constructor.
 * \version 4.0 
 * \sa  Class ActionTable, Class ActionItem, Class ActionCallback.
 */
struct ActionDescription {

	/** A unique identifier for the command (must be unique per table).
			When an action is executed this is the command ID passed to ActionCallback::ExecuteAction().
	*/
	int mCmdID;

	/** A string resource id that describes the command. */
	int mDescriptionResourceID;

		/** A string resource ID for a short name for the action. This name
		appears in the list of Actions in the Customize User Interface dialog. */
		int mShortNameResourceID;

		/** A string resource ID for the category of an operation. This name appears in
		the Category drop down list in the Customize User Interface dialog. */
		int mCategoryResourceID;

};


/// \name ActionOption Constants
//@{
#define AO_DEFAULT 0x0001 //< Default option command to execute
#define AO_CLOSEDIALOG 0x0002 //< Execute closeDialog option
//@}

#define ACTION_OPTION_INTERFACE Interface_ID(0x3c0276f5, 0x190964f5) 
#define ACTION_OPTION_INTERFACE_OPT2 Interface_ID(0x0011dcdc, 0x0012dcdc)

/** This interface should be implemented if an action item (ActionItem) supports an alternate options command.
 * In this case you would override ActionItem::GetInterface(ACTION_OPTION_INTERFACE), to return an instance of this interface. */
class IActionOptions : public BaseInterface
{
public:
	virtual BOOL ExecuteOptions(DWORD options = AO_DEFAULT) = 0;

	// BaseInterface methods
	CoreExport virtual BaseInterface* GetInterface(Interface_ID id);
	CoreExport virtual Interface_ID GetID();
};

/** Identifier of the ActionItem interface */
#define IID_ACTION_ITEM Interface_ID(0x6fbd5f3c, 0x7c5a32db)

/** Represents an operation contained in an ActionTabls. 
	ActionItem is an abstract class with operations to support various UI operations. 
	The system provides a default implementation of this class that works when the table is 
	built with the ActionTable::BuildActionTable() method. However, developers may want to
	specialize this class for more special-purpose applications. For example, MAXScript does 
	this to export macroScripts to an ActionTable. Methods that are marked as internal should 
	not be used. 
		\version 4.0 
	\sa  Class ActionTable, Class ActionCallback, Structure ActionDescription, Class ActionContext, 
	Class IActionManager, Class DynamicMenu, Class DynamicMenuCallback, Class MAXIcon, Class Interface, class FPMixinInterface
*/
class ActionItem : public FPMixinInterface
{
public:
	/// Constructor
	CoreExport ActionItem();
	/// Destructor
	CoreExport virtual ~ActionItem();

	/** Retrieves a unique identifier for the action.
	This action must be unique in the table, but not does not have to be unique
	between tables. */
	virtual int    GetId() = 0;

	/** Executes the operation associated with the action item. This returns
	a BOOL that indicates if the action was actually executed. If the item is
	disabled, or if the table that owns it is not activated, then it won't
	execute, and returns FALSE.
	\return  TRUE if the action is executed; otherwise FALSE. */
	virtual BOOL ExecuteAction() = 0;

	/** Called by 3ds Max to allow the ActionItem to customize macroRecorder output for this action */
	CoreExport virtual void EmitMacro();

	/** Calls ExecuteAction() and handles macro recording etc. Called by 3ds Max. */
	CoreExport BOOL Execute(); 

	/** Retrieves the text that will be used when the
	ActionItem is on a text button. The text is stored into the buttonText parameter.
	\param[out] buttonText - The retrieved text. */
	virtual void GetButtonText(MSTR& buttonText) = 0;

	/** Retrieves the text to use when the item is on a
	menu (either Quad menu or main menu bar). This can be different from the
	button text. This method is called just before the menu is displayed, so it
	can update the text at that time. For example, the "Undo" menu item in 3ds
	Max's main menu adds the name of the command that will be undone.
	\param[out] menuText - The retrieved text. */
	virtual void GetMenuText(MSTR& menuText) = 0;

	/** Retrieves the text that will be used for tool tips and
	menu help. This is also the string that is displayed for the operation in
	all the lists in the customization dialogs.
	\param[out] descText - The retrieved text. */
	virtual void GetDescriptionText(MSTR& descText) = 0;

	/** Retrieves the text for the category of the
	operation. This is used in the customization dialog to fill the "category"
	drop-down list.
	\param[out] catText - The retrieved text. */
	virtual void GetCategoryText(MSTR& catText) = 0;

	/** Returns whether a menu item is in a checked state, or if a button 
	is in a pressed state. 
	For menus, this means that this function will return TRUE if a check mark 
	appears next to the item. If the item is on a button, this is used to determine of
	the button is in the "Pressed" state. Note that button states are
	automatically updated on selection change and command mode changes. If your
	plug-in performs an operation that requires the CUI buttons to be redrawn,
	you need to call the method CUIFrameMgr::SetMacroButtonStates(TRUE).
	\return TRUE if menu item is checked, or if a button is 'pressed'. FALSE otherwise*/
	virtual BOOL IsChecked() = 0;

	/** Determines if an item is visible on a context menu. If it
	returns FALSE, then the item is not included in the menu. This can be used
	to create items that a context sensitive. For example, you may want an item
	to appear on a menu only when the selected object is of a particular type.
	To do this, you have this method check the class id of the current
	selection. */
	virtual BOOL IsItemVisible() = 0;

	/** Determines if the operation is available to the user. 
	If this method returns FALSE, and the ActionItem it is on a 
	menu, or button, the item is grayed out. If it assigned to a keyboard 
	shortcut, then it will not execute the operation if invoked. If your plugin 
	performs an operation that requires the CUI buttons to be redrawn, you need 
	to call the method CUIFrameMgr::SetMacroButtonStates(TRUE)
	\return  TRUE for enabled; FALSE for disabled. */
	virtual BOOL IsEnabled() = 0;
		
	/** Returns an icon associated with an ActionItem. 
	If an icon is available for the ActionItem, then you should override this function
	to return or else returns NULL. The icon is
	used on CUI buttons, and in the list of operations in the customization
	dialogs. */
	virtual MaxIcon* GetIcon() = 0;

	/** Called by 3ds Max to delete the ActionItem. This normally happens when
	the table that owns it is deleted. */
	virtual void DeleteThis() = 0;

	/** Returns a pointer to the table that owns the ActionItem.
	An item can only be owned by a single table.
	\par Default Implementation
	\code
	{ return mpTable; }
	\endcode */
	CoreExport virtual ActionTable* GetTable();

	/** Sets the table that owns the item. Used internally. May be
	used if you implement a custom sub-class of ActionItem.
	\param pTable - Points to the table to set.
	\par Default Implementation:
	\code 
	{ mpTable = pTable; }
	\endcode */
	CoreExport virtual void SetTable(ActionTable* pTable);

	/** Returns the string that describes all the keyboard shortcuts
	associated with the item. This will look something like "Alt+A" or "C,
	Shift+Alt+Q". 
	\return NULL if no keyboard shortcut is associated with the item. */
	CoreExport MCHAR* GetShortcutString();

	/** Returns the representation of the macroScript for the item,
	if it's implemented by a macroScript, it returns NULL otherwise.
	\return A pointer to a MacroEntry
	\par Default Implementation
	\code 
	{ return NULL; }
	\endcode */
	CoreExport virtual MacroEntry* GetMacroScript();

	/** Determines if a menu is created or if an action takes place.    
	\return If this method returns TRUE, then the ActionItem creates a menu. If it
	returns FALSE then an action is performed.
	\par Default Implementation
	\code 
	{ return FALSE; }
	\endcode */
	CoreExport virtual BOOL IsDynamicMenu();

	/** This method may be called after an action item is created to
	tell the system that it is a dynamic menu action. 
	\Note Dynamic menus may be added to the quad menus procedurally (via 
	the IMenuManager API) or 'manually'. */
	CoreExport virtual void SetIsDynamicMenu();

	/** If the ActionItem does produce a menu, this method is called
	to return a pointer to the menu. See class DynamicMenu for an easy way to
	produce these menus.
	\param HWND hwnd -  If the menu is requested by a right-click quad menu, 
	then this hwnd is the handle of the window where the click occurred. 
	If the item is used from a menu bar, this hwnd will be NULL.
	\param m -  If the menu is requested by a right-click quad menu, then this parameter is
	the point in the window where the user clicked.
	\return  A pointer to the menu. 
	\par Default Implementation:
	\code 
	{ return NULL; }
	\endcode*/
	CoreExport virtual IMenu* GetDynamicMenu(HWND hwnd, IPoint2& m);

	/// ActionItems that are deleted after they execute should return TRUE.
	CoreExport virtual BOOL IsDynamicAction();

	// FPMixinInterface methods
	CoreExport virtual FPInterfaceDesc* GetDescByID(Interface_ID id);
	CoreExport virtual FPInterfaceDesc* GetDesc();

	// BaseInterface methods
	CoreExport virtual BaseInterface* GetInterface(Interface_ID id);
	CoreExport virtual Interface_ID GetID();

	/// Function IDs for function published (Fps) methods and properties
	enum ActionItemFps
	{
		kGetId,
		kIsVisible,
		kIsEnabled,
		kIsChecked,
		kGetDescription,
		kGetCategory,
		kGetButtonText,
		kGetMenuText,
	};

protected:
	#pragma warning(push)
	#pragma warning(disable:4100)
	BEGIN_FUNCTION_MAP
		RO_PROP_FN(kGetId, GetId, TYPE_DWORD);
		RO_PROP_FN(kIsVisible, IsItemVisible, TYPE_BOOL);
		RO_PROP_FN(kIsEnabled, IsEnabled, TYPE_BOOL);
		RO_PROP_FN(kIsChecked, IsChecked, TYPE_BOOL);
		VFN_1(kGetDescription, GetDescriptionText, TYPE_TSTR_BR);
		VFN_1(kGetCategory, GetCategoryText, TYPE_TSTR_BR);
		VFN_1(kGetButtonText, GetButtonText, TYPE_TSTR_BR);
		VFN_1(kGetMenuText, GetMenuText, TYPE_TSTR_BR);
	END_FUNCTION_MAP
	#pragma warning(pop)

	/// Points to the table that owns the action. 
	/// \note This does not 'own' the memory for the ActionTable pointer
	ActionTable* mpTable;
};

/// \deprecated To be removed from the SDK.
#define ACTIONITEM_STANDIN_INTERFACE Interface_ID(0x108e1314, 0x5aff3138)

/// \deprecated To be removed from the SDK.
class IActionItemStandin : public BaseInterface
{
public:
	virtual void          SetPersistentActionId(MSTR idString) = 0;
	virtual MSTR&         GetPersistentActionId() = 0;
	virtual void          SetActionTableId( ActionTableId id ) = 0;
	virtual ActionTableId GetActionTableId() = 0;
	virtual MSTR&         GetPrefixString() = 0;

	// BaseInterface methods
	CoreExport virtual BaseInterface* GetInterface(Interface_ID id);
	CoreExport virtual Interface_ID GetID();
};

/// \deprecated To be removed from the SDK.
inline IActionItemStandin* GetIActionItemStandin(ActionItem* a) { return (IActionItemStandin*)a->GetInterface(ACTIONITEM_STANDIN_INTERFACE); }

/// \deprecated To be removed from the SDK.
class ActionItemStandin: public ActionItem, public IActionItemStandin
{
public:
	CoreExport ActionItemStandin(int cmdId );
	CoreExport virtual ~ActionItemStandin();

	CoreExport virtual int  GetId() { return mCmdId; }
	CoreExport virtual void SetId(int id) { mCmdId = id; }

	CoreExport virtual MCHAR*  GetDescription() { return mName.data();}
	CoreExport virtual void    SetDescription(MCHAR* pDesc) { mName = pDesc; }

	CoreExport virtual MCHAR*  GetShortName() { return mName.data();}
	CoreExport virtual void    SetShortName(MCHAR* pShortName) { mName = pShortName; }

	CoreExport virtual MCHAR*  GetCategory() { return mName.data();}
	CoreExport virtual void    SetCategory(MCHAR* pCategory) { mName = pCategory; }

	CoreExport virtual MaxIcon* GetIcon() { return NULL; };

	CoreExport virtual BOOL ExecuteAction() { return FALSE; };

	CoreExport virtual void GetButtonText(MSTR& buttonText) { buttonText = mName; };
	CoreExport virtual void GetMenuText(MSTR& menuText) { menuText = mName; };
	CoreExport virtual void GetDescriptionText(MSTR& descText) { descText = mName; };
	CoreExport virtual void GetCategoryText(MSTR& catText) { catText = mName; };

	CoreExport virtual BOOL IsChecked() { return FALSE; };
	CoreExport virtual BOOL IsItemVisible() { return TRUE; };
	CoreExport virtual BOOL IsEnabled() { return FALSE; };

	CoreExport virtual BOOL IsDynamicMenu() { return FALSE; }
	CoreExport virtual void SetIsDynamicMenu() { };
	#pragma warning(push)
	#pragma warning(disable:4100)
	CoreExport virtual IMenu* GetDynamicMenu(HWND hwnd, IPoint2& m) { return NULL; };
	#pragma warning(pop)

	CoreExport void DeleteThis();

	CoreExport virtual BaseInterface* GetInterface(Interface_ID id);

	CoreExport virtual void SetPersistentActionId(MSTR idString);
	CoreExport virtual MSTR& GetPersistentActionId() { return mPersistentActionId; };
	CoreExport virtual void SetActionTableId( ActionTableId id ) { mId = id; };
	CoreExport virtual ActionTableId GetActionTableId() { return mId; };
	CoreExport virtual MSTR& GetPrefixString() { return mPrefix; };

protected:
	int   mCmdId;
	MSTR  mName;
	MSTR  mPersistentActionId;
	MSTR  mPrefix;
	ActionTableId mId;
};

class ActionCallback;

/** Identifier of the ActionTable interface */
#define IID_ACTION_TABLE Interface_ID(0x27ba191e, 0x4d391886)

/** 
A table of action items that can be used by plug-ins to export 
tables of action items that can be used by the UI to attach to keyboard 
shorcuts, assigned to toolbar buttons, and add to menus. 
3ds Max's core code exports several ActionTables for built-in operations in 3ds Max.
Plug-ins can also export their own action tables via methods available in
ClassDesc.

Every ActionTable also has an ActionContextId associated with it. This
ActionContextId can be shared with other tables.
When assigning keyboard shortcuts to items in an ActionTable, tables that share
a unique context id are forced to have unique shortcuts. Tables with different
context ids can have overlapping keyboard shortcut assignments.

All methods of this class are implemented by the system. Note that this
class may be sub-classed if required and that many functions can be overridden. 
See the Advanced Topics section
<a href="ms-its:3dsMaxSDK.chm::/ui_customization_root.html">UI
Customization</a> for details on sub-classing this class and ActionItem. For
details on implementing an ActionTable please refer to the sample
at /maxsdk/samples/modifiers/ffd.
\version 4.0 
\sa Class FPMixinInterface,  Class ClassDesc, Structure ActionDescription, Class ActionItem, Class ActionCallback, Class ActionContext, Class IActionManager, Class DynamicMenu, Class DynamicMenuCallback, Class Interface.
*/
class ActionTable : public FPMixinInterface, public MaxSDK::Util::Noncopyable 
{
public:
	/** This constructor builds the action table using
	an array of descriptors. It takes the ID of the table, the context id, a
	name for the table, a windows accelerator table that gives default keyboard
	assignments for the operations, the number of items, the table of operation
	descriptions, and the instance of the module where the string resources in
	the table are stored. When the action table is constructed the action context ID
	must be registered with the system using the IActionManager::RegisterActionContext() method.

	\param id          A unique ID for the ActionTable. Every ActionTable has a unique 32-bit
	integer ID. For new tables exported by plug-ins, the developer should
	choose a random 32-bit integer. You can use one of the two DWORDS generated by 
	the gencid.exe program as an ActionTableId.
	\param contextId    The ActionContextID associated with this table. Several tables may share
	the same ActionContextID.
	\param name         The name for the ActionTable.
	\param hDefaults   The handle of the a windows accelerator table that gives default keyboard
	assignments for the operations.
	\param numIds       The number of items in the description array 
	\param pOps         Points to the array of the operator descriptors.
	\param hInst        The handle to the instance of the module where the string resources in the
	array of operator descriptors are stored. 
	*/
	CoreExport ActionTable(ActionTableId id,
												 ActionContextId contextId,
												 MSTR& name,
												 HACCEL hDefaults,
												 int numIds,
												 ActionDescription* pOps,
												 HINSTANCE hInst);
	/** This constructor builds a new empty action table
	with the given ID, context ID and name. You then need to add ActionItems to
	the table separately using the AppendOperation() method.

	\param id           The unique ID for the ActionTable.
	\param contextId    The ActionContextID associated with this table. Several tables may share
											the same ActionContextID.
	\param name         The name for the ActionTable. */
	CoreExport ActionTable(ActionTableId id,
												 ActionContextId contextId,
												 MSTR& name);

	/** Deletes all the operations maintained by the
	table and deletes the keyboard accelerator table if in use. */
	CoreExport virtual ~ActionTable();

	/// \name Keyboard Accelerator Functions
	//@{
	/** Returns the handle of the current keyboard accelerator for the table. */
	CoreExport virtual HACCEL GetHAccel();

	/** Sets the current keyboard accelerator for the table. */
	CoreExport virtual void SetHAccel(HACCEL hAccel);

	/** Gets the default keyboard accelerator table used when
	the user has not assigned any accelerators. */
	CoreExport virtual HACCEL GetDefaultHAccel();

	/** Sets the default keyboard accelerator table used when
	the user has not assigned any accelerators. */
	CoreExport virtual void SetDefaultHAccel(HACCEL accel);
	//@}

	/** Returns the name of the action table */
	CoreExport virtual MSTR& GetName();

	/** Returns the ActionTableID */
	CoreExport virtual ActionTableId GetId();

	/** Returns the ActionContextId. */
	CoreExport virtual ActionContextId GetContextId();

	/** Get the current callback associated with this table. Returns
	NULL if the table is not active. */
	CoreExport virtual ActionCallback* GetCallback();

	/** Sets the callback object used by this ActionTable.    
	\param pCallback Points to the callback to set. */
	CoreExport virtual void SetCallback(ActionCallback* pCallback);

	/** Returns the number of ActionItems in the table. */
	CoreExport virtual int Count();

	/** Returns a pointer to the nth ActionItem.
	\param i The zero based index in the list of ActionItems. */
	CoreExport virtual ActionItem* operator[](int i);

	/** Returns a pointer to the ActionItem associated with the
	command ID passed.
	\param cmdId The command ID. */
	CoreExport virtual ActionItem* GetAction(int cmdId);

	/** Returns a pointer to the nth ActionItem using the 
	same algorithm as the operator[].
	\param index The zero based index of the action. 
	\return A pointer to the ActionItem */
	CoreExport virtual ActionItem* GetActionByIndex(int index); 

	/** Appends an ActionItem to the table.
	\param pAction A pointer the ActionItem to append. */
	CoreExport virtual void AppendOperation(ActionItem* pAction);
	
	/** Removes an operation from the table
	\param pAction Points to the ActionItem to delete.
	\return TRUE if the operation was deleted; FALSE if it could not be found
	and wasn't. */
	CoreExport virtual BOOL DeleteOperation(ActionItem* pAction);

	/** Deletes this ActionItem.
	\par Default implementation:
	\code
	{ delete this; }
	\endcode */
	CoreExport virtual void DeleteThis(); 

	/** Retrieves the text that will be used when the
	ActionItem is on a text button.
	\param cmdId The unique ID of the command whose button text is retrieved.
	\param[out] buttonText The retrieved text.
	\return TRUE if the command is in the table; otherwise FALSE. */
	CoreExport virtual BOOL GetButtonText(int cmdId, MSTR& buttonText);

	/** Retrieves the text to use when the item is on a
	menu (either Quad menu or main menu bar). This can be different from the
	button text.    
	\param cmdId  The unique ID of the command whose menu text is retrieved.
	\param menuText Storage for the text.
	\return  TRUE if the command is in the table; otherwise FALSE.
	\par Default Implementation:
	\code
	{ return GetButtonText(cmdId, menuText); }
	\endcode */
	CoreExport virtual BOOL GetMenuText(int cmdId, MSTR& menuText);

	/** Retrieves the text that is used for tool tips and
	menu help. This is also the string that is displayed for the operation in
	all the lists in the customization dialogs.
	\param cmdId The unique ID of the command whose description text is retrieved.
	\param descText The retrieved text.
	\return  TRUE if the command is in the table; otherwise FALSE. */
	CoreExport virtual BOOL GetDescriptionText(int cmdId, MSTR& descText);

	/** Returns TRUE if the menu item should be checked or a CUI
	button should be in the pressed state.
	\param cmdId The unique ID of the command.
	\par Default Implementation:
	\code
	{ return FALSE; }
	\endcode */
	CoreExport virtual BOOL IsChecked(int cmdId);
	
	/** Determines if an item is to be visible on a menu.
	Returns TRUE if visible; FALSE if not.
	\param cmdId The unique ID of the command.
	\par Default Implementation:
	\code
	{ return TRUE; }
	\endcode */
	CoreExport virtual BOOL IsItemVisible(int cmdId);

	/** Determines if the operation is currently enabled
	and available. Returns TRUE if enabled; FALSE if disabled.
	\param cmdId The unique ID of the command.
	\par Default Implementation:
	\code 
	{ return TRUE; }
	\endcode */
	CoreExport virtual BOOL IsEnabled(int cmdId);

	/** Converts an action item identifier to a string representation
	The textual representation of action item ids are used in .CUI and .KBD files.
	The default implementation is to convert the integer action item ID directly to a string.
	\param cmdId The unique command or action item ID.
	\param idString The string where the action item ID is written to. */
	CoreExport virtual void WritePersistentActionId(int cmdId, MSTR& idString);
	
	/** Converts an action table string id to the action items numeric id.
	This method can be used to convert string based action item identifiers read from a 
	*.CUI or *.KBD file to a numeric identifier. 
	It's default implementation expects the string passed in as argument to contain one number.
	\param idString The action ID string.
	\return Returns -1 if the id could not be converted. */
	CoreExport virtual int ReadPersistentActionId(MSTR& idString);

	/** Returns an optional icon for the command, or NULL if there is none.
	\param cmdID The unique ID of the command. */
	CoreExport virtual MaxIcon* GetIcon(int cmdId);

	/** Fills the action table with the given action descriptions.
	\pararm hDefaults The handle of the a windows accelerator table that provides keyboard
	assignments for the operations.
	\param numIds The number of ID's to add to the action table.
	\param pOps The array of action descriptions to build the table from.
	\param hInst The handle to the instance of the module. */
	CoreExport void BuildActionTable(HACCEL hDefaults,
															int numIds,
															ActionDescription* pOps,
															HINSTANCE hInst);

	/** Gets the action assigned to the given accelerator, if any
	\param accel The accelerator key you wish to check the assignment for. */
	CoreExport virtual ActionItem* GetCurrentAssignment(ACCEL accel);
	
	/** Assigns the command to the given accelerator. Also removes any
	previous assignment to that accelerator.
	\param cmdId The command ID.
	\param accel The accelerator key you wish to assign. */
	CoreExport virtual void AssignKey(int cmdId, ACCEL accel);

	/** Removes the given assignment from the shortcut table.
	\param accel The accelerator key you wish to remove from the shortcut table. */
	CoreExport virtual void RemoveShortcutFromTable(ACCEL accel);

	// From FPMixinInterface
	CoreExport FPInterfaceDesc* GetDescByID(Interface_ID id);
	CoreExport FPInterfaceDesc* GetDesc();

	// BaseInterface methods
	CoreExport virtual BaseInterface* GetInterface(Interface_ID id);
	CoreExport virtual Interface_ID GetID();

	// Function publishing
	enum ActionTableFps
	{
		kGetId,
		kGetName,
		kGetContextId,
		kGetContextName,
		kNumActionItems,
		kGetActionItem,
	};

protected:
	#pragma warning(push)
	#pragma warning(disable:4100)
	BEGIN_FUNCTION_MAP
		RO_PROP_FN(kGetId, GetId, TYPE_DWORD);
		RO_PROP_FN(kGetName, GetName, TYPE_STRING);
		RO_PROP_FN(kGetContextId, GetContextId, TYPE_DWORD);
		RO_PROP_FN(kGetContextName, fpGetContextName, TYPE_STRING);
		RO_PROP_FN(kNumActionItems, Count, TYPE_INT);
		FN_1(kGetActionItem, TYPE_INTERFACE, GetActionByIndex, TYPE_INDEX);
	END_FUNCTION_MAP
	#pragma warning(pop)

	/** Used internally by function publishing */ 
	CoreExport const MCHAR* fpGetContextName() const;

protected:
	// Descriptors of all operations that can have Actions
	Tab<ActionItem*>  mOps; 

private:
	// These values are set by the plug-in to describe a action table

	// Unique identifier of table (like a class id)
	ActionTableId  mId;

	// An identifier to group tables use the same context.  Tables with the
	// same context cannot have overlapping keyboard shortcuts.
	ActionContextId mContextId;

	// Name to use in preference dlg drop-down
	MSTR mName;

	// The windows accelerator table in use when no keyboard shortcuts saved
	HACCEL mhDefaultAccel;
	// The windows accelerator table in use
	HACCEL mhAccel;

	// The currently active callback
	ActionCallback* mpCallback;
};


/** 
3ds Max uses the ExecuteAction() method of an instance of this class 
when an ActionItem is executed. Developers need to override this class and pass an
instance of it to the system when they activate an ActionTable using the method
IActionManager::ActivateActionTable().

\version 4.0
\sa  Class BaseInterfaceServer,  Class ActionTable,  Class ActionItem, Class ActionContext, Class IActionManager, Class DynamicMenu, Class DynamicMenuCallback, Class Interface.
*/
class ActionCallback : public BaseInterfaceServer {
public:
	/** Constructor */
	CoreExport ActionCallback();

	/** Virtual destructor */
	CoreExport virtual ~ActionCallback();
		
	/** Called by 3ds Max to execute the operation associated with an action item
	
	\param id The ID of the item to execute.
	\return  This returns a BOOL that indicates if the action was actually
	executed. If the item is disabled, or if the table that owns it is not
	activated, then it won't execute, and returns FALSE. If it does execute
	then TRUE is returned.
	\par Default Implementation:
	\code 
	{ return FALSE; }
	\endcode 
	*/
	CoreExport virtual BOOL ExecuteAction(int id);

	/** Called by 3ds Max if an action item declares itself as a
	dynamic menu. 
	
	\param id The item ID which is passed to the DynamicMenuCallback::MenuItemSelected()
	\param hwnd If the menu is requested by a right-click quad menu, then hwnd is the
	window where the click occurred. If the item is used from a menu bar, then
	hwnd will be NULL.
	\param m If the menu is requested by a right-click quad menu, then this will be the
	point in the window where the user clicked.
	\par Default Implementation:
	\code
	{ return NULL; }
	\endcode */
	CoreExport virtual IMenu* GetDynamicMenu(int id, HWND hwnd, IPoint2& m);

	/** Returns a pointer to the ActionTable the callback uses.*/
	CoreExport ActionTable* GetTable();
	
	/** Sets the ActionTable the callback uses. 
	\param pTable Points to the ActionTable the callback uses. */
	CoreExport void SetTable(ActionTable* pTable);

private:
	/// Points to the table that uses this ActionCallback.
	ActionTable *mpTable;
};

/** 
An ActionContext is an identifer of a group of keyboard shortcuts. Examples are
the Main 3ds Max UI, Track View, and the Editable Mesh. They are registered
using IActionManager::RegisterActionContext().

\version 4.0
\sa  Class ActionTable, Class ActionItem, Class ActionCallback, Class IActionManager, Class DynamicMenu, Class DynamicMenuCallback, Class Interface.
*/
class ActionContext: public MaxHeapOperators {
public:
	/** Initiatilize the context ID and the name from the passed arguments 
	\param contextId The ID for the ActionContext.
	\param pName The name for the ActionContext. */
	ActionContext(ActionContextId contextId, MCHAR *pName)
			{ mContextId = contextId; mName = pName; mActive = true; }

	/** Returns the name of this ActionContext. */
	MCHAR* GetName() { return mName.data(); }
	
	/** Returns the ID of this ActionContext. */
	ActionContextId GetContextId() { return mContextId; }

	/** Returns true if this ActionContext is active; otherwise
	false. An active ActionContext means that it uses its keyboard
	accelerators. This corresponds to the "Active" checkbox in the keyboard
	customization UI. */
	bool IsActive() { return mActive; }
	
	/** Sets the active state of this ActionContext.
	\param active true for active; false for inactive. */
	void SetActive(bool active) { mActive = active; }
		
private:
	ActionContextId  mContextId;
	MSTR             mName;
	bool             mActive;
};

/** Used to retrieve a handle to a instance of a global class that implements the 
IActionManager interface */
#define ACTION_MGR_INTERFACE  Interface_ID(0x4bb71a79, 0x4e531e4f)

/** 
Manages a set of ActionTables, ActionCallbacks and ActionContext.
The manager handles the keyboard accelerator tables for each ActionTable as
well. You can get a pointer to a class implementing this interface
using Interface::GetActionManager().  
\version 4.0
\sa  Class ActionTable, Class ClassDesc, Class ActionItem, Class ActionCallback, Class ActionContext, Class DynamicMenu, Class DynamicMenuCallback, Class Interface.
*/
class IActionManager : public FPStaticInterface  
{
public:
	/** Register an action table with the manager. 
	Note that plug-ins that expose their action tables via their ClassDesc::NumActionTables() method. 
	do not need to register their action tables explicitely, as 3ds Max will do it for them. 
	Also note that plug-ins do not need to unregister or destroy their action tables and action items, 
	as 3ds Max will take care of this when it shuts down.
	Also note that an action table needs to be activated once registered. 
	For more information see IActionManager::ActivateActionTable()
	See <a href="class_class_desc.html#A_GM_cldesc_actiontable">ClassDesc Action Table Methods</a>.
	\param pTable Points to the Action Table to register. */
	virtual void RegisterActionTable(ActionTable* pTable) = 0;

	/** Returns the number of ActionTables. */
	virtual int NumActionTables() = 0;
	
	/** Returns a pointer to the nth ActionTable.
	\param i The zero based index of the table. */
	virtual ActionTable* GetTable(int i) = 0;

	/** Activate the action table. 
	Action tables need to be activated by their owners. 
	Some plug-ins (for instance Modifiers or Geometric Objects) may only want to
	activate the table when they are being edited in the command panel (between
	BeginEditParams() and EndEditParams()). Others, for instance Global Utility
	Plug-ins, may wish to do so when they are initially loaded so the actions
	are always available.
	Note that if this method is called multiple times, only the callback from
	the last call will be used.
	\param pCallback Points to the callback object which is responsible for executing the
	action.
	\param id This is the ID of the table to activate.
	\return TRUE if the action table was activated. FALSE if the table is
	already active or doesn't exist. */
	virtual int ActivateActionTable(ActionCallback* pCallback, ActionTableId id) = 0;
	
	/** Deactivates the action table. After
	the table is deactivated (for example in EndEditParams()) the callback
	object can be deleted. Tables are initially active, please do not call this
	method without a preceding call to <b>ActivateActionTable()</b>.
	\param pCallback Points to the callback object responsible for executing the action. Pass
	the same callback that was originally passed to
	ActivateActionTable() and do not set this to NULL.
	\param id The ID of the table to deactivate.
	\return  TRUE if the action table was deactivated. FALSE if the table was
	already deactivated or doesn't exist. */
	virtual int DeactivateActionTable(ActionCallback* pCallback, ActionTableId id) = 0;

	/** Returns a pointer to an action table. 
	\param id  The ID of the table to find. */
	virtual ActionTable* FindTable(ActionTableId id) = 0;

	/** Retrieves the string that describes the keyboard shortcut for
	the specified ActionItem. 
	\param tableId The ID of the action table.
	\param commandId The ID of the command for the action.
	\param[out] buf String to contain the retrieved text.
	\return TRUE if found; FALSE if not found. */
	virtual BOOL GetShortcutString(ActionTableId tableId, int commandId, MCHAR* buf) = 0;
	
	/** Retrieves a string that descibes the specified operation from the action table whose ID is passed.
	\param tableId The ID of the action table.
	\param commandId The ID of the command.
	\param MCHAR* buf Points to storage for the description string.
	\return TRUE if the string was returned; FALSE if not. */
	virtual BOOL GetActionDescription(ActionTableId tableId, int commandId, MCHAR* buf) = 0;

	/** Registers an action context. This is called when you create the
			action table that uses this context.
	\param contextId The context ID.
	\param pName The name for the action context.
	\return  If the specified action context is already registered FALSE is
	returned; otherwise TRUE is returned. */
	virtual BOOL RegisterActionContext(ActionContextId contextId, MCHAR* pName) = 0;
	
	/** Returns the number of ActionContexts. */
	virtual int NumActionContexts() = 0;
	
	/** Returns a pointer to the nth ActionContext.
	\param i The zero based index of the ActionContext. */
	virtual ActionContext* GetActionContext(int i) = 0;
	
	/** Returns a pointer to the specified ActionContext.
	\param contextId The ID of the context to find
	\return A poitner to the ActionContext or NULL if it not found.
	*/
	virtual ActionContext* FindContext(ActionContextId contextId) = 0;

	/** 
	Checks if an ActionContext is active or not.
	\param contextId Specifies the context to check. 
	\return Returns TRUE if the specified context is active; otherwise
	FALSE.
	*/
	virtual BOOL IsContextActive(ActionContextId contextId) = 0;

	/// \name Internal Methods 
	///@{
	/// \internal
	virtual MCHAR* GetShortcutFile() = 0;
	/// \internal
	virtual MCHAR* GetShortcutDir() = 0;
	/// \internal
	virtual int IdToIndex(ActionTableId id) = 0;
	/// \internal
	virtual void SaveAllContextsToINI() = 0;
	/// \internal
	virtual int MakeActionSetCurrent(MCHAR* pDir, MCHAR* pFile) = 0;
	/// \internal
	virtual int LoadAccelConfig(LPACCEL *accel, int *cts, ActionTableId tableId = -1, BOOL forceDefault = FALSE) = 0;
	/// \internal
	virtual int SaveAccelConfig(LPACCEL *accel, int *cts) = 0;
	/// \internal
	virtual int GetCurrentActionSet(MCHAR *buf) = 0;
	/// \internal
	virtual BOOL SaveKeyboardFile(MCHAR* pFileName) = 0;
	/// \internal
	virtual BOOL LoadKeyboardFile(MCHAR* pFileName) = 0;
	/// \internal
	virtual MCHAR* GetKeyboardFile() = 0;
	////@}

	/// IDs for function published (Fps) methods and properties
	enum ActionManagerFps
	{
		executeAction,
#ifndef NO_CUI	// russom - 02/12/02
		saveKeyboardFile,
		loadKeyboardFile,
		getKeyboardFile,
#endif // NO_CUI
		numActionTables,
		getActionTable,
		getActionContext,
		};
};

/**
	A mixin-interface extension to IActionManager which allows a client to dispatch a Message back to the application
	if it is not handled by the focused control.
	Can be accessed as follows:

	\code
	Interface10* ip = GetCOREInterface10();
	IActionManager* actionMgr = ip-> GetActionManager();
	IActionManagerExt* ext = 
		static_cast<IActionManagerExt*>(actionMgr->GetInterface(IActionManagerExt::kActionMgr10InterfaceID));
	\endcode
	\remarks This interface is not intended for extension by 3rd-party developers.
	\see IActionManager
*/
class IActionManager10 : public BaseInterface, public MaxSDK::Util::Noncopyable {

public:
	/// The ID for the interface.  
	CoreExport static const Interface_ID kActionMgr10InterfaceID;

	/**	Dispatches a windows Message structure to the application.
			Takes a MSG structure and dispatches it to the main application proc directly.
	This function can be used to dispatch WM_KEYDOWN events to the application for processing.
	If a control or dialog is capturing input messages, then this function is useful to delegate
	message handling to the main application window proc in the case where the message is not handled
	by the focused control hierarchy or the activated (modeless) dialog.
	\param[in, out] message The message to dispatch to the application.
	\return true if the message is handled by the application
	*/
	virtual bool DispatchMessageToApplication(MSG* message) = 0;

	/// Returns the interface ID
	CoreExport virtual Interface_ID GetID(); 

protected:
	/// The constructor / destructor are defined in Core.dll, but bear in mind that this interface is not
	/// intended for extension.
	CoreExport IActionManager10();
	/// The constructor / destructor are defined in Core.dll, but bear in mind that this interface is not
	/// intended for extension.
	CoreExport virtual ~IActionManager10();

};


/** 
When a user makes a selection from a dynamic menu 3ds Max will call DynamicMenuCallback::MenuItemSelected() 
to process that selection.  
\version 4.0
\sa  Class DynamicMenu, Class IMenu, Class ActionTable, Class ActionItem, Class ActionCallback, Class ActionContext, Class IActionManager, Class Interface.
*/
class DynamicMenuCallback: public MaxHeapOperators {
public:
		/** Called to process the user's menu selection.
		\par Parameters:
		<b>int itemId</b>
		The ID of the item selected. */
		virtual void MenuItemSelected(int itemId) = 0;
};

/** 
Provides an easy mechanism for plug-ins to produce the menu needed in the
ActionItem::GetDynamicMenu() method. The constructor of this class is
used to create the menu and the ActionItem::GetMenu() method returns the appropriate 
IMenu pointer.  
\version 4.0
\sa  Class DynamicMenuCallback,  Class IMenu, Class ActionTable, Class ActionItem, Class ActionCallback, Class ActionContext, Class IActionManager, Class Interface.
*/
class DynamicMenu: public MaxHeapOperators {
public:

		/** Constructor.
		\param pCallback Points to the instance of the DynamicMenuCallback class that handles the
		menu selection. */
		CoreExport DynamicMenu(DynamicMenuCallback* pCallback);

		/** 
		This is the value returned from ActionItem::GetDynamicMenu(). This method may be called
		after menu creation to get a pointer to the IMenu created.  
		\return A pointer to Imenu that can be returned from ActionItem::GetDynamicMenu(*) 
		*/
		CoreExport IMenu* GetMenu();

		/// Constants used in AddItem
		enum DynamicMenuFlags {
				kDisabled   = 1 << 0, //< Item is disabled (can't be selected)
				kChecked    = 1 << 1, //< Item has a check mark beside it or is pressed 
				kSeparator  = 1 << 2, //< Item is a seperator between the previous menu item and the next one.
		};

		/** Adds an item to the dynamic menu.
		\param flags One or more of the DynamicMenuFlags.
		\param itemId The ID for the menu item.
		\param pItemTitle The name to appear for the menu item. */
		CoreExport void AddItem(DWORD flags, UINT itemId, MCHAR* pItemTitle);

		/** This begins a new sub menu. Items added after this call will
		appear as sub choices of this one until EndSubMenu() is called.
		\param pTitle The name to appear for the sub menu item. */
		CoreExport void BeginSubMenu(MCHAR* pTitle);

		/** Ends construction of a sub menu. Items added after this call will appear
		as they did prior to calling BeginSubMenu(). */
		CoreExport void EndSubMenu();

private:

		Stack<IMenu*> mMenuStack;
		DynamicMenuCallback *mpCallback;
};

