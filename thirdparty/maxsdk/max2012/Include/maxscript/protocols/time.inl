/*	
 *		time_protocol.h - def_generics for the Time protocol
 *
 *		see def_abstract_generics.h for more info.
 *
 *	
 *			Copyright © John Wainwright 1996
 *
 */
#pragma warning(push)
#pragma warning(disable:4100)

	use_generic						( plus,			"+");
	use_generic						( minus,		"-");
	use_generic						( times,		"*");
	use_generic						( div,			"/");
	use_generic						( uminus,		"u-");

	use_generic						( eq,			"=");
	use_generic						( ne,			"!=");
	use_generic						( gt,			">");
	use_generic						( lt,			"<");
	use_generic						( ge,			">=");
	use_generic						( le,			"<=");

	use_generic						( random,		"random");
	use_generic						( abs,			"abs");

	def_visible_primitive_debug_ok	( normtime,		"normtime" );

#pragma warning(pop)
