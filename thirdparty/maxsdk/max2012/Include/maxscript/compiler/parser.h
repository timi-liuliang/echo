/*	
 *		Parser.h - a compiler for the 3DS MAX MAXScript scripting language
 *
 *			Copyright (c) John Wainwright 1996
 *
 */

#pragma once

#include "..\kernel\value.h"
#include "..\..\imacroscript.h"

class MSPluginClass;
class CodeBlock;
class MouseTool;
class RCMenu;
class IVisualMSForm;
class MSCustAttribDef;
class HashTable;
class RolloutControl;
class Array;

#define MAX_TOKEN_SIZE	256
#define UNDEFINED_MACRO -1

extern ScripterExport HashTable* globals;

/* tokens ... */

enum lex_token
{
	t_local, t_global, t_fn, t_mapped,
	t_if, t_then, t_else, t_do, t_collect, t_while, t_case, 
	t_open_paren, t_close_paren, t_plus, t_times, t_minus, t_div, t_power, t_end,
	t_pluseq, t_minuseq, t_timeseq, t_diveq,
	t_and, t_or, t_not,
	t_number, t_string, t_assign, t_semicolon, t_bad,
	t_badNum, t_eol, t_comma, t_open_bracket, t_close_bracket, t_colon,
	t_quote, t_doubleQuote, t_ampersand, t_name, t_dot, t_name_literal,
	t_openCurly, t_closeCurly, t_hash, t_eq, t_ne, t_lt, t_gt,
	t_ge, t_le, t_for, t_in, t_from, t_to, t_by, t_of, t_where,
	t_as, t_parameter_keyword, t_path_name,
	t_query, t_slash, t_ellipsis, t_level_name, t_wild_card_name, 
	t_dot_dot, t_end_of_path,
	t_with, t_animate, t_coordsys, t_set, t_undo,
	t_about, t_at, t_on, t_off,t_private,t_public,
	t_max, t_nullary_call, t_utility, t_rollout,
	t_return, t_exit, t_when, t_continue,
	t_struct, t_try, t_catch, t_throw, t_eos,
	t_plugin, t_tool, t_persistent, t_parameters, t_rcmenu, 
	t_macroScript, t_dropScript, t_attributes,
	t_tilde, t_resourceID,

	t_missing  // always last
};

typedef RolloutControl* (*create_fn)(Value* name, Value* caption, Value** keyparms, int keyparm_count);
struct rollout_control
{
	Value*	name;
	create_fn creator;
};
extern ScripterExport void install_rollout_control(Value* name, create_fn creator);

class Parser : public Value				
{
	struct ParserSourceData
	{
		int			tok_start;		// store stream seek pos of token start 
		int			last_tok_end;	// store stream seek pos of end of last token 
		int			tok_start_line;		// store stream line number of token start 
		int			last_tok_end_line;	// store stream line number of end of last token 
		int			prev_last_tok_end;	// store stream seek pos of end of previous token 
		int			prev_last_tok_end_line;	// store stream line number of end of previous token 
		int			next_last_tok_end;	// store stream seek pos of end of next token 
		int			next_last_tok_end_line;	// store stream line number of end of next token 
		int			ungetch_count; // store num char ungets back to source stream
		int			eol_whitespace_start; // store position of start of whitespace at end of line
		int			eol_whitespace_end; // store position of end of whitespace at end of line
		bool		at_EOL; // store whether at end of line
		bool		spaced; // store whether detected space at end of last token

		bool		force_EOL_at_EOS; // whether to insert a EOL when reach EOS. True when doing an 'include', false when doing a resource.

		ParserSourceData(bool force_EOL_at_EOS) : force_EOL_at_EOS(force_EOL_at_EOS) {}
		void pushData(Parser& from_parser); // sets all members except force_EOL_at_EOS;
		void popData(Parser& to_parser);
	};
public:
	ScripterExport		   Parser();
	ScripterExport		   Parser(CharStream* errout);
	ScripterExport		   Parser(HashTable* scope);
	ScripterExport		   Parser(CharStream* errout, HashTable* scope);
					void   init();
	ScripterExport	Value* compile(CharStream* stream);
	ScripterExport	Value* compile_factor(CharStream* stream);
	ScripterExport	Value* compile_all(CharStream* stream);
	static			void   setup();

