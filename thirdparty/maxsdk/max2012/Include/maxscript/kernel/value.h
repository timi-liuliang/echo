/*		Value.h - metaclass system  MAXScript values
 *
 *	All MAXScript-specific C++ objects are subclasses of a single root class, Value, 
 *  and allocated & automatically freed in a specially maintained heap.  There is also
 *  a metaclass system to provide a runtime type calculus for the scripter.  Value subclasses
 *  are divided into those that are scripter-visible, (ie, may wind up as objects that the
 *  scripter may pass around or store in variables, etc.), and those that are entirely
 *  internal to the scripter operation (such as thunks, etc.).  The scripter-visible
 *  classes (the majority) are represented in the metasystem by instances of separate
 *  metaclasses.  The metaclasses are all subclasses of ValueMetaClass, the metaclass of
 *  a class X is named XClass and its sole instance is X_class. The class instances are
 *  made visible in globals (usually) named X.  
 *
 *  Each Value instance has a tag word that either contains a pointer to the instance's
 *  class instance (in the case of scripter-visible classes) or the reserved value INTERNAL_CLASS_TAG.
 *  This value is used in performing runtimne type tests and for yielding results to classOf 
 *  methods.
 *
 *  The metaclass, its instance and some of the class calculus methods are usually defined via
 *  a bunch of macros defined here (see visible_class, visible_class_instance, etc.)
 *
 *  Some of the classes are can be instanced directly as literals in a script, such as strings, 
 *  Point3s, arrays, etc.  Some others are instantiable directly by applying the class value
 *  to a set of initializing parameters, ie, using the class as a function in a function call,
 *	for example, ray, quat, interval, etc.  These are defined via a variant macro: applyable_class().
 *  A special case of this is provided in the MAXWrapper subsytem for creatable MAX objects, such as
 *  boxes, lights, camera, etc..  These are represnted by instances of the class MAXClass, and again, thses
 *  instances are exposed in globals to be applied to creation paramters.  These instances
 *  contain a lot of property metadata and are defined in MAX_classes.cpp.  See MAXObject.h for more
 *  info.
 *	
 *		Copyright (c) John Wainwright, 1996
 *
 */

#pragma once

#include "..\ScripterExport.h"
#include "collectable.h"
#include "interupts.h"
#include "MAXScript_TLS.h"
#include "..\macros\value_locals.h"
#include "MaxscriptTypedefs.h"
#include "..\..\matrix3.h"
#include "..\..\box2.h"
#include "..\..\acolor.h"
#include "..\..\interval.h"
#include "..\..\quat.h"
#include "..\..\ifnpub.h"

// forward declarations
class BitArray;
class CharStream;
class Name;
class PathName;
class Undefined;
class UserProp;
class UserGeneric;
class CallContext;
class ValueMetaClass;
struct node_map;
class Mtl;
class Texmap;
class MtlBase;
class Modifier;
class Control;
class Atmospheric;
class Effect;
class IMultiPassCameraEffect;
class ShadowType;
class FPInterfaceDesc;
class FilterKernel;
class ITrackViewNode;
class NURBSIndependentPoint;
class NURBSPoint;
class NURBSObject;
class NURBSControlVertex;
class NURBSCurve;
class NURBSCVCurve;
class NURBSSurface;
class NURBSTexturePoint;
class NURBSSet;
class ReferenceTarget;
class Mesh;
class Thunk;
class Renderer;
class NURBSTextureSurface;
class NURBSDisplay;
class TessApprox;
class SelectionIterator;

#include "..\macros\define_external_functions.h"
#	include "..\protocols\corenames.inl"

// forward declarations...


extern ScripterExport Undefined undefined;
extern ScripterExport bool dontThrowAccessorError;


// the root MAXScript class
class Value : public Collectable
{
private:
	ScripterExport static Matrix3  s_error_matrix;
	ScripterExport static Box2  s_error_box2;
public:
	#pragma warning(push)
	#pragma warning(disable:4100)
	ValueMetaClass* tag;		// runtime type tag; filled in by subclasses

