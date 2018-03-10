/*	
 *		thread_locals.h - thread locals for each interpreter thread in MAXScript
 *
 *			Copyright © John Wainwright 1996
 *
 */

/* thread locals and initial values */
#pragma warning(push)
#pragma warning(disable:4100)

// This file describes the thread local variables. This file is included in various contexts where the definition of 'def_thread_local'
// is context specific. For example, in one place type, 'def_thread_local' is used to declare the thread local variable, in another it
// is used to initialize the thread local variable, in another it is used during garbage collection to mark Value* derived values
// as being in use.

// Column 1 is the type of the thread local variable
// Column 2 is the thread local variable's name
// Column 3 is whether the type derives from Value*, and the contents of the thread local variable should be protected against garbage collection
// Column 4 is the value used to initialize the thread local variable

	def_thread_local( CharStream*,			current_stdout,			TRUE,	new (GC_IN_HEAP) WindowStream(_M("Script Output")));
	def_thread_local( BOOL,					force_listener_open,	FALSE,	TRUE);			// whether to force listener open on output to it

	def_thread_local( Value**,				current_frame,			FALSE,	NULL);			// current interpreter frame (for thunk evals)
	def_thread_local( Value**,				current_scan_frame,		FALSE,	NULL);			// current interpreter frame (for gc scanner) 
	def_thread_local( Value**,				current_locals_frame,	FALSE,	NULL);			// C++ local frame
	def_thread_local( Value*,				current_result,			TRUE,	NULL);			// C++ current Value* function result
	def_thread_local( long,					stack_limit,			FALSE,	ALLOCATOR_STACK_SIZE);	// max stack size to catch recurse loops, 1Mb to start, 
																									// minus buffer to handle calls made during error handling
	def_thread_local( LONG_PTR,				stack_base,				FALSE,	(LONG_PTR)_alloca(sizeof(int)));	// current stack base
	def_thread_local( MSPlugin*,			current_plugin,			TRUE,	NULL);			// current scripted plugin (for plugin thunk evals)
	def_thread_local( Struct*,				current_struct,			TRUE,	NULL);			// current struct (for struct member thunk evals)
	def_thread_local( Value*,				current_container,		TRUE,	NULL);			// current container for nested property access
	def_thread_local( int,					container_index,		FALSE,	0);				// current container index (if any)
	def_thread_local( Value*,				container_prop,			TRUE,	NULL);			// current container prop (if any)
	def_thread_local( Value*,				current_prop,			TRUE,	NULL);			// most recent prop access (if any)

	def_thread_local( Value*,				source_file,			TRUE,	NULL);			// current source file
	def_thread_local( UINT_PTR,				source_pos,				FALSE,	0);				// current pos in source file
	def_thread_local( UINT_PTR,				source_line,			FALSE,	0);				// current pos in source file, instead of INT_PTR, will have an int to simplify things

	def_thread_local( int,					needs_redraw,			FALSE,	0);				// 0- no redraw needed, 1 - redraw needed, 2 - complete redraw needed
	def_thread_local( BOOL,					redraw_mode,			FALSE,	1);				// redraw on
	def_thread_local( BOOL,					pivot_mode,				FALSE,	0);				// pivot off
	def_thread_local( BOOL,					undo_mode,				FALSE,	1);				// undo on
	def_thread_local( Value*,				current_level,			TRUE,	&all_objects);	// $objects
	def_thread_local( BOOL,					use_time_context,		FALSE,	0);				// use MAX time slider
	def_thread_local( TimeValue,			current_time,			FALSE,	0);
	def_thread_local( Value*,				current_coordsys,		TRUE,	n_default);
	def_thread_local( Value*,				center_mode,			TRUE,	n_default);

	def_thread_local( int,					rand_accum,				FALSE,	0);				// for our own rand()
	def_thread_local( HANDLE,				message_event,			FALSE,	NULL);			// listener_message synch event
	def_thread_local( int,					stream_rand_accum,		FALSE,	0);				// for stream_rand()

	def_thread_local( MSZipPackage*,		current_pkg,			TRUE,	NULL);			// currently open zip package, if any

	def_thread_local( void*,				alloc_frame,			FALSE,	NULL);			// top frame of allocator stack
	def_thread_local( void*,				alloc_tos,				FALSE,	NULL);			// top of allocator stack
	def_thread_local( void*,				alloc_stack_lim,		FALSE,	NULL);			// limit of allocator stack

	def_thread_local( Control*,				current_controller,		FALSE,	NULL);			// currently evaluating scripted controller

	def_thread_local( String*,				undo_label,				TRUE,	new (GC_PERMANENT) String(_M("MAXScript"))); // current undo label
	def_thread_local( BOOL,					try_mode,				FALSE,	0);				// try(...)
	def_thread_local( MAXScriptException*,	current_exception,		FALSE,	NULL);			// current exception that was thrown, if any. Non-null only in catch expression

	def_thread_local( BOOL,					thread_not_active,		FALSE,	FALSE);			// set to TRUE when thread found not to be active any more

	def_thread_local( BOOL,					err_occurred,			FALSE,	FALSE);			// set to TRUE when error thrown
	def_thread_local( Value*,				err_source_file,		TRUE,	NULL);			// source file where error thrown
	def_thread_local( UINT_PTR,				err_source_pos,			FALSE,	0);				// pos in source file where error thrown
	def_thread_local( UINT_PTR,				err_source_line,		FALSE,	0);				// line in source file where error thrown

	def_thread_local( Value*,				current_frame_owner,	TRUE,	NULL);			// owner of current stack frame while compiling
	
	def_thread_local( BOOL,					is_dubugger_thread,		FALSE,	FALSE);			// owner of current stack frame while compiling

	def_thread_local( DWORD,				source_flags,			FALSE,	0);				// current source file flags
	def_thread_local( DWORD,				err_source_flags,		FALSE,	0);				// source file flags for source file where error thrown

	def_thread_local( BOOL,					trace_back_active,		FALSE,	FALSE);			// true if have started tracing up the stack due to an error
	def_thread_local( BOOL,					disable_trace_back,		FALSE,	FALSE);			// true if no stack traceback is to occur if an error occurs
	def_thread_local( int,					trace_back_levels,		FALSE,	0);				// when tracing up the stack due to an error, how many stack frames we have traced so far

	def_thread_local( StringStream*,		resource_value_reader,	TRUE,	NULL);			// Stringstream used to prevalidate resource Values read from resource files

#pragma warning(pop)