	static Tab<rollout_control> rollout_controls;
	static MCHAR *token_names[];

	void		collect() { delete this; }
	void		gc_trace();

	CharStream*	source;			// input stream						
	lex_token	token;			// parser's current lexical token	
	Value*		token_value;	//          and value

	bool		at_EOL;			// positioned at \r			
	bool		back_tracked;	// we back-tracked					
	bool		EOL_at_back_track; // remember EOL state at back_track 
	bool		quoted_level_name; // if path level name was quote	
	bool		spaced;			// if space after token				
	bool		space_before;	// if '-' had a space before		
	bool		space_after;	// if '-' had a space after			
	bool		throws_ok;		// indicates if empty throws are OK (only in catches) 
	bool		in_macroscript;	// compiling macroscript, disallow macroscript nesting 
	bool		no_free_refs;	// if on, disallow free refs
	bool		no_stack_refs;	// if on, disallow free and local refs
	bool		dotted_names_ok; // allows '.' as part of a name (for event handler name parsing)		
	bool		ignore_string_escapes; // ignores '\' escapes in string literals		
	bool		mzp_names;		// used by .mzp parser, allows . \ $ * in names	

	HashTable*  current_scope;	// current lexical scope hashtable	
	int			parm_count;		// current function def params		
	int			keyparm_count;	//    "        "     " keyword parms 
	int			local_count;	//    "        "     "  locals		
	int			frame_index;	// running fn frame index for locals,args 
	int			frame_level;	// running frame level				
	int			expr_level;		// running nested expression level	
	CharStream* stdout_stream;  // message & debug output stream (usually listener's window) 
	Value*		lookahead_factor; // store for factor lookahead    
	MCHAR		line_buf[256];	// current line capture for error printing 
	MCHAR*		line_buf_p;		//   "       "    "     pointer	   
	CharStream* source_capture; // non-null, capture source to this stream 
	int			ungetch_count;	
	Tab<CharStream*> src_stack; // include source stack	
	Tab<CharStream*> src_capture_stack; // include source capture stack
	Tab<ParserSourceData> src_parser_data_stack;  // include source position data stack	
	Tab<int> src_stack_encryption_keys; // include source stack encryption keys			

	int			tok_start;		// store stream seek pos of token start 
	int			last_tok_end;	// store stream seek pos of end of last token 

	int			tok_start_line;		// store stream line number of token start 
	int			last_tok_end_line;	// store stream line number of end of last token 

	// following used for backtracking
	int			prev_last_tok_end;	// store stream seek pos of end of previous token 
	int			prev_last_tok_end_line;	// store stream line number of end of previous token 
	int			next_last_tok_end;	// store stream seek pos of end of next token 
	int			next_last_tok_end_line;	// store stream line number of end of next token 

	CodeBlock*	code_block;		// top-level code block if any
	int		    code_block_level; // expr level for code block, used to provide context for 'on handler' parsing
	int			current_y, bottom_y, group_num; // used by VMS form editor construction code
	int			eol_whitespace_start; // offset to beginning of whitespace around an end-of-line
	int			eol_whitespace_end; // offset to ending of whitespace around an end-of-line

