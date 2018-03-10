
#pragma once

#include "..\kernel\MAXScript_TLS.h"
#include "..\..\assert1.h"
// forward declarations
class Value;

/* value local macros - for managing C local variable references to Value*'s for the collector - see Collectable.cpp */

#define one_value_local(n1)											\
	struct { int count; Value** link; Value* n1; } vl =				\
		{ 1, NULL, NULL };											\
	vl.link = thread_local(current_locals_frame);					\
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define one_typed_value_local(n1)									\
	struct { int count; Value** link; n1; } vl =					\
		{ 1, NULL, NULL };											\
	vl.link = thread_local(current_locals_frame);					\
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define two_value_locals(n1, n2)									\
	struct { int count; Value** link; Value *n1, *n2; } vl =		\
		{ 2, NULL, NULL, NULL };									\
	vl.link = thread_local(current_locals_frame);					\
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define two_typed_value_locals(n1, n2)								\
	struct { int count; Value** link; n1; n2; } vl =				\
		{ 2, NULL, NULL, NULL };									\
	vl.link = thread_local(current_locals_frame);					\
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define three_value_locals(n1, n2, n3)								\
	struct { int count; Value** link; Value *n1, *n2, *n3; } vl =	\
		{ 3, NULL, NULL, NULL, NULL };								\
	vl.link = thread_local(current_locals_frame);					\
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define three_typed_value_locals(n1, n2, n3)						\
	struct { int count; Value** link; n1; n2; n3; } vl =			\
		{ 3, NULL, NULL, NULL, NULL };								\
	vl.link = thread_local(current_locals_frame);					\
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define four_value_locals(n1, n2, n3, n4)								\
	struct { int count; Value** link; Value *n1, *n2, *n3, *n4; } vl =	\
		{ 4, NULL, NULL, NULL, NULL, NULL };							\
	vl.link = thread_local(current_locals_frame);						\
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define four_typed_value_locals(n1, n2, n3, n4)						\
	struct { int count; Value** link; n1; n2; n3; n4; } vl =		\
		{ 4, NULL, NULL, NULL, NULL, NULL };						\
	vl.link = thread_local(current_locals_frame);					\
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define five_value_locals(n1, n2, n3, n4, n5)								\
	struct { int count; Value** link; Value *n1, *n2, *n3, *n4, *n5; } vl =	\
		{ 5, NULL, NULL, NULL, NULL, NULL, NULL };						    \
	vl.link = thread_local(current_locals_frame);						    \
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define five_typed_value_locals(n1, n2, n3, n4, n5)							\
	struct { int count; Value** link; n1; n2; n3; n4; n5; } vl =			\
		{ 5, NULL, NULL, NULL, NULL, NULL, NULL };						    \
	vl.link = thread_local(current_locals_frame);						    \
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define six_value_locals(n1, n2, n3, n4, n5, n6)								 \
	struct { int count; Value** link; Value *n1, *n2, *n3, *n4, *n5, *n6; } vl = \
		{ 6, NULL, NULL, NULL, NULL, NULL, NULL, NULL };						 \
	vl.link = thread_local(current_locals_frame);								 \
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define six_typed_value_locals(n1, n2, n3, n4, n5, n6)						\
	struct { int count; Value** link; n1; n2; n3; n4; n5; n6; } vl =		\
		{ 6, NULL, NULL, NULL, NULL, NULL, NULL, NULL };					\
	vl.link = thread_local(current_locals_frame);							\
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define seven_value_locals(n1, n2, n3, n4, n5, n6, n7)								  \
	struct { int count; Value** link; Value *n1, *n2, *n3, *n4, *n5, *n6, *n7; } vl = \
		{ 7, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };						  \
	vl.link = thread_local(current_locals_frame);									  \
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define seven_typed_value_locals(n1, n2, n3, n4, n5, n6, n7)				\
	struct { int count; Value** link; n1; n2; n3; n4; n5; n6; n7; } vl =	\
		{ 7, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };				\
	vl.link = thread_local(current_locals_frame);							\
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define eight_value_locals(n1, n2, n3, n4, n5, n6, n7, n8)								   \
	struct { int count; Value** link; Value *n1, *n2, *n3, *n4, *n5, *n6, *n7, *n8; } vl = \
		{ 8, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };					   \
	vl.link = thread_local(current_locals_frame);										   \
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define eight_typed_value_locals(n1, n2, n3, n4, n5, n6, n7, n8)				\
	struct { int count; Value** link; n1; n2; n3; n4; n5; n6; n7; n8; } vl =	\
		{ 8, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };			\
	vl.link = thread_local(current_locals_frame);								\
	thread_local(current_locals_frame) = (Value**)&vl;

