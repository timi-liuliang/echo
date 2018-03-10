/*	
 *		atmospherics.h - def_generics for the operations on MAX atmosphere objects
 *
 *			Copyright © John Wainwright 1996
 *
 */
 
/* gizmo operations */
#pragma warning(push)
#pragma warning(disable:4100)

#ifndef NO_ATMOSPHERICS
	def_visible_generic_debug_ok	( getGizmo,		"getGizmo");
	def_visible_generic				( deleteGizmo,	"deleteGizmo");
	def_visible_generic				( appendGizmo,	"appendGizmo");
#endif // NO_ATMOSPHERICS

#pragma warning(pop)