	ScripterExport virtual BOOL	is_kind_of(ValueMetaClass* c);
	ScripterExport virtual ValueMetaClass* local_base_class(); // local base class in this class's plug-in
	virtual Value*	eval() { check_interrupts(); return this; }
	virtual Value*	eval_no_wrapper() { check_interrupts(); return this; }
	ScripterExport virtual Value*  apply(Value** arglist, int count, CallContext* cc=NULL);
	ScripterExport virtual Value*  apply_no_alloc_frame(Value** arglist, int count, CallContext* cc=NULL);
	virtual void	export_to_scripter() { }

	virtual Value*  map(node_map& m) { unimplemented(_M("map"), this) ; return this; }
	virtual Value*	map_path(PathName* path, node_map& m) { unimplemented(_M("map_path"), this) ; return this; }
	virtual Value*	find_first(BOOL (*test_fn)(INode* node, int level, void* arg), void* test_arg) { unimplemented(_M("find_first"), this) ; return this; }
	virtual Value*	get_path(PathName* path) { unimplemented(_M("get"), this) ; return this; }

	ScripterExport virtual void	sprin1(CharStream* stream);
	ScripterExport virtual void	sprint(CharStream* stream);

	virtual void	prin1() { sprin1(thread_local(current_stdout)); }
	virtual void	print() { sprint(thread_local(current_stdout)); }
	
	/* include all the protocol declarations */

#include "..\macros\define_abstract_functions.h"
#	include "..\protocols\math.inl"
#	include "..\protocols\vector.inl"
#	include "..\protocols\matrix.inl"
#	include "..\protocols\quat.inl"
#	include "..\protocols\arrays.inl"
#	include "..\protocols\streams.inl"
#	include "..\protocols\strings.inl"
#	include "..\protocols\time.inl"
#	include "..\protocols\color.inl"
#	include "..\protocols\node.inl"
#	include "..\protocols\controller.inl"
#	include "..\protocols\primitives.inl"
#	include "..\protocols\generics.inl"
#	include "..\protocols\bitmaps.inl"
#	include "..\protocols\textures.inl"
#	include "..\protocols\atmospherics.inl"
#	// Moved to ..\maxwrapper\mxsnurbs.h into class NURBSObjectValue
#	include "..\protocols\cameratracker.inl"
#	include "..\protocols\bigmatrix.inl"
#	include	"..\protocols\box.inl"
#	include "..\protocols\physiqueblend.inl"
#	include "..\protocols\physiquemod.inl"
#	include	"..\protocols\biped.inl"
#	include "..\protocols\notekey.inl"
#	include "..\protocols\xrefs.inl"

	ScripterExport virtual Class_ID get_max_class_id() { return Class_ID(0, 0); }
	ScripterExport virtual Value* delete_vf(Value** arglist, int arg_count) { ABSTRACT_FUNCTION(_M("delete"), this, Value*); }	  
	ScripterExport virtual Value* clearSelection_vf(Value** arglist, int arg_count) { ABSTRACT_FUNCTION(_M("clearSelection"), this, Value*); }	  

#undef def_generic
#define def_generic(fn, name) ScripterExport virtual Value* fn##_vf(Value** arglist, int arg_count);
#	include "..\protocols\kernel.inl"
	
