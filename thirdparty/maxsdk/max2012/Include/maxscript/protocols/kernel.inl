/*	
 *		kernel_protocol.h - def_generics for the MAXScript kernel protocol
 *
 *		see def_abstract_generics.h for more info.
 *
 *	
 *			Copyright © John Wainwright 1996
 *
 */
#pragma warning(push)
#pragma warning(disable:4100)

	def_mapped_generic_debug_ok		( print,			"print");

	def_generic						( eq,				"==");
	def_generic						( ne,				"!=");

	def_generic						( coerce,			"coerce");
	def_node_generic				( copy,				"copy");

	def_visible_generic_debug_ok	( classOf,			"classOf");
	def_visible_generic_debug_ok	( superClassOf,		"superClassOf");
	def_visible_generic_debug_ok	( isKindOf,			"isKindOf");

	def_visible_generic				( free,				"free");

#pragma warning(pop)

