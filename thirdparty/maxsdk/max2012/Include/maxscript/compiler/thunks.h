/*		Numbers.h - the Thunk family of classes - variable accessors for MAXScript
 *
 *		Copyright (c) John Wainwright, 1996
 *		
 *
 */

#pragma once

#include "..\kernel\value.h"
#include "..\foundation\streams.h"
#include "..\ui\rollouts.h"
#include "..\ui\uiextend.h"
// forward declarations
class MouseTool;

#pragma warning(push)
#pragma warning(disable:4100)

/* ----------------------- Thunk  ---------------------- */
visible_class (Thunk)

class Thunk : public Value
{
public:
	Value*	name;
	BOOL	clear_container; // outer-level prop in a prop sequence, clear current_container when done

			classof_methods (Thunk, Value);
#	define	is_thunk(v) ((v)->_is_thunk())
#	define  is_indirect_thunk(v) ((v)->_is_indirect_thunk())
	BOOL	_is_thunk() { DbgAssert(!is_sourcepositionwrapper(this)); return TRUE; }
	ScripterExport void	gc_trace();
	Thunk() : clear_container(FALSE), name(NULL) { }

	Thunk*	to_thunk() {return this; }
	virtual Thunk* make_free_thunk(int level) { return NULL; }
	void    assign(Value* val) { assign_vf(&val, 1); }

	ScripterExport Value*	get_property(Value** arg_list, int count);
	ScripterExport Value*	set_property(Value** arg_list, int count);
};

/* -------------------- GlobalThunk  ------------------- */

class GlobalThunk : public Thunk
{
public:
	Value*	cell;

	ScripterExport		GlobalThunk(Value* init_name) { init(init_name); }
	ScripterExport		GlobalThunk(Value* init_name, Value* init_val);
	ScripterExport void	init(Value* init_name);
#	define	is_globalthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_GLOBAL_THUNK_TAG)

	ScripterExport Value* eval();

	ScripterExport void	gc_trace();
	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s);

	Value*	assign_vf(Value**arg_list, int count);

};

class ConstGlobalThunk : public GlobalThunk
{
public:
			ConstGlobalThunk(Value* iname) : GlobalThunk(iname) { tag = INTERNAL_CONST_GLOBAL_THUNK_TAG; }
			ConstGlobalThunk(Value* iname, Value* ival) : GlobalThunk(iname, ival) { tag = INTERNAL_CONST_GLOBAL_THUNK_TAG; }
#	define	is_constglobalthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_CONST_GLOBAL_THUNK_TAG)

	Value*	eval() { return cell->is_const() ? cell->copy_vf(NULL, 0) : cell; }
	void	collect() { delete this; }

	Value*	assign_vf(Value**arg_list, int count) { throw AssignToConstError (this); return &undefined; }
};

/* -------------------- SystemGlobalThunk  ------------------- */

/* system globals are abstractions over some system state accessing functions, such as 
 * animation_range, current_renderer,e tc. */

class SystemGlobalThunk : public Thunk
{
	Value* (*get_fn)();
	Value* (*set_fn)(Value*);
public:
	ScripterExport		SystemGlobalThunk(Value* init_name, Value* (*iget)(), Value* (*iset)(Value*));
// LAM 4/1/00 - added following to prevent AF in name clash debugging output in HashTable::put_new()
#	define	is_systemglobalthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_SYS_GLOBAL_THUNK_TAG)

	ScripterExport Value* eval();

	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s) { s->printf(_M("SystemGlobal:%s"), name->to_string()); }

	Value*	assign_vf(Value**arg_list, int count);
};

/* -------------------- LocalThunk  ------------------- */

class LocalThunk : public Thunk
{
public:
	int		frame_level;	// frame nest level at declaration
	int		index;			// local var's index in local frame

			LocalThunk(Value* init_name, int init_index, int iframe_lvl);
#	define	is_localthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_LOCAL_THUNK_TAG)

	Thunk*	make_free_thunk(int level);

	Value*	eval();
	void	collect() { delete this; }
	void	sprin1(CharStream* s);

	Value*	assign_vf(Value**arg_list, int count);
};

