/*		ColorValue.h - the color class for MAXScript
 *
 *		Copyright (c) John Wainwright, 1997
 *		
 */

#pragma once

#include "..\kernel\value.h"
#include "..\..\bmmlib.h"
#include "3dmath.h"

#define COLOR_CACHE_SIZE	1024	// must be power of 2

/* ------------------------ Color ------------------------------ */

applyable_class_debug_ok (ColorValue)

class ColorValue : public Value
{
public:
	AColor		color;

	ENABLE_STACK_ALLOCATE(ColorValue);

 ScripterExport ColorValue (AColor col);
 ScripterExport ColorValue (Color col);
 ScripterExport ColorValue (COLORREF col);
 ScripterExport ColorValue (BMM_Color_64& col);
 ScripterExport ColorValue (Point3 col);
 ScripterExport ColorValue (Point3Value* col);
 ScripterExport ColorValue (float r, float g, float b, float a = 1.0f);

	static ScripterExport Value* intern(AColor col);
	static ScripterExport Value* intern(float r, float g, float b, float a = 1.0f);
	static ScripterExport Value* intern(BMM_Color_64& col);

				classof_methods (ColorValue, Value);
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);
#	define		is_color(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(ColorValue))

#include "..\macros\define_implementations.h"
#	include "..\protocols\color.inl"
	def_generic  ( coerce,	"coerce");
	def_generic  ( copy,	"copy");

	def_property		 ( red );
	def_local_prop_alias ( r, red );
	def_property		 ( green );
	def_local_prop_alias ( g, green );
	def_property		 ( blue );
	def_local_prop_alias ( b, blue );
	def_property		 ( alpha );
	def_local_prop_alias ( a, alpha );
	def_property		 ( hue );
	def_local_prop_alias ( h, hue );
	def_property		 ( saturation );
	def_local_prop_alias ( s, saturation );
	def_property		 ( value );
	def_local_prop_alias ( v, value );

	AColor		to_acolor() { return color; }
	Color		to_color() { return Color (color.r, color.g, color.b); }
	COLORREF	to_colorref() { return RGB((int)(color.r * 255.0f), (int)(color.g * 255.0f), (int)(color.b * 255.0f)); }
	Point3		to_point3() { return Point3 (color.r * 255.0, color.g * 255.0, color.b * 255.0); }
	Point4		to_point4() { return Point4 (color.r, color.g, color.b, color.a); }
	void		to_fpvalue(FPValue& v);

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

class ConstColorValue : public ColorValue
{
public:
 ScripterExport ConstColorValue (float r, float g, float b, float a = 1.0f) 
					: ColorValue(r, g, b, a) { }

	void		collect() { delete this; }
	BOOL		is_const() { return TRUE; }
#pragma warning(push)
#pragma warning(disable:4100)
	Value*		set_red(Value** arg_list, int count) { throw RuntimeError (_M("Constant color, not settable")); return NULL; }
	Value*		set_green(Value** arg_list, int count) { throw RuntimeError (_M("Constant color, not settable")); return NULL; }
	Value*		set_blue(Value** arg_list, int count) { throw RuntimeError (_M("Constant color, not settable")); return NULL; }
	Value*		set_alpha(Value** arg_list, int count) { throw RuntimeError (_M("Constant color, not settable")); return NULL; }
	Value*		set_hue(Value** arg_list, int count) { throw RuntimeError (_M("Constant color, not settable")); return NULL; }
	Value*		set_h(Value** arg_list, int count) { throw RuntimeError (_M("Constant color, not settable")); return NULL; }
	Value*		set_saturation(Value** arg_list, int count) { throw RuntimeError (_M("Constant color, not settable")); return NULL; }
	Value*		set_value(Value** arg_list, int count) { throw RuntimeError (_M("Constant color, not settable")); return NULL; }
#pragma warning(pop)
};

