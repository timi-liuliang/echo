/*		Functions.h - the Function family class - primitives, generics
 *
 *		Copyright (c) John Wainwright, 1996
 *		
 */

#pragma once

#include "arrays.h"
// forward declarations
class HashTable;
class FPInterfaceProxy;

#undef def_generic
#define def_generic(fn, name)	\
			ScripterExport Value* fn##_vf(Value** arglist, int arg_count)

#define FPS_CACHE_SIZE 512

// internally used structure
struct DumpValueData
{
	Value**		locals;
	int			indentLevel;
	Value*		owner;
};


/* --- function base class -- */

visible_class (Function)

class Function : public Value
{
// Whether the function can be called from debugger thread stored in Collectable::flags3 - bit 0. Default is false.
public:
	MCHAR*		name;
	MCHAR*		struct_name;	// packaged in a struct if non-null

				   Function() { name = NULL; struct_name = NULL; }
	ScripterExport Function(MCHAR* name, MCHAR* struct_name=NULL);
	ScripterExport Function(MCHAR* name, MCHAR* struct_name, bool debuggerSafe);
	ScripterExport ~Function();

				classof_methods (Function, Value);
#	define		is_function(v) ((v)->_is_function())
	BOOL		_is_function() { DbgAssert(!is_sourcepositionwrapper(this)); return 1; }

	ScripterExport void sprin1(CharStream* s);
	ScripterExport void export_to_scripter();
};

/* ----------------  call context base class ----------------- */

class CallContext 
{
	CallContext* previous;
public:
	CallContext() : previous(NULL) { }
	CallContext(CallContext* previous) : previous(previous) { }

	// called by fn applier to establish context AFTER arguments eval'd	
	virtual void push_context() { if (previous) previous->push_context(); }
	virtual void pop_context() { if (previous) previous->pop_context(); }
};

/* ----------------------- Generics ------------------------- */

visible_class (Generic)

class Generic : public Function
{
public:
	value_vf	fn_ptr;

				Generic() { }
 ScripterExport Generic(MCHAR* name, value_vf fn, MCHAR* struct_name = NULL);
 ScripterExport Generic(MCHAR* name, value_vf fn, MCHAR* struct_name, short init_flags=0);
			    Generic(MCHAR* name) : Function(name) { }

				classof_methods (Generic, Function);
	BOOL		_is_function() { DbgAssert(!is_sourcepositionwrapper(this)); return 1; }
	ScripterExport void init(MCHAR* name, value_vf fn);
	void		collect() { delete this; }

	ScripterExport Value* apply(Value** arglist, int count, CallContext* cc=NULL);
};

visible_class (MappedGeneric)

class MappedGeneric : public Generic
{
public:
				MappedGeneric() { }
 ScripterExport MappedGeneric(MCHAR* name, value_vf fn);
 ScripterExport MappedGeneric(MCHAR* name, value_vf fn, short init_flags);
			    MappedGeneric(MCHAR* name) : Generic(name) { }

				classof_methods (MappedGeneric, Generic);
	BOOL		_is_function() { DbgAssert(!is_sourcepositionwrapper(this)); return 1; }
	void		collect() { delete this; }

	ScripterExport Value* apply(Value** arglist, int count, CallContext* cc=NULL);
};

visible_class (NodeGeneric)

class NodeGeneric : public MappedGeneric
{
public:
 ScripterExport NodeGeneric(MCHAR* name, value_vf fn);
 ScripterExport NodeGeneric(MCHAR* name, value_vf fn, short init_flags);
			    NodeGeneric(MCHAR* name) : MappedGeneric(name) { }

				classof_methods (NodeGeneric, MappedGeneric);
	BOOL		_is_function() { DbgAssert(!is_sourcepositionwrapper(this)); return 1; }
	void		collect() { delete this; }

	ScripterExport Value* apply(Value** arglist, int count, CallContext* cc=NULL);
};

/* -------------------------- Primitives ------------------------------ */

enum primitive_flag
{
	LAZY_PRIMITIVE	=	0x0001, // don't pre-evaluate args for lazy primitives
	DEBUGGER_SAFE	=	0x0002, // safe to call from debugger
};

