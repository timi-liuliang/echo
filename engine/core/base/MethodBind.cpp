#include "MethodBind.h"

namespace Echo
{
#ifdef DEBUG_METHODS_ENABLED
	Echo::MethodDefinition DEF_METHOD(const String& name)
	{
		MethodDefinition def(name);
		return def;
	}

	Echo::MethodDefinition DEF_METHOD(const String& name, const String& arg0)
	{
		MethodDefinition def(name);
		def.m_args.push_back(arg0);
		return def;
	}
#endif
}