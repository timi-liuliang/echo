/*	
 *		def_extern_functions.h - macros for reference extern declarations
 *									for MAXScript functions
 *
 *	This will typically be used along with protocol definition files in 
 *  the code wants to reference MAXScript core function objects.
 *
 *  see def_abstract_functions.h for more info.
 *
 *	
 *	Copyright © John Wainwright 1996
 *
 */

/* def_generic macro for extern declarations */

#ifdef def_generic
#	undef def_generic
#	undef def_node_generic
#	undef def_mapped_generic
#	undef def_visible_generic
#	undef def_struct_generic
#	undef def_generic_debug_ok
#	undef def_node_generic_debug_ok
#	undef def_mapped_generic_debug_ok
#	undef def_visible_generic_debug_ok
#	undef def_struct_generic_debug_ok
#	undef use_generic
#	undef def_primitive
#	undef def_mapped_primitive
#	undef def_lazy_primitive
#	undef def_visible_lazy_primitive
#	undef def_visible_primitive
#	undef def_struct_primitive
#	undef def_primitive_debug_ok
#	undef def_mapped_primitive_debug_ok
#	undef def_lazy_primitive_debug_ok
#	undef def_visible_lazy_primitive_debug_ok
#	undef def_visible_primitive_debug_ok
#	undef def_struct_primitive_debug_ok
#	undef def_property
#	undef def_property_alias
#	undef def_2_prop_path
#	undef def_2_prop_path_alias
#	undef def_nested_prop
#	undef def_nested_prop_alias
#endif

#define def_generic(fn, name)	\
	extern Generic fn##_gf
#define def_visible_generic(fn, name) def_generic(fn, name)
#define def_struct_generic(fn, name) def_generic(fn, name)
#define def_node_generic(fn, name) def_generic(fn, name)
#define def_mapped_generic(fn, name) def_generic(fn, name)
#define use_generic(fn, name) def_generic(fn, name)

#define def_generic_debug_ok(fn, name) def_generic(fn, name)
#define def_visible_generic_debug_ok(fn, name) def_generic(fn, name)
#define def_struct_generic_debug_ok(fn, name) def_generic(fn, name)
#define def_node_generic_debug_ok(fn, name) def_generic(fn, name)
#define def_mapped_generic_debug_ok(fn, name) def_generic(fn, name)

#define def_primitive(fn, name)	\
	extern Primitive fn##_pf

#define def_lazy_primitive(fn, name) def_primitive(fn, name)
#define def_visible_lazy_primitive(fn, name) def_primitive(fn, name)
#define def_visible_primitive(fn, name) def_primitive(fn, name)
#define def_mapped_primitive(fn, name) def_primitive(fn, name)
#define def_struct_primitive(fn, _struct, name) def_primitive(fn, name)

#define def_primitive_debug_ok(fn, name) def_primitive(fn, name)
#define def_lazy_primitive_debug_ok(fn, name) def_primitive(fn, name)
#define def_visible_lazy_primitive_debug_ok(fn, name) def_primitive(fn, name)
#define def_visible_primitive_debug_ok(fn, name) def_primitive(fn, name)
#define def_mapped_primitive_debug_ok(fn, name) def_primitive(fn, name)
#define def_struct_primitive_debug_ok(fn, _struct, name) def_primitive(fn, name)

#undef def_name
#define def_name(name)	extern ScripterExport Value* n_##name;	
	
#undef def_marker
#define def_marker(var, str) extern ScripterExport Name var;