visible_class (Primitive)

class Primitive : public Function
{ 
public:
	short		flags;
	value_cf	fn_ptr;

			    Primitive() { flags=0; }
 ScripterExport Primitive(MCHAR* name, value_cf fn, short init_flags=0);
 ScripterExport Primitive(MCHAR* name, MCHAR* structure, value_cf fn, short init_flags=0);
			    Primitive(MCHAR* name) : Function(name) { flags=0; }

				classof_methods (Primitive, Function);
	BOOL		_is_function() { DbgAssert(!is_sourcepositionwrapper(this)); return 1; }
	void		collect() { delete this; }

	ScripterExport Value* apply(Value** arglist, int count, CallContext* cc=NULL);
};

visible_class (MappedPrimitive)

class MappedPrimitive : public Primitive
{ 
public:
 ScripterExport MappedPrimitive(MCHAR* name, value_cf fn);
 ScripterExport MappedPrimitive(MCHAR* name, value_cf fn, short init_flags);

				classof_methods (MappedPrimitive, Primitive);
	BOOL		_is_function() { DbgAssert(!is_sourcepositionwrapper(this)); return 1; }
	void		collect() { delete this; }

	ScripterExport Value* apply(Value** arglist, int count, CallContext* cc=NULL);
};

/* ----- */

visible_class (MAXScriptFunction)

class MAXScriptFunction : public Function
{
public:
	short		parameter_count;
	short		local_count;
	short		keyparm_count;
	short		flags;
	Value**		keyparms;
	Value*		body;
	HashTable*	local_scope;
	value_cf	c_callable_fn;
	Array*		paramNames;
	Value*		owner;

 ScripterExport MAXScriptFunction(MCHAR* name, int parm_count, int keyparm_count, Value** keyparms,
								  int local_count, Value* body, HashTable* local_scope, short flags = 0);
 ScripterExport MAXScriptFunction(MCHAR* name, int parm_count, int keyparm_count, Value** keyparms,
								  int local_count, Value* body, HashTable* local_scope, Array* param_names, 
								  Value* owner, short flags = 0);
				~MAXScriptFunction();
#	define		is_maxscriptfunction(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(MAXScriptFunction))

				classof_methods (MAXScriptFunction, Function);
	BOOL		_is_function() { DbgAssert(!is_sourcepositionwrapper(this)); return TRUE; }
	void		collect() { delete this; }
	void		gc_trace();
	void		sprin1(CharStream* s);

	Value*		apply(Value** arglist, int count, CallContext* cc=NULL);
	Value*		apply_no_alloc_frame(Value** arglist, int count, CallContext* cc=NULL);

	value_cf	get_c_callable_fn();

	Value*		operator()(Value** arg_list, int count);

	// added 3/21/05. Used by debugger to dump locals, parms, and externals to standard out
	void		dump_local_vars_params_and_externals(Value** paramsFrame, Value** localsFrame, int indentLevel);

protected:
	Value*		wrap_arg(Value* arg);
};

#define FN_MAPPED_FN	0x0001		// declared a collection-mapped function
#define FN_BODY_FN		0x0002		// a loop or other body function, don't trap exits here
#define FN_HAS_REFARGS	0x0004		// function has reference arguments
#define FN_MAPPED_EVAL	0x0008		// set while evaluating a mapped function on each item


// StructMethods wrap member functions accessed on a struct instance
// their apply() sets up the appropriate struct instance thread-local
// for member data access thunks
class Struct;
class StructMethod : public Value
{
public:
	Struct* this_struct;
	Value*	fn;

	ScripterExport StructMethod(Struct* t, Value* f);

