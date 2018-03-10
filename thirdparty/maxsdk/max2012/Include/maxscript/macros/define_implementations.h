/*	
 *		define_implementations.h - macros for making implementation declarations
 *									for MAXscript functions
 *
 *	This will typically be used along with protocol definition files in 
 *  the Value subclasses that actually implement those protocols.
 *
 *  see define_abstract_functions.h for more info.
 *
 *	
 *	Copyright © John Wainwright 1996
 *
 */

/* def_generic macro for implementation declaration in Value subclasses */

#ifdef def_generic
#	undef def_generic
#	undef def_node_generic
#	undef def_mapped_generic
#	undef def_visible_generic
#	undef def_struct_generic
#	undef def_generic_debug_ok
#	undef def_node_generic_debug_ok
#	undef def_mapped_generic_debug_ok
#	undef def_visible_generic_debug_ok
#	undef def_struct_generic_debug_ok
#	undef use_generic
#	undef def_primitive
#	undef def_mapped_primitive
#	undef def_lazy_primitive
#	undef def_visible_lazy_primitive
#	undef def_visible_primitive
#	undef def_struct_primitive
#	undef def_primitive_debug_ok
#	undef def_mapped_primitive_debug_ok
#	undef def_lazy_primitive_debug_ok
#	undef def_visible_lazy_primitive_debug_ok
#	undef def_visible_primitive_debug_ok
#	undef def_struct_primitive_debug_ok
#	undef def_property
#	undef def_property_alias
#	undef def_2_prop_path
#	undef def_2_prop_path_alias
#	undef def_nested_prop
#	undef def_nested_prop_alias
#endif
#ifdef def_prop_getter
#	undef def_prop_getter
#	undef def_prop_setter
#endif

#define def_generic(fn, name)	\
	Value* fn##_vf(Value** arglist, int arg_count)
#define def_visible_generic(fn, name) def_generic(fn, name)
#define def_struct_generic(fn, name) def_generic(fn, name)
#define def_node_generic(fn, name) def_generic(fn, name)
#define def_mapped_generic(fn, name) def_generic(fn, name)
#define use_generic(fn, name) def_generic(fn, name)

#define def_generic_debug_ok(fn, name) def_generic(fn, name)
#define def_visible_generic_debug_ok(fn, name) def_generic(fn, name)
#define def_struct_generic_debug_ok(fn, name) def_generic(fn, name)
#define def_node_generic_debug_ok(fn, name) def_generic(fn, name)
#define def_mapped_generic_debug_ok(fn, name) def_generic(fn, name)

#define def_primitive(fn, name)		// nothing for implementation...
#define def_lazy_primitive(fn, name) def_primitive(fn, name)
#define def_visible_lazy_primitive(fn, name) def_primitive(fn, name)
#define def_visible_primitive(fn, name) def_primitive(fn, name)
#define def_mapped_primitive(fn, name) def_primitive(fn, name)
#define def_struct_primitive(fn, _struct, name) def_primitive(fn, name)

#define def_primitive_debug_ok(fn, name)		// nothing for implementation...
#define def_visible_primitive_debug_ok(fn, name)
#define def_mapped_primitive_debug_ok(fn, name)
#define def_lazy_primitive_debug_ok(fn, name)
#define def_visible_lazy_primitive_debug_ok(fn, name)
#define def_struct_primitive_debug_ok(fn, _struct, name)

#define def_property(p)										\
			Value*	get_##p(Value** arg_list, int count);	\
			Value*	set_##p(Value** arg_list, int count)
#define def_prop_getter(p)									\
			Value*	get_##p(Value** arg_list, int count)
#define def_prop_setter(p)									\
			Value*	set_##p(Value** arg_list, int count)
#define def_property_alias(p, real_p)
#define def_2_prop_path(p1, p2)										\
			Value*	get_##p1##_##p2(Value** arg_list, int count);	\
			Value*	set_##p1##_##p2(Value** arg_list, int count)
#define def_2_prop_path_alias(p1, p2, real_p1, real_p2)
#define def_nested_prop(p1)											\
			Value*	get_nested_##p1(Value** arg_list, int count);	\
			Value*	set_nested_##p1(Value** arg_list, int count)

