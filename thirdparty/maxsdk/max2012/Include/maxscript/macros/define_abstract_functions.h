/*	
 *		def_abstract_functions.h - macros for making abstract declarations for MAXScript functions
 *
 *	This will typically be used along with protocol definition files in Value.h
 *	to declare all the possible scripter-visible operations on MAXScript values.
 *
 *	MAXScript generics are scripter-visible first-class values that wrap a
 *	reference to a virtual function declared on class Value.  All core generics
 *	are defined this way via a set of defining macros and protocol files for
 *  each suite of operations.  The core generics are all globally declared
 *  Generic instances, with names of the form xxx_gf. Corresponding virtuals
 *  are declared abstractly in class Value and have names of the form xxx_vf.
 *
 *  All thusly declared genrics have the same signature:  
 *			Value* xxx_vf(Value** arglist, int arg_count);
 *	That is, they are polymorphic up to Values and take an argument list array &
 *  actual count which they should check for conformance at runtime.
 *
 *  Value subclasses that implement a particular protocol should use 
 *  def_implement_generics.h macros to declare implementations.  And one
 *  of these classes should use def_instance_generics.h to actually globally
 *  instantiate the generic objects.  The abstract declarations in Value
 *  all contain type error signals, so that any generic invoked on a
 *  Value subclass instance that doesn't implement the generic will
 *  get a runtime type error.
 *
 *  Similarly, scripter-visible primitive functions (non-generic) are 
 *  represented by instances of the Primitive class and declared using
 *  these same def macro headers & corresponding def_primitive macros.
 *	
 *	Copyright © John Wainwright 1996
 *
 */

/* def_generic macro for abstract declaration in Value class */

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
#ifdef def_prop_getter
#	undef def_prop_getter
#	undef def_prop_setter
#endif

#define def_generic(fn, name)	\
	virtual Value* fn##_vf(Value** arglist, int arg_count) { ABSTRACT_FUNCTION(_M(#name), this, Value*); }
#define def_visible_generic(fn, name) def_generic(fn, name)
#define def_struct_generic(fn, name) def_generic(fn, name)
#define def_node_generic(fn, name) def_generic(fn, name)
#define def_mapped_generic(fn, name) def_generic(fn, name)
#define use_generic(fn, name)

#define def_generic_debug_ok(fn, name) def_generic(fn, name)
#define def_visible_generic_debug_ok(fn, name) def_generic(fn, name)
#define def_struct_generic_debug_ok(fn, name) def_generic(fn, name)
#define def_node_generic_debug_ok(fn, name) def_generic(fn, name)
#define def_mapped_generic_debug_ok(fn, name) def_generic(fn, name)

#define def_primitive(fn, name)		// no member function declarations for primitives
#define def_visible_primitive(fn, name)
#define def_mapped_primitive(fn, name)
#define def_lazy_primitive(fn, name)
#define def_visible_lazy_primitive(fn, name)
#define def_struct_primitive(fn, _struct, name)

#define def_primitive_debug_ok(fn, name)		// no member function declarations for primitives
#define def_visible_primitive_debug_ok(fn, name)
#define def_mapped_primitive_debug_ok(fn, name)
#define def_lazy_primitive_debug_ok(fn, name)
#define def_visible_lazy_primitive_debug_ok(fn, name)
#define def_struct_primitive_debug_ok(fn, _struct, name)

#define def_property(p)																						\
			virtual Value*	get_##p(Value** arg_list, int count) { return get_property(arg_list, count); }	\
			virtual Value*	set_##p(Value** arg_list, int count) { return set_property(arg_list, count); }	
#define def_property_alias(p, real_p)
#define def_2_prop_path(p1, p2)												\
			virtual Value*	get_##p1##_##p2(Value** arg_list, int count)	\
			{																\
				two_value_locals(cont, result);								\
				vl.cont = get_##p1(&n_##p1, 1);								\
				vl.result = vl.cont->get_##p2(&n_##p2, 1);					\
				return_value(vl.result);									\
			}																\
			virtual Value*	set_##p1##_##p2(Value** arg_list, int count)	\
			{																\
				one_value_local(cont);										\
				Value* args[2] = { arg_list[0], n_##p2 };					\
				vl.cont = get_##p1(&n_##p1, 1);								\
				vl.cont->set_##p2(args, 2);									\
				args[0] = vl.cont; args[1] = n_##p1;						\
				set_##p1(args, 2);											\
				pop_value_locals();											\
				return arg_list[0];											\
			}
#define def_2_prop_path_alias(p1, p2, real_p1, real_p2)
#define def_nested_prop(p1)													\
			virtual Value*	get_nested_##p1(Value** arg_list, int count)	\
			{																\
				two_value_locals(cont, result);								\
				vl.cont = _get_property(arg_list[0]);						\
				vl.result = vl.cont->get_##p1(&n_##p1, 1);					\
				return_value(vl.result);									\
			}																\
			virtual Value*	set_nested_##p1(Value** arg_list, int count)	\
			{																\
				one_value_local(cont);										\
				Value* args[2] = { arg_list[0], n_##p1 };					\
				vl.cont = _get_property(arg_list[1]);						\
				vl.cont->set_##p1(args, 2);									\
				_set_property(arg_list[1], vl.cont);						\
				pop_value_locals();											\
				return arg_list[0];											\
			}

#define def_nested_prop_alias(p1, real_p1)									

/* abstract function 'bodies'... if these are called, we have a 'type doesnt implement' error  */

#define ABSTRACT_FUNCTION(m, v, t) throw NoMethodError (m, v); return (t)0		  
#define ABSTRACT_CONVERTER(t, l) throw ConversionError (this, _M(#l)); return (t)0
#define ABSTRACT_WIDENER(a) throw IncompatibleTypes (this, a); return (Value*)&undefined
#define ABSTRACT_GETTER() throw AccessorError (this, arg_list[0]); return (Value*)&undefined
#define ABSTRACT_SETTER() throw AccessorError (this, arg_list[1]); return (Value*)&undefined

