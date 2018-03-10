/*	
 *		string_protocol.h - def_generics for the string protocol
 *
 *			Copyright © John Wainwright 1996
 *
 */
#pragma warning(push)
#pragma warning(disable:4100)
	use_generic						( plus,				"+");
	use_generic						( eq,				"=");
	use_generic						( ne,				"!=");
	use_generic						( gt,				">");
	use_generic						( lt,				"<");
	use_generic						( ge,				">=");
	use_generic						( le,				"<=");
	use_generic						( get,				"get");
	use_generic						( put,				"put");

	def_visible_generic_debug_ok	( findString,		"findString");
	def_visible_generic_debug_ok	( findPattern,		"findPattern");
	def_visible_generic_debug_ok	( substring,		"substring");
	def_visible_generic_debug_ok	( replace,			"replace");

	use_generic						( append,			"append"); // LAM - 5/28/02
	use_generic						( execute,			"execute");
	use_generic						( coerce,			"coerce");
	use_generic						( copy,				"copy");

	use_generic						( free,				"free");
#pragma warning(pop)
