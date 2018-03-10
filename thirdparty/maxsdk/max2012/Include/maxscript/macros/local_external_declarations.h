/*	
 *	local_external_declarations.h - abstract generic function definitions for MAXScript SDK plug-ins
 *  Currently this file is not used anywhere in max.
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
	extern MS_LOCAL_GENERIC_CLASS fn##_gf
#define def_local_generic_debug_ok(fn, name)	\
	extern MS_LOCAL_GENERIC_CLASS fn##_gf
#define use_local_generic(fn, name) \
	def_local_generic(fn, name)
#define use_generic(fn, name) \
	extern Generic fn##_gf

#undef def_name
#define def_name(name)	extern Value* n_##name;	