class IndirectLocalThunk : public LocalThunk
{
public:
	IndirectLocalThunk(Value* init_name, int init_index, int iframe_lvl) :
				LocalThunk(init_name, init_index, iframe_lvl) { }

	BOOL	_is_indirect_thunk() { DbgAssert(!is_sourcepositionwrapper(this)); return TRUE; }
	Thunk*	make_free_thunk(int level);

	Value*	eval();
	void	collect() { delete this; }
	void	sprin1(CharStream* s) { s->printf(_M("&")); LocalThunk::sprin1(s); }
	Value*	assign_vf(Value**arg_list, int count);
};

// ContextThunk created from an IndirectLocal/FreeThunk on entry to a MAXScript function apply
//   to contain the callers frame context for evals and assigns
class ContextThunk : public Thunk
{
public:
	Thunk*  thunk;			// the wrapped thunk
	Value**	frame;			// callers frame

	ENABLE_STACK_ALLOCATE(ContextLocalThunk);

	ContextThunk(Thunk*  thunk, Value** frame) :
				thunk(thunk), frame(frame) { tag = INTERNAL_CONTEXT_THUNK_TAG;  }
#	define	is_contextthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_CONTEXT_THUNK_TAG)

	void	collect() { delete this; }
	void	sprin1(CharStream* s) { s->printf(_M("&")); thunk->sprin1(s); }

	Value*	eval();
	Value*	assign_vf(Value**arg_list, int count);
};

/* -------------------- FreeThunk  ------------------- */

class FreeThunk : public Thunk
{
public:
	int		level;		// how many levels to reach back
	int		index;		// index there
			FreeThunk(Value* init_name, int level, int index);
#	define	is_freethunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_FREE_THUNK_TAG)

	Thunk*	make_free_thunk(int level);

	void	collect() { delete this; }
	void	sprin1(CharStream* s);

	Value*	eval();
	Value*	assign_vf(Value**arg_list, int count);
};

class IndirectFreeThunk : public FreeThunk
{
public:
			IndirectFreeThunk(Value* init_name, int level, int index) :
				FreeThunk(init_name, level, index) { }

	BOOL	_is_indirect_thunk() { DbgAssert(!is_sourcepositionwrapper(this)); return TRUE; }
	Thunk*	make_free_thunk(int level);

	void	collect() { delete this; }
	void	sprin1(CharStream* s) { s->printf(_M("&")); FreeThunk::sprin1(s); }

	Value* eval();
	Value*	assign_vf(Value**arg_list, int count);
};

/* -------------------- ClosureFreeThunk  ------------------- */

class ClosureFreeThunk : public Thunk
{
public:
	ScripterExport Value* eval();

	void	gc_trace();
	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s);

			ClosureFreeThunk();
			~ClosureFreeThunk();
};

/* -------------------- PropertyThunk  ------------------- */

class PropertyThunk : public Thunk
{
public:
	Value*		target_code;	// code to eval to get target
	Value*		property_name;	// property name
	getter_vf	getter;			// getter virtual fn for built-in properties
	setter_vf	setter;			// setter    "     "       "        "

				PropertyThunk(Value* target, Value* prop_name);
				PropertyThunk(Value* target, Value* prop_name, getter_vf get_fn, setter_vf set_fn);

	void		gc_trace();
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);
#	define		is_propertythunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_PROP_THUNK_TAG)

	ScripterExport Value* eval(Value**arg_list, int count);
	ScripterExport Value* eval() { return eval(NULL,0); }
	Value*		assign_vf(Value**arg_list, int count);
	Value*		op_assign_vf(Value**arg_list, int count);
};

// a PropThunk subclass that is used when a Prop access occurs in a function call
// this is basically a hack to support OLE client method calls, since OLE IDISPATCH
// cannot distinguish methods from props
class FnCallPropertyThunk : public PropertyThunk
{
public:
				FnCallPropertyThunk(Value* target, Value* prop_name, getter_vf get_fn, setter_vf set_fn) 
					: PropertyThunk (target, prop_name, get_fn, set_fn) {}
	void		collect() { delete this; }
	ScripterExport Value* eval();
};