// LAM - 6/07/02 - added new defines - no SDK impact	
#define nine_value_locals(n1, n2, n3, n4, n5, n6, n7, n8, n9)									\
	struct { int count; Value** link; Value *n1, *n2, *n3, *n4, *n5, *n6, *n7, *n8, *n9; } vl = \
		{ 9, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };						\
	vl.link = thread_local(current_locals_frame);												\
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define nine_typed_value_locals(n1, n2, n3, n4, n5, n6, n7, n8, n9)					\
	struct { int count; Value** link; n1; n2; n3; n4; n5; n6; n7; n8; n9; } vl =	\
		{ 9, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };			\
	vl.link = thread_local(current_locals_frame);									\
	thread_local(current_locals_frame) = (Value**)&vl;
	
#define ten_value_locals(n1, n2, n3, n4, n5, n6, n7, n8, n9, n10)									\
struct { int count; Value** link; Value *n1, *n2, *n3, *n4, *n5, *n6, *n7, *n8, *n9, *n10; } vl =	\
		{ 10, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };					\
		vl.link = thread_local(current_locals_frame);												\
		thread_local(current_locals_frame) = (Value**)&vl;

#define ten_typed_value_locals(n1, n2, n3, n4, n5, n6, n7, n8, n9, n10)				\
struct { int count; Value** link; n1; n2; n3; n4; n5; n6; n7; n8; n9; n10; } vl =	\
		{ 10, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };	\
		vl.link = thread_local(current_locals_frame);								\
		thread_local(current_locals_frame) = (Value**)&vl;

#define eleven_value_locals(n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11)								\
struct { int count; Value** link; Value *n1, *n2, *n3, *n4, *n5, *n6, *n7, *n8, *n9, *n10, *n11; } vl =	\
		{ 11, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };					\
		vl.link = thread_local(current_locals_frame);													\
		thread_local(current_locals_frame) = (Value**)&vl;

#define eleven_typed_value_locals(n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11)			\
struct { int count; Value** link; n1; n2; n3; n4; n5; n6; n7; n8; n9; n10; n11; } vl =	\
		{ 11, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };	\
		vl.link = thread_local(current_locals_frame);									\
		thread_local(current_locals_frame) = (Value**)&vl;

#define value_local_array(var, count) {								\
	var = &((Value**)_alloca(((count) + 2) * sizeof(Value*)))[2];	\
	memset(var, 0, (count) * sizeof(Value*));						\
   var[-2] = (Value*)(static_cast<INT_PTR>(count));            \
	var[-1] = (Value*)thread_local(current_locals_frame);			\
	thread_local(current_locals_frame) = &var[-2]; }

#define pop_value_local_array(var)									\
	thread_local(current_locals_frame) = (Value**)var[-1];
	
#define value_temp_array(var, count)	{							\
	var = &((Value**)ms_malloc(((count) + 2) * sizeof(Value*)))[2];	\
	memset(var, 0, (count) * sizeof(Value*));						\
   var[-2] = (Value*)(static_cast<INT_PTR>(count));            \
   var[-1] = (Value*)thread_local(current_locals_frame);       \
   thread_local(current_locals_frame) = &var[-2];}

// SR NOTE64: If your 'count' is the constant zero, then use this version instead otherwise
// you may end up with "warning C4318: passing constant zero as the length to memset"
// ie: value_temp_array(var, 0) should be value_temp_array0(var)
#define value_temp_array0(var)   {                              \
   var = &((Value**)ms_malloc(2 * sizeof(Value*)))[2];   \
   var[-2] = 0; \
	var[-1] = (Value*)thread_local(current_locals_frame);			\
	thread_local(current_locals_frame) = &var[-2];}

// LAM - 6/07/02 - fix for when multiple value_temp_arrays in same frame
#define realloc_value_temp_array(var, count, old_count)	{						\
	Value **oldPointer = &var[-2];												\
	Value **sframe = thread_local(current_locals_frame);						\
	var = &((Value**)ms_realloc(&var[-2], ((count) + 2) * sizeof(Value*)))[2];		\
	if ((count) > (old_count))													\
		memset(&var[(old_count)], 0, ((count) - (old_count)) * sizeof(Value*));	\
   var[-2] = (Value*)(static_cast<INT_PTR>(count));               \
	Value **newPointer = &var[-2];												\
	if (sframe == oldPointer)													\
	{	thread_local(current_locals_frame) = newPointer;						\
		DbgAssert( newPointer != (Value**)newPointer[1] );						\
	}																			\
	else																		\
	{	Value **frame;															\
		for (frame = sframe; frame && (Value**)frame[1] != oldPointer && frame != (Value**)frame[1]; frame = (Value**)frame[1]);	\
		if (frame) {															\
			DbgAssert( frame != (Value**)frame[1] );							\
			frame[1] = (Value*)newPointer;										\
		}																		\
	}																			\
	}

// LAM - 6/07/02 - fix for when multiple value_temp_arrays in same frame
#define pop_value_temp_array(var)	{							\
	Value **oldPointer = &var[-2];								\
/*	Value **sframe = thread_local(current_locals_frame); */		\
	Value **head = (Value**)var[-1];							\
	thread_local(current_locals_frame) = head;					\
	ms_free(oldPointer);											\
	}
	
#define return_value(r)	{										\
	thread_local(current_result) = r;							\
	thread_local(current_locals_frame) = vl.link;				\
	return r; }
	 
