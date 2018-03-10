/*	
 *	local_implementations.h -  generic function implementation macros for MAXScript SDK plug-ins
 */

#include "ClassCfg.h"

#ifdef def_local_generic
#	undef def_local_generic
#	undef def_local_generic_debug_ok
#	undef use_local_generic
#endif
#ifdef use_generic
#	undef use_generic
#endif


#define def_local_generic(fn, name)	\
	Value* fn##_vf(Value** arglist, int arg_count)
#define def_local_generic_debug_ok(fn, name)	\
	Value* fn##_vf(Value** arglist, int arg_count)
#define use_local_generic(fn, name)	\
	def_local_generic(fn, name)
#define use_generic(fn, name)		\
	Value* fn##_vf(Value** arglist, int arg_count)

#undef def_name
#define def_name(name)	n_##name = Name::intern(_M(#name));		



