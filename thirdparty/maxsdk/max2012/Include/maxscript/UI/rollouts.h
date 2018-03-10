/*	
 *		Rollouts.h - Rollout panel classes & functions for MAXScript
 *
 *			Copyright (c) John Wainwright 1996
 *
 */

#pragma once

#include "..\kernel\value.h"
#include "..\foundation\streams.h"
#include "..\..\custcont.h"
#include "..\..\control.h"
#include "..\..\maxapi.h"

// forward declarations
class Array;
class IMtlParams;
class TexDADMgr;
class MtlDADMgr;
struct ParamDef;

struct layout_data		// rollout control layout data
{
	int	left;
	int top;
	int	width;
	int height;
	int columns;
};

/* some layout constants (in pixels) ... */

#define TOP_MARGIN				2
#define SIDE_MARGIN				4
#define RADIO_DOT_WIDTH			23
#define CHECK_BOX_WIDTH			24
#define LEFT_ALIGN				13
#define RIGHT_ALIGN				13
#define GROUP_BOX_Y_MARGIN		6
#define GROUP_BOX_X_MARGIN		4
#define SPACING_BEFORE			5

/* ---------------------- Rollout class ----------------------- */

/* contains the defintion of  rollout panel.  This includes:
 *     - an 'instance variable' array, these variables can be accessed as locals in rollout handlers
 *     - a control array, containing rolout control instances
 *     - a hashtable of event handler functions
 *   there are associated Thunks for the locals & controls so you can ref them as variables in
 *   handlers
 */

class RolloutControl;
class RolloutFloater;
class MSPlugin;
class RolloutChangeCallback;
class PB2Param;
struct NotifyInfo;

visible_class_debug_ok (Rollout)

class Rollout : public Value
{
public:
	Value*		name;						// rollout's variable name
	Value*		title;						// title factor			
	HashTable*  local_scope;				// local name space			
	Value**		locals;						// local var array			
	Value**		local_inits;				//   "    "    "  init vals	
	int			local_count;				//   "    "  count			
	RolloutControl** controls;				// control array			
	int			control_count;				//    "    "  count			
	HashTable*	handlers;					// handler tables			
	short		flags;						// rollout flags			
	short		order;						// rollout open order no.	
	Interface*	ip;							// Interface pointer		
	HWND		page;						// my dialog HWND when visible 
	HDC			rollout_dc;					// my dialog dev. context	
	HFONT		font;						// dialog's default font	
	int			text_height;				// metrics....				
	int			default_control_leading;
	int			rollout_width;
	int			rollout_height;
	int			rollout_category;
	int			current_width;				// used for across: processing...
	int			current_left;
	int			max_y, last_y;
	int			across_count;
	WORD		close_button_ID;			// id of gen'd close button 
	BOOL		selected;					// selected to be open  
	BOOL		disabled;					// error in handler -> ro disabled 		
	CharStream* source;						// source text if available 
	BOOL		init_values;				// whether to init ctrl/local values on (re)open
	MSPlugin*	plugin;						// plugin I'm open on if non-NULL
	RolloutChangeCallback* tcb;				// timechange callback if rollout has controller-linked spinners
	IMtlParams* imp;						// MtlEditor interface if open in Mtl Editor and other stuff...
	TexDADMgr*	texDadMgr;
	MtlDADMgr*	mtlDadMgr;
	HWND		hwMtlEdit;
	RolloutFloater* rof;					// owning rolloutfloater window if present there
	WORD        next_id;					// dialog item ID allocators
	Tab<RolloutControl*> id_map;			// dialog item ID map for taking item ID's to associated RolloutControl

				Rollout(short iflags);
	void		init(Value* name, Value* title, int local_count, Value** inits, HashTable* local_scope, RolloutControl** controls, int control_count, HashTable* handlers, CharStream* source);
			   ~Rollout();

	static void	ColorChangeNotifyProc(void* param, NotifyInfo* pInfo);
#	define		is_rollout(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(Rollout))
				classof_methods (Rollout, Value);
	void		collect() { delete this; }
	void		gc_trace();
	ScripterExport void		sprin1(CharStream* s);

	ScripterExport BOOL add_page(Interface *ip, HINSTANCE hInstance, int ro_flags = 0, RolloutFloater* rof = NULL);
	ScripterExport void delete_page(Interface *ip, RolloutFloater* rof = NULL);
	void		open(Interface *ip, BOOL rolled_up = FALSE);
	ScripterExport void close(Interface *ip, RolloutFloater* rof = NULL);
	ScripterExport BOOL ok_to_close(RolloutFloater* rof = NULL);
	ScripterExport void run_event_handler(Value* event, Value** arg_list, int count);
	ScripterExport bool has_event_handler(Value* event); // return true if event handler defined for rollout