	virtual float		to_float() { ABSTRACT_CONVERTER(float, Float); }
	virtual double		to_double() { ABSTRACT_CONVERTER(double, Double); }
	virtual MCHAR*		to_string() { ABSTRACT_CONVERTER(MCHAR*, String); }
	virtual MSTR		to_filename() { ABSTRACT_CONVERTER(MCHAR*, FileName); }
	virtual int			to_int() { ABSTRACT_CONVERTER(int, Integer); }
	virtual INT64		to_int64() { ABSTRACT_CONVERTER(INT64, Integer64); }	
	virtual INT_PTR		to_intptr() { ABSTRACT_CONVERTER(INT_PTR, IntegerPtr); }	
	virtual BOOL		to_bool() { ABSTRACT_CONVERTER(BOOL, Boolean); }
	virtual BitArray&	to_bitarray() { throw ConversionError (this, _M("BitArray")); return *(BitArray*)NULL; }
	virtual Point4		to_point4() { ABSTRACT_CONVERTER(Point4, Point4); }
	virtual Point3		to_point3() { ABSTRACT_CONVERTER(Point3, Point3); }
	virtual Point2		to_point2() { ABSTRACT_CONVERTER(Point2, Point2); }
	virtual AColor		to_acolor() { throw ConversionError (this, _M("Color")); return AColor(0,0,0); }
	virtual COLORREF	to_colorref() { throw ConversionError (this, _M("Color")); return RGB(0,0,0); }
	virtual INode*		to_node() { ABSTRACT_CONVERTER(INode*, <node>); }
	virtual Ray			to_ray() { throw ConversionError (this, _M("Ray")); return Ray(); }
	virtual Interval	to_interval() { throw ConversionError (this, _M("Interval")); return Interval();  }
	virtual Quat		to_quat() { throw ConversionError (this, _M("Quaternion")); return Quat();  }
	virtual AngAxis		to_angaxis() { throw ConversionError (this, _M("AngleAxis")); return AngAxis();  }
	virtual Matrix3&	to_matrix3() { throw ConversionError (this, _M("Matrix")); return s_error_matrix;  }
	virtual float*		to_eulerangles() { ABSTRACT_CONVERTER(float*, Float); }
	virtual Mtl*		to_mtl() { ABSTRACT_CONVERTER(Mtl*, Material); }
	virtual Texmap*		to_texmap() { ABSTRACT_CONVERTER(Texmap*, TextureMap); }
	virtual MtlBase*	to_mtlbase() { ABSTRACT_CONVERTER(MtlBase*, MtlBase); }
	virtual Modifier*	to_modifier() { ABSTRACT_CONVERTER(Modifier*, Modifier); }
	virtual TimeValue	to_timevalue() { ABSTRACT_CONVERTER(TimeValue, Time); }
	virtual Control*	to_controller() { ABSTRACT_CONVERTER(Control*, Controller); }
	virtual Atmospheric* to_atmospheric() { ABSTRACT_CONVERTER(Atmospheric*, Atmospheric); }
	virtual Effect*		to_effect() { ABSTRACT_CONVERTER(Effect*, Effect); }						// RK: Added this
	virtual IMultiPassCameraEffect*	to_mpassCamEffect() { ABSTRACT_CONVERTER(IMultiPassCameraEffect*, Effect); }	// LAM: Added this
	virtual ShadowType*	to_shadowtype() { ABSTRACT_CONVERTER(ShadowType*, ShadowType); }			// RK: Added this
	virtual FilterKernel*	to_filter() { ABSTRACT_CONVERTER(FilterKernel*, FilterKernel); }		// RK: Added this
	virtual INode*		to_rootnode() { ABSTRACT_CONVERTER(INode*, <root>); }						// RK: Added this
	virtual ITrackViewNode* to_trackviewnode() { ABSTRACT_CONVERTER(ITrackViewNode*, TrackViewNode); }
	virtual NURBSIndependentPoint* to_nurbsindependentpoint() { throw ConversionError (this, _M("NURBSIndependentPoint")); return (NURBSIndependentPoint*)0;  }
	virtual NURBSPoint*	to_nurbspoint() { throw ConversionError (this, _M("NURBSPoint")); return (NURBSPoint*)0;  }
	virtual NURBSObject* to_nurbsobject() { throw ConversionError (this, _M("NURBSObject")); return (NURBSObject*)0;  }
	virtual NURBSControlVertex* to_nurbscontrolvertex() { throw ConversionError (this, _M("NURBSControlVertex")); return (NURBSControlVertex*)0;  }
	virtual NURBSCurve* to_nurbscurve() { throw ConversionError (this, _M("NURBSCurve")); return (NURBSCurve*)0;  }
	virtual NURBSCVCurve* to_nurbscvcurve() { throw ConversionError (this, _M("NURBSCVCurve")); return (NURBSCVCurve*)0;  }
	virtual NURBSSurface* to_nurbssurface() { throw ConversionError (this, _M("NURBSSurface")); return (NURBSSurface*)0;  }
	virtual NURBSTexturePoint* to_nurbstexturepoint() { throw ConversionError (this, _M("NURBSTexturePoint")); return (NURBSTexturePoint*)0;  }
	virtual NURBSSet*	to_nurbsset() { throw ConversionError (this, _M("NURBSSet")); return (NURBSSet*)0;  }
	virtual ReferenceTarget* to_reftarg() { ABSTRACT_CONVERTER(ReferenceTarget*, MaxObject); }
	virtual Mesh*		to_mesh() { ABSTRACT_CONVERTER(Mesh*, Mesh); }
	virtual Thunk*		to_thunk() { ABSTRACT_CONVERTER(Thunk*, &-reference); }
	virtual void		to_fpvalue(FPValue& v) { throw ConversionError (this, _M("FPValue")); }

