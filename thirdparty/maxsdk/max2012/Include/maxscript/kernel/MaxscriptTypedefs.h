
#pragma once

#include "..\..\maxtypes.h"
// forward declarations
class Value;
class ReferenceTarget;
class Interval;

typedef Value* (Value::*value_vf)(Value**, int);
typedef Value* (*value_cf)(Value**, int);
typedef Value* (Value::*getter_vf)(Value**, int);
typedef Value* (Value::*setter_vf)(Value**, int);
typedef Value* (*max_getter_cf)(ReferenceTarget*, Value*, TimeValue, Interval&);
typedef void   (*max_setter_cf)(ReferenceTarget*, Value*, TimeValue, Value*);