	void	gc_trace();
	void	collect() { delete this; }
	void	sprin1(CharStream* s) { fn->sprin1(s); }
	BOOL	_is_function() { return fn->_is_function(); }

#	define is_structMethod(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_STRUCT_METHOD_TAG)  // LAM - defect 307069
	Value* classOf_vf(Value** arg_list, int count) { return fn->classOf_vf(arg_list, count); }
	Value* superClassOf_vf(Value** arg_list, int count) { return fn->superClassOf_vf(arg_list, count); }
	Value* isKindOf_vf(Value** arg_list, int count) { return fn->isKindOf_vf(arg_list, count); }
	BOOL   is_kind_of(ValueMetaClass* c) { return fn->is_kind_of(c); }
	Value* get_property(Value** arg_list, int count) { return fn->get_property(arg_list, count); }
	Value* eval() { return fn->eval(); }
	Value* apply(Value** arglist, int count, CallContext* cc=NULL);
	Value* apply_no_alloc_frame(Value** arglist, int count, CallContext* cc=NULL); // LAM - 11/16/02
};

// call context for StructMethod
// used internally when calling a structure method to save current active structure, set the specified structure
// as the current structure, and afer the call to restore the original current structure
class SMCallContext : public CallContext
{
	Struct* save_struct;
	Struct* this_struct;
public:
	SMCallContext(Struct* strct, CallContext* previous=NULL);
	void push_context(); 
	void pop_context();
};

// LAM - 9/6/02 - defect 291499
// PluginMethods wrap member functions accessed on a plugin instance
// their apply() sets up the appropriate plugin instance thread-local
// for member data access thunks
class MSPlugin;
class PluginMethod : public Value
{
public:
	MSPlugin* this_plugin;
	Value*	fn;

	ScripterExport PluginMethod(MSPlugin* t, Value* f);

	void	gc_trace();
	void	collect() { delete this; }
	void	sprin1(CharStream* s) { fn->sprin1(s); }
	BOOL	_is_function() { return fn->_is_function(); }

#	define is_pluginMethod(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == INTERNAL_MSPLUGIN_METHOD_TAG)
	Value* classOf_vf(Value** arg_list, int count) { return fn->classOf_vf(arg_list, count); }
	Value* superClassOf_vf(Value** arg_list, int count) { return fn->superClassOf_vf(arg_list, count); }
	Value* isKindOf_vf(Value** arg_list, int count) { return fn->isKindOf_vf(arg_list, count); }
	BOOL   is_kind_of(ValueMetaClass* c) { return (is_pluginMethod(c)) ? 1 : Value::is_kind_of(c); }
	Value* get_property(Value** arg_list, int count) { return fn->get_property(arg_list, count); }
	Value* eval() { return fn->eval(); }
	Value* apply(Value** arg_list, int count, CallContext* cc=NULL);
	Value* apply_no_alloc_frame(Value** arg_list, int count, CallContext* cc=NULL);

};

// UserProp & UserGeneric instances represent dynamically-added, user-defined generics
//  on built-in classes.  They are kept in sorted tables in ValueMetaClass instances,
// suitable for bsearching.
class UserProp
{
public:
	Value*		prop;
	value_cf	getter;
	value_cf	setter;
				UserProp (Value* p, value_cf g, value_cf s) { prop = p; getter = g; setter = s; }
};

class UserGeneric
{
public:
	Value*		name;
	value_cf	fn;
				
				UserGeneric(Value* n, value_cf f) { name = n; fn = f; }
};

// UserGenericValue is the scripter-visible generic fn value that dispatches the
// UserGeneric 'methods' in a target object's class

visible_class (UserGenericValue)

class UserGenericValue : public Function
{
public:
	Value*		fn_name;
	Value*		old_fn;   // if non-NULL, the original global fn that this usergeneric replaced
				
 ScripterExport UserGenericValue(Value* name, Value* old_fn);

				classof_methods (UserGenericValue, Function);
	BOOL		_is_function() { DbgAssert(!is_sourcepositionwrapper(this)); return TRUE; }
	void		collect() { delete this; }
	void		gc_trace();

	Value*		apply(Value** arglist, int count, CallContext* cc=NULL);
};

#define	def_user_prop(_prop, _cls, _getter, _setter)		\
	_cls##_class.add_user_prop(#_prop, _getter, _setter)

