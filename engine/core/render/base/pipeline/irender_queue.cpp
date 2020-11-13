#include "irender_queue.h"

namespace Echo
{
	void IRenderQueue::bindMethods()
	{
		CLASS_BIND_METHOD(IRenderQueue, getName,   DEF_METHOD("getName"));
		CLASS_BIND_METHOD(IRenderQueue, setName,   DEF_METHOD("setName"));
		CLASS_BIND_METHOD(IRenderQueue, isEnable,  DEF_METHOD("isEnable"));
		CLASS_BIND_METHOD(IRenderQueue, setEnable, DEF_METHOD("setEnable"));

		CLASS_REGISTER_PROPERTY(IRenderQueue, "Name", Variant::Type::String, "getName", "setName");
		CLASS_REGISTER_PROPERTY(IRenderQueue, "Enable", Variant::Type::Bool, "isEnable", "setEnable");
	}
}