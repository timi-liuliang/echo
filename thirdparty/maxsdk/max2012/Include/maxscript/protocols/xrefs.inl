// xrefs_protocol.h - def_generics for XRefs protocol
#pragma warning(push)
#pragma warning(disable:4100)

	def_visible_generic	( merge_xref_file,			"merge");
	use_generic			( updateXRef,				"updateXRef");
	def_visible_generic	( flag_xref_changed,		"flagChanged");	
	use_generic			( delete,					"delete");

#pragma warning(pop)
