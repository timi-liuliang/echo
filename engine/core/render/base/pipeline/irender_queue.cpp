#include "irender_queue.h"

namespace Echo
{
	void IRenderQueue::bindMethods()
	{
		CLASS_BIND_METHOD(IRenderQueue, getName, DEF_METHOD("getName"));
		CLASS_BIND_METHOD(IRenderQueue, setName, DEF_METHOD("setName"));

		CLASS_REGISTER_PROPERTY(IRenderQueue, "Name", Variant::Type::String, "getName", "setName");
	}
}