	Value*		call_event_handler(Value* event, Value** arg_list, int count);
	void		add_close_button(HINSTANCE hInstance, int& current_y);
	void		edit_script();
	void		TimeChanged(TimeValue t);

	// various open/close for scripted plug-in rollouts
	// command panel
	void		BeginEditParams(IObjParam* ip, MSPlugin* plugin, ULONG flags, Animatable *prev);
	void		EndEditParams(IObjParam* ip, MSPlugin* plugin, ULONG flags, Animatable *next);
	// mtl editor
	void		CreateParamDlg(HWND hwMtlEdit, IMtlParams* imp, MSPlugin* plugin, TexDADMgr* texDadMgr, MtlDADMgr* mtlDadMgr);
	void		SetThing(MSPlugin* plugin);
	void		ReloadDialog();
	void		SetTime(TimeValue t);
	void		DeleteThis();
	// update/reload
	void		InvalidateUI();
	void		InvalidateUI(ParamID id, int tabIndex=-1); // nominated param

	virtual	Value*	get_property(Value** arg_list, int count);
	virtual	Value*	set_property(Value** arg_list, int count);
	virtual	Value*	set_nested_controller(Value** arg_list, int count);

	// added 3/21/05. Used by debugger to dump locals and externals to standard out
	void		dump_local_vars_and_externals(int indentLevel);
};

#define RO_NO_CLOSEBUTTON	0x0001
#define RO_HIDDEN			0x0002
#define RO_ROLLED_UP		0x0004
#define RO_IN_FLOATER		0x0008
#define RO_INSTALLED		0x0010
#define RO_UTIL_MASTER		0x0020
#define RO_SILENT_ERRORS	0x0040
#define RO_HIDDEN2			0x0080
#define RO_PLUGIN_OWNED		0x0100
#define RO_CONTROLS_INSTALLED	0x0200

/* --------------------- RolloutFloater class ------------------------ */

visible_class (RolloutFloater)

class RolloutFloater : public Value, public IRollupCallback
{
public:
	//NOTE: May be null.  All affected code should have null checking
	HWND			window;		// modeless dialog window

	HWND			ru_window;	// host rollup winddow cust control
	IRollupWindow*	irw;
	Tab<Rollout*>	rollouts;   // my rollouts
	int				width, height; // window size...
	int				left, top; 
	bool			inDelete;

			    RolloutFloater(HWND hWnd);
			    RolloutFloater(MCHAR* title, int left, int top, int width, int height);
				RolloutFloater() {window=ru_window=NULL;irw=NULL;width=height=left=top=0;inDelete=false;tag = class_tag(RolloutFloater);}
	 		   ~RolloutFloater();
#	define		is_rolloutfloater(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(RolloutFloater))

	void		HandleDestroy(); // from IRollupCallback

				classof_methods (RolloutFloater, Value);
	void		collect() { delete this; }

	// Needed to solve ambiguity between Collectable's operators and MaxHeapOperators
	using Collectable::operator new;
	using Collectable::operator delete;

	void		add_rollout(Rollout* ro, BOOL rolledUp, BOOL borderless);
	void		remove_rollout(Rollout* ro);

	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);
};
#pragma warning(push)
#pragma warning(disable:4100)

/* -------------------- RolloutControl classes ----------------------- */

/* represent controls such as buttons & spinners on rollout panels, RolloutControl is abstract
 * root of all controls */

visible_class (RolloutControl)

class RolloutControl : public Value, public ReferenceMaker
{
public:
	Value*		name;
	Value*		caption;
	Value*		init_caption;
	BOOL		enabled;
	Value**		keyparms;
	int			keyparm_count;
	Rollout*	parent_rollout;
	WORD	    control_ID;
	Control*	controller;		// optional linked animation controller
	ParamDimension*  dim;		// controllers dimension
	PB2Param*	pid;			// if non-NULL, indicates this control is associated with an MSPlugin parameter &
								//   points at ParamUIRep-like data for it
	short		flags;

	ScripterExport  RolloutControl();
	ScripterExport  RolloutControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);
	ScripterExport ~RolloutControl();

				classof_methods (RolloutControl, Value);
	BOOL		_is_rolloutcontrol() { return 1; }