#define	def_user_generic(_fn, _cls, _name)					\
	_cls##_class.add_user_generic(#_name, _fn)


// ------- MAXScript Function Publishing interface ----------------------

#include "..\..\ifnpub.h"
#include "..\..\GetCOREInterface.h"

class InterfaceMethod;
class FPMixinInterfaceValue;
class FPEnum;

// FnPub function, a function published by a plugin using theFnPub system
//     automatically exposed by MAXScript boot code during intial plugin scan

visible_class (InterfaceFunction)

class InterfaceFunction : public Function
{
public:
	FPInterfaceDesc* fpid;
	FPFunctionDef*   fd;

				InterfaceFunction(FPInterface* fpi, FPFunctionDef* fd);
				~InterfaceFunction();

				classof_methods (InterfaceFunction, Function);
	BOOL		_is_function() { DbgAssert(!is_sourcepositionwrapper(this)); return TRUE; }
	void		collect() { delete this; }
	void		gc_trace();
	void		sprin1(CharStream* s);

	Value*		apply(Value** arglist, int count, CallContext* cc=NULL);
	Value*		get_property(Value** arg_list, int count);

	// parameter conversion utilities
	ScripterExport static void	  val_to_FPValue(Value* v, ParamType2 type, FPValue& fpv, FPEnum* e, Tab<void*> *stringStorage, Array* valueStore);
	ScripterExport static void	  val_to_FPValue(Value* v, ParamType2 type, FPValue& fpv, FPEnum* e, Tab<void*> *stringStorage);
	ScripterExport static void	  val_to_FPValue(Value* v, ParamType2 type, FPValue& fpv, FPEnum* e=NULL);
	ScripterExport static Value*  FPValue_to_val(FPValue& fpv, FPEnum* e=NULL);
	ScripterExport static void	  release_param(FPValue& fpv, ParamType2 type, Value* v, FPEnum* e=NULL);
	ScripterExport static void	  init_param(FPValue& fpv, ParamType2 type);
	ScripterExport static void	  validate_params(FPInterface* fpi, FunctionID fid, FPParamDef* pd, ParamType2 type, int paramNum, FPValue& val, Value* v);
	ScripterExport static FPEnum* FindEnum(short id, FPInterfaceDesc* fpid);
};

// ValueConverter - Routines to convert between FPValue and Value
#define VALUECONVERTER_INTERFACE_ID Interface_ID(0x19f37b9f, 0x39ed5680)
class ValueConverter : public FPStaticInterface
{
public:
	enum FPValueSubtype
	{
		SUBTYPE_NONE=0,
		SUBTYPE_IROLLUPWINDOW,
	};

	virtual void	val_to_FPValue(Value* v, ParamType2 type, FPValue& fpv, FPEnum* e, Tab<void*> *stringStorage, Array* valueStore) = 0;
	virtual void	val_to_FPValue(Value* v, ParamType2 type, FPValue& fpv, FPEnum* e, Tab<void*> *stringStorage) = 0;
	virtual void	val_to_FPValue(Value* v, ParamType2 type, FPValue& fpv, FPEnum* e=NULL) = 0;
	virtual Value*	FPValue_to_val(FPValue& fpv, FPEnum* e=NULL, FPValueSubtype subtype=SUBTYPE_NONE) = 0;
	virtual void	release_param(FPValue& fpv, ParamType2 type, Value* v, FPEnum* e=NULL) = 0;
	virtual void	init_param(FPValue& fpv, ParamType2 type) = 0;
	virtual void	validate_params(FPInterface* fpi, FunctionID fid, FPParamDef* pd, ParamType2 type, int paramNum, FPValue& val, Value* v) = 0;
	virtual FPEnum*	FindEnum(short id, FPInterfaceDesc* fpid) = 0;

    // This is how client code can get to the ValueConverter instance
	static ValueConverter* GetValueConverter()
	{
		return static_cast<ValueConverter*>(GetCOREInterface(VALUECONVERTER_INTERFACE_ID));
	}
};

