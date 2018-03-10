/*    Numbers.h - the number family of classes - numbers for MAXScript
 *
 *    Copyright (c) John Wainwright, 1996
 *    
 *
 */

#pragma once

#include "..\kernel\value.h"
#include "mxstime.h"

// forward declarations
// This file defines the following classes:
class Float;
class Double;
class Integer;
class Integer64;
class IntegerPtr;

#define FLOAT_CACHE_SIZE   1024  // must be power of 2
#define DOUBLE_CACHE_SIZE  512   //   "       "      "
#define INT_CACHE_SIZE     512   //   "       "      "
#define INT64_CACHE_SIZE   128   //   "       "      "
#define LOW_INT_RANGE      100

extern Float* float_cache[];
extern Double* double_cache[];
extern Integer* int_cache[];
extern Integer64* int64_cache[];

visible_class_debug_ok (Number)

class Number : public Value
{
public:
#  define  is_integer_number(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(Integer) || (v)->tag == class_tag(Integer64) || (v)->tag == class_tag(IntegerPtr))
#  define  is_float_number(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(Float) || (v)->tag == class_tag(Double))
#  define  is_number(o) (is_integer_number(o) || is_float_number(o))
			classof_methods (Number, Value);

	static   Value* read(MCHAR* str, bool heapAlloc = false);
	static   void setup();

#include "..\macros\define_implementations.h"
	def_generic( coerce, "coerce");
	def_generic( copy,   "copy");
};

applyable_class_debug_ok (Float)
applyable_class_debug_ok (Double)
applyable_class_debug_ok (Integer)     // forward decls for float class 
applyable_class_debug_ok (Integer64)            
applyable_class_debug_ok (IntegerPtr)
#define  is_double(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(Double))
#define  is_integer(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(Integer))
#define  is_integer64(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(Integer64))
#define  is_integerptr(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(IntegerPtr))

class Float : public Number
{
public:
	float value;

	ENABLE_STACK_ALLOCATE(Float);

				   Float() { }
	ScripterExport Float(float init_val);

	static ScripterExport Value* intern(float init_val) { return new Float (init_val); }  // hey!! no longer interns, stack alloc'd instead
	static ScripterExport Value* heap_intern(float init_val);

	classof_methods (Float, Number);
#  define  is_float(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(Float))
	void  collect() { delete this; }
	ScripterExport void  sprin1(CharStream* s);

	/* include all the protocol declarations */

#include "..\macros\define_implementations.h"
#  include "..\protocols\math.inl"

	float    to_float() { return value; }
	double   to_double() { return static_cast<double>(value); }
	int      to_int();
	INT64    to_int64() { return static_cast<INT64>(value); }
	INT_PTR  to_intptr()
	{
#ifndef _WIN64
		return to_int();
#else
		return to_int64();
#endif
	}
	TimeValue to_timevalue() { return (TimeValue)(value * GetTicksPerFrame()); }  // numbers used as times are in frames
	void  to_fpvalue(FPValue& v) { v.f = to_float(); v.type = (ParamType2)TYPE_FLOAT; }

	Value*   widen_to(Value* arg, Value** arg_list);
	BOOL  comparable(Value* arg) { return (is_number(arg) || is_time(arg)); }

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

class Double : public Number
{
public:
	double value;

	ENABLE_STACK_ALLOCATE(Double);

	Double() { }
	ScripterExport Double(double init_val);

	static ScripterExport Value* intern(double init_val) { return new Double (init_val); } 
	static ScripterExport Value* heap_intern(double init_val);

	classof_methods (Double, Number);
#  define  is_double(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(Double))
	void  collect() { delete this; }
	ScripterExport void  sprin1(CharStream* s);

	/* include all the protocol declarations */

#include "..\macros\define_implementations.h"
#  include "..\protocols\math.inl"

	float    to_float() { return static_cast<float>(value); }
	double   to_double() { return value; }
	int      to_int();
	INT64    to_int64() { return static_cast<INT64>(value); }
	INT_PTR  to_intptr()
	{
#ifndef _WIN64
		return to_int();
#else
		return to_int64();
#endif
	}
	TimeValue to_timevalue() { return (TimeValue)(value * GetTicksPerFrame()); }  // numbers used as times are in frames
	void  to_fpvalue(FPValue& v) { v.dbl = to_double(); v.type = (ParamType2)TYPE_DOUBLE; }

