#include "LuaBinder.h"
#include "engine/core/log/Log.h"

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

// 2.math
const char* math = R"(
local metatable = { x = 0, y = 0, z = 0 }
metatable.__index = metatable

function metatable:add()
  self.x = self.y + self.z
end

function vec3(xv, yv, zv) 
  return setmetatable( {x=xv, y=yv, z=zv}, metatable)
end
)";

namespace Echo
{
	void registerCoreToLua()
	{
		// core library
		{
			LuaBinder::instance()->execString(utils, true);
			LuaBinder::instance()->execString(math, true);
		}
	}
}