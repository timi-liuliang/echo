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

	def_generic						(plus,		"+");
	def_generic						(minus,		"-");
	def_generic						(times,		"*");
	def_generic						(div,		"/");
	def_generic						(pwr,		"^");
	def_generic						(uminus,	"u-");

	use_generic						(eq,		"=");
	use_generic						(ne,		"!=");
	def_generic_debug_ok			(gt,		">");
	def_generic_debug_ok			(lt,		"<");
	def_generic_debug_ok			(ge,		">=");
	def_generic_debug_ok			(le,		"<=");

	def_visible_generic_debug_ok	( random,	"random");
	def_visible_generic_debug_ok	( abs,		"abs");

	/* scripter-visible math primitives - implemented as prims since they are type-specific */
	
	def_visible_generic_debug_ok	( exp,		"exp");  // exp is polymorphic (floats, ints & quats)
	
	def_visible_generic_debug_ok	( acos,		"acos");  // these are polymorphic (floats & ints)
	def_visible_generic_debug_ok	( asin,		"asin");
	def_visible_generic_debug_ok	( atan,		"atan");
	def_visible_generic_debug_ok	( ceil,		"ceil");
	def_visible_generic_debug_ok	( cos,		"cos");
	def_visible_generic_debug_ok	( cosh,		"cosh");
	def_visible_generic_debug_ok	( floor,	"floor");
	def_visible_generic_debug_ok	( log,		"log");
	def_visible_generic_debug_ok	( log10,	"log10");
	def_visible_generic_debug_ok	( sin,		"sin");
	def_visible_generic_debug_ok	( sinh,		"sinh");
	def_visible_generic_debug_ok	( sqrt,		"sqrt");
	def_visible_generic_debug_ok	( tan,		"tan");
	def_visible_generic_debug_ok	( tanh,		"tanh");
 
	def_visible_generic_debug_ok	( atan2,	"atan2");
	def_visible_generic_debug_ok	( fmod,		"mod");
	def_visible_generic_debug_ok	( pow,		"pow");
	
	def_visible_primitive			( include,	"include");
	def_visible_primitive_debug_ok	( seed,		"seed");

#pragma warning(pop)