// InterfaceMethod - wraps an InterfaceFunction and its target object for shorthand mixin calls
class InterfaceMethod : public InterfaceFunction
{
private:
				InterfaceMethod(FPMixinInterfaceValue* fpiv, FPFunctionDef* fd);
	static		InterfaceMethod* interface_method_cache[FPS_CACHE_SIZE];
	friend void Collectable::gc();
	friend void Collectable::mark();
public:
	FPMixinInterfaceValue* fpiv;

	static ScripterExport InterfaceMethod* intern(FPMixinInterfaceValue* fpiv, FPFunctionDef* fd);
				~InterfaceMethod();

				def_generic ( isDeleted,			"isDeleted");	// LAM: 11/23/01 - added - doesn't break SDK
				use_generic( eq,					"=" );			// LAM: 11/23/01 - added - doesn't break SDK
				use_generic( ne,					"!=" );			// LAM: 11/23/01 - added - doesn't break SDK
	void		collect() { delete this; }
	void		gc_trace();

	Value*		apply(Value** arglist, int count, CallContext* cc=NULL);
};

// Action predicate function wrappers...
visible_class (ActionPredicate)

class ActionPredicate : public InterfaceFunction
{
public:
	short		pred;

	ActionPredicate(FPInterface* fpi, FPFunctionDef* fd, short pred);

				classof_methods (ActionPredicate, Function);
	BOOL		_is_function() { DbgAssert(!is_sourcepositionwrapper(this)); return TRUE; }
	void		collect() { delete this; }

	Value*		apply(Value** arglist, int count, CallContext* cc=NULL);
};

// IObject, generic wrapper for objects that inherit from IObject
//    this is a way to give simple interface-based wrappers to 
//    classes in MAX or 3rd-party plugins that MAXScript knows
//    nothing about.  The IObject instance must implement GetInterface()

visible_class (IObjectValue)

class IObjectValue : public Value
{
public:
	IObject* iobj;					// the IObject pointer

			IObjectValue(IObject* io);
		   ~IObjectValue();
#	define	is_iobject(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(IObjectValue))

			classof_methods (IObjectValue, Value);
	void	collect() { delete this; }
	void	sprin1(CharStream* s);

	MSTR	to_filename(); // returns guidstring if wraps AssetUser
	void	to_fpvalue(FPValue& v) { v.iobj = iobj; v.type = TYPE_IOBJECT; }

	def_generic (show_interfaces,  "showInterfaces"); 
	def_generic (get_interfaces, "getInterfaces");
	def_generic (get_interface, "getInterface");

// End of 3ds max 4.2 Extension

	BaseInterface* GetInterface(Interface_ID id) { return iobj->GetInterface(id); }

	//accesses interfaces on the IObject
	Value*	get_property(Value** arg_list, int count);
	Value*	set_property(Value** arg_list, int count);
};

// FPInterfaceValue, represents  FPInterfaces in MAXScript

visible_class (FPInterfaceValue)

class FPInterfaceValue : public Value, public InterfaceNotifyCallback
{
public:
	FPInterface* fpi;			// interface
	HashTable*	 fns;			// interface fn lookup
	HashTable*	 props;			// interface prop lookup
	FPInterface::LifetimeType lifetime;	// interface lifetime control type
	static bool	 enable_test_interfaces;  // test interface enable flag
	// Whether to call fpi->ReleaseInterface stored in Collectable::flags3 - bit 0

	ScripterExport FPInterfaceValue(FPInterface* fpi);   
	FPInterfaceValue(FPInterfaceProxy* proxy);
	~FPInterfaceValue();

	// Needed to solve ambiguity between Collectable's operators and MaxHeapOperators
	using Collectable::operator new;
	using Collectable::operator delete;

#	define	is_fpstaticinterface(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(FPInterfaceValue))