#	define		is_rolloutcontrol(v) ((v)->_is_rolloutcontrol())
	void		collect() { delete this; }
	ScripterExport void gc_trace();

	// Needed to solve ambiguity between Collectable's operators and MaxHeapOperators
	using Collectable::operator new;
	using Collectable::operator delete;

	virtual ScripterExport void	add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y);
	virtual LPCMSTR	get_control_class() = 0;
	virtual DWORD	get_control_style() { return WS_TABSTOP; }
	virtual DWORD	get_control_ex_style() { return 0; }
	virtual void	init_control(HWND control) { }
	virtual void	compute_layout(Rollout *ro, layout_data* pos) { }
	virtual ScripterExport void	compute_layout(Rollout *ro, layout_data* pos, int& current_y);
	virtual ScripterExport void	process_layout_params(Rollout *ro, layout_data* pos, int& current_y);
	virtual ScripterExport void	setup_layout(Rollout *ro, layout_data* pos, int& current_y);
	virtual ScripterExport void	process_common_params();
	virtual ScripterExport void	call_event_handler(Rollout *ro, Value* event, Value** arg_list, int count);
	virtual ScripterExport void	run_event_handler(Rollout *ro, Value* event, Value** arg_list, int count);
	virtual BOOL	handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam) { return FALSE; }
			ScripterExport WORD	next_id();

	virtual	ScripterExport Value* get_property(Value** arg_list, int count);
	virtual	ScripterExport Value* set_property(Value** arg_list, int count);
	virtual	ScripterExport void	set_text(MCHAR* text, HWND ctl = NULL, Value* align = NULL);
	virtual	ScripterExport void	set_enable();
	virtual ScripterExport BOOL set_focus();
	virtual ScripterExport int num_controls() { return 1; }
	        ScripterExport Value* get_event_handler(Value* event);
	// Animatable
	virtual Class_ID ClassID() { return Class_ID(0x3d063ac9, 0x7136487c); }


	// ReferenceMaker
	int			NumRefs() { return 1; }
	RefTargetHandle GetReference(int i) { return controller; }
protected:
	virtual void		SetReference(int i, RefTargetHandle rtarg) { controller = (Control*)rtarg; }
public:
	ScripterExport RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message);
	virtual void controller_changed() { }
	virtual BOOL controller_ok(Control* c) { return FALSE; }

	// PB2 UI update
	ScripterExport IParamBlock2* get_pblock();
	virtual void Reload() { }
	virtual void InvalidateUI() { }
	virtual void set_pb2_value() { }
	virtual void get_pb2_value(BOOL load_UI=TRUE) { }
	virtual void SetTexDADMgr(DADMgr* dad) { }
	virtual int FindSubTexFromHWND(HWND hw) { return -1; }
	virtual void SetMtlDADMgr(DADMgr* dad) { }
	virtual int FindSubMtlFromHWND(HWND hw) { return -1; } 

	// added for r8. wraps handler function in a PluginMethod pointing at scripted plugin associated with rollout 
	        ScripterExport Value* get_wrapped_event_handler(Value* event);

	// added for r11
	HWND GetHWND();
};

// flag bits for RolloutControl::flags
#define ROC_FIXED_WIDTH				0x0001	// a specific width: supplied, don't resize buttons on .text =
#define ROC_MAKING_EDIT				0x0002
#define ROC_INVISIBLE				0x0004	// rollout control is set to invisible and is disabled
#define ROC_VAL_CHANGED				0x0008	// value was changed while control had focus
#define ROC_EDITTEXT_MULTILINE		0x0010	// edittext control is multiline
#define ROC_COLORSWATCH_POINT4		0x0010	// Set if color swatch is a Point4 (FRGBA)
#define ROC_PICKBUTTON_AUTODISP		0x0010	// Set if autoDisplay is turned on for pickbutton (autodisplay node name)
#define ROC_SPINNER_KEYBRACKETS		0x0010  // Set if spinner's setKeyBrakets is true(on).

#define ROC_IN_HANDLER				0x0020	// Set if currently running event handler
#define ROC_HANDLER_REENTRANT		0x0040	// Set if rollout control's event handlers are re-entrant. Only case is checkbutton.
#define ROC_EDITTEXT_READONLY		0x0080	// edittext control is read-only

#define ROC_DELETE_TOOLTIP			0x0100	// control has an extender-based tooltip to be manually deleted

