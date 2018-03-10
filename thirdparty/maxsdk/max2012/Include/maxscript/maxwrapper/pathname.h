/*	
 *		PathName.h - PathName class for MAXScript
 *
 *			Copyright (c) John Wainwright 1996
 *
 */

#pragma once

#include "..\foundation\streams.h"
#include "objectsets.h"

enum path_flags { rooted_path = 1, wild_card_path = 2 };

visible_class_debug_ok (PathName)

class PathName : public Set
{
public:
	int			flags;
	short		n_levels;
	MCHAR**		path_levels;
	Thunk*		root_set_thunk;
	Value*		root_set;

				PathName();
				~PathName();
#	define		is_pathname(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(PathName))

				classof_methods (PathName, Set);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);

	ScripterExport Value* eval();
	void		append(MCHAR* level_name);
	Value*		find_object(MCHAR* name);
	Value*		get_object();

	ScripterExport Value* map(node_map& m);

#include "..\macros\define_implementations.h"
	def_generic  (get,		"get");   // indexed get (no put or append)
	def_property ( count );
	def_property ( center );
	def_property ( min );
	def_property ( max );
};

extern MCHAR* ellipsis_level_name;
extern MCHAR* parent_level_name;

// Added: 12/17/03
// This class provides an interface to a derived implementation class that caches scene node names
// based on the node name resolution scheme used by MAXScript. There is a single, static instance 
// of the derived class.
// usage: SceneNodeByNameCache::GetInst().GetSceneNodeByName(theName,true,false);
class SceneNodeByNameCache
{
public:
	ScripterExport virtual ~SceneNodeByNameCache();
	// get the singleton instance of SceneNodeByNameCache
	ScripterExport static SceneNodeByNameCache& GetInst();
	// get the INode* given the name. If exactMatch is false, use inexact (MXS) name matching. Can return NULL
	ScripterExport virtual INode *GetSceneNodeByName(const MCHAR* name, bool exactMatch = false, bool caseSensitive = false) = 0;
};

