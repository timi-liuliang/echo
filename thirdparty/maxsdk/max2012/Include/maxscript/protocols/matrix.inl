/*	
 *		matrix_protocol.h - def_generics for matrix protocol
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

	use_generic						( isIdentity,		"isIdentity" );
	use_generic						( inverse,			"Inverse" );

	def_visible_primitive_debug_ok	( rotateXMatrix,	"RotateXMatrix");   
	def_visible_primitive_debug_ok	( rotateYMatrix,	"RotateYMatrix");
	def_visible_primitive_debug_ok	( rotateZMatrix,	"RotateZMatrix");
	def_visible_primitive_debug_ok	( transMatrix,		"TransMatrix");
	def_visible_primitive_debug_ok	( scaleMatrix,		"ScaleMatrix");
	def_visible_primitive_debug_ok	( rotateYPRMatrix,	"RotateYPRMatrix");

	def_visible_generic_debug_ok	( xFormMat,			"XFormMat" );
	def_mapped_generic_debug_ok		( identity,			"Identity" );
	def_mapped_generic_debug_ok		( zero,				"Zero" );
	def_mapped_generic_debug_ok		( orthogonalize,	"Orthogonalize" );

	def_mapped_generic_debug_ok		( translate,		"Translate" );
	def_mapped_generic_debug_ok		( rotateX,			"RotateX" );
	def_mapped_generic_debug_ok		( rotateY,			"RotateY" );
	def_mapped_generic_debug_ok		( rotateZ,			"RotateZ" );
	use_generic						( scale,			"Scale" );
	def_mapped_generic_debug_ok		( preTranslate,		"PreTranslate" );
	def_mapped_generic_debug_ok		( preRotateX,		"PreRotateX" );
	def_mapped_generic_debug_ok		( preRotateY,		"PreRotateY" );
	def_mapped_generic_debug_ok		( preRotateZ,		"PreRotateZ" );
	def_mapped_generic_debug_ok		( preScale,			"PreScale" );

	use_generic						( rotate,			"Rotate" );
	def_mapped_generic_debug_ok		( preRotate,		"PreRotate" );

#pragma warning(pop)

