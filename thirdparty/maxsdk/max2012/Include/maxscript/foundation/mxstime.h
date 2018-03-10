/*		MSTime.h - the time family of classes for MAXScript
 *
 *		Copyright (c) John Wainwright, 1996
 *		
 *
 */

#pragma once

#include "..\kernel\value.h"
#include "..\..\units.h"

/* ------------------------ Time ------------------------------ */

visible_class_debug_ok (MSTime)

class MSTime : public Value
{
public:
	TimeValue	time;

	ENABLE_STACK_ALLOCATE(MSTime);

				MSTime (TimeValue t);
	static ScripterExport Value* intern(TimeValue t);

#	define		is_time(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(MSTime))
				classof_methods (MSTime, Value);
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);

#include "..\macros\define_implementations.h"
#	include "..\protocols\time.inl"
	def_generic  ( coerce,	"coerce");

	def_property ( ticks );
	def_property ( frame );
	def_property ( normalized );

	TimeValue	to_timevalue() { return time; }
	float	    to_float() { return (float)time / GetTicksPerFrame(); }
	double	    to_double() { return static_cast<double>(time) / GetTicksPerFrame(); }
	int			to_int() { return (int)time / GetTicksPerFrame(); }
	INT64       to_int64() { return static_cast<INT64>(time / GetTicksPerFrame()); }
	INT_PTR     to_intptr() { return static_cast<INT_PTR>(time / GetTicksPerFrame()); }
	void		to_fpvalue(FPValue& v) { v.i = time; v.type = TYPE_TIMEVALUE; }

	Value*	widen_to(Value* arg, Value** arg_list);
	BOOL	comparable(Value* arg);

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

/* ------------------------ Interval ------------------------------ */

applyable_class_debug_ok (MSInterval)

class MSInterval : public Value
{
public:
	Interval	interval;

	ENABLE_STACK_ALLOCATE(MSInterval);


				MSInterval () {};
 ScripterExport MSInterval (Interval i);
 ScripterExport MSInterval (TimeValue s, TimeValue e);

#	define		is_interval(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(MSInterval))
				classof_methods (MSInterval, Value);
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);

#include "..\macros\define_implementations.h"
	def_property ( start );
	def_property ( end );

	Interval	to_interval() { return interval; }
	ScripterExport void to_fpvalue(FPValue& v);

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

