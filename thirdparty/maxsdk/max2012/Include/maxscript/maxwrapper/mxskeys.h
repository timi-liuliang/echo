/*	
 *		MAXKeys.h - MAX controller keyframe access classes
 *
 *			Copyright (c) John Wainwright 1996
 *
 */

#pragma once

#include "..\kernel\value.h"
#include "..\kernel\exceptions.h"

// forward declaration
class MAXControl;
class IKey;
class AnyKey;
class IKeyControl;
class ParamDimension;

visible_class (MAXKeyArray)

class MAXKeyArray : public Value
{
public:
	MAXControl*		 controller;		/* the controller							*/

	ENABLE_STACK_ALLOCATE(MAXKeyArray);
	ScripterExport	MAXKeyArray(Control* icont, ParamDimension* idim);

				classof_methods (MAXKeyArray, Value);
	BOOL		_is_collection() { return 1; }
	void		collect() { delete this; }
	void		gc_trace();
	ScripterExport void sprin1(CharStream* s);

	/* operations */
	
#include "..\macros\define_implementations.h"
#	include "..\protocols\arrays.inl"
	def_generic ( sortKeys,			"sortKeys");
	def_generic ( addNewKey,		"addNewKey");
	def_generic ( deleteKeys,		"deleteKeys");
	def_generic ( deleteKey,		"deleteKey");
//	def_generic ( append_key,		"appendKey"); // RK: 6/19/02, Commenting these, breaks the SDK
//	def_generic ( assign_key,		"assignKey"); // RK: 6/19/02, Commenting these, breaks the SDK

	def_generic (show_props,		"showProperties");
	def_generic (get_props,			"getPropNames");

	ScripterExport Value* map(node_map& m);

	/* built-in property accessors */

	def_property ( count );
};

#define ToTCBUI(a) (((a)+1.0f)*25.0f)  // HEY!! pinched from TCBINTRP.CPP, why not in a header or documented?
#define FromTCBUI(a) (((a)/25.0f)-1.0f)
#define ToEaseUI(a) ((a)*50.0f)
#define FromEaseUI(a) ((a)/50.0f)

visible_class (MAXKey)

class MAXKey : public Value
{
public:
	MAXControl*	controller;			/* MAX-side controller						*/
	int			key_index;

	ENABLE_STACK_ALLOCATE(MAXKey);
	ScripterExport MAXKey (Control* icont, int ikey, ParamDimension* dim);
	ScripterExport MAXKey (Control* icont, int ikey);
	ScripterExport MAXKey (MAXControl* icont, int ikey);

	static void setup();

				classof_methods (MAXKey, Value);
	void		collect() { delete this; }
	void		gc_trace();
	ScripterExport void sprin1(CharStream* s);

	def_generic ( delete,		"delete");
	def_generic ( copy,			"copy");
	def_generic	( show_props,	"showProperties");
	def_generic ( get_props,	"getPropNames");

	ScripterExport IKey* setup_key_access(AnyKey& ak, IKeyControl** kip);

	def_property	(time);
	def_property	(selected);
	def_property	(value);
	def_property	(inTangent);
	def_property	(outTangent);
	def_property	(inTangentLength);
	def_property	(outTangentLength);
	def_property	(inTangentType);
	def_property	(outTangentType);
	def_property	(x_locked);
	def_property	(y_locked);
	def_property	(z_locked);
	def_property	(w_locked);
	def_property	(constantVelocity);
	def_property	(freeHandle);
	def_property	(tension);
	def_property	(continuity);
	def_property	(bias);
	def_property	(easeTo);
	def_property	(easeFrom);

// add implementations of the recursive time controller fns here to complain
// since they wuill default to operating on the key's controller which can be very confusing
// the user should use them on the controller or track, not the key
#ifdef def_time_fn
#	undef def_time_fn
#endif
#define def_time_fn(_fn)	\
	Value* _fn##_vf(Value** arg_list, int count) { throw NoMethodError (_M(#_fn), this); return (Value*)0; }	 
#include "..\protocols\timefunctions.inl"

};