			classof_methods (FPInterfaceValue, Value);
			def_generic ( show_interface,		"showInterface"); // LAM: 08/29/00
			def_generic ( get_props,			"getPropNames"); // LAM: added 2/1/02
			def_generic ( isDeleted,			"isDeleted");	// LAM: 11/23/01 - added - doesn't break SDK
			use_generic( eq,					"=" );			// LAM: 11/23/01 - added - doesn't break SDK
			use_generic( ne,					"!=" );			// LAM: 11/23/01 - added - doesn't break SDK
	void	collect() { delete this; }
	void	gc_trace();
	void	sprin1(CharStream* s);

// The following method has been added
// in 3ds max 4.2.  If your plugin utilizes this new
// mechanism, be sure that your clients are aware that they
// must run your plugin with 3ds max version 4.2 or higher.

	void	to_fpvalue(FPValue& v) { 
		resolve_proxy();
		v.fpi = fpi; 
		v.type = TYPE_INTERFACE; 
	}

// End of 3ds max 4.2 Extension

	// from InterfaceNotifyCallback
	void	InterfaceDeleted(BaseInterface* bi) { UNUSED_PARAM(bi); fpi = NULL; }  

	// accesses methods & props in the interface
	Value*	_get_property(Value* prop);
	Value*	_set_property(Value* prop, Value* val);

	Value*	get_property(Value** arg_list, int count);
	Value*	set_property(Value** arg_list, int count);

	static void UpdateEditorOnResolve(bool update = true);

private:
	void	resolve_proxy();
	void	set_interface(FPInterface* fpi);

	FPInterfaceProxy*	m_fpip;
	static bool			s_updateEditorOnResolve;
};

extern ScripterExport void print_FP_interface(CharStream* out, FPInterface* fpi, bool getPropNames = true, 
			   bool getMethodNames = true, bool getInterfaceNames = true, bool getActionTables = true);

// FPMixinInterfaceValue provides wrappers for mixin interfaces on individual target objects
//  stored in a cache for fast retrieval and to minimize over-consing
// Warning: FPMixinInterfaceValue can wrap a FPStaticInterface. If accessing FPMixinInterface
// specific items, test 'fpi->GetDesc()->flags & FP_MIXIN' first.

visible_class (FPMixinInterfaceValue)

class FPMixinInterfaceValue : public Value, public InterfaceNotifyCallback
{
private:
			FPMixinInterfaceValue(FPInterface* fpi);
		   ~FPMixinInterfaceValue();
	static  FPMixinInterfaceValue* FPMixinInterfaceValue::interface_cache[128];
	friend void Collectable::gc();
	friend void Collectable::mark();
public:
	FPInterface* fpi;						// interface
	FPInterface::LifetimeType lifetime;		// interface lifetime control type
	// Whether to call fpi->ReleaseInterface stored in Collectable::flags3 - bit 0

	// Needed to solve ambiguity between Collectable's operators and MaxHeapOperators
	using Collectable::operator new;
	using Collectable::operator delete;

	static  ScripterExport FPMixinInterfaceValue* intern(Value* prop_name, Value* target);
	static  ScripterExport FPMixinInterfaceValue* intern(FPInterface* fpi);
#	define	is_fpmixininterface(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(FPMixinInterfaceValue))

			classof_methods (FPMixinInterfaceValue, Value);
			def_generic ( show_interface,		"showInterface"); // LAM: 08/29/00
			def_generic ( isDeleted,			"isDeleted");	// LAM: 11/23/01 - added - doesn't break SDK
			use_generic( eq,					"=" );			// LAM: 11/23/01 - added - doesn't break SDK
			use_generic( ne,					"!=" );			// LAM: 11/23/01 - added - doesn't break SDK
			def_generic ( get_props,			"getPropNames"); // LAM: added 2/1/02
	void	collect() { delete this; }
	void	sprin1(CharStream* s);

// The following method has been added
// in 3ds max 4.2.  If your plugin utilizes this new
// mechanism, be sure that your clients are aware that they
// must run your plugin with 3ds max version 4.2 or higher.

	void	to_fpvalue(FPValue& v) { v.fpi = fpi; v.type = TYPE_INTERFACE; }

// End of 3ds max 4.2 Extension

	// from InterfaceNotifyCallback
	void	InterfaceDeleted(BaseInterface* bi); 

	// accesses methods & props in the interface
	Value*	_get_property(Value* prop);
	Value*	_set_property(Value* prop, Value* val);

