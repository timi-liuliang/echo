#include "irender_queue.h"

namespace Echo
{
	void IRenderQueue::bindMethods()
	{
		CLASS_BIND_METHOD(IRenderQueue, getName);
		CLASS_BIND_METHOD(IRenderQueue, setName);
		CLASS_BIND_METHOD(IRenderQueue, isEnable);
		CLASS_BIND_METHOD(IRenderQueue, setEnable);

		CLASS_REGISTER_PROPERTY(IRenderQueue, "Name", Variant::Type::String, getName, setName);
		CLASS_REGISTER_PROPERTY(IRenderQueue, "Enable", Variant::Type::Bool, isEnable, setEnable);
	}
}