#define def_backpatched_setter(_prop, _superclass)					\
			Value*													\
			set_##_prop(Value** arg_list, int count)				\
			{														\
				_superclass::set_##_prop(arg_list, count);			\
				back_patch();										\
				return arg_list[0];									\
			};

#define def_local_prop_alias(p, real_p)																\
			Value*	get_##p(Value** arg_list, int count) { return get_##real_p(arg_list, count); }   \
			Value*	set_##p(Value** arg_list, int count) { return set_##real_p(arg_list, count); }   \

#ifdef def_time_fn
#	undef def_time_fn
#endif
#define def_time_fn(_fn)	\
	Value* _fn##_vf(Value** arglist, int arg_count)

#undef def_name
#define def_name(name)	n_##name = Name::intern(_M(#name));		

#undef def_marker
#define def_marker(var, str)

/* ---------------------------- utility macros ---------------------*/

#define _def_num_bin_op(_class, _conv, _op_fn, _op, _member)	\
	Value*														\
	_class::_op_fn##_vf(Value** arg_list, int count)			\
	{															\
		one_value_local(widened);								\
		Value *arg, *result;									\
		arg = arg_list[0];										\
		if (tag != arg->tag && ((vl.widened = widen_to(arg, arg_list)) != NULL))	\
			result = vl.widened->_op_fn##_vf(arg_list, 1);		\
		else													\
			result = _class::intern(_member _op (arg_list[0])->_conv()); \
		pop_value_locals();										\
		return result;											\
	}

#define _def_bin_op(_class, _conv, _op_fn, _op, _member)		\
	Value*														\
	_class::_op_fn##_vf(Value** arg_list, int count)			\
	{															\
		one_value_local(widened);								\
		Value *arg, *result;									\
		arg = arg_list[0];										\
		if (tag != arg->tag && ((vl.widened = widen_to(arg, arg_list)) != NULL))	\
			result = vl.widened->_op_fn##_vf(arg_list, 1);		\
		else													\
			result = new _class (_member _op (arg_list[0])->_conv()); \
		pop_value_locals();										\
		return result;											\
	}

#define _def_rel_op(_class, _conv, _op_fn, _rel_op, _member)	\
	Value*														\
	_class::_op_fn##_vf(Value** arg_list, int count)			\
	{															\
		Value *cmpnd = arg_list[0], *result;					\
		if (comparable(cmpnd))									\
			result = (_member _rel_op cmpnd->_conv()) ?			\
						&true_value : &false_value;				\
		else													\
			throw IncompatibleTypes(this, cmpnd);				\
		return result;											\
	}
//! Define an equal operator (i.e. '==') for maxscript visible values.
#define def_eq_op(_class, comparison_method, _member)			\
	Value*														\
	_class::eq_vf(Value** arg_list, int count)					\
	{															\
		Value* cmpnd = arg_list[0];								\
		Value* result;											\
		if (comparable(cmpnd))									\
			result = (_member == cmpnd->comparison_method()) ?	\
				&true_value : &false_value;						\
		else													\
			result = &false_value;								\
		return result;											\
	}
//! Define a NOT equal operator (i.e. '!=') for maxscript visible values.
#define def_ne_op(_class, comparison_method, _member)			\
	Value*														\
	_class::ne_vf(Value** arg_list, int count)					\
	{															\
		Value* cmpnd = arg_list[0];								\
		Value* result;											\
		if (comparable(cmpnd))									\
			result = (_member == cmpnd->comparison_method()) ?	\
						&false_value : &true_value;				\
		else													\
			result = &true_value;								\
		return result;											\
	}


#define _def_num_un_op(_class, _op_fn, _op, _member)			\
	Value*														\
	_class::_op_fn##_vf(Value** arg_list, int count)			\
	{															\
		return _class::intern(_op _member);						\
	}

#define _def_un_op(_class, _op_fn, _op, _member)				\
	Value*														\
	_class::_op_fn##_vf(Value** arg_list, int count)			\
	{															\
		return new _class (_op _member);						\
	}