/* -------------------- IndexThunk  ------------------- */

class IndexThunk : public Thunk
{
	Value*		target_code;	// code to eval to get target
	Value*		index_code;		// code to eval to get index

public:
				IndexThunk(Value* index);

#	define		is_indexthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_INDEX_THUNK_TAG)
	void		gc_trace();
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);

	Value*		set_target(Value* targ) { target_code = targ; return this; }
	ScripterExport Value* eval();
	Value*		assign_vf(Value**arg_list, int count);
};

/* -------------------- RolloutControlThunk  ------------------- */

class RolloutControlThunk : public Thunk
{
public:
	int		index;
	Rollout* rollout;

			RolloutControlThunk(Value* name, int control_index, Rollout* rollout);
	BOOL	_is_rolloutthunk() { DbgAssert(!is_sourcepositionwrapper(this)); return 1; }
#	define	is_rolloutthunk(v) ((v)->_is_rolloutthunk())

	Value*	eval() { return rollout->controls[index]; }

	void ScripterExport gc_trace();
	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s);

	Value*	assign_vf(Value**arg_list, int count);
};

/* -------------------- RolloutLocalThunk  ------------------- */

class RolloutLocalThunk : public Thunk
{
public:
	int		index;
	Rollout* rollout;

			RolloutLocalThunk(Value* name, int control_index, Rollout* rollout);
	BOOL	_is_rolloutthunk() { DbgAssert(!is_sourcepositionwrapper(this)); return 1; }
#	define	is_rolloutlocalthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_RO_LOCAL_THUNK_TAG)

	ScripterExport Value* eval();

	void	gc_trace();
	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s);

	Value*	assign_vf(Value**arg_list, int count);
};

class ConstRolloutLocalThunk : public RolloutLocalThunk
{
public:
			ConstRolloutLocalThunk(Value* name, int control_index, Rollout* rollout) 
				: RolloutLocalThunk(name, control_index, rollout) { }

	void	collect() { delete this; }

	Value*	assign_vf(Value**arg_list, int count) { throw AssignToConstError (this); return &undefined; }
};

/* -------------------- ToolLocalThunk  ------------------- */

class ToolLocalThunk : public Thunk
{
public:
	int			index;
	MouseTool*	tool;

			ToolLocalThunk(Value* name, int iindex, MouseTool* tool);

	ScripterExport Value* eval();

#	define	is_toollocalthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_TOOL_LOCAL_THUNK_TAG)
	void	gc_trace();
	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s);

	Value*	assign_vf(Value**arg_list, int count);
};

/* -------------------- CodeBlockLocalThunk  ------------------- */

class CodeBlock;

class CodeBlockLocalThunk : public Thunk
{
public:
	int			index;
	CodeBlock*	block;

			CodeBlockLocalThunk(Value* name, int iindex, CodeBlock* block);

	ScripterExport Value* eval();

#	define	is_codeblocklocalthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_CODEBLOCK_LOCAL_TAG)
	void	gc_trace();
	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s);

	Value*	assign_vf(Value**arg_list, int count);
};

/* -------------------- RCMenuItemThunk  ------------------- */

class RCMenuItemThunk : public Thunk
{
public:
	int			index;
	RCMenu*		rcmenu;

			RCMenuItemThunk(Value* name, int item_index, RCMenu* menu);
	BOOL	_is_rcmenuthunk() { DbgAssert(!is_sourcepositionwrapper(this)); return 1; }
#	define	is_rcmenuitemthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_RCMENU_ITEM_THUNK_TAG)

	Value*	eval() { return rcmenu->items[index]; }

	void ScripterExport gc_trace();
	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s);

	Value*	assign_vf(Value**arg_list, int count);
};

/* -------------------- RCMenuLocalThunk  ------------------- */

class RCMenuLocalThunk : public Thunk
{
public:
	int			index;
	RCMenu*		rcmenu;

