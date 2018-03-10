/*	
 *		ObjectSets.h - ObjectSet classes for MAXScript
 *
 *			Copyright (c) John Wainwright 1996
 *
 */

#pragma once

#include "..\kernel\value.h"
#include "..\foundation\collection.h"
#include "..\foundation\streams.h"

visible_class (Set)

class Set : public Value, public Collection
{
public:
			classof_methods (Set, Value);
	BOOL	_is_collection() { return 1; }

#include "..\macros\define_implementations.h"
	def_generic  (coerce,   "coerce"); 
};

visible_class_s (ObjectSet, Set)

class ObjectSet : public Set
{
protected:
				ObjectSet() { }
public:
	MCHAR*		set_name;
	BOOL		(*selector)(INode* node, int level, void* arg);	// set selector function
	void*		selector_arg;									// selector fn argument

				ObjectSet(MCHAR* name, SClass_ID class_id);
				ObjectSet(MCHAR* init_name, BOOL (*sel_fn)(INode*, int, void*), void* init_arg = NULL);
	void		init(MCHAR* name);

				classof_methods (ObjectSet, Set);
	static void	setup();
	MCHAR*		name() { return set_name; }
	void		collect() { delete this; }
	void		sprin1(CharStream* s) { s->printf(_M("$%s"), set_name); }
	void		export_to_scripter();
#	define		is_objectset(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(ObjectSet))

	ScripterExport Value* map(node_map& m);
	ScripterExport Value* map_path(PathName* path, node_map& m);
	ScripterExport Value* find_first(BOOL (*test_fn)(INode* node, int level, void* arg), void* test_arg);
	ScripterExport Value* get_path(PathName* path);

#include "..\macros\define_implementations.h"
	def_generic  (get,		"get");   // indexed get (no put or append)
	def_property ( count );
	def_property ( center );
	def_property ( min );
	def_property ( max );
};

class CurSelObjectSet : public ObjectSet
{
public:
				CurSelObjectSet(MCHAR* name);

	void		collect() { delete this; }

	ScripterExport Value* map(node_map& m);

#include "..\macros\define_implementations.h"
	def_generic  (get,		"get");   // indexed get (no put or append)
	def_generic  (coerce,   "coerce"); 

	def_property ( count );
};

extern ObjectSet all_objects;
extern ObjectSet all_geometry;
extern ObjectSet all_lights;
extern ObjectSet all_cameras;
extern ObjectSet all_helpers;
extern ObjectSet all_shapes;
extern ObjectSet all_systems;
extern ObjectSet all_spacewarps;
extern CurSelObjectSet current_selection;

