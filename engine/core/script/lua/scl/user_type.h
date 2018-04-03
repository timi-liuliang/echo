#pragma once

#include "scl/type.h"

namespace scl {

union user_data
{
	void*	ptr;
	int		value;
	int64	value64;

	user_data()			: value64	(0) {}
	user_data(void* p)	: ptr		(p) {}
	user_data(int v)	: value		(v) {}
	user_data(int64 v)	: value64	(v) {}
};

} //namespace scl
