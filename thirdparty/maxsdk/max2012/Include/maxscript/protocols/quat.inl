/*	
 *		quat_protocol.h - def_generics for quaternion protocol
 *
 *	
 *			Copyright © John Wainwright 1996
 *
 */
#pragma warning(push)
#pragma warning(disable:4100)
	use_generic						( coerce,			"coerce");

	use_generic						( plus,				"+" );
	use_generic						( minus,			"-" );
	use_generic						( times,			"*" );
	use_generic						( div,				"/" );
	use_generic						( uminus,			"u-" );

	use_generic						( eq,				"=" );
	use_generic						( ne,				"!=" );

	use_generic						( random,			"random" );

	def_visible_generic_debug_ok	( isIdentity,		"isIdentity" );
	use_generic						( normalize,		"normalize" );
	def_visible_generic_debug_ok	( inverse,			"Inverse" );
	def_visible_generic_debug_ok	( conjugate,		"Conjugate" );
	def_visible_generic_debug_ok	( logN,				"LogN" );
	use_generic						( exp,				"Exp" );
	def_visible_generic_debug_ok	( slerp,			"Slerp" );
	def_visible_generic_debug_ok	( lnDif,			"LnDif" );
	def_visible_generic_debug_ok	( qCompA,			"QCompA" );
	def_visible_generic_debug_ok	( squad,			"Squad" );
	def_visible_generic_debug_ok	( qorthog,			"qorthog" );
	def_visible_generic				( transform,		"transform" );

	def_visible_primitive_debug_ok	( squadrev,			"squadrev" );
#pragma warning(pop)
