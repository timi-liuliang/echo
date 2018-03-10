/*	
 *		NamedSet.h - scripter access to named node selection sets 
 *
 *			John Wainwright
 *			Copyright (c) Autodesk, Inc. 1997
 *
 */

#pragma once

#include "..\kernel\value.h"
#include "..\foundation\collection.h"

/* ---------------------- MAXNamedSetArray ----------------------- */

// provides array-like access to the table of named selection sets

visible_class (MAXNamedSetArray)

class MAXNamedSetArray : public Value, public Collection
{
public:
				MAXNamedSetArray();

				classof_methods (MAXNamedSetArray, Value);
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);

	// operations
	ScripterExport Value* map(node_map& m);

#include "..\macros\define_implementations.h"
#	include "..\protocols\arrays.inl"

	// built-in property accessors
	def_property ( count );

};

/* ---------------------- MAXNamedSet ----------------------- */

visible_class (MAXNamedSet)

class MAXNamedSet : public Value, public Collection
{
public:
	MSTR		name;

				MAXNamedSet(MCHAR* iname);
#	define		is_namedset(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(MAXNamedSet))

				classof_methods (MAXNamedSet, Value);
	BOOL		_is_collection() { return 1; }
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);

	// operations
	ScripterExport Value* map(node_map& m);

#include "..\macros\define_implementations.h"
#	include "..\protocols\arrays.inl"

	// built-in property accessors
	def_property ( count );
	def_property ( center );
	def_property ( min );
	def_property ( max );
	def_property ( name );
};

extern MAXNamedSetArray theNamedSetArray;


