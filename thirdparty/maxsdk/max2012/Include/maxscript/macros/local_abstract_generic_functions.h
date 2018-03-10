/*	
 *	local_abstract_generic_functions.h - abstract generic function definitions for MAXScript SDK plug-ins
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
	virtual Value* fn##_vf(Value** arglist, int arg_count) { ABSTRACT_FUNCTION(_M(#name), this, Value*); }
#define def_local_generic_debug_ok(fn, name)	\
	virtual Value* fn##_vf(Value** arglist, int arg_count) { ABSTRACT_FUNCTION(_M(#name), this, Value*); }
#define use_generic(fn, name)
#define use_local_generic(fn, name)

/* abstract function 'bodies'... if these are called, we have a 'type doesnt implement' error  */

#ifndef ABSTRACT_FUNCTION
#	define ABSTRACT_FUNCTION(m, v, t) throw NoMethodError (m, v); return (t)0		  
#	define ABSTRACT_CONVERTER(t, l) throw ConversionError (this, _M(#l)); return (t)0
#	define ABSTRACT_WIDENER(a) throw IncompatibleTypes (this, a); return (Value*)&undefined
#endif

