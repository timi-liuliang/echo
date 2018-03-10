//**************************************************************************/
// Copyright (c) 1998-2008 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
#pragma once


//*! \brief A replacement for the declare_local_generic_class macro shown above.
/*	This macro declares two classes for exposing a type to maxscript. */ 
#define DECLARE_LOCAL_GENERIC_CLASS( _rootclass, _cls )                      \
	class _cls##Class : public ValueMetaClass                               \
	{                                                                       \
	public:                                                                 \
		_cls##Class(MCHAR* name);                                          \
		void collect();                                                    \
	};                                                                      \
	extern _cls##Class _cls##_class;                                        \
	class _rootclass;                                                       \
	typedef Value* (_rootclass::*local_value_vf)(Value**, int);             \
	class _cls : public Generic                                             \
	{                                                                       \
	public:                                                                 \
		local_value_vf fn_ptr;                                             \
		_cls();                                                            \
		_cls(MCHAR* name, local_value_vf fn);                              \
		classof_methods (_cls, Generic);                                   \
		void collect();                                                    \
		Value* apply(Value** arglist, int count, CallContext* cc = NULL);  \
	};

#define DEFINE_LOCAL_GENERIC_CLASS( _rootClass, _cls )						\
	_cls##Class::_cls##Class(MCHAR* name)                                      \
			: ValueMetaClass (name)                                          \
	{                                                                          \
	}                                                                          \
	void _cls##Class::collect()                                                \
	{                                                                          \
		delete this;                                                          \
	}                                                                          \
	                                                                           \
	_cls::_cls()                                                               \
	{                                                                          \
	}                                                                          \
	_cls::_cls(MCHAR*fn_name, local_value_vf fn)											\
	{																			\
		tag = &(_cls##_class);														\
		fn_ptr = fn;																\
		name = save_string(fn_name);													\
	}																			\
	Value* _cls::apply(Value** arg_list, int count, CallContext* cc)							\
	{																			\
		if (thread_local(is_dubugger_thread) && !theMXSDebugger->GetAllowUnsafeMethods())		\
			throw DebuggerRuntimeError("Method not available from debugger: ",name);			\
		Value*  result = NULL;														\
		Value**	evald_args;														\
		if (count < 1)																\
			throw ArgCountError("Generic apply", 1, count);								\
		value_local_array(evald_args, count);											\
		Value** ap = arg_list;														\
		Value** eap = evald_args;													\
		for (int i = count; i > 0; i--)												\
		{																		\
			*eap = (*ap)->eval();													\
			eap++;																\
			ap++;																\
		}																		\
		if (evald_args[0]->local_base_class() == &_rootClass##_class)						\
			result = (((_rootClass*)evald_args[0])->*fn_ptr)(&evald_args[1], count - 1);		\
		else																		\
			throw NoMethodError (name, evald_args[0]);									\
		pop_value_local_array(evald_args);												\
		return result;																\
	}                                                                                              \
	void _cls::collect()                                                                           \
	{                                                                                              \
		delete this;                                                                              \
	}                                                                                              \
	_cls##Class _cls##_class (#_cls);


#define DEFINE_LOCAL_GENERIC_CLASS_DEBUG_OK( _rootClass, _cls )                 \
	_cls##Class::_cls##Class(MCHAR* name)                                      \
			: ValueMetaClass (name)                                          \
	{                                                                          \
	}                                                                          \
	void _cls##Class::collect()                                                \
	{                                                                          \
		delete this;                                                          \
	}                                                                          \
	                                                                           \
	_cls::_cls()                                                               \
	{                                                                          \
	}                                                                          \
	_cls::_cls(MCHAR*fn_name, local_value_vf fn)											\
	{																			\
		tag = &(_cls##_class);														\
		fn_ptr = fn;																\
		name = save_string(fn_name);													\
		flags3 |= VALUE_FLAGBIT_0;													\
	}																			\
	Value* _cls::apply(Value** arg_list, int count, CallContext* )							\
	{																			\
		Value*  result = NULL;														\
		Value**	evald_args;														\
		if (count < 1)																\
			throw ArgCountError("Generic apply", 1, count);								\
		value_local_array(evald_args, count);											\
		Value** ap = arg_list;														\
		Value** eap = evald_args;													\
		for (int i = count; i > 0; i--)												\
		{																		\
			*eap = (*ap)->eval();													\
			eap++;																\
			ap++;																\
		}																		\
		if (evald_args[0]->local_base_class() == &_rootClass##_class)						\
			result = (((_rootClass*)evald_args[0])->*fn_ptr)(&evald_args[1], count - 1);		\
		else																		\
			throw NoMethodError (name, evald_args[0]);									\
		pop_value_local_array(evald_args);												\
		return result;																\
	}																			\
	void _cls::collect()                                                                           \
	{                                                                                              \
		delete this;                                                                              \
	}																			\
	_cls##Class _cls##_class (#_cls);