	Value*	get_property(Value** arg_list, int count);
	Value*	set_property(Value** arg_list, int count);
};

// FPStaticMethodInterfaceValue provides wrappers for static interfaces that
// have been registered with individual Value metaclasses as property
// interfaces on instances of the metaclasses' class, such that calls
// on methods in these interfaces pass the intance along as the first
// argument wrapped in an FPValue.

// these are used to allow factored static interfaces (such as meshOps)
// to appear as though they are mixin interfaces on several MAXScript value
// classes (such as node, baseobject, meshvalue), in which the target object
// is sent as a polymorphic first argument (via FPValue) to static interface
// method call, rather than as a 'this' pointer to a virtual mixin interface method

visible_class (FPStaticMethodInterfaceValue)

class FPStaticMethodInterfaceValue : public Value, public InterfaceNotifyCallback
{
private:
			FPStaticMethodInterfaceValue(FPInterface* fpi, ParamType2 type, void* object);
		   ~FPStaticMethodInterfaceValue();
	static  FPStaticMethodInterfaceValue* interface_cache[FPS_CACHE_SIZE];
	friend void Collectable::gc();
	friend void Collectable::mark();
public:
	FPInterface* fpi;			// interface
	FPValue		 value;         // the target object as FPValue first argument
	FPInterface::LifetimeType lifetime;	// interface lifetime control type
	// Whether to call fpi->ReleaseInterface stored in Collectable::flags3 - bit 0

	// Needed to solve ambiguity between Collectable's operators and MaxHeapOperators
	using Collectable::operator new;
	using Collectable::operator delete;

	static  ScripterExport FPStaticMethodInterfaceValue* intern(FPInterface* fpi, ParamType2 type, void* object);
#	define	is_fpstaticmethodinterface(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(FPStaticMethodInterfaceValue))

			classof_methods (FPStaticMethodInterfaceValue, Value);
			def_generic ( show_interface,		"showInterface"); // LAM: 08/29/00
			def_generic ( isDeleted,			"isDeleted");	// LAM: 11/23/01 - added - doesn't break SDK
			use_generic( eq,					"=" );			// LAM: 11/23/01 - added - doesn't break SDK
			use_generic( ne,					"!=" );			// LAM: 11/23/01 - added - doesn't break SDK
			def_generic ( get_props,			"getPropNames"); // LAM: added 2/1/02
	void	collect() { delete this; }
	void	sprin1(CharStream* s);

	// from InterfaceNotifyCallback
	void	InterfaceDeleted(BaseInterface* bi) { UNUSED_PARAM(bi); fpi = NULL; }  

	// accesses methods & props in the interface
	Value*	_get_property(Value* prop);
	Value*	_set_property(Value* prop, Value* val);

	Value*	get_property(Value** arg_list, int count);
	Value*	set_property(Value** arg_list, int count);
};

// StaticInterfaceMethod - wraps an FPStaticMethodInterfaceValue and its target object for property-based calls
class StaticInterfaceMethod : public InterfaceFunction
{
private:
				StaticInterfaceMethod(FPStaticMethodInterfaceValue* fpiv, FPFunctionDef* fd);
	static		StaticInterfaceMethod* interface_method_cache[FPS_CACHE_SIZE];
	friend void Collectable::gc();
	friend void Collectable::mark();
public:
	FPStaticMethodInterfaceValue* fpiv;

	static ScripterExport StaticInterfaceMethod* intern(FPStaticMethodInterfaceValue* fpiv, FPFunctionDef* fd);
				~StaticInterfaceMethod();

				def_generic ( isDeleted,			"isDeleted");	// LAM: 11/23/01 - added - doesn't break SDK
				use_generic( eq,					"=" );			// LAM: 11/23/01 - added - doesn't break SDK
				use_generic( ne,					"!=" );			// LAM: 11/23/01 - added - doesn't break SDK
	void		collect() { delete this; }
	void		gc_trace();

	Value*		apply(Value** arglist, int count, CallContext* cc=NULL);
};