#define def_bin_op(_class, _conv, _op_fn, _op)					\
			_def_num_bin_op(_class, _conv, _op_fn, _op, value)
#define def_rel_op(_class, _conv, _op_fn, _rel_op)				\
			_def_rel_op(_class, _conv, _op_fn, _rel_op, value)
#define def_un_op(_class, _op_fn, _op)							\
			_def_num_un_op(_class, _op_fn, _op, value)

#define PI_double     (3.14159265358979323846)
#define TWOPI_double  (6.28318530717958647652)
#define HALFPI_double (1.57079632679489661973)

#define DEG_TO_RAD_double (PI_double/180.0)
#define RAD_TO_DEG_double (180.0/PI_double)
#define DegToRad_double(deg) (((double)deg)*DEG_TO_RAD_double)
#define RadToDeg_double(rad) (((double)rad)*RAD_TO_DEG_double)

#define DegToRad_float(deg) DegToRad(deg)
#define RadToDeg_float(rad) RadToDeg(rad)

#define def_math_fn(_class, _outclass, _type, _conv, _fn)			\
	Value*												\
	_class::_fn##_vf(Value** arg_list, int count)		\
	{													\
		check_arg_count(_fn, 1, count+1);				\
		return _outclass::intern((_type)_fn(_conv()));		\
	}

