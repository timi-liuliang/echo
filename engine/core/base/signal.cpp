#include "signal.h"
#include "class.h"
#include "variant.h"

namespace Echo
{
	void Connect::emit(const Variant** args, int argCount)
	{
		Variant::CallError error;
		m_method->call(m_target,  args, argCount, error);
	}

	bool Signal::connect(Object* obj, const Echo::String& methodName)
	{
		ClassMethodBind* method = Class::getMethodBind(obj->getClassName(), methodName);
		return method ? connect(obj, method) : false;
	}

	bool Signal::connect(Object* obj, ClassMethodBind* method)
	{
		m_connects.push_back(Connect(this, obj, method));

		return true;
	}
}