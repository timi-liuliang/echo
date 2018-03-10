/*	
 *		array_protocol.h - def_generics for Array protocol
 *
 *		see def_abstract_generics.h for more info.
 *
 *	
 *			Copyright © John Wainwright 1996
 *
 */
#pragma warning(push)
#pragma warning(disable:4100)
	def_generic						(get,			"get");
	def_generic						(put,			"put");
	def_visible_generic				(append,		"append");
	def_visible_generic				(deleteItem,	"deleteItem");
	def_visible_generic_debug_ok	(findItem,		"findItem");
	def_visible_generic				(join,			"join");
	def_visible_generic				(sort,			"sort");
	
	def_visible_primitive_debug_ok	( deepCopy,		"deepCopy" );
#pragma warning(pop)