			RCMenuLocalThunk(Value* name, int iindex, RCMenu* menu);
	BOOL	_is_rcmenuthunk() { DbgAssert(!is_sourcepositionwrapper(this)); return 1; }
#	define	is_rcmenulocalthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_RCMENU_LOCAL_THUNK_TAG)
#	define	is_rcmenuthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_RCMENU_LOCAL_THUNK_TAG || (v)->tag == INTERNAL_RCMENU_ITEM_THUNK_TAG)

	ScripterExport Value* eval();

	void	gc_trace();
	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s);

	Value*	assign_vf(Value**arg_list, int count);
};

/* -------------------- PluginLocalThunk  ------------------- */

class PluginLocalThunk : public Thunk
{
public:
	int		index;    // access via current_plugin thread local
	BOOL	re_init;  // indicate whether this local needs re-initialization on a redefinition (say for local rollouts, fns, etc.)

			PluginLocalThunk(Value* name, int iindex, BOOL re_init = FALSE);
#	define	is_pluginlocalthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_PLUGIN_LOCAL_THUNK_TAG)

	ScripterExport Value* eval();

	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s);

	Value*	assign_vf(Value**arg_list, int count);
};

class ConstPluginLocalThunk : public PluginLocalThunk
{
public:
	ConstPluginLocalThunk(Value* name, int iindex, BOOL re_init = FALSE) : PluginLocalThunk(name, iindex, re_init) { }
	void	collect() { delete this; }
	Value*	assign_vf(Value**arg_list, int count) { throw AssignToConstError (this); return &undefined; }
};
#pragma warning(pop) // for C4100

/* -------------------- PluginParamThunk  ------------------- */

class PluginParamThunk : public Thunk
{
public:
			PluginParamThunk(Value* name);
#	define	is_pluginparamthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_PLUGIN_PARAM_THUNK_TAG)

	ScripterExport Value* eval();

	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s);

	Value*	assign_vf(Value**arg_list, int count);

	Value*	get_container_property(Value* prop, Value* cur_prop);
	Value*	set_container_property(Value* prop, Value* val, Value* cur_prop);
};

#define push_plugin(_pi)								\
	MSPlugin* _save_cp = thread_local(current_plugin);	\
	thread_local(current_plugin) = _pi;
	
#define pop_plugin()									\
	thread_local(current_plugin) = _save_cp;				

#define push_struct(_struct)								\
	Struct* _save_struct = thread_local(current_struct);	\
	thread_local(current_struct) = _struct;

#define pop_struct()									\
	thread_local(current_struct) = _save_struct;				
/* -------------------- StructMemberThunk  ------------------- */

class StructMemberThunk : public Thunk
{
public:
	int		index;    // access via current_plugin thread local

	ScripterExport StructMemberThunk(Value* name, int iindex);

	ScripterExport Value* eval();

	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s);

	Value*	assign_vf(Value**arg_list, int count);
};

/* -------------------- ThunkReference  ------------------- */
// indirect thunk (eg, &foo)

class ThunkReference : public Thunk
{
public:
	Thunk*	target;			// the target thunk

	ScripterExport ThunkReference(Thunk* target);
#	define	is_thunkref(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_THUNK_REF_TAG)

	void	gc_trace();
	void	collect() { delete this; }
	void	sprin1(CharStream* s);

	Value*  eval();
};

class DerefThunk : public Thunk  // generated by a '*' prefix operator
{
public:
	Value*	target;    // the target to deref

			ScripterExport DerefThunk(Value* target);
#	define	is_derefthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_THUNK_DEREF_TAG)
	void	gc_trace();
	void	collect() { delete this; }
	void	sprin1(CharStream* s) { s->printf(_M("*")); target->sprin1(s); }

	Value*  eval();
	Value*	assign_vf(Value**arg_list, int count);
};

/* -------------------- OwnerThunk  ------------------- */
// used by wrap owner of Functions for use by debugger

class OwnerThunk : public Thunk
{
public:
	Value*	cell;

	ScripterExport		OwnerThunk(Value* owner);
#	define	is_ownerthunk(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_OWNER_THUNK_TAG)

	ScripterExport Value* eval();

	ScripterExport	void	gc_trace();
					void	collect() { delete this; }
	ScripterExport	void	sprin1(CharStream* s);

	Value*	assign_vf(Value**arg_list, int count);

};