	virtual Renderer*	to_renderer() { ABSTRACT_CONVERTER(Renderer*, Renderer); }	// LAM: Added this 9/15/01

	virtual Box2&	to_box2() { throw ConversionError (this, _M("Box2")); return s_error_box2;  }
	virtual NURBSTextureSurface* to_nurbstexturesurface() { throw ConversionError (this, _M("NURBSTextureSurface")); return (NURBSTextureSurface*)0;  }
	virtual NURBSDisplay* to_nurbsdisplay() { throw ConversionError (this, _M("NURBSDisplay")); return (NURBSDisplay*)0;  }
	virtual TessApprox*	 to_tessapprox() { throw ConversionError (this, _M("TessApprox")); return (TessApprox*)0;  }

	virtual Value*	widen_to(Value* arg, Value** arg_list) { ABSTRACT_WIDENER(arg); }
	virtual BOOL	comparable(Value* arg) { return (tag == arg->tag); }
	virtual BOOL	is_const() { return FALSE; }
	// LAM - 7/8/03 - defect 504956 - following identifies classes that derive from MAXWrapper. Only other implementation is in MAXWrapper
	// used by garbage collector to prevent collection of MAXWrapper-derived values while doing light collection
	virtual BOOL	derives_from_MAXWrapper()  { return FALSE; } 

	ScripterExport virtual Value*	get_property(Value** arg_list, int count);
	ScripterExport virtual Value*	set_property(Value** arg_list, int count);
	ScripterExport		   Value*	_get_property(Value* prop);
	ScripterExport virtual Value*	_set_property(Value* prop, Value* val);
	virtual Value*	get_container_property(Value* prop, Value* cur_prop) { if (!dontThrowAccessorError) throw AccessorError (cur_prop, prop); return NULL; }
	virtual Value*	set_container_property(Value* prop, Value* val, Value* cur_prop) { throw AccessorError (cur_prop, prop); return NULL;}

	// polymorphic default type predicates - abstracted over by is_x(v) macros as needed
	virtual BOOL	_is_collection() { return FALSE; }
	virtual BOOL	_is_charstream() { return FALSE; }
	virtual BOOL	_is_rolloutcontrol() { return FALSE; }
	virtual BOOL	_is_rolloutthunk() { return FALSE; }
	virtual BOOL	_is_function()	 { return FALSE; }
	virtual BOOL	_is_selection()	 { return FALSE; }
	virtual BOOL	_is_thunk()		{ return FALSE; }
	virtual BOOL	_is_indirect_thunk() { return FALSE; }

	// yield selection set iterator if you can
	virtual SelectionIterator* selection_iterator() { throw RuntimeError (_M("Operation requires a selection (Array or BitArray)")); return NULL; }

	// scene persistence functions
	ScripterExport virtual IOResult Save(ISave* isave);
	// the Load fn is a static method on loadbale classes, see SceneIO.cpp & .h and each loadable class

	// called during MAX exit to have all MAXScript-side refs dropped (main implementation in MAXWrapper)
	virtual void drop_MAX_refs() { }

	// access ID'd FPInterface if supported
	virtual BaseInterface* GetInterface(Interface_ID id) { return NULL; }


	// stack allocation routines
	ScripterExport Value* make_heap_permanent();
	ScripterExport Value* make_heap_static() { Value* v = make_heap_permanent(); v->flags |= GC_STATIC; return v; }

	ScripterExport Value* get_heap_ptr() { if (!has_heap_copy()) return migrate_to_heap(); return is_on_stack() ? get_stack_heap_ptr() : this; } 
	ScripterExport Value* get_stack_heap_ptr() { return (Value*)next; }
	ScripterExport Value* migrate_to_heap();
	ScripterExport Value* get_live_ptr() { return is_on_stack() && has_heap_copy() ? get_stack_heap_ptr() : this; } 
	#pragma warning(pop)
};

inline Value* heap_ptr(Value* v) { return v ? v->get_heap_ptr() : NULL; }	// ensure v is in heap, migrate if not, return heap pointer
inline Value* live_ptr(Value* v) { return  v->get_live_ptr(); }				// get live pointer, if on stack & migrated, heap copy is live

/* ---------- the base class for all metaclasses  ---------- */