extern LPCMSTR cust_button_class;

/* ------------- PB2Param class -------------------- */

// present in a UI control if rollout is part of a scripted plugin
// and this control is associated with a ParamBlock2  param

class PB2Param
{
public:
	ParamID id;			// pblock param ID
	int index;			// pblock direct index of param
	int tabIndex;		// element index if param is Tab<>
	int	block_id;		// owning block's ID
	int	subobjno;		// texmap or mtl param subobjno in the block
	ParamDimension* dim;// parameter's dimension
	ParamType2 type;	// parameter's type

	PB2Param(const ParamDef& pd, int index, int block_id, int subobjno, int tabIndex = -1);
}; 

/* -------------------- LabelControl  ------------------- */

visible_class (LabelControl)

class LabelControl : public RolloutControl
{
public:
				LabelControl(Value* name, Value* caption, Value** keyparms, int keyparm_count)
					: RolloutControl(name, caption, keyparms, keyparm_count) { tag = class_tag(LabelControl); }

    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
							{ return new LabelControl (name, caption, keyparms, keyparm_count); }

				classof_methods (LabelControl, RolloutControl);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s) { s->printf(_M("LabelControl:%s"), name->to_string()); }

	LPCMSTR		get_control_class() { return _M("STATIC"); }
	DWORD		get_control_style();
	void		compute_layout(Rollout *ro, layout_data* pos);
};

/* -------------------- ButtonControl  ------------------- */

visible_class (ButtonControl)

class ButtonControl : public RolloutControl
{
public:
	HIMAGELIST	images;
	int			image_width, image_height;
	int			iOutEn, iInEn, iOutDis, iInDis;
	MSTR		m_toolTip;
	int			colortheme;
	MSTR		images_filename;

				ButtonControl(Value* name, Value* caption, Value** keyparms, int keyparm_count)
					: RolloutControl(name, caption, keyparms, keyparm_count)
						{
							tag = class_tag(ButtonControl);
							images = NULL;
							colortheme = FALSE;
						}
			   ~ButtonControl();

    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
							{ return new ButtonControl (name, caption, keyparms, keyparm_count); }

				classof_methods (ButtonControl, RolloutControl);
	void		collect() { delete this; }
	void		sprin1(CharStream* s) { s->printf(_M("ButtonControl:%s"), name->to_string()); }

	void		ColorChangeNotifyProc(void* param, NotifyInfo* pInfo);
	LPCMSTR		get_control_class() { return cust_button_class; }
	void		init_control(HWND control);
	void		compute_layout(Rollout *ro, layout_data* pos);
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam);
	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);
	void		set_enable();
};	

/* -------------------- CheckButtonControl  ------------------- */

visible_class (CheckButtonControl)

class CheckButtonControl : public RolloutControl
{
public:
	int			checked;	// LAM - 2/11/02 - changed from BOOL (which is actually an int) since now tristate
	HIMAGELIST	images;
	int			image_width, image_height;
	int			iOutEn, iInEn, iOutDis, iInDis;
	MSTR		m_toolTip;
	int			colortheme;
	MSTR		images_filename;

				CheckButtonControl(Value* name, Value* caption, Value** keyparms, int keyparm_count)
					: RolloutControl(name, caption, keyparms, keyparm_count)
						{
							tag = class_tag(CheckButtonControl);
							images = NULL;
							colortheme = FALSE;
						}
			   ~CheckButtonControl();

    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
							{ return new CheckButtonControl (name, caption, keyparms, keyparm_count); }

				classof_methods (CheckButtonControl, RolloutControl);
	void		collect() { delete this; }
	void		sprin1(CharStream* s) { s->printf(_M("CheckButtonControl:%s"), name->to_string()); }

	void		ColorChangeNotifyProc(void* param, NotifyInfo* pInfo);
	LPCMSTR		get_control_class() { return cust_button_class; }
	void		init_control(HWND control);
	void		compute_layout(Rollout *ro, layout_data* pos);
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam);
	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);
	void		set_enable();

	void		Reload();
	void		InvalidateUI();
	void		set_pb2_value();
	void		get_pb2_value(BOOL load_UI=TRUE);
};	

/* -------------------- EditTextControl  ------------------- */

visible_class (EditTextControl)

class EditTextControl : public RolloutControl
{
public:
	Value*		text;
	Value*		bold;
	bool		in_setvalue;

				EditTextControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);

    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
							{ return new EditTextControl (name, caption, keyparms, keyparm_count); }

				classof_methods (EditTextControl, RolloutControl);
	void		collect() { delete this; }
	void		sprin1(CharStream* s) { s->printf(_M("EditTextControl:%s"), name->to_string()); }
	void		gc_trace();

	void		add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y);
	LPCMSTR		get_control_class() { return CUSTEDITWINDOWCLASS; }
	void		compute_layout(Rollout *ro, layout_data* pos, int& current_y);
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam);
	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);
	void		set_enable();
	int			num_controls() { return 2; }

	void		Reload();
	void		InvalidateUI();
	void		set_pb2_value();
	void		get_pb2_value(BOOL load_UI=TRUE);
};

/* -------------------- ComboBoxControl  ------------------- */

visible_class (ComboBoxControl)

class ComboBoxControl : public RolloutControl
{
public:
	Array*		item_array;
	int			selection;
	short		type;
	short		flags;

				ComboBoxControl(Value* name, Value* caption, Value** keyparms, int keyparm_count, int type = CBS_SIMPLE);

    static RolloutControl* create_cb(Value* name, Value* caption, Value** keyparms, int keyparm_count)
							{ return new ComboBoxControl (name, caption, keyparms, keyparm_count); }
    static RolloutControl* create_dd(Value* name, Value* caption, Value** keyparms, int keyparm_count)
							{ return new ComboBoxControl (name, caption, keyparms, keyparm_count, CBS_DROPDOWNLIST); }

				classof_methods (ComboBoxControl, RolloutControl);
	void		collect() { delete this; }
	void		sprin1(CharStream* s) { s->printf(_M("ComboBoxControl:%s"), name->to_string()); }
	void		gc_trace();

	void		add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y);
	LPCMSTR		get_control_class() { return _M("COMBOBOX"); }
	DWORD		get_control_style() { return CBS_DROPDOWNLIST | CBS_NOINTEGRALHEIGHT | WS_TABSTOP; }
	void		compute_layout(Rollout *ro, layout_data* pos, int& current_y);
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam);
	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);
	void		set_enable();
	int			num_controls() { return 2; }

	void		Reload();
	void		InvalidateUI();
	void		set_pb2_value();
	void		get_pb2_value(BOOL load_UI=TRUE);
};

#define CBF_EDIT_FIELD_CHANGING		0x0001

/* -------------------- ListBoxControl  ------------------- */

visible_class (ListBoxControl)

class ListBoxControl : public RolloutControl
{
public:
	Array*		item_array;
	int			selection;

				ListBoxControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);

    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
							{ return new ListBoxControl (name, caption, keyparms, keyparm_count); }

				classof_methods (ListBoxControl, RolloutControl);
	void		collect() { delete this; }
	void		sprin1(CharStream* s) { s->printf(_M("ListBoxControl:%s"), name->to_string()); }
	void		gc_trace();

	void		add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y);
	LPCMSTR		get_control_class() { return _M("LISTBOX"); }
	DWORD		get_control_style() { return WS_TABSTOP; }
	void		compute_layout(Rollout *ro, layout_data* pos, int& current_y);
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam);
	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);
	void		set_enable();
	int			num_controls() { return 2; }

	void		Reload();
	void		InvalidateUI();
	void		set_pb2_value();
	void		get_pb2_value(BOOL load_UI=TRUE);
};

/* -------------------- SpinnerControl  ------------------- */

visible_class (SpinnerControl)

class SpinnerControl : public RolloutControl
{
public:
	float		fvalue;
	int			ivalue;
	float		max, min;
	float		scale;
	EditSpinnerType spin_type;

				SpinnerControl(Value* name, Value* caption, Value** keyparms, int keyparm_count)
					: RolloutControl(name, caption, keyparms, keyparm_count)  { tag = class_tag(SpinnerControl); }

    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
							{ return new SpinnerControl (name, caption, keyparms, keyparm_count); }

				classof_methods (SpinnerControl, RolloutControl);
	void		collect() { delete this; }
	void		sprin1(CharStream* s) { s->printf(_M("SpinnerControl:%s"), name->to_string()); }

	void		add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y);
	LPCMSTR		get_control_class() { return SPINNERWINDOWCLASS; }
	void		compute_layout(Rollout *ro, layout_data* pos, int& current_y);
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam);

	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);
	void		set_enable();
	BOOL		set_focus();
	int			num_controls() { return 3; }

	void		controller_changed();
	BOOL		controller_ok(Control* c) { return c->SuperClassID() == CTRL_FLOAT_CLASS_ID; }

	void		Reload();
	void		InvalidateUI();
	void		set_pb2_value();
	void		get_pb2_value(BOOL load_UI=TRUE);
};

