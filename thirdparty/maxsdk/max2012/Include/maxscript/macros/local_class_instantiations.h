/*	
 *	local_class_instantiations.h -  generic function instantiation macros for MAXScript SDK plug-ins
 */

#include "ClassCfg.h"
#include "..\util\IMXSDebugger.h"

#ifdef def_local_generic
#	undef def_local_generic
#	undef def_local_generic_debug_ok
#	undef use_local_generic
#endif
#ifdef use_generic
#	undef use_generic
#endif


#pragma pointers_to_members(full_generality, virtual_inheritance)

#define def_local_generic(fn, name)										\
	MS_LOCAL_GENERIC_CLASS fn##_gf (_M(name), &MS_LOCAL_ROOT_CLASS::fn##_vf)
#define def_local_generic_debug_ok(fn, name)										\
	MS_LOCAL_GENERIC_CLASS fn##_gf (_M(name), &MS_LOCAL_ROOT_CLASS::fn##_vf)
#define use_generic(fn, name)
#define use_local_generic(fn, name)

#undef def_name
#define def_name(name) Value* n_##name;	