class MetaClassClass;
extern MetaClassClass value_metaclass;  // the metaclass class

class ValueMetaClass : public Value
{
// Whether the generic functions and property setters of class instances can be called from debugger thread stored 
// in Collectable::flags3 - bit 0. Default is false.
public:
	MCHAR*			name;
	UserProp*		user_props;		// additional, user defined property accessors
	short			uprop_count;
	UserGeneric*	user_gens;		//     "        "      "    generic fns
	short			ugen_count;
	Tab<FPInterfaceDesc*> prop_interfaces;	// static interfaces who methods appear as properties on instances of the class

					ValueMetaClass() { }
	ScripterExport	ValueMetaClass(MCHAR* iname);
	ScripterExport	~ValueMetaClass();

	ScripterExport BOOL	is_kind_of(ValueMetaClass* c);
#	define		is_valueclass(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == (ValueMetaClass*)&value_metaclass)
	ScripterExport void sprin1(CharStream* s);
	ScripterExport void	export_to_scripter();
	ScripterExport void add_user_prop(MCHAR* prop, value_cf getter, value_cf setter);
	ScripterExport void add_user_generic(MCHAR* name, value_cf fn);
	ScripterExport UserGeneric* find_user_gen(Value* name);
	ScripterExport UserProp* find_user_prop(Value* prop);

	// static property interfaces
	ScripterExport void add_prop_interface(FPInterfaceDesc* fpd) { prop_interfaces.Append(1, &fpd); }
	ScripterExport int num_prop_interfaces() { return prop_interfaces.Count(); }
	ScripterExport FPInterfaceDesc* get_prop_interface(int i) { return prop_interfaces[i]; }
};
#define CHECK_ARG_COUNT(fn, w, g)	if ((w) != (g)) throw ArgCountError (_M(#fn), w, g)

