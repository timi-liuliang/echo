/*		Collection.h - MAXScript Collection classes
 *
 *		Copyright (c) John Wainwright, 1996
 *		
 *
 */

#pragma once

class PathName;
#include "..\kernel\value.h" // for value_vf

class Collection
{
public:
#	define  is_collection(v) ((v)->_is_collection())

	virtual Value* map_path(PathName* path, value_vf vfn_ptr, value_cf cfn_ptr, Value** arg_list, int count);
	virtual Value* find_first(BOOL (*test_fn)(INode* node, int  level, void* arg), void* test_arg);
	virtual Value* get_path(PathName* path_name);
};

class SelectionIterator
{
public:
#	define  is_selection(v) ((v)->_is_selection())
	virtual int next() { return -1; }
	virtual BOOL more() { return FALSE; }
	virtual void rewind() { }
	virtual BOOL selected(int index) { UNUSED_PARAM(index); return FALSE; }
};