	Value*   widen_to(Value* arg, Value** arg_list);
	BOOL  comparable(Value* arg) { return (is_number(arg) || is_time(arg)); }

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};


class Integer : public Number
{
public:
	int value;

	ENABLE_STACK_ALLOCATE(Integer);

				   Integer() { };
	ScripterExport Integer(int init_val);

	static  ScripterExport Value* intern(int init_val) { return new Integer (init_val); }  // hey!! no longer interns, stack alloc'd instead
	static  ScripterExport Value* heap_intern(int init_val);

	classof_methods (Integer, Number);
#  define  is_int(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(Integer))
	void  collect() { delete this; }
	ScripterExport void  sprin1(CharStream* s);

	/* include all the protocol declarations */

#include "..\macros\define_implementations.h"
#  include "..\protocols\math.inl"

	float    to_float() { return static_cast<float>(value); }
	double   to_double() { return static_cast<double>(value); }
	int      to_int() { return value; }
	INT_PTR  to_intptr() { return static_cast<INT_PTR>(value); }
	INT64    to_int64() { return static_cast<INT64>(value); }
	TimeValue to_timevalue() { return (TimeValue)(value * GetTicksPerFrame()); }  // numbers used as times are in frames
	void  to_fpvalue(FPValue& v) { v.i = to_int(); v.type = (ParamType2)TYPE_INT; }

	Value*   widen_to(Value* arg, Value** arg_list);
	BOOL  comparable(Value* arg) { return (is_number(arg) || is_time(arg)); }

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

class Integer64 : public Number     
{
public:
	INT64 value;

	ENABLE_STACK_ALLOCATE(Integer64);

				   Integer64() { };
	ScripterExport Integer64(INT64 init_val);

	static  ScripterExport Value* intern(INT64 init_val) { return new Integer64(init_val); }
	static  ScripterExport Value* heap_intern(INT64 init_val);

	classof_methods (Integer64, Number);
#  define  is_int64(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(Integer64))
	void  collect() { delete this; }
	ScripterExport void  sprin1(CharStream* s);

	/* include all the protocol declarations */

#include "..\macros\define_implementations.h"
#  include "..\protocols\math.inl"

	float    to_float() { return static_cast<float>(value); }
	double   to_double() { return static_cast<double>(value); }
	int      to_int() { return static_cast<int>(value); }
	INT_PTR  to_intptr() { return static_cast<INT_PTR>(value); }
	INT64    to_int64() { return value; }
	TimeValue to_timevalue() { return (TimeValue)(value * GetTicksPerFrame()); }
	void  to_fpvalue(FPValue& v) { v.i64 = to_int64(); v.type = (ParamType2)TYPE_INT64; }

	Value*   widen_to(Value* arg, Value** arg_list);
	BOOL  comparable(Value* arg) { return (is_number(arg) || is_time(arg)); }

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

/*
 Used to store temporary values which cannot/should not be streamed out.  This includes
 HWND, pointers, etc.  Everything that changes size between Win32 and Win64, in other
 words.
*/
class IntegerPtr : public Number
{
public:
	INT_PTR value;

	ENABLE_STACK_ALLOCATE(IntegerPtr);

				   IntegerPtr() { };
	ScripterExport IntegerPtr(INT_PTR init_val);

	static  ScripterExport Value* intern(INT_PTR init_val) { return new IntegerPtr(init_val); }
	static  ScripterExport Value* heap_intern(INT_PTR init_val);

	classof_methods (IntegerPtr, Number);
#  define  is_intptr(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(IntegerPtr))
	void  collect() { delete this; }
	ScripterExport void  sprin1(CharStream* s);

	/* include all the protocol declarations */

#include "..\macros\define_implementations.h"
#  include"..\protocols\math.inl"

	float    to_float()  { return static_cast<float>(value); }
	double   to_double() { return static_cast<double>(value); }
	int      to_int()    { return static_cast<int>(value); }
	INT_PTR  to_intptr() { return value; }
	INT64    to_int64()  { return static_cast<INT64>(value); }
	TimeValue to_timevalue() { return (TimeValue)(value * GetTicksPerFrame()); }
	void  to_fpvalue(FPValue& v) { v.intptr = to_intptr(); v.type = (ParamType2)TYPE_INTPTR; }

	Value*   widen_to(Value* arg, Value** arg_list);
	BOOL  comparable(Value* arg) { return (is_number(arg) || is_time(arg)); }

	// scene I/O -- throws exceptions (we're not supposed to load/save IntegerPtr)
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

