/*	
 *		SceneIO.h - MAXScript-related scene file I/O (persistent globals, on-open script, etc.)
 *
 *			Copyright (c) Autodesk, Inc, 1998.  John Wainwright.
 *
 */

#pragma once
#include "mxsMaxFileVersion.h"

#include "..\ScripterExport.h"
#include "..\kernel\value.h"
#include "..\..\maxtypes.h"
#include "..\..\tab.h"
#include "..\..\ioapi.h"

// forward declarations
class ValueLoader;
class ILoad;
class ISave;

/* --------- Scene I/O chunk ID's ---------- */

#define OPENSCRIPT_CHUNK			0x0010    // obsoleted by CALLBACKSCRIPT_CHUNK
#define SAVESCRIPT_CHUNK			0x0020    // obsoleted by CALLBACKSCRIPT_CHUNK
#define PSGLOBALS_CHUNK				0x0030
#define MSPLUGINCLASS_CHUNK			0x0040
#define MSPLUGINCLASSHDR_CHUNK		0x0050
#define LENGTH_CHUNK				0x0060
#define CALLBACKSCRIPT_CHUNK		0x0070
#define CUSTATTRIBDEF_CHUNK			0x0080
#define SOURCE_CHUNK				0x00a0

/* ---- persistent global value loading ----- */

typedef Value* (*load_fn)(ILoad* iload, USHORT chunkID, ValueLoader* vl);

enum LoadableClassID 
{
	Undefined_Chunk = 0,	Boolean_Chunk,			Ok_Chunk,
	Integer_Chunk,			Float_Chunk,			String_Chunk, 
	Name_Chunk,				Array_Chunk,			Point3Value_Chunk, 
	QuatValue_Chunk,		RayValue_Chunk,			AngAxisValue_Chunk,
	EulerAnglesValue_Chunk, Matrix3Value_Chunk,		Point2Value_Chunk,
	ColorValue_Chunk,		MSTime_Chunk,			MSInterval_Chunk,
	MAXWrapper_Chunk,		Unsupplied_Chunk,		Struct_Chunk,
	Point4Value_Chunk,		Empty_Chunk,			Integer64_Chunk,
	DoubleValue_Chunk,

	// add more here...

	HIGH_CLASS_CHUNK  // must be last
};


extern ScripterExport Value* load_value(ILoad* iload, ValueLoader* vload);
extern void save_persistent_callback_scripts(ISave* isave);
extern IOResult load_persistent_callback_script(ILoad* iload);
extern Tab<ValueLoader*> value_loaders;

#ifdef _DEBUG
  extern BOOL dump_load_postload_callback_order;
#endif

// post global load callback scheme, allows different loaders to 
// permit ::Load() fns to register a callback to clean-up a load.  
// Eg, Array loader gets such a callback from MAXWrapper::Load() which
// uses this to build the MAXWrapper at post-load time, after object pointers
// have been back-patched.

// ::Load()'s that need to specialize this to provide a callback
class ValueLoadCallback
{
public:
	virtual Value* post_load() { return &undefined; }  // return the cleaned-up value
};

// each loader specializes this and gives it to the ::Load()
class ValueLoader
{
public:
	virtual void register_callback(ValueLoadCallback* cb) { UNUSED_PARAM(cb); }
	virtual void call_back() { }
};

// A post load callback to process persistent value load callbacks
class ValueLoadPLCB : public PostLoadCallback 
{
public:
	ScripterExport void proc(ILoad *iload);
};

// callback script  (see MAXCallbacks.cpp)
class CallbackScript 
{ 
public:
	MSTR	script;		// callback script or script filename
	Value*  code;		// cached compiled code
	Value*  id;			// script ID
	short	flags;		// flags

	CallbackScript(MCHAR* iscript, Value* iid, short iflags)
	{
		script = iscript; code = NULL; id = iid; flags = iflags;
	}
};

#define MCB_SCRIPT_IS_FILE		0x0001
#define MCB_PERSISTENT			0x0002
#define MCB_HAS_ID				0x0004
#define MCB_INVALID				0x0008

extern Tab<CallbackScript*>* callback_scripts[];