#define return_value_no_pop(r) {								\
	thread_local(current_result) = r;							\
	return r; }

#define return_protected(r) {									\
	thread_local(current_result) = r;							\
	return r; }

#define pop_value_locals()										\
	thread_local(current_locals_frame) = vl.link;

#define reset_locals_frame()									\
	thread_local(current_locals_frame) = (Value**)&vl;

#define reset_locals_array_frame(var)							\
	thread_local(current_locals_frame) = &var[-2]; 

#define clear_current_frames() {								\
	thread_local(current_locals_frame) = NULL;					\
	thread_local(current_frame) = NULL;}

#define save_current_frames()									\
	Value** _sclf = thread_local(current_locals_frame);			\
	Value** _scsf = thread_local(current_scan_frame);			\
	Value** _scf = thread_local(current_frame);
	
#define restore_current_frames() {								\
	thread_local(current_locals_frame) = _sclf;					\
	thread_local(current_scan_frame) = _scsf;					\
	thread_local(current_frame) = _scf;}

#define save_current_source()									\
	Value* _save_source = thread_local(source_file);			\
	MSZipPackage* _save_pkg = thread_local(current_pkg);		\
	unsigned int  _save_pos = thread_local(source_pos);			\
	unsigned int _save_line = thread_local(source_line);		\
	DWORD _save_flags = thread_local(source_flags);

#define restore_current_source() {								\
	thread_local(source_file) = _save_source;					\
	thread_local(current_pkg) = _save_pkg;						\
	thread_local(source_pos) = _save_pos;						\
	thread_local(source_line) = _save_line;						\
	thread_local(source_flags) = _save_flags;}

#define save_current_source_no_pos()							\
	Value* _save_source = thread_local(source_file);			\
	MSZipPackage* _save_pkg = thread_local(current_pkg);		\
	DWORD _save_flags = thread_local(source_flags);

#define restore_current_source_no_pos() {						\
	thread_local(source_file) = _save_source;					\
	thread_local(current_pkg) = _save_pkg;						\
	thread_local(source_flags) = _save_flags;}

#define save_current_source_pos()								\
	unsigned int  _save_pos = thread_local(source_pos);			\
	unsigned int _save_line = thread_local(source_line);		\
	DWORD _save_flags = thread_local(source_flags);

#define restore_current_source_pos() {							\
	thread_local(source_pos) = _save_pos;						\
	thread_local(source_line) = _save_line;						\
	thread_local(source_flags) = _save_flags;}

#define save_current_source_to_frame() {						\
	Value** _frame = thread_local(current_scan_frame);			\
	if (_frame) _frame[3] = thread_local(source_file);			\
	if (_frame) _frame[4] = (Value*)thread_local(source_pos);	\
	if (_frame) _frame[8] = (Value*)thread_local(source_line);  \
	if (_frame) _frame[7] = (Value*)static_cast<DWORD_PTR>(thread_local(source_flags));}

#define save_current_source_to_frame_no_pos() {					\
	Value** _frame = thread_local(current_scan_frame);			\
	if (_frame) _frame[3] = thread_local(source_file);			\
	if (_frame) _frame[7] = (Value*)static_cast<DWORD_PTR>(thread_local(source_flags));}

#define save_current_source_to_frame_pos() {					\
	Value** _frame = thread_local(current_scan_frame);			\
	if (_frame) _frame[4] = (Value*)thread_local(source_pos);	\
	if (_frame) _frame[8] = (Value*)thread_local(source_line);}

#define save_current_source_to_frame_no_file() {				\
	Value** _frame = thread_local(current_scan_frame);			\
	if (_frame) _frame[4] = (Value*)thread_local(source_pos);   \
	if (_frame) _frame[8] = (Value*)thread_local(source_line);}

#define push_control(_c)										\
	Control* _save_cc = thread_local(current_controller);		\
	thread_local(current_controller) = _c;

#define pop_control()											\
	thread_local(current_controller) = _save_cc;

// When an exception occurs, mxs stores data regarding source
// code location corresponding to the code generating the error.
// This data is used by the debugger and to display the source code
// in the editor
// Following macro stores this data in the thread local variables
#define save_error_source_data() {								\
	thread_local(err_source_file) = thread_local(source_file);	\
	thread_local(err_source_pos) = thread_local(source_pos);	\
	thread_local(err_source_line)= thread_local(source_line);   \
	thread_local(err_source_flags) = thread_local(source_flags);\
	thread_local(err_occurred) = TRUE;}

// The following macro clears the data associated with errors.
// This macro must be used if a catch "eats" an mxs runtime exception.
// A catch statement "eats" an exception if it does not either
// call show_source_pos() nor rethrow the exception.
#define clear_error_source_data() {								\
	thread_local(err_source_file) = NULL;						\
	thread_local(err_source_pos) = 0;							\
	thread_local(err_source_flags) = 0;							\
	thread_local(err_source_line) = 0;							\
	thread_local(err_occurred) = FALSE;}
