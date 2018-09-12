#include "method_bind.h"
#include "engine/core/script/lua/lua_binder.h"

namespace Echo
{
	// register method
	bool registerMethodBind( const String& methodName, MethodBind* method)
	{
		// lua
		LuaBinder::instance()->registerMethod(methodName, method);

		return true;
	}
}