/* -------------------- SliderControl  ------------------- */

visible_class (SliderControl)

class SliderControl : public RolloutControl
{
public:
	float		value;
	float		max, min;
	int			ticks;
	int			slider_type;
	bool		vertical;
	bool		sliding;

				SliderControl(Value* name, Value* caption, Value** keyparms, int keyparm_count)
					: RolloutControl(name, caption, keyparms, keyparm_count), sliding(false)  { tag = class_tag(SliderControl); }

    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
							{ return new SliderControl (name, caption, keyparms, keyparm_count); }

				classof_methods (SliderControl, RolloutControl);
	void		collect() { delete this; }
	void		sprin1(CharStream* s) { s->printf(_M("SliderControl:%s"), name->to_string()); }

	void		add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y);
	LPCMSTR		get_control_class();
	void		compute_layout(Rollout *ro, layout_data* pos, int& current_y);
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam);

	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);
	void		set_enable();
	int			num_controls() { return 2; }

	void		controller_changed();
	BOOL		controller_ok(Control* c) { return c->SuperClassID() == CTRL_FLOAT_CLASS_ID; }

	void		Reload();
	void		InvalidateUI();
	void		set_pb2_value();
	void		get_pb2_value(BOOL load_UI=TRUE);
};

/* -------------------- PickerControl  ------------------- */

class PickerControl;

class PickerControlFilter : public PickNodeCallback
{
public:
		Value*	filter_fn;
		PickerControl*  picker;

			    PickerControlFilter(Value* filter, PickerControl* picker) : filter_fn(filter), picker(picker) { }
		BOOL	Filter(INode *node);
};

class PickerControlMode : public PickModeCallback
{
public:
		PickerControlFilter* pick_filter;
		MSTR			msg;
		PickerControl*  picker;

		PickerControlMode(PickerControlFilter* ifilter, MCHAR* imsg, PickerControl* ipick);

		BOOL	HitTest(IObjParam *ip, HWND hWnd, ViewExp *vpt, IPoint2 m, int flags);
		BOOL	Pick(IObjParam *ip, ViewExp *vpt);
		PickNodeCallback *GetFilter() { return pick_filter; }
		BOOL	RightClick(IObjParam *ip, ViewExp *vpt) { return TRUE; }
		void	EnterMode(IObjParam *ip);
		void	ExitMode(IObjParam *ip);
};

visible_class (PickerControl)

class PickerControl : public RolloutControl
{
public:
	PickerControlFilter* pick_filter;
	PickerControlMode*   pick_mode;
    ICustButton*		 cust_button; 
	Value*				 picked_object;
	MSTR				 m_toolTip;

				PickerControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);
			   ~PickerControl();

    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
							{ return new PickerControl (name, caption, keyparms, keyparm_count); }

				classof_methods (PickerControl, RolloutControl);
	void		collect() { delete this; }
	void		gc_trace();
	void		sprin1(CharStream* s) { s->printf(_M("PickerControl:%s"), name->to_string()); }

	LPCMSTR		get_control_class() { return cust_button_class; }
	void		compute_layout(Rollout *ro, layout_data* pos);
	void		init_control(HWND control);
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam);
	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);
	void		set_enable();

	void		Reload();
	void		InvalidateUI();
	void		set_pb2_value();
	void		get_pb2_value(BOOL load_UI=TRUE);
};

/* -------------------- ColorPickerControl  ------------------- */

visible_class (ColorPickerControl)

class ColorPickerControl : public RolloutControl
{
public:
	Value*		  color;
    IColorSwatch* csw;
	Value*		  title;
	BOOL		  notifyAfterAccept;

				ColorPickerControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);
			   ~ColorPickerControl();

    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
							{ return new ColorPickerControl (name, caption, keyparms, keyparm_count); }

				classof_methods (ColorPickerControl, RolloutControl);
	void		collect() { delete this; }
	void		gc_trace();
	void		sprin1(CharStream* s) { s->printf(_M("ColorPickerControl:%s"), name->to_string()); }

	LPCMSTR		get_control_class() { return COLORSWATCHWINDOWCLASS; }
	void		init_control(HWND control);
	void		add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y);
	void		compute_layout(Rollout *ro, layout_data* pos, int& current_y);
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam);
	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);
	void		set_enable();
	int			num_controls() { return 2; }

	void		Reload();
	void		InvalidateUI();
	void		set_pb2_value();
	void		get_pb2_value(BOOL load_UI=TRUE);
};

