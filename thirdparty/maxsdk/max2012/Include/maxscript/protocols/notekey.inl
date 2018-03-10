/*	
 *		noteTrack_protocol.h - protocol for MAX note tracks
 *
 *
 */
#pragma warning(push)
#pragma warning(disable:4100)

	def_visible_generic				( sortNoteKeys,			"sortNoteKeys" );
	def_visible_generic				( addNewNoteKey,		"addNewNoteKey" );
	def_visible_generic				( deleteNoteKeys,		"deleteNoteKeys" );
	def_visible_generic				( deleteNoteKey,		"deleteNoteKey" );
	def_visible_generic_debug_ok	( getNoteKeyTime,		"getNoteKeyTime" );
	def_visible_generic_debug_ok	( getNoteKeyIndex,		"getNoteKeyIndex" );

#pragma warning(pop)