#define classof_methods(_cls, _super)					\
	Value* classOf_vf(Value** arg_list, int count)		\
	{													\
		UNUSED_PARAM(arg_list);							\
		CHECK_ARG_COUNT(classOf, 1, count + 1);			\
		return &_cls##_class;							\
	}													\
	Value* superClassOf_vf(Value** arg_list, int count)	\
	{													\
		UNUSED_PARAM(arg_list);							\
		CHECK_ARG_COUNT(superClassOf, 1, count + 1);	\
		return &_super##_class;							\
	}													\
	Value* isKindOf_vf(Value** arg_list, int count)		\
	{													\
		CHECK_ARG_COUNT(isKindOf, 2, count + 1);		\
		return (arg_list[0] == &_cls##_class) ?			\
			&true_value :								\
			_super::isKindOf_vf(arg_list, count);		\
	}													\
	BOOL is_kind_of(ValueMetaClass* c)					\
	{													\
		return (c == &_cls##_class) ? 1					\
					: _super::is_kind_of(c);			\
	}

#define visible_class(_cls)												\
	class _cls##Class : public ValueMetaClass							\
	{																	\
	public:																\
					_cls##Class(MCHAR* name) : ValueMetaClass (name) { }	\
		void		collect() { delete this; }							\
	};																	\
	extern ScripterExport _cls##Class _cls##_class;

#define visible_class_debug_ok(_cls)									\
	class _cls##Class : public ValueMetaClass							\
	{																	\
	public:																\
					_cls##Class(MCHAR* name) : ValueMetaClass (name) { flags3 |= VALUE_FLAGBIT_0; }	\
		void		collect() { delete this; }							\
	};																	\
	extern ScripterExport _cls##Class _cls##_class;

#define visible_class_s(_cls, _super)									\
	class _cls##Class : public ValueMetaClass							\
	{																	\
	public:																\
					_cls##Class(MCHAR* name) : ValueMetaClass (name) { }	\
		void		collect() { delete this; }							\
		Value*		classOf_vf(Value** arg_list, int count)				\
		{																\
			UNUSED_PARAM(arg_list);										\
			CHECK_ARG_COUNT(classOf, 1, count + 1);						\
			return &_super##_class;										\
		}																\
		Value*		superClassOf_vf(Value** arg_list, int count)		\
		{																\
			UNUSED_PARAM(arg_list);										\
			UNUSED_PARAM(count);										\
			return _super##_class.classOf_vf(NULL, 0);					\
		}																\
	};																	\
	extern ScripterExport _cls##Class _cls##_class;

#define applyable_class(_cls)											\
	class _cls##Class : public ValueMetaClass							\
	{																	\
	public:																\
					_cls##Class(MCHAR* name) : ValueMetaClass (name) { }\
		void		collect() { delete this; }							\
		ScripterExport Value* apply(Value** arglist, int count, CallContext* cc=NULL); \
	};																	\
	extern ScripterExport _cls##Class _cls##_class;

#define applyable_class_debug_ok(_cls)									\
	class _cls##Class : public ValueMetaClass							\
	{																	\
	public:																\
					_cls##Class(MCHAR* name) : ValueMetaClass (name) { flags3 |= VALUE_FLAGBIT_0; }\
		void		collect() { delete this; }							\
		ScripterExport Value* apply(Value** arglist, int count, CallContext* cc=NULL); \
	};																	\
	extern ScripterExport _cls##Class _cls##_class;

#define applyable_class_s(_cls, _super)									\
	class _cls##Class : public ValueMetaClass							\
	{																	\
	public:																\
					_cls##Class(MCHAR* name) : ValueMetaClass (name) { }\
		Value*		classOf_vf(Value** arg_list, int count)				\
		{																\
			UNUSED_PARAM(arg_list);										\
			CHECK_ARG_COUNT(classOf, 1, count + 1);						\
			return &_super##_class;										\
		}																\
		Value*		superClassOf_vf(Value** arg_list, int count)		\
		{																\
			UNUSED_PARAM(arg_list);										\
			UNUSED_PARAM(count);										\
			return _super##_class.classOf_vf(NULL, 0);					\
		}																\
		void		collect() { delete this; }							\
		ScripterExport Value* apply(Value** arglist, int count, CallContext* cc=NULL); \
	};																	\
	extern ScripterExport _cls##Class _cls##_class;

#define visible_class_instance(_cls, _name)				\
	ScripterExport _cls##Class _cls##_class (_M(_name));

#define invisible_class(_cls)											\
	class _cls##Class : public ValueMetaClass							\
	{																	\
	public:																\
					_cls##Class(MCHAR* name) : ValueMetaClass (name) { }	\
		void		collect() { delete this; }							\
		void		export_to_scripter() { }							\
	};																	\
	extern ScripterExport _cls##Class _cls##_class;

#define invisible_class_instance(_cls, _name)				\
	ScripterExport _cls##Class _cls##_class (_M(_name));


#define class_tag(_cls)				&_cls##_class

#define INTERNAL_CLASS_TAG				((ValueMetaClass*)0L)
#define INTERNAL_INDEX_THUNK_TAG		((ValueMetaClass*)1L)
#define INTERNAL_PROP_THUNK_TAG			((ValueMetaClass*)2L)
#define INTERNAL_LOCAL_THUNK_TAG		((ValueMetaClass*)3L)
#define INTERNAL_FREE_THUNK_TAG			((ValueMetaClass*)4L)
#define INTERNAL_RO_LOCAL_THUNK_TAG		((ValueMetaClass*)5L)
#define INTERNAL_CODE_TAG				((ValueMetaClass*)6L)
#define INTERNAL_SOURCEFILEWRAPPER_TAG	((ValueMetaClass*)7L)
#define INTERNAL_PIPE_TAG				((ValueMetaClass*)8L)
#define INTERNAL_TOOL_LOCAL_THUNK_TAG	((ValueMetaClass*)9L)
#define INTERNAL_GLOBAL_THUNK_TAG		((ValueMetaClass*)10L)
#define INTERNAL_CONST_GLOBAL_THUNK_TAG ((ValueMetaClass*)11L)
#define INTERNAL_SYS_GLOBAL_THUNK_TAG	((ValueMetaClass*)12L)
#define INTERNAL_PLUGIN_LOCAL_THUNK_TAG	((ValueMetaClass*)13L)
#define INTERNAL_PLUGIN_PARAM_THUNK_TAG	((ValueMetaClass*)14L)
#define INTERNAL_RCMENU_LOCAL_THUNK_TAG	((ValueMetaClass*)15L)
#define INTERNAL_STRUCT_MEM_THUNK_TAG	((ValueMetaClass*)16L)
#define INTERNAL_MSPLUGIN_TAG			((ValueMetaClass*)17L)
#define INTERNAL_STRUCT_TAG				((ValueMetaClass*)18L)
#define INTERNAL_MAKER_TAG				((ValueMetaClass*)19L)
#define INTERNAL_CODEBLOCK_LOCAL_TAG	((ValueMetaClass*)20L)
#define INTERNAL_CODEBLOCK_TAG			((ValueMetaClass*)21L)
#define INTERNAL_THUNK_REF_TAG			((ValueMetaClass*)22L)
#define INTERNAL_THUNK_DEREF_TAG		((ValueMetaClass*)23L)
#define INTERNAL_STRUCT_METHOD_TAG		((ValueMetaClass*)24L) // LAM - defect 307069
#define INTERNAL_MSPLUGIN_METHOD_TAG	((ValueMetaClass*)25L) // LAM - 9/6/02 - defect 291499
#define INTERNAL_CONTEXT_THUNK_TAG		((ValueMetaClass*)26L) // LAM - 2/8/05
#define INTERNAL_OWNER_THUNK_TAG		((ValueMetaClass*)27L) // LAM - 2/28/05
#define INTERNAL_RCMENU_ITEM_THUNK_TAG	((ValueMetaClass*)28L) // LAM - 3/21/05
#define INTERNAL_STANDINMSPLUGINCLASS_TAG	((ValueMetaClass*)29L) // LAM - 8/29/06
#define INTERNAL_SOURCEPOSWRAPPER_TAG		((ValueMetaClass*)30L) // LAM - 9/4/08

#define INTERNAL_TAGS					((ValueMetaClass*)100L)  // must be higher than all internal tags

visible_class_debug_ok (Value)

#define is_sourcepositionwrapper(v) ((v)->tag == INTERNAL_SOURCEPOSWRAPPER_TAG)

/* ---------- the distinguished value subclasses ---------- */

visible_class_debug_ok (Boolean)

class Boolean;
class ValueLoader;
extern ScripterExport Boolean true_value;
extern ScripterExport Boolean false_value;
#pragma warning(push)
#pragma warning(disable:4100)

class Boolean : public Value
{
public:
			Boolean() { tag = &Boolean_class; }
			classof_methods (Boolean, Value);
	void	collect() { delete this; }
	void	sprin1(CharStream* s);

#	define	is_bool(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == &Boolean_class)
	Value*	not_vf(Value**arg_list, int count);
	Value*  copy_vf(Value** arg_list, int count) { return this; }
	BOOL	to_bool() { return this == &true_value; }
	void	to_fpvalue(FPValue& v) { v.i = (this == &true_value) ? 1 : 0; v.type = (ParamType2)TYPE_BOOL; }

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

/* ----- */

visible_class_debug_ok (Undefined)

class Undefined : public Value
{
public:
			Undefined() { tag = &Undefined_class; }
			classof_methods (Undefined, Value);
	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s);
	Value*  copy_vf(Value** arg_list, int count) { return this; }

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
	Mtl*	to_mtl() { return NULL; }		// undefined is a NULL material
	void	to_fpvalue(FPValue& v);
};

extern ScripterExport Undefined undefined;
extern ScripterExport Undefined dontCollect;
extern ScripterExport Undefined loopExit;

/* ----- */

visible_class_debug_ok (Ok)

class Ok : public Value
{
public:
			Ok() { tag = &Ok_class; }
			classof_methods (Ok, Value);
	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s);
	Value*  copy_vf(Value** arg_list, int count) { return this; }

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
	void	to_fpvalue(FPValue& v);
};

extern ScripterExport Ok ok;

/* ----- */

visible_class_debug_ok (Empty)

class Empty : public Value
{
public:
			Empty() { tag = &Empty_class; }
			classof_methods (Empty, Value);
	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s);
	Value*  copy_vf(Value** arg_list, int count) { return this; }

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
	void	to_fpvalue(FPValue& v);
};

extern ScripterExport Empty empty;

/* ----- */

visible_class_debug_ok (Unsupplied)

class Unsupplied : public Value
{
public:
			Unsupplied() { tag = &Unsupplied_class; }
			classof_methods (Unsupplied, Value);
	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s);
	Value*  copy_vf(Value** arg_list, int count) { return this; }

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
	void	to_fpvalue(FPValue& v);
};
#pragma warning(pop)
extern ScripterExport Unsupplied unsupplied;
