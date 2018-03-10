/*		<maxscript\compiler\codetree.h> - the CodeTree class - parser output
 *
 *		Copyright (c) John Wainwright, 1996
 *		
 *
 */

#pragma once

#include "..\ScripterExport.h"
#include "..\kernel\value.h"
// forward declarations
class HashTable;

/* ------------- debugging support classes -------------- */

// SourcePositionWrapper wraps a piece of code in a source file
// context.  Evaling this pushes the 'source_pos' and 'source_line' thread-locals,
// evals the wrapped code & pops the thread-locals.

class SourcePositionWrapper : public Value
{
public:
	Value*			code;
	unsigned int	pos;		// source stream pos
	unsigned int	line;		// source stream line number
	// Constructor
	SourcePositionWrapper(Value* icode, unsigned int pos = 0, unsigned int lineNumber = 0);

	void	gc_trace();
	void	collect();
	ScripterExport void	sprin1(CharStream* s);

	Value* classOf_vf(Value** arg_list, int count) { DbgAssert(false); return code->classOf_vf(arg_list, count); }
	Value* superClassOf_vf(Value** arg_list, int count) { DbgAssert(false); return code->superClassOf_vf(arg_list, count); }
	Value* isKindOf_vf(Value** arg_list, int count) { DbgAssert(false); return code->isKindOf_vf(arg_list, count); }
	BOOL   is_kind_of(ValueMetaClass* c) { DbgAssert(false); return code->is_kind_of(c); }

	// polymorphic default type predicates - abstracted over by is_x(v) macros as needed
	BOOL	_is_collection()		{ DbgAssert(false); return FALSE; }
	BOOL	_is_charstream()		{ DbgAssert(false); return FALSE; }
	BOOL	_is_rolloutcontrol()	{ DbgAssert(false); return FALSE; }
	BOOL	_is_rolloutthunk()		{ DbgAssert(false); return FALSE; }
	BOOL	_is_function()			{ DbgAssert(false); return code->_is_function(); }
	BOOL	_is_selection()			{ DbgAssert(false); return FALSE; }
	BOOL	_is_thunk()				{ DbgAssert(false); return code->_is_thunk(); }
	BOOL	_is_indirect_thunk()	{ DbgAssert(false); return FALSE; }

	Thunk*	to_thunk()				{ DbgAssert(false); return code->to_thunk(); }

	ScripterExport Value* eval();
	ScripterExport Value* apply(Value** arglist, int count, CallContext* cc=NULL);
	ScripterExport Value* apply_no_alloc_frame(Value** arglist, int count, CallContext* cc=NULL);
	ScripterExport Value* apply(value_vf vfn_ptr, Value** arglist, int count);
};

// SourceFileWrapper wraps a piece of code in a source file
// context.  Evaling this pushes the 'source-file' thread-local,
// evals the wrapped code & pops source-file.

class SourceFileWrapper : public Value
{
public:
	Value*			file_name;
	Value*			code;
	MSZipPackage*	package;
	unsigned int	pos;		// source stream pos
	unsigned int	line;		// source stream line number
	DWORD			flags;

	// Constructor
	SourceFileWrapper(Value* file_name, Value* code, unsigned int ipos, unsigned int lineNumber, DWORD flags = 0);
#define is_sourcefilewrapper(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_SOURCEFILEWRAPPER_TAG)

	void	gc_trace();
	void	collect();
	ScripterExport void	sprin1(CharStream* s) { code->sprin1(s); }
	BOOL	_is_function() { DbgAssert(!is_sourcepositionwrapper(this)); return code->_is_function(); }

	Value* classOf_vf(Value** arg_list, int count) { return code->classOf_vf(arg_list, count); }
	Value* superClassOf_vf(Value** arg_list, int count) { return code->superClassOf_vf(arg_list, count); }
	Value* isKindOf_vf(Value** arg_list, int count) { return code->isKindOf_vf(arg_list, count); }
	BOOL	  is_kind_of(ValueMetaClass* c) { return code->is_kind_of(c); }
	ScripterExport Value* eval();
	ScripterExport Value* eval_no_wrapper();
	ScripterExport Value* apply(Value** arglist, int count, CallContext* cc=NULL);
};

class CodeTree : public Value
{
public:
	Value*	fn;				// the function to apply
	short	count;			// number of arguments
	Value**	arg_list;		// the argument list
	unsigned int pos;		// source stream pos
	unsigned int line;		// source streem line number
			CodeTree(CharStream* source, Value* codeFn, ...);
			~CodeTree();

#define is_codetree(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_CODE_TAG)
	void	gc_trace();
	void	collect();
	ScripterExport void	sprin1(CharStream* s);

    ScripterExport Value* eval();

	Value*	add(Value* arg1, ...);
	Value*	append(Value* arg);
	Value*	put(int index, Value* arg);
};

/*  Maker class, a special CodeTree node that encodes runtime object 
 *               instantiation.  contains a reference to a maker
 *               static method on the class to be instantiated.  It is
 *				 supplied an arg_list like any other codetree fn apply
 */
 
class Maker : public Value
{
	value_cf maker;			/* the maker class static fn				*/
	short	count;			/* number of arguments						*/
	Value**	arg_list;		/* the argument list						*/

public:
			Maker(value_cf maker_fn, ...);
			Maker(Value** arg_list, int count, value_cf maker_fn);
			~Maker();

#define is_maker(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_MAKER_TAG)
	void	gc_trace();
	void	collect();
	ScripterExport void	sprin1(CharStream* s);

	ScripterExport Value* eval();

	Value*	add(Value* arg1, ...);
	Value*	append(Value* arg);
};

// top-level code block, provides a storage for top-level locals
// R4: CodeBlocks can now optionally be 'structured', made of on-handlers and local decls
//     So it now supports call_handler() methods and property access to get at locals.
//     If structured, there must at least be an 'on execute' handler which becomes the main
//     executable body of the CodeBlock, and simple eval()'s on the codeblock turn into
//     call_handler(n_execute...);
//	   Further, locals in a structured codeblock are now effectively static, as are 
//     locals in rollups and plugins, they are initialized once the first time the block
//     is used and have a lifetime corresponding to the lifetime of the codeblock.  The 
//	   per-execution-lifetime locals should be moved into the body of the 'on execute'
//     handler.   -- JBW 2/29/00


class CodeBlock : public Value
{
public:
	Value*		code;			// the code
	Value**		locals;			// local variable array	
	int			local_count;	// local variable count	
	HashTable*  local_scope;	// local name space			
	HashTable*	handlers;		// handler tables	
	BOOL		initialized;	// if locals in structured macroScript have been inited
	Name*		name;			// the macroscript name, used for debugger output	

				CodeBlock();
			   ~CodeBlock();
#define is_codeblock(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_CODEBLOCK_TAG)

	void		init(Value* code, HashTable* local_scope);
	void		init_locals();

	void		collect();
	void		gc_trace();
	void		sprin1(CharStream* s);

	void		add_local();
	Value*		call_handler(Value* handler_or_name, Value** arg_list, int count);
	Value*		get_handler(Value* name);

	Value*		eval();
	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);

	// Used by debugger to dump locals and externals to standard out
	void		dump_local_vars_and_externals(int indentLevel);
};