/* -------------------- RadioControl  ------------------- */

visible_class (RadioControl)

class RadioControl : public RolloutControl
{
public:
	int			state;
	int			btn_count;

				RadioControl(Value* name, Value* caption, Value** keyparms, int keyparm_count)
					: RolloutControl(name, caption, keyparms, keyparm_count)  { tag = class_tag(RadioControl); }

    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
							{ return new RadioControl (name, caption, keyparms, keyparm_count); }

				classof_methods (RadioControl, RolloutControl);
	void		collect() { delete this; }
	void		sprin1(CharStream* s) { s->printf(_M("RadioControl:%s"), name->to_string()); }

	void		add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y);
	LPCMSTR		get_control_class() { return _M("BUTTON"); }
	DWORD		get_control_style() {  return BS_AUTORADIOBUTTON; }
	void		compute_layout(Rollout *ro, layout_data* pos, int& current_y);
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam);

	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);
	void		set_enable();
	BOOL		set_focus();
	int			num_controls() { return btn_count + 1; } // buttons and caption. don't count 1 dummy button that ends group

	void		Reload();
	void		InvalidateUI();
	void		set_pb2_value();
	void		get_pb2_value(BOOL load_UI=TRUE);
};

/* -------------------- CheckBoxControl  ------------------- */

visible_class (CheckBoxControl)

class CheckBoxControl : public RolloutControl
{
public:
	int			checked; // LAM - 2/11/02 - added 3rd state (indeterminate). Changed from BOOL to int just for clarity.

				CheckBoxControl(Value* name, Value* caption, Value** keyparms, int keyparm_count)
					: RolloutControl(name, caption, keyparms, keyparm_count)  { tag = class_tag(CheckBoxControl); }

    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
							{ return new CheckBoxControl (name, caption, keyparms, keyparm_count); }

				classof_methods (CheckBoxControl, RolloutControl);
	void		collect() { delete this; }
	void		sprin1(CharStream* s) { s->printf(_M("CheckBoxControl:%s"), name->to_string()); }

	LPCMSTR		get_control_class() { return _M("BUTTON"); }
	DWORD		get_control_style() {  return BS_AUTO3STATE | BS_MULTILINE | WS_TABSTOP; }
	void		init_control(HWND control);
	void		compute_layout(Rollout *ro, layout_data* pos);
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam);

	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);

	void		Reload();
	void		InvalidateUI();
	void		set_pb2_value();
	void		get_pb2_value(BOOL load_UI=TRUE);
};

/* -------------------- BitmapControl  ------------------- */

visible_class (BitmapControl)

class BitmapControl : public RolloutControl
{
public:
	Value*		file_name;
	Value*		max_bitmap;    // if supplied
	HBITMAP		bitmap;

				BitmapControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);
			   ~BitmapControl();

    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
							{ return new BitmapControl (name, caption, keyparms, keyparm_count); }

				classof_methods (BitmapControl, RolloutControl);
	void		collect() { delete this; }
	void		sprin1(CharStream* s) { s->printf(_M("BitmapControl:%s"), name->to_string()); }
	void		gc_trace();

	LPCMSTR		get_control_class() { return _M("STATIC"); }
	DWORD		get_control_style() { return SS_BITMAP + SS_CENTERIMAGE; }
	DWORD		get_control_ex_style() { return WS_EX_CLIENTEDGE; }
	void		compute_layout(Rollout *ro, layout_data* pos);
	void		process_layout_params(Rollout *ro, layout_data* pos, int& current_y);
	void		init_control(HWND control);
	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);
};

/* -------------------- MapButtonControl  ------------------- */

visible_class (MapButtonControl)

class MapButtonControl : public ButtonControl
{
public:
		Value*       map;
		ICustButton* btn;

				MapButtonControl(Value* name, Value* caption, Value** keyparms, int keyparm_count)
					: ButtonControl(name, caption, keyparms, keyparm_count)
						{
							tag = class_tag(MapButtonControl);
							btn = NULL;
							map = NULL;
						}
				~MapButtonControl() { if (btn != NULL) ReleaseICustButton(btn); }

    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
							{ return new MapButtonControl (name, caption, keyparms, keyparm_count); }