	Value*		single_expr(Value* stream);
	Value*		single_factor(Value* stream);
	Value*		compound_expr(Value* stream);
	Value*		compile_macro_script(Value* stream, MacroID id);
	bool		parse_rollout_for_edit(CharStream* source_stream, IVisualMSForm* form, Value* filename=NULL);
	Value*		compile_attributes(MSCustAttribDef* cad, CharStream* stream, Class_ID* attribID = NULL);

/* lexical analysis methods */
	bool		token_must_be(lex_token wanted_token);
	bool		next_token_must_be(lex_token wanted_token);
	bool		next_token_must_be_name(Value* wanted_name);
	void		back_track(void);
	void		back_track_factor(Value* fac);
	bool		white_space(MCHAR& c);
	void		check_for_EOL(MCHAR c);
	void		flush_EO_expr(void);
	void		reset();
	MCHAR		get_char();
	void		unget_char(MCHAR c);
	void		flush_to_eobuf();
	Value*		get_string(MCHAR delim);
	Value*		get_path_name();
	Value*		get_name_thunk(Value* name, BOOL make_new);
	Value*		get_name_thunk(Value* name, BOOL make_new, BOOL forceGlobal);
	lex_token	get_punct(MCHAR c);
	lex_token	get_token(void);
	lex_token	get_path_token(void);
	int			get_max_command(Value*& code);
	Value*		add_event_handler(HashTable* handlers, BOOL item_based = TRUE, IVisualMSForm* form = NULL);
	Value*		add_event_handler(HashTable* handlers, Value* owner, BOOL item_based = TRUE, IVisualMSForm* form = NULL);
	void		add_tool_local(MCHAR* var, MouseTool* tool, int& local_count, Value**& local_inits);
	void		add_plugin_local(MCHAR* var, MSPluginClass* plugin, int& local_count, Value**& local_inits, BOOL constant = FALSE);
	Value**		add_rollout_control(Value** controls, int control_index, Rollout* rollout, IVisualMSForm* form = NULL);
	Value**		add_rcmenu_item(Value** items, int& item_count, RCMenu* menu, BOOL subMenu = FALSE);
	void		open_include_file(MCHAR c);
	void		check_for_const_lvalue(Value*& lval);
	Value*		optimize_return(Value* expr);
	static bool	should_source_position_wrap(Value* code);
	Value*		source_position_wrap(Value* code);
	Value*		source_position_wrap(Value* code, unsigned int last_token_end,  unsigned int last_token_end_line);
	void		insert_resource(Value* resourceID, CharStream* resourceSource);

/* recursive descent parse methods */

	Value*		expr_seq(lex_token delimiter);
	Value*		vector_literal();
	Value*		index_or_vector();
	Value*		hash_literals();
	Value*		array_literal();
	Value*		bit_array_literal();
	Value*		factor();
//	Value*		property_access();
//	Value*		array_index();
	Value*		property_index_array();
	Value*		coercion();
	Value*		deref_expr();
    Value*		function_call();
	Value*		power_expr();
	Value*		uminus_expr();
	Value*		term();
	Value*		plus_expr();
	Value*		compare_expr();
	Value*		not_expr();
	Value*		and_expr();
	Value*		simple_expr();
	Value*		variable_declaration();
	Value*		context_expr();
	Value*		change_handler();
	Value*		function(MCHAR* name, short flags = 0);
	Value*		function_def();
	Value*		mapped_fn_def();
	Value*		max_command();
	Value*		handler_def();
	Value*		struct_body(Value* name);
	Value*		struct_def();
	Value*		try_catch();
	Value*		tool_body(Value* name);
	Value*		tool_def();
	Value*		rcmenu_body(Value* name);
	Value*		rcmenu_def();
	void		plugin_paramblock(MSPluginClass* plugin, Array* pblock_defs, HashTable* handlers, int base_refno);
	Value*		plugin_def();
	Value*		attributes_body(MSCustAttribDef* cad, Array* remap_param_names);
	Value*		attributes_def(MSCustAttribDef* cad = NULL, Class_ID* attribID = NULL);
	Value*		macro_script(MacroID mid = UNDEFINED_MACRO);
	Value*		rollout_body(Value* name, lex_token type = t_rollout, IVisualMSForm* form = NULL);
	Value*		utility_def();
	Value*		rollout_def();
	Value*		exit_expr();
	Value*		continue_expr();
	Value*		return_expr();
	Value*		for_expr();
	Value*		do_expr();
	Value*		while_expr();
	Value*		case_expr();
	Value*		if_expr();
	Value*		assign_expr();
	Value*		expr();
	Value*		top_level_expr();			
	Value*		function(MCHAR* name, Value* owner, short flags = 0);

private:
	bool		lastCharWasEscape; // used so multiple escapes in a row throw only a single SignalException

};

struct property_accessors
{
	MCHAR*		name;
	getter_vf	getter;
	setter_vf   setter;
};

extern property_accessors* get_property_accessors(Value* prop);
extern MCHAR* command_name_from_code(int com);

#define token_name(tok) token_names[(int)tok]

