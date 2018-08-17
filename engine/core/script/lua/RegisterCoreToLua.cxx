#include "LuaBinder.h"
#include "engine/core/log/Log.h"

namespace Echo
{
	void registerCoreToLua()
	{
		// core library
		{
			// 1.utils
			const char* utils = R"(
			utils = {}
			function utils.append_table(a, b)
			    for k, v in pairs(b) do
			        a[k] = v
			    end
			end

			nodes = {}
		
			function update_all_nodes()
			    for k, v in pairs(nodes) do
			        v:update()
			    end
			end
			)";

			LuaBinder::instance()->execString(utils, true);
		}
	}
}