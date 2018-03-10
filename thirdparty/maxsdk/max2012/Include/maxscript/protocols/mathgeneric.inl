/*	
 *		math_protocol.h - def_generics for the generic function in the Math protocol
 *
 *		see def_abstract_generics.h for more info.
 *
 *	
 *			Copyright © John Wainwright 1996
 *
 */
#pragma warning(push)
#pragma warning(disable:4100)

	def_generic(plus,	"+");
	def_generic(minus,	"-");
	def_generic(times,	"*");
	def_generic(div,	"/");
	def_generic(uminus,	"u-");

	use_generic(eq,		"=");
	def_generic(ne,		"!=");
	def_generic(gt,		">");
	def_generic(lt,		"<");
	def_generic(ge,		">=");
	def_generic(le,		"<=");

#pragma warning(pop)