				classof_methods (MapButtonControl, RolloutControl);
	void		gc_trace();
	void		collect() { delete this; }
	void		sprin1(CharStream* s) { s->printf(_M("MapButtonControl:%s"), name->to_string()); }

	void		init_control(HWND control);
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam);
	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);

	void		Reload();
	void		InvalidateUI();
	void		set_pb2_value();
	void		get_pb2_value(BOOL load_UI=TRUE);

	void		SetTexDADMgr(DADMgr* dad) { if (btn) btn->SetDADMgr(dad); }
	int			FindSubTexFromHWND(HWND hw);
};	

/* -------------------- MtlButtonControl  ------------------- */

visible_class (MtlButtonControl)

class MtlButtonControl : public ButtonControl
{
public:
		Value*		 mtl;
		ICustButton* btn;

				MtlButtonControl(Value* name, Value* caption, Value** keyparms, int keyparm_count)
					: ButtonControl(name, caption, keyparms, keyparm_count)
						{
							tag = class_tag(MtlButtonControl);
							btn = NULL;
							mtl = NULL;
						}
				~MtlButtonControl() { if (btn != NULL) ReleaseICustButton(btn); }

    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
							{ return new MtlButtonControl (name, caption, keyparms, keyparm_count); }

				classof_methods (MtlButtonControl, RolloutControl);
	void		gc_trace();
	void		collect() { delete this; }
	void		sprin1(CharStream* s) { s->printf(_M("MtlButtonControl:%s"), name->to_string()); }

	void		init_control(HWND control);
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam);
	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);
	void		set_enable();

	void		Reload();
	void		InvalidateUI();
	void		set_pb2_value();
	void		get_pb2_value(BOOL load_UI=TRUE);

	void		SetMtlDADMgr(DADMgr* dad) { if (btn) btn->SetDADMgr(dad); }
	int			FindSubMtlFromHWND(HWND hw);
};	

/* ----------------------- GroupControls  ---------------------- */

visible_class (GroupStartControl)

class GroupStartControl : public RolloutControl
{
public:
	int		start_y;			/* y coord of top of group */

			GroupStartControl(Value* caption)
				: RolloutControl(NULL, caption, NULL, 0)  
			{ tag = class_tag(GroupStartControl); flags |= ROC_FIXED_WIDTH; }

			classof_methods (GroupStartControl, RolloutControl);
	void	collect() { delete this; }
	void	sprin1(CharStream* s) { s->printf(_M("GroupStartControl:%s"), caption->to_string()); }

	void	add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y);
	LPCMSTR	get_control_class() { return _M(""); }
	void	compute_layout(Rollout *ro, layout_data* pos) { }
};

visible_class (GroupEndControl)

class GroupEndControl : public RolloutControl
{
	GroupStartControl* my_start;			/* link back to my group start control */
public:
			GroupEndControl(GroupStartControl* starter)
				: RolloutControl(NULL, starter->caption, NULL, 0) 
			{ tag = class_tag(GroupEndControl); my_start = starter; flags |= ROC_FIXED_WIDTH;}

			classof_methods (GroupEndControl, RolloutControl);
	void	collect() { delete this; }
	void	sprin1(CharStream* s) { s->printf(_M("GroupEndControl:%s"), caption->to_string()); }

	void	add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y);
	LPCMSTR	get_control_class() { return _M(""); }
	void	compute_layout(Rollout *ro, layout_data* pos) { }
};
#pragma warning(pop) // for C4100
// ---- time change callback for rollouts with controller-linked spinners in them ----

class RolloutChangeCallback : public TimeChangeCallback
{
public:
	Rollout*	ro;
				RolloutChangeCallback(Rollout* iro) { ro = iro; }
	void		TimeChanged(TimeValue t) { ro->TimeChanged(t); }
};

/* control keyword parameter access macros... */

extern ScripterExport Value* _get_control_param(Value** keyparms, int count, Value* key_name);
extern ScripterExport Value* _get_control_param_or_default(Value** keyparms, int count, Value* key_name, Value* def);

#define control_param(key)					_get_control_param(keyparms, keyparm_count, n_##key)
#define control_param_or_default(key, def)	_get_control_param_or_default(keyparms, keyparm_count, n_##key##, def)
#define int_control_param(key, var, def)	((var = _get_control_param(keyparms, keyparm_count, n_##key)) == &unsupplied ? def : var->to_int())
#define float_control_param(key, var, def)	((var = _get_control_param(keyparms, keyparm_count, n_##key)) == &unsupplied ? def : var->to_float())

