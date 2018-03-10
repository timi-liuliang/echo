/*		Structs.h - the MAXSript struct definition classes
 *
 *		Copyright (c) John Wainwright, 1996
 *		
 */

#pragma once

#include "..\kernel\value.h"
#include "functions.h"
// forward declarations
class HashTabMapper;
class StructMemberThunk;

visible_class_debug_ok (StructDef)

class StructDef : public Value
{
	friend StructMemberThunk;
	friend Struct;
	friend void Function::export_to_scripter();
private:
	Value*		name;						/* struct's global var name */
	Value**		member_inits;				/* member init vals			*/
	int			member_count;				/*   "    count				*/
	HashTable*	members;					/* member name to index table */
	HashTable*	member_isPublicAccess;		/* member name is public?   */
	HashTable*	handlers;					/* event handlers */

public:

	// note: if members is null, a default hashtable is created in the ctor
	// if member_isPublicAccess is null, all members are considered public. If member cannot be found in member_isPublicAccess, member is considered public
	ScripterExport StructDef(Value* name, int member_count, Value** inits, HashTable* members, HashTable* member_isPublicAccess, HashTable* handlers);
				~StructDef();
				classof_methods (StructDef, Value);
#	define		is_structdef(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(StructDef))

	void		collect() { delete this; }
	void		gc_trace();
	ScripterExport void		sprin1(CharStream* s);

#include "..\macros\define_implementations.h"
	def_generic ( get_props,	"getPropNames"); // LAM: added 4/27/00
	def_generic ( show_props,	"showProperties");

	ScripterExport Value* apply(Value** arglist, int count, CallContext* cc=NULL);
	ScripterExport Value* get_property(Value** arg_list, int count);
	ScripterExport Value* set_property(Value** arg_list, int count);
	ScripterExport bool   IsPropertyPublic(Value* prop);
	ScripterExport Value* Name();
	ScripterExport void   map_members(HashTabMapper* mapper);
	ScripterExport void	  map_members(void (*fn)(const void* key, const void* val, void* arg), void* arg);
	ScripterExport Value* get_member_value(Value* key);
	ScripterExport Value* put_member_value(Value* key, Value* value);
};

class Struct : public Value
{
public:
	StructDef*	definition;					/* pointer to my struct def							*/
	Value**		member_data;				/* data elements, indexed via struct def hashtable	*/

	ScripterExport Struct(StructDef* idef, int mem_count);
				~Struct();

#	define		is_struct(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_STRUCT_TAG)
	Value*		classOf_vf(Value** arg_list, int count);
	Value*		superClassOf_vf(Value** arg_list, int count);
	Value*		isKindOf_vf(Value** arg_list, int count);
	BOOL		is_kind_of(ValueMetaClass* c) { return (c == class_tag(StructDef)) ? 1 : Value::is_kind_of(c); }

	void		collect() { delete this; }
	void		gc_trace();
	ScripterExport void		sprin1(CharStream* s);

#include "..\macros\define_implementations.h"
	use_generic ( copy, "copy" );
	def_generic ( get_props,	"getPropNames"); // LAM: added 4/27/00
	def_generic ( show_props,	"showProperties");

	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);

	// added 3/21/05. Used by debugger to dump locals to standard out
	void		dump_local_vars(int indentLevel);

	// added 4/25/08. Used for create handler
	Value* call_handler(Value* handler, Value** arg_list, int count);
};


