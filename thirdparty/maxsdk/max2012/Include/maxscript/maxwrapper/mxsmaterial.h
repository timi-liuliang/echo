/*	
 *		MAXMaterials.h - MAX material & map wrapper classes
 *
 *			Copyright (c) John Wainwright 1996
 *
 */

#pragma once

#include "mxsobjects.h"
// forward declarations
class MultiMtl;


/* -------------------------- MAXMaterial -------------------------- */

visible_class (MAXMaterial)

class MAXMaterial : public MAXWrapper
{
public:
	Mtl*		mat;				/* the MAX-side material	*/

				MAXMaterial() {};
				MAXMaterial(Mtl* imat);
	static ScripterExport Value* intern(Mtl* imat);

	static Value* make(MAXClass* cls, Value** arg_list, int count);

	BOOL		is_kind_of(ValueMetaClass* c) { return (c == class_tag(MAXMaterial)) ? 1 : MAXWrapper::is_kind_of(c); }
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);
	MCHAR*		class_name();

#include "..\macros\define_implementations.h"
	Value*		copy_vf(Value** arg_list, int count) { return MAXWrapper::copy_no_undo(arg_list, count); }

	def_property   ( name );
	def_property   ( effectsChannel );
	def_property   ( showInViewport );
	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);

	Mtl*		to_mtl() { check_for_deletion(); return mat; }
	MtlBase*	to_mtlbase() { check_for_deletion(); return mat; }
	void		to_fpvalue(FPValue& v) { v.mtl = mat; v.type = TYPE_MTL; }
};

/* ---------------------- MAXMultiMaterial ----------------------- */

visible_class (MAXMultiMaterial)

class MAXMultiMaterial : public MAXMaterial
{
public:

				MAXMultiMaterial() {};
				MAXMultiMaterial(MultiMtl* imat);
	static ScripterExport Value* intern(MultiMtl* imat);

	static Value* make(MAXClass* cls, Value** arg_list, int count);

	BOOL		is_kind_of(ValueMetaClass* c) { return (c == class_tag(MAXMultiMaterial)) ? 1 : MAXMaterial::is_kind_of(c); }
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);
	MCHAR*		class_name();

	def_generic (get, "get");
	def_generic (put, "put");
	Value*		copy_vf(Value** arg_list, int count) { return MAXWrapper::copy_no_undo(arg_list, count); }

	ScripterExport Value* map(node_map& m);

	def_property( numsubs );
	def_property( count );

	Mtl*		to_mtl() { return mat; }
	MtlBase*	to_mtlbase() { check_for_deletion(); return mat; }
	void		to_fpvalue(FPValue& v) { v.mtl = mat; v.type = TYPE_MTL; }
};

/* ---------------------- Material Library ----------------------- */

applyable_class (MAXMaterialLibrary)

class MAXMaterialLibrary : public MAXWrapper
{
public:
	MtlBaseLib	new_lib;
	MtlBaseLib&	lib;

				MAXMaterialLibrary(MtlBaseLib& ilib);
				MAXMaterialLibrary(MtlBaseLib* ilib);
				MAXMaterialLibrary();
	static ScripterExport Value* intern(MtlBaseLib& ilib);
	static ScripterExport Value* intern(MtlBaseLib* ilib);
#	define		is_materiallibrary(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(MAXMaterialLibrary))

				classof_methods (MAXMaterialLibrary, MAXWrapper);
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);
	MtlBase*	get_mtlbase(int index);
	MtlBase*	find_mtlbase(MCHAR* name);
	MCHAR*		class_name();

	/* operations */
	
	ScripterExport Value* map(node_map& m);

#include "..\macros\define_implementations.h"
#	include "..\protocols\arrays.inl"
	Value*		copy_vf(Value** arg_list, int count) { return MAXWrapper::copy_no_undo(arg_list, count); }

	/* built-in property accessors */

	def_property ( count );
};

/* ------------------------- MAXTexture ------------------------ */

visible_class (MAXTexture)

class MAXTexture : public MAXWrapper
{
public:
	Texmap*	map;				/* the MAX-side map	*/

				MAXTexture(Texmap* imap);
	static ScripterExport Value* intern(Texmap* imap);

	static Value* make(MAXClass* cls, Value** arg_list, int count);

	BOOL		is_kind_of(ValueMetaClass* c) { return (c == class_tag(MAXTexture)) ? 1 : MAXWrapper::is_kind_of(c); }
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);
	MCHAR*		class_name();

#include "..\macros\define_implementations.h"
#	include "..\protocols\textures.inl"
	Value*		copy_vf(Value** arg_list, int count) { return MAXWrapper::copy_no_undo(arg_list, count); }

	def_property( name );

	Texmap*		to_texmap() { check_for_deletion(); return map; }
	MtlBase*	to_mtlbase() { check_for_deletion(); return map; }
	void		to_fpvalue(FPValue& v) { v.tex = map; v.type = TYPE_TEXMAP; }
};

/* -------------------------- MAXMtlBase -------------------------- */

// a generic wrapper for MtlBase classes such as UVGen, XYZGen, TexOut, etc.

visible_class (MAXMtlBase)

class MAXMtlBase : public MAXWrapper
{
public:
	MtlBase*	mtl;				/* the MAX-side mtlbase	*/

				MAXMtlBase(MtlBase* imtl);
	static ScripterExport Value* intern(MtlBase* imtl);

	static Value* make(MAXClass* cls, Value** arg_list, int count);

	BOOL		is_kind_of(ValueMetaClass* c) { return (c == class_tag(MAXMtlBase)) ? 1 : MAXWrapper::is_kind_of(c); }
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);
	MCHAR*		class_name() { return _M("MapSupportClass"); }
	Value*		copy_vf(Value** arg_list, int count) { return MAXWrapper::copy_no_undo(arg_list, count); }

	MtlBase*	to_mtlbase() { check_for_deletion(); return mtl; }
};

/* ------------------ MEdit materials virtual array -------------------- */

visible_class (MAXMeditMaterials)

class MAXMeditMaterials : public MAXWrapper
{
public:
				MAXMeditMaterials();

				classof_methods (MAXMeditMaterials, MAXWrapper);
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);
	MtlBase*	get_mtl(int index);
	MtlBase*	find_mtl(MCHAR* name);
	static void setup();
	MCHAR*		class_name();

	// operations
	ScripterExport Value* map(node_map& m);

#include "..\macros\define_implementations.h"
#	include "..\protocols\arrays.inl"

	// built-in property accessors
	def_property ( count );
};

extern ScripterExport MAXMeditMaterials medit_materials;

