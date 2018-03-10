// Protocol for BigMatrix class

#pragma warning(push)
#pragma warning(disable:4100)	

	use_generic						( get,			"get");
	use_generic						( put,			"put");	
	use_generic						( identity,		"identity");	//Should actually be mapped_generic
	use_generic						( plus,			"+");

	def_visible_generic_debug_ok	( invert,		"invert");	
	def_visible_generic_debug_ok	( transpose,	"transpose");
	def_visible_generic_debug_ok	( clear,		"clear");
	def_visible_generic_debug_ok	( setSize,		"setSize");

#pragma warning(pop)
	
