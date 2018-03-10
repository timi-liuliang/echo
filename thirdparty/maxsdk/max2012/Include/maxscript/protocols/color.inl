/*	
 *		color_protocol.h - def_generics for MAXScript Color protocol
 *
 *		see def_abstract_generics.h for more info.
 *
 *	
 *			Author John Wainwright
 *			Copyright © Autodesk Inc., 1997
 *
 */
#pragma warning(push)
#pragma warning(disable:4100)
	use_generic						( plus,			"+" );
	use_generic						( minus,		"-" );
	use_generic						( times,		"*" );
	use_generic						( div,			"/" );
	use_generic						( uminus,		"u-");

	use_generic						( eq,			"=");
	use_generic						( ne,			"!=");

	use_generic						( random,		"random");
	def_visible_generic_debug_ok	( composite,	"composite");
#pragma warning(pop)