#define def_angle_trig_fn(_class, _outclass, _type, _conv, _fn)				\
	Value*															\
	_class::_fn##_vf(Value** arg_list, int count)					\
	{																\
		check_arg_count(_fn, 1, count+1);							\
		return _outclass::intern(RadToDeg_##_type((_type)_fn(_conv()))); \
	}

#define def_float_trig_fn(_class, _outclass, _type, _conv, _fn)				\
	Value*															\
	_class::_fn##_vf(Value** arg_list, int count)					\
	{																\
		check_arg_count(_fn, 1, count+1);							\
		return _outclass::intern((_type)_fn(DegToRad_##_type(_conv()))); \
	}

#define def_math_bin_fn(_class, _outclass, _type, _conv, _fn)				\
	Value*														\
	_class::_fn##_vf(Value** arg_list, int count)				\
	{															\
		check_arg_count(_fn, 2, count+1);						\
		return _outclass::intern((_type)_fn(_conv(),				\
							  arg_list[0]->_conv()));			\
	}

#define def_angle_trig_bin_fn(_class, _outclass, _type, _conv, _fn)			\
	Value*															\
	_class::_fn##_vf(Value** arg_list, int count)					\
	{																\
		check_arg_count(_fn, 2, count+1);							\
		return _outclass::intern(RadToDeg_##_type((_type)_fn(_conv(),	\
							  arg_list[0]->_conv())));				\
	}

#define def_num_prop_accessors(_class, _prop, _member, _type, _conv ) \
	Value*															\
	_class::get_##_prop(Value** arg_list, int count)				\
	{																\
		return _type::intern(_member);								\
	}																\
	Value*															\
	_class::set_##_prop(Value** arg_list, int count)				\
	{																\
		Value*  val = arg_list[0];									\
		_member = val->_conv();										\
		return val;													\
	}

#define def_prop_accessors(_class, _prop, _member, _type, _conv )	\
	Value*															\
	_class::get_##_prop(Value** arg_list, int count)				\
	{																\
		return new _type (_member);									\
	}																\
	Value*															\
	_class::set_##_prop(Value** arg_list, int count)				\
	{																\
		Value*  val = arg_list[0];									\
		_member = val->_conv();										\
		return val;													\
	}

#define def_fn_prop_accessors(_class, _prop, _getter, _setter)		\
	Value*															\
	_class::get_##_prop(Value** arg_list, int count)				\
	{																\
		return _getter;												\
	}																\
	Value*															\
	_class::set_##_prop(Value** arg_list, int count)				\
	{																\
		Value*  val = arg_list[0];									\
		_setter;													\
		return val;													\
	}

#define def_fn_prop_getter(_class, _prop, _getter)					\
	Value*															\
	_class::get_##_prop(Value** arg_list, int count)				\
	{																\
		return _getter;												\
	}																

#define def_fn_prop_setter(_class, _prop, _setter)					\
	Value*															\
	_class::set_##_prop(Value** arg_list, int count)				\
	{																\
		Value*  val = arg_list[0];									\
		_setter;													\
		return val;													\
	}

#define def_float_prop_accessors(_class, _prop, _member)				\
			def_num_prop_accessors(_class, _prop, _member, Float, to_float)

#define def_int_prop_accessors(_class, _prop, _member)					\
			def_num_prop_accessors(_class, _prop, _member, Integer, to_int)

#define def_point3_prop_accessors(_class, _prop, _member)				\
			def_fn_prop_accessors(_class, _prop, new Point3Value (_member), _member = val->to_point3())

#define def_angle_prop_accessors(_class, _prop, _member)				\
			def_fn_prop_accessors(_class, _prop, Float::intern(RadToDeg(_member)), _member = DegToRad(val->to_float()))

#define def_time_bin_op(_op_fn, _op)							\
			_def_bin_op(MSTime, to_timevalue, _op_fn, _op, time)
#define def_time_rel_op(_op_fn, _rel_op)						\
			_def_rel_op(MSTime, to_timevalue, _op_fn, _rel_op, time)
#define def_time_un_op(_op_fn, _op)								\
			_def_un_op(MSTime, _op_fn, _op, time)

#define def_quat_bin_op(_class, _conv, _op_fn, _op)				\
			_def_bin_op(_class, _conv, _op_fn, _op, q)
#define def_quat_rel_op(_class, _conv, _op_fn, _rel_op)			\
			_def_rel_op(_class, _conv, _op_fn, _rel_op, q)
#define def_quat_un_op(_class, _op_fn, _op)						\
			_def_un_op(_class, _op_fn, _op, q)
				
#define def_new_quat_fn(_fn, _arg_count, _call)				\
	Value*													\
	QuatValue::_fn##_vf(Value** arg_list, int count)		\
	{														\
		check_arg_count(_fn, _arg_count, count + 1);		\
		return new QuatValue (_call);						\
	}

#define def_mut_quat_fn(_fn, _arg_count, _call)				\
	Value*													\
	QuatValue::_fn##_vf(Value** arg_list, int count)		\
	{														\
		check_arg_count(_fn, _arg_count, count + 1);		\
		_call;												\
		return this;										\
	}

#define def_new_mat_fn(_fn, _arg_count, _call)				\
	Value*													\
	Matrix3Value::_fn##_vf(Value** arg_list, int count)		\
	{														\
		check_arg_count(_fn, _arg_count, count + 1);		\
		return new Matrix3Value (_call);						\
	}

#define def_mut_mat_fn(_fn, _arg_count, _call)				\
	Value*													\
	Matrix3Value::_fn##_vf(Value** arg_list, int count)		\
	{														\
		check_arg_count(_fn, _arg_count, count + 1);		\
		_call;												\
		return this;										\
	}

#define def_mat_primitive(_fn, _arg_count, _call)			\
	Value*													\
	_fn##_cf(Value** arg_list, int count)					\
	{														\
		check_arg_count(_fn, _arg_count, count);			\
		return new Matrix3Value (_call);					\
	}

#define def_mat_bin_op(_class, _conv, _op_fn, _op)			\
			_def_bin_op(_class, _conv, _op_fn, _op, m)
#define def_mat_rel_op(_class, _conv, _op_fn, _rel_op)		\
			_def_rel_op(_class, _conv, _op_fn, _rel_op, m)
#define def_mat_un_op(_class, _op_fn, _op)					\
			_def_un_op(_class, _op_fn, _op, m)
				
// LAM: 2/27/01 - modified following to invalidate foreground/background rectangle instead of just 
// foreground

#define def_bool_node_fns(name, getter, setter)													\
	Value* node_get_##name(ReferenceTarget* obj, Value* prop, TimeValue t, Interval& valid)		\
	{																							\
		return ((INode*)obj)->getter() ? &true_value : &false_value;							\
	}																							\
	void node_set_##name(ReferenceTarget* obj, Value* prop, TimeValue t, Value* val)			\
	{																							\
		((INode*)obj)->setter(val->to_bool());													\
		InvalidateNodeRect((INode*)obj,t);														\
	}

#define def_bool_node_getter(name, getter)														\
	Value* node_get_##name(ReferenceTarget* obj, Value* prop, TimeValue t, Interval& valid)		\
	{																							\
		return ((INode*)obj)->getter() ? &true_value : &false_value;							\
	}																							\
	void node_set_##name(ReferenceTarget* obj, Value* prop, TimeValue t, Value* val)			\
	{																							\
		throw RuntimeError (_M("Property not settable: "), #name);								\
	}
																							
#define def_ulong_node_fns(name, getter, setter)												\
	Value* node_get_##name(ReferenceTarget* obj, Value* prop, TimeValue t, Interval& valid)		\
	{																							\
		return Integer::intern((int)((INode*)obj)->getter());									\
	}																							\
	void node_set_##name(ReferenceTarget* obj, Value* prop, TimeValue t, Value* val)			\
	{																							\
		((INode*)obj)->setter((ULONG)val->to_int());											\
		InvalidateNodeRect((INode*)obj,t);														\
	}

#define def_color_node_fns(name, getter, setter)												\
	Value* node_get_##name(ReferenceTarget* obj, Value* prop, TimeValue t, Interval& valid)		\
	{																							\
		DWORD rgb = ((INode*)obj)->getter();													\
		return ColorValue::intern(GetRValue(rgb) / 255.0f, GetGValue(rgb) / 255.0f, GetBValue(rgb) / 255.0f);	\
	}																							\
	void node_set_##name(ReferenceTarget* obj, Value* prop, TimeValue t, Value* val)			\
	{																							\
		AColor c;																				\
		c = val->to_acolor();																	\
		((INode*)obj)->setter(RGB((BYTE)(c.r * 255), (BYTE)(c.g * 255), (BYTE)(c.b * 255)));	\
		InvalidateNodeRect((INode*)obj,t);														\
	}

// LAM: 2/27/01 - use following for node properties that don't require a redraw

// The following defines has been added
// in 3ds max 4.2.  If your plugin utilizes this new
// mechanism, be sure that your clients are aware that they
// must run your plugin with 3ds max version 4.2 or higher.

#define def_bool_node_noredraw_fns(name, getter, setter)													\
	Value* node_get_##name(ReferenceTarget* obj, Value* prop, TimeValue t, Interval& valid)		\
	{																							\
		return ((INode*)obj)->getter() ? &true_value : &false_value;							\
	}																							\
	void node_set_##name(ReferenceTarget* obj, Value* prop, TimeValue t, Value* val)			\
	{																							\
		((INode*)obj)->setter(val->to_bool());													\
	}

#define def_ulong_node_noredraw_fns(name, getter, setter)												\
	Value* node_get_##name(ReferenceTarget* obj, Value* prop, TimeValue t, Interval& valid)		\
	{																							\
		return Integer::intern((int)((INode*)obj)->getter());									\
	}																							\
	void node_set_##name(ReferenceTarget* obj, Value* prop, TimeValue t, Value* val)			\
	{																							\
		((INode*)obj)->setter((ULONG)val->to_int());											\
	}

#define def_color_node_noredraw_fns(name, getter, setter)												\
	Value* node_get_##name(ReferenceTarget* obj, Value* prop, TimeValue t, Interval& valid)		\
	{																							\
		DWORD rgb = ((INode*)obj)->getter();													\
		return ColorValue::intern(GetRValue(rgb) / 255.0f, GetGValue(rgb) / 255.0f, GetBValue(rgb) / 255.0f);	\
	}																							\
	void node_set_##name(ReferenceTarget* obj, Value* prop, TimeValue t, Value* val)			\
	{																							\
		AColor c;																				\
		c = val->to_acolor();																	\
		((INode*)obj)->setter(RGB((BYTE)(c.r * 255), (BYTE)(c.g * 255), (BYTE)(c.b * 255)));	\
	}

// End of 3ds max 4.2 Extension