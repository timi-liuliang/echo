/*	
 *		vector_protocol.h - def_generics for Vector protocol
 *
 *		see def_abstract_generics.h for more info.
 *
 *	
 *			Copyright © John Wainwright 1996
 *
 */
#pragma warning(push)
#pragma warning(disable:4100)
	use_generic( plus,		"+" );
	use_generic( minus,		"-" );
	use_generic( times,		"*" );
	use_generic( div,		"/" );
	use_generic( uminus,	"u-");

	use_generic( eq,		"=");
	use_generic( ne,		"!=");

	use_generic( random,	"random");

	def_visible_generic_debug_ok	( length,		"length");
	def_visible_generic_debug_ok	( dot,			"dot");
	def_visible_generic_debug_ok	( cross,		"cross");
	def_visible_generic_debug_ok	( normalize,	"normalize");
	def_visible_generic_debug_ok	( distance,		"distance");

#pragma warning(pop)
