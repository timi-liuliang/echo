#include "LuaBinder.h"
#include "engine/core/log/Log.h"

namespace Echo
{
	void register_core_to_lua()
	{
		// core library
		{
			// 1.utils
			String utils = ""\
			"utils = {}\n\n"\
			"function utils.append_table(a, b)\n"\
			"    for k, v in pairs(b) do\n"\
			"        a[k] = v\n"\
			"    end\n"\
			"end\n"\
			"\n"\
			"_nodes = {}\n"\
			"\n"\
			"function _update_all_nodes()\n"\
			"    for k, v in pairs(_nodes) do\n"\
			"        v:update()\n"\
			"    end\n"\
			"end\n"\
			"\n"\
			"";
			LuaBinder::instance()->execString(utils.c_str(), true);
		}
	}
}