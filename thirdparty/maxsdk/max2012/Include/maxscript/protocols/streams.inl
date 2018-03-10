/*	
 *		stream_protocol.h - def_generics for Stream protocol
 *
 *			Copyright © John Wainwright 1996
 */
#pragma warning(push)
#pragma warning(disable:4100)
	def_visible_generic_debug_ok	( read_value,				"readValue");
	def_visible_generic_debug_ok	( read_expr,				"readExpr");
	def_visible_generic_debug_ok	( read_line,				"readLine");
	def_visible_generic_debug_ok	( read_char,				"readChar");
	def_visible_generic_debug_ok	( read_chars,				"readChars");
	def_visible_generic_debug_ok	( read_delimited_string,	"readDelimitedString");
	def_visible_generic_debug_ok	( skip_to_string,			"skipToString");
	def_visible_generic_debug_ok	( skip_to_next_line,		"skipToNextLine");
	def_visible_generic_debug_ok	( execute,					"execute");
	
	def_visible_generic_debug_ok	( file_pos,					"filepos");
	def_visible_generic_debug_ok	( seek,						"seek");
	def_visible_generic_debug_ok	( eof,						"eof");

	def_visible_generic				( close,					"close");
	def_visible_generic				( flush,					"flush");

	use_generic						( free,						"free");
#pragma warning(pop)
