/**********************************************************************
 *<
	FILE: UIExtend.h

	DESCRIPTION: MaxScript user interface extensions

	CREATED BY: Ravi Karra, 1998

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "..\kernel\value.h"
// forward declarations
class RCMenu;
class HashTable;

#define MF_SUBMENU_START	(MF_SEPARATOR+10)
#define MF_SUBMENU_END		(MF_SEPARATOR+11)

visible_class_debug_ok (MenuItem)

class MenuItem : public Value
{	
public:
	Value	*name, *caption, *flt_fn;
	Value	**keyparms;
	RCMenu	*menu;
	HMENU	hmenu;
	UINT	hmenu_index;

	int		keyparm_count;
	UINT	flags;
	
			MenuItem (RCMenu *m, Value* n, Value* c, Value	**keyparms, int keyparm_count, UINT f=0);
			MenuItem () {menu= NULL; name=caption=NULL; keyparms=NULL; flags=keyparm_count=hmenu_index=0; hmenu = NULL;} 
			~MenuItem ();

	ScripterExport void	setup_params();
	void	collect() { delete this; }
	void	gc_trace();
	ScripterExport void sprin1(CharStream* s);

	virtual	Value*	get_property(Value** arg_list, int count);
	virtual	Value*	set_property(Value** arg_list, int count);
};


class MSRightClickMenu : public RightClickMenu
{
public:
	RCMenu	*menu;
	void	Init(RightClickMenuManager* manager, HWND hWnd, IPoint2 m);
	void	Selected(UINT id);
	Value*	call_filt_fn(Value* fn);
};


visible_class_debug_ok (RCMenu)

class RCMenu : public Value
{
public:
	Value*		name;						// menu name
	HashTable*  local_scope;				// local name space	
	MenuItem**	items;						// menu item array
	int			item_count;					//   "	  "  count
	Value**		locals;						// local var array	
	Value**		local_inits;				//   "    "    "  init vals	
	int			local_count;				//   "    "  count	
	HashTable*	handlers;					// handler tables	
	short		flags;						// menu flags
	BOOL		init_values;				// whether to init ctrl/local values on (re)open 
	BOOL		end_rcmenu_mode;			// signals end of rcmenu mode
	MSRightClickMenu msmenu;				// right-click menu
	// command mode locals...
	Value*		result;						// rcmenu result
	MSPlugin*	plugin;						// current plugin under manip if non-NULL

				RCMenu(short iflags);
	void		init(Value* name, int local_count, Value** inits, HashTable* local_scope, MenuItem** iitems, int iitem_count, HashTable* handlers);
			   ~RCMenu();

#	define is_RCMenu(o) ((o)->tag == class_tag(RCMenu))  // LAM - defect 307069
				classof_methods (RCMenu, Value);
	void		collect() { delete this; }
	void		gc_trace();
	ScripterExport void sprin1(CharStream* s);

	Value*		get_event_handler(Value* name, Value* event);
	BOOL		call_event_handler(Value* name, Value* event, Value** arg_list, int count);
	

	virtual	Value*	get_property(Value** arg_list, int count);
	virtual	Value*	set_property(Value** arg_list, int count);

	// added 3/21/05. Used by debugger to dump locals and externals to standard out
	void		dump_local_vars_and_externals(int indentLevel);
};

// LAM - 9/10/01

// The following classes have been added
// in 3ds max 4.2.  If your plugin utilizes this new
// mechanism, be sure that your clients are aware that they
// must run your plugin with 3ds max version 4.2 or higher.

class MSSelectFilterCallback : public SelectFilterCallback
{

public:
	MSSelectFilterCallback() 
		{
		selectFilter_call_back_on = FALSE;
		in_selectfilter_callback = FALSE;

		}
	MCHAR dname[128];
	MCHAR* GetName() {return dname;};
	BOOL IsFiltered(SClass_ID isid, Class_ID icid, INode *node);
	Value* selectFilters_fns;
	BOOL selectFilter_call_back_on;
	BOOL in_selectfilter_callback;

};

class MSDisplayFilterCallback : public DisplayFilterCallback
{

public:
	MSDisplayFilterCallback() 
		{
		displayFilter_call_back_on = FALSE;
		in_displayfilter_callback = FALSE;

		}
	MCHAR dname[128];
	MCHAR* GetName() {return dname;};
	
	BOOL IsVisible(SClass_ID isid, Class_ID icid, INode *node);
	Value* displayFilters_fns;
	BOOL displayFilter_call_back_on;
	BOOL in_displayfilter_callback;
};

// End of 3ds max 4.2 Extension

